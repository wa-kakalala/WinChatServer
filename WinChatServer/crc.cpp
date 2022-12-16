#include "crc.h"
#include <stdio.h>

// ²Î¿¼: https://gitee.com/whik/crc-lib-c
/******************************************************************************
 * Name:    CRC-32/MPEG-2  x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1
 * Poly:    0x4C11DB7
 * Init:    0xFFFFFFF
 * Refin:   False
 * Refout:  False
 * Xorout:  0x0000000
 * Note:
 *****************************************************************************/
unsigned int crc32(unsigned char* data, unsigned short length, unsigned int poly)
{
    unsigned char i;
    int crc = 0xffffffff;  // Initial value
    while (length--)
    {
        crc ^= (int)(*data++) << 24;// crc ^=(uint32_t)(*data)<<24; data++;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 0x80000000)
                crc = (crc << 1) ^ poly;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/******************************************************************************
 * Name:    CRC-32/MPEG-2  x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1
 * Poly:    0x4C11DB74C11DB7
 * Init:    0xFFFFFFFFFFFFFF
 * Refin:   False
 * Refout:  False
 * Xorout:  0x00000000000000
 * Note:
 *****************************************************************************/
unsigned long long crc64(unsigned char* data, unsigned short length, unsigned long long poly)
{
    unsigned char i;
    unsigned long long crc = 0xffffffffffffffff;  // Initial value
    while (length--)
    {
        crc ^= (unsigned long long)(*data++) << 56;// crc ^=(uint32_t)(*data)<<24; data++;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 0x8000000000000000)
                crc = (crc << 1) ^ poly;
            else
                crc <<= 1;
        }
    }
    return crc;
}