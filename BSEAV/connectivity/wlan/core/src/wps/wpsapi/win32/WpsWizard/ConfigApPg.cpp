/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: ConfigApPg.cpp,v 1.4 2008/10/27 20:35:47 Exp $
 */
// ConfigApPg.cpp : implementation file
//

#include "stdafx.h"
#include "WpsWizard.h"
#include "ConfigApPg.h"
#include "ks.h"

// Make sure the key management string definition matches the schema defined in the WPS SDK API 
const char *KEY_MGMT_WPA2PSK			= "WPA2-PSK";
const char *KEY_MGMT_WPAPSK				= "WPA-PSK";
const char *KEY_MGMT_OPEN				= "OPEN";
const char *KEY_MGMT_WPAPSK_WPA2PSK		= "WPA-PSK WPA2-PSK";

const TCHAR PASSWORD_CHAR	= _T('*');

ST_SECURITYMETHOD mapSecurityMethods_WPS1[] = {
	{BRCM_SECURITY_WPA2PSK, IDS_SECURITY_WPA2PERSONAL, KEY_MGMT_WPA2PSK},
	{BRCM_SECURITY_WPAPSK, IDS_SECURITY_WPAPERSONAL, KEY_MGMT_WPAPSK},
	{BRCM_SECURITY_WEP, IDS_SECURITY_WEP, KEY_MGMT_OPEN},
	{BRCM_SECURITY_OPEN, IDS_SECURITY_OPEN, KEY_MGMT_OPEN}
};

ST_SECURITYMETHOD mapSecurityMethods_WPS2[] = {
	{BRCM_SECURITY_WPA2PSK, IDS_SECURITY_WPA2PERSONAL, KEY_MGMT_WPA2PSK},
	{BRCM_SECURITY_OPEN, IDS_SECURITY_OPEN, KEY_MGMT_OPEN}
};

ST_ENCRTYPE mapEncrTypes_WPS1[] = {
	{ENCRYPT_AES, IDS_AES},
	{ENCRYPT_TKIP, IDS_TKIP}
};

ST_ENCRTYPE mapEncrTypes_WPS2[] = {
	{ENCRYPT_AES, IDS_AES},
};

// CConfigApPg dialog

CConfigApPg::CConfigApPg()
	: CWizardPropPg(CConfigApPg::IDD)
	, m_strPinCode(_T(""))
{

}

CConfigApPg::~CConfigApPg()
{
}

void CConfigApPg::DoDataExchange(CDataExchange* pDX)
{
	CWizardPropPg::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SECURITY_METHOD, m_SecurityMethod);
	DDX_Control(pDX, IDC_ENCR_TYPE, m_EncrType);
	DDX_Text(pDX, IDC_AP_PIN, m_strPinCode);
	DDX_Control(pDX, IDC_NW_KEY, m_NetworkKey);
	DDX_Control(pDX, IDC_CHECK_HIDE_CHAR, m_HideChar);
	DDX_Control(pDX, IDC_GENERATE_DEFAULT, m_GenerateDefault);

	DDX_Control(pDX, IDC_STATIC_ENTER_AP_PIN, m_staticEnterApPin);
	DDX_Control(pDX, IDC_STATIC_SSID, m_staticSsid);
	DDX_Control(pDX, IDC_STATIC_SECURITY_METHOD, m_staticSecMethod);
	DDX_Control(pDX, IDC_STATIC_DATA_ENCR, m_staticDataEncr);
	DDX_Control(pDX, IDC_STATIC_NETWORK_KEY, m_staticNetworkKey);
	DDX_Control(pDX, IDC_STATIC_KEY_DESC, m_staticKeyDesc);
	DDX_Control(pDX, IDC_STATIC_AP_PIN_DESC, m_staticApPinDesc);
	DDX_Control(pDX, IDC_GEN_DEFAULT_DESC, m_staticGenDefaultDesc);
	
	DDX_Text(pDX, IDC_NW_KEY, m_strNetworkKey);
	DDX_Text(pDX, IDC_SSID, m_strSsid);
}


BEGIN_MESSAGE_MAP(CConfigApPg, CWizardPropPg)
	ON_BN_CLICKED(IDC_CHECK_HIDE_CHAR, &CConfigApPg::OnBnClickedCheckHideChar)
	ON_BN_CLICKED(IDC_GENERATE_DEFAULT, &CConfigApPg::OnBnClickedGenerateDefault)
	ON_CBN_SELCHANGE(IDC_SECURITY_METHOD, &CConfigApPg::OnCbnSelchangeSecurityMethod)
END_MESSAGE_MAP()


// CConfigApPg message handlers
BOOL CConfigApPg::OnInitDialog()
{
	CWizardPropPg::OnInitDialog();

	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

	SetLabel(IDC_STATIC_ENTER_AP_PIN, IDS_ENTER_AP_PIN);
	SetLabel(IDC_GROUP_NETWORK_SETTINGS, IDS_NETWORK_SETTINGS);
	SetLabel(IDC_STATIC_SSID, IDS_SSID);
	SetLabel(IDC_STATIC_SECURITY_METHOD, IDS_SECURITY_METHOD);
	SetLabel(IDC_STATIC_DATA_ENCR, IDS_DATA_ENCRYPTION);
	SetLabel(IDC_STATIC_NETWORK_KEY, IDS_NETWORK_KEY);
	SetLabel(IDC_CHECK_HIDE_CHAR, IDS_HIDE_CHAR);
	SetLabel(IDC_GENERATE_DEFAULT, IDS_GENERATE_DEFAULT);
	SetLabel(IDC_STATIC_AP_PIN_DESC, IDS_AP_PIN_DESC);

	return TRUE;
}

void CConfigApPg::InitConfigureAPControls()
{
	CString strLocal;
	int nIndex;
	
	// Fill securty method combobox
	m_mapSecurityMethods = mapSecurityMethods_WPS1;
	m_mapEncrTypes = mapEncrTypes_WPS1;
	m_nSecurityMethodsTotal = SIZEOF_ARRAY(mapSecurityMethods_WPS1);
	m_nEncrTypesTotal = SIZEOF_ARRAY(mapEncrTypes_WPS1);;

	m_SecurityMethod.ResetContent();
	for(nIndex = 0; nIndex < m_nSecurityMethodsTotal; nIndex++)
	{
		strLocal.LoadString(m_mapSecurityMethods[nIndex].uDisplayStr);
		m_SecurityMethod.InsertString(nIndex, strLocal);
		m_SecurityMethod.SetItemDataPtr(nIndex, (void *)(&m_mapSecurityMethods[nIndex]));
	}
	m_SecurityMethod.SetCurSel(0);

	// Fill encryption types combobox
	m_EncrType.ResetContent();
	for(nIndex = 0; nIndex < m_nEncrTypesTotal; nIndex++)
	{
		strLocal.LoadString(m_mapEncrTypes[nIndex].uDisplayStr);
		m_EncrType.InsertString(nIndex, strLocal);
		m_EncrType.SetItemDataPtr(nIndex, (void *)(&m_mapEncrTypes[nIndex]));
	}
	m_EncrType.SetCurSel(0);

	OnCbnSelchangeSecurityMethod();
}

BOOL CConfigApPg::ValidateControls()
{
	BOOL bRet = FALSE;
	CString strText;
	ST_SECURITYMETHOD *pSecMethod;
	ST_ENCRTYPE *pEncrType;

	// Validating WPS PIN code
	if(!(bRet = ValidateWpsPin(m_strPinCode)))
	{
		strText.LoadString(IDS_BAD_WPS_PIN);
		MessageBox(strText, NULL, MB_ICONERROR | MB_OK);
		return bRet;
	}

	// Validating SSID
	if(!(bRet = ValidateSsid(m_strSsid)))
	{
		strText.LoadString(IDS_BAD_SSID);
		MessageBox(strText, NULL, MB_ICONERROR | MB_OK);
		return bRet;
	}

	bRet = FALSE;
	int nCurSel = m_SecurityMethod.GetCurSel();
	if(nCurSel != CB_ERR)
	{
		pSecMethod = (ST_SECURITYMETHOD *)m_SecurityMethod.GetItemData(nCurSel);
		if(pSecMethod)
		{
			if(pSecMethod->eSecMethod == BRCM_SECURITY_WPA2PSK || pSecMethod->eSecMethod == BRCM_SECURITY_WPAPSK)
			{
				// Validating WPA network key
				if(!(bRet = ValidatePSKPassword(m_strNetworkKey)))
				{
					strText.LoadString(IDS_BAD_WPA_KEY);
					MessageBox(strText, NULL, MB_ICONERROR | MB_OK);
					return bRet;
				}
			}
			else if(pSecMethod->eSecMethod == BRCM_SECURITY_WEP)
			{
				// Validating WEP key
				if(!(bRet = ValidateWEPPassword(m_strNetworkKey)))
				{
					strText.LoadString(IDS_BAD_WEP_KEY);
					MessageBox(strText, NULL, MB_ICONERROR | MB_OK);
					return bRet;
				}
			}
			else
			{
				// No security
				bRet = TRUE;
			}
		}
	}

	// Validate security settings for WPSv2
	nCurSel = m_EncrType.GetCurSel();
	if (nCurSel != CB_ERR) {
		// Get enryption type
		m_WpsManager.m_bWpsV2 = true;  // v2 by default
		pEncrType = (ST_ENCRTYPE *)m_EncrType.GetItemData(nCurSel);
		if (pSecMethod->eSecMethod == BRCM_SECURITY_WEP ||
			pSecMethod->eSecMethod == BRCM_SECURITY_WPAPSK ||
			pEncrType->uEncrType == ENCRYPT_WEP ||
			pEncrType->uEncrType == ENCRYPT_TKIP)
		{
			CString strText;
			strText.LoadString(IDS_CONFIRM_WPS1_SETTING);
			if (MessageBox(strText, NULL, MB_YESNO | MB_ICONQUESTION) == IDYES) {
				// Since we are using legacy security settings, we need to configure wpsapi to be v1
				m_WpsManager.m_bWpsV2 = false;
				bRet = TRUE;
			}
			else {
				bRet = FALSE;
			}
		}
	}

	return bRet;
}

LRESULT CConfigApPg::OnWizardNext()
{
	UpdateData();

	if(!ValidateControls())
		return -1;  // Prevent the page from chaning

	// If "Open" is mode is selected, warn the user and get confirmation before continuing
	// The warning is required according to Wi-Fi WPS testbed
	int nCurSel = m_SecurityMethod.GetCurSel();
	if(nCurSel != CB_ERR)
	{
		ST_SECURITYMETHOD *pSecMethod = (ST_SECURITYMETHOD *)m_SecurityMethod.GetItemData(nCurSel);
		if(pSecMethod->eSecMethod == BRCM_SECURITY_OPEN)
		{
			CString strText;
			strText.LoadString(IDS_WARN_OPEN_NETWORK);
			if(MessageBox(strText, NULL, MB_ICONWARNING | MB_YESNO) != IDYES)
				return -1;
		}
	}

	// Set PIN code
	m_WpsManager.SetPinCode(LPCTSTR(m_strPinCode));

	// Set wps credential to wps manager
	wps_credentials cred;
	GetWpsCredential(cred);
	m_WpsManager.SetWpsCredential(cred);

	m_pNextPage = &(m_pWizardSheet->m_ConfigStatusPg);

	return CWizardPropPg::OnWizardNext(this);
}

void CConfigApPg::GetWpsCredential(wps_credentials& cred)
{
	UpdateData();

	int nIndex;
	BRCM_WPSSECURITYMETHOD eSelectedSecMethod;

	strcpy(cred.ssid, CT2A(m_strSsid));
	strcpy(cred.nwKey, CT2A(m_strNetworkKey));

	// Fill selected security method
	for(nIndex=0; nIndex < m_nSecurityMethodsTotal; nIndex++)
	{
		if(nIndex == m_SecurityMethod.GetCurSel())
		{
			strcpy(cred.keyMgmt, m_mapSecurityMethods[nIndex].szKeyMgmt);
			eSelectedSecMethod = m_mapSecurityMethods[nIndex].eSecMethod;
			break;
		}
	}

	// Fill selected encryption type
	if(eSelectedSecMethod == BRCM_SECURITY_WPA2PSK || eSelectedSecMethod == BRCM_SECURITY_WPAPSK)
	{
		// "AES" or "TKIP" should be selected
		for(nIndex=0; nIndex < m_nEncrTypesTotal; nIndex++)
		{
			if(nIndex == m_EncrType.GetCurSel())
			{
				cred.encrType = m_mapEncrTypes[nIndex].uEncrType;
				break;
			}
		}
	}
	else if(eSelectedSecMethod == BRCM_SECURITY_WEP)
	{
		cred.encrType = ENCRYPT_WEP;  // WEP
	}
	else
	{
		cred.encrType = ENCRYPT_NONE;  // Open/No-security
	}
}

void CConfigApPg::PopulateNWSettingControls(const wps_credentials& cred)
{
	m_strSsid = cred.ssid;
	m_strNetworkKey = cred.nwKey;

	CString strLocal;
	int nIndex;
	
	// Populate security method combobox
	for(nIndex=0; nIndex < m_nSecurityMethodsTotal; nIndex++)
	{
		// Go through map to find the display string of the given security method
		if(strcmp(cred.keyMgmt, m_mapSecurityMethods[nIndex].szKeyMgmt) == 0)
		{
			strLocal.LoadString(m_mapSecurityMethods[nIndex].uDisplayStr);  // Form string based on string ID
			if((nIndex = m_SecurityMethod.FindString(0, strLocal)) != CB_ERR)  // Get the index of the given string
				m_SecurityMethod.SetCurSel(nIndex);  // Set it to current selection
			break;
		}
	}
	
	// Populate encryption method combobox
	for(nIndex=0; nIndex < m_nEncrTypesTotal; nIndex++)
	{
		// Go through map to find the display string of the given encryption method
		if(cred.encrType == m_mapEncrTypes[nIndex].uEncrType)
		{
			strLocal.LoadString(m_mapEncrTypes[nIndex].uDisplayStr);  // Form string based on string ID
			if((nIndex = m_EncrType.FindString(0, strLocal)) != CB_ERR)  // Get the index of the given string
				m_EncrType.SetCurSel(nIndex);  // Set it to current selection
			break;
		}
	}

	UpdateData(FALSE);
}

void CConfigApPg::OnBnClickedCheckHideChar()
{
	if(m_HideChar.GetCheck() == BST_CHECKED)
	{
		m_NetworkKey.SetPasswordChar(PASSWORD_CHAR);
	}
	else
	{
		m_NetworkKey.SetPasswordChar(0);
	}

	// Update network windows to reflect the changes
	m_NetworkKey.InvalidateRect(NULL);
	m_NetworkKey.UpdateWindow();
}

void CConfigApPg::OnBnClickedGenerateDefault()
{
	UpdateData();

	wps_credentials cred;
	wps_api_generate_cred(&cred);
	PopulateNWSettingControls(cred);  // Populate network setting controls
}

void CConfigApPg::OnCbnSelchangeSecurityMethod()
{
	int nCurSel = m_SecurityMethod.GetCurSel();
	if(nCurSel != CB_ERR)
	{
		ST_SECURITYMETHOD *pSecMethod = (ST_SECURITYMETHOD *)m_SecurityMethod.GetItemData(nCurSel);
		if(pSecMethod)
		{
			switch(pSecMethod->eSecMethod)
			{
			case BRCM_SECURITY_WPA2PSK:
			case BRCM_SECURITY_WPAPSK:
				OnWpapskModeSelected();
				break;
			case BRCM_SECURITY_WEP:
				OnWepModeSelected();
				break;
			case BRCM_SECURITY_OPEN:
				OnOpenModeSelected();
				break;
			}
		}
	}
}

void CConfigApPg::OnWpapskModeSelected()
{
	// Toggle and update related controls
	m_NetworkKey.EnableWindow();
	m_EncrType.EnableWindow();
	m_HideChar.EnableWindow();
	m_staticKeyDesc.SetWPSWWindowText(IDS_WPA_KEY_DESC);
}

void CConfigApPg::OnWepModeSelected()
{
	// Toggle and update related controls
	m_NetworkKey.EnableWindow();
	m_EncrType.EnableWindow(FALSE);
	m_HideChar.EnableWindow();

	m_staticKeyDesc.SetWPSWWindowText(IDS_WEP_KEY_DESC);

	UpdateData();
}

void CConfigApPg::OnOpenModeSelected()
{
	// Toggle and update related controls
	m_NetworkKey.EnableWindow(FALSE);
	m_EncrType.EnableWindow(FALSE);
	m_HideChar.EnableWindow(FALSE);
	m_staticKeyDesc.SetWPSWWindowText(IDS_OPEN_MODE_DESC);

	UpdateData();
}

BOOL CConfigApPg::OnSetActive()
{
	InitConfigureAPControls();

	return CWizardPropPg::OnSetActive();
}
