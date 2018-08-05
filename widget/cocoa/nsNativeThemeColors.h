/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsNativeThemeColors_h_
#define nsNativeThemeColors_h_

#include "nsCocoaFeatures.h"
#import <Cocoa/Cocoa.h>

// backout bug 668195
#if(0)
enum ColorName {
  toolbarTopBorderGrey,
  toolbarFillGrey,
  toolbarBottomBorderGrey,
};

static const int sSnowLeopardThemeColors[][2] = {
  /* { active window, inactive window } */
  // toolbar:
  { 0xD0, 0xF1 }, // top separator line
  { 0xA7, 0xD8 }, // fill color
  { 0x51, 0x99 }, // bottom separator line
};

static const int sLionThemeColors[][2] = {
  /* { active window, inactive window } */
  // toolbar:
  { 0xD0, 0xF0 }, // top separator line
  { 0xB2, 0xE1 }, // fill color
  { 0x59, 0x87 }, // bottom separator line
};
#else
enum ColorName {
  headerStartGrey,
  headerEndGrey,
  headerBorderGrey,
  toolbarTopBorderGrey,
  statusbarFirstTopBorderGrey,
  statusbarSecondTopBorderGrey,
  statusbarGradientStartGrey,
  statusbarGradientEndGrey
};

/* 10.4 FOREVER */
static const int sTigerThemeColors[][2] = {
  /* { active window, inactive window } */
  // titlebar and toolbar:
  { 0xC5, 0xE9 }, // start grey
  { 0xC5, 0xE9 }, // end grey
  { 0x42, 0x89 }, // bottom separator line
  { 0xC0, 0xE2 }, // top separator line
  // statusbar:
  { 0x42, 0x86 }, // first top border
  { 0xD8, 0xEE }, // second top border
  { 0xBD, 0xE4 }, // gradient start
  { 0x96, 0xCF }  // gradient end
};

static const int sLeopardThemeColors[][2] = {
  /* { active window, inactive window } */
  // titlebar and toolbar:
  { 0xC5, 0xE9 }, // start grey
  { 0x96, 0xCA }, // end grey
  { 0x42, 0x89 }, // bottom separator line
  { 0xC0, 0xE2 }, // top separator line
  // statusbar:
  { 0x42, 0x86 }, // first top border
  { 0xD8, 0xEE }, // second top border
  { 0xBD, 0xE4 }, // gradient start
  { 0x96, 0xCF }  // gradient end
};
 
static const int sSnowLeopardThemeColors[][2] = {
  /* { active window, inactive window } */
  // titlebar and toolbar:
  { 0xD1, 0xEE }, // start grey
  { 0xA7, 0xD8 }, // end grey
  { 0x51, 0x99 }, // bottom separator line
  { 0xD0, 0xF1 }, // top separator line
  // statusbar:
  { 0x51, 0x99 }, // first top border
  { 0xE8, 0xF6 }, // second top border
  { 0xCB, 0xEA }, // gradient start
  { 0xA7, 0xDE }  // gradient end
};
// We won't need Lion for 10.4Fx! Good riddance!

#if defined(__i386__) || defined(__x86_64__)
static const int sLionThemeColors[][2] = {
  /* { active window, inactive window } */
  // titlebar and toolbar:
  { 0xD1, 0xEE }, // start grey (Snow)
  { 0xB2, 0xE1 }, // fill color -> end grey
  { 0x59, 0x87 }, // bottom separator line -> bottom separator line
  { 0xD0, 0xF0 }, // top separator line -> top separator line
  // statusbar:
  { 0x51, 0x99 }, // first top border (Snow)
  { 0xE8, 0xF6 }, // second top border (Snow)
  { 0xCB, 0xEA }, // gradient start (Snow)
  { 0xA7, 0xDE }  // gradient end (Snow)
};
#endif /* defined(__i386__) || defined(__x86_64__) */
#endif

__attribute__((unused))
static int NativeGreyColorAsInt(ColorName name, BOOL isMain)
{
  // If we're targetting 10.5+, we render with CoreUI and we can use color 
  // variants.  Otherwise, we use the common tiger theme.
#if (defined(__i386__) || defined(__x86_64__)) && MAC_OS_X_VERSION_MAX_ALLOWED >= 1050
  #if MAC_OS_X_VERSION_MIN_REQUIRED < 1070
  if (nsCocoaFeatures::OnLionOrLater())
  #endif /* 1070 */
    return sLionThemeColors[name][isMain ? 0 : 1];
  #if MAC_OS_X_VERSION_MIN_REQUIRED < 1060
  if (nsCocoaFeatures::OnSnowLeopardOrLater())
  #endif /* 1060 */
    return sSnowLeopardThemeColors[name][isMain ? 0 : 1];
  #if MAC_OS_X_VERSION_MIN_REQUIRED < 1050
  if (nsCocoaFeatures::OnLeopardOrLater())
  #endif /* 1050 */
    return sLeopardThemeColors[name][isMain ? 0 : 1];
#endif /* (__i386__ || __x86_64__) && MAC_OS_X_VERSION_MAX_ALLOWED >= 1050 */
  return sTigerThemeColors[name][isMain ? 0 : 1];
}

__attribute__((unused))
static float NativeGreyColorAsFloat(ColorName name, BOOL isMain)
{
  return NativeGreyColorAsInt(name, isMain) / 255.0f;
}

__attribute__((unused))
static void DrawNativeGreyColorInRect(CGContextRef context, ColorName name,
                                      CGRect rect, BOOL isMain)
{
  float grey = NativeGreyColorAsFloat(name, isMain);
  CGContextSetRGBFillColor(context, grey, grey, grey, 1.0f);
  CGContextFillRect(context, rect);
}

#endif // nsNativeThemeColors_h_
