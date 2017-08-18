/*
 * A cheap version of the "ttcp" command supporting:
 * ttcp -ts and -tsu - transmit tcp and udp
 * ttcp -rs and -rsu - receive tcp and udp
 */

#include "windows.h"
#include "stdio.h"
#include "winsock.h"
#include "string.h"
#include "Winbase.h"

void bzero(char *buf, int n)
{
	while(n--)
		*buf++ = 0;
}

int isprint(int c)
{
	if( (c>=0x20) && (c<=0x7E) )
		return 1;
	else
		return 0;
}

static void
pattern(char *cp, int cnt)
{
	char c;
	int cnt1;

	cnt1 = cnt;
	c = 0;
	while (cnt1-- > 0) {
		while (!isprint(c & 0x7f))
			c++;
		*cp++ = (c++ & 0x7f);
	}
	
}

#define	MAXLEN	(16 * 1024)

static char buf[MAXLEN];

static void delay(int us);

int udp;
int trans;
int num;
int len;
int sinkmode;
short port;
struct sockaddr_in sinhim, sinme;
struct sockaddr_in frominet;
int fromlen;
char *host;
char *place;

int opterr;
int optind;
int optopt;
char *optarg;

#define	EMSG	""

extern int getopt(int argc, char **argv, char *ostr);
extern void syserr(char *s);

void
ttcp(char *line)
{
	int ac;
	char *av[32];
	int c;
	SOCKET fd;
	SOCKET newfd;
	int cnt;
	int nbytes;
	int die;
	int err;
    int i;

	/* init globals with default values */
	udp = 0;
	trans = 0;
	num = 2048;
	len = 8192;
	sinkmode = 1;
	port = 5010;
	host = NULL;
	place = EMSG;
	opterr = 1;
	optind = 0;

	/* chop the command line into whitespace-delimited strings */
	for (ac = 0; ac < 32; ) {
		av[ac] = line;
		while ((*line != ' ') && (*line != '\t') && (*line != '\0'))
			line++;
		ac++;
		if (*line == '\0')
			break;
		*line++ = '\0';
		while ((*line == ' ') || (*line == '\t'))
			line++;
		if (*line == '\0')
			break;
	}

	while ((c = getopt(ac, av, "rtsun:l:p:")) != -1) {
		switch (c) {
		case 'l':
			len = atoi(optarg);
			if (len > MAXLEN) {
				printf("len (%d) too big\n", len);
				goto done;
			}
			break;

		case 'n':
			num = atoi(optarg);
			break;

		case 'p':
			port = atoi(optarg);
			break;

		case 'r':
			trans = 0;
			break;

		case 's':	/* nop */
			break;

        case 't':
			trans = 1;
			break;

		case 'u':
			udp = 1;
			break;
		}
	}

	bzero((char*)&sinme, sizeof (sinme));
	bzero((char*)&sinhim, sizeof (sinhim));

	sinme.sin_family = AF_INET;

	if (trans) {
		host = av[optind];

		if ((*host < '0') || (*host > '9')) {
			printf("only dot notation internet addresses currently supported\n");
			return;
		}
		sinhim.sin_family = AF_INET;
		sinhim.sin_addr.s_addr = inet_addr(host);
		sinhim.sin_port = htons(port);
		sinme.sin_port = INADDR_ANY;
	}
	else
		sinme.sin_port = htons(port);

	if ((fd = socket(AF_INET, udp? SOCK_DGRAM: SOCK_STREAM, 0)) == INVALID_SOCKET) {
		syserr("socket");
		return;
	}

	if (!trans) {
		int val;

		val = 1;
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*) &val, sizeof (val)) == SOCKET_ERROR) {
			syserr("setsockopt: REUSEADDR");
			goto done;
		}
	}

	if (trans) {
		if (connect(fd, (struct sockaddr*) &sinhim, sizeof (sinhim)) == SOCKET_ERROR) {
			syserr("connect failed");
			goto done;
		}
		if (!udp)
			printf("connected\n");
	}
	else {	/* rcvr */

		if (bind(fd, (struct sockaddr*) &sinme, sizeof (sinme)) == SOCKET_ERROR) {
			syserr("bind");
			goto done;
		}

		if (!udp && (listen(fd, 2) == SOCKET_ERROR)) {
			syserr("listen");
			goto done;
		}

		fromlen = sizeof (frominet);

		if (!udp && (newfd = accept(fd, (struct sockaddr*) &frominet, &fromlen)) == INVALID_SOCKET) {
			syserr("accept");
			goto done;
		}

		if (!udp)
			printf("ttcp-r: accepted\n");
  
		if (!udp) {
			closesocket(fd);
			fd = newfd;
		}
	}

	pattern(buf, len);

	nbytes = 0;

	if (trans) {

		if (udp)
			send(fd, buf, 4, 0);	/* rcvr start */
		
		while (num > 0) {
			cnt = send(fd, buf, len, 0);
			if (cnt == SOCKET_ERROR) {
				err = WSAGetLastError();
				if ((err == EMSGSIZE) || (err == ENOBUFS)) {
					delay(18000);
					continue;
				}
				syserr("write");
				goto done;
			}
			else if (cnt != len) {
				syserr("short write");
				goto done;
			}
			nbytes += len;
			num--;
		}

		if (udp)
			send(fd, buf, 4, 0);	/* rcvr stop */
	}
	else {
		if (udp) {
			die = 0;
			while ((cnt = recv(fd, buf, len, 0)) > 0) {
				if (cnt == SOCKET_ERROR) {
					syserr("read");
					goto done;
				}
				if (cnt <= 4) {
					if (die)
						goto done;
					die = 1;
				}
				nbytes += cnt;
			}
		}
		else {
			while ((cnt = recv(fd, buf, len, 0)) > 0)
				nbytes += cnt;
			if (cnt == SOCKET_ERROR) {
				syserr("read");
				goto done;
			}
		}
	}

done:
	if (trans)
		printf("wrote %d bytes\n", nbytes);
	else
		printf("read %d bytes\n", nbytes);

	closesocket(fd);
	return;
}

#define	BADCH ('?')

void
syserr(char *s)
{
	printf("%s: error %d\n", s, WSAGetLastError());
}

static void
error(char *pch)
{
	if (!opterr)
		return;
	printf("ttcp: %s: %c\n", pch, optopt);
}

static void
delay(int us)
{
	int ms;

	ms = (us / 1000);
	Sleep(ms);
}

int
getopt(int argc, char **argv, char *ostr)
{
        register char *oli;                        /* option letter list index */

        if (!*place) {
                /* update scanning pointer */
                if (optind >= argc || *(place = argv[optind]) != '-' || !*++place) {
                        return EOF; 
                }
                if (*place == '-') {
                        /* found "--" */
                        ++optind;
                        return EOF;
                }
        }

        /* option letter okay? */
        if ((optopt = (int)*place++) == (int)':'
                || !(oli = strchr(ostr, optopt))) {
                if (!*place) {
                        ++optind;
                }
                error("illegal option");
                return BADCH;
        }
        if (*++oli != ':') {        
                /* don't need argument */
                optarg = NULL;
                if (!*place)
                        ++optind;
        } else {
                /* need an argument */
                if (*place) {
                        optarg = place;                /* no white space */
                } else  if (argc <= ++optind) {
                        /* no arg */
                        place = EMSG;
                        error("option requires an argument");
                        return BADCH;
                } else {
                        optarg = argv[optind];                /* white space */
                }
                place = EMSG;
                ++optind;
        }
        return optopt;                        /* return option letter */
}

/* WindowsCE Main routine */
void main( DWORD	hInstance, DWORD hPrevInstance, TCHAR *pCmdLine, int nShowCmd )
{
    char cmdline[256];

    /* Convert UNICODE to ansi cmd line string */
    if(wcstombs(cmdline, pCmdLine, 256) == -1) {
        printf("Illegal command line character\n\r");
        return;
    }

    ttcp( cmdline );
}
