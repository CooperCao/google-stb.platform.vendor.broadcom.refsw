/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: SelectAPDlg.h,v 1.3 2008/06/12 02:45:29 Exp $
*/
#pragma once
#include "afxwin.h"
#include "WpsManager.h"

// CSelectAPDlg dialog

class CSelectAPDlg : public CDialog
{
	DECLARE_DYNAMIC(CSelectAPDlg)

public:
	CSelectAPDlg(CWnd* pParent = NULL);   // standard constructor
	CSelectAPDlg(CWnd* pParent, VEC_AP& vecWpsAPs);   // standard constructor
	virtual ~CSelectAPDlg();

// Dialog Data
	enum { IDD = IDD_SELECT_AP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CListBox m_APList;

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	const CString& GetSelectedAP() const { return m_strSelectedAP; }
	LPSTAP GetSelectedAP() { return m_pSelectedAP; }

private:
	VEC_AP m_vecWpsAPs; 
	CString m_strSelectedAP;
	LPSTAP m_pSelectedAP;
};
