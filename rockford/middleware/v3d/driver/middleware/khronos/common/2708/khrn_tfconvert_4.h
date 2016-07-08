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

extern bool khrn_rso_to_tf_convert(MEM_HANDLE_T heglImage, MEM_HANDLE_T srcImg, MEM_HANDLE_T dstImg);
extern bool khrn_tfconvert_lock(BEGL_HWCallbackRecord *cbRecord);
extern void khrn_tfconvert_done(BEGL_HWCallbackRecord *cbRecord);

extern void khrn_tfconvert_logevents(BEGL_HWNotification *notification, BEGL_HWCallbackRecord *cbRec);

typedef struct
{
   KHRN_FMEM_T *fmem;
   MEM_HANDLE_T heglimage;
   bool        conversionKilled;
   bool        semaphoreTaken;
   uint8_t     *controlList;
   uint32_t    numCLBytes;
} CONVERT_CALLBACK_DATA_T;

#endif /* __TFCONVERT_4_H__ */
