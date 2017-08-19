/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: $
 */
#ifndef __WPS_API_H__
#define __WPS_API_H__

#ifdef __cplusplus 
extern "C" {
#endif

#include <oidencap.h>
#include <typedefs.h>
#include <epictrl.h>

#include "reg_prototlv.h"
#include "preflib.h"
#include "wps_sdk.h"

#define XML_PROFILE_MAX_LEN		4096

// define keyMgmt string values inherited from enrollee engine
#define KEY_MGMT_OPEN			"OPEN"
#define KEY_MGMT_SHARED			"SHARED"
#define KEY_MGMT_WPAPSK			"WPA-PSK"
#define KEY_MGMT_WPA2PSK		"WPA2-PSK"
#define KEY_MGMT_WPAPSK_WPA2PSK	"WPA-PSK WPA2-PSK"

// Define generic authentication map
typedef struct _ST_ONEXGENAUTHTYPE {
	unsigned int uiPrototlv;		// Format as defined in reg_prototlv.h
	unsigned int uiPreflib;			// Format as defined in preflib
	char *szWlanXmlElementStr;		// ACM/WLAN profile element string
	char *szProvSvcXmlElementSTR;	// Wireless Provisioning Service profile element string
	char *szKeyMgmt;				// keyMgmt string
} ST_ONEXGENAUTHTYPE;

// Define generic encryption map
typedef struct _ST_ONEXGENENCRTYPE {
	unsigned int uiWpsEngine;		// Engine version as defined in in reg_prototlv.h
	unsigned int uiPreflib;			// Format as defined in preflib
	char *szWlanXmlElementStr;		// ACM/WLAN profile element string
	char *szProvSvcXmlElementSTR;	// Wireless Provisioning Service profile element string
} ST_ONEXGENENCRTYPE;

// Define map across different layer's definition for 802.1x authentication type
// WPS only support "Personal" security mode which is PSK mode for WPA and WPA2
// There is difference (just first letter case) between the schema of WLAN API and that of Wireless Provisioning Service
const ST_ONEXGENAUTHTYPE g_mapOneXGenAuthType [] = {
	{WPS_AUTHTYPE_OPEN,	PN_AUTH_OPEN, "open", "Open", KEY_MGMT_OPEN},
	{WPS_AUTHTYPE_WPAPSK, PN_AUTH_WPA_PSK, "WPAPSK", "WPAPSK", KEY_MGMT_WPAPSK},
	{WPS_AUTHTYPE_SHARED, PN_AUTH_SHARED, "shared", "Shared", KEY_MGMT_SHARED},
	{WPS_AUTHTYPE_WPA2PSK, PN_AUTH_WPA_PSK, "WPA2PSK", "WPA2PSK", KEY_MGMT_WPA2PSK},  // Alway use PN_AUTH_WPA_PSK for preflib as it will take care both WPA2 and WPA
	{WPS_AUTHTYPE_WPAPSK | WPS_AUTHTYPE_WPA2PSK, PN_AUTH_WPA_PSK, "WPA2PSK", "WPA2PSK", KEY_MGMT_WPAPSK_WPA2PSK}  // Always use WPA2PSK and require XP SP2 wireless hotfix.
};

// . Define map across different layer's definition for 802.1x encryption type
// . Alway use "AES" for "TKIP + AES" as "AES" is preferred method and supported across all OS and supplicants that WPS
//   SDK supports currently.
// . It is a little tricky when setting PN_CRYPT value. Always use PN_CRYPT_AUTO for both WPA "TKIP" and "AES" as TrayApp service can figure out automatically when
//   making connection. Also "WEP" mode does not work for TrayApp no matter what PN_CRYPT is set. There may be some incompatibility somewhere. WEP does work for WZC
//   ACM though
const ST_ONEXGENENCRTYPE g_mapOneXGenEncryType[] = {
	{WPS_ENCRTYPE_NONE,	PN_CRYPT_NONE, "none", "None"},
	{WPS_ENCRTYPE_WEP, PN_CRYPT_WEP, "WEP", "WEP"},
	{WPS_ENCRTYPE_TKIP, PN_CRYPT_AUTO, "TKIP", "TKIP"},
	{WPS_ENCRTYPE_AES, PN_CRYPT_AUTO, "AES", "AES"},
	{WPS_ENCRTYPE_TKIP | WPS_ENCRTYPE_AES, PN_CRYPT_AUTO, "AES", "AES"}  // Use "Auto" for preflib and "AES" for WZC/ACM as preferred method
};

/* Structure to hold the adapter information */
typedef struct _adapter_info {
	HANDLE irh;
	TCHAR adaptername[80];
	TCHAR shortadaptername[80];
	uint8 macaddress[6];
	char ssid[64];
} adapter_info;

/* Query oid */
typedef struct _tWL_QUERY_OID_HEADER {	
	ULONG	oid;			// = OID_BCM_GET_INFORMATION
	getinformation_t	getinfo_hdr;	
} WL_QUERY_OID_HEADER, *PWL_QUERY_OID_HEADER;

/* Set oid */
typedef struct _tWL_SET_OID_HEADER {
	ULONG	oid;			// = OID_BCM_SETINFORMATION
	setinformation_t		setinfo_hdr;
	// followed by oid-specific parameters
} WL_SET_OID_HEADER, *PWL_SET_OID_HEADER;

typedef struct WPS_OSL_S
{
	void *cb_ctx;			/* Client call back context */
	fnWpsProcessCB cb;		/* Client call back function for status update */

	DWORD ndevs;
	DWORD os_ver;
	ADAPTER devlist[20];
	adapter_info stAdapterInfo;
	adapter_info stGpioAdapterInfo;
	bool bWpsLedEnabled;
	HMODULE hWlanApi;
	bool associated;
} WPS_OSL_T;

extern WPS_OSL_T *wps_osl_wksp;

extern bool InitializeNetworkEnvironment();

extern bool RestoreNetworkEnvironment();

extern BOOL DoApplyNetwork(const wps_credentials *cred);

extern BOOL wps_create_wlan_profile(LPCTSTR cszAdapterName, const wps_credentials *pCredential);

#ifdef __cplusplus 
}  // extern "C"
#endif

#endif  // __WPS_API_H__
