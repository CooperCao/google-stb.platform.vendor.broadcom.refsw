/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#include "v3d_vpm.h"
#include "v3d_vpm_alloc.h"
#include "v3d_cl.h"
#include "v3d_limits.h"
#include "libs/util/gfx_util/gfx_util.h"
#include <string.h>

#if V3D_HAS_TNG

static inline uint32_t v3d_vpm_words_to_sectors(uint32_t num_words, v3d_cl_vpm_pack_t pk)
{
   assert(1 << pk == V3D_VPAR/v3d_cl_vpm_pack_to_width(pk));
   return gfx_udiv_round_up(num_words, V3D_VPM_ROWS_PER_BLOCK << pk);
}

static inline unsigned Pw_from_Pd(unsigned Pd)
{
   unsigned Pw = 16 / gfx_next_power_of_2(Pd);
   Pw = Pw < 4 ? 1 : Pw;
   return Pw;
}

void v3d_vpm_default_cfg_t(v3d_vpm_cfg_t* t)
{
   memset(t, 0, sizeof(*t));
}

void v3d_vpm_default_cfg_g(v3d_vpm_cfg_g* g)
{
   memset(g, 0, sizeof(*g));
   g->min_gs_segs[0] = 1;
   g->min_gs_segs[1] = 1;
}

bool v3d_vpm_cfg_validate_br(
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
   unsigned Vw = v3d_cl_vpm_pack_to_width(v->output_size[br].pack);
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

   assert(As-1 < 4);
   assert(Vn-1 < 32);
   assert(Vw == 4 || Vw == 8 || Vw == 16);
   assert(Vd > 0);
   assert(Vc-1 < 4);
   assert(Ve < 4);
   if (t)
   {
      assert(Pd-1 < 16);
      assert(Ps-1 < 4);
      assert(Cn-1 < 32);
      assert(Cw == 4 || Cw == 8 || Cw == 16);
      assert(Cd > 0);
      assert(CV < 4);
      assert(Cp-1 < 16);
      assert(Cs-1 < 8);
      assert(Ed > 0);
      assert(EV < 4);
      assert(EC-1 < 8);
      assert(Ep-1 < 16);
      assert(Es-1 < 8);
   }
   if (g)
   {
      assert(Gd > 0);
      assert(GV < 4);
      assert(Gs-1 < 8);
   }

   unsigned num_sectors = As*Ad + (Vc + Ve)*Vd + (t ? Ps + Cs*Cd + Es*Ed : 0) + (g ? Gs*Gd : 0);
   if ( !(num_sectors <= vpm_size_in_sectors) )
      return false;

   if ( !(Vw >= gfx_umin(Vn, 16u)) )
      return false;

   if ( !(Vc >= gfx_udiv_round_up(Vn, Vw)) )
      return false;

   unsigned CVe = CV + (Cp == 1 || Cf == V3D_CL_TCS_BATCH_FLUSH_MODE_SINGLE_PATCH ? 0u : (Vn <= 16 ? 1u : 2u));
   unsigned GVe = GV + (t || Vn <= 16 ? 1u : 2u);

   if (t)
   {
      if ( !(Cw >= gfx_umin(Cn, 16u)) )
         return false;

      if ( !(EC >= (Cf == V3D_CL_TCS_BATCH_FLUSH_MODE_FULLY_PACKED ? gfx_umin(Cn, 2u) : gfx_udiv_round_up(Cn, Cw))) )
         return false;

      if ( !(Ve >= EV + gfx_umax(CVe, 1)) )
         return false;

      if ( !(Ps >= gfx_udiv_round_up(Ep + Cp - 1, Pw_from_Pd(Pd))) )
         return false;

      if ( !(Cs >= EC) )
         return false;

      if ( !(Es >= 2 + (g ? GVe : 0)) )
         return false;

      if ( !((Ep + Cp - 1) * gfx_udiv_round_up(gfx_umax(Vn, Cn), 8) <= 64) )
         return false;
   }
   else if (g)
   {
      if ( !(Ve >= GVe) )
         return false;
   }

   return true;
}

bool v3d_vpm_cfg_validate(
   v3d_vpm_cfg_v const* v,
   v3d_vpm_cfg_t const* t,
   v3d_vpm_cfg_g const* g,
   unsigned vpm_size_in_sectors,
   unsigned max_input_vertices,
   unsigned tes_patch_vertices)
{
   for (unsigned br = 0; br <= 1; ++br)
   {
      if (!v3d_vpm_cfg_validate_br(v, t, g, vpm_size_in_sectors, max_input_vertices, tes_patch_vertices, br))
         return false;
   }
   return true;
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
      cfg->output_size[br].pack = V3D_CL_VPM_PACK_X16;
      cfg->output_size[br].sectors = Vd;
      cfg->output_size[br].min_extra_req = 0;
      cfg->vcm_cache_size[br] = Vc;
   }

   assert(v3d_vpm_cfg_validate(cfg, NULL, NULL, vpm_size_in_sectors, 3u, 0));
}

void v3d_vpm_compute_cfg_t(
   v3d_vpm_cfg_v* v,
   v3d_vpm_cfg_t* t,
   unsigned vpm_size_in_sectors,
   uint8_t const vs_input_words[2],
   uint8_t const vs_output_words[2],
   uint8_t const tcs_patch_vertices,
   uint8_t const tcs_patch_words[2],
   uint8_t const tcs_words[2],
   bool tcs_barriers,
   uint8_t const tes_patch_vertices,
   uint8_t const tes_words[2])
{
   assert(tcs_patch_vertices != 0 && tes_patch_vertices != 0);
   assert(tcs_patch_words != 0);

   memset(v, 0, sizeof(*v));
   memset(t, 0, sizeof(*t));

   for (unsigned br = 0; br != 2; ++br)
   {
      assert(vs_output_words[br] != 0);

      unsigned tcs_patch_words_br = tcs_patch_words[br];
      unsigned vs_input_words_br  = vs_input_words[br];
      unsigned vs_output_words_br = vs_output_words[br];
      unsigned tcs_words_br = tcs_words[br];
      unsigned tes_words_br = tes_words[br];

      // Constants.
      unsigned const Vn = tcs_patch_vertices;
      unsigned const Cn = tes_patch_vertices;
      unsigned const Pd = gfx_udiv_round_up(tcs_patch_words_br, V3D_VPM_ROWS_PER_BLOCK);
      unsigned const Cf = !tcs_barriers ? V3D_CL_TCS_BATCH_FLUSH_MODE_FULLY_PACKED
                          : Cn > 8      ? V3D_CL_TCS_BATCH_FLUSH_MODE_SINGLE_PATCH
                          :               V3D_CL_TCS_BATCH_FLUSH_MODE_PACKED_COMPLETE_PATCHES;

      // Primary variables (decrease packing if necessary?).
      v3d_cl_vpm_pack_t Vpk = V3D_CL_VPM_PACK_X16;
      v3d_cl_vpm_pack_t Cpk = V3D_CL_VPM_PACK_X16;
      v3d_cl_vpm_pack_t Epk = V3D_CL_VPM_PACK_X16;

      // Primary variables (increase if possible).
      unsigned As = 1;
      unsigned CV = 0;
      unsigned Cp = 1;
      unsigned EV = 0;
      unsigned Ep = 1;
      unsigned Es = 2;

      // Derived variables.
      unsigned Ad = v3d_vpm_words_to_sectors(vs_input_words_br, Vpk);
      unsigned Vd = v3d_vpm_words_to_sectors(vs_output_words_br, Vpk);
      unsigned Vw = v3d_cl_vpm_pack_to_width(Vpk);
      unsigned Cd = v3d_vpm_words_to_sectors(tcs_words_br, Cpk);
      unsigned Ed = v3d_vpm_words_to_sectors(tes_words_br, Epk);
      unsigned CVe = CV + (Cp == 1 || Cf == V3D_CL_TCS_BATCH_FLUSH_MODE_SINGLE_PATCH ? 0u : (Vn <= 16 ? 1u : 2u));
      unsigned Cw = v3d_cl_vpm_pack_to_width(Cpk);

      // Secondary variables (increase if possible).
      unsigned Vc = gfx_udiv_round_up(Vn, Vw);
      unsigned Ve = EV + gfx_umax(CVe, 1);
      unsigned Ps = gfx_udiv_round_up(Ep + Cp - 1, Pw_from_Pd(Pd));
      unsigned EC = (Cf == V3D_CL_TCS_BATCH_FLUSH_MODE_FULLY_PACKED ? gfx_umin(Cn, 2u) : gfx_udiv_round_up(Cn, Cw));
      unsigned Cs = EC;

      v->input_size[br].sectors                 = Ad;
      v->input_size[br].min_req                 = As;
      v->output_size[br].pack                   = Vpk;
      v->output_size[br].sectors                = Vd;
      v->output_size[br].min_extra_req          = Ve;
      v->vcm_cache_size[br]                     = Vc;
      t->per_patch_depth[br]                   = Pd;
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

   assert(v3d_vpm_cfg_validate(v, t, NULL, vpm_size_in_sectors, tcs_patch_vertices, tes_patch_vertices));
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
