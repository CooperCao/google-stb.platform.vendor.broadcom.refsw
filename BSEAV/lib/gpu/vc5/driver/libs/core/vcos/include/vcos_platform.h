/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#if defined(__unix__)
#include "../posix/vcos_platform_posix.h"
#elif defined(WIN32)
#include "../win32/vcos_platform_win32.h"
#else
#error "Unknown platform"
#endif
