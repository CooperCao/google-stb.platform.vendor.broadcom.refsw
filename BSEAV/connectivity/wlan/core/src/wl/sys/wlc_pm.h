/*
 * Power Management Mode PM_FAST (PM2) interface
 *
 *   Broadcom Proprietary and Confidential. Copyright (C) 2017,
 *   All Rights Reserved.
 *   
 *   This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 *   the contents of this file may not be disclosed to third parties, copied
 *   or duplicated in any form, in whole or in part, without the prior
 *   written permission of Broadcom.
 *
 *
 *   <<Broadcom-WL-IPTag/Proprietary:>>
 *
 *   $Id$
 */

/** Twiki: [WlDriverPowerSave] */

#ifndef _wlc_pm_h_
#define _wlc_pm_h_

#ifdef STA

/* PM2 Fast Return to Sleep */
extern void wlc_pm2_enter_ps(wlc_bsscfg_t *cfg);
extern void wlc_pm2_sleep_ret_timeout_cb(void *arg);
extern void wlc_pm2_sleep_ret_timeout(wlc_bsscfg_t *cfg);
extern void wlc_pm2_ret_upd_last_wake_time(wlc_bsscfg_t *cfg, uint32* tsf_l);
extern void wlc_pm2_sleep_ret_timer_start(wlc_bsscfg_t *cfg, uint period);
extern void wlc_pm2_sleep_ret_timer_stop(wlc_bsscfg_t *cfg);
extern void wlc_pm2_sleep_ret_timeout_cb(void *arg);
extern int wlc_pm2_start_ps(wlc_bsscfg_t *cfg);

/* Dynamic PM2 Fast Return To Sleep */
extern void wlc_dfrts_reset_counters(wlc_bsscfg_t *bsscfg);
extern void wlc_update_sleep_ret(wlc_bsscfg_t *bsscfg, bool inc_rx, bool inc_tx,
	uint rxbytes, uint txbytes);

/* PM2 Receive Throttle Duty Cycle */
#if defined(WL_PM2_RCV_DUR_LIMIT)
extern void wlc_pm2_rcv_timeout_cb(void *arg);
extern void wlc_pm2_rcv_stagger_timeout_cb(void *arg);
extern void wlc_pm2_rcv_timeout(wlc_bsscfg_t *cfg);
extern void wlc_pm2_rcv_timer_start(wlc_bsscfg_t *cfg);
extern void wlc_pm2_rcv_timer_stop(wlc_bsscfg_t *cfg);
extern void wlc_pm2_rcv_stagger_timer_start(wlc_bsscfg_t *cfg);
#else
#define wlc_pm2_rcv_timer_stop(cfg)
#define wlc_pm2_rcv_timer_start(cfg)
#define wlc_pm2_rcv_stagger_timer_start(cfg)
#endif /* WL_PM2_RCV_DUR_LIMIT */

#ifdef WL_EXCESS_PMWAKE
extern uint32 wlc_get_roam_ms(wlc_info_t *wlc);
extern uint32 wlc_get_pfn_ms(wlc_info_t *wlc);
extern void wlc_generate_pm_alert_event(wlc_info_t *wlc, uint32 reason, void *data, uint32 datalen);
extern void wlc_check_roam_alert_thresh(wlc_info_t *wlc);
extern void wlc_check_excess_pm_awake(wlc_info_t *wlc);
extern void wlc_epm_roam_time_upd(wlc_info_t *wlc, uint32 connect_dur);
extern void wlc_reset_epm_ca(wlc_info_t *wlc);
extern void wlc_reset_epm_dur(wlc_info_t *wlc);
#endif /* WL_EXCESS_PMWAKE */

#endif	/* STA */

extern int wlc_update_pm_history(bool state, void *caller);


/* ******** WORK-IN-PROGRESS ******** */

/* TODO: make data structures oqaque */

/** PM=2 receive duration duty cycle states */
typedef enum _pm2rd_state_t {
	PM2RD_IDLE,		/**< In an idle DTIM period with no data to receive and
				 * no receive duty cycle active.  ie. the last
				 * received beacon had a cleared TIM bit.
				 */
	PM2RD_WAIT_BCN,		/**< In the OFF part of the receive duty cycle.
				 * In PS mode, waiting for next beacon.
				 */
	PM2RD_WAIT_TMO,		/**< In the ON part of the receive duty cycle.
				 * Out of PS mode, waiting for pm2_rcv timeout.
				 */
	PM2RD_WAIT_RTS_ACK	/**< Transitioning from the ON part to the OFF part of
				 * the receive duty cycle.
				 * Started entering PS mode, waiting for a
				 * PM-indicated ACK from AP to complete entering
				 * PS mode.
				 */
} pm2rd_state_t;

/* defines for ps mode requestor module id */
#define WLC_BSSCFG_PS_REQ_CHANSW	(0x00000001)
#define WLC_BSSCFG_PS_REQ_AWDL		(0x00000002)

/* Maximum number of immediate PM1 Null tx retries if no ack received */
#define PM_IMMED_RETRIES_MAX 2		/**< 2 means 3 total tx attempts with 3 seq#'s */

/* Mode of operation for PM1 Null resends */
#define PS_RESEND_MODE_WDOG_ONLY	0	/**< Legacy behavior: Resend at watchdog only */
#define PS_RESEND_MODE_BCN_NO_SLEEP	1	/**< Resend immed 2x, then at bcn rx and at wdog.
						 * Do not sleep while waiting for the next beacon.
						 */
#define PS_RESEND_MODE_BCN_SLEEP	2	/**< Resend immed 2x, then at bcn rx and at wdog.
						 * Sleep while waiting for the next beacon.
						 */
#define PS_RESEND_MODE_MAX		3

/** per bsscfg power management states */
struct wlc_pm_st {
	/* states */
	uint8	PM;			/**< power-management mode (CAM, PS or FASTPS) */
	bool	PM_override;		/**< no power-save flag, override PM(user input) */
	mbool	PMenabledModuleId;	/**< module id that enabled pm mode */
	bool	PMenabled;		/**< current power-management state (CAM or PS) */
	bool	PMawakebcn;		/**< bcn recvd during current waking state */
	bool	PMpending;		/**< waiting for tx status with PM indicated set */
	bool	priorPMstate;		/**< Detecting PM state transitions */
	bool	PSpoll;			/**< whether there is an outstanding PS-Poll frame */
	bool	check_for_unaligned_tbtt;	/**< check unaligned tbtt */

	/* periodic polling */
	uint16	pspoll_prd;		/**< pspoll interval in milliseconds, 0 == disable */
	struct wl_timer *pspoll_timer;	/**< periodic pspoll timer */
	uint16	apsd_trigger_timeout;	/**< timeout value for apsd_trigger_timer (in ms)
					 * 0 == disable
					 */
	struct wl_timer *apsd_trigger_timer;	/**< timer for wme apsd trigger frames */
	struct wl_timer *pm2_rcv_stagger_timer;
	uint	in_pm2_rcv;
	bool	apsd_sta_usp;		/**< Unscheduled Service Period in progress on STA */
	bool	WME_PM_blocked;		/**< Can STA go to PM when in WME Auto mode */

	/* PM2 Receive Throttle Duty Cycle */
	uint16	pm2_rcv_percent;	/**< Duty cycle ON % in each bcn interval */
	pm2rd_state_t pm2_rcv_state;	/**< Duty cycle state */
	uint16	pm2_rcv_time;		/**< Duty cycle ON time in ms */

	/* PM2 Return to Sleep */
	uint	pm2_sleep_ret_time;	/**< configured time to return to sleep in ms */
	uint	pm2_sleep_ret_threshold;	/**< idle threshold in ms before sleep */
	uint	pm2_last_wake_time;	/**< last tx/rx activity tim in gptimer ticks(uSec) */
	bool	pm2_refresh_badiv;	/**< PM2 timeout refresh with bad iv frames */

	/* ADV_PS_POLL */
	/* send pspoll after TX */
	bool	adv_ps_poll;		/**< enable/disable 'send_pspoll_after_tx' */
	bool	send_pspoll_after_tx;   /* send pspoll frame after last TX frame, to check
					 * any buffered frames in AP, during PM = 1,
					 * (or send ps poll in advance after last tx)
					 */

	wlc_hrt_to_t *pm2_rcv_timer; /* recv duration timeout object used with
					 * multiplexed hw timers
					 */
	wlc_hrt_to_t *pm2_ret_timer; /* return to sleep timeout object used with
					 * multiplexed hw timers
					 */
	bool	pm2_ps0_allowed;	/**< allow going to PS off state even when PMpending */
	mbool	PMblocked;		/**< block any PSPolling in PS mode, used to buffer
					 * AP traffic, also used to indicate in progress
					 * of scan, rm, etc. off home channel activity.
					 */

	/* Dynamic Fast Return To Sleep settings */
	uint8 dfrts_logic;			/**< Dynamic FRTS algorithm */
	uint16 dfrts_high_ms;			/**< High FRTS timeout */
	uint16 dfrts_rx_pkts_threshold;		/**< switching threshold # rx pkts */
	uint16 dfrts_tx_pkts_threshold;		/**< switching threshold # tx pkts */
	uint16 dfrts_txrx_pkts_threshold;	/**< switching threshold # tx+rx pkts */
	uint32 dfrts_rx_bytes_threshold;	/**< switching threshold # rx bytes */
	uint32 dfrts_tx_bytes_threshold;	/**< switching threshold # tx bytes */
	uint32 dfrts_txrx_bytes_threshold;	/**< switching threshold # tx+rx bytes */

	/* Dynamic Fast Return To Sleep counters */
	uint16	dfrts_rx_pkts;			/**< PM2 rx bytes count since wake */
	uint16	dfrts_tx_pkts;			/**< PM2 tx pkts count since wake */
	uint32	dfrts_rx_bytes;			/**< PM2 rx bytes count since wake */
	uint32	dfrts_tx_bytes;			/**< PM2 tx bytes count since wake */

	/* Firmware level (not ucode) PM1 Null tx retries with new seq#s */
	uint8	pm_immed_retries;		/**< # times PM1 Null immediately resent */
	uint8	ps_resend_mode;			/**< Mode of operation for PM2 PM Null resends */

	int8	pm2_md_sleep_ext; /* In PM2&PM1 mode with more data indication on PSpoll Response
				   * wake up from PM2 sleep with a regular PM2 Beacon FRTS timer
				   */
	uint8	pspoll_md_cnt;
	uint8	pspoll_md_counter;

	/* Fast Return to sleep enhancements */
	uint16	pm2_bcn_sleep_ret_time;	/**< PM2 return to sleep after bcn */
	uint8	pm2_rx_pkts_since_bcn;	/**< PM2 rx pkts count since bcn */
	bool	dfrts_reached_threshold;	/**< The current PM cycle has reached the
						 * DFRTS pkts/bytes threshold for using
						 * the high FRTS timeout.
						 */
	uint32	tim_bits_in_last_bcn;	/* total number of TIM bit set in last received beacon */
	uint8 PM_oldvalue;	/* Save the old PM mode value, and re-assign this value to
				 * PM mode if STA moved from Radar to non-Radar channel
				 */
	uint8 PMmodeChangeDisabled;	/* Flag to handle allowing pm mode value change by user in
					 * DFS/non-DFS channels
					 */
};

/* attach/detach interface */
wlc_pm_info_t *wlc_pm_attach(wlc_info_t *wlc);
void wlc_pm_detach(wlc_pm_info_t *pmi);

void wlc_pm_ignore_bcmc(wlc_pm_info_t *pmi,  bool ignore_bcmc);

bool wlc_sendapsdtrigger(wlc_info_t *wlc, wlc_bsscfg_t *cfg);

#ifdef STA
#ifdef ADV_PS_POLL
void wlc_adv_pspoll_upd(wlc_info_t *wlc, struct scb *scb, void *pkt, bool sent, uint fifo);
#else
#define wlc_adv_pspoll_upd(wlc, scb, pkt, sent, fifo) do {} while (0)
#endif /* ADV_PS_POLL */
#endif /* STA */

void wlc_bcn_tim_ie_pm2_action(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern uint8 wlc_get_pm_state(wlc_bsscfg_t *cfg);

/* ******** WORK-IN-PROGRESS ******** */

#endif	/* _wlc_pm_h_ */
