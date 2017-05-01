/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GL_PUBLIC_API_H
#define GL_PUBLIC_API_H

/* We want the prototypes for extension functions so that the compiler will
 * check that the implementation signatures are correct */
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3ext_brcm.h>
/* Let GLES/gl.h provide everything we don't have. This fake */
/* is not even a lie, we provide GLES2 functionality... */
#ifndef __gl2_h_
#define __gl2_h_
#endif
/* work around some missing defines, we can resolve this properly as soon a 'real' gl3ext.h is defined... */
#include <GLES2/gl2ext.h>

#include <GLES/gl.h>
#include <GLES/glext.h>

#endif
