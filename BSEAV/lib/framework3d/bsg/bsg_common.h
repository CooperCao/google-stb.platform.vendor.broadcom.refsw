/*=============================================================================
Broadcom Proprietary and Confidential. (c)2012 Broadcom.
All rights reserved.

Project  :  Include file included everywhere in bsg
Module   :

FILE DESCRIPTION
Common include file
=============================================================================*/

#ifndef __BSG_COMMON_H__
#define __BSG_COMMON_H__

#ifdef WIN32

#define _CRTDBG_MAP_ALLOC
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
