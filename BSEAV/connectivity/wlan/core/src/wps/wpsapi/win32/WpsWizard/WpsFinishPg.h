/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: WpsFinishPg.h,v 1.3 2008/08/25 21:35:13 Exp $
 */
#pragma once
#include "afxwin.h"


// CWpsFinishPg dialog

class CWpsFinishPg : public CWizardPropPg
{
public:
	CWpsFinishPg();
	virtual ~CWpsFinishPg();

// Dialog Data
	enum { IDD = IDD_WPSFINISHPG };

	CWPSWStatic m_ResultHeadline;
	CWPSWStatic m_NetworkSsid;
	CWPSWStatic m_labelSsid;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();

	DECLARE_MESSAGE_MAP()
};
