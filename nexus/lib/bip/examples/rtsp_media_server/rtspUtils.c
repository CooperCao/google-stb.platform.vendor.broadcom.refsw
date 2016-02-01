/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
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
 * Module Description: Test application to send rtsp client requests and validate responses from RTSP Media server
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *************************************************************/

#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>

#include <assert.h>
#include <stdbool.h>

#include "TestUtils.h"
#include "IPTestUtils.h"
#include "rtspUtils.h"

#ifdef HAS_NEXUS /* TBD: Need to find the appropriate define */
#include "bstd.h"
#include "bkni.h"
#include "nexus_platform.h"
    #define	MALLOC	BKNI_Malloc
    #define FREE 	BKNI_Free
#else
    #define	MALLOC 	malloc
    #define FREE	free
#endif


int server_ip = 0;
int sock_desc = 0;
int seq_no = 1;

int verbose = 0;

/* Socket receive timeout - 10 secs */
struct timeval tv = {10, 0};

#define SET_SOCKET_TIMEOUT(socket_desc)	setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv))

int setServerIp(char *server /* In : Server IP address in number-and-dots format*/
                )
{
    int ret = NO_ERROR;
    ret = inet_addr(server);
    if (ret != (int)INADDR_NONE) {
        server_ip = ret;
    } else {
        ret = ERROR;
    }
    return ret;
}

int setSockDesc(int sd /* In: Socket Descriptor */
                )
{
    sock_desc = sd;
    return NO_ERROR;
}

int parseRtspHeader (char *header,          /* In: Pointer to RTSP header */
                     int *ret_code,         /* Out: Return code */
                     char *ret_code_msg,    /* Out: Return code message */
                     int *cseq,             /* Out: sequence number */
                     char *session,         /* Out: Session identifier */
                     char *rtp_info         /* Out: RTP information - containing the url - can be null */
                     )
{
    int ret = NO_ERROR;
    char *outer, *inner;
    char *tok1, *tok2, *str;

    /* Initialize return values */
    ret_code_msg[0] = '\0';
    session[0] = '\0';
    if (rtp_info)
        rtp_info[0] = '\0';
    *cseq = 0;
    *ret_code = 0;

    if (header != NULL)
    {
        tok1 = strtok_r(header, "\n", &outer);
        while (tok1 != NULL)
        {
            tok2 = strtok_r(tok1, " ", &inner);
            while (tok2 != NULL)
            {
                if (strcmp (tok2, "RTSP/1.0") == 0)
                {
                    /* Get return code */
                    str = strtok_r(NULL, " ", &inner);
                    *ret_code = strtol(str, NULL, 10);

                    /* get return code message */
                    str = strtok_r(NULL, "\n", &inner);
                    strncpy(ret_code_msg, str, 128);
                }
                else if (strcmp(tok2, "CSeq:") == 0)
                {
                    /* Get sequence number */
                    str = strtok_r(NULL, " ", &inner);
                    *cseq = strtol(str, NULL, 10);
                }
                else if (strcmp(tok2, "Session:") == 0)
                {
                    /* Get session identifier */
                    str = strtok_r(NULL, ";", &inner);
                    strncpy(session, str, 128);
                }
                else if (strncmp(tok2, "RTP-Info:url=", 13) == 0)
                {
                    if (rtp_info)
                        strncpy(rtp_info, &tok2[13], 128);
                }
                else if (strncmp(tok2, "RTP-Info:", 8) == 0)
                {
                    str = strtok_r(NULL, " ", &inner);
                    if (strncmp (str, "url=", 4) == 0)
                    {
                        if (rtp_info)
                            strncpy(rtp_info, &str[4], 128);
                    }
                }

                tok2 = strtok_r(NULL, " ", &inner);
            }

            tok1 = strtok_r(NULL, "\n", &outer);
        }
    }
    else
    {
        ret = PARSE_ERROR;
    }

    /*printf ("Return Code = %d\n", *ret_code);
    printf ("Return Code Message = %s\n", ret_code_msg);
    printf ("Sequence number = %d\n", *cseq);
    printf ("Session = %s\n", session);
    printf ("RTP Info = %s\n", rtp_info);*/

    return ret;
}

int describe(char *session_id,            /* In: Pointer to session id - Can be set to NULL if not in session */
             RTSP_StreamInfo *strm_info,  /* Out: Stream information from the RTSP server */
             int *max_entries             /* In/Out: The size of the array strm_info is passed
                                           * and the number of entreis populated is returned */
             )
{
    int ret = 200;
    char *buf;
    char *recv_buf;
    char *tmp_buf;
    char ret_msg[128], ret_session_id[128], rtp_info[128];
    int ret_cseq;

    buf = (char *)MALLOC(RTSP_HEADER_SIZE+1);
    recv_buf = (char *)MALLOC(RTSP_RESPONSE_SIZE);
    tmp_buf = (char *)MALLOC(RTSP_RESPONSE_SIZE);

    /* construct the describe command header */
    if (session_id)
    {
        snprintf (buf, RTSP_HEADER_SIZE,
                       "DESCRIBE rtsp://%s/ RTSP/1.0\r\n"
                       "CSeq: %d\r\n"
                       "Session: %s\r\n"
                       "Accept: application/sdp\r\n"
                       RTSP_REQUEST_HEADER_CRLF, inet_ntoa(*(struct in_addr *)&server_ip), seq_no, session_id);
    }
    else
    {
        snprintf (buf, RTSP_HEADER_SIZE,
                       "DESCRIBE rtsp://%s/ RTSP/1.0\r\n"
                       "CSeq: %d\r\n"
                       "Accept: application/sdp\r\n"
                       RTSP_REQUEST_HEADER_CRLF, inet_ntoa(*(struct in_addr *)&server_ip), seq_no);
    }

    if (SET_SOCKET_TIMEOUT(sock_desc) != 0)
		goto error;

	if (NO_ERROR != (ret = tcpPayloadSendReceive(sock_desc, buf, recv_buf, RTSP_RESPONSE_SIZE, "RTSP DESCRIBE")))
        goto error;

    PRINT_REQ_RESP(buf, recv_buf);

    /* parse common headers */
    strncpy(tmp_buf, recv_buf, RTSP_RESPONSE_SIZE);
    parseRtspHeader (tmp_buf, &ret, ret_msg, &ret_cseq, ret_session_id, rtp_info);

    SEQUENCE_NUM_CHECK(ret_cseq, seq_no);

    /* parse response headers */
    strncpy(tmp_buf, recv_buf, RTSP_RESPONSE_SIZE);
    parseDescribeResponse(tmp_buf, strm_info, max_entries);
    printRtspStreamInfo(strm_info, *max_entries);

    /* Increment sequence number for next command */
    seq_no++;

error:
    FREE(buf);
    FREE(recv_buf);
    FREE(tmp_buf);
    return ret;
}

void printRtspStreamInfo(RTSP_StreamInfo *strm_info,  /* In: Stream info array */
                       int size                     /* In: Size of the array */
                       )
{
    int i;
    if (size)
    {
        printf ("Found %d streams\n", size);
        for (i = 0; i < size; i++)
        {
            printf ("Stream Info [%d]\n", i);
            printf ("------------------\n");
            printf ("Stream Id          : %d\n", strm_info[i].stream_id);
            if (strm_info[i].mcast_addr != 0)
            {
                printf ("Multicast address  : %s\n", inet_ntoa(*(struct in_addr*)(&strm_info[i].mcast_addr)));
                printf ("Multicast port     : %d\n", strm_info[i].port);
            }
            else
            {
                printf ("Multicast/Unicast  : Unicast\n");
            }
            printf ("Media Type         : %s\n", strm_info[i].media_type);
            printf ("Media Protocol     : %s\n", strm_info[i].media_proto);
            printf ("Media Desc Format  : %s\n", strm_info[i].media_format);
            printf ("Tuner Parameters   : %s\n", strm_info[i].tuner_params);
            printf ("Active             : %s\n", strm_info[i].active ? "yes" : "no");
            printf ("------------------\n");
        }
    }
}

int parseDescribeResponse(char *response,               /* In: Pointer to RTSP response */
                          RTSP_StreamInfo *strm_info,   /* Out: Stream information from the RTSP server */
                          int *size                     /* In/Out: The size of the array strm_info is passed
                                                         * and the number of entreis populated is returned */
                          )
{
    int ret = NO_ERROR;
    int max = *size, count = 0;
    char *outer, *inner;
    char *tok1, *tok2;

    *size = 0;

    if (response != NULL)
    {
        tok1 = strtok_r(response, "\n", &outer);
        while (tok1 != NULL)
        {
            if (strncmp(tok1, "m=", 2) == 0)
            {
                if (max == count)
                    break;

                count++;
                /* Init params */
                strm_info[count-1].active = true;
                strm_info[count-1].port = 0;

                /* extract media info */
                tok2 = strtok_r(&tok1[2], " ", &inner);
                strncpy(strm_info[count-1].media_type, tok2, 31);
                tok2 = strtok_r(NULL, " ", &inner);
                strm_info[count-1].port = strtol(tok2, NULL, 10);
                tok2 = strtok_r(NULL, " ", &inner);
                strncpy(strm_info[count-1].media_proto, tok2, 31);
                tok2 = strtok_r(NULL, " ", &inner);
                strncpy(strm_info[count-1].media_format, tok2, 31);
				strm_info[count-1].media_type[31] = '\0';
				strm_info[count-1].media_proto[31] = '\0';
				strm_info[count-1].media_format[31] = '\0';
            }
            else if (strncmp(tok1, "c=", 2) == 0)
            {
                /* Discard connection params IN and IP4 */
                strtok_r(&tok1[2], " ", &inner);
                strtok_r(NULL, " ", &inner);

                tok2 = strtok_r(NULL, " ", &inner);
                if (strncmp(tok2, "0.0.0.0", 7) == 0)
                {
                    strm_info[count-1].mcast_addr = 0;
                }
                else
                {
                    tok2 = strtok_r(tok2, "/", &inner);
                    strm_info[count-1].mcast_addr = inet_addr(tok2);
                }
            }
            else if (strncmp(tok1, "a=fmtp:", 7) == 0)
            {
                /* Discard the format id */
                strtok_r(&tok1[7], " ", &inner);
                tok2 = strtok_r(NULL, " ", &inner);
                strncpy(strm_info[count-1].tuner_params, tok2, 255);
				strm_info[count-1].tuner_params[255] = '\0';
            }
            else if (strncmp(tok1, "a=control:stream=", 17) == 0)
            {
                strm_info[count-1].stream_id = strtol(&tok1[17], NULL, 10);
            }
            else if (strcmp(tok1, "a=sendonly") == 0)
            {
                strm_info[count-1].active = true;
            }
            else if (strcmp(tok1, "a=inactive") == 0)
            {
                strm_info[count-1].active = false;
            }

            tok1 = strtok_r(NULL, "\n", &outer);
        }
    }
    else
    {
        ret = PARSE_ERROR;
    }

    *size = count;
    return ret;
}

int setup(int *ports,                   /* In: client ports in case of unicast and dest ports in case of multicast */
          bool b_mcast,                 /* In: Multicast setup request? */
          char *url,                    /* In: RTSP url with tuning parameters etc ... for the SAT>IP server */
          RTSP_SessionInfo *session_info/* Out: Session setup information */
          )
{
    int ret = 200;
    char *buf;
    char *recv_buf;
    char *tmp_buf;
    char ret_msg[128], ret_session_id[128], rtp_info[128];
    int ret_cseq;

    buf = (char *)MALLOC(RTSP_HEADER_SIZE+1);
    recv_buf = (char *)MALLOC(RTSP_RESPONSE_SIZE);
    tmp_buf = (char *)MALLOC(RTSP_RESPONSE_SIZE);

    memset(session_info, 0, sizeof(RTSP_SessionInfo));

    snprintf (buf, RTSP_HEADER_SIZE,
                   "SETUP %s RTSP/1.0\r\n"
                   "CSeq: %d\r\n"
                   "Transport: RTP/AVP;%s=%d-%d\r\n"
                   RTSP_REQUEST_HEADER_CRLF, url, seq_no, b_mcast ? "multicast;port" : "unicast;client_port", ports[0], ports[1]);

    if (SET_SOCKET_TIMEOUT(sock_desc) != 0)
		goto error;

    if (NO_ERROR != (ret = tcpPayloadSendReceive(sock_desc, buf, recv_buf, RTSP_RESPONSE_SIZE, "RTSP SETUP")))
        goto error;

    PRINT_REQ_RESP(buf, recv_buf);

    /* parse common headers */
    strncpy(tmp_buf, recv_buf, RTSP_RESPONSE_SIZE);
    parseRtspHeader (tmp_buf, &ret, ret_msg, &ret_cseq, ret_session_id, rtp_info);

    SEQUENCE_NUM_CHECK(ret_cseq, seq_no);

    /* Initialize client ports */
    session_info->client_port[0] = ports[0];
    session_info->client_port[1] = ports[1];

    /* Parse setup response header */
    strncpy(tmp_buf, recv_buf, RTSP_RESPONSE_SIZE);
    parseSetupResponse(tmp_buf, session_info);

    /* Increment sequence number for next command */
    seq_no++;

error:
    FREE(buf);
    FREE(recv_buf);
    FREE(tmp_buf);
    return ret;
}

void printSessionInfo(RTSP_SessionInfo *session_info   /* In: Session information from response to SETUP command */
                      )
{
    printf ("Session Info\n");
    printf ("------------------\n");
    printf ("Stream Id          : %d\n", session_info->stream_id);
    printf ("Session Id         : %s\n", session_info->session_id);
    printf ("Multicast/Unicast  : %s\n", session_info->b_mcast ? "Multicast" : "Unicast");
    if (session_info->b_mcast)
    {
        printf ("Multicast Address  : %s\n", inet_ntoa(*(struct in_addr*)(&session_info->dest_addr)));
    }
    else
    {
        printf ("Server IP Address  : %s\n", inet_ntoa(*(struct in_addr*)(&session_info->src_addr)));
        printf ("Server Ports       : %d-%d\n", session_info->port[0], session_info->port[1]);
    }
    printf ("Client Ports       : %d-%d\n", session_info->client_port[0], session_info->client_port[1]);
    printf ("------------------\n");
}

int parseSetupResponse(char *response,                  /* In: Pointer to RTSP response */
                       RTSP_SessionInfo *session_info   /* Out: Session information from response to SETUP command */
                       )
{
    int ret = NO_ERROR;
    char *outer, *inner, *inner2;
    char *tok1, *tok2, *tok3;
    char *tmp_buf;
    char ret_msg[128], rtp_info[128];
    int ret_cseq;

    tmp_buf = (char *)MALLOC(RTSP_RESPONSE_SIZE);

    session_info->dest_addr = 0;
    if (response != NULL)
    {
        strncpy(tmp_buf, response, RTSP_RESPONSE_SIZE);
        parseRtspHeader(tmp_buf, &ret, ret_msg, &ret_cseq, session_info->session_id, rtp_info);

        tok1 = strtok_r(response, "\n", &outer);
        while (tok1 != NULL)
        {
            if (strncmp(tok1, "Transport:", 10) == 0)
            {
                /* Extract transport parameters */
                tok2 = strtok_r(&tok1[10], ";", &inner);
                while (tok2 != NULL)
                {
                    if (strncmp(tok2, "unicast", 7) == 0)
                    {
                        session_info->b_mcast = false;
                    }
                    else if (strncmp(tok2, "multicast", 9) == 0)
                    {
                        session_info->b_mcast = true;
                    }
                    else if (strncmp(tok2, "client_port=", 12) == 0)
                    {
                        /* Get client ports */
                        tok3 = strtok_r(&tok2[12], "-", &inner2);
                        session_info->client_port[0] = strtol(tok3, NULL, 10);
                        tok3 = strtok_r(NULL, "-", &inner2);
                        session_info->client_port[1] = strtol(tok3, NULL, 10);
                    }
                    else if (strncmp(tok2, "server_port=", 12) == 0)
                    {
                        /* Get server ports */
                        tok3 = strtok_r(&tok2[12], "-", &inner2);
                        session_info->port[0] = strtol(tok3, NULL, 10);
                        tok3 = strtok_r(NULL, "-", &inner2);
                        session_info->port[1] = strtol(tok3, NULL, 10);
                    }
                    else if (strncmp(tok2, "port=", 5) == 0)
                    {
                        /* Get multicast destination ports */
                        tok3 = strtok_r(&tok2[5], "-", &inner2);
                        session_info->client_port[0] = strtol(tok3, NULL, 10);
                        tok3 = strtok_r(NULL, "-", &inner2);
                        session_info->client_port[1] = strtol(tok3, NULL, 10);
                    }
                    else if (strncmp(tok2, "destination=", 12) == 0)
                    {
                        /* Get multicast destination address */
                        session_info->dest_addr = inet_addr(&tok2[12]);
                    }
                    else if (strncmp(tok2, "source=", 7) == 0)
                    {
                        /* Get multicast destination address */
                        session_info->src_addr = inet_addr(&tok2[7]);
                    }

                    tok2 = strtok_r(NULL, ";", &inner);
                }
            }
            else if (strncmp(tok1, "com.ses.streamID:", 17) == 0)
            {
                /* Get the stream identifier */
                session_info->stream_id = strtol(&tok1[17], NULL, 10);
            }
            tok1 = strtok_r(NULL, "\n", &outer);
        }
    }
    else
    {
        ret = PARSE_ERROR;
    }

    FREE(tmp_buf);
    return ret;
}

int play(char *session_id,  /* In: Session id for RTSP session */
         char *url,         /* In: Rtsp URL for stream. Can be NULL */
         int strm_id,       /* In: If url is NULL then strm_id will be used to generate the rtsp url */
         char *rtp_info     /* Out: Rtp Info url - can be null */
         )
{
    int ret = 200;
    char *buf;
    char *recv_buf;
    char *tmp_buf;
    char ret_msg[128], ret_session_id[128];
    int ret_cseq;

    buf = (char *)MALLOC(RTSP_HEADER_SIZE+1);
    recv_buf =(char *)MALLOC(RTSP_RESPONSE_SIZE);
    tmp_buf = (char *)MALLOC(RTSP_RESPONSE_SIZE);

    if (url)
    {
        snprintf (buf, RTSP_HEADER_SIZE,
                       "PLAY %s RTSP/1.0\r\n"
                       "CSeq: %d\r\n"
                       "Session: %s\r\n"
                       RTSP_REQUEST_HEADER_CRLF, url, seq_no, session_id);
    }
    else
    {
        snprintf (buf, RTSP_HEADER_SIZE,
                       "PLAY rtsp://%s:554/stream=%d RTSP/1.0\r\n"
                       "CSeq: %d\r\n"
                       "Session: %s\r\n"
                       RTSP_REQUEST_HEADER_CRLF, inet_ntoa(*(struct in_addr *)&server_ip), strm_id, seq_no, session_id);
    }

    if (SET_SOCKET_TIMEOUT(sock_desc) != 0)
		goto error;

    if (NO_ERROR != (ret = tcpPayloadSendReceive(sock_desc, buf, recv_buf, RTSP_RESPONSE_SIZE, "RTSP PLAY")))
        goto error;

    PRINT_REQ_RESP(buf, recv_buf);

    /* parse common headers */
    strncpy(tmp_buf, recv_buf, RTSP_RESPONSE_SIZE);
    parseRtspHeader (tmp_buf, &ret, ret_msg, &ret_cseq, ret_session_id, rtp_info);

    SEQUENCE_NUM_CHECK(ret_cseq, seq_no);

    /* Increment sequence number for next command */
    seq_no++;

error:
    FREE(buf);
    FREE(recv_buf);
    FREE(tmp_buf);
    return ret;
}

int teardown(char *session_id,  /* In: Session id for RTSP session */
             char *url,         /* In: Rtsp URL for stream. Can be NULL */
             int strm_id        /* In: If url is NULL then strm_id will be used to generate the rtsp url */
             )
{
    int ret = 200;
    char *buf;
    char *recv_buf;
    char *tmp_buf;
    char ret_msg[128], ret_session_id[128], rtp_info[128];
    int ret_cseq;

    buf = (char *)MALLOC(RTSP_HEADER_SIZE+1);
    recv_buf = (char *)MALLOC(RTSP_RESPONSE_SIZE);
    tmp_buf = (char *)MALLOC(RTSP_RESPONSE_SIZE);

    if (url)
    {
        snprintf (buf, RTSP_HEADER_SIZE,
                       "TEARDOWN %s RTSP/1.0\r\n"
                       "CSeq: %d\r\n"
                       "Session: %s\r\n"
                       RTSP_REQUEST_HEADER_CRLF, url, seq_no, session_id);
    }
    else
    {
        snprintf (buf, RTSP_HEADER_SIZE,
                       "TEARDOWN rtsp://%s:554/stream=%d RTSP/1.0\r\n"
                       "CSeq: %d\r\n"
                       "Session: %s\r\n"
                       RTSP_REQUEST_HEADER_CRLF, inet_ntoa(*(struct in_addr *)&server_ip), strm_id, seq_no, session_id);
    }

    if (SET_SOCKET_TIMEOUT(sock_desc) != 0)
		goto error;

    if (NO_ERROR != (ret = tcpPayloadSendReceive(sock_desc, buf, recv_buf, RTSP_RESPONSE_SIZE, "RTSP TEARDOWN")))
        goto error;

    PRINT_REQ_RESP(buf, recv_buf);

    /* parse common headers */
    strncpy(tmp_buf, recv_buf, RTSP_RESPONSE_SIZE);
    parseRtspHeader (tmp_buf, &ret, ret_msg, &ret_cseq, ret_session_id, rtp_info);

    SEQUENCE_NUM_CHECK(ret_cseq, seq_no);

    /* Increment sequence number for next command */
    seq_no++;

error:
    FREE(buf);
    FREE(recv_buf);
    FREE(tmp_buf);
    return ret;
}

int options(char *session_id,  /* In: Session id for RTSP session */
            char *url,         /* In: Rtsp URL for stream. Can be NULL */
            int strm_id        /* In: If url is NULL then strm_id will be used to generate the rtsp url */
            )
{
    int ret = 200;
    char *buf;
    char *recv_buf;
    char *tmp_buf;
    char ret_msg[128], ret_session_id[128], rtp_info[128];
    int ret_cseq;

    buf = (char *)MALLOC(RTSP_HEADER_SIZE+1);
    recv_buf = (char *)MALLOC(RTSP_RESPONSE_SIZE);
    tmp_buf = (char *)MALLOC(RTSP_RESPONSE_SIZE);

    if (url)
    {
        snprintf (buf, RTSP_HEADER_SIZE,
                       "OPTIONS %s RTSP/1.0\r\n"
                       "CSeq: %d\r\n"
                       "Session: %s\r\n"
                       RTSP_REQUEST_HEADER_CRLF, url, seq_no, session_id);
    }
    else
    {
        snprintf (buf, RTSP_HEADER_SIZE,
                       "OPTIONS rtsp://%s:554/stream=%d RTSP/1.0\r\n"
                       "CSeq: %d\r\n"
                       "Session: %s\r\n"
                       RTSP_REQUEST_HEADER_CRLF, inet_ntoa(*(struct in_addr *)&server_ip), strm_id, seq_no, session_id);
    }

    if (SET_SOCKET_TIMEOUT(sock_desc) != 0)
		goto error;

    if (NO_ERROR != (ret = tcpPayloadSendReceive(sock_desc, buf, recv_buf, RTSP_RESPONSE_SIZE, "RTSP OPTIONS")))
        goto error;

    PRINT_REQ_RESP(buf, recv_buf);

    /* parse common headers */
    strncpy(tmp_buf, recv_buf, RTSP_RESPONSE_SIZE);
    parseRtspHeader (tmp_buf, &ret, ret_msg, &ret_cseq, ret_session_id, rtp_info);

    SEQUENCE_NUM_CHECK(ret_cseq, seq_no);

    /* Increment sequence number for next command */
    seq_no++;

error:
    FREE(buf);
    FREE(recv_buf);
    FREE(tmp_buf);

    return ret;
}

int readAndValidateRtpData(int server_addr, /* In: Server Ip address */
                           int server_port, /* In: Server Port */
                           int client_port  /* In: Client Port */
                           )
{
    int ret = NO_ERROR;
    int sd, i, j, total_bytes = 0;
    unsigned char *buf;
    struct sockaddr_in client_addr, serv_addr;
    bool datavalid = true;
    int size = sizeof(serv_addr);

    /* open udp socket for RTP data */
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0) {
        printf ("UDP Socket Open Err\n");
        return INVALID_ARGUMENT;
    }

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port = htons(client_port);

    buf = (unsigned char *)MALLOC(2048);

    /* Bind to client address/port */
    if (bind(sd,(struct sockaddr *) &client_addr, sizeof(client_addr))) {
        printf("Socket Bind Err\n");
        ret = INVALID_ARGUMENT;
        goto close_socket;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = server_addr;
    serv_addr.sin_port = htons(server_port);

    /* Set receive timeout fo the socket */
    if (SET_SOCKET_TIMEOUT(sd) != 0)
		goto close_socket;

    printf ("Validating RTP payload ...\n");
    /* Read 100 RTP packets and validate TS packer header sync byte - 0x47 */
    for (i = 0; i < 100; i++)
    {
        ret = recvfrom(sd, buf, 2048, 0, &serv_addr, (socklen_t *)&size);
        if (ret < 0)
        {
            printf ("Error reading from Socket : %d\n", errno);
            ret = SOCKET_IO_ERROR;
            goto close_socket;
        }
        total_bytes += ret;
        /* Skip 12 byte RTP header and check for sync at every TS packet size (188 bytes) */
        for (j = 12; j < ret; j += 188)
        {
            if (buf[j] != TS_HEADER_SYNC_BYTE)
            {
                datavalid = false;
                break;
            }
        }
    }

    if ((datavalid == false) || (total_bytes < 200))
    {
        printf ("RTP data received does not contain valid MPEG2 TS stream data\n");
        ret = ERROR;
    }
    else
    {
        printf ("RTP data validated to be carrying MPEG2 TS data\n");
    }

close_socket:
    shutdown(sd, 2);
    close(sd);
    FREE(buf);
    return ret;
}

int readAndValidateMcastRtpData(int mcast_addr, /* In: Server Ip address */
                                int client_port  /* In: Client Port */
                                )
{
    int ret = NO_ERROR;
    int sd, i, j, total_bytes = 0;
    unsigned char *buf;
    struct sockaddr_in client_addr;
    struct ip_mreq mreq;
    bool datavalid = true;
    int size = sizeof(mcast_addr);

    /* open udp socket for RTP data */
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0) {
        printf ("UDP Socket Open Err\n");
        return INVALID_ARGUMENT;
    }

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port = htons(client_port);

    buf = (unsigned char *)MALLOC(2048);

    /* Bind to client address/port */
    if (bind(sd,(struct sockaddr *) &client_addr, sizeof(client_addr))) {
        printf("Socket Bind Err\n");
        ret = INVALID_ARGUMENT;
        goto close_socket;
    }

    mreq.imr_multiaddr.s_addr = mcast_addr;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    /* Set the socket option for joining multicast group */
    if (setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0) {
        printf ("Error joining multicast group : %s\n", inet_ntoa(*(struct in_addr *)&mcast_addr));
        ret = SOCKET_ERROR;
        goto close_socket;
    }

    /* Set receive timeout fo the socket */
    if (SET_SOCKET_TIMEOUT(sd) != 0)
		goto close_socket;

    printf ("Validating MCAST RTP payload ...\n");
    /* Read 100 RTP packets and validate TS packer header sync byte - 0x47 */
    for (i = 0; i < 100; i++)
    {
        ret = recvfrom(sd, buf, 2048, 0, (struct sockaddr_in*)&mcast_addr, (socklen_t *)&size);
        if (ret < 0)
        {
            printf ("Error reading from Socket : %d\n", errno);
            ret = SOCKET_IO_ERROR;
            goto close_socket;
        }
        total_bytes += ret;
        /* Skip 12 byte RTP header and check for sync at every TS packet size (188 bytes) */
        for (j = 12; j < ret; j += 188)
        {
            if (buf[j] != TS_HEADER_SYNC_BYTE)
            {
                datavalid = false;
                break;
            }
        }
    }

    if ((datavalid == false) || (total_bytes < 200))
    {
        printf ("RTP mcast data received does not contain valid MPEG2 TS stream data\n");
        ret = ERROR;
    }
    else
    {
        printf ("RTP mcast data validated to be carrying MPEG2 TS data\n");
    }

close_socket:
    shutdown(sd, 2);
    close(sd);
    FREE(buf);
    return ret;
}
