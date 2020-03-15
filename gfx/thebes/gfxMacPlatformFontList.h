/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef gfxMacPlatformFontList_H_
#define gfxMacPlatformFontList_H_

#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>

#include "mozilla/MemoryReporting.h"
#include "nsDataHashtable.h"
#include "nsRefPtrHashtable.h"

#include "gfxPlatformFontList.h"
#include "gfxPlatform.h"
#include "gfxPlatformMac.h"

#include "nsUnicharUtils.h"
#include "nsTArray.h"
#include "mozilla/LookAndFeel.h"

class gfxMacPlatformFontList;

// a single member of a font family (i.e. a single face, such as Times Italic)
class MacOSFontEntry : public gfxFontEntry
{
public:
    friend class gfxMacPlatformFontList;

    MacOSFontEntry(const nsAString& aPostscriptName, int32_t aWeight,
                   bool aIsStandardFace = false);

    // for use with data fonts
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060 /* 1050 */
    // It's possible to use the CGFontRef implementation on 10.5.  
    MacOSFontEntry(const nsAString& aPostscriptName, CGFontRef aFontRef,
#else
    MacOSFontEntry(const nsAString& aPostscriptName, ATSFontRef aFontRef,
#endif
                   uint16_t aWeight, uint16_t aStretch, uint8_t aStyle,
#if MAC_OS_X_VERSION_MIN_REQUIRED < 1060 /* 1050 */
                   ATSFontContainerRef aContainerRef, // 10.4Fx
#endif
                   bool aIsDataUserFont, bool aIsLocal);

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060 /* 1050 */
    virtual ~MacOSFontEntry() {
        ::CGFontRelease(mFontRef);
    }
#else
    virtual ~MacOSFontEntry() {
        if (mFontRefInitialized)
        ::CGFontRelease(mFontRef);
        /* Per Apple, even synthesized CGFontRefs must be released. Also,
           we do need to release our container ref, if any. */
        if (mContainerRef)
                ::ATSFontDeactivate(mContainerRef, NULL,
                        kATSOptionFlagsDefault);
    }

    // 10.4Fx
    ATSFontRef GetATSFontRef();
#endif

    virtual CGFontRef GetFontRef();

    // override gfxFontEntry table access function to bypass table cache,
    // use CGFontRef API to get direct access to system font data
    virtual hb_blob_t *GetFontTable(uint32_t aTag) override;

    virtual void AddSizeOfIncludingThis(mozilla::MallocSizeOf aMallocSizeOf,
                                        FontListSizes* aSizes) const override;

    nsresult ReadCMAP(FontInfoData *aFontInfoData = nullptr) override;

    bool RequiresAATLayout() const { return mRequiresAAT; }

    bool IsCFF();

protected:
    virtual gfxFont* CreateFontInstance(const gfxFontStyle *aFontStyle, bool aNeedsBold) override;

    virtual bool HasFontTable(uint32_t aTableTag) override;

    static void DestroyBlobFunc(void* aUserData);

    CGFontRef mFontRef; // owning reference to the CGFont, released on destruction

#if MAC_OS_X_VERSION_MIN_REQUIRED < 1060 /* 1050 */
    // 10.4Fx class variables
    ATSFontRef mATSFontRef; // 10.4Fx (owning reference to our ATSFont)
    ATSFontContainerRef mContainerRef; // 10.4Fx (for MakePlatformFont)
    bool mATSFontRefInitialized; // 10.4Fx. mUserFontData is in gfxFont.h.
    AutoFallibleTArray<uint8_t,1024> mFontTableDir; // 10.4Fx
    ByteCount mFontTableDirSize; // 10.4Fx
    void TryGlobalFontTableCache();
#endif

    bool mFontRefInitialized;
    bool mRequiresAAT;
    bool mIsCFF;
    bool mIsCFFInitialized;
    nsTHashtable<nsUint32HashKey> mAvailableTables;
};

class gfxMacPlatformFontList : public gfxPlatformFontList {
public:
    static gfxMacPlatformFontList* PlatformFontList() {
        return static_cast<gfxMacPlatformFontList*>(sPlatformFontList);
    }

    static int32_t AppleWeightToCSSWeight(int32_t aAppleWeight);

    gfxFontFamily* GetDefaultFont(const gfxFontStyle* aStyle) override;

    bool GetStandardFamilyName(const nsAString& aFontName, nsAString& aFamilyName) override;

    gfxFontEntry* LookupLocalFont(const nsAString& aFontName,
                                  uint16_t aWeight,
                                  int16_t aStretch,
                                  uint8_t aStyle) override;

    gfxFontEntry* MakePlatformFont(const nsAString& aFontName,
                                   uint16_t aWeight,
                                   int16_t aStretch,
                                   uint8_t aStyle,
                                   const uint8_t* aFontData,
                                   uint32_t aLength) override;

    gfxFontFamily* FindFamily(const nsAString& aFamily,
                              gfxFontStyle* aStyle = nullptr,
                              gfxFloat aDevToCssSize = 1.0) override;

    // lookup the system font for a particular system font type and set
    // the name and style characteristics
    void LookupSystemFont(mozilla::LookAndFeel::FontID aSystemFontID,
                          nsAString& aSystemFontName,
                          gfxFontStyle &aFontStyle,
                          float aDevPixPerCSSPixel);

#if MAC_OS_X_VERSION_MIN_REQUIRED < 1050
    void SetFixedPitch(const nsAString& aFamilyName); // 10.4Fx
#endif

private:
    friend class gfxPlatformMac;

    gfxMacPlatformFontList();
    virtual ~gfxMacPlatformFontList();

    // initialize font lists
    nsresult InitFontList() override;

    // special case font faces treated as font families (set via prefs)
    void InitSingleFaceList();

    // initialize system fonts
    void InitSystemFonts();

    // helper function to lookup in both hidden system fonts and normal fonts
    gfxFontFamily* FindSystemFontFamily(const nsAString& aFamily);

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
    static void RegisteredFontsChangedNotificationCallback(CFNotificationCenterRef center,
                                                           void *observer,
                                                           CFStringRef name,
                                                           const void *object,
                                                           CFDictionaryRef userInfo);
#else
  #if MAC_OS_X_VERSION_MIN_REQUIRED < 1050
    // eliminate faces which have the same ATS font reference
    // backout bug 663688
    void EliminateDuplicateFaces(const nsAString& aFamilyName);
  #endif

	// backout bug 869762
	static void ATSNotification(ATSFontNotificationInfoRef aInfo, void* aUserArg);
	uint32_t mATSGeneration;
#endif /* MAC_OS_X_VERSION_MIN_REQUIRED */

    // search fonts system-wide for a given character, null otherwise
    gfxFontEntry* GlobalFontFallback(const uint32_t aCh,
                                     int32_t aRunScript,
                                     const gfxFontStyle* aMatchStyle,
                                     uint32_t& aCmapCount,
                                     gfxFontFamily** aMatchedFamily) override;

    bool UsesSystemFallback() override { return true; }

    already_AddRefed<FontInfoData> CreateFontInfoData() override;

    // Add the specified family to mSystemFontFamilies or mFontFamilies.
    // Ideally we'd use NSString* instead of CFStringRef here, but this header
    // file is included in .cpp files, so we can't use objective C classes here.
    // But CFStringRef and NSString* are the same thing anyway (they're
    // toll-free bridged).
    void AddFamily(CFStringRef aFamily);

#ifdef MOZ_BUNDLED_FONTS
    void ActivateBundledFonts();
#endif

    enum {
        kATSGenerationInitial = -1
    };

    // default font for use with system-wide font fallback
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060 /* 1050 */
    CTFontRef mDefaultFont;
#else
    ATSFontRef mDefaultFont;
#endif

    // hidden system fonts used within UI elements, there may be a whole set
    // for different locales (e.g. .Helvetica Neue UI, .SF NS Text)
    FontFamilyTable mSystemFontFamilies;

    // font families that -apple-system maps to
    // Pre-10.11 this was always a single font family, such as Lucida Grande
    // or Helvetica Neue. For OSX 10.11, Apple uses pair of families
    // for the UI, one for text sizes and another for display sizes
    bool mUseSizeSensitiveSystemFont;
    RefPtr<gfxFontFamily> mSystemTextFontFamily;
    RefPtr<gfxFontFamily> mSystemDisplayFontFamily; // only used on OSX 10.11
};

#endif /* gfxMacPlatformFontList_H_ */
