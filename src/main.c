#include <stdio.h>
#include "hello.h"
#include "trace.h"
#include "tiny_jpeg.h"
#include "utils.h"

int main(int argc, char const *argv[])
{
    ASSERT_EQUAL(argc, 2, "param invalid.");
    const char * input_file_path = argv[1];
    printf("%x", 0xff>>0);
    JPEG *jpeg_file;
    // jpeg_file = read_jpeg(input_file_path);
    // print_jpeg_breif(jpeg_file);
    // print_jpeg();
    jpeg_file = read_jpeg(input_file_path);
    print_jpeg(jpeg_file);
    print_jpeg_struct(jpeg_file);
    jpeg_mcu_size(jpeg_file->sof0.comps + 0);
    jpeg_decode(jpeg_file);
    jpeg_file->output_file = create_file_bitio(fopen("./test_output/output.yuv", "wb"));
    jpeg_save_raw(jpeg_file->output_file, jpeg_file->decode.Y.data, 1024 * 1024);
    return 0;
}
