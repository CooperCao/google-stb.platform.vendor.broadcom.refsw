/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

/* Simple RF4CE app */

#include "rf4ce_common.h"


const char bindingInstruction[] = "\nNow a remote control can be bound\n\n"
                                  "Binding instruction for RemoteSolution remote control:\n"
                                  "1. Press and hold the Setup button on the remote control,\n"
                                  "   until the Indicator LED changes from red to green color.\n"
                                  "2. Press the Info button on the remote control.\n"
                                  "Above procedure should be executed within 30 seconds.\n";

const char prebindingInstruction[] = "\nPlease press 'b' to issue binding procedure,\n"
                                     "press any other key to use the existing pair references.\n\n";



int main(int argc, char *argv[])
{
    struct zigbeeCallback zcb;
    RF4CE_StartReqDescr_t request;
    static struct termios oldt, newt;

#ifdef BYPASS_RPC
    extern int zigbee_init(int argc, char *argv[]);
    zigbee_init(argc, argv);
#endif

    /*tcgetattr gets the parameters of the current terminal
    STDIN_FILENO will tell tcgetattr that it should write the settings
    of stdin to oldt*/
    tcgetattr( STDIN_FILENO, &oldt);
    /*now the settings will be copied*/
    newt = oldt;

    /*ICANON normally takes care that one line at a time will be processed
    that means it will return if it sees a "\n" or an EOF or an EOL*/
    newt.c_lflag &= ~(ICANON);

    /*Those new settings will be set to STDIN
    TCSANOW tells tcsetattr to change attributes immediately. */
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    /* Register the callback functions you are interested in.  Ones that are not filled out, won't be called back. */
    /* Calling Zigbee_GetDefaultSettings will initialize the callback structure */
    Zigbee_GetDefaultSettings(&zcb);
    zcb.RF4CE_PairInd = BroadBee_ZRC_PairInd;
    zcb.RF4CE_ZRC2_CheckValidationInd = BroadBee_ZRC2_CheckValidationInd;
    zcb.RF4CE_ZRC2_ControlCommandInd = BroadBee_ZRC2_ControlCommandInd;
    zcb.RF4CE_ZRC1_ControlCommandInd = BroadBee_ZRC1_ControlCommandInd;
    zcb.RF4CE_ZRC1_VendorSpecificInd = BroadBee_ZRC1_VendorSpecificInd;
    zcb.SYS_EventNtfy = BroadBee_ZRC_EventNtfy;

    Zigbee_Open(&zcb, argv[1]);

#ifdef BYPASS_RPC
    printf("press any key to proceed...\n");
    getchar();
    BroadBee_ZRC_Restore_Factory_Settings(1);

    //printf("press any key to proceed...\n");
    //getchar();
    //BroadBee_SYS_Get_Fw_Rev();
#endif

    printf(prebindingInstruction);
    char key = getchar();
    switch(key){
        case 'b':
            BroadBee_ZRC_Set_WakeUpActionCode();
            BroadBee_ZRC_Get_WakeUpActionCode();
            BroadBee_ZRC_Set_SupportedDevices();
            BroadBee_ZRC_Start_NWK();
            printf(bindingInstruction);
            BroadBee_ZRC1_TargetBinding();
            break;
        default:
            BroadBee_ZRC_Set_WakeUpActionCode();
            BroadBee_ZRC_Get_WakeUpActionCode();
            BroadBee_ZRC_Start_NWK();
            printf("\nPlease input the pair ref# [0-9]\n");
            char key = getchar();
            BroadBee_ZRC_Show_My_Interest(key - 0x30);
            break;
    }

    while(1){
        printf("\nReady to receive any remote control command\n");
        printf("Press 'p' to see the Pairing Table Entry\n");
        printf("Press 'v' to send a vendor frame\n");
        printf("Press 't' to run the mailbox loopback test with an echo packet to stack\n");
        printf("Press 'e' to exit from this application\n\n");
        key = getchar();
        switch(key){
            case 'p':
            case 'P':
                printf("\nInput the pairing ref# [0-9] to see the detail\n");
                char pairingRef = getchar() - '0';
                printf("\n");
                BroadBee_RF4CE_Get_PairingTableEntry(pairingRef);
                break;
            case 'v':
            case 'V':
                printf("\nSending a vendor frame packet\n");
                BroadBee_ZRC_Send_Vendor_Frame("\x12\x34\x56\x78\x9a\xbc\xde\xf0", 8);
                break;
            case 't':
            case 'T':
                printf("\nLoopback test to send an echo packet with 20 of 32-bit data\n");
                BroadBee_SYS_loopback_test(20);
                break;
            case 'e':
            case 'E':
                goto _exit;
            default:
                break;
        }
    }

_exit:
    printf("Completed 'rf4ce_pair' application successfully.\n");

    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    Zigbee_Close();

    return 0;
}

/* eof rf4ce_pair.c */
