/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: WpsRoleSelectionPg.h,v 1.3 2008/10/27 20:35:48 Exp $
 */
#pragma once


// CWpsRoleSelectionPg dialog

class CWpsRoleSelectionPg : public CWizardPropPg
{
public:
	CWpsRoleSelectionPg();
	virtual ~CWpsRoleSelectionPg();

// Dialog Data
	enum { IDD = IDD_WPSROLESELECTIONPG };

	afx_msg void OnBnClickedRoleJoinAp();
	afx_msg void OnBnClickedRoleConfigAp();
	CWPSWStatic m_staticAutoSelDesc;

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();

	int m_eWpsRole;
	
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_bJoinNW;
};
