/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#if KHRN_DEBUG

#include "libs/util/common.h"
#include "libs/platform/v3d_scheduler.h"

EXTERN_C_BEGIN

typedef struct khrn_memaccess khrn_memaccess;

/* Used by khrn_fmem_flush to save checksums for each output buffer */
void khrn_save_crc_checksums_bin_render(khrn_memaccess* ma, const V3D_BIN_RENDER_INFO_T *br_info);
#if V3D_USE_CSD
void khrn_save_crc_checksums_compute(khrn_memaccess* ma, const struct v3d_compute_subjob *subjobs, size_t num_subjobs);
#endif

EXTERN_C_END

#endif
