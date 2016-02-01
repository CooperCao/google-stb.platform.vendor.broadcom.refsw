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
* Revision History:  
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_DVBT2_H__
#define NEXUS_FRONTEND_DVBT2_H__

#include "nexus_frontend.h"
#include "nexus_frontend_t2_types.h"
#include "nexus_frontend_ofdm.h"

#ifdef __cplusplus
    extern "C" {
#endif

/***************************************************************************
Summary:
    This structure represents Dvbt2 Basic Status
    Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef struct NEXUS_FrontendDvbt2BasicStatus
{
    NEXUS_FrontendOfdmSettings settings; /* Settings provided at last call to NEXUS_Frontend_TuneOfdm */
    bool                           fecLock;              /* Indicates whether the FEC is locked */
    bool                           spectrumInverted;     /* If true, the spectrum is inverted. */
    int32_t                        snr;                  /* SNR value of receiver in 1/100 dB */
    int32_t                        gainOffset;           /* Internal AGC gain offset in 1/100 dB */ 
    int32_t                        carrierOffset;        /* Offset of carrier loop in Hz */
    int32_t                        timingOffset;         /* Offset of timing loop in Hz */
    int32_t                        signalStrength;       /* represents the entire AGC (LNA+TNR+UFE+T2)gain in units of a 1/10dBm */
    unsigned                       signalLevelPercent;   /* Signal Level in percent */
    unsigned                       signalQualityPercent; /* Signal Quality in percent */
    unsigned                       reacquireCount;       /* number of reacquisitions performed */
    NEXUS_FrontendDvbt2Profile     profile;              /* represents the type of DVB-T2 profile present, base or lite */
} NEXUS_FrontendDvbt2BasicStatus;

/***************************************************************************
Summary:
****************************************************************************/
typedef enum NEXUS_FrontendDvbt2StatusType
{
  NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Pre,
  NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Post,
  NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA,
  NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpB,  
  NEXUS_FrontendDvbt2StatusType_eL1Pre,
  NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable,
  NEXUS_FrontendDvbt2StatusType_eL1PostDynamic,
  NEXUS_FrontendDvbt2StatusType_eL1Plp,
  NEXUS_FrontendDvbt2StatusType_eBasic,
  NEXUS_FrontendDvbt2StatusType_eMax 
} NEXUS_FrontendDvbt2StatusType;

/***************************************************************************
Summary:
****************************************************************************/
typedef struct NEXUS_FrontendDvbt2StatusReady
{
    bool type[NEXUS_FrontendDvbt2StatusType_eMax];
} NEXUS_FrontendDvbt2StatusReady;

/***************************************************************************
Summary:
****************************************************************************/
typedef struct NEXUS_FrontendDvbt2Status
{
    NEXUS_FrontendDvbt2StatusType type;

    union
    {
        NEXUS_FrontendDvbt2FecStatistics fecStatistics;
        NEXUS_FrontendDvbt2L1PreStatus   l1Pre;
        NEXUS_FrontendDvbt2L1PostConfigurableStatus l1PostConfigurable;
        NEXUS_FrontendDvbt2L1PostDynamicStatus l1PostDynamic;
        NEXUS_FrontendDvbt2L1PlpStatus l1Plp;
        NEXUS_FrontendDvbt2BasicStatus basic;
    } status;
    
} NEXUS_FrontendDvbt2Status;

/*******************************************************************************
Summary: Request the dvbt2 asynchronous status of NEXUS_FrontendDvbt2AsyncStatus type.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestDvbt2AsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbt2StatusType type
    );

/*******************************************************************************
Summary: Get the dvbt2 asynchronous status ready type.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbt2AsyncStatusReady(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbt2StatusReady *pStatusReady /* [out] */
    );

/*******************************************************************************
Summary: Get the dvbt2 asynchronous status.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbt2AsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbt2StatusType type,
    NEXUS_FrontendDvbt2Status *pStatus   /* [out] */
    );

#ifdef __cplusplus
    }
#endif
    
#endif /* #ifndef NEXUS_FRONTEND_DVBT2_H__ */

