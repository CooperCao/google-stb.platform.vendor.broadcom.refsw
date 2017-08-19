/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: WpsModeSelectionPg.cpp,v 1.8 2008/10/27 20:35:48 Exp $
*/
// WpsModeSelectionPg.cpp : implementation file
//

#include "stdafx.h"
#include "WpsWizard.h"
#include "WpsModeSelectionPg.h"

// CWpsModeSelectionPg dialog

//IMPLEMENT_DYNAMIC(CWpsModeSelectionPg, CPropertyPage)

CWpsModeSelectionPg::CWpsModeSelectionPg()
	: CWizardPropPg(CWpsModeSelectionPg::IDD)
{
	m_eSelectedWpsMode = STA_ENR_JOIN_NW_PIN;
}

CWpsModeSelectionPg::~CWpsModeSelectionPg()
{
}

void CWpsModeSelectionPg::DoDataExchange(CDataExchange* pDX)
{
	CWizardPropPg::DoDataExchange(pDX);

	DDX_Radio(pDX, IDC_MODE_PBC, (int&)m_eSelectedWpsMode);
	DDX_Control(pDX, IDC_STATIC_SEL_MODE_DESC, m_staticSelModeText);
}


BEGIN_MESSAGE_MAP(CWpsModeSelectionPg, CWizardPropPg)
	ON_BN_CLICKED(IDC_MODE_PBC, &CWpsModeSelectionPg::OnBnClickedModePbc)
	ON_BN_CLICKED(IDC_MODE_PIN, &CWpsModeSelectionPg::OnBnClickedModePin)
END_MESSAGE_MAP()


BOOL CWpsModeSelectionPg::OnInitDialog()
{
	CWizardPropPg::OnInitDialog();

	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

	SetLabel(IDC_MODE_PBC, IDS_PBC);
	SetLabel(IDC_MODE_PIN, IDS_PIN);
	SetLabel(IDC_GROUP_SELECT_MODE, IDS_SELECT_WPS_MODE);
	SetLabel(IDC_STATIC_SEL_MODE_DESC, IDS_SELECT_WPS_MODE_DESC);

	// Set PBC mode by default
	OnBnClickedModePbc();

	return TRUE;
}

LRESULT CWpsModeSelectionPg::OnWizardNext()
{
	if(m_eSelectedWpsMode == STA_ENR_JOIN_NW_PBC)
		m_pNextPage = &(m_pWizardSheet->m_PbcModePg);
	else 
		m_pNextPage = &(m_pWizardSheet->m_PinModePg);

	m_WpsManager.SetWpsMode(m_eSelectedWpsMode);

	return CWizardPropPg::OnWizardNext(this);
}

void CWpsModeSelectionPg::OnBnClickedModePbc()
{
	((CButton *)(GetDlgItem(IDC_MODE_PBC)))->SetCheck(BST_CHECKED);
	((CButton *)(GetDlgItem(IDC_MODE_PIN)))->SetCheck(BST_UNCHECKED);

	m_eSelectedWpsMode = STA_ENR_JOIN_NW_PBC;
}

void CWpsModeSelectionPg::OnBnClickedModePin()
{
	((CButton *)(GetDlgItem(IDC_MODE_PBC)))->SetCheck(BST_UNCHECKED);
	((CButton *)(GetDlgItem(IDC_MODE_PIN)))->SetCheck(BST_CHECKED);

	m_eSelectedWpsMode = STA_ENR_JOIN_NW_PIN;
}
