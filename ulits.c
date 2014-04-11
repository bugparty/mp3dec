typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
char Hex2StrTable[][16] =
{
    "0000", "0001", "0010", "0011",
    "0100", "0101", "0110", "0111",
    "1000", "1001", "1010", "1011",
    "1100", "1101", "1110", "1111"
};
char * Byte2Str(BYTE b, char * buffer)
{
    //LBF
    strcpy(buffer, Hex2StrTable[b>>4]);
    strcpy(buffer+4, Hex2StrTable[b&0x0f]);
    return buffer;
}
char * Word2Str(WORD b, char * buffer)
{

    strcpy(buffer, Hex2StrTable[b>>12]);
    strcpy(buffer+4, Hex2StrTable[b>>8&0x0f]);
    strcpy(buffer+8, Hex2StrTable[b>>4&0x0f]);
    strcpy(buffer+12, Hex2StrTable[b&0x0f]);
    return buffer;
}
DWORD dwM2L(DWORD word)
{
    DWORD temp = 0;
    temp = (word &0xff) <<24 | (word &0xff000000) >>24;
    word = word&0x00ffff00 | temp;
    temp = (word &0xff00) <<8 | (word &0x00ff0000) >>8;
    word = word&0xff0000ff | temp;
    return word;
}
void void_dwM2L(DWORD * word)
{
    DWORD temp = 0;
    temp = (*word &0xff) <<24 | (*word &0xff000000) >>24;
    *word = *word&0x00ffff00 | temp;
    temp = (*word &0xff00) <<8 | (*word &0x00ff0000) >>8;
    *word = *word&0xff0000ff | temp;

}
DWORD makeInt32(BYTE* b,int off)
{
    DWORD h;
    b +=off;
    h = *b & 0xff;
    h <<= 8;
    h |= *(++b);
    h <<= 8;
    h |= *(++b);
    h <<= 8;
    h |= *(++b);
    return h;
}
