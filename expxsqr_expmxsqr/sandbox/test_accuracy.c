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

#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpfr.h"

#include "test_accuracy.h"
// #include "DD_arithmetic.h"
#include "utils.h"

#define STRINGIZE_HLPR(A) #A
#define STRINGIZE(A) STRINGIZE_HLPR(A)
#define PPCAT_HLPR(A, B) A ## B
#define PPCAT(A, B) PPCAT_HLPR(A, B)

#if !defined(FUNC_NAME)
#error "No test function named"
#endif
#define FUNC_NAME_STRING STRINGIZE(FUNC_NAME)
#if !defined(MPFR_FUNC_NAME)
#define MPFR_FUNC_NAME PPCAT(mpfr_, FUNC_NAME)
#endif

#define DEFAULT_MPFR_PREC (4 * DBL_MANT_DIG)

static const unsigned int BUFFER_SIZE = 32768;

static inline double
test_function(const double x) {
    extern double FUNC_NAME(const double x);
    return FUNC_NAME(x);
}

static inline void
reference_function(mpfr_ptr result, const double x) {
    extern void MPFR_FUNC_NAME(mpfr_ptr, mpfr_srcptr, mpfr_rnd_t);
    mpfr_t mpfr_x;
    mpfr_inits2(DEFAULT_MPFR_PREC, mpfr_x, (mpfr_ptr)NULL);
    mpfr_set_d(mpfr_x, x, MPFR_RNDN);
    MPFR_FUNC_NAME(result, mpfr_x, MPFR_RNDN);
    mpfr_clears(mpfr_x, (mpfr_ptr)NULL);
    return;
}

static void
doOnePoint(double x) {
    feclearexcept(FE_ALL_EXCEPT);
    double test_result = test_function(x);
    int fp_flags = fetestexcept(FE_ALL_EXCEPT);

    mpfr_t mpfr_result;
    mpfr_inits2(DEFAULT_MPFR_PREC, mpfr_result, (mpfr_ptr)NULL);
    reference_function(mpfr_result, x);

    double lsb_error = compare(mpfr_result, test_result);

    printf("x = %.17e (%.13a)\n", x, x);

    // printf(" ref "FUNC_NAME_STRING" = %.17e (%.13a) correction = %.17e\n", DD_HI(mpfr_result), DD_HI(mpfr_result), DD_LO(mpfr_result));
    double mpfr_result_d = mpfr_get_d(mpfr_result, MPFR_RNDN);
    // mpfr_printf(" ref "FUNC_NAME_STRING" = %.17Re (%.13a)\n", mpfr_result, mpfr_result_d);

    mpfr_t mpfr_temp;
    mpfr_inits2(DEFAULT_MPFR_PREC, mpfr_temp, (mpfr_ptr)NULL);
    mpfr_sub_d(mpfr_temp, mpfr_result, mpfr_result_d, MPFR_RNDN);
    double corr = mpfr_get_d(mpfr_temp, MPFR_RNDN);
    // mpfr_printf(" REF "FUNC_NAME_STRING" = %.13Re (%.13Ra)\n", mpfr_result, mpfr_result);
    // mpfr_printf(" REF "FUNC_NAME_STRING" = %.34Re (%.26Ra)\n", mpfr_result, mpfr_result);
    printf(" ref "FUNC_NAME_STRING" = %.17e (%.13a)  %.17e (%.13a)\n", mpfr_result_d, mpfr_result_d, corr, corr);
    
    printf("test "FUNC_NAME_STRING" = %.17e (%.13a)\n", test_result, test_result);
    printf("error = %.3f ulp\n", lsb_error);
    if (fp_flags != 0) {
        if (fp_flags & FE_DIVBYZERO) printf(" FE_DIVBYZERO");
        if (fp_flags & FE_INEXACT)   printf(" FE_INEXACT");
        if (fp_flags & FE_INVALID)   printf(" FE_INVALID");
        if (fp_flags & FE_OVERFLOW)  printf(" FE_OVERFLOW");
        if (fp_flags & FE_UNDERFLOW) printf(" FE_UNDERFLOW");
        printf("\n");
    } else {
        printf("No FE flags set\n");
    }

    mpfr_clears(mpfr_result, (mpfr_ptr)NULL);
    return;
}

static void
doPoints(char* points[], int n_points) {
    for (int i = 0; i < n_points; i++) {
        double arg = strtod(points[i], NULL);
        if (__builtin_isnan(arg)) {
            printf("Test with SNaN\n");
            doOnePoint(__builtin_nans(""));
        }
        doOnePoint(arg);
    }
    return;
}

int
main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Usage:  %s arg_min arg_max arg_cnt output_filename OR -s arg...\n", argv[0]);
        return -1;
    }
    if (strcmp(argv[ 1 ], "-s") == 0) {
        doPoints(&argv[ 2 ], argc - 2);
        return 0;
    }
    if (argc != 5) {
        printf("Usage:  %s arg_min arg_max arg_cnt output_filename OR -s arg...\n", argv[0]);
        return -1;
    }

    double arg_min = strtod(argv[ 1 ], NULL);
    double arg_max = strtod(argv[ 2 ], NULL);
    int arg_cnt = atoi(argv[ 3 ]);
    char* output_filename = argv[ 4 ];

    double arg_step = (arg_max - arg_min) / (double)arg_cnt;
    if (arg_step <= 0.0) {
        printf("Bad argument range:  min = %e  max = %e  count = %d\n", arg_min, arg_max, arg_cnt);
        return 0;
    }

    FILE* out_datafile = fopen(output_filename, "wb");
    if (out_datafile == NULL) {
        printf("Failed to open output data file\n");
        return -1;
    }

    mpfr_t mpfr_result;
    mpfr_inits2(DEFAULT_MPFR_PREC, mpfr_result, (mpfr_ptr)NULL);
    
    data_point data_buffer[ BUFFER_SIZE ];
    unsigned int index = 0;
    double max_err_ulp = 0.0;
    double max_err_arg = 0.0;
    int correctly_rounded = 0;
    int faithfully_rounded = 0;
    int geq_1_ulp = 0;
    int nans = 0;

    double arg = arg_min;
    for (int i = 1; arg <= arg_max; i++) {
        double test_result = test_function(arg);
        reference_function(mpfr_result, arg);
        double error = compare(mpfr_result, test_result);
        if (isnan(test_result) || isnan(error)) {
            nans++;
            continue;
        }
        if (error > max_err_ulp) {
            max_err_ulp = error;
            max_err_arg = arg;
        }
        if (error <= 0.5) {
            correctly_rounded++;
        } else if (error <= 1.0) {
            faithfully_rounded++;
        } else if (!isnan(error)) {
            geq_1_ulp++;
        } else {
            nans++;
        }
        data_buffer[ index ].arg = arg;
        data_buffer[ index ].ref = mpfr_get_d(mpfr_result, MPFR_RNDN);
        data_buffer[ index ].test = test_result;
        data_buffer[ index ].error = error;
        index++;
        if (index >= BUFFER_SIZE) {
            fwrite(data_buffer, sizeof(data_point), BUFFER_SIZE, out_datafile);
            index = 0;
        }
        arg = arg_min + (double)(i) * arg_step;
    }
    if (index != 0) {
        fwrite(data_buffer, sizeof(data_point), index, out_datafile);
    }
    mpfr_clears(mpfr_result, (mpfr_ptr)NULL);

    fclose(out_datafile);
    printf("arg range: %.18e (%.13a) to %.18e (%.13a)  %d points\n", arg_min, arg_min, arg_max, arg_max, arg_cnt);
    printf("max err = %.3f ulp at x = %.17e (%.13a)\n", max_err_ulp, max_err_arg, max_err_arg);
    printf("Correctly rounded: %d (%.2f)\n", correctly_rounded, 100. * correctly_rounded / arg_cnt);
    printf("Faithfully rounded: %d (%.2f)\n", faithfully_rounded, 100. * faithfully_rounded / arg_cnt);
    printf("Error >= 1 ulp: %d (%.2f)\n", geq_1_ulp, 100. * geq_1_ulp / arg_cnt);
    printf("Nans encountered: %d\n", nans);

    return 0;
}
