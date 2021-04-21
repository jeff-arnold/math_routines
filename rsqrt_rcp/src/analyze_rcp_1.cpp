//  -*-  mode: C++  -*-
//  Time-stamp: "Modified on 20-February-2016 at 10:52:22 by jeff on Jeffs-MacBook-Pro"

//  Analyze an rcp instruction data file to determine the characteristics of rcp instruction.

#include <cmath>
#include <cstring>
#include <limits>

#include <iostream>
#include <fstream>

using namespace std;

const int INPUT_DATA_BUFFER_SIZE = 64;

//  Input data file format (binary):  arg (float) result (float) ...
static ifstream exp_datafile;
static ifstream ref_datafile;

//  Output data file format (text):
//    first_arg (float, %.9e)
//    result (float, %.9e)
//    bin_width (int, %d)
//    max_error_rel (float, %.4e)
//    max error_rel_arg (float, %.9e)
//    max_error_ulp (float, %.4e)
//    max_error_ulp_arg (float, %.9e)
static ofstream out_datafile;

static void
print_output_data(const float first_arg,
                  const float result,
                  const unsigned int bin_width,
                  const float max_error_rel,
                  const float max_error_rel_arg,
                  const float max_error_ulp,
                  const float max_error_ulp_arg ) {
    char output_data_buffer[ 500 ];
    sprintf(output_data_buffer, "%.9e %.9e %d %.4e %.9e %.4e %.9e\n", first_arg, result, bin_width, max_error_rel, max_error_rel_arg, max_error_ulp, max_error_ulp_arg);
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

    //  argc < 4   Error:  fewer than 3 filenames given
    //  argc == 4  All 3 filenames given
    //  argc > 4   Too many arguments

    if (argc != 4) {
        if (argc < 4) {
            cout << "Three filenames must be specified" << endl;
        } else {
            cout << "Too many arguments" << endl;
        }
        cout << "Usage:  pgm_name ref_filename exp_filename output_filename" << endl;
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

    unsigned int first_index;
    unsigned int index = 0;

    float max_rel_error;
    unsigned int max_rel_error_index;
    float max_ulp_error;
    unsigned int max_ulp_error_index;

    int bin_count;
    int bin_width;
    unsigned int cummulative_OR = 0;
    bool initialized = false;
    float current_result;

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
        for (int i = 0; i < INPUT_DATA_BUFFER_SIZE; i++, index++) {
            float rel_error = (float)(abs((ref_data_buffer[ i ] - (double)exp_data_buffer[ i ]) / ref_data_buffer[ i ]));
            float ulp_error = (float)(abs((ref_data_buffer[ i ] - (double)exp_data_buffer[ i ]) / (double)(ulp((float)(ref_data_buffer[ i ])))));
            cummulative_OR |= *(int*)(&exp_data_buffer[ i ]);
            if (!initialized) {
                //  First data point.
                bin_count = 1;
                bin_width = 1;
                first_index = index;
                current_result = exp_data_buffer[ i ];
                max_rel_error = rel_error;
                max_rel_error_index = index;
                max_ulp_error = ulp_error;
                max_ulp_error_index = index;
                initialized = true;
                continue;
            }
            if (exp_data_buffer[ i ] == current_result) {
                //  The result has not changed:  continue in same bin.
                bin_width++;
                if (rel_error > max_rel_error) {
                    max_rel_error = rel_error;
                    max_rel_error_index = index;
                }
                if (ulp_error > max_ulp_error) {
                    max_ulp_error = ulp_error;
                    max_ulp_error_index = index;
                }
                continue;
            }
            //  The result has changed:  end of the current bin.
            print_output_data(index_to_arg(first_index), current_result, bin_width, max_rel_error, index_to_arg(max_rel_error_index), max_ulp_error, index_to_arg(max_ulp_error_index));
            //  Initialize for the next bin.
            bin_count++;
            bin_width = 1;
            first_index = index;
            current_result = exp_data_buffer[ i ];
            max_rel_error = rel_error;
            max_rel_error_index = index;
            max_ulp_error = ulp_error;
            max_ulp_error_index = index;
         }
    }
    if (bin_width != 0) {
        print_output_data(index_to_arg(first_index), current_result, bin_width, max_rel_error, index_to_arg(max_rel_error_index), max_ulp_error, index_to_arg(max_ulp_error_index));
    }
    ref_datafile.close();
    exp_datafile.close();
    out_datafile.close();
    cout << "Bin count: " << bin_count << endl;
    cout << "Cummulative OR:  " << hex << cummulative_OR << endl;
    return 0;
}
