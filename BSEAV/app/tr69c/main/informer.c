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
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <linux/if.h>
#include <syslog.h>
#include <fcntl.h>

#include "../inc/tr69cdefs.h"
#include "../inc/appdefs.h"
#include "../SOAPParser/RPCState.h"
#include "../SOAPParser/xmlParserSM.h"
#include "../SOAPParser/xmlTables.h"
#include "event.h"
#include "types.h"
#include "utils.h"

#include "../webproto/protocol.h"
#include "../webproto/www.h"
#include "../webproto/wget.h"
#include "../main/informer.h"
#include "../main/httpProto.h"
#include "../main/httpDownload.h"
#include "acsListener.h"
#include "../bcmLibIF/bcmWrapper.h"

#define DEBUG
#ifdef DEBUG
#define DBGPRINT(X) fprintf X
#else
#define DBGPRINT(X)
#endif

#include "../inc/tr69cdefs.h" /* defines for ACS state */

/** local definitions **/
#ifndef UINT32
#define UINT32 unsigned long
#endif

#ifndef UBOOL8
#define UBOOL8 unsigned short
#endif

/** from main **/
void *tmrHandle = NULL;
LimitNotificationQInfo limitNotificationList;

void setInformState(eInformState state);
void cancelPeriodicInform(void);

/** external functions **/
/* extern char *strdup(char*str); */
extern int cmsTmr_replaceIfSooner(void *handle, EventHandler func, void *ctxData, unsigned int ms, const char *name);
extern char *generateBasicAuthorizationHdrValue( SessionAuth *sa, char *user, char *pwd);
extern void activeNotificationImpl(void* handle);

/** external data **/
extern ACSState    acsState;
extern int	transferCompletePending;
extern int saveNotificationsFlag;
extern int sendGETRPC;			/* send a GetRPCMetods */
extern int tr69cTerm;			/* TR69C termination flag */

extern TRxObjNode rootDevice[];

/** public data **/
eInformState   informState = eACSNeverContacted;
eSessionState  sessionState = eSessionEnd;
InformEvList   informEvList;

/** private data **/
static CookieHdr *glbCookieHdr = NULL;
static HttpTask    httpTask;       /* http io desc */
static SessionAuth sessionAuth;
static int	sentACSNull; 		/* set if last msg to ACS was NULL, cleared if non-null */
								/* response received from ACS */
static int rebootingFlag = FALSE; /*set if we are doing system reboot or factoryreset*/

/** private functions **/
static int  getInformEvCnt(void);
static void closeACSConnection(HttpTask *ht);
static void acsDisconnect(HttpTask *ht, AcsStatus acsStatus);
static void updateAuthorizationHdr(HttpTask *ht);
static void nullHttpTimeout(void *handle);
static void postComplete(void *handle);
static void Connected(void *handle);
void sendInform(void* handle);
static void sendInformData(void);
static void startACSComm(void *handle);
static int  getRandomNumber(int from, int to);
static int  getDelayTime(int acsConnectFails);
static void sendNullHttp(UBOOL8 disconnect);
static int	sendDiagComplete;	/* send Diagnostic inform */


/* static int	notificationPending; */
/* static int	connectionReqPending; */
/* static int	periodicInformPending; */



int getRandomNumber(int from, int to)
{
   int num = 0;
   srand((unsigned int)time((time_t *)NULL));
   num = (rand() % (to - from)) + from;

   return num;
}

int getDelayTime(int acsConnectFails)
{
   int delayTime = 0;
   /* the following implementation is based on section */
   /* 3.2.1.1 Session Retry Policy in "TR-069 Amendment 1" */
   /* Table 3 - Session Retry Wait Intervals */
   switch (acsConnectFails)
   {
      case 1:
         delayTime = getRandomNumber(5, 10);
         break;
      case 2:
         delayTime = getRandomNumber(10, 20);
         break;
      case 3:
         delayTime = getRandomNumber(20, 40);
         break;
      case 4:
         delayTime = getRandomNumber(40, 80);
         break;
      case 5:
         delayTime = getRandomNumber(80, 160);
         break;
      case 6:
         delayTime = getRandomNumber(160, 320);
         break;
      case 7:
         delayTime = getRandomNumber(320, 640);
         break;
      case 8:
         delayTime = getRandomNumber(640, 1280);
         break;
      case 9:
         delayTime = getRandomNumber(1280, 2560);
         break;
      default:
         delayTime = getRandomNumber(2560, 5120);
         break;
   }

   return delayTime;
}

void retrySessionConnection(void)
{
   HttpTask *ht = &httpTask;
   UINT32 retryCount = acsState.retryCount;

   if (ht->wio != NULL)
   {
      wget_Disconnect(ht->wio);
      ht->wio = NULL;
      ht->xfrStatus = ePostError;
      ht->eHttpState = eClosed;
      ht->eAuthState = sIdle;
   }

   if (acsState.retryCount > 0)
   {
      /* the following implementation is based on section */
      /* 3.2.1.1 Session Retry Policy in "TR-069 Amendment 1" */
      /* Table 3 - Session Retry Wait Intervals */
      /* delay time for acsConnectFails that is greater than 10 */
      /* is the same with acsConnectFails that is equal to 10 */
      if (acsState.retryCount > 10)
         retryCount = 10;
      /* retry ACS connect errors with increasing retry time interval */
      UINT32 backOffTime = getDelayTime(retryCount) * 1000;

      DBGPRINT((stderr, "ACS connect failed, retryCount = %d, backOffTime = %dms\n", retryCount, backOffTime));
      cmsTmr_replaceIfSooner(tmrHandle, sendInform, NULL, backOffTime, "conn_fail_backoff_inform");
   }
   else
   {
		cmsTmr_replaceIfSooner(tmrHandle, sendInform, NULL, 0, "conn_backoff_retryReset");
		DBGPRINT((stderr, "ACS connect retry, no more backoff, retryCount = %d\n", retryCount));
   }
}

int isAcsConnected(void)
{
    int ret;
    ret = ((httpTask.wio != NULL) && (httpTask.wio->pc != NULL));
    DBGPRINT((stderr, "isAcsConnected: <%s>\n",ret? "Connected to ACS" : " NOT Connected to ACS"));
    return ret;
	/* return httpTask.wio!=NULL; */
}


tProtoCtx *getAcsConnDesc(void)
{
   if (!isAcsConnected())
   {
      return NULL;
   }
#ifdef USE_SSL
   if (httpTask.wio->pc->type == iSsl && httpTask.wio->pc->ssl != NULL)
   {
      if (httpTask.wio->pc->sslConn > 0)
      {
         return (httpTask.wio->pc);
      }
      else
      {
         return NULL;
      }
   }
   else
   {
      return (httpTask.wio->pc);
   }
#else
   return (httpTask.wio->pc);
#endif
}

void clearInformEventList(void)
{
   int count, i;

   count = informEvList.informEvCnt;
   for (i=0; i< count; i++)
   {
      informEvList.informEvList[i] = 0;
   }
   informEvList.informEvCnt = 0;
}

static int getInformEvCnt(void)
{
	return informEvList.informEvCnt;
}

#if 0
static int putInformEvent(eInformEvent event)
{
    BCMTRACE_ENTRY
    if (informEvList.informEvCnt<MAXINFORMEVENTS) {
        informEvList.informEvList[informEvList.informEvCnt++] = event;
        return informEvList.informEvCnt;
    }
    return 0;
}
#endif

#if 0
static void putInformEventMmethod( eRPCMethods mMethod )
{

    BCMTRACE_ENTRY

	informEvList.mMethod = mMethod;
}
#endif

#if 0
static int isConnectionReqPending(void)
{
	int ret = connectionReqPending;

    BCMTRACE_ENTRY

    connectionReqPending = 0;
	return ret;
}
#endif

static void closeACSConnection( HttpTask *ht)
{
    if (ht->wio)
    {
		wget_Disconnect(ht->wio);
		ht->wio = NULL;
	}
}
static void acsDisconnect(HttpTask *ht, AcsStatus acsStatus)
{
    static AcsStatus acsStatusPrev;
	int	active = 0;

    stopTimer(nullHttpTimeout, (void *)ht);
    /* saveNotificationAttributes(); */

    if (ht)
    {
        DBGPRINT((stderr, "acsDisconnect, free authHdr authHdr<%p>\n",ht->authHdr));
        free(ht->authHdr);
        ht->authHdr = NULL;
    }


	closeACSConnection(ht);
	ht->xfrStatus = acsStatus;
	ht->eHttpState = eClosed;
	ht->eAuthState = sIdle;

   switch (acsStatus)
   {
    case eAuthError:
        /* ++acsConnectFails; */
        ++acsState.retryCount;
        if (acsState.informEnable)
        {
            /* TODO */
            cancelPeriodicInform();
        }
        DBGPRINT((stderr, "Failed authentication with ACS\n"));
        /* TODO */
        /* saveTR69StatusItems(); */  /* save retryCount to scratchpad */
        break;
	case eConnectError:
	case eGetError:
	case ePostError:
		 /* ++acsConnectFails; */
         ++acsState.retryCount;
         if (acsState.informEnable)
         {
             cancelPeriodicInform();
         }
         DBGPRINT((stderr, "ACS Disconnect with error %d\n", acsStatus));

#ifdef ALLOW_DISCONNECT_ERROR
         DBGPRINT((stderr, "Continue processing even though ACS Disconnect with error\n"));
         /* saveACSContactedState(); */
         /* saveConfigurations(); */
         /* TODO */
         rebootingFlag = 0;
         /* rebootingFlag = factoryResetCompletion(); */ /* this will cause a reboot if factoryResetFlag==1 */
         /* rebootCompletion(); */
		 /* factoryResetCompletion(); */
#endif
         break;
    case eAcsDone:
	default:
         /* acsConnectFails = 0; */
         /* should NOT clear acsState.retryCount here, only clear */
         /* it after receiving InformResponse to make this value */
         /* can be shown correctly in Inform. */
         if (acsStatusPrev != eAcsDone)
         {
            resetPeriodicInform(acsState.informInterval);
         }
		 DBGPRINT((stderr, "ACS Disconnect: ok\n"));

		 /* if no error then run thru pending disconnect actions */
         /*	saveACSContactedState(); */
         /*  saveTR69StatusItems(); */
         /*	saveConfigurations(); */
         rebootCompletion();
	     /* factoryResetCompletion(); */
		 break;
	}

    /*
	if (notificationPending){
		active = checkActiveNotifications();
		notificationPending = 0;
	}
	*/

	if (tr69cTerm)
    {
        DBGPRINT((stderr, "TR69C terminated due to tr69cTerm flag\n"));
        tr69cTerm = 0;			/* Clear tr69c termination flag */

        /* this function calls all cleanup functions and exits with specified code */
        /* TODO  main_cleanup(0) */
		/* unlink(TR69_PID_FILE); */	/* Remove tr69c pid file */
		exit(0);
	}

	if (  (getInformEvCnt() != 0) /* active > 0 */
       || transferCompletePending
       || sendGETRPC
       || (acsStatus==eAuthError)
       || (acsStatus==eConnectError)
       || (acsStatus==eGetError)
       || (acsStatus==ePostError))   /* session retry policy */
    {
        DBGPRINT((stderr, "Sending a new inform from Disconnect - Maybe because AUTH,CONNECT,GET,POST error\n"));
        /* sendInform(NULL); */

        /* DBGPRINT((stderr, "calling retrySessionConnection....from acsDisconnect\n")); */
        retrySessionConnection();
#if 0
        if (acsConnectFails==0)
            sendInform(NULL);
        else
        {
            /* retry ACS connect errors with decreasing retry time interval */
            int backOffTime = acsConnectFails>CONN_DECAYMAX?
                              CONN_DECAYTIME*CONN_DECAYMAX*1000: CONN_DECAYTIME*acsConnectFails*1000;
            setTimer(sendInform, NULL, backOffTime);
        }
#endif
    }
	acsStatusPrev = acsStatus;
}

#if 0
static void refreshMyIPInfo(void)
{
    initACSConnectionURL();
    return;
}
#endif

static void updateAuthorizationHdr( HttpTask *ht )
{
   if (ht->authHdr)
   {
      if ( sessionAuth.qopType==eAuth )
      {
         free(ht->authHdr); ht->authHdr = NULL;
         ht->authHdr = generateNextAuthorizationHdrValue(&sessionAuth, acsState.acsUser, acsState.acsPwd);
         wget_AddPostHdr(ht->wio, "Authorization", ht->authHdr); /* replace header */
      }
      else
      {
         /* must be basic or not "Auth" */
         free(ht->authHdr); ht->authHdr = NULL;
         ht->authHdr = generateBasicAuthorizationHdrValue(&sessionAuth, acsState.acsUser, acsState.acsPwd);
         wget_AddPostHdr(ht->wio, "Authorization", ht->authHdr); /* replace header */
      }
   }
}

static void nullHttpTimeout(void *handle)
{
    HttpTask *ht = (HttpTask*)handle;

	DBGPRINT((stderr, "nullHttpTimeout - disconnect\n"));
    acsDisconnect(ht, eAcsDone);
    return;
}

void sendNullHttp(UBOOL8 disconnect)
{
    HttpTask *ht = &httpTask;

    if (ht->wio)
    {
        DBGPRINT((stderr, "sendNullHttp(%s) to ACS\n",ht->eHttpState==eClose? "close": "keepOpen"));
    }
    if (ht->postMsg)
    {
        free(ht->postMsg);
        ht->postMsg = NULL;
    }
    sendToAcs(0, NULL);
    /* send empty POST only count when holdrequests is false */
    if (acsState.holdRequests == FALSE)
    {
        sentACSNull = 1;
    }
    if (disconnect == TRUE)
    {
        setTimer(nullHttpTimeout,(void *)ht, ACSRESPONSETIME/2); /* half of max */
    }
}

#ifdef SUPPORT_ACS_CISCO
void informRspTimeout( void *handle )
{
    informState = eACSInformed;
    setInformState(eACSInformed);
    if (transferCompletePending)
    {
      sendTransferComplete();
      transferCompletePending = 0;
    }
    else
    {
       sendNullHttp(TRUE);
    }

    /* need this to get CISCO notifications to work.Since we never get a */
    /* InformResponse for the Cisco tool */
    resetNotification();
}
#endif

static void freeMsg(char *buf, int *bufSz)
{
    if (buf != NULL)
    {
        free(buf);
        buf = NULL;
    }
    *bufSz = 0;
}

static char *readLengthMsg(tWget *wg, int readLth, int *mlth, int doFlushStream)
{
    int bufCnt = 0, readCnt = 0;
    int bufLth = readLth;
    char *soapBuf = NULL;

    *mlth = 0;
    /*
         * This is the path taken when we do image download.  Don't zeroize
         * the buffer that is allocated here because that will force linux
         * to immediately assign physical pages to the buffer.  Intead, just
         * let the buffer fill in as the transfer progresses.  This will give
         * smd and the kernel more time to make physical pages available.
         */
    if ((soapBuf = (char *) malloc(readLth + 1)) != NULL)
    {
        while (bufCnt < readLth)
        {
            /* since proto_Readn cannot display SOAP message to console for debug, */
            /* proto_Readn is replaced with proto_ReadWait that has modifications to */
            /* call proto_Readn, and also display SOAP message to console. */
            if ((readCnt = proto_ReadWait(wg->pc, soapBuf+bufCnt, bufLth)) > 0)
            /* if ((readCnt = proto_Readn(wg->pc, soapBuf+bufCnt, bufLth)) > 0) */
            {
                bufCnt += readCnt;
                bufLth -= readCnt;
            }
            else
            {
                if (readCnt == -99)
                {
                    /* read error */
                    *mlth = 0;

                    free(soapBuf);
                    soapBuf = NULL;
                    DBGPRINT((stderr, "download interrupted\n"));
                    break;
                }
            }
        }
        DBGPRINT((stderr, "soapBuf bufCnt=%d readLth=%d\n", bufCnt, readLth));
        if (readCnt != -99)
        {
            *mlth = bufCnt;
            soapBuf[bufCnt] = '\0';
        }
        if (doFlushStream)
        {
            /* If we are not processing a chunked message, */
            /* skip(flush) anything else */
            proto_Skip(wg->pc);
        }
    }

    return soapBuf;
}

static char *readChunkedImage(tWget *wg, int *mlth)
{
   int imageBufSize;
   char *imageBuf = NULL;
   char tmpBuf[128];
   int chunkSize;
   int total=0;
   int bytesRead;

   *mlth = 0;

   imageBufSize = 7*1024; /* get_max_image_buf_size(); */
   if (NULL == (imageBuf = (char *) malloc(imageBufSize)))
   {
      DBGPRINT((stderr, "Failed to allocate %d bytes for chunked image download\n",imageBufSize));
      return NULL;
   }

   /* keep reading the chunk header for length info */
   do {
      bytesRead = proto_Readline(wg->pc, tmpBuf, sizeof(tmpBuf));
      DBGPRINT((stderr, "chunk header bytes=%d\n", bytesRead));

      sscanf(tmpBuf, "%x", &chunkSize);
      DBGPRINT((stderr, "chunksize=0x%x (%d)\n", chunkSize, chunkSize));

      if (total+chunkSize > imageBufSize)
      {
         DBGPRINT((stderr, "next chunk would exceed imageBufSize, %d+%d>%d\n", total, chunkSize, imageBufSize));
         total = 0;
         break;
      }

      if (0 == chunkSize)
      {
         DBGPRINT((stderr, "detected end chunk!\n"));
      }
      else
      {
         /* read the entire chunk of data */
         bytesRead = proto_Readn(wg->pc, imageBuf+total, chunkSize);
         if (chunkSize != bytesRead)
         {
            DBGPRINT((stderr, "===>length mismatch, wanted chunkSize=%d, bytesRead=%d\n", chunkSize, bytesRead));
            total = 0;
            break;
         }

         total += bytesRead;
      }

      /* after reading a chunk, there will be a CRLF (0x0d 0x0a), swallow it */
      bytesRead = proto_Readline(wg->pc, tmpBuf, sizeof(tmpBuf));
   } while (chunkSize > 0);

   if (total > 0)
   {
      /* OK, properly formatted transfer, return the amount of data read. */
      *mlth = total;
      DBGPRINT((stderr, "success! total data=%d\n", total));
   }
   else
   {
      /* something wrong, suck in and discard all data */
      proto_Skip(wg->pc);
      free(imageBuf);
      imageBuf = NULL;
   }

   return imageBuf;
}

static char *readChunkedMsg(tWget *wg, int *mlth, int maxSize)
{
   char *soapBuf = NULL;
   char chunkedBuf[128];

   *mlth = 0;
   /* read chunked size of first chunk */
   if (proto_Readline(wg->pc, chunkedBuf, sizeof(chunkedBuf)) > 0)
   {
      int  chunkedSz = 0, readSz = 0;
      char *newBuf = NULL, *readBuf = NULL;

      sscanf(chunkedBuf, "%x", &chunkedSz);
      while (chunkedSz > 0)
      {
         /* read chunked data */
         int doFlushStream=FALSE;
         readBuf = readLengthMsg(wg, chunkedSz, &readSz, doFlushStream);
         if (chunkedSz != readSz)
            DBGPRINT((stderr, "===> readChunkedMsg, chunked size = %d, read size = %d\n", chunkedSz, readSz));
         if (readBuf == NULL)
         {
            freeMsg(soapBuf, mlth);
            break;
         }
         if ((*mlth + readSz) > maxSize)
         {
            DBGPRINT((stderr, "reading more data than maxSize (%d)\n", maxSize));
            freeMsg(soapBuf, mlth);
            freeMsg(readBuf, &readSz);
            break;
         }
         if (soapBuf == NULL)
         {
            /* allocate the first chunk since cmsMem_realloc */
            /* does not accept soapBuf as NULL pointer. */
            newBuf = soapBuf = malloc(*mlth + readSz);
         }
         else
         {
            /* reallocate soap message size */
            newBuf = realloc(soapBuf, *mlth + readSz);
         }

         if (newBuf == NULL)
         {
            freeMsg(soapBuf, mlth);
            freeMsg(readBuf, &readSz);
            break;
         }
         /* point soap message to new allocated memory */
         soapBuf = newBuf;
         /* append chunked data to soap message */
		 strncpy(soapBuf + *mlth, readBuf, readSz);
		 /* increase soap message size */
        *mlth += readSz;
        /* free chunked data */
        freeMsg(readBuf, &readSz);
        chunkedSz = 0;
        /* flush off trailing crlf */

        /* TODO */
        DBGPRINT((stderr, "chunked msg not implemented!\n"));
#if 0
         do
         {
            chunkedBuf[0] = '\0';
            readSz = proto_Readline(wg->pc, chunkedBuf, sizeof(chunkedBuf));
         } while (readSz > 0 && isxdigit(chunkedBuf[0]) == 0);
         /* read chunked size of next chunk */
         if (isxdigit(chunkedBuf[0]) != 0)
         {
            sscanf(chunkedBuf, "%x", &chunkedSz);
         }
         else
         {
            freeMsg(soapBuf, mlth);
         }
#endif
      }
      /* skip(flush) anything else */
      proto_Skip(wg->pc);
   }

   return soapBuf;
}

/*
 * tWget *wg is an connected web descriptor,
 *      *mlth is pointer to location of result read length,
 *      maxBufferSize is maximum size to read if non-zero. No limit if maxSize is 0.
 * Returns:
 *     pointer to response buffer or NULL.
 *     *mlth contain size of buffer. Undefined if return is NULL.
 */
#if 1
char *readResponse( tWget *wg, int *mlth, int maxBufferSize)
{
    char *soapBuf = NULL;

    if (wg->hdrs->content_length > 0)
    {
        int doFlushStream = TRUE;
        /* this is the path taken by traditional one-shot image downloads */
        DBGPRINT((stderr, "calling readLengthMsg for Download with content_length=%d\n", wg->hdrs->content_length));
        soapBuf = readLengthMsg(wg, wg->hdrs->content_length, mlth, doFlushStream);
    }
    else if (wg->hdrs->TransferEncoding && !strcasecmp(wg->hdrs->TransferEncoding,"chunked"))
    {
		/* content length is 0 */
        if (wg->hdrs->TransferEncoding && !strcasecmp(wg->hdrs->TransferEncoding,"chunked"))
        {
            if (wg->hdrs->content_type && !strncasecmp(wg->hdrs->content_type, "application/octet-stream", sizeof("application/octet-stream") - 1))
            {
                /* this is the path taken by HTTP/1.1 chunked image downloads */
                DBGPRINT((stderr, "starting chunked image download!!\n"));
                soapBuf = readChunkedImage(wg, mlth);
            }
            else
            {
                /* this is a standard soap message, content type prob text/xml */
                int maxSize = (maxBufferSize > 0) ? maxBufferSize : MAXWEBBUFSZ;
                DBGPRINT((stderr, "calling readChunkedMsg with maxSize=%d\n", maxSize));
                soapBuf = readChunkedMsg(wg, mlth, maxSize);
            }
        }
        else
        {
            DBGPRINT((stderr, "Don't know what kind of response this is!?!\n"));
        }
    }

    return soapBuf;
}
#else
/* old readResponse TODO */
char *readResponse( tWget *wg, int *mlth, int maxBufferSize)
{
    int     maxSize;
    char    *soapBuf = NULL;

    BCMTRACE_ENTRY

    *mlth = 0;
    maxSize = maxBufferSize>0? maxBufferSize: MAXWEBBUFSZ;
    if (wg->hdrs->content_length>0)
    {
        int bufCnt = 0;
        int bufLth = wg->hdrs->content_length;
        int readSz;
        if ((soapBuf = (char *) malloc(bufLth+1)))   /* add one for null */
        {
            while ( bufCnt < wg->hdrs->content_length )
            {
                if ( (readSz = proto_ReadWait(wg->pc, soapBuf+bufCnt, bufLth)) > 0 )
                {
                    bufCnt += readSz;
                    bufLth -= readSz;
                }
                else
                {
                    /* read error */
                    *mlth = 0;
                    free(soapBuf);
                    return NULL;
                }
            }
            tr69log(LogDEBUG, "soapBuf readCnt=%d ContentLth=%d\n" , bufCnt, wg->hdrs->content_length );
            *mlth = bufCnt;
            soapBuf[bufCnt] = '\0';
            return soapBuf;
        }
    }
    else if (wg->hdrs->TransferEncoding && !strcasecmp(wg->hdrs->TransferEncoding,"chunked"))
    {
        char cbuf[80];
        /* read length of first chunk*/
        if (proto_Readline(wg->pc, cbuf, sizeof(cbuf)))
        {
            int chksz=0;
            sscanf(cbuf, "%x", &chksz);
            while (chksz)
            {
                /* walk thru chunks and read all chunks into the realloc buffer */
                char *newBuf;
                if ( (*mlth+chksz) > maxSize )
                {
                    free (soapBuf);
                    *mlth = 0;
                    return NULL;
                }
                if ( (newBuf=realloc(soapBuf, chksz))==NULL)
                {
                    free(soapBuf);
                    *mlth = 0;
                    return NULL;
                }
                soapBuf = newBuf;
                if ( proto_ReadWait(wg->pc, soapBuf+*mlth, chksz)!=chksz)
                {
                    free(soapBuf);
                    *mlth = 0;
                    return NULL;
                }
                *mlth += chksz;
                /* read next chunk size */
                if (proto_Readline(wg->pc, cbuf, sizeof(cbuf)))
                {
                    chksz=0;
                    sscanf(cbuf, "%x", &chksz);
                }
                else
                {
                    free(soapBuf);
                    *mlth = 0;
                    return NULL;
                }
            }
            proto_Readline(wg->pc, cbuf, sizeof(cbuf));   /* flush off trailing crlf */
            proto_Skip(wg->pc);         /* skip(flush) anything else */
            return soapBuf;
        }
    }
    return NULL;
}
#endif

static void addCookie(CookieHdr *hdr, CookieHdr *cookie)
{
    CookieHdr *curr = hdr, *prev = hdr;
    /* does cookie already exist in cookie list */
    for (curr = hdr, prev = hdr;
         curr != NULL;
         prev = curr, curr = curr->next)
    {

        if (strcmp(curr->name, cookie->name) == 0 &&
            strcmp(curr->value, cookie->value) == 0)
       {
           break;
       }
    }

    /* if cookie is not in cookie list then add it */
    if (curr == NULL)
    {
        curr = (CookieHdr*) malloc(sizeof (CookieHdr));
        curr->name = strdup(cookie->name);
        curr->value = strdup(cookie->value);
        curr->next = NULL;
        prev->next = curr;
    }
}

static void copyCookies(CookieHdr **dst, CookieHdr *src)
{
    CookieHdr *cp = src;

    /* create cookie head */
    if (*dst == NULL && cp != NULL)
    {
        *dst = (CookieHdr *) malloc(sizeof (CookieHdr));
        (*dst)->name = strdup(cp->name);
        (*dst)->value = strdup(cp->value);
        (*dst)->next = NULL;
        cp = cp->next;
    }

    /* add cookie to cookie head */
    while (cp != NULL)
    {
        addCookie(*dst, cp);
        cp = cp->next;
    }
}

static void freeCookies(CookieHdr **p)
{

    CookieHdr *next = *p;

    while (next)
    {
        CookieHdr *temp;
        temp = next->next;
        if (next->name)
        {
            free(next->name);
            next->name = NULL;
        }
        if (next->value)
        {
            free(next->value);
            next->value = NULL;
        }
        free(next);
        next = temp;
    }
}

static void handleSoapMessage(char *soapmsg, int len)
{
    if ( soapmsg )
    {
        eParseStatus status;
        ParseHow parseReq;

        DBGPRINT((stderr, "SoapInMsg=>>>>>\n%s\n<\n", soapmsg));
        parseReq.topLevel = envelopeDesc;
        parseReq.nameSpace = nameSpaces;
        status = parseGeneric(NULL, soapmsg, len, &parseReq);
        free(soapmsg);
        soapmsg = NULL;
        if (status == NO_ERROR)
        {
            if (runRPC() == eRPCRunFail)
            {
                /* couldn't run the RPC: so just disconnect */
                acsDisconnect(&httpTask, eAcsDone);
            }
        }
        else
        {
            DBGPRINT((stderr, "ACS Msg. Parse Error %80s\n", soapmsg));
            acsDisconnect(&httpTask, eAcsDone);  /* force disconnect on parse error*/
        }
    }
    else
    {
        /* no response */
        DBGPRINT((stderr, "status = 200, no Soapmsg. sentACSNull=%d\n", sentACSNull));
        if (!sentACSNull)
        {
            DBGPRINT((stderr, "sending NULL HTTP to ACS because told to 1\n"));
            sendNullHttp(FALSE);
        }
        acsDisconnect(&httpTask, eAcsDone);
    }
}

static void handleSoapMessageCallBack(void *handle)
{
	HttpTask *ht = &httpTask;
	tWget *wg = (tWget *)handle;
	char *soapmsg = (char *)wg->handle;

	ht->eHttpState = eConnected;

	/* copy cookies from previous connection to the new one */
	if (glbCookieHdr != NULL)
	{
	    copyCookies(&(ht->wio->cookieHdrs), glbCookieHdr);
	}

	handleSoapMessage(soapmsg, strlen(soapmsg));
}

/*
 * Data has been posted to the server or an
 * error has ocurred.
 */
static void postComplete(void *handle)
{
    tWget *wg = (tWget *)handle;
    HttpTask    *ht = &httpTask;
    SessionAuth *sa = &sessionAuth;
	int			skipResult=1;

    DBGPRINT((stderr, "============\n"));
    DBGPRINT((stderr, "postComplete\n"));
    DBGPRINT((stderr, "============\n"));
    stopTimer(nullHttpTimeout, (void *)ht);

    DBGPRINT((stderr, "Username: <%s>\n",acsState.acsUser));
    DBGPRINT((stderr, "Password: <%s>\n",acsState.acsPwd));

    if (wg->status==iWgetStatus_Ok)
    {
        if (ht->wio->hdrs->Connection && !strcasecmp(ht->wio->hdrs->Connection,"close"))
        {
            /* the ACS does not want a HTTP Persistent Connection. It is closing down. */
            ht->eHttpState = eClose;
        }
        DBGPRINT((stderr, "Connection = %s\n", ht->eHttpState==eClose?"close": "keep-alive"));
        if (wg->hdrs->status_code== 401)
        {
            DBGPRINT((stderr, "Debug-postComplete: ACS returned a 401 error\n"));
            /* need to send authenticate */
            char *hdrvalue;
            if (wg->hdrs->content_length > 0 ||(wg->hdrs->TransferEncoding && streq(wg->hdrs->TransferEncoding,"chunked")))
            {
                int mlth;
                char *tmpBuf;
                if ((tmpBuf=readResponse(wg,&mlth,0)))
                {
                    free(tmpBuf); tmpBuf = NULL;
                }
            }
            sentACSNull = 0;
            free(ht->authHdr); ht->authHdr = NULL; /* free in case of reauth requested during connection */
            if ( ht->eAuthState==sAuthenticating)
            {
                DBGPRINT((stderr, "\n++++++++ Authenticating FAILED!!!!!!  disconect and ret\n\n"));
                ht->eAuthState = sAuthFailed;
                free(ht->postMsg); ht->postMsg = NULL;
                ht->postLth = 0;
                /* disconnect and delay */
                acsDisconnect(ht, eAuthError);
                return;
            }
            else
            {
                DBGPRINT((stderr, "eAuthState set to Authenticating\n"));
                ht->eAuthState = sAuthenticating;
            }

            DBGPRINT((stderr, "WWW-Authenticate= %s\n", wg->hdrs->wwwAuthenticate));
            if (!(hdrvalue = generateAuthorizationHdrValue(sa, wg->hdrs->wwwAuthenticate, "POST", ht->wio->uri, acsState.acsUser, acsState.acsPwd)))
            {
                DBGPRINT((stderr, "WWWAuthenticate header parsing error: %s\n", wg->hdrs->wwwAuthenticate));
                free(ht->postMsg); ht->postMsg = NULL;
                acsDisconnect(ht, eAuthError);
                return;
            }
            ht->authHdr = hdrvalue;
            if (skipResult == 0 || ht->eHttpState == eClose)
            {   /* end of data on 401 skip_Proto() */
                /* save cookies of the current connection */
                if (wg->hdrs->setCookies != NULL)
                    copyCookies(&glbCookieHdr, wg->hdrs->setCookies);
                /* close connection and reconnect with Authorization header */
                DBGPRINT((stderr, "postComplete: Closing ACS connection so we can reconnect with Auth header\n"));
                closeACSConnection(ht);
                DBGPRINT((stderr, "DEBUG-postComplete: ACS Connection should be closed now!!!!!z\n"));
                ht->wio = wget_Connect(acsState.acsURL, Connected, NULL);
                DBGPRINT((stderr, "DEBUG-postComplete: ACS Connection should be connected now !!!!!\n"));
                if (ht->wio==NULL)
                {
                    DBGPRINT((stderr, "ACS Connect failed: %s\n", wget_LastErrorMsg()));
                    free (ht->postMsg); ht->postMsg = NULL;
                    free (ht->authHdr); ht->authHdr = NULL;
                    return;
                }

                ht->eHttpState = eConnected;
                /* copy cookies from previous connection to the new one */
                if (glbCookieHdr != NULL)
                {
                    copyCookies(&(ht->wio->cookieHdrs), glbCookieHdr);
                }
                /* return here since sendInformData will be called in Connected */
                /* printf("DEBUG-postComplete: return here since sendInformData will be called in Connected\n"); */
                return;
            }

            /* now just resend the last data with the Authorization header */
            sendInformData();
            /* now we just return to wait on the response from the server */
#ifdef FORCE_NULL_AFTER_INFORM
            DBGPRINT((stderr, "set Timer to Force Null send to ACS (Cisco tool)\n"));
            cmsTmr_set(tmrHandle, informRspTimeout, ht, 1*1000, "informRspTimeout"); /******** ?????????????CISCO TOOL ???????? send null if server doesn't respond */
#endif
        }
        else if (wg->hdrs->status_code  >= 300 &&  wg->hdrs->status_code <= 307)
        {
            if (wg->hdrs->locationHdr != NULL)
            {
                /* Redirect status with new location */
                /* repost msg to new URL */
                closeACSConnection(ht);
                ht->eHttpState = eStart;
                DBGPRINT((stderr, "Redirect to %s\n", wg->hdrs->locationHdr));
                ht->wio = wget_Connect(wg->hdrs->locationHdr, Connected,  NULL);

                if (ht->wio == NULL)
                {
                    DBGPRINT((stderr, "Redirect failed: %s\n", wget_LastErrorMsg()));
                    acsDisconnect(ht, eConnectError);
                }
            }
            else
            {
                DBGPRINT((stderr, "Redirect failed: location header is empty\n"));
                acsDisconnect(ht, eConnectError);
            }
        }
        else
        {
            /* If status != 401 after inform, and eAuthState == sIdle, */
            /* we're in the wrong authentication state.  The correct state should be from */
            /* sAuthenticating to sAutenticated so disconnect unless */
            /*  ACS does not have any authentications and returns status_code == 200 */
            /* right after CPE sends the first Inform message */
            if (ht->eAuthState == sIdle && wg->hdrs->status_code != 200)
            {
                ht->eAuthState = sAuthFailed;
                /* disconnect and delay */
                acsDisconnect(ht, eAuthError);
                DBGPRINT((stderr, "Error: From sIdle -> sAuthenticated w/t sAuthenticating\n"));
                return;
            }

            ht->eAuthState = sAuthenticated;
            if (informState == eACSNeverContacted)
            {
                informState = eACSContacted;
            }

            if ((wg->hdrs->status_code == 200) && ((wg->hdrs->content_length > 0) || (wg->hdrs->TransferEncoding && streq(wg->hdrs->TransferEncoding,"chunked"))))
            {
                /* allocate buffer and parse the response */
                int     mlth;
                char    *soapmsg;

                /* msg posted - free buffer */
                free(ht->postMsg); ht->postMsg = NULL;
                soapmsg = readResponse(wg, &mlth,0);

                /* if TCP connection is closed by ACS */
                if (ht->eHttpState == eClose)
                {
                    /* save cookies of the current connection */
                    if (wg->hdrs->setCookies != NULL)
                    {
                        copyCookies(&glbCookieHdr, wg->hdrs->setCookies);
                    }
                    /* close connection and reconnect since ACS asks for it */
                    closeACSConnection(ht);
                    ht->wio = wget_Connect(acsState.acsURL, handleSoapMessageCallBack, soapmsg);
                    if ( ht->wio == NULL )
                    {
                        DBGPRINT((stderr, "ACS Connect failed2: %s\n", wget_LastErrorMsg()));
                        free(ht->postMsg); ht->postMsg = NULL;
                        free(ht->authHdr); ht->authHdr = NULL;
                        return;
                    }
                    /* return here since handleSoapMessage will be called in handleSoapMessageCallBack */
                    return;
                }
                handleSoapMessage(soapmsg, mlth);
            }
            else if ((wg->hdrs->status_code == 204) ||
                     (wg->hdrs->status_code == 200 && !wg->hdrs->TransferEncoding))
            {
                /* only terminate session if no pending */
                if (transferCompletePending == 0 && sendGETRPC == 0)
                {
                    /* empty ACS message -- ACS is done */
                    DBGPRINT((stderr, "empty ACS msg - sentACSNull=%d\n", sentACSNull));
                    /* msg posted - free buffer */
                    free(ht->postMsg); ht->postMsg = NULL;
                    if (!sentACSNull)
                    {
                        DBGPRINT((stderr, "sending NULL HTTP to ACS because told to 2\n"));
                        sendNullHttp(FALSE);
                    }
                    if (wg->hdrs->status_code == 200 && ht->eHttpState == eClose)
                    {
                        closeACSConnection(ht);
                    }
                    else
                    {
                        acsDisconnect(ht, eAcsDone);
                        /* TR-069 session is finished */
                        sessionState = eSessionEnd;
                        /* free cookies since tr-069 session is finished */
                        if (glbCookieHdr != NULL)
                        {
                            freeCookies(&glbCookieHdr);
                            glbCookieHdr = NULL;
                        }
                    }
                }
                else
                {
                    if (transferCompletePending == 1)
                    {
                        /* transfer complete is pending so send it when receive empty message */
                        sendTransferComplete();
                        transferCompletePending = 0;
                        /* setACSContactedState to eACSInformed for clearing  */
                        /* previous state which is eACSDownloadReboot or eACSUpload */
                        setInformState(eACSInformed);
                    }
                    else if (sendGETRPC == 1)
                    {
                        /* TODO */
                        /* sendGetRPCMethods(); */
                        sendGETRPC = 0;
                    }
                }
            }
            else if (wg->hdrs->status_code == 100)
            {
                /* 100 Continue: Just ignore this status */
                /* msg posted - free buffer */
                free(ht->postMsg); ht->postMsg = NULL;
            }
            else if (wg->hdrs->status_code >= 300 &&
                     wg->hdrs->status_code <= 307 &&
                     wg->hdrs->locationHdr)
            {
                /* Redirect status with new location */
                /* repost msg to new URL */
                closeACSConnection(ht);
                ht->eHttpState = eStart;
                DBGPRINT((stderr, "Redirect to %s\n", wg->hdrs->locationHdr));
                ht->wio = wget_Connect(wg->hdrs->locationHdr, Connected,  NULL);

                if (ht->wio == NULL)
                {
                    DBGPRINT((stderr, "Redirect failed: %s\n", wget_LastErrorMsg()));
                    acsDisconnect(ht, ePostError);
                }
            }
            else
            {
                /* msg posted - free buffer */
                free(ht->postMsg); ht->postMsg = NULL;
                DBGPRINT((stderr, "Unknown status_code=%d received from ACS or encoding\n", wg->hdrs->status_code));
                acsDisconnect(ht, ePostError);
            }
        }
    }
    else
    {
        DBGPRINT((stderr, "Post to ACS failed, Status = %d %s\n", wg->status, wg->msg));
        free(ht->postMsg); ht->postMsg = NULL;
        acsDisconnect(ht, ePostError);
    }
}  /* End of postComplete() */

UINT32 addInformEventToList(int event)
{
    int   i;
    DBGPRINT((stderr, "adding event =%d\n", event));

	/*
	* Check for duplicate event codes in the informEvList.
	* We only need each event to appear once in the List/array.
	* Single cumulative behavior cannot be repeated, but multiple can.
	* At this point, multiple ones are added according to ACSstate.-- May need to change this to rely less on global var.
	*/
    for (i = 0; i < informEvList.informEvCnt; i++)
    {
        if (informEvList.informEvList[i] == event)
        {
            DBGPRINT((stderr, "event %d is already in informEvList, do nothing\n", event));
            return 0;
        }
    }

    if (informEvList.informEvCnt < MAXINFORMEVENTS)
    {
        informEvList.informEvList[informEvList.informEvCnt] = event;
        informEvList.informEvCnt++;
        DBGPRINT((stderr, "Adding event <%d> to informEvList, count is now <%d>\n", event, informEvList.informEvCnt));
        return informEvList.informEvCnt;
    }
    else
    {
        DBGPRINT((stderr, "Too many events in informEvList, count=%d\n", informEvList.informEvCnt));
    }

    return 0;
}


/*
 * The connection to the ACS has been completed or
 * an error has ocurred.
 */
static void Connected(void *handle)
{
	HttpTask *ht = &httpTask;
	tWget    *wg = (tWget *)handle;

	if (wg->status != 0)
	{
	    free(ht->postMsg); ht->postMsg = NULL;
	    acsDisconnect(ht, eConnectError);
	    DBGPRINT((stderr, "ACS Connect Status = %d %s\n", wg->status, wg->msg));
	    return;
	}
	if (ht->wio==NULL)
	{
	    ht->eHttpState = eClosed;
	    DBGPRINT((stderr, "Error -- pointer to IO desc is NULL\n"));
	    return;
	}

	ht->eHttpState = eConnected;
	sendInformData();
}

/*
 * Send the current buffer to the ACS. This is an async call. The
 * return status only indicates that the connection has started.
 * The httpTask structure represents the only connection to an
 * ACS. If the connection is up then it is reused; otherwise,
 * a new connection is attempted.
 *
*/
void sendToAcs(int contentLen, char *buf)
{
    HttpTask *ht = &httpTask;

    if (ht->postMsg)
    {
        DBGPRINT((stderr, "postMsg buffer not null\n"));
        free(ht->postMsg); ht->postMsg = NULL;
    }
    if (getAcsConnDesc() == NULL)
    {
        DBGPRINT((stderr, "Try to send message to ACS while connection is NULL!\n"));
        return;
    }
    ht->content_len = contentLen;
    ht->postMsg     = buf;
    ht->postLth     = buf? strlen(buf): 0;
    wget_ClearPostHdrs(ht->wio);
#ifdef GENERATE_SOAPACTION_HDR
    wget_AddPostHdr(ht->wio,"SOAPAction", "");
#endif
    updateAuthorizationHdr(ht);
    if (ht->eHttpState == eClose)
    {
        printf("sendToAcs is calling wget_PostDataClose because we have eClose!!!\n");
        wget_PostDataClose(ht->wio, ht->postMsg, ht->postLth, "text/xml", postComplete,( void*) ht);
    }
    else
    {
        printf("sendToAcs is calling wget_PostData!\n");
        wget_PostData(ht->wio, ht->postMsg, ht->postLth, "text/xml", postComplete, (void *)ht);
    }
}

/*
 * Send an Inform message. The possible handle values are
 * eIEConnectionRequest
 * eIETransferComplete
 * eIEJustBooted.
 * The other possible events are set by xxxPending flags.
 * eIEPeriodix
 * eIEDiagnostics
 *
 */
void sendInform(void *dummy __attribute__((unused)))
{

   int numOfValueChanged = 0; /*getMdmParamValueChanges();*/
    /* time_t now; */
    /* UINT32 deltaMs; */
    /* int timeToSend = 1; */
    LimitNotificationInfo *ptr;
    UINT32 eventCount;
    /* UINT32 notificationEvent = 0; */

    stopTimer(activeNotificationImpl, NULL);
    eventCount = informEvList.informEvCnt;
    if (eventCount == 0)
    {
      /* inform has been sent, there is nothing to send now */
      DBGPRINT((stderr, " No inform to be sent\n"));
      return;
    }

    if(rebootingFlag)
    {
      /* system is rebooting, we don't send inform */
      DBGPRINT((stderr, "System is rebooting, we don't send inform\n"));
      return;
    }

   numOfValueChanged = getAllNotifications();
   /* printf("NOT: Pending Notifications <%d>\n",numOfValueChanged); */
   if ( numOfValueChanged > 0 )
   {
     addInformEventToList(INFORM_EVENT_VALUE_CHANGE);
   }

    if (isAcsConnected())
    {
#if 0
        /* this change doesn't seem to be needed anymore.   We should just return if there is
        a session going on.   When the session ends, we can try to call inform again.
        */
        if (informState == eACSNeverContacted)
        {
            /* need to close connection to avoid system crash problem
             * when system is booted up without URL, then its value is changed
             * from WEB UI, then connection request is performed, then system is crashed
             * ==> call closeACSConnection to fix CR #16299
             */
            closeACSConnection(&httpTask);
        }
        else
        {
            /* inform needs to be sent, however, there is an outstanding
             * session going on.  Inform needs to sent in the next session.
             * (TR69 amendment 2, section 3.7) Inform message must not occur more
             * than once during a session unless this is retry.
             * Interop with Commtrend ACS, it is slow in sending empty http in Ping Test.
             */
            cmsTmr_set(tmrHandle,sendInform,NULL,5*MSECS_IN_SEC, "sendInformWhenSessionEnds");
            return;
        }
#else
        /* return; */
		DBGPRINT((stderr, "sendInform: ACS not connected, keep going and setup a connection\n"));
#endif
    }

    sentACSNull = 0;

    if (acsState.acsURL != NULL && acsState.acsURL[0] != '\0')
    {
      HttpTask *ht = &httpTask;
      CmsRet ret = CMSRET_SUCCESS;
      /* MdmPathDescriptor path; */
      /* int changed = 0; */
      /* UINT32 i; */

      /* Before inform is sent for valueChanged, we need to see if this is a notification that has limitNotification
       * set by ACS.  If it is, inform should not be sent out; set a timer to send later.
       * If this notification is sent for other purposes or
       * valueChanged of a different parameter, the limitNotification parameter is sent anyway in the same inform.
       * At this point, we cancel the pending notification.
       */
#if 0  /* TODO no notification now */
      if (limitNotificationList.count != 0)
      {


      /* are we sending notification for value changes? */
         for (i=0; i<eventCount; i++)
         {
            if (informEvList.informEvList[i] == eIEValueChanged)
            {
               notificationEvent = 1;
               break;
            }
         }

         if (notificationEvent && (limitNotificationList.count >= numOfValueChanged))
         {
            /* there is a notification that needs to be sent, just to see this is one of those
             * in limit notification list
             */
            ptr = limitNotificationList.limitEntry;
            //cmsTms_get(&now);
            now= time(NULL);

            for (; ptr != NULL; ptr = ptr->next)
            {
               if (cmsMdm_fullPathToPathDescriptor(ptr->parameterFullPathName,&path) != CMSRET_SUCCESS)
               {
                  continue;
               }

               if ((cmsLck_acquireLockWithTimeout(TR69C_LOCK_TIMEOUT)) == CMSRET_SUCCESS)
               {
                  changed = cmsPhl_isParamValueChanged(&path);
                  cmsLck_releaseLock();

                  if (changed)
                  {
                     deltaMs = cmsTms_deltaInMilliSeconds(&now,&ptr->lastSent);
                     if (deltaMs < (UINT32)(ptr->limitValue))
                     {
                        /* it's not time to send inform yet for this event.
                         * If there is another parameter that can send, then this event can go with it.
                         */
                        if (ptr->notificationPending == 0)
                        {
                           tr69log(LogDEBUG,"timerSet: deltaMs %d, limValue %d, toBeSent in %d ms\n",
                                        deltaMs,ptr->limitValue,(ptr->limitValue - deltaMs));

                           cmsTmr_set(tmrHandle,ptr->func,NULL,(ptr->limitValue - deltaMs), "limitNotification");
                           ptr->notificationPending = 1;
                        }

                        timeToSend = 0;
                        continue;
                     }
                     else
                     {
                        /* it's time to send! */
                        timeToSend = 1;
                        break;
                     }
                  } /* param changed */
               } /* acquire lock ok */
            } /* loop notifcation list */
         } /* notification pending */

         if (!timeToSend)
         {
            return;
         }
      } /* limit Notification != NULL */
#endif

      DBGPRINT((stderr, "Connect to ACS at %s\n", acsState.acsURL));
      ht->eHttpState = eStart;
      ht->wio = wget_Connect(acsState.acsURL, Connected, NULL);

      if (ht->wio == NULL)
      {
         DBGPRINT((stderr, "sendInform Failed: %s\n", wget_LastErrorMsg()));
         DBGPRINT((stderr, "set delayed inform timer for %d\n", CHECKWANINTERVAL));
         /* mwang: this looks a bit wrong.  The handle is used to
          * look up this timer.  But handle is this function is actually
          * an eInformEvent enum.  So are we saying if we get here,
          * the eInformEvent enum will always be the same?
          */
         ret = cmsTmr_replaceIfSooner(tmrHandle, sendInform, NULL, CHECKWANINTERVAL, "WANCHECK_inform");

         if (ret != CMSRET_SUCCESS)
         {
            DBGPRINT((stderr, "setting delayed wan inform timer failed, ret=%d\n", ret));
         }
      }
      else
      {
         /* go through the whole notification list, cancel timers because they will all be sent now */
         /* in this case, everything will be sent out, so we cancel all timers and update lastSent field */
         ptr = limitNotificationList.limitEntry;
         while (ptr != NULL)
         {
            /* Everything will be sent, so we update our list.
             * To simplify the whole implementation, I just update the lastSent for everyone in the list.
             * LastSent is basically the last notification sent, not necessarily the notification of this
             * parameter.
             */
            if (ptr->notificationPending)
            {
               DBGPRINT((stderr, "timerCancel for ptr->parameterFullPathName %s\n",ptr->parameterFullPathName));
               stopTimer(tmrHandle,ptr->func);
               ptr->notificationPending = 0;
            }
            /* cmsTms_get(&ptr->lastSent); */
            ptr = ptr->next;
         } /* loop */
      } /* inform sent */
    } /* acs */
    else
    {
        DBGPRINT((stderr, "acsURL is NULL!\n"));
    }
}  /* End of sendInform() */

static void sendInformData(void)
{
   RPCAction *a = newRPCAction();
   HttpTask    *ht = &httpTask;

   if (ht->eAuthState == sAuthenticating)
   {
      /* second Inform is ready to send out for authentication */
      sessionState = eSessionAuthenticating;
   }
   else
   {
      /* first Inform is ready to send out to start TR-069 session */
      sessionState = eSessionStart;
   }
   /* buildInform  TODO */
   int myextIp = 12345678;
   char *msg = NULL;
   msg = buildInform(a, &informEvList, myextIp);

   if (msg)
   {
       DBGPRINT((stderr, "\nsendInformData FLOW: Sending INFORM to ACS\n"));
       sendToAcs(strlen(msg),msg);
   }
   freeRPCAction(a);
}

#if 0
/** Send msg to smd requesting a delayed msg.
 *
 * The delayed msg contains a special id (PERIODIC_INFORM_TIMEOUT_ID) in the
 * wordData field of the message header.  When I get a DELAYED_MSG with this
 * special id, I know it is time to do a periodic inform.
 * Requesting a delayed message will also cancel any previous requests with the
 * same id.
 */
void requestPeriodicInform(UINT32 interval)
{
   char buf[sizeof(CmsMsgHeader) + sizeof(RegisterDelayedMsgBody)] = {0};
   CmsMsgHeader *msg;
   RegisterDelayedMsgBody *body;
   CmsRet ret;

   msg = (CmsMsgHeader *) buf;
   body = (RegisterDelayedMsgBody *) (msg + 1);

   msg->type = CMS_MSG_REGISTER_DELAYED_MSG;
   msg->src = EID_TR69C;
   msg->dst = EID_SMD;
   msg->flags_request = 1;
   msg->wordData = PERIODIC_INFORM_TIMEOUT_ID;
   msg->dataLength = sizeof(RegisterDelayedMsgBody);

   body->delayMs = interval * MSECS_IN_SEC;  /* tr69c uses seconds, smd uses ms */

   ret = cmsMsg_sendAndGetReply(msgHandle, msg);
   if (ret != CMSRET_SUCCESS)
   {
      cmsLog_error("request failed, ret=%d", ret);
   }
   else
   {
      cmsLog_debug("set next periodic inform for %u seconds in future", interval);
   }

   return;
}
#endif

#if 0
void cancelPeriodicInform()
{
   BCMTRACE_ENTRY
   // cancel local periodic inform timer in tr69c
   stopTimer(periodicInformTimeout, NULL);
}
#endif


/** Periodic Inform Timer callback.
 *
 * If peridic inform is enabled and informInterval > 0 then always create another
 * timer event.
 */
void periodicInformTimeout(void *handle __attribute__((unused)))
{
	/* periodicInformPending = 1; */

    DBGPRINT((stderr, "\n\n+++ Periodic Inform Timeout +++ \n\n"));

    addInformEventToList(INFORM_EVENT_PERIODIC);
    sendInform(NULL);

    resetPeriodicInform(acsState.informInterval);
	/* if (acsState.informEnable && acsState.informInterval) */
	/*	setTimer(periodicInformTimeout,NULL, acsState.informInterval*1000); */
}

/*
* Called from setter function to update next inform time
*/
void resetPeriodicInform(unsigned long interval)
{
    if (acsState.informEnable && acsState.informInterval)
    {
        cmsTmr_replaceIfSooner(tmrHandle, periodicInformTimeout, (void*)NULL, interval*1000, "periodic_inform");
        DBGPRINT((stderr, "Reset periodic inform: enable<%d>, interval<%d>\n", acsState.informEnable, acsState.informInterval));
    }
    else
    {
        DBGPRINT((stderr, "Did not reset periodic inform: enable<%d>, interval<%d>\n", acsState.informEnable, acsState.informInterval));
    }

    /* MSECS_IN_SEC = 1000 */
    /* stopTimer(periodicInformTimeout, NULL); */
    /* setTimer(periodicInformTimeout,NULL, interval*1000); */
}

void cancelPeriodicInform(void)
{
   /* cancel local periodic inform timer in tr69c */
   stopTimer(periodicInformTimeout, NULL);
}

void scheduleInformTimeout(void *handle __attribute__((unused)))
{
   addInformEventToList(INFORM_EVENT_SCHEDULED);
   addInformEventToList(INFORM_EVENT_SCHEDULE_METHOD);
   sendInform(NULL);
}

void setScheduleInform(UINT32 interval)
{
   CmsRet ret;

   /* TODO */
   ret = cmsTmr_replaceIfSooner(tmrHandle, scheduleInformTimeout, (void*)NULL, interval*1000/*MSECS_IN_SEC*/, "schedule_inform");

   if (ret != CMSRET_SUCCESS)
   {
      DBGPRINT((stderr, "..could not set timer, ret=%d", ret));
   }
}

/*
*
* This can only be called after doDownload() has already sent the Download response.
* It is called by the httpDownload functions if an error occurs.
* Thus we need to send a TransferComplete message here.
 * If the ACS is not connected then an Inform needs to be started.
*/
void sendDownloadFault()
{
   if (isAcsConnected())
   {
	  DBGPRINT((stderr, "sendDownloadFault: ACS is connected -- set transferCompletePending\n"));
      transferCompletePending = 1;
   }
   else
   {
      DBGPRINT((stderr, "sendDownloadFault: ACS is not connected -- sendInform(TransferComplete)\n"));
      transferCompletePending = 1;
      addInformEventToList(INFORM_EVENT_TRANSER_COMPLETE);
      sendInform(NULL);
   }

   /* updateTransferState(acsState.downloadCommandKey,eTransferCompleted); */
}

/** This is only called at tr69c startup from initInformer.
 *
 */
void startACSComm(void *handle  __attribute__((unused)))
{
    /* Initialize server socket for receiving connection requests from ACS. */
    startACSListener();
    sendInform(NULL);
    resetPeriodicInform(acsState.informInterval);
}

/* called once when tr69c starts - must be called AFTER all instances are initialized */
void initInformer(void)
{
    /* add the bootstrap event here. TODO is move to updateTr69cCfgInfo */
    addInformEventToList(INFORM_EVENT_BOOTSTRAP);
    addInformEventToList(INFORM_EVENT_BOOT);

    /* start trying ACSComm in a moment (ACSINFORMDELAY=500ms) */
    setTimer(startACSComm,NULL, ACSINFORMDELAY);
	initNotification();   /* this creates all the InstanceDope descriptors for current */
						  /* instancesf or all parameters */
	restoreNotificationAttr();
	initNotification();   /* this applies all the notification attributes. Retrieve a */
						  /* current copy of the parameter values for any notify params*/
    setCallback(&informState,sendInform, (void *)eIEConnectionRequest ); /* flag to indcate signal from conn Req*/
}

/* This can get moved to bcmBfcWrapper */
void setInformState(eInformState state)
{
	if (informState != state)
	{
		DBGPRINT((stderr, "Changed informState from <%d> to <%d>\n",informState, state));
		informState = state;
		/* saveTR69StatusItems(); */
	}
}

extern TRX_STATUS setMSrvrInformEnable(const char *value);

void shutdownTr69Core()
{
    /* Clean up */
    DBGPRINT((stderr, "CoreShutdown: disable informs\n"));
	setMSrvrInformEnable("0");
    DBGPRINT((stderr, "CoreShutdown: disable periodic timer\n"));
    stopTimer(periodicInformTimeout, NULL);
    DBGPRINT((stderr, "CoreShutdown: disable ACS callback - NOT DISABLED!!\n"));
    /* stopTimer(startACScallback, NULL); need to do cleanup on ACS listener!!!!! */
    DBGPRINT((stderr, "CoreShutdown: ret\n"));
}

/*
 * Sends data out the CTX. A previous call to sendToAcs should have been
 * made to open/prepare the CTX and send the headers. This call does NOT
 * free the buf.
 */
void sendToAcsRaw(int contentLen, char *buf)
{
    tProtoCtx *pc = NULL;

    if (contentLen < 1 || buf == NULL)
    {
        DBGPRINT((stderr, "Try to send RAW message to ACS with bad inputs!\n"));
        return;
    }

    pc = getAcsConnDesc();
    if (pc == NULL)
    {
        DBGPRINT((stderr, "Try to send RAW message to ACS while connection is NULL!\n"));
        return;
    }

    proto_SendRaw(pc, buf, contentLen);
}
