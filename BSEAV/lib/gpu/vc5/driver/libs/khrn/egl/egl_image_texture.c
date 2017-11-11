/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "egl_image_texture.h"

#include "vcos.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "egl_image.h"
#include "egl_context_gl.h"
#include "../glxx/glxx_server.h"
#include "../glxx/glxx_shared.h"

static uint32_t get_face_for_target(EGLenum target)
{
   switch (target) {
   case EGL_GL_TEXTURE_2D_KHR: return 0;
   case EGL_GL_TEXTURE_3D_KHR: return 0;
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR: return 0;
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR: return 1;
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR: return 2;
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR: return 3;
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR: return 4;
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR: return 5;
   default:
      assert(0);
      return 0;
   }
}

struct image_attribs
{
   unsigned int level;
   unsigned int zoffset;
   bool preserved;
};

/*
 * Parse the attrib_list, returning error if it contains bad attributes.
 */
static EGLint get_image_attribs(struct image_attribs *attr,
      const void *attrib_list, EGL_AttribType attrib_type)
{
   EGLint error = EGL_SUCCESS;

   attr->level = 0;
   attr->zoffset = 0;
   attr->preserved = false;

   if (!attrib_list)
      return EGL_SUCCESS;

   EGLint name;
   EGLAttribKHR value;
   while (error == EGL_SUCCESS && egl_next_attrib(&attrib_list, attrib_type,
         &name, &value))
   {
      switch (name)
      {
      case EGL_GL_TEXTURE_LEVEL_KHR:
         attr->level = (unsigned int)value;
         break;
      case EGL_GL_TEXTURE_ZOFFSET_KHR:
         attr->zoffset = value;
         break;
      case EGL_IMAGE_PRESERVED_KHR:
         if (value != EGL_TRUE && value != EGL_FALSE)
            error = EGL_BAD_ATTRIBUTE;
         attr->preserved = value != EGL_FALSE;
         break;
      default:
         /* EGL_KHR_gl_image: additional values specified in <attr_list> are ignored */
         break;
      }
   }
   return error;
}

static bool egl_is_valid_target(EGLenum target, enum glxx_tex_target tex_target)
{
   bool valid;
   switch (target)
   {
   case EGL_GL_TEXTURE_2D_KHR:
      valid = (tex_target == E_GL_TEXTURE_2D ||
               tex_target == E_GL_TEXTURE_EXTERNAL_OES);
      break;
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
      valid = (tex_target == E_GL_TEXTURE_CUBE_MAP);
      break;
   case EGL_GL_TEXTURE_3D_KHR:
      valid = (tex_target == E_GL_TEXTURE_3D);
      break;
   default:
      valid = false;
      break;
   }
   return valid;
}

EGL_IMAGE_T *egl_image_texture_new(EGL_CONTEXT_T *context,
      EGLenum target, EGLClientBuffer buffer, const void *attrib_list,
      EGL_AttribType attrib_type)
{
   EGLint error = EGL_BAD_ALLOC;
   EGL_GL_CONTEXT_T *gl_context;
   GLXX_SERVER_STATE_T *server = NULL;
   GLXX_SHARED_T *shared;
   khrn_image *image = NULL;
   EGL_IMAGE_T *egl_image = NULL;
   bool complete, share, base_complete;
   unsigned base_level, num_levels, face;
   struct image_attribs attr;
   enum glxx_tex_completeness tex_complete;
   GLXX_TEXTURE_SAMPLER_STATE_T sampler_obj;

   egl_context_gl_lock();

   if (context == NULL || context->api != API_OPENGL)
   {
      error = EGL_BAD_MATCH;
      goto end;
   }

   gl_context = (EGL_GL_CONTEXT_T *) context;
   server = egl_context_gl_server_state(gl_context);
   shared = server->shared;

   /* get_texture returns invalid for default texture (0) */
   GLXX_TEXTURE_T *texture = ((uintptr_t)buffer <= UINT32_MAX) ?
      glxx_shared_get_texture(shared, (uint32_t)(uintptr_t)buffer) :
      NULL;
   if (texture == NULL)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   if (!egl_is_valid_target(target, texture->target))
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   error = get_image_attribs(&attr, attrib_list, attrib_type);
   if (error != EGL_SUCCESS)
      goto end;

   face = get_face_for_target(target);
   if (!glxx_texture_is_legal_level(texture->target, attr.level))
   {
      error = EGL_BAD_MATCH;
      goto end;
   }

   if (texture->img[face][attr.level] == NULL)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   if (texture->target == EGL_GL_TEXTURE_3D_KHR)
   {
      if (!glxx_texture_is_legal_layer(texture->target, attr.zoffset) ||
            attr.zoffset > khrn_image_get_depth(texture->img[face][attr.level]))
      {
         error = EGL_BAD_PARAMETER;
         goto end;
      }
   }

   /* if we already are already share this level, fail;
    * TODO: fail if we are already share this level for eglimage source */
   if (texture->binding == TEX_BOUND_TEXIMAGE ||
       texture->binding == TEX_BOUND_EGLIMAGE_TARGET)
   {
      error = EGL_BAD_ACCESS;
      goto end;
   }

   /* check the texture is either mipmap complete or base complete and it has
    * no other mipmap levels specified outside the completness range */
   share = false;
   base_complete = false;
   complete = glxx_texture_check_completeness(texture, base_complete, &base_level,
         &num_levels);
   if (!complete)
   {
      base_complete = true;
      complete = glxx_texture_check_completeness(texture, base_complete, &base_level,
                &num_levels);
   }

   /* if complete make sure there are no other mipmap levels outside base_level and
   * num_levels */
   if (complete &&
       (attr.level >= base_level && attr.level < base_level + num_levels))
   {
      /* make sure there are no other mipmap levels outside base_level and
      * num_levels */
      if (!glxx_texture_has_images_outside_range(texture, base_level,
            num_levels))
      {
         share = true;
      }
   }

   if (!share)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   /* we set the sampler such that when we call ensure contiguous blob we
    * get base_complete or mip_map complete blob */
   sampler_obj = texture->sampler;
   sampler_obj.filter.min = base_complete ? GL_NEAREST : GL_NEAREST_MIPMAP_NEAREST;

   tex_complete = glxx_texture_ensure_contiguous_blob_if_complete(texture,
         &sampler_obj, &base_level, &num_levels, &server->fences);

   if (tex_complete != COMPLETE)
   {
      assert(tex_complete == OUT_OF_MEMORY);
      error = EGL_BAD_ALLOC;
      goto end;
   }

   if(!glxx_texture_acquire_from_eglimage(texture, face, attr.level,
         attr.zoffset, &image))
   {
      error = EGL_BAD_ALLOC;
      goto end;
   }

   /* we must have an image for this level because we checked completness
    * in the code above, if we don't it means zoffset was out of range */
   if (!image)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   egl_image = egl_image_create(image, 1);
   KHRN_MEM_ASSIGN(image, NULL);
   if (!egl_image)
      goto end;
   error = EGL_SUCCESS;

end:
   egl_context_gl_unlock();
   egl_thread_set_error(error);
   return egl_image;
}
