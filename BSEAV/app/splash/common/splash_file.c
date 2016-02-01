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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "splash_magnum.h"
#include "splash_file.h"
#include <stdio.h>

BDBG_MODULE(splash_bmp_file);

/* this function is for splashgen and splashrun only */
uint8_t *splash_open_bmp(char *name)
{
    unsigned long end = 0;
    uint8_t *buffer ;
    int rc;
    FILE *fp = fopen(name, "rb");
    if(!fp)
    {
        perror("Failed opening file ");
        return NULL;
    }
    rc = fseek(fp, 0, SEEK_END );
    if(rc)
    {
        BDBG_ERR(("Error fseek"));
        return NULL ;
    }
    end = ftell(fp);

    rc = fseek(fp, 0, SEEK_SET );

    buffer = (uint8_t *)BKNI_Malloc(end);
    rc = fread(buffer, sizeof(char), end, fp);
    if(!rc)
    {
        BDBG_ERR(("fread failed"));
        return NULL ;
    }
    return buffer;
}

