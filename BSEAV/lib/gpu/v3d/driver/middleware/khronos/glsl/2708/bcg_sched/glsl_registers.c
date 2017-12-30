/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_exception.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_registers.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.h"

#include <stdint.h>
#include <assert.h>

#ifndef WIN32
#define _snprintf snprintf
#endif

// Undo global namespace pollution
#undef max

extern uint32_t xxx_shader;

///////////////////////////////////////////////////////////////////////////////
// QPUResource static
///////////////////////////////////////////////////////////////////////////////

static INLINE int32_t WhenReadable(const QPUResource *self)
{
   return self->m_readableAtSlot;
}

///////////////////////////////////////////////////////////////////////////////
// QPUResources
///////////////////////////////////////////////////////////////////////////////

void QPUResources_Constr(QPUResources *self, bool isFrag, bool allowThread)
{
   uint32_t r;

   for (r = 0; r < Register_NUM_REGISTERS; r++)
   {
      QPUResource_ConstrName(&self->m_resources[r], (Register_Enum)r);
   }

   self->m_maxARegister = allowThread ? Register_RA15 : Register_RA31;
   self->m_maxBRegister = allowThread ? Register_RB15 : Register_RB31;

   if (isFrag)
   {
      QPUResource_SetWrittenAt(&self->m_resources[Register_RA15], -2);
      QPUResource_SetReferenced(&self->m_resources[Register_RA15], NULL, 0x7fffffff);
      QPUResource_SetWrittenAt(&self->m_resources[Register_RB15], -2);
      QPUResource_SetReferenced(&self->m_resources[Register_RB15], NULL, 0x7fffffff);
   }
}

QPUResource *QPUResources_FindFreeAccumulator(QPUResources *self)
{
   uint32_t a;

   for (a = Register_ACC0; a <= Register_ACC3; a++)
   {
      QPUResource *res = &self->m_resources[a];

      if (!QPUResource_IsReferenced(res))
         return res;
   }

   return NULL;
}

QPUResource *QPUResources_FindFreeRegister(QPUResources *self, DFlowRegFile rf, Register_File banksFreeInInstr, QPUResources_Preference p)
{
   DFlowRegFile   regFile = rf;
   int32_t        r;

   if (regFile == DFlowRegFile_A_OR_ACCUM && banksFreeInInstr != Register_FILE_A && banksFreeInInstr != Register_FILE_EITHER)
      regFile = DFlowRegFile_B_OR_ACCUM;

   if (regFile == DFlowRegFile_B_OR_ACCUM && banksFreeInInstr != Register_FILE_B && banksFreeInInstr != Register_FILE_EITHER)
      regFile = DFlowRegFile_A_OR_ACCUM;

   if (p == QPUResources_FORCE_REGA || p == QPUResources_ACC_OR_REGA)
      regFile = DFlowRegFile_A_OR_ACCUM;
   else if (p == QPUResources_FORCE_REGB || p == QPUResources_ACC_OR_REGB)
      regFile = DFlowRegFile_B_OR_ACCUM;

   switch (regFile)
   {
   case DFlowRegFile_A_ONLY :
   case DFlowRegFile_A_OR_ACCUM :
      if (banksFreeInInstr != Register_FILE_A && banksFreeInInstr != Register_FILE_EITHER)
         return NULL;   // Needs regFile A, but instruction already uses it

      for (r = Register_RA0; r <= (int32_t)self->m_maxARegister; r++)
      {
         QPUResource *res = &self->m_resources[r];
         if (!QPUResource_IsReferenced(res))
            return res;
      }
      break;

   case DFlowRegFile_B_OR_ACCUM :
      if (banksFreeInInstr != Register_FILE_B && banksFreeInInstr != Register_FILE_EITHER)
         return NULL;   // Needs regFile B, but instruction already uses it

      for (r = Register_RB0; r <= (int32_t)self->m_maxBRegister; r++)
      {
         QPUResource *res = &self->m_resources[r];
         if (!QPUResource_IsReferenced(res))
            return res;
      }
      break;

   case DFlowRegFile_ANY :
      if (QPUResources_RegistersFree(self, Register_FILE_A) > QPUResources_RegistersFree(self, Register_FILE_B))
      {
         for (r = Register_RA0; r <= (int32_t)self->m_maxARegister; r++)
         {
            QPUResource *res = &self->m_resources[r];
            if (!QPUResource_IsReferenced(res))
               return res;
         }
      }
      else
      {
         for (r = Register_RB0; r <= (int32_t)self->m_maxBRegister; r++)
         {
            QPUResource *res = &self->m_resources[r];
            if (!QPUResource_IsReferenced(res))
               return res;
         }
      }
      break;
   }

   return NULL;
}

QPUResource *QPUResources_FindFreeRegisterDef(QPUResources *self, DFlowRegFile regFile, Register_File banksFreeInInstr)
{
   return QPUResources_FindFreeRegister(self, regFile, banksFreeInInstr, QPUResources_PREFER_REG);
}


void QPUResources_MoveRefCount(QPUResources *self, Register_Enum from, Register_Enum to)
{
   self->m_resources[to].m_refCount = self->m_resources[from].m_refCount;
}

QPUResource *QPUResources_FindFreeResource(QPUResources *self, DFlowRegFile regFile, Register_File banksFreeInInstr, QPUResources_Preference pref)
{
   QPUResource *res = NULL;

   if (pref == QPUResources_PREFER_ACC || pref == QPUResources_ACC_OR_REGA || pref == QPUResources_ACC_OR_REGB)
   {
      if (regFile != DFlowRegFile_A_ONLY)
         res = QPUResources_FindFreeAccumulator(self);
      else
         pref = QPUResources_FORCE_REGA;

      if (res == NULL)
         res = QPUResources_FindFreeRegister(self, regFile, banksFreeInInstr, pref);
   }
   else
   {
      res = QPUResources_FindFreeRegister(self, regFile, banksFreeInInstr, pref);
      if (res == NULL && (pref == QPUResources_PREFER_ACC || pref == QPUResources_PREFER_REG))
         if (regFile != DFlowRegFile_A_ONLY)
            res = QPUResources_FindFreeAccumulator(self);
   }

   return res;
}

QPUResource *QPUResource_FindFreeResourceDef(QPUResources *self, DFlowRegFile regFile, Register_File banksFreeInInstr)
{
   return QPUResources_FindFreeResource(self, regFile, banksFreeInInstr, QPUResources_PREFER_ACC);
}

#define REG_NAME_SIZE 16

static const char *RegName(uint32_t r, char *buff)
{
   if (r <= Register_RA31)
      _snprintf(buff, REG_NAME_SIZE, "RA%d", r - Register_RA0);
   else if (r <= Register_RB31)
      _snprintf(buff, REG_NAME_SIZE, "RB%d", r - Register_RB0);
   else if (r <= Register_ACC5)
      _snprintf(buff, REG_NAME_SIZE, "R%d", r - Register_ACC0);
   else
      _snprintf(buff, REG_NAME_SIZE, "SPEC%d", r);

   return buff;
}

void QPUResources_DebugDump(const QPUResources *self)
{
   char     buff[REG_NAME_SIZE];
   uint32_t r;

   for (r = 0; r < Register_NUM_REGISTERS; r += 2)
   {
      printf("%4s : %s : Ref %2d R@%3d    ",  RegName(r, buff),     QPUResource_IsReferenced(&self->m_resources[r])     ? "In use" : "Free  ", self->m_resources[r].m_refCount,   WhenReadable(&self->m_resources[r]));
      if (r + 1 < Register_NUM_REGISTERS)
         printf("%4s : %s : Ref %2d R@%3d\n", RegName(r + 1, buff), QPUResource_IsReferenced(&self->m_resources[r + 1]) ? "In use" : "Free  ", self->m_resources[r+1].m_refCount, WhenReadable(&self->m_resources[r + 1]));
      else
         printf("\n");
   }
}

void QPUResources_CheckRefCounts(const QPUResources *self, bool *stillReferenced, bool *underflow)
{
   char     buff[REG_NAME_SIZE];
   bool     first = true;
   uint32_t r;

   *stillReferenced = false;
   *underflow = false;

   for (r = 0; r < Register_NUM_REGISTERS; r++)
   {
      if (self->m_resources[r].m_underflow)
         *underflow = true;

      if (self->m_resources[r].m_refCount > 0)
      {
         if (r == 15 || r == 32 + 15)
            continue;

         if (first)
         {
            printf("*** NON ZERO REF COUNTS in program %d:\n", xxx_shader);
            *stillReferenced = true;
            first = false;
         }
         printf("%s ", RegName(r, buff));
      }
   }

   if (!first)
      printf("\n");
}

uint32_t QPUResources_RegistersFree(const QPUResources *self, Register_File bank)
{
   uint32_t freeRegs = 0;
   int32_t  r;

   if (bank == Register_FILE_A || bank == Register_FILE_EITHER)
   {
      for (r = Register_RA0; r <= (int32_t)self->m_maxARegister; r++)
      {
         if (self->m_resources[r].m_refCount == 0)
            freeRegs++;
      }
   }

   if (bank == Register_FILE_B || bank == Register_FILE_EITHER)
   {
      for (r = Register_RB0; r <= (int32_t)self->m_maxBRegister; r++)
      {
         if (self->m_resources[r].m_refCount == 0)
            freeRegs++;
      }
   }

   return freeRegs;
}

bool QPUResources_IsFull(const QPUResources *self, bool threaded)
{
   uint32_t r;

   if (threaded)
   {
      for (r = Register_RA0; r <= Register_RA15; r++)
         if (self->m_resources[r].m_refCount == 0)
            return false;

      for (r = Register_RB0; r <= Register_RB15; r++)
         if (self->m_resources[r].m_refCount == 0)
            return false;

      for (r = Register_ACC0; r <= Register_ACC3; r++)
         if (self->m_resources[r].m_refCount == 0)
            return false;
   }
   else
   {
      for (r = 0; r <= Register_ACC3; r++)
         if (self->m_resources[r].m_refCount == 0)
            return false;
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////
// QPUResource
///////////////////////////////////////////////////////////////////////////////

void QPUResource_ConstrName(QPUResource *self ,Register_Enum name)
{
   self->m_name           = name;
   self->m_readableAtSlot = 0;
   self->m_refCount       = 0;
   self->m_owner          = NULL;
   self->m_underflow      = false;
}

void QPUResource_Constr(QPUResource *self)
{
   QPUResource_ConstrName(self, Register_UNKNOWN);
}

void QPUResource_SetWrittenAt(QPUResource *self, int32_t slot)
{
   self->m_readableAtSlot = slot + Register_ReadLatency(self->m_name);
}

void QPUResource_SetReferenced(QPUResource *self, DFlowNode *owner, int32_t count)
{
   if (Register_IsRefCounted(self->m_name))
   {
      self->m_refCount += count;
      self->m_owner = owner;
   }
}

void QPUResource_Unreference(QPUResource *self)
{
   if (Register_IsRefCounted(self->m_name))
   {
      if (self->m_refCount == 0)
      {
         char buff[REG_NAME_SIZE];

         self->m_underflow = true;
         RegName(self->m_name, buff);

         printf("*** %s reference count decremented when zero in program %d\n", buff, xxx_shader);
      }

      assert(self->m_refCount > 0);
      if (self->m_refCount > 0)
         self->m_refCount--;

      if (self->m_refCount == 0)
         self->m_owner = NULL;
   }
}

void QPUResource_FullyUnreference(QPUResource *self)
{
   if (Register_IsRefCounted(self->m_name))
   {
      self->m_refCount = 0;
      self->m_readableAtSlot = 0;
      self->m_owner     = NULL;
      self->m_underflow = false;
   }
}
