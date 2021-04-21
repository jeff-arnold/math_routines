//  -*-  mode: C++;  fill-column: 132  -*-
//  Time-stamp: "Modified on 28-April-2016 at 16:39:17 by jearnold on olhswep03.cern.ch"

//  Compile with -Ofast -mavx.  If -march=haswell is added, several of the intrinsics will be combined into FMA instructions.

//  This implements a more accurate Newton-Raphson interation:
//      y = rsqrt_ps(arg)
//      result = y + 0.5 * y * (1 - (arg * (y * y) ) )
//  The final addition of a small correction term to y results in an improved estimate of 1/sqrt(x).

#include <cmath>
#include <immintrin.h>

#if defined(__FMA__)
#   define ROUTINE_NAME rsqrt_Ofast_v_improved_FMA
#else
#   define ROUTINE_NAME rsqrt_Ofast_v_improved
#endif

__m256
ROUTINE_NAME (const __m256 arg_list) {
    const __m256 ones = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    const __m256 halves = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    __m256 y = _mm256_rsqrt_ps(arg_list);
    __m256 a = _mm256_mul_ps(y, y);           // a <- y * y
    __m256 b = _mm256_mul_ps(arg_list, a);    // b <- a * arg = arg * (y * y)
    __m256 c = _mm256_sub_ps(ones, b);        // c <- 1.0f - b = 1.0f - arg * (y * y)
    __m256 d = _mm256_mul_ps(y, c);           // d <- y * c = y * (1.0f - arg * (y * y) )
    __m256 e = _mm256_mul_ps(halves, d);      // e <- 0.5f * d = 0.5f * y * (1.0f - arg * (y * y) )
    __m256 f = _mm256_add_ps(y, e);           // f <- y + e = y + 0.5f * y * (1.0f - arg * (y * y) )
    return f;
}
