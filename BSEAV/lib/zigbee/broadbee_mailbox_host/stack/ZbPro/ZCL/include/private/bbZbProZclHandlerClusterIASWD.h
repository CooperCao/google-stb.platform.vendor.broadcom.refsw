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
* FILENAME: $Workfile: trunk/stack/ZbPro/ZCL/include/private/bbZbProZclHandlerClusterIASWD.h $
*
* DESCRIPTION:
*   ZCL IAS WD cluster handler private interface.
*
* $Revision: 7831 $
* $Date: 2015-07-31 13:31:38Z $
*
*****************************************************************************************/
#ifndef _BB_ZBPRO_ZCL_HANDLER_CLUSTER_IASWD_H_
#define _BB_ZBPRO_ZCL_HANDLER_CLUSTER_IASWD_H_

/************************* INCLUDES *****************************************************/
#include "bbZbProZclSapClusterIASWD.h"
#include "bbZbProZclCommon.h"
#include "private/bbZbProZclDispatcher.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Assembles IAS WD cluster Command Uid.
 * \param[in]   commandId       Numeric 8-bit value of Command Identifier local to Cluster
 *  and its Side.
 * \return  Numeric 64-bit of Command Uid.
 * \details
 *  Parameter \c direction is assumed to be Client-to-Server (0x0) for all the defined
 *  commands of this cluster.
 */
#define ZBPRO_ZCL_MAKE_IAS_WD_CLUSTER_COMMAND_UID(commandId)\
        ZBPRO_ZCL_MAKE_CLUSTER_SPECIFIC_COMMAND_UID(\
                ZBPRO_ZCL_MAKE_COMMAND_UID_PREFIX(\
                        ZBPRO_ZCL_FRAME_TYPE_CLUSTER_SPECIFIC_COMMAND, ZBPRO_ZCL_CLUSTER_ID_IAS_WD,\
                        ZBPRO_ZCL_FRAME_DOMAIN_ZCL_STANDARD, /*manufCode*/ 0),\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, commandId)


/**//**
 * \brief   Enumeration of client-to-server commands specific to IAS WD ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.4.2.3, table 8-18.
 */
typedef enum _ZBPRO_ZCL_SapIASWDClientToServerCommandId_t
{
    ZBPRO_ZCL_SAP_IASWD_CMD_ID_START_WARNING                   =  0x00,     /*!< Start warning. */

    ZBPRO_ZCL_SAP_IASWD_CMD_ID_SQUAWK                          =  0x01,     /*!< Squawk. */

} ZBPRO_ZCL_SapIASWDClientToServerCommandId_t;


/**//**
 * \brief   Unique identifiers of IAS WD ZCL cluster commands.
 */
/**@{*/
#define ZBPRO_ZCL_IAS_WD_CMD_UID_START_WARNING\
        ZBPRO_ZCL_MAKE_IAS_WD_CLUSTER_COMMAND_UID(\
                 ZBPRO_ZCL_SAP_IASWD_CMD_ID_START_WARNING)              /*!< Start warning command Uid. */

#define ZBPRO_ZCL_IAS_WD_CMD_UID_SQUAWK\
        ZBPRO_ZCL_MAKE_IAS_WD_CLUSTER_COMMAND_UID(\
                 ZBPRO_ZCL_SAP_IASWD_CMD_ID_SQUAWK)                     /*!< Squawk command Uid. */
/**@}*/


/**//**
 * \brief List of Confirmation parameters for the ZclDispatcherTables.h
 */
#define ZBPRO_ZCL_IAS_WD_CONF_PARAMETERS_LIST                                                                 \
        ZBPRO_ZCL_IASWDCmdStartWarningConfParams_t              ZBPRO_ZCL_IASWDCmdStartWarningConfParams;     \
        ZBPRO_ZCL_IASWDCmdSquawkConfParams_t                    ZBPRO_ZCL_IASWDCmdSquawkConfParams


/************************* PROTOTYPES ***************************************************/

/*-------------------------------- Start warning Cmd ------------------------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of Start warning command
 *  specific to IAS WD ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 8.4.2.3.1, figure 8-13.
 */
bool zbProZclHandlerClusterIASWDComposeStartWarning(
            SYS_DataPointer_t                               *const   zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const   reqParams);


/*-------------------------------- Squawk Cmd ------------------------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of Squawk command
 *  specific to IAS WD ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 8.4.2.3.1, figure 8-13.
 */
bool zbProZclHandlerClusterIASWDComposeSquawk(
            SYS_DataPointer_t                               *const   zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const   reqParams);

#endif
