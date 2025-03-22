#include "devcart.h"
#include "command_ntr.h"
#include "protocol_ntr.h"
#include "ui.h"
#include "fs.h"
#include "crc32.h"

void get_pagesize(uint8_t num, t_nand *nand)
{
    if ((num | PAGESIZE_1K) == PAGESIZE_1K)
    {
        nand->pagesize = PAGESIZE_1K;
        Debug("1kbit page");
    }
    if ((num & PAGESIZE_2K) == PAGESIZE_2K)
    {
        nand->pagesize = PAGESIZE_2K;
        Debug("2kbit page");
    }
    if ((num & PAGESIZE_4K) == PAGESIZE_4K)
    {
        nand->pagesize = PAGESIZE_4K;
        Debug("4kbit page");
    }
    if ((num & PAGESIZE_8K) == PAGESIZE_8K)
    {
        nand->pagesize = PAGESIZE_8K;
        Debug("8kbit page");
    }
}

void get_blocksize(uint8_t num, t_nand *nand)
{
    uint8_t byte = ((num >> 4) << 4);
    if ((byte | BLOCKSIZE_64K) == BLOCKSIZE_64K)
    {
        nand->blocksize = BLOCKSIZE_64K;
        Debug("64kbit block");
    }
    if ((byte & BLOCKSIZE_128K) == BLOCKSIZE_128K)
    {
        nand->blocksize = BLOCKSIZE_128K;
        Debug("128kbit block");
    }
    if ((byte & BLOCKSIZE_256K) == BLOCKSIZE_256K)
    {
        nand->blocksize = BLOCKSIZE_256K;
        Debug("256kbit block");
    }
    if ((byte & BLOCKSIZE_512K) == BLOCKSIZE_512K)
    {
        nand->blocksize = BLOCKSIZE_512K;
        Debug("512kbit block");
    }
}

int s1_read_copts(void *COPTS_data){

    char filename[256];
    snprintf(filename, sizeof(filename), "0:/temp_copts.bin");
    NTR_Cmd6D(COPTS_data);
    FileSetData(filename, COPTS_data, 0x800, 0, true);

    return 1;
}



int s1_write_copts(void *COPTS_data)
{
    uint8_t buffer[4];

    char output[3 * 30 + 1];  // 2 chars per byte + 1 space + null terminator
    output[0] = '\0';         // Start with an empty string

    NTR_Cmd91(COPTS_data);
    NTR_Cmd6F(buffer);

    while (!(buffer[0] & (1 << 6))) {
        NTR_Cmd6F(buffer);
        // Debug("Waiting");
    }

    if (!(buffer[0] & (1 << 0))) //pass
    {
        return 1;
    }else{
        return 0;
    }

    return 0;
}

// Checks the CRC32 of the file vs the CRC of the Cartirdge. 
int s1_erase(size_t nb_blocks){
    u32 address_multiplier = 0x40;

    u32 blk_num, page_in_blk;
    blk_num = 0;

    ShowProgress(blk_num, nb_blocks * address_multiplier, "Erasing Cartridge!");
    uint8_t buffer[4];

    int send_twice = 0;

    while (blk_num < nb_blocks * address_multiplier)
    {

        NTR_Cmd9D(blk_num);
        NTR_Cmd6F(buffer);

        while (!(buffer[0] & (1 << 6))) //busy
        {
            //poll again, change 6f to take a pointer as parameter or we will run out of mem
            NTR_Cmd6F(buffer);
        }

        // Use the bit rather than the byte
        if(buffer[0] != 0xE0){
            Debug("Bad Block: %d Buffer: %02X%02X%02X%02X",(blk_num / address_multiplier), buffer[0],buffer[1],buffer[2],buffer[3] );
        }

        if(blk_num % 100 == 0)
            ShowProgress(blk_num, nb_blocks * address_multiplier, "Erasing Cartridge!");

        send_twice++;
        if (send_twice == 2) {
            blk_num += address_multiplier;
            send_twice = 0;
        }
    }

    return 1;
}

// Checks the CRC32 of the file vs the CRC of the Cartirdge. 
int s1_verify_data(size_t nb_pages, void *filepath){

    u32 blk_num = 0;
    u32 curr_offset = 0;
    uint8_t crc_data_buffer[0x80000];
    u32 crc32 = ~0;
    uint8_t buffer[4];
    u32 address_multiplier = 0x40;

    while (blk_num < nb_pages)
    {
        crc32 = ~0; 
        memset(crc_data_buffer, 0xFF, sizeof(crc_data_buffer));
        FileGetData(filepath, crc_data_buffer, 0x80000, curr_offset);

        NTR_Cmd9B(blk_num);
        //Calculate 3DS Checksum while we wait for cartridge. 
        crc32 = crc32_calculate(crc32, crc_data_buffer, 0x80000);
        crc32 = ~crc32; 
        NTR_Cmd68(buffer);

        while (!(buffer[0] & (1 << 6))) //busy
        {
            //poll again, change 68 to take a pointer as parameter or we will run out of mem
            NTR_Cmd68(buffer);
        }
        NTR_Cmd97(buffer);
        u32 cart_crc = buffer[3] | (buffer[2] << 8) | (buffer[1] << 16) | (buffer[0] << 24);
        if(cart_crc == crc32){
            // Prints Debug if needed. 
        }else{
            Debug("Block %d CRC File Offset: %lu Check *FAIL*",blk_num / address_multiplier, curr_offset);
            Debug("Cart CRC32 %8X - File CRC32 %8X", cart_crc, crc32);
            // ShowPrompt(false, "Verification Failed!");
            return 0;
        }

        if(blk_num % 10 == 0)
            ShowProgress(blk_num, nb_pages, "Verifying!");
        blk_num += address_multiplier;
        curr_offset+=0x80000;
    }

    return 1;

}

int s1_write_data(size_t nb_pages, void *filepath){
    uint8_t data_buffer [0x200];
    size_t curr_page = 0;
    size_t curr_address = 0;
    u32 curr_offset = 0;
    u32 cmd_dummy[2] = {0x02000000, 0x00000000};
    uint8_t status_buff[4];
    uint8_t buffer[4];

    ShowProgress(curr_page, nb_pages, "Writing Data!");

    uint8_t block_buffer[0x80000];  // 512 KiB temp buffer (Minimise reading from SD)
    u32 pages_loaded = 0;
    while(curr_page < nb_pages) {
        if (pages_loaded == 0) {
            memset(block_buffer, 0xFF, sizeof(block_buffer));
            FileGetData(filepath, block_buffer, 0x80000, curr_offset);
        }

        for(int i = 0; i < 16; i++) {
            u8* chunk = &block_buffer[(pages_loaded * 0x2000) + (i * 0x200)];
            if (i == 0) {
                NTR_Cmd92(curr_address, chunk);
            }else {
                NTR_SendCommandWrite(cmd_dummy, 0x200, 0x100, chunk);
            }
            curr_offset += 0x200;
        }

        NTR_Cmd6F(status_buff);
        while (!(status_buff[0] & (1 << 6))) //busy
        {
            //poll again, change 6f to take a pointer as parameter or we will run out of mem
            NTR_Cmd6F(status_buff);
            if ((status_buff[0] & (1 << 6))) //ready
            {
                if (!(status_buff[0] & (0 << 0))) //pass
                {   
                    break;
                }else{
                    Debug("Critical Error Writing Data!");
                    return 0;
                }
            }
        }
        curr_address += 0x4;
        curr_page += 1;
        pages_loaded++;

        if (pages_loaded >= 64)
            pages_loaded = 0;

        if(curr_page % 10 == 0)
            ShowProgress(curr_page, nb_pages, "Writing Data!");
    }
    return 1;
}