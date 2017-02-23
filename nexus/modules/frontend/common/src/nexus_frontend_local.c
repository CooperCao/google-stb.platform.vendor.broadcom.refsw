/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
***************************************************************************/
#include "bstd.h"
#include "nexus_frontend.h"

BDBG_MODULE(nexus_frontend_local);

/***************************************************************************
Summary:
    Get the dvbt2 asynchronous status.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbt2AsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbt2StatusType type,
    NEXUS_FrontendDvbt2Status *pStatus
    )
{
    NEXUS_Error  rc = NEXUS_SUCCESS;

    switch ( type )
    {
        case NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Pre:
        case NEXUS_FrontendDvbt2StatusType_eFecStatisticsL1Post:
        case NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpA:
        case NEXUS_FrontendDvbt2StatusType_eFecStatisticsPlpB:
            rc = NEXUS_Frontend_GetDvbt2AsyncFecStatistics(handle, type, &pStatus->status.fecStatistics);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        case NEXUS_FrontendDvbt2StatusType_eL1Pre:
            rc = NEXUS_Frontend_GetDvbt2AsyncL1PreStatus(handle, &pStatus->status.l1Pre);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        case NEXUS_FrontendDvbt2StatusType_eL1PostConfigurable:
            rc = NEXUS_Frontend_GetDvbt2AsyncL1PostConfigurableStatus(handle, &pStatus->status.l1PostConfigurable);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        case NEXUS_FrontendDvbt2StatusType_eL1PostDynamic:
            rc = NEXUS_Frontend_GetDvbt2AsyncPostDynamicStatus(handle, &pStatus->status.l1PostDynamic);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        case NEXUS_FrontendDvbt2StatusType_eL1Plp:
            rc = NEXUS_Frontend_GetDvbt2AsyncL1PlpStatus(handle, &pStatus->status.l1Plp);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        case NEXUS_FrontendDvbt2StatusType_eBasic:
            rc = NEXUS_Frontend_GetDvbt2AsyncBasicStatus(handle, &pStatus->status.basic);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        default:
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
    }
done:
    return rc;
}

/***************************************************************************
Summary:
    Get the dvbc2 asynchronous status.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2StatusType type,
    NEXUS_FrontendDvbc2Status *pStatus
    )
{
    NEXUS_Error  rc = NEXUS_SUCCESS;

    switch ( type )
    {
        case NEXUS_FrontendDvbc2StatusType_eFecStatisticsL1Part2:
        case NEXUS_FrontendDvbc2StatusType_eFecStatisticsPlpA:
        case NEXUS_FrontendDvbc2StatusType_eFecStatisticsPlpB:
            rc = NEXUS_Frontend_GetDvbc2AsyncFecStatistics(handle, type, &pStatus->status.fecStatistics);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        case NEXUS_FrontendDvbc2StatusType_eL1Part2:
            rc = NEXUS_Frontend_GetDvbc2AsyncL1Part2Status(handle, &pStatus->status.l1Part2);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        case NEXUS_FrontendDvbc2StatusType_eL1Dslice:
            rc = NEXUS_Frontend_GetDvbc2AsyncL1DsliceStatus(handle, &pStatus->status.l1Dslice);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        case NEXUS_FrontendDvbc2StatusType_eL1Notch:
            rc = NEXUS_Frontend_GetDvbc2AsyncL1NotchStatus(handle, &pStatus->status.l1Notch);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        case NEXUS_FrontendDvbc2StatusType_eL1Plp:
            rc = NEXUS_Frontend_GetDvbc2AsyncL1PlpStatus(handle, &pStatus->status.l1Plp);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        case NEXUS_FrontendDvbc2StatusType_eBasic:
            rc = NEXUS_Frontend_GetDvbc2AsyncBasicStatus(handle, &pStatus->status.basic);
            if(rc){rc = BERR_TRACE(rc); goto done;}
            break;
        default:
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
    }
done:
    return rc;
}
