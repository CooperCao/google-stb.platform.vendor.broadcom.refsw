/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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
 
#ifndef BWFE_PRIV_H__
#define BWFE_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bwfe.h"

#if (BCHP_CHIP==4550)
   #include "bwfe_4550.h"
#elif (BCHP_CHIP==4528) 
   #include "bwfe_4528.h"
#else
   #error "unsupported BCHP_CHIP"
#endif


#define BWFE_CHK_RETCODE(x) \
   { if ((retCode = (x)) != BERR_SUCCESS) goto done; }


struct BWFE_P_ChannelHandle;


/******************************************************************************
Summary:
   This is the structure for the BWFE_Handle. 
******************************************************************************/
typedef struct BWFE_P_Handle
{
   BWFE_Settings                 settings;            /* user settings */
   BREG_Handle                   hRegister;           /* register handle */
   BINT_Handle                   hInterrupt;          /* interrupt handle */
   BTMR_Handle                   hTimer;              /* timer handle */
   BTMR_TimerHandle              tmrCalInl;           /* timer for INL calibration */
   uint32_t                      adcSampleFreqKhz;    /* ADC rx clock in kHz */
   uint8_t                       totalChannels;       /* total output channels */
   struct BWFE_P_ChannelHandle   **pChannels;         /* pointer to array of channel handles */
} BWFE_P_Handle;


/******************************************************************************
Summary:
   This is the structure for the BWFE_ChannelHandle. 
******************************************************************************/
typedef struct BWFE_P_ChannelHandle
{
   BWFE_ChannelSettings settings;         /* channel settings */
   BWFE_P_Handle        *pDevice;         /* pointer to device handle */
   uint32_t             accSaturateCount; /* accumulator saturation count */
   uint32_t             lutSaturateCount; /* lut saturation count */
   uint32_t             licCoeff[BWFE_NUM_LANES][BWFE_NUM_LIC_TAPS];    /* 2D array of LIC coeff (lanes x taps) */
   uint32_t             equCoeff[BWFE_NUM_SLICES][3][BWFE_NUM_EQ_TAPS]; /* 3D array of EQ coeff (slices x mdacs x taps)*/
   uint32_t             rfAgcInt;                  /* rf agc integrator */
   uint32_t             dgsClipCtl;                /* DGS clip control */
   uint32_t             dgsClipCount;              /* DGS clip count */
   uint32_t             licUpdateMask;             /* lane correction update mask */
   uint16_t             licRuntimeMs;              /* lane correction run-time in ms */
   uint16_t             equRuntimeMs;              /* digisum equalization run-time in ms */
   uint16_t             rfAgcThresh;               /* rf agc threshold */
   uint8_t              rfAgcK1;                   /* rf agc bandwidth */
   uint8_t              channel;                   /* channel number */
   uint8_t              totalLanes;                /* total lanes per channel */
   uint8_t              dgsClipLane;               /* DGS clip lane select */
   uint8_t              dgsMu1;                    /* digisum equalization mu1 */
   uint8_t              dgsMu2;                    /* digisum equalization mu2 */
   uint8_t              dgsMu3;                    /* digisum equalization mu3 */
   uint8_t              licMu;                     /* lic mu */
   uint8_t              licEnableRefLane;          /* enable ref lane during lane correction */
   bool                 bEnabled;                  /* ADC enabled */
} BWFE_P_ChannelHandle;


/* bwfe_<chip>.c */
BERR_Code BWFE_P_InitConfig(BWFE_ChannelHandle h);
BERR_Code BWFE_P_ReadRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t *val);
BERR_Code BWFE_P_WriteRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t val);
BERR_Code BWFE_P_ReadLaneRegister(BWFE_ChannelHandle h, uint8_t lane, uint32_t reg, uint32_t *val);
BERR_Code BWFE_P_WriteLaneRegister(BWFE_ChannelHandle h, uint8_t lane, uint32_t reg, uint32_t val);
BERR_Code BWFE_P_SetAdcSampleFreq(BWFE_Handle h, uint32_t freqKhz);
BERR_Code BWFE_P_GetAdcSampleFreq(BWFE_Handle h, uint32_t *freqKhz);
BERR_Code BWFE_P_ResetAdc(BWFE_ChannelHandle h);
BERR_Code BWFE_P_PowerDownRefLane(BWFE_ChannelHandle h);
BERR_Code BWFE_P_PowerUpRefLane(BWFE_ChannelHandle h);
BERR_Code BWFE_P_SetRefPhase(BWFE_ChannelHandle h, uint8_t lane);
BERR_Code BWFE_P_SetAdcCurrent(BWFE_ChannelHandle h);
BERR_Code BWFE_P_CalibrateDAC(BWFE_ChannelHandle h, uint8_t lane, int32_t target);

/* bwfe_priv.c */
void BWFE_P_ReadModifyWriteRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t and_mask, uint32_t or_mask);
void BWFE_P_OrRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t or_mask);
void BWFE_P_AndRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t and_mask);
void BWFE_P_ToggleBit(BWFE_ChannelHandle h, uint32_t reg, uint32_t mask);
void BWFE_P_ReadModifyWriteLaneReg(BWFE_ChannelHandle h, uint8_t lane, uint32_t reg, uint32_t and_mask, uint32_t or_mask);
void BWFE_P_OrLaneReg(BWFE_ChannelHandle h, uint8_t lane, uint32_t reg, uint32_t or_mask);
void BWFE_P_AndLaneReg(BWFE_ChannelHandle h, uint8_t lane, uint32_t reg, uint32_t and_mask);

/* bwfe_priv_adc.c */
BERR_Code BWFE_Adc_P_PowerUp(BWFE_ChannelHandle h);
BERR_Code BWFE_Adc_P_PowerDown(BWFE_ChannelHandle h);
BERR_Code BWFE_Adc_P_ReadAccumulator(BWFE_ChannelHandle h, uint8_t lane, uint8_t accLength, int32_t *acc);
BERR_Code BWFE_Adc_P_CalibrateINL(BWFE_ChannelHandle h);
BERR_Code BWFE_Adc_P_CalibrateINLv2(BWFE_ChannelHandle h);
BERR_Code BWFE_Adc_P_EqualizePipeline(BWFE_ChannelHandle h);
BERR_Code BWFE_Adc_P_ResetLut(BWFE_ChannelHandle h);
BERR_Code BWFE_Adc_P_ResetEqTaps(BWFE_ChannelHandle h);
BERR_Code BWFE_Adc_P_UpdateEqTaps(BWFE_ChannelHandle h);
BERR_Code BWFE_Adc_P_CancelDCOffset(BWFE_ChannelHandle h);

/* bwfe_priv_lic.c */
BERR_Code BWFE_Lic_P_PowerUp(BWFE_ChannelHandle h);
BERR_Code BWFE_Lic_P_PowerDown(BWFE_ChannelHandle h);
BERR_Code BWFE_Lic_P_RunLaneCorrection(BWFE_ChannelHandle h);
BERR_Code BWFE_Lic_P_RunLaneCorrectionAll(BWFE_Handle h);
BERR_Code BWFE_Lic_P_ResetTaps(BWFE_ChannelHandle h);
BERR_Code BWFE_Lic_P_UpdateTaps(BWFE_ChannelHandle h);

/* bwfe_priv_rffe.c */
BERR_Code BWFE_Rffe_P_PowerUp(BWFE_ChannelHandle h);
BERR_Code BWFE_Rffe_P_PowerDown(BWFE_ChannelHandle h);


/******************************************************************************
Summary:
   configuration parameter IDs
Description:
   These are the configuration parameters that can be accessed via 
   BWFE_ReadConfig() and BWFE_WriteConfig().
See Also:
   BWFE_ReadConfig(), BWFE_WriteConfig()
******************************************************************************/
#define BWFE_CONFIG_LIC_RUNTIME_MS              0x0000   /* lic lane correction run time */
#define BWFE_CONFIG_LEN_LIC_RUNTIME_MS          2
#define BWFE_CONFIG_RF_AGC_THRESH               0x0001   /* rf agc threshold */
#define BWFE_CONFIG_LEN_RF_AGC_THRESH           2
#define BWFE_CONFIG_RF_AGC_K1                   0x0002   /* rf agc bandwidth */
#define BWFE_CONFIG_LEN_RF_AGC_K1               1
#define BWFE_CONFIG_RF_AGC_INT                  0x0003   /* rf agc integrator */
#define BWFE_CONFIG_LEN_RF_AGC_INT              4
#define BWFE_CONFIG_DGS_CLIP_LANE_SELECT        0x0004   /* dgs clip lane select */
#define BWFE_CONFIG_LEN_DGS_CLIP_LANE_SELECT    1
#define BWFE_CONFIG_DGS_CLIP_CTL                0x0005   /* dgs clip counter control */
#define BWFE_CONFIG_LEN_DGS_CLIP_CTL            4
#define BWFE_CONFIG_DGS_CLIP_COUNT              0x0006   /* dgs clip counter */
#define BWFE_CONFIG_LEN_DGS_CLIP_COUNT          4
#define BWFE_CONFIG_DGS_EQU_RUNTIME_MS          0x0007   /* digisum equalization run time */
#define BWFE_CONFIG_LEN_DGS_EQU_RUNTIME_MS      2
#define BWFE_CONFIG_DGS_EQU_MU1                 0x0008   /* digisum equalization mu1 */
#define BWFE_CONFIG_LEN_DGS_EQU_MU1             1
#define BWFE_CONFIG_LIC_UPDATE_MASK             0x0009   /* lic update mask */
#define BWFE_CONFIG_LEN_LIC_UPDATE_MASK         4
#define BWFE_CONFIG_LIC_MU                      0x000A   /* lic mu */
#define BWFE_CONFIG_LEN_LIC_MU                  1
#define BWFE_CONFIG_DGS_EQU_MU2                 0x000B   /* digisum equalization mu2 */
#define BWFE_CONFIG_LEN_DGS_EQU_MU2             1
#define BWFE_CONFIG_DGS_EQU_MU3                 0x000C   /* digisum equalization mu3 */
#define BWFE_CONFIG_LEN_DGS_EQU_MU3             1
#define BWFE_CONFIG_LIC_ENABLE_REF_LANE         0x000D   /* enable ref lane during lic lane correction */
#define BWFE_CONFIG_LEN_LIC_ENABLE_REF_LANE     1


#ifdef __cplusplus
}
#endif

#endif /* BWFE_PRIV_H__ */
