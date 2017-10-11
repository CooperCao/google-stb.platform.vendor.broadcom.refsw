// WizTestDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "../../include/wps_sdk.h"
#include "afxwin.h"

// CWizTestDlg dialog
class CWizTestDlg : public CDialog
{
// Construction
public:
	CWizTestDlg(CWnd* pParent = NULL);	// standard constructor
	static bool OnCallback(void *context,unsigned int uiStatus, void *data);
// Dialog Data
	enum { IDD = IDD_WIZTEST_DIALOG };
	~CWizTestDlg();
	void UpdateStatus(LPCTSTR pszStatus);
	int GetCredentials();

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy(); 
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedEnroll();

private:
	afx_msg void OnBnClickedPinMode();
	afx_msg void OnBnClickedRefresh();

	void Refresh();
	void Clear();
	LONG AdjustControls();

	CEdit m_edtPinValue;
	BOOL m_bPinMode;
	char m_pin[80];
	BOOL bWpsCancelled;
	CListCtrl m_lstNetworks;
	CEdit m_edtStatus;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnLvnItemchangedNetworks(NMHDR *pNMHDR, LRESULT *pResult);
	CComboBox m_listWPSMode;
    void GetConfigurationCredentials();
	afx_msg void OnCbnSelchangeComboMode();
	void UpdateControls(int iSel);
	void UpdatePinMode(BOOL bPinMode);
	eWPS_MODE GetWpsMode();
	BOOL m_CredentialsSelected;
	afx_msg void OnBnClickedButtonCredentials();
	wps_credentials m_credentials;
	afx_msg void OnNMKillfocusNetworks(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMSetfocusNetworks(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnSetfocusPinString();
	afx_msg void OnCbnSetfocusComboMode();
	afx_msg void OnEnChangePinString();
};
