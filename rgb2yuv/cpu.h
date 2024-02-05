//==============================================================================
// https://github.com/metarutaiga/xxYUV
//==============================================================================
#pragma once
#include <stdio.h>
#include <stdint.h>

bool is_ssse3();
bool is_avx2();
bool is_avx512bw();
bool is_addr_aligned(const void* __restrict ptr, const size_t byte_count);
//------------------------------------------------------------------------------