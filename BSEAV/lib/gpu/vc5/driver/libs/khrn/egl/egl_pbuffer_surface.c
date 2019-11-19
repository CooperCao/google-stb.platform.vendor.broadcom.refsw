/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "egl_thread.h"
#include "egl_display.h"
#include "egl_context.h"
#include "egl_surface_base.h"
#include "egl_pbuffer_surface.h"
#include "../glxx/glxx_texture.h"
#include "egl_context_gl.h"
#include "../glxx/glxx_server.h"

struct egl_pbuffer_surface
{
   EGL_SURFACE_T              base;
   bool                       mipmap;
   int                        largest_pbuffer;
   GLenum                     texture_format;
   EGLenum                    texture_target;

   unsigned                   num_images;
   khrn_image               *images[KHRN_MAX_MIP_LEVELS];

   /* Which image to actually render to (index into images array above). */
   unsigned                   current_image;

   bool                       bound;      /* a surface can be bound and have a
                                             bound_texture; if the texture gets
                                             deleted, bound stays true, but
                                             bound_texture becomes NULL */
   GLXX_TEXTURE_T             *bound_texture; /* this is not reference counted */
};

static EGL_SURFACE_METHODS_T fns;
static khrn_image *get_back_buffer(EGL_SURFACE_T *surface);

static void set_mipmap_level(EGL_PBUFFER_SURFACE_T *surface, int level)
{
   if (level < 0)
      level = 0;
   else if ((unsigned int)level >= surface->num_images)
      level = surface->num_images - 1;

   egl_context_gl_lock();
   khrn_image *image = get_back_buffer(&surface->base);
   khrn_resource_flush(khrn_image_get_resource(image));
   egl_context_gl_unlock();

   surface->current_image = level;

   EGL_CONTEXT_T *context = surface->base.context;
   if (context)
      egl_context_reattach(context);
}

static void delete_fn(EGL_SURFACE_T *surface)
{
   EGL_PBUFFER_SURFACE_T *surf = (EGL_PBUFFER_SURFACE_T *) surface;
   unsigned i;

   if (!surface) return;

   assert(surface->type == EGL_SURFACE_TYPE_PBUFFER);

   if (surf->bound_texture)
   {
      assert(surf->bound);
      /* this texture is no longer bound to us, but still remains a
       * TEX_BOUND_TEXIMAGE */
      glxx_texture_remove_observer(surf->bound_texture);
   }

   for (i = 0; i < surf->num_images; i++)
      KHRN_MEM_ASSIGN(surf->images[i], NULL);

   egl_surface_base_destroy(surface);
   free(surf);
}

static unsigned count_mipmaps(unsigned width, unsigned height)
{
   unsigned ret = gfx_umax(gfx_msb(width), gfx_msb(height)) + 1;
   assert(ret < KHRN_MAX_MIP_LEVELS);
   return ret;
}

static EGLint init_attrib(EGL_SURFACE_T *surface, EGLint attrib, EGLAttribKHR value)
{
   EGL_PBUFFER_SURFACE_T *surf = (EGL_PBUFFER_SURFACE_T *) surface;

   switch (attrib)
   {
   case EGL_LARGEST_PBUFFER:
      surf->largest_pbuffer = (int)value;
      return EGL_SUCCESS;

   case EGL_TEXTURE_FORMAT:
      if (value != EGL_NO_TEXTURE && value != EGL_TEXTURE_RGBA &&
          value != EGL_TEXTURE_RGB)
         return EGL_BAD_PARAMETER;

      surf->texture_format = (GLenum)value;
      return EGL_SUCCESS;

   case EGL_TEXTURE_TARGET:
      if (value != EGL_NO_TEXTURE && value != EGL_TEXTURE_2D)
         return EGL_BAD_PARAMETER;

      surf->texture_target = (EGLenum)value;
      return EGL_SUCCESS;

   case EGL_MIPMAP_TEXTURE:
      if (value != EGL_FALSE && value != EGL_TRUE)
         return EGL_BAD_PARAMETER;

      surf->mipmap = !!value;
      return EGL_SUCCESS;

   case EGL_WIDTH:
      if (value < 0)
         return EGL_BAD_PARAMETER;
      else if (value > EGL_CONFIG_MAX_WIDTH)
         return EGL_BAD_MATCH;
      surf->base.width = value;
      return EGL_SUCCESS;

   case EGL_HEIGHT:
      if (value < 0)
         return EGL_BAD_PARAMETER;
      else if (value > EGL_CONFIG_MAX_HEIGHT)
         return EGL_BAD_MATCH;
      surf->base.height = value;
      return EGL_SUCCESS;

   default:
      return egl_surface_base_init_attrib(&surf->base, attrib, value);
   }
}

static EGLint set_attrib(EGL_SURFACE_T *surface, EGLint attrib, EGLAttribKHR value)
{
   EGL_PBUFFER_SURFACE_T *surf = (EGL_PBUFFER_SURFACE_T *) surface;

   switch (attrib)
   {
   case EGL_MIPMAP_LEVEL:
      set_mipmap_level(surf, (int)value);
      return EGL_SUCCESS;

   default:
      return egl_surface_base_set_attrib(&surf->base, attrib, value);
   }
}

static EGLint get_attrib(EGL_SURFACE_T *surface, EGLint attrib, EGLint *value)
{
   EGL_PBUFFER_SURFACE_T *surf = (EGL_PBUFFER_SURFACE_T *) surface;

   switch (attrib)
   {
   case EGL_LARGEST_PBUFFER:
      *value = surf->largest_pbuffer;
      return EGL_SUCCESS;

   case EGL_TEXTURE_FORMAT:
      *value = surf->texture_format;
      return EGL_SUCCESS;

   case EGL_TEXTURE_TARGET:
      *value = surf->texture_target;
      return EGL_SUCCESS;

   case EGL_MIPMAP_TEXTURE:
      *value = surf->mipmap;     // EGL_FIXME check this
      return EGL_SUCCESS;

   case EGL_MIPMAP_LEVEL:
      *value = surf->current_image;
      return EGL_SUCCESS;

   /* handled by egl_surface_base_get_attrib()
   case EGL_WIDTH:
   case EGL_HEIGHT: */
   default:
      break;
   }

   return egl_surface_base_get_attrib(&surf->base, attrib, value);
}

static khrn_image *get_back_buffer(EGL_SURFACE_T *surface)
{
   const EGL_PBUFFER_SURFACE_T *surf = (const EGL_PBUFFER_SURFACE_T *) surface;
   return surf->images[surf->current_image];
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePbufferSurface(EGLDisplay dpy,
      EGLConfig config_in, const EGLint *attrib_list)
{
   EGLint error = EGL_BAD_ALLOC;
   EGL_PBUFFER_SURFACE_T *surface = NULL;
   EGLSurface ret = EGL_NO_SURFACE;
   unsigned level;
   static gfx_buffer_usage_t flags;
   khrn_blob *blob = NULL;

   /* width and height should be in the attrib_list */
   const unsigned default_width = 0;
   const unsigned default_height = 0;

   if (!egl_initialized(dpy, true))
      return EGL_NO_SURFACE;

   const EGL_CONFIG_T *config = egl_config_validate(config_in);
   if (!config)
   {
      error = EGL_BAD_CONFIG;
      goto end;
   }

   surface = calloc(1, sizeof *surface);
   if (!surface) goto end;

   surface->base.type = EGL_SURFACE_TYPE_PBUFFER;
   /* default values */
   surface->mipmap = false;
   surface->num_images = 1;
   surface->current_image = 0;
   surface->largest_pbuffer = false;
   surface->texture_format = EGL_NO_TEXTURE;
   surface->texture_target = EGL_NO_TEXTURE;
   surface->bound = false;
   surface->bound_texture = NULL;

   error = egl_surface_base_init(&surface->base,
         &fns, config, attrib_list, attrib_EGLint,
         default_width, default_height, NULL, NULL);
   if (error != EGL_SUCCESS) goto end;

   GFX_LFMT_T color_lfmt = gfx_lfmt_to_2d(egl_api_fmt_to_lfmt(config->color_api_fmt, config->x_padded));
   GFX_LFMT_T color_api_fmt = egl_config_color_api_fmt(config);

   if ((surface->texture_format == EGL_NO_TEXTURE) !=
         (surface->texture_target == EGL_NO_TEXTURE))
   {
      error = EGL_BAD_MATCH;
      goto end;
   }

   if (surface->texture_format != EGL_NO_TEXTURE &&
      !glxx_texture_are_legal_dimensions(GL_TEXTURE_2D,
            surface->base.width, surface->base.height, 1))
   {
      error = EGL_BAD_MATCH;
      goto end;
   }

   //pbuffers can end up being used as textures via glBlitFramebuffer
   flags = GFX_BUFFER_USAGE_V3D_RENDER_TARGET |GFX_BUFFER_USAGE_V3D_TEXTURE;

   // TODO: we should check that color_lfmt matches texture_format(RGB/ RGBA)
   if (surface->mipmap && surface->texture_format != EGL_NO_TEXTURE)
   {
      surface->num_images = count_mipmaps(surface->base.width,
            surface->base.height);

      blob = khrn_blob_create(surface->base.width, surface->base.height,
            1, 1, surface->num_images, &color_lfmt, 1, flags,
            surface->base.secure ? GMEM_USAGE_SECURE : GMEM_USAGE_NONE);
      if (!blob) goto end;
   }
   else
   {
      blob = khrn_blob_create(surface->base.width, surface->base.height,
            1, 1, 1, &color_lfmt, 1, flags, surface->base.secure ? GMEM_USAGE_SECURE : GMEM_USAGE_NONE);
      if (!blob) goto end;
   }

   for (level = 0; level < surface->num_images; level++)
   {
      surface->images[level] = khrn_image_create(blob, 0, 1,
            level, color_api_fmt);
      if (surface->images[level] == NULL)
         goto end;
   }
   KHRN_MEM_ASSIGN(blob, NULL);

   ret = egl_map_surface((EGL_SURFACE_T *) surface);
   if (!ret) goto end;

   error = EGL_SUCCESS;

end:
   if (error != EGL_SUCCESS)
   {
      KHRN_MEM_ASSIGN(blob, NULL);
      delete_fn((EGL_SURFACE_T *) surface);
      egl_unmap_surface(ret);
      ret = EGL_NO_SURFACE;
   }
   egl_thread_set_error(error);
   return ret;
}

static void texture_unbind_callback(void *s)
{
   EGL_PBUFFER_SURFACE_T *surface = s;
   surface->bound_texture = NULL;
}

EGLAPI EGLBoolean EGLAPIENTRY eglBindTexImage(EGLDisplay dpy,
      EGLSurface surface, EGLint buffer)
{
   EGL_SURFACE_T *surf = NULL;
   EGL_PBUFFER_SURFACE_T *pbsurf;
   EGLint error = EGL_BAD_ALLOC;
   bool ok;
   tex_unbind_observer_t observer;

   GLXX_SERVER_STATE_T *state;
   GLXX_TEXTURE_T *texture;

   bool locked = false;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   surf = egl_surface_lock(surface);
   if (!surf || surf->type != EGL_SURFACE_TYPE_PBUFFER)
   {
      error = EGL_BAD_SURFACE;
      goto end;
   }

   if (buffer != EGL_BACK_BUFFER)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   pbsurf = (EGL_PBUFFER_SURFACE_T *) surf;

   if (pbsurf->texture_format == EGL_NO_TEXTURE)
   {
      error = EGL_BAD_MATCH;
      goto end;
   }

   if (pbsurf->bound == true)
   {
      error = EGL_BAD_ACCESS;
      goto end;
   }

   egl_context_gl_lock();
   locked = true;

   error = EGL_SUCCESS;

   state = egl_context_gl_server_state(NULL);
   if (!state) goto end;

   texture = glxx_server_get_active_texture(state, GL_TEXTURE_2D);
   if (!texture) goto end;

   ok = glxx_texture_bind_teximage(texture, pbsurf->images, pbsurf->num_images,
         pbsurf->current_image, &state->fences);

   if (!ok)
      goto end;

   observer.callback = texture_unbind_callback;
   observer.param = pbsurf;
   glxx_texture_add_observer(texture, observer);

   /* this is not refcounted; we will get a callback when the texture gets
    * deleted or the texture unbinds because of glTexImage */
   pbsurf->bound_texture = texture;
   pbsurf->bound = true;

   error = EGL_SUCCESS;
end:
   egl_surface_unlock(surf);
   egl_thread_set_error(error);
   if (locked)
      egl_context_gl_unlock();
   return error == EGL_SUCCESS;
}

EGLAPI EGLBoolean EGLAPIENTRY eglReleaseTexImage(EGLDisplay dpy,
      EGLSurface surface, EGLint buffer)
{
   EGL_SURFACE_T *surf = NULL;
   EGL_PBUFFER_SURFACE_T *pbsurf;
   EGLint error = EGL_BAD_ALLOC;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   surf = egl_surface_lock(surface);
   if (!surf || surf->type != EGL_SURFACE_TYPE_PBUFFER)
   {
      error = EGL_BAD_SURFACE;
      goto end;
   }

   if (buffer != EGL_BACK_BUFFER)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   pbsurf = (EGL_PBUFFER_SURFACE_T *) surf;
   if (pbsurf->texture_format == EGL_NO_TEXTURE)
   {
      error = EGL_BAD_MATCH;
      goto end;
   }

   if (!pbsurf->bound)
   {
      error = EGL_BAD_SURFACE;
      goto end;
   }

   /* if the texture was deleted, no, error is generated */
   if (pbsurf->bound_texture)
   {
      egl_context_gl_lock();
      glxx_texture_release_teximage(pbsurf->bound_texture);
      assert(pbsurf->bound_texture == NULL);
      egl_context_gl_unlock();
   }

   pbsurf->bound = false;
   error = EGL_SUCCESS;
end:
   egl_surface_unlock(surf);
   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePbufferFromClientBuffer(EGLDisplay dpy,
      EGLenum buftype, EGLClientBuffer buffer,
      EGLConfig config, const EGLint *attrib_list)
{
   EGLint error;
   EGL_CONTEXT_T *context;
   EGLSurface ret = EGL_NO_SURFACE;

   if (!egl_initialized(dpy, true))
      return ret;

   if (buftype != EGL_OPENVG_IMAGE)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   if (!egl_config_validate(config))
   {
      error = EGL_BAD_CONFIG;
      goto end;
   }

   if (buftype == EGL_OPENVG_IMAGE)
   {
      /* Client buffer cannot be a valid OpenVG image since we don't support it */
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   context = egl_thread_get_context();
   if (!context || context->api != API_OPENVG)
   {
      error = EGL_BAD_ACCESS;
      goto end;
   }

   /*
    * We don't support OPEN_VG at the moment, which is the only valid kind of
    * ClientBuffer for this. Checking the attrib_list and returning
    * EGL_BAD_ATTRIBUTE is probably something you'll do when actually
    * constructing the PBuffer.
    */
   not_impl();

   error = EGL_SUCCESS;
end:
   egl_thread_set_error(error);
   return ret;
}

static EGL_SURFACE_METHODS_T fns =
{
   .get_back_buffer = get_back_buffer,
   .get_attrib = get_attrib,
   .init_attrib = init_attrib,
   .set_attrib = set_attrib,
   .delete_fn = delete_fn,
};
