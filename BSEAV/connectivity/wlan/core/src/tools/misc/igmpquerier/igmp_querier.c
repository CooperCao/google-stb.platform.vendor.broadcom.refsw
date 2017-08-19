/*
 * IGMP querier that can be controlled remotely (e.g. by UTF scripts)
 *
 * Author Robert J. McMahon (rmcmahon)
 * Last modified: 06/25/2010
 * 
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 */ 
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#define IGMP_HEADER_SIZE 8

void sigint();
void siguser();
void sigalrm();
static int querier_sent_count = 0;
static int sendcount = 0;
static short daemonmode = 0;
static char mcast_queryaddr[20] = "224.0.0.1";

int main (int argc, char **argv) {
    char *tvalue = NULL;
    char *cvalue = NULL;
    int c;
    int queryinterval;
    int pid;

    opterr = 0;
    while ((c = getopt (argc, argv, "c:dg:t:")) != -1) {
	switch (c) {
	case 'c':
	    cvalue = optarg;
	    break;
	case 'g' :
	    strcpy(mcast_queryaddr, optarg);
	    break;
	case 't':
	    tvalue = optarg;
	    break;
	case 'd':
	    daemonmode = 1;
	    break;
	case '?':
	    if (optopt == 't')
		fprintf (stderr, "Option -%c requires an integer argument.\n", optopt);
	    else if (optopt == 'c')
		fprintf (stderr, "Option -%c requires an integer argument.\n", optopt);
	    else if (isprint (optopt))
		fprintf (stderr, "Unknown option `-%c'.\n", optopt);
	    else
		fprintf (stderr,
			 "Unknown option character `\\x%x'.\n",
			 optopt);
	    return 1;
	default:
	    abort ();
	}
    }
    if (tvalue != NULL) {
	queryinterval = atoi(tvalue);
    } else {
	queryinterval = 0;
    }
    if (cvalue != NULL) {
	sendcount = atoi(cvalue);
	if (tvalue == NULL)
	    queryinterval = 0; 
    } else {
	sendcount = 1;
    }
    signal(SIGINT, sigint);
    signal(SIGUSR1, siguser);
    signal(SIGALRM, sigalrm);
    pid = (int) getpid();
    if (daemonmode) {
	if (tvalue != NULL) {
	    sendcount = 1;
	    printf("IGMP All Hosts Querier (pid=%d) started as a daemon with interval of %d seconds to %s\n", pid, queryinterval, mcast_queryaddr);
	} else {
	    sendcount = 0;
	    printf("IGMP All Hosts Querier (pid=%d) to %s started as a daemon only\n", pid, mcast_queryaddr);
	}
    } else {
	if (queryinterval) { 
	    printf("IGMP All Hosts Querier (pid=%d) sending %d reports with interval of %d seconds\n", pid, sendcount, queryinterval);
	}
    }
    fflush(stdout);
    while (daemonmode || sendcount) {
	if (sendcount) {
	    if (send_igmp_allhosts_querier(mcast_queryaddr)) 
		sendcount--;
	}
	if (queryinterval) {
	    alarm(queryinterval);
	    if (daemonmode || sendcount) 
		pause();
	}
    }
}

void sigint (void) {
    exit(0);
}

void siguser (void) {
    if (!send_igmp_allhosts_querier(mcast_queryaddr)) {
	exit (-1);
    }
}

void sigalrm (void) {
    if (daemonmode) 
	sendcount++;
}

int send_igmp_allhosts_querier (char *mcast_queryaddr) {
    int sid=0;
    int rc;
    char buf[IGMP_HEADER_SIZE];
    unsigned int ttl=1;
    char type = 17;
    char maxresptime = 1;
    //	unsigned int groupaddr = 0; 
    char checksum_upper = 0xee;
    char checksum_lower = 0xfe;
    struct sockaddr_in msock;
    const time_t timer = time(NULL);
	    
    bzero (&buf, sizeof(buf));
    /* 
     * Note: with byte writes shouldn't need to worry about network/host
     * byte ordering, though should double check on non intel system
     */
    buf[0] = type;
    buf[1] = maxresptime;
    buf[2] = checksum_upper;
    buf[3] = checksum_lower;
	   
    bzero (&msock, sizeof(msock));
    msock.sin_family = AF_INET;

    inet_pton(AF_INET, mcast_queryaddr, &msock.sin_addr);
    setuid(geteuid());
    sid = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP);
    if (sid != -1) {
	rc = setsockopt(sid, IPPROTO_IP, IP_TTL, (char *) &ttl, sizeof(ttl));
	if (rc != -1) {
	    rc = sendto(sid, &buf, IGMP_HEADER_SIZE, 0, (const struct sockaddr *) &msock, sizeof(msock)); 
	    if (rc == -1) {
		fprintf(stderr, "IGMP Query sendto error = %s\n", strerror(errno));
	    } 
	} else {
	    fprintf(stderr, "IGMP Query setsockopt error = %s\n", strerror(errno));
	} 
    } else {
	fprintf(stderr, "IGMP Query, socket error = %s\n", strerror(errno));
    }
    if (sid > 0) {
	close(sid);
	rc=1;
    } else {
	rc = 0;
    }
    setuid(getuid());
    if (rc) {
	printf("Sent IGMP all hosts querier to %s (count = %d) at %s", mcast_queryaddr, ++querier_sent_count, ctime(&timer));
	fflush(stdout);
    }	
    return rc;
}
