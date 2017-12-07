/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GL11_SHADER_4_H
#define GL11_SHADER_4_H

#include "gl11_shader_cache.h"
#include "../glsl/glsl_dataflow.h"
#include "../glsl/glsl_ir_program.h"
#include "../glxx/glxx_server.h"  /* TODO: For GL11_STATE_T */
#include "../glxx/glxx_shader_ops.h"

#define GL11_STATE_OFFSET(x) (offsetof(GL11_STATE_T, x)/sizeof(uint32_t))

#define GL11_VARYING_COLOR 0                /* vec4 */
#define GL11_VARYING_BACK_COLOR 1           /* vec4 */
#define GL11_VARYING_EYESPACE 2             /* vec3 */
#define GL11_VARYING_POINT_SIZE 3           /* [point_size] */
#define GL11_VARYING_TEX_COORD(i) (4+(i))   /* vec2 or vec3 */
#define GL11_NUM_VARYINGS 8

uint32_t  gl11_get_live_attr_set(const GL11_CACHE_KEY_T *shader);
IRShader *gl11_ir_shader_from_nodes(Dataflow **df, int count, int *out_bindings);
LinkMap  *gl11_link_map_from_bindings(int out_count, const int *out_bindings, int in_count, int unif_count, const int *unif_bindings);

void gl11_load_inputs(GLXX_VEC4_T *inputs, unsigned count);

void gl11_get_cvshader(const GL11_CACHE_KEY_T *v, IRShader **sh_out, LinkMap **lm_out);
void gl11_get_fshader (const GL11_CACHE_KEY_T *v, IRShader **sh_out, LinkMap **lm_out);

IR_PROGRAM_T *gl11_shader_get_dataflow(const GL11_CACHE_KEY_T *v);

#endif
