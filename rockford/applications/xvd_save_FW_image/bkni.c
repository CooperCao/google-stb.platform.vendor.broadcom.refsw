/***************************************************************************
 *     Copyright (c) 2002-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: bkni.c $
 * $brcm_Revision: Hydra_Software_Devel/1 $
 * $brcm_Date: 2/11/11 3:38p $
 *
 * Module Description:
 *
 * THIS IS A STUB of bkni.c. This is only used for the xvd_save_FW_Image program.
 *
 * Revision History:
 *
 * $brcm_Log: /rockford/applications/xvd_save_FW_image/bkni.c $
 *
 * Hydra_Software_Devel/1   2/11/11 3:38p davidp
 * SW7422-22: Initial checkin for xvd_save_image FW signing tool.
 *
 ***************************************************************************/

#include <string.h>
#include <stdlib.h>

void *
BKNI_Malloc(size_t size)
{
    return malloc(size);
}

void
BKNI_Free(void *ptr)
{
    free(ptr);
    return;
}

void *
BKNI_Memset(void *b, int c, size_t len)
{
    return memset(b, c, len);
}

void *
BKNI_Memcpy(void *dst, const void *src, size_t len)
{
    return memcpy(dst, src, len);
}
