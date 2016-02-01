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
* FILENAME: $Workfile: trunk/stack/ZbPro/ZCL/include/bbZbProZclSapClusterIASWD.h $
*
* DESCRIPTION:
*   ZCL IAS WD cluster SAP interface.
*
* $Revision: 7614 $
* $Date: 2015-07-21 14:48:39Z $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_SAP_CLUSTER_IAS_WD_H
#define _BB_ZBPRO_ZCL_SAP_CLUSTER_IAS_WD_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of attributes of IAS WD ZCL cluster Server side.
 * \details
 *  These attributes are provided by Server side of the cluster.
 * \details
 *  IAS WD ZCL cluster has no attributes provided by Client side.
 * \note
 *  This implementation of IAS WD ZCL cluster doesn't provide Server side. *
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 8.4.2.2, Table 8-17.
 */
typedef enum _ZBPRO_ZCL_SapIASWDServerAttributeId_t
{
    ZBPRO_ZCL_SAP_IASWD_ATTR_ID_MAX_DURATION            = 0x0000,        /*!< MaxDuration. */

    ZBPRO_ZCL_SAP_IASWD_ATTR_ID_MAX                     = 0xFFFF,       /*!< Introduced only to make the
                                                                             enumeration 16-bit wide. */
} ZBPRO_ZCL_SapIASWDServerAttributeId_t;


/**//**
 * \brief   Data type shared by MaxDuration attribute of IAS WD cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 8.4.2.2, Table 8-17.
 */
typedef   uint16_t   ZBPRO_ZCL_SapIASWDAttrMaxDuration_t;    /*!< Shared data type for MaxDuration attribute. */


/*-------------------------------- Start warning Cmd ------------------------------------------------------*/
/**//**
 * \brief   Enum for Warning Mode field of the Start Warning Command request
 * specific to IAS WD ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.4.2.3.1.2, Table 8-19.
 */
typedef enum _ZBPRO_ZCL_SapIASWDParamWarningMode_t
{
    ZBPRO_ZCL_SAP_IAS_WD_WARNING_MODE_STOP         =  0x00,        /*!< Stop (no warning). */

    ZBPRO_ZCL_SAP_IAS_WD_WARNING_MODE_BURGLAR      =  0x01,        /*!< Burglar. */

    ZBPRO_ZCL_SAP_IAS_WD_WARNING_MODE_FIRE         =  0x02,        /*!< Fire. */

    ZBPRO_ZCL_SAP_IAS_WD_WARNING_MODE_EMERGENCY    =  0x03,        /*!< Emergency. */

} ZBPRO_ZCL_SapIASWDParamWarningMode_t;


/**//**
 * \brief   Enum for Strobe Field of the Start Warning Command request
 * specific to IAS WD ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.4.2.3.1.3, Table 8-20.
 */
typedef enum _ZBPRO_ZCL_SapIASWDParamWarningStrobe_t
{
    ZBPRO_ZCL_SAP_IAS_WD_WARNING_STROBE_NO_STROBE         =  0x00,        /*!< No strobe. */

    ZBPRO_ZCL_SAP_IAS_WD_WARNING_STROBE_USE_STROBE        =  0x01        /*!< Use strobe in parallel to warning. */

} ZBPRO_ZCL_SapIASWDParamWarningStrobe_t;


/**//**
 * \brief   Enum for Siren Level Field of the Start Warning Command request
 * specific to IAS WD ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.4.2.3.1.4, Table 8-21.
 */
typedef enum _ZBPRO_ZCL_SapIASWDParamSirenLevel_t
{
    ZBPRO_ZCL_SAP_IAS_WD_SIREN_LEVEL_LOW         =  0x00,        /*!< Low level sound. */

    ZBPRO_ZCL_SAP_IAS_WD_SIREN_LEVEL_MEDIUM      =  0x01,        /*!< Medium level sound. */

    ZBPRO_ZCL_SAP_IAS_WD_SIREN_LEVEL_HIGHT       =  0x02,        /*!< High level sound. */

    ZBPRO_ZCL_SAP_IAS_WD_SIREN_LEVEL_VERY_HIGH   =  0x03         /*!< Very high level sound. */

} ZBPRO_ZCL_SapIASWDParamSirenLevel_t;


/**//**
 * \brief Shared type for Strobe Duty Cycle parameter of the Start Warning command.
*/
typedef  uint8_t  ZBPRO_ZCL_SapIASWDParamStrobeDutyCycle_t;


/**//**
 * \brief Shared type for Warning Duration parameter of the Start Warning command.
*/
typedef  uint16_t  ZBPRO_ZCL_SapIASWDParamWarningDuration_t;


/**//**
 * \brief   Enum for Strobe Level Field of the Start Warning Command request
 * specific to IAS WD ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.4.2.3.1.7, Table 8-22.
 */
typedef enum _ZBPRO_ZCL_SapIASWDParamStrobeLevel_t
{
    ZBPRO_ZCL_SAP_IAS_WD_STROB_LEVEL_LOW         =  0x00,       /*!< Low level strobe. */

    ZBPRO_ZCL_SAP_IAS_WD_STROB_LEVEL_MEDIUM      =  0x01,       /*!< Medium level strobe. */

    ZBPRO_ZCL_SAP_IAS_WD_STROB_LEVEL_HIGHT       =  0x02,       /*!< High level strobe. */

    ZBPRO_ZCL_SAP_IAS_WD_STROB_LEVEL_VERY_HIGH   =  0x03        /*!< Very high level strobe. */

} ZBPRO_ZCL_SapIASWDParamStrobeLevel_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Start warning command
 * specific to IAS WD ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.4.2.3.1, figure 8-13.
 */
typedef struct _ZBPRO_ZCL_IASWDCmdStartWarningReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;     /*!< Set of obligatory parameters of ZCL public
                                                                          interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */

    ZBPRO_ZCL_SapIASWDParamWarningDuration_t  warningDuration;    /*!< Requested duration of warning, in seconds. If
                                                                     both Strobe and Warning Mode are "0" this
                                                                     field shall be ignored. */
    /* 8-bit data. */

    ZBPRO_ZCL_SapIASWDParamWarningMode_t      warningMode;        /*!< 4-bit enumeration for Warning Mode. */


    ZBPRO_ZCL_SapIASWDParamWarningStrobe_t    strobe;             /*!< 2-bit enumeration for Strobe. */


    ZBPRO_ZCL_SapIASWDParamSirenLevel_t       sirenLevel;         /*!< 2-bit enumeration for Siren Level. */

    ZBPRO_ZCL_SapIASWDParamStrobeDutyCycle_t  strobeDutyCycle;    /*!< Indicates the length of the flash cycle.
                                                                     For example, if Strobe Duty Cycle Field specifies
                                                                     “40,” then the strobe SHALL flash ON for 4/10ths
                                                                     of a second and then turn OFF for 6/10ths of a
                                                                     second. */

    ZBPRO_ZCL_SapIASWDParamStrobeLevel_t      strobeLevel;        /*!< Indicates the intensity of the strobe. */

} ZBPRO_ZCL_IASWDCmdStartWarningReqParams_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Start warning command
 * specific to IAS WD ZCL cluster.
 *\note
 * Start warning command have no response. So, Confirmation will be empty (with obligatory parameters only).
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.4.2.3.1, figure 8-13.
 */
typedef    ZbProZclLocalPrimitiveParamsPrototype_t     ZBPRO_ZCL_IASWDCmdStartWarningConfParams_t;


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Start warning command
 * specific to IAS WD ZCL cluster.
 */
typedef struct     _ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t     ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Start warning command
 * specific to IAS WD ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_IASWDCmdStartWarningConfCallback_t(
        ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t     *const  reqDescr,
        ZBPRO_ZCL_IASWDCmdStartWarningConfParams_t   *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Start warning command
 * specific to IAS WD ZCL cluster.
 */
struct _ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_IASWDCmdStartWarningConfCallback_t     *callback;       /*!< ZCL Confirmation callback handler entry
                                                                            point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t              service;        /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_IASWDCmdStartWarningReqParams_t         params;         /*!< ZCL Request parameters structure. */
};


/*-------------------------------- Squawk Cmd ------------------------------------------------------*/
/**//**
 * \brief   Enum for Squawk Mode Field of the Squawk Command request
 * specific to IAS WD ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.4.2.3.2.2, Table 8-23.
 */
typedef enum _ZBPRO_ZCL_SapIASWDParamSquawkMode_t
{
    ZBPRO_ZCL_SAP_IAS_WD_SQUAWK_MODE_ARMED         =  0x00,        /*!< Notification sound for “System is armed”. */

    ZBPRO_ZCL_SAP_IAS_WD_SQUAWK_MODE_DISARMED      =  0x01,        /*!< Notification sound for "System is disarmed". */

} ZBPRO_ZCL_SapIASWDParamSquawkMode_t;


/**//**
 * \brief   Enum for Strobe Field of the Squawk Command request
 * specific to IAS WD ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.4.2.3.1.3, Table 8-20.
 */
typedef enum _ZBPRO_ZCL_SapIASWDParamSquawkStrobe_t
{
    ZBPRO_ZCL_SAP_IAS_WD_WARNING_SQUAWK_NO_STROBE     =  0x00,        /*!< No strobe. */

    ZBPRO_ZCL_SAP_IAS_WD_WARNING_SQUAWK_USE_STROBE    =  0x01         /*!< Use strobe in parallel to warning. */

} ZBPRO_ZCL_SapIASWDParamSquawkStrobe_t;


/**//**
 * \brief   Enum for Squawk Level Field of the Squawk Command request
 * specific to IAS WD ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.4.2.3.2.4, Table 8-25.
 */
typedef enum _ZBPRO_ZCL_SapIASWDParamSquawkLevel_t
{
    ZBPRO_ZCL_SAP_IAS_WD_SQUAWK_LEVEL_LOW         =  0x00,        /*!< Low level squawk. */

    ZBPRO_ZCL_SAP_IAS_WD_SQUAWK_LEVEL_MEDIUM      =  0x01,        /*!< Medium level squawk. */

    ZBPRO_ZCL_SAP_IAS_WD_SQUAWK_LEVEL_HIGHT       =  0x02,        /*!< High level squawk. */

    ZBPRO_ZCL_SAP_IAS_WD_SQUAWK_LEVEL_VERY_HIGH   =  0x03         /*!< Very high level squawk. */

} ZBPRO_ZCL_SapIASWDParamSquawkLevel_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Squawk Command
  * specific to IAS WD ZCL cluster.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.4.2.3.2, figure 8-14.
 */
typedef struct _ZBPRO_ZCL_IASWDCmdSquawkReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;  /*!< Set of obligatory parameters of ZCL public
                                                                       interface to local application. */
    /* Custom parameters. */

    /* 8-bit data. */

    ZBPRO_ZCL_SapIASWDParamSquawkMode_t       squawkMode;         /*!< 4-bit enumeration for Squawk Mode. */

    ZBPRO_ZCL_SapIASWDParamSquawkStrobe_t     strobe;             /*!< The strobe field. */

    ZBPRO_ZCL_SapIASWDParamSquawkLevel_t      squawkLevel;        /*!< 2-bit enumeration for Squawk Level. */

} ZBPRO_ZCL_IASWDCmdSquawkReqParams_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Squawk command
 * specific to IAS WD ZCL cluster.
 *\note
 * Squawk command have no response. So, Confirmation will be empty (with obligatory parameters only).
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 8.4.2.3.1, figure 8-13.
 */
typedef    ZbProZclLocalPrimitiveParamsPrototype_t     ZBPRO_ZCL_IASWDCmdSquawkConfParams_t;


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Squawk command
 * specific to IAS WD ZCL cluster.
 */
typedef struct   _ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t     ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Squawk command
 * specific to IAS WD ZCL cluster.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_IASWDCmdSquawkConfCallback_t(
        ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t     *const  reqDescr,
        ZBPRO_ZCL_IASWDCmdSquawkConfParams_t   *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Squawk command
 * specific to IAS WD ZCL cluster.
 */
struct _ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_IASWDCmdSquawkConfCallback_t           *callback;       /*!< ZCL Confirmation callback handler entry
                                                                            point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t              service;        /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_IASWDCmdSquawkReqParams_t               params;         /*!< ZCL Request parameters structure. */
};


/************************* PROTOTYPES ***************************************************/
/**//**
 * \name    Functions accept ZCL Local Requests to issue Start warning, Squawk commands
 *  specific to IAS WD ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 * \details
 *  The caller shall specify the following obligatory parameters of request:
 *  - callback                      assign with ZCL Local Confirm handler function,
 *  - remoteApsAddress.addrMode     specify remote (destination) addressing mode,
 *  - respWaitTimeout               specify timeout of waiting for response, in seconds.
 *      Use value 0x0000 or 0xFFFF as well if default ZCL timeout shall be accepted. Note
 *      that the default ZCL timeout may be different for different commands,
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
 *  - clusterId             assigned to IAD WD Cluster Id,
 *  - manufCode             ignored because \c manufSpecific is assigned with FALSE,
 *  - commandId             assigned to Command Id
 *  - transSeqNum           ignored because \c useSpecifiedTsn is assigned with FALSE,
 *  - overallStatus         just ignored,
 *  - direction             assigned to Client-to-Server,
 *  - clusterSpecific       assigned to TRUE (i.e. Cluster-Specific domain),
 *  - manufSpecific         assigned to FALSE (i.e., ZCL Standard type),
 *  - useSpecifiedTsn       assigned to FALSE (i.e., generate new TSN on transmission),
 *  - nonUnicastRequest     just ignored.
 */


/**//**
 * \brief   Accepts ZCL Local Request to issue Start warning command
 *  command specific to IAS WD ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_IASWDCmdStartWarningReq(ZBPRO_ZCL_IASWDCmdStartWarningReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Squawk command
 *  command specific to IAS WD ZCL cluster.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_IASWDCmdSquawkgReq(ZBPRO_ZCL_IASWDCmdSquawkReqDescr_t *const  reqDescr);


#endif
