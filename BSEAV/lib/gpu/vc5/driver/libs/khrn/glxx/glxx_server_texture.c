/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <string.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#include "vcos.h"

#include "../common/khrn_int_common.h"
#include "../common/khrn_int_util.h"
#include "../gl11/gl11_int_config.h"
#include "libs/util/dglenum/dglenum.h"

#include "../common/khrn_resource.h"
#include "../gl20/gl20_program.h"

#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"
#include "libs/core/lfmt/lfmt_fmt_detail.h"
#include "libs/util/profile/profile.h"
#include "libs/util/gfx_util/gfx_util_conv.h"

#include "glxx_shared.h"
#include "glxx_server.h"
#include "glxx_server_internal.h"
#include "glxx_texture.h"
#include "glxx_buffer.h"
#include "glxx_hw.h"
#include "glxx_renderbuffer.h"
#include "glxx_framebuffer.h"
#include "glxx_translate.h"
#include "glxx_server_texture.h"

#include "libs/khrn/egl/egl_config.h"

static bool is_target_1d(GLenum target)
{
   if (target == GL_TEXTURE_1D_BRCM)
      return true;
   return false;
}

static bool is_target_2d(GLenum target)
{
   switch (target)
   {
      case GL_TEXTURE_2D:
      case GL_TEXTURE_1D_ARRAY_BRCM:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
         return true;
      default:
         return false;
   }
}

static bool is_target_3d(GLenum target)
{
   switch (target)
   {
   case GL_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_3D:
      return true;
#if V3D_VER_AT_LEAST(4,1,34,0)
   case GL_TEXTURE_CUBE_MAP_ARRAY:
      return (V3D_VER_AT_LEAST(3,3,0,0) ? true : false);
#endif
   default:
      return false;
   }
}

GLXX_TEXTURE_T* glxx_server_state_get_texture(GLXX_SERVER_STATE_T *state,
      GLenum target)
{
   if (!glxx_texture_is_tex_target(state, target))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return NULL;
   }

   return glxx_server_get_active_texture(state, target);
}

GLXX_TEXTURE_T* glxx_server_get_active_texture(const GLXX_SERVER_STATE_T *state,
      enum glxx_tex_target target)
{
   return glxx_textures_get_texture(&state->bound_texture[state->active_texture], target);
}

static int get_target_dimensions(GLenum target)
{
   if (is_target_3d(target)) return 3;
   if (is_target_2d(target)) return 2;
   if (is_target_1d(target)) return 1;
   unreachable();
   return 0;
}

static bool copytex_fb_complete_check(GLXX_SERVER_STATE_T *state)
{
   GLXX_FRAMEBUFFER_T *fb = state->bound_read_framebuffer;

   if (!glxx_fb_is_complete(fb, &state->fences))
   {
      glxx_server_state_set_error(state, GL_INVALID_FRAMEBUFFER_OPERATION);
      return false;
   }
   if (fb->name != 0 && (glxx_fb_get_ms_mode(fb) != GLXX_NO_MS))
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return false;
   }

   return true;
}

/* returns true if this target is a valid target for gl*TexImage* functions */
static bool is_supported_teximage_target(const GLXX_SERVER_STATE_T *state, GLenum target)
{
   if (IS_GL_11(state) && target != GL_TEXTURE_2D)
      return false;

   switch(target)
   {
#if V3D_VER_AT_LEAST(4,1,34,0)
      case GL_TEXTURE_1D_BRCM:
      case GL_TEXTURE_1D_ARRAY_BRCM:
#endif
      case GL_TEXTURE_2D:
      case GL_TEXTURE_3D:
      case GL_TEXTURE_2D_ARRAY:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
         return true;
#if V3D_VER_AT_LEAST(4,1,34,0)
      case GL_TEXTURE_CUBE_MAP_ARRAY:
         return  (V3D_VER_AT_LEAST(3,3,0,0) ? true : false);
#endif
      default:
         return false;
   }
}

/* format, type and internalformat can be 0 if the gl function doesn't supply
 * one (eg: SubImage* doesn't have internal format as param,
 * compressedTexSubImage supplies only format as a param, compressedTexImage
 * supplies only internalformat as a param;
 * copyTexImage supplies only internalformat; copyTexSubImage doesn't supply
 * any format, type or internalformat;)
 * It's the caller's responsibility to check that format, type and internalformat
 * are valid and consistent with each other. */
bool glxx_teximage_internal_checks(GLXX_SERVER_STATE_T *state,
      const struct glxx_teximage_sanity_checks *checks,
      GLenum target, GLenum format, GLenum type, GLenum internalformat,
      GLsizei level, GLsizei width_i, GLsizei height_i, GLsizei depth_i,
      GLint xoffset_i, GLint yoffset_i, GLint zoffset_i, GLint border,
      GLXX_TEXTURE_T **tex_out, GLenum *error)
{
   GLXX_TEXTURE_T *texture = NULL;
   khrn_image *img = NULL;
   unsigned start_elem, num_array_elems;
   enum glxx_tex_target tex_target;
   unsigned width, height, depth, xoffset, yoffset, zoffset;

   if (!is_supported_teximage_target(state, target))
   {
      *error = GL_INVALID_ENUM;
      return false;
   }

   if (glxx_texture_is_cube_face(target))
      tex_target = GL_TEXTURE_CUBE_MAP;
   else
      tex_target = target;

   assert(checks->dimensions > 0 && checks->dimensions <= 3);
   if (get_target_dimensions(target) != checks->dimensions)
   {
      *error = GL_INVALID_ENUM;
      return false;
   }

   if (!glxx_texture_is_legal_level(target, level))
   {
      *error = GL_INVALID_VALUE;
      return false;
   }

   if (!glxx_texture_are_legal_dimensions(target, width_i, height_i, depth_i) ||
        xoffset_i < 0 || yoffset_i < 0 || zoffset_i < 0)
   {
      *error = GL_INVALID_VALUE;
      return false;
   }

   width = width_i; height = height_i;  depth = depth_i;
   xoffset = xoffset_i;  yoffset = yoffset_i; zoffset = zoffset_i;

   if (border != 0)
   {
      *error = GL_INVALID_VALUE;
      return false;
   }

   texture = glxx_server_get_active_texture(state, tex_target);
   assert(texture);
   *tex_out = texture;

   glxx_tex_transform_dim_for_target(tex_target, &width,
         &height, &depth, &num_array_elems);
   glxx_tex_transform_offsets_for_target(tex_target, &xoffset,
         &yoffset, &zoffset, &start_elem);

   GFX_LFMT_T compressed_fmt; /* GFX_LFMT_NONE if not compressed */
   if (checks->respecify)
   {
      if (texture->immutable_format != GFX_LFMT_NONE)
      {
         /* Can't respecify an immutable texture */
         *error = GL_INVALID_OPERATION;
         return false;
      }

      if ((tex_target == GL_TEXTURE_CUBE_MAP || tex_target == GL_TEXTURE_CUBE_MAP_ARRAY)
           && (width != height))
      {
         /* Cubemaps must be specified with equal width and height */
         *error = GL_INVALID_VALUE;
         return false;
      }

      if (target == GL_TEXTURE_CUBE_MAP_ARRAY && ((num_array_elems % 6) != 0) )
      {
         *error = GL_INVALID_VALUE;
         return false;
      }

      compressed_fmt = gfx_lfmt_from_compressed_internalformat_maybe(
         khrn_get_lfmt_translate_exts(), internalformat);

      if (checks->compressed != (compressed_fmt != GFX_LFMT_NONE))
      {
         /* Either supplied a compressed format where not permitted or supplied
          * a non-compressed format where a compressed format is required. Note
          * that some functions (eg glTexStorage) permit both compressed and
          * non-compressed formats; in those cases checks->compressed will be
          * set by the caller based on whether or not the incoming format is
          * compressed, so we will never get here. */
         *error = GL_INVALID_ENUM;
         return false;
      }
   }
   else
   {
      unsigned face = glxx_texture_get_face(target);

      /* SubImage gl api call does not not supply an internalformat */
      assert(internalformat == 0);

      /* we must already have an image */
      img = texture->img[face][level];
      if (img == NULL)
      {
         *error = GL_INVALID_OPERATION;
         return false;
      }

      compressed_fmt = gfx_lfmt_is_compressed(img->api_fmt) ? img->api_fmt : GFX_LFMT_NONE;

      if (checks->compressed != (compressed_fmt != GFX_LFMT_NONE))
      {
         *error = GL_INVALID_OPERATION;
         return false;
      }

      if (checks->compressed)
      {
         if (gfx_lfmt_is_paletted(compressed_fmt))
         {
            *error = GL_INVALID_OPERATION;
            return false;
         }

         /* compressedTexSubImage commands do not provide for image conversion,
          * hence format must match the internal format the image being modified
          */
         if (gfx_lfmt_from_compressed_internalformat(
            khrn_get_lfmt_translate_exts(), format) != compressed_fmt)
         {
            *error = GL_INVALID_OPERATION;
            return false;
         }
      }
   }

   if (checks->compressed && target == GL_TEXTURE_3D)
   {
      *error = GL_INVALID_OPERATION;
      return false;
   }

   if (!checks->respecify)
   {
      unsigned img_width, img_height, img_depth, img_elems;
      unsigned face = glxx_texture_get_face(target);

      img = texture->img[face][level];
      assert(img != NULL);


      khrn_image_get_dimensions(img, &img_width, &img_height, &img_depth,
            &img_elems);
      if (checks->compressed)
      {
         GFX_LFMT_BASE_DETAIL_T bd;
         gfx_lfmt_base_detail(&bd, compressed_fmt);

         /* Offsets must be block aligned.
          * Width/height/depth must be a multiple of blocks or hit the edge of
          * the image. */
         if (((xoffset % bd.block_w) != 0) ||
            ((yoffset % bd.block_h) != 0) ||
            ((zoffset % bd.block_d) != 0) ||
            (((width % bd.block_w) != 0) && (xoffset + width != img_width)) ||
            (((height % bd.block_h) != 0) && (yoffset + height != img_height)) ||
            (((depth % bd.block_d) != 0) && (zoffset + depth != img_depth)))
         {
            *error = GL_INVALID_OPERATION;
            return false;
         }
      }

      if (start_elem + num_array_elems > img_elems)
      {
         *error = GL_INVALID_VALUE;
         return false;
      }

      if (xoffset + width  > img_width  ||
          yoffset + height > img_height ||
          zoffset + depth  > img_depth)
      {
         *error = GL_INVALID_VALUE;
         return false;
      }
   }

   if (!checks->compressed && checks->respecify)
   {
      /* this makes sense only for texImage and texStorage2D/3D */
      if ((format == GL_DEPTH_COMPONENT || format == GL_DEPTH_STENCIL ||
           format == GL_STENCIL_INDEX) &&
          (tex_target != GL_TEXTURE_2D && tex_target != GL_TEXTURE_2D_ARRAY &&
           tex_target != GL_TEXTURE_CUBE_MAP && tex_target != GL_TEXTURE_CUBE_MAP_ARRAY))
      {
         *error = GL_INVALID_OPERATION;
         return false;
      }
   }

   return true;
}

static bool glxx_teximage_unpack_buffer_checks(GLenum type, const GLvoid* pixels,
                                               GLXX_BUFFER_T *pixel_buffer, GLenum *error)
{
   if (pixel_buffer != NULL)
   {
      uintptr_t mask = 0;

      // Any GL command which attempts to read from, write to, or change the state of
      // a buffer object may generate an INVALID_OPERATION error if all or part of the
      // buffer object is mapped
      if (pixel_buffer->mapped_pointer != NULL)
      {
         *error = GL_INVALID_OPERATION;
         return false;
      }

      // An INVALID_OPERATION error is generated if a pixel unpack buffer object is bound and
      // data is not evenly divisible by the number of basic machine units needed to store in
      // memory the corresponding GL data type from table 8.4 for the type parameter(or  not
      // evenly divisible by 4 for type FLOAT_32_UNSIGNED_INT_24_8_REV, which does not have a
      // corresponding GL data type)
      switch (type)
      {
      case GL_UNSIGNED_BYTE:
      case GL_BYTE:
         mask = 0x0;
         break;
      case GL_UNSIGNED_SHORT:
      case GL_SHORT:
      case GL_HALF_FLOAT:
      case GL_UNSIGNED_SHORT_5_6_5:
      case GL_UNSIGNED_SHORT_4_4_4_4:
      case GL_UNSIGNED_SHORT_5_5_5_1:
         mask = 0x1;
         break;
      case GL_UNSIGNED_INT:
      case GL_INT:
      case GL_FLOAT:
      case GL_UNSIGNED_INT_2_10_10_10_REV:
      case GL_UNSIGNED_INT_24_8:
      case GL_UNSIGNED_INT_10F_11F_11F_REV:
      case GL_UNSIGNED_INT_5_9_9_9_REV:
      case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
         mask = 0x3;
         break;
      case 0:
         break;
      default:
         unreachable();
      }

      if (((uintptr_t)pixels & mask) != 0)
      {
         *error = GL_INVALID_OPERATION;
         return false;
      }
   }

   return true;
}

static void texImageX(GLenum target, GLint level, GLint
         internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint
         border, GLenum format, GLenum type, const GLvoid* pixels, unsigned dim)
{
   GLXX_SERVER_STATE_T *state   = glxx_lock_server_state(OPENGL_ES_ANY);
   GLXX_TEXTURE_T      *texture = NULL;
   GLenum               error   = GL_NO_ERROR;

   if (!state)
      return;

   if (!gfx_lfmt_gl_format_type_internalformat_combination_valid(format, type, internalformat))
   {
      bool format_type_ok = false;
      bool internalformat_ok = gfx_lfmt_gl_format_type_internalformat_valid_enums(
         format, type, internalformat, &format_type_ok);

      if (format_type_ok && internalformat_ok)
         error = GL_INVALID_OPERATION;
      else if (internalformat_ok)
         error = GL_INVALID_ENUM;
      else
         error = GL_INVALID_VALUE;

      goto end;
   }

   if (!glxx_teximage_unpack_buffer_checks(type, pixels,
      state->bound_buffer[GLXX_BUFTGT_PIXEL_UNPACK].obj, &error))
      goto end;

   struct glxx_teximage_sanity_checks checks;
   checks.dimensions = dim;
   checks.compressed = false;
   checks.respecify = true;

   if (!glxx_teximage_internal_checks(state, &checks, target,
         format, type, internalformat, level, width, height, depth, 0, 0, 0,
         border, &texture, &error))
      goto end;

   unsigned face = glxx_texture_get_face(target);

   glxx_texture_image(texture, face, level, internalformat, width, height,
         depth, format, type, &(state->pixel_store_state),
         state->bound_buffer[GLXX_BUFTGT_PIXEL_UNPACK].obj, pixels,
         &state->fences, &error);

end:

   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   glxx_unlock_server_state();
}

#if V3D_VER_AT_LEAST(4,1,34,0)
GL_APICALL void GL_APIENTRY glTexImage1DBRCM(GLenum target, GLint level, GLint internalformat,
            GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
   PROFILE_FUNCTION_MT("GL");
   texImageX(target, level, internalformat, width, 1, 1, border,
         format, type, pixels, 1);
}
#endif

GL_API void GL_APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat,
      GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
   PROFILE_FUNCTION_MT("GL");
   texImageX(target, level, internalformat, width, height, 1, border,
         format, type, pixels, 2);
}

GL_API void GL_APIENTRY glTexImage3D(GLenum target, GLint level, GLint internalformat,
      GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
   PROFILE_FUNCTION_MT("GL");
   texImageX(target, level, internalformat, width, height, depth, border,
         format, type, pixels, 3);
}

static bool is_supported_texstorage_target(GLenum target, unsigned dim)
{
   switch(dim)
   {
      /* 2D targets don't follow the pattern because we must exclude cubemap faces */
      case 1: return is_target_1d(target);
      case 2: return (target == GL_TEXTURE_2D || target == GL_TEXTURE_CUBE_MAP || target == GL_TEXTURE_1D_ARRAY_BRCM);
      case 3: return is_target_3d(target);
      default: return false;
   }
}

/* supports compressed internalformat and sized internal format */
static bool texstorage_internal_checks(GLXX_SERVER_STATE_T *state,
      GLenum target, GLsizei levels, GLenum internalformat,
      GLsizei width, GLsizei height, GLsizei depth, unsigned dim,
      GLXX_TEXTURE_T **texture, GLenum *error)
{
   unsigned max_levels;
   struct glxx_teximage_sanity_checks teximage_checks;
   bool ok, compressed, sized_intfmt;

   compressed = false;
   sized_intfmt = false;

   assert(dim == 1 || dim == 2 || dim == 3);

   if (!is_supported_texstorage_target(target, dim))
   {
      *error = GL_INVALID_ENUM;
      return false;
   }

   if (width < 1 || height < 1 || depth < 1 || levels < 1)
   {
      *error = GL_INVALID_VALUE;
      return false;
   }

   compressed = gfx_gl_is_compressed_internalformat(
      khrn_get_lfmt_translate_exts(), internalformat);
   if (!compressed)
      sized_intfmt = gfx_gl_is_sized_internalformat(internalformat);

   if (!compressed && !sized_intfmt)
   {
      *error = GL_INVALID_ENUM;
      return false;
   }

   teximage_checks.dimensions = dim;
   teximage_checks.compressed = compressed;
   teximage_checks.respecify = true;

   GLenum format = GL_NONE;
   GLenum type = GL_NONE;
   if (sized_intfmt)
   {
      gfx_gl_sizedinternalformat_to_format_type(internalformat, &format, &type);
   }
   else
      format = internalformat;

   ok = glxx_teximage_internal_checks(state, &teximage_checks,
         (target != GL_TEXTURE_CUBE_MAP) ? target : GL_TEXTURE_CUBE_MAP_POSITIVE_X,
         format, type,  internalformat, levels -1, width, height, depth, 0, 0, 0, 0,
         texture, error);
   if (!ok)
      return false;

   max_levels = glxx_tex_max_levels_from_dim(target, width, height, depth);
   if ((unsigned int)levels > max_levels)
   {
      *error = GL_INVALID_OPERATION;
      return false;
   }

   /* if 0 is bound to target, generate error */
   if ((*texture)->name == 0)
   {
      *error = GL_INVALID_OPERATION;
      return false;
   }

   return true;
}

static void texstorage(GLenum target, GLsizei levels, GLenum internalformat,
      GLsizei width, GLsizei height, GLsizei depth, unsigned dim)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   GLXX_TEXTURE_T *texture;
   GLenum error = GL_NO_ERROR;
   bool ok;

   if (!state) return;

   ok = texstorage_internal_checks(state, target, levels, internalformat, width,
         height, depth, dim, &texture, &error);
   if (!ok)
      goto end;

   ok = glxx_texture_storage(texture, levels, internalformat, width, height, depth,
         GLXX_NO_MS, true);
   if (!ok)
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   glxx_unlock_server_state();
}

#if V3D_VER_AT_LEAST(4,1,34,0)
GL_API void GL_APIENTRY glTexStorage1DBRCM(GLenum target, GLsizei levels,
      GLenum internalformat, GLsizei width)
{
   PROFILE_FUNCTION_MT("GL");
   texstorage(target, levels, internalformat, width, 1, 1, 1);
}
#endif

GL_API void GL_APIENTRY glTexStorage2D(GLenum target, GLsizei levels,
      GLenum internalformat, GLsizei width, GLsizei height)
{
   PROFILE_FUNCTION_MT("GL");
   texstorage(target, levels, internalformat, width, height, 1, 2);
}

GL_API void GL_APIENTRY glTexStorage3D(GLenum target, GLsizei levels,
      GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
   PROFILE_FUNCTION_MT("GL");
   texstorage(target, levels, internalformat, width, height, depth, 3);
}

static bool texstorage_ms_internal_check(GLXX_SERVER_STATE_T *state, GLenum target,
      GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth,
      GLboolean fixedsamplelocations,
      GLXX_TEXTURE_T **texture, GLenum *error)
{
   if (width < 1 || height < 1 || depth < 1 ||
      !glxx_texture_are_legal_dimensions(target, width, height, depth))
   {
      *error = GL_INVALID_VALUE;
      return false;
   }

   if (samples <= 0)
   {
      *error = GL_INVALID_VALUE;
      return false;
   }

   if (!glxx_is_color_renderable_internalformat(internalformat) &&
       !glxx_is_depth_renderable_internalformat(internalformat) &&
       !glxx_is_stencil_renderable_internalformat(internalformat))
   {
      *error = GL_INVALID_ENUM;
      return false;
   }

   if (gfx_gl_is_unsized_internalformat(internalformat))
   {
      *error = GL_INVALID_ENUM;
      return false;
   }

   glxx_ms_mode max_ms_mode = glxx_max_ms_mode_for_internalformat(internalformat);
   // NOTE: gcc and MSVC disagree on whether glxx_ms_mode is signed or not
   if ((unsigned)samples > (unsigned)max_ms_mode)
   {
      *error = GL_INVALID_OPERATION;
      return false;
   }

   *texture = glxx_server_get_active_texture(state, target);
   assert(*texture);
   if ((*texture)->name == 0)
   {
      *error = GL_INVALID_OPERATION;
      return false;
   }

   if ((*texture)->immutable_format)
   {
      *error = GL_INVALID_OPERATION;
      return false;
   }
   return true;
}

#if V3D_VER_AT_LEAST(3,3,0,0)

GL_API void GL_APIENTRY glTexStorage2DMultisample(GLenum target, GLsizei samples,
      GLenum internalformat, GLsizei width, GLsizei height,
      GLboolean fixedsamplelocations)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   GLenum error = GL_NO_ERROR;
   if (target != GL_TEXTURE_2D_MULTISAMPLE)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   GLXX_TEXTURE_T *texture;
   bool ok = texstorage_ms_internal_check(state, target, samples, internalformat,
         width, height, 1, fixedsamplelocations, &texture, &error);
   if (!ok)
      goto end;

   glxx_ms_mode ms_mode = glxx_samples_to_ms_mode(samples);
   ok = glxx_texture_storage(texture, 1, internalformat, width, height, 1, ms_mode,
         fixedsamplelocations);
   if (!ok)
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   glxx_unlock_server_state();
}

#endif

void tex_storage_3d_multisample(GLXX_SERVER_STATE_T *state, GLenum target, GLsizei samples,
                                GLenum internalformat, GLsizei width, GLsizei height,
                                GLsizei depth, GLboolean fixedsamplelocations)
{
   PROFILE_FUNCTION_MT("GL");

   if (target != GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return;
   }

   GLXX_TEXTURE_T *texture;
   GLenum error = GL_NO_ERROR;
   if (!texstorage_ms_internal_check(state, target, samples, internalformat,
         width, height, depth, fixedsamplelocations, &texture, &error))
   {
      glxx_server_state_set_error(state, error);
      return;
   }

   glxx_ms_mode ms_mode = glxx_samples_to_ms_mode(samples);
   if (!glxx_texture_storage(texture, 1, internalformat, width, height, depth, ms_mode,
         fixedsamplelocations))
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      return;
   }
}

#if KHRN_GLES32_DRIVER

GL_API void GL_APIENTRY glTexStorage3DMultisample(GLenum target, GLsizei samples,
      GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth,
      GLboolean fixedsamplelocations)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   tex_storage_3d_multisample(state, target, samples, internalformat,
                              width, height, depth, fixedsamplelocations);

   glxx_unlock_server_state();
}

#endif

#if V3D_VER_AT_LEAST(3,3,0,0)

GL_API void GL_APIENTRY glTexStorage3DMultisampleOES(GLenum target, GLsizei samples,
      GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth,
      GLboolean fixedsamplelocations)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   tex_storage_3d_multisample(state, target, samples, internalformat,
                              width, height, depth, fixedsamplelocations);

   glxx_unlock_server_state();
}

#endif

static void compressedTexImageX(GLenum target, GLint level,
      GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border,
      GLsizei imageSize, const GLvoid *pixels, unsigned dim)
{
   GLXX_SERVER_STATE_T *state   = glxx_lock_server_state(OPENGL_ES_ANY);
   GLXX_TEXTURE_T *texture = NULL;
   GLenum error   = GL_NO_ERROR;
   bool ok;
   unsigned face;
   struct glxx_teximage_sanity_checks checks;

   if (!state)
      return;

   checks.dimensions = dim;
   checks.compressed = true;
   checks.respecify = true;

   if (imageSize < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (!glxx_teximage_unpack_buffer_checks(0, pixels,
      state->bound_buffer[GLXX_BUFTGT_PIXEL_UNPACK].obj, &error))
      goto end;

   ok = glxx_teximage_internal_checks(state, &checks,
         target, 0, 0, internalformat, level, width, height, depth, 0, 0, 0,
         border, &texture, &error);
   if (!ok)
      goto end;

   face = glxx_texture_get_face(target);
   ok = glxx_texture_compressed_image(texture, face, level, internalformat,
         width, height, depth, imageSize,
         state->bound_buffer[GLXX_BUFTGT_PIXEL_UNPACK].obj, pixels,
         &state->fences, &error);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   glxx_unlock_server_state();

}

GL_API void GL_APIENTRY glCompressedTexImage2D(GLenum target, GLint level,
      GLenum internalformat, GLsizei width, GLsizei height, GLint border,
      GLsizei imageSize, const GLvoid *pixels)

{
   PROFILE_FUNCTION_MT("GL");

   if (gfx_gl_is_paletted_compressed_internalformat(
         khrn_get_lfmt_translate_exts(), internalformat))
      glxx_compressed_paletted_teximageX(target, level, internalformat, width,
         height, 1, border, imageSize, pixels, 2);
   else
      compressedTexImageX(target, level, internalformat, width, height, 1, border,
         imageSize, pixels, 2);
}

GL_API void GL_APIENTRY glCompressedTexImage3D(GLenum target, GLint level,
      GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border,
      GLsizei imageSize, const GLvoid *pixels)
{
   PROFILE_FUNCTION_MT("GL");

   compressedTexImageX(target, level, internalformat, width, height, depth,
         border, imageSize, pixels, 3);
}

static void  compressedTexSubImageX(GLenum target, GLint level, GLint xoffset,
      GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei
      depth, GLenum format, GLsizei imageSize, const GLvoid* orig_pixels,
      unsigned dim)
{
   GLXX_SERVER_STATE_T  *state   = glxx_lock_server_state(OPENGL_ES_ANY);
   GLXX_TEXTURE_T *texture = NULL;
   GLenum error    = GL_NO_ERROR;
   GLvoid *pixels  = (GLvoid *)orig_pixels;
   struct glxx_teximage_sanity_checks checks;
   unsigned face;

   if (!state)
      return;

   checks.dimensions = dim;
   checks.compressed = true;
   checks.respecify = false;

   if (!glxx_teximage_unpack_buffer_checks(0, pixels,
      state->bound_buffer[GLXX_BUFTGT_PIXEL_UNPACK].obj, &error))
      goto end;

   if (!glxx_teximage_internal_checks(state, &checks, target, format, 0, 0,
         level, width, height, depth, xoffset, yoffset, zoffset, 0, &texture,
         &error))
      goto end;

   face = glxx_texture_get_face(target);
   glxx_texture_compressed_subimage(texture, face, level,
            xoffset, yoffset, zoffset,
            width, height, depth, format, imageSize,
            state->bound_buffer[GLXX_BUFTGT_PIXEL_UNPACK].obj, pixels,
            &state->fences, &error);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   if (pixels != orig_pixels)
      free(pixels);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glCompressedTexSubImage2D(GLenum target, GLint level,
      GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum
      format, GLsizei imageSize, const GLvoid *orig_pixels)
{
   PROFILE_FUNCTION_MT("GL");
   compressedTexSubImageX(target, level, xoffset, yoffset, 0, width, height, 1,
         format, imageSize, orig_pixels, 2);
}

GL_API void GL_APIENTRY glCompressedTexSubImage3D(GLenum target, GLint level,
      GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei
      height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid*
      orig_pixels)
{
   PROFILE_FUNCTION_MT("GL");
   compressedTexSubImageX(target, level, xoffset, yoffset, zoffset, width,
         height, depth, format, imageSize, orig_pixels, 3);
}

typedef enum {
   GLCOPYTEXIMAGE_SLOT_CLASSIFICATION_FLOATISH,
   GLCOPYTEXIMAGE_SLOT_CLASSIFICATION_UNORM,
   GLCOPYTEXIMAGE_SLOT_CLASSIFICATION_SNORM,
   GLCOPYTEXIMAGE_SLOT_CLASSIFICATION_SRGB,
   GLCOPYTEXIMAGE_SLOT_CLASSIFICATION_UINT,
   GLCOPYTEXIMAGE_SLOT_CLASSIFICATION_INT,
} GLCOPYTEXIMAGE_SLOT_CLASSIFICATION_T;

/*
 * ES3 spec: "The error INVALID_OPERATION is generated if signed integer RGBA
 * data is required and the format of the current color buffer is not signed
 * integer [...]"
 */
static GLCOPYTEXIMAGE_SLOT_CLASSIFICATION_T slot_classify_glcopyteximage(GFX_LFMT_TYPE_T slot_type)
{
   switch (slot_type)
   {
   case GFX_LFMT_TYPE_UFLOAT:
   case GFX_LFMT_TYPE_FLOAT:
      return GLCOPYTEXIMAGE_SLOT_CLASSIFICATION_FLOATISH;
   case GFX_LFMT_TYPE_UNORM:
      return GLCOPYTEXIMAGE_SLOT_CLASSIFICATION_UNORM;
   case GFX_LFMT_TYPE_SNORM:
      return GLCOPYTEXIMAGE_SLOT_CLASSIFICATION_SNORM;
   case GFX_LFMT_TYPE_SRGB:
      return GLCOPYTEXIMAGE_SLOT_CLASSIFICATION_SRGB;
   case GFX_LFMT_TYPE_UINT:
      return GLCOPYTEXIMAGE_SLOT_CLASSIFICATION_UINT;
   case GFX_LFMT_TYPE_INT:
      return GLCOPYTEXIMAGE_SLOT_CLASSIFICATION_INT;
   default:
      unreachable();
      return GLCOPYTEXIMAGE_SLOT_CLASSIFICATION_FLOATISH;
   }
}

/* while srgb textures in OpenGL ES 3.0 store the alpha channel as linear,
 * packed_pixels says glCopyTexImage should fail if the user tries to create a
 * linear alpha-only texture from an srgb texture: it cares only about the
 * srgb-ness of the texture as a whole, not of individual channels. So treat
 * all channels (including alpha) as srgb.
 *
 * slot must be from gfx_lfmt_fmt_detail(lfmt)
 */
static GFX_LFMT_TYPE_T get_slot_type_glcopyteximage(const struct gfx_lfmt_slot_detail *slot, GFX_LFMT_T lfmt)
{
   return gfx_lfmt_contains_srgb(lfmt) ? GFX_LFMT_TYPE_SRGB : slot->type;
}

/* return true if glCopyTexImage2D should support this conversion from src to
 * dst. */
static bool glcopyteximage_convert_permitted(
   GFX_LFMT_T dst_lfmt, GFX_LFMT_T src_lfmt,
   bool is_sized_dst_internalformat)
{
   /* ref ES3 spec: "INVALID_OPERATION error if [...] internalformat of the
    * texel array being (re)specified is RGB9_E5.".
    *
    * Also, fmt_detail can't cope with rgb9e5, so we need to bail out early
    * anyway. */
   if (gfx_lfmt_fmt(dst_lfmt) == GFX_LFMT_R9G9B9SHAREDEXP5_UFLOAT)
      return false;

   /*
    * If the destination is unsized, we are
    * not allowed to convert from a GL_RGB10_A2 surface.
    * See GTFTestPackedPixels.c:1738
    */
   if(!is_sized_dst_internalformat)
   {
      if(gfx_lfmt_fmt(src_lfmt) == GFX_LFMT_R10G10B10A2_UNORM)
         return false;
   }

   GFX_LFMT_FMT_DETAIL_T dst_fd, src_fd;
   gfx_lfmt_fmt_detail(&dst_fd, dst_lfmt);
   gfx_lfmt_fmt_detail(&src_fd, src_lfmt);

   for (uint32_t dst_slot_idx = 0; dst_slot_idx != dst_fd.num_slots; ++dst_slot_idx)
   {
      struct gfx_lfmt_slot_detail *dst_slot = &dst_fd.slts[dst_slot_idx];

      GFX_LFMT_CHANNELS_T ch = dst_slot->channel;
      if (ch == GFX_LFMT_CHANNELS_X)
         continue;

      uint32_t src_slot_idx = gfx_lfmt_fmt_detail_get_slot_by_channel_maybe(&src_fd, ch);

      /* ref ES3 spec: "internalformat is further constrained such that color
      buffer components can be dropped during the conversion to internalformat,
      but new components cannot be added [...]" */
      if (src_slot_idx == GFX_LFMT_FMT_DETAIL_INVALID_SLOT)
      {
         /* Channel not present in source. */

         if (ch == GFX_LFMT_CHANNELS_L)
         {
            /* luminance could be created from red */
            src_slot_idx = gfx_lfmt_fmt_detail_get_slot_by_channel_maybe(&src_fd, GFX_LFMT_CHANNELS_R);
            if (src_slot_idx == GFX_LFMT_FMT_DETAIL_INVALID_SLOT)
            {
               return false;
            }
         }
         else
         {
            return false;
         }
      }

      struct gfx_lfmt_slot_detail *src_slot = &src_fd.slts[src_slot_idx];

      GFX_LFMT_TYPE_T dst_slot_type = get_slot_type_glcopyteximage(dst_slot, dst_lfmt);
      GFX_LFMT_TYPE_T src_slot_type = get_slot_type_glcopyteximage(src_slot, src_lfmt);

      if (slot_classify_glcopyteximage(src_slot_type) !=
         slot_classify_glcopyteximage(dst_slot_type))
      {
         return false;
      }

      /*
       * ES3 spec: ... effective internal format is determined by the row in ta-
       * ble 3.17 or table 3.18 where Destination Internal Format matches inter-
       * nalformat and where the Source Red Size, Source Green Size, Source
       * Blue Size, and Source Alpha Size are consistent with the values of
       * the source buffer's FRAMEBUFFER_RED_SIZE , FRAMEBUFFER_GREEN_-
       * SIZE , FRAMEBUFFER_BLUE_SIZE , and FRAMEBUFFER_ALPHA_SIZE , re-
       * spectively.
       *
       * Also see GTFTestPackedPixels.c:1732
       * If the destination internalFormat is sized we don't allow component size changes.
       */
      if (is_sized_dst_internalformat)
      {
         if (dst_slot->bit_width != src_slot->bit_width)
            return false;
      }
   }

   return true;
}

/* return false if copying from src_lfmt to internalformat is unsupported */
static bool choose_copy_format(GLenum internalformat,
      const khrn_image *src, GFX_LFMT_T *dst_fmt)
{
   GFX_LFMT_T dst_fmts[GFX_BUFFER_MAX_PLANES];
   unsigned dst_num_planes;
   GFX_LFMT_T src_lfmt;
   bool is_sized;

   if (!gfx_gl_is_texture_internalformat(internalformat))
      return false;

   src_lfmt = khrn_image_get_lfmt(src, 0);

   is_sized = gfx_gl_is_sized_internalformat(internalformat);

   if (is_sized)
   {
      GFX_LFMT_T dst_api_fmt = gfx_api_fmt_from_sized_internalformat(internalformat);
      glxx_hw_fmts_from_api_fmt(&dst_num_planes, dst_fmts, dst_api_fmt);
   }
   else
   {
      glxx_choose_copy_format_from_unsized_internalformat(internalformat,
            src_lfmt, dst_fmts, &dst_num_planes);
   }

   if (!glcopyteximage_convert_permitted(dst_fmts[0], src_lfmt, is_sized))
      return false;

    /* to support planar buffers, this function (and a few that use it) needs
     * generalising. As it happens, for all the internalformats that glCopyTex
     * has to support, we always select single-plane buffers, so an assert will
     * do. */
   assert(dst_num_planes == 1);

   *dst_fmt = dst_fmts[0];
   return true;
}

GL_API void GL_APIENTRY glCopyTexImage2D(GLenum target, GLint level, GLenum
      internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint
      border)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   GLXX_TEXTURE_T *texture = NULL;
   GLenum error = GL_NO_ERROR;
   struct glxx_teximage_sanity_checks checks;
   bool ok;
   khrn_image *src = NULL;
   unsigned face;
   GFX_LFMT_T dst_fmt;

   if (!state)
      return;

   if (!gfx_gl_is_texture_internalformat(internalformat))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   if (!copytex_fb_complete_check(state))
      goto end;

   checks.dimensions = 2;
   checks.compressed = false;
   checks.respecify = true;

   ok = glxx_teximage_internal_checks(state, &checks, target, 0, 0,
         internalformat, level, width, height, 1, 0, 0, 0, border, &texture,
         &error);
   if (!ok)
      goto end;

   if (!glxx_fb_acquire_read_image(state->bound_read_framebuffer,
            GLXX_DOWNSAMPLED, &src, NULL))
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }

   if (src == NULL)
   {
      /* A complete framebuffer with no color attachment => invalid op */
      error = GL_INVALID_OPERATION;
      goto end;
   }

   if (!choose_copy_format(internalformat, src, &dst_fmt))
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   face = glxx_texture_get_face(target);
   ok = glxx_texture_copy_image(texture, face, (unsigned)level, dst_fmt, src,
         x, y, width, height, &state->fences);
   if (!ok)
      error = GL_OUT_OF_MEMORY;

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   KHRN_MEM_ASSIGN(src, NULL);
   glxx_unlock_server_state();
}

static void copytexSubImageX(GLenum target, GLint level, GLint xoffset, GLint
      yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height,
      unsigned dim)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   GLenum   error = GL_NO_ERROR;
   GLXX_TEXTURE_T *texture = NULL;
   khrn_image *src = NULL;
   struct glxx_teximage_sanity_checks checks;
   bool ok;
   unsigned int face;

   if (!state)
      return;

   if (!copytex_fb_complete_check(state))
      goto end;

   checks.dimensions = dim;
   checks.compressed = false;
   checks.respecify = false;

   ok = glxx_teximage_internal_checks(state, &checks, target, 0, 0, 0, level,
         width, height, 1, xoffset, yoffset, zoffset, 0, &texture, &error);
   if (!ok)
      goto end;

   if (!glxx_fb_acquire_read_image(state->bound_read_framebuffer,
            GLXX_DOWNSAMPLED, &src, NULL))
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }

   if (src == NULL)
   {
      /* A complete framebuffer with no color attachment => invalid op */
      error = GL_INVALID_OPERATION;
      goto end;
   }

   /* we've already checked in glxx_teximage_internal_checks that xoffset,
    * yoffset, zoffset, width and height >= 0 */
   face = glxx_texture_get_face(target);
   ok = glxx_texture_copy_sub_image(texture, face, level, xoffset,
         yoffset, zoffset, src, x, y, width, height, &state->fences);
   if (!ok)
      error = GL_OUT_OF_MEMORY;

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   KHRN_MEM_ASSIGN(src, NULL);
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glCopyTexSubImage2D(GLenum target, GLint level, GLint
      xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
   PROFILE_FUNCTION_MT("GL");
   copytexSubImageX(target, level, xoffset, yoffset, 0, x, y, width, height, 2);
}

GL_API void GL_APIENTRY glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset,
      GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei
      height)
{
   PROFILE_FUNCTION_MT("GL");
   copytexSubImageX(target, level, xoffset, yoffset, zoffset, x, y, width,
         height, 3);
}

GL_API void GL_APIENTRY glGenTextures(GLsizei n, GLuint *textures)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_ANY);
   int32_t i = 0;

   if (!state)
      return;

   if (n < 0)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);   // The conformance tests insist...
   else if (textures)
   {
      while (i < n)
      {
         if (glxx_shared_get_texture(state->shared, state->shared->next_texture) == NULL)
            textures[i++] = state->shared->next_texture;

         state->shared->next_texture++;
      }
   }
   glxx_unlock_server_state();
}

GL_API GLboolean GL_APIENTRY glIsTexture(GLuint texture)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_ANY);
   GLboolean result;

   if (!state) return GL_FALSE;

   result = glxx_shared_get_texture(state->shared, texture) != NULL;

   glxx_unlock_server_state();

   return result;
}

static bool is_mag_filter(int i)
{
   return i == GL_NEAREST ||
          i == GL_LINEAR;
}

static bool is_min_filter(GLenum target, int i)
{
   switch (i)
   {
   case GL_NEAREST_MIPMAP_NEAREST:
   case GL_NEAREST_MIPMAP_LINEAR:
   case GL_LINEAR_MIPMAP_NEAREST:
   case GL_LINEAR_MIPMAP_LINEAR:
      if (target == GL_TEXTURE_EXTERNAL_OES)
         return false;

      //Fall through
   case GL_NEAREST:
   case GL_LINEAR:
      return true;
   }
   return false;
}

static bool is_wrap(GLenum target, int i)
{
   switch (i)
   {
   case GL_MIRRORED_REPEAT:
   case GL_REPEAT:
      if (target != GL_TEXTURE_EXTERNAL_OES)
         return true;
      break;
   case GL_CLAMP_TO_EDGE:
#if V3D_VER_AT_LEAST(4,1,34,0)
   case GL_CLAMP_TO_BORDER:
#endif
   case GL_MIRROR_CLAMP_TO_EDGE_EXT:
      return true;
   }
   return false;
}

static uint32_t convert_swizzle(GLenum swizzle)
{
   switch (swizzle)
   {
      case GL_ZERO:  return 0;
      case GL_ONE:   return 1;
      case GL_RED:   return 2;
      case GL_GREEN: return 3;
      case GL_BLUE:  return 4;
      case GL_ALPHA: return 5;
      default:
         return ~0u;
   }
}

bool glxx_is_float_texparam(GLenum pname)
{
   return (pname == GL_TEXTURE_MAX_ANISOTROPY_EXT ||
           pname == GL_TEXTURE_MIN_LOD || pname == GL_TEXTURE_MAX_LOD);
}

bool glxx_is_vector_texparam(GLenum pname)
{
   return
#if V3D_VER_AT_LEAST(4,1,34,0)
      (pname == GL_TEXTURE_BORDER_COLOR) ||
#endif
      (pname == GL_TEXTURE_CROP_RECT_OES);
}

void glxx_texparameterf_sampler_internal(GLXX_SERVER_STATE_T *state,
      GLXX_TEXTURE_SAMPLER_STATE_T *sampler, GLenum pname, const GLfloat *f)
{
   /* TODO: Do we really know well enough where these calls come from? */
   if (IS_GL_11(state)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return;
   }

   switch (pname)
   {
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      if (*f >= 1.0f)
         sampler->anisotropy = *f;
      else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
      break;
   case GL_TEXTURE_MIN_LOD:
      sampler->min_lod = *f;
      break;
   case GL_TEXTURE_MAX_LOD:
      sampler->max_lod = *f;
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }
}

static void glxx_texparameterf_internal(GLXX_SERVER_STATE_T *state, GLenum target,
      GLenum pname, const GLfloat *f)
{
   if (target == GL_TEXTURE_BUFFER)
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return;
   }

   // performs valid target check
   GLXX_TEXTURE_T *texture = glxx_server_state_get_texture(state, target);

   if (texture)
   {
      switch (pname)
      {
      case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      case GL_TEXTURE_MIN_LOD:
      case GL_TEXTURE_MAX_LOD:
         if (glxx_tex_target_is_multisample(texture->target))
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
         else
            glxx_texparameterf_sampler_internal(state, &texture->sampler, pname, f);
         break;
      default:
         unreachable();
      }
   }
}

static bool is_func(GLenum func)
{
   return (func == GL_NEVER  || func == GL_ALWAYS || func == GL_LESS    ||
           func == GL_LEQUAL || func == GL_EQUAL  || func == GL_GREATER ||
           func == GL_GEQUAL || func == GL_NOTEQUAL);
}

static bool is_boolean(GLenum value)
{
   return value == GL_TRUE || value == GL_FALSE;
}

// NB: Target may be 0 when called from sampler paths (i.e. glSamplerParameteri)
void glxx_texparameter_sampler_internal(GLXX_SERVER_STATE_T *state,
      GLenum target, GLXX_TEXTURE_SAMPLER_STATE_T *sampler, GLenum pname, const
      GLint *i)
{
   switch (pname)
   {
   case GL_TEXTURE_MIN_FILTER:
      if (is_min_filter(target, *i))
         sampler->filter.min = *i;
      else
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   case GL_TEXTURE_MAG_FILTER:
      if (is_mag_filter(*i))
         sampler->filter.mag = *i;
      else
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   case GL_TEXTURE_WRAP_S:
      if (is_wrap(target, *i))
         sampler->wrap.s = *i;
      else
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   case GL_TEXTURE_WRAP_T:
      if (is_wrap(target, *i))
         sampler->wrap.t = *i;
      else
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   case GL_TEXTURE_WRAP_R:
      if (!IS_GL_11(state) && is_wrap(target, *i))
         sampler->wrap.r = *i;
      else
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   case GL_TEXTURE_COMPARE_MODE:
      if (!IS_GL_11(state) && (*i == GL_NONE || *i == GL_COMPARE_REF_TO_TEXTURE))
         sampler->compare_mode = *i;
      else
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   case GL_TEXTURE_COMPARE_FUNC:
      if (!IS_GL_11(state) && is_func(*i))
         sampler->compare_func = *i;
      else
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
#if V3D_VER_AT_LEAST(3,3,0,0)
   case GL_TEXTURE_UNNORMALISED_COORDS_BRCM:
      if (!IS_GL_11(state) && is_boolean(*i))
         sampler->unnormalised_coords = *i;
      else
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
#endif
   case GL_TEXTURE_SRGB_DECODE_EXT:
      if (!IS_GL_11(state) && (*i == GL_DECODE_EXT || *i == GL_SKIP_DECODE_EXT))
         sampler->skip_srgb_decode = (*i == GL_SKIP_DECODE_EXT);
      else
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
#if V3D_VER_AT_LEAST(4,1,34,0)
   case GL_TEXTURE_BORDER_COLOR:
      if (!IS_GL_11(state))
      {
         sampler->border_color[0] = i[0];
         sampler->border_color[1] = i[1];
         sampler->border_color[2] = i[2];
         sampler->border_color[3] = i[3];
      }
      else
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
#endif
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }
}

static bool gl11_is_texparam(GLenum pname) {
   switch (pname)
   {
   case GL_TEXTURE_MIN_FILTER:
   case GL_TEXTURE_MAG_FILTER:
   case GL_TEXTURE_WRAP_S:
   case GL_TEXTURE_WRAP_T:
   case GL_TEXTURE_CROP_RECT_OES:
   case GL_GENERATE_MIPMAP:
      return true;
   default:
      return false;
   }
}

static bool is_texparam(GLenum pname) {
   switch (pname)
   {
   case GL_TEXTURE_MIN_FILTER:
   case GL_TEXTURE_MAG_FILTER:
   case GL_TEXTURE_WRAP_S:
   case GL_TEXTURE_WRAP_T:
   case GL_TEXTURE_WRAP_R:
   case GL_TEXTURE_COMPARE_MODE:
   case GL_TEXTURE_COMPARE_FUNC:
   case GL_TEXTURE_SWIZZLE_R:
   case GL_TEXTURE_SWIZZLE_G:
   case GL_TEXTURE_SWIZZLE_B:
   case GL_TEXTURE_SWIZZLE_A:
   case GL_TEXTURE_BASE_LEVEL:
   case GL_TEXTURE_MAX_LEVEL:
#if V3D_VER_AT_LEAST(4,1,34,0)
   case GL_TEXTURE_BORDER_COLOR:
#endif
   case GL_TEXTURE_UNNORMALISED_COORDS_BRCM:
   case GL_TEXTURE_SRGB_DECODE_EXT:
   case GL_TEXTURE_PROTECTED_EXT:
      return true;
   case GL_DEPTH_STENCIL_TEXTURE_MODE:
      return V3D_VER_AT_LEAST(3,3,0,0) ? true : false;

   default:
      return false;
   }
}

void glxx_texparameter_internal(GLXX_SERVER_STATE_T *state, GLenum target, GLenum pname, const int *i)
{
   if (target == GL_TEXTURE_BUFFER)
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return;
   }

   // performs valid target check
   GLXX_TEXTURE_T *texture = glxx_server_state_get_texture(state, target);
   if (!texture)
      return;

   if (IS_GL_11(state) && !gl11_is_texparam(pname)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return;
   }

   if (!IS_GL_11(state) && !is_texparam(pname)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      return;
   }

   switch (pname)
   {
   case GL_TEXTURE_MIN_FILTER:
   case GL_TEXTURE_MAG_FILTER:
   case GL_TEXTURE_WRAP_S:
   case GL_TEXTURE_WRAP_T:
   case GL_TEXTURE_WRAP_R:
   case GL_TEXTURE_COMPARE_MODE:
   case GL_TEXTURE_COMPARE_FUNC:
#if V3D_VER_AT_LEAST(4,1,34,0)
   case GL_TEXTURE_BORDER_COLOR:
#endif
   case GL_TEXTURE_UNNORMALISED_COORDS_BRCM:
   case GL_TEXTURE_SRGB_DECODE_EXT:
      if (glxx_tex_target_is_multisample(texture->target))
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      else
      {
         glxx_texparameter_sampler_internal(state, target, &texture->sampler,
               pname, i);
      }
      break;
   case GL_GENERATE_MIPMAP:
      if (*i)
         texture->generate_mipmap = GL_TRUE;
      else
         texture->generate_mipmap = GL_FALSE;
      break;
   case GL_TEXTURE_CROP_RECT_OES:
      glxx_texture_set_crop_rect(texture, i);
      break;
   case GL_TEXTURE_SWIZZLE_R:
   case GL_TEXTURE_SWIZZLE_G:
   case GL_TEXTURE_SWIZZLE_B:
   case GL_TEXTURE_SWIZZLE_A:
      {
         uint32_t swizzle = convert_swizzle(*i);
         if (swizzle != ~0u)
            texture->swizzle[pname - GL_TEXTURE_SWIZZLE_R] = swizzle;
         else
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
      break;
   case GL_TEXTURE_BASE_LEVEL:
      if (*i >= 0)
      {
         if (glxx_tex_target_is_multisample(texture->target) && (*i != 0))
            glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         else
            texture->base_level = *i;
      }
      else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
      break;
   case GL_TEXTURE_MAX_LEVEL:
      if (*i >= 0)
         texture->max_level = *i;
      else
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
      break;
   case GL_DEPTH_STENCIL_TEXTURE_MODE:
      if (*i == GL_DEPTH_COMPONENT || *i == GL_STENCIL_INDEX)
         texture->ds_texture_mode = *i;
      else
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   case GL_TEXTURE_PROTECTED_EXT:
      if (is_boolean(*i))
         texture->create_secure = *i;
      else
         glxx_server_state_set_error(state, GL_INVALID_VALUE); /* This is what the spec says, INVALID_ENUM might be more consistent */
      break;

   default:
      unreachable();
      break;
   }
}

GL_API void GL_APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state) return;

   if (glxx_is_vector_texparam(pname)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (glxx_is_float_texparam(pname))
   {
      glxx_texparameterf_internal(state, target, pname, &param);
   }
   else
   {
      int iparam = gfx_float_to_int32(param);
      glxx_texparameter_internal(state, target, pname, &iparam);
   }

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state) return;

   if (glxx_is_vector_texparam(pname)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (glxx_is_float_texparam(pname))
   {
      float fparam = (float)param;
      glxx_texparameterf_internal(state, target, pname, &fparam);
   }
   else
   {
      glxx_texparameter_internal(state, target, pname, &param);
   }

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (glxx_is_float_texparam(pname))
   {
      glxx_texparameterf_internal(state, target, pname, params);
   }
   else
   {
      int iparams[4];
      for (int i=0; i < (glxx_is_vector_texparam(pname) ? 4 : 1); i++) {
#if V3D_VER_AT_LEAST(4,1,34,0)
         /* XXX: Border colour is weirdly either float or int */
         if (pname == GL_TEXTURE_BORDER_COLOR)
            iparams[i] = gfx_float_to_bits(params[i]);
         else
#endif
            iparams[i] = gfx_float_to_int32(params[i]);
      }
      glxx_texparameter_internal(state, target, pname, iparams);
   }

   glxx_unlock_server_state();
}

void glxx_texparamter_iv_common(GLenum target, GLenum pname, const GLint *params)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state) return;

   if (glxx_is_float_texparam(pname))
   {
      float fparam = (float)params[0];
      glxx_texparameterf_internal(state, target, pname, &fparam);
   }
   else
   {
      glxx_texparameter_internal(state, target, pname, params);
   }

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
   glxx_texparamter_iv_common(target, pname, params);
}

#if KHRN_GLES32_DRIVER
GL_APICALL void GL_APIENTRY glTexParameterIiv(GLenum target, GLenum pname, const GLint *params)
{
   glxx_texparamter_iv_common(target, pname, params);
}

GL_APICALL void GL_APIENTRY glTexParameterIuiv(GLenum target, GLenum pname, const GLuint *params)
{
   glxx_texparamter_iv_common(target, pname, (GLint *) params);
}
#endif

static void texSubImageX(GLenum target, GLint level, GLint
      xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height,
      GLsizei depth, GLenum format, GLenum type, const GLvoid* orig_pixels, unsigned dim)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   GLXX_TEXTURE_T *texture = NULL;
   GLenum          error   = GL_NO_ERROR;
   GLvoid         *pixels  = (GLvoid *)orig_pixels;

   struct glxx_teximage_sanity_checks checks = { .dimensions = dim,
                                                 .compressed = false,
                                                 .respecify  = false };

   if (!gfx_gl_format_type_combination_valid(format, type)) {
      if (!gfx_gl_format_type_valid_enums(format, type))
         error = GL_INVALID_ENUM;
      else
         error = GL_INVALID_OPERATION;
      goto end;
   }

   if (!glxx_teximage_unpack_buffer_checks(type, orig_pixels,
      state->bound_buffer[GLXX_BUFTGT_PIXEL_UNPACK].obj, &error))
      goto end;

   if (!glxx_teximage_internal_checks(state, &checks, target,
            format, type, 0, level, width, height, depth, xoffset, yoffset, zoffset, 0,
            &texture, &error))
      goto end;

   unsigned face = glxx_texture_get_face(target);

   // We must also check for valid combination with the existing internal format.
   // First check against the sized internal format. If that doesn't match, try the unsized.
   GLenum internal_fmt     = gfx_internalformat_from_api_fmt(texture->img[face][level]->api_fmt);
   GLenum unsized_internal = gfx_unsized_internalformat_from_api_fmt_maybe(texture->img[face][level]->api_fmt);

   if (!gfx_lfmt_gl_format_type_internalformat_combination_valid(format, type, internal_fmt))
   {
      if (unsized_internal == GL_NONE ||
          !gfx_lfmt_gl_format_type_internalformat_combination_valid(format, type, unsized_internal))
      {
         error = GL_INVALID_OPERATION;
         goto end;
      }
   }

   glxx_texture_subimage(texture, face, level, xoffset, yoffset, zoffset,
               format, type, &(state->pixel_store_state),
               state->bound_buffer[GLXX_BUFTGT_PIXEL_UNPACK].obj, pixels,
               width, height, depth, &state->fences, &error);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   if (pixels != orig_pixels)
      free(pixels);
   glxx_unlock_server_state();
}

#if V3D_VER_AT_LEAST(4,1,34,0)
GL_API void GL_APIENTRY glTexSubImage1DBRCM(GLenum target, GLint level, GLint xoffset,
      GLsizei width, GLenum format, GLenum type, const GLvoid* orig_pixels)
{
   PROFILE_FUNCTION_MT("GL");
   texSubImageX(target, level, xoffset, 0, 0, width, 1, 1,
         format, type, orig_pixels, 1);
}
#endif

GL_API void GL_APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint
      xoffset, GLint yoffset, GLsizei width, GLsizei height,
      GLenum format, GLenum type, const GLvoid* orig_pixels)
{
   PROFILE_FUNCTION_MT("GL");
   texSubImageX(target, level, xoffset, yoffset, 0, width, height, 1,
         format, type, orig_pixels, 2);
}

GL_API void GL_APIENTRY glTexSubImage3D(GLenum target, GLint level, GLint
      xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height,
      GLsizei depth, GLenum format, GLenum type, const GLvoid* orig_pixels)
{
   PROFILE_FUNCTION_MT("GL");
   texSubImageX(target, level, xoffset, yoffset, zoffset, width, height, depth,
         format, type, orig_pixels, 3);
}


GL_API void GL_APIENTRY glActiveTexture(GLenum texture)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_ANY);
   if (!state)
      return;

  if (texture >= GL_TEXTURE0 && (
         ( IS_GL_11(state) && texture < GL_TEXTURE0 + GL11_CONFIG_MAX_TEXTURE_UNITS) ||
         (!IS_GL_11(state) && texture < GL_TEXTURE0 + GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
      ))
   {
      state->active_texture = texture - GL_TEXTURE0;
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glBindTexture(GLenum target, GLuint texture)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   GLenum error = GL_NONE;
   GLXX_TEXTURE_T *texture_obj = NULL;

   if (!state)
      return;

   if (!glxx_texture_is_tex_target(state, target))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   if (texture == 0)
      texture_obj = glxx_textures_get_texture(&state->default_textures, target);
   else
   {
      texture_obj = glxx_shared_get_or_create_texture(state->shared, texture,
            target, &error);

      if (texture_obj == NULL)
         goto end;
   }
   assert(texture_obj != NULL);

   glxx_textures_set_texture(&state->bound_texture[state->active_texture], texture_obj);

   //state->changed_texture
end:
   if (error != GL_NONE)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}
GL_API void GL_APIENTRY glDeleteTextures(GLsizei n, const GLuint *textures)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (n < 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (!textures)
      goto end;

   for (int i = 0; i < n; i++)
   {
      if (!textures[i]) continue;

      GLXX_TEXTURE_T *texture = glxx_shared_get_texture(state->shared, textures[i]);
      if (texture == NULL)
         continue;

      GLXX_TEXTURE_T *default_tex = glxx_textures_get_texture(&state->default_textures,
                                                              texture->target);

      for (int j = 0; j < GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS; j++)
      {
         GLXX_TEXTURE_T *bound_tex = glxx_textures_get_texture(&state->bound_texture[j],
                                                               texture->target);

         if (bound_tex == texture)
         {
            glxx_textures_set_texture(&state->bound_texture[j],
                default_tex);
         }
      }

      for (int j = 0; j < GLXX_CONFIG_MAX_IMAGE_UNITS; j++)
      {
         glxx_image_unit *image_unit = &state->image_unit[j];
         if (image_unit->texture == texture)
            glxx_image_unit_deinit(image_unit);
      }

      glxx_fb_detach_texture(state->bound_draw_framebuffer, texture);
      glxx_fb_detach_texture(state->bound_read_framebuffer, texture);

      glxx_shared_delete_texture(state->shared, textures[i]);
   }

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGenerateMipmap(GLenum target)
{
   PROFILE_FUNCTION_MT("GL");

   GLenum error = GL_NO_ERROR;

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   GLXX_TEXTURE_T *texture = glxx_server_state_get_texture(state, target);
   if (!texture)
      goto end;

   if (texture->target == GL_TEXTURE_EXTERNAL_OES ||
       glxx_tex_target_is_multisample(texture->target) ||
       texture->target == GL_TEXTURE_BUFFER)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   glxx_texture_generate_mipmap(texture, &state->fences, &error);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

#if V3D_VER_AT_LEAST(4,1,34,0) || KHRN_GLES32_DRIVER
static bool is_allowed_texbuffer_internalformat(GLenum internalformat)
{
   switch(internalformat)
   {
   case GL_R8:
   case GL_R16F:
   case GL_R32F:
   case GL_R8I:
   case GL_R16I:
   case GL_R32I:
   case GL_R8UI:
   case GL_R16UI:
   case GL_R32UI:
   case GL_RG8:
   case GL_RG16F:
   case GL_RG32F:
   case GL_RG8I:
   case GL_RG16I:
   case GL_RG32I:
   case GL_RG8UI:
   case GL_RG16UI:
   case GL_RG32UI:
   case GL_RGB32F:
   case GL_RGB32I:
   case GL_RGB32UI:
   case GL_RGBA8:
   case GL_RGBA16F:
   case GL_RGBA32F:
   case GL_RGBA8I:
   case GL_RGBA16I:
   case GL_RGBA32I:
   case GL_RGBA8UI:
   case GL_RGBA16UI:
   case GL_RGBA32UI:
      return true;
   default:
      return false;
   }
}

static GLenum texbuffer_internal_checks(const GLXX_SERVER_STATE_T *state,
      GLenum target, GLenum internalformat,
      GLuint buffer_id, bool use_range, GLintptr offset_i, GLsizeiptr size_i)
{
   if (target != GL_TEXTURE_BUFFER)
      return GL_INVALID_ENUM;

   if (!is_allowed_texbuffer_internalformat(internalformat))
      return GL_INVALID_ENUM;

   if (buffer_id == 0)
      return GL_NO_ERROR;

   GLXX_BUFFER_T *buffer = glxx_shared_get_buffer(state->shared, buffer_id);
   if (!buffer)
      return GL_INVALID_OPERATION;

   if (use_range)
   {
      if (offset_i < 0 || size_i <= 0)
         return GL_INVALID_VALUE;
      if ((offset_i % GLXX_CONFIG_TEXTURE_BUFFER_OFFSET_ALIGNMENT) != 0)
         return GL_INVALID_VALUE;
      if ((size_t)(offset_i + size_i) >  glxx_buffer_get_size(buffer))
         return GL_INVALID_VALUE;
   }

   return GL_NO_ERROR;
}

static void texBufferRange(GLenum target, GLenum internalformat,
      GLuint buffer_id, bool use_range, GLintptr offset_i, GLsizeiptr size_i)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   GLenum error = texbuffer_internal_checks(state, target, internalformat, buffer_id,
         use_range, offset_i, size_i);
   if (error != GL_NO_ERROR)
      goto end;

   GLXX_TEXTURE_T *texture = glxx_server_get_active_texture(state, GL_TEXTURE_BUFFER);

   GLXX_BUFFER_T *buffer = NULL;
   size_t offset = 0;
   size_t size = 0;
   if (buffer_id != 0)
   {
      buffer = glxx_shared_get_buffer(state->shared, buffer_id);
      offset = use_range ? offset_i : 0;
      size = use_range ? size_i : SIZE_MAX;
   }

   glxx_texture_buffer_attach(texture, buffer, internalformat, offset, size);
   error = GL_NO_ERROR;
end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   glxx_unlock_server_state();
}
#endif

#if V3D_VER_AT_LEAST(4,1,34,0)
GL_APICALL void GL_APIENTRY glTexBufferEXT(GLenum target, GLenum internalformat, GLuint buffer)
{
   texBufferRange(target, internalformat, buffer, false, 0, 0);
}
GL_APICALL void GL_APIENTRY glTexBufferRangeEXT(GLenum target, GLenum internalformat, GLuint buffer,
      GLintptr offset, GLsizeiptr size)
{
   texBufferRange(target, internalformat, buffer, true, offset, size);
}
GL_APICALL void GL_APIENTRY glTexBufferOES(GLenum target, GLenum internalformat, GLuint buffer)
{
   texBufferRange(target, internalformat, buffer, false, 0, 0);
}
GL_APICALL void GL_APIENTRY glTexBufferRangeOES(GLenum target, GLenum internalformat, GLuint buffer,
      GLintptr offset, GLsizeiptr size)
{
   texBufferRange(target, internalformat, buffer, true, offset, size);
}
#endif

#if KHRN_GLES32_DRIVER
GL_APICALL void GL_APIENTRY glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer)
{
   texBufferRange(target, internalformat, buffer, false, 0, 0);
}
GL_APICALL void GL_APIENTRY glTexBufferRange(GLenum target, GLenum internalformat, GLuint buffer,
      GLintptr offset, GLsizeiptr size)
{
   texBufferRange(target, internalformat, buffer, true, offset, size);
}
#endif
