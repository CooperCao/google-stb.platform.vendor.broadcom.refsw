/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glxx
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GL_PUBLIC_API_H
#define GL_PUBLIC_API_H

/* We want the prototypes for extension functions so that the compiler will
 * check that the implementation signatures are correct */
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include "interface/khronos/include/GLES3/gl32.h"
#include "interface/khronos/include/GLES3/gl3ext.h"
#include "interface/khronos/include/GLES3/gl3ext_brcm.h"
/* Let GLES/gl.h provide everything we don't have. This fake */
/* is not even a lie, we provide GLES2 functionality... */
#ifndef __gl2_h_
#define __gl2_h_
#endif
/* work around some missing defines, we can resolve this properly as soon a 'real' gl3ext.h is defined... */
#include "interface/khronos/include/GLES2/gl2ext.h"

#include "interface/khronos/include/GLES/gl.h"
#include "interface/khronos/include/GLES/glext.h"

#endif
