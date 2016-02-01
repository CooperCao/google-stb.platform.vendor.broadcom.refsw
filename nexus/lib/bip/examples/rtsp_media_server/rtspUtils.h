/***************************************************************************
 *     (c)2007-2015 Broadcom Corporation
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
 * Module Description: RTSP client test utils header for test apps
 *
 *************************************************************/


/* Stream info structure describing the
 * information provided by a SAT>IP server
 * in response to a DESCRIBE command */

#define RTSP_OK                     200

#define RTSP_HEADER_SIZE            2048
#define RTSP_RESPONSE_SIZE          16384
#define RTSP_REQUEST_HEADER_CRLF    "Connection:close\r\n\r\n"
#define TS_HEADER_SYNC_BYTE         0x47

extern int server_ip;
extern int sock_desc;
extern int seq_no;
extern int verbose;

#define SEQUENCE_NUM_CHECK(A, B)    \
    if (A != B)                     \
    {                               \
        printf ("ERROR!!! : Returned seq number (%d) != Request Sequence number (%d)\n", A, B); \
    }

#define PRINT_REQ_RESP(A, B)    \
    if (verbose)                \
    {                           \
        printf ("\n###############################################\nSent Request\n-----------------------------------------------\n%s\n", A);          \
        printf ("###############################################\nReceived Response\n-----------------------------------------------\n%s\n###############################################\n", B);\
    }

typedef struct RTSP_StreamInfo {
    int stream_id;          /* stream identifier */
    int mcast_addr;         /* a value of 0 indicates unicast and non-zero indicates multicast */
    int port;               /* valid only for multicast streams */
    char media_type[32];    /* media type */
    char media_proto[32];   /* media protocal */
    char media_format[32];  /* media description format (33 for SAT>IP) */
    char tuner_params[256]; /* Tuner parameters */
    bool active;            /* Indicates if the transport stream is active */
} RTSP_StreamInfo;

typedef struct RTSP_SessionInfo {
    int stream_id;          /* stream identifier */
    char session_id[128];   /* session identifier */
    bool b_mcast;           /* Multicast Session? */
    int src_addr;           /* RTSP Server ip address */
    int dest_addr;          /* if b_mcast = true, then multicast detination address */
    int client_port[2];     /* Client ports for RTP data transfer */
    int port[2];            /* Server ports / Multicast deistination ports for RTP data transfer */
} RTSP_SessionInfo;

int setServerIp(char *server /* In : Server IP address in number-and-dots format*/
                );

int setSockDesc(int sd /* In: Socket Descriptor */
                );

int parseRtspHeader (char *header,          /* In: Pointer to RTSP header */
                     int *ret_code,         /* Out: Return code */
                     char *ret_code_msg,    /* Out: Return code message */
                     int *cseq,             /* Out: sequence number */
                     char *session,         /* Out: Session identifier */
                     char *rtp_info         /* Out: RTP information - containing the url - can be null */
                     );

int describe(char *session_id,            /* In: Pointer to session id - Can be set to NULL if not in session */
             RTSP_StreamInfo *strm_info,  /* Out: Stream information from the RTSP server */
             int *max_entries             /* In/Out: The size of the array strm_info is passed
                                           * and the number of entreis populated is returned */
             );

void printRtspStreamInfo(RTSP_StreamInfo *strm_info,  /* In: Stream info array */
                       int size                     /* In: Size of the array */
                       );

int parseDescribeResponse(char *response,               /* In: Pointer to RTSP response */
                          RTSP_StreamInfo *strm_info,   /* Out: Stream information from the RTSP server */
                          int *size                     /* In/Out: The size of the array strm_info is passed
                                                         * and the number of entreis populated is returned */
                          );

int setup(int *ports,                   /* In: client ports in case of unicast and dest ports in case of multicast */
          bool b_mcast,                 /* In: Multicast setup request? */
          char *url,                    /* In: RTSP url with tuning parameters etc ... for the SAT>IP server */
          RTSP_SessionInfo *session_info/* Out: Session setup information */
          );

void printSessionInfo(RTSP_SessionInfo *session_info  /* In: Session information from response to SETUP command */
                      );

int parseSetupResponse(char *response,                  /* In: Pointer to RTSP response */
                       RTSP_SessionInfo *session_info   /* Out: Session information from response to SETUP command */
                       );

int play(char *session_id,  /* In: Session id for RTSP session */
         char *url,         /* In: Rtsp URL for stream. Can be NULL */
         int strm_id,       /* In: If url is NULL then strm_id will be used to generate the rtsp url */
         char *rtp_info     /* Out: Rtp Info url - can be null */
         );

int teardown(char *session_id,  /* In: Session id for RTSP session */
             char *url,         /* In: Rtps URL for stream. Can be NULL */
             int strm_id        /* In: If url is NULL then strm_id will be used to generate the rtsp url */
             );

int options(char *session_id,  /* In: Session id for RTSP session */
            char *url,         /* In: Rtps URL for stream. Can be NULL */
            int strm_id        /* In: If url is NULL then strm_id will be used to generate the rtsp url */
            );

int readAndValidateRtpData(int server_ip, /* In: Server Ip address */
                           int server_port, /* In: Server Port */
                           int client_port  /* In: Client Port */
                           );

int readAndValidateMcastRtpData(int mcast_addr, /* In: Server Ip address */
                                int client_port  /* In: Client Port */
                                );
