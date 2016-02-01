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

#ifndef __APPDEF_H__
#define __APPDEF_H__

/* Buffer sizes */
/*
#ifndef BSTD_UNUSED
#define BSTD_UNUSED(x)  { volatile void *bstd_unused; bstd_unused = (void *)&(x); }
#endif
*/

#define MAXWEBBUFSZ	132217728	/* 2**28 - Default web message/download max buffer size */
#define PARAMLISTSZ (30000*2*4)    /*size of built-up parameter lists */
#define XMLBUFSZ    (35000*2*4)	 /*size of complete SOAP message */
#define ATTRIBUTESAVESZ	2048	/* maximum size of notification attribute */
								/* storage in scratch pad */
/* compile time options */
/* xml debug output options */
/* #define		DUMPSOAPOUT	*/	/* DUMP xml msg sent to ACS to stderr */
/* #define		DUMPSOAPIN 	*/	/* DUMP xml msg from ACS to stderr */
/* xml formatting options */
/* #define 	OMIT_INDENT	  */  /* don't intend xml tag lines - smaller messages */

/* use openSSL lib to support https: */
/* #define    USE_SSL */
/* #define    WRITEPIDFILE */

/* Allow Reboot and Factory Reset on ACS disconnect error */
#define		ALLOW_DISCONNECT_ERROR	/* acsDisconnect will call reboot and reset clean up*/
									/* even if there is an ACS disconnect error */
/* Authentication options */
#define		ALLOW_AUTH_RECONNECT	/* Some web servers such as Apache will close */
									/* the connection when sending a 401 status	  */
									/* This allows the CPE to close the current connection */
									/* and reconnect to the server with the Authorization */
									/* header in the first POST message. The CPE will */
									/* attempt this if the connection is closed following */
									/* the 401 status from the server */
									/* Undefining this will prohibit the CPE from sending */
									/* the Authorization on a new connection */

/* Generic compile time flags combination may be defined for specific ACS below */

#define GENERATE_SOAPACTION_HDR      /* generate SOAPAction header in POST reqs*/

/* TR-069 schema flags */
#if 0
//#define SUPPRESS_SOAP_ARRAYTYPE		/* suppress generation of soap-env arraytype */
									/* such as
										<Event SOAP-END:arrayType="cwmp:EventStruct[x]">
									   SUPPRESS_SOAP_ARRAYTYPE generates
									    <Event>
									***/
#endif
/* ACS Connection Initiation */
#define ACSCONN_PORT         30005       /* PORT to listen for acs connections */
#define ACSREALM    "IgdAuthentication"
#define ACSDOMAIN   "/"
#ifdef USE_SSL
#define ACS_URL   	"https://acs.qacafe.com:443"
#else
#define ACS_URL   	"http://6.0.0.1:80"
#endif
#define ACSCONNPATH "/"
#define ACS_USER 			"admin"
#define ACS_PASSWD 			"admin"
#define ACSCONN_USER 		"admin"
#define ACSCONN_PASSWD 		"admin"
#define ACS_KICK_URL 		"temp"
#define ACS_DEF_STRING 		"temp"
/* #define CFMLISTENPORT		 30006 */		/* port to listen for signals from cfm*/

/* Timer valuse */
/* #define	LOCK_CHECK_INTERVAL	 1000 */		/* interval to attempt to lock local CFM */
										/* configuration data */

#define ACSINFORMDELAY  500     /* initial delay (ms) after pgm start before */
                                /* sending inform to ACS */
#define CHECKWANINTERVAL    (60*1000) /* check wan up */
#define ACSRESPONSETIME    (30*1000) /* MAX Time to wait on ACS response */
/* #define CFMKICKDELAY		(3*1000) */ /* time to delay following a msg from the cfm*/
									/* before checking notifications. Allows the*/
									/* cfm to complete before starting */
						/* Retry intervals for ACS connect failures    */
						/* Retry time decays by
						   CONN_DECAYTIME*<consecutivefailures> upto a
						   maximum of CONNECT_DECAYTIME*CONNDECAYMAX */
#define CONN_DECAYMAX		6	/* maximum number for decaying multiple */
#define CONN_DECAYTIME		10	/* decay time per multiple */


/* Constants                           */

#define USER_AGENT_NAME    "GS_TR69_CPE-0.2"

/* #define TR69_DIR   "/var/tr69" */
/* #define TR69_PID_FILE  "/var/tr69/tr69pid" */

#define SHELL      "/bin/sh"

#define USE_CERTIFICATES
/* ACS Server Certificate File path */
/* #define	CERT_FILE	"/var/cert/acscert.cacert" */

#ifdef USE_CERTIFICATES
#define ACS_CIPHERS "RSA:DES:SHA+RSA:RC4:SAH+MEDIUM"
#else
/* #define ACS_CIPHERS "EXPORT40:SSLv3" */
#define ACS_CIPHERS "SSLv3"
#endif



/************************************************************/
/* compile time conditions for specific ACS                 */
/* Uncomment the required definition                        */
/************************************************************/
/* #define SUPPORT_ACS_CISCO	 */
/* #define SUPPORT_ACS_GS */
/* #define SUPPORT_ACS_DIMARK */
/* #define SUPPORT_ACS_PIRELLI */
/* #define SUPPORT_ACS_2WIRE	 */


/* set conditional compile flags based on ACS choice */
#ifdef SUPPORT_ACS_CISCO
/* #define    FORCE_NULL_AFTER_INFORM */      /* Set timer to force null http after sending Inform*/
/* #define 	SUPPRESS_EMPTY_PARAM */         /* Cisco doesn't handle <param></param> form */
/* #define	SUPPRESS_XML_NEWLINES */  /* replaces \n with space to avoid cisco tool parser problem*/
#endif /* SUPPORT_ACS_CISCO */



/* #ifdef */ /* next ACS ????? */

/*#endif*/   /*        */

#endif /* __APPDEF_H__ */

