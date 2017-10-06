/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: WpsManager.cpp,v 1.12 2009/10/17 00:11:04 Exp $
 */

#include "StdAfx.h"
#include "WpsManager.h"
#include <windows.h>


#define WPS_SELECTION_EVENT			_T("BRCM_WPS_SELECTION_EVENT")
#define WPS_SELECTION_EVENT_TIMEOUT	(120*1000)  // Milliseconds
#define WPS_CANCELLING_EVENT		_T("BRCM_WPS_CANCELLING_EVENT")
#define WPS_CANCELLING_EVENT_TIMEOUT	(10*1000)  // Milliseconds
#define WPS_HWTRIGGER_POLL_INTERVAL		50  // Milliseconds

static HANDLE g_hEvtWpsSelection = NULL;
static HANDLE g_hEvtWpsCancelling = NULL;
//static bool g_bContinue = TRUE;

CWpsManager::CWpsManager(void)
{
	m_hWndListener = NULL;
	m_hWpsThread = NULL;
	m_hWpsHwTriggerMonThd = NULL;
	m_bWpsOpened = false;
	m_eWpsMode = STA_ENR_JOIN_NW_PBC;
	m_fpWpsStatusCb  = NULL;
	m_pSelectedAP = NULL;
	m_nTotalFoundAP = 0;
	m_pNetworkCred = NULL;
	m_bMonitorHwButtonInProgress = FALSE;
	m_result = PROCESS_RESULT_IDLE;
}

CWpsManager::~CWpsManager(void)
{
}


bool CWpsManager::StartJoinNW(CString& strAdapterGuid, eWPS_MODE eWpsMode, LPCSTR cszPin, const HWND hWnd)
{
	m_eWpsMode = eWpsMode;
	m_hWndListener = hWnd;
	m_nTotalFoundAP = 0;  // reset to 0
	m_strAdapterGuid = strAdapterGuid;

	if (m_eWpsMode == STA_REG_JOIN_NW)
		return WpsConfigureAP(strAdapterGuid, cszPin, hWnd, NULL);

	if(m_eWpsMode != STA_ENR_JOIN_NW_PBC)
	{
		// If PIN mode is selected, pin code can't be empty
		// The PIN could be STA/AP PIN
		if(cszPin && strlen(cszPin))
			strcpy(m_szWpsPin, cszPin);
		else
			return false;
	}
	
	// Create thread to start WPS process
	DWORD dwThreadId;
	m_hWpsThread = CreateThread(NULL, 0, WpsJoinNWThreadProc, this, 0, &dwThreadId);

	return true;
}

bool CWpsManager::WpsConfigureAP(CString& strAdapterGuid, LPCSTR cszPin, const HWND hWnd, const wps_credentials *pNetworkCred)
{
	m_hWndListener = hWnd;
	m_nTotalFoundAP = 0;  // reset to 0
	m_eWpsMode = STA_REG_JOIN_NW;
	m_strAdapterGuid = strAdapterGuid;

	// The PIN will be AP's PIN
	if(cszPin && strlen(cszPin))
		strcpy(m_szWpsPin, cszPin);
	else
		return false;

	if(pNetworkCred)
	{
		m_pNetworkCred = new wps_credentials();
		strcpy(m_pNetworkCred->ssid, pNetworkCred->ssid);
		strcpy(m_pNetworkCred->keyMgmt, pNetworkCred->keyMgmt);
		strcpy(m_pNetworkCred->nwKey, pNetworkCred->nwKey);
		m_pNetworkCred->encrType = pNetworkCred->encrType;
		m_eWpsMode = STA_REG_CONFIG_NW;
	}

	// Create thread to start WPS process to configure AP
	DWORD dwThreadId;
	m_hWpsThread = CreateThread(NULL, 0, WpsConfigureAPThreadProc, this, 0, &dwThreadId);

	return true;
}

void CWpsManager::WpsCleanup()
{
	wps_api_close();
/*
	if(g_hEvtWpsSelection)
		CloseHandle(g_hEvtWpsSelection);

	if(g_hEvtWpsCancelling)
		CloseHandle(g_hEvtWpsCancelling);
*/
	for(int i=0; i<m_vecWpsAPs.size(); i++)
	{
		if(m_vecWpsAPs[i])
			delete m_vecWpsAPs[i];
	}
	m_vecWpsAPs.clear();

	if(m_pNetworkCred)
	{
		delete m_pNetworkCred;
		m_pNetworkCred = NULL;
	}
}

bool CWpsManager::AbortWps(bool bImmediate)
{
	// Set cancellation flag returned by the callback function
	//g_bContinue = FALSE;
	m_result = PROCESS_RESULT_CANCELED;
	
	wps_api_abort();

	if(m_hWpsThread)
	{
		DWORD dwExitCode = 0;
		int nWaitTime = 0;
		while(GetExitCodeThread(m_hWpsThread, &dwExitCode) && dwExitCode == STILL_ACTIVE)
		{
			if(bImmediate)
			{
				TerminateThread(m_hWpsThread, 0);
				m_hWpsThread = NULL;
			}
			else
			{
				Sleep(100);
				nWaitTime += 100;
				if(nWaitTime >= WPS_CANCELLING_EVENT_TIMEOUT)
				{
					TerminateThread(m_hWpsThread, 0);
					m_hWpsThread = NULL;
				}
			}
		}
	}

	WpsCleanup();
	
	// Give some time for the cleanup to complete. Not 100% sure if this is needed (just in case). This is just by experience
	// and more investigation may be needed 
	Sleep(1000);  

	return true;
}

void CWpsManager::SetSelectedWpsAP(const LPSTAP pSelectedAP)
{
	m_pSelectedAP = pSelectedAP;
	if(g_hEvtWpsSelection)
		SetEvent(g_hEvtWpsSelection);
}

const LPSTAP CWpsManager::GetSelectedWpsAP() const
{
	return m_pSelectedAP;
}

void CWpsManager::SetPinCode(const char* szPin)
{
	strcpy(m_szWpsPin, szPin);
}

//bool CWpsManager::_wps_join_callback(void *context, unsigned int uiStatus, void *data)
void CWpsManager::_wps_join_callback(void *context, unsigned int uiStatus, void *data)
{
	CWpsManager *pWpsMgr = (CWpsManager *)context;

	if(!pWpsMgr)
		//return false;  // Unexpected
		return;

	// Check if WPS Canceling is done by lower API
	if(uiStatus == WPS_STATUS_CANCELED)
	{
		if(g_hEvtWpsCancelling)
			SetEvent(g_hEvtWpsCancelling);
	}

	pWpsMgr->UpdateStatus(uiStatus, data);
	//return g_bContinue;
	return;
}

int CWpsManager::get_event(EVENT_T *event)
{
	uint32 retVal;
	
	/* Now we only have WPS event */
	event->buf_len = sizeof(event->buf);
	retVal = wps_api_poll_eapol_packet(event->buf, &event->buf_len);
	if (retVal == WPS_STATUS_SUCCESS) {
		event->type = EVENT_TYPE_WPS;
		return GET_EVENT_SUCCESS;
	}
	else if (retVal == WPS_STATUS_ERROR)
		return GET_EVENT_ERROR;
	else
		return GET_EVENT_IDLE;
}

int CWpsManager::process_success(HWND listener)
{
	wps_credentials credentials;

	PostMessage(listener, WM_WPS_STATUS, BRCM_WPSSTATE_CREATE_PROFILE,
			WPS_STATUS_CREATING_PROFILE);

	if (wps_api_get_credentials(&credentials) == NULL) {
		PostMessage(listener, WM_WPS_STATUS, BRCM_WPSSTATE_CREATE_PROFILE,
			WPS_STATUS_ERROR);
		return PROCESS_RESULT_ERROR;
	}

	if (wps_api_create_profile(&credentials) == false) {
		PostMessage(listener, WM_WPS_STATUS, BRCM_WPSSTATE_CREATE_PROFILE,
			WPS_STATUS_ERROR);
		return PROCESS_RESULT_ERROR;
	}
	else {
		PostMessage(listener, WM_WPS_STATUS, BRCM_WPSSTATE_CREATE_PROFILE,
			WPS_STATUS_SUCCESS);
		return PROCESS_RESULT_SUCCESS;
	}
}

int CWpsManager::process_event(HWND listener, LPSTAP pSelectedAP)
{
	int retVal;
	EVENT_T event;

	retVal = get_event(&event);
	if (retVal != GET_EVENT_SUCCESS) {
		if (retVal == GET_EVENT_ERROR)
			return PROCESS_RESULT_ERROR;

		return PROCESS_RESULT_IDLE;
	}

	/* Now we only process WPS event */
	retVal = wps_api_process_data(event.buf, event.buf_len);
	if (retVal == WPS_STATUS_SUCCESS)
		return process_success(listener);
	else if (retVal == WPS_STATUS_REJOIN) {
		if (wps_api_join(pSelectedAP->bssid, pSelectedAP->ssid, pSelectedAP->wep) == false) {
			/* Connecting Failed */
			PostMessage(listener, WM_WPS_STATUS, BRCM_WPSSTATE_JOIN_AP,
				WPS_STATUS_ERROR);
			return PROCESS_RESULT_ERROR;
		}
		/* Tell wps_api link up */
		wps_api_set_linkup();

		return PROCESS_RESULT_REJOIN;
	}
	else if (retVal == WPS_STATUS_ERROR)
		return PROCESS_RESULT_ERROR;

	return PROCESS_RESULT_IDLE;
}

int CWpsManager::process_timeout(HWND listener, LPSTAP pSelectedAP)
{
	int retVal;

	/* Now we only process WPS timeout */
	retVal = wps_api_process_timeout();
	if (retVal == WPS_STATUS_REJOIN) {
		if (wps_api_join(pSelectedAP->bssid, pSelectedAP->ssid, pSelectedAP->wep) == false) {
			/* Connecting Failed */
			PostMessage(listener, WM_WPS_STATUS, BRCM_WPSSTATE_JOIN_AP,
				WPS_STATUS_ERROR);
			return PROCESS_RESULT_ERROR;
		}
		/* Tell wps_api link up */
		wps_api_set_linkup();

		return PROCESS_RESULT_REJOIN;
	}
	else if (retVal == WPS_STATUS_ERROR)
		return PROCESS_RESULT_ERROR;
	else if (retVal == WPS_STATUS_SUCCESS)
		return process_success(listener);

	return PROCESS_RESULT_IDLE;
}

void CWpsManager::process(HWND listener, LPSTAP pSelectedAP, int result)
{
	/* Not Async mode */
	while (1) {
		/* Event process */
		switch (process_event(listener, pSelectedAP)) {
		case PROCESS_RESULT_SUCCESS:
		case PROCESS_RESULT_CANCELED:
		case PROCESS_RESULT_ERROR:
			return;

		default:
			break;
		}

		/* Timeout process */
		switch (process_timeout(listener, pSelectedAP)) {
		case PROCESS_RESULT_SUCCESS:
		case PROCESS_RESULT_ERROR:
		case PROCESS_RESULT_CANCELED:
			return;

		default:
			break;
		}

		/* User canceled */
		if (result == PROCESS_RESULT_CANCELED)
			break;
	}
}

void CWpsManager::UpdateStatus(unsigned int uiStatus, LPVOID data)
{
	if(m_fpWpsStatusCb)
	{
		// TBA: Map each WPS Status to a WPS state
		eBRCM_WPSSTATE eState = BRCM_WPSSTATE_IDLE;
		// TBA: Map each WPS Status to a WPS state

		m_fpWpsStatusCb(eState, uiStatus, data);
	}

	if(m_hWndListener)
	{
		// TBA: Map each WPS Status to a WPS state
		eBRCM_WPSSTATE eState = BRCM_WPSSTATE_INFO;
		// TBA: Map each WPS Status to a WPS state

		// Notify
		PostMessage(m_hWndListener, WM_WPS_STATUS, eState, uiStatus);
	}
}

DWORD WINAPI CWpsManager::WpsJoinNWThreadProc(LPVOID lpParam)
{
	bool b_v2 = true;
	bool bAPPin = false;

	// Pass in CWpsManager pointer lpParam 
	CWpsManager *pWpsMgr = (CWpsManager *)lpParam;
	if(!pWpsMgr)
		return 0;

	// Reset g_bContinue flag
	//g_bContinue = TRUE;

	// Initialize WPS process
	if(!wps_api_open(pWpsMgr->m_strAdapterGuid.GetLength() == 0? NULL : CT2A(LPCTSTR(pWpsMgr->m_strAdapterGuid)), 
				lpParam, 
				_wps_join_callback,
				NULL,
				bAPPin,
				b_v2))
	{
		PostMessage(pWpsMgr->GetListenerWnd(), WM_WPS_STATUS, BRCM_WPSSTATE_IDLE, WPS_STATUS_ERROR);
		return 0;
	}

	bool bSuccess = false;
	int nAP = 0;
	uint8 bssid[6];  // bssid is a 48-bit identifier
	char ssid[32] = { 0 };
	uint8 wep = 1;
	uint16 band = 0;
	wps_credentials credentials;
	pWpsMgr->m_nTotalFoundAP = 0;
	char szPinLocal[MAX_PIN_LEN+1] = { 0 };
	struct wps_ap_list_info *aplist;
	wps_apinf apinf;

	PostMessage(pWpsMgr->GetListenerWnd(), WM_WPS_STATUS, BRCM_WPSSTATE_SEARCH_AP, WPS_STATUS_SCANNING);

	/* Get all APs */
	aplist = wps_api_surveying(PBC_MODE(pWpsMgr->GetWpsMode()), b_v2);

	/* Try to find:
	* 1. A PBC enabled AP when we WPS through PBC
	* 2. Collect all WPS APs when we WPS through AP's PIN or PIN
	*/
	/* TODO: in STA PIN mode
	* 1. find_pin_aps : find all APs which has PIN my MAC in AuthorizedMACs
	* 2. find_amac_ap : find an AP which has my MAC in AuthorizedMACs
	*/
	bSuccess = wps_api_find_ap(aplist, &nAP, PBC_MODE(pWpsMgr->GetWpsMode()));
	//bSuccess = wps_findAP(&nAP, pWpsMgr->GetWpsMode(), 120);
	if(bSuccess)
	{
		pWpsMgr->m_nTotalFoundAP = nAP;

		// Add all available WPS APs to the collection
		for(int i=0; i<nAP; i++)
		{
			//if(wps_getAP(i, bssid, ssid, &wep, &band))
			if(wps_api_get_ap(i, &apinf))
			{
				LPSTAP pAp = new STAP(apinf.ssid, apinf.bssid, apinf.wep);
				pWpsMgr->m_vecWpsAPs.push_back(pAp);
			}
		}
	}

	if(bSuccess)
	{
		// Notify searching AP success
		PostMessage(pWpsMgr->GetListenerWnd(), WM_WPS_STATUS, BRCM_WPSSTATE_SEARCH_AP, WPS_STATUS_SUCCESS);

		// Wait for an AP to be selected to continue for PIN mode. For PBC mode, go ahead to connect without prompting
		// as only one nearby PBC AP is allowed during WPS (otherwise bSuccess will be FALSE)
		if(pWpsMgr->GetWpsMode() != STA_ENR_JOIN_NW_PBC)
		{
			// For PIN mode, set PIN code
			if(pWpsMgr->GetPinCode())
				strcpy(szPinLocal, pWpsMgr->GetPinCode());

			// Notify to show AP selection dialog
			if(pWpsMgr->m_vecWpsAPs.size() > 0)
			{
				// More than one AP is found, wait for an AP to be selected to continue
				g_hEvtWpsSelection = CreateEvent(NULL, FALSE, FALSE, WPS_SELECTION_EVENT);

				PostMessage(pWpsMgr->GetListenerWnd(), WM_WPS_STATUS, BRCM_WPSSTATE_WAITING_AP_SELECTION, 0);

				DWORD dwRet = WaitForSingleObject(g_hEvtWpsSelection, INFINITE);
				if(dwRet == WAIT_TIMEOUT) 
				{
					bSuccess = false;
					goto WPS_END;  // Timeout and no AP is selected
				}
			}
			else
			{
				// Notify searching AP failure
				bSuccess = false;
				goto WPS_END;  // Timeout and no AP is selected
			}
		}
		else
		{
			// For PBC mode, set the found one to be selected
			LPSTAP pAP = (LPSTAP)(pWpsMgr->m_vecWpsAPs[0]);
			if(pAP)
				pWpsMgr->SetSelectedWpsAP(pAP);
		}

		// Link to selected AP
		LPSTAP pSelectedAP = pWpsMgr->GetSelectedWpsAP();
		if(pSelectedAP)
		{
			//if(bSuccess=wps_join(pSelectedAP->bssid, pSelectedAP->ssid, pSelectedAP->wep))
			if(bSuccess=wps_api_join(pSelectedAP->bssid, pSelectedAP->ssid, pSelectedAP->wep))
			{

				/* Inform link up */
				wps_api_set_linkup();
				
				/* Run WPS not async mode */
				if (wps_api_run(DEVICE_PASSWORD_ID(false, szPinLocal, NULL),
					pSelectedAP->bssid, pSelectedAP->ssid, pSelectedAP->wep,
					strlen(szPinLocal) ? szPinLocal : NULL, NULL, false) == true) {
					/* loop process data */
					process(pWpsMgr->GetListenerWnd(), pSelectedAP, pWpsMgr->GeResult());
				}

			}
		}
	}
	else
	{
		// Notify searching AP failure
		PostMessage(pWpsMgr->GetListenerWnd(), WM_WPS_STATUS, BRCM_WPSSTATE_SEARCH_AP, WPS_STATUS_ERROR);
	}

WPS_END:
	pWpsMgr->WpsCleanup();

	// This must be called after WPS process is done to do cleanup at lower level and restore 
	// network environment
	PostMessage(pWpsMgr->GetListenerWnd(), WM_WPS_STATUS, BRCM_WPSSTATE_IDLE, bSuccess? WPS_STATUS_SUCCESS : WPS_STATUS_ERROR);
	
	return 0;
}

DWORD WINAPI CWpsManager::WpsConfigureAPThreadProc(LPVOID lpParam)
{

	bool b_v2 = true;
	bool bAPPin = true;

	// Pass in CWpsManager pointer lpParam 
	CWpsManager *pWpsMgr = (CWpsManager *)lpParam;
	if(!pWpsMgr)
		return 0;

	// Reset g_bContinue flag
	//g_bContinue = TRUE;

	// Initialize WPS process
	if(!wps_api_open(pWpsMgr->m_strAdapterGuid.GetLength() == 0? NULL : CT2A(LPCTSTR(pWpsMgr->m_strAdapterGuid)), 
				lpParam, 
				_wps_join_callback,
				NULL,
				bAPPin,
				b_v2))
	{
		PostMessage(pWpsMgr->GetListenerWnd(), WM_WPS_STATUS, BRCM_WPSSTATE_IDLE, WPS_STATUS_ERROR);
		return 0;
	}

	bool bSuccess = false;
	int nAP = 0;
	uint8 bssid[6];  // bssid is a 48-bit identifier
	char ssid[32] = "no AP found\0";
	uint8 wep = 1;
	uint16 band = 0;
	pWpsMgr->m_nTotalFoundAP = 0;

	PostMessage(pWpsMgr->GetListenerWnd(), WM_WPS_STATUS, BRCM_WPSSTATE_SEARCH_AP, WPS_STATUS_SCANNING);

	char szPinLocal[MAX_PIN_LEN+1] = { 0 };
	struct wps_ap_list_info *aplist;
	wps_apinf apinf;

	strcpy(szPinLocal, pWpsMgr->GetPinCode());

	/* Get all APs */
	aplist = wps_api_surveying(PBC_MODE(pWpsMgr->GetWpsMode()), b_v2);

	// wps_findAP returns only one AP info, first one in the list.
	/* Try to find:
	* 1. A PBC enabled AP when we WPS through PBC
	* 2. Collect all WPS APs when we WPS through AP's PIN or PIN
	*/
	/* TODO: in STA PIN mode
	* 1. find_pin_aps : find all APs which has PIN my MAC in AuthorizedMACs
	* 2. find_amac_ap : find an AP which has my MAC in AuthorizedMACs
	*/
	bSuccess = wps_api_find_ap(aplist, &nAP, PBC_MODE(pWpsMgr->GetWpsMode()));
	//bSuccess=wps_findAP(&nAP, STA_REG_CONFIG_NW, 120);
	pWpsMgr->m_nTotalFoundAP = nAP;
	if(bSuccess) 
	{
		// Add all available WPS APs to the collection
		for(int i=0; i<nAP; i++)
		{
			//if(wps_getAP(i, bssid, ssid, &wep, &band))
			if(wps_api_get_ap(i, &apinf))
			{
				LPSTAP pAp = new STAP(apinf.ssid, apinf.bssid, apinf.wep);
				pWpsMgr->m_vecWpsAPs.push_back(pAp);
			}
		}
	}

	if(bSuccess)
	{
		// Notify searching AP success
		PostMessage(pWpsMgr->GetListenerWnd(), WM_WPS_STATUS, BRCM_WPSSTATE_SEARCH_AP, WPS_STATUS_SUCCESS);

		// Notify to show AP selection dialog
		g_hEvtWpsSelection = CreateEvent(NULL, FALSE, FALSE, WPS_SELECTION_EVENT);

		PostMessage(pWpsMgr->GetListenerWnd(), WM_WPS_STATUS, BRCM_WPSSTATE_WAITING_AP_SELECTION, 0);

		DWORD dwRet = WaitForSingleObject(g_hEvtWpsSelection, WPS_SELECTION_EVENT_TIMEOUT);
		if(dwRet == WAIT_TIMEOUT) 
		{
			bSuccess = false;
			goto WPS_END;  // Timeout and no AP is selected
		}

		// Link to selected AP
		LPSTAP pSelectedAP = pWpsMgr->GetSelectedWpsAP();
		if(pSelectedAP)
		{
			//if(bSuccess=wps_join(pSelectedAP->bssid, pSelectedAP->ssid, pSelectedAP->wep))
			if(bSuccess=wps_api_join(pSelectedAP->bssid, pSelectedAP->ssid, pSelectedAP->wep))
			{
				//bSuccess = wps_configureAP(pSelectedAP->bssid, szPinLocal, pWpsMgr->m_pNetworkCred);

				/* Inform link up */
				wps_api_set_linkup();
				
				/* Run WPS not async mode */
				if (wps_api_run(DEVICE_PASSWORD_ID(true, szPinLocal, pWpsMgr->m_pNetworkCred),
					pSelectedAP->bssid, pSelectedAP->ssid, pSelectedAP->wep,
					strlen(szPinLocal) ? szPinLocal : NULL, pWpsMgr->m_pNetworkCred, false) == true) {
					/* loop process data */
					process(pWpsMgr->GetListenerWnd(), pSelectedAP, pWpsMgr->GeResult());
				}
			}
		}
	}
	else
	{
		// Notify searching AP failure
		PostMessage(pWpsMgr->GetListenerWnd(), WM_WPS_STATUS, BRCM_WPSSTATE_SEARCH_AP, WPS_STATUS_ERROR);
	}

WPS_END:
	pWpsMgr->WpsCleanup();

	// This must be called after WPS process is done to do cleanup at lower level and restore 
	// network environment
	PostMessage(pWpsMgr->GetListenerWnd(), WM_WPS_STATUS, BRCM_WPSSTATE_IDLE, bSuccess? WPS_STATUS_SUCCESS : WPS_STATUS_ERROR);

	return 0;
}

void CWpsManager::StartMonitorWpsHWTrigger(CString& strAdapterGuid, HWND hWnd)
{
	// Initialize WPS process
	m_hWndListener = hWnd;
	m_strAdapterGuid = strAdapterGuid;

	
	// Create thread to start WPS process
	DWORD dwThreadId;
	m_hWpsHwTriggerMonThd = CreateThread(NULL, 0, WpsHWTriggerMonitorThreadProc, this, 0, &dwThreadId);
}

void CWpsManager::AbortWpsGpioPulling()
{
	wps_api_hwbutton_close();
}

DWORD WINAPI CWpsManager::WpsHWTriggerMonitorThreadProc(LPVOID lpParam)
{
	static bool bWpsHwBnPressed = false;

	CWpsManager *pWpsMgr = (CWpsManager *)lpParam;
	if(!pWpsMgr)
		return 0;

	// Start polling loop
	if(wps_api_hwbutton_open(pWpsMgr->m_strAdapterGuid.GetLength() == 0? NULL:CT2A(LPCTSTR(pWpsMgr->m_strAdapterGuid))))
	{
		while(1)
		{
			pWpsMgr->m_bMonitorHwButtonInProgress = TRUE;

			// Check the current HW WPS PIN is pressed already, ignore. We only trigger WPS process when the PIN state
			// is changed from 0 to 1 (unpressed -> pressed)
			if(!bWpsHwBnPressed)
			{
				bWpsHwBnPressed = wps_api_hwbutton_state();
				if(bWpsHwBnPressed)
					PostMessage(pWpsMgr->GetListenerWnd(), WM_WPS_HWTRIGGER_STATE_UPDATE, 1, 0);  
			}
			else
			{
				bWpsHwBnPressed = wps_api_hwbutton_state();  // Update WPS PIN state
			}
				
			Sleep(WPS_HWTRIGGER_POLL_INTERVAL);
		}
	}

	return 0;
}
