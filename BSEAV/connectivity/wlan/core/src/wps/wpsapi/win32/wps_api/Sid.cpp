/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: Sid.cpp,v 1.2 2008/05/01 23:13:19 Exp $
 */

#include "stdafx.h"



#include <windows.h>

//#include <Sddl.h>

#include <Lmcons.h>



#ifndef SECURITY_WIN32

#define SECURITY_WIN32

#endif

#include <Security.h>



#ifndef UNLEN

#define UNLEN 256

#endif



static BOOL sg_bInitialized = FALSE;



/* advapi32.dll */

static BOOL (PASCAL* lpfnLookupAccountName)(LPCTSTR, LPCTSTR, PSID, LPDWORD, LPTSTR, LPDWORD, PSID_NAME_USE) = NULL;

static BOOL (PASCAL* lpfnConvertSidToStringSid)(PSID, LPTSTR*) = NULL;



/* secur32.dll */

static BOOLEAN (PASCAL* lpfnGetUserNameEx)(EXTENDED_NAME_FORMAT, LPTSTR, PULONG) = NULL;



static BOOL sidhlp_initialize()

{

	if (!sg_bInitialized)

	{

		HINSTANCE hSecur32Dll = LoadLibrary("secur32.dll");

		if (hSecur32Dll)

		{

#ifdef UNICODE

			(FARPROC&)lpfnGetUserNameEx = GetProcAddress(hSecur32Dll, "GetUserNameExW");

#else

			(FARPROC&)lpfnGetUserNameEx = GetProcAddress(hSecur32Dll, "GetUserNameExA");

#endif

		}



		HINSTANCE hAdvapi32Dll = LoadLibrary("advapi32.dll");

		if (hAdvapi32Dll)

		{

#ifdef UNICODE

				(FARPROC&)lpfnLookupAccountName = GetProcAddress(hAdvapi32Dll, "LookupAccountNameW");

				(FARPROC&)lpfnConvertSidToStringSid = GetProcAddress(hAdvapi32Dll, "ConvertSidToStringSidW");

#else

				(FARPROC&)lpfnLookupAccountName = GetProcAddress(hAdvapi32Dll, "LookupAccountNameA");

				(FARPROC&)lpfnConvertSidToStringSid = GetProcAddress(hAdvapi32Dll, "ConvertSidToStringSidA");

#endif

		}



		sg_bInitialized = TRUE;

	}



	return TRUE;

}



/* 

 * Get the SID for a given username.  Use a fully qualified string in

 * the domain\user_name format to ensure that LookupAccountName finds

 * the account in the desired domain.

 *

 * If the corresponding SID cannot be found, or if it is not of the

 * expected type, this routine will return FALSE and rpsid will be

 * unchanged.

 *

 * Upon success, this routine returns TRUE, and rpsid is set to point

 * to heap-allocated space constaining the SID.  rpsid should be

 * released by calling 'delete'.

 *

*/

BOOL SidHlp_GetUserSid(LPCTSTR szUserName, PSID& rpsid) 

{

	BOOL bSuccess = FALSE;

	DWORD cbSid, cchDomain;

	PSID psid = NULL;

	SID_NAME_USE peUse;

	LPTSTR szDomain = NULL;



	sidhlp_initialize();



	__try

	{

		cbSid = 0;

		cchDomain = 0;

		if (!lpfnLookupAccountName((LPTSTR) NULL, szUserName, 

							   NULL, &cbSid, \

							   NULL, &cchDomain, &peUse))

		{

			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 

				__leave;

			

			psid = (PSID) new BYTE[cbSid];

			if (psid == NULL)

				__leave;

			szDomain = (LPTSTR) new TCHAR[cchDomain];

			if (szDomain == NULL)

				__leave;

		}

		

		if (!lpfnLookupAccountName((LPTSTR) NULL, szUserName, 

								   psid, &cbSid, 

								   szDomain, &cchDomain, &peUse))

		{

			//TRACE("delete %s:%d\n", __FILE__, __LINE__);

			delete psid;

			__leave;

		}

		

		switch(peUse)

		{

		case SidTypeUser:

			break;

			

		case SidTypeGroup:

		case SidTypeDomain:

		case SidTypeAlias:

		case SidTypeComputer:

		case SidTypeWellKnownGroup:

		case SidTypeDeletedAccount:

		case SidTypeInvalid:

		case SidTypeUnknown:

		default:

			//TRACE("delete %s:%d\n", __FILE__, __LINE__);

			delete psid;

			__leave;

			break;

		}

		

	rpsid = psid;

	

	bSuccess = TRUE;

	}

	__finally

	{

		

		if (szDomain)

		{

			//TRACE("delete %s:%d\n", __FILE__, __LINE__);

			delete szDomain;

		}

	}

	//TRACE("SidHlp_GetUserSid(%s) returns %d\n", szUserName, bSuccess);

	

	return bSuccess;

}



BOOL SidHlp_GetCurrentSid(PSID& rpsid)

{

	BOOL bSuccess = FALSE;

	TCHAR szUserName[UNLEN + 1];

	DWORD dwUserNameLength = sizeof(szUserName);



	sidhlp_initialize();



	if (lpfnGetUserNameEx &&

		lpfnGetUserNameEx(NameSamCompatible, szUserName, &dwUserNameLength))

	{

		//TRACE("GetUserNameEx returned \"%s\"\n", szUserName);

		bSuccess = SidHlp_GetUserSid(szUserName, rpsid);

	}



	return bSuccess;

}



BOOL SidHlp_GetCurrentSid(LPTSTR& str)

{

	PSID psid;

	BOOL ret = FALSE;



	sidhlp_initialize();



	if (SidHlp_GetCurrentSid(psid))

	{

		if (lpfnConvertSidToStringSid &&

			lpfnConvertSidToStringSid(psid, &str))

			ret = TRUE;

		delete psid;

	}

	return ret;

}



BOOL SidHlp_GetUserSid(LPCTSTR lpAccountName, LPTSTR& str)

{

	BOOL ret = FALSE;



	sidhlp_initialize();



	if (lpAccountName == NULL)

	{

		ret = SidHlp_GetCurrentSid(str);

	}

	else

	{

		PSID psid;

		if (SidHlp_GetUserSid(lpAccountName, psid))

		{

			if (lpfnConvertSidToStringSid &&

				lpfnConvertSidToStringSid(psid, &str))

				ret = TRUE;

			delete psid;

		}

	}



	return ret;

}

