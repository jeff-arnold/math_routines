//  -*-  mode: C++  -*-
//  Time-stamp: "Modified on 28-January-2016 at 11:15:28 by jeff on Jeffs-MacBook-Pro"

#include <cmath>
#include <immintrin.h>

//  Emulate gcc 5.2.0; compile with -Ofast -mrecip but with a more accurate computation of the Newton-Raphson interation.
//  Computes y + y * (1 - (arg * y) ) where y = rcpss(arg)

float
rcp_NR2(const float arg) {
    __m128 arg_list = { 1.0f, 1.0f, 1.0f, 1.0f };
    arg_list[ 0 ] = arg;
    __m128 results = _mm_rcp_ss( arg_list );
    float y = results[ 0 ];
    return (y + (y * (1.0f - (arg * y))));
}
