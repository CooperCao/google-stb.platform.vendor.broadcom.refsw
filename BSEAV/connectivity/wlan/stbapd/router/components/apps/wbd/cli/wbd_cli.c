/*
 * WBD Command Line Interface
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wbd_cli.c 759214 2018-04-24 10:40:05Z pj888946 $
 */

#include <getopt.h>

#include "wbd.h"
#include "wbd_shared.h"
#include "wbd_com.h"
#include "wbd_sock_utility.h"
#include "wbd_json_utility.h"

#define MAX_COMMAND_BUF	1024
#define WBD_MAX_ARG_LEN 32

typedef struct cmdargs {
	wbd_cli_cmd_type_t cmd;
	wbd_cli_cmd_type_t sub_cmd;
	char mac[ETHER_ADDR_LEN * 3];
	char bssid[ETHER_ADDR_LEN * 3];
	int band;
	int priority;
	short use_master;
	chanspec_t chanspec;
	int flags;
} cmdargs_t;

#define WBD_CLI_PRINT(fmt, arg...) printf("CLI >> "fmt, ##arg)

unsigned int wbd_msglevel = WBD_DEBUG_DEFAULT;
char g_wbd_process_name[WBD_MAX_PROCESS_NAME];

static void wbd_test();
extern int test_wbd_ds();

static void
wbd_usage()
{
	printf("usage: wb_cli <options> [ <value>]\n"
			"options are:\n"
			"	-m, --master	Issue the command to Master\n"
			"	-s, --slave	Issue the command to Slave\n"
			"	-a, --mac	Provide Mac Address\n"
			"	-b, --bssid	Provide BSSID\n"
			"	-g, --band	Provide Band : 1(for 2G), 2(for 5G Low),"
						" 3(for 5G High)\n"
			"	-p, --priority	Provide Priority\n"
			"	-e, --clear	Clear the entries. Used along with logs command\n"
			"	-t, --test	Stub Testing\n"
			"	-v, --ver	app version\n"
			"	-h, --help	Help\n"
			"\n"
			"Master Commands are:\n"
			"	version		Displays the version of WBD\n"
			"	info		Displays full Blanket information of all Slaves\n"
			"			with their associated and monitored Clients\n"
			"	slavelist	Displays all Blanket Slaves\n"
			"	clientlist	Displays all Blanket Clients associated\n"
			"			to every Slave\n"
			"	steer		Steer the client from one Slave to other Slave\n"
			"			Ex : wb_cli -m steer -a mac -b bssid\n"
			"	logs		Displays log messages of Blanket\n"
			"\n"
			"Slave Commands are:\n"
			"	version		Displays the version of WBD\n"
			"	info		Displays full Slave information\n"
			"			with its associated and monitored Clients\n"
			"	slavelist	Displays Slave's information\n"
			"	clientlist	Displays all associated STAs to this Slave\n"
			"	monitorlist	Lists all monitored STAs by this Slave\n"
			"	monitoradd	Adds a STA to be monitored by this Slave\n"
			"			Ex : wb_cli -s monitoradd -a mac -b bssid\n"
			"	monitordel	Deletes a STA from monitored if MAC is provided,\n"
			"			else deletes all STAs from monitor\n"
			"	weakclient	Makes a client weak\n"
			"	weakcancel	Makes a weak client normal\n"
			"\n");
}

/* WBD CLI TCP Client */
static int
wbd_cli_send(char *clidata, int portno)
{
	int ret = WBDE_OK, rcv_ret = 0;
	int sockfd = INVALID_SOCKET;
	char *read_buf = NULL;

	/* Connect to the server */
	sockfd = wbd_connect_to_server(WBD_LOOPBACK_IP, portno);
	if (sockfd == INVALID_SOCKET) {
		WBD_CLI_PRINT("Failed to connect\n");
		return WBDE_SOCK_ERROR;
	}

	/* Send the data */
	if (wbd_socket_send_data(sockfd, clidata, strlen(clidata)+1) <= 0) {
		ret = WBDE_SOCK_ERROR;
		WBD_CLI_PRINT("Failed to send\n");
		goto exit;
	}

	/* Get the response from the server */
	rcv_ret = wbd_socket_recv_data(sockfd, &read_buf);
	if ((rcv_ret <= 0) || (read_buf == NULL)) {
		ret = WBDE_SOCK_ERROR;
		WBD_CLI_PRINT("Failed to recieve\n");
		goto exit;
	}
	fputs(read_buf, stdout);

exit:
	wbd_close_socket(&sockfd);
	if (read_buf)
		free(read_buf);

	return ret;
}

/* Parse the commandline parameters and populate the structure cmdarg with the command
 * and parameters if any
 */
static int
wbd_cli_parse_opt(int argc, char *argv[], cmdargs_t *cmdarg)
{
	int ret = WBDE_OK;
	int c, found = 0;

	memset(cmdarg, 0, sizeof(*cmdarg));
	cmdarg->use_master = 1;
	cmdarg->cmd = -1;

	static struct option long_options[] = {
		{"slave",	required_argument,	0, 's'},
		{"master",	required_argument,	0, 'm'},
		{"mac",		required_argument,	0, 'a'},
		{"priority",	required_argument,	0, 'p'},
		{"bssid",	required_argument,	0, 'b'},
		{"chanspec",	required_argument,	0, 'c'},
		{"band",	required_argument,	0, 'g'},
		{"clear",	no_argument,		0, 'e'},
		{"test",	no_argument,		0, 't'},
		{"help",	no_argument,		0, 'h'},
		{"ver",		no_argument,		0, 'v'},
		{0,		0,			0, 0}

	};

	while (1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "s:m:a:p:b:c:g:eth:v", long_options, &option_index);
		if (c == -1) {
			if (found == 0) {
				wbd_usage();
				exit(1);
			}
			goto end;
		}

		found = 1;
		switch (c) {
			case 's' :
				cmdarg->use_master = 0;
				cmdarg->cmd = wbd_get_cli_command_id(optarg);
				break;

			case 'm' :
				cmdarg->use_master = 1;
				cmdarg->cmd = wbd_get_cli_command_id(optarg);
				break;
			case 'a' :
				strncpy(cmdarg->mac, optarg, sizeof(cmdarg->mac)-1);
				break;

			case 'p' :
				cmdarg->priority = atoi(optarg);
				break;

			case 'b' :
				strncpy(cmdarg->bssid, optarg, sizeof(cmdarg->bssid)-1);
				break;

			case 'g' :
				cmdarg->band = atoi(optarg);
				break;

			case 'c' :
				cmdarg->chanspec = strtoul(optarg, NULL, 0);
				break;

			case 'e' :
				cmdarg->flags |= WBD_CLI_CLEAR_LOGS;
				break;

			case 't' :
				wbd_test();
				exit(1);

			case 'v' :
				wbd_print_app_version(argv[0]);
				/* exit, just print version */
				exit(0);

			default :
				wbd_usage();
				exit(1);
		}
	}

end:
	return ret;
}

/* Validates the commands and its arguments */
static int
wbd_cli_validate_cmdarg(cmdargs_t *cmdarg)
{
	int ret = WBDE_OK;
	struct ether_addr mac, bssid;

	/* Validate Arguments for Band value, if given */
	if (cmdarg->band && !WBD_BAND_VALID((cmdarg->band))) {
		ret = WBDE_CLI_INV_BAND;
	}

	/* Validate Arguments for MAC value, if given */
	if (strlen(cmdarg->mac) > 0) {
		/* Get & Validate MAC */
		WBD_GET_VALID_MAC(cmdarg->mac, &mac, "", WBDE_CLI_INV_MAC);
	}

	/* Validate Arguments for BSSID value, if given */
	if (strlen(cmdarg->bssid) > 0) {
		/* Get & Validate BSSID */
		WBD_GET_VALID_MAC(cmdarg->bssid, &bssid, "", WBDE_CLI_INV_BSSID);
	}

	/* Validate Arguments for STEER Command */
	if (cmdarg->cmd == WBD_CMD_CLI_STEER) {
		/* It should be sent to Master */
		if (cmdarg->use_master == 0)
			ret = WBDE_CLI_INV_SLAVE_CMD;
		else if (strlen(cmdarg->mac) <= 0)
			ret = WBDE_CLI_INV_MAC;
		else if (strlen(cmdarg->bssid) <= 0)
			ret = WBDE_CLI_INV_BSSID;
	}

	/* Validate Arguments for STAMON Commands */
	if (cmdarg->cmd == WBD_CMD_CLI_MONITORADD ||
		cmdarg->cmd == WBD_CMD_CLI_MONITORDEL ||
		cmdarg->cmd == WBD_CMD_CLI_MONITORLIST) {
		/* It should be sent to Slave */
		if (cmdarg->use_master != 0)
			ret = WBDE_CLI_INV_MASTER_CMD;
		else if (strlen(cmdarg->bssid) <= 0)
			ret = WBDE_CLI_INV_BSSID;
	}

	/* Validate Arguments for weak client Command */
	if (cmdarg->cmd == WBD_CMD_CLI_WEAK_CLIENT ||
		cmdarg->cmd == WBD_CMD_CLI_WEAK_CANCEL) {
		/* It should be sent to Slave */
		if (cmdarg->use_master != 0)
			ret = WBDE_CLI_INV_MASTER_CMD;
		else if (strlen(cmdarg->mac) <= 0)
			ret = WBDE_CLI_INV_MAC;
		else if (strlen(cmdarg->bssid) <= 0)
			ret = WBDE_CLI_INV_BSSID;
	}

	/* Validate Arguments for logs command */
	if (cmdarg->cmd == WBD_CMD_CLI_LOGS) {
		if (cmdarg->use_master == 0) {
			ret = WBDE_CLI_INV_SLAVE_CMD;
		}
	}

end:
	if (ret != WBDE_OK) {
		WBD_CLI_PRINT("Improper Arguments. Error : %s\n", wbderrorstr(ret));
		wbd_usage();
		exit(1);
	}

	return ret;
}

/* Process the command requested by the command-line */
static int
wbd_cli_process(cmdargs_t *cmdarg)
{
	int ret = WBDE_OK;
	wbd_cli_send_data_t clidata;
	char *data;

	if (cmdarg->cmd == -1) {
		wbd_usage();
		exit(1);
	}

	memset(&clidata, 0, sizeof(clidata));
	clidata.cmd = cmdarg->cmd;
	clidata.sub_cmd = cmdarg->sub_cmd;
	strncpy(clidata.mac, cmdarg->mac, sizeof(clidata.mac)-1);
	strncpy(clidata.bssid, cmdarg->bssid, sizeof(clidata.bssid)-1);
	clidata.priority = cmdarg->priority;
	clidata.band = cmdarg->band;
	clidata.flags |= cmdarg->flags;

	data = (char*)wbd_json_create_cli_cmd((void*)&clidata);
	WBD_ASSERT_ARG(data, WBDE_JSON_ERROR);

	if (cmdarg->use_master)
		wbd_cli_send(data, EAPD_WKSP_WBD_TCP_MASTERCLI_PORT);
	else
		wbd_cli_send(data, EAPD_WKSP_WBD_TCP_SLAVECLI_PORT);

end:
	return ret;
}

int
main(int argc, char **argv)
{
	int ret = WBDE_OK;
	cmdargs_t cmdarg;

	/* Set Process Name */
	snprintf(g_wbd_process_name, sizeof(g_wbd_process_name), "CLI");

	ret = wbd_cli_parse_opt(argc, argv, &cmdarg);
	WBD_ASSERT();

	ret = wbd_cli_validate_cmdarg(&cmdarg);
	WBD_ASSERT();

	ret = wbd_cli_process(&cmdarg);
	WBD_ASSERT();

end:
	return ret;
}

static void
wbd_test()
{
	test_wbd_ds();
}
