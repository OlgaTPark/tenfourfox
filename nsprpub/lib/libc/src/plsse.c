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

PR_IMPLEMENT(char*) sse_strchr(const char *p, int ch) {

    /* Inspired from vmx_strchr and sse_strchr ;-) */

    unsigned char c = (unsigned char)ch;

    /* Unaligned prefix */
    for (; ((uintptr_t)p & 15); ++p) {
        if (*p == c)    return ((char *)p);
        if (*p == '\0') return NULL;
    }

    const __m128i nullVector = _mm_setzero_si128();

    // Fast path for a NULL needle:
    // NOTE: On Intel Core 2 Duo, strchr is faster only with a NULL needle (both 32 and 64 bits)
    if (c == 0) {
        for(; ; p += 16) {
            __m128i w = _mm_load_si128((const __m128i*)p);
            __m128i nullMask = _mm_cmpeq_epi8(w, nullVector);
            int maskedNull = _mm_movemask_epi8(nullMask);
            if (maskedNull) {
                return (char *)(p + __builtin_ffs(maskedNull) - 1);
            }
        } /* for */
    } /* if c==0 */

    // else, if the needle is non-NULL:
    const __m128i searchVector = _mm_set1_epi8(c);

    for(; ; p += 16) {
        // From there, we assume the allocations are large enough in order of 
        // not being out-of-bounds
        __m128i w = _mm_load_si128((const __m128i*)p);
        __m128i nullMask = _mm_cmpeq_epi8(w, nullVector);
        int maskedNull = _mm_movemask_epi8(nullMask);
        if (maskedNull) {
            // If we're there, we have the end (w/ NULL-termination) of the string in (__mm128i)w
            int zero_pos = __builtin_ffs(maskedNull);
            __m128i mask = _mm_cmpeq_epi8(w, searchVector);
            int maskedBits = _mm_movemask_epi8(mask);
            if (maskedBits) {
                int ffs = __builtin_ffs(maskedBits);
                if (ffs < zero_pos) {
                    // If we're there, the searched character is BEFORE the end marker of the string
                    return (char *)(p + ffs - 1);
                }
            }
            // else-block for the TWO previous if's
            return NULL;
        } /* if maskedNull */

        // If we don't have reached the end of the string:
        __m128i mask = _mm_cmpeq_epi8(w, searchVector);
        int maskedBits = _mm_movemask_epi8(mask);
        if (maskedBits) {
            return (char *)(p + __builtin_ffs(maskedBits) - 1);
        }
    } /* for */

    // (normally) unreachable
    fprintf(stderr, "failed sse_strchr()\n");
    return NULL;
}

#endif /* __SSE2__ */
