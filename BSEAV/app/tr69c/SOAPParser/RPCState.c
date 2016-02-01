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
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <syslog.h>

#include "../inc/tr69cdefs.h" /* defines for ACS state */
#include "../standard/baselineparams.h"
#include "../inc/appdefs.h"
#include "../main/utils.h"
#include "../main/event.h"
#include "../nanoxml/nanoxml.h"
#include "xmlParserSM.h"
#include "CPEframework.h"
#include "RPCState.h"
#include "xmlTables.h"
#include "../main/informer.h"
#include "../main/httpDownload.h"

/*#include "bcmtypes.h"*/
/*#include "bcmcfm.h"	 */
#include "../bcmLibIF/bcmWrapper.h"

extern void clearInformEventList(void);
extern int isAcsConnected(void);
extern void setInformState(eInformState state);
extern void retrySessionConnection(void);
extern unsigned int addInformEventToList(int event);

/*#define DEBUG*/
/* #define DEBUGXML */

#ifdef DEBUG
#define DBGPRINT(X) fprintf X
#else
#define DBGPRINT(X)
#endif

#define empty(x) ((x) == NULL || strcmp((x), "") == 0)
#define PNAME_LTH   257         /* parameter name length+1 */
#define MORE 0
#define NOMORE 1
/* forwards */

#ifdef DEBUG
static void dumpNode(TRxObjNode *node);
#endif
static const char    *getRPCMethodName(eRPCMethods );
extern void sendInform(void* handle);
extern eSessionState sessionState;
extern eInformState informState;

RPCAction   *rpcAction;
RPCAction   *dloadAction;

/* State of ACS maintained here */
ACSState    acsState;

int			transferCompletePending;
int 		sendGETRPC;

static eACSContactedState	rebootFlag;			/* a TRX_REBOOT has been returned from a setxxx */
static int	saveConfigFlag;		/* save config on disconnect */
int			saveNotificationsFlag;
/* Active Notification paramValueStruct buffer pointer and size */
void activeNotificationImpl(void* handle);
static int	activeNotifications;
static int	notifyChanges;
static char	*valueBuf;
static int	valueBufSz;
#define ACTIVE_NOTIFICATION_TIMEOUT 5000  /* 5 seconds between check for active notifications */

RPCAction* newRPCAction( void )
{
    RPCAction* rpc;
    if ( (rpc = (RPCAction *)malloc(sizeof(RPCAction)))) {
        memset(rpc,0,sizeof(RPCAction));
    }
    return rpc;
}

#define NUMREQINFORMPARMS 7    /* see following array of pointers */
char    *informParameters[NUMREQINFORMPARMS] ={
    "Device.DeviceInfo.HardwareVersion",
    "Device.DeviceInfo.SoftwareVersion",
    "Device.DeviceInfo.ProvisioningCode",
    "Device.ManagementServer.ConnectionRequestURL",
    "Device.ManagementServer.ParameterKey",
    "Device.ManagementServer.AliasBasedAddressing",
    NULL                        /* this is filled in by buildExternalConnParamName() */
};

char    *informEventStr[]={
    "0 BOOTSTRAP",
    "1 BOOT",
    "2 PERIODIC",
    "3 SCHECULED",
    "4 VALUE CHANGE",
    "5 KICKED",
    "6 CONNECTION REQUEST",
    "7 TRANSFER COMPLETE",
    "8 DIAGNOSTICS COMPLETE"
};

static char *getInformEventStr(eInformEvent iev, eRPCMethods rpcMethod)
{
    static char eventstrbuf[30];
    if (/* iev>=eIEBootStrap && */ iev<eIEMethodResult)
        return informEventStr[iev];
    else if (iev==eIEMethodResult && rpcMethod!=rpcUnknown ) {
        snprintf(eventstrbuf,sizeof(eventstrbuf),"M %s",getRPCMethodName(rpcMethod));
        return eventstrbuf;
    }
    eventstrbuf[0]='\0';
    return eventstrbuf;
}

#ifdef DEBUG
void dumpAcsState(void)
{
    fprintf(stderr, "ACS State dump\n");
    fprintf(stderr, "HoldRequests       %d\n", acsState.holdRequests);
    fprintf(stderr, "NoMoreRequest      %d\n", acsState.noMoreRequests);
    fprintf(stderr, "CommandKey(DL)     %s\n", acsState.downloadCommandKey);
    fprintf(stderr, "CommandKey(Reboot  %s\n", acsState.rebootCommandKey);
    fprintf(stderr, "ParameterKey       %s\n", acsState.parameterKey);
    fprintf(stderr, "MaxEnvelopes       %d\n", acsState.maxEnvelopes);
    fprintf(stderr, "RPC Methods supported by ACS:\n");
    fprintf(stderr, "   GetRpcMethods           %s\n", acsState.acsRpcMethods.rpcGetRPCMethods?
            "Yes": "No");
    fprintf(stderr, "   SetParameterValues      %s\n", acsState.acsRpcMethods.rpcSetParameterValues?
            "Yes": "No");
    fprintf(stderr, "   GetParameterValues      %s\n", acsState.acsRpcMethods.rpcGetParameterValues?
            "Yes": "No");
    fprintf(stderr, "   GetParameterNames       %s\n", acsState.acsRpcMethods.rpcGetParameterNames?
            "Yes": "No");
    fprintf(stderr, "   GetParameterAttributes  %s\n", acsState.acsRpcMethods.rpcGetParameterAttributes?
            "Yes": "No");
    fprintf(stderr, "   SetParameterAttributes  %s\n", acsState.acsRpcMethods.rpcSetParameterAttributes?
            "Yes": "No");
    fprintf(stderr, "   Reboot                  %s\n", acsState.acsRpcMethods.rpcReboot?
            "Yes": "No");
    fprintf(stderr, "   FactoryReset            %s\n", acsState.acsRpcMethods.rpcFactoryReset?
            "Yes": "No");
    fprintf(stderr, "   rpcDownload             %s\n", acsState.acsRpcMethods.rpcDownload?
            "Yes": "No");
}
void dumpRpcAction(RPCAction *a)
{
    fprintf(stderr, "RPC description: RPC Method = %s ID=%s\n",
             getRPCMethodName(a->rpcMethod), a->ID);

}
#endif

/* rebootCompletion routine */
/* This is envoked following the ACS response to the rebootresponse msg */
void rebootCompletion(void)
{
	if (rebootFlag>=eACS_DOWNLOADREBOOT && rebootFlag<=eACS_RPCREBOOT) {
		closeAllFds();
		saveTR69StatusItems( &acsState );
		wrapperReboot(rebootFlag);
	}
	return;
}

/* factoryResetCompletion routine */
/* This is envoked following the ACS response to the FactoryResetResponse msg */
static int factoryResetFlag;
void factoryResetCompletion(void)
{
	if (factoryResetFlag){
		wrapperFactoryReset();
		factoryResetFlag = 0;
	}
	return;
}

/* save Configuration routine */
/* This is envoked following the ACS response to the a set/add/delete RPC msg */
void saveConfigurations(void)
{
    if ( saveConfigFlag ){
        wrapperSaveConfigurations();
		saveConfigFlag = 0;
	}
    return;
}



/* Utility routines for data structures */
static const char    *getRPCMethodName(eRPCMethods m)
{
    const char    *t;
    switch (m) {
    case rpcGetRPCMethods:
        t="GetRPCMethods";
        break;
    case rpcSetParameterValues:
        t = "SetParameterValues";
        break;
    case rpcGetParameterValues:
        t="GetParameterValues";
        break;
    case rpcGetParameterNames:
        t="GetParameterNames";
        break;
    case rpcGetParameterAttributes:
        t="GetParameterAttributes";
        break;
    case rpcSetParameterAttributes:
        t="SetParameterAttributes";
        break;
    case rpcAddObject:
        t="AddObject";
        break;
    case rpcDeleteObject:
        t="DeleteObject";
        break;
    case rpcReboot:
        t="Reboot";
        break;
    case rpcDownload:
        t="Download";
        break;
    case rpcFactoryReset:
        t="FactoryReset";
        break;
    case rpcInformResponse:
        t="InformResponse";
        break;
    default:
        t="no RPC methods";
        break;
    }
    return t;
}

static void freeParamItems( ParamItem *item)
{
    ParamItem *next;
    while(item)
    {
        next = item->next;
        free(item->pname);	  /* free data */
        free(item->pvalue);
		free(item->pOrigValue);
		free(item);			  /* free ParamItem */
        item = next;
    }
}


static void freeAttributeItems( AttributeItem *item)
{
    AttributeItem *next;
    while(item){
        next = item->next;
        free(item->pname);
	free(item);
        item = next;
    }
}

static void freeDownloadReq( DownloadReq *r)
{
	free(r->url);
	free(r->user);
	free(r->pwd);
	free(r->fileName);
}
/*
* item is undefined on return
*/
void freeRPCAction(RPCAction *item){
    free(item->ID);
    free(item->parameterKey);
    free(item->commandKey);
	free(item->informID);
    switch(item->rpcMethod){
    case rpcGetParameterNames:
        free(item->ud.paramNamesReq.parameterPath);
        break;
    case rpcSetParameterValues:
    case rpcGetParameterValues:
        freeParamItems(item->ud.pItem);
        break;
    case rpcSetParameterAttributes:
    case rpcGetParameterAttributes:
        freeAttributeItems(item->ud.aItem);
        break;
    case rpcAddObject:
    case rpcDeleteObject:
        free(item->ud.addDelObjectReq.objectName);
        break;
    case rpcDownload:
	freeDownloadReq( &item->ud.downloadReq );
	break;
    default:
        break;
    }
    free(item);
}
/*----------------------------------------------------------------------*
 * memory printf
 */
static void mprintf(char **ps, int *left, const char *fmt, ...)
{
    int n;
    va_list ap;

    va_start(ap, fmt);
    n = vsnprintf(*ps, *left, fmt, ap);
    va_end(ap);
    #ifdef DEBUGXML
    {
        fprintf(stdout, "%s", *ps);
    }
    #endif
    *left -= n;
    *ps += n;
    if (*left < 0) {
	/* out of memory */
	slog(LOG_ERR, "xml: mprintf: out of memory");
	*left = 0;
    acsState.fault = 9004;
	return;
    }
}
/*----------------------------------------------------------------------*/
static void xml_mprintf(char **ps, int *left, char *s)
{
	if (s) {
		for (; *s; s++) {
			switch (*s) {
			case '&':
				mprintf(ps, left, "&amp;");
				break;
			case '<':
				mprintf(ps, left, "&lt;");
				break;
			case '>':
				mprintf(ps, left, "&gt;");
				break;
			case '"':
				mprintf(ps, left, "&quot;");
				break;
			case '\'':
				mprintf(ps,left, "&apos;");
			case 9:
			case 10:
			case 13:
				mprintf(ps, left, "&#%d;", *s);
				break;
			default:
				if (isprint(*s))
					mprintf(ps, left, "%c", *s);
				else
					mprintf(ps,left," ");
				break;
			}
		}
	}
}
#ifdef OMIT_INDENT
#define xml_mIndent(A,B,C) ;
#else
#define xml_mIndent(A,B,C) XMLmIndent( A, B, C);
#endif
/*----------------------------------------------------------------------*/
static void XMLmIndent(char **ps, int *left, int indent)
{
    int i;

    if (indent < 0)
	return;

    indent *= 2;
    for (i = 0; i < indent; i++) {
	mprintf(ps, left, " ");
    }
}


static void closeBodyEnvelope(char **ps, int *lth)
{
    xml_mIndent(ps, lth, 2);
    mprintf(ps, lth, "</%sBody>\n", nsSOAP);
    mprintf(ps, lth, "</%sEnvelope>\n", nsSOAP);
}
/* Add <SOAP:Body>
*/
static void openBody(char **ps, int *lth)
{
    xml_mIndent(ps, lth, 2);
    mprintf(ps, lth, "<%sBody>\n", nsSOAP);
}
/*
* Add <SOAP:Envelope
 *      xmlns:....
 *      <..:Header>
 *      ....
 *      </..:Header>
 * to the buffer
 */
static void openEnvWithHeader(int noMore, char *idstr, char **ps, int *lth)
{
    NameSpace   *ns;
    mprintf(ps, lth, "<%sEnvelope", nsSOAP);
    /* generate Namespace declarations */
    ns = nameSpaces;
    while(ns->sndPrefix){
        char    pbuf[40];
        char    *e;
        mprintf(ps,lth,"\n");
        strncpy(pbuf,ns->sndPrefix, sizeof(pbuf));
        e=strchr(pbuf,':');
        if (e) *e='\0'; /* find : in prefix */
        xml_mIndent(ps,lth, 2);
        mprintf(ps,lth,"xmlns:%s=\"%s\"", pbuf, ns->nsURL);
        ++ns;
    }
    mprintf(ps,lth,">\n");
    if(idstr || noMore){
        xml_mIndent(ps,lth, 2);
        mprintf(ps, lth, "<%sHeader>\n", nsSOAP);
		if (idstr) {
			xml_mIndent(ps,lth, 3);
			mprintf(ps, lth, "<%sID %smustUnderstand=\"1\">%s</%sID>\n",
				   nsCWMP, nsSOAP,idstr , nsCWMP);
		}
        /* add HoldRequest here if needed */
		if (noMore) {
			xml_mIndent(ps,lth, 3);
			mprintf(ps, lth, "<%sNoMoreRequests>1</%sNoMoreRequests>\n",
				   nsCWMP, nsCWMP);
		}
        xml_mIndent(ps,lth, 2);
        mprintf(ps, lth, "</%sHeader>\n",nsSOAP);
    }
}

static const char *getFaultCode( int fault )
{
	const char *r;
    switch(fault){
    case 9000:
    case 9001:
    case 9002:
    case 9004:
    case 9009:
    case 9010:
    case 9011:
    case 9012:
    case 9013:
        r = "Server";
        break;
    case 9003:
    case 9005:
    case 9006:
    case 9007:
    case 9008:
        r = "Client";
        break;
    default:
        r = "Vendor";
        break;
    }
	return r;
}

static const char *getFaultStr( int fault)
{
	const char *detailFaultStr;

    switch(fault){
    case 9000:
        detailFaultStr = "Method not supported";
        break;
    case 9001:
        detailFaultStr = "Request denied";
        break;
    case 9002:
        detailFaultStr = "Internal Error";
        break;
    case 9003:
        detailFaultStr = "Invalid arguments";
        break;
    case 9004:
        detailFaultStr = "Resources Exceeded";
        break;
    case 9005:
        detailFaultStr = "Invalid Parameter Name";
        break;
    case 9006:
        detailFaultStr = "Invalid parameter type";
        break;
    case 9007:
        detailFaultStr = "Invalid parameter value";
        break;
    case 9008:
        detailFaultStr = "Attempt to set a non-writeable parameter";
        break;
    case 9009:
        detailFaultStr = "Notification request rejected";
        break;
    case 9010:
        detailFaultStr = "Download failure";
        break;
    case 9011:
        detailFaultStr = "Upload failure";
        break;
    case 9012:
        detailFaultStr = "File transfer server authentication failure";
        break;
    case 9013:
        detailFaultStr = "Unsupported protocol for file transfer";
		break;
	case 9014:
		detailFaultStr = "MaxEnvelopes exceeded";
        break;
    default:
        detailFaultStr = "Vendor defined fault";
        break;
    }
	return detailFaultStr;
}

static void writeSoapFault( RPCAction *a,char **ps, int *lth, int fault )
{

    openEnvWithHeader(MORE, a->ID, ps, lth);
    openBody(ps,lth);
    xml_mIndent(ps,lth, 3);
    mprintf(ps,lth,"<%sFault>\n", nsSOAP);
    xml_mIndent(ps,lth, 4);
    mprintf(ps,lth,"<faultcode>%s</faultcode>\n",getFaultCode(fault));
    xml_mIndent(ps,lth, 4);
    mprintf(ps,lth,"<faultstring>CWMP fault</faultstring>\n");
    xml_mIndent(ps,lth, 5);
    mprintf(ps,lth,"<detail>\n");
    xml_mIndent(ps,lth, 6);
    mprintf(ps,lth,"<%sFault>\n",nsCWMP);
    xml_mIndent(ps,lth, 7);
    mprintf(ps,lth,"<FaultCode>%d</FaultCode>\n",fault);
    xml_mIndent(ps,lth, 7);
    mprintf(ps,lth,"<FaultString>%s</FaultString>\n",getFaultStr(fault));
    if (a->rpcMethod == rpcSetParameterValues) {
        ParamItem   *pi = a->ud.pItem;
        /* walk thru parameters to generate errors */
		while (pi != NULL ) {
			if (pi->fault) {
				xml_mIndent(ps,lth, 7);
				mprintf(ps,lth,"<SetParameterValuesFault>\n");
				xml_mIndent(ps,lth, 8);
				mprintf(ps,lth,"<ParameterName>%s</ParameterName>\n", pi->pname);
				xml_mIndent(ps,lth, 8);
				mprintf(ps,lth,"<FaultCode>%d</FaultCode>\n", pi->fault);
				xml_mIndent(ps,lth, 8);
				mprintf(ps,lth,"<FaultString>%s</FaultString>\n", getFaultStr(pi->fault));
				xml_mIndent(ps,lth, 7);
				mprintf(ps,lth,"</SetParameterValuesFault>\n");
			}
			pi = pi->next;
		}
    }
    xml_mIndent(ps,lth, 6);
    mprintf(ps,lth,"</%sFault>\n",nsCWMP);
    xml_mIndent(ps,lth, 5);
    mprintf(ps,lth,"</detail>\n");
    xml_mIndent(ps,lth, 3);
    mprintf(ps,lth,"</%sFault>\n", nsSOAP);
    closeBodyEnvelope(ps,lth);
}

static char *doGetRPCMethods(RPCAction *a)
{
    char *buf=(char *)malloc(XMLBUFSZ);
    char *xs = buf;
    int  bufsz = XMLBUFSZ;
    eRPCMethods m;
    if (buf) {
        openEnvWithHeader( MORE, a->ID, &xs, &bufsz);
        openBody(&xs, &bufsz);
        xml_mIndent(&xs,&bufsz, 3);
        mprintf(&xs,&bufsz,"<%sGetRPCMethodsResponse>\n", nsCWMP);
        xml_mIndent(&xs,&bufsz, 4);
		#ifdef SUPPRESS_SOAP_ARRAYTYPE
        mprintf(&xs, &bufsz,"<MethodList>\n");
		#else
        mprintf(&xs, &bufsz,"<MethodList %sarrayType=\"%sstring[%d]\">\n",
                nsSOAP_ENC, nsXSD, LAST_RPC_METHOD );
        #endif

        for(m=rpcGetRPCMethods; m<=LAST_RPC_METHOD; ++m){
            xml_mIndent(&xs,&bufsz, 5);
            mprintf(&xs, &bufsz, "<string>%s</string>\n",
                    getRPCMethodName(m));
        }
        xml_mIndent(&xs, &bufsz, 4);
        mprintf(&xs,&bufsz,"</MethodList>\n");
        xml_mIndent(&xs,&bufsz,3);
        mprintf(&xs,&bufsz,"</%sGetRPCMethodsResponse>\n", nsCWMP);
        closeBodyEnvelope(&xs,&bufsz);
    }
    return buf;
}

typedef void (*SoapOutCB)(TRxObjNode *node, char **buf, int *bufsz, int isFragment);
static char paramName[PNAME_LTH];
static int  paramCnt;
static int  paramCntNumChars;
static void traverseTreeGetParamNames(TRxObjNode *node, SoapOutCB cback, char **buf, int *bufsz, int noDesend, int oparam);

static void writeGetAttribute( TRxObjNode *node, char **bp, int *bufsz, int isFragment)
{
	InstanceDope *pDope = findDopeInstance(node);
	int accessIndex = pDope? pDope->accessListIndex: 0;

	BSTD_UNUSED(isFragment);

    xml_mIndent(bp,bufsz,5);
    mprintf(bp,bufsz,"<ParameterAttributeStruct>\n");
    xml_mIndent(bp,bufsz,6);
    mprintf(bp,bufsz,"<Name>%s</Name>\n", paramName);
    xml_mIndent(bp,bufsz,6);
    mprintf(bp,bufsz,"<Notification>%d</Notification>\n", pDope? pDope->notification: 0);
    xml_mIndent(bp,bufsz,6);                 /*** ?????? lot more here is more than just subscriber access */
    #ifdef SUPPRESS_SOAP_ARRAYTYPE
    mprintf(bp,bufsz,"<AccessList>\n");
    #else
    mprintf(bp,bufsz,"<AccessList %sarrayType=\"%sstring[%d]\">\n", nsSOAP_ENC, nsXSD,
                                                  accessIndex);
	#endif
	if(accessIndex) {
        xml_mIndent(bp,bufsz,7);
        mprintf(bp,bufsz,"<%sstring>%s</%sstring>\n",nsXSD, "Subscriber", nsXSD);
    }
    xml_mIndent(bp,bufsz,6);
    mprintf(bp,bufsz,"</AccessList>\n");
    xml_mIndent(bp,bufsz,5);
    mprintf(bp,bufsz,"</ParameterAttributeStruct>\n");
    ++paramCnt;
}

static void writeGetPName( TRxObjNode *node, char **bp, int *bufsz, int isFragment)
{
    int     writeable = 0;
    int     makeFragName =0;

    xml_mIndent(bp,bufsz,5);
    mprintf(bp,bufsz,"<ParameterInfoStruct>\n");
    if (node->name != instanceIDMASK && node->paramAttrib.attrib.etype==tInstance) {
        /* current node is tInstance. It points to the instance Node */
        if (node->objDetail && ((TRxObjNode *)(node->objDetail))->setTRxParam) {
            writeable = 1;
        }
		/* tc, use 0 since traverseTreeGetParamNames puts the dot */
        makeFragName = 0;                 /* if instance object then make it a name fragment */
    } else {
        writeable = node->setTRxParam!=NULL;
		makeFragName = node->name == instanceIDMASK;
	}
    xml_mIndent(bp,bufsz,6);
    mprintf(bp,bufsz,"<Name>%s%s</Name>\n", paramName, (isFragment&&makeFragName)?".":"");
    xml_mIndent(bp,bufsz,6);
    mprintf(bp,bufsz,"<Writable>%s</Writable>\n", writeable?"1":"0");
    xml_mIndent(bp,bufsz,5);
    mprintf(bp,bufsz,"</ParameterInfoStruct>\n");
    ++paramCnt;

    /* printf("WriteGetP: paramCnt<%d> paramCntNumChars<%d>  pname<%s>  bufsz<%d>\n",paramCnt, startbz-(*bufsz),paramName, *bufsz ); */
}

/* Used as the callback from traverseTree. This will get the number of objects */
/* and the number of chars that will be in the message. This includes the values. */
static void writeGetPValuesCountOnly( TRxObjNode *node, char **bp, int *bufsz, int isFragment)
{
    /* paramCntNumChars is a global */
    /* paramName is a global */
    /* paramCnt is a global */

	BSTD_UNUSED(bp);
	BSTD_UNUSED(bufsz);
	BSTD_UNUSED(isFragment);

    if ( node->name==instanceIDMASK)
        return;       /* instance node : no value to get */

    /* Get a count of the number of chars we will need */
#if 0
    paramCntNumChars += 12   //xml_mIndent(bp,bufsz, 6);
    paramCntNumChars += 24   //mprintf(bp,bufsz,"<ParameterValueStruct>\n");
    paramCntNumChars += 14   //xml_mIndent(bp,bufsz, 7);
    paramCntNumChars += 14   //mprintf(bp,bufsz,"<Name>%s</Name>\n", paramName);
    paramCntNumChars += 14   //xml_mIndent(bp,bufsz, 7);
    paramCntNumChars += 24   //mprintf(bp,bufsz,"<Value %stype=\"%s%s\">",nsXSI, nsXSD,
                         7   //assume boolean type                       getValTypeStr(n->paramAttrib.attrib.etype));
    paramCntNumChars +=            //xml_mprintf(bp,bufsz,val);
    paramCntNumChars +=  in above  //mprintf(bp,bufsz,"</Value>\n");
    paramCntNumChars += 12         //xml_mIndent(bp,bufsz, 6);
    paramCntNumChars += 24         //mprintf(bp,bufsz,"</ParameterValueStruct>\n");
#endif

    paramCntNumChars += 145; /* note: indent takes 2 */
    /* if (paramName) */
        paramCntNumChars += strlen(paramName);

    /* Add the length of the value */
    if (node)
    {
        if (node->getTRxParam)
        {
            char *val = NULL;
            if (node->getTRxParam(&val) == TRX_OK)
                paramCntNumChars += strlen(val);
        }
    }

    ++paramCnt;

    /* printf("WriteGetPnameOnly: paramCnt<%d> paramCntNumChars<%d>  pname<%s>\n",paramCnt, paramCntNumChars,paramName ); */
}

/* Used as the callback from traverseTreeGetParamNames. This will get the number of objects */
/* and the number of chars that will be in the message. */
static void writeGetPNameCountOnly( TRxObjNode *node, char **bp, int *bufsz, int isFragment)
{
    /* paramCntNumChars is a global */
    /* paramName is a global */
    /* paramCnt is a global */

    /* Get a count of the number of chars we will need */
#if 0
    paramCntNumChars += 5;  10   //xml_mIndent(bp,bufsz,5);
    paramCntNumChars += 23; 21 //mprintf(bp,bufsz,"<ParameterInfoStruct>\n");
    paramCntNumChars += 6;  12  //xml_mIndent(bp,bufsz,6);
    paramCntNumChars += 6;   //mprintf(bp,bufsz,"<Name>\n");
    paramCntNumChars += strlen(paramName);
    paramCntNumChars += 1;  // assume a dot so add 1     //isFragment&&makeFragName)?".":"");
    paramCntNumChars += 9;  7  // </Name>  //mprintf(bp,bufsz,"<Name>%s%s</Name>\n", paramName, (isFragment&&makeFragName)?".":"");
    paramCntNumChars += 6;  12  //xml_mIndent(bp,bufsz,6);
    paramCntNumChars += 24; 22  //mprintf(bp,bufsz,"<Writable>%s</Writable>\n", writeable?"1":"0");
    paramCntNumChars += 5;  10  //xml_mIndent(bp,bufsz,5);
    paramCntNumChars += 24; 22   //mprintf(bp,bufsz,"</ParameterInfoStruct>\n");
#endif
	BSTD_UNUSED(node);
	BSTD_UNUSED(bp);
	BSTD_UNUSED(bufsz);
	BSTD_UNUSED(isFragment);

    paramCntNumChars += 123; /* 109 note: indent takes 2 */
    /* if (paramName) */
        paramCntNumChars += strlen(paramName);

    ++paramCnt;

    /* printf("WriteGetPnameOnly: paramCnt<%d> paramCntNumChars<%d>  pname<%s>\n",paramCnt, paramCntNumChars,paramName ); */
}


/*
* Input:
 * node: Current node to link from
 * cback; Output routine to display node information
 * buf:   *pointer to next available buffer space.
 * bufsz: pointner to size remaining in buffer(buf).
 * noDescend: Just walk next level of tree- don't desend if true
 * oparam: Parameter passed to output (cback) function as parameter 4
 *
 * If the node->name[0] is '#' then the node is ignored by the tree traversal.
 * This name allows pseudo parameter names to be added to the paraemter tree
 * to be used to allow calling of getter/setter functions that are not directly related
 * to  TR069 parameters. These routines are accessed by the CPEFramework searchTree
 * functions.
*/
static void traverseTree(TRxObjNode *node, SoapOutCB cback, char **buf, int *bufsz, int noDesend, int oparam)
{
    InstanceDesc *idp;
    TRxObjNode  *n = node->objDetail;
    int     pNameEnd, plth;

	pNameEnd = plth = strlen(paramName);

    if ( paramName[pNameEnd-1]!='.' && plth<(PNAME_LTH-1))
        strcpy( &paramName[pNameEnd++], ".");

    if (n!=NULL && n->name==instanceIDMASK) {
        /* this is an object instance -- step thru the instances if present */
        idp = n->paramAttrib.instance;
		while (idp!=NULL && !acsState.fault) {
			if (checkInstancePath(idp)) {
				pushInstance(idp);
				snprintf(&paramName[pNameEnd],PNAME_LTH-pNameEnd,"%d", idp->instanceID);
				(cback)(n,buf,bufsz, oparam);
				if (!noDesend)
					traverseTree(n,cback,buf,bufsz, 0, oparam);
				popInstance();
			}
			/* truncate paramName */
			paramName[pNameEnd] = '\0';
			idp = idp->next;
		}
        return;
    }
    while ( n!=NULL && n->name!=NULL && !acsState.fault ) {
		if (n->name[0] != '#') { /* check for pseudo parameter */
			strncpy( &paramName[pNameEnd], n->name, PNAME_LTH-pNameEnd);
			if (n->objDetail==NULL) {
				(cback)(n, buf, bufsz, oparam);   /* Call output function*/
			} else if (!noDesend) {
				/* add current object name to paramName */
				traverseTree(n,cback,buf,bufsz, 0, oparam);
				/* truncate paramName */
				paramName[pNameEnd] = '\0';
			} else if (n->paramAttrib.attrib.etype == tInstance) {
				(cback)(n, buf, bufsz, oparam);   /* special test to get instance write indicator*/
			}                                     /* to indicate AddObject/DeleteObject capability*/
		}
		++n;
    }
    return;
}

/*
* GetParameterNames requests a single parameter path or single parameter path fragment
*/
#if 1  /* original function */
static char *doGetParameterNames(RPCAction *a)
{
    char *buf=(char *)malloc(XMLBUFSZ);
    char *xs = buf;
    int  bufsz = XMLBUFSZ;
    int isFragment;

    if (buf)
    {
        TRxObjNode  *n;
        const char  *pp = NULL;

        if (a != NULL)
            pp = a->ud.paramNamesReq.parameterPath;

        if ( pp==NULL || strlen(pp)==0)
        {
            n = rootDevice;
            pp = n->name;
            isFragment = 1;
        }
        else
        {
            n = findGWParameter(pp);
            isFragment = pp[strlen(pp)-1]=='.';
            /* printf("GPN: found parameter <%s>, isFragment <%d>\n",n->name,isFragment); */
        }

        if (n)
        {
            /* path found: generate response */
            char *plist;
            int plistsz = PARAMLISTSZ;
            char *xbufp;
            int nextLevel;
            nextLevel = 0;


            if (n->objDetail==NULL && (a->ud.paramNamesReq.nextLevel || isFragment))
            {
                /* invalid paramter */
                acsState.fault = 9003;
            }
            else
            {
                /* printf("\n**DGPN: what is a->ID!  = <%s>\n",a->ID); */

                openEnvWithHeader( MORE, a->ID, &xs, &bufsz);
                openBody(&xs, &bufsz);
                xml_mIndent(&xs,&bufsz, 3);
                mprintf(&xs,&bufsz,"<%sGetParameterNamesResponse>\n", nsCWMP);
                plist = (char *)malloc(PARAMLISTSZ);
                if (plist == NULL)
                {
                    DBGPRINT((stderr, "\n**DGPN: ERROR!!! malloc failed for plist \n"));
                    return NULL;
                }
                xbufp = plist;
                paramCnt=0;
                strncpy(paramName, pp, PNAME_LTH   );

                if (a != NULL)
                    nextLevel = a->ud.paramNamesReq.nextLevel;
                else
                    nextLevel = 0;

                if (isFragment )
                {
                    /* traverseTree( n, writeGetPName, &xbufp, &plistsz, a->ud.paramNamesReq.nextLevel, isFragment); */
                    /* printf("GPN:- we are a node so recurse with another call to traverseTreeGpn\n"); */
                    traverseTreeGetParamNames( n, writeGetPName, &xbufp, &plistsz, nextLevel, isFragment);
                }

                else
                {
                    writeGetPName(n, &xbufp, &plistsz, 0);
                }
                xml_mIndent(&xs,&bufsz,4);
#ifdef SUPPRESS_SOAP_ARRAYTYPE
                mprintf(&xs,&bufsz,"<ParameterList>\n");
#else
                mprintf(&xs,&bufsz,"<ParameterList %sarrayType=\"%sParameterInfoStruct[%d]\">\n",
                        nsSOAP_ENC, nsCWMP, paramCnt);
#endif
                if (paramCnt>0)
                    mprintf(&xs,&bufsz, plist);  /* copy parameter list */

                /* printf("\n**DGPN: about to free plist, plist has len <%d> and size <%d>\n\n", strlen(plist), sizeof(plist)); */

                free (plist);
                xml_mIndent(&xs, &bufsz, 4);
                mprintf(&xs,&bufsz,"</ParameterList>\n");
                xml_mIndent(&xs,&bufsz,3);
                mprintf(&xs,&bufsz,"</%sGetParameterNamesResponse>\n", nsCWMP);
                closeBodyEnvelope(&xs,&bufsz);
            }
        }
        else
        {
            acsState.fault = 9005; /* Invalid Parameter Name */
        }
        if ( acsState.fault != 0)
        {
            /* reset buffer and build fault here */
            xs = buf;
            bufsz = XMLBUFSZ;
            writeSoapFault(a,&xs,&bufsz, acsState.fault);
            DBGPRINT((stderr, "Fault in GetParameterNames <%d>\n", acsState.fault));
        }
    }
    #if 0
    if (buf)
        printf("\n**DGPN: doGetParameterNames ret buf <%p> with len <%d> bufsize <%d>\n\n", buf, strlen(buf), sizeof(buf));
    else
        printf("\n**DGPN: buf is NULL - it must have failed the malloc!!\n");

    //BcmHeapBoundsCheck();
    #endif
    return buf;
}
#else
/* a better way using less buffer */
static char *doGetParameterNames(RPCAction *a)
{
    char *buf = NULL;  /* =(char *)malloc(XMLBUFSZ); */
    char *xs  = NULL;  /* = buf; */
    int   bufsz;       /* = XMLBUFSZ; */
    int   isFragment;

    TRxObjNode  *n;
    const char  *pp = NULL;

    if (a != NULL)
        pp = a->ud.paramNamesReq.parameterPath;

    if ( pp==NULL || strlen(pp)==0)
    {
        n = rootDevice;
        pp = n->name;
        isFragment = 1;
    }
    else
    {
        n = findGWParameter(pp);
        isFragment = pp[strlen(pp)-1]=='.';
        /* printf("GPN: found parameter <%s>, isFragment <%d>\n",n->name,isFragment); */
    }

    /* The node is given or root, we have our starting point, now go with it. */
    if (n)
    {
        /* path found: generate response */
        int nextLevel;

        if (a != NULL)
            nextLevel = a->ud.paramNamesReq.nextLevel;
        else
            nextLevel = 0;

        if (n->objDetail==NULL && (a->ud.paramNamesReq.nextLevel || isFragment))
        {
            /* invalid parameter */
            acsState.fault = 9003;
        }
        else
        {
            /* 1) First do a check to see how many objects we will have and the approx */
            /*    size of buffer that we need. */
            char *dummyBuf = "dummyBuffer\n";
            int   dummyBufsz = 11;
            paramCnt=0;
            paramCntNumChars = 0;
            traverseTreeGetParamNames( n, writeGetPNameCountOnly, &dummyBuf, &dummyBufsz, nextLevel, isFragment);
            DBGPRINT((stderr ,"doGetParameterNames - Pre-check for size found objects<%d> and numChars<%d>\n\n", paramCnt, paramCntNumChars));

            /* 2) Size our buffer to hold the complete message */
            /*    The computed buffer size plus some overhead for the rest of the message */
            int preCheckSize = paramCntNumChars + 7000;
            if (preCheckSize > 125*1024) /* make sure it is reasonable */
            {
                DBGPRINT((stderr, "\nFault in GetParameterNames: Message size too big<%d>\n\n", preCheckSize));
                preCheckSize = 1111; /* this is an error and our messge will be truncated!!!!!! */
            }
            buf=(char *)malloc(preCheckSize);
            xs = buf;
            bufsz = preCheckSize;
            /* printf("\ndoGetParameterNames - Using a buffer size<%d>\n", preCheckSize); */

            /* 3) Start our message with some envelope stuff */

            /* printf("\n**DGPN: what is a->ID!  = <%s>\n",a->ID); */
            openEnvWithHeader( MORE, a->ID, &xs, &bufsz);
            openBody(&xs, &bufsz);
            xml_mIndent(&xs,&bufsz, 3);
            mprintf(&xs,&bufsz,"<%sGetParameterNamesResponse>\n", nsCWMP);

            /* 4) Continue by adding the correct SOAP to finish up */
            xml_mIndent(&xs,&bufsz,4);
            #ifdef SUPPRESS_SOAP_ARRAYTYPE
            mprintf(&xs,&bufsz,"<ParameterList>\n");
            #else
            mprintf(&xs,&bufsz,"<ParameterList %sarrayType=\"%sParameterInfoStruct[%d]\">\n",
                    nsSOAP_ENC, nsCWMP, paramCnt);
            #endif

            strncpy(paramName, pp, PNAME_LTH   );

            if (a != NULL)
                nextLevel = a->ud.paramNamesReq.nextLevel;
            else
                nextLevel = 0;

            /* 5) At this spot we are ready for our list of names. The traverseTreeGetParamNames func */
            /*    will put the formatted names in our buffer at this place (in the buffer). */
            if (isFragment )
            {
                paramCnt=0;
                paramCntNumChars = 0;
                traverseTreeGetParamNames( n, writeGetPName, &xs, &bufsz , nextLevel, isFragment);
            }
            else
            {
                writeGetPName(n, &xs, &bufsz, 0);
            }

            /* 6) Close */
            xml_mIndent(&xs, &bufsz, 4);
            mprintf(&xs,&bufsz,"</ParameterList>\n");
            xml_mIndent(&xs,&bufsz,3);
            mprintf(&xs,&bufsz,"</%sGetParameterNamesResponse>\n", nsCWMP);
            closeBodyEnvelope(&xs,&bufsz);
        }
    }
    else
    {
        acsState.fault = 9005; /* Invalid Parameter Name */
    }

    if ( acsState.fault != 0)
    {
        /* reset buffer and build fault here */
        buf=(char *)malloc(2048);  /* (XMLBUFSZ); */
        xs = buf;
        bufsz = 4096;             /* XMLBUFSZ; */
        writeSoapFault(a,&xs,&bufsz, acsState.fault);

        DBGPRINT((stderr, "Handled Fault in GetParameterNames <%d>\n", acsState.fault));
    }

    if (buf)
    {
        DBGPRINT((stderr, "DGPN: doGetParameterNames ret buf <%p> with len <%d> bufsize margin plus<%d>\n\n", buf, strlen(buf), bufsz));
    }
    else
    {
        DBGPRINT((stderr, "DGPN: buf is NULL - it must have failed the malloc!!\n"));
    }

    /* BcmHeapBoundsCheck(); */

    return buf;
}
#endif


/*
* set the Attributes in the DopeDescriptor for the Node,instance.
 * Returns: 0 or fault value.
 *
*/
/*static int setAttributes( TRxObjNode *n, AttributeItem *pi )*/
static void setAttributes( TRxObjNode *n, char **p, int *ret, int fragment)
{
	InstanceDope	*pDope;
	AttributeItem *pi = (AttributeItem *)p;
	*ret = 0;

	if ( (pDope=findDopeInstance(n))) {
		/* found instance dope */
		if (pi->chgNotify ){
			if (!(pi->notification==ACTIVE_NOTIFICATION
				  && n->paramAttrib.attrib.inhibitActiveNotify))
				pDope->notification = pi->notification;
			else if (!fragment)
				*ret=9009;
		}
		if (pi->chgAccess)
			pDope->accessListIndex = pi->subAccess;
	} else
		*ret=9002;
}
static char *doSetParameterAttributes(RPCAction *a)
{

    char *buf=(char *)malloc(XMLBUFSZ);
    char *xs = buf;
    int  bufsz = XMLBUFSZ;
    if (buf) {
        AttributeItem   *pi = a->ud.aItem;
        /* first set attributes */
        while (pi!=NULL && !acsState.fault ) {
			int     isFragment = 0;
			TRxObjNode  *n;
			const char        *pp = pi->pname;
			if ( pp==NULL || strlen(pp)==0){
				n = rootDevice;
				pp = n->name;
				isFragment = 1;
			}
			else {
				n = findGWParameter(pp);
				isFragment = pp[strlen(pp)-1]=='.';
			}
			if (n) {
				/* path found: apply attribute change */
				int status=0;
				if (n->name!=instanceIDMASK && n->paramAttrib.attrib.etype!=tInstance )
					setAttributes(n, (char **)pi, &status , isFragment);
				if (isFragment)
					/*traverseTreeAttributes( n, pi, isFragment);*/
					traverseTree(n, setAttributes, (char **)pi, &status, 0, isFragment);
				acsState.fault = status;
			} else {
				acsState.fault = 9005; /* invalid parameter name */
			}
			pi = pi->next;
		}
        if (!acsState.fault) {
			saveNotificationsFlag = 1;
            /* build good response */
            openEnvWithHeader( MORE, a->ID, &xs, &bufsz);
            openBody(&xs, &bufsz);
            xml_mIndent(&xs,&bufsz, 3);
            mprintf(&xs,&bufsz,"<%sSetParameterAttributesResponse/>\n", nsCWMP);
            closeBodyEnvelope(&xs,&bufsz);
        } else {
            writeSoapFault(a,&xs,&bufsz, acsState.fault);
            #ifdef DEBUG
            fprintf(stderr, "Fault in SetParameterAttribute %d\n", acsState.fault);
            #endif
        }
    }
    return buf;
}

/*
* GetParameterAttributes requests a single parameter path or single parameter path fragment
* This RPC uses the paramItem union member.
*/
static char *doGetParameterAttributes(RPCAction *a)
{

    char *buf=(char *)malloc(XMLBUFSZ);
    char *xs = buf;
    int  bufsz = XMLBUFSZ;
    ParamItem   *pi = a->ud.pItem;
    int cnt = 0;    /* parameter counter -- need to count fragment name expansion */
    char *plist;                    /* buffer for param-value */
    int plistsz = PARAMLISTSZ;
    const char *pname = NULL; /* pointer to bad name */
    char *xbufp;

    plist = (char *)malloc(PARAMLISTSZ);
    xbufp = plist;
    if (buf) {
        if (pi!= NULL) {
            /* create response msg start */
            openEnvWithHeader(MORE, a->ID, &xs, &bufsz);
            openBody(&xs, &bufsz);
            xml_mIndent(&xs,&bufsz, 3);
            mprintf(&xs,&bufsz,"<%sGetParameterAttributesResponse>\n", nsCWMP);
            /* step thru the requested parameter list some may be parameter fragments. */
            while (pi!=NULL && !acsState.fault ) {
                int     isFragment = 0;
                TRxObjNode  *n;
                const char        *pp = pi->pname;
                if ( pp==NULL || strlen(pp)==0){
                    n = rootDevice;
                    pp = n->name;
                    isFragment = 1;
                }
                else {
                    n = findGWParameter(pp);
                    isFragment = pp[strlen(pp)-1]=='.';
                }
                if (n) {
                    /* path found: generate response */
                    paramCnt=0;
                    strncpy(paramName, pp, PNAME_LTH   );
                    if (isFragment ) {
                        traverseTree( n, writeGetAttribute, &xbufp, &plistsz, 0, isFragment);
                        cnt += paramCnt;
                    } else {
                        writeGetAttribute(n, &xbufp, &plistsz, 0);
                        ++cnt;
                    }
                } else {
                    pname = pp;
                    acsState.fault = 9005; /* invalid parameter name */
                }
                pi = pi->next;
            }
        }else
            acsState.fault = 9003; /* no parameter specified - Invalid arguments */
        if (!acsState.fault) {
            /* good response */
            xml_mIndent(&xs,&bufsz,4);
			#ifdef SUPPRESS_SOAP_ARRAYTYPE
			mprintf(&xs,&bufsz,"<ParameterList>\n");
			#else
            mprintf(&xs,&bufsz,"<ParameterList %sarrayType=\"%sParameterAttributeStruct[%d]\">\n",
                 nsSOAP_ENC, nsCWMP, cnt);
			#endif
			if (cnt>0)
				mprintf(&xs,&bufsz, plist);  /* copy parameter list */
            free (plist);
            xml_mIndent(&xs, &bufsz, 4);
            mprintf(&xs,&bufsz,"</ParameterList>\n");
            xml_mIndent(&xs,&bufsz,3);
            mprintf(&xs,&bufsz,"</%sGetParameterAttributesResponse>\n", nsCWMP);
            closeBodyEnvelope(&xs,&bufsz);
        } else {
            /* reset buffer and build fault here */
            xs = buf;
            bufsz = XMLBUFSZ;
            writeSoapFault(a,&xs,&bufsz, acsState.fault);
            #ifdef DEBUG
            fprintf(stderr, "Fault in GetParameterAttribute %d\n", acsState.fault);
            #endif
        }
    }
    return buf;
}

static char *doSetParameterValues(RPCAction *a)
{
	int	setParamReboot = 0;
    char *buf =(char *)malloc(XMLBUFSZ);
    char *xs = buf;
    int  bufsz = XMLBUFSZ;
    char *pname = NULL;

    if (buf) {
        TRxObjNode  *n;
        ParamItem   *pi = a->ud.pItem;
        acsState.fault = 0;		/* Init to no fault */
        /* first set parameters */
        if (pi!= NULL) {
            while (pi!=NULL) {
                TRX_STATUS  stat;
                pname = pi->pname; /* save name for fault msg */
                if ( (n=findGWParameter(pname)) ){
                    /* found parameter node */
                    if (n->setTRxParam!= NULL){
						if (empty(pi->pvalue) && n->paramAttrib.attrib.etype!=tString){
							pi->fault = 9007;
							acsState.fault = TRX_INVALID_ARGUMENTS; /*9003, no parameter specified - Invalid arguments */
						}else {
							/* first save value for restore */
                            if (n->getTRxParam) {
                                if ( pi->pOrigValue != NULL ) {
                                	free(pi->pOrigValue);
									pi->pOrigValue = NULL;
								}
								/* ignore errors here */
								n->getTRxParam( &pi->pOrigValue );
                            }
                            if (pi->pvalue==NULL) /* only valid for tSting params */
								/* fake up a null string to avoid NULL ptr problem*/
								pi->pvalue = strdup("");
							if ((n->setTRxParam != NULL) &&
								((stat = n->setTRxParam(pi->pvalue))!=TRX_ERR)) {
								if( stat==TRX_REBOOT )
									setParamReboot = 1;
								pi->fault = 0;
							}
							else {
								slog(LOG_ERR, "Error setting %s to value %s, message %s\n", pname, pi->pvalue, getFaultStr(pi->fault));
								/*pi->fault = acsState.fault = 9002; *//* Use Internal error for now */
								pi->fault = acsState.fault = n->setTRxParam(pi->pvalue); /* Catch actual return code for setTrxParam function */
							}
						}
                    } else{
						pi->fault = 9008;  /* non-writeable param */
                        acsState.fault = 9003;
					}
                } else {
					pi->fault = 9005; /* invalid parameter name */
                    acsState.fault = TRX_INVALID_ARGUMENTS; /*9003, no parameter specified - Invalid arguments */
				}
                pi = pi->next;
            }
			/* if fault above then restore orig parameter values */
			if (acsState.fault) {
                pi = a->ud.pItem;
				while (pi!=NULL ) {
					pname = pi->pname; /* save name for fault msg */
					if ( (n=findGWParameter(pname)) ){
						/* found parameter node */
						if (n->setTRxParam!= NULL){
							if (pi->pOrigValue==NULL)
								pi->pOrigValue = strdup("");
							n->setTRxParam(pi->pOrigValue);
						}
					}
					pi = pi->next;
				}
			}
        }else
            acsState.fault = TRX_INVALID_ARGUMENTS; /*9003, no parameter specified - Invalid arguments */
        if (!acsState.fault) {
            /* build good response */
            openEnvWithHeader(rebootFlag?NOMORE:MORE, a->ID, &xs, &bufsz);
            openBody(&xs, &bufsz);
            xml_mIndent(&xs,&bufsz, 3);
            mprintf(&xs,&bufsz,"<%sSetParameterValuesResponse>\n", nsCWMP);
            xml_mIndent(&xs,&bufsz, 4);
            if ( setParamReboot )
                rebootFlag = eACS_SETVALUEREBOOT;
            mprintf(&xs,&bufsz,"<Status>%s</Status>\n", rebootFlag?"1": "0");
            xml_mIndent(&xs,&bufsz, 3);
            mprintf(&xs,&bufsz,"</%sSetParameterValuesResponse>\n", nsCWMP);
            closeBodyEnvelope(&xs,&bufsz);
			saveConfigFlag = 1;
        } else {
            writeSoapFault(a,&xs,&bufsz, acsState.fault);
            #ifdef DEBUG
            fprintf(stderr, "Fault in SetParameterValue %d for%s\n", acsState.fault, pname);
            #endif
        }
    }
    return buf;
}
/*
* write SOAP value with type at buffer location **bp. Limit to size *bufsz.
 * increments paramCnt for each value written to buffer.
 *
*/
static void writeParamValueStruct( TRxObjNode *n, char *val, char **bp, int *bufsz)
{
	/* now fill in ParameterValueStruct in response */
	xml_mIndent(bp,bufsz, 6);
	mprintf(bp,bufsz,"<ParameterValueStruct>\n");
	xml_mIndent(bp,bufsz, 7);
	mprintf(bp,bufsz,"<Name>%s</Name>\n", paramName);
	xml_mIndent(bp,bufsz, 7);

	mprintf(bp,bufsz,"<Value %stype=\"%s%s\">",nsXSI, nsXSD,
			getValTypeStr(n->paramAttrib.attrib.etype));
	#ifdef SUPPRESS_EMPTY_PARAM
	if (empty(val))
		xml_mprintf(bp,bufsz,"empty");
	else
		xml_mprintf(bp,bufsz,val);
	#else
	xml_mprintf(bp,bufsz,val);
	#endif
	mprintf(bp,bufsz,"</Value>\n");
	#ifdef DEBUG
	fprintf(stderr, "%s=%s %s\n",paramName, getValTypeStr(n->paramAttrib.attrib.etype),val?val:"NULL");
	#endif
	xml_mIndent(bp,bufsz, 6);
	mprintf(bp,bufsz,"</ParameterValueStruct>\n");
	++paramCnt;
	return;
}

static void writeGetPValue( TRxObjNode *n, char **bp, int *bufsz, int isFragment)
{
    if ( n->name==instanceIDMASK)
        return;       /* instance node : no value to get */
    if (n->getTRxParam!= NULL){
        char    *val = NULL; /* getTRxParam allocated value buffer -- need to free it*/
				            /* init to NULL in case getter returns OK but no data*/
		#ifdef DEBUG
		fprintf(stderr, "> %s =", paramName);
		#endif
		if (n->paramAttrib.attrib.etype == tStringSOnly) {
			/* used to protect passwords, etc.Return null string */
			writeParamValueStruct(n,"",bp,bufsz);
		}
        else if (n->getTRxParam(&val)!=TRX_OK){
            acsState.fault = n->getTRxParam(&val); /* Return error code accordingly with TR69 specs */
			/*slog(LOG_ERR, "Error getting param value for %s\n", paramName);*/
			slog(LOG_ERR, "Error getting param value for %s, message %s\n", paramName, getFaultStr(acsState.fault));
		}
        else {
            /* now fill in ParameterValueStruct in response */
			writeParamValueStruct(n,val,bp,bufsz);
            free(val); /* clean up after getTRxParam */
        }
    } else if (!isFragment )   /* if not walking tree then   */
        acsState.fault = TRX_INVALID_PARAM_NAME/*9005*/; /* non-existant read function */
}

/* static int gRecursiveNumber; */
#if 1  /* original function */
static char *doGetParameterValues(RPCAction *a)
{

    char *buf=(char *)malloc(XMLBUFSZ);
    char *xs = buf;
    int  bufsz = XMLBUFSZ;
    ParamItem   *pi = a->ud.pItem;
    char *plist;                    /* buffer for param-value */
    int plistsz = PARAMLISTSZ;
    const char *pname = NULL; /* pointer to bad name */
    char *xbufp;

    plist = (char *)malloc(PARAMLISTSZ);
    xbufp = plist;
    if (buf) {
        if (pi!= NULL) {

            /* create response msg start */
            openEnvWithHeader( MORE, a->ID, &xs, &bufsz);
            openBody(&xs, &bufsz);
            xml_mIndent(&xs,&bufsz, 3);
			paramCnt=0;
            mprintf(&xs,&bufsz,"<%sGetParameterValuesResponse>\n", nsCWMP);
            /* step thru the requested parameter list some may be parameter fragments. */
            while (pi!=NULL && !acsState.fault ) {
                int     isFragment = 0;
                TRxObjNode  *n;
                const char        *pp = pi->pname;
                if ( pp==NULL || strlen(pp)==0){
                    n = rootDevice;
                    pp = n->name;
                    isFragment =1;
                }
                else {
                    n = findGWParameter(pp);
                    isFragment = pp[strlen(pp)-1]=='.';
                }
                if (n) {
                    /* path found: generate response */
                    strncpy(paramName, pp, PNAME_LTH   );
                    if (isFragment )
                        traverseTree( n, writeGetPValue, &xbufp, &plistsz, 0, isFragment);
                    else if (n->name!=instanceIDMASK)
                        writeGetPValue(n, &xbufp, &plistsz, 0);
                    else
						acsState.fault = TRX_INVALID_PARAM_NAME; /*9005, invalid parameter name */ /* can't get from an instance */
                } else {
                    pname = pp;
                    acsState.fault = TRX_INVALID_PARAM_NAME; /*9005, invalid parameter name */
                }
                pi = pi->next;
            }
        }else
            acsState.fault = TRX_INVALID_ARGUMENTS; /*9003, no parameter specified - Invalid arguments */
        if (!acsState.fault) {
            /* good response */
            xml_mIndent(&xs,&bufsz,4);
            #ifdef SUPPRESS_SOAP_ARRAYTYPE
            mprintf(&xs,&bufsz,"<ParameterList>\n");
			#else
            mprintf(&xs,&bufsz,"<ParameterList %sarrayType=\"%sParameterValueStruct[%d]\">\n",
                 nsSOAP_ENC, nsCWMP, paramCnt);
			#endif
			if (paramCnt>0)
				mprintf(&xs,&bufsz, plist);  /* copy parameter list */
			xml_mIndent(&xs,&bufsz, 4);
            mprintf(&xs,&bufsz,"</ParameterList>\n");
            xml_mIndent(&xs,&bufsz, 3);
            mprintf(&xs,&bufsz,"</%sGetParameterValuesResponse>\n", nsCWMP);
            closeBodyEnvelope(&xs,&bufsz);
        } else {
            /* reset buffer and build fault here */
            xs = buf;
            bufsz = XMLBUFSZ;
            writeSoapFault(a,&xs,&bufsz, acsState.fault);
            DBGPRINT((stderr, "Fault in GetParameterValue %d\n", acsState.fault));
        }
    }
    free(plist);
    return buf;
}
#else
/* a better way using less buffer */
static char *doGetParameterValues(RPCAction *a)
{
    char *buf = NULL; /* (char *)malloc(XMLBUFSZ); */
    char *xs = buf;
    int   bufsz = 0;  /* XMLBUFSZ; */
    int   isFragment = 0;
    ParamItem   *pi = NULL;

    TRxObjNode  *n;
    const char  *pp = NULL;


    if (a != NULL)
        pi = a->ud.pItem;

    if (a == NULL || pi == NULL)
    {
        acsState.fault = 9003; /* no parameter specified - Invalid arguments */
    }
    else
    {
        /* First do a check to see how many objects we will have and the approx */
        /* size of buffer that we need. */
        char *dummyBuf = "dummyBuffer\n";
        int   dummyBufsz = 11;
        int   nextLevel = 0;
        paramCnt=0;
        paramCntNumChars = 0;

        while (pi!=NULL && !acsState.fault )
        {
            pp = pi->pname;
            if ( pp == NULL || strlen(pp)==0)
            {
                n = rootDevice;
                pp = n->name;
                isFragment = 1;
            }
            else
            {
                n = findGWParameter(pp);
                isFragment = pp[strlen(pp)-1]=='.';
            }

            if (n)
            {
                strncpy(paramName, pp, PNAME_LTH   );
                /* printf("calling travesetree with name <%s>\n",n->name); */
                if (isFragment )
                {
                    traverseTree( n, writeGetPValuesCountOnly, &dummyBuf, &dummyBufsz, nextLevel, isFragment);
                    DBGPRINT((stderr,"doGetParameterValues - Pre-check for size found objects<%d> and numChars<%d>\n", paramCnt, paramCntNumChars));
                }
                else if (n->name!=instanceIDMASK)
                {
                    /* writeGetPValue(n, &xbufp, &plistsz, 0); */
                    /* printf("++++++++++ IN the PRE TEST while instanceIDMASK  n->name<%s>\n",n->name); */
                    paramCnt += 1;
                    paramCntNumChars += 300;
                }
                else
                {
                    /* printf("++++++++++ IN the PRE TEST  FAULT 9005\n"); */
                    acsState.fault = 9005; /* can't get from an instance */
                }
            }
            else
            {
                acsState.fault = 9005; /* invalid parameter name */
            }
            pi = pi->next;
        }
    }

    /* printf("Starting step 2 fault<%d>)\n",acsState.fault); */

    /* Step 2. We have the number of objects and good approximation of the data size. */
    /* Now create a buffer and fill it in. */

    if ( acsState.fault == 0)
    {
        /* 2a) Size our buffer to hold the complete message */
        /*     The computed buffer size plus some overhead for the rest of the message */
        int preCheckSize = paramCntNumChars + 7000;
        if (preCheckSize > 125*1024) /* make sure it is reasonable */
        {
            DBGPRINT((stderr, "\nFault in GetParameterValues: Message size too big<%d>\n\n", preCheckSize));
            preCheckSize = 1111; /* this is an error and our messge will be truncated!!!!!! */
        }

        buf=(char *)malloc(preCheckSize);
        xs = buf;
        bufsz = preCheckSize;
        /* printf("\ndoGetParameterValues - Using a buffer size<%d>\n", preCheckSize); */

        /* 2b) Start our message with some envelope stuff */
        /* create response msg start */
        openEnvWithHeader( MORE, a->ID, &xs, &bufsz);
        openBody(&xs, &bufsz);
        xml_mIndent(&xs,&bufsz, 3);
        mprintf(&xs,&bufsz,"<%sGetParameterValuesResponse>\n", nsCWMP);
        xml_mIndent(&xs,&bufsz,4);
#ifdef SUPPRESS_SOAP_ARRAYTYPE
        mprintf(&xs,&bufsz,"<ParameterList>\n");
#else
        mprintf(&xs,&bufsz,"<ParameterList %sarrayType=\"%sParameterValueStruct[%d]\">\n",
                nsSOAP_ENC, nsCWMP, paramCnt);
#endif


        /* 2c) At this spot we are ready for our list of values. The traverseTree func */
        /*     will put the formatted values in our buffer at this place (in the buffer). */

        /* step thru the requested parameter list some may be parameter fragments. */
        if (paramCnt>0)
        {
            if (a != NULL)
                pi = a->ud.pItem;

            while (pi!=NULL && !acsState.fault )
            {
                int     isFragment = 0;
                TRxObjNode  *n;
                const char        *pp = pi->pname;
                if ( pp==NULL || strlen(pp)==0)
                {
                    n = rootDevice;
                    pp = n->name;
                    isFragment = 1;
                }
                else
                {
                    n = findGWParameter(pp);
                    isFragment = pp[strlen(pp)-1]=='.';
                }
                if (n)
                {
                    /* path found: generate response */
                    strncpy(paramName, pp, PNAME_LTH   );
                    if (isFragment )
                        traverseTree( n, writeGetPValue, &xs, &bufsz, 0, isFragment);
                    else if (n->name!=instanceIDMASK)
                        writeGetPValue(n, &xs, &bufsz, 0);
                    else
                        acsState.fault = 9005; /* can't get from an instance */
                }
                else
                {
                    acsState.fault = 9005; /* invalid parameter name */
                }
                pi = pi->next;
            }
        }


        /* 2d) Continue by adding the correct SOAP to finish up */
        xml_mIndent(&xs,&bufsz, 4);
        mprintf(&xs,&bufsz,"</ParameterList>\n");
        xml_mIndent(&xs,&bufsz, 3);
        mprintf(&xs,&bufsz,"</%sGetParameterValuesResponse>\n", nsCWMP);
        closeBodyEnvelope(&xs,&bufsz);
    }


    /* 3) Handle any errors */
    if ( acsState.fault != 0)
    {
        /* reset buffer and build fault here */
        /* We may have a buffer and we may not */
        if (buf)
            free (buf);

        buf=(char *)malloc(2048);  /* (XMLBUFSZ); */
        xs = buf;
        bufsz = 4096;              /* XMLBUFSZ; */
        writeSoapFault(a,&xs,&bufsz, acsState.fault);

        DBGPRINT((stderr, "Handled Fault in GetParameterValues <%d>\n", acsState.fault));
    }

    if (buf)
    {
        DBGPRINT((stderr, "DGPV: doGetParameterValues ret buf <%p> with len <%d> bufsize margin plus<%d>\n\n", buf, strlen(buf), bufsz));
    }
    else
    {
        DBGPRINT((stderr, "DGPV: buf is NULL - it must have failed the malloc!!\n"));
    }

    /* BcmHeapBoundsCheck(); */

    return buf;
}

#endif

/* AddObject
*
*/

static char *doAddObject(RPCAction *a)
{
    TRxObjNode  *n;
    char *pp = a->ud.addDelObjectReq.objectName;
    char    *value = NULL;
    char *buf=(char *)malloc(XMLBUFSZ);
    char *xs = buf;
    int  bufsz = XMLBUFSZ;

    if ( buf ) {
        if ( (pp[strlen(pp)-1]=='.') && (n = findGWParameter(pp))) {
            if (n->paramAttrib.attrib.etype==tInstance) {
                TRxObjNode  *inode;
                TRX_STATUS  trStatus;

                if ( (inode=n->objDetail)
                         && (inode->name==instanceIDMASK)
                         && (inode->getTRxParam!=NULL)
                    ) {

                    if( (trStatus = (inode->getTRxParam(&value))) != TRX_ERR){
                        if (trStatus==TRX_REBOOT)
							rebootFlag = eACS_ADDOBJECTREBOOT;

						saveConfigFlag = 1;
                        /* build AddObject response */
                        openEnvWithHeader(rebootFlag?NOMORE:MORE, a->ID, &xs,&bufsz);
                        openBody(&xs, &bufsz);
                        xml_mIndent(&xs, &bufsz, 3);
                        mprintf(&xs, &bufsz, "<%sAddObjectResponse>\n", nsCWMP);
                        xml_mIndent(&xs, &bufsz, 4);
                        mprintf(&xs, &bufsz, "<InstanceNumber>%s</InstanceNumber>\n", value);
                        xml_mIndent(&xs, &bufsz, 4);
                        mprintf(&xs, &bufsz, "<Status>%s</Status>\n", trStatus==TRX_REBOOT?"1":"0" );
                        xml_mIndent(&xs, &bufsz, 3);
                        mprintf(&xs, &bufsz, "</%sAddObjectResponse>\n", nsCWMP);
                        closeBodyEnvelope(&xs,&bufsz);
                        free (value);
						initNotification();
                    } else {
                        /* fault codes */
                        acsState.fault = 9002;
                    } /* TrStatus == TRX_ERR */
                } else
                    acsState.fault = 9003;
            } /* (n->paramAttrib.attrib.etype==tInstance) */
	    else {
	      /* not an instance */
	      acsState.fault = 9005;
            }
        } else  /* if (pp[strlen(pp)-1]=='.') */
            acsState.fault = 9005;


        if (acsState.fault) {
            writeSoapFault(a, &xs, &bufsz,acsState.fault);
        }
    } /* if (buf) */
    return buf;
}


static TRX_STATUS deleteTree(TRxObjNode *node )
{
    InstanceDesc *idp;
    TRxObjNode  *n = node->objDetail;
	char	*instanceIDstr;
	TRX_STATUS trStatus = TRX_OK;

    if (n!=NULL && n->name==instanceIDMASK) {
        /* this is an object instance -- step thru the instances if present */
        idp = n->paramAttrib.instance;
		while (idp!=NULL && !acsState.fault && trStatus!= TRX_ERR) {
			pushInstance(idp);
			if (checkInstancePath(idp)) {
				trStatus = deleteTree(n);
				if (trStatus==TRX_REBOOT)
					rebootFlag = eACS_DELOBJECTREBOOT;
				saveConfigFlag = 1;
				if (n->setTRxParam != NULL) {
					instanceIDstr = strdup(itoa(idp->instanceID));
                    trStatus = n->setTRxParam( instanceIDstr);
					if (trStatus==TRX_REBOOT )
						rebootFlag = eACS_DELOBJECTREBOOT;
					saveConfigFlag = 1;
					free(instanceIDstr);
				}
			}
            popInstance();
			/* truncate paramName */
			idp = idp->next;
		}
		/* must return here, instance descriptor arrays are not null terminated */
        return trStatus;
    }
    while ( n!=NULL && n->name!=NULL && !acsState.fault && trStatus!=TRX_ERR) {
        if ( n->objDetail!=NULL) {
            trStatus = deleteTree(n);
            /* truncate paramName */
        }
		++n;
    }
    return trStatus;
}
/* DeleteObject
*
*/

static char *doDeleteObject(RPCAction *a)
{
    TRxObjNode  *n;
    char *pp = a->ud.addDelObjectReq.objectName;
    int instanceID;
    char *instanceIDstr;
    char *buf=(char *)malloc(XMLBUFSZ);
    char *xs = buf;
    int  bufsz = XMLBUFSZ;
    TRX_STATUS  trStatus;

    if ( buf ) {
        if ( (pp[strlen(pp)-1]=='.') && (n = findGWParameter(pp)) ){
            if (n->name== instanceIDMASK
                    && n->setTRxParam!=NULL
                    && (instanceID = getCurrentInstanceID())!= -1) {
                if ( (trStatus = deleteTree(n))!= TRX_ERR) {
					if (trStatus==TRX_REBOOT )
						rebootFlag = eACS_DELOBJECTREBOOT;
					saveConfigFlag = 1;
					instanceIDstr = strdup(itoa(instanceID));
					/* Needed so checkInstancePath() succeeds.*/
					popInstance();

					if( (trStatus = n->setTRxParam( instanceIDstr)) != TRX_ERR){
						if (trStatus==TRX_REBOOT )
							rebootFlag = eACS_DELOBJECTREBOOT;
						saveConfigFlag = 1;
						/* build DeleteObject response */
						openEnvWithHeader(rebootFlag?NOMORE:MORE, a->ID, &xs,&bufsz);
						openBody(&xs, &bufsz);
						xml_mIndent(&xs, &bufsz, 3);
						mprintf(&xs, &bufsz, "<%sDeleteObjectResponse>\n", nsCWMP);
						xml_mIndent(&xs, &bufsz, 4);
						mprintf(&xs, &bufsz, "<Status>%s</Status>\n", rebootFlag?"1":"0" );
						xml_mIndent(&xs, &bufsz, 3);
						mprintf(&xs, &bufsz, "</%sDeleteObjectResponse>\n", nsCWMP);
						closeBodyEnvelope(&xs,&bufsz);
					} else
						/* fault codes */
						acsState.fault = 9002;
					free (instanceIDstr);
				} else
					acsState.fault = 9003;
            } else
                acsState.fault = 9003;
        } else
            acsState.fault = 9005;
        if (acsState.fault)
            writeSoapFault(a, &xs, &bufsz, acsState.fault);
    }
    return buf;
}


static char *doRebootRPC(RPCAction *a)
{
    char *buf=(char *)malloc(XMLBUFSZ);
    char *xs = buf;
    int  bufsz = XMLBUFSZ;

    openEnvWithHeader(NOMORE, a->ID, &xs,&bufsz);
	rebootFlag = eACS_RPCREBOOT;
    openBody(&xs, &bufsz);
    xml_mIndent(&xs, &bufsz, 3);
    mprintf(&xs, &bufsz, "<%sRebootResponse/>\n", nsCWMP);
    closeBodyEnvelope(&xs,&bufsz);
    return buf;
}

static char *doFactoryResetRPC(RPCAction *a)
{
    char *buf=(char *)malloc(XMLBUFSZ);
    char *xs = buf;
    int  bufsz = XMLBUFSZ;

	factoryResetFlag = 1;
    openEnvWithHeader( NOMORE, a->ID, &xs,&bufsz);
    openBody(&xs, &bufsz);
    xml_mIndent(&xs, &bufsz, 3);
    mprintf(&xs, &bufsz, "<%sFactoryResetResponse/>\n", nsCWMP);
    closeBodyEnvelope(&xs,&bufsz);
    return buf;
}

/* Build an Inform msg        ???????????????? much more complicated
* Probably will add a lot more args to the call
*/

TRXGFUNC(getManufacturer);
TRXGFUNC(getManufacturerOUI);
TRXGFUNC(getProductClass);
TRXGFUNC(getSerialNumber);

char *buildInform(RPCAction *a, InformEvList *infEvent, int myExtIP)
{
    char *buf=(char *)malloc(XMLBUFSZ);
    char *xs = buf;
    int  bufsz = XMLBUFSZ;
    int  i;
    char *pbuf=(char *)malloc(XMLBUFSZ);
    char *ps = pbuf;
    int pbufsz = XMLBUFSZ;
    time_t  curtime = time(NULL);
    int bootstrap = 0, boot = 0, valuechange = 0;

    acsState.fault = CMSRET_SUCCESS; /* init to no fault */
	paramCnt = 0;
    if (buf && pbuf) {
        char *val;
        a->informID = strdup( itoa(rand()));
        openEnvWithHeader(MORE, a->informID, &xs, &bufsz);
        openBody(&xs, &bufsz);
        xml_mIndent(&xs,&bufsz, 3);
        mprintf(&xs,&bufsz,"<%sInform>\n", nsCWMP);
        xml_mIndent(&xs,&bufsz, 4);
        /* create DeviceId structure */
        mprintf(&xs, &bufsz,"<DeviceId>\n");
        getManufacturer(&val);          /* go direct to parameter since we know it here*/
        xml_mIndent(&xs,&bufsz, 5);
        mprintf(&xs, &bufsz,"<%s>%s</%s>\n",Manufacturer, val, Manufacturer);
        free(val);
        getManufacturerOUI(&val);          /* go direct to parameter since we know it here*/
        xml_mIndent(&xs,&bufsz, 5);
        mprintf(&xs, &bufsz,"<%s>%s</%s>\n", "OUI", val, "OUI");
        free(val);
        getProductClass(&val);          /* go direct to parameter since we know it here*/
        xml_mIndent(&xs,&bufsz, 5);
        mprintf(&xs, &bufsz,"<%s>%s</%s>\n",ProductClass, val, ProductClass);
        free(val);
        getSerialNumber(&val);          /* go direct to parameter since we know it here*/
        xml_mIndent(&xs,&bufsz, 5);
        mprintf(&xs, &bufsz,"<%s>%s</%s>\n",SerialNumber, val, SerialNumber);
        free(val);
        xml_mIndent(&xs,&bufsz, 4);
        mprintf(&xs, &bufsz,"</DeviceId>\n");
        xml_mIndent(&xs,&bufsz, 4);

        /* build Event List */

        /* If we get an BOOTSTRAP event, we need to discard all other event, except BOOT */
        for (i = 0; i < infEvent->informEvCnt; i++)
        {
            if (infEvent->informEvList[i] == INFORM_EVENT_BOOTSTRAP)
                bootstrap = 1;
            else if (infEvent->informEvList[i] == INFORM_EVENT_BOOT)
                boot = 1;
            else if (infEvent->informEvList[i] == INFORM_EVENT_VALUE_CHANGE)
                valuechange = 1;
        }

        if ( bootstrap == 1)
        {
            clearInformEventList(); /* we got BOOTSTRAP event, so discard all event */

            addInformEventToList(INFORM_EVENT_BOOTSTRAP);

            if (boot == 1)
                addInformEventToList(INFORM_EVENT_BOOT);

            #if 0
            /* VALUE_CHANGE event is discarded, so we need to clear all parameters that value changed. */
            if (valuechange == 1)
            {
                cmsPhl_clearAllParamValueChanges();
            }
            #endif
        }

        #if 0
        numParamValueChanges = cmsPhl_getNumberOfParamValueChanges();
        if (numParamValueChanges > 0)
        {
            /* if there is no value_change event in the event list, add it.  It's passive notification */
            for (i = 0; i < infEvent->informEvCnt; i++)
            {
                if (infEvent->informEvList[i] == INFORM_EVENT_VALUE_CHANGE)
                {
                    /* nothing needs to be done */
                    passiveNotification = 0;
                    break;
                }
            }
        }
        #endif


        #ifdef SUPPRESS_SOAP_ARRAYTYPE
		mprintf(&xs, &bufsz,"<Event>\n");
		#else
        mprintf(&xs, &bufsz,"<Event %sarrayType=\"%sEventStruct[%d]\">\n",
                nsSOAP_ENC, nsCWMP, infEvent->informEvCnt);
		#endif
        for (i=0; i < infEvent->informEvCnt; ++i) {
            char    *ck = NULL;
            xml_mIndent(&xs,&bufsz, 5);
            mprintf(&xs, &bufsz,"<EventStruct>\n");
            xml_mIndent(&xs,&bufsz, 6);
            mprintf(&xs, &bufsz,"<EventCode>%s</EventCode>\n",
                     getInformEventStr(infEvent->informEvList[i], infEvent->mMethod) );
            xml_mIndent(&xs,&bufsz, 6);
            if (infEvent->informEvList[i]==eIEMethodResult && infEvent->mMethod==rpcReboot )
                ck=acsState.rebootCommandKey;
            else if (infEvent->informEvList[i]==eIEMethodResult && infEvent->mMethod==rpcDownload )
                ck=acsState.downloadCommandKey;
            #ifdef SUPPRESS_EMPTY_PARAM
            if (empty(ck))
                mprintf(&xs, &bufsz,"<CommandKey>empty</CommandKey>\n");
			else
                mprintf(&xs, &bufsz,"<CommandKey>%s</CommandKey>\n", ck);
            #else
            mprintf(&xs, &bufsz,"<CommandKey>%s</CommandKey>\n", ck? ck: "");
            #endif
            xml_mIndent(&xs,&bufsz, 5);
            mprintf(&xs, &bufsz,"</EventStruct>\n");
        }
        xml_mIndent(&xs,&bufsz, 4);
        mprintf(&xs, &bufsz,"</Event>\n");
        xml_mIndent(&xs,&bufsz, 4);
        mprintf(&xs, &bufsz,"<MaxEnvelopes>1</MaxEnvelopes>\n");
        xml_mIndent(&xs,&bufsz, 4);
        mprintf(&xs, &bufsz,"<CurrentTime>%s</CurrentTime>\n", getXSIdateTime(&curtime));
        xml_mIndent(&xs,&bufsz, 4);
        mprintf(&xs, &bufsz,"<RetryCount>%d</RetryCount>\n", acsState.retryCount);
        xml_mIndent(&xs,&bufsz, 4);
        /* the External IP address parameter name is itself variable based on the current WAN connection configuration */
        /* Need to construct is first. The return buffer is a static char buffer */
#ifdef DEBUG
		printf("build inform: 1. myExtIP = %x\n", myExtIP);
#endif
        informParameters[NUMREQINFORMPARMS-1] = buildExternalConnParamName(myExtIP);
#ifdef DEBUG
		printf("build inform: 2\n");
#endif
        for (i=0; i<NUMREQINFORMPARMS; ++i) {
            TRxObjNode *n;
            if ( (n=findGWParameter(informParameters[i])) ){
                /* found parameter node */ /* need to verify that its an item check TRxType ?????????*/
                if (n->getTRxParam!= NULL){
                    char    *val; /* getTRxParam allocated value buffer -- need to free it*/
                    if ( n->getTRxParam(&val)!=TRX_OK)
                        acsState.fault = 9002; /* Use Internal error for now */
                    else {
						strcpy(paramName,informParameters[i]);
						writeParamValueStruct(n,val,&ps,&pbufsz);
						free(val);
                    }
                } else {
                    #ifdef DEBUG
                    fprintf(stderr, "Unable to read %s\n", informParameters[i])
                    #endif
                    ;
                }
            } else {
                #if 0
                fprintf(stderr, "Unable to locate %s\n", informParameters[i]);
                #endif
                ;
            }
        }

        #ifdef SUPPRESS_SOAP_ARRAYTYPE
		mprintf(&xs, &bufsz,"<ParameterList>\n");
		#else
        mprintf(&xs, &bufsz,"<ParameterList %sarrayType=\"%sParameterValueStruct[%d]\">\n",
				nsSOAP_ENC,nsCWMP,paramCnt+notifyChanges);
		#endif
        if (paramCnt>0)
			mprintf(&xs, &bufsz, pbuf);  /* append parameter buffer */
        free (pbuf);
		if (notifyChanges) {
			mprintf(&xs, &bufsz, valueBuf);  /* append notify change parameter buffer */
		}
		/* notify changes are released after ACS response */
        xml_mIndent(&xs,&bufsz, 4);
        mprintf(&xs, &bufsz,"</ParameterList>\n");
        xml_mIndent(&xs,&bufsz, 3);
        mprintf(&xs,&bufsz,"</%sInform>\n", nsCWMP);
        closeBodyEnvelope(&xs,&bufsz);
    } else
    {
        free(pbuf);
        free(buf);
        buf = NULL;
    }
    acsState.retryCount++;  /* increment retry count -- reset to zero by inform response */
    return buf;
}

char *sendGetRPCMethods(void)
{
	char *buf=(char *)malloc(XMLBUFSZ);
    char *xs = buf;
    int  bufsz = XMLBUFSZ;

	#ifdef DEBUG
	slog(LOG_DEBUG, "sendGetRPCMethods\n");
	#endif
    if (buf) {
        openEnvWithHeader(MORE, NULL, &xs, &bufsz);
        openBody(&xs, &bufsz);
        xml_mIndent(&xs,&bufsz, 3);
        mprintf(&xs,&bufsz,"<%sGetRPCMethods>\n", nsCWMP);
        xml_mIndent(&xs,&bufsz,3);
        mprintf(&xs,&bufsz,"</%sGetRPCMethods>\n", nsCWMP);
        closeBodyEnvelope(&xs,&bufsz);
	}
	return buf;
}

char *sendTransferComplete(void)
{
	char *ck;
	char *buf=(char *)malloc(XMLBUFSZ);
    char *xs = buf;
    int  bufsz = XMLBUFSZ;

	#ifdef DEBUG
	slog(LOG_DEBUG, "sendTransferComplete\n");
	#endif
    if (buf) {
        openEnvWithHeader(MORE, NULL, &xs, &bufsz);
        openBody(&xs, &bufsz);
        xml_mIndent(&xs,&bufsz, 3);
        mprintf(&xs,&bufsz,"<%sTransferComplete>\n", nsCWMP);
        xml_mIndent(&xs,&bufsz, 4);
		ck=acsState.downloadCommandKey;
		#ifdef SUPPRESS_EMPTY_PARAM
		if (ck && strlen(ck)>0)
			mprintf(&xs, &bufsz,"<CommandKey>%s</CommandKey>\n", ck);
		else
			mprintf(&xs, &bufsz,"<CommandKey>empty</CommandKey>\n");
		#else
		mprintf(&xs, &bufsz,"<CommandKey>%s</CommandKey>\n", ck? ck: "");
		#endif
        xml_mIndent(&xs, &bufsz, 4);
        mprintf(&xs, &bufsz,"<FaultStruct>\n");
        xml_mIndent(&xs, &bufsz, 5);
        mprintf(&xs, &bufsz,"<FaultCode>%d</FaultCode>\n",acsState.dlFaultStatus);
        xml_mIndent(&xs, &bufsz, 5);
        mprintf(&xs, &bufsz,"<FaultString>%s</FaultString>\n", acsState.dlFaultMsg?
		#ifdef SUPPRESS_EMPTY_PARAM
				acsState.dlFaultMsg: "empty");
		#else
				acsState.dlFaultMsg: "");
		#endif
        xml_mIndent(&xs, &bufsz, 4);
        mprintf(&xs, &bufsz,"</FaultStruct>\n");
        xml_mIndent(&xs, &bufsz, 4);
        mprintf(&xs, &bufsz,"<StartTime>%s</StartTime>\n", getXSIdateTime(&acsState.startDLTime));
        xml_mIndent(&xs, &bufsz, 4);
        mprintf(&xs, &bufsz,"<CompleteTime>%s</CompleteTime>\n", getXSIdateTime(&acsState.endDLTime));
        xml_mIndent(&xs,&bufsz,3);
        mprintf(&xs,&bufsz,"</%sTransferComplete>\n", nsCWMP);
        closeBodyEnvelope(&xs,&bufsz);
    }
	return buf;
}

static char *doDownload( RPCAction *a)
{
    char *buf=(char *)malloc(XMLBUFSZ);
    char *xs = buf;
    int  bufsz = XMLBUFSZ;

    if (dloadAction)  /* may have tried before so clean up */
        freeRPCAction(dloadAction);
    dloadAction = a;   /* save rpc Action */
    if ( a == rpcAction)
        rpcAction = NULL;  /* if *a is copy of rpcAction. set to NULL */
    if ( preDownloadSetup( &acsState, &a->ud.downloadReq ) )
    {
        setTimer( startDownload, (void *)dloadAction, (1+a->ud.downloadReq.delaySec)*1000 );
        /* build good response */
        openEnvWithHeader(NOMORE, a->ID, &xs, &bufsz);
        openBody(&xs, &bufsz);
        xml_mIndent(&xs,&bufsz, 3);
        mprintf(&xs,&bufsz,"<%sDownloadResponse>\n", nsCWMP);
        xml_mIndent(&xs, &bufsz, 4);
        mprintf(&xs,&bufsz,"<Status>1</Status>\n");
        xml_mIndent(&xs, &bufsz, 4);
        mprintf(&xs,&bufsz,"<StartTime>0001-01-01T00:00:00Z</StartTime>\n");
        xml_mIndent(&xs, &bufsz, 4);
        mprintf(&xs,&bufsz,"<CompleteTime>0001-01-01T00:00:00Z</CompleteTime>\n");
        xml_mIndent(&xs,&bufsz, 3);
        mprintf(&xs,&bufsz,"</%sDownloadResponse>\n", nsCWMP);
        closeBodyEnvelope(&xs,&bufsz);
    }
    else
    {
        xs = buf;
        bufsz = XMLBUFSZ;
        writeSoapFault(a,&xs,&bufsz, acsState.fault);
    }
    return buf;
}
/*
* Update the ACSState parameterKey or commandKeys if they are in
* the RPC.
*/
void updateKeys(RPCAction *rpcAction)
{
    if (rpcAction->parameterKey) {
        if (rebootFlag == NOREBOOT ) {
			/* not rebooting also copy to current parameter key */
			free(acsState.parameterKey);
            acsState.parameterKey = strdup(rpcAction->parameterKey);
		}
        if (acsState.newParameterKey)
            free(acsState.newParameterKey);
        acsState.newParameterKey = rpcAction->parameterKey;
        rpcAction->parameterKey = NULL;
	}
    if (rpcAction->rpcMethod==rpcReboot && rpcAction->commandKey) {
        if (acsState.rebootCommandKey)
            free(acsState.rebootCommandKey);
        acsState.rebootCommandKey = rpcAction->commandKey;
        rpcAction->commandKey = NULL;
	}
}

RunRPCStatus runRPC(void)
{
    char    *rspBuf = NULL;
    RunRPCStatus   rpcStatus = eRPCRunOK;
    informState = eACSInformed;
    rebootFlag = NOREBOOT;

    if (rpcAction != NULL)
    {
        DBGPRINT((stderr,"Now in runRPC and method = <%s>\n", getRPCMethodName(rpcAction->rpcMethod)));
    }
    else
    {
        DBGPRINT((stderr,"Now in runRPC and NULL rpcAction pointer!\n"));
    }

    acsState.fault = 0;

    if (!isAcsConnected())
    {
        DBGPRINT((stderr,"Not connected to ACS, don't runRPC\n"));
        rpcStatus = eRPCRunFail;
    }


    if ((rpcStatus == eRPCRunOK) && (rpcAction != NULL))
    {


        /* after Inform is sent, no other RPCs should be sent before CPE receives InformResponse */
        /* if any RPC is sent before InformResponse (before sessionState is set to eSessionDeliveryConfirm) */
        /* then CPE should cancel current session and retry another session */
        if (rpcAction->rpcMethod != rpcInformResponse && sessionState != eSessionDeliveryConfirm)
        {
            DBGPRINT((stderr,"rpc state machine wrong state, fail conn"));
            rpcStatus = eRPCRunFail;
            acsState.retryCount++;
            retrySessionConnection();
            /* saveTR69StatusItems(); */   /* save retryCount to scratchpad */
        }
        else
        {
            switch (rpcAction->rpcMethod)
            {
            case rpcGetRPCMethods:
                DBGPRINT((stderr,"Handle RPC method: GetRPCMethods\n"));
                rspBuf = doGetRPCMethods(rpcAction);
                break;
            case rpcSetParameterValues:
                DBGPRINT((stderr,"Handle RPC method: SetParameterValues\n"));
                rspBuf = doSetParameterValues(rpcAction);
                break;
            case rpcGetParameterValues:
                DBGPRINT((stderr,"Handle RPC method: GetParameterValues\n"));
                rspBuf = doGetParameterValues(rpcAction);
                break;
            case rpcGetParameterNames:
                DBGPRINT((stderr,"Handle RPC method: GetParameterNames\n"));
                rspBuf = doGetParameterNames(rpcAction);
                break;
            case rpcGetParameterAttributes:
                DBGPRINT((stderr,"Handle RPC method: GetParameterAttributes\n"));
                rspBuf = doGetParameterAttributes(rpcAction);
                break;
            case rpcSetParameterAttributes:
                DBGPRINT((stderr,"Handle RPC method: SetParameterAttributes\n"));
                rspBuf = doSetParameterAttributes(rpcAction);
                break;
            case rpcAddObject:
                DBGPRINT((stderr,"Handle RPC method: AddObject\n"));
                rspBuf = doAddObject(rpcAction);
                break;
            case rpcDeleteObject:
                DBGPRINT((stderr,"Handle RPC method: DeleteObject\n"));
                rspBuf = doDeleteObject(rpcAction);
                break;
            case rpcReboot:
                DBGPRINT((stderr,"Handle RPC method: Reboot\n"));
                rspBuf = doRebootRPC(rpcAction);
                break;
            case rpcFactoryReset:
                DBGPRINT((stderr,"Handle RPC method: FactoryReset\n"));
                rspBuf = doFactoryResetRPC(rpcAction);
                break;
            case rpcDownload:
                DBGPRINT((stderr,"Handle RPC method: Download\n"));
                rspBuf = doDownload(rpcAction);
                break;
            case rpcInformResponse:
                DBGPRINT((stderr,"Handle RPC method: InformResponse\n"));
                sessionState = eSessionDeliveryConfirm;
                acsState.retryCount = 0;            /* must have been successful */

                /* we have received an informResponse, clear inform event list */
                clearInformEventList();
                /* saveLastConnectedURL(); */
                if ( transferCompletePending )
                {
                    /* make any callbacks that were setup when RPC started */
                    rspBuf = sendTransferComplete();
                    transferCompletePending = 0;
                }
                else if ( sendGETRPC )
                {
                    rspBuf = sendGetRPCMethods();
                    sendGETRPC = 0;
                }
                else
                {
                    /* following a InformResponse send a NULL msg */
                    /* send empty message to indcate no more requests */
                    rspBuf = strdup("");  /* sendNullHttp(TRUE); */
                    rpcStatus=eRPCRunEnd;
                }
                /* resetNotification(); */  /* update notifications following informResponse*/
                initNotification();         /* update notifications following informResponse*/
                setInformState(eACSInformed);
                break;
            case rpcTransferCompleteResponse:
                DBGPRINT((stderr,"Handle RPC method: TransferCompleteResponse\n"));
                rspBuf = strdup(""); /*  sendNullHttp(TRUE); */
                rpcStatus = eRPCRunEnd;
                setInformState(eACSInformed);
                break;
            case rpcGetRPCMethodsResponse:
                DBGPRINT((stderr,"Handle RPC method: GetRPCMethodsResponse\n"));
                rspBuf = strdup(""); /* sendNullHttp(TRUE); */
                rpcStatus = eRPCRunEnd;
                break;
            case rpcFault:
                DBGPRINT((stderr,"Handle RPC method: Fault\n"));
            default:
                DBGPRINT((stderr,"Handle RPC method: Method not supported <%s>\n", getRPCMethodName(rpcAction->rpcMethod)));
                rpcStatus = eRPCRunFail;
                break;
            }  /* end of switch(rpcAction->rpcMethod) */
        }

        if (rspBuf)
            sendToAcs(strlen(rspBuf),rspBuf); /* TODO verify param */

        /* if no faults then update ACS state with parameter key or command key. */
        if (rpcStatus != eRPCRunFail && acsState.fault == NO_FAULT && rpcAction!=NULL )
        {
            /* in the case of download this must wait until the download completes*/
            updateKeys( rpcAction );
            DBGPRINT((stderr,"runRPC: ret1\n"));
        }
        DBGPRINT((stderr,"runRPC: ret2\n"));
    }  /* end of if ((rpcStatus == eRPCRunOK) && (rpcAction != NULL)) */

    DBGPRINT((stderr,"runRPC: ret\n"));
    return rpcStatus;
}



typedef int (*WalkTreeCB)(TRxObjNode *node, InstanceDesc *idp, char **buf, int *bufsz, int update);

static int	walkIndex; /* number of node visits- used to id node for saved attributes */
/*
 * getNotifyParams
 * idp is NULL if not instance parameter, such as, .DeviceInfo.ProvisioningCode.
 * idp points to instanceDesc for parameter. Framework stack must be valid.
* If update flag is set then update the copy of the data with the new getTRxParam() data.
* If buf is not NULL then call writeParamValueStruct to fill buf.if notification change found
* return 0: no notification parameter change found
* return 1: notification found change found.
*           If the parameter has an active notification flags set then activeNotifications is
*           incremented.
*/
static int getNotifyParams( TRxObjNode *n, InstanceDesc *idp, char **buf, int *bufsz, int update)
{
	char    *pValue = NULL;
	InstanceDope    *pDope,	   /* pointer to Dope data chained off of parameter node*/
	**pLast;   /* pointer to lask link variable. */

	pLast = &n->instanceDope;
	if ( (pDope=n->instanceDope) ) {
		/* find dope that matches instance path */
		while ( pDope ) {
			if ( (idp==NULL && pDope->instance==NULL)
				 || (pDope->instance==idp && checkInstanceStackPath(pDope->instance))) {
				/* found instance dope */
				if (pDope->notification>NOTIFICATION_OFF) {
					/*fprintf(stderr, "getParam %s\n", paramName);*/
					if ( n->getTRxParam && n->getTRxParam(&pValue)==TRX_OK) {
						if (pDope->pdata && (pValue!=NULL && strcmp(pValue, pDope->pdata)==0)) {
							/* no change */
							free(pValue);
							return 0;
						} else {
							/* parameter changed */
							#ifdef DEBUG
							fprintf(stderr, "%s new>%s\n", paramName, pValue);
							#endif
							if (buf)
								writeParamValueStruct( n, pValue, buf, bufsz);
							if (update) { /* replace Dope data if update is set */
								free (pDope->pdata);
								pDope->pdata = pValue;
							} else
								free(pValue);
							if (pDope->notification==ACTIVE_NOTIFICATION)
								++activeNotifications;
							return 1;
						}
					} else
						return 0;
				}else
					return 0;
			}
			pLast = &pDope->next;
			pDope = pDope->next;
		}
	}
	/* This is necessary following an AddObject */
	/* didn't find idp -- add it to either *n->instancedope or to end of dope list */
	if ( (pDope = (InstanceDope *) malloc(sizeof(struct InstanceDope))) ) {
		memset(pDope,0,sizeof(struct InstanceDope));
		*pLast = pDope;
		pDope->instance = idp;
#if 0
		//#ifdef DEBUG
		//fprintf(stderr, "New InstanceDope(%d) %s\n",walkIndex, paramName);
		//#endif
#endif
	}
	return 0;
}
/*
 * Inputs	*n - node pointer to start at.
 *			*idp - Current Instance descriptor or NULL.
 *			update - Flag to pass to getNotifyParams to indicate it should update is copy of
 *					the save parameter data.
 *			buf - If not NULL the place to put the parameter data for notification (SOAP format).
 *			bufsz - size of buffer for parameter message struct entries.
 * static global:
 *			walkIndex - Should be set to zero before first call. Reflects the order in which
 * 					nodes are visited. Used to remember a specific node for attribute storate.
 *
* return 0: not found
* 		 >0: number of notify changes found
*/

static int walkTree( TRxObjNode *n, InstanceDesc *idp, WalkTreeCB callb, int update, char **buf, int *bufsz  )
{
	int		pathLth = strlen(paramName);
	int		changes = 0;
	InstanceDesc	*idpParent = idp;

	/*fprintf(stderr, "searchTree %s paramPath=%s\n", n->name, paramPath); */

    while (n && n->name) {
		pushNode(n);
		walkIndex++;
		if ( streq(n->name,instanceIDMASK)) {
			InstanceDesc    *idp;
			if ( (idp = n->paramAttrib.instance) ) {
				while (idp) {
					if (idp->parent == idpParent) {
					pushInstance(idp);
					sprintf(paramName, "%s%d.", paramName, idp->instanceID);
					if (n->objDetail)
						 changes += walkTree( n->objDetail, idp, callb, update, buf, bufsz);
					popInstance();
					paramName[pathLth]='\0';
					}
					idp = idp->next;
				}
			} else if (n->objDetail) {
				/* walk to node past instance descriptor- primary purpose is to keep walkIndex the same */
				/* with or without instances */
				changes = walkTree(n->objDetail, NULL, NULL, 0, NULL, 0);
			}
			popNode();
			return changes;   /* return here instance nodes donot have a null terminator*/
		} else if (n->paramAttrib.attrib.etype==tObject
			       || n->paramAttrib.attrib.etype==tInstance) {
			sprintf(paramName, "%s%s.", paramName, n->name);
			changes += walkTree( n->objDetail, idp, callb, update, buf, bufsz );
		} else if ( callb!=NULL) {
			sprintf(paramName, "%s%s", paramName, n->name);
			changes += callb(n,idp,buf,bufsz,update);
		}
        paramName[pathLth]='\0';
        ++n;  /* try next item in node list */
		popNode();
    }
    return changes;
}

void resetNotificationBuf(void)
{
	free (valueBuf);
	valueBuf = NULL;
	valueBufSz = 0;
	notifyChanges = 0;
}
int checkActiveNotifications(void)
{
	int active;
	walkIndex = activeNotifications = 0;
	paramName[0] = '\0';
	walkTree(rootDevice, NULL,getNotifyParams,0, NULL, NULL);
	active = activeNotifications;
	activeNotifications = 0;
	#ifdef DEBUG
	fprintf(stderr, "checkActiveNotificaton active = %d\n", active);
	#endif
	return active;
}

/*
 * This function typically used as a timer callback.
 * It will check if any Active Notifications need to be sent and
 * will send them.
*/
/*
 * setTimer(activeNotificationImpl,NULL, ACTIVE_NOTIFICATION_TIMEOUT );
 * stopTimer(activeNotificationImpl, NULL);
*/
void activeNotificationImpl(void* handle)
{
    int     notifies;

    stopTimer(activeNotificationImpl,handle);

    reInitInstances();
    saveNotificationsFlag = 1;
    if ( ((notifies=checkActiveNotifications())>0)/*|| sendGETRPC>0 */ )
    {
        fprintf(stderr, "activeNotificationImpl: Active notifications =%d\n",notifies);
        stopTimer(sendInform, NULL);
        addInformEventToList(INFORM_EVENT_VALUE_CHANGE);
        setTimer(sendInform,NULL, 0);
    }
    else
    {
        /* no active Notifications- do nothing */
        setTimer(activeNotificationImpl,NULL, ACTIVE_NOTIFICATION_TIMEOUT );
    }
    return;
}

/*
* Get all notifications before inform.
* do not update dope data until inform response has been received.
*/
int getAllNotifications(void)
{
	char *ps;
    walkIndex = notifyChanges = activeNotifications = 0;
	if (valueBuf==NULL)
	valueBuf = (char *)malloc(PARAMLISTSZ);
	ps = valueBuf;
	valueBufSz = PARAMLISTSZ;
	paramName[0] = '\0';
	notifyChanges = walkTree(rootDevice,NULL, getNotifyParams, 0/* no updatecopies*/, &ps, &valueBufSz);
	#ifdef DEBUG
	fprintf(stderr, "getAllNotificaton notifyChanges = %d\n", notifyChanges);
	#endif
	return notifyChanges;
}
/*
* called on start up, following InformResponse and whenever the ACS sets an Attribute
 * , adds or delete objects or sets parameter values.
*/
void initNotification(void)
{
    stopTimer(activeNotificationImpl, NULL);
	walkIndex = activeNotifications = notifyChanges = 0;
	paramName[0] = '\0';
	clearStacks();
	walkTree(rootDevice, NULL, getNotifyParams, 1/*update*/, NULL, NULL);
    setTimer(activeNotificationImpl,NULL, ACTIVE_NOTIFICATION_TIMEOUT );
}



/* callback to save node attributes in save buffer */
static int saveNotifyAttr( TRxObjNode *n, InstanceDesc *idp, char **buf, int *bufsz, int update)
{
	/* some recasting needed for pointers */
	AttEntry	*ap = *(AttEntry **)buf;
	InstanceDope *pDope = findDopeInstance(n);

	BSTD_UNUSED(update);

	if (pDope) {
		if ( (pDope->notification > NOTIFICATION_OFF
			  && *bufsz>(int)sizeof(struct AttEntry)) ){
			/* we only care about parameter with notification values */
			ap->nodeIndex = walkIndex;;
			ap->instanceId = idp==NULL? 0: idp->instanceID;
			ap->attrValue = pDope->notification;
			*buf += sizeof(struct AttEntry);
			*bufsz -= sizeof(struct AttEntry);
			#ifdef DEBUG
			fprintf(stderr, "svAttr %s at %d\n",paramName, ap->nodeIndex);
			#endif
			return 1;
		}
	}
	return 0;
}
/*
* callback to restore Notify attributes:
 * The walkTree call this function for each node. This function scans the
 * retrieved notify table to attempt to match the walkIndex to a AttEntry. If
 * an entry matches the attributes are applied to the node and a 1 is returned
 * as the function value. If no indexs match a 0 is returned.
 */
static int restoreNodeNotifyAttr( TRxObjNode *n, InstanceDesc *idp, char **buf, int *bufsz, int update)
{
	int i;
	/* some recasting needed for pointers */
	AttSaveBuf	*ap = *(AttSaveBuf **)buf;
	InstanceDope *pDope = findDopeInstance(n);

	BSTD_UNUSED(bufsz);
	BSTD_UNUSED(update);

	for (i=0; i<ap->numAttSaved; ++i) {
		AttEntry *e = &ap->attEntry[i];
		if ( walkIndex == e->nodeIndex) {
			if (!pDope ) {
				fprintf(stderr, "Notification initialization error- no InstanceDope for %s.%d\n",
						 n->name, idp?idp->instanceID:0);
				return 0;
			}
			if (n->paramAttrib.attrib.etype == tString
				|| n->paramAttrib.attrib.etype == tInt
				|| n->paramAttrib.attrib.etype == tUnsigned
				|| n->paramAttrib.attrib.etype == tBool
				|| n->paramAttrib.attrib.etype == tDateTime
				|| n->paramAttrib.attrib.etype == tBase64 ) {
				/* passed simple validation */
				pDope->notification = e->attrValue;
				#ifdef DEBUG
				fprintf(stderr, "rdAttr %s %d=%d\n",paramName, e->nodeIndex, e->attrValue);
				#endif
				return 1;
			} else
				slog(LOG_ERROR, "Unable to set notification attribute for %s\n", paramName);
			return 0;
		}
	}
	return 0;
}

int	restoreNotificationAttr(void)
{
	AttSaveBuf	*ap;
	int			bufSz;
	int			attRestored;

	if (tr69RetrieveFromStore(&ap, &bufSz)) {
		#ifdef DEBUG
		{
			int i;
			for (i=0; i<ap->numAttSaved; ++i) {
				fprintf(stderr, "nodeIndex[%d] = %d\n", i, ap->attEntry[i].nodeIndex);
			}
		}
		#endif
        paramName[0] = '\0';
		walkIndex = 0;
		attRestored = walkTree(rootDevice,NULL, restoreNodeNotifyAttr, 0,(char **)&ap, &bufSz);
		free(ap);
		fprintf(stderr, "Parameter Attributes restored from scratch pad = %d\n", attRestored);
		return 1;
	}
	/* slog(LOG_ERROR, "Stored Parameter Attribute data is corrupt or missing"); */
	return 0;
}
/*
*/
void saveNotificationAttributes(void)
{
	void *buf;
	AttSaveBuf *ap;
	char *ps;
	int		bufSz;

	if ( saveNotificationsFlag) {
		saveNotificationsFlag = 0;
		if ( (buf = (char *)malloc(ATTRIBUTESAVESZ))){
			ap = (AttSaveBuf *)buf;
			ps = (char *)&ap->attEntry;
			bufSz = ATTRIBUTESAVESZ-sizeof(struct AttSaveBuf);
			paramName[0] = '\0';
			walkIndex = 0;
			clearStacks();
			ap->numAttSaved = walkTree(rootDevice,NULL, saveNotifyAttr, 1, &ps, &bufSz);
			tr69SaveToStore(ap);
			free (buf);
		}
	}
	return;
}

#ifdef XML_DOC_SUPPORT
/*
* Input:
 * node: Current node to link from
 * cback; Output routine to display node information
 * buf:   *pointer to next available buffer space.
 * bufsz: pointner to size remaining in buffer(buf).
 * noDescend: Just walk next level of tree- don't desend if true
 * oparam: Parameter passed to output (cback) function as parameter 4
 *
 */
static void traverseTreeGetXML(TRxObjNode *node, SoapOutCB cback, char **buf, int *bufsz, int noDesend, int oparam)
{
    TRxObjNode  *n = node->objDetail;
    int pNameEnd, plth;

    pNameEnd = plth = strlen(paramName);

#ifdef DEBUG
    /* dumpNode(node); */
#endif

    if ( paramName[pNameEnd-1]!='.' && plth<(PNAME_LTH-1))
    {
        strcpy( &paramName[pNameEnd++], ".");
    }
    if (n!=NULL && n->name==instanceIDMASK)
    {
#ifdef DEBUG
    /* printf("THIS IS AN INSTANCE\n"); */
#endif
        {
            {
                snprintf(&paramName[pNameEnd],PNAME_LTH-pNameEnd,"{i}");
                (cback)(n,buf,bufsz, oparam);
                if (!noDesend)
                    traverseTreeGetXML(n,cback,buf,bufsz, 0, oparam);
            }
            paramName[pNameEnd] = '\0';
        }
#ifdef DEBUG
	/* printf("......return from traverseTreeGetParamNames 1\n\n"); */
#endif
        return;
    }

    /* Handle a scalar */
    while ( n!=NULL && n->name!=NULL && !acsState.fault )
    {
#ifdef DEBUG
	/* printf("THIS IS A Scalar\n"); */
#endif

        if (n->name[0] != '#') /* check for pseudo parameter */
        {
            strncpy( &paramName[pNameEnd], n->name, PNAME_LTH-pNameEnd);

            if (n->objDetail != NULL)
            {
#ifdef DEBUG
                printf("ttGPN: paramName scalar, <%s>\n", paramName ? paramName : "NULLVALUE");
#endif
                if ( paramName[strlen(paramName)-1]!='.' && strlen(paramName)<(PNAME_LTH-1))
                {
                    strcpy( &paramName[strlen(paramName)], ".");
#ifdef DEBUG
                    printf("ttGPN: paramName scalar, added a dot, now <%s>\n", paramName ? paramName : "NULLVALUE");
#endif
                }
            }

            (cback)(n, buf, bufsz, oparam);   /* Call output function*/

            if (!noDesend)
            {
                /* add current object name to paramName */
                traverseTreeGetXML(n,cback,buf,bufsz, 0, oparam);
                /* truncate paramName */
                paramName[pNameEnd] = '\0';
            }
            else if (n->paramAttrib.attrib.etype == tInstance)
            {
                /*(cback)(n, buf, bufsz, oparam); */  /* special test to get instance write indicator */
            }                                    /* to indicate AddObject/DeleteObject capability */
        }
#ifdef DEBUG
        printf("number of chars in buf<%u>, buf size<%d>\n\n", (unsigned)strlen(*buf),*bufsz);
#endif
        ++n;
    }
    return;
}
#endif

static void traverseTreeGetParamNames(TRxObjNode *node, SoapOutCB cback, char **buf, int *bufsz, int noDesend, int oparam)
{
    InstanceDesc *idp;
    TRxObjNode  *n = node->objDetail;
    int pNameEnd, plth;

    pNameEnd = plth = strlen(paramName);

#ifdef DEBUG
    /* dumpNode(node); */
#endif

    /* if paramName does not end with a dot, then add one */
    if ( paramName[pNameEnd-1]!='.' && plth<(PNAME_LTH-1))
    {
        /* printf("ttGPN: paramName does not end with a dot, add one\n"); */
        strcpy( &paramName[pNameEnd++], ".");
    }
    /* printf("ttGPN: paramName start with <%s>\n", paramName ? paramName : "NULLVALUE"); */


    /* Handle an instance */
    if (n!=NULL && n->name==instanceIDMASK)
    {
#ifdef DEBUG
       /* printf("THIS IS AN INSTANCE\n"); */
#endif
        /* this is an object instance -- step thru the instances if present */
        idp = n->paramAttrib.instance;
        /* printf("GPN:instance, idp <%p>\n",idp ); */

        while (idp!=NULL && !acsState.fault)
        {
            /* printf("GPN:instance, idp->instanceID <%d>\n",idp->instanceID); */
            if (checkInstancePath(idp))
            {
                /* printf("GPN:checkInstancePateh"); */
                pushInstance(idp);
                snprintf(&paramName[pNameEnd],PNAME_LTH-pNameEnd,"%d", idp->instanceID);
                /* printf("%s\n",paramName); */
                (cback)(n,buf,bufsz, oparam);
                if (!noDesend)
                    traverseTreeGetParamNames(n,cback,buf,bufsz, 0, oparam);
                popInstance();
            }
            /* truncate paramName */
            paramName[pNameEnd] = '\0';
            idp = idp->next;
        }
#ifdef DEBUG
    /*  printf("......return from traverseTreeGetParamNames 1\n\n"); */
#endif
        return;
    }

    /* Handle a scalar */
    while ( n!=NULL && n->name!=NULL && !acsState.fault )
    {
#ifdef DEBUG
      /* printf("THIS IS A Scalar\n"); */
      /* dumpNode(n); */
#endif

        if (n->name[0] != '#') /* check for pseudo parameter */
        {
            strncpy( &paramName[pNameEnd], n->name, PNAME_LTH-pNameEnd);

            /* don't add the dot. The writeGetPName output func will add a dot if type==tInstance */
            /* #if 0 */
            /* If there needs to be a ending dot, add it */
            if (n->objDetail != NULL)
            {
#ifdef DEBUG
                /* printf("ttGPN: paramName scalar, <%s>\n", paramName ? paramName : "NULLVALUE"); */
#endif
                if ( paramName[strlen(paramName)-1]!='.' && strlen(paramName)<(PNAME_LTH-1))
                {
                    strcpy( &paramName[strlen(paramName)], ".");
#ifdef DEBUG
                    /* printf("ttGPN: paramName scalar, added a dot, now <%s>\n", paramName ? paramName : "NULLVALUE"); */
#endif
                }
            }
            /* #endif */

            (cback)(n, buf, bufsz, oparam);   /* Call output function*/

            if (!noDesend)
            {
                /* add current object name to paramName */
                traverseTreeGetParamNames(n,cback,buf,bufsz, 0, oparam);
                /* truncate paramName */
                paramName[pNameEnd] = '\0';
            }
            else if (n->paramAttrib.attrib.etype == tInstance)
            {
                /* (cback)(n, buf, bufsz, oparam); */   /* special test to get instance write indicator*/
            }                                     /* to indicate AddObject/DeleteObject capability*/
        }
#ifdef DEBUG
       /* printf("going to next node in scalar while\n\n\n"); */
       /* printf("number of chars in buf<%u>, buf size<%d>\n\n", (unsigned)strlen(*buf),*bufsz); */
#endif
        ++n;
    }
    return;
}

static int numberOfObjects;
static void writeConsole( TRxObjNode *n, char **bp, int *bufsz, int isFragment)
{
	BSTD_UNUSED(bp);
	BSTD_UNUSED(bufsz);
	BSTD_UNUSED(isFragment);

    if ( n->name==instanceIDMASK)
        return;

    printf(" %-61s",paramName);

    numberOfObjects = numberOfObjects+1;

    if (n->getTRxParam!= NULL)
    {
        char    *val = NULL;


        if (n->getTRxParam(&val)!=TRX_OK)
        {
            printf(" ERROR Getting value\n");
        }
        else
        {
            printf(" = <%s>\n",val);
            free(val); /* clean up after getTRxParam */
        }
    }
    else
    {
        printf("\n");
    }
}

#ifdef XML_DOC_SUPPORT

static bool inObject = false;
static void writeXmlToConsole( TRxObjNode *n, char **bp, int *bufsz, int isFragment)
{
	BSTD_UNUSED(bp);
	BSTD_UNUSED(bufsz);
	BSTD_UNUSED(isFragment);

    if (0)
    {
        char    *val = NULL;

        if (n->getTRxParam(&val)!=TRX_OK)
        {
            printf(" ERROR Getting value\n");
        }
        else
        {
            free(val); /* clean up after getTRxParam */
        }
    }
    else
    {
        printf("\n");
    }

    switch (n->paramAttrib.attrib.etype)
    {
        case tObject:
            if (!inObject)
            {
                inObject = true;
            }
            else
            {
                printf("</object>\n");
            }

                if (n->setTRxParam)
                {
					printf("<object name=\"%s\" access=\"readWrite\" minEntries=\"%d\" maxEntries=\"%d\" dmr:version=\"%d.%d\">\n",
                                    paramName, n->minEntries, n->maxEntries, n->majorRev, n->minorRev);
					printf("<description>\n");
					printf("%s.\n", n->supported?"Supported":"Not Supported");
					printf("</description>\n");
                }
                else
                {
                    printf("<object name=\"%s\" access=\"readOnly\" minEntries=\"%d\" maxEntries=\"%d\" dmr:version=\"%d.%d\">\n",
                                    paramName, n->minEntries, n->maxEntries, n->majorRev, n->minorRev);
					printf("<description>\n");
					printf("%s.\n", n->supported?"Supported":"Not Supported");
					printf("</description>\n");
                }

                break;

        case tString:
                if (n->setTRxParam)
                {
                    printf("    <parameter name=\"%s\" access=\"readWrite\" dmr:description=\"Supported\" dmr:version=\"%d.%d\">\n", n->name, n->majorRev, n->minorRev);
					printf("<description>\n");
					printf("%s.\n", n->supported?"Supported":"Not Supported");
					printf("</description>\n");
                }
                else
                {
                    printf("    <parameter name=\"%s\" access=\"readOnly\" dmr:description=\"Supported\" dmr:version=\"%d.%d\">\n", n->name, n->majorRev, n->minorRev);
					printf("<description>\n");
					printf("%s.\n", n->supported?"Supported":"Not Supported");
					printf("</description>\n");
                }

                printf("        <syntax><string/></syntax>\n");
                printf("    </parameter>\n");
                break;

        case tInt:
                if (n->setTRxParam)
                {
                    printf("    <parameter name=\"%s\" access=\"readWrite\" dmr:description=\"Supported\" dmr:version=\"%d.%d\">\n", n->name, n->majorRev, n->minorRev);
					printf("<description>\n");
					printf("%s.\n", n->supported?"Supported":"Not Supported");
					printf("</description>\n");
                }
                else
                {
                    printf("    <parameter name=\"%s\" access=\"readOnly\" dmr:description=\"Supported\" dmr:version=\"%d.%d\">\n", n->name, n->majorRev, n->minorRev);
					printf("<description>\n");
					printf("%s.\n", n->supported?"Supported":"Not Supported");
					printf("</description>\n");
                }

                printf("        <syntax><int/></syntax>\n");
                printf("    </parameter>\n");
                break;


        case tUnsigned:
                if (n->setTRxParam)
                {
                    printf("    <parameter name=\"%s\" access=\"readWrite\" dmr:version=\"%d.%d\">\n", n->name, n->majorRev, n->minorRev);
					printf("<description>\n");
					printf("%s.\n", n->supported?"Supported":"Not Supported");
					printf("</description>\n");
                }
                else
                {
                    printf("    <parameter name=\"%s\" access=\"readOnly\" dmr:version=\"%d.%d\">\n", n->name, n->majorRev, n->minorRev);
					printf("<description>\n");
					printf("%s.\n", n->supported?"Supported":"Not Supported");
					printf("</description>\n");
                }

                printf("        <syntax><unsignedInt/></syntax>\n");
                printf("    </parameter>\n");
                break;

        case tBool:
                if (n->setTRxParam)
                {
                    printf("    <parameter name=\"%s\" access=\"readWrite\" dmr:version=\"%d.%d\">\n", n->name, n->majorRev, n->minorRev);
					printf("<description>\n");
					printf("%s.\n", n->supported?"Supported":"Not Supported");
					printf("</description>\n");
                }
                else
                {
                    printf("    <parameter name=\"%s\" access=\"readOnly\" dmr:version=\"%d.%d\">\n", n->name, n->majorRev, n->minorRev);
					printf("<description>\n");
					printf("%s.\n", n->supported?"Supported":"Not Supported");
					printf("</description>\n");
                }

                printf("        <syntax><boolean/></syntax>\n");
                printf("    </parameter>\n");
                break;

        case tDateTime:
                if (n->setTRxParam)
                {
                    printf("    <parameter name=\"%s\" access=\"readWrite\" dmr:version=\"%d.%d\">\n", n->name, n->majorRev, n->minorRev);
					printf("<description>\n");
					printf("%s.\n", n->supported?"Supported":"Not Supported");
					printf("</description>\n");
                }
                else
                {
                    printf("    <parameter name=\"%s\" access=\"readOnly\" dmr:version=\"%d.%d\">\n", n->name, n->majorRev, n->minorRev);
					printf("<description>\n");
					printf("%s.\n", n->supported?"Supported":"Not Supported");
					printf("</description>\n");
                }
                printf("        <syntax><dateTime/></syntax>\n");
                printf("    </parameter>\n");
                break;

        case tBase64:
                if (n->setTRxParam)
                {
                    printf("    <parameter name=\"%s\" access=\"readWrite\" dmr:version=\"%d.%d\">\n", n->name, n->majorRev, n->minorRev);
					printf("<description>\n");
					printf("%s.\n", n->supported?"Supported":"Not Supported");
					printf("</description>\n");
                }
                else
                {
                    printf("    <parameter name=\"%s\" access=\"readOnly\" dmr:version=\"%d.%d\">\n", n->name, n->majorRev, n->minorRev);
					printf("<description>\n");
					printf("%s.\n", n->supported?"Supported":"Not Supported");
					printf("</description>\n");
                }

                printf("        <syntax><base64/></syntax>\n");
                printf("    </parameter>\n");
                break;

        case tInstance:
                {
                    if (!inObject)
                    {
                        inObject = true;
                    }
                    else
                    {
                        printf("</object>\n");
                    }
                    if (n->setTRxParam)
                    {
                        if (n->maxEntries == 0xffffffff)
                        {
                            printf("<object name=\"%s{i}.\" access=\"readWrite\" minEntries=\"%d\" maxEntries=\"unbounded\" dmr:version=\"%d.%d\">\n",
                                    paramName, n->minEntries, n->majorRev, n->minorRev);
							printf("<description>\n");
							printf("%s.\n", n->supported?"Supported":"Not Supported");
							printf("</description>\n");
                        }
                        else
                        {
                            printf("<object name=\"%s{i}.\" access=\"readWrite\" minEntries=\"%d\" maxEntries=\"%d\" dmr:version=\"%d.%d\">\n",
                                    paramName, n->minEntries, n->maxEntries, n->majorRev, n->minorRev);
							printf("<description>\n");
							printf("%s.\n", n->supported?"Supported":"Not Supported");
							printf("</description>\n");
                        }
                    }
                    else
                    {
                        if (n->maxEntries == 0xffffffff)
                        {
                            printf("<object name=\"%s{i}.\" access=\"readOnly\" minEntries=\"%d\" maxEntries=\"unbounded\" dmr:version=\"%d.%d\">\n",
                                    paramName, n->minEntries, n->majorRev, n->minorRev);
							printf("<description>\n");
							printf("%s.\n", n->supported?"Supported":"Not Supported");
							printf("</description>\n");
                        }
                        else
                        {
                            printf("<object name=\"%s{i}.\" access=\"readOnly\" minEntries=\"%d\" maxEntries=\"%d\" dmr:version=\"%d.%d\">\n",
                                    paramName, n->minEntries, n->maxEntries, n->majorRev, n->minorRev);
							printf("<description>\n");
							printf("%s.\n", n->supported?"Supported":"Not Supported");
							printf("</description>\n");
                        }
                    }
                }
            break;

        default:
            break;
    }
}
#endif

/*
* Input:
 * node: Current node to link from
 * cback; Output routine to display node information
 * buf:   *pointer to next available buffer space.
 * bufsz: pointner to size remaining in buffer(buf).
 * noDescend: Just walk next level of tree- don't desend if true
 * oparam: Parameter passed to output (cback) function as parameter 4
 *
 * This function is called to dump the tree to the console
*/
static void traverseTreePrint(TRxObjNode *node, SoapOutCB cback, char **buf, int *bufsz, int noDesend, int oparam)
{
    InstanceDesc *idp;
    TRxObjNode  *n = node->objDetail;
    int pNameEnd, plth;

    /*memset (paramName,0,sizeof(paramName));*/

    pNameEnd = plth = strlen(paramName);

    if (paramName[pNameEnd-1]!='.' && plth<(PNAME_LTH-1))
        /*strlcpy(&paramName[pNameEnd++], ".", PNAME_LTH-plth);*/
		strncpy(&paramName[pNameEnd++], ".", sizeof(paramName) -1);
		paramName[sizeof(paramName) -1] = '\0';

    if (n!=NULL && n->name==instanceIDMASK)
    {
        idp = n->paramAttrib.instance;
        while (idp!=NULL && !acsState.fault)
        {
            if (checkInstancePath(idp))
            {
                pushInstance(idp);
                snprintf(&paramName[pNameEnd],PNAME_LTH-pNameEnd,"%d", idp->instanceID);
                (cback)(n,buf,bufsz, oparam);
                if (!noDesend)
                    traverseTreePrint(n,cback,buf,bufsz, 0, oparam);
                popInstance();
            }
            /* truncate paramName */
            paramName[pNameEnd] = '\0';
            idp = idp->next;
        }
#ifdef DEBUG
        printf("......return from TraverseTreePrint 1\n\n");
#endif
        return;
    }

    if(n==NULL)
    {
        /* last paramName is not supposed to end with a dot, then remove it */
        if (paramName[pNameEnd-1]=='.' && plth<(PNAME_LTH-1))
            paramName[pNameEnd-1] = '\0';

        if (!noDesend) (cback)(node, buf, bufsz, oparam);   /* Call output function*/

    } else {
        while ( n!=NULL && n->name!=NULL && !acsState.fault )
        {
            if (n->name[0] != '#') /* check for pseudo parameter */
            {
                /*strlcpy( &paramName[pNameEnd], n->name, PNAME_LTH-pNameEnd);*/
				strncpy(&paramName[pNameEnd], n->name, sizeof(paramName) -1);
				paramName[sizeof(paramName) -1] = '\0';
                if (!noDesend)
                {
                    /* add current object name to paramName */
                    traverseTreePrint(n,cback,buf,bufsz, 0, oparam);
                    /* truncate paramName */
                    paramName[pNameEnd] = '\0';
                }
                else
                {
                    (cback)(n, buf, bufsz, oparam);
                }
            }
            ++n;
        }
    }
    return;

}

#ifdef XML_DOC_SUPPORT
static void BcmTr69PrintXMLHeader(void)
{
  printf("<dm:document xmlns:dm=\"urn:broadband-forum-org:cwmp:datamodel-1-4\"\n");
  printf(" xmlns:dmr=\"urn:broadband-forum-org:cwmp:datamodel-report-0-1\"\n");
  printf(" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
  printf(" xsi:schemaLocation=\"urn:broadband-forum-org:cwmp:datamodel-1-4\n");
  printf("                      http://www.broadband-forum.org/cwmp/cwmp-datamodel-1-4.xsd\n");
  printf("                      urn:broadband-forum-org:cwmp:datamodel-report-0-1\n");
  printf("                      http://www.broadband-forum.org/cwmp/cwmp-datamodel-report.xsd\"\n");
  printf("                      spec=\"urn:broadband-forum-org:tr-181-2-5-0\" file=\"tr-181-2-5-0.xml\"\n");
  printf("                      deviceType=\"urn:example-com:device-1-0-0\">\n");
  printf(" <model name=\"X_BROADCOM-COM_Device:2.5\">\n");
  printf(" <object name=\"Device.\" access=\"readOnly\" minEntries=\"1\" maxEntries=\"1\">\n");

  inObject = true;
}
#endif

void BcmTr69PrintTree(const char *ParameterPath, int NextLevel, unsigned int printXML)
{
    int isFragment = 0;
    TRxObjNode  *startNode;
    char *buf=(char *)malloc(20048);
    int  bufsz = 20048;

#ifdef XML_DOC_SUPPORT
    if (printXML)
        BcmTr69PrintXMLHeader();
    else
#else
    BSTD_UNUSED(printXML);
#endif
    printf("\n-------------------- Walking TR-069 Tree --------------------\n\n");

    if ( (ParameterPath==NULL) ||
         (strlen(ParameterPath)==0) ||
         (!strncmp(ParameterPath,"empty",5)) )
    {
        printf("Setting start node to rootDevice\n");
        startNode = rootDevice;
        isFragment = 1;
    }
    else
    {
        startNode = findGWParameter(ParameterPath);
        isFragment = ParameterPath[strlen(ParameterPath)-1]=='.';
    }

    if (startNode)
    {
		#ifdef DEBUG
        printf("found the node <%s> \n",startNode->name);
		#endif
    }
    else
    {
        printf("ERROR: Did NOT find the node for <%s>\n",ParameterPath);
        return;
    }

#ifdef DEBUG
    printf("start with isFragment <%d>\n\n",isFragment);
#endif

    memset (buf,0,bufsz);

    memset (paramName,0,sizeof(paramName));
    if (ParameterPath)
    {
        /*strlcpy (paramName, ParameterPath, sizeof(paramName));*/
		strncpy(paramName, ParameterPath, sizeof(paramName) -1);
		paramName[sizeof(paramName) -1] = '\0';
    }

    numberOfObjects = 0;

#ifdef XML_DOC_SUPPORT
    if (printXML)
    {
        traverseTreeGetXML(startNode, writeXmlToConsole, &buf, &bufsz, NextLevel, isFragment);
    }
    else
#endif
    {
    traverseTreePrint(startNode, writeConsole, &buf, &bufsz, NextLevel, isFragment);
    }
    free (buf);
#ifdef XML_DOC_SUPPORT
    if (printXML)
    {
        printf("    </object>\n");
        printf("   </model>\n");
        printf("</dm:document>\n");
    }
    else
#endif
    {
		printf("\n ** Number of Supported Data Model Objects = <%d> **\n",numberOfObjects);
    }
}


void BcmTr69GetObject(const char *pPath, char **pValue, int verbose)
{
    TRxObjNode  *pParm;
    char    *val = NULL;

    pParm = findGWParameter(pPath);
    if (pParm)
    {
        if (pParm->getTRxParam != NULL)
        {
            if (pParm->getTRxParam(&val) != TRX_ERR)
            {
              if (verbose)
                printf("%s = %s\n", pPath, val);
              else
                *pValue = strdup(val);
            }
            else
            {
              if (verbose)
                printf("%s get failure\n", pPath);
            }
        }
        else
        {
          if (verbose)
            printf("%s is not gettable\n", pPath);
        }
    }
    else
    {
      if (verbose)
        printf("%s not found\n", pPath);
    }
}

void BcmTr69SetObject(const char *pPath, const char *pValue)
{
    TRxObjNode  *pParm;

    pParm = findGWParameter(pPath);
    if (pParm)
    {
        if (pParm->setTRxParam != NULL)
        {
            if (pParm->setTRxParam(pValue) != TRX_ERR)
            {
                printf("%s set to %s\n", pPath, pValue);
            }
            else
            {
                printf("%s set failure\n", pPath);
            }
        }
        else
        {
            printf("%s is not settable\n", pPath);
        }
    }
    else
    {
        printf("%s not found\n", pPath);
    }
}

void BcmTr69AddObject(const char *pPath)
{
    TRxObjNode  *pParm;
    char    *value = NULL;

    printf("\n*** adding %s ***\n", pPath);

    if ( (pPath[strlen(pPath)-1]=='.') && (pParm = findGWParameter(pPath)))
    {
        if (pParm->paramAttrib.attrib.etype==tInstance)
        {
            TRxObjNode  *inode;
            TRX_STATUS  trStatus;

            if ( (inode=pParm->objDetail) &&
                 (inode->name==instanceIDMASK) &&
                 (inode->getTRxParam!=NULL))
            {

                if ( (trStatus = (inode->getTRxParam(&value))) != TRX_ERR)
                {
                    printf (" %s%s created \n", pPath, value);
                }
                else
                {
                    printf (" Instance creation failed for object name (%s). \n", pPath);
                }
            }
            else
            {
                printf (" Instance creation failed for object name (%s). \n", pPath);
            }
        }
        else
        {
            printf (" Bad object name (%s) \n", pPath);
        }
    }
    else
    {
        printf (" Bad object name (%s) \n", pPath);
    }
}

void BcmTr69DeleteObject(const char *pPath)
{
    TRxObjNode  *pParm;
    int instanceID;
    char *instanceIDstr;

    if ((pParm = findGWParameter(pPath)) != NULL)
        if (pParm->name == instanceIDMASK &&
            pParm->setTRxParam != NULL &&
            (instanceID = getCurrentInstanceID()) != -1)
            if (deleteTree(pParm) != TRX_ERR)
            {
                instanceIDstr = strdup((char*)itoa(instanceID));
                popInstance();
                if ( (pParm->setTRxParam( instanceIDstr)) != TRX_ERR)
                    printf (" %s deleted \n", pPath);
                else
                    printf (" %s delete failed \n", pPath);
            }
            else
                printf (" %s delete failed \n", pPath);
        else
            printf (" %s not an instance \n", pPath);
    else
        printf (" %s not found \n", pPath);
}

void *BcmTr69FindHwUserData(const char *pPath)
{
  TRxObjNode  *pParm = NULL;
  InstanceDesc *idp = NULL;
  int instanceID;

  if (pPath == NULL)
    return NULL;

  pParm = findGWParameter(pPath);
  if ((pParm != NULL) && (pParm->name==instanceIDMASK)) {
    instanceID = getCurrentInstanceID();
    /* this is an object instance -- step thru the instances if present */
    idp = pParm->paramAttrib.instance;
    while (idp!=NULL) {
      if (idp->instanceID == instanceID) {
        if (idp->hwUserData != NULL) {
          return idp->hwUserData;
        } else {
          return NULL;
        }
      }
      idp = idp->next;
    }
  }

  return NULL;
}

#ifdef DEBUG
static void dumpNode(TRxObjNode *node)
{
    if (node == NULL)
    {
        printf("NodeDump: Node is NULL\n");
        return;
    }

    printf("Node->name <%s>\n",node->name ? node->name : "NULL");
    printf("  ->SETfunc<%p>  ",node->setTRxParam);
    printf("->GETfunc<%p>  ",node->getTRxParam);
    printf("->objDetail<%p>  ",node->objDetail);
    printf("->InstDope<%p>  ",node->instanceDope);

    if ( node->name == instanceIDMASK)
    {
        printf("->Inst ptr<%p>  ", node->paramAttrib.instance);
        if (node->paramAttrib.instance)
            printf("->InstID<%d>",node->paramAttrib.instance->instanceID);
        printf("\n");
    }
    else
    {
        char type[16];
        switch (node->paramAttrib.attrib.etype)
        {
        case tObject:
            strcpy(type,"OBJ");
            break;
        case tString:
            strcpy(type,"STR");
            break;
        case tInt:
            strcpy(type,"INT");
            break;
        case tUnsigned:
            strcpy(type,"UINT");
            break;
        case tBool:
            strcpy(type,"BOOL");
            break;
        case tBase64:
            strcpy(type,"B64");
            break;
        case tInstance:
            strcpy(type,"INSTANCE");
            break;
        case tStringSOnly:
            strcpy(type,"STRSONLY");
            break;
        default:
            strcpy(type,"UNK");
            break;
        }
        printf("->inhibitActNtfy<%d>  ",node->paramAttrib.attrib.inhibitActiveNotify);
        printf("->etype<%s>\n",type);
    }

}
#endif
