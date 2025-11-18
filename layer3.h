#ifndef LAYER3_H_INCLUDED
#define LAYER3_H_INCLUDED

// Forward declarations for tables defined in tables.c (via tables.h)
// Note: DO NOT include tables.h here to avoid multiple definition errors
extern float floatPow2[];
extern float floatPowIS[];

// Data structures for Layer III decoding
struct GRInfo
{
    int part2_3_length;
    int big_values;
    int global_gain;
    int scalefac_compress;
    int window_switching_flag;
    int block_type;
    int mixed_block_flag;
    int table_select[3];
    int subblock_gain[3];
    int region0_count;
    int region1_count;
    int preflag;
    int scalefac_scale;
    int count1table_select;

    int region1Start;
    int region2Start;
    int part2_bits;   // scalefactor bits count

};
typedef struct GRInfo * PGRInfo;

struct Channel
{
    int scfsi[4];
    struct GRInfo gr[2];
};

struct SideInfo
{
    int main_data_begin;
    struct Channel ch[2];
};

typedef struct SideInfo * PSideInfo;

// Antialias coefficients (defined in layer3_tables.c)
extern float cs[8];
extern float ca[8];

// Intensity Stereo coefficients (defined in layer3_tables.c)
// MPEG 1.0
extern float is_coef[7];

// MPEG 2.0/2.5
extern float lsf_is_coef[2][15];

/*
 * ANNEX B, Table 3-B.8. Layer III scalefactor bands
 * Defined in layer3_tables.c
 */

/* MPEG 1, sampling_frequency=0, 44.1kHz */
extern const int intSfbIdxLong0[23];
extern const int intSfbIdxShort0[14];

/* MPEG 1, sampling_frequency=1, 48kHz */
extern const int intSfbIdxLong1[23];
extern const int intSfbIdxShort1[14];

/* MPEG 1, sampling_frequency=2, 32kHz */
extern const int intSfbIdxLong2[23];
extern const int intSfbIdxShort2[14];

/* MPEG 2, sampling_frequency=0, 22.05kHz */
extern const int intSfbIdxLong3[23];
extern const int intSfbIdxShort3[14];

/* MPEG 2, sampling_frequency=1, 24kHz */
extern const int intSfbIdxLong4[23];
extern const int intSfbIdxShort4[14];

/* MPEG 2, sampling_frequency=2, 16kHz */
extern const int intSfbIdxLong5[23];
extern const int intSfbIdxShort5[14];

/* MPEG 2.5, sampling_frequency=0, 11.025kHz */
extern const int intSfbIdxLong6[23];
extern const int intSfbIdxShort6[14];

/* MPEG 2.5, sampling_frequency=1, 12kHz */
extern const int intSfbIdxLong7[23];
extern const int intSfbIdxShort7[14];

/* MPEG 2.5, sampling_frequency=2, 8kHz */
extern const int intSfbIdxLong8[23];
extern const int intSfbIdxShort8[14];

// MPEG 2.0 scale factor length tables (defined in layer3_tables.c)
extern int i_slen2[256];
extern int n_slen2[512];
extern int slen_tab2[3][6][4];

// Current scale factor band index pointers (set by layer3_init() in layer3.c)
extern const int* intSfbIdxLong;
extern const int* intSfbIdxShort;

#endif // LAYER3_H_INCLUDED
