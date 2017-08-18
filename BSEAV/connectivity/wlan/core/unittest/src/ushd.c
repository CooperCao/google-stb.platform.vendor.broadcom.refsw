/* ushd - remote shell with status codes
 *
 * $ Copyright Broadcom Corporation $
 *
 * $Id: ab0021bc65bdae6b6fdd7f4add6bfa7d3f61765e $
 *
 * ush/ushd is designed to provide simple rsh-like remote command
 * execution (with error codes) for small embedded systems where ssh
 * may be too slow.
 *
 * ushd uses no authentication or encryption so is extremely insecure.
 * DO NOT RUN THIS ON PUBLIC SYSTEMS.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>
#include <net/if.h>

#if defined (__MIPSEL__) || defined (__ARM_EABI__)
/* Disable use of syslog, since it causes an fd leak on the router */
# define openlog(...)
# define syslog(...)
# define closelog(...)
# else
# include <syslog.h>
#endif

#undef max
#define max(x, y) ((x) > (y) ? (x) : (y))

#define DEBUG
#define VERSION "1"

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
listen_socket(int port, char *interface_name)
{
	struct sockaddr_in a;
	int s;
	int yes = 1;
	struct ifreq interface;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
		(char *) &yes, sizeof(yes)) < 0) {
		perror("setsockopt");
		close(s);
		return -1;
	}

	/* Set it to listen on the specified interface */
	if (interface_name) {
#ifdef SO_BINDTODEVICE
		strncpy(interface.ifr_ifrn.ifrn_name,
			interface_name, IFNAMSIZ);
		if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE,
			&interface, sizeof(interface)) < 0) {
#else
			{
				errno = ENOSYS;
#endif
				perror("setsockopt(SO_BINDTODEVICE)");
				close(s);
				return -1;
			}
		}

		memset(&a, 0, sizeof(a));
		a.sin_port = htons(port);
		a.sin_family = AF_INET;
		if (bind(s, (struct sockaddr *) &a, sizeof(a)) < 0) {
			perror("bind");
			close(s);
			return -1;
		}
		if (listen(s, 10) < 0) {
			perror("listen");
			return -1;
		}
		return s;
	}

static int
connect_cmd(char *cmdbuf, int *in, int* out, int* err)
{
	int p[3][2];
	int pid;
	if (pipe(p[0]) < 0 || pipe(p[1]) < 0 || pipe(p[2]) < 0) {
		perror("Can't make pipes.\n");
		exit(1);
	}
	signal(SIGCHLD, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
	pid = fork();
	if (pid == -1)  {
		perror("fork");
		exit(1);
	} else if (pid) { /* parent */
		(void) close(p[0][0]);
		(void) close(p[1][1]);
		(void) close(p[2][1]);
		*in = p[0][1];
		*out = p[1][0];
		*err = p[2][0];
		return pid;
	} else { /* child */
		(void) close(p[0][1]);
		(void) close(p[1][0]);
		(void) close(p[2][0]);
		dup2(p[0][0], 0);
		dup2(p[1][1], 1);
		dup2(p[2][1], 2);
		/* Close unused dups */
		(void) close(p[0][0]);
		(void) close(p[1][1]);
		(void) close(p[2][1]);
		setsid();
		execl("/bin/sh", "sh", "-c", cmdbuf, NULL);
		perror("/bin/sh");
		exit(1);
	}
}

#define SHUT_FD1 \
	if (fd1 >= 0) {                 \
		DEBUG_OUT("shutdown\n"); \
		close(fd1);                \
		fd1 = -1;                   \
	}

#define BUF_SIZE 1024

static void doit(int);

void
usage(char *cmd)
{
	fprintf(stderr, "usage:\t%s [-p port] [-i interface] [-d]\n", cmd);
	exit(1);
}

int
nametosig(char *name)
{
	int sig = 0;
	if (!strcmp(name, "SIGHUP")) return SIGHUP;
	if (!strcmp(name, "SIGINT")) return SIGINT;
	if (!strcmp(name, "SIGQUIT")) return SIGQUIT;
	if (!strcmp(name, "SIGILL")) return SIGILL;
	if (!strcmp(name, "SIGTRAP")) return SIGTRAP;
	if (!strcmp(name, "SIGABRT")) return SIGABRT;
	if (!strcmp(name, "SIGBUS")) return SIGBUS;
	if (!strcmp(name, "SIGFPE")) return SIGFPE;
	if (!strcmp(name, "SIGKILL")) return SIGKILL;
	if (!strcmp(name, "SIGUSR1")) return SIGUSR1;
	if (!strcmp(name, "SIGSEGV")) return SIGSEGV;
	if (!strcmp(name, "SIGUSR2")) return SIGUSR2;
	if (!strcmp(name, "SIGPIPE")) return SIGPIPE;
	if (!strcmp(name, "SIGALRM")) return SIGALRM;
	if (!strcmp(name, "SIGTERM")) return SIGTERM;
	if (!strcmp(name, "SIGCHLD")) return SIGCHLD;
	if (!strcmp(name, "SIGCONT")) return SIGCONT;
	if (!strcmp(name, "SIGSTOP")) return SIGSTOP;
	if (!strcmp(name, "SIGTSTP")) return SIGTSTP;
	if (!strcmp(name, "SIGTTIN")) return SIGTTIN;
	if (!strcmp(name, "SIGTTOU")) return SIGTTOU;
	if (!strcmp(name, "SIGURG")) return SIGURG;
	if (!strcmp(name, "SIGXCPU")) return SIGXCPU;
	if (!strcmp(name, "SIGXFSZ")) return SIGXFSZ;
	if (!strcmp(name, "SIGVTALRM")) return SIGVTALRM;
	if (!strcmp(name, "SIGPROF")) return SIGPROF;
	if (!strcmp(name, "SIGWINCH")) return SIGWINCH;
	if (!strcmp(name, "SIGIO")) return SIGIO;
	if (!strcmp(name, "SIGSYS")) return SIGSYS;

		/* Optional */
#ifdef SIGEMT
	if (!strcmp(name, "SIGEMT")) return SIGEMT;
#endif
#ifdef SIGPWR
	if (!strcmp(name, "SIGPWR")) return SIGPWR;
#endif
#ifdef SIGINFO
	if (!strcmp(name, "SIGINFO")) return SIGINFO;
#endif
#ifdef SIGLOST
	if (!strcmp(name, "SIGLOST")) return SIGLOST;
#endif

	if (sscanf(name, "%d", &sig)) return sig;
	return 0;
}

int
main(int argc, char **argv)
{
	int port = 4700;
	int h;
	int ch;
	int daemonize = 0;
	char *interface_name = NULL;
	int keepalive = 1;

	while ((ch = getopt(argc, argv, "+i:p:ndv")) != EOF)
		switch (ch) {
		case 'i':
			interface_name = strdup(optarg);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'd':
			daemonize = 1;
			break;
		case 'n':
			keepalive = 0;
			break;
#ifdef DEBUG
		case 'v':
			verbose = 1;
			break;
#endif
		case '?':
		default:
			usage(argv[0]);
		}

	argc -= optind;
	argv += optind;

	if (argc > 0)
		usage(argv[0]);

	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);

	/* daemon() only closes 0-2, but router init leaves 3->/dev/nvram */
	for (h = 3; h < 100; h++)
		(void) close(h);

	h = listen_socket(port, interface_name);
	if (h < 0)
		exit(1);

	if (daemonize && daemon(1, 1) < 0) {
		perror("daemon");
		exit(1);
	}

	/* Don't use LOG_PERROR on Cygwin */
	printf("starting: port %d; interface %s\n",
	       port, (interface_name)?interface_name:"any");
	openlog(NULL, LOG_PID, LOG_DAEMON);
	syslog(LOG_INFO, "starting: port %d; interface %s",
	       port, (interface_name)?interface_name:"any");
	closelog();

	/* Accept loop */

	for (;;) {
		struct sockaddr_in sa;
		int fd;
		socklen_t salen;
		salen = sizeof(sa);
		if ((fd = accept(h, (struct sockaddr *)&sa, &salen)) < 0) {
			continue;
		} else {
			/* Create a new session */
			int pid;
			/* Don't ever log connection messages to console */
			openlog(NULL, LOG_PID, LOG_DAEMON);
			syslog(LOG_INFO, "connection from: %s\n",
			       inet_ntoa(sa.sin_addr));

			/* Fall back to console if there's no syslogd */
			openlog(NULL, LOG_CONS | LOG_PID, LOG_DAEMON);

			if ((pid = fork()) < 0) {
				perror("fork");
			}
			if (pid == 0) {
				int on = 1;
				struct linger linger;
				/* In child */
				close(h);

				if (keepalive &&
				    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
					(char *)&on, sizeof(on)) < 0)
					syslog(LOG_WARNING,
					       "setsockopt(SO_KEEPALIVE): %m");
				linger.l_onoff = 1;
				linger.l_linger = 20;
				if (setsockopt(fd, SOL_SOCKET, SO_LINGER,
					(char *)&linger, sizeof(linger)) < 0)
					syslog(LOG_WARNING,
					       "setsockopt(SO_LINGER): %m");

				 doit(fd);
				 exit(0);
			 } else {
				 close(fd);
			 }
		 }
	 }
	/* NOTREACHED */
	return 0;
}

static void
doit(int fd1)
{
	int cstdin = -1, cstdout = -1, cstderr = -1;
	char inbuf[BUF_SIZE];
	char sendbuf[BUF_SIZE*3]; /* Room for stdout + stderr + commands */
	char recvbuf[BUF_SIZE*2];
	int inbuf_avail = 0, inbuf_written = 0;
	int sendbuf_avail = 0, sendbuf_written = 0;
	int recvbuf_avail = 0, recvbuf_written = 0;
	char tmp[BUF_SIZE];
	int close_stdin = 0;
	int pid = 0;
	struct timeval to;

	sendbuf_avail = sprintf(sendbuf, "ush %s\n", VERSION);


	for (;;) {
		int r, nfds = 0;
		int cnt = 0;
		fd_set rd, wr;
		FD_ZERO(&rd);
		FD_ZERO(&wr);

		if (pid > 0) {
			int status = 0;

			if (waitpid(pid, &status, WNOHANG) > 0) {
				DEBUG_OUT("%d exited\n", pid);
				if (WIFEXITED(status)) {
					int l = sprintf(tmp, "%d",
						WEXITSTATUS(status));
					sendbuf_avail +=
						sprintf(sendbuf +
						sendbuf_avail,
						"exit %d %dH%s\n",
						session, l, tmp);
				} else if (WIFSIGNALED(status)) {
					int l = sprintf(tmp, "%d",
						WTERMSIG(status));
					sendbuf_avail +=
						sprintf(sendbuf +
						sendbuf_avail,
						"signal-exit %d %dH%s\n",
						session, l, tmp);
				}
				sendbuf_avail +=
					sprintf(sendbuf + sendbuf_avail,
						"eos %d\n", session);
			}
		}

		DEBUG_OUT("[");

		/* Readers */
		if (fd1 >= 0 && recvbuf_avail < sizeof(recvbuf)) {
			DEBUG_OUT(" %d(net) ", fd1);
			FD_SET(fd1, &rd);
			nfds = max(nfds, fd1);
		}
		if (cstdout >= 0 && sendbuf_avail < BUF_SIZE) {
			DEBUG_OUT(" %d(out) ", cstdout);
			FD_SET(cstdout, &rd);
			nfds = max(nfds, cstdout);
		}
		if (cstderr > 0 && sendbuf_avail < BUF_SIZE) {
			DEBUG_OUT(" %d(err) ", cstderr);
			FD_SET(cstderr, &rd);
			nfds = max(nfds, cstderr);
		}

		DEBUG_OUT("][");

		/* Writers */
		if (fd1 >= 0 && sendbuf_avail - sendbuf_written > 0) {
			DEBUG_OUT(" %d(net) ", fd1);
			FD_SET(fd1, &wr);
			nfds = max(nfds, fd1);
		}
		if (cstdin >= 0) {
			if (inbuf_avail - inbuf_written > 0) {
				DEBUG_OUT(" %d(in) ", cstdin);
				FD_SET(cstdin, &wr);
				nfds = max(nfds, cstdin);
			} else if (close_stdin) { /* close after flushing */
				close(cstdin);
				cstdin = -1;
			}
		}

		DEBUG_OUT("]\n");

		if (nfds == 0) {
			/* Nothing left do do */
			return;
		}

		if (cstdout < 0 && cstderr < 0) {
		  /* Closing - polling for exit status */
		  to.tv_sec = 0;
		  to.tv_usec = 20000;
		  r = select(nfds + 1, &rd, &wr, NULL, &to);
		} else {
		  r = select(nfds + 1, &rd, &wr, NULL, NULL);
		}

		if (r == -1 && errno == EINTR)
			continue;
		if (r < 0) {
			perror("select()");
			exit(1);
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

		if (cstdout >= 0 && FD_ISSET(cstdout, &rd)) {
			r = read(cstdout, tmp, BUF_SIZE);
			if (r < 1) {
				close(cstdout);
				cstdout = -1;
				sendbuf_avail +=
					sprintf(sendbuf + sendbuf_avail,
						"eof-stdout %d\n", session);
			} else {
				sendbuf_avail +=
					sprintf(sendbuf + sendbuf_avail,
						"stdout %d %dH", session, r);
				memcpy(sendbuf + sendbuf_avail, tmp, r);
				sendbuf[sendbuf_avail + r] = '\n';
				sendbuf_avail += r + 1;
			}
		}
		if (cstderr >= 0 && FD_ISSET(cstderr, &rd)) {
			r = read(cstderr, tmp, BUF_SIZE);
			if (r < 1) {
				close(cstderr);
				cstderr = -1;
				sendbuf_avail +=
					sprintf(sendbuf + sendbuf_avail,
						"eof-stderr %d\n", session);
			} else {
				sendbuf_avail +=
					sprintf(sendbuf + sendbuf_avail,
						"stderr %d %dH", session, r);
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
		if (cstdin >= 0 && FD_ISSET(cstdin, &wr)) {
			r = write(cstdin,
				inbuf + inbuf_written,
				inbuf_avail - inbuf_written);
			if (r < 1) {
				close(cstdin);
				cstdin = -1;
			} else
				inbuf_written += r;
		}

		/* check if write data has caught read data */
		if (recvbuf_written == recvbuf_avail)
			recvbuf_written = recvbuf_avail = 0;
		if (sendbuf_written == sendbuf_avail)
			sendbuf_written = sendbuf_avail = 0;
		if (inbuf_written == inbuf_avail)
			inbuf_written = inbuf_avail = 0;

		/* Protocol */
		while (recvbuf_avail > 0) {
			int len = 0;
			int signum;
			char signame[20];
			DEBUG_DATA("parsing [", recvbuf, recvbuf_avail, "]\n");
			if (sscanf(recvbuf, "new %d %dH%n",
				&session, &len, &cnt)) {
				memcpy(tmp, recvbuf + cnt, len);
				tmp[len] = 0;
				pid = connect_cmd(tmp,
					&cstdin, &cstdout, &cstderr);
				close_stdin = 0;
				if (pid < 0) {
					SHUT_FD1;
					break;
				}

			} else if (sscanf(recvbuf, "stdin %d %dH%n",
				&session, &len, &cnt)) {
				if (cnt + len > recvbuf_avail) {
					DEBUG_OUT("incomplete\n");
					break;
				} else if (inbuf_avail + len > sizeof(inbuf)) {
					DEBUG_OUT("in would block(%d, %d)\n",
					inbuf_avail, len);
					break;
				} else {
					memcpy(inbuf + inbuf_avail,
						recvbuf + cnt, len);
					inbuf_avail += len;
				}
			} else if (sscanf(recvbuf, "eof-stdin %d%n",
				&session, &cnt)) {
				DEBUG_OUT("Close stdin\n");
				close_stdin = 1;
			} else if (sscanf(recvbuf, "signal %d %20s%n",
				&session, signame, &cnt)) {
				signum = nametosig(signame);
				DEBUG_OUT("signal %s(%d)\n", signame, signum);
				if (pid > 0) {
					DEBUG_OUT("kill %d %d\n", pid, signum);
					if (kill(-pid, signum) != 0) {
						perror("kill");
						sendbuf_avail +=
							sprintf(sendbuf +
							sendbuf_avail,
								"exit %d 3H255\n",
							session);
					}
				}
			} else {
				WARN_DATA("Unknown command: \"",
					recvbuf, recvbuf_avail, "\"\n");
				/* Clear buffer */
				cnt = -1; len = recvbuf_avail;
				break;
			}
			/* Shift to next command */
			recvbuf_avail -= cnt + len + 1;
			memmove(recvbuf, recvbuf + cnt + len + 1,
				recvbuf_avail);
		}
	}
	return;
}
