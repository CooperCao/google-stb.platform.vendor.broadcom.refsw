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


static void help_screen(void)
{
    printf("\nSupported commands:\n");
    printf("==================================================================\n");
    printf("11: BroadBee_PHY_Set_Channel(channel)\n");
    printf("12: BroadBee_PHY_Continuous_Wave_Start(modulationMode)\n");
    printf("13: BroadBee_PHY_Continuous_Wave_Stop()\n");
    printf("14: BroadBee_PHY_Transmit_Start()\n");
    printf("15: BroadBee_PHY_Transmit_Stop()\n");
    printf("16: BroadBee_PHY_Receive_Start()\n");
    printf("17: BroadBee_PHY_Receive_Stop()\n");
    printf("18: BroadBee_PHY_Echo_Start()\n");
    printf("19: BroadBee_PHY_Echo_Stop()\n");
    printf("20: BroadBee_PHY_Energy_Detect_Scan()\n");
    printf("21: BroadBee_PHY_Get_Stats()\n");
    printf("22: BroadBee_PHY_Reset_Stats()\n");
    printf("23: BroadBee_PHY_Get_TxPower()\n");
    printf("24: BroadBee_PHY_Set_TxPower(power)\n");
    printf("25: BroadBee_PHY_Select_Antenna(selAnt)\n");
    printf("26: BroadBee_PHY_Get_Caps()\n");

    printf("51: BroadBee_RF4CE_Get_Supported_Profiles()\n");
    printf("52: BroadBee_RF4CE_Get_NWK_Information()\n");
    printf("53: BroadBee_RF4CE_Get_pairingTableEntriesMax()\n");

    printf("71: BroadBee_ZRC_Get_TxPower_KeyExchange()\n");
    printf("72: BroadBee_ZRC_Set_TxPower_KeyExchange(power)\n");
    printf("73: BroadBee_ZRC_Get_Num_Paired_Devices()\n");
    printf("74: BroadBee_ZRC_Get_Paired_Devices()\n");
    printf("75: BroadBee_ZRC_Get_Caps_Ex()\n");
    printf("76: BroadBee_ZRC_Get_Diag_Caps()\n");
    printf("77: BroadBee_ZRC_Get_Diag_Agility()\n");
    printf("78: BroadBee_ZRC_Get_Diag_LinkQuality(pairRef)\n");
    printf("79: BroadBee_ZRC_Get_Extended_Cap()\n");
    printf("80: BroadBee_ZRC_Subscribe_Event()\n");

    printf("99: To run all commands one after another with default parameters\n");
    printf("Any other will quit this application\n");
    printf("==================================================================\n");
}


int main(int argc, char *argv[])
{
    struct zigbeeCallback zcb;

#ifdef BYPASS_RPC
    extern int zigbee_init(int argc, char *argv[]);
    zigbee_init(argc, argv);
#endif

    /* Register the callback functions you are interested in.  Ones that are not filled out, won't be called back. */
    /* Calling Zigbee_GetDefaultSettings will initialize the callback structure */
    Zigbee_GetDefaultSettings(&zcb);
    zcb.RF4CE_PairInd = BroadBee_ZRC_PairInd;
    zcb.RF4CE_ZRC2_CheckValidationInd = BroadBee_ZRC2_CheckValidationInd;
    zcb.RF4CE_ZRC2_ControlCommandInd = BroadBee_ZRC2_ControlCommandInd;
    zcb.RF4CE_ZRC1_ControlCommandInd = BroadBee_ZRC1_ControlCommandInd;
    zcb.RF4CE_ZRC1_VendorSpecificInd = BroadBee_ZRC1_VendorSpecificInd;
    zcb.SYS_EventNtfy = BroadBee_ZRC_EventNtfy;

    Zigbee_Open(&zcb, "127.0.0.1");

    BroadBee_ZRC_Set_SupportedDevices();
    BroadBee_ZRC_Start_NWK();

    // In case when one of the previous test is still active
    BroadBee_PHY_Echo_Stop();
    BroadBee_PHY_Continuous_Wave_Stop();
    BroadBee_PHY_Transmit_Stop();
    BroadBee_PHY_Receive_Stop();
    BroadBee_PHY_Reset_Stats();

    while(1)
    {
        char string [10];
        bool quitCommand = 0;

        help_screen();
        printf("Enter the command number ... ");
        fgets(string, 10, stdin);
        int command_number = atoi(string);

        switch (command_number) {

            default:
                printf("\nUnsupported command number\n");
                quitCommand = 1;
                break;

            case 99:
                // Execute all commands below

            case 11:
                {
                    uint8_t channel = 20;   // default channel number
                    if (command_number != 99)
                    {
                        printf("Enter the channel number (11 ~ 26) .. ");
                        fgets(string, 10, stdin);
                        channel = atoi(string);
                    }
                    if ((channel < 11) || (channel > 26))
                    {
                        printf("Unsupported channel number = %d.  Should be 11 ~ 26.\n", channel);
                        break;
                    }
                    printf("\nBroadBee_PHY_Set_Channel(%d)\n", channel);
                    BroadBee_PHY_Set_Channel(channel);
                }
                if (command_number != 99) break;

            case 12:
                printf("\nBroadBee_PHY_Continuous_Wave_Start(modulationMode)\n");
                RF4CE_CTRL_TEST_CONTINUOUS_WAVE_MODE modulationMode = RF4CE_CTRL_TEST_CONTINUOUS_WAVE_MODE_UNMODULATED;
                if (command_number != 99)
                {
                    do
                    {
                        printf("Enter '1' for UNMODULATED or '2' for MODULATED continuous waveform .. ");
                        fgets(string, 10, stdin);
                        modulationMode = atoi(string);
                    } while ((modulationMode != RF4CE_CTRL_TEST_CONTINUOUS_WAVE_MODE_UNMODULATED) &&
                             (modulationMode != RF4CE_CTRL_TEST_CONTINUOUS_WAVE_MODE_MODULATED));
                    BroadBee_PHY_Continuous_Wave_Start(modulationMode);
                    break;
                }
                else
                {
                    BroadBee_PHY_Continuous_Wave_Start(modulationMode);
                    printf("... Press any key to continue ...\n");
                    getchar();
                }

            case 13:
                printf("\nBroadBee_PHY_Continuous_Wave_Stop()\n");
                BroadBee_PHY_Continuous_Wave_Stop();
                if (command_number != 99) break;

            case 14:
                printf("\nBroadBee_PHY_Transmit_Start()\n");
                BroadBee_PHY_Transmit_Start();
                if (command_number != 99)
                    break;
                else {
                    printf("... Press any key to continue ...\n");
                    getchar();
                }

            case 15:
                printf("\nBroadBee_PHY_Transmit_Stop()\n");
                BroadBee_PHY_Transmit_Stop();
                if (command_number != 99) break;

            case 16:
                printf("\nBroadBee_PHY_Receive_Start()\n");
                BroadBee_PHY_Receive_Start();
                if (command_number != 99)
                    break;
                else {
                    printf("... Press any key to continue ...\n");
                    getchar();
                }

            case 17:
                printf("\nBroadBee_PHY_Receive_Stop()\n");
                BroadBee_PHY_Receive_Stop();
                if (command_number != 99) break;

            case 18:
                printf("\nBroadBee_PHY_Echo_Start()\n");
                BroadBee_PHY_Echo_Start();
                if (command_number != 99)
                    break;
                else {
                    printf("... Press any key to continue ...\n");
                    getchar();
                }

            case 19:
                printf("\nBroadBee_PHY_Echo_Stop()\n");
                BroadBee_PHY_Echo_Stop();
                if (command_number != 99) break;

            case 20:
                printf("\nBroadBee_PHY_Energy_Detect_Scan()\n");
                BroadBee_PHY_Energy_Detect_Scan();
                if (command_number != 99)
                    break;
                else {
                    printf("... Press any key to continue ...\n");
                    getchar();
                }

            case 21:
                printf("\nBroadBee_PHY_Get_Stats()\n");
                BroadBee_PHY_Get_Stats();
                if (command_number != 99) break;

            case 22:
                printf("\nBroadBee_PHY_Reset_Stats()\n");
                BroadBee_PHY_Reset_Stats();
                if (command_number != 99) break;

            case 23:
                printf("\nBroadBee_PHY_Get_TxPower()\n");
                BroadBee_PHY_Get_TxPower();
                if (command_number != 99) break;

            case 24:
                {
                    int txPower = 0;    // default txPower
                    if (command_number != 99)
                    {
                        printf("Enter the TX Power in dBm (-30 ~ 3) .. ");
                        fgets(string, 10, stdin);
                        txPower = atoi(string);
                    }

                    printf("\nBroadBee_PHY_Set_TxPower(%d)\n", txPower);
                    BroadBee_PHY_Set_TxPower(txPower);
                }
                if (command_number != 99) break;

            case 25:
                {
                    RF4CE_CTRL_ANTENNA SelAnt = RF4CE_ANTENNA_TX_ANT1_RX_ANT1;  // default anntena
                    if (command_number != 99)
                    {
                        printf("Enter the antenna selection option (0 ~ 4) .. ");
                        fgets(string, 10, stdin);
                        SelAnt = atoi(string);
                    }
                    printf("\nBroadBee_PHY_Select_Antenna(%d)\n", SelAnt);
                    BroadBee_PHY_Select_Antenna(SelAnt);
                }
                if (command_number != 99) break;

            case 26:
                printf("\nBroadBee_PHY_Get_Caps()\n");
                BroadBee_PHY_Get_Caps();
                if (command_number != 99) break;


            case 51:
                printf("\nBroadBee_RF4CE_Get_Supported_Profiles()\n");
                BroadBee_RF4CE_Get_Supported_Profiles();
                if (command_number != 99) break;

            case 52:
                printf("\nBroadBee_RF4CE_Get_NWK_Information()\n");
                BroadBee_RF4CE_Get_NWK_Information();
                if (command_number != 99) break;

            case 53:
                printf("\nBroadBee_RF4CE_Get_pairingTableEntriesMax()\n");
                printf("Max Pairing Table Entries : %d\n", BroadBee_RF4CE_Get_pairingTableEntriesMax());
                if (command_number != 99) break;


            case 71:
                printf("\nBroadBee_ZRC_Get_TxPower_KeyExchange()\n");
                BroadBee_ZRC_Get_TxPower_KeyExchange();
                if (command_number != 99) break;

            case 72:
                {
                    int8_t power = -10; // default power
                    if (command_number != 99)
                    {
                        printf("Enter the TX Power for Key Exchange in dBm (-30 ~ 3) .. ");
                        fgets(string, 10, stdin);
                        power = atoi(string);
                    }
                    printf("\nBroadBee_ZRC_Set_TxPower_KeyExchange(%d)\n", power);
                    BroadBee_ZRC_Set_TxPower_KeyExchange(power);
                }
                if (command_number != 99) break;

            case 73:
                printf("\nBroadBee_ZRC_Get_Num_Paired_Devices()\n");
                printf("%d device(s) has been paired\n", BroadBee_ZRC_Get_Num_Paired_Devices());
                if (command_number != 99) break;

            case 74:
                printf("\nBroadBee_ZRC_Get_Paired_Devices()\n");
                BroadBee_ZRC_Get_Paired_Devices();
                if (command_number != 99) break;

            case 75:
                printf("\nBroadBee_ZRC_Get_Caps_Ex()\n");
                BroadBee_ZRC_Get_Caps_Ex();
                if (command_number != 99) break;

            case 76:
                printf("\nBroadBee_ZRC_Get_Diag_Caps()\n");
                BroadBee_ZRC_Get_Diag_Caps();
                if (command_number != 99) break;

            case 77:
                printf("\nBroadBee_ZRC_Get_Diag_Agility()\n");
                BroadBee_ZRC_Get_Diag_Agility();
                if (command_number != 99) break;

            case 78:
                {
                    uint8_t pairRef = 0;    // default pairRef
                    if (command_number != 99)
                    {
                        printf("Enter the pair reference number (0 ~ 5) .. ");
                        fgets(string, 10, stdin);
                        pairRef = atoi(string);
                    }
                    printf("\nBroadBee_ZRC_Get_Diag_LinkQuality(%d)\n", pairRef);
                    BroadBee_ZRC_Get_Diag_LinkQuality(pairRef);
                }
                if (command_number != 99) break;

            case 79:
                {
                    RF4CEZRCInputCapsEx cap;
                    cap.version = RF4CE_ZRC_INPUT_CAP_V0;
                    cap.capabilitySize = sizeof(RF4CEZRCInputCapsV0);
                    printf("\nBroadBee_ZRC_Get_Extended_Cap(0x%X)\n", &cap);
                    BroadBee_ZRC_Get_Extended_Cap(&cap);
                }
                if (command_number != 99) break;

            case 80:
                printf("\nBroadBee_ZRC_Subscribe_Event()\n");
                BroadBee_ZRC_Subscribe_Event();
                if (command_number != 99) break;
        }

        if (quitCommand == 1)
            break;
    }

    printf("\nCompleted 'rf4ce_phy_test' application successfully.\n");
    Zigbee_Close();

    return 0;
}

/* eof rf4ce_phy_test.c */
