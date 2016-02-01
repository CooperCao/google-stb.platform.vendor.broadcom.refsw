/***************************************************************************
 *	   Copyright (c) 2004-2009, Broadcom Corporation
 *	   All Rights Reserved
 *	   Confidential Property of Broadcom Corporation
 *
 *	THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *	AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *	EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef SIMPLEINIT_H
#define SIMPLEINIT_H

#include "bstd.h"
#include "bchp.h"
#include "bchp_common.h"

#if (BCHP_CHIP == 7400)
#include "bchp_7400.h"
#include "bint_7400.h"
#endif

#if (BCHP_CHIP == 7401)
#include "bchp_7401.h"
#include "bint_7401.h"
#endif

#include "bdbg.h"
#include "breg_mem.h"
#include "bint_plat.h"


#include "bmem.h"
#include "btmr.h"
#include "bgrc.h"
#include "bsur.h"
#include "bpxl.h"
#include "bvdc.h"

#include "bi2c.h"
#include "bspi.h"
#include "bicp.h"
#include "birb.h"
#include "bkir.h"
#include "bkpd.h"
#include "bled.h"
#include "bgio.h"
#include "bxpt.h"
#include "bxvd.h"

#include "bhdm.h"
#include "brap.h"


#include "bxpt_pcr.h"

#include "bxpt_rave.h"
#include "bxpt_pcr_offset.h"


#include "bpcrlib.h"

#define MAX_TSFS_PATH_LEN 200

/* How often the "idle" event must be inserted into the TPIT data for RASP (ms) */
#define TPIT_PACKET_TIMEOUT_MS (50)
#define TPIT_CLOCK_RATE        (108000000)                 /* 108MHz clock rate */
#define TPIT_TICKS_PER_MS      (TPIT_CLOCK_RATE / 1000)

/* Utility to make paths for opening files */
char*				MakePathToTgtSvr(char* subpath, char* path);

/* Primary initialization routine */
BERR_Code 			InitApp(void);

/* Routines for testing */
BERR_Code			CloseAndReopenGIO(void);
BERR_Code			CloseAndReopenURT(void);

#endif
