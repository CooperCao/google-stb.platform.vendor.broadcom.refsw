/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: SelectAPDlg.cpp,v 1.7 2008/10/06 17:03:05 Exp $
*/
// SelectAPDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WpsWizard.h"
#include "SelectAPDlg.h"
#include "WpsWizardUtil.h"

// CSelectAPDlg dialog
#define APLIST_NUM_COLUMNS		2

IMPLEMENT_DYNAMIC(CSelectAPDlg, CDialog)

CSelectAPDlg::CSelectAPDlg(CWnd* pParent /*=NULL*/)
: CDialog(CSelectAPDlg::IDD, pParent)
{
	m_bShowAuthAPOnly = FALSE;
}

CSelectAPDlg::CSelectAPDlg(CWnd* pParent, CWpsManager *pWpsManager)
: CDialog(CSelectAPDlg::IDD, pParent), m_pWpsManager(pWpsManager)
{
	m_bShowAuthAPOnly = FALSE;
}

CSelectAPDlg::~CSelectAPDlg()
{
}

void CSelectAPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_AP_LIST, m_APList);
	DDX_Control(pDX, IDC_LABEL_SEL_NETWORK, m_LabelSelNetwork);
	DDX_Control(pDX, IDC_SHOW_AUTH_AP, m_ShowAuthAPs);
}


BEGIN_MESSAGE_MAP(CSelectAPDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CSelectAPDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_SCAN_NETWORK, &CSelectAPDlg::OnBnClickedScanNetwork)
	ON_BN_CLICKED(IDC_SHOW_AUTH_AP, &CSelectAPDlg::OnBnClickedShowAuthAp)
END_MESSAGE_MAP()


// CSelectAPDlg message handlers

BOOL CSelectAPDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Insert columns
	static struct _ST_COL_HEADING {
		int nStrID;
		int nWidth;
	} mapColHeading[] =
	{
		{IDS_NETWORK_NAME, 150},
		{IDS_MAC_ADDRESS, 120},
		{IDS_BAND, 80},
		{IDS_WPS_VERSION, 80}
	};

	int i=0;
	for(; i<sizeof(mapColHeading)/sizeof(mapColHeading[0]); i++)
	{
		CString strText;
		strText.LoadString(mapColHeading[i].nStrID);
		m_APList.InsertColumn(i, strText, 0, mapColHeading[i].nWidth);
	}

	SetLabel(m_hWnd, IDC_SCAN_NETWORK, IDS_REFRESH_NETWORK);

	PopulateNetworkList(m_pWpsManager->GetWpsAPs());

	CString strText;
	strText.LoadString(IDS_NETWORK_SELECTION);
	SetWindowText(strText);  // Set window title

	strText.LoadString(IDS_PLEASE_SELECT_NETWORK);
	m_LabelSelNetwork.SetWindowText(strText);

	strText.LoadString(IDS_SHOW_AUTH_AP_ONLY);
	m_ShowAuthAPs.SetWindowText(strText);

	// Set default selection
	m_APList.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED); 

	if (!m_pWpsManager->m_bWpsV2)
		m_ShowAuthAPs.ShowWindow(SW_HIDE);

	return TRUE;  
}

void CSelectAPDlg::OnBnClickedOk()
{
	int nSel = m_APList.GetNextItem(-1, LVNI_SELECTED);
	if(nSel == LB_ERR)
		return;

	m_pSelectedAP = (LPSTAP)m_APList.GetItemData(nSel);
	
	// When configuring AP, according to testbed, we need to ask the user to conform before 
	// overwriting the AP's existing network settings
	if(m_pWpsManager->GetWpsMode() == STA_REG_CONFIG_NW && m_pSelectedAP->is_configured)
	{
		CString strText;
		strText.LoadString(IDS_AP_CONFIGURED_CONFIRM_OVERWRITE);
		if(MessageBox(strText, NULL, MB_YESNO | MB_ICONWARNING) == IDNO)
			return;
	}

	m_strSelectedAP = m_pSelectedAP->ssid;

	OnOK();
}

void CSelectAPDlg::PopulateNetworkList(const VEC_AP& vecWpsAPs)
{
	if(!m_pWpsManager)
		return;

	USES_CONVERSION;

	LV_ITEM lvi;
	
	// Cleanup the existing list
	m_APList.DeleteAllItems();

	// insert items
	for(int i=0; i < (int)vecWpsAPs.size(); i++)
	{
		LPSTAP pAP = (LPSTAP)(vecWpsAPs[i]);
		if(pAP)
		{
/*
			if (m_bShowAuthAPOnly)
			{
				// Only show APs that authorize
				if (!m_pWpsManager->DoesAPAuthorize(pAP))
					continue;
			}
*/
			lvi.mask = LVIF_TEXT;
			lvi.iItem = i;
			lvi.iSubItem = 0;
			lvi.pszText = A2T(pAP->ssid);  // Set SSID

			m_APList.InsertItem(&lvi);
			
			m_APList.SetItemData(i, (DWORD_PTR)pAP);
			
			// Set item text for additional columns
			int nCol = 0;

			// format MAC address to be a string
			TCHAR szMacAddr[64] = { _T('\0') };
			uint8 *mac = pAP->bssid;
			_stprintf(szMacAddr, _T("%02X:%02X:%02X:%02X:%02X:%02X"), (BYTE)mac[0], (BYTE)mac[1], (BYTE)mac[2], (BYTE)mac[3], (BYTE)mac[4], (BYTE)mac[5]);
			m_APList.SetItemText(i, ++nCol, szMacAddr);  // Set MAC address

			// Insert band
			CString strText;
			strText.LoadString(pAP->band == WPSAPI_BAND_2G? IDS_BAND_2G : IDS_BAND_5G);
			m_APList.SetItemText(i, ++nCol, strText);

			// Insert WPS version
			strText.LoadString(pAP->band == WPSAPI_BAND_2G? IDS_BAND_2G : IDS_BAND_5G);
			if (pAP->bVer2)
				m_APList.SetItemText(i, ++nCol, _T("2"));
			else
				m_APList.SetItemText(i, ++nCol, _T("1"));
		}
	}
}

void CSelectAPDlg::OnBnClickedScanNetwork()
{
	OnScanStart();

	// Use 10 seconds as time-out value when refreshing network
	m_pWpsManager->SearchWpsAPs(10, 
								PBC_MODE(m_pWpsManager->GetWpsMode()),
								false,
								m_bShowAuthAPOnly);

	OnScanStop();

	PopulateNetworkList(m_pWpsManager->GetWpsAPs());
}

void CSelectAPDlg::OnScanStart()
{
	SetLabel(m_hWnd, IDC_SCAN_NETWORK, IDS_REFRESHING);
	GetDlgItem(IDC_SCAN_NETWORK)->EnableWindow(FALSE);
	GetDlgItem(IDOK)->EnableWindow(FALSE);
}

void CSelectAPDlg::OnScanStop()
{
	SetLabel(m_hWnd, IDC_SCAN_NETWORK, IDS_REFRESH_NETWORK);
	GetDlgItem(IDC_SCAN_NETWORK)->EnableWindow();
	GetDlgItem(IDOK)->EnableWindow();
}

void CSelectAPDlg::OnBnClickedShowAuthAp()
{
	m_bShowAuthAPOnly = (m_ShowAuthAPs.GetCheck() == BST_CHECKED);

	PopulateNetworkList(m_pWpsManager->GetWpsAPs());
}
