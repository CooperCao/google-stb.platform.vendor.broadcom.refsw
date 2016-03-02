/***************************************************************************
*     (c)2003-2016 Broadcom Corporation
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
* Description: DTCP-IP module
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#if defined(LINUX) || defined(__vxworks)

#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_utils.h"
#include "b_dtcp_applib.h"
#include "b_dtcp_constants.h"

#if 0
#define RECORD_CLEAR_DATA
#endif
#ifdef RECORD_CLEAR_DATA
#include <stdio.h>
static FILE * fclear = NULL;
static FILE * fenc = NULL;
#endif

BDBG_MODULE(b_playback_ip_dtcp_ip);
#define DTCP_IP_ENCRYPT_BUF_SIZE (128*1024)
typedef struct B_PlaybackIpDtcpIpCtx
{
    void *akeHandle;
    void *streamHandle;
    B_PlaybackIpSessionOpenSettings openSettings;
    char initialPayload[TMP_BUF_SIZE];  /* buffer to hold any payload data that got read part of the initial HTTP header processing */
    int initialPayloadLength;
    char *encryptBuf;
    int encryptBufSize;
    /* The following members are used within _http_dtcp_ip_socket_read */
    /* They are used to store data during read and decryption for 16 byte aligment */
    char extended_decrypted_bytes[HTTP_AES_BLOCK_SIZE];  /* stores extended decrypted bytes (in clear) as caller asked for less than 16 bytes of data */
    int extended_len;       /* extended bytes read and decrypted */
    char residual_encrypted_bytes[HTTP_AES_BLOCK_SIZE];  /* stores left over bytes from decryption, that did not get decrypted*/
    int residual_encrypted_len;
    char partial_read_bytes[HTTP_AES_BLOCK_SIZE];  /* stores partial read bytes, non mod 16 aligned bytes */
    int partial_read_len;
} B_PlaybackIpDtcpIpCtx;

void B_PlaybackIp_DtcpIpSessionClose(void *voidHandle);
/*
 * This function tries to read the requested amount from the socket and returns any errors or bytes read.
 * It returns:
 *  =-1: for errors other than EINTR & EAGAIN during read call or when channel change occurs
 *  = 0: for EOF where server closed the TCP connection
 *  > 0: for success indicating number of bytes read from the socket
 */
int
_http_dtcp_ip_socket_read(void *voidHandle, B_PlaybackIpHandle playback_ip, int sd, char *rbuf, int rbuf_len)
{
    static int firstTime = 1;
    int rc = 0;
    int bytesRead = 0;
    B_PlaybackIpDtcpIpCtx *securityCtx = (B_PlaybackIpDtcpIpCtx *)voidHandle;
    char *read_ptr;
    int read_len = rbuf_len;
    int bytesToRead = 0;
    int bytesToCopy;
    unsigned int truncated_len = 0;
    char *orig_rbuf = NULL;
    char temp_bytes[HTTP_AES_BLOCK_SIZE];  /* Used for reading upto 16 bytes */
    B_PlaybackIpState *playbackIpState = &playback_ip->playback_state;
    bool dtcp_pcp_header_found = 0;

#ifdef RECORD_CLEAR_DATA
    if(fclear == NULL)
        fclear = fopen("/data/videos/output.mpg", "wb+");
#endif
    /*
     * if using hardware decryption, single buffer is used for reading data from socket and
     * for decryption. For sw decryption, two separate buffers (one for socket read and one for decryption ) are used.
     */
#ifdef B_DTCP_IP_HW_DECRYPTION
    BDBG_MSG(("%s: Using HW decryption for DTCP, rbuf_len %d, sd %d", __FUNCTION__, rbuf_len, sd));
#else
    BDBG_MSG(("%s: Using SW decryption for DTCP, rbuf_len %d, sd %d", __FUNCTION__, rbuf_len, sd));
#endif
    if (rbuf_len > securityCtx->encryptBufSize) {
        /* Realloc */
        B_PlaybackIp_UtilsFreeMemory(securityCtx->encryptBuf);
        if ( (securityCtx->encryptBuf = (char *)B_PlaybackIp_UtilsAllocateMemory(rbuf_len, securityCtx->openSettings.heapHandle)) == NULL) {
            BDBG_ERR(("%s:%d: memory allocation failure\n", __FUNCTION__, __LINE__));
            return -1;
        }
        BDBG_MSG(("%s:%d: reallocated encrypt buffer", __FUNCTION__, __LINE__));
        securityCtx->encryptBufSize = rbuf_len;
    }
    read_ptr = securityCtx->encryptBuf;
#ifdef BDBG_DEBUG_BUILD
    if (playback_ip->ipVerboseLog && firstTime) {
        firstTime = 0;
#ifdef B_DTCP_IP_HW_DECRYPTION
        BDBG_WRN(("%s: Using HW decryption for DTCP", __FUNCTION__));
#else
        BDBG_WRN(("%s: Using SW decryption for DTCP", __FUNCTION__));
#endif
    }
#endif

    /* Return extra decrypted data from previous read */
    if (securityCtx->extended_len && securityCtx->extended_len <= HTTP_AES_BLOCK_SIZE) {
        bytesToCopy = rbuf_len > securityCtx->extended_len ? securityCtx->extended_len : rbuf_len;
        memcpy(rbuf, securityCtx->extended_decrypted_bytes, bytesToCopy);
        bytesRead = bytesToCopy;
        securityCtx->extended_len -= bytesRead;
        if (securityCtx->extended_len != 0) {
            /* move the remaining bytes to the beginning */
            memmove(securityCtx->extended_decrypted_bytes, securityCtx->extended_decrypted_bytes + bytesRead, securityCtx->extended_len);
        }
        BDBG_MSG(("%s: extended bytes returned %d, remaining extended bytes %d", __FUNCTION__, bytesRead, securityCtx->extended_len));
        return bytesRead;
    }

    /* Adjust read amount to meet the AES 16 byte alignment requirement */
    if (securityCtx->openSettings.security.enableDecryption) {
        /* Truncated if greater than 16 and not mod 16 */
        if ((rbuf_len > HTTP_AES_BLOCK_SIZE) && (rbuf_len % HTTP_AES_BLOCK_SIZE)) {
            truncated_len = rbuf_len % HTTP_AES_BLOCK_SIZE;
            rbuf_len -= truncated_len;   /* Truncate length of clear data expected in return */
            BDBG_MSG(("%s: Adjust length for decryption block size, original %d, new %d", __FUNCTION__, rbuf_len+truncated_len, rbuf_len));
        } else if (rbuf_len < HTTP_AES_BLOCK_SIZE) {
            /* since caller is asking to read/decrypt less than AES block size, we will need to read & decrypt extra bytes for 16 byte alignment */
            /* and then save the extended bytes for subsequent read */
            BDBG_MSG(("%s: Extend reading for %d bytes\n", __FUNCTION__,  rbuf_len));
            securityCtx->extended_len = HTTP_AES_BLOCK_SIZE - rbuf_len;
            rbuf_len += securityCtx->extended_len;   /*  Add padding  */
            orig_rbuf = rbuf;
            read_ptr = rbuf = temp_bytes;
        }
    }

    /* Copy previously read encrypted data (which was left over from the decryption due to PCP header being present) to begining of rbuf */
    /* Read length shouuld be 16 byte aligned */
    if (securityCtx->residual_encrypted_len && securityCtx->residual_encrypted_len <= HTTP_AES_BLOCK_SIZE ) {
        BDBG_MSG(("%s: Prepend previously read but not decrypted %d bytes, bytesRead so far %d", __FUNCTION__,  securityCtx->residual_encrypted_len, bytesRead));
        bytesToCopy = rbuf_len > securityCtx->residual_encrypted_len ? securityCtx->residual_encrypted_len : rbuf_len;
        memcpy(read_ptr+bytesRead, securityCtx->residual_encrypted_bytes, bytesToCopy);
        bytesRead += bytesToCopy;
        securityCtx->residual_encrypted_len -= bytesToCopy;
        if (securityCtx->residual_encrypted_len)
            memmove(securityCtx->residual_encrypted_bytes, securityCtx->residual_encrypted_bytes + bytesRead, securityCtx->residual_encrypted_len);
    }

    /* Copy previously read encrypted data (which was not decrypted due to mod 16 requirements) to begining of rbuf */
    /* Read length shouuld be 16 byte aligned */
    if (securityCtx->partial_read_len && securityCtx->partial_read_len <= HTTP_AES_BLOCK_SIZE) {
        bytesToCopy = (rbuf_len-bytesRead) > securityCtx->partial_read_len ? securityCtx->partial_read_len : (rbuf_len-bytesRead);
        BDBG_MSG(("%s: Preprend partial read %d bytes, total partial read %d, bytesRead already read %d", __FUNCTION__, bytesToCopy, securityCtx->partial_read_len, bytesRead));
        memcpy(read_ptr+bytesRead, securityCtx->partial_read_bytes, bytesToCopy);
        bytesRead += bytesToCopy;
        securityCtx->partial_read_len -= bytesToCopy;
        if (securityCtx->partial_read_len > 0)
            /* move the remaining bytes to the beginning of the partial_read_bytes array */
            memmove(securityCtx->partial_read_bytes, securityCtx->partial_read_bytes+bytesToCopy, securityCtx->partial_read_len);
    }

    if (securityCtx->initialPayloadLength && securityCtx->initialPayloadLength <= TMP_BUF_SIZE) {
        /* Copy initially read encrypted data to begining of rbuf */
        bytesToCopy = (rbuf_len-bytesRead) > securityCtx->initialPayloadLength ? securityCtx->initialPayloadLength : (rbuf_len-bytesRead);
        BDBG_MSG(("%s: copying read %d bytes from initial buf length of %d into decryption buffer, bytesRead so far %d", __FUNCTION__,  bytesToCopy, securityCtx->initialPayloadLength, bytesRead));
        memcpy(read_ptr+bytesRead, securityCtx->initialPayload, bytesToCopy);
        bytesRead += bytesToCopy;
        securityCtx->initialPayloadLength -= bytesToCopy;
        if (securityCtx->initialPayloadLength != 0)
            /* move the remaining bytes to the beginning of the intitialPayload */
            memmove(securityCtx->initialPayload, securityCtx->initialPayload + bytesToCopy, securityCtx->initialPayloadLength);
    }

    read_len = rbuf_len - bytesRead;
    bytesToRead = read_len;
    BDBG_MSG(("%s: now read remaining %d bytes, total rbuf_len %d, bytesRead %d, sd %d", __FUNCTION__,  read_len, rbuf_len, bytesRead, sd));

    if (rbuf_len < 0 || read_len < 0 )
        BDBG_ASSERT(NULL);
    /* read remaining bytesToRead bytes from the socket */
    while (read_len != 0) {
        /* read_len will only be 0 when initialPayload had all the requested bytes, so we dont need to read any data from the socket */
        if ((*playbackIpState == B_PlaybackIpState_eStopping) || (*playbackIpState == B_PlaybackIpState_eStopped )) {
            /* user changed the channel, so return */
            BDBG_MSG(("%s: breaking out of read loop due to state (%d) change\n", __FUNCTION__, *playbackIpState));
            return -1;
        }
        rc = read(sd, read_ptr + bytesRead, read_len);
        if (rc < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                if (bytesRead) {
                    BDBG_MSG(("%s: breaking from read loop due to partial data %d bytes recvd, bytesToRead %d", __FUNCTION__, bytesRead, bytesToRead));
                    break;  /* Partial data received */
                }
                else {
                    BDBG_MSG(("%s: Read System Call interrupted or timed out (errno %d)\n", __FUNCTION__, errno));
                    return rc;  /* no data read return */
                }
            }
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_ERR(("%s: read ERROR:%d", __FUNCTION__, errno));
#endif
            return -1;
        }else if (rc == 0) {
#ifdef BDBG_DEBUG_BUILD
            if (playback_ip->ipVerboseLog)
                BDBG_ERR(("%s: Reached EOF, server closed the connection!, bytesRead=%d", __FUNCTION__, bytesRead));
#endif
            break;
        } else if (rc < read_len)
        {
            bytesRead += rc;
            read_len -= rc;
            BDBG_MSG(("%s: Partial read rc %d, bytesRead so far %d, bytes remaining: read_len %d, rbuf_len %d, continue reading", __FUNCTION__, rc, bytesRead, read_len, rbuf_len));
            /* we sleep a little to allow more data to arrive from the server */
            BKNI_Sleep(10);
        } else {
            bytesRead += rc;
            BDBG_MSG(("%s: read all requested data: rc %d, bytesRead %d, read_len %d, rbuf_len %d", __FUNCTION__, rc, bytesRead, read_len, rbuf_len));
            break;
        }
    }

    if (bytesRead == 0) {
        return bytesRead;
    }

    /* if decryption is not yet enabled or is disabled, then simply return the read data */
    if (!securityCtx->openSettings.security.enableDecryption) {
        BDBG_MSG(("decryption is off: so memcopying the result of %d bytes from src %p to dst %p buffer", bytesRead, read_ptr, rbuf));
        memcpy(rbuf, read_ptr, bytesRead);
        return bytesRead;
    }

    BDBG_MSG(("%s: Asked %d bytes Read =%d bytes\n", __FUNCTION__, rbuf_len-securityCtx->extended_len+truncated_len, bytesRead));

    /* Partial data receive.  Data is NOT aligned to 16 byte */
    if (bytesRead % HTTP_AES_BLOCK_SIZE) {
        if (bytesRead > HTTP_AES_BLOCK_SIZE) {
            truncated_len = bytesRead % HTTP_AES_BLOCK_SIZE;
            BDBG_MSG(("%s: Partial data received %d, expected %d, need to truncate %d\n", __FUNCTION__, bytesRead, rbuf_len, truncated_len));
            bytesRead -= truncated_len;   /* Truncate length */
            securityCtx->partial_read_len = truncated_len;
            memcpy(securityCtx->partial_read_bytes, read_ptr+bytesRead, securityCtx->partial_read_len);
        }else {
            BDBG_MSG(("%s: bytesRead %d < AES block size, so returning EAGAIN: asked (rbuf_len) %d, app asked %d, rc %d, extended %d",
                        __FUNCTION__, bytesRead, rbuf_len, rbuf_len-securityCtx->extended_len, rc, securityCtx->extended_len));
            securityCtx->partial_read_len = bytesRead;
            memcpy(securityCtx->partial_read_bytes, read_ptr, securityCtx->partial_read_len);
            errno = EAGAIN;
            bytesRead = 0;
            securityCtx->extended_len = 0;
            return -1;
        }
    }

#if 0
    /* dump the encrypted data into file. */
    fwrite(playback_ip->encrypt_buf, 1, bytesRead, fenc);
    fflush(fenc);
#endif
    {
        unsigned int data_processed = 0 ;           /* This refer to each call to DepacketizData */
        unsigned int clear_buff_size = rbuf_len;
        unsigned int total_clear_data = 0;
        unsigned int beginPointer = 0;
        unsigned int endPointer = bytesRead;
        size_t bytesRemaining = bytesRead;
        BERR_Code errorCode;

        while (beginPointer != endPointer) {
            errorCode = DtcpAppLib_StreamDepacketizeData(securityCtx->streamHandle,
                    securityCtx->akeHandle,
                    (unsigned char *)read_ptr + beginPointer,   /* encrypted buffer */
                    bytesRemaining, /* encrypted buffer length */
                    (unsigned char *)rbuf + total_clear_data, /* clear buffer */
                    &clear_buff_size,   /* input: clear buffer length, output: length of decrypted bytes (may be less than data processed due to DTCP lib taking out PCP header) */
                    &data_processed,    /* how many bytes are processed by DTCP lib after decryption, includes the length of PCP header */
                    &dtcp_pcp_header_found);

            if (errorCode != BERR_SUCCESS) {
                BDBG_ERR(("%s: DTCP_DepacketizeData returned %d\n", __FUNCTION__, bytesRead));
                break;
            }
            if (playback_ip->chunkEncoding && dtcp_pcp_header_found ) {
                playback_ip->dtcpPcpHeaderFound = true;
                BDBG_MSG(("DTCP PCP header flag is set "));
            }

            BDBG_MSG(("len: bytesRemaining (enc) %d, clear %d, processed %d\n", bytesRemaining, clear_buff_size, data_processed));
            beginPointer += data_processed;
            bytesRemaining = endPointer - beginPointer;

            total_clear_data += clear_buff_size;

            /* Inform the API how much room we have left on output buffer.*/
            clear_buff_size = rbuf_len - clear_buff_size;

            /* Check for any unencrypted data */
            if ((bytesRemaining < HTTP_AES_BLOCK_SIZE) && (bytesRemaining > 0)) {
                BDBG_MSG(("bytesRemaining  %d are less than AES block size, save the encrypted bytes and continue next time, read_ptr %p, begin ptr %d, end ptr %d, clear_buf_size %d",
                            bytesRemaining, read_ptr, beginPointer, endPointer, clear_buff_size));
                securityCtx->residual_encrypted_len = bytesRemaining;
                memcpy(securityCtx->residual_encrypted_bytes, read_ptr+beginPointer, securityCtx->residual_encrypted_len);
                if (total_clear_data == 0 && (securityCtx->extended_len || (rbuf_len == HTTP_AES_BLOCK_SIZE))) {
                    /* so caller had asked for data <= AES block size and thus we had to extend the bytes to read, but since bytes remaining are less than AES block size, return EAGAIN to caller */
                    BDBG_MSG(("caller had asked for data < AES block size and thus we had to extend the bytes to read, but since bytes remaining are less than AES block size, return EAGAIN to caller"));
                    securityCtx->extended_len = 0;
                    errno = EAGAIN;
                    return -1;
                }
                break;
            }
        }

        /* Return the total decrypted data count */
        bytesRead = total_clear_data;
        BDBG_MSG(("playback_ip: total_clear_data=%d\n", bytesRead));

#ifdef RECORD_CLEAR_DATA
        /* write data to file */
        fwrite(rbuf, 1, bytesRead , fclear);
        fflush(fclear);
#endif
    }
    /* If padding occurred; then adjust length and copy data back to original buffer */
    if (securityCtx->extended_len) {
        BDBG_MSG(("%s: extended bytes saved %d, rbuf_len %d ", __FUNCTION__, securityCtx->extended_len, rbuf_len));
        rbuf_len -= securityCtx->extended_len;
        bytesRead -= securityCtx->extended_len;

        /* Save decrypted bytes */
        memcpy(temp_bytes, rbuf, bytesRead);
        memcpy(securityCtx->extended_decrypted_bytes,rbuf+rbuf_len,securityCtx->extended_len);

        /* copy from temp_bytes back to original rbuf */
        rbuf = orig_rbuf;
        memcpy(rbuf, temp_bytes, bytesRead);
    }
#if 0
#define VERIFY_SYNC_BYTE
#endif
#ifdef VERIFY_SYNC_BYTE
#define TS_PKT_SIZE 188
#define SYNC_BYTE_OFFSET 4
    {
        static long long totalBytesRead = 0;
        static int nextSyncByteOffset = 0;
        int i=0;
        if (bytesRead < nextSyncByteOffset) {
            nextSyncByteOffset -= bytesRead;
            totalBytesRead += bytesRead;
        }
        else {
            i = nextSyncByteOffset;
            for (i=nextSyncByteOffset; i<bytesRead; i+=TS_PKT_SIZE) {
                if (rbuf[i+SYNC_BYTE_OFFSET] != 0x47) BDBG_WRN(("################ sync bytes mismatch at %lld, i %d, bytes 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", totalBytesRead+i+SYNC_BYTE_OFFSET, i+SYNC_BYTE_OFFSET, rbuf[i], rbuf[i+1], rbuf[i+2], rbuf[i+3], rbuf[i+4], rbuf[i+5], rbuf[i+6], rbuf[i+7]));
#if 0
                else BDBG_MSG(("sync byte 0x%x, i %d, total %lld", rbuf[i+SYNC_BYTE_OFFSET], i+SYNC_BYTE_OFFSET, totalBytesRead));
#endif
            }
            totalBytesRead += bytesRead;
            nextSyncByteOffset = ((TS_PKT_SIZE - (bytesRead % TS_PKT_SIZE)) + nextSyncByteOffset) % TS_PKT_SIZE;
        }
    }
#endif
    return bytesRead;
}

int  B_PlaybackIp_DtcpIpDecryptionDisable(
    void *securityHandle
    )
{
    B_PlaybackIpDtcpIpCtx *securityCtx = (B_PlaybackIpDtcpIpCtx *)securityHandle;

    if (!securityCtx) {
        BDBG_ERR(("%s: invalid securityCtx %p", __FUNCTION__, securityCtx));
        return -1;
    }
    /* set flag to disable decryption */
    BDBG_MSG(("%s: Disabling DTCP/IP Decryption\n", __FUNCTION__));
    securityCtx->openSettings.security.enableDecryption = false;

    return 0;
}

int B_PlaybackIp_DtcpIpDecryptionEnable(
    void *securityHandle,
    char *initialPayload,
    int initialPayloadLength)
{
    B_PlaybackIpDtcpIpCtx *securityCtx = (B_PlaybackIpDtcpIpCtx *)securityHandle;

    if (!securityCtx || (initialPayloadLength && !initialPayload)) {
        BDBG_ERR(("%s: invalid securityCtx %p or initial paylaod params (length %d, payload ptr %p)\n", __FUNCTION__, securityCtx, initialPayloadLength, initialPayload));
        return -1;
    }
    /* set flag to enable decryption */
    BDBG_MSG(("%s: Enabling DTCP/IP Decryption\n", __FUNCTION__));
    securityCtx->openSettings.security.enableDecryption = true;

    if (initialPayloadLength) {
        securityCtx->initialPayloadLength = initialPayloadLength;
        if (initialPayloadLength > TMP_BUF_SIZE) {
            BDBG_ERR(("%s: Need to increase the initialPayload buffer length from %d to %d\n", __FUNCTION__, TMP_BUF_SIZE, initialPayloadLength));
            BDBG_ASSERT(NULL);
            return -1;
        }
        memcpy(securityCtx->initialPayload, initialPayload, initialPayloadLength);
    }

    return 0;
}

/*
 * Initialize session specific context of the security module
 * Called during B_PlaybackIp_SocketOpen().
 */
int B_PlaybackIp_DtcpIpSessionOpen(
    B_PlaybackIpSessionOpenSettings *openSettings,    /* input: provides server ip, port info */
    int sd,                                                     /* input: socket descriptor */
    B_PlaybackIpSecurityOpenOutputParams *securityOpenOutputParams) /* output: security settings return params */
{
    B_PlaybackIpDtcpIpCtx *securityCtx = NULL;
    B_PlaybackIpSecurityOpenSettings *securityOpenSettings;
    BERR_Code rc;

    BSTD_UNUSED(sd);

    securityOpenOutputParams->byteRangeOffset = 0;

    if (openSettings == NULL) {
        BDBG_ERR(("%s: Invalid parameters, Open Settings %p\n", __FUNCTION__, openSettings));
        goto error;
    }
    securityOpenSettings = &openSettings->security;

    if (sd <= 0) {
        BDBG_ERR(("%s: invalid socket, sd = %d", __FUNCTION__, sd));
        goto error;
    }

    if (securityOpenSettings->securityProtocol != B_PlaybackIpSecurityProtocol_DtcpIp) {
        BDBG_ERR(("%s: invoking DTCP/IP module with incorrect security protocol %d", __FUNCTION__, securityOpenSettings->securityProtocol));
        goto error;
    }

    if (securityOpenSettings->initialSecurityContext == NULL) {
        BDBG_ERR(("%s: AKE Handle is not provided by app\n", __FUNCTION__));
        goto error;
    }

    securityCtx = BKNI_Malloc(sizeof(B_PlaybackIpDtcpIpCtx));
    if (!securityCtx) {
        BDBG_ERR(("%s:%d: memory allocation failure\n", __FUNCTION__, __LINE__));
        goto error;
    }
    memset(securityCtx, 0, sizeof(B_PlaybackIpDtcpIpCtx));
    securityCtx->akeHandle = securityOpenSettings->initialSecurityContext;
    memcpy(&securityCtx->openSettings, openSettings, sizeof(B_PlaybackIpSessionOpenSettings));

    /* allocate an encrypt buffer of a default size and then re-malloc if user is asking for larger data */
    securityCtx->encryptBuf = (char *)B_PlaybackIp_UtilsAllocateMemory(DTCP_IP_ENCRYPT_BUF_SIZE, openSettings->heapHandle);
    if (!securityCtx->encryptBuf) {
        BDBG_ERR(("%s:%d: memory allocation failure\n", __FUNCTION__, __LINE__));
        goto error;
    }
    securityCtx->encryptBufSize = DTCP_IP_ENCRYPT_BUF_SIZE;
    if((securityCtx->streamHandle = DtcpAppLib_OpenSinkStream(securityCtx->akeHandle, B_StreamTransport_eHttp)) == NULL)
    {
        BDBG_ERR(("%S: Failed to open DTCP-IP sink stream\n", __FUNCTION__));
        rc = B_ERROR_SOCKET_ERROR;
        goto error;
    }

    BDBG_MSG(("%s: Successfully Opened Sink Stream: handles AKE %p, Stream %p\n", __FUNCTION__, securityCtx->akeHandle, securityCtx->streamHandle));

    BDBG_MSG(("%s: setting up the netIo interface for socket read & write\n", __FUNCTION__));
    securityOpenOutputParams->netIoPtr->read = _http_dtcp_ip_socket_read;
    /* TODO: if data written doesn't need to be protected, then write function need not be implemented */
    /* netIo->write = _http_dtcp_ip_socket_write; */
    securityOpenOutputParams->netIoPtr->close = B_PlaybackIp_DtcpIpSessionClose;
    securityOpenOutputParams->netIoPtr->suspend = NULL;

    *securityOpenOutputParams->securityHandle = (void *)securityCtx;
    return 0;
error:
    B_PlaybackIp_DtcpIpSessionClose(securityCtx);
    return -1;
}

int B_PlaybackIp_DtcpIpCloneSessionOpen(
    int sd,                                                     /* input: socket descriptor */
    void *source,                                               /* intput: original security handle */
    void **targetSecurityHandle)                                /* output: new security handle */
{
    (void)sd;
    B_PlaybackIpDtcpIpCtx *securityCtx = NULL;
    B_PlaybackIpDtcpIpCtx *sourceSecurityHandle = (B_PlaybackIpDtcpIpCtx *)source;

    securityCtx = BKNI_Malloc(sizeof(B_PlaybackIpDtcpIpCtx));
    if (!securityCtx) {
        BDBG_ERR(("%s:%d: memory allocation failure\n", __FUNCTION__, __LINE__));
        goto error;
    }
    memset(securityCtx, 0, sizeof(B_PlaybackIpDtcpIpCtx));
    securityCtx->akeHandle = sourceSecurityHandle->akeHandle;
    memcpy(&securityCtx->openSettings, &sourceSecurityHandle->openSettings, sizeof(B_PlaybackIpSessionOpenSettings));

    /* allocate an encrypt buffer of a default size and then re-malloc if user is asking for larger data */
    securityCtx->encryptBuf = (char *)B_PlaybackIp_UtilsAllocateMemory(DTCP_IP_ENCRYPT_BUF_SIZE, securityCtx->openSettings.heapHandle);
    if (!securityCtx->encryptBuf) {
        BDBG_ERR(("%s:%d: memory allocation failure\n", __FUNCTION__, __LINE__));
        goto error;
    }
    securityCtx->encryptBufSize = DTCP_IP_ENCRYPT_BUF_SIZE;
    if((securityCtx->streamHandle = DtcpAppLib_OpenSinkStream(securityCtx->akeHandle, B_StreamTransport_eHttp)) == NULL)
    {
        BDBG_ERR(("%S: Failed to open DTCP-IP sink stream\n", __FUNCTION__));
        goto error;
    }
    securityCtx->openSettings.security.enableDecryption = false;

    BDBG_MSG(("%s: Successfully Opened Sink Stream: handles AKE %p, Stream %p\n", __FUNCTION__, securityCtx->akeHandle, securityCtx->streamHandle));

    *targetSecurityHandle = securityCtx;
    return 0;
error:
    if (securityCtx && securityCtx->encryptBuf)
        B_PlaybackIp_UtilsFreeMemory(securityCtx->encryptBuf);
    if (securityCtx)
        BKNI_Free(securityCtx);
    return -1;
}

/*
 * Un-Initialize session specific context of the security module
 * Called during B_PlaybackIp_SocketClose().
 */
void B_PlaybackIp_DtcpIpSessionClose(
    void *voidHandle)                                           /* input: security module specific handle */
{
    B_PlaybackIpDtcpIpCtx *securityCtx = (B_PlaybackIpDtcpIpCtx *)voidHandle;
    if (securityCtx)
    {
        BDBG_MSG(("%s: Closing handles AKE %p, Stream %p\n", __FUNCTION__, securityCtx->akeHandle, securityCtx->streamHandle));
        /* Free up DtcpIp Handle & any other saved contexts */
        if(securityCtx->streamHandle) {
            DtcpAppLib_CloseStream(securityCtx->streamHandle);
        }
        if(securityCtx->encryptBuf){
            B_PlaybackIp_UtilsFreeMemory(securityCtx->encryptBuf);
        }
        BKNI_Free(securityCtx);
    }
}

static int gDtcpIpInitRefCnt = 0;
/*
 * Initialize global context of the security module
 * Called during B_PlaybackIp_Open().
 */
void * B_PlaybackIp_DtcpIpInit(void *nexusDmaHandle)
{
    void * ctx = NULL;

#ifndef B_DTCP_IP_HW_DECRYPTION
    BSTD_UNUSED(nexusDmaHandle);
#endif

    if (!gDtcpIpInitRefCnt) {
#if defined(B_DTCP_IP_HW_DECRYPTION)
        if(DtcpInitHWSecurityParams(nexusDmaHandle) != BERR_SUCCESS)
        {
            BDBG_ERR(("Failed to init DtcpIp HW Security params\n"));
            goto error;
        }
#endif

        if((ctx = DtcpAppLib_Startup(B_DeviceMode_eSink, false, 0, false)) == NULL)
        {
            BDBG_ERR(("DtcpIp AppLib faild to start\n"));
            goto error;
        }
        BDBG_MSG(("%s: done\n", __FUNCTION__));
    }
    gDtcpIpInitRefCnt++;

    return ctx;

error:

    DtcpAppLib_Shutdown(ctx);
#if defined(B_DTCP_IP_HW_DECRYPTION) || defined(B_DTCP_IP_HW_ENCRYPTION)
    DtcpCleanupHwSecurityParams();
#endif
    return NULL;
}

/*
 * Un-Initialize global context of the security module
 * Called during B_PlaybackIp_Close().
 */
void B_PlaybackIp_DtcpIpUnInit(void * ctx)
{

    gDtcpIpInitRefCnt--;
    if (!gDtcpIpInitRefCnt) {
        /* Global system un-initialization for DtcpIp library */
#ifdef B_DTCP_IP_HW_DECRYPTION
        DtcpCleanupHwSecurityParams();
#endif
        DtcpAppLib_Shutdown(ctx);
        /* unload library */
        BDBG_MSG(("%s: done\n", __FUNCTION__));
    }
}

#endif /* LINUX || VxWorks */
