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

/* BWFE (Broadcom Wide-band Frontend) porting interface (PI) */

#ifndef BWFE_H__
#define BWFE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bchp.h"
#include "bkni.h"
#include "bint.h"
#include "bmth.h"
#include "berr_ids.h"
#include "bfec.h"


#define BWFE_API_VERSION 5


/******************************************************************************
Summary:
   BWFE error codes
Description:
   These are API error codes specific to the BWFE PI.
See Also:
   None.
******************************************************************************/
#define BWFE_ERR_ACC_SATURATED         BERR_MAKE_CODE(BERR_WFE_ID, 0)   /* BERR_Code indicating accumulator was saturated. */
#define BWFE_ERR_POWERED_DOWN          BERR_MAKE_CODE(BERR_WFE_ID, 1)   /* BERR_Code indicating a required core is powered down */
#define BWFE_ERR_POWERED_UP            BERR_MAKE_CODE(BERR_WFE_ID, 2)   /* BERR_Code indicating core is already powered up */
#define BWFE_ERR_CORR_TIMEOUT          BERR_MAKE_CODE(BERR_WFE_ID, 3)   /* BERR_Code indicating correlator has timed out */
#define BWFE_ERR_CORR_BUSY             BERR_MAKE_CODE(BERR_WFE_ID, 4)   /* BERR_Code indicating correlator is busy */
#define BWFE_ERR_EQU_BUSY              BERR_MAKE_CODE(BERR_WFE_ID, 5)   /* BERR_Code indicating equalizer is busy */
#define BWFE_ERR_DAC_BIN_SEARCH_FAIL   BERR_MAKE_CODE(BERR_WFE_ID, 6)   /* BERR_Code indicating DAC binary search failed */
#define BWFE_ERR_LIC_BUSY              BERR_MAKE_CODE(BERR_WFE_ID, 7)   /* BERR_Code indicating lic is busy */
#define BWFE_ERR_CAL_BUSY              BERR_MAKE_CODE(BERR_WFE_ID, 8)   /* BERR_Code indicating calibration in progress */


/******************************************************************************
Summary:
   Handle for the BWFE device
Description:
   This is an opaque handle for the BWFE device
See Also:
   BWFE_Open()
******************************************************************************/
typedef struct BWFE_P_Handle *BWFE_Handle;


/******************************************************************************
Summary:
   Handle for an BWFE device channel
Description:
   This is an opaque handle for the BWFE device channel
See Also:
   BWFE_OpenChannel()
******************************************************************************/
typedef struct BWFE_P_ChannelHandle *BWFE_ChannelHandle;


/******************************************************************************
Summary:
   Structure containing channel availability info
Description:
   This structure contains BWFE channel availability info
See Also:
   BWFE_GetChannelStatus()
******************************************************************************/
typedef struct BWFE_ChannelInfo
{
   uint8_t     maxChannels;         /* maximum number of channels supported */
   uint8_t     numChannels;         /* number of channels available for use */
   uint32_t    availChannelsMask;   /* bit mask specifying which channels are available for use */
} BWFE_ChannelInfo;


/******************************************************************************
Summary:
   Structure containing channel status
Description:
   This structure contains BWFE channel status
See Also:
   BWFE_GetChannelStatus()
******************************************************************************/
typedef struct BWFE_ChannelStatus
{
   int32_t     freqOffset;          /* freq offset of entire range of ADC */
   uint32_t    freqLower;           /* lower bound of freq range in Hz */
   uint32_t    freqUpper;           /* upper bound of freq range in Hz */
   uint32_t    accSaturateCount;    /* accumulator saturation count */
   uint32_t    lutSaturateCount;    /* lut saturation count */
   bool        bEnabled;            /* ADC enabled or disabled */
   bool        bAgcFrozen;          /* RF AGC frozen indicator */
} BWFE_ChannelStatus;


/******************************************************************************
Summary:
   Structure containing spectrum analyzer parameters
Description:
   This structure contains spectrum analyzer parameters
See Also:
   BWFE_ScanSpectrum()
******************************************************************************/
#define BWFE_SA_MAX_SAMPLES   4096
typedef struct BWFE_SpecAnalyzerParams
{
   uint32_t freqStartHz;   /* start freq in Hz */
   uint32_t freqStopHz;    /* stop freq in Hz */
   uint16_t numSamples;    /* total number of samples to be returned */
   uint8_t  numSweeps;     /* total number of sweeps, cumulative avg if > 1 */
} BWFE_SpecAnalyzerParams;


/******************************************************************************
Summary:
   Structure for API function table
Description:
   This structure contains pointers to all public BWFE functions.
See Also:
   BWFE_Settings
******************************************************************************/
struct BWFE_Settings;
struct BWFE_ChannelSettings;
typedef struct BWFE_ApiFunctTable
{
   BERR_Code (*Open)(BWFE_Handle*, BCHP_Handle, void*, BINT_Handle, const struct BWFE_Settings *pDefSettings);
   BERR_Code (*Close)(BWFE_Handle);
   BERR_Code (*GetTotalChannels)(BWFE_Handle, BWFE_ChannelInfo*);
   BERR_Code (*GetTotalRefChannels)(BWFE_Handle, uint8_t*);
   BERR_Code (*GetChannelDefaultSettings)(BWFE_Handle, uint8_t, struct BWFE_ChannelSettings *pChnDefSettings);
   BERR_Code (*OpenChannel)(BWFE_Handle, BWFE_ChannelHandle*, uint8_t, const struct BWFE_ChannelSettings *pSettings);
   BERR_Code (*CloseChannel)(BWFE_ChannelHandle);
   BERR_Code (*Reset)(BWFE_Handle);
   BERR_Code (*GetVersion)(BWFE_Handle, BFEC_VersionInfo*);
   BERR_Code (*IsInputEnabled)(BWFE_ChannelHandle, bool*);
   BERR_Code (*EnableInput)(BWFE_ChannelHandle);
   BERR_Code (*DisableInput)(BWFE_ChannelHandle);
   BERR_Code (*GetChannelStatus)(BWFE_ChannelHandle, BWFE_ChannelStatus*);
   BERR_Code (*GetLicTaps)(BWFE_ChannelHandle, uint32_t**); /* debug */
   BERR_Code (*GetEqTaps)(BWFE_ChannelHandle, uint32_t****); /* debug */
   BERR_Code (*FreezeRfAgc)(BWFE_ChannelHandle);
   BERR_Code (*UnfreezeRfAgc)(BWFE_ChannelHandle);
   BERR_Code (*RunLaneCorrection)(BWFE_ChannelHandle);
   BERR_Code (*CalibrateINL)(BWFE_ChannelHandle);
   BERR_Code (*EqualizePipeline)(BWFE_ChannelHandle);
   BERR_Code (*SetAdcSampleFreq)(BWFE_ChannelHandle, uint32_t); /* debug */
   BERR_Code (*GetAdcSampleFreq)(BWFE_ChannelHandle, uint32_t*); /* debug */
   BERR_Code (*ReadConfig)(BWFE_ChannelHandle, uint16_t, uint8_t*, uint8_t);
   BERR_Code (*WriteConfig)(BWFE_ChannelHandle, uint16_t, uint8_t*, uint8_t);
   BERR_Code (*ResetDgsLut)(BWFE_ChannelHandle);
   BERR_Code (*ResetEqTaps)(BWFE_ChannelHandle);
   BERR_Code (*ResetLicTaps)(BWFE_ChannelHandle);
   BERR_Code (*CancelDCOffset)(BWFE_ChannelHandle);
   BERR_Code (*GetWfeReadyEventHandle)(BWFE_ChannelHandle, BKNI_EventHandle*);
   BERR_Code (*CalibrateAnalogDelay)(BWFE_ChannelHandle);
   BERR_Code (*GetAnalogDelay)(BWFE_ChannelHandle, uint32_t*);
   BERR_Code (*GetSaDoneEventHandle)(BWFE_Handle, BKNI_EventHandle*);
   BERR_Code (*ScanSpectrum)(BWFE_ChannelHandle, BWFE_SpecAnalyzerParams*);
   BERR_Code (*GetSaSamples)(BWFE_Handle, uint32_t*);
   BERR_Code (*MirrorEquRefTaps)(BWFE_ChannelHandle, BWFE_ChannelHandle);
   BERR_Code (*CalibrateAdcPhase)(BWFE_ChannelHandle);
   BERR_Code (*CompensateDelay)(BWFE_ChannelHandle, uint32_t*);
} BWFE_ApiFunctTable;


/******************************************************************************
Summary:
   BWFE SVT sigma process enumeration.
Desciption:
   This is an enumeration to indicate the SVT sigma variation of the chip.
See Also:
   None.
******************************************************************************/
typedef enum BWFE_ChipSvtSigma
{
   BWFE_CHIP_SVT_SIGMA_LT_ONE_SLOW = 0,
   BWFE_CHIP_SVT_SIGMA_ONE_SLOW_TO_NOM,
   BWFE_CHIP_SVT_SIGMA_NOM_TO_ONE_FAST,
   BWFE_CHIP_SVT_SIGMA_GT_ONE_FAST
} BWFE_ChipSvtSigma;


/******************************************************************************
Summary:
   Structure containing global BWFE settings
Description:
   This structure contains global settings for the BWFE device
See Also:
   BWFE_Open()
******************************************************************************/
typedef struct BWFE_Settings
{
   BWFE_ApiFunctTable api;    /* API function table for per-chip implementation */
   BWFE_ChipSvtSigma sigma;   /* sigma variation of current chip */
   uint32_t xtalFreqKhz;      /* crystal frequency in kHz */
} BWFE_Settings;


/******************************************************************************
Summary:
   Structure containing BWFE device channel settings
Description:
   This structure contains BWFE device channel settings
See Also:
   BWFE_OpenChannel(), BWFE_GetChannelDefaultSettings()
******************************************************************************/
typedef struct BWFE_ChannelSettings
{
   bool      bReference;   /* specify reference adc channel */
} BWFE_ChannelSettings;


/******************************************************************************
Summary:
   Gets default settings for BWFE
Description:
   This function gets the default settings for a BWFE device
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_GetDefaultSettings(BWFE_Settings *pDefSettings);


/******************************************************************************
Summary:
   Open the BWFE API
Description:
   This function initializes a BWFE_Handle
Returns:
   BERR_Code: BERR_SUCCESS = the returned BWFE_Handle is valid
******************************************************************************/
BERR_Code BWFE_Open(
   BWFE_Handle *h,      /* [out] BWFE handle */
   BCHP_Handle hChip,   /* [in] chip handle */
   void *pReg,          /* [in] pointer to register or i2c handle */
   BINT_Handle hInt,    /* [in] interrupt handle */
   const BWFE_Settings *pDefSettings   /* [in] default settings */
);


/******************************************************************************
Summary:
   Close the BWFE API
Description:
   This function releases all the resources allocated by BWFE API
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_Close(BWFE_Handle h);


/******************************************************************************
Summary:
   Gets the total number of logical sub-devices for the BWFE device
Description:
   This function gets the number of channels supported by the BWFE device
   e.g. for BCM4550, totalChannels returns the value 14
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_GetTotalChannels(
   BWFE_Handle       h,       /* [in] BWFE handle */
   BWFE_ChannelInfo  *pInfo   /* [out] channel availability info */
);


/******************************************************************************
Summary:
   Gets the total number of reference sub-devices for the BWFE device
Description:
   This function gets the total number of reference channels supported by the BWFE device
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_GetTotalRefChannels(
   BWFE_Handle  h,                  /* [in] BWFE handle */
   uint8_t      *totalRefChannels   /* [out] number of channels supported */
);


/******************************************************************************
Summary:
   Gets default channel settings
Description:
   This function gets the default settings for a BWFE device channel
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_GetChannelDefaultSettings(
   BWFE_Handle   h,                      /* [in] BWFE handle */
   uint8_t       chanNum,                /* [in] channel number */
   BWFE_ChannelSettings *pChnDefSettings /* [out] default channel settings */
);


/******************************************************************************
Summary:
   Initializes a BWFE device channel.
Description:
   This function initializes a BWFE device channel and returns a BWFE_ChannelHandle
Returns:
   BERR_Code : BERR_SUCCESS = the returned BWFE_ChannelHandle is valid
******************************************************************************/
BERR_Code BWFE_OpenChannel(
   BWFE_Handle                h,               /* [in] BWFE handle */
   BWFE_ChannelHandle         *pChannelHandle, /* [out] BWFE channel handle */
   uint8_t                    chanNum,         /* [in] channel number */
   const BWFE_ChannelSettings *pSettings       /* [in] channel settings */
);


/******************************************************************************
Summary:
   Closes the BWFE device channel.
Description:
   This function frees the channel handle and any resources contained in the channel handle
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_CloseChannel(BWFE_ChannelHandle h);


/******************************************************************************
Summary:
   Reset the BWFE device
Description:
   (Re)initializes the firmware to its power up init state
Returns:
   BERR_Code : BERR_SUCCESS = firmware is running and successfully initialized
******************************************************************************/
BERR_Code BWFE_Reset(BWFE_Handle h);


/******************************************************************************
Summary:
   Gets version for BWFE PI
Description:
   This function returns the version of BWFE porting interface
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_GetVersion(BWFE_Handle h, BFEC_VersionInfo *pVersion);


/******************************************************************************
Summary:
   Check the current power state of a specified ADC
Description:
   The function will return ADC power state as a parameter
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_IsInputEnabled(BWFE_ChannelHandle h, bool *bEnabled);


/******************************************************************************
Summary:
   Turn on the specified ADC input
Description:
   The function will enable the given ADC input
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_EnableInput(BWFE_ChannelHandle h);


/******************************************************************************
Summary:
   Turn off the specified ADC input
Description:
   The function will disable the given ADC input
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_DisableInput(BWFE_ChannelHandle h);


/******************************************************************************
Summary:
   Returns channel status
Description:
   This function returns status information in the BWFE_ChannelStatus structure
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_GetChannelStatus(
   BWFE_ChannelHandle h,        /* [in] BWFE channel handle */
   BWFE_ChannelStatus *pStatus  /* [out] pointer to the status structure */
);


/******************************************************************************
Summary:
   Returns LIC taps
Description:
   This function returns LIC coefficients per channel
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_GetLicTaps(BWFE_ChannelHandle h, uint32_t **pLicCoeff);


/******************************************************************************
Summary:
   Returns equalizer taps
Description:
   This function returns equalizer coefficients per channel
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_GetEqTaps(BWFE_ChannelHandle h, uint32_t ****pEqCoeff);


/******************************************************************************
Summary:
   Freeze RF AGC
Description:
   The function will freeze the RFAGC integrator on the specified input
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_FreezeRfAgc(BWFE_ChannelHandle h);


/******************************************************************************
Summary:
   Unfreeze RF AGC
Description:
   The function will unfreeze the RFAGC integrator on the specified input
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_UnfreezeRfAgc(BWFE_ChannelHandle h);


/******************************************************************************
Summary:
   Runs Lane Imbalance Canceller for all lanes
Description:
   This function will phase-align the specified ADC and should be scheduled periodically
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_RunLaneCorrection(BWFE_ChannelHandle h);


/******************************************************************************
Summary:
   Run Integrated Non-Linearities calibration algorithm
Description:
   This function will run the INL calibration for the specified input
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_CalibrateINL(BWFE_ChannelHandle h);


/******************************************************************************
Summary:
   Runs ADC pipeline equalization algorithm
Description:
   This function will equalize pipeline for the specified ADC
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_EqualizePipeline(BWFE_ChannelHandle h);


/******************************************************************************
Summary:
   Set ADC sample frequency
Description:
   The function will configure the PLL dividers for a specified ADCs sample clock
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_SetAdcSampleFreq(BWFE_ChannelHandle h, uint32_t freqKhz);


/******************************************************************************
Summary:
   Get ADC sample frequency
Description:
   The function will calculate the ADC sample clock based on the current PLL dividers
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_GetAdcSampleFreq(BWFE_ChannelHandle h, uint32_t *freqKhz);


/******************************************************************************
Summary:
   Read configuration parameter
Description:
   The function will read the value of the specified config parameter
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_ReadConfig(
   BWFE_ChannelHandle h,   /* [in] BWFE channel handle */
   uint16_t id,            /* [in] configuration parameter ID */
   uint8_t *p,             /* [out] buffer to hold the read data */
   uint8_t len             /* [in] number of bytes to read */
);


/******************************************************************************
Summary:
   Write configuration parameter
Description:
   The function will modify the value of the specified config parameter
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_WriteConfig(
   BWFE_ChannelHandle h,   /* [in] BWFE channel handle */
   uint16_t id,            /* [in] configuration parameter ID */
   uint8_t *p,             /* [in] data to write */
   uint8_t len             /* [in] number of bytes to write */
);


/******************************************************************************
Summary:
   Reset Digisum Lookup Table
Description:
   The function will clear the DGS lut for specified ADC
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_ResetDgsLut(BWFE_ChannelHandle h);


/******************************************************************************
Summary:
   Reset EQ taps
Description:
   The function will clear the EQ taps for specified ADC
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_ResetEqTaps(BWFE_ChannelHandle h);


/******************************************************************************
Summary:
   Reset LIC taps
Description:
   The function will clear the LIC taps for specified ADC
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_ResetLicTaps(BWFE_ChannelHandle h);


/******************************************************************************
Summary:
   Cancel DC offset
Description:
   The function will cancel DC offset specified ADC by using the DGS LUT
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_CancelDCOffset(BWFE_ChannelHandle h);


/******************************************************************************
Summary:
   Returns wfe ready event handle
Description:
   This function returns the wfe ready event handle
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_GetWfeReadyEventHandle(BWFE_ChannelHandle h, BKNI_EventHandle *hWfeReady);


/******************************************************************************
Summary:
   Calibrate analog delays
Description:
   This function calculates the analog delays using the correlator
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_CalibrateAnalogDelay(BWFE_ChannelHandle h);


/******************************************************************************
Summary:
   Returns analog delays
Description:
   This function returns correlator delays per channel
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_GetAnalogDelay(BWFE_ChannelHandle h, uint32_t *pCorrDelays);


/******************************************************************************
Summary:
   Compensate for analog delays
Description:
   This function adjusts the correlator delays per channel
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_CompensateDelay(BWFE_ChannelHandle h, uint32_t *pSliceAdjust);


/******************************************************************************
Summary:
   Returns spectrum analyzer done event handle
Description:
   This function returns the spectrum analyzer done event handle
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_GetSaDoneEventHandle(BWFE_Handle h, BKNI_EventHandle *hSaDoneEvent);


/******************************************************************************
Summary:
   Spectrum analyzer feature
Description:
   This function scans the spectrum with the specified parameters
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_ScanSpectrum(BWFE_ChannelHandle h, BWFE_SpecAnalyzerParams *pParams);


/******************************************************************************
Summary:
   Returns spectrum analyzer samples
Description:
   This function returns spectrum analyzer samples from most recent scan.  The data is in
   dB scaled up by a factor of 2560.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_GetSaSamples(BWFE_Handle h, uint32_t *pSaSamples);


/******************************************************************************
Summary:
   Copy reference taps to specified input
Description:
   This function copies equ taps from specified ref channel to given input channel
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_MirrorEquRefTaps(BWFE_ChannelHandle h, BWFE_ChannelHandle hRef);


/******************************************************************************
Summary:
   Calibrate ADC phases for specified input
Description:
   This function calibrates the ADC phase settings for an input channel
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BWFE_CalibrateAdcPhase(BWFE_ChannelHandle h);


#ifdef __cplusplus
}
#endif

#endif /* BWFE_H__ */
