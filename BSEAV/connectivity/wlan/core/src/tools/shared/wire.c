/*
 * Copyright 1999-2001, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 *
 * $Id$
 *
 * Keeps a list of Stations on the (udp) link, receives and packets from them 
 * and transmits packet to them.
 *
 * socketstarted is a global defined in pci.c.
 *
 */

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netdb.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typedefs.h>
#include <utils.h>
#include <sys_xx.h>
#include <wire.h>

/* Local definitions */
#define MAXNSTATIONS 100
#define MAXPKTSZ 65535
#define WIMPYPKT 3
#define WIRE_SERVERPORT 16000
#define WIRE_CLIENTPORT 17000

/* Globals and local data structures */
struct sockaddr_in Stations[MAXNSTATIONS];
struct sockaddr_in WireServerSin;
int ns = 0;
int ServerStarted = FALSE;
int WireClientSocket = 0;
extern char host[];

int Wiredrop = 0;
int Wiredropcnt = 1;
#define MAX_DROPINTVL 50
#define MIN_INTVL 4

/* Forward Declarations */
static int WireServer (void *nothing);
static void WireClientInit (void);

/*
 * Create a wire server if needed and initialize the client.
 */
void
WireSetup (void)
{
	void* h;
	

	/* Create a server if needed */
	h = createthread(WireServer, NULL, 0);
	ASSERT (h);

	/* Shouldn't start the client until there is a server */
	while (ServerStarted == FALSE)
		msleep (1);

	/* Now create a client socket */
	WireClientInit ();
}

/*
 * support random and fixed dropping in wire server
 */
static unsigned long pktcnt = 0;
static unsigned long dropintvl = MIN_INTVL;
static unsigned long dropcnt = 1;

void
reinitdrops(void) 
{
        if (Wiredrop > 0) {
                dropintvl = Wiredrop;
                dropcnt = Wiredropcnt;
        } else {
                dropintvl =
                        ((rand() % MAX_DROPINTVL)
                         + MIN_INTVL);
                dropcnt = (rand() % MIN_INTVL);
        }
        if (!dropcnt)
                dropcnt = 1;
        if (dropintvl < dropcnt + MIN_INTVL)
                dropintvl = dropcnt + MIN_INTVL;
        /*printf("new dropintvl %d, dropcnt %d\n",
          dropintvl, dropcnt);*/
}

/*
 * Starts up if no other server is active.  Keeps a list of attached re10
 * nodes, and sends received packets to them.
 */
static int WireServer (void *nothing)
{
	struct sockaddr_in sin;
	struct sockaddr_in from;
	socklen_t from_len;
	int s;
	int rc;
	int n;
	int bytes;
	unsigned char pkt[MAXPKTSZ];
        if (Wiredrop)
                reinitdrops();

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s < 0)
		return 1;

	memset((void *) &sin, 0, sizeof (sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(WIRE_SERVERPORT);

	/* Fails if another wire server is already active */
	rc = bind(s, (struct sockaddr *) &sin, sizeof (struct sockaddr));
	ServerStarted = TRUE;

	if (rc < 0) {
		return 2;
	}

	/* Receive packets forever */
	while (1) {
                
		from_len = sizeof (from);
		bytes = recvfrom(s, pkt, MAXPKTSZ, 0, 
				 (struct sockaddr *) &from, &from_len);
		if (bytes < 0) {
			return 3;
		}

		/* Add Stations to the list */
		for (n = 0; n < ns; n++) {

			if ((Stations[n].sin_addr.s_addr == 
						from.sin_addr.s_addr) &&
			      (Stations[n].sin_port == from.sin_port))
			      break;
		}
		if (n == ns) {
			memcpy ((char *) (Stations + n), (char *) &from,
						sizeof (from));
			ns++;
		}

		/* 
		 * Check for a wimpy packet just to signal the station is
		 * alive
		 */
		if (bytes <= WIMPYPKT)
			continue;

                /*
                 * Perhaps decide to drop the packet, or add an error
                 */
                if (Wiredrop) {
                        pktcnt += 1;
                        if ((pktcnt % dropintvl) >= dropintvl - dropcnt) {
                                /*printf("pktcnt %d, dropintvl %d, dropcnt %d\n",
                                  pktcnt, dropintvl, dropcnt);*/
                                if ((pktcnt % dropintvl) == (dropintvl - 1)) {
                                        reinitdrops();
                                }
                                continue;
                        }
                }
		/* Send the packet to other Stations */
		for (n = 0; n < ns; n++) {
			if ((Stations[n].sin_addr.s_addr == 
						from.sin_addr.s_addr) &&
			    (Stations[n].sin_port == from.sin_port))
				continue;

			rc = sendto(s, pkt, bytes, 0, 
				    (struct sockaddr *) (Stations + n),
				     sizeof (struct sockaddr));
			if (rc < 0) {
				return 4;
			}
		}

	}
}

/*
 * Initialize the client
 */
static void WireClientInit (void)
{
	struct sockaddr_in sin;
	int rc;
	int port;
	struct hostent *hp;

	WireClientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	ASSERT (WireClientSocket >= 0);

	memset ((void *) &sin, 0, sizeof (struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	port = WIRE_CLIENTPORT;


	if (*host == '\0')
		gethostname (host, 256);

	if ((hp = gethostbyname(host)) == NULL) {
		fprintf(stderr, "gethostbyname error: %s\n", host);
		return;
	}

	WireServerSin.sin_addr.s_addr = *(int *)(hp->h_addr);
	WireServerSin.sin_family = AF_INET;
	WireServerSin.sin_port = htons ((short) WIRE_SERVERPORT);

	/* Client addr needs to be distinct */
	do {
		sin.sin_port = htons ((short) port);
		port++;
		rc = bind(WireClientSocket, (struct sockaddr *) &sin, 
						sizeof (struct sockaddr));
	} while (rc < 0 && (port < (WIRE_CLIENTPORT + MAXNSTATIONS)));

	ASSERT (rc >= 0);

}

/*
 * Receive a packet.  The caller has to copy it out.  Returns <= 0 on failure.
 */
char *
WireClientRx (int *len)
{
	static char pkt[MAXPKTSZ];
	struct sockaddr from;
	socklen_t from_len;

	from_len = sizeof (from);
	*len = recvfrom(WireClientSocket, pkt, MAXPKTSZ, 0, &from, &from_len);
	ASSERT (*len >= 0);
	return pkt;
}

/* 
 * Send a packet.
 */
int
WireClientTx (char *pkt, int len)
{

	if (sendto(WireClientSocket, pkt, len, 0, 
		    (struct sockaddr *) &WireServerSin, sizeof
		    (WireServerSin)) < 0)
		return -1;

	return 0;
}
