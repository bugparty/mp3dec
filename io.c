#include "io.h"
#include "id3lib.h"

static const int DEFAULT_BUFSIZE=2048;
//global varible
FILE * fin;
BYTE *byteInBuf;
long longStartPos =0;
long longEndPos = -1;
int intBufSize;
long longCurPos;
bool boolBufDirty;
long longFileEndPos;
int intBufUsedSize;
long longBufSizeMask;
long longFileSize;
long lFrameOffset;
//init varbiles and file buffer
void io_open(char * filename, int intBufSize)
{
    ID3HDL id3hdl;
    int size_v2;
    fin = fopen(filename, "rb");
    intBufSize = intBufSize;
    byteInBuf = (BYTE*)malloc(intBufSize);
    setvbuf(fin, (char*) byteInBuf, _IOFBF, intBufSize);

    fseek(fin, 0, SEEK_END);
    longFileSize = ftell(fin);
    longFileEndPos = longFileSize - 1;

    longBufSizeMask = ~((long) intBufSize - 1L);
    boolBufDirty = false;

    lFrameOffset = 0;	//��һ֡��ƫ����
    long lFrameSize = longFileSize ;


    //ID3 v2
    fseek(fin, 0, SEEK_SET);
    longCurPos = 0;
    //My id3lib calls
    id3hdl = ID3Open(filename);
    size_v2 = ID3TellEnd(id3hdl);
    ID3Close(id3hdl);

    if (size_v2 > 0) {
        lFrameOffset = size_v2;
        lFrameSize -= size_v2;
        //load id3 infomations to be done
        // Seek past ID3v2 tag to start of MP3 frames
        fseek(fin, size_v2, SEEK_SET);
        longCurPos = size_v2;
    }

    //ID3 v1
    //todo:id3 v1
}

int io_read_off(long pos)
{
    if (pos < longStartPos || pos > longEndPos) {
			fflush(fin);
			fseek(fin, pos, SEEK_CUR);

			if ((pos < longStartPos) || (pos > longEndPos)) {
				return -1;
			}
    }
    //why not pos + 1
    longCurPos = pos;

    return fgetc(fin);
}
size_t io_offset()
{
      return ftell(fin);
}
BYTE io_read()
{
    static int read_count = 0;
    long pos_before = ftell(fin);
    BYTE buffer;
    buffer = fgetc(fin);
    if(read_count < 10) {
        fprintf(stderr, "    io_read[%d]: pos_before=%ld, read=0x%02X, pos_after=%ld\n",
                read_count++, pos_before, buffer, ftell(fin));
    }
    return buffer;
}
int io_reads(BYTE* b, int off, int len)
{
    size_t readed;
    fseek(fin, off, SEEK_CUR);
    readed = fread(b, 1, len, fin);
    return readed;

}
int io_reads_dir(BYTE* b, int off, int len)
{
    size_t readed;
    fseek(fin, off, SEEK_SET);
    readed = fread(b, 1, len, fin);
    return readed;

}

//�ӵ�ǰλ�ø���,���ƶ��ļ�"ָ��"

int io_dump(int src_off, BYTE* b, int dst_off, int len)
{
    size_t ori_offset, readed;
    ori_offset = ftell(fin);
    fseek(fin, src_off, SEEK_CUR);
    readed = fread(b+dst_off, 1, len, fin);
    fseek(fin, ori_offset, SEEK_SET);
    return readed;
}
void io_seek(long pos)
{
    fseek(fin, pos, SEEK_SET);
}
void io_close()
{
    free(byteInBuf);
    fclose(fin);
}
long io_length()
{
    return longFileSize;
}
