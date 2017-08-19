#pragma once
#include "afxcmn.h"
#include "../../include/wps_sdk.h"
#include "afxwin.h"

// CDlgWPSCredentials dialog
class CDlgWPSCredentials : public CDialog
{
	DECLARE_DYNAMIC(CDlgWPSCredentials)

public:
	CDlgWPSCredentials(wps_credentials* pCredentials, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgWPSCredentials();
	BOOL IsRandomCredentials();
	wps_credentials* GetCredentials();

// Dialog Data
	enum { IDD = IDD_CREDENTIALS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy(); 
	DECLARE_MESSAGE_MAP()
private:
	LONG AdjustControls();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedCheckRandomCredentials();

	void ShowHideControls();
	BOOL m_bRandom;
	CEdit m_edtSSID;
	CComboBox m_cmbKeyMgmt;
	CComboBox m_cmbEncyption;
	CEdit m_edtKey;
	CEdit m_edtPinValue;
	wps_credentials* m_pCredentials;
	char m_pin[80];
	afx_msg void OnBnClickedButtonGeneratePin();
	afx_msg void OnBnClickedButtonTestPin();
	afx_msg void OnCbnSelchangeComboKeyMgmt();
	afx_msg void OnCbnSelchangeComboEncryption();
};
