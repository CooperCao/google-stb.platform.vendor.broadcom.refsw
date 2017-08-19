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
*       ZDO / ZDP NWK_Addr and IEEE_Addr Services interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZDO_SAP_TYPES_DISCOVERY_MANAGER_H
#define _BB_ZBPRO_ZDO_SAP_TYPES_DISCOVERY_MANAGER_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZdoCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of Address Resolving request/response types.
 * \ingroup ZBPRO_ZDO_Misc
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclauses 2.4.3.1.1, 2.4.3.1.2, figures 2.20, 2.21,
 *  tables 2.45, 2.46.
 */
typedef enum _ZBPRO_ZDO_AddrResolvingReqType_t
{
    ZBPRO_ZDO_ADDR_REQ_TYPE_SINGLE       = 0,       /*!< Single device response. */

    ZBPRO_ZDO_ADDR_REQ_TYPE_EXTENDED     = 1,       /*!< Extended response. */

    ZBPRO_ZDO_ADDR_REQ_TYPE_RESERVED_MIN = 2,       /*!< Reserved values range lower limit. */

} ZBPRO_ZDO_AddrResolvingReqType_t;


/**//**
 * \brief   Structure for parameters of ZDO Local Request to issue ZDP NWK_Addr_req or
 *  IEEE_Addr_req command.
 * \ingroup ZBPRO_ZDO_Misc
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclauses 2.4.3.1.1, 2.4.3.1.2, figures 2.20, 2.21,
 *  tables 2.45, 2.46.
 */
typedef struct _ZBPRO_ZDO_AddrResolvingReqParams_t
{
    /* Structured data, aligned at 32 bits. */

    ZBPRO_ZDO_Address_t               zdpDstAddress;        /*!< Destination address. May be either unicast or broadcast
                                                                to all devices for which macRxOnWhenIdle = TRUE. Must be
                                                                unicast address for the case when IEEE_Addr_req is
                                                                requested. */

    ZBPRO_ZDO_Address_t               addrOfInterest;       /*!< Either the IEEE address to be matched by the Remote
                                                                Device, or NWK address that is used for IEEE address
                                                                mapping. This field denotes also if NWK_Addr_req or
                                                                IEEE_Addr_req shall be issued. */
    /* 8-bit data. */

    ZBPRO_ZDO_AddrResolvingReqType_t  requestType;          /*!< Request type for this command. */

    uint8_t                           startIndex;           /*!< The starting index for the requested elements of the
                                                                associated devices list. Used only if \c requestType is
                                                                equal to 'Extended response' (0x01). */
} ZBPRO_ZDO_AddrResolvingReqParams_t;



/**//**
 * \brief   Structure for parameters of ZDO Local Confirmation on ZDP NWK_Addr_req or
 *  IEEE_Addr_req command.
 * \ingroup ZBPRO_ZDO_AddrResolvingConf
 * \note
 *  This structure takes its origin from ZDP NWK_Addr_rsp and IEEE_Addr_rsp commands.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclauses 2.4.4.1.1, 2.4.4.1.2, figures 2.62, 2.63,
 *  tables 2.90, 2.91.
 */
typedef struct _ZBPRO_ZDO_AddrResolvingConfParams_t
{
    /* 64-bit data. */

    ZBPRO_ZDO_ExtAddr_t  extAddrRemoteDev;      /*!< 64-bit address for the Remote Device. */

    /* Structured / 32-bit data. */

    SYS_DataPointer_t    payload;               /*!< The \c NWKAddrAssocDevList field. A list of 16-bit addresses, one
                                                    corresponding to each associated device to Remote Device. This field
                                                    is set to EMPTY if \c status is not SUCCESS, or the original request
                                                    is of the 'Single Device Response' type, or the list is empty. */
    /* 16-bit data. */

    ZBPRO_ZDO_NwkAddr_t  nwkAddrRemoteDev;      /*!< 16-bit address for the Remote Device. */

    /* 8-bit data. */

    ZBPRO_ZDO_Status_t   status;                /*!< The status of the Address Resolving request command. */

    uint8_t              numAssocDev;           /*!< Count of the number of 16-bit short addresses to follow. This field
                                                    is set to ZERO if \c status field is not SUCCESS, or the original
                                                    request is of the 'Single Device Response' type, or there are no
                                                    associated devices on the Remote Device. */

    uint8_t              startIndex;            /*!< Starting index into the list of associated devices for this report.
                                                    This field is set to ZERO if \c status field is not SUCCESS, or the
                                                    original request is of the 'Single Device Response' type, or there
                                                    are no associated devices on the Remote Device. */
} ZBPRO_ZDO_AddrResolvingConfParams_t;


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue ZDP NWK_Addr_req or
 *  IEEE_Addr_req command.
 * \ingroup ZBPRO_ZDO_AddrResolvingReq
 */
typedef struct _ZBPRO_ZDO_AddrResolvingReqDescr_t  ZBPRO_ZDO_AddrResolvingReqDescr_t;


/**//**
 * \brief   Data type for ZDO Local Confirmation callback function of ZDP NWK_Addr_req or
 *  IEEE_Addr_req command.
 * \ingroup ZBPRO_ZDO_AddrResolvingConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZDO_AddrResolvingConfCallback_t(
    ZBPRO_ZDO_AddrResolvingReqDescr_t   *const  reqDescr,
    ZBPRO_ZDO_AddrResolvingConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue ZDP NWK_Addr_req or
 *  IEEE_Addr_req command.
 * \ingroup ZBPRO_ZDO_AddrResolvingReq
 */
struct _ZBPRO_ZDO_AddrResolvingReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZDO_AddrResolvingConfCallback_t *callback;        /*!< ZDO Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZdoLocalRequest_t                 service;         /*!< ZDO Request Descriptor service field. */

    ZBPRO_ZDO_AddrResolvingReqParams_t     params;          /*!< ZDO Request parameters structure. */
};


/**//**
 * \brief ZDO ZDP System_Server_Discovery request parameters
 * \ingroup ZBPRO_ZDO_ServerDiscoveryReq
 */
typedef struct _ZBPRO_ZDO_ServerDiscoveryReqParams_t
{
    ZBPRO_ZDO_ServerMask_t  serverMask;                     /*!< Server mask attributes */
    SYS_Time_t              respWaitTimeout;                /*!< Response waiting timeout, in milliseconds.
                                                                 Zero means 'Use default ZDO timeout'. */
} ZBPRO_ZDO_ServerDiscoveryReqParams_t;

/**//**
 * \brief ZDO ZDP System_Server_Discovery request confirmation element
 * \ingroup ZBPRO_ZDO_ServerDiscoveryConf
 */
typedef struct PACKED _ZBPRO_ZDO_ServerDiscoveryRespListItem_t
{
    ZBPRO_ZDO_NwkAddr_t     nwkAddress;                     /*!< Network Address */
    ZBPRO_ZDO_ServerMask_t  serverMask;                     /*!< Server mask attributes */
} ZBPRO_ZDO_ServerDiscoveryRespListItem_t;

/**//**
 * \brief ZDO ZDP System_Server_Discovery request confirmation structure
 * \ingroup ZBPRO_ZDO_ServerDiscoveryConf
 */
typedef struct _ZBPRO_ZDO_ServerDiscoveryConfParams_t
{
    ZBPRO_ZDO_Status_t      status;                         /*!< Request status */
    SYS_DataPointer_t       serverList;                     /*!< List of received responses as sequence of
                                                                 ZBPRO_ZDO_ServerDiscoveryRespListItem_t elements. */
} ZBPRO_ZDO_ServerDiscoveryConfParams_t;

/**//**
 * \brief ZDO ZDP System_Server_Discovery request typedef
 * \ingroup ZBPRO_ZDO_ServerDiscoveryReq
 */
typedef struct _ZBPRO_ZDO_ServerDiscoveryReqDescr_t  ZBPRO_ZDO_ServerDiscoveryReqDescr_t;

/**//**
 * \brief ZDO ZDP System_Server_Discovery request confirmation callback
 * \ingroup ZBPRO_ZDO_ServerDiscoveryConf
 */
typedef void ZBPRO_ZDO_ServerDiscoveryConfCallback_t(
    ZBPRO_ZDO_ServerDiscoveryReqDescr_t *const reqDescr,
    ZBPRO_ZDO_ServerDiscoveryConfParams_t *const confParams);

/**//**
 * \brief ZDO ZDP System_Server_Discovery request structure
 * \ingroup ZBPRO_ZDO_ServerDiscoveryReq
 */
struct _ZBPRO_ZDO_ServerDiscoveryReqDescr_t
{
    ZbProZdoLocalRequest_t                   service;       /*!< Request service field */
    SYS_DataPointer_t                        serviceData;   /*!< Request data */
    ZBPRO_ZDO_ServerDiscoveryConfCallback_t  *callback;     /*!< Confirmation callback */
    ZBPRO_ZDO_ServerDiscoveryReqParams_t     params;        /*!< Request parameters */
};

/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Accepts ZDO Local Request to issue ZDP NWK_Addr_req or IEEE_Addr_req command.
 * \ingroup ZBPRO_ZDO_Functions
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZDO_AddrResolvingReq(
    ZBPRO_ZDO_AddrResolvingReqDescr_t *const  reqDescr);

/**//**
 * \brief ZDO ZDP System_Server_Discovery request function.
 * \ingroup ZBPRO_ZDO_Functions
 * \param[in] reqDescr - pointer to the request structure.
 * \return Nothing.
 */
void ZBPRO_ZDO_ServerDiscoveryReq(
    ZBPRO_ZDO_ServerDiscoveryReqDescr_t *const reqDescr);

#endif /* _BB_ZBPRO_ZDO_SAP_TYPES_DISCOVERY_MANAGER_H */

/* eof bbZbProZdoSapTypesDiscoveryManager.h */