/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: WpsWizard.h,v 1.5 2008/10/14 18:06:57 Exp $
*/
// WpsWizard.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "autores.h"
#include "resource.h"		// main symbols
#include "WizardSheet.h"

// CWpsWizardApp:
// See WpsWizard.cpp for the implementation of this class
//

class CWpsWizardApp : public CWinApp
{
public:
	CWpsWizardApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

private:
	BOOL BringUpCurrentInstance();

	DECLARE_MESSAGE_MAP()
};

extern CWpsWizardApp theApp;
