/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: InfoDlg.h,v 1.2 2008/08/25 21:35:13 Exp $
 */
#if !defined(AFX_INFODLG_H__D296C97B_F411_413C_981A_CC99A0B49259__INCLUDED_)
#define AFX_INFODLG_H__D296C97B_F411_413C_981A_CC99A0B49259__INCLUDED_

#if 0> 1000
#pragma once
#endif 
// InfoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInfoDlg dialog

class CInfoDlg : public CDialog
{
// Construction

	UINT m_TitleId;
	UINT m_BodyTextId;  // RTF resource ID 
public:
	CInfoDlg(CWnd* pParent = NULL);   // standard constructor

	void SetId( UINT titleid, UINT bodyid) { m_TitleId=titleid, m_BodyTextId = bodyid; }

// Dialog Data
	//{{AFX_DATA(CInfoDlg)
	enum { IDD = IDD_DIALOG_INFO };
	CRichEditCtrl	m_ctrlRichText;
	CStatic	m_ctrlText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInfoDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INFODLG_H__D296C97B_F411_413C_981A_CC99A0B49259__INCLUDED_)
