/*
 *
 * hwdiags_winpe.h -- Definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: hwdiags_winpe.h,v 1.1104.2.62.2.5 2010-12-27 06:50:50 $
 */

#ifndef _HWDIAGS_WINPE_H
#define _HWDIAGS_WINPE_H


#define DEFAULT_RSSI		-65
#define DEFAULT_AUX_RSSI	DEFAULT_RSSI
#define DEFAULT_MAIN_RSSI   DEFAULT_RSSI

#define WL_MAX_SCANRESULTS 256
#define SCAN_INTERVAL   500
#define SCAN_RESULTS_TO 10000
#define PET_STATUS_UPDATE_TO   2000


/* also defined in wlc_diag.c */
#define WLC_ACTIVITY_LED	1	/*  activity LED is GPIO bit 0 */
#define WLC_RADIO_LED_B		2	/*  radio LED from GPIO bit 1 */
#define WLC_RADIO_LED_A		4	/*  radio LED from GPIO bit 2 */
#define WLC_RADIO_LEDS		6	/*  all radio LEDs: GPIO bit 1 and bit 2 */


/* Defining Test Numbers */

#define CARD_VER_TEST 	1	/* Card Verification Test */
#define WLAN_SW_TEST 	2	/* WLAN Switch Test */
#define WLAN_LED_TEST 	3	/* WLAN LED Test */
#define WLAN_COEX_TEST 	4	/* WLAN COEX Signal status Test */
#define W_DISABLE_TETS	5	/* W_DISABLE# Signal status Test */
#define ANT_CON_TEST	6	/* Antenna Connectivity Test */

/* Assosiation cofiguration */

#define MAX_ASSOC_ATTEMPT  1
#define DEF_ASSOC_TIME 15
#define MAX_RSSI_ERR_CNT 10
#define MAX_RSSI_RUNS 5


/* applicable main function return code */
enum {
RET_SUCCESS,
RET_CTRLC_SIG,
RET_NO_WIRELESS_CARD_FOUND,
RET_ERROR_CL_READ,
RET_ERROR_CARD_INFO,
RET_FAILED_CONNECTION,
RET_UNABLE_TO_ENABLE_CARD,
RET_ERROR_ANTDIV,
RET_ERROR_WLRADIO_OFF,
RET_ERROR_RSSI_MAIN_ANT,
RET_ERROR_RSSI_AUX_ANT,
RET_ERROR_RSSI_MAIN_ANT_DELTA,
RET_ERROR_RSSI_AUX_ANT_DELTA,
RET_ERROR_OUT_OF_MEMORY,
RET_ERROR_LED_TEST_FAILED,
/* Used only in SNR test not to be published */
RET_ERROR_SNR_MAIN_ANT,
RET_ERROR_SNR_AUX_ANT,
RET_ERROR_SNR_MAIN_ANT_DELTA,
RET_ERROR_SNR_AUX_ANT_DELTA,
LAST_ERROR_CODE
} pet_error_codes;
/* Error Codes Help */

#define	WLC_RSSI_MINVAL		-200	/* Low value, e.g. for forcing roam */
#define	WLC_RSSI_NO_SIGNAL	-91	/* NDIS RSSI link quality cutoffs */
#define	WLC_RSSI_VERY_LOW	-80	/* Very low quality cutoffs */
#define	WLC_RSSI_LOW		-70	/* Low quality cutoffs */
#define	WLC_RSSI_GOOD		-68	/* Good quality cutoffs */
#define	WLC_RSSI_VERY_GOOD	-58	/* Very good quality cutoffs */
#define	WLC_RSSI_EXCELLENT	-57	/* Excellent quality cutoffs */

/* command line parameters */
typedef struct param_info {
	/* SSID of the AP to connect to. */
	char	network[DOT11_MAX_SSID_LEN+1];	/* SSID of AP */

	/* Card IDs */
	ushort 	vid;
	ushort 	pid;
	ushort 	svid;
	ushort 	sdevid;
	ULONG 	irq;

	/* Antenna Connectivity Tests */
	bool	antm;		/* Test RSSI using main antenna   */
	int	    rssim;		/* min. RSSI for main antenna   */
	bool	anta;		/* Test RSSI using AUX antenna */
	int	    rssia;		/* min. RSSI for aux. antenna   */
	bool	ant;		/* Test both Main & Aux antennas */
	int     rssi_delta; /* Threshold delta b/w RSSIs */
	uint    retrycount;	/* number of repeats of the link test */
	uint	max_assoc;  /* Max number of retries to associate */
	uint	assoc_time; /* The amount of time application waits to get assosiated in secs */

	/* GPIO Tests */
	bool	led;       /* perform led test */
	uint	led_state; /* on/off */
	uint	led_persistent; /* yes/no */
	bool	led_chkstatus; /* Check with operator for the status */
	bool	wlact;     /* perform COEX_WLAN_ACTIVE */
	uint	wlact_state; /* on/off */
	uint	wlact_persistent; /* yes/no */
	bool	pins; /* W_DISABLE# and COEX_WLAN_ACTIVE State */
	uint	w_disable_state; /* on/off */
	uint	bta_state; /* on/off */

	/* Wireless LAN Switch Status */
	bool    ws;
	uint	ws_state;  /* Whether switch is on/off */

	/* Logging */
	FILE	*fp;	/* fopen pointer of logfile */
	uchar	logfile[256];	/* log filename */
	/* logging modifier:
	 *	0 nothing logged
	 *	1 only failed tests are logged
	 *	2 everything is logged
	 *	3 debug messages are also logged
	 */
	uint	fmodifier;	/* logfile modifier */
	uint	smodifier;	/* screen log modifier */

	/* Print only card info */
	bool	notest;

	/* Data to perform tests */
	uint    band;
	uint	subsystemVendorID;
	uint 	vendor;

	int		noise;
    bool    snr_enb;

	/* Verinformation */
    bool    ver;

	uint	snrm;		/* min. SNR for main antenna   */
	uint	snra;		/* min. SNR for aux. antenna   */
    uint    snr;		/* min. SNR for both antenna's */
	bool	anta0;		/* test aux. or aux 0 (for NPHY) antenna */
	bool	anta1;		/* test aux 1 antenna for NPHY only */
	bool	rep_devs;	/* If set the application reports supported devices  */
	/* Max allowed delta for snr values relative to the max snr
		values on all antennaes
	*/

	uint    snr_delta;
}  param_info_t;

typedef struct _devinfo {
    ushort	deviceID;
    ushort	vendorID;
    ushort	subsystemVendorID;
    ushort	subsystemID;
    ulong   nDevice;
} devinfo_t;

struct devid {
    ushort device;
    ushort vendor;
};

typedef struct logparams {
	uint	fmodifier;	/* logfile modifier */
	uint	smodifier;	/* screen log modifier */
	FILE	*fp;
} logparams_t;

char *pet_log_msg; /* Debug info */
logparams_t s_log;


#define LOG_NONE	0
#define LOG_FAILED	1
#define LOG_WARN	2
#define LOG_SNR		3
#define LOG_TRACE	4
#define LOG_DEBUG	5
#define LOG_RESULTS	1

#define LOG_FILE	"rftest_o.txt"

#define WL_FAILED(args)	do {wl_log_level = LOG_FAILED; update_pet_log_msg args; tee args;} while (0)
#define WL_WARN(args)	do {wl_log_level = LOG_WARN; tee args;} while (0)
#define WL_SNRLOG(args) do {wl_log_level = LOG_SNR; tee args;} while (0)
#define WL_TRACE(args)	do {wl_log_level = LOG_TRACE; tee args;} while (0)
#define WL_DEBUG(args)	do {wl_log_level = LOG_DEBUG; tee args;} while (0)
#define WL_LOG(args)	do {wl_log_level = LOG_RESULTS; tee args;} while (0)

#define ENTER()			WL_DEBUG(("===>Enter: %s\n", __FUNCTION__))
#define EXIT()			WL_DEBUG(("<===Exit: %s\n", __FUNCTION__))


/*
	We start allocating our own error codes at 20
	to allow wlc_attahc() rooom to grow.
*/
#define STATUS_RESOURCES	20
#define STATUS_FAILURE		21
#define STATUS_NODEVICE		22
#define STATUS_HWRESOURCES	23
#define STATUS_TXRESOURCES	24
#define STATUS_RXRESOURCES	25
#define STATUS_WLALLOC		26
#define STATUS_WLRADIOOFF	27

#define STATUS_ERROR(x)  (x != STATUS_SUCCESS)

/* legacy rx Antenna diversity for SISO rates */
#define	ANT_RX_DIV_FORCE_0		0	/* Use antenna 0 */
#define	ANT_RX_DIV_FORCE_1		1	/* Use antenna 1 */
#define	ANT_RX_DIV_START_1		2	/* Choose starting with 1 */
#define	ANT_RX_DIV_START_0		3	/* Choose starting with 0 */
#define	ANT_RX_DIV_ENABLE		3	/* APHY bbConfig Enable RX Diversity */
#define ANT_RX_DIV_DEF		ANT_RX_DIV_START_0	/* default antdiv setting */

/* legacy rx Antenna diversity for SISO rates */
#define ANT_TX_FORCE_0		0	/* Tx on antenna 0, "legacy term Main" */
#define ANT_TX_FORCE_1		1	/* Tx on antenna 1, "legacy term Aux" */
#define ANT_TX_LAST_RX		3	/* Tx on phy's last good Rx antenna */
#define ANT_TX_DEF			3	/* driver's default tx antenna setting */

extern	param_info_t param;
/* Diag Logging Functions */

typedef void * DEVHANDLE;

typedef DWORD status_t;

#define SIZEOF_ARRAY(a) (sizeof(a)/sizeof(a[0]))

typedef DWORD status_t;

#define SIZEOF_ARRAY(a) (sizeof(a)/sizeof(a[0]))

/* Function name primitive */
extern char* str_line(char *file, uint line);

/* Starting function of pet */
extern int pet_start(void *irh, int argc, char **argv);

/* Scan all PCI Devices */
extern void pci_scan();

/* Find the whether found wireless device is in the list */
extern int pet_diag_check_wlancard(param_info_t *param, HANDLE wlh);

/* Check status of WLAN HW Switch */
extern int pet_diag_hwradio_status(param_info_t *param, HANDLE wlh);

/* initialize the device for the test */
extern int pet_diag_init(param_info_t *param, HANDLE wlh);

/* Run antenna connectivity test */
extern int pet_diag_antenna_test(param_info_t *param, HANDLE wlh);

/* Report BT_COEX and W_DISABLE Signal state */
extern int pet_diag_btc_test(param_info_t *param, HANDLE wlh);

/* Enable/disable COEX_WLAN_ACTIVE signal */
extern int pet_diag_wlanact_test(param_info_t *param, HANDLE wlh, BOOL enable);

/* LED On/Off Test */
extern int pet_diag_led_test(param_info_t *param, HANDLE wlh);

/* Dump version information */
extern int pet_dump_verinfo(param_info_t *param, HANDLE wlh);

/* ctrlc signal handling function */
extern void pet_ctrlc_handler();

#endif /* _HWDIAGS_WINPE_H */
