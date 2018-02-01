/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "ControlListBuilder.h"
#include "CommandBuffer.h"
#include "Command.h"
#include "Options.h"

#include "libs/core/v3d/v3d_limits.h"
#include "libs/core/v3d/v3d_cl.h"
#include "libs/core/v3d/v3d_gen.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/core/v3d/v3d_supertile.h"
#include "libs/core/v3d/v3d_tile_size.h"

#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/lfmt/lfmt.h"

#include "libs/core/gfx_buffer/gfx_buffer_translate_v3d.h"

#include "libs/util/gfx_util/gfx_util_morton.h"

#include <algorithm>

namespace bvk {

void ControlListBuilder::SetCurrentControlList(ControlList *l)
{
   // Lazily create the first device command block when the first command
   // list is started. This prevents potentially wasting a block in the
   // command buffer transient state if no renderpass command lists are
   // needed (i.e. only transfer commands being executed).
   if (m_curDeviceBlock == nullptr)
      m_curDeviceBlock = m_cmdBuf->AddDeviceCmdBlock();

   m_curControlList = l;
}

void ControlListBuilder::ExpandDeviceMemory()
{
   // Start a new block and add a jump
   DevMemCmdBlock *newBlock = m_cmdBuf->AddDeviceCmdBlock();
   m_curDeviceBlock->AddJumpTo(newBlock);
   m_curDeviceBlock = newBlock;
}

const uint32_t initialTileAllocBlockSize = V3D_TILE_ALLOC_BLOCK_SIZE_MIN;
const uint32_t tileAllocBlockSize = 128;

void ControlListBuilder::StartBinJobCL(
      CmdBinRenderJobObj *brJob,
      ControlList        *binList,
      uint32_t            numRenderTargets,
      bool                multisample,
      v3d_rt_bpp_t        maxBpp)
{
   // Must always have at least one render target, even if it's a dummy one
   assert(numRenderTargets > 0);

   SetCurrentControlList(binList);
   binList->SetStart(*m_curDeviceBlock);
   brJob->SetBinStart(binList->Start());

   v3d_cl_tile_binning_mode_cfg(CLPtr(),
      v3d_translate_tile_alloc_block_size(initialTileAllocBlockSize),
      v3d_translate_tile_alloc_block_size(tileAllocBlockSize),
      numRenderTargets, maxBpp, multisample,
      /*double_buffer=*/false,
      m_numPixelsX, m_numPixelsY);

   v3d_cl_clear_vcd_cache(CLPtr());

   v3d_cl_start_tile_binning(CLPtr());

   // Reset transform feedback counter to 0 (0 here is a special value meaning
   // reset to 0 -- 1 does not mean reset to 1!)
   v3d_cl_wait_transform_feedback(CLPtr(), 0);

   v3d_cl_occlusion_query_counter_enable(CLPtr(), 0);
}

void ControlListBuilder::InsertNVShaderRenderState(
      const VkRect2D &clipRect,
      bool            updateDepth,
      bool            updateStencil,
      uint32_t        colorWriteMasks)
{
   v3d_cl_viewport_offset(CLPtr(), 0, 0, 0, 0);
   v3d_cl_clip(CLPtr(),
         clipRect.offset.x,
         clipRect.offset.y,
         clipRect.extent.width,
         clipRect.extent.height);

   v3d_cl_clipz(CLPtr(), 0.0f, 1.0f);
   v3d_cl_depth_offset(CLPtr(), 0.0f, 0.0f, 1.0f);

   v3d_cl_zero_all_flatshade_flags(CLPtr());
   v3d_cl_zero_all_centroid_flags(CLPtr());
   v3d_cl_color_wmasks(CLPtr(), colorWriteMasks);

   v3d_cl_cfg_bits(CLPtr(),
         true,                     // front_prims
         true,                     // back_prims
         false,                    // cwise_is_front
         false,                    // depth_offset
         false,                    // aa_lines
         V3D_MS_1X,                // rast_oversample
         false,                    // cov_pipe
         V3D_COV_UPDATE_NONZERO,   // cov_update
         false,                    // wireframe_tris
         V3D_COMPARE_FUNC_ALWAYS,  // depth_test
         updateDepth,              // depth_update
         false,                    // ez
         false,                    // ez_update
         updateStencil,            // stencil
         false,                    // blend
         V3D_WIREFRAME_MODE_LINES, // wireframe_mode
         false                     // d3d_prov_vtx
   );
}

void ControlListBuilder::EndBinJobCL(
      CmdBinRenderJobObj *brJob,
      ControlList        *binList,
      v3d_barrier_flags   syncFlags)
{
   v3d_cl_flush(CLPtr());

   binList->SetEnd(*m_curDeviceBlock);
   SetCurrentControlList(nullptr);
   brJob->SetBinEnd(binList->End());

   brJob->m_brDetails.min_initial_bin_block_size = gfx_uround_up_p2(
      m_numTilesX * m_numTilesY *
      initialTileAllocBlockSize, V3D_TILE_ALLOC_ALIGN) +
      // If HW runs out of memory immediately it may not raise an out of
      // memory interrupt (it only raises one on the transition from having
      // memory to not having memory). Avoid this by ensuring it starts with
      // at least one block.
      V3D_TILE_ALLOC_GRANULARITY;
   brJob->m_brDetails.bin_tile_state_size = m_numTilesX * m_numTilesY * V3D_TILE_STATE_SIZE;

   // Set binner barrier flags
   const v3d_barrier_flags defaultFlags =
         V3D_BARRIER_CLE_CL_READ
      |  V3D_BARRIER_CLE_SHADREC_READ
      |  V3D_BARRIER_CLE_PRIMIND_READ
      |  V3D_BARRIER_CLE_DRAWREC_READ
      |  V3D_BARRIER_VCD_READ
      |  V3D_BARRIER_QPU_INSTR_READ
      |  V3D_BARRIER_QPU_UNIF_READ
      |  V3D_BARRIER_PTB_TILESTATE_WRITE;

   syncFlags |= defaultFlags;

   const V3D_HUB_IDENT_T* hub_ident = v3d_scheduler_get_hub_identity();
   brJob->m_brDetails.bin_cache_ops  = v3d_barrier_cache_flushes(V3D_BARRIER_MEMORY_WRITE, syncFlags, false, hub_ident);
   brJob->m_brDetails.bin_cache_ops &= ~V3D_CACHE_CLEAR_VCD; // VCD cache flushed in the control list
   brJob->m_brDetails.bin_cache_ops |= v3d_barrier_cache_cleans(syncFlags, V3D_BARRIER_MEMORY_READ, false, hub_ident);
}

void ControlListBuilder::StartRenderJobCL(
      CmdBinRenderJobObj *brJob,
      ControlList        *renderList)
{
   SetCurrentControlList(renderList);
   renderList->SetStart(*m_curDeviceBlock);

   brJob->SetRenderStart(renderList->Start());
}

void ControlListBuilder::InsertInitialTLBClear(
      bool doubleBuffer,
      bool renderTargets,
      bool depthStencil)
{
   for (unsigned i = 0; i != (doubleBuffer ? 2 : 1); ++i)
   {
      v3d_cl_tile_coords(CLPtr(), 0, 0);
      v3d_cl_end_loads(CLPtr());
      v3d_cl_store(CLPtr(), // Dummy store
         V3D_LDST_BUF_NONE,
         V3D_MEMORY_FORMAT_RASTER,
         /*flipy=*/false,
         V3D_DITHER_OFF,
         V3D_DECIMATE_SAMPLE0,
         V3D_PIXEL_FORMAT_SRGB8_ALPHA8,
         /*clear=*/false,
         /*chan_reverse=*/false,
         /*rb_swap=*/false,
         /*stride=*/0,
         /*height=*/0,
         /*addr=*/0);
      v3d_cl_clear(CLPtr(), renderTargets, depthStencil);
      v3d_cl_end_tile(CLPtr());
   }
}

void ControlListBuilder::EndRenderJobCL(
      CmdBinRenderJobObj *brJob,
      ControlList        *renderList,
      const ControlList  &genTileList,
      uint32_t            core,
      v3d_barrier_flags   syncFlags)
{
   uint32_t numCores = Options::renderSubjobs;

   numCores = 1;  // TODO - multi-core

   assert(core < numCores);

   v3d_cl_clear_vcd_cache(CLPtr());

   v3d_cl_tile_list_initial_block_size(CLPtr(),
      v3d_translate_tile_alloc_block_size(initialTileAllocBlockSize),
      /*chain=*/true);

   // Calculate the size for our supertiles
   uint32_t supertileWidthInTiles  = 0;
   uint32_t supertileHeightInTiles = 0;

   v3d_choose_supertile_sizes(m_numTilesX, m_numTilesY,
                              Options::minSupertileW, Options::minSupertileH,
                              Options::maxSupertiles,
                              &supertileWidthInTiles, &supertileHeightInTiles);

   uint32_t frameWidthInSupertiles  = gfx_udiv_round_up(m_numTilesX, supertileWidthInTiles);
   uint32_t frameHeightInSupertiles = gfx_udiv_round_up(m_numTilesY, supertileHeightInTiles);

   v3d_cl_multicore_rendering_supertile_cfg(CLPtr(),
      supertileWidthInTiles,
      supertileHeightInTiles,
      frameWidthInSupertiles,
      frameHeightInSupertiles,
      m_numTilesX,
      m_numTilesY,
      numCores > 1,
      V3D_SUPERTILE_ORDER_RASTER, // TODO enable morton
      brJob->m_binSubjobs.size());

   v3d_cl_generic_tile_list(CLPtr(), genTileList.Start(), genTileList.End());

   bool isolateFrame = Options::isolateFrame != ~0u;
   if (isolateFrame && Options::isolateFrame == CmdBinRenderJobObj::Task::BinRenderJobCounter())
   {
      // isolate requested supertile
      v3d_cl_supertile_coords(CLPtr(),
         gfx_umin(Options::isolateSupertileX, frameWidthInSupertiles - 1u),
         gfx_umin(Options::isolateSupertileY, frameHeightInSupertiles - 1u));
   }
   else
   {
      uint32_t mortonFlags, beginSupertile, endSupertile;
      v3d_supertile_range(&mortonFlags, &beginSupertile, &endSupertile,
         numCores, core,
         frameWidthInSupertiles, frameHeightInSupertiles,
         Options::partitionSupertilesInSW,
         Options::allCoresSameStOrder);

      GFX_MORTON_STATE_T morton;
      gfx_morton_init(&morton, frameWidthInSupertiles, frameHeightInSupertiles, mortonFlags);

      uint32_t x, y;
      for (unsigned i = 0; gfx_morton_next(&morton, &x, &y, NULL); ++i)
         if (i >= beginSupertile && i < endSupertile)
            v3d_cl_supertile_coords(CLPtr(), x, y);
   }

   // Ensure render list end address is not 4-byte aligned, or it might match
   // the start address of another buffer. This could cause problems if the
   // other buffer also contained a control list as we could hit the render list
   // end address before actually getting to the end of the render list.
   if ((m_curDeviceBlock->CurPhys() % 4) == 0)
      v3d_cl_nop(CLPtr());

   v3d_cl_end_render(CLPtr());

   renderList->SetEnd(*m_curDeviceBlock);
   brJob->SetRenderEnd(renderList->End());
   SetCurrentControlList(nullptr);

   // Setup render barrier flags
   const v3d_barrier_flags defaultSyncFlags =
         V3D_BARRIER_CLE_CL_READ
      |  V3D_BARRIER_CLE_SHADREC_READ
      |  V3D_BARRIER_VCD_READ
      |  V3D_BARRIER_QPU_INSTR_READ
      |  V3D_BARRIER_QPU_UNIF_READ;

   const V3D_HUB_IDENT_T* hub_ident = v3d_scheduler_get_hub_identity();
   syncFlags |= defaultSyncFlags;
   brJob->m_brDetails.render_cache_ops  = v3d_barrier_cache_flushes(V3D_BARRIER_MEMORY_WRITE,
                                                                    syncFlags, false, hub_ident);
   brJob->m_brDetails.render_cache_ops &= ~V3D_CACHE_CLEAR_VCD; // VCD cache flushed in the control list
   brJob->m_brDetails.render_cache_ops |= v3d_barrier_cache_cleans(syncFlags, V3D_BARRIER_MEMORY_READ,
                                                                   false, hub_ident);
}

void ControlListBuilder::AddTileListBranches(CmdBinRenderJobObj *brJob)
{
   // Must set base instance to 0 at start of tile; PTB assumes this! GFXH-1455
   v3d_cl_set_instance_id(CLPtr(), 0);

   if (brJob->m_numZPrepassBins > 0)
   {
      v3d_cl_enable_z_only(CLPtr());
      for (unsigned i = 0; i != brJob->m_numZPrepassBins; ++i)
         v3d_cl_branch_implicit_tile(CLPtr(), i);
      v3d_cl_disable_z_only(CLPtr());
   }

   for (unsigned i = 0; i != brJob->m_binSubjobs.size(); ++i)
      v3d_cl_branch_implicit_tile(CLPtr(), i);
}

// Create the generic tile list (must be done after binning control lists
// have been created - why?)
void ControlListBuilder::CreateGenericTileList(CmdBinRenderJobObj *brJob, ControlList &gtl)
{
   gtl.SetStart(*m_curDeviceBlock);
   SetCurrentControlList(&gtl);

   v3d_cl_implicit_tile_coords(CLPtr());

   AddTileListLoads();
   v3d_cl_end_loads(CLPtr());
   AddTileListBranches(brJob);
   AddTileListStores();
   v3d_cl_end_tile(CLPtr());

   // Add the final return
   v3d_cl_return(CLPtr());

   gtl.SetEnd(*m_curDeviceBlock);
   SetCurrentControlList(nullptr);

   // Reset forced load & store flags now
   if (m_forceRTLoad)
      SetForceRTLoads(false);
   if (m_forceRTStore)
      SetForceRTStores(false);
}

void ControlListBuilder::CreateMasterControlLists(
   CmdBinRenderJobObj *brJob,
   v3d_barrier_flags   binSyncFlags,
   v3d_barrier_flags   renderSyncFlags)
{
   ControlList genTileList;
   CreateBinnerControlList(brJob, binSyncFlags);
   CreateGenericTileList(brJob, genTileList);
   CreateRenderControlList(brJob, genTileList, renderSyncFlags);
}


} // namespace bvk
