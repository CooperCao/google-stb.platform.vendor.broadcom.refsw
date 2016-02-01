/***************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bwfe.h"
#include "bwfe_priv.h"

BDBG_MODULE(bwfe_priv);


/******************************************************************************
 BWFE_P_ReadModifyWriteRegister()
******************************************************************************/
void BWFE_P_ReadModifyWriteRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t and_mask, uint32_t or_mask)
{
   uint32_t val;

   BWFE_P_ReadRegister(h, reg, &val);
   val = (val & and_mask) | or_mask;
   BWFE_P_WriteRegister(h, reg, val);
}


/******************************************************************************
 BWFE_P_OrRegister()
******************************************************************************/
void BWFE_P_OrRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t or_mask)
{
   uint32_t val;

   BWFE_P_ReadRegister(h, reg, &val);
   val |= or_mask;
   BWFE_P_WriteRegister(h, reg, val);
}


/******************************************************************************
 BWFE_P_AndRegister()
******************************************************************************/
void BWFE_P_AndRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t and_mask)
{
   uint32_t val;

   BWFE_P_ReadRegister(h, reg, &val);
   val &= and_mask;
   BWFE_P_WriteRegister(h, reg, val);
}


/******************************************************************************
 BWFE_P_ToggleBit()
******************************************************************************/
void BWFE_P_ToggleBit(BWFE_ChannelHandle h, uint32_t reg, uint32_t mask)
{
   uint32_t val;
   
   BWFE_P_ReadRegister(h, reg, &val);
      
   val |= mask;
   BWFE_P_WriteRegister(h, reg, val);
      
   val &= ~mask;
   BWFE_P_WriteRegister(h, reg, val);  
}


/******************************************************************************
 BWFE_P_ReadModifyWriteSliceReg()
******************************************************************************/
void BWFE_P_ReadModifyWriteSliceReg(BWFE_ChannelHandle h, uint8_t slice, uint32_t reg, uint32_t and_mask, uint32_t or_mask)
{
   uint32_t val;

   BWFE_P_ReadSliceRegister(h, slice, reg, &val);
   val = (val & and_mask) | or_mask;
   BWFE_P_WriteSliceRegister(h, slice, reg, val);
}


/******************************************************************************
 BWFE_P_OrSliceReg()
******************************************************************************/
void BWFE_P_OrSliceReg(BWFE_ChannelHandle h, uint8_t slice, uint32_t reg, uint32_t or_mask)
{
   uint32_t val;

   BWFE_P_ReadSliceRegister(h, slice, reg, &val);
   val |= or_mask;
   BWFE_P_WriteSliceRegister(h, slice, reg, val);
}


/******************************************************************************
 BWFE_P_AndSliceReg()
******************************************************************************/
void BWFE_P_AndSliceReg(BWFE_ChannelHandle h, uint8_t slice, uint32_t reg, uint32_t and_mask)
{
   uint32_t val;

   BWFE_P_ReadSliceRegister(h, slice, reg, &val);
   val &= and_mask;
   BWFE_P_WriteSliceRegister(h, slice, reg, val);
}


/******************************************************************************
 BWFE_P_ToggleSliceBit()
******************************************************************************/
void BWFE_P_ToggleSliceBit(BWFE_ChannelHandle h, uint8_t slice, uint32_t reg, uint32_t mask)
{
   uint32_t val;
   
   BWFE_P_ReadSliceRegister(h, slice, reg, &val);
      
   val |= mask;
   BWFE_P_WriteSliceRegister(h, slice, reg, val);
      
   val &= ~mask;
   BWFE_P_WriteSliceRegister(h, slice, reg, val);  
}


#ifdef BWFE_FLOAT_MODE
/* floating point arctan(2^-i) table (normalized by PI): */
static const double BWFE_ATAN_FP[BWFE_CORDIC_NUM_ITER_FP] = {
   0.25000000000000,   0.14758361765043,   0.07797913037737,   0.03958342416057,
   0.01986852430554,   0.00994394782359,   0.00497318727895,   0.00248674539367,
   0.00124339166871,   0.00062169820592,   0.00031084939941,   0.00015542473676,
   0.00007771237301,   0.00003885618709,   0.00001942809362,   0.00000971404682,
   0.00000485702341,   0.00000242851170,   0.00000121425585,   0.00000060712793,
   0.00000030356396,   0.00000015178198,   0.00000007589099,   0.00000003794550,
   0.00000001897275,   0.00000000948637,   0.00000000474319,   0.00000000237159,
   0.00000000118580,   0.00000000059290,   0.00000000029645,   0.00000000014822,
   0.00000000007411,   0.00000000003706,   0.00000000001853,   0.00000000000926,
   0.00000000000463,   0.00000000000232,   0.00000000000116,   0.00000000000058
};

/******************************************************************************
 BWFE_P_CordicRotationMode() - approximates sin(theta) and cos(theta)
   where theta = phase normalized by PI
 
 input: vec[0] = I, vec[1] = Q, theta, unitMode(vec=1+j0 if TRUE)
 output: vec[0] = I*cos(theta)-Q*sin(theta)
              vec[1] = I*sin(theta)+Q*cos(theta)
******************************************************************************/
BERR_Code BWFE_P_CordicRotationMode_FP(double *vec, double theta, bool unitMode)
{
   BERR_Code retCode = BERR_SUCCESS;
   double x_i, y_i, two_pow_neg_i;
   uint8_t i;
   
   if (unitMode)
   {
      vec[0] = 1.0;
   	vec[1] = 0.0;
   }
   
   /* case <-PI/2 or >PI/2, rotate by +/-PI back to 1st/4th quadrant */
   if (theta < -0.5)    /* case theta < -PI/2   */
   {
      vec[0] = -vec[0]; /* I = -I */
      vec[1] = -vec[1]; /* Q = -Q */
      theta += 1.0;     /* theta += PI */
   }
   else if (theta > 0.5)   /* case theta > PI/2  */
   {
      vec[0] = -vec[0]; /* I = -I */
      vec[1] = -vec[1]; /* Q = -Q */
      theta -= 1.0;     /* theta -= PI */
   }
   
   two_pow_neg_i = 1.0; /* 2^0 initially */
   for (i = 0; i < BWFE_CORDIC_NUM_ITER_FP; i++)
   {
      x_i = vec[0];
      y_i = vec[1];
      
      if (theta >= 0)
      {
         /* rotate clockwise */
         vec[0] -= y_i * two_pow_neg_i;
         vec[1] += x_i * two_pow_neg_i;
         theta -= BWFE_ATAN_FP[i];
      }
      else
      {
         /* rotate counter-clockwise */
         vec[0] += y_i * two_pow_neg_i;
         vec[1] -= x_i * two_pow_neg_i;
         theta += BWFE_ATAN_FP[i];
      }
      two_pow_neg_i /= 2;  /* div by 2 */
   
   #if 0
      printf("%d:[", i);
      printDouble(vec[0], 8);
      printf(",");
      printDouble(vec[1], 8);
      printf("] theta=");
      printDouble(theta, 8);
      printf(" 2^-i=");
      printDouble(two_pow_neg_i, 8);
      printf("\n");
   #endif      
   }
   
   /* apply scaling */
   vec[0] *= BWFE_CORDIC_K_FP;
   vec[1] *= BWFE_CORDIC_K_FP;
   
   return retCode;
}


/******************************************************************************
 BWFE_P_CordicVectorMode() - converts rect coordinates to polar
 
 input: vec[0]=I, vec[1]=Q
 output: vec[0] = sqrt(I^2+Q^2) magnitude
              vec[1] ~= 0 (at or near zero)
              theta=ATAN^2(Q,I)/PI normalized phase
******************************************************************************/
BERR_Code BWFE_P_CordicVectorMode_FP(double *vec, double *theta)
{
   BERR_Code retCode = BERR_SUCCESS;
   double x_i, y_i, two_pow_neg_i;
   uint8_t i;
  
   if (vec[0] < 0)
   {
      if (vec[1] < 0)
         *theta = -1.0; /*  init phase to -PI normalized */
      else
         *theta = 1.0;  /*  init phase to +PI normalized */
      
      /* rotate by +/-PI to make x positive */
      vec[0] = -vec[0]; /* I = -I  */
      vec[1] = -vec[1]; /* Q = -Q  */
   }
   else
   {
      *theta = 0.0;  /* initial phase is 0 */
   }
  
   two_pow_neg_i = 1.0; /* 2^0 initially */
   for (i = 0; i < BWFE_CORDIC_NUM_ITER_FP; i++)
   {
      x_i = vec[0];
      y_i = vec[1];
      
      if (vec[1] < 0)
      {
         /* rotate clockwise */
         vec[0] -= y_i * two_pow_neg_i;
         vec[1] += x_i * two_pow_neg_i;
         *theta -= BWFE_ATAN_FP[i];
      }
      else
      {
         /* rotate counter-clockwise */
         vec[0] += y_i * two_pow_neg_i;
         vec[1] -= x_i * two_pow_neg_i;
         *theta += BWFE_ATAN_FP[i];
      }
      two_pow_neg_i /= 2;   /* div by 2 */
      
   #if 0
      printf("%d:[", i);
      printDouble(vec[0], 8);
      printf(",");
      printDouble(vec[1], 8);
      printf("] theta=");
      printDouble(*theta, 8);
      printf(" 2^-i=");
      printDouble(two_pow_neg_i, 8);
      printf("\n");
   #endif
   }
   
   /* scale magnitude */
   vec[0] *= BWFE_CORDIC_K_FP;
   
   return retCode;
}

#endif

/* fixed point arctan(2^-i) table normalized by 2*PI  (i=0 --> PI/4): */
static const int32_t BWFE_ATAN[BWFE_CORDIC_NUM_ITER] = {
   0x20000000, 0x12E4051D, 0x09FB385B, 0x051111D4, 0x028B0D43, 0x0145D7E1, 0x00A2F61E, 
   0x00517C55, 0x0028BE53, 0x00145F2E, 0x000A2F98, 0x000517CC, 0x00028BE6, 0x000145F3, 
   0x0000A2F9, 0x0000517C, 0x000028BE, 0x0000145F, 0x00000A2F, 0x00000517, 0x0000028B, 
   0x00000145, 0x000000A2, 0x00000051, 0x00000028, 0x00000014, 0x0000000A, 0x00000005
};

/******************************************************************************
 BWFE_P_CordicRotationMode() - approximates sin(theta) and cos(theta)
   where theta = 32-bit phase normalized by 2*PI
 
 input: vec[0] = I, vec[1] = Q, theta, unitMode(vec=1+j0 if TRUE)
 output: vec[0] = I*cos(theta)-Q*sin(theta)
              vec[1] = I*sin(theta)+Q*cos(theta)
 
   NOTE theta may be interpreted as signed or unsigned
   uint32_t: [0, 2*PI) / 2*PI * 2^32 = 0x00000000 to 0xFFFFFFFF
   int32_t: [-PI, PI) / 2*PI * 2^32 = 0x80000000 to 0x7FFFFFFF
******************************************************************************/
BERR_Code BWFE_P_CordicRotationMode(int32_t *vec, int32_t theta, bool unitMode)
{
   BERR_Code retCode = BERR_SUCCESS;
   int32_t x_i, y_i;
   uint32_t two_pow_i;
   /*uint32_t P_hi, P_lo, Q_hi, Q_lo;*/
   uint8_t i;
   /*bool bNegative = false;*/
   
   if (unitMode)
   {
      vec[0] = 1 * BWFE_CORDIC_K; /* pre-scale initial vector */
      vec[1] = 0;
   }
   else
   {
      /* TBD fixed point rotation works for unit mode only! */
      /* TBD need to implement 64-bit arithmetic for non-unit mode! */
      return BERR_NOT_SUPPORTED;
   #if 0
      if (vec[0] < 0)
      {
         /* must remove sign from vec since extended precision is unsigned */
         bNegative = true;
         vec[0] = -vec[0];
      }
      
      /* pre-scale I */
      BMTH_HILO_32TO64_Mul((uint32_t)vec[0], (uint32_t)BWFE_CORDIC_K, &P_hi, &P_lo);  /* I x BWFE_CORDIC_K */
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, BWFE_CORDIC_SCALE, &Q_hi, &Q_lo);  /* TBD undo cordic scale >> 30 */
      vec[0] = Q_lo;
      
      if (bNegative)
         vec[0] = -vec[0];
      
      if (vec[1] < 0)
      {
         /* must remove sign from vec since extended precision is unsigned */
         bNegative = true;
         vec[1] = -vec[1];
      }
      else
         bNegative = false;
      
      /* pre-scale jQ */
      BMTH_HILO_32TO64_Mul((uint32_t)vec[1], (uint32_t)BWFE_CORDIC_K, &P_hi, &P_lo);  /* Q x BWFE_CORDIC_K */
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, BWFE_CORDIC_SCALE, &Q_hi, &Q_lo);  /* TBD undo cordic scale >> 30 */
      vec[1] = Q_lo;
      
      if (bNegative)
         vec[1] = -vec[1];
   #endif
   }
   
   /* case <-PI/2 or >PI/2, rotate by +/-PI back to 1st/4th quadrant */
   if ((theta < (int32_t)0xC0000000) || (theta > 0x40000000))
   {
      vec[0] = -vec[0];    /* I = -I */
      vec[1] = -vec[1];    /* Q = -Q */
      theta += 0x80000000; /* theta += PI */
   }
   
   two_pow_i = 1; /* 2^0 initially */
   for (i = 0; i < BWFE_CORDIC_NUM_ITER; i++)
   {
      x_i = vec[0];
      y_i = vec[1];
      
      if (theta >= 0)
      {
         /* rotate clockwise */
         vec[0] -= y_i / (int32_t)two_pow_i;
         vec[1] += x_i / (int32_t)two_pow_i;
         theta -= BWFE_ATAN[i];
      }
      else
      {
         /* rotate counter-clockwise */
         vec[0] += y_i / (int32_t)two_pow_i;
         vec[1] -= x_i / (int32_t)two_pow_i;
         theta += BWFE_ATAN[i];
      }
      two_pow_i <<= 1;  /* multiply by 2 */
      /* BDBG_MSG(("%d:[%08X, %08X] theta=%08X, 2^i=%08X", i, vec[0], vec[1], *theta, two_pow_i)); */
   }
   
   return retCode;
}


/******************************************************************************
 BWFE_P_CordicVectorMode() - converts rect coordinates to polar
 
 input: vec[0]=I, vec[1]=Q
 output: vec[0] = sqrt(I^2+Q^2) magnitude
              vec[1] ~= 0 (at or near zero)
              theta=ATAN^2(Q,I)/(2*PI) 32-bit normalized angle
 
   NOTE theta may be interpreted as signed or unsigned
   uint32_t: [0, 2*PI) / 2*PI * 2^32 = 0x00000000 to 0xFFFFFFFF
   int32_t: [-PI, PI) / 2*PI * 2^32 = 0x80000000 to 0x7FFFFFFF
******************************************************************************/
BERR_Code BWFE_P_CordicVectorMode(int32_t *vec, int32_t *theta)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t x_hi, x_lo, y_hi, y_lo;
   uint32_t x_i_hi, x_i_lo, y_i_hi, y_i_lo;
   uint8_t i, two_shift_i;
   
   bool bNegativeI = false;
   bool bNegativeQ = false;
   
   if (vec[0] < 0)
   {
      /* rotate by PI to make x positive */
      vec[0] = -vec[0];    /* I = -I  */
      vec[1] = -vec[1];    /* Q = -Q  */
      *theta = 0x80000000; /*  init phase to +PI normalized */
   }
   else
   {
      *theta = 0;  /* initial phase is 0 */
   }
   
   /*printf("initial:[%08X, %08X] theta=%08X\n", vec[0], vec[1], *theta);*/
   
   /* pre-scale I, positive already */
   BMTH_HILO_32TO64_Mul((uint32_t)vec[0], BWFE_CORDIC_K, &x_hi, &x_lo);  /* I x BWFE_CORDIC_K */
   
   if (vec[1] < 0)
   {
      /* remove sign from Q since extended precision is unsigned */
      bNegativeQ = true;
      vec[1] = -vec[1];
   }
   
   /* pre-scale jQ */
   BMTH_HILO_32TO64_Mul((uint32_t)vec[1], BWFE_CORDIC_K, &y_hi, &y_lo);  /* Q x BWFE_CORDIC_K */
   
   /* restore sign by coverting back to two's comp */
   if (bNegativeQ)
      BMTH_HILO_64TO64_Neg(y_hi, y_lo, &y_hi, &y_lo);
  
   two_shift_i = 0;  /* 2^0 initially */
   for (i = 0; i < BWFE_CORDIC_NUM_ITER; i++)
   {
      bNegativeI = false;
      bNegativeQ = false;
      
      /* convert x to positive unsigned before shift operation */
      if ((int32_t)x_hi < 0)
      {
         bNegativeI = true;
         BMTH_HILO_64TO64_Neg(x_hi, x_lo, &x_hi, &x_lo);
      }
      
      /* convert y to positive unsigned before shift operation */
      if ((int32_t)y_hi < 0)
      {
         bNegativeQ = true;
         BMTH_HILO_64TO64_Neg(y_hi, y_lo, &y_hi, &y_lo);
      }
      
      /* shift operation valid for unsigned only */
      x_i_hi = x_hi >> two_shift_i;
      x_i_lo = (x_hi << (32 - two_shift_i)) | (x_lo >> two_shift_i);
      y_i_hi = y_hi >> two_shift_i;
      y_i_lo = (y_hi << (32 - two_shift_i)) | (y_lo >> two_shift_i);
      /*printf("   x_i=%08X%08X, y_i=%08X%08X\n", x_i_hi, x_i_lo, y_i_hi, y_i_lo);*/
      
      /* restore sign by converting x back to two's comp */
      if (bNegativeI)
         BMTH_HILO_64TO64_Neg(x_hi, x_lo, &x_hi, &x_lo);
      
      /* restore sign by converting y back to two's comp */
      if (bNegativeQ)
         BMTH_HILO_64TO64_Neg(y_hi, y_lo, &y_hi, &y_lo);
      
      if ((int32_t)y_hi < 0)
      {
         /* rotate clockwise */
         /* vec[0] -= y_i / (int32_t)two_pow_i; */
         /* vec[1] += x_i / (int32_t)two_pow_i; */
         *theta -= BWFE_ATAN[i];
         
         /* subtract y_i if postive y */
         if (!bNegativeQ)
           BMTH_HILO_64TO64_Neg(y_i_hi, y_i_lo, &y_i_hi, &y_i_lo);
         
         /* subtract x_i if negative x */
         if (bNegativeI)
            BMTH_HILO_64TO64_Neg(x_i_hi, x_i_lo, &x_i_hi, &x_i_lo);
      }
      else
      {
         /* rotate counter-clockwise */
         /* vec[0] += y_i / (int32_t)two_pow_i; */
         /* vec[1] -= x_i / (int32_t)two_pow_i; */
         *theta += BWFE_ATAN[i];
         
         /* subtract y_i if negative y */
         if (bNegativeQ)
            BMTH_HILO_64TO64_Neg(y_i_hi, y_i_lo, &y_i_hi, &y_i_lo);
         
         /* subtract x_i if positive x */
         if (!bNegativeI)
            BMTH_HILO_64TO64_Neg(x_i_hi, x_i_lo, &x_i_hi, &x_i_lo);
      }
      BMTH_HILO_64TO64_Add(x_hi, x_lo, y_i_hi, y_i_lo, &x_hi, &x_lo);
      BMTH_HILO_64TO64_Add(y_hi, y_lo, x_i_hi, x_i_lo, &y_hi, &y_lo);
      
      /* BDBG_MSG(("%d:[%08X%08X, %08X%08X] theta=%08X, shift=%d\n", i, x_hi, x_lo, y_hi, y_lo, *theta, two_shift_i)); */
      two_shift_i++;
   }
   
   if ((int32_t)x_hi < 0)
   {
      /* convert x to positive unsigned before shift operation */
      bNegativeI = true;
      BMTH_HILO_64TO64_Neg(x_hi, x_lo, &x_hi, &x_lo);
   }
   
   /* round for accuracy */
   BMTH_HILO_64TO64_Add(x_hi, x_lo, 0, BWFE_CORDIC_SCALE >> 1, &x_hi, &x_lo);             /* add LSB/2 */
   vec[0] = (int32_t)((x_hi << (32 - BWFE_CORDIC_SHIFT)) | (x_lo >> BWFE_CORDIC_SHIFT));  /* undo cordic scale for magnitude */
   
   return retCode;
}


/******************************************************************************
 BWFE_P_GCF() - returns greatest common factor
******************************************************************************/
uint32_t BWFE_P_GCF(uint32_t m, uint32_t n)
{
   uint32_t L1, L2, L3, L4;

   L1 = m;
   L2 = n;

   while (L2 > 0)
   {
      L3 = L1 / L2;
      L4 = L1 - (L2 * L3);
      L1 = L2;
      L2 = L4;
   }
   return L1;
}


/******************************************************************************
 BWFE_P_GetMsb() - returns position of most significant non-zero bit
******************************************************************************/
uint8_t BWFE_P_GetMsb(uint32_t i)
{
   uint8_t pos = 0;
   
   while ((i >>= 1) > 0) pos++;
   return pos;
}


/******************************************************************************
 BWFE_P_SumThenMultiply() - returns (x + y) * m
******************************************************************************/
void BWFE_P_SumThenMultiply(int32_t x, int32_t y, int32_t m, uint32_t *result_hi, uint32_t *result_lo)
{
   int32_t sum = x + y;
   bool bNegSum = false;
   bool bNegMul = false;
   
   if (sum < 0)
   {
      /* convert sum to unsigned for extended arithmetic */
      bNegSum = true;
      sum = -sum;
   }
   
   if (m < 0)
   {
      /* convert multiplier to unsigned for extended arithmetic */
      bNegMul = true;
      m = -m;
   }
   
   /* calculates (x + y) x m in 64-bits */
   BMTH_HILO_32TO64_Mul(sum, m, result_hi, result_lo);
   
   if (bNegSum != bNegMul)
   {
      /* take two's comp of result if negative */
      *result_lo = ~(*result_lo) + 1;
      *result_hi = ~(*result_hi) + ((*result_lo == 0) ? 1 : 0);
   }
   return;
}

