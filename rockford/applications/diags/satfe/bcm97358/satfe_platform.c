/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: satfe_platform.c $
 * $brcm_Revision: Hydra_Software_Devel/30 $
 * $brcm_Date: 8/8/13 5:15p $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: /rockford/applications/diags/satfe/bcm97358/satfe_platform.c $
 *
 * Hydra_Software_Devel/30   8/8/13 5:15p ronchan
 * SWSATFE-88: fixed input power estimation to use current channel
 *
 * Hydra_Software_Devel/29   5/1/13 4:08p ckpark
 * SWSATFE-264: fixed bug in special_acq()
 *
 * Hydra_Software_Devel/28   5/1/13 3:47p enavarro
 * SWSATFE-264: set final non-decimating filter to quarterband in
 * special_acq()
 *
 * Hydra_Software_Devel/27   4/30/13 2:55p enavarro
 * SWSATFE-264: added special_acq
 *
 * Hydra_Software_Devel/26   4/16/12 5:18p enavarro
 * SWSATFE-182: added debug code (commented out) to do initial acquisition
 * on startup
 *
 * Hydra_Software_Devel/25   3/5/12 11:12a enavarro
 * SWSATFE-88: fixed compiler warning
 *
 * Hydra_Software_Devel/24   2/6/12 5:54p agin
 * SWNOOS-516:  Need to define sb.
 *
 * Hydra_Software_Devel/23   1/17/12 9:46a ronchan
 * SWSATFE-88: use BAST_SetFskChannel to configure fsk channels
 *
 * Hydra_Software_Devel/22   12/8/11 10:08a ronchan
 * SWSATFE-88: config L1 interrupts if specified
 *
 * Hydra_Software_Devel/21   10/17/11 10:47a enavarro
 * SWSATFE-88: updated input power estimate
 *
 * Hydra_Software_Devel/20   10/12/11 11:13a enavarro
 * SWSATFE-88: updated fixed point implementation of power estimate
 *
 * Hydra_Software_Devel/19   10/11/11 2:57p enavarro
 * SWSATFE-88: updated power estimate
 *
 * Hydra_Software_Devel/18   10/7/11 9:30a enavarro
 * SWSATFE-88: updated input power estimate
 *
 * Hydra_Software_Devel/17   10/5/11 3:35p enavarro
 * SWSATFE-88: updated input power estimate
 *
 * Hydra_Software_Devel/16   9/28/11 2:38p ronchan
 * SWSATFE-88: add reset sds function, add pinmuxing, updated input power
 * estimation
 *
 * Hydra_Software_Devel/15   9/7/11 2:36p enavarro
 * SWSATFE-86: updated SATFE_PlatformFunctTable
 *
 * Hydra_Software_Devel/14   8/30/11 10:29a ronchan
 * SWSATFE-88: implement input power calculations
 *
 * Hydra_Software_Devel/13   7/21/11 1:54p ronchan
 * SWSATFE-88: turn off daisy since unused
 *
 * Hydra_Software_Devel/12   6/17/11 11:23a ronchan
 * SWSATFE-88: bypass LNA only for SV board
 *
 * Hydra_Software_Devel/11   4/27/11 5:35p ronchan
 * SWSATFE-88: bypass internal LNA PGA
 *
 * Hydra_Software_Devel/10   4/23/11 2:36p ronchan
 * SWSATFE-88: added i2c handle for intersil chip
 *
 * Hydra_Software_Devel/9   3/30/11 4:52p enavarro
 * SWSATFE-75: bring in changes from 7344
 *
 * Hydra_Software_Devel/8   1/7/11 2:33p ronchan
 * SWSATFE-88: use BTMR to implement timer functions
 *
 * Hydra_Software_Devel/7   1/7/11 2:25p ronchan
 * SWSATFE-88: moved main on-idle function to satfe level
 *
 * Hydra_Software_Devel/6   11/19/10 3:54p enavarro
 * SWSATFE-88: added SATFE_97358_OpenChannels(); dont need to open AST
 * handles if SATFE_Diags_Config.bAstOpenExternal is true
 *
 * Hydra_Software_Devel/5   10/27/10 4:18p ronchan
 * SWSATFE-75: added OpenChannels to SATFE_PlatformFunctTable
 *
 * Hydra_Software_Devel/4   10/15/10 8:41a enavarro
 * SWSATFE-75: fixed compiler warnings
 *
 * Hydra_Software_Devel/3   9/28/10 9:41a enavarro
 * SWSATFE-75: dont set BDBG_Level n SATFE_97358_Open()
 *
 * Hydra_Software_Devel/2   9/27/10 5:29p enavarro
 * SWSATFE-75: fixed compiler errors
 *
 * Hydra_Software_Devel/1   9/15/10 4:39p enavarro
 * SWSATFE-75: initial version
 *
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "int1.h"
#include "int1_api.h"
#include "btmr.h"
#include "bint_plat.h"
#include "breg_i2c.h"
#include "prompt.h"
#include "satfe.h"
#include "satfe_g3.h"
#include "satfe_platform.h"

#include "bchp_hif_cpu_intr1.h"
#include "bchp_sds_gr_bridge.h"
#include "bchp_aon_pin_ctrl.h"
#include "bchp_sun_top_ctrl.h"

extern void _writeasm(char);
#include "bcmuart.h"

extern BREG_I2C_Handle diags_hRegI2c[4];


/* global variables */
static SATFE_Diags_Config *pSatfeConfig = NULL;
BTMR_TimerHandle hTimer = NULL;
volatile uint32_t SATFE_TimerCount = 0;


/* local functions */
BERR_Code SATFE_97358_Open(SATFE_Chip *pChip, void *pParam);
BERR_Code SATFE_97358_OpenChannels(SATFE_Chip *pChip, void *pParam);
BERR_Code SATFE_97358_SetPinMux(SATFE_Chip *pChip, void *pParam);
BERR_Code SATFE_97358_InitHandle(SATFE_Chip *pChip, void *pParam);
BERR_Code SATFE_97358_Configure(SATFE_Chip *pChip, void *pParam);
void SATFE_97358_OnIdle();
bool SATFE_97358_Command_help(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_97358_Command_diseqc_reset(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_97358_Command_isl9492_status(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_97358_Command_reset_sds(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_97358_Command_special_acq(SATFE_Chip *pChip, int argc, char **argv);


/* define function table for this platform */
static SATFE_PlatformFunctTable SATFE_97358_Functs =
{
   SATFE_97358_Open,         /* open AST device */
   SATFE_97358_OpenChannels, /* open AST channels */
   SATFE_97358_InitHandle,   /* init SATFE_Chip handle */
   SATFE_97358_Configure,    /* chip configuration after AP starts (e.g. LNA, diseqc, etc) */
   NULL,                     /* special handling for TuneAcquire (e.g. external tuner) */
   SATFE_97358_OnIdle,       /* platform idle function */
   NULL                      /* close */
};


/* list all Broadcom frontend chips on this board */
SATFE_ChipDescriptor SATFE_chips[SATFE_NUM_CHIPS] =
{
   {
      "BCM7358",   /* name */
      0x7358,      /* chip id */
      BCHP_VER_A0, /* chip version */
      1,           /* number of downstream channels */
      1,           /* number of internal tuners */
      0,           /* i2c address */
      &SATFE_97358_Functs,
   }
};


/* define additional commands specific to this platform */
SATFE_Command SATFE_97358_CommandSet[] =
{
   {"isl9492_status", SATFE_97358_Command_isl9492_status},
   {"reset_sds", SATFE_97358_Command_reset_sds},
   {"special_acq", SATFE_97358_Command_special_acq},
   {0, NULL},
};


#define SATFE_L1_INT_COUNT 9
/******************************************************************************
 SATFE_Platform_Init()
******************************************************************************/
BERR_Code SATFE_Platform_Init(void *pParam)
{
   uint8_t i;
   uint32_t irq[SATFE_L1_INT_COUNT] = {
      BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_FTM_CPU_INTR_SHIFT,
      32 + BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_SDS0_AFEC_CPU_INTR_SHIFT,
      32 + BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_SDS0_RCVR_0_CPU_INTR_SHIFT,
      32 + BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_SDS0_RCVR_1_CPU_INTR_SHIFT,
      32 + BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_SDS0_TFEC_CPU_INTR_SHIFT,
      32 + BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_UPG_AUX_AON_CPU_INTR_SHIFT,
      32 + BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_UPG_BSC_CPU_INTR_SHIFT,
      32 + BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_UPG_BSC_AON_CPU_INTR_SHIFT,
   #if 0
      32 + BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_UPG_MAIN_CPU_INTR_SHIFT,
      32 + BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_UPG_MAIN_AON_CPU_INTR_SHIFT,
      32 + BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_UPG_SC_CPU_INTR_SHIFT,
      32 + BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_UPG_SPI_CPU_INTR_SHIFT,
   #endif
      32 + BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_UPG_TMR_CPU_INTR_SHIFT
   };

   pSatfeConfig = (SATFE_Diags_Config *)pParam;

   if (pSatfeConfig->bConfigL1Int)
   {
      /* hook up the interrupts */
      for (i = 0; i < SATFE_L1_INT_COUNT; i++)
      {
         CPUINT1_ConnectIsr(irq[i], (FN_L1_ISR)BINT_Isr, (void*)pSatfeConfig->hInt, irq[i]);
         CPUINT1_Enable(irq[i]);
      }
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 SATFE_Platform_Shutdown()
******************************************************************************/
BERR_Code SATFE_Platform_Shutdown()
{
   SATFE_Chip *pChip;
   int i;

   for (i = 0; i < SATFE_NUM_CHIPS; i++)
   {
      pChip = SATFE_GetChipByIndex(i);
      if (pChip->pImpl)
      {
         BKNI_Free(pChip->pImpl);
         pChip->pImpl = NULL;
      }
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 SATFE_Platform_InitDiags()
******************************************************************************/
void SATFE_Platform_InitDiags(void *pParam)
{
   BSTD_UNUSED(pParam);
}


/******************************************************************************
 SATFE_Platform_GetChar()
******************************************************************************/
char SATFE_Platform_GetChar(bool bBlocking)
{
   char c;

   do {
      c = det_in_char();
      SATFE_OnIdle();
   } while (bBlocking && (c <= 0));
   if (c > 0)
      console_out(c);
   return c;
}


/******************************************************************************
 SATFE_Platform_Backspace()
******************************************************************************/
void SATFE_Platform_Backspace()
{
   console_out(' ');
   console_out(0x08);
}


/******************************************************************************
 SATFE_findmsb() - returns the position of most significant bit.
******************************************************************************/
uint32_t SATFE_findmsb(uint32_t i)
{
   uint32_t j=0;
   while (i>0)
   {
      i>>=1;
      j++;
   }
   return j;
}


/******************************************************************************
 SATFE_log10() - returns log10(x) in 4.28 format
 assumptions: i is an integer ,uses interpolation to find logtobase2 then
 find logtobase10 from it.
******************************************************************************/
uint32_t SATFE_log10(uint32_t i)
{
   uint32_t log10=0x34936000,log2,fixans;
   uint32_t diff,den;
   uint32_t divh,divl,j,ansl,mul1h,mul1l,mul2h,mul2l,logh,logl;
   log10=882073600;
   j=SATFE_findmsb(i);  /*the msb position is equal to log2base2*/
   diff =(i-(1<<(j-1)));
   den=(1<<j)-(1<<(j-1));
   BAST_DivU64U32(diff,0,den,&divh,&divl); /*the slope calculated for interpolation*/
   j--;
   log2=j+divh;/*found the log2base2*/
   BAST_DivU64U32(log2,divl,3321,&divh,&ansl);/*convert log2base2 to log2base10 3.321 is log10tobase2*/
   BAST_MultU32U32(divh,1000,&mul1h,&mul1l);
   BAST_MultU32U32(ansl,1000,&mul2h,&mul2l);
   logh=mul1h+mul2h;
   logl=mul1l+mul2l;
   fixans=(logh & 0xf)<<28;/*convert to 4.28 format*/
   fixans+=((logl >> 4));
   return fixans;
}


/******************************************************************************
SATFE_Platform_GetInputPower()
******************************************************************************/
/* #define SATFE_USE_FLOATING_POINT */
void SATFE_Platform_GetInputPower(SATFE_Chip *pChip, uint32_t rfagc,
                                         uint32_t ifagc, uint32_t agf, uint32_t tuner_freq,
                                         float *pPower)
{
   BERR_Code retCode;
   BAST_TunerLnaStatus lnaStatus;
#ifdef SATFE_USE_FLOATING_POINT
   float G_lna, G_rfpga, G_fga, G_ifpga, G_aif, G_agt, G, P_freq, P_ndfctl;
#else
   int32_t G_lna, G_rfpga, G_ifpga, G_fga, G_aif, G_agt, P_ndfctl, P_freq, P_adj, a, d;
   uint32_t P_hi, P_lo, Q_hi, Q_lo, b, c, val32;
#endif
   int32_t r1, r2, r3;

   BSTD_UNUSED(rfagc);
   *pPower = 0;

   if (pChip->idx > 0)
   {
      SATFE_GOTO_DONE_ON_ERROR("pChip->idx", BERR_INVALID_PARAMETER);
   }

   /* get the internal LNA status */
   SATFE_MUTEX(retCode = BAST_GetTunerLnaStatus(pChip->hAstChannel[pChip->currChannel], &lnaStatus));
   SATFE_GOTO_DONE_ON_ERROR("BAST_GetTunerLnaStatus()", retCode);
   r1 = (lnaStatus.lnaAgc >> 26) & 0x3F;
   r2 = (lnaStatus.bbAgc >> 26) & 0x3F;

   /* internal SDS tuner */
   r3 = (ifagc >> 26) & 0x3F;
   if (r3 >= 54)
      r3 = 54;
#ifdef SATFE_USE_FLOATING_POINT
   G_lna = 16.0 - (0.5 * (float)(63 - r1));
   G_rfpga = 1.0 - (0.3 * (float)(63 - r2));
   G_ifpga = (0.5 * (float)r3) - 3.0;
   G_fga = 15.0;
   G_aif = 20.0 * log10(agf & 0xFFFFFF00);

   if ((agf & 0x0F) != 0x0A)
      G_agt = -6.8124;
   else
      G_agt = 0;

   /* non-decimating filter adjustment */
   switch (agf & 0xC0)
   {
      case 0x00:  /* quarterband */
         P_ndfctl = 0;     /* no adjustment */
         break;
      case 0x40:  /* thirdband */
         P_ndfctl = -2.5;   /* adjust by 20log10(.75) */
         break;
      case 0x80:  /* halfband */
         P_ndfctl = -6;     /* adjust by 20log10(.5) */
         break;
      default:    /* sharpened halfband */
         P_ndfctl = -12;    /* adjust by 20log10(.25) */
         break;
   }

   /* tuner_freq in Hz */
   if (tuner_freq <= 950000000UL)
      P_freq = 22.0;
   else if (tuner_freq <= 1085000000UL)
      P_freq = 22.0 - (((float)tuner_freq - 950000000.0) / 135000000.0);
   else if (tuner_freq <= 1300000000UL)
      P_freq = 21.0 + (1.5*((float)tuner_freq - 1085000000.0) / 215000000.0);
   else if (tuner_freq <= 1445000000UL)
      P_freq = 22.5 - (1.5 *((float)tuner_freq - 1300000000.0) / 145000000.0);
   else if (tuner_freq <= 1575000000UL)
      P_freq = 21.0;
   else if (tuner_freq <= 1665000000UL)
      P_freq = 21.0 + (0.5 *((float)tuner_freq - 1575000000.0) / 90000000.0);
   else if (tuner_freq <= 2150000000UL)
      P_freq = 21.5 - (2.0 *((float)tuner_freq - 1665000000.0) / 485000000.0);
   else
      P_freq = 19.5;
   G = G_lna + G_rfpga + G_ifpga + G_fga + G_aif + G_agt + P_ndfctl + P_freq;
#if 0
printf("r1=%d, r2=%d, r3=%d\n", r1, r2, r3);
printf("G_lna=%f, G_rfpga=%f, G_ifpga=%f, G_fga=%f, G_aif=%f, G_agt=%f, P_ndfctl=%f, P_freq=%f\n",
#endif
   G_lna, G_rfpga, G_ifpga, G_fga, G_aif, G_agt, P_ndfctl, P_freq);
   *pPower = 168.0 - G;
#else
   G_lna = ((63-r1) << 23) - (16 << 24); /* scaled 2^24 */
   G_rfpga = 300312166 - (r2*5033165);   /* scaled 2^24 */
   G_ifpga = (3<<24) - (r3*(1<<23)); /* scaled 2^24 */
   G_fga = -251658240; /* scaled 2^24 */
   b = SATFE_log10(agf & 0xFFFFFF00); /* Q28 */
   b = (b >> 8);
   b &= ~0xFF000000;
   G_aif = -20 * (int32_t)b; /* Q20 */
   if ((agf & 0x0F) != 0x0A)
      G_agt = 114293106;
   else
      G_agt = 0;
   switch (agf & 0xC0)
   {
      case 0x00: /* quarterband */
         P_ndfctl = 0; /* no adjustment */
         break;
      case 0x40: /* thirdband */
         P_ndfctl = 41943040; /* adjust by 20log10(.75)*2^24 */
         break;
      case 0x80: /* halfband */
         P_ndfctl = 6<<24; /* adjust by 20log10(.5)*2^24 */
         break;
      default: /* sharpened halfband */
         P_ndfctl = 12<<24; /* adjust by 20log10(.25)*2^24 */
         break;
   }

   if (tuner_freq <= 950000000UL)
      P_freq = 22<<24; /* 22.0*2^24 */
   else if (tuner_freq > 2150000000UL)
      P_freq = 327155712; /* 19.5*2^24 */
   else
   {
      /* P_freq = a*(tuner_freq-b)/c + d */
      if (tuner_freq <= 1085000000UL)
      {
         /* P_freq = 22.0 - (((float)tuner_freq - 950000000.0) / 135000000.0) */
         a = 1;
         b = 950000000UL;
         c = 135000000;
         d = 22<<24;
      }
      else if (tuner_freq <= 1300000000UL)
      {
         /* P_freq = 21.0 + (1.5*((float)tuner_freq - 1085000000.0) / 215000000.0) */
         a = 25165824;
         b = 1085000000UL;
         c = 215000000;
         d = 21<<24;
      }
      else if (tuner_freq <= 1445000000UL)
      {
         /* P_freq = 22.5 - (1.5 *((float)tuner_freq - 1300000000.0) / 145000000.0) */
         a = -25165824;
         b = 1300000000UL;
         c = 145000000;
         d = 377487360;
      }
      else if (tuner_freq <= 1575000000UL)
      {
         /* P_freq = 21.0 */
         a = 0;
         b = 0;
         c = 1;
         d = 21<<24;
      }
      else if (tuner_freq <= 1665000000UL)
      {
         /* P_freq = 21.0 + (0.5 *((float)tuner_freq - 1575000000.0) / 90000000.0) */
         a = 8388608;
         b = 1575000000UL;
         c = 90000000;
         d = 21<<24;
      }
      else /* (tuner_freq <= 2150000000UL) */
      {
         /* P_freq = 21.5 - (2.0 *((float)tuner_freq - 1665000000.0) / 485000000.0) */
         a = -33554432;
         b = 1665000000UL;
         c = 485000000;
         d = 360710144;
      }

      if (a >= 0)
         val32 = (uint32_t)a;
      else
         val32 = (uint32_t)-a;
      BAST_MultU32U32(tuner_freq - b, val32, &P_hi, &P_lo);
      BAST_DivU64U32(P_hi, P_lo, c, &Q_hi, &Q_lo);
      if (a >= 0)
         P_freq = (int32_t)Q_lo;
      else
         P_freq = (int32_t)-Q_lo;
      P_freq += d;
      P_freq = -P_freq;
   }

   P_adj = (int32_t)(168<<20); /* scaled 2^20 */
#if 0
   printf("G_lna=%f, G_rfpga=%f, G_ifpga=%f, G_fga=%f, G_aif=%f, G_agt=%f, P_ndfctl=%f, P_freq=%f\n",
       (double)G_lna/pow(2,24), (double)G_rfpga/pow(2,24), (double)G_ifpga/pow(2,24), (double)G_fga/pow(2,24), (double)G_aif/pow(2,20), (double)G_agt/pow(2,24), (double)P_ndfctl/pow(2,24), (double)P_freq/pow(2,24));
#endif
   *pPower = (float)(G_lna + G_rfpga + G_ifpga + G_fga + G_agt + P_ndfctl + P_freq) / 16777216.0;
   *pPower += (float)(G_aif + P_adj) / 1048576.0;
#endif

   done:
   return;
}


/******************************************************************************
 SATFE_Platform_StartTimer()
******************************************************************************/
void SATFE_Platform_StartTimer()
{
   BTMR_StopTimer(hTimer);
   SATFE_TimerCount = 0;
   BTMR_StartTimer(hTimer, 1000);
}


/******************************************************************************
 SATFE_Platform_KillTimer()
******************************************************************************/
void SATFE_Platform_KillTimer()
{
   BTMR_StopTimer(hTimer);
}


/******************************************************************************
 SATFE_Platform_GetTimerCount()
******************************************************************************/
uint32_t SATFE_Platform_GetTimerCount()
{
   return SATFE_TimerCount;
}


/******************************************************************************
 SATFE_Platform_Rand()
******************************************************************************/
uint32_t SATFE_Platform_Rand()
{
   SATFE_Register *pFTM_PHY_LFSR;
   uint32_t val, i, r = 0;
   SATFE_Chip *pChip;

   pChip = SATFE_GetCurrChip();
   if (SATFE_LookupRegister(pChip, "FTM_PHY_LFSR", &pFTM_PHY_LFSR))
   {
      for (i = 0; i < 4; i++)
      {
         BAST_ReadRegister(pChip->hAstChannel[pChip->currChannel], (uint32_t)pFTM_PHY_LFSR->addr, &val);
         r = (r << 8) | val;
      }
      return r;
   }
   else
      return rand();
}


/******************************************************************************
 SATFE_97358_TimerCallback()
******************************************************************************/
void SATFE_97358_TimerCallback(void *param1, int param2)
{
   BSTD_UNUSED(param1);
   BSTD_UNUSED(param2);
   SATFE_TimerCount++;
}


/******************************************************************************
 SATFE_97358_Open()
******************************************************************************/
BERR_Code SATFE_97358_Open(SATFE_Chip *pChip, void *pParam)
{
   BAST_Settings settings;
   BTMR_Settings timer_settings = { BTMR_Type_ePeriodic, (BTMR_CallbackFunc)SATFE_97358_TimerCallback, NULL, 0, false };
   BERR_Code retCode;

   pSatfeConfig = (SATFE_Diags_Config *)pParam;

   /* set required pinmuxing */
   SATFE_97358_SetPinMux(pChip, pParam);

   /* create a timer instance */
   retCode = BTMR_CreateTimer(pSatfeConfig->hTimer, &hTimer, &timer_settings);
   if (retCode)
   {
      printf("BTMR_CreateTimer() error 0x%X\n", retCode);
      goto done;
   }

   if (pSatfeConfig->bAstOpenExternal)
   {
      pChip->hAst = pSatfeConfig->hAst;
      retCode = BERR_SUCCESS;
   }
   else
   {
      BAST_g3_GetDefaultSettings(&settings);
      retCode = BAST_Open(&pChip->hAst, NULL,
                     (void*)pSatfeConfig->hReg,
                     (BINT_Handle)pSatfeConfig->hInt, &settings);
   }

   done:
   return retCode;
}


/******************************************************************************
 SATFE_97358_OpenChannels()
******************************************************************************/
BERR_Code SATFE_97358_OpenChannels(SATFE_Chip *pChip, void *pParam)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_ChannelSettings chnSettings;
   SATFE_Diags_Config *pSatfeConfig = (SATFE_Diags_Config *)pParam;
   uint32_t nChannels, j;

   if (pSatfeConfig->bAstOpenExternal)
   {
      pChip->hAstChannel = pSatfeConfig->hAstChannel;
   }
   else
   {
      BAST_GetTotalChannels(pChip->hAst, &nChannels);
      pChip->hAstChannel = (BAST_ChannelHandle *)BKNI_Malloc(nChannels * sizeof(BAST_ChannelHandle));

      for (j = 0; j < nChannels; j++)
      {
         BAST_GetChannelDefaultSettings(pChip->hAst, j, &chnSettings);
         retCode = BAST_OpenChannel(pChip->hAst, &(pChip->hAstChannel[j]), j, &chnSettings);
         if (retCode != BERR_SUCCESS)
         {
            printf("BAST_OpenChannel() error 0x%X\n", retCode);
            BKNI_Free((void*)pChip->hAstChannel);
            break;
         }
      }
   }

   return retCode;
}


/******************************************************************************
 SATFE_97358_SetPinMux()
******************************************************************************/
BERR_Code SATFE_97358_SetPinMux(SATFE_Chip *pChip, void *pParam)
{
   uint32_t val;

   BSTD_UNUSED(pChip);
   BSTD_UNUSED(pParam);

   /* set pin mux for BSC_M1 for ISL9492 i2c */
   val = BREG_Read32(pSatfeConfig->hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
   val &= ~0x000000FF;
   val |=  0x00000011;
   BREG_Write32(pSatfeConfig->hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, val);

   /* set AON_GP06 for SDS0_DSEC_VCTL */
   val = BREG_Read32(pSatfeConfig->hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
   val &= ~0x0000000F;
   val |=  0x00000006;
   BREG_Write32(pSatfeConfig->hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, val);

   /* set GP23 for SDS0_DSEC_TXOUT, GP24 for SDS0_DSEC_TXEN */
   val = BREG_Read32(pSatfeConfig->hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0);
   val &= ~0x000000FF;
   val |=  0x00000011;
   BREG_Write32(pSatfeConfig->hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0, val);

   return BERR_SUCCESS;
}


/******************************************************************************
 SATFE_97358_InitHandle()
******************************************************************************/
BERR_Code SATFE_97358_InitHandle(SATFE_Chip *pChip, void *pParam)
{
   BSTD_UNUSED(pParam);

   BDBG_ASSERT(pChip->chip.type == 0x7358);
   SATFE_g3_InitHandle(pChip);

   pChip->hMutex = NULL;
   pChip->pFwImage = NULL;
   pChip->ftmRid = rand();
   pChip->pPlatformCommandSet = SATFE_97358_CommandSet;
   pChip->pImpl = NULL;

   /* override any command implementation function */
   pChip->commonCmds.help = SATFE_97358_Command_help;
   pChip->commonCmds.diseqc_reset = SATFE_97358_Command_diseqc_reset;

   return BERR_SUCCESS;
}


/******************************************************************************
 SATFE_97358_Configure()
******************************************************************************/
BERR_Code SATFE_97358_Configure(SATFE_Chip *pChip, void *pParam)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_TunerLnaSettings lnaSettings;
#if (SFF_BOARD == 0)
   uint8_t sb;
#endif

   BSTD_UNUSED(pParam);

   /* initialize ISL9492 chips */
   SATFE_97358_Command_diseqc_reset(pChip, 0, NULL);

   /* configure internal LNA crossbar */
   lnaSettings.out0 = BAST_TunerLnaOutputConfig_eIn0;
   lnaSettings.out1 = BAST_TunerLnaOutputConfig_eOff;
   lnaSettings.daisy = BAST_TunerLnaOutputConfig_eOff;
   retCode = BAST_ConfigTunerLna(pChip->hAst, &lnaSettings);
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_ConfigTunerLna() error\n");
      goto done;
   }

#if (SFF_BOARD == 0)
   /* bypass internal LNA for SV board */
   retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_TUNER_CTL, &sb, BAST_G3_CONFIG_LEN_TUNER_CTL);
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_ReadConfig(BAST_G3_CONFIG_TUNER_CTL) error\n");
      goto done;
   }
   sb |= BAST_G3_CONFIG_TUNER_CTL_LNA_BYPASS;
   retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_TUNER_CTL, &sb, BAST_G3_CONFIG_LEN_TUNER_CTL);
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_WriteConfig(BAST_G3_CONFIG_TUNER_CTL) error\n");
      goto done;
   }
#endif

   /* configure FSK channels */
   retCode = BAST_SetFskChannel(pChip->hAst, BAST_FskChannel_e1, BAST_FskChannel_e1);
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_SetFskChannel() error\n");
      goto done;
   }

   /* for debug only */
#if 0
   {
      BAST_AcqSettings acqSettings;
      acqSettings.acq_ctl = BAST_ACQSETTINGS_DEFAULT;
      acqSettings.mode = BAST_Mode_eDvb_scan;
      acqSettings.symbolRate = 28125000;
      acqSettings.carrierFreqOffset = 0;
      SATFE_MUTEX(retCode = BAST_TuneAcquire(pChip->hAstChannel[0], 1150000000UL, &acqSettings));
      if (retCode)
         printf("BAST_TuneAcquire() error 0x%X\n", retCode);
   }
#endif

   done:
   return retCode;
}


/******************************************************************************
 SATFE_97358_OnIdle()
******************************************************************************/
void SATFE_97358_OnIdle()
{
   /* TBD */
   return;
}


/******************************************************************************
 SATFE_94525_Command_help()
******************************************************************************/
bool SATFE_97358_Command_help(SATFE_Chip *pChip, int argc, char **argv)
{
   BSTD_UNUSED(pChip);
   BSTD_UNUSED(argc);
   BSTD_UNUSED(argv);

   printf("PLATFORM-SPECIFIC COMMANDS:\n");
   printf("   isl9492_status\n");
   printf("\n");
   return true;
}


/******************************************************************************
 SATFE_97358_Command_diseqc_reset()
******************************************************************************/
bool SATFE_97358_Command_diseqc_reset(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t sb;

   BSTD_UNUSED(pChip);
   BSTD_UNUSED(argc);
   BSTD_UNUSED(argv);

   /* initialize ISL9492 for SDS0, use BSC_M1 */
   sb = 0x20;
   retCode = BREG_I2C_WriteNoAddr(diags_hRegI2c[1], 0x08, &sb, 1);   /* tone controlled by EXTM */
   if (retCode)
      goto done;

   sb = 0x44;
   retCode = BREG_I2C_WriteNoAddr(diags_hRegI2c[1], 0x08, &sb, 1);   /* enable SVTOP pin */
   if (retCode)
      goto done;

   sb = 0x78;
   retCode = BREG_I2C_WriteNoAddr(diags_hRegI2c[1], 0x08, &sb, 1);   /* enable internal linear regulator */

   done:
   if (retCode)
      printf("BREG_I2C_Write() error 0x%X\n", retCode);
   return true;
}


/******************************************************************************
 SATFE_97358_Command_isl9492_status()
******************************************************************************/
bool SATFE_97358_Command_isl9492_status(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t sb[4];

   BSTD_UNUSED(pChip);
   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("isl9492_status", "isl9492_status", "Returns ISL9492 register values.", "none", true);
      return true;
   }

   printf("ISL9492 (I2C addr = 0x08):\n");
   retCode = BREG_I2C_Read(diags_hRegI2c[1], 0x08, 0x00, sb, 4); /* read SR1 register */
   if (retCode)
      goto done;
   printf("   SR1 = 0x%02X\n", sb[0]);
   printf("   SR2 = 0x%02X\n", sb[1]);
   printf("   SR3 = 0x%02X\n", sb[2]);
   printf("   SR4 = 0x%02X\n", sb[3]);

   done:
   if (retCode)
      printf("BREG_I2C_Read() error 0x%X\n", retCode);

   return true;
}


/******************************************************************************
 SATFE_97358_Command_reset_sds()
******************************************************************************/
bool SATFE_97358_Command_reset_sds(SATFE_Chip *p, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   SATFE_Chip *pChip;
   int i, j, count;

   BSTD_UNUSED(argv);
   BSTD_UNUSED(p)

   if ((argc < 1) || (argc > 2))
   {
      SATFE_PrintDescription1("reset_sds", "reset_sds <n>",
         "Reset the SDS and TUNER registers",
         "<n> = (optional, default=1) number of times to repeat reset", true);
      return true;
   }

   if (argc == 2)
      count = atoi(argv[1]);
   else
      count = 1;

   for (i = 0; i < count; i++)
   {
      printf("close AST portinginterface[%d]...\n", i+1);
      pChip = SATFE_GetChipByIndex(0);
      if (pChip->hAstChannel)
      {
         for (j = 0; j < pChip->chip.nDemods; j++)
         {
            if (pChip->hAstChannel[j])
            {
               BAST_AbortAcq(pChip->hAstChannel[j]);
               BAST_CloseChannel(pChip->hAstChannel[j]);
               pChip->hAstChannel[j] = NULL;
            }
         }
      }

      if (pChip->hAst)
      {
         BAST_Close(pChip->hAst);
         BKNI_Free(pChip->hAstChannel);
         pChip->hAst = NULL;
         pChip->hAstChannel = NULL;
      }
      BTMR_DestroyTimer(hTimer);
      pChip->bInit = false;

      /* reset sds and tuner */
      printf("reset SDS and TUNER...\n");
      BREG_Write32(pSatfeConfig->hReg, BCHP_SDS_GR_BRIDGE_SW_INIT_0, 5);
      BREG_Write32(pSatfeConfig->hReg, BCHP_SDS_GR_BRIDGE_SW_INIT_0, 0);

      /* open AST */
      printf("open AST portinginterface...\n");
      pSatfeConfig->bAstOpenExternal = false;
      SATFE_97358_Open(pChip, (void*)pSatfeConfig);
      SATFE_97358_OpenChannels(pChip, (void*)pSatfeConfig);

      /* re-initialize AST */
      SATFE_MUTEX(retCode = BAST_InitAp(pChip->hAst, pChip->pFwImage));
      if (retCode != BERR_SUCCESS)
         printf("BAST_InitAp() error 0x%X\n", retCode);
      else
         pChip->bInit = true;
      SATFE_97358_Configure(pChip, (void*)pSatfeConfig);
      printf("\n");

      BKNI_Sleep(1000);
   }

   return ((retCode == BERR_SUCCESS) ? true : false);
}


/******************************************************************************
 SATFE_97358_Command_special_acq()
******************************************************************************/
bool SATFE_97358_Command_special_acq(SATFE_Chip *pChip, int argc, char **argv)
{
   extern bool SATFE_GetFreqFromString(SATFE_Chip *pChip, char *str, uint32_t *pHz);

   BERR_Code retCode = BERR_SUCCESS;
   int i, j;
   uint32_t freq, symbolRate, beaconFreq, range, searchRange, freqStep, currFreq;
   BAST_AcqSettings acq_params;
   BAST_SignalDetectStatus signalDetectStatus;
   BKNI_EventHandle hLockChangeEvent;
   bool bLocked;
   uint8_t tuner_ctl, misc_ctl;

   if (argc != 2)
   {
      SATFE_PrintDescription1("special_acq", "special_acq <freq>",
         "Test acquisition",
         "<freq> = tuner freq in Hz", true);
      return true;
   }

   /* verify input tuner freq string */
   for (j = 0; j < (int)strlen(argv[1]); j++)
   {
      if (((argv[1][j] < '0') || (argv[1][j] > '9')) && (argv[1][j] != '.'))
      {
         printf("syntax error\n");
         return false;
      }
   }

   /* get tuner freq from the command line */
   if (SATFE_GetFreqFromString(pChip, argv[1], &freq) == false)
      return false;

   BAST_GetLockStateChangeEventHandle(pChip->hAstChannel[pChip->currChannel], &hLockChangeEvent);

   beaconFreq = 1707000000UL;
   symbolRate = pChip->acqSettings[pChip->currChannel].symbolRate;
   if (pChip->acqSettings[pChip->currChannel].acq_ctl & BAST_ACQSETTINGS_NYQUIST_20)
      range = symbolRate * 3 / 5;
   else
      range = symbolRate * 27 / 40;
   BAST_GetSearchRange(pChip->hAst, &searchRange);
   range += searchRange; /* 5MHz search range */
   if ((freq < (beaconFreq - range)) || (freq > (beaconFreq + range)))
   {
      /* do normal acquisition */
      /* make sure DFT is re-enabled for normal acquisition */
      SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_TUNER_CTL, &tuner_ctl, BAST_G3_CONFIG_LEN_TUNER_CTL));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_ReadConfig(BAST_G3_CONFIG_TUNER_CTL) error 0x%X\n", retCode);
         goto done;
      }
      tuner_ctl &= ~BAST_G3_CONFIG_TUNER_CTL_BYPASS_DFT_FREQ_EST;
      SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_TUNER_CTL, &tuner_ctl, BAST_G3_CONFIG_LEN_TUNER_CTL));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_WriteConfig(BAST_G3_CONFIG_TUNER_CTL) error 0x%X\n", retCode);
         goto done;
      }

      /* make sure final non-decimating filter is not overridden for normal acquisition */
      SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_ReadConfig(BAST_G3_CONFIG_MISC_CTL) error 0x%X\n", retCode);
         goto done;
      }
      misc_ctl &= ~BAST_G3_CONFIG_MISC_CTL_OVERRIDE_FNDF;
      SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_WriteConfig(BAST_G3_CONFIG_MISC_CTL) error 0x%X\n", retCode);
         goto done;
      }

      SATFE_MUTEX(retCode = BAST_TuneAcquire(pChip->hAstChannel[pChip->currChannel], freq, &(pChip->acqSettings[pChip->currChannel]))); /* do normal acquisition */
      if (retCode != BERR_SUCCESS)
         printf("BAST_TuneAcquire() error 0x%X\n", retCode);

      return true;
   }

   /* disable DFT in the acquisitions when tuning around the beaconFreq */
   SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_TUNER_CTL, &tuner_ctl, BAST_G3_CONFIG_LEN_TUNER_CTL));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_ReadConfig(BAST_G3_CONFIG_TUNER_CTL) error 0x%X\n", retCode);
      goto done;
   }
   tuner_ctl |= BAST_G3_CONFIG_TUNER_CTL_BYPASS_DFT_FREQ_EST;
   SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_TUNER_CTL, &tuner_ctl, BAST_G3_CONFIG_LEN_TUNER_CTL));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_WriteConfig(BAST_G3_CONFIG_TUNER_CTL) error 0x%X\n", retCode);
      goto done;
   }

   /* set final non-decimating filter as quarterband */
   SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_ReadConfig(BAST_G3_CONFIG_MISC_CTL) error 0x%X\n", retCode);
      goto done;
   }
   misc_ctl &= ~BAST_G3_CONFIG_MISC_CTL_FNDF_MASK;
   misc_ctl |= BAST_G3_CONFIG_MISC_CTL_OVERRIDE_FNDF;
   misc_ctl |= BAST_G3_CONFIG_MISC_CTL_FNDF_QUARTERBAND;
   SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_WriteConfig(BAST_G3_CONFIG_MISC_CTL) error 0x%X\n", retCode);
      goto done;
   }

   freqStep = symbolRate / 16;
   for (currFreq = (freq - searchRange); currFreq < (freq + searchRange); currFreq += freqStep)
   {
      printf("currFreq = %d Hz...\n", currFreq);

      /* attempt to lock the timing loop */
      acq_params.acq_ctl = BAST_ACQSETTINGS_DEFAULT | BAST_ACQSETTINGS_SIGNAL_DETECT_MODE;
      acq_params.symbolRate = symbolRate;
      acq_params.carrierFreqOffset = 0;
      acq_params.mode = BAST_Mode_eDvb_scan; /* BAST_AcqSettings.mode is ignored in signal detect mode, so we can put anything here */
      SATFE_MUTEX(retCode = BAST_TuneAcquire(pChip->hAstChannel[pChip->currChannel], currFreq, &acq_params));
      if (retCode != BERR_SUCCESS)
      {
         printf("SATFE_TuneAcquire() error 0x%X\n", retCode);
         goto done;
      }

      /* wait for signal detection done */
      for (i = 1000; i; i--)
      {
         SATFE_MUTEX(retCode = BAST_GetSignalDetectStatus(pChip->hAstChannel[pChip->currChannel], &signalDetectStatus));
         if (retCode != BERR_SUCCESS)
         {
            printf("BAST_GetSignalDetectStatus() error 0x%X\n", retCode);
            goto done;
         }

         /* sanity check */
         if (signalDetectStatus.bEnabled == false)
         {
            printf("BAST_GetSignalDetectStatus() error: bEnabled should not be false!\n");
            goto done;
         }

         if (signalDetectStatus.bDone)
            break;

         SATFE_OnIdle();
         BKNI_Sleep(1);
      }

      if (i)
      {
         if (signalDetectStatus.bTimingLoopLocked)
         {
            printf("achieved timing lock at freq=%d\n", currFreq);

            /* do an acquisition at this frequency (with DFT still disabled) */
            BKNI_WaitForEvent(hLockChangeEvent, 0); /* clear the lock event before acquisition */
            SATFE_MUTEX(retCode = BAST_TuneAcquire(pChip->hAstChannel[pChip->currChannel], currFreq, &(pChip->acqSettings[pChip->currChannel])));
            if (retCode != BERR_SUCCESS)
            {
               printf("BAST_TuneAcquire() error 0x%X\n", retCode);
               goto done;
            }

            /* check for user abort */
            wait_for_lock:
            retCode = BKNI_WaitForEvent(hLockChangeEvent, 2000);
            if (retCode == BERR_SUCCESS)
            {
               SATFE_MUTEX(retCode = BAST_GetLockStatus(pChip->hAstChannel[pChip->currChannel], &bLocked));
               if (retCode != BERR_SUCCESS)
               {
                  printf("BAST_GetLockStatus() error 0x%X\n", retCode);
                  goto done;
               }

               if (bLocked)
               {
                  printf("LOCKED!\n");
                  return true;
               }
            }
            else if (retCode != BERR_TIMEOUT)
               goto wait_for_lock;
         }
      }
   }

   done:
   return true;
}
