/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  osal
File     :  $RCSfile: $
Revision :  $Revision: $

FILE DESCRIPTION
VideoCore OS Abstraction Layer - platform-specific types and defines
=============================================================================*/

#ifndef VCOS_PLATFORM_TYPES_H
#define VCOS_PLATFORM_TYPES_H

#include <stdint.h>

extern void
vcos_pthreads_logging_assert(const char *file, const char *func, unsigned int line, const char *fmt, ...);

#define VCOSPRE_ extern
#define VCOSPOST_

#define VCOS_ASSERT_LOGGING         1
#define VCOS_ASSERT_LOGGING_DISABLE 0

#define VCOS_ASSERT_MSG(...) ((VCOS_ASSERT_LOGGING && !VCOS_ASSERT_LOGGING_DISABLE) ? vcos_pthreads_logging_assert(__FILE__, __func__, __LINE__, __VA_ARGS__) : (void)0)

#endif
