/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Implementation of OpenGL ES 2.0 framebuffer / Open GL ES 1.1 OES_framebuffer_object structure.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"

#include "middleware/khronos/glxx/glxx_framebuffer.h"
#include "middleware/khronos/glxx/glxx_renderbuffer.h"

#include "middleware/khronos/glxx/glxx_texture.h"

static void attachment_info_init(GLXX_ATTACHMENT_INFO_T *attachment)
{
   vcos_assert(attachment);

   /*
      we never re-init a program structure, so this
      should be shiny and new
   */

   vcos_assert(attachment->mh_object == MEM_INVALID_HANDLE);

   attachment->type = GL_NONE;
   attachment->target = 0;
   attachment->level = 0;
}

void glxx_framebuffer_init(GLXX_FRAMEBUFFER_T *framebuffer, int32_t name)
{
   vcos_assert(framebuffer);

   framebuffer->name = name;

   attachment_info_init(&framebuffer->attachments.color);
   attachment_info_init(&framebuffer->attachments.depth);
   attachment_info_init(&framebuffer->attachments.stencil);
}

static void attachment_info_term(GLXX_ATTACHMENT_INFO_T *attachment)
{
   vcos_assert(attachment);

   // If it is a texture 2D and it is a multisample attachment
   if ((attachment->target == GL_TEXTURE_2D) && (attachment->samples != 0))
   {
      // Delete the multisample buffer allocated when attached
      GLXX_TEXTURE_T *texture = (GLXX_TEXTURE_T *)mem_lock(attachment->mh_object, NULL);
      if ((texture) && (texture->mh_ms_image != MEM_INVALID_HANDLE))
         MEM_ASSIGN(texture->mh_ms_image, MEM_INVALID_HANDLE);

      mem_unlock(attachment->mh_object);
   }
   MEM_ASSIGN(attachment->mh_object, MEM_INVALID_HANDLE);
}

void glxx_framebuffer_term(void *v, uint32_t size)
{
   GLXX_FRAMEBUFFER_T *framebuffer = (GLXX_FRAMEBUFFER_T *)v;
   UNUSED(size);

   attachment_info_term(&framebuffer->attachments.color);
   attachment_info_term(&framebuffer->attachments.depth);
   attachment_info_term(&framebuffer->attachments.stencil);
}

vcos_static_assert(GL_TEXTURE_CUBE_MAP_NEGATIVE_X == GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1);
vcos_static_assert(GL_TEXTURE_CUBE_MAP_POSITIVE_Y == GL_TEXTURE_CUBE_MAP_NEGATIVE_X + 1);
vcos_static_assert(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y == GL_TEXTURE_CUBE_MAP_POSITIVE_Y + 1);
vcos_static_assert(GL_TEXTURE_CUBE_MAP_POSITIVE_Z == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y + 1);
vcos_static_assert(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z == GL_TEXTURE_CUBE_MAP_POSITIVE_Z + 1);
MEM_HANDLE_T glxx_attachment_info_get_image(GLXX_ATTACHMENT_INFO_T *attachment)
{
   MEM_HANDLE_T result = MEM_INVALID_HANDLE;

   switch (attachment->type) {
   case GL_NONE:
      result = MEM_INVALID_HANDLE;
      break;
   case GL_TEXTURE:
   {
      GLXX_TEXTURE_T *texture = (GLXX_TEXTURE_T *)mem_lock(attachment->mh_object, NULL);

      switch (attachment->target) {
      case GL_TEXTURE_2D:
         result = glxx_texture_share_mipmap(texture, TEXTURE_BUFFER_TWOD, attachment->level);
         break;
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
         vcos_assert(attachment->level == 0);

         result = glxx_texture_share_mipmap(texture, TEXTURE_BUFFER_POSITIVE_X + attachment->target - GL_TEXTURE_CUBE_MAP_POSITIVE_X, attachment->level);
         break;
      default:
         UNREACHABLE();
      }
      mem_unlock(attachment->mh_object);
      break;
   }
   case GL_RENDERBUFFER:
   {
      GLXX_RENDERBUFFER_T *renderbuffer = (GLXX_RENDERBUFFER_T *)mem_lock(attachment->mh_object, NULL);

      result = renderbuffer->mh_storage;
      mem_unlock(attachment->mh_object);
      break;
   }
   default:
      UNREACHABLE();
   }

   return result;
}

MEM_HANDLE_T glxx_attachment_info_get_ms_image(GLXX_ATTACHMENT_INFO_T *attachment)
{
   MEM_HANDLE_T result = MEM_INVALID_HANDLE;

   switch (attachment->type) {
   case GL_NONE:
      result = MEM_INVALID_HANDLE;
      break;
   case GL_TEXTURE:
   {
      GLXX_TEXTURE_T *texture = (GLXX_TEXTURE_T *)mem_lock(attachment->mh_object, NULL);

      switch (attachment->target) {
      case GL_TEXTURE_2D:
         result = texture->mh_ms_image;
         break;
      default:
         UNREACHABLE();
      }

      mem_unlock(attachment->mh_object);
   }
   break;
   case GL_RENDERBUFFER:
   {
      GLXX_RENDERBUFFER_T *renderbuffer = (GLXX_RENDERBUFFER_T *)mem_lock(attachment->mh_object, NULL);

      result = renderbuffer->mh_ms_storage;
      mem_unlock(attachment->mh_object);
   }
   break;
   default:
      UNREACHABLE();
   }

   return result;
}

bool glxx_framebuffer_hw_support(KHRN_IMAGE_FORMAT_T format)
{
#ifdef BIG_ENDIAN_CPU
   // one of the suppored formats
   return ((format == RGBA_8888_TF) ||
      (format == RGBX_8888_TF) ||
#else
   return ((format == ABGR_8888_TF) ||
      (format == XBGR_8888_TF) ||
#endif
      (format == RGB_565_TF) ||
      (format == DEPTH_32_TLBD) ||
      (format == DEPTH_COL_64_TLBD));
}

/*
   The framebuffer attachment point attachment is said to be framebuffer attachment complete if the value
   of FRAMEBUFFER ATTACHMENT OBJECT TYPE for attachment is NONE (i.e., no image is attached), or if all
   of the following conditions are true:

   - <image> is a component of an existing object with the name specified by FRAMEBUFFER ATTACHMENT -
     OBJECT NAME, and of the type specified by FRAMEBUFFER ATTACHMENT OBJECT TYPE.
   - The width and height of <image> must be non-zero.
   - If attachment is COLOR ATTACHMENT0, then image must have a color-renderable internal format.
   - If attachment is DEPTH ATTACHMENT, then image must have a depth-renderable internal format.
   - If attachment is STENCIL ATTACHMENT, then image must have a stencil-renderable internal format.

   The framebuffer object target is said to be framebuffer complete if it is the window-system-provided
   framebuffer, or if all the following conditons are true:

   - All framebuffer attachment points are framebuffer attachment complete. FRAMEBUFFER INCOMPLETE ATTACHMENT
   - There is at least one image attached to the framebuffer. FRAMEBUFFER INCOMPLETE MISSING ATTACHMENT
   - All attached images have the same width and height. FRAMEBUFFER INCOMPLETE DIMENSIONS
   - The combination of internal formats of the attached images does not violate an implementationdependent
     set of restrictions. FRAMEBUFFER UNSUPPORTED

   The enums in bold after each clause of the framebuffer completeness rules specifies the return value of
   CheckFramebufferStatus that is generated when that clause is violated. If more than one clause is violated,
   it is implementation-dependent as to exactly which enum will be returned by CheckFramebufferStatus.
*/

typedef enum
{
   ATTACHMENT_MISSING,                    // Attachment type is NONE, therefore complete
   ATTACHMENT_BROKEN,                     // Attachment is incomplete
   ATTACHMENT_COLOR,                      // Attachment type is COLOR and is complete
   ATTACHMENT_DEPTH16,                    // Attachment type is DEPTH, is complete and the image is 16bpp
   ATTACHMENT_DEPTH24,                    // Attachment type is DEPTH, is complete and the image is 24bpp
   ATTACHMENT_STENCIL,                    // Attachment type is STENCIL and is complete
   ATTACHMENT_DEPTH_STENCIL,              // Attachment type is DEPTH_STENCIL and is complete
   ATTACHMENT_UNSUPPORTED = 0xFFFFFFFF    // Attachment type is valid but rendering is not supported
} ATTACHMENT_STATUS_T;

static ATTACHMENT_STATUS_T attachment_get_status(GLXX_ATTACHMENT_INFO_T *attachment,
   uint32_t *width, uint32_t *height, uint32_t *samples, bool *secure)
{
   switch (attachment->type) {
   case GL_NONE:
      return ATTACHMENT_MISSING;
   case GL_RENDERBUFFER:
   {
      GLXX_RENDERBUFFER_T *renderbuffer = (GLXX_RENDERBUFFER_T *)mem_lock(attachment->mh_object, NULL);
      ATTACHMENT_STATUS_T result = ATTACHMENT_BROKEN;

      // Multisample?
      *samples = renderbuffer->samples;

      switch (renderbuffer->type) {
      case RB_NEW_T:
         result = ATTACHMENT_BROKEN; break;
      case RB_COLOR_T:
         result = ATTACHMENT_COLOR; break;
      case RB_DEPTH16_T:
         result = ATTACHMENT_DEPTH16; break;
      case RB_DEPTH24_T:
         result = ATTACHMENT_DEPTH24; break;
      case RB_STENCIL_T:
         result = ATTACHMENT_STENCIL; break;
      case RB_DEPTH24_STENCIL8_T:
         result = ATTACHMENT_DEPTH_STENCIL; break;
      default:
         UNREACHABLE();
      }
      if (((renderbuffer->mh_storage == MEM_INVALID_HANDLE) && (*samples == 0)) ||     // non-multisample and invalid handle for non-multisample
          ((renderbuffer->mh_ms_storage == MEM_INVALID_HANDLE) && (*samples != 0)))    // multisample and invalid handle for multisample
      {
         *width = 0;
         *height = 0;
      } else {
         KHRN_IMAGE_T *image = NULL;
         KHRN_IMAGE_FORMAT_T format;
         if ((*samples != 0) && (result != ATTACHMENT_COLOR))   // If it is multisample and depth/stencil buffer
         {
            image = (KHRN_IMAGE_T *)mem_lock(renderbuffer->mh_ms_storage, NULL);
            *width = image->width / 2;
            *height = image->height / 2;
            *secure = image->secure;
            format = image->format;
            mem_unlock(renderbuffer->mh_ms_storage);
         }
         else
         {  // if a non-multisample or a colour attachment
            // (colour attachments have a resolve buffer but not depth/stencil attachment)
            // For colour attachment we take the resolved version as it has the correct colour format
            // which is checked by glxx_framebuffer_hw_support few lines below
            image = (KHRN_IMAGE_T *)mem_lock(renderbuffer->mh_storage, NULL);
            *width = image->width;
            *height = image->height;
            *secure = image->secure;
            format = image->format;
            mem_unlock(renderbuffer->mh_storage);
         }

         format = khrn_image_no_colorspace_format(image->format);
         if (!glxx_framebuffer_hw_support(format))
            result = ATTACHMENT_UNSUPPORTED;
      }
      if (*width == 0 || *height == 0)
         result = ATTACHMENT_BROKEN;
      mem_unlock(attachment->mh_object);
      return result;
   }
   case GL_TEXTURE:
   {
      MEM_HANDLE_T himage = glxx_attachment_info_get_image(attachment);
      ATTACHMENT_STATUS_T result = ATTACHMENT_BROKEN;
      if (himage == MEM_INVALID_HANDLE) {
         *width = 0;
         *height = 0;
         *secure = false;
         *samples = 0;
      } else {
         KHRN_IMAGE_T *image = (KHRN_IMAGE_T *)mem_lock(himage, NULL);
         KHRN_IMAGE_FORMAT_T format = khrn_image_no_colorspace_format(image->format);
         switch (format)
         {
#ifndef BIG_ENDIAN_CPU
         case ABGR_8888_TF:
         case XBGR_8888_TF:
#if GL_EXT_texture_format_BGRA8888
         case ARGB_8888_TF:
#endif
#endif
#ifdef BIG_ENDIAN_CPU
         case RGBA_8888_TF:
         case RGBX_8888_TF:
#if GL_EXT_texture_format_BGRA8888
         case BGRA_8888_TF:
#endif
#endif
         case RGB_565_TF:
         case ABGR_8888_LT:
         case XBGR_8888_LT:
         case RGB_565_LT:
         case ABGR_8888_RSO:
         case ARGB_8888_RSO:
         case XBGR_8888_RSO:
         case XRGB_8888_RSO:
         case RGB_565_RSO:
            result = ATTACHMENT_COLOR;
            break;
         case RGBA_4444_TF:
         case RGBA_5551_TF:
         case RGBA_4444_LT:
         case RGBA_5551_LT:
            result = ATTACHMENT_UNSUPPORTED;
            break;
         default:  //other texture formats or IMAGE_FORMAT_INVALID
            result = ATTACHMENT_BROKEN;
         }
         *width = image->width;
         *height = image->height;
         *secure = image->secure;
         *samples = attachment->samples;
         mem_unlock(himage);
      }
      if (*width == 0 || *height == 0)
         result = ATTACHMENT_BROKEN;
      return result;
   }
   default:
      UNREACHABLE();
      return ATTACHMENT_BROKEN;
   }
}

GLenum glxx_framebuffer_check_status(GLXX_FRAMEBUFFER_T *framebuffer)
{
   ATTACHMENT_STATUS_T ca, da, sa;
   bool csecure, dsecure, ssecure;
   //          width,   height,  sample
   uint32_t    cw,      ch,      cs;
   uint32_t    dw,      dh,      ds;
   uint32_t    sw,      sh,      ss;
   GLenum result = GL_FRAMEBUFFER_COMPLETE;

   vcos_assert(framebuffer);

   // color/depth/stencil attachment/width/height
   // If attachment is NONE or BROKEN, width and height should be ignored

   ca = attachment_get_status(&framebuffer->attachments.color, &cw, &ch, &cs, &csecure);
   da = attachment_get_status(&framebuffer->attachments.depth, &dw, &dh, &ds, &dsecure);
   sa = attachment_get_status(&framebuffer->attachments.stencil, &sw, &sh, &ss, &ssecure);

   if (ca == ATTACHMENT_MISSING && da == ATTACHMENT_MISSING && sa == ATTACHMENT_MISSING)
      result = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
   else if (ca == ATTACHMENT_UNSUPPORTED || da == ATTACHMENT_UNSUPPORTED || sa == ATTACHMENT_UNSUPPORTED)
      result = GL_FRAMEBUFFER_UNSUPPORTED;
   else {
      if (ca == ATTACHMENT_MISSING)
         result = GL_FRAMEBUFFER_UNSUPPORTED;
      else if (da == ATTACHMENT_DEPTH16 && sa != ATTACHMENT_MISSING)
         result = GL_FRAMEBUFFER_UNSUPPORTED;
      else {
         GLboolean color_complete = (ca == ATTACHMENT_MISSING || ca == ATTACHMENT_COLOR);
         GLboolean depth_complete = (da == ATTACHMENT_MISSING || da == ATTACHMENT_DEPTH16 ||
                                    da == ATTACHMENT_DEPTH24 || da == ATTACHMENT_DEPTH_STENCIL);
         GLboolean stencil_complete = (sa == ATTACHMENT_MISSING || sa == ATTACHMENT_STENCIL ||
                                       sa == ATTACHMENT_DEPTH_STENCIL);

         if (!color_complete || !depth_complete || !stencil_complete)
            result = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
         else {
            /*
               at this point we know we have

               - a complete color buffer
               - a complete depth buffer (if present)
               - a complete stencil buffer (if present)

               we need to check whether we have a depth buffer, and if so whether it has the same size as the color buffer
               similarly for stencil

               If it is a multisample FBO we need to check that the number of sample is the same for all the attachments
            */

            if (da != ATTACHMENT_MISSING && (cw != dw || ch != dh))
               result = GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS;
            else if (sa != ATTACHMENT_MISSING && (cw != sw || ch != sh))
               result = GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS;

            if (da != ATTACHMENT_MISSING && (csecure != dsecure))
               result = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
            else if (sa != ATTACHMENT_MISSING && (csecure != ssecure))
               result = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;

            // If this is a multisample FBO
            if (cs != 0)
            {  // Check that the number of samples is the same for all attachments
               if (da != ATTACHMENT_MISSING && (cs != ds))
                  result = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT;
               else if (sa != ATTACHMENT_MISSING && (cs != ss))
                  result = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT;
            }
         }
      }
   }

   return result;
}
