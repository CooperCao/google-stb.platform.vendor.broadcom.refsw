/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdbool.h>

#if KHRN_DEBUG

#include "khrn_process.h"
#include "khrn_fmem.h"

EXTERN_C_BEGIN

typedef struct khrn_memaccess khrn_memaccess;

void khrn_record(khrn_memaccess* ma, const V3D_BIN_RENDER_INFO_T *br_info);

#if V3D_USE_CSD
void khrn_record_csd(khrn_memaccess* ma, v3d_compute_subjob const* subjobs, unsigned num_subjobs);
#endif

EXTERN_C_END

#endif
