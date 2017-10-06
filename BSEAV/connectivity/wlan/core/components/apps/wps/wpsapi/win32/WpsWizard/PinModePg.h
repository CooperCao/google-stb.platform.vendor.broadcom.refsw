/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: PinModePg.h,v 1.4 2008/10/27 20:35:47 Exp $
*/
#pragma once
#include "WizardPropPg.h"


// CPinModePg dialog

class CPinModePg : public CWizardPropPg
{
public:
	CPinModePg();
	virtual ~CPinModePg();

// Dialog Data
	enum { IDD = IDD_PINMODEPG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual LRESULT OnWizardNext();
	BOOL ValidateControls();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedStaerGetSettings();
	afx_msg void OnBnClickedStaenrGetSettings();

	CString m_strEnrolleePin;
	CString m_strRegistrarPin;
	CWPSWStatic m_staticPinDesc;
	CWPSWStatic m_staticPin;
	CWPSWStatic m_EnrolleePin;
	CWPSWStatic m_staticInstConnAdmin;
	CEdit m_RegPin;
};
