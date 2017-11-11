/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#if KHRN_DEBUG

#include "libs/platform/v3d_scheduler.h"
#include "libs/core/v3d/v3d_addr.h"
#include "libs/tools/autoclif/autoclif.h"

namespace bvk {

class ClifRecorder
{
public:
   ClifRecorder(const V3D_BIN_RENDER_INFO_T &brInfo);
   void Record();

private:
   const char *GetFilename();
   void RecordBin(autoclif &ac, const autoclif_addr *tile_alloc_addrs, v3d_size_t tile_alloc_size);
   void RecordRender(autoclif &ac, const autoclif_addr *tile_list_base_addrs);

private:
   const V3D_BIN_RENDER_INFO_T   &m_info;
};

} // namespace bvk

#endif // KHRN_DEBUG