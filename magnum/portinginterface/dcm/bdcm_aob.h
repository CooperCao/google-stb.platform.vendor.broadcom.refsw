/***************************************************************************
 *     Copyright (c) 2013-2013, Broadcom Corporation
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

#ifndef BDCM_AOB_H__
#define BDCM_AOB_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    A handle for DOCSIS OOB channel.
****************************************************************************/
typedef struct BDCM_AobChannel		*BDCM_AobChannelHandle;

/***************************************************************************
Summary:
    Enumeration for QPSK modulation type used in an OOB channel.
****************************************************************************/
typedef enum BDCM_AobModuationType
{
    BDCM_AobModulationType_eAnnexAQpsk,
    BDCM_AobModulationType_eDvs178Qpsk,
    BDCM_AobModulationType_ePod_AnnexAQpsk,
    BDCM_AobModulationType_ePod_Dvs178Qpsk,
    BDCM_AobModulationType_eLast
} BDCM_AobModulationType;

/***************************************************************************
Summary:
    Enumeration for Annex Mode
****************************************************************************/
typedef enum BDCM_AobAnnexMode
{
    BDCM_AobAnnexMode_eAnnexA,
    BDCM_AobAnnexMode_eDvs178,
    BDCM_AobAnnexMode_eLast
} BDCM_AobAnnexMode;

/***************************************************************************
Summary:
    Enumeration for QPSK OOB spectrum setting
****************************************************************************/
typedef enum BDCM_AobSpectrumMode
{
    BDCM_AobSpectrumMode_eAuto,
    BDCM_AobSpectrumMode_eNoInverted,
    BDCM_AobSpectrumMode_eInverted
} BDCM_AobSpectrumMode;

/***************************************************************************
Summary:
    This structure represents status of DOCSIS OOB channel.
***************************************************************************/
typedef struct BDCM_AobStatus
{
    bool isPowerSaverEnabled;          /* Enable=1, Disable=0 */
    BDCM_AobModulationType modType; /* Modulation Type */
    uint32_t ifFreq;                   /* in Hertz, IF freq. */
    uint32_t loFreq;                   /* in Hertz, LO freq. */
    uint32_t sysXtalFreq;              /* in Hertz, Sys. Xtal freq. */
    uint32_t symbolRate;               /* in Baud */
    bool isFecLock;                    /* lock=1, unlock=0 */
    bool isQamLock;	                   /* lock=1, unlock=0 */
    int32_t snrEstimate;               /* in 1/256 db */
    int32_t agcIntLevel;               /* in 1/10 percent */
    int32_t agcExtLevel;               /* in 1/10 percent */
    int32_t carrierFreqOffset;         /* in 1/1000 Hz */
    int32_t carrierPhaseOffset;        /* in 1/1000 Hz */
    uint32_t uncorrectedCount;         /* not self-clearing  */
    uint32_t correctedCount;           /* not self-clearing*/
    uint32_t berErrorCount;	           /* not self-clearing */
    uint32_t fdcChannelPower;          /* Only valid if using DS0 for OOB! in 1/10 dBmV unit; OCAP DPM support for OOB channel */	
} BDCM_AobStatus;

/***************************************************************************
Summary:
	Required default settings structure for QPSK Out-of_Band module.
****************************************************************************/
#define BDCM_AOB_XTALFREQ					(24000000)	/* 24.00 MHz */

#define BDCM_AOB_IFFREQ					(44000000)	/* 44.00 MHz */

typedef struct BDCM_AobSettings
{
    uint32_t xtalFreq;                  /* Crystal Freqency in Hertz */
    unsigned long ifFreq;               /* IF Frequency in Hertz */
    bool enableFEC;                     /* use OOB FEC or not */
    BDCM_AobSpectrumMode spectrum;   /* default Spectrum setting*/
} BDCM_AobSettings;


typedef enum BDCM_AobCallback
{
	BDCM_AobCallback_eLockChange, /* Callback to notify application of lock change */
	BDCM_AobCallback_eLast        
} BDCM_AobCallback;

/***************************************************************************
Summary:
    Callback used for event notification to the upper layer.
****************************************************************************/
typedef BERR_Code (*BDCM_AobCallbackFunc)(void *pParam );


/***************************************************************************
Summary:
    This function returns the default settings for QPSK Out-of_Band module.
****************************************************************************/
BERR_Code BDCM_Aob_GetChannelDefaultSettings(
    BDCM_AobSettings *pSetttings		/* [out] Returns default setting */
    );

/***************************************************************************
Summary:
    This function opens a DOCSIS OOB channel.
****************************************************************************/
BDCM_AobChannelHandle BDCM_Aob_OpenChannel(
    void *handle,   						  /* [in] RPC handle */ 
    const BDCM_AobSettings *pOpenSettings  /* [in] open settings */
    );

/***************************************************************************
Summary:
    This function closes a DOCSIS OOB channel.
****************************************************************************/
BERR_Code BDCM_Aob_CloseChannel(
    BDCM_AobChannelHandle hChannel   /* [in] Channel handle */
    );

/***************************************************************************
Summary:
    This function tries to acquire a DOCSIS OOB channel
    and will automatically enable the channel it was in power-saver mode.
****************************************************************************/
BERR_Code BDCM_Aob_AcquireChannel(
    BDCM_AobChannelHandle hChannel, /* [in] Channel handle */
    BDCM_AobModulationType modType, /* [in] Modulation type to use */
    uint32_t symbolRate				   /* [in] Symbol rate to use */
    );

/***************************************************************************
Summary:
    This function gets the current QPSK spectrum settings for a DOCSIS
    OOB channel.
****************************************************************************/
BERR_Code BDCM_Aob_GetChannelSpectrum(
    BDCM_AobChannelHandle hChannel, /* [in]  Channel handle */
    BDCM_AobSpectrumMode *spectrum  /* [out] Spectrum setting */
    );

/***************************************************************************
Summary:
    This function sets the current QPSK spectrum settings for a DOCSIS
    OOB channel.
****************************************************************************/
BERR_Code BDCM_Aob_SetChannelSpectrum(
    BDCM_AobChannelHandle hChannel,  /* [in] Channel handle */
    BDCM_AobSpectrumMode spectrum    /* [in] Spectrum setting */
    );

/***************************************************************************
Summary:
    This function gets the status of a DOCSIS OOB channel.
****************************************************************************/
BERR_Code BDCM_Aob_GetChannelStatus(
	BDCM_AobChannelHandle hChannel, /* [in] Channel handle */
	BDCM_AobStatus *pStatus         /* [out] Status */
    );

/***************************************************************************
Summary:
    This function gets the lock status for a DOCSIS OOB channel.
****************************************************************************/
BERR_Code BDCM_Aob_GetChannelLockStatus(
    BDCM_AobChannelHandle hChannel,   /* [in] Channel handle */
    bool *isLock                         /* [out] true=locked false=unlocked */
    );

/***************************************************************************
Summary:
    This function enable the power-saver mode.
***************************************************************************/
BERR_Code BDCM_Aob_EnableChannelPowerSaver(
    BDCM_AobChannelHandle hChannel  /* [in] Channel handle */
    );

/***************************************************************************
Summary:
    This function is responsible for processing a notificiation for the specific
    Qam Out-of-Band Downstream module channel.
****************************************************************************/
BERR_Code BDCM_Aob_ProcessChannelNotification(
    BDCM_AobChannelHandle hChannel, /* [in] Channel handle */
    unsigned int event                 /* [in] Event code and event data*/
    );

/***************************************************************************
Summary:
    This function is responsible for installing a callback function.
***************************************************************************/
BERR_Code BDCM_Aob_InstallChannelCallback(
    BDCM_AobChannelHandle hChannel,  /* [in] Channel handle */
    BDCM_AobCallback callbackType,   /* [in] Type of callback */
    BDCM_AobCallbackFunc pCallback,  /* [in] Function Ptr to callback */
    void *pParam                        /* [in] Generic parameter send on callback */
    );

#ifdef __cplusplus
}
#endif

#endif

