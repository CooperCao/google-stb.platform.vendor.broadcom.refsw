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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

/***********************************************************************/
/* Header files                                                        */
/***********************************************************************/
#include "bstd.h"

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/

/***********************************************************************
 *                      External References
 ***********************************************************************/

/***********************************************************************
 *                      Function Prototypes
 ***********************************************************************/
#if __cplusplus
extern "C" {
#endif

#if __cplusplus
}
#endif

/***********************************************************************
 *                      Global variables
 ***********************************************************************/

 /***********************************************************************
 *
 *  bsu_nvram_open()
 * 
 *  Open bsu interface to nvram.
 *
 ***********************************************************************/
int bsu_nvram_open(void);

 /***********************************************************************
 *
 *  bsu_nvram_close()
 * 
 *  Close bsu interface to nvram.
 *
 ***********************************************************************/
int bsu_nvram_close(void);

 /***********************************************************************
 *
 *  bsu_nvram_read()
 * 
 *  bsu nvram read interface
 *
 ***********************************************************************/
int bsu_nvram_read(unsigned char *buffer,int offset,int length);

 /***********************************************************************
 *
 *  bsu_nvram_write()
 * 
 *  bsu nvram write interface
 *
 ***********************************************************************/
int bsu_nvram_write(unsigned char *buffer,int offset,int length);

 /***********************************************************************
 *
 *  bsu_nvram_erase()
 * 
 *  bsu nvram erase interface
 *
 ***********************************************************************/
int bsu_nvram_erase(void);
