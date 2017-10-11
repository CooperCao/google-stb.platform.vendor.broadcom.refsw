// WizTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WizTest.h"
#include "WizTestDlg.h"
#include "globals.h"
#include "DlgWPSCredentials.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MARGIN 2
class CStoreList
{
public:
	uint8 m_bssid[6];
	char m_ssid[33];
	uint8 m_wep;
	uint16 m_band;
};

enum eWPS_COMBO_MODE {
	STA_CONFIGURE = 0,
	STA_ENROLL = 1,
	STA_JOIN = 2
};


CWizTestDlg* g_cWizTestDlg = 0;
unsigned int g_uiStatus;
HANDLE g_Thread = 0;
DWORD WINAPI GetCredentialsThread(LPVOID lpParam);
bool g_DisableCallback = false;
bool g_WZCDisabled = false;


// CWizTestDlg dialog
CWizTestDlg::CWizTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWizTestDlg::IDD, pParent)
	, m_bPinMode(TRUE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	g_uiStatus=0;
	g_cWizTestDlg = 0;
	m_CredentialsSelected = TRUE;
}

CWizTestDlg::~CWizTestDlg()
{
	g_DisableCallback = true;
	if (g_Thread)
	{
		CloseHandle(g_Thread);
		g_Thread = 0;
	}
}

void CWizTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NETWORKS, m_lstNetworks);
	DDX_Control(pDX, IDC_PIN_STRING, m_edtPinValue);
	DDX_Check(pDX, IDC_PIN_MODE, m_bPinMode);
	DDX_Control(pDX, IDC_EDIT_STATUS, m_edtStatus);
	DDX_Control(pDX, IDC_COMBO_MODE, m_listWPSMode);
}

BEGIN_MESSAGE_MAP(CWizTestDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_ENROLL, &CWizTestDlg::OnBnClickedEnroll)
	ON_BN_CLICKED(IDC_PIN_MODE, &CWizTestDlg::OnBnClickedPinMode)
	ON_BN_CLICKED(IDC_REFRESH, &CWizTestDlg::OnBnClickedRefresh)
	ON_BN_CLICKED(IDCANCEL, &CWizTestDlg::OnBnClickedCancel)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_NETWORKS, &CWizTestDlg::OnLvnItemchangedNetworks)
	ON_CBN_SELCHANGE(IDC_COMBO_MODE, &CWizTestDlg::OnCbnSelchangeComboMode)
	ON_BN_CLICKED(IDC_BUTTON_CREDENTIALS, &CWizTestDlg::OnBnClickedButtonCredentials)
	ON_NOTIFY(NM_KILLFOCUS, IDC_NETWORKS, &CWizTestDlg::OnNMKillfocusNetworks)
	ON_NOTIFY(NM_SETFOCUS, IDC_NETWORKS, &CWizTestDlg::OnNMSetfocusNetworks)
	ON_EN_SETFOCUS(IDC_PIN_STRING, &CWizTestDlg::OnEnSetfocusPinString)
	ON_CBN_SETFOCUS(IDC_COMBO_MODE, &CWizTestDlg::OnCbnSetfocusComboMode)
	ON_EN_CHANGE(IDC_PIN_STRING, &CWizTestDlg::OnEnChangePinString)
END_MESSAGE_MAP()


// CWizTestDlg message handlers

BOOL CWizTestDlg::OnInitDialog()
{
	int index = 0;

	CDialog::OnInitDialog();
	AdjustControls();

	CString strPin(_T("14412783"));
	m_edtPinValue.SetLimitText(8);

	CenterWindow();
	m_edtPinValue.ShowWindow(SW_SHOW);
	m_edtPinValue.SetWindowTextW(strPin);
	wtoa(strPin.GetBuffer(0), m_pin, 79);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_lstNetworks.InsertColumn(0, _T("SSID"));
	m_lstNetworks.InsertColumn(1, _T("BSSID"));

	CRect rect;
	m_lstNetworks.GetWindowRect(&rect);
	m_lstNetworks.SetColumnWidth(0, (int)(rect.Width()/4));
	m_lstNetworks.SetColumnWidth(1, (int)(3*rect.Width()/4));

	m_lstNetworks.SetExtendedStyle(m_lstNetworks.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_SHOWSELALWAYS );

	m_listWPSMode.InsertString(index++, _T("STA Configure AP"));
	m_listWPSMode.InsertString(index++, _T("STA Enrollee Join"));
	m_listWPSMode.InsertString(index++, _T("STA Registrar Join"));

	m_listWPSMode.SetCurSel(STA_ENROLL);

	UpdateControls(STA_ENROLL);

	UpdateStatus(_T("Pin Mode"));
	
	g_cWizTestDlg=this;
	bWpsCancelled = FALSE;

	// TODO: Add your control notification handler code here	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWizTestDlg::OnBnClickedEnroll()
{
	//If already another thread is running, do nothing
	if (!g_Thread)
	{
		g_Thread = CreateThread( NULL, 0, 
		       GetCredentialsThread, NULL, 0, NULL);  
		if (g_Thread)
			GetDlgItem(IDC_ENROLL)->EnableWindow(FALSE);		
	}
}

void CWizTestDlg::OnBnClickedPinMode()
{
	UpdateData(TRUE);
	UpdatePinMode(m_bPinMode);
}

void CWizTestDlg::UpdatePinMode(BOOL bPinMode)
{
	if (!bPinMode)
	{
		UpdateStatus(_T("PBC Mode"));
		//m_edtPinValue.SetWindowTextW(_T(""));
		m_edtPinValue.ShowWindow(SW_HIDE);
	}
	else
	{
		//wchar_t szChar[200];
		//atow(m_pin,szChar,199);
		UpdateStatus(_T("Pin Mode"));
		//m_edtPinValue.SetWindowTextW(szChar);
		m_edtPinValue.ShowWindow(SW_SHOW);
	}
}


void CWizTestDlg::OnBnClickedRefresh()
{
	Refresh();
	UpdateControls(m_listWPSMode.GetCurSel());
}

void CWizTestDlg::Refresh()
{
	char start_ok = 0;
	uint8 wep = 1;
	uint16 band = 0;
	CString strPin;
	int nAP = 0;
	uint8 bssid[6];
	char  ssid[33];
	wchar_t bssidString[100];

	CWaitCursor c;
	UpdateData(TRUE);

	m_edtPinValue.GetWindowTextW(strPin);
	strPin.TrimLeft();
	strPin.TrimRight();
	wtoa(strPin.GetBuffer(0), m_pin, 79);

	Clear();
	UpdateData(FALSE);

	bool bFoundAP = wps_findAP(&nAP, GetWpsMode(), 10);

	if(bFoundAP) 
	{
		if(m_bPinMode && nAP>0)
		{
			int i=0;

			while(wps_getAP(i, bssid, (char *) ssid, &wep, &band))
			{
				CString str(ssid);
				CStoreList* pStore = new CStoreList();
				strcpy(pStore->m_ssid, ssid);
				for(int j=0;j<6;j++)
					pStore->m_bssid[j] = bssid[j];
				wsprintf(bssidString,_T("%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x"),
					pStore->m_bssid[0], pStore->m_bssid[1], pStore->m_bssid[2], 
					pStore->m_bssid[3], pStore->m_bssid[4], pStore->m_bssid[5], 
					pStore->m_bssid[6]);
				pStore->m_wep = wep;
				pStore->m_band = band;
				m_lstNetworks.InsertItem(i, str);
				m_lstNetworks.SetItemText(i, 1, bssidString);
				m_lstNetworks.SetItemData(i, (DWORD_PTR)(pStore));
				i++;
			}
		}
		else if (!m_bPinMode && nAP > 0) 
		{
			if(nAP > 1) 
			{
				AfxMessageBox(_T("More than one PBC AP found. Restarting scanning"), MB_ICONSTOP);
			} 
			else 
			{
				if (wps_getAP(0, bssid, ssid, &wep, &band))
				{
					CString str(ssid);
					CStoreList* pStore = new CStoreList();
					strcpy(pStore->m_ssid, ssid);
					for(int j=0;j<6;j++)
						pStore->m_bssid[j] = bssid[j];
					wsprintf(bssidString,_T("%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x"),
						pStore->m_bssid[0], pStore->m_bssid[1], pStore->m_bssid[2], 
						pStore->m_bssid[3], pStore->m_bssid[4], pStore->m_bssid[5], 
						pStore->m_bssid[6]);
					pStore->m_wep = wep;
					pStore->m_band = band;
					m_lstNetworks.InsertItem(0, str);
					m_lstNetworks.SetItemText(0, 1, bssidString);
					m_lstNetworks.SetItemData(0, (DWORD_PTR)(pStore));
				}
			}
		} 
	} 
}

	
int CWizTestDlg::GetCredentials()
{
	bool bContinue = true;
	CStoreList* pStore = 0;
	g_uiStatus=0;

	int i=20;
	unsigned long pin = strtoul(m_pin,NULL,0);

	if (m_bPinMode && strlen(m_pin) == 0)
	{
		AfxMessageBox(_T("No pin defined."), MB_ICONSTOP);
		bContinue = false;
	} 
	else if((STA_REG_CONFIG_NW == GetWpsMode()) && 
		!wps_validate_checksum(pin))
	{
		AfxMessageBox(_T("PIN Checksum Invalid."), MB_ICONSTOP);
		bContinue = false;
	}
	else
	{
		POSITION pos = m_lstNetworks.GetFirstSelectedItemPosition();
		if (pos == NULL)
		{
			AfxMessageBox(_T("No APs selected."), MB_ICONSTOP);
			bContinue = false;
		}
		else
		{
			while (pos)
			{
				int nItem = m_lstNetworks.GetNextSelectedItem(pos);
				pStore = (CStoreList*)(m_lstNetworks.GetItemData(nItem));
			}
		}
	}

	if (bContinue && pStore)
	{
		CWaitCursor c;

		//Disable Zero Config
		if (wps_configure_wzcsvc(0))
		{
			g_WZCDisabled = true;
			//if it is configure mode, see if it is already configured
			if (GetWpsMode() == STA_REG_CONFIG_NW)
			{
				if (wps_get_AP_scstate(pStore->m_bssid) == WPS_SCSTATE_CONFIGURED)
				{
					wchar_t szConfigured[1024];
					CString strSSID(pStore->m_ssid);
					wsprintf(szConfigured, _T("%s is a configured network.")
						_T(" Are you sure you want to ")
						_T("overwrite existing network")
						_T(" settings?"),
						strSSID);

					//create a WZC profile
					if (AfxMessageBox(szConfigured,  MB_YESNO) == IDNO)
					{
						bContinue = FALSE;
					}
				}
			}

			if (bContinue)
			{		
				if(wps_join(pStore->m_bssid, pStore->m_ssid, pStore->m_wep))
				{
					BOOL bOk = FALSE;
					if (GetWpsMode() != STA_REG_CONFIG_NW)
					{
						memset(&m_credentials, 0 , sizeof(wps_credentials));

						bOk = wps_get_AP_infoEx(GetWpsMode(),
							&(pStore->m_bssid[0]), 
							&(pStore->m_ssid[0]), 
							m_bPinMode?m_pin:NULL,
							&m_credentials);
					}
					else
					{
						bOk = wps_configureAP(&(pStore->m_bssid[0]), &m_pin[0], &m_credentials);
					}

					if (bOk)
					{
						// Wait for WPS to succeed, fail, or be canceled while checking for user cancel action
						while(g_uiStatus!=WPS_STATUS_SUCCESS && g_uiStatus!=WPS_STATUS_CANCELED && g_uiStatus!=WPS_STATUS_ERROR) 
						{
							Sleep(1000);
						}

						GetDlgItem(IDC_ENROLL)->EnableWindow(TRUE);

						if(g_uiStatus==WPS_STATUS_SUCCESS)
						{
							wchar_t szCred[1024];
							CString strKeys;
							CString strSSID(m_credentials.ssid);
							CString strkeyMgmt(m_credentials.keyMgmt);
							CString strnwKey(m_credentials.nwKey);


							if(m_credentials.encrType & WPS_ENCRYPT_NONE) 
								strKeys =_T("NONE\n");
							if(m_credentials.encrType & WPS_ENCRYPT_WEP)
								strKeys.Format(_T(" WEP (Index %d)"), m_credentials.wepIndex);
							if(m_credentials.encrType & WPS_ENCRYPT_TKIP)
								strKeys = _T(" TKIP");
							if(m_credentials.encrType & WPS_ENCRYPT_AES)
								strKeys = _T(" AES");
							wsprintf(szCred, _T("WPS AP Credentials:\nSSID = %s\nKey Mgmt type is %s\nKey : %s\nEncryption : %s\nCreate Profile?"),
								strSSID, strkeyMgmt, strnwKey, strKeys);

							//create a WZC profile
							if (AfxMessageBox(szCred,  MB_YESNO) == IDYES)
							{
								if (wps_create_profile(&m_credentials))
								{
									AfxMessageBox(_T("Profile successully created."),  MB_TOPMOST | MB_SETFOREGROUND |MB_OK);
								}
								else
								{
									AfxMessageBox(_T("Profile creation failed."), MB_TOPMOST | MB_SETFOREGROUND |MB_ICONSTOP);
								}
							}
						}
						else
						{
							if(!bWpsCancelled)
								switch(g_uiStatus)
								{
								case WPS_STATUS_CANCELED:
									AfxMessageBox(_T("WPS protocol CANCELED by user."), MB_ICONSTOP);
									break;
								case WPS_STATUS_ERROR:
									AfxMessageBox(_T("WPS protocol error."), MB_ICONSTOP);
									break;
								default:
									AfxMessageBox(_T("WPS protocol error unknown."), MB_ICONSTOP);
									break;
								}
						}
					}
					else
					{
						if(!bWpsCancelled)
							AfxMessageBox(_T("WPS protocol failed."), MB_ICONSTOP);
					}
				}
				else
				{
					if(!bWpsCancelled)
						AfxMessageBox(_T("Unable to join WPS capable AP"), MB_ICONSTOP);
				}
			}
			if (wps_configure_wzcsvc(1))
			{
				g_WZCDisabled = false;
				Sleep(500);
			}
		}
	}

	if(bWpsCancelled) {
		//User has pressed Cancel
		OnCancel();
	}
	return 0;
}
	
void CWizTestDlg::Clear()
{
	for (int i=0;i < m_lstNetworks.GetItemCount();i++)
	{
		CStoreList* pStore = (CStoreList*)(m_lstNetworks.GetItemData(i));
		if (pStore)
			delete pStore;
	}
	m_lstNetworks.DeleteAllItems();
}

void CWizTestDlg::UpdateStatus(LPCTSTR pszStatus)
{
    m_edtStatus.SetWindowTextW(pszStatus);
}

bool CWizTestDlg::OnCallback(void *context,unsigned int uiStatus, void *data)
{
	TCHAR stStatus[1024];
	g_uiStatus=uiStatus;

	if (g_DisableCallback)
		return false;

	switch(uiStatus)
	{
	case WPS_STATUS_DISABLING_WIFI_MANAGEMENT:
		_tcscpy(stStatus, _T("Disabling WZC"));
		break;
	case WPS_STATUS_SCANNING:
		_tcscpy(stStatus, _T("Scanning"));
		break;
	case WPS_STATUS_SCANNING_OVER:
		_tcscpy(stStatus, _T("Scanning over"));
		break;
	case WPS_STATUS_ASSOCIATING:
		{
			CString ssid((char*)data);
			wsprintf(stStatus, _T("Associating to %s"), ssid);
		}
	break;
	case WPS_STATUS_ASSOCIATED:
		{
			CString ssid((char*)data);
			wsprintf(stStatus, _T("Associated to %s"), ssid);
		}
	break;
	case WPS_STATUS_STARTING_WPS_EXCHANGE:
		_tcscpy(stStatus, _T("Start WPS Exchange"));
	break;
	case WPS_STATUS_SENDING_WPS_MESSAGE:
		_tcscpy(stStatus, _T("Sending Message"));
	break;
	case WPS_STATUS_WAITING_WPS_RESPONSE:
		_tcscpy(stStatus, _T("Waiting for Response"));
	break;
	case WPS_STATUS_GOT_WPS_RESPONSE:
		_tcscpy(stStatus, _T("Got Response"));
	break;
	case WPS_STATUS_DISCONNECTING:
		_tcscpy(stStatus, _T("Disconnecting"));
	break;
	case WPS_STATUS_ENABLING_WIFI_MANAGEMENT:
		_tcscpy(stStatus, _T("Enabling WZC"));
	break;
	case WPS_STATUS_SUCCESS:
		_tcscpy(stStatus, _T("Success"));
	break;
	case WPS_STATUS_CANCELED:
		_tcscpy(stStatus, _T("Cancelled"));
	break;
	case WPS_STATUS_WARNING_TIMEOUT:
		_tcscpy(stStatus, _T("Error: Timeout"));
	break;
	case WPS_STATUS_WARNING_WPS_PROTOCOL_FAILED:
		_tcscpy(stStatus, _T("Error: WPS Protocol"));
	break;
	case WPS_STATUS_WARNING_NOT_INITIALIZED:
		_tcscpy(stStatus, _T("Not Intialized"));
	break;
	case WPS_STATUS_ERROR:
		_tcscpy(stStatus, _T("Error")); 
	break;
	case WPS_STATUS_IDLE:
		_tcscpy(stStatus, _T("Idle"));
	break;
	case WPS_STATUS_CREATING_PROFILE:
		_tcscpy(stStatus, _T("Creating Profile"));
	break;
	case WPS_STATUS_OVERALL_PROCESS_TIMOUT:
		_tcscpy(stStatus, _T("Process Timeout"));
	break;
	case WPS_STATUS_CONFIGURING_ACCESS_POINT:
		_tcscpy(stStatus, _T("Configuring AP"));
	break;
	case WPS_STATUS_REATTEMPTING_WPS:
		_tcscpy(stStatus, _T("Re-attempting WPS"));
	break;
	case WPS_STATUS_SCANNING_OVER_NO_AP_FOUND:
		_tcscpy(stStatus, _T("No AP(s) found."));
		AfxMessageBox(stStatus, MB_ICONSTOP);		
	break;
	case WPS_STATUS_SCANNING_OVER_SESSION_OVERLAP:
		_tcscpy(stStatus, _T("PBC Session overalp detected."));
		 AfxMessageBox(_T("Another active PBC session detected. Try after sometime."), MB_ICONSTOP);
		 break;
	default:
		_tcscpy(stStatus, _T("Unknown"));
	}
	if (g_cWizTestDlg)
		g_cWizTestDlg->UpdateStatus(stStatus);

	wprintf(_T("%s\n"), stStatus);
	return true;
}

DWORD WINAPI GetCredentialsThread( LPVOID lpParam ) 
{ 
	if (g_cWizTestDlg)
		g_cWizTestDlg->GetCredentials();
	
	if (g_Thread)
	{
			CloseHandle(g_Thread);
			g_Thread = NULL;
	}
	return 0; 
} 


void CWizTestDlg::OnBnClickedCancel()
{
	g_DisableCallback = true;

	if (!g_Thread){
		OnCancel();
	} else{
		//Do not allow user to press any more buttons
		GetDlgItem(IDC_REFRESH)->EnableWindow(FALSE);
		GetDlgItem(IDC_ENROLL)->EnableWindow(FALSE);
		bWpsCancelled = TRUE;
	}
}

LONG CWizTestDlg::AdjustControls()
{
	int i;
	CRect rectDlg;
	CRect rect;
	GetWindowRect(&rectDlg);
	ScreenToClient(&rectDlg);

	CWnd *pWnd = 0;
	int iBottomRetry = 0;

    //cancel window
	pWnd = GetDlgItem(IDCANCEL);
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect);
	int iWidth = rect.Width();
    rect.right = rectDlg.right-MARGIN;
	rect.left = rect.right - iWidth;
	int iModeRight = rect.left - MARGIN;
	pWnd->MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());
	InvalidateRect(&rect);

	//mode window
	pWnd = GetDlgItem(IDC_COMBO_MODE);
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.right = iModeRight;
	iBottomRetry = rect.bottom;
	pWnd->MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());
	InvalidateRect(&rect);

	//Pin String
	pWnd = GetDlgItem(IDC_PIN_STRING);
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.right = rectDlg.right-MARGIN;;
	iBottomRetry += rect.Height() + 5 * MARGIN ;
	pWnd->MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());
	InvalidateRect(&rect);

	//Push Buttons
	int iHeight = 0;
	int TopButtons = 0;
	for (i=0;i<3;i++)
	{
		if (i == 0)
		{
			//scan
			pWnd = GetDlgItem(IDC_REFRESH);
		}
		else if (i == 1)
		{
			//enroll
			pWnd = GetDlgItem(IDC_ENROLL);
		}
		else if (i == 2)
		{
			//scan
			pWnd = GetDlgItem(IDC_BUTTON_CREDENTIALS);
		}

		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);
		iHeight = rect.Height();
		rect.bottom = rectDlg.bottom-MARGIN;
		rect.top = rect.bottom - iHeight;
		TopButtons = rect.top;
		pWnd->MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());
		InvalidateRect(&rect);
	}

	//edit status
	int TopStatus = 0;
	pWnd = GetDlgItem(IDC_EDIT_STATUS);
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.right = rectDlg.right-MARGIN;
	iHeight = rect.Height();
	rect.bottom = TopButtons;
	rect.top = rect.bottom - iHeight;
	TopStatus = rect.top;
	pWnd->MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());
	InvalidateRect(&rect);

	//scan list
	pWnd = GetDlgItem(IDC_NETWORKS);
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.right = rectDlg.right-MARGIN;
	iHeight = rect.Height();
	rect.bottom = TopStatus;
	rect.top = iBottomRetry;
	pWnd->MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());
	InvalidateRect(&rect);

	UpdateControls(m_listWPSMode.GetCurSel());
	UpdateWindow();

    return 0;
}

void CWizTestDlg::OnDestroy()
{
	if (g_WZCDisabled)
	{
		wps_configure_wzcsvc(1);
		Sleep(500);
	}

	g_DisableCallback = true;
	CDialog::OnDestroy();
}
void CWizTestDlg::OnLvnItemchangedNetworks(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if (GetWpsMode() != STA_REG_CONFIG_NW)
	{
		m_CredentialsSelected = TRUE;
	}
	else
	{
		m_CredentialsSelected = FALSE;
	}
	UpdateControls(m_listWPSMode.GetCurSel());
}

void CWizTestDlg::GetConfigurationCredentials()
{
	wps_credentials credentials;
	BOOL Ok = true;
	CStoreList* pStore = 0;
	int iSelectedItem = -1;

	POSITION pos = m_lstNetworks.GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		AfxMessageBox(_T("No APs selected."), MB_ICONSTOP);
		Ok = false;
	}
	else
	{
		while (pos)
		{
			int nItem = m_lstNetworks.GetNextSelectedItem(pos);
			iSelectedItem = nItem;
			pStore = (CStoreList*)(m_lstNetworks.GetItemData(nItem));
		}
	}

	if (Ok && pStore)
	{
		strcpy(credentials.ssid, pStore->m_ssid);
		CDlgWPSCredentials dlg(&credentials);
		dlg.DoModal();
		m_CredentialsSelected = TRUE;
		memcpy(&m_credentials, dlg.GetCredentials(), sizeof(wps_credentials));

		if (iSelectedItem != -1)
		{
			m_lstNetworks.SetSelectionMark(iSelectedItem);
			UpdateControls(m_listWPSMode.GetCurSel());
			m_lstNetworks.SetFocus();
		}
	}
}

void CWizTestDlg::OnCbnSelchangeComboMode()
{
	Clear();

	int iSel = m_listWPSMode.GetCurSel();

	if (GetWpsMode() != STA_REG_CONFIG_NW)
	{
		m_CredentialsSelected = TRUE;
	}
	else
	{
		m_CredentialsSelected = FALSE;
	}
	UpdateControls(iSel);
}

void CWizTestDlg::UpdateControls(int iSel)
{
    eWPS_COMBO_MODE nMode = static_cast<eWPS_COMBO_MODE>(iSel);
	CWnd* pWndMode = GetDlgItem(IDC_ENROLL);
	CWnd* pWndCred = GetDlgItem(IDC_BUTTON_CREDENTIALS);
	//CWnd* pWndPin = GetDlgItem(IDC_PIN_MODE);

	//pWndPin->EnableWindow(FALSE);
	pWndMode->EnableWindow(FALSE);
	pWndCred->EnableWindow(FALSE);

	//If another thread is running do not enable buttons
	if (m_lstNetworks.GetItemCount() > 0 && !g_Thread)
	{
		POSITION pos = m_lstNetworks.GetFirstSelectedItemPosition();
		if (pos != NULL)
		{
			if (!m_CredentialsSelected)
			{
				pWndMode->EnableWindow(FALSE);
			}
			else
			{
				pWndMode->EnableWindow(TRUE);
			}
			pWndCred->EnableWindow(TRUE);
		}
	}

	switch(nMode)
	{
		case STA_CONFIGURE:
			pWndMode->SetWindowTextW(_T("Configure"));
			pWndCred->ShowWindow(SW_SHOW);
			break;
		case STA_ENROLL:
			pWndMode->SetWindowTextW(_T("Enroll"));
			pWndCred->ShowWindow(SW_HIDE);
			//pWndPin->EnableWindow(TRUE);
			break;
		case STA_JOIN:
			pWndMode->SetWindowTextW(_T("Join"));
			pWndCred->ShowWindow(SW_HIDE);
		break;
		default:
			break;
	}

	UpdateData(FALSE);
	UpdatePinMode(m_bPinMode);
}

eWPS_MODE CWizTestDlg::GetWpsMode() 
{
	eWPS_MODE iMode;
	int iSel = m_listWPSMode.GetCurSel();

    eWPS_COMBO_MODE nMode = static_cast<eWPS_COMBO_MODE>(iSel);
	switch(nMode)
	{
		case STA_CONFIGURE:
			iMode = STA_REG_CONFIG_NW;
			break;
		case STA_ENROLL:
			iMode = m_bPinMode?STA_ENR_JOIN_NW_PIN:STA_ENR_JOIN_NW_PBC;
			break;
		case STA_JOIN:
			iMode = STA_REG_JOIN_NW;
			break;
		default:
			break;
	}
	return iMode;
}

void CWizTestDlg::OnBnClickedButtonCredentials()
{
	GetConfigurationCredentials();
}


void CWizTestDlg::OnNMKillfocusNetworks(NMHDR *pNMHDR, LRESULT *pResult)
{

}

void CWizTestDlg::OnNMSetfocusNetworks(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdateControls(m_listWPSMode.GetCurSel());
}

void CWizTestDlg::OnEnSetfocusPinString()
{
	CWnd* pWndMode = GetDlgItem(IDC_ENROLL);
	CWnd* pWndCred = GetDlgItem(IDC_BUTTON_CREDENTIALS);
	pWndMode->EnableWindow(FALSE);
	pWndCred->EnableWindow(FALSE);
}


void CWizTestDlg::OnCbnSetfocusComboMode()
{
	CWnd* pWndMode = GetDlgItem(IDC_ENROLL);
	CWnd* pWndCred = GetDlgItem(IDC_BUTTON_CREDENTIALS);
	pWndMode->EnableWindow(FALSE);
	pWndCred->EnableWindow(FALSE);
}

void CWizTestDlg::OnEnChangePinString()
{
	CString strPin;

	UpdateData(TRUE);
	m_edtPinValue.GetWindowTextW(strPin);
	strPin.TrimLeft();
	strPin.TrimRight();
	wtoa(strPin.GetBuffer(0), m_pin, 79);

}

