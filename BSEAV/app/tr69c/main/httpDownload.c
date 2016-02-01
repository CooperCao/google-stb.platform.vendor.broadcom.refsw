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
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>

#include "syscall.h"
/*#include "ifcdefs.h" */
#include "../bcmLibIF/bcmWrapper.h"

#include "../inc/appdefs.h"
#include "../SOAPParser/CPEframework.h"
#include "../SOAPParser/RPCState.h"
#include "event.h"
#include "types.h"
#include "utils.h"

#include "../webproto/protocol.h"
#include "../webproto/www.h"
#include "../webproto/wget.h"
#include "../main/informer.h"
#include "../main/httpProto.h"
#include "../SOAPParser/xmlParserSM.h"
#include "../SOAPParser/xmlTables.h"
#include "../SOAPParser/RPCState.h"

#define DEBUG
#ifdef DEBUG
#define DBGPRINT(X) fprintf X
#else
#define DBGPRINT(X)
#endif

/* !!!JK */
#define SYS_CMD_LEN		256
#define IFC_SMALL_LEN	128

#include "../inc/tr69cdefs.h" /* defines for ACS state */
extern ACSState    acsState;

static void downLoadConnected( void *handle);
static SessionAuth getSessionAuth;
static HttpTask    httpTask;
extern RPCAction   *dloadAction;  /* not very good data hiding here */
                                    /* this is defined in RPCState.c */

void updateDownLoadKey(DownloadReq *dlr)
{
	if (acsState.downloadCommandKey)
		free(acsState.downloadCommandKey);
	acsState.downloadCommandKey = dlr->commandKey;
	dlr->commandKey = NULL;
}

static void getComplete( void *handle)
{
    tWget *wg = (tWget *)handle;
    HttpTask    *ht = (HttpTask*)wg->handle;
    SessionAuth *sa = &getSessionAuth;
    DownloadReq *dlreq = &dloadAction->ud.downloadReq;
    int	skipResult = 1;

    #ifdef DEBUG
    DBGPRINT((stderr, "httpDownload:getComplete() starting download\n"));
    #endif

    if (wg->status==iWgetStatus_Ok){
        if (wg->hdrs->status_code== 401) {
            int closeConn = ht->wio->hdrs->Connection && !strcasecmp(ht->wio->hdrs->Connection,"close");
			if (wg->hdrs->status_code== 401) {
				/* need to send authenticate */
				char *hdrvalue;
				if (wg->hdrs->content_length>0) {
					int	mlth;
					char *tmpBuf;
					if ((tmpBuf=readResponse(wg,&mlth,0)))
						free(tmpBuf);
					DBGPRINT((stderr,"proto_Skip ---\n"));
					skipResult = proto_Skip(wg->pc);
				}
				#ifdef DEBUG
				DBGPRINT((stderr, "WWW-Authenticate= %s\n", wg->hdrs->wwwAuthenticate));
				#endif
				if (!(hdrvalue = generateAuthorizationHdrValue(sa, wg->hdrs->wwwAuthenticate,
								   "GET", ht->wio->uri, dlreq->user, dlreq->pwd))) {
					slog(LOG_ERR, "WWWAuthenticate header parsing error: %s", wg->hdrs->wwwAuthenticate);
					wget_Disconnect(ht->wio);
					ht->wio = NULL;
					ht->xfrStatus = eConnectError;
					acsState.dlFaultStatus = 9012; /* download failed */
					acsState.dlFaultMsg = "Download server- unknow authentication header format";
					updateDownLoadKey(dlreq);
					return;
				}

				if (closeConn) { /* end of data on 401 skip_Proto() */
					/* close connection and reconnect with Authorization header*/
					ht->authHdr = hdrvalue;
					wget_Disconnect(ht->wio);
					ht->wio=wget_Connect(dlreq->url, downLoadConnected, ht);
					if (ht->wio==NULL){
						slog(LOG_ERR, "download reconnect failed: %s", wget_LastErrorMsg());
						free (ht->authHdr); ht->authHdr = NULL;
						ht->wio = NULL;
						ht->xfrStatus = eConnectError;
						acsState.dlFaultStatus = 9012; /* download failed */
						acsState.dlFaultMsg = "Internal error";
						updateDownLoadKey(dlreq);
					}
					return;
				} else {
					wget_AddPostHdr(ht->wio,"Authorization", hdrvalue);
					free(hdrvalue);
					/* now just resend the last data with the Authorization header */
					wget_GetData(ht->wio, getComplete, (void *)ht );
					/* now we just return to wait on the response from the server */
					return;
				}
			} else {
				/* authentication failed */
				slog(LOG_ERR, "download: Server Authenticaion Failed %d", wg->hdrs->status_code );
				wget_Disconnect(ht->wio);
				ht->wio = NULL;
				ht->xfrStatus = eConnectError;
				acsState.dlFaultStatus = 9012; /* download failed */
				acsState.dlFaultMsg = "Download server authentication failure";
				updateDownLoadKey(dlreq);
			}
        } else if (wg->hdrs->status_code==200 && ((wg->hdrs->content_length>0)
                     || (wg->hdrs->TransferEncoding && streq(wg->hdrs->TransferEncoding,"chunked"))))
        {
			/* readin download image */
			int     mlth;
			char    *rambuf = readResponse(wg, &mlth, 0 /*getRAMSize()*/);

			#ifdef DEBUG
			DBGPRINT((stderr, "Download image size = %d\n", mlth));
			#endif
			wget_Disconnect(ht->wio);
			acsState.endDLTime = time(NULL);
			ht->wio = NULL;
			ht->xfrStatus = eDownloadDone;
			if ( rambuf && mlth ){
				dlreq->fileSize = mlth;  /**/
				downloadComplete( dlreq, rambuf );
				/* no return from the downloadComplete if successful */
			} else {
				slog(LOG_ERR, "download from %s failed", dlreq->url);
				acsState.dlFaultStatus = 9010; /* download failed */
				acsState.dlFaultMsg = "Download failed";
				updateDownLoadKey(dlreq);
				if (rambuf) free(rambuf);
			}
		} else if (wg->hdrs->status_code>=100 && wg->hdrs->status_code<200) {
			return; /* ignore these status codes */
		}
		else {
			slog(LOG_ERR, "download: Error %d", wg->hdrs->status_code );
			wget_Disconnect(ht->wio);
			ht->wio = NULL;
			ht->xfrStatus = eConnectError;
			acsState.dlFaultStatus = 9013; /* download failed */
			acsState.dlFaultMsg = "Image file not found";
			updateDownLoadKey(dlreq);
		}
    } else {
		/* if control falls thru to here send a fault status to ACS */
		slog(LOG_ERR, "download: GET from download server failed, Status = %d %s", wg->status, wg->msg);
		wget_Disconnect(ht->wio);
		ht->wio = NULL;
		ht->xfrStatus = eConnectError;
		acsState.dlFaultStatus = 9010; /* download failed */
		acsState.dlFaultMsg = "Unable to connect to download URL";
		updateDownLoadKey(dlreq);
    }
    acsState.endDLTime = time(NULL);
    sendDownloadFault();    /* callback contain rpcAction pointer*/
    return;
}


static void downLoadConnected( void *handle)
{
    tWget *wg = (tWget *)handle;
    HttpTask    *ht = (HttpTask *)wg->handle;
    DownloadReq *dlreq = &dloadAction->ud.downloadReq;

    if (wg->status != 0) {
        wget_Disconnect(ht->wio);
        ht->wio = NULL;
        slog(LOG_ERR, "Download Connect Status = %d %s", wg->status, wg->msg);
        ht->xfrStatus = eConnectError;
        acsState.dlFaultStatus = 9010; /* download failed */
        acsState.dlFaultMsg = "Unable to connect to download server";
        acsState.endDLTime = time(NULL);
		updateDownLoadKey(dlreq);
        sendDownloadFault();
        return;
    }
    wget_ClearPostHdrs(ht->wio);
    if (ht->authHdr){
        wget_AddPostHdr(ht->wio,"Authorization", ht->authHdr);
        free(ht->authHdr); /* only use once*/
        ht->authHdr = NULL;
    }
    wget_GetData( ht->wio, getComplete,(void *) ht);
    acsState.fault = 0;
    return;
}

/* this is called by the callback from the startTimer in doDownload. */
/* we have to use wget_Connect in case of authentication in which case */
/* we need to control the connection */
void startDownload( void *handle)
{
    RPCAction   *a = (RPCAction *)handle;
    HttpTask    *ht = &httpTask;
    DownloadReq *dlreq = &dloadAction->ud.downloadReq;

    memset(ht, 0, sizeof(struct HttpTask));
    memset(&getSessionAuth,0, sizeof(struct SessionAuth));
	if ( strstr(a->ud.downloadReq.url, "http:")!=NULL
		 || strstr(a->ud.downloadReq.url, "https:")!=NULL) { /* simple test for http or https */
		ht->callback = (void *)a; /* save action pointer in callback cell */
		ht->wio=wget_Connect(a->ud.downloadReq.url, downLoadConnected, ht);
		if (ht->wio==NULL){
			slog(LOG_ERR, "wget_Connect to download server failed: %s", wget_LastErrorMsg());
			acsState.dlFaultStatus = 9002; /* internal error */
			acsState.dlFaultMsg = "Connection to download server failed";
			updateDownLoadKey(dlreq);
			sendDownloadFault();
		} else
			acsState.startDLTime = time(NULL);
	} else {
		acsState.dlFaultStatus = 9013;
		acsState.dlFaultMsg = "Protocol not supported";
		updateDownLoadKey(dlreq);
		sendDownloadFault();
	}
    return;
}

