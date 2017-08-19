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
*       ZHA Profile configuration.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZHA_SAP_EZ_MODE_H
#define _BB_ZBPRO_ZHA_SAP_EZ_MODE_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZhaCommon.h"

/**//**
 * \brief   Enumeration of EZ Mode Statuses.
 * \ingroup ZBPRO_ZHA_Misc
 */
typedef enum
{
    ZBPRO_EZ_SUCCESS,
    ZBPRO_EZ_CANNOT_START,
    ZBPRO_EZ_WRONG_EP,
} ZBPRO_ZHA_EzModeStatus_t;

/**//**
 * \brief   Enumeration of EZ Mode Device Roles.
 * \ingroup ZBPRO_ZHA_Misc
 */
typedef enum
{
    ZBPRO_EZ_STEERING_ONLY,
    ZBPRO_EZ_TARGET,
    ZBPRO_EZ_INITIATOR,
} ZBPRO_ZHA_EzModeRole_t;

/**//**
 * \brief   EZ Mode request parameters.
 * \ingroup ZBPRO_ZHA_EzModeReq
 */
typedef struct _ZBPRO_ZHA_EzModeReqParams_t
{
    uint32_t                roundTimeMs;        /*!< Round time in Microseconds. */
    uint8_t                 times;              /*!< Times. */
    ZBPRO_ZHA_EzModeRole_t  ezRole;             /*!< Role to use. */
    Bool8_t                 factoryFresh;       /*!< Is factory fresh? */
    ZBPRO_APS_EndpointId_t  endPoint;           /*!< Endpoint to use. */
} ZBPRO_ZHA_EzModeReqParams_t;

/**//**
 * \brief   EZ Mode request parameters.
 * \ingroup ZBPRO_ZHA_EzModeConf
 */
typedef struct _ZBPRO_ZHA_EzModeConfParams_t
{
    ZBPRO_ZHA_EzModeStatus_t status;            /*!< Request status */
} ZBPRO_ZHA_EzModeConfParams_t;

/**//**
 * \brief   EZ Mode request parameters.
 * \ingroup ZBPRO_ZHA_EzModeReq
 */
typedef struct _ZBPRO_ZHA_EzModeReqDescr_t ZBPRO_ZHA_EzModeReqDescr_t;

/**//**
 * \brief   EZ Mode request parameters.
 * \ingroup ZBPRO_ZHA_EzModeConf
 */
typedef void (*ZBPRO_ZHA_EzModeCallback_t)(ZBPRO_ZHA_EzModeReqDescr_t *const reqDescr,
        ZBPRO_ZHA_EzModeConfParams_t *const confParams);

/**//**
 * \brief   EZ Mode request parameters.
 * \ingroup ZBPRO_ZHA_EzModeReq
 */
struct _ZBPRO_ZHA_EzModeReqDescr_t
{
    SYS_QueueElement_t   queueElement;          /*!< Queue service field */
    ZBPRO_ZHA_EzModeReqParams_t   params;       /*!< Request parameters. */
    ZBPRO_ZHA_EzModeCallback_t    callback;     /*!< Confirmation callback. */
};

/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   EZ mode request primitive.
 * \ingroup ZBPRO_ZHA_Functions
 * \param[in]  reqDescr    Request parameters
 * \return Nothing.
 */
void ZBPRO_ZHA_EzModeReq(ZBPRO_ZHA_EzModeReqDescr_t *reqDescr);

#endif /* _BB_ZBPRO_ZHA_SAP_EZ_MODE_H */

/* eof bbZbProZhaSapEzMode.h */