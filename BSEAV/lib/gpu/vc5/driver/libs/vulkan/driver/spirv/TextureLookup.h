/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Dflow.h"
#include "DflowScalars.h"

namespace bvk
{

class DflowBuilder;
class Node;
class ImageOperands;
class NodeTypeImage;

class TextureLookup
{
public:
   TextureLookup(DflowBuilder &builder,
                 const ImageOperands *imageOperands,
                 const DflowScalars &coord, const DflowScalars &sampledImage,
                 const NodeTypeImage *imageType,
                 const Node *dref = nullptr, bool project = false,
                 bool fetch = false, bool lodQuery = false);

   TextureLookup(DflowBuilder &builder,
                 const ImageOperands *imageOperands,
                 const Node *coord, const Node *sampledImage,
                 const Node *dref = nullptr, bool project = false,
                 bool fetch = false, bool lodQuery = false);

#if V3D_VER_AT_LEAST(4,2,13,0)
   DflowScalars LodQuery() const;
#endif
   DflowScalars ImplicitLodLookup() const;
   DflowScalars ExplicitLodLookup() const;
   DflowScalars GatherLookup(uint32_t component) const;
   DflowScalars ImageFetch() const;
   void         ImageWrite(const DflowScalars &data, BasicBlockHandle block) const;
   DflowScalars Atomic(DataflowFlavour op, const DflowScalars &data, BasicBlockHandle block) const;

private:
   Dflow CalcTexGrad() const;
   static void ProjectCoords(DflowScalars &newCoords, uint32_t numCoords, Dflow *recipDiv);
   void MakeMSFetchCoords(DflowScalars &newCoords) const;
   void MakeTexCoords(bool project, bool lodQuery, Dflow *recipDiv);

   Dflow CalcTexOffset(const ImageOperands *iOps) const;
   Dflow CalcTexBias(const ImageOperands *iOps) const;
   Dflow CalcTexLod(const ImageOperands *iOps) const;

private:
   DflowBuilder          &m_builder;
   const ImageOperands   *m_imageOperands = nullptr;
   DflowScalars           m_coords;
   Dflow                  m_image;
   Dflow                  m_sampler;
   Dflow                  m_offset;
   const NodeTypeImage   *m_imageType = nullptr;
   Dflow                  m_dref;
   bool                   m_fetch;

};

}
