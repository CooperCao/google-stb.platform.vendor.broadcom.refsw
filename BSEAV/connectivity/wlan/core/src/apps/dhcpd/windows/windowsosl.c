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
 * $Id: windowsosl.c,v 1.4 2009-09-10 21:27:03 $
 */

#include <windows.h>
#include <time.h>
#include <stdio.h>

static CRITICAL_SECTION cs;

unsigned long OslGetSeconds() {
	return (unsigned long) time(NULL);
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
