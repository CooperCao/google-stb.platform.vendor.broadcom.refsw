/*
 * FILE-CSTYLED
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
 * $Id$
 */

/*
 *        T T C P . C
 *
 * Test TCP connection.  Makes a connection on port 5001
 * and transfers fabricated buffers or data copied from stdin.
 *
 * Usable on 4.2, 4.3, and 4.1a systems by defining one of
 * BSD42 BSD43 (BSD41a)
 * Machines using System V with BSD sockets should define SYSV.
 *
 * Modified for operation under 4.2BSD, 18 Dec 84
 *       T.C. Slattery, USNA
 * Minor improvements, Mike Muuss and Terry Slattery, 16-Oct-85.
 * Modified in 1989 at Silicon Graphics, Inc.
 *        catch SIGPIPE to be able to print stats when receiver has died 
 *        for tcp, don't look for sentinel during reads to allow small transfers
 *        increased default buffer size to 8K, nbuf to 2K to transfer 16MB
 *        moved default port to 5001, beyond IPPORT_USERRESERVED
 *        make sinkmode default because it is more popular, 
 *                -s now means don't sink/source 
 *        count number of read/write system calls to see effects of 
 *                blocking from full socket buffers
 *        for tcp, -D option turns off buffered writes (sets TCP_NODELAY sockopt)
 *        buffer alignment options, -A and -O
 *        print stats in a format that's a bit easier to use with grep & awk
 *        for SYSV, mimic BSD routines to use most of the existing timing code
 * Modified by Steve Miller of the University of Maryland, College Park
 *        -b sets the socket buffer size (SO_SNDBUF/SO_RCVBUF)
 * Modified Sept. 1989 at Silicon Graphics, Inc.
 *        restored -s sense at request of tcs@brl
 * Modified Oct. 1991 at Silicon Graphics, Inc.
 *        use getopt(3) for option processing, add -f and -T options.
 *        SGI IRIX 3.3 and 4.0 releases don't need #define SYSV.
 *
 * Modified --> Nov 1996 at CERN (Daniel DAVIDS)
 *        printout of the Socket-Buffer-Sizes 
 *        configured for HP-UX 9000 OS        
 *        configured for Windows NT OS 
 * Modified Dec 1996 at CERN (Jacques ROCHEZ)
 *        severe cleanup
 *        addaptation to the gcc compiler (ANSI)
 *        configured for Lynx OS 
 *        automatic format for the rate display (G/M/K/)bytes/sec
 *        added log (L) and more help (h) options.
 * Modified May 1997 at CERN (Jacques ROCHEZ) 
 *        removed the mes() function following err() function.
 *        changed the default port to 5010
 * Modified jul 1997 at CERN (Jacques ROCHEZ)
 *        adapted the timing calculation in microseconds 
 *        addapted the code for Vsisual C++ under NT4.0 
 * Modified aug 1997 at CERN (Jacques ROCHEZ)
 *        initialise to 0 the variables nbytes, numcalls  
 *        moved the buffer pre-load outside the measured timed area 
 * Distribution Status -
 *        Public Domain.  Distribution Unlimited.
 */
#ifndef lint
static char RCSid[] = "ttcp.c $- CERN Revision: 3.8 (dev level) -$";
#endif

static char VersDate[] = "01-aug-97";

/*                system dependent setting
 *                ------------------------
 * uname -a,gcc -v a.c are the tools used                              
 *
 * Platform/OS          #define         MACRO predefined 
 * -----------          -------   ---------------------------------------------------
 * SunOS OS             BSD43     __unix__   __sun__   __sparc__
 * SUN Solaris           SYSV     __unix__   __sun__   __sparc__   __svr4__ 
 * SGI-IRIX  < 3.3 SYSV     set as #define sgi
 * HP-UX 9000           SYSV      __unix__   __hpux__  __hp9k8__
 * OSF/1 V3.2           SYSV      __unix__   __osf__   __alpha__
 * OSF/1 V4.0           SYSV      __unix__   __osf__   __alpha__   _CFE
 * LynxOS               SYSV      __unix__   __lynx__  __powerpc__
 * Windows NT           SYSV                 __WINNT__ __i386__    __M_COFF__
 * AIX                  SYSV      _AIX       _AIX32    _POWER      _IBMR2   _ARCH_PWR
 

 * Micosoft Visual C++ compiler under WindowNT 4.0
 * Windows NT                    _WINDOWS    WIN32 

 * Unix BSD 41a         BSD41a
 *          42          BSD42
 *          43          BSD43
 
 * Machines using System V with BSD sockets should define SYSV.
 *
 *            Compiler commands 
 *            -----------------
 * LynxOS : gcc -c ttcp.c -o ttcp.o | gcc -o ttcp -O ttcp.o -lnetinet -lc_p -lc
 */

/* -------------attempt to set an automatic UNIX  OS family detection -------*/

#if defined(__hp9k8__) || defined(__osf__) || defined(__srv4__)  
#define SYSV
#endif
#if defined(__lynx__)  
#define SYSV
#endif
/* for solaris (__srv4__) the parameters SYSV is already set */

/* version A.09.01  'key words' differs from A.09.05 A */ 
#if defined(__hpux)
#define __hpux__
#endif

#if defined(__sun__)&&!defined(__srv4__)
#define BSD43                  /* changed by BSD42 if necessary */
#endif


#if defined(__FreeBSD__)
#define BSD43
#endif
/*--------------------------------------------------------------------------*/

#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>


#if defined(SYSV)

#if defined(__osf__) 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>           /* struct timeval */
#include <sys/resource.h>       /* definition of struct rusage */    

#else  /* else of __osf__ */ 
#if defined(__NTVIS__)
#include <windows.h>            /* required for all Windows applications */
#include <memory.h>
#include <time.h>
#include <sys\timeb.h> 
#include <process.h>            /* for _beginthread                      */
#include <stdlib.h>

struct rusage { struct timeval ru_utime, ru_stime; };
#define RUSAGE_SELF 0

#define bcopy(a,b,n) memcpy((b), (a), (n))
#define bzero(a,n) memset((a), 0, (n))
extern int getopt();

#else  /* else of __NTVIS__ */  
#if defined(__lynx__)
#include <socket.h>             /* located in  /usr/include/..... */
#include <netinet/in.h>    
#include <netinet/tcp.h> 
#include <arpa/inet.h>     
#include <netdb.h>         
#include <time.h> 
#include <resource.h>           /* definition of struct rusage */    
#include <sys/times.h>
#define RUSAGE_SELF 0
#include <conf.h>               /* definition of TICKSPERSEC (HZ) */
#include <sys/param.h>     

#else  /* else of __Lynx__  */
#if defined(__svr4__)
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>           /* struct timeval */
#include <sys/resource.h>       /* definition of struct rusage */    
#include <sys/times.h>
#define RUSAGE_SELF 0
#include <sys/param.h>     

#else  /* else of __svr4__    all SYSV cases except those mentionned before */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>           /* struct timeval */
#include <sys/resource.h>       /* definition of struct rusage */    
#include <sys/times.h>
#define RUSAGE_SELF 0
#include <sys/param.h>     

#endif /* __svr4__  */
#endif /* __lynx__  */
#endif /* __NTVIS__ */
#endif /* __osf__   */

#else  /* else of SYSV      it is a BSD OS  */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>           /* struct timeval */
#include <sys/resource.h>       /* definition of struct rusage */

#endif /* SYSV */

#if defined(__NTVIS__)
#ifndef _GETOPT_
#define _GETOPT_
int getopt(int argc, char **argv, char *optstring);

extern char *optarg;                // returned arg to go with this option
extern int optind;                  // index to next argv element to process
extern int opterr;                  // should error messages be printed?
extern int optopt;                  //

#define BADCH ('?')
#endif // _GETOPT

/* get option letter from argument vector  */
int
        opterr = 1,                  // should error messages be printed?
        optind = 1,                  // index into parent argv vector
        optopt;                      // character checked for validity
char         *optarg;                // argument associated with option

#define EMSG        ""
char *progname;                      // may also be defined elsewhere
#endif /*__NTVIS__*/

/* sockaddr_in ==  file server address structure 
 * 
 * Socket address, internet style.   declared in : /netinet/in.h
 * struct sockaddr_in {short   sin_family;
 *                     u_short sin_port;
 *                     struct  in_addr sin_addr;
 *                     char    sin_zero[8];
 *                    };
 *
 * Structure used by kernel to store most addresses. declared in ./sys/socket.h
 * struct sockaddr{u_short sa_family;       address family
 *                 char    sa_data[14];     up to 14 bytes of direct address 
 *                };
 * PS : sin stand for "socket internet number" 
 */

#if defined(__sun__)
struct sockaddr_in sockaddr;   /* done in ./X11/Xdmcp.h */
#endif 

struct sockaddr_in sinme;     /* is the socket struct. in the local host */
struct sockaddr_in sinhim;    /* is the socket struc. in the remote host */

#if defined(__lynx__) || defined(__svr4__) || defined(_AIX) 
struct sockaddr frominet;
#else
struct sockaddr_in frominet;
#endif /* __lynx__ */

int domain, fromlen;

#if defined(__NTVIS__)
SOCKET fd;                             /* fd of network socket */

#else
int fd;                                /* fd of network socket */
#endif /* __NTVIS__ */

#if !defined(__lynx__)
extern int errno;
#endif 

#include <stdio.h>

FILE  *fplog;                          /* file pointer for the log file */
char  logfile[100];                    /* file name for the log */
static char logfile_head[] ="ttcp_log";   /* header  name for the log */
int   buflen = 8 * 1024;               /* length of buffer */
char  *buf;                            /* ptr to dynamic buffer */
int   nbuf = 2 * 1024;                 /* number of buffers to send in sinkmode */

int   bufoffset = 0;                   /* align buffer to this */
int   bufalign = 16*1024;              /* modulo this */

int   udp = 0;                         /* 0 = tcp, !0 = udp */
int   options = 0;                     /* socket options */
int   one = 1;                         /* for 4.3 BSD style setsockopt() */
short port = 5010;                     /* TCP port number */
char *host;                            /* ptr to name of host */
int  trans;                            /* 0=receive, !0=transmit mode */
int  debug = 0;                        /* 0=No-Debug, 1=Debug-Set-On */
int  sinkmode = 0;                     /* 0=normal I/O, !0=sink/source mode */
int  verbose = 0;                      /* 0=print basic info, 1=print cpu rate, proc
                                        * resource usage. */
int  nodelay = 0;                      /* set TCP_NODELAY socket option */
int  b_flag = 0;                       /* use mread() */
int  log_cnt = 0;                      /* append result to a log */
int  sockbufsize = 0;                  /* socket buffer size to use */
char fmt = 'A';                        /* output format: k = kilobits, K = kilobytes,
                                        *  m = megabits, M = megabytes, 
                                        *  g = gigabits, G = gigabytes 
                                        *  A = automatic Xbytes (default) */
int  touchdata = 0;                    /* access data after reading */

#define MAX_STRING 256
char     stats[MAX_STRING];            /* is the string for rusage output */
double   nbytes;                       /* bytes on net */
unsigned long numCalls;                /* # of I/O system calls */
double   cput, realt;                  /* user, real time (useconds) */
double   cput_min, realt_min;          /* lower boundary condition */

struct hostent *addr;
extern int optind;
extern char *optarg;



/*-----------Prototype functions definitions -------------------------------*/
/*--------------------------------------------------------------------------*/

                                   /* ANSI input/output functions (stdio.h) */

#if defined(__lynx__)
int  getopt(int, char**, char*);
int  gettimeofday(struct timeval *tp, struct timezone *tzp);

#else
#if defined(__svr4__)

#else 
#if defined(_AIX)

#else
#if defined(__hpux__)
#else
#if defined(__NTVIS__)
#else 
#if defined(BSD42) || defined(BSD43) || defined(linux)
#else

int  printf( char*, ...);
int  fprintf(FILE*,char*, ...);
void perror(char*);
int  getopt(int, char**, char*);
int  gettimeofday(struct timeval *tp, struct timezone *tzp);
#endif
#endif  /* __hpux__ */
#endif  /*  _AIX    */
#endif  /* __svr4__ */
#endif  /* __lynx__ */ 
#endif  /* __NTVIS__ */

void main(int argc, char* argv[]); 
int  read(int, char*, int);
int  write(int, char*, int);
int  close(int);
int  fclose(FILE *stream);

#if !defined(__NTVIS__)
int  atoi(char*);
char strncpy(char *s1,char *s2,size_t n);
void bzero(char*,int);
void bcopy(char*, char*, int);
int  malloc(int);
#endif


                           /* ANSI socket functions prototype /sys/socket.h */
#if defined(__lynx__)
int  select(int, fd_set*, fd_set*, fd_set*, struct timeval*); 

#else 
#if defined(__svr4__)

/*  informations in : /usr/include/sys/socket.h */
int  socket(int, int, int); 
int  connect(int, struct sockaddr *, int);
int  bind(int, struct sockaddr *, int);
int  listen(int, int);
int  accept(int, struct sockaddr *, int *);
int  sendto(int, const char *, int, int, const struct sockaddr *, int);
int  recvfrom(int, char *, int, int, struct sockaddr *, int *);
int  getpeername(int, struct sockaddr *, int *);
int  getsockopt(int, int, int, char *, int *);
int  select(int, fd_set*, fd_set*, fd_set*, struct timeval*); 

#else
#if defined(_AIX)
int  select(unsigned long, void *, void *, void *, struct timeval *);

#else
#if defined(__hpux__)
int getrusage(int who,struct rusage *rusage);

#else 
#if defined(__NTVIS__)

#else
#if defined(BSD42) || defined(BSD43) || defined(linux)

#else

int  socket(int, int, int);
int  connect(int s,struct sockaddr_in *name, int namelen);
int  bind(int s,struct sockaddr *name,int namelen);
int  listen(int, int);
int  accept(int, struct sockaddr_in *, int *);
int  sendto(int, char *, int, int, struct sockaddr_in *, int);
int  recvfrom(int, char *, int, int, struct sockaddr_in *, int *);
int  getpeername(int, struct sockaddr *, int *);
int  setsockopt(int, int, int, char *, int);

int  getsockopt(int, int, int, char*, int*);
int  select(int, fd_set*, fd_set*, fd_set*, struct timeval*); 

#endif
#endif /* __hpux__ */
#endif /*  _AIX    */
#endif /* __svr4__ */
#endif /* __lynx__ */ 
#endif /* __NTVIS__ */

/* ttcp prototype functions */
int  getrusage(int who,struct rusage *rusage);
void err(char *);
void mes(char *);
void pattern(char *, int);
int  Nread(int, void *, int);
int  Nwrite(int, void *, int);
void delay(int);
int  mread(int, char *,unsigned);
char *outfmt(double);
char *outfmtlog(double);
void prep_timer(void);
void read_timer(char *, int);
void prusage(struct rusage*,  struct rusage*,
             struct timeval*, struct timeval*, char *); 
void tvadd(struct timeval *tsum, struct timeval *t0,struct timeval *t1);
void tvsub(struct timeval *tdiff, struct timeval *t1, struct timeval *t0);
void psecs(long, char*);
void sleep(int);
void open_log(void);
void close_log(void);
void do_Usage(void);
void do_help(void);

#if !defined(__NTVIS__)
/* 
void sigpipe(void); 
lynx void(*signal (sig, func))() 
      int sig; 
      void(*func)(); 
      void (*signal (int, void (*)(int))) (int); ... rchrch c'est du chinois 
*/
#endif

/*--------------------------------------------------------------------------*/
#if !defined(__NTVIS__)
void 
sigpipe()
{;
}
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void                       
main(argc,argv)
int argc;
char **argv;
{
  unsigned long addr_tmp;
  int c;
  int sockbufsndsize,sockbufrcvsize;
  int sockbuflen;
  struct sockaddr_in peer;
  int peerlen = sizeof(peer);

#if defined(__NTVIS__)
 extern char *optarg;
 WSADATA WSAData;
 WSAStartup(MAKEWORD(1,1), &WSAData);
#endif /* __NTVIS__ */

 #if defined (DEBUG) 
 debug = 1;
#else 
 debug = 0;
#endif 

  if (argc < 2) { do_Usage();         exit(1);}
  while ((c = getopt(argc, argv, "hdrstuvVBDTLb:f:l:n:p:A:O:")) != -1) 
    {
      switch (c) 
       {
        case 'V':
           fprintf(stdout,"%s %s\n" , RCSid , VersDate );
           exit(0);
        case 'B':
          b_flag = 1;
          break;
        case 'L':
          log_cnt = 1;
          break;
        case 'h':
          do_help();
          exit(1);
          break;
        case 't':
          trans = 1;
          break;
        case 'r':
          trans = 0;
          break;
        case 'd':
          options |= SO_DEBUG;
          break;
        case 'D':
#ifdef TCP_NODELAY
        nodelay = 1;
#else
        fprintf(stderr, 
       "ttcp: -D option ignored: TCP_NODELAY socket option not supported\n");
#endif
          break;
        case 'n':
          nbuf = atoi(optarg);
          break;
        case 'l':
          buflen = atoi(optarg);
          break;
        case 's':
          sinkmode = !sinkmode;
          break;
        case 'p':
          port = atoi(optarg);
          break;
        case 'u':
          udp = 1;
          break;
        case 'v':
          verbose = 1;
          break;
        case 'A':
          bufalign = atoi(optarg);
          break;
        case 'O':
          bufoffset = atoi(optarg);
          break;
        case 'b':
#if defined(SO_SNDBUF) || defined(SO_RCVBUF)
          sockbufsize = atoi(optarg);
#else
          fprintf(stderr, 
"ttcp: -b option ignored: SO_SNDBUF/SO_RCVBUF socket options not supported\n");
#endif
          break;
        case 'f':
          fmt = *optarg;
          break;
        case 'T':
          touchdata = 1;
          break;
                
        default:
          {do_Usage(); exit(1);}
       }/*switch */
    }/* while */

 /* ----------------------- main part ----------------------- */

if (log_cnt) open_log(); 

/* buffer allocation */

 if (udp && buflen < 5) 
   buflen = 5;                /* send more than the sentinel size */
 
 if ( (buf = (char *)malloc(buflen+bufalign)) == (char *)NULL) 
      err("malloc");
 if (bufalign != 0)
      buf +=(bufalign - ((int)buf % bufalign) + bufoffset) % bufalign;
 
 fprintf(stdout,"ttcp%s: buflen=%d, nbuf=%d, align=%d/%d, port=%d\n",
            trans?"-t":"-r",buflen, nbuf, bufalign, bufoffset, port);
 if (log_cnt)fprintf(fplog," %6d %6d %6d %6d %4d",
                      buflen, nbuf, bufalign, bufoffset, port);

 /* preload the buffer for the transmit condition */
 pattern( buf, buflen );

 if(trans)  
   {/* xmitr */
     if (optind == argc) { do_Usage(); exit(1); }
     
     bzero((char *)&sinhim, sizeof(sinhim));
     host = argv[optind];
     if (atoi(host) > 0 )  
       {/* Numeric */
         sinhim.sin_family = AF_INET;
         sinhim.sin_addr.s_addr = inet_addr(host);
       } 
     else 
       {if ((addr=gethostbyname(host)) == NULL) err("bad hostname");
        sinhim.sin_family = addr->h_addrtype;
        bcopy(addr->h_addr,(char*)&addr_tmp, addr->h_length);
        sinhim.sin_addr.s_addr = addr_tmp;
       }
     sinhim.sin_port = htons(port);
     sinme.sin_port = 0;                /* free choice */
   } 
 else 
   {/* rcvr */
     sinme.sin_port =  htons(port);
   }
#if defined(__NTVIS__)
 sinme.sin_family = AF_INET;
#endif
 
#if defined(__NTVIS__)
 if ((fd = socket(AF_INET, udp?SOCK_DGRAM:SOCK_STREAM, 0)) == INVALID_SOCKET)
      err("socket");
#else
 if ((fd = socket(AF_INET, udp?SOCK_DGRAM:SOCK_STREAM, 0)) < 0)
   err("socket");
#endif
 
 if (verbose) 
   {fprintf(stdout,"ttcp%s: File-Descriptor 0x%x Opened\n" ,
            trans?"-t":"-r", fd );
   }


#if defined(__NTVIS__)
 if (bind(fd, (struct sockaddr FAR *)&sinme, 
          sizeof(sinme)) == SOCKET_ERROR) 
   {fprintf(stderr, "bind: %d is the error\n", WSAGetLastError());
    perror("bind");
    exit(1);
   }
#else
#if defined(__lynx__) || defined(__sun__) || defined(_AIX) || defined(__FreeBSD__)

   if (bind(fd, (struct sockaddr * ) &sinme, sizeof(sinme)) < 0) 
   err("bind");

#else  

 if (bind(fd, &sinme, sizeof(sinme)) < 0)
   err("bind");

#endif /* __sun__  __lynx__ _AIX */
#endif /* __NTVIS__ */


#if defined(SO_SNDBUF) || defined(SO_RCVBUF)
 if (sockbufsize) 
   {
#if defined(__lynx__) || defined(__sun__)  || defined(__NTVIS__)
     if (trans) 
       {if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *)&sockbufsize,
                       sizeof sockbufsize) < 0)  
        err("setsockopt: sndbuf");
       } 
     else 
       {if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&sockbufsize,
                       sizeof sockbufsize) < 0)  
        err("setsockopt: rcvbuf");
       }

#else
     if (trans) 

         {if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sockbufsize,
                       sizeof sockbufsize) < 0) 
        err("setsockopt: sndbuf");
       } 
     else 
       {if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &sockbufsize,
                       sizeof sockbufsize) < 0) 
        err("setsockopt: rcvbuf");
       }
#endif /* __lynx__ __sun__ __NTVIS__ */ 
   }
 else
   {/*
    ** Added by Daniel Davids to Know Socket-Buffer-Sizes
    */
     sockbuflen = sizeof sockbufsndsize;
#if defined(__lynx__) || defined(__sun__) || defined(__NTVIS__)
     getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *)&sockbufsndsize, 
                &sockbuflen);
     sockbuflen = sizeof sockbufrcvsize;
     getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&sockbufrcvsize, 
                     &sockbuflen);
#else
     getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sockbufsndsize, &sockbuflen);
     sockbuflen = sizeof sockbufrcvsize;
     getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &sockbufrcvsize, &sockbuflen);
#endif /* __lynx__ __sun__ __NTVIS__ */
     sockbufsize = ( sockbufsndsize + sockbufrcvsize ) / 2;

     if ( sockbufsndsize != sockbufrcvsize )
       {fprintf(stdout, "sockbufsndsize=%d, ", sockbufsndsize );
       fprintf(stdout, "sockbufrcvsize=%d, ", sockbufrcvsize );
       }
   }
#endif /* defined(SO_SNDBUF) || defined(SO_RCVBUF) */
 
 if (sockbufsize) fprintf(stdout, "sockbufsize=%d, \n", sockbufsize);

 if (log_cnt)
   {if (sockbufsize)fprintf(fplog," %6d",sockbufsize);
    else fprintf(fplog," 0");
   }



 if (trans)  fprintf(stdout, "# %s sender -> %s #\n", udp?"udp":"tcp", host);
 else        fprintf(stdout, "# %s receiver #\n", udp?"udp":"tcp");
 
 if (!udp)  
   {
#if !defined(__NTVIS__)
     signal(SIGPIPE, sigpipe);
#endif /* !__NTVIS__ */

     if (trans) 
       {/* We are the client if transmitting */
         if (options)  
           {
#if defined(BSD42)
            if( setsockopt(fd, SOL_SOCKET, options, 0, 0) < 0)
#else /* BSD43 */
       if( setsockopt(fd, SOL_SOCKET, options, (char *)&one, sizeof(one)) < 0)
#endif /* BDS42 */
           err("setsockopt");
           }
#ifdef TCP_NODELAY
         if (nodelay) 
           {struct protoent *p;
            p = getprotobyname("tcp");
#if defined(__lynx__)  || defined(__sun__)        || defined(__NTVIS__)
                if( p && setsockopt(fd, p->p_proto, TCP_NODELAY, 
                                      (char *) &one, sizeof(one)) < 0)
#else 
            if( p && setsockopt(fd, p->p_proto, TCP_NODELAY, 
                                      &one, sizeof(one)) < 0)
#endif /* __lynx__  __sun__ __NTVIS__ */
            err("setsockopt: nodelay");
           }
#endif /* TCP_NODELAY */
#if defined(__lynx__) || defined(__svr4__) || defined(_AIX) || defined(__NTVIS__) || \
	defined(__FreeBSD__)
         if (connect(fd, (struct sockaddr *)&sinhim, sizeof(sinhim) ) < 0) 
#else
         if(connect(fd, &sinhim, sizeof(sinhim) ) < 0)
#endif /* __lynx__ __svr4__ _AIX __NTVIS__ */
         err("connect");
       }/* if (trans) */ 
     else 
       {/* otherwise, we are the server and should listen for connections */

errno = 0;

#if defined(sgi) || ( defined(__osf__) && !defined(_CFE) )
        if( listen(fd,1) <0 )
#else
        if( listen(fd,0) <0 )   /* allow a queue of 0 */
#endif
        err("listen");

        if(options)  
          {
#if defined(BSD42)
           if( setsockopt(fd, SOL_SOCKET, options, 0, 0) < 0)
#else 
#if defined(__lynx__) || defined(__sun__) || defined(__NTVIS__)
            if( setsockopt(fd, SOL_SOCKET, options, (char *) &one,
                              sizeof(one)) < 0)
#else
            if( setsockopt(fd, SOL_SOCKET, options, &one, 
                                sizeof(one)) < 0)
#endif /* __lynx__ __sun__ __NTVIS__ */
#endif /* BSD42 */
                   
              err("setsockopt");
          }
        fromlen = sizeof(frominet);
        domain = AF_INET;
#if defined(__NTVIS__) || defined(__FreeBSD__)
        if((fd=accept(fd, (struct sockaddr * ) &frominet, &fromlen) ) < 0) 
                err("accept");
#else 
        if((fd=accept(fd, &frominet, &fromlen) ) < 0) err("accept");
#endif /* __NTVIS__ */


#if defined(__lynx__)|| defined(__sun__) || defined(_AIX) || defined(__NTVIS__) || \
	defined(__FreeBSD__)
        if (getpeername(fd,(struct sockaddr *) &peer, &peerlen) < 0)
          err("getpeername");
#else
        if (getpeername(fd,(struct sockaddr_in *) &peer,&peerlen) < 0) 
          err("getpeername");
#endif /* __lynx__ __sun__ _AIX __NTVIS__ */
         
        fprintf(stderr,"ttcp-r: accept from %s\n",inet_ntoa(peer.sin_addr));

       } /* otherwise we are ... */
   }

 prep_timer();
 errno = 0;
 nbytes = 0.0;
 numCalls = 0;

 if (sinkmode) 
   {register int cnt,multi;
   int nb = 0;
   multi = nbuf;
   if (trans)  
     {
       /* pattern( buf, buflen );  moved after malloc */
      if(udp)  (void)Nwrite( fd, buf, 4 ); /* rcvr start */
      while (multi-- )
            {if ( (cnt=Nwrite(fd,buf,buflen)) != buflen) 
               {fprintf(stdout,"ttcp%s: failure in the sending command.\n"); break;}
             if(debug)fprintf(stdout,"ttcp%s: %5d | %d Bytes Written in %d write commands \n",
                              trans?"-t":"-r", ++nb, cnt, nbuf );
             nbytes += buflen;
            }
      if(udp)  (void)Nwrite( fd, buf, 4 ); /* rcvr end */
     } /* trans */ 
   else 
     {if (udp) 
       {
         while ((cnt=Nread(fd,buf,buflen)) > 0)  
           {static int going = 0;

           if( cnt <= 4 )  
             {if( going ) break;      /* "EOF" */
             going = 1;
             prep_timer();
             } 
           else 
             {if(debug)fprintf(stdout,
                               "ttcp%s: %5d | %d Bytes Read\n",
                               trans?"-t":"-r", ++nb, cnt );
             nbytes += cnt;
             }
           }
       } /*if (udp) */ 
     else 
       {/* not udp received transfer */
         while ((cnt=Nread(fd,buf,buflen)) > 0)  
           {if(debug)fprintf(stdout,
                             "ttcp%s: %5d | %d Bytes Read\n",
                             trans?"-t":"-r", ++nb, cnt );
           nbytes += cnt;
           }
       }
     }
   }
 else /* if sinkmode */
   {
#if !defined(__NTVIS__)/* else case specific to WINNT */
     register int cnt;
     if (trans)  
       {while((cnt=read(0,buf,buflen)) > 0 &&
              Nwrite(fd,buf,cnt) == cnt)
         nbytes += cnt;
       }  
     else
       {while((cnt=Nread(fd,buf,buflen)) > 0 &&
              write(1,buf,cnt) == cnt)
              nbytes += cnt;
       }
#endif /* __NTVIS__ */
   } /* end of if (sinkmode) case */

 if(errno) err("IO");

 read_timer( stats,sizeof(stats) );

 if(udp&&trans)  
   {(void)Nwrite( fd, buf, 4 ); /* rcvr end */
   (void)Nwrite( fd, buf, 4 ); /* rcvr end */
   (void)Nwrite( fd, buf, 4 ); /* rcvr end */
   (void)Nwrite( fd, buf, 4 ); /* rcvr end */
   }

 /* lower end boundary conditions */ 
cput_min  = 10.0;                                      /* 10 usec */
realt_min = 10.0; 
#if  defined(__NTVIS__)
cput_min  = 1000.0;                    /* 1 msec */
realt_min = 1000.0;
#endif  
 if( cput  <= cput_min )               /* value in usec */
   {cput    = cput_min;  
   fprintf(stdout,
           "ttcp%s: cpu time too short set at %.0f usec, NOT accurate result.\n",
           trans?"-t":"-r",cput_min);
   }

 if( realt <= realt_min)              /* value in usec */  
   {realt   = realt_min; 
   fprintf(stdout,
           "ttcp%s: real time too short, set at %.0f usec, NOT accurate result.\n",
           trans?"-t":"-r",realt_min);
   }
 if (log_cnt)
   { if (realt <= realt_min) fprintf(fplog," 0");
     else fprintf(fplog," 1");
   }
 fprintf(stdout,
         "ttcp%s: %.0f bytes in %.06f real seconds = %s/sec +++\n",
         trans?"-t":"-r",
         nbytes, realt/1000000.0, outfmt((nbytes/realt)*1000000.0)  );


 if (log_cnt)
    {fprintf(fplog," %10.0f %4.06f %s", nbytes, realt/1000000.0,
     outfmtlog( (nbytes/realt)*1000000.0 ) );  
    }

 if (verbose) 
   {
     fprintf(stdout,"ttcp%s: %.0f bytes in %.06f cpu  seconds = %s/cpu sec\n",
             trans?"-t":"-r",
             nbytes,cput/1000000.0, outfmt((nbytes/cput)*1000000.0) );
   }
 /* rchrch the original coding was "1024.0 * realt/(double)numCalls, " why this 1024.0 ?? */
 /* 
 fprintf(stdout,
         "ttcp%s: %ld I/O calls, usec/call = %.3f, calls/sec = %.3f\n",
          trans?"-t":"-r",numCalls,
          realt/(double)numCalls,
         ((double)numCalls/realt)*1000000.0);        
         */
 fprintf(stdout,
         "ttcp%s: %ld I/O calls, %.3f msec(real)/call, %.3f msec(cpu)/call\n",
         trans?"-t":"-r",numCalls,
          (realt/(double)numCalls)/1000.0,
          (cput/(double)numCalls)/1000.0 ) ;

 if (log_cnt)fprintf(fplog," %6ld",numCalls); 

 fprintf(stdout,"ttcp%s: %s\n", trans?"-t":"-r", stats);
 if (verbose) 
   {fprintf(stdout,"ttcp%s: buffer address %#x\n",
            trans?"-t":"-r",(int)buf);     
   }



#if defined(__NTVIS__) 
 closesocket ( fd );
#else 
   close ( fd );
#endif /* __NTVIS__ */

 if (log_cnt) 
   {fflush(stdout);
    close_log();
   }

 if (verbose) fprintf(stdout,"ttcp%s: File-Descriptor  fd 0x%x Closed\n" ,
                     trans?"-t":"-r", fd );

 fprintf(stdout,"ttcp done.\n");
 exit(0);

}
/*--------------------------------------------------------------------------*/
void
err(s)
char *s;
{
        fprintf(stderr,"ttcp%s: ", trans?"-t":"-r");
        perror(s);
        fprintf(stderr,"ttcp%s: errno=%d\n",trans?"-t":"-r",errno);
        exit(1);
}
/*--------------------------------------------------------------------------*/
void
mes(s)
char *s;
{
        fprintf(stderr,"ttcp%s: %s\n", trans?"-t":"-r", s);
}
/*--------------------------------------------------------------------------*/
void                       
pattern( cp, cnt )
register char *cp;
register int cnt;
{
        register char c;
        register int cnt1;
        cnt1 = cnt;
        c = 0;
        while( cnt1-- > 0 )  {
                while( !isprint((c&0x7F)) )  c++;
                *cp++ = (c++&0x7F);
        }
}
/*--------------------------------------------------------------------------*/
char *
outfmt(b)
     double b;           
{
  static char obuf[50];

  switch (fmt) 
    {
      case 'G':
        sprintf(obuf, "%.3f GB", b / 1024.0 / 1024.0 / 1024.0);
        break;
      case 'K':
        sprintf(obuf, "%.3f KB", b / 1024.0);
        break;
      case 'M':
        sprintf(obuf, "%.3f MB", b / 1024.0 / 1024.0);
        break;
      case 'g':
        sprintf(obuf, "%.3f Gbit", b * 8.0 / 1024.0 / 1024.0 / 1024.0);
        break;
      case 'k':
        sprintf(obuf, "%.3f Kbit", b * 8.0 / 1024.0);
        break;
      case 'm':
        sprintf(obuf, "%.3f Mbit", b * 8.0 / 1024.0 / 1024.0);
        break;
      case 'A':
        if ( b >= 1073741824.0)                      /* 1024*1024*1024 */
          sprintf(obuf, "%4.3f GB", b / 1073741824.0 ); 
        else if (b/1048576.0 >= 1.0)
                  sprintf(obuf, "%4.3f MB", b / 1048576.0 );
        else if (b/1024.0 >= 1.0)
                  sprintf(obuf, "%4.3f KB", b / 1024.0);
        else sprintf(obuf, "%4.3f B",b);
        
        break;
      default:
        sprintf(obuf, "default.........."); 
    }
  return obuf;
}
/*--------------------------------------------------------------------------*/
char *
outfmtlog(b)
     double b;           
{
  static char obuf[50];
  /* we print ony in MegaByte */
  sprintf(obuf, "%4.3f", b / 1048576.0 );
  return obuf;
}
/*--------------------------------------------------------------------------*/
static struct        timeval time0;        /* Time at which timing started */
static struct        rusage ru0;        /* Resource utilization at the start */
static struct   tm *tms;        /* system time structure */
/*--------------------------------------------------------------------------*/
#if defined(SYSV)

/*ARGSUSED*/

#if defined(__osf__)
/* getrusage defined in the system lib */
#else 
#if defined(__lynx__)
/* getrusage defined in the system lib */
#else 
#if defined(__sun__) 
/* getrusage defined in the system lib */
#else 

int
getrusage(ignored, ru)
     int ignored;
     register struct rusage *ru;
{


#if defined(__NTVIS__)
  HANDLE phd;
  FILETIME CreateTime, ExitTime, KernelTime, UserTime;
  SYSTEMTIME SysTime;
  phd = GetCurrentProcess();
  if( GetProcessTimes(phd, &CreateTime, &ExitTime, &KernelTime, &UserTime) 
      != TRUE) 
    {ru->ru_stime.tv_sec  = 0;
    ru->ru_stime.tv_usec = 0;
    ru->ru_utime.tv_sec  = 0;
    ru->ru_utime.tv_usec = 0;
    } 
  else 
   {
     (void) FileTimeToSystemTime(&KernelTime, &SysTime);
     /*
      * fprintf(stdout,
      * "System sec=%d, msec=%d\n", SysTime.wSecond, SysTime.wMilliseconds);
      */
     ru->ru_stime.tv_sec  = SysTime.wSecond; 
     ru->ru_stime.tv_usec = SysTime.wMilliseconds * 1000;
     (void) FileTimeToSystemTime(&UserTime, &SysTime);
     /*
      *  fprintf(stdout,
      *  "   User sec=%d, msec=%d\n", SysTime.wSecond, SysTime.wMilliseconds);
      */
     ru->ru_utime.tv_sec  = SysTime.wSecond;
     ru->ru_utime.tv_usec = SysTime.wMilliseconds * 1000;
    }

#else   /* __NTVIS__ */

  struct tms buftime;
  times(&buftime);
  /* Assumption: HZ <= 2147 (LONG_MAX/1000000) */
  /* info : in lynxOS HZ is called TICKSPERSEC (<conf.h>) */

    ru->ru_stime.tv_sec  = buftime.tms_stime / HZ;
    ru->ru_stime.tv_usec = ((buftime.tms_stime % HZ) * 1000000) / HZ;
    ru->ru_utime.tv_sec  = buftime.tms_utime / HZ;
    ru->ru_utime.tv_usec = ((buftime.tms_utime % HZ) * 1000000) / HZ;
#endif /* __NTVIS__ */
return(0);

} /* static getrusage */

#endif /* __sun__ */
#endif /* __lynx__ */
#endif /* __osf__ */
#endif /* SYSV */
/*--------------------------------------------------------------------------*/
#if defined(SYSV) 
#if defined(__hpux__) || defined(_AIX) || defined(__sun__)
/* gettimeofday defined in the system lib */
#else
# if defined(__osf__) ||defined(__lynx__)
/* gettimeofday defined in the system lib */
#else
/*ARGSUSED*/
static 
gettimeofday(tp, zp)
struct timeval *tp;
struct timezone *zp;
{
#if defined(__NTVIS__)
  struct _timeb timeptr;

  _ftime(&timeptr);
  tp->tv_sec = timeptr.time;
  tp->tv_usec = timeptr.millitm * 1000;
#else /* all cases */
  tp->tv_sec = time(0);
  tp->tv_usec = 0;
#endif /* __NTVIS__ */
return(1);
} /* static gettimeofday */

#endif /*__osf__ || __lynx__ */
#endif /* __hpux__ || _AIX || __sun__ || __osf__*/
#endif /* SYSV */
/*--------------------------------------------------------------------------*/
/*
 *                        P R E P _ T I M E R
 */
void
prep_timer()
{
  gettimeofday(&time0, (struct timezone *)0);
  getrusage(RUSAGE_SELF, &ru0);
}
/*--------------------------------------------------------------------------*/
/*
 *                        R E A D _ T I M E R
 * 
 */
void
read_timer(str,len)
char *str;
int len;
{
  struct timeval time1;
  struct rusage ru1;
  char line[MAX_STRING];

  getrusage(RUSAGE_SELF, &ru1);
  gettimeofday(&time1, (struct timezone *)0);

  prusage(&ru0, &ru1, &time1, &time0, line);

  strncpy( str, line, strlen(line)+1 );  

  /* Get real time  already calculated in the function prusage  */
  /* 
   * tvsub( &td, &time1, &time0 );
   * realt = td.tv_sec + ((double)td.tv_usec) / 1000000.0;
   */
  /* Get CPU time (user+sys) already calculated in the function prusage */
  /*
   * tvadd( &tend, &ru1.ru_utime, &ru1.ru_stime );
   * tvadd( &tstart, &ru0.ru_utime, &ru0.ru_stime );
   * tvsub( &td, &tend, &tstart );
   * cput = (double)td.tv_sec + ((double)td.tv_usec) / 1000000.0;
   */
}
/*--------------------------------------------------------------------------*/
void
prusage(r0, r1, t1, t0, outp)

/*         Print the process usage calculated from timers values extracted 
 *         before and after the transfer execution.
 */


register struct rusage *r0, *r1;
struct timeval *t1, *t0;
char *outp;
{ struct timeval tdiff,tend,tstart;
  register time_t t;     /* time_t /usr/include/sys.types.h */
  register char *cp;
  register time_t ms;    
  int i;

  /* t == total user delta time + total system delta time */
if (debug)   
  { 
    printf("timers     : end          startup\n");
    printf("user (sec) : %9ld  %9ld\n",r1->ru_utime.tv_sec, 
           r0->ru_utime.tv_sec);
    printf("user (usec): %9ld  %9ld\n",r1->ru_utime.tv_usec, 
           r0->ru_utime.tv_usec);
    printf("sys  (sec) : %9ld  %9ld\n",r1->ru_stime.tv_sec, 
           r0->ru_stime.tv_sec);
    printf("sys  (usec): %9ld  %9ld\n",r1->ru_stime.tv_usec, 
           r0->ru_stime.tv_usec);
    printf("time (sec) : %9ld  %9ld\n",t1->tv_sec,t0->tv_sec);
    printf("time (usec): %9ld  %9ld\n",t1->tv_usec,t0->tv_usec);
  }
/* for the AIX debug, most counters are outside a good range  
   printf("                               r0                   r1\n");
   printf("ru_ixrss    %20ld %20ld \n", r0->ru_ixrss  ,r1->ru_ixrss  );    
   printf("ru_idrss    %20ld %20ld \n", r0->ru_idrss  ,r1->ru_idrss  );    
   printf("ru_isrss    %20ld %20ld \n", r0->ru_isrss ,r1->ru_isrss );  
   printf("ru_minflt   %20ld %20ld \n", r0->ru_minflt  ,r1->ru_minflt  );   
   printf("ru_majflt   %20ld %20ld \n", r0->ru_majflt  ,r1->ru_majflt  );   
   printf("ru_nswap    %20ld %20ld \n", r0->ru_nswap  ,r1->ru_nswap  );    
   printf("ru_inblock  %20ld %20ld \n", r0->ru_inblock  ,r1->ru_inblock );  
   printf("ru_oublock  %20ld %20ld \n", r0->ru_oublock  ,r1->ru_oublock  );  
   printf("ru_msgsnd   %20ld %20ld \n", r0->ru_msgsnd  ,r1->ru_msgsnd  );   
   printf("ru_msgrcv   %20ld %20ld \n", r0->ru_msgrcv  ,r1->ru_msgrcv   );   
   printf("ru_nsignals %20ld %20ld \n", r0->ru_nsignals  ,r1->ru_nsignals); 
   printf("ru_nvcsw    %20ld %20ld \n", r0->ru_nvcsw   ,r1->ru_nvcsw   );    
   printf("ru_nivcsw   %20ld %20ld \n", r0->ru_nivcsw  ,r1->ru_nivcsw  );   
   */



  /* t == total user delta time + total system delta time */
  tvadd( &tend, &r1->ru_utime, &r1->ru_stime );   /* user + sys time  @ end */
  tvadd( &tstart, &r0->ru_utime, &r0->ru_stime ); /* user + sys time  @ start*/
  tvsub( &tdiff, &tend, &tstart );
  
  t = tdiff.tv_sec*(time_t)1000 + tdiff.tv_usec/(time_t)1000;     /* in mseconds */

  cput = ((double)tdiff.tv_sec)*1000000.0 + (double)tdiff.tv_usec; /* in useconds */                          /* in useconds */

  /* ms == value of the internal clock at the end of the xfer   */      
  /*                             also called real time.         */
  tvsub(&tdiff,t1 ,t0);

  ms = tdiff.tv_sec*(time_t)1000 + tdiff.tv_usec/(time_t)1000;    /* in mseconds */

  realt = ((double)tdiff.tv_sec)*1000000.0 + (double)tdiff.tv_usec; /* in useconds */

#define END(x)        {while(*x) x++;}

 /* The display is based on variables provided by the function getrusage 
    Info located in : /usr/include/sys/resource.h  
  */                       
#if defined(SYSV)

#if defined(_AIX)
 /* with AIX cernsp most counters are wrong  
  * cp = "%Uuser %Ssys %Ereal %P %Xi+%Dd %Mmaxrss %F+%Rpf %Ccsw\0";
  */
  cp = "%Uuser %Ssys %Ereal %P\0";

#else 
#if defined(__osf__)
  cp = "%Uuser %Ssys %Ereal %P %Xi+%Dd %Mmaxrss %F+%Rpf %Ccsw\0";
#else
#if defined(sgi)                    /* IRIX 3.3 will show 0 for %M,%F,%R,%C */
  cp = "%Uuser %Ssys %Ereal %P %Mmaxrss %F+%Rpf %Ccsw\0";
#else 
#if defined(__Lynx__)
  cp = "%Uuser %Ssys %Ereal %P\0";
#else 
  cp = "%Uuser %Ssys %Ereal %P\0";    /* all SYSV except those mentionned */
#endif /*__lynx__ */
#endif /* sgi     */
#endif /*__osf__  */
#endif /* _AIX    */

#else  /* BSD system */
cp = "%Uuser %Ssys %Ereal %P %Xi+%Dd %Mmaxrss %F+%Rpf %Ccsw\0"; 

#endif /* SYSV    */

  for (; *cp; cp++)  
   {
    if (*cp != '%')  *outp++ = *cp;
    else if (cp[1]) 
         switch(*++cp) 
           {
            case 'U':
              tvsub(&tdiff, &r1->ru_utime, &r0->ru_utime);
#if defined(_AIX)
              sprintf(outp,"%ld.%06ld", (long)tdiff.tv_sec, (long)tdiff.tv_usec); 
              if (log_cnt) fprintf(fplog," %ld.%06ld"
                            (long)tdiff.tv_sec, (long)tdiff.tv_usec); 
#else
              sprintf(outp,"%ld.%06ld", tdiff.tv_sec, tdiff.tv_usec);
              if (log_cnt)fprintf(fplog," %ld.%06ld",tdiff.tv_sec, tdiff.tv_usec);
#endif
              END(outp);
              break;

            case 'S':
              tvsub(&tdiff, &r1->ru_stime, &r0->ru_stime);
#if defined(_AIX)
              sprintf(outp,"%ld.%06ld", 
               (long)tdiff.tv_sec, (long)tdiff.tv_usec); 
           if (log_cnt)        fprintf(fplog," %ld.%06ld"
                            (long)tdiff.tv_sec, (long)tdiff.tv_usec);
#else
              sprintf(outp,"%ld.%06ld", tdiff.tv_sec, tdiff.tv_usec);
              if (log_cnt)
                  fprintf(fplog," %ld.%06ld",tdiff.tv_sec, tdiff.tv_usec);
#endif
              END(outp);
              break;

            case 'E':
              psecs( ms / (time_t)1000.0 , outp); END(outp);
              break;

            case 'P':
                  sprintf(outp,"%.1f%%", (cput*100.0 / (realt ? realt : 1.0)) );
                  END(outp);
                  if (log_cnt)
                       fprintf(fplog," %.1f", (cput*100.0 / (realt ? realt : 1.0)) );
                  break;

#if !defined(SYSV) || defined(__osf__) || defined(_AIX)
            case 'W':
              i = r1->ru_nswap - r0->ru_nswap;
              sprintf(outp,"%d", i); END(outp);
              break;

            case 'X':
              sprintf(outp,"%ld", t == 0 ? 0 : (r1->ru_ixrss-r0->ru_ixrss)/t);
              END(outp);
              break;

            case 'D':
              sprintf(outp,"%ld", t == 0 ? 0 :
                  (r1->ru_idrss+r1->ru_isrss - (r0->ru_idrss+r0->ru_isrss) )/t);
              END(outp);
              break;

            case 'K':
              sprintf(outp,"%ld", t == 0 ? 0 :
                      ((r1->ru_ixrss+r1->ru_isrss+r1->ru_idrss) -
                       (r0->ru_ixrss+r0->ru_idrss+r0->ru_isrss))/t);
              END(outp);
              break;

            case 'M':
              sprintf(outp,"%ld", r1->ru_maxrss/2); END(outp);
              break;
  
            case 'F':
              sprintf(outp,"%ld", r1->ru_majflt-r0->ru_majflt);  END(outp);
              break;

            case 'R':
              sprintf(outp,"%ld", r1->ru_minflt-r0->ru_minflt);  END(outp);
              break;

            case 'I':
              sprintf(outp,"%ld", r1->ru_inblock-r0->ru_inblock);  END(outp);
              break;

            case 'O':
              sprintf(outp,"%ld", r1->ru_oublock-r0->ru_oublock); END(outp);
              break;
            case 'C':
              sprintf(outp,"%ld+%ld", r1->ru_nvcsw-r0->ru_nvcsw,
                      r1->ru_nivcsw-r0->ru_nivcsw ); END(outp);
              break;
#endif /* !SYSV || __osf__ */
           } /* switch */
   } /* for */
  END(outp);
  *outp = '\0';
}
/*--------------------------------------------------------------------------*/
/* add 2 times structure and move usec bigger than 1000000 to sec */
void              
tvadd(tsum, t0, t1)
     struct timeval *tsum, *t0, *t1;
{
  tsum->tv_sec = t0->tv_sec + t1->tv_sec;
  tsum->tv_usec = t0->tv_usec + t1->tv_usec;
  if (tsum->tv_usec > 1000000) tsum->tv_sec++, tsum->tv_usec -= 1000000;
}
/*--------------------------------------------------------------------------*/
/* substract 2 time structure (t1 > t0) */
void             
tvsub(tdiff, t1, t0)
     struct timeval *tdiff, *t1, *t0;
{ 
  tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
  tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
  if (tdiff->tv_usec < 0)  tdiff->tv_sec--, tdiff->tv_usec += 1000000;
}
/*--------------------------------------------------------------------------*/
void          
psecs(l,cp)
time_t l;               
register char *cp;
{
  register int i;
   
  i = (int)l / 3600;
  if (i) 
    {sprintf(cp,"%d:", i);
     END(cp);
     i = l % 3600;
     sprintf(cp,"%d%d", (i/60) / 10, (i/60) % 10);
     END(cp);
    } 
  else 
    {i = l;
     sprintf(cp,"%d", i / 60);
     END(cp);
    }
  i %= 60;
  *cp++ = ':';
  sprintf(cp,"%d%d", i / 10, i % 10);
}
/*--------------------------------------------------------------------------*/
/*                        N R E A D                                           */

int                          
Nread( s, bufp, count )
int s;
void *bufp;
int count;
{
#if defined(__lynx__) || defined(__svr4__) || defined(_AIX) || defined(__NTVIS__) || \
	defined(__FreeBSD__)
  struct sockaddr from;
#else
  struct sockaddr_in from;
#endif
  int len = sizeof(from);
  register int cnt;
  if( udp )  
    {cnt = recvfrom( s, bufp, count, 0, &from, &len );
    numCalls++;
    } 
  else 
    {if( b_flag )  cnt = mread( s, bufp, count );        /* fill bufp */
    else 
      {
#if defined(__NTVIS__)
        cnt = recv( s, bufp, count, 0 );
#else
        cnt = read( s, bufp, count );
#endif /* __NTVIS__ */
        numCalls++;
      }
    if (touchdata && cnt > 0) 
      {register int c = cnt, sum = 0;
       register char *b = bufp;
       while (c--) sum += *b++;
      }
    }
  /* rchrch   printf (" numcall %d read buffer %d bytes \n",numCalls,cnt); */
  return(cnt);
}
/*--------------------------------------------------------------------------*/
/*                        N W R I T E                                         */

int                                    
Nwrite( s, bufp, count )
int s;
void *bufp;
int count;
{
  register int cnt;
  if( udp )  
    {
    again:
#if defined(__lynx__) || defined(__svr4__)|| defined(_AIX) || defined(__NTVIS__) || \
	defined(__FreeBSD__)
      cnt = sendto( s, bufp, count, 0, (struct sockaddr *)&sinhim,
                                                sizeof(sinhim) );
#else
      cnt = sendto( s, bufp, count, 0, &sinhim, sizeof(sinhim) );
#endif /* __lynx__ */

      numCalls++;

#if defined(__NTVIS__)
      if( cnt<0 && WSAENOBUFS == WSAGetLastError()) 
#else
      if( cnt<0 && errno == ENOBUFS )  
#endif /* __NTVIS__ */

        { delay(18000); errno = 0; goto again; }
    }  else /* if (udp) */
    {

#if defined(__NTVIS__)
      cnt = send( s, bufp, count, 0 );
      numCalls++;
#else
      cnt = write( s, bufp, count );
      numCalls++;
#endif /* __NTVIS__ */
    }        
  return(cnt);
}
/*--------------------------------------------------------------------------*/
void
delay(us)
int us;        
{
  struct timeval tv;

  tv.tv_sec = 0;
  tv.tv_usec = (time_t)us;

#if defined(__hpux__)
  select( 1, 0, 0, 0, &tv ); 
#else
  select( 1, (struct fd_set *)0, (struct fd_set *)0,
             (struct fd_set *)0, &tv ); 
#endif
}
/*--------------------------------------------------------------------------*/
/*                        M R E A D        
 *
 * This function performs the function of a read(II) but will
 * call read(II) multiple times in order to get the requested
 * number of characters.  This can be necessary because
 * network connections don't deliver data with the same
 * grouping as it is written with.  Written by Robert S. Miles, BRL.
 */
int
mread(s, bufp, n)
int s;
register char        *bufp;
unsigned        n;
{
  register unsigned        count = 0;
  register int                nread;
  
  do
    {
#if defined(__NTVIS__)
     nread = recv(s, bufp, n-count, 0);
#else
     nread = read(s, bufp, n-count);
#endif /* __NTVIS__ */
     numCalls++;
     if(nread < 0)  {perror("ttcp_mread"); return(-1); }
     if(nread == 0)  return((int)count);
     count += (unsigned)nread;
     bufp += nread;
    }while(count < n);
  return((int)count);
}

/*--------------------------------------------------------------------------*/
void
open_log()
{static long sysTicks;
 sprintf(logfile,"%s_%s",logfile_head,trans?"t":"r");

 fprintf(stdout,"open the log file >%s<\n",logfile);
 if ((fplog = fopen(logfile,"r")) == NULL)    
       
   {if ((fplog = fopen(logfile,"a+")) == NULL)
     {fprintf(stdout,"Failure : creation of the file >%s< \n",logfile );
      exit(1);
     }
    else 
     {fprintf(fplog," creation date : ");
     /* get date */
     time(&sysTicks);
     tms = localtime(&sysTicks);
     fprintf(fplog,"%02d-%02d-%02d %02d:%02d\n",
            tms->tm_mday, tms->tm_mon, tms->tm_year,tms->tm_hour, tms->tm_min);
   
     /* An other version will produce : Mon Aug  4 16:32:16 1997
      * long  lDxcomsTicks;  char *pDateTime; 
      * time(&lDxcomsTicks);
      * pDateTime = ctime(&lDxcomsTicks);  *(pDateTime+24) = '\0';
      * fprintf(fplog," ttcp called : %s", pDateTime);
      */
     fprintf(fplog,"format\n");
    fprintf(fplog,",buflen, nbuf(byte), bufalign(byte), bufoffset(byte)");
    fprintf(fplog,",  port, sockbufsize(byte), UserTime(sec), SysTime(sec)\n");
    fprintf(fplog,",  CPUusage(%), Validity, nbytes(byte), realt(sec)");
        fprintf(fplog,",  rate(MB/sec), I/O call, hours*3600+min*60+sec\n\n");
           /* day-month-year, hour:minute\n\n");  */
     }
  } /* file already exist */
   else 
   {fclose (fplog);
     if ((fplog = fopen(logfile,"a+")) == NULL)     
       {fprintf(stdout,"Failure : access of the file >%s< \n",logfile );
        exit(1);
           }
   }
} 
/*--------------------------------------------------------------------------*/
void
close_log()
{
 static long sysTicks;
 time(&sysTicks);
 tms = localtime(&sysTicks);
     fprintf(fplog," %d\n",((tms->tm_hour)*3600 + (tms->tm_min)*60 + tms->tm_sec));
 fclose(fplog);
 fflush(fplog);
}
/*--------------------------------------------------------------------------*/
/* routine emulating UNIX fonction under NT */
#if defined(__NTVIS__)

/*---------------------------------------------------------------------------*/
static void
error(char *pch)
{
        if (!opterr) {
                return;                // without printing
        }
        fprintf(stderr, "%s: %s: %c\n",
                (NULL != progname) ? progname : "getopt", pch, optopt);
}
/*---------------------------------------------------------------------------*/
int
getopt(int argc, char **argv, char *ostr)
{
        static char *place = EMSG;        /* option letter processing */
        register char *oli;                        /* option letter list index */

        if (!*place) {
                // update scanning pointer
                if (optind >= argc || *(place = argv[optind]) != '-' || !*++place) {
                        return EOF; 
                }
                if (*place == '-') {
                        // found "--"
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
        return optopt;                        // return option letter
}
#endif /* __NTVIS__ */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void
do_help()
{
char More_help[] = "\
 Details about the reply: \n\
   Example: \n\
   ttcp-t: buflen=8192, nbuf=100, align=16384/0, port=5010\n\
   ttcp-t: File-Descriptor 0x4 Opened\n\
   # tcp sender -> <host> #\n\
   ttcp-t: 819200 bytes in 1.152557 real seconds = 694.109 KB/sec +++\n\
   ttcp-t: 100 I/O calls, 11.526 msec(real)/call, 0.213 msec(cpu)/call\n\
   ttcp-t: 0.001914user 0.019388sys 0:01real 1% 9i+58d 190maxrss 1+2pf 177+180csw\n\
   ttcp-t: buffer address 0x28000\n\
   ttcp-t: File-Descriptor  fd 0x4 Closed\n\
   ttcp done.\n\n\
cpu seconds  ==  (sec) elapse ru_utime + elapse ru_stime.\n\
                 ru_utime == The total amount of time running in user mode.\n\
                 ru_stime == The total amount of time spent in the system.\n\
                             executing on behalf of the process.\n\
real seconds ==  elapse time calculated by the system timer (date).\n\
I/O calls    ==  I/O call to the driver.\n\
msec/call    ==  average elapse time (Real seconds) between each I/O.\n\
calls/sec    ==  invert of msec/call.\n\
user         ==  (sec.msec) elaspe ru_utime.\n\
sys          ==  (sec.msec) elapse ru_stime.\n\
real         ==  (min:sec)  CPU seconds.\n\
%%           ==  Real seconds / CPU seconds.\n\
(ru_ixrss)i+(ru_idrss)d\n\
             ru_ixrss  == An integral value indicating the amount of memory \n\
                          used by the text segment that was also shared among\n\
                          other processes. This value is expressed in units of\n\
                          kilobytes * seconds-of-execution and is calculated \n\
                          by adding the number of shared memory pages in use \n\
                          each time the internal system clock ticks, and then\n\
                          averaging over one-second intervals.\n\
             ru_idrss  == An integral value of the amount of unshared memory \n\
                          in the data segment of a process (expressed in \n\
                          units of kilobytes * seconds-of-execution).\n";
   char More_help1[] = "\
  (ru_maxrss/2)maxrss.\n\
             ru_maxrss == The maximum size, in kilobytes, of the used\n\
                          resident set size. \n\
  (ru_majflt)+(ru_minflt)pf : Page fault\n\
             ru_majflt == The number of page faults serviced that required\n\
                          I/O activity.\n\
             ru_minflt == The number of page faults serviced without any\n\
                          I/O activity. In this case, I/O activity is \n\
                          avoided by reclaiming a page frame from the list \n\
                          of pages awaiting reallocation. \n\
(ru_nvcsw)+(ru_nivcsw)csw : context switch\n\
             ru_nvcsw  == The number of times a context switch resulted \n\
                          because a process voluntarily gave up the \n\
                          processor before its time slice was completed. \n\
                          This usually occurs while the process waits \n\
                          for availability of a resource.\n\
             ru_nivcsw == The number of times a context switch resulted \n\
                          because a higher priority process ran or because\n\
                          the current process exceeded its time slice.\n\n\
";

char More_help2[] = "\
log file format :\n\
         buflen, nbuf(byte), bufalign(byte), bufoffset(byte)\n\
         port, sockbufsize(byte), UserTime(sec), SysTime(sec), CPUusage(%)\n\
         nbytes(byte), realt(sec), rate(MB/sec), I/O call,\n\
                 hours*3600+min*60+sec\n\n\
";
 fprintf(stderr,More_help);
 fprintf(stderr,More_help1);
 fprintf(stderr,More_help2);
}
/*---------------------------------------------------------------------------*/
void
do_Usage()
{
char Usage[] = "\
  Usage: ttcp -t [-options] host [ < in ]    ttcp -r [-options > out]\n\
Example: ttcp -t -s -v -n100 host            ttcp -r -s -v -n100 \n\
Common options:\n\
    -V      prints version number and date of last modification\n\
    -L      create and append all results to a file named ttcp_log \n\
    -h      more help\n\
    -l ##   length of bufs read from or written to network (default 8192)\n\
    -u      use UDP instead of TCP\n\
    -p ##   port number to send to or listen at (default 5001)\n\
    -s      (ttcp -t) : source a pattern to network\n\
            (ttcp -r) : sink (discard) all data from network\n\
    -A ##   align the start of buffers to this modulus (default 16384)\n\
    -O ##   start buffers at this offset from the modulus (default 0)\n\
    -v      verbose: print more statistics\n\
    -d      set SO_DEBUG socket option\n\
    -b ##   set socket buffer size (if supported)\n\
    -f X    format for rate: k,K = kilo{bit,byte}; m,M = mega; g,G = giga\n\
Options specific to (ttcp -t) :\n\
    -n ##   number of source bufs written to network (default 2048)\n\
    -D      don't buffer TCP writes (sets TCP_NODELAY socket option)\n\
Options specific to (ttcp -r) :\n\
    -B      for -s, only output full blocks as specified by -l (for TAR)\n\
    -T      \"touch\": access each byte as it's read\n\
";        
 fprintf(stderr,Usage);
}
