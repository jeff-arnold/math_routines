#!  /bin/bash
#   -*- mode: shell-script  -*-
#   Time-stamp: "Modified on 30-January-2016 at 15:21:00 by jeff on Jeffs-MacBook-Pro"

# set -vx

#   Figure out hardware:  AMD or Intel

_compiler=${1:-gcc}
_platform=${2:-Intel}

printf "Running on %s\n\n" $(hostname)
printf "Using compiler %s\n\n" "$(${_compiler} --version)"
printf "PLATFORM = ${_platform}\n"

# make -f Makefile_rcp realclean

#   Make all .s files
make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} all_rcp_s

#   Quick test
make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} test_rcp_1_${_compiler}
./test_rcp_1_${_compiler}

#   Get data reference data for 1.0f/std::sqrt(x)
make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} rcp_reference_${_compiler}_${_platform}.data

#   Get data for rcp_ss(x)
make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} rcp_instruction_${_compiler}_${_platform}.data

#   Verify that rcp_ss(x) for any float x matches scaled result from rcp_ss(x) for x in [1,4) 
make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} verify_1_${_platform}

#   Results for 1.0/sqrt(x) for x in [1,4) when compiled with -Ofast
make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} rcp_Ofast_${_compiler}_${_platform}.data

#   Results for vectorized 1.0/sqrt(x) for x in [1,4)
make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} rcp_Ofast_v_${_compiler}_${_platform}.data

#   Results for vectorized 1.0/sqrt(x) with improved Newton-Raphson interation for x in [1,4)
make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} rcp_Ofast_v_improved_${_compiler}_${_platform}.data

#   Results for vectorized 1.0/sqrt(x) with improved Newton-Raphson interation using FMA for x in [1,4)
make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} rcp_Ofast_v_improved_FMA_${_compiler}_${_platform}.data

#   Data file for plotting errors of rcp_ss(x) for x in [1,4); also produces information about the behavior of the instruction
#   on the selected platform.
make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} rcp_instruction_${_compiler}_${_platform}.plot

#   Data file for plotting errors of Ofast rcp for x in [1,4) on the selected platform.
make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} rcp_Ofast_${_compiler}_${_platform}.plot

#   Data file for plotting errors of vectorized Ofast rcp for x in [1,4) on the selected platform.
make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} rcp_Ofast_v_${_compiler}_${_platform}.plot

#   Data file for plotting errors of improved vectorized Ofast rcp for x in [1,4) on the selected platform.
make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} rcp_Ofast_v_improved_${_compiler}_${_platform}.plot

#   Data file for plotting errors of improved vectorized Ofast rcp using FMA for x in [1,4) on the selected platform.
make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} rcp_Ofast_v_improved_FMA_${_compiler}_${_platform}.plot

#   Time various implementations
make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} time_rcp_${_compiler}
./time_rcp_${_compiler}

make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} time_rcp_v_${_compiler}
./time_rcp_v_${_compiler}

make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} time_rcp_v_improved_${_compiler}
./time_rcp_v_improved_${_compiler}

make -f Makefile_rcp PLATFORM=${_platform} CXX=${_compiler} time_rcp_v_improved_FMA_${_compiler}
./time_rcp_v_improved_FMA_${_compiler}
