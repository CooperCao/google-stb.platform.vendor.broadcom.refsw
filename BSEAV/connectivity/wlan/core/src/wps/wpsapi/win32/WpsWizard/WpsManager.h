/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: WpsManager.h,v 1.10 2008/10/14 18:06:57 Exp $
 */

#pragma once

#include "wps_sdk.h"

#define MAX_PIN_LEN		32
#define WM_WPS_STATUS	WM_USER + 1
#define WM_WPS_HWTRIGGER_STATE_UPDATE	WM_USER + 2

// Define WPS process error code
#define BRCM_WPS_SUCCESS			0
#define BRCM_WPS_ERROR_GENERIC		1

// Define the WPS information passing format
#define WPS_CB_MSG(status,info)		MAKEWPARAM(status,info)
#define WPS_CB_STATUS(message)		LOWORD(message)
#define WPS_CB_INFO(message)		HIWORD(message)

#define SIZE_MAC_ADDR	6
const uint8 ZERO_MAC[SIZE_MAC_ADDR] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define WPS_AP_SERCH_TIMEOUT		120  // in seconds

enum eBRCM_GENERATECREDMODE
{
	BRCM_GENERATECRED_MANUAL,
	BRCM_GENERATECRED_RANDOM
};

enum eBRCM_WPSSTATE
{
	BRCM_WPSSTATE_IDLE,
	BRCM_WPSSTATE_INITIALIZING,
	BRCM_WPSSTATE_SEARCHING_AP,
	BRCM_WPSSTATE_JOINING_AP,
	BRCM_WPSSTATE_NEGO_RETR,
	BRCM_WPSSTATE_CREATE_PROFILE,
	BRCM_WPSSTATE_RESTORE_ENV,

	// Defined for temporary use now. Update later by mapping all the WPS_STATUS_X event to one of above state
	BRCM_WPSSTATE_INFO,
	BRCM_WPSSTATE_UNKNOWN = 100
};

enum eBRCM_WPSEVENT
{
	BRCM_WPSEVENT_START_WPS,
	BRCM_WPSEVENT_INIT_FAIL,
	BRCM_WPSEVENT_INIT_SUCCESS,				// Trigger AP scanning
	BRCM_WPSEVENT_NO_AP_FOUND,				// No AP is scanned
	BRCM_WPSEVENT_AP_FOUND,					// One or multiple APs are scanned
	BRCM_WPSSTATE_WAITING_AP_SELECTION,		// Multiple AP found in PIN mode
	BRCM_WPSEVENT_NO_AP_SELECTED,
	BRCM_WPSEVENT_MULTI_AP_OVERLAP,			// Multiple AP found in PBC mode, this is overlap
	BRCM_WPSEVENT_MULTI_UNCONFIG_AP,		// Multiple unconfigured AP found
	BRCM_WPSEVENT_JOIN_AP,
	BRCM_WPSEVENT_JOIN_AP_SUCCESS,
	BRCM_WPSEVENT_JOIN_AP_FAIL,
	BRCM_WPSEVENT_WPS_NEGO_SUCCESS,
	BRCM_WPSEVENT_WPS_NEGO_FAIL,
	BRCM_WPSEVENT_CREATE_PROFILE_END,
	BRCM_WPSEVENT_RESTORE_WIFI_ENV_START,
	BRCM_WPSEVENT_RESTORE_WIFI_ENV_END,
	BRCM_WPSEVENT_ABORT_WPS,
	BRCM_WPSEVENT_CB_STATUS_UPDATE = 100
};

enum ePROCESS_RESULT {
	PROCESS_RESULT_SUCCESS,
	PROCESS_RESULT_ERROR,
	PROCESS_RESULT_REJOIN,
	PROCESS_RESULT_CANCELED,
	PROCESS_RESULT_IDLE
};

struct ST_STATETRANSIATION
{
	eBRCM_WPSSTATE eCurrentState;
	eBRCM_WPSEVENT eEvent;
	eBRCM_WPSSTATE eNextState;
};

// Define the state transition map based on the WPS configuration state diagram
static ST_STATETRANSIATION g_mapStateTransition[] = {
	// Leaving "Idle" state
	{BRCM_WPSSTATE_IDLE,			BRCM_WPSEVENT_START_WPS,			BRCM_WPSSTATE_INITIALIZING},

	// Leaving "WpsInitializing" state
	{BRCM_WPSSTATE_INITIALIZING,	BRCM_WPSEVENT_INIT_SUCCESS,			BRCM_WPSSTATE_SEARCHING_AP},
	{BRCM_WPSSTATE_INITIALIZING,	BRCM_WPSEVENT_INIT_FAIL,			BRCM_WPSSTATE_IDLE},
	
	// Leaving "ScanningWpsAP" state
	{BRCM_WPSSTATE_SEARCHING_AP,	BRCM_WPSEVENT_AP_FOUND,				BRCM_WPSSTATE_JOINING_AP},	
	{BRCM_WPSSTATE_SEARCHING_AP,	BRCM_WPSEVENT_NO_AP_FOUND,			BRCM_WPSSTATE_RESTORE_ENV},

	// Leaving "JoiningAP" state
	{BRCM_WPSSTATE_JOINING_AP,		BRCM_WPSEVENT_MULTI_AP_OVERLAP,		BRCM_WPSSTATE_RESTORE_ENV},
	{BRCM_WPSSTATE_JOINING_AP,		BRCM_WPSEVENT_MULTI_UNCONFIG_AP,	BRCM_WPSSTATE_RESTORE_ENV},
	{BRCM_WPSSTATE_JOINING_AP,		BRCM_WPSEVENT_JOIN_AP,				BRCM_WPSSTATE_JOINING_AP},
	{BRCM_WPSSTATE_JOINING_AP,		BRCM_WPSEVENT_NO_AP_SELECTED,		BRCM_WPSSTATE_RESTORE_ENV},
	{BRCM_WPSSTATE_JOINING_AP,		BRCM_WPSEVENT_JOIN_AP_SUCCESS,		BRCM_WPSSTATE_NEGO_RETR},
	{BRCM_WPSSTATE_JOINING_AP,		BRCM_WPSEVENT_JOIN_AP_FAIL,			BRCM_WPSSTATE_RESTORE_ENV},

	// Leaving "WpsNegotiatingRetrievingAPSettings" state
	{BRCM_WPSSTATE_NEGO_RETR,		BRCM_WPSEVENT_WPS_NEGO_SUCCESS,		BRCM_WPSSTATE_CREATE_PROFILE},
	{BRCM_WPSSTATE_NEGO_RETR,		BRCM_WPSEVENT_WPS_NEGO_FAIL,		BRCM_WPSSTATE_RESTORE_ENV},
	
	// Leaving "CreatingNetworkProfile" state
	{BRCM_WPSSTATE_CREATE_PROFILE,	BRCM_WPSEVENT_CREATE_PROFILE_END,	BRCM_WPSSTATE_RESTORE_ENV},
	
	// Entering "RestoringNetworkEnv" state
	{BRCM_WPSSTATE_RESTORE_ENV,		BRCM_WPSEVENT_RESTORE_WIFI_ENV_START, BRCM_WPSSTATE_RESTORE_ENV},
	
	// Leaving "RestoringNetworkEnv" state
	{BRCM_WPSSTATE_RESTORE_ENV,		BRCM_WPSEVENT_RESTORE_WIFI_ENV_END, BRCM_WPSSTATE_IDLE},
};

typedef struct _STAP
{
	_STAP(wps_apinf *ap_info) 
	{
		memset(this, 0, sizeof(_STAP));
		if (ap_info)
		{
			memcpy(ssid, ap_info->ssid, sizeof(ap_info->ssid));
			memcpy(bssid, ap_info->bssid, 6);
			wep = ap_info->wep;
			band = ap_info->band;
			is_configured = ap_info->configured;
			memcpy(authorizedMACs, ap_info->authorizedMACs, sizeof(ap_info->authorizedMACs));
			bVer2 = ap_info->version2;
		}
	}

	~_STAP() { }

	char ssid[SIZE_32_BYTES+1];
	uint8 bssid[6];  // bssid (aka. MAC Address) is a 48-bit identifier
	uint8 wep;
	uint16 band;
	bool is_configured;
	uint8 authorizedMACs[SIZE_MAC_ADDR * 5];
	BOOL bVer2;
} STAP, *LPSTAP;

typedef std::vector<STAP*> VEC_AP;
typedef std::vector<STAP*>::iterator ITR_AP;

typedef struct event_s
{
	int type;
	char buf[4096];
	uint32 buf_len;
} EVENT_T;

// define WPS manager status callback
typedef void (WINAPI *FPWPSSTATUSCALLBACK)(UINT nState, UINT nWpsStatus, LPVOID lpData);

class CWpsManager
{
public:
	CWpsManager();
	~CWpsManager(void);

	bool StartWps(eWPS_MODE eWpsMode, HWND hWnd, bool bAutoConnect);  // Notify via messaging
	void UpdateStatus(eBRCM_WPSEVENT nWpsEvent, DWORD dwError);
	void AbortWps(BOOL bImmediate = FALSE);
	void SetSelectedWpsAP(const LPSTAP pSelectedAP);
	void SetPinCode(LPCTSTR lpcszPin);
	bool WpsCleanup();
	void SetWpsCredential(const wps_credentials& cred);
	void ClearNetworkList();
	BOOL GetWpsInProgress() { return m_bWpsInProcess; }
	BOOL DoesAPAuthorize(const LPSTAP pAP);
	int SearchWpsAPs(int nTimeout, bool bPbcMode, bool bAutoConnect, bool bAuthorizedAPOnly);

// Gettors
	eWPS_MODE GetWpsMode()
	{
		return m_eWpsMode;
	}

	void SetWpsMode(eWPS_MODE eWpsMode)
	{
		m_eWpsMode = eWpsMode;
	}

	const char* const GetPinCode() const
	{
		return m_szWpsPin;
	}

	HWND GetListenerWnd()
	{
		return m_hWndListener;
	}
	
	wps_credentials *GetWpsCred()
	{
		return m_pNetworkCred;
	}
	
	ePROCESS_RESULT GetProcessResult()
	{
		return m_eProcessResult;
	}

	const LPSTAP GetSelectedWpsAP() const;
	VEC_AP& GetWpsAPs();

	int get_event(EVENT_T *event);
	int process_success(HWND listener);
	int process_event(HWND listener, LPSTAP pSelectedAP);
	int process_timeout(HWND listener, LPSTAP pSelectedAP);
	int process(HWND listener, LPSTAP pSelectedAP);

	static bool _wps_join_callback(void *context, unsigned int uiStatus, void *data);
	static DWORD WINAPI WpsJoinNWThreadProc(LPVOID lpParam);

	int m_nTotalFoundAP;
	bool m_bWpsSuccess;
	bool m_bRequestAbort;
	bool m_bWpsV2;

private:
	VEC_AP m_vecWpsAPs;
	HWND m_hWndListener;
	HANDLE m_hWpsThread;
	HANDLE m_hWpsHwTriggerMonThd;
	bool m_bWpsOpened;
	eWPS_MODE m_eWpsMode;
	char m_szWpsPin[MAX_PIN_LEN+1];
	FPWPSSTATUSCALLBACK m_fpWpsStatusCb;
	LPSTAP m_pSelectedAP;
	wps_credentials *m_pNetworkCred;
	bool m_bAutoConnect;
	BOOL m_bWpsInProcess; 
	ePROCESS_RESULT m_eProcessResult;
	uint8 m_DevMacAddr[SIZE_MAC_ADDR];
};
