#include "tiny_jpeg_base.h"
#include "stdlib.h"
#include "string.h"

const unsigned char zigzag[64] = 
{
   0,  1,  5,  6, 14, 15, 27, 28,
   2,  4,  7, 13, 16, 26, 29, 42,
   3,  8, 12, 17, 25, 30, 41, 43,
   9, 11, 18, 24, 31, 40, 44, 53,
  10, 19, 23, 32, 39, 45, 52, 54,
  20, 22, 33, 38, 46, 51, 55, 60,
  21, 34, 37, 47, 50, 56, 59, 61,
  35, 36, 48, 49, 57, 58, 62, 63
};

void jpeg_idct() {

}

void jpeg_build_huff_table(DHTInfo *dht_info) {
    HuffLookupTable * table = &dht_info->huff_table;
    int sum = 0;
    for(int i=0;i<16;i++){
        byte t = dht_info->bit_table[i];
        sum += t;
    };

    table->code = (word *) malloc(sizeof(word) * sum);
    table->len = (byte *) malloc(sizeof(byte) * sum);
    table->weight = (byte *) malloc(sizeof(byte) * sum);
    table->size = sum;

    word code = 0;
    int idx = 1;
    int weight_idx = 0;
    for(int i=0;i<16;i++)
    {
        int n = dht_info->bit_table[i];

        if(n>0) idx++;
        for(int j=0;j<n;j++) {
            byte weight = dht_info->value_table[weight_idx];
            table->code[weight_idx] = code;
            table->len[weight_idx] = i+1;
            table->weight[weight_idx] = weight;
            code += 1;
            weight_idx += 1;
        }
        if(idx>1) code = (code)<<1;
    }
}

byte jpeg_find_huff_code(DHTInfo *dht_info, int len, word code) {
    HuffLookupTable *table = &dht_info->huff_table;
    
    for(int i=0;i<table->size;i++) {
        int find_len = table->len[i];
        //if(find_len<len) continue;
        //TRACE_DEBUG("is i=%d %d=code=%d findlen=%d len=%d", i, table->code[i], code, find_len, len);
        if(find_len==len) {
            if(table->code[i]==code) {
                return table->weight[i];
            }
            continue;
        } else if(find_len>len) break;
    }
    return 0xff;
}

void jpeg_build_quantization_table(float *qtable, byte * ref_table) {
    int i, j;
    static const double aanscalefactor[8] = {
        1.0, 1.387039845, 1.306562965, 1.175875602,
        1.0, 0.785694958, 0.541196100, 0.275899379};
    const unsigned char *zz = zigzag;

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            *qtable++ = ref_table[*zz++] * aanscalefactor[i] * aanscalefactor[j];
        }
    }
}

void jpeg_idct_float_basic(float ** input, float ** output)
{
    float trans_matrix[8][8] = {
        {0.3536,    0.3536,    0.3536,    0.3536,    0.3536,    0.3536,    0.3536,    0.3536,},
        {0.4904,    0.4157,    0.2778,    0.0975,   -0.0975,   -0.2778,   -0.4157,   -0.4904,},
        {0.4619,    0.1913,   -0.1913,   -0.4619,   -0.4619,   -0.1913,    0.1913,    0.4619,},
        {0.4157,   -0.0975,   -0.4904,   -0.2778,    0.2778,    0.4904,    0.0975,   -0.4157,},
        {0.3536,   -0.3536,   -0.3536,    0.3536,    0.3536,   -0.3536,   -0.3536,    0.3536,},
        {0.2778,   -0.4904,    0.0975,    0.4157,   -0.4157,   -0.0975,    0.4904,   -0.2778,},
        {0.1913,   -0.4619,    0.4619,   -0.1913,   -0.1913,    0.4619,   -0.4619,    0.1913,},
        {0.0975,   -0.2778,    0.4157,   -0.4904,    0.4904,   -0.4157,    0.2778,   -0.0975,},
    };

    float tmp[8][8];
    float dst[8][8];
    for(int i=0;i<8;i++) {
        
        for(int j=0;j<8;j++) {
            float t = 0;
            for(int k=0;k<8;k++) {
                int flat_index = k * 8 + j;
                // short value = dct_img[flat_index];
                // float dequan = value * qtable[flat_index];
                t += trans_matrix[k][i] * input[k][j];
            }
            tmp[i][j] = t;
        }
    }

    for(int i=0;i<8;i++) {
        for(int j=0;j<8;j++) {
            float t = 0;
            for(int k=0;k<8;k++) {
                t += trans_matrix[k][j] * tmp[i][k];
            }
            int flat_index = i * 8 + j;
            output[i][j] = t;
        }
    }
}



//static inline unsigned char descale_and_clamp(int x, int shift)
byte descale_and_clamp(int x, int shift){
  x += (1UL<<(shift-1));
  if (x<0)
    x = (x >> shift) | ((~(0UL)) << (32-(shift)));
  else
    x >>= shift;
  x += 128;
  if (x>255)
    return 255;
  else if (x<0)
    return 0;
  else 
    return x;
}


void jpeg_idct_basic(short * dct_img, float * qtable, byte * output_img) {
    float ** input = (float **) malloc(sizeof(float *) * 8);
    float ** output = (float **) malloc(sizeof(float *) * 8);
    for(int i=0;i<8;i++) {
        input[i] = (float *) malloc(sizeof(float) * 8);
        output[i] = (float *) malloc(sizeof(float) * 8);
        for(int j=0;j<8;j++) {
            int flat_index = i * 8 + j;
            input[i][j] = dct_img[flat_index]* qtable[flat_index];
        }
    }
    jpeg_idct_float_basic(input, output);
    for(int i=0;i<8;i++) {
        for(int j=0;j<8;j++) {
            int flat_index = i * 8 + j;
            byte value = descale_and_clamp((int) output[i][j], 3);
            output_img[flat_index] = value;
            printf("%.2x\t", value);
        }
        printf("\r\n");
    }
    free(input);
    free(output);
}

void jpeg_allocate(DecodeHandler *handler, byte * src)
{
    byte * dst = handler->wp;
    for(int i=0;i<8;i++) {
        memcpy(dst, src, 8);
        dst += handler->width;
        src += 8;
    }
}

void jpeg_set_block(DecodeHandler * handler, int block_x, int block_y)
{
  handler->wp = handler->data + 
                    + block_x * handler->x_stride
                    + handler->y_stride * block_y * handler->width; 
}

/*
 * Perform dequantization and inverse DCT on one block of coefficients.
 */
void
tinyjpeg_idct_float (short * DCT, float *Q_table, byte *output_buf, int stride)
{
  FAST_FLOAT tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  FAST_FLOAT tmp10, tmp11, tmp12, tmp13;
  FAST_FLOAT z5, z10, z11, z12, z13;
  short *inptr;
  FAST_FLOAT *quantptr;
  FAST_FLOAT *wsptr;
  byte *outptr;
  int ctr;
  FAST_FLOAT workspace[DCTSIZE2]; /* buffers data between passes */

  /* Pass 1: process columns from input, store into work array. */

  inptr = DCT;
  quantptr = Q_table;
  wsptr = workspace;
  for (ctr = DCTSIZE; ctr > 0; ctr--) {
    /* Due to quantization, we will usually find that many of the input
     * coefficients are zero, especially the AC terms.  We can exploit this
     * by short-circuiting the IDCT calculation for any column in which all
     * the AC terms are zero.  In that case each output is equal to the
     * DC coefficient (with scale factor as needed).
     * With typical images and quantization tables, half or more of the
     * column DCT calculations can be simplified this way.
     */
    
    if (inptr[DCTSIZE*1] == 0 && inptr[DCTSIZE*2] == 0 &&
	inptr[DCTSIZE*3] == 0 && inptr[DCTSIZE*4] == 0 &&
	inptr[DCTSIZE*5] == 0 && inptr[DCTSIZE*6] == 0 &&
	inptr[DCTSIZE*7] == 0) {
      /* AC terms all zero */
      FAST_FLOAT dcval = DEQUANTIZE(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);
      
      wsptr[DCTSIZE*0] = dcval;
      wsptr[DCTSIZE*1] = dcval;
      wsptr[DCTSIZE*2] = dcval;
      wsptr[DCTSIZE*3] = dcval;
      wsptr[DCTSIZE*4] = dcval;
      wsptr[DCTSIZE*5] = dcval;
      wsptr[DCTSIZE*6] = dcval;
      wsptr[DCTSIZE*7] = dcval;
      
      inptr++;			/* advance pointers to next column */
      quantptr++;
      wsptr++;
      continue;
    }
    
    /* Even part */

    tmp0 = DEQUANTIZE(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);
    tmp1 = DEQUANTIZE(inptr[DCTSIZE*2], quantptr[DCTSIZE*2]);
    tmp2 = DEQUANTIZE(inptr[DCTSIZE*4], quantptr[DCTSIZE*4]);
    tmp3 = DEQUANTIZE(inptr[DCTSIZE*6], quantptr[DCTSIZE*6]);

    tmp10 = tmp0 + tmp2;	/* phase 3 */
    tmp11 = tmp0 - tmp2;

    tmp13 = tmp1 + tmp3;	/* phases 5-3 */
    tmp12 = (tmp1 - tmp3) * ((FAST_FLOAT) 1.414213562) - tmp13; /* 2*c4 */

    tmp0 = tmp10 + tmp13;	/* phase 2 */
    tmp3 = tmp10 - tmp13;
    tmp1 = tmp11 + tmp12;
    tmp2 = tmp11 - tmp12;
    
    /* Odd part */

    tmp4 = DEQUANTIZE(inptr[DCTSIZE*1], quantptr[DCTSIZE*1]);
    tmp5 = DEQUANTIZE(inptr[DCTSIZE*3], quantptr[DCTSIZE*3]);
    tmp6 = DEQUANTIZE(inptr[DCTSIZE*5], quantptr[DCTSIZE*5]);
    tmp7 = DEQUANTIZE(inptr[DCTSIZE*7], quantptr[DCTSIZE*7]);

    z13 = tmp6 + tmp5;		/* phase 6 */
    z10 = tmp6 - tmp5;
    z11 = tmp4 + tmp7;
    z12 = tmp4 - tmp7;

    tmp7 = z11 + z13;		/* phase 5 */
    tmp11 = (z11 - z13) * ((FAST_FLOAT) 1.414213562); /* 2*c4 */

    z5 = (z10 + z12) * ((FAST_FLOAT) 1.847759065); /* 2*c2 */
    tmp10 = ((FAST_FLOAT) 1.082392200) * z12 - z5; /* 2*(c2-c6) */
    tmp12 = ((FAST_FLOAT) -2.613125930) * z10 + z5; /* -2*(c2+c6) */

    tmp6 = tmp12 - tmp7;	/* phase 2 */
    tmp5 = tmp11 - tmp6;
    tmp4 = tmp10 + tmp5;

    wsptr[DCTSIZE*0] = tmp0 + tmp7;
    wsptr[DCTSIZE*7] = tmp0 - tmp7;
    wsptr[DCTSIZE*1] = tmp1 + tmp6;
    wsptr[DCTSIZE*6] = tmp1 - tmp6;
    wsptr[DCTSIZE*2] = tmp2 + tmp5;
    wsptr[DCTSIZE*5] = tmp2 - tmp5;
    wsptr[DCTSIZE*4] = tmp3 + tmp4;
    wsptr[DCTSIZE*3] = tmp3 - tmp4;

    inptr++;			/* advance pointers to next column */
    quantptr++;
    wsptr++;
  }
  
  /* Pass 2: process rows from work array, store into output array. */
  /* Note that we must descale the results by a factor of 8 == 2**3. */

  wsptr = workspace;
  outptr = output_buf;
  for (ctr = 0; ctr < DCTSIZE; ctr++) {
    /* Rows of zeroes can be exploited in the same way as we did with columns.
     * However, the column calculation has created many nonzero AC terms, so
     * the simplification applies less often (typically 5% to 10% of the time).
     * And testing floats for zero is relatively expensive, so we don't bother.
     */
    
    /* Even part */

    tmp10 = wsptr[0] + wsptr[4];
    tmp11 = wsptr[0] - wsptr[4];

    tmp13 = wsptr[2] + wsptr[6];
    tmp12 = (wsptr[2] - wsptr[6]) * ((FAST_FLOAT) 1.414213562) - tmp13;

    tmp0 = tmp10 + tmp13;
    tmp3 = tmp10 - tmp13;
    tmp1 = tmp11 + tmp12;
    tmp2 = tmp11 - tmp12;

    /* Odd part */

    z13 = wsptr[5] + wsptr[3];
    z10 = wsptr[5] - wsptr[3];
    z11 = wsptr[1] + wsptr[7];
    z12 = wsptr[1] - wsptr[7];

    tmp7 = z11 + z13;
    tmp11 = (z11 - z13) * ((FAST_FLOAT) 1.414213562);

    z5 = (z10 + z12) * ((FAST_FLOAT) 1.847759065); /* 2*c2 */
    tmp10 = ((FAST_FLOAT) 1.082392200) * z12 - z5; /* 2*(c2-c6) */
    tmp12 = ((FAST_FLOAT) -2.613125930) * z10 + z5; /* -2*(c2+c6) */

    tmp6 = tmp12 - tmp7;
    tmp5 = tmp11 - tmp6;
    tmp4 = tmp10 + tmp5;

    /* Final output stage: scale down by a factor of 8 and range-limit */

    outptr[0] = descale_and_clamp((int)(tmp0 + tmp7), 3);
    outptr[7] = descale_and_clamp((int)(tmp0 - tmp7), 3);
    outptr[1] = descale_and_clamp((int)(tmp1 + tmp6), 3);
    outptr[6] = descale_and_clamp((int)(tmp1 - tmp6), 3);
    outptr[2] = descale_and_clamp((int)(tmp2 + tmp5), 3);
    outptr[5] = descale_and_clamp((int)(tmp2 - tmp5), 3);
    outptr[4] = descale_and_clamp((int)(tmp3 + tmp4), 3);
    outptr[3] = descale_and_clamp((int)(tmp3 - tmp4), 3);

    
    wsptr += DCTSIZE;		/* advance pointer to next row */
    outptr += stride;
  }
}


void print_block(byte *block, int h, int w)
{
    for(int i=0;i<h;i++){
        for(int j=0;j<w;j++){
            int flat_idx = i * w + j;
            printf("%.2x\t", block[flat_idx]);
        }
        printf("\r\n");
    }
}

int expand_8(int value)
{
    if((value%8)==0) return value;
    else {
        return (value/8)*8 + 8;
    }
}
/**
 * @brief 
    word code = 0;
    int idx = 1;
    int weight_idx = 0;
    for(int i=0;i<16;i++)
    {
        int n = dht_info->bit_table[i];
        //TRACE_INFO("\t\t- Length: \t%d\tNumber: \t%d", i+1, n);

        if(idx>0) {}
        if(n>0) idx++;
        
        for(int j=0;j<n;j++){
            byte weight = dht_info->value_table[weight_idx];
            // TRACE_INFO("\t\t- " BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN " (%d)\t",
            //                     BYTE_TO_BINARY(code>>8), BYTE_TO_BINARY(code),
            //                     weight);
            code += 1;
            weight_idx += 1;

        }
        if(idx>1) code = (code)<<1;
    }
 * 
 */
