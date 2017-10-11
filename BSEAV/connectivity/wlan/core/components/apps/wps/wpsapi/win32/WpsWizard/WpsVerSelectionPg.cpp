// VerSelectionPg.cpp : implementation file
//

#include "stdafx.h"
#include "WpsWizard.h"
#include "WpsVerSelectionPg.h"


// CWpsVerSelectionPg dialog

// IMPLEMENT_DYNAMIC(CWpsVerSelectionPg, CWizardPropPg)

CWpsVerSelectionPg::CWpsVerSelectionPg()
	: CWizardPropPg(CWpsVerSelectionPg::IDD)
{

}

CWpsVerSelectionPg::~CWpsVerSelectionPg()
{
}

void CWpsVerSelectionPg::DoDataExchange(CDataExchange* pDX)
{
	CWizardPropPg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CWpsVerSelectionPg, CWizardPropPg)
	ON_BN_CLICKED(IDC_WPS_V1, &CWpsVerSelectionPg::OnBnClickedWpsV1)
	ON_BN_CLICKED(IDC_WPS_V2, &CWpsVerSelectionPg::OnBnClickedWpsV2)
END_MESSAGE_MAP()


BOOL CWpsVerSelectionPg::OnInitDialog()
{
	CWizardPropPg::OnInitDialog();

	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

//	SetLabel(IDC_WPS_V1, IDS_WPS_V1);
//	SetLabel(IDC_WPS_V2, IDS_WPS_V2);
//	SetLabel(IDC_GROUP_SELECT_MODE, IDS_SELECT_WPS_VER);

	// Select WPS Ver 2 by default
	OnBnClickedWpsV2();

	return TRUE;
}

LRESULT CWpsVerSelectionPg::OnWizardNext()
{
	m_pNextPage = &m_pWizardSheet->m_WpsRoleSelectionPg;

	return CWizardPropPg::OnWizardNext(this);
}

void CWpsVerSelectionPg::OnBnClickedWpsV1()
{
	((CButton *)GetDlgItem(IDC_WPS_V1))->SetCheck(BST_CHECKED);
	m_WpsManager.m_bWpsV2 = FALSE;
}

void CWpsVerSelectionPg::OnBnClickedWpsV2()
{
	((CButton *)GetDlgItem(IDC_WPS_V2))->SetCheck(BST_CHECKED);

	m_WpsManager.m_bWpsV2 = TRUE;
}
