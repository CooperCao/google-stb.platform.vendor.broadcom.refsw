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
*       ZCL Dispatcher Tables definitions and interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZCL_DISPATCHER_TABLES_H
#define _BB_ZBPRO_ZCL_DISPATCHER_TABLES_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"
#include "private/bbZbProZclDispatcher.h"

#include "private/bbZbProZclHandlerProfileWideAttributes.h"
#include "private/bbZbProZclHandlerProfileWideReporting.h"
#include "private/bbZbProZclHandlerProfileWideDefaultResp.h"
#include "private/bbZbProZclHandlerAttributesDiscover.h"
#include "private/bbZbProZclHandlerClusterBasic.h"
#include "private/bbZbProZclHandlerClusterIdentify.h"
#include "private/bbZbProZclHandlerClusterGroups.h"
#include "private/bbZbProZclHandlerClusterScenes.h"
#include "private/bbZbProZclHandlerClusterOnOff.h"
#include "private/bbZbProZclHandlerClusterLevelControl.h"
#include "private/bbZbProZclHandlerClusterColorControl.h"
#include "private/bbZbProZclHandlerClusterDoorLock.h"
#include "private/bbZbProZclHandlerClusterWindowCovering.h"
#include "private/bbZbProZclHandlerClusterIASZone.h"
#include "private/bbZbProZclHandlerClusterIASACE.h"
#include "private/bbZbProZclHandlerClusterIASWD.h"
#include "private/bbZbProZclHandlerClusterOTAU.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Constant used instead of the Service Id for the case when Service is not
 *  enabled or not implemented at all.
 */
#define ZBPRO_ZCL_SERVICE_DISABLED\
        RPC_SERVICE_ID_LAST_RECORD


/**//**
 * \brief   Constant used instead of the Service Id for the case when Service is not
 *  enabled or not implemented at all but must be able to generate supported profile-wide
 *  commands.
 */
#define ZBPRO_ZCL_SERVICE_GHOST\
        0xEEEEEEEE


/**//**
 * \brief   Union of all supported ZCL Local Confirmation structures.
 * \details
 *  Insert all the implemented ZCL Local Confirmation Parameters structures here.
 * \details
 *  This union is used by ZCL Dispatcher when it allocates a chunk of memory in the
 *  program stack for parameters of Local Confirmation on finished Request. The very first
 *  field - \c common - primitive parameters prototype - plays as a hook and is passed by
 *  pointer to confirmation handler function provided by the higher-level layer, and also
 *  it's passed as the destination data structure to received command parser.
 */
typedef union _ZbProZclDispatcherUnionConfParams_t
{
    /* Obligatory part common to all indication types. */

    ZbProZclLocalPrimitiveParamsPrototype_t                 common;
            /*!< Obligatory parameters common to all ZCL Local Confirmations. */


    /* Identify cluster. */

    ZBPRO_ZCL_IdentifyCmdConfParams_t                       identifyCmd;
            /*!< Parameters of ZCL Local Confirm of Request of Identify, IdentifyResponse, IdentifyQuery,
                IdentifyQueryResponse command of Identify cluster. */


    /* Groups cluster. */

    ZBPRO_ZCL_GROUPS_CONF_PARAMETERS_LIST;


    /* Scenes cluster. */

    ZBPRO_ZCL_ScenesCmdAddSceneConfParams_t                 scenesAddSceneConf;
            /*!< Parameters of ZCL Local Confirm of Request of SceneAdd command of Scenes cluster. */

    ZBPRO_ZCL_ScenesCmdViewSceneConfParams_t                scenesViewSceneConf;
            /*!< Parameters of ZCL Local Confirm of Request of SceneView command of Scenes cluster. */

    ZBPRO_ZCL_ScenesCmdRemoveSceneConfParams_t              scenesRemoveSceneConf;
            /*!< Parameters of ZCL Local Confirm of Request of SceneRemove command of Scenes cluster. */

    ZBPRO_ZCL_ScenesCmdRemoveAllScenesConfParams_t          scenesRemoveAllScenesConf;
            /*!< Parameters of ZCL Local Confirm of Request of SceneRemoveAll command of Scenes cluster. */

    ZBPRO_ZCL_ScenesCmdStoreSceneConfParams_t               scenesStoreSceneConf;
            /*!< Parameters of ZCL Local Confirm of Request of SceneStore command of Scenes cluster. */

    ZBPRO_ZCL_ScenesCmdConfParams_t                         scenesRecallSceneConf;
            /*!< Parameters of ZCL Local Confirm of Request of SceneRecall command of Scenes cluster. */


    /* On/Off cluster. */

    ZBPRO_ZCL_OnOffCmdConfParams_t                          onOffCmd;
            /*!< Parameters of ZCL Local Confirm of Request of OnOffToggle commands of On/Off cluster. */


    /* Level Control cluster. */

    ZBPRO_ZCL_LevelControlCmdConfParams_t                   levelControlCmd;
            /*!< Parameters of ZCL Local Confirm of Request of MoveToLevel, Move, Step, Stop commands (with or without
                On/Off) of Level Control cluster. */


    /* Door Lock cluster. */

    ZBPRO_ZCL_DoorLockCmdLockUnlockConfParams_t             doorLockCmdLockUnlock;
            /*!< Parameters of ZCL Local Confirm of Request of Lock, Unlock commands of Door Lock cluster. */


    /* Window Covering cluster. */

    ZBPRO_ZCL_WindowCoveringCmdConfParams_t                 windowCoveringCmd;
            /*!< Parameters of ZCL Local Confirm of Request of Up, Down, Open, Close, Stop, GotoLiftPercentage,
                GotoLiftPercentage commands of Window Covering cluster. */


    /* IAS Zone cluster. */

    ZBPRO_ZCL_IAS_ZONE_CONF_PARAMETERS_LIST;


    /* IAS ACE cluster. */

    ZBPRO_ZCL_SAP_IAS_ACE_CONF_PARAMETERS_LIST;


    /* IAS WD cluster. */

    ZBPRO_ZCL_IAS_WD_CONF_PARAMETERS_LIST;


    /* Color Control Cluster. */

    ZBPRO_ZCL_COLOR_CONTROL_CONF_PARAMETERS_LIST;


    /* Profile-wide. */

    ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t            profileWideCmdReadAttr;
            /*!< Parameters of ZCL Local Confirm of Request of Read Attributes profile-wide command. */

    ZBPRO_ZCL_ProfileWideCmdReadAttrResponseConfParams_t    profileWideCmdReadAttrReasp;
            /*!< Parameters of ZCL Local Confirm of Request of Read Attributes Response profile-wide command. */

    ZBPRO_ZCL_ProfileWideCmdWriteAttrConfParams_t           profileWideCmdWriteAttr;
            /*!< Parameters of ZCL Local Confirm of Request of Write Attributes profile-wide command. */

    ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseConfParams_t   profileWideCmdWriteAttrReasp;
            /*!< Parameters of ZCL Local Confirm of Request of Write Attributes Response profile-wide command. */


    /* Profile-wide Reporting. */

    ZBPRO_ZCL_PROFILE_WIDE_REPORTING_CONF_PARAMETERS_LIST;

    /* OTA Upgrade cluster. */
    ZBPRO_ZCL_OTAUCmdQueryNextImageResponseConfParams_t     otaQueryNextImageReqsponseConfParams;
    ZBPRO_ZCL_OTAUCmdImageBlockResponseConfParams_t         otaImageBlockResponseConfParams;
    ZBPRO_ZCL_OTAUCmdUpgradeEndResponseConfParams_t         otaUpgradeEndResponseConfParams;


} ZbProZclDispatcherUnionConfParams_t;


/**//**
 * \brief   Union of all supported ZCL Local Indication structures.
 * \details
 *  Insert all the implemented ZCL Local Indication Parameters structures here.
 * \details
 *  This union is used by ZCL Dispatcher when it allocates a chunk of memory in the
 *  program stack for parameters of Local Indication on received remote Request. The very
 *  first field - \c common - primitive parameters prototype - plays as a hook and is
 *  passed by pointer to indication handler function provided by the higher-level layer,
 *  and also it's passed as the destination data structure to received command parser.
 */
typedef union _ZbProZclDispatcherUnionIndParams_t
{
    /* Obligatory part common to all indication types. */

    ZbProZclLocalPrimitiveParamsPrototype_t                 common;
            /*!< Obligatory parameters common to all ZCL Local Confirmations. */


    /* Identify cluster. */

    ZBPRO_ZCL_IdentifyCmdIdentifyIndParams_t                identifyCmdIdentify;
            /*!< Parameters of ZCL Local Indication of Identify command of Identify cluster. */

    ZBPRO_ZCL_IdentifyCmdIdentifyQueryIndParams_t           identifyCmdIdentifyQuery;
            /*!< Parameters of ZCL Local Indication of IdentifyQuery command of Identify cluster. */

    ZBPRO_ZCL_IdentifyCmdIdentifyQueryResponseIndParams_t   identifyCmdIdentifyQueryResponse;
            /*!< Parameters of ZCL Local Indication of IdentifyQueryResponse command of Identify cluster. */


    /* Groups cluster. */

    ZBPRO_ZCL_GROUPS_INDICATION_PARAMETERS_LIST;


    /* Scenes cluster. */

    ZBPRO_ZCL_ScenesCmdAddSceneResponseIndParams_t          scenesAddSceneRespInd;
            /*!< Parameters of ZCL AddScene response of Scenes cluster. */

    ZBPRO_ZCL_ScenesCmdViewSceneResponseIndParams_t         scenesViewSceneRespInd;
            /*!< Parameters of ZCL ViewScene response of Scenes cluster. */

    ZBPRO_ZCL_ScenesCmdRemoveAllScenesResponseIndParams_t   scenesRemoveAllScenesRespInd;
            /*!< Parameters of ZCL RemoveAllScenes response of Scenes cluster. */

    ZBPRO_ZCL_ScenesCmdGetSceneMembershipResponseIndParams_t scenesGetSceneMembershipRespInd;
            /*!< Parameters of ZCL GetSceneMembership response of Scenes cluster. */


    /* On/Off cluster has no Local Indications. */

    /* Level Control cluster has no Local Indications. */

    /* Door Lock cluster has no Local Indications. */

    /* Window Covering cluster has no Local Indications. */


    /* IAS Zone cluster. */

    ZBPRO_ZCL_IAS_ZONE_INDICATION_PARAMETERS_LIST;


    /* IAS ACE cluster. */

    ZBPRO_ZCL_SAP_IAS_ACE_INDICATION_PARAMETERS_LIST;


    /* IAS WD cluster has no Local Indications. */

    /* Color Control cluster has no Local Indications. */


    /* Profile-wide. */

    ZBPRO_ZCL_ProfileWideCmdReadAttrIndParams_t             profileWideCmdReadAttr;
            /*!< Parameters of ZCL Local Indication of Read Attributes profile-wide command. */

    ZBPRO_ZCL_ProfileWideCmdWriteAttrIndParams_t            profileWideCmdWriteAttr;
            /*!< Parameters of ZCL Local Indication of Write Attributes profile-wide command. */


    /* Profile-wide Reporting */

    ZBPRO_ZCL_PROFILE_WIDE_REPORTING_IND_PARAMETERS_LIST;

    /* OTA Upgrade cluster */
    ZBPRO_ZCL_OTAUCmdQueryNextImageRequestIndParams_t   otauQueryNextImageRequestInd;
    ZBPRO_ZCL_OTAUCmdImageBlockRequestIndParams_t       otauImageBlockRequestInd;
    ZBPRO_ZCL_OTAUCmdUpgradeEndRequestIndParams_t       otauUpgradeEndRequestInd;

} ZbProZclDispatcherUnionIndParams_t;


/**//**
 * \brief   Data type for entry point of ZCL Local Request Composer function.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be assigned
 *  with allocated dynamic memory for ZCL Frame Payload.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \return  TRUE if ZCL Frame Payload was allocated and composed successfully; FALSE if it
 *  has failed to allocate appropriate amount of dynamic memory for ZCL Frame Payload.
 * \details
 *  ZCL Command Processor has to provide function of this type if this local node
 *  implements generation of such a command (as a request or unsolicited response), and
 *  the frame of this particular command includes at least one obligatory or optional
 *  field in addition to the frame ZCL Header. If this command has no ZCL Payload but only
 *  ZCL Header, its command processor has not to provide this composer callback function.
 * \details
 *  Composer function of this type must not allocate memory for ZCL Frame Header and/or
 *  skip as much octets in allocated chunk of memory as prospective ZCL Frame Header will
 *  occupy. Memory for header is allocated by ZCL Dispatcher; this function is responsible
 *  only for allocation and composing of ZCL Frame Payload.
 * \details
 *  If the higher layer provides local request parameters in the form of payload (saved in
 *  dynamic memory) for the prospective ZCL Frame Payload, this function shall duplicate
 *  the payload provided by the higher layer and include its copy into the resulting ZCL
 *  Frame Payload, but not the original payload object. This copy within ZCL Frame Payload
 *  will be passed to APS layer for transmission and later will be dismissed by ZCL
 *  Dispatcher automatically. The original payload object provided by the higher layer
 *  must be dismissed by the higher layer itself.
 * \details
 *  In the case of misuse of parameters of the original ZCL Local Request by calling
 *  application, but if there is no not-enough-memory error, the composer function shall
 *  nevertheless allocate and compose ZCL Frame Payload until the end (or partially until
 *  it's just possible to continue composing) and return TRUE - i.e., it shall behave
 *  completely similar to the case of successful frame composing case. No parameters error
 *  shall be reported by this function.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.3.1, figure 2-2.
 */
typedef bool  ZbProZclDispatcherComposeReqCallback_t(
                SYS_DataPointer_t                             *const  zclFramePayload,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/**//**
 * \brief   Data type for entry point of ZCL Remote Request Parser function for the case
 *  of commands processed within client transaction.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload.
 * \param[out]      confParams          Pointer to ZCL Local Confirm Parameters Prototype.
 * \param[in]       reqParams           Pointer to ZCL Local Request Parameters Prototype.
 * \details
 *  ZCL Command Processor has to provide function of this type if this local node
 *  implements reception of such a command (as a remote solicited response), and the frame
 *  of this particular command includes at least one obligatory or optional field in
 *  addition to the frame ZCL Header. If this command has no ZCL Payload but only ZCL
 *  Header, its command processor has not to provide this parser callback function.
 * \details
 *  Parser function of this type doesn't have to dismiss memory given with
 *  \p zclFramePayload. ZCL Dispatcher by default will free this memory right after this
 *  function returns. But if this function decides to keep this chunk of dynamic memory
 *  (or its part) to transfer it within parameters of confirmation to the higher layer,
 *  then it shall instruct ZCL Dispatcher to refuse of freeing the \p zclFramePayload. To
 *  do this assign the input-output parameter \p zclFramePayload with Empty in this
 *  function prior to exit. If the dynamic memory must be kept partially, then this
 *  function becomes responsible for freeing the unused part of such memory.
 * \details
 *  Parser function of this type has not to dismiss memory occupied by ZCL Frame Header
 *  and/or skip as much octets in provided chunk of memory as actual ZCL Frame Header
 *  occupies. The \p zclFramePayload describes the pure ZCL Frame Payload part of the
 *  received frame, i.e. already without ZCL Frame Header. Memory occupied by header will
 *  be already dismissed by ZCL Dispatcher when this function is called; this function is
 *  responsible only for parsing (and dismissing, optionally) of ZCL Frame Payload.
 * \details
 *  If the higher layer waits for confirmation parameters (all of them or partially) in
 *  the form of payload (saved in dynamic memory) in the same format as they are given in
 *  ZCL Frame Payload or in a different format, this function is fully responsible for
 *  allocating and filling such dynamic memory. If the chunk of dynamic memory described
 *  with \p zclFramePayload may be fully or partially reused for such confirmation
 *  parameters, then this function may (and strongly recommended to) use the memory
 *  received from ZCL Dispatcher and transfer it as-is or partially to the higher layer.
 *  In this case the higher layer becomes responsible for dismissing the part of memory
 *  that was transferred to it; and the rest of dynamic memory that was not reused by this
 *  function must be dismissed by it itself or by ZCL Dispatcher. If this function needs
 *  the amount of dynamic memory that overcomes that one received from ZCL Dispatcher, it
 *  shall try to allocate it. In the case of failure to allocate such additional memory
 *  this function shall reassign the \p indParams.overallStatus with either
 *  SOFTWARE_FAILURE or FAILURE; this function is not mandatory responsible for dismissing
 *  the dynamic memory occupied with the received frame payload (ZCL Dispatcher may free
 *  it after this function returns). Returning such failure status instructs
 *  ZCL Dispatcher to drop the received frame and respond with Default Response with
 *  corresponding failure status (if it's allowed).
 * \details
 *  In the case of misuse of parameters in the received ZCL Remote Response, but if there
 *  is no not-enough-memory error, the parser function shall try to parse ZCL Frame
 *  Payload until the end (or partially until it's just possible to continue parsing). In
 *  general this function is not responsible for validation of the received command
 *  parameters (the higher layer is responsible for that), but in particular cases it may
 *  perform some validation even of parameters' values. And in any case this function is
 *  responsible for the received command format validation. This function must not hide
 *  any error in the command format, particularly it must not left any of parameters
 *  nonassigned (or assign them with some default values as well). For the case when the
 *  received ZCL Remote Response command has invalid format (for example, it has
 *  insufficient length, obligatory field is omitted, reserved bit contains nonzero value,
 *  or any other case according to the service specific), this function shall reassign the
 *  \p confParams.overallStatus with either MALFORMED_COMMAND or INVALID_FIELD, or
 *  RESERVED_FIELD_NOT_ZERO, or other failure status (at least FAILURE). It instructs ZCL
 *  Dispatcher to drop the received frame and respond with Default Response with
 *  corresponding failure status (if it's allowed). This function, in general, may use
 *  parameters of the original Request to validate parameters of the received Response.
 *  Parameters of the original Request are given with \p reqParams.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.3.1, figure 2-2.
 */
typedef void  ZbProZclDispatcherParseConfCallback_t(
                SYS_DataPointer_t                             *const  zclFramePayload,
                ZbProZclLocalPrimitiveParamsPrototype_t       *const  confParams,
                const ZbProZclLocalPrimitiveParamsPrototype_t *const  reqParams);


/**//**
 * \brief   Data type for entry point of ZCL Remote Request Parser function for the case
 *  of commands indicated separately.
 * \param[in/out]   zclFramePayload     Pointer to payload descriptor which to be parsed
 *  as ZCL Frame Payload.
 * \param[out]      indParams           Pointer to ZCL Local Indication Parameters
 *  Prototype.
 * \details
 *  ZCL Command Processor has to provide function of this type if this local node
 *  implements reception of such a command (as a remote request or unsolicited response),
 *  and the frame of this particular command includes at least one obligatory or optional
 *  field in addition to the frame ZCL Header. If this command has no ZCL Payload but only
 *  ZCL Header, its command processor has not to provide this parser callback function.
 * \details
 *  Parser function of this type doesn't have to dismiss memory given with
 *  \p zclFramePayload. ZCL Dispatcher by default will free this memory right after this
 *  function returns. But if this function decides to keep this chunk of dynamic memory
 *  (or its part) to transfer it within parameters of indication to the higher layer, then
 *  it shall instruct ZCL Dispatcher to refuse of freeing the \p zclFramePayload. To do
 *  this assign the input-output parameter \p zclFramePayload with Empty in this function
 *  prior to exit. If the dynamic memory must be kept partially, then this function
 *  becomes responsible for freeing the unused part of such memory.
 * \details
 *  Parser function of this type has not to dismiss memory occupied by ZCL Frame Header
 *  and/or skip as much octets in provided chunk of memory as actual ZCL Frame Header
 *  occupies. The \p zclFramePayload describes the pure ZCL Frame Payload part of the
 *  received frame, i.e. already without ZCL Frame Header. Memory occupied by header will
 *  be already dismissed by ZCL Dispatcher when this function is called; this function is
 *  responsible only for parsing (and dismissing, optionally) of ZCL Frame Payload.
 * \details
 *  If the higher layer waits for indication parameters (all of them or partially) in the
 *  form of payload (saved in dynamic memory) in the same format as they are given in ZCL
 *  Frame Payload or in a different format, this function is fully responsible for
 *  allocating and filling such dynamic memory. If the chunk of dynamic memory described
 *  with \p zclFramePayload may be fully or partially reused for such indication
 *  parameters, then this function may (and strongly recommended to) use the memory
 *  received from ZCL Dispatcher and transfer it as-is or partially to the higher layer.
 *  In this case the higher layer becomes responsible for dismissing the part of memory
 *  that was transferred to it; and the rest of dynamic memory that was not reused by this
 *  function must be dismissed by it itself or by ZCL Dispatcher. If this function needs
 *  the amount of dynamic memory that overcomes that one received from ZCL Dispatcher, it
 *  shall try to allocate it. In the case of failure to allocate such additional memory
 *  this function shall reassign the \p indParams.overallStatus with either
 *  SOFTWARE_FAILURE or FAILURE; this function is not mandatory responsible for dismissing
 *  the dynamic memory occupied with the received frame payload (ZCL Dispatcher may free
 *  it after this function returns). Returning such failure status instructs
 *  ZCL Dispatcher to drop the received frame and respond with Default Response with
 *  corresponding failure status (if it's allowed).
 * \details
 *  In the case of misuse of parameters in the received ZCL Remote Request, but if there
 *  is no not-enough-memory error, the parser function shall try to parse ZCL Frame
 *  Payload until the end (or partially until it's just possible to continue parsing). In
 *  general this function is not responsible for validation of the received command
 *  parameters (the higher layer is responsible for that), but in particular cases it may
 *  perform some validation even of parameters' values. And in any case this function is
 *  responsible for the received command format validation. This function must not hide
 *  any error in the command format, particularly it must not left any of parameters
 *  nonassigned (or assign them with some default values as well). For the case when the
 *  received ZCL Remote Request command has invalid format (for example, it has
 *  insufficient length, obligatory field is omitted, reserved bit contains nonzero value,
 *  or any other case according to the service specific), this function shall reassign the
 *  \p indParams.overallStatus with either MALFORMED_COMMAND or INVALID_FIELD, or
 *  RESERVED_FIELD_NOT_ZERO, or other failure status (at least FAILURE). It instructs ZCL
 *  Dispatcher to drop the received frame and respond with Default Response with
 *  corresponding failure status (if it's allowed).
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.3.1, figure 2-2.
 */
typedef void  ZbProZclDispatcherParseIndCallback_t(
                SYS_DataPointer_t                       *const  zclFramePayload,
                ZbProZclLocalPrimitiveParamsPrototype_t *const  indParams);


/**//**
 * \brief   Data type for entry point of ZCL Local Indication function.
 * \param[out]      indParams       Pointer to ZCL Local Indication Parameters Prototype.
 * \details
 *  ZCL Command Processor has to provide function of this type if this local node
 *  implements unsolicited indication of such a command.
 * \note
 *  If this command may be transferred to the higher layer only solicitedly within a
 *  particular distributed request processing, then the command processor shall not have
 *  this function for such command.
 */
typedef void  ZbProZclDispatcherIndicationCallback_t(
                ZbProZclLocalPrimitiveParamsPrototype_t *const  indParams);


/**//**
 * \brief   Structure for descriptor of Outgoing ZCL Command.
 */
struct _ZbProZclOutgoingCommandDescr_t
{
    /* 64-bit data. */

    ZbProZclCommandUid_t                    uid;                /*!< Unique identifier of this ZCL Command; or
                                                                    0xFFFFFFFFFFFFFFFF for the trailing record in the
                                                                    table. */
    /* 32-bit data. */

    ZbProZclDispatcherComposeReqCallback_t *composeReqCb;       /*!< Entry point to ZCL Local Request Command Composer
                                                                    function; or NULL if this Command has no ZCL
                                                                    Payload. */
    /* 8-bit data. */

    ZBPRO_ZCL_CommandId_t                   rspCommandId;       /*!< Command Id of the specific response incoming
                                                                    command (if exists) for this request outgoing
                                                                    command. Used for response-request commands matching
                                                                    on reception of response candidate frame. */

    Bool8_t                                 hasSpecResp;        /*!< TRUE if this request outgoing command has specific
                                                                    response incoming command; FALSE if only the Default
                                                                    Response may be issued on this request command.*/
};


/**//**
 * \brief   Structure for descriptor of Incoming ZCL Command.
 */
struct _ZbProZclIncomingCommandDescr_t
{
    /* 64-bit data. */

    ZbProZclCommandUid_t                            uid;                /*!< Unique identifier of this ZCL Command; or
                                                                            0xFFFFFFFFFFFFFFFF for the trailing record
                                                                            in the table. */
    /* Structured data, aligned at 32-bit. */

    union
    {
        struct
        {
            ZbProZclDispatcherParseConfCallback_t  *parseConfCb;        /*!< Entry point to ZCL Remote Response Command
                                                                            Parser function; or NULL if this command
                                                                            has no ZCL Payload. */

            void                                   *nullIndCb;          /*!< Dummy field for NULL as the entry point to
                                                                            ZCL Local Indication function for the case
                                                                            if this command may not be transferred to
                                                                            the higher layer unsolicitedly but only as
                                                                            Confirmation on a Local Request. */
        };

        struct
        {
            ZbProZclDispatcherParseIndCallback_t   *parseIndCb;         /*!< Entry point to ZCL Remote Request Command
                                                                            Parser function; or NULL if this command
                                                                            has no ZCL Payload. */

            ZbProZclDispatcherIndicationCallback_t *issueIndCb;         /*!< Entry point to ZCL Local Indication
                                                                            function; or NULL if this command may not be
                                                                            transferred to the higher layer
                                                                            unsolicitedly but only as Confirmation on
                                                                            former distributed Request. */

            bool                                    makeAutoDefResp;    /*!< TRUE if the higher-layer is just notified
                                                                            with the Local Indication on reception of
                                                                            this command but is not allowed to decide
                                                                            what to respond; in this case ZCL Dispatcher
                                                                            issues automatic Default Response on
                                                                            reception of this command. FALSE if the
                                                                            Local Response on the Local Indication of
                                                                            this command is constructed and commenced by
                                                                            the higher-layer; in this case ZCL
                                                                            Dispatcher will not issue automatic Default
                                                                            Response on reception of this command. */
        };
    };
};


/**//**
 * \brief   Constant that must be specified instead of ZCL Command Composer or Parser in
 *  the case when command has no ZCL Frame Payload.
 * \details
 *  ZCL Dispatcher composes and parses ZCL Frame Header itself and only ZCL Frame Payload
 *  must be composed/parsed by means of dedicated callback function provided for each
 *  command by its processor. If a command has no ZCL Frame Payload field at all (i.e.,
 *  the frame has neither obligatory nor even optional fields after its header) then
 *  corresponding composer/parser is omitted. In such case this constant is used instead
 *  of composer/parser function name in the command descriptor record.
 */
#define ZBPRO_ZCL_COMMAND_HAS_NO_PAYLOAD        NULL


/**//**
 * \brief   Constant that must be specified instead of ZCL Local Indication in the case
 *  when command may not be indicated unsolicitedly.
 * \details
 *  Each received command is checked by ZCL Dispatcher against the set of opened client
 *  transactions waiting for response. If a matched transaction is found, then received
 *  command is processed within it and parameters received in such command are reported to
 *  the higher layer (that originated the request) with local confirmation. Otherwise, if
 *  there is no matching transaction, ZCL Dispatcher tries to process the received command
 *  unsolicitedly. For this case the command processor must provide corresponding
 *  Indication function by means of which ZCL Dispatcher will indicate such command
 *  reception to the local higher layer. If a command may not be processed unsolicitedly
 *  (i.e., if there is no corresponding function for its direct indication), this constant
 *  is used instead of indication function name in the command descriptor record.
 */
#define ZBPRO_ZCL_COMMAND_HAS_NO_INDICATION     NULL


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   ZCL Services' Descriptors table.
 * \note
 *  Do not move the last line in the table. It contains the Default Service descriptor
 *  which, according to the RPC Dispatcher API, shall be the last record in the list.
 */
extern const RpcService_t  zbProZclServicesDescrTable[];


/**//**
 * \brief   Processes Commands from RPC Dispatcher with reference to the specified
 *  ZCL-based profile Transaction.
 * \param[in/out]   rpcTransaction      Pointer to an RPC Transaction; or NULL if the
 *  specified RPC Command is addressed to the whole Service.
 * \param[in]       rpcCommand          Numeric identifier of an RPC Command to be
 *  executed.
 * \return  Numeric identifier of an RPC Directive from the Service returned to RPC
 *  Dispatcher.
 */
RpcServiceDirective_t zbProZclDispatcherCommonServiceHandler(
                RpcTransaction_t   *const  rpcTransaction,
                const RpcServiceCommand_t  rpcCommand);


/**//**
 * \brief   Searches for the Outgoing Command Descriptor by Command Uid.
 * \param[in]   commandUid      Numeric 64-bit value of Command Uid.
 * \return  Pointer to valid Outgoing Command Descriptor. NULL or pointer to EOT record
 *  are never returned.
 * \details
 *  If a command with \p commandUid identifier is not described as one of implemented
 *  outgoing commands, this function halts execution for debugging. Public interface for
 *  Local Requests shall guarantee that there would be no requests of arbitrary (not
 *  implemented) commands, but only requests of implemented and properly described
 *  commands.
 */
const ZbProZclOutgoingCommandDescr_t* zbProZclDispatcherOutgoingCommandDescriptorByUid(
                const ZbProZclCommandUid_t  commandUid);


/**//**
 * \brief   Searches for the Incoming Command Descriptor by Command Uid.
 * \param[in]   commandUid      Numeric 64-bit value of Command Uid.
 * \return  Pointer to valid Incoming Command Descriptor; or NULL if such an incoming
 *  command is not supported.
 * \details
 *  If a command with \p commandUid identifier is not described as one of implemented
 *  incoming commands, this function returns NULL. ZCL Dispatcher shall provide special
 *  processing for this case.
 */
const ZbProZclIncomingCommandDescr_t* zbProZclDispatcherIncomingCommandDescriptorByUid(
                const ZbProZclCommandUid_t  commandUid);


/**//**
 * \brief   Checks if a response command matches the original request command.
 * \param[in]   reqTransaction      Pointer to the descriptor of the original request
 *  command ZCL Transaction.
 * \param[in]   rspFrameHeader      Pointer to the deserialized Frame Header object of the
 *  received response candidate command.
 * \param[in]   isDefaultResponse   TRUE if the received command is the Default Response.
 * \return  TRUE if response matches the request; FALSE otherwise.
 * \details
 *  This function checks if the received response candidate command specified with the
 *  \p rspFrameHeader matches the original request command specified with the
 *  \p reqTransaction.
 * \details
 *  Response is considered matched if all the listed conditions are met:
 *  - ZCL transaction sequence number (TSN) of response equals to the TSN of request,
 *  - cluster Id of response equals to the cluster Id of request,
 *  - cluster side of response is opposite to the cluster side of request,
 *  - frame domain (ZCL Standard or Manufacturer Specific) of response coincides with the
 *      frame domain of request,
 *  - for the case of Manufacturer Specific domain the Manufacturer Code of response
 *      equals to the Manufacturer Code of request, - response command is applicable to
 *      the request command.
 *
 *  The first three conditions are verified by RPC Dispatcher (as TSN and Service Uid
 *  matching), and they are guaranteed to be hold. While the rest three conditions are
 *  verified by this function.
 *
 *  As for the sixth condition, response command is considered matched to the request
 *  command in two cases:
 *  - response candidate command is one of response-type commands and it matches the
 *      request command directly. For example, Read Attributes Response matches the Read
 *      Attributes profile-wide command,
 *  - response candidate command is the Default Response command and the CommandId
 *      parameter of the Default Response frame coincides with the Command Id of the
 *      original request.
 *
 *  Notice that the second case (i.e., when a Default Response is received, and its
 *  CommandId parameter must be validated) is not checked by this function because the
 *  received frame is not parsed yet. This condition must be validated outside of this
 *  function.
 */
bool zbProZclDispatcherCheckResponseMatching(
                const ZbProZclTransaction_t *const  reqTransaction,
                const ZbProZclFrameHeader_t *const  rspFrameHeader,
                const bool                          isDefaultResponse);


#endif /* _BB_ZBPRO_ZCL_DISPATCHER_TABLES_H */

/* eof bbZbProZclDispatcherTables.h */