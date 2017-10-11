/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: SelectAPDlg.h,v 1.5 2008/09/23 18:16:04 Exp $
*/
#pragma once
#include "afxwin.h"
#include "WpsManager.h"

// CSelectAPDlg dialog

class CWpsManager;

class CSelectAPDlg : public CDialog
{
	DECLARE_DYNAMIC(CSelectAPDlg)

public:
	CSelectAPDlg(CWnd* pParent = NULL);   // standard constructor
	CSelectAPDlg(CWnd* pParent, CWpsManager *pWpsManager);   // standard constructor
	virtual ~CSelectAPDlg();

	// Dialog Data
	enum { IDD = IDD_SELECT_AP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CListCtrl m_APList;

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedScanNetwork();
	const CString& GetSelectedAP() const { return m_strSelectedAP; }
	LPSTAP GetSelectedAP() { return m_pSelectedAP; }

	CStatic m_LabelSelNetwork;
	CButton m_ShowAuthAPs;

private:
	void PopulateNetworkList(const VEC_AP& vecWpsAPs);
	void OnScanStart();
	void OnScanStop();

	VEC_AP m_vecWpsAPs; 
	CString m_strSelectedAP;
	LPSTAP m_pSelectedAP;
	CWpsManager *m_pWpsManager;
	bool m_bShowAuthAPOnly;
	afx_msg void OnBnClickedShowAuthAp();
};
