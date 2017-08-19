/*
 * OS independent remote wl declarations
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef _wlu_remote_vista_h
#define _wlu_remote_vista_h

#define WINERR	DWORD
/* Supress deprecated warnings */
#define stricmp _stricmp
#define strnicmp _strnicmp
#define itoa _itoa

extern DWORD ndevs;
extern GUID dev;
extern GUID devlist[];
extern void display_err(PCHAR prefix, WINERR status);
extern WINERR select_adapter(HANDLE irh, int adapter);
extern int remote_server_exec(int argc, char **argv, void *irh);
extern cmd_t vista_cmds[];
extern int
wl_set(void *wl, int cmd, void *buf, int len);
extern int
wl_get(void *wl, int cmd, void *buf, int len);
WINERR ir_vista_init(HANDLE *irh);
WINERR ir_vista_adapter_list(HANDLE irh, GUID *devlist, DWORD *ndevs);
void ir_vista_exit(HANDLE);
WINERR ir_vista_queryinformation(HANDLE m_dh, GUID *dev, ULONG oid, PBYTE inbuf, PDWORD inlen);
WINERR ir_vista_setinformation(HANDLE m_dh, GUID *dev, ULONG oid, PBYTE inbuf, PDWORD inlen);
WINERR ir_vista_adapter_reinitialize(LPCTSTR DeviceName);

#endif /* _wlu_remote_vista_h */
