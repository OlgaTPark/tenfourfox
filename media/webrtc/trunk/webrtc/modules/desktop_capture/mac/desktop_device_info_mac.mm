/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "webrtc/modules/desktop_capture/mac/desktop_device_info_mac.h"
#include <AppKit/AppKit.h>
#include <Cocoa/Cocoa.h>
/* Avoids including this when not needed in order to reduce compile-time */
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1050 && MAC_OS_X_VERSION_MIN_REQUIRED < 1060
  #include <ApplicationServices/ApplicationServices.h>
#endif
#include <unistd.h>
#include <inttypes.h>
#include <stdio.h>
#include <map>

namespace webrtc {

// Helper type to track the number of window instances for a given process
typedef std::map<ProcessId, uint32_t> AppWindowCountMap;


DesktopDeviceInfo * DesktopDeviceInfoImpl::Create() {
  DesktopDeviceInfoMac * pDesktopDeviceInfo = new DesktopDeviceInfoMac();
  if (pDesktopDeviceInfo && pDesktopDeviceInfo->Init() != 0){
    delete pDesktopDeviceInfo;
    pDesktopDeviceInfo = NULL;
  }
  return pDesktopDeviceInfo;
}

DesktopDeviceInfoMac::DesktopDeviceInfoMac() {
}

DesktopDeviceInfoMac::~DesktopDeviceInfoMac() {
}

void DesktopDeviceInfoMac::MultiMonitorScreenshare()
{
#if !defined(MULTI_MONITOR_SCREENSHARE)
  DesktopDisplayDevice* desktop_device_info = new DesktopDisplayDevice;
  if (desktop_device_info) {
    desktop_device_info->setScreenId(CGMainDisplayID());
    desktop_device_info->setDeviceName("Primary Monitor");

    char idStr[64];
    snprintf(idStr, sizeof(idStr), "%" PRIdPTR, desktop_device_info->getScreenId());
    desktop_device_info->setUniqueIdName(idStr);
    desktop_display_list_[desktop_device_info->getScreenId()] = desktop_device_info;
  }
#else
  const UInt32 kMaxScreens = 256;
  CGDirectDisplayID screens[kMaxScreens];
  CGDisplayCount num_of_screens;
  CGGetActiveDisplayList(kMaxScreens, screens, &num_of_screens);

  for (CFIndex i = 0; i < num_of_screens; ++i) {
    DesktopDisplayDevice* desktop_device_info = new DesktopDisplayDevice;
    if (desktop_device_info) {
      desktop_device_info->setScreenId((ScreenId)screens[i]);
      if (1 >= num_of_screens) {
        desktop_device_info->setDeviceName("Primary Monitor");
      } else {
        char nameStr[64];
        snprintf(nameStr, sizeof(nameStr), "Screen %" PRIdPTR, i + 1);
        desktop_device_info->setDeviceName(nameStr);
      }

      char idStr[64];
      snprintf(idStr, sizeof(idStr), "%" PRIdPTR, desktop_device_info->getScreenId());
      desktop_device_info->setUniqueIdName(idStr);
      desktop_display_list_[desktop_device_info->getScreenId()] = desktop_device_info;
    }
  }
#endif
}

void DesktopDeviceInfoMac::InitializeScreenList() {
  MultiMonitorScreenshare();
}

void DesktopDeviceInfoMac::InitializeApplicationList() {
// There shall be none, and none shall there be (at least on Tiger).
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1050
  //List all running applications (excluding background processes).

  // Get a list of all windows, to match to applications
  AppWindowCountMap appWins;
  CFArrayRef windowInfos = CGWindowListCopyWindowInfo(
      kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements,
      kCGNullWindowID);
  CFIndex windowInfosCount = CFArrayGetCount(windowInfos);
  for (CFIndex idx = 0; idx < windowInfosCount; idx++) {
    CFDictionaryRef info = reinterpret_cast<CFDictionaryRef>(
        CFArrayGetValueAtIndex(windowInfos, idx));
    CFNumberRef winOwner = reinterpret_cast<CFNumberRef>(
        CFDictionaryGetValue(info, kCGWindowOwnerPID));

    pid_t owner;
    CFNumberGetValue(winOwner, kCFNumberIntType, &owner);
    AppWindowCountMap::iterator itr = appWins.find(owner);
    if (itr == appWins.end()) {
      appWins[owner] = 1;
    } else {
      appWins[owner]++;
    }
  }

  #if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
  NSArray *running = [[NSWorkspace sharedWorkspace] runningApplications];
  for (NSRunningApplication *ra in running) {
    if (ra.activationPolicy != NSApplicationActivationPolicyRegular)
      continue;
  #else
  NSArray *running = [[NSWorkspace sharedWorkspace] launchedApplications];
  for (NSDictionary *ra in running) {
  // This is maybe useless since -runningApplications returns all user's 
  // applications but -launchedApplications only returns applications with a GUI
    #if __LP64__
    ProcessSerialNumber psn = {
      .highLongOfPSN = [[ra objectForKey:@"NSApplicationProcessSerialNumberHigh"]unsignedIntValue], 
      .lowLongOfPSN = [[ra objectForKey:@"NSApplicationProcessSerialNumberLow"]unsignedIntValue]};
    #else
    ProcessSerialNumber psn = {
      .highLongOfPSN = [[ra objectForKey:@"NSApplicationProcessSerialNumberHigh"]unsignedLongValue], 
      .lowLongOfPSN = [[ra objectForKey:@"NSApplicationProcessSerialNumberLow"]unsignedLongValue]};
    #endif
    CFDictionaryRef infoDict = ProcessInformationCopyDictionary(&psn, kProcessDictionaryIncludeAllInformationMask);
    if (::CFBooleanGetValue((CFBooleanRef)CFDictionaryGetValue(infoDict, @"LSBackgroundOnly")) ||
        ::CFBooleanGetValue((CFBooleanRef)CFDictionaryGetValue(infoDict, @"LSUIElement")))
      continue;
  #endif

  #if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
    ProcessId pid = ra.processIdentifier;
  #else
    ProcessId pid = [[ra objectForKey:@"NSApplicationProcessIdentifier"]longValue];
  #endif
    if (pid == 0) {
      continue;
    }
    if (pid == getpid()) {
      continue;
    }

    DesktopApplication *pDesktopApplication = new DesktopApplication;
    if (!pDesktopApplication) {
      continue;
    }

    pDesktopApplication->setProcessId(pid);
    pDesktopApplication->setWindowCount(appWins[pid]);

    NSString *str;
  #if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
    str = [ra.executableURL absoluteString];
  #else
    str = [ra objectForKey:@"NSApplicationPath"];
  #endif
    pDesktopApplication->setProcessPathName([str UTF8String]);

    // Record <window count> then <localized name>
    // NOTE: localized names can get *VERY* long
  #if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
    str = ra.localizedName;
  #else
    str = [ra objectForKey:@"NSApplicationName"]; // This is the localized name
  #endif
    char nameStr[BUFSIZ];
    snprintf(nameStr, sizeof(nameStr), "%d\x1e%s", pDesktopApplication->getWindowCount(), [str UTF8String]);
    pDesktopApplication->setProcessAppName(nameStr);

    char idStr[64];
    snprintf(idStr, sizeof(idStr), "%ld", pDesktopApplication->getProcessId());
    pDesktopApplication->setUniqueIdName(idStr);

    desktop_application_list_[pDesktopApplication->getProcessId()] = pDesktopApplication;
  }
#endif
}

} //namespace webrtc
