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
 *****************************************************************************/

/*******************************************************************************
 *
 * DESCRIPTION:
 *      PHY-PIB internals definitions.
 *
*******************************************************************************/

#ifndef _BB_PHY_PIB_DEFS_H
#define _BB_PHY_PIB_DEFS_H


/************************* INCLUDES *****************************************************/
#include "bbPhySapPib.h"            /* PHY-PIB for PHY-SAP definitions. */
#include "bbHalRadio.h"             /* Hardware Radio interface. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for storing PHY-PIB attributes.
 */
typedef struct _PhyPib_t
{
    /* 8-bit data. */
    PHY_PageChannel_t     phyCurrentChannelOnPage;      /*!< Values of phyCurrentPage and phyCurrentChannel. */

    /* Structured / 8-bit data. */
    PHY_TransmitPower_t   phyTransmitPower;             /*!< Value of phyTransmitPower. */

    enum PHY_CCAMode_t    phyCcaMode;                   /*!< Value of phyCcaMode. */

} PhyPib_t;


/*
 * PHY-PIB attributes default values.
 * NOTE: Transmitter Power Tolerance measured here in dBm. Divide its value by 3 in order
 *   to obtain the IEEE-Code-of-Tolerance.
 */
#define PHY_ATTR_DEFAULT_VALUE_CURRENT_CHANNEL           HAL_RADIO__CHANNEL_DEF
#define PHY_ATTR_DEFAULT_VALUE_TRANSMIT_POWER_VALUE      HAL_RADIO__TX_POWER_DEF
#define PHY_ATTR_DEFAULT_VALUE_TRANSMIT_POWER_TOLERANCE  HAL_RADIO__TX_POWER_TOL
#define PHY_ATTR_DEFAULT_VALUE_CCA_MODE                  HAL_RADIO__CCA_MODE_DEF
#define PHY_ATTR_DEFAULT_VALUE_CURRENT_PAGE              HAL_RADIO__PAGE_DEF


/*
 * PHY-PIB attributes values constraints.
 * NOTE: Additional constraints must be applied to the phyCurrentChannel and
 *   phyCurrentPage attributes.
 */
#define PHY_ATTR_MAXALLOWED_VALUE_CURRENT_CHANNEL       PHY_CHANNEL_MAX
#define PHY_ATTR_MINALLOWED_VALUE_TRANSMIT_POWER_VALUE  HAL_RADIO__TX_POWER_MIN
#define PHY_ATTR_MAXALLOWED_VALUE_TRANSMIT_POWER_VALUE  HAL_RADIO__TX_POWER_MAX
#define PHY_ATTR_MINALLOWED_VALUE_CCA_MODE              HAL_RADIO__CCA_MODE_MIN
#define PHY_ATTR_MAXALLOWED_VALUE_CCA_MODE              HAL_RADIO__CCA_MODE_MAX
#define PHY_ATTR_MAXALLOWED_VALUE_CURRENT_PAGE          PHY_PAGE_MAX


#endif /* _BB_PHY_PIB_DEFS_H */

/* eof bbPhyPibDefs.h */