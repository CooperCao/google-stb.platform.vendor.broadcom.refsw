/*=============================================================================
Broadcom Proprietary and Confidential. (c)2012 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file
File     :  $RCSfile: $
Revision :  $Revision: $

FILE DESCRIPTION
Translate a Dataflow graph to Backflow. This involves inserting hw-specific
constructs as well as the parts of the shader that depend on things other than
shader source.
=============================================================================*/
#include "glsl_common.h"
#include "glsl_backend_uniforms.h"
#include "glsl_backflow.h"
#include "glsl_backflow_visitor.h"
#include "glsl_backend.h"
#include "glsl_dataflow.h"
#include "glsl_dataflow_visitor.h"
#include "glsl_tex_params.h"

#include "glsl_qbe_fragment.h"
#include "glsl_sched_node_helpers.h"

#include "../glxx/glxx_int_config.h"
#include "../glxx/glxx_shader_cache.h"

#include "libs/util/gfx_util/gfx_util.h"

#include <assert.h>

// Helpers
static Backflow *tr_if(Backflow *c, Backflow *t, Backflow *f)
{
   return create_node(BACKFLOW_MOV, UNPACK_NONE, COND_IFFLAG, c, t, NULL, f);
}

static Backflow *fdiv(Backflow *l, Backflow *r) {
   return tr_binop(BACKFLOW_MUL, l, tr_mov_to_reg(REG_MAGIC_RECIP, r));
}

// If numerator is zero return zero otherwise divide.
static Backflow *tr_blend_div(Backflow *n, Backflow *d)
{
   Backflow *eq0 = tr_uop_cond(BACKFLOW_FMOV, SETF_PUSHZ, NULL, n);
   return tr_if(eq0, n /* 0 */, fdiv(n, d));
}

// If first arg is zero return zero otherwise multiply
static Backflow *blend_mul(Backflow *a, Backflow *b)
{
   Backflow *eq0 = tr_uop_cond(BACKFLOW_FMOV, SETF_PUSHZ, NULL, a);
   return tr_if(eq0, a /* 0 */, mul(a, b));
}

static void adv_blend_copy_rgb(Backflow *rgb[3], Backflow *rgba[4])
{
   for (uint32_t i = 0; i < 3; ++i)
      rgb[i] = rgba[i] != NULL ?  rgba[i] : tr_cfloat(0.0f);
}

//
// Advanced blend functions
//
static Backflow *adv_blend_multiply(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // CsCd
   return mul(cs, cd);
}

static Backflow *adv_blend_screen(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // (CsAd + CdAs) - CsCd
   return sub(add(mul(cs, ad),
                  mul(cd, as)),
              mul(cs, cd));
}

static Backflow *adv_blend_overlay_or_hardlight(
   Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad, Backflow *cond)
{
   // 2CsCd                 , if cond
   // AsAd - 2(As-Cs)(Ad-Cd), otherwise
   Backflow *t =
      mul(tr_cfloat(2.0f), mul(cs, cd));

   Backflow *f =
      sub(mul(as, ad),
          mul(tr_cfloat(2.0f),
              mul(sub(as, cs),
                  sub(ad, cd))));

   return tr_if(cond, t, f);
}

static Backflow *adv_blend_overlay(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   Backflow *c = tr_binop_push(BACKFLOW_FCMP, SETF_PUSHC, cd, mul(tr_cfloat(0.5f), ad));
   return adv_blend_overlay_or_hardlight(cs, cd, as, ad, asad, c);
}

static Backflow *adv_blend_hardlight(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   Backflow *c = tr_binop_push(BACKFLOW_FCMP, SETF_PUSHC, cs, mul(tr_cfloat(0.5f), as));
   return adv_blend_overlay_or_hardlight(cs, cd, as, ad, asad, c);
}

static Backflow *adv_blend_darken(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // min(CsAd, CdAs)
   return imin(mul(cs, ad), mul(cd, as));
}

static Backflow *adv_blend_lighten(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // max(CsAd, CdAs)
   return imax(mul(cs, ad), mul(cd, as));
}

static Backflow *adv_blend_colordodge(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // 0                            , if Cd <= 0
   // min(AsAd, AsAsCd / (As - Cs)), if Cd > 0 && Cs < As
   // AsAd                         , if Cd > 0 && Cs >= As
   Backflow *expr = tr_blend_div(mul(mul(as, as), cd), sub(as, cs));

   Backflow *cd_lte0 = tr_uop_cond(BACKFLOW_FMOV, SETF_PUSHC, NULL, cd);
   Backflow *cs_gte1 = tr_binop_push(BACKFLOW_FCMP, SETF_PUSHC, as, cs);

   return tr_if(cd_lte0, tr_cfloat(0.0f),
                         tr_if(cs_gte1, asad, imin(asad, expr)));
}

static Backflow *adv_blend_colorburn(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // AsAd                                , if Cd >= Ad
   // AsAd - min(AsAd, AsAs(Ad - Cd) / Cs), if Cd < Ad && Cs > 0
   // 0                                   , if Cd < Ad && Cs <= 0
   Backflow *expr = tr_blend_div(mul(mul(as, as), sub(ad, cd)), cs);

   Backflow *cd_gte1 = tr_binop_push(BACKFLOW_FCMP, SETF_PUSHC, ad, cd);
   Backflow *cs_lte0 = tr_uop_cond(BACKFLOW_FMOV, SETF_PUSHC, NULL, cs);

   return tr_if(cd_gte1, asad,
                         tr_if(cs_lte0, tr_cfloat(0.0f), sub(asad, imin(asad, expr))));
}

static Backflow *adv_blend_softlight(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // D(1 + (2S-1)(1-D))          , S <= 0.5
   // D(1 + (2S-1)((16D-12)D + 3)), S > 0.5 && D <= 0.25
   // D + (2S-1)(sqrt(D)-D)       , S > 0.5 && D >  0.25
   Backflow *d  = tr_blend_div(cd, ad);
   Backflow *s  = tr_blend_div(cs, as); // TODO: this divide could be eliminated with some algebra,
                                        // but is it worth it given all the other ops?
   Backflow *one    = tr_cfloat(1.0f);
   Backflow *s2     = mul(s,   tr_cfloat(2.0f)); // 2S
   Backflow *s2_one = sub(s2,  one);             // 2S - 1
   Backflow *one_d  = sub(one, d  );             // 1 - D

   Backflow *case1 = mul(d, add(one, mul(s2_one, one_d)));
   Backflow *expr2 = add(mul(sub(mul(tr_cfloat(16.0f), d), tr_cfloat(12.0f)), d), tr_cfloat(3.0f));
   Backflow *case2 = mul(d, add(one, mul(s2_one, expr2)));
   Backflow *case3 = add(d, mul(s2_one, sub(tr_sqrt(d), d)));

   Backflow *s_lte_half = tr_binop_push(BACKFLOW_FCMP, SETF_PUSHC, s, tr_cfloat(0.5f));
   Backflow *d_lte_qter = tr_binop_push(BACKFLOW_FCMP, SETF_PUSHC, d, tr_cfloat(0.25f));

   return mul(tr_if(s_lte_half, case1, tr_if(d_lte_qter, case2, case3)), asad);
}

static Backflow *adv_blend_difference(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // abs(CdAs - CsAd)
   return absf(sub(mul(cd, as), mul(cs, ad)));
}

static Backflow *adv_blend_exclusion(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // (CsAd + CdAs) - 2CsCd
   return sub(add(mul(cs, ad), mul(cd, as)),
              mul(tr_cfloat(2.0f), mul(cs, cd)));
}

static Backflow *minv3(Backflow *c[3])
{
   return imin(c[0], imin(c[1], c[2]));
}

static Backflow *maxv3(Backflow *c[3])
{
   return imax(c[0], imax(c[1], c[2]));
}

static Backflow *adv_blend_sat(Backflow *c[3])
{
   return sub(maxv3(c), minv3(c));
}

// float lum(vec3 c)  // Convert RGB to luminance
// {
//   return dot(c, vec3(0.30, 0.59, 0.11));
// }
static Backflow *adv_blend_lum(Backflow *c[3])
{
   Backflow *r = mul(c[0], tr_cfloat(0.30f));
   Backflow *g = mul(c[1], tr_cfloat(0.59f));
   Backflow *b = mul(c[2], tr_cfloat(0.11f));

   return add(r, add(g, b));
}

// vec3 SetLum(vec3 cbase, vec3 clum)
// {
//   float lbase = lum(cbase);
//   float llum  = lum(clum);
//   float ldiff = llum - lbase;
//   vec3  color = cbase + vec3(ldiff);
//
//   if (min(color) < 0.0)
//     return llum + ((color - llum) * llum) /
//                    (llum - min(color));
//
//   if (max(color) > 1.0)
//     return llum + ((color - llum) * (1 - llum)) /
//                    (max(color) - llum);
//
//   return color;
// }
static void adv_blend_set_lum(Backflow *res[3], Backflow *cbase[3], Backflow *clum[3])
{
   Backflow *lbase = adv_blend_lum(cbase);
   Backflow *llum  = adv_blend_lum(clum);
   Backflow *ldiff = sub(llum, lbase);

   Backflow *color[3];

   for (uint32_t i = 0; i < 3; ++i)
      color[i] = add(cbase[i], ldiff);

   Backflow *max_color = maxv3(color);
   Backflow *min_color = minv3(color);

   Backflow *min_color_lt0 = tr_uop_cond(BACKFLOW_FMOV, SETF_PUSHN, NULL, min_color);
   Backflow *max_color_gt1 = tr_binop_push(BACKFLOW_FCMP, SETF_PUSHN, tr_cfloat(1.0f), max_color);

   Backflow *color_minus_llum[3];

   for (uint32_t i = 0; i < 3; ++i)
      color_minus_llum[i] = sub(color[i], llum);

   Backflow *case1[3];
   Backflow *case2[3];

   Backflow *one_minus_llum = sub(tr_cfloat(1.0f), llum);

   Backflow *recip_sub_llum_min_color = recip(sub(llum, min_color));
   Backflow *recip_sub_max_color_llum = recip(sub(max_color, llum));

   for (uint32_t i = 0; i < 3; ++i)
   {
      case1[i] = add(llum, mul(mul(color_minus_llum[i], llum),           recip_sub_llum_min_color));
      case2[i] = add(llum, mul(mul(color_minus_llum[i], one_minus_llum), recip_sub_max_color_llum));
   }

   for (uint32_t i = 0; i < 3; ++i)
      res[i] = tr_if(min_color_lt0, case1[i], tr_if(max_color_gt1, case2[i], color[i]));
}

// vec3 SetSatLum(vec3 cbase, vec3 csat, vec3 clum)
// {
//   float minbase = min(cbase);
//   float sbase   = sat(cbase);
//   float ssat    = sat(csat);
//   vec3  color;
//
//   if (sbase > 0)
//     color = (cbase - minbase) * ssat / sbase;
//   else
//     color = vec3(0.0);
//
//   return SetLum(color, clum);
// }
static void adv_blend_set_sat_lum(Backflow *res[3], Backflow *cbase[3], Backflow *csat[3], Backflow *clum[3])
{
   Backflow *minbase = minv3(cbase);
   Backflow *sbase   = adv_blend_sat(cbase);
   Backflow *ssat    = adv_blend_sat(csat);

   Backflow *color[3];

   Backflow *sbase_gt0 = tr_binop_push(BACKFLOW_FCMP, SETF_PUSHN, tr_cfloat(0.0f), sbase);
   Backflow *ssat_over_sbase = tr_if(sbase_gt0, fdiv(ssat, sbase), tr_cfloat(0.0f));

   for (uint32_t i = 0; i < 3; ++i)
      color[i] = mul(sub(cbase[i], minbase), ssat_over_sbase);

   adv_blend_set_lum(res, color, clum);
}

static void adv_blend_hsl_hue(Backflow *res[3], Backflow *cs[3], Backflow *cd[3], Backflow *as, Backflow *ad)
{
   adv_blend_set_sat_lum(res, cs, cd, cd);
}

static void adv_blend_hsl_saturation(Backflow *res[3], Backflow *cs[3], Backflow *cd[3], Backflow *as, Backflow *ad)
{
   adv_blend_set_sat_lum(res, cd, cs, cd);
}

static void adv_blend_hsl_color(Backflow *res[3], Backflow *cs[3], Backflow *cd[3], Backflow *as, Backflow *ad)
{
   adv_blend_set_lum(res, cs, cd);
}

static void adv_blend_hsl_luminosity(Backflow *res[3], Backflow *cs[3], Backflow *cd[3], Backflow *as, Backflow *ad)
{
   adv_blend_set_lum(res, cd, cs);
}

typedef Backflow *(*ADV_BLEND_FN)(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad);

ADV_BLEND_FN adv_blend_fn(GLenum mode)
{
   switch (mode)
   {
   case GLXX_ADV_BLEND_MULTIPLY:   return adv_blend_multiply;
   case GLXX_ADV_BLEND_SCREEN:     return adv_blend_screen;
   case GLXX_ADV_BLEND_OVERLAY:    return adv_blend_overlay;
   case GLXX_ADV_BLEND_DARKEN:     return adv_blend_darken;
   case GLXX_ADV_BLEND_LIGHTEN:    return adv_blend_lighten;
   case GLXX_ADV_BLEND_COLORDODGE: return adv_blend_colordodge;
   case GLXX_ADV_BLEND_COLORBURN:  return adv_blend_colorburn;
   case GLXX_ADV_BLEND_HARDLIGHT:  return adv_blend_hardlight;
   case GLXX_ADV_BLEND_SOFTLIGHT:  return adv_blend_softlight;
   case GLXX_ADV_BLEND_DIFFERENCE: return adv_blend_difference;
   case GLXX_ADV_BLEND_EXCLUSION:  return adv_blend_exclusion;
   default: unreachable();
   }
}

typedef void (*ADV_BLEND_HSL_FN)(Backflow *res[3], Backflow *cs[3], Backflow *cd[3], Backflow *as, Backflow *ad);

ADV_BLEND_HSL_FN adv_blend_hsl_fn(GLenum mode)
{
   switch (mode)
   {
   case GLXX_ADV_BLEND_HSL_HUE:        return adv_blend_hsl_hue;
   case GLXX_ADV_BLEND_HSL_SATURATION: return adv_blend_hsl_saturation;
   case GLXX_ADV_BLEND_HSL_COLOR:      return adv_blend_hsl_color;
   case GLXX_ADV_BLEND_HSL_LUMINOSITY: return adv_blend_hsl_luminosity;
   default: unreachable();
   }
}

void adv_blend(Backflow *res[3], GLenum mode, Backflow *cs[3], Backflow *cd[3], Backflow *as, Backflow *ad, Backflow *asad)
{
   ADV_BLEND_FN  blend_fn = adv_blend_fn(mode);

   for (uint32_t i = 0; i < 3; ++i)
      res[i] = blend_fn(cs[i], cd[i], as, ad, asad);
}

void adv_blend_hsl(Backflow *res[3], GLenum mode, Backflow *cs[3], Backflow *cd[3], Backflow *as, Backflow *ad, Backflow *asad)
{
   Backflow *bcs[3];
   Backflow *bcd[3];

   Backflow *as_rcp = recip(as);
   Backflow *ad_rcp = recip(ad);

   for (uint32_t i = 0; i < 3; ++i)
   {
      bcs[i] = blend_mul(cs[i], as_rcp);
      bcd[i] = blend_mul(cd[i], ad_rcp);
   }

   adv_blend_hsl_fn(mode)(res, bcs, bcd, as, ad);

   for (uint32_t i = 0; i < 3; ++i)
      res[i] = mul(res[i], asad);
}

static bool is_hsl(uint32_t mode)
{
   switch (mode)
   {
   case GLXX_ADV_BLEND_HSL_HUE        :
   case GLXX_ADV_BLEND_HSL_SATURATION :
   case GLXX_ADV_BLEND_HSL_COLOR      :
   case GLXX_ADV_BLEND_HSL_LUMINOSITY : return true;
   default                            : return false;
   }
}

static Backflow *adv_blend_combine_alpha(Backflow *as, Backflow *ad, Backflow *asad)
{
   // Ad + As(1 - Ad) = Ad + As - AsAd
   return add(ad, sub(as, asad));
}

// Advanced blend selection
//
// src is the fragment
// dst is the framebuffer value
// res is where to stick the blend result
void adv_blend_blend(Backflow *res[4], Backflow *src[4], Backflow *dst[4], uint32_t mode)
{
   Backflow *as   = src[3] != NULL ? src[3] : tr_cfloat(1.0f);
   Backflow *ad   = dst[3] != NULL ? dst[3] : tr_cfloat(1.0f);
   Backflow *asad = mul(as, ad);

   Backflow *cs[3];
   adv_blend_copy_rgb(cs, src);

   Backflow *cd[3];
   adv_blend_copy_rgb(cd, dst);

   if (!is_hsl(mode))
      adv_blend(res, mode, cs, cd, as, ad, asad);
   else
      adv_blend_hsl(res, mode, cs, cd, as, ad, asad);

   Backflow *one_minus_ad = sub(tr_cfloat(1.0f), ad);
   Backflow *one_minus_as = sub(tr_cfloat(1.0f), as);

   for (uint32_t i = 0; i < 3; ++i)
   {
      res[i] = add(res[i], mul(cs[i], one_minus_ad)); // + Cs(1 - Ad)
      res[i] = add(res[i], mul(cd[i], one_minus_as)); // + Cd(1 - As)
   }

   res[3] = adv_blend_combine_alpha(as, ad, asad);
}

void adv_blend_read_tlb(Backflow *res[4][4], const FragmentBackendState *s, SchedBlock *block, int rt)
{
   int type = s->rt[rt].type;
   assert(type == GLXX_FB_F16 || type == GLXX_FB_F32);

   Backflow *node[16];
   Backflow *first, *last;
   tr_read_tlb(s->ms, rt, type, 0xf, node, &first, &last);

   if (block->last_tlb_read)
      glsl_iodep(first, block->last_tlb_read);
   else
      block->first_tlb_read = first;
   block->last_tlb_read = last;

   for (int sm = 0; sm < (s->ms ? 4 : 1); sm++)
      for (int c = 0; c < 4; c++)
        res[sm][c] = node[4*sm + c];
}
