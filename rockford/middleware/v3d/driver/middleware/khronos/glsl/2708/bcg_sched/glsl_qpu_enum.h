/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

=============================================================================*/

#ifndef __GLSL_QPU_ENUM_H__
#define __GLSL_QPU_ENUM_H__

#include "interface/khronos/common/khrn_int_common.h"
#include <stdint.h>

typedef enum
{
   AOP_NOP     = 0 ,

   AOP_FADD    = 1 ,
   AOP_FSUB    = 2 ,
   AOP_FMIN    = 3 ,
   AOP_FMAX    = 4 ,
   AOP_FMINABS = 5 ,
   AOP_FMAXABS = 6 ,
   AOP_FTOI    = 7 ,
   AOP_ITOF    = 8 ,

   AOP_ADD     = 12,
   AOP_SUB     = 13,
   AOP_SHR     = 14,
   AOP_ASR     = 15,
   AOP_ROR     = 16,
   AOP_SHL     = 17,
   AOP_MIN     = 18,
   AOP_MAX     = 19,

   AOP_AND     = 20,
   AOP_OR      = 21,
   AOP_XOR     = 22,
   AOP_NOT     = 23,
   AOP_CLZ     = 24,

   AOP_V8ADDS  = 30,
   AOP_V8SUBS  = 31
} AOP_Enum;

static bool AOP_HasFloatInput(AOP_Enum e)
{
   switch (e)
   {
   case AOP_FADD    :
   case AOP_FSUB    :
   case AOP_FMIN    :
   case AOP_FMAX    :
   case AOP_FMINABS :
   case AOP_FMAXABS :
   case AOP_FTOI    :
      return true;

   default:
      return false;
   }
}

typedef enum
{
   MOP_NOP    = 0,
   MOP_FMUL   = 1,
   MOP_MUL24  = 2,
   MOP_V8MULD = 3,
   MOP_V8MIN  = 4,
   MOP_V8MAX  = 5,
   MOP_V8ADDS = 6,
   MOP_V8SUBS = 7
} MOP_Enum;

static INLINE bool MOP_HasFloatInput(MOP_Enum e)
{
   return e == MOP_FMUL;
}

typedef enum
{
   UnpackSource_REGFILE_A = 0,
   UnpackSource_R4
} UnpackSource_Enum;

typedef enum
{
   PackTarget_REGFILE_A = 0,
   PackTarget_MUL_OUT
} PackTarget_Enum;

typedef enum
{
   Unpack_NONE  = 0x00,
   Unpack_16A   = 0x01,
   Unpack_16B   = 0x02,
   Unpack_8R    = 0x03,
   Unpack_8A    = 0x04,
   Unpack_8B    = 0x05,
   Unpack_8C    = 0x06,
   Unpack_8D    = 0x07
} Unpack_Enum;

typedef enum
{
   VirtualUnpack_NONE,
   VirtualUnpack_COL_R,
   VirtualUnpack_COL_G,
   VirtualUnpack_COL_B,
   VirtualUnpack_COL_A,
   VirtualUnpack_16A,
   VirtualUnpack_16A_F,
   VirtualUnpack_16B,
   VirtualUnpack_16B_F,
   VirtualUnpack_8A,
   VirtualUnpack_8B,
   VirtualUnpack_8C,
   VirtualUnpack_8D,
   VirtualUnpack_8R
} VirtualUnpack_Enum;

static Unpack_Enum VirtualUnpack_RealUnpackCode(VirtualUnpack_Enum e)
{
   switch (e)
   {
   default           :
   case VirtualUnpack_NONE  : return Unpack_NONE;
   case VirtualUnpack_COL_R : return Unpack_8A;
   case VirtualUnpack_COL_G : return Unpack_8B;
   case VirtualUnpack_COL_B : return Unpack_8C;
   case VirtualUnpack_COL_A : return Unpack_8D;
   case VirtualUnpack_16A   : return Unpack_16A;
   case VirtualUnpack_16A_F : return Unpack_16A;
   case VirtualUnpack_16B   : return Unpack_16B;
   case VirtualUnpack_16B_F : return Unpack_16B;
   case VirtualUnpack_8A    : return Unpack_8A;
   case VirtualUnpack_8B    : return Unpack_8B;
   case VirtualUnpack_8C    : return Unpack_8C;
   case VirtualUnpack_8D    : return Unpack_8D;
   case VirtualUnpack_8R    : return Unpack_8R;
   }
}

static bool VirtualUnpack_IsFloat(VirtualUnpack_Enum e)
{
   switch (e)
   {
   case VirtualUnpack_COL_R :
   case VirtualUnpack_COL_G :
   case VirtualUnpack_COL_B :
   case VirtualUnpack_COL_A :
   case VirtualUnpack_16A_F :
   case VirtualUnpack_16B_F :
      return true;

   default:
      return false;
   }
}

static bool VirtualUnpack_IsR4Compatible(VirtualUnpack_Enum e)
{
   switch (e)
   {
   case VirtualUnpack_COL_R :
   case VirtualUnpack_COL_G :
   case VirtualUnpack_COL_B :
   case VirtualUnpack_COL_A :
   case VirtualUnpack_8R    :
   case VirtualUnpack_16A_F :
   case VirtualUnpack_16B_F :
      return true;

   default:
      return false;
   }
}

typedef enum
{
   RegA_Pack_NONE    = 0x00,
   RegA_Pack_16A     = 0x01,
   RegA_Pack_16B     = 0x02,
   RegA_Pack_8R      = 0x03,
   RegA_Pack_8A      = 0x04,
   RegA_Pack_8B      = 0x05,
   RegA_Pack_8C      = 0x06,
   RegA_Pack_8D      = 0x07,
   RegA_Pack_S       = 0x08,
   RegA_Pack_16A_S   = 0x09,
   RegA_Pack_16B_S   = 0x0a,
   RegA_Pack_8R_S    = 0x0b,
   RegA_Pack_8A_S    = 0x0c,
   RegA_Pack_8B_S    = 0x0d,
   RegA_Pack_8C_S    = 0x0e,
   RegA_Pack_8D_S    = 0x0f
} RegA_Pack_Enum;

typedef enum
{
   Mul_Pack_NONE    = 0x00,
   Mul_Pack_8R      = 0x03,
   Mul_Pack_8A      = 0x04,
   Mul_Pack_8B      = 0x05,
   Mul_Pack_8C      = 0x06,
   Mul_Pack_8D      = 0x07
} Mul_Pack_Enum;

typedef enum
{
   OperandNumber_FIRST = 0,
   OperandNumber_SECOND
} OperandNumber_Enum;

typedef enum
{
   InputMux_ACC0 = 0,
   InputMux_ACC1 = 1,
   InputMux_ACC2 = 2,
   InputMux_ACC3 = 3,
   InputMux_ACC4 = 4,
   InputMux_ZERO = 5,
   InputMux_REGA = 6,
   InputMux_REGB = 7,
   InputMux_REGA_OR_B = 999   // Special case for when the input can come from either regfile
} InputMux_Enum;

typedef enum
{
   CondCode_NEVER    = 0,
   CondCode_ALWAYS   = 1,
   CondCode_ZS       = 2,
   CondCode_ZC       = 3,
   CondCode_NS       = 4,
   CondCode_NC       = 5,
   CondCode_CS       = 6,
   CondCode_CC       = 7
} CondCode_Enum;

typedef enum
{
   Semaphore_INC = 0,
   Semaphore_DEC = 1
} Semaphore_Enum;

typedef enum
{
   Register_FILE_NONE = 0,
   Register_FILE_A    = 1,
   Register_FILE_B    = 2,
   Register_FILE_EITHER = Register_FILE_A | Register_FILE_B
} Register_File;

typedef enum
{
   Register_READ       = 1,
   Register_WRITE      = 2,
   Register_READ_WRITE = Register_READ | Register_WRITE
} Register_Mode;

typedef enum
{
   Register_RA0,  Register_RA1,  Register_RA2,  Register_RA3,  Register_RA4,  Register_RA5,  Register_RA6,  Register_RA7,
   Register_RA8,  Register_RA9,  Register_RA10, Register_RA11, Register_RA12, Register_RA13, Register_RA14, Register_RA15,
   Register_RA16, Register_RA17, Register_RA18, Register_RA19, Register_RA20, Register_RA21, Register_RA22, Register_RA23,
   Register_RA24, Register_RA25, Register_RA26, Register_RA27, Register_RA28, Register_RA29, Register_RA30, Register_RA31,
   Register_RB0,  Register_RB1,  Register_RB2,  Register_RB3,  Register_RB4,  Register_RB5,  Register_RB6,  Register_RB7,
   Register_RB8,  Register_RB9,  Register_RB10, Register_RB11, Register_RB12, Register_RB13, Register_RB14, Register_RB15,
   Register_RB16, Register_RB17, Register_RB18, Register_RB19, Register_RB20, Register_RB21, Register_RB22, Register_RB23,
   Register_RB24, Register_RB25, Register_RB26, Register_RB27, Register_RB28, Register_RB29, Register_RB30, Register_RB31,

   Register_ACC0, Register_ACC1, Register_ACC2, Register_ACC3, Register_ACC4, Register_ACC5,

   Register_SMALL_IMMED,

   Register_UNIFORM_READ   ,
   Register_VARYING_READ   ,
   Register_TMU_NOSWAP     ,
   Register_ELEMENT_NUMBER ,
   Register_QPU_NUMBER     ,
   Register_HOST_INT       ,
   Register_NOP            ,

   Register_UNIFORMS_ADDRESS ,
   Register_X_PIXEL_COORD    ,
   Register_Y_PIXEL_COORD    ,
   Register_QUAD_X           ,
   Register_QUAD_Y           ,
   Register_MS_FLAGS         ,
   Register_REV_FLAG         ,
   Register_TLB_STENCIL_SETUP,
   Register_TLB_Z            ,
   Register_TLB_COLOUR_MS    ,
   Register_TLB_COLOUR_ALL   ,
   Register_TLB_ALPHA_MASK   ,

   Register_VPM_READ        ,
   Register_VPM_WRITE       ,
   Register_VPM_LD_BUSY     ,
   Register_VPM_ST_BUSY     ,
   Register_VPMVCD_RD_SETUP ,
   Register_VPMVCD_WR_SETUP ,
   Register_VPM_LD_WAIT     ,
   Register_VPM_ST_WAIT     ,
   Register_VPM_LD_ADDR     ,
   Register_VPM_ST_ADDR     ,

   Register_MUTEX_ACQUIRE   ,
   Register_MUTEX_RELEASE   ,
   Register_SFU_RECIP       ,
   Register_SFU_RECIPSQRT   ,
   Register_SFU_EXP         ,
   Register_SFU_LOG         ,

   Register_TMU0_S          ,
   Register_TMU0_T          ,
   Register_TMU0_R          ,
   Register_TMU0_B          ,
   Register_TMU1_S          ,
   Register_TMU1_T          ,
   Register_TMU1_R          ,
   Register_TMU1_B          ,

   Register_VIRTUAL_FLAGS   ,
   Register_VIRTUAL_TMUWRITE,

   Register_UNKNOWN         ,
   Register_NUM_REGISTERS
} Register_Enum;

static bool Register_IsNormalReg(Register_Enum r)
{
   return r <= Register_RB31;
}

static bool Register_IsNormalRegOrAcc(Register_Enum r)
{
   return r <= Register_ACC5;
}

static bool Register_IsReadOnly(Register_Enum r)
{
   switch (r)
   {
   case Register_ACC4 :
   case Register_ACC5 :
   case Register_UNIFORM_READ :
   case Register_VARYING_READ :
   case Register_ELEMENT_NUMBER :
   case Register_QPU_NUMBER :
   case Register_X_PIXEL_COORD :
   case Register_Y_PIXEL_COORD :
   case Register_MS_FLAGS :
   case Register_REV_FLAG :
   case Register_VPM_READ :
   case Register_VPM_LD_BUSY :
   case Register_VPM_ST_BUSY :
   case Register_VPM_LD_WAIT :
   case Register_VPM_ST_WAIT :
   case Register_MUTEX_ACQUIRE :
                                 return true;
   default :                     return false;
   }

   return false;
}

static bool Register_IsAccumulator(Register_Enum r)
{
   return r >= Register_ACC0 && r <= Register_ACC5;
}

static uint8_t Register_GetCode(Register_Enum r)
{
   if (r <= Register_RA31)
      return r;

   if (r <= Register_RB31)
      return r - Register_RB0;

   if (r <= Register_ACC3)
      return 32 + r - Register_ACC0;

   switch (r)
   {
   case Register_SMALL_IMMED:
      return 0;      // Not actually a real reg

   case Register_UNIFORM_READ:
      return 32;

   case Register_VARYING_READ:
      return 35;

   case Register_TMU_NOSWAP:
      return 36;

   case Register_ELEMENT_NUMBER:
   case Register_QPU_NUMBER:
   case Register_HOST_INT:
      return 38;

   case Register_NOP:
   case Register_UNKNOWN:
   case Register_VIRTUAL_FLAGS:
      return 39;

   case Register_UNIFORMS_ADDRESS:
      return 40;

   case Register_X_PIXEL_COORD:
   case Register_Y_PIXEL_COORD:
   case Register_QUAD_X:
   case Register_QUAD_Y:
      return 41;

   case Register_MS_FLAGS:
   case Register_REV_FLAG:
      return 42;

   case Register_TLB_STENCIL_SETUP:
      return 43;

   case Register_TLB_Z:
      return 44;

   case Register_TLB_COLOUR_MS:
      return 45;

   case Register_TLB_COLOUR_ALL:
      return 46;

   case Register_TLB_ALPHA_MASK:
      return 47;

   case Register_VPM_READ:
   case Register_VPM_WRITE:
      return 48;

   case Register_VPM_LD_BUSY:
   case Register_VPM_ST_BUSY:
   case Register_VPMVCD_RD_SETUP:
   case Register_VPMVCD_WR_SETUP:
      return 49;

   case Register_VPM_LD_WAIT:
   case Register_VPM_ST_WAIT:
   case Register_VPM_LD_ADDR:
   case Register_VPM_ST_ADDR:
      return 50;

   case Register_MUTEX_ACQUIRE:
   case Register_MUTEX_RELEASE:
      return 51;

   case Register_SFU_RECIP:
      return 52;

   case Register_SFU_RECIPSQRT:
      return 53;

   case Register_SFU_EXP:
      return 54;

   case Register_SFU_LOG:
      return 55;

   case Register_TMU0_S:
      return 56;

   case Register_TMU0_T:
      return 57;

   case Register_TMU0_R:
      return 58;

   case Register_TMU0_B:
      return 59;

   case Register_TMU1_S:
      return 60;

   case Register_TMU1_T:
      return 61;

   case Register_TMU1_R:
      return 62;

   case Register_TMU1_B:
      return 63;

   default:
      UNREACHABLE();
      break;
   }

   return 0;
}

static Register_File Register_GetFile(Register_Enum r)
{
   if (r <= Register_RA31)
      return Register_FILE_A;

   if (r <= Register_RB31)
      return Register_FILE_B;

   if (r <= Register_ACC3)
      return Register_FILE_EITHER;

   switch (r)
   {
   case Register_UNKNOWN:
   default:
      return Register_FILE_NONE;

   case Register_UNIFORM_READ:
   case Register_VARYING_READ:
   case Register_TMU_NOSWAP:
   case Register_ACC5:
   case Register_HOST_INT:
   case Register_NOP:
   case Register_UNIFORMS_ADDRESS:
   case Register_TLB_STENCIL_SETUP:
   case Register_TLB_Z:
   case Register_TLB_COLOUR_MS:
   case Register_TLB_COLOUR_ALL:
   case Register_TLB_ALPHA_MASK:
   case Register_VPM_READ:
   case Register_VPM_WRITE:
   case Register_MUTEX_ACQUIRE:
   case Register_MUTEX_RELEASE:
   case Register_SFU_RECIP:
   case Register_SFU_RECIPSQRT:
   case Register_SFU_EXP:
   case Register_SFU_LOG:
   case Register_TMU0_S:
   case Register_TMU0_T:
   case Register_TMU0_R:
   case Register_TMU0_B:
   case Register_TMU1_S:
   case Register_TMU1_T:
   case Register_TMU1_R:
   case Register_TMU1_B:
   case Register_VIRTUAL_FLAGS:
      return Register_FILE_EITHER;

   case Register_ELEMENT_NUMBER:
   case Register_X_PIXEL_COORD:
   case Register_QUAD_X:
   case Register_MS_FLAGS:
   case Register_VPM_LD_BUSY:
   case Register_VPM_LD_WAIT:
   case Register_VPMVCD_RD_SETUP:
   case Register_VPM_LD_ADDR:
      return Register_FILE_A;

   case Register_QPU_NUMBER:
   case Register_Y_PIXEL_COORD:
   case Register_QUAD_Y:
   case Register_REV_FLAG:
   case Register_VPM_ST_BUSY:
   case Register_VPM_ST_WAIT:
   case Register_VPMVCD_WR_SETUP:
   case Register_VPM_ST_ADDR:
   case Register_SMALL_IMMED:
      return Register_FILE_B;
   }
}

static bool Register_IsRefCounted(Register_Enum r)
{
   if (r <= Register_ACC5)
      return true;

   switch (r)
   {
   case Register_VIRTUAL_FLAGS:
      return true;

   case Register_TMU_NOSWAP:
   case Register_HOST_INT:
   case Register_NOP:
   case Register_UNIFORMS_ADDRESS:
   case Register_TLB_STENCIL_SETUP:
   case Register_TLB_Z:
   case Register_TLB_COLOUR_MS:
   case Register_TLB_COLOUR_ALL:
   case Register_TLB_ALPHA_MASK:
   case Register_VPM_READ:
   case Register_VPM_WRITE:
   case Register_MUTEX_ACQUIRE:
   case Register_MUTEX_RELEASE:
   case Register_SFU_RECIP:
   case Register_SFU_RECIPSQRT:
   case Register_SFU_EXP:
   case Register_SFU_LOG:
   case Register_TMU0_S:
   case Register_TMU0_T:
   case Register_TMU0_R:
   case Register_TMU0_B:
   case Register_TMU1_S:
   case Register_TMU1_T:
   case Register_TMU1_R:
   case Register_TMU1_B:
   case Register_ELEMENT_NUMBER:
   case Register_X_PIXEL_COORD:
   case Register_QUAD_X:
   case Register_MS_FLAGS:
   case Register_VPM_LD_BUSY:
   case Register_VPM_LD_WAIT:
   case Register_VPMVCD_RD_SETUP:
   case Register_VPM_LD_ADDR:
   case Register_QPU_NUMBER:
   case Register_Y_PIXEL_COORD:
   case Register_QUAD_Y:
   case Register_REV_FLAG:
   case Register_VPM_ST_BUSY:
   case Register_VPM_ST_WAIT:
   case Register_VPMVCD_WR_SETUP:
   case Register_VPM_ST_ADDR:
   case Register_SMALL_IMMED:
   case Register_UNKNOWN:
   case Register_UNIFORM_READ:
   case Register_VARYING_READ:
   default:
      return false;
   }
}

static int32_t Register_ReadLatency(Register_Enum r)
{
   if (r <= Register_RA31)
      return 2;

   if (r <= Register_RB31)
      return 2;

   if (r == Register_VIRTUAL_TMUWRITE)
      return 3;

   return 1;
}

static bool Register_IsMutuallyExclusiveOnWrite(Register_Enum r)
{
   switch (r)
   {
   case Register_TLB_Z:
   case Register_TLB_COLOUR_MS:
   case Register_TLB_COLOUR_ALL:
   case Register_TLB_ALPHA_MASK:
   case Register_VPM_WRITE:
   case Register_VPMVCD_WR_SETUP:
   case Register_VPMVCD_RD_SETUP:
   case Register_MUTEX_RELEASE:
   case Register_SFU_RECIP:
   case Register_SFU_RECIPSQRT:
   case Register_SFU_EXP:
   case Register_SFU_LOG:
   case Register_TMU0_S:
   case Register_TMU0_T:
   case Register_TMU0_R:
   case Register_TMU0_B:
   case Register_TMU1_S:
   case Register_TMU1_T:
   case Register_TMU1_R:
   case Register_TMU1_B:
   case Register_TLB_STENCIL_SETUP:
      return true;

   default:
      return false;
   }
}

static bool Register_IsSelfExclusiveRead(Register_Enum r)
{
   switch (r)
   {
   case Register_UNIFORM_READ:
   case Register_VARYING_READ:
   case Register_VPM_READ:
   case Register_MUTEX_ACQUIRE:
      return true;

   default:
      return false;
   }
}

static Register_Mode Register_GetMode(Register_Enum r)
{
   if (r <= Register_RB31)
      return Register_READ_WRITE;

   if (r <= Register_ACC4)
      return Register_WRITE;

   switch (r)
   {
   case Register_NOP:
   case Register_MS_FLAGS:
   case Register_REV_FLAG:
   case Register_UNKNOWN:
      return Register_READ_WRITE;

   case Register_UNIFORM_READ:
   case Register_VARYING_READ:
   case Register_ELEMENT_NUMBER:
   case Register_QPU_NUMBER:
   case Register_X_PIXEL_COORD:
   case Register_Y_PIXEL_COORD:
   case Register_VPM_READ:
   case Register_VPM_LD_BUSY:
   case Register_VPM_LD_WAIT:
   case Register_VPM_ST_BUSY:
   case Register_VPM_ST_WAIT:
   case Register_SMALL_IMMED:
      return Register_READ;

   case Register_TMU_NOSWAP:
   case Register_ACC5:
   case Register_HOST_INT:
   case Register_UNIFORMS_ADDRESS:
   case Register_TLB_STENCIL_SETUP:
   case Register_TLB_Z:
   case Register_TLB_COLOUR_MS:
   case Register_TLB_COLOUR_ALL:
   case Register_VPM_WRITE:
   case Register_SFU_RECIP:
   case Register_SFU_RECIPSQRT:
   case Register_SFU_EXP:
   case Register_SFU_LOG:
   case Register_TMU0_S:
   case Register_TMU0_T:
   case Register_TMU0_R:
   case Register_TMU0_B:
   case Register_TMU1_S:
   case Register_TMU1_T:
   case Register_TMU1_R:
   case Register_TMU1_B:
   case Register_VPMVCD_RD_SETUP:
   case Register_VPM_LD_ADDR:
   case Register_VPMVCD_WR_SETUP:
   case Register_VPM_ST_ADDR:
   case Register_VIRTUAL_FLAGS:
      return Register_WRITE;

   default:
      UNREACHABLE();
      break;
   }
}

static InputMux_Enum Register_GetInputMux(Register_Enum r)
{
   if (r >= Register_RA0 && r <= Register_RA31)
      return InputMux_REGA;
   else if (r >= Register_RB0 && r <= Register_RB31)
      return InputMux_REGB;
   else if (r >= Register_ACC0 && r <= Register_ACC5)
      return (InputMux_Enum)(InputMux_ACC0 + (r - Register_ACC0));
   else
   {
      Register_File file = Register_GetFile(r);
      if (file == Register_FILE_EITHER || file == Register_FILE_NONE)
         return InputMux_REGA_OR_B;
      else if (file == Register_FILE_A)
         return InputMux_REGA;
      else if (file == Register_FILE_B)
         return InputMux_REGB;

      return InputMux_REGA_OR_B;
   }
}

static bool Register_IsTMUSetupWrite(Register_Enum r)
{
   switch (r)
   {
   case Register_TMU0_S:
   case Register_TMU0_T:
   case Register_TMU0_R:
   case Register_TMU0_B:
   case Register_TMU1_S:
   case Register_TMU1_T:
   case Register_TMU1_R:
   case Register_TMU1_B:
      return true;

   default:
      return false;
   }
}

typedef enum
{
   Sig_SWBRK    = 0,
   Sig_NONE     = 1,
   Sig_THRSW    = 2,
   Sig_END      = 3,
   Sig_SBLOCK   = 4,
   Sig_SBUNLOCK = 5,
   Sig_LTHRSW   = 6,
   Sig_COVLOAD  = 7,
   Sig_COLLOAD  = 8,
   Sig_COLLOAD_PRGEND = 9,
   Sig_LDTMU0   = 10,
   Sig_LDTMU1   = 11,
   Sig_LDALPHA  = 12
 } Sig_Enum;

static bool Sig_ClashesWith(Sig_Enum sig, Register_Enum r)
{
   if (r == Register_VPM_WRITE) // Exclude this from the IsMutuallyExclusiveOnWrite test
      return false;

   if (Register_IsMutuallyExclusiveOnWrite(r))
   {
      switch (sig)
      {
      case Sig_COVLOAD :
      case Sig_COLLOAD :
      case Sig_COLLOAD_PRGEND :
      case Sig_LDTMU0 :
      case Sig_LDTMU1 :
      case Sig_LDALPHA :
         return true;

      default:
         return false;
      }
   }
   return false;
}

#endif /* __GLSL_QPU_ENUM_H__ */
