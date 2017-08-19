/* ush - remote shell with status codes
 *
 * $ Copyright Broadcom Corporation $
 *
 * $Id: feea75dd217a07e3524c35fffb48eb3aaffdd43e $
 *
 * ush/ushd is designed to provide simple rsh-like remote command
 * execution (with error codes) for small embedded systems where ssh
 * may be too slow.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/wait.h>
#include <netdb.h>

#undef max
#define max(x, y) ((x) > (y) ? (x) : (y))

#define DEBUG


#define WARN_DATA(PRE, DATA, LEN, POST) \
	write(2, (PRE), strlen((PRE))); \
	write(2, (DATA), (LEN)); \
	write(2, (POST), strlen((POST))); \

#ifdef DEBUG
static int verbose = 0;
#define DEBUG_OUT(...) (void)(verbose && fprintf(stderr, __VA_ARGS__))
#define DEBUG_DATA(PRE, DATA, LEN, POST) \
	if (verbose) { \
		WARN_DATA((PRE), (DATA), (LEN), (POST)) \
	}
#else
#define DEBUG_OUT(...)
#define DEBUG_DATA(PRE, DATA, LEN, POST)
#endif

/* ush has no sessions, but they make the protocol easier to parse. */
static int session = 0;

static int
connect_socket(int port, const char *address)
{
	struct sockaddr_in a;
	struct hostent *hp;
	int s;

	/* Host name lookup */
	hp = gethostbyname(address);
	if (!hp || !hp->h_name || !hp->h_name[0] || !hp->h_addr_list[0]) {
		herror(address);
		return -1;
	}

	memset(&a, 0, sizeof(a));
	a.sin_port = htons(port);
	a.sin_family = AF_INET;
	a.sin_addr.s_addr = *((unsigned long*)hp->h_addr_list[0]);

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}
	if (connect(s, (struct sockaddr *) &a, sizeof (a)) < 0) {
		perror("connect()");
		shutdown(s, SHUT_RDWR);
		close(s);
		return -1;
	}
	return s;
}

#define SHUT_FD1 {			\
	       if (fd1 >= 0) {		       \
		   DEBUG_OUT("shutdown\n"); \
		   status = 255; \
		   close_stdout = 1; \
		   close_stderr = 1; \
		   eos = 1; \
		   shutdown(fd1, SHUT_RDWR);  \
		   close(fd1);		       \
		   fd1 = -1;		       \
	       }			       \
	   }

#define BUF_SIZE 1024

void
usage(char *cmd)
{
	fprintf(stderr, "usage:\t%s [-p port] hostname cmd ...\n", cmd);
	exit(1);
}

char *
sigtoname(int sig)
{
	switch (sig) {
	case SIGHUP: return "SIGHUP";
	case SIGINT: return "SIGINT";
	case SIGQUIT: return "SIGQUIT";
	case SIGILL: return "SIGILL";
	case SIGTRAP: return "SIGTRAP";
	case SIGABRT: return "SIGABRT";
	case SIGBUS: return "SIGBUS";
	case SIGFPE: return "SIGFPE";
	case SIGKILL: return "SIGKILL";
	case SIGUSR1: return "SIGUSR1";
	case SIGSEGV: return "SIGSEGV";
	case SIGUSR2: return "SIGUSR2";
	case SIGPIPE: return "SIGPIPE";
	case SIGALRM: return "SIGALRM";
	case SIGTERM: return "SIGTERM";
	case SIGCHLD: return "SIGCHLD";
	case SIGCONT: return "SIGCONT";
	case SIGSTOP: return "SIGSTOP";
	case SIGTSTP: return "SIGTSTP";
	case SIGTTIN: return "SIGTTIN";
	case SIGTTOU: return "SIGTTOU";
	case SIGURG: return "SIGURG";
	case SIGXCPU: return "SIGXCPU";
	case SIGXFSZ: return "SIGXFSZ";
	case SIGVTALRM: return "SIGVTALRM";
	case SIGPROF: return "SIGPROF";
	case SIGWINCH: return "SIGWINCH";
	case SIGIO: return "SIGIO";
	case SIGSYS: return "SIGSYS";

		/* Optional */
#ifdef SIGEMT
	case SIGEMT: return "SIGEMT";
#endif
#ifdef SIGPWR
	case SIGPWR: return "SIGPWR";
#endif
#ifdef SIGINFO
	case SIGINFO: return "SIGINFO";
#endif
#ifdef SIGLOST
	case SIGLOST: return "SIGLOST";
#endif

	default: return NULL;
	}
}


/* Catch all the signals we can.  We could catch SIGTSTP, and SIGSEGV
 * but it's better to leave these to the local end.
 */

void
allsignals(void (*catcher)(int))
{
	(void) signal(SIGHUP, catcher);
	(void) signal(SIGINT, catcher);
	(void) signal(SIGQUIT, catcher);
	(void) signal(SIGILL, catcher);
	(void) signal(SIGTRAP, catcher);
	(void) signal(SIGABRT, catcher);
	(void) signal(SIGBUS, catcher);
	(void) signal(SIGFPE, catcher);
	/* SIGKILL */
	(void) signal(SIGUSR1, catcher);
	/* SIGSEGV */
	(void) signal(SIGUSR2, catcher);
	(void) signal(SIGPIPE, catcher);
	(void) signal(SIGALRM, catcher);
	(void) signal(SIGTERM, catcher);
	(void) signal(SIGCHLD, catcher);
	(void) signal(SIGCONT, catcher);
	/* SIGSTOP */
	/* SIGTSTP */
	(void) signal(SIGTTIN, catcher);
	(void) signal(SIGTTOU, catcher);
	(void) signal(SIGURG, catcher);
	(void) signal(SIGXCPU, catcher);
	(void) signal(SIGXFSZ, catcher);
	(void) signal(SIGVTALRM, catcher);
	(void) signal(SIGPROF, catcher);
	(void) signal(SIGWINCH, catcher);
	(void) signal(SIGIO, catcher);
#ifdef SIGPWR
	(void) signal(SIGPWR, catcher);
#endif
#ifdef SIGINFO
	(void) signal(SIGINFO, catcher);
#endif
	(void) signal(SIGSYS, catcher);
}

/* Sendbuffer needs to be global so that it can be updated by the
 * signal handler
 */
static char sendbuf[BUF_SIZE*2];
static int sendbuf_avail = 0, sendbuf_written = 0;

/* Signal catcher */
void
catcher(int sig)
{
	DEBUG_OUT("caught signal %d\n", sig);
	char *name = sigtoname(sig);
	if (name) {
		sendbuf_avail +=
			sprintf(sendbuf + sendbuf_avail,
				"signal %d %s\n", session, name);
	} else {
		sendbuf_avail +=
			sprintf(sendbuf + sendbuf_avail,
				"signal %d %d\n", session, sig);
	}
}

int main(int argc, char **argv)
{
	int port = 4700;
	int fd1 = -1, cstdin = -1, cstdout = -1, cstderr = -1;
	char outbuf[BUF_SIZE], errbuf[BUF_SIZE];
	char recvbuf[BUF_SIZE*2]; /* Room for stdin + commands */
	int outbuf_avail = 0, outbuf_written = 0;
	int errbuf_avail = 0, errbuf_written = 0;
	int recvbuf_avail = 0, recvbuf_written = 0;

	char tmp[BUF_SIZE];

	int close_stdout = 0;
	int close_stderr = 0;
	int status = 0;
	int signal_status = 0;
	int eos = 0;
	int cmdlen = 0;
	char *cmdline;
	char ch;

	char *address;

	if (argc < 3) {
		usage(argv[0]);
	}

	while ((ch = getopt(argc, argv, "+p:l:v")) != EOF)
		switch (ch) {
		case 'l': /* ignored */
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'v':
#ifdef DEBUG
			verbose = 1;
			break;
#endif
		case '?':
		default:
			usage(argv[0]);
		}

	argc -= optind;
	argv += optind;

	address = argv[0];

	{
		int j;
		for (j = 1; j < argc; j++) {
			cmdlen += strlen(argv[j]) + 1; /* +1 for spaces */
		}
		/* Allocate the space */
		cmdline = (char*)malloc(cmdlen + 1);

		/* Append all the bits, no fear of overflows */
		for (j = 1; j < argc; j++) {
			strcat(cmdline, argv[j]);
			strcat(cmdline, " ");
		}
	}

	allsignals(catcher);

	fd1 = connect_socket(port, address);

	if (fd1 < 0)
		exit(1);

	for (;;) {
		int r, nfds = 0;
		int cnt = 0;
		fd_set rd, wr;
		FD_ZERO(&rd);
		FD_ZERO(&wr);
		DEBUG_OUT("[");

		/* Readers */
		if (fd1 >= 0 && recvbuf_avail < sizeof(recvbuf)) {
			DEBUG_OUT(" %d(net) ", fd1);
			FD_SET(fd1, &rd);
			nfds = max(nfds, fd1);
		}
		if (cstdin >= 0 && sendbuf_avail < BUF_SIZE) {
			DEBUG_OUT(" %d(in) ", cstdin);
			FD_SET(cstdin, &rd);
			nfds = max(nfds, cstdin);
		}

		DEBUG_OUT("][");
		/* Writers */
		if (fd1 >= 0 && sendbuf_avail - sendbuf_written > 0) {
			DEBUG_OUT(" %d(net) ", fd1);
			FD_SET(fd1, &wr);
			nfds = max(nfds, fd1);
		}
		if (cstdout >= 0) {
			if (outbuf_avail - outbuf_written > 0) {
				DEBUG_OUT(" %d(out) ", cstdout);
				FD_SET(cstdout, &wr);
				nfds = max(nfds, cstdout);
			} else if (close_stdout) { /* close after flushing */
				DEBUG_OUT("stdout flushed - closing\n");
				close(cstdout);
				cstdout = -1;
			}
		}

		if (cstderr >= 0) {
			if (errbuf_avail - errbuf_written > 0) {
				DEBUG_OUT(" %d(err) ", cstderr);
				FD_SET(cstderr, &wr);
				nfds = max(nfds, cstderr);
			} else if (close_stderr) { /* close after flushing */
				DEBUG_OUT("stderr flushed\n");
				close(cstderr);
				cstderr = -1;
			}
		}

		DEBUG_OUT("]\n");

		if (eos && cstdout < 0 && cstderr < 0) {
			if (signal_status) {
				/* Restore signals */
				allsignals(SIG_DFL);
				kill(getpid(), signal_status);
			} else {
				_exit(status);
			}
		}

		r = select(nfds + 1, &rd, &wr, NULL, NULL);

		if (r == -1 && errno == EINTR)
			continue;
		if (r < 0) {
			perror("select()");
			_exit(255);
		}

		if (fd1 >= 0 && FD_ISSET(fd1, &rd)) {
			r = read(fd1, recvbuf + recvbuf_avail,
				sizeof(recvbuf) - recvbuf_avail);
			if (r < 1) {
				SHUT_FD1;
			} else {
				DEBUG_DATA("receiving <<",
					recvbuf + recvbuf_avail, r, ">>\n");
				recvbuf_avail += r;
			}
		}
		if (cstdin >= 0 && FD_ISSET(cstdin, &rd)) {
			r = read(cstdin, tmp, BUF_SIZE);
			if (r < 1) {
				close(cstdin);
				cstdin = -1;
				sendbuf_avail +=
					sprintf(sendbuf + sendbuf_avail,
						"eof-stdin %d\n", session);
			} else {
				sendbuf_avail +=
					sprintf(sendbuf + sendbuf_avail,
						"stdin %d %dH",	session, r);
				memcpy(sendbuf + sendbuf_avail, tmp, r);
				sendbuf[sendbuf_avail + r] = '\n';
				sendbuf_avail += r + 1;
			}
		}
		if (fd1 >= 0 && FD_ISSET(fd1, &wr)) {
			r = write(fd1,
				sendbuf + sendbuf_written,
				sendbuf_avail - sendbuf_written);
			DEBUG_DATA("sending <<", sendbuf + sendbuf_written,
				sendbuf_avail - sendbuf_written, ">>\n");
			if (r < 1) {
				SHUT_FD1;
			} else
				sendbuf_written += r;
		}
		if (cstdout >= 0 && FD_ISSET(cstdout, &wr)) {
			r = write(cstdout,
				outbuf + outbuf_written,
				outbuf_avail - outbuf_written);
			if (r < 1) {
				close(cstdout);
				cstdout = -1;
			} else
				outbuf_written += r;
		}
		if (cstderr >= 0 && FD_ISSET(cstderr, &wr)) {
			r = write(cstderr,
				errbuf + errbuf_written,
				errbuf_avail - errbuf_written);
			if (r < 1) {
				close(cstderr);
				cstderr = -1;
			} else
				errbuf_written += r;
		}
		/* check if write data has caught read data */
		if (recvbuf_written == recvbuf_avail)
			recvbuf_written = recvbuf_avail = 0;
		if (sendbuf_written == sendbuf_avail)
			sendbuf_written = sendbuf_avail = 0;
		if (outbuf_written == outbuf_avail)
			outbuf_written = outbuf_avail = 0;
		if (errbuf_written == errbuf_avail)
			errbuf_written = errbuf_avail = 0;

		/* Protocol */
		while (recvbuf_avail > 0) {
			int version = 0;
			int len = 0;
			DEBUG_DATA("parsing [[", recvbuf,
				recvbuf_avail, "]]\n");

			if (sscanf(recvbuf, " ush %d\n%n", &version, &cnt)) {
				cstdin = 0;
				cstdout = 1;
				/* Leave the real stderr alone */
				cstderr = dup(2);
				close_stdout = 0;
				close_stderr = 0;

				sendbuf_avail +=
					sprintf(sendbuf + sendbuf_avail,
					"new %d %dH%s\n",
					session, cmdlen, cmdline);

			} else if (sscanf(recvbuf, " stdout %d %dH%n",
				&session, &len, &cnt)) {
				if (cstdout < 0) {
					DEBUG_OUT("stdout closed");
					SHUT_FD1;
					break;
				}
				if (cnt + len > recvbuf_avail) {
					DEBUG_OUT("incomplete\n");
					break;
				} else if (outbuf_avail + len >
					   sizeof(outbuf)) {
					DEBUG_OUT("out would block %d\n",
						outbuf_avail);
					break;
				} else {
					memcpy(outbuf + outbuf_avail,
					       recvbuf + cnt, len);
					outbuf_avail += len;
				}
				/* add final newline */
				cnt++;
			} else if (sscanf(recvbuf, " stderr %d %dH%n",
				&session, &len, &cnt)) {
			  if (cstdout < 0) {
			    DEBUG_OUT("stderr closed");
			    SHUT_FD1;
			    break;
			  }
				if (cnt + len > recvbuf_avail) {
					DEBUG_OUT("incomplete\n");
					break;
				} else if (errbuf_avail + len >
					   sizeof(errbuf)) {
					DEBUG_OUT("err would block\n");
					break;
				} else {
					memcpy(errbuf + errbuf_avail,
					       recvbuf + cnt, len);
					errbuf_avail += len;
				}
				/* add final newline */
				cnt++;
			} else if (sscanf(recvbuf, " eof-stdout %d\n%n",
				&session, &cnt)) {
				DEBUG_OUT("Close stdout\n");
				close_stdout = 1;
			} else if (sscanf(recvbuf, " eof-stderr %d\n%n",
				&session, &cnt)) {
				DEBUG_OUT("Close stderr\n");
				close_stderr = 1;
			} else if (sscanf(recvbuf, " eos %d\n%n",
				&session, &cnt)) {
				DEBUG_OUT("End of Session\n");
				eos = 1;
			} else if (sscanf(recvbuf, " exit %d %*dH%d\n%n",
				&session, &status, &cnt)) {
				DEBUG_OUT("exit %d\n", status);
			} else if (sscanf(recvbuf,
					  " signal-exit %d %*dH%d\n%n",
				&session, &signal_status, &cnt)) {
				DEBUG_OUT("signal-exit %d\n", signal_status);
			} else if (*recvbuf == '\n') {
				/* spare newline */
				cnt = 1;
			} else {
				WARN_DATA("Unknown command: \"",
					recvbuf, recvbuf_avail, "\"\n");
				/* Clear buffer */
				cnt = -1;
				len = recvbuf_avail;
				break;
			}

			if (len == 0 && cnt > recvbuf_avail) {
				/* one of the scanf's got greedy */
				cnt = recvbuf_avail;
			}
			/* Shift to next command */
			recvbuf_avail -= cnt + len;
			memmove(recvbuf, recvbuf + cnt + len, recvbuf_avail);
		}
	}
	return 0;
}
