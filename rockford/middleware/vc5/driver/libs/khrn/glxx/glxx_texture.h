/*=============================================================================
  Broadcom Proprietary and Confidential. (c)2013 Broadcom.
  All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES texture structure declaration.
=============================================================================*/
#ifndef GLXX_TEXTURE_H
#define GLXX_TEXTURE_H

#include "libs/core/lfmt/lfmt.h"

#include "../common/khrn_int_util.h"
#include "gl_public_api.h"

#include "../egl/egl_types.h"
#include "../common/khrn_image.h"
#include "../common/khrn_fmem.h"
#include "glxx_pixel_store.h"
#include "glxx_buffer.h"
#include "glxx_texture_defines.h"
#include "glxx_texture_utils.h"
#include "../common/khrn_image.h"

#include "../glsl/glsl_gadgettype.h"
#include "../glsl/glsl_imageunit_swizzling.h"
#include "glxx_utils.h"
#include "glxx_render_state.h"

#define enumify(x) E_##x=x

enum glxx_tex_binding {
   TEX_BOUND_NONE,
   TEX_BOUND_TEXIMAGE,         /* Bound using eglBindTexImage. */
   TEX_BOUND_EGLIMAGE_SOURCE,  /* used as a source in eglCreateImageKHR */
   TEX_BOUND_EGLIMAGE_TARGET   /* used as a target in Target2DOES */
};

enum glxx_tex_target
{
   enumify(GL_TEXTURE_1D_BRCM),
   enumify(GL_TEXTURE_1D_ARRAY_BRCM),
   enumify(GL_TEXTURE_2D),
   enumify(GL_TEXTURE_EXTERNAL_OES),
   enumify(GL_TEXTURE_CUBE_MAP),
   enumify(GL_TEXTURE_3D),
   enumify(GL_TEXTURE_2D_ARRAY),
   enumify(GL_TEXTURE_2D_MULTISAMPLE),
   enumify(GL_TEXTURE_2D_MULTISAMPLE_ARRAY),
   enumify(GL_TEXTURE_CUBE_MAP_ARRAY),
};

enum glxx_min_filter
{
   enumify(GL_NEAREST),
   enumify(GL_LINEAR),
   enumify(GL_NEAREST_MIPMAP_NEAREST),
   enumify(GL_NEAREST_MIPMAP_LINEAR),
   enumify(GL_LINEAR_MIPMAP_NEAREST),
   enumify(GL_LINEAR_MIPMAP_LINEAR)
};

typedef struct glxx_tex_sampler_state {
   unsigned id;
   struct {
      GLenum mag;
      enum glxx_min_filter min;
   } filter;
   /* GL_EXT_texture_filter_anisotropic */
   float anisotropy;
   struct {
      GLenum s;
      GLenum t;
      GLenum r;
   } wrap;
   float min_lod;
   float max_lod;
   GLenum compare_mode;
   GLenum compare_func;
   /* Ignored for cubemap lookups.
    * Otherwise, if set, texture coords will be treated as unnormalised,
    * mipmapping will be disabled, and wrap modes other than
    * GL_CLAMP_TO_BORDER & GL_CLAMP_TO_EDGE will be treated as one of
    * BORDER/CLAMP_TO_EDGE. */
   bool unnormalised_coords;
   uint32_t border_color[4];
   char   *debug_label;
}GLXX_TEXTURE_SAMPLER_STATE_T;

struct EGL_IMAGE_T_;

typedef struct
{
   void (*callback)(void*);
   void *param;
}tex_unbind_observer_t;

typedef enum
{
   enumify(GL_DEPTH_COMPONENT),
   enumify(GL_STENCIL_INDEX)
}glxx_ds_texture_mode;

/* the state necessary for a texture can be devided in two categories:
 * 1:  texture properties
 * 2:  actual texel arrays */
typedef struct glxx_texture
{
   /*
    * First category for the texture state: texture properties
    */
   int32_t name;
   enum glxx_tex_target target;
   struct glxx_tex_sampler_state sampler;

   /* these are set by TexPrameter */
   unsigned swizzle[4];
   unsigned base_level;
   unsigned max_level;
   bool generate_mipmap; /* in es1.1 if this is true, making any changes to
                            texels of level 0 will also compute a complete set
                            of mipmap arrays */

   glxx_ds_texture_mode ds_texture_mode;

   /* these are set when TexStorage2D/TexStorage3D/TexStorage2DMultisample gets
    * called */
   GFX_LFMT_T immutable_format;        /* immutable texture */
   unsigned immutable_levels;

   glxx_ms_mode ms_mode;   /* non multisample or ms texture */
   bool fixed_sample_locations; /* this makes sense only for ms texture */

   /* GL_OES_draw_texture define a crop rectangle */
   struct {
      GLint Ucr;
      GLint Vcr;
      GLint Wcr;
      GLint Hcr;
   } crop_rect;

   /*
    * Second category for the texture: actual texel arrays.
    * There are mutiple sets of texel arrays:
    *    - one set of mipmaps arrays for the two and three-dimensional texture
    *      and two-dimensional arrays texture targets
    *    - and six sets of mipmap arrays for the cube map texture targets
    */
   KHRN_IMAGE_T *img[MAX_FACES][KHRN_MAX_MIP_LEVELS];

   /* helper data */
   enum glxx_tex_binding   binding;
   EGL_IMAGE_T *source;  /* Set to the source image if the texture is an
                            EGLImage target */

   /* this is used to notify an observer that a bound texture is respecified or
    * deleted; see glxx_add_observer/remove_observer */
   tex_unbind_observer_t observer;

   char *debug_label;

   /* Set from glTexParameter with GL_TEXTURE_PROTECTED_EXT
      and controls whether texture is created secure when TexStorage is
      called */
   bool create_secure;

} GLXX_TEXTURE_T;


extern GLXX_TEXTURE_T* glxx_texture_create(GLenum target, int32_t name);

extern bool glxx_texture_image(GLXX_TEXTURE_T *texture, unsigned face, unsigned level,
      GLenum internalformat, unsigned width, unsigned height, unsigned depth,
      GLenum fmt, GLenum type, const GLXX_PIXEL_STORE_STATE_T *pack_unpack,
      GLXX_BUFFER_T *pixel_buffer, const void *pixels,
      glxx_context_fences *fences, GLenum *error, bool secure);

extern bool glxx_texture_subimage(GLXX_TEXTURE_T *texture, unsigned face, unsigned level,
      unsigned dst_x, unsigned dst_y, unsigned dst_z,
      GLenum fmt, GLenum type, const GLXX_PIXEL_STORE_STATE_T *pack_unpack,
      GLXX_BUFFER_T *pixel_buffer, const void *pixels,
      unsigned width, unsigned height, unsigned depth,
      glxx_context_fences *fences, GLenum *error);

extern bool glxx_texture_compressed_image(GLXX_TEXTURE_T *texture,
      unsigned face, unsigned level, GLenum internalformat,
      unsigned width, unsigned height, unsigned depth,
      unsigned image_size, GLXX_BUFFER_T *pixel_buffer,
      const void *pixels,
      glxx_context_fences *fences, GLenum *error);

extern bool glxx_texture_compressed_subimage(GLXX_TEXTURE_T *texture, unsigned
      face, unsigned level, unsigned dst_x, unsigned dst_y, unsigned dst_z,
      unsigned width, unsigned height, unsigned depth, GLenum fmt, unsigned
      image_size, GLXX_BUFFER_T *pixel_buffer, const void *pixels,
      glxx_context_fences *fences, GLenum *error);

extern bool glxx_texture_storage(GLXX_TEXTURE_T *texture,unsigned levels,
      GLenum internalformat, unsigned width, unsigned height, unsigned depth,
      glxx_ms_mode ms_mode, bool fixed_sample_locations);

enum glxx_tex_completeness
{
   INCOMPLETE,
   COMPLETE,
   OUT_OF_MEMORY
};

/* sampler must not be NULL */
extern enum glxx_tex_completeness
glxx_texture_ensure_contiguous_blob_if_complete(GLXX_TEXTURE_T *texture, const
      GLXX_TEXTURE_SAMPLER_STATE_T *sampler, unsigned *base_level,
      unsigned *num_levels, glxx_context_fences *fences);

typedef struct
{
   uint32_t hw_param[2];
#if !V3D_HAS_NEW_TMU_CFG
   uint32_t hw_param1_gather[4];
   uint32_t hw_param1_fetch;
#endif
   /* For textureSize: */
   uint32_t width;
   uint32_t height;
   uint32_t depth;

#if !V3D_HAS_NEW_TMU_CFG
   /* For working around GFXH-1363 */
   uint32_t base_level;
#endif

#if !V3D_VER_AT_LEAST(3,3,1,0)
   /* To workaround GFXH-1371 */
   bool force_no_pixmask;
#endif

#if !V3D_HAS_TMU_TEX_WRITE
   /* for imageStore */
   uint32_t arr_stride; /* array_stride */
   glsl_imgunit_swizzling lx_swizzling;
   uint32_t lx_addr; /* address for bound level */
   uint32_t lx_pitch, lx_slice_pitch;
#endif
} GLXX_TEXTURE_UNIF_T;

struct glxx_calc_image_unit;
extern enum glxx_tex_completeness
glxx_texture_key_and_uniforms(GLXX_TEXTURE_T *texture, const struct glxx_calc_image_unit *calc_image_unit,
      const GLXX_TEXTURE_SAMPLER_STATE_T *sampler, bool used_in_bin, bool is_32bit,
      glxx_render_state *rs, GLXX_TEXTURE_UNIF_T *texture_unif,
#if !V3D_VER_AT_LEAST(3,3,0,0)
      glsl_gadgettype_t *gadgettype,
#endif
      glxx_context_fences *fences);

extern bool glxx_texture_is_cube_complete(GLXX_TEXTURE_T *texture);

static inline unsigned glxx_texture_clamped_base_level(const GLXX_TEXTURE_T *texture)
{
   /* For an immutable texture, texture->base_level must be clamped (see page
    * 153, mipmapping) */
   return (texture->immutable_levels == 0) ? texture->base_level :
      gfx_umin(texture->base_level, texture->immutable_levels - 1);
}

/* Returns true if the texture is complete (base complete or mipmap
 * complete, depending on de desired action - input param base_complete)
 * A 2D(3D, 2D image array texture) is complete(mip_complete) if:
 *   a) the set of mipmap arrays base_level to q were specified and have the same
 *   effective internal format
 *   b) the dimensions of the arrays follow the
     c) base_level <= level_max
 * A cube map texture is complete if each face is complete(rules above) and:
 *   d) the base_level of each of the six texture images making up the cube
 *   have identical, positive and square dimensions
 *   e) the base_level arrays were each specified with the same internal format
 * An immutable texture is always complete, but base_level and level_max could
 * be clamped to allowed values and because of that we need a new base_level
 * which is different than texture->base_level (see page 153, mipmapping).
 * out params (valid only when we return COMPLETE):
 *  base_level: the base level to use if the texture is complete
 *  num_levels: how many levels from the base level we are supposed to use for texturing
 *
 *  NOTE: This function does not make sure that all the images for completeness are in the same blob;
 *  if you need that, call glxx_texture_ensure_contiguous_blob_if_complete
 */
extern bool glxx_texture_check_completeness(const GLXX_TEXTURE_T *texture,
   bool base_complete, unsigned *base_level, unsigned *num_levels);

/* maxium number of mipmap levels based on the target and the dimensions */
extern unsigned glxx_tex_max_levels_from_dim(enum glxx_tex_target target,
      unsigned width, unsigned height, unsigned depth);

/* this function is for es1, so the texture should be one supported in es1;
 * returns false if there is no data uploaded in face=0, level=0;
 * returns true otherwise and  fills in has color and has_alpha
 */
extern bool glxx_texture_es1_has_color_alpha(const GLXX_TEXTURE_T *texture,
      bool *has_color, bool *has_alpha);

/* GFX_BUFFER_DESC_T knows only 1D, 2D or 3D images, so:
 * 1D_ARRAY is just 1D image with depth elements in the array,
 * 2D_ARRAY is just 2D image with depth elements in the array
 * in params: target, width, height, depth
 * out params: width, height, depth, num_arrays
 * eg: if we have a 2D array texture, this function will set depth to 1, and
 * return in num_array_elemnts the initial value of depth, because in fact we
 * have num_array_elems of 2D images with width and height;
 */
extern void glxx_tex_transform_dim_for_target(enum glxx_tex_target
      target, unsigned *width, unsigned *height, unsigned *depth, unsigned
      *num_array_elems);

/* same as above, but for offsets :
 * eg: if we have a 2D array texture, this function will set zoffset to 0 and
 * return in start_elem the original value of zoffset */
extern void glxx_tex_transform_offsets_for_target(enum glxx_tex_target target,
      unsigned *xoffset, unsigned *yoffset, unsigned *zoffset, unsigned
      *start_elem);

extern bool glxx_texture_generate_mipmap(GLXX_TEXTURE_T *texture,
      glxx_context_fences *fences, GLenum *error);

extern bool glxx_texture_bind_teximage(GLXX_TEXTURE_T *texture, KHRN_IMAGE_T
      **images, unsigned num_images, unsigned mip_level,
      glxx_context_fences *fences);

extern void glxx_texture_release_teximage(GLXX_TEXTURE_T *texture);

/* add an observer that will be notified when this texture gets unbind (the
 * texture gets respecified or deleted */
extern void glxx_texture_add_observer(GLXX_TEXTURE_T *texture,
      tex_unbind_observer_t observer);
extern void glxx_texture_remove_observer(GLXX_TEXTURE_T *texture);

/* Returns false if we run of of memory; otherwise, returns true and fills in
 * img with the texture image for that face, level, layer. The image is
 * reference counted; the caller needs to call khrn_mem_release on the returned
 * image when done with it.
 * *img can be NULL if there is no texture image for that face/level
 */
bool glxx_texture_acquire_one_elem_slice(GLXX_TEXTURE_T* texture,
      unsigned face, unsigned level, unsigned layer, KHRN_IMAGE_T **img);

/* Call this when you want to share a slice from a texture with an eglimage;
 * You should call this function only on an unbound texture or an egl image
 * source texture;
 * Returns false if we run of of memory; otherwise, returns true and fills in
 * img with the texture image for that face, level, layer. The image is
 * reference counted; the caller needs to call khrn_mem_release on the returned
 * image when done with it.
 * *img can be NULL if there is no texture image for that face/level
 */
bool glxx_texture_acquire_from_eglimage(GLXX_TEXTURE_T *texture, unsigned face,
      unsigned level, unsigned layer, KHRN_IMAGE_T **img);

/* return true if the texture has no texel array outside the range levels
 * start_level -> end_level (range includes start_level and end_level) */
extern bool glxx_texture_has_images_outside_range(const GLXX_TEXTURE_T
      *texture, unsigned start_level, unsigned end_level);

/* GL_OES_draw_texture */
void glxx_texture_set_crop_rect(GLXX_TEXTURE_T *texture, const GLint * params);

extern bool glxx_texture_copy_image(GLXX_TEXTURE_T *texture, unsigned face,
      unsigned level, GFX_LFMT_T dst_fmt, KHRN_IMAGE_T *src,
      int src_x, int src_y, int width, int height,
      glxx_context_fences *fences);

extern bool glxx_texture_copy_sub_image(GLXX_TEXTURE_T *texture, unsigned face,
      unsigned level, int dst_x, int dst_y, int dst_z,
      KHRN_IMAGE_T *src, int src_x, int src_y, int width, int height,
      glxx_context_fences *fences);

bool glxx_texture_get_fixed_sample_locations(const GLXX_TEXTURE_T *texture);

extern unsigned get_target_num_dimensions(enum glxx_tex_target target);

/* GL_OES_compressed_paletted_texture */
extern void glxx_compressed_paletted_teximageX(GLenum target, GLint level,
      GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth,
      GLint border, GLsizei imageSize, const GLvoid *pixels, unsigned dim);


#endif
