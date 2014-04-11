#ifndef MP3DEC_H_INCLUDED
#define MP3DEC_H_INCLUDED
#include <stdbool.h>
#include <stdio.h>
#define CH_BOTH   0
#define CH_LEFT   1
#define CH_RIGHT  2
struct AUDIO_HEADER {
	int ID;
	int layer;
	int protection_bit;
	int bit_rate_index;
	int sampling_frequency;
	int padding_bit;
	int private_bit;
	int mode;
	int mode_extension;
	int copyright;
	int original;
	int emphasis;
};
typedef struct AUDIO_HEADER * PAUDIO_HEADER;
typedef struct AUDIO_HEADER AUDIO_HEADER;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;

#endif // MP3DEC_H_INCLUDED

