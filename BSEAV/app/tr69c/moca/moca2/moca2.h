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

#ifndef __MOCA2_H__
#define __MOCA2_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MOCA_IF_STATUS_UP 1
#define MOCA_IF_STATUS_DOWN 2
#define MOCA_IF_STATUS_UNKNOWN 3
#define MOCA_IF_STATUS_DORMANT 4
#define MOCA_IF_STATUS_NOT_PRESENT 5
#define MOCA_IF_STATUS_LOWER_LAYER_DOWN 6
#define MOCA_IF_STATUS_ERROR 7

int moca_link_state_monitor_init(void);
int moca_link_state_monitor_uninit(void);

/* Device.Moca.Interface.{i}.Stats */
int moca_if_stats_bytes_sent(int *value);
int moca_if_stats_bytes_received(int *value);
int moca_if_stats_packets_sent(int *value);
int moca_if_stats_packets_received(int *value);
int moca_if_stats_errors_sent(int *value);
int moca_if_stats_errors_received(int *value);
int moca_if_stats_unicast_packets_sent(int *value);
int moca_if_stats_unicast_packets_received(int *value);
int moca_if_stats_discard_packets_sent(int *value);
int moca_if_stats_discard_packets_received(int *value);
int moca_if_stats_multicast_packets_sent(int *value);
int moca_if_stats_multicast_packets_received(int *value);
int moca_if_stats_broadcast_packets_sent(int *value);
int moca_if_stats_broadcast_packets_received(int *value);
/* int moca_if_stats_unknown_proto_packets_received(int *value); *//* NOT SUPPORT */

/* Device.Ethernet.Interface.{i}. */
int moca_if_get_enable(int *value);
int moca_if_set_enable(int value);
int moca_if_get_status(int *value);
int moca_if_get_name(char *value);
int moca_if_get_last_change(int *value);
int moca_if_get_mac_addr(unsigned char *value);
int moca_if_get_firmware_version(char *value);
int moca_if_get_max_bitrate(int *value);
int moca_if_get_max_ingress_bandwidth(int *value);
int moca_if_get_max_egress_bandwidth(int *value);
int moca_if_get_highest_version(char *value);
int moca_if_get_current_version(char *value);
int moca_if_get_network_coordinator(int *value);
int moca_if_get_node_id(int *value);
int moca_if_get_preferred_nc(int *value);
int moca_if_set_preferred_nc(int value);
int moca_if_get_backup_nc(int *value);
int moca_if_get_privacy_enabled_setting(int *value);
int moca_if_set_privacy_enabled_setting(int value);
int moca_if_get_privacy_enabled(int *value);
int moca_if_get_freq_capability_mask(int *value);
int moca_if_get_freq_current_mask_setting(int *value);
int moca_if_set_freq_current_mask_setting(int value);
int moca_if_get_freq_current_mask(int *value);
int moca_if_get_current_operational_frequency(int *value);
int moca_if_get_last_operational_frequency(int *value);
int moca_if_get_key_passphase(char *value);
int moca_if_set_key_passphase(char *value);
int moca_if_get_max_tx_power(int *value);
int moca_if_set_max_tx_power(int value);
int moca_if_get_power_conrtol_target_phy_rate(int *value);
int moca_if_set_power_conrtol_target_phy_rate(int value);
int moca_if_get_beacon_power_reduction(int *value);
int moca_if_set_beacon_power_reduction(int value);
int moca_if_get_network_taboo_mask(int *value);
int moca_if_get_node_taboo_mask(int *value);
int moca_if_get_tx_broadcast_phy_rate(int *value);
int moca_if_get_tx_broadcast_power_reduction(int *value);
int moca_if_get_qam256_capable(int *value);
int moca_if_get_packet_aggregation_capability (int *value);
int moca_if_get_associated_device_number_of_entries(int *value);

/* Device.Moca.Interface.{i}.AssociatedDevice.{i}. */
int moca_if_get_associated_device_mac_address(int instance_id, unsigned char *value);
int moca_if_get_associated_device_node_id(int instance_id, int *value);
int moca_if_get_associated_device_preferred_nc(int instance_id, int *value);
int moca_if_get_associated_device_phy_tx_rate(int instance_id, int *value);
int moca_if_get_associated_device_phy_rx_rate(int instance_id, int *value);
int moca_if_get_associated_device_tx_power_control_reduction(int instance_id, int *value);
int moca_if_get_associated_device_rx_power_level(int instance_id, int *value);
int moca_if_get_associated_device_tx_broadcast_rate(int instance_id, int *value);
int moca_if_get_associated_device_rx_broadcast_power_level(int instance_id, int *value);
int moca_if_get_associated_device_tx_packets(int instance_id, int *value);
int moca_if_get_associated_device_rx_packets(int instance_id, int *value);
int moca_if_get_associated_device_rx_errored_and_missed_packets(int instance_id, int *value);
int moca_if_get_associated_device_qam256_capable(int instance_id, int *value);
int moca_if_get_associated_device_packet_aggregation_capability(int instance_id, int *value);
int moca_if_get_associated_device_rx_snr(int instance_id, int *value);
/* int moca_if_get_associated_device_active(int instance_id, int *value); *//* NOT SUPPORT */

/* Device.Moca.Interface.{i}.QoS. */
int moca_if_qos_egress_num_flows(int *value);
int moca_if_qos_ingress_num_flows(int *value);
int moca_if_qos_flow_stats_number_of_entries(int *value);

/* Device.Moca.Interface.{i}.QoS.FlowStats.{i}. */
int moca_if_qos_flow_stats_flow_id(int instance_id, unsigned char *value);
int moca_if_qos_flow_stats_packet_da(int instance_id, unsigned char *value);
int moca_if_qos_flow_stats_max_rate(int instance_id, int *value);
int moca_if_qos_flow_stats_max_burst_size(int instance_id, int *value);
int moca_if_qos_flow_stats_lease_time(int instance_id, int *value);
int moca_if_qos_flow_stats_lease_time_left(int instance_id, int *value);
/* int moca_if_qos_flow_stats_flow_packets(int instance_id, int *value); *//* NOT SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /*__MOCA2_H__ */
