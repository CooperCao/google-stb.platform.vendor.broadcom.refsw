/* 
    Broadcom Proprietary and Confidential. Copyright (C) 2017,
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
    the contents of this file may not be disclosed to third parties, copied
    or duplicated in any form, in whole or in part, without the prior
    written permission of Broadcom.
	
    $Id: Service.h,v 1.1 2008/04/17 00:44:52 Exp $
*/

#if !defined(_service_h_)
#define _service_h_

class Service {

 private:
    static int s_bInitialized;
    static const int NUMBER_OF_PENDING_ATTEMPTS;

    /* function pointers from advapi32.dll */
    static SC_HANDLE (PASCAL *OpenSCManagerA)(LPCSTR,LPCSTR,DWORD);
    static SC_HANDLE (PASCAL *OpenServiceA)(SC_HANDLE,LPCSTR,DWORD);
    static BOOL (PASCAL *StartServiceA)(SC_HANDLE,DWORD,LPCSTR*);
    static BOOL (PASCAL *QueryServiceStatus)(SC_HANDLE,LPSERVICE_STATUS);
    static BOOL (PASCAL *CloseServiceHandle)(SC_HANDLE);
    static BOOL (PASCAL *ControlService)(SC_HANDLE,DWORD,LPSERVICE_STATUS);

 public:
    static BOOL	Stop(LPCTSTR lpServiceName);
    static BOOL	Start(LPCTSTR lpServiceName);
    static BOOL	IsRunning(LPCTSTR lpServiceName);
    static BOOL	Initialized();
    
};


#endif /* !defined(_service_h_) */
