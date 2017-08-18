/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: SelectAPDlg.cpp,v 1.5 2008/07/17 17:48:44 Exp $
*/
// SelectAPDlg.cpp : implementation file
//

#include "stdafx.h"
#include "wps_test_gui.h"
#include "SelectAPDlg.h"


// CSelectAPDlg dialog

IMPLEMENT_DYNAMIC(CSelectAPDlg, CDialog)

CSelectAPDlg::CSelectAPDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectAPDlg::IDD, pParent)
{

}

CSelectAPDlg::CSelectAPDlg(CWnd* pParent, VEC_AP& vecWpsAPs)
	: CDialog(CSelectAPDlg::IDD, pParent), m_vecWpsAPs(vecWpsAPs)
{
}

CSelectAPDlg::~CSelectAPDlg()
{
}

void CSelectAPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_AP_LIST, m_APList);
}


BEGIN_MESSAGE_MAP(CSelectAPDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CSelectAPDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSelectAPDlg message handlers

BOOL CSelectAPDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	for(int i=0; i<m_vecWpsAPs.size(); i++)
	{
		LPSTAP pAP = (LPSTAP)m_vecWpsAPs[i];
		if(pAP)
		{
			// format MAC address to be a string
			TCHAR szMacAddr[64] = { _T('\0') };
			uint8 *mac = pAP->bssid;
			_stprintf(szMacAddr, _T("%02X:%02X:%02X:%02X:%02X:%02X"), (BYTE)mac[0], (BYTE)mac[1], (BYTE)mac[2], (BYTE)mac[3], (BYTE)mac[4], (BYTE)mac[5]);

			// Include both ssid and MAC address to display as one AP can have multiple MAC interfaces with same SSID
			CString strAP;
			strAP.Format(_T("%s (%s)"), CString(pAP->ssid), CString(szMacAddr));
			m_APList.InsertString(i, strAP);
			m_APList.SetItemDataPtr(i, (void *)pAP);
		}
	}
	m_APList.SetCurSel(0);

	return TRUE;  
}

void CSelectAPDlg::OnBnClickedOk()
{
	int nSel = m_APList.GetCurSel();
	if(nSel == LB_ERR)
		return;

	m_APList.GetText(nSel, m_strSelectedAP);
	m_pSelectedAP = (LPSTAP)m_APList.GetItemDataPtr(nSel);

	OnOK();
}
