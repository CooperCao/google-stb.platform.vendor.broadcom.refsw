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
#include "test.h"
#include <stdio.h>
#include <string.h>
#include "bstd.h"
#include "sys_handles.h"
#include "upg_handles.h"
#include "prompt.h"
#include "bchp_sun_top_ctrl.h"

/* For smartcard */
#ifdef DIAGS_SMARTCARD_TEST
#include "bscd.h"
#include "bscd_cli_states.h"
#include "bscd_cli_infra.h"
#include "bscd_cli_main.h"
#endif

#include "bicaptest.h"
#include "i2ctest.h"
#include "icaptest.h"
#include "irbtest.h"
#include "kirtest.h"
#include "ledtest.h"
#include "pwmtest.h"
#include "spitest.h"
#include "timertest.h"
#include "uhfrtest.h"
#include "kpdtest.h"

/* For smartcard */

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/
  

/***********************************************************************
 *                      External References
 ***********************************************************************/
#if __cplusplus
extern "C" {
#endif

#if __cplusplus
}
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
 *
 *  bcmUpgTests()
 * 
 *  Main UPG test menu
 *
 ***********************************************************************/
void bcmUpgTests(void)
{
    static char command_id;
        
    while (1)
    {
        printf("\n\n");
        printf("============================\n");
        printf("  PERIPHERAL I/O TEST MENU  \n");
        printf("============================\n");
        printf("    0) Exit\n");
        printf("    1) I2C Test\n");
        printf("    2) SPI Test\n");
        printf("    3) IR Receiver Test\n");
        printf("    4) IR Blaster Test\n");
        printf("    5) PWM Test\n");
        printf("    6) ICAP Test\n");
        printf("    7) BICAP Test\n");
        printf("    8) LED Test\n");
        printf("    9) Keypad Test\n");
        printf("    a) SmartCard Test\n");
        printf("    b) Timer Test\n");
        printf("    c) MODEM Test\n");
        printf("    d) UHF Test\n");
        printf("    e) UART Test\n");

        command_id = PromptChar();

        switch(command_id)
        {
            case '0':
                return;

            case '1':
                bcmI2cTest();
                break;

            case '2':
                bcmSpiTest();
                break;

            case '3':
                bcmKirTest();
                break;

            case '4':
                bcmIrbTest();
                break;

            case '5':
                bcmPwmTest();
                break;

            case '7':
                bcmBicapTest ();
                break;

            case '8':
                bcmLedTest();
                break;

            case '9':
                bcmKpdTest();
                break;

            case 'a':
                #ifdef DIAGS_SMARTCARD_TEST
                    BSCD_CLI_TopMenu (BSCD_CLI_GetScdCliModuleHandle());
                #else
                    printf("Not enabled\n");
                #endif
                break;

            case 'b':
                bcmTimerTest();
                break;

            case 'c':
                #ifdef MODEM_TEST
                    modemTest("7038");
                #else
                    printf("Not enabled\n");
                #endif
                break;

            case 'd':
                uhfr_test();
                break;

            case 'e':
                #ifdef DIAGS_UART_TEST
                    uartTest();
                #else
                    printf("Not enabled\n");
                #endif
                break;

            default:
                printf("Please enter a valid choice\n");
                break;
        }
    }
}

