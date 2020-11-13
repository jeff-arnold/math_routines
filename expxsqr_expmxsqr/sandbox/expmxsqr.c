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

// Calculate e^(-x*x)

// Requires FMA instruction.
// Uses #pragma GCC unroll N

// References:
//  [1] S. Boldo, M. Daumas, and R.-C. Li, “Formally Verified Argument Reduction with a Fused Multiply-Add,” IEEE Transactions on Computers, vol. 58, no. 8, pp. 1139–1145, 2009, doi: 10.1109/TC.2008.216.
//  [2] J. M. Muller, Elementary functions: algorithms and implementation, Third edition. Boston: Birkhäuser, 2016.

#include <float.h>
#include <math.h>
#include <stdint.h>

#include "DD_arithmetic.h"

typedef union {
    double d;
    uint64_t ui64;
} IEEE_BIN64_UNION;

// Minimax coefficients of the polynomial x + 0.5 * x^2 + ... to calculate e^x - 1 for |x| <= log(2)/64; max error ~ 9.1e-20
// Note that the array is in reverse order.
static
double coeffs[5] __attribute__((aligned(64))) = {
    0x1.6c16c4e5c2edbp-10, // 1.38888909117700662e-3  N = 6
    0x1.11115b866f5eep-7,  // 8.33336800576042410e-3  N = 5
    0x1.555555555c02fp-5,  // 4.16666666668564708e-2  N = 4
    0x1.5555555548f44p-3,  // 1.66666666665259314e-1  N = 3
    0x1.0000000000000p-1,  // 5.00000000000000000e-1  N = 2
};

// Table of 2^(-j/32) in double-double for j = 0, 1, ... 31.
static
DD power_2[32] __attribute__((aligned(64))) = {
    {0x1.0000000000000p0,   0x0.0000000000000p0},   // 1.0
    {0x1.f50765b6e4540p-1,  0x1.9d3e12dd8a18bp-55}, // 0.97857206208770008970532217063009738922119140625
    {0x1.ea4afa2a490dap-1, -0x1.e9c23179c2893p-55}, // 0.9576032806985737000360359161277301609516143798828125
    {0x1.dfc97337b9b5fp-1, -0x1.1a5cd4f184b5cp-55}, // 0.93708381705514998127881653999793343245983123779296875
    {0x1.d5818dcfba487p-1,  0x1.2ed02d75b3707p-56}, // 0.91700404320467121532800547356600873172283172607421875
    {0x1.cb720dcef9069p-1,  0x1.503cbd1e949dbp-57}, // 0.89735453750155358410012240710784681141376495361328125
    {0x1.c199bdd85529cp-1,  0x1.11065895048ddp-56}, // 0.878126080186649726755376832443289458751678466796875
    {0x1.b7f76f2fb5e47p-1, -0x1.5584f7e54ac3bp-57}, // 0.85930964906123896707157427954371087253093719482421875
    {0x1.ae89f995ad3adp-1,  0x1.7a1cd345dcc81p-55}, // 0.84089641525371450203607537332572974264621734619140625
    {0x1.a5503b23e255dp-1, -0x1.d2f6edb8d41e1p-55}, // 0.82287773907698247288777793073677457869052886962890625
    {0x1.9c49182a3f090p-1,  0x1.c7c46b071f2bep-57}, // 0.8052451659746271417361640487797558307647705078125
    {0x1.93737b0cdc5e5p-1, -0x1.75fc781b57ebcp-58}, // 0.78799042255394324829609331573010422289371490478515625
    {0x1.8ace5422aa0dbp-1,  0x1.6e9f156864b27p-55}, // 0.77110541270397037205697188255726359784603118896484375
    {0x1.82589994cce13p-1, -0x1.d4c1dd41532d8p-55}, // 0.75458221379671142070577616323134861886501312255859375
    {0x1.7a11473eb0187p-1, -0x1.41577ee04992fp-56}, // 0.73841307296974967311342652465100400149822235107421875
    {0x1.71f75e8ec5f74p-1, -0x1.16e4786887a99p-56}, // 0.722590403488523325137293795705772936344146728515625
    {0x1.6a09e667f3bcdp-1, -0x1.bdd3413b26456p-55}, // 0.70710678118654757273731092936941422522068023681640625
    {0x1.6247eb03a5585p-1, -0x1.383c17e40b497p-55}, // 0.69195494098191601128888805760652758181095123291015625
    {0x1.5ab07dd485429p-1,  0x1.6324c054647adp-55}, // 0.67712777346844632564426547105540521442890167236328125
    {0x1.5342b569d4f82p-1, -0x1.07abe1db13cadp-56}, // 0.6626183215798706616084245979436673223972320556640625
    {0x1.4bfdad5362a27p-1,  0x1.d4397afec42e2p-57}, // 0.64841977732550482027562566145206801593303680419921875
    {0x1.44e086061892dp-1,  0x1.89b7a04ef80d0p-60}, // 0.63452547859586660994324347484507597982883453369140625
    {0x1.3dea64c123422p-1,  0x1.ada0911f09ebcp-56}, // 0.6209289060367420010067007751786150038242340087890625
    {0x1.371a7373aa9cbp-1, -0x1.63aeabf42eae2p-55}, // 0.60762367999023447762141358907683752477169036865234375
    {0x1.306fe0a31b715p-1,  0x1.6f46ad23182e4p-56}, // 0.59460355750136051344867382795200683176517486572265625
    {0x1.29e9df51fdee1p-1,  0x1.612e8afad1255p-56}, // 0.58186242938878873776076261492562480270862579345703125
    {0x1.2387a6e756238p-1,  0x1.9b07eb6c70573p-55}, // 0.56939431737834578228785176179371774196624755859375
    {0x1.1d4873168b9aap-1,  0x1.e016e00a2643cp-55}, // 0.5571933712979462161030141942319460213184356689453125
    {0x1.172b83c7d517bp-1, -0x1.19041b9d78a76p-56}, // 0.54525386633262884483741572694270871579647064208984375
    {0x1.11301d0125b51p-1, -0x1.6c51039449b3ap-55}, // 0.53357020033841184858403039470431394875049591064453125
    {0x1.0b5586cf9890fp-1,  0x1.8a62e4adc610bp-55}, // 0.52213689121370687740153471168014220893383026123046875
    {0x1.059b0d3158574p-1,  0x1.d73e2a475b465p-56}, // 0.510948574327058313571114922524429857730865478515625
};
 
double
expmxsqr(const double x) {

    // Screen for special values
    if (isnan(x)) return x + x; // Raise FE_INVALID if x is a signalling NaN.
    DD x_sqr = sqr_D_DD(x);
    // For |x| <  7.4505805969238298e-09 (x^2 < 5.5511151231257852e-17), expmxsqr(x) is 1.0
    if (DD_HI(x_sqr) < 0x1.0000000000002p-54) return 1.0;
    // For |x| > 27.297128403953796 (x^2 > 745.13321910194111), expmxsqr(x) is 0.0.  This also handles the case where |x| == INFINITY.
    if (DD_HI(x_sqr) > 0x1.74910d52d3051p+9) return 0.0;

    // Calculate the reduced argument:  r = x - k * C where C = log(2)/32 and k = nearestint(x/C).  Thus |r| <= log(2)/64.
    // See see Ref [2], section 11.2.2, algorithm 23.  Also see Ref [1], algorithms 5.1 and 5.2.
    const double CONST = 0x1.8p52;                                // 3.0 * 2^(DBL_MANT_DIG - 2) = 6755399441055744.
    const double R = 0x1.71547652b82fep5;                         // 1 / (log(2)/32) = 46.1662413084468283841488300822675228118896484375.
    const double C1 = 0x1.62e42fefa39fp-6;                        // 1/R rounded to DBL_MANT_DIG - 2 digits = 2.1660849392498293664033326422213576734066009521484375e-2.
    const double C2 =  -0x1.950d871319ffp-59;                     // C - C1 = -2.74474482262664548052179828101159533155297914441654583139751366616110317409038543701171875e-18.
    double k_dbl = nearbyint((fma(DD_HI(x_sqr), R, CONST) - CONST)); // Round to nearest integer.
    int k = (int)k_dbl;
    int m = k / 32;
    int j = k % 32;
    double temp1 = fma(-k_dbl, C1, DD_HI(x_sqr));
    double r_hi = fma(-k_dbl, C2, temp1);
    DD temp2 = mul_D_D_DD(k_dbl, C2);
    DD temp3 = fast_add_D_D_DD(temp1, -DD_HI(temp2));
    double r_lo = ((DD_HI(temp3) - r_hi) + DD_LO(temp3)) - DD_LO(temp2);

    // Evaluate e^(-j/32)*e^-r_hi.
    const int N = sizeof(coeffs) / sizeof(coeffs[0]);
    temp1 = coeffs[0];
#if defined(__OPTIMIZE__)
#pragma GCC unroll N
#endif
    for (int i = 1; i < N; i++) {
        temp1 = fma(-r_hi, temp1, coeffs[i]);
    }
    temp1 = -r_hi + (-r_lo + (r_hi * r_hi * temp1)); // temp1 = e^-r_hi - 1.
    DD temp4 = add_DD_D_DD(power_2[j], DD_HI(power_2[j]) * temp1); // temp4 = e^(-j/32) * e^-r_hi.

    // Evaluate e^-(r_lo + DD_LO(x_sqr)).
    double temp6 = r_lo + DD_LO(x_sqr);
    DD temp5 = add_D_D_DD(1.0, -temp6 + 0.5 * temp6 * temp6); // temp5 ~= e^-(r_lo + DD_LO(x_sqr)).

    // Combine to make e^(j-/32)*e^-(r + DD_LO(x_sqr)) = 2^m * e^x_sqr.
    temp4 = mul_DD_DD_DD(temp4, temp5);

    // Apply scale factor of 2^-m carefully so as to properly handle those cases where the result is subnormal.
    const int scale_expo = -1022;
    int mm = (m < -scale_expo) ? m : m + scale_expo; // mm is either m if m < 1022 or m - 1022 if m >= 1022.
    // Multiply by 2^(-mm) by manipulating the exponent field directly.
    // Because 0.5 < temp4 < 2.0, its unbiased exponent is -1, 0 or +1.  Since mm < 1022, subtracting it from the exponent field
    // will not produce a subnormal or zero result.  However, the subsequent multiplication by 2^-1022, if it occurs, may produce
    // such a result.
    IEEE_BIN64_UNION result; // "Safe in C" type-punning.
    result.d = DD_HI(temp4);
    result.ui64 = result.ui64 - ((uint64_t)(mm) << (DBL_MANT_DIG - 1));
    // If m >= 1022, multiply the result by 2^-1022. This may produce a subnormal result.
    if (m >= -scale_expo) result.d = 0x1.0p-1022 * result.d;

    return result.d;
}
