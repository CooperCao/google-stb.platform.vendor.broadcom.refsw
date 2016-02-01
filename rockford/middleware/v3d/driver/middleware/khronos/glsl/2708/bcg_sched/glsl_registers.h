/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#ifndef __GLSL_REGISTERS_H__
#define __GLSL_REGISTERS_H__

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_qpu_enum.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_forward.h"
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////
// DFlowRegFile
///////////////////////////////////////////////////////////////////////////////

typedef enum
{
   DFlowRegFile_ANY        = 0,
   DFlowRegFile_A_OR_ACCUM = 1,
   DFlowRegFile_B_OR_ACCUM = 2,
   DFlowRegFile_A_ONLY     = 3
} DFlowRegFile;

static INLINE DFlowRegFile DFlowRegFile_OtherFile(DFlowRegFile self, DFlowRegFile deflt)
{
   if (self == DFlowRegFile_A_OR_ACCUM)
      return DFlowRegFile_B_OR_ACCUM;

   if (self == DFlowRegFile_B_OR_ACCUM)
      return DFlowRegFile_A_OR_ACCUM;

   return deflt;
}

static INLINE const char *DFlowRegFile_GetFileString(DFlowRegFile self)
{
   switch (self)
   {
   case DFlowRegFile_ANY:        return "ANY";
   case DFlowRegFile_A_OR_ACCUM: return "A/R";
   case DFlowRegFile_B_OR_ACCUM: return "B/R";
   case DFlowRegFile_A_ONLY:     return "A_ONLY";
   }

   return "???";
}

//////////////////////////////////////////////////////////////////////////////////
// QPUResource
//////////////////////////////////////////////////////////////////////////////////

typedef struct QPUResource_s
{
   Register_Enum   m_name;
   int32_t         m_readableAtSlot;
   int32_t         m_refCount;
   DFlowNode      *m_owner;
   bool            m_underflow;  // Set if register is unreferenced below zero
} QPUResource;

void QPUResource_Constr(QPUResource *self);
void QPUResource_ConstrName(QPUResource *self, Register_Enum name);

// INLINE methods
static INLINE Register_Enum QPUResource_Name(const QPUResource *self)
{
   return self->m_name;
}

static INLINE bool QPUResource_IsReadable(const QPUResource *self, int32_t atSlot)
{
   return atSlot >= self->m_readableAtSlot;
}

static INLINE DFlowNode *QPUResource_GetOwner(const QPUResource *self)
{
   return self->m_owner;
}

static INLINE bool QPUResource_IsReferenced(const QPUResource *self)
{
   return self->m_refCount > 0;
}

static INLINE int32_t QPUResource_RefCount(const QPUResource *self)
{
   return self->m_refCount;
}

// methods
void QPUResource_SetWrittenAt(QPUResource *self, int32_t slot);
void QPUResource_SetReferenced(QPUResource *self, DFlowNode *owner, int32_t count);
void QPUResource_Unreference(QPUResource *self);
void QPUResource_FullyUnreference(QPUResource *self);

//////////////////////////////////////////////////////////////////////////////////
// QPUResources
//////////////////////////////////////////////////////////////////////////////////

typedef enum
{
   QPUResources_PREFER_ACC,
   QPUResources_PREFER_REG,
   QPUResources_FORCE_REGA,
   QPUResources_FORCE_REGB,
   QPUResources_ACC_OR_REGA,
   QPUResources_ACC_OR_REGB
} QPUResources_Preference;

typedef struct QPUResources_s
{
   QPUResource    m_resources[Register_NUM_REGISTERS];
   Register_Enum  m_maxARegister;
   Register_Enum  m_maxBRegister;
} QPUResources;

// constructor
void QPUResources_Constr(QPUResources *self, bool isFrag, bool allowThread);

// INLINE methods
static INLINE void QPUResources_Destr(QPUResources *self)
{
}

static INLINE QPUResource *QPUResources_GetResource(QPUResources *self, Register_Enum reg)
{
   return &self->m_resources[reg];
}

static INLINE const QPUResource *QPUResources_GetResourceConst(const QPUResources *self, Register_Enum reg)
{
   return &self->m_resources[reg];
}

static INLINE bool QPUResources_IsReadable(const QPUResources *self, Register_Enum reg, int32_t atSlot)
{
   return QPUResource_IsReadable(&self->m_resources[reg], atSlot);
}

static INLINE DFlowNode *QPUResources_GetOwner(const QPUResources *self, Register_Enum reg)
{
   return QPUResource_GetOwner(&self->m_resources[reg]);
}

// methods
QPUResource *QPUResources_FindFreeResource(QPUResources *self, DFlowRegFile regFile, Register_File banksFreeInInstr, QPUResources_Preference p);
QPUResource *QPUResources_FindFreeResourceDef(QPUResources *self, DFlowRegFile regFile, Register_File banksFreeInInstr);

QPUResource *QPUResources_FindFreeAccumulator(QPUResources *self);

QPUResource *QPUResources_FindFreeRegister(QPUResources *self, DFlowRegFile regFile, Register_File banksFreeInInstr, QPUResources_Preference p);
QPUResource *QPUResources_FindFreeRegisterDef(QPUResources *self, DFlowRegFile regFile, Register_File banksFreeInInstr);

void     QPUResources_DebugDump(const QPUResources *self);
void     QPUResources_CheckRefCounts(const QPUResources *self, bool *stillReferenced, bool *underflow);
void     QPUResources_MoveRefCount(QPUResources *self, Register_Enum from, Register_Enum to);
bool     QPUResources_IsFull(const QPUResources *self, bool threaded);
uint32_t QPUResources_RegistersFree(const QPUResources *self, Register_File bank);

#endif /* __GLSL_REGISTERS_H__ */
