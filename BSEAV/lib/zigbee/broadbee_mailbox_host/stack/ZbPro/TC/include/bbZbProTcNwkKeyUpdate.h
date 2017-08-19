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
*       Trust Center Network Key Update interface.
*
*******************************************************************************/

#ifndef _ZBPRO_TC_NWKKEY_UPDATE_H
#define _ZBPRO_TC_NWKKEY_UPDATE_H

/************************* INCLUDES *****************************************************/
#include "bbZbProApsCommon.h"
#include "bbZbProApsSapSecurityTypes.h"

#include "bbZbProTcKeywords.h"

/*************************** TYPES ******************************************************/

/**//**
 * \brief TC-NWK-KEY-UPDATE.request descriptor data type pre-declaration.
 * \ingroup ZBPRO_TC_KeyUpdateReq
 */
typedef struct _ZBPRO_TC_NwkKeyUpdateReqDescr_t ZBPRO_TC_NwkKeyUpdateReqDescr_t;

/**//**
 * \brief TC-NWK-KEY-UPDATE.confirmation parameters data structure.
 * \ingroup ZBPRO_TC_KeyUpdateConf
 */
typedef ZBPRO_APS_SecurityServicesConfParams_t ZBPRO_TC_NwkKeyUpdateConfParams_t;

/**//**
 * \brief TC-NWK-KEY-UPDATE.confirm callback type.
 * \ingroup ZBPRO_TC_KeyUpdateConf
 */
typedef void ZBPRO_TC_NwkKeyUpdateConfCallback_t(ZBPRO_TC_NwkKeyUpdateReqDescr_t *const reqDescr,
        ZBPRO_TC_NwkKeyUpdateConfParams_t *const confParams);

/**//**
 * \brief TC-NWK-KEY-UPDATE.request parameters.
 * \ingroup ZBPRO_TC_KeyUpdateReq
 */
typedef struct _ZBPRO_TC_NwkKeyUpdateReqParams_t
{
    ZbProSspKey_t newKey;                           /*!< New Network key */
} ZBPRO_TC_NwkKeyUpdateReqParams_t;

/**//**
 * \brief TC-NWK-KEY-UPDATE.request descriptor data type declaration.
 * \ingroup ZBPRO_TC_KeyUpdateReq
 */
typedef struct _ZBPRO_TC_NwkKeyUpdateReqDescr_t
{
    struct
    {
        SYS_QueueElement_t  next;                  /*!< Service queue field */
    } service;                                     /*!< Service field container */

    ZBPRO_TC_NwkKeyUpdateReqParams_t params;       /*!< Request parameters */
    ZBPRO_TC_NwkKeyUpdateConfCallback_t *callback; /*!< Confirmation callback */
} ZBPRO_TC_NwkKeyUpdateReqDescr_t;

/******************************* PROTOTYPES ********************************************/

/**//**
 * \brief Updates Network Key in the network
 * \ingroup ZBPRO_TC_Functions
 *
 * \param[in] reqDescr - pointer to the request parameters.
 * \return Nothing.
 */
TC_PUBLIC void ZBPRO_TC_NwkKeyUpdateReq(ZBPRO_TC_NwkKeyUpdateReqDescr_t *reqDescr);

/**//**
 * \brief Initializes the Network Key Update handler descriptor
 * \ingroup ZBPRO_TC_Functions
 *
 * \return Nothing.
 */
TC_PUBLIC void ZBPRO_TC_NwkKeyUpdateReset(void);

#endif /* _ZBPRO_TC_NWKKEY_UPDATE_H */

/* eof bbZbProTcNwkKeyUpdate.h */