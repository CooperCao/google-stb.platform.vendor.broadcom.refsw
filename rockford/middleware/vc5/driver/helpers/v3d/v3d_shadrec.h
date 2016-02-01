/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef V3D_SHADREC_H
#define V3D_SHADREC_H

#include "helpers/v3d/v3d_gen.h"

static inline uint32_t v3d_attr_get_num_elems(const V3D_SHADREC_GL_ATTR_T *attr)
{
   switch (attr->type)
   {
   case V3D_ATTR_TYPE_INT2_10_10_10:   return 4; /* attr->size is ignored */
   default:                            return attr->size;
   }
}

static inline uint32_t v3d_attr_get_word_size_in_memory(const V3D_SHADREC_GL_ATTR_T *attr)
{
   switch (attr->type)
   {
   case V3D_ATTR_TYPE_HALF_FLOAT:      return 2;
   case V3D_ATTR_TYPE_FLOAT:           return 4;
   case V3D_ATTR_TYPE_FIXED:           return 4;
   case V3D_ATTR_TYPE_BYTE:            return 1;
   case V3D_ATTR_TYPE_SHORT:           return 2;
   case V3D_ATTR_TYPE_INT:             return 4;
   case V3D_ATTR_TYPE_INT2_10_10_10:   return 4;
   default:                            unreachable(); return 0;
   }
}

static inline uint32_t v3d_attr_get_size_in_memory(const V3D_SHADREC_GL_ATTR_T *attr)
{
   uint32_t size = v3d_attr_get_word_size_in_memory(attr);
   if (attr->type != V3D_ATTR_TYPE_INT2_10_10_10)
      size *= attr->size;
   return size;
}

static inline v3d_threading_t v3d_translate_threading(uint32_t threadability)
{
   switch (threadability) {
   case 1:  return V3D_THREADING_T1;
   case 2:  return V3D_THREADING_T2;
   case 4:  return V3D_THREADING_T4;
   default: unreachable(); return V3D_THREADING_INVALID;
   }
}

static inline uint32_t v3d_get_threadability(v3d_threading_t threading)
{
   switch (threading) {
   case V3D_THREADING_T1:  return 1;
   case V3D_THREADING_T2:  return 2;
   case V3D_THREADING_T4:  return 4;
   default: unreachable(); return 0;
   }
}

#endif
