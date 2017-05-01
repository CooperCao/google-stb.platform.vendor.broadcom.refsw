/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "gl_public_api.h"
#include "glxx_server_state.h"
#include <stdbool.h>

/* Returns false if cannot do this blit with the TLB; the caller should try
 * another method. */
extern bool glxx_try_tlb_blit_framebuffer(GLXX_SERVER_STATE_T *state,
   GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
   GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
   GLbitfield mask);
