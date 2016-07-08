/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacSapTypesReset.h $
*
* DESCRIPTION:
*   MLME-RESET service data types definition.
*
* $Revision: 1195 $
* $Date: 2014-01-23 13:03:59Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_SAP_TYPES_RESET_H
#define _BB_MAC_SAP_TYPES_RESET_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapDefs.h"           /* MAC-SAP common definitions. */
#include "bbMacSapService.h"        /* MAC-SAP service data types. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief MLME-RESET.request parameters data type.
 */
typedef struct _MAC_ResetReqParams_t
{
    /* 8-bit data. */
    Bool8_t  setDefaultPib;     /*!< If TRUE, all MAC PIB attributes
                                    are set to their default values. */
} MAC_ResetReqParams_t;


/**//**
 * \brief MLME-RESET.confirm parameters data type.
 */
typedef struct _MAC_ResetConfParams_t
{
    /* 8-bit data. */
    MAC_Status_t  status;       /*!< The result of the reset operation. */

} MAC_ResetConfParams_t;


/**//**
 * \brief MLME-RESET.request descriptor data type declaration.
 */
typedef struct _MAC_ResetReqDescr_t  MAC_ResetReqDescr_t;


/**//**
 * \brief MLME-RESET.confirm callback function data type.
 * \details Call this function to issue MLME-RESET.confirm to the higher layer.
 * \param reqDescr Pointer to the confirmed request descriptor data structure.
 * \param confParams Pointer to the confirmation parameters data structure. Treat this
 *  data structure in the confirmation handler-function as it has been allocated in the
 *  program stack by MAC before calling this callback-handler and will be destroyed just
 *  this callback function returns.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.9.2.
 */
typedef void MAC_ResetConfCallback_t(MAC_ResetReqDescr_t *const reqDescr, MAC_ResetConfParams_t *const confParams);


/**//**
 * \brief   Structure for descriptor of the MLME-RESET.request.
 */
struct _MAC_ResetReqDescr_t
{
    /* 32-bit data. */
    MAC_ResetConfCallback_t *callback;      /*!< Entry point of the confirmation callback function. */

    /* Structured data. */
    MacServiceField_t        service;       /*!< MAC requests service field. */

    MAC_ResetReqParams_t     params;        /*!< Request parameters structured object. */
};


#endif /* _BB_MAC_SAP_TYPES_RESET_H */