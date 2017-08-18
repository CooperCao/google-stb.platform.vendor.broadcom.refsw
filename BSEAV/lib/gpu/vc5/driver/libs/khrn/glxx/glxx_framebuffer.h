/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_FRAMEBUFFER_H
#define GLXX_FRAMEBUFFER_H

#include "../egl/egl_context_gl.h"
#include "gl_public_api.h"

#include "glxx_texture.h"
#include "../common/khrn_image_plane.h"
#include "glxx_renderbuffer.h"

#include "glxx_hw_framebuffer.h"

#include "../egl/egl_surface.h"
#include "glxx_utils.h"

typedef enum
{
   GL_OBJ_NONE = GL_NONE, /* cannot use enumify on this one because it was
                            already used by glxx_query.h */
   enumify(GL_RENDERBUFFER),
   enumify(GL_TEXTURE),
   enumify(GL_FRAMEBUFFER_DEFAULT)
}glxx_att_obj_type_t;

typedef struct
{
   glxx_att_obj_type_t obj_type;

   union
   {
      GLXX_RENDERBUFFER_T *rb;

      struct texture_info
      {
         unsigned level;      /* mip-map level to use */

         bool use_face_layer; /* if this is true, use the specified face and layer of the texture;
                                 if false, use all the faces and layers for that level,
                                 depending on the texture type
                                 (e.g : only cube_map uses all the faces) */
         unsigned face;
         unsigned layer;

         glxx_ms_mode ms_mode; /* if we multisample, the result will be
                                  stored in a downsampled texture
                                  (the case for FramebufferTexture2DMultisampleEXT)
                                  or multisampled texture
                                  (in the case of texture=2D_MULTISAMPLE) */

        GLXX_TEXTURE_T *texture;
      }tex_info;

      struct
      {
         glxx_ms_mode ms_mode;
         khrn_image *img;
         khrn_image *ms_img;  /* if samples == 0, img != NULL && ms_img == NULL;
                                   if samples != 0, ms_img != NULL and we
                                   might also have a downsampled image in img
                                   (e.g: when we multismaple, in the color attachment case,we have a
                                   multisampled image and a downsampled image;
                                   in the depth attachmnent case, we have only
                                   a multisampled image */
      }fb_default;
   }obj;

}GLXX_ATTACHMENT_T;

typedef enum
{
   GLXX_DEPTH_ATT,
   GLXX_STENCIL_ATT,
   GLXX_COLOR0_ATT,  /* do not add any entries here;
                      * we have GLXX_MAX_RENDER_TARGETS colour attachments */
   GLXX_ATT_COUNT = GLXX_COLOR0_ATT + GLXX_MAX_RENDER_TARGETS,

   GLXX_INVALID_ATT
}glxx_att_index_t;

typedef enum
{
   enumify(GL_DEPTH_ATTACHMENT),
   enumify(GL_STENCIL_ATTACHMENT),
   enumify(GL_DEPTH_STENCIL_ATTACHMENT),
   enumify(GL_COLOR_ATTACHMENT0),
   /* enumify(GL_COLOR_ATTACHMENTi) = enumify(GL_COLOR_ATTACHMENT0) + i,
    * where i = 1.. GLXX_MAX_RENDER_TARGETS */
}glxx_attachment_point_t;

typedef struct
{
   uint32_t name;

   GLXX_ATTACHMENT_T attachment[GLXX_ATT_COUNT];

   /* for each color attachement, specifies if drawing to that buffer is
    * enabled or disabled */
   bool  draw_buffer[GLXX_MAX_RENDER_TARGETS];

   glxx_att_index_t read_buffer; /* can be any of GLXX_COLORx_ATT or
                                    GLXX_INVALID_ATT */

   unsigned default_width;
   unsigned default_height;
   unsigned default_layers;
   unsigned default_samples;
   glxx_ms_mode default_ms_mode; /* default_ms_mode is calculated from default_samples;
                                    we need to keep default_samples because the user can
                                    enquire that value */
   bool default_fixed_sample_locations;

   char *debug_label;

}GLXX_FRAMEBUFFER_T;

extern GLXX_FRAMEBUFFER_T* glxx_fb_create(uint32_t name);

/* return NULL if read buffer was set to GL_NONE */
const GLXX_ATTACHMENT_T* glxx_fb_get_read_buffer(const GLXX_FRAMEBUFFER_T *fb);

const GLXX_ATTACHMENT_T* glxx_fb_get_attachment(const GLXX_FRAMEBUFFER_T *fb,
      glxx_attachment_point_t att_point);

static inline
const GLXX_ATTACHMENT_T* glxx_fb_get_attachment_by_index(const GLXX_FRAMEBUFFER_T *fb,
      glxx_att_index_t att_index)
{
   assert(att_index >= 0 && att_index < GLXX_ATT_COUNT);
   return &fb->attachment[att_index];
}

extern void glxx_fb_attach_texture(GLXX_FRAMEBUFFER_T *fb,
      glxx_attachment_point_t att_point, GLXX_TEXTURE_T *texture,
      unsigned level,
      bool use_face_layer, unsigned face, unsigned layer,
      glxx_ms_mode ms_mode);

extern void glxx_fb_attach_renderbuffer(GLXX_FRAMEBUFFER_T *fb,
      glxx_attachment_point_t att_point, GLXX_RENDERBUFFER_T *rb);

extern void glxx_fb_attach_egl_surface(GLXX_FRAMEBUFFER_T *fb,
      const EGL_SURFACE_T *surface);

/* if this fb has any attachment that contains texture, detach it */
extern void glxx_fb_detach_texture(GLXX_FRAMEBUFFER_T *fb, GLXX_TEXTURE_T
      *texture);
/* if this fb has any attachment that contains renderbuffer, detach it */
extern void glxx_fb_detach_renderbuffer(GLXX_FRAMEBUFFER_T *fb,
      GLXX_RENDERBUFFER_T *rb);

/* detach attachment for point att_point */
extern void glxx_fb_detach_attachment(GLXX_FRAMEBUFFER_T *fb,
      glxx_attachment_point_t att_point);

/* detach all the attachments */
extern void glxx_fb_detach(GLXX_FRAMEBUFFER_T *fb);

/* this should be called only on a complete fb;
 * otherwise the value of multisample mode for the whole buffer is undefined */
extern glxx_ms_mode glxx_fb_get_ms_mode(const GLXX_FRAMEBUFFER_T *fb);

/* glxx_att_index_t must be GLXX_COLORx_ATT;
 * returns true if the specified colour attachment is enabled for drawing and
 * has an object attached */
extern bool glxx_fb_is_valid_draw_buf(const GLXX_FRAMEBUFFER_T *fb,
      glxx_att_index_t att_index_t);

/* starting with position i, find the first color attachment enabled for
 * drawing that has an object attached;
 * return true if such attachment is found; if attachment is found,
 * i will be changed to position where we found attachment + 1
 * and att_index will contain the index for that attachment (GLXX_COLORx_ATT) */
static inline bool glxx_fb_iterate_valid_draw_bufs(const GLXX_FRAMEBUFFER_T *fb, unsigned *i,
      glxx_att_index_t *att_index)
{
   for ( ; *i < GLXX_MAX_RENDER_TARGETS; (*i)++)
   {
      if (glxx_fb_is_valid_draw_buf(fb, GLXX_COLOR0_ATT + *i))
      {
         *att_index = GLXX_COLOR0_ATT + *i;
         (*i)++;
         return true;
      }
   }
   return false;
}

static inline uint32_t glxx_fb_get_valid_draw_buf_mask(const GLXX_FRAMEBUFFER_T *fb)
{
   uint32_t mask = 0;

   glxx_att_index_t att_index;
   unsigned i = 0;
   while (glxx_fb_iterate_valid_draw_bufs(fb, &i, &att_index))
      mask |= 1u << (att_index - GLXX_COLOR0_ATT);

   return mask;
}

extern bool glxx_attachment_equal(const GLXX_ATTACHMENT_T *att1,
      const GLXX_ATTACHMENT_T *att2);
extern uint32_t glxx_attachment_get_name(const GLXX_ATTACHMENT_T *att);

typedef enum
{
   GLXX_PREFER_DOWNSAMPLED,
   GLXX_PREFER_MULTISAMPLED,
   GLXX_DOWNSAMPLED,
   GLXX_MULTISAMPLED
}glxx_att_img_t;

/* Ask for an image from the attachment.
 * An attachement can have attached:
 *  - no image
 *  - a downsampled image
 *  - a multismaple image
 *  - a downsampled and multisampled image
 *  The user can ask for a specific image (downsampled/multisampled),
 *  or a perferred one (prefer_ ).
 *
 *  When a specific att_img (multisampled/donwsampled) is requested, if we
 *  cannot find an image with that type in the attachemnt, we fill in img with
 *  NULL.
 *
 *  When a prefered att_img (prefer_x) is requested, if we cannot find an image
 *  with that type in the attachemnt, we fill in img with whatever image is
 *  present in that attachment (NULL id no image attached).
 *
 *  if the attachment is a layered texture and use_0_if_layered is false,
 *  return an image refering to all the images of the attached texture level;
 *  otherwise return an image refering to layer 0/face 0 of the attached
 *  texture level;
 *
 *  if use_0_if_layers is true, fences can be NULL;
 *
 * Returns false if driver run out of resources.
 * Returns true otherwise. If the returned img != NULL then is_ms will
 * reflect the type of the image returned (is_ms = true --> multisampled image)
 * The image, is reference counted and the caller must release it.
 *
 * is_ms can be NULL if not needed.
 */
extern bool glxx_attachment_acquire_image(const GLXX_ATTACHMENT_T *att,
      glxx_att_img_t att_img, bool use_0_if_layered,
      glxx_context_fences *fences,
      khrn_image **img, bool *is_ms);

/* Acquires the desired att_img image from the read buffer specified for this
 * fb. If read buffer was set to none, img will be NULL.
 * If the attachment is a layered texture, return the image for layer 0 of the
 * attached texture level.
 *
 * Returns false if driver run out of resources.
 * Filled in image is reference counted and the caller must release it.
 */
extern bool glxx_fb_acquire_read_image(const GLXX_FRAMEBUFFER_T *fb,
      glxx_att_img_t att_img,
      khrn_image **img, bool *ms);

extern GFX_LFMT_T glxx_attachment_get_api_fmt(const GLXX_ATTACHMENT_T *attachment);

extern bool glxx_fb_is_complete(const GLXX_FRAMEBUFFER_T *fb, glxx_context_fences *fences);

typedef enum
{
   enumify(GL_FRAMEBUFFER_COMPLETE),
   enumify(GL_FRAMEBUFFER_UNDEFINED),
   enumify(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT),
   enumify(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE),
   enumify(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT),
   enumify(GL_FRAMEBUFFER_UNSUPPORTED)
}glxx_fb_status_t;

extern glxx_fb_status_t glxx_fb_completeness_status(const GLXX_FRAMEBUFFER_T
      *fb, glxx_context_fences *fences);

#endif
