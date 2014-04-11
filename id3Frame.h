#include "id3lib.h"
#ifndef ID3FRAME_H_INCLUDED
#define ID3FRAME_H_INCLUDED


struct ID3MainFrameInfo
{
    char Ver;    /*版本号ID3V2.3就记录3*/
    char Revision;    /*副版本号此版本记录为0*/
    char Flag;    /*存放标志的字节，这个版本只定义了三位，稍后详细解说*/
    size_t fsize;   /*标签大小，包括标签头的10个字节和所有的标签帧的大小*/
};
typedef struct ID3MainFrameInfo* PID3MainFrameInfo;


ID3VER ID3Judge(FILE *fp);
//returns ID3Ver1 or ID3Ver2_3 or ID3Ver2_4
PID3FrameNode ID3NextFrameV3(FILE* fp, PID3FrameNode *PreviousNode);
PID3MainFrameInfo ID3MainFrame(FILE *fp);


#endif
