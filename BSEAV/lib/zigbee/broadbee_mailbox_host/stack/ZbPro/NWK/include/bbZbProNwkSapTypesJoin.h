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
*       Network Layer Management Entity Join primitive declarations
*
*******************************************************************************/

#ifndef _ZBPRO_NWK_SAP_JOIN_H
#define _ZBPRO_NWK_SAP_JOIN_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkCommon.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief NLME-JOIN request primitive structure, see
 * ZigBee Specification r20, 3.2.2.11
 * \ingroup ZBPRO_NWK_JoinReq
 */
typedef struct _ZBPRO_NWK_JoinReqParams_t
{
    ZBPRO_NWK_ExtPanId_t    extendedPANId;         /*!< 64-bit PANid of the network */
    BitField32_t            scanChannels;          /*!< Scan channels bitfield (The 27 least significant bits) */
    ZBPRO_NWK_RejoinMethod_t rejoinNetwork;        /*!< Method of joining */
    uint8_t                 scanDuration;          /*!< Scan channel duration */
    ZBPRO_NWK_Capability_t  capabilityInformation; /*!< The operating capabilities of the device */
    Bool8_t                 securityEnable;        /*!< True if security is enabled */
    Bool8_t                 performDiscovery;      /*!< \note commonly it's always true */
} ZBPRO_NWK_JoinReqParams_t;

/**//**
 * \brief NLME-JOIN confirm primitive structure, see
 * ZigBee Specification r20, 3.2.2.13
 * \ingroup ZBPRO_NWK_JoinConf
 */
typedef struct _ZBPRO_NWK_JoinConfParams_t
{
    ZBPRO_NWK_ExtPanId_t    extendedPANId;         /*!< 64-bit PANid of the network */
    ZBPRO_NWK_NwkAddr_t     networkAddress;        /*!< 16-bit short address allocated or 0xffff if failed */
    PHY_Channel_t           activeChannel;         /*!< phyCurrentChannel attribute value */
    ZBPRO_NWK_Status_t      status;                /*!< Corresponding request status */
} ZBPRO_NWK_JoinConfParams_t;

/**//**
 * \brief NLME-JOIN request descriptor prototype.
 * \ingroup ZBPRO_NWK_JoinReq
 */
typedef struct _ZBPRO_NWK_JoinReqDescr_t ZBPRO_NWK_JoinReqDescr_t;

/**//**
 * \brief NLME-JOIN confirm primitive callback function type.
 * \ingroup ZBPRO_NWK_JoinConf
 */
typedef void (*ZBPRO_NWK_JoinCallback_t)(ZBPRO_NWK_JoinReqDescr_t *const reqDescr,
        ZBPRO_NWK_JoinConfParams_t *const confParams);

/**//**
 * \brief NLME-JOIN request descriptor type.
 * \ingroup ZBPRO_NWK_JoinReq
 */
struct _ZBPRO_NWK_JoinReqDescr_t
{
    ZbProNwkServiceField_t      service;           /*!< Request service field */
    ZBPRO_NWK_JoinReqParams_t   params;            /*!< Request parameters */
    ZBPRO_NWK_JoinCallback_t    callback;          /*!< Confirmation callback */
};

/**//**
 * \brief NLME-JOIN indication primitive structure, see
 * ZigBee Specification r20, 3.2.2.12
 * \ingroup ZBPRO_NWK_JoinInd
 */
typedef struct _ZBPRO_NWK_JoinIndParams_t
{
    ZBPRO_NWK_NwkAddr_t     networkAddress;        /*!< 16-bit short address allocated of 0xffff if failed */
    ZBPRO_NWK_ExtAddr_t     extendedAddress;       /*!< 64-bit extended address of the entity */
    ZBPRO_NWK_Capability_t  capabilityInformation; /*!< The operating capabilities of the device */
    ZBPRO_NWK_RejoinMethod_t rejoinNetwork;        /*!< Method of joining */
    bool                    secureRejoin;          /*!< True if security is enabled */
} ZBPRO_NWK_JoinIndParams_t;

/**//**
 * \brief NLME-JOIN indication primitive prototype.
 * \ingroup ZBPRO_NWK_JoinInd
 */
typedef void (ZBPRO_NWK_JoinIndCallback_t)(ZBPRO_NWK_JoinIndParams_t *const indParams);

/************************* PROTOTYPES **************************************************/

/**//**
 * \brief NLME-JOIN request primitive function
 * \ingroup ZBPRO_NWK_Functions
 *
 * \param[in] req - pointer to the request structure.
 * \return Nothing.
 */
NWK_PUBLIC void ZBPRO_NWK_JoinReq(ZBPRO_NWK_JoinReqDescr_t *req);

/**//**
 * \brief NLME-JOIN indication primitive function
 * \ingroup ZBPRO_NWK_Functions
 *
 * \return Nothing.
 */
NWK_PUBLIC ZBPRO_NWK_JoinIndCallback_t ZBPRO_NWK_JoinInd;

#endif /* _ZBPRO_NWK_SAP_JOIN_H */

/* eof bbZbProNwkSapTypesJoin.h */