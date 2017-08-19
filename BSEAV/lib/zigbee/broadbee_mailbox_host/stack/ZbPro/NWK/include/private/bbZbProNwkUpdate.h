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
*       Contains declaration for update command handler.
*
*******************************************************************************/

#ifndef _ZBPRO_NWK_UPDATE_H
#define _ZBPRO_NWK_UPDATE_H

/************************* INCLUDES ****************************************************/
#include "bbMacSapForZBPRO.h"
#include "private/bbZbProNwkServices.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief Initializer for Update service descriptor.
 */
#define NWK_UPDATE_SERVICE_DESCRIPTOR     \
{                                         \
    .payloadSize    = zbProNwkUpdateSize, \
    .fill           = zbProNwkUpdateFill, \
    .conf           = zbProNwkUpdateConf, \
    .ind            = zbProNwkUpdateInd,  \
    .reset          = zbProNwkUpdateReset,\
}

/************************* TYPES *******************************************************/
/*
 * \brief Update status enumeration.
 */
typedef enum
{
    ZBPRO_NWK_UPDATE_PANID = 0U,
} ZbProNwkUpdateStatus_t;

/**//**
 * \brief Network update request parameters.
 */
typedef struct _ZbProNwkUpdateReqParams_t
{
    ZBPRO_NWK_PanId_t       newPanid;
    ZBPRO_NWK_UpdateId_t    newUpdateId;
    BitField8_t             infoCount   : 5;
    BitField8_t             updateCode  : 3;
} ZbProNwkUpdateReqParams_t;

#define NWK_GET_NWK_UPDATE_INFO_COUNT(options)          GET_BITFIELD_VALUE(options, 0,  5)
#define NWK_GET_NWK_UPDATE_CODE(options)                GET_BITFIELD_VALUE(options, 5,  3)
#define NWK_SET_NWK_UPDATE_INFO_COUNT(options, value)   SET_BITFIELD_VALUE(options, 0,  5, value)
#define NWK_SET_NWK_UPDATE_CODE(options, value)         SET_BITFIELD_VALUE(options, 5,  3, value)

/**//**
 * \brief Network update request descriptor prototype.
 */
typedef struct _ZbProNwkUpdateReqDescriptor_t ZbProNwkUpdateReqDescriptor_t;

/**//**
 * \brief Network update request callback type.
 */
typedef void (*ZbProNwkUpdateReqCallback_t) (ZbProNwkUpdateReqDescriptor_t *const reqDescr,
        ZbProNwkServiceDefaultConfParams_t *const confParams);
/**//**
 * \brief Network update request descriptor type.
 */
struct _ZbProNwkUpdateReqDescriptor_t
{
    ZbProNwkUpdateReqParams_t   params;
    ZbProNwkUpdateReqCallback_t callback;
};

/**//**
 * \brief Network update indication parameters.
 */
typedef struct _ZbProNwkUpdatePandIdIndParams_t
{
    ZBPRO_NWK_UpdateId_t    updateId;
    ZBPRO_NWK_PanId_t       newPanId;
}ZbProNwkUpdatePandIdIndParams_t;

/**//**
 * \brief Network update requests service descriptor.
 */
typedef struct _ZbProNwkUpdateRequestServiceDescr_t
{
    ZbProNwkUpdateReqDescriptor_t  *updateReq;
} ZbProNwkUpdateRequestServiceDescr_t;

/************************* FUNCTIONS PROTOTYPES *****************************************/
/*************************************************************************************//**
  \brief Request to send the network update command.
  \param[in] req - the request descriptor pointer.
*****************************************************************************************/
NWK_PRIVATE void zbProNwkUpdateReq(ZbProNwkUpdateReqDescriptor_t *const req);

/*************************************************************************************//**
  \brief Called when a panId update command has been received.
  \param[in] ind - indication parameters pointer.
*****************************************************************************************/
NWK_PRIVATE void zbProNwkUpdatePanIdInd(ZbProNwkUpdatePandIdIndParams_t *const ind);

/************************************************************************************//**
    \brief Initialize service.
****************************************************************************************/
NWK_PRIVATE ZbProNwkResetServiceHandler_t       zbProNwkUpdateReset;

/************************************************************************************//**
  \brief Returns memory size required for the network update command.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkGetPayloadSizeServiceHandler_t  zbProNwkUpdateSize;

/*************************************************************************************//**
  \brief Fills network update command header and payload.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkFillPacketServiceHandler_t  zbProNwkUpdateFill;

/************************************************************************************//**
    \brief This function invoked when frame dropped into air.
    \param[in] outputBuffer - buffer pointer.
    \param[in] status       - transmission status
****************************************************************************************/
NWK_PRIVATE ZbProNwkConfServiceHandler_t        zbProNwkUpdateConf;

/************************************************************************************//**
    \brief This function invoked when network status frame has been received.
    \param[in] inputBuffer - buffer pointer.
****************************************************************************************/
NWK_PRIVATE ZbProNwkIndServiceHandler_t         zbProNwkUpdateInd;

#endif /* _ZBPRO_NWK_UPDATE_H */

/* eof bbZbProNwkUpdate.h */