/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "TextureLookup.h"
#include "DflowScalars.h"
#include "DflowLibrary.h"
#include "DflowBuilder.h"
#include "Module.h"

#ifdef WIN32
#pragma warning(disable : 4800)  // From autogen'd code
#endif

#include "glsl_dataflow_image.h"
#include "glsl_primitive_types.auto.h"

namespace bvk
{

static uint32_t NumCoords(spv::Dim dim)
{
   switch (dim)
   {
   case spv::Dim::Dim1D:        return 1;
   case spv::Dim::Dim2D:        return 2;
   case spv::Dim::Dim3D:        return 3;
   case spv::Dim::Cube:         return 3;
   case spv::Dim::Rect:         return 2;
   case spv::Dim::Buffer:       return 1;
   case spv::Dim::SubpassData:  return 2;
   default:                 unreachable();
   }
}

TextureLookup::TextureLookup(DflowBuilder &builder,
               const ImageOperands *imageOperands,
               const DflowScalars &coords, const DflowScalars &sampledImage,
               const NodeTypeImage *imageType,
               const Node *dref, bool project,
               bool fetch, bool lodQuery) :
   m_builder(builder),
   m_imageOperands(imageOperands),
   m_coords(coords),
   m_image(sampledImage[0]),
   m_sampler(sampledImage.Size() == 2 ? sampledImage[1] : Dflow()),
   m_imageType(imageType),
   m_fetch(fetch)
{
   m_offset = CalcTexOffset(imageOperands);

   Dflow projRecipDiv;
   MakeTexCoords(project, lodQuery, &projRecipDiv);

   if (dref != nullptr)
   {
      assert(builder.GetDataflow(dref).Size() == 1);
      m_dref = builder.GetDataflow(dref)[0];

      if (project)
         m_dref = m_dref * projRecipDiv;
   }
}

TextureLookup::TextureLookup(
      DflowBuilder &builder,
      const ImageOperands *imageOperands,
      const Node *coord, const Node *sampledImage,
      const Node *dref, bool project, bool fetch, bool lodQuery) :

   TextureLookup(builder, imageOperands,
                 builder.GetDataflow(coord),
                 builder.GetDataflow(sampledImage),
                 DflowBuilder::GetImageType(sampledImage),
                 dref, project, fetch, lodQuery)
{
}

void TextureLookup::ProjectCoords(DflowScalars &newCoords, uint32_t numCoordsOut, Dflow *recipDiv)
{
   assert(newCoords.Size() >= numCoordsOut);

   // Divide by last actual component for projection
   *recipDiv = Dflow::UnaryOp(DATAFLOW_RCP, newCoords[numCoordsOut]);
   for (uint32_t i = 0; i < numCoordsOut; i++)
      newCoords[i] = newCoords[i] * (*recipDiv);

   // Null out the original divisor
   newCoords[numCoordsOut] = Dflow();
}

void TextureLookup::MakeMSFetchCoords(DflowScalars &newCoords) const
{
   auto two = Dflow::Int(2);
   newCoords[0] = newCoords[0] * two;
   newCoords[1] = newCoords[1] * two;

   if (m_imageOperands)
   {
      const Node *sample = m_imageOperands->GetSample();
      if (sample)
      {
         auto one = Dflow::Int(1);
         const DflowScalars &sampleIndex = m_builder.GetDataflow(sample);
         newCoords[0] = newCoords[0] + (sampleIndex[0] & one);
         newCoords[1] = newCoords[1] + (sampleIndex[0] >> one);
      }
   }
}

void TextureLookup::MakeTexCoords(bool project, bool lodQuery, Dflow *recipDiv)
{
   bool arrayed = m_imageType->GetArrayed();
   bool cube    = m_imageType->GetDim() == spv::Dim::Cube;

   if (m_imageType->GetDim() == spv::Dim::SubpassData)
   {
      // InputAttachments have implicit tex coords
      assert(m_fetch && !cube && !arrayed);

      m_coords = reinterpi(DflowScalars::NullaryOp(m_coords.GetAllocator(),
                           { DATAFLOW_FRAG_GET_X_UINT, DATAFLOW_FRAG_GET_Y_UINT }));
   }

   // Notes: Projective image instructions are not supported on Arrayed images.
   //        Cube-arrays when used in fetch mode only have 3 coords, not 4 (x, y, f * 6 + a).
   //        Cube-arrays when used normally have 4 coords (x, y, z, a).
   uint32_t numCoords = NumCoords(m_imageType->GetDim());
   if ((arrayed && !lodQuery && !(m_fetch && cube)) || project)
      numCoords++;   // Include the array index or the divisor

   // Make a deep copy of coords so we can swizzle the order if needed
   DflowScalars newCoords(m_coords.GetAllocator(), 4);
   for (uint32_t i = 0; i < numCoords; i++)
      newCoords[i] = m_coords[i];

   // Multi-sampled fetch needs to double the coords and incorporate the 'sample index'
   if (m_fetch && m_imageType->GetMS())
      MakeMSFetchCoords(newCoords);

   if (project)
      ProjectCoords(newCoords, numCoords-1, recipDiv);

   // Need to scale coords for cube lookups (except when using integer coords)
   if (!m_fetch && cube)
      newCoords = prepareCube4Coord(newCoords);

   if (arrayed)
   {
      // The backend expects to find the array index in the last texture coord,
      // so swizzle appropriately.
      switch (m_imageType->GetDim())
      {
      case spv::Dim::Dim1D:    std::swap(newCoords[1], newCoords[3]); break;
      case spv::Dim::Dim2D:    std::swap(newCoords[2], newCoords[3]); break;
      default: break;
      }
   }

   // Cube lookups in fetch mode (int coords) are treated as 2d-arrays, so we need to get
   // the array index into the W coord.
   if (m_fetch && cube)
      std::swap(newCoords[2], newCoords[3]);

   m_coords = newCoords;
}

static Dflow PackTextureOffsets(const DflowScalars &values)
{
   // Pack the offsets into sequential 4-bit fields in 'packed'
   uint32_t num = values.Size();
   assert(num > 0);

   Dflow mask = Dflow::UInt(0xF);
   Dflow packed = values[0] & mask;

   for (uint32_t i = 1; i < num; i++)
      packed = packed | ((values[i] & mask) << Dflow::UInt(i * 4));

   return packed;
}

static uint32_t ImageTextureBits(const NodeTypeImage *imageType)
{
   return (imageType->GetDim() == spv::Dim::Cube) ? DF_TEXBITS_CUBE : DF_TEXBITS_NONE;
}

Dflow TextureLookup::CalcTexOffset(const ImageOperands *iOps) const
{
   Dflow result;

   if (iOps)
   {
      const Node *constOffset = iOps->GetConstOffset();
      if (constOffset)
         result = PackTextureOffsets(m_builder.GetDataflow(constOffset));

      const Node *offset = iOps->GetOffset();
      if (offset)
         result = PackTextureOffsets(m_builder.GetDataflow(offset));

      const Node *constOffsets = iOps->GetConstOffsets();
      if (constOffsets)
         result = PackTextureOffsets(m_builder.GetDataflow(constOffsets));
   }

   return result;
}

Dflow TextureLookup::CalcTexBias(const ImageOperands *iOps) const
{
   Dflow result;
   if (iOps)
   {
      const Node *bias = iOps->GetBias();
      if (bias)
         result = m_builder.GetDataflow(bias)[0];
   }
   return result;
}

Dflow TextureLookup::CalcTexLod(const ImageOperands *iOps) const
{
   Dflow result;
   if (iOps)
   {
      const Node *lod = iOps->GetLod();
      if (lod)
         result = m_builder.GetDataflow(lod)[0];
   }
   return result;
}

Dflow TextureLookup::CalcTexGrad() const
{
   Dflow result;
   if (m_imageOperands)
   {
      const Node *niDx = m_imageOperands->GetGradDx();
      const Node *niDy = m_imageOperands->GetGradDy();

      if (niDx && niDy)
      {
         const DflowScalars &dPdx = m_builder.GetDataflow(niDx);
         const DflowScalars &dPdy = m_builder.GetDataflow(niDy);

         uint32_t numScalars = dPdx.Size();
         assert(dPdy.Size() == numScalars);

         DflowScalars tSize  = DflowScalars::TextureSize(m_builder.GetAllocator(), numScalars, m_image);
         bool         isCube = m_imageType->GetDim() == spv::Dim::Cube;

         result = calculateLod(tSize, m_coords, dPdx, dPdy, isCube);
      }
   }
   return result;
}

#if V3D_VER_AT_LEAST(4,2,13,0)
DflowScalars TextureLookup::LodQuery() const
{
   uint32_t bits = ImageTextureBits(m_imageType) | DF_TEXBITS_LOD_QUERY;
   Dflow    bias = CalcTexBias(m_imageOperands);

   return DflowScalars::Texture(m_builder.GetAllocator(),
                                m_coords, m_dref, bias,
                                m_offset, m_image, m_sampler, bits);
}
#endif

DflowScalars TextureLookup::ImplicitLodLookup() const
{
   uint32_t bits = ImageTextureBits(m_imageType);
   Dflow    bias = CalcTexBias(m_imageOperands);

   return DflowScalars::Texture(m_builder.GetAllocator(), m_coords, m_dref, bias,
                                m_offset, m_image, m_sampler, bits);
}

DflowScalars TextureLookup::ExplicitLodLookup() const
{
   uint32_t bits = ImageTextureBits(m_imageType) | DF_TEXBITS_BSLOD;
   Dflow    lod  = CalcTexLod(m_imageOperands);

   // Maybe grad instead - can't be both
   if (lod.IsNull())
      lod = CalcTexGrad();

   return DflowScalars::Texture(m_builder.GetAllocator(), m_coords, m_dref, lod,
                                m_offset, m_image, m_sampler, bits);
}

DflowScalars TextureLookup::GatherLookup(uint32_t component) const
{
   uint32_t bits = ImageTextureBits(m_imageType) | DF_TEXBITS_GATHER;
   Dflow    lod  = CalcTexLod(m_imageOperands);

   if (m_imageOperands && m_imageOperands->GetConstOffsets())
      bits |= DF_TEXBITS_I_OFF;

   // Hardware requires that we specify a LoD, but the SPIR-V defaults to 0
   if (lod.IsNull())
      lod = Dflow::Int(0);

   bits |= (component & 0x3) << DF_TEXBITS_GATHER_COMP_SHIFT;

   auto result = DflowScalars::Texture(m_builder.GetAllocator(), m_coords, m_dref, lod,
                                       m_offset, m_image, m_sampler, bits);

   if (bits & DF_TEXBITS_I_OFF)
      return result;
   else
      // The GLSL texture gadget returns the samples in a
      // different order to that which Vulkan expects.
      return result.Swizzle(2, 3, 1, 0);
}

DflowScalars TextureLookup::ImageFetch() const
{
   // Note : we MUST not add DF_TEXBITS_CUBE to bits here even when dealing with cube maps.
   // Since ImageFetch only deals with integer coords, the backend must see the cube coords
   // as a 2d-array (x, y, _, face)
   uint32_t bits = DF_TEXBITS_FETCH;
   Dflow    lod  = CalcTexLod(m_imageOperands);

   return DflowScalars::Texture(m_builder.GetAllocator(), m_coords, m_dref, lod,
                                m_offset, m_image, Dflow(), bits);
}

void TextureLookup::ImageWrite(const DflowScalars &data, BasicBlockHandle block) const
{
   spv::ImageFormat fmt  = m_imageType->GetImageFormat();
   Dflow            addr = Dflow::CreateImageWriteAddress(m_image, m_coords);

   // We need to pack the data into the appropriate in-memory representation
   Dflow d = Dflow::PackImageData(ToFormatQualifier(fmt), data);

   Dflow::Atomic(DATAFLOW_ADDRESS_STORE, /*type=*/DF_VOID, addr, d, block);
}

DflowScalars TextureLookup::Atomic(DataflowFlavour op, const DflowScalars &data,
                                   BasicBlockHandle block) const
{
   assert(data.Size() == 4);

   DflowScalars vs;
   if (op == DATAFLOW_ATOMIC_CMPXCHG)
      vs = data.Swizzle(1, 0, 2, 3);
   else
      vs = data;

   auto addr  = Dflow::CreateImageWriteAddress(m_image, m_coords);
   auto dflow = Dflow::Atomic(op, data[0].GetType(), addr, Dflow::Vec4(vs), block);

   return DflowScalars(m_builder.GetAllocator(), dflow);
}

}
