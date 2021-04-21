//  -*-  mode: C++;  fill-column: 132-*-
//  Time-stamp: "Modified on 28-January-2016 at 10:39:28 by jeff on Jeffs-MacBook-Pro"

#include <cmath>
#include <cstdio>
#include <cstring>

//  External routines:
float rcp_Ofast(float x);
float rcp_NR1(float arg);
float rcp_NR2(float arg);

int
main(int argc, char* argv[]) {

    float arg = 0.5f;
    float result;
    unsigned int iresult;

    printf("\nFull precision:\n");
    result = 1.0f / arg;
    memcpy(&iresult, &result, sizeof(result));
    printf("    result = %.8f  %.6a  0x%8.8x\n", result, result, iresult);

    printf("Fast:\n");
    result = rcp_Ofast(arg);
    memcpy(&iresult, &result, sizeof(result));
    printf("    result = %.8f  %.6a  0x%8.8x\n", result, result, iresult);

    printf("NR1:\n");
    result = rcp_NR1(arg);
    memcpy(&iresult, &result, sizeof(result));
    printf("    result = %.8f  %.6a  0x%8.8x\n", result, result, iresult);

    printf("NR2:\n");
    result = rcp_NR2(arg);
    memcpy(&iresult, &result, sizeof(result));
    printf("    result = %.8f  %.6a  0x%8.8x\n", result, result, iresult);

    return 0;
}
