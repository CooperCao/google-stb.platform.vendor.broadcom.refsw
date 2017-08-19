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
*       ZHA CIE device user policy definitions.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZHA_CIE_COMMAND_PULL_H_
#define _BB_ZBPRO_ZHA_CIE_COMMAND_PULL_H_

/************************* INCLUDES *****************************************************/
#include "bbZbProZdoSapTypesMatchDesc.h"
#include "bbZbProZdoSapTypesDiscoveryManager.h"
#include "bbZbProZclSapProfileWideAttributes.h"
#include "bbZbProZclSapClusterIasZone.h"
#include "bbZbProZhaCommon.h"


typedef union
{
    ZBPRO_ZDO_MatchDescReqDescr_t                       match;
    ZBPRO_ZDO_AddrResolvingReqDescr_t                   resolv;
    ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t         write;
    ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t          read;
    ZBPRO_ZCL_IASZoneCmdZoneEnrollResponseReqDescr_t    enrollResp;
} ZbProZhaCieCmd_t;

typedef struct
{
    ZbProZhaCieCmd_t    table[ZBPRO_ZHA_CIE_CMD_PULL_SIZE];
    Bool8_t             isBusy[ZBPRO_ZHA_CIE_CMD_PULL_SIZE];
} ZbProZhaCieCmdPullDescriptor_t;

/************************* PROTOTYPES ***************************************************/
void zbProZhaCieCmdPullReset(void);
ZbProZhaCieCmd_t *zbProZhaCieCmdPullGet(void);
void zbProZhaCieCmdPullFree(ZbProZhaCieCmd_t *cmd);

#endif /* _BB_ZBPRO_ZHA_CIE_COMMAND_PULL_H_ */

/* eof bbZbProZhaCieCmdPull.h */