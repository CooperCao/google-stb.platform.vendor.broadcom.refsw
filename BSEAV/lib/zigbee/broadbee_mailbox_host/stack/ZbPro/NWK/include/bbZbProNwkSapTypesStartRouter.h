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
*       Network Layer Management Entity Start Router primitive declarations
*
*******************************************************************************/

#ifndef _ZBPRO_NWK_SAP_TYPES_START_ROUTER_H
#define _ZBPRO_NWK_SAP_TYPES_START_ROUTER_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkCommon.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief NLME-START-ROUTER confirm primitive structure. See ZigBee Spec r20, 3.2.2.8.
 * \ingroup ZBPRO_NWK_StartRouterConf
 */
typedef struct _ZBPRO_NWK_StartRouterConfParams_t
{
    ZBPRO_NWK_Status_t status;                    /*!< The result of the attempt to initialize a ZigBee router */
} ZBPRO_NWK_StartRouterConfParams_t;

/**//**
 * \brief NLME-START-ROUTER.request descriptor data type declaration.
 * \ingroup ZBPRO_NWK_StartRouterReq
 */
typedef struct _ZBPRO_NWK_StartRouterReqDescr_t ZBPRO_NWK_StartRouterReqDescr_t;

/**//**
 * \brief NLME-START-ROUTER confirm primitive callback function type
 * \ingroup ZBPRO_NWK_StartRouterConf
 */
typedef void (*ZBPRO_NWK_StartRouterConfCallback_t)(ZBPRO_NWK_StartRouterReqDescr_t *reqDescr,
                                                    ZBPRO_NWK_StartRouterConfParams_t *conf);

/**//**
 * \brief NLME-START-ROUTER request descriptor definition structure.
 * \note Parameters of the request primitive described in ZigBee Spec r20, 3.2.2.7
 *  are NOT SUPPORTED. It includes: BeaconOrder, SuperframeOrder and BatteryLifeExtension
 * \ingroup ZBPRO_NWK_StartRouterReq
 */
typedef struct _ZBPRO_NWK_StartRouterReqDescr_t
{
    ZbProNwkServiceField_t              service;  /*!< Service field */
    ZBPRO_NWK_StartRouterConfCallback_t callback; /*!< Confirmation callback */
} ZBPRO_NWK_StartRouterReqDescr_t;

/************************* PROTOTYPES **************************************************/

/************************************************************************************//**
  \brief NLME-START-ROUTER request primitive function.
  \ingroup ZBPRO_NWK_Functions

  \param[in] req - pointer to the request descriptor.
  \return Nothing.
 ***************************************************************************************/
void ZBPRO_NWK_StartRouterReq(ZBPRO_NWK_StartRouterReqDescr_t *req);

#endif /* _ZBPRO_NWK_SAP_TYPES_START_ROUTER_H */

/* eof bbZbProNwkSapTypesStartRouter.h */