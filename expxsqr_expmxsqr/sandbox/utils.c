// -*-  mode: C;  fill-column: 132  comment-start:  "// "  comment-end:  ""  coding: utf-8  -*-

// ****************************************************************
// * Copyright (C) 2020  J.M. Arnold  jearnold <at> cern <dot> ch *
// ****************************************************************

// *****************************************************************************
// This program is free software: you can redistribute it and/or modify it     *
// under the terms of the GNU Lesser General Public License as published by    *
// the Free Software Foundation, either version 2 of the License, or (at your  *
// option) any later version.                                                  *
//                                                                             *
// This program is distributed in the hope that it will be useful, but WITHOUT *
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public        *
// License for more details.                                                   *
//                                                                             *
// You should have received a copy of the GNU Lesser General Public License    *
// along with this program.  If not, see <https://www.gnu.org/licenses/>.      *
// *****************************************************************************

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>

#include "mpfr.h"
#include "utils.h"
#include "DD_arithmetic.h"

typedef union {
    double d;
    uint64_t ui64;
} IEEE_BIN64_UNION;

int
getexpo(const double x) {
    const IEEE_BIN64_UNION t1 = {.d = x};
    const IEEE_BIN64_UNION t2 = {.d = INFINITY};
    return (int)((t1.ui64 & t2.ui64) >> (DBL_MANT_DIG - 1));
}

// If 2^a <= |x| < 2^(a+1), then ulp(x) = 2^a' where a' = max(a, e_min)-p+1.  For IEEE Binary64, a' = max(a, -1022) - 52.  ulp(0.0) is 2^(e_min-p+1) = 2^(-1074).
// Note:  y = frexp(x, &expo) returns y and expo such that 0.5 <= |y| < 1.0 and x == y * 2^expo.  Thus the use of DBL_MANT_DIG (which is p) rather than p - 1.
// Also, if x == 0.0, expo is set to 0.
float
ulpf (const float x) {
    int class_x = fpclassify(x);
    if (class_x == FP_NAN || class_x == FP_ZERO || class_x == FP_INFINITE)
        return NAN;
    int expo;
    if (class_x != FP_SUBNORMAL)
        frexpf(x, &expo);
    else
        expo = FLT_MIN_EXP;
    return ldexpf(1.0f, expo - FLT_MANT_DIG);
}

double
ulp (const double x) {
    int class_x = fpclassify(x);
    if (class_x == FP_NAN || class_x == FP_INFINITE)
        return NAN;
    int expo;
    if (class_x == FP_NORMAL)
        frexp(x, &expo);
    else // class_x == FP_SUBNORMAL || class_x == ZERO
        expo = DBL_MIN_EXP;
    return ldexp(1.0, expo - DBL_MANT_DIG);
}

// Calculate |ulp error| of a test result by comparing it to an MPFR reference value.
double
compare(mpfr_srcptr ref, const double test) {
    double ref_d = mpfr_get_d(ref, MPFR_RNDN);
    if (isnan(ref_d) && isnan(test)) return 0.0;
    if (isnan(ref_d) || isnan(test)) return NAN;
    if (isinf(ref_d) && isinf(test) && (ref_d * test >= 0.0)) return 0.0;
    if (isinf(ref_d) || isinf(test)) return INFINITY;
    // Calculate (double)((ref - test) / ulp(ref)).
    mpfr_t mpfr_temp;
    mpfr_inits2(mpfr_get_prec(ref), mpfr_temp, (mpfr_ptr)NULL);
    mpfr_sub_d(mpfr_temp, ref, test, MPFR_RNDN);
    mpfr_div_d(mpfr_temp, mpfr_temp, ulp(ref_d), MPFR_RNDN);
    double ulp_error = mpfr_get_d(mpfr_temp, MPFR_RNDN);
    return fabs(ulp_error);
}
