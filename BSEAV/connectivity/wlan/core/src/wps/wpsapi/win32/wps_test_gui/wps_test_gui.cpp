/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wps_test_gui.cpp,v 1.5 2008/09/26 23:06:20 Exp $
 */

// wps_test_gui.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "wps_test_gui.h"
#include "wps_test_guiDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(linker, "/SECTION:.shr,RWS")
#pragma data_seg(".shr")
HWND g_hWnd = NULL;
#pragma data_seg()


// Cwps_test_guiApp

BEGIN_MESSAGE_MAP(Cwps_test_guiApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// Cwps_test_guiApp construction

Cwps_test_guiApp::Cwps_test_guiApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only Cwps_test_guiApp object

Cwps_test_guiApp theApp;


// Cwps_test_guiApp initialization

BOOL Cwps_test_guiApp::InitInstance()
{
	
	// If one instance already exists, bring it up and exit
	if(BringUpCurrentInstance())
		return FALSE;

	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	Cwps_test_guiDlg dlg;
	m_pMainWnd = &dlg;

	INT_PTR nResponse = dlg.DoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

BOOL Cwps_test_guiApp::BringUpCurrentInstance()
{
	HANDLE hMutexOneInstance = ::CreateMutex( NULL, TRUE, _T("BROADCOM_WPS_UTILITY"));
	BOOL bAlreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS);
	if(hMutexOneInstance != NULL) 
		ReleaseMutex(hMutexOneInstance);

	if(bAlreadyRunning)
	{
		if(g_hWnd)
		{
			SetForegroundWindow(g_hWnd);
			ShowWindow(g_hWnd, SW_RESTORE);
		}
		return TRUE; // terminates the creation
	}
	return FALSE;
};
