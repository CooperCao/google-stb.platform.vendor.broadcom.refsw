/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES   1
#endif

#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES 1
#endif

#ifndef EMULATED
#if V3D_TECH_VERSION >= 3
#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3ext_brcm.h>
#else
#include <GLES2/gl2.h>
#endif
#else
#include "etc.h"
#endif
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>
#include <KHR/khrplatform.h>

#ifndef EMULATED
#include <GLES/gl.h>
#include <GLES/glext.h>
#endif
