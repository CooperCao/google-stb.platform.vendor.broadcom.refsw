/*
 * UDP Receiver
 */

#include "udptools.h"

char *pname;

void
usage(void)
{
	fprintf(stderr,
	        "Usage: %s [options]\n", pname);
	fprintf(stderr,
	        "   -p <port>     Specify UDP listen port "
	        "(0-65535, default %d)\n",
	        PORT_DEFAULT);
	fprintf(stderr,
	        "   -q            Do not display packets received\n");
	fprintf(stderr,
	        "   -c <count>    Exit after receiving <count> packets\n");
	fprintf(stderr,
	        "   -w <usec>     Exit if no packets are received "
	        "for <usec> microseconds\n");
	exit(1);
}

#define tvusub(tv0, tv1)	(((tv1)->tv_sec - (tv0)->tv_sec) * 1000000 + \
				 ((tv1)->tv_usec - (tv0)->tv_usec))

int
main(int argc, char **argv)
{
	int			s;
	struct sockaddr_in	cli_addr, serv_addr;
	socklen_t		cli_addr_len;
	int			opt_port, opt_count, opt_quiet, opt_wait;
	int			i, n;
	unsigned char		recvbuf[PKTSIZE_MAX];
	struct in_addr		ina;
	unsigned int		seq_no, usec;
	struct timeval		tv0, tv;

	pname = argv[0];

	opt_port = PORT_DEFAULT;
	opt_count = 0;
	opt_wait = -1;
	opt_quiet = 0;

	while ((i = getopt(argc, argv, "p:c:w:q")) >= 0) {
		switch (i) {
		case 'p':
			opt_port = (int)strtol(optarg, 0, 0);
			break;
		case 'c':
			opt_count = (int)strtol(optarg, 0, 0);
			break;
		case 'w':
			opt_wait = (int)strtol(optarg, 0, 0);
			break;
		case 'q':
			opt_quiet = 1;
			break;
		default:
			usage();
			break;
		}
	}

	if (optind != argc)
		usage();

	/*
	 * Check args
	 */

	if (opt_port < 0 || opt_port > 65535) {
		fprintf(stderr, "%s: invalid port %d\n", pname, opt_port);
		exit(1);
	}

	/*
	 * Configure UDP socket
	 */

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr,
		        "%s: can't open datagram socket: %s\n",
		        pname, strerror(errno));
		exit(1);
	}

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(opt_port);

	if (bind(s, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr,
		        "%s: can't bind local address: %s\n",
		        pname, strerror(errno));
		close(s);
		exit(1);
	}

	/*
	 * Loop receiving packets
	 */

	gettimeofday(&tv0, NULL);

	for (i = 0; opt_count == 0 || i < opt_count; i++) {
		cli_addr_len = sizeof (cli_addr);

		if (opt_wait >= 0) {
			fd_set			rfds;
			struct timeval		tv;
			int			n;

			FD_ZERO(&rfds);
			FD_SET(s, &rfds);

			tv.tv_sec = opt_wait / 1000000;
			tv.tv_usec = opt_wait % 1000000;
			n = select(s + 1, &rfds, NULL, NULL, &tv);

			if (n < 0) {
				fprintf(stderr,
				        "%s: select failed: %s\n",
				        pname, strerror(errno));
				exit(0);
			}

			if (n == 0)
				exit(0);
		}

		n = recvfrom(s, recvbuf, sizeof (recvbuf), 0,
		             (struct sockaddr *)&cli_addr, &cli_addr_len);

		if (n < 0) {
			fprintf(stderr,
			        "%s: recvfrom failed: %s\n",
			        pname, strerror(errno));
			break;
		}

		gettimeofday(&tv, NULL);

		seq_no = recvbuf[0] << 0;
		if (n > 1)
		    seq_no |= recvbuf[1] << 8;
		if (n > 2)
		    seq_no |= recvbuf[2] << 16;
		if (n > 3)
		    seq_no |= recvbuf[3] << 24;

		if (!opt_quiet) {
			ina.s_addr = cli_addr.sin_addr.s_addr;

			usec = (unsigned int)tvusub(&tv0, &tv);

			printf("%-6d %4u.%06u From %-16s Len %-5d Seq %u\n",
			       i + 1,
			       usec / 1000000, usec % 1000000,
			       inet_ntoa(ina), n, seq_no);

			fflush(stdout);
		}

		memcpy(&tv0, &tv, sizeof(tv0));
	}

	close(s);
	exit(0);
}
