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
 ******************************************************************************

#define RF4CE_ZRC_DEFAULT_HDMI_BANK \
    {0x1f, 0x06, 0, 0xe0, 0xff, 0x03, 0x13, 0, \
    0x0f, 0, 0, 0, 0, 0, 0x1e, 0, \
    0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, 0, 0, 0, 0, 0}

/**//**
 * \brief RF4CE ZRC on board attributes initialization.
 */
#define INIT_ZRC1_ATTRIBUTES \
    .aplZRC1KeyRepeatInterval = RF4CE_ZRC1_APLC_MAX_KEY_REPEAT_INTERVAL >> 1, \
    .aplZRC1KeyRepeatWaitTime = RF4CE_ZRC1_APLC_MAX_KEY_REPEAT_INTERVAL << 1, \
    .aplZRC1KeyExchangeTransferCount = RF4CE_ZRC1_APLC_MIN_KEY_EXCHANGE_TRANSFER_COUNT, \
    .aplZRC1CommandDiscovery = RF4CE_ZRC_DEFAULT_HDMI_BANK

/**//**
 * \brief RF4CE ZRC Attributes.
 */
typedef struct _RF4CE_ZRC_Attributes_t
{
    uint32_t aplZRC1KeyRepeatInterval;            /*!< ZRC 1. The interval in ms at which user command repeat
                                                       frames will be transmitted. */
    uint32_t aplZRC1KeyRepeatWaitTime;            /*!< ZRC 1. The duration that a recipient of a user control
                                                       repeated command frame waits before terminating a
                                                       repeated operation. */
    uint8_t aplZRC1KeyExchangeTransferCount;      /*!< ZRC 1. The value of the KeyExTransferCount parameter
                                                       passed to the pair request primitive during the push
                                                       button pairing procedure. */
    uint8_t aplZRC1CommandDiscovery[32];          /*!< ZRC 1. Command discovery bitmap. */
} RF4CE_ZRC_Attributes_t;
