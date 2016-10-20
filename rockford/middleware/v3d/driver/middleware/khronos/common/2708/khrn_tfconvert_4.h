/*=============================================================================
Broadcom Proprietary and Confidential. (c)2012 Broadcom.
All rights reserved.

Project  :  3D Tools
Module   :  Control list creation for RSO to t-format using tile buffer

FILE DESCRIPTION
Builds a control list for RSO to t-format conversion using tile buffer
=============================================================================*/

#ifndef __TFCONVERT_4_H__
#define __TFCONVERT_4_H__

#include "middleware/khronos/common/khrn_mem.h"
#include "interface/khronos/common/khrn_int_image.h"
#include "middleware/khronos/common/2708/khrn_fmem_4.h"
#include "middleware/khronos/ext/egl_khr_image.h"
#include "middleware/khronos/glxx/glxx_server.h"

extern bool khrn_rso_to_tf_convert(GLXX_SERVER_STATE_T *state, MEM_HANDLE_T hsrc, MEM_HANDLE_T hdst);
extern void khrn_tfconvert_done(BEGL_HWCallbackRecord *cbRecord);
extern void khrn_tfconvert_logevents(BEGL_HWNotification *notification);

#endif /* __TFCONVERT_4_H__ */