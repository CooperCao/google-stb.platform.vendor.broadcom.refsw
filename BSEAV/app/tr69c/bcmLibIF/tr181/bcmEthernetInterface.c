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

#include "ethernet.h"

extern int getInstanceCount(TRxObjNode *n);

extern TRxObjNode ethernetInterfaceInstanceDesc[];

/* Device.Ethernet.Interface.{i}.Stats. */

TRX_STATUS getEthernetInterfaceStatsBytesSent(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_stats_bytes_sent(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceStatsBytesReceived(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_stats_bytes_received(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceStatsPacketsSent(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_stats_packets_sent(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceStatsPacketsReceived(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_stats_packets_received(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceStatsErrorsSent(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_stats_errors_sent(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceStatsErrorsReceived(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_stats_errors_received(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceStatsUnicastPacketsSent(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_stats_unicast_packets_sent(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceStatsUnicastPacketsReceived(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_stats_unicast_packets_received(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceStatsDiscardPacketsSent(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_stats_discard_packets_sent(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceStatsDiscardPacketsReceived(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_stats_discard_packets_received(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceStatsMulticastPacketsSent(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_stats_multicast_packets_sent(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceStatsMulticastPacketsReceived(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_stats_multicast_packets_received(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceStatsBroadcastPacketsSent(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_stats_broadcast_packets_sent(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceStatsBroadcastPacketsReceived(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_stats_broadcast_packets_received(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceStatsUnknownProtoPacketsReceived(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_stats_unknown_proto_packets_received(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

/* Device.Ethernet.Interface.{i}. */

TRX_STATUS getEthernetInterfaceEnable(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_get_enable(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS setEthernetInterfaceEnable(char *value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_get_enable(id, &val) != 0)
		return TRX_ERR;

	if (strcmp(value, "1") == 0)
	{
        if (val != 1)
        {
            if (!ethernet_if_set_enable(id, 1))
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
            if (!ethernet_if_set_enable(id, 0))
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

TRX_STATUS getEthernetInterfaceStatus(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_get_status(id, &val) != 0)
		return TRX_ERR;

	switch (val)
	{
		case ETHERNET_IF_STATUS_UP:
			*value = strdup("Up");
			break;
		case ETHERNET_IF_STATUS_DOWN:
			*value = strdup("Down");
			break;
		case ETHERNET_IF_STATUS_UNKNOWN:
			*value = strdup("Unknown");
			break;
		case ETHERNET_IF_STATUS_DORMANT:
			*value = strdup("Dormant");
			break;
		case ETHERNET_IF_STATUS_NOT_PRESENT:
			*value = strdup("Not Present");
			break;
		case ETHERNET_IF_STATUS_LOWER_LAYER_DOWN:
			*value = strdup("Lower Layer Down");
			break;
		case ETHERNET_IF_STATUS_ERROR:
		default:
			*value = strdup("Error");
			break;
	}

	return TRX_OK;
}

static char ethernetIfAlias[64]="Ethernet-Interface";

TRX_STATUS getEthernetInterfaceAlias(char **value)
{
	InstanceDesc *idp;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

	*value = strdup(ethernetIfAlias);

	return TRX_OK;
}

TRX_STATUS setEthernetInterfaceAlias(char *value)
{
	strcpy(ethernetIfAlias, value);
	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceName(char **value)
{
	InstanceDesc *idp;
    char val[16];
    int id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_get_name(id, val) != 0)
		return TRX_ERR;

	*value = strdup(val);

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceLastChange(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_get_last_change(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceLowerLayers(char **value)
{
	*value = strdup("N/A"); /* This doesn't apply to our system */
	return TRX_OK;
}

/* LowerLayers is not settable on this system */
TRX_STATUS setEthernetInterfaceLowerLayers(char *value)
{
	(void) value; /* This doesn't apply to our system */
	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceUpstream(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_get_upstream(id, &val) != 0)
		return TRX_ERR;

	*value = strdup("1"); /* for an End Device, Upstream will be true for all interfaces */

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceMACAddress(char **value)
{
	InstanceDesc *idp;
	char macaddr[18];
    int id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_get_mac_addr(id, macaddr) != 0)
		return TRX_ERR;

	*value = strdup(macaddr);

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceMaxBitRate(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_get_max_bitrate(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS setEthernetInterfaceMaxBitRate(char *value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
    val = strtol(value, NULL, 10);

	if (ethernet_if_set_max_bitrate(id, val) != 0)
		return TRX_ERR;

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceCurrentBitRate(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_get_current_bitrate(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceDuplexMode(char **value)
{
	InstanceDesc *idp;
    char val[10];
    int id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_get_duplex_mode(id, val) != 0)
		return TRX_ERR;

    if (strcmp(val, "half") == 0)
    {
        *value = strdup("Half");
    }
    else if (strcmp(val, "full") == 0)
    {
        *value = strdup("Full");
    }
    else /* if (strcmp(val, "auto") == 0) */
    {
        *value = strdup("Auto");
    }

	return TRX_OK;
}

TRX_STATUS setEthernetInterfaceDuplexMode(char *value)
{
	InstanceDesc *idp;
    char val[10];
    int id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_get_duplex_mode(id, val) != 0)
		return TRX_ERR;

	if (strcmp(value, "Half") == 0)
	{
        if (strcmp(val, "half") != 0)
        {
            if (!ethernet_if_set_duplex_mode(id, "half"))
            {
                return TRX_OK;
            }
            else return TRX_ERR;
        }
	}
	else if (strcmp(value, "Full") == 0)
	{
        if (strcmp(val, "full") != 0)
        {
            if (!ethernet_if_set_duplex_mode(id, "full"))
            {
                return TRX_OK;
            }
            else return TRX_ERR;
        }
	}
	else /* if (strcmp(value, "Auto") == 0) */
	{
        if (strcmp(val, "auto") != 0)
        {
            if (!ethernet_if_set_duplex_mode(id, "auto"))
            {
                return TRX_OK;
            }
            else return TRX_ERR;
        }
	}

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceEEECapability(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_get_eee_capability(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS getEthernetInterfaceEEEEnable(char **value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_get_eee_enable(id, &val) != 0)
		return TRX_ERR;

	*value = strdup(itoa(val));

	return TRX_OK;
}

TRX_STATUS setEthernetInterfaceEEEEnable(char *value)
{
	InstanceDesc *idp;
    int val, id;

	if ((idp = getCurrentInstanceDesc()) == NULL)
		return TRX_ERR;

    id = idp->instanceID - 1;
	if (ethernet_if_get_eee_enable(id, &val) != 0)
		return TRX_ERR;

	if (strcmp(value, "1") == 0)
	{
        if (val != 1)
        {
            if (!ethernet_if_set_eee_enable(id, 1))
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
            if (!ethernet_if_set_eee_enable(id, 0))
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

/* Device.Ethernet.Interface. */

TRX_STATUS addEthernetInterfaceInstance(char **value)
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

TRX_STATUS deleteEthernetInterfaceInstance(char *value)
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

/* Device.Ethernet. */

TRX_STATUS getEthernetInterfaceNumberOfEntries(char **value)
{
	int num = getInstanceCount(ethernetInterfaceInstanceDesc);

	*value = strdup(itoa(num));

	return TRX_OK;
}

TRX_STATUS getEthernetLinkNumberOfEntries(char **value)
{
	int num = getInstanceCount(ethernetInterfaceInstanceDesc);

	*value = strdup(itoa(num));

	return TRX_OK;
}

TRX_STATUS getEthernetVLANTerminationNumberOfEntries(char **value)
{
	int num = 0;

	*value = strdup(itoa(num));

	return TRX_OK;
}

TRX_STATUS getEthernetRMONStatsNumberOfEntries(char **value)
{
	int num = 0;

	*value = strdup(itoa(num));

	return TRX_OK;
}
