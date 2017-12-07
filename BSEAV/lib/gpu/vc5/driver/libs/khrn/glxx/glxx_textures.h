/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_TEXTURES_H
#define GLXX_TEXTURES_H

#include "vcos.h"
#include "glxx_texture.h"

typedef struct
{
   GLXX_TEXTURE_T *m_2d;
   GLXX_TEXTURE_T *m_external;
   GLXX_TEXTURE_T *m_cube;
   GLXX_TEXTURE_T *m_cube_array;
   GLXX_TEXTURE_T *m_3d;
   GLXX_TEXTURE_T *m_2darray;
#if V3D_VER_AT_LEAST(4,0,2,0)
   GLXX_TEXTURE_T *m_1d;
   GLXX_TEXTURE_T *m_1darray;
#endif
   GLXX_TEXTURE_T *m_2dmultisample;
   GLXX_TEXTURE_T *m_2dmultisample_array;
   GLXX_TEXTURE_T *m_texbuffer;
} GLXX_TEXTURES_T;

extern bool glxx_textures_create(GLXX_TEXTURES_T *textures);
extern void glxx_textures_release(GLXX_TEXTURES_T *textures);
extern void glxx_textures_assign(GLXX_TEXTURES_T *tex1, GLXX_TEXTURES_T *tex2);

extern GLXX_TEXTURE_T* glxx_textures_get_texture(const GLXX_TEXTURES_T *textures,
      enum glxx_tex_target target);
extern void glxx_textures_set_texture(GLXX_TEXTURES_T *textures,
      GLXX_TEXTURE_T *texture);
#endif
