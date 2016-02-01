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
* API name: DOCSIS Device Interface
* APIs for DOCSIS device
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_frontend_module.h"
#include "nexus_docsis_data.h"
#include "nexus_docsis_priv.h"

BDBG_MODULE(nexus_docsis_interface);

extern void b_convert_ipaddr(uint8_t *c, uint32_t b);

NEXUS_Error NEXUS_Docsis_GetSystemInfo(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisSystemInfo *pSystemInfo)
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_DocsisDeviceHandle hDevice = NULL;
    BDCM_SystemInfo systemInfo;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSystemInfo);
    hDevice = (NEXUS_DocsisDeviceHandle)handle->pDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);
    BKNI_Memset(pSystemInfo, 0, sizeof(NEXUS_DocsisSystemInfo));
	BKNI_Memset(&systemInfo, 0, sizeof(BDCM_SystemInfo));
    if (hDevice->status.state != NEXUS_DocsisDeviceState_eOperational)
    {
        return (NEXUS_SUCCESS);
    }
    retCode = BDCM_GetDeviceSystemInfo(hDevice->hDocsis,&systemInfo);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("NEXUS_Docsis_GetSystemInfo failed"));
        return NEXUS_NOT_SUPPORTED;
    }
	BKNI_Memcpy(pSystemInfo->ecmMfctName,systemInfo.ecmMfctName,64);
	pSystemInfo->ecmMfctName[63] = '\0';
	BKNI_Memcpy(pSystemInfo->ecmMfctOUI,systemInfo.ecmMfctOUI,64);
	pSystemInfo->ecmMfctOUI[63] = '\0';
	BKNI_Memcpy(pSystemInfo->ecmMfctDate,systemInfo.ecmMfctDate,64);
	pSystemInfo->ecmMfctDate[63] = '\0';
	BKNI_Memcpy(pSystemInfo->ecmSwVersion,systemInfo.ecmSwVersion,64);
	pSystemInfo->ecmSwVersion[63] = '\0';
	BKNI_Memcpy(pSystemInfo->ecmHwVersion,systemInfo.ecmHwVersion,64);
	pSystemInfo->ecmHwVersion[63] = '\0';
	BKNI_Memcpy(pSystemInfo->ecmSerialNum,systemInfo.ecmSerialNum,64);
	pSystemInfo->ecmSerialNum[63] = '\0';
    switch(systemInfo.ecmStandard)
	{
	case 0:
		pSystemInfo->ecmStandard = NEXUS_DocsisStandard_e1_x;
		break;
	case 1:
		pSystemInfo->ecmStandard = NEXUS_DocsisStandard_e2_x;
		break;
	case 2:
		pSystemInfo->ecmStandard = NEXUS_DocsisStandard_e3_x;
		break;
	default:
		pSystemInfo->ecmStandard = NEXUS_DocsisStandard_eMax;
		break;
	}
    switch(systemInfo.ecmIpMode)
	{
	case 0:
		pSystemInfo->ecmIpMode = NEXUS_EcmIpMode_eNone;
		break;
	case 1:
		/*IPV4 LAN address*/
		pSystemInfo->ecmIpMode = NEXUS_EcmIpMode_eV4;
		break;
	case 2:
		/*IPV4 LAN address*/
		pSystemInfo->ecmIpMode = NEXUS_EcmIpMode_eV6;
		break;
	default:
		pSystemInfo->ecmIpMode = NEXUS_EcmIpMode_eMax;
		break;
	}
	/* LAN mac address*/
    pSystemInfo->ecmMacAddress[0] = (systemInfo.ecmMacAddressHi>>24)&0xff;
    pSystemInfo->ecmMacAddress[1] = (systemInfo.ecmMacAddressHi>>16)&0xff;
    pSystemInfo->ecmMacAddress[2] = (systemInfo.ecmMacAddressHi>>8)&0xff;
    pSystemInfo->ecmMacAddress[3] = (systemInfo.ecmMacAddressHi)&0xff;
    pSystemInfo->ecmMacAddress[4] = (systemInfo.ecmMacAddressLo>>24)&0xff;
    pSystemInfo->ecmMacAddress[5] = (systemInfo.ecmMacAddressLo>>16)&0xff;
    pSystemInfo->ecmMacAddress[6] = (systemInfo.ecmMacAddressLo>>8)&0xff;
    pSystemInfo->ecmMacAddress[7] = (systemInfo.ecmMacAddressLo)&0xff;
	/* IP address*/
	if (systemInfo.ecmIpMode == NEXUS_EcmIpMode_eV4 || systemInfo.ecmIpMode == NEXUS_EcmIpMode_eV6)
	{
		b_convert_ipaddr(pSystemInfo->ecmIpAddress, systemInfo.ecmIpAddress);
        b_convert_ipaddr(&pSystemInfo->ecmIpv6Address[0],systemInfo.ecmIpv6Address0);
        b_convert_ipaddr(&pSystemInfo->ecmIpv6Address[4],systemInfo.ecmIpv6Address1);
        b_convert_ipaddr(&pSystemInfo->ecmIpv6Address[8],systemInfo.ecmIpv6Address2);
        b_convert_ipaddr(&pSystemInfo->ecmIpv6Address[12],systemInfo.ecmIpv6Address3);
	}
	switch(systemInfo.ecmWapInterface)
	{
	case 0:
		pSystemInfo->ecmWapInterface = NEXUS_DocsisWapInterfaceType_eNone;
		break;
	case 1:
		pSystemInfo->ecmWapInterface = NEXUS_DocsisWapInterfaceType_e2_4g;
		break;
	case 2:
		pSystemInfo->ecmWapInterface = NEXUS_DocsisWapInterfaceType_e5g;
		break;
	case 3:
		pSystemInfo->ecmWapInterface = NEXUS_DocsisWapInterfaceType_eDual;
		break;
	default:
		pSystemInfo->ecmWapInterface = NEXUS_DocsisWapInterfaceType_eMax;
		break;
	}
	pSystemInfo->ecmMtaInfoAvailable = systemInfo.ecmHasMtaInfo;
	pSystemInfo->ecmRouterInfoAvailable = systemInfo.ecmHasRouterInfo;
    return retCode;
}

NEXUS_Error NEXUS_Docsis_GetWapStatus(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisWapInterfaceType wapIfType,
    NEXUS_DocsisWapStatus *pWapStatus)
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_DocsisDeviceHandle hDevice = NULL;
    BDCM_WapStatus *pDcmWapStatus = NULL;
	uint32_t i = 0;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pWapStatus);
    hDevice = (NEXUS_DocsisDeviceHandle)handle->pDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);

    BKNI_Memset(pWapStatus, 0, sizeof(NEXUS_DocsisWapStatus));
    if (hDevice->status.state != NEXUS_DocsisDeviceState_eOperational)
    {
        return (NEXUS_SUCCESS);
    }
	/* use malloc to avoid kernel stack overflow*/
	pDcmWapStatus = (BDCM_WapStatus *)BKNI_Malloc(sizeof(BDCM_WapStatus));
	if(!pDcmWapStatus)
	{
		BDBG_ERR(("Out of system memory"));
        return NEXUS_OUT_OF_SYSTEM_MEMORY;
	}
	BKNI_Memset(pDcmWapStatus, 0, sizeof(BDCM_WapStatus));
    retCode = BDCM_GetDeviceWapStatus(hDevice->hDocsis,
									  (BDCM_WapInterfaceType)wapIfType,
									  pDcmWapStatus);
    if (retCode != BERR_SUCCESS)
    {
        BKNI_Free(pDcmWapStatus);
        BDBG_ERR(("BDCM_GetDeviceWapStatus failed"));
        return NEXUS_NOT_SUPPORTED;
    }
	if (pDcmWapStatus->numberSSID > NEXUS_DOCSIS_MAX_SSID)
	{
		pDcmWapStatus->numberSSID = NEXUS_DOCSIS_MAX_SSID;
	}

	pWapStatus->numSsid = pDcmWapStatus->numberSSID;
    pWapStatus->wapMacAddress[0] = (pDcmWapStatus->wapMacAddressHi>>24)&0xff;
    pWapStatus->wapMacAddress[1] = (pDcmWapStatus->wapMacAddressHi>>16)&0xff;
    pWapStatus->wapMacAddress[2] = (pDcmWapStatus->wapMacAddressHi>>8)&0xff;
    pWapStatus->wapMacAddress[3] = (pDcmWapStatus->wapMacAddressHi)&0xff;
    pWapStatus->wapMacAddress[4] = (pDcmWapStatus->wapMacAddressLo>>24)&0xff;
    pWapStatus->wapMacAddress[5] = (pDcmWapStatus->wapMacAddressLo>>16)&0xff;
    pWapStatus->wapMacAddress[6] = (pDcmWapStatus->wapMacAddressLo>>8)&0xff;
    pWapStatus->wapMacAddress[7] = (pDcmWapStatus->wapMacAddressLo)&0xff;

	for (i=0; i < pDcmWapStatus->numberSSID; i++){
		NEXUS_DocsisWapSsidInfo *pSsidInfo = &pWapStatus->ssidInfo[i];
		BKNI_Memcpy(&pSsidInfo->ssid[0],&pDcmWapStatus->ssidInfo[i].SSID[0],64);
		pSsidInfo->ssid[63] = '\0';
		pSsidInfo->ssidEnabled = pDcmWapStatus->ssidInfo[i].ssidEnabled;
		pSsidInfo->ssidChannelNo = pDcmWapStatus->ssidInfo[i].ssidChannelNo;
		/* map wap protocol to nexus*/
		switch(pDcmWapStatus->ssidInfo[i].protocol)
		{
		case 0:
			pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_eUnused;
			break;
		case 1:
			pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_e80211b;
			break;
		case 2:
			pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_e80211g;
			break;
		case 3:
			pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_e80211Bg;
			break;
		case 4:
			pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_e80211Mixed;
			break;
		case 5:
			pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_e80211n;
			break;
		case 6:
			pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_e80211Ac;
			break;
		default:
			pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_eMax;
			break;
		}
		/* map wap security scheme to nexus*/
		switch(pDcmWapStatus->ssidInfo[i].security)
		{
		case 0:
			pSsidInfo->security = NEXUS_DocsisWapWifiSecurityType_eWpa;
			break;
		case 1:
			pSsidInfo->security = NEXUS_DocsisWapWifiSecurityType_eMixed;
			break;
		case 2:
			pSsidInfo->security = NEXUS_DocsisWapWifiSecurityType_eWpa2Aes;
			break;
		default:
			pSsidInfo->security = NEXUS_DocsisWapWifiSecurityType_eMax;
			break;
		}
		/* wap mac address*/
		pSsidInfo->macAddress[0] = (pDcmWapStatus->ssidInfo[i].macAddressHi>>24)&0xff;
		pSsidInfo->macAddress[1] = (pDcmWapStatus->ssidInfo[i].macAddressHi>>16)&0xff;
		pSsidInfo->macAddress[2] = (pDcmWapStatus->ssidInfo[i].macAddressHi>>8)&0xff;
		pSsidInfo->macAddress[3] = (pDcmWapStatus->ssidInfo[i].macAddressHi)&0xff;
		pSsidInfo->macAddress[4] = (pDcmWapStatus->ssidInfo[i].macAddressLo>>24)&0xff;
		pSsidInfo->macAddress[5] = (pDcmWapStatus->ssidInfo[i].macAddressLo>>16)&0xff;
		pSsidInfo->macAddress[6] = (pDcmWapStatus->ssidInfo[i].macAddressLo>>8)&0xff;
		pSsidInfo->macAddress[7] = (pDcmWapStatus->ssidInfo[i].macAddressLo)&0xff;
	}
	BKNI_Free(pDcmWapStatus);
    return retCode;
}


NEXUS_Error NEXUS_Docsis_GetMtaStatus(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisMtaStatus *pMtaStatus)

{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_DocsisDeviceHandle hDevice = NULL;
    BDCM_MtaStatus dcmMtaStatus;
	int i = 0;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pMtaStatus);
    hDevice = (NEXUS_DocsisDeviceHandle)handle->pDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);

    BKNI_Memset(pMtaStatus, 0, sizeof(NEXUS_DocsisMtaStatus));
    if (hDevice->status.state != NEXUS_DocsisDeviceState_eOperational)
    {
        return (NEXUS_SUCCESS);
    }

	BKNI_Memset(&dcmMtaStatus, 0, sizeof(BDCM_MtaStatus));
    retCode = BDCM_GetDeviceMtaStatus(hDevice->hDocsis,
									  &dcmMtaStatus);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDCM_GetDeviceMtaStatus failed"));
        return NEXUS_NOT_SUPPORTED;
    }

	pMtaStatus->mtaEnabled = dcmMtaStatus.iActive;
    pMtaStatus->mtaNumLines = dcmMtaStatus.iNumLines;
    for (i=0; i< dcmMtaStatus.iNumLines; i++)
    {
        pMtaStatus->mtaLinesInfo[i].mtaLineStatus = dcmMtaStatus.pLines[i].enStatus;
        BKNI_Memcpy(pMtaStatus->mtaLinesInfo[i].mtaNumber, dcmMtaStatus.pLines[i].szNumber,32);
        BKNI_Memcpy(pMtaStatus->mtaLinesInfo[i].mtaCallerId, dcmMtaStatus.pLines[i].szCallId,64);
    }
    b_convert_ipaddr(pMtaStatus->mtaExtIpv4Address, dcmMtaStatus.mtaExtIPv4Address);
    b_convert_ipaddr(&pMtaStatus->mtaExtIpv6Address[0], dcmMtaStatus.mtaExtIPv6Address0);
    b_convert_ipaddr(&pMtaStatus->mtaExtIpv6Address[4], dcmMtaStatus.mtaExtIPv6Address1);
    b_convert_ipaddr(&pMtaStatus->mtaExtIpv6Address[8], dcmMtaStatus.mtaExtIPv6Address2);
    b_convert_ipaddr(&pMtaStatus->mtaExtIpv6Address[12], dcmMtaStatus.mtaExtIPv6Address3);

    b_convert_ipaddr(pMtaStatus->mtaSipGatewayIpv4Address, dcmMtaStatus.mtaSipGatewayIPv4Address);
    b_convert_ipaddr(&pMtaStatus->mtaSipGatewayIpv6Address[0], dcmMtaStatus.mtaSipGatewayIPv6Address0);
    b_convert_ipaddr(&pMtaStatus->mtaSipGatewayIpv6Address[4], dcmMtaStatus.mtaSipGatewayIPv6Address1);
    b_convert_ipaddr(&pMtaStatus->mtaSipGatewayIpv6Address[8], dcmMtaStatus.mtaSipGatewayIPv6Address2);
    b_convert_ipaddr(&pMtaStatus->mtaSipGatewayIpv6Address[12], dcmMtaStatus.mtaSipGatewayIPv6Address3);

	return retCode;
}

NEXUS_Error NEXUS_Docsis_GetRouterStatus(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisRouterStatus *pRouterStatus)
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_DocsisDeviceHandle hDevice = NULL;
    BDCM_RouterStatus dcmRouterStatus;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pRouterStatus);
    hDevice = (NEXUS_DocsisDeviceHandle)handle->pDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);

    BKNI_Memset(pRouterStatus, 0, sizeof(NEXUS_DocsisRouterStatus));
    if (hDevice->status.state != NEXUS_DocsisDeviceState_eOperational)
    {
        return (NEXUS_SUCCESS);
    }
	BKNI_Memset(&dcmRouterStatus, 0, sizeof(BDCM_RouterStatus));
    retCode = BDCM_GetDeviceRouterStatus(hDevice->hDocsis,
									     &dcmRouterStatus);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDCM_GetDeviceRouterStatus failed"));
        return NEXUS_NOT_SUPPORTED;
    }

	pRouterStatus->routerEnabled = dcmRouterStatus.iActive;
    b_convert_ipaddr(pRouterStatus->routerExtIpv4Address, dcmRouterStatus.routerExtIPv4Address);
    b_convert_ipaddr(&pRouterStatus->routerExtIpv6Address[0],dcmRouterStatus.routerExtIPv6Address0);
    b_convert_ipaddr(&pRouterStatus->routerExtIpv6Address[4],dcmRouterStatus.routerExtIPv6Address1);
    b_convert_ipaddr(&pRouterStatus->routerExtIpv6Address[8],dcmRouterStatus.routerExtIPv6Address2);
    b_convert_ipaddr(&pRouterStatus->routerExtIpv6Address[12],dcmRouterStatus.routerExtIPv6Address3);

	return retCode;
}

NEXUS_Error NEXUS_Docsis_GetVsi(
    NEXUS_FrontendDeviceHandle handle,
    const NEXUS_DocsisVsiRequest *pVsiRequest,    /* [in] */
    NEXUS_DocsisVsi  *pVsi   /* [out] */
    )
{
#ifdef VENDOR_REQUEST
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_DocsisDeviceHandle hDevice=NULL;
    NEXUS_DocsisVsi venReply;
    BDBG_ASSERT(handle);
    hDevice = (NEXUS_DocsisDeviceHandle)handle->pDevice;
    BDBG_OBJECT_ASSERT(hDevice, NEXUS_DocsisDevice);

    BKNI_Memset(pVsi, 0, sizeof(NEXUS_DocsisVsi));
    retCode = BDCM_GetDeviceVsi(hDevice->hDocsis, (BDCM_Vsi_Request*)pVsiRequest, &venReply);

    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDCM_GetDeviceVsi failed"));
        return NEXUS_NOT_SUPPORTED;
    }

    BKNI_Memcpy(pVsi,&venReply,sizeof(NEXUS_DocsisVsi));
    return retCode;

#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pVsiRequest);
    BSTD_UNUSED(pVsi);
    BDBG_ERR(("NEXUS_Docsis_GetVsi Not Supported!"));
    return NEXUS_NOT_SUPPORTED;
#endif
}
