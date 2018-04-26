/*
 * bsd deamon (Linux)
 *
 * $Copyright Broadcom Corporation$
 *
 * $Id: bsd_main.c $
 */
#include "bsd.h"
#include <sys/stat.h>

/* policy extension flag usage */
typedef struct bsd_flag_description_ {
	uint32	flag;
	char	*descr;
} bsd_flag_description_t;

static bsd_flag_description_t bsd_streering_flag_descr[] = {
	{BSD_STEERING_POLICY_FLAG_RULE, "BSD_STEERING_POLICY_FLAG_RULE"},
	{BSD_STEERING_POLICY_FLAG_RSSI, "BSD_STEERING_POLICY_FLAG_RSSI"},
	{BSD_STEERING_POLICY_FLAG_VHT, "BSD_STEERING_POLICY_FLAG_VHT"},
	{BSD_STEERING_POLICY_FLAG_NON_VHT, "BSD_STEERING_POLICY_FLAG_NON_VHT"},
	{BSD_STEERING_POLICY_FLAG_NEXT_RF, "BSD_STEERING_POLICY_FLAG_NEXT_RF"},
	{BSD_STEERING_POLICY_FLAG_PHYRATE, "BSD_STEERING_POLICY_FLAG_PHYRATE"},
	{BSD_STEERING_POLICY_FLAG_LOAD_BAL, "BSD_STEERING_POLICY_FLAG_LOAD_BAL"},
	{BSD_STEERING_POLICY_FLAG_STA_NUM_BAL, "BSD_STEERING_POLICY_FLAG_STA_NUM_BAL"},
	{BSD_STEERING_POLICY_FLAG_CHAN_OVERSUB, "BSD_STEERING_POLICY_FLAG_CHAN_OVERSUB"},
	{0, ""},
};

static bsd_flag_description_t bsd_sta_select_flag_descr[] = {
	{BSD_STA_SELECT_POLICY_FLAG_RULE, "BSD_STA_SELECT_POLICY_FLAG_RULE"},
	{BSD_STA_SELECT_POLICY_FLAG_RSSI, "BSD_STA_SELECT_POLICY_FLAG_RSSI"},
	{BSD_STA_SELECT_POLICY_FLAG_VHT, "BSD_STA_SELECT_POLICY_FLAG_VHT"},
	{BSD_STA_SELECT_POLICY_FLAG_NON_VHT, "BSD_STA_SELECT_POLICY_FLAG_NON_VHT"},
	{BSD_STA_SELECT_POLICY_FLAG_NEXT_RF, "BSD_STA_SELECT_POLICY_FLAG_NEXT_RF"},
	{BSD_STA_SELECT_POLICY_FLAG_PHYRATE, "BSD_STA_SELECT_POLICY_FLAG_PHYRATE"},
	{BSD_STA_SELECT_POLICY_FLAG_LOAD_BAL, "BSD_STA_SELECT_POLICY_FLAG_LOAD_BAL"},
	{BSD_STA_SELECT_POLICY_FLAG_SINGLEBAND, "BSD_STA_SELECT_POLICY_FLAG_SINGLEBAND"},
	{BSD_STA_SELECT_POLICY_FLAG_DUALBAND, "BSD_STA_SELECT_POLICY_FLAG_DUALBAND"},
	{BSD_STA_SELECT_POLICY_FLAG_ACTIVE_STA, "BSD_STA_SELECT_POLICY_FLAG_ACTIVE_STA"},
	{0, ""},
};

static bsd_flag_description_t bsd_if_qualify_flag_descr[] = {
	{BSD_QUALIFY_POLICY_FLAG_RULE, "BSD_QUALIFY_POLICY_FLAG_RULE"},
	{BSD_QUALIFY_POLICY_FLAG_VHT, "BSD_QUALIFY_POLICY_FLAG_VHT"},
	{BSD_QUALIFY_POLICY_FLAG_NON_VHT, "BSD_QUALIFY_POLICY_FLAG_NON_VHT"},
	{BSD_QUALIFY_POLICY_FLAG_PHYRATE, "BSD_QUALIFY_POLICY_FLAG_PHYRATE"},
	{BSD_QUALIFY_POLICY_FLAG_LOAD_BAL, "BSD_QUALIFY_POLICY_FLAG_LOAD_BAL"},
	{BSD_QUALIFY_POLICY_FLAG_STA_BAL, "BSD_QUALIFY_POLICY_FLAG_STA_BAL"},
	{0, ""},
};

static bsd_flag_description_t bsd_debug_flag_descr[] = {
	{BSD_DEBUG_ERROR, "BSD_DEBUG_ERROR"},
	{BSD_DEBUG_WARNING, "BSD_DEBUG_WARNING"},
	{BSD_DEBUG_INFO, "BSD_DEBUG_INFO"},
	{BSD_DEBUG_TO, "BSD_DEBUG_TO"},
	{BSD_DEBUG_STEER, "BSD_DEBUG_STEER"},
	{BSD_DEBUG_EVENT, "BSD_DEBUG_EVENT"},
	{BSD_DEBUG_HISTO, "BSD_DEBUG_HISTO"},
	{BSD_DEBUG_CCA, "BSD_DEBUG_CCA"},
	{BSD_DEBUG_AT, "BSD_DEBUG_AT"},
	{BSD_DEBUG_RPC, "BSD_DEBUG_RPC"},
	{BSD_DEBUG_RPCD, "BSD_DEBUG_RPCD"},
	{BSD_DEBUG_RPCEVT, "BSD_DEBUG_RPCEVT"},
	{BSD_DEBUG_MULTI_RF, "BSD_DEBUG_MULTI_RF"},
	{BSD_DEBUG_BOUNCE, "BSD_DEBUG_BOUNCE"},
	{BSD_DEBUG_DUMP, "BSD_DEBUG_DUMP"},
	{BSD_DEBUG_PROBE, "BSD_DEBUG_PROBE"},
	{BSD_DEBUG_ALL, "BSD_DEBUG_ALL"},
	{0, ""},
};

static void bsd_describe_flag(bsd_flag_description_t *descr)
{
	while (descr->flag != 0) {
		printf("%35s\t0x%08x\n", descr->descr, descr->flag);
		descr++;
	}
}

static void bsd_usage(void)
{
	printf("wlx[.y]_bsd_steering_policy=<bw util percentage> <sample period> "
		"<consecutive sample count> <rssi  threshold> "
		"<phy rate threshold> <extension flag>\n");

	bsd_describe_flag(bsd_streering_flag_descr);

	printf("\nwlx[.y]_bsd_sta_select_policy=<idle_rate> <rssi> <phy rate> "
		"<wprio> <wrssi> <wphy_rate> <wtx_failures> <wtx_rate> <wrx_rate> "
		"<extension_flag>\n");
	bsd_describe_flag(bsd_sta_select_flag_descr);

	printf("\nwlx[.y]_bsd_if_qualify_policy=<bw util percentage> "
		"<extension_flag>\n");
	bsd_describe_flag(bsd_if_qualify_flag_descr);

	printf("\nband steering debug flags\n");
	bsd_describe_flag(bsd_debug_flag_descr);

	printf("\n bsd command line options:\n");
	printf("-f\n");
	printf("-F keep bsd on the foreground\n");
	printf("-i show bsd config info\n");
	printf("-s show bsd sta info\n");
	printf("-l show bsd steer log\n");
	printf("-h\n");
	printf("-H this help usage\n");
	printf("\n");

}

bsd_info_t *bsd_info;
bsd_intf_info_t *bsd_intf_info;
int bsd_msglevel = BSD_DEBUG_ERROR;

static bsd_info_t *bsd_info_alloc(void)
{
	bsd_info_t *info;

	BSD_ENTER();

	info = (bsd_info_t *)malloc(sizeof(bsd_info_t));
	if (info == NULL) {
		BSD_PRINT("malloc fails\n");
	}
	else {
		memset(info, 0, sizeof(bsd_info_t));
		BSD_INFO("info=%p\n", info);
	}

	BSD_EXIT();
	return info;
}

static int
bsd_init(bsd_info_t *info)
{
	int err = BSD_FAIL;
	char *str, *endptr = NULL;
	char tmp[16];

	BSD_ENTER();

	err = bsd_intf_info_init(info);
	if (err != BSD_OK) {
		return err;
	}

	info->version = BSD_VERSION;
	info->event_fd = BSD_DFLT_FD;
	info->event_fd2 = BSD_DFLT_FD;
	info->rpc_listenfd  = BSD_DFLT_FD;
	info->rpc_eventfd = BSD_DFLT_FD;
	info->rpc_ioctlfd = BSD_DFLT_FD;
	info->poll_interval = BSD_POLL_INTERVAL;
	info->mode = BSD_MODE_STEER;
	info->role = BSD_ROLE_STANDALONE;
	info->status_poll = BSD_STATUS_POLL_INTV;
	info->counter_poll = BSD_COUNTER_POLL_INTV;
	info->idle_rate = 10;

	if ((str = nvram_get("bsd_role"))) {
		info->role = (uint8)strtol(str, &endptr, 0);
		if (info->role >= BSD_ROLE_MAX) {
			BSD_ERROR("Err: bsd_role[%s] default to Standalone.\n", str);
			info->role = BSD_ROLE_STANDALONE;
			sprintf(tmp, "%d", info->role);
			nvram_set("bsd_role", tmp);
		}
#ifdef BCM_WBD
		/* If the BSD role is none and WBD is enabled make it as standalone */
		if ((info->role == BSD_ROLE_NONE) &&
			(info->enable_flag & BSD_FLAG_WBD_ENABLED)) {
			info->role = BSD_ROLE_STANDALONE;
			BSD_WBD("info->role[%d]\n", info->role);
		}
#endif /* BCM_WBD */
	}

	if ((str = nvram_get("bsd_helper"))) {
		BSDSTRNCPY(info->helper_addr, str, sizeof(info->helper_addr) - 1);
	}
	else {
		strcpy(info->helper_addr, BSD_DEFT_HELPER_ADDR);
		nvram_set("bsd_helper", BSD_DEFT_HELPER_ADDR);
	}

	info->hport = HELPER_PORT;
	if ((str = nvram_get("bsd_hport"))) {
		info->hport = (uint16)strtol(str, &endptr, 0);
	} else {
		sprintf(tmp, "%d", info->hport);
		nvram_set("bsd_hport", tmp);
	}

	if ((str = nvram_get("bsd_primary"))) {
		BSDSTRNCPY(info->primary_addr, str, sizeof(info->primary_addr) - 1);
	}
	else {
		strcpy(info->primary_addr, BSD_DEFT_PRIMARY_ADDR);
		nvram_set("bsd_primary", BSD_DEFT_PRIMARY_ADDR);
	}

	info->pport = PRIMARY_PORT;
	if ((str = nvram_get("bsd_pport"))) {
		info->pport = (uint16)strtol(str, &endptr, 0);
	}
	else {
		sprintf(tmp, "%d", info->pport);
		nvram_set("bsd_pport", tmp);
	}

	BSD_INFO("role:%d helper:%s[%d] primary:%s[%d]\n",
		info->role, info->helper_addr, info->hport,
		info->primary_addr, info->pport);

	info->scheme = BSD_SCHEME;
	if ((str = nvram_get("bsd_scheme"))) {
		info->scheme = (uint8)strtol(str, &endptr, 0);
		if (info->scheme >= bsd_get_max_scheme(info))
			info->scheme = BSD_SCHEME;
	}
	BSD_INFO("scheme:%d\n", info->scheme);

	err = bsd_info_init(info);
	if (err == BSD_OK) {
		bsd_retrieve_config(info);
		err = bsd_open_eventfd(info);
		if (err == BSD_OK)
			err = bsd_open_rpc_eventfd(info);
	}

	BSD_EXIT();
	return err;
}

static void
bsd_cleanup(bsd_info_t*info)
{
	if (info) {
		bsd_close_eventfd(info);
		bsd_close_rpc_eventfd(info);
		bsd_bssinfo_cleanup(info);
		bsd_cleanup_sta_bounce_table(info);

		if (info->intf_info != NULL) {
			free(info->intf_info);
		}

#ifdef BCM_WBD
		/* Cleanup WBD info */
		bsd_cleanup_wbd(info->wbd_info);
#endif /* BCM_WBD */

		free(info);
	}
}

static void
bsd_watchdog(bsd_info_t*info, uint ticks)
{

	BSD_ENTER();

	BSD_TO("\nticks[%d] [%lu]\n", ticks, (unsigned long)time(NULL));

	if ((info->enable_flag & BSD_FLAG_ENABLED) &&
		(info->role != BSD_ROLE_PRIMARY) &&
		(info->role != BSD_ROLE_STANDALONE)) {
		BSD_TO("no Watchdog operation fro helper...\n");
		BSD_EXIT();
		return;
	}

	if ((info->counter_poll != 0) && (ticks % info->counter_poll == 1)) {
		BSD_TO("bsd_update_counters [%d] ...\n", info->counter_poll);
		bsd_update_stb_info(info);
	}

	if ((info->status_poll != 0) && (ticks % info->status_poll == 1)) {
		BSD_TO("bsd_update_stainfo [%d] ...\n", info->status_poll);
		bsd_update_stainfo(info);
	}

	if ((info->status_poll != 0) && (ticks % info->status_poll == 0)) {
		bsd_update_sta_bounce_table(info);
	}

	bsd_update_cca_stats(info);

	/* Only if BSD is enabled */
	if (info->enable_flag & BSD_FLAG_ENABLED) {
		/* use same poll interval with stainfo */
		if ((info->status_poll != 0) && (ticks % info->status_poll == 1)) {
			BSD_TO("bsd_check_steer [%d] ...\n", info->status_poll);
			bsd_check_steer(info);
		}
	}

#ifdef BCM_WBD
	/* Inform weak STAs to WBD only if WBD is enabled */
	if (info->enable_flag & BSD_FLAG_WBD_ENABLED) {
		/* use same poll interval with stainfo */
		if ((info->status_poll != 0) && (ticks % info->status_poll == 1)) {
			bsd_wbd_check_weak_sta(info);
		}
	}
#endif /* BCM_WBD */

	if ((info->probe_timeout != 0) && (ticks % info->probe_timeout == 0)) {
		BSD_TO("bsd_timeout_prbsta [%d] ...\n", info->probe_timeout);
		bsd_timeout_prbsta(info);
	}

	/* Only if BSD is enabled */
	if (info->enable_flag & BSD_FLAG_ENABLED) {
		if ((info->maclist_timeout != 0) && (ticks % info->maclist_timeout == 0)) {
			BSD_TO("bsd_timeout_maclist [%d] ...\n", info->maclist_timeout);
			bsd_timeout_maclist(info);
		}
	}

	if ((info->sta_timeout != 0) &&(ticks % info->sta_timeout == 0)) {
		BSD_TO("bsd_timeout_sta [%d] ...\n", info->sta_timeout);
		bsd_timeout_sta(info);
	}

	BSD_EXIT();
}

static void bsd_hdlr(int sig)
{
	bsd_info->mode = BSD_MODE_DISABLE;
	return;
}

static void bsd_info_hdlr(int sig)
{
	bsd_dump_config_info(bsd_info);
	return;
}

static void bsd_log_hdlr(int sig)
{
	bsd_steering_record_display();
	return;
}

static void bsd_sta_hdlr(int sig)
{
	bsd_dump_sta_info(bsd_info);
	return;
}

/* service main entry */
int main(int argc, char *argv[])
{
	int err = BSD_OK;
	struct timeval tv;
	char *val;
	int role, flag = 0;
	int c;
	bool foreground = FALSE;
	pid_t pid;
	int sig;
	char filename[128];
	char cmd[128];
	struct stat buffer;
	int wait_time = 0;

	if (argc > 1) {
		while ((c = getopt(argc, argv, "hHfFils")) != -1) {
			switch (c) {
				case 'f':
				case 'F':
					foreground = TRUE;
					break;
				case 'i':
				case 'l':
				case 's':
					/* for both bsd -i/-l/-s (info/log/sta) */
					if ((pid = get_pid_by_name("/usr/sbin/bsd")) <= 0) {
						if ((pid = get_pid_by_name("/bin/bsd")) <= 0) {
							if ((pid = get_pid_by_name("bsd")) <= 0) {
								printf("BSD is not running\n");
								return BSD_FAIL;
							}
						}
					}

					if (c == 'i') {
						snprintf(filename, sizeof(filename), "%s",
							BSD_OUTPUT_FILE_INFO);
						sig = SIGUSR1;
					}
					else if (c == 's') {
						snprintf(filename, sizeof(filename), "%s",
							BSD_OUTPUT_FILE_STA);
						sig = SIGHUP;
					}
					else {
						snprintf(filename, sizeof(filename), "%s",
							BSD_OUTPUT_FILE_LOG);
						sig = SIGUSR2;
					}

					unlink(filename);
					kill(pid, sig);

					while (1) {
						usleep(BSD_OUTPUT_FILE_INTERVAL);
						if (stat(filename, &buffer) == 0) {
							snprintf(cmd, sizeof(cmd), "cat %s",
								filename);
							system(cmd);
							return BSD_OK;
						}
						wait_time += BSD_OUTPUT_FILE_INTERVAL;
						if (wait_time >= BSD_OUTPUT_FILE_TIMEOUT)
							break;
					}

					printf("BSD: info not ready\n");
					return BSD_FAIL;
				case 'h':
				case 'H':
					bsd_usage();
					break;
				default:
					printf("%s invalid option\n", argv[0]);
			}
		}

		if (foreground == FALSE) {
			exit(0);
		}
	}

#ifdef BCM_WBD
	/* Get WBD mode */
	char *wbd_val = nvram_safe_get(BSD_WBD_NVRAM_MODE);
	int wbd_mode = strtoul(wbd_val, NULL, 0);
	if (!BSD_WBD_DISABLED(wbd_mode))
		flag |= BSD_FLAG_WBD_ENABLED;
	else
		printf("WBD is not enabled: %s=%d\n", wbd_val, wbd_mode);
#endif /* BCM_WBD */

	val = nvram_safe_get("bsd_role");
	role = strtoul(val, NULL, 0);
	if ((role != BSD_ROLE_PRIMARY) &&
		(role != BSD_ROLE_HELPER) &&
		(role != BSD_ROLE_STANDALONE)) {
		printf("BSD is not enabled: %s=%d\n", val, role);
	} else {
		flag |= BSD_FLAG_ENABLED;
	}

	if (((flag & BSD_FLAG_ENABLED) == 0) &&
		((flag & BSD_FLAG_WBD_ENABLED) == 0)) {
		printf("Both BSD and WBD is Disabled. flag=%d\n", flag);
		goto done;
	}

	val = nvram_safe_get("bsd_msglevel");
	if (strcmp(val, ""))
		bsd_msglevel = strtoul(val, NULL, 0);

	BSD_INFO("bsd start...\n");

#if !defined(DEBUG)
	if (foreground == FALSE) {
		if (daemon(1, 1) == -1) {
			BSD_ERROR("err from daemonize.\n");
			goto done;
		}
	}
#endif

	if ((bsd_info = bsd_info_alloc()) == NULL) {
		printf("BSD alloc fails. Aborting...\n");
		goto done;
	}

	/* Initialize the flag */
	bsd_info->enable_flag = flag;

#ifdef BCM_WBD
	bsd_wbd_set_ifnames(bsd_info);
#endif /* BCM_WBD */

	if (bsd_init(bsd_info) != BSD_OK) {
		printf("BSD Aborting...\n");
		goto done;
	}

#ifdef BCM_WBD
	if (bsd_wbd_init(bsd_info) != BSD_OK) {
		printf("WBD init failed BSD Aborting...\n");
		goto done;
	}
#endif /* BCM_WBD */

	tv.tv_sec = bsd_info->poll_interval;
	tv.tv_usec = 0;

	signal(SIGTERM, bsd_hdlr);
	signal(SIGUSR1, bsd_info_hdlr);
	signal(SIGUSR2, bsd_log_hdlr);
	signal(SIGHUP, bsd_sta_hdlr);

	while (bsd_info->mode != BSD_MODE_DISABLE) {

		if (tv.tv_sec == 0 && tv.tv_usec == 0) {
			bsd_info->ticks ++;
			tv.tv_sec = bsd_info->poll_interval;
			tv.tv_usec = 0;
			BSD_INFO("ticks: %d\n", bsd_info->ticks);

			bsd_watchdog(bsd_info, bsd_info->ticks);

			val = nvram_safe_get("bsd_msglevel");
			if (strcmp(val, ""))
				bsd_msglevel = strtoul(val, NULL, 0);
		}
		bsd_proc_socket(bsd_info, &tv);
	}

done:
	bsd_cleanup(bsd_info);
	return err;
}
