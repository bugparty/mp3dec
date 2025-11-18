#include "header.h"
#include "io.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAXSTRING 1000
/*
 * intBitrateTable[intLSF][intLayer-1][intBitrateIndex]
 */
const int intBitrateTable[2][3][15] = {
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
 * intSamplingRateTable[intVersionID][intSamplingFrequency]
 */
const int intSamplingRateTable[4][4] = {
    {11025 , 12000 , 8000,0},	//MPEG Version 2.5
    {0,0,0,0,},				//reserved
    {22050, 24000, 16000 ,0,},	//MPEG Version 2 (ISO/IEC 13818-3)
    {44100, 48000, 32000,0}		//MPEG Version 1 (ISO/IEC 11172-3)
};
const int MPEG1 = 3;
const int MPEG2 = 2;
const int MPEG25 = 0;
const int MAX_FRAMESIZE = 1732;	//MPEG 1.0/2.0/2.5, Lay 1/2/3

struct mp3FrameHeader
{
    char b;
    char c;
    char d;
} mp3frame;
const DWORD intSyncmask = 0xFFE00000;
	/*
	 * intVersionID: 2 bits
	 * "00"  MPEG Version 2.5 (unofficial extension of MPEG 2);
	 * "01"  reserved;
	 * "10"  MPEG Version 2 (ISO/IEC 13818-3);
	 * "11"  MPEG Version 1 (ISO/IEC 11172-3).
	 */
static int intVersionID;

	/*
	 * intLayer: 2 bits
	 * "11"	 Layer I
	 * "10"	 Layer II
	 * "01"	 Layer III
	 * "00"	 reserved
	 */
static int intLayer;

	/*
	 * intProtectionBit: 1 bit
	 * "1"  no CRC;
	 * "0"  protected by 16 bit CRC following header.
	 */
static int intProtectionBit;

	/*
	 * intBitrateIndex: 4 bits
	 */
static int intBitrateIndex;

	/*
	 * intSamplingFrequency: 2 bits
	 * '00'	 44.1kHz
	 * '01'	 48kHz
	 * '10'	 32kHz
	 * '11'  reserved
	 */
static int intSamplingFrequency;

static int intPaddingBit;

	/*
	 * intMode: 2 bits
	 * '00'  Stereo;
	 * '01'  Joint Stereo (Stereo);
	 * '10'  Dual channel (Two mono channels);
	 * '11'  Single channel (Mono).
	 */
static int intMode;

	/*
	 * intModeExtension: 2 bits
	 * 		 intensity_stereo	boolMS_Stereo
	 * '00'	 off				off
	 * '01'	 on					off
	 * '10'	 off				on
	 * '11'	 on					on
	 */
static int intModeExtension;

static bool boolSync;
static int intFrameSize;
static int intMainDataSlots;	//main_data length
static int intSideInfoSize;		//side_information length
static int intLSF;
static int intStandardMask = 0xffe00000;
static bool boolMS_Stereo, boolIntensityStereo;

// MP3 file frame info - forward declarations
static int intFrameCounter = 0;
static long longAllFrameSize;	//total frame size
static long longFrameOffset;	//first frame offset
static long longAllTrackFrames;	//frame count
static float floatFrameDuration;	//frame duration (seconds)
static char strDuration[MAXSTRING];
static char progress[50];

// VBR info stored in first frame
static bool boolVBR;
static BYTE* byteVBRToc;
static char strVBREncoder[MAXSTRING];
static char strBitRate[MAXSTRING];

// Forward function declarations
static void headerCRC();
static int progress_index = 1;

bool isMSStereo() {
		return boolMS_Stereo;
	}

bool isIStereo() {
		return boolIntensityStereo;
	}

int getBitrate() {
		return intBitrateTable[intLSF][intLayer-1][intBitrateIndex];
	}

int getBitrateIndex() {
		return intBitrateIndex;
	}

int getChannels() {
		if(intMode == 3)
			return 1;
		return 2;
	}

int getMode() {
		return intMode;
	}

int getModeExtension() {
		return intModeExtension;
	}

int getVersion() {
		return intVersionID;
	}

int getLayer() {
		return intLayer;
	}

int getSampleFrequency() {
		return intSamplingFrequency;
	}

int getFrequency() {
		return intSamplingRateTable[intVersionID][intSamplingFrequency];
	}

int getMainDataSlots() {
		return intMainDataSlots;
	}

int getSideInfoSize() {
		return intSideInfoSize;
	}

int getFrameSize() {
		return intFrameSize;
	}
int mp3syncword()
{
    DWORD h, read_byte;
    int ioff = -4;
    do{
        if(EOF == (read_byte = fgetc(fin)))
            return 0;
        ioff++;
        h = (h<<8) | read_byte;
    }while(syncCheck(h) == false);
    if(ioff > 0)
        boolSync = false;
    return h;
}
bool syncCheck(DWORD h)
{
    if( (h & intSyncmask) != intSyncmask
        || (((h >> 19) & 3) == 1)		// version ID:  01 - reserved
        || (((h >> 17) & 3) == 0)		// Layer index: 00 - reserved
        || (((h >> 12) & 0xf) == 0xf)
        || (((h >> 12) & 0xf) == 0)
        || (((h >> 10) & 3) == 3)) {
			return false;
		}
		return true;
}
DWORD makeDWORD(BYTE *buffer, int offset)
{
    DWORD h;
    h = buffer[offset] ;
    h <<= 8;
    h |= buffer[offset +1];
    h <<= 8;
    h |= buffer[offset +2];
    h <<= 8;
    h |= buffer[offset +3];

    return h;
}
void parseHeader(DWORD h) {
		intVersionID = (h >> 19) & 3;
		intLayer = 4 - ((h >> 17) & 3);  // Fixed: added parentheses
		intProtectionBit = (h >> 16) & 0x1;
		intBitrateIndex = (h >> 12) & 0xF;
		intSamplingFrequency = (h >> 10) & 3;
		intPaddingBit = (h >> 9) & 0x1;
		intMode = (h >> 6) & 3;
		intModeExtension = (h >> 4) & 3;

		boolMS_Stereo = intMode == 1 && (intModeExtension & 2) != 0;
		boolIntensityStereo = intMode == 1 && (intModeExtension & 0x1) != 0;
		intLSF = (intVersionID == MPEG1) ? 0 : 1;

		switch (intLayer) {
		case 1:
			intFrameSize  = intBitrateTable[intLSF][0][intBitrateIndex] * 12000;
			intFrameSize /= intSamplingRateTable[intVersionID][intSamplingFrequency];
			intFrameSize  = ((intFrameSize+intPaddingBit)<<2);
			break;
		case 2:
			intFrameSize  = intBitrateTable[intLSF][1][intBitrateIndex] * 144000;
			intFrameSize /= intSamplingRateTable[intVersionID][intSamplingFrequency];
			intFrameSize += intPaddingBit;
			break;
		case 3:
			intFrameSize  = intBitrateTable[intLSF][2][intBitrateIndex] * 144000;
			intFrameSize /= intSamplingRateTable[intVersionID][intSamplingFrequency]<<(intLSF);
			intFrameSize += intPaddingBit;

			if(intVersionID == MPEG1)
				intSideInfoSize = (intMode == 3) ? 17 : 32;
			else
				intSideInfoSize = (intMode == 3) ? 9 : 17;

			break;
		default:
			break;
		}

		intMainDataSlots = intFrameSize - 4 - intSideInfoSize;
		if(intProtectionBit == 0)
			intMainDataSlots -= 2;
}
bool syncSearch()
{
    DWORD h, cur_mask = 0;
    bool bfind = false;
    long start_pos = io_offset();
    while(!bfind) {
			h = mp3syncword();
			parseHeader(h);

			if(boolSync) {
				bfind = true;
				break;
			}

			//compare next frame header
			cur_mask = 0xffe00000;		//syncword
			cur_mask |= h & 0x180000;	//intVersionID
			cur_mask |= h & 0x60000;	//intLayer
			cur_mask |= h & 0xC00;		//intSamplingFrequency

			BYTE * b4 = (BYTE*)malloc(4);
			if(b4 == NULL) {
				break;
			}
			if(io_dump(intFrameSize-4, b4, 0, 4) < 4) {
				free(b4);
				break;
			}
			bfind = (makeDWORD(b4, 0) & cur_mask) == cur_mask;
			free(b4);

			if(io_offset() - start_pos > 0xffff) {
				printf("\nSearch 64K without finding valid MP3 frame\n");
				break;
			}
		}

		if(!boolSync) {
			boolSync = true;
			if(bfind && intStandardMask == 0xffe00000) {	//first frame:
				intStandardMask = cur_mask;
				longAllFrameSize = io_length();
				longFrameOffset = io_offset()-4;
				longAllFrameSize -= longFrameOffset;
				parseVBR();
				getTrackFrames();
				getDuration();
				printHeaderInfo();
			}
			printf("Begining of syncword: bytes %ld, frame_number =%d",
					(io_offset()-4), intFrameCounter);
		}
		return bfind;
}

bool syncFrame()
{
    if(syncSearch() == false)
			return false;
    if (intProtectionBit == 0)
        headerCRC();
    intFrameCounter++;
    return true;
}

static void headerCRC()
{
    //TODO: implement CRC check
    io_read();
    io_read();
}

	// -------------------------------------------------------------------
	// MP3 file frame info functions

long getTrackFrames() {
		if(longAllTrackFrames == 0)
			longAllTrackFrames = longAllFrameSize / intFrameSize;
		return longAllTrackFrames;
	}
	/*
	 * Calculate MP3 file duration (seconds)
	 */
float getDuration() {
		floatFrameDuration = (float)1152 / (intSamplingRateTable[intVersionID][intSamplingFrequency] << intLSF);
		float duration = floatFrameDuration * longAllTrackFrames;
		int m = (int)(duration / 60);
		sprintf(strDuration, "%02d:%02d", m, (int)(duration - m * 60 + 0.5));
		strcpy(progress, ">----------------------------------------");

		return duration;
}

// -------------------------------------------------------------------
// VBR parsing function

bool parseVBR() {
		int i;
		BYTE* b = (BYTE*)malloc(intFrameSize);
		if(b == NULL)
			return false;

		io_dump(0, b, 0, intFrameSize);
		if (intFrameSize < 124 + intSideInfoSize) {
			free(b);
			return false;
		}
		for (i = 2; i < intSideInfoSize; ++i)
			if (b[i] != 0) {
				free(b);
				return false;
			}

		// Xing or Info header means always VBR
		if (((b[intSideInfoSize] == 'X') && (b[intSideInfoSize + 1] == 'i')
				&& (b[intSideInfoSize + 2] == 'n') && (b[intSideInfoSize + 3] == 'g'))
				|| ((b[intSideInfoSize] == 'I') && (b[intSideInfoSize + 1] == 'n')
				&& (b[intSideInfoSize + 2] == 'f') && (b[intSideInfoSize + 3] == 'o'))) {
			boolVBR = true;
			longAllFrameSize -= intFrameSize;
			longFrameOffset += intFrameSize;
		} else {
			free(b);
			return false;
		}

		int xing_flags = makeDWORD(b, intSideInfoSize + 4);
		if ((xing_flags & 1) == 1) { // track frames
			longAllTrackFrames = makeDWORD(b, intSideInfoSize + 8);
			if (longAllTrackFrames < 0)
				longAllTrackFrames = 0;
			printf("track frames: %ld\n", longAllTrackFrames);
		}
		if ((xing_flags & 0x2) != 0) { // track bytes
			longAllFrameSize = makeDWORD(b, intSideInfoSize + 12);
			printf("track bytes: %ld\n", longAllFrameSize);
		}
		if ((xing_flags & 0x4) != 0) { // TOC: intSideInfoSize+16, 100 bytes.
			byteVBRToc = (BYTE*) malloc(100);
			if(byteVBRToc != NULL) {
				memcpy(byteVBRToc, b+intSideInfoSize+16, 100);  // Fixed: corrected order
				printf("TOC: true\n");
			}
		}
		if ((xing_flags & 0x8) != 0) { // VBR quality
			int xing_quality = makeDWORD(b, intSideInfoSize + 116);
			printf("quality: %d\n", xing_quality);
		}

		if (b[intSideInfoSize + 120] == 0) {
			free(b);
			return true;
		}
		strncpy(strVBREncoder, (char*)(b + intSideInfoSize + 120), 8);
		strVBREncoder[8] = '\0';
		printf("encoder: %s\n", strVBREncoder);

		int lame_vbr = b[intSideInfoSize + 129] & 0xf;
		switch (lame_vbr) {
		// from rev1 proposal... not sure if all good in practice
		case 1:
		case 8: // CBR
			strcpy(strBitRate, "CBR");
			break;
		case 2:
		case 9: // ABR
			strcpy(strBitRate,  "ABR");
			break;
		default: // 00==unknown is taken as VBR
			strcpy(strBitRate,"VBR");
		}

		free(b);
		return true;
	}

	// -------------------------------------------------------------------
	// Print info
void printHeaderInfo() {
		char *sver[] = {"MPEG 2.5", "reserved", "MPEG 2.0", "MPEG 1.0"};
		char *mode_str[] = {", Stereo",", Joint Stereo",", Dual channel",", Single channel(Mono)"};
		char *exmode_str[] = {"","(I/S)","(M/S)","(I/S & M/S)"};
		if(!boolVBR)
			sprintf(strBitRate, "%dK", intBitrateTable[intLSF][intLayer-1][intBitrateIndex]);
		printf("\r%s, Layer %d, %dHz, %s%s%s, %s\n",
			sver[intVersionID], intLayer,
			getFrequency(),
			strBitRate,
			mode_str[intMode],
			exmode_str[intModeExtension],
			strDuration);
	}

void printState() {
		float t = intFrameCounter * floatFrameDuration;
		int m = (int)(t / 60);
		float s = t - 60 * m;
		float percent;
		if(boolVBR)
			percent = (float)intFrameCounter / longAllTrackFrames * 100;
		else
			percent = (float)io_offset() / io_length() * 100;
		int i = ((int)(percent + 0.5) << 2) / 10;
		if(i == progress_index && i < 40) {
			progress[i] = '=';
			progress[i+1] = '>';
			progress_index++;
		}
		printf("\r%02d:%04.1f [%-41s] %.1f%%", m, s, progress, percent);
		fflush(stdout);
	}
