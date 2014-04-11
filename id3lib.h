#ifndef ID3LIB_H_INCLUDED
#define ID3LIB_H_INCLUDED
#include <stdio.h>
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;

typedef DWORD FOURCC;//Four Char code
typedef BYTE ID3VER;

typedef int ID3HDL;
typedef int ID3Status;

#define ID3Ver1         1
#define ID3Ver2_3       2
#define ID3Ver2_4       4

struct ID3FrameNode
{
    FOURCC fourcc;
    size_t fsize;
    char   flag[2];
    long   fileOffset;
    struct ID3FrameNode *next;
};
typedef struct ID3FrameNode* PID3FrameNode;
#define FrameNotFound 0
#define MAKEFOURCC(a, b, c, d) ( (DWORD)(BYTE)(a) | ((DWORD)(BYTE)b)<<8 | ((DWORD)(BYTE)c)<<16 | ((DWORD)(BYTE)d)<<24 )
ID3HDL ID3Open(char* filename);
PID3FrameNode ID3Seek(ID3HDL hdl, FOURCC fourcc);
long ID3TellEnd(ID3HDL hdl);
void ID3Close(ID3HDL hdl);

#endif // ID3LIB_H_INCLUDED
