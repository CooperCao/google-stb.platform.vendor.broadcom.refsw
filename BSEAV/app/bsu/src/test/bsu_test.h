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

/***********************************************************************
    *
    *  bsu_menu()
    * 
    *  Main diagnostics menu for the BSU.
    *
    ***********************************************************************/
void bsu_menu(void);


#if __cplusplus
}
#endif

/***********************************************************************
 *                      Global variables
 ***********************************************************************/

 /***********************************************************************
 *
 *  bsu_usb_fs_open()
 * 
 *  Back end test function
 *
 ***********************************************************************/
int bsu_nvram_open(void);

 /***********************************************************************
 *
 *  bsu_usb_fs_open()
 * 
 *  Back end test function
 *
 ***********************************************************************/
int bsu_nvram_close(void);

 /***********************************************************************
 *
 *  bsu_usb_fs_open()
 * 
 *  Back end test function
 *
 ***********************************************************************/
int bsu_nvram_read(unsigned char *buffer,int offset,int length);

 /***********************************************************************
 *
 *  bsu_usb_fs_open()
 * 
 *  Back end test function
 *
 ***********************************************************************/
int bsu_nvram_write(unsigned char *buffer,int offset,int length);
