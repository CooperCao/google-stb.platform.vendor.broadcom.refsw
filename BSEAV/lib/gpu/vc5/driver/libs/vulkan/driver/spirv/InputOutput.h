/******************************************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <stdint.h>
#include <array>
#include "Nodes.h"
#include "DflowScalars.h"
#include "Qualifier.h"
#include "SymbolTypeHandle.h"

namespace bvk
{

class DflowBuilder;
class Module;
class NodeVariable;

///////////////////////////////////////////////////////////////////////////////////////////////////
class IOMap
{
public:
   static constexpr uint32_t NUM_LOCATIONS = V3D_MAX_VARYING_COMPONENTS;

   // Setters
   void Record(uint32_t location, uint32_t component, int *currentId, SymbolTypeHandle type, const QualifierDecorations &q);
   void SetSymbol(uint32_t i, SymbolHandle symbol)  { m_locs[i].m_symbol = symbol; }

   // Getters
   SymbolTypeHandle            Type(uint32_t i)       const { return m_locs[i].m_type;                           }
   int32_t                     Size(uint32_t i)       const { return m_locs[i].m_maxComponent + 1;               }
   SymbolTypeHandle            VectorType(uint32_t i) const { return SymbolTypeHandle::Vector(Type(i), Size(i)); }
   const QualifierDecorations &Qualifiers(uint32_t i) const { return m_locs[i].m_qualifiers;                     }
   int                        *Ids(uint32_t i)        const { return m_locs[i].m_ids;                            }
   SymbolHandle                Symbol(uint32_t i)     const { return m_locs[i].m_symbol;                         }

private:
   class Location
   {
   public:
      int32_t               m_maxComponent = -1;   // Highest component number used on this row
      SymbolTypeHandle      m_type;                // Type of elements in this row (homgeneity is assumed/required)
      QualifierDecorations  m_qualifiers;          // Qualifiers for this row
      SymbolHandle          m_symbol;              // Symbol assigned to this row i.e. part of the input/output interface
      int                  *m_ids       = nullptr; // Linker ids assigned to each scalar in this row
   };

   std::array<Location, NUM_LOCATIONS>  m_locs;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class IOVisitor : public NodeTypeVisitorAssert
{
public:
   IOVisitor(DflowBuilder &builder, const NodeVariable *var);

   void Visit(const NodeTypeMatrix *type) override;
   void Visit(const NodeTypeArray  *type) override;
   void Visit(const NodeTypeStruct *type) override;

protected:
   DflowBuilder   &m_builder;

   uint32_t  m_location;
   uint32_t  m_component;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class IORecorder : public IOVisitor
{
public:
   static void Record(DflowBuilder &builder, IOMap *map, uint32_t size, int *currentId, const NodeVariable *var);

   void Visit(const NodeTypeBool   *type) override;
   void Visit(const NodeTypeInt    *type) override;
   void Visit(const NodeTypeFloat  *type) override;
   void Visit(const NodeTypeVector *type) override;
   void Visit(const NodeTypeStruct *type) override;

private:
   IORecorder(DflowBuilder &builder, IOMap *map, uint32_t size, int *currentId, const NodeVariable *var);

   void Scalar(SymbolTypeHandle type);

private:
   IOMap                &m_map;
   uint32_t              m_resultIndex;
   QualifierDecorations  m_qualifiers;
   int                  *m_currentId;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class InputLoader : public IOVisitor
{
public:
   static DflowScalars Load(DflowBuilder &builder, const NodeVariable *var, BasicBlockHandle block);

   void Visit(const NodeTypeBool   *type) override;
   void Visit(const NodeTypeInt    *type) override;
   void Visit(const NodeTypeFloat  *type) override;
   void Visit(const NodeTypeVector *type) override;

private:
   InputLoader(DflowBuilder &builder, uint32_t numScalars, const NodeVariable *var, BasicBlockHandle block);

   void LoadScalar();

private:
   uint32_t          m_resultIndex;
   DflowScalars      m_result;
   BasicBlockHandle  m_block;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class OutputStorer : public IOVisitor
{
public:
   static void Store(DflowBuilder &builder, const DflowScalars &scalars, const NodeVariable *var, BasicBlockHandle block);

   void Visit(const NodeTypeBool   *type) override;
   void Visit(const NodeTypeInt    *type) override;
   void Visit(const NodeTypeFloat  *type) override;
   void Visit(const NodeTypeVector *type) override;

private:
   OutputStorer(DflowBuilder &builder, const DflowScalars &scalars, const NodeVariable *var, BasicBlockHandle block);
   void StoreScalar();

private:
   uint32_t            m_scalarsIndex;
   const DflowScalars &m_scalars;
   BasicBlockHandle    m_block;
};

}
