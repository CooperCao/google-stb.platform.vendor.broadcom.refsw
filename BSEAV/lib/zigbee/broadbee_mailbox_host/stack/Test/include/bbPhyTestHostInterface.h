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

/******************************************************************************
*
* DESCRIPTION:
*       The phy test interface implementation.
*
*******************************************************************************/

#ifdef _PHY_TEST_HOST_INTERFACE_

#ifndef _BB_PHY_TEST_H
#define _BB_PHY_TEST_H

typedef enum _PHY_Test_Status{
    /* the phy test reture status common part */
    PHY_TEST_STATUS_SUCCESS = 0,
    PHY_TEST_STATUS_INVALID_STATE,      /* invalid state to handle new request */
    PHY_TEST_STATUS_INVALID_PARAMETER,  /* invalid parameter on the request */
    PHY_TEST_STATUS_FAILED,             /* the request failed */
    PHY_TEST_STATUS_BUSY,
    PHY_TEST_STATUS_FAILED_TRX_OFF,
    PHY_TEST_STATUS_FAILED_TX_ON,
    PHY_TEST_STATUS_FAILED_RX_ON,

    /* the phy test reture status for transmit */
    PHY_TEST_STATUS_TRANSMIT_START = 0x10,
    PHY_TEST_STATUS_TRANSMIT_PAYLOAD_LENGTH_INVALID = PHY_TEST_STATUS_TRANSMIT_START,

    /* the phy test reture status for receive */
    PHY_TEST_STATUS_RECEIVE_START = 0x20,
    PHY_TEST_STATUS_RECEIVE_PAYLOAD_LENGTH_INVALID = PHY_TEST_STATUS_RECEIVE_START,

}PHY_Test_Status;

typedef enum _PHY_Test_State{
    /* the phy idle state */
    PHY_TEST_STATE_IDLE = 0,
    PHY_TEST_STATE_CONT_WAVE,
    PHY_TEST_STATE_TRANSMIT,
    PHY_TEST_STATE_RECEIVE,
    PHY_TEST_STATE_ECHO,
    PHY_TEST_STATE_ENERTY_DETECT,
    PHY_TEST_STATE_AMOUNT
}PHY_Test_State;

typedef enum _PHY_Test_Request_IDs{
    PHY_TEST_REQUEST_ID_GET_CAPS = 0,
    PHY_TEST_REQUEST_ID_GET_STATS,
    PHY_TEST_REQUEST_ID_RESET_STATS,
    PHY_TEST_REQUEST_ID_SET_CHANNEL,
    PHY_TEST_REQUEST_ID_SET_POWER,
    PHY_TEST_REQUEST_ID_CONT_WAVE_START,
    PHY_TEST_REQUEST_ID_CONT_WAVE_STOP,
    PHY_TEST_REQUEST_ID_TRANS_START,
    PHY_TEST_REQUEST_ID_TRANS_STOP,
    PHY_TEST_REQUEST_ID_RECV_START,
    PHY_TEST_REQUEST_ID_RECV_STOP,
    PHY_TEST_REQUEST_ID_ECHO_START,
    PHY_TEST_REQUEST_ID_ECHO_STOP,
    PHY_TEST_REQUEST_ID_ENERGY_DETECT,
    PHY_TEST_REQUEST_ID_AMOUNT
}PHY_Test_Request_IDs;

typedef enum _PHY_Test_Request_Allowed{
    PHY_TEST_REQUEST_ALLOWED,
    PHY_TEST_REQUEST_DISALLOWED,
}PHY_Test_Request_Allowed;

#define ALLOWED PHY_TEST_REQUEST_ALLOWED
#define DISALLOW PHY_TEST_REQUEST_DISALLOWED
/**************************** Transmit related definition ***********************************/
typedef enum _PHY_Transmit_Status{
    PHY_TRANSMIT_STATUS_IDLE = 0,
    PHY_TRANSMIT_STATUS_START,
    PHY_TRANSMIT_STATUS_STOP,
}PHY_Transmit_Status;

typedef struct _PHY_Test_Transmit_Control_Block
{
    uint32_t packetCount;
    uint32_t packetSent;
    uint16_t intervalMs;
    PHY_Transmit_Status  currentStatus;
    PHY_Transmit_Status  expectStatus;
}PHY_Test_Transmit_Control_Block;
/**************************** Transmit related definition end *******************************/

/**************************** Receive related definition ***********************************/
typedef enum _PHY_Receive_Status{
    PHY_RECEIVE_STATUS_IDLE = 0,
    PHY_RECEIVE_STATUS_START,
    PHY_RECEIVE_STATUS_STOP,
}PHY_Receive_Status;

typedef enum _PHY_Test_Receive_Filter_Mode{
    RF4CE_CTRL_TEST_RECEIVE_FILTER_NONE                 = 0,
    RF4CE_CTRL_TEST_RECEIVE_FILTER_SHORT_ADDRESS_FROM   = 0x1,
    RF4CE_CTRL_TEST_RECEIVE_FILTER_SHORT_ADDRESS_TO     = 0x1<<1,
    RF4CE_CTRL_TEST_RECEIVE_FILTER_LONG_ADDRESS_FROM    = 0x1<<2,
    RF4CE_CTRL_TEST_RECEIVE_FILTER_LONG_ADDRESS_TO      = 0x1<<3,
}PHY_Test_Receive_Filter_Mode;

typedef struct _PHY_Test_Receive_Control_Block
{
    uint32_t filter;
    uint16_t shortAddressFrom;
    uint16_t shortAddressTo;
    MAC_Addr64bit_t longAddressFrom;
    MAC_Addr64bit_t longAddressTo;
    PHY_Receive_Status expectStatus;
}PHY_Test_Receive_Control_Block;

typedef struct _RF4CE_HostPairingTableEntry_t
{
    RF4CE_PairingTableEntry_t entry;
    uint8_t linkQuality;
} RF4CE_HostPairingTableEntry_t;

void RF4CE_MAC_DataInd_Stub(MAC_DataIndParams_t *indication);

/**************************** Receive related definition end *******************************/

#endif  /* _PHY_TEST_HOST_INTERFACE_  */

#endif
