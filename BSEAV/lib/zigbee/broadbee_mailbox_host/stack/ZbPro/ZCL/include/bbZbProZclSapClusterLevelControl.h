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
*       ZCL Level Control cluster SAP interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZCL_SAP_CLUSTER_LEVEL_CONTROL_H
#define _BB_ZBPRO_ZCL_SAP_CLUSTER_LEVEL_CONTROL_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of attributes of Level Control ZCL cluster Server side.
 * \details
 *  These attributes are provided by Server side of the cluster.
 * \details
 *  Level Control ZCL cluster has no attributes provided by Client side.
 * \ingroup ZBPRO_ZCL_LevelControlAttr
 * \note
 *  This implementation of Level Control ZCL cluster doesn't provide Server side; and its
 *  Client side isn't able to access any of the optional attributes of the Server side,
 *  it's able to access only the following mandatory attributes of the Server side:
 *  - CurrentLevel.
 *
 *  Client is also able to receive reporting of this CurrentLevel attribute.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.10.2.3, 3.10.2.7, 3.10.3.2, table 3-52.
 */
typedef enum _ZBPRO_ZCL_SapLevelControlServerAttributeId_t
{
    ZBPRO_ZCL_LEVEL_CONTROL_ATTR_ID_CURRENT_LEVEL           = 0x0000,       /*!< CurrentLevel. */

    ZBPRO_ZCL_LEVEL_CONTROL_ATTR_ID_REMAINING_TIME          = 0x0001,       /*!< RemainingTime. */

    ZBPRO_ZCL_LEVEL_CONTROL_ATTR_ID_ONOFF_TRANSITION_TIME   = 0x0010,       /*!< OnOffTransitionTime. */

    ZBPRO_ZCL_LEVEL_CONTROL_ATTR_ID_ON_LEVEL                = 0x0011,       /*!< OnLevel. */

    ZBPRO_ZCL_LEVEL_CONTROL_ATTR_ID_ON_TRANSITION_TIME      = 0x0012,       /*!< OnTransitionTime. */

    ZBPRO_ZCL_LEVEL_CONTROL_ATTR_ID_OFF_TRANSITION_TIME     = 0x0013,       /*!< OffTransitionTime. */

    ZBPRO_ZCL_LEVEL_CONTROL_ATTR_ID_DEFAULT_MOVE_RATE       = 0x0014,       /*!< DefaultMoveRate. */

    ZBPRO_ZCL_LEVEL_CONTROL_ATTR_ID_MAX                     = 0xFFFF,       /*!< Introduced only to make the enumeration
                                                                                16-bit wide. */
} ZBPRO_ZCL_SapLevelControlServerAttributeId_t;


/**//**
 * \brief   Data types shared by attributes and command parameters of Level Control
 *  cluster.
 * \ingroup ZBPRO_ZCL_LevelControlAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.10.2.3, 3.10.2.4, table 3-52, figures
 *  3-39, 3-40, 3-41.
 */
typedef uint8_t   ZBPRO_ZCL_LevelControlParamLevel_t;               /*!< Shared data type for Level parameter, in units
                                                                        specific to particular device. */

/**//**
 * \brief   Data types shared by attributes and command parameters of Level Control
 *  cluster.
 * \ingroup ZBPRO_ZCL_LevelControlAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.10.2.3, 3.10.2.4, table 3-52, figures
 *  3-39, 3-40, 3-41.
 */
typedef uint16_t  ZBPRO_ZCL_LevelControlParamTransitionTime_t;      /*!< Shared data type for TransitionTime parameter,
                                                                        in 1/10ths of a second. */

/**//**
 * \brief   Data types shared by attributes and command parameters of Level Control
 *  cluster.
 * \ingroup ZBPRO_ZCL_LevelControlAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.10.2.3, 3.10.2.4, table 3-52, figures
 *  3-39, 3-40, 3-41.
 */
typedef uint8_t   ZBPRO_ZCL_LevelControlParamRate_t;                /*!< Shared data type for Rate parameter, in units
                                                                        (specific to particular device) per second. */
/**//**
 * \brief   Enumeration of values of the MoveMode and StepMode parameters.
 * \ingroup ZBPRO_ZCL_LevelControlAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.10.2.4.2.2, 3.10.2.4.3.1, tables 3-54,
 *  3-55.
 */
typedef enum _ZBPRO_ZCL_LevelControlParamDirection_t
{
    ZBPRO_ZCL_LEVEL_CONTROL_DIRECTION_UP    = 0x0,      /*!< Make move or step up. */

    ZBPRO_ZCL_LEVEL_CONTROL_DIRECTION_DOWN  = 0x1,      /*!< Make move or step down. */

} ZBPRO_ZCL_LevelControlParamDirection_t;

/*
 * Validate size of Direction data type.
 */
SYS_DbgAssertStatic(1 == sizeof(ZBPRO_ZCL_LevelControlParamDirection_t));


/**//**
 * \brief   Special values shared by attributes and command parameters of Level Control
 *  cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.10.2.3.4, 3.10.2.3.5, 3.10.2.3.6,
 *  3.10.2.3.7.
 */
#define ZBPRO_ZCL_LEVEL_CONTROL_ON_LEVEL_HAS_NO_EFFECT          0xFF        /*!< When assigned to OnLevel attribute, it
                                                                                makes it to have no effect. */

#define ZBPRO_ZCL_LEVEL_CONTROL_TRANSITION_TIME_HAS_NO_EFFECT   0xFFFF      /*!< When assigned to OnTransitionTime or
                                                                                OffTransitionTime attribute, it makes it
                                                                                (or them) to have no effect. */

#define ZBPRO_ZCL_LEVEL_CONTROL_TRANSITION_TIME_USE_DEFAULT     0xFFFF      /*!< When assigned to TransitionTime
                                                                                parameter of MoveToLevel command, it
                                                                                makes it to use the transition time
                                                                                specified with OnOffTransitionTime
                                                                                attribute. */

#define ZBPRO_ZCL_LEVEL_CONTROL_RATE_USE_DEFAULT                0xFF        /*!< When assigned to Rate parameter of Move
                                                                                command, it makes it to use the rate
                                                                                specified with DefaultMoveRate
                                                                                attribute. */

#define ZBPRO_ZCL_LEVEL_CONTROL_TRANSITION_TIME_FASTEST         0xFFFF      /*!< When assigned to TransitionTime
                                                                                parameter of Step command, it makes it
                                                                                to move as fast as it is able. */


/**//**
 * \brief   Data types of attributes of Level Control ZCL cluster.
 * \details
 *  All the listed attributes are transferred by value.
 * \ingroup ZBPRO_ZCL_LevelControlAttr
 * \note
 *  There is an error in Document 075123r05, subclause 3.10.2.3, table 3-52: attribute
 *  DefaultMoveRate has the type 'Unsigned 8-bit integer' indeed, while it is specified to
 *  have the type 'Unsigned 16-bit integer'. Refer its range, specified as 0x00-0xFE, and
 *  description given in 3.10.2.3.7: "The DefaultMoveRate attribute determines the
 *  movement rate, in units per second, when a Move command is received with a Rate
 *  parameter of 0xFF", - it means that it has the same type with the Rate parameter of
 *  Move command, see subclause 3.10.2.4.2.3, figure 3-40.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.10.2.3, table 3-52.
 */
typedef ZBPRO_ZCL_LevelControlParamLevel_t              ZBPRO_ZCL_LevelControlAttrCurrentLevel_t;
                                                                    /*!< CurrentLevel. */

/**//**
 * \brief   Data types of attributes of Level Control ZCL cluster.
 * \ingroup ZBPRO_ZCL_LevelControlAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.10.2.3, table 3-52.
 */
typedef ZBPRO_ZCL_LevelControlParamTransitionTime_t     ZBPRO_ZCL_LevelControlAttrRemainingTime_t;
                                                                    /*!< RemainingTime. */

/**//**
 * \brief   Data types of attributes of Level Control ZCL cluster.
 * \ingroup ZBPRO_ZCL_LevelControlAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.10.2.3, table 3-52.
 */
typedef ZBPRO_ZCL_LevelControlParamTransitionTime_t     ZBPRO_ZCL_LevelControlAttrOnOffTransitionTime_t;
                                                                    /*!< OnOffTransitionTime. */

/**//**
 * \brief   Data types of attributes of Level Control ZCL cluster.
 * \ingroup ZBPRO_ZCL_LevelControlAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.10.2.3, table 3-52.
 */
typedef ZBPRO_ZCL_LevelControlParamLevel_t              ZBPRO_ZCL_LevelControlAttrOnLevel_t;
                                                                    /*!< OnLevel. */

/**//**
 * \brief   Data types of attributes of Level Control ZCL cluster.
 * \ingroup ZBPRO_ZCL_LevelControlAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.10.2.3, table 3-52.
 */
typedef ZBPRO_ZCL_LevelControlParamTransitionTime_t     ZBPRO_ZCL_LevelControlAttrOnTransitionTime_t;
                                                                    /*!< OnTransitionTime. */

/**//**
 * \brief   Data types of attributes of Level Control ZCL cluster.
 * \ingroup ZBPRO_ZCL_LevelControlAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.10.2.3, table 3-52.
 */
typedef ZBPRO_ZCL_LevelControlParamTransitionTime_t     ZBPRO_ZCL_LevelControlAttrOffTransitionTime_t;
                                                                    /*!< OffTransitionTime. */

/**//**
 * \brief   Data types of attributes of Level Control ZCL cluster.
 * \ingroup ZBPRO_ZCL_LevelControlAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 3.10.2.3, table 3-52.
 */
typedef ZBPRO_ZCL_LevelControlParamRate_t               ZBPRO_ZCL_LevelControlAttrDefaultMoveRate_t;
                                                                    /*!< DefaultMoveRate. */


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Move To Level or 'Move
 *  To Level (with On/Off)' command.
 * \ingroup ZBPRO_ZCL_MoveToLevelReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.10.2.4.1, 3.10.2.4.5, figure 3-39.
 */
typedef struct _ZBPRO_ZCL_LevelControlCmdMoveToLevelReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t       zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */

    Bool8_t                                      withOnOff;             /*!< When assigned to TRUE, command 'Move To
                                                                            Level (with On/Off)' with Command Id 0x04 is
                                                                            issued; otherwise command 'Move To Level'
                                                                            with Command Id 0x00 is issued. */

    ZBPRO_ZCL_LevelControlParamLevel_t           level;                 /*!< Level, in units specific to particular
                                                                            device. */

    ZBPRO_ZCL_LevelControlParamTransitionTime_t  transitionTime;        /*!< Transition time, in 1/10ths of a second.
                                                                            When assigned with 0xFFFF this parameter has
                                                                            no effect; the OnOffTransitionTime attribute
                                                                            is used instead of it. */
} ZBPRO_ZCL_LevelControlCmdMoveToLevelReqParams_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Move or 'Move (with
 *  On/Off)' command.
 * \ingroup ZBPRO_ZCL_MoveReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.10.2.4.2, 3.10.2.4.5, figure 3-40.
 */
typedef struct _ZBPRO_ZCL_LevelControlCmdMoveReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* Custom parameters. */

    Bool8_t                                 withOnOff;              /*!< When assigned to TRUE, command 'Move (with
                                                                        On/Off)' with Command Id 0x05 is issued;
                                                                        otherwise command 'Move' with Command Id 0x01 is
                                                                        issued. */

    ZBPRO_ZCL_LevelControlParamDirection_t  moveMode;               /*!< Move mode, either Up or Down. */

    ZBPRO_ZCL_LevelControlParamRate_t       rate;                   /*!< Rate, in units per second. When assigned with
                                                                        0xFF this parameter has no effect;
                                                                        DefaultMoveRate attribute is used instead of
                                                                        it. */
} ZBPRO_ZCL_LevelControlCmdMoveReqParams_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Step or 'Step (with
 *  On/Off)' command.
 * \ingroup ZBPRO_ZCL_StepReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.10.2.4.3, 3.10.2.4.5, figure 3-41.
 */
typedef struct _ZBPRO_ZCL_LevelControlCmdStepReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t       zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */

    Bool8_t                                      withOnOff;             /*!< When assigned to TRUE, command 'Step (with
                                                                            On/Off)' with Command Id 0x06 is issued;
                                                                            otherwise command 'Step' with Command Id
                                                                            0x02 is issued. */

    ZBPRO_ZCL_LevelControlParamDirection_t       stepMode;              /*!< Step mode, either Up or Down. */

    ZBPRO_ZCL_LevelControlParamLevel_t           stepSize;              /*!< Step size, in units specific to particular
                                                                            device. */

    ZBPRO_ZCL_LevelControlParamTransitionTime_t  transitionTime;        /*!< Transition time, in 1/10ths of a second.
                                                                            When assigned with 0xFFFF this parameter has
                                                                            no effect; device should move as fast as it
                                                                            is able. */
} ZBPRO_ZCL_LevelControlCmdStepReqParams_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Stop or 'Stop (with
 *  On/Off)' command.
 * \ingroup ZBPRO_ZCL_StopReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 3.10.2.4.4, 3.10.2.4.5.
 */
typedef struct _ZBPRO_ZCL_LevelControlCmdStopReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* Custom parameters. */

    Bool8_t                                 withOnOff;              /*!< When assigned to TRUE, command 'Stop (with
                                                                        On/Off)' with Command Id 0x07 is issued;
                                                                        otherwise command 'Stop' with Command Id 0x03 is
                                                                        issued. */
} ZBPRO_ZCL_LevelControlCmdStopReqParams_t;


/*
 * Validate structures of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_LevelControlCmdMoveToLevelReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_LevelControlCmdMoveReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_LevelControlCmdStepReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_LevelControlCmdStopReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue
 *  these cluster commands.
 * \details
 *  This structure takes its origin from ZCL Default Response profile-wide command.
 * \ingroup ZBPRO_ZCL_MoveToLevelConf
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.12, figure 2-25.
 */
typedef struct _ZBPRO_ZCL_LevelControlCmdConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* No custom parameters. */

} ZBPRO_ZCL_LevelControlCmdConfParams_t;

/*
 * Validate structure of ZCL Local Confirm Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_LevelControlCmdConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Move To Level or 'Move
 *  To Level (with On/Off)' command.
 * \ingroup ZBPRO_ZCL_MoveToLevelReq
 */
typedef struct _ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t  ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t;


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Move or 'Move (with
 *  On/Off)' command.
 * \ingroup ZBPRO_ZCL_MoveReq
 */
typedef struct _ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t  ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t;


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Step or 'Step (with
 *  On/Off)' command.
 * \ingroup ZBPRO_ZCL_StepReq
 */
typedef struct _ZBPRO_ZCL_LevelControlCmdStepReqDescr_t  ZBPRO_ZCL_LevelControlCmdStepReqDescr_t;


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Stop or 'Stop (with
 *  On/Off)' command.
 * \ingroup ZBPRO_ZCL_StopReq
 */
typedef struct _ZBPRO_ZCL_LevelControlCmdStopReqDescr_t  ZBPRO_ZCL_LevelControlCmdStopReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Move To Level or
 *  'Move To Level (with On/Off)' command.
 * \ingroup ZBPRO_ZCL_MoveToLevelConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_LevelControlCmdMoveToLevelConfCallback_t(
                ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t *const  reqDescr,
                ZBPRO_ZCL_LevelControlCmdConfParams_t          *const  confParams);


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Move or 'Move (with
 *  On/Off)' command.
 * \ingroup ZBPRO_ZCL_MoveConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_LevelControlCmdMoveConfCallback_t(
                ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t *const  reqDescr,
                ZBPRO_ZCL_LevelControlCmdConfParams_t   *const  confParams);


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Step or 'Step (with
 *  On/Off)' command.
 * \ingroup ZBPRO_ZCL_StepConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_LevelControlCmdStepConfCallback_t(
                ZBPRO_ZCL_LevelControlCmdStepReqDescr_t *const  reqDescr,
                ZBPRO_ZCL_LevelControlCmdConfParams_t   *const  confParams);


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Stop or 'Stop (with
 *  On/Off)' command.
 * \ingroup ZBPRO_ZCL_StopConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_LevelControlCmdStopConfCallback_t(
                ZBPRO_ZCL_LevelControlCmdStopReqDescr_t *const  reqDescr,
                ZBPRO_ZCL_LevelControlCmdConfParams_t   *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Move To Level or 'Move
 *  To Level (with On/Off)' command.
 * \ingroup ZBPRO_ZCL_MoveToLevelReq
 */
struct _ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_LevelControlCmdMoveToLevelConfCallback_t *callback;       /*!< ZCL Confirmation callback handler entry
                                                                            point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                service;        /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_LevelControlCmdMoveToLevelReqParams_t     params;         /*!< ZCL Request parameters structure. */
};


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Move or 'Move (with
 *  On/Off)' command.
 * \ingroup ZBPRO_ZCL_MoveReq
 */
struct _ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_LevelControlCmdMoveConfCallback_t *callback;      /*!< ZCL Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t         service;       /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_LevelControlCmdMoveReqParams_t     params;        /*!< ZCL Request parameters structure. */
};


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Step or 'Step (with
 *  On/Off)' command.
 * \ingroup ZBPRO_ZCL_StepReq
 */
struct _ZBPRO_ZCL_LevelControlCmdStepReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_LevelControlCmdStepConfCallback_t *callback;      /*!< ZCL Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t         service;       /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_LevelControlCmdStepReqParams_t     params;        /*!< ZCL Request parameters structure. */
};


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Stop or 'Stop (with
 *  On/Off)' command.
 * \ingroup ZBPRO_ZCL_StopReq
 */
struct _ZBPRO_ZCL_LevelControlCmdStopReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_LevelControlCmdStopConfCallback_t *callback;      /*!< ZCL Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t         service;       /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_LevelControlCmdStopReqParams_t     params;        /*!< ZCL Request parameters structure. */
};


/*
 * Validate structures of ZCL Local Request Descriptors.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_LevelControlCmdStepReqDescr_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_LevelControlCmdStopReqDescr_t);


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief  Functions accept ZCL Local Requests to issue commands of this cluster.
 * \details
 *  The caller shall specify the following obligatory parameters of request: callback,
 *  remoteApsAddress.addrMode, respWaitTimeout, localEndpoint, disableDefaultResp.
 * \details
 *  For the case when remote (destination) node is bound to this (source) node, one may
 *  set the Remote Addressing Mode to NOT_PRESENT and specify only the Local Endpoint and
 *  Cluster on it. APS layer will then accept Remote node Address (extended or group) and
 *  Endpoint corresponding to the specified Local Endpoint according to the Binding Table.
 *  Otherwise, for direct addressing mode, the caller shall also specify the following
 *  parameters: remoteApsAddress, remoteEndpoint. For the case of Local Response to Local
 *  Indication it's enough just to copy all the obligatory parameters of Indication to
 *  Response byte-to-byte to obtain correct destination addressing values of Response.
 * \details
 *  Following parameters are ignored and reassigned by command handlers: localApsAddress,
 *  clusterId, manufCode, commandId, transSeqNum, overallStatus, direction,
 *  clusterSpecific, manufSpecific, useSpecifiedTsn, nonUnicastRequest, useDefaultResponse.
 */

/**//**
 * \brief   Accepts ZCL Local Request to issue Move To Level or 'Move To Level (with
 *  On/Off)' command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_LevelControlCmdMoveToLevelReq(
                ZBPRO_ZCL_LevelControlCmdMoveToLevelReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Move or 'Move (with On/Off)' command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_LevelControlCmdMoveReq(
                ZBPRO_ZCL_LevelControlCmdMoveReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Step or 'Step (with On/Off)' command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_LevelControlCmdStepReq(
                ZBPRO_ZCL_LevelControlCmdStepReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Stop or 'Stop (with On/Off)' command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_LevelControlCmdStopReq(
                ZBPRO_ZCL_LevelControlCmdStopReqDescr_t *const  reqDescr);


#endif /* _BB_ZBPRO_ZCL_SAP_CLUSTER_LEVEL_CONTROL_H */

/* eof bbZbProZclSapClusterLevelControl.h */