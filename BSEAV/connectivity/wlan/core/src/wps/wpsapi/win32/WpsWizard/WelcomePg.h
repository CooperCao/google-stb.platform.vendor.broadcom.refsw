/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: WelcomePg.h,v 1.3 2008/08/20 01:45:24 Exp $
*/
#pragma once
#include "WizardPropPg.h"

// CWelcomePg dialog

class CWelcomePg : public CWizardPropPg
{
public:
	CWelcomePg();
	virtual ~CWelcomePg();

// Dialog Data
	enum { IDD = IDD_WELCOMEPG };
	//{{AFX_DATA(CWelcomeDlg)
	CBitmapXPButton	m_btnSetup;
	CWPSWStatic	m_staticDescriptionText;
	CWPSWStatic	m_staticTitleText;
	//}}AFX_DATA

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();


	DECLARE_MESSAGE_MAP()
};
