/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_backend_uniforms.h"
#include "glsl_backflow.h"
#include "glsl_backflow_visitor.h"
#include "glsl_backend.h"

#include "glsl_sched_node_helpers.h"
#include "glsl_backend_cfg.h"
#include "glsl_qbe_fragment.h"
#include "glsl_qbe_fragment_adv_blend.h"

#include "libs/util/gfx_util/gfx_util.h"

#include <assert.h>

static Backflow *cov_accum(Backflow *mask, Backflow *cov) {
   return cov ? bitand(mask, cov) : mask;
}

static Backflow *coverage_and(Backflow *mask, Backflow *dep) {
   return tr_uop_io(BACKFLOW_SETMSF, bitand(tr_nullary(BACKFLOW_MSF), mask), dep);
}

static Backflow *tr_discard(Backflow *prev, Backflow *cond) {
   Backflow *zero   = tr_const(0);
   Backflow *result = create_node(BACKFLOW_SETMSF, UNPACK_NONE, COND_IFFLAG, cond, zero, NULL, NULL);

   glsl_iodep(result, prev);

   return result;
}

static void fragment_backend(
   const FragmentBackendState *s,
   SchedBlock *block,
   Backflow *const *rgbas,
   Backflow *discard,
   Backflow *depth,
   Backflow *sample_mask,
   bool *writes_z_out, bool *ez_disable_out)
{
#define R(RT, SAMPLE) (rgbas[(((RT) * 4) + 0)*4 + (SAMPLE)])
#define G(RT, SAMPLE) (rgbas[(((RT) * 4) + 1)*4 + (SAMPLE)])
#define B(RT, SAMPLE) (rgbas[(((RT) * 4) + 2)*4 + (SAMPLE)])
#define A(RT, SAMPLE) (rgbas[(((RT) * 4) + 3)*4 + (SAMPLE)])

   Backflow *cov_dep = NULL;

   /* Don't update MS flags until all TMU operations which modify memory have
    * been submitted. Don't need to wait for them to complete -- MS flags are
    * sampled by the retiring writes. */
   if (block->tmu_lookups) {
      for (struct tmu_lookup_s *l = block->tmu_lookups; l != NULL; l = l->next) {
         if (l->is_modify)
            cov_dep = l->last_write;
      }
   }

   Backflow *coverage = NULL;
   if (s->sample_alpha_to_coverage)
      coverage = cov_accum( tr_uop(BACKFLOW_FTOC, A(0,0)), coverage );

   /* Apply any sample mask set in the API */
   if (s->sample_mask)
      coverage = cov_accum( tr_special_uniform(BACKEND_SPECIAL_UNIFORM_SAMPLE_MASK), coverage );

   /* Apply any sample mask set in the shader */
   if (s->ms && sample_mask)
      coverage = cov_accum(sample_mask, coverage);

   /* The conditions appear inverted here because there's an extra invert
    * when going from uniform values to flags for conditions              */
   if (discard && is_const(discard) && discard->unif == CONST_BOOL_TRUE)
      discard = NULL;

   if (coverage != NULL)
      cov_dep = coverage_and(coverage, cov_dep);

   if (discard != NULL)
      cov_dep = tr_discard(cov_dep, discard);

   /* Sort out TLB storing dependencies */
   bool does_discard = (discard != NULL || s->sample_alpha_to_coverage || s->sample_mask);
   bool invariant_z_write = (does_discard && !s->fez_safe_with_discard) || !s->early_fragment_tests;
   bool full_z_write      = (depth != NULL);

   Backflow *tlb_nodes[4*4*V3D_MAX_RENDER_TARGETS+1];
   uint32_t unif_byte[4*4*V3D_MAX_RENDER_TARGETS+1];

   uint32_t tlb_depth_age  = 0;
   unsigned tlb_node_count = 0;

   if (full_z_write)
   {
      unif_byte[tlb_node_count] = 0x84;  /* per-pixel Z write */
      tlb_nodes[tlb_node_count++] = depth;
      tlb_depth_age = depth->age;
   }
   else if (invariant_z_write)
   {
      unif_byte[tlb_node_count] = 0x80;  /* invariant Z write */
      tlb_nodes[tlb_node_count++] = tr_const(0);   /* H/W doesn't use this */
   }

   for (unsigned i = 0; i < V3D_MAX_RENDER_TARGETS; i++)
   {
      if (s->rt[i].type == GLSL_FB_NOT_PRESENT) continue;

      assert(s->rt[i].type == GLSL_FB_F16 ||
             s->rt[i].type == GLSL_FB_F32 ||
             s->rt[i].type == GLSL_FB_I32);

      // Read FB for advanced blending
      // Blending is not supported for integer formats
      Backflow *adv_blend_fb[4][4] = { { NULL, }, };

      if (s->adv_blend != 0)
         adv_blend_read_tlb(adv_blend_fb, s, block, i);

      if (R(i, 0) != NULL)    /* Don't write anything if output is uninitialised */
      {
         int rt_channels = 1;    /* We know R is present */
         if (G(i,0) != NULL) rt_channels++;
         if (B(i,0) != NULL) rt_channels++;
         if (A(i,0) != NULL) rt_channels++;

         /* Work around GFXH-1212 by writing 4 channels for 16 bit images with alpha */
         if (s->rt[i].alpha_16_workaround) rt_channels = 4;

         /* We set this up once per render target. The config byte is
          * emitted with the first sample that's written
          */
         int vec_sz = rt_channels;
         int swap = 0;
         if (s->rt[i].type == GLSL_FB_F16) {
            vec_sz = (rt_channels+1)/2;   /* F16 targets have half the outputs */
            swap = 1;                     /* and support the 'swap' field (bit 2) */
         }
         assert(vec_sz <= 4);

         bool block_per_sample = !V3D_VER_AT_LEAST(4,0,2,0) && block->per_sample;
         bool ms_adv_blend     = s->ms && s->adv_blend;
         bool per_sample       = block_per_sample || ms_adv_blend;
         bool shared_frag      = !block_per_sample && ms_adv_blend;

         uint8_t config_byte   = (s->rt[i].type << 6) | ((7-i) << 3) |
                                 ((!per_sample)<<2) | (swap << 1) | (vec_sz-1);

         int samples = per_sample ? 4 : 1;

         for (int sm=0; sm<samples; sm++) {
            uint32_t  out_sm = shared_frag ? 0 : sm;
            Backflow *out[4] = {R(i, out_sm), G(i, out_sm), B(i, out_sm), A(i, out_sm)};

            if (s->adv_blend != 0)
               adv_blend_blend(out, out, adv_blend_fb[sm], s->adv_blend);

            /* Pad out the output array for working around GFXH-1212 */
            for (int j=0; j<4; j++) if (out[j] == NULL) out[j] = tr_cfloat(0.0f);

            if (s->rt[i].type == GLSL_FB_F16) {
               /* Pack the values in pairs for output */
               for (int j=0; j<vec_sz; j++) {
                  assert(out[2*j] != NULL && out[2*j+1] != NULL);
                  uint32_t dataflow_age = gfx_umax(out[2*j]->age, out[2*j+1]->age);
                  dataflow_age = gfx_umax(dataflow_age, tlb_depth_age);
                  out[j] = tr_binop(BACKFLOW_VFPACK, out[2*j], out[2*j+1]);
                  out[j]->age = dataflow_age;
               }
            }

            /* Write nodes for our output, config_byte with the first */
            for (int j=0; j<vec_sz; j++) {
               unif_byte[tlb_node_count] = (j == 0 && sm == 0) ? config_byte : ~0;
               assert(out[j] != NULL);
               tlb_nodes[tlb_node_count++] = out[j];
            }
         }
      }
   }

#if !V3D_HAS_RELAXED_THRSW
   /* QPU restrictions prevent us writing nothing to the TLB. Write some fake data */
   if (s->requires_sbwait && block->first_tlb_read == NULL && tlb_node_count == 0) {
      /* Check rt[0] for the workaround. If the target is present but the output from
       * the shader uninitialised then the setting will be valid. If the target is not
       * present then the setting will be off, which is correct for the drivers default
       * RT. This would break were the default to change to 16 bits with alpha. */
      bool alpha16 = s->rt[0].alpha_16_workaround;
      int vec_sz = alpha16 ? 4 : 1;
      unif_byte[tlb_node_count] = 0x3C | (vec_sz-1);
      tlb_nodes[tlb_node_count++] = tr_const(0);
      for (int i=1; i<vec_sz; i++) {
         unif_byte[tlb_node_count] = ~0u;
         tlb_nodes[tlb_node_count++] = tr_const(0);
      }
   }
#endif

   Backflow *first_write = NULL;
   Backflow *last_write  = NULL;
   assert(tlb_node_count <= countof(tlb_nodes));
   for (unsigned i = 0; i < tlb_node_count; i++)
   {
      uint32_t dataflow_age = tlb_nodes[i]->age;
      if (last_write)
         dataflow_age = gfx_umax(dataflow_age, last_write->age);
      if (unif_byte[i] != ~0u)
      {
         uint32_t unif = 0;
         uint32_t unif_shift = 0;
         /* Read ahead to find the next (up to) 4 unif bytes */
         for (unsigned j = i; j < tlb_node_count && unif_shift < 32; j++)
         {
            if (unif_byte[j] != ~0u)
            {
               assert(unif_byte[j] <= 0xff);
               unif |= unif_byte[j] << unif_shift;
               unif_shift += 8;
               unif_byte[j] = ~0u;  /* stops us adding it next time */
            }
         }
         /* Unused config entries must be all 1s */
         if (unif_shift < 32)
            unif |= 0xffffffff << unif_shift;

         if (unif != ~0u) {
            last_write = tr_mov_to_reg_io(REG_MAGIC_TLBU, tlb_nodes[i], last_write);
            last_write->unif_type = BACKEND_UNIFORM_LITERAL;
            last_write->unif = unif;
         } else {
            /* If the config is the default, don't write it at all */
            last_write = tr_mov_to_reg_io(REG_MAGIC_TLB, tlb_nodes[i], last_write);
         }
      }
      else
      {
         last_write = tr_mov_to_reg_io(REG_MAGIC_TLB, tlb_nodes[i], last_write);
      }
      last_write->age = dataflow_age;
      if (first_write == NULL) first_write = last_write;
   }

   block->first_tlb_write = first_write;

   if (first_write != NULL)
      glsl_iodep(first_write, cov_dep);

   *writes_z_out   = invariant_z_write || full_z_write;
   *ez_disable_out = full_z_write || !s->early_fragment_tests;

   if (last_write != NULL)
      glsl_backflow_chain_append(&block->iodeps, last_write);

#undef A
#undef B
#undef G
#undef R
}

static void collect_shader_outputs(SchedBlock *block, int block_id, const IRShader *sh,
                                   const LinkMap *link_map, Backflow **nodes, bool out_per_sample)
{
   for (int i=0; i<link_map->num_outs; i++) {
      if (link_map->outs[i] == -1) continue;

      int samples = (!V3D_VER_AT_LEAST(4,0,2,0) && block->per_sample) ? 4 : 1;
      int out_samples = out_per_sample ? 4 : 1;
      IROutput *o = &sh->outputs[link_map->outs[i]];
      if (o->block == block_id) {
         for (int j=0; j<samples; j++) {
            nodes[out_samples*i+j] = block->outputs[samples*o->output + j];
         }
      } else {
         nodes[out_samples*i] = tr_external(o->block, o->output, &block->external_list);
      }
   }
}

void glsl_fragment_backend(
   SchedBlock *block,
   int block_id,
   const IRShader *sh,
   const LinkMap *link_map,
   const FragmentBackendState *s,
   bool *does_discard_out,
   bool *does_z_change_out)
{
   /* If block->per_sample the outputs are at 4*id + sample_num */
   Backflow *bnodes[4*DF_BLOB_FRAGMENT_COUNT] = { 0, };
   collect_shader_outputs(block, block_id, sh, link_map, bnodes, true);

   fragment_backend(s, block,
                    bnodes + DF_FNODE_R(0),
                    bnodes[4*DF_FNODE_DISCARD],
                    bnodes[4*DF_FNODE_DEPTH],
                    bnodes[4*DF_FNODE_SAMPLE_MASK],
                    does_discard_out, does_z_change_out);
}
