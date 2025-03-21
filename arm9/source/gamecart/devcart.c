#include "devcart.h"
#include "command_ntr.h"
#include "ui.h"

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


int write_copts(void *COPTS_data)
{
    uint8_t buffer[4];

    char output[3 * 30 + 1];  // 2 chars per byte + 1 space + null terminator
    output[0] = '\0';         // Start with an empty string

    // for (int i = 0; i < 30; i++) {
    //     char byte_str[4];     // Enough for "FF " and null terminator
    //     snprintf(byte_str, sizeof(byte_str), "%02X ", COPTS_data[i]);
    //     strncat(output, byte_str, sizeof(output) - strlen(output) - 1);
    // }

    // Debug("FileBuffer: %08X", file_buff);

    NTR_Cmd91(COPTS_data);
    NTR_Cmd6F(buffer);

    while (!(buffer[0] & (1 << 6))) {
    
        NTR_Cmd6F(buffer);
        Debug("Waiting");
    }

    if (!(buffer[0] & (1 << 0))) //pass
    {
        Debug("COPTS was written successfully!");
        return 1;
    }else{
        Debug("ERROR! COPTS was NOT Written successfully!");
        return 0;
    }

    return 0;
}