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
 *  bsu_fs_init()
 * 
 *  BSU filesystem init
 *
 ***********************************************************************/
int bsu_fs_init(char *fsname,fileio_ctx_t **fsctx,void *device);

 /***********************************************************************
 *
 *  bsu_fs_uninit()
 * 
 *  BSU filesystem uninit
 *
 ***********************************************************************/
int bsu_fs_uninit(fileio_ctx_t *fsctx);

 /***********************************************************************
 *
 *  bsu_fs_open()
 * 
 *  BSU open interface
 *
 ***********************************************************************/
void *bsu_fs_open(fileio_ctx_t *fsctx,char *filename,int mode);

 /***********************************************************************
 *
 *  bsu_fs_close()
 * 
 *  BSU close interface
 *
 ***********************************************************************/
int bsu_fs_close(fileio_ctx_t *fsctx,void *ref);

 /***********************************************************************
 *
 *  bsu_fs_seek()
 * 
 *  BSU seek interface
 *
 ***********************************************************************/
int bsu_fs_seek(fileio_ctx_t *fsctx,void *ref,int offset,int how);

 /***********************************************************************
 *
 *  bsu_fs_read()
 * 
 *  BSU read interface
 *
 ***********************************************************************/
int bsu_fs_read (fileio_ctx_t *fsctx,uint8_t *buffer,int size,int len,void *ref);

 /***********************************************************************
 *
 *  bsu_fs_write()
 * 
 *  BSU write interface
 *
 ***********************************************************************/
int bsu_fs_write(fileio_ctx_t *fsctx,void *ref,uint8_t *buffer,int len);
