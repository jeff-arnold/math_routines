//  -*-  mode: C++;  fill-column: 132  -*-
//  Time-stamp: "Modified on 28-April-2016 at 17:40:16 by jearnold on olhswep03.cern.ch"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <immintrin.h>

static const uint64_t ITERATIONS = 2 * 1100 * 1000 * 1000ULL;

//  External routines:
#if !defined(VECTOR)
    static const double results_per_call = 1.0;
    float rsqrt_Ofast(float x);
#elif !defined(IMPROVED)
    static const double results_per_call = 8.0;
    __m256 rsqrt_Ofast_v(const __m256 arg_list);
#elif !defined(FMA)
    static const double results_per_call = 8.0;
    __m256 rsqrt_Ofast_v_improved(const __m256 arg_list);
#else
    static const double results_per_call = 8.0;
    __m256 rsqrt_Ofast_v_improved_FMA(const __m256 arg_list);
#endif

int
main(int argc, char* argv[]) {
#if !defined(VECTOR)
    float arg = 1.0f;
#else
    const __m256 arg_list = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    __m256 results;
#endif
    float result;
    std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
    for (uint64_t i = 0; i < ITERATIONS; i++) {
#if !defined(VECTOR)
        result = rsqrt_Ofast(arg);
#elif !defined(IMPROVED)
        results = rsqrt_Ofast_v(arg_list);
        result = results[ 0 ];
#elif !defined(FMA)
        results = rsqrt_Ofast_v_improved(arg_list);
        result = results[ 0 ];
#else
        results = rsqrt_Ofast_v_improved_FMA(arg_list);
        result = results[ 0 ];
#endif
    }
    std::chrono::time_point<std::chrono::system_clock> end_time = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;
    double mega_rate = (double)(ITERATIONS) / elapsed_seconds.count() * results_per_call / 1.0e+6;
    printf("elapsed time = %.6f seconds  mega_results/second = %.0f\nresult = %f\n", elapsed_seconds.count(), mega_rate, result);
    return 0;
}
