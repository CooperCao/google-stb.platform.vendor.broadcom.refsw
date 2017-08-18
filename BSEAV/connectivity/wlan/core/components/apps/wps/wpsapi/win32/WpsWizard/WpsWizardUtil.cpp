/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: WpsWizardUtil.cpp,v 1.3 2008/08/25 21:35:13 Exp $
 */
#include "stdafx.h"
#include "WpsWizardUtil.h"
#include "wps_sdk.h"

const LPCTSTR HEXCHARS		= _T("0123456789abcdefABCDEF");
const LPCTSTR DIGITCHARS	= _T("0123456789");
const int WPS_PIN_LEN		= 8;

// Set dialog control text with given string ID 
void SetLabel(HWND hWnd, UINT uControlId, UINT uStrId)
{
	HWND hItemWnd = GetDlgItem(hWnd, uControlId);
	if(hItemWnd)
	{
		CString strText;
		strText.LoadString(uStrId);
		::SetWindowText(hItemWnd, strText);
	}
}

/* Return TRUE if the supplied password is valid for WPA-PSK.  The
   passphrase is either an ascii string of 8 to 63 characters
   inclusive, or a hex string of exactly 64 characters.  It's
   independent of the crypto algorithm (TKIP or AES).
*/
BOOL ValidatePSKPassword(const CString& password)
{
	int len = password.GetLength();
	int hexlen;

	/* a hex string of exactly 64 characters */
	if (len == 64) 
	{
		hexlen = password.SpanIncluding(HEXCHARS).GetLength();

		return (len == hexlen);
	}
	/*  an ascii string of 8 to 63 characters */
	if (len >= 8 && len <= 63)
		return TRUE;

	return FALSE;
}

/* 
   Validates that the wep information provided is consistent with the key
   format and key length specifications.  

   NOTE - some other implementations of this routine also convert hex
   wep keys to ascii.  This implementation does not (hend it takes a
   const reference).  

   Theoretically it should not be necessary to convert hex wep keys to
   ascii because the SetWEPKey() routine in Wladapter already does
   that just before writing the key to the adapter.
*/
BOOL ValidateWEPPassword(const CString& password)
{
	int hexlen;

	switch (password.GetLength()) {
	case 10: /* HEX */
	case 26:
		hexlen = password.SpanIncluding(HEXCHARS).GetLength();

		if (hexlen != password.GetLength()) 
			return FALSE;

		break;

	case 5:  /* ascii text */
	case 13:
		break;

	default:
		return FALSE;

	}

	return TRUE;
}

BOOL ValidateWpsPin(const CString strPin)
{
	USES_CONVERSION;

	// The WPS pin code is 8 digit characters
	if(strPin.GetLength() == WPS_PIN_LEN && strPin.SpanIncluding(DIGITCHARS).GetLength() == WPS_PIN_LEN)
	{
		return wps_api_validate_checksum(T2A((LPCTSTR)strPin));
	}

	return FALSE;
}

BOOL ValidateSsid(const CString strSsid)
{
	return (!strSsid.IsEmpty() && strSsid.GetLength() <= 32);
}

// Print trace info to debug Window
void print_traceMsg(int level, const char *lpszFile, int nLine, char *lpszFormat, ...)
{
	char szTraceMsg[2000];
	int cbMsg;
	va_list lpArgv;

	/* Format trace msg prefix */
	cbMsg = sprintf(szTraceMsg, "WpsWizard: %s(%d): ", lpszFile, nLine);

	/* Append trace msg to prefix. */
	va_start(lpArgv, lpszFormat);
	cbMsg = vsprintf(szTraceMsg + cbMsg, lpszFormat, lpArgv);
	va_end(lpArgv);

	OutputDebugStringA(szTraceMsg);
}
