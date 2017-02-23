/*=============================================================================
  Broadcom Proprietary and Confidential. (c)2008 Broadcom.
  All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Implementation of OpenGL ES texture structure.
=============================================================================*/
#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/util/gfx_util/gfx_util_conv.h"

#include "libs/core/v3d/v3d_tmu.h"

#include "../common/khrn_int_common.h"
#include "../common/khrn_int_util.h"

#include "glxx_texture.h"
#include "glxx_server_internal.h"

#include "glxx_shader_cache.h"
#include "glxx_inner.h"
#include "../common/khrn_blob.h"
#include "../common/khrn_image.h"
#include "../common/khrn_event_monitor.h"
#include "../egl/egl_display.h"
#include "../egl/egl_image.h"
#include "glxx_image_unit.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "libs/platform/gmem.h"

static bool requires_mipmaps(enum glxx_min_filter min_filter);

static unsigned num_faces(enum glxx_tex_target target);

static void texture_get_main_blob(const GLXX_TEXTURE_T *texture,
      KHRN_BLOB_T **main_blob, unsigned *at_level);

static bool texture_has_any_blobs(struct glxx_texture *texture);
static unsigned num_faces(enum glxx_tex_target target);

static bool upload_data(KHRN_IMAGE_T *img,
      unsigned dst_x, unsigned dst_y, unsigned dst_z, unsigned dst_start_elem,
      unsigned num_array_elems,
      const GFX_BUFFER_DESC_T *src_desc, size_t src_offset, size_t src_array_pitch,
      GLXX_BUFFER_T *pixel_buffer, const void *pixels,
      glxx_context_fences *fences, GLenum *error);

static KHRN_BLOB_T* create_blob(enum glxx_tex_target target, unsigned width,
      unsigned height, unsigned depth, unsigned num_array_elems, unsigned
      num_mip_levels, const GFX_LFMT_T *fmts, unsigned num_planes,
      bool secure_texture);

/* Number of levels past base_level that should be used */
static bool try_get_num_levels(const GLXX_TEXTURE_T *texture, bool only_base_level,
      unsigned *base_level, unsigned *num_levels);

static bool texture_check_or_create_image(GLXX_TEXTURE_T *texture, unsigned
      face, unsigned level, unsigned width, unsigned height, unsigned depth,
      unsigned  num_array_elems, const GFX_LFMT_T *fmts, unsigned num_planes,
      GFX_LFMT_T api_fmt, glxx_context_fences *fences,
      bool secure_texture);

static bool texture_create_image(GLXX_TEXTURE_T *texture, unsigned face, unsigned
      level, unsigned width, unsigned height, unsigned depth, unsigned
      num_array_elems, const GFX_LFMT_T *fmts, unsigned num_planes, GFX_LFMT_T api_fmt,
      bool secure_texture);

static void texture_unbind_and_release_all(GLXX_TEXTURE_T *texture);
static bool texture_unbind(GLXX_TEXTURE_T *texture, glxx_context_fences *fences);
static bool texture_buffer_possible_complete(const GLXX_TEXTURE_T *texture);
static bool texture_buffer_create_images(GLXX_TEXTURE_T *texture);

static bool in_secure_context(void)
{
   GLXX_SERVER_STATE_T  *state = egl_context_gl_server_state(NULL);

   return egl_context_gl_secure(state->context);
}

/* returns the dimensions of the destination image based on the texture target */
unsigned get_target_num_dimensions(enum glxx_tex_target target)
{
   unsigned dim;
   /* GFX_BUFFER_DESC_T knows only 1D, 2D or 3D images, so:
    * 1D_ARRAY is just 1D image with depth elements in the array,
    * 2D_ARRAY is just 2D image with depth elements in the array */
   switch (target)
   {
      case GL_TEXTURE_1D_BRCM:
      case GL_TEXTURE_1D_ARRAY_BRCM:
      case GL_TEXTURE_BUFFER:
         dim = 1;
         break;
      case GL_TEXTURE_2D:
      case GL_TEXTURE_CUBE_MAP:
      case GL_TEXTURE_EXTERNAL_OES:
      case GL_TEXTURE_2D_ARRAY:
      case GL_TEXTURE_2D_MULTISAMPLE:
      case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      case GL_TEXTURE_CUBE_MAP_ARRAY:
         dim = 2;
         break;
      case GL_TEXTURE_3D:
         dim = 3;
         break;
      default:
         unreachable();
         dim = 0;
   }
   return dim;
}

void glxx_tex_transform_dim_for_target(enum glxx_tex_target target,
      unsigned *width, unsigned *height, unsigned *depth, unsigned
      *num_array_elems)
{
   switch (target)
   {
      case GL_TEXTURE_1D_BRCM:
      case GL_TEXTURE_BUFFER:
         *height = 1;
         *depth = 1;
         *num_array_elems = 1;
         break;
      case GL_TEXTURE_1D_ARRAY_BRCM:
         *num_array_elems = *height;
         *height = 1;
         *depth = 1;
         break;
      case GL_TEXTURE_2D:
      case GL_TEXTURE_CUBE_MAP:
      case GL_TEXTURE_EXTERNAL_OES:
      case GL_TEXTURE_2D_MULTISAMPLE:
         *depth = 1;
         *num_array_elems = 1;
         break;
      case GL_TEXTURE_2D_ARRAY:
      case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      case GL_TEXTURE_CUBE_MAP_ARRAY:
         *num_array_elems = *depth;
         *depth = 1;
         break;
      case GL_TEXTURE_3D:
         *num_array_elems = 1;
         break;
      default:
         unreachable();
   }
}

void glxx_tex_transform_offsets_for_target(enum glxx_tex_target target,
      unsigned *xoffset, unsigned *yoffset, unsigned *zoffset, unsigned
      *start_elem)
{
   switch (target)
   {
      case GL_TEXTURE_1D_BRCM:
      case GL_TEXTURE_BUFFER:
         *start_elem = 0;
         *yoffset = 0;
         *zoffset = 0;
         break;
      case GL_TEXTURE_1D_ARRAY_BRCM:
         *start_elem = *yoffset;
         *yoffset = 0;
         *zoffset = 0;
         break;
      case GL_TEXTURE_2D:
      case GL_TEXTURE_CUBE_MAP:
      case GL_TEXTURE_EXTERNAL_OES:
      case GL_TEXTURE_2D_MULTISAMPLE:
         *start_elem = 0;
         *zoffset = 0;
         break;
      case GL_TEXTURE_2D_ARRAY:
      case GL_TEXTURE_CUBE_MAP_ARRAY:
      case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
         *start_elem = *zoffset;
         *zoffset = 0;
         break;
      case GL_TEXTURE_3D:
         *start_elem = 0;
         break;
      default:
         unreachable();
   }
}

static bool requires_mipmaps(enum glxx_min_filter min_filter)
{
   if (min_filter == GL_NEAREST || min_filter == GL_LINEAR)
      return false;
   return true;
}

/* maxium number of mipmap levels based on the target and the dimensions */
unsigned glxx_tex_max_levels_from_dim(enum glxx_tex_target target, unsigned
      w, unsigned h, unsigned d)
{
   unsigned max_levels;
   unsigned max_size = 0;

   switch (target)
   {
      case GL_TEXTURE_1D_BRCM:
      case GL_TEXTURE_1D_ARRAY_BRCM:
         max_size = w;
         break;
      case GL_TEXTURE_2D:
      case GL_TEXTURE_EXTERNAL_OES:
      case GL_TEXTURE_2D_ARRAY:
      case GL_TEXTURE_CUBE_MAP:
      case GL_TEXTURE_CUBE_MAP_ARRAY:
         max_size = gfx_umax(w, h);
         break;
      case GL_TEXTURE_3D:
         max_size = gfx_umax3(w, h, d);
         break;
      case GL_TEXTURE_2D_MULTISAMPLE:
      case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      case GL_TEXTURE_BUFFER:
         return 1;
      default:
         unreachable();
   }

   /* mip_count = log2(maxsize) + 1 */
   max_levels = gfx_msb(max_size) + 1;
   return max_levels;
}

static bool texture_has_any_blobs(struct glxx_texture *texture)
{
   unsigned face, level, faces;
   faces = num_faces(texture->target);

   for (face = 0; face < faces; face++)
   {
      for (level = 0; level < KHRN_MAX_MIP_LEVELS; level++)
      {
         if (texture->img[face][level] != NULL)
            return true;
      }
   }
   return false;
}

static KHRN_BLOB_T* create_blob(enum glxx_tex_target target, unsigned width,
      unsigned height, unsigned depth, unsigned num_array_elems, unsigned
      num_mip_levels, const GFX_LFMT_T *fmts, unsigned num_planes,
      bool secure)
{
   unsigned usage, dim;

   usage =  GFX_BUFFER_USAGE_V3D_TEXTURE;
   if (target == GL_TEXTURE_CUBE_MAP && num_array_elems == 6)
      usage |= GFX_BUFFER_USAGE_V3D_CUBEMAP;

   dim = get_target_num_dimensions(target);
   GFX_LFMT_T lfmts[GFX_BUFFER_MAX_PLANES];
   memcpy(lfmts, fmts, num_planes * sizeof(GFX_LFMT_T));
   glxx_lfmt_add_dim(lfmts, num_planes, dim);

   if (dim != 1)
   {
      /* 1D textures cannot be render targets */
      usage |= GFX_BUFFER_USAGE_V3D_RENDER_TARGET;
#if !V3D_VER_AT_LEAST(4,0,2,0)
      if (glxx_tex_target_is_multisample(target))
         usage |= GFX_BUFFER_USAGE_V3D_TLB_RAW;
#endif
   }

   return khrn_blob_create(width, height, depth, num_array_elems,
         num_mip_levels, lfmts, num_planes, usage,
         secure);
}

/* The definition of a main blob depends on the texture target:
 * - if target == GL_TEXTURE_CUBE_MAP, main blob is a blob that has storage for
 *   all the faces (even if it is for one mip level)
 * - any other target, main blob is a blob that has storage for more than one mip level
 * There can be only one main_blob (if any) in a texture
 * Params:
 *  in : texture
 *  out:
 *    main_blob: pointer to the main blob if we have one, NULL otherwise;
 *    at_level:  what mip level in the texture corresponds to mip level 0 in the blob
 */
static void texture_get_main_blob(const GLXX_TEXTURE_T *texture,
      KHRN_BLOB_T **main_blob, unsigned *at_level)
{
   unsigned face, level, faces;
   KHRN_IMAGE_T *img;

   *main_blob = NULL;
   *at_level = 0;

   faces = num_faces(texture->target);

   bool is_cube = (texture->target == GL_TEXTURE_CUBE_MAP);
   for (face = 0; face < faces; face++)
   {
      for (level = 0; level < KHRN_MAX_MIP_LEVELS; level++)
      {
         img = texture->img[face][level];
         if (img != NULL)
         {
            if ((!is_cube && img->blob->num_mip_levels > 1) ||
                (is_cube && img->blob->num_array_elems == faces))
            {
               *main_blob = img->blob;
               assert(img->level <= level);
               *at_level = level - img->level;
               return;
            }
         }
      }
   }
   return;
}

static unsigned num_faces(enum glxx_tex_target target)
{
   unsigned faces = 1;

   if (target == GL_TEXTURE_CUBE_MAP)
      faces = MAX_FACES;
   return faces;
}

/* set default values for the sampler, based on the target type */
static void init_default_texture_sampler(enum glxx_tex_target target,
      GLXX_TEXTURE_SAMPLER_STATE_T *sampler)
{
   switch (target)
   {
   case GL_TEXTURE_EXTERNAL_OES:
      sampler->filter.min = GL_LINEAR;
      sampler->filter.mag = GL_LINEAR;
      sampler->wrap.s = sampler->wrap.t = sampler->wrap.r = GL_CLAMP_TO_EDGE;
      break;
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_TEXTURE_BUFFER:
      sampler->filter.min = GL_NEAREST;
      sampler->filter.mag = GL_NEAREST;
      sampler->wrap.s = sampler->wrap.t = sampler->wrap.r = GL_CLAMP_TO_EDGE;
      break;
   default:
      sampler->filter.min = GL_NEAREST_MIPMAP_LINEAR;
      sampler->filter.mag = GL_LINEAR;
      sampler->wrap.s = sampler->wrap.t = sampler->wrap.r = GL_REPEAT;
      break;
   }
   sampler->anisotropy = 1.0f;

   sampler->min_lod = -1000.0f;
   sampler->max_lod = 1000.0f;
   sampler->compare_mode = GL_NONE;
   sampler->compare_func = GL_LEQUAL;

   sampler->unnormalised_coords = false;
   sampler->skip_srgb_decode = false;

   memset(sampler->border_color, 0, sizeof(sampler->border_color));

   sampler->debug_label = NULL;
}

static void assign_new_images(GLXX_TEXTURE_T *texture,
         KHRN_IMAGE_T* img[MAX_FACES][KHRN_MAX_MIP_LEVELS],
         unsigned base_level, unsigned num_levels)
{
   unsigned face, level;
   unsigned faces = num_faces(texture->target);

   for (face = 0; face < faces; face++)
   {
      for (level = base_level; level < base_level + num_levels; level++)
         KHRN_MEM_ASSIGN(texture->img[face][level], img[face][level]);
   }
}

static void release_images(KHRN_IMAGE_T* img[MAX_FACES][KHRN_MAX_MIP_LEVELS],
      unsigned faces, unsigned  base_level, unsigned num_levels)
{
   unsigned face, level;
   for (face = 0; face < faces; face++)
   {
      for (level = base_level; level < base_level + num_levels; level++)
         KHRN_MEM_ASSIGN(img[face][level], NULL);
   }
}

static void release_texel_arrays(GLXX_TEXTURE_T *texture)
{
   assert(texture->binding == TEX_BOUND_NONE);

   unsigned faces = num_faces(texture->target);
   release_images(texture->img, faces, 0, KHRN_MAX_MIP_LEVELS);
}

static void texture_term(void *v, size_t size)
{
   GLXX_TEXTURE_T *texture = (GLXX_TEXTURE_T *)v;

   vcos_unused(size);
   texture_unbind_and_release_all(texture);

   free(texture->debug_label);
   texture->debug_label = NULL;
}

static bool texture_init(GLXX_TEXTURE_T *texture, int32_t name, enum
      glxx_tex_target target)
{
   unsigned i;

   texture->name = name;
   texture->target = target;

   init_default_texture_sampler(target, &texture->sampler);

   for (i = 0; i < 4; i++)
      texture->swizzle[i] = 2 + i;
   texture->base_level = 0;
   texture->max_level = 1000;
   texture->generate_mipmap = GL_FALSE;
   texture->ds_texture_mode = GL_DEPTH_COMPONENT;

   texture->immutable_format = GFX_LFMT_NONE;
   texture->immutable_levels = 0;

   if (glxx_tex_target_is_multisample(texture->target))
      texture->ms_mode = GLXX_4X_MS;
   else
      texture->ms_mode = GLXX_NO_MS;
   texture->fixed_sample_locations = true;

   memset(texture->img, 0, sizeof(uintptr_t) * MAX_FACES * KHRN_MAX_MIP_LEVELS);

   texture->binding= TEX_BOUND_NONE;
   texture->source = NULL;
   texture->observer.callback = NULL;

   texture->debug_label = NULL;

   return true;
}

GLXX_TEXTURE_T* glxx_texture_create(GLenum target, int32_t name)
{
    GLXX_TEXTURE_T *ret = KHRN_MEM_ALLOC_STRUCT(GLXX_TEXTURE_T);
    if (ret == NULL)
       return ret;

    if (!texture_init(ret, name, target))
    {
       KHRN_MEM_ASSIGN(ret, NULL);
       return NULL;
    }

    khrn_mem_set_term(ret, texture_term);
    return ret;
}

/* if we orphan a texture that is the source/or target of an eglimage, we
 * should allocate additional space for all specified (and respecified) image
 * arrays, and copy any existing image data to the newly (re)specified texture
 * object; a texture source can have as eglimage 1+ of the mipmaps, a texture
 * target has as eglimage only image with face =0 and level = 0 */
static bool orphan_tex_eglimage(GLXX_TEXTURE_T *texture,
      glxx_context_fences *fences)
{
   KHRN_IMAGE_T* new_img[MAX_FACES][KHRN_MAX_MIP_LEVELS];
   GFX_LFMT_T fmts[GFX_BUFFER_MAX_PLANES];
   unsigned level, face, faces;
   unsigned width, height, depth, num_elems, num_planes;
   bool ok = false;
   enum glxx_tex_binding saved_binding;

   assert(texture->binding == TEX_BOUND_EGLIMAGE_SOURCE || texture->binding == TEX_BOUND_EGLIMAGE_TARGET);

   faces = num_faces(texture->target);

   /* copy the texel arrays pointers from the texture to a temporary array and
    * set the the one from the texture to NULL */
   memcpy(new_img, texture->img, faces * KHRN_MAX_MIP_LEVELS * sizeof(KHRN_IMAGE_T*));
   memset(texture->img, 0, faces * KHRN_MAX_MIP_LEVELS * sizeof(KHRN_IMAGE_T*));
   saved_binding = texture->binding;
   texture->binding = TEX_BOUND_NONE;

   for (face = 0; face < faces; face++)
   {
      for (level = 0; level < KHRN_MAX_MIP_LEVELS; level++)
      {
         KHRN_IMAGE_T *temp = new_img[face][level];
         if (temp)
         {
            khrn_image_get_fmts(temp, fmts, &num_planes);
            width = khrn_image_get_width(temp);
            height = khrn_image_get_height(temp);
            depth = khrn_image_get_depth(temp);
            num_elems = khrn_image_get_num_elems(temp);

            ok = texture_create_image(texture, face, level, width, height, depth, num_elems,
               fmts, num_planes, temp->api_fmt, temp->blob->secure);
            if (!ok)
               goto end;

            ok = khrn_image_convert(texture->img[face][level], temp, fences, in_secure_context());
            if (!ok)
               goto end;
         }
      }
   }

   /* now we can release the original texel arrays */
   release_images(new_img, faces, 0, KHRN_MAX_MIP_LEVELS);
   ok = true;
end:
   if (!ok)
   {
      /* just leave everything as it was at the begining of this function
       * (release any newly created texel arrays in the texture and set back
       * the copied  temporary texel array pointers */
      release_images(texture->img, faces, 0, KHRN_MAX_MIP_LEVELS);
      memcpy(texture->img, new_img, faces * KHRN_MAX_MIP_LEVELS * sizeof(KHRN_IMAGE_T*));

      texture->binding = saved_binding;
   }
   return ok;
}

static KHRN_IMAGE_T* get_image_for_texturing(EGL_IMAGE_T *egl_image,
      glxx_context_fences *fences)
{
   KHRN_IMAGE_T *image = egl_image_get_image(egl_image);
   const GFX_BUFFER_DESC_T *desc = &image->blob->desc[image->level];

   bool is_rso = gfx_lfmt_is_rso(desc->planes[0].lfmt);

   for (unsigned int p = 1; p < desc->num_planes; p++)
      assert(is_rso == gfx_lfmt_is_rso(desc->planes[p].lfmt));

   if (!is_rso)
   {
      khrn_mem_acquire(image);
      return image;
   }

   GFX_LFMT_T dst_fmts[GFX_BUFFER_MAX_PLANES];
   unsigned dst_num_planes;
   GFX_LFMT_T api_fmt;

   /* normally we just want a new image with the same format as the rso image,
    * but suitable for texturing;
    * in the case of YV12/ YUV we want a conversion to rgba + suitable image
    * for texturing;
    */
   if (gfx_lfmt_has_y(desc->planes[0].lfmt))
   {
      /* we want y(u)v conversions to rgba */
      api_fmt = GFX_LFMT_R8_G8_B8_A8_UNORM;
      dst_fmts[0] = GFX_LFMT_R8_G8_B8_A8_UNORM;
      dst_num_planes = 1;
   }
   else
   {
      for (unsigned int p = 0; p < desc->num_planes; p++)
         dst_fmts[p] = gfx_lfmt_fmt(desc->planes[p].lfmt);
      dst_num_planes = desc->num_planes;
      api_fmt = image->api_fmt;
   }

   /* the image must be 2D or a slice from 3D */
   for (unsigned int p = 0; p < desc->num_planes; p++)
      assert(gfx_lfmt_is_2d(desc->planes[p].lfmt) || (gfx_lfmt_is_3d(desc->planes[p].lfmt) && image->num_slices == 1));

   /* we only make a 2D blob */
   glxx_lfmt_add_dim(dst_fmts, dst_num_planes, 2);

   KHRN_BLOB_T* dst_blob = khrn_blob_create(desc->width, desc->height, 1, 1, 1,
         dst_fmts, dst_num_planes, GFX_BUFFER_USAGE_V3D_TEXTURE |
         GFX_BUFFER_USAGE_V3D_RENDER_TARGET, image->blob->secure);

   KHRN_IMAGE_T *dst_image = NULL;
   if (dst_blob)
   {
      dst_image = khrn_image_create(dst_blob, 0, 1, 0, api_fmt);
      khrn_mem_release(dst_blob);
   }
   if (dst_image)
   {
      khrn_image_convert(dst_image, image, fences, in_secure_context());

      if (v3d_platform_explicit_sync())
      {
         KHRN_RES_INTERLOCK_T *res_i = khrn_image_get_res_interlock(dst_image);
         khrn_interlock_flush_writer(&res_i->interlock);
         v3d_scheduler_wait_jobs(&res_i->interlock.pre_write, V3D_SCHED_DEPS_FINALISED);
      }
   }
   return dst_image;
}

/* call this when you need to orphan the texture;
 * if the texture gets deleted or all the texel arrays are released, just
 * call texture_unbind_and_release */
static bool texture_unbind(GLXX_TEXTURE_T *texture,
      glxx_context_fences *fences)
{
   bool ok = true;

   if (texture->binding == TEX_BOUND_NONE)
      return ok;

   if (texture->observer.callback)
   {
      texture->observer.callback(texture->observer.param);
      glxx_texture_remove_observer(texture);
   }

   egl_image_refdec(texture->source);
   texture->source =  NULL;

   switch (texture->binding)
   {
      case TEX_BOUND_TEXIMAGE:
         texture->binding = TEX_BOUND_NONE;
         release_texel_arrays(texture);
         break;
     case TEX_BOUND_EGLIMAGE_TARGET:
     case TEX_BOUND_EGLIMAGE_SOURCE:
         ok = orphan_tex_eglimage(texture, fences);
         break;
     default:
         unreachable();
   }
   return ok;
}

/* if the texture gets deleted or all the texture arrays get released, there is
 * no need to orphan */
static void texture_unbind_and_release_all(GLXX_TEXTURE_T *texture)
{
   if (texture->binding != TEX_BOUND_NONE)
   {
      if (texture->observer.callback)
      {
         texture->observer.callback(texture->observer.param);
         glxx_texture_remove_observer(texture);
      }
      egl_image_refdec(texture->source);
      texture->source =  NULL;

      /* there is not point in orphaning if we are going to release all the texel
       * arrays */
      texture->binding = TEX_BOUND_NONE;
   }
   release_texel_arrays(texture);
   KHRN_MEM_ASSIGN(texture->tex_buffer.buffer, NULL);
}

/* create the texel array for a face and and level;
 * Don't call this for immutable texture (use create_immutable_images)
 */
static bool texture_create_image(GLXX_TEXTURE_T *texture, unsigned face, unsigned
      level, unsigned width, unsigned height, unsigned depth, unsigned
      num_array_elems, const GFX_LFMT_T *fmts, unsigned num_planes, GFX_LFMT_T api_fmt,
      bool secure_texture)
{
   unsigned levels, blob_array_elems;
   KHRN_BLOB_T *frag, *main_blob;
   unsigned at_level;

   assert(texture->immutable_format == GFX_LFMT_NONE);
   assert(texture->img[face][level] == NULL);
   assert(texture->binding == TEX_BOUND_NONE);

   /* 1 - we don't have any blobs and level is base_level */
   if (!texture_has_any_blobs(texture) &&
       texture->base_level == level)
   {
      KHRN_BLOB_T *blob;
      /* allocate the main blob based on this level and the min filter */
      if (requires_mipmaps(texture->sampler.filter.min))
      {
         unsigned max_levels = glxx_tex_max_levels_from_dim(texture->target, width,
               height, depth);

         /* Mipmapping (page 153)
          * p = (max_levels-1) + level_base
          * q = min(p, max_level)
          * levels = q + 1
          */
         levels = gfx_umin((max_levels - 1), texture->max_level - texture->base_level) + 1;
      }
      else
         levels = 1;

      /* for cube maps we allocate space for all the faces */
      if (texture->target == GL_TEXTURE_CUBE_MAP)
      {
         blob_array_elems = num_faces(texture->target);
         assert(num_array_elems == 1);
      }
      else
         blob_array_elems = num_array_elems;

      blob = create_blob(texture->target, width, height, depth,
            blob_array_elems, levels, fmts, num_planes,
            secure_texture);
      if (!blob)
         return false;

      /* create the texel arrays for this face and level */
      texture->img[face][level]= khrn_image_create(blob, face,
            num_array_elems, level - texture->base_level, api_fmt);
      KHRN_MEM_ASSIGN(blob, NULL);
      if (!texture->img[face][level])
         return false;

      return true;
   }

   /* 2 - see if we have space in the main blob for this level and dimensions
    * and fmt*/
   texture_get_main_blob(texture, &main_blob, &at_level);
   if (main_blob && (level >= at_level))
   {
      /* main blob always contains all the faces for CUBE */
      if (texture->target == GL_TEXTURE_CUBE_MAP)
         blob_array_elems = num_faces(texture->target);
      else
         blob_array_elems = num_array_elems;

      if (khrn_blob_has_level_with_spec(main_blob, level - at_level, width,
               height, depth, blob_array_elems, fmts, num_planes))
      {
         /* we fit in the main blob */
         texture->img[face][level]= khrn_image_create(main_blob,
               face, num_array_elems, level - at_level, api_fmt);
         if (!texture->img[face][level])
            return false;

         return true;
      }
   }

   /* 3 - we create a fragmented blob that has space for just this
    * face,level and num_array_elems */
   frag = create_blob(texture->target, width, height, depth, num_array_elems,
         1, fmts, num_planes, secure_texture);
   if (!frag)
      return false;

   /* create the texel arrays for this face and level */
   texture->img[face][level]= khrn_image_create(frag, 0,
         num_array_elems, 0, api_fmt);
   KHRN_MEM_ASSIGN(frag, NULL);
   if (!texture->img[face][level])
      return false;
   return true;
}

/* if we already have an image for this face and level,check that is has the
 * same dimensions and fmt as the one we need; if not, release the existent one
 * (if there is one) and create a new one);
 * return true if the operation is succesful;
 * Note: this functions calls texture_unbind(and orphaning) if the texture gets
 * respecified; As a result any texture->image[face][level] can end up being a
 * new pointer image to a new blob
 * */
static bool texture_check_or_create_image(GLXX_TEXTURE_T *texture, unsigned
      face, unsigned level, unsigned width, unsigned height, unsigned depth,
      unsigned  num_array_elems, const GFX_LFMT_T *fmts, unsigned num_planes,
      GFX_LFMT_T api_fmt, glxx_context_fences *fences,
      bool secure_texture)
{
   bool ok = true;

   /* if we have an image, make sure that we have the same dimensions and format,
    * and secure mode otherwise, release it */
   if (texture->img[face][level] &&
       (!khrn_image_match_dim_and_fmt(texture->img[face][level], width,
                                     height, depth, num_array_elems, fmts, num_planes) ||
       (texture->img[face][level]->blob->secure != secure_texture)))
   {
       /* not the correct lfmt,  dim or secure mode - release it (do the orhpaning if necessary)) */
       if (!texture_unbind(texture, fences))
          return false;
       KHRN_MEM_ASSIGN(texture->img[face][level], NULL);
   }

   if (!texture->img[face][level])
   {
      if (!texture_unbind(texture, fences))
         return false;
      ok = texture_create_image(texture, face, level, width, height,
            depth, num_array_elems, fmts, num_planes, api_fmt,
            secure_texture);
   }
   return ok;
}

/* in es1 every time the user uploads data to level 0, if GENERATE_MIPMAPS is
 * true, we are supposed to generate all the mipmaps. Only es1 can set this
 * param to true */
static bool es1_update_mipmaps(GLXX_TEXTURE_T *texture, unsigned level,
      glxx_context_fences *fences)
{
   bool ok = true;

   if (level == 0 && texture->generate_mipmap) {
      GLenum error;
      ok = glxx_texture_generate_mipmap(texture, fences, &error);
      assert(error == GL_NO_ERROR || error == GL_OUT_OF_MEMORY);
   }

   return ok;
}

/* all the GLenum values where checked and contain supported values. This
 * function should not be called for immutable texture (use
 * glxx_texture_storage)*/
bool glxx_texture_image(GLXX_TEXTURE_T *texture, unsigned face, unsigned level,
      GLenum internalformat, unsigned width, unsigned height, unsigned depth,
      GLenum fmt, GLenum type, const GLXX_PIXEL_STORE_STATE_T *pack_unpack,
      GLXX_BUFFER_T *pixel_buffer, const void *pixels,
      glxx_context_fences *fences, GLenum *error, bool secure_texture)
{
   GFX_LFMT_T dst_fmts[GFX_BUFFER_MAX_PLANES], src_lfmt, api_fmt;
   GFX_BUFFER_DESC_T src_desc;
   unsigned dim, dst_num_planes, num_array_elems;
   bool ok;
   struct glxx_pixels_info src_info;

   assert(face < MAX_FACES && level < KHRN_MAX_MIP_LEVELS);
   assert(texture->immutable_format == GFX_LFMT_NONE);
   *error = GL_NO_ERROR;

   if (!texture_unbind(texture, fences))
   {
      *error = GL_OUT_OF_MEMORY;
      return false;
   }

   if (width == 0 || height == 0 || depth == 0)
   {
      KHRN_MEM_ASSIGN(texture->img[face][level], NULL);
      return true;
   }

   api_fmt = gfx_api_fmt_from_internalformat(type, internalformat);
   glxx_hw_fmts_from_api_fmt(&dst_num_planes, dst_fmts, api_fmt);

   glxx_tex_transform_dim_for_target(texture->target, &width, &height,
         &depth, &num_array_elems);

   /* guess FBO creation.  Create in protected space */
   bool secure = (pixel_buffer == NULL && pixels == NULL && secure_texture);

   ok = texture_check_or_create_image(texture, face, level, width, height,
            depth, num_array_elems, dst_fmts, dst_num_planes, api_fmt, fences,
            secure);

   if (!ok)
   {
      *error = GL_OUT_OF_MEMORY;
      return false;
   }

   glxx_get_pack_unpack_info(pack_unpack, false, width, height, fmt,
         type, &src_info);

   dim = get_target_num_dimensions(texture->target);
   src_lfmt = gfx_lfmt_from_externalformat(fmt, type, internalformat);
   gfx_lfmt_set_dims(&src_lfmt, gfx_lfmt_dims_to_enum(dim));
   src_lfmt = gfx_lfmt_to_rso(src_lfmt);

   src_desc.width = width;
   src_desc.height = height;
   src_desc.depth = depth;
   src_desc.num_planes = 1;
   src_desc.planes[0].lfmt = src_lfmt;
   src_desc.planes[0].offset = 0;
   src_desc.planes[0].pitch = src_info.stride;
   src_desc.planes[0].slice_pitch = depth > 1 ? src_info.slice_pitch : 0;

   ok = upload_data(texture->img[face][level], 0, 0, 0, 0, num_array_elems,
         &src_desc, src_info.offset, src_info.slice_pitch, pixel_buffer, pixels,
         fences, error);
   if (!ok)
      return false;

   ok = es1_update_mipmaps(texture, level, fences);
   if (!ok) *error = GL_OUT_OF_MEMORY;
   return ok;
}

/* internal format is a sized internal format or compressed;
 * creates all the texel arrays for the an immutable texture and if it suceeds
 * sets the immutable_format to true and the immutable_levels to clamped
 * levels allowed or levels supplied */
static bool texture_create_immutable_images(GLXX_TEXTURE_T *texture, unsigned
      levels, unsigned width, unsigned height, unsigned depth, unsigned
      num_array_elems, const GFX_LFMT_T *fmts, unsigned num_planes, GFX_LFMT_T api_fmt)
{
   unsigned max_levels, face, level, blob_array_elems, blob_level;
   unsigned faces = num_faces(texture->target);
   KHRN_BLOB_T *blob;

   assert(texture->immutable_format == GFX_LFMT_NONE);
   assert(width > 0 && height > 0 &&  depth > 0 && levels > 0);
   assert(!texture_has_any_blobs(texture));

   max_levels = glxx_tex_max_levels_from_dim(texture->target, width, height,
         depth);

   /* clamp the levels to the allowed ones */
   if (levels > max_levels)
      levels = max_levels;

   if (texture->target == GL_TEXTURE_CUBE_MAP)
   {
      /* for cubemap the blob contains all the faces */
      blob_array_elems = faces;
      assert(num_array_elems == 1);
   }
   else
      blob_array_elems = num_array_elems;

   blob_level = 0;
   blob = create_blob(texture->target, width, height, depth, blob_array_elems,
         levels, fmts, num_planes, texture->create_secure);

   if (!blob)
      return false;

   /* create the texel arrays */
   for (face = 0; face < faces; face++)
   {
      for (level = 0; level < levels; level++)
      {
         texture->img[face][level] = khrn_image_create(blob, face,
               num_array_elems, level - blob_level, api_fmt);
         if (!texture->img[face][level])
            goto fail;
      }
   }
   /* release the reference in blob since we already created the images */
   KHRN_MEM_ASSIGN(blob, NULL);
   texture->immutable_format = api_fmt;
   texture->immutable_levels = levels;
   return true;

fail:
   release_images(texture->img, faces, 0, levels);
   KHRN_MEM_ASSIGN(blob, NULL);
   return false;
}

/* params were checked and contain allowed values;
 * internalformat must be a compressed or sized internal fromat */
bool glxx_texture_storage(GLXX_TEXTURE_T *texture,unsigned levels,
      GLenum internalformat, unsigned width, unsigned height, unsigned depth,
      glxx_ms_mode ms_mode, bool fixed_sample_locations)
{
   GFX_LFMT_T api_fmt;
   GFX_LFMT_T fmts[GFX_BUFFER_MAX_PLANES];
   unsigned num_array_elems, num_planes;
   bool ok;

   assert(width > 0 && height > 0 &&  depth > 0 && levels > 0);
   if (fixed_sample_locations == false)
      assert(glxx_tex_target_is_multisample(texture->target));
   if (glxx_tex_target_is_multisample(texture->target))
      assert(ms_mode != GLXX_NO_MS);

   texture_unbind_and_release_all(texture);

   api_fmt = gfx_api_fmt_from_sized_internalformat_maybe(internalformat);
   if (api_fmt == GFX_LFMT_NONE)
   {
      assert(ms_mode == GLXX_NO_MS);
      /* if internalformat is not sized, must be compressed */
      api_fmt = gfx_lfmt_from_compressed_internalformat(
         khrn_get_lfmt_translate_exts(), internalformat);
   }

   glxx_hw_fmts_from_api_fmt(&num_planes, fmts, api_fmt);
#if !V3D_VER_AT_LEAST(4,0,2,0)
   // multisample color images must be stored using the TLB raw format.
   if (ms_mode != GLXX_NO_MS && gfx_lfmt_has_color(fmts[0]))
   {
      assert(num_planes == 1);
      fmts[0] = gfx_lfmt_translate_internal_raw_mode(fmts[0]);
   }
#endif

   glxx_tex_transform_dim_for_target(texture->target, &width, &height, &depth,
         &num_array_elems);

   unsigned scale = glxx_ms_mode_get_scale(ms_mode);
   width *= scale;
   height *= scale;
   if (ms_mode != GLXX_NO_MS)
      assert(depth == 1);

   ok = texture_create_immutable_images(texture, levels, width, height, depth,
      num_array_elems, fmts, num_planes, api_fmt);

   if (ok)
   {
      texture->ms_mode = ms_mode;
      texture->fixed_sample_locations = fixed_sample_locations;
   }

   return ok;
}

/* es3.0 spec, 3.8.10 Texture Minification, Mipmapping */
static bool try_get_num_levels(const GLXX_TEXTURE_T *texture,
      bool only_base_level, unsigned *base_level, unsigned *num_levels)
{
   unsigned q;

   *base_level = glxx_texture_clamped_base_level(texture);

   if ((texture->target == GL_TEXTURE_BUFFER) ||
        glxx_tex_target_is_multisample(texture->target))
   {
      //these textures have only level 0
      *num_levels = 1;
      if (*base_level != 0)
         return false;
      if (texture->target == GL_TEXTURE_BUFFER)
         return texture_buffer_possible_complete(texture);
      else
      {
         if(texture->img[0][0] != NULL)
            return true;
         return false;
      }
   }

   if (texture->immutable_format != GFX_LFMT_NONE)
   {
      assert(texture->immutable_levels >= 1);
      q = gfx_uclamp(texture->max_level, *base_level, texture->immutable_levels - 1);
   }
   else
   {
      KHRN_IMAGE_T *img0 = texture->img[0][*base_level];
      /* we need image at base level to calculate values for mipmapping */
      if (!img0)
         return false;

      /* Mipmapping (page 153 es3.0)
       * p = (max_levels-1) + level_base
       * q = min(p, max_level)
       */
      unsigned max_levels = khrn_image_get_max_levels(img0);
      unsigned p = (max_levels-1) + *base_level;
      q = gfx_umin(p, texture->max_level);
   }
   if (only_base_level)
   {
      //texture->max_level does not affect non-mipmapped textures
      *num_levels = 1;
   }
   else
   {
      if (q >= *base_level)
         *num_levels = q - *base_level + 1;
      else
         return false;
   }
   return true;
}

/* Returns true if the texture is complete:
 *   base complete = base level is complete
 *   mipmap complete = texture is mipmap complete
 * A 2D(3D, 2D image array texture) is mip_complete if:
 *   a) the set of mipmap arrays base_level to q were specified and have the same
 *   effective internal format
 *   b) the dimensions of the arrays follow the
     c) base_level <= level_max
 * A cube map texture is cube complete if each face is mip complete(rules above) and:
 *   d) the base_level of each of the six texture images making up the cube
 *   have identical, positive and square dimensions
 *   e) the base_level arrays were each specified with the same internal format
 * An immutable texture is always complete, but base_level and level_max could
 * be clamped to the allowed values; because of this we return a new base_level
 * which is different than texture->base_level (see page 153, mipmapping).
 * out params (valid only when we return COMPLETE):
 *  base_level: the base level to use if the texture is complete
 *  num_levels: how many levels from the base level we are supposed to use for texturing
 */
bool glxx_texture_check_completeness(const GLXX_TEXTURE_T *texture,
   bool base_complete, unsigned *base_level, unsigned *num_levels)
{
   bool ok;

   ok = try_get_num_levels(texture, base_complete, base_level, num_levels);
   if (!ok)
      return false;
   assert((base_complete && (*num_levels == 1))||
        !base_complete);

   if (texture->target == GL_TEXTURE_BUFFER ||
         glxx_tex_target_is_multisample(texture->target) ||
         texture->immutable_format != GFX_LFMT_NONE)
      return true;

   unsigned face = 0;
   KHRN_IMAGE_T *img0 = texture->img[face][*base_level];
   if (!img0)
      return false;

   unsigned faces = num_faces(texture->target);
   for (face = 1 ; face < faces; face++)
   {
      /* check d) and e) */
      KHRN_IMAGE_T *img = texture->img[face][*base_level];
      if (!img || !khrn_image_match_fmt_and_dim(img, img0))
         return false;
   }

   /* we checked for base_completeness up till now */
   if (base_complete)
      return true;

   /* c) base_level <= level_max */
   assert(*base_level <= texture->max_level);

   /* check a) and b)*/
   for (face = 0; face < faces; face++)
   {
      for (unsigned i = 1; i < *num_levels; i++)
      {
         KHRN_IMAGE_T *img = texture->img[face][*base_level + i];
         if (!img)
            return false;

         if (!khrn_image_is_miplevel(img, i, img0))
            return false;
      }
   }

   return true;
}

/* create a blob with enough storage for specified num_levels and faces, where
 * the dimensions and format of the storage are taken from img0; create wrapper
 * images that point new blob, between (base_level, base_level + num_levels);
 * texture target is needed just to set flags for the storage  */
static bool create_blob_and_images(enum glxx_tex_target tex_target,
      const KHRN_IMAGE_T* img0,
      unsigned faces, unsigned base_level, unsigned num_levels,
      KHRN_IMAGE_T* new_img[MAX_FACES][KHRN_MAX_MIP_LEVELS])
{
   unsigned face, level, num_array_elems, blob_array_elems;
   KHRN_BLOB_T *new_blob;
   GFX_LFMT_T fmts[GFX_BUFFER_MAX_PLANES];
   unsigned width, height, depth, num_planes;

   memset(new_img, 0, MAX_FACES * KHRN_MAX_MIP_LEVELS * sizeof(uintptr_t));

   khrn_image_get_dimensions(img0, &width, &height, &depth, &num_array_elems);
   khrn_image_get_fmts(img0, fmts, &num_planes);

   /* create a blob that has enough storage for all faces + num_levels */
   if (faces != 1)
   {
      assert(num_array_elems == 1);
      assert(tex_target == GL_TEXTURE_CUBE_MAP);
      blob_array_elems = faces;
   }
   else
      blob_array_elems = num_array_elems;

   // NOTE: Inherit the secure state of the blob from base level image
   new_blob = create_blob(tex_target, width, height, depth,
         blob_array_elems, num_levels, fmts, num_planes, img0->blob->secure);

   if (!new_blob)
      return false;

   for (face = 0; face < faces; face++)
   {
      for (level = base_level; level < base_level + num_levels; level++)
      {
         new_img[face][level] = khrn_image_create(new_blob, face,
               num_array_elems, level - base_level,
               img0->api_fmt);
         if (!new_img[face][level])
            goto fail;
      }
   }

   /* we always release the new_blob because the new_img have references to it */
   KHRN_MEM_ASSIGN(new_blob, NULL);
   return true;

fail:
   release_images(new_img, faces, base_level, num_levels);
   KHRN_MEM_ASSIGN(new_blob, NULL);
   return false;
}

/* creates an image with storage for one face and one level,
 * where the dimensions and storage format are taken from img0;
 * texture target is needed just to set flags for the storage  */
static KHRN_IMAGE_T* create_image_with_storage(const KHRN_IMAGE_T* img0,
      enum glxx_tex_target tex_target)
{
   GFX_LFMT_T fmts[GFX_BUFFER_MAX_PLANES];
   unsigned width, height, depth, num_planes, num_array_elems;
   KHRN_BLOB_T *frag;

   khrn_image_get_dimensions(img0, &width, &height, &depth, &num_array_elems);
   khrn_image_get_fmts(img0, fmts, &num_planes);

   frag = create_blob(tex_target, width, height, depth, num_array_elems, 1,
         fmts, num_planes, img0->blob->secure);

   if (!frag)
   {
      return NULL;
   }
   KHRN_IMAGE_T *img = khrn_image_create(frag, 0, num_array_elems, 0,
               img0->api_fmt);

   KHRN_MEM_ASSIGN(frag, NULL);
   return img;
}

static bool copy_images_content(KHRN_IMAGE_T* new_img[MAX_FACES][KHRN_MAX_MIP_LEVELS],
   KHRN_IMAGE_T* img[MAX_FACES][KHRN_MAX_MIP_LEVELS],
   unsigned faces, unsigned base_level, unsigned num_levels,
   glxx_context_fences *fences)
{
   for (unsigned face = 0; face < faces; face++)
   {
      for (unsigned level = base_level; level < (base_level + num_levels); level++)
      {
         assert(khrn_image_match_fmt_and_dim(new_img[face][level], img[face][level]));
         if (!khrn_image_convert(new_img[face][level],
                  img[face][level], fences, in_secure_context()))
            return false;
      }
   }
   return true;
}

bool glxx_texture_is_cube_complete(GLXX_TEXTURE_T *texture)
{
   unsigned base_level, num_levels;

   assert(texture->target == GL_TEXTURE_CUBE_MAP);
   return glxx_texture_check_completeness(texture, true, &base_level, &num_levels);
}

struct range
{
   unsigned start;
   unsigned end; /* not including this element */
};

/* a - b */
static void ranges_difference(const struct range *a, const struct range *b,
      struct range out[2], unsigned *n_out)
{
   *n_out = 0;

   if ((a->end <= b->start) || (b->end <= a->start))
   {
      *n_out = 1;
      out[0] = *a;
      return;
   }
   else
   {
      if (a->start < b->start)
      {
         out[*n_out].start = a->start;
         out[*n_out].end = b->start;
         (*n_out)++;
      }
      if (b->end < a->end)
      {
         out[*n_out].start = b->end;
         out[*n_out].end = a->end;
         (*n_out)++;
      }
   }
}

/* Definition: fragmented blob is a blob that has storage just for one image (1
 *             mip-level, 1 face, one or many array elements)
 * Create fragmented blobs(and copy data) for all the texture images at mip
 * levels outside range (base_level, base_level + num_levels) that point to the
 * main blob
 * Return true if the operation succeeds.
 */
static bool move_existing_to_fragmented_blobs(GLXX_TEXTURE_T *texture,
      unsigned base_level, unsigned num_levels, glxx_context_fences *fences)
{
   unsigned faces, face, level, k;
   struct range blob_range, exclude;
   KHRN_BLOB_T *blob;
   unsigned at_level;
   KHRN_IMAGE_T *src_img, **dst_img;

   texture_get_main_blob(texture, &blob, &at_level);
   if (!blob)
      return true;

   faces = num_faces(texture->target);

   if (texture->target == GL_TEXTURE_CUBE_MAP)
   {
      /* for a cube we always have all the faces in a main blob */
      assert(blob->num_array_elems == faces && faces == 6);
      assert(faces == 6);
   }
   else
   {
      /* a main blob must have more than one mip levels (for non cube) */
      assert(blob->num_mip_levels > 1);
   }

   blob_range.start = at_level;
   blob_range.end = at_level + blob->num_mip_levels;
   exclude.start = base_level;
   exclude.end = base_level + num_levels;

   struct range range[2];
   unsigned n_ranges;
   ranges_difference(&blob_range, &exclude, range, &n_ranges);
   if (n_ranges == 0)
   {
      /* nothing to do, blob range inside exclude range */
      return true;
   }

   KHRN_IMAGE_T* new_img[MAX_FACES][KHRN_MAX_MIP_LEVELS];
   memset(new_img, 0, MAX_FACES * KHRN_MAX_MIP_LEVELS * sizeof(uintptr_t));

   /* we just need to create fragmented blobs for all the images in
    * existing blob outside the intersection range */
   for (face = 0; face < faces; face ++)
   {
      for ( k = 0; k < n_ranges; k++)
      {
         for (level = range[k].start; level < range[k].end; level++)
         {
            src_img = texture->img[face][level];
            dst_img = &new_img[face][level];
            if (!src_img || src_img->blob != blob)
               continue;

            /* create a fragmented blob that has space for just one image
             * (with dims and fmt as src_img) */
            *dst_img = create_image_with_storage(src_img, texture->target);
            if (!*dst_img)

               goto fail;

            /* copy data */
            if (!khrn_image_convert(*dst_img, src_img, fences, in_secure_context()))
               goto fail;
         }
      }
   }

   /* we got the new images and we didn't ran out of mem, assign them in the
    * texture */
   for (face = 0; face < faces; face ++)
   {
      for ( k = 0; k < n_ranges; k++)
      {
         for (level = range[k].start; level < range[k].end; level++)
         {
            src_img = texture->img[face][level];
            if (!src_img || src_img->blob != blob)
               continue;
            KHRN_MEM_ASSIGN(texture->img[face][level], new_img[face][level]);
            KHRN_MEM_ASSIGN(new_img[face][level], NULL);
         }
      }
   }

   return true;

fail:
   release_images(new_img, faces, 0, KHRN_MAX_MIP_LEVELS);
   return false;
}

/*
 * Make a new contiguous blob with storage for num_levels, where
 * the dimensions and format of the blob are taken from
 * texture->img[0] [base_level].
 * Create new images for all this levels (pointing to this new blob).
 * Images between (copy_start_level, copy_start_level + copy_num_levels)
 * have their data copied over to the new contiguous blob.
 * The new images are installed in the texture for levels base_level,
 * base_level + num_levels,
 * Return true if we succeed.
 */
bool make_contiguous_blob_and_copy_levels(GLXX_TEXTURE_T *texture,
      unsigned base_level, unsigned num_levels,
      unsigned copy_start_level, unsigned copy_num_levels,
      glxx_context_fences *fences)
{
   KHRN_IMAGE_T *new_img[MAX_FACES][KHRN_MAX_MIP_LEVELS];
   bool ok = false;

   assert(copy_start_level >= base_level);
   assert(copy_start_level + copy_num_levels <= base_level + num_levels);

   memset(new_img, 0, MAX_FACES * KHRN_MAX_MIP_LEVELS * sizeof(uintptr_t));
   unsigned faces = num_faces(texture->target);

   if (!create_blob_and_images(texture->target, texture->img[0][base_level],
            faces, base_level, num_levels, new_img))
      return false;

   if (!copy_images_content(new_img, texture->img, faces, copy_start_level,
            copy_num_levels, fences))
      goto fail;

   /* we've just created a new main_blob;
    * we cannot have more than one main blob at one time;
    * we have to make sure that images that were in the existing blob
    * outside base_level, base_level + num_levels get their own fragmented
    * blobs */
   if (!move_existing_to_fragmented_blobs(texture,
            base_level, num_levels, fences))
      goto fail;

   /* set the new texels array in the texture */
   assign_new_images(texture, new_img, base_level, num_levels);
   ok = true;
fail:
   release_images(new_img, faces, base_level, num_levels);
   return ok;
}

static bool are_images_in_same_blob(KHRN_IMAGE_T *imgs[MAX_FACES][KHRN_MAX_MIP_LEVELS],
      unsigned faces, unsigned base_level, unsigned num_levels)
{

   KHRN_IMAGE_T *img0 = imgs[0][base_level];

   for (unsigned face = 0; face < faces; face++)
   {
      for (unsigned level = base_level; level < base_level + num_levels; level++)
      {
         if (imgs[face][level]->blob != img0->blob)
         {
            return false;
         }
      }
   }
   return true;
}

static bool filtering_ok(const GLXX_TEXTURE_T *texture, bool shadow_compare)
{
   GFX_LFMT_T api_fmt;
   if (!texture->source)
   {
      if (texture->target == GL_TEXTURE_BUFFER)
         api_fmt = texture->tex_buffer.api_fmt;
      else
         api_fmt = texture->img[0][glxx_texture_clamped_base_level(texture)]->api_fmt;
   }
   else
      api_fmt = egl_image_get_image(texture->source)->api_fmt;

   if (gfx_sized_internalformat_from_api_fmt_maybe(api_fmt) != GL_NONE)
   {
      if ((gfx_lfmt_has_color(api_fmt) && !glxx_is_texture_filterable_api_fmt(api_fmt)) ||
          (KHRN_GLES31_DRIVER && gfx_lfmt_has_depth_stencil(api_fmt) &&
            texture->ds_texture_mode == GL_STENCIL_INDEX) ||
          (gfx_lfmt_has_depth(api_fmt) && !shadow_compare) ||
          (gfx_lfmt_has_stencil(api_fmt) && !gfx_lfmt_has_depth(api_fmt)))
         return false;
   }

   return true;
}

static bool glxx_texture_complete_with_sampler(const GLXX_TEXTURE_T *texture,
      const  GLXX_TEXTURE_SAMPLER_STATE_T *sampler,
      unsigned *base_level, unsigned *num_levels)
{
   bool base_complete = !requires_mipmaps(sampler->filter.min);
   if (!glxx_texture_check_completeness(texture, base_complete, base_level, num_levels))
      return false;

   if (filtering_ok(texture, sampler->compare_mode != GL_NONE))
      return true;
   return (sampler->filter.mag == GL_NEAREST) &&
      (sampler->filter.min == GL_NEAREST || sampler->filter.min == GL_NEAREST_MIPMAP_NEAREST);
}

/* If the texture is complete it makes sure that all the needed images are in a contiguous blob.
 * Return COMPLETE in case of success.
 * In the case of texture immutable, base_level and level_max could be clamped
 * to the allowed values; because of that we return a new base_level which is
 * different than texture->base_level.
 * out params (valid only when we return COMPLETE):
 *  base_level: the base level to use if the texture is complete
 *  num_levels: how many levels from the base level we are supposed to use for texturing
 */
enum glxx_tex_completeness
glxx_texture_ensure_contiguous_blob_if_complete(GLXX_TEXTURE_T *texture,
      const  GLXX_TEXTURE_SAMPLER_STATE_T *sampler, unsigned *base_level,
      unsigned *num_levels, glxx_context_fences *fences)
{
   if (!glxx_texture_complete_with_sampler(texture, sampler, base_level, num_levels))
      return INCOMPLETE;

   if (texture->immutable_format == GFX_LFMT_NONE)
   {
      if (texture->target == GL_TEXTURE_BUFFER)
         return texture_buffer_create_images(texture);

      bool same_blob;
      unsigned faces = num_faces(texture->target);
      same_blob = are_images_in_same_blob(texture->img, faces, *base_level, *num_levels);
      if (!same_blob)
      {
         if (!make_contiguous_blob_and_copy_levels(texture, *base_level,
                  *num_levels, *base_level, *num_levels, fences))
            return OUT_OF_MEMORY;
      }
   }
   return COMPLETE;
}

/* Iff COMPLETE is returned, *image will have been set and the image's reference
 * count incremented. The image must be released by calling khrn_mem_release. */
static enum glxx_tex_completeness get_base_level_image_and_num_levels(
   KHRN_IMAGE_T **image, uint32_t *num_levels,
   GLXX_TEXTURE_T *texture, const glxx_calc_image_unit *image_unit,
   const GLXX_TEXTURE_SAMPLER_STATE_T *sampler,
   glxx_context_fences *fences)
{

   if (image_unit)
   {
      if (image_unit->use_face_layer)
      {
         if (!glxx_texture_acquire_one_elem_slice(texture, image_unit->face,
                  image_unit->level, image_unit->layer, image))
            return OUT_OF_MEMORY;
      }
      else
      {
         *image = texture->img[0][image_unit->level];
         khrn_mem_acquire(*image);
      }
      *num_levels = 1;
      return COMPLETE;
   }

   unsigned base_level;
   enum glxx_tex_completeness complete = glxx_texture_ensure_contiguous_blob_if_complete(
         texture, sampler, &base_level, num_levels, fences);
   if (complete != COMPLETE)
      return complete;

   if (texture->source)
   {
      assert(base_level == 0 && *num_levels == 1);
      *image = get_image_for_texturing(texture->source, fences);
   }
   else
   {
      *image = texture->img[0][base_level];
      khrn_mem_acquire(*image);
   }
   return COMPLETE;
}

static GLenum filter_disable_mipmapping(GLenum filter)
{
   switch (filter)
   {
   case GL_NEAREST:
   case GL_NEAREST_MIPMAP_NEAREST:
   case GL_NEAREST_MIPMAP_LINEAR:
      return GL_NEAREST;
   case GL_LINEAR:
   case GL_LINEAR_MIPMAP_NEAREST:
   case GL_LINEAR_MIPMAP_LINEAR:
      return GL_LINEAR;
   default:
      unreachable();
      return 0;
   }
}

/* stride to get to the nth face/layer data*/
static uint32_t get_hw_stride(const KHRN_BLOB_T *blob, unsigned plane_i)
{
   const GFX_BUFFER_DESC_T *l0_desc = &blob->desc[0];
   const GFX_BUFFER_DESC_PLANE_T *l0_p = &l0_desc->planes[plane_i];
   if (gfx_lfmt_is_3d(l0_p->lfmt))
      /* Need to provide L0 slice pitch in arr_str.
       * Slice pitch of other levels is calculated by HW. */
      return l0_p->slice_pitch;
   if (blob->num_array_elems > 1)
      return blob->array_pitch;
   return 0; /* arr_str unused */
}

/* Converts the min filter from the GLenum representation to the internal one
 * used in the HW. */
static v3d_tmu_min_filt_t convert_min_filter(GLenum filter)
{
   switch (filter)
   {
   case GL_NEAREST:
      return V3D_TMU_MIN_FILT_NEAREST;
   case GL_LINEAR:
      return V3D_TMU_MIN_FILT_LINEAR;
   case GL_NEAREST_MIPMAP_NEAREST:
      return V3D_TMU_MIN_FILT_NEAR_MIP_NEAR;
   case GL_NEAREST_MIPMAP_LINEAR:
      return V3D_TMU_MIN_FILT_NEAR_MIP_LIN;
   case GL_LINEAR_MIPMAP_NEAREST:
      return V3D_TMU_MIN_FILT_LIN_MIP_NEAR;
   case GL_LINEAR_MIPMAP_LINEAR:
      return V3D_TMU_MIN_FILT_LIN_MIP_LIN;
   default:
      unreachable();
      return V3D_TMU_MIN_FILT_INVALID;
   }
}

static v3d_tmu_mag_filt_t convert_mag_filter(GLenum filter)
{
   switch (filter)
   {
   case GL_NEAREST:
      return V3D_TMU_MAG_FILT_NEAREST;
   case GL_LINEAR:
      return V3D_TMU_MAG_FILT_LINEAR;
   default:
      unreachable();
      return V3D_TMU_MAG_FILT_INVALID;
   }
}

static v3d_tmu_filter_t get_tmu_filter(
   GLenum mag_filter, GLenum min_filter, float anisotropy,
   bool unnormalised_coords)
{
   /* disable mipmapping if using unnormalised coords */
   if (unnormalised_coords)
   {
      min_filter = filter_disable_mipmapping(min_filter);
      anisotropy = 0.0f;
   }

   if (anisotropy > 1.0f)
   {
      /* Override filter with anisotropic. */
      if (anisotropy > 8.0f)
         return V3D_TMU_FILTER_ANISOTROPIC16;
      if (anisotropy > 4.0f)
         return V3D_TMU_FILTER_ANISOTROPIC8;
      if (anisotropy > 2.0f)
         return V3D_TMU_FILTER_ANISOTROPIC4;
      return V3D_TMU_FILTER_ANISOTROPIC2;
   }

   return convert_mag_filter(mag_filter) |
      convert_min_filter(min_filter) << 1;
}

static bool uses_border_color(const GLXX_TEXTURE_SAMPLER_STATE_T *sampler)
{
   return
      sampler->wrap.s == GL_CLAMP_TO_BORDER ||
      sampler->wrap.t == GL_CLAMP_TO_BORDER ||
      sampler->wrap.r == GL_CLAMP_TO_BORDER;
}

static bool swizzles_01_or(const v3d_tmu_swizzle_t swizzles[4],
   v3d_tmu_swizzle_t cmp0, v3d_tmu_swizzle_t cmp1, v3d_tmu_swizzle_t cmp2, v3d_tmu_swizzle_t cmp3)
{
   v3d_tmu_swizzle_t cmp[] = {cmp0, cmp1, cmp2, cmp3};
   for (unsigned i = 0; i != 4; ++i)
      if ((swizzles[i] != V3D_TMU_SWIZZLE_0) &&
         (swizzles[i] != V3D_TMU_SWIZZLE_1) &&
         (swizzles[i] != cmp[i]))
         return false;
   return true;
}

/* This must be called *before* composing tmu_trans->swizzles with texture->swizzle! */
static GFX_LFMT_T get_hw_border_fmt(const GFX_LFMT_TMU_TRANSLATION_T *tmu_trans, bool output_32)
{
   #define SWIZZLES_01_OR(R,G,B,A)        \
      swizzles_01_or(tmu_trans->swizzles, \
         V3D_TMU_SWIZZLE_##R,             \
         V3D_TMU_SWIZZLE_##G,             \
         V3D_TMU_SWIZZLE_##B,             \
         V3D_TMU_SWIZZLE_##A)

   GFX_LFMT_T fmt;

   if (v3d_tmu_is_depth_type(tmu_trans->type))
   {
#if V3D_VER_AT_LEAST(4,0,2,0)
      /* Depth border is always applied in 32F on new hardware */
      fmt = GFX_LFMT_R32_FLOAT;
#else
      /* Depth border colour is applied at a different stage in the pipeline.
       * Format of border colour matches in-memory format of texture data. */
      fmt = gfx_lfmt_translate_from_tmu_type(tmu_trans->type, /*srgb=*/false);
#endif

      /* See comment in else but we don't expect any format swizzling in this case */
      assert(SWIZZLES_01_OR(R,G,B,A));
   }
   else
   {
      /* Border colour should be in blend format */
      fmt = v3d_get_tmu_blend_fmt(tmu_trans->type, tmu_trans->srgb, /*shadow=*/false, output_32);

      /* Border colour is handled by HW before swizzle, but format bit of
       * swizzle (what's currently in tmu_trans->swizzles) should not be
       * applied to border colour. So we need to apply the inverse of the
       * format swizzle here... */
      assert(gfx_lfmt_get_channels(&fmt) == GFX_LFMT_CHANNELS_RGBA);
      if (SWIZZLES_01_OR(A,B,G,R))
         gfx_lfmt_set_channels(&fmt, GFX_LFMT_CHANNELS_ABGR);
      else if (SWIZZLES_01_OR(B,G,R,A))
         gfx_lfmt_set_channels(&fmt, GFX_LFMT_CHANNELS_BGRA);
      else if (SWIZZLES_01_OR(G,B,A,R))
         gfx_lfmt_set_channels(&fmt, GFX_LFMT_CHANNELS_ARGB);
      else if (SWIZZLES_01_OR(R,G,B,A))
         gfx_lfmt_set_channels(&fmt, GFX_LFMT_CHANNELS_RGBA);
      else if (SWIZZLES_01_OR(R,R,R,G))
         gfx_lfmt_set_channels(&fmt, GFX_LFMT_CHANNELS_RAXX);
      else
         unreachable();
   }

   return fmt;

   #undef SWIZZLES_01_OR
}

static void get_hw_border(uint8_t packed_bcol[16], const uint32_t tex_border_color[4], GFX_LFMT_T fmt)
{
   memset(packed_bcol, 0, 16);

   switch (fmt)
   {
#if !V3D_VER_AT_LEAST(4,0,2,0)
   case GFX_LFMT_R16_UNORM:
   {
      uint32_t packed = gfx_float_to_unorm(gfx_float_from_bits(tex_border_color[0]), 16);
      memcpy(packed_bcol, &packed, 2);
      return;
   }
   case GFX_LFMT_R24X8_UNORM:
   {
      uint32_t packed = gfx_float_to_unorm(gfx_float_from_bits(tex_border_color[0]), 24);
      memcpy(packed_bcol, &packed, 4);
      return;
   }
   case GFX_LFMT_X8R24_UNORM:
   {
      uint32_t packed = gfx_float_to_unorm(gfx_float_from_bits(tex_border_color[0]), 24) << 8;
      memcpy(packed_bcol, &packed, 4);
      return;
   }
#endif
   case GFX_LFMT_R32_FLOAT:
      memcpy(packed_bcol, tex_border_color, 4);
      return;
   default:
      break;
   }

   size_t bytes_per_channel;
   uint32_t bcol[4];
   switch (gfx_lfmt_get_base(&fmt))
   {
   case GFX_LFMT_BASE_C16_C16_C16_C16:
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
      bytes_per_channel = 2;
      for (unsigned i = 0; i != 4; ++i)
      {
         uint32_t c = tex_border_color[i];
         switch (fmt & ~GFX_LFMT_CHANNELS_MASK)
         {
         case GFX_LFMT_C16_C16_C16_C16_FLOAT:         bcol[i] = gfx_floatbits_to_float16(c); break;
         case GFX_LFMT_C16_C16_C16_C16_UNORM:         bcol[i] = gfx_float_to_unorm(gfx_float_from_bits(c), 16); break;
         case GFX_LFMT_C16_C16_C16_C16_SNORM:         bcol[i] = gfx_float_to_snorm(gfx_float_from_bits(c), 16); break;
                                                      /* Bit 15 is not ignored by HW -- it must match bit 14! */
         case GFX_LFMT_C15X1_C15X1_C15X1_C15X1_SNORM: bcol[i] = gfx_sext(gfx_float_to_snorm(gfx_float_from_bits(c), 15), 15) & 0xffff; break;
         default:                                     unreachable();
         }
      }
      break;
   case GFX_LFMT_BASE_C32_C32_C32_C32:
      bytes_per_channel = 4;
      for (unsigned i = 0; i != 4; ++i)
         bcol[i] = tex_border_color[i];

      assert((fmt & ~GFX_LFMT_CHANNELS_MASK) == GFX_LFMT_C32_C32_C32_C32_FLOAT ||
             (fmt & ~GFX_LFMT_CHANNELS_MASK) == GFX_LFMT_C32_C32_C32_C32_INT ||
             (fmt & ~GFX_LFMT_CHANNELS_MASK) == GFX_LFMT_C32_C32_C32_C32_UINT  );
      break;
   default:
      unreachable();
   }

   switch (gfx_lfmt_get_channels(&fmt))
   {
#if V3D_VER_AT_LEAST(4,0,2,0)
   #define PACK(A,B,C,D)                                       \
      memcpy(packed_bcol + 0,  &bcol[A], bytes_per_channel);   \
      memcpy(packed_bcol + 4,  &bcol[B], bytes_per_channel);   \
      memcpy(packed_bcol + 8,  &bcol[C], bytes_per_channel);   \
      memcpy(packed_bcol + 12, &bcol[D], bytes_per_channel);
#else
   #define PACK(A,B,C,D)                                                            \
      memcpy(packed_bcol + (0 * bytes_per_channel), &bcol[A], bytes_per_channel);   \
      memcpy(packed_bcol + (1 * bytes_per_channel), &bcol[B], bytes_per_channel);   \
      memcpy(packed_bcol + (2 * bytes_per_channel), &bcol[C], bytes_per_channel);   \
      memcpy(packed_bcol + (3 * bytes_per_channel), &bcol[D], bytes_per_channel);
#endif
   case GFX_LFMT_CHANNELS_ABGR:  PACK(3,2,1,0); break;
   case GFX_LFMT_CHANNELS_BGRA:  PACK(2,1,0,3); break;
   case GFX_LFMT_CHANNELS_ARGB:  PACK(3,0,1,2); break;
   case GFX_LFMT_CHANNELS_RGBA:  PACK(0,1,2,3); break;
   case GFX_LFMT_CHANNELS_RAXX:  PACK(0,3,0,0); break;
   default:                      unreachable();
   #undef PACK
   }
}

static void compose_swizzles(v3d_tmu_swizzle_t swizzles[4], const unsigned tex_swizzles[4])
{
   v3d_tmu_swizzle_t swizzle_components[] = {
      V3D_TMU_SWIZZLE_0,
      V3D_TMU_SWIZZLE_1,
      swizzles[0],
      swizzles[1],
      swizzles[2],
      swizzles[3]};
   for (unsigned i = 0; i < 4; i++)
   {
      assert(tex_swizzles[i] < countof(swizzle_components));
      swizzles[i] = swizzle_components[tex_swizzles[i]];
   }
}

#if !V3D_VER_AT_LEAST(3,3,0,0)
static glsl_gadgettype_t get_glsl_gadgettype_and_adjust_hw_swizzles(
   GFX_LFMT_TMU_TRANSLATION_T *tmu_trans, bool is_32bit)
{
   if (v3d_tmu_is_depth_type(tmu_trans->type) &&
      (tmu_trans->type != V3D_TMU_TYPE_DEPTH_COMP32F))
      return GLSL_GADGETTYPE_DEPTH_FIXED;

   if (tmu_trans->shader_swizzle)
   {
      glsl_gadgettype_t gadgettype = glsl_make_shader_swizzled_gadgettype(tmu_trans->ret, tmu_trans->swizzles);
      tmu_trans->swizzles[0] = V3D_TMU_SWIZZLE_R;
      tmu_trans->swizzles[1] = V3D_TMU_SWIZZLE_G;
      tmu_trans->swizzles[2] = V3D_TMU_SWIZZLE_B;
      tmu_trans->swizzles[3] = V3D_TMU_SWIZZLE_A;
      return gadgettype;
   }
   else
   {
      bool tmu_output_32bit = v3d_tmu_auto_output_32(
         tmu_trans->type, /*shadow=*/false, /*coefficient=*/false);
      return glsl_make_tmu_swizzled_gadgettype(tmu_output_32bit, is_32bit);
   }
}
#endif

/* Converts the texture wrap setting from the GLenum
 * representation to the internal one used in the HW.
 */
static v3d_tmu_wrap_t get_hw_wrap(GLenum wrap)
{
   switch (wrap)
   {
   case GL_REPEAT:
      return V3D_TMU_WRAP_REPEAT;
   case GL_CLAMP_TO_EDGE:
      return V3D_TMU_WRAP_CLAMP;
   case GL_MIRRORED_REPEAT:
      return V3D_TMU_WRAP_MIRROR;
   case GL_CLAMP_TO_BORDER:
      return V3D_TMU_WRAP_BORDER;
   case GL_MIRROR_CLAMP_TO_EDGE_BRCM:
      return V3D_TMU_WRAP_MIRROR_ONCE;
   default:
      unreachable();
      return 0;
   }
}

/* Shadow compare handling...
 *
 *                          | Depth texture | Non-depth texture
 * -------------------------+---------------+-------------------
 * compare_mode=NONE        | Normal lookup | Normal lookup
 * Normal sampler in shader |               |
 * -------------------------+---------------+-------------------
 * compare_mode=COMPARE     | Undefined     | Normal lookup (4)
 * Normal sampler in shader | results (1)   |
 * -------------------------+---------------+-------------------
 * compare_mode=NONE        | Undefined     | Undefined
 * Shadow sampler in shader | results (2)   | results (3)
 * -------------------------+---------------+-------------------
 * compare_mode=COMPARE     | Shadow lookup | Undefined
 * Shadow sampler in shader |               | results (3)
 *
 * Spec quotes:
 * 1. "Texture Functions" section of the OpenGL ES Shading Language spec: "If a
 *    non-shadow texture call is made to a sampler that represents a depth
 *    texture with depth comparisons turned on, then results are undefined."
 * 2. "Texture Functions" section of the OpenGL ES Shading Language spec: "If a
 *    shadow texture call is made to a sampler that represents a depth texture
 *    with depth comparisons turned off, then results are undefined."
 * 3. "Texture Functions" section of the OpenGL ES Shading Language spec: "If a
 *    shadow texture call is made to a sampler that does not represent a depth
 *    texture, then results are undefined."
 * 4. "Depth Texture Comparison Mode" section of the OpenGL ES spec: "If the
 *    currently bound texture's base internal format is DEPTH_COMPONENT or
 *    DEPTH_STENCIL, then TEXTURE_COMPARE_MODE and TEXTURE_COMPARE_FUNC control
 *    the output of the texture unit as described below. Otherwise, the texture
 *    unit operates in the normal manner and texture comparison
 *    is bypassed."
 *
 * Handling:
 * 1. Shadow compare config bit always comes from the shader, so we'll just do
 *    a normal lookup.
 * 2. Shadow compare config bit always comes from the shader, so we'll just do
 *    a shadow lookup.
 * 3. We'll ask HW to do a shadow lookup. If the texture type is not a depth
 *    type we will end up just doing a normal lookup instead. I don't think
 *    this matters -- the same amount of data will be returned by the TMU.
 * 4. Same as (1). */

static void get_plane_and_lfmt_for_texturing(const KHRN_IMAGE_T *img_base,
      glxx_ds_texture_mode ds_texture_mode,
      unsigned *plane , GFX_LFMT_T *lfmt)
{
   /* we describe everything relative to the beginning of the blob */
   const GFX_BUFFER_DESC_T *desc_base = &img_base->blob->desc[0];

   if ((ds_texture_mode == GL_STENCIL_INDEX && gfx_lfmt_has_depth_stencil(img_base->api_fmt)) ||
        (gfx_lfmt_has_stencil(img_base->api_fmt) && !gfx_lfmt_has_depth(img_base->api_fmt)))
   {
      /* stencil is always in the last plane */
      *plane = khrn_image_get_num_planes(img_base) - 1;
      *lfmt = desc_base->planes[*plane].lfmt;
      *lfmt = gfx_lfmt_depth_to_x(*lfmt);
   }
   else if (gfx_lfmt_has_depth(img_base->api_fmt))
   {
      *plane = 0;
      *lfmt = desc_base->planes[*plane].lfmt;
      *lfmt = gfx_lfmt_stencil_to_x(*lfmt);
   }
   else
   {
      *plane = 0;
      *lfmt = desc_base->planes[*plane].lfmt;
   }

   return;
}

#if !V3D_VER_AT_LEAST(4,0,2,0)
static glsl_imgunit_swizzling to_glsl_imgunit_swizzling(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_SWIZZLING_MASK)
   {
      case GFX_LFMT_SWIZZLING_LT:
         return GLSL_IMGUNIT_SWIZZLING_LT;
      case GFX_LFMT_SWIZZLING_UBLINEAR:
         return GLSL_IMGUNIT_SWIZZLING_UBLINEAR;
      case GFX_LFMT_SWIZZLING_UIF:
         return GLSL_IMGUNIT_SWIZZLING_UIF;
      case GFX_LFMT_SWIZZLING_UIF_XOR:
         return GLSL_IMGUNIT_SWIZZLING_UIF_XOR;
      case GFX_LFMT_SWIZZLING_RSO:
         return GLSL_IMGUNIT_SWIZZLING_RSO;
      default:
         unreachable();
   }
}
#endif

struct hw_tex_params
{
   v3d_addr_t l0_addr;
   uint32_t w, h, d;
   uint32_t arr_str;
   struct gfx_buffer_uif_cfg uif_cfg;
   GFX_LFMT_TMU_TRANSLATION_T tmu_trans;
   bool yflip;
   uint32_t base_level;
   GFX_LFMT_T tex_lfmt;

#if !V3D_VER_AT_LEAST(4,0,2,0)
   /* these are valid only for image units */
   v3d_addr_t lx_addr; /* for bound level and selected plane */
   uint32_t lx_pitch, lx_slice_pitch;
   glsl_imgunit_swizzling lx_swizzling;
#endif
};

static bool record_tex_usage_and_get_hw_params(
   struct hw_tex_params *tp, glxx_render_state *rs,
   const GLXX_TEXTURE_T *texture, const glxx_calc_image_unit *image_unit,
   const KHRN_IMAGE_T *img_base, uint32_t num_levels,
   bool used_in_bin)
{
   bool write = false;
   if (image_unit && image_unit->write)
      write = true;

   const KHRN_BLOB_T *blob_base = img_base->blob;
   /* we need to describe everything relative to the beginning of the blob; */
   const GFX_BUFFER_DESC_T *desc_base = &blob_base->desc[0];

   assert(blob_base->usage & GFX_BUFFER_USAGE_V3D_TEXTURE);

   khrn_interlock_stages_t stages = KHRN_INTERLOCK_STAGE_RENDER
                                  | (used_in_bin ? KHRN_INTERLOCK_STAGE_BIN : 0);
   if (write)
   {
      if (!khrn_fmem_record_res_interlock_write(&rs->fmem, blob_base->res_i, stages,
            /* Could figure out tighter parts here but probably not worth it... */
            KHRN_INTERLOCK_PARTS_ALL, NULL))
         return false;
   }
   else
   {
      if (!khrn_fmem_record_res_interlock_read(&rs->fmem, blob_base->res_i, stages))
         return false;
   }
   if (write)
      rs->has_buffer_writes = true;

   unsigned plane;
   GFX_LFMT_T tex_lfmt; /* lfmt used for texturing --> d or s are used as red */
   get_plane_and_lfmt_for_texturing(img_base, texture->ds_texture_mode,
            &plane, &tex_lfmt);
   if (image_unit)
   {
      assert(plane == 0);
      tex_lfmt = gfx_lfmt_set_format(tex_lfmt, image_unit->fmt);
   }

   const GFX_BUFFER_DESC_PLANE_T *img_base_plane = &blob_base->desc[img_base->level].planes[plane];

   v3d_barrier_flags tmu_flag = V3D_BARRIER_TMU_DATA_READ;
   /* the shader might also do an imageLoad, so mark the handle accordingly */
   if (write)  tmu_flag |= V3D_BARRIER_TMU_DATA_WRITE;
   v3d_addr_t base_addr = khrn_fmem_lock_and_sync(&rs->fmem, blob_base->res_i->handle,
         used_in_bin ? tmu_flag : 0, tmu_flag);
   if (!base_addr)
      return false;
   base_addr += img_base->start_elem * blob_base->array_pitch;

   tp->w = desc_base->width;
   tp->h = desc_base->height;

   gfx_buffer_get_tmu_uif_cfg(&tp->uif_cfg, desc_base, plane);

   tp->tex_lfmt = tex_lfmt;

   gfx_lfmt_translate_tmu(&tp->tmu_trans, gfx_lfmt_ds_to_red(tex_lfmt),
      /* We must use a depth type if we're reading a depth channel -- if we
       * don't and the lookup is a shadow-compare lookup it won't work. The
       * other way around doesn't matter. */
      gfx_lfmt_has_depth(tex_lfmt) ? GFX_LFMT_TMU_DEPTH_ALWAYS : GFX_LFMT_TMU_DEPTH_DONT_CARE);
   tp->yflip = !!(tex_lfmt & GFX_LFMT_YFLIP_YFLIP);

   tp->base_level = img_base->level;

   v3d_addr_t img_base_plane_addr = base_addr + img_base_plane->offset;

   if (blob_base->desc[0].depth != 1 &&
       (texture->target != GL_TEXTURE_3D || (image_unit && image_unit->use_face_layer)))
   {
      assert(img_base->num_slices == 1);
      img_base_plane_addr += img_base->start_slice * img_base_plane->slice_pitch;

      /* texturing from a slice from a 3D image -  calculate a gfx_buffer_desc
       * level0 as if we have a 2D texture with the same widthxheigh as our 3D
       * image level0, but with depth = 1;
       * calculate offset between level_x and level0 in this image with depth=1
       * (level_x = img_base->level);
       * the new base_addr will be offset + khrn_image_get_offest(img_base, 0) */

      GFX_BUFFER_DESC_T desc_2d[KHRN_MAX_MIP_LEVELS];
      v3d_tmu_calc_mip_levels(desc_2d,
            V3D_TMU_LTYPE_2D, tp->tmu_trans.srgb, tp->tmu_trans.type,
            &tp->uif_cfg, /*arr_str=*/0,
            desc_base->width, desc_base->height, /*depth=*/1, blob_base->num_mip_levels);

      size_t offset = desc_2d[0].planes[0].offset - desc_2d[img_base->level].planes[0].offset;
      tp->l0_addr = img_base_plane_addr + offset;

      /* Unused... */
      tp->d = 1;
      tp->arr_str = 0;
   }
   else
   {
      assert(img_base->start_slice == 0);

      tp->l0_addr = base_addr + desc_base->planes[plane].offset;

      tp->d = desc_base->depth * khrn_image_get_num_elems(img_base);

      /* For image units which are using a single face, img_base will already
       * contain only a single 2d slice, so num_elems should be 1 and is
       * correct. For cubemaps img_base *always* contains only a single face,
       * so num_elems will always return 1. For cubemap arrays img_base
       * contains the whole image and so will have num_elems == array_layers * 6.
       * Image units expect a depth of 6*array_layers, but textures expect just
       * array_layers. Correct the two cases as appropriate. */
      if (image_unit && image_unit->use_face_layer)
         assert(tp->d == 1);
      else if (image_unit && texture->target == GL_TEXTURE_CUBE_MAP)
         tp->d *= 6;
      else if (!image_unit && texture->target == GL_TEXTURE_CUBE_MAP_ARRAY)
      {
         assert((tp->d % 6) == 0);
         tp->d /= 6;
      }

      tp->arr_str = get_hw_stride(blob_base, plane);
   }

   assert(v3d_addr_aligned(tp->l0_addr, V3D_TMU_ML_ALIGN));

   if (texture->target == GL_TEXTURE_BUFFER)
   {
      GFX_LFMT_BASE_DETAIL_T bd;
      gfx_lfmt_base_detail(&bd, desc_base->planes[0].lfmt);

      /* this assert is not true for RGB32(F/I/UI), but we are going to treat
       * those as R32 textures*/
      assert(GLXX_CONFIG_TEXBUFFER_ARR_ELEM_BYTES % bd.bytes_per_block == 0);
      tp->w = GLXX_CONFIG_TEXBUFFER_ARR_ELEM_BYTES / bd.bytes_per_block;
      tp->d = gfx_udiv_round_up(desc_base->width, tp->w);
      tp->arr_str = GLXX_CONFIG_TEXBUFFER_ARR_ELEM_BYTES;
   }

#if !V3D_VER_AT_LEAST(4,0,2,0)
   if (image_unit)
   {
      tp->lx_addr = img_base_plane_addr;
      tp->lx_pitch = img_base_plane->pitch;
      tp->lx_slice_pitch = img_base_plane->slice_pitch;
      tp->lx_swizzling = to_glsl_imgunit_swizzling(img_base_plane->lfmt);
   }
#endif

   return true;
}

#if V3D_VER_AT_LEAST(4,0,2,0)

static bool std_bcol_0001_works(bool *reverse_std_bcol, GFX_LFMT_T hw_border_fmt)
{
   GFX_LFMT_CHANNELS_T channels = gfx_lfmt_get_channels(&hw_border_fmt);
   uint32_t alpha_channel = gfx_lfmt_has_alpha((GFX_LFMT_T)channels) ?
      (1u << gfx_lfmt_alpha_index((GFX_LFMT_T)channels)) : 0;
   uint32_t non_alpha_channels = gfx_lfmt_valid_chan_mask((GFX_LFMT_T)channels) & ~alpha_channel;

   if (!(alpha_channel & 0x7) && !(non_alpha_channels & 0x8))
   {
      *reverse_std_bcol = false;
      return true;
   }

   if (!(alpha_channel & 0xe) && !(non_alpha_channels & 0x1))
   {
      *reverse_std_bcol = true;
      return true;
   }

   *reverse_std_bcol = false;
   return false;
}

/* Returns true iff state extension needed */
static bool pack_tex_state(
   uint8_t hw_tex_state[V3D_TMU_TEX_STATE_PACKED_SIZE + V3D_TMU_TEX_EXTENSION_PACKED_SIZE],
   bool *std_bcol_0001_ok,
   const GLXX_TEXTURE_T *texture,
   const struct hw_tex_params *tp, uint32_t num_levels,
   GFX_LFMT_T hw_border_fmt)
{
   V3D_TMU_TEX_STATE_T s;
   s.flipx = false;
   s.flipy = tp->yflip;
   s.srgb = tp->tmu_trans.srgb;
   s.ahdr = false;
   *std_bcol_0001_ok = std_bcol_0001_works(&s.reverse_std_bcol, hw_border_fmt);
   s.l0_addr = tp->l0_addr;
   s.arr_str = tp->arr_str;
   s.width = tp->w;
   s.height = tp->h;
   s.depth = tp->d;
   s.type = tp->tmu_trans.type;
   s.extended = tp->uif_cfg.force;
   memcpy(s.swizzles, tp->tmu_trans.swizzles, sizeof(s.swizzles));
   s.max_level = tp->base_level + num_levels - 1;
   s.base_level = tp->base_level;
   v3d_pack_tmu_tex_state(hw_tex_state, &s);

   if (s.extended)
   {
      V3D_TMU_TEX_EXTENSION_T e;
      e.ub_pad = tp->uif_cfg.ub_pads[0];
      e.ub_xor = tp->uif_cfg.ub_xor;
      assert(!tp->uif_cfg.ub_noutile);
      e.uif_top = tp->uif_cfg.force;
      e.xor_dis = tp->uif_cfg.xor_dis;
      v3d_pack_tmu_tex_extension(hw_tex_state + V3D_TMU_TEX_STATE_PACKED_SIZE, &e);
   }

   return s.extended;
}

static void clamp_bcols(uint32_t bcol_out[4], const uint32_t bcol_in[4], GFX_LFMT_T tex_fmt)
{
   GFX_LFMT_TYPE_T type = gfx_lfmt_get_type(&tex_fmt);

   switch(type) {
      case GFX_LFMT_TYPE_UFLOAT:
      case GFX_LFMT_TYPE_FLOAT:
         for (int i=0; i<4; i++) bcol_out[i] = bcol_in[i];
         break;
      case GFX_LFMT_TYPE_UINT:
      case GFX_LFMT_TYPE_INT:
      {
         int chan_bits[4] = { 0, };
         if (gfx_lfmt_has_stencil(tex_fmt)) chan_bits[0] = gfx_lfmt_stencil_bits(tex_fmt);
         else {
            chan_bits[0] = gfx_lfmt_red_bits(tex_fmt);
            chan_bits[1] = gfx_lfmt_green_bits(tex_fmt);
            chan_bits[2] = gfx_lfmt_blue_bits(tex_fmt);
            chan_bits[3] = gfx_lfmt_alpha_bits(tex_fmt);
         }
         for (int i=0; i<4; i++) {
            if (chan_bits[i] == 0) continue;

            if (type == GFX_LFMT_TYPE_UINT)
               bcol_out[i] = gfx_uclamp_to_bits(bcol_in[i], chan_bits[i]);
            else
               bcol_out[i] = gfx_sclamp_to_bits(bcol_in[i], chan_bits[i]);
         }
         break;
      }
      case GFX_LFMT_TYPE_SNORM:
         for (int i=0; i<4; i++)
            bcol_out[i] = gfx_float_to_bits(gfx_fclamp(gfx_float_from_bits(bcol_in[i]), -1, 1));
         break;
      case GFX_LFMT_TYPE_UNORM:
      case GFX_LFMT_TYPE_SRGB:
      case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
      case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
         for (int i=0; i<4; i++)
            bcol_out[i] = gfx_float_to_bits(gfx_fclamp(gfx_float_from_bits(bcol_in[i]), 0, 1));
         break;
      default: unreachable();
   }
}

static bool bcol_eq(const uint32_t bcol[4], uint32_t c, uint32_t a)
{
   return (bcol[0] == c) && (bcol[1] == c) && (bcol[2] == c) && (bcol[3] == a);
}

/* Returns true iff sampler extension (for non-standard border colour) needed */
static bool pack_sampler(
   uint8_t hw_sampler[V3D_TMU_SAMPLER_PACKED_SIZE + 16],
   const GLXX_TEXTURE_SAMPLER_STATE_T *sampler,
   GFX_LFMT_T tex_fmt,
   GFX_LFMT_T hw_border_fmt, bool std_bcol_0001_ok)
{
   V3D_TMU_SAMPLER_T s;
   s.filter = get_tmu_filter(
      sampler->filter.mag, sampler->filter.min,
      sampler->anisotropy, sampler->unnormalised_coords);
   s.compare_func = glxx_hw_convert_test_function(sampler->compare_func);
   s.srgb_override = sampler->skip_srgb_decode;
   s.max_lod = gfx_umin(gfx_float_to_uint32(sampler->max_lod * 256.0f), (V3D_MAX_MIP_COUNT - 1) << 8);
   s.min_lod = gfx_umin(gfx_float_to_uint32(sampler->min_lod * 256.0f), s.max_lod);
   s.fixed_bias = 0;
   s.wrap_s = get_hw_wrap(sampler->wrap.s);
   s.wrap_t = get_hw_wrap(sampler->wrap.t);
   s.wrap_r = get_hw_wrap(sampler->wrap.r);
   s.wrap_i = V3D_TMU_WRAP_I_CLAMP; /* All lookup types which use a sampler want clamping for array index */
   if (uses_border_color(sampler))
   {
      uint32_t bcol[4];
      uint32_t one = gfx_lfmt_contains_int(hw_border_fmt) ? 1 : gfx_float_to_bits(1.0f);
      clamp_bcols(bcol, sampler->border_color, tex_fmt);

      if (bcol_eq(bcol, 0, 0))
         s.std_bcol = V3D_TMU_STD_BCOL_0000;
      else if (std_bcol_0001_ok && bcol_eq(bcol, 0, one))
         s.std_bcol = V3D_TMU_STD_BCOL_0001;
      else if (bcol_eq(bcol, one, one))
         s.std_bcol = V3D_TMU_STD_BCOL_1111;
      else
      {
         s.std_bcol = V3D_TMU_STD_BCOL_NON_STD;
         get_hw_border(hw_sampler + V3D_TMU_SAMPLER_PACKED_SIZE, bcol, hw_border_fmt);
      }
   }
   else
      s.std_bcol = V3D_TMU_STD_BCOL_0000;
   v3d_pack_tmu_sampler(hw_sampler, &s);
   return s.std_bcol == V3D_TMU_STD_BCOL_NON_STD;
}

#else

/* This function calculates min_lod & max_lod, as passed to the HW for
 * non-fetch lookups. There is one case where we also adjust the filtering:
 * the case when base_level=max_level, choose a filter similar with the input
 * filter but without mipmaps */
static void get_min_max_lod_adjust_filter(
   uint32_t *min_lod, uint32_t *max_lod,
   GLenum *min_filter, float *anisotropy,
   const GLXX_TEXTURE_SAMPLER_STATE_T *sampler,
   const glxx_calc_image_unit *image_unit,
   unsigned base_level, unsigned num_levels)
{
   if (image_unit)
   {
      *max_lod = *min_lod = gfx_float_to_uint32(base_level * 256.0f);
      return;
   }

   *max_lod = gfx_umin(gfx_float_to_uint32((base_level + sampler->max_lod) * 256.0f), (V3D_MAX_MIP_COUNT - 1) << 8);
   *min_lod = gfx_umin(gfx_float_to_uint32((base_level + sampler->min_lod) * 256.0f), *max_lod);

   unsigned max_level = base_level + num_levels - 1;
   if (max_level == base_level)
   {
      /* see GFXS-732 : adjust filtering so that we remove use of mipmaps */
      *min_filter = filter_disable_mipmapping(*min_filter);
      *anisotropy = 0.0f;
   }
   else
   {
      *min_lod = gfx_umin(*min_lod, max_level << 8);
      *max_lod = gfx_umin(*max_lod, max_level << 8);
   }
}

static bool pack_tmu_config(GLXX_TEXTURE_UNIF_T *texture_unif, KHRN_FMEM_T *fmem,
   const GLXX_TEXTURE_T *texture, const glxx_calc_image_unit *image_unit,
   const struct hw_tex_params *tp, unsigned num_levels,
   const GLXX_TEXTURE_SAMPLER_STATE_T *sampler, GFX_LFMT_T hw_border_fmt,
   bool is_32bit)
{
   bool is_cube_map = texture->target == GL_TEXTURE_CUBE_MAP;
   bool unnormalised_coords = sampler->unnormalised_coords && !is_cube_map;
#if !V3D_VER_AT_LEAST(3,3,1,0)
   // 3.2 and early 3.3 h/w needs to work around GFXH-1371
   // Force pix_mask off for ASTC textures
   bool is_astc = tp->tmu_trans.type >= V3D_TMU_TYPE_C_ASTC_4X4 &&
                  tp->tmu_trans.type <= V3D_TMU_TYPE_C_ASTC_12X12;
   texture_unif->force_no_pixmask = is_astc;
#endif

   {
      V3D_TMU_PARAM0_CFG1_T p0;
      memset(&p0, 0, sizeof(p0));
      /* ltype, fetch, gather, bias, bslod, shadow in extra_bits from shader compiler */
      p0.coefficient = false;
      p0.wrap_s = get_hw_wrap(sampler->wrap.s);
      p0.wrap_t = glxx_tex_target_is_1d(texture->target) ? V3D_TMU_WRAP_CLAMP : get_hw_wrap(sampler->wrap.t);
      p0.wrap_r = get_hw_wrap(sampler->wrap.r);

      /* tex_off_s/t/r, pix_mask in extra_bits from shader compiler */
      texture_unif->hw_param[0] = v3d_pack_tmu_param0_cfg1(&p0);
   }

   uint32_t min_lod, max_lod;
   v3d_tmu_filter_t filter;
   {
      GLenum min_filter = sampler->filter.min;
      float anisotropy = sampler->anisotropy;

      get_min_max_lod_adjust_filter(
         &min_lod, &max_lod,
         &min_filter, &anisotropy,
         sampler, image_unit, tp->base_level, num_levels);

      filter = get_tmu_filter(
         sampler->filter.mag, min_filter, anisotropy,
         unnormalised_coords);

#if !V3D_VER_AT_LEAST(3,3,1,0)
      // Workaround GFXH-1371 - ASTC cube-maps must do nearest filtering
      if (is_astc && is_cube_map)
         filter = V3D_TMU_FILTER_MIN_NEAR_MAG_NEAR;
#endif
   }

   V3D_TMU_INDIRECT_T ind;
   ind.filter = filter;
   ind.border_rrra = false;
   ind.base    = tp->l0_addr;
   ind.arr_str = tp->arr_str;
   ind.width   = tp->w;
   ind.height  = tp->h;
   ind.depth   = tp->d;
   ind.ttype   = tp->tmu_trans.type;
   ind.srgb    = tp->tmu_trans.srgb && !sampler->skip_srgb_decode;
   ind.ahdr    = false;
   ind.compare_func = glxx_hw_convert_test_function(sampler->compare_func);
   memcpy(ind.swizzles, tp->tmu_trans.swizzles, sizeof(ind.swizzles));
   ind.flipx     = false;
   ind.flipy     = tp->yflip;
   ind.etcflip   = true;
   ind.bcolour   = 0;
   if (uses_border_color(sampler))
   {
      uint8_t bcolour[16];
      get_hw_border(bcolour, sampler->border_color, hw_border_fmt);
      memcpy(&ind.bcolour, bcolour, 8);
   }
   ind.u.not_child_image.min_lod = min_lod;
   ind.u.not_child_image.max_lod = max_lod;
   ind.u.not_child_image.fixed_bias = 0;
   ind.u.not_child_image.base_level = tp->base_level;
   ind.u.not_child_image.samp_num = 0;
#if V3D_VER_AT_LEAST(3,3,0,0)
   ind.u.not_child_image.output_type = is_32bit ? V3D_TMU_OUTPUT_TYPE_32 : V3D_TMU_OUTPUT_TYPE_16;
#else
   ind.u.not_child_image.output_type = V3D_TMU_OUTPUT_TYPE_AUTO;
#endif
   ind.ub_pad = tp->uif_cfg.ub_pads[0];
   ind.ub_xor = tp->uif_cfg.ub_xor;
   assert(!tp->uif_cfg.ub_noutile);
   ind.uif_top = tp->uif_cfg.force;
   ind.xor_dis = tp->uif_cfg.xor_dis;

   uint32_t hw_indirect[V3D_TMU_INDIRECT_PACKED_SIZE / 4];
   v3d_pack_tmu_indirect_not_child_image(hw_indirect, &ind);

   V3D_TMU_PARAM1_CFG1_T p1;
   memset(&p1, 0, sizeof(p1));
   /* word0/1/2/3_en in extra_bits from shader compiler */
   p1.unnorm = unnormalised_coords;
   p1.ts_base = khrn_fmem_add_tmu_indirect(fmem, hw_indirect);
   if (!p1.ts_base)
      return false;
   texture_unif->hw_param[1] = v3d_pack_tmu_param1_cfg1(&p1);

   v3d_tmu_swizzle_t swizzle_0 = ind.swizzles[0];
   ind.filter = (num_levels == 1) ? V3D_TMU_FILTER_MIN_LIN_MAG_LIN : V3D_TMU_FILTER_MIN_LIN_MIP_NEAR_MAG_LIN;
   for (int i=0; i<4; i++) {
      ind.swizzles[0] = ind.swizzles[i];
      v3d_pack_tmu_indirect_not_child_image(hw_indirect, &ind);
      p1.ts_base = khrn_fmem_add_tmu_indirect(fmem, hw_indirect);
      if (!p1.ts_base)
         return false;
      texture_unif->hw_param1_gather[i] = v3d_pack_tmu_param1_cfg1(&p1);
   }

   ind.swizzles[0] = swizzle_0;
   // See GFXH-1363: must not use min/max LoD from sampler for fetch lookups...
   ind.u.not_child_image.min_lod = tp->base_level << 8;
   ind.u.not_child_image.max_lod = (tp->base_level + num_levels - 1) << 8;
   v3d_pack_tmu_indirect_not_child_image(hw_indirect, &ind);
   p1.unnorm = false; /* Must not set both fetch & unnorm! */
   p1.ts_base = khrn_fmem_add_tmu_indirect(fmem, hw_indirect);
   if (!p1.ts_base)
      return false;
   texture_unif->hw_param1_fetch = v3d_pack_tmu_param1_cfg1(&p1);

   return true;
}
#endif

enum glxx_tex_completeness
glxx_texture_key_and_uniforms(GLXX_TEXTURE_T *texture, const glxx_calc_image_unit *image_unit,
      const GLXX_TEXTURE_SAMPLER_STATE_T *sampler, bool used_in_bin, bool is_32bit,
      glxx_render_state *rs, GLXX_TEXTURE_UNIF_T *texture_unif,
#if !V3D_VER_AT_LEAST(3,3,0,0)
      glsl_gadgettype_t *gadgettype,
#endif
      glxx_context_fences *fences)
{
   if (texture->target == GL_TEXTURE_BUFFER)
   {
      if (!texture_buffer_possible_complete(texture))
         return INCOMPLETE;
      if (!texture_buffer_create_images(texture))
         return OUT_OF_MEMORY;
   }

   KHRN_IMAGE_T *img_base = NULL;
   uint32_t num_levels;
   enum glxx_tex_completeness complete = get_base_level_image_and_num_levels(
      &img_base, &num_levels, texture, image_unit, sampler, fences);
   if (complete != COMPLETE)
      return complete;

   struct hw_tex_params tp;
   if (!record_tex_usage_and_get_hw_params(&tp, rs, texture, image_unit, img_base, num_levels, used_in_bin))
   {
      KHRN_MEM_ASSIGN(img_base, NULL);
      return OUT_OF_MEMORY;
   }

   /* for textureSize/ imageSize */
   unsigned scale = glxx_ms_mode_get_scale(texture->ms_mode);
   texture_unif->width = khrn_image_get_width(img_base)/scale;
   texture_unif->height = khrn_image_get_height(img_base)/scale;
   if (texture->target == GL_TEXTURE_CUBE_MAP_ARRAY)
      texture_unif->depth = khrn_image_get_depth(img_base) * (khrn_image_get_num_elems(img_base)/6);
   else
      texture_unif->depth = khrn_image_get_depth(img_base) * khrn_image_get_num_elems(img_base);

#if !V3D_VER_AT_LEAST(4,0,2,0)
   texture_unif->base_level = tp.base_level;

   texture_unif->arr_stride = tp.arr_str;
   texture_unif->lx_addr = tp.lx_addr;
   texture_unif->lx_pitch = tp.lx_pitch;
   texture_unif->lx_slice_pitch = tp.lx_slice_pitch;
   texture_unif->lx_swizzling = tp.lx_swizzling;
#endif
   if (texture->target == GL_TEXTURE_BUFFER)
   {
      texture_unif->texbuffer_log2_arr_elem_w = gfx_log2(tp.w);
      texture_unif->texbuffer_arr_elem_w_minus_1 = tp.w - 1;
   }
   KHRN_MEM_ASSIGN(img_base, NULL);

#if V3D_VER_AT_LEAST(3,3,0,0)
   GFX_LFMT_T hw_border_fmt = get_hw_border_fmt(&tp.tmu_trans, is_32bit);
#else
   GFX_LFMT_T hw_border_fmt = get_hw_border_fmt(&tp.tmu_trans,
      v3d_tmu_auto_output_32(tp.tmu_trans.type, /*shadow=*/false, /*coefficient=*/false));
#endif

   compose_swizzles(tp.tmu_trans.swizzles, texture->swizzle);

#if V3D_VER_AT_LEAST(4,0,2,0)
   uint8_t hw_tex_state[V3D_TMU_TEX_STATE_PACKED_SIZE + V3D_TMU_TEX_EXTENSION_PACKED_SIZE];
   bool std_bcol_0001_ok;
   bool tex_state_extended = pack_tex_state(hw_tex_state, &std_bcol_0001_ok,
         texture, &tp, num_levels, hw_border_fmt);
   /* Everything else in param 0 comes from the shader */
   texture_unif->hw_param[0] = khrn_fmem_add_tmu_tex_state(&rs->fmem, hw_tex_state, tex_state_extended);
   if (!texture_unif->hw_param[0])
      return OUT_OF_MEMORY;

   if (image_unit)
      texture_unif->hw_param[1] = 0; // hw_param1 gets ignored in this case
   else
   {
      uint8_t hw_sampler[V3D_TMU_SAMPLER_PACKED_SIZE + 16];
      bool sampler_extended = pack_sampler(hw_sampler, sampler, tp.tex_lfmt, hw_border_fmt, std_bcol_0001_ok);
      V3D_TMU_PARAM1_T p1 = {
         /* Everything else in param 1 comes from the shader */
         .unnorm = sampler->unnormalised_coords,
         .sampler_addr = khrn_fmem_add_tmu_sampler(&rs->fmem, hw_sampler, sampler_extended)};
      if (!p1.sampler_addr)
         return OUT_OF_MEMORY;
      texture_unif->hw_param[1] = v3d_pack_tmu_param1(&p1);
   }
#else
# if !V3D_VER_AT_LEAST(3,3,0,0)
   *gadgettype = get_glsl_gadgettype_and_adjust_hw_swizzles(&tp.tmu_trans, is_32bit);
# endif

   if (!pack_tmu_config(texture_unif, &rs->fmem,
      texture, image_unit, &tp, num_levels, sampler, hw_border_fmt, is_32bit))
      return OUT_OF_MEMORY;
#endif

   return COMPLETE;
}

bool glxx_texture_es1_has_color_alpha(const GLXX_TEXTURE_T *texture, bool
      *has_color, bool *has_alpha)
{
   KHRN_IMAGE_T *img;
   GFX_LFMT_T lfmt;

   assert(glxx_tex_target_valid_in_es1(texture->target));

   img = texture->img[0][0];
   if (!img)
      return false;
   lfmt = khrn_image_get_lfmt(img, 0);
   *has_color = gfx_lfmt_has_color(lfmt);
   *has_alpha = gfx_lfmt_has_alpha(lfmt);
   return true;
}

static bool upload_data(KHRN_IMAGE_T *img,
      unsigned dst_x, unsigned dst_y, unsigned dst_z,
      unsigned start_elem, unsigned num_array_elems,
      const GFX_BUFFER_DESC_T *src_desc, size_t src_offset, size_t src_array_pitch,
      GLXX_BUFFER_T *pixel_buffer, const void *pixels,
      glxx_context_fences *fences,
      GLenum *error)
{
   struct v3d_imgconv_ptr_tgt src_tgt;
   void *src_ptr;
   bool ok;
   size_t buf_offset = 0;

   *error = GL_NO_ERROR;

   if (pixel_buffer == NULL && pixels == NULL)
   {
      /* we have no data to copy */
      return true;
   }
   assert(src_desc->num_planes == 1);
   assert(gfx_lfmt_is_rso(src_desc->planes[0].lfmt));

   if (pixel_buffer)
   {

      if (!glxx_check_buffer_valid_access(pixel_buffer, src_desc,
               (uintptr_t)pixels, src_offset, num_array_elems, src_array_pitch))
      {
         *error = GL_INVALID_OPERATION;
         return false;
      }

      buf_offset = src_offset + (uintptr_t)pixels;
      src_ptr = glxx_buffer_map(pixel_buffer, buf_offset, GL_MAP_READ_BIT);
      if (!src_ptr)
      {
         *error = GL_OUT_OF_MEMORY;
         return false;
      }
   }
   else
      src_ptr = ((uint8_t*)pixels) + src_offset;

   v3d_imgconv_init_ptr_tgt(&src_tgt, src_ptr, src_desc, 0, 0, 0, 0, src_array_pitch);

   /* copy pixels to this img */
   ok = khrn_image_convert_from_ptr_tgt(img, dst_x, dst_y, dst_z, start_elem,
      &src_tgt, src_desc->width, src_desc->height, src_desc->depth,
      num_array_elems, fences, in_secure_context());

   if (pixel_buffer)
      glxx_buffer_unmap(pixel_buffer, buf_offset, GL_MAP_READ_BIT);

   if (!ok)
   {
      *error = GL_OUT_OF_MEMORY;
      return false;
   }
   return true;
}

bool glxx_texture_subimage(GLXX_TEXTURE_T *texture, unsigned face, unsigned level,
      unsigned dst_x, unsigned dst_y, unsigned dst_z,
      GLenum fmt, GLenum type, const GLXX_PIXEL_STORE_STATE_T *pack_unpack,
      GLXX_BUFFER_T *pixel_buffer, const void *pixels,
      unsigned width, unsigned height, unsigned depth,
      glxx_context_fences *fences, GLenum *error)
{
   KHRN_IMAGE_T *img;
   GFX_BUFFER_DESC_T src_desc;
   unsigned dim, num_array_elems, start_elem;
   bool ok;
   GFX_LFMT_T src_lfmt, dst_fmt;
   bool is_srgb;
   struct glxx_pixels_info src_info;

   *error = GL_NO_ERROR;

   img = texture->img[face][level];
   assert(img);

   dim = get_target_num_dimensions(texture->target);
   glxx_tex_transform_dim_for_target(texture->target, &width, &height, &depth,
         &num_array_elems);

   glxx_tex_transform_offsets_for_target(texture->target, &dst_x, &dst_y,
         &dst_z, &start_elem);

   glxx_get_pack_unpack_info(pack_unpack, false, width, height, fmt, type,
         &src_info);

   /* get the srgb from the dst image */
   dst_fmt = gfx_lfmt_fmt(khrn_image_get_lfmt(img, 0));
   is_srgb = gfx_lfmt_contains_srgb(dst_fmt);

   src_lfmt = gfx_lfmt_from_format_type(fmt, type, is_srgb);
   gfx_lfmt_set_dims(&src_lfmt, gfx_lfmt_dims_to_enum(dim));
   src_lfmt = gfx_lfmt_to_rso(src_lfmt);

   src_desc.width = width;
   src_desc.height = height;
   src_desc.depth = depth;
   src_desc.num_planes = 1;
   src_desc.planes[0].lfmt = src_lfmt;
   src_desc.planes[0].offset = 0;
   src_desc.planes[0].pitch = src_info.stride;
   src_desc.planes[0].slice_pitch = depth > 1 ? src_info.slice_pitch : 0;

   ok = upload_data(img, dst_x, dst_y, dst_z, start_elem, num_array_elems,
         &src_desc, src_info.offset, src_info.slice_pitch, pixel_buffer, pixels,
         fences, error);
   return ok;
}

static void get_compressed_src_desc(GFX_LFMT_T src_lfmt, unsigned width,
      unsigned height, unsigned depth, unsigned num_array_elems,
      GFX_BUFFER_DESC_T *src_desc, unsigned *src_array_pitch)
{
   GFX_LFMT_BASE_DETAIL_T bd;
   unsigned  pitch, slice_pitch;

   gfx_lfmt_base_detail(&bd, src_lfmt);

   pitch = gfx_udiv_round_up(width, bd.block_w) * bd.bytes_per_block;
   slice_pitch = gfx_udiv_round_up(height, bd.block_h) * pitch;

   src_desc->width = width;
   src_desc->height = height;
   src_desc->depth = depth;
   src_desc->num_planes = 1;
   src_desc->planes[0].lfmt = src_lfmt;
   src_desc->planes[0].offset = 0;
   src_desc->planes[0].pitch = pitch;
   src_desc->planes[0].slice_pitch = depth > 1 ? slice_pitch : 0;

   *src_array_pitch = slice_pitch;
}

bool glxx_texture_compressed_image(GLXX_TEXTURE_T *texture,
      unsigned face, unsigned level, GLenum internalformat,
      unsigned width, unsigned height, unsigned depth,
      unsigned image_size, GLXX_BUFFER_T *pixel_buffer,
      const void *pixels, glxx_context_fences *fences,
      GLenum *error)
{
   unsigned num_array_elems, src_array_pitch, size_plane;
   GFX_BUFFER_DESC_T src_desc;
   bool ok;

   *error = GL_NO_ERROR;

   assert(face < MAX_FACES && level < KHRN_MAX_MIP_LEVELS);
   assert(texture->immutable_format == GFX_LFMT_NONE);

   if (!texture_unbind(texture, fences))
   {
      *error = GL_OUT_OF_MEMORY;
      return false;
   }

   if (width == 0 || height == 0 || depth == 0)
      return true;

   glxx_tex_transform_dim_for_target(texture->target, &width, &height, &depth,
         &num_array_elems);

   GFX_LFMT_T api_fmt = gfx_lfmt_from_compressed_internalformat(
      khrn_get_lfmt_translate_exts(), internalformat);
   assert(api_fmt != GFX_LFMT_NONE);

   GFX_LFMT_T src_lfmt = gfx_lfmt_to_rso(api_fmt);
   glxx_lfmt_add_dim(&src_lfmt, 1, get_target_num_dimensions(texture->target));
   get_compressed_src_desc(src_lfmt, width, height, depth, num_array_elems,
         &src_desc, &src_array_pitch);

   /* check that image_size is consistent with format, dimensions and contents
    * of the compressed image */
   size_plane = gfx_buffer_size_plane(&src_desc, 0);
   if (image_size != size_plane * num_array_elems)
   {
      *error = GL_INVALID_VALUE;
      return false;
   }

   /* Internal format always == API format for compressed formats */
   ok = texture_check_or_create_image(texture, face, level, width, height,
         depth, num_array_elems, &api_fmt, 1, api_fmt, fences,
         false);

   if (!ok)
   {
      *error = GL_OUT_OF_MEMORY;
      return false;
   }

   ok = upload_data(texture->img[face][level],
         0, 0, 0, 0, num_array_elems,
         &src_desc, 0, src_array_pitch, pixel_buffer, pixels,
         fences, error);
   if (!ok)
      return false;

   ok = es1_update_mipmaps(texture, level, fences);
   if (!ok) *error = GL_OUT_OF_MEMORY;
   return ok;
}

bool glxx_texture_compressed_subimage(GLXX_TEXTURE_T *texture, unsigned face,
      unsigned level, unsigned dst_x, unsigned dst_y, unsigned dst_z,
      unsigned width, unsigned height, unsigned depth,
      GLenum fmt, unsigned image_size, GLXX_BUFFER_T *pixel_buffer, const
      void *pixels, glxx_context_fences *fences, GLenum *error)
{
   KHRN_IMAGE_T *img;
   GFX_LFMT_T src_lfmt;
   unsigned dim, num_array_elems, start_elem;
   unsigned src_array_pitch, size_plane;
   GFX_BUFFER_DESC_T src_desc;
   bool ok;

   *error = GL_NO_ERROR;

   img = texture->img[face][level];
   assert(img);

   glxx_tex_transform_dim_for_target(texture->target, &width, &height, &depth,
         &num_array_elems);
   glxx_tex_transform_offsets_for_target(texture->target, &dst_x, &dst_y, &dst_z,
         &start_elem);

   dim = get_target_num_dimensions(texture->target);
   src_lfmt = gfx_lfmt_fmt(khrn_image_get_lfmt(img, 0));
   glxx_lfmt_add_dim(&src_lfmt, 1, dim);
   src_lfmt = gfx_lfmt_to_rso(src_lfmt);

   get_compressed_src_desc(src_lfmt, width, height, depth, num_array_elems,
         &src_desc, &src_array_pitch);

   /* check that image_size is consistent with format, dimensions and contents
    * of the compressed image */
   size_plane = gfx_buffer_size_plane(&src_desc, 0);
   if (image_size != size_plane * num_array_elems)
   {
      *error = GL_INVALID_VALUE;
      return false;
   }

   ok = upload_data(texture->img[face][level], dst_x, dst_y, dst_z, start_elem,
         num_array_elems, &src_desc, 0, src_array_pitch, pixel_buffer,
         pixels, fences, error);
   if (!ok)
      return false;

   ok = es1_update_mipmaps(texture, level, fences);
   if (!ok) *error = GL_OUT_OF_MEMORY;
   return ok;
}

static bool is_luminance_or_alpha(GFX_LFMT_T lfmt)
{
   bool res;
   switch (lfmt & GFX_LFMT_FORMAT_MASK & ~GFX_LFMT_PRE_MASK)
   {
      case GFX_LFMT_L1_UNORM:
      case GFX_LFMT_L4_UNORM:
      case GFX_LFMT_L8_UNORM:
      case GFX_LFMT_L8_A8_UNORM:
      case GFX_LFMT_A8_UNORM:
         res = true;
         break;
      default:
         res = false;
   }
   return res;
}

bool check_or_create_images(GLXX_TEXTURE_T *texture,
      unsigned based_on_level, unsigned base_level, unsigned num_levels,
      glxx_context_fences *fences)
{
   unsigned width, height, depth, num_planes, num_elems;
   GFX_LFMT_T fmts[GFX_BUFFER_MAX_PLANES];
   GFX_LFMT_T api_fmt;
   assert(base_level >= based_on_level);

   {
      /* img0 should is not be visible outside this scope, because
       * texture->check_or_create_image might orphan the texture and as a
       * result img0 would point to released image */
      KHRN_IMAGE_T* img0 = texture->img[0][based_on_level];
      khrn_image_get_dimensions(img0, &width, &height, &depth, &num_elems);
      khrn_image_get_fmts(img0, fmts, &num_planes);
      api_fmt = img0->api_fmt;
   }

   unsigned faces = num_faces(texture->target);
   unsigned offset = base_level - based_on_level;
   for (unsigned face = 0; face < faces; face ++)
   {
      for (unsigned level = 0; level < num_levels; level++)
      {
         unsigned w_l, h_l, d_l;
         w_l = gfx_umax(width >> (level + offset), 1);
         h_l = gfx_umax(height >> (level + offset), 1);
         d_l = gfx_umax(depth >> (level + offset), 1);
         if (!texture_check_or_create_image(texture, face,
               base_level + level,
               w_l, h_l, d_l, num_elems,
               fmts, num_planes, api_fmt, fences,
               false))
         {
            return false;
         }
      }
   }
   return true;
}

bool glxx_texture_generate_mipmap(GLXX_TEXTURE_T *texture,
      glxx_context_fences *fences, GLenum *error)
{
   *error = GL_NO_ERROR;

   if (texture->target == GL_TEXTURE_CUBE_MAP &&
         !glxx_texture_is_cube_complete(texture))
   {
      *error = GL_INVALID_OPERATION;
      return false;
   }

   unsigned base_level;
   unsigned num_levels = 0;
   if (!try_get_num_levels(texture, false, &base_level, &num_levels))
   {
      *error = GL_INVALID_OPERATION;
      return false;
   }
   KHRN_IMAGE_T *img_base = texture->img[0][base_level];
   GFX_LFMT_T lfmt = khrn_image_get_lfmt(texture->img[0][base_level], 0);

   /* if lfmt is not luminance, alpha or luminance alpha the format must be
    * sized internal format,  color renderable and texture filterable */
   if (!is_luminance_or_alpha(lfmt))
   {
      if (gfx_sized_internalformat_from_api_fmt_maybe(img_base->api_fmt) == GL_NONE ||
          !glxx_is_color_renderable_from_api_fmt(img_base->api_fmt) ||
          !glxx_is_texture_filterable_api_fmt(img_base->api_fmt))
      {
         *error = GL_INVALID_OPERATION;
         return false;
      }
   }

   /* nothing to do */
   if (num_levels == 1)
      return true;

   unsigned faces = num_faces(texture->target);

   uint32_t id = khrn_driver_track_next_id(KHRN_DRIVER_TRACK_DRIVER);
   khrn_driver_add_event(KHRN_DRIVER_TRACK_DRIVER, id, KHRN_DRIVER_EVENT_GENERATE_MIPMAPS, BCM_EVENT_BEGIN);

   /* generate_mipmap replaces texels arrays levels base_level+1 through q (base_level+num_levels-1) */;

   /* immutable textures always have images for all the required levels */
   KHRN_IMAGE_T *new_img[MAX_FACES][KHRN_MAX_MIP_LEVELS];
   bool is_new = false;

   if (texture->immutable_format == GFX_LFMT_NONE)
   {
      KHRN_BLOB_T *main_blob;
      unsigned at_level;

      texture_get_main_blob(texture, &main_blob, &at_level);
      if (!main_blob ||
          (main_blob->num_mip_levels == 1 && (at_level == base_level)))
      {
         /* we don't have a main blob, or we have a cube with one mip level and
          * the blob has as level 0 our base_level */

         /* orphan image if bound */
         if (!texture_unbind(texture, fences))
            goto fail;
         if (!create_blob_and_images(texture->target, texture->img[0][base_level],
                  faces, base_level, num_levels, new_img))
            goto fail;
         is_new = true;
      }
      else
      {
         if (!check_or_create_images(texture, base_level, base_level + 1,
               num_levels - 1, fences))
            goto fail;
      }
   }

   KHRN_IMAGE_T* (*p_imgs)[KHRN_MAX_MIP_LEVELS];

   if (!is_new)
      p_imgs = texture->img;
   else
      p_imgs = new_img;

   bool use_tfu = false;
   bool secure_context = in_secure_context();

   if (khrn_get_has_tfu())
   {
      /* see if we can ask the tfu to generate the mipmaps */
      if (texture->target != GL_TEXTURE_3D)
      {
         /* see if images are in same blob */
         if (is_new || are_images_in_same_blob(p_imgs, faces, base_level, num_levels))
         {
            /* base_level needs to correspond to level 0 in the blob */
            KHRN_IMAGE_T *img = p_imgs[0][base_level];
            if (img->level == 0)
               use_tfu = true;
         }
      }
      for (unsigned face = 0; face < faces && use_tfu; face++)
      {
         // TFU can be used in secure mode if destination is also secure
         if (secure_context)
            use_tfu = p_imgs[face][base_level]->blob->secure;
         else
            use_tfu = !p_imgs[face][base_level]->blob->secure &&
                      !texture->img[face][base_level]->blob->secure;
      }
      for (unsigned face = 0; face < faces && use_tfu; face++)
      {
         // If tfu mipmap generation is unsupported (or fails) then we'll break out and
         // fallback to doing the whole lot on the CPU below.
         use_tfu = khrn_image_generate_mipmaps_tfu(
            texture->img[face][base_level],
            &p_imgs[face][base_level],
            num_levels,
            texture->img[face][base_level] == p_imgs[face][base_level],
            texture->sampler.skip_srgb_decode,
            fences, secure_context);
      }
   }

   if (!use_tfu)
   {
      if (is_new)
      {
         /* copy base_level from texture to the new_img */
         if (!copy_images_content(new_img, texture->img, faces, base_level, 1, fences))
            goto fail;
      }

      for (unsigned face = 0; face < faces; face++)
      {
         for (unsigned level = base_level + 1; level < base_level + num_levels; level++)
         {
            bool ok = khrn_image_subsample(p_imgs[face][level], p_imgs[face][level-1],
                                           texture->sampler.skip_srgb_decode, fences);
            if (!ok)
               goto fail;
         }
      }
   }

   if (is_new)
   {
      assign_new_images(texture, new_img, base_level, num_levels);
      release_images(new_img, faces, base_level, num_levels);
   }

   khrn_driver_add_event(KHRN_DRIVER_TRACK_DRIVER, id, KHRN_DRIVER_EVENT_GENERATE_MIPMAPS, BCM_EVENT_END);

   return true;

fail:
   if (is_new)
      release_images(new_img, faces, base_level, num_levels);

   *error = GL_OUT_OF_MEMORY;
   khrn_driver_add_event(KHRN_DRIVER_TRACK_DRIVER, id, KHRN_DRIVER_EVENT_GENERATE_MIPMAPS, BCM_EVENT_END);
   return false;
}

static bool usage_ok(const KHRN_IMAGE_T *image)
{
   const GFX_BUFFER_DESC_T *desc = &image->blob->desc[image->level];
   bool                    ok = true;

   if (image->blob->usage & GFX_BUFFER_USAGE_V3D_TEXTURE)
      return ok;

   for (unsigned int p = 0; p < desc->num_planes; p++)
      ok = ok && gfx_lfmt_is_rso(desc->planes[p].lfmt);

   return ok;
}

static bool texture_bind_images(GLXX_TEXTURE_T *texture, enum glxx_tex_binding binding,
      KHRN_IMAGE_T **images,  unsigned num_images)
{
   unsigned level;

   assert(texture->immutable_format == GFX_LFMT_NONE);

   texture_unbind_and_release_all(texture);

   for (level = 0; level < num_images; level++)
   {
      assert(usage_ok(images[level]));
      KHRN_MEM_ASSIGN(texture->img[0][level], images[level]);
   }

   texture->binding = binding;
   return true;
}

bool glxx_texture_bind_teximage(GLXX_TEXTURE_T *texture, KHRN_IMAGE_T **images,
      unsigned num_images, unsigned mip_level, glxx_context_fences *fences)
{
   bool ok;
   GLenum error;

   if (texture->immutable_format != GFX_LFMT_NONE)
      return false;

   assert(texture->target == GL_TEXTURE_2D && (mip_level < num_images));

   texture_bind_images(texture, TEX_BOUND_TEXIMAGE, images, num_images);

   if (num_images > 1 && texture->generate_mipmap && texture->base_level ==
         mip_level)
   {
      /* this cannot fail because we have all storage for all the levels */
      ok = glxx_texture_generate_mipmap(texture, fences, &error);
      if (!ok)
         goto end;
   }

   return true;
end:
   texture_unbind_and_release_all(texture);
   return false;
}

void glxx_texture_release_teximage(GLXX_TEXTURE_T *texture)
{
   assert(texture->binding == TEX_BOUND_TEXIMAGE);
   texture_unbind_and_release_all(texture);
}

void glxx_texture_add_observer(GLXX_TEXTURE_T *texture, tex_unbind_observer_t
      observer)
{
   assert(texture->binding != TEX_BOUND_NONE);
   assert(texture->observer.callback == NULL);
   texture->observer = observer;
}

void glxx_texture_remove_observer(GLXX_TEXTURE_T *texture)
{
   texture->observer.callback = NULL;
}

/* GL_OES_draw_texture */
void glxx_texture_set_crop_rect(GLXX_TEXTURE_T *texture, const GLint * params)
{
   texture->crop_rect.Ucr = params[0];
   texture->crop_rect.Vcr = params[1];
   texture->crop_rect.Wcr = params[2];
   texture->crop_rect.Hcr = params[3];
}

bool glxx_texture_acquire_one_elem_slice(GLXX_TEXTURE_T *texture,
      unsigned face, unsigned level, unsigned layer, KHRN_IMAGE_T **p_img)
{
   KHRN_IMAGE_T *img = NULL;
   unsigned xoffset, yoffset, zoffset, start_elem, faces;
   bool res = true;

   faces = num_faces(texture->target);
   if (face > faces || !glxx_texture_is_legal_level(texture->target, level))
      goto end;
   img = texture->img[face][level];

   if (!img)
      goto end;

   xoffset = yoffset  = 0;
   zoffset = layer;
   glxx_tex_transform_offsets_for_target(texture->target, &xoffset, &yoffset,
         &zoffset, &start_elem);

   if (khrn_image_is_one_elem_slice(img))
   {
      if (layer != 0 || start_elem != 0)
      {
         img = NULL;
         goto end;
      }

      khrn_mem_acquire(img);
   }
   else
   {
      if (zoffset >= khrn_image_get_depth(img) ||
          start_elem >=  khrn_image_get_num_elems(img))
      {
         img = NULL;
         goto end;
      }
      img = khrn_image_create_one_elem_slice(img->blob, start_elem, zoffset, level, img->api_fmt);
      if (img == NULL)
         res = false;
   }

end:
   *p_img = img;
   return res;
}

bool glxx_texture_acquire_from_eglimage(GLXX_TEXTURE_T *texture, unsigned face,
      unsigned level, unsigned layer, KHRN_IMAGE_T **img)
{
   assert(texture->binding == TEX_BOUND_NONE || texture->binding == TEX_BOUND_EGLIMAGE_SOURCE);

   if (!glxx_texture_acquire_one_elem_slice(texture, face, level, layer, img))
      return false;

   if (*img)
      texture->binding = TEX_BOUND_EGLIMAGE_SOURCE;

   return true;
}

bool glxx_texture_has_images_outside_range(const GLXX_TEXTURE_T *texture, unsigned start_level,
      unsigned num_levels)
{
   unsigned face, level, faces;
   unsigned one_past_end = start_level + num_levels;
   faces = num_faces(texture->target);
   assert(num_levels > 0);

   for (face = 0; face < faces; face++)
   {
      for (level = 0; level < start_level; level++)
      {
         if (texture->img[face][level] != NULL)
            return true;
      }
      for (level = one_past_end; level < KHRN_MAX_MIP_LEVELS; level++)
      {
         if (texture->img[face][level] != NULL)
            return true;
      }
   }
   return false;
}

GL_API void GL_APIENTRY glEGLImageTargetTexture2DOES(GLenum target,
      GLeglImageOES image)
{
   GLXX_SERVER_STATE_T *state;
   EGL_IMAGE_T *egl_image = NULL;
   GLenum error = GL_NO_ERROR;
   GLXX_TEXTURE_T *texture = NULL;
   KHRN_IMAGE_T *khr_image = NULL;
   bool locked = false;

   if (!egl_context_gl_lock())
      goto end;
   locked = true;

   state = egl_context_gl_server_state(NULL);
   if (!state) goto end;

   egl_image = egl_get_image_refinc((EGLImageKHR) image);

   if (egl_image == NULL)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   switch (target)
   {
      case GL_TEXTURE_2D:
      case GL_TEXTURE_EXTERNAL_OES:
         break;
      default:
         error = GL_INVALID_ENUM;
         goto end;
   }

   texture = glxx_server_state_get_texture(state, target);
   if (texture == NULL) goto end;

   if (texture->immutable_format != GFX_LFMT_NONE)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   khr_image = egl_image_get_image(egl_image);
   if (khr_image == NULL)
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }

   texture_bind_images(texture, TEX_BOUND_EGLIMAGE_TARGET,
         &khr_image, 1);

   /*
    * Any existing source should have been detached by
    * glxx_texture_bind_images which did a nullify
    */
   assert(texture->source == NULL);
   /* we've already got a ref count of this egl_image; keep it */
   texture->source = egl_image;

end:
   if (error != GL_NO_ERROR)
   {
      egl_image_refdec(egl_image);
      glxx_server_state_set_error(state, error);
   }

   if (locked)
      egl_context_gl_unlock();
}

bool glxx_texture_copy_image(GLXX_TEXTURE_T *texture, unsigned face,
      unsigned level, GFX_LFMT_T dst_fmt, KHRN_IMAGE_T *src,
      int src_x, int src_y, int width, int height,
      glxx_context_fences *fences)
{
   unsigned depth, num_array_elems;
   bool ok;
   int dst_x, dst_y;

   assert(face < MAX_FACES && level < KHRN_MAX_MIP_LEVELS);
   assert(texture->immutable_format == GFX_LFMT_NONE);
   assert(width >= 0 && height >= 0);

   if (!texture_unbind(texture, fences))
      return false;

   if (width == 0 || height == 0)
   {
      KHRN_MEM_ASSIGN(texture->img[face][level], NULL);
      return true;
   }

   depth = 1;
   glxx_tex_transform_dim_for_target(texture->target, (unsigned*)&width,
         (unsigned*)&height, &depth, &num_array_elems);
   assert(num_array_elems == 1);

   // At this point dst_fmt is set by choose_copy_format(), so it
   // is good as api_fmt as well
   ok = texture_check_or_create_image(texture, face, level, width, height,
            depth, num_array_elems, &dst_fmt, 1, dst_fmt, fences, false);
   if (!ok)
      return false;

   dst_x = 0; dst_y = 0;
   glxx_clamp_rect(khrn_image_get_width(src), khrn_image_get_height(src),
         &src_x, &src_y, &width, &height, &dst_x, &dst_y);

   if (width <= 0 || height <= 0)
      return false;

   ok = khrn_image_convert_one_elem_slice(texture->img[face][level], dst_x, dst_y,
         0, 0, src, (unsigned)src_x,(unsigned) src_y, 0, 0, (unsigned)width,
         (unsigned)height, fences, in_secure_context());
   if (!ok)
      return false;

   return true;
}

bool glxx_texture_copy_sub_image(GLXX_TEXTURE_T *texture, unsigned face,
      unsigned level, int dst_x,int dst_y, int dst_z,
      KHRN_IMAGE_T *src, int src_x, int src_y, int width, int height,
      glxx_context_fences *fences)
{
   unsigned depth, num_array_elems;
   bool ok;

   assert(width >=0 && height >=0);
   assert(dst_x >= 0 && dst_y >= 0 && dst_z >= 0);

   depth = 1;
   glxx_tex_transform_dim_for_target(texture->target, (unsigned*)&width,
         (unsigned*)&height, &depth, &num_array_elems);
   assert(num_array_elems == 1);

   glxx_clamp_rect(khrn_image_get_width(src), khrn_image_get_height(src),
         &src_x, &src_y, &width, &height, &dst_x, &dst_y);

   if (width <= 0 || height <= 0)
      return false;

   ok = khrn_image_convert_one_elem_slice(texture->img[face][level], dst_x, dst_y,
         dst_z, 0, src, (unsigned)src_x,(unsigned) src_y, 0, 0, (unsigned)width,
         (unsigned)height, fences, in_secure_context());
   if (!ok)
      return false;
   return true;
}

bool glxx_texture_get_fixed_sample_locations(const GLXX_TEXTURE_T *texture)
{
   if (!glxx_tex_target_is_multisample(texture->target))
      assert(texture->fixed_sample_locations == true);

   return texture->fixed_sample_locations;
}

size_t glxx_texture_buffer_get_width(const GLXX_TEXTURE_T *texture)
{
   assert(texture->target == GL_TEXTURE_BUFFER);
   const struct tex_buffer_s *tex_buffer = &texture->tex_buffer;

   if (tex_buffer->buffer == NULL)
      return 0;

   size_t size = tex_buffer->size != SIZE_MAX ? tex_buffer->size :
      glxx_buffer_get_size(tex_buffer->buffer);

   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, tex_buffer->api_fmt);
   assert(bd.block_w == 1 && bd.block_h ==1 && bd.block_d == 1);
   size_t width = size / bd.bytes_per_block ;
   return gfx_zmin(width, GLXX_CONFIG_MAX_TEXTURE_BUFFER_SIZE);
}

void glxx_texture_buffer_attach(GLXX_TEXTURE_T *texture, GLXX_BUFFER_T *buffer,
      GLenum internalformat, size_t offset, size_t size)
{
   assert(texture->target == GL_TEXTURE_BUFFER);

   KHRN_MEM_ASSIGN(texture->img[0][0], NULL);
   KHRN_MEM_ASSIGN(texture->tex_buffer.buffer, buffer);

   struct tex_buffer_s *tex_buffer = &texture->tex_buffer;

   if (buffer == NULL)
      memset(tex_buffer, 0, sizeof(struct tex_buffer_s));
   else
   {
      tex_buffer->offset = offset;
      tex_buffer->size = size;
      tex_buffer->api_fmt = gfx_api_fmt_from_sized_internalformat(internalformat);
   }
}

static bool texture_buffer_possible_complete(const GLXX_TEXTURE_T *texture)
{
   assert(texture->target == GL_TEXTURE_BUFFER);

   const struct tex_buffer_s *tex_buffer = &texture->tex_buffer;

   if (tex_buffer->buffer == NULL)
      return false;

   if (glxx_texture_buffer_get_width(texture) == 0)
      return false;

   if (tex_buffer->size == SIZE_MAX)
      return true;

   /* if buffer size has been changed since TexBufferRange was called such that
    * offset + size > new buffer size , report the texture as incomplete
    */
   if (tex_buffer->offset + tex_buffer->size > glxx_buffer_get_size(tex_buffer->buffer))
      return false;

   return true;
}

static bool texture_buffer_create_images(GLXX_TEXTURE_T *texture)
{
   assert(texture->target == GL_TEXTURE_BUFFER);
   assert(texture_buffer_possible_complete(texture));

   const struct tex_buffer_s *tex_buffer = &texture->tex_buffer;

   /* if the buffer storage didn't change, keep this image */
   if (texture->img[0][0] != NULL &&
         texture->img[0][0]->blob->res_i == tex_buffer->buffer->resource)
      return true;

   KHRN_MEM_ASSIGN(texture->img[0][0], NULL);

   GFX_BUFFER_DESC_T desc;
   memset(&desc, 0, sizeof(GFX_BUFFER_DESC_T));
   desc.num_planes = 1;
   desc.planes[0].offset = tex_buffer->offset;
   desc.planes[0].lfmt = tex_buffer->api_fmt;
   gfx_lfmt_set_dims(&desc.planes[0].lfmt, GFX_LFMT_DIMS_1D);
   gfx_lfmt_set_swizzling(&desc.planes[0].lfmt, GFX_LFMT_SWIZZLING_RSO);
   desc.width = glxx_texture_buffer_get_width(texture);
   desc.height = desc.depth = 1;

   assert(desc.width != 0);

   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, desc.planes[0].lfmt);
   assert(bd.block_w == 1 && bd.block_h ==1 && bd.block_d == 1);
   size_t clamped_size = desc.width * bd.bytes_per_block;

   KHRN_BLOB_T *blob = khrn_blob_create_from_res_interlock(tex_buffer->buffer->resource, &desc, 1, 1,
         tex_buffer->offset + clamped_size, GFX_BUFFER_USAGE_V3D_TEXTURE, texture->create_secure);
   if (!blob)
      return false;

   texture->img[0][0] = khrn_image_create(blob, 0, 1, 0, tex_buffer->api_fmt);
   KHRN_MEM_ASSIGN(blob, NULL);

   return (texture->img[0][0] != NULL);
}
