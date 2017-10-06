/*
 * Microsoft skin
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#include <wsIntrn.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <httpd.h>
#endif /* WEBS */

#include <typedefs.h>
#include <proto/ethernet.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <wlioctl.h>
#include <wlutils.h>
#include <netconf.h>
#include <nvparse.h>

/* FILE-CSTYLED */

#if defined(linux)

#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <net/if.h>

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
#include <linux/ethtool.h>
#include <linux/sockios.h>

#define log_msg(module, level, format...)
#define pppoe_up()
#define pppoe_down()

/* Allow some time for the page to reload before killing ourselves */
static int
kill_after(pid_t pid, int sig, unsigned int after)
{
	if (fork() == 0) {
		sleep(after);
		return kill(pid, sig);
	}
	return 0;
}
#define sys_restart() kill_after(1, SIGHUP, 3)
#define sys_reboot() kill_after(1, SIGTERM, 3)

#ifndef WEBS

/* Upgrade from remote server or socket stream */
static int
sys_upgrade(char *url, FILE *stream, int *total)
{
	char upload_fifo[] = "/tmp/uploadXXXXXX";
	FILE *fifo = NULL;
	char *write_argv[] = { "write", upload_fifo, "linux", NULL };
	pid_t pid;
	char buf[1024];
	int count, ret = 0;
	long flags = -1;

	if (url)
		return eval("write", url, "linux");

	/* Feed write from a temporary FIFO */
	if (!mktemp(upload_fifo) ||
	    mkfifo(upload_fifo, S_IRWXU) < 0||
	    (ret = _eval(write_argv, NULL, 0, &pid)) ||
	    !(fifo = fopen(upload_fifo, "w"))) {
		if (!ret) {
			perror("mktemp");
			ret = errno;
		}
		goto err;
	}

	/* Set nonblock on the socket so we can timeout */
	if ((flags = fcntl(fileno(stream), F_GETFL)) < 0 ||
	    fcntl(fileno(stream), F_SETFL, flags | O_NONBLOCK) < 0) {
		perror("fcntl");
		ret = errno;
		goto err;
	}

	/* Pipe the rest to the FIFO */
	printf("Upgrading");
	while (total && *total) {
		if (waitfor(fileno(stream), 5) <= 0)
			break;
		count = fread(buf, 1, sizeof(buf), stream);
		*total -= count;
		fwrite(buf, 1, count, fifo);
		printf(".");
	}
	fclose(fifo);
	fifo = NULL;
	printf("done\n");

	/* Wait for write to terminate */
	waitpid(pid, &ret, 0);

	/* Reset nonblock on the socket */
	if (fcntl(fileno(stream), F_SETFL, flags) < 0) {
		ret = errno;
		goto err;
	}

 err:
	if (fifo)
		fclose(fifo);
	unlink(upload_fifo);
	return ret;
}

#endif

static char *
rfctime(const time_t *timep)
{
	static char s[201];
	struct tm tm;

	setenv("TZ", nvram_safe_get("time_zone"), 1);
	memcpy(&tm, localtime(timep), sizeof(struct tm));
	strftime(s, 200, "%a, %d %b %Y %H:%M:%S %z", &tm);
	return s;
}

#define clearlog() klogctl(5, NULL, 0)

/* Dump firewall log */
static int
dumplog(webs_t wp, char *sep)
{
	char buf[4096], *line, *next, *s;
	int len, ret = 0;

	time_t tm;
	char *verdict, *src, *dst, *proto, *spt, *dpt;

	if (klogctl(3, buf, 4096) < 0) {
		websError(wp, 400, "Insufficient memory\n");
		return -1;
	}

	for (next = buf; (line = strsep(&next, "\n"));) {
		if (!strncmp(line, "<4>DROP", 7))
			verdict = "denied";
		else if (!strncmp(line, "<4>ACCEPT", 9))
			verdict = "accepted";
		else
			continue;

		/* Parse into tokens */
		s = line;
		len = strlen(s);
		while (strsep(&s, " "));

		/* Initialize token values */
		time(&tm);
		src = dst = proto = spt = dpt = "n/a";

		/* Set token values */
		for (s = line; s < &line[len] && *s; s += strlen(s) + 1) {
			if (!strncmp(s, "TIME=", 5))
				tm = strtoul(&s[5], NULL, 10);
			else if (!strncmp(s, "SRC=", 4))
				src = &s[4];
			else if (!strncmp(s, "DST=", 4))
				dst = &s[4];
			else if (!strncmp(s, "PROTO=", 6))
				proto = &s[6];
			else if (!strncmp(s, "SPT=", 4))
				spt = &s[4];
			else if (!strncmp(s, "DPT=", 4))
				dpt = &s[4];
		}

		ret += websWrite(wp, "%s %s connection %s to %s:%s from %s:%s",
				 rfctime(&tm), proto, verdict, dst, dpt, src, spt);
		ret += websWrite(wp, sep);
	}

	return ret;
}

struct lease_t {
	unsigned char chaddr[16];
	u_int32_t yiaddr;
	u_int32_t expires;
	char hostname[256];
};

/* Dump leases */
static int
ej_dhcp_clients(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp = NULL;
	struct lease_t lease;
	int i;
	struct in_addr addr;
	char sigusr1[] = "-XX";
	int ret = 0;

	/* Write out leases file */
	sprintf(sigusr1, "-%d", SIGUSR1);
	eval("killall", sigusr1, "udhcpd");

	if (!(fp = fopen("/tmp/udhcpd.leases", "r")))
		return 0;

	while (fread(&lease, sizeof(lease), 1, fp)) {
		ret += websWrite(wp, "<tr>\n");
		addr.s_addr = lease.yiaddr;
		ret += websWrite(wp, "<td width=\"25%%\" class=\"normalText\">%s</td>\n", inet_ntoa(addr));
		ret += websWrite(wp, "<td width=\"25%%\" class=\"normalText\">%s</td>\n", lease.hostname);
		ret += websWrite(wp, "<td width=\"25%%\" class=\"normalText\">");
		for (i = 0; i < 6; i++) {
			ret += websWrite(wp, "%02X", lease.chaddr[i]);
			if (i != 5) ret += websWrite(wp, ":");
		}
		ret += websWrite(wp, "</td>\n");
		ret += websWrite(wp, "<td width=\"25%%\" class=\"normalText\">&nbsp;</td>\n");
		ret += websWrite(wp, "</tr>\n");
	}

	fclose(fp);
	return ret;
}

static int
ej_link(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name;
	int s;
	struct ifreq ifr;
	struct ethtool_cmd ecmd;
	FILE *fp;

	if (ejArgs(argc, argv, "%s", &name) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	/* PPPoE connection status */
	if (nvram_match("wan_proto", "pppoe")) {
		if ((fp = fopen("/tmp/ppp/link", "r"))) {
			fclose(fp);
			return websWrite(wp, "Connected");
		} else
			return websWrite(wp, "Disconnected");
	}

	/* Open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return websWrite(wp, "Unknown");

	/* Check for non-zero link speed */
	strncpy(ifr.ifr_name, nvram_safe_get(name), IFNAMSIZ);
	ifr.ifr_data = (void *) &ecmd;
	ecmd.cmd = ETHTOOL_GSET;
	if (ioctl(s, SIOCETHTOOL, &ifr) < 0) {
		close(s);
		return websWrite(wp, "Unknown");
	}

	/* Cleanup */
	close(s);

	if (ecmd.speed)
		return websWrite(wp, "Connected");
	else
		return websWrite(wp, "Disconnected");
}

static int
ej_peer_mac(int eid, webs_t wp, int argc, char_t **argv)
{
	int byte = -1;
	struct sockaddr_in peer;
	socklen_t len;
	FILE *fp;
	char line[200], ip[100], hwa[100], mask[100], dev[100];
	unsigned char mac[ETHER_ADDR_LEN];
	int type, flags;
	int s;

	ejArgs(argc, argv, "%d", &byte);

	if (byte >= ETHER_ADDR_LEN) {
		websError(wp, 400, "Invalid byte\n");
		return -1;
	}

	len = sizeof(peer);
#ifdef WEBS
	s = wp->sid;
#else
	s = fileno(wp);
#endif
	if (getpeername(s, (struct sockaddr *) &peer, &len) < 0) {
		websError(wp, 400, strerror(errno));
		return -1;
	}

	/* Parse /proc/net/arp */
	if (!(fp = fopen("/proc/net/arp", "r"))) {
		websError(wp, 400, strerror(errno));
		return -1;
	}

	/* Eat first line */
	(void) fgets(line, sizeof(line), fp);

	/* Read the ARP cache entries */
	while (fgets(line, sizeof(line), fp)) {
		if (sscanf(line, "%s 0x%x 0x%x %100s %100s %100s\n",
			   ip, &type, &flags, hwa, mask, dev) < 4)
			break;
		if (!strcmp(ip, inet_ntoa(peer.sin_addr))) {
			fclose(fp);
			if (byte != -1) {
				ether_atoe(hwa, mac);
				return websWrite(wp, "%02X", mac[byte]);
			} else
				return websWrite(wp, hwa);
		}
	}

	fclose(fp);
	return 0;
}	

/* Renew lease */
static int
sys_renew(void)
{
	char sigusr1[] = "-XX";

	sprintf(sigusr1, "-%d", SIGUSR1);
	return eval("killall", sigusr1, "udhcpc");
}

/* Release lease */
static int
sys_release(void)
{
	char sigusr2[] = "-XX";

	sprintf(sigusr2, "-%d", SIGUSR2);
	return eval("killall", sigusr2, "udhcpc");
}

#endif

static int
valid_ipaddr(char *value, struct in_addr *in)
{
	unsigned int buf[4];

	if (sscanf(value, "%d.%d.%d.%d", &buf[0], &buf[1], &buf[2], &buf[3]) != 4)
		return 0;

	in->s_addr = htonl((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);

	if (in->s_addr == ntohl(INADDR_ANY) || in->s_addr == ntohl(INADDR_BROADCAST))
		return 0;

	return 1;
}

static int
valid_range(char *start, char *end, int min, int max)
{
	int low = atoi(start);
	int high = atoi(end);

	if (low >= min && low <= max &&
	    high >= min && high <= max &&
	    high >= low)
		return 1;
	else
		return 0;
}

static int
ej_peer_ip(int eid, webs_t wp, int argc, char_t **argv)
{
	int byte = -1;
	struct sockaddr_in peer;
	socklen_t len;
	int s;

	ejArgs(argc, argv, "%d", &byte);

	if (byte >= 4) {
		websError(wp, 400, "Invalid byte\n");
		return -1;
	}

	len = sizeof(peer);
#ifdef WEBS
	s = wp->sid;
#else
	s = fileno(wp);
#endif
	if (getpeername(s, (struct sockaddr *) &peer, &len) < 0) {
		websError(wp, 400, strerror(errno));
		return -1;
	}

	if (byte != -1)
		return websWrite(wp, "%d", (int)((peer.sin_addr.s_addr >> (byte * 8)) & 0xff));
	else
		return websWrite(wp, inet_ntoa(peer.sin_addr));
}

static int
ej_dumplog(int eid, webs_t wp, int argc, char_t **argv)
{
	return dumplog(wp, "<br>");
}

/*
 * Example: 
 * lan_ipaddr=192.168.1.1
 * <% nvram_get("lan_ipaddr"); %> produces "192.168.1.1"
 * <% nvram_get("undefined"); %> produces ""
 */
static int
ej_nvram_get(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name, *c;
	int ret = 0;

	if (ejArgs(argc, argv, "%s", &name) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	for (c = nvram_safe_get(name); *c; c++) {
		if (isprint((int) *c) &&
		    *c != '"' && *c != '&' && *c != '<' && *c != '>')
			ret += websWrite(wp, "%c", *c);
		else
			ret += websWrite(wp, "&#%d", *c);
	}

	return ret;
}

/*
 * Example: 
 * wan_proto=dhcp
 * <% nvram_match("wan_proto", "dhcp", "selected"); %> produces "selected"
 * <% nvram_match("wan_proto", "static", "selected"); %> does not produce
 */
static int
ej_nvram_match(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name, *match, *output, *alternate = NULL;

	if (ejArgs(argc, argv, "%s %s %s %s", &name, &match, &output, &alternate) < 3) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if (nvram_match(name, match))
		return websWrite(wp, output);
	else if (alternate)
		return websWrite(wp, alternate);
		
	return 0;
}	

/*
 * Example: 
 * wan_proto=dhcp
 * <% nvram_invmatch("wan_proto", "dhcp", "disabled"); %> does not produce
 * <% nvram_invmatch("wan_proto", "static", "disabled"); %> produces "disabled"
 */
static int
ej_nvram_invmatch(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name, *invmatch, *output, *alternate = NULL;

	if (ejArgs(argc, argv, "%s %s %s %s", &name, &invmatch, &output, &alternate) < 3) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if (nvram_invmatch(name, invmatch))
		return websWrite(wp, output);
	else if (alternate)
		return websWrite(wp, alternate);

	return 0;
}	

/*
 * Example: 
 * filter_mac=00:12:34:56:78:00 00:87:65:43:21:00
 * <% nvram_list("filter_mac", 1); %> produces "00:87:65:43:21:00"
 * <% nvram_list("filter_mac", 100); %> produces ""
 */
static int
ej_nvram_list(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name;
	int which;
	char word[256], *next;

	if (ejArgs(argc, argv, "%s %d", &name, &which) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	foreach(word, nvram_safe_get(name), next) {
		if (which-- == 0)
			return websWrite(wp, word);
	}

	return 0;
}

static int
ej_timezones(int eid, webs_t wp, int argc, char_t **argv)
{
	char *tzs[] = {
		"",
		"(GMT-12) Enewetak, Kwajalein",
		"(GMT-11) Midway Island, Samoa",
		"(GMT-10) Hawaii",
		"(GMT-09) Alaska",
		"(GMT-08) Pacific Time (US &amp; Canada); Tijuana",
		"(GMT-07) Arizona",
		"(GMT-07) Mountain Time (US &amp; Canada)",
		"(GMT-06) Central Time (US &amp; Canada)",
		"(GMT-06) Mexico City, Tegucigalpa",
		"(GMT-06) Saskatchewan",
		"(GMT-05) Bogota, Lima, Quito",
		"(GMT-05) Eastern Time (US &amp; Canada)",
		"(GMT-05) Indiana (East)",
		"(GMT-04) Atlantic Time (Canada)",
		"(GMT-04) Caracas, La Paz",
		"(GMT-04) Santiago",
		"(GMT-03:30) Newfoundland",
		"(GMT-03) Brasilia",
		"(GMT-03) Buenos Aires, Georgetown",
		"(GMT-02) Mid-Atlantic",
		"(GMT-01) Azores, Cape Verde Is.",
		"(GMT) Casablanca, Monrovia",
		"(GMT) Greenwich Mean Time: Dublin, Edinburgh",
		"(GMT) Greenwich Mean Time: Lisbon, London",
		"(GMT+01) Amsterdam, Berlin, Bern, Rome",
		"(GMT+01) Stockholm, Vienna, Belgrade",
		"(GMT+01) Bratislava, Budapest, Ljubljana",
		"(GMT+01) Prague,Brussels, Copenhagen, Madrid",
		"(GMT+01) Paris, Vilnius, Sarajevo, Skopje",
		"(GMT+01) Sofija, Warsaw, Zagreb",
		"(GMT+02) Athens, Istanbul, Minsk",
		"(GMT+02) Bucharest",
		"(GMT+02) Cairo",
		"(GMT+02) Harare, Pretoria",
		"(GMT+02) Helsinki, Riga, Tallinn",
		"(GMT+02) Israel",
		"(GMT+03) Baghdad, Kuwait, Nairobi, Riyadh",
		"(GMT+03) Moscow, St. Petersburg",
		"(GMT+03:30) Tehran",
		"(GMT+04) Abu Dhabi, Muscat, Tbilisi, Kazan",
		"(GMT+04) Volgograd",
		"(GMT+04:30) Kabul",
		"(GMT+05) Islamabad, Karachi, Ekaterinburg",
		"(GMT+05:30) New Dehli,etc...",
		"(GMT+05:45) Kathmandu",
		"(GMT+06) Almaty, Dhaka",
		"(GMT+06:30) Rangoon",
		"(GMT+07) Bangkok, Jakarta, Hanoi",
		"(GMT+08) Beijing, Chongqing, Urumqi",
		"(GMT+08) Hong Kong, Perth, Singapore, Taipei",
		"(GMT+09) Toyko, Osaka, Sapporo, Yakutsk",
		"(GMT+09:30) Darwin/Adelaide",
		"(GMT+10) Brisbane",
		"(GMT+10) Canberra, Melbourne, Sydney",
		"(GMT+10) Guam, Port Moresby, Vladivostok",
		"(GMT+10) Hobart",
		"(GMT+11) Magadan, Solamon, New Caledonia",
		"(GMT+12) Fiji, Kamchatka, Marshall Is.",
		"(GMT+12) Wellington, Auckland",
		"(GMT+13) Nuku&#039;alofa "
	};
	int i, timeZone = atoi(nvram_safe_get("timeZone"));
	int ret = 0;

	for (i = 1; i < ARRAYSIZE(tzs); i++) {
		ret += websWrite(wp, "<OPTION VALUE=\"%d\"%s>%s</OPTION>\n",
				 i, i == timeZone ? " SELECTED" : "", tzs[i]);
	}

	return ret;
}

static int
ej_months(int eid, webs_t wp, int argc, char_t **argv)
{
	char *months[] = {
		"",
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December"
	};
	char *name;
	int i, month;
	int ret = 0;

	if (ejArgs(argc, argv, "%s", &name) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	month = atoi(nvram_safe_get(name));

	for (i = 1; i < ARRAYSIZE(months); i++) {
		ret += websWrite(wp, "<option value=\"%d\"%s>%s</option>\n",
				 i, i == month ? " selected" : "", months[i]);
	}

	return ret;
}

static int
ej_range(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name;
	int i, which, start, end;
	int ret = 0;

	if (ejArgs(argc, argv, "%s %d %d", &name, &start, &end) < 3) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	which = atoi(nvram_safe_get(name));

	for (i = start; i <= end; i++) {
		ret += websWrite(wp, "<option value=\"%d\"%s>%d</option>\n",
				 i, i == which ? " selected" : "", i);
	}

	return ret;
}

static int
ej_options(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name;
	char word[256], *next;
	int ret = 0;

	if (ejArgs(argc, argv, "%s", &name) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	foreach(word, nvram_safe_get(name), next) {
		ret += websWrite(wp, "<option value=\"%s\">%s</option>\n",
				 word, word);
	}

	return ret;
}

static int
ej_enctype(int eid, webs_t wp, int argc, char_t **argv)
{
	int len;
	char *output;
	char wl_key[] = "wl_keyXXX";

	if (ejArgs(argc, argv, "%d %s", &len, &output) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	snprintf(wl_key, sizeof(wl_key), "wl_key%s", nvram_safe_get("wl_key"));
	if (len == strlen(nvram_safe_get(wl_key)))
		return websWrite(wp, output);

	return 0;
}

#define	PHY_TYPE_A		0
#define	PHY_TYPE_B		1
#define	PHY_TYPE_G		2

static int
ej_txrates(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name;
	wl_rateset_t rs;
	int i, rate, cur, ret = 0;

	switch (wl_phytype()) {
	case PHY_TYPE_A:
		name = "d11a_rate";
		break;
	case PHY_TYPE_B:
		name = "d11b_rate";
		break;
	case PHY_TYPE_G:
		name = "d11g_rate";
		break;
	default:
		return -1;
	}

	cur = atoi(nvram_safe_get(name));

	if (wl_ioctl(wl_name(), WLC_GET_RATESET, &rs, sizeof(rs)))
		return -1;

	ret += websWrite(wp, "<option value=\"0\" %s>Automatic</option>\n",
			 cur == 0 ? "selected" : "");
	for (i = 0; i < rs.count; i++) {
		rate = (rs.rates[i] & 0x7f) * 500000;
		if (rate % 1000000)
			ret += websWrite(wp, "<option value=\"%d\" %s>%d.%d Mbps</option>\n",
					 rate, cur == rate ? "selected" : "",
					 rate / 1000000, (rate % 1000000) / 100000);
		else
			ret += websWrite(wp, "<option value=\"%d\" %s>%d Mbps</option>\n",
					 rate, cur == rate ? "selected" : "",
					 rate / 1000000);
	}
		
	return ret;
}

static int
ej_gmode(int eid, webs_t wp, int argc, char_t **argv)
{
	int gmode;
	int ret = 0;

	if (wl_phytype() != PHY_TYPE_G)
		return 0;

	gmode = atoi(nvram_safe_get("d11g_mode"));

	ret += websWrite(wp,
			 "<tr>\n"
			 "<td width=\"190\"><span class=\"normalText\" id=\"gmodeLabel\">54g&#8482;\n"
			 "mode:</span></td>\n"
			 "<td>\n"
			 "<select size=\"1\" name=\"d11g_mode\" id=\"d11g_mode\" onChange=\"enableApply();\">\n"
			 "<option value=\"%d\" %s>54g Auto</option>\n"
			 "<option value=\"%d\" %s>54g Performance</option>\n"
			 "<option value=\"%d\" %s>54g LRS</option>\n",
			 GMODE_AUTO, gmode == GMODE_AUTO ? "selected" : "",
			 GMODE_PERFORMANCE, gmode == GMODE_PERFORMANCE ? "selected" : "",
			 GMODE_LRS, gmode == GMODE_LRS ? "selected" : "");
	ret += websWrite(wp,
			 "<option value=\"%d\" %s>802.11b Only</option>\n"
			 "</select>\n"
			 "</td>\n"
			 "</tr>\n",
			 GMODE_LEGACY_B, gmode == GMODE_LEGACY_B ? "selected" : "");

	return ret;
}

static int
ej_nvram_ip(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name;
	int byte, which = 0;
	struct in_addr in;
	char word[256], *next;

	if (ejArgs(argc, argv, "%s %d %d", &name, &byte, &which) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if (byte >= 4) {
		websError(wp, 400, "Invalid byte\n");
		return -1;
	}

	foreach(word, nvram_safe_get(name), next) {
		if (which-- == 0) {
			if (!inet_aton(word, &in))
				return 0;
			return websWrite(wp, "%d", (int)((ntohl(in.s_addr) >> ((3 - byte) * 8)) & 0xff));
		}
	}

	return 0;
}

static int
ej_nvram_mac(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name;
	int byte, which = 0;
	unsigned char mac[ETHER_ADDR_LEN];
	char word[256], *next;
		
	if (ejArgs(argc, argv, "%s %d %d", &name, &byte, &which) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if (byte >= ETHER_ADDR_LEN) {
		websError(wp, 400, "Invalid byte\n");
		return -1;
	}

	foreach(word, nvram_safe_get(name), next) {
		if (which-- == 0) {
			ether_atoe(word, mac);
			return websWrite(wp, "%02X", mac[byte]);
		}
	}

	return 0;
}

static int
ej_app_triggered_port_forwards(int eid, webs_t wp, int argc, char_t **argv)
{
	int start, end;
	int ret = 0;
	netconf_app_t app;
	bool valid;
	char port[] = "XXXXX";

	if (ejArgs(argc, argv, "%d %d", &start, &end) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	while (start <= end) {
		valid = get_autofw_port(start, &app);
		ret += websWrite(wp,
				 "<tr> \n"
				 "<td width=\"50\"> \n"
				 "<input type=\"checkbox\" name=\"enableCheck%d\" ID=\"enableCheck%d\" onclick=\"enableApply();\" %s>\n"
				 "</td>\n",
				 start, start,
 				 valid && !(app.match.flags & NETCONF_DISABLED) ? "checked" : "");
		ret += websWrite(wp,
				 "<td width=\"98\"> \n"
				 "<input type=\"text\" name=\"descText%d\" size=\"10\" maxlength=\"59\" ID=\"descText%d\" onkeydown=\"enableApply()\" value=\"%s\">\n"
				 "</td>\n",
				 start, start, app.desc);
		if (valid && app.match.dst.ports[0])
			snprintf(port, sizeof(port), "%u", ntohs(app.match.dst.ports[0]));
		else
			*port = '\0';
		ret += websWrite(wp,
				 "<td width=\"90\"> \n"
				 "<input type=\"text\" name=\"triggerPortText%d\" size=\"5\" maxlength=\"5\" ID=\"triggerPortText%d\" onkeydown=\"enableApply()\" value=\"%s\">\n"
				 "</td>\n",
				 start, start, port);
		ret += websWrite(wp,
				 "<td width=\"90\"> \n"
				 "<select name=\"triggerType%d\" ID=\"triggerType%d\" onchange=\"enableApply();\">\n"
				 "<option value=\"1\" %s>TCP</option>\n"
				 "<option value=\"2\" %s>UDP</option>\n"
				 "</select>\n"
				 "</td>\n",
				 start, start, 
				 valid && app.match.ipproto == IPPROTO_TCP ? "selected" : "",
				 valid && app.match.ipproto == IPPROTO_UDP ? "selected" : "");
		if (valid && app.dport[0])
			snprintf(port, sizeof(port), "%u", ntohs(app.dport[0]));
		else
			*port = '\0';
		ret += websWrite(wp,
				 "<td width=\"125\"> \n"
				 "<input type=\"text\" name=\"publicPorts%d\" size=\"15\"  maxlength=\"255\" ID=\"publicPorts%d\" onkeydown=\"enableApply()\" value=\"%s\">\n"
				 "</td>\n",
				 start, start, port);
		ret += websWrite(wp,
				 "<td> \n"
				 "<select name=\"publicType%d\" ID=\"publicType%d\" onchange=\"enableApply();\">\n"
				 "<option value=\"1\" %s>TCP</option>\n"
				 "<option value=\"2\" %s>UDP</option>\n"
				 "</select>\n"
				 "</td>\n"
				 "</tr>\n"
				 "<tr> \n"
				 "<td width=\"50\" height=\"5\"><img src=\"images/clearpixel.gif\" width=\"5\" height=\"5\"></td>\n"
				 "<td width=\"98\" height=\"5\"><img src=\"images/clearpixel.gif\" width=\"5\" height=\"5\"></td>\n"
				 "<td width=\"90\" height=\"5\"><img src=\"images/clearpixel.gif\" width=\"5\" height=\"5\"></td>\n"
				 "<td width=\"90\" height=\"5\"><img src=\"images/clearpixel.gif\" width=\"5\" height=\"5\"></td>\n"
				 "<td width=\"125\" height=\"5\"><img src=\"images/clearpixel.gif\" width=\"5\" height=\"5\"></td>\n"
				 "<td height=\"5\"><img src=\"images/clearpixel.gif\" width=\"5\" height=\"5\"></td>\n"
				 "</tr>\n",
				 start, start,
				 valid && app.proto == IPPROTO_TCP ? "selected" : "",
				 valid && app.proto == IPPROTO_UDP ? "selected" : "");
		start++;
	}

	return ret;
}

static int
ej_persistent_port_forwards(int eid, webs_t wp, int argc, char_t **argv)
{
	int start, end;
	struct in_addr in;
	int ret = 0;
	netconf_nat_t nat;
	bool valid;
	char port[] = "XXXXX";
	char ipa[] = "255";

	if (ejArgs(argc, argv, "%d %d", &start, &end) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	while (start <= end) {
		valid = get_forward_port(start, &nat);
		ret += websWrite(wp,
				 "<tr>\n"
				 "<td width=\"50\">\n"
				 "<input type=\"checkbox\" name=\"enable%d\" id=\"enable%d\" onClick=\"enableApply();\" %s>\n"
				 "</td>\n",
				 start, start,
				 valid && !(nat.match.flags & NETCONF_DISABLED) ? "checked" : "");
		ret += websWrite(wp,
				 "<td width=\"110\">\n"
				 "<input type=\"text\" name=\"desc%d\" id=\"desc%d\" size=\"12\" maxlength=\"59\" onkeyup=\"\" onkeypress=\"enableApply();\" value=\"%s\">\n"
				 "</td>\n",
				 start, start, nat.desc);
		if (valid && nat.match.dst.ports[0])
			snprintf(port, sizeof(port), "%u", ntohs(nat.match.dst.ports[0]));
		else
			*port = '\0';
		ret += websWrite(wp,
				 "<td width=\"135\">\n"
				 "<input type=\"text\" name=\"publicPortLow%d\" id=\"publicPortLow%d\" size=\"5\" maxlength=\"5\" onkeyup=\"validatePorts(%d,true);\" onkeypress=\"enableApply();\" value=\"%s\">\n"
				 "-\n",
				 start, start, start, port);
		if (valid && nat.match.dst.ports[1])
			snprintf(port, sizeof(port), "%u", ntohs(nat.match.dst.ports[1]));
		else
			*port = '\0';
		ret += websWrite(wp,
				 "<input type=\"text\" name=\"publicPortHigh%d\" id=\"publicPortHigh%d\" size=\"5\" maxlength=\"5\" onkeyup=\"validatePorts(%d,true);\" onkeypress=\"enableApply();\" value=\"%s\" >\n"
				 "</td>\n",
				 start, start, start, port);
		ret += websWrite(wp,
				 "<td width=\"70\">\n"
				 "<select name=\"type%d\" id=\"type%d\" onChange=\"enableApply();\">\n"
				 "<option value=\"1\" %s>TCP</option>\n"
				 "<option value=\"2\" %s>UDP</option>\n"
				 "</select>\n"
				 "</td>\n",
				 start, start,
				 valid && nat.match.ipproto == IPPROTO_TCP ? "selected" : "",
				 valid && nat.match.ipproto == IPPROTO_UDP ? "selected" : "");
		in.s_addr = ntohl(nat.ipaddr.s_addr)&0xff;
		if (valid && in.s_addr) {
			snprintf(ipa, sizeof(ipa), "%u", (unsigned char)in.s_addr);
		} else
			*ipa = '\0';
		in.s_addr = ntohl(inet_addr(nvram_safe_get("lan_ipaddr")));
		ret += websWrite(wp,
				 "<td width=\"110\"><span class=\"normalText\">\n"
				 "%d.%d.%d.\n"
				 "<input type=\"text\" name=\"privateIP%d\" id=\"privateIP%d\" size=\"3\" maxlength=\"3\" onkeyup=\"\" onkeypress=\"enableApply();\" value=\"%s\" >\n"
				 "</span></td>\n",
				 (int)((in.s_addr >> 24) & 0xff), (int)((in.s_addr >> 16) & 0xff), (int)((in.s_addr >> 8) & 0xff),
				 start, start, ipa);
		if (valid && nat.ports[0])
			snprintf(port, sizeof(port), "%u", ntohs(nat.ports[0]));
		else
			*port = '\0';
		ret += websWrite(wp,
				 "<td>\n"
				 "<input type=\"text\" name=\"privatePortLow%d\" id=\"privatePortLow%d\" size=\"5\" maxlength=\"5\" onkeyup=\"validatePorts(%d,false);\" onkeypress=\"enableApply();\" value=\"%s\">\n"
				 "-\n",
				 start, start, start, port);
		if (valid && nat.ports[1])
			snprintf(port, sizeof(port), "%u", ntohs(nat.ports[1]));
		else
			*port = '\0';
		ret += websWrite(wp,
				 "<input type=\"text\" name=\"privatePortHigh%d\" id=\"privatePortHigh%d\" size=\"5\" maxlength=\"5\" onkeyup=\"validatePorts(%d,false);\" onkeypress=\"enableApply();\" value=\"%s\" >\n"
				 "</td>\n"
				 "</tr>\n"
				 "<tr>\n"
				 "<td width=\"50\" height=\"3\"><img src=\"images/clearpixel.gif\" width=\"3\" height=\"3\"></td>\n"
				 "<td width=\"90\" height=\"3\"><img src=\"images/clearpixel.gif\" width=\"3\" height=\"3\"></td>\n"
				 "<td width=\"110\" height=\"3\"><img src=\"images/clearpixel.gif\" width=\"3\" height=\"3\"></td>\n"
				 "<td width=\"70\" height=\"3\"><img src=\"images/clearpixel.gif\" width=\"3\" height=\"3\"></td>\n"
				 "<td width=\"120\" height=\"3\"><img src=\"images/clearpixel.gif\" width=\"3\" height=\"3\"></td>\n"
				 "<td height=\"3\"><img src=\"images/clearpixel.gif\" width=\"3\" height=\"3\"></td>\n"
				 "</tr>\n",
				 start, start, start, port);
		start++;
	}

	return ret;
}

static int
ej_mac_filters(int eid, webs_t wp, int argc, char_t **argv)
{
	int start, end, i;
	char buf[100], *value;
	unsigned char mac[ETHER_ADDR_LEN];
	char *ip_allow, *wl_allow;
	int ret = 0;

	if (ejArgs(argc, argv, "%d %d", &start, &end) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	while (start <= end) {
		/* Parse xx:xx:xx:xx:xx:xx[:allow|deny] */
		snprintf(buf, sizeof(buf), "wl_mac%d", start);
		value = nvram_safe_get(buf);
		wl_allow = strstr(value, "allow");
		snprintf(buf, sizeof(buf), "filter_mac%d", start);
		value = nvram_safe_get(buf);
		ip_allow = strstr(value, "allow");
		if (ether_atoe(value, mac)) {
			snprintf(buf, sizeof(buf), "%02X%c%02X%c%02X%c%02X%c%02X%c%02X",
				 mac[0], 0, mac[1], 0, mac[2], 0, mac[3], 0, mac[4], 0, mac[5]);
		} else
			memset(buf, 0, sizeof(buf));

		ret += websWrite(wp,
				 "<tr>\n"
				 "<td width=\"355\">\n");
		for (i = 1; i <= 6; i++) {
			ret += websWrite(wp,
					 "<input type=\"text\" name=\"MACAddr%d_%d\" id=\"MACAddr%d_%d\" size=\"2\"  maxlength=\"2\" onkeyup=\"enableApply();MACChangeForRows('MACAddr',%d,%d);\" value=\"%s\">\n"
					 "%s\n",
					 i, start, i, start, i, start, &buf[(i - 1) * 3], i == 6 ? "" : "-");
		}
		ret += websWrite(wp,
				 "</td>\n");
		ret += websWrite(wp,
				 "<td width=\"150\">\n"
				 "<input type=\"checkbox\" name=\"allowConnOnAddr%d\" id=\"allowConnOnAddr%d\" onclick=\"enableApply();\" %s>\n"
				 "</td>\n",
				 start, start, ip_allow ? "checked" : "");
		ret += websWrite(wp,
				 "<td>\n"
				 "<input type=\"checkbox\" name=\"allowAssocOnAddr%d\" id=\"allowAssocOnAddr%d\" onclick=\"enableApply();\" %s>\n"
				 "</td>\n"
				 "</tr>\n"
				 "<tr>\n"
				 "<td width=\"355\" height=\"10\"><img src=\"images/clearpixel.gif\" width=\"12\" height=\"12\"></td>\n"
				 "<td width=\"150\" height=\"10\"><img src=\"images/clearpixel.gif\" width=\"12\" height=\"12\"></td>\n"
				 "<td height=\"10\"><img src=\"images/clearpixel.gif\" width=\"12\" height=\"12\"></td>\n"
				 "</tr>\n",
				 start, start, wl_allow ? "checked" : "");
		start++;
	}

	return ret;
}

static int
ej_client_filters(int eid, webs_t wp, int argc, char_t **argv)
{
	int start, end, i;
	char *days[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
	char buf[100];
	struct in_addr in;
	int ret = 0;
	netconf_filter_t from, to;
	bool valid;
	bool always;

	if (ejArgs(argc, argv, "%d %d", &start, &end) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	while (start <= end) {
		valid = get_filter_client(start, &from, &to);
		
		/* blocked */
		ret += websWrite(wp,
				 "<tr>\n"
				 "<td width=\"16\">&nbsp;</td>\n"
				 "<td>\n"
				 "<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\" class=\"bubbleTable\" ID=\"Table1\">\n"
				 "<tr>\n"
				 "<td width=\"12\" class=\"background\" height=\"10\"><img src=\"images/corner-UL.gif\" width=\"12\" height=\"12\"></td>\n"
				 "<td height=\"10\"><img src=\"images/clearpixel.gif\" width=\"12\" height=\"12\"></td>\n"
				 "<td width=\"12\" class=\"background\" height=\"10\"><img src=\"images/corner-UR.gif\" width=\"12\" height=\"12\"></td>\n"
				 "</tr>\n"
				 "<tr>\n"
				 "<td width=\"12\">&nbsp;</td>\n"
				 "<td class=\"normalText\">\n"
				 "<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\" ID=\"Table1\">\n"
				 "<tr>\n"
				 "<td valign=\"center\" width=\"70\">\n"
				 "<input type=\"checkbox\" name=\"filterCheckbox%d\" value=\"1\" onClick=\"enableApply();\" ID=\"Checkbox%d\" %s>\n"
				 "<span class=\"normalText\">Block </span></td>\n",
				 start, start,
				 !valid || (from.match.flags&NETCONF_DISABLED) ? "" : "checked");
		/* from IP */
		in.s_addr = ntohl(inet_addr(nvram_safe_get("lan_ipaddr")));
		if (valid && (from.match.src.ipaddr.s_addr&0xff)) {
			snprintf(buf, sizeof(buf), "%u", (unsigned char)from.match.src.ipaddr.s_addr&0xff);
		}
		else {
			*buf = '\0';
		}
		ret += websWrite(wp,
				 "<td valign=\"top\">\n"
				 "<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\" ID=\"Table18\">\n"
				 "<tr valign=\"center\">\n"
				 "<td><span class=\"normalText\">%d. %d. %d. </span>\n"
				 "<input type=\"text\" name=\"ipRangeLow%d\" id=\"ipRangeLow%d\" size=\"3\" maxlength=\"3\" onKeyPress=\"enableApply();\" value=\"%s\">\n"
				 "-\n",
				 (int)((in.s_addr >> 24) & 0xff), (int)((in.s_addr >> 16) & 0xff), (int)((in.s_addr >> 8) & 0xff),
				 start, start, buf);
		/* to IP */
		if (valid && (ntohl(to.match.src.ipaddr.s_addr)&0xff)) {
			snprintf(buf, sizeof(buf), "%u", (unsigned char)ntohl(to.match.src.ipaddr.s_addr)&0xff);
		}
		else {
			*buf = '\0';
		}
		ret += websWrite(wp,
				 "<input type=\"text\" name=\"ipRangeHigh%d\" id=\"ipRangeHigh%d\" size=\"3\" maxlength=\"3\" onKeyPress=\"enableApply();\" value=\"%s\">\n"
				 "</td>\n",
				 start, start, buf);
		/* from port */
		if (valid && from.match.dst.ports[0]) {
			snprintf(buf, sizeof(buf), "%d", (unsigned short)ntohs(from.match.dst.ports[0]));
		}
		else {
			*buf = '\0';
		}
		ret += websWrite(wp,
				 "<td><span class=\"normalText\">Outbound port(s):</span>\n"
				 "<input type=\"text\" name=\"portRangeLow%d\" id=\"portRangeLow%d\" size=\"5\" maxlength=\"5\" onKeyPress=\"enableApply();\" value=\"%s\">\n"
				 "-\n",
				 start, start, buf);
		/* to port */
		if (valid && to.match.dst.ports[1]) {
			snprintf(buf, sizeof(buf), "%d", (unsigned short)ntohs(to.match.dst.ports[1]));
		}
		else {
			*buf = '\0';
		}
		ret += websWrite(wp,
				 "<input type=\"text\" name=\"portRangeHigh%d\" id='=\"portRangeHigh%d\"' size=\"5\" maxlength=\"5\" onKeypress=\"enableApply();\" value=\"%s\">\n"
				 "</td>\n",
				 start, start, buf);
		/* protocol */
		ret += websWrite(wp,
				 "<td><span class=\"normalText\">Protocol:</span>\n"
				 "<select name=\"protocol%d\" onChange=\"enableApply();\" ID=\"protocol%d\">\n"
				 "<option value=\"1\" %s>TCP</option>\n"
				 "<option value=\"2\" %s>UDP</option>\n"
				 "</select>\n"
				 "</td>\n"
				 "</tr>\n"
				 "</table>\n"
				 "</td>\n"
				 "</tr>\n"
				 "<tr>\n"
				 "<td valign=\"top\" width=\"70\">&nbsp;</td>\n"
				 "<td>\n"
				 "<hr noshade width=\"100%%\" size=\"4\" color=\"#d6dff5\">\n"
				 "</td>\n"
				 "</tr>\n"
				 "<tr>\n"
				 "<td valign=\"top\" width=\"70\">&nbsp;</td>\n"
				 "<td valign=\"center\">\n",
				 start, start,
				 valid && from.match.ipproto == IPPROTO_TCP ? "selected" : "",
				 valid && from.match.ipproto == IPPROTO_UDP ? "selected" : "");
		/* always */
		always = from.match.secs[0] == 0 && to.match.secs[1] == 0;
		ret += websWrite(wp,
				 "<input type=\"radio\" name=\"whenRadio%d\" value=\"0\" onClick=\"toggleTimeEnabled(%d,true);enableApply();\" ID=\"Radio1\" %s>\n"
				 "<span class=\"selectedRadioDesc\" id=\"alwaysLabel%d\">Always</span>\n"
				 "</td>\n"
				 "</tr>\n"
				 "<tr>\n"
				 "<td valign=\"top\" width=\"70\" height=\"5\"><img src=\"images/clearpixel.gif\" width=\"5\" height=\"5\"></td>\n"
				 "<td valign=\"center\" height=\"5\"><img src=\"images/clearpixel.gif\" width=\"5\" height=\"5\"></td>\n"
				 "</tr>\n"
				 "<tr>\n"
				 "<td valign=\"top\" width=\"70\">&nbsp;</td>\n"
				 "<td valign=\"center\">\n",
				 start, start, always ? "checked" : "",
				 start);
		/* from day of the week */
		ret += websWrite(wp,
				 "<input type=\"radio\" name=\"whenRadio%d\" value=\"1\" onClick=\"toggleTimeEnabled(%d,false);enableApply();\" ID=\"Radio2\" %s>\n"
				 "<span class=\"normalText\" id=\"fromLabel%d\">From\n"
				 "<select name=\"startDay%d\" id=\"startDay%d\" size=\"1\" onChange=\"enableApply();\">\n",
				 start, start, always ? "" : "checked",
				 start,
				 start, start);
		for (i = 0; i < 7; i++) {
			ret += websWrite(wp,
					 "<option value=\"%d\" %s>%s</option>\n",
					 i, i == from.match.days[0] ? "selected" : "", days[i]);
		}
		/* to day of the week */
		ret += websWrite(wp,
				 "</select>\n"
				 "<span class=\"normalText\" id=\"toLabel%d\">to:\n"
				 "<select name=\"endDay%d\" id=\"endDay%d\" size=\"1\" onChange=\"enableApply();\">\n",
				 start,
				 start, start);
		for (i = 0; i < 7; i++) {
			ret += websWrite(wp,
					 "<option value=\"%d\" %s>%s</option>\n",
					 i, i == to.match.days[1] ? "selected" : "", days[i]);
		}
		/* from time of the day */
		ret += websWrite(wp,
				 "</select>\n"
				 "<span class=\"normalText\" id=\"betweenLabel%d\">between\n"
				 "<select name=\"startTime%d\" id=\"startTime%d\" size=\"1\" onChange=\"enableApply();\">\n",
				 start,
				 start, start);
		for (i = 0; i < 24; i++) {
			ret += websWrite(wp,
					 "<option value=\"%d\" %s>%d:00 %c.M.</option>\n",
					 i * 60 * 60, i * 60 * 60 == from.match.secs[0] ? "selected" : "",
					 (i % 12) ? : 12, i > 11 ? 'P' : 'A');
		}
		/* to time of the day */
		ret += websWrite(wp,
				 "</select>\n"
				 "<span class=\"normalText\" id=\"andLabel%d\">and\n"
				 "<select name=\"endTime%d\" id=\"endTime%d\" size=\"1\" onChange=\"enableApply();\">\n",
				 start,
				 start, start);
		for (i = 0; i < 24; i++) {
			ret += websWrite(wp,
					 "<option value=\"%d\" %s>%d:00 %c.M.</option>\n",
					 i * 60 * 60, i * 60 * 60 == to.match.secs[1] ? "selected" : "",
					 (i % 12) ? : 12, i > 11 ? 'P' : 'A');
		}
		/* finish the web page */
		ret += websWrite(wp,
				 "</select>\n"
				 "</span></span>\n"
				 "</span></span>\n"
				 "</td>\n"
				 "</tr>\n"
				 "</table>\n"
				 "</td>\n"
				 "<td width=\"12\">&nbsp;</td>\n"
				 "</tr>\n"
				 "<tr>\n"
				 "<td width=\"12\" valign=\"bottom\" align=\"left\" height=\"10\" class=\"background\"><img src=\"images/corner-LL.gif\" width=\"12\" height=\"12\"></td>\n"
				 "<td height=\"10\"><img src=\"images/clearpixel.gif\" width=\"12\" height=\"12\"></td>\n"
				 "<td width=\"12\" valign=\"bottom\" align=\"right\" height=\"10\" class=\"background\"><img src=\"images/corner-LR.gif\" width=\"12\" height=\"12\"></td>\n"
				 "</tr>\n"
				 "</table>\n"
				 "</td>\n"
				 "</tr>\n"
				 "<tr>\n"
				 "<td width=\"16\" height=\"12\"><img src=\"images/clearpixel.gif\" width=\"12\" height=\"12\"></td>\n"
				 "<td height=\"12\"><img src=\"images/clearpixel.gif\" width=\"12\" height=\"12\"></td>\n"
				 "</tr>\n");
		start++;
	}

	return ret;
}

static struct {
	struct in_addr addr;
	time_t expires;
} current_user = { { 0 }, 0 };

static int
ej_current_ip(int eid, webs_t wp, int argc, char_t **argv)
{
	int byte = -1;

	ejArgs(argc, argv, "%d", &byte);

	if (byte >= 4) {
		websError(wp, 400, "Invalid byte\n");
		return -1;
	}

	if (byte != -1)
		return websWrite(wp, "%d", (int)((ntohl(current_user.addr.s_addr) >> ((3 - byte) * 8)) & 0xff));
	else
		return websWrite(wp, inet_ntoa(current_user.addr));
}

static int
websRefresh(webs_t wp, char *href)
{
	websHeader(wp);
	websWrite(wp, "<head><meta http-equiv=refresh content='0; url=%s'></head>", href);
	websFooter(wp);
	websDone(wp, 200);

	return 1;
}

static int
websAlert(webs_t wp, char *msg, char *href)
{
	websHeader(wp);
	websWrite(wp, "<body><script language=JavaScript>\nalert(\"%s\")</script></body>\n", msg);
	if (href && *href)
	{
		websWrite(wp, "<head><meta http-equiv=refresh content='0; url=%s'></head>\n", href);
	}
	websFooter(wp);
	websDone(wp, 200);

	return 1;
}

static int
login_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	  char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	char *pws = websGetVar(wp, "pws", "");
	struct sockaddr_in peer;
	socklen_t len;
	int s;
	time_t now;
	char sa[20];

#ifdef WEBS
	s = wp->sid;
#else
	s = fileno(wp);
#endif
	if (getpeername(s, (struct sockaddr *) &peer, &len) < 0) {
		websError(wp, 400, strerror(errno));
		return -1;
	}
	inet_ntop(AF_INET, &peer.sin_addr, sa, 20);

	if (!strcmp(page, "login")) {
		if (nvram_invmatch("http_passwd", pws)) {
			log_msg(-1, -1, "%s login failed, invalid password\n", sa);
			return websRefresh(wp, "/login_error.htm");
		}
		else {
			time(&now);
			current_user.expires = now + (atoi(nvram_safe_get("pwTimeout")) * 60);
			current_user.addr.s_addr = peer.sin_addr.s_addr;
			log_msg(-1, -1, "%s login successful", sa);
			return websRefresh(wp, "/home.htm");
		}
	} else if (!strcmp(page, "logout")) {
		current_user.expires = 0;
		current_user.addr.s_addr = 0;
		log_msg(-1, -1, "%s logout", sa);
	}

	/* This function handles multiple default paths */
	return websDefaultHandler(wp, urlPrefix, webDir, arg, url, "login.htm", query);
}

static int
home_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg,
	 char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	char *connectflag;

	if (!strcmp(page, "status")) {
		connectflag = websGetVar(wp, "connectflag", "");

		/* PPPoE connect */
		if (!strcmp(connectflag, "1")) {
			pppoe_up();
		}
		/* PPPoE disconnect */
		else if (!strcmp(connectflag, "2")) {
			pppoe_down();
		}
		/* DHCP release */
		else if (!strcmp(connectflag, "3")) {
			sys_release();
			nvram_unset("wan_ipaddr");
			nvram_unset("wan_netmask");
			nvram_unset("wan_gateway");
			nvram_unset("wan_dns");
		}
		/* DHCP renew */
		else if (!strcmp(connectflag, "4"))
			sys_renew();
	}

	return websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
}

static int
reset_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	  char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	int ret;

	if (!strcmp(page, "system_reset")) {
		ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, "reset_success.htm", query);
		sys_reboot();
	} else
		ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);

	return ret;
}

static unsigned char nvram[NVRAM_SPACE];

static int
config_bin(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	   char_t *url, char_t *path, char_t *query)
{
	int count = NVRAM_SPACE;
	nvram_dump(nvram, &count);
	websWriteDataNonBlock(wp, nvram, count);
	websDone(wp, 200);

	return 1;
}

static int
backup_restore_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
		   char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	int ret;

	if (!strcmp(page, "system_backup")) {
		/* Restore Base Station Settings from a Backup */
		ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, "tools_restore_return.htm", query);
		sys_reboot();
	} else if (!strcmp(page, "system_restore")) {
		/* Restore Factory Default Settings */
		nvram_set("restore_defaults", "1");
		nvram_commit();
		ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, "backup_success.htm", query);
		sys_reboot();
	} else if (!strcmp(page, "restore_backup_failed")) {
		ret = websAlert(wp, "Installation was unsuccessful.\\n\\n"
				"Make sure you have the correct file, and then try again.",
			"/backup_restore.htm");
	} else
		ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, "backup_restore.htm", query);

	return ret;
}

static int
firmware_upgrade_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
		     char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	int ret;

	if (!strcmp(page, "system_firm")) {
		ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, "upgrade_firm_return.htm", query);
		sys_reboot();
	} else if (!strcmp(page, "upgrade_firmware_failed")) {
		ret = websAlert(wp, "Installation was unsuccessful.\\n\\n"
				"Make sure you have the correct file, and then try again.",
			"/firmware_upgrade.htm");
	} else
		ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);

	return ret;
}

static const int jdays[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

/* Jn - Julian day, 1 == January 1, 60 == March 1 even in leap years */
static
int J1(int month, int day)
{
	return jdays[month - 1] + day;
}

static int
time_settings_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
		  char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	char *timeZone = websGetVar(wp, "timeZone", "0");
	char *dlsAutoAdjust = websGetVar(wp, "dlsAutoAdjust", NULL);
	char *startMonth = websGetVar(wp, "startMonth", "0");
	char *startDay = websGetVar(wp, "startDay", "0");
	char *endMonth = websGetVar(wp, "endMonth", "0");
	char *endDay = websGetVar(wp, "endDay", "0");
	char *tzs[] = {
		"",
		"GMT12", "GMT11", "GMT10", "GMT9", "GMT8",
		"GMT7", "GMT7", "GMT6", "GMT6", "GMT6",
		"GMT5", "GMT5", "GMT5", "GMT4", "GMT4",
		"GMT4", "GMT3:30", "GMT3", "GMT3", "GMT2",
		"GMT1", "GMT", "GMT", "GMT", "GMT-1",
		"GMT-1", "GMT-1", "GMT-1", "GMT-1", "GMT-1",
		"GMT-2", "GMT-2", "GMT-2", "GMT-2", "GMT-2",
		"GMT-2", "GMT-3", "GMT-3", "GMT-3:30", "GMT-4",
		"GMT-4", "GMT-4:30", "GMT-5", "GMT-5:30", "GMT-5:45",
		"GMT-6", "GMT-6:30", "GMT-7", "GMT-8", "GMT-8",
		"GMT-9", "GMT-9:30", "GMT-10", "GMT-10", "GMT-10",
		"GMT-10", "GMT-11", "GMT-12", "GMT-12", "GMT-13"
	};
	char buf[1000], *cur;
	char ip[] = "XXX.XXX.XXX.XXX";
	struct in_addr in;
	char word[256], *next;
	char *serverIPList;
	int restart = 0, ret;
	char *ntps;
	int len;
	char fmt[32];
	int i;

	if (!strcmp(page, "system_time")) {
		if (atoi(timeZone) >= 1 || atoi(timeZone) <= 60) {
			if (dlsAutoAdjust &&
			    valid_range(startMonth, startMonth, 1, 12) &&
			    valid_range(startDay, startDay, 1, 31) &&
			    valid_range(endMonth, endMonth, 1, 12) &&
			    valid_range(endDay, endDay, 1, 31)) {
				nvram_set("dlsAutoAdjust", "1");
				snprintf(buf, sizeof(buf), "%sDST,J%d,J%d",
					 tzs[atoi(timeZone)],
					 J1(atoi(startMonth), atoi(startDay)),
					 J1(atoi(endMonth), atoi(endDay)));
			} else {
				nvram_set("dlsAutoAdjust", "0");
				strncpy(buf, tzs[atoi(timeZone)], sizeof(buf));
			}
			nvram_set("timeZone", timeZone);
			nvram_set("startMonth", startMonth);
			nvram_set("startDay", startDay);
			nvram_set("endMonth", endMonth);
			nvram_set("endDay", endDay);
			nvram_set("time_zone", buf);
		}

		/* check what to do, add or remove */
		/*
		* Some browsers (Netscape) do not post Add and Remove variables with the request (#1) and 
		* some browsers (IE6) do (#2) and some browsers post the same request twice (IE6) (#3).
		* Therefore what we can do is to not check Add and Remove variables. Doing so will satisfy
		* #1 and #2. We also need to check duplicates to address #3.
		*/
		/*if (strcmp(websGetVar(wp, "Add", ""), "Add >>") == 0)*/
		{
			/* Add an address */
			snprintf(ip, sizeof(ip), "%s.%s.%s.%s",
				 websGetVar(wp, "newIP1", ""), websGetVar(wp, "newIP2", ""),
				 websGetVar(wp, "newIP3", ""), websGetVar(wp, "newIP4", ""));
			if (valid_ipaddr(ip, &in)) {
				/* check for duplicate */
				ntps = nvram_safe_get("ntp_server");
				len = strlen(ip);
				cur = ntps;
				while ((cur = strstr(cur, ip)) != NULL && 
						((cur > ntps && *(cur-1) != ' ') || (*(cur+len) != ' ' && *(cur+len) != '\0')))
					cur += len;
				/* add */
				if (cur == NULL) {
					snprintf(buf, sizeof(buf), "%s%s%s", ntps, *ntps != 0 ? " " : "", ip);
					nvram_set("ntp_server", buf);
				}
			}
		}
		/*else if (strcmp(websGetVar(wp, "Remove", ""), "<< Remove") == 0)*/
		{
			/* Remove multiple addresses */
			for (i = 0; (serverIPList = websGetVarAt(wp, "serverIPList", i, NULL)) != NULL; i ++) {
				foreach(word, serverIPList, next) {
					/* check for existance */
					ntps = nvram_safe_get("ntp_server");
					len = strlen(word);
					cur = ntps;
					while ((cur = strstr(cur, word)) != NULL &&
							((cur > ntps && *(cur-1) != ' ') || (*(cur+len) != ' ' && *(cur+len) != '\0')))
						cur += len;
					/* remove */
					if (cur != NULL) {
						snprintf(fmt, sizeof(fmt), "%%.%ds%%s%%s", (int)(cur > ntps ? cur-ntps-1 : 0));
						snprintf(buf, sizeof(buf), fmt, ntps, cur > ntps && *(cur+len) == ' ' ? " " : "", 
							*(cur+len) == ' ' ? cur+len+1 : cur+len);
						nvram_set("ntp_server", buf);
					}
				}
			}
		}
		nvram_commit();
		restart = 1;
	}

	ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
	if (restart)
		sys_restart();
	return ret;
}

static int
change_password_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
		    char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	char *userNewPswd = websGetVar(wp, "userNewPswd", "");
	char *timeout = websGetVar(wp, "timeout", "");
	int ret;

	if (!strcmp(page, "change_password")) {
		if (atoi(timeout) > 0)
			nvram_set("pwTimeout", timeout);
		if (strlen(userNewPswd) >= 3)
			nvram_set("http_passwd", userNewPswd);
		nvram_commit();
		ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, "system_passwordok.htm", query);
		sys_restart();
	} else
		ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);

	return ret;
}

static int
local_network_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
		  char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	char ip[] = "XXX.XXX.XXX.XXX";
	struct in_addr in;
	int leaseTime;
	int restart = 0, ret;

	if (!strcmp(page, "local_network")) {
		snprintf(ip, sizeof(ip), "%s.%s.%s.%s",
			 websGetVar(wp, "IP1", ""), websGetVar(wp, "IP2", ""), 
			 websGetVar(wp, "IP3", ""), websGetVar(wp, "IP4", ""));
		if (valid_ipaddr(ip, &in))
			nvram_set("lan_ipaddr", ip);
		if (!strcmp(websGetVar(wp, "DHCPEnabled", ""), "1"))
			nvram_set("lan_proto", "dhcp");
		else if (!strcmp(websGetVar(wp, "DHCPEnabled", ""), "2"))
			nvram_set("lan_proto", "static");
		snprintf(ip, sizeof(ip), "%s.%s.%s.%s",
			 websGetVar(wp, "DHCPStart1", websGetVar(wp, "IP1", "")), websGetVar(wp, "DHCPStart2", websGetVar(wp, "IP2", "")),
			 websGetVar(wp, "DHCPStart3", websGetVar(wp, "IP3", "")), websGetVar(wp, "DHCPStart4", ""));
		if (valid_ipaddr(ip, &in))
			nvram_set("dhcp_start", ip);
		snprintf(ip, sizeof(ip), "%s.%s.%s.%s",
			 websGetVar(wp, "DHCPEnd1", websGetVar(wp, "IP1", "")), websGetVar(wp, "DHCPEnd2", websGetVar(wp, "IP2", "")),
			 websGetVar(wp, "DHCPEnd3", websGetVar(wp, "IP3", "")), websGetVar(wp, "DHCPEnd4", ""));
		if (valid_ipaddr(ip, &in))
			nvram_set("dhcp_end", ip);
		leaseTime = atoi(websGetVar(wp, "leaseTime", ""));
		if (leaseTime == 900 || leaseTime == 1800 ||
		    leaseTime == 7200 || leaseTime == 86400 ||
		    leaseTime == 259200 || leaseTime == 604800 ||
		    leaseTime == 1209600)
			nvram_set("lan_lease", websGetVar(wp, "leaseTime", ""));
		nvram_set("lan_domain", websGetVar(wp, "DHCPServerName", ""));
		nvram_commit();
		restart = 1;
	}

	ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
	if (restart)
		sys_restart();
	return ret;
}

static int
valid_hwaddr(char *value)
{
	unsigned int hwaddr[6];

	/* Check for bad, multicast, broadcast, or null address */
	if (sscanf(value, "%x:%x:%x:%x:%x:%x",
		   &hwaddr[0], &hwaddr[1], &hwaddr[2],
		   &hwaddr[3], &hwaddr[4], &hwaddr[5]) != 6 ||
	    (hwaddr[0] & 1) ||
	    (hwaddr[0] & hwaddr[1] & hwaddr[2] & hwaddr[3] & hwaddr[4] & hwaddr[5]) == 0xff ||
	    (hwaddr[0] | hwaddr[1] | hwaddr[2] | hwaddr[3] | hwaddr[4] | hwaddr[5]) == 0x00) {
		return FALSE;
	}

	return TRUE;
}

static int
wide_area_network_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
		      char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	char *radiovalue;
	char ip[] = "XXX.XXX.XXX.XXX";
	struct in_addr in;
	char buf[1000], *cur;
	int restart = 0, ret;

	if (!strcmp(page, "wide_area_network")) {
		radiovalue = websGetVar(wp, "radiovalue", "");

		/* PPPoE */
		if (!strcmp(radiovalue, "2")) {
			nvram_set("wan_proto", "pppoe");
			nvram_set("pppoe_username", websGetVar(wp, "UN", ""));
			nvram_set("pppoe_passwd", websGetVar(wp, "PW", ""));
			if (atoi(websGetVar(wp, "idle", "")) <= 9999)
				nvram_set("pppoe_idletime", websGetVar(wp, "idle", ""));
			if (websGetVar(wp, "reconnect", NULL))
				nvram_set("pppoe_keepalive", "1");
			else
				nvram_set("pppoe_keepalive", "0");
			nvram_set("pppoe_service", websGetVar(wp, "SN", ""));

			/* Manually specified DNS addresses */
			if (!strcmp(websGetVar(wp, "pppoeDNSSelect", ""), "2"))
				nvram_set("dhcp_dns", "lan");
			else
				nvram_set("dhcp_dns", "wan");
			cur = buf;
			*cur = '\0';
			snprintf(ip, sizeof(ip), "%s.%s.%s.%s",
				 websGetVar(wp, "pppoeDIP11", ""), websGetVar(wp, "pppoeDIP12", ""),
				 websGetVar(wp, "pppoeDIP13", ""), websGetVar(wp, "pppoeDIP14", ""));
			if (valid_ipaddr(ip, &in))
				cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s",
						cur == buf ? "" : " ",
						inet_ntoa(in));
			snprintf(ip, sizeof(ip), "%s.%s.%s.%s",
				 websGetVar(wp, "pppoeDIP21", ""), websGetVar(wp, "pppoeDIP22", ""),
				 websGetVar(wp, "pppoeDIP23", ""), websGetVar(wp, "pppoeDIP24", ""));
			if (valid_ipaddr(ip, &in))
				cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s",
						cur == buf ? "" : " ",
						inet_ntoa(in));
			nvram_set("lan_dns", buf);
		}
	
		/* Dynamic */
		else if (!strcmp(radiovalue, "4")) {
			nvram_set("wan_proto", "dhcp");
			nvram_set("wan_hostname", websGetVar(wp, "txtHost", ""));
			snprintf(buf, sizeof(buf), "%s:%s:%s:%s:%s:%s",
				 websGetVar(wp, "MACaddr1", ""), websGetVar(wp, "MACaddr2", ""), websGetVar(wp, "MACaddr3", ""),
				 websGetVar(wp, "MACaddr4", ""), websGetVar(wp, "MACaddr5", ""), websGetVar(wp, "MACaddr6", ""));
			if (valid_hwaddr(buf))
				nvram_set("wan_hwaddr", buf);

			/* Manually specified DNS addresses */
			if (!strcmp(websGetVar(wp, "cboObtainAuto", ""), "2"))
				nvram_set("dhcp_dns", "lan");
			else
				nvram_set("dhcp_dns", "wan");
			cur = buf;
			*cur = '\0';
			snprintf(ip, sizeof(ip), "%s.%s.%s.%s",
				 websGetVar(wp, "txtPriDNS1", ""), websGetVar(wp, "txtPriDNS2", ""),
				 websGetVar(wp, "txtPriDNS3", ""), websGetVar(wp, "txtPriDNS4", ""));
			if (valid_ipaddr(ip, &in))
				cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s",
						cur == buf ? "" : " ",
						inet_ntoa(in));
			snprintf(ip, sizeof(ip), "%s.%s.%s.%s",
				 websGetVar(wp, "txtSecDNS1", ""), websGetVar(wp, "txtSecDNS2", ""),
				 websGetVar(wp, "txtSecDNS3", ""), websGetVar(wp, "txtSecDNS4", ""));
			if (valid_ipaddr(ip, &in))
				cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s",
						cur == buf ? "" : " ",
						inet_ntoa(in));
			nvram_set("lan_dns", buf);
		}

		/* Static */
		else if (!strcmp(radiovalue, "1")) {
			nvram_set("wan_proto", "static");
			snprintf(ip, sizeof(ip), "%s.%s.%s.%s",
				 websGetVar(wp, "IP1", ""), websGetVar(wp, "IP2", ""),
				 websGetVar(wp, "IP3", ""), websGetVar(wp, "IP4", ""));
			if (valid_ipaddr(ip, &in))
				nvram_set("wan_ipaddr", ip);
			snprintf(ip, sizeof(ip), "%s.%s.%s.%s",
				 websGetVar(wp, "NM1", ""), websGetVar(wp, "NM2", ""),
				 websGetVar(wp, "NM3", ""), websGetVar(wp, "NM4", ""));
			if (valid_ipaddr(ip, &in))
				nvram_set("wan_netmask", ip);
			snprintf(ip, sizeof(ip), "%s.%s.%s.%s",
				 websGetVar(wp, "GIP1", ""), websGetVar(wp, "GIP2", ""),
				 websGetVar(wp, "GIP3", ""), websGetVar(wp, "GIP4", ""));
			if (valid_ipaddr(ip, &in))
				nvram_set("wan_gateway", inet_ntoa(in));

			/* Manually specified DNS addresses */
			nvram_set("dhcp_dns", "lan");
			cur = buf;
			*cur = '\0';
			snprintf(ip, sizeof(ip), "%s.%s.%s.%s",
				 websGetVar(wp, "txtStaticPriDNS1", ""), websGetVar(wp, "txtStaticPriDNS2", ""),
				 websGetVar(wp, "txtStaticPriDNS3", ""), websGetVar(wp, "txtStaticPriDNS4", ""));
			if (valid_ipaddr(ip, &in))
				cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s",
						cur == buf ? "" : " ",
						inet_ntoa(in));
			snprintf(ip, sizeof(ip), "%s.%s.%s.%s",
				 websGetVar(wp, "txtStaticSecDNS1", ""), websGetVar(wp, "txtStaticSecDNS2", ""),
				 websGetVar(wp, "txtStaticSecDNS3", ""), websGetVar(wp, "txtStaticSecDNS4", ""));
			if (valid_ipaddr(ip, &in))
				cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s",
						cur == buf ? "" : " ",
						inet_ntoa(in));
			nvram_set("lan_dns", buf);
		}

		/* Disabled */
		else
			nvram_set("wan_proto", "disabled");

#ifdef linux
		if (nvram_match("wan_proto", "pppoe"))
			nvram_set("wan_ifname", "ppp0");
		else
			nvram_set("wan_ifname", nvram_get("pppoe_ifname"));
#endif

		nvram_commit();
		restart = 1;
	}

	ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
	if (restart)
		sys_restart();
	return ret;
}

static int
wireless_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	     char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	char *value;
	int restart = 0, ret;

	if (!strcmp(page, "wireless_id")) {
		if (websGetVar(wp, "enableWirelessCheckbox", NULL))
			nvram_set("wl_radio", "1");
		else
			nvram_set("wl_radio", "0");
		if ((value = websGetVar(wp, "channel", NULL)) &&
		    atoi(value) >= 1 && atoi(value) <= 14) {
			nvram_set("d11b_channel", value);
			nvram_set("d11g_channel", value);
		}
		if ((value = websGetVar(wp, "ssid", NULL)))
			nvram_set("wl_ssid", value);
		if ((value = websGetVar(wp, "txrate", NULL))) {
			switch (wl_phytype()) {
			case PHY_TYPE_B:
				nvram_set("d11b_rate", value);
				break;
			case PHY_TYPE_G:
				nvram_set("d11g_rate", value);
				break;
			}
		}
		if ((value = websGetVar(wp, "d11g_mode", NULL)) &&
		    atoi(value) >= 0 && atoi(value) <= 5)
			nvram_set("d11g_mode", value);
		nvram_commit();
		restart = 1;
	}

	ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
	if (restart)
		sys_restart();
	return ret;
}

static int
wireless_encryption_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
			char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	char *value;
	int restart = 0, ret;

	if (!strcmp(page, "wireless_enc")) {
		value = websGetVar(wp, "enctype", "");
		if (!strcmp(value, "1"))
			nvram_set("wl_wep", "restricted");
		else
			nvram_set("wl_wep", "off");
		nvram_set("manuallyEncType", websGetVar(wp, "manuallyEncType", "2"));
		if ((value = websGetVar(wp, "key1", NULL)))
			nvram_set("wl_key1", value);
		if ((value = websGetVar(wp, "key2", NULL)))
			nvram_set("wl_key2", value);
		if ((value = websGetVar(wp, "key3", NULL)))
			nvram_set("wl_key3", value);
		if ((value = websGetVar(wp, "key4", NULL)))
			nvram_set("wl_key4", value);
		if ((value = websGetVar(wp, "manuallyDefaultKey", NULL)) &&
		    atoi(value) >= 1 && atoi(value) <= 4)
			nvram_set("wl_key", value);
		nvram_commit();
		restart = 1;
	}

	ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
	if (restart)
		sys_restart();
	return ret;
}

static int
network_address_translation_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
				char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	int restart = 0, ret;

	if (!strcmp(page, "system_main")) {
		if (!strcmp(websGetVar(wp, "NM", ""), "0"))
			nvram_set("router_disable", "1");
		else
			nvram_set("router_disable", "0");
		nvram_commit();
		restart = 1;
	}

	ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
	if (restart)
		sys_restart();
	return ret;
}

static int
hacker_protection_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
		      char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	int restart = 0, ret;

	if (!strcmp(page, "firewall")) {
		if (!websGetVar(wp, "EnableFirewall", NULL))
			nvram_set("fw_disable", "1");
		else
			nvram_set("fw_disable", "0");
		if (!websGetVar(wp, "ping", NULL))
			nvram_set("wan_ping_disabled", "0");
		else
			nvram_set("wan_ping_disabled", "1");
		nvram_commit();
		restart = 1;
	}

	ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
	if (restart)
		sys_restart();
	return ret;
}

static int
app_triggered_port_forwarding_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
			       char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	int i;
	char name[100];
	char *enable, *desc;
	char *triggerPort, *triggerType, *publicPort, *publicType;
	int restart = 0, ret;
	netconf_app_t app;

	if (!strcmp(page, "firewall_specialapp")) {
		for (i = 1; i <= MAX_NVPARSE; i++) {
			snprintf(name, sizeof(name), "enableCheck%d", i);
			if (websGetVar(wp, name, NULL))
				enable = "on";
			else
				enable = "off";
			snprintf(name, sizeof(name), "descText%d", i);
			desc = websGetVar(wp, name, "");
			snprintf(name, sizeof(name), "triggerPortText%d", i);
			triggerPort = websGetVar(wp, name, "");
			snprintf(name, sizeof(name), "triggerType%d", i);
			if (!strcmp(websGetVar(wp, name, ""), "2"))
				triggerType = "udp";
			else
				triggerType = "tcp";
			snprintf(name, sizeof(name), "publicPorts%d", i);
			publicPort = websGetVar(wp, name, "");
			snprintf(name, sizeof(name), "publicType%d", i);
			if (!strcmp(websGetVar(wp, name, ""), "2"))
				publicType = "udp";
			else
				publicType = "tcp";

			if (!*triggerPort && !*publicPort) {
				del_autofw_port(i);
				continue;
			}
			if (!valid_range(triggerPort, triggerPort, 0, 65535) ||
			    !valid_range(publicPort, publicPort, 0, 65535))
				continue;

			/* Set up parameters */
			memset(&app, 0, sizeof(netconf_app_t));
			strncpy(app.desc, desc, sizeof(app.desc)-1);
			app.desc[sizeof(app.desc)-1] = 0;
			if (!strcmp(triggerType, "tcp"))
				app.match.ipproto = IPPROTO_TCP;
			else if (!strcmp(triggerType, "udp"))
				app.match.ipproto = IPPROTO_UDP;
			app.match.dst.ports[0] = app.match.dst.ports[1] = htons(atoi(triggerPort));
			if (!strcmp(publicType, "tcp"))
				app.proto = IPPROTO_TCP;
			else if (!strcmp(publicType, "udp"))
				app.proto = IPPROTO_UDP;
			app.dport[0] = app.dport[1] = app.to[0] = app.to[1] = htons(atoi(publicPort));
			if (!strcmp(enable, "off"))
				app.match.flags |= NETCONF_DISABLED;
			a_assert(set_autofw_port(i, &app));
		}
		nvram_commit();
		restart = 1;
	}

	ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
	if (restart)
		sys_restart();
	return ret;
}

static int
persistent_port_forwarding_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
			       char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	int i;
	char name[100];
	char *enable, *desc, *type;
	char *publicPortLow, *publicPortHigh, *privateIP, *privatePortLow, *privatePortHigh;
	struct in_addr in;
	int restart = 0, ret;
	netconf_nat_t nat;

	if (!strcmp(page, "persistent_port_forwarding")) {
		for (i = 1; i <= MAX_NVPARSE; i++) {
			snprintf(name, sizeof(name), "enable%d", i);
			if (websGetVar(wp, name, NULL))
				enable = "on";
			else
				enable = "off";
			snprintf(name, sizeof(name), "desc%d", i);
			desc = websGetVar(wp, name, "");
			snprintf(name, sizeof(name), "publicPortLow%d", i);
			publicPortLow = websGetVar(wp, name, "");
			snprintf(name, sizeof(name), "publicPortHigh%d", i);
			publicPortHigh = websGetVar(wp, name, "");
			snprintf(name, sizeof(name), "type%d", i);
			if (!strcmp(websGetVar(wp, name, ""), "2"))
				type = "udp";
			else
				type = "tcp";
			snprintf(name, sizeof(name), "privateIP%d", i);
			privateIP = websGetVar(wp, name, "");
			snprintf(name, sizeof(name), "privatePortLow%d", i);
			privatePortLow = websGetVar(wp, name, "");
			snprintf(name, sizeof(name), "privatePortHigh%d", i);
			privatePortHigh = websGetVar(wp, name, "");

			if (!*publicPortLow && !*publicPortHigh &&
			    !*privateIP && !*privatePortLow && !*privatePortHigh) {
				del_forward_port(i);
				continue;
			}
			if (!valid_range(publicPortLow, publicPortHigh, 0, 65535) ||
			    !valid_range(privatePortLow, privatePortHigh, 0, 65535) ||
			    !valid_range(privateIP, privateIP, 0, 255))
				continue;

			/* Set up parameters */
			memset(&nat, 0, sizeof(netconf_nat_t));
			strncpy(nat.desc, desc, sizeof(nat.desc)-1);
			nat.desc[sizeof(nat.desc)-1] = 0;
			if (!strcmp(type, "tcp"))
				nat.match.ipproto = IPPROTO_TCP;
			else if (!strcmp(type, "udp"))
				nat.match.ipproto = IPPROTO_UDP;
			nat.match.dst.ports[0] = htons(atoi(publicPortLow));
			nat.match.dst.ports[1] = htons(atoi(publicPortHigh));
			(void)inet_aton(nvram_safe_get("lan_ipaddr"), &in);
			nat.ipaddr.s_addr = htonl((ntohl(in.s_addr) & ~0xff) | atoi(privateIP));
			nat.ports[0] = htons(atoi(privatePortLow));
			nat.ports[1] = htons(atoi(privatePortHigh));
			if (!strcmp(enable, "off"))
				nat.match.flags |= NETCONF_DISABLED;
			a_assert(set_forward_port(i, &nat));
		}
		nvram_commit();
		restart = 1;
	}

	ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
	if (restart)
		sys_restart();
	return ret;
}

static int
virtual_dmz_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
			       char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	char *value;
	char *enable;
	char ip[] = "XXX.XXX.XXX.XXX";
	struct in_addr in;
	int restart = 0, ret;

	if (!strcmp(page, "virtual_dmz")) {
		if ((enable = websGetVar(wp, "enableDMZ", NULL)))
			nvram_set("enableDMZ", "1");
		else
			nvram_set("enableDMZ", "0");
		nvram_set("dmzip42", (value = websGetVar(wp, "dmzip42", "")));
		if (enable &&
		    atoi(value) >= 0 && atoi(value) <= 255) {
			(void) inet_aton(nvram_safe_get("lan_ipaddr"), &in);
			in.s_addr = htonl((ntohl(in.s_addr) & ~0xff) | atoi(value));
			strncpy(ip, inet_ntoa(in), sizeof(ip));
			nvram_set("dmz_ipaddr", ip);
		} else
			nvram_set("dmz_ipaddr", "");
		nvram_commit();
		restart = 1;
	}

	ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
	if (restart)
		sys_restart();
	return ret;
}

static int
mac_filtering_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
		  char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	int i, j;
	char name[100];
	char *enableConnectionBox, *enableAssociationBox;
	char buf[100], *cur;
	unsigned char mac[ETHER_ADDR_LEN];
	int restart = 0, ret;

	if (!strcmp(page, "mac_filtering")) {
		if ((enableConnectionBox = websGetVar(wp, "enableConnectionBox", NULL))) {
			nvram_set("enableConnectionBox", "1");
			if (!strcmp(websGetVar(wp, "connectionControlType", ""), "1"))
				nvram_set("filter_macmode", "deny");
			else
				nvram_set("filter_macmode", "allow");
		} else {
			nvram_set("enableConnectionBox", "0");
			nvram_set("filter_macmode", "disabled");
		}
		if ((enableAssociationBox = websGetVar(wp, "enableAssociationBox", NULL))) {
			nvram_set("enableAssociationBox", "1");
			if (!strcmp(websGetVar(wp, "associationControlType", ""), "1"))
				nvram_set("wl_macmode", "deny");
			else
				nvram_set("wl_macmode", "allow");
		} else {
			nvram_set("enableAssociationBox", "0");
			nvram_set("wl_macmode", "disabled");
		}
		for (i = 1; i <= MAX_NVPARSE; i++) {
			cur = buf;
			for (j = 1; j <= 6; j++) {
				snprintf(name, sizeof(name), "MACAddr%d_%d", j, i);
				cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s",
						cur == buf ? "" : ":",
						websGetVar(wp, name, ""));
			}

			if (cur == buf) {
				snprintf(name, sizeof(name), "filter_mac%d", i);
				nvram_unset(name);
				snprintf(name, sizeof(name), "wl_mac%d", i);
				nvram_unset(name);
				continue;
			}
			if (!ether_atoe(buf, mac))
				continue;

			/* Append allow or deny */
			*cur = '\0';
			snprintf(name, sizeof(name), "allowConnOnAddr%d", i);
			if (websGetVar(wp, name, NULL))
				strncat(buf, ":allow", buf + sizeof(buf) - cur);
			else
				strncat(buf, ":deny", buf + sizeof(buf) - cur);
			snprintf(name, sizeof(name), "filter_mac%d", i);
			nvram_set(name, buf);

			/* Append allow or deny */
			*cur = '\0';
			snprintf(name, sizeof(name), "allowAssocOnAddr%d", i);
			if (websGetVar(wp, name, NULL))
				strncat(buf, ":allow", buf + sizeof(buf) - cur);
			else
				strncat(buf, ":deny", buf + sizeof(buf) - cur);
			snprintf(name, sizeof(name), "wl_mac%d", i);
			nvram_set(name, buf);
		}
		nvram_commit();
		restart = 1;
	}				

	ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
	if (restart)
		sys_restart();
	return ret;	
}

static int
client_filtering_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
		     char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	int i;
	char name[100];
	char *ipRangeLow, *ipRangeHigh, *portRangeLow, *portRangeHigh;
	char *startDay, *endDay, *startTime, *endTime;
	char *enable, *proto;
	int restart = 0, ret;
	struct in_addr in;
	netconf_filter_t start, end;

	if (!strcmp(page, "firewall_clientfilter")) {
		for (i = 1; i <= MAX_NVPARSE; i++) {
			/* blocked */
			snprintf(name, sizeof(name), "filterCheckbox%d", i);
			if (websGetVar(wp, name, NULL))
				enable = "on";
			else
				enable = "off";

			/* ip */
			snprintf(name, sizeof(name), "ipRangeLow%d", i);
			ipRangeLow = websGetVar(wp, name, "");
			snprintf(name, sizeof(name), "ipRangeHigh%d", i);
			ipRangeHigh = websGetVar(wp, name, "");
			if (*ipRangeLow && !*ipRangeHigh)
				ipRangeHigh = ipRangeLow;
			else if (!*ipRangeLow && *ipRangeHigh)
				ipRangeLow = ipRangeHigh;
			else if (*ipRangeLow && *ipRangeHigh &&
					!valid_range(ipRangeLow, ipRangeHigh, 1, 254))
				continue;

			/* port */
			snprintf(name, sizeof(name), "portRangeLow%d", i);
			portRangeLow = websGetVar(wp, name, "");
			snprintf(name, sizeof(name), "portRangeHigh%d", i);
			portRangeHigh = websGetVar(wp, name, "");
			if (*portRangeLow && !*portRangeHigh)
				portRangeHigh = portRangeLow;
			else if (!*portRangeLow && *portRangeHigh)
				portRangeLow = portRangeHigh;
			else if (*portRangeLow && *portRangeHigh &&
					!valid_range(portRangeLow, portRangeHigh, 0, 65535))
				continue;

			/* protocol */
			snprintf(name, sizeof(name), "protocol%d", i);
			if (!strcmp(websGetVar(wp, name, ""), "2"))
				proto = "udp";
			else
				proto = "tcp";

			/* when */
			snprintf(name, sizeof(name), "startDay%d", i);
			startDay = websGetVar(wp, name, "");
			if (!valid_range(startDay, startDay, 0, 6))
				continue;
			snprintf(name, sizeof(name), "endDay%d", i);
			endDay = websGetVar(wp, name, "");
			if (!valid_range(endDay, endDay, 0, 6))
				continue;
			snprintf(name, sizeof(name), "startTime%d", i);
			startTime = websGetVar(wp, name, "");
			snprintf(name, sizeof(name), "endTime%d", i);
			endTime = websGetVar(wp, name, "");
			if (!valid_range(startTime, endTime, 0, 24*60*60 - 1))
				continue;
			snprintf(name, sizeof(name), "whenRadio%d", i);
			if (strcmp(websGetVar(wp, name, ""), "1"))
				startDay = endDay = startTime = endTime = "0";
				
			/* Set up parameters */
			(void)inet_aton(nvram_safe_get("lan_ipaddr"), &in);
			memset(&start, 0, sizeof(netconf_filter_t));
			if (!strcmp(proto, "tcp"))
				start.match.ipproto = IPPROTO_TCP;
			else if (!strcmp(proto, "udp"))
				start.match.ipproto = IPPROTO_UDP;
			start.match.src.ipaddr.s_addr = htonl((ntohl(in.s_addr)&~0xff)+atoi(ipRangeLow));
			start.match.src.netmask.s_addr = htonl(0xffffffff);
			start.match.dst.ports[0] = htons(atoi(portRangeLow));
			start.match.dst.ports[1] = htons(atoi(portRangeHigh));
			start.match.days[0] = atoi(startDay);
			start.match.days[1] = atoi(endDay);
			start.match.secs[0] = atoi(startTime);
			start.match.secs[1] = atoi(endTime);
			if (!strcmp(enable, "off"))
				start.match.flags |= NETCONF_DISABLED;
			memcpy(&end, &start, sizeof(netconf_filter_t));
			end.match.src.ipaddr.s_addr = htonl((ntohl(in.s_addr)&~0xff)+atoi(ipRangeHigh));
			a_assert(set_filter_client(i, &start, &end));
		}
		nvram_commit();
		restart = 1;
	}

	ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
	if (restart)
		sys_restart();
	return ret;	
}

static int
security_log_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
		 char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");

	if (!strcmp(page, "system_log"))
		clearlog();

	return websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
}

static int
security_log(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg,
	     char_t *url, char_t *path, char_t *query)
{
	dumplog(wp, "\n");
	websDone(wp, 200);

	return 1;
}

static int
internal_htm(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	     char_t *url, char_t *path, char_t *query)
{
	char *page = websGetVar(wp, "page", "");
	char *value;
	int restart = 0, ret;

	if (!strcmp(page, "internal")) {
		if ((value = websGetVar(wp, "d11b_rateset", NULL)) &&
		    (!strcmp(value, "default") || !strcmp(value, "all"))) {
			nvram_set("d11b_rateset", value);
			nvram_set("d11g_rateset", value);
		}
		nvram_commit();
		restart = 1;
	}

	ret = websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
	if (restart)
		sys_restart();
	return ret;
}

#ifdef WEBS

void
initHandlers(void)
{
	websAspDefine("nvram_get", ej_nvram_get);
	websAspDefine("nvram_match", ej_nvram_match);
	websAspDefine("nvram_invmatch", ej_nvram_invmatch);
	websAspDefine("nvram_list", ej_nvram_list);
	websAspDefine("nvram_ip", ej_nvram_ip);
	websAspDefine("nvram_mac", ej_nvram_mac);
	websAspDefine("link", ej_link);
	websAspDefine("peer_ip", ej_peer_ip);
	websAspDefine("peer_mac", ej_peer_mac);
	websAspDefine("timezones", ej_timezones);
	websAspDefine("months", ej_months);
	websAspDefine("range", ej_range);
	websAspDefine("options", ej_options);
	websAspDefine("enctype", ej_enctype);
	websAspDefine("dhcp_clients", ej_dhcp_clients);
	websAspDefine("app_triggered_port_forwards", ej_persistent_port_forwards);
	websAspDefine("persistent_port_forwards", ej_persistent_port_forwards);
	websAspDefine("mac_filters", ej_mac_filters);
	websAspDefine("client_filters", ej_client_filters);
	websAspDefine("dumplog", ej_dumplog);

	websUrlHandlerDefine("/login.htm", NULL, 0, login_htm, 0);
	websUrlHandlerDefine("/home.htm", NULL, 0, login_htm, 0);
	websUrlHandlerDefine("/reset.htm", NULL, 0, reset_htm, 0);
	websUrlHandlerDefine("/config.bin", NULL, 0, config_bin, 0);
	websUrlHandlerDefine("/backup_restore.htm", NULL, 0, backup_restore_htm, 0);
	websUrlHandlerDefine("/firmware_upgrade.htm", NULL, 0, firmware_upgrade_htm, 0);
	websUrlHandlerDefine("/time_settings.htm", NULL, 0, time_settings_htm, 0);
	websUrlHandlerDefine("/change_password.htm", NULL, 0, change_password_htm, 0);
	websUrlHandlerDefine("/local_network.htm", NULL, 0, local_network_htm, 0);
	websUrlHandlerDefine("/wide_area_network.htm", NULL, 0, wide_area_network_htm, 0);
	websUrlHandlerDefine("/wireless.htm", NULL, 0, wireless_htm, 0);
	websUrlHandlerDefine("/wireless_encryption.htm", NULL, 0, wireless_encryption_htm, 0);
	websUrlHandlerDefine("/network_address_translation.htm", NULL, 0, network_address_translation_htm, 0);
	websUrlHandlerDefine("/hacker_protection.htm", NULL, 0, hacker_protection_htm, 0);
	websUrlHandlerDefine("/app_triggered_port_forwarding.htm", NULL, 0, app_triggered_port_forwarding_htm, 0);
	websUrlHandlerDefine("/persistent_port_forwarding.htm", NULL, 0, persistent_port_forwarding_htm, 0);
	websUrlHandlerDefine("/virtual_dmz.htm", NULL, 0, virtual_dmz_htm, 0);
	websUrlHandlerDefine("/mac_filtering.htm", NULL, 0, mac_filtering_htm, 0);
	websUrlHandlerDefine("/client_filtering.htm", NULL, 0, client_filtering_htm, 0);
	websUrlHandlerDefine("/security_log.htm", NULL, 0, security_log_htm, 0);
	websUrlHandlerDefine("/security.log", NULL, 0, security_log, 0);
	websUrlHandlerDefine("/internal.htm", NULL, 0, internal_htm, 0);
}

#else /* !WEBS */

static char post_buf[10000] = { 0 };

static void
do_post(char *url, FILE *stream, int len, char *boundary)
{
	char *path, *query;
	int bytes;

	/* Parse path */
	query = url;
	path = strsep(&query, "?") ? : url;

	if (!*boundary) {
		/* Get query */
		if (!fgets(post_buf, MIN(len + 1, sizeof(post_buf)), stream))
			return;
		len -= strlen(post_buf);
	} else {
		/* Look for our part */
		while (len > 0) {
			if (!fgets(post_buf, MIN(len + 1, sizeof(post_buf)), stream))
				return;
			len -= strlen(post_buf);
			if (!strncasecmp(post_buf, "Content-Disposition:", 20) &&
			    strstr(post_buf, "name=\"webUpload\""))
				break;
		}

		/* Skip boundary and headers */
		while (len > 0) {
			if (!fgets(post_buf, MIN(len + 1, sizeof(post_buf)), stream))
				return;
			len -= strlen(post_buf);
			if (!strcmp(post_buf, "\n") || !strcmp(post_buf, "\r\n"))
				break;
		}

		if (!strcmp(path, "backup_restore.htm")) {
			bytes = 0;
			while (bytes < MIN(len, NVRAM_SPACE))
				bytes += fread(&nvram[bytes], 1, MIN(len, NVRAM_SPACE) - bytes, stream);
			len -= bytes;

			/* update nvram */
			if (nvram_restore(nvram, bytes))
				strcpy(post_buf, "page=restore_backup_failed");
			else
				strcpy(post_buf, "page=system_backup");
		}

		else if (!strcmp(path, "firmware_upgrade.htm")) {
			if (sys_upgrade(NULL, stream, &len))
				strcpy(post_buf, "page=upgrade_firmware_failed");
			else
				strcpy(post_buf, "page=system_firm");
		}
	}

	/* Initialize CGI */
	init_cgi(post_buf);

	/* Slurp anything remaining in the request */
	while (len--)
		(void) fgetc(stream);
}

static void
do_microsoft(char *url, FILE *stream)
{
	char *path, *query;
	struct sockaddr_in peer;
	socklen_t len;
	time_t now;

	/* Parse path */
	query = url;
	path = strsep(&query, "?") ? : url;

	/* Access control */
	time(&now);
	if (now >= current_user.expires) {
		/* Require login if current user is expired */
		path = "login.htm";
	} else {
		/* Allow only one user at a time */
		len = sizeof(peer);
		if (getpeername(fileno(stream), (struct sockaddr *) &peer, &len) < 0 ||
		    current_user.addr.s_addr != peer.sin_addr.s_addr) {
			do_ej("duplicate.htm", stream);
			return;
		} else {
			/* Bump expiration for current user */
			current_user.expires = now + (atoi(nvram_safe_get("pwTimeout")) * 60);
		}
	}

	if (!strcmp(path, "login.htm") ||
	    !strcmp(path, "index.asp") ||
	    !strcmp(path, "index.htm") ||
	    !strcmp(path, "index.html"))
		login_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "home.htm"))
		home_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "reset.htm"))
		reset_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "config.bin"))
		config_bin(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "backup_restore.htm"))
		backup_restore_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "firmware_upgrade.htm"))
		firmware_upgrade_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "time_settings.htm"))
		time_settings_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "change_password.htm"))
		change_password_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "local_network.htm"))
		local_network_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "wide_area_network.htm"))
		wide_area_network_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "wireless.htm"))
		wireless_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "wireless_encryption.htm"))
		wireless_encryption_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "network_address_translation.htm"))
		network_address_translation_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "hacker_protection.htm"))
		hacker_protection_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "app_triggered_port_forwarding.htm"))
		app_triggered_port_forwarding_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "persistent_port_forwarding.htm"))
		persistent_port_forwarding_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "virtual_dmz.htm"))
		virtual_dmz_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "mac_filtering.htm"))
		mac_filtering_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "client_filtering.htm"))
		client_filtering_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "security_log.htm"))
		security_log_htm(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "security.log"))
		security_log(stream, NULL, NULL, 0, url, path, query);
	else if (!strcmp(path, "internal.htm"))
		internal_htm(stream, NULL, NULL, 0, url, path, query);
	else
		do_ej(path, stream);

	/* Reset CGI */
	init_cgi(NULL);
}

void
do_internetgateway(char *path, FILE *stream)
{
    /* do nothing for the linux router. */
}


struct ej_handler ej_handlers[] = {
	{ "nvram_get", ej_nvram_get },
	{ "nvram_match", ej_nvram_match },
	{ "nvram_invmatch", ej_nvram_invmatch },
	{ "nvram_list", ej_nvram_list },
	{ "nvram_ip", ej_nvram_ip },
	{ "nvram_mac", ej_nvram_mac },
	{ "link", ej_link },
	{ "peer_ip", ej_peer_ip },
	{ "peer_mac", ej_peer_mac },
	{ "timezones", ej_timezones },
	{ "months", ej_months },
	{ "range", ej_range },
	{ "options", ej_options },
	{ "txrates", ej_txrates },
	{ "gmode", ej_gmode },
	{ "enctype", ej_enctype },
	{ "dhcp_clients", ej_dhcp_clients },
	{ "app_triggered_port_forwards", ej_app_triggered_port_forwards },
	{ "persistent_port_forwards", ej_persistent_port_forwards },
	{ "mac_filters", ej_mac_filters },
	{ "client_filters", ej_client_filters },
	{ "dumplog", ej_dumplog },
	{ "current_ip", ej_current_ip },
	{ NULL, NULL }
};

static char no_cache[] =
"Cache-Control: no-cache\r\n"
"Pragma: no-cache\r\n"
"Expires: 0"
;

struct mime_handler mime_handlers[] = {
	{ "**.asp", "text/html", no_cache, do_post, do_microsoft, NULL },
	{ "**.htm", "text/html", no_cache, do_post, do_microsoft, NULL },
	{ "**.bin", "application/x-octet-stream", no_cache, NULL, do_microsoft, NULL },
	{ "**.log", "text/plain", no_cache, NULL, do_microsoft, NULL },
	{ "**.gif", "image/gif", NULL, NULL, do_file, NULL },
	{ "**.jpg", "image/jpeg", NULL, NULL, do_file, NULL },
	{ "**.js", "text/javascript", NULL, NULL, do_file, NULL },
	{ "**.css", "text/css", NULL, NULL, do_file, NULL },
	{ "x_internetgatewaydevice.xml", "text/xml", NULL, NULL, do_internetgateway, NULL },
	{ "**.xml", "text/xml", NULL, NULL, do_file, NULL },
	{ "**.htc", "text/plain", NULL, NULL, do_file, NULL },
	{ NULL, NULL, NULL, NULL, NULL, NULL }
};

#endif /* !WEBS */
