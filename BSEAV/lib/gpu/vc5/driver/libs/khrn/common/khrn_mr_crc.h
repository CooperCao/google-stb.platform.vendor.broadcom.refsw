/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/platform/v3d_scheduler.h"
#include "vcos_types.h"

EXTERN_C_BEGIN

typedef struct khrn_memaccess khrn_memaccess;

/* Used by khrn_fmem_flush to save checksums for each output buffer */
void khrn_save_crc_checksums(khrn_memaccess* ma, const V3D_BIN_RENDER_INFO_T *br_info);

EXTERN_C_END
