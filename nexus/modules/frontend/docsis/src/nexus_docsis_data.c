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
* Generic APIs for DOCSIS Data channel Status
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_frontend_module.h"
#include "nexus_docsis_data.h"
#include "nexus_docsis_priv.h"

BDBG_MODULE(nexus_docsis_data);

void b_convert_ipaddr(uint8_t *c, uint32_t b)
{
    c[0] = (b>>24)&0xff;
    c[1] = (b>>16)&0xff;
    c[2] = (b>>8)&0xff;
    c[3] =  b&0xff;
}

NEXUS_Error NEXUS_Docsis_GetDataChannelStatus(
    NEXUS_FrontendHandle hFrontend,
    NEXUS_DocsisDataChannelStatus *pStatus
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_DocsisDeviceHandle hDevice = NULL;
    NEXUS_DocsisChannelHandle hChannel = NULL;
    BDCM_DataStatus dataStatus;
    BDBG_OBJECT_ASSERT(hFrontend,NEXUS_Frontend);
    BDBG_ASSERT(pStatus);
    hChannel = (NEXUS_DocsisChannelHandle)hFrontend->pDeviceHandle;
    BDBG_OBJECT_ASSERT(hChannel,NEXUS_DocsisChannel);
    hDevice = (NEXUS_DocsisDeviceHandle)hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);
    
    BKNI_Memset(pStatus, 0, sizeof(NEXUS_DocsisDataChannelStatus));
    if (hDevice->status.state != NEXUS_DocsisDeviceState_eOperational) 
    {
        return (NEXUS_SUCCESS);
    }
    retCode = BDCM_GetDeviceDataChannelStatus(hDevice->hDocsis,
                                              hDevice->version,
                                              hChannel->dsChannelNum,
                                              &dataStatus);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("NEXUS_Docsis_GetDataChannelStatus failed"));
        return NEXUS_NOT_SUPPORTED;
    }
	pStatus->downstreamPowerLevel = dataStatus.downstreamPowerLevel;
    pStatus->downstreamCenterFreq = dataStatus.downstreamCenterFreq;
    pStatus->downstreamCarrierLock = dataStatus.downstreamCarrierLock;
    pStatus->channelScdmaStatus = (NEXUS_DocsisDataChannelScdmaType)dataStatus.channelScdmaStatus;
    switch(dataStatus.upstreamModuType) 
    {
    case 0:
        pStatus->upstreamModulationType = NEXUS_DocsisDataChannelModulationType_eQpsk;
        break;
    case 1:
        pStatus->upstreamModulationType = NEXUS_DocsisDataChannelModulationType_eQam;
        pStatus->upstreamQamMode = NEXUS_FrontendQamMode_e16;
        break;
    case 2:
        pStatus->upstreamModulationType = NEXUS_DocsisDataChannelModulationType_eQam;
        pStatus->upstreamQamMode = NEXUS_FrontendQamMode_e32;
        break;
    case 3:
        pStatus->upstreamModulationType = NEXUS_DocsisDataChannelModulationType_eQam;
        pStatus->upstreamQamMode = NEXUS_FrontendQamMode_e64;
        break;
    case 4:
        pStatus->upstreamModulationType = NEXUS_DocsisDataChannelModulationType_eQam;
        pStatus->upstreamQamMode = NEXUS_FrontendQamMode_e128;
        break;
    case 5:
        pStatus->upstreamModulationType = NEXUS_DocsisDataChannelModulationType_eQam;
        pStatus->upstreamQamMode = NEXUS_FrontendQamMode_e256;
        break;
    case 6:
        pStatus->upstreamModulationType = NEXUS_DocsisDataChannelModulationType_eQam;
        pStatus->upstreamQamMode = NEXUS_FrontendQamMode_e512;
    default:
        BDBG_WRN(("invalid upstream modulation type"));
        pStatus->upstreamModulationType = NEXUS_DocsisDataChannelModulationType_eMax;
        pStatus->upstreamQamMode = NEXUS_FrontendQamMode_eMax;

    }
    pStatus->upstreamXmtCenterFreq = dataStatus.upstreamXmtCenterFreq;
    pStatus->upstreamPowerLevel = dataStatus.upstreamPowerLevel;
    pStatus->upstreamSymbolRate = dataStatus.upStreamSymbolrate;
    pStatus->lastKnownGoodFreq = dataStatus.lastKnownGoodFreq;
	pStatus->snrEstimate = dataStatus.snrEstimated;

    pStatus->ecmMacAddress[0] = (dataStatus.ecmMacAddressHi>>24)&0xff;
    pStatus->ecmMacAddress[1] = (dataStatus.ecmMacAddressHi>>16)&0xff;
    pStatus->ecmMacAddress[2] = (dataStatus.ecmMacAddressHi>>8)&0xff;
    pStatus->ecmMacAddress[3] = (dataStatus.ecmMacAddressHi)&0xff;
    pStatus->ecmMacAddress[4] = (dataStatus.ecmMacAddressLo>>24)&0xff;
    pStatus->ecmMacAddress[5] = (dataStatus.ecmMacAddressLo>>16)&0xff;
    pStatus->ecmMacAddress[6] = (dataStatus.ecmMacAddressLo>>8)&0xff;
    pStatus->ecmMacAddress[7] = (dataStatus.ecmMacAddressLo)&0xff;

    if (dataStatus.isEcmIpMode)
    {
        b_convert_ipaddr(pStatus->ecmIpAddress,dataStatus.ecmIpAddress);
        b_convert_ipaddr(&pStatus->ecmIpv6Address[0],dataStatus.ecmIpv6Address0);
        b_convert_ipaddr(&pStatus->ecmIpv6Address[4],dataStatus.ecmIpv6Address1);
        b_convert_ipaddr(&pStatus->ecmIpv6Address[8],dataStatus.ecmIpv6Address2);
        b_convert_ipaddr(&pStatus->ecmIpv6Address[12],dataStatus.ecmIpv6Address3);
    }
    return retCode;
}
