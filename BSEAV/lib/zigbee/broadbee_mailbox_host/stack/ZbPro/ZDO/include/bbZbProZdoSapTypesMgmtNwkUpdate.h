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
*   ZDO / ZDP Mgmt_NWK_Update Service interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZDO_SAP_TYPES_MGMT_NWK_UPDATE_H
#define _BB_ZBPRO_ZDO_SAP_TYPES_MGMT_NWK_UPDATE_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZdoCommon.h"
#include "bbZbProNwkSapTypesIb.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \name    Limit values for Mgmt_NWK_Update_req command parameters.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.3.3.9, table 2.88.
 */
/**@{*/
#define ZBPRO_ZDO_MGMT_NWK_UPDATE_REQ_PARAM_SCAN_DURATION_MAX   0x05        /*!< Maximum allowed value for the
                                                                                \c scanDuration parameter in ED-Scanning
                                                                                Request subtype. */

#define ZBPRO_ZDO_MGMT_NWK_UPDATE_REQ_PARAM_SCAN_COUNT_MAX      0x05        /*!< Maximum allowed value for the
                                                                                \c scanCount parameter in ED-Scanning
                                                                                Request subtype. */
/**@}*/


/**//**
 * \brief   Enumeration of subtypes of Mgmt_NWK_Update_req command.
 * \details
 *  Limit values and special values of 'Change' types (0xFE and 0xFF) may be used as-is
 *  for parsing the received Local or Remote Request parameter \c scanDuration to discover
 *  the type of Request. The 'ED_SCAN' value may be used in internal processing when need
 *  to denote a Request for channel scanning in general.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.3.3.9, table 2.88.
 */
typedef enum _ZBPRO_ZDO_MgmtNwkUpdateReqSubtypeId_t
{
    ZBPRO_ZDO_MGMT_NWK_UPDATE_REQ_SUBTYPE_ED_SCANNING       = 0x00,     /*!< ED-Scanning Request subtype. */

    ZBPRO_ZDO_MGMT_NWK_UPDATE_REQ_SUBTYPES_ED_SCANNING_MAX  =
            ZBPRO_ZDO_MGMT_NWK_UPDATE_REQ_PARAM_SCAN_DURATION_MAX,      /*!< Limit (maximum) value for \c ScanDuration
                                                                            parameter corresponding to the ED-Scanning
                                                                            subtype of Request. */

    ZBPRO_ZDO_MGMT_NWK_UPDATE_REQ_SUBTYPE_RESERVED,                     /*!< Reserved (Invalid) Request subtype. This
                                                                            value is used internally only. */

    ZBPRO_ZDO_MGMT_NWK_UPDATE_REQ_SUBTYPES_CHANGE_MIN       = 0xFE,     /*!< Limit (minimum) value for \c ScanDuration
                                                                            parameter corresponding to the
                                                                            Change-Channel and Assign-Attributes
                                                                            subtypes of Request. */

    ZBPRO_ZDO_MGMT_NWK_UPDATE_REQ_SUBTYPE_CHANGE_CHANNEL    = 0xFE,     /*!< Request to change the current channel. */

    ZBPRO_ZDO_MGMT_NWK_UPDATE_REQ_SUBTYPE_ASSING_ATTRIBUTES = 0xFF,     /*!< Request to change the apsChannelMask and
                                                                            nwkManagerAddr attributes. */
} ZBPRO_ZDO_MgmtNwkUpdateReqSubtypeId_t;


/**//**
 * \brief   Returns ZDO Mgmt_NWK_Update Local or Remote Request sybtype.
 * \param[in]   scanDuration    Value of \c ScanDuration parameter.
 * \return  Either ED-Scanning, Change-Channel, Assign-Attributes, or Reserved subtype.
 */
#define ZBPRO_ZDP_MGMT_NWK_UPDATE_REQ_SUBTYPE(scanDuration)\
        ((scanDuration) <= ZBPRO_ZDO_MGMT_NWK_UPDATE_REQ_SUBTYPES_ED_SCANNING_MAX ?\
                ZBPRO_ZDO_MGMT_NWK_UPDATE_REQ_SUBTYPE_ED_SCANNING :\
                ((scanDuration) >= ZBPRO_ZDO_MGMT_NWK_UPDATE_REQ_SUBTYPES_CHANGE_MIN ?\
                        (ZBPRO_ZDO_MgmtNwkUpdateReqSubtypeId_t)scanDuration :\
                        ZBPRO_ZDO_MGMT_NWK_UPDATE_REQ_SUBTYPE_RESERVED))


/**//**
 * \brief   Structure for parameters of ZDO Local Request to issue ZDP Mgmt_NWK_Update_req
 *  command.
 * \note
 *  The field \c ScanDuration is also used as the request subtype identifier field
 *  according to the Specification.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.3.3.9, figure 2.61, table 2.88.
 */
typedef struct _ZBPRO_ZDO_MgmtNwkUpdateReqParams_t
{
    /* Structured data, aligned at 32 bits. */

    ZBPRO_ZDO_Address_t                    zdpDstAddress;       /*!< Destination address. May be either unicast or
                                                                    broadcast to all devices for which macRxOnWhenIdle =
                                                                    TRUE. Must be unicast in the case when channel
                                                                    scanning is requested; and must be unicast or
                                                                    broadcast for the case of change type request
                                                                    (broadcast is normal). */
    /* 32-bit data. */

    PHY_ChannelsSet_t                      scanChannels;        /*!< This parameter is used for different purposes
                                                                    depending on the request subtype. For the case of
                                                                    ED-Scanning request it contains the \c ScanChannels
                                                                    parameter of the NLME-ED-SCAN.request. For the case
                                                                    of Change-Channel request it contains the channel
                                                                    designator given as a mask with just single bit set
                                                                    to one. For the case of Assign-Attributes request it
                                                                    contains value for the apsChannelMask attribute. */
    /* 16-bit data. */

    ZBPRO_NWK_NIB_ManagerAddr_t            nwkManagerAddr;      /*!< The NWK address for the device with the Network
                                                                    Manager bit set in its Node Descriptor. Must be
                                                                    saved in the nwkManagerAddr attribute. Used only if
                                                                    attributes change is requested. */
    /* 8-bit data. */

    MAC_BeaconOrder_t                      scanDuration;        /*!< The length of time to spend scanning each channel;
                                                                    actual time is calculated with the formula
                                                                    [aBaseSuperframeDuration * (2^n + 1)], where n is
                                                                    \c scanDuration, in symbols. Maximum allowed value
                                                                    is 0x05. Two special values are used also: 0xFE to
                                                                    request for channel change, 0xFF to request to
                                                                    change the apsChannelMask and nwkManagerAddr
                                                                    attributes. This field due to this reason is used
                                                                    also as the \c Subtype field, meaning the subtype of
                                                                    request. */

    uint8_t                                scanCount;           /*!< The number of energy scans to be conducted and
                                                                    reported. Maximum allowed value is 0x05. Used only
                                                                    if channel scanning is requested. */

    ZBPRO_NWK_NIB_UpdateId_t               nwkUpdateId;         /*!< Identifier (incremented in each request) of this
                                                                    update request. Assigned by the Network Channel
                                                                    Manager with the nwkUpdateID attribute value. Used
                                                                    only if channel or attributes change is
                                                                    requested. */
} ZBPRO_ZDO_MgmtNwkUpdateReqParams_t;


/**//**
 * \brief   Structure for parameters of ZDO Local Confirmation on ZDP Mgmt_NWK_Update_req
 *  command.
 * \details
 *  This structure takes its origin from ZDP Mgmt_NWK_Update_notify command.
 * \details
 *  This structure is also used for parameters of (1) ZDO Local Request to issue
 *  Unsolicited ZDP Mgmt_NWK_Update_notify command, and (2) ZDO Indication for received
 *  Unsolicited ZDP Mgmt_NWK_Update_notify command.
 * \note
 *  For the case of ZDO Local Request to issue Unsolicited ZDP Mgmt_NWK_Update_notify
 *  command, the \c zdpAddress shall be assigned with the Network (16-bit short) Address
 *  stored in the nwkManagerAddr attribute.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.4.3.9, figure 2.102, table 2.137.
 */
typedef struct _ZBPRO_ZDO_MgmtNwkUpdateConfParams_t
{
    /* Structured data, aligned at 32 bits. */

    ZBPRO_ZDO_Address_t      zdpAddress;                    /*!< Either Destination or Source Address. Used only for the
                                                                case of Unsolicited Mgmt_NWK_Update_notify. Shall be
                                                                assigned with the NWK Manager Device destination address
                                                                in Local ZDO Unsolicited Response parameters; and is
                                                                assigned with the originator device source address in
                                                                parameters of Local Indication on reception of
                                                                unsolicited response. */
    /* 32-bit data. */

    PHY_ChannelsSet_t        scannedChannels;               /*!< List of channels scanned by the request. */

    SYS_DataPointer_t        payload;                       /*!< The \c EnergyValues field. The result of an energy
                                                                measurement made on this channel. */
    /* 16-bit data. */

    ZBPRO_NWK_NIB_TxTotal_t  totalTransmissions;            /*!< Count of the total transmissions reported by the
                                                                device. */

    ZBPRO_NWK_NIB_TxTotal_t  transmissionFailures;          /*!< Sum of the total transmission failures reported by the
                                                                device. */
    /* 8-bit data. */

    ZBPRO_ZDO_Status_t       status;                        /*!< The status of the Mgmt_NWK_Update_notify command. */

    uint8_t                  scannedChannelsListCount;      /*!< The list shall contain the number of records contained
                                                                in the \c EnergyValues parameter. */
} ZBPRO_ZDO_MgmtNwkUpdateConfParams_t;


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue ZDP Mgmt_NWK_Update_req
 *  command.
 */
typedef struct _ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t  ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t;


/**//**
 * \brief   Data type for ZDO Local Confirmation callback function of ZDP
 *  Mgmt_NWK_Update_req command.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZDO_MgmtNwkUpdateConfCallback_t(
                ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t   *const  reqDescr,
                ZBPRO_ZDO_MgmtNwkUpdateConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue ZDP Mgmt_NWK_Update_req
 *  command.
 */
struct _ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZDO_MgmtNwkUpdateConfCallback_t *callback;        /*!< ZDO Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZdoLocalRequest_t                 service;         /*!< ZDO Request Descriptor service field. */

    ZBPRO_ZDO_MgmtNwkUpdateReqParams_t     params;          /*!< ZDO Request parameters structure. */
};


/**//**
 * \brief   Structure for parameters of ZDO Local Request to issue Unsolicited ZDP
 *  Mgmt_NWK_Update_notify command.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.4.3.9, figure 2.102, table 2.137.
 */
typedef ZBPRO_ZDO_MgmtNwkUpdateConfParams_t  ZBPRO_ZDO_MgmtNwkUpdateUnsolRespParams_t;


/**//**
 * \brief   Structure for parameters of ZDO Local Confirmation on Unsolicited ZDP
 *  Mgmt_NWK_Update_notify command.
 */
typedef struct _ZBPRO_ZDO_MgmtNwkUpdateUnsolConfParams_t
{
    /* 8-bit data. */

    ZBPRO_ZDO_Status_t  status;         /*!< Status field. */

} ZBPRO_ZDO_MgmtNwkUpdateUnsolConfParams_t;


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue Unsolicited ZDP
 *  Mgmt_NWK_Update_notify command.
 */
typedef struct _ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t  ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t;


/**//**
 * \brief   Data type for ZDO Local Confirmation callback function of Unsolicited ZDP
 *  Mgmt_NWK_Update_notify command.
 * \param[in]   respDescr       Pointer to the descriptor of response being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZDO_MgmtNwkUpdateUnsolConfCallback_t(
                ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t  *const  respDescr,
                ZBPRO_ZDO_MgmtNwkUpdateUnsolConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue Unsolicited ZDP
 *  Mgmt_NWK_Update_notify command.
 */
struct _ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZDO_MgmtNwkUpdateUnsolConfCallback_t *callback;       /*!< ZDO Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZdoLocalRequest_t                      service;        /*!< ZDO Request Descriptor service field. */

    ZBPRO_ZDO_MgmtNwkUpdateUnsolRespParams_t    params;         /*!< ZDO Request parameters structure. */
};


/**//**
 * \brief   Structure for parameters of ZDO Indication for received Unsolicited ZDP
 *  Mgmt_NWK_Update_notify command.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.4.3.9, figure 2.102, table 2.137.
 */
typedef ZBPRO_ZDO_MgmtNwkUpdateUnsolRespParams_t  ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t;


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Accepts ZDO Local Request to issue ZDP Mgmt_NWK_Update_req command.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZDO_MgmtNwkUpdateReq(
                ZBPRO_ZDO_MgmtNwkUpdateReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZDO Local Request to issue Unsolicited ZDP Mgmt_NWK_Update_notify
 *  command.
 * \param[in]   respDescr       Pointer to ZDO Local Unsolicited Response descriptor.
 */
void ZBPRO_ZDO_MgmtNwkUpdateUnsolResp(
                ZBPRO_ZDO_MgmtNwkUpdateUnsolRespDescr_t *const  respDescr);


/**//**
 * \brief   Handles ZDO Indication for received Unsolicited ZDP Mgmt_NWK_Update_notify
 *  command.
 * \param[in]   indParams       Pointer to ZDO Indication parameters.
 * \note
 *  This function must be provided by the application.
 */
void ZBPRO_ZDO_MgmtNwkUpdateUnsolInd(
                ZBPRO_ZDO_MgmtNwkUpdateUnsolIndParams_t *const  indParams);


#endif /* _BB_ZBPRO_ZDO_SAP_TYPES_MGMT_NWK_UPDATE_H */
