/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wps_test_guiDlg.h,v 1.9 2009/10/17 00:11:04 Exp $
 */

// wps_test_guiDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "wps_sdk.h"
#include "WpsManager.h"

// Cwps_test_guiDlg dialog
enum BRCM_WPSSECURITYMETHOD
{
	BRCM_SECURITY_WPA2PSK = 0,
	BRCM_SECURITY_WPAPSK,
	BRCM_SECURITY_WEP,
	BRCM_SECURITY_OPEN,
	BRCM_SECURITY_TOTAL
};

class Cwps_test_guiDlg : public CDialog
{
// Construction
public:
	enum eBTN_WPS_STATUS { BTN_WPS_START, BTN_WPS_CANCEL, BTN_WPS_EXIT };

	Cwps_test_guiDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WPS_TEST_GUI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	
	DECLARE_MESSAGE_MAP()

public:
	LRESULT OnWpsStatusUpdate(WPARAM wParam, LPARAM lParam);
	LRESULT OnWpsHWTriggerStateUpdate(WPARAM wParam, LPARAM lParam);

	afx_msg void OnBnClickedEnrolleeJoinNw();
	afx_msg void OnBnClickedStaErJoinNw();
	afx_msg void OnBnClickedExit();
	afx_msg void OnBnClickedClearLog();
	afx_msg void OnBnClickedSelModePin();
	afx_msg void OnBnClickedSelModePbc();
	afx_msg void OnBnClickedConfigureAp();
	afx_msg void OnCbnSelchangeSecurityMethod();
	afx_msg void OnBnClickedGenerateCredRandom();
	afx_msg void OnBnClickedGenerateCredManual();
	afx_msg void OnBnClickedCancelWps();
	afx_msg void OnBnClickedGenerateWpsPin();
	
	CListBox m_Status;
	CButton m_WpsJoinNetwork;
	CComboBox m_SecurityMethod;
	CComboBox m_EncrType;
	CEdit m_NetworkKey;
	CEdit m_Ssid;
	bool m_Continue;
	unsigned int m_uiStatus;
	BOOL m_bIsWpsRunning;
	CButton m_Exit;
	CString m_strWpsPin;


private:
	void StartJoiningNW();
	void StartConfiguringAP();
	void StopWps();
	void UpdateWpsStatus();
	void EnablePinControls(BOOL bFlag = TRUE);
	void InitConfigureAPControls();
	void SetBtnStartWps();
	void SetBtnCancelWps();

	eWPS_MODE m_eSelectedWpsMode;
	eBRCM_GENERATECREDMODE m_eGenerateCredMode;
	CWpsManager m_WpsManager;
	CString m_strSelectedAP;
	CString m_strAPPin;
	CString m_strNetworkKey;
	CString m_strSsid;
	CString m_strAdapterGuid;
	afx_msg void OnBnClickedMonitorHwbutton();
};
