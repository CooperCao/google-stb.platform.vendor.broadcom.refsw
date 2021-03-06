/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Implementation of common OpenGL ES 1.1 and 2.0 state machine functions.
=============================================================================*/

#define VCOS_LOG_CATEGORY (&glxx_fbo_log)

#include "interface/khronos/glxx/gl_public_api.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/glxx/glxx_shared.h"

#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/glxx/glxx_server_internal.h"
#include "interface/khronos/tools/dglenum/dglenum.h"

#include "middleware/khronos/glxx/glxx_texture.h"
#include "middleware/khronos/glxx/glxx_buffer.h"
#include "middleware/khronos/glxx/glxx_log.h"
#include "interface/khronos/common/khrn_int_util.h"
#include "middleware/khronos/common/khrn_interlock.h"
#include "middleware/khronos/common/khrn_image.h"

#include "middleware/khronos/egl/egl_image.h"

#include "middleware/khronos/glxx/glxx_hw.h"
#include "middleware/khronos/glxx/glxx_renderbuffer.h"
#include "middleware/khronos/glxx/glxx_framebuffer.h"
#include "middleware/khronos/glxx/glxx_translate.h"
#include "middleware/khronos/common/khrn_image.h"

#include <string.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#include "vcos.h"

#include "helpers/gfx/gfx_lfmt_translate_gl.h"
#include "helpers/v3d/v3d_debug.h"
#include "glxx_utils.h"

unsigned glxx_get_stencil_size(GLXX_SERVER_STATE_T *state)
{
   unsigned res = 0;

   GLXX_FRAMEBUFFER_T *fb = state->bound_draw_framebuffer;
   GLXX_ATTACHMENT_T *att = &fb->attachment[GLXX_STENCIL_ATT];

   GFX_LFMT_T api_fmt = glxx_attachment_get_api_fmt(att);
   if (api_fmt != GFX_LFMT_NONE)
      res = gfx_lfmt_stencil_bits(api_fmt);
   return res;
}

GLXX_FRAMEBUFFER_T *glxx_server_state_get_framebuffer(GLXX_SERVER_STATE_T *state,
      uint32_t fb_id, bool create)
{
   GLXX_FRAMEBUFFER_T *fb = khrn_map_lookup(&state->framebuffers, fb_id);

   if (create && fb == NULL)
   {
      fb = glxx_fb_create(fb_id);

      if (fb)
      {
         bool ok;
         ok = khrn_map_insert(&state->framebuffers, fb_id, fb);
         khrn_mem_release(fb);
         if (!ok)
            fb = NULL;
      }
   }
   return fb;
}

void glxx_server_state_delete_framebuffer(GLXX_SERVER_STATE_T *state, uint32_t fb_id)
{
   khrn_map_delete(&state->framebuffers, fb_id);
}

static bool is_allowed_fb_target(const GLXX_SERVER_STATE_T *state, GLenum target)
{
   switch (target)
   {
   case GL_FRAMEBUFFER:
      return true;
   case GL_READ_FRAMEBUFFER:
   case GL_DRAW_FRAMEBUFFER:
      if (IS_GL_11(state))
         return false;
      return true;
   default:
      return false;
   }
}

static void invalidate_framebuffer(GLenum target, GLsizei
      num_attachments_signed, const GLenum *attachments, bool really)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   GLXX_FRAMEBUFFER_T *fb;
   bool     color_rt[GLXX_MAX_RENDER_TARGETS];
   bool     color    = false;
   bool     depth    = false;
   bool     stencil  = false;
   unsigned int i, num_attachments;

   if (!state)
      return;

   for (i = 0; i < GLXX_MAX_RENDER_TARGETS; ++i)
      color_rt[i] = false;

   if (!is_allowed_fb_target(state, target))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto unlock_out;
   }

   fb = glxx_server_get_bound_fb(state, target);

   if ((num_attachments_signed < 0) || ((attachments == NULL) && (num_attachments_signed > 0)))
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto unlock_out;
   }
   num_attachments = num_attachments_signed;

   if (fb->name == 0)
   {
      for (i = 0; i < num_attachments; i++)
      {
         switch (attachments[i])
         {
         case GL_COLOR: // == 0x1800 == GL_COLOR_EXT
            color_rt[0] = true;
            color = true;
            break;
         case GL_DEPTH: // == 0x1801 == GL_DEPTH_EXT
            depth = true;
            break;
         case GL_STENCIL: // == 0x1802 == GL_STENCIL_EXT
            stencil = true;
            break;
         case GL_DEPTH_STENCIL_ATTACHMENT: // Not supported by the EXT version
            depth = true;
            stencil = true;
            break;
         default:
            glxx_server_state_set_error(state, GL_INVALID_ENUM);
            goto unlock_out;
         }
      }
   }
   else
   {
      // User created FBO
      for (i = 0; i < num_attachments; i++)
      {
         if ( attachments[i] >= GL_COLOR_ATTACHMENT0 &&
              attachments[i] <= GL_COLOR_ATTACHMENT15)
         {
            unsigned int b = attachments[i] - GL_COLOR_ATTACHMENT0;

            if (b >= GLXX_MAX_RENDER_TARGETS)
            {
               glxx_server_state_set_error(state, GL_INVALID_OPERATION);
               goto unlock_out;
            }

            //  The EXT spec only allows GL_COLOR_ATTACHMENT0,
            //  it does not support MRTs as is at revision 7.
            color_rt[b] = true;
            color = true;
         }
         else
         {
            switch (attachments[i])
            {
            case GL_DEPTH_ATTACHMENT:
               depth = true;
               break;
            case GL_STENCIL_ATTACHMENT:
               stencil = true;
               break;
            case GL_DEPTH_STENCIL_ATTACHMENT:
               depth = true;
               stencil = true;
               break;
            default:
               glxx_server_state_set_error(state, GL_INVALID_ENUM);
               goto unlock_out;
            }
         }
      }
   }

   if (num_attachments_signed == 0)
      goto unlock_out;

   if (really)
   {
      if(!glxx_hw_invalidate_frame(state, fb, color_rt, color, color, depth, stencil, NULL))
      {
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
         goto unlock_out;
      }
   }

unlock_out:
   GLXX_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glDiscardFramebufferEXT(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
   invalidate_framebuffer(target, numAttachments, attachments, true);
}

GL_API void GL_APIENTRY glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
   invalidate_framebuffer(target, numAttachments, attachments, true);
}

GL_API void GL_APIENTRY glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments,
   const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
   vcos_log_trace(
      "glInvalidateSubFramebuffer(target = %s, numAttachments = %u, x = %d, y = %d, width = %u, height = %u",
      khrn_glenum_to_str(target), numAttachments, x, y, (unsigned int)width, (unsigned int)height);

   // This does error checking but otherwise nothing
   invalidate_framebuffer(target, numAttachments, attachments, false);

   // TODO - Check if we are about to invalidate whole framebuffer size
#if 0
      GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
      if (state) {
         if ((x == 0) && (y == 0) && (width == fb_width) && (height == fb_height)) {
            invalidate_framebuffer(target, numAttachments, attachments, true);
         }
         GL30_UNLOCK_SERVER_STATE();
      }
#endif
}


/* OES_framebuffer_object for ES 1.1 and core in ES 2.0 and ES 3.0 */

/* check 1.1 constants match their 2.0 equivalents */
static_assrt(GL_FRAMEBUFFER == GL_FRAMEBUFFER_OES);
static_assrt(GL_INVALID_FRAMEBUFFER_OPERATION == GL_INVALID_FRAMEBUFFER_OPERATION_OES);
static_assrt(GL_FRAMEBUFFER_BINDING == GL_FRAMEBUFFER_BINDING_OES);
static_assrt(GL_RENDERBUFFER_BINDING == GL_RENDERBUFFER_BINDING_OES);
static_assrt(GL_MAX_RENDERBUFFER_SIZE == GL_MAX_RENDERBUFFER_SIZE_OES);
static_assrt(GL_COLOR_ATTACHMENT0 == GL_COLOR_ATTACHMENT0_OES);
static_assrt(GL_DEPTH_ATTACHMENT == GL_DEPTH_ATTACHMENT_OES);
static_assrt(GL_STENCIL_ATTACHMENT == GL_STENCIL_ATTACHMENT_OES);
static_assrt(GL_RENDERBUFFER == GL_RENDERBUFFER_OES);
static_assrt(GL_DEPTH_COMPONENT16 == GL_DEPTH_COMPONENT16_OES);
static_assrt(GL_RGBA4 == GL_RGBA4_OES);
static_assrt(GL_RGB5_A1 == GL_RGB5_A1_OES);
static_assrt(GL_RGB565 == GL_RGB565_OES);
static_assrt(GL_STENCIL_INDEX8 == GL_STENCIL_INDEX8_OES);
static_assrt(GL_RENDERBUFFER_WIDTH == GL_RENDERBUFFER_WIDTH_OES);
static_assrt(GL_RENDERBUFFER_HEIGHT == GL_RENDERBUFFER_HEIGHT_OES);
static_assrt(GL_RENDERBUFFER_INTERNAL_FORMAT == GL_RENDERBUFFER_INTERNAL_FORMAT_OES);
static_assrt(GL_RENDERBUFFER_RED_SIZE == GL_RENDERBUFFER_RED_SIZE_OES);
static_assrt(GL_RENDERBUFFER_GREEN_SIZE == GL_RENDERBUFFER_GREEN_SIZE_OES);
static_assrt(GL_RENDERBUFFER_BLUE_SIZE == GL_RENDERBUFFER_BLUE_SIZE_OES);
static_assrt(GL_RENDERBUFFER_ALPHA_SIZE == GL_RENDERBUFFER_ALPHA_SIZE_OES);
static_assrt(GL_RENDERBUFFER_DEPTH_SIZE == GL_RENDERBUFFER_DEPTH_SIZE_OES);
static_assrt(GL_RENDERBUFFER_STENCIL_SIZE == GL_RENDERBUFFER_STENCIL_SIZE_OES);
static_assrt(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE == GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_OES);
static_assrt(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME == GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_OES);
static_assrt(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL == GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_OES);
static_assrt(GL_NONE == GL_NONE_OES);
static_assrt(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_OES);
static_assrt(GL_FRAMEBUFFER_UNSUPPORTED == GL_FRAMEBUFFER_UNSUPPORTED_OES);
static_assrt(GL_FRAMEBUFFER_UNSUPPORTED == GL_FRAMEBUFFER_UNSUPPORTED_OES);
static_assrt(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_OES);
static_assrt(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_OES);

static bool is_valid_attachment(GLXX_SERVER_STATE_T *state, GLenum attachment)
{
   if ((attachment >= GL_COLOR_ATTACHMENT0) &&
        (attachment < GL_COLOR_ATTACHMENT0 + GLXX_MAX_RENDER_TARGETS))
      return true;
   if (attachment == GL_DEPTH_ATTACHMENT)
      return true;
   if (attachment == GL_STENCIL_ATTACHMENT)
      return true;
   if (!IS_GL_11(state) && (attachment == GL_DEPTH_STENCIL_ATTACHMENT))
      return true;
   return false;
}

GL_API void GL_APIENTRY glDrawBuffers(GLsizei n_signed, const GLenum* bufs)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   unsigned int i, n;
   GLXX_FRAMEBUFFER_T *fb;
   GLenum error = GL_NO_ERROR;

   if (!state) return;

   if (n_signed < 1 ||
       n_signed > GLXX_MAX_RENDER_TARGETS)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }
   n = n_signed;

   fb = state->bound_draw_framebuffer;
   if (fb->name == 0)
   {
      if (n != 1 || (bufs[0] != GL_NONE && bufs[0] != GL_BACK))
      {
         error = GL_INVALID_OPERATION;
         goto end;
      }
   }
   else
   {
      for (i = 0; i < n; i++)
      {
         if (bufs[i] != GL_NONE &&
             bufs[i] != GL_BACK &&
             (bufs[i] < GL_COLOR_ATTACHMENT0 ||
              bufs[i] >= (GL_COLOR_ATTACHMENT0 + GLXX_MAX_RENDER_TARGETS)))
         {
            error = GL_INVALID_ENUM;
            goto end;
         }
         /* the ith argument value must be either GL_NONE or GL_COLOR_ATTACHMENTi*/
         if (bufs[i] != GL_NONE &&
             bufs[i] != (GL_COLOR_ATTACHMENT0 + i))
         {
            error = GL_INVALID_OPERATION;
            goto end;
         }
      }
   }

   state->dirty.color_write = KHRN_RENDER_STATE_SET_ALL;

   for (i = 0; i < n; i++)
   {
      if (bufs[i] != GL_NONE)
         fb->draw_buffer[i] = true;
      else
         fb->draw_buffer[i] = false;
   }
   for ( ; i < GLXX_MAX_RENDER_TARGETS; i++)
         fb->draw_buffer[i] = false;
end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   GL30_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glReadBuffer(GLenum src)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE_UNCHANGED();
   GLXX_FRAMEBUFFER_T *fb;
   GLenum error = GL_NO_ERROR;

   if (!state) return;

   fb = state->bound_read_framebuffer;

   if (src == GL_NONE)
   {
      fb->read_buffer = GLXX_INVALID_ATT;
      goto end;
   }

   // GL_INVALID_ENUM is generated if src is not GL_BACK, GL_NONE, or GL_COLOR_ATTACHMENTi
   // (max possible value of i is 31). Note: the check for GL_COLOR_ATTACHMENT0 + GLXX_MAX_RENDER_TARGETS
   // appears lower down and gives GL_INVALID_OPERATION instead.
   if (src != GL_BACK && (src < GL_COLOR_ATTACHMENT0 ||
                          src > (GL_COLOR_ATTACHMENT0 + 31)))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   if (fb->name == 0)
   {
      if (src != GL_BACK)
      {
         error = GL_INVALID_OPERATION;
         goto end;
      }
      fb->read_buffer = GLXX_COLOR0_ATT;
   }
   else
   {
      if (src < GL_COLOR_ATTACHMENT0 ||
          src >= (GL_COLOR_ATTACHMENT0 + GLXX_MAX_RENDER_TARGETS))
      {
         error = GL_INVALID_OPERATION;
         goto end;
      }

      fb->read_buffer = GLXX_COLOR0_ATT + (src - GL_COLOR_ATTACHMENT0);
   }

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   GL30_UNLOCK_SERVER_STATE();
}

GL_API GLboolean GL_APIENTRY glIsRenderbuffer(GLuint renderbuffer)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();
   GLboolean result;
   if (!state)
      return 0;

   result = glxx_shared_get_renderbuffer(state->shared, renderbuffer, false) != NULL;

   GLXX_UNLOCK_SERVER_STATE();

   return result;
}

/*
   glBindRenderbuffer()

   Called with target set to RENDERBUFFER and renderbuffer set to the unused name. If renderbuffer is not zero,
   then the resulting renderbuffer object is a new state vector, initialized with a zero-sized memory buffer, and
   comprising the state values listed in table 6.32. Any previous binding to target is broken.

   BindRenderbuffer may also be used to bind an existing renderbuffer object. If the bind is successful, no
   change is made to the state of the newly bound renderbuffer object, and any previous binding to target is
   broken.
*/
GL_API void GL_APIENTRY glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   GLXX_RENDERBUFFER_T *rb = NULL;

   if (!state) return;

   if (target != GL_RENDERBUFFER) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (renderbuffer)
   {
      rb = glxx_shared_get_renderbuffer(state->shared, renderbuffer, true);

      if (rb == NULL)
      {
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
         goto end;
      }
   }

   KHRN_MEM_ASSIGN(state->bound_renderbuffer, rb);

end:
   GLXX_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   int i;
   if (!state) return;

   if (n < 0) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   for (i = 0; i < n; i++)
      if (renderbuffers[i])
      {
         GLXX_RENDERBUFFER_T *rb = glxx_shared_get_renderbuffer(state->shared, renderbuffers[i], false);

         if (rb != NULL)
         {
            if (state->bound_renderbuffer == rb)
               KHRN_MEM_ASSIGN(state->bound_renderbuffer, NULL);

            glxx_fb_detach_renderbuffer(state->bound_draw_framebuffer, rb);
            glxx_fb_detach_renderbuffer(state->bound_read_framebuffer, rb);

            glxx_shared_delete_renderbuffer(state->shared, renderbuffers[i]);
         }
      }

end:
   GLXX_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glGenRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();

   int32_t i = 0;

   if (!state) return;

   if (n < 0)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);   // The conformance tests insist...
   else if (renderbuffers)
      while (i < n)
      {
         if (glxx_shared_get_renderbuffer(state->shared, state->shared->next_renderbuffer, false)
               == NULL)
            renderbuffers[i++] = state->shared->next_renderbuffer;

         state->shared->next_renderbuffer++;
      }

   GLXX_UNLOCK_SERVER_STATE();
}


static void renderbuffer_storage(GLenum target, GLsizei samples,
      GLenum internalformat, GLsizei width, GLsizei height)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   bool renderable_format;
   GLenum error = GL_NO_ERROR;

   if (state == NULL)
      return;


   if (target != GL_RENDERBUFFER)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   if (state->bound_renderbuffer == NULL)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   renderable_format =
      glxx_is_color_renderable_internalformat(internalformat) ||
      glxx_is_depth_renderable_internalformat(internalformat) ||
      glxx_is_stencil_renderable_internalformat(internalformat);
   if (!renderable_format)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   if (width < 0 || width > GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE ||
       height < 0 || height > GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE )
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (samples < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   glxx_ms_mode max_ms_mode = glxx_max_ms_mode_for_internalformat(internalformat);
   // NOTE: gcc and MSVC disagree on whether glxx_ms_mode is signed or not
   if ((unsigned)samples > (unsigned)max_ms_mode)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   glxx_ms_mode ms_mode = glxx_samples_to_ms_mode(samples);

   if (!glxx_renderbuffer_storage(state->bound_renderbuffer, ms_mode,
            internalformat, width, height))
   {
      error = GL_OUT_OF_MEMORY;
   }
end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   GLXX_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glRenderbufferStorage(GLenum target, GLenum
      internalformat, GLsizei width, GLsizei height)
{
   renderbuffer_storage(target, 0, internalformat, width, height);
}

GL_API void GL_APIENTRY glRenderbufferStorageMultisampleEXT(GLenum target,
      GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
   renderbuffer_storage(target, samples, internalformat, width, height);
}

GL_API void GL_APIENTRY glRenderbufferStorageMultisample(GLenum target, GLsizei
      samples, GLenum internalformat, GLsizei width, GLsizei height)
{
   renderbuffer_storage(target, samples, internalformat, width, height);
}

GL_API void GL_APIENTRY glGetRenderbufferParameteriv(GLenum target,
      GLenum pname, GLint* params)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();
   KHRN_IMAGE_T *image;

   if (!state)
      return;

   if (target != GL_RENDERBUFFER)
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto out;
   }

   if (state->bound_renderbuffer == NULL)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto out;
   }

   image = state->bound_renderbuffer ? state->bound_renderbuffer->image : NULL;

   switch (pname)
   {
   case GL_RENDERBUFFER_WIDTH:
      params[0] = image ? khrn_image_get_width(image) : 0;
      break;
   case GL_RENDERBUFFER_HEIGHT:
      params[0] = image ? khrn_image_get_height(image) : 0;
      break;
   case GL_RENDERBUFFER_INTERNAL_FORMAT:
      params[0] = GL_RGBA4;
      if (image)
      {
         params[0] = gfx_sized_internalformat_from_api_fmt_maybe(image->api_fmt);
         assert(params[0] != GL_NONE);
      }
      break;
   case GL_RENDERBUFFER_RED_SIZE:
      params[0] = image ? gfx_lfmt_red_bits(image->api_fmt) : 0;
      break;
   case GL_RENDERBUFFER_SAMPLES:
      /* number of samples of the image currently bound to renderbuffer */
      params[0] = image ? state->bound_renderbuffer->ms_mode : GLXX_NO_MS;
      break;
   case GL_RENDERBUFFER_GREEN_SIZE:
      params[0] = image ? gfx_lfmt_green_bits(image->api_fmt) : 0;
      break;
   case GL_RENDERBUFFER_BLUE_SIZE:
      params[0] = image ? gfx_lfmt_blue_bits(image->api_fmt) : 0;
      break;
   case GL_RENDERBUFFER_ALPHA_SIZE:
      params[0] = image ? gfx_lfmt_alpha_bits(image->api_fmt) : 0;
      break;
   case GL_RENDERBUFFER_DEPTH_SIZE:
      params[0] = image ? gfx_lfmt_depth_bits(image->api_fmt) : 0;
      break;
   case GL_RENDERBUFFER_STENCIL_SIZE:
      params[0] = image ? gfx_lfmt_stencil_bits(image->api_fmt) : 0;
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

out:
   GLXX_UNLOCK_SERVER_STATE();
}

GL_API GLboolean GL_APIENTRY glIsFramebuffer(GLuint fb_id)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();
   GLboolean result;

   if (!state)
      return 0;

   if (fb_id == 0)
      result = 0;
   else
      result = glxx_server_state_get_framebuffer(state, fb_id, false) != NULL;

   GLXX_UNLOCK_SERVER_STATE();

   return result;

}
GL_API void GL_APIENTRY glBindFramebuffer(GLenum target, GLuint fb_id)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   GLXX_FRAMEBUFFER_T *fb_read, *fb_draw;
   GLenum error = GL_NO_ERROR;

   if (!state)
      return;

   if (!is_allowed_fb_target(state, target))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   if (fb_id == 0)
   {
       fb_read = state->default_framebuffer[GLXX_READ_SURFACE];
       fb_draw = state->default_framebuffer[GLXX_DRAW_SURFACE];
   }
   else
   {
      fb_read = fb_draw = glxx_server_state_get_framebuffer(state, fb_id, true);
      if (fb_read == NULL)
      {
         error = GL_OUT_OF_MEMORY;
         goto end;
      }
   }

   switch(target)
   {
      case GL_FRAMEBUFFER:
         KHRN_MEM_ASSIGN(state->bound_read_framebuffer, fb_read);
         KHRN_MEM_ASSIGN(state->bound_draw_framebuffer, fb_draw);
         break;
      case GL_DRAW_FRAMEBUFFER:
         KHRN_MEM_ASSIGN(state->bound_draw_framebuffer, fb_draw);
         break;
      case GL_READ_FRAMEBUFFER:
         KHRN_MEM_ASSIGN(state->bound_read_framebuffer, fb_read);
         break;
      default:
         assert(0);
   }

#if defined(GLXX_FORCE_FLUSH_ON_BINDFRAMEBUFFER)
   khrn_hw_common_flush();
   khrn_hw_common_wait();
#endif

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   GLXX_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   GLenum error = GL_NO_ERROR;
   GLsizei i;

   if (!state)
      return;

   if (n < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }
   if (!framebuffers)
      goto end;

   i = 0;
   for (i = 0; i < n; i++)
   {
      GLXX_FRAMEBUFFER_T *fb;

      if (!framebuffers[i])
         continue;

      fb = glxx_server_state_get_framebuffer(state, framebuffers[i], false);

      if (fb != NULL)
      {
         if (fb->name == 0)
            continue;

         if (state->bound_draw_framebuffer == fb)
            KHRN_MEM_ASSIGN(state->bound_draw_framebuffer,
                  state->default_framebuffer[GLXX_DRAW_SURFACE]);

         if (state->bound_read_framebuffer == fb)
            KHRN_MEM_ASSIGN(state->bound_read_framebuffer,
                  state->default_framebuffer[GLXX_READ_SURFACE]);

         glxx_server_state_delete_framebuffer(state, framebuffers[i]);
      }
   }
end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   GLXX_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glGenFramebuffers(GLsizei n, GLuint *framebuffers)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();
   GLsizei i = 0;
   GLenum error = GL_NO_ERROR;

   if (!state)
      return;

   if (n < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (!framebuffers)
      goto end;

   i = 0;
   while (i < n)
   {
      if (glxx_server_state_get_framebuffer(state, state->next_framebuffer, false) == NULL)
         framebuffers[i++] = state->next_framebuffer;

      state->next_framebuffer++;
   }
end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   GLXX_UNLOCK_SERVER_STATE();
}

GL_API GLenum GL_APIENTRY glCheckFramebufferStatus(GLenum target)
{
   GLXX_FRAMEBUFFER_T *fb;
   GLenum result = 0; // glCheckFramebufferStatus man page: If an error occurs, zero is returned.

   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();
   if (!state) return 0;

   if (!is_allowed_fb_target(state, target))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   fb = glxx_server_get_bound_fb(state, target);
   result = glxx_fb_completeness_status(fb);

end:
   GLXX_UNLOCK_SERVER_STATE();
   return result;
}

static bool is_supported_fb_tex_target(const GLXX_SERVER_STATE_T *state,
      GLenum tex_target)
{
   switch (tex_target)
   {
   case GL_TEXTURE_2D:
       return true;
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
   case GL_TEXTURE_3D:
   case GL_TEXTURE_2D_ARRAY:
      return !IS_GL_11(state);
   case GL_TEXTURE_2D_MULTISAMPLE:
      return KHRN_GLES31_DRIVER ? !IS_GL_11(state) : false;
   default:
       return false;
   }
}

static void  framebuffer_texture(GLenum target, GLenum att_e, bool textarget_valid,
      GLenum textarget, GLuint tex_id, GLint level, GLint layer, GLsizei
      samples)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   GLXX_FRAMEBUFFER_T *fb;
   GLXX_TEXTURE_T *texture;
   enum glxx_tex_target temp_target;
   glxx_attachment_point_t att_point;
   unsigned face;
   GLenum error = GL_NO_ERROR;

   if (!state)
      return;

   if (!is_allowed_fb_target(state, target))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   if (!is_valid_attachment(state, att_e))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }
   att_point = att_e;

   fb = glxx_server_get_bound_fb(state, target);
   if (fb->name == 0)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   if (samples < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (samples > GLXX_CONFIG_MAX_SAMPLES)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (tex_id == 0)
   {
      /* = detach */
      glxx_fb_detach_attachment(fb, att_point);
      goto end;
   }

   texture = glxx_shared_get_texture(state->shared, tex_id);

   if (texture == NULL)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   if (!textarget_valid)
   {
      textarget = texture->target;

      if (textarget != GL_TEXTURE_3D && textarget != GL_TEXTURE_2D_ARRAY)
      {
         /* glFramebufferTextureLayer: INVALID_OPERATION is generated if texture is non-zero
          * and is not the name of a three-dimensional texture or two-dimensional array texture. */
         error = GL_INVALID_OPERATION;
         goto end;
      }
   }

   if (!is_supported_fb_tex_target(state, textarget))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   if (glxx_texture_is_cube_face(textarget))
      temp_target = GL_TEXTURE_CUBE_MAP;
   else
      temp_target = textarget;

   if (texture->target != temp_target)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   if (!glxx_texture_is_legal_level(textarget, level))
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if ((texture->target == GL_TEXTURE_3D || texture->target == GL_TEXTURE_2D_ARRAY) &&
        !glxx_texture_is_legal_layer(texture->target, layer))
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   face = glxx_texture_get_face(textarget);
   glxx_ms_mode ms_mode = glxx_samples_to_ms_mode(samples);

   glxx_fb_attach_texture(fb, att_point, texture, face, level, layer, ms_mode);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   GLXX_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glFramebufferTexture2DMultisampleEXT(GLenum target,
   GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples)
{
   if (attachment != GL_COLOR_ATTACHMENT0)
   {
      GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
      /* not sure what error to return; the spec says "attachment must be
       * COLOR_ATTACHMENT0" */
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      GLXX_UNLOCK_SERVER_STATE();
      return;
   }
   framebuffer_texture(target, attachment, true, textarget, texture, level, 0,
      samples);
}

GL_API void GL_APIENTRY glFramebufferTexture2D(GLenum target, GLenum
   attachment, GLenum textarget, GLuint texture, GLint level)
{
   framebuffer_texture(target, attachment, true, textarget, texture,
      level, 0, 0);
}

GL_API void GL_APIENTRY glFramebufferTextureLayer(GLenum target, GLenum
      att_e, GLuint texture, GLint level, GLint layer)
{
   framebuffer_texture(target, att_e, false, GL_NONE, texture, level,
         layer, 0);
}

GL_API void GL_APIENTRY glFramebufferRenderbuffer(GLenum target, GLenum att_e,
      GLenum rb_target, GLuint rb_id)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   GLXX_FRAMEBUFFER_T *fb;
   GLXX_RENDERBUFFER_T *rb;
   GLenum error = GL_NO_ERROR;
   glxx_attachment_point_t att_point;

   if (!state)
      return;
   if (!is_allowed_fb_target(state, target))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   if (!is_valid_attachment(state, att_e))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }
   att_point = att_e;

   if (rb_target != GL_RENDERBUFFER)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   fb = glxx_server_get_bound_fb(state, target);
   if (fb->name == 0)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   if (rb_id == 0)
   {
      glxx_fb_detach_attachment(fb, att_point);
      goto end;
   }

   rb = glxx_shared_get_renderbuffer(state->shared, rb_id, false);
   if (rb == NULL)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   glxx_fb_attach_renderbuffer(fb, att_point, rb);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   GLXX_UNLOCK_SERVER_STATE();
}

static bool attachment_parameter_common(GFX_LFMT_T api_fmt,
   GLenum pname, GLint *params)
{
   bool result = true;

   switch (pname)
   {
   case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
      if (api_fmt == GFX_LFMT_NONE)
         params[0] = 0;
      else
         params[0] = gfx_lfmt_red_bits(api_fmt);
      break;
   case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
      if (api_fmt == GFX_LFMT_NONE)
         params[0] = 0;
      else
         params[0] = gfx_lfmt_green_bits(api_fmt);
      break;
   case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
      if (api_fmt == GFX_LFMT_NONE)
         params[0] = 0;
      else
         params[0] = gfx_lfmt_blue_bits(api_fmt);
      break;
   case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
      if (api_fmt == GFX_LFMT_NONE)
         params[0] = 0;
      else
         params[0] = gfx_lfmt_alpha_bits(api_fmt);
      break;
   case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
      if (api_fmt == GFX_LFMT_NONE)
         params[0] = 0;
      else
         params[0] = gfx_lfmt_depth_bits(api_fmt);
      break;
   case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
      if (api_fmt == GFX_LFMT_NONE)
         params[0] = 0;
      else
         params[0] = gfx_lfmt_stencil_bits(api_fmt);
      break;
   case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
      if (api_fmt == GFX_LFMT_NONE)
         params[0] = GL_NONE;
      else
      {
         switch (gfx_lfmt_get_type(&api_fmt))
         {
         case GFX_LFMT_TYPE_UFLOAT:
         case GFX_LFMT_TYPE_FLOAT:
            params[0] = GL_FLOAT;
            break;
         case GFX_LFMT_TYPE_UINT:
            params[0] = GL_UNSIGNED_INT;
            break;
         case GFX_LFMT_TYPE_INT:
            params[0] = GL_INT;
            break;
         case GFX_LFMT_TYPE_UNORM:
         case GFX_LFMT_TYPE_SRGB:
         case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
            params[0] = GL_UNSIGNED_NORMALIZED;
            break;
         case GFX_LFMT_TYPE_SNORM:
            params[0] = GL_SIGNED_NORMALIZED;
            break;
         default:
            result = false;
            break;
         }
      }
      break;
   case GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING:
      if (api_fmt == GFX_LFMT_NONE)
         params[0] = GL_LINEAR;
      else
      {
         if (gfx_lfmt_contains_srgb(api_fmt))
            params[0] = GL_SRGB;
         else
            params[0] = GL_LINEAR;
      }
      break;
   default:
      result = false;
      break;
   }
   return result;
}

GL_API void GL_APIENTRY glGetFramebufferAttachmentParameteriv(GLenum target,
      GLenum att_e, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();
   GLXX_FRAMEBUFFER_T *fb;
   glxx_attachment_point_t att_point;
   GLenum error = GL_NO_ERROR;

   if (!state)
      return;

   if (!is_allowed_fb_target(state, target))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   fb = glxx_server_get_bound_fb(state, target);

   if (fb->name == 0)
   {
      if (IS_GL_11(state))
      {
         /* according to the spec, we should report invalid operation for es2.0 too */
         error  = GL_INVALID_OPERATION;
         goto end;
      }
      switch(att_e)
      {
         case GL_BACK:
            att_point = GL_COLOR_ATTACHMENT0;
            break;
         case GL_DEPTH:
            att_point = GL_DEPTH_ATTACHMENT;
            break;
         case GL_STENCIL:
            att_point = GL_STENCIL_ATTACHMENT;
            break;
         default:
            error = GL_INVALID_OPERATION;
            goto end;
      }
   }
   else
   {
      if (!is_valid_attachment(state, att_e))
      {
         error = GL_INVALID_ENUM;
         goto end;
      }
      att_point = att_e;
   }

   const GLXX_ATTACHMENT_T *att = glxx_fb_get_attachment(fb, att_point);
   if (att_point == GL_DEPTH_STENCIL_ATTACHMENT)
   {
      GLXX_ATTACHMENT_T *stencil_att;
      if (pname == GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE)
      {
         error = GL_INVALID_OPERATION;
         goto end;
      }
      stencil_att = &fb->attachment[GLXX_STENCIL_ATT];
      /* same objects must be attached to depth and stencil */
      if (!glxx_attachment_equal(att, stencil_att))
      {
         error = GL_INVALID_OPERATION;
         goto end;
      }
   }

   if (!params)
      goto end;

   switch(pname)
   {
      case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
         params[0] = att->obj_type;
         goto end;
      case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
         switch (att->obj_type)
         {
            case GL_NONE:
            case GL_TEXTURE:
            case GL_RENDERBUFFER:
               params[0] = glxx_attachment_get_name(att);
               goto end;
            case GL_FRAMEBUFFER_DEFAULT:
               error = GL_INVALID_ENUM;
               goto end;
            default:
               unreachable();
               goto end;
         }
      default:
         break;
   }

   if (att->obj_type == GL_NONE)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   if (att->obj_type == GL_TEXTURE)
   {
      const struct texture_info *tex_info = &att->obj.tex_info;
      switch(pname)
      {
         case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
            params[0] = tex_info->layer;
            goto end;
         case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
            params[0] = tex_info->level;
            goto end;
         case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
            if (tex_info->texture->target == GL_TEXTURE_CUBE_MAP)
              params[0] = GL_TEXTURE_CUBE_MAP_POSITIVE_X + tex_info->face;
            else
              params[0] = 0;
            goto end;
         default:
            break;
      }
   }

   GFX_LFMT_T api_fmt = glxx_attachment_get_api_fmt(att);
   if (!attachment_parameter_common(api_fmt, pname, params))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   GLXX_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glFramebufferParameteri(GLenum target,
      GLenum pname, GLint param)
{
   GLXX_SERVER_STATE_T *state = GL31_LOCK_SERVER_STATE();
   GLXX_FRAMEBUFFER_T *fb = NULL;
   GLenum error = GL_NO_ERROR;

   if (!state)
      return;

   if (!is_allowed_fb_target(state, target))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   fb = glxx_server_get_bound_fb(state, target);
   if (fb->name == 0)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   if (param < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   switch (pname)
   {
   case GL_FRAMEBUFFER_DEFAULT_WIDTH:
      if (param > GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE)
      {
         error = GL_INVALID_VALUE;
         goto end;
      }
      fb->default_width = param;
      break;
   case GL_FRAMEBUFFER_DEFAULT_HEIGHT:
      if (param > GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE)
      {
         error = GL_INVALID_VALUE;
         goto end;
      }
      fb->default_height = param;
      break;
   case GL_FRAMEBUFFER_DEFAULT_SAMPLES:
      if (param > GLXX_CONFIG_MAX_SAMPLES)
      {
         error = GL_INVALID_VALUE;
         goto end;
      }
      fb->default_samples = param;
      fb->default_ms_mode = glxx_samples_to_ms_mode(fb->default_samples);
      break;
   case GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS:
      fb->default_fixed_sample_locations = (param != 0);
      break;
   default:
      error = GL_INVALID_ENUM;
      goto end;
   }
end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   GL31_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glGetFramebufferParameteriv(GLenum target,
      GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state = GL31_LOCK_SERVER_STATE();
   GLXX_FRAMEBUFFER_T *fb = NULL;
   GLenum error = GL_NO_ERROR;

   if (!state)
      return;
   if (!is_allowed_fb_target(state, target))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   fb = glxx_server_get_bound_fb(state, target);
   if (fb->name == 0)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   switch (pname)
   {
   case GL_FRAMEBUFFER_DEFAULT_WIDTH:
      params[0] = fb->default_width;
      break;

   case GL_FRAMEBUFFER_DEFAULT_HEIGHT:
      params[0] = fb->default_height;
      break;

   case GL_FRAMEBUFFER_DEFAULT_SAMPLES:
      params[0] = fb->default_samples;
      break;

   case GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS:
      params[0] = fb->default_fixed_sample_locations;
      break;

   default:
      error = GL_INVALID_ENUM;
      goto end;
   }

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   GL31_UNLOCK_SERVER_STATE();
}
