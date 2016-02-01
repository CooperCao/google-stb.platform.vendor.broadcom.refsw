/*==============================================================================
 Copyright (c) 2015 Broadcom Europe Limited.
 All rights reserved.

 Module   :  khrn_mr_crc.h

 FILE DESCRIPTION
 Saves CRC checksums after an fmem flush.
==============================================================================*/
#ifndef KHRN_MR_CRC_H
#define KHRN_MR_CRC_H

#include "v3d_platform/v3d_scheduler.h"
#include "vcos_types.h"

VCOS_EXTERN_C_BEGIN

/* Used by khrn_fmem_flush to save checksums for each output buffer */
extern void khrn_save_crc_checksums(const V3D_BIN_RENDER_INFO_T *br_info);

VCOS_EXTERN_C_END

#endif
