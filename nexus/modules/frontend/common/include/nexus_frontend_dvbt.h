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
*   API name: Frontend DVBT Status
*    Generic APIs for DVBT status.
*    This is used in DVB-H and DVB-T environments.
*
***************************************************************************/
#ifndef NEXUS_FRONTEND_DVBT_H__
#define NEXUS_FRONTEND_DVBT_H__

#include "nexus_frontend_ofdm_types.h"
#include "nexus_frontend_ofdm.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    This structure represents DVB-T Transmission Parameter Signalling (TPS) Status
****************************************************************************/
typedef struct NEXUS_FrontendDvbtTpsStatus
{
    NEXUS_FrontendOfdmModulation         modulation;             /* Current modulation type */
    NEXUS_FrontendOfdmTransmissionMode   transmissionMode;       /* Detected trasmission mode */
    NEXUS_FrontendOfdmGuardInterval      guardInterval;          /* Detected guard interval */
    NEXUS_FrontendOfdmCodeRate           codeRate;               /* Code rate of the input signal */
    NEXUS_FrontendOfdmHierarchy          hierarchy;              /* Hierarchy of the input signal */
    unsigned                             cellId;                 /* Indicates Cell Id obtained from the TPS parameters */
    bool                                 inDepthSymbolInterleave;/* If true, in depth symbol interleaving is enabled.  Applies only for DVB-H. */
    bool                                 timeSlicing;            /* If true, time slicing of the services is used for receiver power savings. Applies only for DVB-H. */
    bool                                 mpeFec;                 /* If true, MultiProtocol Encapsulation (MPE) Forward Error Correction (FEC) is used. Applies only for DVB-H*/
} NEXUS_FrontendDvbtTpsStatus;

/***************************************************************************
Summary:
    OFDM tuning status
****************************************************************************/
typedef struct NEXUS_FrontendDvbtBasicStatus
{
    NEXUS_FrontendOfdmSettings settings; /* Settings provided at last call to NEXUS_Frontend_TuneOfdm */

    bool                                 fecLock;                /* Indicates whether the FEC is locked */
    bool                                 spectrumInverted;       /* If true, the spectrum is inverted. */
    int                                  snr;                    /* SNR value of receiver in 1/100 dB */
    int                                  gainOffset;             /* Internal AGC gain offset in 1/100 dB */ 
    int                                  carrierOffset;          /* Offset of carrier loop in Hz */
    int                                  timingOffset;           /* Offset of timing loop in Hz */
    int                                  signalStrength;         /* signal strength in units of 1/10 dBm */
    unsigned                             signalLevelPercent;     /* Signal Level in percent */
    unsigned                             signalQualityPercent;   /* Signal Quality in percent */
    unsigned                             reacquireCount;         /* number of reacquisitions performed */
    NEXUS_FrontendDvbtTpsStatus          tps;
    NEXUS_FrontendBlockCounts            fecBlockCounts;         /* accumulated since tune or NEXUS_Frontent_ResetStatus. */
    NEXUS_FrontendErrorRate              viterbiErrorRate;       /* Viterbi (aka pre-Reed-Solomon) bit counts. Only the rate is applicable for now. Accumulated since
                                                                    tune or NEXUS_Frontent_ResetStatus. */
} NEXUS_FrontendDvbtBasicStatus;

/***************************************************************************
Summary:
****************************************************************************/
typedef enum NEXUS_FrontendDvbtStatusType
{
  NEXUS_FrontendDvbtStatusType_eBasic,
  NEXUS_FrontendDvbtStatusType_eMax 
} NEXUS_FrontendDvbtStatusType;

/***************************************************************************
Summary:
****************************************************************************/
typedef struct NEXUS_FrontendDvbtStatusReady
{
    bool type[NEXUS_FrontendDvbtStatusType_eMax];
} NEXUS_FrontendDvbtStatusReady;

/***************************************************************************
Summary:
****************************************************************************/
typedef struct NEXUS_FrontendDvbtStatus
{
    NEXUS_FrontendDvbtStatusType type;

    struct
    {
        NEXUS_FrontendDvbtBasicStatus basic;
    } status;
    
} NEXUS_FrontendDvbtStatus;

/*******************************************************************************
Summary: Request the Dvbt asynchronous status of NEXUS_FrontendDvbtAsyncStatus type.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestDvbtAsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbtStatusType type
    );

/*******************************************************************************
Summary: Get the Dvbt asynchronous status ready type.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbtAsyncStatusReady(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbtStatusReady *pStatusReady /* [out] */
    );

/*******************************************************************************
Summary: Get the Dvbt asynchronous status.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbtAsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbtStatusType type,
    NEXUS_FrontendDvbtStatus *pStatus   /* [out] */
    );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_DVBT_H__ */


