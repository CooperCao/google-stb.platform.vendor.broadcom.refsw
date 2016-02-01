/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Playready library
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef __DRMHTTP_H__
#define __DRMHTTP_H__


#include <stdint.h>                 /* standard includes*/
#include <stdio.h>
#include "blst_slist.h"             /* brcm linked list */

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
typedef BLST_S_HEAD(bhttp_hdr_lst, bhttp_hdr_indiv) bhttp_hdr_lst;

typedef enum bdrm_http_state {
    bdrm_http_status_failed_getpost          = (-7),  /* http get/post failure */
    bdrm_http_status_failed_connect          = (-6),  /* connection failure */
    bdrm_http_status_failed_internal         = (-5),  /* internal failure */
    bdrm_http_status_failed_response_read    = (-4),  /* could not read response */
    bdrm_http_status_failed_response_code    = (-3),  /* response code was not ok */
    bdrm_http_status_failed_no_license       = (-2),  /* response was ok but no license */
    bdrm_http_status_failed_generic          = (-1),  /* generic failure */
    bdrm_http_status_ok                      = (0),   /* response had a license */
    bdrm_http_status_undefined               = (1)    /* http was not involved */
} bdrm_http_state;

/* individual http header structure
*/
typedef struct bhttp_hdr_indiv {
    BLST_S_ENTRY(bhttp_hdr_indiv) link; /* list link */
    const char* hdr_name;               /* header name */
    char* hdr_value;                    /* header value */
    uint32_t hdr_name_len;              /* header name length */
    uint32_t hdr_value_len;             /* header value length */
} bhttp_hdr_indiv;

/* http engine context structure
*/
typedef struct bhttp_engine {
    uint8_t _useSendRecv;           /* if 1, use _currentFd, else _inputFd/_outputFd; */
    FILE *_currentFile;             /* use this if !NULL, else _currentFd */
    int32_t _currentFd;             /* currently used file descriptor */
    int32_t _inputFd, _outputFd;    /* i/o file descriptors */
    uint8_t _printHttp;             /* toggle debug headers */
    bhttp_hdr_lst _headers;         /* singly linked list of http headers */
    uint8_t _wroteHeaders;          /* bool for write header status */
    uint8_t _readHeaders;           /* bool for read header status */
    uint32_t _responseCode;         /* header response code */
} bhttp_engine;

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
int bhttpengine_headers_cleanup (
    bhttp_engine* http              /* [in] http engine instance */
    );

/* read one line at a time from the read end of the connection
*/
char* bhttpengine_read_line (
    bhttp_engine* http,             /* [in] http engine instance */
    char* buf,                      /* [out] line buffer */
    uint32_t len                    /* [in] line buffer length */
    );

/* add new header using a name/int value pair
*/
void bhttpengine_set_header (
    bhttp_engine* http,             /* [in] http engine instance */
    const char* name,               /* [in] header name */
    uint32_t value                  /* [in] numeric header value */
    );

/* add a new header name/value pair to the http context
*/
void bhttpengine_set_headers (
    bhttp_engine* http,             /* [in] http engine instance */
    const char* name,               /* [in] header name */
    char* value                     /* [in] stringized header value */
    );

/* clean up and refill the list of headers
*/
uint32_t bhttpengine_read_headers (
    bhttp_engine* http              /* [in] http engine instance */
    );

/* format and write to output all headers in the specified instance
*/
int32_t bhttpengine_write_headers (
    bhttp_engine* http              /* [in] http engine instance */
    );

/* close/shutdown opened descriptors for the current instance
*/
void bhttpengine_close (
    bhttp_engine* http              /* [in] http engine instance */
    );

/* internal function used for write ops
*/
int32_t bhttpengine_internal_write (
    bhttp_engine* http,             /* [in] http engine instance */
    const char* buf,                /* [in] internal buffer */
    int32_t len                     /* [in] internal buffer length */
    );

/* wrapper on top of the write functions for headers and data
*/
int32_t bhttpengine_write (
    bhttp_engine* http,             /* [in] http engine instance */
    const char* buf,                /* [in] write buffer */
    int32_t len                     /* [in] write buffer length */
    );

/* flush outputs for current file descriptor
*/
void bhttpengine_flush (
    bhttp_engine* http              /* [in] http engine instance */
    );

/* generic read function, wrapper on top of the IOs
*/
uint32_t bhttpengine_read (
    bhttp_engine* http,             /* [in] http engine instance */
    char* buf,                      /* [out] read buffer */
    uint32_t len                    /* [in] read buffer length */
    );

/* create a new instance of the http engine and init
*/
void bhttpengine_init (
    bhttp_engine* http              /* [in] http engine instance */
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
int32_t bhttpclient_post (
    bhttp_engine* http,             /* [in] http engine instance */
    char *url                       /* [in] url to post to */
    );

/* handling function for the headers coming inbound from the server
*/
int32_t bhttpclient_read_responsehdr (
    bhttp_engine* http              /* [in] http engine instance */
    );

/* drm license post function; feeds a SOAP drm challenge to a server, parses
   the response looking for the license section, and passes it back to
   the drm engine for processing.
*/
int32_t bhttpclient_license_post_soap (
    char* url,                      /* [in] url to post to */
    char* chall,                    /* [in] drm challenge */
    unsigned char** resp,           /* [out] drm response */
    uint32_t* resp_len              /* [out] drm response length */
    );

#endif /*__DRMHTTP_H__*/
