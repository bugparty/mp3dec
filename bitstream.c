#include "bitstream.h"
#include <stdlib.h>
static const int RESELEN = 4096;
static BYTE * bitReservoir;   ////bitReservoir��������ֽ���
static unsigned long bitPos=0, bytePos=0, endPos=0;

void bitstream_init()
{
    bitReservoir = (BYTE*)malloc(RESELEN +4); // ���Ȳ�С��512+1732(���֡��)
}
void bitstream_setbuf_size(size_t size)
{
    bitReservoir = (BYTE*)malloc(size);  // ���ⳤ��,����֡����Ϣ����
}
int bitstream_append(int len)
{
    if(len+endPos > RESELEN)
    {
        //��������bytePos��֮����ֽ��ƶ�����������
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
* �ӻ�����bitReservoir��ȡ1 bit
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
    bytePos += bitPos >> 3;//ֻ����8�ı���,Ҳ�����ֽ���
    bitPos &= 0x7;//0111  ����λ���϶���8�ı�����ֱ������
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
        //wRet &= 0xffff;  // ��16λ��0;
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
     * ����ֵ��0-255���޷�������
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
