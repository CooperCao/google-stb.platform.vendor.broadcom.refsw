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
#include <net/if.h>

#include "../../inc/tr69cdefs.h"
#include "../../main/utils.h"

#include "moca2.h"

extern int getInstanceCount(TRxObjNode *n);

extern TRxObjNode mocaInterfaceDesc[];
extern TRxObjNode mocaInterfaceQoSFlowStatsDesc[];
extern TRxObjNode mocaInterfaceAssociatedDeviceDesc[];

/* Device.Moca.Interface.{i}.AssociatedDevice.{i}. */

TRX_STATUS getMocaInterfaceAssociatedDeviceMACAddress(char **value)
{
	InstanceDesc *idp;
    unsigned char macaddr[6];
	char mac_addr[18];

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_associated_device_mac_address(idp->instanceID, macaddr) != 0)
		return TRX_ERR;

	sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
		macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);

	*value = strdup(mac_addr);

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDeviceNodeID(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_associated_device_node_id(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDevicePreferredNC(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_associated_device_preferred_nc(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDeviceHighestVersion(char **value)
{
	InstanceDesc *idp;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    *value = strdup("2.0");

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDevicePHYTxRate(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_associated_device_phy_tx_rate(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDevicePHYRxRate(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_associated_device_phy_rx_rate(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDeviceTxPowerControlReduction(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_associated_device_tx_power_control_reduction(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDeviceRxPowerLevel(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_associated_device_rx_power_level(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDeviceTxBcastRate(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_associated_device_tx_broadcast_rate(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDeviceRxBcastPowerLevel(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_associated_device_rx_broadcast_power_level(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDeviceTxPackets(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_associated_device_tx_packets(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDeviceRxPackets(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_associated_device_rx_packets(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDeviceRxErroredAndMissedPackets(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_associated_device_rx_errored_and_missed_packets(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDeviceQAM256Capable(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_associated_device_qam256_capable(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDevicePacketAggregationCapability(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_associated_device_packet_aggregation_capability(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDeviceRxSNR(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_associated_device_rx_snr(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

#if 0 /* NOT SUPPORT */
TRX_STATUS getMocaInterfaceAssociatedDeviceActive(char **value)
{
	InstanceDesc *idp;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    *value = strdup("1");

	return TRX_OK;
}
#endif

/* Device.Moca.Interface.{i}.AssociatedDevice. */

TRX_STATUS addMocaInterfaceAssociatedDeviceInstance(char **value)
{
	InstanceDesc *idp;

	if ((idp = getNewInstanceDesc(getCurrentNode(), getCurrentInstanceDesc(), 0)))
	{
        idp->hwUserData = NULL;
        *value = strdup(itoa(idp->instanceID));
        return TRX_OK;
	}

	return TRX_ERR;
}

TRX_STATUS deleteMocaInterfaceAssociatedDeviceInstance(char *value)
{
	TRxObjNode *n;
	InstanceDesc *idp;
	int id = atoi(value);

	if ((idp = findInstanceDesc(n=getCurrentNode(), id)))
	{
		if (!deleteInstanceDesc(n, id))
		{
			return TRX_OK;
		}
	}

	return TRX_ERR;
}

/* Device.Moca.Interface.{i}.QoS.FlowStats.{i}. */

TRX_STATUS getMocaInterfaceQoSFlowStatsFlowID(char **value)
{
	InstanceDesc *idp;
    unsigned char macaddr[6];
	char mac_addr[18];

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_qos_flow_stats_flow_id(idp->instanceID, macaddr) != 0)
		return TRX_ERR;

	sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
		macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);

	*value = strdup(mac_addr);

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceQoSFlowStatsPacketDA(char **value)
{
	InstanceDesc *idp;
    unsigned char macaddr[6];
	char mac_addr[18];

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_qos_flow_stats_packet_da(idp->instanceID, macaddr) != 0)
		return TRX_ERR;

	sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
		macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);

	*value = strdup(mac_addr);

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceQoSFlowStatsMaxRate(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_qos_flow_stats_max_rate(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceQoSFlowStatsMaxBurstSize(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_qos_flow_stats_max_burst_size(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceQoSFlowStatsLeaseTime(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_qos_flow_stats_lease_time(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceQoSFlowStatsLeaseTimeLeft(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_qos_flow_stats_lease_time_left(idp->instanceID, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

#if 0 /* NOT SUPPORT */
TRX_STATUS getMocaInterfaceQoSFlowStatsFlowPackets(char **value)
{
	InstanceDesc *idp;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    *value = strdup("1");

	return TRX_OK;
}
#endif

/* Device.Moca.Interface.{i}.QoS.FlowStats. */

TRX_STATUS addMocaInterfaceQoSFlowStatsInstance(char **value)
{
	InstanceDesc *idp;

	if ((idp = getNewInstanceDesc(getCurrentNode(), getCurrentInstanceDesc(), 0)))
	{
        idp->hwUserData = NULL;
        *value = strdup(itoa(idp->instanceID));
        return TRX_OK;
	}

	return TRX_ERR;
}

TRX_STATUS deleteMocaInterfaceQoSFlowStatsInstance(char *value)
{
	TRxObjNode *n;
	InstanceDesc *idp;
	int id = atoi(value);

	if ((idp = findInstanceDesc(n=getCurrentNode(), id)))
	{
		if (!deleteInstanceDesc(n, id))
		{
			return TRX_OK;
		}
	}

	return TRX_ERR;
}

/* Device.Moca.Interface.{i}.QoS. */

TRX_STATUS getMocaInterfaceQoSEgressNumFlows(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_qos_egress_num_flows(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceQoSIngressNumFlows(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_qos_ingress_num_flows(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceQoSFlowStatsNumberOfEntries(char **value)
{
	int num = getInstanceCount(mocaInterfaceQoSFlowStatsDesc);

	*value = strdup(itoa(num));

	return TRX_OK;
}

/* Device.Moca.Interface.{i}.Stats. */

TRX_STATUS getMocaInterfaceStatsBytesSent(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_stats_bytes_sent(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceStatsBytesReceived(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_stats_bytes_received(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceStatsPacketsSent(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_stats_packets_sent(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceStatsPacketsReceived(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_stats_packets_received(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceStatsErrorsSent(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_stats_errors_sent(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceStatsErrorsReceived(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_stats_errors_received(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceStatsUnicastPacketsSent(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_stats_unicast_packets_sent(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceStatsUnicastPacketsReceived(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_stats_unicast_packets_received(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceStatsDiscardPacketsSent(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_stats_discard_packets_sent(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceStatsDiscardPacketsReceived(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_stats_discard_packets_received(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceStatsMulticastPacketsSent(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_stats_multicast_packets_sent(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceStatsMulticastPacketsReceived(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_stats_multicast_packets_received(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceStatsBroadcastPacketsSent(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_stats_broadcast_packets_sent(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceStatsBroadcastPacketsReceived(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_stats_broadcast_packets_received(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

#if 0 /* NOT SUPPORT */
TRX_STATUS getMocaInterfaceStatsUnknownProtoPacketsReceived(char **value)
{
	InstanceDesc *idp;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    *value = strdup("1");

	return TRX_OK;
}
#endif

/* Device.Moca.Interface.{i}. */

TRX_STATUS getMocaInterfaceEnable(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_enable(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS setMocaInterfaceEnable(char *value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_enable(&val) != 0)
		return TRX_ERR;

	if (strcmp(value, "1") == 0)
	{
        if (val != 1)
        {
            if (!moca_if_set_enable(1))
            {
                return TRX_OK;
            }
            else return TRX_ERR;
        }
	}
	else if (strcmp(value, "0") == 0)
	{
        if (val != 0)
        {
            if (!moca_if_set_enable(0))
            {
                return TRX_OK;
            }
            else return TRX_ERR;
        }
	}
    else
    {
        return TRX_ERR;
    }

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceStatus(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_status(&val) != 0)
		return TRX_ERR;

	switch (val)
	{
		case MOCA_IF_STATUS_UP:
			*value = strdup("Up");
			break;
		case MOCA_IF_STATUS_DOWN:
			*value = strdup("Down");
			break;
		case MOCA_IF_STATUS_UNKNOWN:
			*value = strdup("Unknown");
			break;
		case MOCA_IF_STATUS_DORMANT:
			*value = strdup("Dormant");
			break;
		case MOCA_IF_STATUS_NOT_PRESENT:
			*value = strdup("Not Present");
			break;
		case MOCA_IF_STATUS_LOWER_LAYER_DOWN:
			*value = strdup("Lower Layer Down");
			break;
		case MOCA_IF_STATUS_ERROR:
		default:
			*value = strdup("Error");
			break;
	}

	return TRX_OK;
}

static char mocaIfAlias[64]="BRCM-MoCA-2.0";

TRX_STATUS getMocaInterfaceAlias(char **value)
{
	InstanceDesc *idp;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	*value = strdup(mocaIfAlias);

	return TRX_OK;
}

TRX_STATUS setMocaInterfaceAlias(char *value)
{
	strcpy(mocaIfAlias, value);
	return TRX_OK;
}

TRX_STATUS getMocaInterfaceName(char **value)
{
	InstanceDesc *idp;
    char val[16];

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_name(val) != 0)
		return TRX_ERR;

	*value = strdup(val);

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceLastChange(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_last_change(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceLowerLayers(char **value)
{
	InstanceDesc *idp;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	*value = strdup("N/A"); /* This doesn't apply to our system */

	return TRX_OK;
}

/* LowerLayers is not settable on this system */
TRX_STATUS setMocaInterfaceLowerLayers(char *value)
{
	(void) value; /* This doesn't apply to our system */
	return TRX_OK;
}

TRX_STATUS getMocaInterfaceUpstream(char **value)
{
	InstanceDesc *idp;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	*value = strdup("1"); /* for an End Device, Upstream will be true for all interfaces */

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceMACAddress(char **value)
{
	InstanceDesc *idp;
    unsigned char macaddr[6];
	char mac_addr[18];

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_mac_addr(macaddr) != 0)
		return TRX_ERR;

	sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
		macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);

	*value = strdup(mac_addr);

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceMaxBitRate(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_max_bitrate(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceFirmwareVersion(char **value)
{
	InstanceDesc *idp;
    char val[32];

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_firmware_version(val) != 0)
		return TRX_ERR;

	*value = strdup(val);

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceMaxIngressBW(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_max_ingress_bandwidth(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceMaxEgressBW(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_max_egress_bandwidth(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceHighestVersion(char **value)
{
	InstanceDesc *idp;
    char val[16];

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_highest_version(val) != 0)
		return TRX_ERR;

	*value = strdup(val);

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceCurrentVersion(char **value)
{
	InstanceDesc *idp;
    char val[16];

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_current_version(val) != 0)
		return TRX_ERR;

	*value = strdup(val);

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceNetworkCoordinator(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_network_coordinator(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceNodeID(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_node_id(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceMaxNodes(char **value)
{
	*value = strdup("1"); /* always true */
	return TRX_OK;
}

TRX_STATUS setMocaInterfacePreferredNC(char *value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_preferred_nc(&val) != 0)
		return TRX_ERR;

	if (strcmp(value, "1") == 0)
	{
        if (val != 1)
        {
            if (moca_if_set_preferred_nc(1) != 0)
                return TRX_ERR;
        }
	}
	else if (strcmp(value, "0") == 0)
	{
        if (val != 0)
        {
            if (moca_if_set_preferred_nc(0) != 0)
                return TRX_ERR;
        }
	}
    else
    {
        return TRX_ERR;
    }

	return TRX_OK;
}

TRX_STATUS getMocaInterfacePreferredNC(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_preferred_nc(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceBackupNC(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_backup_nc(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS setMocaInterfacePrivacyEnabledSetting(char *value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_privacy_enabled_setting(&val) != 0)
		return TRX_ERR;

	if (strcmp(value, "1") == 0)
	{
        if (val != 1)
        {
            if (moca_if_set_privacy_enabled_setting(1) != 0)
                return TRX_ERR;
        }
	}
	else if (strcmp(value, "0") == 0)
	{
        if (val != 0)
        {
            if (moca_if_set_privacy_enabled_setting(0) != 0)
                return TRX_ERR;
        }
	}
    else
    {
        return TRX_ERR;
    }

	return TRX_OK;
}

TRX_STATUS getMocaInterfacePrivacyEnabledSetting(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_privacy_enabled_setting(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfacePrivacyEnabled(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_privacy_enabled(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceFreqCapabilityMask(char **value)
{
	InstanceDesc *idp;
    int val;
    char mask[9];

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_freq_capability_mask(&val) != 0)
		return TRX_ERR;

    sprintf(mask, "%08X", val);

	*value = strdup(mask);

	return TRX_OK;
}

TRX_STATUS setMocaInterfaceFreqCurrentMaskSetting(char *value)
{
	InstanceDesc *idp;
    int val, val2;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    val = strtol(value, NULL, 16);

	if (moca_if_get_freq_current_mask_setting(&val2) != 0)
		return TRX_ERR;

    if (val != val2)
    {
        if (moca_if_set_freq_current_mask_setting(val) != 0)
            return TRX_ERR;
    }

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceFreqCurrentMaskSetting(char **value)
{
	InstanceDesc *idp;
    int val;
    char mask[9];

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_freq_current_mask_setting(&val) != 0)
		return TRX_ERR;

    sprintf(mask, "%08X", val);

	*value = strdup(mask);

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceFreqCurrentMask(char **value)
{
	InstanceDesc *idp;
    int val;
    char mask[9];

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_freq_current_mask(&val) != 0)
		return TRX_ERR;

    sprintf(mask, "%08X", val);

	*value = strdup(mask);

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceCurrentOperFreq(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_current_operational_frequency(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceLastOperFreq(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_last_operational_frequency(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS setMocaInterfaceKeyPassphrase(char *value)
{
	InstanceDesc *idp;
    char val[33] = { 0 };

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_key_passphase(val) != 0)
		return TRX_ERR;

    if (strcmp(value, val) != 0)
    {
        if (moca_if_set_key_passphase(value) != 0)
            return TRX_ERR;
    }

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceKeyPassphrase(char **value)
{
	InstanceDesc *idp;
    char val[33] = { 0 };

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_key_passphase(val) != 0)
		return TRX_ERR;

	*value = strdup(val);

	return TRX_OK;
}

TRX_STATUS setMocaInterfaceTxPowerLimit(char *value)
{
	InstanceDesc *idp;
    int val, val2;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    val = strtol(value, NULL, 10);

	if (moca_if_get_max_tx_power(&val2) != 0)
		return TRX_ERR;

    if (val != val2)
    {
        if (moca_if_set_max_tx_power(val) != 0)
            return TRX_ERR;
    }

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceTxPowerLimit(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_max_tx_power(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS setMocaInterfacePowerCntlPhyTarget(char *value)
{
	InstanceDesc *idp;
    int val, val2;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    val = strtol(value, NULL, 10);

	if (moca_if_get_power_conrtol_target_phy_rate(&val2) != 0)
		return TRX_ERR;

    if (val != val2)
    {
        if (moca_if_set_power_conrtol_target_phy_rate(val) != 0)
            return TRX_ERR;
    }

	return TRX_OK;
}

TRX_STATUS getMocaInterfacePowerCntlPhyTarget(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_power_conrtol_target_phy_rate(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS setMocaInterfaceBeaconPowerLimit(char *value)
{
	InstanceDesc *idp;
    int val, val2;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    val = strtol(value, NULL, 10);

	if (moca_if_get_beacon_power_reduction(&val2) != 0)
		return TRX_ERR;

    if (val != val2)
    {
        if (moca_if_set_beacon_power_reduction(val) != 0)
            return TRX_ERR;
    }

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceBeaconPowerLimit(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_beacon_power_reduction(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceNetworkTabooMask(char **value)
{
	InstanceDesc *idp;
    int val;
    char mask[9];

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_network_taboo_mask(&val) != 0)
		return TRX_ERR;

    sprintf(mask, "%08X", val);

	*value = strdup(mask);

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceNodeTabooMask(char **value)
{
	InstanceDesc *idp;
    int val;
    char mask[9];

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_node_taboo_mask(&val) != 0)
		return TRX_ERR;

    sprintf(mask, "%08X", val);

	*value = strdup(mask);

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceTxBcastRate(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_tx_broadcast_phy_rate(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceTxBcastPowerReduction(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_tx_broadcast_power_reduction(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceQAM256Capable(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_qam256_capable(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfacePacketAggregationCapability(char **value)
{
	InstanceDesc *idp;
    int val;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	if (moca_if_get_packet_aggregation_capability(&val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getMocaInterfaceAssociatedDeviceNumberOfEntries(char **value)
{
    int num = getInstanceCount(mocaInterfaceAssociatedDeviceDesc);

    *value = strdup(itoa(num));

    return TRX_OK;
}

/* Device.Moca.Interface. */

TRX_STATUS addMocaInterfaceInstance(char **value)
{
	InstanceDesc *idp;

	if ((idp = getNewInstanceDesc(getCurrentNode(), getCurrentInstanceDesc(), 0)))
	{
        idp->hwUserData = NULL;
        *value = strdup(itoa(idp->instanceID));
        return TRX_OK;
	}

	return TRX_ERR;
}

TRX_STATUS deleteMocaInterfaceInstance(char *value)
{
	TRxObjNode *n;
	InstanceDesc *idp;
	int id = atoi(value);

	if ((idp = findInstanceDesc(n=getCurrentNode(), id)))
	{
		if (!deleteInstanceDesc(n, id))
		{
			return TRX_OK;
		}
	}

	return TRX_ERR;
}

/* Device.Moca. */

TRX_STATUS getMocaInterfaceNumberOfEntries(char **value)
{
	int num = getInstanceCount(mocaInterfaceDesc);

	*value = strdup(itoa(num));

	return TRX_OK;
}
