/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#ifndef _SATFE_FSK_H_
#define _SATFE_FSK_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SATFE_FSK_FRAME_NO_REPLY_FIRST_TX    0xE0
#define SATFE_FSK_FRAME_NO_REPLY_REPEAT_TX   0xE1
#define SATFE_FSK_FRAME_REPLY_REQ_FIRST_TX   0xE2
#define SATFE_FSK_FRAME_REPLY_REQ_REPEAT_TX  0xE3

#define SATFE_FSK_ADDR_ANY_DEVICE      0x00
#define SATFE_FSK_ADDR_LNB_OR_SWITCH   0x10
#define SATFE_FSK_ADDR_LNB_DEVICE      0x11
#define SATFE_FSK_ADDR_EXT_LNB_DEVICE  0x12
#define SATFE_FSK_ADDR_STB             0xFD

#define SATFE_FSK_CMD_RESET_CHANNEL       0x00
#define SATFE_FSK_CMD_CLEAR_RESET_FLAG    0x01
#define SATFE_FSK_CMD_STATUS              0x10
#define SATFE_FSK_CMD_READ_SHORT_CIRCUIT  0x13
#define SATFE_FSK_CMD_SWITCH_STATUS       0x14
#define SATFE_FSK_CMD_COMMITED_SWITCHES   0x38
#define SATFE_FSK_CMD_HARDWARE_ID         0x54
#define SATFE_FSK_CMD_GET_VERSION         0x56
#define SATFE_FSK_CMD_SET_RF_COMM_MODE    0x5A
#define SATFE_FSK_CMD_GET_CHANNEL_BW      0x70
#define SATFE_FSK_CMD_REQ_USER_BAND_GRID  0x71
#define SATFE_FSK_CMD_ALLOCATE            0x72
#define SATFE_FSK_CMD_SET_CHANNEL_BW      0x73
#define SATFE_FSK_CMD_DEALLOCATE          0x74
#define SATFE_FSK_CMD_REQ_CHANNEL_ID      0x77
#define SATFE_FSK_CMD_GET_DEF_SIGNAL_VER  0x78
#define SATFE_FSK_CMD_SET_DEF_SIGNAL_VER  0x7D
#define SATFE_FSK_CMD_ODU_RESET           0x90


BERR_Code SATFE_Fsk_Reset(SATFE_Chip *pChip);
void      SATFE_Ftm_LogWrite(SATFE_Chip *pChip, uint8_t *buf, uint8_t len);
void      SATFE_Ftm_LogRead(SATFE_Chip *pChip, uint8_t *buf, uint8_t len);
BERR_Code SATFE_Fsk_SendMessage(SATFE_Chip *pChip, uint8_t *pSendBuf, uint8_t sendLen, uint8_t *pRcvBuf, int timeoutMsec, uint8_t expReplyCmd);
BERR_Code SATFE_Fsk_GetMessage(SATFE_Chip *pChip, uint8_t *pBuf, int timeoutMsec);
BERR_Code SATFE_Fsk_Register(SATFE_Chip *pChip, uint8_t num_tuners);

bool SATFE_Command_fsk_reset(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_fsk_write(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_fsk_reprogram(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_fsk_read(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_fsk_allocate(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_fsk_deallocate(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_fsk_odu_reset(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_fsk_power_down(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_fsk_power_up(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_fsk_carrier(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_config_fsk(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_config_protocol(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_fsk_test_tdma(SATFE_Chip *pChip, int argc, char **argv);

#endif /* _SATFE_FSK_H_ */
