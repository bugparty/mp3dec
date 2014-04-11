#include "haffman.h"
#include "layer3.h"
#include "mp3dec.h"
static int rzero_index[2];
static int *is;	//[32 * 18 + 4];
extern PSideInfo si;
extern int intSfbIdxLong;
//many things to be done

/*
 * intMask: 暂存位流缓冲区不超过32比特数据,位流2级缓冲
 * intBitNum: intMask剩余的比特数
 * intPart2Remain: 哈夫曼编码的主数据剩余的比特数
 * intRegion[]: 大值区某一码表解码主数据的区域
 */
int intBitNum, intMask, intPart2Remain;
int intRegion[3];


struct HuffTab {
    int linbits;
    short *table;
    };
typedef struct HuffTab HuffTab;
typedef struct HuffTab * PHuffTab;

PHuffTab htBV[];
PHuffTab htCount1[];

void haffman_init()
{
    htBV[0] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[0]->linbits = 0;
    htBV[0]->table   = htbv0;
    htBV[1] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[1]->linbits = 0;
    htBV[1]->table   = htbv1;
    htBV[2] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[2]->linbits = 0;
    htBV[2]->table   = htbv2;
    htBV[3] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[3]->linbits = 0;
    htBV[3]->table   = htbv3;
    htBV[4] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[4]->linbits = 0;
    htBV[4]->table   = htbv0;
    htBV[5] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[5]->linbits = 0;
    htBV[5]->table   = htbv5;
    htBV[6] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[6]->linbits = 0;
    htBV[6]->table   = htbv6;
    htBV[7] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[7]->linbits = 0;
    htBV[7]->table   = htbv7;
    htBV[8] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[8]->linbits = 0;
    htBV[8]->table   = htbv8;
    htBV[9] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[9]->linbits = 0;
    htBV[9]->table   = htbv9;
    htBV[10] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[10]->linbits = 0;
    htBV[10]->table   = htbv11;
    htBV[11] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[11]->linbits = 0;
    htBV[11]->table   = htbv11;
    htBV[12] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[12]->linbits = 0;
    htBV[12]->table   = htbv12;
    htBV[13] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[13]->linbits = 0;
    htBV[13]->table   = htbv13;
    htBV[14] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[14]->linbits = 0;
    htBV[14]->table   = htbv0;
    htBV[15] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[15]->linbits = 0;
    htBV[15]->table   = htbv15;
    htBV[16] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[16]->linbits = 1;
    htBV[16]->table   = htbv0;
    htBV[17] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[17]->linbits = 2;
    htBV[17]->table   = htbv16;
    htBV[18] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[18]->linbits = 3;
    htBV[18]->table   = htbv16;
    htBV[19] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[19]->linbits = 4;
    htBV[19]->table   = htbv16;
    htBV[20] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[20]->linbits = 6;
    htBV[20]->table   = htbv16;
    htBV[21] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[21]->linbits = 8;
    htBV[21]->table   = htbv16;
    htBV[22] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[22]->linbits = 10;
    htBV[22]->table   = htbv16;
    htBV[23] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[23]->linbits = 13;
    htBV[23]->table   = htbv16;
    htBV[24] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[24]->linbits = 4;
    htBV[24]->table   = htbv0;
    htBV[25] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[25]->linbits = 5;
    htBV[25]->table   = htbv24;
    htBV[26] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[26]->linbits = 6;
    htBV[26]->table   = htbv24;
    htBV[27] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[0]->linbits = 7;
    htBV[0]->table   = htbv24;
    htBV[28] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[0]->linbits = 8;
    htBV[0]->table   = htbv24;
    htBV[29] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[0]->linbits = 9;
    htBV[0]->table   = htbv24;
    htBV[30] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[0]->linbits = 11;
    htBV[0]->table   = htbv24;
    htBV[31] = (PHuffTab) malloc(sizeof(HuffTab));
    htBV[0]->linbits = 13;
    htBV[0]->table   = htbv24;

    htCount1[0] = (PHuffTab) malloc(sizeof(HuffTab));
    htCount1[1] = (PHuffTab) malloc(sizeof(HuffTab));


}
void huffman_decoder(int ch, int gr)
{
    PGRinfo s = &si->ch[ch].gr[gr];
    int r1, r2;

    if( 0 != s->wnidow_switching_flag)
    {
        int v = frame_getID();
        if(frame_MPEG1 == v || frame_MPEG2 == v && s->block_type == 2)
        {
            s->region1Start = 36;
            s->region2Start = 576;

        }
        else
        {
            if( frame_MPEG25 == v)
                if(2 == s->block_type && 0 == s->mixed_block_flag)
                    s->region1Start = intSfbIdxLong[6];
                else
                    s->region1Start = intSfbIdxLong[8];
        }
    }
    else
    {
        r1 = s->regino1_count + 1;
        r2 = r1 + s->region1_count + 1;

        if( r2 > sizeof(intSfbIdxLong) -1)
        {
            r2 = sizeof(intSfbIdxLong) -1;
        }
        s->region1Start = intSfbIdxLong[r1];
        s->region2Start = intSfbIdxLong[r2];


    }

    rzero_index[ch] = haffman_decode(ch, s, is);

}
void haffman_refreshMask()
{
    while(intBitNum < 24)
    {
        intMask |= bitstream_get1Byte() << (24-intBitNum);
        intBitNum += 8;
        intPart2Remain -= 8;
    }
}

int haffman_decode(int ch, PGRInfo gri, int * intHaffValue)
{
    int i, x, y , iTmp, linbits, intIndex = 0;
    intBitNum = intMask = 0;
    intPart2Remain = gri->part2_3_length - gri->part2_bits;
    x = gri->region1Start;  //region1
    y = gri->region2Start;  //region2
    i = gri->big_values << 1;  //bv
    if(i > 574)
        i = 574;
    if(x < i) {
        intRegion[0] = x;
        if( y < i
           {
             intRegion[1] = y;
             intRegion[2] = i;
           }
           else
            intRegion[1] = intRegion[2] = i;
    }else
        intRegion[0] = intRegion[1] = intRegion[2] = i;
    //使位流缓冲区字节对齐
    intBitNum = 8 - bitstream_getBitPos();
    intMask = bitstream_getBits9(intBitNum);
    intMask <<= 32 - intBitNum;
    intPart2Remain -= intBitNum;

    //decode big value section
    for(i = 0; i < 3; i++) {
        htCur =
    }


    }
}
