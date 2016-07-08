/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Implementation of OpenGL ES 2.0 framebuffer / Open GL ES 1.1 OES_framebuffer_object structure.
=============================================================================*/

#include "../common/khrn_int_common.h"
#include "glxx_int_config.h"

#include "glxx_server.h"
#include "glxx_framebuffer.h"
#include "glxx_renderbuffer.h"
#include "glxx_hw.h"
#include "../common/khrn_render_state.h"
#include "../egl/egl_surface_base.h"

#include "glxx_texture.h"

#include "libs/core/lfmt/lfmt_translate_v3d.h"

typedef enum

{
   GLXX_COLOR_RENDERABLE,
   GLXX_DEPTH_RENDERABLE,
   GLXX_STENCIL_RENDERABLE
}glxx_renderable_cond;

static bool image_is_gl_renderable(const KHRN_IMAGE_T *image,
      glxx_renderable_cond renderable_cond);

static void attachment_init(GLXX_ATTACHMENT_T *att)
{
   att->obj_type = GL_NONE;
   memset(att, 0, sizeof(GLXX_ATTACHMENT_T));
}

static void attachment_reset(GLXX_ATTACHMENT_T *att)
{
   switch(att->obj_type)
   {
      case GL_NONE:
         break;
      case GL_RENDERBUFFER:
         KHRN_MEM_ASSIGN(att->obj.rb, NULL);
         break;
      case GL_TEXTURE:
         KHRN_MEM_ASSIGN(att->obj.tex_info.texture, NULL);
         break;
      case GL_FRAMEBUFFER_DEFAULT:
         KHRN_MEM_ASSIGN(att->obj.fb_default.img, NULL);
         KHRN_MEM_ASSIGN(att->obj.fb_default.ms_img, NULL);
         break;
      default:
         break;
   }
   att->obj_type = GL_NONE;
   memset(att, 0, sizeof(GLXX_ATTACHMENT_T));
}

uint32_t glxx_attachment_get_name(const GLXX_ATTACHMENT_T *att)
{
   uint32_t name;
   switch(att->obj_type)
   {
      case GL_NONE:
      case GL_FRAMEBUFFER_DEFAULT:
         name = 0;
         break;
      case GL_RENDERBUFFER:
         name = att->obj.rb->name;
         break;
      case GL_TEXTURE:
         name = att->obj.tex_info.texture->name;
         break;
      default:
         unreachable();
   }
   return name;
}

glxx_ms_mode attachment_get_ms_mode(const GLXX_ATTACHMENT_T *att)
{
   glxx_ms_mode ms_mode = GLXX_NO_MS;

   switch (att->obj_type)
   {
      case GL_NONE:
         ms_mode = GLXX_NO_MS;
         break;
      case GL_TEXTURE:
         ms_mode = att->obj.tex_info.ms_mode;
         break;
      case GL_RENDERBUFFER:
         ms_mode = att->obj.rb->ms_mode;
         break;
      case GL_FRAMEBUFFER_DEFAULT:
         ms_mode = att->obj.fb_default.ms_mode;
         break;
      default:
         unreachable();
   }
   return ms_mode;
}

bool glxx_attachment_equal(const GLXX_ATTACHMENT_T *att1,
      const GLXX_ATTACHMENT_T *att2)
{
   bool res = false;

   if (att1->obj_type != att2->obj_type)
      return false;

   switch(att1->obj_type)
   {
      case GL_NONE:
         res = true;
         break;
      case GL_RENDERBUFFER:
         res = att1->obj.rb == att2->obj.rb;
         break;
      case GL_TEXTURE:
         if (att1->obj.tex_info.face == att2->obj.tex_info.face &&
             att1->obj.tex_info.level == att2->obj.tex_info.level &&
             att1->obj.tex_info.layer == att2->obj.tex_info.layer &&
             att1->obj.tex_info.texture == att2->obj.tex_info.texture &&
             att1->obj.tex_info.ms_mode == att2->obj.tex_info.ms_mode)

         {
            res = true;
         }
         break;
      case GL_FRAMEBUFFER_DEFAULT:
         /* should we compare that this images are equal as oppose to pointers?
          * */
         if (att1->obj.fb_default.ms_mode == att2->obj.fb_default.ms_mode &&
             att1->obj.fb_default.img == att2->obj.fb_default.img &&
             att1->obj.fb_default.ms_img == att2->obj.fb_default.ms_img)
         {
            res = true;
         }
         break;
      default:
         assert(0);
   }
   return res;
}

void framebuffer_init(GLXX_FRAMEBUFFER_T *fb, int32_t name)
{
   unsigned i;

   memset(fb, 0, sizeof(GLXX_FRAMEBUFFER_T));
   fb->name = name;

   for (i = 0; i < GLXX_ATT_COUNT; i++)
      attachment_init(&fb->attachment[i]);

   fb->read_buffer = GLXX_COLOR0_ATT;

   fb->draw_buffer[0] = true;
   fb->default_ms_mode = GLXX_NO_MS;
   fb->debug_label = NULL;
}


void framebuffer_term(void *v, size_t size)
{
   GLXX_FRAMEBUFFER_T *fb = (GLXX_FRAMEBUFFER_T *)v;
   unsigned i;
   UNUSED(size);

   for (i = 0; i < GLXX_ATT_COUNT; i++)
      attachment_reset(&fb->attachment[i]);

   free(fb->debug_label);
   fb->debug_label = NULL;
}

GLXX_FRAMEBUFFER_T* glxx_fb_create(uint32_t name)
{
   GLXX_FRAMEBUFFER_T *fb;

   fb = KHRN_MEM_ALLOC_STRUCT(GLXX_FRAMEBUFFER_T);
   if (!fb)
      return NULL;

   framebuffer_init(fb, name);
   khrn_mem_set_term(fb, framebuffer_term);
   return fb;
}

static glxx_att_index_t attachment_point_to_att_index(glxx_attachment_point_t att_point)
{
   glxx_att_index_t att_index;

   if (att_point >= GL_COLOR_ATTACHMENT0 &&
       att_point < GL_COLOR_ATTACHMENT0 + GLXX_MAX_RENDER_TARGETS)
   {
      return GLXX_COLOR0_ATT + att_point - GL_COLOR_ATTACHMENT0;
   }

   switch(att_point)
   {
      case GL_DEPTH_ATTACHMENT:
      case GL_DEPTH_STENCIL_ATTACHMENT:
         att_index = GLXX_DEPTH_ATT;
         break;
      case GL_STENCIL_ATTACHMENT:
         att_index = GLXX_STENCIL_ATT;
         break;
      default:
         unreachable();
   }
   return att_index;
}

static GLXX_ATTACHMENT_T* get_attachment(GLXX_FRAMEBUFFER_T *fb,
   glxx_attachment_point_t att_point)
{
   glxx_att_index_t att_index;

   att_index = attachment_point_to_att_index(att_point);
   return &fb->attachment[att_index];
}

const GLXX_ATTACHMENT_T* glxx_fb_get_attachment(const GLXX_FRAMEBUFFER_T *fb,
   glxx_attachment_point_t att_point)
{
   glxx_att_index_t att_index;

   att_index = attachment_point_to_att_index(att_point);
   return &fb->attachment[att_index];
}

void glxx_fb_detach_attachment(GLXX_FRAMEBUFFER_T *fb,
      glxx_attachment_point_t att_point)
{
   GLXX_ATTACHMENT_T *att;

   att = get_attachment(fb, att_point);
   for (unsigned i = 0; i < 2; i++)
   {
      attachment_reset(att);
      if (att_point != GL_DEPTH_STENCIL_ATTACHMENT)
         break;
      att = &fb->attachment[GLXX_STENCIL_ATT];
   }
}

void glxx_fb_detach(GLXX_FRAMEBUFFER_T *fb)
{
   uint32_t name = fb->name;
   framebuffer_term(fb, sizeof(GLXX_FRAMEBUFFER_T));
   framebuffer_init(fb, name);
}

void glxx_fb_attach_texture(GLXX_FRAMEBUFFER_T *fb,
      glxx_attachment_point_t att_point, GLXX_TEXTURE_T *texture,
      unsigned face, unsigned level, unsigned layer,
      glxx_ms_mode ms_mode)
{
   if (glxx_tex_target_is_multisample(texture->target))
   {
      assert(ms_mode == GLXX_NO_MS);
      assert(texture->ms_mode != GLXX_NO_MS);
      ms_mode = texture->ms_mode;
   }

   GLXX_ATTACHMENT_T *att = get_attachment(fb, att_point);
   for (unsigned i = 0; i < 2; i++)
   {
      attachment_reset(att);

      att->obj_type = GL_TEXTURE;
      KHRN_MEM_ASSIGN(att->obj.tex_info.texture, texture);
      att->obj.tex_info.face = face;
      att->obj.tex_info.level = level;
      att->obj.tex_info.layer = layer;

      /* we can have ms fb but store the result in a downsampled buffer,
       * or multisample and store result with all samples */
      assert(texture->ms_mode == GLXX_NO_MS ||
         texture->ms_mode == ms_mode);

      att->obj.tex_info.ms_mode = ms_mode;
      if (att_point != GL_DEPTH_STENCIL_ATTACHMENT)
         break;
      att = &fb->attachment[GLXX_STENCIL_ATT];
   }
}

void glxx_fb_attach_renderbuffer(GLXX_FRAMEBUFFER_T *fb,
      glxx_attachment_point_t att_point, GLXX_RENDERBUFFER_T *rb)
{
   GLXX_ATTACHMENT_T *att;
   unsigned i;

   att = get_attachment(fb, att_point);
   for (i = 0; i < 2; i++)
   {
      attachment_reset(att);

      att->obj_type = GL_RENDERBUFFER;
      KHRN_MEM_ASSIGN(att->obj.rb, rb);
      if (att_point != GL_DEPTH_STENCIL_ATTACHMENT)
         break;
      att = &fb->attachment[GLXX_STENCIL_ATT];
   }
}

static void fb_attach_fb_default(GLXX_FRAMEBUFFER_T *fb,
      glxx_attachment_point_t att_point,
      KHRN_IMAGE_T *img, KHRN_IMAGE_T *ms_img, glxx_ms_mode ms_mode)
{
   GLXX_ATTACHMENT_T *att;
   unsigned i;

   assert((ms_mode == GLXX_NO_MS && img != NULL) ||
            (ms_mode != GLXX_NO_MS && ms_img != NULL));

   att = get_attachment(fb, att_point);
   for (i = 0; i < 2; i++)
   {
      attachment_reset(att);
      att->obj_type = GL_FRAMEBUFFER_DEFAULT;
      KHRN_MEM_ASSIGN(att->obj.fb_default.img, img);
      KHRN_MEM_ASSIGN(att->obj.fb_default.ms_img, ms_img);
      att->obj.fb_default.ms_mode = ms_mode;

      if (att_point != GL_DEPTH_STENCIL_ATTACHMENT)
         break;
      att = &fb->attachment[GLXX_STENCIL_ATT];
   }
}

#ifndef NDEBUG

/* We need to have:
 * - a color image
 * - depth and stencil images are optional
 * - if we multisample (aux_samples > 0), we must have a multisample image
 * - if we multimsaple, size of each aux_buffer must be aux_sample * size of color_image;
 */
static void consistent_for_default_fb(const EGL_SURFACE_T *surface)
{
   unsigned width, height;
   KHRN_IMAGE_T *color0;
   KHRN_IMAGE_T *aux_bufs[3];

   color0 = egl_surface_get_back_buffer(surface);
   aux_bufs[0] = egl_surface_get_aux_buffer(surface, AUX_MULTISAMPLE);
   aux_bufs[1] = egl_surface_get_aux_buffer(surface, AUX_DEPTH);
   aux_bufs[2] = egl_surface_get_aux_buffer(surface, AUX_STENCIL);

   width = khrn_image_get_width(color0);
   height = khrn_image_get_height(color0);

   assert(width <= GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE &&
         height <= GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE);

   glxx_ms_mode ms_mode = glxx_samples_to_ms_mode(surface->config->samples);
   unsigned scale  = glxx_ms_mode_get_scale(ms_mode);

   if (ms_mode != GLXX_NO_MS)
   {
      /* if we multisample, we must have a multisample aux image */
      if (!aux_bufs[0])
         assert(0);
   }

   for (unsigned i = 0; i < 3; i++)
   {
      unsigned aux_width, aux_height;

      if (aux_bufs[i])
      {
         aux_width = khrn_image_get_width(aux_bufs[i]);
         aux_height = khrn_image_get_height(aux_bufs[i]);
         /* we would normally check for equality here, but in the pbuffer case
          * with mimmap levels we do not re-create the depth and stencil , so
          * as long as they are big enough, we are ok */
         if (aux_width < (scale * width))
            assert(0);
         if (aux_height < (scale * height))
            assert(0);
      }
   }

   /* if both depth and stencil are present, they must point to the same image
    */
   if (aux_bufs[1] && aux_bufs[2])
   {
      khrn_image_equal(aux_bufs[1], aux_bufs[2]);
   }
}
#endif

void glxx_fb_attach_egl_surface(GLXX_FRAMEBUFFER_T *fb,
      const EGL_SURFACE_T *surface)
{
   KHRN_IMAGE_T *color0, *ms_color0, *depth, *stencil;

   /* we want this check only in debug */
#ifndef NDEBUG
   consistent_for_default_fb(surface);
#endif

   glxx_fb_detach(fb);

   glxx_ms_mode ms_mode = glxx_samples_to_ms_mode(surface->config->samples);
   bool ms = ms_mode != GLXX_NO_MS;

   color0 = egl_surface_get_back_buffer(surface);

   ms_color0 = egl_surface_get_aux_buffer(surface, AUX_MULTISAMPLE);
   depth = egl_surface_get_aux_buffer(surface, AUX_DEPTH);
   stencil = egl_surface_get_aux_buffer(surface, AUX_STENCIL);

   fb_attach_fb_default(fb, GL_COLOR_ATTACHMENT0, color0, ms_color0,
         ms_mode);

   /* if we multisample, depth and stencil are multisampled images */
   if (depth)
      fb_attach_fb_default(fb, GL_DEPTH_ATTACHMENT, ms ? NULL : depth,
           ms ? depth : NULL, ms_mode);
   if(stencil)
      fb_attach_fb_default(fb, GL_STENCIL_ATTACHMENT, ms ? NULL : stencil,
           ms ? stencil : NULL, ms_mode);
}

void glxx_fb_detach_renderbuffer(GLXX_FRAMEBUFFER_T *fb,
      GLXX_RENDERBUFFER_T *rb)
{
   unsigned i;

   for ( i = 0; i < GLXX_ATT_COUNT; i++)
   {
      GLXX_ATTACHMENT_T *att  = &fb->attachment[i];
      if (att->obj_type == GL_RENDERBUFFER && att->obj.rb == rb)
         attachment_reset(att);
   }
}

void glxx_fb_detach_texture(GLXX_FRAMEBUFFER_T *fb,
      GLXX_TEXTURE_T *texture)
{
   unsigned i;

   for ( i = 0; i < GLXX_ATT_COUNT; i++)
   {
      GLXX_ATTACHMENT_T *att  = &fb->attachment[i];
      if (att->obj_type == GL_TEXTURE &&
          att->obj.tex_info.texture == texture)
      {
         attachment_reset(att);
      }
   }
}

const GLXX_ATTACHMENT_T* glxx_fb_get_read_buffer(const GLXX_FRAMEBUFFER_T *fb)
{
   if (fb->read_buffer == GLXX_INVALID_ATT)
      return NULL;

   return &fb->attachment[fb->read_buffer];
}

glxx_ms_mode glxx_fb_get_ms_mode(const GLXX_FRAMEBUFFER_T *fb)
{
   unsigned i;

   /* this should be called only on a complete fb; on a complete fb all the
    * present attachments must have equal samples
    */

   for (i = 0; i < GLXX_ATT_COUNT; i++)
   {
      const GLXX_ATTACHMENT_T *att = &fb->attachment[i];
      if (att->obj_type != GL_NONE)
        return attachment_get_ms_mode(&fb->attachment[i]);
   }
   return fb->default_ms_mode;
}

bool glxx_fb_acquire_read_image(const GLXX_FRAMEBUFFER_T *fb,
      glxx_att_img_t img_type, KHRN_IMAGE_T **img, bool *ms)
{
   const GLXX_ATTACHMENT_T *att;
   *img = NULL;

   att = glxx_fb_get_read_buffer(fb);
   if (att == NULL)
      return true;

   return glxx_attachment_acquire_image(att, img_type, img, ms);
}

static bool attachment_acquire_specific_image(const GLXX_ATTACHMENT_T *att,
      bool require_ms_img, KHRN_IMAGE_T **img)
{
   bool res = true;
   *img = NULL;

   switch (att->obj_type)
   {
   case GL_NONE:
      break;
   case GL_TEXTURE:
      {
         const struct texture_info* tex_info = &att->obj.tex_info;
         bool is_ms_txt = tex_info->texture->ms_mode != GLXX_NO_MS;
         if (require_ms_img == is_ms_txt)
         {
            res = glxx_texture_acquire_one_elem_slice(tex_info->texture,
                  tex_info->face, tex_info->level, tex_info->layer, img);
         }
      }
      break;
   case GL_RENDERBUFFER:
      {
         bool is_ms_img = att->obj.rb->ms_mode != GLXX_NO_MS;
         if (require_ms_img == is_ms_img)
         {
            KHRN_MEM_ASSIGN(*img, att->obj.rb->image);
         }
      }
      break;
   case GL_FRAMEBUFFER_DEFAULT:
      if (require_ms_img)
         KHRN_MEM_ASSIGN(*img, att->obj.fb_default.ms_img);
      else
         KHRN_MEM_ASSIGN(*img, att->obj.fb_default.img);
      break;
   default:
      UNREACHABLE();
      res = false;
   }
   return res;
}

bool glxx_attachment_acquire_image(const GLXX_ATTACHMENT_T *att,
      glxx_att_img_t img_type, KHRN_IMAGE_T **img, bool *is_ms)
{
   bool res;
   bool ms;
   switch(img_type)
   {
      case GLXX_DOWNSAMPLED:
         ms = false;
         res = attachment_acquire_specific_image(att, ms, img);
         break;
      case GLXX_MULTISAMPLED:
         ms = true;
         res = attachment_acquire_specific_image(att, ms, img);
         break;
     case GLXX_PREFER_DOWNSAMPLED:
         ms = false;
         res = attachment_acquire_specific_image(att, ms, img);
         if (res && !*img)
         {
            ms = !ms;
            res = attachment_acquire_specific_image(att, ms, img);
         }
         break;
     case GLXX_PREFER_MULTISAMPLED:
         ms = true;
         res = attachment_acquire_specific_image(att, ms, img);
         if (res && !*img)
         {
            ms = !ms;
            res = attachment_acquire_specific_image(att, ms, img);
         }
         break;
     default:
         UNREACHABLE();
         res = false;
   }

   if (res && *img && is_ms)
      *is_ms = ms;
   return res;
}


GFX_LFMT_T glxx_attachment_get_api_fmt(const GLXX_ATTACHMENT_T *att)
{
   GFX_LFMT_T res = GFX_LFMT_NONE;
   KHRN_IMAGE_T *img;
   bool img_ms;

   /* the api_fmt is the same for multisample or downsampled image, so it
    * doesn't matter what op we use */
   if (!glxx_attachment_acquire_image(att, GLXX_PREFER_DOWNSAMPLED, &img, &img_ms))
      return res;

   if (img)
   {
      res = img->api_fmt;
      KHRN_MEM_ASSIGN(img, NULL);
   }
   return res;
}

static bool image_is_gl_renderable(const KHRN_IMAGE_T *image,
      glxx_renderable_cond renderable_cond)
{
   bool (*fct_is_renderable)(GFX_LFMT_T lfmt);

   switch (renderable_cond)
   {
      case GLXX_DEPTH_RENDERABLE:
         fct_is_renderable = glxx_is_depth_renderable_from_api_fmt;
         break;
      case GLXX_STENCIL_RENDERABLE:
         fct_is_renderable = glxx_is_stencil_renderable_from_api_fmt;
         break;
      case GLXX_COLOR_RENDERABLE:
         fct_is_renderable = glxx_is_color_renderable_from_api_fmt;
         break;
      default:
         unreachable();
   }

   if (fct_is_renderable(image->api_fmt))
      return true;
   else
      return false;
}

static glxx_renderable_cond get_renderable_condition(glxx_att_index_t att_index)
{
   if (att_index >= GLXX_COLOR0_ATT &&
      att_index < GLXX_COLOR0_ATT + GLXX_MAX_RENDER_TARGETS)
         return GLXX_COLOR_RENDERABLE;
   if (att_index == GLXX_DEPTH_ATT)
         return GLXX_DEPTH_RENDERABLE;
   assert(att_index == GLXX_STENCIL_ATT);
   return GLXX_STENCIL_RENDERABLE;
}

typedef enum {
   ATTACHMENT_MISSING = 0,
   ATTACHMENT_INCOMPLETE,
   ATTACHMENT_COMPLETE,
} att_status_t;

static att_status_t attachment_status(const GLXX_ATTACHMENT_T *att,
      glxx_renderable_cond renderable_cond)
{
   att_status_t status = ATTACHMENT_INCOMPLETE;
   KHRN_IMAGE_T *img = NULL;

   switch (att->obj_type)
   {
      case GL_NONE:
         status = ATTACHMENT_MISSING;
         break;
      case GL_TEXTURE:
      case GL_RENDERBUFFER:
         {
            bool is_ms;
            unsigned scale = 1;

            if(!glxx_attachment_acquire_image(att, GLXX_PREFER_DOWNSAMPLED, &img, &is_ms))
               break;

            if (!img)
               break;

            /* samples must be <= glGetInteger(GL_MAX_FRAMEBUFFER_SAMPLES) */
            if (attachment_get_ms_mode(att) >  GLXX_CONFIG_MAX_SAMPLES)
               break;

            /* image dimensions must be <= glGetInteger(GL_MAX_FRAMEBUFFER_WIDTH)
             * and glGetInteger(GL_MAX_FRAMEBUFFER_HEIGHT) */
            if (is_ms)
               scale = 2;
            if ((khrn_image_get_width(img)/scale) >  GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE ||
                 (khrn_image_get_height(img)/scale) > GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE )
               break;

            if (image_is_gl_renderable(img, renderable_cond))
               status = ATTACHMENT_COMPLETE;
         }
         break;
      case GL_FRAMEBUFFER_DEFAULT:
         /* fb is always complete */
         status = ATTACHMENT_COMPLETE;
         break;
      default:
         UNREACHABLE();
         break;
   }
   KHRN_MEM_ASSIGN(img, NULL);
   return status;
}


glxx_fb_status_t glxx_fb_completeness_status(const GLXX_FRAMEBUFFER_T *fb)
{
   glxx_ms_mode common_ms_mode = GLXX_NO_MS;
   bool no_attachments = true;

   if (fb->name == 0)
   {
      const GLXX_ATTACHMENT_T *att = &fb->attachment[GLXX_COLOR0_ATT];
      /* if target is the default framebuffer, fb is complete if the default
       * framebuffer exists */
      if (att->obj_type == GL_FRAMEBUFFER_DEFAULT)
         return GL_FRAMEBUFFER_COMPLETE;

      /* eglMakecurrent was called with NULL egl_surface */
      assert(att->obj_type == GL_NONE);
      return GL_FRAMEBUFFER_UNDEFINED;
   }

   bool has_rb_att = false;
   bool has_texture_att = false;
   bool fixed_sample_loc = false;
   for (unsigned b = 0; b < GLXX_ATT_COUNT; b++)
   {
      glxx_renderable_cond renderable_cond;
      att_status_t status;
      const GLXX_ATTACHMENT_T *att = &fb->attachment[b];

      renderable_cond = get_renderable_condition(b);
      status = attachment_status(att, renderable_cond);

      if (status == ATTACHMENT_MISSING)
         continue;
      if (status == ATTACHMENT_INCOMPLETE)
        return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;

      assert(status == ATTACHMENT_COMPLETE);
      glxx_ms_mode ms_mode = attachment_get_ms_mode(att);
      if (no_attachments)
         common_ms_mode = ms_mode;
      else
      {
         if (common_ms_mode != ms_mode)
            return GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE;
      }
      no_attachments = false;
      switch (att->obj_type)
      {
      case GL_RENDERBUFFER:
         has_rb_att = true;
         break;
      case GL_TEXTURE:
         {
            bool tex_fixed_sample_loc =
               glxx_texture_get_fixed_sample_locations(att->obj.tex_info.texture);
            if (!has_texture_att)
            {
               has_texture_att = true;
               fixed_sample_loc = tex_fixed_sample_loc;
            }
            else if (tex_fixed_sample_loc != fixed_sample_loc)
               return GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE;
         }
         break;
      default:
         unreachable();
      }
   }

   if (has_rb_att && has_texture_att)
   {
      if (fixed_sample_loc != true)
         return GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE;
   }

   if (no_attachments)
   {
      if (fb->default_width == 0 || fb->default_height == 0)
         return GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
   }

   // Depth and stencil attachments, if present, are the same image.
   if (fb->attachment[GLXX_DEPTH_ATT].obj_type != GL_NONE &&
       fb->attachment[GLXX_STENCIL_ATT].obj_type != GL_NONE)
   {
      if (!glxx_attachment_equal(&fb->attachment[GLXX_DEPTH_ATT],
               &fb->attachment[GLXX_STENCIL_ATT]))
         return GL_FRAMEBUFFER_UNSUPPORTED;
   }

   return GL_FRAMEBUFFER_COMPLETE;
}

bool glxx_fb_is_complete(const GLXX_FRAMEBUFFER_T* fb)
{
   glxx_fb_status_t status;
   status = glxx_fb_completeness_status(fb);
   return status == GL_FRAMEBUFFER_COMPLETE;
}

bool glxx_fb_iterate_valid_draw_bufs(const GLXX_FRAMEBUFFER_T *fb,
      unsigned *i, glxx_att_index_t *att_index)
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

bool glxx_fb_is_valid_draw_buf(const GLXX_FRAMEBUFFER_T *fb,
      glxx_att_index_t att_index)
{
   assert(att_index >= GLXX_COLOR0_ATT &&
          att_index < (GLXX_COLOR0_ATT + GLXX_MAX_RENDER_TARGETS));

   return fb->draw_buffer[att_index - GLXX_COLOR0_ATT] &&
      fb->attachment[att_index].obj_type != GL_NONE;
}
