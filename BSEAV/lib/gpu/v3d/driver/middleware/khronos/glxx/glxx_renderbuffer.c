/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "interface/khronos/glxx/glxx_int_config.h"

#include "middleware/khronos/glxx/glxx_renderbuffer.h"
#include "middleware/khronos/glxx/glxx_framebuffer.h"
#include "middleware/khronos/glxx/glxx_server.h"

#include "middleware/khronos/common/khrn_mem.h"

static KHRN_IMAGE_FORMAT_T convert_format(GLenum format);
static GLXX_RENDERBUFFER_TYPE_T convert_renderbuffer_type(GLenum format);

void glxx_renderbuffer_init(GLXX_RENDERBUFFER_T *renderbuffer, int32_t name)
{
   assert(renderbuffer);

   renderbuffer->name = name;

   renderbuffer->type = RB_NEW_T;
   assert(renderbuffer->storage == NULL);
   assert(renderbuffer->ms_storage == NULL);
}

void glxx_renderbuffer_term(void *p)
{
   GLXX_RENDERBUFFER_T *renderbuffer = p;

   KHRN_MEM_ASSIGN(renderbuffer->storage, NULL);
   KHRN_MEM_ASSIGN(renderbuffer->ms_storage, NULL);
}

static KHRN_IMAGE_FORMAT_T convert_format(GLenum format)
{
   switch (format) {
   /* case GL_DEPTH_COMPONENT: TODO confirm if this is a permissable value */
   case GL_DEPTH_COMPONENT24_OES:
   case GL_DEPTH_COMPONENT32_OES:
   case GL_DEPTH_STENCIL_OES:
   case GL_DEPTH24_STENCIL8_OES:
   /* case GL_STENCIL_INDEX: TODO confirm if this is a permissable value */
   case GL_STENCIL_INDEX8:
      return DEPTH_32_TLBD;
   case GL_RGB: /* This probably isn't a permissable value but is used in 2.0 conformance tests */
   case GL_RGB8_OES:
      #ifdef BIG_ENDIAN_CPU
         return RGBX_8888_TF;
      #else
         return XBGR_8888_TF;
      #endif
   case GL_RGBA: /* This probably isn't a permissable value but is used in 2.0 conformance tests */
   case GL_RGBA8_OES:
      #ifdef BIG_ENDIAN_CPU
         return RGBA_8888_TF;
      #else
         return ABGR_8888_TF;
      #endif
   case GL_RGBA4:
      return RGBA_4444_TF;
   case GL_RGB5_A1:
      return RGBA_5551_TF;
   case GL_RGB565:
      return RGB_565_TF;
   case GL_DEPTH_COMPONENT16:
      return DEPTH_32_TLBD;
   default:
      UNREACHABLE();
      break;
   }

   return IMAGE_FORMAT_INVALID;
}

bool glxx_renderbuffer_storage_multisample(GLXX_RENDERBUFFER_T *renderbuffer, GLsizei samples,
   GLenum internalformat, GLuint width, GLuint height, bool secure)
{
   KHRN_IMAGE_FORMAT_T format = convert_format(internalformat);
   GLXX_RENDERBUFFER_TYPE_T type = convert_renderbuffer_type(internalformat);
   KHRN_IMAGE_T *oldimage = renderbuffer->storage;

   /* TODO: what about if we're sharing the image? */
   bool imagechanged = !oldimage ||
      oldimage->format != format ||
      oldimage->width != width ||
      oldimage->height != height ||
      oldimage->secure != secure ||
      renderbuffer->type != type ||
      renderbuffer->samples != samples;

   if (imagechanged) {
      uint32_t flags = 0;

#ifdef DEBUG
      flags = IMAGE_CREATE_FLAG_ONE;
#endif
      if (type == RB_COLOR_T) flags |= IMAGE_CREATE_FLAG_TEXTURE;
      if (glxx_framebuffer_hw_support(format)) flags |= IMAGE_CREATE_FLAG_RENDER_TARGET;

      // Clamp the number of samples to 0 (non-multisample) or 4 (multisample)
      // 1 is considerated as multisample in the spec EXT_framebuffer_multisample, revision #7, Issue (2)
      // (Written based on the wording of the OpenGL 1.5 specification.)
      samples = samples ? GLXX_CONFIG_SAMPLES : 0;

      if (type == RB_COLOR_T) {
         // Default state for renderbuffer object
         renderbuffer->type = RB_NEW_T;
         renderbuffer->samples = 0;
         KHRN_MEM_ASSIGN(renderbuffer->storage, NULL);
         KHRN_MEM_ASSIGN(renderbuffer->ms_storage, NULL);

         // Allocate the resolve buffer if it is a colour render buffer
         KHRN_IMAGE_T *image = khrn_image_create(format, width, height, flags, secure); /* todo: check usage flags */
         if (image == NULL)
            return false;
         KHRN_MEM_ASSIGN(renderbuffer->storage, image);
         KHRN_MEM_ASSIGN(image, NULL);

         if (samples) {
            // Allocate the multisample buffer which is stored in the renderbuffer
            KHRN_IMAGE_T *ms_image = glxx_image_create_ms(COL_32_TLBD, width, height,
               IMAGE_CREATE_FLAG_RENDER_TARGET | IMAGE_CREATE_FLAG_ONE, secure);

            // If the ms image fails to allocate, invalidate the resolve just allocated above
            if (ms_image == NULL) {
               KHRN_MEM_ASSIGN(renderbuffer->storage, NULL);
               return false;
            }
            KHRN_MEM_ASSIGN(renderbuffer->ms_storage, ms_image);
            KHRN_MEM_ASSIGN(ms_image, NULL);
         }
      }
      else {
         // Color handled above, this is depth/stencil only.  Only push into the none ms mode reguardless
         // as depths are not resolved
         KHRN_IMAGE_T *(*image_create)(KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height,
            KHRN_IMAGE_CREATE_FLAG_T flags, bool secure);
         image_create = (samples) ? glxx_image_create_ms : khrn_image_create;

         KHRN_IMAGE_T *image = image_create(format, width, height, flags, secure); /* todo: check usage flags */
         if (image == NULL)
            return false;
         KHRN_MEM_ASSIGN(renderbuffer->storage, image);
         KHRN_MEM_ASSIGN(image, NULL);
      }

      renderbuffer->type = type;
      renderbuffer->samples = samples;
   }

   return true;
}

static bool valid_image(KHRN_IMAGE_T *image)
{
   KHRN_IMAGE_FORMAT_T format;
   if ((image->width > GLXX_CONFIG_MAX_RENDERBUFFER_SIZE) || (image->height > GLXX_CONFIG_MAX_RENDERBUFFER_SIZE))
      return false;

   if (!(khrn_image_is_tformat(image->format) || (khrn_image_is_color(image->format) && khrn_image_is_lineartile(image->format))))
      return false;

   format = khrn_image_to_tf_format(image->format);

   return
      format == ABGR_8888_TF ||
      format == XBGR_8888_TF ||
      format == RGBA_8888_TF ||
      format == RGBX_8888_TF ||
      format == RGBA_4444_TF ||
      format == RGBA_5551_TF ||
      format == RGB_565_TF;
      /* currently don't allow depth/stencil formats
      format == DEPTH_16_TF ||
      format == DEPTH_32_TF;*/
}

bool glxx_renderbuffer_bind_image(GLXX_RENDERBUFFER_T *renderbuffer, KHRN_IMAGE_T *image)
{
   bool result;

   if (valid_image(image)) {
      KHRN_MEM_ASSIGN(renderbuffer->storage, image);
      assert(khrn_image_is_color(image->format));
      renderbuffer->type = RB_COLOR_T;
      renderbuffer->merged = false;
      result = true;
   } else
      result = false;

   return result;
}

static bool single_ref_count(GLXX_RENDERBUFFER_T *renderbuffer)
{
   uint32_t refcount;
   assert(renderbuffer->type != RB_NEW_T);
   assert(renderbuffer->storage != NULL);

   refcount = khrn_mem_get_ref_count(renderbuffer->storage);
   assert(refcount > 0);

   return refcount == 1;
}

// If this renderbuffer is in the merged state, force it into the shared state by
// allocating new storage.
bool glxx_renderbuffer_unmerge(GLXX_RENDERBUFFER_T *renderbuffer)
{
   if (renderbuffer->merged && !single_ref_count(renderbuffer)) {
      // Renderbuffer is in merged state. We must unmerge it.

      assert(renderbuffer->type == RB_DEPTH24_T || renderbuffer->type == RB_STENCIL_T);  // The only sorts of buffer we can merge

      assert(renderbuffer->storage->format == DEPTH_32_TF);

      KHRN_IMAGE_T *image = khrn_image_create_dup(renderbuffer->storage, IMAGE_CREATE_FLAG_NONE);

      if (image == NULL)
         return false;

      KHRN_MEM_ASSIGN(renderbuffer->storage, image);
      KHRN_MEM_ASSIGN(image, NULL);
   }
   renderbuffer->merged = false;   // This ensures we're in the correct state if someone subsequently adds a reference
   return true;
}

// Attempt to merge these buffers so that they both use the same storage buffer.
// This can only happen if there is just one reference to each buffer. If this is
// not the case then we do nothing - this is just an optimisation so it is allowed
// to fail.
// Currently if either buffer is in the merged state we will not unmerge it (though
// we could do this safely).
void glxx_renderbuffer_attempt_merge(GLXX_RENDERBUFFER_T *depth, GLXX_RENDERBUFFER_T *stencil)
{
   KHRN_IMAGE_T *dstorage = depth->storage;
   KHRN_IMAGE_T *sstorage = stencil->storage;

   assert(depth && stencil);
   assert(depth->type == RB_DEPTH24_T && stencil->type == RB_STENCIL_T);  // The only sorts of buffer we can merge
   assert(dstorage->format == DEPTH_32_TF && sstorage->format == DEPTH_32_TF);
   assert(dstorage->width == sstorage->width && dstorage->height == sstorage->height);

   if (single_ref_count(depth) && single_ref_count(stencil)) {
      khrn_image_copy_stencil_channel(dstorage, sstorage);
      KHRN_MEM_ASSIGN(stencil->storage, depth->storage);
      depth->merged = true;
      stencil->merged = true;
   }
}

static GLXX_RENDERBUFFER_TYPE_T convert_renderbuffer_type(GLenum format)
{
   switch (format) {
   /* case GL_DEPTH_COMPONENT: TODO confirm whether this is a permissable format */
   case GL_DEPTH_COMPONENT24_OES:
   case GL_DEPTH_COMPONENT32_OES:
      return RB_DEPTH24_T;
   case GL_DEPTH_STENCIL_OES:
   case GL_DEPTH24_STENCIL8_OES:
      return RB_DEPTH24_STENCIL8_T;
   case GL_RGB:    /* This probably isn't a permissable value but is used in 2.0 conformance tests */
   case GL_RGBA:   /* This probably isn't a permissable value but is used in 2.0 conformance tests */
   case GL_RGBA4:
   case GL_RGB5_A1:
   case GL_RGB565:
   case GL_RGB8_OES:
   case GL_RGBA8_OES:
      return RB_COLOR_T;
   case GL_DEPTH_COMPONENT16:
      return RB_DEPTH16_T;
   /* case GL_STENCIL_INDEX: TODO confirm whether this is a permissable format */
   case GL_STENCIL_INDEX8:
      return RB_STENCIL_T;
   default:
      UNREACHABLE();
      return RB_NEW_T;
   }
}
