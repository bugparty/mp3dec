#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "io.h"
#define bool2Str(value)  ((value) ? "true": "false")
void test_io_c()
{
    BYTE byte;

    WORD word1, word2;
    size_t offset, length;

//***********testing for io.c**********************
    printf("test io.c!\n");
    io_open("d:/sound/id3tagID3V1_ID3V2_3.mp3", 2048);
    length = io_length();
    printf("the file size is %i\n", length);

    printf("seek to %i\n", lFrameOffset);
    io_seek(lFrameOffset);
    offset = io_offset();
    printf("now the position is %i\n", offset);

    printf("the mp3 frame offset is %i\n", lFrameOffset);
    byte = io_read();
    printf("one byte from io_read:%#x\n", byte);
    offset = io_offset();
    printf("now the position is %i\n", offset);
    byte = io_read();
    printf("one byte from io_read:%#x\n", byte);
    offset = io_offset();
    printf("now the position is %i\n", offset);
    io_dump(-2 , &word1, 0, 2);
    printf("dump a word from %i,:%#x\n", -2, word1);
    io_seek(lFrameOffset);
    io_reads(&word2, 0, 2);
    printf("io_reads :%#x\n",  word2);

    io_close();
//***********end of  io.c**********************

}
bool False()
{
    return false;
}
void test_mp3frame_c_old()
{
    WORD h;
    DWORD dw;
    int i;
    bool flag1 = false, flag2;
    size_t offset;
    PAUDIO_HEADER hdr;
    int bitrate,samplerate;

    puts("testing mp3frame.c!");




    io_open("d:/sound/320k.mp3", 1000000);
    io_seek(lFrameOffset);
    printf("the offset is %#08x\n", lFrameOffset);

    io_reads(&dw, 0, 4);
    dw = dwM2L(dw);
    i = frame_check_sync(dw);
    flag2 = False();

    printf("check_sync %s\n", i? "OK": "Oh,no");
    io_seek(lFrameOffset);
    offset = frame_search_header(lFrameOffset);
    printf("header is located:%#08x\n", offset);

   /* hdr = frame_get_header(offset);
    frame_praseHeader(hdr);
    bitrate = frame_get_bit_rate(hdr);
    printf("bitrate is %i\n", bitrate);
    */

    samplerate = frame_get_sample_rate(hdr);
    printf("sample rate is %i\n", samplerate);
    printf("framesize is %i\n", frame_getFrameSize());
    printf("MPEG layer is %i\n", frame_get_layer());
    printf("Channels %i\n", frame_getChannels());

    printf("is protected %s\n", bool2Str(frame_is_protected(hdr)));
    printf("is MSSteroeo %s\n", bool2Str(frame_isMSStereo()));
    printf("is ISteroeo %s\n", bool2Str(frame_isIStereo()));


}
void test_bitstream_c()
{
    BYTE a,b;
    WORD da;
    int i;
    char textbuf[13];
    io_open("d:/sound/320k.mp3", 1000000);
    bitstream_init();
    bitstream_append(10);

    b = bitstream_getBits9(8);
    printf("get9bit %s\n", Byte2Str(b, textbuf));

    for(i=0 ; i<8 ; i++)
    {
        a = bitstream_get1Bit();
        printf("get1bit %#04x\n", a);
    }
    bitstream_reset_index();
    da = bitstream_getBits17(16);
    printf("get17bit %s\n", Word2Str(da, textbuf));

}
void test_ulits_c()
{
    char buffer[10];
    DWORD dw;

    dw = dwM2L(0x12345678);
    printf("12345678 to LSB %x\n", dw);
    void_dwM2L(&dw);
    printf("87654321 to LSB %x\n", dw);
    printf("hex 49  01001001 :%s", Byte2Str(0x49, buffer));
    printf("hex fe:%s", Byte2Str(0xfe, buffer));


}

void test_mp3frame_c()
{
    io_open("res/320k.mp3", 1000000);
    while(frame_syncFrame())
        frame_printStatus();

}
void main()
{

   //test_io_c();
   //test_ulits_c();
   //test_bitstream_c();
   test_mp3frame_c();



}
