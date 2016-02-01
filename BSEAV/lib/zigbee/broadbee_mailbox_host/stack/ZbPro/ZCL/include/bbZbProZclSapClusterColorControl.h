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
*   ZCL Color Control cluster SAP interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_SAP_CLUSTER_COLOR_CONTROL_H
#define _BB_ZBPRO_ZCL_SAP_CLUSTER_COLOR_CONTROL_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of attributes of Color Control ZCL cluster Server side.
 * \details
 *  These attributes are provided by Server side of the cluster.
 * \details
 *  Color Control ZCL cluster has no attributes provided by Client side.
 * \note
 *  This implementation of Color Control ZCL cluster doesn't provide Server side.
 * \note
 * There are only mandatory attributes.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 5.2.2.2, Table 5-2, Table 5-3.
 */
typedef enum _ZBPRO_ZCL_SapColorControlServerAttributeId_t
{
    ZBPRO_ZCL_SAP_COLOR_CONTROL_ATTR_ID_CURRENT_X                       = 0x0003,  /*!< CurrentX. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_ATTR_ID_CURRENT_Y                       = 0x0004,  /*!< CurrentY. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_ATTR_ID_ENHANCED_CURRENT_HUE            = 0x4000,  /*!< EnhancedCurrentHue. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_ATTR_ID_ENHANCED_COLOR_MODE             = 0x4001,  /*!< EnhancedColor Mode. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_ATTR_ID_COLOR_LOOP_ACTIVE               = 0x4002,  /*!< ColorLoopActive. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_ATTR_ID_COLOR_LOOP_DIRECTION            = 0x4003,  /*!< ColorLoopDirection. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_ATTR_ID_COLOR_LOOP_TIME                 = 0x4004,  /*!< ColorLoopTime. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_ATTR_ID_COLOR_LOOP_START_ENHANCED_HUE   = 0x4005,  /*!< ColorLoopStartEnhancedHue. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_ATTR_ID_COLOR_LOOP_STORED_ENHANCED_HUE  = 0x4006,  /*!< ColorLoopStoredEnhancedHue. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_ATTR_ID_COLOR_CAPABILITIES              = 0x400A,  /*!< ColorCapabilities. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_ATTR_ID_COLOR_TEMP_PHYSICAL_MIN         = 0x400B,  /*!< ColorTempPhysicalMin. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_ATTR_ID_COLOR_TEMP_PHYSICAL_MAX         = 0x400C,  /*!< ColorTempPhysicalMax. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_ATTR_ID_MAX                     = 0xFFFF,       /*!< Introduced only to make the
                                                                                     enumeration 16-bit wide. */
} ZBPRO_ZCL_SapColorControlServerAttributeId_t;


/**//**
 * \brief    Data types shared by attributes of Color Control cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 5.2.2.2, Table 5-2, Table 5-3.
 */
typedef  uint16_t  ZBPRO_ZCL_SapColorControlAttrCurrentX_t;                     /*!< Shared data type for
                                                                                     CurrentX attribute. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlAttrCurrentY_t;                     /*!< Shared data type for
                                                                                     CurrentY attribute. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlAttrEnhancedCurrent_t;              /*!< Shared data type for
                                                                                     EnhancedCurrent Hue attribute. */

typedef enum _ZBPRO_ZCL_SapColorControlAttrEnhancedColorMode
{
    ZBPRO_ZCL_COLOR_CONTROL_ENHANCED_COLOR_MODE_HUE_AND_SATURATION           =  0x00,

    ZBPRO_ZCL_COLOR_CONTROL_ENHANCED_COLOR_MODE_X_AND_Y                      =  0x01,

    ZBPRO_ZCL_COLOR_CONTROL_ENHANCED_COLOR_MODE_COLOR_TEMPERATURE            =  0x02,

    ZBPRO_ZCL_COLOR_CONTROL_ENHANCED_COLOR_MODE_ENHANCED_HUE_AND_SATURATION  =  0x03,

} ZBPRO_ZCL_SapColorControlAttrEnhancedColorMode;                               /*!< Shared data type for
                                                                                     EnhancedColorMode attribute. */

typedef  uint8_t  ZBPRO_ZCL_SapColorControlAttrColorLoopActive_t;               /*!< Shared data type for
                                                                                     ColorLoopActive attribute. */

typedef  uint8_t  ZBPRO_ZCL_SapColorControlAttrColorLoopDirection_t;            /*!< Shared data type for
                                                                                     ColorLoopDirection attribute. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlAttrColorLoopTime_t;                /*!< Shared data type for
                                                                                     ColorLoopTime attribute. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlAttrColorLoopStartEnhancedHue_t;    /*!< Shared data type for
                                                                                     ColorLoopStartEnhancedHue
                                                                                     attribute. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlAttrColorLoopStoredEnhancedHue_t;   /*!< Shared data type for
                                                                                     ColorLoopStoredEnhancedHue
                                                                                     attribute. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlAttrColorCapabilities_t;            /*!< Shared data type for
                                                                                     ColorCapabilities attribute. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlAttrColorTempPhysicalMin_t;         /*!< Shared data type for
                                                                                     ColorTempPhysicalMin attribute. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlAttrColorTempPhysicalMax_t;         /*!< Shared data type for
                                                                                     ColorTempPhysicalMax attribute. */



/**//**
 * \brief    Data types shared by command parameters of Color Control ZCL cluster.
 */
typedef  uint16_t  ZBPRO_ZCL_SapColorControlParamTransitionTime_t;              /*!< Shared data type for
                                                                                     Transition Time parameter. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlParamColor_t;                       /*!< Shared data type for
                                                                                     ColorX and ColorY parameters. */

typedef  int16_t  ZBPRO_ZCL_SapColorControlParamRate_t;                         /*!< Shared data type for
                                                                                     RateX and RateY parameters. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlParamEnhanceHue_t;                  /*!< Shared data type for
                                                                                     Enhanced Hue parameters */

typedef enum _ZBPRO_ZCL_SapColorControlParamHueDirection_t
{
    ZBPRO_ZCL_SAP_COLOR_CONTROL_PARAM_HUE_DIRECTION_SHORTEST_DISTANCE = 0x00,   /*!< Shortest distance. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_PARAM_HUE_DIRECTION_LONGEST_DISTANCE  = 0x01,   /*!< Longest distance. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_PARAM_HUE_DIRECTION_UP                = 0x02,   /*!< Up. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_PARAM_HUE_DIRECTION_DOWN              = 0x03    /*!< Down. */

} ZBPRO_ZCL_SapColorControlParamHueDirection_t;                                 /*!< Shared data type for
                                                                                     Direction parameter in the
                                                                                     Move to Hue and
                                                                                     Encanced Move to Hue  commands. */

typedef enum _ZBPRO_ZCL_SapColorControlParamHueMoveMode_t
{
    ZBPRO_ZCL_SAP_COLOR_CONTROL_PARAM_HUE_MOVE_MODE_STOP  =  0x00,            /*!< Stop. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_PARAM_HUE_MOVE_MODE_UP    =  0x01,            /*!< Up. */

    /* 0x02 is Reserved value. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_PARAM_HUE_MOVE_MODE_DOWN  =  0x03             /*!< Down. */

} ZBPRO_ZCL_SapColorControlParamHueMoveMode_t;                                  /*!< Shared data type for
                                                                                     Move Mode parameter in the
                                                                                     Encanced Move Hue and
                                                                                     Move Color Temperature commands. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlParamHueRate_t;                     /*!< Shared data type for
                                                                                     Rate parameter in the
                                                                                     Enhanced Move Hue command. */


typedef enum _ZBPRO_ZCL_SapColorControlParamHueStepMode_t
{
    /* 0x00 value is reserved. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_PARAM_HUE_STEP_MODE_UP    =  0x00,      /*!< Up. */

    /* 0x02 value is reserved. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_PARAM_HUE_STEP_MODE_DOWN  =  0x02       /*!< Down. */

} ZBPRO_ZCL_SapColorControlParamHueStepMode_t;                                  /*!< Shared data type for
                                                                                     Step Mode parameter in the
                                                                                     Enhanced Step Hue command. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlParamStepSize_t;                    /*!< Shared data type for
                                                                                     Step Size parameter in the
                                                                                     Enhanced Step Hue and
                                                                                     Step Color Temperature commands. */

typedef  uint8_t  ZBPRO_ZCL_SapColorControlParamSaturation_t;                   /*!< Shared data type for
                                                                                     Saturation parameter in the
                                                                                     Enhanced Move to Hue and Saturation command. */

typedef  BitField8_t  ZBPRO_ZCL_SapColorControlParamUpdateFlags_t;              /*!< Shared data type for
                                                                                     Update Flags parameter in the
                                                                                     Color Loop Set command. */

typedef enum _ZBPRO_ZCL_SapColorControlParamAction_t
{
    ZBPRO_ZCL_SAP_COLOR_CONTROL_PARAM_ACTION_DEACTIVATE                             = 0x00,

    ZBPRO_ZCL_SAP_COLOR_CONTROL_PARAM_ACTION_ACTIVATE_COLOR_LOOP_START_ENHANCED_HUE = 0x01,

    ZBPRO_ZCL_SAP_COLOR_CONTROL_PARAM_ACTION_ACTIVATE_ENHANCED_CURRENT_HUE          = 0x02

} ZBPRO_ZCL_SapColorControlParamAction_t;                                       /*!< Shared data type for
                                                                                     Action parameter in the
                                                                                     Color Loop Set command. */

typedef enum _ZBPRO_ZCL_SapColorControlParamColorLoopDirection_t
{
    ZBPRO_ZCL_SAP_COLOR_CONTROL_PARAM_COLOR_LOOP_DIRECTION_DECREMENT = 0x00,    /*!< Decrement the hue in the color loop. */

    ZBPRO_ZCL_SAP_COLOR_CONTROL_PARAM_COLOR_LOOP_DIRECTION_INCREMENT = 0x01     /*!< Increment the hue in the color loop. */

} ZBPRO_ZCL_SapColorControlParamColorLoopDirection_t;                           /*!< Shared data type for
                                                                                     Direction parameter in the
                                                                                     Color Loop Set command. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlParamColorLoopTime_t;               /*!< Shared data type for
                                                                                     Time parameter in the
                                                                                     Color Loop Set command. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlParamColorLoopStartHue_t;           /*!< Shared data type for
                                                                                     Start Hue parameter in the
                                                                                     Color Loop Set command. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlParamMoveColorRate_t;               /*!< Shared data type for
                                                                                     Rate parameter in the
                                                                                     Move Color Temperature command. */

typedef  uint16_t  ZBPRO_ZCL_SapColorControlParamColorTemperature_t;            /*!< Shared data type for
                                                                                     Color Temperature parameter in the
                                                                                     Move Color Temperature and
                                                                                     Step Color Temperature commands. */

typedef  int16_t  ZBPRO_ZCL_SapColorControlParamColorStep_t;                    /*!< Shared data type for Step parameter
                                                                                     in the Step Color Command. */

/*-------------------------------- Move to Color Cmd ------------------------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Move to Color command
 * specific to Color Control ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.10, figure 5-9.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdMoveToColorReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */

    ZBPRO_ZCL_SapColorControlParamColor_t               colorX;                 /*!< ColorX. */

    ZBPRO_ZCL_SapColorControlParamColor_t               colorY;                 /*!< ColorY. */

    ZBPRO_ZCL_SapColorControlParamTransitionTime_t      transitionTime;         /*!< Transition Time. */

} ZBPRO_ZCL_ColorControlCmdMoveToColorReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdMoveToColorReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Move to Color command
 * specific to Color Control ZCL cluster.
 * \note
 * This command have no response from server. So, confirmation parameters structure has no custom fields.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdMoveToColorConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_ColorControlCmdMoveToColorConfParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdMoveToColorConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Move to Color command
 * specific to Color Control ZCL cluster.
 */
typedef struct  _ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t  ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Move to Color command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the empty confirmation parameters because responses will be
 poroceed in the Indication, not in the callback.
 */
typedef void ZBPRO_ZCL_ColorControlCmdMoveToColorConfCallback_t(
    ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t        *const  reqDescr,
    ZBPRO_ZCL_ColorControlCmdMoveToColorConfParams_t      *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Move to Color command
 * specific to Color Control ZCL cluster.
 */
struct _ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ColorControlCmdMoveToColorConfCallback_t   *callback;   /*!< ZCL Confirmation callback handler entry
                                                                            point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                  service;    /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_ColorControlCmdMoveToColorReqParams_t       params;     /*!< ZCL Request parameters structure. */
};


/*-------------------------------- Move Color Cmd ------------------------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Move Color command
 * specific to Color Control ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.11, figure 5-10.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdMoveColorReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */

    ZBPRO_ZCL_SapColorControlParamRate_t     rateX;                  /*!< RateX. */

    ZBPRO_ZCL_SapColorControlParamRate_t     rateY;                  /*!< RateY. */

} ZBPRO_ZCL_ColorControlCmdMoveColorReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdMoveColorReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Move Color command
 * specific to Color Control ZCL cluster.
 * \note
 * This command have no response from server. So, confirmation parameters structure has no custom fields.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdMoveColorConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                             interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_ColorControlCmdMoveColorConfParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdMoveColorConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Move Color command
 * specific to Color Control ZCL cluster.
 */
typedef struct  _ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t  ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Move Color command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the empty confirmation parameters because responses will be
 poroceed in the Indication, not in the callback.
 */
typedef void ZBPRO_ZCL_ColorControlCmdMoveColorConfCallback_t(
    ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t        *const  reqDescr,
    ZBPRO_ZCL_ColorControlCmdMoveColorConfParams_t      *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Move Color command
 * specific to Color Control ZCL cluster.
 */
struct _ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ColorControlCmdMoveColorConfCallback_t   *callback;   /*!< ZCL Confirmation callback handler entry
                                                                            point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                service;    /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_ColorControlCmdMoveColorReqParams_t       params;     /*!< ZCL Request parameters structure. */
};


/*-------------------------------- Step Color Cmd ------------------------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Step Color command
 * specific to Color Control ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.12, figure 5-11.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdStepColorReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */

    ZBPRO_ZCL_SapColorControlParamColorStep_t           stepX;                  /*!< StepX. */

    ZBPRO_ZCL_SapColorControlParamColorStep_t           stepY;                  /*!< StepY. */

    ZBPRO_ZCL_SapColorControlParamTransitionTime_t      transitionTime;         /*!< TransitionTime. */

} ZBPRO_ZCL_ColorControlCmdStepColorReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdStepColorReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Step Color command
 * specific to Color Control ZCL cluster.
 * \note
 * This command have no response from server. So, confirmation parameters structure has no custom fields.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdStepColorConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_ColorControlCmdStepColorConfParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdStepColorConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Step Color command
 * specific to Color Control ZCL cluster.
 */
typedef struct  _ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t  ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Step Color command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the empty confirmation parameters because responses will be
 poroceed in the Indication, not in the callback.
 */
typedef void ZBPRO_ZCL_ColorControlCmdStepColorConfCallback_t(
    ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t        *const  reqDescr,
    ZBPRO_ZCL_ColorControlCmdStepColorConfParams_t      *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Step Color command
 * specific to Color Control ZCL cluster.
 */
struct _ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ColorControlCmdStepColorConfCallback_t   *callback;   /*!< ZCL Confirmation callback handler entry
                                                                            point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                service;    /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_ColorControlCmdStepColorReqParams_t       params;     /*!< ZCL Request parameters structure. */
};


/*-------------------------------- Enhanced Move to Hue Cmd ------------------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Enhanced Move to Hue command
 * specific to Color Control ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.14, figure 5-13.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */

    ZBPRO_ZCL_SapColorControlParamEnhanceHue_t         enhancedHue;            /*!< Enhanced Hue. */

    ZBPRO_ZCL_SapColorControlParamTransitionTime_t     transitionTime;         /*!< Transition Time. */

    /* 8-bit data. */

    ZBPRO_ZCL_SapColorControlParamHueDirection_t       direction;              /*!< Direction. */

} ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Enhanced Move to Hue command
 * specific to Color Control ZCL cluster.
 * \note
 * This command have no response from server. So, confirmation parameters structure has no custom fields.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueConfParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Enhanced Move to Hue command
 * specific to Color Control ZCL cluster.
 */
typedef struct  _ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Enhanced Move to Hue command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the empty confirmation parameters because responses will be
 poroceed in the Indication, not in the callback.
 */
typedef void ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueConfCallback_t(
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t        *const  reqDescr,
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueConfParams_t      *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Enhanced Move to Hue command
 * specific to Color Control ZCL cluster.
 */
struct _ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueConfCallback_t   *callback;   /*!< ZCL Confirmation callback handler entry
                                                                            point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                        service;    /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqParams_t       params;     /*!< ZCL Request parameters structure. */
};


/*-------------------------------- Enhanced Move Hue Cmd ------------------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Enhanced Move Hue command
 * specific to Color Control ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.15, figure 5-14.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */
    ZBPRO_ZCL_SapColorControlParamHueRate_t             rate;        /*!< Rate. */

    /* 8-bit data. */
    ZBPRO_ZCL_SapColorControlParamHueMoveMode_t         moveMode;    /*!< Move Mode. */


} ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Enhanced Move Hue command
 * specific to Color Control ZCL cluster.
 * \note
 * This command have no response from server. So, confirmation parameters structure has no custom fields.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueConfParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Enhanced Move Hue command
 * specific to Color Control ZCL cluster.
 */
typedef struct  _ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t  ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Enhanced Move Hue command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the empty confirmation parameters because responses will be
 poroceed in the Indication, not in the callback.
 */
typedef void ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueConfCallback_t(
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t        *const  reqDescr,
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueConfParams_t      *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Enhanced Move Hue command
 * specific to Color Control ZCL cluster.
 */
struct _ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueConfCallback_t   *callback;   /*!< ZCL Confirmation callback handler entry
                                                                               point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                      service;    /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqParams_t       params;     /*!< ZCL Request parameters structure. */
};


/*-------------------------------- Enhanced Step Hue Cmd ------------------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Enhanced Step Hue command
 * specific to Color Control ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.16, figure 5-15.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */
    ZBPRO_ZCL_SapColorControlParamStepSize_t         stepSize;        /*!< Step size. */

    ZBPRO_ZCL_SapColorControlParamTransitionTime_t   transitionTime;  /*!< Transition time. */

    /* 8-bit data. */
    ZBPRO_ZCL_SapColorControlParamHueStepMode_t      stepMode;        /*!< Step Mode. */


} ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Enhanced Step Hue command
 * specific to Color Control ZCL cluster.
 * \note
 * This command have no response from server. So, confirmation parameters structure has no custom fields.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdEnhancedStepHueConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_ColorControlCmdEnhancedStepHueConfParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdEnhancedStepHueConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Enhanced Step Hue command
 * specific to Color Control ZCL cluster.
 */
typedef struct  _ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t  ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Enhanced Step Hue command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the empty confirmation parameters because responses will be
 poroceed in the Indication, not in the callback.
 */
typedef void ZBPRO_ZCL_ColorControlCmdEnhancedStepHueConfCallback_t(
    ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t        *const  reqDescr,
    ZBPRO_ZCL_ColorControlCmdEnhancedStepHueConfParams_t      *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Enhanced Step Hue command
 * specific to Color Control ZCL cluster.
 */
struct _ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ColorControlCmdEnhancedStepHueConfCallback_t   *callback;   /*!< ZCL Confirmation callback handler entry
                                                                               point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                      service;    /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqParams_t       params;     /*!< ZCL Request parameters structure. */
};



/*-------------------------------- Enhanced Move to Hue and Saturation Cmd -----------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Enhanced Move to Hue and Saturation command
 * specific to Color Control ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.17, figure 5-16.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */

    ZBPRO_ZCL_SapColorControlParamEnhanceHue_t       enhancedHue;            /*!< Enhanced Hue. */

    ZBPRO_ZCL_SapColorControlParamTransitionTime_t   transitionTime;         /*!< Transition Time. */

    /* 8-bit data. */

    ZBPRO_ZCL_SapColorControlParamSaturation_t       saturation;              /*!< Saturation. */

} ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Enhanced Move to Hue and Saturation command
 * specific to Color Control ZCL cluster.
 * \note
 * This command have no response from server. So, confirmation parameters structure has no custom fields.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationConfParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Enhanced Move to Hue and Saturation command
 * specific to Color Control ZCL cluster.
 */
typedef struct  _ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Enhanced Move to Hue and Saturation command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the empty confirmation parameters because responses will be
 poroceed in the Indication, not in the callback.
 */
typedef void ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationConfCallback_t(
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t        *const  reqDescr,
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationConfParams_t      *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Enhanced Move to Hue and Saturation command
 * specific to Color Control ZCL cluster.
 */
struct _ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationConfCallback_t   *callback;   /*!< ZCL Confirmation callback
                                                                                              handler entry point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                                     service;    /*!< ZCL Request Descriptor
                                                                                              service field. */

    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqParams_t       params;     /*!< ZCL Request parameters
                                                                                              structure. */
};


/*-------------------------------- Color Loop Set Cmd -----------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Color Loop Set command
 * specific to Color Control ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.18, figure 5-17.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdColorLoopSetReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */

    ZBPRO_ZCL_SapColorControlParamColorLoopTime_t       time;            /*!< Time. */

    ZBPRO_ZCL_SapColorControlParamColorLoopStartHue_t   startHue;        /*!< Start Hue. */

    /* 8-bit data. */

    ZBPRO_ZCL_SapColorControlParamUpdateFlags_t         updateFlags;     /*!< Update Flags. */

    ZBPRO_ZCL_SapColorControlParamAction_t              action;          /*!< Action. */

    ZBPRO_ZCL_SapColorControlParamColorLoopDirection_t  direction;       /*!< Direction. */

} ZBPRO_ZCL_ColorControlCmdColorLoopSetReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdColorLoopSetReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Color Loop Set command
 * specific to Color Control ZCL cluster.
 * \note
 * This command have no response from server. So, confirmation parameters structure has no custom fields.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdColorLoopSetConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_ColorControlCmdColorLoopSetConfParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdColorLoopSetConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Color Loop Set command
 * specific to Color Control ZCL cluster.
 */
typedef struct  _ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t  ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Color Loop Set command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the empty confirmation parameters because responses will be
 poroceed in the Indication, not in the callback.
 */
typedef void ZBPRO_ZCL_ColorControlCmdColorLoopSetConfCallback_t(
    ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t        *const  reqDescr,
    ZBPRO_ZCL_ColorControlCmdColorLoopSetConfParams_t      *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Color Loop Set command
 * specific to Color Control ZCL cluster.
 */
struct _ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ColorControlCmdColorLoopSetConfCallback_t   *callback;   /*!< ZCL Confirmation callback handler entry
                                                                            point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                   service;    /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_ColorControlCmdColorLoopSetReqParams_t       params;     /*!< ZCL Request parameters structure. */
};


/*-------------------------------- Stop Move Step Cmd -----------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Stop Move Step command
 * specific to Color Control ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.19
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdStopMoveStepReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* This command has no custom parameters. */

} ZBPRO_ZCL_ColorControlCmdStopMoveStepReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdStopMoveStepReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Stop Move Step command
 * specific to Color Control ZCL cluster.
 * \note
 * This command have no response from server. So, confirmation parameters structure has no custom fields.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdStopMoveStepConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_ColorControlCmdStopMoveStepConfParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdStopMoveStepConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Stop Move Step command
 * specific to Color Control ZCL cluster.
 */
typedef struct  _ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t  ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Stop Move Step command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the empty confirmation parameters because responses will be
 poroceed in the Indication, not in the callback.
 */
typedef void ZBPRO_ZCL_ColorControlCmdStopMoveStepConfCallback_t(
    ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t        *const  reqDescr,
    ZBPRO_ZCL_ColorControlCmdStopMoveStepConfParams_t      *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Stop Move Step command
 * specific to Color Control ZCL cluster.
 */
struct _ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ColorControlCmdStopMoveStepConfCallback_t   *callback;   /*!< ZCL Confirmation callback handler entry
                                                                            point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                   service;    /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_ColorControlCmdStopMoveStepReqParams_t       params;     /*!< ZCL Request parameters structure. */
};


/*-------------------------------- Move Color Temperature Cmd -----------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Move Color Temperature command
 * specific to Color Control ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.20, Figure 5-19.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */
    ZBPRO_ZCL_SapColorControlParamMoveColorRate_t      rate;                 /*!< Rate. */

    ZBPRO_ZCL_SapColorControlParamColorTemperature_t   colorTemperatureMin;  /*!< Color Temperature Minimum. */

    ZBPRO_ZCL_SapColorControlParamColorTemperature_t   colorTemperatureMax;  /*!< Color Temperature Maximum. */

    /* 8-bit data. */
    ZBPRO_ZCL_SapColorControlParamHueMoveMode_t        moveMode;             /*!< Move mode. */

} ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Move Color Temperature command
 * specific to Color Control ZCL cluster.
 * \note
 * This command have no response from server. So, confirmation parameters structure has no custom fields.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureConfParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Move Color Temperature command
 * specific to Color Control ZCL cluster.
 */
typedef struct  _ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t
    ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Move Color Temperature command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the empty confirmation parameters because responses will be
 poroceed in the Indication, not in the callback.
 */
typedef void ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureConfCallback_t(
    ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t        *const  reqDescr,
    ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureConfParams_t      *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Move Color Temperature command
 * specific to Color Control ZCL cluster.
 */
struct _ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureConfCallback_t   *callback;   /*!< ZCL Confirmation callback handler entry
                                                                                    point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                           service;    /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqParams_t       params;     /*!< ZCL Request parameters structure. */
};


/*-------------------------------- Step Color Temperature Cmd -----------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Step Color Temperature command
 * specific to Color Control ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 5.2.2.3.21, Figure 5-20.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */
    ZBPRO_ZCL_SapColorControlParamStepSize_t            stepSize;             /*!< Step Size. */

    ZBPRO_ZCL_SapColorControlParamTransitionTime_t      transitionTime;       /*!< Transition Time. */

    ZBPRO_ZCL_SapColorControlParamColorTemperature_t    colorTemperatureMin;  /*!< Color Temperature Minimum. */

    ZBPRO_ZCL_SapColorControlParamColorTemperature_t    colorTemperatureMax;  /*!< Color Temperature Maximum. */

    /* 8-bit data. */
    ZBPRO_ZCL_SapColorControlParamHueStepMode_t         stepMode;             /*!< Step mode. */

} ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Step Color Temperature command
 * specific to Color Control ZCL cluster.
 * \note
 * This command have no response from server. So, confirmation parameters structure has no custom fields.
 */
typedef struct _ZBPRO_ZCL_ColorControlCmdStepColorTemperatureConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_ColorControlCmdStepColorTemperatureConfParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ColorControlCmdStepColorTemperatureConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Step Color Temperature command
 * specific to Color Control ZCL cluster.
 */
typedef struct  _ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t
	ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Step Color Temperature command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the empty confirmation parameters because responses will be
 poroceed in the Indication, not in the callback.
 */
typedef void ZBPRO_ZCL_ColorControlCmdStepColorTemperatureConfCallback_t(
    ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t        *const  reqDescr,
    ZBPRO_ZCL_ColorControlCmdStepColorTemperatureConfParams_t      *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Step Color Temperature command
 * specific to Color Control ZCL cluster.
 */
struct _ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ColorControlCmdStepColorTemperatureConfCallback_t   *callback;   /*!< ZCL Confirmation callback handler entry
                                                                                    point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                           service;    /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqParams_t       params;     /*!< ZCL Request parameters structure. */
};


/************************* PROTOTYPES ***************************************************/
/**//**
 * \name    Functions accept ZCL Local Requests to issue Add Groupm, View group,
 * Get group membership, ReStep group, Remove all groups, Add group if identifying commands specific
 *  Groups ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 * \details
 *  The caller shall specify the following obligatory parameters of request:
 *  - callback                              assign with ZCL Local Confirm handler function,
 *  - remoteApsAddress.addrMode             specify remote (destination) addressing mode,
 *  - respWaitTimeout                       specify timeout for waiting for response, in seconds.
 *      Use value 0xFFFF if default ZCL timeout shall be accepted. Use value 0x0000 if ZCL
 *      layer shall not wait for response and issue confirmation right on confirmation
 *      from APS layer on request to send the command,
 *  - localEndpoint                 specify endpoint on this node with ZCL-based profile
 *      which will be used as the source endpoint,
 *  - disableDefaultResp            set to TRUE if ZCL Default Response is necessarily
 *      needed even for the case of successful command processing on the remote node; set
 *      to FALSE if it's enough to have ZCL Default Response only for the case of failure
 *      on the remote node.
 *
 * \details
 *  For the case when remote (destination) node is bound to this (source) node, one may
 *  set the Remote Addressing Mode to NOT_PRESENT and specify only the Local Endpoint and
 *  Cluster on it. APS layer will then assume Remote node Address (extended or group) and
 *  Endpoint corresponding to the specified Local Endpoint according to the Binding Table.
 *  Otherwise, for direct addressing mode, the caller shall also specify the following
 *  parameters:
 *  - remoteApsAddress      specify destination address (either short, extended or group),
 *  - remoteEndpoint        specify destination endpoint on the remote node with the same
 *      ZCL-based profile as for the Local Endpoint.
 *
 * \details
 *  Following parameters are ignored even if specified by the caller and reassigned by
 *  this command handler:
 *  - localApsAddress       assigned by APS layer with this node address,
 *  - clusterId             assigned to Groups Cluster Id,
 *  - manufCode             ignored because \c manufSpecific is assigned with FALSE,
 *  - commandId             assigned to Command Id
 *  - transSeqNum           ignored because \c useSpecifiedTsn is assigned with FALSE,
 *  - overallStatus        just ignored,
 *  - direction             assigned to Client-to-Server,
 *  - clusterSpecific       assigned to TRUE,
 *  - manufSpecific         assigned to FALSE (i.e., ZCL Standard type),
 *  - useSpecifiedTsn       assigned to FALSE,
 *  - nonUnicastRequest     just ignored.
 */

/**//**
 * \brief   Accepts ZCL Local Request to issue Move to Color command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_ColorControlCmdMoveToColorReq(ZBPRO_ZCL_ColorControlCmdMoveToColorReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Move Color command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_ColorControlCmdMoveColorReq(ZBPRO_ZCL_ColorControlCmdMoveColorReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Step Color command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_ColorControlCmdStepColorReq(ZBPRO_ZCL_ColorControlCmdStepColorReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Enhanced Move to Hue command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReq(
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Enhanced Move Hue command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReq(
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveHueReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Enhanced Step Hue command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReq(
    ZBPRO_ZCL_ColorControlCmdEnhancedStepHueReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Enhanced Move to Hue and Saturation command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReq(
    ZBPRO_ZCL_ColorControlCmdEnhancedMoveToHueAndSaturationReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Color Loop Set command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_ColorControlCmdColorLoopSetReq(
    ZBPRO_ZCL_ColorControlCmdColorLoopSetReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Stop Move Step command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_ColorControlCmdStopMoveStepReq(
    ZBPRO_ZCL_ColorControlCmdStopMoveStepReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Move Color Temperature command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReq(
    ZBPRO_ZCL_ColorControlCmdMoveColorTemperatureReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Step Color Temperature command
 * specific to Color Control ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReq(
    ZBPRO_ZCL_ColorControlCmdStepColorTemperatureReqDescr_t *const  reqDescr);

#endif
