//  -*-  mode: C;  fill-column: 132  -*-

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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static const unsigned int BUFFER_SIZE = 32768;
typedef struct {
    double arg;
    double ref;
    double test;
    double error;
} data_point;

typedef struct {
    double arg;
    double error;
} bin_data_point;

int
main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage:  %s input_filename output_filename bin_width\n", argv[0]);
        return -1;
    }
    char* input_filename = argv[1];
    char* output_filename = argv[2];
    int bin_width = atoi(argv[3]);

    FILE* input_datafile = fopen(input_filename, "rb");
    if (input_datafile == NULL) {
        printf("Failed to open input data file\n");
        return -1;
    }
    FILE* output_datafile = fopen(output_filename, "wb");
    if (output_datafile == NULL) {
        printf("Failed to open output data file\n");
        return -1;
    }

    int bin_count = 0;
    double first_arg;
    double max_error;

    while (!feof(input_datafile) || !ferror(input_datafile)) {
        data_point input_data_buffer[ BUFFER_SIZE ];
        size_t ret_code = fread(input_data_buffer, sizeof(data_point), BUFFER_SIZE, input_datafile);
        if (ret_code <= 0)
            break;
        for (int i = 0; i < ret_code; i++) {
            double arg = input_data_buffer[i].arg;
            double error = input_data_buffer[i].error;
            if (bin_count == 0) {
                first_arg = arg;
                max_error = fabs(error);
            }
            if (fabs(error) > max_error) {
                max_error = fabs(error);
            }
            bin_count++;
            if (bin_count >= bin_width) {
                bin_data_point bin_data;
                bin_data.arg = first_arg;
                bin_data.error = max_error;
                fwrite(&bin_data, sizeof(bin_data), 1, output_datafile);
                bin_count = 0;
            }
        }
    }
    fclose(input_datafile);
    fclose(output_datafile);
    
    return 0;
}
