/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
* Description: Code for decrypting and encrypting directly using AES algorithm
*
***************************************************************************/
#if defined(LINUX) || defined(__vxworks)

#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_utils.h"
#ifdef HW_DEC
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
#include "nexus_dma.h"
#include "nexus_security.h"
#endif
#else
/* SW decryption */
#include "openssl/aes.h"
#endif

#if 0
#define RECORD_CLEAR_DATA
#endif
#ifdef RECORD_CLEAR_DATA
#include <stdio.h>
static FILE * fclear = NULL;
#endif

BDBG_MODULE(b_playback_ip_aes);
#if 0
#define AES_ENCRYPT_BUFFER_SIZE (16*1024)
#endif
#define AES_ENCRYPT_BUFFER_SIZE 15040
typedef struct B_PlaybackIpAesCtx
{
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    NEXUS_KeySlotHandle encKeyHandle;
    NEXUS_DmaHandle dmaHandle;
#endif
    B_PlaybackIpSessionOpenSettings openSettings;
    char initialPayload[TMP_BUF_SIZE];  /* buffer to hold any payload data that got read part of the initial HTTP header processing */
    int initialPayloadLength;
    char *encryptBuf;
    int encryptBufSize;
    char *clearBuf;
    int clearBufSize;
    /* The following members are used within aes_socket_read */
    /* They are used to store data during read and decryption for 16 byte aligment */
    char extended_decrypted_bytes[HTTP_AES_BLOCK_SIZE];  /* stores extended decrypted bytes (in clear) as caller asked for less than 16 bytes of data */
    int extended_len;       /* extended bytes read and decrypted */
    char residual_encrypted_bytes[HTTP_AES_BLOCK_SIZE];  /* stores left over bytes from decryption, that did not get decrypted*/
    int residual_encrypted_len;
    char partial_read_bytes[HTTP_AES_BLOCK_SIZE];  /* stores partial read bytes, non mod 16 aligned bytes */
    int partial_read_len;
#ifndef HW_DEC
    AES_KEY aesSwDecKey;
#endif
} B_PlaybackIpAesCtx;

/* HW_DEC is disabled for now as it is currently not working. Instead, we are using OpenSSL for SW based decryption. */
#ifdef HW_DEC
bool
aes_setup_nexus_dma(
    B_PlaybackIpAesCtx *securityCtx
    )
{
    bool status = false;
    NEXUS_SecurityAlgorithmSettings NexusConfig;
    NEXUS_SecurityClearKey key;

    /* Set up encryption key */
    NEXUS_Security_GetDefaultAlgorithmSettings(&NexusConfig);
    NexusConfig.algorithm = NEXUS_SecurityAlgorithm_eAes;
    NexusConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eCbc;
    NexusConfig.terminationMode     = NEXUS_SecurityTerminationMode_eClear;
    NexusConfig.keyDestEntryType    = NEXUS_SecurityKeyType_eClear;
    NexusConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
    NexusConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
    NexusConfig.operation = NEXUS_SecurityOperation_eDecrypt;

    if (NEXUS_Security_ConfigAlgorithm(securityCtx->encKeyHandle, &NexusConfig)!= 0) {
        BDBG_ERR(("%s: NEXUS_Security_ConfigAlgorithm", __FUNCTION__));
        goto error;
    }

    /* Load key */
    NEXUS_Security_GetDefaultClearKey(&key);
    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    key.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;
    key.keySize = sizeof(securityCtx->openSettings.security.settings.aes128.key);
    BKNI_Memcpy(key.keyData, securityCtx->openSettings.security.settings.aes128.key, key.keySize);
    if (NEXUS_Security_LoadClearKey(securityCtx->encKeyHandle, &key) != 0) {
        BDBG_ERR(("%s: NEXUS_Security_LoadClearKey Failed to load Dec Key", __FUNCTION__));
        goto error;
    }
    for (int i=0; i<16; i++) {
        BDBG_MSG_FLOW(("%s: key[%d] 0x%02x", __FUNCTION__, i, (unsigned char)key.keyData[i]));
    }
    /* Load IV */
    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    key.keyIVType    = NEXUS_SecurityKeyIVType_eIV;
    key.keySize = sizeof(securityCtx->openSettings.security.settings.aes128.iv);
    BKNI_Memcpy(key.keyData, securityCtx->openSettings.security.settings.aes128.iv, key.keySize);
    if (NEXUS_Security_LoadClearKey(securityCtx->encKeyHandle, &key) != 0) {
        BDBG_ERR(("%s: NEXUS_Security_LoadClearKey Failed to load IV", __FUNCTION__));
        goto error;
    }
    for (int i=0; i<16; i++) {
        BDBG_MSG_FLOW(("%s: iv[%d] 0x%02x", __FUNCTION__, i, (unsigned char)key.keyData[i]));
    }
    status = true;
    BDBG_MSG(("%s: successfull!!", __FUNCTION__));
error:
    return status;
}

bool
aes_decrypt_data(
    B_PlaybackIpAesCtx *securityCtx,
    unsigned char *read_ptr,
    int bytesRead,
    unsigned char *clear_buff,
    int *clear_buff_size
    )
{
    bool status = false;
    NEXUS_Error rc;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_DmaJobHandle dmaJob;
    NEXUS_DmaJobBlockSettings blockSettings;
    NEXUS_DmaJobStatus jobStatus;

    BDBG_MSG(("%s: buf enc %p, clear %p, length %d", __FUNCTION__, read_ptr, clear_buff, bytesRead));
    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks                   = 1;
    jobSettings.keySlot                     = securityCtx->encKeyHandle;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = NULL;
    dmaJob = NEXUS_DmaJob_Create(securityCtx->dmaHandle, &jobSettings);
    if (dmaJob == NULL) {
        BDBG_ERR(("%s: Failed to create Nexus DMA job", __FUNCTION__));
        goto error;
    }

    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
    blockSettings.pSrcAddr                  = read_ptr;
    blockSettings.pDestAddr                 = securityCtx->clearBuf;
    blockSettings.blockSize                 = bytesRead;
    blockSettings.resetCrypto               = true;
    blockSettings.scatterGatherCryptoStart  = true;
    blockSettings.scatterGatherCryptoEnd    = true;
    blockSettings.cached                    = true;
    rc = NEXUS_DmaJob_ProcessBlocks(dmaJob, &blockSettings, 1);
    BDBG_MSG(("%s: DMA job sumitted", __FUNCTION__));
    for(;;) {
        NEXUS_DmaJob_GetStatus(dmaJob, &jobStatus);
        if(jobStatus.currentState == NEXUS_DmaJobState_eComplete ) {
           break;
        }
        BKNI_Delay(1);
    }
    BDBG_MSG(("%s: DMA job completed", __FUNCTION__));

    NEXUS_DmaJob_Destroy(dmaJob);

    status = true;
    /* AES CBC uses the last 16 bytes of current block as the IV for the next block */
    BKNI_Memcpy(securityCtx->openSettings.security.settings.aes128.iv, &read_ptr[bytesRead-HTTP_AES_BLOCK_SIZE], HTTP_AES_BLOCK_SIZE);
    if (aes_setup_nexus_dma(securityCtx) != true) {
        BDBG_ERR(("%s: ERROR: failed to setup nexus dma", __FUNCTION__));
        goto error;
    }
    *clear_buff_size = bytesRead;
    BKNI_Memcpy(clear_buff, securityCtx->clearBuf, bytesRead);
error:
    return status;
}
#else /* HW_DEC */

bool
aes_setup_sw_dec(
    B_PlaybackIpAesCtx *securityCtx
    )
{
    if (AES_set_decrypt_key(
                securityCtx->openSettings.security.settings.aes128.key,
                sizeof(securityCtx->openSettings.security.settings.aes128.key)*8,
                &securityCtx->aesSwDecKey) < 0) {
        BDBG_ERR(("%s: Failed to set the AES key", __FUNCTION__));
        return false;
    }
    return true;
}
bool
aes_sw_decrypt_data(
    B_PlaybackIpAesCtx *securityCtx,
    unsigned char *read_ptr,
    int bytesRead,
    unsigned char *clear_buff,
    int *clear_buff_size
    )
{
    /* decrypt the block */
    AES_cbc_encrypt(read_ptr, clear_buff, (unsigned long)bytesRead, &securityCtx->aesSwDecKey, securityCtx->openSettings.security.settings.aes128.iv, AES_DECRYPT);
    /* AES CBC uses the last 16 bytes of current block as the IV for the next block */
    BKNI_Memcpy(securityCtx->openSettings.security.settings.aes128.iv, &read_ptr[bytesRead-HTTP_AES_BLOCK_SIZE], HTTP_AES_BLOCK_SIZE);
    *clear_buff_size = bytesRead;
    return true;
}
#endif

/*
 * This function tries to read the requested amount from the socket and returns any errors or bytes read.
 * It returns:
 *  =-1: for errors other than EINTR & EAGAIN during read call or when channel change occurs
 *  = 0: for EOF where server closed the TCP connection
 *  > 0: for success indicating number of bytes read from the socket
 */
int
aes_socket_read(void *voidHandle, B_PlaybackIpHandle playback_ip, int sd, char *rbuf, int rbuf_len)
{
    /* rbuf -> clearBuffer, rbuf_len -> clearBuffferLength */
    int bytesRead = 0;
    char *read_ptr;
    int rc = 0;
    bool serverClosed = false;
    B_PlaybackIpAesCtx *securityCtx = (B_PlaybackIpAesCtx *)voidHandle;
    int read_len = rbuf_len;
    int bytesToRead = 0;
    int bytesToCopy;
    unsigned int truncated_len = 0;
    char *orig_rbuf = NULL;
    char temp_bytes[HTTP_AES_BLOCK_SIZE];  /* Used for reading 16 bytes in case caller asked for less due to AES decryption block size requirement */

    if (!securityCtx)
        return -1;
#ifdef RECORD_CLEAR_DATA
    if(fclear == NULL)
        fclear = fopen("/data/videos/output.mpg", "wb+");
#endif
    /*
     * if use hardware decryption, single buffer is reading data from socket and
     * decryption use M2M, other wise double buffers are used.
     */
    if (rbuf_len > AES_ENCRYPT_BUFFER_SIZE ) {
        /* try to read & decrypt only upto AES_ENCRYPT_BUFFER_SIZE */
        rbuf_len = AES_ENCRYPT_BUFFER_SIZE;
    }
    read_len = rbuf_len;
    read_ptr = securityCtx->encryptBuf;
    /* Return extra decrypted data from previous read */
    if (securityCtx->extended_len && securityCtx->extended_len <= HTTP_AES_BLOCK_SIZE) {
        bytesToCopy = rbuf_len > securityCtx->extended_len ? securityCtx->extended_len : rbuf_len;
        if (bytesToCopy <= HTTP_AES_BLOCK_SIZE)
            memcpy(rbuf, securityCtx->extended_decrypted_bytes, bytesToCopy);
        bytesRead = bytesToCopy;
        securityCtx->extended_len -= bytesRead;
        if (securityCtx->extended_len != 0 && bytesToCopy < HTTP_AES_BLOCK_SIZE) {
            /* move the remaining bytes to the beginning */
            memmove(securityCtx->extended_decrypted_bytes, securityCtx->extended_decrypted_bytes + bytesToCopy, securityCtx->extended_len);
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
            rbuf_len += securityCtx->extended_len;
            orig_rbuf = rbuf;
            read_ptr = rbuf = temp_bytes;
        }
    }

    /* Copy previously read encrypted data (which was left over from the decryption due to PCP header being present) to begining of rbuf */
    /* Read length shouuld be 16 byte aligned */
    if (securityCtx->residual_encrypted_len && securityCtx->residual_encrypted_len <= HTTP_AES_BLOCK_SIZE ) {
        BDBG_MSG(("%s: Prepend previously read but not decrypted %d bytes, bytesRead so far %d", __FUNCTION__,  securityCtx->residual_encrypted_len, bytesRead));
        bytesToCopy = rbuf_len > securityCtx->residual_encrypted_len ? securityCtx->residual_encrypted_len : rbuf_len;
        if (bytesToCopy <= HTTP_AES_BLOCK_SIZE)
            memcpy(read_ptr+bytesRead, securityCtx->residual_encrypted_bytes, bytesToCopy);
        bytesRead += bytesToCopy;
        securityCtx->residual_encrypted_len -= bytesToCopy;
        if (securityCtx->residual_encrypted_len && bytesToCopy < HTTP_AES_BLOCK_SIZE)
            memmove(securityCtx->residual_encrypted_bytes, securityCtx->residual_encrypted_bytes + bytesToCopy, securityCtx->residual_encrypted_len);
    }

    /* Copy previously read encrypted data (which was not decrypted due to mod 16 requirements) to begining of rbuf */
    /* Read length shouuld be 16 byte aligned */
    if (securityCtx->partial_read_len && securityCtx->partial_read_len <= HTTP_AES_BLOCK_SIZE) {
        bytesToCopy = (rbuf_len-bytesRead) > securityCtx->partial_read_len ? securityCtx->partial_read_len : (rbuf_len-bytesRead);
        BDBG_MSG(("%s: Preprend partial read %d bytes, total partial read %d, bytesRead already read %d", __FUNCTION__, bytesToCopy, securityCtx->partial_read_len, bytesRead));
        if (bytesToCopy <= HTTP_AES_BLOCK_SIZE)
            /* coverity[overrun-local: FALSE] */
            memcpy(read_ptr+bytesRead, securityCtx->partial_read_bytes, bytesToCopy);
        bytesRead += bytesToCopy;
        securityCtx->partial_read_len -= bytesToCopy;
        if (securityCtx->partial_read_len > 0 && bytesToCopy < HTTP_AES_BLOCK_SIZE)
            /* move the remaining bytes to the beginning of the partial_read_bytes array */
            memmove(securityCtx->partial_read_bytes, securityCtx->partial_read_bytes+bytesToCopy, securityCtx->partial_read_len);
    }

    if (securityCtx->initialPayloadLength && securityCtx->initialPayloadLength <= TMP_BUF_SIZE) {
        /* Copy initially read encrypted data to begining of rbuf */
        bytesToCopy = (rbuf_len-bytesRead) > securityCtx->initialPayloadLength ? securityCtx->initialPayloadLength : (rbuf_len-bytesRead);
        BDBG_MSG(("%s: copying read %d bytes from initial buf length of %d into decryption buffer, bytesRead so far %d", __FUNCTION__,  bytesToCopy, securityCtx->initialPayloadLength, bytesRead));
        if (bytesToCopy < TMP_BUF_SIZE)
            memcpy(read_ptr+bytesRead, securityCtx->initialPayload, bytesToCopy);
        bytesRead += bytesToCopy;
        securityCtx->initialPayloadLength -= bytesToCopy;
        if (securityCtx->initialPayloadLength != 0 && bytesToCopy < TMP_BUF_SIZE)
            /* move the remaining bytes to the beginning of the intitialPayload */
            memmove(securityCtx->initialPayload, securityCtx->initialPayload + bytesToCopy, securityCtx->initialPayloadLength);
    }

    read_len = rbuf_len - bytesRead;
    bytesToRead = read_len;
    BDBG_MSG(("%s:%p now read remaining %d bytes from sockFd %d, total rbuf_len %d, bytesRead %d", __FUNCTION__, (void *)playback_ip, read_len, sd, rbuf_len, bytesRead));

    if (rbuf_len < 0 || read_len < 0 )
        BDBG_ASSERT(NULL);
    /* read remaining bytesToRead bytes from the socket */
    while (read_len != 0) {
        /* read_len will only be 0 when initialPayload had all the requested bytes, so we dont need to read any data from the socket */
        if ( breakFromLoop(playback_ip)) {
            /* user changed the channel, so return */
            BDBG_MSG(("%s:%p breaking out of read loop due to state (%d) change\n", __FUNCTION__, (void *)playback_ip, playbackIpState(playback_ip) ));
            return -1;
        }
        rc = read(sd, read_ptr + bytesRead, read_len);
        if (rc < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                if (bytesRead) {
                    BDBG_MSG(("%s: breaking from read loop due to partial data %d bytes recvd, bytesToRead %d", __FUNCTION__, bytesRead, bytesToRead));
                    break;
                }
                else {
                    BDBG_MSG(("%s: Read System Call interrupted or timed out (errno %d)\n", __FUNCTION__, errno));
                    return rc;  /* no data read return */
                }
            }
            BDBG_ERR(("%s: read ERROR:%d", __FUNCTION__, errno));
            return -1;
        }else if (rc == 0) {
            BDBG_MSG(("%s: Reached EOF, server closed the connection!\n", __FUNCTION__));
            serverClosed = true;
            break;
        } else if (rc < read_len) {
            bytesRead += rc;
            read_len -= rc;
            BDBG_MSG(("%s: Partial read rc %d, bytesRead so far %d, bytes remaining: read_len %d, rbuf_len %d, continue reading", __FUNCTION__, rc, bytesRead, read_len, rbuf_len));
            /* we sleep a little to allow more data to arrive from the server */
            BKNI_Sleep(100);
        } else {
            bytesRead += rc;
            BDBG_MSG(("%s: read all requested data: rc %d, bytesRead %d, read_len %d, rbuf_len %d", __FUNCTION__, rc, bytesRead, read_len, rbuf_len));
            break;
        }
    }

    /* if decryption is not yet enabled or is disabled, then simply return the read data */
    if (!securityCtx->openSettings.security.enableDecryption) {
        BDBG_MSG(("%s:%p: decryption is off: so memcopying the result of %d bytes from src %p to dst %p buffer", __FUNCTION__, (void *)playback_ip, bytesRead, (void *)read_ptr, rbuf));
        memcpy(rbuf, read_ptr, bytesRead);
        return bytesRead;
    }

    BDBG_MSG(("%s:%p Asked %d bytes Read =%d bytes\n", __FUNCTION__, (void *)playback_ip, rbuf_len-securityCtx->extended_len+truncated_len, bytesRead));

    /* Partial data receive.  Data is NOT aligned to 16 byte */
    if (bytesRead % HTTP_AES_BLOCK_SIZE) {
        if (serverClosed) {
            /* for blocking reads, only time we can receive non-16byte aligned data is when server server has closed */
            /* even then it didn't pad the data w/ 0 to make it 16 byte aligned, so we are ignoring those extra bytes */
            /* and thus truncate the encrypted buffer to mod16 size */
            bytesRead -= (bytesRead % HTTP_AES_BLOCK_SIZE);
            /* and we fall thru below unless we have nothing left */
            if (bytesRead <= 0)
                return 0;
        }
        else if (bytesRead > HTTP_AES_BLOCK_SIZE) {
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

#ifdef HW_DEC
    if (aes_decrypt_data(securityCtx, (unsigned char *)read_ptr, bytesRead, (unsigned char*)rbuf, &bytesRead) != true) {
        BDBG_ERR(("%s: AES decryption failed", __FUNCTION__));
        return 0;
    }
#else
    if (aes_sw_decrypt_data(securityCtx, (unsigned char *)read_ptr, bytesRead, (unsigned char*)rbuf, &bytesRead) != true) {
        BDBG_ERR(("%s: AES decryption failed", __FUNCTION__));
        return 0;
    }
#endif
    BDBG_MSG(("%s:%p: AES decryption successfull: clear bytes %d", __FUNCTION__, (void *)playback_ip, bytesRead));
#ifdef RECORD_CLEAR_DATA
        /* write data to file */
        fwrite(rbuf, 1, bytesRead , fclear);
        fflush(fclear);
#endif
    /* If we had to read more than requested due to AES block size requirement, then adjust length and copy data back to original buffer */
    if (securityCtx->extended_len) {
        BDBG_MSG(("%s: extended bytes saved %d, rbuf_len %d ", __FUNCTION__, securityCtx->extended_len, rbuf_len));
        rbuf_len -= securityCtx->extended_len;
        bytesRead -= securityCtx->extended_len;

        /* Save decrypted bytes */
        memcpy(temp_bytes, rbuf, bytesRead);
        /* coverity[overrun-buffer-arg: FALSE] */
        memcpy(securityCtx->extended_decrypted_bytes,rbuf+rbuf_len,securityCtx->extended_len);

        /* copy from temp_bytes back to original rbuf */
        rbuf = orig_rbuf;
        memcpy(rbuf, temp_bytes, bytesRead);
    }
    if (serverClosed) {
        /* server may have added some padding bytes to make the last block mod 16 aligned for AES, throw away those padding bytes */
        int remainingBytes;
        remainingBytes = bytesRead % 188;
        bytesRead -= (bytesRead % 188);
        /* coverity[var_deref_op: FALSE] */
        BDBG_MSG(("last rem bytes %d, bytes 0x%x, 0x%x, 0x%x, 0x%x",
                                remainingBytes, rbuf[bytesRead-remainingBytes], rbuf[bytesRead-remainingBytes+1], rbuf[bytesRead-remainingBytes+2], rbuf[bytesRead-remainingBytes+3]));
    }
#if 0
#define VERIFY_SYNC_BYTE
#endif
#ifdef VERIFY_SYNC_BYTE
#define TS_PKT_SIZE 188
#define SYNC_BYTE_OFFSET 0
    {
        static long long totalBytesRead = 0;
        static int nextSyncByteOffset = 0;
        int i=0;
        if (bytesRead < nextSyncByteOffset) {
            nextSyncByteOffset -= bytesRead;
            totalBytesRead += bytesRead;
        }
        else {
            for (i=nextSyncByteOffset; i<bytesRead; i+=TS_PKT_SIZE) {
                if (rbuf[i+SYNC_BYTE_OFFSET] != 0x47)
                    BDBG_WRN(("################ sync bytes mismatch at %lld, i %d, bytes 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
                                totalBytesRead+i+SYNC_BYTE_OFFSET, i+SYNC_BYTE_OFFSET, rbuf[i], rbuf[i+1], rbuf[i+2], rbuf[i+3], rbuf[i+4], rbuf[i+5], rbuf[i+6], rbuf[i+7]));
#if 0
                else
                    BDBG_MSG(("sync byte 0x%x, i %d, total %lld", rbuf[i+SYNC_BYTE_OFFSET], i+SYNC_BYTE_OFFSET, totalBytesRead));
#endif
            }
            totalBytesRead += bytesRead;
            if (!serverClosed)
                nextSyncByteOffset = ((TS_PKT_SIZE - (bytesRead % TS_PKT_SIZE)) + nextSyncByteOffset) % TS_PKT_SIZE;
            else {
                /* server closed, reset it for next session */
                nextSyncByteOffset = 0;
                totalBytesRead = 0;
            }
        }
    }
#endif
    return bytesRead;
}

int  B_PlaybackIp_AesDecryptionDisable(
    void *securityHandle
    )
{
    B_PlaybackIpAesCtx *securityCtx = (B_PlaybackIpAesCtx *)securityHandle;

    if (!securityCtx) {
        BDBG_ERR(("%s: invalid securityCtx %p", __FUNCTION__, (void *)securityCtx));
        return -1;
    }
    /* set flag to disable decryption */
    BDBG_MSG(("%s: Disabling AES Decryption\n", __FUNCTION__));
    securityCtx->openSettings.security.enableDecryption = false;

    return 0;
}

int B_PlaybackIp_AesDecryptionEnable(
    void *securityHandle,
    char *initialPayload,
    int initialPayloadLength)
{
    B_PlaybackIpAesCtx *securityCtx = (B_PlaybackIpAesCtx *)securityHandle;

    if (!securityCtx || (initialPayloadLength && !initialPayload)) {
        BDBG_ERR(("%s: invalid securityCtx %p or initial paylaod params (length %d, payload ptr %p)\n", __FUNCTION__, (void *)securityCtx, initialPayloadLength, (void *)initialPayload));
        return -1;
    }
    /* set flag to enable decryption */
    BDBG_MSG(("%s: Enabling AES Decryption: initial payload %d", __FUNCTION__, initialPayloadLength));
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
 * Un-Initialize session specific context of the security module
 * Called during B_PlaybackIp_SocketClose().
 */
void B_PlaybackIp_AesSessionClose(
    void *voidHandle)                                           /* input: security module specific handle */
{
    B_PlaybackIpAesCtx *securityCtx = (B_PlaybackIpAesCtx *)voidHandle;
    if (securityCtx)
    {
        BDBG_MSG(("%s:", __FUNCTION__));
        /* Free up Aes Handle & any other saved contexts */
#ifdef HW_DEC
        if (securityCtx->encKeyHandle) {
            NEXUS_Security_FreeKeySlot(securityCtx->encKeyHandle);
        }
#endif
        if(securityCtx->encryptBuf){
            B_PlaybackIp_UtilsFreeMemory(securityCtx->encryptBuf);
        }
        if(securityCtx->clearBuf){
            B_PlaybackIp_UtilsFreeMemory(securityCtx->clearBuf);
        }
        BKNI_Free(securityCtx);
    }
}

/*
 * Initialize session specific context of the security module
 * Called during B_PlaybackIp_SocketOpen().
 */
int B_PlaybackIp_AesSessionOpen(
    B_PlaybackIpSessionOpenSettings *openSettings,    /* input: provides server ip, port info */
    int sd,                                                     /* input: socket descriptor */
    B_PlaybackIpSecurityOpenOutputParams *securityOpenOutputParams) /* output: security settings return params */
{
    B_PlaybackIpAesCtx *securityCtx = NULL;
    B_PlaybackIpSecurityOpenSettings *securityOpenSettings;
#ifdef HW_DEC
    NEXUS_SecurityKeySlotSettings keySettings;
#endif

    BDBG_MSG(("%s:", __FUNCTION__));

    BSTD_UNUSED(sd);

    if (openSettings == NULL) {
        BDBG_ERR(("%s: Invalid parameters, Open Settings %p", __FUNCTION__, (void *)openSettings));
        goto error;
    }
    securityOpenSettings = &openSettings->security;
    securityOpenOutputParams->byteRangeOffset = 0;

    if (sd <= 0) {
        BDBG_ERR(("%s: invalid socket, sd = %d", __FUNCTION__, sd));
        goto error;
    }

    if (securityOpenSettings->securityProtocol != B_PlaybackIpSecurityProtocol_Aes128) {
        BDBG_ERR(("%s: invoking AES module with incorrect security protocol %d", __FUNCTION__, securityOpenSettings->securityProtocol));
        goto error;
    }

#ifdef HW_DEC
    if (securityOpenSettings->dmaHandle == NULL) {
        BDBG_ERR(("%s: App needs to provide M2M DMA Handle in the dmaHandle", __FUNCTION__));
        goto error;
    }
#endif

    securityCtx = BKNI_Malloc(sizeof(B_PlaybackIpAesCtx));
    if (!securityCtx) {
        BDBG_ERR(("%s:%d: memory allocation failure\n", __FUNCTION__, __LINE__));
        goto error;
    }
    memset(securityCtx, 0, sizeof(B_PlaybackIpAesCtx));
    /* copy keys & iv */
    memcpy(&securityCtx->openSettings, openSettings, sizeof(B_PlaybackIpSessionOpenSettings));

    /* allocate an encrypt buffer of a default size and then re-malloc if user is asking for larger data */
    securityCtx->encryptBuf = (char *)B_PlaybackIp_UtilsAllocateMemory(AES_ENCRYPT_BUFFER_SIZE, openSettings->heapHandle);
    if (!securityCtx->encryptBuf) {
        BDBG_ERR(("%s:%d: memory allocation failure\n", __FUNCTION__, __LINE__));
        goto error;
    }
    securityCtx->encryptBufSize = AES_ENCRYPT_BUFFER_SIZE;
    securityCtx->clearBuf = (char *)B_PlaybackIp_UtilsAllocateMemory(AES_ENCRYPT_BUFFER_SIZE, openSettings->heapHandle);
    if (!securityCtx->clearBuf) {
        BDBG_ERR(("%s:%d: memory allocation failure\n", __FUNCTION__, __LINE__));
        goto error;
    }
    securityCtx->clearBufSize = AES_ENCRYPT_BUFFER_SIZE;

#ifdef HW_DEC
    securityCtx->dmaHandle = securityOpenSettings->dmaHandle;
    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    securityCtx->encKeyHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
    if(!securityCtx->encKeyHandle) {
        BDBG_ERR(("%s: ERROR: Nexus Key Allocate Slot failed", __FUNCTION__));
        goto error;
    }
    if (aes_setup_nexus_dma(securityCtx) != true) {
        BDBG_ERR(("%s: ERROR: failed to setup nexus dma", __FUNCTION__));
        goto error;
    }
    BDBG_MSG(("%s: Successfully Allocated Nexus Key Slot: handle %p, encrypt Buffer %p", __FUNCTION__, (void *)securityCtx->encKeyHandle, (void *)securityCtx->encryptBuf));
#else
    if (aes_setup_sw_dec(securityCtx) != true ) {
        BDBG_ERR(("%s: ERROR: failed to setup sw dec key", __FUNCTION__));
        goto error;
    }
#endif

    securityOpenOutputParams->netIoPtr->read = aes_socket_read;
    securityOpenOutputParams->netIoPtr->close = B_PlaybackIp_AesSessionClose;
    securityOpenOutputParams->netIoPtr->suspend = NULL;
    *securityOpenOutputParams->securityHandle = (void *)securityCtx;
    BDBG_MSG(("%s: done", __FUNCTION__));
    return 0;
error:
    if (securityCtx)
        B_PlaybackIp_AesSessionClose(securityCtx);
    return -1;
}

static int gAesInitRefCnt = 0;
/*
 * Initialize global context of the security module
 * Called during B_PlaybackIp_Open().
 */
void * B_PlaybackIp_AesInit(void *nexusDmaHandle)
{
    BSTD_UNUSED(nexusDmaHandle);
    if (!gAesInitRefCnt) {
        BDBG_MSG(("%s: done\n", __FUNCTION__));
    }
    gAesInitRefCnt++;
    return NULL;
}

/*
 * Un-Initialize global context of the security module
 * Called during B_PlaybackIp_Close().
 */
void B_PlaybackIp_AesUnInit(void * ctx)
{

    BSTD_UNUSED(ctx);
    gAesInitRefCnt--;
    if (!gAesInitRefCnt) {
        /* Global system un-initialization for Aes library */
        BDBG_MSG(("%s: done\n", __FUNCTION__));
    }
}

#endif /* LINUX || VxWorks */
