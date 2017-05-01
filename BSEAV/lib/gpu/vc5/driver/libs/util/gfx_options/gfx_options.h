/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "vcos.h"
#include "libs/util/log/log.h"

#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
#include <string>
#endif

VCOS_EXTERN_C_BEGIN

extern bool gfx_options_bool_log_cat(struct log_cat *cat, const char *name, bool def);
extern uint32_t gfx_options_uint32_log_cat(struct log_cat *cat, const char *name, uint32_t def);
extern uint64_t gfx_options_uint64_log_cat(struct log_cat *cat, const char *name, uint64_t def);
extern int32_t gfx_options_int32_log_cat(struct log_cat *cat, const char *name, int32_t def);
extern double gfx_options_double_log_cat(struct log_cat *cat, const char *name, double def);
extern void gfx_options_str_log_cat(struct log_cat *cat, const char *name, const char *def, char * value, size_t value_size);
extern int gfx_options_enum_log_cat(struct log_cat *cat, const char *name, int def,
   /* List of (enum_name, enum_value) pairs ending with enum_name == NULL */
   ...);

VCOS_EXTERN_C_END

#ifdef __cplusplus
static inline std::string gfx_options_str_cpp_log_cat(struct log_cat *cat, const char *name, const char *def)
{
   char value[VCOS_PROPERTY_VALUE_MAX];
   gfx_options_str_log_cat(cat, name, def, value, sizeof(value));
   return value;
}
#endif

#define gfx_options_bool(...)    gfx_options_bool_log_cat(&log_default_cat, __VA_ARGS__)
#define gfx_options_uint32(...)  gfx_options_uint32_log_cat(&log_default_cat, __VA_ARGS__)
#define gfx_options_uint64(...)  gfx_options_uint64_log_cat(&log_default_cat, __VA_ARGS__)
#define gfx_options_int32(...)   gfx_options_int32_log_cat(&log_default_cat, __VA_ARGS__)
#define gfx_options_double(...)  gfx_options_double_log_cat(&log_default_cat, __VA_ARGS__)
#define gfx_options_str(...)     gfx_options_str_log_cat(&log_default_cat, __VA_ARGS__)
#define gfx_options_enum(...)    gfx_options_enum_log_cat(&log_default_cat, __VA_ARGS__)
#ifdef __cplusplus
#define gfx_options_str_cpp(...) gfx_options_str_cpp_log_cat(&log_default_cat, __VA_ARGS__)
#endif
