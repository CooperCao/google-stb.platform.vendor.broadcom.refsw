/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

#if defined(__unix__)
#include "../posix/vcos_platform_types_posix.h"
#elif defined(WIN32)
#include "../win32/vcos_platform_types_win32.h"
#else
#error "Unknown platform"
#endif
