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
*   ZCL Color Control cluster handler private interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/
#ifndef _BB_ZBPRO_ZCL_HANDLER_CLUSTER_COLOR_CONTROL_H_
#define _BB_ZBPRO_ZCL_HANDLER_CLUSTER_COLOR_CONTROL_H_

/************************* INCLUDES *****************************************************/
#include "bbZbProZclSapClusterColorControl.h"
#include "bbZbProZclCommon.h"
#include "private/bbZbProZclDispatcher.h"

/************************* DEFINITIONS **************************************************/

/**//**
 * \brief   Enumeration of client-to-server commands specific to Color Control ZCL cluster.
 * \note
 * Here are mandatory only commands.
 * \note
 * Color Control ZCL cluster has no server-to-client commands.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3, table 5-11.
 */
typedef enum _ZBPRO_ZCL_SapColorControlClientToServerCommandId_t
{
    ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_MOVE_TO_COLOR                        =  0x07,  /*!< Move to Color. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_MOVE_COLOR                           =  0x08,  /*!< Move Color. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_STEP_COLOR                           =  0x09,  /*!< Step Color. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_ENHANCED_MOVE_TO_HUE                 =  0x40,  /*!< Enhanced Move to Hue. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_ENHANCED_MOVE_HUE                    =  0x41,  /*!< Enhanced Move Hue. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_ENHANCED_STEP_HUE                    =  0x42,  /*!< Enhanced Step Hue. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_ENHANCED_MOVE_TO_HUE_AND_SATURATION  =  0x43,  /*!< Enhanced Move to Hue
                                                                                           and Saturation. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_COLOR_LOOP_SET                       =  0x44,  /*!< Color Loop Set. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_STOP_MOVE_STEP                       =  0x47,  /*!< Stop Move Step. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_MOVE_COLOR_TEMPERATURE               =  0x4B,  /*!< Move Color Temperature. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_STEP_COLOR_TEMPERATURE               =  0x4C,  /*!< Step Color Temperature. */

} ZBPRO_ZCL_SapColorControlClientToServerCommandId_t;


/**//**
 * \brief   Assembles Color Control cluster Command Uid.
 * \param[in]   commandId       Numeric 8-bit value of Command Identifier local to Cluster
 *  and its Side.
 * \return  Numeric 64-bit of Command Uid.
 * \details
 *  Parameter \c direction is assumed to be Client-to-Server (0x0) for all the defined
 *  commands of this cluster.
 */
#define ZBPRO_ZCL_MAKE_COLOR_CONTROL_CLUSTER_COMMAND_UID(commandId)\
        ZBPRO_ZCL_MAKE_CLUSTER_SPECIFIC_COMMAND_UID(\
                ZBPRO_ZCL_MAKE_COMMAND_UID_PREFIX(\
                        ZBPRO_ZCL_FRAME_TYPE_CLUSTER_SPECIFIC_COMMAND, ZBPRO_ZCL_CLUSTER_ID_COLOR_CONTROL,\
                        ZBPRO_ZCL_FRAME_DOMAIN_ZCL_STANDARD, /*manufCode*/ 0),\
                ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, commandId)


/**//**
 * \brief   Unique identifiers of Color Control ZCL cluster commands.
 */
/**@{*/
#define ZBPRO_ZCL_COLOR_CONTROL_CMD_UID_MOVE_TO_COLOR\
        ZBPRO_ZCL_MAKE_COLOR_CONTROL_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_MOVE_TO_COLOR)               /*!< Move to Color command Uid. */

#define ZBPRO_ZCL_COLOR_CONTROL_CMD_UID_MOVE_COLOR\
        ZBPRO_ZCL_MAKE_COLOR_CONTROL_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_MOVE_COLOR)                  /*!< Move Color command Uid. */

#define ZBPRO_ZCL_COLOR_CONTROL_CMD_UID_STEP_COLOR\
        ZBPRO_ZCL_MAKE_COLOR_CONTROL_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_STEP_COLOR)                  /*!< Step Colo command Uid. */

#define ZBPRO_ZCL_COLOR_CONTROL_CMD_UID_ENHANCED_MOVE_TO_HUE\
        ZBPRO_ZCL_MAKE_COLOR_CONTROL_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_ENHANCED_MOVE_TO_HUE)        /*!< Enhanced Move to Hue */

#define ZBPRO_ZCL_COLOR_CONTROL_CMD_UID_ENHANCED_MOVE_HUE\
        ZBPRO_ZCL_MAKE_COLOR_CONTROL_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_ENHANCED_MOVE_HUE)           /*!< Enhanced Move Hue */

#define ZBPRO_ZCL_COLOR_CONTROL_CMD_UID_ENHANCED_STEP_HUE\
        ZBPRO_ZCL_MAKE_COLOR_CONTROL_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_ENHANCED_STEP_HUE)           /*!< Enhanced Step Hue */

#define ZBPRO_ZCL_COLOR_CONTROL_CMD_UID_ENHANCED_MOVE_TO_HUE_AND_SATURATION\
        ZBPRO_ZCL_MAKE_COLOR_CONTROL_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_ENHANCED_MOVE_TO_HUE_AND_SATURATION)     /*!< Enhanced Move to Hue
                                                                                               and Saturation.*/
#define ZBPRO_ZCL_COLOR_CONTROL_CMD_UID_COLOR_LOOP_SET\
        ZBPRO_ZCL_MAKE_COLOR_CONTROL_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_COLOR_LOOP_SET)              /*!< Color Loop Set. */

#define ZBPRO_ZCL_COLOR_CONTROL_CMD_UID_STOP_MOVE_STEP\
        ZBPRO_ZCL_MAKE_COLOR_CONTROL_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_STOP_MOVE_STEP)              /*!< Stop Move Step. */

#define ZBPRO_ZCL_COLOR_CONTROL_CMD_UID_MOVE_COLOR_TEMPERATURE\
        ZBPRO_ZCL_MAKE_COLOR_CONTROL_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_MOVE_COLOR_TEMPERATURE)      /*!< Move Color Temperature. */

#define ZBPRO_ZCL_COLOR_CONTROL_CMD_UID_STEP_COLOR_TEMPERATURE\
        ZBPRO_ZCL_MAKE_COLOR_CONTROL_CLUSTER_COMMAND_UID(\
                ZBPRO_ZCL_SAP_COLOR_CONTROL_CMD_ID_STEP_COLOR_TEMPERATURE)      /*!< Step Color Temperature. */
/**@}*/


/**//**
 * \brief List of Confirmation parameters for bbZbProZclDispatcherTables.h
 */
#define ZBPRO_ZCL_COLOR_CONTROL_CONF_PARAMETERS_LIST \
    ZBPRO_ZCL_ColorControlCmdMoveToColorConfParams_t        ZBPRO_ZCL_ColorControlCmdMoveToColorConfParams;         \
    ZBPRO_ZCL_ColorControlCmdMoveColorConfParams_t          ZBPRO_ZCL_ColorControlCmdMoveColorConfParams;           \
    ZBPRO_ZCL_ColorControlCmdStepColorConfParams_t          ZBPRO_ZCL_ColorControlCmdStepColorConfParams;           \
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueConfParams_t  ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueConfParams;   \
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueConfParams_t    ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueConfParams;     \
    ZBPRO_ZCL_ColorControlCmdEnhancedStepHueConfParams_t    ZBPRO_ZCL_ColorControlCmdEnhancedStepHueConfParams;     \
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationConfParams_t  ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationConfParams;   \
    ZBPRO_ZCL_ColorControlCmdColorLoopSetConfParams_t        ZBPRO_ZCL_ColorControlCmdColorLoopSetConfParams;       \
    ZBPRO_ZCL_ColorControlCmdStopMoveStepConfParams_t        ZBPRO_ZCL_ColorControlCmdStopMoveStepConfParams;       \
    ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureConfParams_t  ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureConfParams;     \
    ZBPRO_ZCL_ColorControlCmdStepColorTemperatureConfParams_t  ZBPRO_ZCL_ColorControlCmdStepColorTemperatureConfParams


/*********************************** PROTOTYPES ***************************************************/

/*-------------------------------- Move to Color Cmd ------------------------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of  Move to Color command
 * specific to Color Control ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.10, figure 5-9.
 */
bool zbProZclHandlerClusterColorControlComposeMoveToColorReq(
            SYS_DataPointer_t                               *const   zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const   reqParams);


/*-------------------------------- Move Color Cmd ------------------------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of  Move Color command
 * specific to Color Control ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.11, figure 5-10.
 */
bool zbProZclHandlerClusterColorControlComposeMoveColorReq(
            SYS_DataPointer_t                               *const   zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const   reqParams);


/*-------------------------------- Step Color Cmd ------------------------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of  Step Color command
 * specific to Color Control ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.12, figure 5-11.
 */
bool zbProZclHandlerClusterColorControlComposeStepColorReq(
            SYS_DataPointer_t                               *const   zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const   reqParams);


/*-------------------------------- Enhanced Move to Hue Cmd ------------------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of Enhanced Move to Hue command
 * specific to Color Control ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.14, figure 5-13.
 */
bool zbProZclHandlerClusterColorControlComposeEnhancedMoveToHueReq(
            SYS_DataPointer_t                               *const   zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const   reqParams);


/*-------------------------------- Enhanced Move Hue Cmd ------------------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of Enhanced Move Hue command
 * specific to Color Control ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.15, figure 5-14.
 */
bool zbProZclHandlerClusterColorControlComposeEnhancedMoveHueReq(
            SYS_DataPointer_t                               *const   zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const   reqParams);


/*-------------------------------- Enhanced Step Hue Cmd ------------------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of Enhanced Step Hue command
 * specific to Color Control ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.16, figure 5-15.
 */
bool zbProZclHandlerClusterColorControlComposeEnhancedStepHueReq(
            SYS_DataPointer_t                               *const   zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const   reqParams);


/*-------------------------------- Enhanced Move to Hue and Saturation Cmd -----------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of Enhanced Move to Hue and Saturation command
 * specific to Color Control ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.17, figure 5-16.
 */
bool zbProZclHandlerClusterColorControlComposeEnhancedMoveToHueAndSaturationReq(
            SYS_DataPointer_t                               *const   zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const   reqParams);


/*-------------------------------- Color Loop Set Cmd -------------------------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of Color Loop Set command
 * specific to Color Control ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.18, figure 5-17.
 */
bool zbProZclHandlerClusterColorControlComposeColorLoopSetReq(
            SYS_DataPointer_t                               *const   zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const   reqParams);


/*-------------------------------- Stop Move Step Cmd -----------------------------------------*/

/* Stop Move Step request has no custom parameters. Nothing to implement here. */


/*-------------------------------- Move Color Temperature Cmd -----------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of Move Color Temperature command
 * specific to Color Control ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.20, figure 5-19.
 */
bool zbProZclHandlerClusterColorControlComposeMoveColorTemperatureReq(
            SYS_DataPointer_t                               *const   zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const   reqParams);


/*-------------------------------- Step Color Temperature Cmd -----------------------------------------*/
/**//**
 * \brief   Allocates dynamic memory and composes ZCL Frame Payload of Step Color Temperature command
 * specific to Color Control ZCL cluster.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully;
 *          FALSE if it has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.21, figure 5-20.
 */
bool zbProZclHandlerClusterColorControlComposeStepColorTemperatureReq(
            SYS_DataPointer_t                               *const   zclFramePayload,
            const ZbProZclLocalPrimitiveParamsPrototype_t   *const   reqParams);


#endif
