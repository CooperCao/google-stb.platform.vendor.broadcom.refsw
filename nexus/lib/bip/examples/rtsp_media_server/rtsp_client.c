/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Utilities for rtsp client test application
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *************************************************************/

#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>

#include <assert.h>
#include <stdbool.h>

#include "TestUtils.h"
#include "IPTestUtils.h"
#include "rtspUtils.h"

/* Define the RTSP server parameters for a valid tunable stream */
#define STREAM_PARAMS "src=1&freq=1202&pol=v&ro=0.35&msys=dvbs&mtype=qpsk&plts=off&sr=20000&fec=56&pids=0,33,5922,5923,5924"

/* Initialized to no error by default.
 * Overridden by command line param -retval.
 * Needed for test automation purposes only */
int expected_retval = RTSP_OK;  /* RTSP OK */

/* RTSP server ip */
char *server;

/* Test function prototypes */
int SetupTeardown(void);
int SetupDescribeTearDown(void);
int SetupPlayTearDown(void);
int UnicastRTPDataVerify(void);
int UnicastRTPDataVerifyWithDescribe(void);
int MulticastRTSPWithPlay(void);
int MulticastRTSPWithIGMPJoin(void);

/* Test case details */
TestCaseInfo sTCInfo[] =
{
	{
		"Simple Setup and Teardown test",
		SetupTeardown,
	},
	{
		"Simple Setup Describe and Teardown",
		SetupDescribeTearDown,
	},
	{
		"Setup Play and Teardown test",
		SetupPlayTearDown,
	},
	{
		"Setup unicast RTSP session, read data and verify",
		UnicastRTPDataVerify,
	},
	{
		"Setup unicast RTSP session, read data and verify and issue describe to see ongoing RTSP sessions",
		UnicastRTPDataVerifyWithDescribe,
	},
	{
		"Setup multicast RTSP session and being RTP data streaming, by issue a PLAY and verify data",
		MulticastRTSPWithPlay,
	},
	{
		"Setup multicast RTSP session and being RTP data streaming, by sending IGMP Join and verify data",
		MulticastRTSPWithIGMPJoin,
	},
};

int num_tests = sizeof(sTCInfo)/sizeof(TestCaseInfo);

CLArgs args[] = {
    {"ipaddr",   0, 1, NULL, "RTSP Server IP Address"},
    {"testcase", 0, 1, NULL, "Test case id"},
    {"verbose",  0, 0, NULL, "Verbose output"},
    {"help",     0, 0, NULL, "Print Usage"},
    {"retval",   0, 1, NULL, "Expected return value"}
};

int rtsp_port = 554;

void usage(char *argv0)
{
    printf ("Usage: %s -ipaddr <server_ip> -testcase <test_id> [-verbose -help]\n", argv0);
    printf("options are:\n");
    printf(" -ipaddr <server_ip> # IP address of RTSP Server (e.g. 192.168.1.110)\n");
    printf(" -testcase <test_id> # Test case id number (1 - %d)\n", num_tests);
    printf(" -verbose            # print verbose information (ex: RTSP command REQ/RESP) (default: no)\n");
    printf(" -help               # prints rtsp client usage\n");
    printf("\n");
}

int rtsp_test_init(char *server)
{
    int sd;
    /* Open TCP connection */
    sd = tcpConnect(server, rtsp_port, 0);
    if (sd < 0)
    {
        printf ("Error opening socket for RTSP session\n");
        return SOCKET_ERROR;
    }

    /* Set server ip and sock addr params */
    setServerIp(server);
    setSockDesc(sd);

    return NO_ERROR;
}

void rtsp_test_shutdown(int id)
{
    closeConnection(id);
}

int SetupTeardown(void)
{
    int ports[2] = {5001, 5002}, ret = NO_ERROR;
    char cmd[128];
    RTSP_SessionInfo session_info;

    /* Setup a UNICAST session */
    sprintf (cmd, "rtsp://%s:%d/?" STREAM_PARAMS, server, rtsp_port);
    ret = setup(ports, false, cmd, &session_info);
    if (ret != RTSP_OK)
    {
        printf ("SETUP Error!\n"); return ret;
    }
    printSessionInfo(&session_info);

    /* Teardown session */
    ret = teardown(session_info.session_id, NULL, session_info.stream_id);
    if (ret != RTSP_OK)
    {
        printf ("TEARDOWN Error!\n");
    }

    RETURN_TEST_RESULT(ret);
}

int SetupDescribeTearDown(void)
{
    int ports[2] = {5001, 5002}, ret = NO_ERROR;
    char cmd[128];
    RTSP_SessionInfo session_info;
    int sz = 5;
    RTSP_StreamInfo strminfo[5];

    /* Setup a UNICAST session */
    sprintf (cmd, "rtsp://%s:%d/?" STREAM_PARAMS, server, rtsp_port);
    ret = setup(ports, false, cmd, &session_info);
    if (ret != RTSP_OK)
    {
        printf ("SETUP Error!\n"); return ret;
    }
    printSessionInfo(&session_info);

    /* Do a describe to see ongoing server sessions */
    ret = describe(session_info.session_id, strminfo, &sz);
    if (ret < 0)
    {
        printf ("DESCRIBE Error!\n"); return ret;
    }

    /* Teardown session */
    ret = teardown(session_info.session_id, NULL, session_info.stream_id);
    if (ret != RTSP_OK)
    {
        printf ("TEARDOWN Error!\n");
    }

    RETURN_TEST_RESULT(ret);
}

int SetupPlayTearDown(void)
{
    int ports[2] = {5001, 5002}, ret = NO_ERROR;
    char cmd[128];
    RTSP_SessionInfo session_info;
    char rtp_info[128];

    /* Setup a UNICAST session */
    sprintf (cmd, "rtsp://%s:%d/?" STREAM_PARAMS, server, rtsp_port);
    ret = setup(ports, false, cmd, &session_info);
    if (ret != RTSP_OK)
    {
        printf ("SETUP Error!\n"); return ret;
    }
    printSessionInfo(&session_info);

    /* Start Playing */
    ret = play(session_info.session_id, NULL, session_info.stream_id, rtp_info);
    if (ret != RTSP_OK)
    {
        printf ("PLAY Error!\n"); return ret;
    }

    /* Teardown session */
    ret = teardown(session_info.session_id, NULL, session_info.stream_id);
    if (ret != RTSP_OK)
    {
        printf ("TEARDOWN Error!\n");
    }

    RETURN_TEST_RESULT(ret);
}

int UnicastRTPDataVerify(void)
{
    int ports[2] = {5001, 5002}, ret = NO_ERROR;
    char cmd[128];
    RTSP_SessionInfo session_info;
    char rtp_info[128];

    /* Setup a UNICAST session */
    sprintf (cmd, "rtsp://%s:%d/?" STREAM_PARAMS, server, rtsp_port);
    ret = setup(ports, false, cmd, &session_info);
    if (ret != RTSP_OK)
    {
        printf ("SETUP Error!\n"); return ret;
    }
    printSessionInfo(&session_info);

    /* Start Playing */
    ret = play(session_info.session_id, NULL, session_info.stream_id, rtp_info);
    if (ret != RTSP_OK)
    {
        printf ("PLAY Error!\n"); return ret;
    }

    /* Read and verify RTP data */
    ret = readAndValidateRtpData(session_info.src_addr, session_info.port[0], session_info.client_port[0]);
    if (ret < 0)
    {
        printf ("Error validating Unicast RTP data!\n"); return ret;
    }

    /* Teardown session */
    ret = teardown(session_info.session_id, NULL, session_info.stream_id);
    if (ret != RTSP_OK)
    {
        printf ("TEARDOWN Error!\n");
    }

    RETURN_TEST_RESULT(ret);
}

int UnicastRTPDataVerifyWithDescribe(void)
{
    int ports[2] = {5001, 5002}, ret = NO_ERROR;
    char cmd[128];
    RTSP_SessionInfo session_info;
    char rtp_info[128];
    int sz = 5;
    RTSP_StreamInfo strminfo[5];

    /* Setup a UNICAST session */
    sprintf (cmd, "rtsp://%s:%d/?" STREAM_PARAMS, server, rtsp_port);
    ret = setup(ports, false, cmd, &session_info);
    if (ret != RTSP_OK)
    {
        printf ("SETUP Error!\n"); return ret;
    }
    printSessionInfo(&session_info);

    /* Start Playing */
    ret = play(session_info.session_id, NULL, session_info.stream_id, rtp_info);
    if (ret != RTSP_OK)
    {
        printf ("PLAY Error!\n"); return ret;
    }

    /* Do a describe to see ongoing server sessions */
    ret = describe(session_info.session_id, strminfo, &sz);
    if (ret != RTSP_OK)
    {
        printf ("DESCRIBE Error!\n"); return ret;
    }

    /* Read and verify RTP data */
    ret = readAndValidateRtpData(session_info.src_addr, session_info.port[0], session_info.client_port[0]);
    if (ret < 0)
    {
        printf ("Error validating unicast RTP data!\n"); return ret;
    }

    /* Teardown session */
    ret = teardown(session_info.session_id, NULL, session_info.stream_id);
    if (ret != RTSP_OK)
    {
        printf ("TEARDOWN Error!\n");
    }

    RETURN_TEST_RESULT(ret);
}

int MulticastRTSPWithPlay(void)
{
    int mports[2] = {11992, 11993};
    char cmd[128];
    RTSP_SessionInfo session_info;
    int sz = 5, ret = NO_ERROR;
    RTSP_StreamInfo strminfo[5];

    /* Setup a MULTICAST session */
    sprintf (cmd, "rtsp://%s:%d/?src=1&freq=1202&pol=v&ro=0.35&msys=dvbs&mtype=qpsk&plts=off&sr=20000&fec=56&pids=0,33,5922,5923,5924", server, rtsp_port);
    ret = setup(mports, true, cmd, &session_info);
    if (ret != RTSP_OK)
    {
        printf ("SETUP Error!\n"); return ret;
    }
    printSessionInfo(&session_info);

    /* Start Playing - This is optional for multicast? */
    ret = play(session_info.session_id, NULL, session_info.stream_id, NULL);
    if (ret != RTSP_OK)
    {
        printf ("PLAY Error!\n"); return ret;
    }

    /* Read and verify RTP data */
    ret = readAndValidateMcastRtpData(session_info.dest_addr, session_info.client_port[0]);
    if (ret < 0)
    {
        printf ("Error validating Multicast RTP data !\n"); return ret;
    }

    /* Do a describe to see ongoing server sessions */
    ret = describe(NULL, strminfo, &sz);
    if (ret != RTSP_OK)
    {
        printf ("DESCRIBE Error!\n");
    }

    /* Teardown session */
    ret = teardown(session_info.session_id, NULL, session_info.stream_id);
    if (ret != RTSP_OK)
    {
        printf ("TEARDOWN Error!\n");
    }

    RETURN_TEST_RESULT(ret);
}


int MulticastRTSPWithIGMPJoin(void)
{
    int mports[2] = {11992, 11993};
    char cmd[128];
    RTSP_SessionInfo session_info;
    int src_addr;
    int sz = 5, ret = NO_ERROR;
    RTSP_StreamInfo strminfo[5];

    /* Setup a MULTICAST session */
    sprintf (cmd, "rtsp://%s:%d/?src=1&freq=1202&pol=v&ro=0.35&msys=dvbs&mtype=qpsk&plts=off&sr=20000&fec=56&pids=0,33,5922,5923,5924", server, rtsp_port);
    ret = setup(mports, true, cmd, &session_info);
    if (ret != RTSP_OK)
    {
        printf ("SETUP Error!\n"); return ret;
    }
    printSessionInfo(&session_info);

    /* Get IPV4 interface address */
    src_addr = getFirstIPV4Address();

    /* Send IGMP Join report */
    ret = sendIGMPMembershipReport(src_addr, 12000, session_info.dest_addr, JOIN_IGMP_GROUP);
    if (ret < 0)
    {
        printf ("IGMP JOIN Error!\n"); return ret;
    }

    /* Read and verify RTP data */
    ret = readAndValidateMcastRtpData(session_info.dest_addr, session_info.client_port[0]);
    if (ret < 0)
    {
        printf ("Error validating Multicast RTP data !\n"); return ret;
    }

    /* Send IGMP Leave report */
    ret = sendIGMPMembershipReport(src_addr, 12000, session_info.dest_addr, LEAVE_IGMP_GROUP);
    if (ret < 0)
    {
        printf ("IGMP LEAVE Error!\n"); return ret;
    }

    /* Do a describe to see ongoing server sessions */
    ret = describe(session_info.session_id, strminfo, &sz);
    if (ret != RTSP_OK)
    {
        printf ("DESCRIBE Error!\n");
    }

    /* Teardown session */
    ret = teardown(session_info.session_id, NULL, session_info.stream_id);
    if (ret != RTSP_OK)
    {
        printf ("TEARDOWN Error!\n");
    }

    RETURN_TEST_RESULT(ret);
}

int main (int argc, char** argv)
{
	int ret = 0, id, tc;
    char *ptr;

    ret = OPTIONS_PARSE(argc, argv);
    if (ret < 0)
    {
        usage(argv[0]);
        RETURN_TEST_RESULT(WRONG_USAGE);
    }

    if (GET_OPTION("help"))
    {
        usage(argv[0]);
        return NO_ERROR;
    }

    server = GET_OPTION_VALUE("ipaddr");
    ptr = GET_OPTION_VALUE("testcase");
    if (ptr != NULL)
    {
        tc = atof(ptr);
    }
    if ((tc <= 0) || ( tc > num_tests))
    {
        printf ("Invalid test case id\n");
        usage(argv[0]);
        RETURN_TEST_RESULT(WRONG_USAGE);
    }

    verbose = GET_OPTION("verbose");

    if (argc < 2)
    {
        printf ("Usage : %s <server_ip> [--verbose]", argv[0]);
        return NO_ERROR;
    }

    if ((argc == 3) && (strcmp(argv[2], "--verbose") == 0))
    {
        verbose = 1;
    }

    /* Initialize rtsp test setup */
    id = rtsp_test_init (server);
    if (id < 0)
    {
        printf ("RTSP Test initialization error!\n");
        return id;
    }

    /* Run RTSP Tests */
    printf("Running test case : %d \n", tc);
    printf("%s\n", sTCInfo[tc-1].name);
    ret = sTCInfo[tc-1].fn();
    /* Report result here */
    if (ret != NO_ERROR)
    {
        printf("Test case (%d) result: FAILED\n", tc);
    }
    else
    {
        printf("Test case (%d) result: PASSED\n", tc);
    }


    rtsp_test_shutdown(id);

    return ret;
}
