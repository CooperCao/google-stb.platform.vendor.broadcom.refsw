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
 *      MLME-SET service data types definition.
 *
*******************************************************************************/

#ifndef _BB_MAC_SAP_TYPES_SET_H
#define _BB_MAC_SAP_TYPES_SET_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapPib.h"            /* MAC-PIB for MAC-SAP definitions. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of the MLME-SET.request.
 * \ingroup SetReq
 * \details Fixed length attributes are transferred directly by their values via the
 *  \c attributeValue parameter. For this case the \c payload may be left uninitialized;
 *  the MAC will not use this parameter.
 * \details Variable length attribute values are transferred as dynamically or statically
 *  allocated payloads via the \c payload parameter. This is used for MAC-PIB attribute
 *  macBeaconPayload.
 * \note    In the case of variable length attribute the payload described by the
 *  \c payload parameter must be dynamically or statically allocated by the caller (the
 *  higher layer). The MAC makes its private copy of the payload object, so the original
 *  payload object established by the caller, may be freed (it also may be preserved by
 *  the caller for other activities). The higher layer is responsible for freeing memory
 *  allocated for the payload; the MAC layer never frees it. The higher layer should
 *  dismiss the payload object when the MAC layer confirms the corresponding request; the
 *  payload object shall persist until the confirmation from the MAC is issued.
 * \note    Security MAC-PIB attributes are excluded because MAC security is not
 *  implemented.
 * \note    The PIBAttributeIndex parameter is implemented but ignored by MAC because it
 *  is used just with MAC-PIB security attributes which are not implemented. For such
 *  attributes MLME-SET.confirm will return UNSUPPORTED_ATTRIBUTE status (status
 *  INVALID_INDEX is never returned).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.13.1.1, table 70.
 */
typedef struct _MAC_SetReqParams_t
{
    /* 64-bit data. */
    MAC_PibAttributeValue_t  attributeValue;        /*!< The value to write to the indicated PIB attribute. */

    /* 32-bit data. */
    SYS_DataPointer_t        payload;               /*!< The value of attribute with variable data size. */

    /* 16-bit data. */
    MAC_PibAttributeIndex_t  attributeIndex;        /*!< The index within the table of the specified PIB attribute to
                                                        write. */
    /* 8-bit data. */
    MAC_PibAttributeId_t     attribute;             /*!< The identifier of the PIB attribute to write. */

} MAC_SetReqParams_t;


/**//**
 * \brief   Structure for parameters of the MLME-SET.confirm.
 * \ingroup SetConf
 * \details Possible values for the \c status parameter are the following:
 *  - SUCCESS                   The requested operation was completed successfully.
 *  - INVALID_PARAMETER         A parameter in the primitive is either not supported or is
 *      out of the valid range.
 *  - READ_ONLY                 A SET request was issued with the identifier of an
 *      attribute that is read only.
 *  - UNSUPPORTED_ATTRIBUTE     A SET request was issued with the identifier of a PIB
 *      attribute that is not supported.
 *  - RESET                     An MLME-RESET.request was issued prior to execution of the
 *      MLME-SET.request being confirmed.
 *
 * \note    The following values for the \c status parameter are not used due to listed
 *  reasons:
 *  - INVALID_INDEX             MAC security is not implemented.
 *
 * \note    Security MAC-PIB attributes are excluded because MAC security is not
 *  implemented. The PIBAttributeIndex parameter is left unassigned because it is used
 *  just with MAC-PIB security attributes which are not implemented; or assigned with 0x00
 *  'Not used' if _MAC_SAP_PROCESS_REDUNDANT_PARAMS_ conditional build key is defined by
 *  the project make configuration file. For all security attributes MLME-GET.confirm will
 *  return UNSUPPORTED_ATTRIBUTE status (status INVALID_INDEX is never returned).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.13.2.1, table 71.
 */
typedef struct _MAC_SetConfParams_t
{
    /* 16-bit data. */
    MAC_PibAttributeIndex_t  attributeIndex;        /*!< The index within the table of the specified PIB attribute to
                                                        write. */
    /* 8-bit data. */
    MAC_PibAttributeId_t     attribute;             /*!< The identifier of the PIB attribute that was written. */

    MAC_Status_t             status;                /*!< The result of the request to write the PIB attribute. */

} MAC_SetConfParams_t;


/**//**
 * \brief   Structure for descriptor of the MLME-SET.request.
 * \ingroup SetReq
 */
typedef struct _MAC_SetReqDescr_t  MAC_SetReqDescr_t;


/**//**
 * \brief   Template for callback handler-function of the MLME-SET.confirm.
 * \ingroup SetConf
 * \param[in]   reqDescr    Pointer to the confirmed request descriptor.
 * \param[in]   confParams  Pointer to the confirmation parameters object.
 * \details Call functions of this type provided by higher layers of corresponding MAC
 *  contexts, ZigBee PRO and RF4CE, from the MAC to issue the MLME-SET.confirm to the
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
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.13.2.
 */
typedef void MAC_SetConfCallback_t(MAC_SetReqDescr_t *const reqDescr, MAC_SetConfParams_t *const confParams);


/**//**
 * \brief   Structure for descriptor of the MLME-SET.request.
 * \ingroup SetReq
 */
struct _MAC_SetReqDescr_t
{
    /* 32-bit data. */
    MAC_SetConfCallback_t *callback;        /*!< Entry point of the confirmation callback function. */
#ifndef _HOST_
    /* Structured data. */
    MacServiceField_t      service;         /*!< MAC requests service field. */
#endif
    MAC_SetReqParams_t     params;          /*!< Request parameters structured object. */
};


#endif /* _BB_MAC_SAP_TYPES_SET_H */

/* eof bbMacSapTypesSet.h */