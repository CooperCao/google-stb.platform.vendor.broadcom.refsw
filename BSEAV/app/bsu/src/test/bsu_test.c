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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
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
#ifdef BSU_MOCA_TEST
extern void bsu_moca_test(void);
#endif
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
        printf( "Copyright (C) Broadcom Corporation 2003. All rights reserved.\n");

        printf("    1) Front end test\n");
        printf("    2) Display color bar\n");
        printf("    3) Message test\n");
        printf("    4) USB test\n");
        printf("    5) Flash test\n");
        printf("    6) Ethernet test\n");
        printf("    7) UART test\n");
        printf("    8) IR input test\n");
#ifdef BSU_MOCA_TEST
        printf("    9) MoCA test\n");
#endif
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
                bsu_uart_test();
                break;

            case '8':
                bsu_ir_input_test();
                break;

#ifdef BSU_MOCA_TEST
            case '9':
                bsu_moca_test();
                break;
#endif

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
                                NEXUS_Platform_Uninit();
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
