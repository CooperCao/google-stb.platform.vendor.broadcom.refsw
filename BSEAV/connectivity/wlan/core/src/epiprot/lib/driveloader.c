/*
  Based on the sample code INSTDRV from Microsoft.
  Copyright (c) 1993  Microsoft Corporation
  Copyright (c) 1999  Epigram, Inc.

  $Id$

*/

#include <stdio.h>
#include <wtypes.h>
#include <winsvc.h>
#include <io.h>
#include <TCHAR.H>
#include "vendor.h"
#ifdef NTDDKSIM
#include "k32.h"
#endif

typedef DWORD WINERR;

#define ARRAYSIZE(a)  (sizeof(a)/sizeof(a[0]))
#define snprintf _snprintf

static DWORD
InstallDriver(
    IN SC_HANDLE  SchSCManager,
    IN LPCTSTR    DriverName,
    IN LPCTSTR    ServiceExe
    );

static DWORD
RemoveDriver(
    IN SC_HANDLE  SchSCManager,
    IN LPCTSTR    DriverName
    );

static DWORD
StartDriver(
    IN SC_HANDLE  SchSCManager,
    IN LPCTSTR    DriverName
    );

static DWORD
StopDriver(
    IN SC_HANDLE  SchSCManager,
    IN LPCTSTR    DriverName
    );

static DWORD LoadAndOpen(IN LPCTSTR DriverName, IN LPCTSTR ServiceExe);



/* These paths are used when searching for the hwdriver.sys file in
   preparation for loading it as a service. */
static LPCTSTR Paths[] = {
    _T("%SYSTEMROOT%\\System"),
    _T("%SYSTEMROOT%\\System32"),
    _T("%SYSTEMROOT%\\System32\\drivers")
};

/* 
   Find and dynamically load a device.
   
   A set of paths is searched to find the driver before any attempt is
   made to dyanamically load it.

   Returnes TRUE on success, FALSE otherwise.
*/
BOOL findloaddriver(LPCTSTR DriverFilename, LPCTSTR DriverService)
{    
    DWORD status;
    int i, needlen;
    TCHAR path0[200];
    TCHAR DriverExe[200];

    for (i = 0; i < ARRAYSIZE(Paths); i++) {
	if (Paths[i] != NULL) {
	    needlen = ExpandEnvironmentStrings(Paths[i], path0, sizeof(path0));
	    if (needlen <= sizeof(path0)) {
		_sntprintf(DriverExe, sizeof(DriverExe), _T("%s\\%s"), 
			   path0, DriverFilename);
		if (_taccess(DriverExe, 04) == 0) {
		    status = LoadAndOpen(DriverService, DriverExe);
		    switch (status) {
		    case ERROR_SUCCESS:
			/* Fall-Through */
		    case ERROR_SERVICE_EXISTS:
			return TRUE;
		    default:
			return FALSE;
		    }
		}
	    }
		}
    }
    return FALSE;
}



/* Returns ERROR_SUCCESS on success, error code otherwise. 
   REMIND - this need to be made bulletproof!!!
*/
static DWORD LoadAndOpen(IN LPCTSTR DriverName, IN LPCTSTR ServiceExe)
{
    SC_HANDLE   schSCManager;
    DWORD err = ERROR_SUCCESS;

    schSCManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS );
                              /* ( machine (NULL == local), 
				   database (NULL == default), 
				   access required */
    if (schSCManager == NULL) {
		err = GetLastError();
    } else {
		InstallDriver( schSCManager, DriverName, ServiceExe );
		StartDriver( schSCManager, DriverName );
		CloseServiceHandle (schSCManager);
    }
    return err;
}


static DWORD
InstallDriver(
    IN SC_HANDLE  SchSCManager,
    IN LPCTSTR    DriverName,
    IN LPCTSTR    ServiceExe
    )
{
    SC_HANDLE  schService;
    WINERR     status = ERROR_SUCCESS;

    //
    // NOTE: This creates an entry for a standalone driver. If this
    //       is modified for use with a driver that requires a Tag,
    //       Group, and/or Dependencies, it may be necessary to
    //       query the registry for existing driver information
    //       (in order to determine a unique Tag, etc.).
    //

    schService = CreateService (SchSCManager,          // SCManager database
                                DriverName,           // name of service
                                DriverName,           // name to display
                                SERVICE_ALL_ACCESS,    // desired access
                                SERVICE_KERNEL_DRIVER, // service type
                                SERVICE_DEMAND_START,  // start type
                                SERVICE_ERROR_NORMAL,  // error control type
                                ServiceExe,            // service's binary
                                NULL,                  // no load ordering group
                                NULL,                  // no tag identifier
                                NULL,                  // no dependencies
                                NULL,                  // LocalSystem account
                                NULL                   // no password
                                );

    if (schService == NULL) {
        status = GetLastError();
    } 

    return status;
}



static DWORD
RemoveDriver(
    IN SC_HANDLE  SchSCManager,
    IN LPCTSTR    DriverName
    )
{
    SC_HANDLE  schService;
    DWORD err = 0;
    BOOL  ret;

    schService = OpenService (SchSCManager,
                              DriverName,
                              SERVICE_ALL_ACCESS
                              );

    if (schService != NULL) {
		ret = DeleteService (schService);
	
		if (ret == 0) {
			err = GetLastError(); 
		} 
		CloseServiceHandle (schService);
    }

    return err;
}



static DWORD
StartDriver(
    IN SC_HANDLE  SchSCManager,
    IN LPCTSTR    DriverName
    )
{
    SC_HANDLE  schService;
    BOOL       ret;
    DWORD      err = 0;

    schService = OpenService (SchSCManager,
                              DriverName,
                              SERVICE_ALL_ACCESS );

    if (schService == NULL) {
		err = GetLastError();
    } else {
		ret = StartService (schService,    // service identifier
							0,             // number of arguments
							NULL           // pointer to arguments
		);
		if (ret == 0) {
			err = GetLastError();
		} 
		CloseServiceHandle (schService);
    }

    return err;
}



static DWORD
StopDriver(
    IN SC_HANDLE  SchSCManager,
    IN LPCTSTR    DriverName
    )
{
    SC_HANDLE       schService;
    BOOL            ret; 
    DWORD err = 0;
    SERVICE_STATUS  serviceStatus;

    schService = OpenService (SchSCManager,
                              DriverName,
                              SERVICE_ALL_ACCESS
                              );

    if (schService == NULL) {
	err = GetLastError();
    } else {

	ret = ControlService (schService,
			      SERVICE_CONTROL_STOP,
			      &serviceStatus );
	if (ret == 0) {
	    err = GetLastError();
	    printf ("failure: ControlService (0x%02x)\n", err);
	}

	CloseServiceHandle (schService);
    }

    return err;
}
