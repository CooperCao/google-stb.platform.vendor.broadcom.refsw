/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  VG path

FILE DESCRIPTION
Platform-internal include.
=============================================================================*/

#ifndef VG_PATH_PRIV_4_H
#define VG_PATH_PRIV_4_H

#include "middleware/khronos/vg/vg_path.h"

extern bool vg_path_tess_retain(VG_PATH_T *path);

extern bool vg_path_bake_interp_to(VG_PATH_T *path);
extern bool vg_path_bake_interps_from(VG_PATH_T *path);

#endif
