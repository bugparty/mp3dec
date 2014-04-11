#ifndef CRC_H
#define CRC_H
#include <stdbool.h>
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;

WORD GetCrc16(const BYTE* pData, size_t nLength);
DWORD GetCrc32(const BYTE* pData, size_t dwSize);
bool IsCrc16Good(const BYTE* pData, int nLength, WORD crcCode);

#endif
