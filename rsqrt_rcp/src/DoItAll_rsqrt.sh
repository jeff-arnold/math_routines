#!  /bin/bash
#   -*- mode: shell-script  -*-
#   Time-stamp: "Modified on 30-January-2016 at 15:25:53 by jeff on Jeffs-MacBook-Pro"

#   set -vx

#   Figure out hardware:  AMD or Intel

_compiler=${1:-gcc}
_platform=${2:-Intel}

printf "Running on %s\n\n" $(hostname)
printf "Using compiler %s\n\n" "$(${_compiler} --version)"
printf "PLATFORM = ${_platform}\n"

# make -f Makefile_rsqrt realclean

#   Make all .s files
make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} all_rsqrt_s

#   Quick test
make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} test_rsqrt_1_${_compiler}
./test_rsqrt_1_${_compiler}

#   Get data reference data for 1.0f/std::sqrt(x)
make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} rsqrt_reference_${_compiler}_${_platform}.data

#   Get data for rsqrt_ss(x)
make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} rsqrt_instruction_${_compiler}_${_platform}.data

#   Verify that rsqrt_ss(x) for any float x matches scaled result from rsqrt_ss(x) for x in [1,4) 
make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} verify_1_${_platform}

#   Results for 1.0/sqrt(x) for x in [1,4) when compiled with -Ofast
make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} rsqrt_Ofast_${_compiler}_${_platform}.data

#   Results for vectorized 1.0/sqrt(x) for x in [1,4)
make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} rsqrt_Ofast_v_${_compiler}_${_platform}.data

#   Results for vectorized 1.0/sqrt(x) with improved Newton-Raphson interation for x in [1,4)
make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} rsqrt_Ofast_v_improved_${_compiler}_${_platform}.data

#   Results for vectorized 1.0/sqrt(x) with improved Newton-Raphson interation using FMA for x in [1,4)
make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} rsqrt_Ofast_v_improved_FMA_${_compiler}_${_platform}.data

#   Data file for plotting errors of rsqrt_ss(x) for x in [1,4); also produces information about the behavior of the instruction
#   on the selected platform.
make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} rsqrt_instruction_${_compiler}_${_platform}.plot

#   Data file for plotting errors of Ofast rsqrt for x in [1,4) on the selected platform.
make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} rsqrt_Ofast_${_compiler}_${_platform}.plot

#   Data file for plotting errors of vectorized Ofast rsqrt for x in [1,4) on the selected platform.
make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} rsqrt_Ofast_v_${_compiler}_${_platform}.plot

#   Data file for plotting errors of improved vectorized Ofast rsqrt for x in [1,4) on the selected platform.
make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} rsqrt_Ofast_v_improved_${_compiler}_${_platform}.plot

#   Data file for plotting errors of improved vectorized Ofast rsqrt using FMA for x in [1,4) on the selected platform.
make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} rsqrt_Ofast_v_improved_FMA_${_compiler}_${_platform}.plot

#   Time various implementations
make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} time_rsqrt_${_compiler}
./time_rsqrt_${_compiler}

make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} time_rsqrt_v_${_compiler}
./time_rsqrt_v_${_compiler}

make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} time_rsqrt_v_improved_${_compiler}
./time_rsqrt_v_improved_${_compiler}

make -f Makefile_rsqrt PLATFORM=${_platform} CXX=${_compiler} time_rsqrt_v_improved_FMA_${_compiler}
./time_rsqrt_v_improved_FMA_${_compiler}
