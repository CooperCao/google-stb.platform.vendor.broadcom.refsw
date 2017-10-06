/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: PinModePg.cpp,v 1.9 2009/10/17 00:11:03 Exp $
*/
// PinModePg.cpp : implementation file
//

#include "stdafx.h"
#include "WpsWizard.h"
#include "PinModePg.h"


// CPinModePg dialog

//IMPLEMENT_DYNAMIC(CPinModePg, CWizardPropPg)

CPinModePg::CPinModePg()
	: CWizardPropPg(CPinModePg::IDD)
{

}

CPinModePg::~CPinModePg()
{
}

void CPinModePg::DoDataExchange(CDataExchange* pDX)
{
	CWizardPropPg::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_REG_PINCODE, m_strRegistrarPin);
	DDX_Text(pDX, IDC_ENROLLEE_PINCODE, m_strEnrolleePin);
	DDX_Control(pDX, IDC_STATIC_PIN_DESCRIPTOIN, m_staticPinDesc);
	DDX_Control(pDX, IDC_STATIC_PIN, m_staticPin);
	DDX_Control(pDX, IDC_ENROLLEE_PINCODE, m_EnrolleePin);
	DDX_Control(pDX, IDC_REG_PINCODE, m_RegPin);
	DDX_Control(pDX, IDC_INST_CABLE_CONNECT_AP, m_staticInstConnAdmin);
}


BEGIN_MESSAGE_MAP(CPinModePg, CWizardPropPg)
	ON_BN_CLICKED(IDC_STAER_GET_SETTINGS, &CPinModePg::OnBnClickedStaerGetSettings)
	ON_BN_CLICKED(IDC_STAENR_GET_SETTINGS, &CPinModePg::OnBnClickedStaenrGetSettings)
END_MESSAGE_MAP()

BOOL CPinModePg::OnInitDialog()
{
	CWizardPropPg::OnInitDialog();

	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

	SetLabel(IDC_GROUP_PIN_MODE, IDS_PIN);
	SetLabel(IDC_STATIC_PIN, IDS_PIN_SHORT);
	SetLabel(IDC_STAENR_GET_SETTINGS, IDS_MODE_STAENR_GET_SETTINGS);
	SetLabel(IDC_STAER_GET_SETTINGS, IDS_MODE_STAER_GET_SETTINGS);
	SetLabel(IDC_INST_CABLE_CONNECT_AP, IDS_INST_CABLE_CONNECT_AP);

	OnBnClickedStaenrGetSettings();

	return TRUE;
}

LRESULT CPinModePg::OnWizardNext()
{
	UpdateData();

	// Set PIN code
	if(m_eSelectedWpsMode == STA_REG_JOIN_NW)
		m_WpsManager.SetPinCode(LPCTSTR(m_strRegistrarPin));  // STR ER joins network
	else
		m_WpsManager.SetPinCode(LPCTSTR(m_strEnrolleePin));  // STA Enr joins network

	m_WpsManager.SetWpsMode(m_eSelectedWpsMode);

	if(!ValidateControls())
		return -1;  // Prevent the page from advancing 

	m_pNextPage = &(m_pWizardSheet->m_ConfigStatusPg);

	return CWizardPropPg::OnWizardNext(this);
}

void CPinModePg::OnBnClickedStaerGetSettings()
{
	// Mode of STA External Registrar joining network

	((CButton *)(GetDlgItem(IDC_STAENR_GET_SETTINGS)))->SetCheck(BST_UNCHECKED);
	((CButton *)(GetDlgItem(IDC_STAER_GET_SETTINGS)))->SetCheck(BST_CHECKED);
	m_staticPinDesc.SetWPSWWindowText(IDS_STAER_GET_SETTINGS_DESC);
	m_EnrolleePin.ShowWindow(SW_HIDE);
	m_RegPin.ShowWindow(SW_SHOW);

	m_eSelectedWpsMode = STA_REG_JOIN_NW;
}

void CPinModePg::OnBnClickedStaenrGetSettings()
{
	// Mode of STA Enrollee joining network

	((CButton *)(GetDlgItem(IDC_STAER_GET_SETTINGS)))->SetCheck(BST_UNCHECKED);
	((CButton *)(GetDlgItem(IDC_STAENR_GET_SETTINGS)))->SetCheck(BST_CHECKED);
	m_staticPinDesc.SetWPSWWindowText(IDS_STAENR_GET_SETTINGS_DESC);
	m_EnrolleePin.ShowWindow(SW_SHOW);
	m_RegPin.ShowWindow(SW_HIDE);

	m_eSelectedWpsMode = STA_ENR_JOIN_NW_PIN;

	// Generate a PIN for user automatically and don't allow the user to change
	char szPin[WPS_PIN_TOTAL_DIGIT+1] = { 0 };
	if(wps_api_generate_pin(szPin, sizeof(szPin)))
		m_strEnrolleePin = szPin;
	else
		m_strEnrolleePin = _T("12345670");  // Default PIN although wps_generate_pin should never fail

	m_EnrolleePin.SetWPSWWindowText(m_strEnrolleePin);  // Show the generated PIN code
}

BOOL CPinModePg::ValidateControls()
{
	if(!ValidateWpsPin((m_eSelectedWpsMode == STA_ENR_JOIN_NW_PIN)? m_strEnrolleePin : m_strRegistrarPin))
	{
		CString strText;
		strText.LoadString(IDS_BAD_WPS_PIN);
		MessageBox(strText, NULL, MB_ICONERROR | MB_OK);
		return FALSE;
	}
	return TRUE;
}
