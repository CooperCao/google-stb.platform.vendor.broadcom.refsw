/*
 * c_main
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
#include <tx_api.h>

void c_main(void);

/* C entry function from assembly */
void
c_main(void)
{
	tx_kernel_enter();
}
