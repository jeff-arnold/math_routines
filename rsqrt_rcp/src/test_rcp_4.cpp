//  -*-  mode: C++;  fill-column: 132  -*-
//  Time-stamp: "Modified on 28-April-2016 at 19:51:28 by jearnold on olhswep03.cern.ch"

//  The output file consists of 2^23 single-precision floating-point numbers corresponding to the values of various forms of the
//  "Ofast rcp" function for each single-precision floating-point number in [1,2).

#include <cmath>
#include <fstream>
#include <immintrin.h>
#include <iostream>
#include <limits>

using namespace std;

const unsigned int BUFFER_SIZE = 8192;
const float FLT_MAX = numeric_limits< float >::max();
const float FLT_MIN = numeric_limits< float >::min();

//  External routines:
#if !defined(VECTOR)
    float rcp_Ofast(float x);
#elif !defined(IMPROVED)
    __m256 rcp_Ofast_v(const __m256 arg_list);
#elif !defined(FMA)
    __m256 rcp_Ofast_v_improved(const __m256 arg_list);
#else
    __m256 rcp_Ofast_v_improved_FMA(const __m256 arg_list);
#endif

int
main(int argc, char* argv[]) {

    //  argc < 2  Error:  no output filename given.
    //  argc == 2  Output filename given
    //  argc > 2   Too many arguments.

    if (argc != 2 ) {
        if (argc < 2 ) {
            cout << "No filename given" << endl;
        } else {
            cout << "Too many arguments" << endl;
        }
        cout << "Usage:  pgm_name output_filename" << endl;
        return -1;
    }

    //  Data file format:  result (float) ...
    //  There are 4 * 2^23 = 67108864 bytes of data in the file.
    ofstream output_datafile;

    const char* output_datafile_name = argv[ 1 ];
    output_datafile.open(output_datafile_name, ios::out | ios::binary | ios::trunc);
    if (!output_datafile.is_open()) {
        cout << "Failed to open output datafile" << endl;
        return -1;
    }

    float data_buffer[ BUFFER_SIZE ];
    unsigned int index = 0;

    float first_arg = 1.0f;
    float last_arg = nextafterf(2.0f, FLT_MIN);
    float current_arg = first_arg;

    __m256 arg_list;
    arg_list[ 0 ] = current_arg;

    while (current_arg <= last_arg) {
        for (unsigned int i = 1; i < sizeof(arg_list) / sizeof(float); i++) {
            current_arg = nextafterf(current_arg, FLT_MAX);
            arg_list[ i ] = current_arg;
        }

#if !defined(VECTOR)
        for (unsigned int i = 0; i < sizeof(arg_list) / sizeof(float); i++) {
            data_buffer[ index++ ] = rcp_Ofast(arg_list[ i ]);
        }
#else
        __m256 results;
#if !defined(IMPROVED)
        results = rcp_Ofast_v(arg_list);
#elif !defined(FMA)
        results = rcp_Ofast_v_improved(arg_list);
#else
        results = rcp_Ofast_v_improved_FMA(arg_list);
#endif
        for (unsigned int i = 0; i < sizeof(results) / sizeof(float); i++) {
            data_buffer[ index++ ] = results[ i ];
        }
#endif
        if (index >= BUFFER_SIZE) {
            output_datafile.write((char*)data_buffer, sizeof(data_buffer));
            index = 0;
        }
        current_arg = nextafterf(current_arg, FLT_MAX);
        arg_list[ 0 ] = current_arg;
    }
    output_datafile.close();
    if (index != 0) {
        cout << "index not zero at end of program" << endl;
        return 1;
    }
    return 0;
}
