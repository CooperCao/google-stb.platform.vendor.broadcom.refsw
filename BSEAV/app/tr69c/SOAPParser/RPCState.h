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

#ifndef __RPCSTATE_H__
#define __RPCSTATE_H__

/* fault codes */
#define NO_FAULT    0
#define RPC_PENDING 1
#define RPC_FAULTS  9000

/* Has the ACS been contacted,
* This is used to determine the type of Inform Event on
* startup.
*/
typedef enum {
    eACS_NEVERCONTACTED = 0,
    eACS_CONTACTED,		     /* NOTICE: These enum are order dependent*/
    eACS_DOWNLOADREBOOT,
    eACS_SETVALUEREBOOT,
	eACS_ADDOBJECTREBOOT,
	eACS_DELOBJECTREBOOT,
	eACS_RPCREBOOT
} eACSContactedState;		 /* NOTICE: These enum are order dependent*/
#define NOREBOOT eACS_NEVERCONTACTED

/* status enum for acs connection and msg xfer */
typedef enum {
    eOK,
    eConnectError,
    eGetError,
    ePostError,
    eAuthError,
    eDownloadDone,
    eAcsDone
}AcsStatus;

/* rpcRun return status */
typedef enum {
	eRPCRunOK,		/* sent RPC response to ACS */
	eRPCRunEnd,		/* sent NULL http msg to ACS */
	eRPCRunFail		/* RPC run failed no reponse to send */
					/*  this should probably send a fault */
}RunRPCStatus;

/* inform event enumb -- kind of inform msg */
typedef enum {
    eIEBootStrap,
    eIEBoot,
    eIEPeriodix,
    eIEScheduled,
    eIEValueChanged,
    eIEKicked,
    eIEConnectionRequest,
    eIETransferComplete,
    eIEDiagnostics,
    eIEMethodResult,
    eIEXVendor
} eInformEvent;


typedef enum {
    rpcUnknown=0,
    rpcGetRPCMethods,
    rpcSetParameterValues,
    rpcGetParameterValues,
    rpcGetParameterNames,
    rpcGetParameterAttributes,
    rpcSetParameterAttributes,
    rpcAddObject,
    rpcDeleteObject,
    rpcReboot,
    rpcDownload,
	rpcScheduleInform,
    rpcFactoryReset,            /******** last rpc method ******/
    rpcInformResponse,          /* responses start here */
    rpcTransferCompleteResponse,
	rpcGetRPCMethodsResponse,
    rpcFault					/* soapenv:Fault response from ACS */
} eRPCMethods;
#define LAST_RPC_METHOD    rpcFactoryReset     /* see above enumeration */

#define MAXINFORMEVENTS 16
typedef struct InformEvList {
    eInformEvent     informEvList[MAXINFORMEVENTS];
    int              informEvCnt;   /* number of events in list */
	eRPCMethods		 mMethod;		/* set if M <method> event required */
} InformEvList;

typedef enum {
    eFirmwareUpgrade=1,
    eWebContent=2,
    eVendorConfig=3
} eFileType;

typedef struct DownloadReq {
    eFileType efileType;
	char	*commandKey;
    char    *url;
    char    *user;
    char    *pwd;
    int     fileSize;
    char    *fileName;   /* ignore in this implementation- everything is in memory */
    int     delaySec;
} DownloadReq;

typedef struct ParamItem {
    struct ParamItem   *next;
    char   *pname;
    char   *pvalue;
	char   *pOrigValue;
	int		fault;		/* 0 or set fault code */
}ParamItem;

typedef struct AttributeItem {
    struct AttributeItem *next;
    char   *pname;
    int    notification;
    int    chgNotify;
    int    chgAccess;
    int    subAccess;   /* need to add list here if spec changes or vendor reqmts */
} AttributeItem;

typedef struct ParamNamesReq {
    char    *parameterPath;
    int     nextLevel;
} ParamNamesReq;

typedef struct AddDelObject {
    char    *objectName;    /* For Add the object name is xxx.yyy. */
                            /* for Delete the object anme is xxx.yyy.i. */
                            /* where i is the instance number */
} AddDelObject;

typedef struct RPCAction {
    char   *ID;    /* pointer to ID string */
    char    *informID;  /* ID string sent with last inform */
    eRPCMethods rpcMethod;
    int     arrayItemCnt;   /* cnt of items in parameter list-not used */
    char    *commandKey;    /* */
    char    *parameterKey;  /* for setParameterValue key */
    union {
        ParamItem       *pItem;
        AttributeItem   *aItem;
        ParamNamesReq   paramNamesReq;
        AddDelObject    addDelObjectReq;
        DownloadReq     downloadReq;
        /* more items here later for each rpc method*/
    } ud;
} RPCAction;

/* structures to save notification entries */
typedef struct AttEntry {
	short int	nodeIndex;			/* index in CPE param table */
	short int	attrValue;			/* attribute value (1..2)  0 is not saved*/
	int			instanceId;			/* Id of instance or zero */
} AttEntry;

typedef struct AttSaveBuf {
	short int	sigValue;
	short int	numAttSaved;		/* number of notification attributes saved*/
	AttEntry	*attEntry;
} AttSaveBuf;


RPCAction* newRPCAction(void);
void freeRPCAction(RPCAction *item);

void dumpAcsState(void);
void dumpRpcAction(RPCAction *);
char *buildInform(RPCAction *a, InformEvList *, int);
void updateKeys( RPCAction *a);
RunRPCStatus runRPC(void);
int  checkActiveNotifications(void);
void initNotification(void);
int  getAllNotifications(void);
void resetNotificationBuf(void);
void saveConfigurations(void);
void rebootCompletion(void);
void factoryResetCompletion(void);
char *sendTransferComplete(void);
int	restoreNotificationAttr(void);
void saveNotificationAttributes(void);

#endif /* __RPCSTATE_H__ */
