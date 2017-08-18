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
*       Network Layer Management Entity Permit Joining primitive declarations
*
*******************************************************************************/

#ifndef _ZBPRO_NWK_SAP_TYPES_PERMIT_JOINING_H
#define _ZBPRO_NWK_SAP_TYPES_PERMIT_JOINING_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkCommon.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Special value of Permit duration parameter. ZigBee spec r20 3.2.2.5.3.
 */
#define ZBPRO_NWK_PERMIT_JOINING_PERMANENTLY    0xFF

/**//**
 * \brief Special value of Permit duration parameter. ZigBee spec r20 3.2.2.5.3.
 */
#define ZBPRO_NWK_FORBID_JOINING_PERMANENTLY    0x00

/************************* TYPES *******************************************************/
/**//**
 * \brief NLME-PERMIT-JOINING request parameters. ZigBee Spec r20 3.2.2.5.
 * \ingroup ZBPRO_NWK_PermitJoinReq
 */
typedef struct _ZBPRO_NWK_PermitJoiningReqParams_t
{
    ZBPRO_NWK_PermitJoinDuration_t permitDuration;
    /*!< The length of time in seconds during which the ZigBee coordinator or
     * router will allow associations. The value 0x00 and 0xff indicate that
     * permission is disabled or enabled, respectively, without a specified
     * time limit. */
} ZBPRO_NWK_PermitJoiningReqParams_t;

/**//**
 * \brief NLME-PERMIT-JOINING confirmation parameters. ZigBee Spec r20 3.2.2.6.
 * \ingroup ZBPRO_NWK_PermitJoinConf
 */
typedef struct _ZBPRO_NWK_PermitJoiningConfParams_t
{
    ZBPRO_NWK_Status_t status;                          /*!< The status of the corresponding request */
} ZBPRO_NWK_PermitJoiningConfParams_t;

/**//**
 * \brief NLME-PERMIT-JOINING.request descriptor data type declaration.
 * \ingroup ZBPRO_NWK_PermitJoinReq
 */
typedef struct _ZBPRO_NWK_PermitJoiningReqDescr_t  ZBPRO_NWK_PermitJoiningReqDescr_t;

/**//**
 * \brief NLME-PERMIT-JOINING.confirm primitive callback function type.
 * \ingroup ZBPRO_NWK_PermitJoinConf
 */
typedef void (*ZBPRO_NWK_PermitJoiningConfCallback_t)(ZBPRO_NWK_PermitJoiningReqDescr_t *const reqDescr,
        ZBPRO_NWK_PermitJoiningConfParams_t *const conf);

/**//**
 * \brief NLME-PERMIT-JOINING.request descriptor data type definition.
 * \ingroup ZBPRO_NWK_PermitJoinReq
 */
struct _ZBPRO_NWK_PermitJoiningReqDescr_t
{
    ZbProNwkServiceField_t                    service;  /*!< Service field */
    ZBPRO_NWK_PermitJoiningReqParams_t        params;   /*!< Request parameters */
    ZBPRO_NWK_PermitJoiningConfCallback_t     callback; /*!< Confirmation callback */
};

/************************* PROTOTYPES **************************************************/

/************************************************************************************//**
  \brief NLME-PERMIT-JOINING request primitive function
  \ingroup ZBPRO_NWK_Functions

  \param[in] reqDescr - pointer to the request descriptor structure
  \return Nothing.
****************************************************************************************/
void ZBPRO_NWK_PermitJoiningReq(ZBPRO_NWK_PermitJoiningReqDescr_t *reqDescr);

#endif /* _ZBPRO_NWK_SAP_TYPES_PERMIT_JOINING_H */

/* eof bbZbProNwkSapTypesPermitJoining.h */