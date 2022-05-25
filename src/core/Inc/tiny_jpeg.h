#include "tiny_jpeg_base.h"
#include "tiny_jpeg_ext.h"

void hello_tiny_jpeg();
JPEG * read_jpeg(const char * file_path);
void print_jpeg_breif(JPEG * jpeg_file);
int print_jpeg(JPEG *jpeg);
void jpeg_mcu_size(SOF0Comp *sof_comp);
//void huffman_data_unit_test(JPEG * jpeg, int comp_id);
void jpeg_decode(JPEG *jpeg);
void jpeg_save_raw(BITIO *file, byte *raw, int size);