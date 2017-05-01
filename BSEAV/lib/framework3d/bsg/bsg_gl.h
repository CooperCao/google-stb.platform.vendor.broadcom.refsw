/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_GL_H__
#define __BSG_GL_H__

#include "bsg_common.h"

// Avoid some namespace pollution by X
#define Time X_Time__
#define Region X_Region__

#include <EGL/egl.h>
#include <EGL/eglext.h>

#undef Time
#undef Region

#include "bsg_glapi.h"


#endif /* __BSG_GL_H__ */
