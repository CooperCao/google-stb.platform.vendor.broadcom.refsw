/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: WizardPropPg.h,v 1.6 2008/10/27 20:35:48 Exp $
*/
#pragma once
#include "wps_sdk.h"
#include "WizardSheet.h"
#include "WpsManager.h"
#include "WpsWizardUtil.h"

// CWizardPropPg dialog
class CWizardSheet;
class CWpsManager;

class CWizardPropPg : public CPropertyPage
{
public:
	CWizardPropPg() {}
	CWizardPropPg(UINT nIDTemplate, UINT nIDCaption = 0 , UINT nIDHeaderTitle = 0, UINT nIDHeaderSubTitle = 0);
	virtual ~CWizardPropPg();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	virtual BOOL OnInitDialog();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnKillActive();
	virtual void OnCancel();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	LRESULT OnWizardNext(CWizardPropPg* pPrevPage);

	CXColorStatic	m_staticRightBk;
	CXColorStatic	m_staticLeftBk;
	CStaticBitmap	m_staticCompanyLogoBitmap;
	CStaticBitmap	m_staticSecureEZLogo;
	static CWpsManager m_WpsManager;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support


	void SetControlDefaultFont(UINT nCtrlID);
	CWpsManager* GetWpsManager() const;
	BOOL SetLabel(UINT uControlId, UINT uStrId);
	void SetWizardButtons(DWORD dwFlags);

	DECLARE_MESSAGE_MAP()

	static CWizardSheet *m_pWizardSheet;
	static BOOL m_bAutoConnect;
	CWizardPropPg *m_pNextPage;
	CWizardPropPg *m_pPrevPage;
	static eWPS_MODE m_eSelectedWpsMode;

private:
	void Init();

	CFont m_Font;
};
