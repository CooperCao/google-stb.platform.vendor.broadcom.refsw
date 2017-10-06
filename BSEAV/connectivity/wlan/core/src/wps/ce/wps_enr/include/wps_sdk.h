/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef _WPS_SDK_H_
#define _WPS_SDK_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Definition of Wi-Fi security encryption mask type */
#define WPS_ENCRYPT_NONE	0x0001
#define WPS_ENCRYPT_WEP		0x0002
#define WPS_ENCRYPT_TKIP	0x0004
#define WPS_ENCRYPT_AES		0x0008

/* Definition of Wi-fi ban */
#define WL_CHANSPEC_BAND_5G		0x1000
#define WL_CHANSPEC_BAND_2G		0x2000

typedef unsigned int	uint32;
typedef unsigned char	uint8;
typedef unsigned short	uint16;

#undef SIZE_20_BYTES
#undef SIZE_32_BYTES
#undef SIZE_64_BYTES
#define SIZE_20_BYTES	20
#define SIZE_32_BYTES	32
#define SIZE_64_BYTES	64

#define ENCRYPT_NONE	1
#define ENCRYPT_WEP		2
#define ENCRYPT_TKIP	4
#define ENCRYPT_AES		8

#undef WPS_SCSTATE_UNKNOWN
#undef WPS_SCSTATE_UNCONFIGURED
#undef WPS_SCSTATE_CONFIGURED

/* Simple Config state */
enum eWPS_CONIG_STATE {
	WPS_SCSTATE_UNKNOWN	= 0,
	WPS_SCSTATE_UNCONFIGURED,
	WPS_SCSTATE_CONFIGURED
};

/* STA_ENR_JOIN_NW_PIN and STA_REG_CONFIG_NW require PIN as well */
enum eWPS_MODE {
	STA_ENR_JOIN_NW_PBC = 0,
	STA_ENR_JOIN_NW_PIN,
	STA_REG_JOIN_NW,
	STA_REG_CONFIG_NW
};

typedef struct _wps_credentials
{
	char	ssid[SIZE_32_BYTES+1];
	char	keyMgmt[SIZE_20_BYTES+1];
	char	nwKey[SIZE_64_BYTES+1];
	uint32	encrType;
	uint16	wepIndex;
} wps_credentials;

/*
	WPS status values provided to the callback wps_join_callback (see below)
*/
enum {
	WPS_STATUS_SUCCESS = 0,
	WPS_STATUS_ERROR,
	WPS_STATUS_CANCELED,
	WPS_STATUS_WARNING_TIMEOUT,
	WPS_STATUS_WARNING_WPS_PROTOCOL_FAILED,
	WPS_STATUS_WARNING_NOT_INITIALIZED,
	WPS_STATUS_DISABLING_WIFI_MANAGEMENT,
	WPS_STATUS_INIT,
	WPS_STATUS_SCANNING,
	WPS_STATUS_SCANNING_OVER,
	WPS_STATUS_ASSOCIATING,
	WPS_STATUS_ASSOCIATED,
	WPS_STATUS_STARTING_WPS_EXCHANGE,
	WPS_STATUS_SENDING_WPS_MESSAGE,
	WPS_STATUS_WAITING_WPS_RESPONSE,
	WPS_STATUS_GOT_WPS_RESPONSE,
	WPS_STATUS_DISCONNECTING,
	WPS_STATUS_ENABLING_WIFI_MANAGEMENT,
	WPS_STATUS_CREATING_PROFILE,
	WPS_STATUS_OVERALL_PROCESS_TIMOUT,
	WPS_STATUS_SCANNING_OVER_SUCCESS,
	WPS_STATUS_SCANNING_OVER_SESSION_OVERLAP,
	WPS_STATUS_SCANNING_OVER_NO_AP_FOUND,
	WPS_STATUS_CONFIGURING_ACCESS_POINT,
	WPS_STATUS_REATTEMPTING_WPS,
	WPS_STATUS_IDLE = 0xffff, /* Ignore this status notification */
};

/*
	WPS callback function definition
	if it returns FALSE, the WPS protocol will be canceled
*/
typedef bool (*fnWpsProcessCB)(void *context, unsigned int uiStatus, void *data);

#ifndef BRCM_WPSSDK
/*
	wps_open function must be called first, before any other wps api call
*/
typedef bool (*wps_open_fptr)(void *context, fnWpsProcessCB callback);

/*
	wps_close function must be called once you are done using the wps api
*/
typedef bool (*wps_close_fptr)(void);

/*	
	Since Wi-Fi managing utilities such as Microsoft Windows Zero Config can
	interfere with operation of WPS SDK, it needs to be disabled or configured
	for proper operation of WPS SDK APIs. The application should use this API
	if the platform makes use of Microsoft Windows Zero Config to manage the
	adapter. If not, the application can define its own method for disabling
	and enabling any other Wi-Fi managing service.
*/
typedef bool (*wps_configure_wzcsvc_fptr)(bool enable);
/*
	wps_findAP scans for WPS PBC APs and returns the one with the strongest RSSI
	Returns true if it finds an AP within the specified time. This function is
	designed to be called repeatidly with small timeouts in seconds (say 4 or 5 secs)
	to allow for UI updates and user cancelation. If multiple PBC APs are found,
	this is an error condition and FALSE is returned. 8nAP will contain the number
	of PBC APs found (will be greater than 1).

	The value of nAP is updated with the number of APs found. For PBC APs, it will be
	always 1 on success (or if more than 1 is returned, the UI should warn the user to
	try again later). For PIN APs, it will varie from 0 to the max numbers of the list.

	Call wps_getAP to get the APs found
*/
typedef bool (*wps_findAP_fptr)(int *nAP, int mode, int timeout);

/*
	wps_getAP returns the AP #nAP from the list of WPS APs found by wps_findAP.
*/
typedef bool (*wps_getAP_fptr)(int nAP, unsigned char * bssid, char *ssid, uint8 *wep, uint16 *band);

/*
	wps_join function is used to connect to a WPS AP. Usualy, this function is called after
	wps_findAP returns successfully
*/
typedef bool (*wps_join_fptr)(uint8 * bssid, char *ssid, uint8 wep);


/*
	This function starts the WPS exchange protocol and gathers the credentials
	of the AP. Call this function once wps_join is successful.

	The calling process provides a callback function in wps_open() that will be called
	periodically by the WPS API. When called, this callback function will be provided
	with the current status. If the calling process wants to cancel the WPS protocol, it
	should return FALSE (upon the user pressing a Cancel button, for example).

	If the calling process does not want to be called back, it should send NULL as a
	function pointer.

	GUI applications should use the asynchronous version of this function so as not to
	block or slow down a UI's message loop.
*/
typedef bool (*wps_get_AP_info_fptr)(int wps_mode, uint8 *bssid, char *ssid, char *pin, wps_credentials *credentials);

/*
	Asynchronous version of wps_get_AP_info(). This function returns immediately and starts the
	WPS protocol in a separate thread.
	The calling process uses the status callback to determine the state of the WPS protocol.

	The calling process will get a WPS_STATUS_SUCCESS once the WPS protocol completed successfully
	The calling process will get a WPS_STATUS_ERROR if the WPS protocol completed unsuccessfully
	The calling process will get a WPS_STATUS_CANCELED if the WPS protocol was canceled by the calling thread

	The calling process must wait for any one of these 3 status notifications or any error notification
	before calling wps_close() or terminating.

	Unlike the synchronous version of this API call, the callback parameter in wps_open()CANNOT be NULL.
	A callback is required for this function to work correctly.

	Before this function returns, it will call the calling process' callback with a status of
	WPS_STATUS_START_WPS_EXCHANGE

*/
typedef bool (*wps_get_AP_infoEx_fptr)(int wps_mode, uint8 * bssid, char *ssid, char *pin, wps_credentials *credentials);

/*
	This function creates a preferred network profile that can be used by Windows Zero Config (WZC)
	to connect to the network. Call this function with the results of the last WPS exchange.

	This function will return WPS_STATUS_ERROR if WZC does not control the adapter or if the creation
	of profile failed.

*/
typedef bool (*wps_create_profile_fptr)(const wps_credentials *credentials);

/*
	Configure an AP with the given networking credentials as ER (External Registrar)
*/
typedef bool (*wps_configureAP_fptr)(uint8 *bssid, const char *pin, const wps_credentials *credentials);

/*
	Randomly generate an 8-digit numeric PIN with valid checksum 8th digit
*/
typedef bool (*wps_generate_pin_fptr)(char *pin);

/*
	Generate secure "Personal" network settings in the following format:
	"SSID" - derived from STA Wi-fi adapter MAC address
	"Network Key" - well-formated network key derived from random bytes
	"Authentication Method" - Fixed "WPA2-PSK"
	"Encryption Method" - Fixed "AES"
*/
typedef bool (*wps_generate_cred_fptr)(wps_credentials *credentials);

/*
	Return whether the Registrar associated with the given AP is recently activated or not
*/
typedef bool (*wps_is_reg_activated_fptr)(const uint8 *bssid);

/*
	Return whether the given PIN code passes the WPS PIN checksum validation or not. 8th digit of valid PIN
	is computerd by the other 7 PIN digits according to WPS specification
*/
typedef bool (*wps_validate_checksum_fptr)(const unsigned long pin);

/*
	Return the eWPS_CONIG_STATE state of whether the network is configured or not
*/
typedef uint8 (*wps_get_AP_scstate_fptr)(const uint8 *bssid);
#else
#define DLLExport __declspec(dllexport)

DLLExport bool wps_open(void *context, fnWpsProcessCB callback);
DLLExport bool wps_close(void);
DLLExport bool wps_configure_wzcsvc(bool enable);
DLLExport bool wps_findAP(int *nAP, int mode, int timeout);
DLLExport bool wps_getAP(int nAP, unsigned char * bssid, char *ssid, uint8 *wep, uint16 *band);
DLLExport bool wps_join(uint8 * bssid, char *ssid, uint8 wep);
DLLExport bool wps_get_AP_info(int wps_mode, uint8 *bssid, char *ssid, char *pin, wps_credentials *credentials);
DLLExport bool wps_get_AP_infoEx(int wps_mode, uint8 * bssid, char *ssid, char *pin, wps_credentials *credentials);
DLLExport bool wps_create_profile(const wps_credentials *credentials);
DLLExport bool wps_configureAP(uint8 *bssid, const char *pin, const wps_credentials *credentials);
DLLExport bool wps_generate_pin(char *pin);
DLLExport bool wps_generate_cred(wps_credentials *credentials);
DLLExport bool wps_is_reg_activated(const uint8 *bssid);
DLLExport bool wps_validate_checksum(const unsigned long pin);
DLLExport uint8 wps_get_AP_scstate(const uint8 *bssid);
#endif /* !BRCM_WPSSDK */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __BRCM_WPSAPI_Hs_ */
