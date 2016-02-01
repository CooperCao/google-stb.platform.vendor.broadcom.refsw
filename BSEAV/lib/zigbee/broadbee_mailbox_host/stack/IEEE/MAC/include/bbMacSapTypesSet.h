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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacSapTypesSet.h $
*
* DESCRIPTION:
*   MLME-SET service data types definition.
*
* $Revision: 2722 $
* $Date: 2014-06-24 19:37:15Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_SAP_TYPES_SET_H
#define _BB_MAC_SAP_TYPES_SET_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapPib.h"            /* MAC-PIB for MAC-SAP definitions. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of the MLME-SET.request.
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
 *  payload object shall persists up to the confirmation from the MAC is issued.
 * \note    Security MAC-PIB attributes are excluded because MAC security is not
 *  implemented. The PIBAttributeIndex parameter is excluded, because it is used just with
 *  MAC-PIB security attributes.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.13.1, table 70.
 */
typedef struct _MAC_SetReqParams_t
{
    /* 64-bit data. */
    MAC_PibAttributeValue_t  attributeValue;        /*!< The value to write to the indicated PIB attribute. */

    /* 32-bit data. */
    SYS_DataPointer_t        payload;               /*!< The value of attribute with variable data size. */

    /* 8-bit data. */
    MAC_PibAttributeId_t     attribute;             /*!< The identifier of the PIB attribute to write. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    MAC_PibAttributeIndex_t  attributeIndex;        /*!< The index within the table of the specified PIB attribute to
                                                        write. */

} MAC_SetReqParams_t;


/**//**
 * \brief   Structure for parameters of the MLME-SET.confirm.
 * \note    Security MAC-PIB attributes are excluded because MAC security is not
 *  implemented. The PIBAttributeIndex parameter is excluded, because it is used just with
 *  MAC-PIB security attributes.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.13.2, table 71.
 */
typedef struct _MAC_SetConfParams_t
{
    /* 8-bit data. */
    MAC_Status_t             status;                /*!< The result of the request to write the PIB attribute. */

    MAC_PibAttributeId_t     attribute;             /*!< The identifier of the PIB attribute that was written. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    MAC_PibAttributeIndex_t  attributeIndex;        /*!< The index within the table of the specified PIB attribute to
                                                        write. */
} MAC_SetConfParams_t;


/**//**
 * \brief   Structure for descriptor of the MLME-SET.request.
 */
typedef struct _MAC_SetReqDescr_t  MAC_SetReqDescr_t;


/**//**
 * \brief   Template for callback handler-function of the MLME-SET.confirm.
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
 */
struct _MAC_SetReqDescr_t
{
    /* 32-bit data. */
    MAC_SetConfCallback_t *callback;        /*!< Entry point of the confirmation callback function. */

    /* Structured data. */
    MacServiceField_t      service;         /*!< MAC requests service field. */

    MAC_SetReqParams_t     params;          /*!< Request parameters structured object. */
};


#endif /* _BB_MAC_SAP_TYPES_SET_H */