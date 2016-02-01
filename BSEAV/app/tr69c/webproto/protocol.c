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
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <malloc.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/poll.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include "bstd.h"
#include "bkni.h"

#include "../inc/appdefs.h"

#ifdef USE_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include "../main/types.h"
#include "../main/event.h"
#include "../main/utils.h"

#include "protocol.h"
#include "www.h"

#define SERVER_NAME "milli_httpd"
#define PROTOCOL "HTTP/1.1"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

/*#define WRITETRACE*/ /* prints everything written to socket */
/*#define READTRACE*/  /* prints everything read to socket */
/*#define DEBUG*/      /* general debug log */
#ifdef DEBUG
#define DEBUGSSL /* log ssl io and connections */
/* Define DBGSSLC(X) to debug SSL connection and SSL_shutdown */
#define DBGSSLC(X) fprintf X
#define DBGPRINT(X) fprintf X
#else
#define DBGSSLC(X)
#define DBGPRINT(X)
#endif
/* #define DEBUGHDRS */ /* log html headers */

#include "../inc/tr69cdefs.h" /* defines for ACS state */
extern ACSState    acsState;
#ifdef DEBUGSSL
    #include "sys/time.h"
    #define mkstr(S) # S
    #define setListener(A,B,C) { fprintf(stderr, mkstr(%s setListener B fd=%d\n), getticks(), A); setListener( A,B,C); }
    #define setListenerType(A,B,C,E) { fprintf(stderr, mkstr(%s setListenerType B-E fd=%d\n), getticks(), A); setListenerType( A,B,C,E); }
    #define stopListener(A) { fprintf(stderr, "%s stopListener fd=%d\n", getticks(), A); stopListener( A ); }
#endif

#ifdef DEBUGSSL
/* extern void gettimeofday(struct timeval *tv, struct timezone *tz); */
char timestr[40];
char *getticks()
{
    struct timeval now;
    gettimeofday( &now,NULL);
    sprintf(timestr,"%04ld.%06ld", now.tv_sec%1000, now.tv_usec);
    return timestr;
}

#endif

#define BUF_SIZE_MAX 4096

#ifdef USE_SSL
static SSL_CTX *ssl_ctx = NULL;
#endif

#ifdef DEBUGSSL
static void showSocketStatus( unsigned fd)
{

    struct pollfd fdl[1];
    int e;

    fdl[0].fd = fd;
    fdl[0].events = 0xff;
    if ( (e=poll(fdl,1,0))<0)
        fprintf(stderr,"*poll() error\n");
    else
        fprintf(stderr, "poll=%0x\n",fdl[0].revents);
}
#endif

#ifdef USE_SSL
#ifdef USE_CERTIFICATES
/*
 * verify server certificate	 see SSL_CTS_set_verify()
 * Returns: 0 - verification failed: stop verifying in failed state.
 *          1 - continue  verifying.
 */
static int verify_callback(int ok, X509_STORE_CTX *store)
{
    char subject[BUF_SIZE_MAX], issuer[BUF_SIZE_MAX];
    char *pCN = NULL, *pEnd = NULL;
    X509 *cert;
    int  err, depth;

    cert = X509_STORE_CTX_get_current_cert(store);
    err = X509_STORE_CTX_get_error(store);
    depth = X509_STORE_CTX_get_error_depth(store);

	/*
	* Retrieve the pointer to the SSL of the connection currently treated
	* and the application specific data stored into the SSL object.
	*/

    X509_NAME_oneline(X509_get_subject_name(cert), subject, BUF_SIZE_MAX);
    X509_NAME_oneline(X509_get_issuer_name(store->current_cert), issuer, BUF_SIZE_MAX);

    if (ok == 0)
    {
        DBGPRINT((stderr, "verify_callback failed to verify cert: ok<%d> error_num = %d, err_msg = %s, depth = %d,\nsubject = %s,\nissuer = %s\n", ok, err, X509_verify_cert_error_string(err), depth, subject, issuer));
    }
    else
    {
        DBGPRINT((stderr, "verify_callback is good: ok<%d> error_num = %d, err_msg = %s, depth = %d,\nsubject = %s,\nissuer = %s\n", ok, err, X509_verify_cert_error_string(err), depth, subject, issuer));
    }

	if (subject != NULL)
	{
	    if ((pCN = strstr(subject, "CN=")) != NULL)
	    {
	         pCN += 3;   /* pass "CN=" to point to value of CN */
	         if ((pEnd = strchr(pCN, '/')) != NULL)
	         {
	             *pEnd = '\0';
#if 0 /* NOT Currently doing this check - May need to enable in the future */
	             /* check that the CN in the cert matches the ACS URL */
	             if (strstr(acsState.acsURL, pCN) != NULL)
	             {
	                 printf("SSL-- return X509_V_OK, CN = %s, URL = %s\n", pCN, acsState.acsURL);
	             }
	             else
	             {
	                 ok = 0;
	                 err = X509_V_ERR_APPLICATION_VERIFICATION;
	                 X509_STORE_CTX_set_error(store, err);
	                 printf("SSL-- return X509_V_ERR_APPLICATION_VERIFICATION, CN is not in URL, CN = %s, URL = %s\n", pCN, acsState.acsURL);
	             }
#endif
	         }
	    }
	}
	DBGPRINT((stderr, "SSL-- verify_callback is returing <%d>\n", ok));
	return ok;
}
#endif  /* USE_CERTIFICATES */
#endif  /* USE_SSL */

/*======================================================================*
 * Init
 *======================================================================*/
#if 0  /* historic only */
void proto_Init(void)
{
#ifdef USE_SSL
    SSL_load_error_strings();
    SSL_library_init();
    ssl_ctx = SSL_CTX_new(SSLv3_client_method());
    if (ssl_ctx == NULL) {
        slog(LOG_ERROR,"Could not create SSL context");
        exit(1);
    }
    if (! SSL_CTX_set_cipher_list(ssl_ctx, ACS_CIPHERS) )
        slog(LOG_ERROR, "Could not set cipher list for SSL");
#ifdef USE_CERTIFICATES
	{
	struct stat fstat;
	if ( lstat(CERT_FILE, &fstat)==0) {
		/* if CERT_FILE is present then setup cert verification */
		if (! SSL_CTX_use_certificate_chain_file(ssl_ctx, CERT_FILE)){
			slog(LOG_ERROR,"Error loading certificate chain" );
		}

		if (! SSL_CTX_load_verify_locations(ssl_ctx, CERT_FILE, NULL))
			slog(LOG_ERROR, "Could not load verify locations");
		SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, verify_callback);
	}
	}

#endif
    /*SSL_CTX_set_mode(ssl_ctx, SSL_MODE_AUTO_RETRY); */
	ERR_print_errors_fp(stderr);
    SSL_CTX_set_session_cache_mode(ssl_ctx, SSL_SESS_CACHE_OFF);
#endif
}
#else
void proto_Init(void)
{
    /* Define the cert here for now */
    /* Future enhancements will store certs in NonVol memory */

#ifdef USE_SSL
    unsigned char CDRouterCA_certRoot[963]={
        0x30,0x82,0x03,0xBF,0x30,0x82,0x03,0x28,0xA0,0x03,0x02,0x01,0x02,0x02,0x09,0x00,
        0xA8,0x02,0xD6,0xA9,0xF2,0x02,0x96,0x4D,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,
        0xF7,0x0D,0x01,0x01,0x05,0x05,0x00,0x30,0x81,0x9C,0x31,0x0B,0x30,0x09,0x06,0x03,
        0x55,0x04,0x06,0x13,0x02,0x55,0x53,0x31,0x16,0x30,0x14,0x06,0x03,0x55,0x04,0x08,
        0x13,0x0D,0x4E,0x65,0x77,0x20,0x48,0x61,0x6D,0x70,0x73,0x68,0x69,0x72,0x65,0x31,
        0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x07,0x13,0x0A,0x50,0x6F,0x72,0x74,0x73,0x6D,
        0x6F,0x75,0x74,0x68,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x0A,0x13,0x0A,0x71,
        0x61,0x63,0x61,0x66,0x65,0x2E,0x63,0x6F,0x6D,0x31,0x13,0x30,0x11,0x06,0x03,0x55,
        0x04,0x0B,0x13,0x0A,0x71,0x61,0x63,0x61,0x66,0x65,0x2E,0x63,0x6F,0x6D,0x31,0x13,
        0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0A,0x71,0x61,0x63,0x61,0x66,0x65,0x2E,
        0x63,0x6F,0x6D,0x31,0x21,0x30,0x1F,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,
        0x09,0x01,0x16,0x12,0x73,0x75,0x70,0x70,0x6F,0x72,0x74,0x40,0x71,0x61,0x63,0x61,
        0x66,0x65,0x2E,0x63,0x6F,0x6D,0x30,0x1E,0x17,0x0D,0x31,0x31,0x31,0x30,0x31,0x37,
        0x31,0x35,0x35,0x36,0x33,0x31,0x5A,0x17,0x0D,0x32,0x35,0x30,0x36,0x32,0x35,0x31,
        0x35,0x35,0x36,0x33,0x31,0x5A,0x30,0x81,0x9C,0x31,0x0B,0x30,0x09,0x06,0x03,0x55,
        0x04,0x06,0x13,0x02,0x55,0x53,0x31,0x16,0x30,0x14,0x06,0x03,0x55,0x04,0x08,0x13,
        0x0D,0x4E,0x65,0x77,0x20,0x48,0x61,0x6D,0x70,0x73,0x68,0x69,0x72,0x65,0x31,0x13,
        0x30,0x11,0x06,0x03,0x55,0x04,0x07,0x13,0x0A,0x50,0x6F,0x72,0x74,0x73,0x6D,0x6F,
        0x75,0x74,0x68,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x0A,0x13,0x0A,0x71,0x61,
        0x63,0x61,0x66,0x65,0x2E,0x63,0x6F,0x6D,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,
        0x0B,0x13,0x0A,0x71,0x61,0x63,0x61,0x66,0x65,0x2E,0x63,0x6F,0x6D,0x31,0x13,0x30,
        0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0A,0x71,0x61,0x63,0x61,0x66,0x65,0x2E,0x63,
        0x6F,0x6D,0x31,0x21,0x30,0x1F,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x09,
        0x01,0x16,0x12,0x73,0x75,0x70,0x70,0x6F,0x72,0x74,0x40,0x71,0x61,0x63,0x61,0x66,
        0x65,0x2E,0x63,0x6F,0x6D,0x30,0x81,0x9F,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,
        0xF7,0x0D,0x01,0x01,0x01,0x05,0x00,0x03,0x81,0x8D,0x00,0x30,0x81,0x89,0x02,0x81,
        0x81,0x00,0xCD,0x88,0x43,0xF6,0x6C,0x58,0x1E,0xD8,0x3C,0x88,0x72,0xEB,0x2C,0x73,
        0x24,0xE0,0x43,0xE4,0xCE,0x5D,0x75,0xD9,0xE7,0x42,0x20,0x7E,0x09,0xB2,0x66,0x2A,
        0x5F,0xCB,0xB5,0xF2,0x69,0xF6,0xD4,0x2E,0xC0,0xA4,0xD1,0x07,0x86,0x47,0x3A,0xF4,
        0x85,0xCF,0x6A,0x95,0x7C,0xD5,0x74,0xD5,0xDE,0xF5,0x97,0xBB,0xBB,0xB9,0x17,0xF6,
        0xC0,0x9B,0x17,0xB5,0xDD,0xDB,0x98,0x9A,0x66,0xE5,0xA9,0x6F,0x15,0xAE,0xF5,0x41,
        0x8B,0x7A,0x0B,0xBF,0xF0,0x51,0xA1,0x66,0x92,0x47,0x57,0x0F,0x26,0xE2,0x71,0x43,
        0xB6,0x6B,0x38,0xA1,0x89,0x52,0xA1,0xD7,0xB7,0xB7,0xC3,0xAF,0x8E,0xCE,0x04,0x8B,
        0x96,0x35,0xC1,0x26,0x3B,0x15,0xBE,0xC4,0x1C,0x44,0xBD,0x55,0x28,0xC7,0x26,0x85,
        0xD7,0xC5,0x02,0x03,0x01,0x00,0x01,0xA3,0x82,0x01,0x05,0x30,0x82,0x01,0x01,0x30,
        0x1D,0x06,0x03,0x55,0x1D,0x0E,0x04,0x16,0x04,0x14,0xE0,0x16,0xCF,0xCA,0x51,0x2B,
        0x2D,0x38,0x83,0x90,0x34,0x04,0x18,0xCF,0x7D,0x80,0x4F,0x9E,0x55,0xAB,0x30,0x81,
        0xD1,0x06,0x03,0x55,0x1D,0x23,0x04,0x81,0xC9,0x30,0x81,0xC6,0x80,0x14,0xE0,0x16,
        0xCF,0xCA,0x51,0x2B,0x2D,0x38,0x83,0x90,0x34,0x04,0x18,0xCF,0x7D,0x80,0x4F,0x9E,
        0x55,0xAB,0xA1,0x81,0xA2,0xA4,0x81,0x9F,0x30,0x81,0x9C,0x31,0x0B,0x30,0x09,0x06,
        0x03,0x55,0x04,0x06,0x13,0x02,0x55,0x53,0x31,0x16,0x30,0x14,0x06,0x03,0x55,0x04,
        0x08,0x13,0x0D,0x4E,0x65,0x77,0x20,0x48,0x61,0x6D,0x70,0x73,0x68,0x69,0x72,0x65,
        0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x07,0x13,0x0A,0x50,0x6F,0x72,0x74,0x73,
        0x6D,0x6F,0x75,0x74,0x68,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x0A,0x13,0x0A,
        0x71,0x61,0x63,0x61,0x66,0x65,0x2E,0x63,0x6F,0x6D,0x31,0x13,0x30,0x11,0x06,0x03,
        0x55,0x04,0x0B,0x13,0x0A,0x71,0x61,0x63,0x61,0x66,0x65,0x2E,0x63,0x6F,0x6D,0x31,
        0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0A,0x71,0x61,0x63,0x61,0x66,0x65,
        0x2E,0x63,0x6F,0x6D,0x31,0x21,0x30,0x1F,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,
        0x01,0x09,0x01,0x16,0x12,0x73,0x75,0x70,0x70,0x6F,0x72,0x74,0x40,0x71,0x61,0x63,
        0x61,0x66,0x65,0x2E,0x63,0x6F,0x6D,0x82,0x09,0x00,0xA8,0x02,0xD6,0xA9,0xF2,0x02,
        0x96,0x4D,0x30,0x0C,0x06,0x03,0x55,0x1D,0x13,0x04,0x05,0x30,0x03,0x01,0x01,0xFF,
        0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x05,0x05,0x00,0x03,
        0x81,0x81,0x00,0x4B,0x05,0x7E,0x45,0xC6,0x69,0xAA,0xB2,0x25,0xA9,0xCC,0x84,0x1A,
        0x35,0xA7,0x94,0xD5,0xE7,0x8E,0x5A,0xFE,0x68,0x88,0x0F,0xCE,0xAD,0xF5,0xA8,0xF8,
        0x86,0x10,0x2F,0x1C,0x0F,0xBD,0xC0,0xC6,0x10,0xBC,0x42,0xB2,0xE8,0x25,0x0C,0xDF,
        0x67,0xC6,0x3D,0x65,0x89,0xF3,0x44,0xC7,0xEC,0xAA,0x02,0xFD,0xF5,0x04,0xA8,0x65,
        0x77,0xF8,0x65,0xEE,0x42,0x7A,0x0C,0xCA,0xEE,0xCC,0xB8,0xBE,0xBE,0xAB,0xEF,0x07,
        0xBF,0xC1,0xA3,0xE7,0xA9,0x17,0x8B,0x8F,0x45,0x10,0x4B,0x3D,0x00,0x0E,0x1F,0x82,
        0x5A,0x33,0x78,0x97,0xDB,0x03,0xDF,0x9A,0xC1,0x77,0x3F,0xD1,0x8F,0x2B,0x2E,0x32,
        0x79,0x8C,0x18,0x74,0xCF,0x53,0xE2,0xF6,0x69,0x42,0xA3,0xAC,0x0C,0x59,0xE3,0x9F,
        0x85,0xE8,0x7B,
	};
    int AcsCACertLength_1 = 963;
    unsigned char *pAcsCACert_1 = &CDRouterCA_certRoot[0];
    X509 *x509_ServiceProviderCACert = NULL;
    int i;

    DBGPRINT((stderr, "** SSL- USING the acs.qacafe.com-ca-self.pem certificate\n"));
    DBGPRINT((stderr, "** SSL- USING the single certificate from source len<%d>\n",AcsCACertLength_1));

    /* Initialize the library */
    SSL_library_init();
    SSLeay_add_all_algorithms();
    SSL_load_error_strings();

    ssl_ctx = SSL_CTX_new(SSLv23_client_method()); /*-TLSv1_client_method(),SSLv23_method() */

    if ( ssl_ctx == NULL )
    {
        DBGPRINT((stderr, "Unable to create new ssl_ctx!\n"));
        return ;
    }

    x509_ServiceProviderCACert = d2i_X509(NULL,(const unsigned char **)&pAcsCACert_1,AcsCACertLength_1);

    if ( x509_ServiceProviderCACert == NULL )
    {
        DBGPRINT((stderr, "ServiceProviderCA Cert INVALID!\n"));
        return ;
    }

    i = X509_STORE_add_cert(ssl_ctx->cert_store,x509_ServiceProviderCACert);
    if ( !i )
    {
        DBGPRINT((stderr, "Unable to Load Service Provider CA Cert into CTX Structure!\n"));
        return;
    }

    SSL_CTX_set_default_verify_paths(ssl_ctx);
    SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL|SSL_OP_NO_SSLv2);

    SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, verify_callback);

#if 0 /* 0ptional */
    if ( SSL_CTX_set_cipher_list(ssl_ctx, "RC4-SHA") != 1 )
    {
        gLogMessageRaw
        << "Error setting Cipher list. NO VALID CIPHERS!" << endl;
        return ;
    }
    SSL_CTX_set_mode(ssl_ctx, SSL_MODE_AUTO_RETRY);
#endif

#endif /* USE_SSL */

    return;
}

#endif
/*======================================================================*
 * Ctx
 *======================================================================*/
/*----------------------------------------------------------------------*/
tProtoCtx *proto_NewCtx(int fd)
{
    tProtoCtx *pc;

    pc = (tProtoCtx *) malloc(sizeof(tProtoCtx));
    memset(pc, 0, sizeof(tProtoCtx));
    pc->type = iNormal;
    pc->fd = fd;
    return pc;
}
#ifdef USE_SSL
/*----------------------------------------------------------------------*/
static void server_wait_for_ssl(void *handle) {
    tProtoCtx *pc = handle;
    int res;

#ifdef DEBUGSSL
    fprintf(stderr,"%s server_wait_for_ssl() SSL connect fd=%d ", getticks(), pc->fd);
    showSocketStatus(pc->fd);
#endif
    /* stopListener(pc->fd); */
   if ((pc->sslConn = SSL_connect(pc->ssl)) <= 0)
   {
      int sslres;
      sslres = SSL_get_error(pc->ssl, pc->sslConn);
      if (sslres == SSL_ERROR_WANT_READ)
      {
#ifdef DEBUGSSL
            fprintf(stderr,"%s SSL connection wants to read fd=%d\n", getticks(), pc->fd);
#endif
            setListener(pc->fd, server_wait_for_ssl, pc);
      }
      else if (sslres == SSL_ERROR_WANT_WRITE)
      {
#ifdef DEBUGSSL
            fprintf(stderr,"%s SSL connection wants to write fd=%d\n",getticks(), pc->fd);
#endif
            setListenerType(pc->fd, server_wait_for_ssl, pc, iListener_Write);
      }
      else
      {
         char errstr[256];
         ERR_error_string_n(sslres, errstr, 256);
         DBGPRINT((stderr, "Error connecting to server(Possible certificate error): (res=%d,sslres=%d) %s", pc->sslConn, sslres, errstr));
         (*(pc->cb))(pc->data, PROTO_ERROR_SSL);
         /* Note: pc may have been freed by the callback */
         return;
      }
   }
   else
   {
#ifdef DEBUGSSL
        fprintf(stderr,"%s SSL server_wait_for_ssl callback fd=%d\n",getticks(), pc->fd);
#endif
        (*(pc->cb))(pc->data, PROTO_OK);
    }
}

/*----------------------------------------------------------------------*
 * callback errorcodes
 *  PROTO_OK          all ok
 *  PROTO_ERROR       generic error
 *  PROTO_ERROR_SSL   ssl error
 */
void proto_SetSslCtx(tProtoCtx *pc, tProtoHandler cb, void *data)
{
    pc->type = iSsl;
    if (pc->ssl == NULL)
    {
        pc->ssl = SSL_new(ssl_ctx);
    }

    if (pc->ssl != NULL)
    {
        DBGSSLC((stderr, "%s proto_SetSslCtx()ssl=%p fd=%d\n",getticks(),pc->ssl,pc->fd));
        if ( SSL_set_fd(pc->ssl, pc->fd)>0 )
        {
            pc->cb = cb;
            pc->data = data;
            /* TBD: add a timeout */
            server_wait_for_ssl(pc);
        }
        else
        {
            slog(LOG_ERROR, "SSL_set_fd failed");
        }
    }
    else
    {
        slog(LOG_ERROR,"SSL_new failed");
    }
}

static void postShutdownCleanUp( void *handle)
{
	tProtoCtx	*pc = (tProtoCtx *)handle;

	DBGSSLC( (stderr,"%s  postShutdownCleanUp() ssl=%p fd=%d\n", getticks(), pc->ssl, pc->fd) );
	stopTimer( postShutdownCleanUp, (void*)pc );
	stopListener(pc->fd);
	close(pc->fd);
	if (pc->ssl)
		SSL_free(pc->ssl);
    pc->sslConn = 0;
	free(pc);
	return;
}
static void wait_for_sslShutdown( void *handle)
{
	tProtoCtx	*pc = (tProtoCtx *)handle;
	int	r;

	DBGSSLC((stderr, "%s  wait_for_sslShutdown()fd=%d\n", getticks(), pc->fd));
	r= SSL_shutdown(pc->ssl);
	DBGSSLC((stderr, "%s  SSL_shutdown= %d ssl=%p fd=%d\n", getticks(), r, pc->ssl, pc->fd ));
	if (r == 0) {
		/* started shutdown -- now call again */
		r= SSL_shutdown(pc->ssl);
		DBGSSLC((stderr, "%s  2nd SSL_shutdown= %d ssl=%p fd=%d\n", getticks(), r, pc->ssl, pc->fd) );
	}
	if( r == 1 ) {
		postShutdownCleanUp(pc);
	} else if ( r == -1 ) {
		int sslres, sslerrno;
		errno = 0;
		sslres = SSL_get_error(pc->ssl, r );
		sslerrno = ERR_get_error();
		if (sslres == SSL_ERROR_WANT_READ) {
			DBGSSLC((stderr,"%s SSL_shutdown wants to read fd=%d\n", getticks(), pc->fd));
			setListener(pc->fd, wait_for_sslShutdown, pc);
		} else if (sslres == SSL_ERROR_WANT_WRITE) {
			DBGSSLC((stderr,"%s SSL_shutdown wants to write fd=%d\n",getticks(), pc->fd));
			setListenerType(pc->fd, wait_for_sslShutdown, pc, iListener_Write);
		} else {
			char errstr[256];
			ERR_error_string_n(sslres,errstr,256);
			slog(LOG_ERROR, "SSL_shutdown server: (r=%d,sslres=%d) %s (sslerrno=%d) (strerror(errno=%d):%s)",
				r, sslres, errstr, sslerrno, errno, strerror(errno));
			postShutdownCleanUp( pc );
			return;
		}
	} else {
		DBGSSLC((stderr, "%s  SSL_shutdown state error ssl=%p fd=%d\n", getticks(), pc->ssl, pc->fd));
		postShutdownCleanUp(pc);
	}
	return;
}
#endif

/*----------------------------------------------------------------------*/
/* For iNormal protoCtx stopListener and close the fd					*/
/* For iSSL start shutdown												*/

void proto_FreeCtx(tProtoCtx *pc)
{
    switch (pc->type) {
    case iNormal:
	stopListener(pc->fd);
	close(pc->fd);
	free (pc);
        break;
#ifdef USE_SSL
    case iSsl:
	stopListener(pc->fd);
	DBGSSLC((stderr, "%s   proto_FreeCtx()ssl=%p fd=%d\n",getticks(),pc->ssl,pc->fd));
	setTimer(postShutdownCleanUp, (void *)pc, 3000);
        if (pc->ssl) {
		/* not completed */
		wait_for_sslShutdown(pc);
        } else
		postShutdownCleanUp(pc);
        break;
#endif
    default:
        slog(LOG_ERROR,"Impossible error; proto_FreeCtx() illegal ProtoCtx type (%d)", pc->type);
        free(pc);
        break;
    }
}

/*======================================================================*
 * Util
 *======================================================================*/
#ifdef USE_SSL
typedef struct {
    tProtoCtx *pc;
    tSSLIO    iofunc;
    char *ptr;
    int nbytes;
    tProtoHandler cb;
    void *userdata;
} SSL_io_ctx;
/*
* The following SSL io routines ensure that the parameters are saved
 * and restored in the subsequent call to SSL_read/write whenever the
 * functions return a -1 indicating non-blocking inprogress io.
 */
static void SSL_io_handler(void *handle) {
    SSL_io_ctx *rc = handle;
    int nresult;
    int sslres;
    if (rc->iofunc == sslRead ) {
        nresult = SSL_read(rc->pc->ssl, (void *)rc->ptr, rc->nbytes);
#ifdef DEBUGSSL
        fprintf(stderr, "%s SSL_io_handler read ssl=%p socket=%d nresult=%d\n", getticks(), rc->pc->ssl, rc->pc->fd, nresult);
#endif
    } else {
        nresult = SSL_write(rc->pc->ssl, (void *)rc->ptr, rc->nbytes);
#ifdef DEBUGSSL
        fprintf(stderr, "%s SSL_io_handler write ssl=%p socket=%d nresult=%d\n", getticks(),rc->pc->ssl, rc->pc->fd, nresult);
#endif
    }
    if (nresult < 0) {
        sslres = SSL_get_error(rc->pc->ssl, nresult);
        if (sslres == SSL_ERROR_WANT_READ) {
#ifdef DEBUGSSL
            fprintf(stderr, "%s SSL connection listen to read fd=%d\n", getticks(), rc->pc->fd);
#endif
            setListener(rc->pc->fd, SSL_io_handler, rc);
            return;
        } else if (sslres == SSL_ERROR_WANT_WRITE) {
#ifdef DEBUGSSL
            fprintf(stderr, "%s SSL connection listen to write fd=%d\n",getticks(), rc->pc->fd);
#endif
            setListenerType(rc->pc->fd, SSL_io_handler, rc, iListener_Write);
            return;
        }
        slog(LOG_DEBUG,"SSL_io_handler %s error fd=%d errcode=%d",
             rc->iofunc==sslRead? "read": "write", rc->pc->fd, sslres);
        return;
    }
    /* If we get here, we're done */

    stopListener(rc->pc->fd);
    (*(rc->cb))((void*)rc->userdata, nresult);
    free(rc);
}

/*----------------------------------------------------------------------*/
int proto_SSL_IO(tSSLIO func, tProtoCtx *pc, char *ptr, int nbytes, tProtoHandler cb, void *data) {

    SSL_io_ctx *rc;
    int nresult = 0;

    if (func == sslRead) {
        nresult = SSL_read(pc->ssl, ptr, nbytes);
#ifdef DEBUGSSL
        fprintf(stderr, "%s proto_SSL_io read fd=%d nresult=%d\n", getticks(), pc->fd,nresult);
#endif
    } else if (func == sslWrite) {
        nresult = SSL_write(pc->ssl, ptr, nbytes);
#ifdef DEBUGSSL
        fprintf(stderr, "%s proto_SSL_io write fd=%d nresult=%d\n",getticks(), pc->fd,nresult);
#endif
    }
    if (nresult < 0) {
        int sslres = SSL_get_error(pc->ssl, nresult);
        rc = (SSL_io_ctx *)malloc(sizeof(SSL_io_ctx));
        if (!rc)
            return -2;
        memset(rc, 0, sizeof(SSL_io_ctx));
        rc->iofunc = func;
        rc->pc = pc;
        rc->ptr = ptr;
        rc->nbytes = nbytes;
        rc->cb = cb;
        rc->userdata = data;
        if (sslres == SSL_ERROR_WANT_READ) {
#ifdef DEBUGSSL
            fprintf(stderr, "%s SSL_IO connection listen to read fd=%d\n", getticks(),rc->pc->fd);
#endif
            setListenerType(rc->pc->fd, SSL_io_handler, rc,iListener_ReadWrite);
        } else if (sslres == SSL_ERROR_WANT_WRITE) {
#ifdef DEBUGSSL
            fprintf(stderr, "%s SSL_IO connection listen to write fd=%d\n", getticks(),rc->pc->fd);
#endif
            setListenerType(rc->pc->fd, SSL_io_handler, rc, iListener_Write);
        } else {
#ifdef DEBUGSSL
            fprintf(stderr, "%s SSL_IO fd=%d error=%d\n", getticks(),rc->pc->fd, sslres);
#endif
            free (rc);
        }
    }
    return nresult;
}
#endif
/*----------------------------------------------------------------------*/

static int read_with_timeout(int fd, void *ptr, int nbytes, struct timeval *timeout)
{
	fd_set set;
	int rv;
	int nread = -99;

	FD_ZERO(&set); /* clear the set */
	FD_SET(fd, &set); /* add our file descriptor to the set */

	rv = select(fd + 1, &set, NULL, NULL, timeout);
	if (rv == -1) {
		perror("select"); /* an error accured */
		slog(LOG_ERROR, "select() failed");
	} else if (rv == 0) {
		slog(LOG_ERROR, "read() timeout"); /* a timeout occured */
	} else
		nread = read(fd, ptr, nbytes); /* there was data to read */

	return nread;
}

/* blocking read */
int proto_ReadWait(tProtoCtx *pc, char *ptr, int nbytes)
{
    int nread=0;
    int flags, bflags;
	struct timeval timeout;

	/* set the timeout value to 1 second */
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

    /* turn on synchroneous I/O, this call will block. */
    {
        flags = (long) fcntl(pc->fd, F_GETFL);
        bflags = flags & ~O_NONBLOCK; /* clear non-block flag, i.e. block */
        fcntl(pc->fd, F_SETFL, bflags);
    }

    errno = 0;
    switch (pc->type)
	{
	    case iNormal:
			/* nread = read(pc->fd, ptr, nbytes); */
	        nread = read_with_timeout(pc->fd, ptr, nbytes, &timeout);
	        break;
#ifdef USE_SSL
	    case iSsl:
#ifdef DEBUGSSL
	        fprintf(stderr, "%s read_SSL(%d, lth=%d)", getticks(),pc->fd, nbytes);
	        fprintf(stderr, " result=%d\n", nread = SSL_read(pc->ssl, (void *) ptr, nbytes));
#else
	        nread = SSL_read(pc->ssl, (void *) ptr, nbytes);
#endif
	        break;
#endif
	    default:
	        slog(LOG_ERROR, "Impossible error; readn() illegal ProtoCtx type (%d)", pc->type);
	        break;
    }
    if (nread > nbytes) {
        slog(LOG_ERROR, "proto_READ of %d returned %d", nbytes, nread);
    }

    fcntl(pc->fd, F_SETFL, flags); /* remove blocking flags */

#ifdef READTRACE
    {   int i;
        fprintf(stderr, "\n");
        for (i=0; i<nread; ++i)
        {
            if ((i != 0) && (i%16 == 0)) fprintf(stderr, "\n");
            fprintf(stderr, "%02X ", *(ptr+i));
        }
        fprintf(stderr, "\n\n");
    }
#endif

    return nread;
}
/* */
/*----------------------------------------------------------------------*/
int proto_Readn(tProtoCtx *pc, char *ptr, int nbytes)
{
    int nleft, nread=0;
    int   errnoval;

    nleft = nbytes;
    while (nleft > 0) {
        errno =0;
        switch (pc->type) {
        case iNormal:
            nread = read(pc->fd, ptr, nleft);
            break;
#ifdef USE_SSL
        case iSsl:
#ifdef DEBUGSSL
            fprintf(stderr, "%s SSL_read(%d, lth=%d)", getticks(),pc->fd, nleft);
            nread = SSL_read(pc->ssl, (void *) ptr, nleft);
            fprintf(stderr, " result=%d\n", nread);
#else
            nread = SSL_read(pc->ssl, (void *) ptr, nleft);
#endif
            break;
#endif
        default:
            slog(LOG_ERROR, "Impossible error; readn() illegal ProtoCtx type (%d)", pc->type);
            break;
        }

        if (nread < 0)
		{                                           /* This function will read until the byte cnt*/
            errnoval=errno;                         /* is reached or the return is <0. In the case*/
            if (errnoval==EAGAIN )                  /* of non-blocking reads this may happen after*/
                return nbytes-nleft;                /* some bytes have been retrieved. The EAGAIN*/
            else                                    /* status indicates that more are coming */
                                                    /* Other possibilites are ECONNRESET indicating*/
                /* that the tcp connection is broken */
                fprintf(stderr,"!!!!!!!! read(fd=%d) error=%d\n", pc->fd, errnoval);
            return nread; /* error, return < 0 */
        }
		else if (nread == 0)
        {
            break; /* EOF */
        }
        nleft -= nread;
        ptr += nread;
    }

    return nbytes - nleft; /* return >= 0 */
}
/*
 * Return number of bytes written or -1.
 * If -1 check for errno for EAGAIN and recall.
 *----------------------------------------------------------------------*/
int proto_Writen(tProtoCtx *pc, const char *ptr, int nbytes)
{
    int  nwritten=0;
    errno = 0;
    switch (pc->type) {
    case iNormal:
        nwritten = write(pc->fd, ptr, nbytes);
        break;
#ifdef USE_SSL
    case iSsl:
#ifdef DEBUGSSL
        fprintf(stderr, "%s SSL_write(%d, lth=%d)", getticks(),pc->fd, nbytes);
        nwritten = SSL_write(pc->ssl, ptr, nbytes);
        fprintf(stderr, "result=%d\n", nwritten);
#else
        nwritten = SSL_write(pc->ssl, ptr, nbytes);
#endif
        break;
#endif
    default:
        slog(LOG_ERROR, "Impossible error; writen() illegal ProtoCtx type (%d)", pc->type);
        break;
    }

#ifdef WRITETRACE
	if ( nwritten>0 )
	{
		int  i;
        fprintf(stderr, "\n");
		for (i=0;i<nwritten;++i)
		{
			if ((i != 0) && (i%16 == 0)) fprintf(stderr, "\n");
			fprintf(stderr,"%02X ", *(ptr+i));
		}
        fprintf(stderr, "\n\n");
	}
#endif

    if (nwritten <= 0)
	{
        if (errno!=EAGAIN)
        {
            /* fprintf(stderr,"proto_Writen() status = %d Error%s(%d)\n",nwritten,strerror(errno),errno); */
            return nwritten;
        }
		/* else fprintf(stderr,"proto_Writen() status = %d Error%s(%d)\n",nwritten,strerror(errno),errno); */
	}
    /* if (nwritten != nbytes) fprintf(stderr,"proto_Writen() short write rlth=%d actual=%d\n", nbytes, nwritten); */
    return nwritten;
}

/*----------------------------------------------------------------------*
 * Read a line from a descriptor. Read the line one byte at a time,
 * looking for the newline. We store the newline in the buffer,
 * then follow it with a \0 (the same as fgets).
 * We return the number of characters up to, but not including,
 * the \0 (the same as strlen).
 */

int proto_Readline(tProtoCtx *pc, char *buf, int maxlen)
{
    int n, rc;
    char   *ptr = buf;
    char c;
    int flags, bflags;

    /* turn on synchroneous I/O, this call will block. */
    {
        flags = (long) fcntl(pc->fd, F_GETFL);
        bflags = flags & ~O_NONBLOCK; /* clear non-block flag, i.e. block */
        fcntl(pc->fd, F_SETFL, bflags);
    }

    for (n = 1; n < maxlen; n++) {
        rc = proto_Readn(pc, &c, 1);
        if (rc == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            if (n == 1) {
                fcntl(pc->fd, F_SETFL, flags); /* TBD: fix part2, remove blocking flags */
                return 0; /* EOF, no data read */
            } else
                break;    /* EOF, some data was read */
        } else {
#ifdef DEBUG
            fprintf(stderr, "ERROR: proto_Readline fd=%d (%d)\n", pc->fd, errno);
#endif
            fcntl(pc->fd, F_SETFL, flags); /* remove blocking flags */
            return -1; /* ERROR */
        }
    }

    *ptr = '\0';

#ifdef READTRACE
    fprintf(stderr, "%s", buf);
#endif

    fcntl(pc->fd, F_SETFL, flags); /* remove blocking flags */
    return n;
}


/*----------------------------------------------------------------------*/
void proto_Printline(tProtoCtx *pc, const char *fmt, ...)
{
    char *p;
    va_list ap;
    int n;
    int size;

    size = 1024;
    if ((p = malloc(size)) == NULL) {
        slog(LOG_ERROR, "failed to malloc(%d)", size);
        return;
    }

	errno = 0;
    while (1) {
        /* try to print in the allocated space */
        va_start(ap, fmt);
        n = vsnprintf(p, size, fmt, ap);
        va_end(ap);

        if (n < 0) {
            slog(LOG_WARNING, "fdprintf() vsnprintf failed *%d): %s (%d) fmt=\"%s\"\n", n, strerror(errno), errno, fmt);
#ifdef DEBUG
            fprintf(stderr, "vsnprintf failed (%d): %s (%d) fmt=\"%s\"\n", n, strerror(errno), errno, fmt);
#endif
            return;
        } else if (n >= 0 && n < size) {
            /* print succeeded, let's write it on outstream */
            proto_Writen(pc, p, n);
            free(p);
            return;
        } else {
#ifdef DEBUG
            fprintf(stderr, "vsnprintf, only wrote %d bytes, retrying: fmt=\"%s\" strlen(fmt)=%d size=%d\n",
                    n, fmt, strlen(fmt), size);
#endif
            size *= 2;
            if ((p = realloc(p, size)) == NULL) {
                slog(LOG_ERROR, "failed to realloc(%d)", size);
                return;
            }
        }
    }/*end while(1)*/
}

/*======================================================================*
 * Data
 *======================================================================*/
tHttpHdrs *proto_NewHttpHdrs()
{
    tHttpHdrs *p;

    p = (tHttpHdrs *) malloc(sizeof(tHttpHdrs));
    if (p == NULL) {
        return NULL;
    }
    memset(p, 0, sizeof(tHttpHdrs));
    return p;
}

/*----------------------------------------------------------------------*/
void proto_FreeHttpHdrs(tHttpHdrs *p)
{
    CookieHdr	*cp, *last;
    free(p->content_type);
    free(p->protocol);
    free(p->wwwAuthenticate);
    free(p->Authorization);
    free(p->TransferEncoding);
    free(p->Connection);
    free(p->method);
    free(p->path);
    free(p->host);
    cp = p->setCookies;
    while (cp) {
        last = cp->next;
        free(cp->name);
        free(cp->value);
        free(cp);
        cp = last;
    }
    free(p->message);
    free(p->locationHdr);
    free(p->filename);
    free(p->arg);
    free(p);
}

/*======================================================================*
 * Sending
 *======================================================================*/
/*----------------------------------------------------------------------*/
void proto_SendRequest(tProtoCtx *pc, const char *method, const char *url)
{
    char buf[BUF_SIZE_MAX];
    int len;

    len = snprintf(buf,BUF_SIZE_MAX, "%s %s HTTP/1.1\r\n", method, url);
    if (len != proto_Writen(pc, buf, len)) {
        /* error in sending */
        ;
    }
#ifdef DEBUG
    fprintf(stderr, "proto_SendRequest(%s %s HTTP/1.1)\n", method, url);
#endif
}

/*----------------------------------------------------------------------*/
void proto_SendCookie(tProtoCtx *pc, CookieHdr *c)
{
    char buf[BUF_SIZE_MAX];
    int len;

    len = snprintf(buf,BUF_SIZE_MAX, "Cookie: %s=%s\r\n", c->name, c->value);
    if (len != proto_Writen(pc, buf, len)) {
        /* error in sending */
        ;
    }
/*
#ifdef DEBUGHDRS
    fprintf(stderr, "proto_SendCookie -> %s", buf);
#endif
*/
}
/*----------------------------------------------------------------------*/
void proto_SendHeader(tProtoCtx *pc,  const char *header, const char *value)
{
    char buf[BUF_SIZE_MAX];
    int len;

    if ( header == NULL || value == NULL ) return;

    len = snprintf(buf,BUF_SIZE_MAX, "%s: %s\r\n", header, value);
    if (len != proto_Writen(pc, buf, len)) {
        /* error in sending */
        ;
    }
/*
#ifdef DEBUGHDRS
    fprintf(stderr, "proto_SendHeader -> %s", buf);
#endif
*/
}

/*----------------------------------------------------------------------*/
void proto_SendRaw(tProtoCtx *pc, const char *arg, int len)
{
	int	wlth;
	int totWlth = 0;
    int eagainCounter = 0;
    int eagainCounterM = 0;

    /* printf("proto_SendRaw is starting with a data<%p> and  len <%d>  on fd<%d>\n", arg, len, pc->fd); */

    if (*arg!='<' && *arg!= '\r') {
        /* debuggging*/
        DBGPRINT((stderr, "!!%10.10s!!\n", arg));
    }

    while ( totWlth<len )
    {
        if ( (wlth = proto_Writen(pc, arg+totWlth, len-totWlth)) >= 0)
        {
            /* some or all data sent*/
            totWlth += wlth;
            continue;
        }
        else
        {
            if ( errno == EAGAIN )
            {
                /* can't send is all keep trying -- busy wait on writes */
                eagainCounter++;
                if(eagainCounter > 100)
                {
					BKNI_Sleep(10); /* 10ms */
                    eagainCounter = 0;
                    eagainCounterM++;
                }
                continue;
            }
            /* data send error */
            printf("\nproto_SendRaw: Data Send ERROR <%d>\n",errno);
            break;
        }
    }
    DBGPRINT((stderr, "\nFinished with proto_SendRaw, Sent<%d>   EAGAIN errors<%d>, Counter resets <%d>\n\n ",totWlth, eagainCounter, eagainCounterM));
}

/*----------------------------------------------------------------------*/
void proto_SendEndHeaders(tProtoCtx *pc)
{
#ifdef DEBUGHDRS
    fprintf(stderr, "proto_SendEndHeaders()\n");
#endif
    proto_SendRaw(pc, "\r\n", 2);
}

/*----------------------------------------------------------------------*/
void proto_SendHeaders(tProtoCtx *pc, int status, const char* title, const char* extra_header, const char* content_type)
{
    time_t now;
    char timebuf[100];

    proto_Printline(pc, "%s %d %s\r\n", PROTOCOL, status, title);
    now = time((time_t*) 0);
    (void) strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
    proto_Printline(pc, "Date: %s\r\n", timebuf);
    proto_Printline(pc, "MIME-Version: 1.0\r\n");
    proto_Printline(pc, "Server: %s\r\n", SERVER_NAME);
    proto_Printline(pc, "Connection: Close\r\n" );
    if (extra_header != NULL)
        proto_Printline(pc, "%s\r\n", extra_header );
    if (content_type != NULL)
        proto_Printline(pc, "Content-Type: %s\r\n", content_type);
    proto_Printline(pc, "\r\n" );
}

/*----------------------------------------------------------------------*/
void proto_SendRedirect(tProtoCtx *pc, const char *host, const char* location)
{
    char header[BUF_SIZE_MAX];
    char slash[2];

    if (location[0] == '/')
        strcpy(slash, "");
    else
        strcpy(slash, "/");
#ifdef DEBUG
    fprintf(stderr, "web: proto_SEndRedirect: host=%s location=%s\n",host, location);
#endif
    (void) snprintf(header, sizeof(header), "Location: http://%s%s%s", host, slash, location);
    proto_SendHeaders(pc, 307, "Redirect", header, "text/html");
#ifdef DEBUG
    fprintf(stderr, "web: proto_SEndRedirect: %s\n", header);
#endif
}


/*----------------------------------------------------------------------*/
void proto_SendRedirectProtoHost(tProtoCtx *pc, const char *protohost, const char* location)
{
    char header[BUF_SIZE_MAX];

    (void) snprintf(header, sizeof(header), "Location: %s%s", protohost, location);
    proto_SendHeaders(pc, 307, "Redirect", header, "text/html");
#ifdef DEBUG
    fprintf(stderr, "web: proto_SendRedirectProtoHost: %s\n", header);
#endif
}

/*----------------------------------------------------------------------*/
void proto_SendRedirectViaRefresh(tProtoCtx *pc, const char *host, const char* location)
{
    char slash[2];

    if (location[0] == '/')
        strcpy(slash, "");
    else
        strcpy(slash, "/");

    proto_SendHeaders(pc, 200, "Ok", NULL, "text/html");
    proto_Printline(pc, "<HTML><HEAD><TITLE>Redirecting to requested site...</TITLE>\n");
    proto_Printline(pc, "<meta http-equiv=\"refresh\" content=\"0;URL=http://%s%s%s\"></HEAD>\n",
                    host, slash, location);
}

/*----------------------------------------------------------------------*/
void proto_SendError(tProtoCtx *pc, int status, const char* title, const char* extra_header, const char* text)
{
    proto_SendHeaders(pc, status, title, extra_header, "text/html");
    proto_Printline(pc, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n", status, title);
    proto_Printline(pc, "<BODY BGCOLOR=\"#cc9999\"><H4>%d %s</H4>\n", status, title);
    proto_Printline(pc, "%s\n", text );
    proto_Printline(pc, "</BODY></HTML>\n");
}

/*======================================================================*
 * Receiving
 *======================================================================*/
/*----------------------------------------------------------------------*
 * return
 *   0 if ok
 *  -1 on failure
 */
int proto_ParseRequest(tProtoCtx *pc, tHttpHdrs *hdrs)
{
    char buf[BUF_SIZE_MAX];
    char method[BUF_SIZE_MAX];
    char path[BUF_SIZE_MAX];
    char protocol[BUF_SIZE_MAX];

    /* Parse the first line of the request. */
    if (proto_Readline(pc, buf, BUF_SIZE_MAX) <= 0) {
#ifdef DEBUG
        fprintf(stderr, "DEBUG: error =%d proto_ParseRequest() proto_Readline() rtns empty\n",
                errno);
#endif
        return -1;
    }
    if (sscanf(buf, "%[^ ] %[^ ] %[^ ]", method, path, protocol) != 3) {
#ifdef DEBUG
        fprintf(stderr,"sscanf error on >>%s<<\n",buf);
#endif
        return -1;
    }

    www_StripTail(method);
    www_StripTail(path);
    www_StripTail(protocol);
    free(hdrs->method);
    hdrs->method = strdup(method);
    free(hdrs->path);
    hdrs->path = strdup(path);
    free(hdrs->protocol);
    hdrs->protocol = strdup(protocol);

#ifdef DEBUGHDRS
    fprintf(stderr, "proto_ParseRequest method=\"%s\" path=\"%s\" protocol=\"%s\"\n",
         hdrs->method, hdrs->path, hdrs->protocol);
#endif
    return 0; /* OK */
}

/*----------------------------------------------------------------------*
 * return
 *   0 if ok
 *  -1 on failure
 */
int proto_ParseResponse(tProtoCtx *pc, tHttpHdrs *hdrs)
{
    char buf[BUF_SIZE_MAX];
    char protocol[BUF_SIZE_MAX];
    char status[BUF_SIZE_MAX];
    char message[BUF_SIZE_MAX];

#ifdef DEBUGSSL
    fprintf(stderr, "%s proto_ParseResponse()\n", getticks());
#endif
    /* Parse the first line of the request. */
    if (proto_Readline(pc, buf, BUF_SIZE_MAX) <= 0) {
        return -1;
    }

    if (sscanf(buf, "%[^ ] %[^ ] %[^\r]", protocol, status, message ) != 3) {
#ifdef DEBUG
        fprintf(stderr,"sscanf error on >>%s<<\n",buf);
#endif

        return -1;
    }

    www_StripTail(protocol);
    www_StripTail(status);
    www_StripTail(message);
    free(hdrs->protocol);
    hdrs->protocol = strdup(protocol);
    hdrs->status_code = atoi(status); /* TBD: add sanity check */
    free(hdrs->message);
    hdrs->message = strdup(message);
#ifdef DEBUG
    fprintf(stderr, "proto_ParseResponse(protocol=\"%s\", status=%d message=\"%s\")\n",
            hdrs->protocol, hdrs->status_code, hdrs->message);
#endif

    return 0; /* OK */
}

static char HostStr[]="Host:";
static char ConnectionStr[]="Connection:";
static char SetCookieStr[]="Set-Cookie:";
static char SetCookieStr2[]="Set-Cookie2:";
static char ContentLthStr[]="Content-Length:";
static char ContentTypeStr[]="Content-Type:";
static char WWWAuthenticateStr[]="WWW-Authenticate:";
static char AuthorizationStr[]="Authorization:";
static char TransferEncoding[]="Transfer-Encoding:";
static char LocationStr[]="Location:";

extern char *strndup(const char *s, size_t n);

static void addCookieHdr( CookieHdr **p, char *c)
{
	CookieHdr	*newCookie = (CookieHdr*) malloc(sizeof (CookieHdr));
	char	*cp;

	if ( (cp = strchr(c,'='))){
		newCookie->next = *p;
		newCookie->name =  (char *)strndup(c,cp-c);
		newCookie->value = strdup(cp+1);
		*p = newCookie;
	} else
		free(newCookie);
}
/*----------------------------------------------------------------------*
 * hdrs->type needs to be initiated
 * Only read headers according to type
 * Reads all headers until an empty '\r\n" is found.
 */
void proto_ParseHdrs(tProtoCtx *pc, tHttpHdrs *hdrs)
{
    char buf[BUF_SIZE_MAX];
    char *cp;
    int n;

#ifdef DEBUGHDRS
    fprintf(stderr, "DEBUG: proto_ParseHdrs() pc=%p pc->type=%d\n", pc, pc->type);
#endif

    /* Parse the rest of the request headers. */
    while ((n = proto_Readline(pc, buf, BUF_SIZE_MAX)) > 0) {
        www_StripTail(buf);
#ifdef DEBUGHDRS
        fprintf(stderr, "  DEBUG: read \"%s\"\n", buf);
#endif
        if (strcmp(buf, "") == 0) {
            break;
        } else if (strncasecmp(buf, HostStr,sizeof(HostStr)-1) == 0) {
            cp = &buf[sizeof(HostStr)-1];
            cp += strspn(cp, " \t");
            free(hdrs->host);
            hdrs->host = strdup(cp);
        } else if (strncasecmp(buf, ContentLthStr,sizeof(ContentLthStr)-1) == 0) {
            cp = &buf[sizeof(ContentLthStr)-1];
            cp += strspn(cp, " \t");
            hdrs->content_length = atoi(cp);
        } else if (strncasecmp(buf, ContentTypeStr,sizeof(ContentTypeStr)-1) == 0) {
            cp = &buf[sizeof(ContentTypeStr)-1];
            cp += strspn(cp, " \t");
            free(hdrs->content_type);
            hdrs->content_type = strdup(cp);
        } else if (strncasecmp(buf, ConnectionStr,sizeof(ConnectionStr)-1) == 0) {
            cp = &buf[sizeof(ConnectionStr)-1];
            cp += strspn(cp, " \t");
            free(hdrs->Connection);
            hdrs->Connection = strdup(cp);
        } else if (strncasecmp(buf, WWWAuthenticateStr, sizeof(WWWAuthenticateStr)-1)==0) {
            cp =&buf[sizeof(WWWAuthenticateStr)-1];
            cp += strspn(cp, " \t");
            free(hdrs->wwwAuthenticate);
            hdrs->wwwAuthenticate = strdup(cp);
        } else if (strncasecmp(buf, AuthorizationStr, sizeof(AuthorizationStr)-1)==0) {
            cp =&buf[sizeof(AuthorizationStr)-1];
            cp += strspn(cp, " \t");
            free(hdrs->Authorization);
            hdrs->Authorization = strdup(cp);
        } else if (strncasecmp(buf, TransferEncoding, sizeof(TransferEncoding)-1)==0) {
            cp =&buf[sizeof(TransferEncoding)-1];
            cp += strspn(cp, " \t");
            free(hdrs->TransferEncoding);
            hdrs->TransferEncoding = strdup(cp);
        } else if (strncasecmp(buf, LocationStr, sizeof(LocationStr)-1)==0) {
            cp =&buf[sizeof(LocationStr)-1];
            cp += strspn(cp, " \t");
            free(hdrs->locationHdr);
            hdrs->locationHdr = strdup(cp);
        }  else if ( (strncasecmp(buf, SetCookieStr, sizeof(SetCookieStr)-1)==0)
			      || (strncasecmp(buf, SetCookieStr2, sizeof(SetCookieStr2)-1)==0) ) {
            char *c;
            cp =&buf[sizeof(SetCookieStr)-1];
            cp += strspn(cp, " \t:");   /* colon is added to skip : in SetCookieStr2 str*/
            /* don't need anything after ";" if it exists */
            if ( (c = strstr(cp,";")))
                *c = '\0';
            addCookieHdr( &hdrs->setCookies, cp );
        }


    }

#ifdef DEBUGHDRS
    fprintf(stderr, "DEBUG: proto_ParseHdrs done.\n");
#endif
}

/*----------------------------------------------------------------------*/
void proto_ParsePost(tProtoCtx *pc, tHttpHdrs *hdrs)
{
    char *data;
    size_t n;
    int len;

#ifdef DEBUG
    fprintf(stderr, "DEBUG: proto_ParsePost() to read %d bytes\n", hdrs->content_length);
#endif
    len = hdrs->content_length;
    data = (char *) malloc(len + 1); /* make room for terminating '\0' */
    n = proto_Readn(pc, data, len);
    if (n>0)
        data[n] = '\0';
    else
        data[0] = '\n';
#ifdef DEBUG
    fprintf(stderr, "DEBUG: proto_ParsePost() read %d bytes \"%s\"\n", n, data);
#endif
    free(hdrs->arg);
    hdrs->arg = data;

    proto_Skip(pc);
}

/*----------------------------------------------------------------------*
 * discard all there is to read on the in buffer
 * This is used since some stupid browsers (e.g. IE) sends more data
 * than specified in the content-lenght header
 * Returns result of last read():
 *  	0 - eof
 *     -1 - connection error.
 *      1 - no data, possibly more.
 */
int proto_Skip(tProtoCtx *pc)
{
    char c;
    int nread = 0;
    int ret = 0;
    long flags, nbflags;

#ifdef DEBUG
    fprintf(stderr, "DEBUG: proto_Skip() read all from fd and ignore\n");
#endif

    flags = (long) fcntl(pc->fd, F_GETFL);
    nbflags = flags | O_NONBLOCK;
    fcntl(pc->fd, F_SETFL, nbflags);

    do {
		switch (pc->type) {
		case iNormal:
			nread = read(pc->fd, &c, 1);
			break;
	#ifdef USE_SSL
		case iSsl:
			nread = SSL_read(pc->ssl, &c, 1);
			break;
	#endif
		default:
			slog(LOG_ERROR, "Impossible error; illegal ProtoCtx type (%d)", pc->type);
			break;
		}
		if (nread<0) {
			ret = errno == EAGAIN? 1: -1;
			break;
			}
	} while (nread>0);
    fcntl(pc->fd, F_SETFL, flags);

#ifdef DEBUG
    fprintf(stderr, "DEBUG: proto_Skip() done.ret=%d\n", ret);
#endif
	return ret;
}
