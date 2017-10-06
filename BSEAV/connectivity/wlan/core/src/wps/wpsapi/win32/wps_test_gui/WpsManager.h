/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: WpsManager.h,v 1.6 2009/10/17 00:11:04 Exp $
 */

#pragma once

#include "wps_sdk.h"

#define MAX_PIN_LEN		32
#define WM_WPS_STATUS	WM_USER + 1
#define WM_WPS_HWTRIGGER_STATE_UPDATE	WM_USER + 2
/*
enum eBRCM_WPSMODE
{
	BRCM_WPSMODE_PBC,
	BRCM_WPSMODE_PIN
};
*/

enum eBRCM_GENERATECREDMODE
{
	BRCM_GENERATECRED_MANUAL,
	BRCM_GENERATECRED_RANDOM
};

enum eBRCM_WPSSTATE
{
	BRCM_WPSSTATE_IDLE,
	BRCM_WPSSTATE_SEARCH_AP,
	BRCM_WPSSTATE_WAITING_AP_SELECTION,
	BRCM_WPSSTATE_JOIN_AP,
	BRCM_WPSSTATE_GET_WPS_SETTING,
	BRCM_WPSSTATE_CREATE_PROFILE,

	// Defined for temporary use now. Update later by mapping all the WPS_STATUS_X event to one of above state
	BRCM_WPSSTATE_INFO  
};

typedef struct _STAP
{
	_STAP() 
	{
		strcpy(ssid, "");
		for(int i=0; i<6; i++)
			bssid[i] = 0;
		wep = 0;
	}
	_STAP(const char *ssid_in, uint8 *bssid_in, uint8 wep_in) 
	{
		if(ssid_in) strcpy(ssid, ssid_in);
		for(int i=0; i<6; i++)
			bssid[i] = bssid_in[i];
		wep = wep_in;
	}
	_STAP(const _STAP& ap) 
	{
		if(ap.ssid) strcpy(ssid, ap.ssid);
		for(int i=0; i<6; i++)
			bssid[i] = ap.bssid[i];
		wep = ap.wep;
	}
	~_STAP() { }

	const _STAP& operator=(const _STAP& ap) 
	{
		strcpy(ssid, ap.ssid);
		for(int i=0; i<6; i++)
			bssid[i] = ap.bssid[i];
		wep = ap.wep;
		return (*this);
	}

	char ssid[256];
	uint8 bssid[6];  // bssid (aka. MAC Address) is a 48-bit identifier
	uint8 wep;
} STAP, *LPSTAP;

typedef struct event_s
{
	int type;
	char buf[4096];
	uint32 buf_len;
} EVENT_T;
#define GET_EVENT_SUCCESS	1
#define GET_EVENT_ERROR		2
#define GET_EVENT_IDLE		3

#define EVENT_TYPE_WPS		1

#define PROCESS_RESULT_SUCCESS		1
#define PROCESS_RESULT_ERROR		2
#define PROCESS_RESULT_REJOIN		3
#define PROCESS_RESULT_CANCELED		4
#define PROCESS_RESULT_IDLE			5

typedef std::vector<STAP*> VEC_AP;
typedef std::vector<STAP*>::iterator ITR_AP;

// define WPS manager status callback
typedef void (WINAPI *FPWPSSTATUSCALLBACK)(UINT nState, UINT nWpsStatus, LPVOID lpData);

class CWpsManager
{
public:
	CWpsManager();
	~CWpsManager(void);

	bool StartJoinNW(CString& strAdapterGuid, eWPS_MODE eWpsMode, LPCSTR cszPin, const HWND hWnd);
	bool WpsConfigureAP(CString& strAdapterGuid, LPCSTR cszPin, const HWND hWnd, const wps_credentials *pNetworkCred);
	void UpdateStatus(unsigned int uiStatus, LPVOID data);
	bool AbortWps(bool bImmediate = FALSE);
	void SetSelectedWpsAP(const LPSTAP pSelectedAP);
	void SetPinCode(LPCSTR lpcszPin);
	void WpsCleanup();
	void StartMonitorWpsHWTrigger(CString& strAdapterGuid, HWND hWnd);
	void AbortWpsGpioPulling();

// Gettors
	const VEC_AP& GetWpsAPs() const
	{
		return m_vecWpsAPs;
	}

	eWPS_MODE GetWpsMode() const
	{
		return m_eWpsMode;
	}

	const char* const GetPinCode() const
	{
		return m_szWpsPin;
	}

	HWND GetListenerWnd() const
	{
		return m_hWndListener;
	}

	int GeResult() const
	{
		return m_result;
	}
	const LPSTAP GetSelectedWpsAP() const;

	static void _wps_join_callback(void *context, unsigned int uiStatus, void *data);

	static int get_event(EVENT_T *event);
	static int process_success(HWND listener);
	static int process_event(HWND listener, LPSTAP pSelectedAP);
	static int process_timeout(HWND listener, LPSTAP pSelectedAP);
	static void process(HWND listener, LPSTAP pSelectedAP, int result);
		

	static DWORD WINAPI WpsJoinNWThreadProc(LPVOID lpParam);
	static DWORD WINAPI WpsConfigureAPThreadProc(LPVOID lpParam);
	static DWORD WINAPI WpsHWTriggerMonitorThreadProc(LPVOID lpParam);

	VEC_AP m_vecWpsAPs;
	int m_nTotalFoundAP;
	wps_credentials *m_pNetworkCred;
	CString m_strAdapterGuid;
	BOOL m_bMonitorHwButtonInProgress;

private:
	HWND m_hWndListener;
	HANDLE m_hWpsThread;
	HANDLE m_hWpsHwTriggerMonThd;
	bool m_bWpsOpened;
	eWPS_MODE m_eWpsMode;
	char m_szWpsPin[MAX_PIN_LEN+1];
	FPWPSSTATUSCALLBACK m_fpWpsStatusCb;
	LPSTAP m_pSelectedAP;
	int m_result;
};
