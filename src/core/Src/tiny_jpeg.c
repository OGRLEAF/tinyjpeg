#define _GNU_SOURCE
#include<string.h>
#include<stdio.h>
#include "tiny_jpeg.h"
#include "utils.h"
#include "trace.h"

typedef enum READ_S {INIT, FIND_SEG_START, READ_SEG_LENGTH, READ_SEG_CONT, COMMIT_SEG_INFO, READ_BODY, END} READ_S; 

void hello_tiny_jpeg() {
    printf("hello tiny jpeg\r\n");
}


JPEG * read_jpeg(const char * file_path) {
    JPEG *jpeg_file = malloc(sizeof(JPEG));
    FILE * fp;
    BITIO * bitio;

    ASSERT_EQUAL(sizeof(word), 2, "Word size");

    fp = fopen(file_path, "rb");
    bitio = create_file_bitio(fp);
    jpeg_file->bitio = bitio;
    jpeg_file->ac_dht.n = 0;
    jpeg_file->dc_dht.n = 0;
    jpeg_file->dqt.n = 0;
    jpeg_file->decode.Y.prev_dc = 0;
    jpeg_file->decode.Cb.prev_dc = 0;
    jpeg_file->decode.Cr.prev_dc = 0;
    //TRACE_DEBUG("DHT ac_n %d dc_n %d", jpeg_file->ac_dht.n, jpeg_file->dc_dht.n);
    ASSERT_NOT_NULL(fp, "File is null.");

    int file_path_len = strlen(file_path);
    asprintf(&jpeg_file->file_path, "%s", file_path);

    return jpeg_file;
}

void print_jpeg_breif(JPEG * jpeg_file) {
    //TRACE_INFO("Read jpeg file %s.", jpeg_file->file_path);
}


void short_reverse(short *t){
    byte * ts = (byte *) t;
    ts[0] = ts[0] ^ ts[1];
    ts[1] = ts[0] ^ ts[1];
    ts[0] = ts[0] ^ ts[1];
}

void read_app0(JPEG * jpeg, int len, byte * info){
    AppInfo * app_info = &jpeg->app0;
    memcpy(app_info, info, len);
    short_reverse(&(app_info->x_den));
    short_reverse(&(app_info->y_den));
}

void read_app2(JPEG * jpeg, int len, byte * info)
{
    jpeg->app2.len = len;
    jpeg->app2.data = (byte *) malloc(len);
    memcpy(jpeg->app2.data, info, len);
}

void read_sof0(JPEG * jpeg, int len, byte * info) {
    SOF0Info * sof_info = &jpeg->sof0;
    SOF0Comp sof_comp;
    sof_info->accur = *info;
    info += 1;
    sof_info->height = *((short *) (info));
    info += 2;
    sof_info->width = *((short *) (info));
    info += 2;
    sof_info->comp = *info;
    info++;
    short_reverse(&(sof_info->height));
    short_reverse(&(sof_info->width));
    // printf("\t\t- Accur: %d\n", sof_info->accur);
    // printf("\t\t- Height: %d, Width: %d\n", sof_info->height, sof_info->width);
    // printf("\t\t- Comps: %d\n", sof_info->comp);

    sof_info->comps = (SOF0Comp *) malloc(sizeof(SOF0Comp) * sof_info->comp);
    memcpy(sof_info->comps, info, sizeof(SOF0Comp) * sof_info->comp);
    // for(int i=0;i<sof_info->comp;i++){
    //     SOF0Comp * sof_comp = sof_info->comps + i;
    //     short_reverse(&sof_comp->sample);
    //     //printf("\t\t- Comp Id: %d, sample: %d, dqt: %d\n", sof_comp->comId, sof_comp->sample, sof_comp->dqt_no);
    // }
}

void read_dht(JPEG * jpeg, int len, byte *info){
    DHTInfo dht_info;
    dht_info.info = info[0];
    info++;
    memcpy(dht_info.bit_table, info, 16);
    info += 16;

    // printf("\t\t- DHT#%d, %cC\n", dht_info.info>>4, dht_info.info&1?'A':'D');
    // printf("\t\t- BitTable: ");

    DHT_T ac_dc = (dht_info.info>>4)&1?AC_DHT:DC_DHT;
    int table_n = dht_info.info & 0x0f;
    int sum = 0;
    for(int i=0;i<16;i++){
        byte t = dht_info.bit_table[i];
        //printf("%02x ", t);
        sum += t;
    };
    //printf(" (sum: %d)\n", sum);

    ASSERT(sum<256, "DHT bit table sum error.");
    //printf("\t\t- ValueTable: ");
    dht_info.value_table = (byte *) malloc(sum);
    memcpy(dht_info.value_table, info, sum);
    
    jpeg_build_huff_table(&dht_info);
    
    DHTSet * dht_target = ac_dc==AC_DHT?&jpeg->ac_dht:&jpeg->dc_dht;
    ASSERT(dht_target->n<2, "DHT more than 2");
    memcpy(&dht_target->dht[table_n], &dht_info, sizeof(dht_info));
    dht_target->n += 1;
    //TRACE_DEBUG("DHT update: t %d ac_n %d dc_n %d", ac_dc, jpeg->ac_dht.n, jpeg->dc_dht.n);
}

void read_sos(JPEG *jpeg, int len, byte * info) {
    int comps_n = info[0];
    SOSInfo sos_info ;
    sos_info.comps = (SOSComp *) malloc(sizeof(SOSComp) * comps_n);
    sos_info.comps_n = comps_n;
    info += 1;
    memcpy(sos_info.comps, info, sizeof(SOSComp) * comps_n);
    info += sizeof(SOSComp) * comps_n;
    memcpy(&sos_info.compress, info, sizeof(SOSCompress));
    ASSERT_EQUAL(sos_info.compress.start, 0x00, "Imag start != 0x00");
    ASSERT_EQUAL(sos_info.compress.end, 0x3f, "Imag end != 0x3f");
    ASSERT_EQUAL(sos_info.compress.choose, 0x00, "Imag chose != 0x00");
    // printf("\t\t- Comps: %d\n", comps_n);
    // for(int i=0;i<comps_n;i++) {
    //     SOSComp * comp = sos_info.comps + i;
    //     byte a = comp->HTid>>4;
    //     byte b = comp->HTid & 0x0f;
    //     printf("\t\t- CompId:%d\tAC:%d DC:%d\n", comp->id, a, b);
    // }
    memcpy(&jpeg->sos, &sos_info, sizeof(sos_info));

}

void read_dqt(JPEG *jpeg, int len, byte * info) {
    DQTTable dqt;
    byte qt_info = info[0];
    dqt.info = qt_info;
    byte qt_prec = qt_info>>4;
    byte qt_id = qt_info & 0x0f;
    int qt_size = 64 * (qt_prec?2: 1);

    ASSERT(qt_id<3, "QT_ID over than 3");
    ASSERT_EQUAL(len, 1 + qt_size, "QT Size assumption");


    dqt.table = (float *) malloc(qt_size * sizeof(float));
    jpeg_build_quantization_table(dqt.table, info + 1);
    
    ASSERT(jpeg->dqt.n<2, "DQT more than 2");

    memcpy(&jpeg->dqt.dqt[qt_id], &dqt, sizeof(DQTTable));
    jpeg->dqt.n += 1;
}

void read_body(JPEG *jpeg, int len, byte *buf)
{

}

int print_jpeg(JPEG *jpeg){
    BITIO * bitio = jpeg->bitio;
    byte next_char;
    int read_state;
    READ_S s = INIT;
    short seg_len = 0;

    enum SEG_T seg_t;
    while(1) {
        if(s==END) {
            break;
        }
        next_char = read_byte(bitio);
        if(next_char==0xff) {
            //printf("Read 0xff, skip to next byte\r\n");
            s  = FIND_SEG_START;
            //TRACE_INFO("SEG \tLABEL: \t(0x%02x@0x%x)", next_char, bitio->offset);
            continue;
        }
        if(s==INIT){
            if(next_char==0xff) {
                //TRACE_INFO("%s", "Read next sector..");
                s  = FIND_SEG_START;
            }
        }else if(s==FIND_SEG_START) {
            seg_t =  next_char;
            switch (seg_t)
            {
                case SOI : TRACE_DEBUG("SOI: Start of Image"); break;
                case APP0: TRACE_DEBUG("APP0 Application specific"); break;
                case APP1: TRACE_DEBUG("APP1 Application specific"); break;
                case APP2: TRACE_DEBUG("APP2 Application specific"); break;
                case DQT : TRACE_DEBUG("DQT Define Quantization Table"); break;
                case SOF0: TRACE_DEBUG("SOF0 Start of Frame"); break;
                case DHT : TRACE_DEBUG("DHT Define Huffman Tables"); break;
                case SOS : TRACE_DEBUG("SOS Start Of Scan"); break;
                case _UNUSED: break;
                default:
                    s = END;
                    TRACE_DEBUG("UNKOWN: %02x", next_char);
                    continue;
                    break;
            }
            s = READ_SEG_LENGTH;
        }
        else if(s==READ_SEG_LENGTH){
            bitio_seek(bitio, -1, SEEK_CUR);
            byte *_seg_len = (byte *) malloc(sizeof(byte)*2);
            //fread(_seg_len, 1, 2, fp);
            read_bytes(bitio, _seg_len, 1, 2);
            short_reverse((short *) _seg_len);
            seg_len = *((short *) _seg_len) - 2;
            free(_seg_len);
            TRACE_DEBUG("\tLEN: \t(%d)%04x", seg_len, seg_len);
            s = READ_SEG_CONT;
        }
        else if(s==READ_SEG_CONT) {
            bitio_seek(bitio, -1, SEEK_CUR);
            byte * info = (byte *) malloc(sizeof(byte)*seg_len);
            TRACE_INFO("position %X", bitio->offset);
            read_bytes(bitio, info, 1, seg_len);
            if(seg_t==APP0){
                read_app0(jpeg, seg_len, info);
            }else if(seg_t==APP2) {
                read_app2(jpeg, seg_len, info);
            }else if(seg_t==SOF0) {
                read_sof0(jpeg, seg_len, info);
            }else if(seg_t==DHT){
                read_dht(jpeg, seg_len, info);
            } else if(seg_t==SOS) {
                TRACE_INFO("position %X", bitio->offset);
                read_sos(jpeg, seg_len, info);
                // s = READ_BODY;
                // continue;
                break;
            } else if(seg_t==DQT) {
                read_dqt(jpeg, seg_len, info);
            }
            //printf("\n");
            free(info);
            s = INIT;
        } else if(s==READ_BODY)
        {
            ASSERT(0, "start read body.");
        } else {
            TRACE_DEBUG("Missing");
        }
        
    }
    //printf("\r\n");
    return 0;
}

byte huffman_data_read(JPEG *jpeg, DHT_T type, int index){
    BITIO *bitio = jpeg->bitio;
    DHTInfo *dht = (type==DC_DHT?jpeg->dc_dht:jpeg->ac_dht).dht + index;
    word b = read_bit(bitio)<<1;
    byte weight;
    int j = 0;
    //TRACE_DEBUG("file_offset=0x%x bit_offset=%d next_char=%.2x", bitio->offset, bitio->offset_bit, bitio->next_byte);
    for (j = 2; j <= 16; j++)
    {
        word k = read_bit(bitio);
        b =  b + k;
        
        //TRACE_DEBUG("b="BYTE_TO_BINARY_PATTERN"_"BYTE_TO_BINARY_PATTERN"(%x) len=%d bit=%d", BIN(b>>8), BIN(b), b, j, k);
        weight = jpeg_find_huff_code(dht, j, b);
        //TRACE_DEBUG("b="BYTE_TO_BINARY_PATTERN"_"BYTE_TO_BINARY_PATTERN"(%d) word=%d weight=%d", BIN(b>>8), BIN(b), j, b, weight);
        b = b<<1;
        if (weight != 0xff) break;
    }
    ASSERT((weight!=0xff), "Huffman code not found!", TRACE_CONTENT("Test "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(b)));
    //TRACE_DEBUG("b="BYTE_TO_BINARY_PATTERN"_"BYTE_TO_BINARY_PATTERN"(%d) word=%d weight=%d", BIN(b>>9), BIN(b>>1), j, b, weight);
    return weight;
}


void huffman_data_unit_test(JPEG * jpeg, int comp_id)
{
    BITIO * bitio = jpeg->bitio;
    int dc_dht_idx = jpeg->sos.comps[comp_id].HTid & 0x0f;
    int ac_dht_idx = jpeg->sos.comps[comp_id].HTid >> 4;
    int dqt_idx = jpeg->sof0.comps[comp_id].dqt_no;
    DecodeHandler * handler = DECODE_HANDLER(jpeg->decode, comp_id);

    TRACE_DEBUG("Use ac_dht_id=%d, dc_dht_id=%d, dqt_id=%d", ac_dht_idx, dc_dht_idx, dqt_idx);
    short int DCT_r[64];
    memset(DCT_r, 0, sizeof(DCT_r));

    byte weight = huffman_data_read(jpeg, DC_DHT, dc_dht_idx);
    ASSERT(weight<=16, "Weight value error");
    DCT_r[0] = read_bits_signed(bitio, weight);
    DCT_r[0] += handler->prev_dc;
    handler->prev_dc = DCT_r[0];

    int j=1;
    while(j<64) {
        weight = huffman_data_read(jpeg, AC_DHT, ac_dht_idx);
        byte size_val = weight & 0x0f;
        byte count_0 = weight >> 4;
        //TRACE_DEBUG("(%d) weight=%.2x size_val=%d count_0=%d", j+1, weight, size_val, count_0);
        if(size_val == 0) {
            if(count_0 == 0) {
                //TRACE_DEBUG("EOB found");
                break;
            } else if(count_0 == 0x0f) {
                j += 16; // skip 16 zeros
            }
        } else {
            j += count_0;
            ASSERT(j<64, "Bad huffman data (buffer overflow)");

            short s = read_bits_signed(bitio, size_val);
            DCT_r[j] = s;
            //TRACE_DEBUG("DCT[%d]=%d", j, s);
            j++;
        }
    }

    short * DCT = (short * ) malloc(sizeof(short) * 64);
    for(int i=0;i<64;i++) {
        DCT[i] = DCT_r[zigzag[i]];
        //TRACE_DEBUG("Huff_code=%d", DCT[i]);
    }
    byte * idct_output = malloc(64);
    //jpeg_idct_basic(DCT, jpeg->dqt.dqt[dqt_idx].table, idct_output);
    tinyjpeg_idct_float(DCT, jpeg->dqt.dqt[dqt_idx].table, idct_output, 8);
    jpeg_allocate(handler, idct_output);
    if(comp_id==COMP_Y) PRINT_BLOCK(DCT_r, 8, 8, "%.4x");
    free(DCT);
}

// void jpeg_decode_mcu_huffman(JPEG *jpeg, int dc_dht_id, int ac_dht_id, short * DCT)
// {
//     BITIO * bitio = jpeg->bitio;
//     byte weight = huffman_data_read(jpeg, DC_DHT, dc_dht_idx);
//     ASSERT(weight<=16, "Weight value error");
//     DCT_r[0] = read_bits_signed(bitio, weight);
//     DCT_r[0] += handler->prev_dc;
//     handler->prev_dc = DCT_r[0];

//     int j=1;
//     while(j<64) {
//         weight = huffman_data_read(jpeg, AC_DHT, ac_dht_idx);
//         byte size_val = weight & 0x0f;
//         byte count_0 = weight >> 4;
//         //TRACE_DEBUG("(%d) weight=%.2x size_val=%d count_0=%d", j+1, weight, size_val, count_0);
//         if(size_val == 0) {
//             if(count_0 == 0) {
//                 //TRACE_DEBUG("EOB found");
//                 break;
//             } else if(count_0 == 0x0f) {
//                 j += 16; // skip 16 zeros
//             }
//         } else {
//             j += count_0;
//             ASSERT(j<64, "Bad huffman data (buffer overflow)");

//             short s = read_bits_signed(bitio, size_val);
//             DCT_r[j] = s;
//             //TRACE_DEBUG("DCT[%d]=%d", j, s);
//             j++;
//         }
//     }

//     short * DCT = (short * ) malloc(sizeof(short) * 64);
//     for(int i=0;i<64;i++) {
//         DCT[i] = DCT_r[zigzag[i]];
//         //TRACE_DEBUG("Huff_code=%d", DCT[i]);
//     }
// }

// void jpeg_decode_mcu_channel(JPEG *jpeg, int comp_id)
// {
//     BITIO * bitio = jpeg->bitio;
//     int dc_dht_idx = jpeg->sos.comps[comp_id].HTid & 0x0f;
//     int ac_dht_idx = jpeg->sos.comps[comp_id].HTid >> 4;
//     int dqt_idx = jpeg->sof0.comps[comp_id].dqt_no;
//     DecodeHandler * handler = DECODE_HANDLER(jpeg->decode, comp_id);

//     TRACE_DEBUG("Use ac_dht_id=%d, dc_dht_id=%d, dqt_id=%d", ac_dht_idx, dc_dht_idx, dqt_idx);
//     short int DCT_r[64];
//     memset(DCT_r, 0, sizeof(DCT_r));

//     byte * idct_output = malloc(64);
//     //jpeg_idct_basic(DCT, jpeg->dqt.dqt[dqt_idx].table, idct_output);
//     tinyjpeg_idct_float(DCT, jpeg->dqt.dqt[dqt_idx].table, idct_output, 8);
//     jpeg_allocate(handler, idct_output);
//     if(comp_id==COMP_Y) PRINT_BLOCK(DCT_r, 8, 8, "%.4x");
//     free(DCT);
// }

// void jpeg_decode_mcu(JPEG *jpeg) {

// }

void jpeg_init_decode_handler(JPEG * jpeg, int comp_id) {
    int full_size = jpeg->sof0.width * jpeg->sof0.height;
    int dc_dht_idx = jpeg->sos.comps[comp_id].HTid & 0x0f;
    int ac_dht_idx = jpeg->sos.comps[comp_id].HTid >> 4;
    int dqt_idx = jpeg->sof0.comps[comp_id].dqt_no;

    DecodeHandler * handler = (DecodeHandler *) (&jpeg->decode) + comp_id;
    handler->data = (byte *) malloc(full_size);
    handler->width = jpeg->sof0.width;
    handler->height = jpeg->sof0.height;
    handler->size = full_size;
    handler->wp = handler->data;
    handler->ac_dht = &jpeg->ac_dht.dht[comp_id];
    handler->dc_dht = &jpeg->dc_dht.dht[comp_id];
    handler->dqt = &jpeg->dqt.dqt[comp_id];
}

void jpeg_decode(JPEG *jpeg)
{
    //bitio_resync_byte(jpeg->bitio);
    int xstride_by_mcu, ystride_by_mcu;
    xstride_by_mcu = ystride_by_mcu = 8;
    int full_size = jpeg->sof0.width * jpeg->sof0.height;
    for(int i=0;i<3;i++){
        jpeg_init_decode_handler(jpeg, i);
    }
    int blocks_y = jpeg->sof0.height/ystride_by_mcu; 
    int blocks_x = jpeg->sof0.width/xstride_by_mcu; 
    TRACE_DEBUG("blocks_y=%d, blocks_x=%d", blocks_y, blocks_x);
    int count = 0;
    for(int y=0;y<blocks_y;y++){
        for(int x=0;x<blocks_x;x++) {
            TRACE_DEBUG("Block #%x", count++);
            huffman_data_unit_test(jpeg, COMP_Y);
            jpeg->decode.Y.wp = jpeg->decode.Y.data 
                                + x*xstride_by_mcu
                                + 8 * y * jpeg->sof0.width;
            huffman_data_unit_test(jpeg, COMP_Cb);
            huffman_data_unit_test(jpeg, COMP_Cr);
            TRACE_DEBUG("File offset: 0x%x", jpeg->bitio->offset);
        }
    }
    TRACE_DEBUG("Length: %d", jpeg->decode.Y.wp - jpeg->decode.Y.data);
}

void jpeg_save_raw(BITIO *file, byte *raw, int size)
{
    write_file_bytes(file, raw, size);
    byte * empty = (byte *) malloc(size);
    memset(empty, 128, size);
    write_file_bytes(file, empty, size);
}

void jpeg_mcu_size(SOF0Comp *sof_comp)
{
    int xstride_by_mcu, ystride_by_mcu;
    xstride_by_mcu = ystride_by_mcu = 8;
    byte h_factor = sof_comp->sample >> 4;
    byte v_factor = sof_comp->sample & 0x0f;
    TRACE_DEBUG("H Factor: %d, V Factor: %d", h_factor, v_factor);
    if(h_factor | v_factor == 1) {
        TRACE_DEBUG("Use decode 1x1 sampling");
    } else {
        ASSERT(0, "Format not supported!");
    }
}
