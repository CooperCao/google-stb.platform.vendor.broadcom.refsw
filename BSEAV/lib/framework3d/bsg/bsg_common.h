/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_COMMON_H__
#define __BSG_COMMON_H__

#ifdef WIN32

#include <stdlib.h>
#include <crtdbg.h>

#ifdef BSG_MEMORY_TRACKING
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG
#endif

#endif

#include "bsg_gl_intercept.h"

#undef min
#undef max

#endif /* __BSG_COMMON_H__ */
