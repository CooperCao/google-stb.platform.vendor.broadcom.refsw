/*=============================================================================
Copyright (c) 2009-2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  autoclif recording interface

FILE DESCRIPTION
Drivers use this interface to trigger autoclif recording
=============================================================================*/
#pragma once

#include <stdbool.h>

#ifdef KHRN_AUTOCLIF

#include "middleware/khronos/common/khrn_process.h"
#include "middleware/khronos/common/khrn_fmem.h"

extern void khrn_record(const V3D_BIN_RENDER_INFO_T *br_info);

#endif
