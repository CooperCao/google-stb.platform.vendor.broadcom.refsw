/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/
#pragma once

#include <stdint.h>
#include "vcos_string.h"

VCOS_EXTERN_C_BEGIN

extern size_t v3d_sprint_reg(char *buf, size_t buf_size, size_t offset, uint32_t addr);

#define V3D_SPRINT_REG(BUF_NAME, ADDR) \
   VCOS_SAFE_STRFUNC_TO_LOCAL_BUF(BUF_NAME, 64, v3d_sprint_reg, ADDR)

/* Accepts eg:
 * - V3D_HUB_CTL_IDENT3
 * - V3D_CLE_BFC(3)
 * - V3D_CLE_1_BFC
 * - V3D_CLE_BFC(2) + 0x10
 * Will return UINT32_MAX if desc is not recognised. */
extern uint32_t v3d_try_reg_from_desc(const char *desc);

static inline uint32_t v3d_reg_from_desc(const char *desc)
{
   uint32_t addr = v3d_try_reg_from_desc(desc);
   assert(addr != UINT32_MAX);
   return addr;
}

VCOS_EXTERN_C_END
