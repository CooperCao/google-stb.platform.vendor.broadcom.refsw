/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h" 
#include "bwfe.h"
#include "bwfe_priv.h"
#include "bwfe_g3_priv.h"

BDBG_MODULE(bwfe_g3_priv_lic);


/******************************************************************************
 BWFE_g3_Lic_P_PowerUp()
******************************************************************************/
BERR_Code BWFE_g3_Lic_P_PowerUp(BWFE_ChannelHandle h)
{
#ifndef BWFE_BYPASS_LIC2
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
#endif
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t slice;
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   BDBG_MSG(("LIC%d pwrup", h->channel));
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_CLK_CTRL, 0x00000006);    /* enable lic/lic2 clock */
#ifndef BWFE_EXCLUDE_LIC
   BWFE_P_ToggleBit(h, BCHP_WFE_CORE_LICCTL1, 0x00000003);     /* reset LIC taps */
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_LICCTL1, 0x00000010);    /* enable LIC output */
#endif

#ifdef BWFE_BYPASS_LIC2
   /* bypass lic2 */
   BDBG_MSG(("bypass NR%d", h->channel));
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_NRNOTCHCTL, ~0x00000300, 0x00000200);  /* select dco inputs */
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_NRDCOCTL_THR, 0x00000006);       /* freeze and bypass NR DCO for AGC THR */
   
   for (slice = 0; slice < h->totalSlices; slice++)
   {
      BWFE_P_OrSliceReg(h, slice, BCHP_WFE_CORE_NRDCOCTL_PI_SLC, 0x00000006);    /* freeze and bypass NR DCO for ping */
      BWFE_P_OrSliceReg(h, slice, BCHP_WFE_CORE_NRDCOCTL_PO_SLC, 0x00000006);    /* freeze and bypass NR DCO for pong */
      
      BWFE_P_OrSliceReg(h, slice, BCHP_WFE_CORE_NRNOTCHCTL_PI_SLC, 0x00000006);  /* freeze and bypass NR NOTCH for ping */
      BWFE_P_OrSliceReg(h, slice, BCHP_WFE_CORE_NRNOTCHCTL_PO_SLC, 0x00000006);  /* freeze and bypass NR NOTCH for pong */
      
      BWFE_P_OrSliceReg(h, slice, BCHP_WFE_CORE_NRAGCCTL_PI_SLC, 0x00100006);    /* freeze and bypass NR AGC for ping */
      BWFE_P_OrSliceReg(h, slice, BCHP_WFE_CORE_NRAGCCTL_PO_SLC, 0x00100006);    /* freeze and bypass NR AGC for pong */
   }
#else
   /* initialize lic2 */
   #ifdef BWFE_HYBRID_ADC
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_NRNOTCHCTL, ~0x00000300, 0x00000300);  /* select 8 lane dco */
   #else
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_NRNOTCHCTL, ~0x00000300, 0x00000100);  /* select dco outputs */
   #endif
   BWFE_P_WriteRegister(h, BCHP_WFE_CORE_NRDCOCTL_THR, 0x00000021);     /* reset NR DCO for AGC THR: beta_dco=2, unfreeze, unbypass */
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_NRDCOCTL_THR, ~0x00000001);      /* release reset */
   
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_NRAGCTHR, 0x00100000);            /* use agc threshold offset */ 
   
   for (slice = 0; slice < h->totalSlices; slice++)
   {
      BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_NRDCOCTL_PI_SLC, 0x00000031);  /* reset NR DCO for ping: acq beta_dco=3, unfreeze, unbypass */
      BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_NRDCOCTL_PI_SLC, ~0x00000001);        /* release reset */
      BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_NRDCOCTL_PO_SLC, 0x00000031);  /* reset NR DCO for pong: acq beta_dco=3, unfreeze, unbypass */
      BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_NRDCOCTL_PO_SLC, ~0x00000001);        /* release reset */
      
      BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_NRNOTCHCTL_PI_SLC, 0x000000A1);   /* reset NR NOTCH for ping: beta_dco=10, unfreeze, unbypass */
      BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_NRNOTCHCTL_PI_SLC, ~0x00000001);         /* release reset */   
      BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_NRNOTCHCTL_PO_SLC, 0x000000A1);   /* reset NR NOTCH for pong: beta_dco=10, unfreeze, unbypass */
      BWFE_P_AndSliceReg(h, slice, BCHP_WFE_CORE_NRNOTCHCTL_PO_SLC, ~0x00000001);         /* release reset */
      
      BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_NRAGCCTL_PI_SLC, 0x00005000 | ((hChn->nrAgcK1 & 0xF) << 4)); /* ping: set acq agc_k1, unfreeze, unbypass */
      BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_NRAGCCTL_PO_SLC, 0x00005000 | ((hChn->nrAgcK1 & 0xF) << 4)); /* pong: set acq agc_k1, unfreeze, unbypass */
      
      BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_REG_NR_AGC_LF_INT_WDATA_PI_SLC, 0x90000000);  /* reset agc integrators */
      BWFE_P_WriteSliceRegister(h, slice, BCHP_WFE_CORE_REG_NR_AGC_LF_INT_WDATA_PO_SLC, 0x90000000);  /* reset agc integrators */
   }

   BKNI_EnterCriticalSection();
   retCode = BWFE_g3_P_EnableTimer_isr(h, BWFE_g3_TimerSelect_e0, 50000, BWFE_g3_Lic_P_NrDcoTracking);
   BKNI_LeaveCriticalSection();
#endif

#ifndef BWFE_EXCLUDE_LIC
   /* reset lic taps */
   BWFE_g3_Lic_P_ResetTaps(h);

#ifdef BWFE_BYPASS_LIC
   /* bypass lic since not used */
   BDBG_MSG(("bypass LIC%d", h->channel));
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_LICCTL1, 0x00000008);    /* bypass lic */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_CLK_CTRL, ~0x00000004);  /* disable lic clock */
#endif
#endif

#ifdef BWFE_BYPASS_LIC2
   /* signal ready event if lic2 bypassed */
   BKNI_SetEvent(hChn->hWfeReady);
   hChn->bLicReady = true;
#endif
   return retCode;
}


/******************************************************************************
 BWFE_g3_Lic_P_NrDcoTracking()
******************************************************************************/
BERR_Code BWFE_g3_Lic_P_NrDcoTracking(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   uint8_t slice;
   
   for (slice = 0; slice < h->totalSlices; slice++)
   {
      BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_NRDCOCTL_PI_SLC, ~0x000000F0, 0x000000C0);   /* tracking ping: beta_dco=12 */
      BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_NRDCOCTL_PO_SLC, ~0x000000F0, 0x000000C0);   /* tracking pong: beta_dco=12 */
   }

   /* signal ready event */
   BKNI_SetEvent(hChn->hWfeReady);
   hChn->bLicReady = true;

   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_Lic_P_PowerDown()
******************************************************************************/
BERR_Code BWFE_g3_Lic_P_PowerDown(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   BDBG_MSG(("LIC%d pwrdown", h->channel));
#ifndef BWFE_EXCLUDE_LIC
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_LICCTL1, ~0x00000030);  /* disable LIC output */
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_LICCTL2, ~0x00000003);  /* deselect lanes */
#endif
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_CLK_CTRL, ~0x00000006);  /* disable lic/lic2 clock */
   
   return retCode;
}


#ifndef BWFE_EXCLUDE_LIC
/******************************************************************************
 BWFE_g3_Lic_P_ResetTaps()
******************************************************************************/
BERR_Code BWFE_g3_Lic_P_ResetTaps(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   /* reset LIC and read default taps */
   BWFE_P_ToggleBit(h, BCHP_WFE_CORE_LICCTL1, 0x00000003);
#ifndef BWFE_EXCLUDE_LIC_TAP_COMPUTER
   retCode = BWFE_g3_Lic_P_ReadTaps(h);
#endif
   
   return retCode;
}
#endif


#ifndef BWFE_EXCLUDE_LIC_TAP_COMPUTER
/******************************************************************************
 BWFE_g3_Lic_P_ReadTaps()
******************************************************************************/
BERR_Code BWFE_g3_Lic_P_ReadTaps(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t lane, tap;
   uint32_t val;
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   for (lane = 0; lane < BWFE_NUM_LANES; lane++)
   {
      /* lane selection */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_LICCTL2, ~0x0000000F, 1 << lane);   /* TBD slice context? */
      
      /* read lic taps */
      BWFE_P_WriteRegister(h, BCHP_WFE_CORE_LICCOEFA, 0);   /* auto-incrementing address */
      for (tap = 0; tap < BWFE_NUM_LIC_TAPS; tap++)
      {
         BWFE_P_ReadRegister(h, BCHP_WFE_CORE_LICCOEFD, &val);
         hChn->licCoeff[lane][tap] = val;
      }
   }
   
   /* deselect lanes */
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_LICCTL2, ~0x0000000F);
   
   return retCode;
}


/******************************************************************************
 BWFE_g3_Lic_P_WriteTaps()
******************************************************************************/
BERR_Code BWFE_g3_Lic_P_WriteTaps(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t lane, tap;
   uint32_t val;
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   for (lane = 0; lane < BWFE_NUM_LANES; lane++)
   {
      /* lane selection */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_LICCTL2, ~0x00000003, 1 << lane);
      
      /* write lic taps */
      BWFE_P_WriteRegister(h, BCHP_WFE_CORE_LICCOEFA, 0);   /* auto-incrementing address */
      for (tap = 0; tap < BWFE_NUM_LIC_TAPS; tap++)
      {
         val = hChn->licCoeff[lane][tap];
         BWFE_P_WriteRegister(h, BCHP_WFE_CORE_LICCOEFD, val);
      }
      
      /* load lic coeffs */
      BWFE_P_ToggleBit(h, BCHP_WFE_CORE_LICCOEFEN, 0x00000001);
   }
   
   /* deselect lanes */
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_LICCTL1, ~0x00000003);
   
   return retCode;
}


#ifdef BWFE_FLOAT_MODE
/******************************************************************************
 BWFE_g3_Lic_P_CalcMatrixEEt() - EEt = E * E'
 input EEt: matrix(s*L x s*L), d: matrix(L x 1)
******************************************************************************/
#if 0
/* define the common LIC parameters denoted as LIC_ */
#define LICn1  100 /* 100 */ /* 400 */ /* group delay filter length = 2*n1+1  */
#define LANE_LEN 32      /* specific to 3128C0  */
#endif
void BWFE_g3_Lic_P_CalcMatrixEEt(double **EEt, double *d)
{
   double x_frac, sin_xfrac, sin_x, sinc_x;
   double vec[2];
   int32_t x_int;
   uint8_t i, j, m, n;

   for (i = 0; i < BWFE_LIC_L; i++)
   {
      for (j = i; j < BWFE_LIC_L; j++)
      {
         x_frac = d[BWFE_LIC_L - 1 - j] - d[BWFE_LIC_L - 1 - i];
         if (BWFE_ABS(x_frac) < BWFE_SINC_LBOUND)
         {
            /* case lower bound exceeded */
            for (m = 0; m < BWFE_LIC_S; m++)
            {
               for (n = 0; n < BWFE_LIC_S; n++)
               {
                  /* set diagonals to one, others to zero */
                  if ((m == n) && (j == i))
                     EEt[i * BWFE_LIC_S + m][j * BWFE_LIC_S + n] = 1.0;
                  else
                     EEt[i * BWFE_LIC_S + m][j * BWFE_LIC_S + n] = 0.0;
               }
            }
         }
         else
         {
            /* calculate sin_xfrac = sin(PI*x_frac) */
            BWFE_P_CordicRotationMode(vec, x_frac, true);   /* unit mode */
            sin_xfrac = vec[1];
            
            for (m = 0; m < BWFE_LIC_S; m++)
            {
               x_int = m * BWFE_LIC_L + j - i;
               sin_x = sin_xfrac;
               
               if (x_int % 2 != 0)
                  sin_x = -sin_x;
               
               /* calculate sinc(x) */
               sinc_x = sin_x / ((x_int + x_frac) * BWFE_PI);
               
               for (n = 0; n < BWFE_LIC_S - m; n++)
                  EEt[i * BWFE_LIC_S + n][j * BWFE_LIC_S + n + m] = sinc_x;
            }
            
            for (m = 1; m < BWFE_LIC_S; m++)
            {
               x_int = -m * BWFE_LIC_L + j - i;
               sin_x = sin_xfrac;
               
               if (x_int % 2 != 0)
                  sin_x = -sin_x;
               
               /* calculate sinc(x) */
               sinc_x = sin_x / ((x_int + x_frac) * BWFE_PI);
               
               for (n = 0; n < BWFE_LIC_S - m; n++)
                  EEt[i * BWFE_LIC_S + n + m][j * BWFE_LIC_S + n] = sinc_x;
            }
         }
      }
      
      for (j = 0; j < i; j++)
      {
         for (m = 0; m < BWFE_LIC_S; m++)
         {
            /* use symmetry */
            for (n = 0; n < BWFE_LIC_S; n++)
               EEt[i * BWFE_LIC_S + m][j * BWFE_LIC_S + n] = EEt[j * BWFE_LIC_S + n][i * BWFE_LIC_S + m];
         }
      }
   }
}


/******************************************************************************
 BWFE_g3_Lic_P_CalcMatrixEPt() - EPt = E * P'
 input Rt: matrix(s*L x L), d: matrix(L x 1)
******************************************************************************/
void BWFE_g3_Lic_P_CalcMatrixEPt(double **Rt, double *d)
{
   double sin_xfrac, sin_x, sinc_x;
   double vec[2];
   uint8_t d_prime, mod_i, z1;
   int32_t x_int;
   uint8_t i, j, k;
   
   d_prime = (BWFE_LIC_S - 1) / 2;
   for (i = 0; i < BWFE_LIC_L; i++)
   {
      /* TBD no main tap therefore mod_i = i? */
      mod_i = i % BWFE_LIC_L;
      if (i > mod_i)
         z1 = (d_prime + 1) * BWFE_LIC_L + mod_i;
      else
         z1 = d_prime * BWFE_LIC_L + mod_i;
      
      for (j = 0; j < BWFE_LIC_L; j++)
      {
         /* calculate sin_xfrac = sin(PI*d[BWFE_NUM_LANES-1-j]) */
         BWFE_P_CordicRotationMode(vec, d[BWFE_LIC_L - 1 - j], true);   /* unit mode */
         sin_xfrac = vec[1];
         
         for (k = 0; k < BWFE_LIC_S; k++)
         {
            x_int = z1 - k * BWFE_LIC_L - j;
            if (BWFE_ABS(d[BWFE_LIC_L - 1 - j]) < BWFE_SINC_LBOUND)
            {
               /* case lower bound exceeded */
               if (x_int)
                  sinc_x = 0.0;
               else
                  sinc_x = 1.0;
            }
            else
            {
               if (x_int % 2)
                  sin_x = sin_xfrac;
               else
                  sin_x = -sin_xfrac;
               
               /* calculate sinc(x) */
               sinc_x = sin_x / ((x_int - d[BWFE_LIC_L - 1 - j]) * BWFE_PI);
            }
            Rt[j * BWFE_LIC_S + k][i] = sinc_x;
         }
      }
   }
}


/**************************************************************
 BWFE_g3_Lic_P_CalcMatrixGaussJ - reduced row echelon form
**************************************************************/
#define BWFE_SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}
void BWFE_g3_Lic_P_CalcMatrixGaussJ(double **a, int n, double **b, int m)
{
	uint32_t indxc[BWFE_LIC_S * BWFE_LIC_L];
   uint32_t indxr[BWFE_LIC_S * BWFE_LIC_L];
   uint32_t ipiv[BWFE_LIC_S * BWFE_LIC_L];
	uint8_t i, icol = 0, irow = 0;
   uint8_t j, k, l, ll;
	double big, dum, pivinv, temp;
   
   n = BWFE_LIC_S * BWFE_LIC_L;
   m = BWFE_LIC_L;
   
   for (j = 0; j < n; j++)
      ipiv[j] = 0;
   
   for (i = 0; i < n; i++)
   {
      big = 0.0;
      for (j = 0; j < n; j++)
      {
         if (ipiv[j] != 1)
         {
            for (k = 0; k < n; k++)
            {
               if (ipiv[k] == 0)
               {
                  if (BWFE_ABS(a[j][k]) >= big)
                  {
                     big = BWFE_ABS(a[j][k]);
                     irow = j;
                     icol = k;
                  }
               }
            }
         }
      }
      
		++(ipiv[icol]);
		if (irow != icol)
      {
			for (l = 0; l < n; l++)
            BWFE_SWAP(a[irow][l], a[icol][l])
			for (l = 0; l < m; l++)
            BWFE_SWAP(b[irow][l], b[icol][l])
		}
		indxr[i] = irow;
		indxc[i] = icol;
		if (a[icol][icol] == 0.0)
      {
         BDBG_ERR(("Singular Matrix!\n"));
      }
      
		pivinv = 1.0 / a[icol][icol];
		a[icol][icol] = 1.0;
		for (l = 0; l < n; l++)
         a[icol][l] *= pivinv;
		for (l = 0; l < m; l++)
         b[icol][l] *= pivinv;
      
		for (ll = 0; ll < n; ll++)
      {
			if (ll != icol)
         {
				dum = a[ll][icol];
				a[ll][icol] = 0.0;
				for (l = 0; l < n; l++)
               a[ll][l] -= a[icol][l] * dum;
				for (l = 0; l < m; l++)
               b[ll][l] -= b[icol][l] * dum;
			}
      }
	}
   
	for (l = n - 1; l >= 0; l--)
   {
		if (indxr[l] != indxc[l])
      {
			for (k = 0; k < n; k++)
				BWFE_SWAP(a[k][indxr[l]], a[k][indxc[l]]);
      }
	}
}


/**************************************************************
 BWFE_g3_Lic_P_CalcMatrixLuDecomp()
**************************************************************/
#define BWFE_TINY 1.0e-20
void BWFE_g3_Lic_P_CalcMatrixLuDecomp(double **a, int n, int *indx, double *d)
{
   uint8_t i, j, k, i_max = 0;
   double big, dum, sum, temp;
   double vv[BWFE_LIC_S * BWFE_LIC_L];
   
   n = BWFE_LIC_S * BWFE_LIC_L;
   *d = 1.0;
   
   for (i = 0; i < n; i++)
   {
      big = 0.0;
      for (j = 0; j < n; j++)
      {
         if ((temp = BWFE_ABS(a[i][j])) > big)
            big = temp;
      }
      
      if (big == 0.0)
      {
         /* check for singular matrix */
         BDBG_ERR(("Singular Matrix!\n"));
      }
      vv[i] = 1.0 / big;
   }
   
   for (j = 0; j < n; j++)
   {
      for (i = 0; i < j; i++)
      {
         sum = a[i][j];
         for (k = 0; k < i; k++)
            sum -= a[i][k] * a[k][j];
         a[i][j] = sum;
      }
      
      big = 0.0;
      for (i = j; i < n; i++)
      {
         sum = a[i][j];
         for (k = 0; k < j; k++)
            sum -= a[i][k] * a[k][j];
         a[i][j] = sum;
         if ((dum = vv[i] * BWFE_ABS(sum)) >= big)
         {
            big = dum;
            i_max = i;
         }
      }
      if (j != i_max)
      {
         for (k = 0; k < n; k++)
         {
            dum = a[i_max][k];
            a[i_max][k] = a[j][k];
            a[j][k] = dum;
         }
         *d = -(*d);
         vv[i_max] = vv[j];
      }
      indx[j] = i_max;
      
      if (a[j][j] == 0.0)
         a[j][j] = BWFE_TINY;
         
      if (j != (n - 1)) 
      {
         dum = 1.0 / (a[j][j]);
         for (i = j + 1; i < n; i++)
            a[i][j] *= dum;
      }
   }
}
#undef BWFE_TINY


/**************************************************************
 BWFE_g3_Lic_P_CalcMatrixLuBackSub()
**************************************************************/
void BWFE_g3_Lic_P_CalcMatrixLuBackSub(double **a, int n, int *indx, double b[])
{
   int i, ii = 0, ip, j;
   double sum;
   
   for (i = 0; i < n; i++)
   {
      ip = indx[i];
      sum = b[ip];
      b[ip] = b[i];
      if (ii)
      {
         for (j = ii - 1; j < i; j++)
            sum -= a[i][j]*b[j];
      }
      else if (sum != 0.0)
      {
         ii = i + 1;
      }
      b[i] = sum;
   }
   
   for (i = n - 1; i >= 0; i--)
   {
      sum = b[i];
      for (j = i + 1; j < n; j++)
         sum -= a[i][j] * b[j];
      b[i] = sum / a[i][i];
   }
}
#endif


/******************************************************************************
 BWFE_g3_Lic_P_ComputeTaps()
 inputs: h->corrDelay
 outputs: h->licCoeff
******************************************************************************/
BERR_Code BWFE_g3_Lic_P_ComputeTaps(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_NOT_SUPPORTED;
#ifdef BWFE_FLOAT_MODE
   int32_t deltaDelay, val;
   uint8_t slice, lane;
   uint8_t i, j, k, n;
#endif
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
#ifdef BWFE_FLOAT_MODE
   double vec[2], delay[BWFE_LIC_L], sin_delay[BWFE_LIC_L];
   double EEt[BWFE_LIC_S * BWFE_LIC_L][BWFE_LIC_S * BWFE_LIC_L];  /* matrix(s*L x s*L) */
   double Rt[BWFE_LIC_S * BWFE_LIC_L][BWFE_LIC_L];                /* matrix(s*L x L) */
   
   uint32_t indx[BWFE_LIC_S * BWFE_LIC_L];
   double bvect[BWFE_LIC_S * BWFE_LIC_L];
   double dummy;
   
   double tapOut[BWFE_LIC_L][BWFE_LIC_S * BWFE_LIC_L];   /* matrix(L x s*L) */
   double tapCoeff[BWFE_LIC_S * BWFE_LIC_L][BWFE_LIC_L]; /* matrix(s*L x L) */
   
   for (slice = 0; slice < BWFE_NUM_SLICES; slice++)
   {
   #if 0
      /* TBD enforce n to be multiple of L? */
      if( LICn1 % BWFE_NUM_LANES > 0)
         n = LICn1 + (BWFE_NUM_LANES - LICn1 % BWFE_NUM_LANES);
      else
         n = LICn1;
      hL = 2*n/BWFE_NUM_LANES + 1;
      t = BWFE_NUM_LIC_TAPS + hL - 1;
   #endif
      
      /* current LIC2 delays based on new corr values, limit change to +/-BWFE_DELAY_DELTA  */
      for (lane = 0; lane < BWFE_NUM_LANES; lane++)
      {
         if (h->bLicTrackingOn)
         {
            /* get delta from prev delays and limit change */
            deltaDelay = h->corrDelay[slice][lane] - h->licDelay[slice][lane];
            if (deltaDelay >= BWFE_DELAY_DELTA)
               h->licDelay[slice][lane] += BWFE_DELAY_DELTA;
            else if (deltaDelay <= -BWFE_DELAY_DELTA)
               h->licDelay[slice][lane] -= BWFE_DELAY_DELTA;
            else
               h->licDelay[slice][lane] = h->corrDelay[slice][lane];
         }
         else
         {
            /* use new correlator values */
            h->licDelay[slice][lane] = h->corrDelay[slice][lane];
         }
         delay[lane] = (double)(h->licDelay[slice][lane] / BWFE_DELAY_SCALE);
      }
      
      /* calculate sin(x) used by sin(x)/x later, note delay already norm by PI */
      for (i = 0; i < BWFE_LIC_L; i++)
      {
         /* calculate sin(x) using cordic */
         BWFE_P_CordicRotationMode(vec, delay[i], true); /* unit mode */
         sin_delay[i] = vec[1];
      }
      
      /* perform E*E' direct closed form calculations, exploits symmetry */
      BWFE_Lic_P_CalcMatrixEEt((double**)EEt, delay);
      
      /* calculate Rt = E*P'  */
      BWFE_Lic_P_CalcMatrixEPt((double**)Rt, delay);
      
      /* solve (E*E')*R' = E*P' */
   #if 0
      /* gauss-jordan elimination method */
      BWFE_Lic_P_CalcMatrixGaussJ((double**)EEt, BWFE_NUM_LIC_TAPS * BWFE_NUM_LANES, (double**)Rt, BWFE_NUM_LANES);
   #else
      /* LU decomposition method */
      BWFE_Lic_P_CalcMatrixLuDecomp((double**)EEt, BWFE_LIC_S * BWFE_LIC_L, (int*)indx, &dummy);
      
      /* solve one column at a time */
      for (i = 0; i < BWFE_LIC_L; i++)
      {
         for (j = 0; j < BWFE_LIC_S * BWFE_LIC_L; j++)
            bvect[j] = Rt[j][i];
         BWFE_Lic_P_CalcMatrixLuBackSub((double**)EEt, BWFE_LIC_S * BWFE_LIC_L, (int*)indx, bvect);
         for (j = 0; j < BWFE_LIC_S * BWFE_LIC_L; j++)
            Rt[j][i] = bvect[j];
      }
   #endif
      
      /* print matrix */
      BDBG_MSG(("\nMatrix EEt:\n"));
      for (i = 0; i < BWFE_LIC_S * BWFE_LIC_L; i++)
      {
         for (j = 0; j < BWFE_LIC_S * BWFE_LIC_L; j++)
            BDBG_MSG(("%10d ", (int32_t)(65536.0 * EEt[i][j])));
      }
      
      /* sort taps in intermediate array */
      for (i = 0; i < BWFE_LIC_L; i++)
      {
         for (j = 0; j < BWFE_LIC_L; j++)
         {
            for (k = 0; k < BWFE_LIC_S; k++)
               tapOut[i][j + k * BWFE_LIC_L] = Rt[i * BWFE_LIC_S + k][BWFE_LIC_L - 1 - j];
         }
      }
      
      /* final coefficients */
      for (i = 0; i < BWFE_LIC_L; i++)
      {
         n = 0;
         for (j = 0; j < BWFE_LIC_S; j++)
         {
            for (k = 0; k < BWFE_LIC_L; k++)
            {
               tapCoeff[n][i] = tapOut[k][i + j * BWFE_LIC_L];
               n++;
            }
         }
      }
      
      BDBG_MSG(("\nLIC tap coeffs INT (scale 2^16)\n"));
      for (i = 0; i < BWFE_LIC_S * BWFE_LIC_L; i++)
      {
         for (j = 0; j < BWFE_LIC_L; j++)
         {
            BDBG_MSG(("%18d ", BWFE_ROUND(65536.0 * tapCoeff[i][j])));  /* 2^16=65536   (was 2^18 == 262144)  */
         }
      }
      
      /* zero out lic coefficients */
      BKNI_Memset(h->licCoeff, 0, BWFE_NUM_LANES * BWFE_NUM_LIC_TAPS);
      
      /* format lic taps for register programming TC2.16 */
      for (i = 0; i < BWFE_LIC_L; i++)
      {
         k = BWFE_LIC_L - 1 - i;
         
         for (j = k; j < BWFE_LIC_S * BWFE_LIC_L + k; j++)
         {
            val = BWFE_ROUND(65536.0 * tapCoeff[BWFE_LIC_S * BWFE_LIC_L - 1 - (j - k)][i]);
            h->licCoeff[i][j] = (uint32_t)val;
         }
      }
      
      /* write lic coefficients */
      BWFE_Lic_P_WriteTaps(h);
   }
#else
   /* TBD fixed point implementation */
#endif
   
   return retCode;
}
#endif
