/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../inc/tr69cdefs.h"
#include "../../main/utils.h"

#include "moca2.h"

extern int getInstanceCount(TRxObjNode *n);

extern TRxObjNode mocaInterfaceDesc[];
extern TRxObjNode mocaInterfaceQoSFlowStatsDesc[];
extern TRxObjNode mocaInterfaceAssociatedDeviceDesc[];

static InstanceDesc *mocaIfIdp;
static InstanceDesc *mocaIfQoSFlowStatsIdp;
static InstanceDesc *mocaIfAssociatedDeviceIdp;

void initMoca(void)
{
	TRxObjNode *n;
	int id, counts;
    int numFlowStats;
    int numAssociatedDevice;

	/* instance of Device.MoCA.Interface.{i}. */
	n = mocaInterfaceDesc;
	id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        mocaIfIdp = getNewInstanceDesc(n, NULL, id);
    }

    /* instance of Device.MoCA.Interface.{i}.QoS.FlowStats.{i}. */
    if (moca_if_qos_flow_stats_number_of_entries(&numFlowStats) == 0)
    {
        n = mocaInterfaceQoSFlowStatsDesc;
        counts = getInstanceCount(mocaInterfaceQoSFlowStatsDesc);

        if (numFlowStats != counts)
        {
            for (id = 1; id <= counts; id++)
            {
                if(findInstanceDescNoPathCheck(n, id) != NULL)
                {
                    deleteInstanceDesc(n, id);
                }
            }
        }

        for (id = 1; id <= numFlowStats; id++)
        {
            if(findInstanceDescNoPathCheck(n, id) == NULL)
            {
                mocaIfQoSFlowStatsIdp = getNewInstanceDesc(n, mocaIfIdp, id);
            }
        }
    }

    /* instance of Device.MoCA.Interface.{i}.AssociatedDevice.{i}. */
    if (moca_if_get_associated_device_number_of_entries(&numAssociatedDevice) == 0)
    {
        n = mocaInterfaceAssociatedDeviceDesc;
        counts = getInstanceCount(mocaInterfaceAssociatedDeviceDesc);

        if (numAssociatedDevice != counts)
        {
            for (id = 1; id <= counts; id++)
            {
                if(findInstanceDescNoPathCheck(n, id) != NULL)
                {
                    deleteInstanceDesc(n, id);
                }
            }
        }

        for (id = 1; id <= numAssociatedDevice; id++)
        {
            if(findInstanceDescNoPathCheck(n, id) == NULL)
            {
                mocaIfAssociatedDeviceIdp = getNewInstanceDesc(n, mocaIfIdp, id);
            }
        }
    }

	return;
}
