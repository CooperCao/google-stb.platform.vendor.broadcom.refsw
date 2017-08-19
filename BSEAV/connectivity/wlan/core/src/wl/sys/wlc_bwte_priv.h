/*
* BT WLAN TUNNEL ENGINE private header file
*
* Broadcom Proprietary and Confidential. Copyright (C) 2017,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*
* $Id$
*
*/
#ifndef _WLC_BWTE_PRIV_H
#define _WLC_BWTE_PRIV_H

/* #define BWTE_FULLLOG */

#ifdef BWTE_FULLLOG
#ifdef WL_ERROR
#undef WL_ERROR
#endif
#define WL_ERROR(x) printf x

#ifdef WL_TRACE
#undef WL_TRACE
#endif
#define WL_TRACE(x) printf x
#endif /* BWTE_FULLLOG */

#define FIXED_LOCCATION_MAGIC 0xCAFEBABE
#define BT_ADDR_OFFSET	0x19000000

#define BT_CBL_ADDR	0x0020a32c
#define BT_TEST_MEM	0x0026cbc0

#ifdef BWTE_DEBUG_ISR
#define GPIO_ISR_IN	10
#define GPIO_ISR_OUT	9
#endif

typedef enum {
	BWTE_GPIO_TRIGGER_LEVEL_LOW = 0,
	BWTE_GPIO_TRIGGER_LEVEL_HIGH = 1,
	BWTE_GPIO_TRIGGER_EDGE_FALLING = 2,
	BWTE_GPIO_TRIGGER_EDGE_RISING = 3,
	BWTE_GPIO_TRIGGER_EDGE_BOTH = 4
} bwte_gpio_trigger_type_t;

typedef struct _bwte_gpio {
	void* addr;
	uint16 trigger_type;
	uint16 trigger_bit;
} bwte_gpio_t;

typedef struct {
	void*	msg;
	uint16	msg_len;
	uint16	msg_inuse;
	wlc_bwte_cb free_func;
	void*	arg;
} bwte_ctlchan_t;

typedef struct {
	void* p;
	uint32 len;
	wlc_bwte_cb free_func;
	void* arg;
} bwte_pkt_t;

#define DATA_PKT_SLOT_CNT 8
typedef struct {
	uint16 wr_idx;
	uint16 rd_idx;
	uint16 free_idx;
	uint16 nodes_cnt;
	bwte_pkt_t pkts[DATA_PKT_SLOT_CNT];
} bwte_datachan_t;

typedef struct {
	bwte_ctlchan_t bt2wlan_ctlchan;
	bwte_ctlchan_t wlan2bt_ctlchan;

	bwte_datachan_t bt2wlan_aclchan;
	bwte_datachan_t wlan2bt_aclchan;

	bwte_datachan_t bt2wlan_scochan;
	bwte_datachan_t wlan2bt_scochan;
} bwte_client_shared_block;

#define BWTE_MAX_CLIENT_CNT 8
#define BWTE_SHARED_BLCOK_VERSION 0x2
typedef struct {
	uint32 magic_num;
	uint16 version;
	uint16 length;
	bwte_gpio_t btgpio;
	bwte_gpio_t wlangpio;
	uint32 max_clients;
	bwte_client_shared_block** csb; /* [BWTE_MAX_CLIENT_CNT] */
} bwte_shared_block;

typedef struct {
	wlc_bwte_cb ctl_func;
	wlc_bwte_cb lo_data_func;
	wlc_bwte_cb hi_data_func;
	void* arg;
} bwte_client_info_t;

struct bwte_info {
	/* init one time */
	bwte_shared_block *sb;
	osl_t* osh;
	wlc_obj_registry_t* objr;
	si_t* sih;
	bwte_client_info_t *client; /* [BWTE_MAX_CLIENT_CNT] */
	int client_cnt;

	void *gci_event_handle; /* gci event interrupt handle */
	bool inttobt_pending;
	bwte_gci_intstate_t inttobt;
	bwte_gci_intstate_t intfrombt;
	bwte_stats_t *stats;

#ifdef BWTE_DEBUG_ISR
	uint32 wlan_isr_level;
#endif
};

#endif /* _WLC_BWTE_PRIV_H */
