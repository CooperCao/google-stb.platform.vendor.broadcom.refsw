/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdint.h>
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_qpu_enum.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_registers.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_exception.h"

struct Uniform_s;

typedef enum
{
   QPUInstr_UNKNOWN,
   QPUInstr_ALU,
   QPUInstr_IMMED32,
   QPUInstr_IMMED_PER_ELEMENT,
   QPUInstr_SEMAPHORE,
   QPUInstr_BRANCH
}  QPUInstr_Type;

typedef struct
{
   // Generic fields
   QPUInstr_Type     m_type;
   uint64_t          m_coding;
   Register_Enum     m_addOutReg;
   Register_Enum     m_mulOutReg;

   // Non-branch instruction fields
   bool m_packPMSet;
   bool m_unpackPMSet;

   // ALU instructions fields
   Register_Enum        m_addInput[2];
   Register_Enum        m_mulInput[2];
   bool                 m_usingSmallImmediate;
   VirtualUnpack_Enum   m_virtualUnpack;

   // Branch instruction fields
   // TODO
} QPUInstr;

void QPUInstr_Constr(QPUInstr *self, QPUInstr_Type type);
void QPUInstr_Copy(QPUInstr *self, const QPUInstr *rhs);

uint64_t QPUInstr_GetCoding(QPUInstr *self);

// These use a static so are not thread safe
const char *QPUInstr_Disassemble(QPUInstr *self);
const char *QPUInstr_DisassembleEx(QPUInstr *self, uint32_t numUniforms, const struct Uniform_s *uniforms, uint32_t *curUnif);

static inline Register_Enum  QPUInstr_GetAddOutRegister(const QPUInstr *self) { return self->m_addOutReg; }
static inline Register_Enum  QPUInstr_GetMulOutRegister(const QPUInstr *self) { return self->m_mulOutReg; }

bool QPUInstr_HasInvalidReadsForProgramEnd(const QPUInstr *self);
bool QPUInstr_WritesTo(const QPUInstr *self, Register_Enum reg);
bool QPUInstr_ReadsFrom(const QPUInstr *self, Register_Enum reg);
bool QPUInstr_HasImplicitSBWait(const QPUInstr *self);

bool QPUInstr_SetAddOutputRegister(QPUInstr *self, Register_Enum reg);   // Writes waddr_add and ws
bool QPUInstr_SetMulOutputRegister(QPUInstr *self, Register_Enum reg);   // Writes waddr_mul and ws

static inline QPUInstr_Type QPUInstr_GetType(const QPUInstr *self) { return self->m_type; }

Register_File QPUInstr_GetFreeRegisterBanks(QPUInstr *self);

// Non-branch interfaces
bool QPUInstr_SetFlags(QPUInstr *self, bool val);
bool QPUInstr_SetAddCondition(QPUInstr *self, CondCode_Enum val);
bool QPUInstr_SetMulCondition(QPUInstr *self, CondCode_Enum val);

bool QPUInstr_SetPackRegA(QPUInstr *self, RegA_Pack_Enum mode);
bool QPUInstr_SetPackMul(QPUInstr *self, Mul_Pack_Enum mode);

// ALU interfaces
bool QPUInstr_SetUnpack(QPUInstr *self, VirtualUnpack_Enum mode, UnpackSource_Enum src);
bool QPUInstr_SetSignal(QPUInstr *self, Sig_Enum sig);
void QPUInstr_ClearSignal(QPUInstr *self);
Sig_Enum QPUInstr_GetSignal(const QPUInstr *self);

bool QPUInstr_SetAddOperandReg(QPUInstr *self, OperandNumber_Enum which, Register_Enum reg);
bool QPUInstr_SetMulOperandReg(QPUInstr *self, OperandNumber_Enum which, Register_Enum reg);

bool QPUInstr_SetAddOperandInt(QPUInstr *self, OperandNumber_Enum which, int8_t i);
bool QPUInstr_SetAddOperandFloat(QPUInstr *self, OperandNumber_Enum which, float i);
bool QPUInstr_SetMulOperandInt(QPUInstr *self, OperandNumber_Enum which, int8_t i);
bool QPUInstr_SetMulOperandFloat(QPUInstr *self, OperandNumber_Enum which, float i);

bool QPUInstr_SetAddOperation(QPUInstr *self, AOP_Enum op);
bool QPUInstr_SetMulOperation(QPUInstr *self, MOP_Enum op);

// "static" methods
//bool QPUInstr_IsValidSmallImmediateInt32(int32_t i);
bool QPUInstr_IsValidSmallImmediateInt8(int8_t i);
bool QPUInstr_IsValidSmallImmediateFloat(float f);

// Immediate
bool QPUInstr_SetImmediateInt32(QPUInstr *self, uint32_t i, bool perElementSigned);

//////////////////////////////////////////////////////////////////////////////////

typedef enum
{
   QPUOperand_NONE,
   QPUOperand_REGISTER,
   QPUOperand_SMALL_INT,
   QPUOperand_SMALL_FLOAT,
   QPUOperand_BYPASS
} QPUOperand_Type;

typedef struct QPUOperand_s
{
   QPUOperand_Type      m_type;
   Register_Enum        m_reg;
   int8_t               m_i8;
   float                m_sf;
   VirtualUnpack_Enum   m_unpack;
} QPUOperand;

//void QPUOperand_QPUOperand(QPUOperand *self);
void QPUOperand_ConstrCopy(QPUOperand *self, const QPUOperand *rhs);
void QPUOperand_ConstrInt(QPUOperand *self, int8_t i);
void QPUOperand_ConstrFloat(QPUOperand *self, float f);
void QPUOperand_ConstrReg(QPUOperand *self, Register_Enum reg);
void QPUOperand_ConstrType(QPUOperand *self, QPUOperand_Type t, VirtualUnpack_Enum unpack);
void QPUOperand_ConstrResource(QPUOperand *self, QPUResource *res);

bool QPUOperand_IsReadable(const QPUOperand *self, int32_t atSlot, const QPUResources *res);
void QPUOperand_Retire(const QPUOperand *self, QPUResources *res);
bool QPUOperand_SetUnpack(QPUOperand *self, VirtualUnpack_Enum unpack);

static inline QPUOperand_Type     QPUOperand_GetType(const QPUOperand *self)         { return self->m_type;    }
static inline float               QPUOperand_ValueSmallFloat(const QPUOperand *self) { return self->m_sf;      }
static inline int8_t              QPUOperand_ValueSmallInt(const QPUOperand *self)   { return self->m_i8;      }
static inline Register_Enum       QPUOperand_ValueRegister(const QPUOperand *self)   { return self->m_reg;     }
static inline VirtualUnpack_Enum  QPUOperand_GetUnpack(const QPUOperand *self)       { return self->m_unpack;  }

static inline bool QPUOperand_IsBypass(const QPUOperand *self)     { return self->m_type == QPUOperand_BYPASS;         }
static inline bool QPUOperand_IsRegister(const QPUOperand *self)   { return self->m_type == QPUOperand_REGISTER;       }
static inline bool QPUOperand_IsSmallInt(const QPUOperand *self)   { return self->m_type == QPUOperand_SMALL_INT;      }
static inline bool QPUOperand_IsSmallFloat(const QPUOperand *self) { return self->m_type == QPUOperand_SMALL_FLOAT;    }
static inline bool QPUOperand_IsSmallConst(const QPUOperand *self) { return QPUOperand_IsSmallInt(self) || QPUOperand_IsSmallFloat(self);    }

//////////////////////////////////////////////////////////////////////////////////

#define DISASM_BUFFER_SIZE 128

typedef enum
{
   QPUGenericInstr_PREFER_ADD,
   QPUGenericInstr_PREFER_MUL,
   QPUGenericInstr_ADD,
   QPUGenericInstr_MUL
} QPUGenericInstr_MovUnit;

// Wrapper over QPUInstr types
typedef struct
{
   QPUInstr            m_instr;
   bool                m_adderUsed;
   bool                m_mulUsed;
   bool                m_sigUsed;
   bool                m_immedUsed;
   bool                m_packUsed;

   bool                m_unpackUsed;
   VirtualUnpack_Enum  m_virtualUnpack;
   UnpackSource_Enum   m_unpackSrc;
   PackTarget_Enum     m_packTarget;

   bool                m_full;        // Flagged when we don't want anything else in this instruction
} QPUGenericInstr;

// Constructors and assign
void QPUGenericInstr_Constr(QPUGenericInstr *self);
void QPUGenericInstr_ConstrType(QPUGenericInstr *self, QPUInstr_Type type);
void QPUGenericInstr_ConstrCopy(QPUGenericInstr *self, const QPUGenericInstr *rhs);
void QPUGenericInstr_Destr(QPUGenericInstr *self);
void QPUGenericInstr_Assign(QPUGenericInstr *self, const QPUGenericInstr *rhs);

// Operations
static inline QPUInstr_Type   QPUGenericInstr_GetType(QPUGenericInstr *self)                   { return QPUInstr_GetType(&self->m_instr);      }
static inline QPUInstr *      QPUGenericInstr_GetInstruction(QPUGenericInstr *self)            { return &self->m_instr;                        }
static inline const QPUInstr *QPUGenericInstr_GetInstructionConst(const QPUGenericInstr *self) { return &self->m_instr;                    }
static inline uint64_t        QPUGenericInstr_GetCoding(QPUGenericInstr *self)                 { return QPUInstr_GetCoding(&self->m_instr);    }

// Not thread safe
static inline const char     *QPUGenericInstr_Disassemble(QPUGenericInstr *self)               { return QPUInstr_Disassemble(&self->m_instr);  }

Register_File          QPUGenericInstr_GetFreeOutputRegisters(QPUGenericInstr *self);

static inline bool QPUGenericInstr_IsFull(const QPUGenericInstr *self)         { return self->m_full;       }
static inline void QPUGenericInstr_SetFull(QPUGenericInstr *self)              { self->m_full = true;       }

static inline bool QPUGenericInstr_AdderUsed(const QPUGenericInstr *self)     { return self->m_adderUsed;  }
static inline bool QPUGenericInstr_MulUsed(const QPUGenericInstr *self)       { return self->m_mulUsed;    }
static inline bool QPUGenericInstr_SigUsed(const QPUGenericInstr *self)       { return self->m_sigUsed;    }
static inline bool QPUGenericInstr_ImmedUsed(const QPUGenericInstr *self)     { return self->m_immedUsed;  }
static inline bool QPUGenericInstr_UnpackUsed(const QPUGenericInstr *self)    { return self->m_unpackUsed; }

static inline bool QPUGenericInstr_SetAddOutputRegister(QPUGenericInstr *self, Register_Enum reg)       { return QPUInstr_SetAddOutputRegister(&self->m_instr, reg);                    }
static inline bool QPUGenericInstr_SetMulOutputRegister(QPUGenericInstr *self, Register_Enum reg)       { return QPUInstr_SetMulOutputRegister(&self->m_instr, reg);                    }
static inline bool QPUGenericInstr_SetAddOutputRegisterRes(QPUGenericInstr *self, const QPUResource *reg)  { return QPUInstr_SetAddOutputRegister(&self->m_instr, QPUResource_Name(reg));  }
static inline bool QPUGenericInstr_SetMulOutputRegisterRes(QPUGenericInstr *self, const QPUResource *reg)  { return QPUInstr_SetMulOutputRegister(&self->m_instr, QPUResource_Name(reg));  }

// Non-branch interfaces (ALU, LoadImmed & Semaphore)
static inline bool QPUGenericInstr_SetFlags(QPUGenericInstr *self, bool val)                 { return QPUInstr_SetFlags(&self->m_instr, val);         }
static inline bool QPUGenericInstr_SetAddCondition(QPUGenericInstr *self, CondCode_Enum val) { return QPUInstr_SetAddCondition(&self->m_instr, val);  }
static inline bool QPUGenericInstr_SetMulCondition(QPUGenericInstr *self, CondCode_Enum val) { return QPUInstr_SetMulCondition(&self->m_instr, val);  }

bool QPUGenericInstr_SetPackedMovRegA(QPUGenericInstr *self, const QPUOperand *from, const QPUOperand *to, RegA_Pack_Enum mode);
bool QPUGenericInstr_SetPackedMovMul(QPUGenericInstr *self, const QPUOperand *from, const QPUOperand *to, Mul_Pack_Enum mode);

bool QPUGenericInstr_SetPackRegA(QPUGenericInstr *self, RegA_Pack_Enum mode);
bool QPUGenericInstr_SetPackMul(QPUGenericInstr *self, Mul_Pack_Enum mode);

// ALU interfaces
bool QPUGenericInstr_SetUnpack(QPUGenericInstr *self, VirtualUnpack_Enum mode, UnpackSource_Enum src);
bool QPUGenericInstr_SetSignal(QPUGenericInstr *self, Sig_Enum sig);
void QPUGenericInstr_ClearSignal(QPUGenericInstr *self);

static inline UnpackSource_Enum QPUGenericInstr_UnpackSrc(const QPUGenericInstr *self) { return self->m_unpackSrc; }

bool QPUGenericInstr_SetAdd(QPUGenericInstr *self, AOP_Enum op, const QPUOperand *leftOperand, const QPUOperand *rightOperand);
bool QPUGenericInstr_SetMul(QPUGenericInstr *self, MOP_Enum op, const QPUOperand *leftOperand, const QPUOperand *rightOperand);

bool QPUGenericInstr_SetMovOpRes(QPUGenericInstr *self, const QPUOperand *from, QPUResource *to);
bool QPUGenericInstr_SetMovOpOp(QPUGenericInstr *self, const QPUOperand *from, const QPUOperand *to);

bool QPUGenericInstr_SetMovOpResEx(QPUGenericInstr *self, const QPUOperand *from, QPUResource *to,     QPUGenericInstr_MovUnit unit, QPUGenericInstr_MovUnit *unitUsed);
bool QPUGenericInstr_SetMovOpOpEx(QPUGenericInstr *self, const QPUOperand *from, const QPUOperand *to, QPUGenericInstr_MovUnit unit, QPUGenericInstr_MovUnit *unitUsed);

bool QPUGenericInstr_SetMovIntRes(QPUGenericInstr *self, int32_t immed, QPUResource *to);
bool QPUGenericInstr_SetMovIntOp(QPUGenericInstr *self, int32_t immed, const QPUOperand *to);

bool QPUGenericInstr_SetMovFloatRes(QPUGenericInstr *self, float immed, QPUResource *to);
bool QPUGenericInstr_SetMovFloatOp(QPUGenericInstr *self, float immed, const QPUOperand *to);

// Semaphore interfaces
bool QPUGenericInstr_SetSemaphore(QPUGenericInstr *self, uint8_t semaphoreNumber, Semaphore_Enum incOrDec);

// Branch interfaces
// TODO
