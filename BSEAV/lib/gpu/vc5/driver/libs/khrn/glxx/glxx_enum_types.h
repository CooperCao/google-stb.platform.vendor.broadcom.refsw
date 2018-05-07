/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_ENUM_TYPES_H
#define GLXX_ENUM_TYPES_H

#include "gl_public_api.h"
#include "libs/core/v3d/v3d_ver.h"

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
#if V3D_VER_AT_LEAST(4,1,34,0)
   enumify(GL_PATCHES),
   enumify(GL_LINES_ADJACENCY),
   enumify(GL_LINE_STRIP_ADJACENCY),
   enumify(GL_TRIANGLES_ADJACENCY),
   enumify(GL_TRIANGLE_STRIP_ADJACENCY),
#endif
}GLXX_PRIMITIVE_T;

#endif /* GLXX_ENUM_TYPES_H */
