/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *   The source code for the stub ikos server.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include "utils.h"
#define P_UNUSED(x) ((void)x)

/* Environment and blocked signal state for restoration in a signal handler. */
static sigjmp_buf sj_env;

static void sigbus_hdl (int sig, siginfo_t *siginfo, void *ptr);


void jump_setup (void)
{
	struct sigaction act;
	memset (&act, 0, sizeof(act));
	act.sa_sigaction = sigbus_hdl;
	act.sa_flags = SA_SIGINFO;

	if (sigaction(SIGBUS, &act, 0)) {
		perror ("sigaction");
		exit (1);
	}

	if (sigaction(SIGSEGV, &act, 0)) {
		perror ("sigaction");
		exit (1);
	}
}

unsigned long deref32 (
	unsigned long seq, unsigned long addr, void* offset, unsigned long* regval)
{
	if (sigsetjmp(sj_env, 1))
	{
		*regval =  0xDEADC0DE;
	}
	else
	{
		*regval = *((volatile unsigned long *)
			((unsigned long)offset + (addr & BTST_REG_OFFSET_MASK)));
	}

	return seq + 1;
}

static void sigbus_hdl (int sig, siginfo_t *siginfo, void *ptr)
{
	/* Jump (goto) to the saved program state where we don't use mmapped()
	 * memory. */
P_UNUSED(sig);
P_UNUSED(siginfo);
P_UNUSED(ptr);
	siglongjmp (sj_env, 1);
}
