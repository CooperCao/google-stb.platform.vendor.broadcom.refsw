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
 *      MLME-RESET service data types definition.
 *
*******************************************************************************/

#ifndef _BB_MAC_SAP_TYPES_RESET_H
#define _BB_MAC_SAP_TYPES_RESET_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapDefs.h"           /* MAC-SAP common definitions. */
#include "bbMacSapService.h"        /* MAC-SAP service data types. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of the MLME-RESET.request.
 * \ingroup ResetReq
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.9.1.1, table 63.
 */
typedef struct _MAC_ResetReqParams_t
{
    /* 8-bit data. */
    Bool8_t  setDefaultPib;     /*!< If TRUE, the MAC sublayer is reset, and all MAC PIB attributes are set to their
                                    default values. If FALSE, the MAC sublayer is reset, but all MAC PIB attributes
                                    retain their values prior to the generation of the MLME-RESET.request primitive. */
} MAC_ResetReqParams_t;


/**//**
 * \brief   Structure for parameters of the MLME-RESET.confirm.
 * \ingroup ResetConf
 * \details Possible values for the \c status parameter are the following:
 *  - SUCCESS       The requested operation was completed successfully.
 *  - RESET         Another MLME-RESET.request was issued prior to execution of this
 *      MLME-RESET.request being confirmed.
 *
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.9.2.1, table 64.
 */
typedef struct _MAC_ResetConfParams_t
{
    /* 8-bit data. */
    MAC_Status_t  status;       /*!< The result of the reset operation. */

} MAC_ResetConfParams_t;


/**//**
 * \brief   Structure for descriptor of the MLME-RESET.request.
 * \ingroup ResetReq
 */
typedef struct _MAC_ResetReqDescr_t  MAC_ResetReqDescr_t;


/**//**
 * \brief   Template for callback handler-function of the MLME-RESET.confirm.
 * \ingroup ResetConf
 * \param[in]   reqDescr    Pointer to the confirmed request descriptor.
 * \param[in]   confParams  Pointer to the confirmation parameters object.
 * \details Call functions of this type provided by higher layers of corresponding MAC
 *  contexts, ZigBee PRO and RF4CE, from the MAC to issue the MLME-RESET.confirm to the
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
 *  See IEEE 802.15.4-2006, subclause 7.1.9.2.
 */
typedef void MAC_ResetConfCallback_t(MAC_ResetReqDescr_t *const reqDescr, MAC_ResetConfParams_t *const confParams);


/**//**
 * \brief   Structure for descriptor of the MLME-RESET.request.
 * \ingroup ResetReq
 */
struct _MAC_ResetReqDescr_t
{
    /* 32-bit data. */
    MAC_ResetConfCallback_t *callback;      /*!< Entry point of the confirmation callback function. */

#ifndef _HOST_
    /* Structured data. */
    MacServiceField_t        service;       /*!< MAC requests service field. */
#endif

    MAC_ResetReqParams_t     params;        /*!< Request parameters structured object. */
};


#endif /* _BB_MAC_SAP_TYPES_RESET_H */

/* eof bbMacSapTypesReset.h */