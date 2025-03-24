#ifndef DEVCART_H
#define DEVCART_H
#include "common.h"

//page size mask
#define PAGESIZE_1K 0b00000000
#define PAGESIZE_2K 0b00000001
#define PAGESIZE_4K 0b00000010
#define PAGESIZE_8K 0b00000011

//block size mask
#define BLOCKSIZE_64K 0b00000000
#define BLOCKSIZE_128K 0b00010000
#define BLOCKSIZE_256K 0b00100000
#define BLOCKSIZE_512K 0b00110000

//manufacturer codes
#define TOSHIBA 0x98
#define UNK_1 0xEC

typedef struct s_nand
{
    uint8_t pagesize;
    uint8_t blocksize;
    uint8_t chipid;
    uint8_t maker_code;
} t_nand;

void get_pagesize(uint8_t num, t_nand *nand);
void get_blocksize(uint8_t num, t_nand *nand);
int v1_read_copts(void *COPTS_data);
int v1_write_copts(void  *COPTS_data);
int v1_erase(size_t nb_blocks);
int v1_verify_data(size_t nb_pages, void *loadpath, void* dec_title_key);
int v1_write_data(size_t nb_pages, void *filepath, void* dec_title_key);

// Some stuff to help with Card Crypto
void rol128(uint8_t* dst, const uint8_t* src, int rbits);
void derive_title_key(uint8_t* out_key, const uint8_t* keyX, const uint8_t* keyY);
int decrypt_card_title_Key(uint8_t* seed, uint8_t* mac, uint8_t* nonce, bool is_debug_signed, uint8_t* encrypted_key, uint8_t* decrypted_key);


#endif