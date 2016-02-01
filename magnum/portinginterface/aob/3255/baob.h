/***************************************************************************
 *     Copyright (c) 2005-2011, Broadcom Corporation
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

/*= Module Overview *********************************************************
<verbatim>

Overview
The Advanced Qam Out Of Band (ADS) Downstream receiver is responsible for taking an
Out-of-Band RF signal (70-130 MHz), down converting into IF signal. This IF
signal is then digitalized, demodulated and error corrected. This error
corrected data stream can be then routed to Davic MAC.


Design
  The design for BAOB PI API is broken into three parts.

o Part 1 (open/close/configuration):

    These APIs are used for opening and closing BAOB device.
    When a device is opened, the device can be configured.

o Part 2 (acquire/get/set):

    These APIs are used for getting and setting runtime attributes of BAOB.
    Along with get and set APIs, an acquire API is also provided.  The acquire
    API is used to acquire (lock) OutOfBand Downstream signal.

o Part 3 (process):

    This API is used to process notification.

Usage
The usage of BAOB involves the following:

   * Configure/Open of BQOB

      * Configure BAOB device for the target system
      * Open BAOB device

   * Program/Acquire device

      * Try to acquire OutOfBand Downstream signal
      * Check status of acquisition
	  * Wait for Lock/Unlock ntotification sent from 325X
	     * Process Lock/Unlock Notification


Interrupt Requirements
No interrrupt processing is done by this module.


Sample Code:
//
// NOTE: The following sample code does not do any error checking.
//
// In this example, the target H/W board is configured for an
// Out-of-Band Downstream DVS 178 application, with a Crystal Frequency of 24.00 MHz
// and IF Frequency of 44 MHz.
//
static BAOB_Handle aobDev;
static BCHP_Handle hChip3255;
static BREG_Handle hReg3255;
static BRPC_Handle hRpc;

static BAOB_CallbackFunc cbLockChangeFunc(
	BAOB_Handle hIbChn,
	void *pParam
	)
{
	bool *lockStatus = (bool *)	pParam;

	BAOB_GetLockStatus( hIbChn, lockStatus );

	return;
}

main( void )
{
	BAOB_Settings aobDevSettings;
	bool isLock;

	// Initialize hChip3255, hReg3255, and hRpc . . .

	// Initialize BTNR_3255Ob


	// BAOB initialization for BCM3255
	BAOB_GetDefaultSetting( &aobDevSettings );

	// Configure the Crystal and IF Frequency of H/W board.
	aobDevSettings.xtalFreq = 24000000;
	aobDevSettings.ifFreq = 44000000;
	aobDevSettings.enableFEC = true; // false: CableCARD will do FEC
									  // true: legacy OOB needs FEC
	aobDevSettings.hRpc = hRpc;
	BAOB_Open( &aobDev, hChip3255, hReg3255, &aobDevSettings );


	// Now startup the RPC Notificiation Background thread/task . . .

	// Install callback for state change event, where last parameter
	// is for general use.
	BAOB_InstallCallback( aobDev, BAOB_Callback_eLockChange, cbLockChangeFunc, (void *) &isLock );

	// Tune Tuner . . .


	// Acquire OutOfBand Downstream Signal, DVS 178 QPSK modulation with a
	// symbol rate of 777 KBaud.
	BAOB_Set_Spectrum( aobDev, BAOB_SpectrumMode_eNoInverted);

	BAOB_Acquire( aobDev, BAOB_ModulationType_eDvs178Qpsk, 777000 );

	while(1)
	{
		// Print the lock status every second
		BKNI_Sleep(1000);
		BAOB_GetLOckStatus(aobDev, &isLock);
		printf("Lock Status: %s\n", (isLock==true): "locked", "not lock");
	}
}

void checkRpcNotificationThread(. . .)
{
	while (!system_terminated)
	{
		uint32_t device_id, event;

		BRPC_CheckNotification(hRpc, &device_id, &event, 0);
		if ((BRPC_GET_NOTIFICATION_EVENT(event))
		{
			switch (device_id)
			{
				case BRPC_DevId_3255_DS0:
					break;
				case BRPC_DevId_3255_DS1:
					break;
				case BRPC_DevId_3255_OB0:
					BAOB_ProcessNotification(aobDev);
					break;
				default:
					// device not used
					break;
			}
		}
		else
		{
			BKNI_Sleep(100);
		}
	}
}

</verbatim>

***************************************************************************/

#ifndef BAOB_H__
#define BAOB_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	Error Codes specific to BAOB

Description:

See Also:

****************************************************************************/
#define BAOB_ERR_NOTAVAIL_MODULATION_TYPE	BERR_MAKE_CODE(BERR_AOB_ID, 0)

/***************************************************************************
Summary:
	The handles for Qam Out-of-Band Downstream module.

Description:
	An opaque handle for BAOB device.

See Also:
	BAOB_Open()

****************************************************************************/
typedef struct BAOB_P_Handle				*BAOB_Handle;

/***************************************************************************
Summary:
	Enumeration for modulation type

Description:
	This enumeration defines the valid modulation types.

See Also:
	BAOB_Acquire(), BAOB_GetStatus()

****************************************************************************/
typedef enum BAOB_ModuationType
{
	BAOB_ModulationType_eAnnexAQpsk,
	BAOB_ModulationType_eDvs178Qpsk,
	BAOB_ModulationType_ePod_AnnexAQpsk,
	BAOB_ModulationType_ePod_Dvs178Qpsk,
	BAOB_ModulationType_eLast
} BAOB_ModulationType;

/***************************************************************************
Summary:
	Enumeration for Annex Mode

Description:
	This enumeration defines the two Annex modes.

See Also:
	BAOB_ChannelSettings

****************************************************************************/
typedef enum BAOB_AnnexMode
{
	BQOB_AnnexMode_eAnnexA,
	BQOB_AnnexMode_eDvs178,
	BQOB_AnnexMode_eLast
} BQOB_AnnexMode;

/***************************************************************************
Summary:
	Enumeration for QPSK OOB spectrum setting

Description:
	This enumeration defines the valid spectrum setting.

See Also:
	BAOB_Get_Spectrum(), BAOB_Set_Spectrum()

****************************************************************************/
typedef enum BAOB_SpectrumMode
{
	BAOB_SpectrumMode_eAuto,
	BAOB_SpectrumMode_eNoInverted,
	BAOB_SpectrumMode_eInverted
} BAOB_SpectrumMode;
/***************************************************************************
Summary:
	This structure represents the AOB Status for a Qam Out-of-Band Downstream
	module.

Description:
	This structure is returned when BAOB_GetStatus() is called.  This
	structure contains the complete status of Qam Out-of-Band Downstream
	module.

See Also:
	BAOB_GetStatus()

****************************************************************************/
typedef struct BAOB_Status
{
	bool isPowerSaverEnabled;			/* Enable=1, Disable=0 */
	BAOB_ModulationType modType;		/* Modulation Type */
	uint32_t ifFreq;					/* in Hertz, IF freq. */
	uint32_t loFreq;					/* in Hertz, LO freq. */
	uint32_t sysXtalFreq;				/* in Hertz, Sys. Xtal freq. */
	uint32_t symbolRate;				/* in Baud */
	bool isFecLock;						/* lock=1, unlock=0 */
	bool isQamLock;						/* lock=1, unlock=0 */
	int32_t snrEstimate;				/* in 1/256 db */
	int32_t agcIntLevel;				/* in 1/10 percent */
	int32_t agcExtLevel;				/* in 1/10 percent */
	int32_t carrierFreqOffset;			/* in 1/1000 Hz */
	int32_t carrierPhaseOffset;			/* in 1/1000 Hz */
	uint32_t uncorrectedCount;			/* not self-clearing  */
	uint32_t correctedCount;			/* not self-clearing*/
	uint32_t berErrorCount;				/* not self-clearing */
	uint32_t fdcChannelPower;           /* Only valid if using DS0 for OOB! in 1/10 dBmV unit; OCAP DPM support for OOB channel */	
} BAOB_Status;

/***************************************************************************
Summary:
	Required default settings structure for QPSK Out-of_Band module.

Description:
	The default setting structure defines the default configuration of
	QPSK Out-of_Band when the device is opened.

See Also:
	BAOB_Open()

****************************************************************************/
#define BAOB_SETTINGS_XTALFREQ					(24000000)	/* 24.00 MHz */

#if BCHP_CHIP == 7405 || BCHP_CHIP == 7420
#define BAOB_SETTINGS_IFFREQ					(1750000)	/* 1.75 MHz */
#elif BCHP_CHIP == 7125
#define BAOB_SETTINGS_IFFREQ					(1250000)	/* 1.25 MHz */
#else
#define BAOB_SETTINGS_IFFREQ					(44000000)	/* 44.00 MHz */
#endif
typedef struct BAOB_Settings
{
	unsigned int devId;		/* generic device ID */
	void *hGeneric;			/* generic handle can be used for anything */
	uint32_t xtalFreq; /* Crystal Freqency in Hertz */
	unsigned long ifFreq; /* IF Frequency in Hertz */
	bool enableFEC; /* use OOB FEC or not */
	BAOB_SpectrumMode spectrum; /*default Spectrum setting*/
} BAOB_Settings;


typedef enum BAOB_Callback
{
	BAOB_Callback_eLockChange,			/* Callback to notify application of lock change */
	BAOB_Callback_eLast                 /* More may be required */
} BAOB_Callback;

/***************************************************************************
Summary:
	Callback used for event notification.

Description:
	When this PI wants to notify an application, it will call this callback
	function the callback function is registered.

See Also:
	BAOB_InstallLockStateChangeCallback()

****************************************************************************/
typedef BERR_Code (*BAOB_CallbackFunc)(void *pParam );


/***************************************************************************
Summary:
	This function returns the default settings for QPSK Out-of_Band module.

Description:
	This function is responsible for returns the default setting for
	BAOB module. The returning default setting should be when
	opening the device.

Returns:
	TODO:

See Also:
	BAOB_Open()

****************************************************************************/
BERR_Code BAOB_GetDefaultSettings(
	BAOB_Settings *pDefSettings,		/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	);



/***************************************************************************
Summary:
	This function opens Qam Out-of-Band Downstream module.

Description:
	This function is responsible for opening BAOB module. When BAOB is
	opened, it will create a module handle and configure the module based
	on the default settings. Once the device is opened, it must be closed
	before it can be opened again.

Returns:
	TODO:

See Also:
	BAOB_Close(), BAOB_GetDefaultSettings()

****************************************************************************/
BERR_Code BAOB_Open(
	BAOB_Handle *pAob,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_Handle hRegister,				/* [in] Register handle */
	BINT_Handle hInterrupt,				/* [in] Interrupt handle, Bcm3250 */
	const BAOB_Settings *pDefSettings	/* [in] Default settings */
	);

/***************************************************************************
Summary:
	This function closes Qam Out-of-Band Downstream module.

Description:
	This function is responsible for closing BAOB module. Closing BAOB
	will free main BAOB handle.

Returns:
	TODO:

See Also:
	BAOB_Open()

****************************************************************************/
BERR_Code BAOB_Close(
	BAOB_Handle hAob					/* [in] Device handle */
	);


/***************************************************************************
Summary:
	This function tries to acquire downstream lock for the specific
	Qam Out-of-Band Downstream module.

Description:
	This function is responsible for trying to acquire downstream lock of
	the input Out-of-Band signal. Acquiring Out-of-Band downstream lock involves
	configuring the H/W to desire configuration, then running a Qam Out-of-Band
	downstream acquisition script. If this is the first acquisition for the
	current annex mode, then a Qam Out-of-Band downstream configuration script
	will be run prior to running acquisition script.
	This function will automatically enable the downstream receiver if
	the receiver was in power-saver mode.

Returns:
	TODO:

See Also:
	BAOB_GetLock(), BAOB_GetStatus(), BAOB_GetSoftDecision()

****************************************************************************/
BERR_Code BAOB_Acquire(
	BAOB_Handle hDev,					/* [in] Device handle */
	BAOB_ModulationType modType,		/* [in] Modulation type to use */
	uint32_t symbolRate					/* [in] Symbol rate to use */
	);

/***************************************************************************
Summary:
	This function returns the current spectrum  settings for QPSK Out-of_Band module.

Description:
	This function is responsible for getting the current spectrum setting for
	QPSK Out-of_Band.

Returns:
	TODO:

See Also:
	BAOB_Set_Spectrum()

****************************************************************************/
BERR_Code BAOB_Get_Spectrum(
	BAOB_Handle hDev,					/* [in] Device handle */
	BAOB_SpectrumMode *spectrum			/* [out]current spectrum setting*/
	);


/***************************************************************************
Summary:
	This function sets the spectrum settings for QPSK Out-of_Band module.

Description:
	This function is responsible for setting spectrum for QPSK Out-of_Band.

Returns:
	TODO:

See Also:
	BAOB_Get_Spectrum()

****************************************************************************/
BERR_Code BAOB_Set_Spectrum(
	BAOB_Handle hDev,					/* [in] Device handle */
	BAOB_SpectrumMode spectrum			/* [in] spectrum setting to be set*/
	);
/***************************************************************************
Summary:
	This function gets the status of Qam Out-of-Band Downstream module.

Description:
	This function is responsible for getting the complete status for
	a Qam Out-of-Band Downstream module.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BAOB_GetStatus(
	BAOB_Handle hDev,					/* [in] Device handle */
	BAOB_Status *pStatus				/* [out] Returns status */
	);

/***************************************************************************
Summary:
	This function gets the lock status for a Qam Out-of-Band Downstream
	module.

Description:
	This function is responsible for getting the lock status
	for a BAOB module.

Returns:
	TODO:

See Also:
	BAOB_GetStatus()

****************************************************************************/
BERR_Code BAOB_GetLockStatus(
	BAOB_Handle hDev,					/* [in] Device handle */
	bool *isLock						/* [out] Returns lock status, 0=not lock, 1=locked */
	);

/***************************************************************************
Summary:
	This function gets the I and Q values for soft decision of a
	Qam Out-of-Band Downstream module.

Description:
	This function is responsible for getting the I and Q values for soft
	decision of a BAOB module.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BAOB_GetSoftDecision(
	BAOB_Handle hDev,					/* [in] Device handle */
	int16_t nbrToGet,					/* [in] Number values to get */
	int16_t *iVal,						/* [out] Ptr to array to store output I soft decision */
	int16_t *qVal,						/* [out] Ptr to array to store output Q soft decision */
	int16_t *nbrGotten					/* [out] Number of values gotten/read */
	);

/***************************************************************************
Summary:
	This function enable the power-saver mode.

Description:
	This function is responsible for enabling the OOB downstream receiver
	power-saver mode.  When the BAOB is in the power-saver mode, the
	Qam/QPSK OOB Downstream receiver is shutdown. BAOB_Acquire() will disable
	power-saver mode automatically.

Returns:
	TODO:

See Also:
	BAOB_Acquire()

****************************************************************************/
BERR_Code BAOB_EnablePowerSaver(
	BAOB_Handle hDev					/* [in] Device handle */
	);

/***************************************************************************
Summary:
	This function is responsible for processing a notificiation for the specific
	Qam Out-of-Band Downstream module channel.

Description:
	This function needs to called when notification is received.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BAOB_ProcessNotification(
	BAOB_Handle hDev,						/* [in] Device handle */
	unsigned int event						/* [in] Event code and event data*/
	);

/***************************************************************************
Summary:
	This function is responsible for installing a callback function.

Description:
	This function installs a callback function.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BAOB_InstallCallback(
	BAOB_Handle hDev,					/* [in] Device handle */
	BAOB_Callback callbackType,			/* [in] Type of callback */
	BAOB_CallbackFunc pCallback,		/* [in] Function Ptr to callback */
	void *pParam						/* [in] Generic parameter send on callback */
	);

#ifdef __cplusplus
}
#endif

#endif

