/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_qpu_instr.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_registers.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_exception.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_bcg_sched.h"
#include "middleware/khronos/glsl/2708/glsl_qdisasm_4.h"
#include "middleware/khronos/glsl/2708/glsl_allocator_4.h"
#include <assert.h>

#ifndef WIN32
#define _snprintf snprintf
#endif

#define CODING(name, startBit, numBits) \
   static const uint32_t SHIFT_##name = (startBit);\
   static const uint64_t MASK_##name  = (uint64_t)((((uint64_t)1) << numBits) - 1) << (startBit);

static void _SetExplicitCoding(uint64_t *var, uint64_t value, uint64_t mask, uint64_t shift)
{
   (*var) &= ~mask;
   (*var) |=  mask & ((value) << shift);
}

#define SET_EXPLICIT_CODING(var, name, value) \
   _SetExplicitCoding(&var, (uint64_t)value, MASK_##name, SHIFT_##name)

#define SET_CODING(name, value) \
   SET_EXPLICIT_CODING(self->m_coding, name, value)

#define GET_EXPLICIT_CODING(var, name) \
   (((var) & MASK_##name) >> SHIFT_##name)

#define GET_CODING(name) \
   ((unsigned int)GET_EXPLICIT_CODING(self->m_coding, name))

CODING(IMMEDIATE,       0, 32);
CODING(LS_BIT,          0, 16);
CODING(SEMAPHORE,       0, 4);
CODING(MUL_B,           0, 3);
CODING(MUL_A,           3, 3);
CODING(SA,              4, 1);
CODING(ADD_B,           6, 3);
CODING(ADD_A,           9, 3);
CODING(RADDR_B,        12, 6);
CODING(SMALL_IMMED,    12, 6);
CODING(BRANCH_RADDR_A, 13, 5);
CODING(MS_BIT,         16, 16);
CODING(RADDR_A,        18, 6);
CODING(REG,            18, 1);
CODING(REL,            19, 1);
CODING(COND_BR,        20, 4);
CODING(OP_ADD,         24, 5);
CODING(OP_MUL,         29, 3);

CODING(WADDR_MUL,      32, 6);
CODING(WADDR_ADD,      38, 6);
CODING(WS,             44, 1);
CODING(SF,             45, 1);
CODING(COND_MUL,       46, 3);
CODING(COND_ADD,       49, 3);
CODING(PACK,           52, 4);
CODING(PM,             56, 1);
CODING(UNPACK,         57, 3);
CODING(SIG,            60, 4);

CODING(TOP4,           60, 4);
CODING(TOP7,           57, 7);
///////////////////////////////////////////////////////////////////////////////
// This is a quick simple implementation of a set of registers to avoid
// using a generic set which implies memory management and tree maintenance which
// for a small set is probably overkill
///////////////////////////////////////////////////////////////////////////////
#define RegSet_SIZE  4

typedef struct
{
   Register_Enum  m_regs[RegSet_SIZE];
   Register_Enum *m_end;
} RegSet;

typedef Register_Enum        *RegSet_iterator;
typedef const Register_Enum  *RegSet_const_iterator;

static void RegSet_Constr(RegSet *self)
{
   uint32_t i;

   for (i = 0; i < RegSet_SIZE; ++i)
      self->m_regs[i] = Register_UNKNOWN;

   self->m_end = &self->m_regs[0];
}

static RegSet_const_iterator RegSet_const_begin(RegSet *self)
{
   return &self->m_regs[0];
}

static RegSet_const_iterator RegSet_const_end(RegSet *self)
{
   return self->m_end;
}

void RegSet_insert(RegSet *self, Register_Enum r)
{
   uint32_t i;

   for (i = 0; i < RegSet_SIZE; ++i)
   {
      if (self->m_regs[i] == Register_UNKNOWN)
      {
         self->m_regs[i] = r;
         self->m_end     = &self->m_regs[i + 1];
         break;
      }

      if (self->m_regs[i] == r)
         break;
   }
}

///////////////////////////////////////////////////////////////////////////////
// This is a quick simple implementation of a list of registers to avoid
// using a generic list which implies memory management and is probably overkill
///////////////////////////////////////////////////////////////////////////////
#define RegList_SIZE 4

typedef struct
{
   Register_Enum m_regs[RegList_SIZE];
   Register_Enum *m_end;
} RegList;

static void RegList_Constr(RegList *self)
{
   uint32_t i;

   for (i = 0; i < RegList_SIZE; ++i)
      self->m_regs[i] = Register_UNKNOWN;

   self->m_end = &self->m_regs[0];
}

static void RegList_push_back(RegList *self, Register_Enum r)
{
   *(self->m_end) = r;
   (self->m_end)++;
}

static Register_Enum RegList_front(RegList *self)
{
   return self->m_regs[0];
}

static Register_Enum RegList_back(RegList *self)
{
   return *(self->m_end - 1);
}

///////////////////////////////////////////////////////////////////////////////
// statics
///////////////////////////////////////////////////////////////////////////////

static Exception  s_lastError = eExceptionNone;
static char       s_disasmBuffer[DISASM_BUFFER_SIZE];

static uint64_t ErrorInt64(Exception e, uint64_t retCode)
{
   s_lastError = e;
   return retCode;
}

static bool ErrorBool(Exception e, bool retCode)
{
   s_lastError = e;
   return retCode;
}

static bool EncodeSmallImmediateInt(QPUInstr *self, int8_t i);
static bool EncodeSmallImmediateFloat(QPUInstr *self, float i);
static bool HasSelfExclusiveClash(QPUInstr *self, Register_Enum test, bool add, Register_Enum r1, Register_Enum r2, Register_Enum r3);

///////////////////////////////////////////////////////////////////////////////
// QPUInstr
///////////////////////////////////////////////////////////////////////////////

void QPUInstr_Constr(QPUInstr *self, QPUInstr_Type type)
{
   self->m_type      = type;
   self->m_coding    = 0;
   self->m_addOutReg = Register_UNKNOWN;
   self->m_mulOutReg = Register_UNKNOWN;

   switch (type)
   {
   case QPUInstr_ALU:
   case QPUInstr_IMMED32:
   case QPUInstr_IMMED_PER_ELEMENT:
   case QPUInstr_SEMAPHORE:
      self->m_packPMSet   = false;
      self->m_unpackPMSet = false;
      SET_CODING(COND_ADD, CondCode_NEVER);
      SET_CODING(COND_MUL, CondCode_NEVER);
      break;

   default:
      break;
   }

   switch (type)
   {
   case QPUInstr_UNKNOWN:
      break;

   case QPUInstr_ALU:
      self->m_usingSmallImmediate = false;
      self->m_virtualUnpack       = VirtualUnpack_NONE;

      self->m_addInput[0] = Register_ACC0;
      self->m_addInput[1] = Register_ACC0;
      self->m_mulInput[0] = Register_ACC0;
      self->m_mulInput[1] = Register_ACC0;

      SET_CODING(SIG, Sig_NONE);
      SET_CODING(OP_ADD, AOP_NOP);
      SET_CODING(OP_MUL, MOP_NOP);
      SET_CODING(RADDR_A, Register_GetCode(Register_NOP));
      SET_CODING(RADDR_B, Register_GetCode(Register_NOP));
      break;

   case QPUInstr_IMMED32:
      SET_CODING(TOP7, 0x70);  // 1110000b
      break;

   case QPUInstr_IMMED_PER_ELEMENT:
      SET_CODING(TOP7, 0x73);  // 1110011b (unsigned initially)
      break;

   case QPUInstr_SEMAPHORE:
      SET_CODING(TOP7, 0x74);  // 1110100b
      break;

   case QPUInstr_BRANCH:
   default:
      vcos_assert(0);
      break;
   }
}

bool QPUInstr_SetAddOutputRegister(QPUInstr *self, Register_Enum reg)
{
   Register_Enum old = self->m_addOutReg;
   self->m_addOutReg = reg;

   // Check for register file clashes
   if (QPUInstr_GetCoding(self) == 0)
   {
      self->m_addOutReg = old;
      return false;
   }

   return true;
}

bool QPUInstr_SetMulOutputRegister(QPUInstr *self, Register_Enum reg)
{
   Register_Enum old = self->m_mulOutReg;
   self->m_mulOutReg = reg;

   // Check for register file clashes
   if (QPUInstr_GetCoding(self) == 0)
   {
      self->m_mulOutReg = old;
      return false;
   }

   return true;
}

static uint64_t Base_GetCoding(QPUInstr *self)
{
   Register_File addFile = Register_GetFile(self->m_addOutReg);
   Register_File mulFile = Register_GetFile(self->m_mulOutReg);

   // Check outputs
   if ((addFile == Register_FILE_A && mulFile == Register_FILE_A) ||
       (addFile == Register_FILE_B && mulFile == Register_FILE_B))
      return ErrorInt64(eExceptionRegfileWriteClash, (uint64_t)0);

   // Check that both units aren't writing the same output
   if (self->m_addOutReg == self->m_mulOutReg && self->m_addOutReg != Register_NOP && self->m_addOutReg != Register_UNKNOWN)
      return ErrorInt64(eExceptionRegfileWriteClash, (uint64_t)0);

   // Check that only one special reg is present where applicable
   if (Register_IsMutuallyExclusiveOnWrite(self->m_addOutReg) && Register_IsMutuallyExclusiveOnWrite(self->m_mulOutReg))
      return ErrorInt64(eExceptionSpecialRegWriteClash, (uint64_t)0);

   // Encode outputs
   if (addFile == Register_FILE_B || mulFile == Register_FILE_A)
      SET_CODING(WS, 1);
   else
      SET_CODING(WS, 0);

   SET_CODING(WADDR_ADD, Register_GetCode(self->m_addOutReg));
   SET_CODING(WADDR_MUL, Register_GetCode(self->m_mulOutReg));

   return self->m_coding;
}

static uint64_t NonBranch_GetCoding(QPUInstr *self)
{
   if (self->m_packPMSet != self->m_unpackPMSet)
      if (GET_CODING(PACK) != 0 && GET_CODING(UNPACK) != 0)
         return ErrorInt64(eExceptionPackUnpackClash, (uint64_t)0);

   return Base_GetCoding(self);
}

static uint64_t ALU_GetCoding(QPUInstr *self)
{
   uint32_t                numA = 0, numB = 0, numBoth = 0;
   uint64_t                coding;
   RegSet                  uniqueRegs;
   InputMux_Enum           addmux[2];
   InputMux_Enum           mulmux[2];
   uint8_t                 regA;
   uint8_t                 regB;
   RegList                 bothList;
   RegSet_const_iterator   iter;

   // Validate the unpack mode against the float/integer inputs
   if (self->m_virtualUnpack != VirtualUnpack_NONE)
   {
      AOP_Enum aop = (AOP_Enum)GET_CODING(OP_ADD);
      MOP_Enum mop = (MOP_Enum)GET_CODING(OP_MUL);

      bool isFloat = VirtualUnpack_IsFloat(self->m_virtualUnpack);

      if (isFloat)
      {
         if (!AOP_HasFloatInput(aop) && !MOP_HasFloatInput(mop))
            return ErrorInt64(eExceptionUnpackClash, (uint64_t)0);
      }
      else
      {
         if (AOP_HasFloatInput(aop) || MOP_HasFloatInput(mop))
            return ErrorInt64(eExceptionUnpackClash, (uint64_t)0);
      }

      // Check that only one unpack is happening
      if (Register_GetFile(self->m_addInput[0]) == Register_FILE_A)
         if (self->m_addInput[0] == self->m_mulInput[0] || self->m_addInput[0] == self->m_mulInput[1])
            return ErrorInt64(eExceptionUnpackClash, (uint64_t)0);

      if (Register_GetFile(self->m_addInput[1]) == Register_FILE_A)
         if (self->m_addInput[1] == self->m_mulInput[0] || self->m_addInput[1] == self->m_mulInput[1])
            return ErrorInt64(eExceptionUnpackClash, (uint64_t)0);
   }

   // Check for clash of uniform and TMU access
   // TODO : This is over-restrictive as direct memory accesses from the vertex shader don't
   // have this restriction
   if (Register_IsTMUSetupWrite(self->m_addOutReg) || Register_IsTMUSetupWrite(self->m_mulOutReg))
   {
      if (self->m_addInput[0] == Register_UNIFORM_READ || self->m_addInput[1] == Register_UNIFORM_READ ||
          self->m_mulInput[0] == Register_UNIFORM_READ || self->m_mulInput[1] == Register_UNIFORM_READ)
          return ErrorInt64(eExceptionTMUUniformClash, (uint64_t)0);
   }

   // Check for signal clashing with reg
   if (Sig_ClashesWith((Sig_Enum)GET_CODING(SIG), self->m_addOutReg) ||
       Sig_ClashesWith((Sig_Enum)GET_CODING(SIG), self->m_mulOutReg))
       return ErrorInt64(eExceptionSignalRegClash, (uint64_t)0);

   // Resolve input muxing now
   // Only one regfile A and one regfile B are allowed (unless reading the same register number)
   coding = NonBranch_GetCoding(self);
   if (coding == 0)
      return 0;

   // First check for special cases where we can't read in the same instruction
   if (HasSelfExclusiveClash(self, self->m_addInput[0], true,  self->m_addInput[1], self->m_mulInput[0], self->m_mulInput[1]) ||
       HasSelfExclusiveClash(self, self->m_addInput[1], true,  self->m_addInput[0], self->m_mulInput[0], self->m_mulInput[1]) ||
       HasSelfExclusiveClash(self, self->m_mulInput[0], false, self->m_mulInput[1], self->m_addInput[0], self->m_addInput[1]) ||
       HasSelfExclusiveClash(self, self->m_mulInput[1], false, self->m_mulInput[0], self->m_addInput[0], self->m_addInput[1]))
       return ErrorInt64(eExceptionRegfileReadClash, (uint64_t)0);

   RegSet_Constr(&uniqueRegs);

   RegSet_insert(&uniqueRegs, self->m_addInput[0]);
   RegSet_insert(&uniqueRegs, self->m_addInput[1]);
   RegSet_insert(&uniqueRegs, self->m_mulInput[0]);
   RegSet_insert(&uniqueRegs ,self->m_mulInput[1]);

   addmux[0] = Register_GetInputMux(self->m_addInput[0]);
   addmux[1] = Register_GetInputMux(self->m_addInput[1]);
   mulmux[0] = Register_GetInputMux(self->m_mulInput[0]);
   mulmux[1] = Register_GetInputMux(self->m_mulInput[1]);

   regA = Register_GetCode(Register_NOP);
   regB = Register_GetCode(Register_NOP);

   RegList_Constr(&bothList);

   for (iter = RegSet_const_begin(&uniqueRegs); iter != RegSet_const_end(&uniqueRegs); ++iter)
   {
      switch (Register_GetInputMux(*iter))
      {
      case InputMux_REGA      : numA++; regA = Register_GetCode(*iter); break;
      case InputMux_REGB      : numB++; regB = Register_GetCode(*iter); break;
      case InputMux_REGA_OR_B : numBoth++; RegList_push_back(&bothList, *iter); break;

      default:
         break;
      }
   }

   if (self->m_usingSmallImmediate && ((numA > 0 && numBoth > 0) || numBoth == 2))
      return ErrorInt64(eExceptionRegfileBCannotBeUsed, (uint64_t)0);

   if (numA > 1 || numB > 1 || (numA + numB + numBoth > 2))
      return ErrorInt64(eExceptionRegfileReadClash, (uint64_t)0);

   if (numBoth + numA + numB > 2)
      return ErrorInt64(eExceptionRegfileReadClash, (uint64_t)0);

   if (numBoth == 2)
   {
      Register_Enum  bothListFront = RegList_front(&bothList);
      Register_Enum  bothListBack  = RegList_back(&bothList);

      // Just assign them
      regA = Register_GetCode(bothListFront);
      regB = Register_GetCode(bothListBack);

      if (bothListFront == self->m_addInput[0])
         addmux[0] = InputMux_REGA;
      if (bothListFront == self->m_addInput[1])
         addmux[1] = InputMux_REGA;
      if (bothListFront == self->m_mulInput[0])
         mulmux[0] = InputMux_REGA;
      if (bothListFront == self->m_mulInput[1])
         mulmux[1] = InputMux_REGA;

      if (bothListBack == self->m_addInput[0])
         addmux[0] = InputMux_REGB;
      if (bothListBack == self->m_addInput[1])
         addmux[1] = InputMux_REGB;
      if (bothListBack == self->m_mulInput[0])
         mulmux[0] = InputMux_REGB;
      if (bothListBack == self->m_mulInput[1])
         mulmux[1] = InputMux_REGB;
   }
   else if (numBoth == 1)
   {
      InputMux_Enum muxForBoth = InputMux_REGA;
      uint8_t        *regAB;

      if (numA == 0)
      {
         muxForBoth = InputMux_REGA;
         regAB      = &regA;
      }
      else
      {
         muxForBoth = InputMux_REGB;
         regAB      = &regB;
      }

      if (addmux[0] == InputMux_REGA_OR_B)
      {
         addmux[0] = muxForBoth;
         *regAB = Register_GetCode(self->m_addInput[0]);
      }
      if (addmux[1] == InputMux_REGA_OR_B)
      {
         addmux[1] = muxForBoth;
         *regAB = Register_GetCode(self->m_addInput[1]);
      }
      if (mulmux[0] == InputMux_REGA_OR_B)
      {
         mulmux[0] = muxForBoth;
         *regAB = Register_GetCode(self->m_mulInput[0]);
      }
      if (mulmux[1] == InputMux_REGA_OR_B)
      {
         mulmux[1] = muxForBoth;
         *regAB = Register_GetCode(self->m_mulInput[1]);
      }
   }

   assert(addmux[0] != InputMux_REGA_OR_B);
   assert(addmux[1] != InputMux_REGA_OR_B);
   assert(mulmux[0] != InputMux_REGA_OR_B);
   assert(mulmux[1] != InputMux_REGA_OR_B);

   SET_EXPLICIT_CODING(coding, RADDR_A, regA);
   if (!self->m_usingSmallImmediate)
      SET_EXPLICIT_CODING(coding, RADDR_B, regB);

   SET_EXPLICIT_CODING(coding, ADD_A, addmux[0]);
   SET_EXPLICIT_CODING(coding, ADD_B, addmux[1]);
   SET_EXPLICIT_CODING(coding, MUL_A, mulmux[0]);
   SET_EXPLICIT_CODING(coding, MUL_B, mulmux[1]);

   return coding;
}

uint64_t QPUInstr_GetCoding(QPUInstr *self)
{
   uint64_t ret = 0;

   switch (self->m_type)
   {
   case QPUInstr_UNKNOWN:
      ret = 0x100009e7009e7000; /* NOP */
      break;

   case QPUInstr_ALU:
      ret = ALU_GetCoding(self);
      break;

   case QPUInstr_IMMED32:
   case QPUInstr_IMMED_PER_ELEMENT:
   case QPUInstr_SEMAPHORE:
      ret = NonBranch_GetCoding(self);
      break;

   case QPUInstr_BRANCH:
   default:
      vcos_assert(0);
      break;
   }

   return ret;
}

// Not thread safe
const char *QPUInstr_Disassemble(QPUInstr *self)
{
   uint64_t c = QPUInstr_GetCoding(self);

   if (c == 0)
   {
      PrintException(s_lastError);
      return "Invalid instruction";
   }

   glsl_qdisasm_instruction(s_disasmBuffer, DISASM_BUFFER_SIZE, (uint32_t)(c & 0xFFFFFFFF), (uint32_t)(c >> 32));

   return s_disasmBuffer;
}

// Not thread safe
const char *QPUInstr_DisassembleEx(QPUInstr *self, uint32_t numUniforms, const Uniform *uniforms, uint32_t *curUnif)
{
   char instr_buffer[DISASM_BUFFER_SIZE];

   uint64_t c = QPUInstr_GetCoding(self);

   if (c == 0)
   {
      PrintException(s_lastError);
      return "Invalid instruction";
   }

   glsl_qdisasm_instruction(instr_buffer, DISASM_BUFFER_SIZE, (uint32_t)(c & 0xFFFFFFFF), (uint32_t)(c >> 32));

   // Skip any hidden uniform types
   while (*curUnif < numUniforms &&
            uniforms[*curUnif].m_type >= BACKEND_UNIFORM_TEX_PARAM0 &&
            uniforms[*curUnif].m_type <= BACKEND_UNIFORM_TEX_NOT_USED)
      (*curUnif)++;

   if (strstr(instr_buffer, "unif") != NULL)
   {
      vcos_assert(*curUnif < numUniforms);

      switch (uniforms[*curUnif].m_type)
      {
      case BACKEND_UNIFORM:
         _snprintf(s_disasmBuffer, DISASM_BUFFER_SIZE, "%s    [unif %x]", instr_buffer, uniforms[*curUnif].m_value);
         break;
      case BACKEND_UNIFORM_ADDRESS:
         _snprintf(s_disasmBuffer, DISASM_BUFFER_SIZE, "%s    [unif_addr %x]", instr_buffer, uniforms[*curUnif].m_value);
         break;
      case BACKEND_UNIFORM_LITERAL:
         _snprintf(s_disasmBuffer, DISASM_BUFFER_SIZE, "%s    [literal 0x%08x]", instr_buffer, uniforms[*curUnif].m_value);
         break;
      case BACKEND_UNIFORM_SCALE_X:
         _snprintf(s_disasmBuffer, DISASM_BUFFER_SIZE, "%s    [unif scale_x]", instr_buffer);
         break;
      case BACKEND_UNIFORM_SCALE_Y:
         _snprintf(s_disasmBuffer, DISASM_BUFFER_SIZE, "%s    [unif scale_y]", instr_buffer);
         break;
      case BACKEND_UNIFORM_SCALE_Z:
         _snprintf(s_disasmBuffer, DISASM_BUFFER_SIZE, "%s    [unif scale_z]", instr_buffer);
         break;
      case BACKEND_UNIFORM_OFFSET_Z:
         _snprintf(s_disasmBuffer, DISASM_BUFFER_SIZE, "%s    [unif offset_z]", instr_buffer);
         break;
      case BACKEND_UNIFORM_VPM_READ_SETUP:
         _snprintf(s_disasmBuffer, DISASM_BUFFER_SIZE, "%s    [unif vpm_read_setup]", instr_buffer);
         break;
      case BACKEND_UNIFORM_VPM_WRITE_SETUP:
         _snprintf(s_disasmBuffer, DISASM_BUFFER_SIZE, "%s    [unif vpm_write_setup]", instr_buffer);
         break;
      default:
         _snprintf(s_disasmBuffer, DISASM_BUFFER_SIZE, "%s    [?unif? 0x%08x 0x%08x]", instr_buffer, uniforms[*curUnif].m_type,
                                                                                          uniforms[*curUnif].m_value);
         break;
      }

      (*curUnif)++;
   }
   else
      strcpy(s_disasmBuffer, instr_buffer);

   return s_disasmBuffer;
}

Register_File QPUInstr_GetFreeRegisterBanks(QPUInstr *self)
{
   Register_File addFile = Register_GetFile(self->m_addOutReg);
   Register_File mulFile = Register_GetFile(self->m_mulOutReg);

   if (addFile != Register_FILE_NONE && mulFile != Register_FILE_NONE)
      return Register_FILE_NONE;
   else if (addFile == Register_FILE_NONE && mulFile == Register_FILE_NONE)
      return Register_FILE_EITHER;

   if (addFile == Register_FILE_A)
      return Register_FILE_B;
   else if (addFile == Register_FILE_B)
      return Register_FILE_A;
   else if (addFile == Register_FILE_EITHER)
      return Register_FILE_EITHER;

   if (mulFile == Register_FILE_A)
      return Register_FILE_B;
   else if (mulFile == Register_FILE_B)
      return Register_FILE_A;
   else if (mulFile == Register_FILE_EITHER)
      return Register_FILE_EITHER;

   return Register_FILE_NONE;
}

bool QPUInstr_WritesTo(const QPUInstr *self, Register_Enum reg)
{
   return self->m_addOutReg == reg || self->m_mulOutReg == reg;
}

bool QPUInstr_HasImplicitSBWait(const QPUInstr *self)
{
   Sig_Enum sig = QPUInstr_GetSignal(self);

   if (QPUInstr_WritesTo(self, Register_TLB_Z) || QPUInstr_WritesTo(self, Register_TLB_COLOUR_ALL) || QPUInstr_WritesTo(self, Register_TLB_COLOUR_MS) ||
       sig == Sig_COVLOAD || sig == Sig_COLLOAD || sig == Sig_COLLOAD_PRGEND || sig == Sig_LDALPHA)
       return true;

   return false;
}

bool QPUInstr_SetFlags(QPUInstr *self, bool val)
{
   bool ret = true;

   switch (self->m_type)
   {
   case QPUInstr_ALU:
   case QPUInstr_IMMED32:
   case QPUInstr_IMMED_PER_ELEMENT:
   case QPUInstr_SEMAPHORE:
      SET_CODING(SF, val ? 1 : 0);
      break;

   case QPUInstr_UNKNOWN:
   case QPUInstr_BRANCH:
   default:
      ret = false;
      break;
   }

   return ret;
}

bool QPUInstr_SetAddCondition(QPUInstr *self, CondCode_Enum val)
{
   bool ret = true;

   switch (self->m_type)
   {
   case QPUInstr_ALU:
   case QPUInstr_IMMED32:
   case QPUInstr_IMMED_PER_ELEMENT:
   case QPUInstr_SEMAPHORE:
      SET_CODING(COND_ADD, val);
      break;

   case QPUInstr_UNKNOWN:
   case QPUInstr_BRANCH:
   default:
      ret = false;
      break;
   }

   return ret;
}

bool QPUInstr_SetMulCondition(QPUInstr *self, CondCode_Enum val)
{
   bool ret = true;

   switch (self->m_type)
   {
   case QPUInstr_ALU:
   case QPUInstr_IMMED32:
   case QPUInstr_IMMED_PER_ELEMENT:
   case QPUInstr_SEMAPHORE:
      SET_CODING(COND_MUL, val);
      break;

   case QPUInstr_UNKNOWN:
   case QPUInstr_BRANCH:
   default:
      ret = false;
      break;
   }

   return ret;
}

bool QPUInstr_SetPackRegA(QPUInstr *self, RegA_Pack_Enum mode)
{
   bool ret = true;

   switch (self->m_type)
   {
   case QPUInstr_ALU:
   case QPUInstr_IMMED32:
   case QPUInstr_IMMED_PER_ELEMENT:
   case QPUInstr_SEMAPHORE:
      SET_CODING(PACK, mode);
      SET_CODING(PM, 0);

      self->m_packPMSet = false;
      break;

   case QPUInstr_UNKNOWN:
   case QPUInstr_BRANCH:
   default:
      ret = false;
      break;
   }

   return ret;
}

bool QPUInstr_SetPackMul(QPUInstr *self, Mul_Pack_Enum mode)
{
   bool ret = true;

   switch (self->m_type)
   {
   case QPUInstr_ALU:
   case QPUInstr_IMMED32:
   case QPUInstr_IMMED_PER_ELEMENT:
   case QPUInstr_SEMAPHORE:
      SET_CODING(PACK, mode);
      SET_CODING(PM, 1);

      self->m_packPMSet = true;
      break;

   case QPUInstr_UNKNOWN:
   case QPUInstr_BRANCH:
   default:
      ret = false;
      break;
   }

   return ret;
}

static bool HasSelfExclusiveClash(QPUInstr *self, Register_Enum test, bool add, Register_Enum r1, Register_Enum r2, Register_Enum r3)
{
   if (self->m_type != QPUInstr_ALU)
      return false;

   if (!Register_IsSelfExclusiveRead(test))
      return false;

   // Clash in other ALU unit
   if (test == r2 || test == r3)
      return true;

   // May have a clash in our unit, but could be a MOV, so check
   if (test == r1)
   {
      if (add)
      {
         switch (GET_CODING(OP_ADD))
         {
         case AOP_FTOI : // Single input operations always have a duplicate argument
         case AOP_ITOF :
         case AOP_NOT  :
         case AOP_CLZ  :
         case AOP_OR   : // This is a mov - both operands will be the same, so not an error
            return false;
         }
      }
      else
      {
         if (GET_CODING(OP_MUL) == MOP_V8MIN) // MOV instruction - both operands will be the same, so not an error
            return false;
      }

      return true;
   }

   return false;
}

bool QPUInstr_SetAddOperandReg(QPUInstr *self, OperandNumber_Enum which, Register_Enum reg)
{
   Register_Enum old = self->m_addInput[which];

   if (self->m_type != QPUInstr_ALU)
      return false;

   self->m_addInput[which] = reg;

   // Check for register file clashes
   if (QPUInstr_GetCoding(self) == 0)
   {
      self->m_addInput[which] = old;
      return false;
   }

   return true;
}

bool QPUInstr_SetMulOperandReg(QPUInstr *self, OperandNumber_Enum which, Register_Enum reg)
{
   Register_Enum old = self->m_mulInput[which];

   if (self->m_type != QPUInstr_ALU)
      return false;

   self->m_mulInput[which] = reg;

   // Check for register file clashes
   if (QPUInstr_GetCoding(self) == 0)
   {
      self->m_mulInput[which] = old;
      return false;
   }

   return true;
}

bool QPUInstr_SetAddOperandInt(QPUInstr *self, OperandNumber_Enum which, int8_t i)
{
   EncodeSmallImmediateInt(self, i);
   return QPUInstr_SetAddOperandReg(self, which, Register_SMALL_IMMED);
}

bool QPUInstr_SetAddOperandFloat(QPUInstr *self, OperandNumber_Enum which, float i)
{
   EncodeSmallImmediateFloat(self, i);
   return QPUInstr_SetAddOperandReg(self, which, Register_SMALL_IMMED);
}

bool QPUInstr_SetMulOperandInt(QPUInstr *self, OperandNumber_Enum which, int8_t i)
{
   EncodeSmallImmediateInt(self, i);
   return QPUInstr_SetMulOperandReg(self, which, Register_SMALL_IMMED);
}

bool QPUInstr_SetMulOperandFloat(QPUInstr *self, OperandNumber_Enum which, float i)
{
   EncodeSmallImmediateFloat(self, i);
   return QPUInstr_SetMulOperandReg(self, which, Register_SMALL_IMMED);
}

bool QPUInstr_SetAddOperation(QPUInstr *self, AOP_Enum op)
{
   if (self->m_type != QPUInstr_ALU)
      return false;

   SET_CODING(OP_ADD, op);
   return true;
}

bool QPUInstr_SetMulOperation(QPUInstr *self, MOP_Enum op)
{
   if (self->m_type != QPUInstr_ALU)
      return false;

   SET_CODING(OP_MUL, op);
   return true;
}

bool QPUInstr_SetUnpack(QPUInstr *self, VirtualUnpack_Enum mode, UnpackSource_Enum src)
{
   if (self->m_type != QPUInstr_ALU)
      return false;

   // Convert virtual unpack mode to real unpack mode
   self->m_virtualUnpack = mode;
   SET_CODING(UNPACK, VirtualUnpack_RealUnpackCode(self->m_virtualUnpack));
   SET_CODING(PM, src);

   self->m_unpackPMSet = (src == UnpackSource_R4);

   if (src == UnpackSource_R4 && !VirtualUnpack_IsR4Compatible(mode))
      return false; // Invalid combination

   return true;
}

static bool EncodeSmallImmediateInt(QPUInstr *self, int8_t i)
{
   if (self->m_usingSmallImmediate)
      ErrorBool(eExceptionOnlyOneImmediateAllowed, false);

   self->m_usingSmallImmediate = true;

   SET_CODING(TOP4, 13);   // 1101b

   if (i >= 0 && i <= 15)
      SET_CODING(SMALL_IMMED, i);
   else if (i <= -1 && i >= -16)
      SET_CODING(SMALL_IMMED, (i + 32));
   else
      ErrorBool(eExceptionInvalidSmallImmediate, false);

   return true;
}

static bool EncodeSmallImmediateFloat(QPUInstr *self, float f)
{
   if (self->m_usingSmallImmediate)
      ErrorBool(eExceptionOnlyOneImmediateAllowed, false);

   self->m_usingSmallImmediate = true;

   SET_CODING(TOP4, 13);   // 1101b

   if (f >= 1.0f)
   {
      if (f == 1.0f)
         SET_CODING(SMALL_IMMED, 32);
      else if (f == 2.0f)
         SET_CODING(SMALL_IMMED, 33);
      else if (f == 4.0f)
         SET_CODING(SMALL_IMMED, 34);
      else if (f == 8.0f)
         SET_CODING(SMALL_IMMED, 35);
      else if (f == 16.0f)
         SET_CODING(SMALL_IMMED, 36);
      else if (f == 32.0f)
         SET_CODING(SMALL_IMMED, 37);
      else if (f == 64.0f)
         SET_CODING(SMALL_IMMED, 38);
      else if (f == 128.0f)
         SET_CODING(SMALL_IMMED, 39);
      else
         ErrorBool(eExceptionInvalidSmallImmediate, false);
   }
   else
   {
      if (f == 0.0f || f == -0.0f)
         SET_CODING(SMALL_IMMED, 0);
      else if (f == 1.0f / 256.0f)
         SET_CODING(SMALL_IMMED, 40);
      else if (f == 1.0f / 128.0f)
         SET_CODING(SMALL_IMMED, 41);
      else if (f == 1.0f / 64.0f)
         SET_CODING(SMALL_IMMED, 42);
      else if (f == 1.0f / 32.0f)
         SET_CODING(SMALL_IMMED, 43);
      else if (f == 1.0f / 16.0f)
         SET_CODING(SMALL_IMMED, 44);
      else if (f == 1.0f / 8.0f)
         SET_CODING(SMALL_IMMED, 45);
      else if (f == 1.0f / 4.0f)
         SET_CODING(SMALL_IMMED, 46);
      else if (f == 1.0f / 2.0f)
         SET_CODING(SMALL_IMMED, 47);
      else
         ErrorBool(eExceptionInvalidSmallImmediate, false);
   }

   return true;
}

bool QPUInstr_SetSignal(QPUInstr *self, Sig_Enum sig)
{
   if (self->m_type != QPUInstr_ALU)
      return false;

   if (GET_CODING(SIG) != 1)
      return false;

   SET_CODING(SIG, sig);
   return true;
}

void QPUInstr_ClearSignal(QPUInstr *self)
{
   if (self->m_type != QPUInstr_ALU)
      return;

   SET_CODING(SIG, Sig_NONE);
}

Sig_Enum QPUInstr_GetSignal(const QPUInstr *self)
{
   if (self->m_type != QPUInstr_ALU)
      return Sig_NONE;

   return (Sig_Enum)GET_CODING(SIG);
}

bool QPUInstr_IsValidSmallImmediateInt8(int8_t i)
{
   return (i >= 0 && i <= 15) || (i <= -1 && i >= -16);
}

bool QPUInstr_IsValidSmallImmediateFloat(float f)
{
   if (f >= 1.0f)
   {
      if (f == 1.0f)
         return true;
      else if (f == 2.0f)
         return true;
      else if (f == 4.0f)
         return true;
      else if (f == 8.0f)
         return true;
      else if (f == 16.0f)
         return true;
      else if (f == 32.0f)
         return true;
      else if (f == 64.0f)
         return true;
      else if (f == 128.0f)
         return true;
   }
   else
   {
      if (f == 0.0f || f == -0.0f)
         return true;
      else if (f == 1.0f / 256.0f)
         return true;
      else if (f == 1.0f / 128.0f)
         return true;
      else if (f == 1.0f / 64.0f)
         return true;
      else if (f == 1.0f / 32.0f)
         return true;
      else if (f == 1.0f / 16.0f)
         return true;
      else if (f == 1.0f / 8.0f)
         return true;
      else if (f == 1.0f / 4.0f)
         return true;
      else if (f == 1.0f / 2.0f)
         return true;
   }

   return false;
}

bool QPUInstr_ReadsFrom(const QPUInstr *self, Register_Enum reg)
{
   if (self->m_type != QPUInstr_ALU)
      return false;

   return self->m_addInput[0] == reg || self->m_addInput[1] == reg || self->m_mulInput[0] == reg || self->m_mulInput[1] == reg;
}


bool QPUInstr_HasInvalidReadsForProgramEnd(const QPUInstr *self)
{
   if (self->m_type != QPUInstr_ALU)
      return false;

   return QPUInstr_ReadsFrom(self, Register_UNIFORM_READ) || QPUInstr_ReadsFrom(self, Register_VARYING_READ) ||
          QPUInstr_ReadsFrom(self, Register_RA14)         || QPUInstr_ReadsFrom(self, Register_RB14);
}

bool QPUInstr_SetImmediateInt32(QPUInstr *self, uint32_t i, bool perElementSigned)
{
   bool ret = true;

   switch (self->m_type)
   {
   case QPUInstr_IMMED32:
      SET_CODING(IMMEDIATE, i);
      break;

   case QPUInstr_IMMED_PER_ELEMENT:
      if (perElementSigned)
         SET_CODING(TOP7, 0x71);  // 1110001b
      else
         SET_CODING(TOP7, 0x73);  // 1110011b

      SET_CODING(IMMEDIATE, i);
      break;

   default:
      vcos_assert(0);
      ret = false;
      break;
   }

   return ret;
}

bool QPUInstr_SetSemaphore(QPUInstr *self, uint8_t semaphoreNumber, Semaphore_Enum incOrDec)
{
   bool ret = true;
   vcos_assert(semaphoreNumber < 16);

   switch (self->m_type)
   {
   case QPUInstr_SEMAPHORE:
      SET_CODING(SEMAPHORE, semaphoreNumber);
      SET_CODING(SA, incOrDec);
      break;

   default:
      vcos_assert(0);
      ret = false;
      break;
   }

   return ret;
}

//////////////////////////////////////////////////////////////////////////////
// QPUGenericInstr
//////////////////////////////////////////////////////////////////////////////

// Private interface
static void NeedsALU(QPUGenericInstr *self)
{
   if (QPUInstr_GetType(&self->m_instr) == QPUInstr_UNKNOWN)
   {
      QPUInstr_Constr(&self->m_instr, QPUInstr_ALU);
   }
}

static void NeedsLDI32(QPUGenericInstr *self)
{
   if (QPUInstr_GetType(&self->m_instr) == QPUInstr_UNKNOWN)
   {
      QPUInstr_Constr(&self->m_instr, QPUInstr_IMMED32);
   }
}

static void NeedsSem(QPUGenericInstr *self)
{
   if (QPUInstr_GetType(&self->m_instr) == QPUInstr_UNKNOWN)
   {
      QPUInstr_Constr(&self->m_instr, QPUInstr_SEMAPHORE);
   }
}

static void NeedsBranch(QPUGenericInstr *self)
{
   if (QPUInstr_GetType(&self->m_instr) == QPUInstr_UNKNOWN)
   {
      QPUInstr_Constr(&self->m_instr, QPUInstr_BRANCH);
   }
}

static bool CheckUnpackClash(QPUGenericInstr *self, const QPUOperand *op)
{
   // Check that the new operation we want to add has the same unpack mode as the existing instruction
   if (QPUOperand_IsRegister(op) && QPUOperand_GetUnpack(op) != VirtualUnpack_NONE)
   {
      if (Register_GetFile(QPUOperand_ValueRegister(op)) == Register_FILE_A)
         return self->m_virtualUnpack == QPUOperand_GetUnpack(op) && self->m_unpackSrc == UnpackSource_REGFILE_A;
      else if (QPUOperand_ValueRegister(op) == Register_ACC4)
         return self->m_virtualUnpack == QPUOperand_GetUnpack(op) && self->m_unpackSrc == UnpackSource_R4;
   }
   else if (QPUOperand_IsRegister(op) && QPUOperand_GetUnpack(op) == VirtualUnpack_NONE && self->m_virtualUnpack != VirtualUnpack_NONE)
   {
      if (Register_GetFile(QPUOperand_ValueRegister(op)) == Register_FILE_A)
         return self->m_unpackSrc != UnpackSource_REGFILE_A;
      else if (QPUOperand_ValueRegister(op) == Register_ACC4)
         return self->m_unpackSrc != UnpackSource_R4;
   }

   return true;
}

static bool ProcessUnpack(QPUGenericInstr *self, const QPUOperand *operand)
{
   if (QPUOperand_GetUnpack(operand) != VirtualUnpack_NONE)
   {
      if (!QPUOperand_IsRegister(operand))
         return false;

      // Do we have an operation already that doesn't have unpacking?
      if (self->m_adderUsed || self->m_mulUsed)
      {
         // We have something already - unpack has to match new operand
         if (self->m_virtualUnpack != QPUOperand_GetUnpack(operand))
            return false;
      }

      if (QPUOperand_ValueRegister(operand) == Register_ACC4)
      {
         if (!VirtualUnpack_IsR4Compatible(QPUOperand_GetUnpack(operand)))
            return false;  // Shouldn't really get here

         return QPUGenericInstr_SetUnpack(self, QPUOperand_GetUnpack(operand), UnpackSource_R4);
      }
      else
      {
         if (Register_GetFile(QPUOperand_ValueRegister(operand)) != Register_FILE_A)
            return false;  // Shouldn't really get here

         return QPUGenericInstr_SetUnpack(self, QPUOperand_GetUnpack(operand), UnpackSource_REGFILE_A);
      }
   }
   return true;
}

static void GI_Init(QPUGenericInstr *self)
{
   self->m_adderUsed     = false;
   self->m_mulUsed       = false;
   self->m_sigUsed       = false;
   self->m_immedUsed     = false;
   self->m_packUsed      = false;
   self->m_unpackUsed    = false;
   self->m_virtualUnpack = VirtualUnpack_NONE;
   self->m_unpackSrc     = UnpackSource_REGFILE_A;
   self->m_packTarget    = PackTarget_REGFILE_A;
   self->m_full          = false;
}

void QPUGenericInstr_Constr(QPUGenericInstr *self)
{
   GI_Init(self);

   QPUInstr_Constr(&self->m_instr, QPUInstr_UNKNOWN);
}

void QPUGenericInstr_ConstrType(QPUGenericInstr *self, QPUInstr_Type type)
{
   GI_Init(self);

   QPUInstr_Constr(&self->m_instr, type);
}

void QPUGenericInstr_ConstrCopy(QPUGenericInstr *self, const QPUGenericInstr *rhs)
{
   self->m_adderUsed     = rhs->m_adderUsed;
   self->m_mulUsed       = rhs->m_mulUsed;
   self->m_sigUsed       = rhs->m_sigUsed;
   self->m_immedUsed     = rhs->m_immedUsed;
   self->m_packUsed      = rhs->m_packUsed;
   self->m_unpackUsed    = rhs->m_unpackUsed;
   self->m_virtualUnpack = rhs->m_virtualUnpack;
   self->m_unpackSrc     = rhs->m_unpackSrc;
   self->m_packTarget    = rhs->m_packTarget;
   self->m_full          = rhs->m_full;
   self->m_instr         = rhs->m_instr;
}

void QPUGenericInstr_Assign(QPUGenericInstr *self, const QPUGenericInstr *rhs)
{
   if (rhs != self)
      QPUGenericInstr_ConstrCopy(self, rhs);
}

void QPUGenericInstr_Destr(QPUGenericInstr *self)
{
}

bool QPUGenericInstr_SetAdd(QPUGenericInstr *self, AOP_Enum op, const QPUOperand *leftOperand, const QPUOperand *rightOperand)
{
   bool ok = true;

   NeedsALU(self);

   if (QPUInstr_GetType(&self->m_instr) != QPUInstr_ALU)
      return false;

   if (self->m_adderUsed)
      return false;

   if (self->m_sigUsed && (QPUOperand_IsSmallConst(leftOperand) || QPUOperand_IsSmallConst(rightOperand)))
      return false; // Small immediates use signaling bits

   if (!ProcessUnpack(self, leftOperand))
      return false;
   if (leftOperand != rightOperand && !ProcessUnpack(self, rightOperand))
      return false;

   if (!CheckUnpackClash(self, leftOperand) || !CheckUnpackClash(self, rightOperand))
      return false;

   ok = ok && QPUInstr_SetAddOperation(&self->m_instr, op);
   ok = ok && QPUInstr_SetAddCondition(&self->m_instr, CondCode_ALWAYS);

   switch (QPUOperand_GetType(leftOperand))
   {
   case QPUOperand_NONE         : break;
   case QPUOperand_REGISTER     : ok = ok && QPUInstr_SetAddOperandReg(&self->m_instr, OperandNumber_FIRST, QPUOperand_ValueRegister(leftOperand)); break;
   case QPUOperand_SMALL_INT    : ok = ok && QPUInstr_SetAddOperandInt(&self->m_instr, OperandNumber_FIRST, QPUOperand_ValueSmallInt(leftOperand)); self->m_sigUsed = true; break;
   case QPUOperand_SMALL_FLOAT  : ok = ok && QPUInstr_SetAddOperandFloat(&self->m_instr, OperandNumber_FIRST, QPUOperand_ValueSmallFloat(leftOperand)); self->m_sigUsed = true; break;

   default:
      break;
   }

   switch (QPUOperand_GetType(rightOperand))
   {
   case QPUOperand_NONE         : break;
   case QPUOperand_REGISTER     : ok = ok && QPUInstr_SetAddOperandReg(&self->m_instr, OperandNumber_SECOND, QPUOperand_ValueRegister(rightOperand)); break;
   case QPUOperand_SMALL_INT    : ok = ok && QPUInstr_SetAddOperandInt(&self->m_instr, OperandNumber_SECOND, QPUOperand_ValueSmallInt(rightOperand)); self->m_sigUsed = true; break;
   case QPUOperand_SMALL_FLOAT  : ok = ok && QPUInstr_SetAddOperandFloat(&self->m_instr, OperandNumber_SECOND, QPUOperand_ValueSmallFloat(rightOperand)); self->m_sigUsed = true; break;

   default:
      break;
   }

   if (ok)
      self->m_adderUsed = true;

   return ok;
}

bool QPUGenericInstr_SetMul(QPUGenericInstr *self, MOP_Enum op, const QPUOperand *leftOperand, const QPUOperand *rightOperand)
{
   bool ok = true;

   NeedsALU(self);

   if (QPUInstr_GetType(&self->m_instr) != QPUInstr_ALU)
      return false;

   if (self->m_mulUsed)
      return false;

   if (self->m_sigUsed && (QPUOperand_IsSmallConst(leftOperand) || QPUOperand_IsSmallConst(rightOperand)))
      return false; // Small immediates use signaling bits

   if (!ProcessUnpack(self, leftOperand))
      return false;
   if (&leftOperand != &rightOperand && !ProcessUnpack(self, rightOperand))
      return false;

   if (!CheckUnpackClash(self, leftOperand) || !CheckUnpackClash(self, rightOperand))
      return false;

   ok = ok && QPUInstr_SetMulOperation(&self->m_instr, op);
   ok = ok && QPUInstr_SetMulCondition(&self->m_instr, CondCode_ALWAYS);

   switch (QPUOperand_GetType(leftOperand))
   {
   case QPUOperand_NONE         : break;
   case QPUOperand_REGISTER     : ok = ok && QPUInstr_SetMulOperandReg(&self->m_instr, OperandNumber_FIRST, QPUOperand_ValueRegister(leftOperand)); break;
   case QPUOperand_SMALL_INT    : ok = ok && QPUInstr_SetMulOperandInt(&self->m_instr, OperandNumber_FIRST, QPUOperand_ValueSmallInt(leftOperand)); self->m_sigUsed = true; break;
   case QPUOperand_SMALL_FLOAT  : ok = ok && QPUInstr_SetMulOperandFloat(&self->m_instr, OperandNumber_FIRST, QPUOperand_ValueSmallFloat(leftOperand)); self->m_sigUsed = true; break;

   default:
      break;
   }

   switch (QPUOperand_GetType(rightOperand))
   {
   case QPUOperand_NONE         : break;
   case QPUOperand_REGISTER     : ok = ok && QPUInstr_SetMulOperandReg(&self->m_instr, OperandNumber_SECOND, QPUOperand_ValueRegister(rightOperand)); break;
   case QPUOperand_SMALL_INT    : ok = ok && QPUInstr_SetMulOperandInt(&self->m_instr, OperandNumber_SECOND, QPUOperand_ValueSmallInt(rightOperand)); self->m_sigUsed = true; break;
   case QPUOperand_SMALL_FLOAT  : ok = ok && QPUInstr_SetMulOperandFloat(&self->m_instr, OperandNumber_SECOND, QPUOperand_ValueSmallFloat(rightOperand)); self->m_sigUsed = true; break;

   default:
      break;
   }

   if (ok)
      self->m_mulUsed = true;

   return ok;
}

bool QPUGenericInstr_SetMovOpResEx(QPUGenericInstr *self, const QPUOperand *from, QPUResource *to, QPUGenericInstr_MovUnit unit, QPUGenericInstr_MovUnit *unitUsed)
{
   QPUOperand  op;
   QPUOperand_ConstrResource(&op, to);
   return QPUGenericInstr_SetMovOpOpEx(self, from, &op, unit, unitUsed);
}

bool QPUGenericInstr_SetMovOpOp(QPUGenericInstr *self, const QPUOperand *from, const QPUOperand *to)
{
   return QPUGenericInstr_SetMovOpOpEx(self, from, to, QPUGenericInstr_PREFER_ADD, NULL);
}

bool QPUGenericInstr_SetMovOpRes(QPUGenericInstr *self, const QPUOperand *from, QPUResource *to)
{
   return QPUGenericInstr_SetMovOpResEx(self, from, to, QPUGenericInstr_PREFER_ADD, NULL);
}

bool QPUGenericInstr_SetMovOpOpEx(QPUGenericInstr *self, const QPUOperand *from, const QPUOperand *to, QPUGenericInstr_MovUnit unit, QPUGenericInstr_MovUnit *unitUsed)
{
   bool unpackFloat;
   bool ok = true;

   vcos_assert(QPUOperand_IsRegister(to));
   vcos_assert(QPUOperand_ValueRegister(to) != Register_ACC4 && QPUOperand_ValueRegister(to) != Register_ACC5);

   NeedsALU(self);

   if (QPUInstr_GetType(&self->m_instr) != QPUInstr_ALU)
      return false;

   if (self->m_adderUsed && self->m_mulUsed)
      return false;

   if (!ProcessUnpack(self, from))
      return false;

   if (!CheckUnpackClash(self, from))
      return false;

   unpackFloat = VirtualUnpack_IsFloat(QPUOperand_GetUnpack(from));

   if (unit == QPUGenericInstr_PREFER_ADD || unit == QPUGenericInstr_PREFER_MUL)
   {
      if (unpackFloat)
         unit = QPUGenericInstr_ADD; // Can only do a mov with float unpack in the adder
      else
      {
         if (unit == QPUGenericInstr_PREFER_ADD && self->m_adderUsed)
            unit = QPUGenericInstr_MUL;

         if (unit == QPUGenericInstr_PREFER_MUL && self->m_mulUsed)
            unit = QPUGenericInstr_ADD;

         if (unit == QPUGenericInstr_PREFER_ADD)
            unit = QPUGenericInstr_ADD;

         if (unit == QPUGenericInstr_PREFER_MUL)
            unit = QPUGenericInstr_MUL;
      }
   }

   if (unit == QPUGenericInstr_ADD && self->m_adderUsed)
      return false;

   if (unit == QPUGenericInstr_MUL && self->m_mulUsed)
      return false;

   if (self->m_sigUsed && QPUOperand_IsSmallConst(from))
      return false; // Small immediates use signaling bits

   if (unit == QPUGenericInstr_ADD)
   {
      vcos_assert(!self->m_adderUsed);

      ok = ok && QPUInstr_SetAddOperation(&self->m_instr, unpackFloat ? AOP_FMIN : AOP_OR);
      ok = ok && QPUInstr_SetAddCondition(&self->m_instr, CondCode_ALWAYS);

      switch (QPUOperand_GetType(from))
      {
      case QPUOperand_NONE         : return false;
      case QPUOperand_REGISTER     : ok = ok && QPUInstr_SetAddOperandReg(&self->m_instr, OperandNumber_FIRST,  QPUOperand_ValueRegister(from));
                                     ok = ok && QPUInstr_SetAddOperandReg(&self->m_instr, OperandNumber_SECOND, QPUOperand_ValueRegister(from)); break;
      case QPUOperand_SMALL_INT    : ok = ok && QPUInstr_SetAddOperandInt(&self->m_instr, OperandNumber_FIRST,  QPUOperand_ValueSmallInt(from));
                                     ok = ok && QPUInstr_SetAddOperandReg(&self->m_instr, OperandNumber_SECOND, Register_SMALL_IMMED); self->m_sigUsed = true; break;
      case QPUOperand_SMALL_FLOAT  : ok = ok && QPUInstr_SetAddOperandFloat(&self->m_instr, OperandNumber_FIRST,  QPUOperand_ValueSmallFloat(from));
                                     ok = ok && QPUInstr_SetAddOperandReg(&self->m_instr, OperandNumber_SECOND, Register_SMALL_IMMED); self->m_sigUsed = true; break;

      default:
         break;
      }

      switch (QPUOperand_GetType(to))
      {
      case QPUOperand_NONE         : return false;
      case QPUOperand_REGISTER     : ok = ok && QPUInstr_SetAddOutputRegister(&self->m_instr, QPUOperand_ValueRegister(to)); break;
      case QPUOperand_SMALL_INT    :
      case QPUOperand_SMALL_FLOAT  : return false;

      default:
         break;
      }

      if (ok)
         self->m_adderUsed = true;
   }
   else if (unit == QPUGenericInstr_MUL)
   {
      vcos_assert(!self->m_mulUsed);

      ok = ok && QPUInstr_SetMulOperation(&self->m_instr, MOP_V8MIN);
      ok = ok && QPUInstr_SetMulCondition(&self->m_instr, CondCode_ALWAYS);

      switch (QPUOperand_GetType(from))
      {
      case QPUOperand_NONE         : return false;
      case QPUOperand_REGISTER     : ok = ok && QPUInstr_SetMulOperandReg(&self->m_instr, OperandNumber_FIRST,  QPUOperand_ValueRegister(from));
                                     ok = ok && QPUInstr_SetMulOperandReg(&self->m_instr, OperandNumber_SECOND, QPUOperand_ValueRegister(from)); break;
      case QPUOperand_SMALL_INT    : ok = ok && QPUInstr_SetMulOperandInt(&self->m_instr, OperandNumber_FIRST,  QPUOperand_ValueSmallInt(from));
                                     ok = ok && QPUInstr_SetMulOperandReg(&self->m_instr, OperandNumber_SECOND, Register_SMALL_IMMED); self->m_sigUsed = true; break;
      case QPUOperand_SMALL_FLOAT  : ok = ok && QPUInstr_SetMulOperandFloat(&self->m_instr, OperandNumber_FIRST,  QPUOperand_ValueSmallFloat(from));
                                     ok = ok && QPUInstr_SetMulOperandReg(&self->m_instr, OperandNumber_SECOND, Register_SMALL_IMMED); self->m_sigUsed = true; break;

      default:
         break;
      }

      switch (QPUOperand_GetType(to))
      {
      case QPUOperand_NONE         : return false;
      case QPUOperand_REGISTER     : ok = ok && QPUInstr_SetMulOutputRegister(&self->m_instr, QPUOperand_ValueRegister(to)); break;
      case QPUOperand_SMALL_INT    :
      case QPUOperand_SMALL_FLOAT  : return false;

      default:
         break;
      }

      if (ok)
         self->m_mulUsed = true;
   }
   else
      vcos_assert(0);

   if (unitUsed != NULL)
      *unitUsed = unit;

   return ok;
}

bool QPUGenericInstr_SetMovIntRes(QPUGenericInstr *self, int32_t immed, QPUResource *to)
{
   QPUOperand  op;
   QPUOperand_ConstrResource(&op, to);
   return QPUGenericInstr_SetMovIntOp(self, immed, &op);
}

bool QPUGenericInstr_SetMovIntOp(QPUGenericInstr *self, int32_t immed, const QPUOperand *to)
{
   bool ok = true;

   vcos_assert(QPUOperand_IsRegister(to));
   vcos_assert(QPUOperand_ValueRegister(to) != Register_ACC4 && QPUOperand_ValueRegister(to) != Register_ACC5);

   NeedsLDI32(self);

   if (QPUInstr_GetType(&self->m_instr) != QPUInstr_IMMED32)
      return false;

   if (self->m_immedUsed)
      return false;

   if (!CheckUnpackClash(self, to))
      return false;

   ok = ok && QPUInstr_SetImmediateInt32(&self->m_instr, *(uint32_t*)&immed, false);
   ok = ok && QPUInstr_SetAddCondition(&self->m_instr, CondCode_ALWAYS);
   ok = ok && QPUInstr_SetMulCondition(&self->m_instr, CondCode_NEVER);

   switch (QPUOperand_GetType(to))
   {
   case QPUOperand_NONE         : return false;
   case QPUOperand_REGISTER     : ok = ok && QPUInstr_SetAddOutputRegister(&self->m_instr, QPUOperand_ValueRegister(to)); break;
   case QPUOperand_SMALL_INT    :
   case QPUOperand_SMALL_FLOAT  : return false;

   default:
      break;
   }

   if (ok)
      self->m_immedUsed = true;

   return ok;
}

bool QPUGenericInstr_SetMovFloatRes(QPUGenericInstr *self, float immed, QPUResource *to)
{
   return QPUGenericInstr_SetMovIntRes(self, *(int32_t*)&immed, to);
}

bool QPUGenericInstr_SetMovFloatOp(QPUGenericInstr *self, float immed, const QPUOperand *to)
{
   return QPUGenericInstr_SetMovIntOp(self, *(int32_t*)&immed, to);
}

bool QPUGenericInstr_SetUnpack(QPUGenericInstr *self, VirtualUnpack_Enum mode, UnpackSource_Enum src)
{
   NeedsALU(self);

   if (QPUInstr_GetType(&self->m_instr) != QPUInstr_ALU)
      return false;

   if (self->m_unpackUsed)
      return false;

   self->m_unpackUsed = true;
   self->m_virtualUnpack = mode;
   self->m_unpackSrc = src;

   return QPUInstr_SetUnpack(&self->m_instr, mode, src);
}

bool QPUGenericInstr_SetSignal(QPUGenericInstr *self, Sig_Enum sig)
{
   NeedsALU(self);

   if (QPUInstr_GetType(&self->m_instr) != QPUInstr_ALU)
      return false;

   if (self->m_sigUsed)
   {
      // Thrend and loadc can be merged
      if (sig == Sig_COLLOAD_PRGEND)
      {
         Sig_Enum curSig = QPUInstr_GetSignal(&self->m_instr);
         if (curSig != Sig_COLLOAD && curSig != Sig_END)
            return false;
         else
            QPUInstr_ClearSignal(&self->m_instr);
      }
      else
         return false;
   }

   self->m_sigUsed = true;

   return QPUInstr_SetSignal(&self->m_instr, sig);
}

void QPUGenericInstr_ClearSignal(QPUGenericInstr *self)
{
   QPUInstr_ClearSignal(&self->m_instr);
   self->m_sigUsed = false;
}

bool QPUGenericInstr_SetSemaphore(QPUGenericInstr *self, uint8_t semaphoreNumber, Semaphore_Enum incOrDec)
{
   NeedsSem(self);

   if (QPUInstr_GetType(&self->m_instr) != QPUInstr_SEMAPHORE)
      return false;

   return QPUInstr_SetSemaphore(&self->m_instr, semaphoreNumber, incOrDec);
}

bool QPUGenericInstr_SetPackedMovRegA(QPUGenericInstr *self, const QPUOperand *from, const QPUOperand *to, RegA_Pack_Enum mode)
{
   Register_File destFile, rf;

   if (self->m_packUsed)
      return false;

   if (!ProcessUnpack(self, from))
      return false;

   if (!CheckUnpackClash(self, from))
      return false;

   destFile = Register_GetFile(QPUOperand_ValueRegister(to));
   if (destFile == Register_FILE_B)
      return false;

   rf = QPUInstr_GetFreeRegisterBanks(&self->m_instr);
   if (rf == Register_FILE_EITHER || rf == Register_FILE_A)
   {
      if (QPUGenericInstr_SetMovOpOp(self, from, to))
      {
         self->m_packUsed = true;
         vcos_verify(QPUInstr_SetPackRegA(&self->m_instr, mode));
         self->m_packTarget = PackTarget_REGFILE_A;
         return true;
      }
   }

   return false;
}

bool QPUGenericInstr_SetPackedMovMul(QPUGenericInstr *self, const QPUOperand *from, const QPUOperand *to, Mul_Pack_Enum mode)
{
   NeedsALU(self);

   if (self->m_packUsed)
      return false;

   if (self->m_mulUsed)
      return false;

   if (QPUGenericInstr_SetMovOpOpEx(self, from, to, QPUGenericInstr_MUL, 0)) // Must use the multiplier
   {
      self->m_packUsed = true;
      vcos_verify(QPUInstr_SetPackMul(&self->m_instr, mode));
      self->m_packTarget = PackTarget_MUL_OUT;
      return true;
   }

   return false;
}


bool QPUGenericInstr_SetPackRegA(QPUGenericInstr *self, RegA_Pack_Enum mode)
{
   Register_File rf;

   NeedsALU(self);

   if (QPUInstr_GetType(&self->m_instr) != QPUInstr_ALU)
      return false;

   if (self->m_unpackUsed)
   {
      if (self->m_unpackSrc != UnpackSource_REGFILE_A)
         return false;
   }

   if (self->m_packUsed)
      return false;

   rf = QPUInstr_GetFreeRegisterBanks(&self->m_instr);
   if (rf == Register_FILE_EITHER || rf == Register_FILE_A)
   {
      self->m_packUsed = true;
      vcos_verify(QPUInstr_SetPackRegA(&self->m_instr, mode));
      self->m_packTarget = PackTarget_REGFILE_A;
      return true;
   }

   return false;
}

bool QPUGenericInstr_SetPackMul(QPUGenericInstr *self, Mul_Pack_Enum mode)
{
   NeedsALU(self);

   if (QPUInstr_GetType(&self->m_instr) != QPUInstr_ALU)
      return false;

   if (self->m_unpackUsed)
   {
      if (self->m_unpackSrc != UnpackSource_R4)
         return false;
   }

   if (self->m_packUsed)
      return false;

   // Note: This could be called before or after the mul operation has been inserted in m_gi, so don't check this
   // if (m_mulUsed)
   //    return false;

   self->m_packUsed = true;
   vcos_verify(QPUInstr_SetPackMul(&self->m_instr, mode));
   self->m_packTarget = PackTarget_MUL_OUT;

   return true;
}

Register_File QPUGenericInstr_GetFreeOutputRegisters(QPUGenericInstr *self)
{
   return QPUInstr_GetFreeRegisterBanks(&self->m_instr);
}

//////////////////////////////////////////////////////////////////////////////
// QPUOperand
//////////////////////////////////////////////////////////////////////////////

void QPUOperand_ConstrCopy(QPUOperand *self, const QPUOperand *rhs)
{
   vcos_assert(self != NULL);
   vcos_assert(rhs  != NULL);

   self->m_type    = rhs->m_type;
   self->m_reg     = rhs->m_reg;
   self->m_i8      = rhs->m_i8;
   self->m_sf      = rhs->m_sf;
   self->m_unpack  = rhs->m_unpack;
}

void QPUOperand_ConstrReg(QPUOperand *self, Register_Enum reg)
{
   vcos_assert(self != NULL);

   self->m_type   = QPUOperand_REGISTER;
   self->m_reg    = reg;
   self->m_unpack = VirtualUnpack_NONE;
}

void QPUOperand_ConstrType(QPUOperand *self, QPUOperand_Type t, VirtualUnpack_Enum unpack)
{
   vcos_assert(self != NULL);

   self->m_type   = t;
   self->m_reg    = Register_UNKNOWN;
   self->m_unpack = unpack;
}

void QPUOperand_ConstrResource(QPUOperand *self, QPUResource *res)
{
   vcos_assert(self != NULL);

   self->m_type = QPUOperand_REGISTER;
   self->m_reg  = QPUResource_Name(res);
   self->m_unpack = VirtualUnpack_NONE;
}

void QPUOperand_ConstrInt(QPUOperand *self, int8_t i)
{
   vcos_assert(self != NULL);

   self->m_unpack = VirtualUnpack_NONE;

   if (QPUInstr_IsValidSmallImmediateInt8(i))
   {
      self->m_type = QPUOperand_SMALL_INT;
      self->m_i8 = i;
   }
   else
      vcos_assert(0);
}

void QPUOperand_ConstrFloat(QPUOperand *self, float f)
{
   vcos_assert(self != NULL);

   self->m_unpack = VirtualUnpack_NONE;

   if (QPUInstr_IsValidSmallImmediateFloat(f))
   {
      self->m_type = QPUOperand_SMALL_FLOAT;
      self->m_sf = f;
   }
   else
      vcos_assert(0);
}

bool QPUOperand_IsReadable(const QPUOperand *self, int32_t atSlot, const QPUResources *res)
{
   vcos_assert(self != NULL);

   if (self->m_type == QPUOperand_REGISTER)
      return QPUResources_IsReadable(res, self->m_reg, atSlot);

   return true;
}

bool QPUOperand_SetUnpack(QPUOperand *self, VirtualUnpack_Enum unpack)
{
   vcos_assert(self != NULL);

   if (self->m_type != QPUOperand_REGISTER)
      return false;

   self->m_unpack = unpack;

   return true;
}

void QPUOperand_Retire(const QPUOperand *self, QPUResources *res)
{
   vcos_assert(self != NULL);
   vcos_assert(self->m_type != QPUOperand_BYPASS);

   if (self->m_type == QPUOperand_REGISTER)
      QPUResource_Unreference(QPUResources_GetResource(res, self->m_reg));
}
