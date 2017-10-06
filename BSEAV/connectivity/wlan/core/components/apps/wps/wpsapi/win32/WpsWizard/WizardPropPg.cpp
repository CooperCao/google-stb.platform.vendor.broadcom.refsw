/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: WizardPropPg.cpp,v 1.7 2008/10/27 20:35:48 Exp $
*/
// WizardPropPg.cpp : implementation file
//

#include "stdafx.h"
#include "WpsWizard.h"
#include "WizardPropPg.h"
#include "FontSize.h"

// Initialization of static class members
CWizardSheet* CWizardPropPg::m_pWizardSheet = NULL;
CWpsManager CWizardPropPg::m_WpsManager;
BOOL CWizardPropPg::m_bAutoConnect = TRUE;
eWPS_MODE CWizardPropPg::m_eSelectedWpsMode = STA_ENR_JOIN_NW_PBC;

//IMPLEMENT_DYNAMIC(CWizardPropPg, CPropertyPage)

CWizardPropPg::CWizardPropPg(UINT nIDTemplate, UINT nIDCaption, UINT nIDHeaderTitle, UINT nIDHeaderSubTitle)
	: CPropertyPage(nIDTemplate)
{
	Init();
}

CWizardPropPg::~CWizardPropPg()
{
}

void CWizardPropPg::Init()
{
	// Remove wizard "Help" button 
	m_psp.dwFlags &= ~(PSP_HASHELP);

	m_pNextPage = NULL;
	m_pPrevPage = NULL;
}

void CWizardPropPg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogStep1)
	DDX_Control(pDX, IDC_COMPANY_LOGO, m_staticCompanyLogoBitmap);
	DDX_Control(pDX, IDC_STATIC_SECUREEZ_BITMAP, m_staticSecureEZLogo);
	DDX_Control(pDX, IDC_STATIC_RIGHTBK, m_staticRightBk);
	DDX_Control(pDX, IDC_STATIC_LEFTBK, m_staticLeftBk);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizardPropPg, CPropertyPage)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

LRESULT CWizardPropPg::OnWizardNext() 
{
	LRESULT ret = -1;
	if (m_pNextPage)
		m_pWizardSheet->SetActivePage(m_pNextPage);

	return ret;
}

LRESULT CWizardPropPg::OnWizardNext(CWizardPropPg* pPrevPage)
{
	if (m_pNextPage)
		m_pNextPage->m_pPrevPage = pPrevPage;

	return CWizardPropPg::OnWizardNext();
}

LRESULT CWizardPropPg::OnWizardBack() 
{
	LRESULT ret = -1;
	if(m_pPrevPage)
		m_pWizardSheet->SetActivePage(m_pPrevPage);
	
	return ret;
}

BOOL CWizardPropPg::OnSetActive() 
{
	CString strText;
	strText.LoadString(IDS_APP_CAPTION);

	m_pWizardSheet->SetPageTitle(m_pWizardSheet->GetPageIndex(this), (LPCTSTR)strText);

	return CPropertyPage::OnSetActive();
}

BOOL CWizardPropPg::OnKillActive() 
{
	return CPropertyPage::OnKillActive();
}

void CWizardPropPg::OnCancel() 
{

	CPropertyPage::OnCancel();
}

BOOL CWizardPropPg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_pWizardSheet = (CWizardSheet*)GetParent();

	m_staticRightBk.SetBackgroundColor( RGB(255,255,255) );
	m_staticLeftBk.SetBackgroundColor( RGB( 255, 255, 255 ) );

	m_staticSecureEZLogo.SetBkColor( RGB(255,255,255) );
	m_staticSecureEZLogo.SetTranspartColor( RGB(255,255,255) );
	m_staticSecureEZLogo.SetBitmapId(IDB_BITMAP_WPSLOGO_VERT);
	m_staticSecureEZLogo.SetXLeading( 3 );
	m_staticSecureEZLogo.SetYLeading( 10 );

	m_staticCompanyLogoBitmap.SetBkColor( RGB(255,255,255) );
	m_staticCompanyLogoBitmap.SetTranspartColor( RGB(255,255,255) );
	m_staticCompanyLogoBitmap.SetBitmapId( IDB_BITMAP_COMPANY_VT );
	m_staticCompanyLogoBitmap.SetXLeading( 3 );
	m_staticCompanyLogoBitmap.SetYLeading( 5 );

	// Create default font
	LOGFONT lf;
	GetFont()->GetLogFont(&lf);
	lf.lfHeight = GetFontHeight(10);
	_tcscpy(lf.lfFaceName, _T("Times Roman"));
	m_Font.CreateFontIndirect(&lf);	

	return TRUE;  // return TRUE unless you set the focus to a control
}

HBRUSH CWizardPropPg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	BOOL bBkFlag = FALSE;
	TCHAR szClassName[256] = { _T('\0') };
	::GetClassName(pWnd->m_hWnd, szClassName, sizeof(szClassName)/sizeof(TCHAR));
	if(_tcsicmp(szClassName, _T("Button")) == 0)
	{
		// Set background color to "WHITE" when for checkbox, radio button and group box
		if(pWnd->GetStyle() & (BS_CHECKBOX | BS_AUTOCHECKBOX | BS_AUTORADIOBUTTON | BS_RADIOBUTTON | BS_GROUPBOX))
		{
			pDC->SetBkMode(TRANSPARENT);
			hbr = (HBRUSH)GetStockObject (WHITE_BRUSH);
		}
	}
	return hbr;
}

void CWizardPropPg::SetControlDefaultFont(UINT nCtrlID)
{
	CWnd *pCtrlWnd = GetDlgItem(nCtrlID);
	if(pCtrlWnd)
		pCtrlWnd->SetFont(&m_Font);
}

CWpsManager* CWizardPropPg::GetWpsManager() const
{
	if(m_pWizardSheet)
		return m_pWizardSheet->GetWpsManager();

	return NULL;
}

BOOL CWizardPropPg::SetLabel(UINT uControlId, UINT uStrId)
{
	BOOL bRet = FALSE;
	CWnd *pCtrlWnd = GetDlgItem(uControlId);
	if(pCtrlWnd)
	{
		pCtrlWnd->SetFont(&m_Font);

		CString strText;
		strText.LoadString(uStrId);
		pCtrlWnd->SetWindowText(strText);
		bRet = TRUE;
	}

	return bRet;
}

void CWizardPropPg::SetWizardButtons(DWORD dwFlags)
{
	((CWizardSheet*)GetParent())->SetWizardButtons(dwFlags);
}
