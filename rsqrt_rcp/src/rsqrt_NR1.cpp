//  -*-  mode: C++  -*-
//  Time-stamp: "Modified on 28-January-2016 at 10:41:21 by jeff on Jeffs-MacBook-Pro"

#include <cmath>
#include <immintrin.h>

//  Emulate gcc 5.2.0
//  Computes (-0.5 * y) * (-3 + (y * (y * arg) ) ) where y = rsqrtss(arg)

float
rsqrt_NR1(const float arg) {
    __m128 arg_list = {1.0f, 1.0f, 1.0f, 1.0f};
    arg_list[ 0 ] = arg;
    __m128 results = _mm_rsqrt_ss( arg_list );
    float y = results[ 0 ];
    return ((-0.5 * y) * (-3.0f + (y * (y * arg))));
}
