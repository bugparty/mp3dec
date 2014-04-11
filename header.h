#include<stdio.h>
#include "mp3dec.h"
struct AUDIO_HEADER {
	int ID;
	int layer;
	int protection_bit;
	int bitrate_index;
	int sampling_frequency;
	int padding_bit;
	int private_bit;
	int mode;
	int mode_extension;
	int copyright;
	int original;
	int emphasis;
};
//global varible from io.c
extern FILE * fin;

//inner varibles

int frame_mp3syncword();
bool frame_syncCheck(DWORD h);
DWORD makeDWORD(BYTE *buffer, int offset);
bool frame_syncSearch();
bool frame_syncFrame();
