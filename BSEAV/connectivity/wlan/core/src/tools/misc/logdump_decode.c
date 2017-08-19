/*
 * File PRINTLOG utility - process event log dump messages
 *
 * Copyright (C) 2012 Broadcom Corporation
 *
 * $Id$
 */

#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <typedefs.h>

#include "wlioctl.h"
#include "bcmendian.h"
#include "bcmutils.h"
#include "logdump_decode.h"
#include <proto/event_log_payload.h>

/* This define turns pointers into uint32 for compatability between
 * the target processor and the processor running this utility
 */
#define EVENT_LOG_COMPILE
#define EVENT_LOG_DUMPER
#include "event_log.h"

bool g_swap = FALSE;

/* IOCTL swapping mode for Big Endian host with Little Endian dongle.  Default to off */
/* The below macros handle endian mis-matches between wl utility and wl driver. */
#define htod64(i) (g_swap?bcmswap64(i):(uint64)(i))
#define htod32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define htod16(i) (g_swap?bcmswap16(i):(uint16)(i))
#define dtoh64(i) (g_swap?bcmswap64(i):(uint64)(i))
#define dtoh32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh16(i) (g_swap?bcmswap16(i):(uint16)(i))
#define htodchanspec(i) (g_swap?htod16(i):i)
#define dtohchanspec(i) (g_swap?dtoh16(i):i)
#define htodenum(i) (g_swap?((sizeof(i) == 4) ? htod32(i) : ((sizeof(i) == 2) ? htod16(i) : i)):i)
#define dtohenum(i) (g_swap?((sizeof(i) == 4) ? dtoh32(i) : ((sizeof(i) == 2) ? htod16(i) : i)):i)

#define	PRVAL(name)	pbuf += sprintf(pbuf, "%u,", dtoh32(cnt->name))
#define	PRNL()
#define	PRVAL_NAME(name)	pbuf += sprintf(pbuf, "%s,", #name)

#define DUMP_VAL(name, is_string) do {	\
	bool i = is_string;	\
	if (i == TRUE) PRVAL_NAME(name);	\
	else PRVAL(name); \
} while (0)

#define WL_CNT_VERSION_SIX 6

#if WL_CNT_T_VERSION <= 11

static int
wl_counters(uint8 *data, bool print_names, FILE *fp)
{
	char *statsbuf;
	wl_cnt_t *cnt;
	wl_cnt_ver_six_t *cnt_six;
	uint i;
	char *pbuf = buf;
	uint16 ver;

	statsbuf = (char *)data;
	ver = *(uint16*)statsbuf;

	if (ver > WL_CNT_T_VERSION) {
		printf("\tIncorrect version of counters struct: expected %x; got %x\n",
		       WL_CNT_T_VERSION, ver);
		return -1;
	}
	else if (ver == WL_CNT_VERSION_SIX) {
		printf("\tUse version 6 counters struct\n");
	}
	else {
		if (ver != WL_CNT_T_VERSION) {
			printf("\tIncorrect version of counters struct: expected %x; got %x\n",
			       WL_CNT_T_VERSION, ver);
			printf("\tDisplayed values may be incorrect\n");
		}
	}

	cnt_six = (wl_cnt_ver_six_t*)malloc(sizeof(wl_cnt_ver_six_t));
	if (cnt_six == NULL) {
		printf("\tCan not allocate %d bytes for counters six struct\n",
		       (int)sizeof(wl_cnt_ver_six_t));
		return BCME_NOMEM;
	} else
		memcpy(cnt_six, statsbuf, sizeof(wl_cnt_ver_six_t));

	cnt = (wl_cnt_t*)malloc(sizeof(wl_cnt_t));
	if (cnt == NULL) {
		printf("\tCan not allocate %d bytes for counters struct\n",
		       (int)sizeof(wl_cnt_t));
		return BCME_NOMEM;
	} else
		memcpy(cnt, statsbuf, sizeof(wl_cnt_t));

	/* summary stat counter line */
	DUMP_VAL(txframe, print_names); DUMP_VAL(txbyte, print_names);
	DUMP_VAL(txretrans, print_names); DUMP_VAL(txerror, print_names);
	DUMP_VAL(rxframe, print_names); DUMP_VAL(rxbyte, print_names);
	DUMP_VAL(rxerror, print_names); PRNL();

	DUMP_VAL(txprshort, print_names); DUMP_VAL(txdmawar, print_names);
	DUMP_VAL(txnobuf, print_names); DUMP_VAL(txnoassoc, print_names);
	DUMP_VAL(txchit, print_names); DUMP_VAL(txcmiss, print_names); PRNL();

	DUMP_VAL(reset, print_names); DUMP_VAL(txserr, print_names);
	DUMP_VAL(txphyerr, print_names); DUMP_VAL(txphycrs, print_names);
	DUMP_VAL(txfail, print_names); DUMP_VAL(tbtt, print_names); PRNL();

	if (print_names) {
		pbuf += sprintf(pbuf, "%s,%s,%s,%s,", "d11_txfrag", "d11_txmulti",
			"d11_txretry", "d11_txretrie");
		pbuf += sprintf(pbuf, "%s,%s,%s,%s,", "d11_txrts", "d11_txnocts",
			"d11_txnoack", "d11_txfrmsnt");
	} else {
		pbuf += sprintf(pbuf, "%u,%u,%u,%u,",
			dtoh32(cnt->txfrag), dtoh32(cnt->txmulti), dtoh32(cnt->txretry),
			dtoh32(cnt->txretrie));

		pbuf += sprintf(pbuf, "%u,%u,%u,%u,",
			dtoh32(cnt->txrts), dtoh32(cnt->txnocts), dtoh32(cnt->txnoack),
			dtoh32(cnt->txfrmsnt));
	}

	DUMP_VAL(rxcrc, print_names); DUMP_VAL(rxnobuf, print_names);
	DUMP_VAL(rxnondata, print_names); DUMP_VAL(rxbadds, print_names);
	DUMP_VAL(rxbadcm, print_names); DUMP_VAL(rxdup, print_names);
	if (cnt->version == 7) {
		if (cnt->length >= OFFSETOF(wl_cnt_t, dma_hang) + sizeof(uint32))
			DUMP_VAL(rxrtry, print_names);
	}
	DUMP_VAL(rxfragerr, print_names); PRNL();
	DUMP_VAL(rxrunt, print_names); DUMP_VAL(rxgiant, print_names);
	DUMP_VAL(rxnoscb, print_names); DUMP_VAL(rxbadproto, print_names);
	DUMP_VAL(rxbadsrcmac, print_names); PRNL();

	if (print_names) {
		pbuf += sprintf(pbuf, "%s,%s,%s,", "d11_rxfrag", "d11_rxmulti", "d11_rxundec");
	} else {
		pbuf += sprintf(pbuf, "%u,%u,%u,",
			dtoh32(cnt->rxfrag), dtoh32(cnt->rxmulti), dtoh32(cnt->rxundec));
	}

	DUMP_VAL(rxctl, print_names); DUMP_VAL(rxbadda, print_names);
	DUMP_VAL(rxfilter, print_names); PRNL();

	if (print_names) {
		for (i = 0; i < NFIFO; i++)
			pbuf += sprintf(pbuf, "%s%u,", "rxuflo", i);
	} else {
		for (i = 0; i < NFIFO; i++)
			pbuf += sprintf(pbuf, "%u,", dtoh32(cnt->rxuflo[i]));
	}

	DUMP_VAL(txallfrm, print_names); DUMP_VAL(txrtsfrm, print_names);
	DUMP_VAL(txctsfrm, print_names); DUMP_VAL(txackfrm, print_names); PRNL();
	DUMP_VAL(txdnlfrm, print_names); DUMP_VAL(txbcnfrm, print_names);
	DUMP_VAL(txtplunfl, print_names); DUMP_VAL(txphyerr, print_names); PRNL();

	if (print_names) {
		for (i = 0; i < NFIFO; i++)
			pbuf += sprintf(pbuf, "%s%u,", "txfunfl", i);
	} else {
		for (i = 0; i < NFIFO; i++)
			pbuf += sprintf(pbuf, "%u,", dtoh32(cnt->txfunfl[i]));
	}

	/* WPA2 counters */
	PRNL();
	if ((cnt->version == WL_CNT_VERSION_SIX) && (cnt->version != WL_CNT_T_VERSION)) {
		DUMP_VAL(tkipmicfaill, print_names); DUMP_VAL(tkipicverr, print_names);
		DUMP_VAL(tkipcntrmsr, print_names); PRNL();
		DUMP_VAL(tkipreplay, print_names); DUMP_VAL(ccmpfmterr, print_names);
		DUMP_VAL(ccmpreplay, print_names); PRNL();
		DUMP_VAL(ccmpundec, print_names); DUMP_VAL(fourwayfail, print_names);
		DUMP_VAL(wepundec, print_names); PRNL();
		DUMP_VAL(wepicverr, print_names); DUMP_VAL(decsuccess, print_names);
		DUMP_VAL(rxundec, print_names); PRNL();
	} else {
		DUMP_VAL(tkipmicfaill, print_names); DUMP_VAL(tkipicverr, print_names);
		DUMP_VAL(tkipcntrmsr, print_names); PRNL();
		DUMP_VAL(tkipreplay, print_names); DUMP_VAL(ccmpfmterr, print_names);
		DUMP_VAL(ccmpreplay, print_names); PRNL();
		DUMP_VAL(ccmpundec, print_names); DUMP_VAL(fourwayfail, print_names);
		DUMP_VAL(wepundec, print_names); PRNL();
		DUMP_VAL(wepicverr, print_names); DUMP_VAL(decsuccess, print_names);
		DUMP_VAL(rxundec, print_names); PRNL();
	}
	PRNL();
	DUMP_VAL(rxfrmtoolong, print_names); DUMP_VAL(rxfrmtooshrt, print_names);
	DUMP_VAL(rxinvmachdr, print_names); DUMP_VAL(rxbadfcs, print_names); PRNL();
	DUMP_VAL(rxbadplcp, print_names); DUMP_VAL(rxcrsglitch, print_names);
	DUMP_VAL(rxstrt, print_names); DUMP_VAL(rxdfrmucastmbss, print_names); PRNL();
	DUMP_VAL(rxmfrmucastmbss, print_names); DUMP_VAL(rxcfrmucast, print_names);
	DUMP_VAL(rxrtsucast, print_names); DUMP_VAL(rxctsucast, print_names); PRNL();
	DUMP_VAL(rxackucast, print_names); DUMP_VAL(rxdfrmocast, print_names);
	DUMP_VAL(rxmfrmocast, print_names); DUMP_VAL(rxcfrmocast, print_names); PRNL();
	DUMP_VAL(rxrtsocast, print_names); DUMP_VAL(rxctsocast, print_names);
	DUMP_VAL(rxdfrmmcast, print_names); DUMP_VAL(rxmfrmmcast, print_names); PRNL();
	DUMP_VAL(rxcfrmmcast, print_names); DUMP_VAL(rxbeaconmbss, print_names);
	DUMP_VAL(rxdfrmucastobss, print_names); DUMP_VAL(rxbeaconobss, print_names); PRNL();
	DUMP_VAL(rxrsptmout, print_names); DUMP_VAL(bcntxcancl, print_names);
	DUMP_VAL(rxf0ovfl, print_names); DUMP_VAL(rxf1ovfl, print_names); PRNL();
	DUMP_VAL(rxf2ovfl, print_names); DUMP_VAL(txsfovfl, print_names);
	DUMP_VAL(pmqovfl, print_names); PRNL();	DUMP_VAL(rxcgprqfrm, print_names);
	DUMP_VAL(rxcgprsqovfl, print_names); DUMP_VAL(txcgprsfail, print_names);
	DUMP_VAL(txcgprssuc, print_names); PRNL(); DUMP_VAL(prs_timeout, print_names);
	DUMP_VAL(rxnack, print_names); DUMP_VAL(frmscons, print_names);
	DUMP_VAL(txnack, print_names); DUMP_VAL(txphyerror, print_names); PRNL();

	if ((cnt->version == WL_CNT_VERSION_SIX) && (cnt->version != WL_CNT_T_VERSION)) {
		DUMP_VAL(txchanrej, print_names); PRNL();
		/* per-rate receive counters */
		DUMP_VAL(rx1mbps, print_names); DUMP_VAL(rx2mbps, print_names);
		DUMP_VAL(rx5mbps5, print_names); PRNL();
		DUMP_VAL(rx6mbps, print_names); DUMP_VAL(rx9mbps, print_names);
		DUMP_VAL(rx11mbps, print_names); PRNL();
		DUMP_VAL(rx12mbps, print_names); DUMP_VAL(rx18mbps, print_names);
		DUMP_VAL(rx24mbps, print_names); PRNL();
		DUMP_VAL(rx36mbps, print_names); DUMP_VAL(rx48mbps, print_names);
		DUMP_VAL(rx54mbps, print_names); PRNL();

		DUMP_VAL(pktengrxducast, print_names); DUMP_VAL(pktengrxdmcast, print_names);
		PRNL(); DUMP_VAL(txmpdu_sgi, print_names); DUMP_VAL(rxmpdu_sgi, print_names);
		DUMP_VAL(txmpdu_stbc, print_names);	DUMP_VAL(rxmpdu_stbc, print_names); PRNL();
	} else {
		DUMP_VAL(txchanrej, print_names); PRNL();
		if (cnt->version >= 4) {
			/* per-rate receive counters */
			DUMP_VAL(rx1mbps, print_names); DUMP_VAL(rx2mbps, print_names);
			DUMP_VAL(rx5mbps5, print_names); PRNL(); DUMP_VAL(rx6mbps, print_names);
			DUMP_VAL(rx9mbps, print_names); DUMP_VAL(rx11mbps, print_names);
			PRNL(); DUMP_VAL(rx12mbps, print_names); DUMP_VAL(rx18mbps, print_names);
			DUMP_VAL(rx24mbps, print_names); PRNL(); DUMP_VAL(rx36mbps, print_names);
			DUMP_VAL(rx48mbps, print_names); DUMP_VAL(rx54mbps, print_names); PRNL();
		}

		if (cnt->version >= 5) {
			DUMP_VAL(pktengrxducast, print_names);
			DUMP_VAL(pktengrxdmcast, print_names);
			PRNL();
		}

		if (cnt->version >= 6) {
			DUMP_VAL(txmpdu_sgi, print_names);
			DUMP_VAL(rxmpdu_sgi, print_names);
			DUMP_VAL(txmpdu_stbc, print_names);
			DUMP_VAL(rxmpdu_stbc, print_names); PRNL();
		}

		if (cnt->version >= 8) {
			if (cnt->length >= OFFSETOF(wl_cnt_t, cso_passthrough) + sizeof(uint32)) {
				DUMP_VAL(cso_normal, print_names);
				DUMP_VAL(cso_passthrough, print_names);
				PRNL();
			}
			DUMP_VAL(chained, print_names); DUMP_VAL(chainedsz1, print_names);
			DUMP_VAL(unchained, print_names); DUMP_VAL(maxchainsz, print_names);
			DUMP_VAL(currchainsz, print_names); PRNL();
		}
		if (cnt->version >= 9) {
			DUMP_VAL(pciereset, print_names); DUMP_VAL(cfgrestore, print_names); PRNL();
		}
		if ((cnt->version >= 10) && (cnt->length >= OFFSETOF(wl_cnt_t, rxaction))) {
			DUMP_VAL(txbar, print_names); DUMP_VAL(txback, print_names);
			DUMP_VAL(txpspoll, print_names); PRNL(); DUMP_VAL(rxbar, print_names);
			DUMP_VAL(rxback, print_names); DUMP_VAL(rxpspoll, print_names); PRNL();
			DUMP_VAL(txnull, print_names); DUMP_VAL(txqosnull, print_names);
			DUMP_VAL(rxnull, print_names); DUMP_VAL(rxqosnull, print_names);
			PRNL(); DUMP_VAL(txassocreq, print_names);
			DUMP_VAL(txreassocreq, print_names); DUMP_VAL(txdisassoc, print_names);
			DUMP_VAL(txassocrsp, print_names); DUMP_VAL(txreassocrsp, print_names);
			PRNL(); DUMP_VAL(txauth, print_names); DUMP_VAL(txdeauth, print_names);
			DUMP_VAL(txprobereq, print_names); DUMP_VAL(txprobersp, print_names);
			DUMP_VAL(txaction, print_names); PRNL();
			DUMP_VAL(rxassocreq, print_names); DUMP_VAL(rxreassocreq, print_names);
			DUMP_VAL(rxdisassoc, print_names); DUMP_VAL(rxassocrsp, print_names);
			DUMP_VAL(rxreassocrsp, print_names); PRNL(); DUMP_VAL(rxauth, print_names);
			DUMP_VAL(rxdeauth, print_names); DUMP_VAL(rxprobereq, print_names);
			DUMP_VAL(rxprobersp, print_names); DUMP_VAL(rxaction, print_names); PRNL();
		}
	}

	pbuf += sprintf(pbuf, "\n");
	fputs(buf, fp);

	if (cnt)
		free(cnt);

	if (cnt_six)
		free(cnt_six);

	return (0);
}

#endif /* WL_CNT_T_VERSION <= 11 */

#ifdef ECOUNTERS_TRIGGER_REASON_VERSION_1
void write_ecounters_trigger_reason_header(FILE *fp)
{
	fprintf(fp, "%s,%s,%s,%s,", "pmu", "arm cycle", "arm cycle", "tag");

	fprintf(fp, "%s,%s,%s,%s,%s\n",
		"version", "trigger_reason", "sub_reason_code",
		"trigger_time_now", "host_ref_time");
}
#endif /* ECOUNTERS_TRIGGER_REASON_VERSION_1 */

#ifdef WL_LEAKY_AP_STATS_PKT_TYPE
void write_leaky_ap_stat_header(FILE *fp)
{
	fprintf(fp, "%s,%s,%s,%s,", "pmu", "arm cycle", "arm cycle", "tag");

	fprintf(fp, "%s,%s,%s,%s,%s,",
		"type", "len", "seq_number",
		"start_time", "gt_tsf_l");

	fprintf(fp, "%s,%s,%s,%s,%s,",
		"guard_duration", "num_pkts", "flag",
		"ppdu_len_bytes", "num_mpdus");

	fprintf(fp, "%s,%s,%s,%s,%s\n",
		"ppdu_time", "rate", "seq_number",
		"rssi", "tid");

}
#endif /* WL_LEAKY_AP_STATS_PKT_TYPE */

void write_lqm_strcut_header(FILE *fp)
{
	fprintf(fp, "%s,%s,%s,%s,", "pmu", "arm cycle", "arm cycle", "tag");

	fprintf(fp, "%s,%s,%s,%s,%s,",
		"version", "flags", "pad", "noise_level", "current_BSS_ether_address");

	fprintf(fp, "%s,%s,%s,",
		"current_BSS_chanspec", "current_BSS_rssi", "current_BSS_snr");

	fprintf(fp, "%s,%s,%s,",
		"target_bss_ether_address", "target_BSS_chanspec", "target_BSS_rssi");

	fprintf(fp, "%s\n", "target_BSS_snr");
}

void write_three_tags_channel_switch_header(FILE *fp)
{
	fprintf(fp, "%s,%s,%s,%s,", "pmu", "arm cycle", "arm cycle", "tag");

	fprintf(fp, "%s,%s,%s,%s,%s,",
		"time", "old", "new", "reason", "dwelltime");

	fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,",
		"type", "len", "rate1", "rate2", "rate3", "rate4",
		"rate5", "rate6", "rate7", "rate8", "rate9", "rate10");

	fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,",
		"type", "len", "txnull", "rxnull", "txqosnull",
		"rxqosnull", "txassocreq", "rxassocreq",
		"txreassocreq", "rxreassocreq", "txdisassoc",
		"rxdisassoc");

	fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,",
		"txassocrsp", "rxassocrsp", "txreassocrsp",
		"rxreassocrsp", "txauth", "rxauth", "txdeauth",
		"rxdeauth", "txprobereq", "rxprobereq");

	fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,",
		"txprobersp", "rxprobersp", "txaction",
		"rxaction", "txrts", "rxrts", "txcts",
		"rxcts", "txack", "rxack");

	fprintf(fp, "%s,%s,%s,%s,%s,%s\n", "txbar", "rxbar",
		"txback", "rxback", "txpspoll", "rxpspoll");

}


void write_ecounters_pwrstates_phy_header(FILE *fp)
{

	fprintf(fp, "%s,%s,%s,%s,", "pmu", "arm cycle", "arm cycle", "tag");

	fprintf(fp, "%s,%s,%s,%s", "type", "len", "tx_dur", "rx_dur");

	fprintf(fp, "\n");
}


void write_channel_switch_header(FILE *fp)
{
	fprintf(fp, "%s,%s,%s,%s,", "pmu", "arm cycle", "arm cycle", "tag");

	fprintf(fp, "%s,%s,%s,%s,%s,",
		"time", "old", "new", "reason", "dwelltime");
	fprintf(fp, "\n");
}

void write_rate_cnt_header(FILE *fp)
{
	fprintf(fp, "%s,%s,%s,%s,", "pmu", "arm cycle", "arm cycle", "tag");

	fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,",
		"type", "len", "rate1", "rate2", "rate3", "rate4",
		"rate5", "rate6", "rate7", "rate8", "rate9", "rate10");
	fprintf(fp, "\n");

}

void write_mgt_rate_cnt_header(FILE *fp)
{
	fprintf(fp, "%s,%s,%s,%s,", "pmu", "arm cycle", "arm cycle", "tag");

	fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,",
		"type", "len", "txnull", "rxnull", "txqosnull",
		"rxqosnull", "txassocreq", "rxassocreq",
		"txreassocreq", "rxreassocreq", "txdisassoc",
		"rxdisassoc");

	fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,",
		"txassocrsp", "rxassocrsp", "txreassocrsp",
		"rxreassocrsp", "txauth", "rxauth", "txdeauth",
		"rxdeauth", "txprobereq", "rxprobereq");

	fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,",
		"txprobersp", "rxprobersp", "txaction",
		"rxaction", "txrts", "rxrts", "txcts",
		"rxcts", "txack", "rxack");

	fprintf(fp, "%s,%s,%s,%s,%s,%s\n", "txbar", "rxbar",
		"txback", "rxback", "txpspoll", "rxpspoll");
}

#ifdef WL_AMPDU_STATS_MAX_CNTS
void write_ampdu_dump_header(FILE *fp)
{
int i;

	fprintf(fp, "%s,%s,%s,%s,", "pmu", "arm cycle", "arm cycle", "tag");

	fprintf(fp, "%s,%s,", "type", "len");

	for (i = 0; i < WL_AMPDU_STATS_MAX_CNTS; i ++)
		fprintf(fp, "%s,", "counter");

	fprintf(fp, "\n");

}
#endif /* WL_AMPDU_STATS_MAX_CNTS */

void write_ecounters_btcx_stats(FILE *fp)
{
	fprintf(fp, "%s,%s,%s,%s,", "pmu", "arm cycle", "arm cycle", "tag");

	fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
		"version", "len", "stats_update_timestamp", "btc_status",
		"bt_req_type_map", "bt_req_cnt", "bt_gnt_cnt",
		"bt_gnt_dur", "bt_abort_cnt", "bt_rxf1ovfl_cnt",
		"bt_latency_cnt", "rsvd");
}

void write_ecounters_ipcstates_header(FILE *fp)
{
	fprintf(fp, "%s,%s,%s,%s,", "pmu", "arm cycle", "arm cycle", "tag");

	fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,", "d3_suspend_ct",
		"d0_resume_ct", "perst_assrt_ct", "perst_deassrt_ct",
		"active_dur", "d3_suspend_dur", "perst_dur", "l0_cnt",
		"l0_usecs", "l1_cnt");

	fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,", "l1_usecs",
		"l1_1_cnt", "l1_1_usecs", "l1_2_cnt", "l1_2_usecs",
		"l2_cnt", "l2_usecs", "timestamp", "num_h2d_doorbell",
		"num_d2h_doorbell");

	fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
		"num_submissions", "num_completions", "num_rxcmplt",
		"num_rxcmplt_drbl", "num_txstatus", "num_txstatus_drb",
		"deepsleep_count", "deepsleep_dur", "ltr_active_ct",
		"ltr_active_dur", "ltr_sleep_ct", "ltr_sleep_dur");
}

void write_ecounters_pwrstates_scan_header(FILE *fp)
{
int i;
char print_string[20];

	fprintf(fp, "%s,%s,%s,%s,", "pmu", "arm cycle", "arm cycle", "tag");

	fprintf(fp, "%s,%s,%s,%s,", "type", "len", "user_scans_count", "user_scans_dur");

	fprintf(fp, "%s,%s,%s,%s,", "assoc_scans_count", "assoc_scans_dur",
		"roam_scans_count", "roam_scans_dur");

	for (i = 0; i < 8; i ++) {
		sprintf(print_string, "%s%d%s", "pno_scans_", i, "_count");
		fprintf(fp, "%s,", print_string);
		sprintf(print_string, "%s%d%s", "pno_scans_", i, "_dur");
		fprintf(fp, "%s,", print_string);
	}

	fprintf(fp, "%s,%s", "other_scans_count", "other_scans_dur");

	fprintf(fp, "\n");
}


void write_ecounters_pwrstates_wake_v2_header(FILE *fp)
{

	fprintf(fp, "%s,%s,%s,%s,", "pmu", "arm cycle", "arm cycle", "tag");

	fprintf(fp, "%s,%s,%s,%s,", "type", "len", "curr_time", "hw_macc");

	fprintf(fp, "%s,%s,%s,%s,", "sw_macc", "pm_dur", "mpc_dur", "last_drift");

	fprintf(fp, "%s,%s,%s,%s,", "min_drift", "max_drift", "avg_drift", "drift_cnt");

	fprintf(fp, "%s,%s", "frts_end_cnt", "frts_time");

	fprintf(fp, "\n");
}

void write_scan_summary_stat_header(FILE *fp)
{
	fprintf(fp, "\n");

	fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s \n",
		"Channel_Info: Type","sync ID","length","Chanspec","Scan_type","core","start_time",
		"channel_dwell_time","Received Probe responses count",
		"Count of Scan_results found");

	fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s,%s \n",
		"Scan_summary_Info: Type","sync ID","length","Total number of channels scanned",
		"isparallel","is_5g_SIB_enabled","is_2g_SIB_enabled","Total scan duration","SSID list for scanning");

	fprintf(fp, "\n");

}

#ifdef WL_AMPDU_STATS_MAX_CNTS
/* for EVENT_LOG_TAG_AMPDU_DUMP log log "wl_ampdu_stats_generic_t" */
/* structure */
void logdump_ampdu_dump(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size)
{
int i;
wl_ampdu_stats_generic_t *ampdu_stats_generic;
int ampdu_stats_count;

	if (trace_info->first_dump_info) {
		trace_info->first_dump_info = FALSE;
		write_ampdu_dump_header(trace_info->statistic_file);
	}
	fprintf(trace_info->statistic_file,
		",,%.6f,%s,",
		(double)TIME_IN_SECONDS(arm_time_cycle),
		"AMPDU_DUMP");
	ampdu_stats_generic = (wl_ampdu_stats_generic_t*)(data);

	fprintf(trace_info->statistic_file, "%d,%d,",
		ampdu_stats_generic->type,
		ampdu_stats_generic->len);

	ampdu_stats_count = data_size -2;

	if (ampdu_stats_count > WL_AMPDU_STATS_MAX_CNTS)
		ampdu_stats_count = WL_AMPDU_STATS_MAX_CNTS;

	if (ampdu_stats_count < 0)
		ampdu_stats_count = 0;

	for (i = 0; i < ampdu_stats_count; i ++)
		fprintf(trace_info->statistic_file, "%u,",
			ampdu_stats_generic->counters[i]);

	fprintf(trace_info->statistic_file, "\n");

}
#endif /* WL_AMPDU_STATS_MAX_CNTS */

/* log channel switch info for EVENT_LOG_TAG_TRACE_CHANSW */
void logdump_channel_switch(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size)
{
uint i;

	if (trace_info->first_dump_info) {
		trace_info->first_dump_info = FALSE;
		write_three_tags_channel_switch_header(trace_info->statistic_file);
	}
	fprintf(trace_info->statistic_file,
		",,%.6f,%s,",
		(double)TIME_IN_SECONDS(arm_time_cycle),
		"CHANNEL_SWITCH");
	for (i = 0; i < data_size - 1; i ++)
		fprintf(trace_info->statistic_file,
			"%x,",
			data[i]);
	fprintf(trace_info->statistic_file, "\n");

}

/* for EVENT_LOG_TAG_RATE_CNT */
void logdump_rate_cnt(eventdump_info_t *trace_info, uint32 *data,
	uint32 arm_time_cycle, uint32 data_size)
{
uint i;

	fprintf(trace_info->statistic_file,
		",,%.6f,%s,",
		(double)TIME_IN_SECONDS(arm_time_cycle),
		"RATE_CNT");
	fprintf(trace_info->statistic_file,
		",,,,,");
	fprintf(trace_info->statistic_file,
		"%d,%d,",
		((uint16*)(data))[0],
		((uint16*)(data))[1]);
	for (i = 1; i < data_size - 1; i ++)
		fprintf(trace_info->statistic_file,
			"%d,", data[i]);
	fprintf(trace_info->statistic_file, "\n");

}

/* for EVENT_LOG_TAG_CTL_MGT_CNT log the "wl_ctl_mgt_cnt_t" */
/* structure */
void logdump_mgt_rate_cnt(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size)
{
wl_ctl_mgt_cnt_t *ctl_mgt_cnt;

	fprintf(trace_info->statistic_file,
		",,%.6f,%s,",
		(double)TIME_IN_SECONDS(arm_time_cycle),
		"CTL_MGT_CNT");
	fprintf(trace_info->statistic_file, ",,,,,");
	fprintf(trace_info->statistic_file, ",,,,,,,,,,,,");

	ctl_mgt_cnt = (wl_ctl_mgt_cnt_t*)(data);

	fprintf(trace_info->statistic_file, "%d,%d,%u,%u,%u,%u,",
		ctl_mgt_cnt->type,
		ctl_mgt_cnt->len,
		ctl_mgt_cnt->txnull,
		ctl_mgt_cnt->rxnull,
		ctl_mgt_cnt->txqosnull,
		ctl_mgt_cnt->rxqosnull);

	fprintf(trace_info->statistic_file, "%d,%d,%u,%u,%u,%u,",
		ctl_mgt_cnt->txassocreq,
		ctl_mgt_cnt->rxassocreq,
		ctl_mgt_cnt->txreassocreq,
		ctl_mgt_cnt->rxreassocreq,
		ctl_mgt_cnt->txdisassoc,
		ctl_mgt_cnt->rxdisassoc);

	fprintf(trace_info->statistic_file, "%d,%d,%u,%u,%u,%u,",
		ctl_mgt_cnt->txassocrsp,
		ctl_mgt_cnt->rxassocrsp,
		ctl_mgt_cnt->txreassocrsp,
		ctl_mgt_cnt->rxreassocrsp,
		ctl_mgt_cnt->txauth,
		ctl_mgt_cnt->rxauth);

	fprintf(trace_info->statistic_file, "%d,%d,%u,%u,%u,%u,",
		ctl_mgt_cnt->txdeauth,
		ctl_mgt_cnt->rxdeauth,
		ctl_mgt_cnt->txprobereq,
		ctl_mgt_cnt->rxprobereq,
		ctl_mgt_cnt->txprobersp,
		ctl_mgt_cnt->rxprobersp);

	fprintf(trace_info->statistic_file, "%d,%d,%u,%u,%u,%u,",
		ctl_mgt_cnt->txaction,
		ctl_mgt_cnt->rxaction,
		ctl_mgt_cnt->txrts,
		ctl_mgt_cnt->rxrts,
		ctl_mgt_cnt->txcts,
		ctl_mgt_cnt->rxcts);

	fprintf(trace_info->statistic_file, "%d,%d,%u,%u,%u,%u,",
		ctl_mgt_cnt->txack,
		ctl_mgt_cnt->rxack,
		ctl_mgt_cnt->txbar,
		ctl_mgt_cnt->rxbar,
		ctl_mgt_cnt->txback,
		ctl_mgt_cnt->rxback);

	fprintf(trace_info->statistic_file, "%d,%d,",
		ctl_mgt_cnt->txpspoll,
		ctl_mgt_cnt->rxpspoll);


	fprintf(trace_info->statistic_file, "\n");

}

#ifdef BTCX_STATS_VER
/* for EVENT_LOG_TAG_BTCX_STATS log the "wlc_btc_stats" */
/* structure */
void logdump_btcx_stats(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size)
{
wlc_btc_stats_t *btc_states;

	if (trace_info->first_dump_info) {
		write_ecounters_btcx_stats(trace_info->statistic_file);
		trace_info->first_dump_info = FALSE;
	}
	fprintf(trace_info->statistic_file,
		",,%.6f,%s,",
		(double)TIME_IN_SECONDS(arm_time_cycle),
		"ECOUNTERS_BTCX_STATS");

	btc_states = (wlc_btc_stats_t*)(data);

	fprintf(trace_info->statistic_file, "%d,%d,%u,%u,%u,%u,",
		btc_states->version,
		btc_states->valid,
		btc_states->stats_update_timestamp,
		btc_states->btc_status,
		btc_states->bt_req_type_map,
		btc_states->bt_req_cnt);

	fprintf(trace_info->statistic_file, "%d,%d,%u,%u,%u,%u\n",
		btc_states->bt_gnt_cnt,
		btc_states->bt_gnt_dur,
		btc_states->bt_abort_cnt,
		btc_states->bt_rxf1ovfl_cnt,
		btc_states->bt_latency_cnt,
		btc_states->rsvd);
}
#endif /* BTCX_STATS_VER */

/* for EVENT_LOG_TAG_ECOUNTERS_IPCSTATS log the "pcie_bus_metrics" */
/* structure */
void logdump_ecounters_ipcstats(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size)
{
pcie_bus_metrics_t *pcie_bus_metrics;

	if (trace_info->first_dump_info) {
		write_ecounters_ipcstates_header(
			trace_info->statistic_file);
			trace_info->first_dump_info = FALSE;
	}
	fprintf(trace_info->statistic_file,
		",,%.6f,%s,",
		(double)TIME_IN_SECONDS(arm_time_cycle),
		"ECOUNTERS_IPCSTATS");

	pcie_bus_metrics = (pcie_bus_metrics_t*)(data);

	fprintf(trace_info->statistic_file, "%d,%d,%u,%u,%u,%u,",
		pcie_bus_metrics->d3_suspend_ct,
		pcie_bus_metrics->d0_resume_ct,
		pcie_bus_metrics->perst_assrt_ct,
		pcie_bus_metrics->perst_deassrt_ct,
		pcie_bus_metrics->active_dur,
		pcie_bus_metrics->d3_suspend_dur);

	fprintf(trace_info->statistic_file, "%d,%d,%u,%u,%u,%u,",
		pcie_bus_metrics->perst_dur,
		pcie_bus_metrics->l0_cnt,
		pcie_bus_metrics->l0_usecs,
		pcie_bus_metrics->l1_cnt,
		pcie_bus_metrics->l1_usecs,
		pcie_bus_metrics->l1_1_cnt);

	fprintf(trace_info->statistic_file, "%d,%d,%u,%u,%u,%u,",
		pcie_bus_metrics->l1_1_usecs,
		pcie_bus_metrics->l1_2_cnt,
		pcie_bus_metrics->l1_2_usecs,
		pcie_bus_metrics->l2_cnt,
		pcie_bus_metrics->l2_usecs,
		pcie_bus_metrics->timestamp);

	fprintf(trace_info->statistic_file, "%d,%d,%u,%u,%u,%u,",
		pcie_bus_metrics->num_h2d_doorbell,
		pcie_bus_metrics->num_d2h_doorbell,
		pcie_bus_metrics->num_submissions,
		pcie_bus_metrics->num_completions,
		pcie_bus_metrics->num_rxcmplt,
		pcie_bus_metrics->num_rxcmplt_drbl);

	fprintf(trace_info->statistic_file, "%d,%d,%u,%u,%u,%u,",
		pcie_bus_metrics->num_txstatus,
		pcie_bus_metrics->num_txstatus_drbl,
		pcie_bus_metrics->deepsleep_count,
		pcie_bus_metrics->deepsleep_dur,
		pcie_bus_metrics->ltr_active_ct,
		pcie_bus_metrics->ltr_active_dur);

	fprintf(trace_info->statistic_file, "%d,%d,",
		pcie_bus_metrics->ltr_sleep_ct,
		pcie_bus_metrics->ltr_sleep_dur);

	fprintf(trace_info->statistic_file, "\n");
}

#if WL_CNT_T_VERSION <= 11
/* for EVENT_LOG_TAG_WL_COUNTERS save the wl_cnt_t structure */
void logdump_wl_counters(eventdump_info_t *trace_info, uint32 *data,
	uint32 arm_time_cycle, uint32 data_size)
{
	if (trace_info->first_dump_info) {
		fprintf(trace_info->statistic_file,
			"%s, %s, %s,%s,",
			"pmu",
			"arm cycle",
			"arm cycle",
			"tag");
		wl_counters((uint8*)(data),
			TRUE,
			trace_info->statistic_file);
		trace_info->first_dump_info = FALSE;
	}
	fprintf(trace_info->statistic_file,
		",,%.6f,%s,",
		(double)TIME_IN_SECONDS(arm_time_cycle),
		"ECOUNTERS");
	wl_counters((uint8*)(data),
		FALSE,
		trace_info->statistic_file);
}
#endif /* #if WL_CNT_T_VERSION <= 11 */

/* the dump data for the tag EVENT_LOG_TAG_PWRSTATS_PHY is */
/* the "wl_pwr_phy_stats" structure information */
void logdump_wl_powerstats_phy(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size)
{
wl_pwr_phy_stats_t *pwr_phy_stats;

	if (trace_info->first_dump_info) {
		trace_info->first_dump_info = FALSE;
		write_ecounters_pwrstates_phy_header(trace_info->statistic_file);
	}
	fprintf(trace_info->statistic_file,
		",,%.6f,%s,",
		(double)TIME_IN_SECONDS(arm_time_cycle),
		"PWRSTATS_PHY");

	pwr_phy_stats = (wl_pwr_phy_stats_t*)(data);

	fprintf(trace_info->statistic_file, "%d,%d,%u,%u,",
		pwr_phy_stats->type,
		pwr_phy_stats->len,
		pwr_phy_stats->tx_dur,
		pwr_phy_stats->rx_dur);

	fprintf(trace_info->statistic_file, "\n");

}

/* the dump data for the tag EVENT_LOG_TAG_PWRSTATS_SCAN is */
/* the "wl_pwr_scan_stats" structure information */
void logdump_wl_powerstats_scan(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size)
{
uint i;
wl_pwr_scan_stats_t *pwr_scan_stats;

	if (trace_info->first_dump_info) {
		trace_info->first_dump_info = FALSE;
		write_ecounters_pwrstates_scan_header(trace_info->statistic_file);
	}
	fprintf(trace_info->statistic_file,
		",,%.6f,%s,",
		(double)TIME_IN_SECONDS(arm_time_cycle),
		"PWRSTATS_SCAN");
	pwr_scan_stats = (wl_pwr_scan_stats_t*)(data);

	fprintf(trace_info->statistic_file, "%d,%d,%u,%u,%u,%u,",
		pwr_scan_stats->type,
		pwr_scan_stats->len,
		pwr_scan_stats->user_scans.count,
		pwr_scan_stats->user_scans.dur,
		pwr_scan_stats->assoc_scans.count,
		pwr_scan_stats->assoc_scans.dur);

	fprintf(trace_info->statistic_file, "%u,%u,",
		pwr_scan_stats->roam_scans.count,
		pwr_scan_stats->roam_scans.dur);

	for (i = 0; i < 8; i ++)
		fprintf(trace_info->statistic_file, "%u,%u,",
			pwr_scan_stats->pno_scans[i].count,
			pwr_scan_stats->pno_scans[i].dur);

	fprintf(trace_info->statistic_file, "%u,%u",
		pwr_scan_stats->other_scans.count,
		pwr_scan_stats->other_scans.dur);

	fprintf(trace_info->statistic_file, "\n");
}



/* the dump data for the tag EVENT_LOG_TAG_PWRSTATS_WAKE_V2 is */
/* the "wl_pwr_pm_awake_stats_v2_t" structure information */
void logdump_wl_powerstats_v2(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size)
{
wl_pwr_pm_awake_stats_v2_t *powerstats_v2;

	if (trace_info->first_dump_info) {
		trace_info->first_dump_info = FALSE;
		write_ecounters_pwrstates_wake_v2_header(trace_info->statistic_file);
	}
	fprintf(trace_info->statistic_file,
		",,%.6f,%s,",
		(double)TIME_IN_SECONDS(arm_time_cycle),
		"PWRSTATS_WAKE_V2");

	powerstats_v2 = (wl_pwr_pm_awake_stats_v2_t*)(data);

	fprintf(trace_info->statistic_file, ",%d,%d,%u,0x%08x,0x%08x,%u,",
		powerstats_v2->type,
		powerstats_v2->len,
		powerstats_v2->awake_data.curr_time,
		powerstats_v2->awake_data.hw_macc,
		powerstats_v2->awake_data.sw_macc,
		powerstats_v2->awake_data.pm_dur);

	fprintf(trace_info->statistic_file, "%u,%d,%d,%d,%u,%u,",
		powerstats_v2->awake_data.mpc_dur,
		powerstats_v2->awake_data.last_drift,
		powerstats_v2->awake_data.min_drift,
		powerstats_v2->awake_data.max_drift,
		powerstats_v2->awake_data.avg_drift,
		powerstats_v2->awake_data.drift_cnt);

	fprintf(trace_info->statistic_file, "%u,%u",
		powerstats_v2->awake_data.frts_end_cnt,
		powerstats_v2->awake_data.frts_time);

	fprintf(trace_info->statistic_file, "\n");
}

#ifdef WL_LQM_VERSION_1
/* the dump data for the tag EVENT_LOG_TAG_LQM */
/* the "wl_lqm_t" structure information */
void logdump_wl_LQM(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size)
{
wl_lqm_t *lqm;

	if (trace_info->first_dump_info) {
		trace_info->first_dump_info = FALSE;
		write_lqm_strcut_header(trace_info->statistic_file);
	}
	fprintf(trace_info->statistic_file,
		",,%.6f,%s,",
		(double)TIME_IN_SECONDS(arm_time_cycle),
		"LQM");

	lqm = (wl_lqm_t*)(data);

	fprintf(trace_info->statistic_file, "%d,%d,%d,%08d,",
		lqm->version,
		lqm->flags,
		lqm->pad,
		lqm->noise_level);

	fprintf(trace_info->statistic_file, "%02x:%02x:%02x:%02x:%02x:%02x,",
		((uint8*)(&lqm->current_bss.BSSID))[0],
		((uint8*)(&lqm->current_bss.BSSID))[1],
		((uint8*)(&lqm->current_bss.BSSID))[2],
		((uint8*)(&lqm->current_bss.BSSID))[3],
		((uint8*)(&lqm->current_bss.BSSID))[4],
		((uint8*)(&lqm->current_bss.BSSID))[5]);

	fprintf(trace_info->statistic_file, "%04x,%08d,%08d,",
		lqm->current_bss.chanspec,
		lqm->current_bss.rssi,
		lqm->current_bss.snr);

	fprintf(trace_info->statistic_file, "%02x:%02x:%02x:%02x:%02x:%02x,",
		((uint8*)(&lqm->target_bss.BSSID))[0],
		((uint8*)(&lqm->target_bss.BSSID))[1],
		((uint8*)(&lqm->target_bss.BSSID))[2],
		((uint8*)(&lqm->target_bss.BSSID))[3],
		((uint8*)(&lqm->target_bss.BSSID))[4],
		((uint8*)(&lqm->target_bss.BSSID))[5]);

	fprintf(trace_info->statistic_file, "%04x,%08d,%08d\n",
		lqm->target_bss.chanspec,
		lqm->target_bss.rssi,
		lqm->target_bss.snr);
}
#endif /* WL_LQM_VERSION_1 */

#ifdef ECOUNTERS_TRIGGER_REASON_VERSION_1
/* the dump data for the tag EVENT_LOG_TAG_ECOUNTERS_TIME_DATA */
/* the "ecounters_trigger_reason_t" structure information */
void ecounters_trigger_reason(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size)
{
ecounters_trigger_reason_t *trigger_reason;

	if (trace_info->first_dump_info) {
		trace_info->first_dump_info = FALSE;
		write_ecounters_trigger_reason_header(trace_info->statistic_file);
	}

	fprintf(trace_info->statistic_file,
		",,%.6f,%s,",
		(double)TIME_IN_SECONDS(arm_time_cycle),
		"ECOUNTERS_TRIGGER_REASON");

	trigger_reason = (ecounters_trigger_reason_t*)(data);

	fprintf(trace_info->statistic_file, "%d,%d,%d,%u,%u\n",
		trigger_reason->version,
		trigger_reason->trigger_reason,
		trigger_reason->sub_reason_code,
		trigger_reason->trigger_time_now,
		trigger_reason->host_ref_time);
}
#endif /* ECOUNTERS_TRIGGER_REASON_VERSION_1 */

#ifdef WL_LEAKY_AP_STATS_PKT_TYPE
/* the dump data for the tag EVENT_LOG_TAG_LEAKY_AP_STATS */
/* the "wlc_leaked_infra_guard_marker_t" structure information */
/* or the "wlc_leaked_infra_packet_stat_t" structure info */
void leaky_ap_stat(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size)
{
	wlc_leaked_infra_guard_marker_t	*gt_summary;
	wlc_leaked_infra_packet_stat_t	*pkt_stats;
	uint16 type, len;

	if (trace_info->first_dump_info) {
		trace_info->first_dump_info = FALSE;
		write_leaky_ap_stat_header(trace_info->statistic_file);
	}
	fprintf(trace_info->statistic_file,
		",,%.6f,%s,",
		(double)TIME_IN_SECONDS(arm_time_cycle),
		"LEAKY_AP_STATS");

	type = ((uint16*)(data))[0];
	len = ((uint16*)(data))[1];

	fprintf(trace_info->statistic_file, "%u,%u,", type, len);

	if (type == WL_LEAKY_AP_STATS_GT_TYPE)
	{
		gt_summary = (wlc_leaked_infra_guard_marker_t*)(data);
		fprintf(trace_info->statistic_file, "%u,%u,%u,%u,%u,%u\n",
			gt_summary->seq_number,
			gt_summary->start_time,
			gt_summary->gt_tsf_l,
			gt_summary->guard_duration,
			gt_summary->num_pkts,
			gt_summary->flag);
	}
	else {
		pkt_stats = (wlc_leaked_infra_packet_stat_t*)(data);
		fprintf(trace_info->statistic_file, ",,,,,,");
		fprintf(trace_info->statistic_file, "%u,%u,%u,%u,%u,%u,%u\n",
			pkt_stats->ppdu_len_bytes,
			pkt_stats->num_mpdus,
			pkt_stats->ppdu_time,
			pkt_stats->rate,
			pkt_stats->seq_number,
			pkt_stats->rssi,
			pkt_stats->tid);
	}
}
#endif /* WL_LEAKY_AP_STATS_PKT_TYPE */

/* the dump data for the tag EVENT_LOG_TAG_SCAN_SUMMARY */
/* the "wl_scan_summary_t" structure information */
void scan_summary_stat(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size)
{
	struct wl_scan_summary *scan_sum;
	uint32 scan_time = 0;
	if (trace_info->first_dump_info) {
		trace_info->first_dump_info = FALSE;
		write_scan_summary_stat_header(trace_info->statistic_file);
	}
	scan_sum = (struct wl_scan_summary *)(data);
	fprintf(trace_info->statistic_file, "\n");
	if (scan_sum->version == SCAN_SUMMARY_VERSION) {
		char *type;
		if (scan_sum->scan_flags & SCAN_SUM_CHAN_INFO)
		{
			char *scan_type;
			uint16 prb = 0;
			uint16 scn_res = 0;
			scan_time = (scan_sum->u.scan_chan_info.end_time -
				scan_sum->u.scan_chan_info.start_time);
			if (scan_sum->scan_flags & HOME_CHAN) {
				scan_type = "INVALID";
				type = "HOME_CHANNEL_INFO";

			} else {
				scan_type = (scan_sum->scan_flags & ACTIVE_SCAN_SCN_SUM) ?
					"active" : "passive";
				prb = scan_sum->u.scan_chan_info.probe_count;
				scn_res = scan_sum->u.scan_chan_info.scn_res_count;
				type = "CHANNEL_INFO";
			}
			fprintf(trace_info->statistic_file, "%s,%d,%d,%x,%s,%u,%d,%d,%d,%d \n",
					type, scan_sum->sync_id, scan_sum->len,
					scan_sum->u.scan_chan_info.chanspec, scan_type,
					scan_sum->scan_flags & SCAN_SUM_WLC_CORE0 ? 0 : 1,
					scan_sum->u.scan_chan_info.start_time,
					scan_time, prb, scn_res);
		}
		else {
			char *ssid_list = ",";
			type = "SCAN_SUMMARY_INFO";
			scan_time = (scan_sum->u.scan_sum_info.scan_end_time -
				scan_sum->u.scan_sum_info.scan_start_time);
			fprintf(trace_info->statistic_file, "%s,%d,%d,%d,%s,%s,%s,%d,%s \n",
					type, scan_sum->sync_id, scan_sum->len,
					scan_sum->u.scan_sum_info.total_chan_num,
					(scan_sum->scan_flags & PARALLEL_SCAN) ?
					"Yes" : "No",
					(scan_sum->scan_flags & BAND5G_SIB_ENAB) ?
					"Yes" : "No",
					(scan_sum->scan_flags & BAND2G_SIB_ENAB) ?
					"Yes" : "No",
					scan_time, ssid_list);
		}
	} else
		fprintf(trace_info->statistic_file, "%s \n", "Invalid Version");
}

/* from wlu.c */
char *
wl_ether_etoa(const struct ether_addr *n)
{
	static char etoa_buf[ETHER_ADDR_LEN * 3];
	char *c = etoa_buf;
	int i;

	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		if (i)
			*c++ = ':';
		c += sprintf(c, "%02X", n->octet[i] & 0xff);
	}
	return etoa_buf;
}

#ifdef WL_PROXD_TUNE_VERSION
/* uncomment for some debugging prints into the proxd statstic file */
#define PROXD_SAMPLE_COLLECT_LOGPRINT_DBG
static void
proxd_collec_header_dump(wl_proxd_collect_header_t *pHdr, eventdump_info_t *trace_info)
{
	int i;

	fprintf(trace_info->statistic_file,
		"total_frames %lu\n", (unsigned long)ltoh16(pHdr->total_frames));
	fprintf(trace_info->statistic_file,
		"nfft %lu\n", (unsigned long)ltoh16(pHdr->nfft));
	fprintf(trace_info->statistic_file,
		"bandwidth %lu\n", (unsigned long)ltoh16(pHdr->bandwidth));
	fprintf(trace_info->statistic_file,
		"channel %lu\n", (unsigned long)ltoh16(pHdr->channel));
	fprintf(trace_info->statistic_file,
		"chanspec %lu\n", (unsigned long)ltoh32(pHdr->chanspec));
	fprintf(trace_info->statistic_file,
		"fpfactor %lu\n", (unsigned long)ltoh32(pHdr->fpfactor));
	fprintf(trace_info->statistic_file,
		"fpfactor_shift %lu\n", (unsigned long)ltoh16(pHdr->fpfactor_shift));
	fprintf(trace_info->statistic_file,
		"distance %li\n", (long)ltoh32(pHdr->distance));
	fprintf(trace_info->statistic_file,
		"meanrtt %lu\n", (unsigned long)ltoh32(pHdr->meanrtt));
	fprintf(trace_info->statistic_file,
		"modertt %lu\n", (unsigned long)ltoh32(pHdr->modertt));
	fprintf(trace_info->statistic_file,
		"medianrtt %lu\n", (unsigned long)ltoh32(pHdr->medianrtt));
	fprintf(trace_info->statistic_file,
		"sdrtt %lu\n", (unsigned long)ltoh32(pHdr->sdrtt));
	fprintf(trace_info->statistic_file,
		"clkdivisor %lu\n", (unsigned long)ltoh32(pHdr->clkdivisor));
	fprintf(trace_info->statistic_file,
		"chipnum %lu\n", (unsigned long)ltoh16(pHdr->chipnum));
	fprintf(trace_info->statistic_file,
		"chiprev %lu\n", (unsigned long)pHdr->chiprev);
	fprintf(trace_info->statistic_file,
		"phyver %lu\n", (unsigned long)pHdr->phyver);
	fprintf(trace_info->statistic_file,
		"loaclMacAddr %s\n", wl_ether_etoa(&(pHdr->loaclMacAddr)));
	fprintf(trace_info->statistic_file,
		"remoteMacAddr %s\n", wl_ether_etoa(&(pHdr->remoteMacAddr)));
	fprintf(trace_info->statistic_file,
		"params_Ki %lu\n", (unsigned long)ltoh32(pHdr->params.Ki));
	fprintf(trace_info->statistic_file,
		"params_Kt %lu\n", (unsigned long)ltoh32(pHdr->params.Kt));
	fprintf(trace_info->statistic_file,
		"params_vhtack %li\n", (long)ltoh16(pHdr->params.vhtack));
	fprintf(trace_info->statistic_file,
		"params_N_log2 %d\n", TOF_BW_NUM);
	for (i = 0; i < TOF_BW_NUM; i++) {
		fprintf(trace_info->statistic_file, "%li\n",
			(long)ltoh16(pHdr->params.N_log2[i]));
	}
	fprintf(trace_info->statistic_file, "params_N_scale %d\n", TOF_BW_NUM);
	for (i = 0; i < TOF_BW_NUM; i++) {
		fprintf(trace_info->statistic_file, "%li\n",
			(long)ltoh16(pHdr->params.N_scale[i]));
	}
	fprintf(trace_info->statistic_file,
		"params_sw_adj %lu\n", (unsigned long)pHdr->params.sw_adj);
	fprintf(trace_info->statistic_file,
		"params_hw_adj %lu\n", (unsigned long)pHdr->params.hw_adj);
	fprintf(trace_info->statistic_file,
		"params_seq_en %lu\n", (unsigned long)pHdr->params.seq_en);
	fprintf(trace_info->statistic_file,
		"params_core %lu\n", (unsigned long)pHdr->params.core);
	fprintf(trace_info->statistic_file,
		"params_N_log2_seq 2\n");
	for (i = 0; i < 2; i++) {
		fprintf(trace_info->statistic_file, "%li\n",
			(long)ltoh16(pHdr->params.N_log2[i + TOF_BW_NUM]));
	}
	fprintf(trace_info->statistic_file, "params_N_scale_seq 2\n");
	for (i = 0; i < 2; i++) {
		fprintf(trace_info->statistic_file, "%li\n",
			(long)ltoh16(pHdr->params.N_scale[i + TOF_BW_NUM]));
	}
	fprintf(trace_info->statistic_file, "params_w_offset %d\n", TOF_BW_NUM);
	for (i = 0; i < TOF_BW_NUM; i++) {
		fprintf(trace_info->statistic_file, "%li\n",
			(long)ltoh16(pHdr->params.w_offset[i]));
	};
	fprintf(trace_info->statistic_file, "params_w_len %d\n", TOF_BW_NUM);
	for (i = 0; i < TOF_BW_NUM; i++) {
		fprintf(trace_info->statistic_file, "%li\n",
			(long)ltoh16(pHdr->params.w_len[i]));
	};
	fprintf(trace_info->statistic_file,
		"params_maxDT %li\n", (long)ltoh32(pHdr->params.maxDT));
	fprintf(trace_info->statistic_file,
		"params_minDT %li\n", (long)ltoh32(pHdr->params.minDT));
	fprintf(trace_info->statistic_file,
		"params_totalfrmcnt %lu\n", (unsigned long)pHdr->params.totalfrmcnt);
	fprintf(trace_info->statistic_file,
		"params_rsv_media %lu\n", (unsigned long)ltoh16(pHdr->params.rsv_media));
}

static void
proxd_collec_info_dump(wl_proxd_collect_info_t *info, eventdump_info_t *trace_info)
{
	fprintf(trace_info->statistic_file,
		"info_type %lu\n", (unsigned long)ltoh16(info->type));
	fprintf(trace_info->statistic_file,
		"info_index %lu\n", (unsigned long)ltoh16(info->index));
	fprintf(trace_info->statistic_file,
		"info_tof_cmd %lu\n", (unsigned long)ltoh16(info->tof_cmd));
	fprintf(trace_info->statistic_file,
		"info_tof_rsp %lu\n", (unsigned long)ltoh16(info->tof_rsp));
	fprintf(trace_info->statistic_file,
		"info_tof_avb_rxl %lu\n", (unsigned long)ltoh16(info->tof_avb_rxl));
	fprintf(trace_info->statistic_file,
		"info_tof_avb_rxh %lu\n", (unsigned long)ltoh16(info->tof_avb_rxh));
	fprintf(trace_info->statistic_file,
		"info_tof_avb_txl %lu\n", (unsigned long)ltoh16(info->tof_avb_txl));
	fprintf(trace_info->statistic_file,
		"info_tof_avb_txh %lu\n", (unsigned long)ltoh16(info->tof_avb_txh));
	fprintf(trace_info->statistic_file,
		"info_tof_id %lu\n", (unsigned long)ltoh16(info->tof_id));
	fprintf(trace_info->statistic_file,
		"info_tof_frame_type %lu\n", (unsigned long)info->tof_frame_type);
	fprintf(trace_info->statistic_file,
		"info_tof_frame_bw %lu\n", (unsigned long)info->tof_frame_bw);
	fprintf(trace_info->statistic_file,
		"info_tof_rssi %li\n", (long)info->tof_rssi);
	fprintf(trace_info->statistic_file,
		"info_tof_cfo %li\n", (long)ltoh32(info->tof_cfo));
	fprintf(trace_info->statistic_file,
		"info_gd_adj_ns %li\n", (long)ltoh32(info->gd_adj_ns));
	fprintf(trace_info->statistic_file,
		"info_gd_h_adj_ns %li\n", (long)ltoh32(info->gd_h_adj_ns));
	fprintf(trace_info->statistic_file,
		"info_nfft %li\n", (long)ltoh16(info->nfft));
	fprintf(trace_info->statistic_file,
		"H %d\n", (int)ltoh16(info->nfft));
}

static void
proxd_collect_data_dump(wl_proxd_tlv_t *tlv, eventdump_info_t *trace_info)
{
	int i;
	uint32 *data_p = NULL;
	uint8 *data_p_8 = NULL;

	data_p = (uint32 *)tlv->data;
	data_p_8 = tlv->data;

	switch (tlv->id) {
	case WL_PROXD_TLV_ID_COLLECT_DATA:
	case WL_PROXD_TLV_ID_COLLECT_CHAN_DATA:
		for (i = 0; i < tlv->len; i++) {
			fprintf(trace_info->statistic_file, "%lu\n",
				(unsigned long)ltoh32(*(data_p + i)));
		}
		break;
	case WL_PROXD_TLV_ID_RI_RR:
		for (i = 0; i < FTM_TPK_RI_RR_LEN; i++) {
			fprintf(trace_info->statistic_file, "%02x\n",
				*(data_p_8 + i));
		}
		break;
	default:
		break;
	}
}

void proxd_sample_collect(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size)
{
	wl_proxd_event_t *event = (wl_proxd_event_t *)data;
	wl_proxd_tlv_t *tlv = event->tlvs;
	double arm_cycle = (double)TIME_IN_SECONDS(arm_time_cycle);

	if (event->type != WL_PROXD_EVENT_COLLECT) {
		return;
	}

	switch (tlv->id) {
	case WL_PROXD_TLV_ID_COLLECT_HEADER:
		/* Output header (can be used a delimiter) */
		fprintf(trace_info->statistic_file,
			"#### header_sid=%u, arm_time_cycle=[%.6f]\n",
			event->sid, arm_cycle);
		fprintf(trace_info->statistic_file, "d_ref   -1.0\n");
		proxd_collec_header_dump((wl_proxd_collect_header_t *)tlv->data, trace_info);
		break;
	case WL_PROXD_TLV_ID_COLLECT_INFO:
#ifdef PROXD_SAMPLE_COLLECT_LOGPRINT_DBG
		fprintf(trace_info->statistic_file,
			"#### info_sid=%u, arm_time_cycle=[%.6f]\n",
			event->sid, arm_cycle);
#endif /* PROXD_SAMPLE_COLLECT_LOGPRINT_DBG */
		proxd_collec_info_dump((wl_proxd_collect_info_t *)tlv->data, trace_info);
		break;
	case WL_PROXD_TLV_ID_COLLECT_DATA:
#ifdef PROXD_SAMPLE_COLLECT_LOGPRINT_DBG
		fprintf(trace_info->statistic_file,
			"### data_sid=%u, data_fid=%u, %u/%u frag,"
			" frag_size=%u, arm_time_cycle=[%.6f]\n",
			event->sid, event->pad[0], event->pad[1]+1, 2,
			tlv->len, arm_cycle);
#endif /* PROXD_SAMPLE_COLLECT_LOGPRINT_DBG */
		proxd_collect_data_dump(tlv, trace_info);
		break;
	case WL_PROXD_TLV_ID_COLLECT_CHAN_DATA:
#ifdef PROXD_SAMPLE_COLLECT_LOGPRINT_DBG
		fprintf(trace_info->statistic_file,
			"### chan_data_sid=%u, %u/%u frag, frag_size=%u, "
			"arm_time_cycle[%.6f]\n",
			event->sid, event->pad[0]+1, event->pad[1],
			tlv->len,
			arm_cycle);
#endif /* PROXD_SAMPLE_COLLECT_LOGPRINT_DBG */
		if (event->pad[0] == 0) {
			/* first frag only */
			fprintf(trace_info->statistic_file, "chan_est %u\n",
				tlv->len);
		}
		proxd_collect_data_dump(tlv, trace_info);
		break;
	case WL_PROXD_TLV_ID_RI_RR:
#ifdef PROXD_SAMPLE_COLLECT_LOGPRINT_DBG
		fprintf(trace_info->statistic_file,
			"### rirr_sid=%u, arm_time_cycle[%.6f]\n",
			event->sid, arm_cycle);
#endif /* PROXD_SAMPLE_COLLECT_LOGPRINT_DBG */
		fprintf(trace_info->statistic_file, "ri_rr %u\n",
			FTM_TPK_RI_RR_LEN);
		proxd_collect_data_dump(tlv, trace_info);
		break;
	default:
		break;
	}
}
#endif /* WL_PROXD_TUNE_VERSION */

/* register event log dump messages processing callback functions */
void register_dump_processing_callbacks(void)
{
	register_eventlog_dump_processing_callback("chnl_switch",
		EVENT_LOG_TAG_TRACE_CHANSW, &logdump_channel_switch);

	register_eventlog_dump_processing_callback("chnl_switch_rate_cnt",
		EVENT_LOG_TAG_RATE_CNT, &logdump_rate_cnt);

	register_eventlog_dump_processing_callback("chnl_switch_mgt_rate_cnt",
		EVENT_LOG_TAG_CTL_MGT_CNT, &logdump_mgt_rate_cnt);

#ifdef WL_AMPDU_STATS_MAX_CNTS
	register_eventlog_dump_processing_callback("ampdu_dump",
		EVENT_LOG_TAG_AMPDU_DUMP, &logdump_ampdu_dump);
#endif

#if WL_CNT_T_VERSION <= 11
	register_eventlog_dump_processing_callback("wl_cnt_t",
		EVENT_LOG_TAG_WL_COUNTERS, &logdump_wl_counters);
#endif

	register_eventlog_dump_processing_callback("ecounters_ipcstates",
		EVENT_LOG_TAG_ECOUNTERS_IPCSTATS, &logdump_ecounters_ipcstats);

#ifdef logdump_btcx_stats
	register_eventlog_dump_processing_callback("ecounters_btcx_stats",
		EVENT_LOG_TAG_BTCX_STATS, &logdump_btcx_stats);
#endif

	register_eventlog_dump_processing_callback("ecounters_pwrstates_phy",
		EVENT_LOG_TAG_PWRSTATS_PHY, &logdump_wl_powerstats_phy);

	register_eventlog_dump_processing_callback("ecounters_pwrstates_scan",
		EVENT_LOG_TAG_PWRSTATS_SCAN, &logdump_wl_powerstats_scan);


	register_eventlog_dump_processing_callback("ecounters_pwrstates_wake_v2",
		EVENT_LOG_TAG_PWRSTATS_WAKE_V2, &logdump_wl_powerstats_v2);

#ifdef WL_LQM_VERSION_1
	register_eventlog_dump_processing_callback("LQM_struct_log",
		EVENT_LOG_TAG_LQM, &logdump_wl_LQM);
#endif

#ifdef ECOUNTERS_TRIGGER_REASON_VERSION_1
	register_eventlog_dump_processing_callback("ecounters_trigger_reason",
		EVENT_LOG_TAG_ECOUNTERS_TIME_DATA, &ecounters_trigger_reason);
#endif

#ifdef WL_LEAKY_AP_STATS_PKT_TYPE
	register_eventlog_dump_processing_callback("leaky_ap_stats",
		EVENT_LOG_TAG_LEAKY_AP_STATS, &leaky_ap_stat);
#endif
	register_eventlog_dump_processing_callback("scan_summary_stats",
		EVENT_LOG_TAG_SCAN_SUMMARY, &scan_summary_stat);
#ifdef WL_PROXD_TUNE_VERSION
	register_eventlog_dump_processing_callback("proxd_sample_collect",
		EVENT_LOG_TAG_PROXD_SAMPLE_COLLECT, &proxd_sample_collect);
#endif
}
