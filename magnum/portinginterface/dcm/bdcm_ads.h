/***************************************************************************
 *     Copyright (c) 2012-2013, Broadcom Corporation
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
#ifndef BDCM_ADS_H__
#define BDCM_ADS_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    A handle for a DOCSIS downstream channel.
****************************************************************************/
typedef struct BDCM_AdsChannel         *BDCM_AdsChannelHandle;



/***************************************************************************
Summary:
    Enumeration for Interleaver

Description:
    This enumeration defines the different Interleaver types.

See Also:

****************************************************************************/
typedef enum BDCM_Ads_Interleaver
{
    BADS_Interleaver_eI128_J1,
    BADS_Interleaver_eI128_J2,
    BADS_Interleaver_eI128_J3,
    BADS_Interleaver_eI128_J4,
    BADS_Interleaver_eI64_J2,
    BADS_Interleaver_eI32_J4,
    BADS_Interleaver_eI16_J8,
    BADS_Interleaver_eI8_J16,
    BADS_Interleaver_eI4_J32,
    BADS_Interleaver_eI2_J64,
    BADS_Interleaver_eI1_J128,
    BADS_Interleaver_eI12_J17,
    BADS_Interleaver_eUnsupported,
    BADS_Interleaver_eLast
} BDCM_Ads_Interleaver;

typedef enum BDCM_Ads_AcquisitionStatus
{
    BADS_AcquisitionStatus_eNoSignal,
    BADS_AcquisitionStatus_eUnlocked,
    BADS_AcquisitionStatus_eLockedFast,
    BADS_AcquisitionStatus_eLockedSlow,
    BADS_AcquisitionStatus_eLast
} BDCM_Ads_AcquisitionStatus;


/***************************************************************************
Summary:
    Enumeration for modulation type
****************************************************************************/
typedef enum BDCM_AdsModulationType
{
    BDCM_AdsModulationType_eAnnexAQam16,
    BDCM_AdsModulationType_eAnnexAQam32,
    BDCM_AdsModulationType_eAnnexAQam64,
    BDCM_AdsModulationType_eAnnexAQam128,
    BDCM_AdsModulationType_eAnnexAQam256,
    BDCM_AdsModulationType_eAnnexAQam512,
    BDCM_AdsModulationType_eAnnexAQam1024,
    BDCM_AdsModulationType_eAnnexAQam2048,
    BDCM_AdsModulationType_eAnnexAQam4096,
    BDCM_AdsModulationType_eAnnexBQam16,
    BDCM_AdsModulationType_eAnnexBQam32,
    BDCM_AdsModulationType_eAnnexBQam64,
    BDCM_AdsModulationType_eAnnexBQam128,
    BDCM_AdsModulationType_eAnnexBQam256,
    BDCM_AdsModulationType_eAnnexBQam512,
    BDCM_AdsModulationType_eAnnexBQam1024,
    BDCM_AdsModulationType_eAnnexBQam2048,
    BDCM_AdsModulationType_eAnnexBQam4096,
    BDCM_AdsModulationType_eAnnexCQam16,
    BDCM_AdsModulationType_eAnnexCQam32,
    BDCM_AdsModulationType_eAnnexCQam64,
    BDCM_AdsModulationType_eAnnexCQam128,
    BDCM_AdsModulationType_eAnnexCQam256,
    BDCM_AdsModulationType_eAnnexCQam512,
    BDCM_AdsModulationType_eAnnexCQam1024,
    BDCM_AdsModulationType_eAnnexCQam2048,
    BDCM_AdsModulationType_eAnnexCQam4096,
    BDCM_AdsModulationType_eLast
} BDCM_AdsModulationType;

/***************************************************************************
Summary:
    Callback used for notifying the upper layer about the relevant events.
****************************************************************************/
typedef BERR_Code (*BDCM_AdsCallbackFunc)(void *pParam );

/***************************************************************************
Summary:
    Enumeration for Callback types
****************************************************************************/
typedef enum BDCM_AdsCallback
{
    BDCM_AdsCallback_eLockChange,          /* Callback to notify application of lock change */
    BDCM_AdsCallback_eUpdateGain,          /* Callback to notify application to update gain */ 
    BDCM_AdsCallback_eNoSignal,            /* Callback to notify application there is no signal */ 
    BDCM_AdsCallback_eAsyncStatusReady,    /* Callback to notify application there is no signal */    
    BDCM_AdsCallback_eTuner,               /* Callback to tuner regarding a new setting */ 
    BDCM_AdsCallback_eSpectrumDataReady,   /* Callback to notify application that spectrum analyzer data is ready */ 
    BDCM_AdsCallback_eLast                 /* More may be required */
} BDCM_AdsCallback;

/***************************************************************************
Summary:
    Enumeration for Acquire types
****************************************************************************/
typedef enum BDCM_AdsAcquireType
{
	BDCM_AdsAcquireType_eAuto,
	BDCM_AdsAcquireType_eFast,
	BDCM_AdsAcquireType_eSlow,
	BDCM_AdsAcquireType_eScan,
	BDCM_AdsAcquireType_eLast
}BDCM_AdsAcquireType;

/***************************************************************************
Summary:
    Enumeration for Lock Status
****************************************************************************/
typedef enum BDCM_AdsLockStatus
{           
    BDCM_AdsLockStatus_eUnlocked,
    BDCM_AdsLockStatus_eLocked, 
    BDCM_AdsLockStatus_eNoSignal,      
    BDCM_AdsLockStatus_eLast    
} BDCM_AdsLockStatus;

/***************************************************************************
Summary:
    This structure represents DOCSIS (QAM) downstream acquisition parameters
****************************************************************************/
typedef struct BDCM_AdsInbandParam
{
    BDCM_AdsModulationType modType;        /* Modulation type */
    uint32_t            symbolRate;             /* in Baud, for 3128/3461 this is only valid for Annex A  */
    bool                autoAcquire;
    bool                enableNullPackets; /* Enables/disables improved locking mechanism for Annex_A signals containing >98% null packets.*/
    unsigned            frequencyOffset; /*   Automatic frequency Offset (pull-in) range of the qam demodulator 
                                                        For example "frequencyOffset = 180000" can pull in carriers offset +/-180KHz   from the tuned frequency */
    BDCM_AdsAcquireType	acquisitionType;
    bool                tuneAcquire; /* This bit directs the Downstream Demodulator to initiate an acquire immediately after its associated WFE or Tuner is tuned.  
                                        If the bit is set, then the Demodulator will initiate an acquire immediately after the tuner has completed a tune  regardless of autoacquire. 	
                                        Note: Unless otherwise indicated, this flag only applies to the Acquisition Processors Internal WFE or Internal Tuner. */
} BDCM_AdsInbandParam;

/***************************************************************************
Summary:
    This structure represents status of a DOCSIS (QAM) downstream channel.
****************************************************************************/
typedef struct BDCM_AdsStatus
{
    bool isPowerSaverEnabled;           /* Eanble=1, Disable=0 */
    BDCM_AdsModulationType modType;        /* Modulation type */
    uint32_t ifFreq;                    /* in Hertz, IF freq. */
    uint32_t symbolRate;                /* in Baud */
    int32_t  symbolRateError;            /* symbol rate error in Baud */
    bool isFecLock;                     /* lock=1, unlock=0 */
    bool isQamLock;                     /* lock=1, unlock=0 */
    uint32_t correctedCount;            /* reset on every read */
    uint32_t uncorrectedCount;          /* reset on every read */
    int32_t snrEstimate;                /* in 1/256 dB */
    int32_t agcIntLevel;                /* in 1/10 percent */
    int32_t agcExtLevel;                /* in 1/10 percent */
    int32_t carrierFreqOffset;          /* in 1/1000 Hz */
    int32_t carrierPhaseOffset;         /* in 1/1000 Hz */
    uint32_t rxSymbolRate;              /* in Baud, received symbol rate */
    uint16_t interleaveDepth;           /* use in Docsis */
    uint32_t goodRsBlockCount;          /* reset on every read */
    uint32_t berRawCount;               /* reset on every read */
    int32_t dsChannelPower;             /* in 1/10th of a dBmV unit; OCAP DPM support for video channels */
    uint32_t mainTap;                   /* Channel main tap coefficient */
    int32_t feGain;                     /* in 1/100th of a dB */
    int32_t digitalAgcGain;             /* in 1/100th of a dB */    
    uint32_t equalizerGain;             /* Channel equalizer gain value in in 1/100th of a dB */
    /* OCAP required postRsBER for all DS channels. postRsBER and elapsedTimeSec will be reset on every channel change*/
    /* fpostRsBER normally between xxxe-6 to xxxe-12 float value, to send this small float number over rMagnum to host, we convert it to uint32 using the formula*/
    uint32_t postRsBER;                 /* Converted floating point fpostRsBER --> uint32_t postRsBER for rMagnum transport: */
                                        /* int ipostRsBER = (int)( log(fpostRsBER) * 1000000.0 ); uint32_t postRsBER = (uint32_t)ipostRsBER; */
                                        /* Host side will need this to convert it back: int ipostRsBER = (int)postRsBER; float fpostRsBER = exp( (float)ipostRsBER/1000000.0 ); */
    uint32_t elapsedTimeSec;            /* postRsBER over this time */
    bool isSpectrumInverted;            /* If 1, Spectrum is inverted relative to the AP bootup. */
    uint32_t preRsBER;                  /* Convert floating point fpreRsBER --> uint32_t preRsBER for rMagnum transport: */
	                                    /* int ipreRsBER = (int)( log(fpreRsBER) * 1000000.0 ); uint32_t preRsBER = (uint32_t)ipostRsBER; */
	                                    /* Host side will need this to convert it back: int ipreRsBER = (int)preRsBER; float fpreRsBER = exp( (float)ipreRsBER/1000000.0 ); */
    uint32_t cleanCount;                /* reset on every reset status*/
    int32_t agcAGFLevel;                /* in 1/10 percent */
    int32_t agcAGF2Level;               /* in 1/10 percent */
    uint32_t correctedBits;             /* reset on every reset status */
    uint32_t accCorrectedCount;         /* Accumulated corrected block count. Reset on every reset status */
    uint32_t accUncorrectedCount;       /* Accumulated un corrected block count. Reset on every reset status */
    uint32_t accCleanCount;             /* Accumulated clean block count. Reset on every reset status */
} BDCM_AdsStatus;

#define BDCM_ADS_IFFREQ                    (43750000)      /* 43.75 MHz */

/***************************************************************************
Summary:
    This structure represents the settings for a DOCSIS downstream channel.
****************************************************************************/
typedef struct BDCM_AdsSettings
{
    unsigned long ifFreq; /* IF Frequency in Hertz */
    bool autoAcquire;     /* does auto-reacuire or not*/
    bool fastAcquire;     /* uses 2X faster acquire time
                            in expense of stability or not*/
    uint32_t    minVer; /* minor chip revision number */
} BDCM_AdsSettings;




/***************************************************************************
Summary:
    Scan Parameter settings.

Description:

See Also:
    BADS_SetScanParam()

****************************************************************************/
typedef struct BDCM_Ads_ChannelScanSettings
{
    bool		AI;			/* Enable/Disable auto invert spectrum */
    bool		QM;			/* Enable/Disable QAM mode search  */
    bool		CO;			/* Enable/Disable Carrier Offset search  */
    bool		TO;			/* Enable/Disable auto baud rate detection  */
    bool		B1024;		/* Enable/Disable Auto Search for 1024Q while receiver is in Annex B mode */
    bool		B256;		/* Enable/Disable Auto Search for 256Q while receiver is in Annex B mode */
    bool		B64;
    bool		A1024;		 /* Enable/Disable Auto Search for 1024Q while receiver is in Annex A mode  */
    bool		A512;
    bool		A256;
    bool		A128;
    bool		A64;
    bool		A32;
    bool		A16;		/* Enable/Disable Auto Search for 16Q while receiver is in Annex A mode */
    uint32_t    carrierSearch;		/* 256*Hz */
    uint32_t    upperBaudSearch;  /*upper baud search range in Hz */
    uint32_t    lowerBaudSearch; /*Lower baud search range in Hz */
} BDCM_Ads_ChannelScanSettings;

typedef struct BDCM_Ads_ScanStatus
{
    BDCM_Ads_AcquisitionStatus acquisitionStatus; /* Acquisition Status (NoSignal, Unlocked, LockedFast, LockedSlow)  */
    BDCM_AdsModulationType modType;        /* Modulation type */
    bool isSpectrumInverted;            /* If 1, Spectrum is inverted. */
    bool autoAcquire;                   /* If 1, autoAcquire is enabled */
    BDCM_Ads_Interleaver interleaver;	    /* FEC interleaver */
    uint32_t symbolRate;                /* symbol rate in Baud */
    int32_t carrierFreqOffset;          /* carrier frequency offset in 1/1000 Hz */
} BDCM_Ads_ScanStatus;

/***************************************************************************
Summary:
	This function opens a DOCSIS (QAM) channel
****************************************************************************/
BDCM_AdsChannelHandle BDCM_Ads_OpenChannel(
    void *handle,                             /* [in] Device Handle */
    unsigned int channelNo,				                     /* [in] Channel number to open */
    const struct BDCM_AdsSettings *pSettings      /* [in] Channel default setting */
    );

/***************************************************************************
Summary:
	This function closes a DOCSIS (QAM) channel.
****************************************************************************/
BERR_Code BDCM_Ads_CloseChannel(
    BDCM_AdsChannelHandle hChannel /* [in] Channel handle */
    );

/***************************************************************************
Summary:
	This function gets default setting for a Qam In-Band Downstream module channel.
****************************************************************************/
BERR_Code BDCM_Ads_GetChannelDefaultSettings(
    BDCM_AdsSettings *pSettings /* [out] Returns channel default setting */
    );

/***************************************************************************
Summary:
    This function gets requests a sync status of a DOCSIS (QAM) channel.
    Whenever the status is ready, DOCSIS shall send a notification to
    host.
****************************************************************************/
BERR_Code BDCM_Ads_RequestAsyncChannelStatus(
    BDCM_AdsChannelHandle hChannel
    );

/***************************************************************************
Summary:
	This function gets the status of a DOCSIS (QAM) channel.

****************************************************************************/
BERR_Code BDCM_Ads_GetChannelStatus(
    BDCM_AdsChannelHandle hChannel, /* [in] Channel handle */
    BDCM_AdsStatus *pStatus		   /* [out] Returns status */
    );

/***************************************************************************
Summary:
	This function resets the accumulated status of a DOCSIS (QAM) channel
****************************************************************************/
BERR_Code BDCM_Ads_ResetChannelStatus(
    BDCM_AdsChannelHandle hChannel /* [in] Channel handle */
    );

/***************************************************************************
Summary:
    This function gets the lock status for a DOCSIS (QAM) channel.
****************************************************************************/
BERR_Code BDCM_Ads_GetChannelLockStatus(
    BDCM_AdsChannelHandle hChannel,   /* [in]  Channel handle */
    BDCM_AdsLockStatus *pLockStatus   /* [out] Returns lock status */
    );

/***************************************************************************
Summary:
    This function gets the I and Q values for soft decision of a
    (QAM) DOCSIS channel.
****************************************************************************/
BERR_Code BDCM_Ads_GetChannelSoftDecision(
	BDCM_AdsChannelHandle hChannel, /* [in] Channel handle */
	int16_t nbrToGet,                  /* [in] Number values to get */
	int16_t *ival,                     /* [out] Ptr to array to store output I soft decision */
	int16_t *qVal,                     /* [out] Ptr to array to store output Q soft decision */
	int16_t *nbrGotten                 /* [out] Number of values gotten/read */
	);


/***************************************************************************
Summary:
	This function installs a callback function for Lock State Change event.
    The application code should use this function to install a callback function,
    which will be called when a DOCSIS QAM channel's lock state changes.
	A lock state change is defined at switching from Lock-Unlock or Unlock-Lock.
	To determine the current lock state, a call to BDCM_Ads_GetChannelLockStatus() is
	required. To get a more detailed status, call BDCM_Ads_GetChannelStatus().
	Note: It is "highly" recommended that the callback function do the minimum
	require to notify the application of this event, such sent a message or
	fire an event.  This callback function may be called from an
	interrupt context.  Please use with caution.
****************************************************************************/
BERR_Code BDCM_Ads_InstallChannelCallback(
	BDCM_AdsChannelHandle channel,	    /* [in] Channel handle */
	BDCM_AdsCallback callbackType,		/* [in] type of Callback */
	BDCM_AdsCallbackFunc pCallbackFunc,	/* [in] Pointer to completion callback. */
	void *pParam						    /* [in] Pointer to callback user data. */
	);

/***************************************************************************
Summary:
    This function tries to acquire DOCSIS (QAM) downStream channel lock for a
    specific channel which will automatically enable the downstream channel
    if the channel was in power-saver mode.
***************************************************************************/
BERR_Code BDCM_Ads_AcquireChannel(
	BDCM_AdsChannelHandle hChannel, /* [in] Channel handle */
	BDCM_AdsInbandParam *ibParam    /* [in] Inband Parameters to use */
	);

/***************************************************************************
Summary:
	This function enable the power-saver mode.

Description:
	This function is responsible for enabling downstream (QAM)channel
    power-saver mode. When a channel is in the power-saver mode, the channel
    is shutdown.
****************************************************************************/
BERR_Code BDCM_Ads_EnableChannelPowerSaver(
	BDCM_AdsChannelHandle hChannel /* [in] Channel handle */
	);

/***************************************************************************
Summary:
    This function is responsible for processing a notificiation for a
    specific DOCSIS QAM downstream channel and it should be called when a
    notification is received.
****************************************************************************/
BERR_Code BDCM_Ads_ProcessChannelNotification(
	BDCM_AdsChannelHandle hChannel,  /* [in] Channel handle */
	unsigned int event                  /* [in] Event code and event data*/
	);


/***************************************************************************
Summary:
    This function tries to acquire DOCSIS (QAM) downStream channel lock for a
    specific channel which will automatically enable the downstream channel
    if the channel was in power-saver mode.
***************************************************************************/
BERR_Code BDCM_Ads_SetScanParam(
	BDCM_AdsChannelHandle hChannel, /* [in] Channel handle */
	BDCM_Ads_ChannelScanSettings *scanParam    /* [in] Inband Parameters to use */
	);

/***************************************************************************
Summary:
    This function gets the lock status for a DOCSIS (QAM) channel.
****************************************************************************/
BERR_Code BDCM_Ads_GetScanStatus(
    BDCM_AdsChannelHandle hChannel,            /* [in] Device channel handle */
    BDCM_Ads_ScanStatus *pScanStatus        /* [out] Returns status */
    );
#endif

#ifdef __cplusplus
}


#endif
