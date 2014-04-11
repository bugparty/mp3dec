#ifndef  BITSTREAM_H
#define BITSTREAM_H
#include "mp3dec.h"
void bitstream_init();
int bitstream_append(int len);
void bitstream_reset_index();
BYTE bitstream_get1Bit();
DWORD bitstream_getBits17(int n);
WORD bitstream_getBits9(int n);
unsigned long bitstream_getBytePos();
unsigned long bitstream_getBuffBytes();
unsigned long bitstream_getBitPos();
BYTE bitstream_get1Byte();
void bitstream_backBits(int n);
void bitstream_skipBytes(int nbytes);
#endif
