/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_TMU_BLIT_H
#define GLXX_TMU_BLIT_H

#include "gl_public_api.h"
#include "../common/khrn_image.h"

typedef GLint point_t[2];

struct box
{
   point_t     inf;
   point_t     sup;
};

struct blit_geom
{
   /*
    * src_size is the total size of the source image. src is a subrectangle
    * of the rectangle (0,0)->src_size
    */
   GLint       src_size[2];

   /* For each dimension, whether to flip it */
   bool        flip[2];

   struct box  src;
   struct box  dst;
};

/* Call this when you shutdown the whole process */
extern void glxx_blitframebuffer_shutdown(void);

#ifndef NDEBUG
#include <EGL/egl.h>
#include <EGL/eglext.h>
extern void glxx_debug_save_image(const char *fname, const khrn_image *img);
#endif

void glxx_tmu_blit_framebuffer(
      GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
      GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
      GLbitfield mask, GLenum filter);

#endif /* GLXX_TMU_BLIT_H */
