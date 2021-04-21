//  -*-  mode: C++;  fill-column: 132  -*-
//  Time-stamp: "Modified on 17-February-2016 at 14:36:29 by jeff on Jeffs-MacBook-Pro"

//  Analyze a datafile and compare it to a reference file.  Select the maximun errors, both absolute (i.e., ulps) and relative, in a
//  bin.

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <istream>
#include <limits>

using namespace std;

const int BIN_WIDTH = 4096;
const int INPUT_DATA_BUFFER_SIZE = 64;

//  Input reference data file format (binary):  result (double) ...
static ifstream ref_datafile;

//  Input experiment data file format (binary):  result (double) ...
static ifstream exp_datafile;

//  Output data file format (text):
//    first_arg (float, %.9e)
//    max_error_rel (float, %.4e)
//    max error_rel_arg (float, %.9e)
//    max_error_ulp (float, %.4e)
//    max_error_ulp_arg (float, %.9e)
static ofstream out_datafile;

static void
print_output_data(const float first_arg,
                  const float max_error_rel,
                  const float max_error_rel_arg,
                  const float max_error_ulp,
                  const float max_error_ulp_arg ) {
    char output_data_buffer[ 500 ];
    sprintf(output_data_buffer, "%.9e %.4e %.9e %.4e %.9e\n", first_arg, max_error_rel, max_error_rel_arg, max_error_ulp, max_error_ulp_arg);
    out_datafile.write(output_data_buffer, strlen(output_data_buffer));
    return;
}

//  denorm_min:  1.401298464e-45  0x1.000000p-149  0x00000001  characteristic:  5.000000000e-01  expo:  -148
//    norm_min:  1.175494351e-38  0x1.000000p-126  0x00800000  characteristic:  5.000000000e-01  expo:  -125
//  min_exponent:  -125  (FLT_MIN_EXP)
//  max_exponent:  +128  (FLT_MAX_EXP)
//        digits: 24  (FLT_MANT_DIG)
//      digits10: 6  (FLT_DIG)
//  max_digits10: 9

static float
ulp (const float x) {
    const int FLT_MANT_DIG = numeric_limits< float >::digits;
    const int FLT_MIN_EXP = numeric_limits< float >::min_exponent;
    int expo;
    frexpf(x, &expo);
    if (expo < FLT_MIN_EXP) {
        expo = FLT_MIN_EXP;
    }
    return ldexpf(1.0f, expo - FLT_MANT_DIG);
}

// //  denorm_min:  4.94065645841246544e-324  0x1.0000000000000p-1074  0x0000000000000001  characteristic:  5.00000000000000000e-01  expo:  -1073
// //    norm_min:  2.22507385850720138e-308  0x1.0000000000000p-1022  0x0010000000000000  characteristic:  5.00000000000000000e-01  expo:  -1021
// //  min_exponent:  -1021  (DBL_MIN_EXP)
// //  max_exponent:  +1024  (DBL_MAX_EXP)
// //        digits: 53  (DBL_MANT_DIG)
// //      digits10: 15  (DBL_DIG)
// //  max_digits10: 17

// static double
// ulp (const double x) {
//     const int DBL_MANT_DIG = numeric_limits< double >::digits;
//     const int DBL_MIN_EXP = numeric_limits< double >::min_exponent;
//     int expo;
//     frexp(x, &expo);
//     if (expo < DBL_MIN_EXP) {
//         expo = DBL_MIN_EXP;
//     }
//     return ldexp(1.0, expo - DBL_MANT_DIG);
// }

//  Assumes index_to_arg(0) is 1.0f;
static float
index_to_arg(const unsigned int index) {
    unsigned int ix;
    float x = 1.0f;
    memcpy(&ix, &x, sizeof(ix));
    ix += index;
    memcpy(&x, &ix, sizeof(x));
    return x;
}

int
main(int argc, char* argv[]) {

    //  pgm_name ref_file exp_file output_file
    //  argc < 4   Error:  not all filenames given
    //  argc == 4  All filenames given
    //  argc > 4   Bad usage

    if (argc != 4) {
        if (argc < 4) {
            cout << "Three filenames required\n";
        } else {
            cout << "Too many arguments\n";
        }
        cout << endl << "Usage:  pgm_name ref_filename exp_filename output_filename" << endl;
        return -1;
    }

    char* ref_filename = argv[ 1 ];
    char* exp_filename = argv[ 2 ];
    char* output_filename = argv[ 3 ];

    ref_datafile.open(ref_filename, ios::in | ios::binary);
    if (!ref_datafile.is_open()) {
        cout << "Failed to open ref datafile" << endl;
        return -1;
    }
    exp_datafile.open(exp_filename, ios::in | ios::binary);
    if (!exp_datafile.is_open()) {
        cout << "Failed to open exp datafile" << endl;
        return -1;
    }
    out_datafile.open(output_filename, ios::out | ios::trunc);
    if (!out_datafile.is_open()) {
        std::cout << "Failed to open output datafile" << std::endl;
        return -1;
    }

    int bin_count = 0;
    unsigned int first_index;
    unsigned int index = 0;
    float max_error_rel;
    unsigned int max_error_rel_index;
    float max_error_ulp;
    unsigned int max_error_ulp_index;

    while (!ref_datafile.eof()) {
        double ref_data_buffer[ INPUT_DATA_BUFFER_SIZE ];
        ref_datafile.read((char*)(&ref_data_buffer), sizeof(ref_data_buffer));
        if (!ref_datafile.good()) {
            break;
        }
        float exp_data_buffer[ INPUT_DATA_BUFFER_SIZE ];
        exp_datafile.read((char*)(&exp_data_buffer), sizeof(exp_data_buffer));
        if (!exp_datafile.good()) {
            break;
        }
        for (unsigned int i = 0; i < INPUT_DATA_BUFFER_SIZE; i++, index++) {
            float rel_error = (float)(abs((ref_data_buffer[ i ] - (double)exp_data_buffer[ i ]) / ref_data_buffer[ i ]));
            float ulp_error = (float)(abs((ref_data_buffer[ i ] - (double)exp_data_buffer[ i ]) / (double)(ulp((float)(ref_data_buffer[ i ])))));
            if (bin_count == 0) {
                //  First data point in a bin.
                first_index = index;
                max_error_ulp = ulp_error;
                max_error_ulp_index = index;
                max_error_rel = rel_error;
                max_error_rel_index = index;
                bin_count++;
                continue;
            }
            if (rel_error > max_error_rel) {
                max_error_rel = rel_error;
                max_error_rel_index = index;
            }
            if (ulp_error > max_error_ulp) {
                max_error_ulp = ulp_error;
                max_error_ulp_index = index;
            }
            bin_count++;
            if (bin_count < BIN_WIDTH) {
                continue;
            }
            //  End of the current bin.
            print_output_data(index_to_arg(first_index), max_error_rel, index_to_arg(max_error_rel_index), max_error_ulp, index_to_arg(max_error_ulp_index));
            //  Initialize for the next bin.
            bin_count = 0;
        }
    }
    ref_datafile.close();
    exp_datafile.close();
    out_datafile.close();
    if (bin_count != 0) {
        print_output_data(index_to_arg(first_index), max_error_rel, index_to_arg(max_error_rel_index), max_error_ulp, index_to_arg(max_error_ulp_index));
    }
    return 0;
}
