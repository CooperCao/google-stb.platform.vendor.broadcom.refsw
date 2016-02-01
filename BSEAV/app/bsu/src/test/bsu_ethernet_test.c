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
#include "bsu_net.h"
#include "bsu_error.h"
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

int bsu_ethernet_tftp_test(void)
{
    char *fname="stb-irva-09:/agin/src/a.txt";
    fileio_ctx_t *fsctx;
    void *filectx;
    uint8_t buffer[1024];
    int res;
    int total;
    int count;

    res = bsu_fs_init("tftp",&fsctx,"");
    if (res < 0) {
        printf("Could not init file system\n");
        return -1;
    }

    filectx = bsu_fs_open(fsctx,fname,FILE_MODE_READ);
    if (!filectx) {
        printf("Could not open %s",fname);
        return -1;
    }
    else {
    total = 0;
    count = 0;
    for (;;) {
        res = bsu_fs_read(fsctx,buffer,1,sizeof(buffer),filectx);
        if (res < 0) break;

        /* print out the bytes in a nice format */
        printf("\n");
        if (res > 0) {
            int i,j;
            for (i=0; i<res; i++) {
                printf("%02x", buffer[i]);
                if (((i+1)%16==0)) {
                    printf(" : ");
                    for (j=i-15; j<res; j++) {
                        printf("%c", buffer[j]);
                        if ((j+1)%16==0) {
                            break;
                        }
                    }
                    printf("\n");
                }
                else if (i==res-1) {
                    int k, rem;
                    rem=(16-res%16);
                    for (k=0; k<rem; k++)
                        printf("  ");
                    printf(" : ");
                    for (j=i-(15-rem); j<res; j++)
                        printf("%c", buffer[j]);
                }
            }
            printf("\n\n");
        }

        total += res;
        if (res != sizeof(buffer)) break;
        count++;
        if (count == 256) {
        printf(".");
        count = 0;
        }
        }
    if (res < 0) printf("read error %s\n",bsu_errortext(res));
    else printf("Total bytes read: %d\n",total);
    bsu_fs_close(fsctx,filectx);
    }

    bsu_fs_uninit(fsctx);
    return 0;
}

int bsu_ethernet_ifconfig(void)
{
    int err;	
    char *devname = "eth0";

    bsu_net_uninit();

    err = bsu_net_init(devname);
    if (err < 0) {
    printf("Could not activate device %s: %s\n",
        devname,bsu_errortext(err));
    return err;
    }

    return bsu_do_dhcp_request(devname);
}

void bsu_ethernet_test (void)
{
    static char command_id;

    while (1)
    {
        printf("\n\n");
        printf("=================\n");
        printf("  Ethernet Menu  \n");
        printf("=================\n");
        printf("    0) Exit\n");
        printf("    1) ifconfig eth0 -auto\n");
        printf("    2) TFTP test\n");

        #ifdef NO_OS_DIAGS
            command_id = BkgdPromptChar( bkgdFunc, (void *) NULL );
        #else
            command_id = PromptChar();
        #endif

        switch(command_id)
        {
            case '0':
                return;

            case '1':
                bsu_ethernet_ifconfig();
                break;

            case '2':
                bsu_ethernet_tftp_test();
                break;

            default:
                printf("Invalid selection\n");
                break;
        }
    }
}

