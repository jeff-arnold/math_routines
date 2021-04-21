//  -*-  mode: C++;  fill-column: 132  -*-
//  Time-stamp: "Modified on 10-January-2016 at 18:14:25 by jeff on Jeffs-MacBook-Pro"

//  Verify that rcp_ps(x) for x in [1.0,2.0) matches SCALE * rcp_ps(y) for all normal floats.

//  Read each reference result for x in [1.0,2.0), compute all possible Binary32 args, place in a single vector and then call
//  rcp_ps.

//  NOTES:
//      For x > 2^126, the rcp_ps instruction returns 0!

#include <cmath>
#include <fstream>
#include <immintrin.h>
#include <iostream>
#include <limits>

using namespace std;

//  NOTE:  mod(size_of_instr_datafile, INPUT_DATA_BUFFER_SIZE) must be 0.
const int INPUT_DATA_BUFFER_SIZE = 64;

const int ARG_LIST_SIZE = 32;

static const float FLT_MAX = numeric_limits< float >::max();
static const float FLT_MIN = numeric_limits< float >::min();
static const int FLT_MAX_EXP = numeric_limits< float >::max_exponent;
static const int FLT_MIN_EXP = numeric_limits< float >::min_exponent;

int
main(int argc, char* argv[]) {

    //  argc < 2  Error:  no filename given.
    //  argc == 2  Instruction data filename given.
    //  argc > 2   Bad usage.

    if (argc != 2) {
        if (argc < 2) {
            cout << "No instruction datafile name given" << endl;
        } else {
            cout << "Bad usage; too many arguments" << endl;
        }
        cout << "Usage:  pgm_name instruction datafile name" << endl;
        return -1;
    }

    //  Data file format:  result (float) ...
    //  There are 4 * 2^23 = 33554432 bytes of data in the file.
    ifstream instr_datafile;

    const char* instr_data_filename = argv[ 1 ];
    instr_datafile.open(instr_data_filename, ios::in | ios::binary);
    if (!instr_datafile.is_open()) {
        cout << "Failed to open instruction data file" << endl;
        return -1;
    }

    int failure_count = 0;

    float current_arg = 1.0f;

    while (!instr_datafile.eof()) {
        float data_buffer[ INPUT_DATA_BUFFER_SIZE ];
        instr_datafile.read((char*)&data_buffer, sizeof(data_buffer));
        if (!instr_datafile.good()) {
            break;
        }
        for (unsigned int i = 0; i < INPUT_DATA_BUFFER_SIZE; i++) {
            int expo;  //  NOTE:  expo is never used since it's corrected value is always 0 for x in [1,2).
            float characteristic = frexpf(current_arg, &expo);
            //  Correct results from frexpf so that characteristic is in the range [1.0,2.0).  
            characteristic *= 2.0f; //  expo -= 1;
            //  Check all 254 possible results for each float in [1,2).
            //  E.g., 1.0f -> 2^-126, 2^-125, ..., 2^127 etc.
            __m256 arg_list[ ARG_LIST_SIZE ];  //  8 * 32 = 256 floats in 32 __m256 vectors.
            float* ptr = (float*)(&arg_list);
            for (int j = FLT_MIN_EXP - 1; j <= FLT_MAX_EXP - 1; j++) {
                *ptr = ldexpf(characteristic, j);
                ptr++;
            }
            //  Fill two last vector elements.
            *ptr = 1.0f; ptr++; *ptr = 1.0f;
            __m256 results[ ARG_LIST_SIZE ];
            //  Calculate results.
            for (int j = 0; j < ARG_LIST_SIZE; j++) {
                results[ j ] = _mm256_rcp_ps(arg_list[ j ]);
            }
            //  Compare results with reference.
            ptr = (float*)(&results);
            for (int j = FLT_MIN_EXP - 1; j <= FLT_MAX_EXP - 1; j++) {
                //  Scale reference result.
                float expected_result = ldexpf(data_buffer[ i ], -j);
                if (expected_result < FLT_MIN) {
                    expected_result = 0.0f;
                }
                if (*ptr != expected_result) {
                    float actual_arg = ldexpf(characteristic, j);  //  Easier to compute again than to figure out indexing into arg_list.
                    printf("FAILED!  arg: %.6a  expected:  %.6a  actual:  %.6a\n", actual_arg, expected_result, *ptr);
                    printf("%.8e %d %d\n", characteristic, expo, j);
                    failure_count++;
                }
                ptr++;
            }
            current_arg = nextafterf(current_arg, FLT_MAX);
        }
    }
    instr_datafile.close();
    if (failure_count != 0) {
        return 1;
    }
    cout << "PASSED" << endl;
    return 0;
}
