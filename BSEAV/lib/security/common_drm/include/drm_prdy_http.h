/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ******************************************************************************/

#ifndef __DRM_PRDY_HTTP_H__
#define __DRM_PRDY_HTTP_H__

#ifndef __KERNEL__
#include <stdint.h>                 /* standard includes*/
#include <stdio.h>
#else
#include "drmcommon.h"
#include <generated/autoconf.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/string.h>
#endif




#include "blst_slist.h"             /* brcm linked list */

#if defined(_WIN32)
    #include <io.h>                 /* open/close */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* external buffer sizes
*/
#define MAX_RESPONSE_NUM    (0x04)  /* max response buff size */
#define MAX_PROTO           (0x08)  /* proto:// max size */
#define MAX_HOST            (0x40)  /* host max size */
#define MAX_QUERY           (0xFF)  /* max query buff size */
#define MAX_FRAGMENT        (0x20)  /* fragment/label max size */
#define MAX_PATH            (0x80)  /* path max size */

/* http headers singly linked list
*/
typedef BLST_S_HEAD(DRM_Prdy_http_hdr_lst, DRM_Prdy_http_hdr_indiv) DRM_Prdy_http_hdr_lst;

/* individual http header structure
*/
typedef struct DRM_Prdy_http_hdr_indiv {
    BLST_S_ENTRY(DRM_Prdy_http_hdr_indiv) link; /* list link */
    char* hdr_name;                     /* header name */
    char* hdr_value;                    /* header value */
    uint32_t hdr_name_len;              /* header name length */
    uint32_t hdr_value_len;             /* header value length */
} DRM_Prdy_http_hdr_indiv;

/* http engine context structure
*/
typedef struct DRM_Prdy_http_engine {
    uint8_t _useSendRecv;           /* if 1, use _currentFd, else _inputFd/_outputFd; */
    FILE *_currentFile;             /* use this if !NULL, else _currentFd */
    int32_t _currentFd;             /* currently used file descriptor */
    int32_t _inputFd, _outputFd;    /* i/o file descriptors */
    uint8_t _printHttp;             /* toggle debug headers */
    DRM_Prdy_http_hdr_lst _headers;         /* singly linked list of http headers */
    uint8_t _wroteHeaders;          /* bool for write header status */
    uint8_t _readHeaders;           /* bool for read header status */
    uint32_t _responseCode;         /* header response code */
#ifdef _WINSOCKAPI_
    WSADATA WinSockData;            /* winsock data descriptor */
#endif
} DRM_Prdy_http_engine;

/* signifies drm encrypted or not
*/
typedef enum bdrm_encr_type {
    bdrm_http_encr_none     = (0),                /* no encryption */
    bdrm_http_encr_wmdrm    = (1),                /* wmdrm encrypted asf */
    bdrm_http_encr_aes_ctr  = (2),                /* AES CTR encrypted stream */
    bdrm_http_encr_aes_cbc  = (3),                /* AES CBC encrypted stream */
    bdrm_http_encr_max      = (4)
} bdrm_encr_type;


/* url object context
*/
typedef struct bdrm_url {
    uint8_t _search;                /* is this a search */
    uint16_t _port;                 /* port value */
    char* _url;                     /* full url ptr */
    char* _query;                   /* ptr into query */
    char _protocol[MAX_PROTO];      /* protocol prefix */
    char _host[MAX_HOST];           /* server host */
    char _fragment[MAX_FRAGMENT];   /* label after '#' */
    char _path[MAX_PATH];           /* remote server path */
} bdrm_url;


/* HTTP ENGINE APIs
*/

/* clean up the list of headers and free allocated ones
*/
int DRM_Prdy_http_engine_headers_cleanup (
    DRM_Prdy_http_engine* http              /* [in] http engine instance */
    );

/* read one line at a time from the read end of the connection
*/
char* DRM_Prdy_http_engine_read_line (
    DRM_Prdy_http_engine* http,             /* [in] http engine instance */
    char* buf,                      /* [out] line buffer */
    uint32_t len                    /* [in] line buffer length */
    );

/* add new header using a name/int value pair
*/
void DRM_Prdy_http_engine_set_header (
    DRM_Prdy_http_engine* http,             /* [in] http engine instance */
    const char* name,               /* [in] header name */
    uint32_t value                  /* [in] numeric header value */
    );

/* add a new header name/value pair to the http context
*/
void DRM_Prdy_http_engine_set_headers (
    DRM_Prdy_http_engine* http,             /* [in] http engine instance */
    const char* name,               /* [in] header name */
    char* value                     /* [in] stringized header value */
    );

/* clean up and refill the list of headers
*/
uint32_t DRM_Prdy_http_engine_read_headers (
    DRM_Prdy_http_engine* http              /* [in] http engine instance */
    );

/* format and write to output all headers in the specified instance
*/
int32_t DRM_Prdy_http_engine_write_headers (
    DRM_Prdy_http_engine* http              /* [in] http engine instance */
    );

/* close/shutdown opened descriptors for the current instance
*/
void DRM_Prdy_http_engine_close (
    DRM_Prdy_http_engine* http              /* [in] http engine instance */
    );

/* internal function used for write ops
*/
int32_t DRM_Prdy_http_engine_internal_write (
    DRM_Prdy_http_engine* http,             /* [in] http engine instance */
    const char* buf,                /* [in] internal buffer */
    int32_t len                     /* [in] internal buffer length */
    );

/* wrapper on top of the write functions for headers and data
*/
int32_t DRM_Prdy_http_engine_write (
    DRM_Prdy_http_engine* http,             /* [in] http engine instance */
    const char* buf,                /* [in] write buffer */
    int32_t len                     /* [in] write buffer length */
    );

/* flush outputs for current file descriptor
*/
void DRM_Prdy_http_engine_flush (
    DRM_Prdy_http_engine* http              /* [in] http engine instance */
    );

/* generic read function, wrapper on top of the IOs
*/
uint32_t DRM_Prdy_http_engine_read (
    DRM_Prdy_http_engine* http,             /* [in] http engine instance */
    char* buf,                      /* [out] read buffer */
    uint32_t len                    /* [in] read buffer length */
    );

/* create a new instance of the http engine and init
*/
void DRM_Prdy_http_engine_init (
    DRM_Prdy_http_engine* http              /* [in] http engine instance */
    );

/* set url from char, parse elements and store in members.
   determine the protocol port number, based on proto prefix.
*/
void burl_set (
    bdrm_url* drm_url,              /* [in] url object instance */
    char *drm_surl                  /* [in] stringized drm url */
    );

/* URL PROCESSOR APIs
*/

/* print members of a url object
*/
void burl_print (
    char *drm_surl                  /* [in] stringized drm url */
    );

/* translate url from http encoded array
*/
void burl_from_encoding (
    char* drm_surl,                 /* [out] stringized drm url */
    const char *str                 /* [in] stringized urlencoded drm url */
    );

/* translate from url to http encoding
*/
void burl_to_encoding (
    char* drm_surl,                 /* [out] stringized urlencoded drm url */
    const char *str                 /* [in] stringized drm url */
    );

/* HTTP CLIENT APIs
*/

/* 'http post' wrapper function
*/
int32_t DRM_Prdy_http_client_post (
    DRM_Prdy_http_engine* http,             /* [in] http engine instance */
    char *url                       /* [in] url to post to */
    );

/* handling function for the headers coming inbound from the server
*/
int32_t DRM_Prdy_http_client_read_responsehdr (
    DRM_Prdy_http_engine* http              /* [in] http engine instance */
    );

/* drm license post function; feeds an XML drm challenge to a server, parses
   the response looking for the license section, and passes it back to
   the drm engine for processing.
*/
int32_t DRM_Prdy_http_client_license_post_default (
    char* url,                      /* [in] url to post to */
    char* chall,                    /* [in] drm challenge */
    uint8_t non_quiet,              /* [in] indicates acq quietness */
    uint32_t app_sec,               /* [in] application security level */
    unsigned char** resp,           /* [out] drm response */
    uint32_t resp_len,              /* [in] drm response buffer length */
    uint32_t* offset,     /* [out] ptr to license response start  */
    uint32_t* out_resp_len          /* [out] drm response length */
    );

/* drm license post function; feeds a SOAP drm challenge to a server, parses
   the response looking for the license section, and passes it back to
   the drm engine for processing.
*/
int32_t DRM_Prdy_http_client_license_post_soap (
    char* url,                      /* [in] url to post to */
    char* chall,                    /* [in] drm challenge */
    uint8_t non_quiet,              /* [in] indicates acq quietness */
    uint32_t app_sec,               /* [in] application security level */
    unsigned char** resp,           /* [out] drm response */
    uint32_t resp_len,              /* [in] drm response buffer length */
    uint32_t* offset,     /* [out] ptr to license response start */
    uint32_t* out_resp_len          /* [out] drm response length */
    );

/* drm secure time challenge post function; feeds a SOAP drm secure time challenge
   to a server, return the response.

   The secure clock challenge request is an HTTP/1.0 POST request as shown below:

   POST ChallengePath HTTP/1.0
   Content-Type: application/x-www-form-urlencoded
   Content-Length: 376
   User-Agent: Client-User-Agent
   Proxy-Connection: Keep-Alive
   Pragma: no-cache
*/
int32_t DRM_Prdy_http_client_time_challenge_post (
    char* url,                      /* [in] url to post to */
    char* chall,                    /* [in] drm challenge */
    uint8_t non_quiet,              /* [in] indicates acq quietness */
    uint32_t app_sec,               /* [in] application security level */
    unsigned char** resp,           /* [out] drm response */
    uint32_t resp_len,              /* [in] drm response buffer length */
    uint32_t* offset,     /* [out] ptr to license response start */
    uint32_t* out_resp_len          /* [out] drm response length */
    );

/* drm secure time petition get request; feeds a http GET petition request
   to a server, return the response from the server

   The petition request is a plain HTTP/1.0 GET request, which must have contain the
   following in the header:

   GET PetitionPath HTTP/1.0
   User-Agent: Client-User-Agent
*/
int32_t DRM_Prdy_http_client_get_petition (
    char* petition_url,             /* [in] petition url */
    uint32_t *petition_response,    /* [out] either 200, 301 or 302 */
    char** time_chall_url           /* [out] the time challenge URL or redirect url */
    );

#ifdef __cplusplus
}
#endif

#endif /*__DRMHTTP_H__*/
