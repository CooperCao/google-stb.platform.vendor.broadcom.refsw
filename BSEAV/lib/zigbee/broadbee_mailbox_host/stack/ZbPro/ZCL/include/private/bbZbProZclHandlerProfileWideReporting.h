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
*   ZCL Profile-Wide Reporting handler private interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_HANDLER_PROFILE_WIDE_REPORTING_H
#define _BB_ZBPRO_ZCL_HANDLER_PROFILE_WIDE_REPORTING_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclSapProfileWideReporting.h"
#include "private/bbZbProZclHandlerProfileWideCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief List of Confirmation parameters for bbZbProZclDispatcherTables.h
 */
#define ZBPRO_ZCL_PROFILE_WIDE_REPORTING_CONF_PARAMETERS_LIST \
    ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfParams_t           profileWideCmdConfigureReportingConfParams;         \
    ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationConfParams_t    profileWideCmdReadReportingConfigurationConfParams

/**//**
 * \brief List of indication parameters for bbZbProZclDispatcherTables.h
 */
#define ZBPRO_ZCL_PROFILE_WIDE_REPORTING_IND_PARAMETERS_LIST \
        ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t            profileWideCmdReportAttributesIndParams


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of profile-wide
 *  command Configure Reporting.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully; FALSE if it
 *  has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.7, figure 2-15 and figure 2-16.
 */
bool zbProZclHandlerProfileWideComposeConfigureReporting(
            SYS_DataPointer_t                             *const  zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/**//**
 * \brief   Parses ZCL Frame Payload of profile-wide
 *  command Configure Reporting Response.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.8 figure 2-17 and figure 2-18.
 */
void zbProZclHandlerProfileWideParseConfigureReportingResponse(
            SYS_DataPointer_t                           *const  zclFramePayload,
            ZbProZclLocalPrimitiveParamsPrototype_t     *const  indParam,
            const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of profile-wide
 *  command Read Reporting Configuration.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully; FALSE if it
 *  has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.9, figure 2-19 and figure 2-20.
 */
bool zbProZclHandlerProfileWideComposeReadReportingConfiguration(
            SYS_DataPointer_t                             *const  zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/**//**
 * \brief   Parses ZCL Frame Payload of profile-wide
 *  command Read Reporting Configuration Response.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.10 figure 2-21 and figure 2-22.
 */
void zbProZclHandlerProfileWideParseReadReportingConfigurationResponse(
            SYS_DataPointer_t                           *const  zclFramePayload,
            ZbProZclLocalPrimitiveParamsPrototype_t     *const  indParams,
            const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/**//**
 * \brief   Parses ZCL Frame Payload of profile-wide
 *  command Report Attributes.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.11.1, figure 2-23 and figure 2-24.
 */
void zbProZclHandlerProfileWideParseReportAttributes(
        SYS_DataPointer_t                       *const  zclFramePayload,
        ZbProZclLocalPrimitiveParamsPrototype_t *const  indParams);



#endif
