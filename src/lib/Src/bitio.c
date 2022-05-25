#include<stdlib.h>
#include<stdio.h>
#include "bitio.h"
#include "trace.h"


#define SWITCH_TYPE(TYPE, FIL_HANDLE_EXP, STR_HANDLE_EXP) switch(TYPE) {case FIL: FIL_HANDLE_EXP;break;case STR: STR_HANDLE_EXP;break;default: break;}

BITIO *create_file_bitio(FILE * fp)
{
    BITIO *bitio_inst = (BITIO *) malloc(sizeof(BITIO));
    ASSERT_NOT_NULL(fp, "FILE is null");
    bitio_inst->src = fp;
    bitio_inst->io_type = FIL;
    bitio_inst->offset = 0;
    bitio_inst->offset_bit = 0;
}

byte read_byte(BITIO *bitio_inst) {
    SWITCH_TYPE(bitio_inst->io_type, 
                return read_file_byte(bitio_inst),
                (0));
}

int read_bytes(BITIO * bitio_inst, char * ptr, int size, int n) {
    SWITCH_TYPE(bitio_inst->io_type, 
                return read_file_bytes(bitio_inst, ptr, size, n),
                (0));
}

bit read_bit(BITIO *bitio_inst)
{
    SWITCH_TYPE(bitio_inst->io_type, 
             return read_file_bit(bitio_inst);,
            (0));
}

word read_bits(BITIO * bitio_inst, int bits_n)
{
    SWITCH_TYPE(bitio_inst->io_type, 
                return read_file_bits(bitio_inst, bits_n), // is return?
                (0));
}

short read_bits_signed(BITIO *bitio_inst, int bits_n)
{
    word ret;
    SWITCH_TYPE(bitio_inst->io_type, 
                ret = read_file_bits(bitio_inst, bits_n),
                (0));

    if ((word)ret < (1UL<<((bits_n)-1))) \
        ret += (0xFFFFUL<<(bits_n))+1;
    return (short) ret;
}

byte read_file_byte(BITIO * bitio_inst)
{
    byte b;
    read_file_bytes(bitio_inst, &b, 1, 1);
    return b;
}

int read_file_bytes(BITIO * bitio_inst, char * ptr, int size, int n)
{
    FILE * fp = (FILE *) bitio_inst->src;
    long file_pos = ftell(fp);
    //TRACE_DEBUG("read_file_byte, check %ld %d", file_pos, bitio_inst->offset);
    ASSERT_EQUAL(file_pos, bitio_inst->offset, "BUG: file_pos != offset");
    int read_size = fread(ptr, size, n, fp);
    bitio_inst->offset = ftell(fp);
    bitio_inst->next_byte = ptr[read_size-1];
    bitio_inst->offset_bit = 8;
    return read_size;
}

bit read_file_bit(BITIO *bitio_inst)
{
    FILE * fp = (FILE *) bitio_inst->src;
    int offset_bit = bitio_inst->offset_bit;
    //TRACE_DEBUG("offset_bit %d", offset_bit);
    if(offset_bit>=8) {
        fread(&bitio_inst->next_byte, 1, 1, fp);
        byte tmp;
        fread(&tmp, 1, 1, fp);
        if(bitio_inst->next_byte==0xff && tmp==0x00) {
            //TRACE_DEBUG("Catch 0xff 0x00");
        } else {
            fseek(fp, -1, SEEK_CUR);
        }
        //TRACE_DEBUG("Read file bit: get next byte 0x%x", bitio_inst->next_byte);
        bitio_inst->offset = ftell(fp);
        bitio_inst->offset_bit = 0;
    }
    byte ret = ((bitio_inst->next_byte) >> (7 - bitio_inst->offset_bit));
    bitio_inst->offset_bit += 1;
    ret = ret & 0b1;
    // TRACE_DEBUG("offset 0x%x offset_bit %d: %d %x", bitio_inst->offset, bitio_inst->offset_bit, 
    //     ret, bitio_inst->next_byte);
    return ret;
}

word read_file_bits(BITIO * bitio_inst, int bits_n) {
    word b = 0;
    for(int i=0;i<bits_n;i++) {
        bit next_bit = read_file_bit(bitio_inst);
        b = b + (next_bit);
        if(i<bits_n-1) b = b<<1;
    }
    return b;
}

int write_file_bytes(BITIO * bitio_inst, byte * src, int n)
{
    FILE * fp = (FILE *) bitio_inst->src;
    int size = fwrite(src, 1, n, fp);
    bitio_inst->offset = ftell(fp);
    return size;
}

int bitio_seek(BITIO * bitio_inst, long __off, int __whence)
{
    return bitio_seekbit(bitio_inst, __off, 0, __whence);
}

void bitio_resync_byte(BITIO * bitio)
{
    bitio->offset_bit = 8;
}

int bitio_seekbit(BITIO * bitio_inst, long __off, int __off_bit, int __whence)
{
    FILE * fp = (FILE *) bitio_inst->src;
    fseek(fp, __off, __whence);
    long curr = ftell(fp);
    bitio_inst->offset = curr;
    bitio_inst->offset_bit = __off_bit;
    return 1;
}

long bitio_tell(BITIO * bitio_inst)
{

}
