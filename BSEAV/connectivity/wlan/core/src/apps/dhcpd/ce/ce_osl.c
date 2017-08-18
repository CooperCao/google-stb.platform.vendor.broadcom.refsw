/*
 * Broadcom DHCP Server
 * OS specific routines
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: ce_osl.c,v 1.1 2010-01-04 13:06:54 $
 */

#include <windows.h>
#include <time.h>
#include <stdio.h>

static CRITICAL_SECTION cs;

static const __int64 SECS_BETWEEN_EPOCHS = 11644473600;
static const __int64 SECS_TO_100NS = 10000000; /* 10^7 */

unsigned long OslGetSeconds() {
	SYSTEMTIME systemTime;
	FILETIME fileTime;
	__int64 UnixTime;

	GetSystemTime( &systemTime );
	SystemTimeToFileTime( &systemTime, &fileTime );

	/* get the full win32 value, in 100ns */
	UnixTime = ((__int64)fileTime.dwHighDateTime << 32) + 
		fileTime.dwLowDateTime;

	/* convert to the Unix epoch */
	UnixTime -= (SECS_BETWEEN_EPOCHS * SECS_TO_100NS);

	UnixTime /= SECS_TO_100NS; /* now convert to seconds */

	return (long)(UnixTime);
}

void OslHandleAssert(char *fileName, int Line) {
	printf("Assert failed: File %s, Line %d\n", fileName, Line);
	exit(1);
}

void *OslCreateLock() {
	InitializeCriticalSection( &cs );

	return (void *) &cs;
}

void OslDeleteLock(void *Lock) {
	DeleteCriticalSection( (LPCRITICAL_SECTION) Lock );
}

void OslLock(void *Lock) {
	EnterCriticalSection( (LPCRITICAL_SECTION) Lock );

}

void OslUnlock(void *Lock) {
	LeaveCriticalSection( (LPCRITICAL_SECTION) Lock );
}
