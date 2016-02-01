/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Frontend QAM
*    Generic APIs for QAM tuning.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_FRONTEND_QAM_H__
#define NEXUS_FRONTEND_QAM_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
QAM tuning modes
***************************************************************************/
typedef enum NEXUS_FrontendQamMode
{
    NEXUS_FrontendQamMode_e16,
    NEXUS_FrontendQamMode_e32,
    NEXUS_FrontendQamMode_e64,
    NEXUS_FrontendQamMode_e128,
    NEXUS_FrontendQamMode_e256,
    NEXUS_FrontendQamMode_e512,
    NEXUS_FrontendQamMode_e1024,
    NEXUS_FrontendQamMode_e2048,
    NEXUS_FrontendQamMode_e4096,
    NEXUS_FrontendQamMode_eAuto_64_256, /* Automatically scan both QAM-64 and QAM-256.
                                           Not available on all chipsets. */
    NEXUS_FrontendQamMode_eMax
} NEXUS_FrontendQamMode;

/***************************************************************************
Summary:
QAM Annex
***************************************************************************/
typedef enum NEXUS_FrontendQamAnnex
{
    NEXUS_FrontendQamAnnex_eA,
    NEXUS_FrontendQamAnnex_eB,
    NEXUS_FrontendQamAnnex_eC,
    NEXUS_FrontendQamAnnex_eMax
} NEXUS_FrontendQamAnnex;

/***************************************************************************
Summary:
Determines if QAM spectrum mode.
***************************************************************************/
typedef enum NEXUS_FrontendQamSpectrumMode
{
    NEXUS_FrontendQamSpectrumMode_eAuto,
    NEXUS_FrontendQamSpectrumMode_eManual,
    NEXUS_FrontendQamSpectrumMode_eMax
} NEXUS_FrontendQamSpectrumMode;

/***************************************************************************
Summary:
Determines if QAM spectral inversion is enabled or not
***************************************************************************/
typedef enum NEXUS_FrontendQamSpectralInversion
{
    NEXUS_FrontendQamSpectralInversion_eNormal,
    NEXUS_FrontendQamSpectralInversion_eInverted,
    NEXUS_FrontendQamSpectralInversion_eMax
} NEXUS_FrontendQamSpectralInversion;

/***************************************************************************
Summary:
Determines the acquisition type.
***************************************************************************/
typedef enum NEXUS_FrontendQamAcquisitionMode
{
    NEXUS_FrontendQamAcquisitionMode_eAuto,
    NEXUS_FrontendQamAcquisitionMode_eFast,
    NEXUS_FrontendQamAcquisitionMode_eSlow,
    NEXUS_FrontendQamAcquisitionMode_eScan,
    NEXUS_FrontendQamAcquisitionMode_eMax
} NEXUS_FrontendQamAcquisitionMode;

/***************************************************************************
The NEXUS_FrontendQamBandwidth enum has been deprecated. It is an unsigned integer in units of Hz now.
The following #defines are for backward compatibility only.
***************************************************************************/
#define NEXUS_FrontendQamBandwidth unsigned
#define NEXUS_FrontendQamBandwidth_e5Mhz 5000000
#define NEXUS_FrontendQamBandwidth_e6Mhz 6000000
#define NEXUS_FrontendQamBandwidth_e7Mhz 7000000
#define NEXUS_FrontendQamBandwidth_e8Mhz 8000000
#define NEXUS_FrontendQamBandwidth_eMax  4

/***************************************************************************
The NEXUS_FrontendQamFrequencyOffset enum has been deprecated. It is an unsigned integer in units of Hz now.
The following #defines are for backward compatibility only.
***************************************************************************/
#define NEXUS_FrontendQamFrequencyOffset unsigned
#define NEXUS_FrontendQamFrequencyOffset_e125Khz 125000
#define NEXUS_FrontendQamFrequencyOffset_e180Khz 180000 /* Applies only to Annex_B. */
#define NEXUS_FrontendQamFrequencyOffset_e200Khz 200000 /* Applies only to Annex_A. */
#define NEXUS_FrontendQamFrequencyOffset_e250Khz 250000
#define NEXUS_FrontendQamFrequencyOffset_eMax 4

/***************************************************************************
Summary:
QAM tuning parameters
***************************************************************************/
typedef struct NEXUS_FrontendQamSettings
{
    NEXUS_FrontendQamMode mode;
    NEXUS_FrontendQamAnnex annex;
    unsigned frequency;                 /* In Hz */
    unsigned ifFrequency;               /* IF Frequency in Hz */
    unsigned symbolRate;                /* In Baud. The default value is 0, which causes Nexus to select default symbol rates based on the QamMode. */
    bool fastAcquisition;               /* True=fast acquisition mode, false=default */
    bool terrestrial;                   /* Receiver mode: true=terrestrial, false=cable */
    NEXUS_CallbackDesc lockCallback;    /* Lock changed callback. This is called when demod locks or unlocks. You must get status to determine lock status.
                                           After tuning to a new frequency, it is normal to get two callbacks: one because the previous freq was unlocked
                                           and a second when the new freq is locked. The first callback and its unlocked status does not mean that the new
                                           freq will not be acquired later. Because callbacks can be collapsed under system load, an application should not 
                                           count callbacks. Instead, it should only rely on what get status returns. See tune_qam.c for an example algorithm. */
    bool autoAcquire;                   /* True = Enable, False = Disable. */
    unsigned bandwidth;                 /* Tuner frequency bandwidth in Hz */
    bool enablePowerMeasurement;        /* True = enables calculation of downstream channel power for some frontends like 31xx. For others it is always enabled. False = Disable. */
    NEXUS_FrontendQamSpectrumMode spectrumMode; /* Spectrum mode. */
    NEXUS_FrontendQamSpectralInversion spectralInversion; /* Spectral inversion. */
    unsigned frequencyOffset; /* Automatic Frequency offset range from the tuned frequency in Hz. */
    bool enableNullPackets; /* Enables/disables improved locking mechanism for Annex_A signals containing >98% null packets.*/
    NEXUS_CallbackDesc asyncStatusReadyCallback;   /* Callback will be called when the async qam status is ready. */
    NEXUS_FrontendQamAcquisitionMode acquisitionMode; /* Acquisition mode. */
    struct {
        unsigned    upperBaudSearch;    /* Upper baud search range in Hz. Applicable only in scan mode. */ 
        unsigned    lowerBaudSearch;    /*Lower baud search range in Hz. Applicable only in scan mode. */
        bool        mode[NEXUS_FrontendQamAnnex_eMax][NEXUS_FrontendQamMode_eMax]; /* Enable/Disable QAM mode search in scan mode. */
        unsigned    frequencyOffset;    /* in NEXUS_FrontendQamAcquisitionMode_eAuto, frontend uses non-scan frequencyOffset, then uses the scan.frequencyOffset. */
    } scan; /* only applies if NEXUS_FrontendQamAcquisitionMode_eScan (or eAuto mode when it internally goes into eScan mode) */
} NEXUS_FrontendQamSettings;

/***************************************************************************
Summary:
    Initialize a QAM settings structure to defaults
***************************************************************************/
void NEXUS_Frontend_GetDefaultQamSettings(
    NEXUS_FrontendQamSettings *pSettings
    );

/***************************************************************************
Summary:
    Tune to a QAM channel

Description:
    After this function is called, a new unlock callback will be generated. The the frontend tunes and acquires to the requested frequency and other settings.
    Another callback will be issued to indicate a lock/unlock or no_signal condition. The application needsto call NEXUS_Frontend_GetFastStatus() to determine the outcome.
    Sometimes, the two callbacks can be collapsed into one. In this case, the application only receives just one callback.
    Here, the application can check the "acquireInProgress" member of the NEXUS_FrontendFastStatus structure to determine if the first run of the tune and acquisition is complete or not.
    In case of unlock, if the auto acquire is set to true in NEXUS_FrontendQamSettings, the application might receive another callback only if it happens to lock to a signal after more than one reacquires.

Note:
    See NEXUS_Frontend_Untune to reduce power with possible performance cost.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_TuneQam(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendQamSettings *pSettings
    );

/**
Summary:
Bit error rate (BER) units

Description:
BER is the fraction of bits that are errors. 0.3 means 30% of bits are errors.
Various frontends report BER with different integer units which can be converted to a standard floating point representation in the application.
**/
typedef enum NEXUS_FrontendErrorRateUnits
{
    NEXUS_FrontendErrorRateUnits_eLinear, /* 1/2147483648 or 1/(2^31) units. Convert to floating point: float fBer = (float)ber / 2147483648.0. */
    NEXUS_FrontendErrorRateUnits_eNaturalLog, /* (log(ber) * 1000000.0) units. Convert to floating point: float fBer = exp((float)ber/1000000.0). */
    NEXUS_FrontendErrorRateUnits_eMax
} NEXUS_FrontendErrorRateUnits;

/***************************************************************************
Summary:
QAM tuning status
***************************************************************************/
typedef struct NEXUS_FrontendQamStatus
{
    NEXUS_FrontendQamSettings settings; /* Settings provided at last call to NEXUS_Frontend_TuneQam */

    bool receiverLock;          /* Do we have QAM lock? */
    bool fecLock;               /* Is the FEC locked? */
    bool opllLock;              /* Is the output PLL locked? */
    bool spectrumInverted;      /* Is the spectrum inverted? */

    unsigned symbolRate;        /* Baud. received symbol rate (in-band) */
    int      symbolRateError;   /* symbol rate error in Baud */

    int berEstimate;            /* deprecated */

    unsigned ifAgcLevel;        /* IF AGC level in units of 1/10 percent */
    unsigned rfAgcLevel;        /* tuner AGC level in units of 1/10 percent */
    unsigned intAgcLevel;       /* Internal AGC level in units of 1/10 percent */
    unsigned snrEstimate;       /* snr estimate in 1/100 dB (in-Band). */

    unsigned fecCorrected;      /* FEC corrected block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned fecUncorrected;    /* FEC uncorrected block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned fecClean;          /* FEC clean block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned bitErrCorrected;   /* deprecated: cumulative bit correctable errors. same as viterbiUncorrectedBits. */
    unsigned reacquireCount;    /* cumulative reacquisition count */

    unsigned viterbiUncorrectedBits; /* uncorrected error bits output from Viterbi, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned viterbiTotalBits;       /* total number of bits output from Viterbi, accumulated since tune or NEXUS_Frontent_ResetStatus */
    uint32_t viterbiErrorRate;  /* Viterbi or pre-Reed-Solomon bit error rate (preRsBer). For units see errorRateUnits.
                                   This is the ratio of uncorrected bits / total bits since the last GetQamStatus.
                                   For a Docsis frontend, units are NEXUS_FrontendErrorRateUnits_eNaturalLog and this value is valid since the last tuner lock. */
    NEXUS_FrontendErrorRateUnits errorRateUnits; /* units for viterbiErrorRate and postRsBer. Default is NEXUS_FrontendErrorRateUnits_eLinear. */

    int      carrierFreqOffset; /* carrier frequency offset in 1/1000 Hz */
    int      carrierPhaseOffset;/* carrier phase offset in 1/1000 Hz */

    unsigned goodRsBlockCount;  /* reset on every read */
    unsigned berRawCount;       /* reset on every read */

    int      dsChannelPower;    /* units of 1/10 dBmV */
    unsigned mainTap;           /* Channel main tap coefficient */
    unsigned postRsBer;         /* post reed-solomon bit error rate. Same behavior as viterbiErrorRate. For units see errorRateUnits. */
    unsigned postRsBerElapsedTime; /* time used in postRsBer calculation. In units of milliseconds. */
    uint16_t interleaveDepth;   /* Used in DOCSIS */
    unsigned lnaAgcLevel;       /* LNA AGC level in units of 1/10 percent */
    
    int equalizerGain;          /* Channel equalizer gain value in dB */
    int frontendGain;           /* in 1/100th of a dB. Is the accumulated gain of the tuner/ufe/wfe. */
    int digitalAgcGain;         /* in 1/100th of a dB. Is the AGC gain in the demod core. */
    int highResEqualizerGain;   /* in 1/100th of a dB. Is the equalizer gain in the demod core. higher resolution than previous equalizerGain. */
} NEXUS_FrontendQamStatus;

/***************************************************************************
Summary:
    Get the synchronous status of a QAM tuner
***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetQamStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendQamStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
    Request the asynchronous status of a QAM tuner
***************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestQamAsyncStatus(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
Summary:
    Get the asynchronous status of a QAM tuner
***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetQamAsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendQamStatus *pStatus /* [out] */
    );

typedef enum NEXUS_FrontendQamAcquisitionStatus
{
    NEXUS_FrontendQamAcquisitionStatus_eNoSignal,
    NEXUS_FrontendQamAcquisitionStatus_eNoLock,
    NEXUS_FrontendQamAcquisitionStatus_eLockedFast,
    NEXUS_FrontendQamAcquisitionStatus_eLockedSlow,
    NEXUS_FrontendQamAcquisitionStatus_eMax
}NEXUS_FrontendQamAcquisitionStatus;

typedef enum NEXUS_FrontendQamInterleaver
{
    NEXUS_FrontendQamInterleaver_eI128_J1,
    NEXUS_FrontendQamInterleaver_eI128_J2,
    NEXUS_FrontendQamInterleaver_eI128_J3,
    NEXUS_FrontendQamInterleaver_eI128_J4,
    NEXUS_FrontendQamInterleaver_eI64_J2,
    NEXUS_FrontendQamInterleaver_eI32_J4,
    NEXUS_FrontendQamInterleaver_eI16_J8,
    NEXUS_FrontendQamInterleaver_eI8_J16,
    NEXUS_FrontendQamInterleaver_eI4_J32,
    NEXUS_FrontendQamInterleaver_eI2_J64,
    NEXUS_FrontendQamInterleaver_eI1_J128,
    NEXUS_FrontendQamInterleaver_eI12_J17,
    NEXUS_FrontendQamInterleaver_eUnsupported,
    NEXUS_FrontendQamInterleaver_eMax
}NEXUS_FrontendQamInterleaver;

/***************************************************************************
Summary:
QAM scan status
***************************************************************************/
typedef struct NEXUS_FrontendQamScanStatus
{
    unsigned symbolRate;                      /* Detected symbol rate. */
    int frequencyOffset;                      /* Detected carrier frequency offset.  */
    NEXUS_FrontendQamMode mode;               /* Detected QAM mode.  */
    NEXUS_FrontendQamAnnex annex;             /* Detected annex. */
    NEXUS_FrontendQamInterleaver interleaver; /* Detected interleaver. */
    NEXUS_FrontendQamSpectralInversion spectrumInverted;  /* Is spectrum inverted? */
    NEXUS_FrontendQamAcquisitionStatus acquisitionStatus; /* Acquisition status. */
} NEXUS_FrontendQamScanStatus;


/***************************************************************************
Summary:
    Get the scan status of a QAM tuner
Description:
    For scan mode (NEXUS_Frontend_TuneQam with mode set to NEXUS_FrontendQamAcquisitionType_eScan),
    initially NEXUS_Frontend_GetFastStatus can be used to determine the lock status faster.
    Once locked, NEXUS_Frontend_GetQamScanStatus can be used to get the complete scan status.
    This way the scan time can be improved vs calling NEXUS_Frontend_GetQamScanStatus everytime.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetQamScanStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendQamScanStatus *pScanStatus /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_QAM_H__ */

