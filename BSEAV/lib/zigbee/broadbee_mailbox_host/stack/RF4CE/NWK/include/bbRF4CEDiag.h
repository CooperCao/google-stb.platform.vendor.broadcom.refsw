/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
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
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/RF4CE/NWK/include/bbRF4CEDiag.h $
*
* DESCRIPTION:
*   RF4CE layer diagnostics definition.
*
* $Revision: $
* $Date: $
*
*****************************************************************************************/


#ifndef _BB_RF4CE_DIAG_H
#define _BB_RF4CE_DIAG_H


/************************* INCLUDES *****************************************************/
typedef enum _RF4CE_Ctrl_Constant_Diagnostics_t{
    RF4CE_CTRL_CONSTANT_DIAGNOSTICS_AGILITY = 0x01,
    RF4CE_CTRL_CONSTANT_DIAGNOSTICS_LINK_QUALITY = 0x02,
    RF4CE_CTRL_CONSTANT_DIAGNOSTICS_TX_POWER = 0x04,
    RF4CE_CTRL_CONSTANT_DIAGNOSTICS_TX_POWER_KEY_EXCHANGE = 0x08,
}RF4CE_Ctrl_Constant_Diagnostics_t;

typedef struct _RF4CE_Diag_Caps_ConfParams_t{
    uint32_t constant;
    uint32_t managed;
}RF4CE_Diag_Caps_ConfParams_t;

typedef struct _RF4CE_Diag_Caps_ReqDescr_t RF4CE_Diag_Caps_ReqDescr_t;

typedef struct _RF4CE_Diag_Caps_ReqDescr_t{
    void (*callback)(RF4CE_Diag_Caps_ReqDescr_t *, RF4CE_Diag_Caps_ConfParams_t *);
}RF4CE_Diag_Caps_ReqDescr_t;



typedef struct _RF4CE_Diag_ReqParams_t{
    uint32_t constant;
    uint32_t managed;
}RF4CE_Diag_ReqParams_t;

typedef struct
{
    uint8_t logicalChannel;
    uint16_t transmitFailures;
} Diag_RF4CECtrlChannel;

typedef struct
{
    uint8_t agilityThreshold;
    Diag_RF4CECtrlChannel operational;
    Diag_RF4CECtrlChannel bkChan1;
    Diag_RF4CECtrlChannel bkChan2;
} Diag_RF4CECtrlAgility;

typedef struct
{
    MAC_Addr64bit_t address;
    uint8_t pairingRef;
    uint8_t linkQuality;
} Diag_RF4CECtrlLinkQuality;

typedef struct
{
    int8_t power;
    int8_t powerMax;
    int8_t powerMin;
} Diag_RF4CECtrlTxPower;


typedef struct _RF4CE_Diag_ConfParams_t
{
    union
    {
        Diag_RF4CECtrlAgility agility;
        Diag_RF4CECtrlLinkQuality linkQuality;
        Diag_RF4CECtrlTxPower txPower;
        Diag_RF4CECtrlTxPower txPowerKeyExchange;
        uint8_t reserved[64];
    } u;
} RF4CE_Diag_ConfParams_t;

typedef struct _RF4CE_Diag_ReqDescr_t RF4CE_Diag_ReqDescr_t;

typedef struct _RF4CE_Diag_ReqDescr_t{
    RF4CE_Diag_ReqParams_t   params;
    void (*callback)(RF4CE_Diag_ReqDescr_t *, RF4CE_Diag_ConfParams_t *);
}RF4CE_Diag_ReqDescr_t;

void RF4CE_Get_Diag_Caps_Req(RF4CE_Diag_Caps_ReqDescr_t *req);
void RF4CE_Get_Diag_Req(RF4CE_Diag_ReqDescr_t *req);

#endif
