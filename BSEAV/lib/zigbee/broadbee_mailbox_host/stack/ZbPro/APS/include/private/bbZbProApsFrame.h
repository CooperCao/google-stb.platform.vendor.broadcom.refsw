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
 *      Contains types declarations for the ZigBee PRO APS frame structures.
 *
*******************************************************************************/

#ifndef _ZBPRO_APS_FRAME_H
#define _ZBPRO_APS_FRAME_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"

#include "bbZbProApsKeywords.h"
#include "bbZbProApsCommon.h"

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief Macros to work with frame control fields.
 */
#define APS_GET_FRAME_TYPE(frameControl)                GET_BITFIELD_VALUE(frameControl, 0,  2)
#define APS_SET_FRAME_TYPE(frameControl, value)         SET_BITFIELD_VALUE(frameControl, 0,  2, value)

#define APS_GET_DELIVERY_MODE(frameControl)             GET_BITFIELD_VALUE(frameControl, 2,  2)
#define APS_SET_DELIVERY_MODE(frameControl, value)      SET_BITFIELD_VALUE(frameControl, 2,  2, value)

/************************* TYPES *******************************************************/
/**//**
 * \brief ZigBeePRO APS Frame type values.
 */
typedef enum _ZbProApsFrameType_t
{
    APS_DATA_FRAME_TYPE                             = 0x00,
    APS_COMMAND_FRAME_TYPE                          = 0x01,
    APS_ACK_FRAME_TYPE                              = 0x02,
    APS_RESERVED_FRAME_TYPE                         = 0x03
} ZbProApsFrameType_t;

/**//**
 * \brief ZigBeePRO APS Delivery mode values.
 */
typedef enum _ZbProApsDeliveryMode_t
{
    APS_UNICAST_DELIVERY_MODE                       = 0x00,
    APS_RESERVED_DELIVERY_MODE                      = 0x01,
    APS_BROADCAST_DELIVERY_MODE                     = 0x02,
    APS_GROUP_DELIVERY_MODE                         = 0x03
} ZbProApsDeliveryMode_t;

/**//**
 * \brief ZigBeePRO APS Ack format values.
 */
typedef enum _ZbProApsAckFormat_t
{
    APS_ACK_FORMAT_DATA                             = 0x00,
    APS_ACK_FORMAT_COMMAND                          = 0x01
} ZbProApsAckFormat_t;

/**//**
 * \brief ZigBeePRO APS Frame control field private type.
 */
typedef union PACKED _ZbProApsFrameControlField_t
{
    struct
    {
        BitField8_t frameType               : 2;
        BitField8_t deliveryMode            : 2;
        BitField8_t ackFormat               : 1;
        BitField8_t security                : 1;
        BitField8_t ackRequest              : 1;
        BitField8_t extHeader               : 1;
    };
    uint8_t         raw;
} ZbProApsFrameControlField_t;

/**//**
 * \brief ZigBeePRO APS Common Frame Header
 */
typedef struct PACKED _ZbProApsFrameHeaderCommon_t
{
    ZbProApsFrameControlField_t   frameControl;
    union
    {
        ZBPRO_APS_ShortAddr_t     group;
        ZBPRO_APS_EndpointId_t    dstEndpoint;
    };
    ZBPRO_APS_ClusterId_t         clusterId;
    ZBPRO_APS_ProfileId_t         profileId;
    ZBPRO_APS_EndpointId_t        srcEndpoint;
    ZbProApsCounter_t             apsCounter;
} ZbProApsFrameHeaderCommon_t;

/**//**
 * \brief ZigBeePRO APS Non-Group Frame Header
 */
typedef struct PACKED _ZbProApsFrameHeaderNonGroup_t
{
    ZbProApsFrameControlField_t   frameControl;
    ZBPRO_APS_EndpointId_t        dstEndpoint;
    ZBPRO_APS_ClusterId_t         clusterId;
    ZBPRO_APS_ProfileId_t         profileId;
    ZBPRO_APS_EndpointId_t        srcEndpoint;
    ZbProApsCounter_t             apsCounter;
} ZbProApsFrameHeaderNonGroup_t;

/**//**
 * \brief ZigBeePRO APS Command Frame Header
 */
typedef struct PACKED _ZbProApsFrameHeaderCmd_t
{
    ZbProApsFrameControlField_t   frameControl;
    ZbProApsCounter_t             apsCounter;
} ZbProApsFrameHeaderCmd_t;

/**//**
 * \brief Parsed Common APS Frame Header
 */
typedef struct _ZbProApsFrameHeaderCommonParsed_t
{
    ZbProApsFrameType_t         frameType;
    ZbProApsDeliveryMode_t      deliveryMode;
    ZbProApsAckFormat_t         ackFormat;
    Bool8_t                     security;
    Bool8_t                     ackRequest;
    Bool8_t                     extHeader;

    union
    {
        ZBPRO_APS_ShortAddr_t     groupId;
        ZBPRO_APS_EndpointId_t    dstEndpoint;
    };

    ZBPRO_APS_ClusterId_t         clusterId;
    ZBPRO_APS_ProfileId_t         profileId;
    ZBPRO_APS_EndpointId_t        srcEndpoint;
    ZbProApsCounter_t             apsCounter;
} ZbProApsFrameHeaderCommonParsed_t;

/**//**
 * \brief APS Extended Frame Control
 */
typedef enum _ZbProApsFrameExtControl_t
{
    APS_FRAME_EXT_NOT_FRAGMENTED        = 0x00,
    APS_FRAME_EXT_FIRST_FRAGMENT        = 0x01,
    APS_FRAME_EXT_NEXT_FRAGMENT         = 0x02,
    APS_FRAME_EXT_RESERVED              = 0x03
} ZbProApsFrameExtControl_t;

/**//**
 * \brief APS Frame Extended Header for a Not-Fragmented frame
 */
typedef struct _ZbProApsFrameExtHeaderNotFrag_t
{
    ZbProApsFrameExtControl_t   frameControl;
} ZbProApsFrameExtHeaderNotFrag_t;

/**//**
 * \brief APS Frame Extended Header for blocks
 */
typedef struct PACKED _ZbProApsFrameExtHeaderBlock_t
{
    ZbProApsFrameExtControl_t   frameControl;
    uint8_t                     blockNumber;
} ZbProApsFrameExtHeaderBlock_t;

/**//**
 * \brief APS Frame Extended Header for ack
 */
typedef struct PACKED _ZbProApsFrameExtHeaderAck_t
{
    ZbProApsFrameExtControl_t   frameControl;
    uint8_t                     blockNumber;
    uint8_t                     ackMask;        /* Refer to ZBPRO_APS_MAX_MAX_WINDOW_SIZE */
} ZbProApsFrameExtHeaderAck_t;

/**//**
 * \brief Union of all supported APS Extended Header formats.
 */
typedef union PACKED _ZbProApsFrameExtHeader_t
{
    ZbProApsFrameExtControl_t           frameControl;
    ZbProApsFrameExtHeaderNotFrag_t     nonfragmented;
    ZbProApsFrameExtHeaderBlock_t       fragmentedData;
    ZbProApsFrameExtHeaderAck_t         fragmentedAck;
} ZbProApsFrameExtHeader_t;

/************************* PROTOTYPES **************************************************/

/**//*
 * \brief Fills in APS header frame control field
 */
INLINE void fillFrameCtrl(ZbProApsFrameControlField_t *frameControl,
        ZbProApsFrameType_t type, ZbProApsDeliveryMode_t mode, bool security, bool ack, bool extHeader)
{
    APS_SET_FRAME_TYPE(frameControl->raw, type);
    APS_SET_DELIVERY_MODE(frameControl->raw, mode);

    frameControl->security = security;
    frameControl->ackRequest = ack;
    frameControl->extHeader  = extHeader;
}

/**//*
 * \brief Detaches and Parses an APS header from the remaining part of the frame
 */
APS_PRIVATE bool zbProApsFrameDetachParseHeader(SYS_DataPointer_t *const apsHeader, SYS_DataPointer_t *const data,
        ZbProApsFrameHeaderCommonParsed_t *const pParsedHeader, ZbProApsFrameExtHeaderAck_t *const pParsedExtHeader);

#endif /* _ZBPRO_APS_FRAME_H */

/* eof bbZbProApsFrame.h */