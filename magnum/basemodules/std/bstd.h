/***************************************************************************
 *     Copyright (c) 2003-2006, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BSTD_H__
#define BSTD_H__

/* standard types */
#include "bstd_defs.h"
typedef uint64_t BSTD_DeviceOffset;

/* standard defines */
#define BSTD_ENDIAN_BIG 4321
#define BSTD_ENDIAN_LITTLE 1234

/* platform specific include file. This must
come before other basemodules which may depend upon #defines. 
Setting compile options in bstd_cfg.h or on the command line
should be equivalent. */
#include "bstd_cfg.h"

/* base error codes */
#include "berr.h"

/* debug interface */
#include "bdbg.h"

/* chip interface */
#include "bchp.h"

/*
 * insuring we have a proper configuration
 */

#if ((BSTD_CPU_ENDIAN != BSTD_ENDIAN_BIG) && \
     (BSTD_CPU_ENDIAN != BSTD_ENDIAN_LITTLE))
#error Must define BSTD_CPU_ENDIAN as BSTD_ENDIAN_BIG or BSTD_ENDIAN_LITTLE.
#endif

#include "bstd_file.h"

#endif /* #ifndef BSTD_H__ */

/* end of file */
