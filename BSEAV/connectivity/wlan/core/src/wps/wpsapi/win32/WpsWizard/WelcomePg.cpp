/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: WelcomePg.cpp,v 1.5 2008/09/15 19:06:15 Exp $
*/
// WelcomePg.cpp : implementation file
//

#include "stdafx.h"
#include "WpsWizard.h"
#include "WelcomePg.h"


// CWelcomePg dialog

CWelcomePg::CWelcomePg()
	: CWizardPropPg(CWelcomePg::IDD)
{

}

CWelcomePg::~CWelcomePg()
{
}

void CWelcomePg::DoDataExchange(CDataExchange* pDX)
{
	CWizardPropPg::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CWelcomeDlg)
	DDX_Control(pDX, IDC_STATIC_DESCRIPTION, m_staticDescriptionText);
	DDX_Control(pDX, IDC_STATIC_TITLE_TEXT, m_staticTitleText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWelcomePg, CWizardPropPg)
END_MESSAGE_MAP()


BOOL CWelcomePg::OnInitDialog()
{
	CWizardPropPg::OnInitDialog();

	CString strText;

	strText.LoadString( IDS_WELCOME_TITLE );
	m_staticTitleText.SetFont(_T( "Times Roman" ), 14 );
	m_staticTitleText.SetWindowText( strText );
	
	strText.LoadString( IDS_WELCOME_BODYTEXT );
	m_staticDescriptionText.SetWindowText( strText );

	return TRUE;  // return TRUE unless you set the focus to a control
}

LRESULT CWelcomePg::OnWizardNext()
{
	// Restore property sheet "Back" button
	m_pWizardSheet->GetDlgItem(ID_WIZBACK)->ShowWindow(SW_SHOW);

//	m_pNextPage = &(m_pWizardSheet->m_WpsVerSelectionPg);
	m_pNextPage = &(m_pWizardSheet->m_WpsRoleSelectionPg);

	return CWizardPropPg::OnWizardNext(this);
}

BOOL CWelcomePg::OnSetActive()
{
	// Hide property sheet "Back" button
	m_pWizardSheet->GetDlgItem(ID_WIZBACK)->ShowWindow(SW_HIDE);

	return CWizardPropPg::OnSetActive();
}
