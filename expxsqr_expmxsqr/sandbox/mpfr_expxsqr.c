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

#include "mpfr.h"

// Using MPFR, compute exp(x^2)
void
mpfr_expxsqr(mpfr_ptr result, mpfr_srcptr mpfr_x, mpfr_rnd_t mpfr_rnd) {
    mpfr_t mpfr_temp;
    mpfr_prec_t mpfr_prec_result = mpfr_get_prec(result);
    mpfr_inits2(2 * mpfr_prec_result, mpfr_temp, (mpfr_ptr)NULL);
    mpfr_sqr(mpfr_temp, mpfr_x, mpfr_rnd);
    mpfr_exp(result, mpfr_temp, mpfr_rnd);
    mpfr_clears(mpfr_temp, (mpfr_ptr)NULL);
    return;
}
