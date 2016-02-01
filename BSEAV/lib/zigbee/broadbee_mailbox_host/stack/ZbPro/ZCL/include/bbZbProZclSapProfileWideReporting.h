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
*   ZCL Profile-Wide Reporting SAP interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_SAP_PROFILE_WIDE_REPORTING_H
#define _BB_ZBPRO_ZCL_SAP_PROFILE_WIDE_REPORTING_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"
#include "bbZbProZclAttrLocal.h"

/************************* COMMON DEFINITIONS ************************************************/
/**//**
 * \brief Common types for Attribute commands.
 * TODO move it to common part.
 */
typedef uint16_t  ZBPRO_ZCL_AttributeId_t;
//typedef uint8_t  ZBPRO_ZCL_AttrDataType_t;




/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumerator for Direction parameter of the  Attribute Reporting Configuration Record of the
 *  Configure Reporting and Read Reporting Configuration Response profile-wide commands.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.7.1.2 and 2.4.10.1.3.
 */
typedef enum _ZBPRO_ZCL_AttributeReportingDirection_t
{
    ZBPRO_ZCL_ATTRIBUTE_REPORTING_CONFIGURATION_DIRECTION_REPORT  = 0x00,    /*!< The attribute data type field, the
                                                                                  minimum reporting interval field,
                                                                                  the maximum reporting interval field
                                                                                  and the reportable change field are
                                                                                  included in the payload, and the
                                                                                  timeout period field is omitted.
                                                                                  The record is sent to a cluster server .
                                                                                  (or client) to configure how it sends
                                                                                  reports to a client (or server) of
                                                                                  the same cluster. */

    ZBPRO_ZCL_ATTRIBUTE_REPORTING_CONFIGURATION_DIRECTION_RECIEVE = 0x01,    /*!< The timeout period field is included
                                                                                  in the payload, and the attribute data
                                                                                  type field, the minimum reporting
                                                                                  interval field, the maximum reporting
                                                                                  interval field and the reportable
                                                                                  change field are omitted. The record
                                                                                  is sent to a cluster client (or server)
                                                                                  to configure how it should expect
                                                                                  reports from a server (or client)
                                                                                  of the same cluster. */

} ZBPRO_ZCL_AttributeReportingDirection_t;



/**//**
 * \brief  Shared type for Minimum Reporting Interval and Maximum Reporting Interval
 * parameters of the  Attribute Reporting Configuration Record of the
 *  Configure Reporting and Read Reporting Configuration Response profile-wide commands.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.7.1.5, 2.4.7.1.6 and 2.4.10.1.5, 2.4.10.1.6.
 */
typedef  uint16_t  ZBPRO_ZCL_AttribureReportingInterval_t;


/**//**
 * \brief  Shared type for Timeout Period parameter of the  Attribute Reporting Configuration Record of the
 *  Configure Reporting and of the Read Reporting Configuration Response profile-wide commands.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.7.1.8 and 2.4.10.1.8.
 */
typedef  uint16_t  ZBPRO_ZCL_AttribureReportingTimeoutPeriod_t;


/*-------------------------------- Configure Reporting Cmd ------------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Configure Reporting
 *  profile-wide command.
 * \note
 * Configure Reporting command with only one Attribute reporting configuration is supported.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.7, figure 2-15 and figure 2-16.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;       /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */

    /* 32-bit data. */
    SYS_DataPointer_t                             payload;             /*!< Reportable change - the minimum change to
                                                                             the attribute that will result in a report
                                                                             being issued. Variable-length field. */

    /* 16-bit data. */

    ZBPRO_ZCL_AttributeId_t                       attributeID;          /*!< Attribute Identifier - If the direction
                                                                             field is 0x00, this field contains the
                                                                             identifier of the attribute that is to be
                                                                             reported. If instead the direction field is
                                                                             0x01, the device shall expect reports of
                                                                             values of this attribute.*/

    ZBPRO_ZCL_AttribureReportingInterval_t        minReportingInterval; /*!< Minimum Reporting Interval - the minimum
                                                                             interval, in seconds, between issuing
                                                                             reports of the specified attribute. */

    ZBPRO_ZCL_AttribureReportingInterval_t        maxReportingInterval; /*!< Maximum Reporting Interval - the maximum
                                                                             interval, in seconds, between issuing
                                                                             reports of the specified attribute. */

    ZBPRO_ZCL_AttribureReportingTimeoutPeriod_t   timeoutPeriod;        /*!< Timeout Period - the maximum expected time,
                                                                             in seconds, between received reports for
                                                                             the attribute specified in the attribute
                                                                             identifier field. */

    /* 8-bit data. */
    ZBPRO_ZCL_AttributeReportingDirection_t       directionReporting;            /*!< Direction - The direction field specifies
                                                                             whether values of the attribute are to be
                                                                             reported, or whether reports of the
                                                                             attribute are to be received. */

    ZBPRO_ZCL_AttrDataType_t                      attributeDataType;    /*!< Attribute data type - the data type of the
                                                                             attribute that is to be reported. */

} ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Configure Reporting
 *  profile-wide command.
 * \note
 * Configure Reporting command with only one Attribute reporting configuration is supported. So, there only
 * one Attribute status record in the Configure Reporting Response.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.8, figure 2-17 and figure 2-18.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;       /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */

    /* Custom parameters. */

    /* 16-bit data. */
    ZBPRO_ZCL_AttributeId_t                  attributeID; /*!< Attribute Identifier - If the direction field is 0x00,
                                                               this field contains the identifier of the attribute that
                                                               is to be reported. If instead the direction field is
                                                               0x01, the device shall expect reports of
                                                               values of this attribute.*/

    /* 8-bit data. */

    ZBPRO_ZCL_Status_t                       statusReporting;      /*!< Status field - specifies the status of the configure
                                                               reporting operation attempted on this attribute. */

    ZBPRO_ZCL_AttributeReportingDirection_t  directionReporting;   /*!< Direction field specifies whether values of the
                                                               attribute are reported (0x00), or whether reports of
                                                               the attribute are received (0x01). */

} ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Configure Reporting
 *  profile-wide command.
 */
typedef struct     _ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t
    ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Configure Reporting
 *  profile-wide command.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfCallback_t(
    ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t     *const  reqDescr,
    ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfParams_t   *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Configure Reporting
 *  profile-wide command.
 */
struct _ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ProfileWideCmdConfigureReportingConfCallback_t     *callback;       /*!< ZCL Confirmation callback handler
                                                                                       entry point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                          service;        /*!< ZCL Request Descriptor service
                                                                                       field. */

    ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqParams_t         params;         /*!< ZCL Request parameters
                                                                                        structure. */
};


/*-------------------------------- Read Reporting Configuration Cmd ------------------------------------------*/
/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Read Reporting Configuration
 *  profile-wide command.
 * \note
 * Read Reporting Configuration command with only one Attribute record is supported.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.9, figure 2-19 and figure 2-20.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;       /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */
    /* 16-bit data. */
    ZBPRO_ZCL_AttributeId_t                   attributeID;    /*!< Attribute identifier field shall contain the
                                                                   identifier of the attribute whose reporting
                                                                   configuration details are to be read. */

    /* 8-bit data/ */
    ZBPRO_ZCL_AttributeReportingDirection_t   directionReporting;      /*!< Direction field specifies whether values of the
                                                                   attribute are reported (0x00), or whether reports of
                                                                   the attribute are received (0x01). */

} ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Read Reporting Configuration
 *  profile-wide command.
 * \note
  * Read Reporting Configuration command with only one Attribute record is supported. So, there only
 * one Attribute record in the Read Reporting Configuration Response.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.10.1, figure 2-21 and figure 2-22.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;       /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */

    /* 32-bit data. */
    SYS_DataPointer_t                         payload;                  /*!< Reportable change - the minimum change to
                                                                             the attribute that will result in a report
                                                                             being issued. Variable-length field. */

    /* 16-bit data. */
    ZBPRO_ZCL_AttributeId_t                   attributeID;              /*!< Attribute identifier field shall contain the
                                                                             identifier of the attribute whose reporting
                                                                             configuration details are to be read. */

    ZBPRO_ZCL_AttrDataType_t                  attributeDataType;        /*!< Attribute data type - the data type of the
                                                                             attribute that is to be reported. */

    ZBPRO_ZCL_AttribureReportingInterval_t    minReportingInterval;     /*!< Minimum Reporting Interval - the minimum
                                                                             interval, in seconds, between issuing
                                                                             reports of the specified attribute. */

    ZBPRO_ZCL_AttribureReportingInterval_t    maxReportingInterval;     /*!< Maximum Reporting Interval - the maximum
                                                                             interval, in seconds, between issuing
                                                                             reports of the specified attribute. */

    ZBPRO_ZCL_AttribureReportingTimeoutPeriod_t   timeoutPeriod;        /*!< Timeout Period - the maximum expected time,
                                                                             in seconds, between received reports for
                                                                             the attribute specified in the attribute
                                                                             identifier field. */
    /* 8-bit data. */

    ZBPRO_ZCL_Status_t                       statusReporting;      /*!< Status field - specifies the status of the configure
                                                               reporting operation attempted on this attribute. */

    ZBPRO_ZCL_AttributeReportingDirection_t  directionReporting;   /*!< Direction field specifies whether values of the
                                                               attribute are reported (0x00), or whether reports of
                                                               the attribute are received (0x01). */

} ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationConfParams_t;


/*
 * Validate structure of ZCL Local Request Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Read Reporting Configuration
 *  profile-wide command.
 */
typedef struct     _ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t
    ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Read Reporting Configuration
 *  profile-wide command.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationConfCallback_t(
    ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t     *const  reqDescr,
    ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationConfParams_t   *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Read Reporting Configuration
 *  profile-wide command.
 */
struct _ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationConfCallback_t  *callback;  /*!< ZCL Confirmation callback handler
                                                                                       entry point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                               service;   /*!< ZCL Request Descriptor service
                                                                                       field. */

    ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqParams_t      params;    /*!< ZCL Request parameters
                                                                                       structure. */
};


/*-------------------------------- Report Attributes Command indication ------------------------------------------*/
/**//**
 * \brief   Structure for Attribure report record in the Report Attribute
 * profile-wide command.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.11.1, figure 2-24.
 */
 typedef struct PACKED _ZBPRO_ZCL_ProfileWideCmdAttributeReport_t
 {
    ZBPRO_ZCL_AttributeId_t   attributeID;    /*!< The attribute identifier field is 16 bits in length and shall
                                                   contain the identifier of the attribute that is being reported. */

    ZBPRO_ZCL_AttrDataType_t  attributeType;  /*!< The attribute data type field contains the data type of the
                                                   attribute that is being reported. */

    /*!< The attribute data field is variable in length and shall contain
         the actual value of the attribute being reported.
    uint8_t                   attributeData[N];  */

 } ZBPRO_ZCL_ProfileWideCmdAttributeReport_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Report Attributes
 * profile-wide command.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.11.1, figure 2-23 and figure 2-24.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;   /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* Custom parameters. */

    /* 32-bit data. */
    SYS_DataPointer_t                         payload;             /*!< There is a list of the Attribure reports.
                                                                        See the ZBPRO_ZCL_ProfileWideCmdAttributeReport_t*/

} ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t;


/*
 * Validate structure of ZCL Local Confirmation Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t);


/************************* PROTOTYPES ***************************************************/
/**//**
 * \name    Functions accept ZCL Local Requests to issue Configure Reporting and
 * Read Reporting Configuration profile-wide commands.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 * \details
 *  The caller shall specify the following obligatory parameters of request:
 *  - callback                      assign with ZCL Local Confirm handler function,
 *  - remoteApsAddress.addrMode     specify remote (destination) addressing mode,
 *  - clusterId                     specify cluster to which this command is applied,
 *  - respWaitTimeout               specify timeout of waiting for response, in seconds.
 *      Use value 0x0000 or 0xFFFF as well if default ZCL timeout shall be accepted. Note
 *      that the default ZCL timeout may be different for different commands,
 *  - localEndpoint                 specify endpoint on this node with ZCL-based profile
 *      which will be used as the source endpoint,
 *  - manufSpecific                 specify the domain of command, either Manufacturer-
 *      Specific or ZCL Standard,
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
 *  Depending on some parameters other parameters may be either adopted by the command
 *  processor or ignored (substituted with default values). Here is the list:
 *  - manufCode             specify Manufacturer Code if \p manufSpecific is set to TRUE.
 *      For the case of ZCL Standard command (i.e., when \p manufSpecific equals to FALSE)
 *      this parameter may be left unassigned; it is ignored by ZCL Dispatcher and not
 *      included into the ZCL Frame.
 *
 *  For the case of Local Request to send Request command these parameters are ignored.
 * \details
 *  Following parameters are ignored even if specified by the caller and reassigned by
 *  this command handler:
 *  - localApsAddress       assigned by APS layer with this node address,
 *  - commandId             assigned to Command Id according to particular Local Request,
 *  - clusterSpecific       assigned to FALSE (i.e., Profile-Wide type),
 *  - useSpecifiedTsn       assigned to FALSE for request commands; assigned to TRUE for
 *      solicited response commands.
 *  - transSeqNum           assigned by APS layer.
 *  - overallStatus         specify ZCL Responder application status,
 *  - nonUnicastRequest     assigned to FALSE.
 *  - direction             assigned to Client-to-Server, because only this direction is supported.
 */

/**//**
 * \brief   Accepts ZCL Local Request to issue Configure Reporting
 *  profile-wide command.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_ProfileWideCmdConfigureReportingReq(
    ZBPRO_ZCL_ProfileWideCmdConfigureReportingReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Read Reporting Configuration
 *  profile-wide command.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReq(
    ZBPRO_ZCL_ProfileWideCmdReadReportingConfigurationReqDescr_t *const  reqDescr);


/**//**
 * \brief   Handles ZCL Local Indication on reception of Report Attributes
 *  profile-wide command.
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 */
void ZBPRO_ZCL_ProfileWideCmdReportAttributesInd(
		ZBPRO_ZCL_ProfileWideCmdReportAttributesIndParams_t *const  indParams);

#endif
