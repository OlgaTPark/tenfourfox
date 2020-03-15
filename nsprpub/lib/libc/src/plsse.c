#if defined(__SSE2__) && __SSE2__

#include <stdio.h>
#include <xmmintrin.h>
#include "plvmx.h"
#include "prtypes.h"

PR_IMPLEMENT(void*) sse_memchr(const void* b, int c, size_t length) {
    // Copied from https://github.com/ridiculousfish/HexFiend/blob/184646d0a42a70b6a8015daeb510aca47714d863/framework/sources/HFFastMemchr.m
    // with some adjustments (more-or-less the same as vmx_memchr).

    const unsigned char *haystack = (const unsigned char *)b;
    unsigned char needle = (unsigned char)c;

    /* SSE likes 16 byte alignment */
    
    /* Unaligned prefix */
    while (((intptr_t)haystack) % 16) {
        if (! length--) return NULL;
        if (*haystack == needle) return (void *)haystack;
        haystack++;
    }
    
    /* Compute the number of vectors we can compare, and the unaligned suffix */
    size_t numVectors = length / 16;
    size_t suffixLength = length % 16;
    
    const __m128i searchVector = _mm_set1_epi8(needle);
    while (numVectors--) {
        __m128i bytesVec = _mm_load_si128((const __m128i*)haystack);
        __m128i mask = _mm_cmpeq_epi8(bytesVec, searchVector);
        int maskedBits = _mm_movemask_epi8(mask);
        if (maskedBits) {
            /* some byte has the result - find the LSB of maskedBits */
            haystack += __builtin_ffs(maskedBits) - 1;
            return (void *)haystack;
        }
        
        haystack += 16;
    }
    
    /* Unaligned suffix */
    while (suffixLength--) {
        if (*haystack == needle) return (void *)haystack;
        haystack++;
    }
    
    return NULL;
}

#endif /* __SSE2__ */
