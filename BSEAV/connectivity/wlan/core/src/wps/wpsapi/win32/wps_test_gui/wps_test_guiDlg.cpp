/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wps_test_guiDlg.cpp,v 1.17 2009/10/17 00:11:04 Exp $
 */

// wps_test_guiDlg.cpp : implementation file
//

#include "stdafx.h"
#include "wps_test_gui.h"
#include "wps_test_guiDlg.h"
#include ".\wps_test_guidlg.h"
#include "WpsManager.h"
#include "SelectAPDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BCM_WPS_MSGBOX_TITLE	_T("WPS Test Utility")
#define DEFAULT_WPS_PIN			_T("12345670")

static long index=0;
extern HWND g_hWnd;

Cwps_test_guiDlg::Cwps_test_guiDlg(CWnd* pParent /*=NULL*/)
	: CDialog(Cwps_test_guiDlg::IDD, pParent)
	, m_bIsWpsRunning(FALSE)
	, m_strWpsPin(DEFAULT_WPS_PIN)
	, m_strAPPin(DEFAULT_WPS_PIN)
	, m_strNetworkKey(_T("password"))
	, m_strSsid(_T("BroadcomWpsAP"))
	, m_eGenerateCredMode(BRCM_GENERATECRED_RANDOM)
	, m_strAdapterGuid(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cwps_test_guiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_Status);
	DDX_Control(pDX, IDC_ENROLLEE_JOIN_NW, m_WpsJoinNetwork);
	DDX_Control(pDX, IDC_EXIT, m_Exit);
	DDX_Radio(pDX, IDC_GENERATE_CRED_MANUAL, (int&)m_eGenerateCredMode);
	DDX_Text(pDX, IDC_PIN, m_strWpsPin);
	DDX_Text(pDX, IDC_AP_PIN, m_strAPPin);
	DDV_MaxChars(pDX, m_strAPPin, 8);
	DDX_Control(pDX, IDC_SECURITY_METHOD, m_SecurityMethod);
	DDX_Control(pDX, IDC_ENCR_TYPE, m_EncrType);
	DDX_Text(pDX, IDC_NW_KEY, m_strNetworkKey);
	DDX_Text(pDX, IDC_SSID, m_strSsid);
	DDX_Control(pDX, IDC_NW_KEY, m_NetworkKey);
	DDX_Control(pDX, IDC_SSID, m_Ssid);
	DDX_Text(pDX, IDC_ADAPTER_GUID, m_strAdapterGuid);
	DDV_MaxChars(pDX, m_strAdapterGuid, 39);
}

BEGIN_MESSAGE_MAP(Cwps_test_guiDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	
	ON_BN_CLICKED(IDC_ENROLLEE_JOIN_NW, &Cwps_test_guiDlg::OnBnClickedEnrolleeJoinNw)
	ON_BN_CLICKED(IDC_STAER_JOIN_NW, &Cwps_test_guiDlg::OnBnClickedStaErJoinNw)
	ON_BN_CLICKED(IDC_EXIT, &Cwps_test_guiDlg::OnBnClickedExit)
	ON_BN_CLICKED(IDC_CLEAR_LOG, &Cwps_test_guiDlg::OnBnClickedClearLog)

	ON_MESSAGE(WM_WPS_STATUS, OnWpsStatusUpdate)
	ON_MESSAGE(WM_WPS_HWTRIGGER_STATE_UPDATE, OnWpsHWTriggerStateUpdate)
	
	ON_BN_CLICKED(IDC_SEL_MODE_PIN, &Cwps_test_guiDlg::OnBnClickedSelModePin)
	ON_BN_CLICKED(IDC_SEL_MODE_PBC, &Cwps_test_guiDlg::OnBnClickedSelModePbc)
	ON_BN_CLICKED(IDC_CONFIGURE_AP, &Cwps_test_guiDlg::OnBnClickedConfigureAp)
	ON_BN_CLICKED(IDC_GENERATE_CRED_RANDOM, &Cwps_test_guiDlg::OnBnClickedGenerateCredRandom)
	ON_BN_CLICKED(IDC_GENERATE_CRED_MANUAL, &Cwps_test_guiDlg::OnBnClickedGenerateCredManual)
	ON_BN_CLICKED(IDC_CANCEL_WPS, &Cwps_test_guiDlg::OnBnClickedCancelWps)
	ON_BN_CLICKED(IDC_GENERATE_WPS_PIN, &Cwps_test_guiDlg::OnBnClickedGenerateWpsPin)
	ON_CBN_SELCHANGE(IDC_SECURITY_METHOD, &Cwps_test_guiDlg::OnCbnSelchangeSecurityMethod)
	ON_BN_CLICKED(IDC_MONITOR_HWBUTTON, &Cwps_test_guiDlg::OnBnClickedMonitorHwbutton)
END_MESSAGE_MAP()


// Cwps_test_guiDlg message handlers

BOOL Cwps_test_guiDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_hWnd = m_hWnd;

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_uiStatus=WPS_STATUS_IDLE;

	EnablePinControls(m_eSelectedWpsMode != STA_ENR_JOIN_NW_PBC);

	InitConfigureAPControls();

	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void Cwps_test_guiDlg::InitConfigureAPControls()
{
	/* 
		Supported encryption types:
			BRCM_SECURITY_WPA2PSK
			BRCM_SECURITY_WPAPSK
			BRCM_SECURITY_WEP
			BRCM_SECURITY_OPEN
	*/
	static CString strSecurityMethods[] = {_T("WPA2-Personal"), _T("WPA-Personal"), _T("WEP"), _T("No Security")};

	int nIndex = 0;
	for(; nIndex<BRCM_SECURITY_TOTAL; nIndex++)
		m_SecurityMethod.InsertString(nIndex, strSecurityMethods[nIndex]);
	m_SecurityMethod.SetCurSel(0);

	// Initialize encryption types
	nIndex = 0;
	m_EncrType.InsertString(nIndex++, _T("AES"));
	m_EncrType.InsertString(nIndex++, _T("TKIP"));
	m_EncrType.SetCurSel(0);

	OnCbnSelchangeSecurityMethod();

	OnBnClickedGenerateCredRandom();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Cwps_test_guiDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Cwps_test_guiDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Cwps_test_guiDlg::SetBtnStartWps()
{
	m_WpsJoinNetwork.EnableWindow();
	m_Exit.EnableWindow();
}

void Cwps_test_guiDlg::SetBtnCancelWps()
{
	m_WpsJoinNetwork.EnableWindow();
	m_Exit.EnableWindow();
}

LRESULT Cwps_test_guiDlg::OnWpsStatusUpdate(WPARAM wParam, LPARAM lParam)
{
	m_uiStatus=lParam;

	switch(wParam)  // WPS State Info.
	{
	case BRCM_WPSSTATE_WAITING_AP_SELECTION:
	{
		VEC_AP vecAPs = m_WpsManager.GetWpsAPs();
		CSelectAPDlg dlg(NULL, vecAPs);
		if(dlg.DoModal() == IDOK)
			m_WpsManager.SetSelectedWpsAP(dlg.GetSelectedAP());
		else
		{
			m_WpsManager.AbortWps(TRUE);
			m_bIsWpsRunning = FALSE;
			SetBtnStartWps();
		}
		break;
	}
	case BRCM_WPSSTATE_IDLE:
	{
		m_bIsWpsRunning = FALSE;
		SetBtnStartWps();
		switch(m_uiStatus)
		{
		case WPS_STATUS_SUCCESS:
			m_Status.InsertString(index++,_T("STATUS: SUCCESS"));
			break;
		case WPS_STATUS_ERROR:
			m_Status.InsertString(index++,_T("STATUS: ERROR"));
			break;
		case WPS_STATUS_CANCELED:
			m_Status.InsertString(index++,_T("STATUS: User canceled"));
			break;
		default:
				;
		}
		m_Status.InsertString(index++,_T("************* WPS END *************"));
		goto UPDATESTATUS_END;
	}
	case BRCM_WPSSTATE_SEARCH_AP:
	{
		switch(m_uiStatus)
		{
		case WPS_STATUS_SUCCESS:
			m_Status.InsertString(index++,_T("STATUS: AP found"));
			break;
		case WPS_STATUS_ERROR:
			if(m_WpsManager.m_nTotalFoundAP > 1)
			{
				// Multiple APs found in PBC mode (OVERLAP)
				m_Status.InsertString(index++,_T("STATUS: Overlap APs found. Try again in 2 minutes."));
			}
			else
			{
				// No AP is found 
				m_Status.InsertString(index++,_T("STATUS: No AP found"));
			}
			break;
		case WPS_STATUS_SCANNING:
			m_Status.InsertString(index++,_T("STATUS: Start AP Scanning......"));
			break;
		default:
				;
		}
		break;
	}
	case BRCM_WPSSTATE_CREATE_PROFILE:
	{
		switch(m_uiStatus)
		{
		case WPS_STATUS_CREATING_PROFILE:
			m_Status.InsertString(index++,_T("STATUS: Creating profile"));
			break;
		case WPS_STATUS_SUCCESS:
			m_Status.InsertString(index++,_T("STATUS: Creating profile succeeded"));
			break;
		case WPS_STATUS_ERROR:
			m_Status.InsertString(index++,_T("STATUS: Failed to create profile"));
			break;
		default: 
			;
		}
		goto UPDATESTATUS_END;
	}
	case BRCM_WPSSTATE_INFO:
	{
		CString str;
		switch(m_uiStatus)
		{
		case WPS_STATUS_INIT:
			SetBtnCancelWps();
			break;
		case WPS_STATUS_DISABLING_WIFI_MANAGEMENT:
			m_Status.InsertString(index++,_T("STATUS: DISABLING_WIFI_MANAGEMENT"));
			break;
		case WPS_STATUS_SCANNING:
			m_Status.InsertString(index++,_T("STATUS: SCANNING"));
			break;
		case WPS_STATUS_SCANNING_OVER_SUCCESS:
			m_Status.InsertString(index++,_T("STATUS: SCANNING OVER. WPS AP FOUND"));
			break;
		case WPS_STATUS_ASSOCIATING:
			m_Status.InsertString(index++, _T("STATUS: ASSOCIATING TO:"));
			m_Status.InsertString(index++, CString(m_WpsManager.GetSelectedWpsAP()->ssid));
			break;
		case WPS_STATUS_ASSOCIATED:
			m_Status.InsertString(index++,_T("STATUS: ASSOCIATED TO:"));
			m_Status.InsertString(index++, CString(m_WpsManager.GetSelectedWpsAP()->ssid));
			break;
		case WPS_STATUS_STARTING_WPS_EXCHANGE:
			m_Status.InsertString(index++,_T("STATUS: STARTING_WPS_EXCHANGE"));
			break;
		case WPS_STATUS_SENDING_WPS_MESSAGE:
			m_Status.InsertString(index++,_T("STATUS: SENDING_WPS_MESSAGE"));
			break;
		case WPS_STATUS_WAITING_WPS_RESPONSE:
			m_Status.InsertString(index++,_T("STATUS: WAITING_WPS_RESPONSE"));
			break;
		case WPS_STATUS_GOT_WPS_RESPONSE:
			m_Status.InsertString(index++,_T("STATUS: GOT_WPS_RESPONSE"));
			break;
		case WPS_STATUS_DISCONNECTING:
			m_Status.InsertString(index++,_T("STATUS: DISCONNECTING"));
			break;
		case WPS_STATUS_ENABLING_WIFI_MANAGEMENT:
			m_Status.InsertString(index++,_T("STATUS: ENABLING_WIFI_MANAGEMENT"));
			break;
		case WPS_STATUS_CREATING_PROFILE:
			m_Status.InsertString(index++,_T("STATUS: CREATING_PROFILE"));
			break;
//		case WPS_STATUS_WARNING_TIMEOUT:
//			m_Status.InsertString(index++,_T("STATUS: ERROR_TIMEOUT"));
//			break;
		case WPS_STATUS_WARNING_WPS_PROTOCOL_FAILED:
			m_Status.InsertString(index++,_T("STATUS: ERROR_WPS_PROTOCOL"));
			break;
		case WPS_STATUS_WARNING_NOT_INITIALIZED:
			m_Status.InsertString(index++,_T("STATUS: WARNING_NOT_INITIALIZED"));
			break;
		case WPS_STATUS_SCANNING_OVER_SESSION_OVERLAP:
			m_Status.InsertString(index++,_T("STATUS: WPS_STATUS_SCANNING_OVER_SESSION_OVERLAP"));
			break;
		case WPS_STATUS_WRONG_PIN:
			m_Status.InsertString(index++,_T("STATUS: WPS_STATUS_WRONG_PIN"));
			break;
		case WPS_STATUS_SUCCESS:
			m_Status.InsertString(index++,_T("STATUS: SUCCESS"));
			break;
	/*
		case WPS_STATUS_CANCELED:
			m_Status.InsertString(index++,_T("STATUS: CANCELED"));
			m_Button.SetWindowText("Quit");
			break;
	*/
		case WPS_STATUS_ERROR:
			m_Status.InsertString(index++,_T("STATUS: ERROR"));
			SetBtnStartWps();
			break;
		case WPS_STATUS_IDLE:
			m_Status.InsertString(index++,_T("STATUS: IDLE"));
			break;
		default:
			break;
		}
	}
	}

UPDATESTATUS_END:
	int nCount = m_Status.GetCount();
	if(nCount > 0)
	   m_Status.SetCurSel(nCount-1);
	return m_Continue;
}

void Cwps_test_guiDlg::StartConfiguringAP()
{
	m_Continue=TRUE;

	UpdateData();  

	m_Status.InsertString(index++,_T("************* WPS START *************"));

	// Make sure initialize in order of security method enum definition as following:
	// BRCM_SECURITY_WPA2PSK, BRCM_SECURITY_WPAPSK,	BRCM_SECURITY_WEP, BRCM_SECURITY_OPEN
	// We only support "OPEN" authentication method
	static char* szKeyMgmt[] = {"WPA2-PSK", "WPA-PSK", "OPEN", "OPEN"};  

	if(m_eGenerateCredMode == BRCM_GENERATECRED_MANUAL)
	{
		wps_credentials cred;
		BRCM_WPSSECURITYMETHOD eSecMethod = (BRCM_WPSSECURITYMETHOD)m_SecurityMethod.GetCurSel();

		strcpy(cred.ssid, CT2A(m_strSsid));
		strcpy(cred.nwKey, CT2A(m_strNetworkKey));
		strcpy(cred.keyMgmt, szKeyMgmt[eSecMethod]);
		if(eSecMethod == BRCM_SECURITY_WPA2PSK || eSecMethod == BRCM_SECURITY_WPAPSK)
		{
			int nSel = m_EncrType.GetCurSel();
			if(nSel != CB_ERR)
			{
				CString strEncrType;
				m_EncrType.GetLBText(nSel, strEncrType);
				if(strEncrType == CString(_T("AES")))
					cred.encrType = ENCRYPT_AES;  // "AES"
				else  
					cred.encrType = ENCRYPT_TKIP;  // "TKIP"
			}
		}
		else if(eSecMethod == BRCM_SECURITY_WEP)
		{
			cred.encrType = ENCRYPT_WEP;  // WEP
		}
		else
		{
			cred.encrType = ENCRYPT_NONE;  // Open/No-security
		}
		m_WpsManager.WpsConfigureAP(m_strAdapterGuid, CT2A(LPCTSTR(m_strAPPin)), m_hWnd, &cred);
	}
	else
	{
		m_WpsManager.WpsConfigureAP(m_strAdapterGuid, CT2A(LPCTSTR(m_strAPPin)), m_hWnd, NULL);
	}
}

void Cwps_test_guiDlg::StartJoiningNW()
{
	UpdateData();

	if(m_bIsWpsRunning)
	{
		StopWps();
	}
	else
	{
		if(m_strWpsPin.IsEmpty())
		{
			AfxMessageBox(_T("PIN can't be empty!"), MB_OK | MB_ICONWARNING);
			GetDlgItem(IDC_PIN)->SetFocus();
		}
		else
		{
			// To start WPS
			m_bIsWpsRunning = TRUE;
			m_Continue=TRUE;
			m_WpsJoinNetwork.EnableWindow(FALSE);
			m_Exit.EnableWindow(FALSE);

			m_Status.InsertString(index++,_T("************* WPS START *************"));

			m_WpsManager.StartJoinNW(m_strAdapterGuid, m_eSelectedWpsMode, CT2A(LPCTSTR(m_strWpsPin)), m_hWnd);

		}
	}
}

void Cwps_test_guiDlg::OnBnClickedStaErJoinNw()
{
	m_eSelectedWpsMode = STA_REG_JOIN_NW;

	StartJoiningNW();
}


void Cwps_test_guiDlg::OnBnClickedEnrolleeJoinNw()
{
	if(((CButton *)(GetDlgItem(IDC_SEL_MODE_PIN)))->GetCheck() == BST_CHECKED)
		m_eSelectedWpsMode = STA_ENR_JOIN_NW_PIN;
	else
		m_eSelectedWpsMode = STA_ENR_JOIN_NW_PBC;

	StartJoiningNW();
}

void Cwps_test_guiDlg::OnBnClickedExit()
{
	// Stop WPS before exiting
	if(m_bIsWpsRunning)
		StopWps();

	m_WpsManager.AbortWpsGpioPulling();
	
	OnOK();
}

void Cwps_test_guiDlg::StopWps()
{
	CWaitCursor wait;

	m_WpsManager.AbortWps();
	m_bIsWpsRunning = FALSE;

	wait.Restore();

	SetBtnStartWps();
}

void Cwps_test_guiDlg::OnBnClickedClearLog()
{
	m_Status.ResetContent();  // clear list box
	index = 0;  // reset index 
}

void Cwps_test_guiDlg::OnBnClickedSelModePin()
{
	((CButton *)(GetDlgItem(IDC_SEL_MODE_PBC)))->SetCheck(BST_UNCHECKED);
	((CButton *)(GetDlgItem(IDC_SEL_MODE_PIN)))->SetCheck(BST_CHECKED);

	EnablePinControls(TRUE);
}

void Cwps_test_guiDlg::OnBnClickedSelModePbc()
{
	((CButton *)(GetDlgItem(IDC_SEL_MODE_PBC)))->SetCheck(BST_CHECKED);
	((CButton *)(GetDlgItem(IDC_SEL_MODE_PIN)))->SetCheck(BST_UNCHECKED);

	EnablePinControls(FALSE);
}

void Cwps_test_guiDlg::EnablePinControls(BOOL bFlag)
{
	GetDlgItem(IDC_STATIC_ENTER_PIN)->EnableWindow(bFlag);
	GetDlgItem(IDC_PIN)->EnableWindow(bFlag);
	GetDlgItem(IDC_PIN)->SetFocus();

	GetDlgItem(IDC_STAER_JOIN_NW)->EnableWindow(bFlag);
}

void Cwps_test_guiDlg::OnBnClickedConfigureAp()
{
	UpdateData();

	m_eSelectedWpsMode = STA_REG_CONFIG_NW;

	if(m_strAPPin.IsEmpty())
	{
		AfxMessageBox(_T("PIN can't be empty!"), MB_OK | MB_ICONWARNING);
		GetDlgItem(IDC_AP_PIN)->SetFocus();
	}
	else
	{
		// To start WPS
		m_bIsWpsRunning = TRUE;
		m_bIsWpsRunning = TRUE;
		m_WpsJoinNetwork.EnableWindow(FALSE);
		StartConfiguringAP();
	}
}

void Cwps_test_guiDlg::OnCbnSelchangeSecurityMethod()
{
	BRCM_WPSSECURITYMETHOD eSecMethod = (BRCM_WPSSECURITYMETHOD)m_SecurityMethod.GetCurSel();
	switch(eSecMethod)
	{
		case BRCM_SECURITY_WPA2PSK:
		case BRCM_SECURITY_WPAPSK:
			m_NetworkKey.EnableWindow();
			m_EncrType.EnableWindow();
			break;
		case BRCM_SECURITY_WEP:
			m_NetworkKey.EnableWindow();
			m_EncrType.EnableWindow(0);
			break;
		case BRCM_SECURITY_OPEN:
			m_NetworkKey.EnableWindow(0);
			m_EncrType.EnableWindow(0);
			break;
	}
}

void Cwps_test_guiDlg::OnBnClickedGenerateCredRandom()
{
	m_eGenerateCredMode = BRCM_GENERATECRED_RANDOM;
	m_NetworkKey.EnableWindow(0);
	m_EncrType.EnableWindow(0);
	m_SecurityMethod.EnableWindow(0);
	m_Ssid.EnableWindow(0);
}

void Cwps_test_guiDlg::OnBnClickedGenerateCredManual()
{
	m_eGenerateCredMode = BRCM_GENERATECRED_MANUAL;
	m_NetworkKey.EnableWindow();
	m_EncrType.EnableWindow();
	m_SecurityMethod.EnableWindow();
	m_Ssid.EnableWindow();
}

void Cwps_test_guiDlg::OnBnClickedCancelWps()
{
	if(m_bIsWpsRunning)
	{
		StopWps();
	}
}

LRESULT Cwps_test_guiDlg::OnWpsHWTriggerStateUpdate(WPARAM wParam, LPARAM lParam)
{
	// wParam contains whether the WPS button is pressed or not
	if(wParam)
	{
		if(m_bIsWpsRunning)
		{
			// Prompt that WPS is running already. We only pop up one message box on this
			HWND hWnd = ::FindWindow(NULL, BCM_WPS_MSGBOX_TITLE);
			if(!hWnd)
				MessageBox(_T("WPS is running already!"), BCM_WPS_MSGBOX_TITLE, MB_ICONEXCLAMATION | MB_APPLMODAL);
		}
		else
		{
			OnBnClickedSelModePbc();
			UpdateData(FALSE);  // Update UI to reflect change of WPS mode

			OnBnClickedEnrolleeJoinNw();
		}
	}
	return 0;
}

void Cwps_test_guiDlg::OnBnClickedGenerateWpsPin()
{
	char szPin[WPS_PIN_TOTAL_DIGIT+1] = { 0 };
	
	wps_api_generate_pin(szPin, sizeof(szPin));

	m_strWpsPin = szPin;

	UpdateData(FALSE);
}

void Cwps_test_guiDlg::OnBnClickedMonitorHwbutton()
{
	UpdateData();

	if(m_WpsManager.m_bMonitorHwButtonInProgress)
		MessageBox(_T("Monitoring Hardware WPS Button is already in progress!"), NULL, MB_ICONINFORMATION);
	else
		m_WpsManager.StartMonitorWpsHWTrigger(m_strAdapterGuid, m_hWnd);
}
