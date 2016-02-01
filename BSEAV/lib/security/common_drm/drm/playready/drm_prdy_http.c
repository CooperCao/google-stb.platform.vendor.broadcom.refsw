/******************************************************************************
* (c) 2015 Broadcom Corporation
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
******************************************************************************/

#include <sys/types.h>                  /* standard includes */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "bstd.h"                       /* brcm includes */
#include "bdbg.h"
#include "bkni.h"

#include <drm_prdy_http.h>                    /* local function decls */

#if defined(__vxworks)
    #include <sockLib.h>                /* struct timeval */
    #include <selectLib.h>              /* select() */
#endif

#if defined(__unix__) || defined(__vxworks)
#include <netinet/in.h>
    #include <arpa/inet.h>              /* inet_pton */
    #include <netdb.h>                  /* gethostbyname */
    #include <unistd.h>
    #include <sys/socket.h>             /* socket/bind/etc. */
#endif

BDBG_MODULE(DRM_Prdy_http);                 /* drm http debug handle */

/* local defines
*/
#define HTTP_NOTCONNECTED       (0x00)      /* connection status is off */
#define HTTP_CONNECTED          (0x01)      /* connection is on */
#define HTTP_KEEP_ALIVE_OFF     (0x00)      /* keep alive header off */
#define HTTP_KEEP_ALIVE_ON      (0x01)      /* keep alive header on */
#define HTTP_MAX_HDR_LINE       (0x400)     /* manipulation buffer defines */
#define HTTP_MAX_FILE_BUFF      (0xFFF)     /* max header line size */
#define MAX_HDR_INT_NUM         (16)        /* max digits in a header int */
#define HTTP_XMLRPC_BUFSIZE     (1024*64)   /* xml RPC buffer size */
#define HTTP_OK_RESPONSE        (200)       /* http 'OK' response code */

/* stringized/misc. defines
*/
/* URL encoded content type */
#define HDR_VALUE_URLENCODED_CT    "application/x-www-form-urlencoded"         /* specifies urlencoded posting */
/* Content type is xml.  Character set is utf-8 (used by wmdrm server?) */
#define HDR_VALUE_XML_UTF8_CT      "text/xml; charset=utf-8"         /* specifies urlencoded posting */
#define HDR_VALUE_HOST_CT      "playready.directtaps.net"

#define DRM_POST_PREFIX         "appsecurity=%d&nonsilent=%d&challenge="    /* drm specific post prefix, pre-challenge */
#define PROTO_FILE_TOKEN        "file:"                 /* protocol token present in file:// */
#define PROTO_HTTP_UNSAFE       ";@&<>\"#%{}|\\^~[]` "  /* invalid chars in http address */
#define LICRESPONSE_OPENED      "<LICENSERESPONSE>"     /* open xml tag in license response */
#define LICRESPONSE_CLOSED      "</LICENSERESPONSE>"    /* closed xml tag in license response */
#define HDR_TAG_CONTENT_LENGTH  "Content-Length"        /* header tag for content-length */
#define HDR_TAG_CONTENT_TYPE    "Content-Type"          /* header tag for content-type */
#define HDR_TAG_SOAP_ACTION     "SOAPAction"
#define HDR_TAG_HOST            "HOST"
#define HDR_ACTION_VALUE        "\"http://schemas.microsoft.com/DRM/2007/03/protocols/AcquireLicense\""
#define HDR_TAG_LOCATION_VALUE  "Location"

/* zero-pad string macros
*/
#define CLRSTR_LEN(s, len)  bzero((s), (len));
#define CLRSTR(str)         CLRSTR_LEN((str), (strlen((str))));

/* add a new header name/value pair to the http context
*/
void
DRM_Prdy_http_engine_set_headers(DRM_Prdy_http_engine* http, const char* name, char* value)
{
    uint32_t name_len, value_len;
    DRM_Prdy_http_hdr_indiv* http_hdr;

    http_hdr = (DRM_Prdy_http_hdr_indiv*)BKNI_Malloc(sizeof(DRM_Prdy_http_hdr_indiv));
    name_len = strlen(name);
    http_hdr->hdr_name = (char*)BKNI_Malloc(name_len + 1);
    strcpy(http_hdr->hdr_name,&name[0]);
    /*http_hdr->hdr_name = name;*/

    value_len = strlen(value);
    http_hdr->hdr_value = (char*)BKNI_Malloc(value_len + 1);
    strcpy(http_hdr->hdr_value, value);

    http_hdr->hdr_name_len = name_len;
    http_hdr->hdr_value_len = value_len;
    BLST_S_INSERT_HEAD(&http->_headers, http_hdr, link);
}

/* create a new instance of the http engine and init
*/
void
DRM_Prdy_http_engine_init(DRM_Prdy_http_engine* http)
{
    BLST_S_INIT(&http->_headers);
    http->_readHeaders = 0;
    http->_printHttp = 0;
    http->_useSendRecv = 1;
    http->_currentFile = NULL;
    http->_currentFd = http->_outputFd = http->_inputFd = -1;
#ifdef _WINSOCKAPI_
    WSAStartup(MAKEWORD(1,1), &http->WinSockData);
#endif
}

/* destroy an existing http engine instance
*/
void
DRM_Prdy_http_engine_uninit(DRM_Prdy_http_engine* http)
{
    BSTD_UNUSED(http);
#ifdef _WINSOCKAPI_
    WSACleanup();
#endif
}

/* close/shutdown opened descriptors for the current instance
*/
void
DRM_Prdy_http_engine_close(DRM_Prdy_http_engine* http)
{
#ifndef NO_SOCKETS
    if (http->_currentFd >= 0)
        shutdown(http->_currentFd, 2);      /* SHUT_RDWR */
#endif
    if (http->_currentFile) {
        fclose(http->_currentFile);
        http->_currentFd = -1;
        http->_currentFile = NULL;
    }
    else if (http->_currentFd >= 0) {
#if defined(_WIN32)
        closesocket(http->_currentFd);
#else
        close(http->_currentFd);
#endif
        http->_currentFd = -1;
    }
}

/* returns connection state for the specified instance
*/
uint8_t
DRM_Prdy_http_engine_is_connected(DRM_Prdy_http_engine* http)
{
    fd_set socket_set;
    struct timeval tv;

    if (http->_currentFile)
        return (HTTP_CONNECTED);
    if (!(http->_useSendRecv) && ((http->_inputFd >= 0) || (http->_outputFd >= 0)))
        return (HTTP_CONNECTED);
    if (http->_currentFd == -1)
        return (HTTP_NOTCONNECTED);

    FD_ZERO(&socket_set); FD_SET(http->_currentFd, &socket_set);
    tv.tv_sec = tv.tv_usec = 0;
    if (select(http->_currentFd + 1, &socket_set, NULL, NULL, &tv) < 0)
        return (HTTP_NOTCONNECTED);

    return (HTTP_CONNECTED);
}

/* add new header using a name/int value pair
*/
void
DRM_Prdy_http_engine_set_header(DRM_Prdy_http_engine* http, const char* name, uint32_t value)
{
    char strval[MAX_HDR_INT_NUM];
    sprintf(strval, "%d", value);
    DRM_Prdy_http_engine_set_headers(http, name, strval);
}

void
DRM_Prdy_http_engine_get_header2(DRM_Prdy_http_engine* http, const char* name, char *buf )
{
    DRM_Prdy_http_hdr_indiv* http_hdr = NULL;

    for (http_hdr = BLST_S_FIRST(&http->_headers); http_hdr;
        http_hdr = BLST_S_NEXT(http_hdr, link)) {
            if (strcasecmp(http_hdr->hdr_name, name) == 0) {
                strcpy( buf, http_hdr->hdr_value);
                break;
        }
    }
}
/* return a value based on the header name, from the headers collection
*/
const char*
DRM_Prdy_http_engine_get_header(DRM_Prdy_http_engine* http, const char* name )
{
    DRM_Prdy_http_hdr_indiv* http_hdr = NULL;
    const char* value = NULL;

    for (http_hdr = BLST_S_FIRST(&http->_headers);
         http_hdr != NULL;
         http_hdr = BLST_S_NEXT(http_hdr, link))
    {
        /*printf("%s:%d: checking %s vs %s\n",__FUNCTION__,__LINE__,http_hdr->hdr_name,name);*/
        if (strcasecmp(http_hdr->hdr_name, name) == 0)
        {
            value = http_hdr->hdr_value;
            break;
        }
    }

    return (value);
}

/* truncates white-space chars off end of 'str'
*/
static void
DRM_Prdy_http_engine_truncsp(char* str)
{
    size_t str_len;

    if(str == NULL) return;
    str_len = strlen(str);
    while(str_len && (unsigned char)str[str_len - 1] <= ' ') {
        str_len--;
    }
    str[str_len] = 0;
}

/* skip up to 'chars'
*/
char*
DRM_Prdy_http_engine_strskip(const char* str, const char* chars)
{
    return (char*)str + strspn(str, chars);
}

/* read one line at a time from the read end of the connection
*/
char*
DRM_Prdy_http_engine_read_line(DRM_Prdy_http_engine* http, char* buf, uint32_t len)
{
    char ch, *s;
    fd_set socket_set;
    uint32_t rd = 0;

    if (!http->_currentFile) {
        while (rd < (len - 1)) {
            FD_ZERO(&socket_set);
#ifndef NO_SOCKETS
            if (http->_useSendRecv) {
                FD_SET(http->_currentFd, &socket_set);
                if (select(http->_currentFd + 1, &socket_set, NULL, NULL, NULL) < 1)
                    return (NULL);                          /* invalid fd */
                if (recv(http->_currentFd, &ch, 1, 0) < 1)
                    return (NULL);                          /* disconnected */
            }
            else
#endif
            {
                FD_SET(http->_inputFd, &socket_set);
                if (select(http->_inputFd + 1, &socket_set, NULL, NULL, NULL) < 1)
                    return (NULL);                          /* invalid fd */
                if (read(http->_inputFd, &ch, 1) < 1)
                    return (NULL);                          /* disconnected */
            }
            if (ch == '\n') break;
            buf[rd++] = ch;
        }
        buf[rd] = 0; s = buf;
    } else{
        s = fgets(buf, len, http->_currentFile);
    }

    DRM_Prdy_http_engine_truncsp(s);
    if ((http->_printHttp) && (s))
        BDBG_MSG(("RX: %s", s));

    return (s);
}

/* clean up the list of headers and free allocated ones
*/
int
DRM_Prdy_http_engine_headers_cleanup(DRM_Prdy_http_engine* http)
{
    DRM_Prdy_http_hdr_indiv* http_hdr=NULL;
    DRM_Prdy_http_hdr_indiv* pNext=NULL;
    int32_t ret = 0;

    for(http_hdr = BLST_S_FIRST(&http->_headers); http_hdr; http_hdr = pNext)
    {
        pNext = BLST_S_NEXT(http_hdr,link);
        BLST_S_REMOVE(&http->_headers,http_hdr,DRM_Prdy_http_hdr_indiv,link);
        if( http_hdr) {
            if( http_hdr->hdr_name) BKNI_Free(http_hdr->hdr_name);
            if( http_hdr->hdr_value) BKNI_Free(http_hdr->hdr_value);
            BKNI_Free(http_hdr);
        }
    }

    return (ret);
}

/* clean up and refill the list of headers
*/
uint32_t
DRM_Prdy_http_engine_read_headers(DRM_Prdy_http_engine* http)
{
    uint32_t name_len, value_len;
    char* line, *colon, *value;
    char buf[HTTP_MAX_HDR_LINE];
    DRM_Prdy_http_hdr_indiv* http_hdr;

    DRM_Prdy_http_engine_headers_cleanup(http);
    while ((line = DRM_Prdy_http_engine_read_line(http,buf, HTTP_MAX_HDR_LINE))) { /* now read the headers */
        if (!(!!*line)) break;                                                /* we've finished reading the header */

        if ((colon = strchr(line, ':')) != NULL) {
            value = DRM_Prdy_http_engine_strskip(colon + 1, " \t");
            http_hdr = (DRM_Prdy_http_hdr_indiv*)BKNI_Malloc(sizeof(DRM_Prdy_http_hdr_indiv));

            name_len = colon - line;
            *colon = '\0';
            http_hdr->hdr_name = (char*)BKNI_Malloc(name_len + 1);
            strcpy(http_hdr->hdr_name,line);
            /*http_hdr->hdr_name = line;*/
            value_len = strlen(value);
            http_hdr->hdr_value = (char*)BKNI_Malloc(value_len + 1);

            strcpy(http_hdr->hdr_value, value);
            http_hdr->hdr_name_len = name_len;
            http_hdr->hdr_value_len = value_len;
            BLST_S_INSERT_HEAD(&http->_headers, http_hdr, link);

            /* BEN Why freeing them?
            BKNI_Free(http_hdr->hdr_value);
            BKNI_Free(http_hdr);
            */
        }
    }
    if (!line) return (-1);

    return (0);
}

/* generic read function, wrapper on top of the IOs
*/
uint32_t
DRM_Prdy_http_engine_read(DRM_Prdy_http_engine* http, char* buf, uint32_t len)
{
    uint32_t rd_len = 0;

    if (!http->_currentFile) {
#ifndef NO_SOCKETS
        if (http->_useSendRecv)
        {
            rd_len = recv(http->_currentFd, buf, len, 0);
        }
        else
#endif
        {
            rd_len = read(http->_inputFd, buf, len);
        }
    }
    else
    {
        rd_len = fread(buf, 1, len, http->_currentFile);

        BDBG_MSG(("DRM_Prdy_http_engine_read len %d rd_len %d 0x%x",len, rd_len, http->_currentFile));
    }

    if (http->_printHttp)
    {
        BDBG_MSG(("RX: %.*s", rd_len, buf));
    }

    return (rd_len);
}

/* internal function used for write ops
*/
int32_t
DRM_Prdy_http_engine_internal_write(DRM_Prdy_http_engine* http, const char* buf, int32_t len)
{
    if (len == -1) len = strlen(buf);
    if (http->_printHttp) BDBG_MSG(("TX: %.*s", len, buf));
    if (!http->_currentFile) {
#ifndef NO_SOCKETS
        if (http->_useSendRecv)
            return send(http->_currentFd, (char* )buf, len, 0);
        else
#endif
            return write(http->_outputFd, (char* )buf, len);
    }
    else return fwrite(buf, 1, len, http->_currentFile);
}

/* format and write to output all headers in the specified instance
*/
int32_t
DRM_Prdy_http_engine_write_headers(DRM_Prdy_http_engine* http)
{
    DRM_Prdy_http_hdr_indiv* http_hdr;

    http->_wroteHeaders = 1;
    for (http_hdr = BLST_S_FIRST(&http->_headers);
         http_hdr;
         http_hdr = BLST_S_NEXT(http_hdr, link)) {
        if (DRM_Prdy_http_engine_internal_write(http, http_hdr->hdr_name, -1) == -1 ||
            DRM_Prdy_http_engine_internal_write(http, ": ", -1) == -1 ||
            DRM_Prdy_http_engine_internal_write(http, http_hdr->hdr_value, -1) == -1 ||
            DRM_Prdy_http_engine_internal_write(http, "\r\n", -1) == -1) {
                return (-1);
        }
    }
    if (DRM_Prdy_http_engine_internal_write(http, "\r\n", -1) == -1) {
        return (-1);
    }

    return (0);
}

/* wrapper on top of the write functions for headers and data
*/
int32_t
DRM_Prdy_http_engine_write(DRM_Prdy_http_engine* http, const char* buf, int32_t len)
{
    if (!http->_wroteHeaders)
        if (DRM_Prdy_http_engine_write_headers(http) == -1) {
            BDBG_MSG(("Couldn't write headers, errno %d", errno));
            return (-1);
        }
    if (buf)
    {
        return DRM_Prdy_http_engine_internal_write(http, buf, len);
    }

    return(-1);
}

/* format and output one single line of clrf terminated data
*/
uint32_t
DRM_Prdy_http_engine_writeline(DRM_Prdy_http_engine* http, const char* buf)
{
    DRM_Prdy_http_engine_write(http, buf, -1);
    return (DRM_Prdy_http_engine_write(http, "\r\n", -1));
}

/* get buffer containing response body, accounting for content length
*/
int32_t
DRM_Prdy_http_engine_read_body(DRM_Prdy_http_engine* http, char* buf, int32_t len)
{
    int32_t cl;
    const char* clstr;

    clstr = DRM_Prdy_http_engine_get_header(http, HDR_TAG_CONTENT_LENGTH);
    cl = (clstr) ? atoi(clstr) : (len);
    if (cl < len) len = cl;

    return DRM_Prdy_http_engine_read(http, buf, len);
}

/* write headers and data to a file
*/
uint32_t
DRM_Prdy_http_engine_write_file(DRM_Prdy_http_engine* http, const char* filename)
{
    uint32_t r;
    char buf[HTTP_MAX_FILE_BUFF];
    FILE* f = NULL;

    if ((f = fopen(filename, "r")) != NULL) {
        while (!feof(f)) {
            if (!(r = fread(buf, 1, HTTP_MAX_FILE_BUFF, f))) break;
            if (DRM_Prdy_http_engine_write(http, buf, r) == -1) break;
        }
        fclose(f);
        return (0);
    }

    return (-1);
}

/* read http engine data from a file
*/
uint32_t
DRM_Prdy_http_engine_read_file(DRM_Prdy_http_engine* http, const char* filename)
{
    uint32_t n;
    char buf[HTTP_MAX_FILE_BUFF];
    FILE* f = NULL;

    if ((f = fopen(filename, "w+")) != NULL) {
        do {n = DRM_Prdy_http_engine_read(http, buf, sizeof(buf));
             if (n) fwrite(buf, 1, n, f);
        } while (n > 0);
        fclose(f);
        return (0);
    }

    return (-1);
}

/* print headers loaded by the current engine instance
*/
void
DRM_Prdy_http_engine_print_headers(DRM_Prdy_http_engine* http)
{
    DRM_Prdy_http_hdr_indiv* http_hdr;

    for (http_hdr = BLST_S_FIRST(&http->_headers);
         http_hdr; http_hdr = BLST_S_NEXT(http_hdr, link)) {
            BDBG_MSG(("%s: %s", http_hdr->hdr_name, http_hdr->hdr_value));
    }
}

/* flush outputs for current file descriptor
*/
void
DRM_Prdy_http_engine_flush(DRM_Prdy_http_engine* http)
{
    if (http->_currentFile != NULL)
        fflush(http->_currentFile);
}

/* For some older platforms like 7413(7405C0) which use gethostbyname() instead of getaddrinfo() to resolve host names */
#if (BCHP_CHIP == 7405  && BCHP_VER == BCHP_VER_C0) || (BCHP_CHIP == 7335)
int TcpConnect(char *server, int port, int socket_type)
{
        int sd,rc;

        struct sockaddr_in localAddr, servAddr;
        struct hostent *h;

        BDBG_MSG(("TCP - Connection to %s:%d ...\n",server, port));

        h = gethostbyname(server);
        if (h == NULL) {
                perror("Unknown Host");
                return (-EINVAL);
        }

        servAddr.sin_family = h->h_addrtype;
        memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0],h->h_length);
        servAddr.sin_port = htons(port);

        /* Open Socket */
        sd = socket(AF_INET, socket_type, 0);
        if (sd<0) {
                perror("Socket Open Err");
                return -EINVAL;
        }

        localAddr.sin_family = AF_INET;
        localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        localAddr.sin_port = htons(0);

        if (bind(sd,(struct sockaddr *) &localAddr, sizeof(localAddr))) {
                perror("Socket Bind Err");
                return -EINVAL;
        }

        /* Connect to server */
        rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
        if (rc<0) {
                perror("Connect Error: Server busy or unavailable:");
                return -EBUSY;
        }

        BDBG_MSG(("TCP: Connection Success\n"));
        return sd;
}
#endif


/* client connect function. resolve host and open a connection
   to the remote http server.
*/
int32_t
DRM_Prdy_http_client_connect(DRM_Prdy_http_engine* http, const char *host, uint32_t port)
{
/* For some older platforms like 7413(7405C0) which use gethostbyname() instead of getaddrinfo() to resolve host names */
#if (BCHP_CHIP == 7405  && BCHP_VER == BCHP_VER_C0) || (BCHP_CHIP == 7335)
    http->_currentFd = TcpConnect((char*)host, port, SOCK_STREAM);
#else
    struct addrinfo hints;
    struct addrinfo *addrInfo = NULL;
    char portString[16];

    http->_currentFd = socket(AF_INET, SOCK_STREAM, 0);
    if(http->_currentFd < 0) {
        perror("cannot open socket");
        return (-1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* we dont know the family */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    memset(portString, 0, sizeof(portString));  /* getaddrinfo() requires port # in the string form */
    snprintf(portString, sizeof(portString)-1, "%d", port);
    if (getaddrinfo(host, portString, &hints, &addrInfo) != 0) {
        BDBG_ERR(("%s: ERROR: getaddrinfo failed for server:port: %s:%d, errno %d", __FUNCTION__, host, port, errno));
        return (-1);
    }

    if (connect(http->_currentFd, addrInfo->ai_addr, addrInfo->ai_addrlen) < 0) {
        BDBG_WRN(("Cannot connect to %s:%d, errno %d. Try using gethostbyname() inplace of getaddrinfo()", host, port, errno));
        DRM_Prdy_http_engine_close(http);
        return (-1);
    }
#endif
    http->_currentFile = fdopen(http->_currentFd, "r+");
    if (!http->_currentFile) {
        DRM_Prdy_http_engine_close(http);
        return (-1);
    }

    return (0);
}

/* client function to send a request. specify method to be either 'get',
   'post' or 'put', and urlstr to specify the url
*/
int32_t
DRM_Prdy_http_client_send_request(DRM_Prdy_http_engine* http, const char *method, char *urlstr)
{
    bdrm_url url;
    const char *query;
    char buf[HTTP_MAX_HDR_LINE];
    uint32_t len = 0;
    uint8_t keepalive = HTTP_KEEP_ALIVE_OFF;

    burl_set(&url, urlstr);
    if (http->_currentFd == -1) {
        if (!(!!*url._host) || DRM_Prdy_http_client_connect(http, url._host, url._port) == -1)
            return -1; //(bdrm_http_status_failed_connect);
    } else keepalive = HTTP_KEEP_ALIVE_ON;

    query = url._query; if (!query || !(!!*query)) { query = "/"; }
    len = snprintf(buf, HTTP_MAX_HDR_LINE, "%s %s HTTP/1.0\r\n", method, query);

    if (DRM_Prdy_http_engine_internal_write(http, buf, len) == -1)
        return -1; // (bdrm_http_status_failed_getpost);

    DRM_Prdy_http_engine_headers_cleanup(http);
    if (HTTP_KEEP_ALIVE_ON == keepalive)
    {
        DRM_Prdy_http_engine_set_headers(http, "Connection", "Keep-Alive");
    }

    http->_wroteHeaders = 0; http->_readHeaders = 0;

    return 0; // (bdrm_http_status_ok);
}

/* handling function for the headers coming inbound from the server
*/
int32_t
DRM_Prdy_http_client_read_responsehdr(DRM_Prdy_http_engine* http)
{
    char *line, *find;
    char buf[HTTP_MAX_HDR_LINE];
    char response[MAX_RESPONSE_NUM];

    BKNI_Memset( buf, 0, ( sizeof( char ) * HTTP_MAX_HDR_LINE ));
    BKNI_Memset( response, 0, ( sizeof( char ) * MAX_RESPONSE_NUM ));

    if (http->_readHeaders) {
        BDBG_ERR(("Cannot read headers twice."));
        return (-1);
    }

    http->_readHeaders = 1;     /* read the top line */
    line = DRM_Prdy_http_engine_read_line(http, buf, HTTP_MAX_HDR_LINE);
    if (!line) return (-1);

    find = strchr(line, ' ');  if (!find) return (-1);
    line = find + 1; find = strchr(line, ' ');  if (!find) return (-1);
    if((find - line) < (int32_t)sizeof(response)) {
        strncpy(response, line, find - line);
        http->_responseCode = atoi(response);;
        /*printf("%s %d - http repsonse code %d. \n",__FUNCTION__,__LINE__,http->_responseCode);*/
    }

    return DRM_Prdy_http_engine_read_headers(http);
}

/* 'http get' wrapper function
*/
int32_t
DRM_Prdy_http_get_petition(DRM_Prdy_http_engine* http, char *urlstr)
{
    bdrm_url url;
    const char *query;
    char buf[HTTP_MAX_HDR_LINE];
    uint32_t len = 0;

    burl_set(&url, urlstr);
    if (http->_currentFd == -1) {
        if (!(!!*url._host) || DRM_Prdy_http_client_connect(http, url._host, url._port) == -1)
            return -1;
    }

    /*query = url._query; if (!query || !*query) { query = "/"; }*/
    /*query = strstr(url._url,"//");*/
    query = url._url;
    if (!query || !(!!*query)) { query = "/";  }
    /*else { query += 2; } */
    printf("\t\t query: %s\n",query);
    len = snprintf(buf, HTTP_MAX_HDR_LINE, "%s %s HTTP/1.0\r\n", "GET", query);

    if (DRM_Prdy_http_engine_internal_write(http, buf, len) == -1)
    {
        BDBG_ERR(("%s:%d - Couldn't write headers internal", __FUNCTION__,__LINE__));
        return -1;
    }

    DRM_Prdy_http_engine_headers_cleanup(http);
    DRM_Prdy_http_engine_set_headers(http, "User-Agent", "Client-User-Agent");
    if (DRM_Prdy_http_engine_write_headers(http) == -1) {
            BDBG_ERR(("Couldn't write headers, errno %d", errno));
            return (-1);
    }
    DRM_Prdy_http_engine_flush(http);

    return DRM_Prdy_http_client_read_responsehdr(http);
}

int32_t DRM_Prdy_http_client_get_petition (
    char* petition_url,
    uint32_t *petition_response,
    char** time_chall_url
    )
{
    DRM_Prdy_http_engine http;
    int32_t post_ret;

    /*
    int32_t len = 0;
    int32_t resp_len = HTTP_XMLRPC_BUFSIZE;
    */

    BDBG_ASSERT(petition_url != NULL);
    BDBG_ASSERT(petition_response != NULL);
    BDBG_ASSERT(time_chall_url != NULL);

    if (*time_chall_url == NULL) return -1;

    /* initialize http engine and post */
    DRM_Prdy_http_engine_init(&http);
    /* printf("%s - http engine init success. \n",__FUNCTION__)*/
    if ((post_ret = DRM_Prdy_http_get_petition(&http, petition_url)) != 0) {
        BDBG_WRN(("DRM_Prdy_http_license_get failed on POST"));
        return (post_ret);
    }

   *petition_response =  http._responseCode;

   if((*petition_response == 301) || (*petition_response == 302))
   {
     const char* clstr;

     /*printf("%s:%d - the response from the petition server is redirect\n",__FUNCTION__,__LINE__);*/

     clstr = DRM_Prdy_http_engine_get_header(&http,HDR_TAG_LOCATION_VALUE );

     strcpy(*time_chall_url,clstr);
   }
   else if( *petition_response == 200)
   {
       char resp[HTTP_XMLRPC_BUFSIZE] = {0};
       int32_t len;
       char *pch = NULL;

       /* look for a time Challenge URL */
       bzero(resp, HTTP_XMLRPC_BUFSIZE);
       len = DRM_Prdy_http_engine_read(&http, (char *)resp, HTTP_XMLRPC_BUFSIZE);

       if( len <= 0 )
       {
           BDBG_ERR(("%s:%d - The response from the petition is empty", __FUNCTION__,__LINE__));
           return -1;
       }

       pch = strstr(resp,"http");
       strcpy( *time_chall_url,pch);

   }
   else
   {
     BDBG_ERR(("%s:%d - Response from the petition is unknown - %d\n",__FUNCTION__,__LINE__,*petition_response));
     return -1;
   }

   DRM_Prdy_http_engine_close(&http);
   DRM_Prdy_http_engine_headers_cleanup(&http);
   http._wroteHeaders = 0; http._readHeaders = 0;
   return 0; //(bdrm_http_status_ok);
}

int32_t
DRM_Prdy_http_client_time_challenge_post  (char* url, char* chall, uint8_t non_quiet,
                               uint32_t app_sec, unsigned char** resp,uint32_t resp_len,
                               uint32_t *offset, uint32_t* out_resp_len
                               )
{
    DRM_Prdy_http_engine http;
    int32_t post_ret;

    int32_t len = 0;
    /*int32_t resp_len = HTTP_XMLRPC_BUFSIZE;*/

    BSTD_UNUSED(non_quiet);
    BSTD_UNUSED(app_sec);
    BDBG_ASSERT(resp != NULL); BDBG_ASSERT(out_resp_len != NULL);

    if (*resp == NULL) return -1; //(bdrm_http_status_failed_internal);

    /* append drm specific tokens */
    #if 0
    BDBG_MSG(("\nposting to : %s", url));
    #endif
    #if 0
    len += sprintf(buf, DRM_POST_PREFIX, app_sec, non_quiet);
    #endif
    len += sprintf((char *)*resp + len, "%s", chall);
    /* printf("%s - %d get the len of the challenge %d\n",__FUNCTION__,__LINE__,len);*/

    /* initialize http engine and post */
    DRM_Prdy_http_engine_init(&http);
    /* printf("%s - http engine init success. \n",__FUNCTION__);*/
    if ((post_ret = DRM_Prdy_http_client_post(&http, url)) != 0) {
        BDBG_ERR(("%s:%d - failed on POST request",__FUNCTION__,__LINE__));
        return (post_ret);
    }

    /* set headers, read response */
    DRM_Prdy_http_engine_set_headers(&http, "Accept", "*/*");
    DRM_Prdy_http_engine_set_headers(&http, HDR_TAG_CONTENT_TYPE, HDR_VALUE_URLENCODED_CT);
    DRM_Prdy_http_engine_set_header(&http,  HDR_TAG_CONTENT_LENGTH, len);
    DRM_Prdy_http_engine_set_headers(&http, "User-Agent", "Client-User-Agent");
    DRM_Prdy_http_engine_set_headers(&http, "Proxy-Connection", "Keep-Alive");
    DRM_Prdy_http_engine_set_headers(&http, "Pragma", "no-cache");

    DRM_Prdy_http_engine_write(&http, (const char *)*resp, len);
    DRM_Prdy_http_engine_flush(&http);
    if (DRM_Prdy_http_client_read_responsehdr(&http)) {
        BDBG_WRN(("failed on readResponseHeader"));
        return -1; //(bdrm_http_status_failed_response_read);
    }
    #if 0
    BDBG_MSG(("HTTP DEBUG :: len <%d>, buf<%s>", len, buf));
    #endif

    /* look for a license */
    bzero(*resp, resp_len); len = DRM_Prdy_http_engine_read(&http, (char *)*resp, resp_len);
    /* printf("%s - get the len of the response message %d\n",__FUNCTION__,resp_len); */
    #if 0
    BDBG_MSG(("HTTP :: resp_len <%d>, buf<%s>", resp_len, buf));
    #endif

    *out_resp_len = len;
    *offset = 0;

	DRM_Prdy_http_engine_close(&http);
    DRM_Prdy_http_engine_headers_cleanup(&http);
    http._wroteHeaders = 0; http._readHeaders = 0;

    return 0; //(bdrm_http_status_ok);
}

/* 'http post' wrapper function
*/
int32_t
DRM_Prdy_http_client_post(DRM_Prdy_http_engine* http, char *url)
{
    return DRM_Prdy_http_client_send_request(http, "POST", url);
}

/* 'http put' wrapper function
*/
int32_t
DRM_Prdy_http_client_put(DRM_Prdy_http_engine* http, char *url)
{
    return DRM_Prdy_http_client_send_request(http, "PUT", url);
}

/* drm license post function; feeds an XML challenge to a server, parses
   the response looking for the license section, and passes it back to
   the drm engine for processing.
*/
int32_t
DRM_Prdy_http_client_license_post_default (char* url, char* chall, uint8_t non_quiet,
                                  uint32_t app_sec, unsigned char** resp,uint32_t resp_len,
                                  uint32_t *offset, uint32_t* out_resp_len
                                  )
{
    DRM_Prdy_http_engine http;
    int32_t post_ret;

    int32_t len = 0, licresp_len = 0;
    /*int32_t resp_len = HTTP_XMLRPC_BUFSIZE;*/
    char* licresp_opened = NULL, *licresp_closed = NULL;
    char* new_resp = BKNI_Malloc(resp_len);
    char *temp;
    char *tmpbuf = NULL;
    int i;


    BDBG_ASSERT(resp != NULL); BDBG_ASSERT(out_resp_len != NULL);

    if (*resp == NULL) return -1; //(bdrm_http_status_failed_internal);

    /* append drm specific tokens */
    #if 0
    BDBG_ERR(("posting to : %s", url));
    #endif

    len += sprintf((char *)*resp, DRM_POST_PREFIX, app_sec, non_quiet);
    len += sprintf((char *)*resp + len, "%s", chall);

    /* initialize http engine and post */
    DRM_Prdy_http_engine_init(&http);
    if ((post_ret = DRM_Prdy_http_client_post(&http, url)) != 0) {
        BDBG_WRN(("DRM_Prdy_http_license_post failed on POST"));
        return (post_ret);
    }
    /*printf("%s %d - DRM_Prdy_http_client_post success. \n",__FUNCTION__,__LINE__);*/
    /* set headers, read response */
    DRM_Prdy_http_engine_set_headers(&http, HDR_TAG_CONTENT_TYPE, HDR_VALUE_URLENCODED_CT);
    DRM_Prdy_http_engine_set_header(&http,  HDR_TAG_CONTENT_LENGTH, len);
    DRM_Prdy_http_engine_write(&http, (const char*)*resp, len);
    DRM_Prdy_http_engine_flush(&http);
    if (DRM_Prdy_http_client_read_responsehdr(&http)) {
        BDBG_WRN(("failed on readResponseHeader"));
        return -1; //(bdrm_http_status_failed_response_read);
    }

    /* look for a license */
    bzero(*resp, resp_len);
    len = DRM_Prdy_http_engine_read(&http, (char *)*resp, resp_len);

    /*Sometimes the response from server contains '\' characters, which the playready XML parser cannot process.
    This causes the failure to retrive the license from the response. Below we will check the entire response for '\' characters and
    if found, remove them from the reaponse. The modifed final response is stored in the same location */
    tmpbuf = strstr((char *)*resp, "\\");

    while(tmpbuf != NULL)
    {
        strcpy(new_resp, (const char*)*resp);
        temp = tmpbuf + 1;
        i = tmpbuf - (char*)*resp;
        strncpy((char *)*resp, new_resp, i-1 );
        *resp[i] = '\0';
        strcat((char *)*resp, temp);
        tmpbuf = strstr((char *)*resp, "\\");
    }
    BKNI_Free(new_resp);

    /* Note that in the case of an XML license, the SDK expect the license buffer to be pointing to <LICENSERESPONSE> tag. */

    licresp_opened = strstr((char*)*resp, LICRESPONSE_OPENED);                    /* look for response open tag */
    licresp_closed = strstr((char*)*resp, LICRESPONSE_CLOSED);                    /* look for response close tag */
    /* process response */
    if ((licresp_opened != NULL) && (licresp_closed != NULL)) {
        BDBG_MSG(("-------->LICENSE TAGS FOUND\n"));
        licresp_len = licresp_closed - licresp_opened + sizeof(LICRESPONSE_CLOSED);
        *offset = licresp_opened - (char*)*resp;
        if (out_resp_len != NULL) { *out_resp_len = len; }              /* length of license response */

        if ((len == -1) || (http._responseCode != HTTP_OK_RESPONSE)) {
            BDBG_MSG(("failed on responseCode != %d", HTTP_OK_RESPONSE));
            return -1; //(bdrm_http_status_failed_response_code);
        }
    }
    else
    {
        BDBG_MSG(("-------->LICENSE TAGS NOT FOUND\n"));
        return -1; //(bdrm_http_status_failed_no_license);
    }

    return 0; // (bdrm_http_status_ok);
}

/* drm license post function; feeds a SOAP challenge to a server, parses
   the response looking for the license section, and passes it back to
   the drm engine for processing.
*/
int32_t
DRM_Prdy_http_client_license_post_soap (char* url, char* chall, uint8_t non_quiet,
                               uint32_t app_sec, unsigned char** resp,uint32_t resp_len,
                               uint32_t *offset, uint32_t* out_resp_len
                               )
{
    DRM_Prdy_http_engine http;
    int32_t post_ret;

    int32_t len = 0;
    /*int32_t licresp_len = 0;*/
    /*int32_t resp_len = HTTP_XMLRPC_BUFSIZE;*/
    /*char* licresp_opened = NULL, *licresp_closed = NULL;*/

    BSTD_UNUSED(non_quiet);
    BSTD_UNUSED(app_sec);
    BDBG_ASSERT(resp != NULL); BDBG_ASSERT(out_resp_len != NULL);

    if (*resp == NULL) return -1; //(bdrm_http_status_failed_internal);

    /* append drm specific tokens */
    #if 0
    BDBG_MSG(("\nposting to : %s", url));
    #endif
    #if 0
    len += sprintf(buf, DRM_POST_PREFIX, app_sec, non_quiet);
    #endif
    len += sprintf((char *)*resp + len, "%s", chall);
   // printf("%s - %d get the len of the challenge %d\n",__FUNCTION__,__LINE__,len);

    /* initialize http engine and post */
    DRM_Prdy_http_engine_init(&http);
   // printf("%s - http engine init success. \n",__FUNCTION__);
    if ((post_ret = DRM_Prdy_http_client_post(&http, url)) != 0) {
        BDBG_WRN(("DRM_Prdy_http_license_post failed on POST"));
        return (post_ret);
    }
   // printf("%s - DRM_Prdy_http_client_post success. \n",__FUNCTION__);


    /* set headers, read response */
    DRM_Prdy_http_engine_set_headers(&http, HDR_TAG_SOAP_ACTION, HDR_ACTION_VALUE);
    DRM_Prdy_http_engine_set_headers(&http, HDR_TAG_CONTENT_TYPE, HDR_VALUE_XML_UTF8_CT);
    DRM_Prdy_http_engine_set_headers(&http, HDR_TAG_HOST, HDR_VALUE_HOST_CT);

    DRM_Prdy_http_engine_set_headers(&http, "Pragma", "no-cache");
    DRM_Prdy_http_engine_set_headers(&http, "Accept", "*/*");
    DRM_Prdy_http_engine_set_headers(&http, "Accept-Language", "en-US");
    DRM_Prdy_http_engine_set_headers(&http, "User-Agent", "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)");

    DRM_Prdy_http_engine_set_header(&http,  HDR_TAG_CONTENT_LENGTH, len);
    DRM_Prdy_http_engine_write(&http, (const char *)*resp, len);
    DRM_Prdy_http_engine_flush(&http);
    if (DRM_Prdy_http_client_read_responsehdr(&http)) {
        BDBG_WRN(("failed on readResponseHeader"));
        return -1; //(bdrm_http_status_failed_response_read);
    }
    #if 0
    BDBG_MSG(("HTTP DEBUG :: len <%d>, buf<%s>", len, buf));
    #endif

    /* look for a license */
    bzero(*resp, resp_len); len = DRM_Prdy_http_engine_read(&http, (char *)*resp, resp_len);
    #if 0
    BDBG_MSG(("HTTP :: resp_len <%d>, buf<%s>", resp_len, buf));
    #endif



    *out_resp_len = len;
    *offset = 0;

	DRM_Prdy_http_engine_close(&http);
    DRM_Prdy_http_engine_headers_cleanup(&http);

    return 0; //(bdrm_http_status_ok);
}

/* replace 'from' char with 'to' in array
*/
uint32_t
burl_str_replace(char* str, char to, char from)
{
    uint32_t idx;
    uint32_t count = 0, len = strlen(str);

    for(idx = 0; idx < len; idx++) {
        if (str[idx] == from) {
            idx = to;
            count++;
        }
    }

    return (count);
}

/* get chunk from array, 'from' -> 'to' (TODO: 'to')
*/
char*
burl_str_mid(char* str, uint32_t from)
{
    return (str) + (from);
}

/* find char in array, report position
*/
int32_t
burl_str_find(char* str, char ch)
{
    int32_t idx;
    int32_t count = 0, len = strlen(str);

    for(idx = 0; idx < len; idx++) {
        if (str[idx] == ch) {
            count = idx;
            break;
        }
    }

    return ((count > 0) ? count : -1);
}

/* concat in source with length src_len, str
*/
void
burl_str_strncat(char* source, uint32_t src_len, const char *str, int32_t len)
{
    int32_t _len = src_len;
    char* _s = source;

    if (!str) return;
    if (len == -1) len = strlen(str);

    strncpy(_s+_len, str, len);
    _len += len;
    _s[_len] = 0;
}

/* set str length and pad with 'padchar'
*/
void
burl_str_setlen(char* str, int32_t src_len, int32_t length, char pad)
{
    int32_t idx, l;
    char* temp;
    int32_t _len = src_len;
    char* _s = str;

    if ((length < _len) && (length >= 0)) {
        _len = length; _s[_len] = 0;
    } else if (length > _len) {
        l = length - _len; temp = BKNI_Malloc(l);
        for (idx = 0; idx < l; idx++) temp[idx] = pad;
        burl_str_strncat(str, src_len, temp, l);
        BKNI_Free(temp);
    }
}

/* look up the default port for service/protocol
*/
uint32_t
burl_lookup_port(const char *serv, const char *proto)
{
    struct servent* svt = getservbyname(serv, proto);
    return (svt != (struct servent*)NULL) ? (uint32_t)ntohs(svt->s_port) : 0;
}

/* clean up the members of the url struct
*/
void
burl_clear(bdrm_url* drm_url)
{
    drm_url->_port = 0; drm_url->_search = 0;
    CLRSTR_LEN(drm_url->_protocol, sizeof(drm_url->_protocol));
    CLRSTR_LEN(drm_url->_host, sizeof(drm_url->_host));

    drm_url->_url = NULL; drm_url->_query = NULL;
    CLRSTR_LEN(drm_url->_fragment, sizeof(drm_url->_fragment));
    CLRSTR_LEN(drm_url->_path, sizeof(drm_url->_path));
}

/* get path from url object
*/
const char*
burl_path(bdrm_url* drm_url)
{
    return (!!*drm_url->_path) ? (const char *)(drm_url->_path) : (drm_url->_query);
}

/* get search from url object
*/
const char*
burl_search(bdrm_url* drm_url)
{
    return (drm_url->_search==1) ? (&drm_url->_query[strlen(drm_url->_path) + 1]) : (NULL);
}

/* get fragment from url object
*/
const char*
burl_fragment(bdrm_url* drm_url)
{
    return (drm_url->_search == 2) ? (&drm_url->_query[strlen(drm_url->_path) + 1]) : (NULL);
}

/* get if absolute path from url object
*/
uint32_t
burl_is_absolute(bdrm_url* drm_url)
{
    return ((!!*drm_url->_host) || ((drm_url->_query) && (drm_url->_query[0] == '/')));
}

/* set url from char, parse elements and store in members.
   determine the protocol port number, based on proto prefix.
*/
void
burl_set(bdrm_url* drm_url, char *url)
{
    int32_t colon;
    char *findhost = NULL, *s = NULL;
    char *find = NULL, *search = NULL;

    burl_clear(drm_url); drm_url->_url = url;
    if (drm_url->_url == NULL) return;

    burl_str_replace(drm_url->_url, '\\','/'); s = drm_url->_url;
    if (!strncmp(s, PROTO_FILE_TOKEN, strlen(PROTO_FILE_TOKEN))) {
        strcpy(drm_url->_protocol, "file"); drm_url->_query = s + 5;
        return;
    }
    if ((findhost = strstr(s, "//")) != NULL) {
        if ((findhost > s) && (*(findhost-1) == ':')) {
            strncpy(drm_url->_protocol, s, findhost-s-1);
        }
        findhost += 2;
        if ((find = strchr(findhost, '/')) != NULL) {
            strncpy(drm_url->_host, findhost, find-findhost);
            drm_url->_query = find;
        } else { strcpy(drm_url->_host, findhost); drm_url->_query = NULL; }

        if ((colon = burl_str_find(drm_url->_host, ':')) != -1) {
            drm_url->_port = atoi(burl_str_mid(drm_url->_host, colon + 1));
            burl_str_setlen(drm_url->_host, strlen(drm_url->_host), colon, 0);
#ifdef ANDROID
        } else {
            drm_url->_port = burl_lookup_port(drm_url->_protocol, "tcp");
            // Yuichi: getservbyname still doesn't work on Android
            // TODO: need better solution
            if (drm_url->_port == 0 && strcmp(drm_url->_protocol, "http") == 0) {
                drm_url->_port = 80;
            }
        }
#else
        } else drm_url->_port = burl_lookup_port(drm_url->_protocol, "tcp");
#endif
    }
    else { drm_url->_port = 0; drm_url->_query = s;
    }
    if (drm_url->_query) {
        if ((search = strchr(drm_url->_query, '?')) != NULL) {
            strncpy(drm_url->_path, drm_url->_query, search - drm_url->_query);
            drm_url->_search = 1;
        } else if ((search = strchr(drm_url->_query, '#')) != NULL) {
            strncpy(drm_url->_path, drm_url->_query, search - drm_url->_query);
            drm_url->_search = 2;
        } else { CLRSTR(drm_url->_path); drm_url->_search = 0; }
    } else { CLRSTR(drm_url->_path); drm_url->_search = 0; }
}

/* translate from url to http encoding
*/
void
burl_to_encoding(char* drm_url, const char *str)
{
    uint32_t n;
    char buf[4];
    char* result = drm_url;

    while (*str) {
        if ((n = strcspn(str, PROTO_HTTP_UNSAFE))) {
            burl_str_strncat(result, strlen(result), str, n);
            str += n; if (!(!!*str)) break;
        }
        sprintf(buf, "%%%02x", str[0]);
        burl_str_strncat(result, strlen(result), buf, -1);
        str++;
    }

    return;
}

/* translate url from http encoded array
*/
void
burl_from_encoding(char* drm_url, const char *str)
{
    uint32_t ch, _len = 0;
    const char *next;
    char buf[3];

    buf[2] = 0;
    while ((next = strchr(str, '%')) && next[1] && next[2]) {
        burl_str_strncat(drm_url, _len, str, next-str); next++;     /* skip % */
        memcpy(buf, next, 2); sscanf(buf, "%x", &ch);
        _len = strlen(drm_url); (*(drm_url+_len)) = (char)ch;
        _len++; (*(drm_url+_len)) = (char)0; str = next + 2;        /* skip 2 digits */
    }
    burl_str_strncat(drm_url, strlen(drm_url), str, -1);

    return;
}

/* print members of a url object
*/
void
burl_print(char *s)
{
    bdrm_url drm_url;

    burl_set(&drm_url, s);
    #if 0
    BDBG_MSG(("\nURL=%s", s));

    BDBG_MSG(("%s,%s,%d,qry=%s,path=%s\n  search=%s, frag=%s",
        drm_url._protocol, drm_url._host, drm_url._port, drm_url._query,
        burl_path(&drm_url), burl_search(&drm_url),
        burl_fragment(&drm_url)));
    #endif
}
