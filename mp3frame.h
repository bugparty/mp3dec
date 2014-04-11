#include "mp3dec.h"
#define frame_MPEG1  3
#define frame_MPEG2  2
#define frame_MPEG25  0
#define MAX_FRAMESIZE  1732	//MPEG 1.0/2.0/2.5, Lay 1/2/3
bool frame_headerCRC();

bool frame_syncSearch();
DWORD frame_syncWord();
bool frame_syncFrame();
size_t frame_search_header(size_t offset);
int frame_check_sync(DWORD h);

long  frame_getTrackFrames();

int frame_getFrameSize();
int frame_getSideInfoSize();

double frame_getDuration();
int frame_getChannels();

int frame_getMainDataSlots();
bool frame_is_protected();
bool frame_isIStereo();
bool frame_isMSStereo();

int frame_get_LSF();
int frame_get_layer();
int frame_get_sample_rate();
int frame_get_bit_rate();

bool frame_parseVBR();
void frame_praseHeader(DWORD h);
PAUDIO_HEADER frame_praseHeader_(DWORD h);

void frame_printStatus();
void frame_printHeaderInfo();




