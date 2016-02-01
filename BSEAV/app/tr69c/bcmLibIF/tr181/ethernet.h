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

#ifndef __ETHERNET_H__
#define __ETHERNET_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ETHERNET_IF_STATUS_UP 1
#define ETHERNET_IF_STATUS_DOWN 2
#define ETHERNET_IF_STATUS_UNKNOWN 3
#define ETHERNET_IF_STATUS_DORMANT 4
#define ETHERNET_IF_STATUS_NOT_PRESENT 5
#define ETHERNET_IF_STATUS_LOWER_LAYER_DOWN 6
#define ETHERNET_IF_STATUS_ERROR 7

int ethernet_link_state_monitor_init(void);
int ethernet_link_state_monitor_uninit(void);

/* Device.Ethernet.Interface. */

int ethernet_if_get_number_of_entries(int *value);

/* Device.Ethernet.Interface.{i}. */

int ethernet_if_get_enable(int id, int *value);
int ethernet_if_set_enable(int id, int value);
int ethernet_if_get_status(int id, int *value);
int ethernet_if_get_name(int id, char *value);
int ethernet_if_get_last_change(int id, int *value);
int ethernet_if_get_lower_layers(int id, int *value);
int ethernet_if_set_lower_layers(int id, int value);
int ethernet_if_get_upstream(int id, int *value);
int ethernet_if_get_mac_addr(int id, char *value);
int ethernet_if_get_max_bitrate(int id, int *value);
int ethernet_if_set_max_bitrate(int id, int value);
int ethernet_if_get_current_bitrate(int id, int *value);
int ethernet_if_get_duplex_mode(int id, char *value);
int ethernet_if_set_duplex_mode(int id, char *value);
int ethernet_if_get_eee_capability(int id, int *value);
int ethernet_if_get_eee_enable(int id, int *value);
int ethernet_if_set_eee_enable(int id, int value);

/* Device.Ethernet.Interface.{i}.Stats. */

int ethernet_if_stats_bytes_sent(int id, int *value);
int ethernet_if_stats_bytes_received(int id, int *value);
int ethernet_if_stats_packets_sent(int id, int *value);
int ethernet_if_stats_packets_received(int id, int *value);
int ethernet_if_stats_errors_sent(int id, int *value);
int ethernet_if_stats_errors_received(int id, int *value);
int ethernet_if_stats_unicast_packets_sent(int id, int *value);
int ethernet_if_stats_unicast_packets_received(int id, int *value);
int ethernet_if_stats_discard_packets_sent(int id, int *value);
int ethernet_if_stats_discard_packets_received(int id, int *value);
int ethernet_if_stats_multicast_packets_sent(int id, int *value);
int ethernet_if_stats_multicast_packets_received(int id, int *value);
int ethernet_if_stats_broadcast_packets_sent(int id, int *value);
int ethernet_if_stats_broadcast_packets_received(int id, int *value);
int ethernet_if_stats_unknown_proto_packets_received(int id, int *value);

#ifdef __cplusplus
}
#endif

#endif /*__ETHERNET_H__ */
