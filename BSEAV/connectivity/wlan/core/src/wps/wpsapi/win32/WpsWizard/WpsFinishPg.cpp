/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: WpsFinishPg.cpp,v 1.8 2009/10/09 19:32:52 Exp $
 */
// WpsFinishPg.cpp : implementation file
//

#include "stdafx.h"
#include "WpsWizard.h"
#include "WpsFinishPg.h"


// CWpsFinishPg dialog

CWpsFinishPg::CWpsFinishPg()
	: CWizardPropPg(CWpsFinishPg::IDD)
{

}

CWpsFinishPg::~CWpsFinishPg()
{
}

void CWpsFinishPg::DoDataExchange(CDataExchange* pDX)
{
	CWizardPropPg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_RESULT_DESC, m_ResultHeadline);
	DDX_Control(pDX, IDC_STATIC_SSID, m_NetworkSsid);
	DDX_Control(pDX, IDC_LABEL_SSID, m_labelSsid);
}


BEGIN_MESSAGE_MAP(CWpsFinishPg, CWizardPropPg)
END_MESSAGE_MAP()


// CWpsFinishPg message handlers
BOOL CWpsFinishPg::OnInitDialog()
{
	CWizardPropPg::OnInitDialog();

	CString strText;
	
	m_ResultHeadline.SetFont(_T("Times Roman"), 12);
	m_labelSsid.SetFont(_T("Times Roman"), 12);
	m_NetworkSsid.SetFont(_T("Times Roman"), 16);

	return TRUE;
}

LRESULT CWpsFinishPg::OnWizardNext()
{
	m_pNextPage = &(m_pWizardSheet->m_WelcomePg);
	
	// Reset m_bToRunWps to so we can start WPS when getting to ConfigStatus page 
	m_pWizardSheet->m_bToRunWps = TRUE;

	return CWizardPropPg::OnWizardNext(this);
}

BOOL CWpsFinishPg::OnSetActive()
{
	CString strText;

	if(m_WpsManager.m_bWpsSuccess)
	{
		// WPS succeeded
		m_ResultHeadline.SetWPSWWindowText(m_eSelectedWpsMode == STA_REG_CONFIG_NW? IDS_WPS_CONFIG_SUCCESS:IDS_WPS_JOIN_NW_SUCCESS);

		m_labelSsid.ShowWindow(SW_SHOW);
		m_NetworkSsid.ShowWindow(SW_SHOW);

		m_labelSsid.SetWPSWWindowText(IDS_SSID);
		if(m_WpsManager.GetWpsCred())
		{
			strText = m_WpsManager.GetWpsCred()->ssid;
			m_NetworkSsid.SetWPSWWindowText(strText);
		}

		// No need to show "Cancel" button in success case. The user will click "Finish" button to exit wizard
		m_pWizardSheet->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
		SetWizardButtons(PSWIZB_FINISH | PSWIZB_BACK);
	}
	else
	{
		// WPS failed
		m_ResultHeadline.SetWPSWWindowText(m_eSelectedWpsMode == STA_REG_CONFIG_NW? IDS_WPS_CONFIG_FAIL:IDS_WPS_JOIN_NW_FAIL);
		
		// Hide network setting labels and controls
		m_labelSsid.ShowWindow(SW_HIDE);
		m_NetworkSsid.ShowWindow(SW_HIDE);

		// Show property sheet "Cancel" button to allow user to exit wizard
		CString strText;
		strText.LoadString(IDS_CANCEL);
		m_pWizardSheet->GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
		m_pWizardSheet->GetDlgItem(IDCANCEL)->SetWindowText(strText);

		SetWizardButtons(PSWIZB_NEXT | PSWIZB_BACK);
	}

	return CWizardPropPg::OnSetActive();
}
