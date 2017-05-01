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

extern void khrn_record(khrn_memaccess* ma, const V3D_BIN_RENDER_INFO_T *br_info);

EXTERN_C_END

#endif
