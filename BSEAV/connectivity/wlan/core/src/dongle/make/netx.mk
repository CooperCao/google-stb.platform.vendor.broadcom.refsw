# NetX basic configuration

# Netx files
NETX_OBJECTS := \
	nx_arp_enable.o \
	nx_arp_entry_allocate.o \
	nx_arp_packet_receive.o \
	nx_arp_packet_send.o \
	nx_arp_periodic_update.o \
	nx_arp_queue_process.o \
	nx_icmp_cleanup.o \
	nx_icmp_enable.o \
	nx_icmp_packet_process.o \
	nx_icmp_packet_receive.o \
	nx_icmp_queue_process.o \
	nx_icmpv4_packet_process.o \
	nx_ip_checksum_compute.o \
	nx_ip_create.o \
	nx_ip_delete.o \
	nx_ip_delete_queue_clear.o \
	nx_ip_dispatch_process.o \
	nx_ip_fast_periodic_timer_entry.o \
	nx_ip_fragment_assembly.o \
	nx_ip_fragment_disable.o \
	nx_ip_fragment_enable.o \
	nx_ip_fragment_packet.o \
	nx_ip_fragment_timeout_check.o \
	nx_ip_initialize.o \
	nx_ip_packet_deferred_receive.o \
	nx_ip_packet_receive.o \
	nx_ip_packet_send.o \
	nx_ip_periodic_timer_entry.o \
	nx_ip_route_find.o \
	nx_ip_thread_entry.o \
	nx_ipv4_packet_receive.o \
	nx_packet_allocate.o \
	nx_packet_copy.o \
	nx_packet_data_append.o \
	nx_packet_pool_cleanup.o \
	nx_packet_pool_initialize.o \
	nx_packet_release.o \
	nx_packet_transmit_release.o \
	nx_system_initialize.o \
	nx_tcp_cleanup_deferred.o \
	nx_tcp_client_bind_cleanup.o \
	nx_tcp_connect_cleanup.o \
	nx_tcp_deferred_cleanup_check.o \
	nx_tcp_disconnect_cleanup.o \
	nx_tcp_enable.o \
	nx_tcp_fast_periodic_processing.o \
	nx_tcp_initialize.o \
	nx_tcp_mss_option_get.o \
	nx_tcp_no_connection_reset.o \
	nx_tcp_packet_process.o \
	nx_tcp_packet_receive.o \
	nx_tcp_packet_send_ack.o \
	nx_tcp_packet_send_fin.o \
	nx_tcp_packet_send_rst.o \
	nx_tcp_packet_send_syn.o \
	nx_tcp_periodic_processing.o \
	nx_tcp_queue_process.o \
	nx_tcp_receive_cleanup.o \
	nx_tcp_socket_connection_reset.o \
	nx_tcp_socket_packet_process.o \
	nx_tcp_socket_receive_queue_flush.o \
	nx_tcp_socket_state_ack_check.o \
	nx_tcp_socket_state_closing.o \
	nx_tcp_socket_state_data_check.o \
	nx_tcp_socket_state_established.o \
	nx_tcp_socket_state_fin_wait1.o \
	nx_tcp_socket_state_fin_wait2.o \
	nx_tcp_socket_state_last_ack.o \
	nx_tcp_socket_state_syn_received.o \
	nx_tcp_socket_state_syn_sent.o \
	nx_tcp_socket_state_transmit_check.o \
	nx_tcp_socket_thread_resume.o \
	nx_tcp_socket_transmit_queue_flush.o \
	nx_tcp_transmit_cleanup.o \
	nx_igmp_multicast_check.o \
	nx_ip_raw_packet_cleanup.o \
	nx_ip_raw_packet_send.o \
	nx_ip_raw_packet_source_send.o \
	nx_ip_raw_receive_queue_max_set.o \
	nx_ipv6_util.o \
	nx_ipv6_process_fragment_option.o \
	nx_ipv6_process_hop_by_hop_option.o \
	nx_ipv6_process_routing_option.o \
	nx_nd_cache_init.o \
	nx_icmpv6_send_error_message.o \
	nx_icmpv6_send_ns.o \
	nx_ipv6_fragment_process.o \
	nx_ipv6_option_error.o \
	nx_ipv6_packet_send.o \
	nxd_ipv6_interface_find.o \
	nxd_ipv6_router_lookup.o \
	nxd_ipv6_search_onlink.o \
	nx_ipv6_packet_copy.o \
	nxd_ipv6_find_max_prefix_length.o \
	nx_icmpv6_perform_DAD.o \
	nxd_ipv6_router_solicitation_check.o \
	nxd_ipv6_prefix_router_timer_tick.o \
	nx_icmpv6_send_rs.o \
	nx_ipv6_prefix_list_delete_entry.o \
	nx_ipv6_address_find_prefix.o \
	nx_icmpv6_DAD_clear_NDCache_entry.o \
	nx_ipv6_multicast.o \
	nx_packet_pool_create.o \
	nx_ip_interface_mtu_set.o \
	nx_ip_interface_physical_address_set.o \
	nx_ip_interface_address_mapping_configure.o \
	nx_arp_packet_deferred_receive.o \
	nxd_http_client.o

#below objects are exluded files. keeping them seperate.
NETX_EXT_OBJECTS := \
	nxd_nd_cache_hardware_address_find.o \
	nx_icmpv6_process_redirect.o \
	nx_nd_cache_find_entry_by_mac_addr.o \
	nxd_nd_cache_ip_address_find.o \
	nxd_nd_cache_invalidate_internal.o \
	nx_icmpv6_process_na.o \
	nx_icmpv6_destination_table_periodic_update.o \
	nx_icmpv6_dest_table_add.o \
	nx_nd_cache_fast_periodic_update.o \
	nx_icmpv6_dest_table_find.o \
	nx_nd_cache_clear_IsRouter_field.o \
	nx_nd_cache_slow_periodic_update.o \
	nx_nd_cache_add.o \
	nx_nd_cache_find_entry.o \
	nx_icmpv6_process_ra.o \
	nx_icmpv6_process_ns.o \
	nxd_nd_cache_invalidate.o \
	nx_nd_cache_delete_internal.o \
	nxd_nd_cache_entry_delete.o \
	nx_invalidate_destination_entry.o \
	nxd_ipv6_destination_table_find_next_hop.o \
	nx_nd_cache_add_entry.o \
	nxd_icmp_enable.o

# Add the excluded fileas as well to the main stream NETX objects
NETX_OBJECTS += $(NETX_EXT_OBJECTS)

# Define Cfiles and AS files to filter while compiling and assembling
CFILES_NETX = $(NETX_OBJECTS:%.o=%.c)
ASFILES_NETX = $(NETX_OBJECTS:%.o=%.S)

# Flag to suppress the warning shown as error in netx library
NETX_NOWERR	= -Wno-error

# Add the Objects to the RTOS objects which get added along with the threadx objects
RTOS_OBJECTS += $(NETX_OBJECTS)
EXTRA_DFLAGS += -DNX_DISABLE_ERROR_CHECKING

# Application specific flags to not to use Filex
EXTRA_DFLAGS += -DNX_HTTP_NO_FILEX

# Base path for NETX files
NETXBASE := $(SRCBASE)/../components/netx

vpath %.c $(NETXBASE)
vpath %.c $(NETXBASE)/netx_bsd_layer
vpath %.c $(NETXBASE)/netx_applications
vpath %.c $(NETXBASE)/netx_applications/http

vpath %.S $(NETXBASE)
vpath %.S $(NETXBASE)/netx_bsd_layer
vpath %.S $(NETXBASE)/netx_applications
vpath %.S $(NETXBASE)/netx_applications/http

EXTRA_IFLAGS += -I$(NETXBASE)
EXTRA_IFLAGS += -I$(NETXBASE)/netx_bsd_layer
EXTRA_IFLAGS += -I$(NETXBASE)/netx_applications
EXTRA_IFLAGS += -I$(NETXBASE)/netx_applications/http
