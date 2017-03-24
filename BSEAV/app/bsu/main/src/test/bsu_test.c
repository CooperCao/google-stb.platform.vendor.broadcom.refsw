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
 * Module Description:
 *
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include "bsu_version.h"
#include "bsu_prompt.h"
#include "nexus_types.h"
#ifdef MIPS_SDE
#include "cfe.h"
#else
#include "bolt.h"
#endif
#include "bsu-api.h"
#include "bsu-api2.h"
#ifndef MIPS_SDE
#include "interrupts.h"
#endif
#include "nexus_platform_client.h"

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/

/***********************************************************************
 *                      External References
 ***********************************************************************/
#if __cplusplus
extern "C" {
#endif

extern void bsu_colorbar_test(void);
extern void bsu_message_test(void);
extern void bsu_nvram_test(void);
extern void bsu_satellite_test(void);
extern void bsu_usb_test (void);
extern void bsu_ethernet_test (void);
extern void bsu_ofdm_test (void);
extern void bsu_qam_test(void);
extern void bsu_uart_test (void);
extern void bsu_ir_input_test(void);
extern void bsu_moca_test(void);
extern void mini_exit(void);


#if __cplusplus
}
#endif

/***********************************************************************
 *                        Local Functions
 ***********************************************************************/

/***********************************************************************
    *
    *  bsu_Menu()
    *
    *  Main test menu for BSU
    *
    ***********************************************************************/
void bsu_menu(void)
{
    static char command_id;

    while (1)
    {
        printf( "\n\nBCM%d Set Top Boot Software Updater (BSU) Code - Version %s, Compiled on %s, %s\n",
                        BCM_BOARD, EVAL_VERSION, __DATE__, __TIME__);
        printf( "Copyright (C) Broadcom 2016. All rights reserved.\n");

        printf("    1) Front end test\n");
        printf("    2) Display color bar\n");
        printf("    3) Message test\n");
        printf("    4) USB test\n");
        printf("    5) Flash test\n");
        printf("    6) Ethernet test\n");
        printf("    7) UART test\n");
        printf("    8) IR input test\n");
        printf("    9) MoCA test\n");
#ifdef MIPS_SDE
        printf("    g) CFE command shell\n");
#else
        printf("    g) BOLT command shell\n");
        printf("    h) Exit to BOLT\n");
#endif

        command_id = PromptChar();

        switch(command_id)
        {
            case '1':
                #if defined(BSU_OFDM_TEST) || defined(BSU_QAM_TEST) || defined(BSU_SAT_TEST)
                    #ifdef BSU_OFDM_TEST
                        bsu_ofdm_test();
                    #endif
                    #ifdef BSU_QAM_TEST
                        bsu_qam_test();
                    #endif
                    #ifdef BSU_SAT_TEST
                        bsu_satellite_test();
                    #endif
                #else
                    printf("Not enabled\n");
                #endif
                break;

            case '2':
                bsu_colorbar_test();
                break;

            case '3':
                bsu_message_test();
                break;

            case '4':
                bsu_usb_test();
                break;

            case '5':
                bsu_nvram_test();
                break;

            case '6':
                bsu_ethernet_test();
                break;

            case '7':
#ifdef BSU_UART_TEST
                bsu_uart_test();
#else
                printf("not supported\n");
#endif
                break;

            case '8':
                bsu_ir_input_test();
                break;

            case '9':
#ifdef BSU_MOCA_TEST
                bsu_moca_test();
#else
                printf("not supported\n");
#endif
                break;

            case 'g':
                {
#define BUFSIZE 128
                    char buffer[BUFSIZE];
                    char buffer2[BUFSIZE];
                    int status;
                    char *p;
                    int i;

                    for (;;) {
                        console_readline("BSU> ", buffer, sizeof(buffer));
                        if (strcmp(buffer,"quit") == 0) break;;

                        /* Determine if command is "boot" or "go" */
                        memcpy(buffer2, buffer, BUFSIZE);
                        p = strtok(buffer2, " ");
                        i=0;
                        if (p == NULL) {
                            while ((p = strtok(NULL, " ")) == NULL) {
                                i++;
                                if (i>BUFSIZE) {
                                    break;
                                }
                            };
                        }
                        if (p) {
                            if (!strcmp(p, "boot") || !strcmp(p, "go")) {
#ifdef BSU_OSD_ACROSS_BOOT
                                NEXUS_Platform_UninitInterrupts();
#else
                                NEXUS_Platform_Uninit();
#endif
#ifndef MIPS_SDE
                                interrupts_uninit();
#endif
                            }
                        }

#ifdef MIPS_SDE
                        status = cfe_docommands(buffer);
#else
                        status = bolt_docommands(buffer);
#endif
                        if (status != CMD_ERR_BLANK) {
                            printf("bsu command status = %d\n", status);
                        }
                    }
                }
                break;

#ifndef MIPS_SDE
            case 'h':
                NEXUS_Platform_Uninit();
                interrupts_uninit();
                mini_exit();
                break;
#endif

            default:
                printf("Please enter a valid choice\n");
                break;
        }
    }
}