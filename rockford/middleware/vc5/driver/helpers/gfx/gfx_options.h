/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GFX_OPTIONS_H
#define GFX_OPTIONS_H

#include "vcos_types.h"

VCOS_EXTERN_C_BEGIN

#include <stdbool.h>
#include <stdint.h>

extern void gfx_options_begin(const char *name);
extern void gfx_options_end(void);

extern bool gfx_options_bool(const char *name, bool def);
extern uint32_t gfx_options_uint32(const char *name, uint32_t def);
extern uint64_t gfx_options_uint64(const char *name, uint64_t def);
extern int32_t gfx_options_int32(const char *name, int32_t def);
extern int gfx_options_str(const char *name, const char *def, char * value, size_t value_size);
extern int gfx_options_enum(const char *name, int def,
   /* List of (enum_name, enum_value) pairs ending with enum_name == NULL */
   ...);

VCOS_EXTERN_C_END

#endif
