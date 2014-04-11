#ifndef IO_H
#define IO_H
#include "mp3dec.h"
//#include "../id3lib/id3lib.h"
#include <stdio.h>
#include <malloc.h>
void io_open(char * filename, int intBufSize);
int io_read_off(long offset);
BYTE io_read();
int io_reads_dir(BYTE* b, int off, int len);
int io_reads(BYTE* dstBuffer, int offset, int len);
int io_dump(int src_off, BYTE* b, int dst_off, int len);
void io_seek(long pos);
long io_length();
size_t io_offset();
void io_close();
//varibles
extern long lFrameOffset;
extern FILE * fin;

#endif
