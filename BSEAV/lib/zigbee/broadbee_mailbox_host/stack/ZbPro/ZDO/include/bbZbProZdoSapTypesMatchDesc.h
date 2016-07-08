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
 *
 * FILENAME: $Workfile: trunk/stack/ZbPro/ZDO/include/bbZbProZdoSapTypesMatchDesc.h $
 *
 * DESCRIPTION:
 *   This header describes types and API for the ZDO Match Desc service.
 *
 * $Revision: 9885 $
 * $Date: 2016-02-05 15:39:57Z $
 *
 ****************************************************************************************/

#ifndef _ZBPRO_ZDO_SAP_TYPES_MATCH_DESC_H
#define _ZBPRO_ZDO_SAP_TYPES_MATCH_DESC_H

/************************* INCLUDES *****************************************************/
#include "bbZbProZdoCommon.h"

/************************* TYPES ********************************************************/

/*
 * \brief ZDO ZDP Match_Desc request parameters which correspond
 *        the appropriate fields of the Match_Desc command frame
 */
typedef struct _ZBPRO_ZDO_MatchDescReqParams_t
{
    ZBPRO_ZDO_Address_t     zdpDstAddress;

    ZBPRO_ZDO_NwkAddr_t     nwkAddrOfInterest;
    ZBPRO_ZDO_ProfileId_t   profileId;
    uint8_t                 numInClusters;
    uint8_t                 numOutClusters;
    SYS_DataPointer_t       inOutClusterList;   /* List includes input Clusters Ids from the beginning
                                                 * and output Clusters Ids after that. */
    SYS_Time_t              respWaitTimeout;    /* Response waiting timeout, in milliseconds.
                                                 * Zero means 'Use default ZDO timeout'. */
} ZBPRO_ZDO_MatchDescReqParams_t;

/**//**
 * \brief Match_Desc response list item which fields corresponds with appropriate fields of
 *        ZDP Match_Desc response frame
 */
typedef struct PACKED _ZBPRO_ZDO_MatchDescRespListItem_t
{
    ZBPRO_ZDO_NwkAddr_t     nwkAddrOfInterest;
    uint8_t                 matchLength;
    ZBPRO_ZDO_Endpoint_t    matchList[];
} ZBPRO_ZDO_MatchDescRespListItem_t;

/**//**
 * \brief ZDO ZDP Match_Desc request confirmation parameters
 */
typedef struct _ZBPRO_ZDO_MatchDescConfParams_t
{
    ZBPRO_ZDO_Status_t      status;
    SYS_DataPointer_t       responseList;       /* List of received responses as sequence of
                                                 * ZBPRO_ZDO_MatchDescRespListItem_t elements.
                                                 */
} ZBPRO_ZDO_MatchDescConfParams_t;

/**//**
 * \brief ZDO ZDP Match_Desc request descriptor prototype
 */
typedef struct _ZBPRO_ZDO_MatchDescReqDescr_t  ZBPRO_ZDO_MatchDescReqDescr_t;

/**//**
 * \brief Callback function type.
 */
typedef void ZBPRO_ZDO_MatchDescConfCallback_t(ZBPRO_ZDO_MatchDescReqDescr_t *const reqDescr,
                                                ZBPRO_ZDO_MatchDescConfParams_t *const confParams);

/**//**
 * \brief ZDO ZDP Match_Desc request descriptor
 */
struct _ZBPRO_ZDO_MatchDescReqDescr_t
{
    ZbProZdoLocalRequest_t              service;
    ZBPRO_ZDO_MatchDescConfCallback_t   *callback;
    ZBPRO_ZDO_MatchDescReqParams_t      params;
};

/************************* PROTOTYPES ***************************************************/

/*************************************************************************************//**
  \brief ZDO ZDP Match_Desc request function.
  \param[in] reqDescr - pointer to the request structure.
*****************************************************************************************/
void ZBPRO_ZDO_MatchDescReq(ZBPRO_ZDO_MatchDescReqDescr_t *const reqDescr);

#endif  /* _ZBPRO_ZDO_SAP_TYPES_MATCH_DESC_H */
