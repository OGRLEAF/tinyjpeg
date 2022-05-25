#include "utils.h"

typedef unsigned char byte;
typedef byte bit;
typedef unsigned short word;

enum IO_TYPE {FIL, STR};

typedef struct _bitio
{
    enum IO_TYPE io_type;
    void * src;
    int offset;
    int offset_bit;
    byte next_byte;
} BITIO;

BITIO *create_file_bitio(FILE * fp);

byte read_byte(BITIO *bitio_inst);
int read_bytes(BITIO * bitio_inst, char * ptr, int size, int n);
bit read_bit(BITIO *bitio_inst);
word read_bits(BITIO * bitio_inst, int bits_n);
short read_bits_signed(BITIO *bitio_inst, int bits_n);

int bitio_seekbit(BITIO * bitio_inst, long __off, int __off_bit, int __whence);
int bitio_seek(BITIO * bitio_inst, long __off, int __whence);

byte read_file_byte(BITIO * bitio_inst);
int read_file_bytes(BITIO * bitio_inst, char * ptr, int size, int n);
bit read_file_bit(BITIO *bitio_inst);
word read_file_bits(BITIO * bitio_inst, int bits_n);

void bitio_resync_byte(BITIO * bitio);

int write_file_bytes(BITIO * bitio_inst, byte * src, int n);