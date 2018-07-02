/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to
*  the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied),
*  right to use, or waiver of any kind with respect to the Software, and
*  Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
*  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
*  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* API Description:
*   API name: Frontend OutOfBand
*    Generic APIs for Out-of-band tuning.
*
***************************************************************************/
#ifndef NEXUS_FRONTEND_OOB_H__
#define NEXUS_FRONTEND_OOB_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Out of band tuning modes
***************************************************************************/
#define NEXUS_FrontendOutOfBandMode_ePod_Dvs167Qpsk  NEXUS_FrontendOutOfBandMode_ePod_AnnexAQpsk /* Changed to use "#define" to eliminate Coverity MIXED_ENUM issue*/
#define NEXUS_FrontendOutOfBandMode_eDvs167Qpsk      NEXUS_FrontendOutOfBandMode_eAnnexAQpsk /* Changed to use "#define" to eliminate Coverity MIXED_ENUM issue*/
typedef enum NEXUS_FrontendOutOfBandMode
{
    NEXUS_FrontendOutOfBandMode_eAnnexAQpsk,
    NEXUS_FrontendOutOfBandMode_eDvs178Qpsk,
    NEXUS_FrontendOutOfBandMode_ePod_AnnexAQpsk,
    NEXUS_FrontendOutOfBandMode_ePod_Dvs178Qpsk,
    NEXUS_FrontendOutOfBandMode_eMax
} NEXUS_FrontendOutOfBandMode;

/***************************************************************************
Summary:
Spectrum inversion control
***************************************************************************/
typedef enum NEXUS_FrontendOutOfBandSpectrum
{
    NEXUS_FrontendOutOfBandSpectrum_eAuto,
    NEXUS_FrontendOutOfBandSpectrum_eNonInverted,
    NEXUS_FrontendOutOfBandSpectrum_eInverted,
    NEXUS_FrontendOutOfBandSpectrum_eMax
} NEXUS_FrontendOutOfBandSpectrum;

/***************************************************************************
Summary:
    Enumeration for BERT Input Source
****************************************************************************/
typedef enum NEXUS_FrontendOutOfBandBertSource
{
    NEXUS_FrontendOutOfBandBertSource_eIChOutput, /* BERT input source is the receiver I-channel output */
    NEXUS_FrontendOutOfBandBertSource_eQChOutput, /* BERT input source is the receiver Q-channel output */
    NEXUS_FrontendOutOfBandBertSource_eIQChOutputInterleave, /* BERT input source is the receiver I-channel and Q-channel output interleaved */
    NEXUS_FrontendOutOfBandBertSource_eFecOutput, /* BERT input source is the FEC output */
    NEXUS_FrontendOutOfBandBertSource_eMax
} NEXUS_FrontendOutOfBandBertSource;

/***************************************************************************
Summary:
    Enumeration for the kind of Nyquist Filter for out of band.
****************************************************************************/
typedef enum NEXUS_FrontendOutOfBandNyquistFilter
{           
    NEXUS_NyquistFilter_eRaisedCosine50,      /* Raised Cosine Nyquist Filter with 50% Roll Off. Applicable only to Annex-B. */
    NEXUS_NyquistFilter_eRootRaisedCosine50,  /* Root Raised Cosine Nyquist Filter with 50% Roll Off. Applicable only to Annex-B */
    NEXUS_NyquistFilter_eRootRaisedCosine30,  /* Root Raised Cosine Nyquist Filter with 30% Roll Off. Applicable only to Annex-A */
    NEXUS_NyquistFilter_eMax    
} NEXUS_FrontendOutOfBandNyquistFilter;

/***************************************************************************
Summary:
    Enumeration for OOB output modes
****************************************************************************/
typedef enum NEXUS_FrontendOutOfBandOutputMode
{
    NEXUS_FrontendOutOfBandOutputMode_eDifferentialDecoder, /* pre-FEC Transport Stream sent to Cable Card */
    NEXUS_FrontendOutOfBandOutputMode_eFec,                 /* post FEC transport sent to the backend chip */
    NEXUS_FrontendOutOfBandOutputMode_eMax
} NEXUS_FrontendOutOfBandOutputMode;

/***************************************************************************
Summary:
    OOB BERT polynomial used for bit error rate testing.
****************************************************************************/
typedef enum  NEXUS_FrontendOutOfBandBertPolynomial
{
     NEXUS_FrontendOutOfBandBertPolynomial_e23, /* 23-bit polynomial */
     NEXUS_FrontendOutOfBandBertPolynomial_e15, /* 15-bit polynomial */
     NEXUS_FrontendOutOfBandBertPolynomial_eMax
}  NEXUS_FrontendOutOfBandBertPolynomial;

/***************************************************************************
Summary:
    Out-of-band tuning parameters
***************************************************************************/
typedef struct NEXUS_FrontendOutOfBandSettings
{
    NEXUS_FrontendOutOfBandMode mode;
    unsigned frequency;
    NEXUS_FrontendOutOfBandSpectrum spectrum;
    unsigned symbolRate;                        /* Symbol Rate in Baud */
    NEXUS_CallbackDesc lockCallback;            /* Lock status changed callback */
    bool openDrain;                             /* true = open drain, false = normal mode; not supported for 3255 */
    bool autoAcquire;
    NEXUS_FrontendOutOfBandBertSource bertSource; /* BERT input source */
    NEXUS_CallbackDesc asyncStatusReadyCallback;  /* Callback will be called when the async out of band status is ready */
    struct
    {
      NEXUS_FrontendOutOfBandBertPolynomial polynomial; /* PRBS(Pseudo Random Binary Sequence) used by the BERT module */
    } bert;
} NEXUS_FrontendOutOfBandSettings;

/***************************************************************************
Summary:
    Initialize an out-of-band settings structure to defaults
***************************************************************************/
void NEXUS_Frontend_GetDefaultOutOfBandSettings(
    NEXUS_FrontendOutOfBandSettings *pSettings
    );

/***************************************************************************
Summary:
    Tune a frontend to an out-of-band channel

Description:
    See NEXUS_Frontend_TuneQam for details on how lockCallback and fast status should be used.
    See NEXUS_Frontend_Untune to reduce power with possible performance cost.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_TuneOutOfBand(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendOutOfBandSettings *pSettings
    );

/***************************************************************************
Summary:
Out-of-band tuning status
***************************************************************************/
typedef struct NEXUS_FrontendOutOfBandStatus
{
    NEXUS_FrontendOutOfBandSettings settings; /* Settings provided at last call to NEXUS_Frontend_TuneOutOfBand */    

    NEXUS_FrontendOutOfBandMode mode;
    NEXUS_FrontendOutOfBandSpectrum spectrum;
    unsigned symbolRate;                      /* Symbol Rate in Baud */
    unsigned ifFreq;                          /* in Hertz, IF freq. */
    unsigned loFreq;                          /* in Hertz, LO freq. */
    unsigned sysXtalFreq;                     /* in Hertz, Sys. Xtal freq. */
    bool isFecLocked;
    bool isQamLocked;
    bool isBertLocked;                        /* This bit indicates whether BERT is locked. lock=1, unlock=0 */
    int snrEstimate;                          /* in 1/100 db */
    int agcIntLevel;                          /* in 1/10 percent */
    int agcExtLevel;                          /* in 1/10 percent */
    int carrierFreqOffset;                    /* in 1/1000 Hz */
    int carrierPhaseOffset;                   /* in 1/1000 Hz */
    unsigned uncorrectedCount;                /* not self-clearing  */
    unsigned correctedCount;                  /* not self-clearing*/
    unsigned berErrorCount;                   /* not self-clearing */
    unsigned postRsBer;                       /* Post Reed Solomon bit error rate (BER) in 1/2147483648 th units. This is accumulated bit error
                                                 rate since the last GetOutOfBandStatus call.
                                                 Convert to floating point by dividing by 2147483648.0 */
    unsigned postRsBerElapsedTime;            /* postRsBer over this time. In units of seconds. */
    int fdcChannelPower;                      /* units of 1/10 dBmV */
    int frontendGain;                         /* in 1/100th of a dB. Is the accumulated gain of the tuner/ufe/wfe. */
    int digitalAgcGain;                       /* in 1/100th of a dB. Is the AGC gain in the demod core. */
    int highResEqualizerGain;                 /* in 1/100th of a dB. Is the equalizer gain in the demod core. */
    unsigned acquisitionTime;                 /* time in mS for the last acquisition */
    unsigned totalAcquisitionTime;            /* time in mS for all acquisitions to acquire the signal. This value can be different from
                                                 acquisitionTime as the number of tries to acquire can be more than one in case of autoAcquire. */
} NEXUS_FrontendOutOfBandStatus;

/***************************************************************************
Summary:
    Get the out-of-band status of a frontend
***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetOutOfBandStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendOutOfBandStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
    Request the asynchronous status of an OutOfBand frontend
***************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestOutOfBandAsyncStatus(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
Summary:
    Get the asynchronous status of an OutOfBand frontend
***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetOutOfBandAsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendOutOfBandStatus *pStatus /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_OOB_H__ */
