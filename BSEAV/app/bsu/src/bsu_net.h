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
#if 0
#include "cfe_fileops.h"
#include "cfe_iocb.h"
#include "cfe_device.h"
#endif

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
 *  bsu_net_init()
 * 
 *  BSU network init
 *
 ***********************************************************************/
int bsu_net_init(char *devname);

 /***********************************************************************
 *
 *  bsu_net_uninit()
 * 
 *  BSU network uninit
 *
 ***********************************************************************/
void bsu_net_uninit(void);

 /***********************************************************************
 *
 *  bsu_do_dhcp_request()
 * 
 *  BSU DHCP request
 *
 ***********************************************************************/
int bsu_do_dhcp_request(char *devname);
