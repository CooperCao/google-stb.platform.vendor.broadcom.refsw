/*
 * WBD Related functions
 *
 * $Copyright Broadcom Corporation$
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: bsd_wbd.c 642646 2016-06-09 12:13:08Z spalanga $
 */

#include "bsd.h"
#include <fcntl.h>

/* Read "wbd_ifnames" NVRAM and get actual ifnames */
extern int wbd_read_actual_ifnames(char *wbd_ifnames1, int len1, bool create);

static bsd_wbd_weak_sta_policy_t wbd_predefined_policy[] = {
/* idle_rate rssi phyrate tx_failures flags */
/* 0: low rssi or phyrate or high tx_failure */
{1000, -65, 50, 20, 0x0000001C},
/* 1: low rssi or phyrate */
{1000, -65, 50, 0, 0x0000000C},
/* 2: low rssi BSD_POLICY_LOW_RSSI */
{1000, -65, 0, 0, 0x00000004},
/* 3: low phyrate BSD_POLICY_LOW_PHYRATE */
{1000, 0, 50, 0, 0x00000008},
/* 4: high tx_failures */
{1000, 0, 0, 20, 0x00000010},
/* 5: low rssi and phyrate and high tx_failure(all conditions should met) */
{1000, -65, 50, 20, 0x0000001D},
/* End */
{0, 0, 0, 0, 0}
};

static int bsd_wbd_find_weak_sta_policy(bsd_info_t *info, bsd_wbd_bss_list_t *wbd_bssinfo,
	bsd_sta_info_t *sta);

typedef int (*bsd_wbd_algo_t)(bsd_info_t *info, bsd_wbd_bss_list_t *wbd_bssinfo,
	bsd_sta_info_t *sta);

/* Weak STA identification Algorithm for WBD, should be only one and controlled by config
 * Currently the Index 0 is for BSD and index 1 for WBD's internal weak STA alogirthm.
 * This list should be mutually exclusive. Lets say if we add one algorithm here,
 * then NULL needs to be added to the list in WBD(variable predefined_wbd_wc_algo in file
 * wbd/slave/wbd_slave_control.c) at the same index.
 */
static bsd_wbd_algo_t wbd_predefined_algo[] = {
	bsd_wbd_find_weak_sta_policy,
	NULL
};

#define BSD_WBD_MAX_POLICY (sizeof(wbd_predefined_policy)/sizeof(bsd_wbd_weak_sta_policy_t) - 1)
#define BSD_WBD_MAX_ALGO (sizeof(wbd_predefined_algo)/sizeof(bsd_wbd_algo_t))

#define BSD_WBD_MIN_PHYRATE	6

static int
bsd_wbd_get_max_algo(bsd_info_t *info)
{
	UNUSED_PARAMETER(info);
	return BSD_WBD_MAX_ALGO;
}

static int
bsd_wbd_get_max_policy(bsd_info_t *info)
{
	UNUSED_PARAMETER(info);
	return BSD_WBD_MAX_POLICY;
}

static bsd_wbd_weak_sta_policy_t*
bsd_wbd_get_weak_sta_cfg(bsd_wbd_bss_list_t *wbd_bssinfo)
{
	return &wbd_predefined_policy[wbd_bssinfo->policy];
}

static char*
bsd_wbd_nvram_safe_get(const char *name, int *status)
{
	char *p = NULL;

	p = nvram_get(name);
	*status = p ? BSD_OK : BSD_FAIL;

	return p ? p : "";
}

/* Allocate WBD info */
static bsd_wbd_info_t*
bsd_wbd_info_alloc(void)
{
	bsd_wbd_info_t *info;

	BSD_ENTER();

	info = (bsd_wbd_info_t *)malloc(sizeof(*info));
	if (info == NULL) {
		BSD_PRINT("malloc fails\n");
	} else {
		memset(info, 0, sizeof(*info));
		BSD_WBD("info=%p\n", info);
	}

	BSD_EXIT();
	return info;
}

static bsd_wbd_bss_list_t*
bsd_wbd_add_bssinfo(bsd_wbd_info_t *wbd_info, bsd_bssinfo_t *bssinfo)
{
	bsd_wbd_bss_list_t *newnode;
	BSD_ENTER();

	newnode = (bsd_wbd_bss_list_t*)malloc(sizeof(*newnode));
	if (newnode == NULL) {
		BSD_WBD("Failed to allocate memory for bsd_wbd_bss_list_t\n");
		return NULL;
	}
	memset(newnode, 0, sizeof(*newnode));

	newnode->weak_sta_cfg =
		(bsd_wbd_weak_sta_policy_t*)malloc(sizeof(*(newnode->weak_sta_cfg)));
	if (newnode->weak_sta_cfg == NULL) {
		BSD_WBD("Failed to allocate memory for weak_sta_cfg\n");
		free(newnode);
		return NULL;
	}
	memset(newnode->weak_sta_cfg, 0, sizeof(*(newnode->weak_sta_cfg)));

	newnode->bssinfo = bssinfo;
	newnode->next = wbd_info->bss_list;
	wbd_info->bss_list = newnode;

	BSD_EXIT();
	return newnode;
}

/* Initializes the bss info for WBD */
static int
bsd_wbd_init_bss(bsd_info_t *info, bsd_wbd_bss_list_t *wbd_bssinfo)
{
	char *str, *endptr = NULL, *prefix;
	char tmp[100];
	int ret, num;
	bsd_wbd_weak_sta_policy_t policy;

	prefix = wbd_bssinfo->bssinfo->prefix;
	/* index to weak STA finding algorithm. */
	str = bsd_wbd_nvram_safe_get(strcat_r(prefix, BSD_WBD_NVRAM_WEAK_STA_ALGO, tmp), &ret);
	if (ret == BSD_OK) {
		wbd_bssinfo->algo = (uint8)strtol(str, &endptr, 0);
		if (wbd_bssinfo->algo >= bsd_wbd_get_max_algo(info))
			wbd_bssinfo->algo = 0;
		if (wbd_predefined_algo[wbd_bssinfo->algo] == NULL) {
			BSD_WBD("WBD Algo[%d] is NULL\n", wbd_bssinfo->algo);
			return BSD_FAIL;
		}
	}

	/* Index to weak STA finding configuration */
	str = bsd_wbd_nvram_safe_get(strcat_r(prefix, BSD_WBD_NVRAM_WEAK_STA_POLICY, tmp), &ret);
	if (ret == BSD_OK) {
		wbd_bssinfo->policy = (uint8)strtol(str, &endptr, 0);
		if (wbd_bssinfo->policy >= bsd_wbd_get_max_policy(info))
			wbd_bssinfo->policy = 0;
	}

	memcpy(wbd_bssinfo->weak_sta_cfg, bsd_wbd_get_weak_sta_cfg(wbd_bssinfo),
		sizeof(*wbd_bssinfo->weak_sta_cfg));

	/* Configurations defined in NVRAM */
	str = bsd_wbd_nvram_safe_get(strcat_r(prefix, BSD_WBD_NVRAM_WEAK_STA_CFG, tmp), &ret);

	if (ret == BSD_OK) {
		num = sscanf(str, "%d %d %d %d %x",
			&policy.idle_rate, &policy.rssi, &policy.phyrate, &policy.tx_failures,
			&policy.flags);
		if (num == 5) {
			memcpy(wbd_bssinfo->weak_sta_cfg, &policy,
				sizeof(*wbd_bssinfo->weak_sta_cfg));
		} else {
			BSD_ERROR("BSS[%s] %s[%s] format error\n", wbd_bssinfo->bssinfo->ifnames,
				BSD_WBD_NVRAM_WEAK_STA_CFG, str);
		}
	}

	BSD_WBD("Algo[%d] weak_sta_policy[%d]: "
		"idle_rate=%d rssi=%d phyrate=%d "
		"tx_failures=%d flags=0x%x\n",
		wbd_bssinfo->algo, wbd_bssinfo->policy,
		wbd_bssinfo->weak_sta_cfg->idle_rate,
		wbd_bssinfo->weak_sta_cfg->rssi,
		wbd_bssinfo->weak_sta_cfg->phyrate,
		wbd_bssinfo->weak_sta_cfg->tx_failures,
		wbd_bssinfo->weak_sta_cfg->flags);

	return BSD_OK;
}

/* As there is no deault ifnames for BSD, setting ifnames for bsd to detect weak client for
 * WBD if there is no ifnames for BSD.
 */
int
bsd_wbd_set_ifnames(bsd_info_t *info)
{
	char var_intf[BSD_IFNAME_SIZE];
	char *next_intf;
	char bsd_ifnames[80], wbd_ifnames[80];

	BSDSTRNCPY(bsd_ifnames, nvram_safe_get(BSD_IFNAMES_NVRAM), sizeof(bsd_ifnames) - 1);
	BSDSTRNCPY(wbd_ifnames, nvram_safe_get(BSD_WBD_NVRAM_IFNAMES), sizeof(wbd_ifnames) - 1);
	BSD_WBD("nvram %s=%s and %s=%s\n", BSD_IFNAMES_NVRAM, bsd_ifnames,
		BSD_WBD_NVRAM_IFNAMES, wbd_ifnames);

	if (strlen(wbd_ifnames) <= 0) {
		BSD_WBD("%s not specified. So skipping\n", BSD_WBD_NVRAM_IFNAMES);
		return BSD_FAIL;
	}

	if (strlen(bsd_ifnames) > 0) {
		/* Now add missing interface from wbd_ifnames to bsd_ifnames */
		foreach(var_intf, wbd_ifnames, next_intf) {
			if (find_in_list(bsd_ifnames, var_intf) == NULL) {
				BSD_WBD("Adding missing %s interface which is in %s to %s\n",
					var_intf, BSD_WBD_NVRAM_IFNAMES, BSD_IFNAMES_NVRAM);
				add_to_list(var_intf, bsd_ifnames, sizeof(bsd_ifnames));
			}
		}
		BSD_WBD("Final %s=%s\n", BSD_IFNAMES_NVRAM, bsd_ifnames);
		nvram_set(BSD_IFNAMES_NVRAM, bsd_ifnames);
		return BSD_OK;
	}

	/* If bsd_ifnames not present copy wbd_ifnames to bsd_ifnames */
	nvram_set(BSD_IFNAMES_NVRAM, wbd_ifnames);
	BSD_WBD("Successfully set bsd_ifnames same as %s = %s\n", BSD_WBD_NVRAM_IFNAMES,
		wbd_ifnames);

	return BSD_OK;
}

/* Allocate and initialize WBD related information */
int
bsd_wbd_init(bsd_info_t *info)
{
	bsd_wbd_info_t *wbd_info;
	char ifnames[64], name[IFNAMSIZ], *next = NULL;
	int idx_intf, idx, ret = BSD_OK;
	bsd_intf_info_t *intf_info;
	bsd_bssinfo_t *bssinfo;
	bsd_wbd_bss_list_t *cur;

	BSD_ENTER();

	if ((info->enable_flag & BSD_FLAG_WBD_ENABLED) == 0)
		goto done;

	/* Get the ifnames */
	/* Read "wbd_ifnames" NVRAM and get actual ifnames */
	if (wbd_read_actual_ifnames(ifnames, sizeof(ifnames), FALSE) != 0) {
		BSD_EXIT();
		return BSD_FAIL;
	}
	BSD_WBD("wbd_ifnames=%s\n", ifnames);

	wbd_info = bsd_wbd_info_alloc();
	if (wbd_info == NULL) {
		BSD_EXIT();
		return BSD_FAIL;
	}
	info->wbd_info = wbd_info;

	/* Now for each ifname find corresponding bss_info structure */
	foreach(name, ifnames, next) {
		/* For this ifname find the bssinfo */
		for (idx_intf = 0; idx_intf < info->max_ifnum; idx_intf++) {
			intf_info = &(info->intf_info[idx_intf]);
			for (idx = 0; idx < WL_MAXBSSCFG; idx++) {
				bssinfo = &(intf_info->bsd_bssinfo[idx]);
				if (bssinfo->valid &&
					(strncmp(name, bssinfo->ifnames, strlen(name)) == 0)) {
					BSD_WBD("Matching name=%s and bssinfo->ifnames=%s. BSSINFO"
						" ssid=%s BSSID="MACF"\n", name, bssinfo->ifnames,
						bssinfo->ssid, ETHER_TO_MACF(bssinfo->bssid));
					cur = bsd_wbd_add_bssinfo(wbd_info, bssinfo);
					if (cur) {
						ret = bsd_wbd_init_bss(info, cur);
						if (ret != BSD_OK)
							goto done;
					}
				}
			}
		}
	}

done:
	BSD_EXIT();
	return ret;
}

/* Cleanup all WiFi Blanket related info */
void
bsd_cleanup_wbd(bsd_wbd_info_t *info)
{
	BSD_ENTER();

	if (info != NULL) {
		bsd_wbd_bss_list_t *cur, *next;

		/* Free the memory allocated for bss on which WiFi Blanket is enabled */
		cur = info->bss_list;
		while (cur != NULL) {
			next = cur->next;
			if (cur->weak_sta_cfg)
				free(cur->weak_sta_cfg);
			free(cur);
			cur = next;
		}

		/* Free main info */
		free(info);
		BSD_WBD("WBD Cleanup Done\n");
	}

	BSD_EXIT();
}

/* Sends the data to socket */
static int
bsd_wbd_socket_send_data(int sockfd, char *data, unsigned int len)
{
	int	nret = 0;
	int	totalsize = len, totalsent = 0;
	BSD_ENTER();

	/* Loop till all the data sent */
	while (totalsent < totalsize) {
		fd_set WriteFDs;
		struct timeval tv;

		FD_ZERO(&WriteFDs);

		if (sockfd == BSD_DFLT_FD)
			return BSD_DFLT_FD;

		FD_SET(sockfd, &WriteFDs);

		tv.tv_sec = 5;
		tv.tv_usec = 0;
		if (select(sockfd+1, NULL, &WriteFDs, NULL, &tv) > 0) {
			if (FD_ISSET(sockfd, &WriteFDs))
				;
			else {
				BSD_EXIT();
				return BSD_DFLT_FD;
			}
		}

		nret = send(sockfd, &(data[totalsent]), len, 0);
		if (nret < 0) {
			BSD_WBD("send error is : %s\n", strerror(errno));
			BSD_EXIT();
			return BSD_DFLT_FD;
		}
		totalsent += nret;
		len -= nret;
		nret = 0;
	}

	BSD_EXIT();
	return totalsent;
}

/* To recieve data */
static int
bsd_wbd_socket_recv_data(int sockfd, char *read_buf, int read_buf_len)
{
	unsigned int nbytes, totalread = 0;
	struct timeval tv;
	fd_set ReadFDs, ExceptFDs;

	FD_ZERO(&ReadFDs);
	FD_ZERO(&ExceptFDs);
	FD_SET(sockfd, &ReadFDs);
	FD_SET(sockfd, &ExceptFDs);
	tv.tv_sec = 5; /* 5 seconds */
	tv.tv_usec = 0;

	/* Read till the null character or error */
	while (1) {
		/* Allocate memory for the buffer */
		if (totalread >= read_buf_len) {
			return totalread;
		}
		if (select(sockfd+1, &ReadFDs, NULL, &ExceptFDs, &tv) > 0) {
			if (FD_ISSET(sockfd, &ReadFDs)) {
				/* fprintf(stdout, "SOCKET : Data is ready to read\n"); */;
			} else {
				return BSD_DFLT_FD;
			}
		}

		nbytes = read(sockfd, read_buf+totalread, read_buf_len);
		totalread += nbytes;

		if (nbytes <= 0) {
			BSD_WBD("read error is : %s\n", strerror(errno));
			return BSD_DFLT_FD;
		}

		/* Check the last byte for NULL termination */
		if (read_buf[totalread-1] == '\0') {
			break;
		}
	}

	return totalread;
}

/* Connects to the server given the IP address and port number */
static int
bsd_wbd_connect_to_server(char* straddrs, unsigned int nport)
{
	struct sockaddr_in server_addr;
	int res, valopt;
	long arg;
	fd_set readfds;
	struct timeval tv;
	socklen_t lon;
	int sockfd;
	BSD_ENTER();

	sockfd = BSD_DFLT_FD;
	memset(&server_addr, 0, sizeof(server_addr));

	BSD_WBD("Connecting to server = %s\t port = %d\n", straddrs, nport);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		BSD_WBD("Error in socket is : %s\n", strerror(errno));
		goto error;
	}

	/* Set nonblock on the socket so we can timeout */
	if ((arg = fcntl(sockfd, F_GETFL, NULL)) < 0 ||
		fcntl(sockfd, F_SETFL, arg | O_NONBLOCK) < 0) {
			BSD_WBD("Error in fcntl is : %s\n", strerror(errno));
			goto error;
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(nport);
	server_addr.sin_addr.s_addr = inet_addr(straddrs);

	res = connect(sockfd, (struct sockaddr*)&server_addr,
		sizeof(struct sockaddr));
	if (res < 0) {
		if (errno == EINPROGRESS) {
			tv.tv_sec = 5;
			tv.tv_usec = 0;
			FD_ZERO(&readfds);
			FD_SET(sockfd, &readfds);
			if (select(sockfd+1, NULL, &readfds, NULL, &tv) > 0) {
				lon = sizeof(int);
				getsockopt(sockfd, SOL_SOCKET, SO_ERROR,
					(void*)(&valopt), &lon);
				if (valopt) {
					BSD_WBD("Error in connection() %d - %s\n",
						valopt, strerror(valopt));
					goto error;
				}
			} else {
				BSD_WBD("Timeout or error() %d - %s\n",
					valopt, strerror(valopt));
				goto error;
			}
		} else {
			BSD_WBD("Error connecting %d - %s\n",
				errno, strerror(errno));
			goto error;
		}
	}
	BSD_WBD("Connection Successfull with server : %s on port : %d\n", straddrs, nport);

	BSD_EXIT();
	return sockfd;

	/* Error handling */
error:
	if (sockfd != BSD_DFLT_FD)
		close(sockfd);
	BSD_EXIT();
	return BSD_DFLT_FD;
}

/* Sends the request to WBD server */
static int
bsd_wbd_send_req(char *data, char *read_buf, int read_buf_len)
{
	int sockfd = BSD_DFLT_FD, ret = BSD_OK;
	BSD_ENTER();

	/* Connect to the server */
	sockfd = bsd_wbd_connect_to_server(BSD_WBD_LOOPBACK_IP, BSD_WBD_SERVERT_PORT);
	if (sockfd == BSD_DFLT_FD) {
		ret = BSD_FAIL;
		goto end;
	}

	/* Send the data */
	if (bsd_wbd_socket_send_data(sockfd, data, strlen(data)+1) <= 0) {
		ret = BSD_FAIL;
		goto end;
	}

	/* Recieve the data */
	if (bsd_wbd_socket_recv_data(sockfd, read_buf, read_buf_len) <= 0) {
		ret = BSD_FAIL;
		goto end;
	}

end:
	if (sockfd != BSD_DFLT_FD)
		close(sockfd);

	BSD_EXIT();
	return ret;
}

/* Get the value for the token
 * Buffer is in the format "resp&mac=XX:XX:XX:XX:XX:XX&errorcode=1"
 */
static int
bsd_wbd_extract_token_val(char *data, const char *token, char *output, int len)
{
	char *p, *c, *val;
	char copydata[BSD_WBD_REQ_BUFSIZE];

	if (data == NULL)
		goto err;

	BSDSTRNCPY(copydata, data, sizeof(copydata));

	p = strstr(copydata, token);
	if (!p)
		goto err;

	if ((c = strchr(p, '&')))
		*c++ = '\0';

	val = strchr(p, '=');
	if (!val)
		goto err;

	val += 1;
	BSD_WBD("Token[%s] value[%s]\n", token, val);

	BSDSTRNCPY(output, val, len);

	return strlen(output);

err:
	return BSD_FAIL;
}

/* Get the value from the buffer for token.
 * Buffer is in the format "resp&mac=XX:XX:XX:XX:XX:XX&errorcode=1&dwell=100"
 */
static int
bsd_wbd_get_token_val(char *data, char *token, int *value)
{
	char output[10];

	memset(output, 0, sizeof(output));
	if (bsd_wbd_extract_token_val(data, token, output, sizeof(output)) <= 0)
		return BSD_FAIL;
	*value = atoi(output);

	return BSD_OK;
}

/* Creates the request and sends the weak client request to WBD */
static int
bsd_wbd_send_weak_client_req(bsd_info_t *info, bsd_bssinfo_t *bssinfo, bsd_sta_info_t *sta,
	int *wbd_ret)
{
	char data[BSD_WBD_REQ_BUFSIZE], read_buf[BSD_WBD_REQ_BUFSIZE];
	int ret = BSD_OK;
	BSD_ENTER();

	snprintf(data, sizeof(data), "{ \"Cmd\": \"WEAK_CLIENT_BSD\", "
		"\"MAC\": \""MACF"\", \"Band\": %d, \"Data\": { \"MAC\": \""MACF"\"} }",
		ETHER_TO_MACF(bssinfo->bssid), bssinfo->intf_info->band, ETHER_TO_MACF(sta->addr));
	BSD_WBD("Request Data : %s\n", data);

	ret = bsd_wbd_send_req(data, read_buf, sizeof(read_buf));
	if (ret == BSD_OK) {
		BSD_WBD("Read Data : %s\n", read_buf);
		ret = bsd_wbd_get_token_val(read_buf, "errorcode", wbd_ret);
	}

	BSD_EXIT();
	return ret;
}

/* Creates the request and sends the weak cancel request to WBD */
static int
bsd_wbd_send_weak_cancel_req(bsd_info_t *info, bsd_bssinfo_t *bssinfo, bsd_sta_info_t *sta,
	int *wbd_ret)
{
	char data[BSD_WBD_REQ_BUFSIZE], read_buf[BSD_WBD_REQ_BUFSIZE];
	int ret = BSD_OK;
	BSD_ENTER();

	snprintf(data, sizeof(data), "{ \"Cmd\": \"WEAK_CANCEL_BSD\", "
		"\"MAC\": \""MACF"\", \"Band\": %d, \"Data\": { \"MAC\": \""MACF"\"} }",
		ETHER_TO_MACF(bssinfo->bssid), bssinfo->intf_info->band, ETHER_TO_MACF(sta->addr));
	BSD_WBD("Request Data : %s\n", data);

	ret = bsd_wbd_send_req(data, read_buf, sizeof(read_buf));
	if (ret == BSD_OK) {
		BSD_WBD("Read Data : %s\n", read_buf);
		ret = bsd_wbd_get_token_val(read_buf, "errorcode", wbd_ret);
	}

	BSD_EXIT();
	return ret;
}

/* Creates the request and sends the sta status request to WBD */
static int
bsd_wbd_send_sta_status_req(bsd_info_t *info, bsd_bssinfo_t *bssinfo, bsd_sta_info_t *sta,
	int *wbd_ret)
{
	char data[BSD_WBD_REQ_BUFSIZE], read_buf[BSD_WBD_REQ_BUFSIZE];
	int ret = BSD_OK;
	BSD_ENTER();

	snprintf(data, sizeof(data), "{ \"Cmd\": \"STA_STATUS_BSD\", "
		"\"MAC\": \""MACF"\", \"Band\": %d, \"Data\": { \"MAC\": \""MACF"\"} }",
		ETHER_TO_MACF(bssinfo->bssid), bssinfo->intf_info->band, ETHER_TO_MACF(sta->addr));
	BSD_WBD("Request Data : %s\n", data);

	ret = bsd_wbd_send_req(data, read_buf, sizeof(read_buf));
	if (ret == BSD_OK) {
		BSD_WBD("Read Data : %s\n", read_buf);
		ret = bsd_wbd_get_token_val(read_buf, "errorcode", wbd_ret);
		if ((*wbd_ret) == BSDE_WBD_BOUNCING_STA) {
			ret = bsd_wbd_get_token_val(read_buf, "dwell", (int*)&sta->wbd_dwell_time);
			if (ret == BSD_OK)
				sta->wbd_dwell_start = time(NULL);
		}
	}

	BSD_EXIT();
	return ret;
}

/* Special STA check for WBD */
static int
bsd_wbd_special_sta_check(bsd_info_t *info, bsd_wbd_bss_list_t *wbd_bssinfo,
	bsd_sta_info_t *sta)
{
	int ret = BSD_OK, wbd_ret = BSDE_WBD_FAIL;
	BSD_ENTER();

	/* Don't consider ignored STAs */
	if ((sta->wbd_sta_status & BSD_WBD_STA_IGNORE)) {
		BSD_WBD("ifname[%s] STA["MACF"] Ignored[0x%X]\n",
			wbd_bssinfo->bssinfo->ifnames, ETHER_TO_MACF(sta->addr),
			sta->wbd_sta_status);
		ret = BSD_FAIL;
		goto end;
	}

	/* Get STA status from WBD for the pending weak STAs */
	if ((sta->wbd_sta_status & BSD_WBD_STA_WEAK_PENDING)) {
		ret = bsd_wbd_send_sta_status_req(info, wbd_bssinfo->bssinfo, sta, &wbd_ret);

		/* If we get successfull response. remove weak pending */
		if (ret == BSD_OK) {
			sta->wbd_sta_status &= ~(BSD_WBD_STA_WEAK_PENDING);
		}

		if (wbd_ret == BSDE_WBD_OK) {
			sta->wbd_sta_status |= BSD_WBD_STA_WEAK;
		} else if (wbd_ret == BSDE_WBD_IGNORE_STA) {
			sta->wbd_sta_status |= BSD_WBD_STA_IGNORE;
		} else if (wbd_ret == BSDE_WBD_BOUNCING_STA) {
			sta->wbd_sta_status |= BSD_WBD_STA_DWELL;
			sta->wbd_sta_status &= ~(BSD_WBD_STA_WEAK);
		} else {
			sta->wbd_sta_status &= ~(BSD_WBD_STA_WEAK);
		}
		BSD_WBD("STA["MACF"] in ifname[%s] status[0x%x] dwell[%lu] dwell_start[%lu]\n",
			ETHER_TO_MACF(sta->addr), wbd_bssinfo->bssinfo->ifnames,
			sta->wbd_sta_status, (unsigned long)sta->wbd_dwell_time,
			(unsigned long)sta->wbd_dwell_start);
	}

	/* Skip the WBD's bouncing STA */
	if ((sta->wbd_sta_status & BSD_WBD_STA_DWELL)) {
		time_t now = time(NULL);
		time_t gap;

		gap = now - sta->wbd_dwell_start;
		/* Check if the STA still in dwell state */
		if (sta->wbd_dwell_time > gap) {
			BSD_WBD("Still in dwell state. STA["MACF"] in ifname[%s] "
				"status[0x%x] dwell[%lu] dwell_start[%lu] gap[%lu]\n",
				ETHER_TO_MACF(sta->addr), wbd_bssinfo->bssinfo->ifnames,
				sta->wbd_sta_status, (unsigned long)sta->wbd_dwell_time,
				(unsigned long)sta->wbd_dwell_start, (unsigned long)gap);
			ret = BSD_FAIL;
			goto end;
		}
		/* Dwell state is expired. so, remove the dwell state */
		sta->wbd_sta_status &= ~(BSD_WBD_STA_DWELL);
		sta->wbd_dwell_start = 0;
		sta->wbd_dwell_time = 0;
	}

end:
	BSD_EXIT();
	return ret;
}

/* Get the mask for all the rules defined in flag */
static uint32
bsd_wbd_get_weak_sta_select_mask(bsd_wbd_weak_sta_policy_t *cfg)
{
	uint32 mask = 0;

	if (cfg->flags & BSD_WBD_WEAK_STA_POLICY_FLAG_RULE) {
		mask = ((cfg->flags) & (BSD_WBD_WEAK_STA_POLICY_FLAG_RSSI |
			BSD_WBD_WEAK_STA_POLICY_FLAG_PHYRATE |
			BSD_WBD_WEAK_STA_POLICY_FLAG_TXFAILURE));
	}

	return (mask);
}

/* Check whether the STA is weak or not */
static int
bsd_wbd_find_weak_sta_policy(bsd_info_t *info, bsd_wbd_bss_list_t *wbd_bssinfo,
	bsd_sta_info_t *sta)
{
	bsd_bssinfo_t *bssinfo;
	int isweak = FALSE, fail_cnt = 0;
	bool active_sta_check;
	bool check_rule = 0;
	uint32 check_rule_mask = 0; /* a sta select config mask for AND logic */
	uint32 sta_rule_met = 0; /* processed mask for AND logic */
	uint32 adj_mcs_phyrate = 0;
	BSD_ENTER();

	bssinfo = wbd_bssinfo->bssinfo;
	BSD_WBD("STA="MACF" in ifname=%s ssid=%s BSSID="MACF" cfg_flags[0x%x]\n",
		ETHER_TO_MACF(sta->addr), bssinfo->ifnames,
		bssinfo->ssid, ETHER_TO_MACF(bssinfo->bssid), wbd_bssinfo->weak_sta_cfg->flags);
	BSD_WBD("RSSI[%d] steer_flag[%d] band[%d] tx_rate[%d] flags[0x%X] "
		"phyrate[%d] datarate[%d] mcs_phyrate[%d] tx_bps[%d] rx_bps[%d] "
		"tx_failures[%d]\n",
		sta->rssi, sta->steerflag, sta->band, sta->tx_rate, sta->flags,
		sta->phyrate, sta->datarate, sta->mcs_phyrate, sta->tx_bps, sta->rx_bps,
		sta->tx_failures);

	if (bsd_bcm_special_sta_check(info, sta)) {
		BSD_WBD("sta[%p]:"MACF" - skip\n", sta, ETHERP_TO_MACF(&sta->addr));
		goto end;
	}

	/* skipped non-steerable STA */
	if (sta->steerflag & BSD_BSSCFG_NOTSTEER) {
		BSD_WBD("sta[%p]:"MACF" is not steerable. Skipped.\n",
			sta, ETHERP_TO_MACF(&sta->addr));
		goto end;
	}

	/* Skipped macmode mismatch STA */
	if (bsd_aclist_steerable(bssinfo, &sta->addr) == BSD_FAIL) {
		BSD_WBD("sta[%p]:"MACF" not steerable match w/ static maclist. Skipped.\n",
			sta, ETHERP_TO_MACF(&sta->addr));
		goto end;
	}

	/* check_rule, check_rule_mask and sta_rule_met are used for checking policy flags in AND
	 * and OR format. If the check_rule is TRUE, then all the rules defined in flags
	 * should be met. If it is FALSE, the STA will be weak if any one of the rules defined
	 * in the flag is met.
	 */
	check_rule = (wbd_bssinfo->weak_sta_cfg->flags & BSD_WBD_WEAK_STA_POLICY_FLAG_RULE);

	/* active sta check if bit is 0, not check if bit is 1 */
	active_sta_check =
		(wbd_bssinfo->weak_sta_cfg->flags & BSD_WBD_WEAK_STA_POLICY_FLAG_ACTIVE_STA) ?
		0 : 1;
	/* Skipped idle, or active STA */
	if (active_sta_check && (sta->datarate > wbd_bssinfo->weak_sta_cfg->idle_rate)) {
		BSD_WBD("Skip %s STA:"MACF" idle_rate[%d] tx+rx_rate[%d: %d+%d]\n",
			active_sta_check ? "active" : "idle",
			ETHERP_TO_MACF(&sta->addr),
			wbd_bssinfo->weak_sta_cfg->idle_rate,
			sta->datarate,
			sta->tx_bps, sta->rx_bps);
		goto end;
	}

	/* Skipped bouncing STA */
	if (bsd_check_bouncing_sta(info, &sta->addr)) {
		BSD_WBD("Skip bouncing STA:"MACF", skip ..\n",
			ETHERP_TO_MACF(&sta->addr));
		goto end;
	}

	/* check for RSSI threshold */
	if (wbd_bssinfo->weak_sta_cfg->flags & BSD_WBD_WEAK_STA_POLICY_FLAG_RSSI) {
		if (sta->rssi < wbd_bssinfo->weak_sta_cfg->rssi) {
			sta_rule_met |= BSD_WBD_WEAK_STA_POLICY_FLAG_RSSI;
			BSD_WBD("rssi[%d] threshold[%d]\n",
				sta->rssi, wbd_bssinfo->weak_sta_cfg->rssi);
			fail_cnt++;
			if (check_rule == 0)
				goto weakfound;
		}
	}

	/* Check for phyrate(tx_rate) */
	if (wbd_bssinfo->weak_sta_cfg->flags & BSD_WBD_WEAK_STA_POLICY_FLAG_PHYRATE) {
		/* Using tx_rate directly because, if the tx_rate drops drastically,
		 * the mcs_phyrate is not getting updated.
		 * Adjust MCS phyrate by filtering bogus tx_rate
		 */
		adj_mcs_phyrate = (sta->tx_rate < BSD_MAX_DATA_RATE) ? sta->tx_rate : 0;
		if ((adj_mcs_phyrate > BSD_WBD_MIN_PHYRATE) &&
			(adj_mcs_phyrate < wbd_bssinfo->weak_sta_cfg->phyrate)) {
			sta_rule_met |= BSD_WBD_WEAK_STA_POLICY_FLAG_PHYRATE;
			BSD_WBD("sta->tx_rate[%d] adj_mcs_phyrate[%d] threshold[%d]\n",
				sta->tx_rate, adj_mcs_phyrate, wbd_bssinfo->weak_sta_cfg->phyrate);
			fail_cnt++;
			if (check_rule == 0)
				goto weakfound;
		}
	}

	/* check for Tx Failure threshold */
	if (wbd_bssinfo->weak_sta_cfg->flags & BSD_WBD_WEAK_STA_POLICY_FLAG_TXFAILURE) {
		if (sta->tx_failures > wbd_bssinfo->weak_sta_cfg->tx_failures) {
			sta_rule_met |= BSD_WBD_WEAK_STA_POLICY_FLAG_TXFAILURE;
			BSD_WBD("tx_failures[%d] threshold[%d]\n",
				sta->tx_failures, wbd_bssinfo->weak_sta_cfg->tx_failures);
			fail_cnt++;
			if (check_rule == 0)
				goto weakfound;
		}
	}

weakfound:
	/* If the fail_cnt is not 0, then its a weak client */
	if (fail_cnt != 0) {
		/* logic AND all for the above. If check rule is TRUE, the rules which are
		 * met should be equal to the rules defined in the flags
		 */
		if (check_rule) {
			check_rule_mask = bsd_wbd_get_weak_sta_select_mask(
				wbd_bssinfo->weak_sta_cfg);
			BSD_WBD("STA="MACF" fail_cnt=%d AND check check_rule_mask:0x%x\n",
				ETHER_TO_MACF(sta->addr), fail_cnt, check_rule_mask);
			if ((sta_rule_met & check_rule_mask) == check_rule_mask) {
				BSD_WBD("STA="MACF" AND logic rule met - 0x%x\n",
					ETHER_TO_MACF(sta->addr), sta_rule_met);
			} else {
				BSD_WBD("STA="MACF" AND logic rule not met - 0x%x, skip\n",
					ETHER_TO_MACF(sta->addr), sta_rule_met);
				goto end;
			}
		}

		BSD_WBD("Found weak STA="MACF" fail_cnt=%d in ifname=%s ssid=%s BSSID="MACF"\n",
			ETHER_TO_MACF(sta->addr), fail_cnt, bssinfo->ifnames,
			bssinfo->ssid, ETHER_TO_MACF(bssinfo->bssid));
		isweak = TRUE;
	}

end:
	BSD_EXIT();
	return isweak;
}

/* Checks the STA is weak or not using algorithms defined */
static int
bsd_wbd_is_weak_sta(bsd_info_t *info, bsd_wbd_bss_list_t *wbd_bssinfo, bsd_sta_info_t *sta)
{
	return (wbd_predefined_algo[wbd_bssinfo->algo])(info, wbd_bssinfo, sta);
}

/* Checks for the weak STAs in all the BSS on which WBD is enabled */
void
bsd_wbd_check_weak_sta(bsd_info_t *info)
{
	int ret;
	bsd_wbd_info_t *wbd_info;
	bsd_wbd_bss_list_t *cur;
	bsd_bssinfo_t *bssinfo;
	bsd_sta_info_t *sta = NULL;
	BSD_ENTER();

	BCM_REFERENCE(ret);

	if ((info->enable_flag & BSD_FLAG_WBD_ENABLED) == 0) {
		BSD_WBD("WBD enable_flag[%d]. So skipping\n", info->enable_flag);
		goto end;
	}

	wbd_info = info->wbd_info;
	cur = wbd_info->bss_list;

	if (cur == NULL) {
		BSD_WBD("Ifnames not specified for WBD. So skipping\n");
		goto end;
	}

	/* For each BSS */
	while (cur) {
		bssinfo = cur->bssinfo;
		if (!bssinfo) {
			BSD_WBD("BSS INFO is NULL\n");
			goto nextbss;
		}
		BSD_WBD("Checking STAs in ifname=%s ssid=%s BSSID="MACF"\n", bssinfo->ifnames,
			bssinfo->ssid, ETHER_TO_MACF(bssinfo->bssid));

		/* assoclist */
		sta = bssinfo->assoclist;
		while (sta) {
			int wbd_ret = BSDE_WBD_FAIL;

			/* Skip DWDS STA */
			if ((sta->wbd_sta_status & BSD_WBD_STA_DWDS))
				goto nextsta;

			/* Do a wbd specific STA check */
			if (bsd_wbd_special_sta_check(info, cur, sta) != BSD_OK)
				goto nextsta;

			if (bsd_wbd_is_weak_sta(info, cur, sta) == TRUE) {
				/* If its already weak, don't inform to WBD */
				if (!(sta->wbd_sta_status & BSD_WBD_STA_WEAK)) {
					ret = bsd_wbd_send_weak_client_req(info, bssinfo, sta,
						&wbd_ret);
					if (wbd_ret == BSDE_WBD_OK) {
						sta->wbd_sta_status |= BSD_WBD_STA_WEAK;
						sta->wbd_sta_status |= BSD_WBD_STA_WEAK_PENDING;
					}
				}
			} else {
				/* Not weak. So check if its already weak */
				if ((sta->wbd_sta_status & BSD_WBD_STA_WEAK)) {
					/* Inform WBD */
					ret = bsd_wbd_send_weak_cancel_req(info, bssinfo, sta,
						&wbd_ret);
					if (wbd_ret == BSDE_WBD_OK) {
						sta->wbd_sta_status &= ~(BSD_WBD_STA_WEAK);
						sta->wbd_sta_status &= ~(BSD_WBD_STA_WEAK_PENDING);
					}
				}
			}
nextsta:
			sta = sta->next;
		}
nextbss:
		cur = cur->next;
	}

end:
	BSD_EXIT();
}
