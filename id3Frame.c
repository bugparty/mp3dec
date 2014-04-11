#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include "id3lib.h"
#include "id3Frame.h"
#define SAVEOFFSET(fp)     long offset;offset = ftell(fp)
#define LOADOFFSET(fp)     fseek(fp,offset,SEEK_SET)
#define MAXLINE 1000
/*id3 common struct*/
struct ID3MainFrame
{
    char Ver;    /*版本号ID3V2.3就记录3*/
    char Revision;    /*副版本号此版本记录为0*/
    char Flag;    /*存放标志的字节，这个版本只定义了三位，稍后详细解说*/
    char Size[4];    /*标签大小，包括标签头的10个字节和所有的标签帧的大小*/
};

/*offset  type  len   name
--------------------------------------------
0       char  3                   "TAG"
3       char  30    title
33      char  30    artist
63      char  30    album
93      char  4     year
97      char  30    comments
127     byte  1     genre
--------------------------------------------*/

const char* ID3_GenreTable[] =
{
    "Blues",     "Classic Rock", "Country",   "Dance",       "Disco",          "Funk",
    "Grunge",    "Hip-Hop"  , "Jazz",      "Metal",       "New Age",        "Oldies",
    "Other",     "Pop",          "R&B",       "Rap",         "Reggae",         "Rock",
    "Techno",    "Industrial",   "Alternative",  "Ska",         "Death Metal",    "Pranks",
    "Soundtrack", "Euro-Techno",  "Ambient",      "Trip-Hop",    "Vocal",          "Jazz+Funk",
    "Fusion",     "Trance",       "Classical",    "Instrumental",  "Acid",           "House",
    "Game",       "Sound Clip",   "Gospel",       "Noise",         "Alt. Rock",      "Bass",
    "Soul",       "Punk",         "Space",        "Meditative",    "Instrum. Pop",   "Instrum. Rock",
    "Ethnic",     "Gothic",       "Darkwave",     "Techno-Indust.", "Electronic",     "Pop-Folk",
    "Eurodance",  "Dream",        "Southern Rock", "Comedy",        "Cult",           "Gangsta",
    "Top 40",     "Christian Rap", "Pop/Funk",     "Jungle",        "Native American", "Cabaret",
    "New Wave",   "Psychadelic",  "Rave",         "Showtunes",     "Trailer",        "Lo-Fi",
    "Tribal",     "Acid Punk",    "Acid Jazz",    "Polka",         "Retro",          "Musical",
    "Rock & Roll", "Hard Rock",    "Folk",         "Folk/Rock",     "National Folk",  "Swing",
    "Fusion",     "Bebob",        "Latin",        "Revival",       "Celtic",         "Bluegrass",
    "Avantgarde", "Gothic Rock",  "Progress. Rock", "Psychadel. Rock", "Symphonic Rock", "Slow Rock",
    "Big Band",   "Chorus",       "Easy Listening", "Acoustic",    "Humour",          "Speech",
    "Chanson",    "Opera",        "Chamber Music", "Sonata",         "Symphony",    "Booty Bass",
    "Primus",     "Porn Groove",   "Satire",
//extended
    "Slow Jam",   "Club",        "Tango",     "Samba",       "Folklore",    "Ballad",
    "Power Ballad", "Rhythmic Soul", "Freestyle",   "Duet",       "Punk Rock",       "Drum Solo",
    "A Capella",  "Euro-House",   "Dance Hall",    "Goa",      "Drum & Bass",      "Club-House",
    "Hardcore",   "Terror",      "Indie",        "BritPop",  "Negerpunk",        "Polsk Punk",
    "Beat",      "Christian Gangsta Rap",  "Heavy Metal",   "Black Metal", "Crossover", "Contemporary Christian",
    "Christian Rock",
    /* winamp 1.91 genres*/
    "Merengue",   "Salsa",      "Thrash Metal",
    /* winamp 1.92 genres*/
    "Anime",      "Jpop",       "Synthpop", NULL
};
/* id3 v1*/
struct ID3_V1tag
{

    char title[30];
    char artist[30];
    char album[30];
    char year[4];
    char comments[30];
    unsigned char genre;
} idinfo;

struct id_sub_header
{
    char frameid[4];
    unsigned char size[4];
    char flag[2];
} ;
struct id3_V2_COMM_Header
{
    char encoding;
    char lang[3];
} HComm;




PID3FrameNode ID3NextFrameV3(FILE* fp,PID3FrameNode *prevNode)
{
    struct id_sub_header sub_header;
    PID3FrameNode Pnode;
    Pnode = (PID3FrameNode)malloc(sizeof(struct ID3FrameNode));
    if(NULL == *prevNode)
    {

        *prevNode = Pnode;
        Pnode->next = NULL;
        //seek to the first sub frame
        fseek(fp, 10, SEEK_SET);

        Pnode->fileOffset = ftell(fp);

        fread(&sub_header,1,10,fp);
        Pnode->flag[0] = sub_header.flag[0];
        Pnode->flag[1] = sub_header.flag[1];

        //printf("%i,%u,%i,%i",sub_header.size[3],sub_header.size[2]*0x100,sub_header.size[1]*0x10000,sub_header.size[0]*0x100000000);
        Pnode->fsize =  sub_header.size[0]*0x100000000
                     +sub_header.size[1]*0x10000
                     +sub_header.size[2]*0x100
                     +sub_header.size[3];
        //if size is char  a large bit will be positive
        //but why then unsigned long long fsize could be positive??

       Pnode->fourcc = MAKEFOURCC(sub_header.frameid[0], sub_header.frameid[1], sub_header.frameid[2], sub_header.frameid[3]);

    }
    else
    {
        Pnode->next = NULL;
        (**prevNode).next = Pnode;
        //skip previous frame content
        fseek(fp, (**prevNode).fsize, SEEK_CUR);
        Pnode->fileOffset = ftell(fp);

        fread(&sub_header,1,10,fp);
        Pnode->flag[0] = sub_header.flag[0];
        Pnode->flag[1] = sub_header.flag[1];

        //printf("%i,%u,%i,%i",sub_header.size[3],sub_header.size[2]*0x100,sub_header.size[1]*0x10000,sub_header.size[0]*0x100000000);
        Pnode->fsize =  sub_header.size[0]*0x100000000
                     +sub_header.size[1]*0x10000
                     +sub_header.size[2]*0x100
                     +sub_header.size[3];
        //if size is char  a large bit will be positive
        //but why then unsigned long long fsize could be positive??

       Pnode->fourcc = MAKEFOURCC(sub_header.frameid[0], sub_header.frameid[1], sub_header.frameid[2], sub_header.frameid[3]);
    }
    return Pnode;
}
PID3MainFrameInfo ID3MainFrame(FILE *fp)
{
    size_t fsize;
    struct ID3MainFrame header;
    PID3MainFrameInfo ID3MainInfo;
    ID3MainInfo = (PID3MainFrameInfo)malloc(sizeof(struct ID3MainFrameInfo));

    fseek(fp, 3, SEEK_SET);
    fread(&header,1,7,fp);
    fsize =   (header.Size[0]&0x7F)*0x200000+(header.Size[1]&0x7F)*0x400
                                      +(header.Size[2]&0x7F)*0x80
                                      +(header.Size[3]&0x7F);
    ID3MainInfo->Ver =      header.Ver;
    ID3MainInfo->Revision = header.Revision;
    ID3MainInfo->Flag  =    header.Flag;
    ID3MainInfo->fsize =    fsize;

    return ID3MainInfo;
}
ID3VER ID3Judge(FILE *fp)
{
    char buffer[1000];
    char vtag[4];
    ID3VER flag = 0;
    struct ID3MainFrame header;
    //save the offset
    fseek(fp,-128,SEEK_END);
    fread(&vtag,3,1,fp);

    vtag[3] = '\0';

    if ( strcmp(vtag,"TAG") == 0)
        flag |= ID3Ver1;

    fseek(fp,0,SEEK_SET);
    fread(&buffer,1,3,fp);
    buffer[3] = '\0';
    if( strcmp("ID3",buffer)== 0)
    {

        fread(&header,1,7,fp);

        if (header.Ver == 3 &&header.Revision == 0)
        {
            flag |= ID3Ver2_3;
        }

        if (header.Ver == 4 && header.Revision == 0)
        {
            flag |= ID3Ver2_4;
        }
    }

    return flag;

}

