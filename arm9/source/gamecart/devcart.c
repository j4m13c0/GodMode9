#include "devcart.h"
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