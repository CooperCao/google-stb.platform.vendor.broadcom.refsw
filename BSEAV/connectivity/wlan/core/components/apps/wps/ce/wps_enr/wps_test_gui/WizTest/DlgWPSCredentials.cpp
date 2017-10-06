// DlgWPSCredentials.cpp : implementation file
//

#include "stdafx.h"
#include <stdlib.h>
#include "WizTest.h"
#include "DlgWPSCredentials.h"

#define RIGHTMARGIN 2
#define BOTTOMMARGIN 2

// CDlgWPSCredentials dialog

IMPLEMENT_DYNAMIC(CDlgWPSCredentials, CDialog)

CDlgWPSCredentials::CDlgWPSCredentials(wps_credentials* pCredentials,
									   CWnd* pParent /*=NULL*/)
	: CDialog(CDlgWPSCredentials::IDD, pParent)
	, m_bRandom(FALSE)
{
    m_pCredentials = pCredentials;
}

CDlgWPSCredentials::~CDlgWPSCredentials()
{
}

void CDlgWPSCredentials::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_RANDOM_CREDENTIALS, m_bRandom);
	DDX_Control(pDX, IDC_EDIT_SSID, m_edtSSID);
	DDX_Control(pDX, IDC_COMBO_KEY_MGMT, m_cmbKeyMgmt);
	DDX_Control(pDX, IDC_COMBO_ENCRYPTION, m_cmbEncyption);
	DDX_Control(pDX, IDC_EDIT_KEY, m_edtKey);
	DDX_Control(pDX, IDC_EDIT_PIN_GENERATED, m_edtPinValue);
}


BEGIN_MESSAGE_MAP(CDlgWPSCredentials, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_OK, &CDlgWPSCredentials::OnBnClickedButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CDlgWPSCredentials::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_CHECK_RANDOM_CREDENTIALS, &CDlgWPSCredentials::OnBnClickedCheckRandomCredentials)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE_PIN, &CDlgWPSCredentials::OnBnClickedButtonGeneratePin)
	ON_BN_CLICKED(IDC_BUTTON_TEST_PIN, &CDlgWPSCredentials::OnBnClickedButtonTestPin)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_COMBO_KEY_MGMT, &CDlgWPSCredentials::OnCbnSelchangeComboKeyMgmt)
	ON_CBN_SELCHANGE(IDC_COMBO_ENCRYPTION, &CDlgWPSCredentials::OnCbnSelchangeComboEncryption)
END_MESSAGE_MAP()


// CDlgWPSCredentials message handlers
BOOL CDlgWPSCredentials::OnInitDialog()
{
	int index = 0;
	CDialog::OnInitDialog();
	AdjustControls();
	CenterWindow();

	m_edtSSID.SetLimitText(32);
	m_edtKey.SetLimitText(64);

	m_cmbKeyMgmt.InsertString(index++, _T("OPEN"));
	m_cmbKeyMgmt.InsertString(index++, _T("WPA-PSK"));
	m_cmbKeyMgmt.InsertString(index++, _T("WPA2-PSK"));
	m_cmbKeyMgmt.InsertString(index++, _T("WPA-PSK WPA2-PSK"));
	m_cmbKeyMgmt.SetCurSel(1); //WPA-PSK

	index = 0;
	m_cmbEncyption.InsertString(index++, _T("TKIP"));
	m_cmbEncyption.InsertString(index++, _T("AES"));
	m_cmbEncyption.InsertString(index++, _T("TKIP and AES"));
	m_cmbEncyption.SetCurSel(0); //TKIP

	wchar_t szChar[80];
	atow(m_pCredentials->ssid,szChar,32);
	m_edtSSID.SetWindowTextW(szChar);

	CString strPin(_T("14412783"));
	m_edtPinValue.SetWindowTextW(strPin);
	wtoa(strPin.GetBuffer(0), m_pin, 79);

	// TODO: Add your control notification handler code here	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

LONG CDlgWPSCredentials::AdjustControls()
{
	int i;
	CRect rectDlg;
	GetWindowRect(&rectDlg);
	ScreenToClient(&rectDlg);

	CWnd *pWnd = 0;
	int iBottomRetry = 0;
	for (i=0;i<4;i++)
	{
		if (i==0)
		{
			//SSID
			pWnd = GetDlgItem(IDC_EDIT_SSID);
		}
		else if (i==1)

		{
			//key management
			pWnd = GetDlgItem(IDC_COMBO_KEY_MGMT);
		}
		else if (i==2)

		{
			//key management
			pWnd = GetDlgItem(IDC_COMBO_ENCRYPTION);
		}
		else 
		{
			//key
			pWnd = GetDlgItem(IDC_EDIT_KEY);
		}
		if (pWnd)
		{
			CRect rect;
			pWnd->GetWindowRect(&rect);
			ScreenToClient(&rect);
			rect.right = rectDlg.right-RIGHTMARGIN;
			iBottomRetry = rect.bottom;
			pWnd->MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());
			InvalidateRect(&rect);
		}
	}
	UpdateWindow();
    return 0;
}

void CDlgWPSCredentials::OnBnClickedButtonOk()
{
	BOOL bContinue = TRUE;
	UpdateData(TRUE);
	
	CWaitCursor c;

	if (!m_bRandom)
	{
		//check SSID
		CString strSSID;
		m_edtSSID.GetWindowTextW(strSSID);
		strSSID.TrimLeft();
		strSSID.TrimRight();
		wtoa(strSSID.GetBuffer(0), m_pCredentials->ssid, 32);

		if (strlen(m_pCredentials->ssid) == 0)
		{
			AfxMessageBox(_T("No SSID defined."), MB_ICONSTOP);
			bContinue = FALSE;
		}

		//key management
		if (m_cmbKeyMgmt.GetCurSel() == 0 ) //OPEN
		{
			m_pCredentials->keyMgmt[0] = '\0';
			if (m_cmbEncyption.GetCurSel() == 0 ) //NONE
			{
				m_pCredentials->encrType = WPS_ENCRYPT_NONE;
			}
			else if (m_cmbEncyption.GetCurSel() == 1 ) //WEP
			{
				m_pCredentials->encrType = WPS_ENCRYPT_WEP;
				m_pCredentials->wepIndex = 1;
			}			
		}
		else if (m_cmbKeyMgmt.GetCurSel() == 1 ) //WPA-PSK

		{
			strcpy(m_pCredentials->keyMgmt, "WPA-PSK");
		}
		else if (m_cmbKeyMgmt.GetCurSel() == 2 ) //WPA2-PSK

		{
			strcpy(m_pCredentials->keyMgmt, "WPA2-PSK");
		}
		else
		{
			strcpy(m_pCredentials->keyMgmt, "WPA-PSK WPA2-PSK");
		}

		//encryption
		if (m_cmbKeyMgmt.GetCurSel() != 0 ) //!OPEN
		{
			if (m_cmbEncyption.GetCurSel() == 0 ) //TKIP
			{
				m_pCredentials->encrType = WPS_ENCRYPT_TKIP;
			}
			else if (m_cmbEncyption.GetCurSel() == 1 ) //AES
			{
				m_pCredentials->encrType = WPS_ENCRYPT_AES;
			}
			else //both
			{
				m_pCredentials->encrType = (WPS_ENCRYPT_TKIP | WPS_ENCRYPT_AES);
			}
		}

		//networt key
		CString strKey;
		m_edtKey.GetWindowTextW(strKey);
		strKey.TrimLeft();
		strKey.TrimRight();
		memset(m_pCredentials->nwKey, 0, sizeof(m_pCredentials->nwKey));
		if(m_pCredentials->encrType != WPS_ENCRYPT_NONE) {
			wtoa(strKey.GetBuffer(0), m_pCredentials->nwKey, 64);
			if (strlen(m_pCredentials->nwKey) == 0)
			{
				AfxMessageBox(_T("No Key defined."), MB_ICONSTOP);
				bContinue = FALSE;
			}
		}
	}
	else
	{
		wps_generate_cred(m_pCredentials);
	}
	if (bContinue)
		OnOK();
}

void CDlgWPSCredentials::OnBnClickedButtonCancel()
{
	m_bRandom = TRUE;
	OnCancel();
}

void CDlgWPSCredentials::OnBnClickedCheckRandomCredentials()
{
	UpdateData(TRUE);
	ShowHideControls();
}

void CDlgWPSCredentials::ShowHideControls()
{
	CWnd *pWnd = 0;
	int i;
	for (i=0;i<5;i++)
	{
		if (i==0)
		{
			//SSID
			pWnd = GetDlgItem(IDC_EDIT_SSID);

		}
		else if (i==1)

		{
			//key management
			pWnd = GetDlgItem(IDC_COMBO_KEY_MGMT);
		}
		else if (i==2)

		{
			//encyption
			pWnd = GetDlgItem(IDC_COMBO_ENCRYPTION);
		}
		else if (i==3)
		{
			//key
			pWnd = GetDlgItem(IDC_EDIT_KEY);
		}
		else 
		{
			//pin
			pWnd = GetDlgItem(IDC_EDIT_PIN_GENERATED);
		}
		if (pWnd)
		{
			if (m_bRandom)
				pWnd->EnableWindow(FALSE);
			else
				pWnd->EnableWindow(TRUE);
		}
	}
}

BOOL CDlgWPSCredentials::IsRandomCredentials()
{
	return m_bRandom;
}

wps_credentials* CDlgWPSCredentials::GetCredentials()
{
	return m_pCredentials;
}

void CDlgWPSCredentials::OnBnClickedButtonGeneratePin()
{
	CWaitCursor c;
	wps_generate_pin(&m_pin[0]);
	CString strPin(m_pin);
	m_edtPinValue.SetWindowTextW(strPin);
	UpdateData(FALSE);

}

void CDlgWPSCredentials::OnBnClickedButtonTestPin()
{
	CWaitCursor c;

    CString strPin;
	m_edtPinValue.GetWindowTextW(strPin);
	strPin.TrimLeft();
	strPin.TrimRight();
	wtoa(strPin.GetBuffer(0), m_pin, 79);

	if (strlen(m_pin) == 0)
	{
		AfxMessageBox(_T("No pin defined."), MB_ICONSTOP);
	}
	else
	{
		unsigned long pin = strtoul(m_pin,NULL,0);
		if (wps_validate_checksum(pin))
		{
			AfxMessageBox(_T("Checksum Okay."), MB_OK);
		}
		else
		{
			AfxMessageBox(_T("Checksum Error."), MB_ICONSTOP);
		}
	}
}

void CDlgWPSCredentials::OnDestroy()
{
	CDialog::OnDestroy();
}
void CDlgWPSCredentials::OnCbnSelchangeComboKeyMgmt()
{
	int index = 0;

	m_cmbEncyption.ResetContent();

	if (m_cmbKeyMgmt.GetCurSel() == 0 ) //OPEN
	{
		m_cmbEncyption.InsertString(index++, _T("None"));
		m_cmbEncyption.InsertString(index++, _T("WEP"));
		m_cmbEncyption.SetCurSel(1); //WEP
		m_edtKey.EnableWindow(TRUE);
	}
	else 
	{
		m_cmbEncyption.InsertString(index++, _T("TKIP"));
		m_cmbEncyption.InsertString(index++, _T("AES"));
		m_cmbEncyption.InsertString(index++, _T("TKIP and AES"));
		m_cmbEncyption.SetCurSel(0); //TKIP
		m_edtKey.EnableWindow(TRUE);
	}
	
}

void CDlgWPSCredentials::OnCbnSelchangeComboEncryption()
{
	if (m_cmbKeyMgmt.GetCurSel() == 0 && 
		m_cmbEncyption.GetCurSel() == 0) {

		//OPEN-None selected
		if (IDNO == MessageBox(_T("You have selected the encryption option to be 'None'. The configured network will be insecure. Do you really wish to select 'None' for the encryption option?"),
			_T("WARNING"),MB_YESNO|MB_ICONWARNING)) {
			m_cmbEncyption.SetCurSel(1);
			m_edtKey.EnableWindow(TRUE);
		}
		else
			m_edtKey.EnableWindow(FALSE);
	}
}
