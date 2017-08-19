/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>
#include "egl_context_gl.h"
#include "egl_image.h"
#include "egl_image_framebuffer.h"
#include "../glxx/glxx_server.h"
#include "../glxx/glxx_framebuffer.h"
#include "../glxx/glxx_ds_to_color.h"
#include "../common/khrn_blob.h"

/* this should be changed to say which image type prefers */
static bool framebuffer_acquire_image(GLXX_SERVER_STATE_T *state,
     glxx_fb_target_t target, glxx_attachment_point_t att_point, khrn_image **img)
{
   const GLXX_ATTACHMENT_T *att = NULL;
   GLXX_FRAMEBUFFER_T *fb;
   glxx_att_img_t img_type;
   bool img_ms;

   *img = NULL;

   fb = glxx_server_get_bound_fb(state, target);
   /* we always have a bound framebuffer */
   assert(fb);
   if (target == GL_READ_FRAMEBUFFER)
      img_type = GLXX_PREFER_DOWNSAMPLED;
   else
      img_type = GLXX_DOWNSAMPLED;

   att = glxx_fb_get_attachment(fb, att_point);

   if (!att) return true;

   return glxx_attachment_acquire_image(att, img_type, true, &state->fences, img, &img_ms);
}

static bool parse_attribs(const void *attrib_list,
      EGL_AttribType attrib_type, glxx_fb_target_t *target,
      glxx_attachment_point_t *att_point)
{
   bool target_found = false;
   bool att_point_found = false;

   if (!attrib_list)
      return false;

   EGLint name;
   EGLAttribKHR value;
   while (egl_next_attrib(&attrib_list, attrib_type, &name, &value))
   {
      switch (name)
      {
      case EGL_GL_FRAMEBUFFER_TARGET_BRCM:
         switch (value)
         {
         case GL_READ_FRAMEBUFFER:
         case GL_DRAW_FRAMEBUFFER:
            *target = (glxx_fb_target_t)value;
            target_found = true;
            break;
         default:
            return false;
         }
         break;

      case EGL_GL_FRAMEBUFFER_ATTACHMENT_BRCM:
         if (value >= GL_COLOR_ATTACHMENT0 &&
             value < (GL_COLOR_ATTACHMENT0 + GLXX_MAX_RENDER_TARGETS))
         {
            *att_point = (glxx_attachment_point_t)value;
         }
         else if (value == GL_DEPTH_ATTACHMENT || value == GL_STENCIL_ATTACHMENT)
            *att_point = (glxx_attachment_point_t)value;
         else
            return false;
         att_point_found = true;
         break;
      default:
         return false;
      }
   }

   return att_point_found && target_found;
}

/*
 * Create a new image/blob that uses the same resource as the passed in
 * depth/stencil image. The new image will describe only one plane in the
 * original image (if the original image has more than one plane).
 * We also change the fmt of the new image from DS to RG(BA)
 */
static khrn_image* create_color_image_from_plane_in_ds(const khrn_image *image,
      unsigned plane)
{
   GFX_LFMT_T img_plane_lfmt = khrn_image_get_lfmt(image, plane);

   assert(gfx_lfmt_has_depth(img_plane_lfmt) || gfx_lfmt_has_stencil(img_plane_lfmt));
   assert(khrn_image_get_num_planes(image) > plane);

   const khrn_blob *blob = image->blob;
   GFX_BUFFER_DESC_T desc[KHRN_MAX_MIP_LEVELS];

   GFX_LFMT_T color_lfmt = glxx_ds_lfmt_to_color(img_plane_lfmt);
   memset(desc, 0, sizeof(desc));
   for (unsigned i = 0; i < blob->num_mip_levels; i++)
   {
      desc[i].width = blob->desc[i].width;
      desc[i].height = blob->desc[i].height;
      desc[i].depth = blob->desc[i].depth;
      desc[i].num_planes = 1;
      desc[i].planes[0] = blob->desc[i].planes[plane];
      /* change lfmt to color; since we wrap the original resource, do it for
       * each mip level */
      assert(gfx_lfmt_fmt(desc[i].planes[0].lfmt) == gfx_lfmt_fmt(img_plane_lfmt));
      desc[i].planes[0].lfmt = gfx_lfmt_set_format(desc[i].planes[0].lfmt, color_lfmt);
   }

   khrn_blob *new_blob = khrn_blob_create_from_resource(blob->res,
         desc, blob->num_mip_levels, blob->num_array_elems,
         blob->array_pitch, blob->usage, blob->secure);

   GFX_LFMT_T api_fmt = glxx_ds_lfmt_to_color(img_plane_lfmt);
   khrn_image *plane_img;
   /* this is  from a fb; it will always be one elem one slice */
   assert(khrn_image_is_one_elem_slice(image));
   plane_img = khrn_image_create_one_elem_slice(new_blob,
             image->start_elem, image->start_slice, image->level, api_fmt);

   KHRN_MEM_ASSIGN(new_blob, NULL); /* plane_img has a reference to the new_blob */

   return plane_img;
}

EGL_IMAGE_T *egl_image_framebuffer_new(EGL_CONTEXT_T *context,
      EGLenum target, EGLClientBuffer buffer, const void *attrib_list,
      EGL_AttribType attrib_type)
{
   EGLint error = EGL_BAD_ALLOC;
   GLXX_SERVER_STATE_T *state;
   khrn_image *image = NULL;
   EGL_GL_CONTEXT_T *ctx;
   glxx_fb_target_t fb_target = 0;
   glxx_attachment_point_t att_point = 0;
   EGL_IMAGE_T *egl_image = NULL;
   bool locked = false;

   if (!egl_context_gl_lock())
      goto end;
   locked = true;

   if (context == NULL || context->api != API_OPENGL)
   {
      error = EGL_BAD_MATCH;
      goto end;
   }

   if (buffer != NULL)
   {
      /*
       * Guess it might make sense to use a non-bound FBO if this was a real
       * extension
       */
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   if (!parse_attribs(attrib_list, attrib_type, &fb_target, &att_point))
   {
      error = EGL_BAD_ATTRIBUTE;
      goto end;
   }

   ctx = (EGL_GL_CONTEXT_T *) context;


   state = egl_context_gl_server_state(ctx);
   assert(state);

   if (!framebuffer_acquire_image(state, fb_target, att_point, &image))
      goto end;

   if (!image)
   {
      error = EGL_BAD_MATCH;
      goto end;
   }

   if (att_point == GL_DEPTH_ATTACHMENT || att_point == GL_STENCIL_ATTACHMENT)
   {
      khrn_image *new_image;
      unsigned plane = 0;
      if (image->api_fmt == GFX_LFMT_D32_S8X24_FLOAT_UINT)
      {
         assert(khrn_image_get_num_planes(image) == 2);
         if (att_point == GL_STENCIL_ATTACHMENT)
            plane = 1;
      }
      else
      {
         assert(khrn_image_get_num_planes(image) == 1);
      }
      new_image = create_color_image_from_plane_in_ds(image, plane);
      KHRN_MEM_ASSIGN(image, NULL);
      image = new_image;
   }

   if (!image)
      goto end;

   egl_image = egl_image_create(image);
   KHRN_MEM_ASSIGN(image, NULL);
   if (!egl_image)
      goto end;
   error = EGL_SUCCESS;

end:
   if (locked)
      egl_context_gl_unlock();
   if (error != EGL_SUCCESS)
      KHRN_MEM_ASSIGN(image, NULL);
   egl_thread_set_error(error);
   return egl_image;
}
