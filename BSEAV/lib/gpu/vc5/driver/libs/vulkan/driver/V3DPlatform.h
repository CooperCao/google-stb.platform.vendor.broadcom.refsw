/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include "platforms/V3DPlatformWin32.h"
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
#include "platforms/V3DPlatformXcb.h"
#endif

#if EMBEDDED_SETTOP_BOX
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
#include "platforms/V3DPlatformWayland.h"
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#include "platforms/V3DPlatformAndroid.h"
#elif defined(SINGLE_PROCESS)
#include "platforms/V3DPlatformNexusExclusive.h"
#else
#include "platforms/V3DPlatformNxClient.h"
#endif
#endif
