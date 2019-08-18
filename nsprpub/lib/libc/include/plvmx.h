#include <stdlib.h>
#include <sys/types.h>

/* In general, you should include "mozilla-config.h" or the equivalent
   before including this file to choose the proper macro. */

#ifndef _nspr_plvmx_h
#define _nspr_plvmx_h

#if defined (__cplusplus)
extern "C" {
#endif /* __cplusplus */

#if TENFOURFOX_VMX

void *vmx_memchr(const void *b, int c, size_t len);

#define VMX_MEMCHR vmx_memchr

#elif defined(__SSE2__) && __SSE2__
/* This should have been placed in another file but this allows us to use the 
   same file for both SSE and VMX optimized versions of memchr.  I also hacked 
   the VMX_MEMCHR macro to avoid monkeypatching other files. */

extern void *sse_memchr(const void *b, int c, size_t len);

#define VMX_MEMCHR sse_memchr

#else /* !__SSE2__ && !TENFOURFOX_VMX */
#define VMX_MEMCHR memchr
#endif

#if defined (__cplusplus)
}
#endif /* __cplusplus */

#endif /* _nspr_plvmx_h */
