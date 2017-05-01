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

#ifndef BWFE_PRIV_H__
#define BWFE_PRIV_H__

#include "bwfe.h"


#define BWFE_CHK_RETCODE(x) \
   { if ((retCode = (x)) != BERR_SUCCESS) goto done; }

#define BWFE_ABS(x) (((x) < 0) ? -(x):(x))

#define BWFE_P_WriteRegister BWFE_P_WriteRegister_isrsafe

/* fixed point defines */
#define BWFE_CORDIC_NUM_ITER  28          /* iterations needs to be <= 31 to avoid div by 2^31 int/uint issue */
#define BWFE_CORDIC_SCALE     1073741824  /* 2^30 */
#define BWFE_CORDIC_K         652032874   /* 0.60725293500888 * 2^30 */
#define BWFE_CORDIC_SHIFT     30
#define BWFE_DELAY_SCALE      262144      /* 2^18 is a very rough femtosecond ~2^18.49 */
#define BWFE_AGC_SCALE        16384       /* 2^14 */

#ifdef BWFE_FLOAT_MODE
   /* floating poiint defines */
   #define BWFE_ROUND(x) ((x >= 0) ? (int)(x + 0.5) : (int)(x - 0.5))
   #define BWFE_CORDIC_NUM_ITER_FP  40
   #define BWFE_CORDIC_K_FP         0.60725293500888
   #define BWFE_PI_FP               3.14159265358979323846
   #define BWFE_SINC_LBOUND_FP      1.0e-8   /* lower bound for sin(x)/x */
   #define BWFE_DELAY_SCALE_FP      262144.0 /* 2^18 is a very rough femtosecond ~2^18.49 */
   #define BWFE_DELAY_DELTA_FP      (int32_t)(2e-10 * BWFE_DEF_FS_ADC_KHZ * BWFE_DELAY_SCALE)   /* 0.2ps*Fadc  *DLY_SCL */
#endif


struct BWFE_P_ChannelHandle;


/******************************************************************************
Summary:
   This is the structure for the BWFE_Handle. 
******************************************************************************/
typedef struct BWFE_P_Handle
{
   BWFE_Settings                 settings;            /* user settings */
   uint8_t                       totalChannels;       /* total input channels */
   uint8_t                       totalRefChannels;    /* total reference channels */
   struct BWFE_P_ChannelHandle   **pChannels;         /* pointer to array of channel handles */
   struct BWFE_P_ChannelHandle   **pRefChannels;      /* pointer to array of ref channel handles */
   void                          *pImpl;              /* pointer to specific implementation */
} BWFE_P_Handle;


/******************************************************************************
Summary:
   This is the structure for the BWFE_ChannelHandle. 
******************************************************************************/
typedef struct BWFE_P_ChannelHandle
{
   BWFE_ChannelSettings    settings;      /* channel settings */
   BWFE_P_Handle           *pDevice;      /* pointer to device handle */
   uint8_t                 channel;       /* channel number */
   uint8_t                 totalSlices;   /* number of slices */
   bool                    bEnabled;      /* ADC enabled */
   bool                    bReference;    /* reference ADC indicator */
   void                    *pImpl;        /* pointer to specific implementation */
} BWFE_P_ChannelHandle;


/* private functions */
BERR_Code BWFE_P_InitConfig(BWFE_ChannelHandle h);
BERR_Code BWFE_P_ReadRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t *val);
BERR_Code BWFE_P_WriteRegister_isrsafe(BWFE_ChannelHandle h, uint32_t reg, uint32_t val);
BERR_Code BWFE_P_ReadSliceRegister(BWFE_ChannelHandle h, uint8_t slice, uint32_t reg, uint32_t *val);
BERR_Code BWFE_P_WriteSliceRegister(BWFE_ChannelHandle h, uint8_t slice, uint32_t reg, uint32_t val);
BERR_Code BWFE_P_SetAdcSampleFreq(BWFE_ChannelHandle h, uint32_t freqKhz);
BERR_Code BWFE_P_GetAdcSampleFreq(BWFE_ChannelHandle h, uint32_t *freqKhz);
BERR_Code BWFE_P_SetDpmPilotFreq(BWFE_ChannelHandle h, uint32_t freqKhz);
BERR_Code BWFE_P_GetDpmPilotFreq(BWFE_ChannelHandle h, uint32_t *freqKhz);
BERR_Code BWFE_P_EnableDpmPilot(BWFE_ChannelHandle h);
BERR_Code BWFE_P_DisableDpmPilot(BWFE_ChannelHandle h);
BERR_Code BWFE_P_InitAdc(BWFE_ChannelHandle h);
BERR_Code BWFE_P_ShutdownAdc(BWFE_ChannelHandle h);
bool BWFE_P_IsAdcOn(BWFE_ChannelHandle h);
#ifndef BWFE_EXCLUDE_SPECTRUM_ANALYZER
BERR_Code BWFE_P_CalcCorrPower(BWFE_ChannelHandle h, int32_t *corrI, int32_t *corrQ, uint32_t *corrPowerDb);
#endif


/* bwfe_priv.c */
void BWFE_P_ReadModifyWriteRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t and_mask, uint32_t or_mask);
void BWFE_P_OrRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t or_mask);
void BWFE_P_AndRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t and_mask);
void BWFE_P_ToggleBit(BWFE_ChannelHandle h, uint32_t reg, uint32_t mask);
void BWFE_P_ReadModifyWriteSliceReg(BWFE_ChannelHandle h, uint8_t slice, uint32_t reg, uint32_t and_mask, uint32_t or_mask);
void BWFE_P_OrSliceReg(BWFE_ChannelHandle h, uint8_t slice, uint32_t reg, uint32_t or_mask);
void BWFE_P_AndSliceReg(BWFE_ChannelHandle h, uint8_t slice, uint32_t reg, uint32_t and_mask);
void BWFE_P_ToggleSliceBit(BWFE_ChannelHandle h, uint8_t slice, uint32_t reg, uint32_t mask);
#ifdef BWFE_FLOAT_MODE
BERR_Code BWFE_P_CordicRotationMode_FP(double *vec, double theta, bool unitMode);
BERR_Code BWFE_P_CordicVectorMode_FP(double *vec, double *theta);
#endif
BERR_Code BWFE_P_CordicRotationMode(int32_t *vec, int32_t theta, bool unitMode);
BERR_Code BWFE_P_CordicVectorMode(int32_t *vec, int32_t *theta);

uint32_t BWFE_P_GCF(uint32_t m, uint32_t n);
uint8_t BWFE_P_GetMsb(uint32_t i);
void BWFE_P_SumThenMultiply(int32_t x, int32_t y, int32_t m, uint32_t *result_hi, uint32_t *result_lo);


#endif /* BWFE_PRIV_H__ */