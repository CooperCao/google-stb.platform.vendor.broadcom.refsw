
/******************************************************************************
 *    (c)2010-2013 Broadcom Corporation
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
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
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
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <syslog.h>
#include "bcmWrapper.h"
#include "../main/utils.h"
#include "../inc/tr69cdefs.h"
#include "../inc/appdefs.h"
#include "../main/informer.h"
#include "../SOAPParser/CPEframework.h"
#include "../main/httpDownload.h"
#include "../main/md5.h"
#include "../main/types.h"
#include "syscall.h"
#include "../bcmLibIF/tr135/bcmStbService.h"
#include "../bcmLibIF/tr181/bcmMoca.h"
#include "../bcmLibIF/tr181/bcmEthernet.h"

#include "../inc/tr69cdefs.h" /* defines for ACS state */

#define MD5SUMLEN 16
#define MD5SUMSUPPORT
#define DEBUG
#define DEBUGDLMD5SUM
/*#define DEBUGDLCONTENT*/

extern ACSState acsState;
GWStateData	gwState;
eACSContactedState acsContactedState;

/* getACSContactedState returns eACS_NEVERCONTACTED if the ACS has never */
/* been contacted since initial customer configuration.  eACS_CONTACTED is */
/* returned if the ACS has been contacted before. This is used to determine*/
/* the boot event in the Inform */

eACSContactedState getACSContactedState(void)
{
	eACSContactedState state = eACS_NEVERCONTACTED;
	state = gwState.contactedState;
	acsContactedState = state;
	fprintf(stderr, "getACSContactedState returns %d\n", acsContactedState);
	return state;
}

void setACSContactedState(eACSContactedState state)
{
	acsContactedState = state;
}

/*
* Save the TR69 state values across the reboot
*/
void  saveTR69StatusItems( ACSState *a )
{
	memset(&gwState, 0, sizeof(GWStateData));

	/* fill State Data structure from acsState data */
	if ( a->downloadCommandKey )
		strncpy(gwState.downloadCommandKey, a->downloadCommandKey, sizeof(gwState.downloadCommandKey));
	if ( a->rebootCommandKey )
		strncpy(gwState.rebootCommandKey, a->rebootCommandKey, sizeof(gwState.rebootCommandKey));
	if ( a->newParameterKey )
		strncpy(gwState.newParameterKey, a->newParameterKey, sizeof(gwState.newParameterKey));
	else if (a->parameterKey) /* otherwise keep old one */
		strncpy(gwState.newParameterKey, a->parameterKey, sizeof(gwState.newParameterKey));

	/* gwState.lastInstaceID = lastInstanceId; */
	gwState.contactedState = acsContactedState;
	gwState.dlFaultStatus = a->dlFaultStatus;
	gwState.startDLTime = a->startDLTime;
	gwState.endDLTime = a->endDLTime;
}

/* save to scratch pad */
void saveACSContactedState(void)
{
	saveTR69StatusItems(&acsState);
}

void retrieveTR69StatusItems(void)
{
	if ( acsState.downloadCommandKey )
	{
		free(acsState.downloadCommandKey);
		acsState.downloadCommandKey = NULL;
	}
	if ( gwState.downloadCommandKey )
		acsState.downloadCommandKey = strdup(gwState.downloadCommandKey);
	if ( acsState.rebootCommandKey )
	{
		free(acsState.rebootCommandKey);
		acsState.rebootCommandKey = NULL;
	}
	if ( gwState.rebootCommandKey )
		acsState.rebootCommandKey = strdup(gwState.rebootCommandKey);
	if ( acsState.newParameterKey )
	{
		free(acsState.newParameterKey);
		acsState.newParameterKey = NULL;
	}
	if ( gwState.newParameterKey )
		acsState.parameterKey = strdup(gwState.newParameterKey);
	acsState.dlFaultStatus = gwState.dlFaultStatus;
	acsState.startDLTime = gwState.startDLTime;
	acsState.endDLTime = gwState.endDLTime;
}

void wrapperReboot(eACSContactedState rebootContactValue)
{
	setACSContactedState( rebootContactValue );	 /* reset on BcmCfm_download error */
	saveTR69StatusItems(&acsState);
#ifdef DEBUG
	fprintf(stderr, "CPE is REBOOTING with rebootContactValue =%d\n", rebootContactValue);
#endif
}

/*
* Call library factory reset
*/
void wrapperFactoryReset(void)
{
}

/*
* Call library save configuration
*/
void wrapperSaveConfigurations(void)
{
}

/* preDownloadSetup(ACSState *acs, DownloadReq *)
* This function is called before the framework begins downloading of the image file.
* Any unnecessary applications or memory resources should be freed.
* Return 1: to continue download.
*        0: to Abort download.
*/
int  preDownloadSetup(ACSState *acs, DownloadReq *r)
{
#ifdef DEBUG
	fprintf(stderr, "preDownloadSetup: URL=%s\n", r->url);
	fprintf(stderr, "User/pw: %s:%s\n", r->user, r->pwd);
	fprintf(stderr, "Required memory buffer size will be %d\n", r->fileSize);
#endif
	BSTD_UNUSED(acs);
	return 1;
}

/* downloadComplete()
*  Called when image has been downloaded. If successful the *buf will point to the
*  image buffer. If *buf is NULL the download failed.
*  Control is returned to the framework with a 1 if the flash was successful.
*  If the flash image failed the return is a 0. Free the buffer prior
*  to return to the framework.
*/
int  downloadComplete(DownloadReq *r, char *buf)
{
	if ( buf )
    {
		int i, fileSize;
        MD5Context ctx;
        unsigned char md5sum[MD5SUMLEN];

#ifdef MD5SUMSUPPORT
        fileSize = r->fileSize - MD5SUMLEN;
#else
        fileSize = r->fileSize;
#endif

        MD5Init(&ctx);
        MD5Update(&ctx, buf, fileSize);
        MD5Final(md5sum, &ctx);
#ifdef DEBUG
#ifdef DEBUGDLMD5SUM
        fprintf(stderr, "*********************************\n");
        fprintf(stderr, "MD5 (128-bit) checksum:\n");
        for (i = 0; i < MD5SUMLEN; i++)
            fprintf(stderr, "%x", md5sum[i]);
        fprintf(stderr, "\n*********************************\n");
#endif
#ifdef DEBUGDLCONTENT
		fprintf(stderr, "=======================================================\n");
		fprintf(stderr, "Content (len=%d):\n\t", r->fileSize); /* $ od -txC */
		for (i = 0; i < r->fileSize; i++)
		{
			if ((i != 0) && (i%16 == 0)) fprintf(stderr, "\n\t");
			fprintf(stderr, "%02x ", *(unsigned char *)(buf + i));
		}
		fprintf(stderr, "\n=======================================================\n");
#endif
#endif
#ifdef MD5SUMSUPPORT
        for (i = 0; i < MD5SUMLEN; i++)
        {
            if (md5sum[i] != *(unsigned char *)(buf + fileSize + i))
            {
                updateDownLoadKey( r );
                acsState.dlFaultStatus = 9010; /* download failure*/
                acsState.dlFaultMsg = "Corrupted download image";
                setACSContactedState( eACS_CONTACTED );
                saveTR69StatusItems(&acsState);
#ifdef DEBUG
                fprintf(stderr, "downloadComplete -- incorrect md5 checksums, download failed!\n");
#endif
                free(buf);
                return 1;
            }
        }
#ifdef DEBUG
       fprintf(stderr, "downloadComplete -- md5 checksums matched\n");
#endif
#endif
#ifdef DEBUG
        if (r->efileType == eFirmwareUpgrade)
			fprintf(stderr, "downloadComplete -- save flash image\n");
		else if (r->efileType == eVendorConfig)
			fprintf(stderr, "downloadComplete -- save vendor config file\n");
#endif
		if ( r->efileType == eFirmwareUpgrade || r->efileType == eVendorConfig)
		{
			updateDownLoadKey( r );
			/* does not return */
			acsState.dlFaultStatus = 0;	/* no download fault */
			acsState.dlFaultMsg = "Download successful";
            setACSContactedState( eACS_DOWNLOADREBOOT );
			saveTR69StatusItems(&acsState);
#ifdef DEBUG
			fprintf(stderr, "downloadComplete -- save ACSState\n");
#endif
		}
		else
		{
			updateDownLoadKey( r );
			acsState.dlFaultStatus = 9010; /* download failure*/
			acsState.dlFaultMsg = "Bad flash image format";
			setACSContactedState( eACS_CONTACTED );
			saveTR69StatusItems(&acsState);
		}
		free(buf);
		return 1;
	}

	return 0;
}

int getRAMSize(void)
{
	return 1;
}


/* getNewInstanceId return an instance id in the integer range that
* has not yet been used.This should be save in persistent memory
* somewhere and initialized on startup.
* Need to think about integer wrap-around and asigning duplicates.
*  ?????????
*/
static int lastInstanceId;
int getNewInstanceId(void)
{
	return ++lastInstanceId;
}

/* Used to save and restore tr69 parameter attributes.
*/
int tr69RetrieveFromStore(AttSaveBuf **bufp, int *size)
{
	BSTD_UNUSED(bufp);
	BSTD_UNUSED(size);
	return 0;
}

int	tr69SaveToStore( AttSaveBuf *ap )
{
	BSTD_UNUSED(ap);
	return 1;
}
/**********************************************************************
* STUBS for get/setParameter****************
**********************************************************************/

/*
* .MangementServer.
*/
TRX_STATUS setMSrvrURL(const char *value)
{
	free(acsState.acsURL);
	acsState.acsURL = strdup(value);
	return TRX_OK;
}
TRX_STATUS getMSrvrURL(char **value)
{
	*value = acsState.acsURL?strdup(acsState.acsURL):strdup("");
	return TRX_OK;
}

TRX_STATUS getMSrvrUsername(char **value)
{
	*value = acsState.acsUser?strdup(acsState.acsUser):strdup("");
	return TRX_OK;
}

TRX_STATUS setMSrvrUsername(const char *value)
{
	free(acsState.acsUser);
	acsState.acsUser= strdup(value);
	return TRX_OK;
}

TRX_STATUS getMSrvrPassword(char **value)
{
	*value = acsState.acsPwd?strdup(acsState.acsPwd):strdup("");
	return TRX_OK;
}

TRX_STATUS setMSrvrPassword(const char *value)
{
	free(acsState.acsPwd);
	acsState.acsPwd= strdup(value);

	return TRX_OK;
}

TRX_STATUS setMSrvrInformEnable(const char *value)
{
	acsState.informEnable = testBoolean(value);
	return TRX_OK;
}
TRX_STATUS getMSrvrInformEnable(char **value)
{
	*value = strdup(acsState.informEnable? "1": "0");
	return TRX_OK;
}
TRX_STATUS setMSrvrInformInterval(const char *value)
{
	acsState.informInterval = atoi(value);
	resetPeriodicInform(acsState.informInterval);
	return TRX_OK;
}
TRX_STATUS getMSrvrInformInterval(char **value)
{
	char    buf[10];
	snprintf(buf,sizeof(buf),"%d",(int)acsState.informInterval);
	*value = strdup(buf);
	return TRX_OK;
}

TRX_STATUS getMSrvrInformTime(char **value)
{
    char    buf[30];
	if (acsState.informTime) {
		struct tm *bt=localtime(&acsState.informTime);
		strftime(buf,sizeof(buf),"%Y-%m-%dT%H:%M:%S",bt );
		*value = strdup(buf);
	}
	else
		*value = strdup("0000-00-00T00:00:00");
    return TRX_OK;
}

TRX_STATUS setMSrvrInformTime(const char *value)
{
	extern char *strptime(const char *s, const char *format, struct tm *tm);
	struct tm bt;
	strptime(value,"%Y-%m-%dT%H:%M:%S", &bt );
	/* acsState.informTime = bt;  ???????????????????????*/
	return TRX_OK;
}

/* The ParameterKey needs to survice a power-off/reboot cycle */
TRX_STATUS getMSrvrParameterKey(char **value)
{
	*value = acsState.parameterKey?strdup(acsState.parameterKey):strdup("");
	return TRX_OK;
}
TRX_STATUS getConnectionReqURL(char **value)
{
	*value = acsState.connReqURL?strdup(acsState.connReqURL):strdup("");
	return TRX_OK;
}

TRX_STATUS getConnectionUsername(char **value)
{
	*value = acsState.connReqUser?strdup(acsState.connReqUser):strdup("");

	return TRX_OK;
}
TRX_STATUS setConnectionUsername(const char *value)
{
	free(acsState.connReqUser);
	acsState.connReqUser= strdup(value);
	return TRX_OK;
}
TRX_STATUS getConnectionPassword(char **value)
{
	*value = acsState.connReqPwd?strdup(acsState.connReqPwd):strdup("");

	return TRX_OK;
}
TRX_STATUS setConnectionPassword(const char *value)
{

	free(acsState.connReqPwd);
	acsState.connReqPwd= strdup(value);
	return TRX_OK;
}

TRX_STATUS getKickURL(char **value)
{
	*value = acsState.kickURL?strdup(acsState.kickURL):strdup("");
	return TRX_OK;
}

TRX_STATUS setKickURL(const char *value)
{
	free(acsState.kickURL);
	acsState.kickURL= strdup(value);
	return TRX_OK;
}
TRX_STATUS getUpgradesManaged(char **value)
{

	*value = strdup(acsState.upgradesManaged?"1":"0");
	return TRX_OK;
}

TRX_STATUS setUpgradesManaged(const char *value)
{
	acsState.upgradesManaged = testBoolean(value);
	return TRX_OK;
}

TRX_STATUS getDeviceSummary (char **value)
{

	*value = strdup("");
    return TRX_OK;
}

void reInitInstances(void)
{
	char	*value;

	/* get items that may have been change by WEB-UI */
	getMSrvrInformEnable(&value);
	/* need to free value since it's allocated memory by strdup        */
	if ( value != NULL ) free(value);
	getMSrvrInformInterval(&value);
	if ( value != NULL ) free(value);
	getMSrvrInformTime(&value);
	if ( value != NULL ) free(value);
	getMSrvrURL(&value);
	if ( value != NULL ) free(value);
	getMSrvrUsername(&value);
	if ( value != NULL ) free(value);
	getMSrvrPassword(&value);
	if ( value != NULL ) free(value);
	getConnectionUsername(&value);
	if ( value != NULL ) free(value);
	getConnectionPassword(&value);
	if ( value != NULL ) free(value);

#ifdef TR135_SUPPORT
    /* (re)init STBService object */
    /* printf("(re)init STBService\n"); */
    initSTBService();
#endif
#ifdef TR181_SUPPORT
    /* (re)init MoCA object */
    /* printf("(re)init MoCA\n"); */
    initMoca();
    /* (re)init Ethernet object */
    /* printf("(re)init Ethernet\n"); */
    initEthernet();
#endif
}

extern void initBaslineProfile(void);
TRXGFUNC(getProvisioningCode);

/*
* Called to initialize the interface to the BCM shared lib
* and also the instance objects as requried by the cfm configuration
*/
void initBCMLibIF(void)
{
	char *value;

	/* initBaslineProfile just gets timestamp */
	initBaslineProfile();
#if 0
	getUpgradesManaged(&value);
	if ( value != NULL ) free(value);	/*JJC */
	getConnectionReqURL(&value);
	if ( value != NULL ) free(value);	/*JJC */
	/*getMSrvrParameterKey(&value);*/
	getKickURL(&value);
	if ( value != NULL ) free(value);	/*JJC */
	getProvisioningCode(&value);
	if ( value != NULL ) free(value);	/*JJC */
#endif
	retrieveTR69StatusItems();

	reInitInstances();

	BSTD_UNUSED(value);
}

