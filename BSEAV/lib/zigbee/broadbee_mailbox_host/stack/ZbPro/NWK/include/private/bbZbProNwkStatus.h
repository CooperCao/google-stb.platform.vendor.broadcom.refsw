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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkStatus.h $
*
* DESCRIPTION:
*   Contains declaration of link status service.
*
* $Revision: 3955 $
* $Date: 2014-10-08 12:45:05Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_STATUS_H
#define _ZBPRO_NWK_STATUS_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkSapTypesNetworkStatus.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief The default confirmation of a transmission command type.
 */
typedef struct _ZbProNwkStatusConfParams_t
{
    ZBPRO_NWK_Status_t status;
} ZbProNwkStatusConfParams_t;

/**//**
 * \brief Network status payload type. (ZigBee spec r20 3.4.3 p.323)
 */
typedef struct _ZbProNwkStatusPayload_t
{
    ZBPRO_NWK_NetworkStatusCode_t   code;
    ZBPRO_NWK_NwkAddr_t             addr;
} ZbProNwkStatusPayload_t;

/**//**
 * \brief Network status request descriptor type.
 */
typedef struct _ZbProNwkStatusReqParams_t
{
    /* Next hop destination address shall be equal to ZBPRO_NWK_INVALID_SHORT_ADDR if unused. */
    ZBPRO_NWK_NwkAddr_t             nextHopAddr;
    /* Status message destination address */
    ZBPRO_NWK_NwkAddr_t             dstAddr;
    /* Address into message */
    ZBPRO_NWK_NwkAddr_t             targetAddr;
    /* The status code */
    ZBPRO_NWK_NetworkStatusCode_t   errorCode;
} ZbProNwkStatusReqParams_t;

/**//**
 * \brief Network status request descriptor prototype.
 */
typedef struct _ZbProNwkStatusReqDescr_t ZbProNwkStatusReqDescr_t;

/**//**
 * \brief Network status callback function type.
 */
typedef void (*ZbProNwkStatusConfCallback_t)(ZbProNwkStatusReqDescr_t *const reqDescr,
        ZbProNwkStatusConfParams_t *const conf);

/**//**
 * \brief Network status request descriptor type.
 */
struct _ZbProNwkStatusReqDescr_t
{
    SYS_QueueElement_t elem;
    void *appropriateBuffer;
    ZbProNwkStatusReqParams_t params;
    ZbProNwkStatusConfCallback_t callback;
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/**************************************************************************//**
  \brief Request to send the network status command.
  \param[in] req - pointer to the network status request descriptor.
******************************************************************************/
NWK_PRIVATE void zbProNwkStatusReq(ZbProNwkStatusReqDescr_t *const req);

#endif /* _ZBPRO_NWK_STATUS_H */