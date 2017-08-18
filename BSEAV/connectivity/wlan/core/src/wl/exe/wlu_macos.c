/*
 * Mac OS port of wl command line utility
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
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#include <signal.h>
#include <time.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/kern_event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <Kernel/IOKit/apple80211/apple80211_ioctl.h>
#include <Kernel/IOKit/apple80211/apple80211_var.h>
#include <typedefs.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include <bcmwifi_channels.h>
#include "wlu.h"
#include "miniopt.h"
#ifdef MACOS_DHD_DONGLE
#include "dhdioctl_usr.h"
#endif /* MACOS_DHD_DONGLE */

/* Backward compatability for 10.9 header misspell
 * 10.9 apple80211_var.h has the misspelled name, and also APPLE80211_M_MAX==52
 * Include a rename if we are using the 10.9 header.
 */
#if (APPLE80211_M_MAX == 52)
#define apple80211_channel_switch_announcement_t apple80211_channel_switch_anoucement_t
#endif

typedef struct wl_info {
	int socket_fd;          /* socket for driver ioctls */
	char ifname[IFNAMSIZ];  /* interface name for ioctls, provided or discovered */
} wl_info_t;

static cmd_t *wl_find_cmd(char* name);
static int wl_apple80211_ioctl_basic(int if_socket, const char ifname[IFNAMSIZ], int type, int val,
                            void *buf, uint *len, bool set);
static int wl_apple80211_ioctl(void *wl, int type, int val, void *buf, uint *len, bool set);
static int wl_parse_apple_channel_list(char* list_str,
                                       struct apple80211_channel *channels, uint32 *channel_count);
static int wl_parse_apple_channel(char *channel_str, struct apple80211_channel *channel);

static cmd_func_t apple80211_rsn_ie;
static cmd_func_t apple80211_ap_ies;
static cmd_func_t apple80211_associate;
static cmd_func_t apple80211_ssid;
static cmd_func_t apple80211_bssid;
static cmd_func_t apple80211_card_capabilities;
static cmd_func_t apple80211_scan_request;
static cmd_func_t apple80211_events;


/* Support for the apple80211_events command is only for non-OLYMPIC_RWL */

/* for interrupt signal */
static int quit = 0;

/* Kern event reader of messages of SYSPROTO_EVENT */
static int apple80211_event_create_socket();
static int apple80211_event_get(int event_socket, struct kern_event_msg* msg, uint msg_len,
                                uint *data_len);
static int apple80211_event_process(int if_socket, int event_socket);

/* functions for apple80211 specific ioctls */
static int apple80211_bssid_get(int if_socket, const char ifname[IFNAMSIZ],
                                struct ether_addr *ea);
static int apple80211_ssid_get(int if_socket, const char ifname[IFNAMSIZ],
                               struct apple80211_ssid *ssid);

/*
 * Name table to associate strings with numeric IDs
 */
struct name_entry {
	int id;
	const char *name;
};

/*
 * Name lookup utility.
 * Given a name table and an ID, will return the name matching the ID, or a string with the raw ID.
 */
static const char* lookup_name(const struct name_entry* tbl, int id);

/* names for the apple80211 events */
static const struct name_entry apple80211_event_names[];

/* overlay structure to give some form to the kern_event_msg.event_data area
 * for KEV_IEEE80211_CLASS/KEV_APPLE80211_EVENT_SUBCLASS events
 */
struct apple80211_event_data {
	char ifname[IFNAMSIZ];
	union {
		struct apple80211_link_quality lq;
		struct apple80211_ibss_peer ibss_peer;
		struct apple80211_link_status link;
		struct apple80211_channel_event_data channel_event;
		struct apple80211_act_frm_tx_complete_event_data action_frame;
		struct ether_addr ea;
		apple80211_channel_switch_announcement_t csa;
	} payload;
};


#define MSG_BUF_SZ 256 // Kernel event message read buffer
#define TIME_STR_SZ 20 // string buffer size for timestamp formatting


static cmd_t apple80211_cmds[] = {
	{"associate", apple80211_associate, -1, APPLE80211_IOC_ASSOCIATE,
	"Start an association\n"
	"\tUsage: wl associate [Options] SSID\n"
	"\tOptions:\n"
	"\t--bss_type=BT\t\t[bss | infra | ibss | adhoc] target bss type\n"
	"\t-b MAC, --bssid=MAC\tparticular BSSID MAC address to join, xx:xx:xx:xx:xx:xx\n"
	"\t-c L, --channels=L\tcomma or space separated list of target channels"
	},
	{"apple_ssid", apple80211_ssid, APPLE80211_IOC_SSID, APPLE80211_IOC_SSID,
	"Get/Set the SSID"
	},
	{"apple_bssid", apple80211_bssid, APPLE80211_IOC_BSSID, APPLE80211_IOC_BSSID,
	"Get/Set the BSSID"
	},
	{"card_cap", apple80211_card_capabilities, APPLE80211_IOC_CARD_CAPABILITIES, -1,
	"Get the Apple CARD_CAPABILITIES"
	},
	{"rsn_ie", apple80211_rsn_ie, APPLE80211_IOC_RSN_IE, APPLE80211_IOC_RSN_IE,
	"Get/Set the RSN_IE\n"
	"\tOn set, provide a hextring or \"clear\" to clear the saved RSN_IE info"
	},
	{"ap_ies", apple80211_ap_ies, APPLE80211_IOC_AP_IE_LIST, -1,
	"Get the AP_IES, IE information from the currently associated AP"
	},
	{"apple_scan", apple80211_scan_request, -1, APPLE80211_IOC_SCAN_REQ,
	"Start an APPLE80211_IOC_SCAN_REQ scan\n"
	"\tDefault an active scan across all channels for any SSID.\n"
	"\tOptional arg: SSID, the SSID to scan.\n"
	"\tOptions:\n"
	"\t-s S, --ssid=S\t\tSSID to scan\n"
	"\t-t ST, --scan_type=ST\t[active | passive | fast] scan type\n"
	"\t--bss_type=BT\t\t[bss | infra | ibss | adhoc | any] bss type to scan\n"
	"\t-b MAC, --bssid=MAC\tparticular BSSID MAC address to scan, xx:xx:xx:xx:xx:xx\n"
	"\t-d N, --dwell=N\tdwell time per channel (ms)\n"
	"\t-r N, --rest=N\ttime between scanning each channel (ms)\n"
	"\t-p N, --phy_mode=N\t\tphy_modes for scan, comma or space separated list \n"
	 "\t                 \t\tof [11a 11b 11g 11n gturbo aturbo]\n"
	"\t-c L, --channels=L\tcomma or space separated list of channels to scan"
	},
	{"apple80211_events", apple80211_events, -1, -1,
	"Start monitoring APPLE80211 events. Shell interrupt (^C) to exit monitoring."
	},
	{ NULL, NULL, 0, 0, NULL }
};

static void
syserr(char *s)
{
	perror(s);
	exit(errno);
}


static int
wl_apple80211_ioctl(void *wl, int type, int val, void *buf, uint *len, bool set)
{
	wl_info_t *wli = (wl_info_t*)wl;
	int err;

	err = wl_apple80211_ioctl_basic(wli->socket_fd, wli->ifname, type, val, buf, len, set);

	return err;
}

static int
wl_apple80211_ioctl_basic(int if_socket, const char ifname[IFNAMSIZ], int type, int val,
                    void *buf, uint *len, bool set)
{
	int ret = 0;
	struct apple80211req req;

	memset(&req, 0, sizeof(req));

	memcpy(req.req_if_name, ifname, IFNAMSIZ);

	req.req_type = type;
	req.req_val = val;
	req.req_len = *len;
	req.req_data = CAST_USER_ADDR_T(buf);

	ret = ioctl(if_socket, set ? SIOCSA80211 : SIOCGA80211, (char*)&req);

	if (ret == 0) {
		*len = req.req_len;
	} else {
		*len = 0;
		ret = errno;
	}

	return ret;
}

static int
wl_ioctl(void *wl, int cmd, void *buf, int len, bool set)
{
	int ret = 0;

	ret = wl_apple80211_ioctl(wl, APPLE80211_IOC_CARD_SPECIFIC, cmd,
	                          buf, (uint*)&len, set);

#ifdef MACOS_DHD_DONGLE
	if (ret != 0)
		ret = dhdioctl_usr_dongle(wl, cmd, buf, len, set, TRUE);
#endif /* MACOS_DHD_DONGLE */

	if (ret != 0) {
		if (cmd != WLC_GET_MAGIC)
			ret = BCME_IOCTL_ERROR;
	}

	return ret;
}


int
wl_get(void *wl, int cmd, void *buf, int len)
{
	int error = 0;
	error = wl_ioctl(wl, cmd, buf, len, FALSE);

	if (error == BCME_SERIAL_PORT_ERR)
		return BCME_SERIAL_PORT_ERR;
	else if (error == BCME_NODEVICE)
		return BCME_NODEVICE;
	else if (error != 0)
		return BCME_IOCTL_ERROR;
	return 0;
}

int
wl_set(void *wl, int cmd, void *buf, int len)
{
	int error = 0;
	error = wl_ioctl(wl, cmd, buf, len, TRUE);
	if (error == BCME_SERIAL_PORT_ERR)
		return BCME_SERIAL_PORT_ERR;
	else if (error == BCME_NODEVICE)
		return BCME_NODEVICE;
	else if (error != 0)
		return BCME_IOCTL_ERROR;
	return 0;
}

int
wl_find(wl_info_t *wl)
{
	struct ifaddrs *ifastart;
	struct ifaddrs *ifalist;
	int err;

	err = getifaddrs(&ifastart);
	if (err)
		syserr("getifaddrs failed");

	for (ifalist = ifastart; ifalist; ifalist = ifalist->ifa_next) {
		int name_len = strlen(ifalist->ifa_name);

		/* Make sure the ifaddr.ifa_name fits in the apple80211req.req_if_name field.
		 * If not, skip this interface.
		 */
		if (name_len + 1 > IFNAMSIZ) {
			continue;
		}

		memset(wl->ifname, 0, IFNAMSIZ);

		/* copy the interface name and NUL termination */
		memcpy(wl->ifname, ifalist->ifa_name, name_len + 1);

		if (!wl_check(wl))
			break;
	}

	freeifaddrs(ifastart);

	/* clear the ifname from the last attempt if we exhausted the ifalist */
	if (!ifalist) {
		memset(wl->ifname, 0, IFNAMSIZ);
		err = BCME_NOTFOUND;
	}

	return err;
}


int
main(int argc, char **argv)
{
	wl_info_t wl;
	cmd_t *cmd = NULL;
	int err = 0;
	char *ifname = NULL;
	int help = 0;
	int status = CMD_WL;

	wlu_av0 = argv[0];

	wlu_init();
	memset(&wl, 0, sizeof(wl));

	for (++argv; *argv;) {
		/* command option */
		if ((status = wl_option(&argv, &ifname, &help)) == CMD_OPT) {

			if (ifname) {
				int name_len = strlen(ifname);

				/* Make sure the ifname fits in
				 * the apple80211req.req_if_name field.
				 */
				if (name_len + 1 > IFNAMSIZ) {
					fprintf(stderr, "interface name \"%s\" too long\n",
					        ifname);
					return -1;
				}

				memset(wl.ifname, 0, IFNAMSIZ);

				/* copy the interface name and NUL termination */
				memcpy(wl.ifname, ifname, name_len + 1);
			}

			if (help)
				break;

			continue;
		}
		else if (status == CMD_ERR) {
			fprintf(stderr, "command line error\n");
			return -1;
		} else
			break;
	}

	if (help || *argv == NULL) {
		if (*argv == NULL) {
			wl_usage(stdout, NULL);
		} else {
			cmd = wl_find_cmd(*argv);
			if (cmd)
				wl_cmd_usage(stdout, cmd);
			else
				printf("%s: Unrecognized command \"%s\", type -h for help\n",
				       wlu_av0, *argv);
		}

		return 0;
	}

	/* open socket to kernel */
	if ((wl.socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	/* validate provided interface name */
	if (wl.ifname[0] != '\0' && wl_check(&wl)) {
		fprintf(stderr, "interface \"%s\" is not a valid wl driver\n", wl.ifname);
		err = 1;
		goto exit;
	}
	/* find a default interface */
	if (wl.ifname[0] == '\0') {
		if (wl_find(&wl) != 0) {
			fprintf(stderr, "wl driver adapter not found\n");
			err = 1;
			goto exit;
		}
	}

	/* search for command */
	cmd = wl_find_cmd(*argv);

	/* defaults to using the set_var and get_var commands */
	if (!cmd)
		cmd = &wl_varcmd;

	/* do command */
	err = (*cmd->func)(&wl, cmd, argv);

	if (err == BCME_USAGE_ERROR)
		wl_cmd_usage(stderr, cmd);
	else if (err == BCME_IOCTL_ERROR)
		wl_printlasterror(&wl);

exit:
	close(wl.socket_fd);

	return err;
}

/* Search the  wl_cmds table for a matching command name.
 * Return the matching command or NULL if no match found.
 */
static cmd_t*
wl_find_cmd(char* name)
{
	cmd_t *cmd = NULL;

	/* search the apple80211_cmds for a matching name */
	for (cmd = apple80211_cmds; cmd->name && strcmp(cmd->name, name); cmd++);
	return (cmd->name != NULL) ? cmd : wlu_find_cmd(name);
}

static int
wl_hextobytes(char *hexstr, void *buf, uint bufsize, char **endptr, uint *outsize)
{
	uint written = 0;
	char hexbyte[3];
	uint i;
	uint len;
	uint8 *p = (uint8*)buf;
	char *local_endptr;
	int err = 0;

	if (hexstr == NULL)
		goto exit;

	len = strlen(hexstr) / 2;

	for (i = 0; i < len; i++) {
		if (i >= bufsize) {
			err = BCME_BUFTOOSHORT;
			goto exit;
		}

		hexbyte[0] = hexstr[0];
		hexbyte[1] = hexstr[1];
		hexbyte[2] = '\0';

		*p = (uint8)strtoul(hexbyte, &local_endptr, 16);
		if (local_endptr[0] != '\0') {
			err = BCME_USAGE_ERROR;
			break;
		}

		p++;
		written++;
		hexstr += 2;
	}

exit:
	if (endptr)
		*endptr = hexstr;

	*outsize = written;

	return err;
}

static int
apple80211_associate(void *wl, cmd_t *cmd, char **argv)
{
	const char* fn_name = "apple80211_associate";
	miniopt_t mo;
	int opt_err, err;
	struct apple80211_assoc_data ad;
	uint iolen = sizeof(struct apple80211_assoc_data);

	/* assoc defaults */
	memset(&ad, 0, sizeof(ad));
	ad.version = APPLE80211_VERSION;
	ad.ad_mode = APPLE80211_AP_MODE_INFRA;
	ad.ad_auth_lower = APPLE80211_AUTHTYPE_OPEN;
	ad.ad_auth_upper = APPLE80211_AUTHTYPE_NONE;
	ad.ad_key.version = APPLE80211_VERSION;

	/* skip the command name */
	argv++;

	miniopt_init(&mo, fn_name, NULL, FALSE);

	/* process the first option */
	opt_err = miniopt(&mo, argv);

	miniopt_init(&mo, fn_name, NULL, FALSE);
	while ((opt_err = miniopt(&mo, argv)) != -1) {
		if (opt_err == 1)
			return BCME_USAGE_ERROR;
		argv += mo.consumed;

		if (mo.opt == 's' || !strcmp(mo.key, "ssid") || mo.positional) {
			if (strlen(mo.valstr) > 32) {
				fprintf(stderr,
					"SSID arg \"%s\" must be 32 chars or less\n",
					mo.valstr);
				return BCME_USAGE_ERROR;
			}
			ad.ad_ssid_len = strlen(mo.valstr);
			memcpy(ad.ad_ssid, mo.valstr, ad.ad_ssid_len);
		} else if (mo.opt == 'b' || !strcmp(mo.key, "bssid")) {
			err = wl_ether_atoe(mo.valstr, &ad.ad_bssid);
			if (err) {
				fprintf(stderr, "%s: error parsing \"%s\" as an ether addr\n",
				        __FUNCTION__, *argv);
				return BCME_USAGE_ERROR;
			}
		} else if (!strcmp(mo.key, "bss_type")) {
			if (!strcmp(mo.valstr, "bss") || !strcmp(mo.valstr, "infra"))
				ad.ad_mode = APPLE80211_AP_MODE_INFRA;
			else if (!strcmp(mo.valstr, "ibss") || !strcmp(mo.valstr, "adhoc"))
				ad.ad_mode = APPLE80211_AP_MODE_IBSS;
			else {
				fprintf(stderr,
					"%s: could not parse \"%s\" as a bss_type\n",
					fn_name, mo.valstr);
				return BCME_USAGE_ERROR;
			}
		} else if (mo.opt == 'c' || !strcmp(mo.key, "channels")) {
			uint i;
			static struct apple80211_channel chan_list[APPLE80211_MAX_CHANNELS];
			uint chan_list_len = APPLE80211_MAX_CHANNELS;

			err = wl_parse_apple_channel_list(mo.valstr, chan_list, &chan_list_len);
			if (err)
				return BCME_USAGE_ERROR;

#ifndef RDR_5946960
			ad.version = 2;
			ad.ad_rsn_ie_len = chan_list_len;

			for (i = 0; i < ad.ad_rsn_ie_len; i++)
				ad.ad_rsn_ie[i] = chan_list[i].channel;
#else
			ad.num_channels = chan_list_len;
			for (i = 0; i < chan_list_len; i++)
				ad.channels[i] = chan_list[i].channel;
#endif /* RDR_5946960 */
		}
	}

	if (*argv) {
		fprintf(stderr, "%s: unexpected args starting with \"%s\"\n", __FUNCTION__, *argv);
		return BCME_USAGE_ERROR;
	}

	err = wl_apple80211_ioctl(wl, cmd->set, 0, &ad, &iolen, TRUE);
	if (err) {
		fprintf(stderr, "%s: error %d from ioctl set of APPLE80211_IOC_ASSOCIATE\n",
			__FUNCTION__, err);
		return BCME_IOCTL_ERROR;
	}

	return 0;
}

static int
apple80211_ssid(void *wl, cmd_t *cmd, char **argv)
{
	uint8 ssid[APPLE80211_MAX_SSID_LEN];
	char ssidbuf[SSID_FMT_BUF_LEN];
	uint iolen;
	int err = 0;

	/* skip the command name */
	argv++;

	if (!*argv) {
		iolen = APPLE80211_MAX_SSID_LEN;
		err = wl_apple80211_ioctl(wl, cmd->get, 0, ssid, &iolen, FALSE);
		if (err == ENXIO) {
			printf("<Not Asssociated>\n");
			err = 0;
		} else if (err) {
			fprintf(stderr, "%s: error %d trying to get the SSID\n",
			        __FUNCTION__, err);
			err = 0;
		} else {
			if (iolen > APPLE80211_MAX_SSID_LEN) {
				printf("%s: iolen claims %d bytes returned but io buf was %d bytes",
				       __FUNCTION__, iolen, APPLE80211_MAX_SSID_LEN);
				iolen = APPLE80211_MAX_SSID_LEN;
			}
			wl_format_ssid(ssidbuf, ssid, iolen);
			printf("\"%s\"\n", ssidbuf);
		}
	} else {
		iolen = strlen(*argv);

		/* verify that SSID was specified and is a valid length */
		if (iolen > APPLE80211_MAX_SSID_LEN)
			return BCME_BADARG;

		memcpy(ssid, *argv, iolen);

		err = wl_apple80211_ioctl(wl, cmd->set, 0, ssid, &iolen, TRUE);
		if (err)
			fprintf(stderr, "%s: error %d trying to set the SSID\n",
			        __FUNCTION__, err);
		err = 0;
	}

	return err;
}

static int
apple80211_bssid(void *wl, cmd_t *cmd, char **argv)
{
	struct ether_addr ea;
	uint iolen;
	int err = 0;

	/* skip the command name */
	argv++;

	if (!*argv) {
		iolen = ETHER_ADDR_LEN;
		err = wl_apple80211_ioctl(wl, cmd->get, 0, &ea, &iolen, FALSE);
		if (err == ENXIO) {
			printf("<Not Asssociated>\n");
			err = 0;
		} else if (err) {
			fprintf(stderr, "%s: error %d trying to get the BSSID\n",
			        __FUNCTION__, err);
			err = 0;
		} else {
			if (iolen > ETHER_ADDR_LEN) {
				printf("%s: iolen claims %d bytes returned but io buf was %d bytes",
				       __FUNCTION__, iolen, ETHER_ADDR_LEN);
				iolen = ETHER_ADDR_LEN;
			}
			printf("%s\n", wl_ether_etoa(&ea));
		}
	} else {
		iolen = ETHER_ADDR_LEN;

		if (!wl_ether_atoe(*argv, &ea)) {
			fprintf(stderr, "%s: error parsing \"%s\" as an ether addr\n",
			        __FUNCTION__, *argv);
			return BCME_USAGE_ERROR;
		}

		err = wl_apple80211_ioctl(wl, cmd->set, 0, &ea, &iolen, TRUE);
		if (err)
			fprintf(stderr, "%s: error %d trying to set the BSSID\n",
			        __FUNCTION__, err);
		err = 0;
	}

	return err;
}

static int
apple80211_card_capabilities(void *wl, cmd_t *cmd, char **argv)
{
	struct apple80211_capability_data capabilities;
	uint iolen;
	int i;
	int got_one = FALSE;
	int err = 0;

	/* skip the command name */
	argv++;

	/* error if there are any args. This is only a 'get' command */
	if (*argv) {
		fprintf(stderr, "%s: command %s is a get only, expected no args\n",
		        __FUNCTION__, cmd->name);
		return BCME_USAGE_ERROR;
	}

	memset(&capabilities, 0, sizeof(capabilities));
	iolen = sizeof(capabilities.capabilities);
	err = wl_apple80211_ioctl(wl, cmd->get, 0, &capabilities.capabilities, &iolen, FALSE);
	if (err) {
		fprintf(stderr, "%s: error %d trying to get the CARD_CAPABILITIES\n",
		        __FUNCTION__, err);
		return err;
	}

	printf("Capabilities: ");
	for (i = 0; i < APPLE80211_CAP_MAX; i++) {
		if (isclr(capabilities.capabilities, i))
			continue;
		if (got_one)
			printf(", %d", i);
		else
			printf("%d", i);
		got_one = TRUE;
	}
	printf("\n");

	return err;
}

static int
apple80211_rsn_ie(void *wl, cmd_t *cmd, char **argv)
{
	uint8 ie_data[APPLE80211_MAX_RSN_IE_LEN];
	uint iolen = APPLE80211_MAX_RSN_IE_LEN;
	int err = 0;
	bcm_tlv_t* ie;
	int dump_len;
	char* endptr;

	/* skip the command name */
	argv++;

	if (!*argv) {
		err = wl_apple80211_ioctl(wl, cmd->get, 0, ie_data, &iolen, FALSE);
		if (err == ENXIO) {
			printf("No RSN_IE\n");
		} else if (err) {
			fprintf(stderr, "%s: error %d trying to get the RSN_IE data\n",
			        __FUNCTION__, err);
			return err;
		}

		if (iolen == 0) {
			printf("RSN_IE: IE data empty\n");
		} else if (iolen < TLV_HDR_LEN) {
			printf("RSN_IE: IE too short, only %d bytes\n", iolen);
		} else {
			ie = (bcm_tlv_t*)ie_data;
			dump_len = ie->len;

			if (iolen != (TLV_HDR_LEN + ie->len)) {
				dump_len = MIN(iolen - TLV_HDR_LEN, ie->len);
				printf("RSN_IE: IO buffer length val %d does not match\n"
				       "\tIE header len %d plus IE len val %d\n"
				       "\tDumping %d bytes of IE data\n",
				       iolen, TLV_HDR_LEN, ie->len, dump_len);
			}
			printf("RSN_IE: ID 0x%2X Len %d bytes\n", ie->id, ie->len);
			printf("RSN_IE: Data:\n");
			wl_hexdump(ie->data, dump_len);
		}
	} else {
		if (!strcmp(*argv, "clear")) {
			iolen = 0;
		} else {
			err = wl_hextobytes(*argv, ie_data, APPLE80211_MAX_RSN_IE_LEN,
			                    &endptr, &iolen);

			if (err == BCME_BUFTOOSHORT) {
				fprintf(stderr,
				        "%s: buffer overflow writing \"%s\" "
				        "(output buffer bytes: %u)\n",
				        __FUNCTION__, *argv, APPLE80211_MAX_RSN_IE_LEN);
				err = BCME_USAGE_ERROR;
				goto exit;
			}
			if (endptr[0] != '\0') {
				fprintf(stderr,
				        "%s: could not parse all of \"%s\" as a hex string, "
				        "stopped at \"%s\"\n",
				        __FUNCTION__, *argv, endptr);
				err = BCME_USAGE_ERROR;
				goto exit;
			}
			if (err) {
				fprintf(stderr, "%s: error parsing hex string \"%s\"\n",
				        __FUNCTION__, *argv);
				err = BCME_USAGE_ERROR;
				goto exit;
			}
		}

		err = wl_apple80211_ioctl(wl, cmd->set, 0, &ie_data, &iolen, TRUE);
		if (err) {
			fprintf(stderr, "%s: error %d trying to set the RSN_IE data\n",
			        __FUNCTION__, err);
			return err;
		}
	}

exit:
	return err;
}

static int
apple80211_ap_ies(void *wl, cmd_t *cmd, char **argv)
{
#define IES_SIZE 1024
	uint iolen = IES_SIZE;
	static uint8 ies[IES_SIZE];
	bcm_tlv_t* ie;
	uint ies_len;
	int err;

	/* skip the command name */
	argv++;

	/* error if there are any args. This is only a 'get' command */
	if (*argv) {
		fprintf(stderr, "%s: command %s is a get only, expected no args\n",
		        __FUNCTION__, cmd->name);
		return BCME_USAGE_ERROR;
	}

	memset(ies, 0, IES_SIZE);

	err = wl_apple80211_ioctl(wl, cmd->get, 0, &ies, &iolen, FALSE);

	if (err == ENXIO) {
		printf("No AP_IE_LIST information available\n");
		return 0;
	} else if (err) {
		fprintf(stderr, "%s: error %d trying to get the AP_IE_LIST data\n",
		        __FUNCTION__, err);
		return err;
	}

	if (iolen == 0) {
		printf("AP_IES: IE data empty\n");
		return 0;
	} else if (iolen > IES_SIZE) {
		iolen = IES_SIZE;
	}

	/* print each of the IEs */
	ie = (bcm_tlv_t*)ies;
	ies_len = iolen;

	do {
		wl_dump_raw_ie(ie, ies_len);

		/* validate current elt */
		if (!bcm_valid_tlv(ie, ies_len))
			break;

		/* advance to next elt */
		ies_len -= (2 + ie->len);
		ie = (bcm_tlv_t*)(ie->data + ie->len);
	} while (1);

	return 0;
}

static int
apple80211_scan_request(void *wl, cmd_t *cmd, char **argv)
{
	const char* fn_name = "apple80211_scan_request";
	miniopt_t mo;
	int opt_err, err;
	struct apple80211_scan_data sd;
	uint iolen = sizeof(struct apple80211_scan_data);

	/* scan defaults */
	memset(&sd, 0, sizeof(sd));
	sd.version = APPLE80211_VERSION;
	sd.bss_type = APPLE80211_AP_MODE_ANY;
	sd.scan_type = APPLE80211_SCAN_TYPE_ACTIVE;
	sd.phy_mode = APPLE80211_MODE_AUTO;

	/* skip the command name */
	argv++;

	miniopt_init(&mo, fn_name, NULL, FALSE);

	/* process the first option */
	opt_err = miniopt(&mo, argv);

	miniopt_init(&mo, fn_name, NULL, FALSE);
	while ((opt_err = miniopt(&mo, argv)) != -1) {
		if (opt_err == 1)
			return BCME_USAGE_ERROR;
		argv += mo.consumed;

		if (mo.opt == 's' || !strcmp(mo.key, "ssid") || mo.positional) {
			if (strlen(mo.valstr) > 32) {
				fprintf(stderr,
					"SSID arg \"%s\" must be 32 chars or less\n",
					mo.valstr);
				return BCME_USAGE_ERROR;
			}
			sd.ssid_len = strlen(mo.valstr);
			memcpy(sd.ssid, mo.valstr, sd.ssid_len);
		} else if (mo.opt == 't' || !strcmp(mo.key, "scan_type")) {
			if (!strcmp(mo.valstr, "active"))
				sd.scan_type = APPLE80211_SCAN_TYPE_ACTIVE;
			else if (!strcmp(mo.valstr, "passive"))
				sd.scan_type = APPLE80211_SCAN_TYPE_PASSIVE;
			else if (!strcmp(mo.valstr, "fast"))
				sd.scan_type = APPLE80211_SCAN_TYPE_FAST;
			else if (!strcmp(mo.valstr, "background"))
				sd.scan_type = APPLE80211_SCAN_TYPE_BACKGROUND;
			else {
				fprintf(stderr,
					"%s: could not parse \"%s\" as a scan_type\n",
					fn_name, mo.valstr);
				return BCME_USAGE_ERROR;
			}
		} else if (!strcmp(mo.key, "bss_type")) {
			if (!strcmp(mo.valstr, "bss") || !strcmp(mo.valstr, "infra"))
				sd.bss_type = APPLE80211_AP_MODE_INFRA;
			else if (!strcmp(mo.valstr, "ibss") || !strcmp(mo.valstr, "adhoc"))
				sd.bss_type = APPLE80211_AP_MODE_IBSS;
			else if (!strcmp(mo.valstr, "any"))
				sd.bss_type = APPLE80211_AP_MODE_ANY;
			else {
				fprintf(stderr,
					"%s: could not parse \"%s\" as a bss_type\n",
					fn_name, mo.valstr);
				return BCME_USAGE_ERROR;
			}
		} else if (mo.opt == 'p' || !strcmp(mo.key, "phy_mode")) {
			if (!mo.good_int)
				return BCME_USAGE_ERROR;
			sd.phy_mode = (uint32)mo.val;
		} else if (mo.opt == 'd' || !strcmp(mo.key, "dwell")) {
			if (!mo.good_int)
				return BCME_USAGE_ERROR;
			sd.dwell_time = (uint32)mo.val;
		} else if (mo.opt == 'r' || !strcmp(mo.key, "rest")) {
			if (!mo.good_int)
				return BCME_USAGE_ERROR;
			sd.rest_time = (uint32)mo.val;
		} else if (mo.opt == 'c' || !strcmp(mo.key, "channels")) {
			err = wl_parse_apple_channel_list(mo.valstr, sd.channels, &sd.num_channels);
			if (err)
				return BCME_USAGE_ERROR;
		}
	}

	if (*argv) {
		fprintf(stderr, "%s: unexpected args starting with \"%s\"\n", __FUNCTION__, *argv);
		return BCME_USAGE_ERROR;
	}

	err = wl_apple80211_ioctl(wl, cmd->set, 0, &sd, &iolen, TRUE);
	if (err) {
		fprintf(stderr, "%s: error %d from ioctl set of APPLE80211_IOC_SCAN_REQ\n",
			__FUNCTION__, err);
		return BCME_IOCTL_ERROR;
	}

	return 0;
}

static int
wl_parse_apple_channel_list(char* list_str,
                            struct apple80211_channel *channels, uint32 *channel_count)
{
	int count = 0;
	char *channel_str;
	int err;

	*channel_count = 0;

	if (list_str == NULL)
		return -1;

	while ((channel_str = strsep(&list_str, " \t,")) != NULL) {
		if (*channel_str == '\0')
			continue;	/* consecutive delimiters */
		err = wl_parse_apple_channel(channel_str, &channels[count]);
		if (err)
			return err;

		count++;
		if (count >= APPLE80211_MAX_CHANNELS)
			break;
	}

	if (channel_str != NULL) {
		fprintf(stderr,
			"too many channels in channel list, "
		        "maximum %d in scan data channel structure\n",
		        APPLE80211_MAX_CHANNELS);
		return -1;
	}

	*channel_count = (uint32)count;

	return 0;
}

static int
wl_parse_apple_channel(char *channel_str, struct apple80211_channel *channel)
{
	char *endp, *p;
	char c;
	uint channel_num, band, bw, ext_sb;

	channel_num = strtoul(channel_str, &endp, 10);

	/* check for no digits parsed */
	if (endp == channel_str) {
		fprintf(stderr,
			"could not parse channel number at beginning of \"%s\"\n",
			channel_str);
		return -1;
	}

	band = ((channel_num <= CH_MAX_2G_CHANNEL) ?
	        APPLE80211_C_FLAG_2GHZ : APPLE80211_C_FLAG_5GHZ);
	bw = APPLE80211_C_FLAG_20MHZ;
	ext_sb = 0;

	p = endp;

	c = tolower(p[0]);
	if (c == '\0')
		goto done;

	/* parse the optional ['A' | 'B'] band spec */
	if (c == 'a' || c == 'b') {
		band = (c == 'b') ? APPLE80211_C_FLAG_2GHZ : APPLE80211_C_FLAG_5GHZ;
		p++;
		c = tolower(p[0]);
		if (c == '\0')
			goto done;
	}

	/* parse bandwidth 'N' (10MHz) or 40MHz ctl sideband ['L' | 'U'] */
	if (c == 'n') {
		bw = APPLE80211_C_FLAG_10MHZ;
	} else if (c == 'l') {
		bw = APPLE80211_C_FLAG_40MHZ;
		ext_sb = APPLE80211_C_FLAG_EXT_ABV;
	} else if (c == 'u') {
		bw = APPLE80211_C_FLAG_40MHZ;
	} else {
		fprintf(stderr,
			"unknown channel modifier '%c' in channel string \"%s\"\n",
			c, channel_str);
		return -1;
	}

done:
	channel->version = APPLE80211_VERSION;
	channel->channel = (uint32)channel_num;
	channel->flags = (band | bw | ext_sb);

	return 0;
}

/* Support for the apple80211_events command is only for non-OLYMPIC_RWL */

static void
sig_int(int signal)
{
	quit = 1;
	return;
}

/* command handler to monitor apple80211 system events */
int
apple80211_events(void *wl, cmd_t *cmd, char **argv)
{
	wl_info_t *wli = (wl_info_t*)wl;
	int err;
	int event_socket = -1;
	struct sigaction sa;

	/* set up an interrupt signal handler */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sig_int;
	err = sigaction(SIGINT, &sa, NULL);
	if (err) {
		fprintf(stderr, "error installing SIGINT handler: %d, %s\n",
		        errno, strerror(errno));
		return -1;
	}

	/* create the socket for the monitor events */
	event_socket = apple80211_event_create_socket();
	if (event_socket < 0) {
		err = -1;
		goto exit;
	}

	/* process and print events until we catch an interrupt or hit an error */
	while (!quit) {
		err = apple80211_event_process(wli->socket_fd, event_socket);
	}

exit:
	if (event_socket >= 0) {
		close(event_socket);
	}

	/* reset the interrupt signal handler */
	sa.sa_handler = SIG_DFL;
	err = sigaction(SIGINT, &sa, NULL);
	if (err) {
		fprintf(stderr, "error resetting SIGINT handler: %d, %s\n",
		        errno, strerror(errno));
		return -1;
	}

	return 0;
}

/* read and process one apple80211 event.
 * Event is printed to stdout and suplimentary information queried.
 * Eg. on a new BSSID event, the new BSSID is queried and printed.
 */
static int
apple80211_event_process(int if_socket, int event_socket)
{
	errno_t err = 0;
	uint event_data_len;
	uint payload_len;
	struct timeval tp;
	struct tm tm;
	char time_str[TIME_STR_SZ];
	/* make a larger message buffer start with a kern_event_msg structure */
	union {
		struct kern_event_msg msg;
		char buf[MSG_BUF_SZ];
	} msg_buf;
	struct kern_event_msg* msg;
	struct apple80211_event_data *apple80211_data;

	/* convenience pointer to kern_event_msg */
	msg = &msg_buf.msg;

	/* apple80211_event_data is the 'event_data' variable length section of
	 * the kern_event_msg.
	 */
	apple80211_data = (struct apple80211_event_data*)msg->event_data;

	/* get the next APPLE80211_EVENT */
	err = apple80211_event_get(event_socket, msg, MSG_BUF_SZ, &event_data_len);
	if (err) {
		return err;
	}

	/* Timestamp generation
	 * grab the current time, construct the struct tm so that it can be used by strftime()
	 */
	gettimeofday(&tp, NULL);
	localtime_r(&tp.tv_sec, &tm);
	if (strftime(time_str, TIME_STR_SZ, "%F %T", &tm) == 0) {
		time_str[0] = '0';
		time_str[1] = '\0';
	}

	/* print the initial part of the event line,
	 * timestamp, ifname, and event name
	 */
	printf("%s.%06d %-8.16s %s",
	       time_str, tp.tv_usec,
	       apple80211_data->ifname,
	       lookup_name(apple80211_event_names, msg->event_code));

	/* event_data_len is the length of the kern_event_msg.event_data section that holds the
	 * apple80211 event data.
	 * calc the length of just the variable apple80211 payload that follows the ifname
	 */
	payload_len = event_data_len - IFNAMSIZ;

	/* based on the type of the event, print more for the event line */
	switch (msg->event_code) {

	case APPLE80211_M_LINK_CHANGED:
		/* on a link event, print extra information in the event payload */
		if (payload_len < sizeof(apple80211_data->payload.link)) {
			printf(" event payload too short (%d bytes) for message\n", payload_len);
			break;
		}
		printf(" link %s reason %d\n",
		       apple80211_data->payload.link.up ? "UP" : "DOWN",
		       apple80211_data->payload.link.reason);
		break;

	case APPLE80211_M_LINK_QUALITY:
		/* on a link quality event, print extra information in the event payload */
		if (payload_len < sizeof(apple80211_data->payload.lq)) {
			printf(" event payload too short (%d bytes) for message\n", payload_len);
			break;
		}
		printf(" rssi %d dBm tx %u Mbps\n",
		       apple80211_data->payload.lq.rssi,
		       apple80211_data->payload.lq.tx_rate);
		break;

	case APPLE80211_M_BSSID_CHANGED: {
		/* on a BSSID event, query the new BSSID and print */
		struct ether_addr ea;

		/* get the current BSSID */
		err = apple80211_bssid_get(if_socket, apple80211_data->ifname, &ea);

		if (err == ENXIO) {
			printf(" -> <Not Asssociated>\n");
		} else if (err) {
			printf(" APPLE80211_IOC_BSSID error %d, %s\n", err, strerror(err));
		} else {
			printf(" -> %02X:%02X:%02X:%02X:%02X:%02X\n",
			       ea.octet[0], ea.octet[1], ea.octet[2],
			       ea.octet[3], ea.octet[4], ea.octet[5]);
		}
		err = 0;
		break;
	}

	case APPLE80211_M_SSID_CHANGED: {
		/* on an SSID event, query the new SSID and print */
		struct apple80211_ssid ssid;

		/* get the current BSSID */
		err = apple80211_ssid_get(if_socket, apple80211_data->ifname, &ssid);

		if (err == ENXIO) {
			printf(" -> <Not Asssociated>\n");
		} else if (err) {
			printf(" APPLE80211_IOC_SSID error %d, %s\n", err, strerror(err));
		} else {
			char ssid_buf[SSID_FMT_BUF_LEN];
			wl_format_ssid(ssid_buf, ssid.ssid_bytes, ssid.ssid_length);
			printf(" -> \"%s\"\n", ssid_buf);
		}
		err = 0;
		break;
	}

	case APPLE80211_M_STA_ARRIVE:
	case APPLE80211_M_STA_LEAVE: {
		/* on a STA Arrive or Leave event, print extra information in the event payload */
		struct ether_addr *ea = &apple80211_data->payload.ea;

		if (payload_len < ETHER_ADDR_LEN) {
			printf(" event payload too short (%d bytes) for MAC Address\n",
			       payload_len);
			break;
		}
		printf(" %02X:%02X:%02X:%02X:%02X:%02X\n",
		       ea->octet[0], ea->octet[1], ea->octet[2],
		       ea->octet[3], ea->octet[4], ea->octet[5]);
		break;
	}

	case APPLE80211_M_ACT_FRM_TX_COMPLETE: {
		/* on an action frame event, print extra information in the event payload */
		struct ether_addr *ea = &apple80211_data->payload.action_frame.dst;

		if (payload_len < sizeof(apple80211_data->payload.action_frame)) {
			printf(" event payload too short (%d bytes) for message\n", payload_len);
			break;
		}
		printf(" %02X:%02X:%02X:%02X:%02X:%02X token %d\n",
		       ea->octet[0], ea->octet[1], ea->octet[2],
		       ea->octet[3], ea->octet[4], ea->octet[5],
		       apple80211_data->payload.action_frame.token);
		break;
	}

	case APPLE80211_M_CHANNEL_SWITCH: {
		/* on channel switch event, print extra information in the event payload */
		apple80211_channel_switch_announcement_t *csa = &apple80211_data->payload.csa;

		if (payload_len < sizeof(apple80211_channel_switch_announcement_t)) {
			printf(" event payload too short (%d bytes) for channel switch info\n",
			       payload_len);
			break;
		}
		printf(" mode %u regClass %u channel %u (f:0x%X) count %u BI %u complete %u\n",
		       csa->mode, csa->regulatory_class,
		       csa->channel.channel, csa->channel.flags,
		       csa->count, csa->beacon_interval, csa->complete);
		break;
	}

	default:
		/* if no specific handler, print some information about any unhandled event payload,
		 * otherwise, just close out the line.
		 */
		if (payload_len > 0) {
			printf(" payload %u bytes\n", payload_len);
		} else {
			puts("");
		}
		break;
	}

	/* push stdout in case it is buffered */
	fflush(stdout);

	return err;
}

/* create and return the kernel event socket and set the filter
 * only monitor KEV_IEEE80211_CLASS/KEV_APPLE80211_EVENT_SUBCLASS events.
 */
static int
apple80211_event_create_socket()
{
	struct kev_request kev_req;
	int event_socket;
	int err;

	event_socket = socket(PF_SYSTEM, SOCK_RAW, SYSPROTO_EVENT);
	if (event_socket < 0) {
		fprintf(stderr, "error creating event socket: %d, %s\n",
		        errno, strerror(errno));
		return -1;
	}

	kev_req.vendor_code = KEV_VENDOR_APPLE;
	kev_req.kev_class = KEV_IEEE80211_CLASS;
	kev_req.kev_subclass = KEV_APPLE80211_EVENT_SUBCLASS;

	err = ioctl(event_socket, SIOCSKEVFILT, (void*)&kev_req);
	if (err) {
		fprintf(stderr, "error setting event filter: %d, %s\n", errno, strerror(errno));
		return -1;
	}

	return event_socket;
}

/* Read one generic kernel event */
static int
apple80211_event_get(int event_socket, struct kern_event_msg* msg, uint msg_len, uint *data_len)
{
	int err = 0;
	ssize_t ret;

	ret = recv(event_socket, msg, msg_len, 0);
	if (ret == -1) {
		err = errno;

		/* handle interrupt */
		if (err == EINTR) {
			printf(" interrupt caught, exit event monitor\n");
		} else {
			fprintf(stderr, "error reading event: %d, %s\n", err, strerror(err));
		}

		*data_len = 0;
		return err;
	}

	if (ret < KEV_MSG_HEADER_SIZE) {
		fprintf(stderr, "event socket read to short (%ld bytes) for kern_event_msg\n",
		        ret);

		err = EBADMSG;
		*data_len = 0;
	} else if (msg->total_size < KEV_MSG_HEADER_SIZE) {
		fprintf(stderr, "kern_event_msg.total_size (%d) too short for kern_event_msg\n",
		        msg->total_size);

		err = EBADMSG;
		*data_len = 0;
	} else {
		*data_len = msg->total_size - KEV_MSG_HEADER_SIZE;
	}

	return err;
}

/* read the BSSID using the apple80211 ioctl */
static int
apple80211_bssid_get(int if_socket, const char ifname[IFNAMSIZ], struct ether_addr *ea)
{
	uint iolen;
	int err = 0;

	iolen = ETHER_ADDR_LEN;
	err = wl_apple80211_ioctl_basic(if_socket, ifname, APPLE80211_IOC_BSSID, 0, ea, &iolen, 0);

	return err;
}

/* read the SSID using the apple80211 ioctl */
static int
apple80211_ssid_get(int if_socket, const char ifname[IFNAMSIZ], struct apple80211_ssid *ssid)
{
	uint iolen;
	int err = 0;

	iolen = APPLE80211_MAX_SSID_LEN;
	err = wl_apple80211_ioctl_basic(if_socket, ifname, APPLE80211_IOC_SSID, 0,
	                                ssid->ssid_bytes, &iolen, 0);

	if (iolen > APPLE80211_MAX_SSID_LEN) {
		err = ERANGE;
	}
	ssid->ssid_length = iolen;

	return err;
}

/* Name table lookup utility
 * Returns a pointer to the the name table entry matching the ID, or a string
 * with the ID if there is no match.
 */
static const char*
lookup_name(const struct name_entry* tbl, int id)
{
	const struct name_entry *elt = tbl;
	static char unknown[64];

	for (elt = tbl; elt->name != NULL; elt++) {
		if (id == elt->id)
			return elt->name;
	}

	snprintf(unknown, sizeof(unknown), "ID:%d", id);

	return unknown;
}

#define NAME_ENTRY(x) {x, #x}

#if (APPLE80211_M_MAX == 52)
/* Table of all the apple80211 event IDs for 10.9 defs */
static const struct name_entry apple80211_event_names[] =
{
	NAME_ENTRY(APPLE80211_M_POWER_CHANGED),
	NAME_ENTRY(APPLE80211_M_SSID_CHANGED),
	NAME_ENTRY(APPLE80211_M_BSSID_CHANGED),
	NAME_ENTRY(APPLE80211_M_LINK_CHANGED),
	NAME_ENTRY(APPLE80211_M_MIC_ERROR_UCAST),
	NAME_ENTRY(APPLE80211_M_MIC_ERROR_MCAST),
	NAME_ENTRY(APPLE80211_M_INT_MIT_CHANGED),
	NAME_ENTRY(APPLE80211_M_MODE_CHANGED),
	NAME_ENTRY(APPLE80211_M_ASSOC_DONE),
	NAME_ENTRY(APPLE80211_M_SCAN_DONE),
	NAME_ENTRY(APPLE80211_M_COUNTRY_CODE_CHANGED),
	NAME_ENTRY(APPLE80211_M_STA_ARRIVE),
	NAME_ENTRY(APPLE80211_M_STA_LEAVE),
	NAME_ENTRY(APPLE80211_M_DECRYPTION_FAILURE),
	NAME_ENTRY(APPLE80211_M_SCAN_CACHE_UPDATED),
	NAME_ENTRY(APPLE80211_M_INTERNAL_SCAN_DONE),
	NAME_ENTRY(APPLE80211_M_LINK_QUALITY),
	NAME_ENTRY(APPLE80211_M_IBSS_PEER_ARRIVED),
	NAME_ENTRY(APPLE80211_M_IBSS_PEER_LEFT),
	NAME_ENTRY(APPLE80211_M_RSN_HANDSHAKE_DONE),
	NAME_ENTRY(APPLE80211_M_BT_COEX_CHANGED),
	NAME_ENTRY(APPLE80211_M_P2P_PEER_DETECTED),
	NAME_ENTRY(APPLE80211_M_P2P_LISTEN_COMPLETE),
	NAME_ENTRY(APPLE80211_M_P2P_SCAN_COMPLETE),
	NAME_ENTRY(APPLE80211_M_P2P_LISTEN_STARTED),
	NAME_ENTRY(APPLE80211_M_P2P_SCAN_STARTED),
	NAME_ENTRY(APPLE80211_M_P2P_INTERFACE_CREATED),
	NAME_ENTRY(APPLE80211_M_P2P_GROUP_STARTED),
	NAME_ENTRY(APPLE80211_M_BGSCAN_NET_DISCOVERED),
	NAME_ENTRY(APPLE80211_M_ROAMED),
	NAME_ENTRY(APPLE80211_M_ACT_FRM_TX_COMPLETE),
	NAME_ENTRY(APPLE80211_M_DEAUTH_RECEIVED),
	NAME_ENTRY(APPLE80211_M_CHANNEL_SWITCH),
	NAME_ENTRY(APPLE80211_M_PEER_STATE),
	NAME_ENTRY(APPLE80211_M_AWDL_AVAILABILITY_WINDOW_START),
	NAME_ENTRY(APPLE80211_M_AWDL_AVAILABILITY_WINDOW_EXTENSION_START),
	NAME_ENTRY(APPLE80211_M_AWDL_AVAILABILITY_WINDOW_EXTENSIONS_END),
	NAME_ENTRY(APPLE80211_M_AWDL_INTERLEAVED_SCAN_START),
	NAME_ENTRY(APPLE80211_M_AWDL_INTERLEAVED_SCAN_STOP),
	NAME_ENTRY(APPLE80211_M_AWDL_SYNC_STATE_CHANGED),
	NAME_ENTRY(APPLE80211_M_PEER_PRESENCE),
	NAME_ENTRY(APPLE80211_M_AWDL_ACTION_FRAME_RX),
	NAME_ENTRY(APPLE80211_M_RESET_INTERFACE),
	NAME_ENTRY(APPLE80211_M_PEER_CREDIT_GRANT),
	NAME_ENTRY(APPLE80211_M_PEER_CACHE_CONTROL),
	NAME_ENTRY(APPLE80211_M_SERVICE_INDICATION),
	NAME_ENTRY(APPLE80211_M_AWDL_OOB_REQUEST_END),
	NAME_ENTRY(APPLE80211_M_DRIVER_AVAILABLE),
	{0, NULL}
};
#else
/* Table of all the apple80211 event IDs for MacOSX10.10/MacOSX10.11 defs */
static const struct name_entry apple80211_event_names[] =
{
	NAME_ENTRY(APPLE80211_M_POWER_CHANGED),
	NAME_ENTRY(APPLE80211_M_SSID_CHANGED),
	NAME_ENTRY(APPLE80211_M_BSSID_CHANGED),
	NAME_ENTRY(APPLE80211_M_LINK_CHANGED),
	NAME_ENTRY(APPLE80211_M_MIC_ERROR_UCAST),
	NAME_ENTRY(APPLE80211_M_MIC_ERROR_MCAST),
	NAME_ENTRY(APPLE80211_M_INT_MIT_CHANGED),
	NAME_ENTRY(APPLE80211_M_MODE_CHANGED),
	NAME_ENTRY(APPLE80211_M_ASSOC_DONE),
	NAME_ENTRY(APPLE80211_M_SCAN_DONE),
	NAME_ENTRY(APPLE80211_M_COUNTRY_CODE_CHANGED),
	NAME_ENTRY(APPLE80211_M_STA_ARRIVE),
	NAME_ENTRY(APPLE80211_M_STA_LEAVE),
	NAME_ENTRY(APPLE80211_M_DECRYPTION_FAILURE),
	NAME_ENTRY(APPLE80211_M_SCAN_CACHE_UPDATED),
	NAME_ENTRY(APPLE80211_M_INTERNAL_SCAN_DONE),
	NAME_ENTRY(APPLE80211_M_LINK_QUALITY),
	NAME_ENTRY(APPLE80211_M_IBSS_PEER_ARRIVED),
	NAME_ENTRY(APPLE80211_M_IBSS_PEER_LEFT),
	NAME_ENTRY(APPLE80211_M_RSN_HANDSHAKE_DONE),
	NAME_ENTRY(APPLE80211_M_BT_COEX_CHANGED),
	NAME_ENTRY(APPLE80211_M_P2P_PEER_DETECTED),
	NAME_ENTRY(APPLE80211_M_P2P_LISTEN_COMPLETE),
	NAME_ENTRY(APPLE80211_M_P2P_SCAN_COMPLETE),
	NAME_ENTRY(APPLE80211_M_P2P_LISTEN_STARTED),
	NAME_ENTRY(APPLE80211_M_P2P_SCAN_STARTED),
	NAME_ENTRY(APPLE80211_M_P2P_INTERFACE_CREATED),
	NAME_ENTRY(APPLE80211_M_WIFI_DIRECT_INTERFACE_CREATED),
	NAME_ENTRY(APPLE80211_M_P2P_GROUP_STARTED),
#if defined(APPLE80211_M_WIFI_DIRECT_GROUP_STARDED)
	NAME_ENTRY(APPLE80211_M_WIFI_DIRECT_GROUP_STARDED),
#endif
#if defined(APPLE80211_M_WIFI_DIRECT_GROUP_STARTED)
	NAME_ENTRY(APPLE80211_M_WIFI_DIRECT_GROUP_STARTED),
#endif
	NAME_ENTRY(APPLE80211_M_BGSCAN_NET_DISCOVERED),
	NAME_ENTRY(APPLE80211_M_ROAMED),
	NAME_ENTRY(APPLE80211_M_ACT_FRM_TX_COMPLETE),
	NAME_ENTRY(APPLE80211_M_DEAUTH_RECEIVED),
	NAME_ENTRY(APPLE80211_M_BLACKLIST_NETWORK),
	NAME_ENTRY(APPLE80211_M_RESUME_SCAN),
	NAME_ENTRY(APPLE80211_M_BGSCAN_SUSPENDED),
	NAME_ENTRY(APPLE80211_M_BGSCAN_RESUMED),
	NAME_ENTRY(APPLE80211_M_RSSI_CHANGED),
	NAME_ENTRY(APPLE80211_M_PEER_STATE),
	NAME_ENTRY(APPLE80211_M_AWDL_AVAILABILITY_WINDOW_START),
	NAME_ENTRY(APPLE80211_M_AWDL_AVAILABILITY_WINDOW_EXTENSION_START),
	NAME_ENTRY(APPLE80211_M_AWDL_AVAILABILITY_WINDOW_EXTENSIONS_END),
	NAME_ENTRY(APPLE80211_M_AWDL_INTERLEAVED_SCAN_START),
	NAME_ENTRY(APPLE80211_M_AWDL_INTERLEAVED_SCAN_STOP),
	NAME_ENTRY(APPLE80211_M_AWDL_SYNC_STATE_CHANGED),
	NAME_ENTRY(APPLE80211_M_PEER_PRESENCE),
	NAME_ENTRY(APPLE80211_M_AWDL_ACTION_FRAME_RX),
	NAME_ENTRY(APPLE80211_M_RESET_INTERFACE),
	NAME_ENTRY(APPLE80211_M_PEER_CREDIT_GRANT),
	NAME_ENTRY(APPLE80211_M_PEER_CACHE_CONTROL),
	NAME_ENTRY(APPLE80211_M_SERVICE_INDICATION),
	NAME_ENTRY(APPLE80211_M_AWDL_OOB_REQUEST_END),
	NAME_ENTRY(APPLE80211_M_CHANNEL_SWITCH),
	NAME_ENTRY(APPLE80211_M_DRIVER_AVAILABLE),
	NAME_ENTRY(APPLE80211_M_AWDL_BTLE_STATE_CHANGED),
	NAME_ENTRY(APPLE80211_M_GAS_DONE),
	NAME_ENTRY(APPLE80211_M_INTERFACE_STATE),
	NAME_ENTRY(APPLE80211_M_MAC_ADDRESS_CHANGED),
	NAME_ENTRY(APPLE80211_M_HOMECHAN_QUAL_CHANGED),
	NAME_ENTRY(APPLE80211_M_POWER_STATS_UPDATE),
	NAME_ENTRY(APPLE80211_M_AWDL_PSCAN_BEGIN),
	NAME_ENTRY(APPLE80211_M_BGSCAN_CACHED_NETWORK_AVAILABLE),
	NAME_ENTRY(APPLE80211_M_AWDL_TXCALIBRATION_REQUEST),
	NAME_ENTRY(APPLE80211_M_AWDL_STATISTICS),
	NAME_ENTRY(APPLE80211_M_ASSOC_READY),
	NAME_ENTRY(APPLE80211_M_AWDL_REALTIME_MODE_START),
	NAME_ENTRY(APPLE80211_M_AWDL_REALTIME_MODE_END),
#if defined(APPLE80211_M_AWDL_UNICAST_AF_STATUS) /* MacOSX10.11 defs */
	NAME_ENTRY(APPLE80211_M_AWDL_UNICAST_AF_STATUS),
	NAME_ENTRY(APPLE80211_M_ROAM_START),
	NAME_ENTRY(APPLE80211_M_ROAM_END),
	NAME_ENTRY(APPLE80211_M_DISSASOC_RECEIVED),
	NAME_ENTRY(APPLE80211_M_REASSOC),
	NAME_ENTRY(APPLE80211_M_AUTH),
	NAME_ENTRY(APPLE80211_M_PRUNE),
	NAME_ENTRY(APPLE80211_M_SUPPLICANT_EVENT),
	NAME_ENTRY(APPLE80211_M_AWDL_ADVERTISERS_LIST),
	NAME_ENTRY(APPLE80211_M_ASSOC),
#endif
	{0, NULL}
};
#endif /* APPLE80211_M_MAX */
