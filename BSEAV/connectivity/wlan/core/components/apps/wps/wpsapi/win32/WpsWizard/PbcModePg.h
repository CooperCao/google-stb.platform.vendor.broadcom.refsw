/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: PbcModePg.h,v 1.2 2008/06/14 01:45:57 Exp $
*/
#pragma once
#include "WizardPropPg.h"


// CPbcModePg dialog

class CPbcModePg : public CWizardPropPg
{
public:
	CPbcModePg();
	virtual ~CPbcModePg();

// Dialog Data
	enum { IDD = IDD_PBCMODEPG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual LRESULT OnWizardNext();

	CWPSWStatic m_staticPbcDesc;

	DECLARE_MESSAGE_MAP()
};
