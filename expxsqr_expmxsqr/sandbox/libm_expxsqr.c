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

// Calculate e^(x*x) using the libm exp function; optionally, split x^2 into high and low parts.

#if !defined(USE_SPLIT)
#include <math.h>
#else
#include "DD_arithmetic.h"
#endif

double
libm_expxsqr(const double arg) {
#if !defined(USE_SPLIT)
    return exp(arg*arg);
#else
    DD arg_sqr = sqr_D_DD(arg);
    return exp(DD_HI(arg_sqr)) * exp(DD_LO(arg_sqr));
#endif
}
