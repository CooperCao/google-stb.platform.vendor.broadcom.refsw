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
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkServices.h $
*
* DESCRIPTION:
*   Contains network service type definitions.
*
* $Revision: 2595 $
* $Date: 2014-06-03 15:11:16Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_SERVICES_H
#define _ZBPRO_NWK_SERVICES_H

/************************* INCLUDES *****************************************************/
#include "bbSysPayload.h"
#include "private/bbZbProNwkCommonPrivate.h"
#include "private/bbZbProNwkBufferManager.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief The default confirmation of a transmission command type.
 */
typedef struct _ZbProNwkServiceDefaultConfParams_t
{
    ZBPRO_NWK_Status_t status;
} ZbProNwkServiceDefaultConfParams_t;

/**//**
 * \brief Reset handler type.
 */
typedef void (ZbProNwkResetServiceHandler_t)(void);

/**//**
 * \brief Payload length handler type.
 */
typedef uint8_t (ZbProNwkGetPayloadSizeServiceHandler_t)(void);

/**//**
 * \brief Fill buffer handler type.
 */
typedef bool (ZbProNwkFillPacketServiceHandler_t)(ZbProNwkOutputBuffer_t *const outputBuffer);

/**//**
 * \brief Tx confirmation handler type.
 */
typedef void (ZbProNwkConfServiceHandler_t)(ZbProNwkOutputBuffer_t *const outputBuffer, const ZBPRO_NWK_Status_t status);

/**//**
 * \brief Indication handler type.
 */
typedef bool (ZbProNwkIndServiceHandler_t)(ZbProNwkInputBuffer_t *const inputBuffer);

/**//**
 * \brief Service descriptor type definition.
 */
typedef struct _ZbProNwkServiceDescriptor_t
{
    ZbProNwkGetPayloadSizeServiceHandler_t  *payloadSize;   /*!< Returns a number of bytes to be allocated
                                                         by the dispatcher. Can be equal to NULL. */
    ZbProNwkFillPacketServiceHandler_t  *fill;      /*!< Fills the allocated output buffer structure.
                                                         Shall be specified for all services.  */
    ZbProNwkConfServiceHandler_t        *conf;      /*!< Receives a confirmation at the end of
                                                         transmission. Can be equal to NULL. */
    ZbProNwkIndServiceHandler_t         *ind;       /*!< Accepts an indication received from
                                                         MAC layer. Shall be specified for all services. */
    ZbProNwkResetServiceHandler_t       *reset;     /*!< Resets the corresponding service. */
} ZbProNwkServiceDescriptor_t;

#endif /* _ZBPRO_NWK_SERVICES_H */