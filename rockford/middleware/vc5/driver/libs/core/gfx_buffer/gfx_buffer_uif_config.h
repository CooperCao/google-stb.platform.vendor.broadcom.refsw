/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/
#pragma once

#ifdef GFX_UIF_SYSTEM_CONFIG_AT_RUNTIME

#include <stdint.h>
#include "vcos_types.h"

VCOS_EXTERN_C_BEGIN

extern uint32_t gfx_buffer_get_uif_page_size(void);
extern void gfx_buffer_set_uif_page_size(uint32_t page_size);
extern uint32_t gfx_buffer_get_uif_num_banks(void);
extern void gfx_buffer_set_uif_num_banks(uint32_t num_banks);
extern uint32_t gfx_buffer_get_uif_xor_addr(void);
extern void gfx_buffer_set_uif_xor_addr(uint32_t xor_addr);

/* Applies to SAND layouts. Either 2 (MAP 2.0), 5 (MAP 5.0), or 8 (MAP 8.0). */
extern uint32_t gfx_buffer_get_dram_map_mode(void);
extern void gfx_buffer_set_dram_map_mode(uint32_t map_mode);

VCOS_EXTERN_C_END

#define GFX_UIF_PAGE_SIZE gfx_buffer_get_uif_page_size()
#define GFX_UIF_NUM_BANKS gfx_buffer_get_uif_num_banks()
#define GFX_UIF_XOR_ADDR gfx_buffer_get_uif_xor_addr()

#define GFX_DRAM_MAP_MODE gfx_buffer_get_dram_map_mode()

#else

#define GFX_UIF_PAGE_SIZE GFX_DEFAULT_UIF_PAGE_SIZE
#define GFX_UIF_NUM_BANKS GFX_DEFAULT_UIF_NUM_BANKS
#define GFX_UIF_XOR_ADDR GFX_DEFAULT_UIF_XOR_ADDR

#define GFX_DRAM_MAP_MODE GFX_DEFAULT_DRAM_MAP_MODE

#endif
