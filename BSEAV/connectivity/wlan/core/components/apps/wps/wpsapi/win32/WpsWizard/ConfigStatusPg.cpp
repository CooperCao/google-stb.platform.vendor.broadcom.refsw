/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: ConfigStatusPg.cpp,v 1.14 2009/02/17 18:22:20 Exp $
*/

// ConfigStatusPg.cpp : implementation file
//

#include "stdafx.h"
#include <ks.h>
#include "WpsWizard.h"
#include "ConfigStatusPg.h"
#include "SelectAPDlg.h"
#include "WpsWizardUtil.h"

#define COLOR_BKROUND_WHITE  RGB( 255, 255, 255 )
#define COLOR_SUCCESS		 RGB( 0, 128, 0 )
#define COLOR_FAILED		 RGB( 255, 0, 0 )
#define COLOR_NORMAL		 RGB( 0, 0, 0 )
#define COLOR_TXTINDICATOR_TRANSPARENT  RGB( 0, 0, 255 )
#define STATUS_IDLE_TEXT	_T( " " )

// Please make sure that WPS_MSG_STRS definition is consistent to that in sta_eap_sm.c file
// and message types defined in reg_prototlv.h file
static UINT WPS_MSG_STRS[] = {
	IDS_WPS_MSGTYPE_NONE,		// NONE"
	IDS_WPS_MSGTYPE_BEACON,		// (BEACON)"
	IDS_WPS_MSGTYPE_PROBE_REQ,	// (PROBE_REQ)"
	IDS_WPS_MSGTYPE_PROBE_RESP,	// (PROBE_RESP)"
	IDS_WPS_MSGTYPE_M1,			// (M1)"
	IDS_WPS_MSGTYPE_M2,			// (M2)"
	IDS_WPS_MSGTYPE_M2D,		// (M2D)"
	IDS_WPS_MSGTYPE_M3,			// (M3)"
	IDS_WPS_MSGTYPE_M4,			// (M4)"
	IDS_WPS_MSGTYPE_M5,			// (M5)"
	IDS_WPS_MSGTYPE_M6,			// (M6)"
	IDS_WPS_MSGTYPE_M7,			// (M7)"
	IDS_WPS_MSGTYPE_M8,			// (M8)"
	IDS_WPS_MSGTYPE_ACK,		// (ACK)"
	IDS_WPS_MSGTYPE_NACK,		// (NACK)"
	IDS_WPS_MSGTYPE_DONE,		// (DONE)"
	IDS_WPS_MSGTYPE_Identity,	// Identity"
	IDS_WPS_MSGTYPE_Start,		// (Start)"
	IDS_WPS_MSGTYPE_FAILURE,	// (FAILURE)"
	IDS_WPS_MSGTYPE_FRAG,		// (FRAGMENT)"
	IDS_WPS_MSGTYPE_FRAG_ACK,	// (FRAGMENT ACK)"
	IDS_EPAOL_STARTS,			// (EAPOL-START)"
};

//IMPLEMENT_DYNAMIC(CConfigStatusPg, CWizardPropPg)

CConfigStatusPg::CConfigStatusPg()
	: CWizardPropPg(CConfigStatusPg::IDD)
{
	m_eCurWpsState = BRCM_WPSSTATE_IDLE;
	m_nLogIndex = 0;
}

CConfigStatusPg::~CConfigStatusPg()
{
}

void CConfigStatusPg::DoDataExchange(CDataExchange* pDX)
{
	CWizardPropPg::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STATUS_LOG, m_StatusLog);

	DDX_Control(pDX, IDC_BMPINDICATOR_WPS_INITIALIZING, m_BmpIndInitializing);
	DDX_Control(pDX, IDC_BMPINDICATOR_SEARCHING_AP, m_BmpIndSearchingAP);
	DDX_Control(pDX, IDC_BMPINDICATOR_JOINING_AP, m_BmpIndJoiningAP);
	DDX_Control(pDX, IDC_BMPINDICATOR_NEGOTIATING_RETRIEVING, m_BmpIndNegoRetr);
	DDX_Control(pDX, IDC_BMPINDICATOR_CREATING_PROFILE, m_BmpIndCreatingProfile);
	DDX_Control(pDX, IDC_BMPINDICATOR_RESTORING_NETWORK_ENV, m_BmpIndRestoringEnv);

	DDX_Control(pDX, IDC_LABEL_INITIALIZING_WPS, m_LabelInitWps);
	DDX_Control(pDX, IDC_LABEL_SEARCHING_AP, m_LabelSearchAP);
	DDX_Control(pDX, IDC_LABEL_JOINING_AP, m_LabelJoinAP);
	DDX_Control(pDX, IDC_LABEL_NEGOTIATING_RETRIEVING, m_LabelNegoRetr);
	DDX_Control(pDX, IDC_LABEL_CREATING_PROFILE, m_LabelCreateProfile);
	DDX_Control(pDX, IDC_LABEL_RESTORING_NETWORK_ENV, m_LabelRestoreEnv);

	DDX_Control(pDX, IDC_TXTINDICATOR_WPS_INITIALIZING, m_LabelInitWpsRel);
	DDX_Control(pDX, IDC_TXTINDICATOR_SEARCHING_AP, m_LabelSearchAPRel);
	DDX_Control(pDX, IDC_TXTINDICATOR_JOINING_AP, m_LabelJoinAPRel);
	DDX_Control(pDX, IDC_TXTINDICATOR_NEGOTIATING_RETRIEVING, m_LabelNegoRetrRel);
	DDX_Control(pDX, IDC_TXTINDICATOR_CREATING_PROFILE, m_LabelCreateProfileRel);
	DDX_Control(pDX, IDC_TXTINDICATOR_RESTORING_NETWORK_ENV, m_LabelRestoreEnvRel);

	DDX_Control(pDX, IDC_LABEL_CONFIG_STATUS, m_staticConfStatus);
	DDX_Control(pDX, IDC_LABEL_RESULT, m_staticResult);
	DDX_Control(pDX, IDC_STATIC_PIN, m_staticPin);
	DDX_Control(pDX, IDC_LABEL_PIN, m_labelCurPin);
}


BEGIN_MESSAGE_MAP(CConfigStatusPg, CWizardPropPg)
	ON_MESSAGE(WM_WPS_STATUS, OnWpsStatusUpdate)
	ON_BN_CLICKED(IDC_CANCEL_WPS, &CConfigStatusPg::OnBnClickedCancelWps)
END_MESSAGE_MAP()


// CConfigStatusPg message handlers
BOOL CConfigStatusPg::OnInitDialog()
{
	CWizardPropPg::OnInitDialog();

	SetLabel(IDC_CANCEL_WPS, IDS_CANCEL_WPS);

	m_LabelInitWps.SetWPSWWindowText(IDS_WPSSTATE_INITIALIZING);
	m_LabelSearchAP.SetWPSWWindowText(IDS_WPSSTATE_SEARCHING_AP);
	m_LabelJoinAP.SetWPSWWindowText(IDS_WPSSTATE_JOINING_AP);
	m_LabelNegoRetr.SetWPSWWindowText(IDS_WPSSTATE_NEGOTIATING_RETRIEVING);
	m_LabelCreateProfile.SetWPSWWindowText(IDS_WPSSTATE_CREATING_PROFILE);
	m_LabelRestoreEnv.SetWPSWWindowText(IDS_WPSSTATE_RESTORING_NETWORK_ENV);
	m_staticConfStatus.SetWPSWWindowText(IDS_CONFIG_STATUS);
	m_staticResult.SetWPSWWindowText(IDS_RESULT);

	m_staticConfStatus.SetFont(_T("Times Roman"), 12);
	m_staticResult.SetFont(_T("Times Roman"), 12);

	m_StatusLog.SetHorizontalExtent(500);

	return TRUE;
}

void CConfigStatusPg::InitBmpIndicators()
{
	CStaticBitmap3States* BmpIndicators[] =  {
		&m_BmpIndInitializing,
		&m_BmpIndSearchingAP,
		&m_BmpIndJoiningAP,
		&m_BmpIndNegoRetr,
		&m_BmpIndCreatingProfile,
		&m_BmpIndRestoringEnv
	};

	for(int i=0; i<SIZEOF_ARRAY(BmpIndicators); i++)
	{
		BmpIndicators[i]->SetTranspartColor( COLOR_TXTINDICATOR_TRANSPARENT );
		BmpIndicators[i]->SetBkColor( COLOR_BKROUND_WHITE );
		BmpIndicators[i]->SetStateBitmapId( IDB_IDLE, IDB_SUCCESS, IDB_FAIL  );
		BmpIndicators[i]->SetXLeading( 0 ); 
		BmpIndicators[i]->SetYLeading( 0 );
		BmpIndicators[i]->SetState(IDLE_STATE);
	}
}

int CConfigStatusPg::StartWPS()
{
	// Initialize status result indicators
	m_LabelInitWpsRel.SetWPSWWindowText(_T(""));
	m_LabelSearchAPRel.SetWPSWWindowText(_T(""));
	m_LabelJoinAPRel.SetWPSWWindowText(_T(""));
	m_LabelNegoRetrRel.SetWPSWWindowText(_T(""));
	m_LabelCreateProfileRel.SetWPSWWindowText(_T(""));
	m_LabelRestoreEnvRel.SetWPSWWindowText(_T(""));

	// Reset status log windows
	m_nLogIndex = 0;
	m_StatusLog.ResetContent();

	// Initialize bmp status indicators
	InitBmpIndicators();

	// Set currrent PIN code
	if(m_WpsManager.GetWpsMode() != STA_ENR_JOIN_NW_PBC)
	{
		m_labelCurPin.SetWPSWWindowText(IDS_CURRENT_PIN);
		m_staticPin.SetWPSWWindowText(CString(m_WpsManager.GetPinCode()));
		m_labelCurPin.ShowWindow(SW_SHOW);
		m_staticPin.ShowWindow(SW_SHOW);
	}
	else
	{
		m_labelCurPin.ShowWindow(SW_HIDE);
		m_staticPin.ShowWindow(SW_HIDE);
	}

	// Start WPS process
	m_WpsManager.StartWps(m_WpsManager.GetWpsMode(), m_hWnd, m_bAutoConnect? true : false);

	return 0;
}

void CConfigStatusPg::StopWps()
{
	CWaitCursor wait;

	m_WpsManager.AbortWps();

	wait.Restore();
}

LRESULT CConfigStatusPg::OnWpsStatusUpdate(WPARAM wParam, LPARAM lParam)
{
	// Update status of text indicator
	UpdateTextIndicators((eBRCM_WPSEVENT)wParam, (int)lParam);

	// Update status of flashing LCD bitmap indicator
	UpdateBmpIndicators((eBRCM_WPSEVENT)wParam, (int)lParam);

	CString strText;

	// Adding status log information
	switch((eBRCM_WPSEVENT)wParam)
	{
	case BRCM_WPSEVENT_START_WPS:
		// Disable "Next" button, not allowing cancellation during initialization
		SetWizardButtons(0);
		AddStatusLog(IDS_LOG_START_WPS);
		break;
	case BRCM_WPSEVENT_INIT_FAIL:
		// Allow cancellation after initialization
		SetWizardButtons(PSWIZB_NEXT);
		AddStatusLog(IDS_LOG_WPS_INIT_FAIL);
		break;
	case BRCM_WPSEVENT_INIT_SUCCESS:
		// Allow cancellation after initialization
		SetWizardButtons(PSWIZB_NEXT);
		AddStatusLog(IDS_LOG_WPS_INIT_SUCCESS);
		break;
	case BRCM_WPSEVENT_NO_AP_FOUND:
		AddStatusLog(IDS_LOG_NO_WPSAP_FOUND);
		break;				// No AP is scanned
	case BRCM_WPSSTATE_WAITING_AP_SELECTION:
		{
		AddStatusLog(IDS_LOG_WAIT_FOR_AP_SEL);
		CSelectAPDlg dlg(NULL, &m_WpsManager);
		if(dlg.DoModal() == IDOK)
		{
			m_WpsManager.SetSelectedWpsAP(dlg.GetSelectedAP());
		}
		else
		{
			// Cancelled, so no AP selected
			m_WpsManager.SetSelectedWpsAP(NULL);  
		}

		break;		// Multiple AP found in PIN mode
		}
	case BRCM_WPSEVENT_MULTI_AP_OVERLAP:
		AddStatusLog(IDS_LOG_MULTI_WPSAP_TRY_AGAIN);
		break;		// Multiple AP found in PBC mode, this is overlap
	case BRCM_WPSEVENT_MULTI_UNCONFIG_AP:
		AddStatusLog(IDS_LOG_MULTI_UNCONFIG_WPSAP);
		break;
	case BRCM_WPSEVENT_JOIN_AP:
		if(m_WpsManager.GetSelectedWpsAP())
			strText.Format(IDS_LOG_AP_XXX_SELECTED, CString(m_WpsManager.GetSelectedWpsAP()->ssid));
		AddStatusLog(strText);
		break;
	case BRCM_WPSEVENT_JOIN_AP_SUCCESS:
		AddStatusLog(IDS_LOG_AP_ASSOCIATION_SUCCESS);
		break;
	case BRCM_WPSEVENT_JOIN_AP_FAIL:
		AddStatusLog(IDS_LOG_AP_ASSOCIATION_FAIL);
		break;
	case BRCM_WPSEVENT_WPS_NEGO_SUCCESS:
		AddStatusLog(IDS_LOG_WPS_NEGO_SUCCESS);
		break;
	case BRCM_WPSEVENT_WPS_NEGO_FAIL:
		AddStatusLog(IDS_LOG_WPS_NEGO_FAIL);
		break;
	case BRCM_WPSEVENT_CREATE_PROFILE_END:
		if(lParam == WPS_STATUS_SUCCESS)
			AddStatusLog(IDS_LOG_CREATE_PROFILE_SUCCESS);
		else
			AddStatusLog(IDS_LOG_CREATE_PROFILE_FAIL);
		break;
	case BRCM_WPSEVENT_RESTORE_WIFI_ENV_START:
		// Disable "Next" button while network environment is being restored
		SetWizardButtons(0);
		break;
	case BRCM_WPSEVENT_RESTORE_WIFI_ENV_END:
		// Enable "Next" button after network env. is restored
		SetWizardButtons(PSWIZB_NEXT);
		if(lParam == WPS_STATUS_SUCCESS)
			AddStatusLog(IDS_LOG_RESTORE_WIFIENV_SUCCESS);
		else
			AddStatusLog(IDS_LOG_RESTORE_WIFIENV_FAIL);
		break;
	case BRCM_WPSEVENT_CB_STATUS_UPDATE:
		// WPS protocol callback status update
		switch(WPS_CB_STATUS(lParam))
		{
		case WPS_STATUS_SENDING_WPS_MESSAGE:
			strText.LoadString(IDS_SEND_WPS_MSGTYPE);
			strText += CString(GetWpsMsgTypeString(WPS_CB_INFO(lParam)));
			AddStatusLog(strText);
			break;
		case WPS_STATUS_GOT_WPS_RESPONSE:
			strText.LoadString(IDS_REC_WPS_MSGTYPE);
			strText += CString(GetWpsMsgTypeString(WPS_CB_INFO(lParam)));
			AddStatusLog(strText);
			break;
		default:
			;
		}
		break;
	default:
		;
	}

	return 0;
}

CString CConfigStatusPg::GetWpsMsgTypeString(int nMsgType)
{
	CString strText;
	strText.LoadString(WPS_MSG_STRS[0]);
	if (nMsgType >= 0 && nMsgType < (ARRAYSIZE(WPS_MSG_STRS)))
		strText.LoadString(WPS_MSG_STRS[nMsgType]);

	TUTRACE((TUTRACE_INFO, "Message Type %d\n", nMsgType));
	return strText;
}

eBRCM_WPSSTATE CConfigStatusPg::GetNextState(eBRCM_WPSSTATE eCurrent, eBRCM_WPSEVENT eEvent)
{
	eBRCM_WPSSTATE eNext = BRCM_WPSSTATE_INFO;
	
	if (eEvent == BRCM_WPSEVENT_ABORT_WPS)
	{
		// WPS is aborted, notify to restore network env.
		return BRCM_WPSSTATE_RESTORE_ENV;
	}

	BOOL bSuccess = FALSE;
	for(int i=0; i<SIZEOF_ARRAY(g_mapStateTransition) && !bSuccess; i++)
	{
		if(eEvent == g_mapStateTransition[i].eEvent && eCurrent == g_mapStateTransition[i].eCurrentState)
		{
			bSuccess = TRUE;
			eNext = g_mapStateTransition[i].eNextState;
		}
	}
	return eNext;
}

void CConfigStatusPg::UpdateBmpIndicators(eBRCM_WPSEVENT eWpsEvent, int nError)
{
	// Map each state/wps state to one of the bmp indicator
	static const struct _MAP_STATE_BMPINDICATOR {
		eBRCM_WPSSTATE eState;
		CStaticBitmap3States *pBmpIndicator;
	} mapStateIndicator[] = {
		{BRCM_WPSSTATE_INITIALIZING,	&m_BmpIndInitializing},
		{BRCM_WPSSTATE_SEARCHING_AP,	&m_BmpIndSearchingAP},
		{BRCM_WPSSTATE_JOINING_AP,		&m_BmpIndJoiningAP},
		{BRCM_WPSSTATE_NEGO_RETR,		&m_BmpIndNegoRetr},
		{BRCM_WPSSTATE_CREATE_PROFILE,	&m_BmpIndCreatingProfile},
		{BRCM_WPSSTATE_RESTORE_ENV,		&m_BmpIndRestoringEnv}
	};

	// Get next state based on current state and coming event
	eBRCM_WPSSTATE eNext = GetNextState(m_eCurWpsState, eWpsEvent);

	CString strDebug;
	strDebug.Format(_T("\nCurrent state is %d, Incoming event %d, Next state %d\n"), m_eCurWpsState, eWpsEvent, eNext);
	OutputDebugString(strDebug);


	// Check if WPS state is changed and ignore BRCM_WPSSTATE_INFO state. BRCM_WPSSTATE_INFO state is generated
	// due to the incoming event does not trigger state transition and is informative only
	if(m_eCurWpsState != eNext && eNext != BRCM_WPSSTATE_INFO)
	{
		// Update indicators
		for(int j=0; j<SIZEOF_ARRAY(mapStateIndicator); j++)
		{
			// Stop old indicator blinking and set it to "SUCCESS" or "FAIL"
			if(m_eCurWpsState == mapStateIndicator[j].eState)
			{
				eIndicattorState stateIndicator = (nError == WPS_STATUS_SUCCESS)? SUCCESS_STATE : FAIL_STATE;
				mapStateIndicator[j].pBmpIndicator->StopBlinking();
				mapStateIndicator[j].pBmpIndicator->SetState(stateIndicator);
			}

			// Start new indicator blinking
			if(eNext == mapStateIndicator[j].eState)
				mapStateIndicator[j].pBmpIndicator->StartBlinking();
		}
		m_eCurWpsState = eNext;
	}
}

void CConfigStatusPg::OnBnClickedCancelWps()
{
	StopWps();
}

void CConfigStatusPg::AddStatusLog(UINT uStrID)
{
	CString strText;
	if(strText.LoadString(uStrID))
		AddStatusLog(strText);
}

void CConfigStatusPg::AddStatusLog(const CString& strLog)
{
	// Insert new status information
	m_StatusLog.InsertString(m_nLogIndex++, strLog);

	// This is a workround by force-showing both scrollbars all the time. Otherwise, when status messages
	// are added to the list box quickly (quick call back messages arrive), the list control will lose the border
	// at the moment when a vertical or horizontal scrollbar shows up automatically
	m_StatusLog.ShowScrollBar(SB_BOTH);

	// Set focus to the latest log
	int nCount = m_StatusLog.GetCount();
	if(nCount > 0)
		m_StatusLog.SetCurSel(nCount-1);
}

void CConfigStatusPg::UpdateTextIndicators(eBRCM_WPSEVENT eWpsEvent, int nError)
{
	// We should only update text indicator when the event tells the end of current stage with 
	// either SUCCESS or FAILURE
	const static struct ST_WPSEVENTTXTINDICATOR
	{
		eBRCM_WPSEVENT eEvent;
		CWPSWStatic* pTxtIndicator;
	} mapStateTxtIndicator[] = {
		// Update "Initializing WPS process" text indicator 
		{BRCM_WPSEVENT_INIT_SUCCESS,			&m_LabelInitWpsRel},
		{BRCM_WPSEVENT_INIT_FAIL,				&m_LabelInitWpsRel},

		// Update "Searching for WPS Access Point" text indicator 
		{BRCM_WPSEVENT_NO_AP_FOUND,				&m_LabelSearchAPRel},
		{BRCM_WPSEVENT_AP_FOUND,				&m_LabelSearchAPRel},
		{BRCM_WPSEVENT_JOIN_AP,					&m_LabelSearchAPRel},

		// Update "Connecting to Access Point" text indicator 
		{BRCM_WPSEVENT_NO_AP_SELECTED,			&m_LabelJoinAPRel},
		{BRCM_WPSEVENT_MULTI_AP_OVERLAP,		&m_LabelJoinAPRel},
		{BRCM_WPSEVENT_MULTI_UNCONFIG_AP,		&m_LabelJoinAPRel},
		{BRCM_WPSEVENT_JOIN_AP_SUCCESS,			&m_LabelJoinAPRel},
		{BRCM_WPSEVENT_JOIN_AP_FAIL,			&m_LabelJoinAPRel},

		// Update "Negotiating security" text indicator 
		{BRCM_WPSEVENT_WPS_NEGO_SUCCESS,		&m_LabelNegoRetrRel},
		{BRCM_WPSEVENT_WPS_NEGO_FAIL,			&m_LabelNegoRetrRel},
		
		// Update "Creating network profile" text indicator 
		{BRCM_WPSEVENT_CREATE_PROFILE_END,		&m_LabelCreateProfileRel},
		
		// Update "Restoring network environment" text indicator 
		{BRCM_WPSEVENT_RESTORE_WIFI_ENV_END,	&m_LabelRestoreEnvRel}
	};
	
	BOOL bSuccess = FALSE;
	CWPSWStatic* pIndicator = NULL;
	for(int i=0; i<SIZEOF_ARRAY(mapStateTxtIndicator) && !bSuccess; i++)
	{
		pIndicator = NULL;
		if(bSuccess = (eWpsEvent == mapStateTxtIndicator[i].eEvent))
			pIndicator = mapStateTxtIndicator[i].pTxtIndicator;
	}

	if(bSuccess)
	{
		// Update text result indicators to "Success" or "Error" based on error code
		CString strText;
		strText.LoadString(nError == WPS_STATUS_SUCCESS? IDS_SUCCESS : IDS_FAILURE);
		if(pIndicator)
			pIndicator->SetWPSWWindowText(strText);
	}
}

BOOL CConfigStatusPg::OnSetActive()
{
	if(m_pWizardSheet->m_bToRunWps)
	{
		StartWPS();  // Start WPS Process
		m_pWizardSheet->m_bToRunWps = FALSE;
	}

	SetWizardButtons(PSWIZB_NEXT);

	// Disable "Cancel" button. Clicking "Next" when WPS is in process will cancel WPS process
	m_pWizardSheet->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);

	return CWizardPropPg::OnSetActive();
}


LRESULT CConfigStatusPg::OnWizardNext()
{
	if(m_WpsManager.GetWpsInProgress())
	{
		CString strText;
		strText.LoadString(IDS_WARNING_CANCEL_WPS);
		if(MessageBox(strText, NULL, MB_OKCANCEL | MB_ICONWARNING) == IDOK)
		{
			// Notify that WPS will be aborted
			m_WpsManager.UpdateStatus(BRCM_WPSEVENT_ABORT_WPS, BRCM_WPS_ERROR_GENERIC);
			StopWps();
		}

		return -1;  // Go back to this page after cancellation
	}
	
	m_pNextPage = &(m_pWizardSheet->m_WpsFinishPg);

	return CWizardPropPg::OnWizardNext(this);
}
