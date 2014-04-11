#include "id3lib.h"
#ifndef ID3FRAME_H_INCLUDED
#define ID3FRAME_H_INCLUDED


struct ID3MainFrameInfo
{
    char Ver;    /*�汾��ID3V2.3�ͼ�¼3*/
    char Revision;    /*���汾�Ŵ˰汾��¼Ϊ0*/
    char Flag;    /*��ű�־���ֽڣ�����汾ֻ��������λ���Ժ���ϸ��˵*/
    size_t fsize;   /*��ǩ��С��������ǩͷ��10���ֽں����еı�ǩ֡�Ĵ�С*/
};
typedef struct ID3MainFrameInfo* PID3MainFrameInfo;


ID3VER ID3Judge(FILE *fp);
//returns ID3Ver1 or ID3Ver2_3 or ID3Ver2_4
PID3FrameNode ID3NextFrameV3(FILE* fp, PID3FrameNode *PreviousNode);
PID3MainFrameInfo ID3MainFrame(FILE *fp);


#endif
