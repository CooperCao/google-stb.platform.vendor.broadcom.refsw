/*
 * wl awdl command module
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Private1743:http://confluence.broadcom.com/display/WLAN/BuildIPValidation>>
 *
 * $Id: wluc_awdl.c $
 */

#ifdef WIN32
#include <windows.h>
#endif

#include <wlioctl.h>


/* Because IL_BIGENDIAN was removed there are few warnings that need
 * to be fixed. Windows was not compiled earlier with IL_BIGENDIAN.
 * Hence these warnings were not seen earlier.
 * For now ignore the following warnings
 */
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4761)
#endif

#include <bcmutils.h>
#include <bcmendian.h>
#include "wlu_common.h"
#include "wlu.h"

#if defined(linux)
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>
#endif /* linux */

#if defined(linux)
#include <time.h>
#endif

#error "WLAWDL flag is not enabled but file is included for compilation!"

static cmd_func_t wl_awdl_sync_params;
static cmd_func_t wl_awdl_af_hdr;
static cmd_func_t wl_awdl_ext_counts;
static cmd_func_t wl_awdl_opmode;
static cmd_func_t wl_bsscfg_awdl_enable;
static cmd_func_t wl_awdl_mon_bssid;
static cmd_func_t wl_awdl_advertisers;
static cmd_func_t wl_awdl_election_tree;
static cmd_func_t wl_awdl_payload;
static cmd_func_t wl_awdl_long_payload;
static cmd_func_t wl_awdl_chan_seq;
static cmd_func_t wl_awdl_peer_op;
static cmd_func_t wl_awdl_oob_af;
static cmd_func_t wl_awdl_oob_af_async;
static cmd_func_t wl_awdl_oob_af_auto;
static cmd_func_t wl_awdl_uct_test;
static cmd_func_t wl_awdl_counters;
static cmd_func_t wl_awdl_uct_counters;
static cmd_func_t wl_awdl_wowl_sleeper_addr;
static cmd_func_t wl_awdl_pscan;


#define PSCAN_USAGE	"" \
"\tDefault to an active scan across all channels for any SSID.\n" \
"\tOptional arg: SSIDs, list of [up to 10] SSIDs to scan (comma separated).\n" \
"\tOptions:\n" \
"\t-s S, --ssid=S\t\tSSIDs to scan\n" \
"\t-t ST, --scan_type=ST\t[active|passive scan type\n" \
"\t-b MAC, --bssid=MAC\tparticular BSSID MAC address to scan, xx:xx:xx:xx:xx:xx\n" \
"\t-n N, --nprobes=N\tnumber of probes per scanned channel\n" \
"\t-c L, --awseq=L\t\tcomma separated list of aw sequence counters to scan\n" \
"\t-p PT,--pscan_type=PT\t[host|fw|abort] pscan action type\n" \
"\t-r N, --results=N\t zero or non-zero, wait for results or not non-zero means wait \n" \


static cmd_t wl_awdl_cmds[] = {
	{"awdl_sync_params", wl_awdl_sync_params, -1, -1,
	"Send specified \"awdl_sync_params\" action frames Sync params.\n"
	"\tUsage: wl awdl_sync_params <period> <aw period> <aw cmn time>  <Guard time>"
	" <master_chan>\n"},
	{"awdl_af_hdr", wl_awdl_af_hdr, -1, -1,
	"Send specified \"awdl_af_hdr\" action frame header.\n"
	"\tUsage: wl awdl_af_hdr <dest mac > <4 byte hdr> \n"
	"\t\tpacket: Hex packet contents to transmit. \n"
	"\twl awdl_af_hdr xxxxxx \n" },
	{"awdl_extcounts", wl_awdl_ext_counts, -1, -1,
	"Set the AWDL max/min extension counts.\n"
	"\tUsage: wl awdl_ext_counts <minExt> <maxExtMulti> <maxExtUni> <maxAfExt>\n"},
	{"awdl_opmode", wl_awdl_opmode, -1, -1,
	"Set AWDL operating mode.\n"
	"\tUsage: wl awdl_opmode mode role bcast_period master_addr\n"
	"\t\tmode: 0: Auto, 1: Fixed 2: Forced.\n"
	"\t\trole: 0: Slave, 1: NE Master, 2: Master.\n"
	"\t\tbcast_period: bcasting period(ms) for NE master.\n"
	"\t\tmaster_addr: remote master address for fixed mode.\n"},
	{ "awdl_if", wl_bsscfg_awdl_enable, WLC_GET_VAR, WLC_SET_VAR,
	"set/get awdl bsscfg enabled status: up/down \n"},
	{ "awdl_mon_bssid", wl_awdl_mon_bssid, -1, WLC_SET_VAR,
	"set/get awdl monitor BSSID: wl awdl_mon_bssid <BSSID>\n"},
	{ "awdl_advertisers", wl_awdl_advertisers, WLC_GET_VAR, WLC_SET_VAR,
	"set/get awdl peers table\n"},
	{"awdl_election_tree", wl_awdl_election_tree, -1, -1,
	"Send specified \"awdl_election_tree\" Election metrics .\n"
	"\tUsage: wl awdl_election <flag> <id> <top master dis> \n"
	"\t        <master mac> <closeSyncRssiThld> <masterRssiBoost>\n"
	"\t        <edgeSyncRssiThld> <clseRangeRssiThld> \n"
	"\t        <midRangeRssiThld> <maxHigherMastersCloseRange> \n"
	"\t        <maxHigherMastersMidRange> <maxTreeDepth>\n"
	"\t        <edgeMasterDwellTime> <privateElectionID>\n"},
	{"awdl_payload", wl_awdl_payload, -1, -1,
	"Send specified \"awdl_payload\" Payload .\n"
	"\tUsage: wl awdl_payload xxxxxxxx \n"},
	{"awdl_long_psf", wl_awdl_long_payload, -1, -1,
	"Send specified \"awdl_long_psf\" Payload .\n"
	"\tUsage: wl awdl_long_psf <long psf period> <long psf tx offset> xxxxxxxx \n"},
	{"awdl_chan_seq", wl_awdl_chan_seq, -1, -1,
	"Send specified \"awdl_chan_seq\" Channel Sequence .\n"
	"\tUsage: wl awdl_chan_seq <encoding> <Step count> <duplicate> <fill chan> <Sequence>\n"},
	{"awdl_peer_op", wl_awdl_peer_op, -1, WLC_SET_VAR,
	"add or delete awdl peer.\n"
	"\tUsage: \twl awdl_peer_op add <peer addr> [<TLVs hex string>]\n"
	"\t\twl awdl_peer_op del <peer addr>\n"},
	{"awdl_oob_af", wl_awdl_oob_af, -1, WLC_SET_VAR,
	"Send an out of band AWDL action frame.\n"
	"\tUsage: \twl awdl_oob_af <bssid> <dst_mac> <channel> <dwell_time>\n"
	"\t\t <flags> <pkt_lifetime> <tx_rate> <max_retries>\n"
	"\t\t <hex payload>\n"},
	{"awdl_oob_af_async", wl_awdl_oob_af_async, -1, WLC_SET_VAR,
	"Send an out of band AWDL action frame at a specific time.\n"
	"\tUsage: \twl awdl_oob_af_async <bssid> <dst_mac> <channel> <dwell_time>\n"
	"\t\t <flags> <pkt_lifetime> <tx_rate> <max_retries> <tx_time> <pkt_tag>\n"
	"\t\t <hex payload>\n"},
	{"awdl_oob_af_auto", wl_awdl_oob_af_auto, -1, WLC_SET_VAR,
	"Send an out of band AWDL action frame periodically at an offset from aw start.\n"
	"\tUsage: \twl awdl_oob_af_auto <bssid> <dst_mac> <channel> <dwell_time>\n"
	"\t\t <flags> <pkt_lifetime> <tx_rate> <max_retries> <chan_map> <tx_aws_offset>\n"
	"\t\t <hex payload>\n"},
	{"awdl_uct_test", wl_awdl_uct_test, -1, WLC_SET_VAR,
	"Set AW interval and offset.\n"
	"\tUsage: \twl awdl_uct_test <interval> <offset>\n"},
	{ "awdl_stats", wl_awdl_counters, WLC_GET_VAR, -1,
	"Return awdl counter values" },
	{ "awdl_uct_stats", wl_awdl_uct_counters, WLC_GET_VAR, -1,
	"Return awdl uct counter values" },
	{"awdl_wowl_sleeper_addr", wl_awdl_wowl_sleeper_addr, -1, WLC_SET_VAR,
	"Set Mac address of the sleeping device to be awakened\n"
	"\tUsage: \twl awdl_wowl_sleeper_addr <mac>\n"},
	{ "awdl_pscan", wl_awdl_pscan, -1, WLC_SET_VAR,
	"Start an awdl pscan.\n" PSCAN_USAGE
	},
#ifdef WLAWDL_LATENCY_SUPPORT
	{ "awdl_tx_latency_clr", wl_awdl_tx_latency_clr, -1, -1,
	"Clear awdl tx latency (in ms)" },
#endif
	{ NULL, NULL, 0, 0, NULL }
};

static char *buf;

/* module initialization */
void
wluc_awdl_module_init(void)
{
	/* get the global buf */
	buf = wl_get_buf();

	/* register obss commands */
	wl_module_cmds_register(wl_awdl_cmds);
}

#define	PRVAL(name)	pbuf += sprintf(pbuf, "%s %u ", #name, dtoh32(cnt->name))
#define PRVALV1(name) pbuf += sprintf(pbuf, "%s %u ", #name, dtoh32(cnt_v1->name))
#define PRVALV2(name) pbuf += sprintf(pbuf, "%s %u ", #name, dtoh32(cnt_v2->name))
#define PRVALCMN(name) pbuf += sprintf(pbuf, "%s %u ", #name, dtoh32(cnt->cmnstats.name))
#define PRVALCORE(name) pbuf += sprintf(pbuf, "%s %u ", #name, dtoh32(cnt->corestats[i].name))

static int
wl_awdl_counters(void *wl, cmd_t *cmd, char **argv)
{
	char *statsbuf;
	awdl_stats_v3_t *cnt;
	awdl_stats_v2_t *cnt_v2;
	awdl_stats_v1_t *cnt_v1;
	char *pbuf = buf;
	int err;
	void *ptr;
	int i;
	int wlc_ver_maj;

	UNUSED_PARAMETER(argv);

	wlc_ver_maj = wlc_ver_major(wl);

	if ((err = wlu_var_getbuf_med (wl, cmd->name, NULL, 0, &ptr)))
		return (err);
	statsbuf = (char *)ptr;

	/* Check for the firmware version and process accordingly */
	if (wlc_ver_maj < 3) {
		/* Print Bison version if wlc_ver not supported */
		cnt_v1 = (awdl_stats_v1_t*)malloc(sizeof(awdl_stats_v1_t));
		if (cnt_v1 == NULL) {
			printf("Cannot allocate %d bytes for awdl_stats struct\n",
				(int)sizeof(awdl_stats_v1_t));
			return (-1);
		}
		memcpy(cnt_v1, statsbuf, sizeof(awdl_stats_v1_t));
		/* summary stat counter line */
		PRVALV1(aftx); PRVALV1(afrx); PRVALV1(datatx); PRVALV1(datarx);
		PRVALV1(txdrop); PRVALV1(rxdrop); PRVALV1(monrx); PRVALV1(txsupr); PRNL();
		PRVALV1(afrxdrop); PRVALV1(lostmaster);	PRVALV1(misalign); PRVALV1(aw_dur);
		PRVALV1(aws); PRVALV1(rx80211);	PRVALV1(awdrop); PRVALV1(noawchansw); PRNL();
		PRVALV1(debug);	PRVALV1(peeropdrop); PRVALV1(chancal); PRVALV1(nopreawint);
		PRVALV1(psfchanswtchskip);  PRVALV1(psfstateupdskip); PRNL();
#ifdef AWDL_FAMILY
		PRVALV1(awdropchsw); PRVALV1(nopreawchsw); PRVALV1(nopreawprep);
		PRVALV1(infra_offchpsf); PRVALV1(awdl_offchpsf); PRVALV1(pmnoack);
		PRVALV1(scanreq); PRVALV1(chseqreq); PRVALV1(peerdelreq);
		PRVALV1(aws_misalign); PRVALV1(txeval_fail); PRVALV1(infra_reqrcq);
		PRVALV1(awdl_reqtxq);
#endif
	} else if (wlc_ver_maj <= 5) {
		cnt_v2 = (awdl_stats_v2_t*)malloc(sizeof(awdl_stats_v2_t));
		if (cnt_v2 == NULL) {
			printf("Cannot allocate %d bytes for awdl_stats struct\n",
				(int)sizeof(awdl_stats_v2_t));
			return (-1);
		}
		memcpy(cnt_v2, statsbuf, sizeof(awdl_stats_v2_t));
		/* summary stat counter line */
		PRVALV2(aftx); PRVALV2(afrx); PRVALV2(datatx); PRVALV2(datarx);
		PRVALV2(txdrop); PRVALV2(rxdrop); PRVALV2(monrx); PRVALV2(txsupr); PRNL();
		PRVALV2(afrxdrop); PRVALV2(lostmaster);	PRVALV2(misalign); PRVALV2(aw_dur);
		PRVALV2(aws); PRVALV2(rx80211);	PRVALV2(awdrop); PRVALV2(noawchansw); PRNL();
		PRVALV2(debug);	PRVALV2(peeropdrop); PRVALV2(chancal); PRVALV2(nopreawint);
		PRVALV2(psfchanswtchskip);  PRVALV2(psfstateupdskip); PRNL();
#ifdef AWDL_FAMILY
		PRVALV2(awdropchsw); PRVALV2(nopreawchsw); PRVALV2(nopreawprep);
		PRVALV2(aws_misalign); PRVALV2(txeval_fail); PRVALV2(infra_reqrcq);
		PRVALV2(awdl_reqtxq);
#endif
	} else {

		cnt = (awdl_stats_v3_t *)malloc(sizeof(awdl_stats_v3_t));
		if (cnt == NULL) {
			printf("Cannot allocate %d bytes for awdl_stats struct\n",
					(int)sizeof(awdl_stats_v3_t));
			return (-1);
		}
		memcpy(cnt, statsbuf, sizeof(awdl_stats_v3_t));
		if (cnt->version != AWDL_STATS_VERSION_3) {
			printf("Incorrect version of awdl_stats struct: expected %d; got %d\n",
			  AWDL_STATS_VERSION_3, cnt->version);
			err = BCME_USAGE_ERROR;
			goto exit;
		}

		if (cnt->length < (int)sizeof(awdl_stats_v3_t)) {
			printf("Incorrect length: expected %d, got %d\n",
			   (int)sizeof(awdl_stats_v3_t), cnt->length);
			err = BCME_USAGE_ERROR;
			goto exit;
		}
		PRVALCMN(aftx); PRVALCMN(afrx); PRVALCMN(afrxdrop); PRVALCMN(lostmaster);
		PRVALCMN(misalign); PRNL();
		PRVALCMN(aw_dur); PRVALCMN(aws); PRVALCMN(awend); PRVALCMN(awrealign);
		PRVALCMN(awchmismatch); PRVALCMN(awdrop); PRVALCMN(noawchansw); PRNL();
		PRVALCMN(debug); PRVALCMN(peeropdrop); PRVALCMN(chancal); PRVALCMN(nopreawint);
		PRVALCMN(psfchanswtchskip); PRVALCMN(psfstateupdskip); PRNL();
#ifdef AWDL_FAMILY
		PRVALCMN(awdropchsw); PRVALCMN(nopreawchsw); PRVALCMN(nopreawprep);
		PRVALCMN(infra_offchpsf); PRVALCMN(awdl_offchpsf); PRVALCMN(chseqreq);
		PRVALCMN(peerdelreq); PRNL();
		PRVALCMN(aws_misalign); PRVALCMN(txeval_fail); PRVALCMN(infra_reqrcq);
		PRVALCMN(awdl_reqtxq);
#endif
		for (i = 0; i < MAX_NUM_D11CORES; i++) {
			pbuf += sprintf(pbuf, "CORE[%d]:\n", i);
			PRVALCORE(slotstart); PRVALCORE(slotend); PRVALCORE(slotskip);
			PRVALCORE(slotstart_partial); PRVALCORE(slotend_partial); PRNL();
			PRVALCORE(psfstart); PRVALCORE(psfend);	PRVALCORE(psfskip);
			PRVALCORE(psfreqfail); PRVALCORE(psfcnt); PRVALCORE(micnt);
			PRVALCORE(chansw); PRVALCORE(awrealignfail); PRNL();
			PRVALCORE(datatx); PRVALCORE(datarx); PRVALCORE(txdrop);
			PRVALCORE(rxdrop); PRVALCORE(monrx); PRVALCORE(txsupr); PRVALCORE(rx80211);
			PRNL();

		}
	}
	PRNL();
	pbuf += snprintf(pbuf, sizeof(char), "\n");
	fputs(buf, stdout);

exit :
	if (wlc_ver_maj < 3) {
		if (cnt_v1 != NULL)
			free(cnt_v1);
	} else if (wlc_ver_maj <= 5) {
		if (cnt_v2 != NULL)
			free(cnt_v2);
	} else {
		if (cnt)
			free(cnt);
	}

	return (0);
}

static int
wl_awdl_uct_counters(void *wl, cmd_t *cmd, char **argv)
{
	awdl_uct_stats_t cnt_st, *cnt;
	char *pbuf = buf;
	int err;
	void *ptr;
	UNUSED_PARAMETER(argv);

	if ((err = wlu_var_getbuf_med (wl, cmd->name, NULL, 0, &ptr)))
		return (err);

	cnt = &cnt_st;
	memcpy(cnt, ptr, sizeof(cnt_st));

	/* summary stat counter line */
	PRVAL(aw_proc_in_aw_sched);
	PRVAL(aw_upd_in_pre_aw_proc);
	PRVAL(pre_aw_proc_in_aw_set);
	PRVAL(ignore_pre_aw_proc);
	PRVAL(miss_pre_aw_intr);
	PRVAL(aw_dur_zero);
	PRVAL(aw_sched);
	PRVAL(aw_proc);
	PRVAL(pre_aw_proc);
	PRVAL(not_init);
	PRVAL(null_awdl);
	PRNL();

	pbuf += snprintf(pbuf, sizeof(char), "\n");
	fputs(buf, stdout);
	return (0);
}

static int
wl_awdl_wowl_sleeper_addr(void *wl, cmd_t *cmd, char **argv)
{
	struct ether_addr mac;

	UNUSED_PARAMETER(cmd);

	if (*++argv != NULL) {
		if (!wl_ether_atoe(*argv, &mac))
			return -1;

		return wlu_iovar_set(wl, "awdl_wowl_sleeper_addr", &mac, ETHER_ADDR_LEN);
	} else {
		printf("Mac Address not set\n");
		return -1;
	}
}

static int
wl_parse_awseq_list(char* list_str, uint16* awseq_list, int awseq_num)
{
	int num;
	int val;
	char* str;
	char* endptr = NULL;

	if (list_str == NULL)
		return -1;

	str = list_str;
	num = 0;
	while (*str != '\0') {
		val = (int)strtol(str, &endptr, 0);
		if (endptr == str) {
			fprintf(stderr,
				"could not parse awseq number starting at"
				" substring \"%s\" in list:\n%s\n",
				str, list_str);
			return -1;
		}
		str = endptr + strspn(endptr, " ,");

		if (num == awseq_num) {
			fprintf(stderr, "too many awseq (more than %d) in awseq list:\n%s\n",
				awseq_num, list_str);
			return -1;
		}

		awseq_list[num++] = (uint16)val;
	}

	return num;
}

static int
wl_awdl_pscan_prep(void *wl, cmd_t *cmd, char **argv, wl_awdl_pscan_params_t *params,
int *params_size, bool *wait_result, uint16 *action)
{
	int val = 0;
	char key[64];
	int keylen;
	char *p, *eq, *valstr, *endptr = NULL;
	char opt;
	bool positional_param;
	bool good_int;
	bool opt_end;
	int err = 0;
	int i;

	int nawseq = 0;
	int nssid = 0;
	wlc_ssid_t ssids[WL_SCAN_PARAMS_SSID_MAX];

	UNUSED_PARAMETER(wl);
	UNUSED_PARAMETER(cmd);

	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->scan_type = 0;
	params->nprobes = 0;
	params->aw_seq_num = 0;
	memset(ssids, 0, WL_SCAN_PARAMS_SSID_MAX * sizeof(wlc_ssid_t));

	/* skip the command name */
	argv++;

	opt_end = FALSE;
	while ((p = *argv) != NULL) {
		argv++;
		positional_param = FALSE;
		memset(key, 0, sizeof(key));
		opt = '\0';
		valstr = NULL;
		good_int = FALSE;

		if (opt_end) {
			positional_param = TRUE;
			valstr = p;
		}
		else if (!strcmp(p, "--")) {
			opt_end = TRUE;
			continue;
		}
		else if (!strncmp(p, "--", 2)) {
			eq = strchr(p, '=');
			if (eq == NULL) {
				fprintf(stderr,
				"wl_awdl_pscan: missing \" = \" in long param \"%s\"\n", p);
				err = -1;
				goto exit;
			}
			keylen = eq - (p + 2);
			if (keylen > 63) keylen = 63;
			memcpy(key, p + 2, keylen);

			valstr = eq + 1;
			if (*valstr == '\0') {
				fprintf(stderr,
				"wl_awdl_pscan: missing value after \" = \" in long param \"%s\"\n",
				p);
				err = -1;
				goto exit;
			}
		}
		else if (!strncmp(p, "-", 1)) {
			opt = p[1];
			if (strlen(p) > 2) {
				fprintf(stderr,
				"wl_awdl_pscan: only single char options, error on param \"%s\"\n",
				p);
				err = -1;
				goto exit;
			}
			if (*argv == NULL) {
				fprintf(stderr,
				"wl_awdl_pscan: missing value parameter after \"%s\"\n", p);
				err = -1;
				goto exit;
			}
			valstr = *argv;
			argv++;
		} else {
			positional_param = TRUE;
			valstr = p;
		}

		/* parse valstr as int just in case */
		val = (int)strtol(valstr, &endptr, 0);
		if (*endptr == '\0') {
			/* not all the value string was parsed by strtol */
			good_int = TRUE;
		}

		if (opt == 's' || !strcmp(key, "ssid") || positional_param) {
			nssid = wl_parse_ssid_list(valstr, ssids, nssid, WL_SCAN_PARAMS_SSID_MAX);
			if (nssid < 0) {
				err = -1;
				goto exit;
			}
		}

		/* pscan_type to host/fw/abort pscans */
		if (opt == 'p' || !strcmp(key, "pscan_type")) {
			if (!strcmp(valstr, "host")) {
				*action = AWDL_HOST_PSCAN;
				/* do nothing - scan_type is initialized outside of while loop */
			} else if (!strcmp(valstr, "fw")) {
				*action = AWDL_FW_PSCAN;
			} else if (!strcmp(valstr, "abort")) {
				*action = AWDL_ABORT_PSCAN;
			} else {
				fprintf(stderr,
				"pscan_type value should be \"host\", "
				"\"fw\", \"abort\", but got \"%s\"\n", valstr);
				err = -1;
				goto exit;
			}
		}

		/* scan_type is a bitmap value and can have multiple options */
		if (opt == 't' || !strcmp(key, "scan_type")) {
			if (!strcmp(valstr, "active")) {
				/* do nothing - scan_type is initialized outside of while loop */
			} else if (!strcmp(valstr, "passive")) {
				params->scan_type |= WL_SCANFLAGS_PASSIVE;
			} else {
				fprintf(stderr,
				"scan_type value should be \"active\", "
				"\"passive\", but got \"%s\"\n", valstr);
				err = -1;
				goto exit;
			}
		}

		if (opt == 'b' || !strcmp(key, "bssid")) {
			if (!wl_ether_atoe(valstr, &params->bssid)) {
				fprintf(stderr,
				"could not parse \"%s\" as an ethernet MAC address\n", valstr);
				err = -1;
				goto exit;
			}
		}
		if (opt == 'n' || !strcmp(key, "nprobes")) {
			if (!good_int) {
				fprintf(stderr,
				"could not parse \"%s\" as an int for value nprobes\n", valstr);
				err = -1;
				goto exit;
			}
			params->nprobes = val;
		}
		if (opt == 'c' || !strcmp(key, "awseq")) {
			nawseq = wl_parse_awseq_list(valstr, params->aw_counter_list,
			                              WL_NUMCHANNELS);
			printf("awseq %d\n", nawseq);
			if (nawseq == -1) {
				fprintf(stderr, "error parsing channel list arg\n");
				err = -1;
				goto exit;
			}
		}
		if (opt == 'r' || !strcmp(key, "results")) {
			if (!good_int) {
				fprintf(stderr,
				"could not parse \"%s\" result required or not\n",
					valstr);
				err = -1;
				goto exit;
			}
			*wait_result = val ? TRUE : FALSE;
		}
	}

	if (nssid > WL_SCAN_PARAMS_SSID_MAX) {
		fprintf(stderr, "ssid count %d exceeds max of %d\n",
		        nssid, WL_SCAN_PARAMS_SSID_MAX);
		err = -1;
		goto exit;
	}

	params->nprobes = htod32(params->nprobes);

	for (i = 0; i < nawseq; i++) {
		params->aw_counter_list[i] = htod16(params->aw_counter_list[i]);
	}
	params->aw_seq_num = htod32(nawseq);
	for (i = 0; i < nssid; i++) {
		ssids[i].SSID_len = htod32(ssids[i].SSID_len);
	}

	/* For a single ssid, use the single fixed field */
	if (nssid == 1) {
		nssid = 0;
		memcpy(&params->ssid, &ssids[0], sizeof(ssids[0]));
	}

	/* Copy ssid array if applicable */
	if (nssid > 0) {
		i = OFFSETOF(wl_awdl_pscan_params_t, aw_counter_list) + nawseq * sizeof(uint16);
		i = ROUNDUP(i, sizeof(uint32));
		if (i + nssid * sizeof(wlc_ssid_t) > (uint)*params_size) {
			fprintf(stderr, "additional ssids exceed params_size\n");
			err = -1;
			goto exit;
		}

		p = (char*)params + i;
		memcpy(p, ssids, nssid * sizeof(wlc_ssid_t));
		p += nssid * sizeof(wlc_ssid_t);
	} else {
		p = (char*)params->aw_counter_list + nawseq * sizeof(uint16);
	}
	params->nssid = htod32(nssid);
	*params_size = p - (char*)params + nssid * sizeof(wlc_ssid_t);

exit:
	return err;
}


static int
wl_awdl_pscan(void *wl, cmd_t *cmd, char **argv)
{
	int params_size = (WL_AWDL_PSCAN_PARAMS_FIXED_SIZE + OFFSETOF(wl_pscan_params_t, params)) +
	    (WL_AWDL_MAX_NUM_AWSEQ * sizeof(uint16)) + sizeof(wlc_ssid_t) * WL_SCAN_PARAMS_SSID_MAX;
	wl_pscan_params_t *params;
	int err;
	uint16 action = AWDL_HOST_PSCAN;
#if defined(linux)
	uint32 status;
	char *data;
	int event_type;
	int fd, octets;
	struct sockaddr_ll sll;
	struct ifreq ifr;
	char ifnames[IFNAMSIZ] = {"eth1"};
	bcm_event_t *event;
	bool wait_result = TRUE;
	uint8 event_inds_mask[WL_EVENTING_MASK_LEN];	/* 128-bit mask */
	wl_escan_result_t *escan_data;
	struct escan_bss *escan_bss_head = NULL;
	struct escan_bss *escan_bss_tail = NULL;
	struct escan_bss *result;
#else
	bool wait_result = FALSE;
#endif /* linux */

	params_size += OFFSETOF(wl_pscan_params_t, params);
	params = (wl_pscan_params_t *)malloc(params_size);
	if (params == NULL) {
		fprintf(stderr, "Error allocating %d bytes for scan params\n", params_size);
		return -1;
	}
	memset(params, 0, params_size);
	err = wl_awdl_pscan_prep(wl, cmd, argv, &params->params, &params_size,
		&wait_result, &action);

	if (err) {
		printf("wl_awdl_pscan_prep() failed, err = %d\n", err);
		goto exit2;
	}
	params->version = htod32(AWDL_PSCAN_REQ_VERSION);
	params->action = htod16(action);

	srand((unsigned)time(NULL));
	params->sync_id = htod16(rand() & 0xffff);

	params_size += OFFSETOF(wl_pscan_params_t, params);
	err = wlu_iovar_setbuf(wl, "awdl_pscan", params, params_size, buf, WLC_IOCTL_MAXLEN);
	if (err)
		printf("awdl_pscan: err = %d\n", err);

	if (!wait_result || err || (action != AWDL_HOST_PSCAN))
		goto exit2;

#if defined(linux)
	memset(&ifr, 0, sizeof(ifr));
	if (wl)
		strncpy(ifr.ifr_name, ((struct ifreq *)wl)->ifr_name, (IFNAMSIZ - 1));
	else
		strncpy(ifr.ifr_name, ifnames, (IFNAMSIZ - 1));

	memset(event_inds_mask, '\0', WL_EVENTING_MASK_LEN);
	event_inds_mask[WLC_E_ESCAN_RESULT / 8] |= 1 << (WLC_E_ESCAN_RESULT % 8);
	if ((err = wlu_iovar_set(wl, "event_msgs", &event_inds_mask, WL_EVENTING_MASK_LEN)))
		goto exit2;

	fd = socket(PF_PACKET, SOCK_RAW, hton16(ETHER_TYPE_BRCM));
	if (fd < 0) {
		printf("Cannot create socket %d\n", fd);
		err = -1;
		goto exit2;
	}

	err = ioctl(fd, SIOCGIFINDEX, &ifr);
	if (err < 0) {
		printf("Cannot get index..... %d\n", err);
		close(fd);
		goto exit2;
	}

	/* bind the socket first before starting escan so we won't miss any event */
	memset(&sll, 0, sizeof(sll));
	sll.sll_family = AF_PACKET;
	sll.sll_protocol = hton16(ETHER_TYPE_BRCM);
	sll.sll_ifindex = ifr.ifr_ifindex;
	err = bind(fd, (struct sockaddr *)&sll, sizeof(sll));
	if (err < 0) {
		printf("Cannot bind %d\n", err);
		close(fd);
		goto exit2;
	}
	data = (char*)malloc(ESCAN_EVENTS_BUFFER_SIZE);

	if (data == NULL) {
		printf("Cannot not allocate %d bytes for events receive buffer\n",
			ESCAN_EVENTS_BUFFER_SIZE);
		err = -1;
		goto exit2;
	}
	printf("Waiting for SCAN results through ESCAN\n");
	/* receive scan result */
	while (1) {
		octets = recv(fd, data, ESCAN_EVENTS_BUFFER_SIZE, 0);
		if (octets > 0) {
			event = (bcm_event_t *)data;
			event_type = ntoh32(event->event.event_type);

			if (event_type == WLC_E_ESCAN_RESULT) {
				escan_data = (wl_escan_result_t*)&data[sizeof(bcm_event_t)];
				status = ntoh32(event->event.status);
				if (status == WLC_E_STATUS_PARTIAL) {
					wl_bss_info_t *bi = &escan_data->bss_info[0];
					wl_bss_info_t *bss;

					if ((bi->length < sizeof(struct wl_bss_info)) ||
						(bi->length > ETHER_MAX_DATA)) {
						fprintf(stderr,
							"Cannot trust bi->length %d, skipping\n",
							bi->length);
					    continue;
					}

					/* check if we've received info of same BSSID */
					for (result = escan_bss_head; result;
							result = result->next) {
						bss = result->bss;

						if (!memcmp(bi->BSSID.octet, bss->BSSID.octet,
							ETHER_ADDR_LEN) &&
							CHSPEC_BAND(bi->chanspec) ==
							CHSPEC_BAND(bss->chanspec) &&
							bi->SSID_len == bss->SSID_len &&
							!memcmp(bi->SSID, bss->SSID, bi->SSID_len))
							break;
					}

					if (!result) {
						/* New BSS. Allocate memory and save it */
						struct escan_bss *ebss = malloc(
							OFFSETOF(struct escan_bss, bss)	+
							bi->length);

						if (!ebss) {
							perror("can't allocate memory for bss");
							goto exit1;
						}

						ebss->next = NULL;
						memcpy(&ebss->bss, bi, bi->length);
						if (escan_bss_tail) {
							escan_bss_tail->next = ebss;
						}
						else {
							escan_bss_head = ebss;
						}
						escan_bss_tail = ebss;
					}
					else if (bi->RSSI != WLC_RSSI_INVALID) {
						/* We've got this BSS. Update rssi if necessary */
						if (((bss->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL) ==
						    (bi->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL)) &&
						    ((bss->RSSI == WLC_RSSI_INVALID) ||
							(bss->RSSI < bi->RSSI))) {
							/* preserve max RSSI if the measurements are
							 * both on-channel or both off-channel
							 */
							bss->RSSI = bi->RSSI;
							bss->SNR = bi->SNR;
							bss->phy_noise = bi->phy_noise;
						} else if ((bi->flags &
							WL_BSS_FLAGS_RSSI_ONCHANNEL) &&
							(bss->flags &
							WL_BSS_FLAGS_RSSI_ONCHANNEL) == 0) {
							/* preserve the on-channel rssi measurement
							 * if the new measurement is off channel
							*/
							bss->RSSI = bi->RSSI;
							bss->SNR = bi->SNR;
							bss->phy_noise = bi->phy_noise;
							bss->flags |= WL_BSS_FLAGS_RSSI_ONCHANNEL;
						}
					}
				}
				else if (status == WLC_E_STATUS_SUCCESS) {
					/* Escan finished. Let's go dump the results. */
					break;
				}
				else {
					printf("sync_id: %d, status:%d, misc. error/abort\n",
						escan_data->sync_id, status);
					goto exit1;
				}
			}

		}
	}

	/* print scan results */
	for (result = escan_bss_head; result; result = result->next) {
		dump_bss_info(result->bss);
	}

exit1:
	/* free scan results */
	result = escan_bss_head;
	while (result) {
		struct escan_bss *tmp = result->next;
		free(result);
		result = tmp;
	}

	free(data);
	close(fd);
#endif /* linux */
exit2:
	free(params);
	return err;
}
/* Send a periodic bcast/mcast action frame at the specificed intervalm at low power */
static int
wl_awdl_sync_params(void *wl, cmd_t *cmd, char **argv)
{
	const char		*str;
	awdl_sync_params_t	awdl_config;
	awdl_sync_params_t	*pawdl_config;
	int	buf_len;
	int	str_len;
	int	rc;
	void	*ptr = NULL;

	if (*++argv == NULL) {
	   /*
	   ** Get current AWDL params.
	   */
		if ((rc = wlu_var_getbuf(wl, cmd->name, NULL, 0, &ptr)) < 0)
			return rc;

		pawdl_config = (awdl_sync_params_t *) ptr;

		printf("AFPeriod(msec):%d\n"
		       "AW Period     :%d\n"
		       "AW Cmn time   :%d\n"
		       "Guard timr    :%d\n"
		       "Ext period    :%d\n"
		       "AWDL Flags    :%#04x\n"
		       "Master chan   :%d\n",
		        dtoh16(pawdl_config->action_frame_period),
		        dtoh16(pawdl_config->aw_period),
		        dtoh16(pawdl_config->aw_cmn_length),
		        pawdl_config->guard_time,
		        dtoh16(pawdl_config->aw_ext_length),
		        dtoh16(pawdl_config->awdl_flags),
		        pawdl_config->master_chan);
		printf("\n");
	}
	else {
		/*
		** Set the attributes.
		*/

		str = "awdl_sync_params";
		str_len = strlen(str);
		strncpy(buf, str, str_len);
		buf[ str_len ] = '\0';
		bzero(&awdl_config, sizeof(awdl_sync_params_t));
		pawdl_config = (awdl_sync_params_t *) (buf + str_len + 1);
		awdl_config.action_frame_period = htod16(strtoul(*argv, NULL, 0));
		awdl_config.aw_period = htod16(strtoul(*(argv + 1), NULL, 0));
		awdl_config.aw_cmn_length = htod16(strtoul(*(argv + 2), NULL, 0));
		awdl_config.guard_time = htod16(strtoul(*(argv + 3), NULL, 0));
		awdl_config.aw_ext_length = htod16(strtoul(*(argv + 4), NULL, 0));
		awdl_config.master_chan = strtoul(*(argv + 5), NULL, 0);
		buf_len = str_len + 1 + sizeof(awdl_sync_params_t);

		memcpy((char *)pawdl_config, &awdl_config,
			sizeof(awdl_sync_params_t));

		rc = wlu_set(wl,
		            WLC_SET_VAR,
		            buf,
		            buf_len);

	}

	return (rc);
}

static int
wl_awdl_af_hdr(void *wl, cmd_t *cmd, char **argv)
{
	const char *str;
	int	buf_len;
	int	str_len;
	int	i;
	int	rc;
	void	*ptr = NULL;
	awdl_af_hdr_t *paf_hdr;

	if (*++argv == NULL) {
	   /*
	   ** Get current AWDL action frame.
	   */
		if ((rc = wlu_var_getbuf(wl, cmd->name, NULL, 0, &ptr)) < 0)
			return rc;
		paf_hdr = (awdl_af_hdr_t *) ptr;

		printf("Dst Addr      :%02x:%02x:%02x:%02x:%02x:%02x \n",
			paf_hdr->dst_mac.octet[0],
			paf_hdr->dst_mac.octet[1],
			paf_hdr->dst_mac.octet[2],
			paf_hdr->dst_mac.octet[3],
			paf_hdr->dst_mac.octet[4],
			paf_hdr->dst_mac.octet[5]);

		printf("Category and OUI[3]    :0x");
		for (i = 0; i < 4; i++)
			printf("%02x", paf_hdr->action_hdr[i]);

		printf("\n");
	}
	else {
		/*
		** Set the attributes.
		*/

		str = "awdl_af_hdr";
		str_len = strlen(str);
		strncpy(buf, str, str_len);
		buf[ str_len ] = '\0';

		paf_hdr = (awdl_af_hdr_t *) (buf + str_len + 1);
		if (*argv)
			if (!wl_ether_atoe(*argv, (struct ether_addr *)&paf_hdr->dst_mac)) {
				printf("Problem parsing MAC address \"%s\".\n", *argv);
				return -1;
			}
		argv++;
		buf_len = str_len + 1 + sizeof(awdl_sync_params_t);
		htod16(wl_pattern_atoh(*argv, (char *) paf_hdr->action_hdr));

		rc = wlu_set(wl,
		            WLC_SET_VAR,
		            buf,
		            buf_len);

	}

	return (rc);
}

static int
wl_awdl_ext_counts(void *wl, cmd_t *cmd, char **argv)
{
	const char *str;
	int	buf_len;
	int	str_len;
	int	rc;
	void	*ptr = NULL;
	awdl_extcount_t extcnt, *pextcnt;

	if (*++argv == NULL) {
	   /*
	   ** Get current AWDL action frame.
	   */
		if ((rc = wlu_var_getbuf(wl, cmd->name, NULL, 0, &ptr)) < 0)
			return rc;
		pextcnt = (awdl_extcount_t *) ptr;

		memcpy(&extcnt, (char *)pextcnt, sizeof(extcnt));
		printf("MinExt:\t%d\nMaxExtMulti:\t%d\nMaxExtUni:\t%d\nMaxExt:\t%d\n",
			extcnt.minExt, extcnt.maxExtMulti, extcnt.maxExtUni, extcnt.maxAfExt);
	}
	else {
		/*
		** Set the attributes.
		*/
		str = "awdl_extcounts";
		str_len = strlen(str);
		strncpy(buf, str, str_len);
		buf[ str_len ] = '\0';

		pextcnt = (awdl_extcount_t *) (buf + str_len + 1);
		extcnt.minExt = strtoul(*argv++, NULL, 0);
		if (*argv) {
			extcnt.maxExtMulti = strtoul(*argv++, NULL, 0);
		} else {
			return BCME_BADARG;
		}
		if (*argv) {
			extcnt.maxExtUni = strtoul(*argv++, NULL, 0);
		} else {
			return BCME_BADARG;
		}
		if (*argv) {
			extcnt.maxAfExt = strtoul(*argv++, NULL, 0);
		} else {
			return BCME_BADARG;
		}
		memcpy((char *)pextcnt, &extcnt, sizeof(extcnt));

		buf_len = str_len + 1 + sizeof(awdl_extcount_t);

		rc = wlu_set(wl,
		            WLC_SET_VAR,
		            buf,
		            buf_len);
	}

	return (rc);
}

static int
wl_awdl_opmode(void *wl, cmd_t *cmd, char **argv)
{
	int			rc;
	awdl_opmode_un_t	opmode_un;
	unsigned short		bufsiz = 0;

	if (*++argv == NULL) {
		awdl_opmode_t opmode;
		void *ptr = NULL;
		if ((rc = wlu_var_getbuf(wl, cmd->name, NULL, 0, &ptr)) < 0)
			return rc;
		awdl_opmode_t *popmode;
		popmode = (awdl_opmode_t *)ptr;
		memcpy(&opmode, (char *)popmode, sizeof(opmode));
		printf("Mode:\t%d\nRole:\t%d\n",
			opmode.mode, opmode.role);
		printf(" Non-Election Bcasting Period: %d\n Current Bcasting Period: %d\n",
			opmode.bcast_tu, opmode.cur_bcast_tu);
		printf("Master Addr :%02x:%02x:%02x:%02x:%02x:%02x \n",
			opmode.master.octet[0], opmode.master.octet[1],
			opmode.master.octet[2], opmode.master.octet[3],
			opmode.master.octet[4], opmode.master.octet[5]);
	}
	else {
		awdl_opmode_t *opmode = NULL;
		/*
		 * Check the if_ver, 3 indicates DINGO - size is v2 & the rest all use v1/v3.
		 */
		if (wlc_ver_major(wl) == 3) {
			opmode = (awdl_opmode_t *)&opmode_un.opmode_v2;
			bufsiz = sizeof(((awdl_opmode_un_t *)0)->opmode_v2);
		} else {
			/* If its any other version or unsupported version */
			opmode = &opmode_un.opmode_v1;
			bufsiz = sizeof(((awdl_opmode_un_t *)0)->opmode_v1);
		}
		/*
		 * Set the attributes.
		 */
		opmode->mode = strtoul(*argv++, NULL, 0);
		if (*argv) {
			opmode->role = strtoul(*argv++, NULL, 0);
			if (*argv)
				opmode->bcast_tu = strtoul(*argv++, NULL, 0);
			else
				opmode->bcast_tu = 1000; /* default */
		}
		if (*argv)
			if (!wl_ether_atoe(*argv, &opmode->master)) {
				printf("Problem parsing MAC address \"%s\".\n", *(argv + 3));
				if (opmode->mode == AWDL_OPMODE_FORCED)
					memset(&opmode->master, 0, sizeof(opmode->master));
				else
					return -1;
			}
		rc = wlu_iovar_set(wl, "awdl_opmode", opmode, bufsiz);
	}
	return (rc);
}

static int
wl_awdl_election_tree(void *wl, cmd_t *cmd, char **argv)
{
	const char			*str;
	awdl_election_tree_info_t	*pawdl_config;
	int	buf_len;
	int	str_len;
	int	rc;
	void	*ptr = NULL;

	if (*++argv == NULL) {
	   /*
	   ** Get current AWDL election info.
	   */
		if ((rc = wlu_var_getbuf(wl, cmd->name, NULL, 0, &ptr)) < 0)
			return rc;

		pawdl_config = (awdl_election_tree_info_t *) ptr;

		printf("Flags		:%d\n"
		       "ID		:%d\n"
		       "Self Metric		:%d\n",
		        pawdl_config->election_flags,
		        pawdl_config->election_ID,
		        pawdl_config->self_metrics);

		printf("RSSI thresholds:\n"
			"    close  sync		:%d\n"
			"    master rssi boost	:%d\n"
			"    edge   sync		:%d\n"
			"    close range		:%d\n"
			"    mid range		:%d\n",
			pawdl_config->close_sync_rssi_thld,
			pawdl_config->master_rssi_boost,
			pawdl_config->edge_sync_rssi_thld,
			pawdl_config->close_range_rssi_thld,
			pawdl_config->mid_range_rssi_thld);

		printf("Master dwell count:\n"
			"    Edge master		:%d\n",
			pawdl_config->edge_master_dwell_cnt);

		printf("Max number of higher masters:\n"
			"    close range		:%d\n"
			"    mid range			:%d\n"
			"Max tree depth		:%d\n",
			pawdl_config->max_higher_masters_close_range,
			pawdl_config->max_higher_masters_mid_range,
			pawdl_config->max_tree_depth);

		printf("Current top master:\n");
		printf("    addr		: %02x:%02x:%02x:%02x:%02x:%02x\n",
			pawdl_config->top_master.octet[0],
			pawdl_config->top_master.octet[1],
			pawdl_config->top_master.octet[2],
			pawdl_config->top_master.octet[3],
			pawdl_config->top_master.octet[4],
			pawdl_config->top_master.octet[5]);

		printf("    self metric    :%d\n", pawdl_config->top_master_self_metric);
		printf("Current tree depth:%d\n", pawdl_config->current_tree_depth);

		if (pawdl_config->election_flags) {
			printf("\nPrivate election ID :%d\n", pawdl_config->private_election_ID);
			printf("Current private top master:\n");
			printf("    addr		: %02x:%02x:%02x:%02x:%02x:%02x\n",
				pawdl_config->private_top_master.octet[0],
				pawdl_config->private_top_master.octet[1],
				pawdl_config->private_top_master.octet[2],
				pawdl_config->private_top_master.octet[3],
				pawdl_config->private_top_master.octet[4],
				pawdl_config->private_top_master.octet[5]);

			printf("    self metric    :%d\n", pawdl_config->private_top_master_metric);
			printf("Current private tree depth:%d\n",
				pawdl_config->private_distance_from_top);
		}

	} else {
		/*
		** Set the attributes.
		*/

		str = "awdl_election_tree";
		str_len = strlen(str);
		strncpy(buf, str, str_len);
		buf[ str_len ] = '\0';

		pawdl_config = (awdl_election_tree_info_t *) (buf + str_len + 1);
		pawdl_config->election_flags = strtoul(*argv, NULL, 0);
		pawdl_config->election_ID = strtoul(*(argv + 1), NULL, 0);
		pawdl_config->self_metrics = strtoul(*(argv + 2), NULL, 0);

		pawdl_config->close_sync_rssi_thld = strtoul(*(argv+3), NULL, 0);
		pawdl_config->master_rssi_boost = strtoul(*(argv+4), NULL, 0);
		pawdl_config->edge_sync_rssi_thld = strtoul(*(argv+5), NULL, 0);
		pawdl_config->close_range_rssi_thld = strtoul(*(argv+6), NULL, 0);
		pawdl_config->mid_range_rssi_thld = strtoul(*(argv+7), NULL, 0);
		pawdl_config->max_higher_masters_close_range = strtoul(*(argv+8), NULL, 0);
		pawdl_config->max_higher_masters_mid_range = strtoul(*(argv+9), NULL, 0);
		pawdl_config->max_tree_depth = strtoul(*(argv+10), NULL, 0);

		pawdl_config->edge_master_dwell_cnt = strtoul(*(argv+11), NULL, 0);
		pawdl_config->private_election_ID = strtoul(*(argv+12), NULL, 0);

		buf_len = str_len + 1 + sizeof(awdl_election_tree_info_t);

		rc = wlu_set(wl,
		            WLC_SET_VAR,
		            buf,
		            buf_len);

	}
	return (rc);
}


static int
wl_awdl_payload(void *wl, cmd_t *cmd, char **argv)
{
	const char	*str;
	awdl_payload_t	*pawdl_config;
	int	buf_len;
	int	str_len;
	int	rc;
	uint32	i;
	void	*ptr = NULL;

	if (*++argv == NULL) {
	   /*
	   ** Get current AWDL params.
	   */
		if ((rc = wlu_var_getbuf(wl, cmd->name, NULL, 0, &ptr)) < 0)
			return rc;

		pawdl_config = (awdl_payload_t *) ptr;
		printf("Length	      :%d\n"
			"Packet	      :0x",
			dtoh16(pawdl_config->len));

		for (i = 0; i < pawdl_config->len; i++)
			printf("%02x", pawdl_config->payload[i]);
		printf("\n");
	}
	else {
		/*
		** Set the attributes.
		*/

		str = "awdl_payload";
		str_len = strlen(str);
		strncpy(buf, str, str_len);
		buf[ str_len ] = '\0';

		pawdl_config = (awdl_payload_t *) (buf + str_len + 1);
		pawdl_config->len = htod16(wl_pattern_atoh(*argv, (char *) pawdl_config->payload));
		buf_len = str_len + 1 + pawdl_config->len + sizeof(uint16);


		rc = wlu_set(wl,
		            WLC_SET_VAR,
		            buf,
		            buf_len);

	}
	return (rc);
}

static int
wl_awdl_long_payload(void *wl, cmd_t *cmd, char **argv)
{
	const char		*str;
	awdl_long_payload_t	*pawdl_config;
	int	buf_len;
	int	str_len;
	int	rc, i;
	void	*ptr = NULL;

	if (*++argv == NULL) {
	   /*
	   ** Get current AWDL params.
	   */
		if ((rc = wlu_var_getbuf(wl, cmd->name, NULL, 0, &ptr)) < 0)
			return rc;

		pawdl_config = (awdl_long_payload_t *) ptr;
		printf("long psf period	      :%d\n"
			"long psf tx offset	      :%d\n"
			"Length	      :%d\n"
			"Packet	      :0x",
			pawdl_config->long_psf_period,
			pawdl_config->long_psf_tx_offset,
			dtoh16(pawdl_config->len));

		for (i = 0; i < pawdl_config->len; i++)
			printf("%02x", pawdl_config->payload[i]);
		printf("\n");
	}
	else {
		/*
		** Set the attributes.
		*/

		str = "awdl_long_psf";
		str_len = strlen(str);
		strncpy(buf, str, str_len);
		buf[ str_len ] = '\0';

		pawdl_config = (awdl_long_payload_t *) (buf + str_len + 1);

		pawdl_config->long_psf_period = strtoul(*argv, NULL, 0);
		pawdl_config->long_psf_tx_offset = strtoul(*(argv + 1), NULL, 0);
		pawdl_config->len = 0;
		if (*(argv + 2))
			pawdl_config->len = htod16(wl_pattern_atoh(*(argv + 2),
			(char *)pawdl_config->payload));
		buf_len = str_len + 1 + pawdl_config->len + 2 + sizeof(uint16);

		rc = wlu_set(wl,
		            WLC_SET_VAR,
		            buf,
		            buf_len);
	}
	return (rc);
}


static int
wl_awdl_chan_seq(void *wl, cmd_t *cmd, char **argv)
{
	const char	*str;
	awdl_channel_sequence_t	*pawdl_config;
	int	buf_len;
	int	str_len;
	int	rc, i;
	void	*ptr = NULL;

	if (*++argv == NULL) {
	   /*
	   ** Get current AWDL params.
	   */
		if ((rc = wlu_var_getbuf(wl, cmd->name, NULL, 0, &ptr)) < 0)
			return rc;

		pawdl_config = (awdl_channel_sequence_t *) ptr;

		printf("Seq Len		:%d\n"
			"Encoding	:%d\n"
			"Step count	:%d\n"
			"duplicate	:%d\n"
			"fill channel	:%d\n"
			"Chan Sequence   :",
			pawdl_config->aw_seq_len,
			pawdl_config->aw_seq_enc,
			pawdl_config->seq_step_cnt,
			pawdl_config->aw_seq_duplicate_cnt,
			pawdl_config->seq_fill_chan);

		for (i = 0; i < (pawdl_config->aw_seq_len + 1) *
			((pawdl_config->aw_seq_enc? 1 : 0) + 1); i++)
			printf("%02x", pawdl_config->chan_sequence[i]);
		printf("\n");
	}
	else {
		/*
		** Set the attributes.
		*/

		str = "awdl_chan_seq";
		str_len = strlen(str);
		strncpy(buf, str, str_len);
		buf[ str_len ] = '\0';

		pawdl_config = (awdl_channel_sequence_t *) (buf + str_len + 1);
		pawdl_config->aw_seq_enc = strtoul(*argv, NULL, 0);
		pawdl_config->seq_step_cnt = strtoul(*(argv + 1), NULL, 0);
		pawdl_config->aw_seq_duplicate_cnt = strtoul(*(argv + 2), NULL, 0);
		pawdl_config->seq_fill_chan = strtoul(*(argv + 3), NULL, 0);
		pawdl_config->aw_seq_len = 0;
		if (*(argv + 4))
			pawdl_config->aw_seq_len += wl_pattern_atoh(*(argv + 4),
			(char *)pawdl_config->chan_sequence);
		buf_len = str_len + 1 + pawdl_config->aw_seq_len + WL_AWDL_CHAN_SEQ_FIXED_LEN;
		pawdl_config->aw_seq_len /= ((pawdl_config->aw_seq_enc? 1 : 0) + 1);
		pawdl_config->aw_seq_len--;

		rc = wlu_set(wl,
		            WLC_SET_VAR,
		            buf,
		            buf_len);

	}
	return (rc);
}


static int
wl_bsscfg_awdl_enable(void *wl, cmd_t *cmd, char **argv)
{
	char *endptr;
	const char *val_name = "awdl_if";
	int bsscfg_idx = 0;
	int val;
	int consumed;
	int ret;

	UNUSED_PARAMETER(cmd);

	/* skip the command name */
	argv++;

	memset(buf, 0, WLC_IOCTL_MAXLEN);

	/* parse a bsscfg_idx option if present */
	if ((ret = wl_cfg_option(argv, val_name, &bsscfg_idx, &consumed)) != 0)
		return ret;

	argv += consumed;
	if (consumed == 0) { /* Use the -i parameter if that was present */
		bsscfg_idx = -1;
	}

	if (!*argv) {
		bsscfg_idx = htod32(bsscfg_idx);
		ret = wlu_iovar_getbuf(wl, val_name, &bsscfg_idx, sizeof(bsscfg_idx),
			buf, WLC_IOCTL_MAXLEN);
		if (ret < 0)
			return ret;
		val = *(int*)buf;
		val = dtoh32(val);
		if (val)
			printf("up\n");
		else
			printf("down\n");
		return 0;
	} else {
		wl_awdl_if2_t bss_setbuf;

		bzero(&bss_setbuf, sizeof(wl_awdl_if2_t));
		if (!wl_ether_atoe(*argv, &bss_setbuf.bssid))
			return -1;
		argv++;
		if (!stricmp(*argv, "up"))
			val = 1;
		else if (!stricmp(*argv, "down"))
			val = 0;
		else {
			val = strtol(*argv, &endptr, 0);
			if (*endptr != '\0') {
				/* not all the value string was parsed by strtol */
				fprintf(stderr, "error!\n");
				return -1;
			}
		}
		bss_setbuf.cfg_idx = htod32(bsscfg_idx);
		bss_setbuf.up = htod32(val);
		ret = wlu_iovar_set(wl, val_name, &bss_setbuf, sizeof(bss_setbuf));
		return ret;
	}
}

static int
wl_awdl_mon_bssid(void *wl, cmd_t *cmd, char **argv)
{
	struct ether_addr bssid;

	UNUSED_PARAMETER(cmd);

	/* skip the command name */
	argv++;

	if (!wl_ether_atoe(*argv, &bssid))
		return -1;

	return wlu_iovar_set(wl, "awdl_mon_bssid", &bssid, ETHER_ADDR_LEN);
}

static int
wl_awdl_advertisers(void *wl, cmd_t *cmd, char **argv)
{
	awdl_peer_table_t	*pawdl_peer_tbl;
	awdl_peer_node_t	*pawdl_peer;
	uint32	rem_len;
	int	rc = BCME_OK;
	void	*ptr = NULL;

	if (*++argv == NULL) {
	   /*
	   ** Get current AWDL params.
	   */
		if ((rc = wlu_var_getbuf(wl, cmd->name, NULL, 0, &ptr)) < 0)
			return rc;
		pawdl_peer_tbl = (awdl_peer_table_t *) ptr;
		rem_len = pawdl_peer_tbl->len;
		pawdl_peer = (awdl_peer_node_t *)pawdl_peer_tbl->peer_nodes;
		while (rem_len >= sizeof(awdl_peer_node_t)) {
			printf("----------------------------\n");
			printf("Address    : %02x:%02x:%02x:%02x:%02x:%02x\n",
				pawdl_peer->addr.octet[0],
				pawdl_peer->addr.octet[1],
				pawdl_peer->addr.octet[2],
				pawdl_peer->addr.octet[3],
				pawdl_peer->addr.octet[4],
				pawdl_peer->addr.octet[5]);
			printf("Top Master : %02x:%02x:%02x:%02x:%02x:%02x\n",
				pawdl_peer->top_master.octet[0],
				pawdl_peer->top_master.octet[1],
				pawdl_peer->top_master.octet[2],
				pawdl_peer->top_master.octet[3],
				pawdl_peer->top_master.octet[4],
				pawdl_peer->top_master.octet[5]);
			printf("Type/state        : %x\n"
			       "AF rssi (avg)     : %d\n"
			       "AF rssi (last)    : %d\n"
			       "AW period         : %d\n"
			       "Aw counter        : %d\n"
			       "AW Cmn time       : %d\n"
			       "Tx counter        : %d\n"
			       "Tx delay          : %d\n"
			       "Aw Ext length     : %d\n"
			       "Action period     : %d\n"
			       "Election Metrics  : %d\n"
			       "Top Master Metrics: %d\n"
			       "Distance From Top : %d\n",
			        dtoh16(pawdl_peer->type_state),
			        pawdl_peer->rssi,
			        pawdl_peer->last_rssi,
			        dtoh16(pawdl_peer->aw_period),
			        dtoh16(pawdl_peer->aw_counter),
			        dtoh16(pawdl_peer->aw_cmn_length),
			        dtoh16(pawdl_peer->tx_counter),
			        dtoh16(pawdl_peer->tx_delay),
			        dtoh16(pawdl_peer->aw_ext_length),
			        dtoh16(pawdl_peer->period_tu),
			        dtoh32(pawdl_peer->self_metrics),
			        dtoh32(pawdl_peer->top_master_metrics),
			        pawdl_peer->dist_top);

			if (pawdl_peer->has_private_election_params) {
				printf("Private top Master : %02x:%02x:%02x:%02x:%02x:%02x\n",
					pawdl_peer->private_top_master.octet[0],
					pawdl_peer->private_top_master.octet[1],
					pawdl_peer->private_top_master.octet[2],
					pawdl_peer->private_top_master.octet[3],
					pawdl_peer->private_top_master.octet[4],
					pawdl_peer->private_top_master.octet[5]);
				printf(
			       "Private election ID			: %d\n"
			       "Private top master metric		: %d\n"
			       "Private distance From Top		: %d\n",
			        dtoh32(pawdl_peer->private_election_ID),
			        dtoh32(pawdl_peer->private_top_master_metric),
			        pawdl_peer->private_distance_from_top);
			} else
				printf("No private election params\n");

			printf("-----------------------------\n");
			rem_len -= sizeof(awdl_peer_node_t);
			pawdl_peer++;
		}
	}

	return (rc);
}
static int
wl_awdl_peer_op(void *wl, cmd_t *cmd, char **argv)
{
	awdl_peer_op_t *peer_op;
	int peer_op_len = 0;
	int rc = BCME_OK;
	int count;
	void	*ptr = NULL;

	count = ARGCNT(argv);

	/* GET operation */
	if (count < 2) {
		awdl_peer_op_tbl_t *pawdl_peer_tbl;
		awdl_peer_op_node_t *pawdl_peer;
		uint32	rem_len, i;
		if ((rc = wlu_var_getbuf(wl, cmd->name, NULL, 0, &ptr)) < 0)
			return rc;
		pawdl_peer_tbl = (awdl_peer_op_tbl_t *) ptr;
		rem_len = pawdl_peer_tbl->len;
		if (!rem_len)
			goto exit;

		pawdl_peer = (awdl_peer_op_node_t *)pawdl_peer_tbl->tbl;
		printf("----------------------------------------------------------------------\n");
		printf(" Address \t       Flags \t         Chanseq \n");
		while (rem_len >= (sizeof(awdl_peer_op_node_t) - sizeof(uint8))) {
			printf("%02x:%02x:%02x:%02x:%02x:%02x\t",
				pawdl_peer->addr.octet[0],
				pawdl_peer->addr.octet[1],
				pawdl_peer->addr.octet[2],
				pawdl_peer->addr.octet[3],
				pawdl_peer->addr.octet[4],
				pawdl_peer->addr.octet[5]);
			printf("0x%x\t", pawdl_peer->flags);
			if (pawdl_peer->chanseq_len)
				for (i = 0; i < pawdl_peer->chanseq_len; i++)
					printf("%x ", pawdl_peer->chanseq[i]);
			printf("\n");

			peer_op_len = sizeof(awdl_peer_op_node_t) - sizeof(uint8) +
				pawdl_peer->chanseq_len;
			rem_len -= peer_op_len;
			pawdl_peer = (awdl_peer_op_node_t *)((uint8 *)pawdl_peer + peer_op_len);
		}
		printf("----------------------------------------------------------------------\n");
		goto exit;
	}

	peer_op = (awdl_peer_op_t *)buf;
	peer_op->version = AWDL_PEER_OP_CUR_VER;

	/* opcode */
	if (strcmp(argv[1], "add") == 0)
		peer_op->opcode = AWDL_PEER_OP_ADD;
	else if (strcmp(argv[1], "del") == 0)
		peer_op->opcode = AWDL_PEER_OP_DEL;
	else {
		fprintf(stderr, "operation '%s' is not supported\n", argv[1]);
		rc = -1;
		goto exit;
	}

	if (count < 3) {
		fprintf(stderr, "Peer address is expected\n");
		rc = -1;
		goto exit;
	}

	/* peer address */
	if (!wl_ether_atoe(argv[2], &peer_op->addr)) {
		fprintf(stderr, "Peer address parse error\n");
		rc = -1;
		goto exit;
	}

	if (count < 4) {
		fprintf(stderr, "Peer Operation mode is expected\n");
		rc = -1;
		goto exit;
	}
	peer_op->mode = strtoul(argv[3], NULL, 0);

	peer_op_len = sizeof(awdl_peer_op_t);

	/* TLV string */
	if (count > 4)
		peer_op_len += wl_pattern_atoh(argv[4], (char *)peer_op + peer_op_len);

	rc = wlu_iovar_set(wl, cmd->name, peer_op, peer_op_len);

exit:
	return rc;
}


static int
wl_awdl_oob_af_cmn(int count, char **argv, awdl_oob_af_params_t *af_params)
{
	int rc = BCME_OK;

	if (!wl_ether_atoe(argv[1], &af_params->bssid)) {
		fprintf(stderr, "Invalid BSSID\n");
		rc = -1;
		goto exit;
	}

	if (count < 3) {
		fprintf(stderr, "Dst mac is expected\n");
		rc = -1;
		goto exit;
	}

	if (!wl_ether_atoe(argv[2], &af_params->dst_mac)) {
		fprintf(stderr, "Invalid dst mac\n");
		rc = -1;
		goto exit;
	}

	if (count < 4) {
		fprintf(stderr, "Channel is expected\n");
		rc = -1;
		goto exit;
	}

	af_params->channel = htod32(strtoul(*(argv + 3), NULL, 0));

	if (count < 5) {
		fprintf(stderr, "Dwell time is expected\n");
		rc = -1;
		goto exit;
	}

	af_params->dwell_time = htod32(strtoul(*(argv + 4), NULL, 0));

	if (count < 6) {
		fprintf(stderr, "Flags are expected\n");
		rc = -1;
		goto exit;
	}

	af_params->flags = htod32(strtoul(*(argv + 5), NULL, 0));

	if (count < 7) {
		fprintf(stderr, "Pkt lifetime is expected\n");
		rc = -1;
		goto exit;
	}

	af_params->pkt_lifetime = htod32(strtoul(*(argv + 6), NULL, 0));

	if (count < 8) {
		fprintf(stderr, "Tx rate is expected\n");
		rc = -1;
		goto exit;
	}

	af_params->tx_rate = htod32(strtoul(*(argv + 7), NULL, 0));

	if (count < 9) {
		fprintf(stderr, "Max retries count is expected\n");
		rc = -1;
		goto exit;
	}

	af_params->max_retries = htod32(strtoul(*(argv + 8), NULL, 0));

exit:
	return rc;
}


static int
wl_awdl_oob_af(void *wl, cmd_t *cmd, char **argv)
{
	awdl_oob_af_params_t *af_params;
	int rc = BCME_OK;
	int err;
	int count;
	const char *str;
	int str_len;
	int buf_len;

	count = ARGCNT(argv);

	UNUSED_PARAMETER(cmd);

	/* GET operation not supported */
	if (count < 2) {
		fprintf(stderr, "GET operation is not supported\n");
		rc = -1;
		goto exit;
	}

	str = "awdl_oob_af";
	str_len = strlen(str);
	strncpy(buf, str, str_len);
	buf[ str_len ] = '\0';

	af_params = (awdl_oob_af_params_t *) (buf + str_len + 1);

	rc = wl_awdl_oob_af_cmn(count, argv, af_params);
	if (rc != BCME_OK)
		goto exit;

	if (count < 10) {
		fprintf(stderr, "Payload is expected\n");
		rc = -1;
		goto exit;
	}

	/* Variable length payload */
	af_params->payload_len = htod16(strlen(argv[9])) / 2;

	if ((err = get_ie_data ((uchar *)argv[9],
		&af_params->payload[0],
		af_params->payload_len))) {
		fprintf(stderr, "Error parsing payload\n");
		rc = -1;
		goto exit;
	}

	/* AF params contains 1 byte of payload - reduce buf_len accordingly */
	buf_len = str_len + 1 + sizeof(awdl_oob_af_params_t) - 1 + af_params->payload_len;

	rc = wlu_set(wl, WLC_SET_VAR, buf, buf_len);

exit:
	return rc;
}

static int
wl_awdl_oob_af_async(void *wl, cmd_t *cmd, char **argv)
{
	awdl_oob_af_params_async_t *af_params;
	int rc = BCME_OK;
	int err;
	int count;
	const char	*str;
	int	str_len;
	int	buf_len;

	count = ARGCNT(argv);

	UNUSED_PARAMETER(cmd);

	/* GET operation not supported */
	if (count < 2) {
		fprintf(stderr, "GET operation is not supported\n");
		rc = -1;
		goto exit;
	}

	str = "awdl_oob_af_async";
	str_len = strlen(str);
	strncpy(buf, str, str_len);
	buf[ str_len ] = '\0';

	af_params = (awdl_oob_af_params_async_t *) (buf + str_len + 1);

	rc = wl_awdl_oob_af_cmn(count, argv, (awdl_oob_af_params_t *)&af_params->bssid);
	if (rc != BCME_OK)
		goto exit;

	af_params->flags = htod32(strtoul(*(argv + 5), NULL, 0));

	if (count < 10) {
		fprintf(stderr, "Tx time is expected\n");
		rc = -1;
		goto exit;
	}
	af_params->tx_time = htod32(strtoul(*(argv + 9), NULL, 0));

	if (count < 11) {
		fprintf(stderr, "Pkt tag is expected\n");
		rc = -1;
		goto exit;
	}
	af_params->tag = htod32(strtoul(*(argv + 10), NULL, 0));

	if (count < 12) {
		fprintf(stderr, "Payload is expected\n");
		rc = -1;
		goto exit;
	}
	/* Variable length payload */
	af_params->payload_len = htod16(strlen(argv[11])) / 2;

	if ((err = get_ie_data ((uchar *)argv[11],
		&af_params->payload[0],
		af_params->payload_len))) {
		fprintf(stderr, "Error parsing payload\n");
		rc = -1;
		goto exit;
	}

	/* AF params contains 1 byte of payload - reduce buf_len accordingly */
	buf_len = str_len + 1 + sizeof(awdl_oob_af_params_async_t) - 1 + af_params->payload_len;

	rc = wlu_set(wl, WLC_SET_VAR, buf, buf_len);

exit:
	return rc;
}


static int
wl_awdl_oob_af_auto(void *wl, cmd_t *cmd, char **argv)
{
	awdl_oob_af_params_auto_t *af_params;
	int rc = BCME_OK;
	int err;
	int count;
	const char	*str;
	int	str_len;
	int	buf_len;

	count = ARGCNT(argv);

	UNUSED_PARAMETER(cmd);

	/* GET operation not supported */
	if (count < 2) {
		fprintf(stderr, "GET operation is not supported\n");
		rc = -1;
		goto exit;
	}

	str = "awdl_oob_af_auto";
	str_len = strlen(str);
	strncpy(buf, str, str_len);
	buf[ str_len ] = '\0';

	af_params = (awdl_oob_af_params_auto_t *) (buf + str_len + 1);

	rc = wl_awdl_oob_af_cmn(count, argv, (awdl_oob_af_params_t *)&af_params->bssid);
	if (rc != BCME_OK)
		goto exit;

	if (count < 10) {
		fprintf(stderr, "Tx channel map is expected\n");
		rc = -1;
		goto exit;
	}
	af_params->tx_chan_map = htod32(strtoul(*(argv + 9), NULL, 0));

	if (count < 11) {
		fprintf(stderr, "Tx aws offset is expected\n");
		rc = -1;
		goto exit;
	}
	af_params->tx_aws_offset = htod32(strtoul(*(argv + 10), NULL, 0));

	if (count < 12) {
		fprintf(stderr, "Payload is expected\n");
		rc = -1;
		goto exit;
	}
	/* Variable length payload */
	af_params->payload_len = htod16(strlen(argv[11])) / 2;

	if ((err = get_ie_data ((uchar *)argv[11],
		&af_params->payload[0],
		af_params->payload_len))) {
		fprintf(stderr, "Error parsing payload\n");
		rc = -1;
		goto exit;
	}

	/* AF params contains 1 byte of payload - reduce buf_len accordingly */
	buf_len = str_len + 1 + sizeof(awdl_oob_af_params_auto_t) - 1 + af_params->payload_len;

	rc = wlu_set(wl, WLC_SET_VAR, buf, buf_len);

exit:
	return rc;
}

static int
wl_awdl_uct_test(void *wl, cmd_t *cmd, char **argv)
{
	int count;
	struct {
		uint interval;
		uint offset;
	} param;

	count = ARGCNT(argv);

	/* GET operation not supported */
	if (count < 3)
		return -1;

	param.interval = strtoul(argv[1], NULL, 0);
	param.offset = strtoul(argv[2], NULL, 0);

	return wlu_iovar_set(wl, cmd->name, &param, sizeof(param));
}

#ifdef WLAWDL_LATENCY_SUPPORT
static int
wl_awdl_tx_latency_clr(void *wl, cmd_t *cmd, char **argv)
{
	int err;

	UNUSED_PARAMETER(argv);

	err = wlu_var_setbuf(wl, cmd->name, NULL, 0);
	return (err);
}
#endif /* WLAWDL_LATENCY_SUPPORT */
