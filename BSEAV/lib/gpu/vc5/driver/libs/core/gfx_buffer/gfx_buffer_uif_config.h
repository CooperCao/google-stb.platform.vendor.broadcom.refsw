/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#ifdef GFX_UIF_SYSTEM_CONFIG_AT_RUNTIME

#include <stdint.h>
#include "vcos_types.h"

EXTERN_C_BEGIN

extern uint32_t gfx_buffer_get_uif_page_size(void);
extern void gfx_buffer_set_uif_page_size(uint32_t page_size);
extern uint32_t gfx_buffer_get_uif_num_banks(void);
extern void gfx_buffer_set_uif_num_banks(uint32_t num_banks);
extern uint32_t gfx_buffer_get_uif_xor_addr(void);
extern void gfx_buffer_set_uif_xor_addr(uint32_t xor_addr);

EXTERN_C_END

#define GFX_UIF_PAGE_SIZE gfx_buffer_get_uif_page_size()
#define GFX_UIF_NUM_BANKS gfx_buffer_get_uif_num_banks()
#define GFX_UIF_XOR_ADDR gfx_buffer_get_uif_xor_addr()

#else

#define GFX_UIF_PAGE_SIZE GFX_DEFAULT_UIF_PAGE_SIZE
#define GFX_UIF_NUM_BANKS GFX_DEFAULT_UIF_NUM_BANKS
#define GFX_UIF_XOR_ADDR GFX_DEFAULT_UIF_XOR_ADDR

#endif
