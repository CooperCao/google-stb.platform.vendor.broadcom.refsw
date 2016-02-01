/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BTNR_7125_H__
#define BTNR_7125_H__

#include "bchp.h"
#include "breg_i2c.h"
#include "btnr.h"
#include "btmr.h"
#include "bmem.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BTNR_7125_LNA_GAIN_BOOST_ON (1<<12)
#define BTNR_7125_LNA_SUPERBOOST_ON (1<<13)
#define BTNR_7125_LNA_TILT_ON (1<<14)

typedef struct BTNR_7125_VGA_Cal
{	/* AGC thresholds set by VGA CAL.  Set them (Min_AGC_Threshold) to 0 if you want 
	   btnr_7125 to attempt its own VGA Calibration. Set them to good, calibrated
	   values if you have them.  Leave them as set by BTNR_7125_GetDefaultSettings
	   otherwise. */
	int32_t 			Min_AGC_Threshold;	/* Lower AGC Threshold */
	int32_t 			Max_AGC_Threshold;  /* Upper range of AGC */
} BTNR_7125_VGA_Cal;

typedef struct
{
	BREG_I2C_Handle hI2cLNA;/* handle to the I2C to which the LNA is connected */
	unsigned unitNumber;  	/* Which tuner to open from this device. This is set only at open-time. */
	BTMR_Handle hTmr;
	BMEM_Heap_Handle hHeap;
	BTNR_7125_VGA_Cal VgaCal; /* VGA Calibration values, only used at open-time. See above. */
	bool skipInit;			/* skip initialization that might disrupt ongoing reception */
} BTNR_7125_Settings;

/***************************************************************************
Summary:
    This function returns the default settings for Bcm7125 Tuner module.

Description:
    This function is responsible for returns the default setting for 
    BTNR module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BTNR_7125_Open()

****************************************************************************/
BERR_Code BTNR_7125_GetDefaultSettings(
    BTNR_7125_Settings *pDefSettings  /* [out] Returns default setting */
    );


/***************************************************************************
Summary:
    This function opens Bcm7125 Tuner module.

Description:
    This function is responsible for opening Bcm7125 BTNR module. When BTNR is
    opened, it will create a module handle and configure the module based
    on the default settings. Once the device is opened, it must be closed
    before it can be opened again.

Returns:
    TODO:

See Also:
    BTNR_Close(), BTNR_7125_GetDefaultSettings()

****************************************************************************/
BERR_Code BTNR_7125_Open(
    BTNR_Handle *phDev,                 /* [out] Returns handle */
	BREG_Handle hRegister,					/* Register handle */
    BTNR_7125_Settings *pSettings
    );

/***************************************************************************
Summary:
	7125 Tuner Functions Options 
****************************************************************************/
typedef enum BTNR_7125_TunerAcquisitionType
{
  BTNR_7125_TunerAcquisitionType_eInitTune=1,
  BTNR_7125_TunerAcquisitionType_eTune=2,
  BTNR_7125_TunerAcquisitionType_eMiniTune=3
} BTNR_7125_TunerAcquisitionType;

/***************************************************************************
Summary:
	7125 Tuner Bandwidth Options 
****************************************************************************/
typedef enum BTNR_7125_Bandwidth
{
  BTNR_7125_Bandwidth_e8MHz=1,
  BTNR_7125_Bandwidth_e7MHz=2,
  BTNR_7125_Bandwidth_e6MHz=3,
  BTNR_7125_Bandwidth_e5MHz=4
} BTNR_7125_Bandwidth;

/***************************************************************************
Summary:
	7125 Tuner Tune Settings 
****************************************************************************/
typedef struct BTNR_7125_TuneSettings
{
	BTNR_7125_Bandwidth   Bandwidth;
	uint32_t			  Freq;
} BTNR_7125_TuneSettings;

/***************************************************************************
Summary:
	Get Default 7125 Tuner Tune Settings 
****************************************************************************/
BERR_Code BTNR_7125_GetDefaultTuneSettings(
    BTNR_7125_TuneSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Tune the 7125 tuner.
 
Description:
    This function should be used instead of
    BTNR_SetRfFrequency() when Bandwidth and AcquisitionModes are
    different from the default settings (8MHz DVB-T).
 
See Also: 
    BTNR_7125_Tune
****************************************************************************/
BERR_Code BTNR_7125_Tune(
    BTNR_Handle hTnr,
    const BTNR_7125_TuneSettings *pSettings
    );

/***************************************************************************
Summary:
    Process Interrupt Event.
 
Description:

See Also: 
    BTNR_7125_Tune
****************************************************************************/

BERR_Code BTNR_7125_ProcessInterruptEvent(BTNR_Handle h);


/***************************************************************************
Summary:
    Get Interrupt Event Handle
 
Description:

See Also: 
    BTNR_7125_Tune
****************************************************************************/
BERR_Code BTNR_7125_GetInterruptEventHandle(
	BTNR_Handle h, 
	BKNI_EventHandle* hEvent);

/***************************************************************************
Summary:
    Enable/Disable RF by pass mode
 
Description: Enable/Disable RF by pass mode
mode = True means enabled the RF by pass
mode = false means diable RF by pass

See Also: 
    BTNR_7125_RFByPassMode
****************************************************************************/
BERR_Code BTNR_7125_SetRFByPassMode(
    BTNR_Handle hTnr,
    bool mode);



/***************************************************************************
Summary:
    This function opens tells if the 7125 device's RF bypass is enabled/disabled.
 
Description: Enable/Disable RF by pass mode
mode = True means enabled the RF by pass
mode = false means diable RF by pass

See Also: 
    BTNR_7125_RFByPassMode
****************************************************************************/
BERR_Code BTNR_7125_GetRFByPassMode(
    BTNR_Handle hTnr,
    bool *mode);

/***************************************************************************
Summary:
    7125 Tuner Status
****************************************************************************/
typedef struct BTNR_7125_TunerStatus
{
    int32_t rfAgcVal;   /* RF AGC value in 1/256 dB */
	int32_t ifAgcVal;   /* IF AGC value in 1/256 dB */  
    int32_t lnaAgcVal;   /* LNA AGC value in 1/256 dB */
	int32_t	dpmLvel;		/* dpm dvbm level 1/256 */
} BTNR_7125_TunerStatus;

/***************************************************************************
Summary:
    BTNR_7125_GetTunerStatus
****************************************************************************/
BERR_Code BTNR_7125_GetTunerStatus(
    BTNR_Handle hTnr,
    BTNR_7125_TunerStatus *pStatus
    );

/***************************************************************************
Summary:
    Data for the ADS interrupt callback
****************************************************************************/
typedef struct BTNR_7125_AdsInterruptData
{
	enum 
	{
	  BTNR_7125_SetDpm,
	  BTNR_7125_ResetDpm,
	  BTNR_7125_RequestLnaGain
	} action;
    uint8_t iOutdivMs;
} BTNR_7125_AdsInterruptData;


/***************************************************************************
Summary:
    Callback to pass tuning parameters to the demodulator
****************************************************************************/
typedef void (*BTNR_7125_AdsInterruptCallback)(void *pParam1, int param2, const BTNR_7125_AdsInterruptData *pData);


/***************************************************************************
Summary:
    Callback info
****************************************************************************/
typedef struct BTNR_P_CallbackInfo
{
    BTNR_7125_AdsInterruptCallback func;
    void *pParm1;
	int Parm2;
} BTNR_P_CallbackInfo;

/***************************************************************************
Summary:
    BTNR_7125_InstallAdsInterruptCallback
****************************************************************************/
BERR_Code BTNR_7125_InstallAdsInterruptCallback(
	BTNR_Handle			            hDev,
	BTNR_7125_AdsInterruptCallback  fCallBack_isr,
    void                            *pParam1,
    int					            param2
    );

/***************************************************************************
Summary:
    BTNR_7125_RemoveAdsInterruptCallback
****************************************************************************/
BERR_Code BTNR_7125_RemoveAdsInterruptCallback(
    BTNR_Handle handle  /* [in] BTNR handle */
    );

#ifdef __cplusplus
}
#endif
 
#endif

