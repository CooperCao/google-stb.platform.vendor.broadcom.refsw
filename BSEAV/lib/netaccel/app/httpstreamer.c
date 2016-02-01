/***************************************************************************
 *     Copyright (c) 2007-2008, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: app to start HTTP streamer
 *  
 * Revision History:
 *    
 * $brcm_Log: $
 * 
 *************************************************************/ 
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
extern void launch_http_servers(int streamers, int recorders, int pvrs, int cpu_affinity_mask, int server_port, int record_port, int accel_socket);

void usage()
{
	printf("Usage: httpserver [-s N1 -r N2 -c N3 -p N4 -a N5 -n N6 ]\n"
		"\tN1 = number of streamers (default 3)\n"
		"\tN2 = number of recorders (default 0)\n"
		"\tN3 = cpu affinity mask 1: CPU0, 2: CPU1, 3: CPU0 || CPU1 (default is 1)\n"
		"\tN4 = streaming server port (default 5000)\n"
		"\tN5 = Use Accelerated sockets (default is Yes ) \n"
		"\tN6 = recording server port (default 6000) \n"
	);
}

int main(int argc, char **argv)
{
	int c;
	int streamers = 6;			/* # of streaming threads */
	int recorders = 0;			/* # of recording threads */
	int pvrs = 0;				/* # of PVR Control threads: obsolete */
	int cpu_affinity_mask = 1;  /* 1: CPU0, 2: CPU1, 3: CPU0 || CPU1 */
	int server_port = 5000;		/* Streaming Server Port */
	int record_port = 6000;		/* Recording Server Port */
	int accel_socket = 1;		/* Use Accelerated Socket by default */

    while ((c = getopt(argc, argv, "s:r:c:a:p:h")) != -1)
    {
        switch (c) 
        {
        case 's':
            streamers = atoi(optarg);
			if (streamers <= 0 || streamers > 6) {
				printf("Invalid # of streamers %d, ", streamers);
				streamers = 3;
				printf("resetting it to %d\n", streamers);
			}
			break;
        case 'r':
            recorders = atoi(optarg);
			if (recorders < 0 || recorders > 3) {
				printf("Invalid # of recorders %d, ", recorders);
				recorders = 3;
				printf("resetting it to %d\n", recorders);
			}
			break;
        case 'c':
            cpu_affinity_mask = atoi(optarg);
			if (cpu_affinity_mask <=0 || cpu_affinity_mask > 3) {
				printf("Invalid cpu affinity mask %d, ", cpu_affinity_mask);
				cpu_affinity_mask = 1; /* run on CPU 0 */
				printf("resetting it to %d\n", cpu_affinity_mask);
			}
			break;
        case 'p':
            server_port = atoi(optarg);
			if (server_port <=0 || server_port > 65535) {
				printf("Invalid server port %d, ", server_port);
				server_port = 5000;
				printf("resetting it to %d\n", server_port);
			}
			break;
        case 'a':
            accel_socket = atoi(optarg);
			if (accel_socket % 1)
				accel_socket = 1;
			else
				accel_socket = 0;
			break;
		case 'h':
		default:
			usage();
			exit(1);
		}
	}

	if (argc < 7) {
		usage();
	}

	printf("Starting HTTP Server : Streamers=%d Recorders=%d, CPU Affinity Mask %d, Server Port %d, Use Accelerated Socket %s\n", 
			streamers, recorders, cpu_affinity_mask, server_port, (accel_socket == 1 ? "YES" : "NO"));	
	launch_http_servers(streamers, recorders, pvrs, cpu_affinity_mask, server_port, record_port, accel_socket);
}
