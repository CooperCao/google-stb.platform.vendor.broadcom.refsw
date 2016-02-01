/***************************************************************************
 *     Copyright (c) 2002-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: bdbg.c $
 * $brcm_Revision: Hydra_Software_Devel/1 $
 * $brcm_Date: 2/11/11 3:35p $
 *
 * Module Description:
 *
 * THIS IS A STUB of bdbg.c. This is only used for the xvd_save_FW_Image
 * program.
 *
 * Revision History:
 *
 * $brcm_Log: /rockford/applications/xvd_save_FW_image/bdbg.c $
 *
 * Hydra_Software_Devel/1   2/11/11 3:35p davidp
 * SW7422-22: Initial checkin for xvd_save_image FW signing tool.
 *
 *
 ***************************************************************************/

#include <stdarg.h>
#include <stdio.h>

int
BDBG_P_Vprintf(const char *fmt, va_list ap)
{
    return vfprintf(stdout, fmt, ap);
}

void
BDBG_P_PrintString(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    BDBG_P_Vprintf(fmt, ap);

    va_end( ap );

    return;
}
