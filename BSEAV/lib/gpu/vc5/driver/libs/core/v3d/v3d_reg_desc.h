/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdint.h>
#include <assert.h>
#ifdef __cplusplus
#include <functional>
#endif
#include "vcos_string.h"

EXTERN_C_BEGIN

extern size_t v3d_sprint_reg(char *buf, size_t buf_size, size_t offset, uint32_t addr);

#define V3D_SPRINT_REG(BUF_NAME, ADDR) \
   VCOS_SAFE_STRFUNC_TO_LOCAL_BUF(BUF_NAME, 64, v3d_sprint_reg, ADDR)

/* Accepts eg:
 * - V3D_HUB_CTL_IDENT3
 * - V3D_CLE_BFC(3)
 * - CLE_BFC (V3D_CLE_BFC(default_core) returned)
 * - CLE_BFC(2) + 0x10
 * Will return UINT32_MAX if desc is not recognised. */
extern uint32_t v3d_try_reg_from_desc(const char *desc, uint32_t default_core);

#ifdef __cplusplus

extern void v3d_for_each_matching_reg(
   const char *pattern, uint32_t default_core_mask,
   const std::function<void(uint32_t)> &f);

#endif

EXTERN_C_END
