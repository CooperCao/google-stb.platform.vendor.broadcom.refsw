/* 
    Broadcom Proprietary and Confidential. Copyright (C) 2017,
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
    the contents of this file may not be disclosed to third parties, copied
    or duplicated in any form, in whole or in part, without the prior
    written permission of Broadcom.
	
    $Id: Service.cpp,v 1.1 2008/04/17 00:44:52 Exp $
*/

#include <windows.h>
#include <winsvc.h>
#include "Service.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL Service::s_bInitialized = FALSE;

/* function pointers from advapi32.dll */
SC_HANDLE (PASCAL *Service::OpenSCManagerA)(LPCSTR,LPCSTR,DWORD) = NULL;
SC_HANDLE (PASCAL *Service::OpenServiceA)(SC_HANDLE,LPCSTR,DWORD) = NULL;
BOOL (PASCAL *Service::StartServiceA)(SC_HANDLE,DWORD,LPCSTR*) = NULL;
BOOL (PASCAL *Service::QueryServiceStatus)(SC_HANDLE,LPSERVICE_STATUS) = NULL;
BOOL (PASCAL *Service::CloseServiceHandle)(SC_HANDLE) = NULL;
BOOL (PASCAL *Service::ControlService)(SC_HANDLE,DWORD,LPSERVICE_STATUS) = NULL;
const int Service::NUMBER_OF_PENDING_ATTEMPTS = 30;

/* static method to initialize the function pointers from advapi32.dll.
   Do not call this routine directly.  It will be authomatically
   called by the statis routines that need it.
*/
BOOL Service::Initialized()
{
	HINSTANCE hDll;

	if (!s_bInitialized) {
		hDll = LoadLibraryW(L"advapi32.dll");
		if (NULL != hDll) {
			(FARPROC&)OpenSCManagerA = ::GetProcAddress(hDll, "OpenSCManagerA");
			(FARPROC&)OpenServiceA = ::GetProcAddress(hDll, "OpenServiceA");
			(FARPROC&)StartServiceA = ::GetProcAddress(hDll, "StartServiceA");
			(FARPROC&)QueryServiceStatus = ::GetProcAddress(hDll, "QueryServiceStatus");
			(FARPROC&)CloseServiceHandle = ::GetProcAddress(hDll, "CloseServiceHandle");
			(FARPROC&)ControlService = ::GetProcAddress(hDll, "ControlService");
		}
		s_bInitialized = TRUE;
	}

	return (OpenSCManager != NULL);
}

/*
  Write the current AP configuration to the registry.
*/
BOOL Service::Stop(LPCTSTR lpServiceName)
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	SERVICE_STATUS ssStatus;
	BOOL bRet = FALSE;
	int iAttempts;

	if (!Initialized()) {
		return FALSE;
	}

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if( schSCManager ) {
		schService = OpenService(schSCManager, lpServiceName, SERVICE_ALL_ACCESS);
		
		if( schService ) {
			// try to stop the service
			if( ControlService(schService, SERVICE_CONTROL_STOP, &ssStatus) ) {
				Sleep(1000);

				iAttempts = 0;
				while( QueryServiceStatus(schService, &ssStatus) 
					   && ++iAttempts <= NUMBER_OF_PENDING_ATTEMPTS) {
					if( ssStatus.dwCurrentState == SERVICE_STOP_PENDING ) {
						Sleep( 1000 );
					} else
						break;
				}

				if( ssStatus.dwCurrentState == SERVICE_STOPPED )
					bRet = TRUE;
			} else {
			}

			CloseServiceHandle(schService);
		} else {
		}

		CloseServiceHandle(schSCManager);
	} else {
	}
	
	return bRet;
}

BOOL Service::IsRunning(LPCTSTR lpServiceName)
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	SERVICE_STATUS ssStatus;
	BOOL bRet = FALSE;

	if (!Initialized()) {
		return FALSE;
	}

	schSCManager = OpenSCManager(NULL, NULL, GENERIC_READ);
	if( schSCManager ) {
		schService = OpenService(schSCManager, lpServiceName, 
								 SERVICE_QUERY_STATUS);
		
		if( schService ) {
			QueryServiceStatus(schService, &ssStatus);
			if( ssStatus.dwCurrentState == SERVICE_RUNNING )
				bRet = TRUE;
			CloseServiceHandle(schService);
		} else {
		}
		CloseServiceHandle(schSCManager);
	} else {
	}

	return bRet;
}

BOOL Service::Start(LPCTSTR lpServiceName)
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	SERVICE_STATUS ssStatus;
	BOOL bRet = FALSE;
	int iAttempts;

	if (!Initialized()) {
		return FALSE;
	}

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if( schSCManager ) {
		schService = OpenService(schSCManager, lpServiceName, SERVICE_ALL_ACCESS);
		
		if( schService ) {
			// try to start the service
			if( StartService(schService, NULL, NULL) ) {
				Sleep(1000);

				iAttempts = 0;
				while( QueryServiceStatus(schService, &ssStatus)
					   && ++iAttempts <= NUMBER_OF_PENDING_ATTEMPTS &&
					   ssStatus.dwCurrentState != SERVICE_RUNNING )
				{
					if( ssStatus.dwCurrentState == SERVICE_START_PENDING )
						Sleep( 1000 );
				}

				if( ssStatus.dwCurrentState == SERVICE_RUNNING )
					bRet = TRUE;

			} else {
			}

			CloseServiceHandle(schService);
		} else {
		}

		CloseServiceHandle(schSCManager);
	} else {
	}
	

	return bRet;
}

