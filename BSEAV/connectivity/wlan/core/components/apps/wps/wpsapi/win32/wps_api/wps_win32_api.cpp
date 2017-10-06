/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wps_api.cpp,v 1.10 2009/01/15 01:31:04 Exp $
 */

// wps_api.cpp : Defines the entry point for the DLL application.
//

extern "C" {
#include "stdafx.h"
}

#include <comdef.h>
#include <ks.h>
#include "atlbase.h"
#include "wpshlp.h"
#include "wps_win32_api.h"
#include "wpscommondefines.h"
#include "wps_sdk.h"
#include "wps_api_osl.h"

#define WZC_PROFILE_SUCCESS							0
#define WZC_PROFILE_SET_ERROR_DUPLICATE_NETWORK		27

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

MIDL_INTERFACE("c96fbd51-24dd-11d8-89fb-00904b2ea9c6")
IProvisioningProfileWireless : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE CreateProfile( 
        /* [in] */ BSTR bstrXMLWirelessConfigProfile,
        /* [in] */ BSTR bstrXMLConnectionConfigProfile,
        /* [in] */ GUID *pAdapterInstanceGuid,
        /* [out] */ ULONG *pulStatus) = 0;
    
};

class DECLSPEC_UUID("2aa2b5fe-b846-4d07-810c-b21ee45320e3")
NetProvisioning;

_COM_SMARTPTR_TYPEDEF(IProvisioningProfileWireless, __uuidof(IProvisioningProfileWireless));


#define RADIOSTATE_SW_DISABLE	(1<<0)
#define RADIOSTATE_HW_DISABLE	(1<<1)

#define MGMTSTATE_RUN_BCM		(1<<0)
#define MGMTSTATE_RUN_WZC		(1<<1)
#define MGMTSTATE_RUN_ACM		(1<<2)

#define MGMTSTATE_MANAGE_NONE	0
#define MGMTSTATE_MANAGE_BCM	1
#define MGMTSTATE_MANAGE_WZC	2
#define MGMTSTATE_MANAGE_ACM	3

BOOL SidHlp_GetUserSid(LPCTSTR szUserName, PSID& rpsid);
BOOL SidHlp_GetCurrentSid(PSID& rpsid);
BOOL SidHlp_GetCurrentSid(LPTSTR& str);
BOOL SidHlp_GetUserSid(LPCTSTR lpAccountName, LPTSTR& str);

DWORD g_ManagementStateOld=0;

bool InitializeNetworkEnvironment()
{
	g_ManagementStateOld=0;
	if (!GetManagementState(NULL, &g_ManagementStateOld))
	{	// Restore to BCM adapter management if we can't get the current state
		g_ManagementStateOld = MAKELONG(MGMTSTATE_RUN_BCM, MGMTSTATE_MANAGE_BCM);
	}

	return SetManagementState(NULL, 0)? true : false;

}

bool RestoreNetworkEnvironment()

{
	// Restore adapter management
	return SetManagementState(NULL, g_ManagementStateOld)? true : false;
}

BOOL DoApplyNetworkAcm(const wps_credentials *cred)
{
	return wps_create_wlan_profile(wps_osl_get_short_adapter_name(), cred);
}

BOOL DoApplyNetworkBrcm(const wps_credentials *cred)
{
	BOOL bRet;
	PN_STATUS pnStatus = PN_FAILED;
	PN_AUTH eAuthType = PN_AUTH_OPEN;
	PN_CRYPT eCryptType = PN_CRYPT_NONE;
	PREF_ST prefAP;
	int ssidLen;
	LPTSTR szSid = NULL;

	USES_CONVERSION;

	InitializeBCM();

	// Look up auth type
	bRet = FALSE;
	for(int i=0; i<SIZEOF_ARRAY(g_mapOneXGenAuthType) && !bRet; i++)
	{
		if(bRet = (stricmp(g_mapOneXGenAuthType[i].szKeyMgmt, cred->keyMgmt) == 0))
			eAuthType = (PN_AUTH)(g_mapOneXGenAuthType[i].uiPreflib);
	}
	if(!bRet) return bRet;  // Failed to find auth type

	// Look up encryption type
	bRet = FALSE;
	for(int i=0; i<SIZEOF_ARRAY(g_mapOneXGenEncryType) && !bRet; i++)
	{
		if(bRet = (g_mapOneXGenEncryType[i].uiWpsEngine == cred->encrType))
			eCryptType = (PN_CRYPT)(g_mapOneXGenEncryType[i].uiPreflib);
	}
	if(!bRet) return bRet;  // Failed to find encryption type

	memset(&prefAP, 0, sizeof(PREF_ST));
	prefAP.dwLength = sizeof(PREF_ST);
	ssidLen = (int)strlen(cred->ssid);
	strncpy(prefAP.cSsid, cred->ssid, ssidLen);
	prefAP.cSsid[ssidLen]='\0';
	prefAP.dwSsidLength = ssidLen;
	prefAP.e_auth  = eAuthType;
	prefAP.e_crypt = eCryptType;
	strcpy(prefAP.cKey, (char *)cred->nwKey);
	prefAP.dwKeyLength = (DWORD)strlen(cred->nwKey);
	
	// Set key index for WEP mode
	if (cred->wepIndex>=2 && cred->wepIndex <=4)
		prefAP.iKeyindex = cred->wepIndex-1;

	pnStatus = BCMEncryptStr(prefAP.cKey, &prefAP.dwKeyLength);

	// add the new PN
	BOOL bIsUserPN;
	if (IsUserPN(&bIsUserPN) == TRUE && bIsUserPN)
	{
		//append_line(_T("\tstoring in user area\r\n"));
		SidHlp_GetCurrentSid(szSid);
	}
	else
	{
		//append_line(_T("\tstoring in machine area\r\n"));
	}

	// If there exists a PN with the same SSID, remove it.
	DWORD nIndex;
	if ((pnStatus == PN_SUCCESS) &&
		(FindPreferredNetwork(wps_osl_get_short_adapter_name(), T2CA(szSid), prefAP.cSsid, &nIndex)))
	{
		//append_line(_T("\tremoving existing network (ssid=%s)\r\n"), A2T(prefAP.cSsid));
		pnStatus = RemovePreferredNetwork(wps_osl_get_short_adapter_name(), T2CA(szSid), nIndex);
	}

	// add the new PN
	if ((pnStatus == PN_SUCCESS) &&
		(pnStatus = AddPreferredNetwork(wps_osl_get_short_adapter_name(), T2CA(szSid), &prefAP, 0)) == PN_SUCCESS)
	{
		///append_line(_T("\tnetwork saved\r\n"));
		//append_line(_T("Enabling adapter management..."));
		BOOL bStatus = SetManagementState(wps_osl_get_short_adapter_name(), MAKELONG(MGMTSTATE_RUN_BCM, MGMTSTATE_MANAGE_BCM));
		//append_line(_T("%s\r\n"), bStatus ? _T("succeeded") : _T("failed"));
		if (bStatus)
		{
			DWORD radioState;
			GetRadioState(wps_osl_get_short_adapter_name(), &radioState);
			if (radioState & (RADIOSTATE_SW_DISABLE | RADIOSTATE_HW_DISABLE))
			{
				//append_line(_T("Turning radio on\r\n"));
				SetRadioState(wps_osl_get_short_adapter_name(), 0);
			}
		}
	}
	else
	{
		//append_line(_T("\terror saving network (code=%d)\r\n"), pnStatus);
		return FALSE;
	}

	return TRUE;
}

static BOOL String2Guid(LPCTSTR szGuid, GUID& rGuid)
{
	const TCHAR *p;
	TCHAR lpsz[3];

	::ZeroMemory(&rGuid, sizeof(GUID));
	lpsz[2] = '\0';

	size_t len = _tcslen(szGuid);
	switch (len)
	{
	case 36: p = szGuid;	break;	// w/o braces
	case 38: p = szGuid+1;	break;	// w/braces
	default: p = NULL;		break;
	}

	if (p != NULL)
	{
		rGuid.Data1 = _tcstoul(p, const_cast<TCHAR**>(&p), 16);
		p++;
		rGuid.Data2 = (unsigned short) _tcstoul(p, const_cast<TCHAR**>(&p), 16);
		p++;
		rGuid.Data3 = (unsigned short) _tcstoul(p, const_cast<TCHAR**>(&p), 16);
		p++;

		for (int j=0; j < 8; j++)
		{
			_tcsncpy(lpsz, p, 2);
			rGuid.Data4[j] = (unsigned char) _tcstoul(lpsz, NULL, 16);
			p += 2;
			if (*p == '-')
				p++;
		}
	}

	return TRUE;
}

static BOOL BuildWZCProfileXml(const wps_credentials *cred, char *szProfileXml)
{
	BOOL bRet = TRUE;

	// NULL pinter validation
	if(!szProfileXml)
		return FALSE;

	// Define WZC XML templates 
	
	// Non-secure OPEN/No-WEP xml profile template
	// According to the WirelessProfile schema validation rule, <networkKey> element needs to be removed
	const char *cszTemplateXmlOpenNoWep = "<?xml version=\"1.0\" ?> \
											<WirelessProfile xmlns=\"http://www.microsoft.com/provisioning/WirelessProfile\"> \
												<version>1</version> \
												<ssid>%s</ssid> \
												<connectionType>ESS</connectionType> \
												<authentication>Open</authentication> \
												<encryption>None</encryption> \
												<keyProvidedAutomatically>0</keyProvidedAutomatically> \
												<IEEE802.1XEnabled>0</IEEE802.1XEnabled> \
											</WirelessProfile>";

	//Secure xml profile for WPAPSK and WPA2PSK
	const char *cszTemplateXmlSecureWPA = "<?xml version=\"1.0\" ?> \
										<WirelessProfile xmlns=\"http://www.microsoft.com/provisioning/WirelessProfile\"> \
											<version>1</version> \
											<ssid>%s</ssid> \
											<connectionType>ESS</connectionType> \
											<authentication>%s</authentication> \
											<encryption>%s</encryption> \
											<networkKey>%s</networkKey> \
											<keyProvidedAutomatically>0</keyProvidedAutomatically> \
											<IEEE802.1XEnabled>0</IEEE802.1XEnabled> \
										</WirelessProfile>";

	// Secure xml profile for WEP mode
	const char *cszTemplateXmlSecureWEP = "<?xml version=\"1.0\" ?> \
										<WirelessProfile xmlns=\"http://www.microsoft.com/provisioning/WirelessProfile\"> \
											<version>1</version> \
											<ssid>%s</ssid> \
											<connectionType>ESS</connectionType> \
											<authentication>%s</authentication> \
											<encryption>%s</encryption> \
											<networkKey>%s</networkKey> \
											<keyIndex>%d</keyIndex> \
											<keyProvidedAutomatically>0</keyProvidedAutomatically> \
											<IEEE802.1XEnabled>0</IEEE802.1XEnabled> \
										</WirelessProfile>";


	// Build profile xml by template
	if(stricmp(cred->keyMgmt, KEY_MGMT_OPEN) == 0 && cred->encrType == WPS_ENCRTYPE_NONE)
	{
		// Open/Non-WEP
		sprintf(szProfileXml, cszTemplateXmlOpenNoWep, cred->ssid);
	}
	else
	{
		// Secure types

		char szAuthType[16] = { '\0' }, szEncryptType[16] = { '\0' };

		// Look up auth type
		bRet = FALSE;
		for(int i=0; i<SIZEOF_ARRAY(g_mapOneXGenAuthType) && !bRet; i++)
		{
			if(bRet = (stricmp(g_mapOneXGenAuthType[i].szKeyMgmt, cred->keyMgmt) == 0))
				strcpy(szAuthType, g_mapOneXGenAuthType[i].szProvSvcXmlElementSTR);
		}

		if(bRet)
		{
			// Look up encryption type
			bRet = FALSE;
			for(int i=0; i<SIZEOF_ARRAY(g_mapOneXGenEncryType) && !bRet; i++)
			{
				if(bRet = (g_mapOneXGenEncryType[i].uiWpsEngine == cred->encrType))
					strcpy(szEncryptType, g_mapOneXGenEncryType[i].szProvSvcXmlElementSTR);
			}
			
			if(bRet)
			{
				// Both authentication type and encryption type are find, create the profile by template
				sprintf(szProfileXml, cszTemplateXmlSecureWPA, cred->ssid, szAuthType, szEncryptType, cred->nwKey);
			}

			if(bRet)
			{
				// Both authentication type and encryption type are find, create the profile by template
				if (cred->encrType == WPS_ENCRTYPE_WEP)
					sprintf(szProfileXml, cszTemplateXmlSecureWEP, cred->ssid, szAuthType, szEncryptType, cred->nwKey,
						(cred->wepIndex >= 2 && cred->wepIndex <= 4)? cred->wepIndex : 1);  // Set WEP key index
				else
					sprintf(szProfileXml, cszTemplateXmlSecureWPA, cred->ssid, szAuthType, szEncryptType, cred->nwKey);				
			}
		}
	}

	return bRet;
}

INT DoApplyNetworkWzc(const wps_credentials *cred)
{
	BOOL bRet;
	ULONG ulStatus;
	GUID guid;
	
	CoInitialize(NULL);

	IProvisioningProfileWirelessPtr	spIWirelessProfile;
	try
	{
		bRet = SUCCEEDED(spIWirelessProfile.CreateInstance(__uuidof(NetProvisioning)));
	}
	catch(_com_error&)
	{
		bRet = FALSE;
	}
	
	if(!bRet) 
		return bRet;

	char szXmlWirelessProfile[XML_PROFILE_MAX_LEN];
	if(!(bRet = BuildWZCProfileXml(cred, szXmlWirelessProfile)))
		return bRet;  // Failed to create WZC xml profile

	String2Guid(wps_osl_get_short_adapter_name(), guid);
	try
	{
		HRESULT hResult = spIWirelessProfile->CreateProfile(_bstr_t(szXmlWirelessProfile), NULL, &guid, &ulStatus);
		if (FAILED(hResult))
		{
			// Considered successful if the same network profile exists already
			bRet = ((ulStatus == WZC_PROFILE_SET_ERROR_DUPLICATE_NETWORK) ? TRUE : FALSE);
		}
		else
		{
			// Successful
			bRet = TRUE;
		}
	}
	catch (_com_error&)
	{
		bRet = FALSE;
	}

	return bRet;
}

BOOL IsManagedByWZC()
{

	BOOL bRet;
	DWORD dwMs;

	if (GetManagementState(wps_osl_get_short_adapter_name(), &dwMs))
	{
		bRet = TRUE;
		switch (LOWORD(dwMs))
		{
		case MGMTSTATE_MANAGE_BCM:
			bRet = FALSE;
			break;
		case MGMTSTATE_MANAGE_WZC:
			bRet=TRUE;
			break;
		default:
			bRet = FALSE;
		}
	}
	else
		bRet = FALSE;

	return bRet;
}

BOOL DoApplyNetwork(const wps_credentials *cred)
{
	BOOL bRet = FALSE;
	DWORD dwMs;

	if(wps_osl_wksp->os_ver == VER_VISTA)
	{
		bRet = DoApplyNetworkAcm(cred);
	}
	else
	{
		RestoreNetworkEnvironment();

		if (GetManagementState(wps_osl_get_short_adapter_name(), &dwMs))
		{
			bRet = TRUE;
			switch (LOWORD(dwMs))
			{
			case MGMTSTATE_MANAGE_BCM:
				bRet = DoApplyNetworkBrcm(cred);
				break;
			case MGMTSTATE_MANAGE_WZC:
				bRet = DoApplyNetworkWzc(cred);
				break;
			default:
				bRet = FALSE;
			}
		}
		else
			bRet = FALSE;

		InitializeNetworkEnvironment();
	}

	return bRet;
}
