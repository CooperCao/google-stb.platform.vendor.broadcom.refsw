/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: WpsManager.cpp,v 1.11 2009/10/17 00:11:03 Exp $
 */

#include "StdAfx.h"
#include "WpsManager.h"

#define WPS_SELECTION_EVENT				_T("BRCM_WPS_SELECTION_EVENT")
#define WPS_CANCELLING_EVENT			_T("BRCM_WPS_CANCELLING_EVENT")
#define WPS_SELECTION_EVENT_TIMEOUT		(120*1000)  // Milliseconds
#define WPS_CANCELLING_EVENT_TIMEOUT	(10*1000)  // Milliseconds
#define WPS_HWTRIGGER_POLL_INTERVAL		200  // Milliseconds

static HANDLE g_hEvtWpsSelection = NULL;
static HANDLE g_hEvtWpsCancelling = NULL;
static bool g_bContinue = true;




#define DEVICE_PASSWORD_ID(appin, pin, new_cred) \
	(((appin) && (new_cred)) ? STA_REG_CONFIG_NW : \
	(appin) ? STA_REG_JOIN_NW : \
	(pin) == NULL ? STA_ENR_JOIN_NW_PBC : \
	strlen((pin)) ? STA_ENR_JOIN_NW_PIN : STA_ENR_JOIN_NW_PBC)

#define PIN_MODE(emode)	(((emode) == STA_REG_JOIN_NW) || \
	((emode) == STA_REG_CONFIG_NW) || \
	((emode) == STA_ENR_JOIN_NW_PIN))
#define PBC_MODE(emode)	((emode) == STA_ENR_JOIN_NW_PBC)

#define GET_EVENT_SUCCESS	1
#define GET_EVENT_ERROR		2
#define GET_EVENT_IDLE		3

#define EVENT_TYPE_WPS		1

CWpsManager::CWpsManager(void)
{
	m_hWndListener = NULL;
	m_hWpsThread = NULL;
	m_bWpsOpened = false;
	m_eWpsMode = STA_ENR_JOIN_NW_PIN;
	m_fpWpsStatusCb  = NULL;
	m_pSelectedAP = NULL;
	m_nTotalFoundAP = 0;
	m_pNetworkCred = NULL;
	m_bAutoConnect = false;
	m_bWpsSuccess = false;
	m_bWpsInProcess = FALSE;
	m_bWpsV2 = true;
	m_bRequestAbort = false;
	memcpy(m_DevMacAddr, ZERO_MAC, SIZE_MAC_ADDR);
}

CWpsManager::~CWpsManager(void)
{
	if(m_pNetworkCred)
	{
		delete m_pNetworkCred;
		m_pNetworkCred = NULL;
	}
}

bool CWpsManager::StartWps(eWPS_MODE eWpsMode, HWND hWnd, bool bAutoConnect)
{
	m_bAutoConnect = bAutoConnect;
	m_eWpsMode = eWpsMode;
	m_hWndListener = hWnd;
	m_nTotalFoundAP = 0;  // reset to 0

	m_bWpsSuccess = false;
	m_bWpsInProcess = TRUE;  // WPS is in process now

	// Create thread to start WPS process
	DWORD dwThreadId;
	m_hWpsThread = CreateThread(NULL, 0, WpsJoinNWThreadProc, this, 0, &dwThreadId);

	return true;
}

BOOL CWpsManager::DoesAPAuthorize(const LPSTAP pAP)
{
	BOOL bRet = FALSE;

	wps_api_get_dev_mac(m_DevMacAddr, sizeof(m_DevMacAddr));

	for (int i = 0; i < sizeof(pAP->authorizedMACs) && !bRet; )
	{
		if (memcmp(m_DevMacAddr, &(pAP->authorizedMACs[i]), SIZE_MAC_ADDR) == 0)
			bRet = TRUE;

		i += SIZE_MAC_ADDR;
	}

	return bRet;
}

VEC_AP& CWpsManager::GetWpsAPs()
{
	return m_vecWpsAPs;
}


bool CWpsManager::WpsCleanup()
{
	bool bRet = wps_api_close();

	ClearNetworkList();

	m_pSelectedAP = NULL;

	m_bWpsInProcess = FALSE;

	return bRet;
}

void CWpsManager::ClearNetworkList()
{
	for(int i=0; i < (int)m_vecWpsAPs.size(); i++)
	{
		if(m_vecWpsAPs[i])
			delete m_vecWpsAPs[i];
	}

	m_vecWpsAPs.clear();
}

void CWpsManager::AbortWps(BOOL bImmediate)
{
	// Set cancellation flag returned by the callback function
	g_bContinue = FALSE;

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
					
					// Timeout so we need to restore network env. and update indicators 
					UpdateStatus(BRCM_WPSEVENT_RESTORE_WIFI_ENV_START, BRCM_WPS_SUCCESS);
					bool bSuccess = WpsCleanup();
					UpdateStatus(BRCM_WPSEVENT_RESTORE_WIFI_ENV_END, bSuccess? BRCM_WPS_SUCCESS : BRCM_WPS_ERROR_GENERIC);
				}
			}
		}
	}
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

void CWpsManager::SetPinCode(LPCTSTR lpcszPin)
{
	USES_CONVERSION;

	if(lpcszPin)
		strcpy(m_szWpsPin, CT2A(lpcszPin));
}

bool CWpsManager::_wps_join_callback(void *context, unsigned int uiStatus, void *data)
{
	CWpsManager *pWpsMgr = (CWpsManager *)context;

	if(!pWpsMgr)
		return false;  // Unexpected

	// Check if WPS Canceling is done by lower API
	if(uiStatus == WPS_STATUS_CANCELED)
	{
		if(g_hEvtWpsCancelling)
			SetEvent(g_hEvtWpsCancelling);
	}

	int nMsgType = 0;
	switch(uiStatus)
	{
	case WPS_STATUS_STARTING_WPS_EXCHANGE:
		pWpsMgr->UpdateStatus(BRCM_WPSEVENT_CB_STATUS_UPDATE, (DWORD)WPS_CB_MSG(uiStatus, nMsgType));
		break;
	case WPS_STATUS_SENDING_WPS_MESSAGE:
		if(data)
		{
			memcpy(&nMsgType, data, sizeof(int));
			pWpsMgr->UpdateStatus(BRCM_WPSEVENT_CB_STATUS_UPDATE, (DWORD)WPS_CB_MSG(uiStatus, nMsgType));
		}
		break;
	case WPS_STATUS_WAITING_WPS_RESPONSE:
	case WPS_STATUS_GOT_WPS_RESPONSE:
		if(data)
		{
			memcpy(&nMsgType, data, sizeof(int));
			pWpsMgr->UpdateStatus(BRCM_WPSEVENT_CB_STATUS_UPDATE, (DWORD)WPS_CB_MSG(uiStatus, nMsgType));
		}
		break;
	default:
		;  // Ignore all other callback status messages for now
	}

	return g_bContinue;
}

void CWpsManager::UpdateStatus(eBRCM_WPSEVENT nWpsEvent, DWORD dwError)
{
	if(m_hWndListener)
		PostMessage(m_hWndListener, WM_WPS_STATUS, nWpsEvent, dwError);
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

	UpdateStatus(BRCM_WPSEVENT_WPS_NEGO_SUCCESS, BRCM_WPS_SUCCESS);

	if (wps_api_get_credentials(&credentials) == NULL) {
		UpdateStatus(BRCM_WPSEVENT_CREATE_PROFILE_END, WPS_STATUS_ERROR);
		return PROCESS_RESULT_ERROR;
	}

	if (wps_api_create_profile(&credentials) == false) {
		UpdateStatus(BRCM_WPSEVENT_CREATE_PROFILE_END, WPS_STATUS_ERROR);
		return PROCESS_RESULT_ERROR;
	}
	else {
		SetWpsCredential(credentials);
		UpdateStatus(BRCM_WPSEVENT_CREATE_PROFILE_END, BRCM_WPS_SUCCESS);
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

	if (event.type != EVENT_TYPE_WPS)
		return retVal;

	/* Now we only process WPS event */
	retVal = wps_api_process_data(event.buf, event.buf_len);
	if (retVal == WPS_STATUS_SUCCESS)
		return process_success(listener);
	else if (retVal == WPS_STATUS_REJOIN) {
		UpdateStatus(BRCM_WPSEVENT_JOIN_AP, WPS_STATUS_SUCCESS);
		if (wps_api_join(pSelectedAP->bssid, pSelectedAP->ssid, pSelectedAP->wep) == false) {
			UpdateStatus(BRCM_WPSEVENT_JOIN_AP_FAIL, WPS_STATUS_ERROR);
			return PROCESS_RESULT_ERROR;
		}

		UpdateStatus(BRCM_WPSEVENT_JOIN_AP_FAIL, WPS_STATUS_SUCCESS);

		/* Tell wps_api link up */
		wps_api_set_linkup();

		return PROCESS_RESULT_REJOIN;
	}
	else if (retVal == WPS_STATUS_ERROR) {
		UpdateStatus(BRCM_WPSEVENT_WPS_NEGO_FAIL, WPS_STATUS_ERROR);
		return PROCESS_RESULT_ERROR;
	}

	return PROCESS_RESULT_IDLE;
}

int CWpsManager::process_timeout(HWND listener, LPSTAP pSelectedAP)
{
	int retVal;

	/* Now we only process WPS timeout */
	retVal = wps_api_process_timeout();
	if (retVal == WPS_STATUS_REJOIN) {
		UpdateStatus(BRCM_WPSEVENT_JOIN_AP, BRCM_WPS_SUCCESS);
		if (wps_api_join(pSelectedAP->bssid, pSelectedAP->ssid, pSelectedAP->wep) == false) {
			/* Connecting Failed */
			UpdateStatus(BRCM_WPSEVENT_JOIN_AP, BRCM_WPS_SUCCESS);
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

int CWpsManager::process(HWND listener, LPSTAP pSelectedAP)
{
	int nResult = PROCESS_RESULT_ERROR;

	/* Not Async mode */
	while (1) {
		/* Event process */
		nResult = process_event(listener, pSelectedAP);
		switch (nResult) {
		case PROCESS_RESULT_SUCCESS:
		case PROCESS_RESULT_CANCELED:
		case PROCESS_RESULT_ERROR:
			goto END;

		default:
			break;
		}

		/* Timeout process */
		nResult = process_timeout(listener, pSelectedAP);
		switch (nResult) {
		case PROCESS_RESULT_SUCCESS:
		case PROCESS_RESULT_ERROR:
		case PROCESS_RESULT_CANCELED:
			return nResult;

		default:
			break;
		}
	}

END:
	return nResult;
}

int CWpsManager::SearchWpsAPs(int nTimeout, bool bPbcMode, bool bAutoConnect, bool bAuthorizedAPOnly)
{
	int nAP = 0;
	time_t lstart, lcurrent;
	time(&lstart);
	static bool bWpsIEAdded = false;
	bool bRel;

	ClearNetworkList();
	while((lstart+nTimeout)>(lcurrent=time(&lcurrent)) && m_vecWpsAPs.size()==0 && g_bContinue)
	{
		/* Get all APs */
		wps_ap_list_info *aplist = wps_api_surveying(bPbcMode, m_bWpsV2, !bWpsIEAdded);
		if(!bWpsIEAdded)
			bWpsIEAdded = TRUE;  // WPS IE added now

		if(aplist != NULL)
		{
			uint8 mac[6];
			uint8* host_mac = NULL;
			bool bPbcAPFound;
			
			if(bAuthorizedAPOnly)
			{
				if(!wps_api_get_dev_mac(mac, sizeof(mac)))
					break;  // System error
				else
					host_mac = mac;
			}
				
			if(bPbcMode)
			{
				// PBC mode. Ignore activated APs running PIN mode by setting b_auto to FALSE 
				bRel = wps_api_find_ap(aplist, &nAP, bPbcMode, host_mac, true, &bPbcAPFound, FALSE);
			}
			else
			{
				// PIN mode.
				bRel = wps_api_find_ap(aplist, &nAP, bPbcMode, host_mac,  bAuthorizedAPOnly, &bPbcAPFound, bAutoConnect);
			}

			if(!bRel)
				continue;  // No AP found, go back and survey again

			for(int i=0; i<nAP; i++)
			{
				wps_apinf apinf;
				wps_api_get_ap(i, &apinf);

				LPSTAP pAp = new STAP(&apinf);
				m_vecWpsAPs.push_back(pAp);
/*
				if(GetWpsMode() == STA_REG_CONFIG_NW)
				{
					// Handle the use case of ER configuring AP/Network. We will add all "Unconfigured"
					// WPS AP to the list
					if(!apinf.configured)
					{
						LPSTAP pAp = new STAP(&apinf);
						m_vecWpsAPs.push_back(pAp);
					}
				}
				else
				{
					LPSTAP pAp = new STAP(&apinf);
					m_vecWpsAPs.push_back(pAp);
				}
*/
			}
		}
	}
	
	return (int)m_vecWpsAPs.size();
}

DWORD WINAPI CWpsManager::WpsJoinNWThreadProc(LPVOID lpParam)
{
	// Pass in CWpsManager pointer lpParam 
	CWpsManager *pWpsMgr = (CWpsManager *)lpParam;
	bool bUseAPPin = false;

	if(!pWpsMgr)
		return 0;

	// Reset g_bContinue flag
	g_bContinue = TRUE;

	pWpsMgr->UpdateStatus(BRCM_WPSEVENT_START_WPS, BRCM_WPS_SUCCESS);

	if (pWpsMgr->GetWpsMode() == STA_REG_CONFIG_NW || pWpsMgr->GetWpsMode() == STA_REG_JOIN_NW)
		bUseAPPin = true;

	// Initialize WPS process
	if(!wps_api_open(NULL, 
					lpParam, 
					(fnWpsProcessCB)_wps_join_callback,
					NULL, 
					bUseAPPin,  // Use AP or enrollee PIN
					pWpsMgr->m_bWpsV2))
	{
		pWpsMgr->UpdateStatus(BRCM_WPSEVENT_INIT_FAIL, BRCM_WPS_ERROR_GENERIC);
		pWpsMgr->WpsCleanup();
		return 0;
	}

	pWpsMgr->UpdateStatus(BRCM_WPSEVENT_INIT_SUCCESS, BRCM_WPS_SUCCESS);

	bool bSuccess = false;
	int nAP = 0;
	char ssid[32] = { 0 };
	uint8 wep = 1;
	uint16 band = 0;
	char szPinLocal[MAX_PIN_LEN+1] = { 0 };

	// Search WPS APs. If m_bAutoConnect, only include APs controlled by activated Registrar
	pWpsMgr->m_nTotalFoundAP = pWpsMgr->SearchWpsAPs(WPS_AP_SERCH_TIMEOUT,
													 PBC_MODE(pWpsMgr->GetWpsMode()), 
													 pWpsMgr->m_bAutoConnect,
													 false);
	if(pWpsMgr->m_nTotalFoundAP == 0)
	{
		// Notify searching AP failure
		pWpsMgr->UpdateStatus(BRCM_WPSEVENT_NO_AP_FOUND, BRCM_WPS_ERROR_GENERIC);
		goto WPS_END;
	}

	pWpsMgr->UpdateStatus(BRCM_WPSEVENT_AP_FOUND, BRCM_WPS_SUCCESS);

	if(pWpsMgr->m_bAutoConnect)
	{
		// Automatically connect to WPS AP
		if(pWpsMgr->m_nTotalFoundAP > 1)
		{
			if(pWpsMgr->GetWpsMode() == STA_REG_CONFIG_NW)
				pWpsMgr->UpdateStatus(BRCM_WPSEVENT_MULTI_UNCONFIG_AP, BRCM_WPS_ERROR_GENERIC);
			else
				pWpsMgr->UpdateStatus(BRCM_WPSEVENT_MULTI_AP_OVERLAP, BRCM_WPS_ERROR_GENERIC);
			goto WPS_END;
		}
		else
		{
			// One activated AP in the list only, set it as selected and continue
			pWpsMgr->SetSelectedWpsAP((LPSTAP)pWpsMgr->m_vecWpsAPs[0]);
		}
	}
	else
	{
		// still consider this as session overlap and won't join AP network
		if(pWpsMgr->m_nTotalFoundAP > 1 && pWpsMgr->GetWpsMode() == STA_ENR_JOIN_NW_PBC)
		{
			pWpsMgr->UpdateStatus(BRCM_WPSEVENT_MULTI_AP_OVERLAP, BRCM_WPS_ERROR_GENERIC);
			goto WPS_END;
		}

		// More than one AP is found, notify to show AP selection dialog
		g_hEvtWpsSelection = CreateEvent(NULL, FALSE, FALSE, WPS_SELECTION_EVENT);

		pWpsMgr->UpdateStatus(BRCM_WPSSTATE_WAITING_AP_SELECTION, BRCM_WPS_SUCCESS);

		WaitForSingleObject(g_hEvtWpsSelection, INFINITE);  // Wait for user to select AP to continue
	}

	// Link to selected AP
	LPSTAP pSelectedAP = pWpsMgr->GetSelectedWpsAP();
	if(pSelectedAP)
	{
		// Set PIN code to local variable
		if(pWpsMgr->GetPinCode())
			strcpy(szPinLocal, pWpsMgr->GetPinCode());

		pWpsMgr->UpdateStatus(BRCM_WPSEVENT_JOIN_AP, BRCM_WPS_SUCCESS);
		if(bSuccess = wps_api_join(pSelectedAP->bssid, pSelectedAP->ssid, pSelectedAP->wep))
		{
			pWpsMgr->UpdateStatus(BRCM_WPSEVENT_JOIN_AP_SUCCESS, BRCM_WPS_SUCCESS);

			// Inform link up
			wps_api_set_linkup();
			
			// Run WPS in sync mode */
			if (wps_api_run(pWpsMgr->GetWpsMode(),
							pSelectedAP->bssid, 
							pSelectedAP->ssid, 
							pSelectedAP->wep,
							strlen(szPinLocal) ? szPinLocal : NULL,
							pWpsMgr->GetWpsCred(), 
							false)) 
			{
				int nResult;

				/* loop process data */
				nResult = pWpsMgr->process(pWpsMgr->GetListenerWnd(), pSelectedAP);

				// WPS is done
				if (nResult ==  PROCESS_RESULT_SUCCESS) {
					pWpsMgr->m_bWpsSuccess = true;
					pWpsMgr->UpdateStatus(BRCM_WPSEVENT_WPS_NEGO_SUCCESS, BRCM_WPS_SUCCESS);
			}
				else {
					pWpsMgr->m_bWpsSuccess = false;
					pWpsMgr->UpdateStatus(BRCM_WPSEVENT_WPS_NEGO_FAIL, BRCM_WPS_ERROR_GENERIC);
				}
			}
		}
		else
		{
			pWpsMgr->UpdateStatus(BRCM_WPSEVENT_JOIN_AP_FAIL, BRCM_WPS_ERROR_GENERIC);
		}
	}
	else
	{
		// User "Cancel" from Select AP dialog
		pWpsMgr->UpdateStatus(BRCM_WPSEVENT_NO_AP_SELECTED, BRCM_WPS_ERROR_GENERIC);
	}

WPS_END:
	pWpsMgr->UpdateStatus(BRCM_WPSEVENT_RESTORE_WIFI_ENV_START, BRCM_WPS_SUCCESS);

	bSuccess = pWpsMgr->WpsCleanup();

	pWpsMgr->UpdateStatus(BRCM_WPSEVENT_RESTORE_WIFI_ENV_END, bSuccess? BRCM_WPS_SUCCESS : BRCM_WPS_ERROR_GENERIC);

	return 0;
}

void CWpsManager::SetWpsCredential(const wps_credentials& cred)
{
	if(m_pNetworkCred)
	{
		delete m_pNetworkCred;
		m_pNetworkCred = NULL;
	}

	m_pNetworkCred = new wps_credentials();
	strcpy(m_pNetworkCred->ssid, cred.ssid);
	strcpy(m_pNetworkCred->keyMgmt, cred.keyMgmt);
	strcpy(m_pNetworkCred->nwKey, cred.nwKey);
	m_pNetworkCred->encrType = cred.encrType;
}
