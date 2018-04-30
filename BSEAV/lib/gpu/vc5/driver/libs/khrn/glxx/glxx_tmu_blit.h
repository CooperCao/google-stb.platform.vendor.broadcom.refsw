/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "gl_public_api.h"
#include "../common/khrn_int_common.h"

void glxx_tmu_blit_framebuffer(
      GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
      GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
      GLbitfield mask, GLenum filter);
