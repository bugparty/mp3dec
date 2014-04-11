#include <stdio.h>
#include <malloc.h>
#include "id3Frame.h"
#define MAXID 100
struct ID3lib_handle
{
    int id;
    FILE * fp;
    ID3VER ver;
    PID3MainFrameInfo MainFrameInfo;
    PID3FrameNode FrameNodeHead;
};
struct ID3lib_handle ID3hdl[MAXID];
static int instance;

ID3HDL ID3Open(char* filename)
{
    FILE* fp;
    ID3VER id3ver;
    if( (fp = fopen(filename, "rb")) == NULL)
        return 0;
   /* if( (id3ver = id3Judge(fp))==0)
        return 0;

    */
    id3ver = ID3Judge(fp);
    if(!( (id3ver &ID3Ver2_3) | (id3ver & ID3Ver2_4) ))
        return 0;
    instance++;
    ID3hdl[instance].fp = fp;
    ID3hdl[instance].id          = instance;
    ID3hdl[instance].ver         = id3ver;
    ID3hdl[instance].MainFrameInfo = NULL;
    ID3hdl[instance].FrameNodeHead = NULL;

    ID3hdl[instance].MainFrameInfo = ID3MainFrame(ID3hdl[instance].fp);

    return (ID3HDL)instance;
}
long ID3TellEnd(ID3HDL hdl)
{
    return ID3hdl[hdl].MainFrameInfo->fsize+10;
}
PID3FrameNode ID3Seek(ID3HDL hdl, FOURCC fourcc)
{
    PID3FrameNode CurFrameNode, PrevFrameNode;
    //read main frame

    //read sub frames

    //id3v2.3frame
    if(NULL == ID3hdl[hdl].FrameNodeHead && 3 == ID3hdl[hdl].MainFrameInfo->Ver )
    {//if there is no frame node yet

        CurFrameNode = ID3NextFrameV3(ID3hdl[hdl].fp, &ID3hdl[hdl].FrameNodeHead);
         if( 0 != CurFrameNode && CurFrameNode->fourcc == fourcc)
        {
            return CurFrameNode;
        }

        do
        {
            PrevFrameNode = CurFrameNode;
            CurFrameNode = ID3NextFrameV3(ID3hdl[hdl].fp, &PrevFrameNode);


        }while(0 != CurFrameNode->fsize && CurFrameNode->fourcc != fourcc );
        if( 0 == CurFrameNode && CurFrameNode->fourcc != fourcc)
        {
            return NULL;
        }
        return CurFrameNode;
    }
    else
    {

        //or continue previous search

        //Set Root Node
        CurFrameNode = ID3hdl[hdl].FrameNodeHead;
        //search from nodes in memory first
        while(CurFrameNode->fourcc != fourcc && CurFrameNode->next != NULL)
        {
            CurFrameNode = CurFrameNode->next;
        }
        if(CurFrameNode->fourcc == fourcc)
            return CurFrameNode;
        else
        {//contine search in harddisk
                do
            {
                PrevFrameNode = CurFrameNode;
                CurFrameNode = ID3NextFrameV3(ID3hdl[hdl].fp, &PrevFrameNode);


            }while(0 != CurFrameNode->fsize && CurFrameNode->fourcc != fourcc );

            //if found
            if(CurFrameNode->fourcc == fourcc)
                return CurFrameNode;
            //not found
            return NULL;
        }
    }

}

void ID3Close(ID3HDL hdl)
{//problems has no time to solve ,the space can`t be reused temperately
    PID3FrameNode Current, Previous;
    fclose(ID3hdl[hdl].fp);

    free(ID3hdl[hdl].MainFrameInfo);

    Current = ID3hdl[hdl].FrameNodeHead;
    if(NULL == Current)
        return;
    do
    {
        Previous = Current;
        if(NULL != Current->next)
            Current  = Current->next;
        free(Previous);
    }while(Current != NULL);


}

