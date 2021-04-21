//  -*-  mode: C++  -*-
//  Time-stamp: "Modified on 01-October-2015 at 11:14:45 by jeff on Jeffs-MacBook-Pro"

#include <cmath>
#include <immintrin.h>

//  Computes y + 0.5 * y * (1 - (arg * (y * y) ) ) where y = rsqrtss(arg)

float
rsqrt_NR2(const float arg) {
    __m128 arg_list = { 1.0f, 1.0f, 1.0f, 1.0f };
    arg_list[ 0 ] = arg;
    __m128 results = _mm_rsqrt_ss( arg_list );
    float y = results[ 0 ];
    return (y + 0.5 * y * (1.0f - (arg * (y * y))));
}
