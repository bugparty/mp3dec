#include <stdio.h>
#include "mp3frame.h"
#include "io.h"
#include <stdbool.h>
#include "ulits.h"
#include <string.h>


/*
 * intBitrateTable[intLSF][intLayer-1][intBitrateIndex]
 */
static const int BitrateTable[2][3][15] =
{
    {
        //MPEG 1
        //Layer I
        {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},
        //Layer II
        {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},
        //Layer III
        {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,}
    },
    {
        //MPEG 2.0/2.5
        //Layer I
        {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},
        //Layer II
        {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},
        //Layer III
        {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,}
    }
};

/*
 * SamplingRateTable[intVersionID][intSamplingFrequency]
 */
static const int SamplingRateTable[4][4] =
{
    {11025 , 12000 , 8000,0},	//MPEG Version 2.5
    {0,0,0,0,},				//reserved
    {22050, 24000, 16000 ,0,},	//MPEG Version 2 (ISO/IEC 13818-3)
    {44100, 48000, 32000,0}		//MPEG Version 1 (ISO/IEC 11172-3)
};
/*
 * ID: 2 bits
 * "00"  MPEG Version 2.5 (unofficial extension of MPEG 2);
 * "01"  reserved;
 * "10"  MPEG Version 2 (ISO/IEC 13818-3);
 * "11"  MPEG Version 1 (ISO/IEC 11172-3).
 */

static int intStandardMask = 0xffe00000;
//the varible  will be modified during reading frames


struct mp3HdrBuf
{
    BYTE a;
    BYTE b;
    BYTE c;
    BYTE d;
};

typedef struct AUDIO_HEADER * PAUDIO_HEADER;

static int protection_bit;

static int  intLSF;
static int  intFrameSize;
static int  intMainDataSlots;
static int  intSideInfoSize;

static int  intbitrate;
static int  intsamplerate;
static int  intlayer;
static int  intMode;
static int  intModeExtension;
static int  intID;

static bool boolMS_Stereo;
static bool boolIntensityStereo;

static int intFrameCounter = 0;
static bool boolSync;

static long longAllFrameSize;  	//帧长度总和(文件长度减去ID3 tag, APE tag 等长度)
static long longFrameOffset;    //第一帧的偏移量
static long longAllTrackFrames;	//帧数
static double floatFrameDuration;	//一帧时长(秒)

static bool boolVBR;
static BYTE * byteVBRToc;


char strBitRate[10];
char strDuration[20];
char strProgress[50];

int frame_check_sync(DWORD h)
{
    // the syncword
    //1111 1111 1111
    //added mepg2.5
    //1111 1111 111
    if( (h & intStandardMask) != intStandardMask
            || (((h >> 19) & 3) == 1)		// version ID:  01 - reserved
            || (((h >> 17) & 3) == 0)		// Layer index: 00 - reserved
            || (((h >> 12) & 0xf) == 0xf)
            || (((h >> 12) & 0xf) == 0)
            || (((h >> 10) & 3) == 3))
        return 0;
    else
        return 1;

}

size_t frame_search_header(size_t offset)
{
    size_t offnow = offset;
    DWORD h = 0;
    io_seek(offset);

    while(++offnow-offset < 64*1024)
    {
        h = h<<8;
        h |= io_read();
        if(frame_check_sync(h))
        {
            return offnow-4;
        }
    }
    puts("error, can`t find header");
    return 0;
}
DWORD frame_syncWord()
{
    int ioff = -4;
    DWORD h = 0;
    do
    {
        h = (h<<8) | io_read();
        ioff ++;

    }
    while (frame_check_sync(h) == false);

    if(ioff > 0)
        boolSync = false;

    return h;
}
PAUDIO_HEADER frame_praseHeader_(DWORD h)
{
    PAUDIO_HEADER audio_info;
    audio_info = (PAUDIO_HEADER) malloc(sizeof(AUDIO_HEADER));


    audio_info->ID =  (h >> 19) & 3;
    audio_info->layer =(h >> 17) & 3;
    audio_info->protection_bit = (h >> 16) & 0x1;
    audio_info->bit_rate_index = (h >> 12) & 0xF;
    audio_info->sampling_frequency = (h >> 10) & 3;
    audio_info->padding_bit = (h >> 9) & 0x1;
    audio_info->private_bit = (h >> 8) & 0x1;
    audio_info->mode  = (h >> 6) & 3;
    audio_info->mode_extension = (h >> 4) & 3;

    audio_info->copyright = (h>>3) & 0x01;
    audio_info->original = (h>>2)  & 0x01;
    audio_info->emphasis =  h& 0x03;
    return audio_info;
}
void frame_praseHeader(DWORD h)
{
    PAUDIO_HEADER hdr = frame_praseHeader_(h);
    intID = hdr->ID;
    intlayer = 4 - hdr->layer;
    boolMS_Stereo = hdr->mode == 1 && (hdr->mode_extension & 2) != 0;
    boolIntensityStereo = hdr->mode == 1 && (hdr->mode_extension & 0x1) != 0;
    intLSF = (hdr->ID == frame_MPEG1) ? 0 : 1;
    intbitrate = BitrateTable[intLSF][intlayer-1][hdr->bit_rate_index];
    intsamplerate = SamplingRateTable[hdr->ID][hdr->sampling_frequency];

    protection_bit = hdr->protection_bit;
    intMode = hdr->mode;
    intModeExtension = hdr->mode_extension;

    switch (intlayer)
    {
    case 1:
        intFrameSize  = BitrateTable[intLSF][0][hdr->bit_rate_index] * 12000;
        intFrameSize /= SamplingRateTable[hdr->ID][hdr->sampling_frequency];
        intFrameSize  = ((intFrameSize+hdr->padding_bit)<<2);
        break;
    case 2:
        intFrameSize  = BitrateTable[intLSF][1][hdr->bit_rate_index] * 144000;
        intFrameSize /= SamplingRateTable[hdr->ID][hdr->sampling_frequency];
        intFrameSize += hdr->padding_bit;
        break;
    case 3:
        intFrameSize  = BitrateTable[intLSF][2][hdr->bit_rate_index] * 144000;
        intFrameSize /= SamplingRateTable[hdr->ID][hdr->sampling_frequency]<<(intLSF);
        intFrameSize += hdr->padding_bit;

        //计算帧边信息长度
        if(hdr->ID == 1)  //version 1 MEPG! 0 MEPG2
            intSideInfoSize = (hdr->mode == 3) ? 17 : 32;
        else
            intSideInfoSize = (hdr->mode == 3) ? 9 : 17;

        break;
    default:
        break;
    }

    //计算主数据长度
    intMainDataSlots = intFrameSize - 4 - intSideInfoSize;
    if(hdr->protection_bit == 0)
        intMainDataSlots -= 2;
}
bool frame_syncFrame()
{
    if(frame_syncSearch() == false)
        return false;
    if( protection_bit == 0)
        frame_headerCRC();
    intFrameCounter++;
    return true;
}
bool frame_headerCRC()
{
    //todo: check crc
    io_read();
    io_read();
    return true;
}
int frame_get_layer()
{
    return intlayer;
}
int frame_get_LSF()
{
    return intLSF;
}
int frame_get_bit_rate()
{
    return intbitrate;
}
int frame_getID()
{
    return intID;
}
int frame_get_sample_rate()
{
    return intsamplerate;
}
/*
 * protection_bit: 1 bit
 * "1"  no CRC;
 * "0"  protected by 16 bit CRC following header.
 */
bool frame_is_protected()
{
    return protection_bit == 1 ? false: true;
}
int frame_getMainDataSlots()
{
    return intMainDataSlots;
}

int frame_getSideInfoSize()
{
    return intSideInfoSize;
}

int frame_getFrameSize()
{
    return intFrameSize;
}
int frame_getChannels()
{
    if(intMode == 3)
        return 1;
    return 2;
}

bool frame_isMSStereo()
{
    return boolMS_Stereo;
}

bool frame_isIStereo()
{
    return boolIntensityStereo;
}
/*
* 帧同步: 查找到帧同步字后与下一帧的intVersionID等比较,
确定是否找到有效的同步字.
* 怎样更简单、有效帧同步?
*/

bool frame_syncSearch()
{
    DWORD h, cur_mask = 0;
    bool bfind = false;
    size_t start_pos = io_offset();
    DWORD dwH;

    while(!bfind)
    {
        h = frame_syncWord();
        frame_praseHeader(h);
        //若intVersionID等帧的特征未改变,不用与下一帧的同步头比较.
        if(boolSync)
        {
            bfind = true;
            break;
        }
        //compare with next frame header
        cur_mask = 0xffe00000;		//syncword
        cur_mask |= h & 0x180000;	//intVersionID
        cur_mask |= h & 0x60000;	//intLayer
        cur_mask |= h & 0x60000;	//intSamplingFrequency
        //cur_mask |= h & 0xC0;		//intMode
        //intModeExtension 不是始终不变.

        if(io_dump(intFrameSize -4, &dwH, 0, 4) < 4)
            break;
        void_dwM2L(&dwH);

        bfind = (dwH & cur_mask) == cur_mask;

        if(io_offset() - start_pos > 0xffff)
        {

            puts("搜索 64K 未发现MP3帧后放弃。");
            break;
        }

    }
    if(!boolSync)
    {
        boolSync = true;
        if(bfind && intStandardMask == 0xffe00000)  //first frame
        {
            intStandardMask  = cur_mask;
            longAllFrameSize = io_length();
            longFrameOffset  = io_offset() - 4;
            longAllFrameSize-= longFrameOffset;
            frame_parseVBR();
            frame_getTrackFrames();
            frame_getDuration();
            frame_printHeaderInfo();
        }
    }
    return bfind;
}
long  frame_getTrackFrames()
{
    if(longAllTrackFrames == 0)
        longAllTrackFrames = longAllFrameSize / intFrameSize;
    return longAllTrackFrames;
}
/*
* 返回MP3文件时长(秒)
*/
double frame_getDuration()
{
    double duration;
    int m;
    floatFrameDuration = (double)1152 / (intsamplerate <<intLSF);
    duration = floatFrameDuration * longAllTrackFrames;
    m = duration / 60;
    sprintf(strDuration, "%2im:%2is", m, (int)(duration - m * 60 + 0.5));
    strcpy(strProgress, ">----------------------------------------");
    return duration;
}

bool frame_parseVBR()
{
    int i;
    DWORD xing_flags, xing_quality;;
    BYTE * b = (BYTE *)malloc(intFrameSize);
    BYTE * pointer;
    io_dump(0, b, 0, intFrameSize);

    if(intFrameSize < 124 + intSideInfoSize)
    {
        free(b);
        return false;
    }
    for(i=2; i < intSideInfoSize; i++)
        if( *(b+i))
        {
            free(b);
            return false;
        }
    // Xing or Info header means always VBR
    pointer = b + intSideInfoSize;
    if(((*pointer++ == 'X') && (*pointer++ == 'i') && (*pointer++ == 'n') && (*pointer++ == 'g'))
            ||(*(--pointer) == 'o' && *(--pointer) == 'f' && *(--pointer) == 'n' && *(--pointer) == 'i'))
    {
        boolVBR = true;
        longAllFrameSize -= intFrameSize;
        longFrameOffset += intFrameSize;

    }
    else
        return false;
    xing_flags = makeInt32(b, intSideInfoSize + 4);
    if( (xing_flags & 1) == 1)
    {
        //track frame
        longAllTrackFrames = makeInt32(b, intSideInfoSize + 8);
        if(longAllTrackFrames < 0)
            longAllTrackFrames = 0;
        printf("track frames: %i", longAllTrackFrames);
    }
    if((xing_flags & 0x2) != 0)  //track bytes
    {
        longAllFrameSize = makeInt32(b, intSideInfoSize + 12);
        printf("track bytes: %i", longAllFrameSize);

    }
    if((xing_flags & 0x4) != 0)   //TOC: intSideInfoSize + 16, 100 bytes
    {
        byteVBRToc = (BYTE*)malloc(100);
        memcpy(byteVBRToc, b+intSideInfoSize+16, 100);
        puts("TOC:true");
    }
    if((xing_flags & 0x8) != 0)   //VBR quality
    {
        xing_quality = makeInt32(b, intSideInfoSize+116);
        printf("quality %i", xing_quality);

    }
    if(0 == b[intSideInfoSize+100] )
    {
        free(b);
        return true;
    }
    char strVBREncoder[9];
    memcpy(strVBREncoder, b+intSideInfoSize+120,8);
    strVBREncoder[8] = '\0';
    int lame_vbr = b[intSideInfoSize +129] & 0xf;
    switch(lame_vbr)
    {
        //from rev1 proposal.... not sure if all goud in practice
    case 1:
    case 8:// CBR
        //strBitRate = "CBR";
        break;
    case 2:
    case 9://ABR
        strcpy(strBitRate, "CBR");
        break;
    default:  //00 = unkown is taken as VBR
        strcpy(strBitRate, "VBR");
    }

    free(b);
    return true;
}
//printf info

void frame_printHeaderInfo()
{
    char *sver[] = {"MPEG 2.5", "reserved", "MPEG 2.0", "MPEG 1.0"};
    char *mode_str[] = {"Stereo","Joint Stereo","Dual channel","Single channel(Mono)"};
    char *exmode_str[] = {"(I/S)","(M/S)","(I/S & M/S)"};
    if(!boolVBR)
        sprintf(strBitRate, "%d", intbitrate);

    printf("\r%s, Layer %i, %iHz,%s , %s, %s\n",
           sver[intID], intlayer, frame_get_sample_rate(), mode_str[intMode], exmode_str[intModeExtension], strDuration);
}
static int progress_index = 1;
void frame_printStatus()
{
    int i;
    double t = intFrameCounter * floatFrameDuration;
    int m = (int) (t / 60);
    double s = t - 60 *m;
    double percent;
    if(boolVBR)
        percent = (double) intFrameCounter / longAllTrackFrames * 100;
    else
        percent = (double) io_offset() / io_length() * 100;
    i = ((int)(percent + 0.5) <<2) / 10;
    if(i == progress_index)
    {
        strncpy(strProgress+i, "->", 2);
        progress_index++;
    }
    printf("%02i:%04.1f[%-41s] %4.1f\n", m, s, strProgress, percent);

}



