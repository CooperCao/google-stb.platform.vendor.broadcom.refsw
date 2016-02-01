/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
*   API name: Frontend VSB
*    Generic APIs for VSB tuning.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_VSB_H__
#define NEXUS_FRONTEND_VSB_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
VSB tuning modes
***************************************************************************/
typedef enum NEXUS_FrontendVsbMode
{
    NEXUS_FrontendVsbMode_e8,
    NEXUS_FrontendVsbMode_e16,
    NEXUS_FrontendVsbMode_eMax
} NEXUS_FrontendVsbMode;

/***************************************************************************
Summary:
Determines if VSB spectral inversion is enabled or not
***************************************************************************/
typedef enum NEXUS_FrontendVsbSpectralInversion
{
    NEXUS_FrontendVsbSpectralInversion_eNormal,
    NEXUS_FrontendVsbSpectralInversion_eInverted,
    NEXUS_FrontendVsbSpectralInversion_eMax
} NEXUS_FrontendVsbSpectralInversion;

/***************************************************************************
Summary:
VSB tuning parameters
***************************************************************************/
typedef struct NEXUS_FrontendVsbSettings
{
    NEXUS_FrontendVsbMode mode;         /* Modulation Scheme */

    unsigned frequency;                 /* In Hz */
    unsigned ifFrequency;               /* IF Frequency in Hz */
    unsigned symbolRate;                /* In Baud */

    int ifFrequencyOffset;              /* IF Frequency offset in Hz */
    int symbolRateOffset;               /* In Baud */
    int ntscOffset;                     /* In Hz */

    bool fastAcquisition;               /* True=fast acquisition mode, false=default */
    bool terrestrial;                   /* Receiver mode: true=terrestrial, false=cable */

    bool autoAcquire;                   /* automatically reacquire signal if out of lock */
    bool ntscSweep;                     /* enable NTSC canceller */
    bool rfiSweep;                      /* enable RFI canceller */

    NEXUS_CallbackDesc lockCallback;    /* Callback will be called when lock status changes */
    NEXUS_CallbackDesc asyncStatusReadyCallback; /* Callback will be called when lock status changes */
    NEXUS_FrontendVsbSpectralInversion spectralInversion; /* Indciate if the spectrum is inverted or not. */
} NEXUS_FrontendVsbSettings;

/***************************************************************************
Summary:
    Initialize a VSB settings structure to defaults
***************************************************************************/
void NEXUS_Frontend_GetDefaultVsbSettings(
    NEXUS_FrontendVsbSettings *pSettings
    );

/***************************************************************************
Summary:
    Tune to a VSB channel
***************************************************************************/
NEXUS_Error NEXUS_Frontend_TuneVsb(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendVsbSettings *pSettings
    );

/***************************************************************************
Summary:
VSB tuning status
***************************************************************************/
typedef struct NEXUS_FrontendVsbStatus
{
    NEXUS_FrontendVsbSettings settings; /* Settings provided at last call to NEXUS_Frontend_TuneVsb */

    bool     receiverLock;      /* true if the receiver is locked */
    bool     fecLock;           /* true if the FEC is locked */
    bool     opllLock;          /* true if the output PLL is locked */

    unsigned symbolRate;        /* standard symbol rate in Baud */
    int      symbolRateError;   /* symbol rate error in Baud */

    unsigned ifFrequency;       /* standard IF frequency in Hz */
    int      ifFrequencyError;  /* IF frequency error in Hz */

    unsigned ifAgcLevel;        /* IF AGC level in units of 1/10 percent */
    unsigned rfAgcLevel;        /* tuner AGC level in units of 1/10 percent */
    unsigned intAgcLevel;       /* Internal AGC level in units of 1/10 percent */
    unsigned snrEstimate;       /* SNR estimate in units of 1/100 dB */

    unsigned fecCorrected;      /* FEC corrected block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned fecUncorrected;    /* FEC uncorrected block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned fecClean;          /* FEC clean block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned bitErrCorrected;   /* deprecated: cumulative bit correctable errors. same as viterbiUncorrectedBits. */
    unsigned reacquireCount;    /* cumulative reacquisition count */

    unsigned viterbiUncorrectedBits; /* uncorrected error bits output from Viterbi, accumulated since tune or NEXUS_Frontent_ResetStatus. */
    unsigned viterbiTotalBits;       /* total number of bits output from Viterbi, accumulated since tune or NEXUS_Frontent_ResetStatus. */
    uint32_t viterbiErrorRate;       /* viterbi bit error rate (BER) in 1/2147483648 th units.
                                        Convert to floating point by dividing by 2147483648.0 */

    unsigned timeElapsed; /* time elapsed in milliseconds since the last call to NEXUS_Frontend_ResetStatus.
                             the elapsed time is measured at the same time that the rest of the values in NEXUS_FrontendVsbStatus are captured. */
    int      carrierOffset; /* carrier offset in Hz */
} NEXUS_FrontendVsbStatus;

/***************************************************************************
Summary:
    Get the status of a VSB tuner
***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetVsbStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendVsbStatus *pStatus    /* [out] */
    );

/***************************************************************************
Summary:
    Request the status asynchronously of a VSB tuner
See Also:
    NEXUS_Frontend_GetVsbAsyncStatus
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestVsbAsyncStatus(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
Summary:
    Get the status asynchronously of a VSB tuner
See Also:
    NEXUS_Frontend_TuneVsb
    NEXUS_Frontend_RequestVsbAsyncStatus
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetVsbAsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendVsbStatus *pStatus /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_VSB_H__ */

