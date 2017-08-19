/*
 *
 * hwdiags_winpe.c -- PE Tool Diag Functions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: hwdiags_winpe.c,v 1.1104.2.62.2.5 2010-12-27 06:50:50 $
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <conio.h>
#include <ctype.h>
#include <Setupapi.h>
#include <Cfgmgr32.h>
#include <typedefs.h>
#include <epivers.h>
#include <proto/ethernet.h>
#include <proto/802.11.h>
#include <proto/802.1d.h>
#include <proto/802.11e.h>
#include <wlioctl.h>
#include <bcmdevs.h>
#include <bcmwifi_channels.h>
#include <bcmendian.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <wlu.h>
#include <wlu_common.h>
#include <wlu_remote_vista.h>
#include <wlu_cmd.h>
#include <hwdiags_winpe.h>

char s_band_desc[][20] =
	{
		"Auto",
		"2G Band",
		"5G Band"
	};

uint wl_log_level = LOG_FAILED;


extern void pet_ctrlc_handler()
{
	printf("##### PET interrupted by user.\n"
		"##### Exiting the application.");
	exit(RET_CTRLC_SIG);
}


/* Loggind Functions */
void tee(char *data, ...)
{
	va_list ap;

	va_start(ap, data);
	if (wl_log_level <= s_log.smodifier) {
		vprintf(data, ap);
		fflush(stdout);
	}
	va_end(ap);
}

void update_pet_log_msg(char *data, ...)
{
	char *tmp_log_msg; /* Temparary string Debug Info */
	va_list ap;
	tmp_log_msg = (char *)malloc(1024 * sizeof(char));
	va_start(ap, data);
	vsprintf(tmp_log_msg, data, ap);
	va_end(ap);
	strcat(pet_log_msg, tmp_log_msg);
	if (tmp_log_msg)
		free(tmp_log_msg);
}

void pet_log(param_info_t *param, int test_num, int err_num)
{
	char *pet_tests[6] = {"Card Verification Test",
						"WLAN Switch Test",
						"WLAN LED Test",
						"WLAN COEX Signal status Test",
						"W_DISABLE# Signal status Test",
						"Antenna Connectivity Test"};
	char *month[12] = {"Jan", "Feb", "Mar", "Apr", "May", "June",
						"July", "Aug", "Sep", "Oct", "Nov", "Dec"};
	PSYSTEMTIME systime = (PSYSTEMTIME)malloc(sizeof(SYSTEMTIME));
	GetLocalTime(systime);
	if (s_log.fp) {
		if (err_num)
			fprintf(s_log.fp, "\nStatus : FAIL\n");
		else
			fprintf(s_log.fp, "\nStatus : PASS\n");
		fprintf(s_log.fp, "OS : WinPE\nSource : BRCM"
		"\nRevision : 1\nUnit Under Test : BRCM WLAN\n"
		"Test Name : %s\nTest Number : %d\n"
		"Device : VEN_%04X&DEV_%04X \n"
		"Time Stamp :  %s %ld %ld %ld:%ld:%ld\n",
		pet_tests[test_num-1], test_num, param->vid, param->pid,
		month[(systime->wMonth)-1],
		systime->wDay, systime->wYear, systime->wHour,
		systime->wMinute, systime->wSecond);
		if (err_num) {
			fprintf(s_log.fp, "Error Number : %d \nMsg : %s\n", err_num, pet_log_msg);
		}
	}
}


static bool pet_check_missing_arg(char *parm, char *sw)
{
	if (!parm || parm[0] == '-') {
		fprintf(stderr, "ERR: missing value for %s switch\n", sw);
		return FALSE;
	}
	return TRUE;
}

static bool pet_missing_arg(char *parm)
{
	if (!parm || parm[0] == '-')
		return TRUE;
	return FALSE;
}

static void pet_dump_param(param_info_t *p)
{
	WL_TRACE(("\nCommand line parameters:\n"));

	WL_TRACE((" network=\"%s\"\n", p->network));
	WL_TRACE((" vid  = 0x%02x, pid    = 0x%02x\n"
		   " svid = 0x%02x, sdevid = 0x%02x\n",
		   p->vid, p->pid, p->svid, p->sdevid));
	WL_TRACE((" irq = %d\n", p->irq));

	WL_TRACE((" antm = %d,  rssim = %d, anta = %d, rssia = %d\n"
		   " ant  = %d,   rssi_delta = %d\n"
		   " retrycount = %d, max_assoc = %d\n",
		   p->antm, p->rssim, p->anta, p->rssia,
		   p->ant, p->rssi_delta,
		   p->retrycount, p->max_assoc));

	WL_TRACE((" led  =%d,      led_state = %d,   led_persistent = %d\n"
		   " wlact=%d,    wlact_state = %d, wlact_persistent = %d\n"
		   " pins=%d, w_disable_state = %d,        bta_state = %d\n",
		   p->led, p->led_state, p->led_persistent,
		   p->wlact, p->wlact_state, p->wlact_persistent,
		   p->pins, p->w_disable_state, p->bta_state));

	WL_TRACE((" ws = %d, ws_state = %d \n",
		p->ws, p->ws_state));
	WL_TRACE((" logfile = \"%s\" fmodifier = %d smodifier = %d\n",
		p->logfile, p->fmodifier, p->smodifier));
}
static void pet_print_help(char *pgm)
{
	int i;
	static const char pet_err_hlp[LAST_ERROR_CODE][128] = {
	"\t00 : All test successfully passed.\n",
	"\t01 : Application interrupted by user.\n",
	"\t02 : No Supported Broadcom wireless adapter found.\n",
	"\t03 : Error while parsing command line inputs.\n",
	"\t04 : Error in validating card information.\n"
	"\t05 : Connection Failed.",
	"\t06 : Error in enabling wireless adapter.\n",
	"\t07 : Error in selecting antenna.\n",
	"\t08 : Wireless LAN switch is off.\n",
	"\t09 : RSSI test for main antenna failed.\n",
	"\t10 : RSSI test for auxiliary antenna failed.\n",
	"\t11 : RSSI DELTA test for main antenna failed.\n",
	"\t12 : RSSI DELTA test for auxiliary antenna failed.\n",
	"\t13 : Out of memory.\n",
	"\t14 : LED test failed by the user.\n"
	};
	printf("\nBroadcom 802.11a/b/g/n WinPETool %s"
			"(Compiled at %s on %s)\n",
			EPI_VERSION_STR, __TIME__, __DATE__);
	printf("Usage:\n");
	printf("%s [options]\n", pgm);
	printf("###Card verification options\n");
	printf("\t-ssid\tSSID of the Access Point.\n"
		   "\t     \tDefault is \"TEST-AP\"\n");
	printf("\t-vid \tVendor ID. Optional\n"
		   "\t     \tShould be mentioned together with pid \n");
	printf("\t-pid \tProduct ID. Optional\n"
		   "\t     \tShould be mentioned together with vid \n");
	printf("\t-svid\tSubvendor ID. Optional \n");
	printf("\t-sid \tSubsystem ID. Optional \n");
	printf("Press Enter to continue...\n");
	while (getchar() != '\n');
	printf("###Antenna Connectivity Test Options\n");
	printf("\t-antm\tPerform only the MAIN antenna RSSI Test.\n");
	printf("\t-ssm \tRSSI threshold to determine success or failure of the\n");
	printf("\t     \tMAIN Antenna Connectivity Test.  Default: %d dB.\n", DEFAULT_RSSI);
	printf("\t-anta\tPerform only the AUX antenna RSSU Test.\n");
	printf("\t-ssa \tRSSI threshold to determine success or failure of the\n"
	       "\t     \tAUXILIARY Antenna Connectivity Test.  Default: %d dB.\n",
	       DEFAULT_AUX_RSSI);
	printf("\t-ant \tPerform all antennas connectivity tests.\n"
		   "\t     \tDefault: TRUE\n");
	printf("\t-ss  \tRSSI threshold (in dB) to determine success or failure\n");
	printf("\t     \tof all antennas connectivity Tests.\n");
	printf("\t     \t Default: %d dB.\n", DEFAULT_MAIN_RSSI);
	printf("\t-r   \tNumber of times of running Antenna Connectivity Test.\n"
		   "\t     \tDefault: 1. Note: \"-r 0\" turns off RSSI Test.\n");
	printf("\t-d   \tMax RSSI delta threshold to determine success or failure\n");
	printf("\t     \tof all antennas RSSI delta tests.\n");
	printf("Press Enter to continue...\n");
	while (getchar() != '\n');
	printf("###Wireless LAN LED Tests\n");
	printf("\t-t led on/off on/off \tTurn on or off all LEDs. On/Off leds exit\n");
	printf("\t-t pins        \tDisplay W_DISABLE# and COEX1_BT_ACTIVE pins.\n");
	printf("\t-t wlact on/off\tSet COEX2_WLAN_ACTIVE pin on or off.\n");
	printf("Press Enter to continue...\n");
	while (getchar() != '\n');
	printf("###Wireless Switch Tests\n");
	printf("\t-ws\tReoport status of Wireless LAN switch\n");
	printf("Press Enter to continue...\n");
	while (getchar() != '\n');
	printf("###Other Options\n");
	printf("\t-l [filename]\tSpecify the filename for logging.Default: file logging\n");
	printf("\t             \tis enabled and default filename is rftest_o.txt\n");
	printf("\t-sm <n>      \tDebug Print Message Level:\n");
#ifdef BCMINTDBG
	printf("\t-fm <n>      \tFile logging modifier:\n");
#endif
	printf("\t             \t0<->nothing logged\n");
	printf("\t             \t1<->only failures are logged\n");
	printf("\t             \t2<->everything is logged\n");
	printf("\t             \t3<->debug logging -- more than level 2\n");
	printf("\t-v           \tPrint version.\n");
	printf("\t-info        \tDisplay wireless card information\n");
	printf("\t-ma <val>    \tMax number of association attempts\n");
	printf("\t-at <val>    \tAmount of time in seconds the application "
			"\t\t		\twait for getting assosiated\n");
	printf("\t-sd     \tPrints the Device ids of supported devices\n");
	printf("\t-h or -?\tPrint this help.\n");
	printf("Press Enter to continue...\n");
	while (getchar() != '\n');
	printf("###Error Codes\n");
	for (i = 0; i < LAST_ERROR_CODE; i++) {
		printf("%s", pet_err_hlp[i]);
	}
	while (getchar() != '\n');
}

/* Refer to print_help for the usage */
static bool pet_parse_param(param_info_t *param, int argc, char **argv)
{
	char *pgm;

	ENTER();

	memset(param, sizeof(param), 0);

	/* init param struct with default values */
	strcpy(param->network, "*");

	param->irq = 0;	/* Initiaize */

	param->ant   = FALSE;
	param->antm  = FALSE;
	param->anta  = FALSE;
	param->retrycount = 1;
	param->max_assoc = MAX_ASSOC_ATTEMPT;
	param->assoc_time = DEF_ASSOC_TIME * 1000;

	param->led = FALSE;
	param->led_state = 0;
	param->led_chkstatus = TRUE;
	param->wlact = FALSE;
	param->wlact_state = 0;
	param->pins = FALSE;
	param->w_disable_state = 0;
	param->bta_state = 0;

	strcpy(param->logfile, LOG_FILE); /* NULL means no file logging */
	param->fmodifier = LOG_FAILED;
	param->smodifier = LOG_FAILED;
	param->fp = NULL;
	param->rep_devs = FALSE;
	param->notest = FALSE;

	param->subsystemVendorID = VENDOR_DELL;
	param->vendor = VENDOR_DELL;

	param->band = WLC_BAND_AUTO;

	pgm = *argv;			/* save pointer to program name */

	/* read parameters from command line */
	for (++argv; *argv; argv++) {
		if (!strcmp(*argv, "-ssid")) {
			if (!pet_check_missing_arg(*++argv, "-ssid"))
				return FALSE;
			strcpy(param->network, *argv);
		}
		else if (!strcmp(*argv, "-vid")) {
			if (!pet_check_missing_arg(*++argv, "-vid"))
				return FALSE;
			sscanf(*argv, "%hx", &param->vid);
		}
		else if (!strcmp(*argv, "-pid")) {
			if (!pet_check_missing_arg(*++argv, "-pid"))
				return FALSE;
			sscanf(*argv, "%hx", &param->pid);
		}
		else if (!strcmp(*argv, "-svid")) {
			if (!pet_check_missing_arg(*++argv, "-svid"))
				return FALSE;
			sscanf(*argv, "%hx", &param->svid);
		}
		else if (!strcmp(*argv, "-sid")) {
			if (!pet_check_missing_arg(*++argv, "-sid"))
				return FALSE;
			sscanf(*argv, "%hx", &param->sdevid);
		}
		else if (!strcmp(*argv, "-info")) {
			param->notest = TRUE;
		}
		else if (!strcmp(*argv, "-ma")) {
			if (!pet_check_missing_arg(*++argv, "-ma"))
				return FALSE;
			param->max_assoc = atoi(*argv);
		}
		else if (!strcmp(*argv, "-at")) {
			if (!pet_check_missing_arg(*++argv, "-at"))
				return FALSE;
			param->assoc_time = atoi(*argv) * 1000;
		}
		else if (!strcmp(*argv, "-r")) {
			if (!pet_check_missing_arg(*++argv, "-r"))
				return FALSE;
			param->retrycount = atoi(*argv);
		}
		else if (!strcmp(*argv, "-ws")) {
			param->ws = TRUE;
		}
		else if (!strcmp(*argv, "-ss")) {
			if (*++argv == NULL) {
				fprintf(stderr, "ERR: missing value for %s switch\n", "-ss");
				return FALSE;
			}
			param->rssim = atoi(*argv);
			param->rssia = atoi(*argv);
		}
		else if (!strcmp(*argv, "-ssm")) {
			if (*++argv == NULL) {
				fprintf(stderr, "ERR: missing value for %s switch\n", "-ssm");
				return FALSE;
			}
			param->rssim = atoi(*argv);
		}
		else if (!strcmp(*argv, "-ssa")) {
			if (*++argv == NULL) {
				fprintf(stderr, "ERR: missing value for %s switch\n", "-ssa");
				return FALSE;
			}
			param->rssia = atoi(*argv);
		}
		else if (!strcmp(*argv, "-ant")) {
			param->ant = TRUE;
			param->antm = TRUE;
			param->anta = TRUE;
		}
		else if (!strcmp(*argv, "-antm")) {
			param->ant = FALSE;
			param->antm = TRUE;
			param->anta = FALSE;
		}
		else if (!strcmp(*argv, "-anta")) {
			param->ant = FALSE;
			param->antm = FALSE;
			param->anta = TRUE;
		}
		else if (!strcmp(*argv, "-d")) {
			if (!pet_check_missing_arg(*++argv, "-d"))
				return FALSE;
			param->rssi_delta = atoi(*argv);
		}
		else if (!strcmp(*argv, "-l")) {
			if (pet_missing_arg(*++argv))
				--argv;		/* step back arg pointer */
			else
				strcpy(param->logfile, *argv);
		}
#ifdef BCMINTDBG
		else if (!strcmp(*argv, "-fm")) {
			if (!pet_check_missing_arg(*++argv, "-fm"))
				return FALSE;
			param->fmodifier = atoi(*argv);
		}
#endif
		else if (!strcmp(*argv, "-sd")) {
			param->rep_devs = TRUE;
			param->notest = TRUE;
		}
		else if (!strcmp(*argv, "-sm")) {
			if (!pet_check_missing_arg(*++argv, "-sm"))
				return FALSE;
			param->smodifier = atoi(*argv);
		}
		/* Broadcom Home Networking specific switches: */
		else if (!strcmp(*argv, "-v")) {
			param->ver = TRUE;
		}
		else if (!strcmp(*argv, "-t")) {
			if (!pet_check_missing_arg(*++argv, "-t"))
				return FALSE;
			if (!strcmp(*argv, "led")) {
				param->led = TRUE;
				if (!pet_check_missing_arg(*++argv, "led"))
					return FALSE;
				if (!strcmp(*argv, "on")) {
					param->led_state = 1;
				}
				else if (!strcmp(*argv, "off")) {
					param->led_state = 0;
				}
				if (pet_missing_arg(*++argv)) {
					argv--;
					param->led_persistent = param->led_state;
				}
				else {
					if (!strcmp(*argv, "on")) {
						param->led_persistent = 1;
					}
					else if (!strcmp(*argv, "off")) {
						param->led_persistent = FALSE;
					}
				}
			}
			else if (!strcmp(*argv, "pins")) {
				param->pins = TRUE;
			}
			else if (!strcmp(*argv, "wlact")) {
				param->wlact = TRUE;
				if (!pet_check_missing_arg(*++argv, "wlact"))
					return FALSE;
				if (!strcmp(*argv, "on"))
					param->wlact_state = 1;
				else if (!strcmp(*argv, "off"))
					param->wlact_state = 0;
			}
		}
		else if (!strcmp(*argv, "-h") || !strcmp(*argv, "-?")) {
			pet_print_help(pgm);
			exit(RET_SUCCESS);
		}
		else {
			printf("ABORT: invalid switch %s\n", *argv);
			pet_print_help(pgm);
			return FALSE;
		}
	}

	if (param->anta == FALSE && param->antm == FALSE &&
	   param->led == FALSE && param->wlact == FALSE &&
	   param->pins == FALSE && param->ws == FALSE && param->notest == FALSE &&
	   param->ver == FALSE) {
		/* Enable RSSI test if no other tests are enabled */
		param->ant = param->anta = param->antm = TRUE;
	}
	/* Verify for VID and PID */
	if (((param->vid == 0) && (param->pid != 0)) ||
		(param->vid != 0) && (param->pid == 0)) {
		printf("ABORT: Invalid arguments: Mention both VID & PID\n", *argv);
		return FALSE;
	}
	/* Verify for delta prerequisite */
	if ((param->ant == FALSE) && (param->rssi_delta != 0)) {
		param->rssi_delta = 0;
		printf("WARNING: Invalid arguments: anta or antm is FALSE. Skipping Delta test\n");
		return FALSE;
	}
	/* Verify ss/ssa/ssm values being +ve */
	if ((param->rssia > 0) || (param->rssim > 0)) {
		printf("ABORT: Argument ss/ssa/ssm cannot take +ve integer values.");
		return FALSE;
	}
	/* Update default params */
	if (param->antm && param->rssim == 0) {
		param->rssim = DEFAULT_MAIN_RSSI;
	}
	if (param->anta && param->rssia == 0) {
		param->rssia = DEFAULT_AUX_RSSI;
	}

	param->fp = fopen(param->logfile, "w+");

	if (param->fp == NULL) {
		fprintf(stderr, "ERR: failed to open log file %s\n",
			param->logfile);
			return FALSE;
	}

	/* for external builds, limit the debug info */
	if (param->smodifier > LOG_SNR)
		param->smodifier = LOG_SNR;
	if (param->fmodifier > LOG_SNR)
		param->fmodifier = LOG_SNR;
	s_log.smodifier = param->smodifier;
	s_log.fmodifier = param->fmodifier;
	s_log.fp = param->fp;

	return TRUE;
}

static void pet_cleanup(param_info_t *param, HANDLE wlh)
{
	if (param->fp != NULL) {
		fclose(param->fp);
	}
	if (pet_log_msg)
		free(pet_log_msg);
	/* Setting adapter to clean state */
	wlu_set(wlh, WLC_DOWN, NULL, 0);
	wlu_set(wlh, WLC_UP, NULL, 0);
}

int pet_deviectest(param_info_t *param, HANDLE wlh)
{
	int exit_code = RET_SUCCESS;
	do {
		if (param->ver) {
			pet_dump_verinfo(param, wlh);
			break;
		}
		/* Card verification test */
		exit_code = pet_diag_check_wlancard(param, wlh);
		if (exit_code != RET_SUCCESS)
			break;

		if (param->notest)
			break;

		/* Hardware Switch Status */
		if (param->ws)
			exit_code = pet_diag_hwradio_status(param, wlh);
		if (exit_code != RET_SUCCESS)
			break;

		/* LED Test */
		if (param->led)
			exit_code = pet_diag_led_test(param, wlh);
		if (exit_code != RET_SUCCESS)
			break;

		/* BT Coex Signal, W_DISABLE# Signal status */
		if (param->pins)
			exit_code = pet_diag_btc_test(param, wlh);
		if (exit_code != RET_SUCCESS)
			break;

		if (param->wlact)
			exit_code = pet_diag_wlanact_test(param, wlh, param->wlact_state);
		if (exit_code != RET_SUCCESS)
			break;
		/* Antenna Test */
		if (param->antm || param->anta) {
			exit_code =  pet_diag_init(param, wlh);
		}
		if (exit_code != RET_SUCCESS)
			break;

		if (param->antm || param->anta) {
			exit_code =  pet_diag_antenna_test(param, wlh);
		}
		if (exit_code != RET_SUCCESS)
			break;
	} while (0);
	return exit_code;
}

static int pet_entry(HANDLE wlh, int argc, char *argv[])
{
	int exit_code = RET_SUCCESS;
	param_info_t param;

	pet_log_msg = (char *)malloc(4096 * sizeof(char));
	memset(pet_log_msg, 0, 4096);
	ENTER();
	memset(&param, 0, sizeof(param_info_t));

	if (!pet_parse_param(&param, argc, argv))
		return RET_ERROR_CL_READ;

	pet_dump_param(&param);

	exit_code = pet_deviectest(&param, wlh);

	pet_cleanup(&param, wlh);

	EXIT();

	return exit_code;

}

int
pet_start(void *irh, int argc, char **argv)
{
	return pet_entry(irh, argc, argv);

}

cmd_t* find_callback(const char *cmd)
{
	return wlu_find_cmd(cmd);
}

/* Scan ALL PCI Devices in the system */
void pci_scan()
{
	HDEVINFO devs = INVALID_HANDLE_VALUE;
	DWORD	idx = 0;
	SP_DEVINFO_DATA devinfo_data;
	SP_DEVINFO_LIST_DETAIL_DATA  detail_data;
	TCHAR	devinst_str[256];

	printf("Enter : %s\n", __FUNCTION__);

	devs = SetupDiGetClassDevsEx(
			NULL, (PCTSTR)(L"PCI"), NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT,
			NULL, NULL, NULL);

	if (devs != INVALID_HANDLE_VALUE) {
		detail_data.cbSize = sizeof(SP_DEVINFO_LIST_DETAIL_DATA);
		SetupDiGetDeviceInfoListDetail(devs, &detail_data);

		devinfo_data.cbSize = sizeof(devinfo_data);
		for (idx = 0; SetupDiEnumDeviceInfo(devs, idx, &devinfo_data); idx++) {
			CM_Get_Device_ID_Ex(
				devinfo_data.DevInst, devinst_str,
				SIZEOF_ARRAY(devinst_str), 0,
				detail_data.RemoteMachineHandle);

			WL_TRACE(("Devinst : %ws \n", devinst_str));
		}
		SetupDiDestroyDeviceInfoList(devs);
	}
}

int
wl_get_hwradio_status(HANDLE wlh, bool *on)
{
	int ret;
	uint32 radio_status = 0;

	ENTER();

	/* Initialize status */
	*on = 0;

	/* Get Radio status */
	ret = wlu_get(wlh, WLC_GET_RADIO, &radio_status, sizeof(radio_status));
	WL_DEBUG((" radio_status = 0x%x\n", radio_status));

	if (ret != BCME_OK) {
		WL_FAILED((" Error in getting the HW WLAN Radio Switch status\n"));
		ret = RET_ERROR_CARD_INFO;
	}
	*on = (radio_status & WL_RADIO_HW_DISABLE) ? 0 : 1;

	WL_DEBUG(("Radio Staus 0x%x, HW Radio Status \n", radio_status, *on));
	EXIT();
	return ret;
}

static BOOL pci_read_irq_resource(PSP_DEVINFO_DATA pdevinfo_data, ULONG *pirq)
{
	ULONG status = 0;
	ULONG problem = 0;
	LOG_CONF config = 0;
	BOOL haveConfig = FALSE;
	RES_DES resdes = 0;
	ULONG	data_size;
	PBYTE	p_data;

	/* Get the device state */
	if (CM_Get_DevNode_Status_Ex(&status, &problem, pdevinfo_data->DevInst, 0,
		NULL) != CR_SUCCESS) {
		return FALSE;
	}
	/* see if the device is running and what resources it might be using */
	if (status & DN_HAS_PROBLEM) {
		WL_FAILED(("Device is not running \n"));
		return FALSE;
	}
	/* Get the  resource requirements */
	if (CM_Get_First_Log_Conf_Ex(&config,
		pdevinfo_data->DevInst,
		ALLOC_LOG_CONF,
		NULL) != CR_SUCCESS) {
		WL_FAILED(("Error in querying the logical configuration \n"));
		return FALSE;
	}

	if ((CM_Get_Next_Res_Des_Ex(&resdes, (RES_DES)config, ResType_IRQ, NULL, 0,
		NULL) != CR_SUCCESS)) {
		WL_FAILED(("Error in querying the logical configuration \n"));
		return FALSE;
	}

	CM_Get_Res_Des_Data_Size_Ex(&data_size, resdes, 0, NULL);
	p_data = (PBYTE)malloc(sizeof(BYTE) * data_size);
	if (p_data == NULL) {
		WL_FAILED(("Error in querying the logical configuration \n"));
		return FALSE;
	}

	if (CM_Get_Res_Des_Data_Ex(resdes, p_data, data_size, 0, NULL)
		== CR_SUCCESS) {

		PIRQ_RESOURCE  p_irqdata = (PIRQ_RESOURCE)p_data;
		*pirq = p_irqdata->IRQ_Header.IRQD_Alloc_Num;

		WL_TRACE(("IRQ Resource %d \n", *pirq));
		free(p_data);
		return TRUE;
	}

	free(p_data);
	WL_FAILED(("Error in qyeryigng IRQ Resource\n"));
	return FALSE;
}

static BOOL pci_get_irq(param_info_t *param, ULONG *pirq)
{
	HDEVINFO devs = INVALID_HANDLE_VALUE;
	DWORD	idx = 0;
	SP_DEVINFO_LIST_DETAIL_DATA  detail_data;
	SP_DEVINFO_DATA devinfo_data;
	WCHAR	devinst_str_w[256];
	char	devinst_str[256];
	char	ip_devinst[64];
	char	*p;
	int     ret = FALSE;
	DWORD	cmapi_ret = CR_SUCCESS;
	bool	done = FALSE;

	ENTER();

	_snprintf(ip_devinst, sizeof(ip_devinst)/sizeof(ip_devinst[0]),
		"VEN_%04X&DEV_%04X",
		param->vid, param->pid /* param->sdevid, param->svid */);
	WL_DEBUG(("ip_devstr %s\n", ip_devinst));

	/* Handle PCI class devices */
	devs = SetupDiGetClassDevsEx(NULL, (PCTSTR)("PCI"), NULL,
		DIGCF_ALLCLASSES | DIGCF_PRESENT, NULL, NULL, NULL);

	if (devs == INVALID_HANDLE_VALUE) {
		WL_FAILED(("Error in retrieving device information set\n"));
		goto error;
	}

	/* Detailed info about PCI class device enumeration */
	detail_data.cbSize = sizeof(SP_DEVINFO_LIST_DETAIL_DATA);
	if (SetupDiGetDeviceInfoListDetail(devs, &detail_data)) {
		/* Enumerate each device */
		devinfo_data.cbSize = sizeof(devinfo_data);
		for (idx = 0; SetupDiEnumDeviceInfo(devs, idx, &devinfo_data); idx++) {
			/* Get device ID for each device */
			cmapi_ret =
				CM_Get_Device_ID_Ex(devinfo_data.DevInst,
				devinst_str,
				SIZEOF_ARRAY(devinst_str), 0,
				detail_data.RemoteMachineHandle);
			if (cmapi_ret != CR_SUCCESS) {
				WL_FAILED(("Error in getting device instance ID\n"));
				break;
			}
			WideCharToMultiByte(CP_ACP, WC_SEPCHARS, devinst_str_w, -1,
				devinst_str,
				sizeof(devinst_str)/sizeof(devinst_str[0]),
				NULL, NULL);
			WL_DEBUG(("Devinst : %s \n", devinst_str));
			if ((p = strstr(devinst_str, ip_devinst)) != NULL)  {
				WL_TRACE(("Getting IRQ resource for %s \n", devinst_str));
				ret = pci_read_irq_resource(&devinfo_data, pirq);
			}
		}
		SetupDiDestroyDeviceInfoList(devs);
		if (!done)
			goto error;
	}
	else {
		WL_FAILED(("Error in enumerating PCI Devices \n"));
	}

error:
	EXIT();
	return (ret == TRUE ? BCME_OK : BCME_BADARG);
}

/* check whether the found wireless adapter falls within the list */
int pet_diag_check_wlancard(param_info_t *param, HANDLE wlh)
{
	int ret = RET_SUCCESS;
	uint32	vid = 0;
	ulong   pid_cur = 0;
	ulong   vid_cur = 0;
	bool	bdevicefound = FALSE;
	bool	chk_user = FALSE;
	ulong   sdevid, svenid;
	int i;
	char c = 0;
	uint32	cfgreg_offset = 0;
	uint32	cfgreg_val = 0;
	wlc_rev_info_t revinfo;
	cmd_t *cmd_ce;
	struct ether_addr ea = {{0, 0, 0, 0, 0, 0}};

	/* Place most recent devices on top of the list: */
	struct devid ids[] = {
		{ BCM43142_D11N2G_ID, VENDOR_BROADCOM },
		{ BCM43227_D11N2G_ID, VENDOR_BROADCOM },
		{ BCM43228_D11N_ID, VENDOR_BROADCOM },
		{ BCM43224_D11N_ID, VENDOR_BROADCOM },
		{ BCM4313_D11N2G_ID, VENDOR_BROADCOM },
	};

	ENTER();
	WL_LOG(("\n##### DETECT_WIRELESS_CARD\n"));
	memset(&revinfo, 0, sizeof(revinfo));
	if (wlu_get(wlh, WLC_GET_REVINFO, &revinfo, sizeof(revinfo))) {
		ret = RET_ERROR_CARD_INFO;
		WL_FAILED((" Error while reading VID/PID from the device. Exiting \n"));
		param->pid = 0;
		param->vid = 0;
		goto exit;
	}

	/* Read VID, DEVID */
	vid_cur = revinfo.vendorid;
	pid_cur = revinfo.deviceid;

	WL_DEBUG((" VID = 0x%x DEVID= 0x%x of the installed card\n",
		vid_cur, pid_cur));

	/* Check whether the current version of PET Support this card and */
	/* whether this is the intended card */
	for (i = 0; i < ARRAYSIZE(ids); i++) {
		if ((pid_cur == ids[i].device) && (vid_cur == ids[i].vendor)) {
			WL_DEBUG((" Card is supported by PET\n"));
			bdevicefound = TRUE;

			if ((param->pid != 0) && (param->vid != 0)) {
				if ((pid_cur != param->pid) || (vid_cur != param->vid)) {
					bdevicefound = FALSE;
					WL_FAILED((" This is not intended card \n"));
					/* Updating param for logging purpose */
					param->pid = (ushort)pid_cur;
					param->vid = (ushort)vid_cur;
				}
			}
			else {
				param->pid = (ushort)pid_cur;
				param->vid = (ushort)vid_cur;
				WL_DEBUG((" Update vid, pid params\n"));
			}
			break;
		}
	}

	if (bdevicefound == FALSE) {
		WL_LOG(("##### No Supported device detected\n"));
		WL_LOG(("##### Supported Device ids :\t0x%x\n", ids[0].device));
		for (i = 1; i < SIZEOF_ARRAY(ids); i++) {
			WL_LOG(("\t\t\t\t0x%x \n", ids[i].device));
		}
		i = 0;
		ret = RET_NO_WIRELESS_CARD_FOUND;
		goto exit;
	}

	svenid = revinfo.boardvendor;
	sdevid = revinfo.boardid;

	if (param->svid && param->svid != svenid) {
		WL_FAILED((" Sub-vendor ID 0x%04x. Was looking for 0x%04x\n",
			svenid, param->svid));
		chk_user = TRUE;
	}
	if (param->sdevid && param->sdevid != sdevid) {
		WL_FAILED((" Sub-system ID 0x%04x. Was looking for 0x%04x\n",
			sdevid, param->sdevid));
		chk_user = TRUE;
	}
	if (chk_user) {
		printf("\nPress:\nI to ignore this error.\n "
			   "Or press Enter to exit...\n\n>>>");
		do {
			c = getchar();
		}  while (c != '\n' && c != 's' && c != 'S' && c != 'i' && c != 'I');

		if (c != 'i' && c != 'I') {
			bdevicefound = FALSE;
		}
	}
	if (bdevicefound == FALSE) {
		ret = RET_NO_WIRELESS_CARD_FOUND;
		goto exit;
	}
	param->svid = (ushort)svenid;
	param->sdevid = (ushort)sdevid;

	/* Get IRQ from the device	*/
	ret = pci_get_irq(param, &param->irq);
	if (ret != BCME_OK) {
		WL_FAILED(("Error while reading IRQ from the device. Exiting \n"));
		ret = RET_ERROR_CARD_INFO;
		goto exit;
	}
	/* Get MAC address from the device */
	cmd_ce = find_callback("cur_etheraddr");
	if (!cmd_ce) {
		WL_FAILED(("Error in finding cur_etheraddr callback\n"));
		ret = RET_ERROR_CARD_INFO;
		EXIT();
		goto exit;
	}
	if ((ret = wlu_iovar_get(wlh, cmd_ce->name, &ea, ETHER_ADDR_LEN)) < 0) {
			printf(("Error getting variable %s\n", cmd_ce->name));
			ret = RET_ERROR_CARD_INFO;
			goto exit;
	}
	/* Log test results	*/
	WL_LOG(("##### VID  0x%04x PID 0x%04x \n", param->vid, param->pid));
	WL_LOG(("##### SVID 0x%04x SID 0x%04x \n", param->svid, param->sdevid));
	WL_LOG(("##### IRQ %d\n", param->irq));
	WL_LOG(("##### MAC ADDRESS %s\n", wl_ether_etoa(&ea)));
	WL_LOG(("##### PASS DETECT_WIRELESS_CARD\n"));
	if (param->rep_devs) {
		WL_LOG(("\n##### SUPPORTED DEVICE IDs :\t0x%x\n", ids[0].device));
		for (i = 1; i < SIZEOF_ARRAY(ids); i++) {
			WL_LOG(("\t\t\t\t0x%x \n", ids[i].device));
		}
	}
	ret = RET_SUCCESS;

exit:
	if (ret != RET_SUCCESS) {
		WL_LOG(("##### Error code %d \n", ret));
		WL_LOG(("##### FAIL DETECT_WIRELESS_CARD\n"));
	}
	pet_log(param, CARD_VER_TEST, ret);
	return ret;
}

int
pet_diag_hwradio_status(param_info_t *param, HANDLE wlh)
{
	int ret;
	bool on = 0;

	ENTER();

	WL_LOG(("\n##### REPORT WLAN HW SWITCH STATUS\n"));

	ret = wl_get_hwradio_status(wlh, &on);

	if (ret == RET_SUCCESS) {
		WL_LOG(("##### HW Switch is %s \n", on ? "on" : "off"));
		WL_LOG(("\n##### PASS REPORT WLAN HW SWITCH STATUS\n"));
	}
	else {
		WL_FAILED(("\n##### FAIL REPORT WLAN HW SWITCH STATUS\n"));
		WL_DEBUG(("##### Error code %d \n", ret));
	}
	EXIT();
	pet_log(param, WLAN_SW_TEST, ret);
	return ret;
}

/* initialize the device for the test */
int pet_diag_init(param_info_t *param, HANDLE wlh)
{
	int err = RET_SUCCESS;
	int	val = 0;

	UNREFERENCED_PARAMETER(param);

	ENTER();

	/* Disable MPC */
	WL_TRACE((" Disabling mpc\n"));
	wlu_iovar_setint(wlh, "mpc", 0);
	wlu_iovar_getint(wlh, "mpc", &val);
	WL_DEBUG((" MPC value = %d\n", val));

	if (val) {
		WL_WARN((" Cannot disable MPC\n"));
	}

	/* Check wther device isup */
	wlu_get(wlh, WLC_GET_UP, &val, sizeof(val));
	WL_DEBUG((" is up returned = %d\n", val));

	if (val != 1) {
		bool on = FALSE;
		/* If H/w switch is not turned off, try to up WL Driver */
		err = wl_get_hwradio_status(wlh, &on);
		if (on) {
			err = wlu_set(wlh, WLC_UP, NULL, 0);
			Sleep(500);
			err = wlu_get(wlh, WLC_GET_UP, &val, sizeof(val));
			if (val) {
				err = RET_SUCCESS;
			}
			else {
				err = RET_UNABLE_TO_ENABLE_CARD;
				WL_FAILED((" Unable to up the driver.\n"));
				goto error;
			}
		}
		else {
			err = RET_ERROR_WLRADIO_OFF;
			WL_FAILED((" Driver not coming up. HW Switch is turned off.\n"));
			goto error;
		}
	}

	WL_TRACE((" Successfully initialized\n"));
	EXIT();
	return RET_SUCCESS;
	/* more stuff from wl_ndis.c goes here */
error:
	WL_FAILED(("Err Code : %d\n", err));
	pet_log(param, ANT_CON_TEST, err);
	EXIT();
	return err;
}

DWORD
wl_scan_results(HANDLE wlh, wl_scan_results_t *p_scanresults, ULONG size)
{
	int ret = RET_SUCCESS;
	int i = 0;

	ENTER();
	ret = wl_get_scan(wlh, WLC_SCAN_RESULTS, (char *)p_scanresults, size);

	if (ret == BCME_OK) {
		if (p_scanresults->count == 0) {
			WL_FAILED((" Zero scan results\n"));
			ret = RET_FAILED_CONNECTION;
		}
	}
	else
		WL_WARN((" Scan results still not ready, err code %d\n", ret));

	EXIT();
	return ret;
}

int
pet_select_ssid(param_info_t *param, HANDLE wlh)
{
	cmd_t *cmd_scan;
	int err = RET_SUCCESS;
	int i;
	ULONG ntimeout = 0;
	wl_scan_results_t *pscanrslt = NULL;
	ULONG scan_rslts_size = sizeof(wl_scan_results_t) * WL_MAX_SCANRESULTS;
	int16 max_rssi = WLC_RSSI_MINVAL;

	/* scan with default parameters */
	char *scan_params[2]   = { "scan", NULL };

	if (param->network[0] != '\0' && param->network[0] != '*') {
		WL_DEBUG((" Using SSID (%s) set by the user", param->network));
		EXIT();
		return err;
	}
	/* Get the scan function */
	cmd_scan = find_callback("scan");
	if (!cmd_scan) {
		err = RET_FAILED_CONNECTION;
		WL_FAILED(("Error in finding scan callback\n"));
		EXIT();
		return err;
	}
	WL_DEBUG((" Invoking scan callback\n"));
	/* Call scan function */
	err = (cmd_scan->func)(wlh, cmd_scan, scan_params);
	if (err != BCME_OK) {
		err = RET_FAILED_CONNECTION;
		WL_FAILED(("Error in scanning \n"));
		WL_DEBUG(("Err Code : %d\n", err));
		EXIT();
		return err;
	}
	/* Get Scan results */
	WL_DEBUG((" Allocating %d bytes for scan results\n", scan_rslts_size));

	pscanrslt = (wl_scan_results_t *)malloc(scan_rslts_size);

	if (pscanrslt == NULL) {
		WL_FAILED((" Failed to allocate memory of %d bytes\n", scan_rslts_size));
		err = RET_ERROR_OUT_OF_MEMORY;
		return err;
	}
	do {
		if ((ntimeout % PET_STATUS_UPDATE_TO) == 0) {
			WL_LOG((" Waiting for scan results \n"));
		}
		Sleep(SCAN_INTERVAL);
		err = wl_scan_results(wlh, pscanrslt, scan_rslts_size);
		ntimeout += SCAN_INTERVAL;
	} while (err != RET_SUCCESS && (ntimeout <= SCAN_RESULTS_TO));

	if (err == BCME_OK) {
		char ssidbuf[SSID_FMT_BUF_LEN];
		wl_bss_info_t *bi;
		bi = pscanrslt->bss_info;

		/* Finding BSS with good RSSI */
		WL_DEBUG((" Dumping scan results\n"));
		for (i = 0; i < (int)(pscanrslt->count); i++,
			bi = (wl_bss_info_t*)((int8*)bi + dtoh32(bi->length))) {

			wl_format_ssid(ssidbuf, bi->SSID, bi->SSID_len);
			WL_DEBUG(("  SSID:%s, RSSI %d\n", ssidbuf,
				(int16)(dtoh16(bi->RSSI))));
			 if (bi->RSSI < 0 && bi->RSSI > max_rssi) {
				 max_rssi = bi->RSSI;
				 memset(param->network, 0, SIZEOF_ARRAY(param->network));
				 memcpy(param->network, bi->SSID, bi->SSID_len);
				 param->network[bi->SSID_len] = '\0';
			 }
		}
		WL_LOG((" Selected SSID \"%s\"\n", param->network));
		WL_LOG((" RSSI %d\n", max_rssi));
	}

	/* Select the SSID */
	free(pscanrslt);
	EXIT();
	return err;
}

unsigned int wl_get_phytype(HANDLE wlh)
{
	unsigned int phyType = WLC_PHY_TYPE_B;

	ENTER();
	wlu_get(wlh, WLC_GET_PHYTYPE, &phyType, sizeof(phyType));

	WL_DEBUG((" Queried PHY Type = 0x%d\n", phyType));
	EXIT();
	return phyType;
}

bool wl_isphytype_N_MIMO(HANDLE wlh)
{
	return (wl_get_phytype(wlh) == WLC_PHY_TYPE_N);
}

uint wl_get_max_antennas(HANDLE wlh)
{
	uint	ret = 2;
	uint	var;
	ENTER();

	if (wl_isphytype_N_MIMO(wlh)) {
		if (!wlu_iovar_get(wlh, "antennas", &var, sizeof(var))) {
			switch (var) {
				case ANTENNA_NUM_4:
					ret = 4;
					break;

				case ANTENNA_NUM_3:
					ret = 3;
					break;

				default:
					ret = 2;
				break;
			}
		}
		else {
			WL_FAILED((" Error in retrievng number of antennas\n"));
		}
	}
	EXIT();
	return ret;
}

/* Send NULL Data */
int wl_sendnulldata(HANDLE *wlh)
{
	struct ether_addr ea = {{0, 0, 0, 0, 0, 0}};
	int ret;
	ENTER();
	/*  Get the BSSID */
	if (!(ret = wlu_get(wlh, WLC_GET_BSSID, &ea, ETHER_ADDR_LEN))) {
		WL_DEBUG((" Sending NULL Data to %02x:%02x:%02x:%02x:%02x:%02x:\n",
			ea.octet[0], ea.octet[1], ea.octet[2], ea.octet[3],
			ea.octet[6], ea.octet[5]));
		if ((ret = wlu_iovar_set(wlh, "send_nulldata", &ea, ETHER_ADDR_LEN)) < 0)
				WL_DEBUG((" Unable to send NULL data, err code %d \n", ret));
	}
	else {
		WL_DEBUG((" Error in getting bssid err code %d \n", ret));
	}
	EXIT();
	return (ret == 0);
}

bool wl_issta_associated(HANDLE wlh)
{
	struct ether_addr bssid;
	/* Query the current BSSID to check the status of association */
	return (wlu_get(wlh, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN) == 0);
}

int
pet_vista_join(param_info_t *param, HANDLE wlh)
{
	cmd_t	*cmd_join;
	int		err = RET_SUCCESS;
	uint	wait_time = 0;
	/* join with default parameters */
	char *join_params[3]   = { "join", &param->network[0], NULL };
	struct ether_addr bssid;

	err = wlu_iovar_setint(wlh, "wsec", 0);
	if (err != BCME_OK) {
		err = RET_FAILED_CONNECTION;
		WL_WARN(("Could not disable security, continuing the test\n"));
	}

	/* Get the join function */
	cmd_join = find_callback("join");
	if (!cmd_join) {
		err = RET_FAILED_CONNECTION;
		WL_FAILED(("Could not join the network\n"));
		EXIT();
		return err;
	}
	WL_DEBUG((" Invoking join callback\n"));
	/* Call join function */
	err = (cmd_join->func)(wlh, cmd_join, join_params);
	if (err != BCME_OK) {
		err = RET_FAILED_CONNECTION;
		WL_FAILED(("Error in joining\n"));
		EXIT();
		return err;
	}
	do {
		Sleep(200);
		wait_time += 200;
		if ((wait_time % PET_STATUS_UPDATE_TO) == 0) {
			WL_LOG((" Waiting for connection to complete\n"));
		}
	} while ((wlu_get(wlh, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN) != 0) &&
		(wait_time <= param->assoc_time));

	if (wait_time <= param->assoc_time) {
		WL_LOG((" Connection succeeded \n"));
	}
	else {
		WL_FAILED((" Connect timeout. \n"));
		err = RET_FAILED_CONNECTION;

	}
	EXIT();
	return err;
}

/* Get SNR as well as RSSI as per ant */
int pet_snrtest_run(param_info_t *param, HANDLE wlh,
	uint ant, uint *snr, uint *rssi)
{
	ULONG val;
	int ret = RET_SUCCESS;
	uint rssi_run = 0, rssi_err_cnt = 0;
	int txant = 0;
	uint snr_sta = 0;
	int  rssi_sta = 0;
	scb_val_t scb_val;
	scb_val_t scb_val2;
	wl_rssi_ant_t ant_info;
	volatile int *pval0, *pnoise;

	ENTER();

	pval0 = &(scb_val.val);
	pnoise = &(scb_val2.val);

	if (ant == ANT_RX_DIV_FORCE_0)
		txant = 0;
	else if (ant == ANT_RX_DIV_FORCE_1)
		txant = 1;
	else if (ant == 2 && wl_isphytype_N_MIMO(wlh))
		txant = 2;

	do {
		if (!wl_isphytype_N_MIMO(wlh)) {
			/* set antenna diversity */
			if (wlu_set(wlh, WLC_SET_ANTDIV, &ant, sizeof(ant))) {
				WL_FAILED((" Failed to set ant diversity\n"));
				WL_LOG(("\n----- FAILED SNR/RSSI TEST\n"));
				ret = RET_ERROR_ANTDIV;
				break;
			}
			/* read back antenna diversity */
			if (wlu_get(wlh, WLC_GET_ANTDIV, &val, sizeof(val))) {
				WL_FAILED((" Failed to set read back ant diversity\n"));
				WL_LOG(("\n----- FAILED SNR/RSSI TEST\n"));
				ret = RET_ERROR_ANTDIV;
				break;
			}
			WL_DEBUG(("Set antdiv=%d; read back antdiv=%d\n", ant, val));
		}
		if (wlu_set(wlh, WLC_SET_TXANT, &txant, sizeof(txant))) {
			WL_FAILED(("Failed to set tx ant\n"));
			WL_LOG(("\n----- FAILED SNR/RSSI TEST\n"));
			ret = RET_ERROR_ANTDIV;
			break;
		}
		if (wlu_get(wlh, WLC_GET_TXANT, &val, sizeof(val))) {
			WL_LOG(("\n----- FAILED SNR TEST\n"));
			WL_FAILED(("Failed to set read back tx ant\n"));
			ret = RET_ERROR_ANTDIV;
			break;
		}
		WL_DEBUG(("Set tx ant=%d; read back antdiv=%d\n", txant, val));
		Sleep(500);

		snr_sta = 0;
		for (rssi_run = 0; rssi_run < MAX_RSSI_RUNS; rssi_run++) {
			if (!wl_issta_associated(wlh)) {
				WL_FAILED((" WLAN Card is not associated. Not running SNR/RSSI"
						"test on ant %u \n", ant));
				ret = RET_FAILED_CONNECTION;
			}
			if (!wl_sendnulldata(wlh)) {
				WL_DEBUG((" wlc_sendnulldata failed, run %d \n", rssi_run));
			}
			Sleep(500);

			memset(&ant_info, 0, sizeof(wl_rssi_ant_t));
			memset(&scb_val, 0, sizeof(scb_val_t));
			memset(&scb_val2, 0, sizeof(scb_val_t));

			if (param->noise == 0) {
				if (wlu_get(wlh, WLC_GET_PHY_NOISE, &scb_val2, sizeof(scb_val_t)))
					WL_DEBUG((" WLC_GET_PHY_NOISE Failed!\n"));

				WL_DEBUG((" WLC_GET_PHY_NOISE, noise = %d\n", scb_val2.val));
			}
			else {
				*pnoise = param->noise;
			}

			/* Get RSSI */
			if (!wl_isphytype_N_MIMO(wlh)) {
				/*
				  TBD: Cann't we use "phy_rssi_ant" even for non-N phys?
				 */
				if (wlu_get(wlh, WLC_GET_RSSI, &scb_val, sizeof(scb_val)))
					WL_FAILED((" WLC_GET_RSSI Failed!\n"));

				WL_DEBUG((" Iteration %d SNR %d ([RSSI]%d/[Noise]%d)\n",
					rssi_run, (*pval0-*pnoise), *pval0, *pnoise));

				if (*pval0 >= 0) {
					rssi_err_cnt++;
					if (rssi_err_cnt > MAX_RSSI_ERR_CNT) {
						WL_FAILED((" WLAN Card is not associated."
						"Not running SNR/RSSI test on ant %u \n", ant));
						return RET_FAILED_CONNECTION;
					}
					WL_WARN(("[BOGUS REJECT]"));
					rssi_run--;
				}
				else {
					snr_sta += (*pval0 - *pnoise);
					rssi_sta += *pval0;
				}
			}
			else {
				wlu_iovar_get(wlh, "phy_rssi_ant", &ant_info,
					sizeof(wl_rssi_ant_t));
				if (ant_info.count > (uint32)txant) {
					WL_DEBUG((" Iteration %d SNR %d ([RSSI]%d/[Noise])%d]\n",
						rssi_run, (ant_info.rssi_ant[txant]-*pnoise),
						ant_info.rssi_ant[txant],
						*pnoise));

					if (ant_info.rssi_ant[txant] >= 0) {
						if (rssi_err_cnt > MAX_RSSI_ERR_CNT) {
							WL_FAILED((" WLAN Card is not associated."
							"Not running SNR/RSSI test on ant %u \n",
							ant));
							return RET_FAILED_CONNECTION;
						}
						WL_WARN(("[BOGUS REJECT]"));
						rssi_run--;
					}
					else {
						snr_sta  += (ant_info.rssi_ant[txant]-*pnoise);
						rssi_sta += ant_info.rssi_ant[txant];
					}
				}
			}
		}

		*snr = snr_sta/MAX_RSSI_RUNS;
		*rssi = (int)rssi_sta/MAX_RSSI_RUNS;

		if (*snr > 100) {
			*snr  = 0; /* a bogus values is a null value */
			*rssi = 0;
		}
	} while (0);

	Sleep(100);

	WL_TRACE((" SNR %d, RSSI %d error code %d \n", *snr, *rssi, ret));
	EXIT();
	return ret;
}
/* run SNR test for a specified antenna */
int pet_snrtest_ant(param_info_t *param, HANDLE wlh, uint ant, char *antstr,
	uint req_snr, int req_rssi, uint iteration, char *ssid, bool bInfo,
	uint *snr, int *rssi)
{
	int ret = RET_SUCCESS;

	ENTER();

	*snr = 0;
	*rssi = 0;
	if (!wl_issta_associated(wlh)) {
		WL_WARN((" Waiting for STA to be associated \n"));

		Sleep(500);
		if (!wl_issta_associated(wlh)) {
			if (bInfo) {
				WL_FAILED((" WLAN Card is not associated. Not running SNR/RSSI"
						"test on ant %u \n", ant));
			}
			ret = RET_FAILED_CONNECTION;
		}
	}

	if (ret == RET_SUCCESS) {
		if (bInfo) {
			WL_DEBUG((" Sampling SNR/RSSI on %s antenna: \n", antstr));
		}
		if ((ret = pet_snrtest_run(param, wlh, ant, snr, rssi)) == RET_SUCCESS) {
		    BOOL test_pass = FALSE;
			if (bInfo) {
				if (param->snr_enb) {
					WL_SNRLOG((" [%5.5s] SNR = %u dB (Req. SNR = %u dB)",
						antstr, *snr, req_snr));
				}
				WL_LOG((" [%5.5s] RSSI= %d dB (Req. RSSI= %d dB)",
					antstr, *rssi, req_rssi));
			}
			if (param->snr_enb && (*snr >= req_snr || req_snr == 0)) {
				if (req_snr && bInfo) {
					WL_LOG((" -> PASSED SNR TEST"));
				}
				test_pass = TRUE;
			}
			if (*rssi >= req_rssi || req_rssi == 0) {
				if (req_rssi && bInfo) {
						WL_LOG((" -> PASSED RSSI TEST"));
				}
				test_pass = TRUE;
			}
			if (!test_pass) {
				if (bInfo) {
					WL_FAILED((" Poor RSSI for %s ANTENNA", antstr));
				}
				if (ant == ANT_RX_DIV_FORCE_0)
					ret = RET_ERROR_RSSI_MAIN_ANT;
				else if (ant == ANT_RX_DIV_FORCE_1)
					ret = RET_ERROR_RSSI_AUX_ANT;
				else if (ant == 2)
					ret = RET_ERROR_RSSI_AUX_ANT;
			}
			if (bInfo) {
				WL_LOG((" \n"));
			}
		}
	}
	EXIT();
	return ret;
}

int pet_snrtest(param_info_t *param, HANDLE wlh)
{
	uint i;
	int ret = RET_SUCCESS;
	uint snr_main = 0,  snr_aux0 = 0,  snr_aux1 = 0,  snr_max = 0;
	int rssi_main = 0, rssi_aux0 = 0, rssi_aux1 = 0, rssi_max = 0;

	ENTER();

	if (param->retrycount >= 1) {
		if (param->noise == 0) {

			WL_WARN((" Please wait. Updating Phy Noise\n"));
			for (i = 1; i < 3; i++) {
				pet_snrtest_ant(param, wlh, ANT_RX_DIV_FORCE_0, "MAIN",
				0, 0, i, param->network, FALSE, &snr_main, &rssi_main);
				pet_snrtest_ant(param, wlh, ANT_RX_DIV_FORCE_1, "AUX",
					0, 0, i, param->network, FALSE, &snr_aux0, &rssi_aux0);

				if (wl_isphytype_N_MIMO(wlh) && wl_get_max_antennas(wlh) > 2)
					pet_snrtest_ant(param, wlh, 2, "AUX",  0, 0, i,
						param->network, FALSE, &snr_aux1, &rssi_aux1);
			}
			WL_WARN((" Updated phynoise\n"));
		}

		for (i = 1; ret == RET_SUCCESS && i <= param->retrycount; i++) {
			snr_main = rssi_main = 0;
			snr_aux0 = rssi_aux0 = 0;
			snr_aux1 = rssi_aux1 = 0;
			snr_max  = rssi_max  = 0;

			WL_LOG(("\n RSSI test #%d to \"%s\" AP:\n", i, param->network));

			if (ret == RET_SUCCESS && param->antm) {
				ret = pet_snrtest_ant(param, wlh, ANT_RX_DIV_FORCE_0, "MAIN",
					param->snr ? param->snr : param->snrm,
					param->rssim,
					i, param->network, TRUE, &snr_main, &rssi_main);
			}
			if (ret == RET_SUCCESS && param->anta) {
				ret = pet_snrtest_ant(param, wlh, ANT_RX_DIV_FORCE_1,
					wl_isphytype_N_MIMO(wlh)?"AUX 0":"AUX",
					param->snr ? param->snr : param->snra,
					param->rssia,
					i, param->network, TRUE, &snr_aux0, &rssi_aux0);
			}
			if (ret == RET_SUCCESS && param->anta1 && wl_isphytype_N_MIMO(wlh) &&
				wl_get_max_antennas(wlh) > 2) {
				ret = pet_snrtest_ant(param, wlh, 2, "AUX 1",
					param->snr ? param->snr : param->snra,
					param->rssia,
  					/* TBD: Should it be rssia1 ? */
					i, param->network, TRUE, &snr_aux1, &rssi_aux1);
			}
			if (ret == RET_SUCCESS) {
				rssi_max = rssi_main > rssi_aux0 ? rssi_main: rssi_aux0;
				if (!wl_isphytype_N_MIMO(wlh) && (wl_get_max_antennas(wlh) > 2)) {
					rssi_max = rssi_max > rssi_aux1 ? rssi_max : rssi_aux1;
				}
				if (param->rssi_delta) {
					if (param->rssi_delta < (rssi_max-rssi_main)) {
						WL_FAILED((" MAIN ANTENNA FAILED "
							"MAX RSSI DELTA TEST\n"));
						ret = RET_ERROR_RSSI_MAIN_ANT_DELTA;
					}
					if (param->rssi_delta < (rssi_max-rssi_aux0)) {
						if (!wl_isphytype_N_MIMO(wlh))
							WL_FAILED((" AUX ANTENNA FAILED "
								"MAX RSSI DELTA TEST\n"));
						else
							WL_FAILED((" AUX 0 ANTENNA FAILED "
								"MAX RSSI DELTA TEST\n"));
						ret = RET_ERROR_RSSI_AUX_ANT_DELTA;
					}

					if (wl_get_max_antennas(wlh) > 2) {
						if (param->rssi_delta < (rssi_max-rssi_aux1)) {
							WL_FAILED((" AUX 1 ANTENNA FAILED "
								"MAX RSSI DELTA TEST\n"));
							ret = RET_ERROR_RSSI_AUX_ANT_DELTA;
						}
					}
				}
				snr_max = snr_main > snr_aux0 ? snr_main: snr_aux0;
				if (!wl_isphytype_N_MIMO(wlh)) {
					if (param->antm && (snr_main < 5 ||
						(snr_aux0 > (snr_main-5)*2 && snr_main > 4)))
						WL_FAILED((" MAIN ANTENNA MIGHT BE "
							"DISCONNECTED\n"));
					if (param->anta && (snr_aux0 < 5 ||
						(snr_main > (snr_aux0-5)*2 && snr_aux0 > 4)))
						WL_FAILED((" AUX  ANTENNA MIGHT BE "
							"DISCONNECTED\n"));
				}
				else {
					if (wl_get_max_antennas(wlh) > 2)
						snr_max = snr_max > snr_aux1 ? snr_max : snr_aux1;

					if (param->antm &&
					(snr_main < 5 || (snr_max > (snr_main)*2)))
						WL_FAILED((" MAIN ANTENNA MIGHT BE "
							"DISCONNECTED\n"));
					if (param->anta &&
					(snr_aux0 < 5 || (snr_max > (snr_aux0)*2)))
						WL_FAILED((" AUX 0 ANTENNA MIGHT BE "
							"DISCONNECTED\n"));
					if (wl_get_max_antennas(wlh) > 2) {
						if (snr_aux1 < 5 || (snr_max > (snr_aux1)*2))
							WL_FAILED((" AUX 1 ANTENNA "
							"MIGHT BE DISCONNECTED\n"));
					}
				}
				if (param->snr_enb && param->snr_delta) {
					if (param->snr_delta < (snr_max-snr_main)) {
						WL_FAILED((" MAIN ANTENNA FAILED "
							"MAX SNR DELTA TEST\n"));
						ret = RET_ERROR_SNR_MAIN_ANT_DELTA;
					}
					if (param->snr_delta < (snr_max-snr_aux0)) {
						if (!wl_isphytype_N_MIMO(wlh))
							WL_FAILED((" MAIN ANTENNA FAILED "
								"MAX SNR DELTA TEST\n"));
						else
							WL_FAILED((" AUX 0 ANTENNA FAILED "
								"MAX SNR DELTA TEST\n"));
						ret = RET_ERROR_SNR_AUX_ANT_DELTA;
					}

					if (wl_get_max_antennas(wlh) > 2) {
						if (param->snr_delta < (snr_max-snr_aux1)) {
							WL_FAILED((" AUX 1 ANTENNA FAILED "
								"MAX SNR DELTA TEST\n"));
							ret = RET_ERROR_SNR_AUX_ANT_DELTA;
						}
					}
				}
			}
		} /* for loop */
	}
	EXIT();
	return ret;
}

int
pet_diag_antenna_test(param_info_t *param, HANDLE wlh)
{
	int ret = RET_SUCCESS;
	wlc_ssid_t ssid = { 0, {0} };
	uint8 associated = 0;
	int i = 0;
	int txant = -1;
	uint count = 0;
	/* Default BSS Mode */
	int infra = WL_BSSTYPE_INFRA;

	ENTER();

	if (!param->antm && !param->anta) {
		EXIT();
		return ret;
	}

	WL_LOG(("\n##### ANTENNA CONNECTIVITY TEST\n"));
	WL_LOG((" RSSI will done for %s%s", param->antm ? "\"Main Antenna\" ": "",
	param->anta ? "\"Aux Antenna\"" : ""));
	WL_LOG(("\n"));
	ret = pet_select_ssid(param, wlh);
	if (ret != RET_SUCCESS) {
		WL_DEBUG((" Error while selecting SSID (%d). "
				 " Trying to connect default SSID \n", ret));
		strcpy(param->network, "AP-TEST");
		param->network[strlen("AP-TEST")+1] = '\0';
		ret = RET_SUCCESS;
	}

	/* Attempt to join a BSS with the requested SSID */
	WL_LOG(("\nConnecting to \"%s\" band (%s) \n", param->network,
		s_band_desc[param->band]));

	if (param->band != WLC_BAND_AUTO) {
		WL_TRACE((" Setting the band in wl driver\n"));
		ret = wlu_set(wlh, WLC_SET_BAND, &param->band, sizeof(param->band));
		if (BCME_OK != ret) {
			WL_WARN((" Error in setting the band to %s\n, Continuing the test\n",
			s_band_desc[param->band]));
			ret = RET_SUCCESS;
		}
	}

	/* Set SSID Parameters */
	ssid.SSID_len = strlen(param->network);
	memcpy(ssid.SSID, param->network, ssid.SSID_len);
	associated = 0;

	for (i = 0; i < (int)param->max_assoc; i++) {
		wlu_set(wlh, WLC_SET_INFRA, &infra, sizeof(infra));

		WL_TRACE(("\n Set SSID to %s (try %u of %u)\n",
			param->network, i+1, param->max_assoc));

		ret = pet_vista_join(param, wlh);

		if (ret == RET_SUCCESS) {
			associated = 1;
			break;
		}
		if (i & 1) {
			WL_WARN(("Reseting Chip...(associated failed)\n"));
			Sleep(200);
			wlu_set(wlh, WLC_DOWN, NULL, 0);
			Sleep(200);
			wlu_set(wlh, WLC_UP, NULL, 0);
			Sleep(200);
		}
	}  /* for ( i = 0; i < MAX_ASSOC_ATTEMPT; i++) */

	if (associated) {
		WL_LOG(("\n Connected to \"%s\"\n", param->network));
	}
	else {
		WL_FAILED(("\n Failed to Connected to \"%s\"\n", param->network));
		ret = RET_FAILED_CONNECTION;
		goto error;
	}
	WL_TRACE((" Starting SNR Test\n"));
	ret = pet_snrtest(param, wlh);
	if (ret != RET_SUCCESS)
		goto error;
	pet_log(param, ANT_CON_TEST, ret);
	WL_LOG(("\n##### PASS ANTENNA CONNECTIVITY TEST\n"));
	return ret;
error:
	WL_LOG((" Antenna connectivity test failed, err code %d \n", ret));
	WL_LOG(("\n#####FAIL ANTENNA CONNECTIVITY TEST\n"));
	pet_log(param, ANT_CON_TEST, ret);
	return ret;
}

int
pet_diag_btc_test(param_info_t *param, HANDLE wlh)
{
	int32 btc_wire;
	int ret;
	uint32 radio_status = 0;
	uint32 gpioin_status;
	uint32 read_mask;
	uint32 gmask = 0;
	uint32 boardflags = 0;

	ENTER();

	WL_LOG(("\n##### REPORT STATUS OF COEX_BT_ACTIVE AND W_DISABLE#\n"));

	ret = wlu_iovar_getint(wlh, "btc_wire", &btc_wire);
	if (ret != BCME_OK) {
		WL_FAILED(("Error in reading btc wire \n"));
		WL_DEBUG(("Err code %d \n", ret));
		ret = RET_ERROR_CARD_INFO;
		goto error;
	}
	WL_DEBUG((" Btc Wire %d\n", btc_wire));

	ret =  wlu_iovar_get(wlh, "boardflags", &boardflags, sizeof(boardflags));
	if (ret != BCME_OK) {
		WL_FAILED(("Error in reading board flags \n"));
		WL_DEBUG(("Err code %d \n", ret));
		ret = RET_ERROR_CARD_INFO;
		goto error;
	}
	WL_DEBUG((" Boardflags 0x%x\n", boardflags));

	/* Pin 4 - BT-ACT Pin 5 - WLAN_ACT */
	gmask = BOARD_GPIO_BTCMOD_OUT | BOARD_GPIO_BTCMOD_IN;

	if ((btc_wire == WL_BTC_2WIRE) && ((boardflags & BFL_BTC2WIRE_ALTGPIO) == 0))
		gmask = BOARD_GPIO_BTC_OUT | BOARD_GPIO_BTC_IN;

	WL_DEBUG((" gmask 0x%x\n", gmask));

	/* Get Radio status */
	ret = wlu_get(wlh, WLC_GET_RADIO, &radio_status, sizeof(radio_status));
	if (ret) {
		WL_FAILED((" Error in reading radio status\n"));
		WL_DEBUG(("Err code %d \n", ret));
		ret = RET_ERROR_CARD_INFO;
		goto error;
	}
	WL_DEBUG((" Radio Status = 0x%x\n", radio_status));
	/* TBD: Returning software status?? */
	WL_LOG((" W_DISABLE# status is %s\n", radio_status ? "high" : "low"));

	/* Set GPIO Parameters */
	ret =  wlu_iovar_get(wlh, "ccgpioin", &gpioin_status, sizeof(gpioin_status));
	if (ret != BCME_OK) {
		WL_FAILED((" Error in reading GPIO in status \n"));
		WL_DEBUG(("Err code %d \n", ret));
		ret = RET_ERROR_CARD_INFO;
		goto error;
	}
	if (gmask & BOARD_GPIO_BTCMOD_OUT)
		read_mask = BOARD_GPIO_BTCMOD_IN;
	else
		read_mask = BOARD_GPIO_BTC_IN;

	WL_DEBUG((" Read mask is 0x%x\n", read_mask));

	WL_LOG(("##### COEX_BT_ACTIVE signal is %s\n",
	(gpioin_status & read_mask) ? "high" : "low"));

	WL_LOG(("\n##### PASS STATUS OF COEX_BT_ACTIVE AND W_DISABLE#\n"));
	EXIT();
	pet_log(param, WLAN_COEX_TEST, ret);
	return ret;
error:
	ret = RET_ERROR_CARD_INFO;
	WL_LOG(("##### Error in talking to WL Driver\n"));
	WL_LOG(("\n##### FAIL STATUS OF COEX_BT_ACTIVE AND W_DISABLE#\n"));
	pet_log(param, WLAN_COEX_TEST, ret);
	EXIT();
	return ret;
}

int
pet_diag_wlanact_test(param_info_t *param, HANDLE wlh, BOOL enable)
{
	int32 btc_wire;
	int ret;
	uint32 radio_status = 0;
	uint32 gpio_out_params[2];
	uint32 gmask = 0, gout = 0;
	uint32 boardflags = 0;

	ENTER();

	WL_LOG(("\n##### %s COEX_WLAN_ACTIVE Signal Test \n",
		enable ? "ENABLE" : "DISABLE"));

	ret = wlu_iovar_getint(wlh, "btc_wire", &btc_wire);
	if (ret != BCME_OK) {
		WL_FAILED(("Error in reading btc wire \n"));
		WL_DEBUG(("Err code %d \n", ret));
		ret = RET_ERROR_CARD_INFO;
		goto error;
	}
	WL_DEBUG((" Btc Wire %d\n", btc_wire));

	ret =  wlu_iovar_get(wlh, "boardflags", &boardflags, sizeof(boardflags));
	if (ret != BCME_OK) {
		WL_FAILED(("Error in reading board flags \n"));
		WL_DEBUG(("Err code %d \n", ret));
		ret = RET_ERROR_CARD_INFO;
		goto error;
	}
	WL_DEBUG((" Boardflags 0x%x\n", boardflags));

	/* Pin 4 - BT-ACT Pin 5 - WLAN_ACT */
	gmask = BOARD_GPIO_BTCMOD_OUT | BOARD_GPIO_BTCMOD_IN;

	if ((btc_wire == WL_BTC_2WIRE) && ((boardflags & BFL_BTC2WIRE_ALTGPIO) == 0))
		gmask = BOARD_GPIO_BTC_OUT | BOARD_GPIO_BTC_IN;

	if (!enable) {
		gmask = BOARD_GPIO_BTCMOD_OUT;
		if ((btc_wire == WL_BTC_2WIRE) && ((boardflags & BFL_BTC2WIRE_ALTGPIO) == 0))
			gmask = BOARD_GPIO_BTC_OUT;
	}
	else {
		gmask = BOARD_GPIO_BTCMOD_OUT | BOARD_GPIO_BTCMOD_IN;
		if ((btc_wire == WL_BTC_2WIRE) && ((boardflags & BFL_BTC2WIRE_ALTGPIO) == 0))
			gmask = BOARD_GPIO_BTC_OUT | BOARD_GPIO_BTC_IN;
	}
	WL_DEBUG((" gmask 0x%x, gout 0x%x \n", gmask, gout));

	/* Down WL Driver, (to correctly detect BT is active ?? */
	ret = wlu_set(wlh, WLC_DOWN, NULL, 0);
	if (ret != BCME_OK) {
		WL_FAILED(("Could not down WL Driver \n"));
		WL_DEBUG(("Err code %d \n", ret));
		ret = RET_ERROR_CARD_INFO;
		goto error;
	}

	/* mask */
	gpio_out_params[0] = gmask;
	/* val */
	gpio_out_params[1] = enable? gmask : 0;

	/* Set GPIO Parameters */
	ret = wlu_iovar_set(wlh, "gpioout", gpio_out_params, sizeof(gpio_out_params));
	if (ret != BCME_OK) {
		WL_FAILED((" Error in setting GPIO Params\n"));
		WL_DEBUG((" Err code %d\n", ret));
		ret = RET_ERROR_CARD_INFO;
		goto error;
	}

	WL_LOG(("\n##### PASS %s COEX_WLAN_ACTIVE Signal Test\n",
		enable ? "ENABLE" : "FALSE"));
	EXIT();
	pet_log(param, W_DISABLE_TETS, ret);
	return ret;
error:
	WL_LOG((" Error code %d \n", ret));
	WL_LOG(("\n##### FAIL %s COEX_WLAN_ACTIVE Signal Test\n",
		enable ? "ENABLE" : "FALSE"));
	EXIT();
	pet_log(param, W_DISABLE_TETS, ret);
	return ret;
}

/* initialize the device for the test */
int pet_dump_verinfo(param_info_t *param, HANDLE wlh)
{
	cmd_t *cmd_ver;
	int err = RET_SUCCESS;
	/* Version parameters */
	char *ver_params[2]   = { "ver", NULL };

	UNREFERENCED_PARAMETER(param);

	ENTER();

	/* Get the ver function */
	cmd_ver = find_callback("ver");
	if (!cmd_ver) {
		err = RET_ERROR_CARD_INFO;
		WL_LOG(("Error in getting version info\n"));
		goto error;
	}
	WL_DEBUG((" Invoking versio callback\n"));
	/* Call verinfo function */
	err = (cmd_ver->func)(wlh, cmd_ver, ver_params);
	if (err != BCME_OK) {
		err = RET_ERROR_CARD_INFO;
		WL_LOG(("Error in getting version info\n"));
		goto error;
	}

	WL_TRACE((" Successfully retrieved version info\n"));
	EXIT();
	return err;
	/* more stuff from wl_ndis.c goes here */
error:
	WL_LOG(("Error in getting verion information \n", err));
	WL_DEBUG(("Err code %d \n", err));
	EXIT();
	return err;
}


/* TBD: Need to get this fixed. */
int
pet_diag_led_test(param_info_t *param, HANDLE wlh)
{
	uint32 ledoe = WLC_ACTIVITY_LED | WLC_RADIO_LEDS | 0x8;
	uint32 boardflags2;
	int ret;
	uint32 off = 0;

	ENTER();

	WL_LOG(("\n##### TURNING %s  WLAN LED \n",
		param->led_state ? "ON" : "OFF"));

	/* Down WL Driver */
	ret = wlu_set(wlh, WLC_DOWN, NULL, 0);
	if (ret != BCME_OK) {
		WL_FAILED(("Could not take control of Driver \n"));
		WL_DEBUG(("Err code %d \n", ret));
		ret = RET_ERROR_CARD_INFO;
		goto error;
	}

	ret =  wlu_iovar_get(wlh, "boardflags2", &boardflags2, sizeof(boardflags2));
	if (ret != BCME_OK) {
		WL_FAILED(("Error in reading board flags \n"));
		WL_DEBUG(("Err code %d \n", ret));
		ret = RET_ERROR_CARD_INFO;
		goto error;
	}
	WL_DEBUG((" Boardflags 0x%x\n", boardflags2));

	if (boardflags2 & BFL2_TRISTATE_LED)
		ret =
			wlu_iovar_set(wlh, "ccgpioouten", param->led_state ? &ledoe : &off,
			sizeof(ledoe));
	else
		ret =
			wlu_iovar_set(wlh, "ccgpioout", param->led_state ? &off : &ledoe,
			sizeof(ledoe));

	if (ret != BCME_OK) {
		WL_FAILED((" Error in setting GPIO pins(1)\n"));
		WL_DEBUG((" Err code %d\n", ret));
		ret = RET_ERROR_CARD_INFO;
		goto error;
	}

	if (param->led_chkstatus) {
		char c;
		printf("\nDo you see WLAN Led turned %s ?\n",
		param->led_state ? "ON" : "OFF");
		printf("\nPress:\nY to pass the test.\n "
			"Or press Enter to fail the test...\n\n>>>");
		do {
			c = getchar();
		}  while (c != '\n' && c != 'y' && c != 'Y');

		if (c != 'y' && c != 'Y') {
			WL_FAILED(("Failed reported by user.\n"));
			ret = RET_ERROR_LED_TEST_FAILED;
			goto error;
		}
	}
	if (param->led_persistent != param->led_state) {
		WL_LOG((" Keep %s state after exit\n", param->led_state ? "OFF" : "ON"));
		off = 0;
		if (boardflags2 & BFL2_TRISTATE_LED)
			ret =
				wlu_iovar_set(wlh, "ccgpioouten", param->led_state ? &off: &ledoe,
				sizeof(ledoe));
		else
			ret =
				wlu_iovar_set(wlh, "ccgpioout", param->led_state ? &ledoe : &off,
				sizeof(ledoe));

		if (ret != BCME_OK) {
			WL_FAILED((" Error in setting GPIO pins(2) \n"));
			WL_DEBUG((" Err code %d\n", ret));
			ret = RET_ERROR_CARD_INFO;
			goto error;
		}
	}

	WL_LOG(("\n##### PASS TURNING %s  WLAN LEDs TEST\n",
	param->led_state ? "ON" : "OFF"));
	EXIT();
	pet_log(param, WLAN_LED_TEST, ret);
	return ret;

error:
	WL_LOG((" Error code %d \n", ret));
	WL_LOG(("\n##### FAIL TURNING %s  WLAN LED\n",
		param->led_state ? "ENABLE" : "FALSE"));
	EXIT();
	pet_log(param, WLAN_LED_TEST, ret);
	return ret;
}
