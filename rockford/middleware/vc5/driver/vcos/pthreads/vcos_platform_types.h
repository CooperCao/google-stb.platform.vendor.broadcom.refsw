/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  osal

FILE DESCRIPTION
VideoCore OS Abstraction Layer - platform-specific types and defines
=============================================================================*/

#ifndef VCOS_PLATFORM_TYPES_H
#define VCOS_PLATFORM_TYPES_H

#include <stdarg.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__) && (( __GNUC__ > 2 ) || (( __GNUC__ == 2 ) && ( __GNUC_MINOR__ >= 3 )))
#define VCOS_FORMAT_ATTR_(ARCHETYPE, STRING_INDEX, FIRST_TO_CHECK)  __attribute__ ((format (ARCHETYPE, STRING_INDEX, FIRST_TO_CHECK)))
#else
#define VCOS_FORMAT_ATTR_(ARCHETYPE, STRING_INDEX, FIRST_TO_CHECK)
#endif

#if defined(__linux__) && !defined(NDEBUG) && defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
   #define VCOS_BKPT ({ __asm volatile ("int3":::"memory"); })
#else
   #define VCOS_BKPT ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif
