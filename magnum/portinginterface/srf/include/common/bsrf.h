/*************************************************************************
*     (c)2005-2014 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
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
   uint32_t threshold;
   uint32_t step;
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
   BERR_Code (*GetInputPower)(BSRF_ChannelHandle, uint32_t*);
   BERR_Code (*SetRfAgcSettings)(BSRF_ChannelHandle, BSRF_RfAgcSettings);
   BERR_Code (*GetRfAgcSettings)(BSRF_ChannelHandle, BSRF_RfAgcSettings*);
   BERR_Code (*SetFastDecayGainThreshold)(BSRF_ChannelHandle, uint32_t);
   BERR_Code (*GetFastDecayGainThreshold)(BSRF_ChannelHandle, uint32_t*);
   BERR_Code (*SetAntennaOverThreshold)(BSRF_ChannelHandle, uint8_t);
   BERR_Code (*GetAntennaOverThreshold)(BSRF_ChannelHandle, uint8_t*);
   BERR_Code (*SetAntennaDetectThreshold)(BSRF_ChannelHandle, uint8_t);
   BERR_Code (*GetAntennaDetectThreshold)(BSRF_ChannelHandle, uint8_t*);
   BERR_Code (*GetAntennaStatus)(BSRF_ChannelHandle, BSRF_AntennaStatus*);
   BERR_Code (*Tune)(BSRF_ChannelHandle, int32_t);
   BERR_Code (*GetTunerStatus)(BSRF_ChannelHandle, BSRF_TunerStatus*);
   BERR_Code (*ResetClipCount)(BSRF_ChannelHandle);
   BERR_Code (*GetClipCount)(BSRF_ChannelHandle, uint32_t*);
   BERR_Code (*ConfigTestMode)(BSRF_ChannelHandle);
   BERR_Code (*ConfigOutput)(BSRF_Handle, BSRF_OutputSelect, bool, uint8_t);
   BERR_Code (*ConfigTestDac)(BSRF_Handle, int32_t, int16_t*);
   BERR_Code (*EnableTestDac)(BSRF_Handle);
   BERR_Code (*DisableTestDac)(BSRF_Handle);
   BERR_Code (*EnableTestDacTone)(BSRF_Handle, bool, uint16_t);
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
   Estimate input power at chip input
Description:
   The function will return the input power estimate in dBm scaled by 256.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_GetInputPower(BSRF_ChannelHandle h, uint32_t *pPower);


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
   Sets the fast decay gain threshold
Description:
   The function will set the fast decay gain threshold, where the RFAGC switches from
   fast decay settings to decay settings.  Setting a threshold of zero will disable fast
   decay operation.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_SetFastDecayGainThreshold(BSRF_ChannelHandle h, uint32_t threshold);


/******************************************************************************
Summary:
   Gets the fast decay gain threshold
Description:
   The function will return the fast decay gain threshold.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_GetFastDecayGainThreshold(BSRF_ChannelHandle h, uint32_t *pThreshold);


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
   Tunes the satellite radio frontend
Description:
   The function will tune the mixer to the specified frequency.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_Tune(BSRF_ChannelHandle h, int32_t freqHz);


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
   Configure test mode
Description:
   The function will configure the various test modes TBD.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSRF_ConfigTestMode(BSRF_ChannelHandle h);  /* TBD */


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


#ifdef __cplusplus
}
#endif

#endif /* BSRF_H__ */
