#ifndef __RGB2YUV__
#define __RGB2YUV__

#ifdef __cplusplus
extern "C"{
#endif

/** swap_yuv=0 for yuyv, swap_yuv=1 for uyvy */
int  rgb2yuyv(unsigned char const*  rgb,
              unsigned int          width,
              unsigned int          height,
              unsigned int          rgb_linesize,
              unsigned char*        yuv,
              unsigned int          yuv_linesize,
              int                   swap_yuv);

#ifdef __cplusplus
}
#endif

#endif
