/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: WpsWizardUtil.h,v 1.2 2008/08/25 21:35:13 Exp $
 */
#ifndef __WPSWIZARDUTIL_H__
#define __WPSWIZARDUTIL_H__

void SetLabel(HWND hWnd, UINT uControlId, UINT uStrId);
BOOL ValidatePSKPassword(const CString& password);
BOOL ValidateWEPPassword(const CString& password);
BOOL ValidateWpsPin(const CString strPin);
BOOL ValidateSsid(const CString strSsid);

/* trace levels */
#define TUINFO  0x0001
#define TUERR   0x0010

#define TUTRACE_ERR        TUERR, __FILE__, __LINE__
#define TUTRACE_INFO       TUINFO, __FILE__, __LINE__

#define TUTRACE(VARGLST)   print_traceMsg VARGLST

void print_traceMsg(int level, const char *lpszFile, int nLine, char *lpszFormat, ...);

#endif  // __WPSWIZARDUTIL_H__
