/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "egl_surface_base.h"
#include "egl_config.h"
#include "egl_display.h"
#include "egl_thread.h"
#include "egl_context_base.h"

LOG_DEFAULT_CAT("egl_surface_base")

static EGLint init_attrib(EGL_SURFACE_T *surface,
      EGLint attrib, EGLAttribKHR value)
{
   if (surface->fns->init_attrib)
      return surface->fns->init_attrib(surface, attrib, value);
   else
      return egl_surface_base_init_attrib(surface, attrib, value);
}

static EGLint init_attribs(EGL_SURFACE_T *surface, const void *attrib_list,
      EGL_AttribType attrib_type)
{
   EGLint name;
   EGLAttribKHR value;
   while (egl_next_attrib(&attrib_list, attrib_type, &name, &value))
   {
      EGLint ret = init_attrib(surface, name, value);
      if (ret != EGL_SUCCESS) return ret;
   }

   return EGL_SUCCESS;
}

static void manage_buffer_age_heuristics(EGL_BUFFER_AGE_DAMAGE_STATE_T *state)
{
   // When an application queries buffer age for a surface, we are required to have preserved
   // the content of that buffer whenever we return a non-zero buffer age. In a naive
   // implementation, this effectively means the first age query latches the preserve state
   // forever. The preserve costs bandwidth and hurts performance slightly, so we want to do
   // better. When small damage regions are used, the savings outweigh the cost.
   //
   // We use a heuristic approach to attempt to get some performance back when apps use
   // fullscreen damage regions, or query the age but never set a damage region.
   //
   // We monitor the number of sequential frames where no damage region is set, or a fullscreen
   // region is present. If we see a number of these in a row, we start to return buffer age
   // of zero. In this mode, the application must redraw the entire frame and we don't need
   // to preserve. Once in this mode however, the application will never give us a damage region
   // again, so we wait for a particular number of frames and then give back a real buffer age.
   // This gives the app a chance to send a non-fullscreen damage region again.
   if (state->buffer_age_enabled && !khrn_options.disable_big_damage_opt)
   {
      if (state->buffer_age_queried)
      {
         // Default new_mode will be the current mode
         egl_buffer_count_mode_t new_mode = state->mode;

         if (state->buffer_age == 0)
         {
            // Next frame has a real age of zero, so reset state machine
            new_mode = MODE_REPORT_REAL_AGE_ZERO;
            state->big_damage_count   = 0;
            state->age_override_count = 0;
         }
         else
         {
            if (state->mode == MODE_REPORT_REAL_AGE_ZERO)
            {
               // The last buffer had a real age of zero
               if (state->buffer_age > 0)
                  new_mode = MODE_REPORT_REAL_BUFFER_AGE;
            }
            else if (state->mode == MODE_REPORT_ZERO_BUFFER_AGE)
            {
               // We are counting frames whilst reporting ages of zero
               state->age_override_count++;

               if (state->age_override_count >= khrn_options.zero_age_retry_cnt)
               {
                  // Transition back to giving out real ages and counting big damage regions
                  new_mode = MODE_REPORT_REAL_BUFFER_AGE;
                  state->age_override_count = 0;
                  state->big_damage_count   = 0;
               }
            }
            else if (state->mode == MODE_REPORT_REAL_BUFFER_AGE)
            {
               // We are reporting real buffer ages, and counting sequences of big damage
               bool big_region = state->num_damage_rects <= 0 || ((100.0f * state->damage_coverage) >=
                                                                   khrn_options.big_damage_thresh);
               if (big_region)
                  state->big_damage_count++;
               else
                  state->big_damage_count = 0;

               if (state->big_damage_count >= khrn_options.big_damage_opt_cnt)
               {
                  // We've seen enough big damage frames in a row, so transition to reporting
                  // zero ages
                  new_mode = MODE_REPORT_ZERO_BUFFER_AGE;
                  state->age_override_count = 0;
               }
            }
         }

         // Assign the new state machine mode
         state->mode = new_mode;

         log_trace("big_damage_count = %u, age_override_count = %u",
                   state->big_damage_count, state->age_override_count);
      }
      else
      {
         // If buffer age wasn't queried this frame, disable until it's queried again to prevent
         // having to preserve.
         state->buffer_age_enabled = false;
         state->mode = MODE_REPORT_REAL_BUFFER_AGE;
      }
   }
   else
      state->mode = MODE_REPORT_REAL_BUFFER_AGE;

   switch (state->mode)
   {
   case MODE_REPORT_REAL_AGE_ZERO  :
   case MODE_REPORT_ZERO_BUFFER_AGE: state->buffer_age_override = 0;                 break;
   case MODE_REPORT_REAL_BUFFER_AGE: state->buffer_age_override = state->buffer_age; break;
   }
}

void egl_surface_base_update_buffer_age_heuristics(EGL_BUFFER_AGE_DAMAGE_STATE_T *state)
{
   // Calculate the buffer age we want to report and update the heuristics
   manage_buffer_age_heuristics(state);

   // If no-one has ever queried the buffer age on this surface, treating it as undefined
   // allows later optimizations, so force age to 0. Setting the khrn_option to disable buffer
   // age will force buffer_age_enabled off and therefore also treat the buffer as undefined.
   if (!state->buffer_age_enabled || state->age_override_count > 0)
      state->buffer_age_override = 0;
}

void egl_surface_base_swap_done(EGL_SURFACE_T *surface)
{
   EGL_BUFFER_AGE_DAMAGE_STATE_T *state = &surface->age_damage_state;

   /* Reset the per-swap state in the surface */
   KHRN_MEM_ASSIGN(state->damage_rects, NULL);
   state->num_damage_rects    = -1;
   state->buffer_age_queried  = false;
}

EGLint egl_surface_base_init(EGL_SURFACE_T *surface,
      const EGL_SURFACE_METHODS_T *fns,
      const EGL_CONFIG_T *config, const void *attrib_list,
      EGL_AttribType attrib_type,
      unsigned width, unsigned height,
      EGLNativeWindowType win, EGLNativePixmapType pix)
{
   if (win != NULL && egl_any_surfaces_using_native_win(win))
      return EGL_BAD_ALLOC;

   if (pix != NULL && egl_any_surfaces_using_native_pixmap(pix))
      return EGL_BAD_ALLOC;

   /* check config has required WINDOW, PIXMAP or PBUFFER bit set */
   EGLint valid_types = egl_config_get_attrib(config, EGL_SURFACE_TYPE, NULL);
   EGLint surface_type;
   if (win != NULL)
      surface_type = EGL_WINDOW_BIT;
   else if (pix != NULL)
      surface_type = EGL_PIXMAP_BIT;
   else
      surface_type = EGL_PBUFFER_BIT;

   if (!(valid_types & surface_type))
      return EGL_BAD_MATCH;

   surface->fns = fns;
   surface->config = config;
   surface->width = width;
   surface->height = height;

   surface->multisample_resolve = EGL_MULTISAMPLE_RESOLVE_DEFAULT;

   surface->native_window = win;
   surface->native_pixmap = pix;

   EGL_BUFFER_AGE_DAMAGE_STATE_T *dmg_state = &surface->age_damage_state;
   dmg_state->buffer_age          = 0;
   dmg_state->buffer_age_override = 0;
   dmg_state->buffer_age_queried  = false;
   dmg_state->buffer_age_enabled  = false;
   dmg_state->age_override_count  = 0;
   dmg_state->big_damage_count    = 0;
   dmg_state->num_damage_rects    = -1;
   dmg_state->damage_coverage     = 0.0f;
   dmg_state->damage_rects        = NULL;
   dmg_state->mode                = MODE_REPORT_REAL_AGE_ZERO;

   EGLint status = init_attribs(surface, attrib_list, attrib_type);
   if (status != EGL_SUCCESS) return status;

   if (surface->width > EGL_CONFIG_MAX_WIDTH
         || surface->height > EGL_CONFIG_MAX_HEIGHT)
   {
      return EGL_BAD_NATIVE_WINDOW;
   }

   if (!egl_surface_base_init_aux_bufs(surface))
      return EGL_BAD_ALLOC;

   if (vcos_mutex_create(&surface->lock, "EGL Surface") != VCOS_SUCCESS)
      return EGL_BAD_ALLOC;

   return EGL_SUCCESS;
}

void egl_surface_base_destroy(EGL_SURFACE_T *surface)
{
   KHRN_MEM_ASSIGN(surface->age_damage_state.damage_rects, NULL);
   egl_surface_base_delete_aux_bufs(surface);
   vcos_mutex_delete(&surface->lock);
}

EGLint egl_surface_base_get_attrib(EGL_SURFACE_T *surface, EGLint attrib, EGLint *value)
{
   switch (attrib)
   {
   case EGL_GL_COLORSPACE:
      if (surface->gl_colorspace == SRGB)
         *value = EGL_GL_COLORSPACE_SRGB;
      else
         *value = EGL_GL_COLORSPACE_LINEAR;
      return EGL_SUCCESS;

   case EGL_VG_ALPHA_FORMAT:
      *value = EGL_VG_ALPHA_FORMAT_NONPRE;
      return EGL_SUCCESS;

   case EGL_VG_COLORSPACE:
      *value = EGL_VG_COLORSPACE_sRGB;
      return EGL_SUCCESS;

   case EGL_CONFIG_ID:
      *value = egl_config_get_attrib(surface->config, EGL_CONFIG_ID, NULL);
      return EGL_SUCCESS;

   case EGL_HORIZONTAL_RESOLUTION:
   case EGL_VERTICAL_RESOLUTION:
      *value = EGL_UNKNOWN;
      return EGL_SUCCESS;

   case EGL_PIXEL_ASPECT_RATIO:
      *value = EGL_DISPLAY_SCALING;
      return EGL_SUCCESS;

   case EGL_RENDER_BUFFER:
      if (surface == NULL)
         *value = EGL_NONE;
      else if (surface->type == EGL_SURFACE_TYPE_PIXMAP)
         *value = EGL_SINGLE_BUFFER;
      else
         *value = EGL_BACK_BUFFER;
      return EGL_SUCCESS;

   case EGL_SWAP_BEHAVIOR:
      *value = EGL_BUFFER_DESTROYED;
      return EGL_SUCCESS;

   case EGL_MULTISAMPLE_RESOLVE:
      *value = surface->multisample_resolve;
      return EGL_SUCCESS;

   case EGL_WIDTH:
      {
         if (surface->type == EGL_SURFACE_TYPE_NATIVE_WINDOW)
         {
            khrn_image *back_buffer = egl_surface_get_back_buffer(surface);

            if (back_buffer)
            {
               unsigned width;
               khrn_image_get_dimensions(back_buffer, &width, NULL, NULL, NULL);
               *value = width;
            }
            else
               *value = surface->width;
         }
         else
            *value = surface->width;

      }
      return EGL_SUCCESS;

   case EGL_HEIGHT:
      {
         if (surface->type == EGL_SURFACE_TYPE_NATIVE_WINDOW)
         {
            khrn_image *back_buffer = egl_surface_get_back_buffer(surface);

            if (back_buffer)
            {
               unsigned height;
               khrn_image_get_dimensions(back_buffer, NULL, &height, NULL, NULL);
               *value = height;
            }
            else
               *value = surface->height;
         }
         else
            *value = surface->height;
      }
      return EGL_SUCCESS;

   case EGL_BUFFER_AGE_EXT:
      {
         if (surface->type == EGL_SURFACE_TYPE_NATIVE_WINDOW && !khrn_options.disable_buffer_age)
         {
            EGL_CONTEXT_T *context = egl_thread_get_context();
            if (!context || context->draw != surface)
               return EGL_BAD_SURFACE;

            EGL_BUFFER_AGE_DAMAGE_STATE_T *state = &surface->age_damage_state;

            state->buffer_age_queried = true; // Queried this frame
            state->buffer_age_enabled = true;

            *value = state->buffer_age_override;
         }
         else
            *value = 0;  // Non-postable buffers have age 0
      }
      log_trace("Buffer age query = %d", *value);
      return EGL_SUCCESS;

   case EGL_LARGEST_PBUFFER:
   case EGL_TEXTURE_FORMAT:
   case EGL_TEXTURE_TARGET:
   case EGL_MIPMAP_TEXTURE:
   case EGL_MIPMAP_LEVEL:
      /* Querying these for a non-pbuffer surface is not an error, but value is
       * not modified */
      return EGL_SUCCESS;

   default:
      break;
   }
   return EGL_BAD_ATTRIBUTE;
}

EGLint egl_surface_base_init_attrib(EGL_SURFACE_T *surface,
      EGLint attrib, EGLAttribKHR value)
{
   switch (attrib)
   {
   case EGL_GL_COLORSPACE:
      switch (value)
      {
      case EGL_GL_COLORSPACE_SRGB:
         surface->gl_colorspace = SRGB;
         return EGL_SUCCESS;
      case EGL_GL_COLORSPACE_LINEAR:
         surface->gl_colorspace = LINEAR;
         return EGL_SUCCESS;
      default:
         return EGL_BAD_PARAMETER;
      }

   case EGL_VG_COLORSPACE:
      switch (value)
      {
      case EGL_VG_COLORSPACE_sRGB:
         return EGL_SUCCESS;
      case EGL_VG_COLORSPACE_LINEAR:
         return EGL_BAD_MATCH;
      default:
         return EGL_BAD_PARAMETER;
      }

   case EGL_VG_ALPHA_FORMAT:
      switch (value)
      {
      case EGL_VG_ALPHA_FORMAT_NONPRE:
         return EGL_SUCCESS;
      case EGL_VG_ALPHA_FORMAT_PRE:
         return EGL_BAD_MATCH;
      default:
         return EGL_BAD_PARAMETER;
      }

   case EGL_RENDER_BUFFER:
      switch (value)
      {
         /*
          * We only support BACK_BUFFER rendering, but it's ok just to not
          * "respect" any attempt to set this to anything else (EGL 1.4 3.5.1)
          */
      case EGL_BACK_BUFFER:
      case EGL_SINGLE_BUFFER:
         return EGL_SUCCESS;
      default:
         return EGL_BAD_PARAMETER;
      }

#if EGL_EXT_protected_content
   case EGL_PROTECTED_CONTENT_EXT:
      if (value == EGL_TRUE || value == EGL_FALSE)
      {
         surface->secure = value;
         return EGL_SUCCESS;
      }
      else
         return EGL_BAD_PARAMETER;
#endif

   default:
      return EGL_BAD_ATTRIBUTE;
   }
}

EGLint egl_surface_base_set_attrib(EGL_SURFACE_T *surface,
      EGLint attrib, EGLAttribKHR value)
{
   switch (attrib)
   {
   case EGL_MIPMAP_LEVEL:
      // May be set on non-pbuffer surfaces, but doing so has no effect
      return EGL_SUCCESS;

   case EGL_SWAP_BEHAVIOR:
      switch (value)
      {
      case EGL_BUFFER_PRESERVED:
         return EGL_BAD_MATCH;
      case EGL_BUFFER_DESTROYED:
         return EGL_SUCCESS;
      default:
         return EGL_BAD_PARAMETER;
      }

   case EGL_MULTISAMPLE_RESOLVE:
      switch (value)
      {
      case EGL_MULTISAMPLE_RESOLVE_BOX:
         if (!(egl_config_get_attrib(surface->config, EGL_SURFACE_TYPE, NULL)
            & EGL_MULTISAMPLE_RESOLVE_BOX_BIT))
            return EGL_BAD_MATCH;
         break;
      case EGL_MULTISAMPLE_RESOLVE_DEFAULT:
         break;
      default:
         return EGL_BAD_PARAMETER;
      }

      surface->multisample_resolve = (EGLenum)value;
      return EGL_SUCCESS;

   default:
      return EGL_BAD_ATTRIBUTE;
   }
}

bool egl_surface_base_init_aux_bufs(EGL_SURFACE_T *surface)
{
   const EGL_CONFIG_T *config = surface->config;
   GFX_LFMT_T color_format = egl_api_fmt_to_lfmt(config->color_api_fmt, config->x_padded);
   if (surface->gl_colorspace == SRGB)
      color_format = egl_surface_lfmt_to_srgb(color_format);

   // TODO This currently only handles single plane image formats,
   //      formats like D32 S8 would need some extra work.
   struct
   {
      GFX_LFMT_T           api_fmt;
      GFX_LFMT_T           image_fmt;
      gfx_buffer_usage_t   flags;
      unsigned             multiplier;
      unsigned             bpp;
   }
   params[AUX_MAX] =
   {
      /* depth and stencil */
      {
         config->depth_stencil_api_fmt,
         config->depth_stencil_api_fmt,
         GFX_BUFFER_USAGE_V3D_TEXTURE | GFX_BUFFER_USAGE_V3D_TLB,
         1,
         egl_config_get_attrib(config, EGL_DEPTH_SIZE, NULL),
      },

      /* stencil */
      {
         config->stencil_api_fmt,
         config->stencil_api_fmt,
         GFX_BUFFER_USAGE_V3D_TEXTURE | GFX_BUFFER_USAGE_V3D_TLB,
         1,
         egl_config_get_attrib(config, EGL_STENCIL_SIZE, NULL),
      },

      /* multisample */
      {
         config->color_api_fmt,
#if V3D_VER_AT_LEAST(4,1,34,0)
         color_format,
#else
         gfx_lfmt_translate_internal_raw_mode(color_format),
#endif
         GFX_BUFFER_USAGE_V3D_TEXTURE | GFX_BUFFER_USAGE_V3D_RENDER_TARGET
#if !V3D_VER_AT_LEAST(4,1,34,0)
         | GFX_BUFFER_USAGE_V3D_TLB_RAW
#endif
         ,
         2,
         0,
      }
   };

   if (config->samples == 4)
   {
      for (egl_aux_buf_t i = AUX_DEPTH; i <= AUX_STENCIL; i++)
      {
         params[i].multiplier *= 2;
#if !V3D_VER_AT_LEAST(4,1,34,0)
         params[i].flags |= GFX_BUFFER_USAGE_V3D_TLB_RAW;
#endif
      }
   }
   else
   {
      assert(config->samples == 0);

      params[AUX_MULTISAMPLE].api_fmt = GFX_LFMT_NONE;
      params[AUX_MULTISAMPLE].image_fmt = GFX_LFMT_NONE;
   }

   bool secure = !!(surface->secure);

   for (egl_aux_buf_t i = 0; i < AUX_MAX; i++)
   {
      if (params[i].api_fmt == GFX_LFMT_NONE)
         continue;

      GFX_LFMT_T lfmt = gfx_lfmt_to_2d(params[i].image_fmt);
      unsigned   k    = params[i].multiplier;
      khrn_blob *blob = khrn_blob_create_no_storage(
                              k * surface->width,
                              k * surface->height, 1, 1, 1, &lfmt, 1,
                              params[i].flags, secure);
      if (!blob) return false;

      khrn_image *image = khrn_image_create(blob, 0, 1, 0, params[i].api_fmt);
      KHRN_MEM_ASSIGN(blob, NULL);
      if (!image) return false;

      surface->aux_bufs[i].image = image;
   }

   if (surface->aux_bufs[AUX_DEPTH].image)
   {
      if (gfx_lfmt_has_stencil(surface->aux_bufs[AUX_DEPTH].image->api_fmt))
      {
         /* if we have depth and stencil, both buffers should point to the same
          * image */
         assert(surface->aux_bufs[AUX_STENCIL].image == NULL);
         KHRN_MEM_ASSIGN(surface->aux_bufs[AUX_STENCIL].image,
               surface->aux_bufs[AUX_DEPTH].image);
      }
   }

   return true;
}

khrn_image *egl_surface_base_get_aux_buffer(const EGL_SURFACE_T *surface,
      egl_aux_buf_t which)
{
   assert(which < AUX_MAX);
   return surface->aux_bufs[which].image;
}

void egl_surface_base_delete_aux_bufs(EGL_SURFACE_T *surface)
{
   egl_aux_buf_t i;

   for (i = 0; i < AUX_MAX; i++)
   {
      KHRN_MEM_ASSIGN(surface->aux_bufs[i].image, NULL);
      memset(surface->aux_bufs + i, 0, sizeof (EGL_AUX_BUF_T));
   }
}
