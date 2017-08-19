/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bfile_crypto.h"

#if NEXUS_HAS_SECURITY && NEXUS_SECURITY_API_VERSION==1
#include "nexus_platform_client.h"
#include "nexus_dma.h"
#include "nexus_security.h"
#include "nexus_memory.h"
#include "bfile_io.h"

BDBG_MODULE(bfile_crypto);

struct bfile_crypto {
    struct bfile_io_read file; /* shall be the first member */
    bfile_io_read_t fd;
    off_t pos;
    NEXUS_DmaHandle dma;
    BKNI_EventHandle dmaEvent;
    NEXUS_DmaJobBlockSettings blockSettings;
    NEXUS_DmaJobHandle dmaJob;
    NEXUS_KeySlotHandle decKeyHandle;
    unsigned packetsize;
    unsigned packets;
    uint8_t *packet;
};

static void CompleteCallback(void *pParam, int iParam)
{
    BSTD_UNUSED(iParam);
    /*printf("CompleteCallback:%#lx fired\n", (unsigned long)pParam);*/
    BKNI_SetEvent(pParam);
}

static int
dmaSetup(bfile_crypto_t file, NEXUS_SecurityAlgorithm algo, const unsigned char *keyData, unsigned size)
{
    NEXUS_SecurityKeySlotSettings keySettings;
    NEXUS_SecurityAlgorithmSettings algoSettings;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_SecurityClearKey key;
    int rc;

    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    keySettings.keySlotType = NEXUS_SecurityKeySlotType_eType2;
    file->decKeyHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
    if (!file->decKeyHandle) {
        rc = BERR_TRACE(-1);
        goto err_alloc;
    }

    /*printf("Allocate dec keyslot successfully\n");*/

    NEXUS_Security_GetDefaultAlgorithmSettings(&algoSettings);

    algoSettings.algorithm           = algo;
    algoSettings.algorithmVar        = NEXUS_SecurityAlgorithmVariant_eEcb;
    algoSettings.operation           = NEXUS_SecurityOperation_eDecrypt;
    algoSettings.keyDestEntryType    = NEXUS_SecurityKeyType_eOdd;
    algoSettings.dest                = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
    algoSettings.modifyScValue[NEXUS_SecurityPacketType_eRestricted] = true;
    algoSettings.modifyScValue[NEXUS_SecurityPacketType_eGlobal]     = true;
    algoSettings.scValue[NEXUS_SecurityPacketType_eRestricted] = NEXUS_SecurityAlgorithmScPolarity_eClear;
    algoSettings.scValue[NEXUS_SecurityPacketType_eGlobal] = NEXUS_SecurityAlgorithmScPolarity_eClear;

    rc = NEXUS_Security_ConfigAlgorithm(file->decKeyHandle, &algoSettings);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto err_config;
    }

    /*printf("ConfigAlg enc keyslot Success\n");*/

    NEXUS_Security_GetDefaultClearKey(&key);
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    key.dest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
    key.keySize = size;
    BKNI_Memcpy(key.keyData, keyData, size);
    rc = NEXUS_Security_LoadClearKey(file->decKeyHandle, &key);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto err_load;
    }

    /* Open DMA handle */
    file->dma = NEXUS_Dma_Open(0, NULL);
    if (!file->dma) {
        rc = BERR_TRACE(-1);
        goto err_opendma;
    }

    /* and DMA event */
    rc = BKNI_CreateEvent(&file->dmaEvent);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto err_createevent;
    }

    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks                   = 1;
    jobSettings.keySlot                     = file->decKeyHandle;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eMpeg;
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context  = file->dmaEvent;
    file->dmaJob = NEXUS_DmaJob_Create(file->dma, &jobSettings);
    if (!file->dmaJob) {
        rc = BERR_TRACE(-1);
        goto err_createdmajob;
    }

    NEXUS_DmaJob_GetDefaultBlockSettings(&file->blockSettings);
    file->blockSettings.resetCrypto               = true;
    file->blockSettings.scatterGatherCryptoStart  = true;
    file->blockSettings.scatterGatherCryptoEnd    = true;
    file->blockSettings.cached                    = true;
    return 0;

err_createdmajob:
    BKNI_DestroyEvent(file->dmaEvent);
err_createevent:
    NEXUS_Dma_Close(file->dma);
err_opendma:
err_load:
err_config:
    NEXUS_Security_FreeKeySlot(file->decKeyHandle);
err_alloc:
    return rc;
}

static void
dmaDestroy(bfile_crypto_t file)
{
    NEXUS_Security_FreeKeySlot(file->decKeyHandle);
    NEXUS_DmaJob_Destroy(file->dmaJob);
    NEXUS_Dma_Close(file->dma);
    BKNI_DestroyEvent(file->dmaEvent);
}

static void
decrypt(bfile_crypto_t file, void *pSrc, unsigned size)
{
    NEXUS_Error rc;
    NEXUS_DmaJobStatus jobStatus;

    file->blockSettings.pSrcAddr                  = pSrc;
    file->blockSettings.pDestAddr                 = pSrc;
    file->blockSettings.blockSize                 = size;

    rc = NEXUS_DmaJob_ProcessBlocks(file->dmaJob, &file->blockSettings, 1);
    if (rc == NEXUS_DMA_QUEUED )
    {
        rc = BKNI_WaitForEvent(file->dmaEvent, BKNI_INFINITE);
        BDBG_ASSERT(!rc);
        NEXUS_DmaJob_GetStatus(file->dmaJob, &jobStatus);
        BDBG_ASSERT(jobStatus.currentState == NEXUS_DmaJobState_eComplete);
    }
    else {
        BDBG_ASSERT(!rc);
    }
}

static ssize_t
b_file_crypto_read(bfile_io_read_t fd, void *buf, size_t length)
{
    bfile_crypto_t file=(void *)fd;
    BDBG_MSG(("crypto_read(%d)@"BDBG_UINT64_FMT, (unsigned)length, BDBG_UINT64_ARG(file->pos)));
    if(file->fd) {
        unsigned r = file->pos % file->packetsize;
        uint8_t *ucbuf;
        uint8_t *uclim;
        off_t pos;
        unsigned wp;
        unsigned skipcopy;
        pos = file->pos - r;
        wp = (r + length + file->packetsize - 1) / file->packetsize;
        ucbuf = buf;
        uclim = ucbuf + length;
        skipcopy = r;
        BDBG_MSG(("crypto_read: whole %d", wp));
        while (wp) {
            unsigned trp;
            unsigned toread;
            unsigned tocopy;
            trp = wp;
            if (trp > file->packets) {
                trp = file->packets;
            }
            toread = file->packetsize * trp;
            BDBG_MSG(("crypto_read: read@"BDBG_UINT64_FMT"/%d", BDBG_UINT64_ARG(pos), toread));
            file->fd->seek(file->fd, pos, SEEK_SET);
            file->fd->read(file->fd, file->packet, toread);
            decrypt(file, file->packet, toread);
            tocopy = toread - skipcopy;
            if (ucbuf + tocopy > uclim) {
                tocopy = uclim - ucbuf;
            }
            BDBG_MSG(("crypto_read: skip %d copy %d", skipcopy, tocopy));
            BKNI_Memcpy(ucbuf, file->packet + skipcopy, tocopy);
            pos += toread;
            ucbuf += tocopy;
            skipcopy = 0;
            wp -= trp;
        }
        file->pos += length;
        return length;
    }
    return -1;
}

static off_t
b_file_crypto_seek(bfile_io_read_t fd, off_t offset, int whence)
{
    bfile_crypto_t file=(void *)fd;
    BDBG_MSG(("crypto_seek("BDBG_UINT64_FMT", %d)", BDBG_UINT64_ARG(offset), whence));
    if(file->fd) {
        off_t first, last;
        int rc;

        switch(whence) {
        case SEEK_CUR:
        case SEEK_END:
            rc = file->fd->bounds(file->fd, &first, &last);
            if(rc<0) { goto error;}
            if(whence==SEEK_END) {
                offset = last + offset;
            } else {
                offset = file->pos + offset;
            }
            if(offset<first) {
                offset = first;
            } else if (offset>last) {
                offset = last;
            }
            /* keep going */
        case SEEK_SET:
        default:
            file->pos = offset;
            break;
        }
        return offset;
    }
error:
    return -1;
}

static int
b_file_crypto_bounds(bfile_io_read_t fd, off_t *first, off_t *last)
{
    bfile_crypto_t file=(void *)fd;
    if(file->fd) {
        return file->fd->bounds(file->fd, first, last);
    }
    *first = *last = 0;
    return -1;
}

static const struct bfile_io_read b_file_crypto_ops = {
    b_file_crypto_read,
    b_file_crypto_seek,
    b_file_crypto_bounds,
    BIO_DEFAULT_PRIORITY
};

bfile_crypto_t
bfile_crypto_create(NEXUS_SecurityAlgorithm algo, const void *key, unsigned size)
{
    bfile_crypto_t  file;
    int rc;
    NEXUS_MemoryAllocationSettings settings;
    NEXUS_ClientConfiguration clientConfig;

    NEXUS_Platform_GetClientConfiguration(&clientConfig);

    file = BKNI_Malloc(sizeof(*file));
    if(!file) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    file->fd = NULL;
    file->pos = 0;
    file->file = b_file_crypto_ops;
    file->packetsize = 188;
    file->packets = 8192/188 + 2; /* enough for worst case alignment of 8192 byte reads from probe */
    rc = dmaSetup(file, algo, key, size);
    if (rc) { BERR_TRACE(rc); goto err_dma; }
    NEXUS_Memory_GetDefaultAllocationSettings(&settings);
    settings.heap = NEXUS_Heap_Lookup(NEXUS_HeapLookupType_eMain);
    rc = NEXUS_Memory_Allocate(file->packetsize * file->packets, &settings, (void **)&file->packet);
    if (rc) { BERR_TRACE(rc); goto err_memalloc; }

    return file;

err_memalloc:
    dmaDestroy(file);
err_dma:
    BKNI_Free(file);
err_alloc:
    return NULL;
}

bfile_io_read_t
bfile_crypto_attach(bfile_crypto_t file, bfile_io_read_t fd)
{
    file->pos = 0;
    file->fd = fd;
    return &file->file;
}

void
bfile_crypto_detach(bfile_crypto_t file)
{
    file->pos = 0;
    file->fd = NULL;
}

void
bfile_crypto_destroy(bfile_crypto_t file)
{
    dmaDestroy(file);
    NEXUS_Memory_Free(file->packet);
    BKNI_Free(file);
}
#else
bfile_crypto_t bfile_crypto_create(NEXUS_SecurityAlgorithm algo, const void *key, unsigned size)
{
    BSTD_UNUSED(algo);
    BSTD_UNUSED(key);
    BSTD_UNUSED(size);
    return NULL;
}

void bfile_crypto_destroy(bfile_crypto_t file)
{
    BSTD_UNUSED(file);
}

bfile_io_read_t bfile_crypto_attach(bfile_crypto_t file, bfile_io_read_t fd)
{
    BSTD_UNUSED(file);
    BSTD_UNUSED(fd);
    return NULL;
}

void bfile_crypto_detach(bfile_crypto_t file)
{
    BSTD_UNUSED(file);
}
#endif
