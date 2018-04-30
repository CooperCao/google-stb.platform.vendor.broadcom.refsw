/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Common.h"
#include "Allocating.h"
#include "DeviceMemory.h"
#include "Formats.h"
#include "CmdBufState.h"
#include "ForwardDecl.h"
#include "DevMemCmdBlock.h"

// All subclasses and the implementation need these so we might as well include
// them right now.
#include "libs/core/v3d/v3d_cl.h"
#include "libs/core/v3d/v3d_barrier.h"
#include "libs/core/v3d/v3d_limits.h"
#include "libs/core/gfx_buffer/gfx_buffer.h"

namespace bvk {

class CmdBinRenderJobObj;
class SysMemCmdBlock;
class DevMemCmdBlock;
class DevMemDataBlock;

// Base class for command buffer control list builder helper classes -
//    - provides a common implementation of device memory block management for
//      control list generation.
//    - provides helpers to construct the master bin and render control lists
class ControlListBuilder : public Allocating
{
public:
   class ControlList
   {
   public:
      void SetStart(DevMemCmdBlock &devBlock) { m_startAddr = devBlock.CurPhys(); }
      void SetEnd(DevMemCmdBlock &devBlock)   { m_endAddr = devBlock.CurPhys();   }

      v3d_addr_t Start() const { return m_startAddr; }
      v3d_addr_t End() const   { return m_endAddr;   }

   private:
      v3d_addr_t  m_startAddr = 0;
      v3d_addr_t  m_endAddr = 0;
   };

   ControlListBuilder(const VkAllocationCallbacks *pCallbacks, CommandBuffer *cmdBuf):
         Allocating(pCallbacks), m_cmdBuf(cmdBuf) {}

   virtual ~ControlListBuilder() {}

   // Start building a control list. Until this is called CLPtr() or accessing
   // m_curDeviceBlock directly is undefined.
   void SetCurrentControlList(ControlList *l);

   // Note: this allows the block's m_curPtr to be modified externally.
   // Used by the control list generators.
   uint8_t **CLPtr(size_t addSize = V3D_CL_MAX_INSTR_SIZE)
   {
      assert(m_curControlList != nullptr);

      if (m_curDeviceBlock->CanFit(addSize))
         ExpandDeviceMemory();

      return m_curDeviceBlock->CurDataPtr();
   }

   uint8_t **CLPtrFinal(size_t addSize = V3D_CL_MAX_INSTR_SIZE) noexcept
   {
      assert(m_curControlList != nullptr);
      assert(m_curDeviceBlock->CanFitFinal(addSize));
      return m_curDeviceBlock->CurDataPtr();
   }

   // Helper for inserting a simple NV shader + drawcall with a default
   // render state into a bin control list. This needs to be public for the
   // command buffer clear attachment implementation.
   void InsertNVShaderRenderState(
         const VkRect2D &clipRect,
         bool            updateDepth,
         bool            updateStencil,
         uint32_t        colorWriteMasks);

   // Create master bin and render control lists and add them to the binRenderJob
   void CreateMasterControlLists(
         CmdBinRenderJobObj *brJob,
         v3d_barrier_flags   binSyncFlags = V3D_BARRIER_NO_ACCESS,
         v3d_barrier_flags   renderSyncFlags = V3D_BARRIER_NO_ACCESS);

   void SetForceRTLoads(bool tf)
   {
      assert(m_forceRTLoad != tf);
      m_forceRTLoad = tf;
   }

   void SetForceRTStores(bool tf)
   {
      assert(m_forceRTStore != tf);
      m_forceRTStore = tf;
   }

protected:
   // Custom bin and render control list generation, implemented by the
   // specialized builder sub-classes
   virtual void CreateBinnerControlList(CmdBinRenderJobObj *brJob, v3d_barrier_flags syncFlags) = 0;
   virtual void CreateRenderControlList(CmdBinRenderJobObj *brJob, const ControlList  &genTileList,
                                        v3d_barrier_flags syncFlags, bool allowEarlyDSClear) = 0;

   // Bin job control list helpers
   void StartBinJobCL(
         CmdBinRenderJobObj *brJob,
         ControlList        *binList,
         uint32_t            numRenderTargets,
         bool                multisample,
         v3d_rt_bpp_t        maxBpp);

   void EndBinJobCL(
         CmdBinRenderJobObj *brJob,
         ControlList        *binList,
         v3d_barrier_flags   syncFlags);

   // Render job control list helpers
   void StartRenderJobCL(
         CmdBinRenderJobObj *brJob,
         ControlList        *renderList);

   void InsertDummyTiles(
         bool clearDoubleBuffer,
         bool clearRenderTargets,
         bool clearDepthStencil);

   void EndRenderJobCL(
         CmdBinRenderJobObj *brJob,
         ControlList        *renderList,
         const ControlList  &genTileList,
         uint32_t            core,
         v3d_barrier_flags   syncFlags);

private:
   // Generic Tile List generation
   void CreateGenericTileList(CmdBinRenderJobObj *brJob, ControlList &gtl, bool *allowEarlyDSClear);
   void AddTileListBranches(CmdBinRenderJobObj *brJob);

protected:
   // Custom Tile List load/store generation, implemented by the specialized
   // builder sub-classes.
   virtual void AddTileListLoads(bool *allowEarlyDSClear) = 0;
   virtual void AddTileListStores(bool *allowEarlyDSClear) = 0;

private:
   void ExpandDeviceMemory();

protected:
   // Parent command buffer, to obtain device memory blocks from
   CommandBuffer     *m_cmdBuf;
   ControlList       *m_curControlList = nullptr;

public:
   // Current dev mem block pointers
   DevMemCmdBlock    *m_curDeviceBlock     = nullptr;

   // Framebuffer or Image subresource cached values
   uint32_t           m_numPixelsX = 0;
   uint32_t           m_numPixelsY = 0;
   uint32_t           m_numTilesX = 0;
   uint32_t           m_numTilesY = 0;

   // Flags to control forced render target loads and stores (for intermediate flushes)
   bool               m_forceRTLoad  = false;
   bool               m_forceRTStore = false;
};

} // namespace bvk
