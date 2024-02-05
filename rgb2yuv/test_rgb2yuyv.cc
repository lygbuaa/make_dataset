#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "SignalBase.h"
#include "rgb2yuyv.h"
#include "cpu.h"

// #define img_w 1920
// #define img_h 1080

bool load_image_file(const char* img_path, uint8_t* const buffer, const size_t len)
{
    FILE * pFile;
    long lSize;
    // uint8_t * buffer;
    size_t result;
    LOGPF("reading image file: %s", img_path);

    pFile = fopen(img_path , "rb");
    if (pFile==NULL) 
    {
        LOGPF("open file %s failed", img_path);
        return false;
    }

    // obtain file size:
    fseek(pFile , 0 , SEEK_END);
    lSize = ftell (pFile);
    rewind (pFile);
    LOGPF("file data size: %ld, read len: %ld", lSize, len);
    if(lSize < len)
    {
        LOGPF("image file size short!");
        return false;
    }

    // copy the file into the buffer:
    result = fread (buffer, 1, len, pFile);
    if (result != len) 
    {
        LOGPF("read file error");
        return false;
    }
    LOGPF("load image data %ld from %s", len, img_path);

    // terminate
    fclose (pFile);
    return true;
}

int main(int argc, char **argv)
{
    for(int i = 0; i < argc; i++)
    {
        LOGPF("argv[%d] = %s\n", i, argv[i]);
    }

    const char* input_img_path = argv[1];
    const int img_w = atoi(argv[2]);
    const int img_h = atoi(argv[3]);
    const char* output_img_path = argv[4];

    SignalBase::CatchSignal();
    LOGPF("x86 cpu version, is_ssse3: %d, is_avx2: %d, is_avx512bw: %d", is_ssse3(), is_avx2(), is_avx512bw());
  
    const int rgb3_size = img_w * img_h * 3;
    const int yuyv_size = img_w * img_h * 2;

    /** ensure pointer 16-bytes aligned */
    uint8_t* rgb_buffer = (uint8_t*)aligned_alloc(16, rgb3_size);
    uint8_t* yuv_buffer = (uint8_t*)aligned_alloc(16, yuyv_size);
    LOGPF("rgb_buffer is 16-bytes aligned: %d, yuv_buffer is 16-bytes aligned: %d", is_addr_aligned(rgb_buffer, 16), is_addr_aligned(yuv_buffer, 16));

    memset(rgb_buffer, 0x0, rgb3_size);
    memset(yuv_buffer, 0x0, yuyv_size);

    load_image_file(input_img_path, rgb_buffer, rgb3_size);

    for(int i=0; i<100; i++)
    {
        HANG_STOPWATCH();
        rgb2yuyv(rgb_buffer,
                img_w, 
                img_h, 
                3*img_w, 
                yuv_buffer, 
                2*img_w,
                1  // 0 for yuyv, 1 for uyvy
                );
    }

    /** save uyvy file */
    FILE * pFile;
    pFile = fopen(output_img_path, "w");
    if (pFile == NULL)
    {
        LOGPF("open file %s failed", output_img_path);
        fclose (pFile);
        return false;
    }
    fwrite(yuv_buffer, sizeof(char), yuyv_size, pFile);
    fclose (pFile);

    free(rgb_buffer);
    free(yuv_buffer);
    return 0;
}