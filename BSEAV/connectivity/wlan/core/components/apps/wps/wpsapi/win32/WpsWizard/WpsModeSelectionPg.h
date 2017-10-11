/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: WpsModeSelectionPg.h,v 1.5 2008/10/27 20:35:48 Exp $
*/

#pragma once
#include "afxwin.h"
#include "WizardPropPg.h"

// CWpsModeSelectionPg dialog
class CWpsModeSelectionPg : public CWizardPropPg
{

public:
	CWpsModeSelectionPg();
	virtual ~CWpsModeSelectionPg();

// Dialog Data
	enum { IDD = IDD_WPSMODESELECTIONPG };

	afx_msg void OnBnClickedModePbc();
	afx_msg void OnBnClickedModePin();

	CWPSWStatic	m_staticSelModeText;

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT OnWizardNext();

	DECLARE_MESSAGE_MAP()
};
