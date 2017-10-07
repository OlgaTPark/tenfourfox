/* CTGradientCPP object which is a wrapper for CTGradient */

#ifndef __CTGradientCPP_
#define __CTGradientCPP_

#include "Types.h"

#ifndef CGFLOAT_DEFINED
  #if defined(__LP64__) && __LP64__
    typedef double CGFloat;
  #else	/* !defined(__LP64__) || !__LP64__ */
    typedef float CGFloat; // nasty
  #endif	/* !defined(__LP64__) || !__LP64__ */
  #define CGFLOAT_DEFINED 1
#endif	/* CGFLOAT_DEFINED */

namespace mozilla {
namespace gfx {

class CTGradientCPP {
	public:
	CTGradientCPP(CGColorSpaceRef cs, GradientStop *stops, size_t count);
	CTGradientCPP(CGColorSpaceRef cs, CGFloat *colours, CGFloat *offsets,
                             size_t count);
	~CTGradientCPP();

  	void DrawAxial(CGContextRef cg, CGPoint startPoint, CGPoint endPoint);
  	void DrawRadial(CGContextRef cg, CGPoint startCenter,
       		CGFloat startRadius, CGPoint endCenter, CGFloat endRadius);

	void *mGradient; // opaque class
};

}
}

#endif
