/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
GL enum types
=============================================================================*/
#ifndef GLXX_ENUM_TYPES_H
#define GLXX_ENUM_TYPES_H

#include "gl_public_api.h"

#define enumify(x) E_##x=x

typedef enum
{
   enumify(GL_UNSIGNED_BYTE),
   enumify(GL_UNSIGNED_SHORT),
   enumify(GL_UNSIGNED_INT),
}GLXX_INDEX_T;

typedef enum
{
   enumify(GL_POINTS),
   enumify(GL_LINES),
   enumify(GL_LINE_LOOP),
   enumify(GL_LINE_STRIP),
   enumify(GL_TRIANGLES),
   enumify(GL_TRIANGLE_STRIP),
   enumify(GL_TRIANGLE_FAN),
}GLXX_PRIMITIVE_T;

#endif /* GLXX_ENUM_TYPES_H */
