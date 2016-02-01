/***************************************************************************
*     (c)2012-2014 Broadcom Corporation
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
* API name: DOCSIS Data channel
* Generic APIs for DOCSIS data channel
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_DOCSIS_DATA_H__
#define NEXUS_DOCSIS_DATA_H__

#include "nexus_frontend.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary: 
Enumeration for DOCSIS data channel CDMA type 
***************************************************************************/
typedef enum NEXUS_DocsisDataChannelScdmaType
{
    NEXUS_DocsisDataChannelScdmaType_eNo,
    NEXUS_DocsisDataChannelScdmaType_eTcm,
    NEXUS_DocsisDataChannelScdmaType_eTdma,
    NEXUS_DocsisDataChannelScdmaType_eMax
} NEXUS_DocsisDataChannelScdmaType;


/***************************************************************************
Summary: 
Enumeration for DOCSIS data channel modulation. 
***************************************************************************/
typedef enum NEXUS_DocsisDataChannelModulationType
{
    NEXUS_DocsisDataChannelModulationType_eQpsk,
    NEXUS_DocsisDataChannelModulationType_eQam,
    NEXUS_DocsisDataChannelModulationType_eMax
} NEXUS_DocsisDataChannelModulationType;

/***************************************************************************
Summary:
Status of a DOCSIS data channel. Data channels are controlled by 
eCM software. This structure maps to BRPC_Param_ECM_GetStatus 
***************************************************************************/
typedef struct NEXUS_DocsisDataChannelStatus
{
    unsigned downstreamCenterFreq;                /* Docsis channel DS frequency */
    bool downstreamCarrierLock;                   /* Docsis channel DS lock status */
    NEXUS_DocsisDataChannelScdmaType channelScdmaStatus; /* Docsis channel DS scdma status */
    
    unsigned downstreamPowerLevel;               /* US power unit = 1/10 dBmV */
	unsigned snrEstimate;       			 	  /* Docsis channel DS primary channel SNR estimate in 1/256 dB */
    /* if QAM, see upstreamModulationType */
    NEXUS_DocsisDataChannelModulationType upstreamModulationType; /* US modulation type. If eQam, see upstreamQamMode. */
    NEXUS_FrontendQamMode upstreamQamMode;
    unsigned upstreamXmtCenterFreq;               /* US transmission frequency unit = Hz */
    unsigned upstreamPowerLevel;                  /* US power unit = dBm */
    unsigned upstreamSymbolRate;                  /* US symbol rate unit = symbols per second */
    
    uint8_t ecmMacAddress[8];
    NEXUS_EcmIpMode ecmIpMode;
    uint8_t ecmIpAddress[4];                      /* eCM WAN IPV4 address */
    uint8_t ecmIpv6Address[32];                   /* eCM WAN IPV6 address */
    unsigned lastKnownGoodFreq;                   /* Last Frequency that Docsis successfully registered on. 0 if unknown. */
} NEXUS_DocsisDataChannelStatus;

NEXUS_Error NEXUS_Docsis_GetDataChannelStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_DocsisDataChannelStatus *pStatus /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_DOCSIS_DATA_H__ */
