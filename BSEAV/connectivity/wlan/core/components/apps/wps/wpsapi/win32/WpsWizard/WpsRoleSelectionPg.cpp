/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: WpsRoleSelectionPg.cpp,v 1.4 2008/10/27 20:35:48 Exp $
 */
// WpsRoleSelectionPg.cpp : implementation file
//

#include "stdafx.h"
#include "WpsWizard.h"
#include "WpsRoleSelectionPg.h"


// CWpsRoleSelectionPg dialog

CWpsRoleSelectionPg::CWpsRoleSelectionPg()
	: CWizardPropPg(CWpsRoleSelectionPg::IDD)
{
	m_bJoinNW = TRUE;
}

CWpsRoleSelectionPg::~CWpsRoleSelectionPg()
{
}

void CWpsRoleSelectionPg::DoDataExchange(CDataExchange* pDX)
{
	CWizardPropPg::DoDataExchange(pDX);

	DDX_Radio(pDX, IDC_ROLE_JOIN_AP, (int&)m_eWpsRole);
	DDX_Check(pDX, IDC_SEL_AUTO_CONNECT, m_bAutoConnect);
	DDX_Control(pDX, IDC_STATIC_AUTO_SEL_DESC, m_staticAutoSelDesc);
}


BEGIN_MESSAGE_MAP(CWpsRoleSelectionPg, CWizardPropPg)
	ON_BN_CLICKED(IDC_ROLE_JOIN_AP, &CWpsRoleSelectionPg::OnBnClickedRoleJoinAp)
	ON_BN_CLICKED(IDC_ROLE_CONFIG_AP, &CWpsRoleSelectionPg::OnBnClickedRoleConfigAp)
END_MESSAGE_MAP()

BOOL CWpsRoleSelectionPg::OnInitDialog()
{
	CWizardPropPg::OnInitDialog();

	SetLabel(IDC_ROLE_JOIN_AP, IDS_JOIN_WIRELESS_NETWORK);
	SetLabel(IDC_ROLE_CONFIG_AP, IDS_CONFIG_WIRELESS_NETWORK);
	SetLabel(IDC_GROUP_SELECT_MODE, IDS_SELECT_ROLE);
	SetLabel(IDC_SEL_AUTO_CONNECT, IDS_AUTO_SEL_NETWORK);
	SetLabel(IDC_STATIC_AUTO_SEL_DESC, IDS_AUTO_SEL_DESC);

	OnBnClickedRoleConfigAp();

	return TRUE;
}

void CWpsRoleSelectionPg::OnBnClickedRoleJoinAp()
{
	((CButton *)(GetDlgItem(IDC_ROLE_JOIN_AP)))->SetCheck(BST_CHECKED);
	((CButton *)(GetDlgItem(IDC_ROLE_CONFIG_AP)))->SetCheck(BST_UNCHECKED);

	m_bJoinNW = TRUE;

	UpdateData();
}

void CWpsRoleSelectionPg::OnBnClickedRoleConfigAp()
{
	((CButton *)(GetDlgItem(IDC_ROLE_JOIN_AP)))->SetCheck(BST_UNCHECKED);
	((CButton *)(GetDlgItem(IDC_ROLE_CONFIG_AP)))->SetCheck(BST_CHECKED);

	m_bJoinNW = FALSE;
	m_eSelectedWpsMode = STA_REG_CONFIG_NW;

	UpdateData();
}

LRESULT CWpsRoleSelectionPg::OnWizardNext()
{
	UpdateData();

	if(m_bJoinNW)
	{
		// Go to mode selection page to select which mode to join network
		m_pNextPage = &(m_pWizardSheet->m_WpsModeSelectionPg);
	}
	else 
	{
		m_pNextPage = &(m_pWizardSheet->m_ConfigApPg);
		m_WpsManager.SetWpsMode(m_eSelectedWpsMode);
	}

	return CWizardPropPg::OnWizardNext(this);
}

BOOL CWpsRoleSelectionPg::OnSetActive()
{
	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

	return CWizardPropPg::OnSetActive();
}
