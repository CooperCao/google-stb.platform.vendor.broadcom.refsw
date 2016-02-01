/***************************************************************************
 *     Copyright (c) 2007, Broadcom Corporation
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
 * Interface for alloc libraries
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "balloc.h"

static void *
b_kni_alloc(balloc_iface_t alloc, size_t size)
{
	BSTD_UNUSED(alloc);
	return BKNI_Malloc(size);
}

static void
b_kni_free(balloc_iface_t alloc, void *ptr)
{
	BSTD_UNUSED(alloc);
	BKNI_Free(ptr);
}

const struct balloc_iface bkni_alloc[] = {
	{b_kni_alloc, b_kni_free}
};

