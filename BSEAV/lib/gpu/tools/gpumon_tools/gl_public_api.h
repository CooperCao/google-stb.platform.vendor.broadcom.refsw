/******************************************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#ifdef HAS_ES3
# include <GLES3/gl32.h>
# include <GLES3/gl3ext.h>
# include <GLES3/gl3ext_brcm.h>

# ifndef __gl2_h_
# define __gl2_h_
# endif

#else /* No ES3 */

# include <GLES2/gl2.h>

#endif

#include <GLES2/gl2ext.h>

/* Always have EGL & ES1 */
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <GLES/gl.h>
#include <GLES/glext.h>
