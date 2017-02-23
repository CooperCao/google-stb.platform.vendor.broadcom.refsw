/*==============================================================================
 Broadcom Proprietary and Confidential. (c)2015 Broadcom.
 All rights reserved.

 Module   :  khrn_mr_crc.h

 FILE DESCRIPTION
 Saves CRC checksums after an fmem flush.
==============================================================================*/
#pragma once

#include "libs/platform/v3d_scheduler.h"
#include "vcos_types.h"

VCOS_EXTERN_C_BEGIN

typedef struct khrn_memaccess khrn_memaccess;

/* Used by khrn_fmem_flush to save checksums for each output buffer */
void khrn_save_crc_checksums(khrn_memaccess* ma, const V3D_BIN_RENDER_INFO_T *br_info);

VCOS_EXTERN_C_END
