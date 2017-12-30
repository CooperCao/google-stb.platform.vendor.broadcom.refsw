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

#include "bstd.h"
#include "bkni.h"
#include "bhsm.h"
#include "nexus_security_module.h"
#include "nexus_base.h"
#include "nexus_types.h"
#include "nexus_security_common.h"
#include "nexus_hash.h"
#include "priv/nexus_hash_priv.h"
#include "bhsm_hash.h"
#include "bhsm_hmac.h"
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_hash);

#define NEXUS_HASH_BUFFER_SIZE (1024*1024)
#define NEXUS_HASH_BUFFER_LEFTOVER_SIZE (64)

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(5,2)
  #define _REVERSE_DATA_BUFFER  0
#else
  #define _REVERSE_DATA_BUFFER  1
#endif

#if _REVERSE_DATA_BUFFER
#define NEXUS_HASH_INTERNAL_BUFFER_SIZE (NEXUS_HASH_BUFFER_SIZE + NEXUS_HASH_BUFFER_LEFTOVER_SIZE)
#else
#define NEXUS_HASH_INTERNAL_BUFFER_SIZE (NEXUS_HASH_BUFFER_LEFTOVER_SIZE)
#endif

typedef struct NEXUS_P_HashHmacQueue {
#if _REVERSE_DATA_BUFFER
    uint8_t* swapBuffer;
#endif
    uint8_t* leftOverBuffer;
    uint32_t leftOverBufferSize;
}NEXUS_P_HashHmacQueue;

NEXUS_HashHmacQueue * NEXUS_HashHmac_QueueCreate(void)
{
    NEXUS_MemoryAllocationSettings allocSettings;
    NEXUS_P_HashHmacQueue *pQueue = NULL ;

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    allocSettings.alignment = 64;
    allocSettings.heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    NEXUS_Memory_Allocate(NEXUS_HASH_INTERNAL_BUFFER_SIZE + sizeof(NEXUS_P_HashHmacQueue), &allocSettings, (void**)&pQueue);
    if (pQueue == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        return NULL;
    }
    pQueue->leftOverBuffer = (uint8_t*)pQueue + sizeof(NEXUS_P_HashHmacQueue);
#if _REVERSE_DATA_BUFFER
    pQueue->swapBuffer = &pQueue->leftOverBuffer[NEXUS_HASH_BUFFER_LEFTOVER_SIZE];
#endif
    pQueue->leftOverBufferSize = 0;

    return (NEXUS_HashHmacQueue*)pQueue;
}

void NEXUS_HashHmac_QueueDestroy(NEXUS_HashHmacQueue *pHashQueue)
{
    NEXUS_P_HashHmacQueue *pQueue = (NEXUS_P_HashHmacQueue *)pHashQueue ;

    if(pQueue == NULL) { BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }

    NEXUS_Memory_Free(pQueue);
}

void NEXUS_HashHmac_ResetBuffer(NEXUS_HashHmacQueue *pHashQueue)
{
    NEXUS_P_HashHmacQueue *pQueue = (NEXUS_P_HashHmacQueue *)pHashQueue ;

    if(pQueue == NULL) { BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }

    pQueue->leftOverBufferSize = 0;
}


static void NEXUS_P_Mem32Swap( uint8_t* pDest, const uint8_t* pSrc, unsigned byteSize )
{
    unsigned wordSize = byteSize/4;
    uint32_t *pS = (uint32_t*)pSrc;
    uint32_t *pD = (uint32_t*)pDest;
    unsigned i;

    if( byteSize % 4) { BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }
    if( wordSize == 0) { BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }

    for( i = 0; i < wordSize; i++ )
    {
#if _REVERSE_DATA_BUFFER
        *pD =  ( *pS & 0x000000FF ) << 24 |
               ( *pS & 0x0000FF00 ) << 8  |
               ( *pS & 0x00FF0000 ) >> 8  |
               ( *pS & 0xFF000000 ) >> 24;
#else
        if(pD == pS)
        {
            break;
        }
        *pD = *pS;
#endif
        pD++; pS++;
    }
}


NEXUS_Error NEXUS_HashHmac_QueueUpData(NEXUS_HashHmac_QueueUp *pQueueUp)
{
    size_t tosend, sendsize;
    uint8_t *src = NULL;
    NEXUS_Addr bufferOffset = 0;
    BERR_Code hsmRc;
    BHSM_HashSubmitData  hashData;
    BHSM_HmacSubmitData  hmacData;
    NEXUS_P_HashHmacQueue *pQueue = NULL;

    if(pQueueUp == NULL) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    pQueue = (NEXUS_P_HashHmacQueue *)pQueueUp->pQueue;
    BKNI_Memset(&hashData, 0, sizeof(hashData));
    BKNI_Memset(&hmacData, 0, sizeof(hmacData));

    if(pQueueUp->dataSize)
    {
        src = (uint8_t*) NEXUS_OffsetToCachedAddr(pQueueUp->dataOffset);
        /* all the data to be sent */
        sendsize  = pQueueUp->dataSize + pQueue->leftOverBufferSize;

        /* if leftOverBuffer contain some data, we have to fill it */
        if( pQueue->leftOverBufferSize != 0)
        {
            /* fill it, if you can */
            if(pQueue->leftOverBufferSize != NEXUS_HASH_BUFFER_LEFTOVER_SIZE )
            {
                uint32_t leftOverBufferSize =  (sendsize <= NEXUS_HASH_BUFFER_LEFTOVER_SIZE) ?  sendsize : NEXUS_HASH_BUFFER_LEFTOVER_SIZE;
                uint32_t bytes2copy = leftOverBufferSize - pQueue->leftOverBufferSize;
                if(bytes2copy > (NEXUS_HASH_BUFFER_LEFTOVER_SIZE - pQueue->leftOverBufferSize)) {return BERR_TRACE(BERR_INVALID_PARAMETER);}
                BKNI_Memcpy(&pQueue->leftOverBuffer[pQueue->leftOverBufferSize], src, bytes2copy);

                src += bytes2copy;
                pQueue->leftOverBufferSize = leftOverBufferSize;

                /*  exit if all the data are used. we always some data are kept*/
                if(sendsize == pQueue->leftOverBufferSize)
                {
                    goto finish;
                }
            }

            /* we can send a full leftOverBuffer, we are sure there will be further data */
            sendsize -= NEXUS_HASH_BUFFER_LEFTOVER_SIZE;
            NEXUS_P_Mem32Swap(pQueue->leftOverBuffer, pQueue->leftOverBuffer, NEXUS_HASH_BUFFER_LEFTOVER_SIZE);
            bufferOffset = NEXUS_AddrToOffset(pQueue->leftOverBuffer);
            NEXUS_Memory_FlushCache( pQueue->leftOverBuffer, NEXUS_HASH_BUFFER_LEFTOVER_SIZE );
            if(pQueueUp->hashNotHmac)
            {
                hashData.dataOffset = bufferOffset;
                hashData.dataSize = NEXUS_HASH_BUFFER_LEFTOVER_SIZE;
                hashData.last = false;
                hsmRc = BHSM_Hash_SubmitData(pQueueUp->hHsmHandle, &hashData);
            }
            else
            {
                hmacData.dataOffset = bufferOffset;
                hmacData.dataSize = NEXUS_HASH_BUFFER_LEFTOVER_SIZE;
                hmacData.last = false;
                hsmRc = BHSM_Hmac_SubmitData(pQueueUp->hHsmHandle, &hmacData);
            }
            if (hsmRc)
            {
                return BERR_TRACE(hsmRc);
            }
            pQueue->leftOverBufferSize = 0;
        }

        /* sent all the data in multiple of 64 of the big buffer (keeping always a bit) */
        tosend = sendsize;
        if (tosend % 64)
        {
            tosend = (tosend/64)*64;
        }
        else
        {
            tosend -= 64;
        }

#if _REVERSE_DATA_BUFFER
        {
            size_t sent = 0, chunck = 0;

            bufferOffset = NEXUS_AddrToOffset(pQueue->swapBuffer);
            while (sent < tosend)
            {

                chunck = (tosend-sent);
                if (chunck > NEXUS_HASH_BUFFER_SIZE)
                {
                    chunck = NEXUS_HASH_BUFFER_SIZE;
                }

                NEXUS_P_Mem32Swap(pQueue->swapBuffer, src, chunck);

                /* Send the data */
                NEXUS_Memory_FlushCache( pQueue->swapBuffer, chunck );

                if(pQueueUp->hashNotHmac)
                {
                    hashData.dataOffset = bufferOffset;
                    hashData.dataSize = chunck;
                    hashData.last = false;
                    hsmRc = BHSM_Hash_SubmitData(pQueueUp->hHsmHandle, &hashData);
                }
                else
                {
                    hmacData.dataOffset = bufferOffset;
                    hmacData.dataSize = chunck;
                    hmacData.last = false;
                    hsmRc = BHSM_Hmac_SubmitData(pQueueUp->hHsmHandle, &hmacData);
                }
                if (hsmRc)
                {
                    return BERR_TRACE(hsmRc);
                }
                src += chunck;
                sent += chunck;
            }
        }
#else
        bufferOffset = NEXUS_AddrToOffset(src);
        NEXUS_Memory_FlushCache( src, tosend );

        if(pQueueUp->hashNotHmac)
        {
            hashData.dataOffset = bufferOffset;
            hashData.dataSize = tosend;
            hashData.last = false;
            hsmRc = BHSM_Hash_SubmitData(pQueueUp->hHsmHandle, &hashData);
        }
        else
        {
            hmacData.dataOffset = bufferOffset;
            hmacData.dataSize = tosend;
            hmacData.last = false;
            hsmRc = BHSM_Hmac_SubmitData(pQueueUp->hHsmHandle, &hmacData);
        }

        if (hsmRc)
        {
            return BERR_TRACE(hsmRc);
        }
        src += tosend;
#endif
        /* copy the leftover in the small buffer*/
        if((sendsize - tosend) > NEXUS_HASH_BUFFER_LEFTOVER_SIZE) {return BERR_TRACE(BERR_INVALID_PARAMETER);}
        BKNI_Memcpy( pQueue->leftOverBuffer, src, sendsize - tosend);
        pQueue->leftOverBufferSize = sendsize - tosend;
    }

    finish:

    if(pQueueUp->last)
    {
        size_t swapSize = pQueue->leftOverBufferSize;
        if(swapSize % 4)
        {
            swapSize = ((swapSize/4) + 1)*4;
        }
        NEXUS_P_Mem32Swap(pQueue->leftOverBuffer, pQueue->leftOverBuffer, swapSize);
        NEXUS_Memory_FlushCache( pQueue->leftOverBuffer, swapSize);

#if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(5,0)
        if(pQueue->leftOverBufferSize == 1)
        {
            if(pQueueUp->hashNotHmac)
            {
                BHSM_HashSubmitData_1char oneCharHashData;
                BKNI_Memset(&oneCharHashData, 0, sizeof(oneCharHashData));
                oneCharHashData.data = pQueue->leftOverBuffer[3];
                hsmRc = BHSM_Hash_SubmitData_1char( pQueueUp->hHsmHandle, &oneCharHashData);
                if (hsmRc)
                {
                    return BERR_TRACE(hsmRc);
                }
                if(oneCharHashData.hashLength > sizeof(pQueueUp->digest)) {return BERR_TRACE(BERR_INVALID_PARAMETER);}
                BKNI_Memcpy(&pQueueUp->digest, oneCharHashData.hash, oneCharHashData.hashLength);
                pQueueUp->digestLength = oneCharHashData.hashLength;
            }
            else
            {
                BHSM_HmacSubmitData_1char oneCharHmacData;
                BKNI_Memset(&oneCharHmacData, 0, sizeof(oneCharHmacData));
                oneCharHmacData.data = pQueue->leftOverBuffer[3];
                hsmRc = BHSM_Hmac_SubmitData_1char( pQueueUp->hHsmHandle, &oneCharHmacData);
                if (hsmRc)
                {
                    return BERR_TRACE(hsmRc);
                }
                if(oneCharHmacData.hmacLength > sizeof(pQueueUp->digest)) {return BERR_TRACE(BERR_INVALID_PARAMETER);}
                BKNI_Memcpy(&pQueueUp->digest, oneCharHmacData.hmac, oneCharHmacData.hmacLength);
                pQueueUp->digestLength = oneCharHmacData.hmacLength;
            }
        }
        else
#endif
        {
            bufferOffset = NEXUS_AddrToOffset(pQueue->leftOverBuffer);
            if(pQueueUp->hashNotHmac)
            {
                hashData.dataOffset = bufferOffset;
                hashData.dataSize = pQueue->leftOverBufferSize;
                hashData.last = true;
                hsmRc = BHSM_Hash_SubmitData(pQueueUp->hHsmHandle, &hashData);
                if (hsmRc)
                {
                    return BERR_TRACE(hsmRc);
                }
                if(hashData.hashLength > sizeof(pQueueUp->digest)) {return BERR_TRACE(BERR_INVALID_PARAMETER);}
                BKNI_Memcpy(&pQueueUp->digest, hashData.hash, hashData.hashLength);
                pQueueUp->digestLength = hashData.hashLength;
            }
            else
            {
                hmacData.dataOffset = bufferOffset;
                hmacData.dataSize = pQueue->leftOverBufferSize;
                hmacData.last = true;
                hsmRc = BHSM_Hmac_SubmitData( pQueueUp->hHsmHandle, &hmacData );
                if (hsmRc)
                {
                    return BERR_TRACE(hsmRc);
                }
                if(hmacData.hmacLength > sizeof(pQueueUp->digest)) {return BERR_TRACE(BERR_INVALID_PARAMETER);}
                BKNI_Memcpy(&pQueueUp->digest, hmacData.hmac, hmacData.hmacLength);
                pQueueUp->digestLength = hmacData.hmacLength;
            }
        }
        pQueue->leftOverBufferSize = 0;
    }
    return NEXUS_SUCCESS;
}
