/*
 * wlc_tsync.h
 *
 * Timestamp synchronization and Packet timestamping
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

/**
 * @file
 * @brief
 * Timestamp synchronization allows different clock domains on the host and
 * the device to be synchronized. Packets on ingress and egress are timestamped
 * and the timestamp information is provided in the completion work items.
 */


#ifndef _wlc_tsync_h_
#define _wlc_tsync_h_

#define MAX_FW_CLK		3		/* Max clks profiled */
#define HS_MSG_SIZE		256		/* Max message from host */
#define TSPOOL_SIZE		8		/* Packets reserved */
#define TSPOOL_PKTSIZE		256		/* Pktsize for bus layer */
#define CLK_ID_MASK		0xF0000000	/* Bitmask for clkid */
#define CLK_ID_SHIFT		28		/* Bitshift for clkid */
#define DEF_WDG_PERIOD		1		/* Time in seconds */
#define TS_GPIO_PULSE_WD	20		/* Pulse width in usec */
#define TS_CLKID_INVL		0xFFFFFFFF	/* Invalid clock identifer */

#define TS_GE(x, y)		((x) >= (y))
#define SET_LOW_16(x, y)	(x |= y)
#define SET_HIGH_16(x, y)	(x |= (y << 16))
#define IS_ENAB(clk, min, max)	((clk >= min) && (clk <= max))
#define IS_DUP_RANGE(x, y)	((x - y) < 0x80000000)

#define TSF_LOW_32(x)		(x.tsf_ticks.t_low)
#define TSF_HIGH_32(x)		(x.tsf_ticks.t_high)
#define PMU_LOW_32(x)		(x.pmu_ticks.t_low)
#define PMU_HIGH_32(x)		(x.pmu_ticks.t_high)
#define AVB_LOW_32(x)		(x.avb_ticks.t_low)
#define AVB_HIGH_32(x)		(x.avb_ticks.t_high)

#define HTCK_LOW_32(x)		(x.hst_ticks.t_low)
#define HTCK_HIGH_32(x)		(x.hst_ticks.t_high)
#define HTME_LOW_32(x)		(x.hst_time.t_low)
#define HTME_HIGH_32(x)		(x.hst_time.t_high)

#define WLC_TSYNC_DBG(data, len)	\
	do {	\
		int cnt = 0, tmp = len;	\
		uint8 *ptr = data;	\
		while (tmp--) {	\
			WL_PRINT(("%2x", *ptr));	\
			if ((tmp % 4) == 0)	\
				WL_PRINT((" "));	\
			if ((++cnt % 32) == 0)	\
				WL_PRINT(("\n"));	\
			ptr++;	\
		}	\
		WL_PRINT(("\n"));	\
	} while (0)	\

#define SET_CLKID(ts, clkid)	\
		ts &= ~CLK_ID_MASK;	\
		ts |= (clkid << CLK_ID_SHIFT);	\

typedef enum clk_id {
	CLK_PMU = 0,			/* 32 KHz PMU clock */
	CLK_TSF = 1,			/* 1 MHz TSF clock */
	CLK_AVB	= 2,			/* 160 MHz AVB clock */
	CLK_INV = 15			/* Invalid clock */
} clk_id_t;

typedef enum clk_freq {
	FREQ_PMU = 0x8000,		/* 32 * 1024 */
	FREQ_TSF = 0x100000,		/* 1 *1024 * 1024 */
	FREQ_AVB = 0xa000000		/* 160 * 1024 * 1024 */
} clk_freq_t;

typedef enum ts_src {
	SRC_FW = 0,			/* FW based collection */
	SRC_UCODE = 1			/* Microcode assisted */
} ts_src_t;

typedef enum psm_src {
	PSM_0 = 0,			/* PSM0 based collection */
	PSM_1 = 1			/* PSM1 assisted */
} psm_src_t;

typedef enum ds_event {
	DS_ENTER = 0,			/* Enter deep sleep */
	DS_EXIT = 1			/* Exit deep sleep */
} ds_event_t;

typedef enum d3_event {
	D3_ENTER = 0,			/* Enter D3 */
	D3_EXIT = 1			/* Exit D3 */
} d3_event_t;

typedef enum tsync_tmr {
	SYNC_TMR = 0,			/* Synchronization timer */
	WDG_TMR				/* Watchdog timer */
} tsync_tmr_t;

typedef enum ts_event {
	EVT_SYNC_REQ = 0,		/* Timestamp synchronization event */
	EVT_UCODE_RSP,			/* Microcode completed Timestamp capture */
	EVT_HOST_MSG,			/* Host Timestamp message received */
	EVT_WDOG_EXP			/* Watchdog expired waiting for host message */
} ts_event_t;

typedef enum ts_state {
	TS_SM_INIT = 0,			/* TS state machine initial state */
	TS_SM_START,			/* Start of SM with message from host */
	TS_WAIT_UCODE,			/* Waiting for TS response from microcode */
	TS_SENT_HOST			/* TS event sent to the host */
} ts_state_t;

typedef enum ts_errinject {
	ERR_OOO_SEQ = 0,		/* OOO sequence number */
	ERR_INV_TLVID,			/* Invalid TLV identifer */
	ERR_INV_TLVLEN,			/* Invalid TLV length */
	ERR_DROP_FWTS,			/* Drop FW TS */
	ERR_MAX_VAL			/* Last error marker */
} ts_errinject_t;

typedef enum gpio_bmap {
	GPIO_0 = (1 << 0),		/* Use GPIO0 */
	GPIO_1 = (1 << 1),		/* Use GPIO1 */
	GPIO_13 = (1 << 13)		/* Use GPIO13 */
} gpio_bmap_t;

typedef struct mac_buf {
	gpio_bmap_t gpio;		/* GPIO bitmask for PSM */
	uint16 pulse_wd;		/* Pulse width in usec */
} mac_buf_t;

typedef struct ts_clk {
	uint32 t_low;			/* Bits 31:0 of timer ticks */
	uint32 t_high;			/* Bits 63:32 of timer ticks */
} ts_clk_t;

typedef struct fw_clk {
	ts_clk_t pmu_ticks;		/* Bits 59:0 of PMU ticks */
	ts_clk_t tsf_ticks;		/* Bits 59:0 of TSF ticks */
	ts_clk_t avb_ticks;		/* Bits 59:0 of AVB ticks */
	ts_src_t ts_src;		/* TS collection source */
	uint8 clk_vld;			/* Valid clock information */
	uint8 clk_active;		/* Active clock */
} fw_clk_t;

typedef struct psm_clk {
	fw_clk_t psm_0;			/* PSM0 clock information */
	fw_clk_t psm_1;			/* PSM1 clock information */
} psm_clk_t;

typedef struct hs_clk {
	ts_clk_t hst_ticks;		/* Bits 63:0 of Host ticks */
	ts_clk_t hst_time;		/* Bits 63:0 of Host time */
} hs_clk_t;

typedef uint32 rst_cnt_t;

typedef struct tlv_info {
	uint16 tlv_id;			/* TS TLV identifier */
	uint16 tlv_len;			/* TS TLV length */
} tlv_info_t;

typedef struct ts_stats {
	uint32 wdg_cnt;			/* Watchdog expiry count */
	uint32 sync_cnt;		/* Times Sync timer exec */
	uint32 host_req;		/* Host TS request count */
	uint32 host_rsp;		/* Host TS response count */
	uint32 ucode_req;		/* Microcode request count */
	uint32 ucode_rsp;		/* Microcode response count */
	uint32 fw_req;			/* Firmware TS stat */
	uint32 tlv_invbuf;		/* Invalid TLV buffer stat */
	uint32 tlv_buflen;		/* Invalid TLV buflen stat */
	uint32 tlv_invid;		/* Invalid TLV id stat */
	uint32 tlv_invlen;		/* Invalid TLV len stat */
	uint32 sm_valid;		/* Valid SM count */
	uint32 fwclk_tag;		/* TLV fw clk count */
	uint32 hclk_tag;		/* TLV host clk count */
	uint32 clksel_tag;		/* TLV clk sel count */
	uint32 tsconf_tag;		/* TLV ts cfg count */
	uint32 tscorr_tag;		/* TLV ts correct count */
	uint32 pm_enter;		/* Times PM D3 state */
	uint32 pm_exit;			/* Times PM D0 state */
	uint32 ds_enter;		/* Times Deep sleep enter */
	uint32 ds_exit;			/* Times Deep sleep exit */
	uint32 ds_evt;			/* Deep sleep event */
	uint32 pkt_avb;			/* AVB TS in pkt */
	uint32 pkt_tsf;			/* TSF TS in pkt */
	uint32 pkt_pmu;			/* PMU TS in pkt */
	uint32 pkt_inv;			/* INV TS in pkt */
	uint32 seq_err;			/* Out of Sequence error */
	uint32 tlvid_err;		/* Invalid TLV id error */
	uint32 tlvlen_err;		/* Wrong TLV len error */
	uint32 fwts_err;		/* FW Timestamp drop error */
	uint32 mpc_upd;			/* Power on with mpc */
	uint32 pulse_wd;		/* Pulse width */
	uint32 gpio_acnt;		/* Microcode assert count */
	uint32 gpio_dcnt;		/* Deassert count */
	ts_state_t save_state;		/* SM saved state */
	int period_tmr;			/* Sync timer result */
} ts_stats_t;

struct wlc_tsync {
	wlc_info_t *wlc;		/* Back pointer to wlc */
	fw_clk_t fw_clk;		/* Device clock information */
	hs_clk_t hs_clk;		/* Host clock information */
	ts_state_t state;		/* Time sync state variable */
	uint16 rcv_seq;			/* Host sequence number */
	uint16 snd_seq;			/* Device sequence number */
	uint16 sync_period;		/* Time sync period in msec */
	uint16 wdg_period;		/* Watchdog period in sec */
	bool sync_active;		/* Sync timer active */
	bool wdg_active;		/* Watchdog timer active */
	bool avb_active;		/* AVB timer active */
	bool d3_active;			/* PM state active */
	uint8 min_clkid;		/* Lowest frequenct clk id setup by host */
	uint8 max_clkid;		/* Highest frquency clk id setup by host */
	uint8 clk_vld;			/* Available clock */
	uint8 clk_active;		/* Active clock */
	struct wl_timer	*sync_tmr;	/* Timestamp sync timer */
	struct wl_timer	*wdg_tmr;	/* Host watchdog timer */
	pktpool_t ts_pool;		/* Packet pool for fw_clk_info to host */
	ts_stats_t stats;		/* TS statistics */
	uint8 tsync_gpio;		/* TS GPIO number */
	uint8 err_inject;		/* Host injected error */
	psm_clk_t psm_clk;		/* PSM captured TS in reset */
	uint32 pmu_delta;		/* PSM difference in PMU */
	uint32 tsf_delta;		/* TSF difference in PSM */
	uint32 avb_delta;		/* AVB difference in PSM */
	rst_cnt_t fw_rst[MAX_FW_CLK];	/* Times fw reset clk */
	rst_cnt_t hs_rst;		/* Host reset counter */
	uint8 psm_vld;			/* Valid psm information */
	uint8 hs_msg[HS_MSG_SIZE];	/* Context for host message */
	uint16 hs_len;			/* Length of hs message */
	bool ds_active;			/* DS state active */
	bool tsync_mpc;			/* Enable/disable for mpc */
	bool tsync_awake;		/* PM = 0 */
	bool psm_delta;			/* Delta computation for RSDB */
};

extern wlc_tsync_t* wlc_tsync_attach(wlc_info_t *wlc);
extern void wlc_tsync_detach(wlc_tsync_t *tsync);
extern int wlc_tsync_process_ucode(wlc_tsync_t *tsync);
extern int wlc_tsync_process_host(wlc_tsync_t *tsync, void *pkt, int len);
extern void wlc_tsync_update_ts(wlc_tsync_t *tsync, void *p, uint32 clk_tsf,
		uint32 clk_avb, uint8 pkt_dir);
extern void wlc_tsync_process_pmevt(wlc_tsync_t *tsync, bool hmem_acc);
extern void wlc_tsync_process_dsevt(wlc_tsync_t *tsync, void *buf, int len);
extern void wlc_start_tsync(wlc_info_t *wlc);

#endif /* _wlc_tsync_h_ */
