/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
*   
*  Except as expressly set forth in the Authorized License,
*   
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*   
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
*  USE OR PERFORMANCE OF THE SOFTWARE.
*  
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
*  ANY LIMITED REMEDY.
* 
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <signal.h>
#include <ctype.h>
#include <syslog.h>

#include "../inc/appdefs.h"
#ifdef USE_SSL
    #include <openssl/ssl.h>
#endif
#include "../main/utils.h"
#include "../main/types.h"
#include "../main/event.h"

#include "protocol.h"
#include "www.h"
#include "wget.h"
#include "../bcmLibIF/bcmWrapper.h"


#define BUF_SIZE 1024
/*#define DEBUG*/

/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
#ifdef  DEBUG
    #define mkstr(S) # S
    #define setListener(A,B,C) {fprintf(stderr,mkstr(%s setListener B fd=%d\n), getticks(), A);\
setListener( A,B,C);}

    #define setListenerType(A,B,C,E) {fprintf(stderr,mkstr(%s setListenerType B-E fd=%d\n), getticks(), A);\
setListenerType( A,B,C,E);}

    #define stopListener(A) {fprintf(stderr,"%s stopListener fd=%d\n", getticks(), A);\
stopListener( A );}

static char timestr[40];
static char *getticks()
{
    struct timeval now;
    gettimeofday( &now,NULL);
    sprintf(timestr,"%04ld.%06ld", now.tv_sec%1000, now.tv_usec);
    return timestr;
}
#endif
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

/*----------------------------------------------------------------------*
 * forwards
 *----------------------------------------------------------------------*/
static void timer_connect(void *p);
static void do_connect(void *p);
#ifdef USE_SSL
static void do_connect_ssl(void *p, int errorcode);
#endif
static void timer_response(void *p);
static void do_response(void *p);
static void do_send_request(void *p, int errorcode);
static char noHostConnectMsg[] = "Could not establish connection to host %s(%s):%d";
static char noHostResolve[] = "Could not resolve host %s";
static char lastErrorMsg[255];

static void freeHdrs( XtraPostHdr **p )
{
	XtraPostHdr *next = *p;
        while( next ){
            XtraPostHdr *temp;
            temp = next->next;
            free(next->hdr);
            free(next->value);
            free(next);
            next = temp;
        }
}

static void freeCookies( CookieHdr **p )
{
	CookieHdr *next = *p;
	while( next ){
		CookieHdr *temp;
		temp = next->next;
		free(next->name);
		free(next->value);
		free(next);
		next = temp;
	}
}
/*----------------------------------------------------------------------*/
static void freeData(tWgetInternal *wg)
{
    if (wg != NULL) {
        if (wg->pc != NULL) {
            proto_FreeCtx(wg->pc);
            wg->pc = NULL;
        }
        free(wg->proto);
        free(wg->host);
        free(wg->uri);
        if (wg->hdrs != NULL)
            proto_FreeHttpHdrs(wg->hdrs);
        freeCookies( &wg->cookieHdrs );
        freeHdrs( &wg->xtraPostHdrs );
        /* don't free content_type and postdata since they're only pointers, there are no strdup for them.*/
        free(wg);
    }
}

typedef enum {
	REPLACE,
	ADDTOLIST
} eHdrOp;

static int addCookieHdr( CookieHdr **hdrQp, char *cookieName, char *value, eHdrOp replaceDups)
{
    CookieHdr *xh;
    CookieHdr **p = hdrQp;
    xh= (CookieHdr *)malloc( sizeof(struct CookieHdr));
    memset(xh, 0, sizeof(struct CookieHdr));
    if (xh) {
        xh->name = strdup(cookieName);
        xh->value = strdup(value);
		if (replaceDups == REPLACE) {
			while (*p) {
				CookieHdr *xp = *p;
				if ( strcasecmp(xp->name, xh->name)==0) {
					/* replace header */
					xh->next = xp->next;
					free(xp->name);
					free(xp->value);
					free(xp);
					*p = xh;
					return 1;
				}
				p = &xp->next;
			}
		}
		/* just stick it at beginning of list */
        xh->next = *hdrQp;
        *hdrQp = xh;
        return 1;
    }
    return 0;
}

static int addPostHdr( XtraPostHdr **hdrQp, char *xhdrname, char *value, eHdrOp replaceDups)
{
    XtraPostHdr *xh;
    XtraPostHdr **p = hdrQp;
    xh= (XtraPostHdr *)malloc( sizeof(struct XtraPostHdr));
    memset(xh, 0, sizeof(struct XtraPostHdr));
    if (xh) {
        xh->hdr = strdup(xhdrname);
        xh->value = strdup(value);
		if (replaceDups == REPLACE) {
			while (*p) {
				XtraPostHdr *xp = *p;
				if ( strcmp(xp->hdr, xh->hdr)==0) {
					/* replace header */
					xh->next = xp->next;
					free(xp->hdr);
					free(xp->value);
					free(xp);
					*p = xh;
					return 1;
				}
				p = &xp->next;
			}
		}
		/* just stick it at beginning of list */
        xh->next = *hdrQp;
        *hdrQp = xh;
        return 1;
    }
    return 0;
}
/*----------------------------------------------------------------------*/
static void report_status(tWgetInternal *data, tWgetStatus status,
                          const char *msg)
{
    tWget wg;
    /* internal error, call callback */
    wg.status = status;
    wg.pc = data->pc;
    wg.hdrs = data->hdrs;
    wg.msg = msg;
    wg.handle = data->handle;
    data->cbActive = 1;

	/* printf("\n!!!report_status: before callback function\n\n"); */

    (*data->cb)(&wg);
    data->cbActive = 0;
	/* printf("\n!!!report_status: <%s>\n\n", msg); */

    if (data->keepConnection==eCloseConnection)
	{
		/* printf("\n!!!report_status: close connection\n\n"); */
        freeData(data);
	}
	else
	{
		/* printf("\n!!!report_status: keep connection\n\n"); */
	}
}

/*----------------------------------------------------------------------*
 * returns
 *   0 if ok
 *  -1 if WAN interface is not active
 */
static int send_get_request(tWgetInternal *p, const char *host, int port, const char *uri)
{
    tProtoCtx   *pc = p->pc;
    XtraPostHdr *next;

    proto_SendRequest(pc, "GET", uri);
    proto_SendHeader(pc,  "Host", host);
    proto_SendHeader(pc,  "User-Agent", USER_AGENT_NAME);
    if (p->keepConnection==eCloseConnection)
        proto_SendHeader(pc,  "Connection", "close");
    else
        proto_SendHeader(pc,  "Connection", "keep-alive");

    next = p->xtraPostHdrs;
    while (next) {
        proto_SendHeader(pc,next->hdr, next->value);
        next = next->next;
    }
    proto_SendEndHeaders(pc);

    return 0;
}

/*----------------------------------------------------------------------*
 * returns
 *   0 if ok
 *  -1 if WAN interface is not active
 *  arg_keys is a NULL terminated array of (char *)
 *  arg_values is a NULL terminated array of (char *) of same length as arg_keys
 */
static int send_post_request(tWgetInternal *p, const char *host, int port, const char *uri,
                              const char *data, int datalen, const char *content_type)
{
    tProtoCtx   *pc = p->pc;
    char buf[BUF_SIZE];
    XtraPostHdr *next;
    CookieHdr	*cookie;

    proto_SendRequest(pc, "POST", uri);
    sprintf(buf, "%s:%d", host, port);
    proto_SendHeader(pc,  "Host", buf);
    proto_SendHeader(pc,  "User-Agent", USER_AGENT_NAME);
    if (p->keepConnection==eCloseConnection)
        proto_SendHeader(pc,  "Connection", "close");
    else
        proto_SendHeader(pc,  "Connection", "keep-alive");
    next = p->xtraPostHdrs;
    while (next) {
        proto_SendHeader(pc,next->hdr, next->value);
        next = next->next;
    }
    cookie = p->cookieHdrs;
    while(cookie) {
        proto_SendCookie(pc, cookie);
        cookie = cookie->next;
    }
    proto_SendHeader(pc,  "Content-Type", content_type);
    sprintf(buf, "%d", datalen);
    proto_SendHeader(pc,  "Content-Length", buf);

    proto_SendEndHeaders(pc);
    if (data && datalen)
        proto_SendRaw(pc, data, datalen);

    return 0;
}

/*----------------------------------------------------------------------
 * connect timeout
 */
static void timer_connect(void *p)
{
    tWgetInternal *data = (tWgetInternal *) p;
    char buf[256];

    stopListener(data->pc->fd);

    sprintf(buf, "Connection timed out to host %s:%d", data->host, data->port);
    report_status(data, iWgetStatus_ConnectionError, buf);
}

/*----------------------------------------------------------------------*/
static void timer_response(void *p)
{
    tWgetInternal *data = (tWgetInternal *) p;
    char buf[512];
    stopListener(data->pc->fd);
    sprintf(buf, "Host (%s:%d) is not responding, timeout", data->host, data->port);
    report_status(data, iWgetStatus_ConnectionError, buf);
}

/*----------------------------------------------------------------------*/
static void do_connect(void *p)
{
    tWgetInternal *data = (tWgetInternal *) p;
    int err;
    u_int n;

    stopTimer(timer_connect, data);
    stopListener(data->pc->fd);

    /* check fd status */
    n = sizeof(int);
    if (getsockopt(data->pc->fd, SOL_SOCKET, SO_ERROR, &err, &n) < 0) {
        report_status(data, iWgetStatus_InternalError,
                      "internal error: do_connect(): getsockopt failed");

        return;
    }

    if (err != 0) {
        /* connection not established */
        char buf[256];

        snprintf(buf,sizeof(buf), "Connection to host %s(%s):%d failed %d (%s)",
                data->host, writeIp(data->host_addr), data->port,
                err, strerror(err));
        report_status(data, iWgetStatus_ConnectionError, buf);
        return;
    }
#ifdef USE_SSL
    /* init ssl if proto is https */
    if ((strcmp(data->proto, "https") == 0) && (data->pc->sslConn <= 0))
    {
        proto_SetSslCtx(data->pc, do_connect_ssl, data);
        return;
    }
#endif

    /* return at this point if function is connect only */
    if(data->request==eConnect)	{
        report_status(data, iWgetStatus_Ok, NULL);
        return;
	}

#ifdef USE_SSL
    /* init ssl if proto is https */
    if (strcmp(data->proto, "https") == 0)
    {
        proto_SetSslCtx(data->pc, do_send_request, data);
    }
    else
    {
		printf("do_connect: sending request\n");
        do_send_request(data, PROTO_OK);
    }
#else
    do_send_request(data, PROTO_OK);
#endif
}

/*----------------------------------------------------------------------*/
#ifdef USE_SSL
static void do_connect_ssl(void *p, int errorcode)
{
    tWgetInternal *data = (tWgetInternal *) p;

    if (errorcode < 0)
    {
        report_status(data, iWgetStatus_ConnectionError,
                      "Failed to establish SSL connection");
    }
    else
    {
        do_connect(p);
    }
}
#endif /* USE_SSL */

/*----------------------------------------------------------------------*/
static void do_send_request(void *p, int errorcode)
{
    tWgetInternal *data = (tWgetInternal *) p;
    int res;
/*#ifdef DEBUG*/
#if 1
    fprintf(stderr, "\ndo_send_request keepConn=%d status=%d\n", data->keepConnection, data->status);
#endif
    if (errorcode < 0) {
        report_status(data, iWgetStatus_ConnectionError,
                      "Failed to establish SSL connection");
        return;
    }

    /* send request */
    if (data->request == eGetData)
    {
		printf("do_send_request: get_request\n");
        res = send_get_request(p, data->host, data->port, data->uri);
    }
    else
    {
		printf("do_send_request: post_request(data->uri = %s)\n", data->uri);
        res = send_post_request(p, data->host, data->port, data->uri, data->postdata,
                                data->datalen, data->content_type);
    }

    if (res < 0)
    {
        report_status(data, iWgetStatus_ConnectionError,
                      "Failed to send request on connection");
        return;
    }

	/* printf("do_send_request: 3\n\n"); */
    /* wait for response */
    setListener(data->pc->fd, do_response, data);
    setTimer(timer_response, data, ACSRESPONSETIME); /* response timeout is 60 sec */
    return;
}

/*----------------------------------------------------------------------*/
static void do_response(void *p)
{
    CookieHdr	*cp;
    tWgetInternal *data = (tWgetInternal *) p;
#ifdef DEBUG
    slog(LOG_DEBUG,"do_response data->pc=%p",data->pc);
#endif
    printf("do_response: data->pc=%p\n",(void*)data->pc);

    stopTimer(timer_response, data);

    if (data->pc == NULL) {
        slog(LOG_ERROR, "wget %s", "Internal Error");
		printf("do_response: wget %s\n", "Internal Error");
        report_status(data, iWgetStatus_InternalError,
                      "internal error: no filedescriptor");
        return;
    }
    if (data->pc->fd <= 0) {
        report_status(data, iWgetStatus_InternalError,
                      "internal error: no filedescriptor");
        return;
    }
    /* printf("do_response: 1\n"); */
    if (data->hdrs)
        proto_FreeHttpHdrs(data->hdrs);

    /* printf("do_response: 2\n"); */
    data->hdrs = proto_NewHttpHdrs();
    if (data->hdrs == NULL) {
        /* memory exhausted?!? */
        slog(LOG_ERROR, "wget %s", "Memory exhausted");
		printf("do_response: wget %s\n", "Memory exhausted");
    }

    /* printf("do_response: 3\n"); */
    if (proto_ParseResponse(data->pc, data->hdrs) < 0) {
        report_status(data, iWgetStatus_Error,
                      "error: illegal http response or read failure");
        return;
    }

    /* printf("do_response: 4\n"); */
    proto_ParseHdrs(data->pc, data->hdrs);
    cp = data->hdrs->setCookies;
    while (cp) { /* save new cookies if present*/
        addCookieHdr( &data->cookieHdrs,cp->name, cp->value, REPLACE );
        cp = cp->next;
    }

    /* printf("do_response: 5\n"); */
    if (data->hdrs->status_code >= 100 && data->hdrs->status_code < 600 )
    {
    	/* printf("do_response: 6\n\n"); */
        report_status(data, iWgetStatus_Ok, NULL);
    }
    else
    {
        char buf[1024];
    	/* printf("do_response: 7\n\n"); */
        sprintf(buf, "Host %s returned error \"%s\"(%d)",
                data->host, data->hdrs->message, data->hdrs->status_code);
        report_status(data, iWgetStatus_HttpError, buf);
    }
}

/*----------------------------------------------------------------------*/
static void do_resolve(void *handle)
{
    tWgetInternal *data = handle;
    int res, fd;

	stopTimer(timer_connect, data);
    /* if callback does not exist, this is a non-op */
    stopCallback(&(data->host_addr), do_resolve, data);

    if (data->host_addr == 0) {
        char buf[1024];
        sprintf(buf, noHostResolve, data->host);
        report_status(data, iWgetStatus_ConnectionError, buf);
    } else if ((res = www_EstablishConnection(data->host_addr, data->port, &fd)) < 0) {
        if (res == -1) {
            report_status(data, iWgetStatus_InternalError, "Socket creation error");
        } else {
            char buf[1024];
            sprintf(buf, noHostConnectMsg,
                    data->host, writeIp(data->host_addr), data->port);
            report_status(data, iWgetStatus_ConnectionError, buf);
        }
    } else {
        data->pc = proto_NewCtx(fd);
        setListenerType(fd, do_connect, data, iListener_Write);
    }
}

/*----------------------------------------------------------------------*/
int do_wget(const char *url, EventHandler callback, void *handle, char *postdata, int datalen, char *content_type)
{
    char proto[10];
    char host[1024];
    char uri[1024];
    int port;

#ifdef DEBUG
    slog(LOG_DEBUG, "do_wget(\"%s\", ...)", url);
#endif
    if (www_ParseUrl(url, proto, host, &port, uri) < 0) {
        return -5;
    }

    if (port == 0) {
        if (strcmp(proto, "http") == 0) {
            port = 80;
        } else if (strcmp(proto, "https") == 0) {
            port = 443;
        } else {
            slog(LOG_ERROR, "unsupported protocol in url \"%s\"", proto);
            port = 80; /* guess http and port 80 */
        }
    }

    {
        /* connect() started, lets wait until we have a connection */
        tWgetInternal *data;
        data = (tWgetInternal *) malloc(sizeof(tWgetInternal));
        memset(data, 0, sizeof(tWgetInternal));
        data->keepConnection = eKeepConnectionOpen; /* eCloseConnection; */
        data->pc = NULL;
        data->cb = callback;
        data->handle = handle;
        data->proto = strdup(proto);
        data->host = strdup(host);
        data->host_addr = 0;
        data->port = port;
        data->uri = strdup(uri);
        data->content_type = content_type;
        data->postdata = postdata;
        data->request = postdata? ePostData: eGetData;
        data->datalen = datalen;

        if (dns_lookup(data->host, &(data->host_addr))) {
            do_resolve(data);
        } else {
            setCallback(&(data->host_addr), do_resolve, data);
        }
    }

    return 0;
}

/*----------------------------------------------------------------------*
 * return
 *   0 Ok
 *  -1 Could not create socket
 *  -2 Could not connect to host
 *  -3 Could not resolve host or host syntax error
 *  -4 WAN interface is not active
 *  -5 URL syntax error
 *  -10 unknown error
 */
int wget_Get(const char *url, EventHandler callback, void *handle)
{
    return do_wget(url, callback, handle, NULL, 0, NULL);
}

/*----------------------------------------------------------------------*
 * return
 *   0  Ok
 *  -1  Could not create socket
 *  -2  Could not connect to host
 *  -3  Could not resolve host or host syntax error
 *  -4  WAN interface is not active
 *  -5  URL syntax error
 *  -10 unknown error
 */
int wget_Post(const char *url, char *arg_keys[], char *arg_values[], EventHandler callback, void *handle)
{
    char *argbuf;
    int arglen;
    int i;
    char buf[BUF_SIZE];

    argbuf = (char *) malloc(BUF_SIZE * 4);
    if (argbuf == NULL) {
        return -10;
    }
    memset(argbuf, 0, BUF_SIZE * 4);

    /* build arguments */

#if 0
    fprintf(stderr, "DEBUG: wget_Post()\n");
    cvprint(arg_keys);
    cvprint(arg_values);
#endif
    strcpy(argbuf, "");
    for (i = 0; arg_keys[i] != NULL; i++) {
        if (i > 0) {
            strcat(argbuf, "&");
        }
        www_UrlEncode(arg_values[i], buf);
        sprintf(&argbuf[strlen(argbuf)], "%s=%s", arg_keys[i], buf);
    }
    arglen = strlen(argbuf);

    return do_wget(url, callback, handle, argbuf, arglen, "application/x-www-form-urlencoded");
}

/*----------------------------------------------------------------------*
 * return
 *   0  Ok
 *  -1  Could not create socket
 *  -2  Could not connect to host
 *  -3  Could not resolve host or host syntax error
 *  -4  WAN interface is not active
 *  -5  URL syntax error
 *  -10 unknown error
 */
int wget_PostRaw(const char *url, char *content_type, char *data, int len, EventHandler callback, void *handle)
{
    return do_wget(url, callback, handle, data, len, content_type);
}

/*----------------------------------------------------------------------*
 * wget_GetFile
 *----------------------------------------------------------------------*/
typedef struct {
    EventHandler cb;
    void *handle;
    char *filename;
} tWgetFile;

/*----------------------------------------------------------------------*
 * returns
 *    0 Ok
 *   -1 File could not be open for writing
 *   -2 Error when reading from fd
 *   -3 Error when writing to file
 */
static int copy_to_file(tProtoCtx *pc, const char *filename)
{
    int ofd;
    char buf[BUF_SIZE];
    int rlen, wlen;
    mode_t mode;
    int flags, bflags;

    /* 664 */
    mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    if ((ofd = creat(filename, mode)) < 0) {
        return -1;
    }

    /* TBD: Temporary fix part 1, turn on synchroneous I/O, this call will block. */
    {
        flags = (long) fcntl(pc->fd, F_GETFL);
        bflags = flags & ~O_NONBLOCK; /* clear non-block flag, i.e. block */
        fcntl(pc->fd, F_SETFL, bflags);
    }

    while ((rlen = proto_Readn(pc, buf, BUF_SIZE)) != 0) {
        if (rlen < 0) {
            if (errno == EAGAIN) {
#ifdef DEBUG
                fprintf(stderr, "wget, data not ready, let's busy wait\n");
#endif
                continue;
            }
            /* error when reading from fd */
            return -2;
        } else if (rlen > 0) {
            /* Bytes read */
            wlen = write(ofd, buf, rlen);
            if (wlen != rlen) {
                /* error on writing */
                return -3;
            }
        }
    }

    /* TBD: Temp fix part 2. remove blocking flags */
    {
        fcntl(pc->fd, F_SETFL, flags);
    }
    /* EOF */
    close(ofd);
    return 0;
}

/*----------------------------------------------------------------------
 * argument of callback is of type (tWget *)
 */
static void save_to_file(void *p)
{
    tWgetFile *wf;
    tWget *w;
    tWget cbw;
    int res;

#ifdef DEBUG
    fprintf(stderr, "DEBUG: wget.save_to_file()...\n");
#endif
    w = (tWget *) p;
    wf = (tWgetFile *) w->handle;

    memset(&cbw, 0, sizeof(tWget));

    if (w->status != iWgetStatus_Ok) {
#ifdef DEBUG
        fprintf(stderr, "DEBUG: wget.save_to_file, status NOT ok (%d)\n", w->status);
#endif
        cbw.status = iWgetStatus_InternalError;
    } else if ((res = copy_to_file(w->pc, wf->filename)) < 0) {
#ifdef DEBUG
        fprintf(stderr, "DEBUG: wget.save_to_file, copy_to_file failed (%d), status NOT ok (%d)\n",
                res, w->status);
#endif
        cbw.status = iWgetStatus_InternalError;
    } else {
#ifdef DEBUG
        fprintf(stderr, "DEBUG: wget.save_to_file, status OK (%d)\n", w->status);
#endif
        cbw.status = iWgetStatus_Ok;
    }

    /* call callback */
    cbw.msg = w->msg;
    cbw.pc = w->pc;
    cbw.hdrs = w->hdrs;
    cbw.handle = wf->handle;
#ifdef DEBUG
    fprintf(stderr, "DEBUG: wget.save_to_file() calling callback = %p...\n", (void *) wf->cb);
#endif
    (*wf->cb)(&cbw);

    free(wf->filename);
    free(wf);
#ifdef DEBUG
    fprintf(stderr, "DEBUG: wget.save_to_file() done.\n");
#endif
}

/*----------------------------------------------------------------------*
 * returns
 *    0  Ok
 *   -1 to -5 same error codes as wget_Get
 * set status in callback to
 *    0 Ok
 *   -1 Could not open file for writing
 */
int wget_GetFile(const char *url, const char *filename, EventHandler callback, void *handle)
{
    int res;
    tWgetFile *wf;

    wf = (tWgetFile *) malloc(sizeof(tWgetFile));
    memset(wf, 0, sizeof(tWgetFile));
    wf->cb = callback;
    wf->handle = handle;
    wf->filename = strdup(filename);

    res = wget_Get(url, save_to_file, wf);
    if (res < 0) {
        free(wf->filename);
        free(wf);
        return res;
    }
    return 0;
}

/*----------------------------------------------------------------------*
 * returns
 *    0  Ok
 *   -1 to -5 same error codes as wget_Get
 * set status in callback to
 *    0 Ok
 *   -1 Could not open file for writing
 */
int wget_PostXmlFile(const char *url, char *data, int len, const char *filename, EventHandler callback, void *handle)
{
    int res;
    tWgetFile *wf;

    wf = (tWgetFile *) malloc(sizeof(tWgetFile));
    memset(wf, 0, sizeof(tWgetFile));
    wf->cb = callback;
    wf->handle = handle;
    wf->filename = strdup(filename);

    res = wget_PostRaw(url, "text/xml", data, len, save_to_file, wf);
    if (res < 0) {
        free(wf->filename);
        free(wf);
        return res;
    }
    return 0;
}
/*
* Connect to the specified url
* Returns: NULL  failed allocate memory or immediate connection error.
 * 			     Call wget_LastErrorMsg() to retrieve last error msg.
 *         pointer to Connection descriptor tWgetInternal.
*/
tWgetInternal *wget_Connect(const char *url, EventHandler callback, void *handle)
{
	tWgetInternal *wg;
	char proto[10];
	char host[1024];
	char uri[1024];
	int port;

#ifdef DEBUG
	slog(LOG_DEBUG, "wget_Connect(\"%s\", ...)", url);
	printf("wget_Connect(\"%s\", ...)", url);
#endif
	if ( (wg = (tWgetInternal*) malloc(sizeof(tWgetInternal)))==NULL)
		return NULL;
	memset(wg, 0, sizeof(tWgetInternal));
	lastErrorMsg[0] = '\0';
	wg->request = eConnect;
	wg->keepConnection = eKeepConnectionOpen;
	if (www_ParseUrl(url, proto, host, &port, uri) < 0)
	{
		wg->status = -5;
		return wg;
	}

	printf("wget_Connect: proto:%s\n", proto);
	printf("wget_Connect: host:%s\n", host);
	printf("wget_Connect: port:%d\n", port);
	printf("wget_Connect: uri:%s\n", uri);

	if (port == 0)
	{
		if (strcmp(proto, "http") == 0)
		{
			port = 80;
		}
		else if (strcmp(proto, "https") == 0)
		{
			port = 443;
		}
		else
		{
			slog(LOG_ERROR, "unsupported protocol in url \"%s\"", proto);
			port = 80; /* guess http and port 80 */
		}
	}

	wg->pc = NULL;
	wg->cb = callback;
	wg->handle = handle;
	wg->proto = strdup(proto);
	wg->host = strdup(host);
	wg->host_addr = 0;
	wg->port = port;
	if (strlen(uri))
		wg->uri = strdup(uri);
	else
		wg->uri	= strdup("/");

	if (dns_lookup(wg->host, &(wg->host_addr)))
    {
		/* immediate return requires special handling. */
		int res; int fd;
		printf("wget_Connect: after dns_lookup: %x\n",wg->host_addr);
		if (wg->host_addr == 0)
		{
			snprintf(lastErrorMsg,sizeof(lastErrorMsg), noHostResolve, wg->host);
			freeData(wg);
			wg=NULL;
            fprintf(stderr, "Connect err<%s>\n", lastErrorMsg);
        }
		else if ((res = www_EstablishConnection(wg->host_addr, wg->port, &fd)) < 0)
		{
			if (res == -1)
				strncpy(lastErrorMsg, "Socket creation error", sizeof(lastErrorMsg));
			else
				snprintf(lastErrorMsg,sizeof(lastErrorMsg), noHostConnectMsg, wg->host, writeIp(wg->host_addr), wg->port);
            fprintf(stderr, "EstablishConnection: err<%s>\n", lastErrorMsg);
			freeData(wg);
			wg=NULL;
		}
		else
		{ /* connection complete- start it */
            fprintf(stderr, "connection complete- start it\n");
			wg->pc = proto_NewCtx(fd);
			setListenerType(fd, do_connect, wg, iListener_Write);
		}
	}
#if 0
	else
	{
		setTimer(timer_connect, wg, ACSRESPONSETIME); /* response timeout is 60 sec */
		setCallback(&(wg->host_addr), do_resolve, wg);
	}
#endif

	return wg;
}

int wget_GetData(tWgetInternal *wg,EventHandler callback, void *handle)
{
    wg->request = eGetData;
    wg->handle = handle;
    wg->cb = callback;
    if (wg->hdrs) {
        wg->hdrs->status_code = 0; /* reset status_code */
        wg->hdrs->content_length = 0;
    }

#ifdef USE_SSL
    /* init ssl if proto is https */
    if ( (strcmp(wg->proto, "https") == 0) && wg->pc->ssl==NULL) {
        proto_SetSslCtx(wg->pc, do_send_request, wg);
    } else {
        do_send_request(wg, PROTO_OK);
    }
#else
    do_send_request(wg, PROTO_OK);
#endif
    return 0;
}

int wget_PostData(tWgetInternal *wg,char *postdata, int datalen, char *content_type, EventHandler callback, void *handle)
{
    wg->request = ePostData;
    wg->content_type = content_type;
    wg->postdata = postdata;
    wg->datalen = datalen;
    wg->handle = handle;
    wg->cb = callback;
    if (wg->hdrs) {
        wg->hdrs->status_code = 0; /* reset status_code */
        wg->hdrs->content_length = 0;
    }

/* printf("wget_PostData: 1\n"); */
#ifdef USE_SSL
    /* init ssl if proto is https */
    if ( (strcmp(wg->proto, "https") == 0) && wg->pc->ssl==NULL)
    {
        proto_SetSslCtx(wg->pc, do_send_request, wg);
    }
    else
    {
        do_send_request(wg, PROTO_OK);
    }
#else
/* printf("wget_PostData: 4\n"); */
    do_send_request(wg, PROTO_OK);
#endif
    return 0;
}

int wget_PostDataClose(tWgetInternal *wg,char *postdata, int datalen, char *content_type, EventHandler callback, void *handle)
{
    wg->request = ePostData;
    wg->content_type = content_type;
    wg->postdata = postdata;
    wg->datalen = datalen;
    wg->handle = handle;
    wg->cb = callback;
    if (wg->hdrs) {
        wg->hdrs->status_code = 0; /* reset status_code */
        wg->hdrs->content_length = 0;
    }
    wg->keepConnection = eCloseConnection;
#ifdef USE_SSL
    /* init ssl if proto is https */
    if ( (strcmp(wg->proto, "https") == 0) && wg->pc->ssl==NULL) {
        proto_SetSslCtx(wg->pc, do_send_request, wg);
    } else {
        do_send_request(wg, PROTO_OK);
    }
#else
    do_send_request(wg, PROTO_OK);
#endif
    return 0;
}

/*
* Disconnect maybe called from within a callback called
 * by report_status. Don't freeData() if cbActive is set.
 * Setting cCloseConnection will cause report_status
 * to free up the data on return by the callback.
*
*/

int wget_Disconnect(tWgetInternal *wio)
{
    if (wio!=NULL) {
        stopTimer(timer_response, wio); /* may be running */
        wio->request = eDisconnect;
        wio->keepConnection = eCloseConnection;
        if(!wio->cbActive)
            freeData(wio);
    }
    return 0;
}

int wget_AddPostHdr( tWgetInternal *wio, char *xhdrname, char *value)
{
    XtraPostHdr	**p = &wio->xtraPostHdrs;
    return addPostHdr(p, xhdrname,value, REPLACE );
}

void wget_ClearPostHdrs( tWgetInternal *wio)
{
    XtraPostHdr *xh = wio->xtraPostHdrs;

    while (xh) {
        XtraPostHdr *nxt;
        free( xh->hdr);
        free(xh->value);
        nxt = xh->next;
        free(xh);
        xh= nxt;
    }
    wio->xtraPostHdrs = NULL;
}

const char *wget_LastErrorMsg(void)
{
	return lastErrorMsg;
}

