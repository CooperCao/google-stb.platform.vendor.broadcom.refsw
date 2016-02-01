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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/bbZbProNwkSapTypesData.h $
*
* DESCRIPTION:
*   Network Layer Data Entity declarations
*
* $Revision: 2385 $
* $Date: 2014-05-14 08:41:03Z $
*
****************************************************************************************/

#ifndef _ZBPRO_NWK_DATA_H
#define _ZBPRO_NWK_DATA_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkCommon.h"

/************************* TYPES *******************************************************/

/**//**
 * \brief NLME-DATA request parameters structure. See ZigBee Spec r20, 3.2.1.1.
 */
typedef struct _ZBPRO_NWK_DataReqParams_t
{
    ZBPRO_NWK_AddrMode_t    dstAddrMode;
    ZBPRO_NWK_NwkAddr_t     dstAddr;
    SYS_DataPointer_t       payload;
    uint8_t                 nsduHandle;
    uint8_t                 radius;
    uint8_t                 nonmemberRadius;
    Bool8_t                 discoverRoute;
    Bool8_t                 securityEnable;
} ZBPRO_NWK_DataReqParams_t;


/**//**
 * \brief NLME-DATA confirm parameters structure. See ZigBee Spec r20, 3.2.1.2.
 */
typedef struct _ZBPRO_NWK_DataConfParams_t
{
    ZBPRO_NWK_Status_t      status;
    uint8_t                 nsduHandle;
} ZBPRO_NWK_DataConfParams_t;

/**//**
 * \brief NLME-DATA.request descriptor data type declaration.
 */
typedef struct _ZBPRO_NWK_DataReqDescr_t  ZBPRO_NWK_DataReqDescr_t;

/**//**
 * \brief NLME-DATA confirm primitive callback function type
 */
typedef void (*ZBPRO_NWK_DataConfCallback_t)(ZBPRO_NWK_DataReqDescr_t *const reqDescr,
                                             ZBPRO_NWK_DataConfParams_t *const conf);

/**//**
 * \brief NLME-DATA.request descriptor data type definition.
 */
struct _ZBPRO_NWK_DataReqDescr_t
{
    ZbProNwkServiceField_t          service;
    ZBPRO_NWK_DataReqParams_t       params;
    ZBPRO_NWK_DataConfCallback_t    callback;
};

/**//**
 * \brief NLDE-DATA indication parameters structure. See ZigBee Spec r20, 3.2.1.3.
 */
typedef struct _ZBPRO_NWK_DataIndParams_t
{
    ZBPRO_NWK_AddrMode_t    dstAddrMode;
    ZBPRO_NWK_NwkAddr_t     dstAddr;
    ZBPRO_NWK_NwkAddr_t     srcAddr;
    SYS_DataPointer_t       payload;
    ZBPRO_NWK_Lqi_t         linkQuality;
    bool                    secured;
} ZBPRO_NWK_DataIndParams_t;

/************************* PROTOTYPES **************************************************/

/************************************************************************************//**
    \brief NLME-DATA request primitive function.
    \param[in] reqDescr - pointer to the request structure.
****************************************************************************************/
NWK_PUBLIC void ZBPRO_NWK_DataReq(ZBPRO_NWK_DataReqDescr_t *const req);

/************************************************************************************//**
    \brief NLME-DATA indication primitive function.
    \param[in] indParams - pointer to the indication parameters.
****************************************************************************************/
extern void ZBPRO_NWK_DataInd(ZBPRO_NWK_DataIndParams_t *const ind);

#endif /* _ZBPRO_NWK_DATA_H */