#include<stdlib.h>
#include<stdio.h>
#include "trace.h"

#define ASSERT_DIRECT(CMP, MSG) if(CMP==0) {TRACE_ERROR("Assert failed(%d): %s", !CMP, MSG); exit(1);};
#define ASSERT_EXP(CMP, MSG, EXP) if(CMP==0) {TRACE_ERROR_START();TRACE_CONTENT("Assert failed(%d): %s ", !CMP, MSG); EXP; TRACE_END(); exit(1);};

#define ASSERT_RET_ARG4(arg1, arg2, arg3, arg4, ...) arg4
#define ASSERT_CHOOSER(...) ASSERT_RET_ARG4(__VA_ARGS__, ASSERT_EXP, ASSERT_DIRECT)

#define ASSERT(...) ASSERT_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define ASSERT_NOT_NULL(OBJ, MSG) ASSERT( ((void *)OBJ), MSG)
#define ASSERT_EQUAL(X, Y, MSG) ASSERT(X==Y, MSG)

#define PRINT_BLOCK(PRT, H, W, FMT) {     \
    for(int i=0;i<H;i++){ \
        for(int j=0;j<W;j++){ \
            int flat_idx = i * W + j; \
            printf(FMT"\t", PRT[flat_idx]); \
        } \
        printf("\r\n"); \
    }}


#define SPLIT_BYTE(X, HIGH4b, LOW4b) {HIGH4b = X>>4; LOW4b = X & 0x0f;}