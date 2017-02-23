/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#include "v3d_vpm.h"
#include "v3d_vpm_alloc.h"
#include "v3d_cl.h"
#include "v3d_limits.h"
#include "v3d_ver.h"
#include "libs/util/gfx_util/gfx_util.h"
#include <string.h>

#if V3D_VER_AT_LEAST(4,0,2,0)

static inline uint32_t v3d_vpm_words_to_sectors(uint32_t num_words, v3d_cl_vpm_pack_t pk)
{
   assert(1 << pk == V3D_VPAR/v3d_cl_vpm_pack_to_width(pk));
   return gfx_udiv_round_up(num_words, V3D_VPM_ROWS_PER_BLOCK << pk);
}

static inline uint32_t v3d_vpm_geom_words_to_sectors(uint32_t num_words, v3d_cl_geom_output_pack_t pk)
{
   return gfx_udiv_round_up(num_words, V3D_VPM_ROWS_PER_BLOCK * V3D_VPAR / v3d_cl_geom_pack_to_width(pk));
}

static inline unsigned Pw_from_Pd(unsigned Pd)
{
   unsigned Pw = 16 / gfx_next_power_of_2(Pd);
   Pw = Pw < 4 ? 1 : Pw;
   return Pw;
}

static inline v3d_cl_tcs_batch_flush_mode_t get_Cfe(v3d_cl_tcs_batch_flush_mode_t Cf, unsigned Cp, unsigned Cn, unsigned Cw)
{
   if (Cp == 1 || Cf == V3D_CL_TCS_BATCH_FLUSH_MODE_SINGLE_PATCH)
      return V3D_CL_TCS_BATCH_FLUSH_MODE_SINGLE_PATCH;

   if (gfx_is_power_of_2(Cn))
      return Cn < Cw ? V3D_CL_TCS_BATCH_FLUSH_MODE_PACKED_COMPLETE_PATCHES : V3D_CL_TCS_BATCH_FLUSH_MODE_SINGLE_PATCH;

   if (Cn*Cp <= Cw)
      return V3D_CL_TCS_BATCH_FLUSH_MODE_PACKED_COMPLETE_PATCHES;

   return Cf;
}

void v3d_vpm_default_cfg_t(v3d_vpm_cfg_t* t)
{
   memset(t, 0, sizeof(*t));
   t->max_patches_per_tcs_batch = 1;
   t->max_patches_per_tes_batch = 1;
   for (unsigned br = 0; br != 2; br++)
   {
      t->per_patch_depth[br] = 1;
      t->min_tcs_segs[br] = 1;
      t->min_per_patch_segs[br] = 1;
      t->max_tcs_segs_per_tes_batch[br] = 1;
      t->min_tes_segs[br] = 1;
   }
}

void v3d_vpm_default_cfg_g(v3d_vpm_cfg_g* g)
{
   memset(g, 0, sizeof(*g));
   g->min_gs_segs[0] = 1;
   g->min_gs_segs[1] = 1;
}

void v3d_vpm_cfg_validate_br(
   v3d_vpm_cfg_v const* v,
   v3d_vpm_cfg_t const* t,
   v3d_vpm_cfg_g const* g,
   unsigned vpm_size_in_sectors,
   unsigned max_input_vertices,
   unsigned tes_patch_vertices,
   unsigned br)
{
   union
   {
      v3d_vpm_cfg_t t;
      v3d_vpm_cfg_g g;
   } dummy;
   if (!t || !g)
      memset(&dummy, 0, sizeof(dummy));

   v3d_vpm_cfg_t const* pt = t ? t : &dummy.t;
   v3d_vpm_cfg_g const* pg = g ? g : &dummy.g;

   unsigned Ad = v->input_size[br].sectors;
   unsigned As = v->input_size[br].min_req;
   unsigned Vn = max_input_vertices;
   unsigned Vd = v->output_size[br].sectors;
   unsigned Ve = v->output_size[br].min_extra_req;
   unsigned Vc = v->vcm_cache_size[br];
   unsigned Pd = pt->per_patch_depth[br];
   unsigned Ps = pt->min_per_patch_segs[br];
   unsigned Cn = tes_patch_vertices;
   unsigned Cw = v3d_cl_vpm_pack_to_width(pt->tcs_output[br].pack);
   unsigned Cd = pt->tcs_output[br].size_sectors;
   unsigned CV = pt->max_extra_vert_segs_per_tcs_batch[br];
   unsigned Cp = pt->max_patches_per_tcs_batch;
   unsigned Cf = pt->tcs_batch_flush;
   unsigned Cs = pt->min_tcs_segs[br];
   unsigned Ed = pt->tes_output[br].size_sectors;
   unsigned EV = pt->max_extra_vert_segs_per_tes_batch[br];
   unsigned EC = pt->max_tcs_segs_per_tes_batch[br];
   unsigned Ep = pt->max_patches_per_tes_batch;
   unsigned Es = pt->min_tes_segs[br];
   unsigned Gd = pg->geom_output[br].size_sectors;
   unsigned GV = pg->max_extra_vert_segs_per_gs_batch[br];
   unsigned Gs = pg->min_gs_segs[br];

   assert(As <= 4);
   assert(Ad < 16);
   assert(Vn-1 < 32);
   assert(Vd-1 < 16);
   assert(Vc-1 < 4);
   assert(Ve < 4);
   if (t)
   {
      assert(Pd-1 < 16);
      assert(Ps-1 < 4);
      assert(Cn-1 < 32);
      assert(Cw == 4 || Cw == 8 || Cw == 16);
      assert(Cd-1 < 16);
      assert(CV < 4);
      assert(Cp-1 < 16);
      assert(Cs-1 < 8);
      assert(Ed-1 < 16);
      assert(EV < 4);
      assert(EC-1 < 8);
      assert(Ep-1 < 16);
      assert(Es-1 < 8);
   }
   if (g)
   {
      assert(Gd-1 < 16);
      assert(GV < 4);
      assert(Gs-1 < 8);
   }

   unsigned num_sectors = As*Ad + (Vc + Ve)*Vd + (t ? Ps + Cs*Cd + Es*Ed : 0) + (g ? Gs*Gd : 0);
   assert(num_sectors <= vpm_size_in_sectors);
   assert(Vc >= gfx_udiv_round_up(Vn, V3D_VPAR));

   unsigned GVe = GV + (!V3D_HAS_RELAX_VPM_LIMS ? (t || Vn <= 16 ? 1u : 2u) : 0);
   if (t)
   {
      unsigned Cfe = get_Cfe(Cf, Cp, Cn, Cw);
      unsigned CVe = CV + (V3D_HAS_RELAX_VPM_LIMS || Cfe == V3D_CL_TCS_BATCH_FLUSH_MODE_SINGLE_PATCH ? 0u : (Vn <= 16 ? 1u : 2u));

      assert(Cw >= gfx_umin(Cn, 16u));
      assert(EC >= (Cfe != V3D_CL_TCS_BATCH_FLUSH_MODE_FULLY_PACKED ? gfx_udiv_round_up(Cn, Cw) : gfx_umin(Cn, 2u)));
      assert(Ve >= EV + gfx_umax(CVe, !V3D_HAS_RELAX_VPM_LIMS ? 1u : 0u));
      assert(Ps >= gfx_udiv_round_up(Ep + Cp - 1, Pw_from_Pd(Pd)));
      assert(Cs >= EC + (!V3D_HAS_RELAX_VPM_LIMS && Cfe == V3D_CL_TCS_BATCH_FLUSH_MODE_FULLY_PACKED ? 1u : 0u));
      assert(Es >= 2 + (g ? GVe : 0));
      assert((Ep + Cp - 1) * gfx_udiv_round_up(gfx_umax(Vn, Cn), 8) <= 64);
   }
   else if (g)
   {
      assert(Ve >= GVe);
   }
}

void v3d_vpm_cfg_validate(
   v3d_vpm_cfg_v const* v,
   v3d_vpm_cfg_t const* t,
   v3d_vpm_cfg_g const* g,
   unsigned vpm_size_in_sectors,
   unsigned max_input_vertices,
   unsigned tes_patch_vertices)
{
   for (unsigned br = 0; br <= 1; ++br)
      v3d_vpm_cfg_validate_br(v, t, g, vpm_size_in_sectors, max_input_vertices, tes_patch_vertices, br);
}

void v3d_vpm_compute_cfg(
   v3d_vpm_cfg_v* cfg,
   unsigned vpm_size_in_sectors,
   uint8_t const vs_input_words_per_vertex[2],
   uint8_t const vs_output_words_per_vertex[2],
   bool z_pre_pass)
{
   (void)z_pre_pass;
   memset(cfg, 0, sizeof(*cfg));

   for (unsigned br = 0; br != 2; ++br)
   {
      // Constants.
      unsigned const Ad = v3d_vpm_words_to_sectors(vs_input_words_per_vertex[br], V3D_CL_VPM_PACK_X16);
      unsigned const Vd = v3d_vpm_words_to_sectors(vs_output_words_per_vertex[br], V3D_CL_VPM_PACK_X16);
      unsigned const As = 1u;

      // Size the VCM cache size so that no more than half of the VPM is required.
      assert(vpm_size_in_sectors/2u >= As*Ad);
      unsigned Vc = gfx_uclamp((vpm_size_in_sectors/2u - As*Ad) / Vd, 1u, 4u);

      cfg->input_size[br].sectors = Ad;
      cfg->input_size[br].min_req = As;
      cfg->output_size[br].sectors = Vd;
      cfg->output_size[br].min_extra_req = 0;
      cfg->vcm_cache_size[br] = Vc;
   }

   debug_only(v3d_vpm_cfg_validate(cfg, NULL, NULL, vpm_size_in_sectors, 3u, 0));
}

bool v3d_vpm_compute_cfg_tg(
   v3d_vpm_cfg_v* v,
   v3d_vpm_cfg_t* t,
   v3d_vpm_cfg_g* g,
   unsigned vpm_size_in_sectors,
   uint8_t const vs_input_words[2],
   uint8_t const vs_output_words[2],
   uint8_t const tcs_patch_vertices,
   uint8_t const tcs_patch_words[2],
   uint8_t const tcs_words[2],
   bool tcs_barriers,
   uint8_t tes_patch_vertices,
   uint8_t const tes_words[2],
   v3d_cl_tess_type_t tess_type,
   uint8_t gs_max_prim_vertices,
   uint16_t const gs_output_words[2])
{
   assert(t || g);
   if (t)
   {
      assert(tcs_patch_vertices != 0 && tes_patch_vertices != 0);
      assert(tcs_patch_words != 0);
   }

   memset(v, 0, sizeof(*v));
   if (t) memset(t, 0, sizeof(*t));
   if (g) memset(g, 0, sizeof(*g));

   // Constants.
   unsigned const Vn = t ? tcs_patch_vertices : gs_max_prim_vertices;
   unsigned const Cn = t ? tes_patch_vertices : 0;
   unsigned Pd[2];
   unsigned Pw[2];

   // Global primary variables, decrease if necessary.
   unsigned Cf = !tcs_barriers ? V3D_CL_TCS_BATCH_FLUSH_MODE_FULLY_PACKED
               : Cn > 8        ? V3D_CL_TCS_BATCH_FLUSH_MODE_SINGLE_PATCH
               :                 V3D_CL_TCS_BATCH_FLUSH_MODE_PACKED_COMPLETE_PATCHES;
   unsigned Cp = 0;
   unsigned Ep = 0;

   if (t)
   {
      for (unsigned br = 0; br != 2; ++br)
      {
         Pd[br] = gfx_udiv_round_up(tcs_patch_words[br], V3D_VPM_ROWS_PER_BLOCK);
         Pw[br] = Pw_from_Pd(Pd[br]);
      }

      // Minimum number of TES vertices per patch.
      unsigned En;
      switch (tess_type)
      {
      case V3D_CL_TESS_TYPE_TRIANGLE:  En = 3; break;
      case V3D_CL_TESS_TYPE_QUAD:      En = 4; break;
      case V3D_CL_TESS_TYPE_ISOLINES:  En = 2; break;
      default:
         unreachable();
      }

      switch (Cf)
      {
      case V3D_CL_TCS_BATCH_FLUSH_MODE_FULLY_PACKED:
         Cp = gfx_udiv_round_up(16, Cn) + (unsigned)!gfx_is_power_of_2(Cn);
         break;
      case V3D_CL_TCS_BATCH_FLUSH_MODE_SINGLE_PATCH:
         Cp = 1;
         break;
      case V3D_CL_TCS_BATCH_FLUSH_MODE_PACKED_COMPLETE_PATCHES:
         Cp = 16 / Cn;
         break;
      default:
         unreachable();
      }

      Ep = gfx_udiv_round_up(16, En) + (unsigned)!gfx_is_power_of_2(En);

      // Constrain by VRT usage.
      // (Ep + Cp - 1) * gfx_udiv_round_up(gfx_umax(Vn, Cn), 8) <= 64
      unsigned EpCpMax = 1 + 64u/gfx_udiv_round_up(gfx_umax(Vn, Cn), 8u);

      // Constrain by maximum Ps.
      // (Ep + Cp - 1) <= Ps*Pw
      for (unsigned br = 0; br != 2; ++br)
         EpCpMax = gfx_umin(EpCpMax, 1 + 4*Pw[br]);

      if (Ep+Cp > EpCpMax)
      {
         if (Cp+Cp > EpCpMax)
            Cp = EpCpMax / 2;
         Ep = EpCpMax - Cp;
      }
   }

   // Do render first as this is likely more constrained.
   for (unsigned br = 2; br-- != 0; )
   {
      assert(vs_output_words[br] != 0);

      // Start with conservative values. Attempt to iteratively
      // increase these in phase 1. Values prefixed with attempt_ will
      // be increased to meet the VPM constraints.
      unsigned As = 1;
      unsigned CV = 0;
      unsigned EV = 0;
      unsigned Gs = 1;
      unsigned GV = 0;
      unsigned attempt_Vc = 1;
      unsigned attempt_Ve = 0;
      unsigned attempt_EC = 1;
      unsigned attempt_Ps = 0;
      unsigned attempt_Cs = 0;
      unsigned attempt_Es = 0;

      v3d_cl_vpm_pack_t Epk = V3D_CL_VPM_PACK_X16;
      v3d_cl_vpm_pack_t Cpk = V3D_CL_VPM_PACK_X16;
      v3d_cl_geom_output_pack_t Gpk = V3D_CL_GEOM_OUTPUT_PACK_X16;
      while (v3d_vpm_geom_words_to_sectors(gs_output_words[br], Gpk) > 16)
      {
         assert(Gpk != V3D_CL_GEOM_OUTPUT_PACK_X1);
         Gpk += 1;
      }

      v3d_cl_vpm_pack_t min_Cpk = Cn >= 16 ? V3D_CL_VPM_PACK_X16 : Cn >= 8  ? V3D_CL_VPM_PACK_X8 : V3D_CL_VPM_PACK_X4;
      v3d_cl_vpm_pack_t min_Epk = V3D_CL_VPM_PACK_X4;
      v3d_cl_geom_output_pack_t min_Gpk = V3D_CL_GEOM_OUTPUT_PACK_X1;

      for (unsigned phase = 0; phase != 2; )
      {
         unsigned Ad = v3d_vpm_words_to_sectors(vs_input_words[br], V3D_CL_VPM_PACK_X16);
         unsigned Vd = v3d_vpm_words_to_sectors(vs_output_words[br], V3D_CL_VPM_PACK_X16);

         v3d_cl_tcs_batch_flush_mode_t Cfe = Cf;
         unsigned Cd = 0, Ed = 0, CVe = 0, Cw = 0;
         if (t)
         {
            Cw = v3d_cl_vpm_pack_to_width(Cpk);
            Cfe = get_Cfe(Cf, Cp, Cn, Cw);

            Cd = v3d_vpm_words_to_sectors(tcs_words[br], Cpk);
            Ed = v3d_vpm_words_to_sectors(tes_words[br], Epk);
            CVe = CV + (V3D_HAS_RELAX_VPM_LIMS || Cfe == V3D_CL_TCS_BATCH_FLUSH_MODE_SINGLE_PATCH ? 0u : (Vn <= 16 ? 1u : 2u));
         }

         unsigned Gd = 0, GVe = 0;
         if (g)
         {
            Gd = v3d_vpm_geom_words_to_sectors(gs_output_words[br], Gpk);
            GVe = GV + (!V3D_HAS_RELAX_VPM_LIMS ? (t || Vn <= 16 ? 1u : 2u) : 0);
         }

         unsigned num_sectors = As*Ad;

         unsigned Vc = gfx_umax(attempt_Vc, gfx_udiv_round_up(Vn, V3D_VPAR));
         unsigned Ve = 0;
         unsigned Ps = 0;
         unsigned EC = 0;
         unsigned Cs = 0;
         unsigned Es = 0;
         if (t)
         {
            Ve = gfx_umax(attempt_Ve, EV + gfx_umax(CVe, !V3D_HAS_RELAX_VPM_LIMS ? 1u : 0u));
            EC = gfx_umax(attempt_EC, Cfe != V3D_CL_TCS_BATCH_FLUSH_MODE_FULLY_PACKED ? gfx_udiv_round_up(Cn, Cw) : gfx_umin(Cn, 2u));
            Ps = gfx_umax(attempt_Ps, gfx_udiv_round_up(Ep + Cp - 1, Pw[br]));
            Cs = gfx_umax(attempt_Cs, EC + (!V3D_HAS_RELAX_VPM_LIMS && Cfe == V3D_CL_TCS_BATCH_FLUSH_MODE_FULLY_PACKED ? 1u : 0u));
            Es = gfx_umax(attempt_Es, 2 + (g ? GVe : 0));

            num_sectors += Ps + Cs*Cd + Es*Ed;
         }
         else
         {
            Ve = gfx_umax(attempt_Ve, GVe);
         }

         num_sectors += (Vc + Ve)*Vd;
         num_sectors += (g ? Gs*Gd : 0);

         switch (phase)
         {
         case 0:
            // Check VPM limit.
            if (num_sectors > vpm_size_in_sectors)
            {
               // Lower Gpk, then Cpk, Epk
               if (g && Gpk != min_Gpk) { Gpk += 1; continue; }
               if (t)
               {
                  if (Cpk != min_Cpk) { Cpk += 1; continue; }
                  if (Epk != min_Epk) { Epk += 1; continue; }

                  // Lower greater of either Ep or Cp
                  if (Cp >= Ep && Cp > 1) { Cp -= 1; continue; }
                  if (Ep > Cp && Ep > 1) { Ep -= 1; continue; }
               }

               // Give up...
               return false;
            }
            /* fallthrough */

         case 1:
            // Save currently valid settings.
            if (phase == 0 || num_sectors <= vpm_size_in_sectors/2)
            {
               phase = 1;

               v->input_size[br].sectors        = Ad;
               v->input_size[br].min_req        = As;
               v->output_size[br].sectors       = Vd;
               v->output_size[br].min_extra_req = Ve;
               v->vcm_cache_size[br]            = Vc;

               if (t)
               {
                  t->per_patch_depth[br]                   = Pd[br];
                  t->min_per_patch_segs[br]                = Ps;
                  t->tcs_output[br].pack                   = Cpk;
                  t->tcs_output[br].size_sectors           = Cd;
                  t->max_extra_vert_segs_per_tcs_batch[br] = CV;
                  t->max_patches_per_tcs_batch             = Cp;
                  t->tcs_batch_flush                       = Cf;
                  t->tes_output[br].pack                   = Epk;
                  t->min_tcs_segs[br]                      = Cs;
                  t->tes_output[br].size_sectors           = Ed;
                  t->max_extra_vert_segs_per_tes_batch[br] = EV;
                  t->max_tcs_segs_per_tes_batch[br]        = EC;
                  t->max_patches_per_tes_batch             = Ep;
                  t->min_tes_segs[br]                      = Es;
               }

               if (g)
               {
                  g->geom_output[br].pack = Gpk;
                  g->geom_output[br].size_sectors = Gd;
                  g->max_extra_vert_segs_per_gs_batch[br] = GV;
                  g->min_gs_segs[br] = Gs;
               }

               // Assert not walking through invalid configurations either.
               debug_only(v3d_vpm_cfg_validate_br(v, t, g, vpm_size_in_sectors, gs_max_prim_vertices, tes_patch_vertices, br));
            }

            // Spare VPM to play with?
            if (num_sectors < vpm_size_in_sectors/2)
            {
               // Increase CV to 1.
               if (t && CV < 1) { CV += 1; continue; }

               // Increase Vc to 2; assuming there is little benefit beyond this.
               if (Vc < 2) { attempt_Vc = 2; continue; }

               // Stop now...
            }

            // done...
            phase = 2;
            break;
         }
      }
   }

   debug_only(v3d_vpm_cfg_validate(v, t, g, vpm_size_in_sectors, gs_max_prim_vertices, tes_patch_vertices));
   return true;
}

#else

void v3d_vpm_compute_cfg(
   v3d_vpm_cfg_v* cfg,
   unsigned vpm_size_in_sectors,
   uint8_t const vs_input_words[2],
   uint8_t const vs_output_words[2],
   bool z_pre_pass)
{
   // The hardware vertex cache manager (VCM) receives a stream of indices and produces
   // batches of unique vertices and a remapped index stream. Each thread's VCM
   // (bin/render) can be configured to generate references to vertices in the last
   // 1-4 batches. A thread must not require more than half the total VPM memory at any
   // one time since a deadlock can occur if this happens in conjunction with the other
   // thread.

   // The VPM has vpm_size_in_sectors*512 bytes of memory, split into sectors of 512 bytes.
   // For the purpose of configuring the VCM, we allocate half to each thread.
   // An individual thread can use more than this if available, but is always
   // guaranteed to make progress with only half.

   // Compute VCM cache size for bin then render.
   for (unsigned br = 0; br != 2; ++br)
   {
      assert(vs_output_words[br] != 0);
      unsigned Ad = gfx_udiv_round_up(vs_input_words[br], V3D_VPM_ROWS_PER_BLOCK);
      unsigned Vd = gfx_udiv_round_up(vs_output_words[br], V3D_VPM_ROWS_PER_BLOCK);
      cfg->input_size[br] = Ad;
      cfg->output_size[br] = Vd;

      // In the case where the VCM cache size is 1, a HW thread can make progress with 1 input
      // segment and 2 output segments, or can be stuck with 1 output segment allocated.
      // Therefore, as long as a progressing thread does not require more than 2/3 of the VPM
      // and a stuck thread does not hold more than 1/3 of the VPM, progress can be made.
      // In the case where the VCM cache size is greater than 1, then we ensure that no more than
      // 1/2 of the VPM is required to make progress. This should allow all permutations of
      // bin/render VCM configuration to coexist.
      // This constraint can fail with separate IO blocks (which are not enabled) on 16kb VPM.
      assert(Ad + 2*Vd <= vpm_size_in_sectors/3*2);

      // We need space for an additional input and output segment, on top of the span
      // of segments referenced by the remapped indices from the VCM as a new segment
      // is required to displace the oldest one in the PTB/PSE (see v3d_vbt.v).
      assert(vpm_size_in_sectors/2u >= Ad + Vd);
      cfg->vcm_cache_size[br] = gfx_uclamp((vpm_size_in_sectors/2u - Ad - Vd) / Vd, 1u, 4u);
   }

   // If using z-prepass, then account for running the coordinate shader in rendering.
   if (z_pre_pass)
      cfg->vcm_cache_size[1] = gfx_umin(cfg->vcm_cache_size[0], cfg->vcm_cache_size[1]);
}

#endif
