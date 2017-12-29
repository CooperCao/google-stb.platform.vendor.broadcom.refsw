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
#ifndef __RF4CE_COMMON_H__
#define __RF4CE_COMMON_H__

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#pragma GCC optimize "short-enums"
#pragma pack(4)

#include "bbMailAPI.h"
#include "zigbee_api.h"
#include "zigbee.h"
#include "bbSysPayload.h"
#include "zigbee_rf4ce_registration.h"
#include "zigbee_dbg.h"

void BroadBee_SYS_Get_Fw_Rev();
void BroadBee_SYS_loopback_test(int words_to_test);
void BroadBee_ZRC_Start_NWK();
void BroadBee_ZRC1_TargetBinding();
void BroadBee_ZRC1_TargetBinding_BindingTime(uint32_t bindingTimeSeconds);
void BroadBee_ZRC2_EnableBinding();
void BroadBee_ZRC_PairInd(RF4CE_PairingIndParams_t *indication);
void BroadBee_ZRC1_ControlCommandInd(RF4CE_ZRC1_ControlCommandIndParams_t *commandInd);
void BroadBee_ZRC2_ControlCommandInd(RF4CE_ZRC2_ControlCommandIndParams_t *commandInd);
void BroadBee_ZRC2_CheckValidationInd(RF4CE_ZRC2_CheckValidationIndParams_t *indication);
void BroadBee_ZRC_Show_My_Interest(uint8_t pairRef);
void BroadBee_ZRC_Set_SupportedDevices();
void BroadBee_ZRC_Get_WakeUpActionCode();
void BroadBee_ZRC_Set_WakeUpActionCode();
void BroadBee_ZRC_Send_Vendor_Frame(uint8_t *vendorFrame, int length);
void BroadBee_ZRC1_VendorSpecificInd(RF4CE_ZRC1_VendorSpecificIndParams_t *indParams);
void BroadBee_ZRC_Restore_Factory_Settings(uint8_t restore);
void BroadBee_ZRC_Set_Discovery_Duration(uint32_t durationSeconds);
int  BroadBee_ZRC_Get_Num_Paired_Devices();
void BroadBee_ZRC_Get_Paired_Devices();
void BroadBee_ZRC1_Unpair(uint8_t pairRef);
void BroadBee_ZRC1_Set_KeyRepeatWaitTime(uint32_t waitTime);
void BroadBee_ZRC1_Config_KeyRepeat(uint8_t keyRepeatEnable);
void BroadBee_ZRC_EventNtfy(SYS_EventNotifyParams_t *event);
void BroadBee_ZRC_Subscribe_Event();
void BroadBee_ZRC2_Set_Default_Check_Validation_Period(uint8_t pairingRef);
void BroadBee_ZRC2_Set_Default_Link_Lost_Wait_Time(uint8_t pairingRef);
void BroadBee_ZRC2_Set_PushButtonStimulus();

uint16_t BroadBee_RF4CE_Get_PanId();
void BroadBee_RF4CE_Get_NWK_Information();
bool BroadBee_RF4CE_Get_PairingTableEntry(char pairingRef);
uint8_t BroadBee_RF4CE_Get_pairingTableEntriesMax();
void BroadBee_RF4CE_Set_Base_Channel(uint8_t channel);
void BroadBee_RF4CE_Get_Supported_Profiles();
uint8_t BroadBee_RF4CE_Get_numSupportedProfiles();
uint8_t BroadBee_RF4CE_Get_antennasAvailable();
void BroadBee_RF4CE_Set_Discovery_Lqi_Threshold(uint8_t threshold);
int BroadBee_RF4CE_Get_FA_Attributes(uint8_t attrId);
void BroadBee_RF4CE_Set_FA_Attributes(uint8_t attrId, int8_t value);
void BroadBee_RF4CE_Get_FA_Threshold();
void BroadBee_RF4CE_Set_FA_Threshold(int8_t threshold);

uint16_t BroadBee_MAC_Get_ShortAddress();
MAC_ExtendedAddress_t BroadBee_MAC_Get_IEEEAddress();

/*****************************************************************************************/
void BroadBee_SYS_Get_Fw_Rev();
void BroadBee_SYS_loopback_test(int words_to_test);
void BroadBee_ZRC_Start_NWK();
void BroadBee_ZRC1_TargetBinding();
void BroadBee_ZRC1_TargetBinding_BindingTime(uint32_t bindingTimeSeconds);
#ifdef USE_RF4CE_PROFILE_ZRC2
void BroadBee_ZRC2_EnableBinding();
#endif
void BroadBee_ZRC_PairInd(RF4CE_PairingIndParams_t *indication);
void BroadBee_ZRC1_ControlCommandInd(RF4CE_ZRC1_ControlCommandIndParams_t *commandInd);
void BroadBee_ZRC2_ControlCommandInd(RF4CE_ZRC2_ControlCommandIndParams_t *commandInd);
#ifdef USE_RF4CE_PROFILE_ZRC2
void BroadBee_ZRC2_CheckValidationInd(RF4CE_ZRC2_CheckValidationIndParams_t *indication);
#endif
void BroadBee_ZRC_Show_My_Interest(uint8_t pairRef);
void BroadBee_ZRC_Set_SupportedDevices();
void BroadBee_ZRC_Get_WakeUpActionCode();
void BroadBee_ZRC_Set_WakeUpActionCode();
void BroadBee_ZRC_Send_Vendor_Frame(uint8_t *vendorFrame, int length);
void BroadBee_ZRC1_VendorSpecificInd(RF4CE_ZRC1_VendorSpecificIndParams_t *indParams);
void BroadBee_ZRC_Restore_Factory_Settings(uint8_t restore);
void BroadBee_ZRC_Set_Discovery_Duration(uint32_t durationSeconds);
int BroadBee_ZRC_Get_Num_Paired_Devices();
void BroadBee_ZRC_Get_Paired_Devices();
void BroadBee_ZRC1_Unpair(uint8_t pairRef);
void BroadBee_ZRC1_Set_KeyRepeatWaitTime(uint32_t waitTime);
void BroadBee_ZRC1_Config_KeyRepeat(uint8_t keyRepeatEnable);
void BroadBee_ZRC_EventNtfy(SYS_EventNotifyParams_t *event);
void BroadBee_ZRC_Subscribe_Event();
#ifdef USE_RF4CE_PROFILE_ZRC2
void BroadBee_ZRC2_Set_Default_Check_Validation_Period(uint8_t pairingRef);
void BroadBee_ZRC2_Set_Default_Link_Lost_Wait_Time(uint8_t pairingRef);
#endif
void BroadBee_ZRC2_Set_PushButtonStimulus();
uint16_t BroadBee_RF4CE_Get_PanId();
void BroadBee_RF4CE_Get_NWK_Information();
bool BroadBee_RF4CE_Get_PairingTableEntry(char pairingRef);
uint8_t BroadBee_RF4CE_Get_pairingTableEntriesMax();
void BroadBee_RF4CE_Set_Base_Channel(uint8_t channel);
void BroadBee_RF4CE_Get_Supported_Profiles();
uint8_t BroadBee_RF4CE_Get_numSupportedProfiles();
uint8_t BroadBee_RF4CE_Get_antennasAvailable();
void BroadBee_RF4CE_Set_Discovery_Lqi_Threshold(uint8_t threshold);
int BroadBee_RF4CE_Get_FA_Attributes(uint8_t attrId);
void BroadBee_RF4CE_Set_FA_Attributes(uint8_t attrId, int8_t value);
void BroadBee_RF4CE_Get_FA_Threshold();
void BroadBee_RF4CE_Set_FA_Threshold(int8_t threshold);
uint16_t BroadBee_MAC_Get_ShortAddress();
MAC_ExtendedAddress_t BroadBee_MAC_Get_IEEEAddress();

/*****************************************************************************************/

#define RF4CE_CTRL_FEATURE_DIAGNOSTICS  (0x00000001)
#define RF4CE_CTRL_FEATURE_PAIRING_REF  (0x00000100)
#define RF4CE_CTRL_FEATURE_TX_POWER     (0x00000200)

typedef struct
{
    uint32_t features;
    uint32_t numSupportedProfiles;
    uint32_t pairingTableEntriesMax;
    uint32_t antennasAvailable;
} RF4CECtrlCapsV1;

void BroadBee_ZRC_Get_Caps_Ex();


#define RF4CE_ZRC_INPUT_CAP_V0 (0x00000001)
#define USER_INPUT_MODE_RAW (1)
#define USER_INPUT_MODE_COOKED (2)
#define RF4CE_PROFILE_ZRC (1)

typedef struct
{
    uint32_t version;
    void     *capability;
    uint32_t capabilitySize;
} RF4CEZRCInputCapsEx;

typedef struct
{
    uint32_t mode;
    uint16_t profileId;
}RF4CEZRCInputCapsV0;

uint8_t BroadBee_ZRC_Get_Extended_Cap(RF4CEZRCInputCapsEx *cap);
void BroadBee_ZRC_Get_Diag_Caps();
void BroadBee_ZRC_Get_Diag_Agility();
void BroadBee_ZRC_Get_Diag_LinkQuality(uint8_t pairRef);
void BroadBee_ZRC_Get_TxPower_KeyExchange();
void BroadBee_ZRC_Set_TxPower_KeyExchange(int8_t power);
void BroadBee_PHY_Get_Caps();
void BroadBee_PHY_Set_Channel(uint8_t channel);
void BroadBee_PHY_Continuous_Wave_Start(RF4CE_CTRL_TEST_CONTINUOUS_WAVE_MODE modulationMode);
void BroadBee_PHY_Continuous_Wave_Stop();
void BroadBee_PHY_Transmit_Start();
void BroadBee_PHY_Transmit_Stop();
void BroadBee_PHY_Receive_Start();
void BroadBee_PHY_Receive_Stop();
void BroadBee_PHY_Echo_Start();
void BroadBee_PHY_Echo_Stop();
void BroadBee_PHY_Energy_Detect_Scan();
void BroadBee_PHY_Get_Stats();
void BroadBee_PHY_Reset_Stats();
void BroadBee_PHY_Get_TxPower();
void BroadBee_PHY_Set_TxPower(int power);
void BroadBee_PHY_Select_Antenna(RF4CE_CTRL_ANTENNA selAnt);

#pragma GCC optimize "no-short-enums"
#pragma pack()

#endif


/* eof rf4ce_common.h */
