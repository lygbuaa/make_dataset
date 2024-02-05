#include "rgb2yuyv.h"
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>

#define QN_BITS   15

/*
    unsigned char y0 = (unsigned char)((signed char)(0.299f * r0 + 0.587f * g0 + 0.114f * b0));
    unsigned char y1 = (unsigned char)((signed char)(0.299f * r1 + 0.587f * g1 + 0.114f * b1));

    unsigned char u = (unsigned char)((signed char)(-0.169f * r0 - 0.331f * g0 + 0.500f * b0) + 128);
    unsigned char v = (unsigned char)((signed char)( 0.500f * r0 - 0.419f * g0 - 0.081f * b0) + 128);
*/

static int rgb24_to_yuyv(unsigned char const*       rgb,
                         unsigned int               src_width,
                         unsigned int               src_height,
                         unsigned int               rgb_linesize,
                         unsigned char*             yuv,
                         unsigned int               yuv_linesize,
                         signed short int const*    rgb2yuv_coef){
  int i = 0, j = 0;
  const unsigned int yb      = ((unsigned short int const*)(rgb2yuv_coef))[0];
  const unsigned int yg      = ((unsigned short int const*)(rgb2yuv_coef))[1];
  const unsigned int yr      = ((unsigned short int const*)(rgb2yuv_coef))[2];
  const unsigned int ub      = ((unsigned short int const*)(rgb2yuv_coef))[3];
  const unsigned int ug      = ((unsigned short int const*)(rgb2yuv_coef))[4];
  const unsigned int ur      = ((unsigned short int const*)(rgb2yuv_coef))[5];
  const unsigned int vb      = ((unsigned short int const*)(rgb2yuv_coef))[6];
  const unsigned int vg      = ((unsigned short int const*)(rgb2yuv_coef))[7];
  const unsigned int vr      = ((unsigned short int const*)(rgb2yuv_coef))[8];

  const __m128i  y_coef0     = _mm_set1_epi32(yb | (yg << 16));
  const __m128i  y_coef1     = _mm_set1_epi32(yr);

  const __m128i  u_coef0     = _mm_set1_epi32(ub | (ug << 16));
  const __m128i  u_coef1     = _mm_set1_epi32(ur);

  const __m128i  v_coef0     = _mm_set1_epi32(vb | (vg << 16));
  const __m128i  v_coef1     = _mm_set1_epi32(vr);

  const __m128i  half        = _mm_set1_epi32(1 << (QN_BITS - 1));
  const __m128i  v_128_half  = _mm_set1_epi32((128 << QN_BITS) + (1 << (QN_BITS - 1)));
  const __m128i  s_mask0     = _mm_setr_epi8( 0, -1,  1, -1,  3, -1,  4, -1,  6, -1,  7, -1,  9, -1, 10, -1);
  const __m128i  s_mask1     = _mm_setr_epi8( 2, -1, -1, -1,  5, -1, -1, -1,  8, -1, -1, -1, 11, -1, -1, -1);
  const __m128i  s_mask2     = _mm_setr_epi8(12, -1, 13, -1, 15, -1,  0, -1,  2, -1,  3, -1,  5, -1,  6, -1);
  const __m128i  s_mask3     = _mm_setr_epi8(14, -1, -1, -1,  1, -1, -1, -1,  4, -1, -1, -1,  7, -1, -1, -1);
  const __m128i  s_mask4     = _mm_setr_epi8( 8, -1,  9, -1, 11, -1, 12, -1, 14, -1, 15, -1,  1, -1,  2, -1);
  const __m128i  s_mask5     = _mm_setr_epi8(10, -1, -1, -1, 13, -1, -1, -1,  0, -1, -1, -1,  3, -1, -1, -1);
  const __m128i  s_mask6     = _mm_setr_epi8( 4, -1,  5, -1,  7, -1,  8, -1, 10, -1, 11, -1, 13, -1, 14, -1);
  const __m128i  s_mask7     = _mm_setr_epi8( 6, -1, -1, -1,  9, -1, -1, -1, 12, -1, -1, -1, 15, -1, -1, -1);
  const __m128i  blend_mask0 = _mm_setr_epi32(0, 0,          0, 0xFFFFFFFF);
  const __m128i  blend_mask1 = _mm_setr_epi32(0, 0, 0xFFFFFFFF, 0xFFFFFFFF);
  const __m128i  m_zero      = _mm_xor_si128(half, half);
  for(i = 0 ; i < src_height ; i ++){
    unsigned char const*  cur_src_ptr = rgb;
    unsigned char*        cur_dst_ptr = yuv;
    for(j = 0 ; j < src_width ; j += 16){
      __m128i data00, data01, data10, data11;
      __m128i data20, data21, data30, data31;
      {
        __m128i d0      = _mm_loadu_si128((const __m128i*)cur_src_ptr);
        __m128i d1      = _mm_loadu_si128((const __m128i*)(cur_src_ptr + 16));
        __m128i d2      = _mm_loadu_si128((const __m128i*)(cur_src_ptr + 32));
        __m128i blend_0 = _mm_blendv_epi8(d1, d0, blend_mask0);
        __m128i blend_1 = _mm_blendv_epi8(d2, d1, blend_mask1);
        data00          = _mm_shuffle_epi8(d0,      s_mask0);           //bg
        data01          = _mm_shuffle_epi8(d0,      s_mask1);           //ra
        data10          = _mm_shuffle_epi8(blend_0, s_mask2);           //bg
        data11          = _mm_shuffle_epi8(blend_0, s_mask3);           //ra
        data20          = _mm_shuffle_epi8(blend_1, s_mask4);           //bg
        data21          = _mm_shuffle_epi8(blend_1, s_mask5);           //ra
        data30          = _mm_shuffle_epi8(d2,      s_mask6);           //bg
        data31          = _mm_shuffle_epi8(d2,      s_mask7);           //ra
      }

      __m128i   res00   = _mm_madd_epi16(data00, y_coef0);           //bg
      __m128i   res01   = _mm_madd_epi16(data01, y_coef1);           //ra
      __m128i   res10   = _mm_madd_epi16(data10, y_coef0);           //bg
      __m128i   res11   = _mm_madd_epi16(data11, y_coef1);           //ra
      __m128i   res20   = _mm_madd_epi16(data20, y_coef0);           //bg
      __m128i   res21   = _mm_madd_epi16(data21, y_coef1);           //ra
      __m128i   res30   = _mm_madd_epi16(data30, y_coef0);           //bg
      __m128i   res31   = _mm_madd_epi16(data31, y_coef1);           //ra

      res00             = _mm_add_epi32(res00, res01);
      res10             = _mm_add_epi32(res10, res11);
      res20             = _mm_add_epi32(res20, res21);
      res30             = _mm_add_epi32(res30, res31);
      res00             = _mm_add_epi32(res00, half);
      res10             = _mm_add_epi32(res10, half);
      res20             = _mm_add_epi32(res20, half);
      res30             = _mm_add_epi32(res30, half);
      res00             = _mm_srli_epi32(res00, QN_BITS);
      res10             = _mm_srli_epi32(res10, QN_BITS);
      res20             = _mm_srli_epi32(res20, QN_BITS);
      res30             = _mm_srli_epi32(res30, QN_BITS);
      res00             = _mm_packus_epi32(res00, res10);   //Y, first 8
      res20             = _mm_packus_epi32(res20, res30);   //Y, second 8
      //res00             = _mm_packus_epi16(res00, res20);
      //_mm_storeu_si128((__m128i*)(cur_dst_ptr), res00);

      //A(bg0)(bg2), B(bg0)(bg1)
      res01 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data00), 
                                              _mm_castsi128_ps(data10), 
                                              (2 << 6) | (0 << 4) | (2 << 2) | 0));
      //A(bg1)(bg3), B(bg1)(bg3)
      res10 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data00), 
                                              _mm_castsi128_ps(data10), 
                                              (3 << 6) | (1 << 4) | (3 << 2) | 1));
      
      //C(bg0)(bg2), D(bg0)(bg1)
      res11 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data20), 
                                              _mm_castsi128_ps(data30), 
                                              (2 << 6) | (0 << 4) | (2 << 2) | 0));
      //C(bg1)(bg3), D(bg1)(bg3)
      res21 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data20), 
                                              _mm_castsi128_ps(data30), 
                                              (3 << 6) | (1 << 4) | (3 << 2) | 1));

      //(bg0)(bg1)(bg2)(bg3)
      res01 = _mm_avg_epu16(res01, res10);
      //(bg4)(bg5)(bg6)(bg7)
      res11 = _mm_avg_epu16(res11, res21);


      //A(ra0)(ra2), B(ra0)(ra1)
      res10 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data01), 
                                              _mm_castsi128_ps(data11), 
                                              (2 << 6) | (0 << 4) | (2 << 2) | 0));
      //A(ra1)(ra3), B(ra1)(ra3)
      res21 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data01), 
                                              _mm_castsi128_ps(data11), 
                                              (3 << 6) | (1 << 4) | (3 << 2) | 1));
      
      //C(ra0)(ra2), D(ra0)(ra1)
      res30 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data21), 
                                              _mm_castsi128_ps(data31), 
                                              (2 << 6) | (0 << 4) | (2 << 2) | 0));
      //C(ra1)(ra3), D(ra1)(ra3)
      res31 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data21), 
                                              _mm_castsi128_ps(data31), 
                                              (3 << 6) | (1 << 4) | (3 << 2) | 1));
      //(ra0)(ra1)(ra2)(ra3)
      res10 = _mm_avg_epu16(res10, res21);
      //(ra4)(ra5)(ra6)(ra7)
      res30 = _mm_avg_epu16(res30, res31);

      __m128i   res_u_00  = _mm_madd_epi16(res01, u_coef0);   //bg_u
      __m128i   res_u_01  = _mm_madd_epi16(res10, u_coef1);   //ra_u
      __m128i   res_u_10  = _mm_madd_epi16(res11, u_coef0);   //bg_u
      __m128i   res_u_11  = _mm_madd_epi16(res30, u_coef1);   //ra_u

      __m128i   res_v_00  = _mm_madd_epi16(res01, v_coef0);   //bg_v
      __m128i   res_v_01  = _mm_madd_epi16(res10, v_coef1);   //ra_v
      __m128i   res_v_10  = _mm_madd_epi16(res11, v_coef0);   //bg_v
      __m128i   res_v_11  = _mm_madd_epi16(res30, v_coef1);   //ra_v

      res_u_00 = _mm_add_epi32(res_u_00, res_u_01);
      res_u_10 = _mm_add_epi32(res_u_10, res_u_11);
      res_v_00 = _mm_add_epi32(res_v_00, res_v_01);
      res_v_10 = _mm_add_epi32(res_v_10, res_v_11);

      res_u_00 = _mm_add_epi32(res_u_00, v_128_half);
      res_u_10 = _mm_add_epi32(res_u_10, v_128_half);
      res_v_00 = _mm_add_epi32(res_v_00, v_128_half);
      res_v_10 = _mm_add_epi32(res_v_10, v_128_half);

      res_u_00 = _mm_srli_epi32(res_u_00, QN_BITS);
      res_u_10 = _mm_srli_epi32(res_u_10, QN_BITS);
      res_v_00 = _mm_srli_epi32(res_v_00, QN_BITS);
      res_v_10 = _mm_srli_epi32(res_v_10, QN_BITS);

#if 0
      //uv0,uv1,uv2,uv3
      res_u_00 = _mm_packus_epi32(res_u_00, res_v_00);
      //uv4,uv5,uv6,uv7
      res_u_10 = _mm_packus_epi32(res_u_10, res_v_10);
#else
      res_u_00 = _mm_max_epi32(res_u_00, m_zero);
      res_u_10 = _mm_max_epi32(res_u_10, m_zero);
      res_v_00 = _mm_max_epi32(res_v_00, m_zero);
      res_v_10 = _mm_max_epi32(res_v_10, m_zero);

      res_v_00 = _mm_slli_epi32(res_v_00,  16);
      res_v_10 = _mm_slli_epi32(res_v_10,  16);
      res_u_00 = _mm_or_si128(res_u_00, res_v_00);
      res_u_10 = _mm_or_si128(res_u_10, res_v_10);
#endif

      //y0u0y1v0, y2u1y3v1
      __m128i res_yuv0 = _mm_unpacklo_epi16(res00, res_u_00);
      //y4u2y5v2, y6u3y7v3
      __m128i res_yuv1 = _mm_unpackhi_epi16(res00, res_u_00);

      //y8u4y9v4,   y10u5y11v5
      __m128i res_yuv2 = _mm_unpacklo_epi16(res20, res_u_10);
      //y12u6y13v6, y14u7y15v7
      __m128i res_yuv3 = _mm_unpackhi_epi16(res20, res_u_10);

      res_yuv0 = _mm_packus_epi16(res_yuv0, res_yuv1);
      res_yuv2 = _mm_packus_epi16(res_yuv2, res_yuv3);

      _mm_storeu_si128((__m128i*)(cur_dst_ptr),      res_yuv0);
      _mm_storeu_si128((__m128i*)(cur_dst_ptr + 16), res_yuv2);

      cur_src_ptr += 48;
      cur_dst_ptr += 32;
    }
    rgb += rgb_linesize;
    yuv += yuv_linesize;
  }
  return 0;
}

#if 1
static int rgb24_to_uyvy(unsigned char const*      rgb,
                         unsigned int              src_width,
                         unsigned int              src_height,
                         unsigned int              rgb_linesize,
                         unsigned char*            yuv,
                         unsigned int              yuv_linesize,
                         signed short int const*   rgb2yuv_coef){
  int i = 0, j = 0;
  const unsigned int yb      = ((unsigned short int const*)(rgb2yuv_coef))[0];
  const unsigned int yg      = ((unsigned short int const*)(rgb2yuv_coef))[1];
  const unsigned int yr      = ((unsigned short int const*)(rgb2yuv_coef))[2];
  const unsigned int ub      = ((unsigned short int const*)(rgb2yuv_coef))[3];
  const unsigned int ug      = ((unsigned short int const*)(rgb2yuv_coef))[4];
  const unsigned int ur      = ((unsigned short int const*)(rgb2yuv_coef))[5];
  const unsigned int vb      = ((unsigned short int const*)(rgb2yuv_coef))[6];
  const unsigned int vg      = ((unsigned short int const*)(rgb2yuv_coef))[7];
  const unsigned int vr      = ((unsigned short int const*)(rgb2yuv_coef))[8];

  const __m128i  y_coef0     = _mm_set1_epi32(yb | (yg << 16));
  const __m128i  y_coef1     = _mm_set1_epi32(yr);

  const __m128i  u_coef0     = _mm_set1_epi32(ub | (ug << 16));
  const __m128i  u_coef1     = _mm_set1_epi32(ur);

  const __m128i  v_coef0     = _mm_set1_epi32(vb | (vg << 16));
  const __m128i  v_coef1     = _mm_set1_epi32(vr);

  const __m128i  half        = _mm_set1_epi32(1 << (QN_BITS - 1));
  const __m128i  v_128_half  = _mm_set1_epi32((128 << QN_BITS) + (1 << (QN_BITS - 1)));
  const __m128i  s_mask0     = _mm_setr_epi8( 0, -1,  1, -1,  3, -1,  4, -1,  6, -1,  7, -1,  9, -1, 10, -1);
  const __m128i  s_mask1     = _mm_setr_epi8( 2, -1, -1, -1,  5, -1, -1, -1,  8, -1, -1, -1, 11, -1, -1, -1);
  const __m128i  s_mask2     = _mm_setr_epi8(12, -1, 13, -1, 15, -1,  0, -1,  2, -1,  3, -1,  5, -1,  6, -1);
  const __m128i  s_mask3     = _mm_setr_epi8(14, -1, -1, -1,  1, -1, -1, -1,  4, -1, -1, -1,  7, -1, -1, -1);
  const __m128i  s_mask4     = _mm_setr_epi8( 8, -1,  9, -1, 11, -1, 12, -1, 14, -1, 15, -1,  1, -1,  2, -1);
  const __m128i  s_mask5     = _mm_setr_epi8(10, -1, -1, -1, 13, -1, -1, -1,  0, -1, -1, -1,  3, -1, -1, -1);
  const __m128i  s_mask6     = _mm_setr_epi8( 4, -1,  5, -1,  7, -1,  8, -1, 10, -1, 11, -1, 13, -1, 14, -1);
  const __m128i  s_mask7     = _mm_setr_epi8( 6, -1, -1, -1,  9, -1, -1, -1, 12, -1, -1, -1, 15, -1, -1, -1);
  const __m128i  blend_mask0 = _mm_setr_epi32(0, 0,          0, 0xFFFFFFFF);
  const __m128i  blend_mask1 = _mm_setr_epi32(0, 0, 0xFFFFFFFF, 0xFFFFFFFF);
  const __m128i  m_zero      = _mm_xor_si128(half, half);
  for(i = 0 ; i < src_height ; i ++){
    unsigned char const*  cur_src_ptr = rgb;
    unsigned char*        cur_dst_ptr = yuv;
    for(j = 0 ; j < src_width ; j += 16){
      __m128i data00, data01, data10, data11;
      __m128i data20, data21, data30, data31;
      {
        __m128i d0      = _mm_loadu_si128((const __m128i*)cur_src_ptr);
        __m128i d1      = _mm_loadu_si128((const __m128i*)(cur_src_ptr + 16));
        __m128i d2      = _mm_loadu_si128((const __m128i*)(cur_src_ptr + 32));
        __m128i blend_0 = _mm_blendv_epi8(d1, d0, blend_mask0);
        __m128i blend_1 = _mm_blendv_epi8(d2, d1, blend_mask1);
        data00          = _mm_shuffle_epi8(d0,      s_mask0);           //bg
        data01          = _mm_shuffle_epi8(d0,      s_mask1);           //ra
        data10          = _mm_shuffle_epi8(blend_0, s_mask2);           //bg
        data11          = _mm_shuffle_epi8(blend_0, s_mask3);           //ra
        data20          = _mm_shuffle_epi8(blend_1, s_mask4);           //bg
        data21          = _mm_shuffle_epi8(blend_1, s_mask5);           //ra
        data30          = _mm_shuffle_epi8(d2,      s_mask6);           //bg
        data31          = _mm_shuffle_epi8(d2,      s_mask7);           //ra
      }

      __m128i   res00   = _mm_madd_epi16(data00, y_coef0);           //bg
      __m128i   res01   = _mm_madd_epi16(data01, y_coef1);           //ra
      __m128i   res10   = _mm_madd_epi16(data10, y_coef0);           //bg
      __m128i   res11   = _mm_madd_epi16(data11, y_coef1);           //ra
      __m128i   res20   = _mm_madd_epi16(data20, y_coef0);           //bg
      __m128i   res21   = _mm_madd_epi16(data21, y_coef1);           //ra
      __m128i   res30   = _mm_madd_epi16(data30, y_coef0);           //bg
      __m128i   res31   = _mm_madd_epi16(data31, y_coef1);           //ra

      res00             = _mm_add_epi32(res00, res01);
      res10             = _mm_add_epi32(res10, res11);
      res20             = _mm_add_epi32(res20, res21);
      res30             = _mm_add_epi32(res30, res31);
      res00             = _mm_add_epi32(res00, half);
      res10             = _mm_add_epi32(res10, half);
      res20             = _mm_add_epi32(res20, half);
      res30             = _mm_add_epi32(res30, half);
      res00             = _mm_srli_epi32(res00, QN_BITS);
      res10             = _mm_srli_epi32(res10, QN_BITS);
      res20             = _mm_srli_epi32(res20, QN_BITS);
      res30             = _mm_srli_epi32(res30, QN_BITS);
      res00             = _mm_packus_epi32(res00, res10);   //Y, first 8
      res20             = _mm_packus_epi32(res20, res30);   //Y, second 8
      //res00             = _mm_packus_epi16(res00, res20);
      //_mm_storeu_si128((__m128i*)(cur_dst_ptr), res00);

      //A(bg0)(bg2), B(bg0)(bg1)
      res01 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data00), 
                                              _mm_castsi128_ps(data10), 
                                              (2 << 6) | (0 << 4) | (2 << 2) | 0));
      //A(bg1)(bg3), B(bg1)(bg3)
      res10 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data00), 
                                              _mm_castsi128_ps(data10), 
                                              (3 << 6) | (1 << 4) | (3 << 2) | 1));
      
      //C(bg0)(bg2), D(bg0)(bg1)
      res11 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data20), 
                                              _mm_castsi128_ps(data30), 
                                              (2 << 6) | (0 << 4) | (2 << 2) | 0));
      //C(bg1)(bg3), D(bg1)(bg3)
      res21 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data20), 
                                              _mm_castsi128_ps(data30), 
                                              (3 << 6) | (1 << 4) | (3 << 2) | 1));

      //(bg0)(bg1)(bg2)(bg3)
      res01 = _mm_avg_epu16(res01, res10);
      //(bg4)(bg5)(bg6)(bg7)
      res11 = _mm_avg_epu16(res11, res21);


      //A(ra0)(ra2), B(ra0)(ra1)
      res10 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data01), 
                                              _mm_castsi128_ps(data11), 
                                              (2 << 6) | (0 << 4) | (2 << 2) | 0));
      //A(ra1)(ra3), B(ra1)(ra3)
      res21 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data01), 
                                              _mm_castsi128_ps(data11), 
                                              (3 << 6) | (1 << 4) | (3 << 2) | 1));
      
      //C(ra0)(ra2), D(ra0)(ra1)
      res30 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data21), 
                                              _mm_castsi128_ps(data31), 
                                              (2 << 6) | (0 << 4) | (2 << 2) | 0));
      //C(ra1)(ra3), D(ra1)(ra3)
      res31 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(data21), 
                                              _mm_castsi128_ps(data31), 
                                              (3 << 6) | (1 << 4) | (3 << 2) | 1));
      //(ra0)(ra1)(ra2)(ra3)
      res10 = _mm_avg_epu16(res10, res21);
      //(ra4)(ra5)(ra6)(ra7)
      res30 = _mm_avg_epu16(res30, res31);

      __m128i   res_u_00  = _mm_madd_epi16(res01, u_coef0);   //bg_u
      __m128i   res_u_01  = _mm_madd_epi16(res10, u_coef1);   //ra_u
      __m128i   res_u_10  = _mm_madd_epi16(res11, u_coef0);   //bg_u
      __m128i   res_u_11  = _mm_madd_epi16(res30, u_coef1);   //ra_u

      __m128i   res_v_00  = _mm_madd_epi16(res01, v_coef0);   //bg_v
      __m128i   res_v_01  = _mm_madd_epi16(res10, v_coef1);   //ra_v
      __m128i   res_v_10  = _mm_madd_epi16(res11, v_coef0);   //bg_v
      __m128i   res_v_11  = _mm_madd_epi16(res30, v_coef1);   //ra_v

      res_u_00 = _mm_add_epi32(res_u_00, res_u_01);
      res_u_10 = _mm_add_epi32(res_u_10, res_u_11);
      res_v_00 = _mm_add_epi32(res_v_00, res_v_01);
      res_v_10 = _mm_add_epi32(res_v_10, res_v_11);

      res_u_00 = _mm_add_epi32(res_u_00, v_128_half);
      res_u_10 = _mm_add_epi32(res_u_10, v_128_half);
      res_v_00 = _mm_add_epi32(res_v_00, v_128_half);
      res_v_10 = _mm_add_epi32(res_v_10, v_128_half);

      res_u_00 = _mm_srli_epi32(res_u_00, QN_BITS);
      res_u_10 = _mm_srli_epi32(res_u_10, QN_BITS);
      res_v_00 = _mm_srli_epi32(res_v_00, QN_BITS);
      res_v_10 = _mm_srli_epi32(res_v_10, QN_BITS);
#if 0
      //uv0,uv1,uv2,uv3
      res_u_00 = _mm_packus_epi32(res_u_00, res_v_00);
      //uv4,uv5,uv6,uv7
      res_u_10 = _mm_packus_epi32(res_u_10, res_v_10);
#else
      res_u_00 = _mm_max_epi32(res_u_00, m_zero);
      res_u_10 = _mm_max_epi32(res_u_10, m_zero);
      res_v_00 = _mm_max_epi32(res_v_00, m_zero);
      res_v_10 = _mm_max_epi32(res_v_10, m_zero);

      res_v_00 = _mm_slli_epi32(res_v_00,  16);
      res_v_10 = _mm_slli_epi32(res_v_10,  16);
      res_u_00 = _mm_or_si128(res_u_00, res_v_00);
      res_u_10 = _mm_or_si128(res_u_10, res_v_10);
#endif

      //u0y0v0y1, u1y2v1y3
      __m128i res_yuv0 = _mm_unpacklo_epi16(res_u_00, res00);
      //u2y4v2y5, u3y6v3y7
      __m128i res_yuv1 = _mm_unpackhi_epi16(res_u_00, res00);

      //u4y8v4y9,   u5y10v5y11
      __m128i res_yuv2 = _mm_unpacklo_epi16(res_u_10, res20);
      //u6y12v6y13, u7y14v7y15
      __m128i res_yuv3 = _mm_unpackhi_epi16(res_u_10, res20);

      res_yuv0 = _mm_packus_epi16(res_yuv0, res_yuv1);
      res_yuv2 = _mm_packus_epi16(res_yuv2, res_yuv3);

      _mm_storeu_si128((__m128i*)(cur_dst_ptr),      res_yuv0);
      _mm_storeu_si128((__m128i*)(cur_dst_ptr + 16), res_yuv2);

      cur_src_ptr += 48;
      cur_dst_ptr += 32;
    }
    rgb += rgb_linesize;
    yuv += yuv_linesize;
  }
  return 0;
}
#else
static int rgb24_to_uyvy(unsigned char const*      rgb,
                         unsigned int              src_width,
                         unsigned int              src_height,
                         unsigned int              rgb_linesize,
                         unsigned char*            yuv,
                         unsigned int              yuv_linesize,
                         signed short int const*   rgb2yuv_coef){
  int i = 0, j = 0;
  const unsigned int yb      = ((unsigned short int const*)(rgb2yuv_coef))[0];
  const unsigned int yg      = ((unsigned short int const*)(rgb2yuv_coef))[1];
  const unsigned int yr      = ((unsigned short int const*)(rgb2yuv_coef))[2];
  const unsigned int ub      = ((unsigned short int const*)(rgb2yuv_coef))[3];
  const unsigned int ug      = ((unsigned short int const*)(rgb2yuv_coef))[4];
  const unsigned int ur      = ((unsigned short int const*)(rgb2yuv_coef))[5];
  const unsigned int vb      = ((unsigned short int const*)(rgb2yuv_coef))[6];
  const unsigned int vg      = ((unsigned short int const*)(rgb2yuv_coef))[7];
  const unsigned int vr      = ((unsigned short int const*)(rgb2yuv_coef))[8];

  const __m256i  y_coef0     = _mm256_set1_epi32(yb | (yg << 16));
  const __m256i  y_coef1     = _mm256_set1_epi32(yr);

  const __m256i  u_coef0     = _mm256_set1_epi32(ub | (ug << 16));
  const __m256i  u_coef1     = _mm256_set1_epi32(ur);

  const __m256i  v_coef0     = _mm256_set1_epi32(vb | (vg << 16));
  const __m256i  v_coef1     = _mm256_set1_epi32(vr);

  const __m256i  half        = _mm256_set1_epi32(1 << (QN_BITS - 1));
  const __m256i  v_128_half  = _mm256_set1_epi32((128 << QN_BITS) + (1 << (QN_BITS - 1)));
  const __m128i  s_mask0     = _mm_setr_epi8( 0, -1,  1, -1,  3, -1,  4, -1,  6, -1,  7, -1,  9, -1, 10, -1);
  const __m128i  s_mask1     = _mm_setr_epi8( 2, -1, -1, -1,  5, -1, -1, -1,  8, -1, -1, -1, 11, -1, -1, -1);
  const __m128i  s_mask2     = _mm_setr_epi8(12, -1, 13, -1, 15, -1,  0, -1,  2, -1,  3, -1,  5, -1,  6, -1);
  const __m128i  s_mask3     = _mm_setr_epi8(14, -1, -1, -1,  1, -1, -1, -1,  4, -1, -1, -1,  7, -1, -1, -1);
  const __m128i  s_mask4     = _mm_setr_epi8( 8, -1,  9, -1, 11, -1, 12, -1, 14, -1, 15, -1,  1, -1,  2, -1);
  const __m128i  s_mask5     = _mm_setr_epi8(10, -1, -1, -1, 13, -1, -1, -1,  0, -1, -1, -1,  3, -1, -1, -1);
  const __m128i  s_mask6     = _mm_setr_epi8( 4, -1,  5, -1,  7, -1,  8, -1, 10, -1, 11, -1, 13, -1, 14, -1);
  const __m128i  s_mask7     = _mm_setr_epi8( 6, -1, -1, -1,  9, -1, -1, -1, 12, -1, -1, -1, 15, -1, -1, -1);
  const __m128i  blend_mask0 = _mm_setr_epi32(0, 0,          0, 0xFFFFFFFF);
  const __m128i  blend_mask1 = _mm_setr_epi32(0, 0, 0xFFFFFFFF, 0xFFFFFFFF);
  for(i = 0 ; i < src_height ; i ++){
    unsigned char const*  cur_src_ptr = rgb;
    unsigned char*        cur_dst_ptr = yuv;
    for(j = 0 ; j < src_width ; j += 16){
      __m256i data00, data01, data10, data11;
      {
        __m128i d0      = _mm_loadu_si128((const __m128i*)cur_src_ptr);
        __m128i d1      = _mm_loadu_si128((const __m128i*)(cur_src_ptr + 16));
        __m128i d2      = _mm_loadu_si128((const __m128i*)(cur_src_ptr + 32));
        __m128i blend_0 = _mm_blendv_epi8(d1, d0, blend_mask0);
        __m128i blend_1 = _mm_blendv_epi8(d2, d1, blend_mask1);
        __m128i data00_128  = _mm_shuffle_epi8(d0,      s_mask0);           //bg
        __m128i data01_128  = _mm_shuffle_epi8(d0,      s_mask1);           //ra
        __m128i data10_128  = _mm_shuffle_epi8(blend_0, s_mask2);           //bg
        __m128i data11_128  = _mm_shuffle_epi8(blend_0, s_mask3);           //ra
        __m128i data20_128  = _mm_shuffle_epi8(blend_1, s_mask4);           //bg
        __m128i data21_128  = _mm_shuffle_epi8(blend_1, s_mask5);           //ra
        __m128i data30_128  = _mm_shuffle_epi8(d2,      s_mask6);           //bg
        __m128i data31_128  = _mm_shuffle_epi8(d2,      s_mask7);           //ra


        data00 = _mm256_insertf128_si256(data00, data00_128, 0);             //bg
        data01 = _mm256_insertf128_si256(data01, data20_128, 0);             //bg

        data10 = _mm256_insertf128_si256(data10, data01_128, 0);             //ra
        data11 = _mm256_insertf128_si256(data11, data21_128, 0);             //ra

        data00 = _mm256_insertf128_si256(data00, data10_128, 1);             //bg
        data01 = _mm256_insertf128_si256(data01, data30_128, 1);             //bg

        data10 = _mm256_insertf128_si256(data10, data11_128, 1);             //ra
        data11 = _mm256_insertf128_si256(data11, data31_128, 1);             //ra
      }

      __m256i   res00   = _mm256_madd_epi16(data00, y_coef0);           //bg
      __m256i   res01   = _mm256_madd_epi16(data10, y_coef1);           //ra
      __m256i   res10   = _mm256_madd_epi16(data01, y_coef0);           //bg
      __m256i   res11   = _mm256_madd_epi16(data11, y_coef1);           //ra


      res00             = _mm256_add_epi32(res00, res01);
      res10             = _mm256_add_epi32(res10, res11);
      res00             = _mm256_add_epi32(res00, half);
      res10             = _mm256_add_epi32(res10, half);
      res00             = _mm256_srli_epi32(res00, QN_BITS);
      res10             = _mm256_srli_epi32(res10, QN_BITS);
      res00             = _mm256_packus_epi32(res00, res10);   //Y
      //res00             = _mm_packus_epi16(res00, res20);
      //_mm_storeu_si128((__m128i*)(cur_dst_ptr), res00);

      //(bg0)(bg2)(bg8)(bg10), (bg4)(bg6)(bg12)(bg14)
      res01 = _mm256_castps_si256(_mm256_shuffle_ps(_mm256_castsi256_ps(data00), 
                                                    _mm256_castsi256_ps(data01), 
                                                    (2 << 6) | (0 << 4) | (2 << 2) | 0));
      //(bg1)(bg3)(bg9)(bg11), (bg5)(bg7)(bg13)(bg15)
      res10 = _mm256_castps_si256(_mm256_shuffle_ps(_mm256_castsi256_ps(data00), 
                                                    _mm256_castsi256_ps(data01), 
                                                    (3 << 6) | (1 << 4) | (3 << 2) | 1));

      //(bg0)(bg2)(bg8)(bg10)(bg4)(bg6)(bg12)(bg14)
      res01 = _mm256_avg_epu16(res01, res10);

      //(ra0)(ra2)(ra8)(ra10), (ra4)(ra6)(ra12)(ra14)
      res10 = _mm256_castps_si256(_mm256_shuffle_ps(_mm256_castsi256_ps(data10), 
                                                    _mm256_castsi256_ps(data11), 
                                                    (2 << 6) | (0 << 4) | (2 << 2) | 0));
      //(ra1)(ra3)(ra9)(ra11), (ra5)(ra7)(ra13)(ra15)
      res11 = _mm256_castps_si256(_mm256_shuffle_ps(_mm256_castsi256_ps(data10), 
                                                    _mm256_castsi256_ps(data11), 
                                                    (3 << 6) | (1 << 4) | (3 << 2) | 1));

      //(ra0)(ra2)(ra8)(ra10)(ra4)(ra6)(ra12)(ra14)
      res10 = _mm256_avg_epu16(res10, res11);

      __m256i   res_u_00  = _mm256_madd_epi16(res01, u_coef0);   //bg_u
      __m256i   res_u_01  = _mm256_madd_epi16(res10, u_coef1);   //ra_u
      __m256i   res_v_00  = _mm256_madd_epi16(res01, v_coef0);   //bg_v
      __m256i   res_v_01  = _mm256_madd_epi16(res10, v_coef1);   //ra_v

      res_u_00 = _mm256_add_epi32(res_u_00, res_u_01);
      res_v_00 = _mm256_add_epi32(res_v_00, res_v_01);

      res_u_00 = _mm256_add_epi32(res_u_00, v_128_half);
      res_v_00 = _mm256_add_epi32(res_v_00, v_128_half);

      res_u_00 = _mm256_srli_epi32(res_u_00, QN_BITS);
      res_v_00 = _mm256_srli_epi32(res_v_00, QN_BITS);

#if 0
#else
      //(u0)(u1)(u4)(u5)(u2)(u3)(u6)(u7)
      res_u_00 = _mm256_max_epi32(res_u_00, _mm256_setzero_si256());
      //(v0)(v1)(v4)(v5)(v2)(v3)(v6)(v7)
      res_v_00 = _mm256_max_epi32(res_v_00, _mm256_setzero_si256());
      res_v_00 = _mm256_slli_epi32(res_v_00,  16);

      //(uv0)(uv1)(uv4)(uv5)(uv2)(uv3)(uv6)(uv7)
      res_u_00 = _mm256_or_si256(res_u_00, res_v_00);
      //(uv0)(uv1)(uv2)(uv3)(uv4)(uv5)(uv6)(uv7)
      /*
      res_u_00 = _mm256_shuffle_epi32(res_u_00, 
      (7 << 14) | (6 << 12) | (3 << 10) | (2 << 8) | (5 << 6) | (4 << 4) | (1 << 2) | 0);
      */
      res_u_00 = _mm256_set_epi64x(_mm256_extract_epi64(res_u_00, 3),
                                   _mm256_extract_epi64(res_u_00, 1),
                                   _mm256_extract_epi64(res_u_00, 2),
                                   _mm256_extract_epi64(res_u_00, 0));
#endif

      //u0y0v0y1, u1y2v1y3, u2y4v2y5, u3y6v3y7
      __m256i res_yuv0  = _mm256_unpacklo_epi16(res_u_00, res00);
      //u0y0v0y1, u1y2v1y3, u2y4v2y5, u3y6v3y7
      __m256i res_yuv1  = _mm256_unpackhi_epi16(res_u_00, res00);

      __m256i res_yuv00 = _mm256_inserti128_si256(res_yuv0, _mm256_extracti128_si256(res_yuv1, 0), 1);
      __m256i res_yuv10 = _mm256_inserti128_si256(res_yuv1, _mm256_extracti128_si256(res_yuv0, 1), 0);

      res_yuv0 = _mm256_packus_epi16(res_yuv00, res_yuv10);

      _mm256_storeu_si256((__m256i*)(cur_dst_ptr), res_yuv0);

      cur_src_ptr += 48;
      cur_dst_ptr += 32;
    }
    rgb += rgb_linesize;
    yuv += yuv_linesize;
  }
  return 0;
}
#endif

int  rgb2yuyv(unsigned char const*  rgb,
              unsigned int          width,
              unsigned int          height,
              unsigned int          rgb_linesize,
              unsigned char*        yuv,
              unsigned int          yuv_linesize,
              int                   swap_yuv){
  if(0 != (width&15) ||
     0 != (rgb_linesize&15) ||
     0 != (yuv_linesize&15)){
    return -1;
  }

  signed short int  rgb2yuv_coef[] = {
    //Y, coef
    (signed short int)(0.114f * ((float)(1 << QN_BITS)) + 0.5f), 
    (signed short int)(0.587f * ((float)(1 << QN_BITS)) + 0.5f), 
    (signed short int)(0.299f * ((float)(1 << QN_BITS)) + 0.5f),

    //U, coef
     (signed short int)(0.5f   * ((float)(1 << QN_BITS)) + 0.5f), 
    -(signed short int)(0.331f * ((float)(1 << QN_BITS)) + 0.5f), 
    -(signed short int)(0.169f * ((float)(1 << QN_BITS)) + 0.5f), 

    //V, coef
    -(signed short int)(0.081f * ((float)(1 << QN_BITS)) + 0.5f), 
    -(signed short int)(0.419f * ((float)(1 << QN_BITS)) + 0.5f), 
     (signed short int)(0.5f   * ((float)(1 << QN_BITS)) + 0.5f), 
  };

  if(0 == swap_yuv){
    rgb24_to_yuyv(rgb,
                  width,
                  height,
                  rgb_linesize,
                  yuv,
                  yuv_linesize,
                  rgb2yuv_coef);
  }else{
    rgb24_to_uyvy(rgb,
                  width,
                  height,
                  rgb_linesize,
                  yuv,
                  yuv_linesize,
                  rgb2yuv_coef);
  }

  return 0;
}