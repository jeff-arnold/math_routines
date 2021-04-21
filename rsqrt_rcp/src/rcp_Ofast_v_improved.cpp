//  -*-  mode: C++  -*-
//  Time-stamp: "Modified on 28-April-2016 at 19:57:04 by jearnold on olhswep03.cern.ch"

//  Compile with -Ofast -mavx -mrecip=div.  Optionally, add -mfma.

//  Computes y + y * (1 - arg * y) where y = rcp(arg)

#include <cmath>
#include <immintrin.h>

#if !defined(__FMA__)
#   define ROUTINE_NAME rcp_Ofast_v_improved
#else
#   define ROUTINE_NAME rcp_Ofast_v_improved_FMA
#endif

__m256
ROUTINE_NAME (const __m256 arg_list) {
    const __m256 ones = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    __m256 a = _mm256_rcp_ps(arg_list);
    __m256 b = _mm256_mul_ps(arg_list, a);
    __m256 c = _mm256_sub_ps(ones, b);
    __m256 d = _mm256_mul_ps(a, c);
    __m256 e = _mm256_add_ps(a, d);
    return e;
}
