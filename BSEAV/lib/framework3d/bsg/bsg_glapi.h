/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_GLAPI_H__
#define __BSG_GLAPI_H__

#ifdef BSG_USE_ES3
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES2/gl2ext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#ifdef BSG_USE_ES32
#include <GLES3/gl32.h>
#endif

#endif // __BSG_GLAPI_H__
