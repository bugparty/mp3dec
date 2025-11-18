#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED

#include<stdio.h>
#include "mp3dec.h"

//global varible from io.c
extern FILE * fin;

//Function declarations
int mp3syncword();
bool syncCheck(DWORD h);
void parseHeader(DWORD h);
DWORD makeDWORD(BYTE *buffer, int offset);
bool syncSearch();
bool syncFrame();
long getTrackFrames();
float getDuration();
bool parseVBR();
void printHeaderInfo();
void printState();

#endif // HEADER_H_INCLUDED
