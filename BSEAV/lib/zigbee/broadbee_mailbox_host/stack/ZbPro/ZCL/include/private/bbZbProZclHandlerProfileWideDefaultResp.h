/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   ZCL Profile-Wide for default response handler private interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_HANDLER_PROFILE_WIDE_DEFAULT_RESP_H
#define _BB_ZBPRO_ZCL_HANDLER_PROFILE_WIDE_DEFAULT_RESP_H


/************************* INCLUDES *****************************************************/
#include "private/bbZbProZclHandlerProfileWideCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \name    Unique identifier of profile-wide ZCL command Default Response.
 */
/**@{*/
#define ZBPRO_ZCL_PROFILE_WIDE_CMD_UID_DEFAULT_RESP\
        ZBPRO_ZCL_MAKE_PROFILE_WIDE_COMMAND_UID(\
                ZBPRO_ZCL_PROFILE_WIDE_CMD_ID_DEFAULT_RESP)             /*!< Default Response command Uid. */
/**@}*/


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of profile-wide
 *  command Default Response.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully; FALSE if it
 *  has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.12.1, figure 2-25.
 */
bool zbProZclHandlerProfileWideComposeDefaultResp(
                SYS_DataPointer_t                             *const  zclFramePayload,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/**//**
 * \brief   Parses ZCL Frame Payload and dismisses dynamic memory of profile-wide command
 *  Default Response.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload and dismissed finally.
 * \param[out]      confParams          Pointer to ZCL Local Confirm Parameters Prototype.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 *  Not used particularly in this function; may be assigned with NULL or other value.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.12.1, figures 2-25.
 */
void zbProZclHandlerProfileWideParseDefaultResp(
                SYS_DataPointer_t                             *const  zclFramePayload,
                ZbProZclLocalPrimitiveParamsPrototype_t       *const  confParams,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParamsNotUsed);


#endif /* _BB_ZBPRO_ZCL_HANDLER_PROFILE_WIDE_DEFAULT_RESP_H */
