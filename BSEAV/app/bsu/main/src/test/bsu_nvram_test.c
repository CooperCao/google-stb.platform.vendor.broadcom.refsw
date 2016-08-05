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
#include <string.h>
#include "bstd.h"
#include "bsu_prompt.h"
#include "bsu_nvram.h"
#include "bsu-api.h"
#include "bsu-api2.h"

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/

/***********************************************************************
 *                      External References
 ***********************************************************************/

extern uint64_t xtoq(const char *dest);

#ifdef NO_OS_DIAGS
    extern void bkgdFunc(void *p_generic);
#endif

/***********************************************************************
 *                      Function Prototypes
 ***********************************************************************/
#if __cplusplus
extern "C" {
#endif

void bcmSataTest (void);

#if __cplusplus
}
#endif

/***********************************************************************
 *                      Global variables
 ***********************************************************************/

 /***********************************************************************
 *
 *  bsu_nvram_test()
 *
 *  Test BSU nvram functions.
 *
 ***********************************************************************/
void bsu_nvram_test (void)
{
    static char command_id;
    int res;
    static int read_offset=0;
    static int write_offset=0;
    char str[20];
    char *eptr;
    char partition_name[20];
    uint8_t buffer[512];

    strcpy(partition_name, "flash0.nvram");
#ifdef MIPS_SDE
    cfe_set_envdevname(partition_name);
#else
    bolt_set_envdevname(partition_name);
#endif
    res = bsu_nvram_open();

    while (1)
    {
        printf("\n\n");
        printf("================\n");
        printf("  FLASH MENU  \n");
        printf("================\n");
        printf("    0) Exit\n");
        printf("    1) Read from flash partition (starting offset=0x%x)\n", read_offset);
        printf("    2) Write to flash partition (starting offset=0x%x)\n", write_offset);
        printf("    3) Change read starting offset\n");
        printf("    4) Change write starting offset\n");
        printf("    5) Change partition (current partition=%s)\n", partition_name);
        printf("    6) Erase flash\n");

        #ifdef NO_OS_DIAGS
            command_id = BkgdPromptChar( bkgdFunc, (void *) NULL );
        #else
            command_id = PromptChar();
        #endif

        switch(command_id)
        {
            case '0':
                bsu_nvram_close();
                return;

            case '1':
                res = bsu_nvram_read(buffer,read_offset,sizeof(buffer));
                if (res != 0) {
                    unsigned int i;
                    if (res != sizeof(buffer))
                        printf("res=%d, sizeof(buffer)=%d\n", res, sizeof(buffer));
                    for (i=0; i<sizeof(buffer); i++) {
                        if (i%16==0) printf("%08x:  ", read_offset+i);
                        printf("%02x ", buffer[i]);
                        if (i%16==15) printf("\n");
                    }
                    read_offset += res;
                }
                else {
                    printf("error from nvram_read\n");
                }

                break;

            case '2':
                memset(buffer, 0, sizeof(buffer));
                printf("writing %d bytes at offset 0x%x\n", sizeof(buffer), write_offset);
                res = bsu_nvram_write(buffer,write_offset,sizeof(buffer));
                if (res != 0) {
                    if (res != sizeof(buffer))
                        printf("res=%d, sizeof(buffer)=%d\n", res, sizeof(buffer));
                    write_offset += res;
                }
                else
                    printf("error from nvram_read\n");
                break;

            case '3':
                console_readline("enter read starting offset in hex:  ", str, sizeof(str));
                read_offset = strtoul(str, &eptr, 16);
                break;

            case '4':
                console_readline("enter write starting offset in hex:  ", str, sizeof(str));
                write_offset = strtoul(str, &eptr, 16);
                break;

            case '5':
                printf("enter parition name:  ");
                rdstr(partition_name);
                printf("\nnew partition name=%s\n", partition_name);
                bsu_nvram_close();
#ifdef MIPS_SDE
                cfe_set_envdevname(partition_name);
#else
                bolt_set_envdevname(partition_name);
#endif
                bsu_nvram_open();
                break;

            case '6':
                printf("erasing flash...");
                bsu_nvram_erase();
                break;

            default:
                printf("invalid selection\n");
                break;
        }
    }
}
