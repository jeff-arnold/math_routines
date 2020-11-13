// -*-  mode: C; tab-width:4; fill-column: 132; comment-start:  "// "; comment-end:  ""; coding: utf-8;  -*-

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

#if  !defined(_UTILS_H)
#define _UTILS_H 1

#include "DD_arithmetic.h"

int getexpo(const double x);
float ulpf (const float x);
double ulp (const double x);
double compare(mpfr_srcptr ref, const double test);

#define COMPARE_CORRECTLY_ROUNDED (1)
#define COMPARE_FAITHFULLY_ROUNDED (2)
#define COMPARE_GEQ_1_ROUNDING (3)
#define COMPARE_ERROR (4)

#endif // _UTILS_H
