//  -*-  mode: C++;  fill-column: 132  -*-
//  Time-stamp: "Modified on 28-April-2016 at 16:38:43 by jearnold on olhswep03.cern.ch"

//  Compile with -Ofast -mavx

//  The generated code should consist of an rsqrt_ps(arg) instruction followed by a vectorized Newton-Raphson interation:
//      y = rsqrt_ps(arg)
//      result = (0.5 * y) * (3.0 - (y * (y * arg) ) )


#include <cmath>
#include <immintrin.h>

__m256
rsqrt_Ofast_v(const __m256 arg_list) {
    __m256 results;
    for (unsigned int i = 0; i < sizeof(__m256) / sizeof(float); i++) {
        results[ i ] = 1.0f / std::sqrt(arg_list[ i ]);
    }
    return results;
}
