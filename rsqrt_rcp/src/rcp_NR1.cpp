//  -*-  mode: C++  -*-
//  Time-stamp: "Modified on 28-January-2016 at 11:33:31 by jeff on Jeffs-MacBook-Pro"

#include <cmath>
#include <immintrin.h>

//  Emulate gcc 5.2.0; compile with -Ofast -mrecip.
//  Computes (y + y) - (y * (y * arg) ) where y = rcpss(arg)

float
rcp_NR1(const float arg) {
    __m128 arg_list = {1.0f, 1.0f, 1.0f, 1.0f};
    arg_list[ 0 ] = arg;
    __m128 results = _mm_rcp_ss( arg_list );
    float y = results[ 0 ];
    return (y + y) - (y * (y * arg));
}
