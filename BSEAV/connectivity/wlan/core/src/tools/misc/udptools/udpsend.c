/*
 * UDP Sender
 */

#include "udptools.h"

char *pname;

void
usage(void)
{
	fprintf(stderr,
	        "Usage: %s [options] <ip-addr>\n", pname);
	fprintf(stderr,
	        "   -p <port>     Specify UDP destination port "
	        "(0-65535, default %d)\n",
	        PORT_DEFAULT);
	fprintf(stderr,
	        "   -t <tos>      IP TOS value; default 0x%x\n", TOS_DEFAULT);
	fprintf(stderr,
	        "   -c <count>    Specify number of packets to send; "
	        "0 to loop until interrupted\n");
	fprintf(stderr,
	        "   -s <size>     Specify size of UDP payload "
	        "(1-%d, default %d)\n",
	        PKTSIZE_MAX, PKTSIZE_DEFAULT);
	fprintf(stderr,
	        "   -w <usec>     Specify wait time after each packet\n");
	fprintf(stderr,
	        "   -n <seq_no>   Specify starting sequence number, "
	        "default 1\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	int			s;
	struct sockaddr_in	cli_addr, serv_addr;
	int			opt_port, opt_tos, opt_count, opt_size;
	unsigned int		opt_seq_no;
	int			opt_wait;
	char			*opt_ipaddr;
	int			i, n;
	unsigned int		pkt_no;
	unsigned char		sendbuf[PKTSIZE_MAX];

	pname = argv[0];

	opt_port = PORT_DEFAULT;
	opt_tos = TOS_DEFAULT;
	opt_count = COUNT_DEFAULT;
	opt_size = PKTSIZE_DEFAULT;
	opt_wait = 0;
	opt_seq_no = SEQ_NO_DEFAULT;

	while ((i = getopt(argc, argv, "p:t:c:s:w:n:")) >= 0) {
		switch (i) {
		case 'p':
			opt_port = (int)strtol(optarg, 0, 0);
			break;
		case 't':
			opt_tos = (int)strtol(optarg, 0, 0);
			break;
		case 'c':
			opt_count = (int)strtol(optarg, 0, 0);
			break;
		case 's':
			opt_size = (int)strtol(optarg, 0, 0);
			break;
		case 'w':
			opt_wait = (int)strtol(optarg, 0, 0);
			break;
		case 'n':
			opt_seq_no = (unsigned int)strtoul(optarg, 0, 0);
			break;
		default:
			usage();
			break;
		}
	}

	if (optind + 1 != argc)
		usage();

	opt_ipaddr = argv[optind];

	/*
	 * Check args
	 */

	if (opt_port < 0 || opt_port > 65535) {
		fprintf(stderr, "%s: invalid port %d\n", pname, opt_port);
		exit(1);
	}

	if (opt_tos < 0 || opt_tos > 0xff) {
		fprintf(stderr, "%s: invalid TOS %d\n", pname, opt_tos);
		exit(1);
	}

	if (opt_count < 0) {
		fprintf(stderr, "%s: invalid count %d\n", pname, opt_count);
		exit(1);
	}

	if (opt_size < 0 || opt_size > PKTSIZE_MAX) {
		fprintf(stderr, "%s: invalid size %d\n", pname, opt_size);
		exit(1);
	}

	/*
	 * Configure UDP socket
	 */

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(opt_ipaddr);
	serv_addr.sin_port = htons(opt_port);

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr,
		        "%s: can't open datagram socket: %s\n",
		        pname, strerror(errno));
		exit(1);
	}

	/* The protocol number (param #2) comes from /etc/protocols.  On macos, there
	 * isn't a header file defining the protocol number for IP, so just use the value
	 * (0) directly. 
	 */
	if (setsockopt(s, 0, IP_TOS, &opt_tos, sizeof (opt_tos)) < 0) {
		fprintf(stderr,
		        "%s: can't set IP_TOS to 0x%x: %s\n",
		        pname, opt_tos, strerror(errno));
		close(s);
		exit(1);
	}

	bzero((char *)&cli_addr, sizeof(cli_addr));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	cli_addr.sin_port = htons(0);

	if (bind(s, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
		fprintf(stderr,
		        "%s: can't bind local address: %s\n",
		        pname, strerror(errno));
		close(s);
		exit(1);
	}

	/*
	 * Make some data to send.
	 *
	 * The first four bytes will be overwritten with a little-endian
	 * sequence number.
	 */

	for (i = 4; i < opt_size; i++)
		sendbuf[i] = (unsigned char)((i - 4) & 255);

	/*
	 * Send the packets
	 */

	for (pkt_no = 1; opt_count == 0 || pkt_no <= opt_count; pkt_no++) {
		if (opt_size > 0)
			sendbuf[0] = (opt_seq_no >> 0) & 0xff;
		if (opt_size > 1)
			sendbuf[1] = (opt_seq_no >> 8) & 0xff;
		if (opt_size > 2)
			sendbuf[2] = (opt_seq_no >> 16) & 0xff;
		if (opt_size > 3)
			sendbuf[3] = (opt_seq_no >> 24) & 0xff;

		opt_seq_no++;

		n = sendto(s, sendbuf, opt_size, 0,
		           (struct sockaddr *)&serv_addr, sizeof (serv_addr));
		if (n < 0) {
			fprintf(stderr,
			        "%s: sendto "
			        "failed for packet %u seq_no %u: %s\n",
			        pname, pkt_no, opt_seq_no, strerror(errno));
			break;
		}

		if (n != opt_size)
			fprintf(stderr,
			        "%s: sendto "
			        "truncated packet %u to %d bytes\n",
			        pname, pkt_no, n);

		if (opt_wait)
			usleep(opt_wait);
	}

	close(s);
	exit(0);
}
