/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Dflow.h"
#include "DflowScalars.h"
#include "BasicBlock.h"
#include "libs/util/demand.h"

namespace bvk {


Dflow Dflow::Vec4(const DflowScalars &coord)
{
   Dataflow *u = coord.Size() > 0 ? coord[0].m_dflow : nullptr;
   Dataflow *v = coord.Size() > 1 ? coord[1].m_dflow : nullptr;
   Dataflow *s = coord.Size() > 2 ? coord[2].m_dflow : nullptr;
   Dataflow *t = coord.Size() > 3 ? coord[3].m_dflow : nullptr;

   return Dflow(glsl_dataflow_construct_vec4(u, v, s, t));
}

Dflow Dflow::Atomic(DataflowFlavour flavour, DataflowType type,
                    const Dflow &addr, const Dflow &data,
                    BasicBlockHandle block)
{
   Dataflow *s  = glsl_dataflow_construct_atomic(flavour, type, addr, data, /*cond=*/nullptr,
                                                 block->GetMemoryAccessChain());
   block->SetMemoryAccessChain(s);

   return Dflow(s);
}

Dflow Dflow::PackImageData(FormatQualifier f, const DflowScalars &data)
{
   return Dflow(glsl_dataflow_image_pack_data(f, data.GetDataflowArray()));
}

Dflow Dflow::CreateImageWriteAddress(const Dflow &image, const DflowScalars &coord)
{
   return Dflow(glsl_dataflow_construct_texture_addr(image, coord[0], coord[1], coord[2], coord[3]));
}

} // namespace bvk
