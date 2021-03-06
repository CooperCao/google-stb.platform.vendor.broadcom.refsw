/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.
Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
   header file for textures used  by the server state
=============================================================================*/
#ifndef GLXX_TEXTURES_H
#define GLXX_TEXTURES_H

#include "vcos.h"
#include "middleware/khronos/glxx/glxx_texture.h"

typedef struct
{
   GLXX_TEXTURE_T *m_2d;
   GLXX_TEXTURE_T *m_external;
   GLXX_TEXTURE_T *m_cube;
   GLXX_TEXTURE_T *m_3d;
   GLXX_TEXTURE_T *m_2darray;
   GLXX_TEXTURE_T *m_1d;
   GLXX_TEXTURE_T *m_1darray;
   GLXX_TEXTURE_T *m_2dmultisample;
} GLXX_TEXTURES_T;

extern bool glxx_textures_create(GLXX_TEXTURES_T *textures);
extern void glxx_textures_release(GLXX_TEXTURES_T *textures);
extern void glxx_textures_assign(GLXX_TEXTURES_T *tex1, GLXX_TEXTURES_T *tex2);

extern GLXX_TEXTURE_T* glxx_textures_get_texture(const GLXX_TEXTURES_T *textures,
      enum glxx_tex_target target);
extern void glxx_textures_set_texture(GLXX_TEXTURES_T *textures,
      GLXX_TEXTURE_T *texture);
#endif
