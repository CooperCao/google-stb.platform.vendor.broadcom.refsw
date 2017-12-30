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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      MLME-GET service data types definition.
 *
*******************************************************************************/

#ifndef _BB_MAC_SAP_TYPES_GET_H
#define _BB_MAC_SAP_TYPES_GET_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapPib.h"            /* MAC-PIB for MAC-SAP definitions. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of the MLME-GET.request.
 * \ingroup GetReq
 * \note    Security MAC-PIB attributes are excluded because MAC security is not
 *  implemented.
 * \note    The PIBAttributeIndex parameter is implemented but ignored by MAC because it
 *  is used just with MAC-PIB security attributes which are not implemented. For such
 *  attributes MLME-GET.confirm will return UNSUPPORTED_ATTRIBUTE status (status
 *  INVALID_INDEX is never returned).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.6.1.1, table 56.
 */
typedef struct _MAC_GetReqParams_t
{
    /* 16-bit data. */
    MAC_PibAttributeIndex_t  attributeIndex;        /*!< The index within the table of the specified PIB attribute to
                                                        read. */
    /* 8-bit data. */
    MAC_PibAttributeId_t     attribute;             /*!< The identifier of the PIB attribute to read. */

} MAC_GetReqParams_t;


/**//**
 * \brief   Structure for parameters of the MLME-GET.confirm.
 * \ingroup GetConf
 * \details Possible values for the \c status parameter are the following:
 *  - SUCCESS                   The requested operation was completed successfully.
 *  - UNSUPPORTED_ATTRIBUTE     A GET request was issued with the identifier of a PIB
 *      attribute that is not supported.
 *  - RESET                     An MLME-RESET.request was issued prior to execution of the
 *      MLME-GET.request being confirmed.
 *
 * \note    The following values for the \c status parameter are not used due to listed
 *  reasons:
 *  - INVALID_INDEX             MAC security is not implemented.
 *
 * \details Fixed length attributes are transferred directly by their values via the
 *  \c attributeValue parameter. For this case the \c payload will be set to empty payload
 *  by the MAC; the higher layer is not ought to, but recommended, to free it.
 * \details Variable length attribute values are transferred as statically allocated
 *  payloads via the \c payload parameter. This is used for PHY-PIB attribute
 *  phyChannelsSupported and MAC-PIB attribute macBeaconPayload. The higher layer is not
 *  ought to, but recommended, to free it.
 * \note    In the case of variable length attribute the payload described by the
 *  \c payload parameter will be statically allocated by the MAC. The higher layer
 *  nevertheless is recommended to treat this payload as a general case (i.e., as a
 *  dynamic payload) and to free it after use. The higher layer is responsible for freeing
 *  memory allocated for the payload; the MAC layer never frees it.
 * \note    Due to the static nature of the payload object returned here, the higher layer
 *  shall work with it carefully in order not to disrupt the internal static memory of the
 *  MAC. On the other hand, the higher layer has the ability to edit the MAC static memory
 *  content directly without issuing MLME-SET.request; it is enough to edit the content of
 *  the static payload object returned by the MLME-GET.confirm. MAC takes its static
 *  content for processing only from a task execution flow; thus content of its static
 *  memory may be changed safely only from the execution flow of another task, but not
 *  from an interrupt handler.
 * \note    Security MAC-PIB attributes are excluded because MAC security is not
 *  implemented. The PIBAttributeIndex parameter is left unassigned because it is used
 *  just with MAC-PIB security attributes which are not implemented; or assigned with 0x00
 *  'Not used' if _MAC_SAP_PROCESS_REDUNDANT_PARAMS_ conditional build key is defined by
 *  the project make configuration file. For all security attributes MLME-GET.confirm will
 *  return UNSUPPORTED_ATTRIBUTE status (status INVALID_INDEX is never returned).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.6.2.1, table 57.
 */
typedef struct _MAC_GetConfParams_t
{
    /* 64-bit data. */
    MAC_PibAttributeValue_t  attributeValue;        /*!< The value of the indicated PIB attribute that was read. */

    /* 32-bit data. */
    SYS_DataPointer_t        payload;               /*!< The value of attribute with variable data size. */

    /* 16-bit data. */
    MAC_PibAttributeIndex_t  attributeIndex;        /*!< The index within the table or array of the specified PIB
                                                        attribute to read. */
    /* 8-bit data. */
    MAC_PibAttributeId_t     attribute;             /*!< The identifier of the PIB attribute that was read. */

    MAC_Status_t             status;                /*!< The result of the request for PIB attribute information. */

} MAC_GetConfParams_t;


/**//**
 * \brief   Structure for descriptor of the MLME-GET.request.
 * \ingroup GetReq
 */
typedef struct _MAC_GetReqDescr_t  MAC_GetReqDescr_t;


/**//**
 * \brief   Template for callback handler-function of the MLME-GET.confirm.
 * \ingroup GetConf
 * \param[in]   reqDescr    Pointer to the confirmed request descriptor.
 * \param[in]   confParams  Pointer to the confirmation parameters object.
 * \details Call functions of this type provided by higher layers of corresponding MAC
 *  contexts, ZigBee PRO and RF4CE, from the MAC to issue the MLME-GET.confirm to the
 *  higher layer that originally issued the request primitive to the MAC.
 * \details To issue the confirmation primitive the MAC calls the confirmation callback
 *  handler-function that was specified with the \c callback parameter of the original
 *  request primitive descriptor that is pointed here by the \p reqDescr argument.
 * \details The request descriptor object that was originally used to issue the request to
 *  the MAC and is pointed here with the \p reqDescr is released by the MAC for random
 *  use by the higher layer (the owner of the request descriptor object) when this
 *  confirmation callback handler-function is called by the MAC.
 * \details Treat the parameters structure pointed by the \p confParams and passed into
 *  the confirmation callback handler-function as it has been allocated in the program
 *  stack by the MAC before calling this callback handler and will be destroyed just after
 *  this callback function returns.
 * \note    The payload described by the \c payload parameter is statically allocated in
 *  the MAC private memory; nevertheless the higher layer is recommended to dismiss the
 *  payload object after using it to stay compatible with general rules of using payloads.
 *  Via the payload descriptor the higher layer is granted with the full read-write access
 *  to the static payload in the MAC private memory; the higher layer is allowed to
 *  perform write-back operation on the static payload array received with this
 *  confirmation parameters (i.e., instead of performing changes in the local memory and
 *  issuing the MLME-SET.request to save the changes into the MAC-PIB attribute). If the
 *  caller (the higher layer) needs to transfer the data in turn to a layer above and
 *  wishes to prohibit the write-back access to the MAC private memory, the caller shall
 *  make a private copy of the data returned by the MAC.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.6.2.
 */
typedef void MAC_GetConfCallback_t(MAC_GetReqDescr_t *const reqDescr, MAC_GetConfParams_t *const confParams);


/**//**
 * \brief   Structure for descriptor of the MLME-GET.request.
 * \ingroup GetReq
 */
struct _MAC_GetReqDescr_t
{
    /* 32-bit data. */
    MAC_GetConfCallback_t *callback;        /*!< Entry point of the confirmation callback function. */
#ifndef _HOST_
    /* Structured data. */
    MacServiceField_t      service;         /*!< MAC requests service field. */
#endif
    MAC_GetReqParams_t     params;          /*!< Request parameters structured object. */
};


#endif /* _BB_MAC_SAP_TYPES_GET_H */

/* eof bbMacSapTypesGet.h */