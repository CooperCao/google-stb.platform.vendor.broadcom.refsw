/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
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
* API Description:
*   API name: Frontend OFDM
*    Generic APIs for OFDM (Orthogonal Frequency-Division Multiplexing) tuning.
*    This is used in DVB-H and DVB-T environments.
*
***************************************************************************/
#ifndef NEXUS_FRONTEND_OFDM_H__
#define NEXUS_FRONTEND_OFDM_H__

#include "nexus_frontend.h"
#include "nexus_frontend_ofdm_types.h"
#include "nexus_frontend_t2_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Initialize an OFDM settings structure to defaults
 ***************************************************************************/
void NEXUS_Frontend_GetDefaultOfdmSettings(
    NEXUS_FrontendOfdmSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Tune to an OFDM channel

Description:
    See NEXUS_Frontend_TuneQam for details on how lockCallback and fast status should be used.
    See NEXUS_Frontend_Untune to reduce power with possible performance cost.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_TuneOfdm(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendOfdmSettings *pSettings
    );

/***************************************************************************
Summary:
    OFDM tuning status
****************************************************************************/
typedef struct NEXUS_FrontendOfdmStatus
{
    NEXUS_FrontendOfdmSettings settings; /* Settings provided at last call to NEXUS_Frontend_TuneOfdm */

    bool                                 fecLock;                /* Indicates whether the FEC is locked */
    bool                                 receiverLock;           /* Indicates whether the receiver is locked */
    bool                                 noSignalDetected;       /* True if the demodulator detects no signal for channel scan purposes.
                                                                    This should only be used for channel scanning. */
    bool                                 spectrumInverted;       /* If true, the spectrum is inverted. */
    bool                                 ews;                    /* If true, the ISDB-T EWS (Emergency Warning System) Indicator is present */
    bool                                 partialReception;       /* If true, the ISDB-T signal is partially received */
    NEXUS_FrontendOfdmModulation         modulation;             /* Current modulation type */
    NEXUS_FrontendOfdmTransmissionMode   transmissionMode;       /* Detected transmission mode */
    NEXUS_FrontendOfdmGuardInterval      guardInterval;          /* Detected guard interval */
    NEXUS_FrontendOfdmCodeRate           codeRate;               /* Code rate of the input signal */
    NEXUS_FrontendOfdmHierarchy          hierarchy;              /* Hierarchy of the input signal */
    unsigned                             cellId;                 /* Indicates Cell Id obtained from the TPS parameters */
    int                                  carrierOffset;          /* Offset of carrier loop in Hz */
    int                                  timingOffset;           /* Offset of timing loop in Hz */
    int                                  snr;                    /* SNR value of receiver in 1/100 dB */

    unsigned                             fecCorrectedBlocks;     /* FEC corrected block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned                             fecUncorrectedBlocks;   /* FEC uncorrected block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned                             fecCleanBlocks;         /* FEC clean block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned                             reacquireCount;         /* number of reacquisitions performed */
    int                                  signalStrength;         /* signal strength in units of 1/10 dBmV */
    unsigned                             signalLevelPercent;     /* Signal Level in percent */
    unsigned                             signalQualityPercent;   /* Signal Quality in percent */

    unsigned                             viterbiUncorrectedBits; /* uncorrected error bits output from Viterbi, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned                             viterbiTotalBits;       /* total number of bits output from Viterbi, accumulated since tune or NEXUS_Frontent_ResetStatus */
    uint32_t                             viterbiErrorRate;       /* viterbi bit error rate (BER) in 1/2147483648 th units.
                                                                    Convert to floating point by dividing by 2147483648.0 */

    uint32_t                             preViterbiErrorRate;    /* This indicates the pre-viterbi bit error rate in 1/2147483648 th unit.
                                                                    Convert to floating point by dividing by 2147483648.0 */

    unsigned                             ifAgcLevel;             /* IF AGC level in units of 1/10 percent */
    unsigned                             rfAgcLevel;             /* tuner AGC level in units of 1/10 percent */

    uint32_t                             rsPerDelta;             /* RS PER delta in 1/2147483648 th units. Valid for a GetStatus call no more than 5 seconds after a previous GetStatus call. */
    uint32_t                             vitBerDelta;            /* Viterbi BER delta in 1/2147483648 th units. Valid for a GetStatus call no more than 5 seconds after a previous GetStatus call. */

    NEXUS_FrontendOfdmModulation       modulationA;             /* ISDB-T Layer A modulation type. */
    NEXUS_FrontendOfdmCodeRate         codeRateA;               /* ISDB-T Layer A code rate. */
    uint32_t                           isdbtAPreRS;             /* ISDB-T Layer A uncorrected error by Viterbi before ReedSolomon */
    NEXUS_FrontendOfdmTimeInterleaving timeInterleavingA;       /* ISDB-T Layer A time interleaving.*/
    unsigned                           numSegmentsA;            /* ISDB-T Layer A number of segments. */
    unsigned                           fecCorrectedBlocksA;     /* ISDB-T Layer A FEC corrected block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned                           fecUncorrectedBlocksA;   /* ISDB-T Layer A FEC uncorrected block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned                           fecCleanBlocksA;         /* ISDB-T Layer A FEC clean block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned                           signalLevelPercentA;     /* ISDB-T Layer A Signal Level in percent */
    unsigned                           signalQualityPercentA;   /* ISDB-T Layer A Signal Quality in percent */

    NEXUS_FrontendOfdmModulation       modulationB;             /* ISDB-T Layer B modulation type. */
    NEXUS_FrontendOfdmCodeRate         codeRateB;               /* ISDB-T Layer B code rate. */
    uint32_t                           isdbtBPreRS;             /* ISDB-T Layer B uncorrected error by Viterbi before ReedSolomon */
    NEXUS_FrontendOfdmTimeInterleaving timeInterleavingB;       /* ISDB-T Layer B time interleaving.*/
    unsigned                           numSegmentsB;            /* ISDB-T Layer B number of segments. */
    unsigned                           fecCorrectedBlocksB;     /* ISDB-T Layer B FEC corrected block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned                           fecUncorrectedBlocksB;   /* ISDB-T Layer B FEC uncorrected block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned                           fecCleanBlocksB;         /* ISDB-T Layer B FEC clean block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned                           signalLevelPercentB;     /* ISDB-T Layer B Signal Level in percent */
    unsigned                           signalQualityPercentB;   /* ISDB-T Layer B Signal Quality in percent */

    NEXUS_FrontendOfdmModulation       modulationC;             /* ISDB-T Layer C modulation type. */
    NEXUS_FrontendOfdmCodeRate         codeRateC;               /* ISDB-T Layer C code rate. */
    uint32_t                           isdbtCPreRS;             /* ISDB-T Layer C uncorrected error by Viterbi before ReedSolomon */
    NEXUS_FrontendOfdmTimeInterleaving timeInterleavingC;       /* ISDB-T Layer C time interleaving.*/
    unsigned                           numSegmentsC;            /* ISDB-T Layer C number of segments. */
    unsigned                           fecCorrectedBlocksC;     /* ISDB-T Layer C FEC corrected block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned                           fecUncorrectedBlocksC;   /* ISDB-T Layer C FEC uncorrected block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned                           fecCleanBlocksC;         /* ISDB-T Layer C FEC clean block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned                           signalLevelPercentC;     /* ISDB-T Layer C Signal Level in percent */
    unsigned                           signalQualityPercentC;   /* ISDB-T Layer C Signal Quality in percent */
    struct
    {
       int32_t                                     gainOffset;
       NEXUS_FrontendDvbt2FecStatistics            l1PreStatistics;     /* This Structure contains the lock status, SNR and FEC Statistics for L1 Pre */
       NEXUS_FrontendDvbt2FecStatistics            l1PostStatistics;    /* This Structure contains the lock status, SNR and FEC Statistics for L1 Post */
       NEXUS_FrontendDvbt2FecStatistics            plpAStatistics;      /* This Structure contains the lock status, SNR and FEC Statistics for PLPA */
       NEXUS_FrontendDvbt2FecStatistics            plpBStatistics;      /* This Structure contains the lock status, SNR and FEC Statistics for PLPB */
       NEXUS_FrontendDvbt2L1PreStatus              l1PreStatus;         /* This Structure contains the decoded L1 Pre parameters */
       NEXUS_FrontendDvbt2L1PostConfigurableStatus l1PostCfgStatus;     /* This Structure contains the decoded L1 Post Configurable parameters */
       NEXUS_FrontendDvbt2L1PostDynamicStatus      l1PostDynamicStatus; /* This Structure contains the decoded L1 Post Dynamic parameters */
       NEXUS_FrontendDvbt2L1PlpStatus              l1PlpStatus;         /* This Structure contains a list of the PLPs found on the RF signal */
    } dvbt2Status;
} NEXUS_FrontendOfdmStatus;

/***************************************************************************
Summary:
    Get the status of a OFDM tuner
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetOfdmStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendOfdmStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
    Request the asynchronous status of a Ofdm tuner
***************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestOfdmAsyncStatus(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
Summary:
    Get the asynchronous status of a Ofdm tuner
***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetOfdmAsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendOfdmStatus *pStatus /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_OFDM_H__ */

