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
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   ZCL Profile-Wide for attributes accessing handler private interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_HANDLER_PROFILE_WIDE_ATTRIBUTES_H
#define _BB_ZBPRO_ZCL_HANDLER_PROFILE_WIDE_ATTRIBUTES_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclSapProfileWideAttributes.h"
#include "private/bbZbProZclHandlerProfileWideCommon.h"


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of profile-wide
 *  command Read Attributes.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully; FALSE if it
 *  has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.1.1, figure 2-5.
 */
bool zbProZclHandlerProfileWideComposeReadAttr(
                SYS_DataPointer_t                             *const  zclFramePayload,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of profile-wide
 *  command Read Attributes Response.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully; FALSE if it
 *  has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.2.1, figures 2-6, 2-7.
 */
bool zbProZclHandlerProfileWideComposeReadAttrResp(
                SYS_DataPointer_t                             *const  zclFramePayload,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of profile-wide
 *  command Write Attributes.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully; FALSE if it
 *  has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.3.1, figure 2-10, 2-11.
 */
bool zbProZclHandlerProfileWideComposeWriteAttr(
                SYS_DataPointer_t                             *const  zclFramePayload,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of profile-wide
 *  command Write Attributes Response.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully; FALSE if it
 *  has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.5.1, figure 2-12, 2-13.
 */
bool zbProZclHandlerProfileWideComposeWriteAttrResp(
                SYS_DataPointer_t                             *const  zclFramePayload,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/**//**
 * \brief   Parses ZCL Frame Payload and dismisses dynamic memory of profile-wide command
 *  Read Attributes.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload and dismissed finally.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.1.1, figures 2-5.
 */
void zbProZclHandlerProfileWideParseReadAttr(
                SYS_DataPointer_t                       *const  zclFramePayload,
                ZbProZclLocalPrimitiveParamsPrototype_t *const  indParams);


/**//**
 * \brief   Parses ZCL Frame Payload and dismisses dynamic memory of profile-wide command
 *  Read Attributes Response.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload and dismissed finally.
 * \param[out]      confParams          Pointer to ZCL Local Confirm Parameters Prototype.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.2.1, figures 2-6, 2-7.
 */
void zbProZclHandlerProfileWideParseReadAttrResp(
                SYS_DataPointer_t                             *const  zclFramePayload,
                ZbProZclLocalPrimitiveParamsPrototype_t       *const  confParams,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/**//**
 * \brief   Parses ZCL Frame Payload and dismisses dynamic memory of profile-wide command
 *  Write Attributes.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload and dismissed finally.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.3.1, figures 2-10, 2-11.
 */
void zbProZclHandlerProfileWideParseWriteAttr(
                SYS_DataPointer_t                       *const  zclFramePayload,
                ZbProZclLocalPrimitiveParamsPrototype_t *const  indParams);


/**//**
 * \brief   Parses ZCL Frame Payload and dismisses dynamic memory of profile-wide command
 *  Write Attributes Response.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload and dismissed finally.
 * \param[out]      confParams          Pointer to ZCL Local Confirm Parameters Prototype.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.5.1, figures 2-12, 2-13.
 */
void zbProZclHandlerProfileWideParseWriteAttrResp(
                SYS_DataPointer_t                             *const  zclFramePayload,
                ZbProZclLocalPrimitiveParamsPrototype_t       *const  confParams,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


#endif /* _BB_ZBPRO_ZCL_HANDLER_PROFILE_WIDE_ATTRIBUTES_H */
