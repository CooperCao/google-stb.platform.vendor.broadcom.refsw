/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GFX_LFMT_TRANSLATE_GL_H
#define GFX_LFMT_TRANSLATE_GL_H

#include "helpers/gfx/gfx_lfmt.h"
#include "GLES3/gl31.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t gfx_lfmt_translate_gl_ext_t;
#define GFX_LFMT_TRANSLATE_GL_EXT_ASTC       (1u << 0)
#define GFX_LFMT_TRANSLATE_GL_EXT_SR8_SRG8   (1u << 1)
#define GFX_LFMT_TRANSLATE_GL_EXT_ALL        ((1u << 2) - 1)

/* GL "external format" <-> lfmt */
extern GFX_LFMT_T gfx_lfmt_from_externalformat(GLenum format, GLenum type, GLenum internalformat);
extern GFX_LFMT_T gfx_lfmt_from_format_type(GLenum format, GLenum type, bool is_dst_srgb);
extern void gfx_lfmt_to_format_type_maybe(GLenum *format, GLenum *type, GFX_LFMT_T lfmt);
extern void gfx_lfmt_to_format_type(GLenum *format, GLenum *type, GFX_LFMT_T lfmt);

/* sized internalformat -> api fmt */
extern GFX_LFMT_T gfx_api_fmt_from_sized_internalformat_maybe(gfx_lfmt_translate_gl_ext_t exts, GLenum internalformat);
extern GFX_LFMT_T gfx_api_fmt_from_sized_internalformat(gfx_lfmt_translate_gl_ext_t exts, GLenum internalformat);
extern GFX_LFMT_T gfx_api_fmt_from_internalformat(
   gfx_lfmt_translate_gl_ext_t exts,
   GLenum type,
   GLenum internalformat);
extern GLenum gfx_sized_internalformat_from_api_fmt_maybe(GFX_LFMT_T api_fmt);
extern GLenum gfx_unsized_internalformat_from_api_fmt_maybe(GFX_LFMT_T api_fmt);
extern GLenum gfx_internalformat_from_api_fmt_maybe(GFX_LFMT_T api_fmt);
extern GLenum gfx_internalformat_from_api_fmt(GFX_LFMT_T api_fmt);
extern bool gfx_gl_is_sized_internalformat(gfx_lfmt_translate_gl_ext_t exts, GLenum internalformat);
extern bool gfx_gl_is_unsized_internalformat(GLenum internalformat);

/* compressed texture formats */
extern unsigned int gfx_compressed_format_enumerate(GLenum *formats, gfx_lfmt_translate_gl_ext_t exts);
extern GFX_LFMT_T gfx_lfmt_from_compressed_internalformat_maybe(gfx_lfmt_translate_gl_ext_t exts, GLenum internalformat);
static inline GFX_LFMT_T gfx_lfmt_from_compressed_internalformat(gfx_lfmt_translate_gl_ext_t exts, GLenum internalformat)
{
   GFX_LFMT_T fmt = gfx_lfmt_from_compressed_internalformat_maybe(exts, internalformat);
   assert(fmt != GFX_LFMT_NONE);
   return fmt;
}
static inline bool gfx_gl_is_compressed_internalformat(gfx_lfmt_translate_gl_ext_t exts, GLenum internalformat)
{
   GFX_LFMT_T fmt = gfx_lfmt_from_compressed_internalformat_maybe(exts, internalformat);
   return fmt != GFX_LFMT_NONE;
}
static inline bool gfx_gl_is_paletted_compressed_internalformat(gfx_lfmt_translate_gl_ext_t exts, GLenum internalformat)
{
   GFX_LFMT_T fmt = gfx_lfmt_from_compressed_internalformat_maybe(exts, internalformat);
   return (fmt != GFX_LFMT_NONE) && gfx_lfmt_is_paletted(fmt);
}

extern GLenum gfx_compressed_internalformat_from_api_fmt_maybe(GFX_LFMT_T api_fmt);

/* valid combinations of (format, type, internalformat) for glTexImage */
extern bool gfx_lfmt_gl_format_type_internalformat_combination_valid(
   gfx_lfmt_translate_gl_ext_t exts,
   GLenum format, GLenum type, GLenum internalformat);
extern bool gfx_lfmt_gl_format_type_internalformat_valid_enums(
   gfx_lfmt_translate_gl_ext_t exts,
   GLenum format, GLenum type, GLenum internalformat);

/* Check that the combination of format, type are valid */
extern bool gfx_gl_format_type_combination_valid(gfx_lfmt_translate_gl_ext_t exts, GLenum format, GLenum type);
extern bool gfx_gl_format_type_valid_enums(gfx_lfmt_translate_gl_ext_t exts, GLenum format, GLenum type);

extern GLenum gfx_lfmt_gl_format_type_to_internalformat(gfx_lfmt_translate_gl_ext_t exts, GLenum format, GLenum type);
extern bool gfx_gl_is_texture_internalformat(gfx_lfmt_translate_gl_ext_t exts, GLenum internalformat);
extern void gfx_gl_sizedinternalformat_to_format_type(gfx_lfmt_translate_gl_ext_t exts,
      GLenum sizedinternalformat, GLenum *format, GLenum *type);

#ifdef __cplusplus
}
#endif

#endif
