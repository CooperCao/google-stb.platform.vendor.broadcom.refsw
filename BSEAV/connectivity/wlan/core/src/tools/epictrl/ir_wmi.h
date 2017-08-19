/*
 * Copyright 2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 *
 * $Id$
 *
 * Exposed API for the WMI <-> driver interface.  
 * Provides OID access to network adapters.
 *
 */


#ifndef _ir_wmi_h_
#define _ir_wmi_h_

extern DWORD WMIOpen (VOID);
extern DWORD WMICmd (BOOL setoid, LPCTSTR friendlyname, ULONG cmd, PVOID iobuf, DWORD *iobufsize);
extern VOID WMIClose (VOID);

#endif /* _ir_wmi_h_ */
