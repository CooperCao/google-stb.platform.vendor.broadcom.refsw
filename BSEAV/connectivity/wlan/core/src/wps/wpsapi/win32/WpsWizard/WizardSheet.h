/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: WizardSheet.h,v 1.6 2009/02/17 18:22:20 Exp $
*/
#pragma once

#include "WelcomePg.h"
#include "ConfigStatusPg.h"
#include "PbcModePg.h"
#include "PinModePg.h"
#include "WpsModeSelectionPg.h"
#include "WpsRoleSelectionPg.h"
#include "ConfigApPg.h"
#include "WpsFinishPg.h"
#include "WpsManager.h"
#include "WpsVerSelectionPg.h"

// CWizardSheet

class CWizardSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CWizardSheet)

public:
	CWizardSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CWizardSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CWizardSheet();

	virtual BOOL OnInitDialog();
	CString GetPinCode() const;
	void SetPinCode(const CString& strPin);
	LRESULT OnWpsStatusUpdate(WPARAM wParam, LPARAM lParam);
	CWpsManager *GetWpsManager() const;
	BOOL SetPageTitle(int nPage, LPCTSTR pszText); 

protected:
	DECLARE_MESSAGE_MAP()

// Data
public:
	CWelcomePg m_WelcomePg;
	CConfigStatusPg m_ConfigStatusPg;
	CPbcModePg m_PbcModePg;
	CPinModePg m_PinModePg;
	CWpsModeSelectionPg m_WpsModeSelectionPg;
	CWpsRoleSelectionPg m_WpsRoleSelectionPg;
	CConfigApPg m_ConfigApPg;
	CWpsFinishPg m_WpsFinishPg;
	CWpsVerSelectionPg m_WpsVerSelectionPg;

	BOOL m_bToRunWps;

private:
	CWpsManager *m_pWpsManager;
	CString m_strPin;
};
