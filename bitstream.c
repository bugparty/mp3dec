#include "bitstream.h"
#include <stdlib.h>
static const int RESELEN = 4096;
static BYTE * bitReservoir;   ////bitReservoir已填入的字节数
static unsigned long bitPos=0, bytePos=0, endPos=0;

void bitstream_init()
{
    bitReservoir = (BYTE*)malloc(RESELEN +4); // 长度不小于512+1732(最大帧长)
}
void bitstream_setbuf_size(size_t size)
{
    bitReservoir = (BYTE*)malloc(size);  // 任意长度,用于帧边信息解码
}
int bitstream_append(int len)
{
    if(len+endPos > RESELEN)
    {
        //将缓冲区bytePos及之后的字节移动到缓冲区首
        memcpy(bitReservoir+bytePos, bitReservoir, endPos - bytePos);
        endPos -= bytePos;
        bitPos = bytePos = 0;
    }

    if(io_reads(bitReservoir, endPos, len) < len)
        return -1;

    endPos += len;
    return len;
}
void bitstream_reset_index()
{
    bytePos = bitPos = endPos = 0;
}

/*
* 从缓冲区bitReservoir读取1 bit
                                */
BYTE bitstream_get1Bit()
{
    int bit;
    bit = bitReservoir[bytePos] << bitPos;
    bit >>= 7;
    bit &= 0x1;
    bitPos++;
    bytePos += bitPos >> 3;
    bitPos &= 0x7;
    return bit;
}
// 2 <= n <= 17
DWORD bitstream_getBits17(int n)
{
    DWORD dwRet = bitReservoir[bytePos];
    dwRet <<= 8;
    dwRet |= bitReservoir[bytePos +1] & 0xff;
    dwRet <<= 8;
    dwRet |= bitReservoir[bytePos +2] & 0xff;
    dwRet <<= bitPos;
    dwRet >>= 24 - n;
    bitPos += n;
    bytePos += bitPos >> 3;//只保留8的倍数,也就是字节数
    bitPos &= 0x7;//0111  第四位以上都是8的倍数，直接消掉
    return dwRet;

}
/**
* 2<= n <= 9
*/
WORD bitstream_getBits9(int n) {
        WORD wRet;
        wRet = bitReservoir[bytePos];
        wRet <<= 8;
        wRet |= bitReservoir[bytePos + 1] ;
        wRet <<= bitPos;
        //wRet &= 0xffff;  // 高16位置0;
        wRet >>= 16 - n;
        bitPos += n;
        bytePos += bitPos >> 3;
        bitPos &= 0x7;
        return wRet;
    }

unsigned long bitstream_getBytePos() {
        return bytePos;
}

unsigned long bitstream_getBuffBytes() {
        return endPos;
    }

unsigned long bitstream_getBitPos() {
        return bitPos;
    }

    /**
     * 返回值是0-255的无符号整数
     */
BYTE bitstream_get1Byte() {
        return bitReservoir[bytePos++] & 0xff;
    }

void bitstream_backBits(int n) {
        bitPos -= n;
        bytePos += bitPos >> 3;
        bitPos &= 0x7;
    }

void bitstream_skipBytes(int nbytes) {
        bytePos += nbytes;
        bitPos = 0;
    }
