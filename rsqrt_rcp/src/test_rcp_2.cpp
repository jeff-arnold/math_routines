//  -*-  mode: C++;  fill-column: 132  -*-
//  Time-stamp: "Modified on 28-December-2015 at 17:54:00 by jearnold on olhswep03.cern.ch"

//  Generate a datafile of the results of 1.0/x for x in [1,2).  These are the "reference" values.

//  The output file consists of 2^23 double-precision floating-point numbers corresponding to the values of 1.0/x for
//  each single-precision floating-point number in [1,2).  E.g., file_value = 1.0/(double)(x) for float x in [1,2).

#include <cmath>
#include <fstream>
#include <immintrin.h>
#include <iostream>
#include <limits>

using namespace std;

const unsigned int BUFFER_SIZE = 8192;
const float FLT_MAX = numeric_limits< float >::max();
const float FLT_MIN = numeric_limits< float >::min();

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
        cout << "Usage:  pgm_name out_filename" << endl;
        return -1;
    }

    //  Data file format:  result (double) ...
    //  There are 8 * 2^23 = 67108864 bytes of data in the file.
    ofstream out_datafile;

    const char* data_filename = argv[ 1 ];
    out_datafile.open(data_filename, ios::out | ios::binary | ios::trunc);
    if (!out_datafile.is_open()) {
        cout << "Failed to open output datafile" << endl;
        return -1;
    }

    double data_buffer[ BUFFER_SIZE ];
    unsigned int index = 0;

    const __m256d ones = {1.0, 1.0, 1.0, 1.0};

    float first_arg = 1.0f;
    float last_arg = nextafterf(2.0f, FLT_MIN);

    __m256d arg_list;
    float current_arg = first_arg;
    arg_list[ 0 ] = (double)current_arg;

    while (current_arg <= last_arg) {
        for (unsigned int i = 1; i < sizeof(arg_list) / sizeof(double); i++) {
            current_arg = nextafterf(current_arg, FLT_MAX);
            arg_list[ i ] = (double)current_arg;
        }
        __m256d results = _mm256_div_pd(ones, arg_list);
        for (unsigned int i = 0; i < sizeof(results) / sizeof(double); i++) {
            data_buffer[ index++ ] = results[ i ];
        }
        if (index >= BUFFER_SIZE) {
            out_datafile.write((char*)data_buffer, sizeof(data_buffer));
            index = 0;
        }
        current_arg = nextafterf(current_arg, FLT_MAX);
        arg_list[ 0 ] = (double)current_arg;
    }
    out_datafile.close();
    if (index != 0) {
        cout << "index not zero at end of program" << endl;
        return 1;
    }
    return 0;
}
