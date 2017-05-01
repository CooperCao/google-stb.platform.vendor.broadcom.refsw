/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/include/EGL/eglext.h"

#include "interface/khronos/egl/egl_client_config.h"

//#define BGR_FB

typedef uint64_t FEATURES_T;

#define FEATURES_PACK(r, g, b, a, d, s, m, mask, lockable, framebuffer_target_android, recordable_android) \
   ((FEATURES_T)((((uint64_t)(r)             ) << 36) | \
                 (((uint64_t)(g)             ) << 32) | \
                 (((uint64_t)(b)             ) << 28) | \
                 ((a                         ) << 24) | \
                 ((d                         ) << 16) | \
                 ((s                         ) << 12) | \
                 ((m                         ) <<  4) | \
                 (((mask) >> 3               ) <<  3) | \
                 ((lockable                  ) <<  2) | \
                 ((framebuffer_target_android) <<  1) | \
                  (recordable_android        ) <<  0))

typedef struct {
   FEATURES_T features;
   KHRN_IMAGE_FORMAT_T color, depth, multisample, mask;
} FEATURES_AND_FORMATS_T;

/*
   formats

   For 0 <= id < EGL_MAX_CONFIGS:
      formats[id].features is valid
*/

static const FEATURES_AND_FORMATS_T formats_TFORMAT[] = {
/*                R  G  B  A   D  S  M  MASK
                                        |  LOCKABLE
                                        |  |  FRAMEBUFFER_TARGET_ANDROID
                                        |  |  |  RECORDABLE
                                        |  |  |  |   COLOR        DEPTH                 MULTISAMPLE           MASK */
   {FEATURES_PACK(8, 8, 8, 8, 24, 8, 0, 0, 0, 0, 0), ABGR_8888_TF,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0, 24, 8, 0, 0, 0, 0, 0), XBGR_8888_TF,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8, 24, 0, 0, 0, 0, 0, 0), ABGR_8888_TF,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0, 24, 0, 0, 0, 0, 0, 0), XBGR_8888_TF,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 8, 0, 0, 0, 0, 0), ABGR_8888_TF,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0,  0, 8, 0, 0, 0, 0, 0), XBGR_8888_TF,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 0, 0, 0, 0, 0, 0), ABGR_8888_TF,IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0,  0, 0, 0, 0, 0, 0, 0), XBGR_8888_TF,IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},

   {FEATURES_PACK(8, 8, 8, 8, 24, 8, 1, 0, 0, 0, 0), ABGR_8888_TF,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0, 24, 8, 1, 0, 0, 0, 0), XBGR_8888_TF,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8, 24, 0, 1, 0, 0, 0, 0), ABGR_8888_TF,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0, 24, 0, 1, 0, 0, 0, 0), XBGR_8888_TF,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 8, 1, 0, 0, 0, 0), ABGR_8888_TF,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0,  0, 8, 1, 0, 0, 0, 0), XBGR_8888_TF,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 0, 1, 0, 0, 0, 0), ABGR_8888_TF,IMAGE_FORMAT_INVALID, COL_32_TLBD,          IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0,  0, 0, 1, 0, 0, 0, 0), XBGR_8888_TF,IMAGE_FORMAT_INVALID, COL_32_TLBD,          IMAGE_FORMAT_INVALID},

   {FEATURES_PACK(5, 6, 5, 0, 24, 8, 0, 0, 0, 0, 0), RGB_565_TF,  DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(5, 6, 5, 0, 24, 0, 0, 0, 0, 0, 0), RGB_565_TF,  DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(5, 6, 5, 0,  0, 8, 0, 0, 0, 0, 0), RGB_565_TF,  DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(5, 6, 5, 0,  0, 0, 0, 0, 0, 0, 0), RGB_565_TF,  IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},

   {FEATURES_PACK(5, 6, 5, 0, 24, 8, 1, 0, 0, 0, 0), RGB_565_TF,  DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(5, 6, 5, 0, 24, 0, 1, 0, 0, 0, 0), RGB_565_TF,  DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(5, 6, 5, 0,  0, 8, 1, 0, 0, 0, 0), RGB_565_TF,  DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(5, 6, 5, 0,  0, 0, 1, 0, 0, 0, 0), RGB_565_TF,  IMAGE_FORMAT_INVALID, COL_32_TLBD,          IMAGE_FORMAT_INVALID},

   {FEATURES_PACK(5, 6, 5, 0, 16, 0, 0, 0, 0, 0, 0), RGB_565_TF,  DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},

   {FEATURES_PACK(8, 8, 8, 8,  0, 0, 0, 8, 0, 0, 0), ABGR_8888_TF,IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, A_8_RSO},
   {FEATURES_PACK(8, 8, 8, 0,  0, 0, 0, 8, 0, 0, 0), XBGR_8888_TF,IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, A_8_RSO},
   {FEATURES_PACK(5, 6, 5, 0,  0, 0, 0, 8, 0, 0, 0), RGB_565_TF,  IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, A_8_RSO},

#if EGL_ANDROID_framebuffer_target
   {FEATURES_PACK(8, 8, 8, 8,  0, 0, 0, 0, 0, 1, 0), ABGR_8888_TF,IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
#endif

#if EGL_ANDROID_recordable
   {FEATURES_PACK(8, 8, 8, 8, 24, 8, 0, 0, 0, 0, 1), ABGR_8888_TF,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8, 24, 0, 0, 0, 0, 0, 1), ABGR_8888_TF,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 8, 0, 0, 0, 0, 1), ABGR_8888_TF,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 0, 0, 0, 0, 0, 1), ABGR_8888_TF,IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},

   {FEATURES_PACK(8, 8, 8, 8, 24, 8, 1, 0, 0, 0, 1), ABGR_8888_TF,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8, 24, 0, 1, 0, 0, 0, 1), ABGR_8888_TF,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 8, 1, 0, 0, 0, 1), ABGR_8888_TF,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 0, 1, 0, 0, 0, 1), ABGR_8888_TF,IMAGE_FORMAT_INVALID, COL_32_TLBD,          IMAGE_FORMAT_INVALID},
#endif
};


static const FEATURES_AND_FORMATS_T formats_RSO[] = {
/*                R  G  B  A   D  S  M  MASK
                                        |  LOCKABLE
                                        |  |  FRAMEBUFFER_TARGET_ANDROID
                                        |  |  |  RECORDABLE
                                        |  |  |  |   COLOR         DEPTH                 MULTISAMPLE           MASK */
   {FEATURES_PACK(8, 8, 8, 8, 24, 8, 0, 0, 0, 0, 0), ABGR_8888_RSO,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0, 24, 8, 0, 0, 0, 0, 0), XBGR_8888_RSO,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8, 24, 0, 0, 0, 0, 0, 0), ABGR_8888_RSO,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0, 24, 0, 0, 0, 0, 0, 0), XBGR_8888_RSO,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 8, 0, 0, 0, 0, 0), ABGR_8888_RSO,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0,  0, 8, 0, 0, 0, 0, 0), XBGR_8888_RSO,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 0, 0, 0, 0, 0, 0), ABGR_8888_RSO,IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0,  0, 0, 0, 0, 0, 0, 0), XBGR_8888_RSO,IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},

   {FEATURES_PACK(8, 8, 8, 8, 24, 8, 1, 0, 0, 0, 0), ABGR_8888_RSO,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0, 24, 8, 1, 0, 0, 0, 0), XBGR_8888_RSO,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8, 24, 0, 1, 0, 0, 0, 0), ABGR_8888_RSO,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0, 24, 0, 1, 0, 0, 0, 0), XBGR_8888_RSO,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 8, 1, 0, 0, 0, 0), ABGR_8888_RSO,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0,  0, 8, 1, 0, 0, 0, 0), XBGR_8888_RSO,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 0, 1, 0, 0, 0, 0), ABGR_8888_RSO,IMAGE_FORMAT_INVALID, COL_32_TLBD,          IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 0,  0, 0, 1, 0, 0, 0, 0), XBGR_8888_RSO,IMAGE_FORMAT_INVALID, COL_32_TLBD,          IMAGE_FORMAT_INVALID},

   {FEATURES_PACK(5, 6, 5, 0, 24, 8, 0, 0, 0, 0, 0), RGB_565_RSO,  DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(5, 6, 5, 0, 24, 0, 0, 0, 0, 0, 0), RGB_565_RSO,  DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(5, 6, 5, 0,  0, 8, 0, 0, 0, 0, 0), RGB_565_RSO,  DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(5, 6, 5, 0,  0, 0, 0, 0, 0, 0, 0), RGB_565_RSO,  IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},

   {FEATURES_PACK(5, 6, 5, 0, 24, 8, 1, 0, 0, 0, 0), RGB_565_RSO,  DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(5, 6, 5, 0, 24, 0, 1, 0, 0, 0, 0), RGB_565_RSO,  DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(5, 6, 5, 0,  0, 8, 1, 0, 0, 0, 0), RGB_565_RSO,  DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD,    IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(5, 6, 5, 0,  0, 0, 1, 0, 0, 0, 0), RGB_565_RSO,  IMAGE_FORMAT_INVALID, COL_32_TLBD,          IMAGE_FORMAT_INVALID},

   {FEATURES_PACK(5, 6, 5, 0, 16, 0, 0, 0, 0, 0, 0), RGB_565_RSO,  DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},

   {FEATURES_PACK(8, 8, 8, 8,  0, 0, 0, 8, 0, 0, 0), ABGR_8888_RSO,IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, A_8_RSO},
   {FEATURES_PACK(8, 8, 8, 0,  0, 0, 0, 8, 0, 0, 0), XBGR_8888_RSO,IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, A_8_RSO},
   {FEATURES_PACK(5, 6, 5, 0,  0, 0, 0, 8, 0, 0, 0), RGB_565_RSO,  IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, A_8_RSO},

#if EGL_ANDROID_framebuffer_target
   {FEATURES_PACK(8, 8, 8, 8,  0, 0, 0, 0, 0, 1, 0), ABGR_8888_RSO,IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
#endif

#if EGL_ANDROID_recordable
   {FEATURES_PACK(8, 8, 8, 8, 24, 8, 0, 0, 0, 0, 1), ABGR_8888_RSO,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8, 24, 0, 0, 0, 0, 0, 1), ABGR_8888_RSO,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 8, 0, 0, 0, 0, 1), ABGR_8888_RSO,DEPTH_32_TLBD,        IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 0, 0, 0, 0, 0, 1), ABGR_8888_RSO,IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID, IMAGE_FORMAT_INVALID},

   {FEATURES_PACK(8, 8, 8, 8, 24, 8, 1, 0, 0, 0, 1), ABGR_8888_RSO,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8, 24, 0, 1, 0, 0, 0, 1), ABGR_8888_RSO,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 8, 1, 0, 0, 0, 1), ABGR_8888_RSO,DEPTH_32_TLBD /*?*/,  DEPTH_COL_64_TLBD, IMAGE_FORMAT_INVALID},
   {FEATURES_PACK(8, 8, 8, 8,  0, 0, 1, 0, 0, 0, 1), ABGR_8888_RSO,IMAGE_FORMAT_INVALID, COL_32_TLBD, IMAGE_FORMAT_INVALID},
#endif
};

/* used anywhere we need the number of configs */
int EGL_MAX_CONFIGS = 0;

/* this table needs filling in from either the tables above, depending on what the platform supports */
static FEATURES_AND_FORMATS_T formats[sizeof(formats_RSO) / sizeof(FEATURES_AND_FORMATS_T)];
vcos_static_assert(sizeof(formats_RSO) == sizeof(formats_TFORMAT));
vcos_static_assert(sizeof(formats) == sizeof(formats_RSO));

void egl_config_install_configs(int type)
{
   /* 0 = RSO, 1 = TFORMAT */
   EGL_MAX_CONFIGS = sizeof(formats) / sizeof(FEATURES_AND_FORMATS_T);
   memcpy(formats, (type == 0) ? formats_RSO : formats_TFORMAT, sizeof(formats));
}

static bool bindable_rgb(FEATURES_T features);
static bool bindable_rgba(FEATURES_T features);

#define FEATURES_UNPACK_RED(c)                           ((EGLint)((c) >> 36 & 0xf))
#define FEATURES_UNPACK_GREEN(c)                         ((EGLint)((c) >> 32 & 0xf))
#define FEATURES_UNPACK_BLUE(c)                          ((EGLint)((c) >> 28 & 0xf))
#define FEATURES_UNPACK_ALPHA(c)                         ((EGLint)((c) >> 24 & 0xf))
#define FEATURES_UNPACK_DEPTH(c)                         ((EGLint)((c) >> 16 & 0xff))
#define FEATURES_UNPACK_STENCIL(c)                       ((EGLint)((c) >> 12 & 0xf))
#define FEATURES_UNPACK_MULTI(c)                         ((EGLint)((c) >> 4 & 0x1))
#define FEATURES_UNPACK_MASK(c)                          ((EGLint)(((c) >> 3 & 0x1) << 3))
#define FEATURES_UNPACK_LOCKABLE(c)                      ((EGLint)((c) >> 2 & 0x1))
#define FEATURES_UNPACK_ANDROID_FRAMEBUFFER_TARGET(c)    ((EGLint)((c) >> 1 & 0x1))
#define FEATURES_UNPACK_ANDROID_RECORDABLE(c)            ((EGLint)((c)      & 0x1))
#define FEATURES_UNPACK_COLOR(c)                         (FEATURES_UNPACK_RED(c)+FEATURES_UNPACK_GREEN(c)+FEATURES_UNPACK_BLUE(c)+FEATURES_UNPACK_ALPHA(c))

bool egl_config_check_attribs(const EGLint *attrib_list)
{
   if (!attrib_list)
      return true;

   while (*attrib_list != EGL_NONE) {
      EGLint name = *attrib_list++;
      EGLint value = *attrib_list++;

      switch (name) {
      case EGL_BUFFER_SIZE:
      case EGL_RED_SIZE:
      case EGL_GREEN_SIZE:
      case EGL_BLUE_SIZE:
      case EGL_LUMINANCE_SIZE:
      case EGL_ALPHA_SIZE:
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
         if (value != EGL_DONT_CARE && (value & ~(EGL_OPENGL_BIT | EGL_OPENGL_ES_BIT | EGL_OPENGL_ES2_BIT | EGL_OPENVG_BIT)))
            return false;
         break;
      case EGL_DEPTH_SIZE:
         if (value != EGL_DONT_CARE && value < 0) return false;
         break;
      case EGL_LEVEL:
         break;
      case EGL_MATCH_NATIVE_PIXMAP:
         /* 1.4 Spec is poor here - says that value has to be a valid handle, but also says that any attribute
         * value (other than EGL_LEVEL) can be EGL_DONT_CARE. It also says that the default value is EGL_NONE,
         * but that doesn't really make sense - sensible to assume that the default is EGL_DONT_CARE, and don't
         * support EGL_NONE as an explicit parameter. (Could theoretically collide with a real handle...)
         */
         if (value != EGL_DONT_CARE) {
            KHRN_IMAGE_WRAP_T image;
            if (!platform_get_pixmap_info((EGLNativePixmapType)(intptr_t)value, &image))
               return false;
            khrn_platform_release_pixmap_info((EGLNativePixmapType)(intptr_t)value, &image);
         }
         break;
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
         if (value != EGL_DONT_CARE && (value & ~(EGL_OPENGL_BIT | EGL_OPENGL_ES_BIT | EGL_OPENGL_ES2_BIT | EGL_OPENVG_BIT)))
            return false;
         break;
      case EGL_SAMPLE_BUFFERS:
      case EGL_SAMPLES:
      case EGL_STENCIL_SIZE:
         if (value != EGL_DONT_CARE && value < 0) return false;
         break;
      case EGL_SURFACE_TYPE:
      {
         int valid_bits = EGL_WINDOW_BIT | EGL_PIXMAP_BIT | EGL_PBUFFER_BIT |
            EGL_MULTISAMPLE_RESOLVE_BOX_BIT | EGL_SWAP_BEHAVIOR_PRESERVED_BIT |
            EGL_VG_COLORSPACE_LINEAR_BIT | EGL_VG_ALPHA_FORMAT_PRE_BIT;
#if EGL_KHR_lock_surface
         valid_bits |= EGL_LOCK_SURFACE_BIT_KHR | EGL_OPTIMAL_FORMAT_BIT_KHR;
#endif
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
#if EGL_KHR_lock_surface
      case EGL_MATCH_FORMAT_KHR:
         switch (value) {
         case EGL_DONT_CARE:
         case EGL_NONE:
         case EGL_FORMAT_RGB_565_EXACT_KHR:
         case EGL_FORMAT_RGB_565_KHR:
         case EGL_FORMAT_RGBA_8888_EXACT_KHR:
         case EGL_FORMAT_RGBA_8888_KHR:
            break;
         default:
            return false;
         }
         break;
#endif
#if EGL_ANDROID_recordable
      case EGL_RECORDABLE_ANDROID:
         switch (value) {
         case EGL_DONT_CARE:
         case EGL_TRUE:
         case EGL_FALSE:
            break;
         default:
            return false;
         }
         break;
#endif
#if EGL_ANDROID_framebuffer_target
      case EGL_FRAMEBUFFER_TARGET_ANDROID:
         switch (value) {
         case EGL_DONT_CARE:
         case EGL_TRUE:
         case EGL_FALSE:
            break;
         default:
            return false;
         }
         break;
#endif
      default:
         return false;
      }
   }

   return true;
}

static bool less_than(int id0, int id1, bool use_red, bool use_green, bool use_blue, bool use_alpha)
{
   FEATURES_T features0 = formats[id0].features;
   FEATURES_T features1 = formats[id1].features;

   EGLint all0 = FEATURES_UNPACK_COLOR(features0);
   EGLint all1 = FEATURES_UNPACK_COLOR(features1);

   EGLint multi0 = FEATURES_UNPACK_MULTI(features0);
   EGLint multi1 = FEATURES_UNPACK_MULTI(features1);

   EGLint depth0 = FEATURES_UNPACK_DEPTH(features0);
   EGLint depth1 = FEATURES_UNPACK_DEPTH(features1);

   EGLint stencil0 = FEATURES_UNPACK_STENCIL(features0);
   EGLint stencil1 = FEATURES_UNPACK_STENCIL(features1);

   EGLint mask0 = FEATURES_UNPACK_MASK(features0);
   EGLint mask1 = FEATURES_UNPACK_MASK(features1);

   int used0 = 0;
   int used1 = 0;

   if (use_red) {
      used0 += FEATURES_UNPACK_RED(features0);
      used1 += FEATURES_UNPACK_RED(features1);
   }
   if (use_green) {
      used0 += FEATURES_UNPACK_GREEN(features0);
      used1 += FEATURES_UNPACK_GREEN(features1);
   }
   if (use_blue) {
      used0 += FEATURES_UNPACK_BLUE(features0);
      used1 += FEATURES_UNPACK_BLUE(features1);
   }
   if (use_alpha) {
      used0 += FEATURES_UNPACK_ALPHA(features0);
      used1 += FEATURES_UNPACK_ALPHA(features1);
   }

   return used0 > used1 || (used0 == used1 &&
      (all0 < all1 || (all0 == all1 &&
      (multi0 < multi1 || (multi0 == multi1 &&
      (depth0 < depth1 || (depth0 == depth1 &&
      (stencil0 < stencil1 || (stencil0 == stencil1 &&
      (mask0 < mask1))))))))));
}

void egl_config_sort(int *ids, bool use_red, bool use_green, bool use_blue, bool use_alpha)
{
   int i, j;

   for (i = 1; i < EGL_MAX_CONFIGS; i++)
      for (j = 0; j < EGL_MAX_CONFIGS - i; j++)
         if (less_than(ids[j + 1], ids[j], use_red, use_green, use_blue, use_alpha)) {
            int temp = ids[j];
            ids[j] = ids[j + 1];
            ids[j + 1] = temp;
         }
}

bool egl_config_get_attrib(int id, EGLint attrib, EGLint *value)
{
   FEATURES_T features = formats[id].features;

   switch (attrib) {
   case EGL_BUFFER_SIZE:
      *value = FEATURES_UNPACK_COLOR(features);
      return true;
   case EGL_RED_SIZE:
      *value = FEATURES_UNPACK_RED(features);
      return true;
   case EGL_GREEN_SIZE:
      *value = FEATURES_UNPACK_GREEN(features);
      return true;
   case EGL_BLUE_SIZE:
      *value = FEATURES_UNPACK_BLUE(features);
      return true;
   case EGL_LUMINANCE_SIZE:
      *value = 0;
      return true;
   case EGL_ALPHA_SIZE:
      *value = FEATURES_UNPACK_ALPHA(features);
      return true;
   case EGL_ALPHA_MASK_SIZE:
      *value = FEATURES_UNPACK_MASK(features);
      return true;
   case EGL_BIND_TO_TEXTURE_RGB:
      *value = bindable_rgb(features);
      return true;
   case EGL_BIND_TO_TEXTURE_RGBA:
      *value = bindable_rgba(features);
      return true;
   case EGL_COLOR_BUFFER_TYPE:
      *value = EGL_RGB_BUFFER;
      return true;
   case EGL_CONFIG_CAVEAT:
      *value = EGL_NONE;
      return true;
   case EGL_CONFIG_ID:
      *value = (EGLint)(uintptr_t)egl_config_from_id(id);
      return true;
   case EGL_CONFORMANT:
      *value = egl_config_get_api_conformance(id);
      return true;
   case EGL_DEPTH_SIZE:
      *value = FEATURES_UNPACK_DEPTH(features);
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
      *value = khrn_platform_get_color_format(egl_config_get_color_format(id));
      return true;
   case EGL_NATIVE_VISUAL_TYPE:
      *value = EGL_NONE;
      return true;
   case EGL_RENDERABLE_TYPE:
      *value = egl_config_get_api_support(id);
      return true;
   case EGL_SAMPLE_BUFFERS:
      *value = FEATURES_UNPACK_MULTI(features);
      return true;
   case EGL_SAMPLES:
      *value = FEATURES_UNPACK_MULTI(features) * 4;
      return true;
   case EGL_STENCIL_SIZE:
      *value = FEATURES_UNPACK_STENCIL(features);
      return true;
   case EGL_SURFACE_TYPE:
      *value = (EGLint)(EGL_PBUFFER_BIT |
         EGL_PIXMAP_BIT |
         EGL_WINDOW_BIT |
         EGL_VG_COLORSPACE_LINEAR_BIT |
         EGL_VG_ALPHA_FORMAT_PRE_BIT |
         EGL_MULTISAMPLE_RESOLVE_BOX_BIT
         );
#if EGL_KHR_lock_surface
      if (egl_config_is_lockable(id))
      {
         *value |= EGL_LOCK_SURFACE_BIT_KHR;
         if (egl_config_get_mapped_format(id) == egl_config_get_color_format(id))
            *value |= EGL_OPTIMAL_FORMAT_BIT_KHR;      /* Considered optimal if no format conversion needs doing. Currently all lockable surfaces are optimal */
      }
#endif
      return true;
   case EGL_TRANSPARENT_TYPE:
      *value = EGL_NONE;
      return true;
   case EGL_TRANSPARENT_RED_VALUE:
   case EGL_TRANSPARENT_GREEN_VALUE:
   case EGL_TRANSPARENT_BLUE_VALUE:
      *value = 0;
      return true;
#if EGL_KHR_lock_surface
   case EGL_MATCH_FORMAT_KHR:
      if (!egl_config_is_lockable(id))
         *value = EGL_NONE;
      else {
         switch (egl_config_get_mapped_format(id))
         {
         case RGB_565_RSO:
            *value = EGL_FORMAT_RGB_565_EXACT_KHR;
            break;
         case ARGB_8888_RSO:
            *value = EGL_FORMAT_RGBA_8888_EXACT_KHR;
            break;
         default:
            UNREACHABLE();
         }
      }
      return true;
#endif
#if EGL_ANDROID_recordable
   case EGL_RECORDABLE_ANDROID:
      if (egl_config_is_recordable(id))
         *value = EGL_TRUE;
      else
         *value = EGL_FALSE;
      return true;
#endif
#if EGL_ANDROID_framebuffer_target
   case EGL_FRAMEBUFFER_TARGET_ANDROID:
      if (egl_config_is_framebuffer_target(id))
         *value = EGL_TRUE;
      else
         *value = EGL_FALSE;
      return true;
#endif
   default:
      return false;
   }
}

bool egl_config_filter(int id, const EGLint *attrib_list)
{
   if (!attrib_list)
      return true;

   while (*attrib_list != EGL_NONE) {
      EGLint name = *attrib_list++;
      EGLint value = *attrib_list++;
      EGLint actual_value;

      if (!egl_config_get_attrib(id, name, &actual_value))
      {
         UNREACHABLE();
         return false;
      }

      switch (name) {
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
#if EGL_ANDROID_recordable
      case EGL_RECORDABLE_ANDROID:
#endif
#if EGL_ANDROID_framebuffer_target
      case EGL_FRAMEBUFFER_TARGET_ANDROID:
#endif
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
         if (value != EGL_DONT_CARE) { /* see comments in egl_config_check_attribs */
            EGLNativePixmapType pixmap = (EGLNativePixmapType)(intptr_t)value;
            KHRN_IMAGE_WRAP_T image;
            if (!platform_get_pixmap_info(pixmap, &image)) {
               /*
               Not actually unreachable in theory!
               We should have detected this in egl_config_check_attribs
               It's possible that the validity of pixmap has changed since then however...
               */
               UNREACHABLE();
               return false;
            }
            if (!egl_config_match_pixmap_info(id, &image) ||
               !platform_match_pixmap_api_support(pixmap, egl_config_get_api_support(id)))
            {
               khrn_platform_release_pixmap_info(pixmap, &image);
               return false;
            }

            khrn_platform_release_pixmap_info(pixmap, &image);
         }
         break;
#if EGL_KHR_lock_surface
      case EGL_MATCH_FORMAT_KHR:
         if (!(value == EGL_DONT_CARE || value == actual_value
            || (value == EGL_FORMAT_RGB_565_KHR && actual_value == EGL_FORMAT_RGB_565_EXACT_KHR)
            || (value == EGL_FORMAT_RGBA_8888_KHR && actual_value == EGL_FORMAT_RGBA_8888_EXACT_KHR)))
         {
            return false;
         }
         break;
#endif

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
         UNREACHABLE();
         break;
      }
   }

   return true;
}

/*
   KHRN_IMAGE_FORMAT_T egl_config_get_color_format(int id)

   Implementation notes:

   We may return an image format which cannot be rendered to.

   Preconditions:

   0 <= id < EGL_MAX_CONFIGS

   Postconditions:

   Return value is a hardware framebuffer-supported uncompressed color KHRN_IMAGE_FORMAT_T
*/

KHRN_IMAGE_FORMAT_T egl_config_get_color_format(int id)
{
   vcos_assert(id >= 0 && id < EGL_MAX_CONFIGS);

   return formats[id].color;
}

/*
   KHRN_IMAGE_FORMAT_T egl_config_get_depth_format(int id)

   Preconditions:

   0 <= id < EGL_MAX_CONFIGS

   Postconditions:

   Return value is a hardware framebuffer-supported depth KHRN_IMAGE_FORMAT_T or IMAGE_FORMAT_INVALID
*/

KHRN_IMAGE_FORMAT_T egl_config_get_depth_format(int id)
{
   vcos_assert(id >= 0 && id < EGL_MAX_CONFIGS);

   return formats[id].depth;
}

/*
   KHRN_IMAGE_FORMAT_T egl_config_get_mask_format(int id)

   Preconditions:

   0 <= id < EGL_MAX_CONFIGS

   Postconditions:

   Return value is a hardware framebuffer-supported mask KHRN_IMAGE_FORMAT_T or IMAGE_FORMAT_INVALID
*/

KHRN_IMAGE_FORMAT_T egl_config_get_mask_format(int id)
{
   vcos_assert(id >= 0 && id < EGL_MAX_CONFIGS);

   return formats[id].mask;
}

/*
   KHRN_IMAGE_FORMAT_T egl_config_get_multisample_format(int id)

   Preconditions:

   0 <= id < EGL_MAX_CONFIGS

   Postconditions:

   Return value is a hardware framebuffer-supported multisample color format or IMAGE_FORMAT_INVALID
*/

KHRN_IMAGE_FORMAT_T egl_config_get_multisample_format(int id)
{
   vcos_assert(id >= 0 && id < EGL_MAX_CONFIGS);

   return formats[id].multisample;
}

/*
   bool egl_config_get_multisample(int id)

   Preconditions:

   0 <= id < EGL_MAX_CONFIGS

   Postconditions:

   -
*/

bool egl_config_get_multisample(int id)
{
   vcos_assert(id >= 0 && id < EGL_MAX_CONFIGS);

   return FEATURES_UNPACK_MULTI(formats[id].features);
}

/*
   bool bindable_rgb(FEATURES_T features)
   bool bindable_rgba(FEATURES_T features)

   Preconditions:

   features is a valid FEATURES_T

   Postconditions:

   -
*/

static bool bindable_rgb(FEATURES_T features)
{
   return !FEATURES_UNPACK_MULTI(features) && !FEATURES_UNPACK_ALPHA(features);
}

static bool bindable_rgba(FEATURES_T features)
{
   return !FEATURES_UNPACK_MULTI(features);
}

bool egl_config_bindable(int id, EGLenum format)
{
   vcos_assert(id >= 0 && id < EGL_MAX_CONFIGS);
   switch (format) {
   case EGL_NO_TEXTURE:
      return true;
   case EGL_TEXTURE_RGB:
      return bindable_rgb(formats[id].features);
   case EGL_TEXTURE_RGBA:
      return bindable_rgba(formats[id].features);
   default:
      UNREACHABLE();
      return false;
   }
}

/*
   bool egl_config_match_pixmap_info(int id, KHRN_IMAGE_WRAP_T *image)

   TODO: decide how tolerant we should be when matching to native pixmaps.
   At present they match if the red, green, blue and alpha channels
   all have the same bit depths.

   Preconditions:

   0 <= id < EGL_MAX_CONFIGS
   image is a pointer to a valid KHRN_IMAGE_WRAP_T, possibly with null data pointer
   match is a bitmask which is a subset of PLATFORM_PIXMAP_MATCH_NONRENDERABLE|PLATFORM_PIXMAP_MATCH_RENDERABLE

   Postconditions:

   -
*/

bool egl_config_match_pixmap_info(int id, KHRN_IMAGE_WRAP_T *image)
{
   FEATURES_T features = formats[id].features;
   KHRN_IMAGE_FORMAT_T format = image->format;

   vcos_assert(id >= 0 && id < EGL_MAX_CONFIGS);

   return
      khrn_image_get_red_size(format)   == (uint32_t)FEATURES_UNPACK_RED(features) &&
      khrn_image_get_green_size(format) == (uint32_t)FEATURES_UNPACK_GREEN(features) &&
      khrn_image_get_blue_size(format)  == (uint32_t)FEATURES_UNPACK_BLUE(features) &&
      khrn_image_get_alpha_size(format) == (uint32_t)FEATURES_UNPACK_ALPHA(features);
}

/*
   uint32_t egl_config_get_api_support(int id)

   Preconditions:

   0 <= id < EGL_MAX_CONFIGS

   Postconditions:

   Result is a bitmap which is a subset of (EGL_OPENGL_ES_BIT | EGL_OPENVG_BIT | EGL_OPENGL_ES2_BIT)
*/

uint32_t egl_config_get_api_support(int id)
{
   /* no configs are api-specific (ie if you can use a config with gl, you can
    * use it with vg too, and vice-versa). however, some configs have color
    * buffer formats that are incompatible with the hardware, and so can't be
    * used with any api. such configs may still be useful eg with the surface
    * locking extension... */

#if EGL_KHR_lock_surface
   /* to reduce confusion, just say no for all lockable configs. this #if can be
    * safely commented out -- the color buffer format check below will catch
    * lockable configs we actually can't use */
   if (egl_config_is_lockable(id)) {
      return 0;
   }
#endif

   switch (egl_config_get_color_format(id)) {
   case ABGR_8888_RSO: case ABGR_8888_TF: case ABGR_8888_LT:
   case XBGR_8888_RSO: case XBGR_8888_TF: case XBGR_8888_LT:
   case ARGB_8888_RSO: case ARGB_8888_TF: case ARGB_8888_LT:
   case XRGB_8888_RSO: case XRGB_8888_TF: case XRGB_8888_LT:
   case RGB_565_RSO: case RGB_565_TF: case RGB_565_LT:
#ifndef NO_OPENVG
      return (uint32_t)(EGL_OPENGL_ES_BIT | EGL_OPENVG_BIT | EGL_OPENGL_ES2_BIT);
#else
      return (uint32_t)(EGL_OPENGL_ES_BIT | EGL_OPENGL_ES2_BIT);
#endif

   default:
      return 0;
   }
}

/*
   uint32_t egl_config_get_api_conformance(int id)

   Preconditions:

   0 <= id < EGL_MAX_CONFIGS

   Postconditions:

   Result is a bitmap which is a subset of (EGL_OPENGL_ES_BIT | EGL_OPENVG_BIT | EGL_OPENGL_ES2_BIT)
*/

uint32_t egl_config_get_api_conformance(int id)
{
   /* vg doesn't support multisampled surfaces properly */
   return egl_config_get_api_support(id) & ~(FEATURES_UNPACK_MULTI(formats[id].features) ? EGL_OPENVG_BIT : 0);
}

bool egl_config_bpps_match(int id0, int id1) /* bpps of all buffers match */
{
   FEATURES_T config0 = formats[id0].features;
   FEATURES_T config1 = formats[id1].features;

   return
      FEATURES_UNPACK_RED(config0)     == FEATURES_UNPACK_RED(config1) &&
      FEATURES_UNPACK_GREEN(config0)   == FEATURES_UNPACK_GREEN(config1) &&
      FEATURES_UNPACK_BLUE(config0)    == FEATURES_UNPACK_BLUE(config1) &&
      FEATURES_UNPACK_ALPHA(config0)   == FEATURES_UNPACK_ALPHA(config1) &&
      FEATURES_UNPACK_DEPTH(config0)   == FEATURES_UNPACK_DEPTH(config1) &&
      FEATURES_UNPACK_STENCIL(config0) == FEATURES_UNPACK_STENCIL(config1) &&
      FEATURES_UNPACK_MASK(config0)    == FEATURES_UNPACK_MASK(config1);
}

#if EGL_KHR_lock_surface

/*
   KHRN_IMAGE_FORMAT_T egl_config_get_mapped_format(int id)

   Returns the format of the mapped buffer when an EGL surface is locked.

   Preconditions:

   0 <= id < EGL_MAX_CONFIGS
   egl_config_is_lockable(id)

   Postconditions:

   Return value is RGB_565_RSO or ARGB_8888_RSO
*/

KHRN_IMAGE_FORMAT_T egl_config_get_mapped_format(int id)
{
   KHRN_IMAGE_FORMAT_T result;

   vcos_assert(id >= 0 && id < EGL_MAX_CONFIGS);
   vcos_assert(FEATURES_UNPACK_LOCKABLE(formats[id].features));

   /* If any t-format images were lockable, we would convert to raster format here */
   result = egl_config_get_color_format(id);
   vcos_assert(khrn_image_is_rso(result));
   return result;
}

/*
   bool egl_config_is_lockable(int id)

   Preconditions:

   0 <= id < EGL_MAX_CONFIGS

   Postconditions:

   -
*/

bool egl_config_is_lockable(int id)
{
   vcos_assert(id >= 0 && id < EGL_MAX_CONFIGS);
   return FEATURES_UNPACK_LOCKABLE(formats[id].features);
}
#endif /* EGL_KHR_lock_surface */

#if EGL_ANDROID_framebuffer_target
bool egl_config_is_framebuffer_target(int id)
{
   vcos_assert(id >= 0 && id < EGL_MAX_CONFIGS);
   return FEATURES_UNPACK_ANDROID_FRAMEBUFFER_TARGET(formats[id].features);
}
#endif /* EGL_ANDROID_framebuffer_target */

#if EGL_ANDROID_recordable
bool egl_config_is_recordable(int id)
{
   vcos_assert(id >= 0 && id < EGL_MAX_CONFIGS);
   return FEATURES_UNPACK_ANDROID_RECORDABLE(formats[id].features);
}
#endif /* EGL_ANDROID_recordable */
