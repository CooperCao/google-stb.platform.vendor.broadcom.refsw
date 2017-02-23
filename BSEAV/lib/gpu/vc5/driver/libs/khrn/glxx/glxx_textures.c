/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
   textures used  by the server state
=============================================================================*/
#include "glxx_textures.h"

bool glxx_textures_create(GLXX_TEXTURES_T *textures)
{
   memset(textures, 0, sizeof(GLXX_TEXTURES_T));

   textures->m_2d = glxx_texture_create(GL_TEXTURE_2D, 0);
   if (textures->m_2d == NULL)
      goto end;
   textures->m_external = glxx_texture_create(GL_TEXTURE_EXTERNAL_OES, 0);
   if (textures->m_external == NULL)
      goto end;
   textures->m_cube = glxx_texture_create(GL_TEXTURE_CUBE_MAP, 0);
   if (textures->m_cube == NULL)
      goto end;
   textures->m_cube_array = glxx_texture_create(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
   if (textures->m_cube_array == NULL)
      goto end;
   textures->m_3d = glxx_texture_create(GL_TEXTURE_3D, 0);
   if (textures->m_3d == NULL)
      goto end;
   textures->m_2darray = glxx_texture_create(GL_TEXTURE_2D_ARRAY, 0);
   if (textures->m_2darray == NULL)
      goto end;
   textures->m_1d = glxx_texture_create(GL_TEXTURE_1D_BRCM, 0);
   if (textures->m_1d == NULL)
      goto end;
   textures->m_1darray = glxx_texture_create(GL_TEXTURE_1D_ARRAY_BRCM, 0);
   if (textures->m_1darray == NULL)
      goto end;
   textures->m_2dmultisample = glxx_texture_create(GL_TEXTURE_2D_MULTISAMPLE, 0);
   if (textures->m_2dmultisample == NULL)
      goto end;
   textures->m_2dmultisample_array = glxx_texture_create(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, 0);
   if (textures->m_2dmultisample_array == NULL)
      goto end;
   textures->m_texbuffer = glxx_texture_create(GL_TEXTURE_BUFFER, 0);
   if (textures->m_texbuffer == NULL)
      goto end;

   return true;
end:
   glxx_textures_release(textures);
   return false;
}

void glxx_textures_release(GLXX_TEXTURES_T *textures)
{
   KHRN_MEM_ASSIGN(textures->m_2d, NULL);
   KHRN_MEM_ASSIGN(textures->m_external, NULL);
   KHRN_MEM_ASSIGN(textures->m_cube, NULL);
   KHRN_MEM_ASSIGN(textures->m_cube_array, NULL);
   KHRN_MEM_ASSIGN(textures->m_3d, NULL);
   KHRN_MEM_ASSIGN(textures->m_2darray, NULL);
   KHRN_MEM_ASSIGN(textures->m_1d, NULL);
   KHRN_MEM_ASSIGN(textures->m_1darray, NULL);
   KHRN_MEM_ASSIGN(textures->m_2dmultisample, NULL);
   KHRN_MEM_ASSIGN(textures->m_2dmultisample_array, NULL);
   KHRN_MEM_ASSIGN(textures->m_texbuffer, NULL);
}

void glxx_textures_assign(GLXX_TEXTURES_T *tex1, GLXX_TEXTURES_T *tex2)
{
   KHRN_MEM_ASSIGN(tex1->m_2d, tex2->m_2d);
   KHRN_MEM_ASSIGN(tex1->m_external, tex2->m_external);
   KHRN_MEM_ASSIGN(tex1->m_cube, tex2->m_cube);
   KHRN_MEM_ASSIGN(tex1->m_cube_array, tex2->m_cube_array);
   KHRN_MEM_ASSIGN(tex1->m_3d, tex2->m_3d);
   KHRN_MEM_ASSIGN(tex1->m_2darray, tex2->m_2darray);
   KHRN_MEM_ASSIGN(tex1->m_1d, tex2->m_1d);
   KHRN_MEM_ASSIGN(tex1->m_1darray, tex2->m_1darray);
   KHRN_MEM_ASSIGN(tex1->m_2dmultisample, tex2->m_2dmultisample);
   KHRN_MEM_ASSIGN(tex1->m_2dmultisample_array, tex2->m_2dmultisample_array);
   KHRN_MEM_ASSIGN(tex1->m_texbuffer, tex2->m_texbuffer);
}

GLXX_TEXTURE_T* glxx_textures_get_texture(const GLXX_TEXTURES_T *textures,
      enum glxx_tex_target target)
{
   GLXX_TEXTURE_T *texture = NULL;
   switch (target)
   {
   case GL_TEXTURE_2D:
      texture = textures->m_2d;
      break;
   case GL_TEXTURE_EXTERNAL_OES:
      texture = textures->m_external;
      break;
   case GL_TEXTURE_CUBE_MAP:
      texture = textures->m_cube;
      break;
   case GL_TEXTURE_CUBE_MAP_ARRAY:
      texture = textures->m_cube_array;
      break;
   case GL_TEXTURE_3D:
      texture = textures->m_3d;
      break;
   case GL_TEXTURE_2D_ARRAY:
      texture = textures->m_2darray;
      break;
   case GL_TEXTURE_1D_BRCM:
      texture = textures->m_1d;
      break;
   case GL_TEXTURE_1D_ARRAY_BRCM:
      texture = textures->m_1darray;
      break;
   case GL_TEXTURE_2D_MULTISAMPLE:
      texture = textures->m_2dmultisample;
      break;
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      texture = textures->m_2dmultisample_array;
      break;
   case GL_TEXTURE_BUFFER:
      texture = textures->m_texbuffer;
      break;
   default:
      unreachable();
   }
   return texture;
}

void glxx_textures_set_texture(GLXX_TEXTURES_T *textures, GLXX_TEXTURE_T *texture)
{
   switch (texture->target)
   {
   case GL_TEXTURE_2D:
      KHRN_MEM_ASSIGN(textures->m_2d, texture);
      break;
   case GL_TEXTURE_EXTERNAL_OES:
      KHRN_MEM_ASSIGN(textures->m_external, texture);
      break;
   case GL_TEXTURE_CUBE_MAP:
      KHRN_MEM_ASSIGN(textures->m_cube, texture);
      break;
   case GL_TEXTURE_CUBE_MAP_ARRAY:
      KHRN_MEM_ASSIGN(textures->m_cube_array, texture);
      break;
   case GL_TEXTURE_3D:
      KHRN_MEM_ASSIGN(textures->m_3d, texture);
      break;
   case GL_TEXTURE_2D_ARRAY:
      KHRN_MEM_ASSIGN(textures->m_2darray, texture);
      break;
   case GL_TEXTURE_1D_BRCM:
      KHRN_MEM_ASSIGN(textures->m_1d, texture);
      break;
   case GL_TEXTURE_1D_ARRAY_BRCM:
      KHRN_MEM_ASSIGN(textures->m_1darray, texture);
      break;
   case GL_TEXTURE_2D_MULTISAMPLE:
      KHRN_MEM_ASSIGN(textures->m_2dmultisample, texture);
      break;
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      KHRN_MEM_ASSIGN(textures->m_2dmultisample_array, texture);
      break;
   case GL_TEXTURE_BUFFER:
      KHRN_MEM_ASSIGN(textures->m_texbuffer, texture);
      break;
   default:
      unreachable();
   }
}
