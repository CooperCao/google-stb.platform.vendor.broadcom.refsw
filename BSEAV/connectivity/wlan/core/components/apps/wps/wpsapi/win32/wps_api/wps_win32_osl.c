/* 
 * Copyright (C) 2011, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: $
 */

#include "stdafx.h"

#include <windows.h>
#include <wlanapi.h>
#include <ks.h>
#include <tchar.h>

#include "epictrl.h"
#include "wpscommondefines.h"
#include "wps_win32_api.h"

#include "wlioctl.h"
#include "wps_sdk.h"
#include "wps_api_osl.h"

extern void RAND_windows_init();

static DWORD wps_GetOSVer();

static int QueryOid(HANDLE irh, LPCTSTR cszAdapterName, ULONG oid, void* results, ULONG nbytes);
static int SetOid(ULONG oid, void* data, ULONG nbytes);

/* constant definition on WPS LED behaviors */
#define LED_2_WPS_START			2
#define LED_3_WPS_END			3

#define LED_BLINK_FREQ_NEGOTIATING		0x00C800C8
#define LED_BLINK_FREQ_ERROR_OTHERS		0x00640064
#define LED_BLINK_FREQ_ERROR_OVERLAP	0x00200020
#define LED_BLINK_FREQ_OFF				0x0


#ifndef WLAN_CONNECTION_EAPOL_PASSTHROUGH_BIT
#define WLAN_CONNECTION_EAPOL_PASSTHROUGH_BIT 0x8
#endif

#define WLANAPI_DLL_FILENAME TEXT("wlanapi.dll")

static const __int64 SECS_BETWEEN_EPOCHS = 11644473600;
static const __int64 SECS_TO_100NS = 10000000; /* 10^7 */

WPS_OSL_T *wps_osl_wksp = NULL;

/* Debug Output */
#ifdef _DEBUG
void WpsDebugOutput(LPCSTR fmt, ...)
{
	CHAR szString[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf(szString, fmt, args);
	OutputDebugString(szString);
}
#else
void WpsDebugOutput(LPCTSTR fmt, ...) {}
#endif


/* ########## */
/* Local static APIs */
/* ########## */
/* WL ioctl implementation */
static int
_wps_osl_wl_ioctl(HANDLE irh, LPCTSTR cszAdapterName, int cmd, void *buf, int len, bool set)
{
	DWORD dwlen = len;
	int error, oid_wl_cmd = WL_OID_BASE + cmd;
	WINERR err = ERROR_SUCCESS;

	if (!set)
		error = QueryOid(irh, cszAdapterName, oid_wl_cmd, buf, dwlen);
	else
		error = SetOid(oid_wl_cmd, buf, dwlen);

	return error;
}

static int
_wps_osl_ioctl_get(HANDLE irh, LPCTSTR cszAdapterName, int cmd, void *buf, int len)
{
	return _wps_osl_wl_ioctl(irh, cszAdapterName, cmd, buf, len, FALSE);
}

static int
_wps_osl_ioctl_set(int cmd, void *buf, int len)
{
	return _wps_osl_wl_ioctl(NULL, NULL, cmd, buf, len, TRUE);
}

/*
 * format an iovar buffer
 * iovar name is converted to lower case
 */
static uint
_wps_osl_iovar_mkbuf(const char *name, char *data, uint datalen, char *iovar_buf, uint buflen, int *perr)
{
	uint iovar_len;
	char *p;

	iovar_len = (uint) strlen(name) + 1;

	/* check for overflow */
	if ((iovar_len + datalen) > buflen) {
		*perr = -1;
		return 0;
	}

	/* copy data to the buffer past the end of the iovar name string */
	if (datalen > 0)
		memmove(&iovar_buf[iovar_len], data, datalen);

	/* copy the name to the beginning of the buffer */
	strcpy(iovar_buf, name);

	/* wl command line automatically converts iovar names to lower case for
	 * ease of use
	 */
	p = iovar_buf;
	while (*p != '\0') {
		*p = tolower((int)*p);
		p++;
	}

	*perr = 0;
	return (iovar_len + datalen);
}

/*
 * set named iovar providing both parameter and i/o buffers
 * iovar name is converted to lower case
 */
static int
_wps_osl_iovar_setbuf(const char *iovar, void *param, int paramlen, void *bufptr, int buflen)
{
	int err;
	int iolen;

	iolen = _wps_osl_iovar_mkbuf(iovar, param, paramlen, bufptr, buflen, &err);
	if (err)
		return err;

	return _wps_osl_ioctl_set(WLC_SET_VAR, bufptr, iolen);
}

/*
 * set named iovar given the parameter buffer
 * iovar name is converted to lower case
 */
static int
_wps_osl_iovar_set(const char *iovar, void *param, int paramlen)
{
	char smbuf[WLC_IOCTL_SMLEN];

	memset(smbuf, 0, sizeof(smbuf));
	return _wps_osl_iovar_setbuf(iovar, param, paramlen, smbuf, sizeof(smbuf));
}

static void
display_err(PCHAR prefix, DWORD status)
{
	PCHAR   ErrStr;
	DWORD   ErrLen;

	ErrLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, status,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &ErrStr, 0, NULL);
	if (ErrLen > 0) {
		fprintf(stderr, "Error 0x%x: %s -- %s\n", status, prefix, ErrStr);
		LocalFree(ErrStr);
	} else
		fprintf(stderr, "Error 0x%x: %s -- Unknown error\n", status, prefix);
}

static WPS_OSL_T *
_wps_osl_alloc_wksp()
{
	WPS_OSL_T *wksp;

	/* Allocate wps_osl_init */
	if ((wksp = (WPS_OSL_T *)malloc(sizeof(WPS_OSL_T))) != NULL)
		memset(wksp, 0, sizeof(WPS_OSL_T));

	return wksp;
}

static bool
_wps_osl_init_adapter(const char *adapter_guid, adapter_info *pAdapterInfo)
{
	bool bRet = false;
	HANDLE irh = NULL;
	WINERR err = ERROR_SUCCESS;
	PADAPTER pdev = NULL;
	DWORD adapter_indx;
	DWORD status= ERROR_SUCCESS;
    DWORD instance;
	int len;
  	wl_instance_info_t instanceInfo;

	if(pAdapterInfo == NULL) 
		return bRet;

	/* initialize irelay and select default adapter */
	irh = INVALID_HANDLE_VALUE;

	if ((err = ir_init(&irh)) != ERROR_SUCCESS)
		return bRet;

	wps_osl_wksp->ndevs = ARRAYSIZE(wps_osl_wksp->devlist);
	if ((err = ir_adapter_list(irh, &wps_osl_wksp->devlist[0], &wps_osl_wksp->ndevs)) !=
		ERROR_SUCCESS)
		goto END;

	for (adapter_indx = 0; adapter_indx < wps_osl_wksp->ndevs &&!bRet; adapter_indx++) {
		pdev = &wps_osl_wksp->devlist[adapter_indx];
		if(adapter_guid == NULL) {
			// No adpater GUID passed in, select the first wireless adapter in the enumeration list
			if (pdev->type == IR_WIRELESS) {
				bRet = true;
			}
			else if(pdev->type == IR_UNKNOWN) {
				// Additional way to determine adapter type on Vista
				if(wps_osl_wksp->os_ver == VER_VISTA) {
					/* On Vista, in order to determine that this is a broadcom wireless adapter,
					   do more than just using WLC_GET_INSTANCE since other broadcom
					   adapters like ethernet seem to implement this OID. Use the WLC_GET_MAGIC
					   OID to ascertain as an additional mechanism. */
					_tcscpy(pAdapterInfo->shortadaptername, pdev->name);
					len = sizeof(instance);
					status = _wps_osl_ioctl_get(irh, pdev->name, WLC_GET_MAGIC,
						&instance, len);
					if (status == ERROR_SUCCESS && instance == WLC_IOCTL_MAGIC) {
						memset( &instanceInfo, 0, sizeof(wl_instance_info_t) );
						len = sizeof(wl_instance_info_t);
						status = _wps_osl_ioctl_get(irh, pdev->name, WLC_GET_INSTANCE,
							&instanceInfo, len);
						if (status == ERROR_SUCCESS) {
							pdev->type = IR_WIRELESS;
							bRet = true;
						}
					}
				}
			}
		}
		else {
			// Adapter GUID is specified, select the adapter matching the GUID. If no maching,
			// return failure
			if(strcmp(pdev->name, adapter_guid) == 0) {
				// Adapter is found, fill information to AdapterInfo
				_tcscpy(pAdapterInfo->shortadaptername, pdev->name);
				bRet = true;
			}
		}
	}

	if(bRet) {
		// Wlan adapter is found
		status = ir_bind(irh, pdev->name);
		if (status == ERROR_SUCCESS) {
			// Succeeded to initialize adapter and copy over adapter info
			pAdapterInfo->irh = irh;
			memcpy(pAdapterInfo->macaddress, pdev->macaddr, 6);
			_tcscpy(pAdapterInfo->adaptername, _T("\\Device\\NPF_"));
			_tcscat(pAdapterInfo->adaptername, pdev->name);
			_tcscpy(pAdapterInfo->shortadaptername, pdev->name);
			bRet = true;
		}
		else {
			display_err("select_adapter:ir_bind failed: status=", status);
		}
	}
	else {
		fprintf(stderr, "No wireless adapters were found!\n");
	}

END:
	if(!bRet) {
		if (irh != INVALID_HANDLE_VALUE) {
			ir_unbind(irh);
			ir_exit(irh);
		}
	}
	return bRet;
}

static bool
_wps_osl_is_wps_led_enabled()
{
	int8 nWpsLed;
	char cBuf[WLC_IOCTL_MAXLEN] = { 0 };
	DWORD dwBufLen = WLC_IOCTL_MAXLEN;  // WLC_IOCTL_MAXLEN size is required for this IOCTL call
	DWORD dwRel;

	// "wl wpsled" will return 3 or more values:
	// "-1" - LED OPT not programmed implying WPS LED is disabled 
	// "0" -LED OPT is programmed, but WPS LED is disabled
	// "1" - LED OPT is pgrammed and WPS LED is enabled.
	// "2" (or other values) : Reserved for future use defining different LED behaviors, currently not implemented.
	strcpy(cBuf, "wpsled");
	dwRel = wps_osl_wl_ioctl(WLC_GET_VAR, cBuf, dwBufLen, FALSE);
	if(dwRel == NO_ERROR)
		memcpy(&nWpsLed, cBuf, sizeof(nWpsLed));  // Get "wpsled" value
	else
		return false;  // Failed to retrieve "wpsled" value, implying wps led is disabled
		
	return (nWpsLed >= 1);
}

static char *
get_scan_results_xp(char *buf, int buf_len)
{
    int ret;
    wl_scan_params_t* params;
    wl_scan_results_t *list = (wl_scan_results_t*)buf;
    int params_size = WL_SCAN_PARAMS_FIXED_SIZE + WL_NUMCHANNELS * sizeof(uint16);
   
    params = (wl_scan_params_t*)malloc(params_size);
    if (params == NULL) {
        fprintf(stderr, "Error allocating %d bytes for scan params\n", params_size);
        return NULL;
    }
    
    memset(params, 0, params_size);
    params->bss_type = DOT11_BSSTYPE_ANY;
    memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
    params->scan_type = -1;
    params->nprobes = -1;
    params->active_time = -1;
    params->passive_time = -1;
    params->home_time = -1;
    params->channel_num = 0;
    
    ret = wps_osl_wl_ioctl(WLC_SCAN, params, params_size, TRUE); 
    if (ret < 0)
	    return NULL;

    WpsSleep(2);

    list->buflen = buf_len;
    ret = _wps_osl_ioctl_get(wps_osl_wksp->stAdapterInfo.irh,
		wps_osl_wksp->stAdapterInfo.adaptername, WLC_SCAN_RESULTS, buf, buf_len);
    free(params);

    if (ret < 0)
	    return NULL;

     return buf;
}

static char *
ValidIfName(const char *ifname)
{
	static char ValidACMName[256];
	unsigned int i;

	strcpy(ValidACMName,ifname);

	if(ValidACMName[0]=='{')
	{
		for(i=0;i<strlen(ValidACMName)-1;i++)
		{
			ValidACMName[i]=ValidACMName[i+1];
		}
		ValidACMName[strlen(ValidACMName)-2]='\0';
	}

	return ValidACMName;
}

/* ###### */
/* For VISTA */
/* ###### */
static int
wps_dot11_scan(char const *ifname)
{
	int ret = -1;

	DWORD dwError = ERROR_SUCCESS;
	DWORD dwNegotiatedVersion;
	HANDLE hClient = NULL;
    GUID guidIntf;
	BOOL bTMP=TRUE;

	// open a handle to the service
	dwError = WlanOpenHandle(2,NULL,&dwNegotiatedVersion,&hClient);
	if (dwError == ERROR_SUCCESS)
	{
		// get the interface GUID
		if (UuidFromString(ValidIfName(ifname), &guidIntf) == RPC_S_OK)
		{
			// Start scan
			if(WlanScan(hClient,&guidIntf,NULL,NULL,NULL)!=ERROR_SUCCESS)
				WpsDebugOutput("ERROR: WlanScan failed\n");

			ret = 0;
		}
		else
		{
			dwError = GetLastError();
			printf("ERROR: UuidFromString failed (%s)\n",ValidIfName(ifname));
		}

		if (hClient != NULL) WlanCloseHandle(hClient, NULL);
	}
	else
	{
		WpsDebugOutput("ERROR: WlanOpenHandle failed\n");
	}

	return ret;
}

static int
freq2channel(DWORD freq)
{
	int ch;

	if (freq <= 2484) {
		/* 11b */
		if (freq == 2484)
			ch = 14;
		else
			ch = (freq - 2407) / 5;
	} else {
		/* 11a */
		ch = (freq - 5000) / 5;
		if (ch < 0)
			ch += 256;
	}

	return ch;
}

/* ###### */
/* For VISTA */
/* ###### */
static wl_scan_results_t *
scan_list_acm_to_wl(PWLAN_BSS_LIST const pBssidList, char *buf, int buf_len)
{
	ULONG i, wl_list_length, ielength;
	wl_bss_info_t *pWlBssInfo;
	wl_scan_results_t *pWlScanList = (wl_scan_results_t *)buf;
    PBYTE pIe = NULL;

	if (pBssidList)
	{
		/* calculate how much memory we need to allocate for
		 * the IEs in the WL equivalent structure
		 */
		ielength = 0;
		for (i=0; i < pBssidList->dwNumberOfItems; i++)
		{
			/* DONT strip off the NDIS fixed IEs */
				ielength += pBssidList->wlanBssEntries[i].ulIeSize;// - sizeof(NDIS_802_11_FIXED_IEs);
		}

		wl_list_length = WL_SCAN_RESULTS_FIXED_SIZE + (sizeof(wl_bss_info_t)*pBssidList->dwNumberOfItems) + ielength;
		if (pWlScanList)
		{
			pWlScanList->buflen = wl_list_length;
			pWlScanList->version = WL_BSS_INFO_VERSION;
			pWlScanList->count = pBssidList->dwNumberOfItems;

			/* begin copying the bssinfo elements */
			pWlBssInfo = (wl_bss_info_t *)&pWlScanList->bss_info[0];
			for (i=0; i < pBssidList->dwNumberOfItems; i++)
			{
				ielength = /*pBssidList->wlanBssEntries[i].ulIeSize > sizeof(NDIS_802_11_FIXED_IEs) ?*/
					pBssidList->wlanBssEntries[i].ulIeSize/* - sizeof(NDIS_802_11_FIXED_IEs) : 0*/;

				memset(pWlBssInfo, 0, sizeof(wl_bss_info_t) + ielength);

				pWlBssInfo->version = WL_BSS_INFO_VERSION;
				pWlBssInfo->length = sizeof(wl_bss_info_t) + ielength;
				memcpy(&pWlBssInfo->BSSID, pBssidList->wlanBssEntries[i].dot11Bssid, sizeof(pBssidList->wlanBssEntries[i].dot11Bssid));
				pWlBssInfo->beacon_period = (uint16)pBssidList->wlanBssEntries[i].usBeaconPeriod;
				if (pBssidList->wlanBssEntries[i].usCapabilityInformation & DOT11_CAPABILITY_INFO_PRIVACY)
					pWlBssInfo->capability |= DOT11_CAP_PRIVACY; 
				switch (pBssidList->wlanBssEntries[i].dot11BssType)
				{
				case dot11_BSS_type_independent:
					pWlBssInfo->capability |= DOT11_CAP_IBSS;
					break;
				case dot11_BSS_type_infrastructure:
					pWlBssInfo->capability |= DOT11_CAP_ESS;
					break;
				}
				pWlBssInfo->SSID_len = (uint8)pBssidList->wlanBssEntries[i].dot11Ssid.uSSIDLength;
				memcpy(pWlBssInfo->SSID, pBssidList->wlanBssEntries[i].dot11Ssid.ucSSID, pWlBssInfo->SSID_len);
				
				/*
				for(j=0; j < ARRAYSIZE(pBssid->SupportedRates) && pBssid->SupportedRates[j] != 0; j++)
					pWlBssInfo->rateset.rates[j] = pBssid->SupportedRates[j];
				*/

				pWlBssInfo->rateset.count = 0; //j;
				
				// Set channel number
				pWlBssInfo->ctl_ch = freq2channel(pBssidList->wlanBssEntries[i].ulChCenterFrequency/1000);

				//pWlBssInfo->atim_window = (uint16)pBssid->Configuration.ATIMWindow;

				pWlBssInfo->dtim_period = 0;
				pWlBssInfo->RSSI = (int16)pBssidList->wlanBssEntries[i].lRssi;
				pWlBssInfo->phy_noise = 0;

				/* copy only the variable length IEs */
				pWlBssInfo->ie_length = ielength;
				pWlBssInfo->ie_offset = sizeof(wl_bss_info_t); //(uint8) (OFFSETOF(wl_bss_info_t, ie_length) + 1);
				pIe = (PBYTE)(&pBssidList->wlanBssEntries[i]) + pBssidList->wlanBssEntries[i].ulIeOffset;
				if (ielength > 0)
					memcpy(
						((uint8 *)(((uint8 *)pWlBssInfo) + pWlBssInfo->ie_offset)),
						((uint8 *) pIe),  /*+ sizeof(NDIS_802_11_FIXED_IEs)*/
						ielength);

				Sleep(1);
				
				/* next record */
				pWlBssInfo = (wl_bss_info_t *)(((uint8 *)pWlBssInfo) + pWlBssInfo->length);
			}
		}
	}

	return pWlScanList;
}

/* ###### */
/* For VISTA */
/* ###### */
static char *
wps_dot11_scan_results(char const *ifname, char *buf, int buf_len)
{
	int ret = -1;

	wl_scan_results_t *pWlScanList;
	DWORD dwError = ERROR_SUCCESS;
	DWORD dwNegotiatedVersion;
	HANDLE hClient = NULL;
	GUID guidIntf;
	PWLAN_BSS_LIST pWlanBssList = NULL;


	// open a handle to the service
	dwError = WlanOpenHandle(2,NULL,&dwNegotiatedVersion,&hClient);
	if (dwError == ERROR_SUCCESS)
	{
		// get the interface GUID
		if (UuidFromString(ValidIfName(ifname), &guidIntf) == RPC_S_OK)
		{
			// Get scan results
			if (WlanGetNetworkBssList(hClient, &guidIntf, NULL, dot11_BSS_type_infrastructure, FALSE, NULL, &pWlanBssList) == ERROR_SUCCESS)
			{
				if(pWlanBssList->dwNumberOfItems > 0)
				{
					if(pWlScanList = scan_list_acm_to_wl(pWlanBssList, buf, buf_len))
					{
						ret = 0;
					}
				}
				WlanFreeMemory(pWlanBssList);
			}
			else
			{
				printf("ERROR: UuidFromString failed (%s)\n",ValidIfName(ifname));
			}
		}
		else
		{
			dwError = GetLastError();
			printf("ERROR: UuidFromString failed (%s)\n",ValidIfName(ifname));
		}

		WlanCloseHandle(hClient, NULL);
	}
	else
	{
		WpsDebugOutput("ERROR: WlanOpenHandle failed\n");
	}

	if (ret != 0)
		return NULL;
	
	return buf;
}

static int
wps_do_join_xp(char *ssid, uint16 capability)
{
	uint32 length = 0;
	int ret = -1;
	wl_instance_info_t instance;
	NDIS_802_11_SSID s;

	s.SsidLength = (ULONG) strlen(ssid);
	if (s.SsidLength > 0)
		memcpy(s.Ssid, ssid, s.SsidLength);

	do 
	{
		//NDIS_802_11_AUTHENTICATION_MODE amode = Ndis802_11AuthModeAutoSwitch;
		NDIS_802_11_AUTHENTICATION_MODE amode = Ndis802_11AuthModeOpen;
		NDIS_802_11_NETWORK_INFRASTRUCTURE imode = Ndis802_11Infrastructure;
		NDIS_802_11_ENCRYPTION_STATUS cstatus;
		NDIS_802_11_PRIVACY_FILTER privacy=Ndis802_11PrivFilterAcceptAll;

		length = sizeof(imode);
		if (ir_setinformation(wps_osl_wksp->stAdapterInfo.irh, OID_802_11_INFRASTRUCTURE_MODE,
			(PUCHAR)&imode, &length) != NO_ERROR)
			break;

		length = sizeof(amode);
		if (ir_setinformation(wps_osl_wksp->stAdapterInfo.irh, OID_802_11_AUTHENTICATION_MODE,
			(PUCHAR)&amode, &length) != NO_ERROR)
			break;
		

		/* Clear wpa_cfg for our drivers */
		length = sizeof(instance);
		if (ir_queryinformation(wps_osl_wksp->stAdapterInfo.irh, WLC_GET_INSTANCE,
			(PUCHAR) &instance, &length) == ERROR_SUCCESS)
		{
			DWORD length;
			setinformation_t *setinfo;

			length = SETINFORMATION_SIZE + (DWORD) strlen("wpa_cfg") + 1;
			setinfo = (setinformation_t *) malloc(length);
			if (setinfo)
			{
				setinfo->cookie = OIDENCAP_COOKIE;
				setinfo->oid = WLC_SET_VAR;
				strcpy(SETINFORMATION_DATA(setinfo), "wpa_cfg");
				ir_setinformation(wps_osl_wksp->stAdapterInfo.irh, OID_BCM_SETINFORMATION,
					(PUCHAR)setinfo, &length);
				free(setinfo);
			}
		}

		if (capability & DOT11_CAP_PRIVACY) {
			length = sizeof(cstatus);
			cstatus = Ndis802_11Encryption2Enabled;

			if (ir_setinformation(wps_osl_wksp->stAdapterInfo.irh, OID_802_11_ENCRYPTION_STATUS,
				(PUCHAR)&cstatus, &length) != NO_ERROR)
				break;
		} else {
			length = sizeof(cstatus);
			cstatus = Ndis802_11EncryptionDisabled;
			if (ir_setinformation(wps_osl_wksp->stAdapterInfo.irh, OID_802_11_ENCRYPTION_STATUS,
				(PUCHAR)&cstatus, &length) != NO_ERROR)
				break;
		}

		length = sizeof(s);
		if (ir_setinformation(wps_osl_wksp->stAdapterInfo.irh,OID_802_11_SSID,
			(PUCHAR)&s, &length) != NO_ERROR)
			break;

		ret = 0;
	} while(0);

	return ret;
}

// copy SSID to a null-terminated WCHAR string
// count is the number of WCHAR in the buffer.
static LPWSTR
SsidToStringW2(LPWSTR buf, ULONG count, PDOT11_SSID pSsid)
{
    ULONG   bytes, i;

    bytes = min( count-1, pSsid->uSSIDLength);
    for( i=0; i<bytes; i++)
        mbtowc( &buf[i], (const char *)&pSsid->ucSSID[i], 1);
    buf[bytes] = '\0';

    return buf;
}

// copy SSID to a null-terminated WCHAR string
// count is the number of WCHAR in the buffer.
static void
StringToStringW2(LPWSTR buf,const char *pStr)
{
    unsigned int i;

    for( i=0; i<strlen(pStr); i++) mbtowc( &buf[i], (const char *)&(pStr[i]), 1);
    buf[i] = '\0';
}

static BOOL
XmlEncode(LPSTR cszOld, LPSTR szEncoded, int nBufLen)
{
	BOOL bRet = TRUE;
	int i = 0, nLen = 0, nOldLen;
	char szEntity[8], *cur;

	if (cszOld == NULL || szEncoded == NULL)
		return FALSE;

	nOldLen = (int)strlen(cszOld);
	cur = szEncoded;
	for (i = 0; i < nOldLen && nLen < nBufLen; i++) {
		if (cszOld[i] == '&') {
			strcpy(szEntity, "&amp;");
		}
		else if (cszOld[i] == '\'') {
			strcpy(szEntity, "&apos;");
		}
		else if (cszOld[i] == '<') {
			strcpy(szEntity, "&lt;");
		}
		else if (cszOld[i] == '>') {
			strcpy(szEntity, "&gt;");
		}
		else {
			*szEntity = '\0';  // Make it empty string 
		}

		if (strlen(szEntity) > 0) {
			// Need to transfer character
			nLen += (int)strlen(szEntity);
			if (nBufLen >= nLen + 1) {  // Leave the space for '\0'
				strcpy(cur, szEntity);
				cur += strlen(szEntity);
			}
			else {
				bRet = FALSE;
				goto END;  // Not enough buffer
			}
		}
		else {
			nLen++;
			*cur = cszOld[i];
			cur++;
		}
	}

	*cur = '\0';
	bRet = TRUE;

END:
	return bRet;
}

static BOOL
BuildWlanProfileXml(const wps_credentials *pCredential, char *szProfileXml)
{
	BOOL bRet = FALSE;
	char szAuthType[16] = { 0 }, szEncryptType[16], szKeyType[16] = { 0 };
	char szEncodedSsid[512], szPassphraseEncoded[512];
	int i;

	// Define WLAN profile XML templates

	// OPEN/NO-WEP	
	const char *cszTemplateXMLOpenNoWep = "<?xml version=\"1.0\"?> \
										  <WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\"> \
										  <name>%s</name> \
										  <SSIDConfig> \
										  <SSID> \
										  <name>%s</name> \
										  </SSID> \
										  </SSIDConfig> \
										  <connectionType>ESS</connectionType> \
										  <connectionMode>auto</connectionMode> \
										  <autoSwitch>true</autoSwitch> \
										  <MSM> \
										  <security> \
										  <authEncryption> \
										  <authentication>open</authentication> \
										  <encryption>none</encryption> \
										  <useOneX>0</useOneX> \
										  </authEncryption> \
										  </security> \
										  </MSM> \
										  </WLANProfile>";

	// OPEN/WEP, Shared/WEP, OPEN/WPAPSK, OPEN/WPA2PSK
	const char *cszTemplateXMLSecure = "<?xml version=\"1.0\"?> \
										<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\"> \
										<name>%s</name> \
										<SSIDConfig> \
										<SSID> \
										<name>%s</name> \
										</SSID> \
										</SSIDConfig> \
										<connectionType>ESS</connectionType> \
										<connectionMode>auto</connectionMode> \
										<autoSwitch>true</autoSwitch> \
										<MSM> \
										<security> \
										<authEncryption> \
										<authentication>%s</authentication> \
										<encryption>%s</encryption> \
										<useOneX>0</useOneX> \
										</authEncryption> \
										<sharedKey> \
										<keyType>%s</keyType> \
										<protected>false</protected> \
										<keyMaterial>%s</keyMaterial> \
										</sharedKey> \
										</security> \
										</MSM> \
										</WLANProfile>";

	// NULL pinter validation
	if(!szProfileXml)
		goto END;


	// Encode ssid
	if(!XmlEncode((LPSTR)pCredential->ssid, szEncodedSsid, sizeof(szEncodedSsid)))
		goto END;

	// Encode passphrase/network key
	if(!XmlEncode((LPSTR)pCredential->nwKey, szPassphraseEncoded, sizeof(szPassphraseEncoded)))
		goto END;


	// Look up authentication type and encryption type
	for(i=0; i<SIZEOF_ARRAY(g_mapOneXGenAuthType) && !bRet; i++)
	{
		if(bRet = (stricmp(g_mapOneXGenAuthType[i].szKeyMgmt, pCredential->keyMgmt) == 0))
			strcpy(szAuthType, g_mapOneXGenAuthType[i].szWlanXmlElementStr);
	}

	if(bRet)
	{
		// Look up encryption type
		bRet = FALSE;
		for(i=0; i<SIZEOF_ARRAY(g_mapOneXGenEncryType) && !bRet; i++)
		{
			if(bRet = (g_mapOneXGenEncryType[i].uiWpsEngine == pCredential->encrType))
				strcpy(szEncryptType, g_mapOneXGenEncryType[i].szWlanXmlElementStr);
		}
	}

	if(bRet) 
	{
		// Both authentication and encryption type need to be available

		// Build profile xml by template
		if(stricmp(pCredential->keyMgmt, KEY_MGMT_OPEN) == 0 && pCredential->encrType == WPS_ENCRTYPE_NONE)
		{
			// Open/Non-WEP
			sprintf(szProfileXml, cszTemplateXMLOpenNoWep, pCredential->ssid, pCredential->ssid);
		}
		else
		{
			// All secure profile
			int nKeyLen = (int)strlen(pCredential->nwKey);

			if (stricmp(pCredential->keyMgmt, KEY_MGMT_WPAPSK) == 0 ||
				strcmp(pCredential->keyMgmt, KEY_MGMT_WPA2PSK) == 0 ||
				strcmp(pCredential->keyMgmt, KEY_MGMT_WPAPSK_WPA2PSK) == 0) {
				// Validate network key length for WPA-PSK auth methods
				if (nKeyLen == 64)
					strcpy(szKeyType, "networkKey");  // 64 all hex characters
				else if (nKeyLen >= 8 && nKeyLen <= 63)
					strcpy(szKeyType, "passPhrase");  // 8 to 63 ASCII characters
				else {
					OutputDebugStringA("Invalid network key!");
					bRet = FALSE;
					goto END;
				}
			}

			// No support to WEP key index yet
			sprintf(szProfileXml, 
					cszTemplateXMLSecure, 
					szEncodedSsid,			// <name>
					szEncodedSsid,			// <ssid>
					szAuthType,				// <authentication>
					szEncryptType,			// <encryption>
					szKeyType,				// <keyType> (refer to schema details)
					szPassphraseEncoded);	// <keyMaterial> 
			OutputDebugStringA(szProfileXml);
		}
	}

END:
	return bRet;
}

static DWORD wps_GetOSVer()
{
    OSVERSIONINFO osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));

	osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if(GetVersionEx(&osvi)) 
	{
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) 
		{
			wps_osl_wksp->os_ver = VER_WIN9X;
		}
		else 
		{
			if (osvi.dwMajorVersion == 6) {
				wps_osl_wksp->os_ver = VER_VISTA;
			} else if (osvi.dwMajorVersion == 5) {
				wps_osl_wksp->os_ver = VER_WIN2K;
			} else {
				wps_osl_wksp->os_ver = VER_WINNT;
			}
		}
    }
	else
	{
		printf("GetVersionEx() failed with error %u.\n", GetLastError());
	}

	return wps_osl_wksp->os_ver;
}


/* ################################# */
/* Vista Native WiFi API Runtime DLL loading & linking */
/* ################################# */
void
LoadWlanApi()
{
	if(wps_osl_wksp->os_ver == VER_VISTA && wps_osl_wksp->hWlanApi == NULL) {
		wps_osl_wksp->hWlanApi = LoadLibrary(WLANAPI_DLL_FILENAME);
	}
}

PVOID
GetWlanApiFunction(LPCSTR FunctionName)
{
	if(wps_osl_wksp->os_ver == VER_VISTA)
	{
		LoadWlanApi();
		if(wps_osl_wksp->hWlanApi)
		{
			return GetProcAddress(wps_osl_wksp->hWlanApi, FunctionName); 
		}
	}

	return NULL;
}

void
UnloadWlanApi()
{
	if(wps_osl_wksp->os_ver == VER_VISTA && wps_osl_wksp->hWlanApi != NULL)
	{
		FreeLibrary(wps_osl_wksp->hWlanApi);
		wps_osl_wksp->hWlanApi = NULL;
	}
}

// Public APIs
DWORD WINAPI
WlanOpenHandle(
    __in DWORD dwClientVersion,
    __reserved PVOID pReserved,
    __out PDWORD pdwNegotiatedVersion,
    __out PHANDLE phClientHandle
)
{
	DWORD retval=-1;
	typedef DWORD (WINAPI*ApiCall)(DWORD,PVOID,PDWORD,PHANDLE);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetWlanApiFunction(TEXT("WlanOpenHandle")); 
	if(ProcAdd)
	{
		retval=ProcAdd(dwClientVersion,pReserved,pdwNegotiatedVersion,phClientHandle);
	}

	return retval;
}

DWORD WINAPI
WlanCloseHandle(
    __in HANDLE hClientHandle,
    __reserved PVOID pReserved
)
{
	DWORD retval=-1;
	typedef DWORD (WINAPI*ApiCall)(HANDLE,PVOID);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetWlanApiFunction(TEXT("WlanCloseHandle")); 
	if(ProcAdd)
	{
		retval=ProcAdd(hClientHandle,pReserved);
	}

	return retval;
}

DWORD WINAPI
WlanQueryInterface(
    __in HANDLE hClientHandle,
    __in CONST GUID *pInterfaceGuid, 
    __in WLAN_INTF_OPCODE OpCode,
    __reserved PVOID pReserved,
    __out PDWORD pdwDataSize,
    __deref_out_bcount(*pdwDataSize) PVOID *ppData,
    __out_opt PWLAN_OPCODE_VALUE_TYPE pWlanOpcodeValueType
)
{
	DWORD retval=-1;
	typedef DWORD (WINAPI*ApiCall)(
    __in HANDLE hClientHandle,
    __in CONST GUID *pInterfaceGuid, 
    __in WLAN_INTF_OPCODE OpCode,
    __reserved PVOID pReserved,
    __out PDWORD pdwDataSize,
    __deref_out_bcount(*pdwDataSize) PVOID *ppData,
    __out_opt PWLAN_OPCODE_VALUE_TYPE pWlanOpcodeValueType
	);

	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetWlanApiFunction(TEXT("WlanQueryInterface")); 
	if(ProcAdd)
	{
		retval=ProcAdd(hClientHandle,
			pInterfaceGuid, 
			OpCode,
			pReserved,
			pdwDataSize,
			ppData,
			pWlanOpcodeValueType
			);

	}

	return retval;
}

DWORD WINAPI
WlanScan(
    __in HANDLE hClientHandle,
    __in CONST GUID *pInterfaceGuid, 
    __in_opt CONST PDOT11_SSID pDot11Ssid,
    __in_opt CONST PWLAN_RAW_DATA pIeData,
    __reserved PVOID pReserved
)
{
	DWORD retval=-1;
	typedef DWORD (WINAPI*ApiCall)(
    __in HANDLE hClientHandle,
    __in CONST GUID *pInterfaceGuid, 
    __in_opt CONST PDOT11_SSID pDot11Ssid,
    __in_opt CONST PWLAN_RAW_DATA pIeData,
    __reserved PVOID pReserved
		);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetWlanApiFunction(TEXT("WlanScan")); 
	if(ProcAdd)
	{
		retval=ProcAdd(hClientHandle,pInterfaceGuid,pDot11Ssid,pIeData,pReserved);
	}

	return retval;
}

DWORD WINAPI
WlanGetNetworkBssList(
    __in HANDLE hClientHandle,
    __in CONST GUID *pInterfaceGuid, 
    __in_opt CONST PDOT11_SSID pDot11Ssid,
    __in DOT11_BSS_TYPE dot11BssType,
    __in BOOL bSecurityEnabled,
    __reserved PVOID pReserved,
    __deref_out PWLAN_BSS_LIST *ppWlanBssList
)
{
	DWORD retval=-1;
	typedef DWORD (WINAPI*ApiCall)(
    __in HANDLE hClientHandle,
    __in CONST GUID *pInterfaceGuid, 
    __in_opt CONST PDOT11_SSID pDot11Ssid,
    __in DOT11_BSS_TYPE dot11BssType,
    __in BOOL bSecurityEnabled,
    __reserved PVOID pReserved,
    __deref_out PWLAN_BSS_LIST *ppWlanBssList
		);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetWlanApiFunction(TEXT("WlanGetNetworkBssList")); 
	if(ProcAdd)
	{
		retval=ProcAdd(hClientHandle,pInterfaceGuid,pDot11Ssid,dot11BssType,bSecurityEnabled,pReserved,ppWlanBssList);
	}

	return retval;
}

DWORD WINAPI 
WlanConnect(
    __in HANDLE hClientHandle,
    __in CONST GUID *pInterfaceGuid, 
    __in CONST PWLAN_CONNECTION_PARAMETERS pConnectionParameters,
    __reserved PVOID pReserved
)
{
	DWORD retval=-1;
	typedef DWORD (WINAPI*ApiCall)(
    __in HANDLE hClientHandle,
    __in CONST GUID *pInterfaceGuid, 
    __in CONST PWLAN_CONNECTION_PARAMETERS pConnectionParameters,
    __reserved PVOID pReserved
		);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetWlanApiFunction(TEXT("WlanConnect")); 
	if(ProcAdd)
	{
		retval=ProcAdd(hClientHandle,pInterfaceGuid,pConnectionParameters,pReserved);
	}

	return retval;
}

DWORD WINAPI 
WlanDisconnect(
    __in HANDLE hClientHandle,
    __in CONST GUID *pInterfaceGuid, 
    __reserved PVOID pReserved
)
{
	DWORD retval=-1;
	typedef DWORD (WINAPI*ApiCall)(
    __in HANDLE hClientHandle,
    __in CONST GUID *pInterfaceGuid, 
    __reserved PVOID pReserved
		);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetWlanApiFunction(TEXT("WlanDisconnect")); 
	if(ProcAdd)
	{
		retval=ProcAdd(hClientHandle,pInterfaceGuid,pReserved);
	}

	return retval;
}

DWORD WINAPI
WlanSetProfile(
    __in HANDLE hClientHandle,
    __in CONST GUID *pInterfaceGuid,
    __in DWORD dwFlags,
    __in LPCWSTR strProfileXml,
    __in_opt LPCWSTR strAllUserProfileSecurity,
    __in BOOL bOverwrite,
    __reserved PVOID pReserved,
    __out DWORD *pdwReasonCode
)
{
	DWORD retval=-1;
	typedef DWORD (WINAPI*ApiCall)(
    __in HANDLE hClientHandle,
    __in CONST GUID *pInterfaceGuid,
    __in DWORD dwFlags,
    __in LPCWSTR strProfileXml,
    __in_opt LPCWSTR strAllUserProfileSecurity,
    __in BOOL bOverwrite,
    __reserved PVOID pReserved,
    __out DWORD *pdwReasonCode
		);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetWlanApiFunction(TEXT("WlanSetProfile")); 
	if(ProcAdd)
	{
		retval=ProcAdd(hClientHandle,pInterfaceGuid,dwFlags,strProfileXml,strAllUserProfileSecurity,bOverwrite,pReserved,pdwReasonCode);
	}

	return retval;
}

DWORD WINAPI
WlanIhvControl(
  __in         HANDLE hClientHandle,
  __in         const GUID* pInterfaceGuid,
  __in         WLAN_IHV_CONTROL_TYPE Type,
  __in         DWORD dwInBufferSize,
  __in         PVOID pInBuffer,
  __in         DWORD dwOutBufferSize,
  __inout_opt  PVOID pOutBuffer,
  __out        PDWORD pdwBytesReturned
)
{
	DWORD retval=-1;
	typedef DWORD (WINAPI*ApiCall)(
	  __in         HANDLE hClientHandle,
	__in         const GUID* pInterfaceGuid,
	__in         WLAN_IHV_CONTROL_TYPE Type,
	__in         DWORD dwInBufferSize,
	__in         PVOID pInBuffer,
	__in         DWORD dwOutBufferSize,
	__inout_opt  PVOID pOutBuffer,
	__out        PDWORD pdwBytesReturned
		);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetWlanApiFunction(TEXT("WlanIhvControl")); 
	if(ProcAdd)
	{
		retval=ProcAdd(hClientHandle,pInterfaceGuid,Type,dwInBufferSize,pInBuffer,dwOutBufferSize,pOutBuffer,pdwBytesReturned);
	}

	return retval;
}

VOID WINAPI 
WlanFreeMemory(
    __in PVOID pMemory
)
{
	DWORD retval=-1;
	typedef DWORD (WINAPI*ApiCall)(PVOID);
	ApiCall ProcAdd;

	ProcAdd = (ApiCall) GetWlanApiFunction(TEXT("WlanFreeMemory")); 
	if(ProcAdd)
	{
		ProcAdd(pMemory);
	}
}

BOOL
wps_create_wlan_profile(LPCTSTR cszAdapterName, const wps_credentials *pCredential)
{
	BOOL bRet = FALSE;
	DWORD dwError, dwReasonCode;
	DWORD dwNegotiatedVersion;
	HANDLE hClient = NULL;
	GUID guidIntf;
	char szProfileXml[XML_PROFILE_MAX_LEN] = { '\0' };
	WCHAR wszProfileXml[XML_PROFILE_MAX_LEN] = { L'\0' }; 

	bRet = BuildWlanProfileXml(pCredential, szProfileXml);
	if(bRet)
	{
		// open a handle to the service
		if(WlanOpenHandle(1, NULL, &dwNegotiatedVersion, &hClient) == ERROR_SUCCESS)
		{
			if(UuidFromString(ValidIfName(cszAdapterName), &guidIntf) == RPC_S_OK)
			{
				if(MultiByteToWideChar(CP_ACP, 
									0, 
									szProfileXml, 
									(int)strlen(szProfileXml)+1, 
									wszProfileXml, 
									sizeof(wszProfileXml)/sizeof(wszProfileXml[0])))
				{
					dwError = WlanSetProfile(hClient, &guidIntf, 0, wszProfileXml, NULL, TRUE, NULL, &dwReasonCode);
					bRet = (dwError == ERROR_SUCCESS);
				}
			}
			WlanCloseHandle(hClient, NULL);
		}
	}

	return bRet;
}

char *
wps_osl_get_scan_results(char *buf, int buf_len)
{
	char *results;

	if(wps_osl_wksp->os_ver == VER_VISTA)
	{
		// !!! We used a BAD approach to create wps ap list for Vista. Inside wps_dot11_scan_results, we converted ACM bss structure
		// to XP bss structure which is redundant and bad.  We should build wps ap/bss list directly from ACM bss data when there
		// is a chance to do so
		if(wps_dot11_scan(wps_osl_wksp->stAdapterInfo.shortadaptername)==0)
		{
			// This is really a hack assuming scan can finish within 3 seconds. We should use  WlanRegisterNotification
			// to get notified the scanning status and retrieve scan list after scan is completed
			Sleep(5000);  
			results = wps_dot11_scan_results(wps_osl_wksp->stAdapterInfo.shortadaptername,
				buf, buf_len);
		}
	}
	else
	{
		results = get_scan_results_xp(buf, buf_len);
	}

	return results;
}

int
SetOid_Xp(HANDLE irh, ULONG oid, void* data, ULONG nbytes)
{
	// 0 - Success, 1 - Failure
	if(ir_setinformation((HANDLE)(void *)irh, oid, data, &nbytes) == NO_ERROR )
		return 0;

	return 1;
}

int
SetOid_Vista(LPCTSTR cszAdapterName, ULONG oid, PUCHAR pInData, DWORD inlen)
{
	HANDLE hClient = NULL;
    GUID guidIntf;
	DWORD dwNegotiatedVersion;
	DWORD outDataSize = 0;
	DWORD dwInBuffSize;
	PWL_SET_OID_HEADER pInBuffer = NULL;
	DWORD inDataSize = inlen;
	DWORD dwBytesReturned = 0;
	DWORD dwStatus = ERROR_SUCCESS;

	dwStatus = WlanOpenHandle(2,NULL, &dwNegotiatedVersion,&hClient); 
	if(dwStatus != ERROR_SUCCESS)
		return 1;

	// get the interface GUID
	if (UuidFromString(ValidIfName(cszAdapterName), &guidIntf) != RPC_S_OK)
		return 1;

	if (pInData == (LPBYTE) NULL)
		inDataSize = 0;	

	dwInBuffSize = sizeof(WL_SET_OID_HEADER)		// request header
						+ inDataSize	// oid-specific inData (to the driver)
						+ outDataSize;	// outData expected from the driver

	// allow one buffer to store a request and reserve space for storing the result
	pInBuffer = (PWL_SET_OID_HEADER)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwInBuffSize);		
	if (pInBuffer == NULL)
			return ERROR_NOT_ENOUGH_MEMORY;


	// setup the request header
	pInBuffer->oid = OID_BCM_SETINFORMATION;
	// setup the SetInfo header
	pInBuffer->setinfo_hdr.oid = oid;
	pInBuffer->setinfo_hdr.cookie = OIDENCAP_COOKIE;
	// followed by the oid-specific input data 
	if (inDataSize > 0)
	{
		memcpy((PCHAR)(pInBuffer+1), pInData, inDataSize);
	}

	// Does driver set a driver-specific error code in the output buffer ?
	dwStatus = WlanIhvControl(hClient, 
							  &guidIntf, 
							  wlan_ihv_control_type_driver, 
							  dwInBuffSize, 
							  (PVOID)pInBuffer, 
							  (DWORD)dwInBuffSize, 
							  pInBuffer, 
							  &dwBytesReturned);
	
	// cleanup: Free the allocated buffers.	
	if (pInBuffer)
		HeapFree(GetProcessHeap(), 0, (LPVOID)pInBuffer);

	if (hClient != NULL) 
		WlanCloseHandle(hClient, NULL);

	return dwStatus;
}


int
QueryOid_Xp(HANDLE irh, ULONG oid, void* results, ULONG nbytes)
{
	if(irh == NULL)
		return 1;  // Failure

	return ir_queryinformation(irh, oid, results, &nbytes);
}

int
QueryOid_Vista(LPCTSTR cszAdapterName, ULONG oid, void* results, ULONG nbytes)
{
	int nRet = 1;  // 0 - Success, 1 - Fail
	DWORD dwNegotiatedVersion;
	HANDLE hClient = NULL;
    GUID guidIntf;
	DWORD dwStatus = ERROR_SUCCESS;
	DWORD dwBytesReturned = 0;
	PWL_QUERY_OID_HEADER pInBuffer = NULL;
	DWORD dwInBuffSize = sizeof(WL_QUERY_OID_HEADER) + nbytes;			// outData expected from the driver

	if(cszAdapterName == NULL)
		return 1;  // Failure

	// open a handle to the service
	dwStatus = WlanOpenHandle(2,NULL,&dwNegotiatedVersion,&hClient);
	if(dwStatus == ERROR_SUCCESS)
	{
		// get the interface GUID
		if (UuidFromString(ValidIfName(cszAdapterName), &guidIntf) == RPC_S_OK)
		{
			// allow one buffer to store a request and reserve space for storing the result
			pInBuffer = (PWL_QUERY_OID_HEADER)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwInBuffSize);		
			if (pInBuffer == NULL)
				return 1;

			// initialize the request header
			pInBuffer->oid = OID_BCM_GETINFORMATION;

			// initialize the "getinfo" header
			pInBuffer->getinfo_hdr.oid = oid;
			pInBuffer->getinfo_hdr.len = dwInBuffSize;	// hdr+dataSize
			pInBuffer->getinfo_hdr.cookie = OIDENCAP_COOKIE;

			// followed by the oid-specific input data 
			if (nbytes > 0)
			{	
				// the top 'inDataSize' bytes in pResults are oid-specific data
				memcpy((PCHAR)(pInBuffer+1), results, nbytes);
			}

			// use 1 buffer for input and output
			dwStatus = WlanIhvControl(hClient, 
									  &guidIntf, 
									  wlan_ihv_control_type_driver, 
									  dwInBuffSize, 
									  (PVOID) pInBuffer, 
									  (DWORD) dwInBuffSize, 
									  pInBuffer, 
									  &dwBytesReturned);

			// transfer data
			if (dwStatus == ERROR_SUCCESS)
			{
				// note: 1. inBuffer = outBuffer; so let's remove the request-header before returning to the caller.
				//       2. driver should return a "wl" (e.g. wl_channels_in_country) header 
				PCHAR pTmpOutData = (PCHAR) (pInBuffer + 1);		// skip the "request" header
				DWORD outDataSize = dwBytesReturned - sizeof(WL_QUERY_OID_HEADER);
				if (outDataSize > nbytes)
					outDataSize = nbytes;			// only return what caller asks for

				if (outDataSize > 0)
				{
					memcpy(results, (PCHAR)(pInBuffer+1), outDataSize);
					dwBytesReturned = outDataSize;	// remove the inRequest header
				}
				nRet = 0;  // Success
			}

			// Free the allocated buffers.	
			if (pInBuffer)
				HeapFree(GetProcessHeap(), 0, (LPVOID)pInBuffer);
		}
	}
	
	if (hClient != NULL) 
		WlanCloseHandle(hClient, NULL);

	return nRet;
}

int
QueryOid(HANDLE irh, LPCTSTR cszAdapterName, ULONG oid, void* results, ULONG nbytes)
{
	if(wps_osl_wksp->os_ver == VER_VISTA)
		return QueryOid_Vista(cszAdapterName, oid, results, nbytes);
	else
		return QueryOid_Xp(irh, oid, results, nbytes);
}

int
SetOid(ULONG oid, void* data, ULONG nbytes)
{
	if(wps_osl_wksp->os_ver == VER_VISTA)
		return SetOid_Vista(wps_osl_wksp->stAdapterInfo.shortadaptername, oid, data, nbytes);
	else
		return SetOid_Xp(wps_osl_wksp->stAdapterInfo.irh, oid, data, nbytes);
}


/* ############### */
/* Exported WPS_OSL APIs */
/* ############### */
#ifdef _TUDEBUGTRACE
void
wps_osl_print_buf(unsigned char *buff, int buflen)
{
	int i;

	printf("\n print buf %d: \n", buflen);
	for (i = 0; i < buflen; i++) {
		printf("%02X ", buff[i]);
		if (!((i+1)%16))
			printf("\n");
	}
	printf("\n");
}
#endif /* _TUDEBUGTRACE */

int
wps_osl_join_network(char *s_ssid, uint32 wsec)
{
    int auth = 0, infra = 1, ret = -1;

	WpsDebugOutput("joining network\n");

	if(wps_osl_wksp->os_ver == VER_VISTA)
	{
		DWORD dwError = ERROR_SUCCESS;
		DWORD dwNegotiatedVersion;
		HANDLE hClient = NULL;
		GUID guidIntf;
		WLAN_CONNECTION_PARAMETERS connPara;
		DOT11_BSSID_LIST bssidList;
		DOT11_SSID dot11_ssid;
		PWLAN_INTERFACE_STATE pIntrefaceState;
		DWORD dwTmp;
		char szEncodedSsid[512];
		WCHAR wszEncodedSsid[512], strProfile[5096];
		int i = 0;
		
		LPWSTR strProfilePrototype_part1 =
		L"<?xml version=\"1.0\"?><WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\"><name>Linksys_golan</name><SSIDConfig><SSID><name>"; /*/<hex>4861696D5345534E6574</hex> */
		LPWSTR strProfilePrototype_part2 =
		L"</name></SSID><nonBroadcast>true</nonBroadcast></SSIDConfig><connectionType>ESS</connectionType><connectionMode>manual</connectionMode><MSM><security><authEncryption><authentication>open</authentication><encryption>WEP</encryption><useOneX>false</useOneX></authEncryption><sharedKey><keyType>networkKey</keyType><protected>false</protected><keyMaterial>melco</keyMaterial></sharedKey></security></MSM></WLANProfile>";
		BOOL bTMP=FALSE;
		LPWSTR strProfile_fake =
		L"<?xml version=\"1.0\"?><WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\"><name>Linksys_golan</name><SSIDConfig><SSID><name>Le chien mange</name></SSID><nonBroadcast>true</nonBroadcast></SSIDConfig><connectionType>ESS</connectionType><connectionMode>manual</connectionMode><MSM><security><authEncryption><authentication>open</authentication><encryption>WEP</encryption><useOneX>false</useOneX></authEncryption><sharedKey><keyType>networkKey</keyType><protected>false</protected><keyMaterial>12345</keyMaterial></sharedKey></security></MSM></WLANProfile>";

		ZeroMemory(&connPara, sizeof(WLAN_CONNECTION_PARAMETERS));
		ZeroMemory(&bssidList, sizeof(DOT11_BSSID_LIST));
		ZeroMemory(&dot11_ssid, sizeof(DOT11_SSID));

		// Encode ssid with processing of special characters
		XmlEncode(s_ssid, szEncodedSsid, sizeof(szEncodedSsid));

		// Fill DOT11 SSID structure
		dot11_ssid.uSSIDLength = (ULONG)strlen(s_ssid);
		memcpy(&dot11_ssid.ucSSID, s_ssid, dot11_ssid.uSSIDLength);

		// open a handle to the service
		if ((dwError = WlanOpenHandle(2,NULL,&dwNegotiatedVersion,&hClient)) != ERROR_SUCCESS)
		{
			WpsDebugOutput("ERROR: WlanOpenHandle\n");
			return -1;
		}

		// get the interface GUID
		if (UuidFromString(ValidIfName(wps_osl_wksp->stAdapterInfo.shortadaptername),
			&guidIntf) != RPC_S_OK)
		{
			WpsDebugOutput("ERROR: WlanOpenHandle\n");
			return -1;
		}
			
		do
		{
			// Convert ssid to wide character string
			MultiByteToWideChar(CP_ACP, 0, szEncodedSsid, (int)strlen(szEncodedSsid) + 1, wszEncodedSsid, sizeof(szEncodedSsid)/sizeof(szEncodedSsid[0]));

			wcscpy(strProfile, strProfilePrototype_part1);
			wcscat(strProfile, wszEncodedSsid);
			wcscat(strProfile, strProfilePrototype_part2);

			if(wsec & DOT11_CAP_PRIVACY)
			{
				connPara.dot11BssType		= dot11_BSS_type_infrastructure;
				connPara.dwFlags			= WLAN_CONNECTION_HIDDEN_NETWORK|WLAN_CONNECTION_EAPOL_PASSTHROUGH_BIT;
				connPara.pDot11Ssid			= NULL;
				connPara.strProfile			= strProfile;
				connPara.wlanConnectionMode	= wlan_connection_mode_temporary_profile;
				connPara.pDesiredBssidList	= NULL;
			}
			else
			{
				//
				// Do a fake connect to clear any previous connection settings from the driver
				//
				connPara.dot11BssType		= dot11_BSS_type_infrastructure;
				connPara.dwFlags			= WLAN_CONNECTION_HIDDEN_NETWORK|WLAN_CONNECTION_EAPOL_PASSTHROUGH_BIT;
				connPara.pDot11Ssid			= NULL;
				connPara.strProfile			= strProfile_fake;
				connPara.wlanConnectionMode	= wlan_connection_mode_temporary_profile;
				connPara.pDesiredBssidList	= NULL;

				WlanConnect(hClient, &guidIntf, &connPara ,NULL);
				Sleep(2000);
				WlanDisconnect(hClient, &guidIntf, NULL);
				Sleep(2000);

				//
				// Now setup the right connection parameters
				//
				connPara.dot11BssType		= dot11_BSS_type_infrastructure;
				connPara.dwFlags			= 0;
				connPara.pDot11Ssid			= &dot11_ssid;
				connPara.strProfile			= NULL;
				connPara.wlanConnectionMode	= wlan_connection_mode_discovery_unsecure;
				connPara.pDesiredBssidList	= NULL;
			}

			// Connect
			if(WlanConnect(hClient, &guidIntf, &connPara, NULL) != ERROR_SUCCESS)
			{
				break;
			}

			// Now wait until we are associated or up to 20s
			while(i++ < 100)
			{
				if(WlanQueryInterface(hClient,
									&guidIntf,
									wlan_intf_opcode_interface_state,
									NULL,
									&dwTmp,
									&pIntrefaceState,
									NULL) != ERROR_SUCCESS)
				{
					break;
				}
				
				if(*pIntrefaceState == wlan_interface_state_connected)
				{
					// Associated -> Success!
					wps_osl_wksp->associated = true;
					ret = 0;
					break;
				}
				Sleep(200);
			};

		} while(0);

		// Close the handle
		if(hClient != NULL)
			WlanCloseHandle(hClient, NULL);
	}
	else
	{
		ret = wps_do_join_xp(s_ssid, (uint16)wsec);
	}

	WpsSleep(2);
    return ret;
}

int
wps_osl_join_network_with_bssid(char *ssid, uint32 wsec, char *bssid)
{
	return (wps_osl_join_network(ssid, wsec));
}

int
wps_osl_leave_network()
{
	if(wps_osl_wksp->os_ver == VER_VISTA)
	{
		DWORD dwNegotiatedVersion;
		HANDLE hClient = NULL;
	    GUID guidIntf;

		// open a handle to the service
		if (WlanOpenHandle(2,NULL,&dwNegotiatedVersion,&hClient) != ERROR_SUCCESS)
		{
			WpsDebugOutput("ERROR: WlanOpenHandle\n");
			return -1;
		}

		// get the interface GUID
		if (UuidFromString(ValidIfName(wps_osl_wksp->stAdapterInfo.shortadaptername),
			&guidIntf) != RPC_S_OK)
			WpsDebugOutput("ERROR: UuidFromString\n");

		WlanDisconnect(hClient,&guidIntf,NULL);

		if (hClient != NULL) WlanCloseHandle(hClient, NULL);

	}
	else wps_osl_wl_ioctl(WLC_DISASSOC, NULL, 0, TRUE);

	return 0;
}

#ifdef ASYNC_MODE
void *
wps_osl_thread_create(fnAsyncThread start_routine, void *arg)
{
	HANDLE WINAPI handle;

	if (start_routine == NULL)
		printf("Thread create failed\n");

	handle = CreateThread(NULL, 0, start_routine, arg, 0, NULL);
	if (handle != NULL) {
#ifdef _TUDEBUGTRACE
		printf("Thread created\n");
#endif
		return (void *)handle;
	}

	printf("Thread create failed\n");
	return NULL;
}

int
wps_osl_thread_join(void *thread, void **value_ptr)
{
	int retVal = 0;

	/* not implement yet, please reference Linux platform */

	return retVal;
}
#endif /* ASYNC_MODE */

bool
wps_osl_create_profile(const wps_credentials *credentials)
{
	//CallClientCallback(WPS_STATUS_CREATING_PROFILE,NULL);

	if(credentials == NULL)
		return false;

	if(!DoApplyNetwork(credentials)) {
		//CallClientCallback(WPS_STATUS_ERROR,NULL);
		return false;
	}
	else {
		//CallClientCallback(WPS_STATUS_SUCCESS,NULL);
	}

	return true;
}

char *
wps_osl_get_adapter_name()
{
	return wps_osl_wksp->stAdapterInfo.adaptername;
}

char *
wps_osl_get_short_adapter_name()
{
	return wps_osl_wksp->stAdapterInfo.shortadaptername;
}


int
wps_osl_get_mac(uint8 *mac)
{
	if (mac == NULL)
		return WPS_OSL_ERROR;

	if (wps_osl_wksp)
		memcpy(mac, wps_osl_wksp->stAdapterInfo.macaddress, 6);

	return WPS_OSL_SUCCESS;
}

char *
wps_osl_get_ssid()
{
	return wps_osl_wksp->stAdapterInfo.ssid;
}

uint32
wps_osl_htonl(uint32 intlong)
{
	return htonl(intlong);
}

uint16
wps_osl_htons(uint16 intshort)
{
	return htons(intshort);
}

/* in MS */
void
wps_osl_sleep(uint32 ms)
{
	Sleep(ms);
}

unsigned long
wps_osl_get_current_time()
{
	SYSTEMTIME systemTime;
	FILETIME fileTime;
	__int64 UnixTime;

	GetSystemTime( &systemTime );
	SystemTimeToFileTime( &systemTime, &fileTime );


	/* get the full win32 value, in 100ns */
	UnixTime = ((__int64)fileTime.dwHighDateTime << 32) + 
		fileTime.dwLowDateTime;

	/* convert to the Unix epoch */
	UnixTime -= (SECS_BETWEEN_EPOCHS * SECS_TO_100NS);

	UnixTime /= SECS_TO_100NS; /* now convert to seconds */

	return (long)(UnixTime);
}

/* Link to wl driver. */
int
wps_osl_wl_ioctl(int cmd, void *buf, int len, bool set)
{
	return _wps_osl_wl_ioctl(wps_osl_wksp->stAdapterInfo.irh,
		wps_osl_wksp->stAdapterInfo.shortadaptername,
		cmd, buf, len, set);
}

static void
change_led(unsigned int nLedGpio, unsigned int nLedhb, unsigned int nBlinkFreq)
{
	wl_led_info_t led_info;
	
	if(wps_osl_wksp == NULL || !wps_osl_wksp->bWpsLedEnabled)
		return;  // WPS LED is disabled, no need to change

	// wl.exe can be used to verify the iovar call working properly as in the following commands:
	// "wl mpc 0": Turn off the auto power saving mode for the adapter
	// "wl up": Set the driver to be up 
	// "wl ledbh 2 0": Turn off LED 2
	// "wl led_blinkcustom 0x00c800c8": Set custom blinking frequency to LED 2 as "On" 0x00c8 cycles and "Off" 0x00c8 cycles alternatively
	
	// "wl ledbh 2 18": Apply custom blink frequency to LED 2

	// "wl ledbh 2 19" blink periodically. To steps to make this work: 
	//	  (1) wl led_blinkcustom 0x00200020 (set the fast blink freq.). The blinking period and off period alternation is determined by driver by default
	//	  (2) wl ledbh 2 19 (apply 0x00200020 to led 2 under ledbh 19 (WL_LED_BLINKPERIODIC)

	// "wl leddc 0x0001000a": Optional. Change the LED lightness  (0001 "On" cycles/000a "Off" cycles, alternate very fast). When "Off" cycles increase, the LED becomes lighter and vice versa
	// Similar commands can be apply to LED 1(Green) and LED 2(Blue)/3(Amber) as well. LED2 (BLue) and LED 3(Amber) share same physical LED in different colors
	if(nLedhb == WL_LED_OFF || nLedhb == WL_LED_ASSOC_WITH_SEC)
	{
		// Turn off the LED or apply WL_LED_ASSOC_WITH_SEC behavior
		led_info.index = nLedGpio;
		led_info.behavior = nLedhb;
		_wps_osl_iovar_set("ledbh", &led_info, sizeof(led_info));
	}
	else
	{
		led_info.index = nLedGpio;
		led_info.behavior = WL_LED_OFF;
		_wps_osl_iovar_set("ledbh", &led_info, sizeof(led_info));

		// Set blinking frequency
		_wps_osl_iovar_set("led_blinkcustom", &nBlinkFreq, sizeof(nBlinkFreq));

		// Make frequency take effect
		led_info.index = nLedGpio;
		led_info.behavior = nLedhb;  // WL_LED_BLINKCUSTOM or WL_LED_BLINKPERIODIC;
		_wps_osl_iovar_set("ledbh", &led_info, sizeof(led_info));
	}
}

void
wps_osl_update_led(unsigned int uiStatus, bool b_secure_nw)
{
	switch(uiStatus)
	{	
	case WPS_STATUS_INIT:  // WPS negotiation begins
		change_led(LED_3_WPS_END, WL_LED_OFF, LED_BLINK_FREQ_OFF);
		change_led(LED_2_WPS_START, WL_LED_BLINKCUSTOM, LED_BLINK_FREQ_NEGOTIATING);
		break;
	case WPS_STATUS_SUCCESS:  
		// WPS negotiation ends with success. For secure network, turn the LED to WL_LED_ASSOC_WITH_SEC for secure network, 
		// so the driver will monitor connectivity (Secure/Unsecure) state to set LED blinking pattern. For unsecure network, 
		// turn off LED 2
		if(b_secure_nw)
			change_led(LED_2_WPS_START, WL_LED_ASSOC_WITH_SEC, LED_BLINK_FREQ_OFF);
		else
			change_led(LED_2_WPS_START, WL_LED_OFF, LED_BLINK_FREQ_OFF);
		break;
	case WPS_STATUS_SCANNING_OVER_SESSION_OVERLAP:  // Error of session overlap
		change_led(LED_2_WPS_START, WL_LED_OFF, LED_BLINK_FREQ_OFF);
		change_led(LED_3_WPS_END, WL_LED_BLINKPERIODIC, LED_BLINK_FREQ_ERROR_OVERLAP);
		break;
	case WPS_STATUS_ERROR:
	case WPS_STATUS_WRONG_PIN:
	/* case WPS_STATUS_WARNING_TIMEOUT: */
	case WPS_STATUS_WARNING_WPS_PROTOCOL_FAILED:
	case WPS_STATUS_WARNING_NOT_INITIALIZED:
	case WPS_STATUS_SCANNING_OVER_NO_AP_FOUND:
		change_led(LED_2_WPS_START, WL_LED_OFF, LED_BLINK_FREQ_OFF);
		change_led(LED_3_WPS_END, WL_LED_BLINKCUSTOM, LED_BLINK_FREQ_ERROR_OTHERS);
		break;
	default:
		break;
	}
}

/* HW Button */
bool
wps_osl_hwbutton_supported(const char *guid)
{
	bool bRet = false;
	DWORD dwRel = 0;
	char cBuf[WLC_IOCTL_MAXLEN] = { 0 };
	DWORD dwBufLen = WLC_IOCTL_MAXLEN;  // WLC_IOCTL_MAXLEN size is required for this IOCTL call
	adapter_info stLocalAdapterInfo;

	// If adapter info is not initialized, do that first 
	memset(&stLocalAdapterInfo, 0, sizeof(stLocalAdapterInfo));
	if(_wps_osl_init_adapter(guid, &stLocalAdapterInfo))
	{
		// Get wps GPIO button pin number. If failed, the button is not supported
		// The IOCTL call is equivalent to "wl.exe nvget wpsgpio". It requires the passed-in
		// buffer size to be WLC_IOCTL_MAXLEN.
		strcpy(cBuf, "wpsgpio");
		dwRel = _wps_osl_ioctl_get(stLocalAdapterInfo.irh, stLocalAdapterInfo.shortadaptername,
			WLC_GET_VAR, cBuf, dwBufLen);
		if(dwRel == NO_ERROR)
			bRet = true;
		ir_exit(stLocalAdapterInfo.irh);
	}

	return bRet;
}

bool
wps_osl_hwbutton_open(const char *guid)
{
	bool bRet = false;
	DWORD dwRel = 0;
	char cBuf[WLC_IOCTL_MAXLEN] = { 0 };
	DWORD dwBufLen = WLC_IOCTL_MAXLEN;  // WLC_IOCTL_MAXLEN size is required for this IOCTL call

	if (wps_osl_wksp == NULL)
		wps_osl_wksp = _wps_osl_alloc_wksp();

	if (wps_osl_wksp == NULL)
		return false;

	// If adapter info is not initialized, do that first 
	memset(&wps_osl_wksp->stGpioAdapterInfo, 0, sizeof(wps_osl_wksp->stGpioAdapterInfo));
	if(_wps_osl_init_adapter(guid, &wps_osl_wksp->stGpioAdapterInfo))
	{
		// Get wps GPIO button pin number. If failed, the button is not supported
		// The IOCTL call is equivalent to "wl.exe wpsgpio". It requires the passed-in
		// buffer size to be WLC_IOCTL_MAXLEN.
		strcpy(cBuf, "wpsgpio");
		dwRel = _wps_osl_ioctl_get(wps_osl_wksp->stGpioAdapterInfo.irh,
			wps_osl_wksp->stGpioAdapterInfo.shortadaptername,
			WLC_GET_VAR, cBuf, dwBufLen);
		if(dwRel == NO_ERROR)
			bRet = true;
	}

	return bRet;
}

void
wps_osl_hwbutton_close()
{
	if (wps_osl_wksp == NULL)
		return;

	// Close adapter handle if it is opened
	if (wps_osl_wksp->stGpioAdapterInfo.irh &&
	    wps_osl_wksp->stGpioAdapterInfo.irh != INVALID_HANDLE_VALUE)
	{
		ir_exit(wps_osl_wksp->stGpioAdapterInfo.irh);
		wps_osl_wksp->stGpioAdapterInfo.irh = NULL;
	}
}

bool
wps_osl_hwbutton_state()
{
	bool bRet = false;
	DWORD dwPinNo = 0;
	DWORD mask = 0x1;
	DWORD dwRel = 0;
	DWORD dwPinState = 0;
	char cBuf[WLC_IOCTL_MAXLEN] = { 0 };
	DWORD dwBufLen = WLC_IOCTL_MAXLEN;
	DWORD i = 0;

	if (wps_osl_wksp == NULL)
		return false;

	// Get wps GPIO PIN number. If failed, the GPIO button is not supported. Verify use command "wl.exe wpsgpio"
	// We should consider putting this call (get PIN number) in wps_gpio_open only. We will make this change together 
	// when adding support to WPS LED control
	strcpy(cBuf, "wpsgpio");
	dwRel = _wps_osl_ioctl_get(wps_osl_wksp->stGpioAdapterInfo.irh,
		wps_osl_wksp->stGpioAdapterInfo.shortadaptername,
		WLC_GET_VAR, cBuf, dwBufLen);
	memcpy(&dwPinNo, cBuf, sizeof(dwPinNo));
	if(dwRel == NO_ERROR)
	{
		// get GPIO PIN status indicating whether WPS button is pressed or not
		strcpy(cBuf, "ccgpioin");  // Verify use command "wl.exe ccgpioin"
		dwRel = _wps_osl_ioctl_get(wps_osl_wksp->stGpioAdapterInfo.irh,
			wps_osl_wksp->stGpioAdapterInfo.shortadaptername,
			WLC_GET_VAR, cBuf, dwBufLen);
		if(dwRel == NO_ERROR)
		{
			memcpy(&dwPinState, cBuf, sizeof(dwPinState));
			
			// mask out button state. "0" means that button is being pressed. "1" means
			// button is relesed
			for(i = 0; i < dwPinNo; i++)
				mask = mask << 1;
			if((dwPinState & mask) == 0)
				bRet = true;
			else
				bRet = false;
		}
	}

	return bRet;
}

uint32
wps_osl_init(void *cb_ctx, const void *cb, const char *adapter_id)
{
	char *ifname = (char*) adapter_id;

	if (wps_osl_wksp == NULL)
		wps_osl_wksp = _wps_osl_alloc_wksp();

	if (wps_osl_wksp == NULL)
		return WPS_OSL_ERROR;

	wps_osl_wksp->cb = (fnWpsProcessCB)cb;
	wps_osl_wksp->cb_ctx = cb_ctx;

	/* Get OS version */
	wps_GetOSVer();

	/* 1. RAND */
	RAND_windows_init();

	/* 2. Adapter, adapter_id is a guid string */
	/* We have the adapter handler stAdapterInfo for further use */
	if(!_wps_osl_init_adapter(adapter_id, &wps_osl_wksp->stAdapterInfo)) {
		if (wps_osl_wksp->stAdapterInfo.irh &&
		    wps_osl_wksp->stAdapterInfo.irh != INVALID_HANDLE_VALUE) {
			ir_exit(wps_osl_wksp->stAdapterInfo.irh);
			wps_osl_wksp->stAdapterInfo.irh = NULL;
		}
		return WPS_OSL_ERROR;
	}

	/* 3. WPS Led */
	wps_osl_wksp->bWpsLedEnabled = _wps_osl_is_wps_led_enabled();
	if(wps_osl_wksp->bWpsLedEnabled) {
		// When LED is enabled, we need to turn off "mpc" mode in order to make LED blink constantly as per spec.
		// "wl.exe mpc 0": Turn off mpc mode
		// "wl.exe up": Bring up interface
		uint32 mpc = 0;
		_wps_osl_iovar_set("mpc", &mpc, sizeof(mpc)); // Turn off "mpc" mode
		_wps_osl_ioctl_set(WLC_UP, NULL, 0); // Bring up interface
	}

	/* 4. Disable Management */
	/* Update status */
	wps_api_status_cb(&wps_osl_wksp->cb, wps_osl_wksp->cb_ctx, WPS_STATUS_DISABLING_WIFI_MANAGEMENT, NULL);
	
	InitializeNetworkEnvironment();

	return WPS_OSL_SUCCESS;	
}

void
wps_osl_deinit()
{
	/* Enable wpa_supplicant */
	/* Update status */
	wps_api_status_cb(&wps_osl_wksp->cb, wps_osl_wksp->cb_ctx, WPS_STATUS_ENABLING_WIFI_MANAGEMENT, NULL);
	
	RestoreNetworkEnvironment();

	if (wps_osl_wksp->associated)
		wps_osl_leave_network();

	wps_osl_cleanup_802_1x();

	// Close adapter handle if it is opened
	if (wps_osl_wksp->stAdapterInfo.irh &&
	    wps_osl_wksp->stAdapterInfo.irh != INVALID_HANDLE_VALUE) {
		ir_exit(wps_osl_wksp->stAdapterInfo.irh);
		wps_osl_wksp->stAdapterInfo.irh = NULL;
	}

	if (wps_osl_wksp) {
		free(wps_osl_wksp);
		wps_osl_wksp = NULL;
	}
}

void
wps_osl_abort()
{
	/* not implement yet, please reference Linux platform */
}
