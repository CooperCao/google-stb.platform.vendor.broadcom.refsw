/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Common include

FILE DESCRIPTION
Common include.
=============================================================================*/

#ifndef KHRN_INT_COMMON_H
#define KHRN_INT_COMMON_H
#ifdef __cplusplus
extern "C" {
#endif

#include "helpers/v3d/v3d_common.h"
#include "vcos.h"
#include <string.h> /* size_t */

#define UNREACHABLE() unreachable()

#if defined(_MSC_VER) || defined (__GNUC__)
   #define UNUSED(X) ((void)X)
#else
   #define UNUSED(X)
#endif

#ifdef NDEBUG
   #define UNUSED_NDEBUG(X) UNUSED(X)
#else
   #define UNUSED_NDEBUG(X)
#endif

#ifdef __cplusplus
 }
#endif

#endif
