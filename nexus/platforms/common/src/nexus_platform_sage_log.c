/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

 ******************************************************************************/
#include "nexus_platform_module.h"
#include "nexus_platform_sage_log.h"
#if NEXUS_HAS_SAGE
#include "nexus_sage.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

BDBG_MODULE(nexus_platform_sage_log);

static bool g_sageLogThreadDone;
static pthread_t g_sageLogThread;

#define SAGE_LOG_PATH_MAX 128
#define SAGE_LOG_BUFFER_SIZE ((16*1024) + 16) /*+16 for 16byte alignment for AES ENC*/
#define SAGE_B64_LOG_BUFFER_SIZE 21*1024 /*(4*SAGE_LOG_BUFFER_SIZE)/3 ceiled to next 1K*/
#define SAGE_RSA_2048_KEY_SIZE 256

static void NEXUS_Platform_P_Hex2Base64_EncodeBlock(unsigned char *in, unsigned char *out, int len)
{
    const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    out[0] = (unsigned char) cb64[(int)(in[0] >> 2)];
    out[1] = (unsigned char) cb64[(int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4))];
    out[2] = (unsigned char) (len > 1 ? cb64[(int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6))] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[(int)(in[2] & 0x3f)] : '=');
    return;
}

static void NEXUS_Platform_P_Hex2Base64(uint8_t *inp, uint32_t inplen, uint8_t *b64out, uint32_t *b64len)
{
    unsigned char in[3]={0};
    unsigned char out[4]={0};
    uint32_t i, len,j = 0,k=0;

    while(j < inplen) {
        len = 0;
        for(i = 0; i < 3; i++) {
            in[i] = inp[j+i];

            if( (i+j) < inplen) {
                len++;
            }
            else {
                in[i] = (unsigned char) 0;
            }
        }
        if (len > 0) {
            NEXUS_Platform_P_Hex2Base64_EncodeBlock(in, out, len);
            for(i = 0; i < 4; i++) {
                b64out[k+i] = out[i];
            }
            k += 4;
        }
        j += 3;
    }
    *b64len = k;
    return;
}

static void *NEXUS_Platform_P_SageLogThread(void *pParam)
{
    const char *pEnv = "sage_log";
    NEXUS_Error errCode;
    FILE *pFile=NULL;
    char pathname[SAGE_LOG_PATH_MAX];
    char header[128];
    uint8_t *pLogBuffer=NULL;
    uint32_t bufferSize=0;
    uint8_t *pWrapLogBuffer=NULL;
    uint32_t wrapBufSize=0;
    uint8_t keyBuffer[256];
    uint32_t keySize=0;
    uint8_t *pBase64Buffer=NULL;
    uint32_t base64Outlen;
    uint32_t actualBufferSize=0,actualWrapBufSize=0;

    NEXUS_MemoryAllocationSettings allocSettings;

    BSTD_UNUSED(pParam);

    snprintf(pathname, sizeof(pathname), "%s.bin", pEnv);
    pFile = fopen(pathname, "wb+");
    if ( NULL == pFile )
    {
        goto exit;
    }

    /* alloc from heap just in case app is expecting it. */
    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    allocSettings.alignment = 16;
    errCode = NEXUS_Memory_Allocate(SAGE_LOG_BUFFER_SIZE, &allocSettings, (void **)&pLogBuffer);
    if ( errCode != NEXUS_SUCCESS )
    {
        BDBG_ERR(("NEXUS_Memory_Allocate Failed"));
        goto exit;
    }
    errCode = NEXUS_Memory_Allocate (SAGE_B64_LOG_BUFFER_SIZE, &allocSettings, (void **)&pBase64Buffer);
    if ( errCode != NEXUS_SUCCESS )
    {
        BDBG_ERR(("NEXUS_Memory_Allocate Failed"));
        goto exit;
    }

    while ( false == g_sageLogThreadDone )
    {
        BKNI_Sleep(2000);
        errCode = NEXUS_Sage_GetEncKey(keyBuffer, SAGE_RSA_2048_KEY_SIZE, &keySize);
        if ( errCode != NEXUS_SUCCESS )
        {
            break;
        }
        errCode = NEXUS_Sage_GetLogBuffer(pLogBuffer, SAGE_LOG_BUFFER_SIZE, &bufferSize, &wrapBufSize, &actualBufferSize, &actualWrapBufSize);
        if ( errCode != NEXUS_SUCCESS )
        {
            break;
        }

        bufferSize -= wrapBufSize; /*buffsize = bufflen+wrapbuflen. Syncthunk will do copytouser only pOutbufsize.*/

        BDBG_MSG(("bufSize %d, wrapBufSize %d, actBufSize %d, actWrapBufSize %d",
                        bufferSize, wrapBufSize, actualBufferSize, actualWrapBufSize));

        NEXUS_Platform_P_Hex2Base64(keyBuffer, keySize, pBase64Buffer, &base64Outlen);

        snprintf(header, sizeof(header), "#KB:%04x:%04x:%04x:", keySize, keySize,base64Outlen);
        if((keySize != 0)&&(bufferSize != 0))/*Do not write just key data only*/
        {
            (void)fwrite(header, strlen(header), 1, pFile);
            (void)fwrite(pBase64Buffer, base64Outlen, 1, pFile);
        }

        NEXUS_Platform_P_Hex2Base64(pLogBuffer, bufferSize, pBase64Buffer, &base64Outlen);

        snprintf(header, sizeof(header), "#LB:%04x:%04x:%04x:",actualBufferSize,bufferSize,base64Outlen);
        if(bufferSize != 0)
        {
            (void)fwrite(header, strlen(header), 1, pFile);
            (void)fwrite(pBase64Buffer, base64Outlen, 1, pFile);
        }
        if(wrapBufSize != 0)
        {
            pWrapLogBuffer = &pLogBuffer[bufferSize];

            NEXUS_Platform_P_Hex2Base64(keyBuffer, keySize, pBase64Buffer, &base64Outlen);

            snprintf(header, sizeof(header), "#KB:%04x:%04x:%04x:", keySize, keySize,base64Outlen);
            if((keySize != 0)&&(bufferSize != 0))/*Do not write just key data only*/
            {
                (void)fwrite(header, strlen(header), 1, pFile);
                (void)fwrite(pBase64Buffer, base64Outlen, 1, pFile);
            }

            NEXUS_Platform_P_Hex2Base64(pWrapLogBuffer, wrapBufSize, pBase64Buffer, &base64Outlen);

            snprintf(header, sizeof(header), "#LB:%04x:%04x:%04x:", actualWrapBufSize, wrapBufSize, base64Outlen);

            (void)fwrite(header, strlen(header), 1, pFile);
            (void)fwrite(pBase64Buffer, base64Outlen, 1, pFile);
        }
        fflush(pFile);
    }

exit:
    if(pBase64Buffer != NULL)
    {
        NEXUS_Memory_Free(pBase64Buffer);
    }
    if(pLogBuffer != NULL)
    {
        NEXUS_Memory_Free(pLogBuffer);
    }
    if(pFile != NULL)
    {
        fclose(pFile);
        pFile = NULL;
    }

    pthread_exit(NULL);
    return NULL;
}

#define UINT32_SWAP(value) ( ((value) << 24) | (((value) << 8) & 0x00ff0000) | (((value) >> 8) & 0x0000ff00) | ((value) >> 24) )

#define UINT32_TO_BE(value) UINT32_SWAP(value)
#define BE_TO_UINT32(value) UINT32_SWAP(value)

static bool g_sageSecureLogThreadDone;
static pthread_t g_sageSecureLogThread;

#define SAGE_SECURE_LOG_PATH_MAX 128
static void *NEXUS_Platform_P_SageSecureLogThread(void *pParam)
{
    const char *pEnv = "secure_log";
    NEXUS_Error errCode;
    FILE *pFile=NULL;
    char pathname[SAGE_SECURE_LOG_PATH_MAX];
    NEXUS_Sage_Secure_Log_TlBufferContext *pBuffContext = NULL;
    uint8_t *pLogBuffAddr = NULL;
    uint32_t logBuffCtxSize = sizeof(NEXUS_Sage_Secure_Log_TlBufferContext);
    uint32_t logBufferSize = SAGE_SECURE_LOG_BUFFER_SIZE;

    NEXUS_MemoryAllocationSettings allocSettings;

    BSTD_UNUSED(pParam);

    if ( NEXUS_SUCCESS != NEXUS_Sage_SecureLog_StartCaptureOK() )
    {
        BDBG_MSG(("%s %d secure log not enabled",BSTD_FUNCTION,__LINE__));
        goto exit;
    }

    snprintf(pathname, sizeof(pathname), "%s.bin", pEnv);
    pFile = fopen(pathname, "wb+");
    if ( NULL == pFile )
    {
        BDBG_ERR(("%s %d file %s open Failed",BSTD_FUNCTION,__LINE__,pathname));
        goto exit;
    }

    /* alloc from heap just in case app is expecting it. */
    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    allocSettings.alignment = 4096;
    errCode = NEXUS_Memory_Allocate(logBufferSize, &allocSettings, (void **)&pLogBuffAddr);
    if ( errCode != NEXUS_SUCCESS )
    {
        BDBG_ERR(("NEXUS_Memory_Allocate Failed"));
        goto exit;
    }
    errCode = NEXUS_Memory_Allocate (logBuffCtxSize, &allocSettings, (void **)&pBuffContext);
    if ( errCode != NEXUS_SUCCESS )
    {
        BDBG_ERR(("NEXUS_Memory_Allocate Failed"));
        goto exit;
    }

    while ( false == g_sageSecureLogThreadDone )
    {
        BKNI_Sleep(1000);
        BKNI_Memset(pBuffContext,0,logBuffCtxSize);
        BKNI_Memset(pLogBuffAddr,0,logBufferSize);
        errCode = NEXUS_Sage_SecureLog_GetBuffer(
                                                (uint8_t *)pBuffContext,logBuffCtxSize,
                                                pLogBuffAddr,    logBufferSize,
                                                NEXUS_Sage_SecureLog_BufferId_eFirstAvailable);

        if(errCode == NEXUS_SUCCESS)
        {

            if(pBuffContext->secHead.secure_logtotal_cnt > 0)
            {
                uint32_t buflen,datalen;

                buflen = BE_TO_UINT32(pBuffContext->secHead.secure_logbuf_len);
                datalen = BE_TO_UINT32(pBuffContext->secHead.secure_logtotal_cnt);

                if( buflen >  datalen + 15)
                {
                    buflen = (datalen/16 + 1)*16;
                }

                BDBG_MSG(("buflen 0x%x(<0x%x) datalen 0x%x",buflen,logBufferSize,datalen));

                pBuffContext->secHead.secure_logbuf_len = UINT32_TO_BE(buflen);

                fwrite((uint8_t*)pBuffContext ,sizeof(*pBuffContext),1,pFile);
                fwrite((uint8_t*)pLogBuffAddr,buflen,1,pFile);
                fflush(pFile);
            }
        }
    }

exit:
    if(pLogBuffAddr != NULL)
    {
        NEXUS_Memory_Free(pLogBuffAddr);
    }
    if(pBuffContext != NULL)
    {
        NEXUS_Memory_Free(pBuffContext);
    }
    if(pFile != NULL)
    {
        fclose(pFile);
        pFile = NULL;
    }

    pthread_exit(NULL);
    return NULL;
}
#endif

NEXUS_Error NEXUS_Platform_P_InitSageLog(void)
{
#if NEXUS_HAS_SAGE
    if ( NEXUS_GetEnv("sage_log_file") )
    {
        if ( pthread_create(&g_sageLogThread, NULL, NEXUS_Platform_P_SageLogThread, NULL) )
        {
            return BERR_TRACE(BERR_OS_ERROR);
        }
    }
    if ( pthread_create(&g_sageSecureLogThread, NULL, NEXUS_Platform_P_SageSecureLogThread, NULL) )
    {
        return BERR_TRACE(BERR_OS_ERROR);
    }
#endif
    return BERR_SUCCESS;
}

void NEXUS_Platform_P_UninitSageLog(void)
{
#if NEXUS_HAS_SAGE
    if ( (pthread_t) NULL != g_sageLogThread )
    {
        g_sageLogThreadDone = true;
        pthread_join(g_sageLogThread, NULL);
        g_sageLogThread = (pthread_t)NULL;
        g_sageLogThreadDone = false;
    }
    if ( (pthread_t) NULL != g_sageSecureLogThread )
    {
        g_sageSecureLogThreadDone = true;
        pthread_join(g_sageSecureLogThread, NULL);
        g_sageSecureLogThread = (pthread_t)NULL;
        g_sageSecureLogThreadDone = false;
    }
#endif
}
