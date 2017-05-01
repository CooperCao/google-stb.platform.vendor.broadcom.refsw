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

/*APIs in this file are no longer to be used for REFSW implementation anymore. These APIs can be romoved at anytime.
  Please use at your own risk. This file is kept for previous implemetation refernce and as an example code. */

#include <sys/types.h>                  /* standard includes */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#include "bstd.h"                       /* brcm includes */
#include "bdbg.h"
#include "bkni.h"

#include "http.h"                    /* local function decls */

#if defined(__unix__)
    #include <netinet/in.h>
    #include <arpa/inet.h>              /* inet_pton */
    #include <netdb.h>                  /* gethostbyname */
    #include <unistd.h>
    #include <sys/socket.h>             /* socket/bind/etc. */
#endif

BDBG_MODULE(bhttp);                 /* http debug handle */

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

#define DRM_POST_PREFIX         "appsecurity=%d&nonsilent=%d&challenge="    /* drm specific post prefix, pre-challenge */
#define PROTO_FILE_TOKEN        "file:"                 /* protocol token present in file:// */
#define PROTO_HTTP_UNSAFE       ";@&<>\"#%{}|\\^~[]` "  /* invalid chars in http address */
#define LICRESPONSE_OPENED      "<LICENSERESPONSE>"     /* open xml tag in license response */
#define LICRESPONSE_CLOSED      "</LICENSERESPONSE>"    /* closed xml tag in license response */
#define HDR_TAG_CONTENT_LENGTH  "Content-Length"        /* header tag for content-length */
#define HDR_TAG_CONTENT_TYPE    "Content-Type"          /* header tag for content-type */
#define HDR_TAG_SOAP_ACTION     "SOAPAction"
#define HDR_ACTION_VALUE        "\"http://schemas.microsoft.com/DRM/2007/03/protocols/AcquireLicense\""

/* zero-pad string macros
*/
#define CLRSTR_LEN(s, len)  bzero((s), (len));
#define CLRSTR(str)         CLRSTR_LEN((str), (strlen((str))));

/* add a new header name/value pair to the http context
*/
void
bhttpengine_set_headers(bhttp_engine* http, const char* name, char* value)
{
    uint32_t name_len, value_len;
    bhttp_hdr_indiv* http_hdr;

    http_hdr = (bhttp_hdr_indiv*)BKNI_Malloc(sizeof(bhttp_hdr_indiv));
    name_len = strlen(name);
    http_hdr->hdr_name = name;

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
bhttpengine_init(bhttp_engine* http)
{
    BLST_S_INIT(&http->_headers);
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
bhttpengine_uninit(bhttp_engine* http)
{
    BSTD_UNUSED(http);
#ifdef _WINSOCKAPI_
    WSACleanup();
#endif
}

/* close/shutdown opened descriptors for the current instance
*/
void
bhttpengine_close(bhttp_engine* http)
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
bhttpengine_is_connected(bhttp_engine* http)
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
bhttpengine_set_header(bhttp_engine* http, const char* name, uint32_t value)
{
    char strval[MAX_HDR_INT_NUM];
    sprintf(strval, "%d", value);
    bhttpengine_set_headers(http, name, strval);
}

/* return a value based on the header name, from the headers collection
*/
const char*
bhttpengine_get_header(bhttp_engine* http, const char* name)
{
    bhttp_hdr_indiv* http_hdr = NULL;
    const char* value = NULL;

    for (http_hdr = BLST_S_FIRST(&http->_headers); http_hdr;
        http_hdr = BLST_S_NEXT(http_hdr, link)) {
            if (strcasecmp(http_hdr->hdr_name, name) == 0) {
                value = http_hdr->hdr_value;
                break;
        }
    }

    return (value);
}

/* truncates white-space chars off end of 'str'
*/
static void
bhttpengine_truncsp(char* str)
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
bhttpengine_strskip(const char* str, const char* chars)
{
    return (char*)str + strspn(str, chars);
}

/* read one line at a time from the read end of the connection
*/
char*
bhttpengine_read_line(bhttp_engine* http, char* buf, uint32_t len)
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

    bhttpengine_truncsp(s);
    if ((http->_printHttp) && (s))
        BDBG_MSG(("RX: %s", s));

    return (s);
}

/* clean up the list of headers and free allocated ones
*/
int
bhttpengine_headers_cleanup(bhttp_engine* http)
{
    bhttp_hdr_indiv* http_hdr, *lnk;
    int32_t ret = 0, count = 0;

    for(http_hdr = BLST_S_FIRST(&http->_headers); http_hdr;
        http_hdr = lnk, count++) {
            BDBG_MSG(("cleaning up stream (%d)", count));
            BKNI_Free(http_hdr->hdr_value);
            lnk = BLST_S_NEXT(http_hdr, link); BKNI_Free(http_hdr);
        }

    return (ret);
}

/* clean up and refill the list of headers
*/
uint32_t
bhttpengine_read_headers(bhttp_engine* http)
{
    uint32_t name_len, value_len;
    char* line, *colon, *value;
    char buf[HTTP_MAX_HDR_LINE];
    bhttp_hdr_indiv* http_hdr;

    bhttpengine_headers_cleanup(http);
    while ((line = bhttpengine_read_line(http,buf, HTTP_MAX_HDR_LINE))) { /* now read the headers */
        if (!*line) break;                                                /* we've finished reading the header */
        if ((colon = strchr(line, ':')) != NULL) {
            value = bhttpengine_strskip(colon + 1, " \t");
            http_hdr = (bhttp_hdr_indiv*)BKNI_Malloc(sizeof(bhttp_hdr_indiv));

            name_len = colon - line;
            *colon = '\0'; http_hdr->hdr_name = line;
            value_len = strlen(value);
            http_hdr->hdr_value = (char*)BKNI_Malloc(value_len + 1);

            strcpy(http_hdr->hdr_value, value);
            http_hdr->hdr_name_len = name_len; http_hdr->hdr_value_len = value_len;
            BLST_S_INSERT_HEAD(&http->_headers, http_hdr, link);
            BKNI_Free(http_hdr->hdr_value);
            BKNI_Free(http_hdr);
        }
    }
    if (!line) return (-1);

    return (0);
}

/* generic read function, wrapper on top of the IOs
*/
uint32_t
bhttpengine_read(bhttp_engine* http, char* buf, uint32_t len)
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

        BDBG_MSG(("bhttpengine_read len %d rd_len %d 0x%x",len, rd_len, http->_currentFile));
	}

    if (http->_printHttp)
        BDBG_MSG(("RX: %.*s", rd_len, buf));

    return (rd_len);
}

/* internal function used for write ops
*/
int32_t
bhttpengine_internal_write(bhttp_engine* http, const char* buf, int32_t len)
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
bhttpengine_write_headers(bhttp_engine* http)
{
    bhttp_hdr_indiv* http_hdr;

    http->_wroteHeaders = 1;
    for (http_hdr = BLST_S_FIRST(&http->_headers);
         http_hdr; http_hdr = BLST_S_NEXT(http_hdr, link)) {
        if (bhttpengine_internal_write(http, http_hdr->hdr_name, -1) == -1 ||
            bhttpengine_internal_write(http, ": ", -1) == -1 ||
            bhttpengine_internal_write(http, http_hdr->hdr_value, -1) == -1 ||
            bhttpengine_internal_write(http, "\r\n", -1) == -1) {
                return (-1);
        }
    }
    if (bhttpengine_internal_write(http, "\r\n", -1) == -1) {
        return (-1);
    }

    return (0);
}

/* wrapper on top of the write functions for headers and data
*/
int32_t
bhttpengine_write(bhttp_engine* http, const char* buf, int32_t len)
{
    if (!http->_wroteHeaders)
        if (bhttpengine_write_headers(http) == -1) {
            BDBG_MSG(("Couldn't write headers, errno %d", errno));
            return (-1);
        }
    if (buf)
        return bhttpengine_internal_write(http, buf, len);

    return(-1);
}

/* format and output one single line of clrf terminated data
*/
uint32_t
bhttpengine_writeline(bhttp_engine* http, const char* buf)
{
    bhttpengine_write(http, buf, -1);
    return (bhttpengine_write(http, "\r\n", -1));
}

/* get buffer containing response body, accounting for content length
*/
int32_t
bhttpengine_read_body(bhttp_engine* http, char* buf, int32_t len)
{
    int32_t cl;
    const char* clstr;

    clstr = bhttpengine_get_header(http, HDR_TAG_CONTENT_LENGTH);
    cl = (clstr) ? atoi(clstr) : (len);
    if (cl < len) len = cl;

    return bhttpengine_read(http, buf, len);
}

/* write headers and data to a file
*/
uint32_t
bhttpengine_write_file(bhttp_engine* http, const char* filename)
{
    uint32_t r;
    char buf[HTTP_MAX_FILE_BUFF];
    FILE* f = NULL;

    if ((f = fopen(filename, "r")) != NULL) {
        while (!feof(f)) {
            if (!(r = fread(buf, 1, HTTP_MAX_FILE_BUFF, f))) break;
            if (bhttpengine_write(http, buf, r) == -1) break;
        }
        fclose(f);
        return (0);
    }

    return (-1);
}

/* read http engine data from a file
*/
uint32_t
bhttpengine_read_file(bhttp_engine* http, const char* filename)
{
    uint32_t n;
    char buf[HTTP_MAX_FILE_BUFF];
    FILE* f = NULL;

    if ((f = fopen(filename, "w+")) != NULL) {
        do {n = bhttpengine_read(http, buf, sizeof(buf));
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
bhttpengine_print_headers(bhttp_engine* http)
{
    bhttp_hdr_indiv* http_hdr;

    for (http_hdr = BLST_S_FIRST(&http->_headers);
         http_hdr; http_hdr = BLST_S_NEXT(http_hdr, link)) {
            BDBG_MSG(("%s: %s", http_hdr->hdr_name, http_hdr->hdr_value));
    }
}

/* flush outputs for current file descriptor
*/
void
bhttpengine_flush(bhttp_engine* http)
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
bhttpclient_connect(bhttp_engine* http, const char *host, uint32_t port)
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
        bhttpengine_close(http);
        return (-1);
    }
#endif
    http->_currentFile = fdopen(http->_currentFd, "r+");
    if (!http->_currentFile) {
        bhttpengine_close(http);
        return (-1);
    }

    return (0);
}

/* client function to send a request. specify method to be either 'get',
   'post' or 'put', and urlstr to specify the url
*/
int32_t
bhttpclient_send_request(bhttp_engine* http, const char *method, char *urlstr)
{
    bdrm_url url;
    const char *query;
    char buf[HTTP_MAX_HDR_LINE];
    uint32_t len = 0;
    uint8_t keepalive = HTTP_KEEP_ALIVE_OFF;

    burl_set(&url, urlstr);
    if (http->_currentFd == -1) {
        if (!url._host || bhttpclient_connect(http, url._host, url._port) == -1)
            return (bdrm_http_status_failed_connect);
    } else keepalive = HTTP_KEEP_ALIVE_ON;

    query = url._query; if (!query || !*query) { query = "/"; }
    len = snprintf(buf, HTTP_MAX_HDR_LINE, "%s %s HTTP/1.0\r\n", method, query);
    if (bhttpengine_internal_write(http, buf, len) == -1)
        return (bdrm_http_status_failed_getpost);

    bhttpengine_headers_cleanup(http);
    if (HTTP_KEEP_ALIVE_ON == keepalive)
        bhttpengine_set_headers(http, "Connection", (char *)"Keep-Alive");

    http->_wroteHeaders = 0; http->_readHeaders = 0;

    return (bdrm_http_status_ok);
}

/* handling function for the headers coming inbound from the server
*/
int32_t
bhttpclient_read_responsehdr(bhttp_engine* http)
{
    char *line, *find;
    char buf[HTTP_MAX_HDR_LINE];
    char response[MAX_RESPONSE_NUM];

    BKNI_Memset( buf, 0, ( sizeof( char ) * HTTP_MAX_HDR_LINE ));
    BKNI_Memset( response, 0, ( sizeof( char ) * MAX_RESPONSE_NUM ));

    if (http->_readHeaders) {
        BDBG_MSG(("Cannot read headers twice."));
        return (-1);
    }

    http->_readHeaders = 1;     /* read the top line */
    line = bhttpengine_read_line(http, buf, HTTP_MAX_HDR_LINE);
    if (!line) return (-1);

    find = strchr(line, ' ');  if (!find) return (-1);
    line = find + 1; find = strchr(line, ' ');  if (!find) return (-1);
    if((find - line) < (int32_t)sizeof(response)) {
        strncpy(response, line, find - line);
        http->_responseCode = atoi(response);;
    }
    return bhttpengine_read_headers(http);
}

/* 'http get' wrapper function
*/
int32_t
bhttpclient_get(bhttp_engine* http, char *urlstr)
{
    int32_t res;

    if ((res = bhttpclient_send_request(http, "GET", urlstr)) != 0) return (res);
    if ((res = bhttpengine_write_headers(http)) != 0) return (res);

    return bhttpclient_read_responsehdr(http);
}

/* 'http post' wrapper function
*/
int32_t
bhttpclient_post(bhttp_engine* http, char *url)
{
    return bhttpclient_send_request(http, "POST", url);
}

/* 'http put' wrapper function
*/
int32_t
bhttpclient_put(bhttp_engine* http, char *url)
{
    return bhttpclient_send_request(http, "PUT", url);
}

/* drm license post function; feeds a SOAP challenge to a server, parses
   the response looking for the license section, and passes it back to
   the drm engine for processing.
*/
int32_t
bhttpclient_license_post_soap (
    char* url,
    char* chall,
    unsigned char** resp,
    uint32_t* out_resp_len
)
{
    bhttp_engine http;
    int32_t post_ret;

    int32_t len = 0;
    int32_t resp_len = HTTP_XMLRPC_BUFSIZE;
    char *internal_resp;
    internal_resp = (char *)BKNI_Malloc(resp_len);

    if (*resp == NULL) return (bdrm_http_status_failed_internal);
    /* append drm specific tokens */

    len += sprintf(internal_resp + len, "%s", chall);

    /* initialize http engine and post */
    bhttpengine_init(&http);
    if ((post_ret = bhttpclient_post(&http, url)) != 0) {
        BDBG_WRN(("bhttp_license_post failed on POST"));
        return (post_ret);
    }

    /* set headers, read response */
    bhttpengine_set_headers(&http, HDR_TAG_SOAP_ACTION, (char *) HDR_ACTION_VALUE);
    bhttpengine_set_headers(&http, HDR_TAG_CONTENT_TYPE, (char *)HDR_VALUE_XML_UTF8_CT);
    bhttpengine_set_header(&http,  HDR_TAG_CONTENT_LENGTH, len);
    bhttpengine_write(&http, (const char *)internal_resp, len); bhttpengine_flush(&http);
    if (bhttpclient_read_responsehdr(&http)) {
        BDBG_WRN(("failed on readResponseHeader"));
        return (bdrm_http_status_failed_response_read);
    }

    /* look for a license */
    bzero(internal_resp, resp_len);

    len = bhttpengine_read(&http, internal_resp, resp_len);

    *resp =  (unsigned char *)internal_resp;

    *out_resp_len = len;
    return (bdrm_http_status_ok);
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
        l = length - _len; temp = (char *)BKNI_Malloc(l);
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
    return (drm_url->_path) ? (const char *)(drm_url->_path) : (drm_url->_query);
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
    return ((drm_url->_host) || ((drm_url->_query) && (drm_url->_query[0] == '/')));
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
        } else drm_url->_port = burl_lookup_port(drm_url->_protocol, "tcp");
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
            str += n; if (!*str) break;
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
