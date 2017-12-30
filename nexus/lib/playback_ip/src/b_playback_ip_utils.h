/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
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
* Description:
*
***************************************************************************/
#if defined(LOG_IP_LATENCY)
#include "ts_packet.h"
#endif

void B_PlaybackIp_UtilsTuneNetworkStack(int fd);
int B_PlaybackIp_UtilsWaitForSocketData(B_PlaybackIpHandle playback_ip, bool *recvTimeout);
int
B_PlaybackIp_UtilsWaitForSocketWriteReady(int fd, int timeout /* in usec */);
int B_PlaybackIp_UtilsGetPlaypumpBuffer(B_PlaybackIpHandle playback_ip, unsigned int size);
int B_PlaybackIp_UtilsGetPlaypumpBufferNoSkip(B_PlaybackIpHandle playback_ip, unsigned int *size);
int B_PlaybackIp_UtilsGetPlaypumpBufferDepthInUsec( B_PlaybackIpHandle playback_ip, unsigned int *pDepthInUsec);
int B_PlaybackIp_UtilsFlushAvPipeline(B_PlaybackIpHandle playback_ip);
int B_PlaybackIp_UtilsFlushSocket(B_PlaybackIpHandle playback_ip);
B_PlaybackIpError B_PlaybackIp_UtilsReadUdpPayloadChunk(
    B_PlaybackIpHandle playback_ip,
    void *buf,       /* buffer big enough to hold (bufSize * iterations) bytes */
    int bufSize,     /* how much to read per iteration */
    int iterations, /* number of iterations (i.e. recv calls) */
    int *bytesRecv,    /* pointer to list of bytesRecv for each recvfrom call */
    int *totalBytesRecv, /* total bytes received */
    bool *recvTimeout   /* set if timeout occurs while waiting to receive a IP packet: helps detect unmarked discontinuity */
    );
bstream_mpeg_type B_PlaybackIp_UtilsNexus2Mpegtype(NEXUS_TransportType nexus_value);
NEXUS_VideoCodec B_PlaybackIp_UtilsVideoCodec2Nexus(bvideo_codec settop_value);
NEXUS_AudioCodec B_PlaybackIp_UtilsAudioCodec2Nexus(baudio_format settop_value);
NEXUS_TransportType B_PlaybackIp_UtilsMpegtype2ToNexus(bstream_mpeg_type settop_value);
B_PlaybackIpError B_PlaybackIp_UtilsTcpSocketConnect(volatile B_PlaybackIpState *playbackIpState, char *server, int port, bool nonBlockingSocket, int timeout, int *psd);
int B_PlaybackIp_UtilsTcpSocketWrite(volatile B_PlaybackIpState *playbackIpState, int sd, char *wbuf, int wbuf_len);
char *B_PlaybackIp_UtilsStristr(char *str, char *subStr);
/* enable this to track the caller of strdups */
#if 0
#define B_PlaybackIp_UtilsStrdup(uri) B_PlaybackIp_UtilsStrdup_Tagged(uri, BSTD_FUNCTION, __LINE__)
#endif
char *B_PlaybackIp_UtilsStrdup(char *src);
char *B_PlaybackIp_UtilsRealloc(char *curBuffer, int curBufferSize, int newBufferSize);
int B_PlaybackIp_UtilsWaitForPlaypumpDecoderSetup(B_PlaybackIpHandle playback_ip);

#define HTTP_HDR_TERMINATOR_STRING "\r\n\r\n"
typedef enum B_PlaybackIpHttpHdrParsingResult {
    B_PlaybackIpHttpHdrParsingResult_eSuccess,          /* Header is successfully parsed */
    B_PlaybackIpHttpHdrParsingResult_eIncompleteHdr,    /* Header is not terminated w/ CRNL sequence */
    B_PlaybackIpHttpHdrParsingResult_eIncorrectHdr,     /* Some key header fields such as CR/NL may be missing */
    B_PlaybackIpHttpHdrParsingResult_eReadNextHdr,      /* Need to Do another Read to get the Next HTTP Header Response */
    B_PlaybackIpHttpHdrParsingResult_eStatusRedirect,   /* Received HTTP Redirect */
    B_PlaybackIpHttpHdrParsingResult_eStatusBadRequest, /* Request contains bad syntax or can not be fullfilled by the server */
    B_PlaybackIpHttpHdrParsingResult_eStatusServerError,/* Server failed to fullfill a valid request due to some error */
    B_PlaybackIpHttpHdrParsingResult_eStatusNotSupported /* Currently, this HTTP return status is not supported by HTTP client */
}B_PlaybackIpHttpHdrParsingResult;

typedef struct B_PlaybackIpHttpMsgFields {
    B_PlaybackIpHttpHdrParsingResult parsingResult; /* Defined above */
    int statusCode;          /* HTTP response status code value, may be set even in ERROR case */
    /* rest of the fields are only set if B_ERROR_SUCCESS is returned */
    off_t contentLength;      /* length of the content being played via HTTP */
    bool chunkEncoding;       /* if true, server is sending content using HTTP Chunk Transfer Encoding */
    char *httpHdr;            /* pointer to the HTTP hdr if present */
    char *httpPayload;        /* pointer to the HTTP payload if present */
    int httpPayloadLength;    /* HTTP payload length, 0 if not present */
} B_PlaybackIpHttpMsgFields;

/* Function to parse HTTP Response Header. Returns B_ERROR_SUCCESS if header is successfully parsed */
B_PlaybackIpError B_PlaybackIp_UtilsHttpResponseParse(
    char *rbuf,                             /* read buffer pointing to start of HTTP Response Header */
    unsigned int bytesRead,                 /* # of bytes present in the read buffer */
    B_PlaybackIpHttpMsgFields *httpFields   /* output structure containing various fields from successfully parsed header */
    );

/* Function to byte-swap a buffer */
void
B_PlaybackIp_UtilsByteSwap(
    char *buf,
    int bufSize
    );

/* Function to build a Wav Header */
int
B_PlaybackIp_UtilsBuildWavHeader(
    uint8_t *buf,
    size_t bufSize,
    unsigned bitsPerSample,
    unsigned sampleRate,
    unsigned numChannels
    );

/* Functions to stream live or pre-recorded content to a network client */
int B_PlaybackIp_UtilsSendNullRtpPacket(struct bfile_io_write_net * data);
B_PlaybackIpError B_PlaybackIp_UtilsStreamingCtxOpen(struct bfile_io_write_net *data);
void B_PlaybackIp_UtilsStreamingCtxClose(struct bfile_io_write_net *data);
ssize_t B_PlaybackIp_UtilsStreamingCtxWrite(bfile_io_write_t self, const void *buf, size_t length);
ssize_t B_PlaybackIp_UtilsStreamingCtxWriteAll(bfile_io_write_t self, const void *buf, size_t bufSize);
extern B_PlaybackIpError B_PlaybackIp_UtilsRtpUdpStreamingCtxOpen(B_PlaybackIpSecurityOpenSettings *securitySettings, struct bfile_io_write_net *data);
extern void B_PlaybackIp_UtilsRtpUdpStreamingCtxClose(struct bfile_io_write_net *data);
void B_PlaybackIp_UtilsSetRtpPayloadType( NEXUS_TransportType mpegType, int *rtpPayloadType);
B_PlaybackIpError B_PlaybackIp_UtilsPvrDecryptionCtxOpen(B_PlaybackIpSecurityOpenSettings *securitySettings, struct bfile_io_write_net *data);
void B_PlaybackIp_UtilsPvrDecryptionCtxClose(struct bfile_io_write_net *data);
B_PlaybackIpError B_PlaybackIp_UtilsPvrDecryptBuffer(struct bfile_io_write_net *data, unsigned char *buf, unsigned char *clearBuf, int bufSize, int *outBufSize);
#ifdef EROUTER_SUPPORT
B_PlaybackIpError B_PlaybackIp_UtilsSetErouterFilter(B_PlaybackIpHandle playback_ip, B_PlaybackIpSessionOpenSettings *openSettings);
void B_PlaybackIp_UtilsUnsetErouterFilter(B_PlaybackIpHandle playback_ip);
#endif
void B_PlaybackIp_UtilsFreeMemory(void *buffer);
void *B_PlaybackIp_UtilsAllocateMemory(int size, NEXUS_HeapHandle heapHandle);
void B_PlaybackIp_UtilsMediaProbeCreate(void *data);
void B_PlaybackIp_UtilsMediaProbeDestroy(void *data);
#ifdef B_HAS_DTCP_IP
void B_PlaybackIp_UtilsDtcpServerCtxClose(struct bfile_io_write_net *data);
B_PlaybackIpError B_PlaybackIp_UtilsDtcpServerCtxOpen(B_PlaybackIpSecurityOpenSettings *securitySettings, struct bfile_io_write_net *data);
#endif
int B_PlaybackIp_UtilsParsePlaySpeedString(const char *playSpeedStringOrig, int *speedNumerator, int *speedDenominator, int *direction);
bool B_PlaybackIp_UtilsEndOfStream( B_PlaybackIpHandle playback_ip);
unsigned B_PlaybackIp_UtilsPlayedCount( B_PlaybackIpHandle playback_ip);
unsigned B_PlaybackIp_UtilsGetEndOfStreamTimeout( B_PlaybackIpHandle playback_ip);
void B_PlaybackIp_UtilsBuildPictureTagBtp(unsigned pid, unsigned pictureTag, unsigned timestampOffset, uint8_t *pkt);
void B_PlaybackIp_UtilsBuildInlineFlushBtp(unsigned pid, unsigned timestampOffset, uint8_t *pkt);
void B_PlaybackIp_UtilsBuildPictureOutputCountBtp(unsigned pid, unsigned pictureCount, unsigned timestampOffset, uint8_t *pkt);

#if defined(LOG_IP_LATENCY)
bool B_PlaybackIp_UtilsReadEnvInt(char *varName, int *var, int varMin, int varMax);
NEXUS_Error B_PlaybackIp_UtilsGetVideoDecoderFifoStatus(B_PlaybackIpHandle playback_ip, NEXUS_VideoDecoderFifoStatus *pfifoStatus);
void B_PlaybackIp_UtilsTrkLatencyStreamToDecodePts(B_PlaybackIpHandle playback_ip, TS_packet *pTsPkt);
void B_PlaybackIp_UtilsTrkPcrJitter(B_PlaybackIpHandle playback_ip, TS_packet *pTsPkt);
void B_PlaybackIp_UtilsTrkLatencyLog(B_PlaybackIpHandle playback_ip);
void B_PlaybackIp_UtilsTrkLatencyInit(B_PlaybackIpHandle playback_ip);
#endif

bool B_PlaybackIp_UtilsDiscardBufferedData( B_PlaybackIpHandle playback_ip, int socketFd, int socketType);
B_PlaybackIpError B_PlaybackIp_UtilsGetTcpWriteQueueSize(int socketFd, int *pWriteQueueDepth);
