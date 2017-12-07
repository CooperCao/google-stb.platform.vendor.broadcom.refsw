/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#define _USE_MATH_DEFINES
#include <math.h>

#include "gl11_shader.h"
#include "../glsl/glsl_dataflow.h"
#include "../glxx/glxx_shader_ops.h"

const GLXX_VEC4_T *texenv_src(uint32_t           src,
                              const GLXX_VEC4_T *texture,
                              const GLXX_VEC4_T *constant,
                              const GLXX_VEC4_T *primary_color,
                              const GLXX_VEC4_T *previous)
{
   switch (src)
   {
   case GL11_SRC_S: return texture;
   case GL11_SRC_K: return constant;
   case GL11_SRC_F: return primary_color;
   case GL11_SRC_P: return previous;
   default:
      unreachable();
      return NULL;
   }
}

static void texenv_op(GLXX_VEC4_T *result, uint32_t op, const GLXX_VEC4_T *color)
{
   GLXX_VEC4_T accum;
   switch (op)
   {
   case GL11_OP_C:
      accum = *color;
      break;
   case GL11_OP_CX:
      glxx_v_crep4(&accum, 1.0f);
      glxx_v_sub4 (&accum, &accum, color);
      break;
   case GL11_OP_A:
      assert(color->w != NULL);
      glxx_v_rep4(&accum, color->w);
      break;
   case GL11_OP_AX:
      glxx_v_rep4(&accum, glxx_sub(glxx_cfloat(1.0f), color->w));
      break;
   default:
      unreachable();
      break;
   }
   *result = accum;
}

static void texture_combine(GLXX_VEC4_T       *result,
                            uint32_t           combine,
                            const GLXX_VEC4_T *arg0,
                            const GLXX_VEC4_T *arg1,
                            const GLXX_VEC4_T *arg2)
{
   GLXX_VEC4_T accum;
   switch (combine)
   {
   case GL11_COMB_REP:
      accum = *arg0;
      break;
   case GL11_COMB_MOD:
      glxx_v_mul4(&accum, arg0, arg1);
      break;
   case GL11_COMB_ADD:
      glxx_v_add4(&accum, arg0, arg1);
      break;
   case GL11_COMB_ADS:
   {
      GLXX_VEC4_T tmp;
      glxx_v_add4(&accum, arg0, arg1);
      glxx_v_crep4(&tmp, 0.5f);
      glxx_v_sub4(&accum, &accum, &tmp);
      break;
   }
   case GL11_COMB_INT:
      glxx_v_interp4(&accum, arg1, arg0, arg2);
      break;
   case GL11_COMB_SUB:
      glxx_v_sub4(&accum, arg0, arg1);
      break;
   case GL11_COMB_DOT:
   case GL11_COMB_DOTA:
   {
      GLXX_VEC3_T tmp0, tmp1, tmp2;
      glxx_v_crep3(&tmp2, 0.5f);
      glxx_v_dropw(&tmp0, arg0);
      glxx_v_sub3 (&tmp0, &tmp0, &tmp2);
      glxx_v_dropw(&tmp1, arg1);
      glxx_v_sub3 (&tmp1, &tmp1, &tmp2);
      glxx_v_rep4 (&accum, glxx_mul(glxx_dot3(&tmp0, &tmp1),glxx_cfloat(4.0f)));
      break;
   }
   default:
      unreachable();
      break;
   }
   *result = accum;
}

static void texture_clamp(GLXX_VEC4_T       *result,
                          const GLXX_VEC4_T *color,
                          bool               need_clamp_alpha_below,
                          bool               need_clamp_alpha_above,
                          bool               need_clamp_rgb_below,
                          bool               need_clamp_rgb_above)
{
   Dataflow *r = color->x;
   Dataflow *g = color->y;
   Dataflow *b = color->z;
   Dataflow *a = color->w;

   if (need_clamp_alpha_below)
      a = glxx_fmax(a, glxx_cfloat(0.0f));
   if (need_clamp_alpha_above)
      a = glxx_fmin(a, glxx_cfloat(1.0f));

   if (need_clamp_rgb_below)
   {
      r = glxx_fmax(r, glxx_cfloat(0.0f));
      g = glxx_fmax(g, glxx_cfloat(0.0f));
      b = glxx_fmax(b, glxx_cfloat(0.0f));
   }
   if (need_clamp_rgb_above)
   {
      r = glxx_fmin(r, glxx_cfloat(1.0f));
      g = glxx_fmin(g, glxx_cfloat(1.0f));
      b = glxx_fmin(b, glxx_cfloat(1.0f));
   }

   glxx_v_vec4(result, r, g, b, a);
}

static bool need_clamp_below(uint32_t combine)
{
   switch (combine) {
   case GL11_COMB_REP:
   case GL11_COMB_MOD:
   case GL11_COMB_ADD:
   case GL11_COMB_INT:
      return false;
   case GL11_COMB_ADS:
   case GL11_COMB_SUB:
   case GL11_COMB_DOT:
   case GL11_COMB_DOTA:
      return true;
   default:
      unreachable();
      return 0;
   }
}

static bool need_clamp_above(uint32_t combine)
{
   switch (combine) {
   case GL11_COMB_REP:
   case GL11_COMB_MOD:
   case GL11_COMB_INT:
   case GL11_COMB_SUB:
      return false;
   case GL11_COMB_ADD:
   case GL11_COMB_ADS:
   case GL11_COMB_DOT:
   case GL11_COMB_DOTA:
      return true;
   default:
      unreachable();
      return 0;
   }
}

static void texture_lookup(GLXX_VEC4_T *result, Dataflow *image, Dataflow *sampler, const GLXX_VEC4_T *coord)
{
   Dataflow *coords = glsl_dataflow_construct_vec4(coord->x, coord->y, NULL, NULL);
   Dataflow *c[4];
   glsl_dataflow_construct_texture_lookup(c, 4, DF_TEXBITS_NONE, image, coords, NULL, NULL, NULL, sampler);

   Dataflow *l = c[0]->d.unary_op.operand;
   assert(l->flavour == DATAFLOW_TEXTURE);
   l->u.texture.required_components = 0xF;

   glxx_v_vec4(result, c[0], c[1], c[2], c[3]);
}

static void project_coordinate(GLXX_VEC4_T *result, const GLXX_VEC4_T *coord)
{
   Dataflow *rw = glxx_recip(coord->w);
   glxx_v_vec4(result, glxx_mul(coord->x, rw), glxx_mul(coord->y, rw), NULL, NULL);
}

struct u_texunit_s {
   Dataflow *image;
   Dataflow *sampler;
   GLXX_VEC4_T color;
   Dataflow *rgb_scale;
   Dataflow *a_scale;
};

static void texturing(GLXX_VEC4_T       *result,
                      const GLXX_VEC4_T *varying,
                      const uint32_t    *texunits,
                      const struct u_texunit_s *u_texunits,
                      const GLXX_VEC4_T *primary_color,
                      Dataflow          *point_x,
                      Dataflow          *point_y)
{
   GLXX_VEC4_T previous = *primary_color;

   for (int i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      uint32_t texunit = texunits[i];

      if (texunit & GL11_TEX_ENABLE)
      {
         GLXX_VEC4_T combined;
         GLXX_VEC4_T texture;
         GLXX_VEC4_T coord;
         GLXX_VEC4_T arg[3];
         GLXX_VEC4_T combined_c, combined_a;
         Dataflow *scale_c, *scale_a;
         bool need_clamp_alpha_below, need_clamp_alpha_above, need_clamp_rgb_below, need_clamp_rgb_above;
         const bool coord_replace = !!(texunit & GL11_TEX_COORDREPLACE);
         uint32_t cc, ac;

         if (coord_replace)
            glxx_v_vec4(&coord, point_x, point_y, glxx_cfloat(0.0f), glxx_cfloat(1.0f));
         else
            coord = varying[GL11_VARYING_TEX_COORD(i)];

         if (texunit & GL11_TEX_COMPLEX)
            project_coordinate(&coord, &coord);

         texture_lookup(&texture, u_texunits[i].image, u_texunits[i].sampler, &coord);

         /* Marshall arguments of the combine function. Interpolate uses 3 */
         for (int j = 0; j < 3; j++)
         {
            const GLXX_VEC4_T *constant = &u_texunits[i].color;
            const GLXX_VEC4_T *src_c;
            const GLXX_VEC4_T *src_a;
            GLXX_VEC4_T arg_c, arg_a;

            src_c = texenv_src((texunit>>(GL11_TEX_CS_S+2*j))&GL11_SRC_M,
                               &texture, constant, primary_color, &previous);
            src_a = texenv_src((texunit>>(GL11_TEX_AS_S+2*j))&GL11_SRC_M,
                               &texture, constant, primary_color, &previous);

            texenv_op(&arg_c, (texunit>>(GL11_TEX_CO_S + 2*j)) & GL11_OPC_M, src_c);
            /* This shift looks wrong but is deliberate */
            texenv_op(&arg_a, (texunit>>(GL11_TEX_AO_S + j))   & GL11_OPA_M, src_a);

            glxx_v_vec4(&arg[j], arg_c.x, arg_c.y, arg_c.z, arg_a.w);
         }

         cc = (texunit & GL11_TEX_CC_M)>>GL11_TEX_CC_S;
         ac = (texunit & GL11_TEX_AC_M)>>GL11_TEX_AC_S;

         texture_combine(&combined_c, cc, &arg[0], &arg[1], &arg[2]);

         if (cc == GL11_COMB_DOTA)
            combined_a = combined_c;
         else
            texture_combine(&combined_a, ac, &arg[0], &arg[1], &arg[2]);

         scale_c = (texunit&GL11_TEX_CSCALE) ? u_texunits[i].rgb_scale : glxx_cfloat(1.0f);
         scale_a = (texunit&GL11_TEX_ASCALE) ? u_texunits[i].a_scale   : glxx_cfloat(1.0f);

         glxx_v_vec4(&combined,
                   glxx_mul(combined_c.x, scale_c),
                   glxx_mul(combined_c.y, scale_c),
                   glxx_mul(combined_c.z, scale_c),
                   glxx_mul(combined_a.w, scale_a));

         need_clamp_alpha_below = need_clamp_below(ac) || cc == GL11_COMB_DOTA;
         need_clamp_alpha_above = need_clamp_above(ac) || cc == GL11_COMB_DOTA || (texunit&GL11_TEX_ASCALE);

         need_clamp_rgb_below = need_clamp_below(cc);
         need_clamp_rgb_above = need_clamp_above(cc) || (texunit&GL11_TEX_CSCALE);

         texture_clamp(&combined, &combined, need_clamp_alpha_below, need_clamp_alpha_above, need_clamp_rgb_below, need_clamp_rgb_above);

         previous = combined;
      }
   }

   *result = previous;
}

struct fog_s {
   Dataflow *end;
   Dataflow *scale;
   Dataflow *coeff_exp;
   Dataflow *coeff_exp2;
   GLXX_VEC3_T color;
};

static void fog(GLXX_VEC4_T *result, const GLXX_VEC4_T *varying, uint32_t fog_mode, const GLXX_VEC4_T *color, const struct fog_s *fog)
{
   GLXX_VEC3_T eyespace;
   GLXX_VEC3_T color3;

   glxx_v_dropw(&eyespace, &varying[GL11_VARYING_EYESPACE]);
   Dataflow *cc = glxx_dot3(&eyespace, &eyespace);

   Dataflow *f;
   switch (fog_mode)
   {
   case GL11_FOG_LINEAR:
      f = glxx_fmax(glxx_mul(glxx_sub(fog->end, glxx_sqrt(cc)), fog->scale), glxx_cfloat(0.0f));
      break;
   case GL11_FOG_EXP:
      f = glxx_exp2(glxx_mul(fog->coeff_exp, glxx_sqrt(cc)));
      break;
   case GL11_FOG_EXP2:
      f = glxx_exp2(glxx_mul(fog->coeff_exp2, cc));
      break;
   default:
      unreachable();
      break;
   }

   f = glxx_fmin(f, glxx_cfloat(1.0f));

   glxx_v_dropw(&color3, color);
   glxx_v_blend3(&color3, &fog->color, &color3, f);
   glxx_v_vec4(result, color3.x, color3.y, color3.z, color->w);
}

static Dataflow *get_point_aa_coverage(Dataflow *point_size, Dataflow *point_x, Dataflow *point_y)
{
   Dataflow *x = glxx_add(glxx_square(glxx_sub(point_x, glxx_cfloat(0.5))), glxx_square(glxx_sub(point_y, glxx_cfloat(0.5))));
   return glxx_mul(glxx_sub(glxx_cfloat(0.25), x), point_size);
}

static Dataflow *get_line_aa_coverage(Dataflow *line_coord, Dataflow *line_width)
{
   /* line_coord varies across line from 0.0 to 1.0 across line
      calculated at centre of pixel
      so if 0.5 < line_coord*line_width || line_coord*line_width > (line_width-0.5)
      then we will have partial coverage */

   Dataflow *half_line = glxx_mul(glxx_cfloat(0.5f), line_width);
   Dataflow *drawn_line_width = glxx_add( glxx_floor( glxx_mul( line_width, glxx_cfloat((float)M_SQRT2) )), glxx_cfloat(3.0f));

   /* mirror at 0.5 */
   Dataflow *x = glxx_abs(glxx_sub(glxx_cfloat(0.5f), line_coord));
   Dataflow *pixels = glxx_mul(drawn_line_width, x);         /* number of pixels from centre of line */

   /* fraction of pixel covered */
   return glxx_sub( glxx_cfloat(0.5f), glxx_mul(glxx_cfloat(1.0f/(float)M_SQRT2), glxx_sub(pixels, half_line)));
}

static Dataflow *perform_user_clip(uint32_t user_clip, const GLXX_VEC4_T *clip)
{
   Dataflow *x = glxx_fragment_get(DATAFLOW_FRAG_GET_X);
   Dataflow *y = glxx_fragment_get(DATAFLOW_FRAG_GET_Y);
   Dataflow *z = glxx_fragment_get(DATAFLOW_FRAG_GET_Z);

   GLXX_VEC4_T coord;
   glxx_v_vec4(&coord, x, y, z, glxx_cfloat(1.0f));
   Dataflow *scalar = glxx_mul(glxx_dot4(&coord, clip), glxx_fragment_get(DATAFLOW_FRAG_GET_W));

   assert(user_clip == GL11_UCLIP_A || user_clip == GL11_UCLIP_B);
   if (user_clip == GL11_UCLIP_A)
      return glxx_less(scalar, glxx_cfloat(0.0f));
   else
      return glxx_lequal(scalar, glxx_cfloat(0.0f));
}

static Dataflow *alpha_test(uint32_t test, Dataflow *a, Dataflow *b)
{
   switch (test)
   {
   case GL11_AFUNC_NEVER:
      return glxx_cbool(false);
   case GL11_AFUNC_LESS:
      return glxx_less(a, b);
   case GL11_AFUNC_EQUAL:
      return glxx_equal(a, b);
   case GL11_AFUNC_LEQUAL:
      return glxx_lequal(a, b);
   case GL11_AFUNC_GREATER:
      return glxx_less(b, a);
   case GL11_AFUNC_NOTEQUAL:
      return glxx_logicnot(glxx_equal(a, b));
   case GL11_AFUNC_GEQUAL:
      return glxx_lequal(b, a);
   case GL11_AFUNC_ALWAYS:
      return glxx_cbool(true);
   default:
      unreachable();
      return NULL;
   }
}

static Dataflow *byte_not(Dataflow *x) {
   return glxx_bitand( glxx_bitnot(x), glxx_cint(0xff) );
}

static void apply_logic_op(GLXX_VEC4_T *result, const GLXX_VEC4_T *col_in, uint32_t logic_op) {
   Dataflow *in[4] = { col_in->x, col_in->y, col_in->z, col_in->w };
   Dataflow *cur[4];
   Dataflow *out[4];

   /* Get the existing fb color. 0xf -> all components, 0 -> rt 0 */
   glsl_dataflow_construct_frag_get_col(cur, DF_FLOAT, 0);

   Dataflow *g = cur[0]->d.unary_op.operand;
   assert(g->flavour == DATAFLOW_FRAG_GET_COL);
   g->u.texture.required_components = 0xF;


   /* The colours come back as floats in [0,1] so convert back to int */
   for (int i=0; i<4; i++) {
      in[i] = glxx_f_to_in(glxx_mul(in[i], glxx_cfloat(255.0f)));
      cur[i] = glxx_f_to_in(glxx_mul(cur[i], glxx_cfloat(255.0f)));
   }

   for (int i=0; i<4; i++) {
      switch (logic_op) {
      case GL11_LOGIC_CLEAR:
         out[i] = glxx_cint(0); break;
      case GL11_LOGIC_AND:
         out[i] = glxx_bitand(in[i], cur[i]); break;
      case GL11_LOGIC_AND_REVERSE:
         out[i] = glxx_bitand(in[i], byte_not(cur[i])); break;
      case GL11_LOGIC_AND_INVERTED:
         out[i] = glxx_bitand(byte_not(in[i]), cur[i]); break;
      case GL11_LOGIC_NOOP:
         out[i] = cur[i]; break;
      case GL11_LOGIC_XOR:
         out[i] = glxx_bitxor(in[i], cur[i]); break;
      case GL11_LOGIC_OR:
         out[i] = glxx_bitor(in[i], cur[i]); break;
      case GL11_LOGIC_NOR:
         out[i] = byte_not(glxx_bitor(in[i], cur[i])); break;
      case GL11_LOGIC_EQUIV:
         out[i] = byte_not(glxx_bitxor(in[i], cur[i])); break;
      case GL11_LOGIC_INVERT:
         out[i] = byte_not(cur[i]); break;
      case GL11_LOGIC_OR_REVERSE:
         out[i] = glxx_bitor(in[i], byte_not(cur[i])); break;
      case GL11_LOGIC_COPY_INVERTED:
         out[i] = byte_not(in[i]); break;
      case GL11_LOGIC_OR_INVERTED:
         out[i] = glxx_bitor(byte_not(in[i]), cur[i]); break;
      case GL11_LOGIC_NAND:
         out[i] = byte_not(glxx_bitand(in[i], cur[i])); break;
      case GL11_LOGIC_SET:
         out[i] = glxx_cint(255); break;
      case GL11_LOGIC_COPY:      /* Should have been handled already */
      default:
         unreachable();
         break;
      }
   }

   for (int i=0; i<4; i++) out[i] = glxx_mul(glxx_i_to_f(out[i]), glxx_cfloat(1.0f/255.0f));
   glxx_v_vec4(result, out[0], out[1], out[2], out[3]);
}

struct builtin_unifs_s {
   struct fog_s fog_params;
   Dataflow *line_width;
   Dataflow *alpha_func_ref;
   GLXX_VEC4_T projected_clip_plane;
   struct u_texunit_s texunits[GL11_CONFIG_MAX_TEXTURE_UNITS];
};

static void new_bindings(int *id, int *bindings, int loc, int count) {
   for (int i=0; i<count; i++) bindings[*id + i] = loc + i;
   *id += count;
}

static void new_bound_v3_u(GLXX_VEC3_T *v_nodes, int *compiler_id, int *bindings, int loc) {
   glxx_v_u3(v_nodes, *compiler_id);
   new_bindings(compiler_id, bindings, loc, 3);
}

static void new_bound_v4_u(GLXX_VEC4_T *v_nodes, int *compiler_id, int *bindings, int loc) {
   glxx_v_u4(v_nodes, *compiler_id);
   new_bindings(compiler_id, bindings, loc, 4);
}

static void new_bound_u(Dataflow **node, int *compiler_id, int *bindings, int loc) {
   *node = glxx_u(*compiler_id);
   new_bindings(compiler_id, bindings, loc, 1);
}


static void construct_builtins(struct builtin_unifs_s *unif, int *bindings) {
   int count = 0;

   new_bound_u(&unif->fog_params.end, &count, bindings, GL11_STATE_OFFSET(fog.end));
   new_bound_u(&unif->fog_params.scale, &count, bindings, GL11_STATE_OFFSET(fog.scale));
   new_bound_u(&unif->fog_params.coeff_exp, &count, bindings, GL11_STATE_OFFSET(fog.coeff_exp));
   new_bound_u(&unif->fog_params.coeff_exp2, &count, bindings, GL11_STATE_OFFSET(fog.coeff_exp2));
   new_bound_v3_u(&unif->fog_params.color, &count, bindings, GL11_STATE_OFFSET(fog.color));
   new_bound_u(&unif->line_width, &count, bindings, GL11_STATE_OFFSET(line_width));
   new_bound_u(&unif->alpha_func_ref, &count, bindings, GL11_STATE_OFFSET(alpha_func.ref));
   new_bound_v4_u(&unif->projected_clip_plane, &count, bindings, GL11_STATE_OFFSET(projected_clip_plane));
   for (int i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++) {
      unif->texunits[i].image = glsl_dataflow_construct_const_image(DF_F_SAMP_IMG, count, false);
      bindings[count++] = i;
      unif->texunits[i].sampler = glsl_dataflow_construct_linkable_value(DATAFLOW_CONST_SAMPLER, DF_SAMPLER, count);
      bindings[count++] = i;
      new_bound_v4_u(&unif->texunits[i].color, &count, bindings, GL11_STATE_OFFSET(texunits[i].color));
      new_bound_u(&unif->texunits[i].rgb_scale, &count, bindings, GL11_STATE_OFFSET(texunits[i].rgb_scale));
      new_bound_u(&unif->texunits[i].a_scale, &count, bindings, GL11_STATE_OFFSET(texunits[i].alpha_scale));
   }
}

/* If you ask for POINTSMOOTH, LINESMOOTH or TEXCOORDREPLACE in the wrong
 * primitive mode you'll get funny looking results, but won't crash
 */
void gl11_get_fshader(const GL11_CACHE_KEY_T *v, IRShader **sh_out, LinkMap **lm_out)
{
   glsl_dataflow_begin_construction();

   const uint32_t fog_mode   = v->fragment & GL11_FOG_M;
   const uint32_t alpha_func = v->fragment & GL11_AFUNC_M;
   const uint32_t user_clip  = v->fragment & GL11_UCLIP_M;
   const uint32_t logic_op   = v->fragment & GL11_LOGIC_M;
   Dataflow *point_x     = glxx_fragment_get(DATAFLOW_GET_POINT_COORD_X);
   Dataflow *point_y     = glxx_fragment_get(DATAFLOW_GET_POINT_COORD_Y);
   Dataflow *line_coord  = glxx_fragment_get(DATAFLOW_GET_LINE_COORD);

   struct builtin_unifs_s unif;
   int unif_count = sizeof(struct builtin_unifs_s)/sizeof(uint32_t);
   /* TODO: Is this considered abusive? */
   int *unif_bindings = glsl_dataflow_malloc(unif_count * sizeof(int));

   Dataflow *out_nodes[DF_BLOB_FRAGMENT_COUNT];
   memset(out_nodes, 0, sizeof(Dataflow *) * DF_BLOB_FRAGMENT_COUNT);

   GLXX_VEC4_T varying[GL11_NUM_VARYINGS];
   gl11_load_inputs(varying, GL11_NUM_VARYINGS);

   construct_builtins(&unif, unif_bindings);

   /* Start generating code */

   GLXX_VEC4_T color = varying[GL11_VARYING_COLOR];
   if (v->vertex & GL11_TWOSIDE)
      glxx_v_cond4(&color, glxx_fragment_get(DATAFLOW_FRAG_GET_FF), &color, &varying[GL11_VARYING_BACK_COLOR]);

   texturing(&color, varying, v->texture, unif.texunits, &color, point_x, point_y);
   if (fog_mode)
      fog(&color, varying, fog_mode, &color, &unif.fog_params);

   Dataflow *discard = glxx_cbool(false);
   if (v->fragment & (GL11_POINTSMOOTH | GL11_LINESMOOTH))
   {
      Dataflow *coverage;
      if (v->fragment & GL11_POINTSMOOTH)
         coverage = get_point_aa_coverage(varying[GL11_VARYING_POINT_SIZE].x, point_x, point_y);
      else
         coverage = get_line_aa_coverage(line_coord, unif.line_width);

      /* Discard to ensure we only generate fragments with alpha > 0 */
      Dataflow *a_discard = glxx_lequal(coverage, glxx_cfloat(0.0f));
      coverage = glxx_clamp(coverage);

      glxx_v_vec4(&color, color.x, color.y, color.z, glxx_mul(coverage, color.w));
      discard = glxx_logicor(discard, a_discard);
   }

   //TODO: sample coverage
   if (alpha_func != GL11_AFUNC_ALWAYS) {
      Dataflow *alpha_res = alpha_test(alpha_func, color.w, unif.alpha_func_ref);
      discard = glxx_logicor(discard, glxx_logicnot(alpha_res));
   }

   if (user_clip != 0) {
      discard = glxx_logicor(discard, perform_user_clip(user_clip, &unif.projected_clip_plane));
   }

   if (logic_op != GL11_LOGIC_COPY)
      apply_logic_op(&color, &color, logic_op);

   out_nodes[DF_FNODE_R(0)] = color.x;
   out_nodes[DF_FNODE_G(0)] = color.y;
   out_nodes[DF_FNODE_B(0)] = color.z;
   out_nodes[DF_FNODE_A(0)] = color.w;
   out_nodes[DF_FNODE_DISCARD] = discard;
   varying[GL11_VARYING_COLOR] = color;

   int out_bindings[DF_BLOB_FRAGMENT_COUNT];
   *sh_out = gl11_ir_shader_from_nodes(out_nodes, DF_BLOB_FRAGMENT_COUNT, out_bindings);
   *lm_out = gl11_link_map_from_bindings(DF_BLOB_FRAGMENT_COUNT, out_bindings, 4*GL11_NUM_VARYINGS, unif_count, unif_bindings);

   glsl_dataflow_end_construction();
}
