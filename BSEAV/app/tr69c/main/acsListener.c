/******************************************************************************
 *	  (c)2010-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * $brcm_Log: $
 * 
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>

#include "../inc/appdefs.h"
#include "../SOAPParser/CPEframework.h"
#include "../SOAPParser/RPCState.h"
#include "event.h"
#include "types.h"
#include "utils.h"

#include "../webproto/protocol.h"
#include "../webproto/www.h"
#include "../webproto/wget.h"
#include "../SOAPParser/xmlParserSM.h"
#include "../SOAPParser/xmlTables.h"
#include "../SOAPParser/RPCState.h"
#include "../bcmLibIF/bcmWrapper.h"

#include "informer.h"

#define DEBUG

#include "../inc/tr69cdefs.h" /* defines for ACS state */
extern ACSState    acsState;
extern unsigned int addInformEventToList(int event);
extern void sendInform(void* dummy);


extern eInformState informState;

static void startACScallback(void *handle);

typedef enum {
    sIdle,
    sAuthenticating,
	sAuthenticated,
    sAuthFailed,
    sShutdown
} eACSConnState;

typedef struct ACSConnection {
    eACSConnState cState; /* authentication state */
    tProtoCtx   *cpc;   /* so we can use wget proto functions */
    int         lfd;    /* listener socket */
    tHttpHdrs   hdrs;
} ACSConnection;

static ACSConnection  connection;
static SessionAuth acsConnSess;

WanState	wanState;

void handleConnectionRequest(void);

static void free_http_headers(tHttpHdrs *hdrs) {
    CookieHdr	*cp, *last;
    free(hdrs->content_type);
    free(hdrs->protocol);
    free(hdrs->wwwAuthenticate);
    free(hdrs->Authorization);
    free(hdrs->TransferEncoding);
    free(hdrs->Connection);
    free(hdrs->method);
    free(hdrs->path);
    free(hdrs->host);
    cp = hdrs->setCookies;
    while (cp) {
        last = cp->next;
        free(cp->name);
        free(cp->value);
        free(cp);
        cp = last;
    }
    free(hdrs->message);
    free(hdrs->locationHdr);
    free(hdrs->filename);
    free(hdrs->arg);
    /* do not free(hdrs) since it's needed for Connection Request*/
    memset(hdrs, 0, sizeof(tHttpHdrs));
}

void initACSConnectionURL(void)
{
    char buf[50];

	getWanInfo( &wanState );
    snprintf(buf, sizeof(buf), "http://%s:%d%s", writeIp(wanState.ip),ACSCONN_PORT,ACSCONNPATH);

    free(acsState.connReqURL);
    acsState.connReqURL =  strdup(buf);

    free(acsState.connReqPath);
    acsState.connReqPath = strdup(ACSCONNPATH);
}

static int testChallenge( ACSConnection *cd)
{
    return ( parseAuthorizationHdr(cd->hdrs.Authorization, &acsConnSess,
                                    acsState.connReqUser,acsState.connReqPwd));
}

void statusWriteComplete(void *handle)
{
    ACSConnection *cd = (ACSConnection*)handle;
    free_http_headers(&cd->hdrs);
    proto_FreeCtx(cd->cpc);
    memset(cd, 0, sizeof(struct ACSConnection));
    setTimer(startACScallback, NULL, 2000);   /* may want smarter time value here ????*/
}

static int sendOK( ACSConnection *cd)
{
    char    response[300];
    int     i;

    i = snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\n");
    i+= snprintf(response+i, sizeof(response)-i, "Content-Length: 0\r\n\r\n");
    if ( proto_Writen(cd->cpc, response, i) < i)
        return 0;
    return 1;
}

static int sendAuthFailed( ACSConnection *cd )
{
    char    response[300];
    int     i;

    i = snprintf(response, sizeof(response), "HTTP/1.1 401 Unauthorized\r\n");
    i+= snprintf(response+i, sizeof(response)-i, "Content-Length: 0\r\n\r\n");
    if ( proto_Writen(cd->cpc, response, i) < i)
        return 0;
    return 1;
}

static int sendChallenge( ACSConnection *cd )
{
    char    response[300];
    char    *h;
    int     i;

    i = snprintf(response, sizeof(response), "HTTP/1.1 401 Unauthorized\r\n");
    i+= snprintf(response+i, sizeof(response)-i, "Content-Length: 0\r\n");
    h = generateWWWAuthenticateHdr( &acsConnSess, ACSREALM, ACSDOMAIN, cd->hdrs.method);
    i+= snprintf(response+i, sizeof(response)-i,"%s\r\n\r\n", h);
    free(h);
    if ( proto_Writen(cd->cpc, response, i) < i)
        return 0;
    return 1;
}
/**
 * A connected ACS is sending us data,
 * Our action is to generate a digest authentication challange
 * with a 401 Unauthorized status code and
 * wait for the response to the challange. Then  send a
 * 200 OK or a 401 Unauthorized. */
static void acsReadData(void *handle)
{
    ACSConnection *cd = (ACSConnection *)handle;

#ifdef DEBUG
    fprintf(stderr, "acsReadData\n");
	fprintf(stderr, "connection state %x\n", cd->cState);
#endif
    /* Free resources allocated earlier */
    free_http_headers(&cd->hdrs);
    if ( proto_ParseRequest(cd->cpc, &cd->hdrs) == 0 )
    {
        proto_ParseHdrs(cd->cpc, &cd->hdrs);
#ifdef DEBUG
		fprintf(stderr, "protocol = %s\n", cd->hdrs.protocol);
#endif
        if (!strcasecmp("http/1.1", cd->hdrs.protocol))
        {
            /* protocol is correct */
#ifdef DEBUG
			fprintf(stderr, "acsState.connReqPath = %s\n", acsState.connReqPath);
			fprintf(stderr, "hdr path = %s\n", cd->hdrs.path);
#endif
            if (!strcmp(acsState.connReqPath,cd->hdrs.path))
            {
#ifdef DEBUG
				fprintf(stderr, "connReqPath is correct\n");
				fprintf(stderr, "connReqUser %s\n", acsState.connReqUser);
#endif
                /* path is correct proceed with authorization */
				if (acsState.connReqUser==NULL || acsState.connReqUser[0]=='\0')
					cd->cState = sAuthenticated;
                if (cd->cState == sIdle)
                {
#ifdef DEBUG
					fprintf(stderr, "send 401 with digest challange\n");
#endif
                    /* send 401 with digest challange */
                    sendChallenge( cd );
                    cd->cState = sAuthenticating;
                    setListener(cd->cpc->fd, acsReadData, (void *)cd);
                    return;
                }
                else if (cd->cState == sAuthenticating)
                {
                    if ( testChallenge( cd ) )
                    {
                        sendOK(cd);
#ifdef DEBUG
						fprintf(stderr, "send OK\n");
#endif
                        /* notifyCallbacks(&informState); */  /* this will nudge the inform code */
                        /* avoid race condition between periodic inform and connection request inform */
                        resetPeriodicInform(acsState.informInterval);
                        handleConnectionRequest();

                        cd->cState = sShutdown;
                    }
                    else
                    {
						slog(LOG_DEBUG, "ConnectRequest authentication error");
                        sendAuthFailed(cd);
                        cd->cState = sShutdown;
                    }
                    setListenerType(cd->cpc->fd, statusWriteComplete,cd, iListener_Write);
                    return;
                }
                else if (cd->cState == sAuthenticated)
                {
					sendOK(cd);
                    /* notifyCallbacks(&informState); */  /* this will nudge the inform code */
                    /* avoid race condition between periodic inform and connection request inform */
                    resetPeriodicInform(acsState.informInterval);
                    handleConnectionRequest();
					cd->cState = sShutdown;
					setListenerType(cd->cpc->fd, statusWriteComplete,cd, iListener_Write);
					return;
				}
            }
        }
    } else {
        #ifdef DEBUG
        fprintf(stderr, "acsListener Error reading response\n");
        #endif
    }
    cd->cState = sShutdown;
    setListenerType(cd->cpc->fd, statusWriteComplete, cd, iListener_Write);
}


/**
 * A client is trying to connect to us.
 */
static void acsConnect(void *handle)
{
    int res;
    struct sockaddr_in addr;
    ACSConnection *cd = (ACSConnection *) handle;
    socklen_t sz = sizeof(struct sockaddr_in);
    int     flags=1;
    int     fd;

    stopListener(cd->lfd);
    memset(&addr, 0, sz);
    if (( fd = accept(cd->lfd, (struct sockaddr *)&addr, &sz)) < 0)
    {
        slog(LOG_ERR, "acsListen accept failed errno=%d.%s",errno, strerror(errno));
        close(cd->lfd);
        setTimer(startACScallback, NULL, 5000 ); /* reenable listen in 5 sec */
        return; /* return errno */
    }
    close(cd->lfd); /* close the listener socket - only one connection at a time */
    cd->lfd = 0;
    cd->cpc = proto_NewCtx(fd);
    if ( (res = setsockopt(cd->cpc->fd, SOL_SOCKET, SO_REUSEADDR, &flags,sizeof(flags)))<0)
        slog(LOG_ERR, "proxy_connection() setsockopt error %d %d %s", cd->cpc->fd, errno, strerror(errno));

    setListener(cd->cpc->fd, acsReadData, cd);
}
/*
* return -1: for error
*       != -1 is socket
*/
static int initSocket(unsigned int ip, int port)
{
    struct sockaddr_in addr;
    int port_sock = 0;
    int res, i = 1;
    port_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (port_sock < 0)
    {
        slog(LOG_ERR, "web: init_listen_socket(port=%d), socket failed", port);
        return -1;
    }

    res = setsockopt(port_sock, SOL_SOCKET, SO_REUSEADDR, (char*) &i, sizeof(i));
    if (res < 0)
    {
        slog(LOG_ERR,"web: %s", "Socket error listening to ACS");
        close(port_sock);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family       = AF_INET;
    addr.sin_addr.s_addr  = htonl(ip);
    addr.sin_port         = htons(port);

    res = bind(port_sock, (struct sockaddr *)&addr, sizeof(addr));
    if (res < 0)
    {
        slog(LOG_ERR, "acsListener bind failed errno=%d.%s",errno, strerror(errno));
        close(port_sock);
        return -1;
    }

    res = listen(port_sock,1);
    if (res < 0)
    {
        slog(LOG_ERR, "acsListener listent failed errno=%d.%s",errno, strerror(errno));
        close(port_sock);
        return -1;
    }
    return port_sock;
}

static void startACScallback(void *handle) {
    ACSConnection *cd = &connection;

	BSTD_UNUSED(handle);
    /* setup path for acs connection */
    initACSConnectionURL();
    if( (cd->lfd =initSocket(INADDR_ANY, ACSCONN_PORT))==-1)
        setTimer(startACScallback, NULL, 5000 ); /* retry init_socket in 5 sec */
    else {
        setListener(cd->lfd, acsConnect, cd );
    }
}


/*
* Listen for connections from the ACS
*/
void startACSListener(void)
{
    memset(&connection,0, sizeof(struct ACSConnection));
    startACScallback(NULL);

}

void handleConnectionRequest(void)
{
    /* avoid race condition between periodic inform and connection request inform */
    resetPeriodicInform(acsState.informInterval);

    addInformEventToList(INFORM_EVENT_CONNECTION_REQUEST);

    /* connection request should be handled right away.
     * When test with Cisco ACS simulator which is not always in the mode
     * to accept inform from CPE.  CPE is sitting in session retry algorithm
     * when the retry interval is getting bigger and bigger.
     * cmsTmr_set doesn't allow same routine being added.
     * So, we should cancel the current retry and execute this one right away.
     */
    stopTimer(sendInform, NULL);

    setTimer(sendInform,NULL, 0);
}

