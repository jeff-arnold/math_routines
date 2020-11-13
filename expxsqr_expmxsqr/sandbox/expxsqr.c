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

// Calculate e^(x*x)

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

// Minimax coefficients of the polynomial x + 0.5 * x^2 + ... to calculate e^x - 1 for |x| ~< log(2)/64; max error ~ 9.1e-20
// Note that the array is in reverse order.
static
double coeffs[5] __attribute__((aligned(64))) = {
    0x1.6c16c4e5c2edbp-10, // 1.38888909117700662e-3  N = 6
    0x1.11115b866f5eep-7,  // 8.33336800576042410e-3  N = 5
    0x1.555555555c02fp-5,  // 4.16666666668564708e-2  N = 4
    0x1.5555555548f44p-3,  // 1.66666666665259314e-1  N = 3
    0x1.0000000000000p-1,  // 5.00000000000000000e-1  N = 2
};

// Table of 2^(j/32) in doubledouble for j = 0, 1, ... 31.
static
DD power_2[32] __attribute__((aligned(64))) = {
    {0x1.0000000000000p0,   0x0.0000000000000p0},   // 1.0
    {0x1.059b0d3158574p0,   0x1.d73e2a475b465p-55}, // 1.02189714865411667823448013478329865135948390552117137111188911930727274413044369794079102575778961181640625
    {0x1.0b5586cf9890fp0,   0x1.8a62e4adc610bp-54}, // 1.0442737824274138403219664787399333395756837043858642647451456689049187342988034288282506167888641357421875
    {0x1.11301d0125b51p0,  -0x1.6c51039449b3ap-54}, // 1.067140400676823618169521120992806674867648982094983365047713235428117339864684254280291497707366943359375
    {0x1.172b83c7d517bp0,  -0x1.19041b9d78a76p-55}, // 1.0905077326652576592070106557607059619378242300539842806908640460324122489765841237385757267475128173828125
    {0x1.1d4873168b9aap0,   0x1.e016e00a2643cp-54}, // 1.11438674259589253630881295691960159209878389548116926603083475487421249994213212630711495876312255859375
    {0x1.2387a6e756238p0,   0x1.9b07eb6c70573p-54}, // 1.1387886347566916537038302838415132621348759387615339330966556723005733697817731808754615485668182373046875
    {0x1.29e9df51fdee1p0,   0x1.612e8afad1255p-55}, // 1.16372485877757751381357359909218459263323319712506847222383121590906507325513530304306186735630035400390625
    {0x1.306fe0a31b715p0,   0x1.6f46ad23182e4p-55}, // 1.189207115002721066717499970560474773333315207010475316930993056226129045427342134644277393817901611328125
    {0x1.371a7373aa9cbp0,  -0x1.63aeabf42eae2p-54}, // 1.215247359980468878116520251338793740417660344343095710747381927967214476637991538154892623424530029296875
    {0x1.3dea64c123422p0,   0x1.ada0911f09ebcp-55}, // 1.241857812073484048593677468726597919910144782992293380011409177458847619135440254467539489269256591796875
    {0x1.44e086061892dp0,   0x1.89b7a04ef80d0p-59}, // 1.26905095719173322255441908103233805488765523241270641061315227839968733380970888902083970606327056884765625
    {0x1.4bfdad5362a27p0,   0x1.d4397afec42e2p-56}, // 1.29683955465100966593375411779245099115709868759247192343833928016190448762046116826240904629230499267578125
    {0x1.5342b569d4f82p0,  -0x1.07abe1db13cadp-55}, // 1.32523664315974129462953709549872091209174203063984378736264884194283319995832925997092388570308685302734375
    {0x1.5ab07dd485429p0,   0x1.6324c054647adp-54}, // 1.3542555469368927282980147401407050450916257598394039915778826123522737834292684055981226265430450439453125
    {0x1.6247eb03a5585p0,  -0x1.383c17e40b497p-54}, // 1.3839098819638319548726595272651922920481841178771973893956524618610581800481895697885192930698394775390625
    {0x1.6a09e667f3bcdp0,  -0x1.bdd3413b26456p-54}, // 1.414213562373095048801688724209693939894363175963335998701832945702305810442567235440947115421295166015625
    {0x1.71f75e8ec5f74p0,  -0x1.16e4786887a99p-55}, // 1.44518080697704662003700624147167267869039863050317396063176647309604649915826257711159996688365936279296875
    {0x1.7a11473eb0187p0,  -0x1.41577ee04992fp-55}, // 1.47682614593949931138690748037405004508134151527662272305085045778945407324300731488619931042194366455078125
    {0x1.82589994cce13p0,  -0x1.d4c1dd41532d8p-54}, // 1.5091644275934227397660195510331933272280355048675691299401289448278706828432405018247663974761962890625
    {0x1.8ace5422aa0dbp0,   0x1.6e9f156864b27p-54}, // 1.5422108254079408236122918620907357573024506717288439856794389991832827657702864598832093179225921630859375
    {0x1.93737b0cdc5e5p0,  -0x1.75fc781b57ebcp-57}, // 1.57598084510788648645527016018190446499171206664148557502544621103553190710755416148458607494831085205078125
    {0x1.9c49182a3f090p0,   0x1.c7c46b071f2bep-56}, // 1.61049033194925430817952066735739951377465936223897914369579330534469618907422727716038934886455535888671875
    {0x1.a5503b23e255dp0,  -0x1.d2f6edb8d41e1p-54}, // 1.6457554781539648445187567247258231195058161695956003502688637699612195230969291515066288411617279052734375
    {0x1.ae89f995ad3adp0,   0x1.7a1cd345dcc81p-54}, // 1.6817928305074290860622509524664246865648737966202232310082144797237779787479894366697408258914947509765625
    {0x1.b7f76f2fb5e47p0,  -0x1.5584f7e54ac3bp-56}, // 1.718619298122477915629344376456311862953238692694799428012951807436155260422339097203803248703479766845703125
    {0x1.c199bdd85529cp0,   0x1.11065895048ddp-55}, // 1.75625216037329948311216061937531198781207486594875267496506138936197827238316904185921885073184967041015625
    {0x1.cb720dcef9069p0,   0x1.503cbd1e949dbp-56}, // 1.794709075003107186427703242127780392590101493494374820660256341221631959381710430534440092742443084716796875
    {0x1.d5818dcfba487p0,   0x1.2ed02d75b3707p-55}, // 1.83400808640934246348708318958828949858624512817790771672885267030810030330911786222713999450206756591796875
    {0x1.dfc97337b9b5fp0,  -0x1.1a5cd4f184b5cp-54}, // 1.87416763411029990132999894995444124855382223821278817442950053763495166236907607526518404483795166015625
    {0x1.ea4afa2a490dap0,  -0x1.e9c23179c2893p-54}, // 1.9152065613971472938726112702958339456200332044370186797392979706662619054924334704992361366748809814453125
    {0x1.f50765b6e4540p0,   0x1.9d3e12dd8a18bp-54}, // 1.9571441241754002690183222516268724544578878887912042076490756639488288737283028240199200809001922607421875
};

double
expxsqr(const double x) {

    // Screen for special values
    if (isnan(x)) return x + x; // Raise FE_INVALID if x is a signalling NaN.
    DD x_sqr = sqr_D_DD(x);
    // For |x| < 1.0536712127723509e-08 (x^2 < 1.1102230246251568e-16), expxsqr(x) is 1.0
    if (DD_HI(x_sqr) < 0x1.0000000000001p-53) return 1.0;
    // For |x| > 26.641747557046326 (X^2 > 709.78271289338386), expxsqr(x) is Inf.
    if (DD_HI(x_sqr) > 0x1.62e42fefa39eep+9) return INFINITY;

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

    // Evaluate e^(j/32)*e^r_hi.
    const int N = sizeof(coeffs) / sizeof(coeffs[0]);
    temp1 = coeffs[0];
#if defined(__OPTIMIZE__)
#pragma GCC unroll N
#endif
    for (int i = 1; i < N; i++) {
        temp1 = fma(r_hi, temp1, coeffs[i]);
    }
    temp1 = r_hi + (r_lo + (r_hi * r_hi * temp1)); // temp1 = e^r_hi - 1.
    DD temp4 = add_DD_D_DD(power_2[j], DD_HI(power_2[j]) * temp1); // temp4 = e^(j/32) * e^r_hi.

    // Evaluate e^(r_lo + DD_LO(x_sqr)).
    double temp6 = r_lo + DD_LO(x_sqr);
    DD temp5 = add_D_D_DD(1.0, temp6 + 0.5 * temp6 * temp6); // temp5 ~= e^(r_lo + DD_LO(x_sqr)).

    // Combine to make e^(j/32)*e^(r + DD_LO(x_sqr)) = 2^-m * x^x_sqr.
    temp4 = mul_DD_DD_DD(temp4, temp5); // Note:  only the high part of temp4 will be used.

    // Apply the scale factor 2^m.
    // Since values of |x| for which expxsqr(x) overflows have been screened out, this manipulation will produce a finite value.
    IEEE_BIN64_UNION result; // "Safe in C" type-punning.
    result.d = DD_HI(temp4);
    result.ui64 = result.ui64 + ((uint64_t)(m) << (DBL_MANT_DIG - 1));

    return result.d;
}
