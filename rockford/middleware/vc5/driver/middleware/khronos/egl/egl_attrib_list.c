/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#include "egl_attrib_list.h"

EGLAttribKHR egl_attrib_list_item(const void **list_ptr,
      EGL_AttribType type, bool increment)
{
   EGLAttribKHR item = EGL_NONE;

   switch (type)
   {
   case attrib_EGLint:
   {
      const EGLint **list = (const EGLint **)list_ptr;
      item = increment ? *((*list)++) : **list;
      break;
   }
   case attrib_EGLAttribKHR:
   /* no case for attrib_EGLAttrib == attrib_EGLAttribKHR */
   {
      const EGLAttribKHR **list = (const EGLAttribKHR **)list_ptr;
      item = increment ? *((*list)++) : **list;
      break;
   }
   /* default case left out to let the compiler catch unhandled enums */
   }

   return item;
}

bool egl_next_attrib(const void **list_ptr, EGL_AttribType type,
      EGLint *name, EGLAttribKHR *value)
{
   bool got_next = false;
   if (*list_ptr)
   {
      *name = (EGLint)egl_attrib_list_item(list_ptr, type, /*increment=*/true);
      if (*name != EGL_NONE)
      {
         *value = egl_attrib_list_item(list_ptr, type, /*increment=*/true);
         got_next = true;
      }
   }
   return got_next;
}
