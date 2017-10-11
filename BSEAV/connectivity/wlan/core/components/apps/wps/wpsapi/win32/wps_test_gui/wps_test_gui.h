/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wps_test_gui.h,v 1.4 2008/06/05 23:10:59 Exp $
 */

// wps_test_gui.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// Cwps_test_guiApp:
// See wps_test_gui.cpp for the implementation of this class
//

class Cwps_test_guiApp : public CWinApp
{
public:
	Cwps_test_guiApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()

private:
	BOOL BringUpCurrentInstance();
};

extern Cwps_test_guiApp theApp;
