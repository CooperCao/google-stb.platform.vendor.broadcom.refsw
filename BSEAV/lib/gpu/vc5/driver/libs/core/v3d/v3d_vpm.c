/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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

static void v3d_vpm_cfg_validate_br(
   v3d_vpm_cfg_v const* v,
   // tg may be NULL if !t && !g
   V3D_VPM_CFG_TG_T const* tg, bool t, bool g,
   unsigned vpm_size_in_sectors,
   unsigned max_input_vertices,
   unsigned tes_patch_vertices)
{
   unsigned Ad = v->input_size.sectors;
   unsigned As = v->input_size.min_req;
   unsigned Vn = max_input_vertices;
   unsigned Vd = v->output_size.sectors;
   unsigned Ve = v->output_size.min_extra_req;
   unsigned Vc = v->vcm_cache_size;
   unsigned Pd = t ? tg->per_patch_depth : 0;
   unsigned Ps = t ? tg->min_per_patch_segs : 0;
   unsigned Cn = tes_patch_vertices;
   unsigned Cw = t ? v3d_cl_vpm_pack_to_width(tg->tcs_output.pack) : 0;
   unsigned Cd = t ? tg->tcs_output.size_sectors : 0;
   unsigned CV = t ? tg->max_extra_vert_segs_per_tcs_batch : 0;
   unsigned Cp = t ? tg->max_patches_per_tcs_batch : 0;
   unsigned Cf = t ? tg->tcs_batch_flush : 0;
   unsigned Cs = t ? tg->min_tcs_segs : 0;
   unsigned Ed = t ? tg->tes_output.size_sectors : 0;
   unsigned EV = t ? tg->max_extra_vert_segs_per_tes_batch : 0;
   unsigned EC = t ? tg->max_tcs_segs_per_tes_batch : 0;
   unsigned Ep = t ? tg->max_patches_per_tes_batch : 0;
   unsigned Es = t ? tg->min_tes_segs : 0;
   unsigned Gd = g ? tg->geom_output.size_sectors : 0;
   unsigned GV = g ? tg->max_extra_vert_segs_per_gs_batch : 0;
   unsigned Gs = g ? tg->min_gs_segs : 0;

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

   unsigned GVe = GV + (!V3D_VER_AT_LEAST(4,1,34,0) ? (t || Vn <= 16 ? 1u : 2u) : 0);
   if (t)
   {
      unsigned Cfe = get_Cfe(Cf, Cp, Cn, Cw);
      unsigned CVe = CV + (V3D_VER_AT_LEAST(4,1,34,0) || Cfe == V3D_CL_TCS_BATCH_FLUSH_MODE_SINGLE_PATCH ? 0u : (Vn <= 16 ? 1u : 2u));

      assert(Cw >= gfx_umin(Cn, 16u));
      assert(EC >= (Cfe != V3D_CL_TCS_BATCH_FLUSH_MODE_FULLY_PACKED ? gfx_udiv_round_up(Cn, Cw) : gfx_umin(Cn, 2u)));
      assert(Ve >= EV + gfx_umax(CVe, !V3D_VER_AT_LEAST(4,1,34,0) ? 1u : 0u));
      assert(Ps >= gfx_udiv_round_up(Ep + Cp - 1, Pw_from_Pd(Pd)));
      assert(Cs >= EC + (!V3D_VER_AT_LEAST(4,1,34,0) && Cfe == V3D_CL_TCS_BATCH_FLUSH_MODE_FULLY_PACKED ? 1u : 0u));
      assert(Es >= 2 + (g ? GVe : 0));
      assert((Ep + Cp - 1) * gfx_udiv_round_up(gfx_umax(Vn, Cn), 8) <= 64);
   }
   else if (g)
   {
      assert(Ve >= GVe);
   }
}

void v3d_vpm_cfg_validate(
   v3d_vpm_cfg_v const v[2],
   // tg may be NULL if !t && !g
   V3D_VPM_CFG_TG_T const tg[2], bool t, bool g,
   unsigned vpm_size_in_sectors,
   unsigned max_input_vertices,
   unsigned tes_patch_vertices)
{
#if !V3D_VER_AT_LEAST(4,1,34,0)
   if (t || g)
   {
      assert(tg[0].tcs_batch_flush == tg[1].tcs_batch_flush);
      assert(tg[0].max_patches_per_tcs_batch == tg[1].max_patches_per_tcs_batch);
      assert(tg[0].max_patches_per_tes_batch == tg[1].max_patches_per_tes_batch);
   }
#endif
   for (unsigned br = 0; br <= 1; ++br)
      v3d_vpm_cfg_validate_br(&v[br], tg ? &tg[br] : NULL, t, g, vpm_size_in_sectors, max_input_vertices, tes_patch_vertices);
}

void v3d_vpm_compute_cfg(
   v3d_vpm_cfg_v cfg[2],
   unsigned vpm_size_in_sectors,
   uint8_t const vs_input_words_per_vertex[2],
   uint8_t const vs_output_words_per_vertex[2],
   bool z_pre_pass)
{
   (void)z_pre_pass;
   memset(cfg, 0, 2 * sizeof(*cfg));

   for (unsigned br = 0; br != 2; ++br)
   {
      // Constants.
      unsigned const Ad = v3d_vpm_words_to_sectors(vs_input_words_per_vertex[br], V3D_CL_VPM_PACK_X16);
      unsigned const Vd = v3d_vpm_words_to_sectors(vs_output_words_per_vertex[br], V3D_CL_VPM_PACK_X16);
      unsigned const As = 1u;

      // Size the VCM cache size so that no more than half of the VPM is required.
      assert(vpm_size_in_sectors/2u >= As*Ad);
      unsigned Vc = gfx_uclamp((vpm_size_in_sectors/2u - As*Ad) / Vd, 1u, 4u);

      cfg[br].input_size.sectors = Ad;
      cfg[br].input_size.min_req = As;
      cfg[br].output_size.sectors = Vd;
      cfg[br].output_size.min_extra_req = 0;
      cfg[br].vcm_cache_size = Vc;
   }

   debug_only(v3d_vpm_cfg_validate(cfg, NULL, false, false, vpm_size_in_sectors, 3u, 0));
}

static unsigned get_max_sensible_Cp(unsigned Cf, unsigned Cn)
{
   switch (Cf)
   {
   case V3D_CL_TCS_BATCH_FLUSH_MODE_FULLY_PACKED:
      return gfx_udiv_round_up(16, Cn) + (unsigned)!gfx_is_power_of_2(Cn);
   case V3D_CL_TCS_BATCH_FLUSH_MODE_SINGLE_PATCH:
      return 1;
   case V3D_CL_TCS_BATCH_FLUSH_MODE_PACKED_COMPLETE_PATCHES:
      return 16 / Cn;
   default:
      unreachable();
      return 0;
   }
}

static unsigned get_max_sensible_Ep(v3d_cl_tess_type_t tess_type)
{
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

   return gfx_udiv_round_up(16, En) + (unsigned)!gfx_is_power_of_2(En);
}

static void constrain_Cp_plus_Ep(unsigned *Cp, unsigned *Ep,
   unsigned Vn, unsigned Cn, unsigned min_Pw)
{
   // Constrain by VRT usage.
   // (Ep + Cp - 1) * gfx_udiv_round_up(gfx_umax(Vn, Cn), 8) <= 64
   unsigned max_Ep_plus_Cp = 1 + 64u/gfx_udiv_round_up(gfx_umax(Vn, Cn), 8u);

   // Constrain by maximum Ps.
   // (Ep + Cp - 1) <= Ps*Pw
   max_Ep_plus_Cp = gfx_umin(max_Ep_plus_Cp, 1 + 4*min_Pw);

   if (*Ep+*Cp > max_Ep_plus_Cp)
   {
      if (*Cp+*Cp > max_Ep_plus_Cp)
         *Cp = max_Ep_plus_Cp / 2;
      *Ep = max_Ep_plus_Cp - *Cp;
   }
}

bool v3d_vpm_compute_cfg_tg(
   v3d_vpm_cfg_v v[2],
   V3D_VPM_CFG_TG_T tg[2], bool t, bool g,
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

   memset(v, 0, 2 * sizeof(*v));
   memset(tg, 0, 2 * sizeof(*tg));
   for (unsigned br = 0; br != 2; ++br)
   {
      if (!t)
      {
         // Some encodable values
         tg[br].per_patch_depth = 1;
         tg[br].max_patches_per_tcs_batch = 1;
         tg[br].min_tcs_segs = 1;
         tg[br].min_per_patch_segs = 1;
         tg[br].max_patches_per_tes_batch = 1;
         tg[br].max_tcs_segs_per_tes_batch = 1;
         tg[br].min_tes_segs = 1;
      }
      if (!g)
         tg[br].min_gs_segs = 1;
   }

   // Constants.
   unsigned const Vn = t ? tcs_patch_vertices : gs_max_prim_vertices;
   unsigned const Cn = t ? tes_patch_vertices : 0;
   unsigned const Cf = !tcs_barriers ? V3D_CL_TCS_BATCH_FLUSH_MODE_FULLY_PACKED
                     : Cn > 8        ? V3D_CL_TCS_BATCH_FLUSH_MODE_SINGLE_PATCH
                     :                 V3D_CL_TCS_BATCH_FLUSH_MODE_PACKED_COMPLETE_PATCHES;
   unsigned const max_sensible_Cp = t ? get_max_sensible_Cp(Cf, Cn) : 0;
   unsigned const max_sensible_Ep = t ? get_max_sensible_Ep(tess_type) : 0;
   unsigned Pd[2];
   unsigned Pw[2];
   if (t)
   {
      for (unsigned br = 0; br != 2; ++br)
      {
         Pd[br] = gfx_udiv_round_up(tcs_patch_words[br], V3D_VPM_ROWS_PER_BLOCK);
         Pw[br] = Pw_from_Pd(Pd[br]);
      }
   }

#if !V3D_VER_AT_LEAST(4,1,34,0)
   // These start high and are decreased if necessary during phase 0. Note that
   // they are common to bin and render...
   unsigned Cp = max_sensible_Cp;
   unsigned Ep = max_sensible_Ep;
   if (t)
      constrain_Cp_plus_Ep(&Cp, &Ep, Vn, Cn, gfx_umin(Pw[0], Pw[1]));

   // Do render first as this is likely more constrained.
#endif
   for (unsigned br = 2; br-- != 0; )
   {
      assert(vs_output_words[br] != 0);

      unsigned const Ad = v3d_vpm_words_to_sectors(vs_input_words[br], V3D_CL_VPM_PACK_X16);
      unsigned const Vd = v3d_vpm_words_to_sectors(vs_output_words[br], V3D_CL_VPM_PACK_X16);

      // These start high and are decreased if necessary during phase 0
      v3d_cl_vpm_pack_t Epk = V3D_CL_VPM_PACK_X16;
      v3d_cl_vpm_pack_t Cpk = V3D_CL_VPM_PACK_X16;
      v3d_cl_geom_output_pack_t Gpk = V3D_CL_GEOM_OUTPUT_PACK_X16;
      while (g && (v3d_vpm_geom_words_to_sectors(gs_output_words[br], Gpk) > 16))
      {
         assert(Gpk != V3D_CL_GEOM_OUTPUT_PACK_X1);
         Gpk += 1;
      }
#if V3D_VER_AT_LEAST(4,1,34,0)
      unsigned Cp = max_sensible_Cp;
      unsigned Ep = max_sensible_Ep;
      if (t)
         constrain_Cp_plus_Ep(&Cp, &Ep, Vn, Cn, Pw[br]);
#endif

      // We won't decrease Cpk/Epk/Gpk below these values
      v3d_cl_vpm_pack_t const min_Cpk = Cn >= 16 ? V3D_CL_VPM_PACK_X16 : Cn >= 8  ? V3D_CL_VPM_PACK_X8 : V3D_CL_VPM_PACK_X4;
      v3d_cl_vpm_pack_t const min_Epk = V3D_CL_VPM_PACK_X4;
      v3d_cl_geom_output_pack_t const min_Gpk = V3D_CL_GEOM_OUTPUT_PACK_X1;

      // These start low and are iteratively increased in phase 1 if there is
      // some spare VPM space to play with. For the min_ ones we may use larger
      // values to satisfy the HW constraints.
      unsigned const As = 1;
      unsigned CV = 0;
      unsigned const EV = 0;
      unsigned const Gs = 1;
      unsigned const GV = 0;
      unsigned min_Vc = 1;
      unsigned const min_Ve = 0;
      unsigned const min_EC = 1;
      unsigned const min_Ps = 0;
      unsigned const min_Cs = 0;
      unsigned const min_Es = 0;

      for (unsigned phase = 0; phase != 2; )
      {
         v3d_cl_tcs_batch_flush_mode_t Cfe = Cf;
         unsigned Cd = 0, Ed = 0, CVe = 0, Cw = 0;
         if (t)
         {
            Cw = v3d_cl_vpm_pack_to_width(Cpk);
            Cfe = get_Cfe(Cf, Cp, Cn, Cw);

            Cd = v3d_vpm_words_to_sectors(tcs_words[br], Cpk);
            Ed = v3d_vpm_words_to_sectors(tes_words[br], Epk);
            CVe = CV + (V3D_VER_AT_LEAST(4,1,34,0) || Cfe == V3D_CL_TCS_BATCH_FLUSH_MODE_SINGLE_PATCH ? 0u : (Vn <= 16 ? 1u : 2u));
         }

         unsigned num_sectors = As*Ad;

         unsigned Gd = 0, GVe = 0;
         if (g)
         {
            Gd = v3d_vpm_geom_words_to_sectors(gs_output_words[br], Gpk);
            GVe = GV + (!V3D_VER_AT_LEAST(4,1,34,0) ? (t || Vn <= 16 ? 1u : 2u) : 0);

            num_sectors += Gs*Gd;
         }

         unsigned Vc = gfx_umax(min_Vc, gfx_udiv_round_up(Vn, V3D_VPAR));
         unsigned Ve = 0;
         unsigned Ps = 0;
         unsigned EC = 0;
         unsigned Cs = 0;
         unsigned Es = 0;
         if (t)
         {
            Ve = gfx_umax(min_Ve, EV + gfx_umax(CVe, !V3D_VER_AT_LEAST(4,1,34,0) ? 1u : 0u));
            EC = gfx_umax(min_EC, Cfe != V3D_CL_TCS_BATCH_FLUSH_MODE_FULLY_PACKED ? gfx_udiv_round_up(Cn, Cw) : gfx_umin(Cn, 2u));
            Ps = gfx_umax(min_Ps, gfx_udiv_round_up(Ep + Cp - 1, Pw[br]));
            Cs = gfx_umax(min_Cs, EC + (!V3D_VER_AT_LEAST(4,1,34,0) && Cfe == V3D_CL_TCS_BATCH_FLUSH_MODE_FULLY_PACKED ? 1u : 0u));
            Es = gfx_umax(min_Es, 2 + (g ? GVe : 0));

            num_sectors += Ps + Cs*Cd + Es*Ed;
         }
         else
         {
            Ve = gfx_umax(min_Ve, GVe);
         }

         num_sectors += (Vc + Ve)*Vd;

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

               v[br].input_size.sectors        = Ad;
               v[br].input_size.min_req        = As;
               v[br].output_size.sectors       = Vd;
               v[br].output_size.min_extra_req = Ve;
               v[br].vcm_cache_size            = Vc;

               if (t)
               {
                  tg[br].per_patch_depth                   = Pd[br];
                  tg[br].min_per_patch_segs                = Ps;
                  tg[br].tcs_output.pack                   = Cpk;
                  tg[br].tcs_output.size_sectors           = Cd;
                  tg[br].max_extra_vert_segs_per_tcs_batch = CV;
#if V3D_VER_AT_LEAST(4,1,34,0)
                  tg[br].max_patches_per_tcs_batch         = Cp;
#else
                  tg[0].max_patches_per_tcs_batch          = Cp;
                  tg[1].max_patches_per_tcs_batch          = Cp;
#endif
                  tg[br].tcs_batch_flush                   = Cf;
                  tg[br].tes_output.pack                   = Epk;
                  tg[br].min_tcs_segs                      = Cs;
                  tg[br].tes_output.size_sectors           = Ed;
                  tg[br].max_extra_vert_segs_per_tes_batch = EV;
                  tg[br].max_tcs_segs_per_tes_batch        = EC;
#if V3D_VER_AT_LEAST(4,1,34,0)
                  tg[br].max_patches_per_tes_batch         = Ep;
#else
                  tg[0].max_patches_per_tes_batch          = Ep;
                  tg[1].max_patches_per_tes_batch          = Ep;
#endif
                  tg[br].min_tes_segs                      = Es;
               }

               if (g)
               {
                  tg[br].geom_output.pack = Gpk;
                  tg[br].geom_output.size_sectors = Gd;
                  tg[br].max_extra_vert_segs_per_gs_batch = GV;
                  tg[br].min_gs_segs = Gs;
               }

               // Assert not walking through invalid configurations either.
               debug_only(v3d_vpm_cfg_validate_br(&v[br], &tg[br], t, g, vpm_size_in_sectors, gs_max_prim_vertices, tes_patch_vertices));
            }

            // Spare VPM to play with?
            if (num_sectors < vpm_size_in_sectors/2)
            {
               // Increase CV to 1.
               if (t && CV < 1) { CV += 1; continue; }

               // Increase Vc to 2; assuming there is little benefit beyond this.
               if (Vc < 2) { min_Vc = 2; continue; }

               // Stop now...
            }

            // done...
            phase = 2;
            break;
         }
      }
   }

   debug_only(v3d_vpm_cfg_validate(v, tg, t, g, vpm_size_in_sectors, gs_max_prim_vertices, tes_patch_vertices));
   return true;
}

#else

void v3d_vpm_compute_cfg(
   v3d_vpm_cfg_v cfg[2],
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
      cfg[br].input_size = Ad;
      cfg[br].output_size = Vd;

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
      cfg[br].vcm_cache_size = gfx_uclamp((vpm_size_in_sectors/2u - Ad - Vd) / Vd, 1u, 4u);
   }

   // If using z-prepass, then account for running the coordinate shader in rendering.
   if (z_pre_pass)
      cfg[1].vcm_cache_size = gfx_umin(cfg[0].vcm_cache_size, cfg[1].vcm_cache_size);
}

#endif
