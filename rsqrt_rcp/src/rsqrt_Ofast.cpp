//  -*-  mode: C++;  fill-column: 132  -*-
//  Time-stamp: "Modified on 20-January-2016 at 17:46:38 by jeff on Jeffs-MacBook-Pro"

//  Compile with -Ofast

//  The generated code should consist of an rsqrt_ss(arg) instruction followed by a Newton-Raphson interation:
//      y = rsqrt_ss(arg)
//      result = (0.5 * y) * (3.0 - (y * (y * arg) ) )

#include <cmath>

float
rsqrt_Ofast(const float arg) {
    return (1.0f / std::sqrt(arg));
}
