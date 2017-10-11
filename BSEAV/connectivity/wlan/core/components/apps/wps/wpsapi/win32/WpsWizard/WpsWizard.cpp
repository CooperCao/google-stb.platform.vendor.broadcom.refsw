/* 
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id: WpsWizard.cpp,v 1.4 2008/09/26 23:06:18 Exp $
*/
// WpsWizard.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WpsWizard.h"
#include "WizardSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(linker, "/SECTION:.shr,RWS")
#pragma data_seg(".shr")
HWND g_hWnd = NULL;
#pragma data_seg()

#define BROADCOM_WPS_WIZARD_MUTEXT	_T("BROADCOM_WPS_WIZARD_MUTEXT")

// CWpsWizardApp

BEGIN_MESSAGE_MAP(CWpsWizardApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CWpsWizardApp construction

CWpsWizardApp::CWpsWizardApp()
{
}


// The one and only CWpsWizardApp object

CWpsWizardApp theApp;


// CWpsWizardApp initialization

BOOL CWpsWizardApp::InitInstance()
{
	CWinApp::InitInstance();

	// If one instance already exists, bring it up and exit
	if(BringUpCurrentInstance())
		return FALSE;

	CWizardSheet WizardSheet(IDS_WPS_WIZARD_TITLE, NULL, 0);
	m_pMainWnd = &WizardSheet;
	WizardSheet.SetWizardMode();
	INT_PTR nResponse = WizardSheet.DoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

BOOL CWpsWizardApp::BringUpCurrentInstance()
{
	HANDLE hMutexOneInstance = ::CreateMutex( NULL, TRUE, BROADCOM_WPS_WIZARD_MUTEXT);
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
