/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * [File Description:]
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
