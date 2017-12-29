/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#if KHRN_DEBUG

#include "ClifRecorder.h"
#include "Command.h"
#include "Options.h"

#include "libs/util/demand.h"
#include "libs/util/ssprintf.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/core/vcos/include/vcos_string.h"
#include "libs/platform/gmem_memaccess.h"

LOG_DEFAULT_CAT("bvk::ClifRecorder")

namespace bvk {

ClifRecorder::ClifRecorder(const V3D_BIN_RENDER_INFO_T &brInfo) :
   m_info(brInfo)
{
}

const char *ClifRecorder::GetFilename()
{
   const char *s = Options::autoclifOnlyOneClifName;
   if (s[0] && Options::autoclifOnlyOneClifI >= 0)
      return s;

   // use "rec" for filename if not specified
   s = s[0] ? s : "rec";

   // split filename into base + ext, use .clif if no ext specified
   const char* dot = strrchr(s, '.');
   const char* ext = dot ? dot + 1 : "clif";
   size_t slen = dot ? dot - s : strlen(s);

   static char filename[256];
   size_t offset = VCOS_SAFE_SPRINTF(filename, 0, "%*.*s_%.4u.%s",
      (int)slen, (int)slen, s, CmdBinRenderJobObj::Task::BinRenderJobCounter(), ext);
   demand(offset < sizeof(filename));
   return filename;
}

void ClifRecorder::RecordBin(autoclif &ac, const autoclif_addr *tile_alloc_addrs, v3d_size_t tile_alloc_size)
{
   for (unsigned core = 0; core != m_info.bin_subjobs.num_subjobs; ++core)
   {
      ac.bin(core,
         m_info.bin_subjobs.subjobs[core].start, m_info.bin_subjobs.subjobs[core].end,
         tile_alloc_addrs[core], tile_alloc_size
#if V3D_VER_AT_LEAST(4,1,34,0)
         , ac.create_explicit_buf(ssprintf("tile_state_%u", core).c_str(),
            m_info.details.bin_tile_state_size, V3D_TILE_STATE_ALIGN)
#endif
         );
   }
   ac.wait_bin_all_cores();
}

void ClifRecorder::RecordRender(autoclif &ac, const autoclif_addr *tile_list_base_addrs)
{
   uint32_t subindex = 0;
   for (uint32_t layer = 0; layer != m_info.num_layers; ++layer)
   {
      unsigned num_subjobs_per_layer = m_info.render_subjobs.num_subjobs / m_info.num_layers;

      if (num_subjobs_per_layer > 1)
         ac.new_multicore_frame();

      autoclif_addr layer_tile_base_addrs[V3D_MAX_CORES];
      for (unsigned i = 0; i != m_info.bin_subjobs.num_subjobs; ++i)
         layer_tile_base_addrs[i] = tile_list_base_addrs[i] + layer * m_info.details.tile_alloc_layer_stride;

      for (uint32_t core = 0; core != num_subjobs_per_layer; ++core)
      {
         ac.render(core,
            m_info.render_subjobs.subjobs[subindex].start,
            m_info.render_subjobs.subjobs[subindex].end,
            m_info.bin_subjobs.num_subjobs,
            layer_tile_base_addrs
            );
         ++subindex;
      }
   }
}

void ClifRecorder::Record()
{
   v3d_scheduler_wait_all();

   int32_t  only = Options::autoclifOnlyOneClifI;
   uint32_t frameNum = CmdBinRenderJobObj::Task::BinRenderJobCounter();

   if ((only < 0) || (static_cast<uint32_t>(only) == frameNum))
   {
      log_info("Recording frame %u", frameNum);

      gmem_memaccess_ro memAccess;
      autoclif ac(&memAccess,
         /* TODO Is this ok for Vulkan? */
         /*min_qpus_per_core=*/4);

      autoclif_addr tile_alloc_addrs[V3D_MAX_CORES];
      for (unsigned i = 0; i != m_info.bin_subjobs.num_subjobs; ++i)
      {
         tile_alloc_addrs[i] = ac.create_explicit_buf(
            ssprintf("tile_alloc_%u", i).c_str(),
            Options::autoclifBinBlockSize, V3D_TILE_ALLOC_ALIGN);
      }

      RecordBin(ac, tile_alloc_addrs, Options::autoclifBinBlockSize);
      RecordRender(ac, tile_alloc_addrs);

      ac.write_clif(GetFilename());

      if (only >= 0)
      {
         log_warn("recorded frame %u, exiting", frameNum);
         fflush(NULL);
         _Exit(0);
      }
   }
   else
      log_info("not recording frame %u", frameNum);
}

} // namespace bvk

#endif // KHRN_DEBUG
