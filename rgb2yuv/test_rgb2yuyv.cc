#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "SignalBase.h"
#include "rgb2yuyv.h"
#include "cpu.h"

#define IMG_W 1920
#define IMG_H 1080

int main(int argc, char **argv)
{
    for(int i = 0; i < argc; i++)
    {
        LOGPF("argv[%d] = %s\n", i, argv[i]);
    }

    SignalBase::CatchSignal();
    LOGPF("x86 cpu version, is_ssse3: %d, is_avx2: %d, is_avx512bw: %d", is_ssse3(), is_avx2(), is_avx512bw());
  
    const int rgb3_size = IMG_W * IMG_H * 3;
    const int yuyv_size = IMG_W * IMG_H * 2;

    /** ensure pointer 16-bytes aligned */
    uint8_t* rgb_buffer_ = (uint8_t*)aligned_alloc(16, rgb3_size);
    uint8_t* yuv_buffer_ = (uint8_t*)aligned_alloc(16, yuyv_size);
    LOGPF("rgb_buffer_ is 16-bytes aligned: %d, yuv_buffer_ is 16-bytes aligned: %d", is_addr_aligned(rgb_buffer_, 16), is_addr_aligned(yuv_buffer_, 16));

    memset(rgb_buffer_, 0x0f, rgb3_size);
    memset(yuv_buffer_, 0x0f, yuyv_size);

    for(int i=0; i<1000; i++)
    {
        HANG_STOPWATCH();
        rgb2yuyv(rgb_buffer_,
                IMG_W, 
                IMG_H, 
                3*IMG_W, 
                yuv_buffer_, 
                2*IMG_W,
                1  // 0 for yuyv, 1 for uyvy
                );
    }

    free(rgb_buffer_);
    free(yuv_buffer_);

    return 0;
}