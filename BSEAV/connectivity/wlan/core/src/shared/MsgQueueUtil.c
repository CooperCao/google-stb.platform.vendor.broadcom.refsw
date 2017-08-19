/*
 * MsgQueueUtil.c
 * Windows CE message queue utilities
 *
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
 *
 */

#include <windows.h>
#include "MsgQueueUtil.h"


HANDLE MsgQueueCreate(char *stringIn, bool rw)
{
	HANDLE qhandle = NULL;
	wchar_t unicodeIfstr[100];

	MultiByteToWideChar(CP_ACP, 0, stringIn, -1, unicodeIfstr, 100);

	if (unicodeIfstr[0])
	{
		MSGQUEUEOPTIONS	QueueOptions;

		memset(&QueueOptions, 0, sizeof(QueueOptions));
		QueueOptions.dwSize        = sizeof(QueueOptions);

		/* MSGQUEUE_NOPRECOMMIT */
		QueueOptions.dwFlags       = MSGQUEUE_ALLOW_BROKEN;
		QueueOptions.dwMaxMessages = 100;
		QueueOptions.cbMaxMessage  = 1000;

		/* TRUE means read access, FALSE means WRITE access */
		QueueOptions.bReadAccess   = rw;
		qhandle = CreateMsgQueue(unicodeIfstr, &QueueOptions);

	}

	return (qhandle);
}

void MsgQueueDelete(HANDLE qhandle)
{
	CloseMsgQueue(qhandle);

}

BOOL MsgQueueSend(HANDLE hWriteMsgQueue, LPVOID pMsgData, DWORD dwMsgLen)
{
	BOOL	bRet = FALSE;

	if (hWriteMsgQueue != INVALID_HANDLE_VALUE)
	{
		bRet = WriteMsgQueue(hWriteMsgQueue, pMsgData, dwMsgLen, 0, 0);
	}

	return (bRet);
}

/* pMsgData		- Returns the data read
 * pdwBytesRead	- Pass in the size of the pMsgData buffer,
 * returns the # of bytes read
 */
BOOL MsgQueueRead(HANDLE hReadMsgQueue, LPVOID pMsgData, LPDWORD pdwBytesRead)
{
	BOOL	bRet = FALSE;

	if (hReadMsgQueue != INVALID_HANDLE_VALUE)
	{
		DWORD dwFlags = 0;

		bRet = ReadMsgQueue(hReadMsgQueue, pMsgData, *pdwBytesRead,
			pdwBytesRead, INFINITE, &dwFlags);
	}

	return (bRet);
}

static char    MsgData[1000];
BOOL MsgQueueFlush(HANDLE qhandle)
{
	BOOL	bRet;
	DWORD	dwFlags;
	DWORD	BytesRead;

	for (; qhandle != NULL; )
	{
		bRet = ReadMsgQueue(qhandle, MsgData, 1000, &BytesRead, 0, &dwFlags);
		if ((FALSE == bRet) || (BytesRead == 0)) {
			break;
		}
	}
	return TRUE;
}

BOOL MsgQueueGetInfo(HANDLE hMsgQueue, LPMSGQUEUEINFO pMsgQueueInfo)
{
	BOOL	bRet = FALSE;

	if (hMsgQueue != INVALID_HANDLE_VALUE)
	{
		bRet = GetMsgQueueInfo(hMsgQueue, pMsgQueueInfo);
	}
	return (bRet);
}
