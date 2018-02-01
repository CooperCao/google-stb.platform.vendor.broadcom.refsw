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

/******************************************************************************
*
* DESCRIPTION:
*       Contains ZigBee PRO APS layer constants.
*
*******************************************************************************/

#ifndef _ZBPRO_APS_CONFIG_H
#define _ZBPRO_APS_CONFIG_H

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Binding Table size, in 8-byte chunks.
 * \details This constant specifies the maximum number of rows in the Binding Table of the
 *  type link-to-group. If there are some rows of the type link-to-device, the total
 *  number of all rows of both types must not exceed this constant.
 * \note The size of a row of the type link-to-group is 8 bytes, while the size of a row
 *  of the type link-to-device is 12 bytes, so the maximum number of rows of the type
 *  link-to-device equals to this constant multiplied by 2/3.
 * \note The maximum allowed value for this constant is 255.
 */
#define ZBPRO_APS_BINDING_TABLE_SIZE    24          /* Gives 16 records of the type link-to-device. */

/* Number of elements in the APS Group Table. */
#define ZBPRO_APS_GROUP_TABLE_SIZE      4U

/*
 * Number of ZbProApsBuffer_t buffers in the Buffer Pool
 */
#ifndef ZBPRO_APS_BUFFER_POOL_SIZE
#define ZBPRO_APS_BUFFER_POOL_SIZE      8
#endif

#define ZBPRO_APS_MAX_FRAME_RETRIES     3

/*
 * Row number of the Key-Pair table.
 */
#define ZBPRO_APS_KEYPAIR_ROWS          4

/*
 * Number of endpoints on this node. It also defines number of serving data structures.
 */
#define ZBPRO_APS_NODE_ENDPOINTS        3

/*
 * A default global trust center link key must be supported by the device if no other
 * link key is specified by the application at the time of joining. This default link key
 * shall have a value of 5A 69 67 42 65 65 41 6C 6C 69 61 6E 63 65 30 39(ZigBeeAlliance09).
 * See ZigBee Spec r20, 4.6.3.2.2.1 Standard Security Mode
 */
#define ZBPRO_APS_DEFAULT_TC_GLOBAL_KEY "ZigBeeAlliance09"
#endif /* _ZBPRO_APS_CONFIG_H */

/* eof bbZbProApsConfig.h */