/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009-2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  autoclif recording interface

FILE DESCRIPTION
Drivers use this interface to trigger autoclif recording
=============================================================================*/
#pragma once

#include <stdbool.h>

#ifdef KHRN_AUTOCLIF

#include "khrn_process.h"
#include "khrn_fmem.h"

extern void khrn_record(const V3D_BIN_RENDER_INFO_T *br_info);

#endif
