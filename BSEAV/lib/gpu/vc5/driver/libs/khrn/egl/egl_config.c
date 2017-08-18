/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../common/khrn_int_common.h"
#include "../common/khrn_int_util.h"
#include "egl_thread.h"
#include "egl_display.h"
#include "egl_platform.h"
#include "egl_config.h"
#include "egl_context_gl.h"
#include "egl_surface_base.h"

#include "libs/core/lfmt/lfmt_translate_v3d.h"

/*
 * Note: "EGL Config ids" are position of config in this array +1. Any
 * application that uses EGL_CONFIG_ID in the attrs passed to eglChooseConfig
 * to choose a specific config (for example our regression tests) will receive
 * a surprise if you change the order of the configs in this array.
 */
static const EGL_CONFIG_T egl_configs[] = {
   // EGLConfig
   // |    SAMPLES
   // |    |  LOCKABLE
   // |    |  |  COLOR                       DEPTH                      STENCIL           MASK (OpenVG)  INVALID
   /* 1*/ {0, 0, GFX_LFMT_R8_G8_B8_A8_UNORM, GFX_LFMT_S8D24_UINT_UNORM, GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /* 2*/ {0, 0, GFX_LFMT_R8_G8_B8_X8_UNORM, GFX_LFMT_S8D24_UINT_UNORM, GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /* 3*/ {0, 0, GFX_LFMT_R8_G8_B8_A8_UNORM, GFX_LFMT_D24X8_UNORM,      GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /* 4*/ {0, 0, GFX_LFMT_R8_G8_B8_X8_UNORM, GFX_LFMT_D24X8_UNORM,      GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /* 5*/ {0, 0, GFX_LFMT_R8_G8_B8_A8_UNORM, GFX_LFMT_NONE,             GFX_LFMT_S8_UINT, GFX_LFMT_NONE},
   /* 6*/ {0, 0, GFX_LFMT_R8_G8_B8_X8_UNORM, GFX_LFMT_NONE,             GFX_LFMT_S8_UINT, GFX_LFMT_NONE},
   /* 7*/ {0, 0, GFX_LFMT_R8_G8_B8_A8_UNORM, GFX_LFMT_NONE,             GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /* 8*/ {0, 0, GFX_LFMT_R8_G8_B8_X8_UNORM, GFX_LFMT_NONE,             GFX_LFMT_NONE,    GFX_LFMT_NONE},

   /* 9*/ {4, 0, GFX_LFMT_R8_G8_B8_A8_UNORM, GFX_LFMT_S8D24_UINT_UNORM, GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*10*/ {4, 0, GFX_LFMT_R8_G8_B8_X8_UNORM, GFX_LFMT_S8D24_UINT_UNORM, GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*11*/ {4, 0, GFX_LFMT_R8_G8_B8_A8_UNORM, GFX_LFMT_D24X8_UNORM,      GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*12*/ {4, 0, GFX_LFMT_R8_G8_B8_X8_UNORM, GFX_LFMT_D24X8_UNORM,      GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*13*/ {4, 0, GFX_LFMT_R8_G8_B8_A8_UNORM, GFX_LFMT_NONE,             GFX_LFMT_S8_UINT, GFX_LFMT_NONE},
   /*14*/ {4, 0, GFX_LFMT_R8_G8_B8_X8_UNORM, GFX_LFMT_NONE,             GFX_LFMT_S8_UINT, GFX_LFMT_NONE},
   /*15*/ {4, 0, GFX_LFMT_R8_G8_B8_A8_UNORM, GFX_LFMT_NONE,             GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*16*/ {4, 0, GFX_LFMT_R8_G8_B8_X8_UNORM, GFX_LFMT_NONE,             GFX_LFMT_NONE,    GFX_LFMT_NONE},

   /*17*/ {0, 0, GFX_LFMT_B5G6R5_UNORM,      GFX_LFMT_S8D24_UINT_UNORM, GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*18*/ {0, 0, GFX_LFMT_B5G6R5_UNORM,      GFX_LFMT_D24X8_UNORM,      GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*19*/ {0, 0, GFX_LFMT_B5G6R5_UNORM,      GFX_LFMT_NONE,             GFX_LFMT_S8_UINT, GFX_LFMT_NONE},
   /*20*/ {0, 0, GFX_LFMT_B5G6R5_UNORM,      GFX_LFMT_NONE,             GFX_LFMT_NONE,    GFX_LFMT_NONE},

   /*21*/ {4, 0, GFX_LFMT_B5G6R5_UNORM,      GFX_LFMT_S8D24_UINT_UNORM, GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*22*/ {4, 0, GFX_LFMT_B5G6R5_UNORM,      GFX_LFMT_D24X8_UNORM,      GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*23*/ {4, 0, GFX_LFMT_B5G6R5_UNORM,      GFX_LFMT_NONE,             GFX_LFMT_S8_UINT, GFX_LFMT_NONE},
   /*24*/ {4, 0, GFX_LFMT_B5G6R5_UNORM,      GFX_LFMT_NONE,             GFX_LFMT_NONE,    GFX_LFMT_NONE},

   /*25*/ {0, 0, GFX_LFMT_B5G6R5_UNORM,      GFX_LFMT_D16_UNORM,        GFX_LFMT_NONE,    GFX_LFMT_NONE},

   /*26*/ {0, 0, GFX_LFMT_R8_G8_B8_UNORM,    GFX_LFMT_S8D24_UINT_UNORM, GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*27*/ {0, 0, GFX_LFMT_R8_G8_B8_UNORM,    GFX_LFMT_D24X8_UNORM,      GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*28*/ {0, 0, GFX_LFMT_R8_G8_B8_UNORM,    GFX_LFMT_NONE,             GFX_LFMT_S8_UINT, GFX_LFMT_NONE},
   /*29*/ {0, 0, GFX_LFMT_R8_G8_B8_UNORM,    GFX_LFMT_NONE,             GFX_LFMT_NONE,    GFX_LFMT_NONE},

   /* This config should look the same as config 1 except for having a larger
    * ID. So it should never be the top config returned by eglChooseConfig() or
    * eglGetConfigs() -- if it appears, config 1 will always appear earlier. */
   /*30*/ {0, 0, GFX_LFMT_BSTC_RGBA_UNORM,   GFX_LFMT_S8D24_UINT_UNORM, GFX_LFMT_NONE,    GFX_LFMT_NONE, !V3D_VER_AT_LEAST(3,3,0,0)},

   /*31*/ {0, 0, GFX_LFMT_A4B4G4R4_UNORM,    GFX_LFMT_S8D24_UINT_UNORM, GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*32*/ {0, 0, GFX_LFMT_A4B4G4R4_UNORM,    GFX_LFMT_D24X8_UNORM,      GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*33*/ {0, 0, GFX_LFMT_A4B4G4R4_UNORM,    GFX_LFMT_NONE,             GFX_LFMT_S8_UINT, GFX_LFMT_NONE},
   /*34*/ {0, 0, GFX_LFMT_A4B4G4R4_UNORM,    GFX_LFMT_NONE,             GFX_LFMT_NONE,    GFX_LFMT_NONE},

   /*35*/{ 4, 0, GFX_LFMT_A4B4G4R4_UNORM,    GFX_LFMT_S8D24_UINT_UNORM, GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*36*/{ 4, 0, GFX_LFMT_A4B4G4R4_UNORM,    GFX_LFMT_D24X8_UNORM,      GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*37*/{ 4, 0, GFX_LFMT_A4B4G4R4_UNORM,    GFX_LFMT_NONE,             GFX_LFMT_S8_UINT, GFX_LFMT_NONE},
   /*38*/{ 4, 0, GFX_LFMT_A4B4G4R4_UNORM,    GFX_LFMT_NONE,             GFX_LFMT_NONE,    GFX_LFMT_NONE},

#if 0
   /* The 1-bit alpha component might cause some surprises if we add 5551 configs as it is internally
    * maintained at higher precision. This can sometimes lead to unexpected results, so for now, we won't
    * expose these formats.
    */
   /*39*/ {0, 0, GFX_LFMT_A1B5G5R5_UNORM,    GFX_LFMT_S8D24_UINT_UNORM, GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*40*/ {0, 0, GFX_LFMT_A1B5G5R5_UNORM,    GFX_LFMT_D24X8_UNORM,      GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*41*/ {0, 0, GFX_LFMT_A1B5G5R5_UNORM,    GFX_LFMT_NONE,             GFX_LFMT_S8_UINT, GFX_LFMT_NONE},
   /*42*/ {0, 0, GFX_LFMT_A1B5G5R5_UNORM,    GFX_LFMT_NONE,             GFX_LFMT_NONE,    GFX_LFMT_NONE},

   /*43*/{ 4, 0, GFX_LFMT_A1B5G5R5_UNORM,    GFX_LFMT_S8D24_UINT_UNORM, GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*44*/{ 4, 0, GFX_LFMT_A1B5G5R5_UNORM,    GFX_LFMT_D24X8_UNORM,      GFX_LFMT_NONE,    GFX_LFMT_NONE},
   /*45*/{ 4, 0, GFX_LFMT_A1B5G5R5_UNORM,    GFX_LFMT_NONE,             GFX_LFMT_S8_UINT, GFX_LFMT_NONE},
   /*46*/{ 4, 0, GFX_LFMT_A1B5G5R5_UNORM,    GFX_LFMT_NONE,             GFX_LFMT_NONE,    GFX_LFMT_NONE},
#endif
};

typedef struct
{
   bool     use_red;
   bool     use_green;
   bool     use_blue;
   bool     use_alpha;
}
SORT_PARAMS_T;

typedef struct
{
   const EGL_CONFIG_T    *descriptor;
   const SORT_PARAMS_T   *sort_params;
}
CONFIG_CHOICE_T;

static const EGLint s_api_bits = EGL_OPENGL_BIT | EGL_OPENVG_BIT |
                                 EGL_OPENGL_ES_BIT | EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES3_BIT_KHR;

bool egl_config_is_valid(const EGL_CONFIG_T *config)
{
   const EGL_CONFIG_T *start = &egl_configs[0];
   const EGL_CONFIG_T *end = start + countof(egl_configs);
   ptrdiff_t offset;

   if (config < start || config >= end)
      return false;

   offset = (char *) config - (char *) start;
   if (offset % sizeof (EGL_CONFIG_T))
      return false;

   if (config->invalid)
      return false;

   return true;
}

static bool bindable_rgb(const EGL_CONFIG_T *config)
{
   GFX_LFMT_T c;
   uint32_t alpha_bits;

   c = config->color_api_fmt;
   alpha_bits = (c != GFX_LFMT_NONE) ? gfx_lfmt_alpha_bits(c) : 0;

   return config->samples == 0 && alpha_bits == 0;
}

bool egl_config_bindable(const EGL_CONFIG_T *config, EGLenum format)
{
   assert(egl_config_is_valid(config));

   switch (format)
   {
   case EGL_NO_TEXTURE:
      return true;

   case EGL_TEXTURE_RGB:
      return bindable_rgb(config);

   case EGL_TEXTURE_RGBA:
      return config->samples == 0;

   default:
      unreachable();
      return false;
   }
}

static bool get_attrib(const EGL_CONFIG_T *config,
      EGLint attrib, EGLint *value)
{
   GFX_LFMT_T c, d, s, m;

   assert(egl_config_is_valid(config));

   c = config->color_api_fmt;
   d = config->depth_stencil_api_fmt;
   s = config->stencil_api_fmt;
   m = config->mask_api_fmt;

   assert(c != GFX_LFMT_NONE);

   switch (attrib) {
   case EGL_BUFFER_SIZE:
      *value = (c != GFX_LFMT_NONE) ? (gfx_lfmt_red_bits(c) + gfx_lfmt_green_bits(c) + gfx_lfmt_blue_bits(c) + gfx_lfmt_alpha_bits(c)) : 0;
      return true;
   case EGL_RED_SIZE:
      *value = (c != GFX_LFMT_NONE) ? gfx_lfmt_red_bits(c) : 0;
      return true;
   case EGL_GREEN_SIZE:
      *value = (c != GFX_LFMT_NONE) ? gfx_lfmt_green_bits(c) : 0;
      return true;
   case EGL_BLUE_SIZE:
      *value = (c != GFX_LFMT_NONE) ? gfx_lfmt_blue_bits(c) : 0;
      return true;
   case EGL_LUMINANCE_SIZE:
      *value = 0;
      return true;
   case EGL_ALPHA_SIZE:
      *value = (c != GFX_LFMT_NONE) ? gfx_lfmt_alpha_bits(c) : 0;
      return true;
   case EGL_ALPHA_MASK_SIZE:
      *value = (m != GFX_LFMT_NONE) ? gfx_lfmt_alpha_bits(m) : 0;
      return true;
   case EGL_BIND_TO_TEXTURE_RGB:
      *value = bindable_rgb(config);
      return true;
   case EGL_BIND_TO_TEXTURE_RGBA:
      *value = config->samples == 0;
      return true;
   case EGL_COLOR_BUFFER_TYPE:
      *value = EGL_RGB_BUFFER;
      return true;
   case EGL_CONFIG_CAVEAT:
      *value = egl_config_get_api_conformance(config) ?
         EGL_NONE : EGL_NON_CONFORMANT_CONFIG;
      return true;
   case EGL_CONFIG_ID:
      /* These are required to start at 1 (see EGL 1.4 3.4) */
      *value = (config - egl_configs) + 1;
      return true;
   case EGL_CONFORMANT:
      *value = egl_config_get_api_conformance(config);
      return true;
   case EGL_DEPTH_SIZE:
      *value = (d != GFX_LFMT_NONE) ? gfx_lfmt_depth_bits(d) : 0;
      return true;
   case EGL_LEVEL:
      *value = 0;
      return true;
   case EGL_MATCH_NATIVE_PIXMAP:
      *value = 0;
      return true;
   case EGL_MAX_PBUFFER_WIDTH:
      *value = EGL_CONFIG_MAX_WIDTH;
      return true;
   case EGL_MAX_PBUFFER_HEIGHT:
      *value = EGL_CONFIG_MAX_HEIGHT;
      return true;
   case EGL_MAX_PBUFFER_PIXELS:
      *value = EGL_CONFIG_MAX_WIDTH * EGL_CONFIG_MAX_HEIGHT;
      return true;
   case EGL_MAX_SWAP_INTERVAL:
      *value = EGL_CONFIG_MAX_SWAP_INTERVAL;
      return true;
   case EGL_MIN_SWAP_INTERVAL:
      *value = EGL_CONFIG_MIN_SWAP_INTERVAL;
      return true;
   case EGL_NATIVE_RENDERABLE:
      *value = EGL_TRUE;
      return true;
   case EGL_NATIVE_VISUAL_ID:
      egl_platform_color_format(config->color_api_fmt, value);
      return true;
   case EGL_NATIVE_VISUAL_TYPE:
      *value = EGL_NONE;
      return true;
   case EGL_RENDERABLE_TYPE:
      *value = egl_config_get_api_support(config);
      return true;
   case EGL_SAMPLE_BUFFERS:
      *value = config->samples > 0 ? 1 : 0;
      return true;
   case EGL_SAMPLES:
      *value = config->samples;
      return true;
   case EGL_STENCIL_SIZE:
      if (d != GFX_LFMT_NONE)
         *value = gfx_lfmt_stencil_bits(d);
      else if (s != GFX_LFMT_NONE)
         *value = gfx_lfmt_stencil_bits(s);
      else
         *value = 0;
      return true;
   case EGL_SURFACE_TYPE:
      *value = (EGLint)(
         EGL_VG_COLORSPACE_LINEAR_BIT    |
         EGL_VG_ALPHA_FORMAT_PRE_BIT     |
         EGL_MULTISAMPLE_RESOLVE_BOX_BIT);

      if (egl_can_render_format(c))
      {
         bool can_display = egl_can_display_format(c);
         bool can_texture = egl_can_texture_from_format(c);

         if (can_display && can_texture)  // Pixmaps are used as EGLimages, so must be texturable too
            *value = *value | EGL_PIXMAP_BIT;

         if (can_texture)  // We don't have to be able to display pbuffers
            *value = *value | EGL_PBUFFER_BIT;

         if (can_display)  // We don't have to be able to texture window buffers
            *value = *value | EGL_WINDOW_BIT;
      }
      return true;
   case EGL_TRANSPARENT_TYPE:
      *value = EGL_NONE;
      return true;
   case EGL_TRANSPARENT_RED_VALUE:
   case EGL_TRANSPARENT_GREEN_VALUE:
   case EGL_TRANSPARENT_BLUE_VALUE:
      *value = 0;
      return true;
   default:
      return false;
   }
}

static EGLint color_size(const EGL_CONFIG_T *config,
      const SORT_PARAMS_T *params)
{
   EGLint ret = 0;
   GFX_LFMT_T color = config->color_api_fmt;

   if (params->use_red)
      ret += gfx_lfmt_red_bits(color);
   if (params->use_green)
      ret += gfx_lfmt_green_bits(color);
   if (params->use_blue)
      ret += gfx_lfmt_blue_bits(color);
   if (params->use_alpha)
      ret += gfx_lfmt_alpha_bits(color);

   return ret;
}

static int get_caveat_sort_priority(EGLenum caveat)
{
   static const EGLenum caveat_order[3] = { EGL_NONE, EGL_SLOW_CONFIG, EGL_NON_CONFORMANT_CONFIG };
   for (int i=0; i<3; i++)
      if (caveat == caveat_order[i])
         return i;

   unreachable();
   return 4;
}

static int compar(const void *a, const void *b)
{
   const CONFIG_CHOICE_T *choice_a = a;
   const CONFIG_CHOICE_T *choice_b = b;
   const EGL_CONFIG_T *conf_a = choice_a->descriptor;
   const EGL_CONFIG_T *conf_b = choice_b->descriptor;
   const SORT_PARAMS_T *params = choice_a->sort_params;
   bool less_than = false;
   static const EGLint attrs[9] =
   {
      /*
       * These are in sort priority order (see EGL 1.4 3.4.1.2), leaving out
       * EGL_COLOR_BUFFER_TYPE, since all our configs are the same.
       */
      EGL_CONFIG_CAVEAT,
      EGL_RED_SIZE,
      EGL_BUFFER_SIZE,
      EGL_SAMPLE_BUFFERS,
      EGL_SAMPLES,
      EGL_DEPTH_SIZE,
      EGL_STENCIL_SIZE,
      EGL_ALPHA_MASK_SIZE,
      EGL_CONFIG_ID,
   };

   for (int i = 0; i < 9; i++)
   {
      EGLint attr = attrs[i];
      EGLint value_a, value_b;

      /*
       * Red size is compared in a "special" way-- you add up all the
       * components you're interested in. Green, blue and alpha size are
       * exactly the same, so there's no point comparing on those as well.
       */
      if (attr == EGL_RED_SIZE)
      {
         value_a = color_size(conf_a, params);
         value_b = color_size(conf_b, params);

         if (value_a == value_b)
            continue;

         /*
          * Configs with larger numbers of colour bits are preferred, so are
          * "less than" from the point of view of the sort.
          */
         less_than = value_a > value_b;
      }
      else if (attr == EGL_CONFIG_CAVEAT)
      {
         value_a = egl_config_get_attrib(conf_a, attr, NULL);
         value_b = egl_config_get_attrib(conf_b, attr, NULL);
         int order_a = get_caveat_sort_priority(value_a);
         int order_b = get_caveat_sort_priority(value_b);

         if (value_a == value_b)
            continue;

         less_than = order_a < order_b;
      }
      else
      {
         value_a = egl_config_get_attrib(conf_a, attr, NULL);
         value_b = egl_config_get_attrib(conf_b, attr, NULL);

         if (value_a == value_b)
            continue;

         /* Smaller is preferred for all these */
         less_than = value_a < value_b;
      }
      break;
   }
   return less_than ? -1 : 1;
}

/*
 * Returns true if all attributes and values are valid.
 */
static bool check_attribs(const EGLint *attrib_list)
{
   if (!attrib_list)
      return true;

   while (*attrib_list != EGL_NONE)
   {
      EGLint name = *attrib_list++;
      EGLint value = *attrib_list++;

      if (egl_platform_config_check_attrib(name, value))
         continue;

      switch (name)
      {
      case EGL_RED_SIZE:
      case EGL_GREEN_SIZE:
      case EGL_BLUE_SIZE:
      case EGL_ALPHA_SIZE:
      case EGL_BUFFER_SIZE:
      case EGL_LUMINANCE_SIZE:
      case EGL_ALPHA_MASK_SIZE:
         if (value != EGL_DONT_CARE && value < 0) return false;
         break;
      case EGL_BIND_TO_TEXTURE_RGB:
      case EGL_BIND_TO_TEXTURE_RGBA:
         if (value != EGL_DONT_CARE && value != EGL_FALSE && value != EGL_TRUE)
            return false;
         break;
      case EGL_COLOR_BUFFER_TYPE:
         if (value != EGL_DONT_CARE && value != EGL_RGB_BUFFER && value != EGL_LUMINANCE_BUFFER)
            return false;
         break;
      case EGL_CONFIG_CAVEAT:
         if (value != EGL_DONT_CARE && value != EGL_NONE && value != EGL_SLOW_CONFIG && value != EGL_NON_CONFORMANT_CONFIG)
            return false;
         break;
      case EGL_CONFIG_ID:
         if (value != EGL_DONT_CARE && value < 1)
            return false;
         break;
      case EGL_CONFORMANT:
         if (value != EGL_DONT_CARE && (value & ~s_api_bits))
            return false;
         break;
      case EGL_DEPTH_SIZE:
         if (value != EGL_DONT_CARE && value < 0) return false;
         break;
      case EGL_LEVEL:
         break;
      case EGL_MATCH_NATIVE_PIXMAP:
         /* The pixmap value should have been checked in egl_platform_config_check_attrib above.
            If we get here it failed, so return false. */
         return false;
      case EGL_MAX_PBUFFER_WIDTH:
      case EGL_MAX_PBUFFER_HEIGHT:
      case EGL_MAX_PBUFFER_PIXELS:
         break;
      case EGL_MAX_SWAP_INTERVAL:
      case EGL_MIN_SWAP_INTERVAL:
         if (value != EGL_DONT_CARE && value < 0) return false;
         break;
      case EGL_NATIVE_RENDERABLE:
         if (value != EGL_DONT_CARE && value != EGL_FALSE && value != EGL_TRUE)
            return false;
         break;
      case EGL_NATIVE_VISUAL_ID:
      case EGL_NATIVE_VISUAL_TYPE:
         break;
      case EGL_RENDERABLE_TYPE:
         if (value != EGL_DONT_CARE && (value & ~s_api_bits))
            return false;
         break;
      case EGL_SAMPLE_BUFFERS:
      case EGL_SAMPLES:
      case EGL_STENCIL_SIZE:
         if (value != EGL_DONT_CARE && value < 0) return false;
         break;
      case EGL_SURFACE_TYPE:
      {
         int valid_bits = EGL_WINDOW_BIT|EGL_PIXMAP_BIT|EGL_PBUFFER_BIT|
            EGL_MULTISAMPLE_RESOLVE_BOX_BIT|EGL_SWAP_BEHAVIOR_PRESERVED_BIT|
            EGL_VG_COLORSPACE_LINEAR_BIT|EGL_VG_ALPHA_FORMAT_PRE_BIT;
         if (value != EGL_DONT_CARE && (value & ~valid_bits))
            return false;
         break;
      }
      case EGL_TRANSPARENT_TYPE:
         if (value != EGL_DONT_CARE && value != EGL_NONE && value != EGL_TRANSPARENT_RGB)
            return false;
         break;
      case EGL_TRANSPARENT_RED_VALUE:
      case EGL_TRANSPARENT_GREEN_VALUE:
      case EGL_TRANSPARENT_BLUE_VALUE:
         if (value != EGL_DONT_CARE && value < 0) return false;
         break;
      default:
         return false;
      }
   }

   return true;
}

/*
   Returns whether the given EGL config satisfies the supplied attrib_list.

   Implementation notes:

   The following attributes:
   EGL_COLOR_BUFFER_TYPE
   EGL_LEVEL
   EGL_RENDERABLE_TYPE
   EGL_SURFACE_TYPE
   EGL_TRANSPARENT_TYPE
   (possibly EGL_MATCH_NATIVE_PIXMAP - see comment)

   have default values not equivalent to EGL_DONT_CARE. But all of our configs
   match the default value in all of these cases so we can treat the default as
   EGL_DONT_CARE for all attributes.
*/
static bool config_matches(const EGL_CONFIG_T *config,
      const EGLint *attrib_list)
{
   if (!attrib_list)
      return true;

   while (*attrib_list != EGL_NONE) {
      EGLint name = *attrib_list++;
      EGLint value = *attrib_list++;
      EGLint actual_value;
      bool valid;

      actual_value = egl_config_get_attrib(config, name, &valid);

      if (!valid)
         return false;

      if (egl_platform_config_check_attrib(name, actual_value))
      {
         if (egl_platform_config_match_attrib(name, value, actual_value)) {
            continue;
         }
         else {
            return false;
         }
      }

      switch (name)
      {
         /* Selection Criteria: AtLeast */
      case EGL_BUFFER_SIZE:
      case EGL_RED_SIZE:
      case EGL_GREEN_SIZE:
      case EGL_BLUE_SIZE:
      case EGL_LUMINANCE_SIZE:
      case EGL_ALPHA_SIZE:
      case EGL_ALPHA_MASK_SIZE:
      case EGL_DEPTH_SIZE:
      case EGL_SAMPLE_BUFFERS:
      case EGL_SAMPLES:
      case EGL_STENCIL_SIZE:
         if (value != EGL_DONT_CARE && value > actual_value)
            return false;
         break;

         /* Selection Criteria: Exact */
         /*
            Excluding EGL_TRANSPARENT_x_VALUE and EGL_MATCH_FORMAT_KHR which are listed in
            the table as Exact, but seem to have special rules attached to them.

            Excluding EGL_NATIVE_VISUAL_TYPE which is in the ignore list
            Excluding EGL_LEVEL because EGL_DONT_CARE is not allowed
            */
      case EGL_BIND_TO_TEXTURE_RGB:
      case EGL_BIND_TO_TEXTURE_RGBA:
      case EGL_COLOR_BUFFER_TYPE:
      case EGL_CONFIG_CAVEAT:
      case EGL_CONFIG_ID:
      case EGL_MAX_SWAP_INTERVAL:
      case EGL_MIN_SWAP_INTERVAL:
      case EGL_NATIVE_RENDERABLE:
      case EGL_TRANSPARENT_TYPE:
         if (value != EGL_DONT_CARE && value != actual_value)
            return false;
         break;

      case EGL_LEVEL:
         if (value != actual_value)
            return false;
         break;

         /* Selection Criteria: Mask */
      case EGL_CONFORMANT:
      case EGL_RENDERABLE_TYPE:
      case EGL_SURFACE_TYPE:
         if (value != EGL_DONT_CARE && (value & ~actual_value))
            return false;
         break;

         /* Selection Criteria: Special */
      case EGL_MATCH_NATIVE_PIXMAP:
         if (!egl_platform_match_pixmap((EGLNativePixmapType)(intptr_t)value, config))
            return false;
         break;

         /* Attributes we can completely ignore */
      case EGL_MAX_PBUFFER_WIDTH:
      case EGL_MAX_PBUFFER_HEIGHT:
      case EGL_MAX_PBUFFER_PIXELS:
      case EGL_NATIVE_VISUAL_ID:
         /*
            "If EGL_MAX_PBUFFER_WIDTH, EGL_MAX_PBUFFER_HEIGHT,
            EGL_MAX_PBUFFER_PIXELS, or EGL_NATIVE_VISUAL_ID are specified in
            attrib_list, then they are ignored"
            */

      case EGL_NATIVE_VISUAL_TYPE:
         /*
            "if there are no native visual types, then the EGL NATIVE VISUAL TYPE attribute is
            ignored."
            */

      case EGL_TRANSPARENT_BLUE_VALUE:
      case EGL_TRANSPARENT_GREEN_VALUE:
      case EGL_TRANSPARENT_RED_VALUE:
         /*
            "If EGL_TRANSPARENT_TYPE is set to EGL_NONE in attrib_list, then
            the EGL_TRANSPARENT_RED_VALUE, EGL_TRANSPARENT_GREEN_VALUE, and
            EGL_TRANSPARENT_BLUE_VALUE attributes are ignored."

            Possible spec deviation if EGL_TRANSPARENT_TYPE is specified as EGL_DONT_CARE
            and EGL_TRANSPARENT_*_VALUE is also specified?
            */

         break;

      default:
         unreachable();
         break;
      }
   }

   return true;
}

EGLint egl_config_get_attrib(const EGL_CONFIG_T *config,
      EGLint attrib, bool *valid)
{
   EGLint ret = 0;
   bool is_valid = egl_platform_config_get_attrib(config, attrib, &ret);
   if (!is_valid)
      is_valid = get_attrib(config, attrib, &ret);

   if (valid) *valid = is_valid;
   return ret;
}

/* Result is a bitmap which is a subset of s_api_bits */
uint32_t egl_config_get_api_support(const EGL_CONFIG_T *config)
{
   /* no configs are api-specific (ie if you can use a config with gl, you can
    * use it with vg too, and vice-versa). however, some configs have color
    * buffer formats that are incompatible with the hardware, and so can't be
    * used with any api. such configs may still be useful eg with the surface
    * locking extension... */

   return (uint32_t)(EGL_OPENGL_ES_BIT | EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES3_BIT_KHR);
}

/* Result is a bitmap which is a subset of s_api_bits */
uint32_t egl_config_get_api_conformance(const EGL_CONFIG_T *config)
{
   /* BSTC configs don't fully work. Report them as non-conformant to stop
    * dEQP/CTS testing them. */
   if (gfx_lfmt_is_bstc_family(config->color_api_fmt))
      return 0;

   /* 24-bit framebuffers can't be blitted from (we can't read them via the TMU) */
   if (gfx_lfmt_get_base(&config->color_api_fmt) == GFX_LFMT_BASE_C8_C8_C8)
      return 0;

   /* vg doesn't support multisampled surfaces properly */
   int samples = config->samples;
   return egl_config_get_api_support(config) & ~(samples > 0 ? EGL_OPENVG_BIT : 0);
}

bool egl_config_bpps_match(const EGL_CONFIG_T *config0,
      const EGL_CONFIG_T *config1)
{
   GFX_LFMT_T c0, d0, s0, m0;
   GFX_LFMT_T c1, d1, s1, m1;

   assert(egl_config_is_valid(config0));
   assert(egl_config_is_valid(config1));

   c0 = config0->color_api_fmt;
   d0 = config0->depth_stencil_api_fmt;
   s0 = config0->stencil_api_fmt;
   m0 = config0->mask_api_fmt;

   c1 = config1->color_api_fmt;
   d1 = config1->depth_stencil_api_fmt;
   s1 = config1->stencil_api_fmt;
   m1 = config1->mask_api_fmt;

   assert(c0 != GFX_LFMT_NONE);
   assert(c1 != GFX_LFMT_NONE);

   return
      gfx_lfmt_red_bits  (c0) == gfx_lfmt_red_bits  (c1) &&
      gfx_lfmt_green_bits(c0) == gfx_lfmt_green_bits(c1) &&
      gfx_lfmt_blue_bits (c0) == gfx_lfmt_blue_bits (c1) &&
      gfx_lfmt_alpha_bits(c0) == gfx_lfmt_alpha_bits(c1) &&
      (d0 != GFX_LFMT_NONE ? gfx_lfmt_depth_bits  (d0) : 0) == (d1 != GFX_LFMT_NONE ? gfx_lfmt_depth_bits  (d1) : 0) &&
      (d0 != GFX_LFMT_NONE ? gfx_lfmt_stencil_bits(d0) : 0) == (d1 != GFX_LFMT_NONE ? gfx_lfmt_stencil_bits(d1) : 0) &&
      (s0 != GFX_LFMT_NONE ? gfx_lfmt_stencil_bits(s0) : 0) == (s1 != GFX_LFMT_NONE ? gfx_lfmt_stencil_bits(s1) : 0) &&
      (m0 != GFX_LFMT_NONE ? gfx_lfmt_alpha_bits  (m0) : 0) == (m1 != GFX_LFMT_NONE ? gfx_lfmt_alpha_bits  (m1) : 0);
}

bool egl_config_context_surface_compatible(const EGL_CONTEXT_T *context,
      const EGL_SURFACE_T *surface)
{
   /*
      from section 2.2 of the (1.3) spec, a context and surface are compatible
      if:
      1) they support the same type of color buffer (rgb or luminance). this is
         trivially true for us as we only support rgb color buffers
      2) they have color buffers and ancillary buffers of the same depth
      3) the surface was created with respect to an EGLConfig supporting client
         api rendering of the same type as the api type of the context
      4) they were created with respect to the same EGLDisplay. this is
         trivially true for us as we only have one EGLDisplay
   */
   EGL_GL_CONTEXT_T *gl_ctx;
   unsigned api = 0;

   switch (context->api)
   {
   case API_OPENVG:
      api = EGL_OPENVG_BIT;
      break;
   case API_OPENGL:
      gl_ctx = (EGL_GL_CONTEXT_T *) context;
      switch (egl_context_gl_api(gl_ctx, OPENGL_ES_ANY))
      {
      case OPENGL_ES_11:
         api = EGL_OPENGL_ES_BIT;
         break;
      case OPENGL_ES_3X:
         api = EGL_OPENGL_ES3_BIT_KHR;
         break;
      default:
         unreachable();
         break;
      }
      break;
   default:
      unreachable();
      break;
   }

   if (!egl_config_bpps_match(context->config, surface->config))
      return false;

   if ((egl_config_get_api_support(surface->config) & api) == 0)
      return false;

   return true;
}

GFX_LFMT_T egl_config_colorformat(const EGL_CONFIG_T *config,
      egl_surface_colorspace_t colorspace,
      egl_surface_alphaformat_t alphaformat)
{
   GFX_LFMT_T color = config->color_api_fmt;

   if (alphaformat == PRE)
      color = gfx_lfmt_to_pre(color);

   if (colorspace == SRGB)
   {
      switch (color & ~GFX_LFMT_LAYOUT_MASK)
      {
      case GFX_LFMT_R8_G8_B8_A8_UNORM:
         color = (color & GFX_LFMT_LAYOUT_MASK)
            | GFX_LFMT_R8_G8_B8_A8_SRGB_SRGB_SRGB_UNORM;
         break;

      case GFX_LFMT_R8_G8_B8_X8_UNORM:
         color = (color & GFX_LFMT_LAYOUT_MASK) | GFX_LFMT_R8_G8_B8_X8_SRGB;
         break;

      case GFX_LFMT_R8_G8_B8_UNORM:
         color = (color & GFX_LFMT_LAYOUT_MASK) | GFX_LFMT_R8_G8_B8_SRGB;
         break;

      default:
         /* ignore sRGB-ness (EGL_KHR_gl_colorspace extension allows that) */
         break;
      }
   }

   return color;
}

GFX_LFMT_T egl_config_color_api_fmt(const EGL_CONFIG_T *config)
{
   return config->color_api_fmt;
}

GFX_LFMT_T egl_config_depth_stencil_api_fmt(const EGL_CONFIG_T *config)
{
   return config->depth_stencil_api_fmt;
}

GFX_LFMT_T egl_config_stencil_api_fmt(const EGL_CONFIG_T *config)
{
   return config->stencil_api_fmt;
}

bool egl_can_render_format(GFX_LFMT_T lfmt)
{
#if V3D_HAS_TLB_SWIZZLE
   v3d_pixel_format_t px_fmt;
   bool reverse, rb_swap;
   // This checks if the tile-buffer can support the format
   return gfx_lfmt_maybe_translate_pixel_format(lfmt, &px_fmt, &reverse, &rb_swap);
#else
   return gfx_lfmt_maybe_translate_pixel_format(lfmt) != V3D_PIXEL_FORMAT_INVALID;
#endif
}

bool egl_can_texture_from_format(GFX_LFMT_T lfmt)
{
   GFX_LFMT_TMU_TRANSLATION_T tran;

   // This checks if the hardware can texture from it
   return gfx_lfmt_maybe_translate_tmu(&tran, lfmt
#if !V3D_HAS_TMU_R32F_R16_SHAD
      , /*need_depth_type=*/false
#endif
      );
}

bool egl_can_display_format(GFX_LFMT_T lfmt)
{
   EGLint val;

   // This checks if the platform can display it / represent it in a native surface type
   return egl_platform_color_format(lfmt, &val);
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigAttrib(EGLDisplay dpy,
      EGLConfig config, EGLint attribute, EGLint *value)
{
   EGLint error = EGL_BAD_DISPLAY;
   EGL_CONFIG_T *conf;
   bool valid;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   if (config == NULL)
   {
      error = EGL_BAD_CONFIG;
      goto end;
   }

   if (value == NULL)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   conf = (EGL_CONFIG_T *) config;

   if (!egl_config_is_valid(conf))
   {
      error = EGL_BAD_CONFIG;
      goto end;
   }

   EGLint ret = egl_config_get_attrib(conf, attribute, &valid);
   if (!valid)
   {
      error = EGL_BAD_ATTRIBUTE;
      goto end;
   }

   *value = ret;
   error = EGL_SUCCESS;
end:
   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

static void update_sort_params(EGLint *cleaned_attrib_list, SORT_PARAMS_T *sort_params)
{
   while (*cleaned_attrib_list != EGL_NONE)
   {
      EGLint name = *cleaned_attrib_list++;
      EGLint value = *cleaned_attrib_list++;

      switch (name)
      {
      case EGL_RED_SIZE:
         sort_params->use_red    = (value != 0 && value != EGL_DONT_CARE);
         break;
      case EGL_GREEN_SIZE:
         sort_params->use_green  = (value != 0 && value != EGL_DONT_CARE);
         break;
      case EGL_BLUE_SIZE:
         sort_params->use_blue   = (value != 0 && value != EGL_DONT_CARE);
         break;
      case EGL_ALPHA_SIZE:
         sort_params->use_alpha  = (value != 0 && value != EGL_DONT_CARE);
         break;
      default:
         break;
      }
   }
}

/*
 * Add the attribute to the list if it is not already there or
 * change its value if the attribute is in the list
 */
static void add_attrib_value(EGLint *attrib_list, EGLint name, EGLint value)
{
   if (name == EGL_NONE)
      return;

   /* Find attribute and update its value*/
   while (*attrib_list != EGL_NONE)
   {
      if (*attrib_list == name)
      {
         attrib_list++;
         *attrib_list = value;
         /* Return because it might be the last pair before EGL_NONE */
         return;
      }

      attrib_list+=2;
   }

   /* If not found: add attribute and EGL_NONE to the end */
   if (*attrib_list == EGL_NONE)
   {
      *attrib_list++ = name;
      *attrib_list++ = value;
      *attrib_list = EGL_NONE;
   }
}

static void clean_attrib_list(const EGLint *attrib_list, EGLint *cleaned_attrib_list)
{
   while (*attrib_list != EGL_NONE)
   {
      EGLint name = *attrib_list++;
      EGLint value = *attrib_list++;

      add_attrib_value(cleaned_attrib_list, name, value);
   }
}
/*
 * Counts the number of elements (name or value) in the attribute list
 * including EGL_NONE
 */
static unsigned int get_size_of_attrib_list(const EGLint *attrib_list)
{
   unsigned int size = 0;

   while (*attrib_list != EGL_NONE)
   {
      size += 2;
      attrib_list +=2;
   }

   // Number of elements (name or value) + EGL_NONE
   return size+1;
}


static EGLBoolean egl_choose_config(EGLDisplay dpy,
      const EGLint *attrib_list, EGLConfig *configs,
      EGLint config_size, EGLint *num_config)
{
   EGLint error = EGL_BAD_DISPLAY;
   SORT_PARAMS_T sort_params;
   CONFIG_CHOICE_T choices[countof(egl_configs)];
   unsigned j = 0;
   EGLint *cleaned_attrib_list = NULL;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   if (!num_config)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   if (!check_attribs(attrib_list))
   {
      error = EGL_BAD_ATTRIBUTE;
      goto end;
   }

   memset(&sort_params, 0, sizeof sort_params);

   if (attrib_list)
   {
      unsigned int num_attrib = get_size_of_attrib_list(attrib_list);
      cleaned_attrib_list = malloc(num_attrib * sizeof(EGLint));
      if (cleaned_attrib_list == NULL)
      {
         error = EGL_BAD_ALLOC;
         goto end;
      }
      cleaned_attrib_list[0] = EGL_NONE;
      // Create an attribute list without duplicate
      clean_attrib_list(attrib_list, cleaned_attrib_list);

      update_sort_params(cleaned_attrib_list, &sort_params);
   }

   for (unsigned int i = 0; i < countof(egl_configs); i++)
   {
      const EGL_CONFIG_T *candidate = egl_configs + i;

      bool advertiseMe =  egl_can_render_format(candidate->color_api_fmt) &&
                          (egl_can_display_format(candidate->color_api_fmt) ||
                          egl_can_texture_from_format(candidate->color_api_fmt)) &&
                          !candidate->invalid;

      if (advertiseMe && config_matches(candidate, cleaned_attrib_list))
      {
         choices[j].descriptor = candidate;
         choices[j].sort_params = &sort_params;
         ++j;
      }
   }

   if (j > 0)
      qsort(choices, j, sizeof (CONFIG_CHOICE_T), compar);

   *num_config = j;
   if (configs != NULL)
   {
      /* Note config_size may be negative, so gfx_smax(,0) last! */
      *num_config = gfx_smax(gfx_smin(*num_config, config_size), 0);

      for (int i = 0; i < *num_config; i++)
         configs[i] = (EGLConfig *) choices[i].descriptor;
   }

   error = EGL_SUCCESS;
end:
   free(cleaned_attrib_list);
   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

EGLAPI EGLBoolean EGLAPIENTRY eglChooseConfig(EGLDisplay dpy,
   const EGLint *attrib_list, EGLConfig *configs,
   EGLint config_size, EGLint *num_config)
{
   return egl_choose_config(dpy, attrib_list, configs, config_size, num_config);
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigs(EGLDisplay dpy,
      EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
   /*
    * Technically I don't think you need to sort them for eglGetConfigs, but
    * since if you don't, the order is unspecified, it doesn't do any harm.
    */
   return egl_choose_config(dpy, NULL, configs, config_size, num_config);
}
