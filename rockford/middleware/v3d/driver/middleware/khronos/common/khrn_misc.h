/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  External interface
File     :  $RCSfile$
Revision :  $Revision$

FILE DESCRIPTION
External interface.
=============================================================================*/

#ifndef KHRN_MISC_H
#define KHRN_MISC_H

//if you want KHRN_USE_VCHIQ define it in the platform makefile
//e.g in vcfw/platform/broadcom/2763dbrev2/2763dbrev2_linux.mk
//#ifndef KHRN_USE_VCHIQ
//#define KHRN_USE_VCHIQ
//#endif

#include "middleware/khronos/common/khrn_hw.h"
#include "interface/khronos/common/khrn_int_misc_impl.h"
#include "interface/khronos/include/GLES/gl.h"
#include "interface/khronos/include/GLES/glext.h"
#include "interface/khronos/glxx/glxx_int_attrib.h"
#ifndef NO_OPENVG
#include "interface/khronos/include/VG/openvg.h"
#include "interface/khronos/include/VG/vgext.h"
#include "interface/khronos/include/VG/vgu.h"
#include "interface/khronos/vg/vg_int.h"
#include "interface/khronos/vg/vg_int_mat3x3.h"
#endif /* NO_OPENVG */
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/common/khrn_int_image.h"
#include "interface/khronos/egl/egl_int.h"

#endif
