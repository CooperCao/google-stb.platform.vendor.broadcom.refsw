/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  gl11
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GL11_SHADERS_H__
#define GL11_SHADERS_H__

#include "middleware/khronos/glxx/glxx_server.h"  /* TODO: For GL11_STATE_T */
#include "middleware/khronos/glsl/glsl_dataflow.h"

#define GL11_STATE_OFFSET(x) (offsetof(GL11_STATE_T, x)/sizeof(uint32_t))

#define GL11_VARYING_COLOR 0                /* vec4 */
#define GL11_VARYING_BACK_COLOR 1           /* vec4 */
#define GL11_VARYING_EYESPACE 2             /* vec3 */
#define GL11_VARYING_POINT_SIZE 3           /* [point_size] */
#define GL11_VARYING_TEX_COORD(i) (4+(i))   /* vec2 or vec3 */
#define GL11_NUM_VARYINGS 8

IRShader *gl11_ir_shader_from_nodes(Dataflow **df, int count);
LinkMap  *gl11_link_map_from_bindings(int out_count, int in_count, int unif_count, int *unif_bindings);

void gl11_get_cvshader(const GL11_CACHE_KEY_T *v, IRShader **sh_out, LinkMap **lm_out);
void gl11_get_fshader (const GL11_CACHE_KEY_T *v, IRShader **sh_out, LinkMap **lm_out);

#endif
