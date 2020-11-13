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

// References:
//   [1] M. M. Joldes, J.-M. Muller, and V. Popescu, "Tight and rigourous error bounds for basic building blocks of double-word arithmetic," ACM Transactions on Mathematical Software, vol. 44, no. 2, pp. 1-27, 2017.
//   [2] S. Boldo and J.-M. Muller, "Some Functions Computable with a Fused-Mac," in 17th IEEE Symposium on Computer Arithmetic (ARITH'05), Cape Cod, MA, USA, 2005, pp. 52-58.
//   [3] V. Popescu, "Towards fast and certified multiple-precision librairies," Theses, Université de Lyon, 2017
//   [4] C.-P. Jeannerod, J.-M. Muller, and P. Zimmermann, “On Various Ways to Split a Floating-Point Number,” in 2018 IEEE 25th Symposium on Computer Arithmetic (ARITH), Amherst, MA, Jun. 2018, pp. 53–60, doi: 10.1109/ARITH.2018.8464793.

// Do not allow unsafe optimizations when compiling any source which uses this code.

#if !defined(_DD_ARITHMETIC_H)
#define _DD_ARITHMETIC_H 1

#include <float.h>
#include <math.h>

typedef struct {
    double high;
    double low;
} DD;

#define DD_HI(x) ((x).high)
#define DD_LO(x) ((x).low)

// All the following functions are declared "static inline". It is assumed that the compiler will inline them wherever they are called.

// -DD -> DD
static inline DD
negate_DD_DD(const DD x) {
    DD result;
    DD_HI(result) = -DD_HI(x);
    DD_LO(result) = -DD_LO(x);
    return result;
}

// D, D -> DD
static inline DD
load_D_D_DD(const double hi, const double lo) {
    DD result;
    DD_HI(result) = hi;
    DD_LO(result) = lo;
    return result;
}

// DD -> D, D
static inline void
unpack_DD_D_D(const DD x, double* y, double* z) {
    *y = DD_HI(x);
    *z = DD_LO(x);
    return;
}

// Split a double into two doubles using Veltkamp's splitting with FMA.
// See Ref [4], algorithm 4.
static inline void
split_D_D_D(const double x, double* hi, double* lo) {
    const double C1 = -134217728.0; // -2^ceil(DBL_MANT_DIG / 2)
    const double C2 = +134217729.0; //  2^ceil(DBL_MANT_DIG / 2) + 1
    double t = C2 * x;
    *hi = fma(C1, x, t);
    *lo = fma(C2, x, -t); // Same as x - *hi;
    return;
}

// D + D -> DD
// See Ref[1], algorithm 2.
static inline DD
add_D_D_DD (const double x, const double y) {
    DD result;
    DD_HI(result) = x + y;
    DD_LO(result) = (x - (DD_HI(result) - y)) + (y - (DD_HI(result) - (DD_HI(result) - y)));
    return result;
}

// D + D -> DD
// See Ref[1], algorithm 1.
// Assumes |x| >= |y|.
static inline DD
fast_add_D_D_DD (const double x, const double y) {
    DD result;
    DD_HI(result) = x + y;
    DD_LO(result) = y - (DD_HI(result) - x);
    return result;
}

// DD + D -> DD
// See Ref[1], algorithm 4.
static inline DD
add_DD_D_DD (const DD x, const double y) {
    DD s = add_D_D_DD(DD_HI(x), y); 
    DD result = fast_add_D_D_DD(DD_HI(s), DD_LO(x) + DD_LO(s));
    return result;
}

// DD + DD -> DD
// See Ref[1], algorithm 6.
static inline DD
add_DD_DD_DD(const DD x, const DD y) {
    DD result;
    DD s = add_D_D_DD(DD_HI(x), DD_HI(y));
    DD t = add_D_D_DD(DD_LO(x), DD_LO(y));
    double c = DD_LO(s) + DD_HI(t);
    DD v = fast_add_D_D_DD(DD_HI(s), c);
    double w = DD_LO(t) + DD_LO(v);
    result = fast_add_D_D_DD(DD_HI(v), w);
    return result;
}

// D * D -> DD
// See Ref [1], algorithm 3
static inline DD
mul_D_D_DD (const double x, const double y) {
    DD result;
    DD_HI(result) = x * y;
    DD_LO(result) = fma(x, y, -DD_HI(result));
    return result;
}

// D * D -> DD
// Same as mul_D_D_DD except specialized for x = y.
static inline DD
sqr_D_DD (const double x) {
    DD result;
    DD_HI(result) = x * x;
    DD_LO(result) = fma(x, x, -DD_HI(result));
    return result;
}

// DD * D -> DD
// See Ref [1], algorithm 9 or Ref [3], algorithm 12.
// Requires FMA instruction
// USE ALGORITHM 7 IF NO FMA (IT'S MORE ACCURATE THAN ALGORITHM 8 BUT SLOWER)
static inline DD
mul_DD_D_DD (const DD x, const double y) {
    DD c = mul_D_D_DD(DD_HI(x), y);
    DD result = fast_add_D_D_DD(DD_HI(c), fma(DD_LO(x), y, DD_LO(c)));
    return result;
}

//  See Ref. [1], algorithm 12 or Ref. [3], algorithm 15.
static inline DD
mul_DD_DD_DD (const DD x, const DD y) {
    DD c = mul_D_D_DD(DD_HI(x), DD_HI(y));
    double tl0 = DD_LO(x) * DD_LO(y);
    double tl1 = fma(DD_HI(x), DD_LO(y), tl0);
    double cl2 = fma(DD_LO(x), DD_HI(y), tl1);
    double cl3 = DD_LO(c) + cl2;
    DD result = fast_add_D_D_DD(DD_HI(c), cl3);
    return result;
}

// DD / D -> DD
// See Ref [1], algorithm 15.  However, see for corrections to algorithm 15.  (In Ref. [3], it's algorithm 17.)
static inline DD
div_DD_D_DD(const DD x, const double y) {
    DD t;
    DD_HI(t) = DD_HI(x) / y;
    DD pi = mul_D_D_DD (DD_HI(t), y);
    DD delta = load_D_D_DD(DD_HI(x) - DD_HI(pi), DD_LO(x) - DD_LO(pi));
    double delta_1 = DD_HI(delta) + DD_LO(delta);
    DD_LO(t) = delta_1 / y;
    DD result = fast_add_D_D_DD (DD_HI(t), DD_LO(t));
    return result;
}

// DD / DD -> DD
// See Ref [1], algorithm 18.  (Or algorithm 18 in Ref. [3].)
static inline DD
div_DD_DD_DD(const DD x, const DD y) {
    double t = 1.0 / DD_HI(y);
    DD  r;
    DD_HI(r) = fma(-DD_HI(y), t, 1);
    DD_LO(r) = -DD_LO(y) * t;
    DD e = fast_add_D_D_DD (DD_HI(r), DD_LO(r));
    DD delta = mul_DD_D_DD (e, t);
    DD m = add_DD_D_DD (delta, t);
    DD result = mul_DD_DD_DD (x, m);
    return result;
}

#endif // _DD_ARITHMETIC_H
