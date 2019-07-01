/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ProcessUtils.h"

#include "nsString.h"

#include "mozilla/plugins/PluginUtilsOSX.h"

namespace mozilla {
namespace ipc {

void SetThisProcessName(const char *aName)
{
#ifdef MOZ_PLUGINS
  mozilla::plugins::PluginUtilsOSX::SetProcessName(aName);
#endif /* MOZ_PLUGINS */
}

} // namespace ipc
} // namespace mozilla
