//  -*-  mode: C++  -*-
//  Time-stamp: "Modified on 28-April-2016 at 19:55:36 by jearnold on olhswep03.cern.ch"

//  Compile with -Ofast -mavx -mrecip=div

#include <cmath>
#include <immintrin.h>

__m256
rcp_Ofast_v(const __m256 arg_list) {
    __m256 results;
    for (unsigned int i = 0; i < sizeof(__m256) / sizeof(float); i++) {
        results[ i ] = 1.0f / arg_list[ i ];
    }
    return results;
}
