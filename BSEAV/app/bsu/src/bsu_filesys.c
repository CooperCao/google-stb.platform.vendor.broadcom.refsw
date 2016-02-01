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
#include "bsu.h"
#include "bsu-api.h"
#include "bsu-api2.h"

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

int bsu_fs_init(char *fsname,fileio_ctx_t **fsctx,void *device)
{
    return (fs_init(fsname,fsctx,device));
}

int bsu_fs_uninit(fileio_ctx_t *fsctx)
{
    return (fs_uninit(fsctx));
}

void *bsu_fs_open(fileio_ctx_t *fsctx,char *filename,int mode)
{
    void *ref;
    fs_open(fsctx, &ref, filename, mode);
    return ref;
}

int bsu_fs_close(fileio_ctx_t *fsctx,void *ref)
{
    return (fs_close(fsctx, ref));
}

int bsu_fs_seek(fileio_ctx_t *fsctx,void *ref,int offset,int how)
{
    return (fs_seek(fsctx, ref, offset, how));
}

int bsu_fs_read (fileio_ctx_t *fsctx,uint8_t *buffer,int size, int count,void *ref)
{
    int bytes_read = fs_read(fsctx,ref,buffer,size*count);
    return (bytes_read/size);
}

int bsu_fs_write(fileio_ctx_t *fsctx,void *ref,uint8_t *buffer,int len)
{
    BSTD_UNUSED(fsctx);
    BSTD_UNUSED(ref);
    BSTD_UNUSED(buffer);
    BSTD_UNUSED(len);
    return BSU_UNSUPPORTED;
}
