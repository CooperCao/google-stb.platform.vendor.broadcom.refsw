/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: ConfigApPg.h,v 1.3 2008/10/27 20:35:47 Exp $
 */
#pragma once
#include "afxwin.h"

enum BRCM_WPSSECURITYMETHOD
{
	BRCM_SECURITY_WPA2PSK = 0,
	BRCM_SECURITY_WPAPSK,
	BRCM_SECURITY_WEP,
	BRCM_SECURITY_OPEN,
	BRCM_SECURITY_TOTAL
};

// Define the security method map of enum definition, display string and key management
// We only support "OPEN" authentication method
struct ST_SECURITYMETHOD {
	BRCM_WPSSECURITYMETHOD eSecMethod;
	UINT uDisplayStr;
	const char *szKeyMgmt;
}; 


// Define the encryption type map of type definition and display string
struct ST_ENCRTYPE {
	uint32 uEncrType;
	UINT uDisplayStr;
};


// CConfigApPg dialog

class CConfigApPg : public CWizardPropPg
{
	// Cwps_test_guiDlg dialog
public:
	CConfigApPg();
	virtual ~CConfigApPg();

// Dialog Data
	enum { IDD = IDD_CONFIGAPPG };

	afx_msg void OnBnClickedCheckHideChar();
	afx_msg void OnBnClickedGenerateDefault();
	afx_msg void OnCbnSelchangeSecurityMethod();

	CComboBox m_SecurityMethod;
	CComboBox m_EncrType;
	CString m_strNetworkKey;
	CString m_strSsid;
	CString m_strPinCode;
	CEdit m_NetworkKey;
	CButton m_HideChar;
	CButton m_GenerateDefault;
	CWPSWStatic m_staticEnterApPin;
	CWPSWStatic m_staticSsid;
	CWPSWStatic m_staticSecMethod;
	CWPSWStatic m_staticDataEncr;
	CWPSWStatic m_staticNetworkKey;
	CWPSWStatic m_staticKeyDesc;
	CWPSWStatic m_staticApPinDesc;
	CWPSWStatic m_staticGenDefaultDesc;

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT OnWizardNext();
	BOOL ValidateControls();
	virtual BOOL OnSetActive();

	DECLARE_MESSAGE_MAP()

private:
	void InitConfigureAPControls();
	void GetWpsCredential(wps_credentials& cred);
	void PopulateNWSettingControls(const wps_credentials& cred);
	void OnWpapskModeSelected();
	void OnWepModeSelected();
	void OnOpenModeSelected();

	ST_SECURITYMETHOD *m_mapSecurityMethods;
	ST_ENCRTYPE *m_mapEncrTypes;
	int m_nSecurityMethodsTotal;
	int m_nEncrTypesTotal;
};
