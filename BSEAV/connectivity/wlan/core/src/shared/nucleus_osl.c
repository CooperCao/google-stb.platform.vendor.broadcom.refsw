/*
 * Nucleus OS Support Layer
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id$
 */

/* ---- Include Files ---------------------------------------------------- */

#include "typedefs.h"
#include "bcmdefs.h"
#include "bcmendian.h"
#include "bcmutils.h"
#include "nucleus_osl.h"
#include <nucleus.h>


/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
/* ---- Private Variables ------------------------------------------------ */
/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Functions -------------------------------------------------------- */

/* ----------------------------------------------------------------------- */
void osl_delay(uint usec)
{
	uint msec;
	uint ticks;

	if (usec == 0)
		return;

	/* Micro-second resolution not supported. Convert to milli-seconds. */
	if (usec < 1000) {
		msec = 1;
	}
	else {
		msec = usec / 1000;
	}

	ticks = OSL_MSEC_TO_TICKS(msec);
	NU_Sleep(ticks);
}
