#include <math.h>
#include <stdbool.h>
#include "mp3frame.h"
#include "bitstream.h"
#include "layer3.h"
#include <stdio.h>
PSideInfo si;
int scfL[2][23];
int scfS[2][3][13];
int is[32* 18 + 4];
int xr[2][32][13];
int intWidthLong[22];
int intWidthShort[13];
float floatRawOut[36];
float PrevBlck[2][32][18];

static int intWhichChannel, intFirstChannel, intLastChannel;
static int intChannels;
static int intMaxGr;
static const int*  intSfbIdxLong;
static const int*  intSfbIdxShort;
static bool boolIntensityStereo;
static int intSfreq;

static int rzero_bandL;
static int rzero_bandS[3];

void layer3_init(int wch)
{
    int i;
    intWhichChannel = wch;
    intChannels = frame_getChannels();

    si = (PSideInfo)malloc(sizeof (struct SideInfo));
    bitstream_setbuf_size(36);
    if(2 == intChannels)
        switch(intWhichChannel)
        {
        case CH_LEFT:
            intFirstChannel = intLastChannel = 0;
            break;
        case CH_RIGHT:
            intFirstChannel = intLastChannel = 1;
            break;
        case CH_BOTH:
        default:
            intFirstChannel = 0;
            intLastChannel  = 1;
            break;
        }
    else
        intFirstChannel = intLastChannel = 0;
    //待解码文件的不同特征用到不同的变量.初始化:
    intSfreq =  frame_get_sample_rate();  //frequency
    intSfreq += (frame_getID() == frame_MPEG1) ? 0 : (frame_getID() == frame_MPEG2 ? 3 : 6);
    /*
    * ANNEX B,Table 3-B.8. Layer III scalefactor bands
    */
    switch(intSfreq)
    {
    case 0:
        intSfbIdxLong = intSfbIdxLong0;
        intSfbIdxShort = intSfbIdxShort0;
        break;
    case 1:
        intSfbIdxLong = intSfbIdxLong1;
        intSfbIdxShort = intSfbIdxShort1;
    case 2:
        intSfbIdxLong = intSfbIdxLong2;
        intSfbIdxShort = intSfbIdxShort2;
    case 3:
        intSfbIdxLong = intSfbIdxLong3;
        intSfbIdxShort = intSfbIdxShort3;
    case 4:
        intSfbIdxLong = intSfbIdxLong4;
        intSfbIdxShort = intSfbIdxShort4;
    case 5:
        intSfbIdxLong = intSfbIdxLong5;
        intSfbIdxShort = intSfbIdxShort5;
    case 6:
        intSfbIdxLong = intSfbIdxLong6;
        intSfbIdxShort = intSfbIdxShort6;
    case 7:
        intSfbIdxLong = intSfbIdxLong7;
        intSfbIdxShort = intSfbIdxShort7;
    case 8:
        intSfbIdxLong = intSfbIdxLong8;
        intSfbIdxShort = intSfbIdxShort8;
    }
    for(i=0 ; i < 22; i++)
        intWidthLong[i] = intSfbIdxLong[i+1] - intSfbIdxLong[i];
    for(i = 0; i< 13; i++)
        intWidthShort[i] = intSfbIdxShort[i+1] - intSfbIdxShort[i];
    //强度立体声
    boolIntensityStereo = frame_isIStereo();

}
bool layer3_getSideInfo(PSideInfo si)
{
    int ch, gr;
    bitstream_reset_index();
    bitstream_append(frame_getSideInfoSize());
    PGRInfo s;
    int *scfsi;

    //iso11172-3 2.4.1.7
    if(frame_get_layer() == frame_MPEG1)
    {
        si->main_data_begin = bitstream_getBits9(9); //uimsbf
        if( 1 == intChannels ) //if mode == single_channel
            bitstream_getBits9(5);  //private_bits
        else  //if mode == steroeo || dual_channel || ms_stereo
            bitstream_getBits9(3);  //private_bits

        for( ch = 0; ch < intChannels; ch++)
        {
            scfsi = (si->ch[ch].scfsi);
            scfsi[0] = bitstream_get1Bit(); //bslbf
            scfsi[1] = bitstream_get1Bit();
            scfsi[2] = bitstream_get1Bit();
            scfsi[3] = bitstream_get1Bit();
        }

        for (gr =0 ; gr < 2; gr++)
        {
            for (ch = 0; ch < intChannels; ch++)
            {
                s = &si->ch[ch].gr[gr];
                s->part2_3_length = bitstream_getBits17(12);
                s->big_values = bitstream_getBits9(9);
                s->global_gain = bitstream_getBits9(8);
                s->scalefac_compress = bitstream_getBits9(4);
                s->window_switching_flag = bitstream_get1Bit(); //blocksplit_flag[gr]
                if(0 != s->window_switching_flag)
                {
                    s->block_type = bitstream_getBits9(2);
                    s->mixed_block_flag = bitstream_get1Bit();//switch_point[gr]
                    s->table_select[0]  = bitstream_getBits9(5);
                    s->table_select[1]  = bitstream_getBits9(5);
                    s->subblock_gain[0] = bitstream_getBits9(3);
                    s->subblock_gain[1] = bitstream_getBits9(3);
                    s->subblock_gain[2] = bitstream_getBits9(3);
                    //the following was not metioned in the docment
                    if( 0 == s->block_type )
                        return false;
                    else if(2 == s->block_type && 0 == s->mixed_block_flag)
                        s->region0_count = 8;
                    else
                        s->region0_count = 7;

                    s->region1_count = 20 - s->region0_count;
                }
                else
                {
                    s->table_select[0] = bitstream_getBits9(5);
                    s->table_select[1] = bitstream_getBits9(5);
                    s->table_select[2] = bitstream_getBits9(5);
                    s->region0_count   = bitstream_getBits9(4);//region_address1[gr]
                    s->region1_count   = bitstream_getBits9(3);//region_address2[gr]
                    s->block_type      = 0;
                }
                s->preflag            = bitstream_get1Bit();
                s->scalefac_scale     = bitstream_get1Bit();
                s->count1table_select = bitstream_get1Bit();

            }
        }
    }
    else
    {
        //MPEG 2.0 2.5
        si->main_data_begin = bitstream_getBits9(8);
        if(1 == intChannels)
            bitstream_get1Bit();
        else
            bitstream_getBits9(2);
        for (ch = 0; ch < intChannels; ch++)
        {
            s = &si->ch[ch].gr[0];
            s->part2_3_length = bitstream_getBits17(12);
            s->big_values = bitstream_getBits9(9);
            s->global_gain = bitstream_getBits9(8);
            s->scalefac_compress = bitstream_getBits9(9);  //different than MEPG1
            s->window_switching_flag = bitstream_get1Bit(); //blocksplit_flag[gr]
            if(0 != s->window_switching_flag)
            {
                s->block_type = bitstream_getBits9(2);
                s->mixed_block_flag = bitstream_get1Bit();//switch_point[gr]
                s->table_select[0]  = bitstream_getBits9(5);
                s->table_select[1]  = bitstream_getBits9(5);
                s->subblock_gain[0] = bitstream_getBits9(3);
                s->subblock_gain[1] = bitstream_getBits9(3);
                s->subblock_gain[2] = bitstream_getBits9(3);
                if( 0 == s->block_type )
                    return false;
                else if(2 == s->block_type && 0 == s->mixed_block_flag)
                    s->region0_count = 8;
                else
                {
                    s->region0_count = 7;
                    s->region1_count = 20 - s->region0_count;//different than MEPG1
                }
            }
            else
            {
                s->table_select[0]  = bitstream_getBits9(5);
                s->table_select[1]  = bitstream_getBits9(5);
                s->table_select[2]  = bitstream_getBits9(5);
                s->region0_count    = bitstream_getBits9(4);
                s->region1_count    = bitstream_getBits9(3);
                s->block_type       = 0;
                s->mixed_block_flag = 0; //different from MEPG1
            }
            s->scalefac_scale = bitstream_get1Bit();
            s->count1table_select = bitstream_get1Bit();
        }

    }
    return true;
}
int slen0[] = { 0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4 };
int slen1[] = { 0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3 };
void layer3_getScaleFacors_1(int ch, int gr)
{
    PGRInfo gr_info = &si->ch[ch].gr[gr];
    int scale_comp = gr_info->scalefac_compress;
    int length0 = slen0[scale_comp];
    int length1 = slen1[scale_comp];
    int sfb, window;
    int *l;
    int **s;
    l = scfL[ch];
    s = scfS[ch];
    gr_info->part2_bits = 0;

    if(0 != gr_info->window_switching_flag && 2 == gr_info->block_type)
    {
        if(gr_info->mixed_block_flag)
        {
            //Mixed block
            gr_info->part2_bits = 17 * length0 + 18 * length1;
            for(sfb = 0; sfb <8 ; sfb++)
                l[sfb] = bitstream_getBits9(length0);
            for(sfb = 3; sfb < 6; sfb++)
                for(window = 0; window < 3; window++)
                    s[window][sfb] = bitstream_getBits9(length0);
            for(sfb = 6; sfb < 12; sfb++)
                for(window = 0; window < 3; window++)
                    s[window][sfb] = bitstream_getBits9(length1);
        }
        else
        {
            //pure ShORT block
            gr_info->part2_bits = 18 * (length0 + length1);
            for( sfb = 0; sfb < 6; sfb++)
                for(window = 0; window < 3; window++)
                    s[window][sfb] = bitstream_getBits9(length0);
            for(sfb = 6; sfb < 12; sfb++)
                for(window = 0; window < 3; window++)
                    s[window][sfb] = bitstream_getBits9(length1);
        }

    }
    else
    {
        //LONG types 0,1,3
        int * si_t = si->ch[ch].scfsi;
        if( 0 == gr)
        {
            gr_info->part2_bits = 10 * (length0 + length1) + length0;
            for(sfb = 0; sfb < 11; sfb++)
                l[sfb] = bitstream_getBits9(length0);
            for(sfb = 11; sfb < 21; sfb++)
                l[sfb] = bitstream_getBits9(length1);
        }
        else
        {
            gr_info->part2_bits = 0;
            if(0 == si_t[0])
            {
                for(sfb = 0; sfb < 6; sfb++)
                    l[sfb] = bitstream_getBits9(length1);
                gr_info->part2_bits += 6 * length0;
            }
            if(0 == si_t[1])
            {
                for(sfb = 6; sfb < 11; sfb++)
                    l[sfb] = bitstream_getBits9(length1);
                gr_info->part2_bits += 5 * length0;
            }
            if(0 == si_t[2])
            {
                for(sfb = 11; sfb < 16; sfb++)
                    l[sfb] = bitstream_getBits9(length1);
                gr_info->part2_bits += 5 * length0;
            }
            if(0 == si_t[3])
            {
                for(sfb = 16; sfb < 21; sfb++)
                    l[sfb] = bitstream_getBits9(length1);
                gr_info->part2_bits += 5 * length0;
            }


        }
    }
}


void layer3_getScaleFactors_2(int ch, int gr)
{
    BYTE *pnt;
    int i, j, k, slen, n=0, scf = 0;
    int *l, **s;
    bool i_stereo = frame_isIStereo();
    PGRInfo gr_info = &si->ch[ch].gr[gr];
    rzero_bandL = 0;
    l = scfL[ch];
    s = scfS[ch];
    if((ch > 0) && i_stereo)
        slen = i_slen2[gr_info->scalefac_compress >> 1];
    else
        slen = n_slen2[gr_info->scalefac_compress];
    gr_info->preflag = (slen >> 15) & 0x1;
    gr_info->part2_bits = 0;
    if( 2 == gr_info->block_type)
    {
        n++;
        if (0 !=(gr_info->mixed_block_flag))
            n++;
        pnt = slen_tab2[n][(slen >> 12) & 0x7];

        for(i = 0; i < 4; i++)
        {
            int num  = slen & 0x7;
            slen >>= 3;
            if( 0 != num)
            {
                for(j = 0; j , pnt[i] ; j += 3)
                {
                    for(k = 0; k < 3; k++)
                        s[k][scf] = bitstream_getBits17(num);
                    scf++;
                }
                gr_info->part2_bits += pnt[i] * num;

            }
            else
            {
                for(j = 0; j < pnt[i]; j +=3)
                {
                    for(k = 0; k < 3; k++)
                        s[k][scf] = 0;
                    scf++;

                }
            }

        }

        n = (n << 1) + 1;
        for( i= 0; i < n; i += 3)
        {
            for( k = 0; k < 3; k++)
                s[k][scf] = 0;
            scf++;
        }
    }
    else
    {
        pnt = slen_tab2[n][(slen >> 12) & 0x7];
        for(i = 0; i < 4; i++)
        {
            int num = slen & 0x7;
            slen >>= 3;
            if (num != 0)
            {
                for(j = 0; j < pnt[i]; j++)
                    l[scf++] = bitstream_getBits17(num);
                gr_info->part2_bits += pnt[i] * num;
            }
            else
            {
                for(j = 0; j < pnt[i]; j++)
                    l[scf++] = 0;
            }
        }
    }
}




