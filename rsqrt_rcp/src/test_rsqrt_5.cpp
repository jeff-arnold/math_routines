//  -*-  mode: C++;  fill-column: 132  -*-
//  Time-stamp: "Modified on 29-October-2015 at 16:23:54 by jeff on Jeffs-MacBook-Pro"

//  Verify that rsqrt_ps(x) for x in [1.0,4.0) matches SCALE * rsqrt_ps(y) for all normal floats.

//  Read each reference result for x in [1.0,4.0), compute all possible Binary32 args, place in a single vector and then call
//  rsqrt_ps.

#include <cfloat>
#include <cmath>
#include <cstring>
#include <fstream>
#include <immintrin.h>
#include <iostream>

using namespace std;

const int INPUT_DATA_BUFFER_SIZE = 64;

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
    //  There are 4 * ( 2 * 2^23) = 67108864 bytes of data in the file.
    ifstream instr_datafile;

    const char* instr_data_filename = argv[ 1 ];
    instr_datafile.open(instr_data_filename, ios::in | ios::binary);
    if (!instr_datafile.is_open()) {
        cout << "Failed to open instruction data file" << endl;
        return -1;
    }

    int failure_count = 0;
    unsigned int index = 0;

    while (!instr_datafile.eof()) {
        float data_buffer[ INPUT_DATA_BUFFER_SIZE ];
        instr_datafile.read((char*)&data_buffer, sizeof(data_buffer));
        if (!instr_datafile.good()) {
            break;
        }
        for (unsigned int i = 0; i < INPUT_DATA_BUFFER_SIZE; i++, index++) {
            int expo;
            float characteristic = frexpf(index_to_arg(index), &expo);
            //  Correct results from frexpf so that characteristic is in the range [1.0,2.0).  
            characteristic *= 2.0f; expo -= 1;
            //  For each result in the data buffer, check all 127 possible results.
            //  Generate all possible multiples of the argument.
            //  E.g., 1.0f -> 2^-126, 2^-124, ..., 2^126
            //        2.0f -> 2^-125, 2^-123, ..., 2^127
            __m256 arg_list[ 16 ];  //  8 * 16 = 128 floats in 16 __m256 vectors.
            float* ptr = (float*)(&arg_list);
            for (int j = FLT_MIN_EXP - 1; j <= FLT_MAX_EXP - 1; j += 2) {
                *ptr = ldexpf(characteristic, j + expo);
                ptr++;
            }
            *ptr = 1.0f; //  Fill last unused vector element.
            __m256 results[ 16 ];
            //  Calculate results.
            for (int j = 0; j < 16; j++) {
                results[ j ] = _mm256_rsqrt_ps(arg_list[ j ]);
            }
            //  Compare results with reference.
            ptr = (float*)(&results);
            for (int j = FLT_MIN_EXP - 1; j <= FLT_MAX_EXP - 1; j += 2) {
                //  Scale reference result.
                float expected_result = ldexpf(data_buffer[ i ], -j/2);
                if (*ptr != expected_result) {
                    float actual_arg = ldexpf(characteristic, j + expo);  //  Easier to compute again than to figure out indexing into arg_list.
                    printf("FAILED!  arg: %.6a  expected:  %.6a  actual:  %.6a\n", actual_arg, expected_result, *ptr);
                    printf("%.8e %d %d\n", characteristic, expo, j);
                    failure_count++;
                }
                ptr++;
            }
        }
    }
    instr_datafile.close();
    if (failure_count != 0) {
        return 1;
    }
    cout << "PASSED" << endl;
    return 0;
}
