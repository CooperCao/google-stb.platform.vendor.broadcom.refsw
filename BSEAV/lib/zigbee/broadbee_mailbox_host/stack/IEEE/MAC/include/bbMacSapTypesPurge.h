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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacSapTypesPurge.h $
*
* DESCRIPTION:
*   MCPS-PURGE service data types definition.
*
* $Revision: 2999 $
* $Date: 2014-07-21 13:30:43Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_SAP_TYPES_PURGE_H
#define _BB_MAC_SAP_TYPES_PURGE_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapTypesData.h"      /* MCPS-DATA service data types. */


/************************* VALIDATIONS **************************************************/
#if !defined(_MAC_CONTEXT_ZBPRO_)
# error This header shall be compiled only if the ZigBee PRO context is included into the build.
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of the MCPS-PURGE.request.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.1.4.1, table 44.
 */
typedef struct _MAC_PurgeReqParams_t
{
    /* 8-bit data. */
    MAC_MsduHandle_t  msduHandle;       /*!< The handle of the MSDU to be purged from the transaction queue. */

} MAC_PurgeReqParams_t;


/**//**
 * \brief   Structure for parameters of the MCPS-PURGE.confirm.
 * \details Possible values for the \c status parameter are the following:
 *  - SUCCESS           The requested operation was completed successfully.
 *  - INVALID_HANDLE    A request to purge an MSDU from the transaction queue was made
 *      using an MSDU handle that was not found in the transaction table.
 *  - RESET             An MLME-RESET.request was issued prior to execution of the
 *      MCPS-PURGE.request being confirmed.
 *
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.1.1.5.1, 7.1.17, tables 45, 78.
 */
typedef struct _MAC_PurgeConfParams_t
{
    /* 8-bit data. */
    MAC_MsduHandle_t  msduHandle;       /*!< The handle of the MSDU requested to be purged from the transaction
                                            queue. */

    MAC_Status_t      status;           /*!< The status of the request to be purged an MSDU from the transaction
                                            queue. */
} MAC_PurgeConfParams_t;


/**//**
 * \brief   Structure for descriptor of the MCPS-PURGE.request.
 */
typedef struct _MAC_PurgeReqDescr_t  MAC_PurgeReqDescr_t;


/**//**
 * \brief   Template for the callback handler-function of the MCPS-PURGE.confirm.
 * \param[in]   reqDescr    Pointer to the confirmed request descriptor.
 * \param[in]   confParams  Pointer to the confirmation parameters object.
 * \details Call functions of this type provided by the ZigBee PRO higher layer from the
 *  MAC to issue the MCPS-PURGE.confirm to the ZigBee PRO higher layer that originally
 *  issued the request primitive to the MAC.
 * \details To issue the confirmation primitive the MAC calls the confirmation callback
 *  handler-function that was specified with the \c callback parameter of the original
 *  request primitive descriptor that is pointed here by the \p reqDescr argument.
 * \details The request descriptor object that was originally used to issue request to the
 *  MAC and is pointed here with the \p reqDescr is released by the MAC for random use by
 *  the higher layer (the owner of the request descriptor object) when this confirmation
 *  callback handler-function is called by the MAC.
 * \details Treat the parameters structure pointed by the \p confParams and passed into
 *  the confirmation callback handler-function as it has been allocated in the program
 *  stack by the MAC before calling this callback function and will be destroyed just
 *  after this callback function returns.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.1.5.
 */
typedef void MAC_PurgeConfCallback_t(MAC_PurgeReqDescr_t *const reqDescr, MAC_PurgeConfParams_t *const confParams);


/**//**
 * \brief   Structure for descriptor of the MCPS-PURGE.request.
 */
struct _MAC_PurgeReqDescr_t
{
    /* 32-bit data. */
    MAC_PurgeConfCallback_t *callback;      /*!< Entry point of the confirmation callback function. */

    /* Structured data. */
    MacServiceField_t        service;       /*!< MAC requests service field. */

    MAC_PurgeReqParams_t     params;        /*!< Request parameters structured object. */
};


#endif /* _BB_MAC_SAP_TYPES_PURGE_H */