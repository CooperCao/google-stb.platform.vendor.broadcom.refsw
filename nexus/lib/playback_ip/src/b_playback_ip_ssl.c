/***************************************************************************
*     (c)2003-2015 Broadcom Corporation
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
* Description: SSL/TLS module
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#if defined(LINUX) || defined(__vxworks)

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_ssl.h"
#include "bfile_buffered.h"
#include "fnmatch.h"
#if 0
#define BDBG_MSG_FLOW(x)  printf x; printf("\n");
#else
#define BDBG_MSG_FLOW(x)
#endif

#define CLIENT_AUTH 1

BDBG_MODULE(b_playback_ip_ssl);
#ifdef B_HAS_NETACCEL
#if defined(LINUX)
#include <sys/syscall.h>
#include <asm/cachectl.h>
#endif


static int _cacheflush(char * addr, int size, int type)
{
    return syscall(SYS_cacheflush, addr, size, type);
}
#endif

void B_PlaybackIp_SslSessionSuspend( void *voidHandle);
void B_PlaybackIp_SslSessionClose( void *voidHandle);
void B_PlaybackIp_SslSessionShutdown( void *voidHandle, unsigned int flag );

typedef struct B_PlaybackIpSsl
{
    int sd;
    SSL_CTX *ctx;
    SSL *ssl;
    BIO *sbio;
    void *meth; /* TODO: change it to actual type */
    bool suspended;  /* Used for resumption. User suspended or server shutdown. */
    SSL_SESSION *session; /* session saved for resumption. */
    unsigned networkTimeout;
} B_PlaybackIpSsl;



#ifdef CLIENT_AUTH
static char *ssl_password;
/*The password code is not thread safe*/
static int ssl_password_cb(char *buf, int num, int rwflag,void *userdata)
{
    BSTD_UNUSED(rwflag);
    BSTD_UNUSED(userdata);

    if( (unsigned int)num < strlen(ssl_password)+1)
      return(0);

    strncpy(buf, ssl_password, num);
    return(strlen(ssl_password));
}
#endif

#ifdef BRCM_BIO_ERR
/* put this in ssl context ptr*/
BIO *bio_err=0;
/* Print SSL errors */
int berr_exit(const char *string)
{
    BIO_printf(bio_err,"%s\n",string);
    ERR_print_errors(bio_err);
}
#else
#define berr_exit(x) BDBG_ERR((x))
#endif

static int _ssl_connect( B_PlaybackIpSsl *sslHandle )
{
    int rc,width,err;
    fd_set readfds;
    struct timeval tv;

    tv.tv_sec = sslHandle->networkTimeout;
    tv.tv_usec = 0;
    while (1) {
        BDBG_MSG(("%s: before SSL_Connect: fd %d", __FUNCTION__, sslHandle->sd));
        rc=SSL_connect(sslHandle->ssl);
        if(rc<=0) {
            err =SSL_get_error(sslHandle->ssl,rc);
            if (err == SSL_ERROR_WANT_READ) {
                BDBG_MSG_FLOW(("SSL connect retry\n"));
                rc=SSL_get_fd(sslHandle->ssl);
                BDBG_MSG(("%s: SSL_Connect retry, before select, fd %d, fd from ssllib %d ARE %s", __FUNCTION__, sslHandle->sd, rc, (sslHandle->sd == rc?"SAME":"DIFFERENT")));
                rc=sslHandle->sd;
                width=rc+1;
                FD_ZERO(&readfds);
                FD_SET(rc,&readfds);
                if ((rc = select(width,&readfds,NULL,NULL,&tv)) < 0) {
                    BDBG_ERR(("%s: select failure: errno %d\n", __FUNCTION__, errno));
                    return -1;
                }
                else if (rc == 0 || !FD_ISSET(sslHandle->sd, &readfds)) {
                    BDBG_ERR(("%s: SSL_Connect timed out: select returned nothing for %d sec : fd %d", __FUNCTION__, tv.tv_sec, sslHandle->sd));
                    return -1;
                }
                continue;
            }
            BDBG_ERR(("SSL connect error %d, errno %d\n", err, errno));
            return -1;
        } else
            break;
    }

    BDBG_MSG(("%s: SSL connection successful",__FUNCTION__));
    return 0;
 }

static void
_ssl_shutdown( SSL *ssl, unsigned int retries )
{
    BSTD_UNUSED(retries);
    SSL_shutdown(ssl);
    ERR_clear_error();
    SSL_clear(ssl);
    return;
}

/*
 * This function tries to read the requested amount from the socket and returns any errors or bytes read.
 * Since data is encrypted with SSL Encryption, this function is
 * responsible for performing the SSL Decryption and proving the clear data
 * back to the caller.
 * It returns:
 *  =-1: for errors other than EINTR & EAGAIN during read call or when channel change occurs
 *  = 0: for EOF where server closed the TCP connection
 *  > 0: for success indicating number of bytes read from the socket
 *
 * Note1: this function is invoked by the HTTP module of the IP Applib in the
 * context of the Nexus Playback Thread.
 *
 * Note2: if this function needs to loop, it must periodically check the state
 * of Playback IP (B_PlaybackIpState) to see if it has changed to Closing or
 * CLosed state. This can happen when user changes channel and this function
 * can be busy reading data from the socket.
 */
static int
_http_ssl_socket_read(void *voidHandle, B_PlaybackIpHandle playback_ip, int sd, char *rbuf, int rbuf_len)
{
    B_PlaybackIpSsl *sslHandle = (B_PlaybackIpSsl *)voidHandle;
    int rc;
    int err;

    BSTD_UNUSED(sd);

    if (!sslHandle){
      BDBG_ERR(("SSL Handle not set"));
      return -1;
    }

    if (!rbuf || rbuf_len <=0) {
        BDBG_ERR(("%s: invalid parameters to read, rbuf %p, rbuf_len %d\n", __FUNCTION__, rbuf, rbuf_len));
        return -1;
    }

    /* Socket may have been closed */
    if (sslHandle->suspended ) {
        BDBG_ERR(("%s: SSL resumption (%d)", __FUNCTION__, (unsigned int)sslHandle->session ));
        sslHandle->ssl=SSL_new(sslHandle->ctx);
        if (!sslHandle->ssl) {
            BDBG_ERR(("%s: SSL resumption failed, SSL_new", __FUNCTION__ ));
            return -1;
        }
        sslHandle->sbio=BIO_new_socket(sd,BIO_NOCLOSE);
        SSL_set_bio(sslHandle->ssl,sslHandle->sbio,sslHandle->sbio);
        SSL_set_session(sslHandle->ssl,sslHandle->session);
        if (_ssl_connect(sslHandle)){
            BDBG_ERR(("%s: SSL resumption failed", __FUNCTION__ ));
            return -1;
        }
        sslHandle->sd = sd;
        sslHandle->suspended = false;
    };


    while (true) {
        if ( breakFromLoop(playback_ip)) {
            /* user changed the channel, so return */
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
            BDBG_WRN(("%s: breaking out of SSL read loop due to state (%d) change\n", __FUNCTION__, playbackIpState(playback_ip)));
#endif
            if (playback_ip->playback_state == B_PlaybackIpState_eWaitingToEnterTrickMode) {
                playback_ip->selectTimeout = true;
                errno = EINTR;
            }
            else {
                playback_ip->serverClosed = true;
            }
            return -1;
        }


        ERR_clear_error();
        if (sslHandle->ssl) {
            rc=SSL_read(sslHandle->ssl,rbuf,rbuf_len);
            err = SSL_get_error(sslHandle->ssl,rc);
            switch(err){
                case SSL_ERROR_NONE:
                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                    break;
                case SSL_ERROR_ZERO_RETURN:
                    BDBG_MSG(("SSL Error Zero return"));
                    goto shutdown;
                case SSL_ERROR_SYSCALL:
                    BDBG_MSG(("SSL Read Error: Premature close, rc = %d, errno %d", rc, errno));
                    return -1;
                default:
                    BDBG_MSG(("SSL Read Error: %d, rc %d\n",err, rc));
                    rc = -1;
            }
        } else {
                BDBG_MSG(("%s: Security session invalid", __FUNCTION__ ));
                return -1;
        }
        if (rc < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                BDBG_MSG(("%s: Read System Call interrupted or timed out, retrying (errno %d)\n", __FUNCTION__, errno));
                BKNI_Sleep(100);
                continue;
            }
            BDBG_MSG(("%s: read ERROR:%d", __FUNCTION__, errno));
            return -1;
        }
        else if (rc == 0) {
            BDBG_MSG(("%s: Reached EOF, server closed the connection!\n", __FUNCTION__));
            return 0;
        }
        BDBG_MSG(("%s: bytes read %d\n", __FUNCTION__, rc));
        return rc;
    }

shutdown:
    BDBG_MSG(("%s: SSL session closed.\n", __FUNCTION__));
    sslHandle->suspended = true; /* used for resumption */
    _ssl_shutdown(sslHandle->ssl,5); /* Limit retries for quick resumption */
    SSL_free(sslHandle->ssl);
    sslHandle->sbio = NULL; /* SSL_set_bio ties BIO to SSL, so SSL_free will free BIO */
    sslHandle->ssl=NULL;
    return 0;

    /* we shouldn't get here */
}

/*
 * This function writes the given data to the socket and returns any errors or the bytes wrote.
 * If the data needs to be Encrypted before being sent to the server, it
 * is responsibility of this function to do so.
 * It returns:
 *  =-1: for errors other than EINTR & EAGAIN during write call or when channel change occurs
 *  = 0: for EOF where server closed the TCP connection
 *  > 0: for success indicating number of bytes written to the socket
 *
 * Note: this function is invoked by the HTTP module of the IP Applib in the
 * context of the App thread (via the B_PlaybackIp_SocketOpen() ).
 *
 * Note2: if this function needs to loop, it must periodically check the state
 * of Playback IP (B_PlaybackIpState) to see if it has changed to Closing or
 * CLosed state. This can happen when user changes channel and this function
 * can be busy reading data from the socket.
 */
static int
_http_ssl_socket_write(void *voidHandle, volatile B_PlaybackIpState *playbackIpState, int sd, char *wbuf, int wbuf_len)
{
    B_PlaybackIpSsl *sslHandle = (B_PlaybackIpSsl *)voidHandle;
    int rc;

    BSTD_UNUSED(playbackIpState);
    BSTD_UNUSED(sd);

    if (!voidHandle)
        BDBG_ERR(("%s: invalid parameter, SSL handle %p ", __FUNCTION__, voidHandle));

    if (!wbuf || wbuf_len <=0) {
        BDBG_ERR(("%s: invalid parameters to write, wbuf %p, wbuf_len %d\n", __FUNCTION__, wbuf, wbuf_len));
        return -1;
    }


#ifdef B_HAS_NETACCEL
    _cacheflush(wbuf, strlen(wbuf), DCACHE);
    /* b_cacheflush(wbuf, strlen(wbuf)); */
#endif

    if (!sslHandle){
        BDBG_ERR(("%s: invalid parameter, SSL handle %p ", __FUNCTION__, sslHandle));
      return -1;
    }

    /* Socket may have been closed */
    if (sslHandle->suspended ) {
        BDBG_ERR(("%s: SSL resumption (%d)", __FUNCTION__, (unsigned int)sslHandle->session ));
        sslHandle->ssl=SSL_new(sslHandle->ctx);
        if (!sslHandle->ssl) {
            BDBG_ERR(("%s: SSL resumption failed, SSL_new", __FUNCTION__ ));
            return -1;
        }
        sslHandle->sbio=BIO_new_socket(sd,BIO_NOCLOSE);
        SSL_set_bio(sslHandle->ssl,sslHandle->sbio,sslHandle->sbio);
        SSL_set_session(sslHandle->ssl,sslHandle->session);
        if (_ssl_connect(sslHandle)){
            BDBG_ERR(("%s: SSL resumption failed", __FUNCTION__ ));
            return -1;
        }
        sslHandle->sd = sd;
        sslHandle->suspended = false;
    };

    if (sslHandle->ssl) {
        rc=SSL_write(sslHandle->ssl,wbuf, wbuf_len);
        switch(SSL_get_error(sslHandle->ssl,rc)){
            case SSL_ERROR_NONE:
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                if(wbuf_len!=rc)
                    BDBG_ERR(("Incomplete SSL write!"));
                break;
            default:
                 BDBG_ERR(("SSL write problem"));
                 return -1;
         }
    }
    else {
        BDBG_ERR(("Security context not initialized\n"));
        return -EBUSY;
    }

    return 0;
}

/* Check that the common name matches the host name*/
int check_cert(SSL *ssl, char *host)
{

    X509 *server_cert;
    char peer_CN[256];
    char *str;

    long ret=SSL_get_verify_result(ssl);


    switch(ret){
        case X509_V_OK:
                BDBG_MSG(("Certificate  valid."));
                break;
        case X509_V_ERR_CERT_NOT_YET_VALID:
                BDBG_ERR(("Certificate not yet valid. Verify client date and time. x509: %d", ret));
                return -1;
                break;
        case X509_V_ERR_CERT_HAS_EXPIRED:
                BDBG_ERR(("Certificate expired. Verify certificate and client date. x509: %d", ret));
                return -1;
                break;
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
                BDBG_ERR(("Unable to get local issuer certificate.  x509: %d", ret));
                return -1;
                break;
        default:
                BDBG_MSG(("Certificate verification failed: X509_V_ERR: %d", ret));
                return -1;
    }


    /*Check the cert chain. The chain length
      is automatically checked by OpenSSL when
      we set the verify depth in the ctx */

    /*Check the common name*/
    server_cert=SSL_get_peer_certificate(ssl);
    if (!server_cert) {
        BDBG_ERR(("Can't obtain server cert"));
        goto err_cert;
    }

    str = X509_NAME_oneline(X509_get_subject_name(server_cert),0,0);
    if (str==NULL){
        BDBG_ERR(("CERT subject NULL string"));
        goto err_cert;
    }
    BDBG_MSG_FLOW(("\t CERT subject: %s\n", str));
    OPENSSL_free (str);

    str = X509_NAME_oneline(X509_get_issuer_name(server_cert),0,0);
    if (str==NULL){
        BDBG_ERR(("CERT issuer NULL string"));
        goto err_cert;
    }
    BDBG_MSG_FLOW(("\t CERT issuer: %s\n", str));
    OPENSSL_free (str);

    X509_NAME_get_text_by_NID(X509_get_subject_name(server_cert),
                      NID_commonName,
                              peer_CN,
                              256);

    BDBG_MSG_FLOW(("\t CERT host: %s\n",peer_CN));

    if (fnmatch(peer_CN,host, FNM_CASEFOLD|FNM_NOESCAPE)) {
        BDBG_ERR(("SSL: Common name ( %s ) doesn't match host name ( %s )", peer_CN, host));
        goto err_cert;
    }

    X509_free(server_cert);
    return 0;

err_cert:
    if (server_cert)
        X509_free(server_cert);
    return -1;
}


/*
 * Initialize session specific context of the security module
 * Called during B_PlaybackIp_SocketOpen().
 */
int B_PlaybackIp_SslSessionOpen(
    B_PlaybackIpSessionOpenSettings *openSettings,         /* input: provides server ip, port info */
    int sd,                                                     /* input: socket descriptor */
    B_PlaybackIpSecurityOpenOutputParams *securityOpenParams) /* output: security settings return params */
{
    B_PlaybackIpSsl *sslHandle=NULL;

    securityOpenParams->byteRangeOffset = 0;

    if (openSettings == NULL ) {
        BDBG_ERR(("%s: Invalid parameters, Open Settings pts: socket %p \n", __FUNCTION__, openSettings));
        goto error;
    }

    if (sd <= 0) {
      BDBG_ERR(("%s: invalid socket, sd = %d", __FUNCTION__, sd));
      goto error;
    }

    if (openSettings->security.securityProtocol != B_PlaybackIpSecurityProtocol_Ssl) {
      BDBG_ERR(("%s: invoking SSL module with incorrect security protocol %d", __FUNCTION__, openSettings->security.securityProtocol));
      goto error;
    }


    BDBG_MSG(("%s: setting up the netIo interface for socket read & write\n", __FUNCTION__));
    securityOpenParams->netIoPtr->read = _http_ssl_socket_read;
    securityOpenParams->netIoPtr->write = _http_ssl_socket_write;
    securityOpenParams->netIoPtr->suspend = B_PlaybackIp_SslSessionSuspend;
    securityOpenParams->netIoPtr->close = B_PlaybackIp_SslSessionClose;
    securityOpenParams->netIoPtr->shutdown = B_PlaybackIp_SslSessionShutdown;

    sslHandle = (B_PlaybackIpSsl *)BKNI_Malloc(sizeof(B_PlaybackIpSsl));
    if (!sslHandle){
        BDBG_ERR(("%s: Failed allocation", __FUNCTION__));
        goto error;
    }

    /* Create our session/object */
    sslHandle->sd = sd;
    sslHandle->ctx = openSettings->security.initialSecurityContext;
    sslHandle->networkTimeout = openSettings->networkTimeout;


    /* Connect the SSL socket */
    sslHandle->suspended = false;
    sslHandle->ssl=SSL_new(sslHandle->ctx);
    if (!sslHandle->ssl) {
        BDBG_ERR(("%s: SSL_new failed", __FUNCTION__ ));
        goto error;
    }
    sslHandle->sbio=BIO_new_socket(sd,BIO_NOCLOSE);
    SSL_set_bio(sslHandle->ssl,sslHandle->sbio,sslHandle->sbio);

    BDBG_MSG(("%s: fd %d, SSL %x, networkTimeout %d SBIO %x \n", __FUNCTION__, sd, (unsigned int)sslHandle->ssl, sslHandle->networkTimeout, (unsigned int)sslHandle->sbio));

    if (_ssl_connect(sslHandle))
        goto error;

    BDBG_MSG(("%s: fd %d SSL connection successful",__FUNCTION__, sd));

    /* server certificate verification */
    if (check_cert(sslHandle->ssl, openSettings->socketOpenSettings.ipAddr)) {
        BDBG_MSG(("%s: Server Certificate verification failed, but continuing with the playback...", __FUNCTION__));
    }

#if 1
    /* SWDTV-7892: previous code was using the SSL_get_session() which would *not* increment the reference count on ssl session */
    /* thus the ssl session would get freed during the shutdown call and we were using a freed up memory causing the seg-faults */
    /* fix is to use the SSL_get1_session() which actually increments the ssl session reference count and thus avoids the free during shutdown */
    sslHandle->session=SSL_get1_session(sslHandle->ssl);
#else
    sslHandle->session=SSL_get_session(sslHandle->ssl);
#endif

    *securityOpenParams->securityHandle = (void *)sslHandle;

    return 0;

error:
    if (sslHandle) {
        /* if ssl free ssl */
        if (sslHandle->ssl){
            _ssl_shutdown(sslHandle->ssl, 0);
            SSL_free(sslHandle->ssl);
            sslHandle->sbio = NULL; /* SSL_set_bio ties BIO to SSL, so SSL_free will free BIO */
            sslHandle->ssl=NULL;
        }
        BKNI_Free(sslHandle);
        sslHandle=NULL;
    }

    return -1;
}

void B_PlaybackIp_SslSessionShutdown(
    void *voidHandle,                                             /* input: security module specific handle */
    unsigned int flag )
{
    B_PlaybackIpSsl *sslHandle = (B_PlaybackIpSsl *)voidHandle;

    BSTD_UNUSED(flag);

    BDBG_MSG(("%s: Shutdown SSL handle %p ", __FUNCTION__, sslHandle));

    if (!sslHandle) {
        BDBG_ERR(("%s: invalid parameter, SSL handle %p ", __FUNCTION__, sslHandle));
        return;
    }


    if (sslHandle->ssl) {
        _ssl_shutdown(sslHandle->ssl, 5); /* Limit retries to avoid delays */
        SSL_free(sslHandle->ssl);
        sslHandle->sbio = NULL; /* SSL_set_bio ties BIO to SSL, so SSL_free will free BIO */
        sslHandle->ssl = NULL;
    }


}

/*
 * Un-Initialize session specific context of the security module
 * Called during B_PlaybackIp_SocketClose().
 */
void B_PlaybackIp_SslSessionClose(
    void *voidHandle)                                            /* input: security module specific handle */
{
    B_PlaybackIpSsl *sslHandle = (B_PlaybackIpSsl *)voidHandle;

    BDBG_MSG(("%s: Closing SSL handle %p ", __FUNCTION__, sslHandle));

    if (!sslHandle) {
        BDBG_ERR(("%s: invalid parameter, SSL handle %p ", __FUNCTION__, sslHandle));
        return;
    }


    /* Free up sslHandle & any other saved SSL context */
    if (sslHandle->ssl){
        _ssl_shutdown(sslHandle->ssl, 1);
        SSL_SESSION_free(sslHandle->session);
        SSL_free(sslHandle->ssl);
        sslHandle->sbio = NULL; /* SSL_set_bio ties BIO to SSL, so SSL_free will free BIO */
        sslHandle->ssl = NULL;
    }
    BKNI_Free(sslHandle);
    sslHandle=NULL;
}

/*
 * Clone session.
 * A duplicate session is opened using a separate file descriptor
 * Provided for separate video and audio seeks.
 * The source security will be suspended.
 */
int B_PlaybackIp_SslCloneSessionOpen(
    int sd,                                                     /* input: socket descriptor */
    void *sourceSecurityHandle,                                 /* intput: original security handle */
    void **targetSecurityHandle)                                /* output: new security handlei */
{

    B_PlaybackIpSsl *sslHandle=NULL;
    B_PlaybackIpSsl *srcHandle = sourceSecurityHandle;
    if (!srcHandle){
        BDBG_ERR(("%s: invalid parameter, security handle %p ", __FUNCTION__, srcHandle));
        return -1;
    }

    if (sd <= 0) {
      BDBG_ERR(("%s: invalid socket, sd = %d", __FUNCTION__, sd));
      goto error;
    }

    sslHandle = (B_PlaybackIpSsl *)BKNI_Malloc(sizeof(B_PlaybackIpSsl));
    if (!sslHandle){
        BDBG_ERR(("%s: Failed allocation", __FUNCTION__));
        goto error;
    }

    /* Create our session/object */
    sslHandle->sd = sd;
    sslHandle->ctx = srcHandle->ctx;


    /* Connect the SSL socket */
    sslHandle->suspended = false;
    sslHandle->ssl=SSL_new(srcHandle->ctx);
    if (!sslHandle->ssl) {
        BDBG_ERR(("%s: SSL_new failed", __FUNCTION__ ));
        goto error;
    }
    sslHandle->sbio=BIO_new_socket(sd,BIO_NOCLOSE);
    SSL_set_bio(sslHandle->ssl,sslHandle->sbio,sslHandle->sbio);

    BDBG_MSG(("%s: fd %d, SSL %x, SBIO %x \n", __FUNCTION__, sd, (unsigned int)sslHandle->ssl, (unsigned int)sslHandle->sbio));

    SSL_set_session(sslHandle->ssl, srcHandle->session);
    sslHandle->session = srcHandle->session;

    if (_ssl_connect(sslHandle))
        goto error;

    *targetSecurityHandle = (void *)sslHandle;


    return 0;

error:
    if (sslHandle) {
        /* if ssl free ssl */
        /* if sbio free sbio */
        if (sslHandle->ssl){
            _ssl_shutdown(sslHandle->ssl, 0);
            SSL_free(sslHandle->ssl);
            sslHandle->sbio = NULL; /* SSL_set_bio ties BIO to SSL, so SSL_free will free BIO */
            sslHandle->ssl=NULL;
        }
        free(sslHandle);
        sslHandle=NULL;
    }


    return -1;

}

/*
 * Suspend session.
 * SSL session is shutdown, but session id is retained.
 * Session will be resumed in subseqent read and write requests.
 */
void B_PlaybackIp_SslSessionSuspend( void *voidHandle)
{
    B_PlaybackIpSsl *sslHandle = (B_PlaybackIpSsl *)voidHandle;

    if (!sslHandle) {
        BDBG_ERR(("%s: invalid parameter, SSL handle %p ", __FUNCTION__, sslHandle));
        return;
    }

    if (sslHandle->suspended){
        BDBG_MSG_FLOW(("%s: SSL session (%d) already suspended", __FUNCTION__, (unsigned int)sslHandle->session));
        return;
    }

    BDBG_MSG_FLOW(("%s: SSL session (%d) suspended", __FUNCTION__, (unsigned int)sslHandle->session));
    sslHandle->suspended = true; /* used for resumption */
    _ssl_shutdown(sslHandle->ssl, 0);
    SSL_free(sslHandle->ssl);
    sslHandle->sbio = NULL; /* SSL_set_bio ties BIO to SSL, so SSL_free will free BIO */
    sslHandle->ssl=NULL;
    return;

}

static int gSslInitRefCnt = 0;
/*
 * Initialize global context of the security module
 * Called during B_PlaybackIp_Open().
 */
void* B_PlaybackIp_SslInit( B_PlaybackIpSslInitSettings *initSettings)
{
    const SSL_METHOD *meth;
    SSL_CTX *ctx=NULL;

    if (!initSettings) {
        BDBG_ERR(("%s: invalid parameter, SSL initSettings %p ", __FUNCTION__, initSettings));
        return NULL;
    }

    if (!gSslInitRefCnt) {
        /* Global system initialization*/
        if (!initSettings->sslLibInitDone) {
            SSL_library_init();
            SSL_load_error_strings();
            ERR_load_crypto_strings();
        }
        else {
            BDBG_WRN(("%s: App has set the sslLibInitDone, so NOT initializing OpenSSL lib!", __FUNCTION__));
        }

        meth = TLSv1_method();
        if (!meth) {
            BDBG_ERR(("%s: SSLv23_method failure %p", __FUNCTION__, meth));
            BDBG_ASSERT(meth);
            return NULL;
        }
        ctx = SSL_CTX_new(meth);
        if (!ctx) {
            BDBG_ERR(("%s: SSL_CTX_new failure %p", __FUNCTION__, ctx ));
            BDBG_ASSERT(ctx);
            return NULL;
        }

#ifdef CLIENT_AUTH
        if (initSettings->clientAuth) {
            /* Load our keys and certificates*/
            if (initSettings->ourCertPath ) {
                if (SSL_CTX_use_certificate_file(ctx, initSettings->ourCertPath, SSL_FILETYPE_PEM) <= 0)  {
                    BDBG_ERR(("%s: Certificate file failure %s", __FUNCTION__, initSettings->ourCertPath));
                    return NULL;
                }
            } else {
                    BDBG_ERR(("%s: Client Authentication enabled, but certificate is not defined", __FUNCTION__ ));
                    return NULL;
            }

            if (initSettings->password) {
                ssl_password = initSettings->password;
                SSL_CTX_set_default_passwd_cb(ctx, ssl_password_cb);
            }

            if(initSettings->privKeyPath ) {
                if(!(SSL_CTX_use_PrivateKey_file(ctx, initSettings->privKeyPath, SSL_FILETYPE_PEM))) {
                    BDBG_ERR(("%s: Key file failure %s", __FUNCTION__, initSettings->privKeyPath));
                    return NULL;
                }
            } else {
                    BDBG_ERR(("%s: Client Authentication enabled, but private key is not defined", __FUNCTION__ ));
                    return NULL;

            }
        }
#endif

        /* Load the CAs we trust */
        if(initSettings->rootCaCertPath ) {
            if(!(SSL_CTX_load_verify_locations(ctx, initSettings->rootCaCertPath, 0))) {
                BDBG_ERR(("%s: Can't read CA list %s", __FUNCTION__, initSettings->rootCaCertPath));
                return NULL;
            }
        } else {
            BDBG_ERR(("%s: Root CA certificate is not defined.",__FUNCTION__ ));
            return NULL;
        }

#ifdef CLIENT_AUTH
        if (initSettings->clientAuth) {
            if (!SSL_CTX_check_private_key(ctx)) {
                BDBG_ERR(("%s: Private key does not match the certificate public key", __FUNCTION__ ));
                return NULL;
            }
        }
#endif

#if (OPENSSL_VERSION_NUMBER < 0x00905100L)
        SSL_CTX_set_verify_depth(ctx,1);
#endif

    BDBG_MSG_FLOW(("%s: Context %x\n", __FUNCTION__, (unsigned int)ctx));



      /* An error write context */
#ifdef BRCM_BIO_ERR
      bio_err=BIO_new_fp(stderr,BIO_NOCLOSE);
#endif
    }
    /* check for max gSslInitRefCnt */
    gSslInitRefCnt++;

    return ctx;
}

/*
 * Un-Initialize global context of the security module
 * Called during B_PlaybackIp_Close().
 */
void B_PlaybackIp_SslUnInit(void *ctx)
{

    gSslInitRefCnt--;
    if (!gSslInitRefCnt) {
      /* Global system un-initialization*/
       SSL_CTX_free(ctx);

      /* unload library */

      /* An error write context */
#ifdef BRCM_BIO_ERR
      bio_err=0;
#endif
    }
    if (gSslInitRefCnt < 0) {
        gSslInitRefCnt = 0;
        BDBG_ERR(("%s: SSL reference count < 0", __FUNCTION__ ));
    }
}

#endif /* LINUX || VxWorks */
