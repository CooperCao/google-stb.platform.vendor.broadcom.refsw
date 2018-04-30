/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
#include "glsl_backend_cfg.h"

#include "libs/util/gfx_util/gfx_util.h"

#include <assert.h>

// If numerator is zero return zero otherwise divide.
static Backflow *tr_blend_div(Backflow *n, Backflow *d)
{
   Backflow *eq0 = tr_uop_push(V3D_QPU_OP_FMOV, SETF_PUSHZ, n);
   return tr_cond(eq0, n /* 0 */, fdiv(n, d), false);
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
   // (CsAd + CdAs) - CsCd ==> Cs + Cd - CsCd
   return sub(add(cs, cd), mul(cs, cd));
}

static Backflow *adv_blend_overlay_or_hardlight(
   Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad, Backflow *cond)
{
   // 2CsCd                 , if cond
   // AsAd - 2(As-Cs)(Ad-Cd), otherwise
   Backflow *t = mul(tr_cfloat(2.0f), mul(cs, cd));
   Backflow *f = sub(mul(as, ad),
                     mul(tr_cfloat(2.0f),
                         mul(sub(as, cs),
                             sub(ad, cd))));

   return tr_cond(cond, t, f, false);
}

static Backflow *adv_blend_overlay(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   Backflow *c = tr_binop_push(V3D_QPU_OP_FCMP, SETF_PUSHC, cd, mul(tr_cfloat(0.5f), ad));
   return adv_blend_overlay_or_hardlight(cs, cd, as, ad, asad, c);
}

static Backflow *adv_blend_hardlight(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   Backflow *c = tr_binop_push(V3D_QPU_OP_FCMP, SETF_PUSHC, cs, mul(tr_cfloat(0.5f), as));
   return adv_blend_overlay_or_hardlight(cs, cd, as, ad, asad, c);
}

static Backflow *adv_blend_darken(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // min(CsAd, CdAs)
   // Including the + Cd(1-As) + Cs(1-Ad) term ==> Cs + Cd - max(CsAd, CdAs)
   return sub(add(cs,cd), imax(mul(cs, ad), mul(cd, as)));
}

static Backflow *adv_blend_lighten(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // max(CsAd, CdAs)
   // Including the + Cd(1-As) + Cs(1-Ad) term ==> Cs + Cd - min(CsAd, CdAs)
   return sub(add(cs,cd), imin(mul(cs, ad), mul(cd, as)));
}

static Backflow *adv_blend_colordodge(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // 0                            , if Cd <= 0
   // min(AsAd, AsAsCd / (As - Cs)), if Cd > 0 && Cs < As
   // AsAd                         , if Cd > 0 && Cs >= As
   Backflow *expr = fdiv(mul(mul(as, as), cd), sub(as, cs));

   Backflow *cd_lte0 = tr_uop_push(V3D_QPU_OP_FMOV, SETF_PUSHC, cd);
   Backflow *cs_gte1 = tr_binop_push(V3D_QPU_OP_FCMP, SETF_PUSHC, as, cs);

   return tr_cond(cd_lte0, tr_cfloat(0.0f),
                           tr_cond(cs_gte1, asad, imin(asad, expr), false), false);
}

static Backflow *adv_blend_colorburn(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // AsAd                                , if Cd >= Ad
   // AsAd - min(AsAd, AsAs(Ad - Cd) / Cs), if Cd < Ad && Cs > 0
   // 0                                   , if Cd < Ad && Cs <= 0
   Backflow *expr = fdiv(mul(mul(as, as), sub(ad, cd)), cs);

   Backflow *cd_gte1 = tr_binop_push(V3D_QPU_OP_FCMP, SETF_PUSHC, ad, cd);
   Backflow *cs_lte0 = tr_uop_push(V3D_QPU_OP_FMOV, SETF_PUSHC, cs);

   return tr_cond(cd_gte1, asad,
                           tr_cond(cs_lte0, tr_cfloat(0.0f), sub(asad, imin(asad, expr)), false), false);
}

static Backflow *adv_blend_softlight(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // Cd (As + (2Cs - As) E), Where E is given by:
   //       (1-D)          , S <= 0.5
   //       ((16D-12)D + 3), S > 0.5 && D <= 0.25
   //       (rsqrt(D)-1)   , S > 0.5 && D >  0.25
   Backflow *d = tr_blend_div(cd, ad);
   Backflow *one = tr_cfloat(1.0f);

   Backflow *e1 = sub(one, d);
   Backflow *e2 = add(mul(sub(mul(tr_cfloat(16.0f), d), tr_cfloat(12.0f)), d), tr_cfloat(3.0f));
   Backflow *e3 = sub(rsqrt(d), one);

   Backflow *s_lte_half = tr_binop_push(V3D_QPU_OP_FCMP, SETF_PUSHC, cs, mul(tr_cfloat(0.5f), as));
   Backflow *d_lte_qter = tr_binop_push(V3D_QPU_OP_FCMP, SETF_PUSHC, d, tr_cfloat(0.25f));
   Backflow *e = tr_cond(s_lte_half, e1, tr_cond(d_lte_qter, e2, e3, false), false);

   Backflow *s2_one = sub(mul(cs, tr_cfloat(2.0f)), as);
   return mul(cd, add(as, mul(s2_one, e)));
}

static Backflow *adv_blend_difference(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // abs(CdAs - CsAd)
   // Including the + Cd(1-As) + Cs(1-Ad) term ==> Cs + Cd - 2 min(CsAd, CdAs)
   return sub(add(cs, cd), mul(tr_cfloat(2.0f), imin(mul(cs, ad), mul(cd, as))));
}

static Backflow *adv_blend_exclusion(Backflow *cs, Backflow *cd, Backflow *as, Backflow *ad, Backflow *asad)
{
   // (CsAd + CdAs) - 2CsCd
   // Including the + Cd(1-As) + Cs(1-Ad) term ==> Cs + Cd - 2CsCd
   return sub(add(cs, cd), mul(tr_cfloat(2.0f), mul(cs, cd)));
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

   Backflow *min_color_lt0 = tr_uop_push(V3D_QPU_OP_FMOV, SETF_PUSHN, min_color);
   Backflow *max_color_gt1 = tr_binop_push(V3D_QPU_OP_FCMP, SETF_PUSHN, tr_cfloat(1.0f), max_color);

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
      res[i] = tr_cond(min_color_lt0, case1[i], tr_cond(max_color_gt1, case2[i], color[i], false), false);
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

   Backflow *sbase_gt0 = tr_binop_push(V3D_QPU_OP_FCMP, SETF_PUSHN, tr_cfloat(0.0f), sbase);
   Backflow *ssat_over_sbase = tr_cond(sbase_gt0, fdiv(ssat, sbase), tr_cfloat(0.0f), false);

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

ADV_BLEND_FN adv_blend_fn(uint32_t mode)
{
   switch (mode)
   {
   case GLSL_ADV_BLEND_MULTIPLY:   return adv_blend_multiply;
   case GLSL_ADV_BLEND_SCREEN:     return adv_blend_screen;
   case GLSL_ADV_BLEND_OVERLAY:    return adv_blend_overlay;
   case GLSL_ADV_BLEND_DARKEN:     return adv_blend_darken;
   case GLSL_ADV_BLEND_LIGHTEN:    return adv_blend_lighten;
   case GLSL_ADV_BLEND_COLORDODGE: return adv_blend_colordodge;
   case GLSL_ADV_BLEND_COLORBURN:  return adv_blend_colorburn;
   case GLSL_ADV_BLEND_HARDLIGHT:  return adv_blend_hardlight;
   case GLSL_ADV_BLEND_SOFTLIGHT:  return adv_blend_softlight;
   case GLSL_ADV_BLEND_DIFFERENCE: return adv_blend_difference;
   case GLSL_ADV_BLEND_EXCLUSION:  return adv_blend_exclusion;
   default: unreachable();
   }
}

typedef void (*ADV_BLEND_HSL_FN)(Backflow *res[3], Backflow *cs[3], Backflow *cd[3], Backflow *as, Backflow *ad);

ADV_BLEND_HSL_FN adv_blend_hsl_fn(uint32_t mode)
{
   switch (mode)
   {
   case GLSL_ADV_BLEND_HSL_HUE:        return adv_blend_hsl_hue;
   case GLSL_ADV_BLEND_HSL_SATURATION: return adv_blend_hsl_saturation;
   case GLSL_ADV_BLEND_HSL_COLOR:      return adv_blend_hsl_color;
   case GLSL_ADV_BLEND_HSL_LUMINOSITY: return adv_blend_hsl_luminosity;
   default:                            return NULL;
   }
}

void adv_blend(Backflow *res[3], uint32_t mode, Backflow *cs[3], Backflow *cd[3], Backflow *as, Backflow *ad, Backflow *asad)
{
   ADV_BLEND_FN blend_fn = adv_blend_fn(mode);

   for (uint32_t i = 0; i < 3; ++i)
      res[i] = blend_fn(cs[i], cd[i], as, ad, asad);
}

void adv_blend_hsl(Backflow *res[3], uint32_t mode, Backflow *cs[3], Backflow *cd[3], Backflow *as, Backflow *ad, Backflow *asad)
{
   Backflow *bcs[3];
   Backflow *bcd[3];

   Backflow *as_rcp  = recip(as);
   Backflow *as_zero = tr_uop_push(V3D_QPU_OP_FMOV, SETF_PUSHZ, as);
   Backflow *ad_rcp  = recip(ad);
   Backflow *ad_zero = tr_uop_push(V3D_QPU_OP_FMOV, SETF_PUSHZ, ad);

   for (uint32_t i = 0; i < 3; ++i) {
      bcs[i] = tr_cond(as_zero, as, mul(cs[i], as_rcp), false);
      bcd[i] = tr_cond(ad_zero, ad, mul(cd[i], ad_rcp), false);
   }

   adv_blend_hsl_fn(mode)(res, bcs, bcd, as, ad);

   for (uint32_t i = 0; i < 3; ++i)
      res[i] = mul(res[i], asad);
}

static bool is_hsl(uint32_t mode) { return adv_blend_hsl_fn(mode) != NULL; }

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

   // These terms have been included in some of the equations. Add them for the rest here.
   if (mode != GLSL_ADV_BLEND_EXCLUSION  && mode != GLSL_ADV_BLEND_SCREEN &&
       mode != GLSL_ADV_BLEND_DIFFERENCE && mode != GLSL_ADV_BLEND_DARKEN &&
       mode != GLSL_ADV_BLEND_LIGHTEN)
   {
      Backflow *one_minus_ad = sub(tr_cfloat(1.0f), ad);
      Backflow *one_minus_as = sub(tr_cfloat(1.0f), as);

      for (uint32_t i = 0; i < 3; ++i) {
         res[i] = add(res[i], mul(cs[i], one_minus_ad)); // + Cs(1 - Ad)
         res[i] = add(res[i], mul(cd[i], one_minus_as)); // + Cd(1 - As)
      }
   }

   res[3] = adv_blend_combine_alpha(as, ad, asad);
}

void adv_blend_read_tlb(Backflow *res[4][4], const FragmentBackendState *s, SchedBlock *block, int rt)
{
   Backflow *node[16];
   Backflow *first, *last;
   tr_read_tlb(s->ms, rt, s->rt[rt].is_16, s->rt[rt].is_int, 0xf, node, &first, &last);

   if (block->last_tlb_read)
      glsl_iodep(first, block->last_tlb_read);
   else
      block->first_tlb_read = first;
   block->last_tlb_read = last;

   for (int sm = 0; sm < (s->ms ? 4 : 1); sm++)
      for (int c = 0; c < 4; c++)
        res[sm][c] = node[4*sm + c];
}
