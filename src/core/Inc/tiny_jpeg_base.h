#ifndef _JPEG_BASE_DELCARE
#define _JPEG_BASE_DELCARE

#include "bitio.h"

#define COMP_Y  0
#define COMP_Cb 1
#define COMP_Cr 2

#define DECODE_HANDLER(DECODE, COMP)  (((DecodeHandler *)&DECODE) + COMP)

#define FAST_FLOAT float
#define DCTSIZE 8
#define DCTSIZE2 (DCTSIZE*DCTSIZE)
#define DEQUANTIZE(coef,quantval)  (((FAST_FLOAT) (coef)) * (quantval))
// typedef unsigned char byte;
// typedef unsigned short word;
extern const unsigned char zigzag[64];

enum SEG_T {SOI=0xD8, EOI=0xD9, SOF0=0xC0, SOF1=0xC1, DHT=0xC4, SOS=0xDA, DQT=0xDB, DRI=0xDD, APP0=0xE0, APP1=0xE1, APP2=0xE2, COM=0xFE, _UNUSED=0x1ff};
typedef enum _enum_DHT_T {AC_DHT=0x01, DC_DHT=0x00} DHT_T;

typedef struct _app0_info_struct
{
    char format[5];
    byte main_ver;
    byte sub_ver;
    byte unit;
    short x_den;
    short y_den;
    byte thumb_x;
    byte thumb_y;
} AppInfo;

typedef struct _app2_struct 
{
    int len;
    byte * data;
} App2Info;

typedef struct _sof0_comp_struct {
    byte comId;
    byte sample;
    byte dqt_no;
} SOF0Comp;

typedef struct _sof0_info_struct {
    byte accur;
    short height;
    short width;
    byte comp;
    SOF0Comp * comps;    
} SOF0Info;

typedef struct _dqt_struct {
    byte info;
    float * table;
} DQTTable;

typedef struct _dqt_set_struct {
    byte n;
    DQTTable dqt[2];
} DQTSet;

typedef struct _huff_lookup_table_struct {
    int size;
    word * code;
    byte * len;
    byte * weight;
} HuffLookupTable;

typedef struct _dht_struct {
    byte info;
    byte bit_table[16];
    byte * value_table;
    HuffLookupTable huff_table;
} DHTInfo;

typedef struct _dht_set_struct {
    DHT_T type;
    int n;
    DHTInfo dht[2];
} DHTSet;

typedef struct _sos_comp_struct {
    byte id;
    byte HTid;
} SOSComp;

typedef struct _sos_compressed {
    byte start;
    byte end;
    byte choose;
} SOSCompress;

typedef struct _sos_struct {
    byte comps_n;
    SOSComp * comps;
    SOSCompress compress;
} SOSInfo;


typedef struct _jpeg_decode_handler_struct {
    byte * data;
    short prev_dc;
    byte * wp;
    int size;
    int height;
    int width;
    DHTInfo * ac_dht;
    DHTInfo * dc_dht;
    DQTTable * dqt;
} DecodeHandler;

typedef struct _jpeg_decode_struct {
    DecodeHandler Y;
    DecodeHandler Cb;
    DecodeHandler Cr;
} DecodeData;

// typedef struct {

// };

typedef struct _jpeg
{
    char * file_path;
    byte * file_buf;
    BITIO * bitio;
    AppInfo app0;
    App2Info app2;
    SOF0Info sof0;
    DQTSet dqt;
    DHTSet ac_dht;
    HuffLookupTable ac_huff_table;
    DHTSet dc_dht;
    HuffLookupTable dc_huff_table;
    SOSInfo sos;
    DecodeData decode;
    BITIO * output_file;
} JPEG;

typedef struct _jpeg_seg {
    byte start;
    enum SEG_T seg_type;
    void * seg_content;
} JPEG_SEG;

void jpeg_build_huff_table(DHTInfo *dht_info);
byte jpeg_find_huff_code(DHTInfo *dht_info, int len, word code);
void jpeg_build_quantization_table(float *qtable, byte * ref_table);
void jpeg_idct_basic(short * dct_img, float * qtable, byte * output_img);
void jpeg_allocate(DecodeHandler *handler, byte * src);

/* Migrate from tinyjpeg.c */
void tinyjpeg_idct_float (short * DCT, float *Q_table, byte *output_buf, int stride);

/* Debug functions */
void print_block(byte *block, int h, int w);
#endif