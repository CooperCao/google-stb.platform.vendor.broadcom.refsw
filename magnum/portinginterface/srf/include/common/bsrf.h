/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/

/* BSRF (Broadcom Satellite Radio Frontend) porting interface (PI) */

#ifndef BSRF_H__
#define BSRF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bchp.h"
#include "bkni.h"
#include "bint.h"
#include "bmth.h"
#include "berr_ids.h"
#include "bfec.h"

#define BSRF_API_VERSION 1


/******************************************************************************
Summary:
   BSRF error codes
Description:
   These are API error codes specific to the BSRF PI.
See Also:
   None.
******************************************************************************/
#define BSRF_ERR_POWERED_DOWN    BERR_MAKE_CODE(BERR_SCS_ID, 0)   /* BERR_Code indicating a required core is powered down */


/******************************************************************************
Summary:
   Handle for the BSRF device
Description:
   This is an opaque handle for the BSRF device.
See Also:
   BSRF_Open()
******************************************************************************/
typedef struct BSRF_P_Handle *BSRF_Handle;


/******************************************************************************
Summary:
   Handle for an BSRF device channel
Description:
   This is an opaque handle for the BSRF device channel.
See Also:
   BSRF_OpenChannel()
******************************************************************************/
typedef struct BSRF_P_ChannelHandle *BSRF_ChannelHandle;


/******************************************************************************
Summary:
   Structure containing BSRF version info
Description:
   This structure contains BSRF chip and version information.
See Also:
   BSRF_GetVersion()
******************************************************************************/
typedef struct BSRF_Version
{
   uint16_t productID;
   uint16_t chipID;
   uint16_t chipRev;
   uint16_t internalID;
   uint16_t verMajor;
   uint16_t verMinor;
} BSRF_Version;


/******************************************************************************
Summary:
   Structure containing basic AGC parameters
Description:
   This structure contains basic AGC parameters used in the different AGC modes.
See Also:
   BSRF_RfAgcSettings()
******************************************************************************/
typedef struct
{
   uint32_t timeNs;
   int32_t threshold;   /* tc16.16 */
   int32_t step;        /* tc6.11 */
} BSRF_AgcSettings;


/******************************************************************************
Summary:
   Structure containing RFAGC parameters
Description:
   This structure contains RFAGC parameters for the different AGC modes.
See Also:
   BSRF_SetRfAgcSettings()
******************************************************************************/
typedef struct
{
   BSRF_AgcSettings  attackSettings;
   BSRF_AgcSettings  decaySettings;
   BSRF_AgcSettings  fastDecaySettings;
   BSRF_AgcSettings  clipWindowSettings;
   uint32_t          powerMeasureBw;
   uint8_t           agcSlewRate;
} BSRF_RfAgcSettings;


/******************************************************************************
Summary:
   Structure containing antenna status
Description:
   This structure contains antenna status information.
See Also:
   BSRF_GetAntennaStatus()
******************************************************************************/
typedef struct
{
   uint8_t modeAoc;
   uint8_t modeAd;
   bool bOverCurrent;
   bool bDetected;
} BSRF_AntennaStatus;


/******************************************************************************
Summary:
   Structure containing tuner status
Description:
   This structure contains tuner status information.
See Also:
   BSRF_GetAntennaStatus()
******************************************************************************/
typedef struct
{
   uint32_t tunerFreqHz;
   bool bPllLock;
} BSRF_TunerStatus;


/******************************************************************************
Summary:
   Enumeration to select a set of outputs
Description:
   This enumeration specifies the set of outputs to configure.
See Also:
   BSRF_ConfigOutput()
******************************************************************************/
typedef enum BSRF_OutputSelect
{
   BSRF_OutputSelect_eGpio = 0,
   BSRF_OutputSelect_eTestport,
   BSRF_OutputSelect_eClock,
   BSRF_OutputSelect_eDataI,
   BSRF_OutputSelect_eDataQ
} BSRF_OutputSelect;


/******************************************************************************
Summary:
   Enumeration to select a testport
Description:
   This enumeration specifies the testport to enable.
See Also:
   BSRF_ConfigOutput()
******************************************************************************/
typedef enum BSRF_TestportSelect
{
   BSRF_TestportSelect_eLoPll = 0,
   BSRF_TestportSelect_eIfInput,
   BSRF_TestportSelect_eIfOutput
} BSRF_TestportSelect;


/******************************************************************************
Summary:
   Structure containing IQ imbalance equalizer parameters
Description:
   This structure contains IQ imbalance equalizer parameters.
See Also:
   BSRF_SetIqEqSettings()
******************************************************************************/
typedef struct
{
   bool              bBypass;    /* bypass phase and amplitude correction */
   bool              bFreeze;    /* freeze phase and amplitude correction */
   uint8_t           bandwidth;  /* phase and amplitude bandwidth */
   uint8_t           delay;      /* IQ imbalance equ delay in main path */
} BSRF_IqEqSettings;


/******************************************************************************
Summary:
   Structure for API function table
Description:
   This structure contains pointers to all public BSRF functions.
See Also:
   BSRF_Settings
******************************************************************************/
struct BSRF_Settings;
struct BSRF_ChannelSettings;
typedef struct BSRF_ApiFunctTable
{
   BERR_Code (*Open)(BSRF_Handle*, BCHP_Handle, void*, BINT_Handle, const struct BSRF_Settings *pDefSettings);
   BERR_Code (*Close)(BSRF_Handle);
   BERR_Code (*GetTotalChannels)(BSRF_Handle, uint8_t*);
   BERR_Code (*GetChannelDefaultSettings)(BSRF_Handle, uint8_t, struct BSRF_ChannelSettings *pChnDefSettings);
   BERR_Code (*OpenChannel)(BSRF_Handle, BSRF_ChannelHandle*, uint8_t, const struct BSRF_ChannelSettings *pSettings);
   BERR_Code (*CloseChannel)(BSRF_ChannelHandle);
   BERR_Code (*Reset)(BSRF_Handle);
   BERR_Code (*GetVersion)(BSRF_Handle, BFEC_VersionInfo*);
   BERR_Code (*PowerUp)(BSRF_ChannelHandle);
   BERR_Code (*PowerDown)(BSRF_ChannelHandle);
   BERR_Code (*FreezeRfAgc)(BSRF_ChannelHandle);
   BERR_Code (*UnfreezeRfAgc)(BSRF_ChannelHandle);
   BERR_Code (*WriteRfAgc)(BSRF_ChannelHandle, uint32_t);
   BERR_Code (*ReadRfAgc)(BSRF_ChannelHandle, uint32_t*);
   BERR_Code (*WriteRfGain)(BSRF_ChannelHandle, uint8_t);
   BERR_Code (*ReadRfGain)(BSRF_ChannelHandle, uint8_t*);
   BERR_Code (*GetInputPower)(BSRF_ChannelHandle, int32_t*);
   BERR_Code (*SetRfAgcSettings)(BSRF_ChannelHandle, BSRF_RfAgcSettings);
   BERR_Code (*GetRfAgcSettings)(BSRF_ChannelHandle, BSRF_RfAgcSettings*);
   BERR_Code (*EnableFastDecayMode)(BSRF_ChannelHandle, bool);
   BERR_Code (*SetFastDecayGainThreshold)(BSRF_ChannelHandle, int8_t);
   BERR_Code (*GetFastDecayGainThreshold)(BSRF_ChannelHandle, int8_t*);
   BERR_Code (*SetAntennaOverThreshold)(BSRF_ChannelHandle, uint8_t);
   BERR_Code (*GetAntennaOverThreshold)(BSRF_ChannelHandle, uint8_t*);
   BERR_Code (*SetAntennaDetectThreshold)(BSRF_ChannelHandle, uint8_t);
   BERR_Code (*GetAntennaDetectThreshold)(BSRF_ChannelHandle, uint8_t*);
   BERR_Code (*GetAntennaStatus)(BSRF_ChannelHandle, BSRF_AntennaStatus*);
   BERR_Code (*PowerUpAntennaSense)(BSRF_ChannelHandle);
   BERR_Code (*PowerDownAntennaSense)(BSRF_ChannelHandle);
   BERR_Code (*Tune)(BSRF_ChannelHandle, uint32_t);
   BERR_Code (*GetTunerStatus)(BSRF_ChannelHandle, BSRF_TunerStatus*);
   BERR_Code (*ResetClipCount)(BSRF_ChannelHandle);
   BERR_Code (*GetClipCount)(BSRF_ChannelHandle, uint32_t*);
   BERR_Code (*ConfigTestMode)(BSRF_ChannelHandle, BSRF_TestportSelect, bool);
   BERR_Code (*ConfigOutput)(BSRF_Handle, BSRF_OutputSelect, bool, uint8_t);
   BERR_Code (*ConfigTestDac)(BSRF_Handle, int32_t, int16_t*);
   BERR_Code (*EnableTestDac)(BSRF_Handle);
   BERR_Code (*DisableTestDac)(BSRF_Handle);
   BERR_Code (*EnableTestDacTone)(BSRF_Handle, bool, uint16_t);
   BERR_Code (*RunDataCapture)(BSRF_Handle);
   BERR_Code (*DeleteAgcLutCodes)(BSRF_Handle, uint32_t*, uint32_t);
   BERR_Code (*ConfigOutputClockPhase)(BSRF_Handle, uint8_t, bool);
   BERR_Code (*SetIqEqCoeff)(BSRF_ChannelHandle, int16_t*, int16_t*);
   BERR_Code (*SetIqEqSettings)(BSRF_ChannelHandle, BSRF_IqEqSettings);
   BERR_Code (*BoostMixPllCurrent)(BSRF_ChannelHandle, bool);
} BSRF_ApiFunctTable;


/******************************************************************************
Summary:
   Structure containing global BSRF settings
Description:
   This structure contains global settings for the BSRF device.
See Also:
   BSRF_Open()
******************************************************************************/
typedef struct BSRF_Settings
{
   BSRF_ApiFunctTable api;    /* API function table for per-chip implementation */
} BSRF_Settings;


/******************************************************************************
Summary:
   Structure containing BSRF device channel settings
Description:
   This structure contains BSRF device channel settings.
See Also:
   BSRF_OpenChannel(), BSRF_GetChannelDefaultSettings()
******************************************************************************/
typedef struct BSRF_ChannelSettings
{
   uint32_t    reserved;    /* TBD */
} BSRF_ChannelSettings;


/******************************************************************************
Summary:
   Gets default settings for BSRF
Description:
   This function gets the default settings for a BSRF device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_GetDefaultSettings(BSRF_Settings *pDefSettings);


/******************************************************************************
Summary:
   Open the BSRF API
Description:
   This function initializes a BSRF_Handle.
Returns:
   BERR_Code: BERR_SUCCESS = the returned BSRF_Handle is valid
******************************************************************************/
BERR_Code BSRF_Open(
   BSRF_Handle *h,      /* [out] BSRF handle */
   BCHP_Handle hChip,   /* [in] chip handle */
   void *pReg,          /* [in] pointer to register or i2c handle */
   BINT_Handle hInt,    /* [in] interrupt handle */
   const BSRF_Settings *pDefSettings   /* [in] default settings */
);


/******************************************************************************
Summary:
   Close the BSRF API
Description:
   This function releases all the resources allocated by BSRF API.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_Close(BSRF_Handle h);


/******************************************************************************
Summary:
   Gets the total number of channels for the BSRF device
Description:
   This function gets the total number of channels supported by the BSRF device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_GetTotalChannels(
   BSRF_Handle h,             /* [in] BSRF handle */
   uint8_t     *totalChannels /* [out] number of channels supported */
);


/******************************************************************************
Summary:
   Gets default channel settings
Description:
   This function gets the default settings for a BSRF device channel.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_GetChannelDefaultSettings(
   BSRF_Handle   h,                      /* [in] BSRF handle */
   uint8_t       chanNum,                /* [in] channel number */
   BSRF_ChannelSettings *pChnDefSettings /* [out] default channel settings */
);


/******************************************************************************
Summary:
   Initializes a BSRF device channel.
Description:
   This function initializes a BSRF device channel and returns a BSRF_ChannelHandle.
Returns:
   BERR_Code : BERR_SUCCESS = the returned BSRF_ChannelHandle is valid
******************************************************************************/
BERR_Code BSRF_OpenChannel(
   BSRF_Handle                h,                /* [in] BSRF handle */
   BSRF_ChannelHandle         *pChannelHandle,  /* [out] BSRF channel handle */
   uint8_t                    chanNum,          /* [in] channel number */
   const BSRF_ChannelSettings *pSettings        /* [in] channel settings */
);


/******************************************************************************
Summary:
   Closes the BSRF device channel.
Description:
   This function frees the channel handle and any resources contained in the channel handle.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_CloseChannel(BSRF_ChannelHandle h);


/******************************************************************************
Summary:
   Reset the BSRF device
Description:
   (Re)initializes the firmware to its power up init state.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_Reset(BSRF_Handle h);


/******************************************************************************
Summary:
   Gets version for BSRF PI
Description:
   This function returns the version of BSRF porting interface.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_GetVersion(BSRF_Handle h, BFEC_VersionInfo *pVersion);


/******************************************************************************
Summary:
   Powers up the specified BSRF channel.
Description:
   This function enables the given BSRF channel.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_PowerUp(BSRF_ChannelHandle h);


/******************************************************************************
Summary:
   Power down the specified BSRF channel.
Description:
   This function disables the given BSRF channel.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_PowerDown(BSRF_ChannelHandle h);


/******************************************************************************
Summary:
   Freeze RF AGC
Description:
   The function will freeze the RFAGC integrator on the specified channel.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_FreezeRfAgc(BSRF_ChannelHandle h);


/******************************************************************************
Summary:
   Unfreeze RF AGC
Description:
   The function will unfreeze the RFAGC integrator on the specified channel.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_UnfreezeRfAgc(BSRF_ChannelHandle h);


/******************************************************************************
Summary:
   Write RF AGC
Description:
   The function will overwrite the RFAGC integrator if frozen.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_WriteRfAgc(BSRF_ChannelHandle h, uint32_t val);


/******************************************************************************
Summary:
   Read RF AGC
Description:
   The function will read back the current RFAGC integrator value.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_ReadRfAgc(BSRF_ChannelHandle h, uint32_t *pVal);


/******************************************************************************
Summary:
   Write RF gain
Description:
   The function will overwrite the RF gain if frozen.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_WriteRfGain(BSRF_ChannelHandle h, uint8_t gain);


/******************************************************************************
Summary:
   Read RF gain
Description:
   The function will read back the current RF gain value.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_ReadRfGain(BSRF_ChannelHandle h, uint8_t *pGain);


/******************************************************************************
Summary:
   Estimate input power at chip input
Description:
   The function will return the input power estimate in dBm scaled by 256.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_GetInputPower(BSRF_ChannelHandle h, int32_t *pPower);


/******************************************************************************
Summary:
   Sets the RFAGC settings
Description:
   The function will configure the RFAGC settings.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_SetRfAgcSettings(BSRF_ChannelHandle h, BSRF_RfAgcSettings settings);


/******************************************************************************
Summary:
   Gets the RFAGC settings
Description:
   The function will return the RFAGC settings.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_GetRfAgcSettings(BSRF_ChannelHandle h, BSRF_RfAgcSettings *pSettings);


/******************************************************************************
Summary:
   Enables or disables fast decay operation
Description:
   The function will enable/disable fast dcay operation.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_EnableFastDecayMode(BSRF_ChannelHandle h, bool bEnable);


/******************************************************************************
Summary:
   Sets the fast decay gain threshold
Description:
   The function will set the fast decay gain threshold, where the RFAGC switches from
   fast decay settings to decay settings.  The threshold ranges from -64 to 15.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_SetFastDecayGainThreshold(BSRF_ChannelHandle h, int8_t threshold);


/******************************************************************************
Summary:
   Gets the fast decay gain threshold
Description:
   The function will return the fast decay gain threshold.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_GetFastDecayGainThreshold(BSRF_ChannelHandle h, int8_t *pThreshold);


/******************************************************************************
Summary:
   Sets the antenna over-current threshold
Description:
   The function will set the antenna over-current threshold, which can be set from 0 to 3,
   corresponding to AOC_mode_0 to AOC_mode_3.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_SetAntennaOverThreshold(BSRF_ChannelHandle h, uint8_t mode);


/******************************************************************************
Summary:
   Gets the antenna over-current threshold
Description:
   The function will return the antenna over-current threshold.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_GetAntennaOverThreshold(BSRF_ChannelHandle h, uint8_t *pMode);


/******************************************************************************
Summary:
   Sets the antenna detection threshold
Description:
   The function will set the antenna detection threshold, which can be set from 0 to 3,
   corresponding to AD_mode_0 to AD_mode_3.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_SetAntennaDetectThreshold(BSRF_ChannelHandle h, uint8_t mode);


/******************************************************************************
Summary:
   Gets the antenna detection threshold
Description:
   The function will return the antenna detection threshold.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_GetAntennaDetectThreshold(BSRF_ChannelHandle h, uint8_t *pMode);


/******************************************************************************
Summary:
   Gets the antenna status
Description:
   The function will return the antenna status.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_GetAntennaStatus(BSRF_ChannelHandle h, BSRF_AntennaStatus *pStatus);


/******************************************************************************
Summary:
   Powers up the antenna sense.
Description:
   This function enables the antenna sense block.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_PowerUpAntennaSense(BSRF_ChannelHandle h);


/******************************************************************************
Summary:
   Power down the antenna sense.
Description:
   This function disables the antenna sense block.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_PowerDownAntennaSense(BSRF_ChannelHandle h);


/******************************************************************************
Summary:
   Tunes the satellite radio frontend
Description:
   The function will tune the mixer to the specified frequency.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_Tune(BSRF_ChannelHandle h, uint32_t freqHz);


/******************************************************************************
Summary:
   Gets the tuner status
Description:
   The function will return the tuner status.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_GetTunerStatus(BSRF_ChannelHandle h, BSRF_TunerStatus *pStatus);


/******************************************************************************
Summary:
   Resets the clip counter
Description:
   The function will reset the clip counter.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_ResetClipCount(BSRF_ChannelHandle h);


/******************************************************************************
Summary:
   Gets the clip counter value
Description:
   The function will return the clip counter value.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_GetClipCount(BSRF_ChannelHandle h, uint32_t *pClipCount);


/******************************************************************************
Summary:
   Configure and enable test ports
Description:
   The function will configure and enable the specified test port.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_ConfigTestMode(BSRF_ChannelHandle h, BSRF_TestportSelect tp, bool bEnable);


/******************************************************************************
Summary:
   Configure output parameters
Description:
   The function will configure the output slew rate (slewed or fast) and drive strength
   (2 to 16mA in steps of 2mA) for the specified set of outputs.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_ConfigOutput(BSRF_Handle h, BSRF_OutputSelect output, bool bSlewEdges, uint8_t driveStrength_ma);


/******************************************************************************
Summary:
   Configure test DAC
Description:
   The function will configure the test DAC frequency and sinx/x filter coefficients
   (4 coefficients since 7 taps with odd symmetry).
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_ConfigTestDac(BSRF_Handle h, int32_t freqHz, int16_t *pCoeff);


/******************************************************************************
Summary:
   Enable test DAC
Description:
   The function will enable the test DAC.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_EnableTestDac(BSRF_Handle h);


/******************************************************************************
Summary:
   Disable test DAC
Description:
   The function will disable the test DAC.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_DisableTestDac(BSRF_Handle h);


/******************************************************************************
Summary:
   Enable or disable test DAC tone
Description:
   The function will enable or disable the test DAC tone at a specified amplitude.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_EnableTestDacTone(BSRF_Handle h, bool bToneOn, uint16_t toneAmpl);


/******************************************************************************
Summary:
   Runs diagnostic data capture
Description:
   The function will run one interation of the diagnostic data capture for 8192 data points.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_RunDataCapture(BSRF_Handle h);


/******************************************************************************
Summary:
   Delete AGC LUT code
Description:
   The function will specify which AGC LUT codes to omit when initializing the RFAGC.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_DeleteAgcLutCodes(BSRF_Handle h, uint32_t *pIdx, uint32_t n);


/******************************************************************************
Summary:
   Configures output clock phase
Description:
   The function will configure the output phase of the clock on the 32-bit digital interface.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_ConfigOutputClockPhase(BSRF_Handle h, uint8_t phase, bool bDisableOutput);


/******************************************************************************
Summary:
   Sets IQ imbalance equalization taps
Description:
   The function will program the IQ imbalance equalization coefficients.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_SetIqEqCoeff(BSRF_ChannelHandle h, int16_t *iTaps, int16_t *qTaps);


/******************************************************************************
Summary:
   Sets IQ imbalance equalization settings
Description:
   The function will program the IQ imbalance equalization parameters.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_SetIqEqSettings(BSRF_ChannelHandle h, BSRF_IqEqSettings settings);


/******************************************************************************
Summary:
   Enables mixpll current boost
Description:
   The function will enable mixpll current boost for enhanced performance
   at the expense of increased power.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_BoostMixPllCurrent(BSRF_ChannelHandle h, bool bBoost);


#ifdef __cplusplus
}
#endif

#endif /* BSRF_H__ */
