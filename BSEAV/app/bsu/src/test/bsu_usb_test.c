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
#include <stdio.h>
#include "bstd.h"
#include "bsu.h"
#include "bsu_prompt.h"
#include "bsu_filesys.h"
#include "bsu-api.h"
#include "bsu-api2.h"

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/

/***********************************************************************
 *                      External References
 ***********************************************************************/

#ifdef NO_OS_DIAGS
    extern void bkgdFunc(void *p_generic);
#endif

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

static fileio_ctx_t *fsctx_usb0;
static const char *fname = "The King's Speech LB.ts";

void bsu_usb_test (void)
{
    static char command_id;

    int count=0;
    int res;
    void *ref;
    uint8_t buffer[512*8];
    fsctx_usb0 = 0;

    while (1)
    {
        printf("\n\n");
        printf("================\n");
        printf("  USB MENU  \n");
        printf("================\n");
        printf("    0) Exit\n");
        printf("    1) Init USB with raw file system\n");
        printf("    2) Init USB with FAT file system\n");
        printf("    3) Read a FAT file system file from USB\n");
        printf("    4) Read raw from USB\n");
        printf("    5) Un-init USB file system\n");

        #ifdef NO_OS_DIAGS
            command_id = BkgdPromptChar( bkgdFunc, (void *) NULL );
        #else
            command_id = PromptChar();
        #endif

        switch(command_id)
        {
            case '0':
                if (fsctx_usb0) {
                    if (bsu_fs_uninit(fsctx_usb0) != 0) 
                        printf("error from bsu_fs_uninit\n");
                    else
                        printf("usb file system un-initialized\n");
                }
                return;

            case '1':
                if (bsu_fs_init("raw",&fsctx_usb0,"usbdisk0") == 0) {
                    printf("usb initialized with raw file system\n");
                }
                else {
                    printf("error from bsu_fs_init\n");
                    fsctx_usb0 = 0;
                }
                break;

            case '2':
                if (bsu_fs_init("fat",&fsctx_usb0,"usbdisk0") == 0) {
                    printf("usb initialized with fat file system\n");
                }
                else {
                    printf("error from bsu_fs_init\n");
                    fsctx_usb0 = 0;
                }
                break;

            case '3':
                ref = bsu_fs_open(fsctx_usb0, (char *)fname, FILE_MODE_READ);
                if (!ref) {
                    printf("error from bsu_fs_open\n");
                    break;
                }

                res = bsu_fs_seek(fsctx_usb0, ref, 0, FILE_SEEK_BEGINNING);
                if (res < 0) {
                    printf("error from bsu_fs_seek\n");
                    break;
                }

                res = bsu_fs_read(fsctx_usb0, buffer, 1, sizeof(buffer), ref);
                if (res <0 ) break;

                for (;;) 
                {
                    res = bsu_fs_read(fsctx_usb0, buffer, 1, sizeof(buffer), ref);
                    if (res < 0) break;
                    if (res != sizeof(buffer)) break;
                    if( det_in_char() > 0 ) break;
                    count++;
                    if ((count % 256) == 0)
                    {
                        printf(".");
                    }
                }

                res = fs_close(fsctx_usb0, ref);
                if (res < 0) {
                    printf("error from fs_close\n");
                }
                break;

            case '4':
                ref = bsu_fs_open(fsctx_usb0, "usbdisk0", FILE_MODE_READ);
                if (!ref) {
                    printf("error from bsu_fs_open\n");
                    break;
                }

                res = bsu_fs_seek(fsctx_usb0, ref, 0, FILE_SEEK_BEGINNING);
                if (res < 0) {
                    printf("error from bsu_fs_seek\n");
                    break;
                }

                res = bsu_fs_read(fsctx_usb0, buffer, 1, sizeof(buffer), ref);
                if (res <0 ) break;

                for (;;) 
                {
                    res = bsu_fs_read(fsctx_usb0, buffer, 1, sizeof(buffer), ref);
                    if (res < 0) break;
                    if (res != sizeof(buffer)) break;
                    if( det_in_char() > 0 ) break;
                    count++;
                    if ((count % 256) == 0)
                    {
                        printf(".");
                    }
                }

                res = fs_close(fsctx_usb0, ref);
                if (res < 0) {
                    printf("error from fs_close\n");
                }
                break;

            case '5':
                if (fsctx_usb0) {
                    if (bsu_fs_uninit(fsctx_usb0) != 0) printf("error from bsu_fs_uninit\n");
                    fsctx_usb0 = 0;
                    printf("usb file system un-initialized\n");
                }
                break;

            default:
                printf("Invalid selection\n");
                break;
        }
    }
}

