/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "gl11_shader.h"
#include "../glsl/glsl_dataflow.h"
#include "../glxx/glxx_shader_ops.h"

typedef struct {
   GLXX_VEC4_T  vertex;
   Dataflow    *point_size;
   GLXX_VEC4_T  varying[GL11_NUM_VARYINGS];
} GL11_VERTEX_CARD_T;

struct light_s {
   GLXX_VEC3_T position;
   GLXX_VEC3_T position3;

   struct {
      Dataflow *c;
      Dataflow *l;
      Dataflow *q;
   } atten;

   struct spot_s {
      GLXX_VEC3_T direction;
      Dataflow *exponent;
      Dataflow *cos_cutoff;
   } spot;

   GLXX_VEC3_T ambient;
   GLXX_VEC3_T diffuse;
   GLXX_VEC3_T specular;
};

struct mat_s {
   Dataflow *shininess;
   GLXX_VEC3_T emission;
   GLXX_VEC3_T ambient;
   GLXX_VEC3_T specular;
   GLXX_VEC4_T diffuse;
};

struct point_params_s {
   GLXX_VEC3_T attenuation;

   Dataflow *size_min;
   Dataflow *size_max;
};

/*
    VP = vec3(glLightSource[i].position);
    VP = normalize(VP);
    attenuation = 1.0;
*/
static void lighting_calc_vp_nonlocal(GLXX_VEC3_T *result, Dataflow **attenuation, const struct light_s *light)
{
   GLXX_VEC3_T VP;

   glxx_v_normalize3(&VP, &light->position);
   *attenuation = glxx_cfloat(1.0f);

   *result = VP;
}

/*
    VP = gl_LightSource[i].position3 - ecPosition3;
    d = length(VP);

    VP = normalize(VP);
    attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +
                         gl_LightSource[i].linearAttenuation * d +
                         gl_LightSource[i].quadraticAttenuation * d * d);
*/

static void lighting_calc_vp_local(GLXX_VEC3_T *result, Dataflow **attenuation, const struct light_s *light, const GLXX_VEC3_T *position)
{
   GLXX_VEC3_T VP;
   Dataflow *d, *dd, *rd;

   glxx_v_sub3(&VP, &light->position3, position);
   dd = glxx_dot3(&VP, &VP);
   rd = glxx_rsqrt(dd);
   glxx_v_scale3(&VP, rd, &VP);    //== normalize(VP);

   d  = glxx_recip(rd);
   *attenuation = glxx_recip(glxx_sum3(light->atten.c,
                                       glxx_mul(light->atten.l, d),
                                       glxx_mul(light->atten.q, dd)));

   *result = VP;
}

 /*

    spotDot = glxx_dot(-VP, gl_LightSource[i].spotDirection);
    if (spotDot < gl_lightSource[i].spotCosCutoff)
       spotAttenuation = 0.0;
    else
       spotAttenuation = pow(spotDot, gl_LightSource[i].spotExponent);
    attenuation *= spotAttenuation;
*/

static Dataflow *lighting_apply_spot(Dataflow *attenuation,
                                     const GLXX_VEC3_T *VP,
                                     const struct spot_s *spot)
{
   Dataflow *spotDot;
   Dataflow *spotAttenuation;

   spotDot = glxx_negate(glxx_dot3(VP, &spot->direction));

   spotAttenuation = glxx_cond(
      glxx_less(spotDot, spot->cos_cutoff),
      glxx_cfloat(0.0f),
      glxx_pow(spotDot, spot->exponent));

   return glxx_mul(attenuation, spotAttenuation);
}

 /*
    nDotVP = max(0.0, glxx_dot(normal, VP));
    nDotHV = max(0.0, glxx_dot(normal, normalize(VP + eye)));
    if (nDotVP == 0.0)
       pf = 0.0;
    else
       pf = pow(nDotHV, gl_FrontMaterial.shininess);
    ambient += glLightSource[i].ambient * attenuation;
    diffuse += glLightSource[i].diffuse * nDotVP * attenuation;
    specular += glLightSource[i].specular * pf * attenuation;
 */

static void lighting_accumulate(GLXX_VEC3_T          *ambient,
                                GLXX_VEC3_T          *diffuse,
                                GLXX_VEC3_T          *specular,
                                const GLXX_VEC3_T    *VP,
                                Dataflow             *attenuation,
                                const GLXX_VEC3_T    *normal,
                                const struct light_s *l,
                                const struct mat_s   *m)
{
   Dataflow *nDotVP, *nDotHV, *pf, *zero, *one;
   GLXX_VEC3_T amb, diff, spec;
   GLXX_VEC3_T eye;

   zero = glxx_cfloat(0.0f);
   one = glxx_cfloat(1.0f);
   glxx_v_vec3(&eye, zero, zero, glxx_cfloat(1.0f));

   nDotVP = glxx_fmax(zero, glxx_dot3(normal, VP));
   {
      GLXX_VEC3_T tmp;
      glxx_v_add3(&tmp, VP, &eye);
      glxx_v_normalize3(&tmp, &tmp);
      nDotHV = glxx_fmax(zero, glxx_dot3(normal, &tmp));
   }

   pf = glxx_cond(
      glxx_equal(nDotVP, zero),
      zero,
      glxx_pow(nDotHV, m->shininess));

   pf = glxx_cond(
      glxx_equal(m->shininess, zero),
      one,
      pf);

   glxx_v_scale3(&amb, attenuation, &l->ambient);
   glxx_v_add3  (&amb, ambient,     &amb);
   glxx_v_scale3(&diff, glxx_mul(nDotVP, attenuation), &l->diffuse);
   glxx_v_add3  (&diff, diffuse,                       &diff);
   glxx_v_scale3(&spec, glxx_mul(pf, attenuation), &l->specular);
   glxx_v_add3  (&spec, specular,                  &spec);

   *ambient  = amb;
   *diffuse  = diff;
   *specular = spec;
}

static void lighting_combine(GLXX_VEC4_T        *result,
                             const GLXX_VEC3_T  *ambient,
                             const GLXX_VEC3_T  *diffuse,
                             const GLXX_VEC3_T  *specular,
                             const struct mat_s *m)
{
   GLXX_VEC3_T combined;
   GLXX_VEC3_T amb, diff, spec;
   GLXX_VEC4_T accum;

   glxx_v_mul3 (&amb,  ambient, &m->ambient);
   glxx_v_dropw(&diff, &m->diffuse);
   glxx_v_mul3 (&diff, diffuse, &diff);
   glxx_v_mul3 (&spec, specular,&m->specular);

   combined = m->emission;
   glxx_v_add3 (&combined, &combined, &amb);
   glxx_v_add3 (&combined, &combined, &diff);
   glxx_v_add3 (&combined, &combined, &spec);

   glxx_v_vec4  (&accum, combined.x, combined.y, combined.z, m->diffuse.w);
   glxx_v_clamp4(&accum, &accum);
   *result = accum;
}

static void lighting(GLXX_VEC4_T          *front_out,
                     GLXX_VEC4_T          *back_out,
                     uint32_t              light_state,
                     const GLXX_VEC3_T    *position,
                     const GLXX_VEC3_T    *normal,
                     const GLXX_VEC3_T    *lightmodel_ambient,
                     const struct light_s *lights,
                     const struct mat_s   *m,
                     bool                  two_side)
{
   Dataflow *zero;
   GLXX_VEC3_T ambient, diffuse, specular;
   GLXX_VEC3_T ambientb, diffuseb, specularb, normalb;
   GLXX_VEC4_T accum_front, accum_back;
   int i;

   zero = glxx_cfloat(0.0f);
   ambient = *lightmodel_ambient;
   glxx_v_vec3(&diffuse,  zero, zero, zero);
   glxx_v_vec3(&specular, zero, zero, zero);

   if (!two_side) {
      normalb = *normal;//initialised to avoid warnings
   } else {
      ambientb  = ambient;
      diffuseb  = diffuse;
      specularb = specular;
      glxx_v_negate3(&normalb, normal);
   }

   for (i = 0; i < GL11_CONFIG_MAX_LIGHTS; i++) {
      uint32_t light = (light_state >> i) & GL11_LIGHT_M;

      if (light & GL11_LIGHT_ENABLE) {
         GLXX_VEC3_T VP;
         Dataflow *attenuation;

         if (!(light & GL11_LIGHT_LOCAL))
            lighting_calc_vp_nonlocal(&VP, &attenuation, &lights[i]);
         else
            lighting_calc_vp_local   (&VP, &attenuation, &lights[i], position);

         if (light & GL11_LIGHT_SPOT)
            attenuation = lighting_apply_spot(attenuation, &VP, &lights[i].spot);

         lighting_accumulate(&ambient, &diffuse, &specular, &VP, attenuation, normal, &lights[i], m);

         if (two_side)
            lighting_accumulate(&ambientb, &diffuseb, &specularb, &VP, attenuation, &normalb, &lights[i], m);
      }
   }

   lighting_combine(&accum_front, &ambient, &diffuse, &specular, m);

   if (two_side) {
      lighting_combine(&accum_back, &ambientb, &diffuseb, &specularb, m);
   }

   *front_out = accum_front;
   if(two_side) {
      *back_out  = accum_back;
   }
}

static Dataflow *vertex_calculate_point_sizes(const GLXX_VEC3_T     *eyespace,
                                              Dataflow              *size,
                                              const struct point_params_s *params)
{
   Dataflow *dd = glxx_dot3(eyespace, eyespace);
   Dataflow *d  = glxx_recip(glxx_rsqrt(dd));

   Dataflow *attenuation = glxx_sum3(params->attenuation.x,
                                     glxx_mul(params->attenuation.y, d),
                                     glxx_mul(params->attenuation.z, dd));

   size = glxx_mul(size, glxx_rsqrt(attenuation));

   return glxx_fmin(glxx_fmax(size, params->size_min), params->size_max);
}

static void matrix_palette_invert_4x4_to_3x3(Dataflow *mat_in[16], Dataflow *mat_out[9], bool will_normalize_later, Dataflow **determinant) {
   mat_out[0] = glxx_sub(glxx_mul(mat_in[5],mat_in[10]), glxx_mul(mat_in[9], mat_in[6]));
   mat_out[1] = glxx_sub(glxx_mul(mat_in[9],mat_in[2]), glxx_mul(mat_in[1], mat_in[10]));
   mat_out[2] = glxx_sub(glxx_mul(mat_in[1],mat_in[6]), glxx_mul(mat_in[5], mat_in[2]));
   mat_out[3] = glxx_sub(glxx_mul(mat_in[8],mat_in[6]), glxx_mul(mat_in[4], mat_in[10]));
   mat_out[4] = glxx_sub(glxx_mul(mat_in[0],mat_in[10]), glxx_mul(mat_in[8], mat_in[2]));
   mat_out[5] = glxx_sub(glxx_mul(mat_in[4],mat_in[2]), glxx_mul(mat_in[0], mat_in[6]));
   mat_out[6] = glxx_sub(glxx_mul(mat_in[4],mat_in[9]), glxx_mul(mat_in[8], mat_in[5]));
   mat_out[7] = glxx_sub(glxx_mul(mat_in[8],mat_in[1]), glxx_mul(mat_in[0], mat_in[9]));
   mat_out[8] = glxx_sub(glxx_mul(mat_in[0],mat_in[5]), glxx_mul(mat_in[4], mat_in[1]));

   if (!will_normalize_later)
      *determinant = glxx_add( glxx_add( glxx_mul(mat_in[0], mat_out[0]),
                                         glxx_mul(mat_in[4], mat_out[1])),
                                         glxx_mul(mat_in[8], mat_out[3]));
}

static void matrix_palette_unit_result(GLXX_VEC4_T       *unit_vertex,
                                       GLXX_VEC3_T       *unit_normal,
                                       bool               do_normal,
                                       bool               will_normalize_later,
                                       Dataflow          *base_ptr,
                                       Dataflow          *unit_idx,
                                       Dataflow          *unit_weight,
                                       const GLXX_VEC4_T *attr_vertex,
                                       const GLXX_VEC4_T *attr_normal)
{
   Dataflow *unit_matrix[16];
   Dataflow *unit_matrix_inv[9];
   GLXX_VEC4_T vertex_accum;
   GLXX_VEC3_T normal_accum;

   /* Get matrix pointer for lookup. Offset < MAX_MATRICES*16*4 < 2^24 so imul is ok */
   unit_idx = glxx_f_to_iz( unit_idx );
   unit_idx = glxx_fmax   ( unit_idx, glxx_cint(0) );
   unit_idx = glxx_fmin   ( unit_idx, glxx_cint(GL11_CONFIG_MAX_PALETTE_MATRICES_OES - 1) );
   unit_idx = glxx_mul    ( unit_idx, glxx_cint(16 * 4) );      /* Byte offset = matrix_idx * 16*4 */
   unit_idx = glxx_add    ( unit_idx, base_ptr  );              /* Not really an index now, more a pointer */

   for (int i=0; i<4; i++) {
      Dataflow *vec = glsl_dataflow_construct_vector_load(DF_FLOAT, glxx_add(unit_idx, glxx_cint(16*i)));
      vec->u.vector_load.required_components = 0xf;
      for (unsigned j=0; j<4; j++)
         unit_matrix[4*i+j] = glsl_dataflow_construct_get_vec4_component(j, vec, DF_FLOAT);
   }

   {
      GLXX_VEC4_T row[4];
      GLXX_VEC4_T unit_v;
      GLXX_VEC4_T uvec4;
      glxx_v_vec4(&row[0], unit_matrix[0], unit_matrix[4], unit_matrix[8],  unit_matrix[12]);
      glxx_v_vec4(&row[1], unit_matrix[1], unit_matrix[5], unit_matrix[9],  unit_matrix[13]);
      glxx_v_vec4(&row[2], unit_matrix[2], unit_matrix[6], unit_matrix[10], unit_matrix[14]);
      glxx_v_vec4(&row[3], unit_matrix[3], unit_matrix[7], unit_matrix[11], unit_matrix[15]);
      glxx_v_transform4x4(&unit_v, &row[0], &row[1], &row[2], &row[3], attr_vertex);

      glxx_v_rep4(&uvec4, unit_weight);
      glxx_v_mul4(&vertex_accum, &unit_v, &uvec4);
   }

   /* Transform normal by inverse transpose */
   if (do_normal) {
      GLXX_VEC3_T row[3];
      GLXX_VEC3_T unit_n;
      GLXX_VEC3_T norm3;
      GLXX_VEC3_T uvec3;
      Dataflow *weight;
      Dataflow *determinant;
      matrix_palette_invert_4x4_to_3x3(unit_matrix, unit_matrix_inv, will_normalize_later, &determinant);

      glxx_v_dropw(&norm3, attr_normal);
      glxx_v_vec3(&row[0], unit_matrix_inv[0], unit_matrix_inv[1], unit_matrix_inv[2]);
      glxx_v_vec3(&row[1], unit_matrix_inv[3], unit_matrix_inv[4], unit_matrix_inv[5]);
      glxx_v_vec3(&row[2], unit_matrix_inv[6], unit_matrix_inv[7], unit_matrix_inv[8]);
      glxx_v_transform3x3(&unit_n, &row[0], &row[1], &row[2], &norm3);

      if (will_normalize_later) {
         weight = unit_weight;
      } else {
         weight = glxx_mul(unit_weight, glxx_recip(determinant));
      }
      glxx_v_rep3(&uvec3, weight);
      glxx_v_mul3(&normal_accum, &unit_n, &uvec3);
   }

   *unit_vertex = vertex_accum;
   if(do_normal) {
      *unit_normal = normal_accum;
   }
}

static void matrix_palette_transformations(GLXX_VEC4_T       *vertex_res,
                                           GLXX_VEC3_T       *eyespace_res,
                                           GLXX_VEC3_T       *normal_res,
                                           const GLXX_VEC4_T *attr,
                                           const GLXX_MAT4_T *projection,
                                           Dataflow          *base_ptr,
                                           int                n_vertex_units,
                                           bool               do_normal,
                                           bool               will_normalize_later)
{
   GLXX_VEC4_T vertex, unit_vertex;
   GLXX_VEC3_T normal, unit_normal;
   GLXX_VEC4_T vertex_accum;
   GLXX_VEC3_T eyespace_accum;

   Dataflow *indices[4] = { attr[GL11_IX_MATRIX_INDEX].x, attr[GL11_IX_MATRIX_INDEX].y,
                            attr[GL11_IX_MATRIX_INDEX].z, attr[GL11_IX_MATRIX_INDEX].w };
   Dataflow *weights[4] = { attr[GL11_IX_MATRIX_WEIGHT].x, attr[GL11_IX_MATRIX_WEIGHT].y,
                            attr[GL11_IX_MATRIX_WEIGHT].z, attr[GL11_IX_MATRIX_WEIGHT].w };

   assert(n_vertex_units > 0);
   assert(n_vertex_units <= 4);      /* In fact, 3 is the current maximum, but this function will work for 4 */

   matrix_palette_unit_result(&vertex,
                              &normal,
                              do_normal,
                              will_normalize_later,
                              base_ptr,
                              indices[0],
                              weights[0],
                              &attr[GL11_IX_VERTEX],
                              &attr[GL11_IX_NORMAL]);

   for (int i=1; i<n_vertex_units; i++) {
      matrix_palette_unit_result(&unit_vertex,
                                 &unit_normal,
                                 do_normal,
                                 will_normalize_later,
                                 base_ptr,
                                 indices[i],
                                 weights[i],
                                 &attr[GL11_IX_VERTEX],
                                 &attr[GL11_IX_NORMAL]);

      glxx_v_add4(&vertex, &vertex, &unit_vertex);
      if (do_normal) {
         glxx_v_add3(&normal, &normal, &unit_normal);
      }
   }

   /* Project the vertex */
   glxx_m4_transform(&vertex_accum, projection, &vertex);
   glxx_v_dehomogenize(&eyespace_accum, &vertex);

   *vertex_res = vertex_accum;
   *eyespace_res = eyespace_accum;
   if (do_normal) {
      *normal_res = normal;
   }
}

static void drawtex_vshader(GL11_VERTEX_CARD_T *result, const GL11_CACHE_KEY_T *v, const GLXX_VEC4_T *attr)
{
   const GLXX_VEC4_T zero4 = { NULL, NULL, NULL, NULL };

   /* clear the varyings as non NULL = activated */
   for (int i = 0; i < GL11_NUM_VARYINGS; i++)
      result->varying[i] = zero4;

   result->vertex = attr[GL11_IX_VERTEX];
   result->point_size = NULL;

   glxx_v_clamp4(&result->varying[GL11_VARYING_COLOR],    &attr[GL11_IX_COLOR]); /* No Lighting */
   glxx_v_vec4  (&result->varying[GL11_VARYING_EYESPACE], glxx_cfloat(0.0f),glxx_cfloat(0.0f),glxx_cfloat(0.0f), NULL);

   for (int i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++) {
      if (v->texture[i] & GL11_TEX_ENABLE) {
         result->varying[GL11_VARYING_TEX_COORD(i)] = attr[GL11_IX_TEXTURE_COORD + i];
      }
   }
}

struct builtin_uniforms_s {
   GLXX_MAT4_T projection;
   GLXX_MAT4_T projection_modelview;
   GLXX_MAT4_T modelview;
   GLXX_MAT4_T modelview_inverse;

   GLXX_MAT4_T tex_matrix[GL11_CONFIG_MAX_TEXTURE_UNITS];

   Dataflow *mpal_base_ptr;
   struct point_params_s params;
   struct light_s l[GL11_CONFIG_MAX_LIGHTS];
   struct mat_s m;

   GLXX_VEC3_T lightmodel_ambient;
};

static void new_bindings(int *id, int *bindings, int loc, int count) {
   for (int i=0; i<count; i++) bindings[*id + i] = loc + i;
   *id += count;
}

static void new_bound_m4_u(GLXX_MAT4_T *m_nodes, int *compiler_id, int *bindings, int loc) {
   glxx_m4_u(m_nodes, *compiler_id);
   new_bindings(compiler_id, bindings, loc, 16);
}

static void new_bound_v4_u(GLXX_VEC4_T *v_nodes, int *compiler_id, int *bindings, int loc) {
   glxx_v_u4(v_nodes, *compiler_id);
   new_bindings(compiler_id, bindings, loc, 4);
}

static void new_bound_v3_u(GLXX_VEC3_T *v_nodes, int *compiler_id, int *bindings, int loc) {
   glxx_v_u3(v_nodes, *compiler_id);
   new_bindings(compiler_id, bindings, loc, 3);
}

static void new_bound_u(Dataflow **node, int *compiler_id, int *bindings, int loc) {
   *node = glxx_u(*compiler_id);
   new_bindings(compiler_id, bindings, loc, 1);
}

static void fill_state_uniforms(struct builtin_uniforms_s *unif, int *bindings) {
   int count = 0;

   unif->mpal_base_ptr = glxx_u_int(count);
   new_bindings(&count, bindings, GL11_STATE_OFFSET(palette_matrices_base_ptr), 1);

   new_bound_m4_u(&unif->projection,           &count, bindings, GL11_STATE_OFFSET(current_projection));
   new_bound_m4_u(&unif->projection_modelview, &count, bindings, GL11_STATE_OFFSET(projection_modelview));
   new_bound_m4_u(&unif->modelview,            &count, bindings, GL11_STATE_OFFSET(current_modelview));
   new_bound_m4_u(&unif->modelview_inverse,    &count, bindings, GL11_STATE_OFFSET(modelview_inv));

   new_bound_v3_u(&unif->params.attenuation, &count, bindings, GL11_STATE_OFFSET(point_params.distance_attenuation));
   new_bound_u(&unif->params.size_min, &count, bindings, GL11_STATE_OFFSET(point_params.size_min_clamped));
   new_bound_u(&unif->params.size_max, &count, bindings, GL11_STATE_OFFSET(point_params.size_max));

   for (int i=0; i<GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
      new_bound_m4_u(&unif->tex_matrix[i], &count, bindings, GL11_STATE_OFFSET(texunits[i].current_matrix));

   for (int i=0; i<GL11_CONFIG_MAX_LIGHTS; i++) {
      struct light_s *l = &unif->l[i];
      new_bound_v3_u(&l->position,  &count, bindings, GL11_STATE_OFFSET(lights[i].position));
      new_bound_v3_u(&l->position3, &count, bindings, GL11_STATE_OFFSET(lights[i].position3));
      new_bound_u(&l->atten.c, &count, bindings, GL11_STATE_OFFSET(lights[i].attenuation.constant));
      new_bound_u(&l->atten.l, &count, bindings, GL11_STATE_OFFSET(lights[i].attenuation.linear));
      new_bound_u(&l->atten.q, &count, bindings, GL11_STATE_OFFSET(lights[i].attenuation.quadratic));

      new_bound_v3_u(&l->spot.direction,  &count, bindings, GL11_STATE_OFFSET(lights[i].spot.direction));
      new_bound_u   (&l->spot.exponent,   &count, bindings, GL11_STATE_OFFSET(lights[i].spot.exponent));
      new_bound_u   (&l->spot.cos_cutoff, &count, bindings, GL11_STATE_OFFSET(lights[i].cos_cutoff));

      new_bound_v3_u(&l->ambient,  &count, bindings, GL11_STATE_OFFSET(lights[i].ambient));
      new_bound_v3_u(&l->diffuse,  &count, bindings, GL11_STATE_OFFSET(lights[i].diffuse));
      new_bound_v3_u(&l->specular, &count, bindings, GL11_STATE_OFFSET(lights[i].specular));
   }

   new_bound_u   (&unif->m.shininess, &count, bindings, GL11_STATE_OFFSET(material.shininess));
   new_bound_v3_u(&unif->m.emission,  &count, bindings, GL11_STATE_OFFSET(material.emission));
   new_bound_v3_u(&unif->m.specular,  &count, bindings, GL11_STATE_OFFSET(material.specular));
   new_bound_v3_u(&unif->m.ambient,   &count, bindings, GL11_STATE_OFFSET(material.ambient));
   new_bound_v4_u(&unif->m.diffuse,   &count, bindings, GL11_STATE_OFFSET(material.diffuse));

   new_bound_v3_u(&unif->lightmodel_ambient, &count, bindings, GL11_STATE_OFFSET(lightmodel.ambient));
}

/* Fill in the dataflow for the vertex shader.
 *
 * We take the view that filling in varyings is fine even if they are ignored,
 * while using attributes that are ignored may not be.
 */
static void vshader(GL11_VERTEX_CARD_T *result, const GL11_CACHE_KEY_T *v,
                    const GLXX_VEC4_T *attr, const struct builtin_uniforms_s *unif,
                    bool points)
{
   const GLXX_VEC4_T zero4 = { NULL, NULL, NULL, NULL };

   GLXX_VEC3_T eyespace;
   GLXX_VEC3_T normal;
   bool normalize = !(v->vertex & GL11_NO_NORMALIZE);
   bool have_lights = !!(v->vertex & GL11_LIGHTING);

   /* clear everything as non NULL = activated */
   result->point_size = NULL;
   for (int i = 0; i < GL11_NUM_VARYINGS; i++)
      result->varying[i] = zero4;

   if ( (v->vertex & GL11_MPAL_M) != 0 ) {
      uint32_t n_vertex_units = (v->vertex & GL11_MPAL_M) >> GL11_MPAL_S;

      matrix_palette_transformations(&result->vertex, &eyespace, &normal, attr,
                                     &unif->projection, unif->mpal_base_ptr, n_vertex_units,
                                     have_lights, normalize);
   } else {
      /* Normal operation here ... */
      glxx_m4_transform(&result->vertex, &unif->projection_modelview, &attr[GL11_IX_VERTEX]);

      /* We only need this if (fog_mode || points || have_lights) but doing it
       * anyway doesn't hurt */
      {
         GLXX_VEC4_T tmp;
         glxx_m4_transform(&tmp, &unif->modelview, &attr[GL11_IX_VERTEX]);
         glxx_v_dehomogenize(&eyespace, &tmp);
      }

      if (have_lights) {
         GLXX_MAT3_T m_u_inv;
         GLXX_VEC3_T tmp;

         glxx_m3_upper_left(&m_u_inv, &unif->modelview_inverse);
         glxx_m3_transpose(&m_u_inv);
         /* transform normals into eye space */
         glxx_v_dropw(&tmp, &attr[GL11_IX_NORMAL]);
         glxx_m3_transform(&normal, &m_u_inv, &tmp);
      }
   }

   glxx_v_nullw(&result->varying[GL11_VARYING_EYESPACE], &eyespace);

   if (points)
   {
      result->point_size = vertex_calculate_point_sizes(&eyespace, attr[GL11_IX_POINT_SIZE].x, &unif->params);
      glxx_v_vec4(&result->varying[GL11_VARYING_POINT_SIZE], result->point_size, NULL, NULL, NULL);
   }

   for (int i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if (v->texture[i] & GL11_TEX_ENABLE)
      {
         glxx_m4_transform(&result->varying[GL11_VARYING_TEX_COORD(i)],
                           &unif->tex_matrix[i], &attr[GL11_IX_TEXTURE_COORD+i]);
      }
   }

   if (have_lights)
   {
      const bool two_side = !!(v->vertex & GL11_TWOSIDE);
      struct mat_s material = unif->m;

      if(v->vertex & GL11_COLORMAT)
      {
         glxx_v_dropw(&material.ambient, &attr[GL11_IX_COLOR]);
         material.diffuse = attr[GL11_IX_COLOR];
      }

      /* run lighting calculation */
      if (normalize) {
         glxx_v_normalize3(&normal, &normal);
      }

      lighting(&result->varying[GL11_VARYING_COLOR],
               &result->varying[GL11_VARYING_BACK_COLOR],
               v->vertex & GL11_LIGHTS_M,
               &eyespace, &normal,
               &unif->lightmodel_ambient,
               unif->l, &material,
               two_side);
   }
   else
   {
      /* No lighting */
      glxx_v_clamp4(&result->varying[GL11_VARYING_COLOR], &attr[GL11_IX_COLOR]);
   }
}

void gl11_get_cvshader(const GL11_CACHE_KEY_T *v, IRShader **sh_out, LinkMap **lm_out) {
   glsl_dataflow_begin_construction();

   Dataflow *shaded[DF_BLOB_VERTEX_COUNT];
   int unif_count = sizeof(struct builtin_uniforms_s) / sizeof(uint32_t);
   int *unif_bindings = glsl_dataflow_malloc(unif_count * sizeof(int));

   memset(shaded, 0, sizeof(Dataflow *) * DF_BLOB_VERTEX_COUNT);

   GLXX_VEC4_T attr[GL11_IX_MAX_ATTRIBS];
   gl11_load_inputs(attr, GL11_IX_MAX_ATTRIBS);

   struct builtin_uniforms_s gl_unifs;
   fill_state_uniforms(&gl_unifs, unif_bindings);

   GL11_VERTEX_CARD_T vcard;
   if (!(v->vertex & GL11_DRAW_TEX))
      vshader(&vcard, v, attr, &gl_unifs, v->points);
   else
      drawtex_vshader(&vcard, v, attr);

   shaded[DF_VNODE_X] = vcard.vertex.x;
   shaded[DF_VNODE_Y] = vcard.vertex.y;
   shaded[DF_VNODE_Z] = vcard.vertex.z;
   shaded[DF_VNODE_W] = vcard.vertex.w;
   shaded[DF_VNODE_POINT_SIZE] = vcard.point_size;

   for (int i = 0; i < GL11_NUM_VARYINGS; i++) {
      shaded[DF_VNODE_VARY(4*i+0)] = vcard.varying[i].x;
      shaded[DF_VNODE_VARY(4*i+1)] = vcard.varying[i].y;
      shaded[DF_VNODE_VARY(4*i+2)] = vcard.varying[i].z;
      shaded[DF_VNODE_VARY(4*i+3)] = vcard.varying[i].w;
   }

   int out_bindings[DF_BLOB_VERTEX_COUNT];
   *sh_out = gl11_ir_shader_from_nodes(shaded, DF_BLOB_VERTEX_COUNT, out_bindings);
   *lm_out = gl11_link_map_from_bindings(DF_BLOB_VERTEX_COUNT, out_bindings, 4*GL11_IX_MAX_ATTRIBS, unif_count, unif_bindings);

   glsl_dataflow_end_construction();
}
