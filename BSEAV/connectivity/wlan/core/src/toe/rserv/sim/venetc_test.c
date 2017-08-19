/*
 * Virtual Ethernet Client/Switch Testing Utility
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "venet.h"
#include "venetc.h"

char *prog_name;

void
usage(void)
{
	fprintf(stderr,
	        "Usage: %s\n",
	        prog_name);
	fprintf(stderr,
	        "    VENET_HOST and VENET_PORT variables must be set to the host name\n");
	fprintf(stderr,
	        "    and a port number on the virtual ethernet switch.\n");
	exit(1);
}

unsigned char pkt[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* DA */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* SA */
	0x08, 0x00,					/* Type/Length */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

static void
ethdump(unsigned char *msg, int msglen)
{
	int i, col;

	if (msglen < 14) {
		printf("SHORT PACKET:");
		for (i = 0; i < msglen; i++)
			printf(" %02x", msg[i]);
		printf("\n");
		return;
	}

	printf("\t{%02x %02x %02x %02x %02x %02x} {%02x %02x %02x %02x %02x %02x} {%02x %02x}\n",
	       msg[0], msg[1], msg[2], msg[3], msg[4], msg[5],
	       msg[6], msg[7], msg[8], msg[9], msg[10], msg[11],
	       msg[12], msg[13]);

	col = 0;

	for (i = 14; i < msglen; i++, col++) {
		if (col % 16 == 0)
			printf("\t");
		printf("%02x", msg[i]);
		if ((col + 1) % 16 == 0)
			printf("\n");
		else
			printf(" ");
	}

	if (col % 16 != 0)
		printf("\n");
}


static int
parse_mac(unsigned char mac[6], char *buf)
{
	int i;
	char *buf_next;

	for (i = 0; i < 6; i++) {
		mac[i] = (unsigned char)strtol(buf, &buf_next, 16);
		if ((i < 5 && buf_next[0] != ':') ||
		    (i == 5 && buf_next[0] != 0))
			return -1;
		buf = buf_next + 1;
	}

	return 0;
}

int
main(int argc, char **argv)
{
	extern int optind;
	/*
	  extern char *optarg;
	*/
	int c;
	unsigned char *msg;
	int msglen;
	venetc_t *vt;
	char *s;
	char *hostname;
	int port;

	prog_name = argv[0];

	if ((hostname = getenv("VENET_HOST")) == NULL)
		usage();

	if ((s = getenv("VENET_PORT")) == NULL)
		usage();

	port = (int)strtol(s, 0, 0);

	while ((c = getopt(argc, argv, "")) != EOF)
		switch (c) {
		default:
			usage();
		}

	if (optind != argc)
		usage();

	if ((vt = venetc_open(hostname, port)) == NULL) {
		fprintf(stderr,
		        "%s: error opening connection to %s port %d\n",
		        prog_name, hostname, port);
		exit(1);
	}

	printf("MAC = %x:%x:%x:%x:%x:%x\n",
	       vt->mac[0], vt->mac[1], vt->mac[2], vt->mac[3], vt->mac[4], vt->mac[5]);

	for (;;) {
		fd_set rfds;
		char buf[40];

		memcpy(pkt + 6, vt->mac, 6);		/* SA */

		printf("Enter DA: ");
		fflush(stdout);

		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		FD_SET(vt->serv_fd, &rfds);

		select(vt->serv_fd + 1, &rfds, 0, 0, NULL);

		if (FD_ISSET(0, &rfds)) {
			if (fgets(buf, sizeof(buf), stdin) == NULL)
				break;

			parse_mac(pkt, buf);		/* DA */

			printf("send to %x:%x:%x:%x:%x:%x\n",
			       pkt[0], pkt[1], pkt[2], pkt[3], pkt[4], pkt[5]);

			msg = pkt;
			msglen = (int)sizeof(pkt);

			venetc_send(vt, msg, msglen);
		}

		if (FD_ISSET(vt->serv_fd, &rfds)) {
			venetc_recv(vt, &msg, &msglen);
			printf("Received packet length %d:\n", msglen);
			ethdump(msg, msglen);
			venetc_free(vt, msg);
		}
	}

	venetc_close(vt);

	exit(0);

	/*NOTREACHED*/
	return 0;
}
