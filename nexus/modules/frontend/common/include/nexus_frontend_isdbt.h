/***************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*   API name: Frontend ISDBT Status
*    Generic APIs for ISDBT status.
*    This is used in ISDB-T environment.
*
***************************************************************************/
#ifndef NEXUS_FRONTEND_ISDBT_H__
#define NEXUS_FRONTEND_ISDBT_H__

#include "nexus_frontend_ofdm_types.h"
#include "nexus_frontend_ofdm.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    This structure represents ISDB-T Transmission & Multiplexing Configuration Control (TMCC) Status
****************************************************************************/
typedef struct NEXUS_FrontendIsdbtTmccStatus
{
    NEXUS_FrontendOfdmTransmissionMode   transmissionMode;       /* Detected trasmission mode */
    NEXUS_FrontendOfdmGuardInterval      guardInterval;          /* Detected guard interval */
    bool                                 ews;                    /* If true, the EWS (Emergency Warning System) Indicator is present */
    bool                                 partialReception;       /* If true, the signal is partially received */
} NEXUS_FrontendIsdbtTmccStatus;

/***************************************************************************
Summary:
    OFDM layer status
****************************************************************************/
typedef struct NEXUS_FrontendIsdbtLayerStatus
{
    NEXUS_FrontendOfdmModulation       modulation;             /* Layer modulation type. */
    NEXUS_FrontendOfdmCodeRate         codeRate;               /* Layer code rate. */
    NEXUS_FrontendOfdmTimeInterleaving timeInterleaving;       /* Layer time interleaving.*/
    unsigned                           numSegments;            /* Layer number of segments. */
    unsigned                           signalLevelPercent;     /* Layer Signal Level in percent */
    unsigned                           signalQualityPercent;   /* Layer Signal Quality in percent */
    NEXUS_FrontendBlockCounts          fecBlockCounts;         /* Accumulated since tune or NEXUS_Frontent_ResetStatus. */    
    NEXUS_FrontendErrorRate            viterbiErrorRate;       /* Viterbi (aka pre-Reed-Solomon) bit counts. Only the rate is applicable for now.
        Accumulated since tune or NEXUS_Frontent_ResetStatus. */
    struct {
        bool                           locked;                 /* True if the BER tester is locked.  If so, see 'errorRate'. */
        NEXUS_FrontendErrorRate        errorRate;              /* This is bit error rate only for test signals like pn15/pn23. Accumulated since
                                                                  tune or NEXUS_Frontent_ResetStatus. To ensure the 32 bit total bits counters don't
                                                                  overflow, the status need to be read in less than three seconds intervals. */
    } bert;
}NEXUS_FrontendIsdbtLayerStatus;

/***************************************************************************
Summary:
    OFDM tuning status
****************************************************************************/
typedef struct NEXUS_FrontendIsdbtBasicStatus
{
    NEXUS_FrontendOfdmSettings settings; /* Settings provided at last call to NEXUS_Frontend_TuneOfdm */

    bool                                 fecLock;                /* Indicates whether the FEC is locked */
    bool                                 spectrumInverted;       /* If true, the spectrum is inverted. */
    int                                  snr;                    /* SNR value of receiver in 1/100 dB */
    int                                  gainOffset;             /* Internal AGC gain offset in 1/100 dB */
    int                                  carrierOffset;          /* Offset of carrier loop in Hz */
    int                                  timingOffset;           /* Offset of timing loop in Hz */
    int                                  signalStrength;         /* signal strength in units of 1/10 dBm */
    unsigned                             reacquireCount;         /* number of reacquisitions performed */
    NEXUS_FrontendIsdbtTmccStatus        tmcc;

    NEXUS_FrontendIsdbtLayerStatus       layerA;
    NEXUS_FrontendIsdbtLayerStatus       layerB;
    NEXUS_FrontendIsdbtLayerStatus       layerC;
} NEXUS_FrontendIsdbtBasicStatus;

/***************************************************************************
Summary:
****************************************************************************/
typedef enum NEXUS_FrontendIsdbtStatusType
{
  NEXUS_FrontendIsdbtStatusType_eBasic,
  NEXUS_FrontendIsdbtStatusType_eMax
} NEXUS_FrontendIsdbtStatusType;

/***************************************************************************
Summary:
****************************************************************************/
typedef struct NEXUS_FrontendIsdbtStatusReady
{
    bool type[NEXUS_FrontendIsdbtStatusType_eMax];
} NEXUS_FrontendIsdbtStatusReady;

/***************************************************************************
Summary:
****************************************************************************/
typedef struct NEXUS_FrontendIsdbtStatus
{
    NEXUS_FrontendIsdbtStatusType type;

    union
    {
        NEXUS_FrontendIsdbtBasicStatus basic;
    } status;

} NEXUS_FrontendIsdbtStatus;

/*******************************************************************************
Summary: Request the Isdbt asynchronous status of NEXUS_FrontendIsdbtAsyncStatus type.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestIsdbtAsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendIsdbtStatusType type
    );

/*******************************************************************************
Summary: Get the Isdbt asynchronous status ready type.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_GetIsdbtAsyncStatusReady(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendIsdbtStatusReady *pStatusReady /* [out] */
    );

/*******************************************************************************
Summary: Get the Isdbt asynchronous status.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_GetIsdbtAsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendIsdbtStatusType type,
    NEXUS_FrontendIsdbtStatus *pStatus   /* [out] */
    );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_ISDBT_H__ */



