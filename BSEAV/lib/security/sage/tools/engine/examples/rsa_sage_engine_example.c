/***************************************************************************
 *     (c)2014 Broadcom Corporation
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
 **************************************************************************/

#define ENGINE_DEFAULT_CIPHER_LIST "-SSLV2:-SSLv3:AES128-SHA"
#define ENGINE_DEFAULT_SSL_BINFILE "./drm_ssl.bin"
#define ENGINE_DEFAULT_SO_PATH     "./librsa_sage_engine.so"
#define ENGINE_DEFAULT_PORT        4437

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <execinfo.h>

#include <memory.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#include "bstd.h"
#include "bkni.h"

#include "bsagelib_types.h"
#include "rsa_sage_engine.h"

#include "nexus_platform.h"
#include "nexus_platform_init.h"

#if USE_NXCLIENT
#include "nxclient.h"
#endif

#define BUFFER_LENGTH (2*1024*1024)

BDBG_MODULE(rsa_sage_engine_example);

static const char *bin_name = NULL;
static struct engine_ex_ctx {
    uint8_t server;
    uint8_t tls1_2;
    uint8_t debug;
    uint8_t key_id;
    int32_t port;
    const char *hostname;
    const char *cert_path;
    const char *binfile_path;
    const char *CAcert_path;
    const char *engine_path;
    const char *cipher;
} _ctx = {
    /* .server       = */ 0,
    /* .tlsv1_2      = */ 1,
    /* .debug        = */ 0,
    /* .key_id       = */ 0,
    /* .port         = */ ENGINE_DEFAULT_PORT,
    /* .hostname     = */ NULL,
    /* .cert_path    = */ NULL,
    /* .binfile_path = */ ENGINE_DEFAULT_SSL_BINFILE,
    /* .CAcert_path  = */ NULL,
    /* .engine_path  = */ ENGINE_DEFAULT_SO_PATH,
    /* .cipher       = */ ENGINE_DEFAULT_CIPHER_LIST
};

/* static function definitions */
static int SAGE_app_join_nexus(void);
static void SAGE_app_leave_nexus(void);
static void print_openssl_error_stack(void);
static int generic_load_engine_fn(
    const char *engine_id,
    const char **pre_cmds, int pre_num,
    const char **post_cmds, int post_num);

static int OpensslEngine_Test(ENGINE *engine);

static int
SAGE_app_join_nexus(void)
{
    NEXUS_Error rc_nexus;
    int rc;
#if USE_NXCLIENT
    NxClient_JoinSettings joinSettings;
    const char *mode = "NxClient_Join";
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "openssl_rsa_sage");
    rc_nexus = NxClient_Join(&joinSettings);
#else
    NEXUS_PlatformSettings platformSettings;
    const char *mode = "Platform_Init";

    BDBG_LOG(("\t up all Nexus modules for platform using default settings\n\n"));

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

    rc_nexus = NEXUS_Platform_Init(&platformSettings);
#endif
    if (rc_nexus == NEXUS_SUCCESS) {
        BDBG_LOG(("\t Nexus has initialized successfully (mode=%s)\n", mode));
        rc = 0;
    }
    else {
        BDBG_ERR(("\t Failed to Join Nexus (mode=%s, error=%u)\n", mode, rc_nexus));
        rc = 1;
    }
    return rc;
}

static void
SAGE_app_leave_nexus(void)
{
#if USE_NXCLIENT
    NxClient_Uninit();
#else
    NEXUS_Platform_Uninit();
#endif
}


static int
OpensslEngine_Test(ENGINE *engine)
{
    int             rc = 0;
    uint8_t         srcData[64] = {0};
    uint8_t         signature[384] = {0};
    uint32_t        retsig_len = 0;
    SSL_CTX *       ssl_ctx = NULL;
    EVP_PKEY *      pkey = NULL;
    const SSL_METHOD * meth = NULL;
    int             sock = -1;
    struct sockaddr_in  sa_srv;
    int             err;
    X509*           client_cert;
    SSL *           ssl_client=NULL;
    socklen_t       client_len;
    int             listen_sd = -1;
    struct hostent *host;
    char key_id_str[16] = {0};

#define SLOT_FORMAT_STR "slot_0-id_%d"

    retsig_len = sizeof(signature);
    BKNI_Memset(srcData, 0xFF, sizeof(srcData));

    /* Load keys from entry _ctx.key_id in BinFile */
    BKNI_Snprintf(key_id_str, sizeof(SLOT_FORMAT_STR), SLOT_FORMAT_STR, (int)_ctx.key_id);
    pkey = ENGINE_load_private_key(engine, key_id_str, NULL, NULL);
    if (_ctx.debug > 1) BDBG_MSG(("ENGINE_load_private_key(%p, %s, NULL, NULL) returned %p", engine, key_id_str, pkey));
    if (pkey==NULL)
    {
        BDBG_ERR(("Error calling ENGINE_load_private_key()"));
        rc = -1;
        goto handle_error;
    }

    BDBG_LOG(("EVP_PKEY = '%p'", pkey));

    if (_ctx.server)
    {
        if (_ctx.tls1_2)
        {
            meth = TLSv1_2_server_method();
            if (_ctx.debug > 1) BDBG_MSG(("TLSv1_2_server_method() returned %p", meth));
        }
        else
        {
            meth = TLSv1_server_method();
            if (_ctx.debug > 1) BDBG_MSG(("TLSv1_server_method() returned %p", meth));
        }
    }
    else
    {
        if (_ctx.tls1_2)
        {
            meth = TLSv1_2_method();
            if (_ctx.debug > 1) BDBG_MSG(("TLSv1_2_method() returned %p", meth));
        }
        else
        {
            meth = TLSv1_method();
            if (_ctx.debug > 1) BDBG_MSG(("TLSv1_method() returned %p", meth));
        }
    } /* end of client/server branch */
    if (meth == NULL)
    {
        BDBG_ERR(("TLSv1..._method() call failed"));
        goto handle_error;
    }

    ssl_ctx = SSL_CTX_new(meth);
    if (_ctx.debug > 1) BDBG_MSG(("SSL_CTX_new(%p) returned %p", meth, ssl_ctx));
    if (ssl_ctx == NULL)
    {
        BDBG_ERR(("SSL_new failed"));
        goto handle_error;
    }

    if (_ctx.CAcert_path)
    {
        if (_ctx.debug > 1) BDBG_MSG(("SSL_CTX_load_verify_locations(%p, %s, NULL)", ssl_ctx, _ctx.CAcert_path));
        if(SSL_CTX_load_verify_locations(ssl_ctx, _ctx.CAcert_path, NULL)<=0)
        {
            BDBG_ERR(("Error calling SSL_CTX_load_verify_locations()"));
            print_openssl_error_stack();
            rc = -1;
            goto handle_error;
        }

        if (_ctx.debug > 1) BDBG_MSG(("SSL_CTX_set_verify(%p, %u, NULL)", ssl_ctx, SSL_VERIFY_PEER));
        SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, NULL);
        if (_ctx.debug > 1) BDBG_MSG(("SSL_CTX_set_verify_depth(%p, 5)", ssl_ctx));
        SSL_CTX_set_verify_depth(ssl_ctx, 5);
    }
    else
    {
        if (_ctx.debug > 1) BDBG_MSG(("SSL_CTX_set_verify(%p, %u, NULL)", ssl_ctx, SSL_VERIFY_NONE));
        SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);
    }

    if(_ctx.cert_path)
    {
        BDBG_MSG(("Checking match of keys in certificate '%s' and binfile '%s', key_id '%s'",
                  _ctx.cert_path, _ctx.binfile_path, key_id_str));
        if (_ctx.debug > 1) BDBG_MSG(("SSL_CTX_use_PrivateKey(%p, %p)", ssl_ctx, pkey));
        if(SSL_CTX_use_PrivateKey(ssl_ctx, pkey)==0)
        {
            BDBG_ERR(("Error calling SSL_CTX_use_PrivateKey()"));
            print_openssl_error_stack();
            rc = -1;
            goto handle_error;
        }
        if (_ctx.debug > 1) BDBG_MSG(("SSL_CTX_use_certificate_file(%p, %s, %u)", ssl_ctx, _ctx.cert_path, SSL_FILETYPE_PEM));
        if (SSL_CTX_use_certificate_file(ssl_ctx, _ctx.cert_path, SSL_FILETYPE_PEM) <=0)
        {
            BDBG_ERR(("Error calling SSL_CTX_use_certificate_file()"));
            print_openssl_error_stack();
            rc = -1;
            goto handle_error;
        }
        if (_ctx.debug > 1) BDBG_MSG(("SSL_CTX_check_private_key(%p)", ssl_ctx));
        if (!SSL_CTX_check_private_key(ssl_ctx))
        {
            BDBG_ERR(("Error calling SSL_CTX_check_private_key()"));
            print_openssl_error_stack();
            rc = -1;
            goto handle_error;
        }
        if (_ctx.debug > 1) BDBG_MSG(("SSL_CTX_set_cipher_list(%p, %s)", ssl_ctx, _ctx.cipher));
        if (SSL_CTX_set_cipher_list(ssl_ctx, _ctx.cipher)==0)
        {
            BDBG_ERR(("Error configuring cipher list: %s", _ctx.cipher));
            exit(1);
        }
    }
    else
    {
        BDBG_MSG(("Bypassing EVP_PKEY certificate check......"));
    }

    if (_ctx.server)
    {
        listen_sd = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_sd == -1)
        {
            BDBG_ERR(("socket: %s", strerror(errno)));
            return(0);
        }
        {
            int opt = 1;
            if (setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
            {
                BDBG_ERR(("setsockopt server: %s", strerror(errno)));
                return(0);
            }
        }
        memset(&sa_srv, 0, sizeof(sa_srv));
        sa_srv.sin_family = AF_INET;
        sa_srv.sin_addr.s_addr = INADDR_ANY;
        sa_srv.sin_port = htons(_ctx.port);      /* Server HTTPS port */

        while(1)
        {
            err = bind(listen_sd, (struct sockaddr *)&sa_srv, sizeof(sa_srv));
            if (err < 0)
            {
                BDBG_ERR(("bind: %s", strerror(errno)));
                BDBG_WRN(("bind error sleep 1s"));
                sleep(1);
            }
            else
            {
                break;
            }
        }

        /* Receive a TCP connection. */

        err = listen(listen_sd, 5);
        if (err < 0)
        {
            BDBG_ERR(("listen: %s", strerror(errno)));
            return(0);
        }

        BDBG_MSG(("init ok, enter main loop"));
    } /* end of client/server branch */

    do
    {
        if (_ctx.server)
        {
            client_len = sizeof(sa_srv);
            sock = accept(listen_sd, (struct sockaddr*) &sa_srv, &client_len);
            if (sock < 0)
            {
                BDBG_ERR(("accept: %s", strerror(errno)));
                /* continue; */
                break;
            }

            {
                int opt=1;
                if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0)
                {
                    BDBG_ERR(("setsockopt sock: %s",strerror(errno)));
                    continue;
                }
            }
        }
        else
        {
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == -1)
            {
                BDBG_ERR(("socket: %s", strerror(errno)));
                return(0);
            }
            if ( (host = gethostbyname(_ctx.hostname)) == NULL )
            {
                BDBG_ERR(("==>Bad hostname '%s'", _ctx.hostname));
                abort();
            }
            BKNI_Memset(&sa_srv, 0, sizeof(sa_srv));
            sa_srv.sin_family = AF_INET;
            sa_srv.sin_addr.s_addr = *(long*)(host->h_addr);
            sa_srv.sin_port = htons(_ctx.port);      /* Server HTTPS port */

            err = connect(sock, (struct sockaddr *)&sa_srv, sizeof(sa_srv));
            if (err < 0)
            {
                BDBG_ERR(("connect: %s", strerror(errno)));
                BDBG_WRN(("connect error sleep 1s"));
                sleep(1);
                continue;
            }
        } /* end of client/server branch */
        /* Receive a TCP connection. */

        /* ----------------------------------------------- */
        /* TCP connection is ready. Do server side SSL. */

        ssl_client=SSL_new(ssl_ctx);
        if (_ctx.debug > 1) BDBG_MSG(("SSL_new(%p) returned %p", ssl_ctx, ssl_client));
        if (ssl_client==NULL)
        {
            ERR_print_errors_fp(stderr);
            BDBG_ERR(("error SSL_new"));
            break;
        }
        if (_ctx.debug > 1) BDBG_MSG(("SSL_set_fd(%p, %d)", ssl_client, sock));
        err=SSL_set_fd(ssl_client, sock);
        if (err<=0)
        {
            ERR_print_errors_fp(stderr);
            BDBG_ERR(("error SSL_set_fd"));
            break;
        }
        if (_ctx.server)
        {
            if (_ctx.debug > 1) BDBG_MSG(("SSL_accept(%p)", ssl_client));
            err=SSL_accept(ssl_client);
        }
        else
        {
            if (_ctx.debug > 1) BDBG_MSG(("SSL_connect(%p)", ssl_client));
            err=SSL_connect(ssl_client);
        } /* end of client/server branch */
        if (err<0)
        {
            ERR_print_errors_fp(stderr);
            BDBG_ERR(("SSL error while establishing connection with peer"));
        }
        else
        {
            BDBG_MSG(("SSL connection using %s", SSL_get_cipher(ssl_client)));
            client_cert=SSL_get_peer_certificate(ssl_client);
            if (_ctx.debug > 1) BDBG_MSG(("SSL_get_peer_certificate(%p) returned %p", ssl_client, client_cert));
            if (client_cert!=NULL)
            {
                if (_ctx.debug > 1) BDBG_MSG(("X509_free(%p)", client_cert));
                X509_free(client_cert);
                if (_ctx.debug > 1) BDBG_MSG(("SSL_get_verify_result(%p)", ssl_client));
                if (SSL_get_verify_result(ssl_client) != X509_V_OK)
                {
                    BDBG_WRN(("Error verifying server X509 certificate"));
                }
                else
                {
                    BDBG_MSG(("Verifying server X509 certificate OK"));
                }
            }
            else
            {
                BDBG_MSG(("Peer does not have certificate"));
            }
            /* communicate */
            {
                char *msg = "# Hello???";
                char buf[1024];
                int bytes;
                BDBG_MSG(("entering SSL_write"));
                err=SSL_write(ssl_client, msg, strlen(msg)); /* encrypt & send message */
                BDBG_MSG(("leaving SSL_write"));
                if (err<=0)
                {
                    ERR_print_errors_fp(stderr);
                    BDBG_WRN(("error SSL_write"));
                    /* keep going */
                }
                BDBG_MSG(("entering SSL_read"));
                bytes = SSL_read(ssl_client, buf, sizeof(buf)); /* get reply & decrypt */
                BDBG_MSG(("leaving SSL_read"));
                if (bytes>=0)
                {
                    buf[bytes] = 0;
                    BDBG_MSG(("Received: \"%.*s\"", sizeof(buf), buf));
                }
            }
        }
        if (ssl_client!=NULL)
        {
            SSL_free(ssl_client);
            ssl_client=NULL;
        }
        if (sock != -1)
        {
            close(sock);
            sock = -1;
        }
        while (waitpid(-1, NULL, WNOHANG) > 0); /* clean up child processes */
    } while (1);

/*--------------------------------------------------------*/

handle_error:
    if (listen_sd != -1)
    {
        close(listen_sd);
        listen_sd = -1;
    }
    if (ssl_ctx) {
        SSL_CTX_free(ssl_ctx);
        ssl_ctx = NULL;
    }
    if (pkey) {
        EVP_PKEY_free(pkey);
        pkey = NULL;
    }

    return rc;
}

static void
print_openssl_error_stack(void)
{
    unsigned long error;

    BDBG_LOG(("MESSAGE FROM INSIDE OPENSSL_ERROR_STACK"));
    while((error = ERR_get_error()))
    {
        /*BDBG_ERR(("# %s ", ERR_error_string(error, NULL) ));*/
        BDBG_ERR(("lib*:%s   func:%s   reason:%s",
                  ERR_lib_error_string(error),
                  ERR_func_error_string(error),
                  ERR_reason_error_string(error) ));
    }
    return;
}

static void
_usage(void)
{
    BDBG_LOG(("Usage, either client or server as follow (client or server is mandatory):"));
    BDBG_LOG(("./%s client|server [-hostname host] [-port port] [-tls1] [-tls1_2] [-cert cert_path] [-binfile binfile_path] [-slot slot_index] [-CAcert ca_cert_path] [-engine engine_path] [-cipher cipher_list] [-debug]",
              bin_name));
    BDBG_LOG(("-hostname host       \tSpecifies the 'host' to connect to as a client, ignored as a server"));
    BDBG_LOG(("-port port           \tSpecifies the 'port' to connect to as a client, or to listen on as a server. Default to %d", ENGINE_DEFAULT_PORT));
    BDBG_LOG(("-tls1                \tUse TLS version 1."));
    BDBG_LOG(("-tls1_2              \tuse TLS version 1.2. This is the default."));
    BDBG_LOG(("-cert cert_path      \tUse 'cert_path' as the certificate that holds same public keys as in the binfile."));
    BDBG_LOG(("-binfile binfile_path\tUse 'binfile_path' as the binfile that contains my public and private keys for the SSL connection. Default is %s", ENGINE_DEFAULT_SSL_BINFILE));
    /* BDBG_LOG(("-key_slot key_slot_index\tUse 'key_slot_index' as the SSL certificate slot index to use from the DRM BinFile. Unused.")); */
    BDBG_LOG(("-key_id key_id       \tUse 'key_id' as the SSL certificate key index to use from the DRM BinFile. Default is 0, valid values [0, 7]"));
    BDBG_LOG(("-CAcert ca_cert_path \tUse 'ca_cert_path' as the certificate to verify remote identify"));
    BDBG_LOG(("-engine engine_path  \tUse 'engine_path' SAGE Rsa Engine. Default is %s", ENGINE_DEFAULT_SO_PATH));
    BDBG_LOG(("-cipher cipher_list  \tUse 'cipher list' ciphers during the SSL connection with the remote peer. Default is %s", ENGINE_DEFAULT_CIPHER_LIST));
    BDBG_LOG(("-debug               \tOutput more debug information"));
}

static int
_verify_dump_arguments(void)
{
    BDBG_MSG(("Arguments:"));
    BDBG_MSG(("\tMode: %s", _ctx.server ? "server" : "client"));
    if (_ctx.server)
    {
        BDBG_MSG(("\tlisten on port %d", _ctx.port));
    }
    else
    {
        BDBG_MSG(("\tconnect to %s:%d", _ctx.hostname, _ctx.port));
    }
    BDBG_MSG(("\tTLS version: %s", _ctx.tls1_2 ? "1.2" : "1.0"));
    BDBG_MSG(("\tCipher List: %s", _ctx.cipher));
    BDBG_MSG(("\tCertificate Path: %s", _ctx.cert_path));
    BDBG_MSG(("\tBinFile Path: %s", _ctx.binfile_path));
    BDBG_MSG(("\tKey index: %u", _ctx.key_id));
    BDBG_MSG(("\tCA Cert Path: %s", _ctx.CAcert_path));
    BDBG_MSG(("\tEngine .so Path: %s", _ctx.engine_path));

    if (!_ctx.server && !_ctx.hostname)
    {
        BDBG_ERR(("Server mode requires a hostname to connect to."));
        _usage();
        return 1;
    }

    if (_ctx.key_id > 7)
    {
        BDBG_ERR(("Key Index shall be in [0, 7] range"));
        _usage();
        return 2;
    }

    if (_ctx.cert_path && access(_ctx.cert_path, R_OK))
    {
        BDBG_ERR(("Cannot access Certificate '%s'", _ctx.cert_path));
        _usage();
        return 3;
    }

    if (access(_ctx.binfile_path, R_OK))
    {
        BDBG_ERR(("Cannot access DRM BinFile '%s'", _ctx.binfile_path));
        _usage();
        return 4;
    }

    if (_ctx.CAcert_path && access(_ctx.CAcert_path, R_OK))
    {
        BDBG_ERR(("Cannot access CA certificate '%s'", _ctx.CAcert_path));
        _usage();
        return 5;
    }

    if (access(_ctx.engine_path, R_OK))
    {
        BDBG_ERR(("Cannot access Engine '%s'", _ctx.engine_path));
        _usage();
        return 6;
    }

    return 0;
}

static int
_parse_cmdline(int ac, char **av)
{
    int i = 2;

    if (ac < 2) { goto err; }

    if (!strcmp(av[1], "client"))
    {
        _ctx.server = 0;
    }
    else if (!strcmp(av[1], "server"))
    {
        _ctx.server = 1;
    }
    else { goto err; }

    while (i < ac) {
        if (!strcmp(av[i], "-port"))
        {
            if (++i >= ac) { goto err; }
            _ctx.port = atoi(av[i++]);
        }
        else if (!strcmp(av[i], "-key_id"))
        {
            if (++i >= ac) { goto err; }
            _ctx.key_id = (uint8_t)atoi(av[i++]);
        }
        else if (!strcmp(av[i], "-hostname"))
        {
            if (++i >= ac) { goto err; }
            _ctx.hostname = av[i++];
        }
        else if (!strcmp(av[i], "-cert"))
        {
            if (++i >= ac) { goto err; }
            _ctx.cert_path = av[i++];
        }
        else if (!strcmp(av[i], "-binfile"))
        {
            if (++i >= ac) { goto err; }
            _ctx.binfile_path = av[i++];
        }
        else if (!strcmp(av[i], "-CAcert"))
        {
            if (++i >= ac) { goto err; }
            _ctx.CAcert_path = av[i++];
        }
        else if (!strcmp(av[i], "-engine"))
        {
            if (++i >= ac) { goto err; }
            _ctx.engine_path = av[i++];
        }
        else if (!strcmp(av[i], "-tls1_2"))
        {
            _ctx.tls1_2 = 1;
            i++;
        }
        else if (!strcmp(av[i], "-tls1"))
        {
            _ctx.tls1_2 = 0;
            i++;
        }
        else if (!strcmp(av[i], "-cipher"))
        {
            if (++i >= ac) { goto err; }
            _ctx.cipher = av[i++];
        }
        else if (!strcmp(av[i], "-debug"))
        {
            if (++_ctx.debug == 1)
            {
                BDBG_SetModuleLevel("rsa_sage_engine_example", BDBG_eMsg);
            }
            i++;
        }
        else { goto err; }
    }

    return 0;

err:
    _usage();
    return -i;
}

/*********************************************************************************
 * main
 *********************************************************************************/
int main(int argc, char* argv[])
{
    BIO *bio_err=NULL;
    BERR_Code rc = BERR_SUCCESS;
    const char *engine_id="rsa_sage_engine";
    const char *pre_cmds[]={
        "SO_PATH", ENGINE_DEFAULT_SO_PATH, /* must be absolute according to doc? */
        "ID","rsa_sage_engine",
        "LOAD", NULL /* <- triggers the call to bind() */
    };
    const char *post_cmds[]={
        "SAGE_UTILITY_TL", NULL
    };
    ENGINE *engine = NULL;

    bin_name = argv[0];

    BDBG_LOG(("***************************************************************************************"));
    BDBG_LOG(("***\tBroadcom Corporation / OpenSSL RSA engine example (Copyright 2014)"));
    BDBG_LOG(("***************************************************************************************"));

    if (_parse_cmdline(argc, argv) || _verify_dump_arguments())
    {
        goto handle_error_no_join;
    }

    rc = SAGE_app_join_nexus();
    if(rc != 0)
    {
        goto handle_error;
    }

    bio_err=BIO_new(BIO_s_file());
    BIO_set_fp(bio_err,stderr,BIO_NOCLOSE|BIO_FP_TEXT);
    ERR_load_crypto_strings();
    SSL_load_error_strings();
    SSLeay_add_ssl_algorithms();
    ENGINE_load_builtin_engines();

    ENGINE_load_dynamic();
    BDBG_MSG(("Dynamic engine loaded"));

    print_openssl_error_stack();

    /* patch post_cmds */
    post_cmds[1] = _ctx.binfile_path;
    if (generic_load_engine_fn(engine_id,
                               pre_cmds,sizeof(pre_cmds)/sizeof(char*)/2,
                               post_cmds,sizeof(post_cmds)/sizeof(char*)/2) != 0)
    {
        BDBG_ERR(("Call to 'generic_load_engine_fn' failed"));
        print_openssl_error_stack();
        goto handle_error;
    }

    BDBG_MSG(("Engine loaded and bound"));
    engine = ENGINE_by_id(engine_id);
    if (!engine)
    {
        BDBG_ERR(("ENGINE_by_id FAILED, '%s' isn't available *", engine_id));
        goto handle_error;
    }

    BDBG_MSG(("ENGINE_by_id('%s') SUCCESS.  engine = '%p'", engine_id, engine));

    if (!ENGINE_init(engine))
    {
        ENGINE_free(engine);
        BDBG_ERR(("The engine couldn't initialise, release 'e'"));
        goto handle_error;
    }

    rc = OpensslEngine_Test(engine);
    if (rc != 0)
    {
        BDBG_ERR(("TEST FAILURE"));
        goto handle_error;
    }

    BDBG_LOG(("Test success!!!!"));
handle_error:

    /* Leave Nexus: Finalize platform ... */
    BDBG_MSG(("Closing Nexus driver..."));
    SAGE_app_leave_nexus();

handle_error_no_join:

    if (rc)
    {
        BDBG_ERR(("# Failure in SAGE OpenSSL engine test"));
    }

    return rc;
}



int
generic_load_engine_fn(
    const char *engine_id,
    const char **pre_cmds, int pre_num,
    const char **post_cmds, int post_num)
{
    ENGINE *e = ENGINE_by_id(engine_id);

    if(!e)
    {
        BDBG_MSG(("'%s' failed to load, trying dynamic", engine_id));
        if(!e)
        {
            e = ENGINE_by_id("dynamic");
            if (e)
            {
                BDBG_MSG(("Dynamic loaded OK ('e' = '%p')", e));
            }
            else
            {
                BDBG_ERR(("Dynamic engine FAILED to load (call ENGINE_load_builtin_engines)"));
                return(1);
            }
        }
    }

    while(pre_num--)
    {
        BDBG_MSG(("ENGINE (%p) pre ctrl '%s' --> '%s'", e, pre_cmds[0], pre_cmds[1]));
        if(!ENGINE_ctrl_cmd_string(e, pre_cmds[0], pre_cmds[1], 0))
        {
            BDBG_WRN(("Failed command (%s - %s:%s)",
                      engine_id, pre_cmds[0],
                      pre_cmds[1] ? pre_cmds[1] : "(NULL)"));
            BDBG_WRN(("FIXME !!! BRCM TOOLCHAIN BUG IGNORED !!!"));

/*          WORKAROUND >>
            FOR A BUG IN OpenSSL compiled with toolchain for 3.8.xxx kernel          */
/*          ENGINE_free(e);
            return 1;*/
/*          WORKAROUND << */
        }
        pre_cmds += 2;
    }

    if(!ENGINE_init(e))
    {
        BDBG_ERR(("Failed initialization"));
        ENGINE_free(e);
        return 1;
    }
    /* ENGINE_init() returned a functional reference, so free the structural
     * reference from ENGINE_by_id(). */

    ENGINE_free(e);

    while(post_num--)
    {
        BDBG_MSG(("ENGINE (%p) post ctrl '%s' --> '%s'", e, post_cmds[0], post_cmds[1]));
        if(!ENGINE_ctrl_cmd_string(e, post_cmds[0], post_cmds[1], 0))
        {
            BDBG_WRN(("Failed command (%s - %s:%s)",
                      engine_id, post_cmds[0],
                      post_cmds[1] ? post_cmds[1] : "(NULL)"));
            BDBG_WRN(("FIXME !!! BRCM TOOLCHAIN BUG IGNORED !!!"));
        }
        post_cmds += 2;
    }

    /* Success */
    return 0;
}
