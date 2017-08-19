/*
 * Patch values and address for Broadcom 802.11abgn
 * Networking Adapter Device Drivers.
 *
 * If patchtable0 is used functionality of bcm_atoi breaks
 * enabling the patch by programming patch instructions with nops
 * introduced as part of this function.
 *
 * If patchtable1 is used functionality remains unchanged. Hence even
 * enabling the patch and loading patch instructions bcm_atoi behaves
 * sane.
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
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <sbsocram.h>
#include "patchtable.h"


/* All patch table should be of SRPC_PATCHNLOC size
 * Hence if patch is less than SRPC_PATCHNLOC then it is neccessary to have
 * table loaded with consecutive working instruction.
 */

/* Replace atoi function disassembly by introducing nop
 * during branch call to string to ulong
 */
CONST  patchaddrvalue_t  BCMATTACHDATA(patchtbl0)[SRPC_PATCHNLOC] = {
					{0x1e0017ec, 0x2100b500},
					{0x1e0017f0, 0x220ab081},
					{0x1e0017f4, 0x46c046c0}, /* Nops */
					{0x1e0017f8, 0xbd00b001}
				};

/* Actual assembly of bcm_atoi
const  patchaddrvalue_t patchtbl1[SRPC_PATCHNLOC] = {
					{0x1e0017ec, 0x2100b500},
					{0x1e0017f0, 0x220ab081},
					{0x1e0017f4, 0xff8cf7ff},
					{0x1e0017f8, 0xbd00b001}
				      };
*/

void
BCMATTACHFN(hnd_patch_init)(void *srp)
{
	sbsocramregs_t *sr = (sbsocramregs_t *)srp;
	hnd_tcam_load((void *)sr, patchtbl0);
}
