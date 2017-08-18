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
 *      This header describes types and API for the ZDO Active_Ep (Active Endpoints) service.
 *
*******************************************************************************/

#ifndef _ZBPRO_ZDO_SAP_TYPES_ACTIVE_EP_H
#define _ZBPRO_ZDO_SAP_TYPES_ACTIVE_EP_H

/************************* INCLUDES *****************************************************/
#include "bbZbProZdoCommon.h"

/************************* TYPES ********************************************************/

/**//**
 * \brief ZDO ZDP Active_Ep request parameters which correspond
 *        the appropriate fields of the Active_Ep command frame
 * \ingroup ZBPRO_ZDO_ActiveEpReq
 */
typedef struct _ZBPRO_ZDO_ActiveEpReqParams_t
{
    ZBPRO_ZDO_Address_t     zdpDstAddress;         /*!< Destination address. */

    ZBPRO_ZDO_NwkAddr_t     nwkAddrOfInterest;     /*!< Network short address of interest. */

    SYS_Time_t              respWaitTimeout;       /*!< Response waiting timeout, in milliseconds.
                                                        Zero means 'Use default ZDO timeout'. */
} ZBPRO_ZDO_ActiveEpReqParams_t;

/**//**
 * \brief ZDO ZDP Active_Ep request confirmation parameters which correspond
 *        the appropriate fields of the Active_Ep response frame
 * \ingroup ZBPRO_ZDO_ActiveEpConf
 */
typedef struct _ZBPRO_ZDO_ActiveEpConfParams_t
{
    ZBPRO_ZDO_Status_t      status;                /*!< Request status. */
    ZBPRO_ZDO_NwkAddr_t     nwkAddrOfInterest;     /*!< Short address. */
    uint8_t                 activeEpCount;         /*!< Endpoint count. */
    SYS_DataPointer_t       activeEpList;          /*!< Active endpoints list. */
} ZBPRO_ZDO_ActiveEpConfParams_t;

/**//**
 * \brief ZDO ZDP Active_Ep request descriptor prototype
 * \ingroup ZBPRO_ZDO_ActiveEpReq
 */
typedef struct _ZBPRO_ZDO_ActiveEpReqDescr_t  ZBPRO_ZDO_ActiveEpReqDescr_t;

/**//**
 * \brief Callback function type.
 * \ingroup ZBPRO_ZDO_ActiveEpConf
 */
typedef void ZBPRO_ZDO_ActiveEpConfCallback_t(ZBPRO_ZDO_ActiveEpReqDescr_t *const reqDescr,
                                              ZBPRO_ZDO_ActiveEpConfParams_t *const confParams);

/**//**
 * \brief ZDO ZDP Active_Ep request descriptor
 * \ingroup ZBPRO_ZDO_ActiveEpReq
 */
struct _ZBPRO_ZDO_ActiveEpReqDescr_t
{
    ZbProZdoLocalRequest_t               service;  /*!< Request service field. */
    ZBPRO_ZDO_ActiveEpConfCallback_t    *callback; /*!< Confirmation callback. */
    ZBPRO_ZDO_ActiveEpReqParams_t        params;   /*!< Request parameters. */
};

/************************* PROTOTYPES ***************************************************/

/*************************************************************************************//**
 * \brief ZDO ZDP Active_Ep request function.
 * \ingroup ZBPRO_ZDO_Functions
 * \param[in] reqDescr - pointer to the request structure.
 * \return Nothing.
*****************************************************************************************/
void ZBPRO_ZDO_ActiveEpReq(ZBPRO_ZDO_ActiveEpReqDescr_t *const reqDescr);

#endif  /* _ZBPRO_ZDO_SAP_TYPES_ACTIVE_EP_H */

/* eof bbZbProZdoSapTypesActiveEp.h */