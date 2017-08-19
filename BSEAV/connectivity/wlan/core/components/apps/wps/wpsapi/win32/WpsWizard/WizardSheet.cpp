/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: WizardSheet.cpp,v 1.8 2009/02/17 18:22:20 Exp $
*/
// WizardSheet.cpp : implementation file
//

#include "stdafx.h"
#include "WpsWizard.h"
#include "WizardSheet.h"

extern HWND g_hWnd;

// CWizardSheet

IMPLEMENT_DYNAMIC(CWizardSheet, CPropertySheet)

CWizardSheet::CWizardSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
	, m_strPin(_T(""))
{
	m_pWpsManager = new CWpsManager();

	// Adding property pages
	AddPage(&m_WelcomePg);
//	AddPage(&m_WpsVerSelectionPg);
	AddPage(&m_WpsRoleSelectionPg);
	AddPage(&m_ConfigApPg);
	AddPage(&m_ConfigStatusPg);
	AddPage(&m_PbcModePg);
	AddPage(&m_PinModePg);
	AddPage(&m_WpsModeSelectionPg);
	AddPage(&m_WpsFinishPg);

	m_psh.dwFlags &= ~PSH_HASHELP;

	// By default WPS will start to run 
	m_bToRunWps = TRUE;
}

CWizardSheet::CWizardSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	m_pWpsManager = new CWpsManager();
}

CWizardSheet::~CWizardSheet()
{
	if(m_pWpsManager)
	{
		delete m_pWpsManager;
		m_pWpsManager = NULL;
	}
}


BEGIN_MESSAGE_MAP(CWizardSheet, CPropertySheet)
	ON_MESSAGE(WM_WPS_STATUS, OnWpsStatusUpdate)
END_MESSAGE_MAP()


// CWizardSheet message handlers

BOOL CWizardSheet::OnInitDialog()
{
	g_hWnd = m_hWnd;

	BOOL bResult = CPropertySheet::OnInitDialog();

	return bResult;
}

CString CWizardSheet::GetPinCode() const
{
	return m_strPin;
}

void CWizardSheet::SetPinCode(const CString& strPin)
{
	m_strPin = strPin;
}

CWpsManager* CWizardSheet::GetWpsManager() const
{
	return m_pWpsManager;
}

LRESULT CWizardSheet::OnWpsStatusUpdate(WPARAM wParam, LPARAM lParam)
{
	// Dispatch the message to the active child page
	CPropertyPage *pActive = GetActivePage();
	if(pActive)
		::PostMessage(pActive->m_hWnd, WM_WPS_STATUS, wParam, lParam);
	return 0;
}

BOOL CWizardSheet::SetPageTitle(int nPage, LPCTSTR pszText) 
{ 
	CPropertyPage *pPage = GetPage(nPage);
	CTabCtrl* pTab = GetTabControl();
	if (GetActivePage() == pPage) 
		SetWindowText(pszText); //If its the active page

	//If its not the active page, just set the tab item
	TC_ITEM ti;
	ti.mask = TCIF_TEXT;
	ti.pszText = const_cast<LPTSTR>(pszText);
	VERIFY (pTab->SetItem(nPage, &ti));

	return TRUE;
}
