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
#ifndef NEXUS_DOCSIS_INFO_H__
#define NEXUS_DOCSIS_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Enumeration for DOCSIS eCM IP mode
***************************************************************************/
typedef enum NEXUS_EcmIpMode
{
    NEXUS_EcmIpMode_eNone,
    NEXUS_EcmIpMode_eV4,
    NEXUS_EcmIpMode_eV6,
    NEXUS_EcmIpMode_eMax
} NEXUS_EcmIpMode;

#define NEXUS_DOCSIS_MAX_SSID (32)
#define NEXUS_DOCSIS_MAX_MTA  (8)
#define NEXUS_DOCSIS_MAX_VSI_LEN (32)  /* 1 unit =  4 bytes */

typedef enum NEXUS_DocsisStandard
{
    NEXUS_DocsisStandard_e1_x, /* DOCSIS 1.x standard compliant */
    NEXUS_DocsisStandard_e2_x, /* DOCSIS 2.x standard compliant */
    NEXUS_DocsisStandard_e3_x, /* DOCSIS 3.x standard compliant */
    NEXUS_DocsisStandard_eMax
}NEXUS_DocsisStandard;

typedef enum NEXUS_DocsisWapWifiProtocolType
{
    NEXUS_DocsisWapWifiProtocolType_eUnused, /* frequency not used */
    NEXUS_DocsisWapWifiProtocolType_e80211b, /* 802.11b  (2.4GHz only) */
    NEXUS_DocsisWapWifiProtocolType_e80211g, /* 802.11g  (2.4GHz only) */
    NEXUS_DocsisWapWifiProtocolType_e80211Bg, /* B and G (2.4GHz only) */
    NEXUS_DocsisWapWifiProtocolType_e80211Mixed, /* B/G/N mixed mode (2.4GHz only) */
    NEXUS_DocsisWapWifiProtocolType_e80211n, /* 802.11n  (2.4GHz and 5.0GHz) */
    NEXUS_DocsisWapWifiProtocolType_e80211Ac, /* 802.11ac (5.0GHz only) */
    NEXUS_DocsisWapWifiProtocolType_eMax
}NEXUS_DocsisWapWifiProtocolType;

typedef enum NEXUS_DocsisWapWifiSecurityType
{
    NEXUS_DocsisWapWifiSecurityType_eWpa, /* WIFI Protected Access (partial 802.11i) */
    NEXUS_DocsisWapWifiSecurityType_eMixed, /* WIFI Protected Access (full 802.11i-2004) CCMP+TKIP */
    NEXUS_DocsisWapWifiSecurityType_eWpa2Aes, /* WIFI Protected Access (full 802.11i-2004) CCMP/AES only */
    NEXUS_DocsisWapWifiSecurityType_eMax
}NEXUS_DocsisWapWifiSecurityType;

typedef struct NEXUS_DocsisWapSsidInfo
{
    char ssid[64]; /* SSID, up to 32 characters (as a NULL terminated string) */
    bool ssidEnabled;
    unsigned ssidChannelNo; /* Wireless channel */
    NEXUS_DocsisWapWifiProtocolType protocol; /* Wireless protocol */
    NEXUS_DocsisWapWifiSecurityType security; /* Security scheme */
    uint8_t macAddress[8]; /* MAC address per SSID */
}NEXUS_DocsisWapSsidInfo;

typedef struct NEXUS_DocsisWapStatus
{
    unsigned numSsid; /* Number of configured SSIDs */
    uint8_t wapMacAddress[8]; /* Wap base MAC address  */
    NEXUS_DocsisWapSsidInfo ssidInfo[NEXUS_DOCSIS_MAX_SSID];
}NEXUS_DocsisWapStatus;

typedef enum NEXUS_DocsisWapInterfaceType
{
    NEXUS_DocsisWapInterfaceType_eNone, /* No WAP */
    NEXUS_DocsisWapInterfaceType_e2_4g, /* WAP 2.4GHz */
    NEXUS_DocsisWapInterfaceType_e5g, /* WAP 5GHz */
    NEXUS_DocsisWapInterfaceType_eDual, /* WAP dual 2.4GHz and 5GHz */
    NEXUS_DocsisWapInterfaceType_eMax
}NEXUS_DocsisWapInterfaceType;

typedef struct NEXUS_DocsisSystemInfo
{
    char ecmMfctName[64]; /* Manufacturer name (as a NULL terminated string) */
    char ecmMfctOUI[64];  /* The manufacturer Organisationally Unique Identifier (OUI) (as a NULL terminated string) */
    char ecmMfctDate[64]; /* The manufacture date (as a NULL terminated string) */
    char ecmSwVersion[64]; /* The software version (as a NULL terminated string) */
    char ecmHwVersion[64]; /* The hardware version (as a NULL terminated string) */
    char ecmSerialNum[64];  /* The serial number (as a NULL terminated string) */
    NEXUS_DocsisStandard ecmStandard;
    NEXUS_EcmIpMode ecmIpMode;
    uint8_t ecmMacAddress[8]; /* eCM LAN MAC address */
    uint8_t ecmIpAddress[4]; /* eCM LAN IPV4 address */
    uint8_t ecmIpv6Address[32]; /* eCM LAN IPV6 address */
    NEXUS_DocsisWapInterfaceType ecmWapInterface;
    bool ecmMtaInfoAvailable; /* eCM Multimedia Terminal Adaptor status*/
    bool ecmRouterInfoAvailable; /* eCM Router status*/
}NEXUS_DocsisSystemInfo;

typedef enum NEXUS_DocsisMtaLineStatus
{
    NEXUS_DocsisMtaLineStatus_eNone, /* Phone line is not allocated for use */
    NEXUS_DocsisMtaLineStatus_eOn, /* Phone line provisioned for use, but not busy */
    NEXUS_DocsisMtaLineStatus_eOff, /* Phone is off hook, but no call in progress */
    NEXUS_DocsisMtaLineStatus_eRinging, /* Phone ringing due to incoming call */
    NEXUS_DocsisMtaLineStatus_eActive, /* phone call in progress */
    NEXUS_DocsisMtaLineStatus_eMax
}   NEXUS_DocsisMtaLineStatus;

typedef struct NEXUS_DocsisMtaLineInfo
{
    NEXUS_DocsisMtaLineStatus mtaLineStatus; /* Phone line status */
    char mtaNumber[32]; /* Phone number of this line ((as a NULL terminated string) )*/
    char mtaCallerId[64]; /* Phone number of caller or callee ((as a NULL terminated string) )*/
}   NEXUS_DocsisMtaLineInfo;

typedef struct NEXUS_DocsisMtaStatus
{
    bool mtaEnabled;
    unsigned mtaNumLines; /* Number of phones lines */
    NEXUS_DocsisMtaLineInfo mtaLinesInfo[NEXUS_DOCSIS_MAX_MTA]; /* info per phone line */
    uint8_t mtaExtIpv4Address[4]; /* IPv4 address of the external (WAN) side */
    uint8_t mtaExtIpv6Address[32]; /* IPv6 address of the external (WAN) side */
    uint8_t mtaSipGatewayIpv4Address[4]; /* IPv4 address of the SIP gateway */
    uint8_t mtaSipGatewayIpv6Address[32]; /* IPv6 address of the SIP gateway */
}   NEXUS_DocsisMtaStatus;

typedef struct NEXUS_DocsisRouterStatus
{
    bool routerEnabled; /* router enabled = true disabled = false */
    uint8_t routerExtIpv4Address[4]; /* IPv4 address of the external (WAN) side */
    uint8_t routerExtIpv6Address[32]; /* IPv6 address of the external (WAN) side */
}   NEXUS_DocsisRouterStatus;

/***************************************************************************
Summary:
Docsis VSI (Vendor Specific Info) request based on the customer specific request type.
There could be more than one VSI types. Customer defines the VSI types.
***************************************************************************/
typedef struct NEXUS_DocsisVsiRequest
{
 unsigned vsiRequestType;          /* defined by the customer enumeration */
  unsigned vsiRequestLen;            /*  VSI Request len */
  unsigned vsiRequest[NEXUS_DOCSIS_MAX_VSI_LEN];
} NEXUS_DocsisVsiRequest;

/***************************************************************************
Summary:
Docsis VSI (Vendor Specific Info)  based on Docsis VSI request type (see NEXUS_DocsisVsiRequest)

***************************************************************************/
typedef struct NEXUS_DocsisVsi
{
  unsigned vsiLen;            /*  VSI  length */
  unsigned vsi[NEXUS_DOCSIS_MAX_VSI_LEN];
} NEXUS_DocsisVsi;


/***************************************************************************
Summary:
	This function gets DOCSIS device system info
****************************************************************************/
NEXUS_Error NEXUS_Docsis_GetSystemInfo(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisSystemInfo *pSystemInfo /* [out] */
    );

/***************************************************************************
Summary:
	This function gets the status of WAP associated with a DOCSIS device
****************************************************************************/
NEXUS_Error NEXUS_Docsis_GetWapStatus(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisWapInterfaceType wapIfType,
    NEXUS_DocsisWapStatus *pWapStatus /* [out] */
    );

/*************************************************************************************
Summary:
	This function gets the status of a phone line associated with a DOCSIS device
**************************************************************************************/
NEXUS_Error NEXUS_Docsis_GetMtaStatus(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisMtaStatus *pMtaStatus /* [out] */
    );

/***************************************************************************
Summary:
	This function gets the status of a router associated with a DOCSIS device
****************************************************************************/
NEXUS_Error NEXUS_Docsis_GetRouterStatus(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisRouterStatus *pRouterStatus /* [out] */
    );

/***************************************************************************
Summary:
Get VSI (vendor specific information) for a DOCSIS device based on a specific
VSI request type.
***************************************************************************/
NEXUS_Error NEXUS_Docsis_GetVsi(
    NEXUS_FrontendDeviceHandle handle,
    const NEXUS_DocsisVsiRequest *pVsiRequest,    /* [in] */
    NEXUS_DocsisVsi  *pVsi   /* [out] */
    );

#ifdef __cplusplus
    }
#endif

#endif /* #ifndef NEXUS_DOCSIS_INFO_H__ */
