/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  gl11
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GL11_DRAW_H
#define GL11_DRAW_H

#include "middleware/khronos/glxx/glxx_server.h"

bool gl11_cache_uniforms(GLXX_SERVER_STATE_T *state, KHRN_FMEM_T *fmem);
bool gl11_is_points(GLXX_PRIMITIVE_T draw_mode);
bool gl11_is_lines(GLXX_PRIMITIVE_T draw_mode);
uint32_t gl11_compute_vertex(GL11_STATE_T *state, GLXX_PRIMITIVE_T draw_mode);
uint32_t gl11_compute_fragment(GL11_STATE_T *state,
                               GLXX_PRIMITIVE_T draw_mode,
                               const GLXX_HW_FRAMEBUFFER_T *fb);
void gl11_compute_texture_key(GLXX_SERVER_STATE_T *state, bool points);

#endif
