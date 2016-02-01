/******************************************************************************
#     (c)2003-2013 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
#
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <string.h>
#include <syslog.h>
#include <netinet/in.h>
#include <netdb.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "nexus_platform.h"
#include "nexus_platform_client.h"
#include "nxclient.h"
#include "../inc/appdefs.h"
#include "utils.h"
#include "event.h"
#include "informer.h"
#include "../SOAPParser/CPEframework.h"
#include "../webproto/protocol.h"
#include "../bcmLibIF/bcmWrapper.h"
#include "../bcmLibIF/tr135/bcmStbService.h"
#include "../SOAPParser/RPCState.h"
#include "../inc/tr69cdefs.h"
#include "types.h"
#include "tr69clib.h"
#include "tr69clib_priv.h"
#ifdef TR181_SUPPORT
#include "moca2.h"
#include "ethernet.h"
#endif
#ifdef TR135_SUPPORT
#include "bcmTR135Objs.h"
#endif

#define CONF_FILE_PATH "/mnt/flash/data/tr69c.conf"

extern ACSState acsState;
vendorSpecificOption option;
b_tr69c_server_t tr69c_server;

int tr69cTerm = 0;	/*TR69C termination flag*/

void sigIgnore(int sig)
{
	BSTD_UNUSED(sig);
    slog(LOG_DEBUG, "Ignore sig %d");
    return;
}

void tr69c_sigTermHandler(int sig)
{
	BSTD_UNUSED(sig);
	/*printf("TR69C terminating......\n");*/
 	tr69cTerm = 1;

	/*Set the timer to send out an inform to ACS.  The purpose is to
	 *create an ACS disconnect event so that the tr69c termination
	 *flag will be examined and acted on.
	 */
	resetPeriodicInform(1);
}

/*
* Initialize all the various tasks
 */
static void initTasks(void)
{
	/* INIT Protocol http, ssl */
	printf("init protocol\n");
	proto_Init();
	/* init bcm library interface */
	printf("init BCMLibIF\n");
	initBCMLibIF();
	/* Just booted so send initial Inform */
	printf("init Informer\n");
	initInformer();
}

static void daemonize(void)
{
    slog(LOG_DEBUG,"Daemonizing process ");
    if (fork()!=0) exit(0);
    setsid();
}

void init_tr69_def(void)
{
	if (option.acsUrl)
		acsState.acsURL	= strdup(option.acsUrl);
	else
		acsState.acsURL	= strdup(ACS_URL);
	acsState.acsUser = strdup(ACS_USER);
	acsState.acsPwd = strdup(ACS_PASSWD);
	acsState.connReqPath = strdup(ACSCONNPATH);
	acsState.connReqUser = strdup(ACSCONN_USER);
	acsState.connReqPwd = strdup(ACSCONN_PASSWD);
	acsState.kickURL = strdup(ACS_KICK_URL);
	if (option.provisioningCode)
		acsState.provisioningCode = strdup(option.provisioningCode);
	else
		acsState.provisioningCode = strdup(ACS_DEF_STRING);
	acsState.downloadCommandKey = NULL;
	acsState.rebootCommandKey = NULL;
	acsState.rebootCommandKey = NULL;
	acsState.parameterKey = strdup(ACS_DEF_STRING);
	acsState.newParameterKey = NULL;
	acsState.upgradesManaged = 1;
	acsState.dlFaultStatus = 0;
	acsState.startDLTime = 0;
	acsState.endDLTime = 0;
	acsState.informEnable = 1;
	acsState.informInterval = 50;
	acsState.informTime = 1;
}

static int parse_dhcpv4_option125(vendorSpecificOption *pOption)
{
	FILE *fp;
    fpos_t pos, pos2, curpos;
	char buf[512], data[512];
    char *str, *token[10], *subtoken[100], *saveptr;
	int i, j, offset, option_code, data_length;
	int found_vivso = 0;

	if (!(fp = fopen("/var/db/dhclient.leases", "r")))
	{
        return found_vivso;
	}

    while (!feof(fp))
    {
        fgetpos(fp, &curpos);

        if (fgets(buf, 1024, fp) != NULL)
        {
            if (strncmp(buf, "lease {", 7) == 0)
            {
                pos = curpos;
            }
            else if (strstr(buf, "option vendor.unknown-3561"))
            {
                found_vivso = 1;
                pos2 = pos;
                continue;
            }
        }
    }

    if (found_vivso)
    {
        fsetpos(fp, &pos2);
    }
    else
    {
        fclose(fp);
        return found_vivso;
    }

	memset(buf, 0, 512);
	memset(token, 0, 10);
	memset(subtoken, 0, 100);

    while (!feof(fp))
	{
		if (fgets(buf, 512, fp) != NULL)
		{
            if (*buf == '}') break;

            for (i = 0, str = buf; ;i++, str = NULL)
            {
                token[i] = strtok_r(str, " ", &saveptr);
                if (token[i] == NULL)
                    break;
            }

            if (!strcmp(token[0], "option") && strstr(token[1], "3561"))
            {
                for (i = 0, str = token[2]; ;i++, str = NULL)
                {
                    subtoken[i] = strtok_r(str, ":;", &saveptr);
                    if (subtoken[i] == NULL)
                        break;
                }

                offset = 0;
                for (j = 0; j < 2; j++)
                {
                    memset(data, 0, 512);

                    option_code = strtol(subtoken[offset+0], NULL, 16);
                    data_length = strtol(subtoken[offset+1], NULL, 16);
                    for (i = 0; i < data_length; i++)
                        data[i] = strtol(subtoken[offset+2+i], NULL, 16);
                    data[i] = '\0';
                    offset += 2 + data_length;
                    if (j == 0 && option_code == 11)
                        pOption->acsUrl = strdup(data);
                    else if (j == 1 && option_code == 12)
                        pOption->provisioningCode = strdup(data);
                }
                for (j = 2; j < 4; j++)
                {
                    memset(data, 0, 512);

                    option_code = strtol(subtoken[offset+0], NULL, 16);
                    data_length = strtol(subtoken[offset+1], NULL, 16);
                    for (i = 0; i < data_length; i++)
                        strcat(data, subtoken[offset+2+i]);
                    offset += 2 + data_length;
                    if (j == 2 && option_code == 13)
                        pOption->cwmpRetryMinimumWaitInterval = strtol(data, NULL, 16);
                    else if (j == 3 && option_code == 14)
                        pOption->cwmpRetryIntervalMultiplier = strtol(data, NULL, 16);
                }
            }
		}
	}

	fclose(fp);
	return found_vivso;
}

int main(int argc, char** argv)
{
	NxClient_JoinSettings joinSettings;
	NEXUS_Error rc;
    int verbose = 0;
    int no_daemonize=0;
    int c;
#ifdef TR135_SUPPORT
	struct timeval tv;
	unsigned startTime;
	b_total_t *total;
	b_mainStream_t *mainStream;
#endif

	NxClient_GetDefaultJoinSettings(&joinSettings);
	strcpy(joinSettings.name, "status"); /* request for eVerified mode */
	rc = NxClient_Join(&joinSettings);
	if (rc) { printf("Failed to join\n"); return -1; }

	if (parse_dhcpv4_option125(&option))
	{
		printf("===================================================\n");
		printf("DHCPv4 option 125 found\n");
		printf("ManagementServer.URL: %s\n", option.acsUrl);
		printf("DeviceInfo.ProvisioningCode: %s\n", option.provisioningCode);
		printf("ManagementServer.CWMPRetryMinimumWaitInterval: %d\n", option.cwmpRetryMinimumWaitInterval);
		printf("ManagementServer.CWMPRetryIntervalMultiplier: %d\n", option.cwmpRetryIntervalMultiplier);
		printf("===================================================\n");
	}

	printf("Initialize basic config data from default values\n");
	init_tr69_def();

    while ((c=getopt(argc, argv, "dv")) != -1) {
        switch (c) {
        case 'v':
            verbose = 1;
            break;
        case 'd':
            no_daemonize = 1;
            break;
        default:
            break;
        }
    }

    initLog(verbose);
    if (!no_daemonize)
        daemonize();

    /* set signal masks */
    signal(SIGPIPE, SIG_IGN); /* Ignore SIGPIPE signals */
    signal(SIGTERM, tr69c_sigTermHandler);

    /* init a tr69c_socket */
    tr69c_server = b_tr69c_server_init();
    /* init a monitor for the moca link */
#ifdef TR181_SUPPORT
    moca_link_state_monitor_init();
    ethernet_link_state_monitor_init();
#endif
	/* init tr135 data objects */
#ifdef TR135_SUPPORT
	init_135();
	gettimeofday(&tv, NULL);
	startTime = tv.tv_sec;
	getTotal(&total);
	total->startTime= startTime;
	setTotal(total);
#endif

#ifdef SPECTRUM_ANALYZER
	char *value;
	char *freq;
	FILE *fp;

	char tmp[10];
	char *sep = ", ";
	char freqArray[MAX_NUMSAMPLES*DEFAULT_NUMENTRIES*2*10] = "\0";
	unsigned i;
	spectrumAnalyzer *spectrum;

	value = malloc(1024);
	freq = malloc(128);
	spectrum = getSpectrumAnalyzer();

	printf("numAverages = %d numSamples = %d nunEntries = %d measurementPerBin = %d \n", spectrum->numAverages,
			spectrum->numSamples, spectrum->measurementTableNumberOfEntries, spectrum->measurementsPerBin);

	for (i=0; i < (spectrum->numSamples)* (spectrum->measurementTableNumberOfEntries); i++)
	{
		int freq;
		if (i==0)
			freq = 0;
		else
			freq += ((MAX_FREQ/1000)/((spectrum->numSamples)*(spectrum->measurementTableNumberOfEntries)));
		snprintf(tmp, 10, "%d", freq);
		strncat(freqArray, tmp, strlen(tmp));
		if (i < (spectrum->numSamples)* (spectrum->measurementTableNumberOfEntries) -1)
			strncat(freqArray, sep, strlen(sep));
	}
	fp = fopen("freq.txt", "w");
	fwrite(freqArray, 1 , strlen(freqArray), fp);
	fclose(fp);


	getAmplitudeData(&value);
	/*printf("Amplitude %s \n", value);*/
	fp = fopen("amp.txt", "w");
	fwrite(value, 1 , strlen(value), fp);
	fclose(fp);
	system("awk 'FNR==2{print ""}1' freq.txt amp.txt > test.txt");
	system("cp test.txt test.csv");

#endif

#ifdef XML_DOC_SUPPORT
	BcmTr69PrintTree("Device", 0, true);
    return 0;
#endif

	printf("InitTask\n");
    initTasks();
    printf("Before eventLoop\n");
    eventLoop();

    /*Clean up spectrum data pointer */
#ifdef TR135_SUPPORT
	dataCleanUp();
#endif
    /* uninit the monitor for the moca link */
#ifdef TR181_SUPPORT
    moca_link_state_monitor_uninit();
    ethernet_link_state_monitor_uninit();
#endif
    /* uninit tr69c_socket */
    b_tr69c_server_uninit(tr69c_server);
    /* example shutdown */
    NxClient_Uninit();

    return 0;
}

