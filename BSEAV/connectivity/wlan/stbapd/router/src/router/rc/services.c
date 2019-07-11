/*
 * Miscellaneous services
 *
 * Copyright (C) 2018, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: services.c 735675 2017-12-12 01:19:19Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>
#include <pmon.h>
#if defined(LINUX_2_6_36)
#include <mntent.h>
#include <fcntl.h>
#endif /* LINUX_2_6_36 */

#define MAX_NVPARSE	16

#ifdef BCMDBG
#include <assert.h>
#else
#define assert(a)
#endif

#ifdef TARGETENV_android
#define DHCP_CONF "/data/tmp/udhcpd%d.conf"
#define DHCP_SETTING_PIDFILE "pidfile /data/var/run/udhcpd%d.pid\n"
#define DHCP_SETTING_LEASEFILE "lease_file /data/tmp/udhcpd%d.leases\n"

#define DNS_RESOLV_CONF "/data/tmp/resolv.conf"
#define DNS_CMD "/vendor/bin/dnsmasq -h -n %s -r /data/tmp/resolv.conf %s&"

#define HTTPD_WWW_PATH "/vendor/www"
#define HTTPD_PATH "/vendor/bin/httpd /data/tmp/httpd.conf"

#define WPS_MONITOR_PID "/data/tmp/wps_monitor.pid"
#define WPS_MONITOR_PATH "/vendor/bin/wps_monitor"

#define HSPOTAP_PATH "/vendor/bin/hspotap"

#define EAPD_PATH "/vendor/bin/eapd"

#define ACSD_PATH "/vendor/bin/acsd"

#define TOAD_PATH "/vendor/bin/toad"

#define BSD_PATH "/vendor/bin/bsd"

#define APPEVENTD_PATH "/vendor/bin/appeventd"

#define SSD_PATH "/vendor/bin/ssd"

#define EVENTD_PATH "/vendor/bin/eventd"

#define DHD_MONITOR_PATH "/vendor/bin/dhd_monitor"
#else
#define DHCP_CONF "/tmp/udhcpd%d.conf"
#define DHCP_SETTING_PIDFILE "pidfile /var/run/udhcpd%d.pid\n"
#define DHCP_SETTING_LEASEFILE "lease_file /tmp/udhcpd%d.leases\n"

#define DNS_RESOLV_CONF "/tmp/resolv.conf"
#define DNS_CMD "/usr/sbin/dnsmasq -h -n %s -r /tmp/resolv.conf %s&"

#define HTTPD_WWW_PATH "/www"
#define HTTPD_PATH "/usr/sbin/httpd /tmp/httpd.conf"

#define WPS_MONITOR_PID "/tmp/wps_monitor.pid"
#define WPS_MONITOR_PATH "/bin/wps_monitor"

#define HSPOTAP_PATH "/bin/hspotap"

#define EAPD_PATH "/bin/eapd"

#define ACSD_PATH "/usr/sbin/acsd"

#define TOAD_PATH "/usr/sbin/toad"

#define BSD_PATH "/usr/sbin/bsd"

#define APPEVENTD_PATH "/usr/sbin/appeventd"

#define SSD_PATH "/usr/sbin/ssd"

#define EVENTD_PATH "/usr/sbin/eventd"

#define DHD_MONITOR_PATH "/usr/sbin/dhd_monitor"
#endif


static char
*make_var(char *prefix, int index, char *name)
{
	static char buf[100];

	assert(prefix);
	assert(name);

	if (index)
		snprintf(buf, sizeof(buf), "%s%d%s", prefix, index, name);
	else
		snprintf(buf, sizeof(buf), "%s%s", prefix, name);
	return buf;
}
#if defined(__CONFIG_NAT__) || defined(__CONFIG_STBAP__)
int
start_dhcpd(void)
{
	FILE *fp;
	char name[100];
	char word[32], *next;
	char dhcp_conf_file[128];
	char dhcp_lease_file[128];
	char dhcp_ifnames[128] = "";
	int index, offset = 0;
	char tmp[20];
	int i = 0;
	char *lan_ifname = NULL;

#ifndef __CONFIG_STBAP__
	if (nvram_match("router_disable", "1"))
		return 0;
#endif /* !__CONFIG_STBAP__ */

	/* Setup individual dhcp severs for the bridge and the every unbridged interface */
	for (i = 0; i < MAX_NO_BRIDGE; i++) {
		if (!i)
			snprintf(tmp, sizeof(tmp), "lan_ifname");
		else
			snprintf(tmp, sizeof(tmp), "lan%x_ifname", i);

		lan_ifname = nvram_safe_get(tmp);

		if (!strcmp(lan_ifname, ""))
			continue;

		offset = snprintf(dhcp_ifnames + offset, sizeof(dhcp_ifnames) - offset,
			"%s ", lan_ifname);
	}

	index = 0;
	foreach(word, dhcp_ifnames, next) {

		if (strstr(word, "br0"))
			index = 0;
		else
			index = 1;

		if (nvram_invmatch(make_var("lan", index, "_proto"), "dhcp"))
			continue;

		dprintf("%s %s %s %s\n",
			nvram_safe_get(make_var("lan", index, "_ifname")),
			nvram_safe_get(make_var("dhcp", index, "_start")),
			nvram_safe_get(make_var("dhcp", index, "_end")),
			nvram_safe_get(make_var("lan", index, "_lease")));

		/* Touch leases file */
		sprintf(dhcp_lease_file, "/tmp/udhcpd%d.leases", index);
		if (!(fp = fopen(dhcp_lease_file, "a"))) {
			perror(dhcp_lease_file);
			return errno;
		}
		fclose(fp);

		/* Write configuration file based on current information */
		sprintf(dhcp_conf_file, DHCP_CONF, index);
		if (!(fp = fopen(dhcp_conf_file, "w"))) {
			perror(dhcp_conf_file);
			return errno;
		}
		fprintf(fp, DHCP_SETTING_PIDFILE, index);
		fprintf(fp, "start %s\n", nvram_safe_get(make_var("dhcp", index, "_start")));
		fprintf(fp, "end %s\n", nvram_safe_get(make_var("dhcp", index, "_end")));
		fprintf(fp, "interface %s\n", word);
		fprintf(fp, "remaining yes\n");
		fprintf(fp, DHCP_SETTING_LEASEFILE, index);
		fprintf(fp, "option subnet %s\n",
			nvram_safe_get(make_var("lan", index, "_netmask")));
#ifdef __CONFIG_STBAP__
		fprintf(fp, "option router %s\n",
		nvram_safe_get(make_var("lan", index, "_gateway")));
		fprintf(fp, "option dns %s\n", nvram_safe_get(make_var("lan", index, "_gateway")));
#else
		fprintf(fp, "option router %s\n",
			nvram_safe_get(make_var("lan", index, "_ipaddr")));
		fprintf(fp, "option dns %s\n", nvram_safe_get(make_var("lan", index, "_ipaddr")));
#endif
		fprintf(fp, "option lease %s\n", nvram_safe_get(make_var("lan", index, "_lease")));
		snprintf(name, sizeof(name), "%s_wins",
			nvram_safe_get(make_var("dhcp", index, "_wins")));
		if (nvram_invmatch(name, ""))
			fprintf(fp, "option wins %s\n", nvram_get(name));
		snprintf(name, sizeof(name), "%s_domain",
			nvram_safe_get(make_var("dhcp", index, "_domain")));
		if (nvram_invmatch(name, ""))
			fprintf(fp, "option domain %s\n", nvram_get(name));
		fclose(fp);

		eval("udhcpd", dhcp_conf_file);
		index++;
	}
	dprintf("done\n");
	return 0;
}

int
stop_dhcpd(void)
{
	char sigusr1[] = "-XX";
	int ret;

/*
* Process udhcpd handles two signals - SIGTERM and SIGUSR1
*
*  - SIGUSR1 saves all leases in /tmp/udhcpd.leases
*  - SIGTERM causes the process to be killed
*
* The SIGUSR1+SIGTERM behavior is what we like so that all current client
* leases will be honorred when the dhcpd restarts and all clients can extend
* their leases and continue their current IP addresses. Otherwise clients
* would get NAK'd when they try to extend/rebind their leases and they
* would have to release current IP and to request a new one which causes
* a no-IP gap in between.
*/
	sprintf(sigusr1, "-%d", SIGUSR1);
	eval("killall", sigusr1, "udhcpd");
	ret = eval("killall", "udhcpd");

	dprintf("done\n");
	return ret;
}

int
start_dns(void)
{
	int ret;
	FILE *fp;
	char dns_ifnames[64] = "";
	char tmp[20];
	int i = 0, offset = 0;
	char *lan_ifname = NULL;
	char dns_cmd[384];
	char if_hostnames[128] = "";

	if (nvram_match("router_disable", "1"))
		return 0;

	/* Create resolv.conf with empty nameserver list */
	if (!(fp = fopen(DNS_RESOLV_CONF, "w"))) {
		perror(DNS_RESOLV_CONF);
		return errno;
	}
	fclose(fp);

	/* Setup interface list for the dns relay */
	for (i = 0; i < MAX_NO_BRIDGE; i++) {
		if (!i)
			snprintf(tmp, sizeof(tmp), "lan_ifname");
		else
			snprintf(tmp, sizeof(tmp), "lan%x_ifname", i);

		lan_ifname = nvram_safe_get(tmp);

		if (!strcmp(lan_ifname, ""))
			continue;

		offset = snprintf(dns_ifnames + offset, sizeof(dns_ifnames) - offset,
			"-i %s ", lan_ifname);
	}

	/* Start the dns relay */
	sprintf(dns_cmd, DNS_CMD,
		dns_ifnames, if_hostnames);
	ret = system(dns_cmd);

	dprintf("done\n");
	return ret;
}

int
stop_dns(void)
{
	int ret = eval("killall", "dnsmasq");

	/* Remove resolv.conf */
	unlink(DNS_RESOLV_CONF);

	dprintf("done\n");
	return ret;
}
#endif	/* __CONFIG_NAT__ */

int
start_nfc(void)
{
	int ret = 0;
#ifdef __CONFIG_NFC__
	char nsa_cmd[255];
	char *nsa_msglevel = NULL;

	/* For PF#1 debug only + */
	if (nvram_match("nsa_only", "1")) {
		dprintf("nsa_only mode, ignore nsa_server!\n");
		return 0;
	}
	/* For PF#1 debug only - */

#ifdef BCMDBG
	/*
	 * --all=#            set trace level to # for all layers
	 * --app=#          set APPL trace level to #
	 *
	 * The possible levels are the following:
	 *    NONE       0           No trace messages at all
	 *    ERROR      1           Error condition trace messages
	 *    WARNING  2           Warning condition trace messages
	 *    API          3           API traces
	 *    EVENT      4           Debug messages for events
	 *    DEBUG      5           Full debug messages
	 */
	nsa_msglevel = nvram_get("nsa_msglevel");
#endif /* BCMDBG */

	sprintf(nsa_cmd, "/bin/nsa_server -d /dev/ttyS1 -u /tmp/ --all=%s&",
		nsa_msglevel ? nsa_msglevel : "0");
	eval("sh", "-c", nsa_cmd);
#endif /* __CONFIG_NFC__ */

	return ret;
}

int
stop_nfc(void)
{
	int ret = 0;
#ifdef __CONFIG_NFC__

	/* For PF#1 debug only + */
	if (nvram_match("nsa_only", "1")) {
		dprintf("nsa_only mode, ignore killall nsa_server!\n");
		return 0;
	}
	/* For PF#1 debug only - */

	ret = eval("killall", "nsa_server");
#endif /* __CONFIG_NFC__ */

	return ret;
}

/*
*/

int
start_httpd(void)
{
	int ret;

	chdir(HTTPD_WWW_PATH);
	ret = system(HTTPD_PATH);
	chdir("/");

	dprintf("done\n");
	return ret;
}

int
stop_httpd(void)
{
	int ret = eval("killall", "httpd");

	dprintf("done\n");
	return ret;
}
#ifdef	__CONFIG_VISUALIZATION__
static int
start_visualization_tool(void)
{
	int ret;

	ret = eval("vis-dcon");
	ret = eval("vis-datacollector");

	dprintf("done\n");

	return ret;
}

static int
stop_visualization_tool(void)
{
	int ret;

	ret = eval("killall", "vis-dcon");
	ret = eval("killall", "vis-datacollector");

	dprintf("done\n");
	return ret;
}
#endif /* __CONFIG_VISUALIZATION__ */

#ifdef  __CONFIG_WBD__
static int
start_wbd(void)
{
	int ret;

	ret = eval("wbd_master");
	ret = eval("wbd_slave");

	dprintf("done\n");
	return ret;
}

static int
stop_wbd(void)
{
	int ret;

	ret = eval("killall", "wbd_master");
	ret = eval("killall", "wbd_slave");
	/* Intentional Delay to make sure wbd_master running
	 * on another machine gets to know of leave message
	 * from wbd_slave. It is observed that stop_ipv6 routine
	 * disables forwarding which prevents connect_to_server
	 * call from wbd_slave to succeed.
	 */
	sleep(1);
	dprintf("done\n");
	return ret;
}
#endif /* __CONFIG_WBD__ */

#ifdef PLC
static int
start_gigled(void)
{
	int ret;

	chdir("/tmp");
	ret = eval("/usr/sbin/gigled");
	chdir("/");

	dprintf("done\n");
	return ret;
}

static int
stop_gigled(void)
{
	int ret = eval("killall", "gigled");

	dprintf("done\n");
	return ret;
}

static int
start_plcnvm(void)
{
	int ret;

	chdir("/tmp");
	ret = eval("/usr/sbin/plcnvm");
	chdir("/");

	dprintf("done\n");
	return ret;
}

static int
stop_plcnvm(void)
{
	int ret = eval("killall", "plcnvm");

	dprintf("done\n");
	return ret;
}

static int
start_plcboot(void)
{
	int ret;

	chdir("/tmp");
	ret = eval("/usr/sbin/plcboot");
	chdir("/");

	dprintf("done\n");
	return ret;
}

static int
stop_plcboot(void)
{
	int ret = eval("killall", "plcboot");

	dprintf("done\n");
	return ret;
}
#endif /* PLC */

#ifdef __CONFIG_NAT__
#ifdef __CONFIG_UPNP__
int
start_upnp(void)
{
	char *wan_ifname;
	int ret;
	char var[100], prefix[] = "wanXXXXXXXXXX_";

	if (!nvram_invmatch("upnp_enable", "0"))
		return 0;

	ret = eval("killall", "-SIGUSR1", "upnp");
	if (ret != 0) {
		snprintf(prefix, sizeof(prefix), "wan%d_", wan_primary_ifunit());
		if (nvram_match(strcat_r(prefix, "proto", var), "pppoe"))
			wan_ifname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", var));
		else
			wan_ifname = nvram_safe_get(strcat_r(prefix, "ifname", var));

		ret = eval("upnp", "-D", "-W", wan_ifname);

	}
	dprintf("done\n");
	return ret;
}

int
stop_upnp(void)
{
	int ret = 0;

	if (nvram_match("upnp_enable", "0"))
	    ret = eval("killall", "upnp");

	dprintf("done\n");
	return ret;
}
#endif /* __CONFIG_UPNP__ */

#ifdef	__CONFIG_IGD__
int
start_igd(void)
{
	int ret = 0;

	if (nvram_match("upnp_enable", "1")) {
		ret = eval("igd", "-D");
	}

	return ret;
}

int
stop_igd(void)
{
	int ret = 0;

	ret = eval("killall", "igd");

	return ret;
}
#endif	/* __CONFIG_IGD__ */
#endif	/* __CONFIG_NAT__ */

int
start_nas(void)
{
	int ret = system("nas");

	return ret;
}

int
stop_nas(void)
{
	int ret;

	ret = eval("killall", "nas");
	return ret;
}

#ifdef __CONFIG_WAPI__
int
start_wapid(void)
{
	int ret = eval("wapid");

	return ret;
}

int
stop_wapid(void)
{
	int ret = eval("killall", "wapid");

	return ret;
}
#endif /* __CONFIG_WAPI__ */

#ifdef __CONFIG_WAPI_IAS__
int
start_ias(void)
{
	char *ias_argv[] = {"ias", "-F", "/etc/AS.conf", "-D", NULL};
	pid_t pid;

	if (nvram_match("as_mode", "enabled")) {
		_eval(ias_argv, NULL, 0, &pid);
	}

	return 0;
}

int
stop_ias(void)
{
	int ret = 0;

	if (!nvram_match("as_mode", "enabled")) {
		/* Need add signal handler in as */
		ret = eval("killall", "ias");
	}

	return ret;
}
#endif /* __CONFIG_WAPI_IAS__ */

int
start_ntpc(void)
{
	char *servers = nvram_safe_get("ntp_server");

	if (strlen(servers)) {
		char *nas_argv[] = {"ntpclient", "-h", servers, "-i", "3", "-l", "-s", NULL};
		pid_t pid;

		_eval(nas_argv, NULL, 0, &pid);
	}

	dprintf("done\n");
	return 0;
}

int
stop_ntpc(void)
{
	int ret = eval("killall", "ntpclient");

	dprintf("done\n");
	return ret;
}

int
start_telnet(void)
{
	char tmp[20];
	int i = 0;
	int ret = 0;
	char *lan_ifname = NULL;


	for (i = 0; i < MAX_NO_BRIDGE; i++) {
		if (!i)
			snprintf(tmp, sizeof(tmp), "lan_ifname");
		else
			snprintf(tmp, sizeof(tmp), "lan%x_ifname", i);

		lan_ifname = nvram_safe_get(tmp);

		if (!strcmp(lan_ifname, ""))
			continue;

		ret = eval("utelnetd", "-d", "-i", lan_ifname);
	}

	return ret;
}

int
stop_telnet(void)
{
	int ret;
	ret = eval("killall", "utelnetd");
	return ret;
}

int
stop_wps(void)
{
	int ret = 0;
#ifdef __CONFIG_WPS__
	FILE *fp = NULL;
	char saved_pid[32];
	int i, wait_time = 3;
	pid_t pid;

	if (((fp = fopen(WPS_MONITOR_PID, "r")) != NULL) &&
	    (fgets(saved_pid, sizeof(saved_pid), fp) != NULL)) {
		/* remove new line first */
		for (i = 0; i < sizeof(saved_pid); i++) {
			if (saved_pid[i] == '\n')
				saved_pid[i] = '\0';
		}
		saved_pid[sizeof(saved_pid) - 1] = '\0';
		eval("kill", saved_pid);

		do {
			if ((pid = get_pid_by_name(WPS_MONITOR_PATH)) <= 0)
				break;
			wait_time--;
			sleep(1);
		} while (wait_time);

		if (wait_time == 0)
			dprintf("Unable to kill wps_monitor!\n");
	}
	if (fp)
		fclose(fp);
#endif /* __CONFIG_WPS__ */

	return ret;
}

int
start_wps(void)
{
	int ret = 0;
#ifdef __CONFIG_WPS__
	char *wps_argv[] = {WPS_MONITOR_PATH, NULL};
	pid_t pid;

	/* For PF#1 debug only + */
	if (nvram_match("nsa_only", "1")) {
		dprintf("nsa_only mode, ignore wps_monitor!\n");
		return 0;
	}
	/* For PF#1 debug only - */

	if (nvram_match("wps_restart", "1")) {
#ifdef BCA_HNDROUTER
		wait_lan_port_to_forward_state();
#endif /* BCA_HNDROUTER */
		nvram_set("wps_restart", "0");
	}
	else {
		nvram_set("wps_restart", "0");
		nvram_set("wps_proc_status", "0");
	}

	nvram_set("wps_sta_pin", "00000000");

	/* stop wps first in case some one call start_wps for restart it. */
	stop_wps();

	_eval(wps_argv, NULL, 0, &pid);
#else
	/* if we don't support WPS, make sure we unset any remaining wl_wps_mode */
	nvram_unset("wl_wps_mode");
#endif /* __CONFIG_WPS__ */

	return ret;
}

#ifdef __CONFIG_IGMP_PROXY__
void
start_igmp_proxy(void)
{
	/* Start IGMP Proxy in Router mode only */
	if (!nvram_match("router_disable", "1"))
		eval("igmp", nvram_get("wan_ifname"));
	else {
		if (!nvram_match("igmp_enable", "0")) {
			/* Start IGMP proxy in AP mode for media router build */
			eval("igmp", nvram_get("lan_ifname"));
		}
	}

	return;
}

void
stop_igmp_proxy(void)
{
	eval("killall", "igmp");
	return;
}
#endif /* __CONFIG_IGMP_PROXY__ */

#if defined(BCA_HNDROUTER) && defined(MCPD_PROXY)
static void
mcpd_conf(void)
{
	FILE *fp;
	char *conf_file = "/var/mcpd.conf";
	char *proxy_ifname;

	if (!(fp = fopen(conf_file, "w+"))) {
		perror(conf_file);
	}

	/* IGMP configuration */
	fprintf(fp, "##### IGMP configuration #####\n");
	fprintf(fp, "igmp-default-version 3\n");
	fprintf(fp, "igmp-query-interval 20\n");
	fprintf(fp, "igmp-query-response-interval 100\n");
	fprintf(fp, "igmp-last-member-query-interval 10\n");
	fprintf(fp, "igmp-robustness-value 2\n");
	fprintf(fp, "igmp-max-groups 25\n");
	fprintf(fp, "igmp-max-sources 25\n");
	fprintf(fp, "igmp-max-members 25\n");
	fprintf(fp, "igmp-fast-leave 1\n");
	fprintf(fp, "igmp-admission-required 0\n");
	fprintf(fp, "igmp-admission-bridging-filter 0\n");

	/* Start MCPD Proxy in router mode */
	if (!nvram_match("router_disable", "1")) {
		proxy_ifname = nvram_safe_get("wan_ifname");
	} else {
		if (!nvram_match("igmp_enable", "0")) {
			/* Start MCPD proxy in AP mode for media router build */
			proxy_ifname = nvram_safe_get("lan_ifname");
		}
	}

	fprintf(fp, "igmp-proxy-interfaces %s\n", proxy_ifname);
	fprintf(fp, "igmp-snooping-interfaces %s\n", nvram_safe_get("lan_ifname"));
	fprintf(fp, "igmp-mcast-interfaces %s\n", proxy_ifname);

	/* Mcast configuration */
	fprintf(fp, "\n##### MCAST configuration #####\n");
	fprintf(fp, "igmp-mcast-snoop-exceptions "
		"239.255.255.250/255.255.255.255 "
		"224.0.255.135/255.255.255.255\n");
	fprintf(fp, "mld-mcast-snoop-exceptions "
		"ff05::0001:0003/ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff\n");

	if (fp)
		fclose(fp);
}

void
start_mcpd_proxy(void)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");

	/* Create mcpd.conf */
	mcpd_conf();

	/* Run mcpd */
	system("/bin/mcpd &");

	/* Enable LAN-to-LAN snooping in Router mode */
	if (!nvram_match("router_disable", "1")) {
		eval("/bin/bmc", "l2l", "-e", "1", "-p", "1", "-i", lan_ifname);

		eval("/bin/bmc", "l2l", "-e", "1", "-p", "2", "-i", lan_ifname);
	}

	dprintf("done\n");
	return;
}

void
stop_mcpd_proxy(void)
{
	eval("killall", "mcpd");

	dprintf("done\n");
	return;
}

void
restart_mcpd_proxy(void)
{
	/* stop mcpd */
	eval("killall", "mcpd");

	/* Run mcpd */
	system("/bin/mcpd &");

	dprintf("done\n");
	return;
}
#endif /* BCA_HNDROUTER && MCPD_PROXY */

#ifdef __CONFIG_HSPOT__

int
start_hspotap(void)
{
	char *hs_argv[] = {HSPOTAP_PATH, NULL};
	pid_t pid;
	int wait_time = 3;

	eval("killall", "hspotap");
	do {
		if ((pid = get_pid_by_name(HSPOTAP_PATH)) <= 0)
			break;
		wait_time--;
		sleep(1);
	} while (wait_time);
	if (wait_time == 0)
		dprintf("Unable to kill hspotap!\n");

	_eval(hs_argv, NULL, 0, &pid);

	dprintf("done\n");
	return 0;
}

int
stop_hspotap(void)
{
	int ret = 0;
	ret = eval("killall", "hspotap");

	dprintf("done\n");
	return ret;
}
#endif /* __CONFIG_HSPOT__ */

#ifdef __CONFIG_LLD2D__
int start_lltd(void)
{
	chdir("/usr/sbin");
	eval("lld2d", "br0");
	chdir("/");

	return 0;
}

int stop_lltd(void)
{
	int ret;

	ret = eval("killall", "lld2d");

	return ret;
}
#endif /* __CONFIG_LLD2D__ */

#ifdef BCM_ASPMD
int
start_aspmd(void)
{
	int ret = system("/usr/sbin/aspmd");

	return ret;
}

int
stop_aspmd(void)
{
	int ret = eval("killall", "aspmd");

	return ret;
}
#endif /* BCM_ASPMD */

int
start_eapd(void)
{
	int ret = system(EAPD_PATH);

	return ret;
}

int
stop_eapd(void)
{
	int ret = eval("killall", "eapd");

	return ret;
}

#ifdef  __CONFIG_RPCAPD__
int
start_rpcapd(void)
{
	char *lan_ifnames, *lan_ifname, name[80], *next, prefix[32];
	char tmp[100], *str;
	int  start = 0, ret = 0;
	lan_ifname = nvram_safe_get("lan_ifname");
	lan_ifnames = nvram_safe_get("lan_ifnames");
	foreach(name, lan_ifnames, next) {
		osifname_to_nvifname(name, prefix, sizeof(prefix));
		if (strncmp(prefix, "wl", 2) != 0) {
			continue; /* Ignore other than wl interfaces */
		}
		str = nvram_safe_get(strcat_r(prefix, "_mode", tmp));
		if (!strcmp(str, "monitor")) {
			start = 1;
			break;
		}
	} /* foreach().... */
	if (start) {
		ret = eval("rpcapd", "-d", "-n");
	}
	return ret;
}

	int
stop_rpcapd(void)
{
	int ret = eval("killall", "rpcapd");
	return ret;
}
#endif /* __CONFIG_RPCAPD__ */

#if defined(BCM_DCS) || defined(EXT_ACS)
int
start_acsd(void)
{
	int ret = system(ACSD_PATH);

	return ret;
}

int
stop_acsd(void)
{
	int ret = eval("killall", "acsd");

	return ret;
}
#endif /* BCM_DCS || EXT_ACS */

#if defined(__CONFIG_TOAD__)
static void
start_toads(void)
{
	char toad_ifname[16];
	char *next;

	foreach(toad_ifname, nvram_safe_get("toad_ifnames"), next) {
		eval(TOAD_PATH, "-i", toad_ifname);
	}
}

static void
stop_toads(void)
{
	eval("killall", "toad");
}
#endif /* __CONFIG_TOAD__ */

#if defined(BCM_BSD)
int start_bsd(void)
{
	int ret = eval(BSD_PATH);

	return ret;
}

int stop_bsd(void)
{
	int ret = eval("killall", "bsd");

	return ret;
}
#endif /* BCM_BSD */

#if defined(BCM_APPEVENTD)
int start_appeventd(void)
{
	int ret = 0;
	char *appeventd_argv[] = {APPEVENTD_PATH, NULL};
	pid_t pid;

	if (nvram_match("appeventd_enable", "1"))
		ret = _eval(appeventd_argv, NULL, 0, &pid);

	return ret;
}

int stop_appeventd(void)
{
	int ret = eval("killall", "appeventd");

	return ret;
}
#endif /* BCM_APPEVENTD */

#if defined(BCM_SSD)
int start_ssd(void)
{
	int ret = 0;
	char *ssd_argv[] = {SSD_PATH, NULL};
	pid_t pid;

	if (nvram_match("ssd_enable", "1"))
		ret = _eval(ssd_argv, NULL, 0, &pid);

	return ret;
}

int stop_ssd(void)
{
	int ret = eval("killall", "ssd");

	return ret;
}
#endif /* BCM_SSD */

#if defined(BCM_EVENTD)
int start_eventd(void)
{
	int ret = 0;
	char *ssd_argv[] = {EVENTD_PATH, NULL};
	pid_t pid;

	if (nvram_match("eventd_enable", "1"))
		ret = _eval(ssd_argv, NULL, 0, &pid);

	return ret;
}

int stop_eventd(void)
{
	int ret = eval("killall", "eventd");

	return ret;
}
#endif /* BCM_EVENTD */


#if defined(__CONFIG_DHDAP__)
int start_dhd_monitor(void)
{
	int ret = system("killall dhd_monitor");
	usleep(300000);
	ret = system(DHD_MONITOR_PATH);

	return ret;
}

int stop_dhd_monitor(void)
{
	/* Don't kill dhd_monitor here */
	return 0;
}
#endif /* __CONFIG_DHDAP__ */


#if defined(WL_AIR_IQ)
int start_airiq_service(void)
{
	int ret = 0;

	if (nvram_match("airiq_service_enable", "1")) {
		ret = system("/usr/sbin/airiq_service -c /usr/sbin/airiq_service.cfg "
			"-pfs /usr/sbin/flash_policy.xml &");
	}
	return ret;
}

int stop_airiq_service(void)
{
	int ret = 0;
	if (nvram_match("airiq_service_enable", "1"))
		ret = eval("killall", "airiq_service");
	return ret;
}

#define NUM_AIRIQ_FREQ_BAND_OPTION 3 /* 0: 2Gz, 1: 5Ghz, 2: dual band */
int start_airiq_app(void)
{
	char command[256], active_arg[32], band_arg[32];
        char active_if[NUM_AIRIQ_FREQ_BAND_OPTION][16] = {0};
	char *lan_ifnames, *lan_ifname, name[80], *next, prefix[32];
	int i, band, index, if_cnt = 0;
	int ret = 0;

	if (!nvram_match("airiq_app_enable", "1"))
		return ret;

	lan_ifnames = nvram_safe_get("lan_ifnames");
	foreach(name, lan_ifnames, next) {
		osifname_to_nvifname(name, prefix, sizeof(prefix));
		if (strncmp(prefix, "wl", 2) != 0) {
			continue; /* Ignore other than wl interfaces */
		}
		snprintf(active_arg, sizeof(active_arg), "%s_airiq_active", prefix);
		snprintf(band_arg, sizeof(band_arg), "%s_airiq_scan_band", prefix);

		if (nvram_match(active_arg, "1")) {
			band = atoi(nvram_safe_get(band_arg));
			index = band -1;
			switch (band)
			{
			case 1: /* 2Ghz */
				snprintf(active_if[index], sizeof(active_if[index]),
					"-i %s -b", name);
				if_cnt++;
				break;
			case 2: /* 5Ghz */
				snprintf(active_if[index], sizeof(active_if[index]),
					"-i %s -a", name);
				if_cnt++;
				break;
			case 3: /* dual band */
				snprintf(active_if[index], sizeof(active_if[index]),
					"-i %s", name);
				if_cnt++;
				break;
			default:
				/* ignored not a valid option */
				break;
			}
		}
	} /* foreach().... */

	snprintf(command, sizeof(command), "/usr/sbin/airiq_app %s %s %s > /dev/null 2>&1 &",
		active_if[0], active_if[1], active_if[2]);
	if (if_cnt) {
		ret = system(command);
	}
	return ret;
}

int stop_airiq_app(void)
{
	int ret = 0;
	if (nvram_match("airiq_app_enable", "1"))
		ret = eval("killall", "airiq_app");
	return ret;
}
#endif /* WL_AIR_IQ */


#if defined(PHYMON)
int
start_phymons(void)
{
	int ret = eval("/usr/sbin/phymons");

	return ret;
}

int
stop_phymons(void)
{
	int ret = eval("killall", "phymons");

	return ret;
}
#endif /*  PHYMON */

#ifdef __CONFIG_SAMBA__
#ifdef BCA_HNDROUTER
#define SMB_CONF	"/var/samba/smb.conf"
#else
#define SMB_CONF	"/tmp/samba/lib/smb.conf"
#endif /* BCA_HNDROUTER */

void enable_gro(int interval)
{
#ifdef LINUX26
	char *argv[3] = {"echo", "", NULL};
	char lan_ifname[32], *lan_ifnames, *next;
	char path[64] = {0};
	char parm[32] = {0};

	if (nvram_match("inet_gro_disable", "1"))
		return;

	/* enabled gro on vlan interface */
	lan_ifnames = nvram_safe_get("lan_ifnames");
	foreach(lan_ifname, lan_ifnames, next) {
		if (!strncmp(lan_ifname, "vlan", 4)) {
			sprintf(path, ">>/proc/net/vlan/%s", lan_ifname);
#if defined(LINUX_2_6_36)
			sprintf(parm, "-gro %d", interval);
#else
			/* 131072 define the max length gro skb can chained */
			sprintf(parm, "-gro %d %d", interval, 131072);
#endif /* LINUX_2_6_36 */
			argv[1] = parm;
			_eval(argv, path, 0, NULL);
		}
	}
#endif /* LINUX26 */
}

#if defined(LINUX_2_6_36)
static void
parse_blkdev_port(char *devname, int *port)
{
	FILE *fp;
	char buf[256];
	char uevent_path[32];

	sprintf(uevent_path, "/sys/block/%s/uevent", devname);

	if ((fp = fopen(uevent_path, "r")) == NULL)
		goto exit;

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (strstr(buf, "PHYSDEVPATH=") != NULL) {
			/* PHYSDEVPATH=/devices/pci/hcd/root_hub/root_hub_num-lport/... */
			sscanf(buf, "%*[^/]/%*[^/]/%*[^/]/%*[^/]/%*[^/]/%*[^-]-%d", port);
			break;
		}
	}

exit:
	if (fp)
		fclose(fp);
}

static void
samba_storage_conf(void)
{
	extern char *mntdir;
	FILE *mnt_file;
	struct mntent *mount_entry;
	char *mount_point = NULL;
	char basename[16], devname[16];
	char *argv[3] = {"echo", "", NULL};
	char share_dir[16], path[32];
	int port = -1;

	mnt_file = setmntent("/proc/mounts", "r");

	while ((mount_entry = getmntent(mnt_file)) != NULL) {
		mount_point = mount_entry->mnt_dir;

		if (strstr(mount_point, mntdir) == NULL)
			continue;

		/* Parse basename */
#ifdef BCA_HNDROUTER
		sscanf(mount_point, "/%*[^/]/%*[^/]/%*[^/]/%s", basename);
#else /* !BCA_HNDROUTER */
		sscanf(mount_point, "/%*[^/]/%*[^/]/%s", basename);

		/* Parse mounted storage partition */
		if (strncmp(basename, "sd", 2) == 0) {
			sscanf(basename, "%3s", devname);

			parse_blkdev_port(devname, &port);

			if (port == 1)
				sprintf(share_dir, "[usb3_%s]", basename);
			else if (port == 2)
				sprintf(share_dir, "[usb2_%s]", basename);
			else
				sprintf(share_dir, "[%s]", basename);
		} else
#endif /* BCA_HNDROUTER */
			sprintf(share_dir, "[%s]", basename);

		/* Create storage partitions */
		argv[1] = share_dir;
		_eval(argv, ">>"SMB_CONF, 0, NULL);

		sprintf(path, "path = %s", mount_point);
		argv[1] = path;
		_eval(argv, ">>"SMB_CONF, 0, NULL);

		argv[1] = "writeable = yes";
		_eval(argv, ">>"SMB_CONF, 0, NULL);

		argv[1] = "browseable = yes";
		_eval(argv, ">>"SMB_CONF, 0, NULL);

		argv[1] = "guest ok = yes";
		_eval(argv, ">>"SMB_CONF, 0, NULL);
	}

	endmntent(mnt_file);
}
#endif /* LINUX_2_6_36 */

int
start_samba()
{
	char *argv[3] = {"echo", "", NULL};
	char *samba_mode;
	char *samba_passwd;
#if defined(LINUX_2_6_36)
	int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
	int taskset_ret = -1;
#endif	/* LINUX_2_6_36 */

#ifndef BCA_HNDROUTER
#if defined(LINUX_2_6_36)
	enable_gro(2);
#else
	enable_gro(1);
#endif	/* LINUX_2_6_36 */
#endif /* !BCA_HNDROUTER */

	samba_mode = nvram_safe_get("samba_mode");
	samba_passwd = nvram_safe_get("samba_passwd");

	/* Samba is disabled */
	if (strncmp(samba_mode, "1", 1) && strncmp(samba_mode, "2", 1)) {
#ifndef BCA_HNDROUTER
		if (nvram_match("txworkq", "1")) {
			nvram_unset("txworkq");
			nvram_commit();
		}
#endif /* !BCA_HNDROUTER */
		return 0;
	}

#ifndef BCA_HNDROUTER
	if (!nvram_match("txworkq", "1")) {
		nvram_set("txworkq", "1");
		nvram_commit();
	}
#endif /* !BCA_HNDROUTER */

	/* Create smb.conf */
	argv[1] = "[global]";
	_eval(argv, ">"SMB_CONF, 0, NULL);

	argv[1] = "workgroup = mygroup";
	_eval(argv, ">>"SMB_CONF, 0, NULL);

	if (!strncmp(samba_mode, "1", 1))
		argv[1] = "security = user";
	else
		argv[1] = "security = share";
	_eval(argv, ">>"SMB_CONF, 0, NULL);

#ifdef BCA_HNDROUTER
	argv[1] = "socket options = TCP_NODELAY SO_RCVBUF=262144 SO_SNDBUF=262144";
	_eval(argv, ">>"SMB_CONF, 0, NULL);
#endif /* BCA_HNDROUTER */

	argv[1] = "use sendfile = yes";
	_eval(argv, ">>"SMB_CONF, 0, NULL);

#if defined(LINUX_2_6_36)
	argv[1] = "use recvfile = yes";
	_eval(argv, ">>"SMB_CONF, 0, NULL);
#endif /* LINUX_2_6_36 */

#ifdef LINUX26
	argv[1] = "[media]";
#else
	argv[1] = "[mnt]";
#endif
	_eval(argv, ">>"SMB_CONF, 0, NULL);

#ifdef BCA_HNDROUTER
	argv[1] = "path = /var/tmp/media";
#elif defined(LINUX26)
	argv[1] = "path = /media";
#else
	argv[1] = "path = /mnt";
#endif
	_eval(argv, ">>"SMB_CONF, 0, NULL);

	argv[1] = "writeable = yes";
	_eval(argv, ">>"SMB_CONF, 0, NULL);

	argv[1] = "browseable = yes";
	_eval(argv, ">>"SMB_CONF, 0, NULL);

	argv[1] = "guest ok = yes";
	_eval(argv, ">>"SMB_CONF, 0, NULL);

#if defined(LINUX_2_6_36)
	samba_storage_conf();
#endif /* LINUX_2_6_36 */

	/* Start smbd */
#if defined(LINUX_2_6_36) && !defined(BCA_HNDROUTER)
	if (cpu_num > 1)
		taskset_ret = eval("taskset", "-c", "1", "smbd", "-D");

	if (taskset_ret != 0)
#endif	/* LINUX_2_6_36 && !BCA_HNDROUTER */
		eval("smbd", "-D");

	/* Set admin password */
	argv[1] = samba_passwd;
	_eval(argv, ">/tmp/spwd", 0, NULL);
	_eval(argv, ">>/tmp/spwd", 0, NULL);
	system("smbpasswd -a admin -s </tmp/spwd");

	return 0;
}

int
stop_samba()
{
	eval("killall", "smbd");
#ifdef BCA_HNDROUTER
	if (!access("/var/samba/locks", 0))
		eval("rm", "-r", "/var/samba/locks");

	if (!access("/var/samba/secrets.tdb", 0))
		eval("rm", "/var/samba/secrets.tdb");
#else /* !BCA_HNDROUTER */
	eval("rm", "-r", "/tmp/samba/var/locks");
	eval("rm", "/tmp/samba/private/passdb.tdb");
#if defined(LINUX_2_6_36)
	eval("rm", "/tmp/samba/private/secrets.tdb");
#endif	/* LINUX_2_6_36 */
	enable_gro(0);
#endif /* BCA_HNDROUTER */

	return 0;
}

#if defined(LINUX_2_6_36)
#define SAMBA_LOCK_FILE      "/tmp/samba_lock"

int
restart_samba(void)
{
	int lock_fd = -1, retry = 3;

	while (retry--) {
		if ((lock_fd = open(SAMBA_LOCK_FILE, O_RDWR|O_CREAT|O_EXCL, 0444)) < 0)
			sleep(1);
		else
			break;
	}

	if (lock_fd < 0)
		return -1;

	stop_samba();
	start_samba();
	usleep(200000);

	close(lock_fd);
	unlink(SAMBA_LOCK_FILE);

	return 0;
}

#define MEM_SIZE_THRESH	65536
void
reclaim_mem_earlier(void)
{
	FILE *fp;
	char memdata[256] = {0};
	uint memsize = 0;

	if ((fp = fopen("/proc/meminfo", "r")) != NULL) {
		while (fgets(memdata, 255, fp) != NULL) {
			if (strstr(memdata, "MemTotal") != NULL) {
				sscanf(memdata, "MemTotal:        %d kB", &memsize);
				break;
			}
		}
		fclose(fp);
	}

	/* Reclaiming memory at earlier time */
	if (memsize > MEM_SIZE_THRESH)
		system("echo 14336 > /proc/sys/vm/min_free_kbytes");
}
#endif /* LINUX_2_6_36 */
#endif /* __CONFIG_SAMBA__ */

#ifdef __CONFIG_DLNA_DMR__
int start_dlna_dmr()
{
	char *dlna_dmr_enable = nvram_safe_get("dlna_dmr_enable");

	if (strcmp(dlna_dmr_enable, "1") == 0) {
		cprintf("Start bcmmrenderer.\n");
		eval("sh", "-c", "bcmmrenderer&");
	}
}

int stop_dlna_dmr()
{
	cprintf("Stop bcmmrenderer.\n");
	eval("killall", "bcmmrenderer");
}
#endif /* __CONFIG_DLNA_DMR__ */


#ifdef __CONFIG_WL_ACI__
int start_wl_aci(void)
{
	int ret = 0;

	/*
	 * If the ACI daemon is enabled, start the ACI daemon.  If not
	 *  simply return
	 */
	if (!nvram_match("acs_daemon", "up"))
		return 0;

	ret = eval("acs");
	return ret;
}

int
stop_wl_aci(void)
{
	int ret;

	ret = eval("killall", "acs");
	return ret;
}
#endif /* __CONFIG_WL_ACI__ */

#ifdef RWL_SOCKET
int
start_server_socket(void)
{
	pid_t pid;
	char *argv[3];
	char *lan_ifname = nvram_safe_get("lan_ifname");

	if (!strcmp(lan_ifname, "")) {
		lan_ifname = "br0";
	}
	argv[0] = "/usr/sbin/wl_server_socket";
	argv[1] = lan_ifname;
	argv[2] = NULL;
	_eval(argv, NULL, 0, &pid);

	return 0;
}

int
stop_server_socket(void)
{
	int ret = eval("killall", "wl_server_socket");

	return ret;
}
#endif /* RWL_SOCKET */

#if defined(LINUX_2_6_36) && defined(__CONFIG_TREND_IQOS__)
static int
start_broadstream_iqos(void)
{
	if (!nvram_match("broadstream_iqos_enable", "1"))
		return 0;
	eval("bcmiqosd", "start");

	return 0;
}

static int
stop_broadstream_iqos(void)
{
	eval("bcmiqosd", "stop");

	return 0;
}
#endif /* LINUX_2_6_36 && __CONFIG_TREND_IQOS__ */

#ifdef __CONFIG_AMIXER__
int
start_amixer(void)
{
	system("amixer cset numid=27 on");
	system("amixer cset numid=29 on");

	return 0;
}

int
stop_amixer(void)
{
	/*
	 * This is not going to stop but just mute the speakers
	 * Need to find a way to kill this driver. TBD
	 */
	system("amixer cset numid=27 off");
	system("amixer cset numid=29 off");

	return 0;
}
#endif /* __CONFIG_AMIXER__ */

#ifdef __CONFIG_MDNSRESPONDER__
int
start_mdns(void)
{
	system("/etc/init.d/mdns start");

	return 0;
}

int
stop_mdns(void)
{
	system("/etc/init.d/mdns stop");

	return 0;
}
#endif /* __CONFIG_MDNSRESPONDER__ */

#ifdef __CONFIG_AIRPLAY__
int
start_airplay(void)
{
	/* We do not run as daemon because of a uclibc bug with daemon function */
	system("/usr/sbin/airplayd&");

	return 0;
}

int
stop_airplay(void)
{
	eval("killall", "airplayd");

	return 0;
}
#endif /* __CONFIG_AIRPLAY__ */

int
start_services(void)
{

/*
*/

	start_httpd();
#if defined(__CONFIG_NAT__) || defined(__CONFIG_STBAP__)
	start_dns();
	start_dhcpd();
#ifdef __CONFIG_UPNP__
	start_upnp();
#endif
#ifdef	__CONFIG_IGD__
	start_igd();
#endif
#endif	/* __CONFIG_NAT__ */
	start_eapd();
#ifdef CONFIG_HOSTAPD
	if (!nvram_match("hapd_enable", "0")) {
		start_hapd_wpasupp();
	} else
#endif	/* CONFIG_HOSTAPD */
	{
		start_nas();
	}
#ifdef __CONFIG_WAPI_IAS__
	start_ias();
#endif /* __CONFIG_WAPI_IAS__ */
#ifdef __CONFIG_WAPI__
	start_wapid();
#endif /* __CONFIG_WAPI__ */
#ifdef CONFIG_HOSTAPD
	if (!nvram_match("hapd_enable", "0")) {
	} else
#endif	/* CONFIG_HOSTAPD */
	{
		start_wps();
	}
#ifndef __CONFIG_STBAP__
#if defined(WLTEST)
	start_telnet();
#else
	if (nvram_match("telnet_enable", "1")) {
		start_telnet();
	}
#endif 
#endif /* __CONFIG_STBAP__ */
#ifdef __CONFIG_HSPOT__
	start_hspotap();
#endif /* __CONFIG_HSPOT__ */
#ifdef __CONFIG_WL_ACI__
	start_wl_aci();
#endif /* __CONFIG_WL_ACI__ */
#ifdef __CONFIG_LLD2D__
	start_lltd();
#endif /* __CONFIG_LLD2D__ */
#if defined(PHYMON)
	start_phymons();
#endif /* PHYMON */
#ifdef WL_AIR_IQ
	/* moved here to give it time to start before starting airiq_app  */
	start_airiq_service();
#endif /* WL_AIR_IQ */

#if defined(BCM_BSD)
	start_bsd();
#endif
#if defined(BCM_APPEVENTD)
	start_appeventd();
#endif
#if defined(BCM_SSD)
	start_ssd();
#endif
#if defined(BCM_EVENTD)
	start_eventd();
#endif
#if defined(BCM_DCS) || defined(EXT_ACS)
	start_acsd();
#endif
#if defined(__CONFIG_DHDAP__)
	start_dhd_monitor();
#endif
#if defined(__CONFIG_TOAD__)
	start_toads();
#endif
#ifdef __CONFIG_SAMBA__
#if defined(LINUX_2_6_36)
	/* Avoid race condition from uevent */
	restart_samba();
#else
	start_samba();
#endif /* LINUX_2_6_36 */
#endif /* __CONFIG_SAMBA__ */

#ifdef __CONFIG_DLNA_DMR__
	start_dlna_dmr();
#endif

#ifdef RWL_SOCKET
	start_server_socket();
#endif

#ifdef PLC
	start_plcnvm();
	start_plcboot();
	start_gigled();
#endif

#if defined(LINUX_2_6_36) && defined(__CONFIG_TREND_IQOS__)
	start_broadstream_iqos();
#endif /* LINUX_2_6_36 && __CONFIG_TREND_IQOS__ */

#ifdef	__CONFIG_VISUALIZATION__
	start_visualization_tool();
#endif /* __CONFIG_VISUALIZATION__ */

#ifdef BCM_ASPMD
	start_aspmd();
#endif /* BCM_ASPMD */

#ifdef  __CONFIG_WBD__
	start_wbd();
#endif /* __CONFIG_WBD__ */

#ifdef  __CONFIG_RPCAPD__
	start_rpcapd();
#endif /* __CONFIG_RPCAPD__ */

#ifdef WL_AIR_IQ
	start_airiq_app();
#endif /* WL_AIR_IQ */
	dprintf("done\n");
	return 0;
}

int
stop_services(void)
{
#ifdef BCM_ASPMD
	stop_aspmd();
#endif /* BCM_ASPMD */
	stop_wps();
#ifdef __CONFIG_WAPI__
	stop_wapid();
#endif /* __CONFIG_WAPI__ */
#ifdef __CONFIG_WAPI_IAS__
	stop_ias();
#endif /* __CONFIG_WAPI_IAS__ */
#ifdef CONFIG_HOSTAPD
	stop_hapd_wpasupp();
#endif	/* CONFIG_HOSTAPD */
	stop_nas();
	stop_eapd();
#if defined(__CONFIG_NAT__) || defined(__CONFIG_STBAP__)
#ifdef	__CONFIG_IGD__
	stop_igd();
#endif
#ifdef	__CONFIG_UPNP__
	stop_upnp();
#endif
	stop_dhcpd();
	stop_dns();
#endif	/* __CONFIG_NAT__ */
	stop_httpd();
#ifndef __CONFIG_STBAP__
#if defined(WLTEST)
	stop_telnet();
#endif
#endif /* __CONFIG_STBAP__ */
#ifdef __CONFIG_HSPOT__
	stop_hspotap();
#endif /* __CONFIG_HSPOT__ */
#ifdef __CONFIG_WL_ACI__
	stop_wl_aci();
#endif /* __CONFIG_WL_ACI__ */
#ifdef __CONFIG_LLD2D__
	stop_lltd();
#endif 	/* __CONFIG_LLD2D__ */
#ifdef __CONFIG_WBD__
	stop_wbd();
#endif /* __CONFIG_WBD__ */
/*
*/
#ifdef __CONFIG_NFC__
	stop_nfc();
#endif
#if defined(PHYMON)
	stop_phymons();
#endif /* PHYMON */
#if defined(__CONFIG_TOAD__)
	stop_toads();
#endif
#if defined(BCM_DCS) || defined(EXT_ACS)
	stop_acsd();
#endif
#if defined(BCM_BSD)
	stop_bsd();
#endif
#if defined(BCM_APPEVENTD)
	stop_appeventd();
#endif
#if defined(BCM_SSD)
	stop_ssd();
#endif
#if defined(BCM_EVENTD)
	stop_eventd();
#endif
#if defined(__CONFIG_DHDAP__)
	stop_dhd_monitor();
#endif
#ifdef __CONFIG_SAMBA__
	stop_samba();
#endif
#ifdef __CONFIG_DLNA_DMR__
	stop_dlna_dmr();
#endif

#ifdef RWL_SOCKET
	stop_server_socket();
#endif

#ifdef PLC
	stop_gigled();
	stop_plcboot();
	stop_plcnvm();
#endif

#if defined(LINUX_2_6_36) && defined(__CONFIG_TREND_IQOS__)
	stop_broadstream_iqos();
#endif /* LINUX_2_6_36 && __CONFIG_TREND_IQOS__ */

#ifdef	__CONFIG_VISUALIZATION__
	stop_visualization_tool();
#endif /* __CONFIG_VISUALIZATION__ */

#ifdef  __CONFIG_RPCAPD__
	stop_rpcapd();
#endif /* __CONFIG_RPCAPD__ */
#ifdef WL_AIR_IQ
	stop_airiq_app();
	stop_airiq_service();
#endif /* WL_AIR_IQ */


	dprintf("done\n");
	return 0;
}
