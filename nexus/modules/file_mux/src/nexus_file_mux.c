/***************************************************************************
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
 **************************************************************************/
#include "nexus_file_mux_module.h"
#include "nexus_video_encoder_output.h"
#include "nexus_file_muxio.h"

BDBG_MODULE(nexus_file_mux);
BDBG_FILE_MODULE(nexus_file_mux_io);
BDBG_FILE_MODULE(nexus_file_mux_video);
BDBG_FILE_MODULE(nexus_file_mux_audio);
BDBG_FILE_MODULE(nexus_file_mux_timer);

BDBG_OBJECT_ID(NEXUS_FileMux);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */
#define BDBG_MSG_IO(x)      /* BDBG_MSG(x) */
#define NEXUS_P_FILEMUX_QUEUE_THRESHOLD (NEXUS_P_FILEMUX_QUEUE_SIZE/4)

NEXUS_FileMux_P_State g_NEXUS_FileMux_P_State;
static void NEXUS_FileMux_P_DoMux(NEXUS_FileMuxHandle mux);

void
NEXUS_FileMuxModule_GetDefaultSettings( NEXUS_FileMuxModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

NEXUS_ModuleHandle
NEXUS_FileMuxModule_Init( const NEXUS_FileMuxModuleSettings *pSettings)
{
    NEXUS_Error rc;
    NEXUS_ModuleSettings moduleSettings;
    BMMA_CreateSettings mmaCreateSettings;

    BSTD_UNUSED(pSettings);

    BDBG_ASSERT(g_NEXUS_FileMux_P_State.module==NULL);
    BKNI_Memset(&g_NEXUS_FileMux_P_State, 0, sizeof(g_NEXUS_FileMux_P_State));
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eHigh;
    g_NEXUS_FileMux_P_State.module = NEXUS_Module_Create("file_mux", &moduleSettings);
    if(g_NEXUS_FileMux_P_State.module == NULL) { rc = BERR_TRACE(BERR_OS_ERROR); goto error_module; }

    BMMA_GetDefaultCreateSettings(&mmaCreateSettings);
    rc = BMMA_Create(&g_NEXUS_FileMux_P_State.mma, &mmaCreateSettings);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc); goto error_mma;}

    return g_NEXUS_FileMux_P_State.module;

error_mma:
    NEXUS_Module_Destroy(g_NEXUS_FileMux_P_State.module);
error_module:
    return NULL;
}

void
NEXUS_FileMuxModule_Uninit(void)
{
    if(g_NEXUS_FileMux_P_State.module==NULL) {return;}

    BMMA_Destroy(g_NEXUS_FileMux_P_State.mma);

    NEXUS_Module_Destroy(g_NEXUS_FileMux_P_State.module);
    g_NEXUS_FileMux_P_State.module = NULL;
    return;
}

static void NEXUS_P_FileMux_IoComplete(void *cntx, ssize_t size);

BDBG_OBJECT_ID(NEXUS_P_FileMux_File);

static void NEXUS_P_FileMux_File_Attach(NEXUS_P_FileMux_File *muxFile, NEXUS_FileMuxHandle mux, NEXUS_MuxFileHandle file)
{
    BDBG_OBJECT_INIT(muxFile, NEXUS_P_FileMux_File);
    muxFile->mux = mux;
    muxFile->file = file;
    muxFile->completed = 0;
    muxFile->lastResult = 0;
    muxFile->ioError = false;
    muxFile->ioInProgress = false;
    muxFile->ioCanceled = false;
    BFIFO_INIT(&muxFile->fifo, muxFile->descriptors, NEXUS_P_FILEMUX_QUEUE_SIZE);
    return;
}

static void NEXUS_P_FileMux_File_Detach(NEXUS_P_FileMux_File *muxFile)
{
    BDBG_OBJECT_DESTROY(muxFile, NEXUS_P_FileMux_File);
}

static void NEXUS_P_FileMux_Dequeue(NEXUS_P_FileMux_File *muxFile)
{
    const BMUXlib_StorageDescriptor *desc;

    if(BFIFO_READ_LEFT(&muxFile->fifo)==0) { (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);return;}
    desc = *BFIFO_READ(&muxFile->fifo);
    if(muxFile->ioCanceled) {
        BDBG_MSG(("%p:%p canceled io", (void*)muxFile->mux, (void*)muxFile));
        return;
    }
    BDBG_ASSERT(!muxFile->ioInProgress);
    muxFile->ioInProgress = true;
    BDBG_MODULE_MSG(nexus_file_mux_io, ("%p:%p processing request %p:%s %u:%u -> %p[%u]", (void *)muxFile->mux, (void *)muxFile, (void *)desc, desc->bWriteOperation?"write":"read", (unsigned)desc->uiOffset, (unsigned)desc->iov[0].uiLength, desc->iov[0].pBufferAddress, desc->uiVectorCount));
    if(desc->bWriteOperation) {
        NEXUS_ASSERT_FIELD(NEXUS_FileMuxIoWriteAtom, base, BMUXlib_StorageBuffer, pBufferAddress);
        NEXUS_ASSERT_FIELD(NEXUS_FileMuxIoWriteAtom, len, BMUXlib_StorageBuffer, uiLength);
        NEXUS_ASSERT_STRUCTURE(NEXUS_FileMuxIoWriteAtom, BMUXlib_StorageBuffer);
        NEXUS_File_AsyncMuxWrite(&muxFile->file->mux, desc->uiOffset, (NEXUS_FileMuxIoWriteAtom *)desc->iov, desc->uiVectorCount, NEXUS_MODULE_SELF, NEXUS_P_FileMux_IoComplete, muxFile);
    } else {
        NEXUS_ASSERT_FIELD(NEXUS_FileMuxIoWriteAtom, base, BMUXlib_StorageBuffer, pBufferAddress);
        NEXUS_ASSERT_FIELD(NEXUS_FileMuxIoWriteAtom, len, BMUXlib_StorageBuffer, uiLength);
        NEXUS_ASSERT_STRUCTURE(NEXUS_FileMuxIoWriteAtom, BMUXlib_StorageBuffer);
        NEXUS_File_AsyncMuxRead(&muxFile->file->mux, desc->uiOffset, (NEXUS_FileMuxIoReadAtom *)desc->iov, desc->uiVectorCount, NEXUS_MODULE_SELF, NEXUS_P_FileMux_IoComplete, muxFile);
    }
    return;
}

static void NEXUS_P_FileMux_IoComplete(void *cntx, ssize_t size)
{
    NEXUS_P_FileMux_File *muxFile = cntx;
    const BMUXlib_StorageDescriptor *desc;
    NEXUS_FileMuxHandle mux;
    size_t expectedSize;
    unsigned i;

    BDBG_OBJECT_ASSERT(muxFile, NEXUS_P_FileMux_File);
    BDBG_ASSERT(muxFile->ioInProgress);
    NEXUS_ASSERT_MODULE();
    mux = muxFile->mux;
    if(mux->ioWaitingThreshold) {
        if(BFIFO_WRITE_PEEK(&muxFile->fifo)>=NEXUS_P_FILEMUX_QUEUE_THRESHOLD) {
            mux->ioWaitingThreshold = false;
            if(!mux->stopping) {
                NEXUS_FileMux_P_DoMux(mux);
            }
        }
    }
    muxFile->ioInProgress = false;
    if(muxFile->ioCanceled) {
        BDBG_MSG(("I/O:%p canceled", (void*)muxFile));
        return;
    }
    desc = *BFIFO_READ(&muxFile->fifo);
    BDBG_MODULE_MSG(nexus_file_mux_io, ("%p:%p completed request %p:%s %u:%u (%u)-> %px:%d", (void*)muxFile->mux, (void*)muxFile, (void *)desc, desc->bWriteOperation?"write":"read", (unsigned)desc->uiOffset, (unsigned)desc->iov[0].uiLength, (unsigned)desc->uiVectorCount, (void *)desc->iov[0].pBufferAddress, (int)size));
    if(BFIFO_READ_LEFT(&muxFile->fifo)==0) { (void)BERR_TRACE(NEXUS_UNKNOWN); return;}
    muxFile->lastResult = size;
    for(expectedSize=0,i=0;i<desc->uiVectorCount;i++) {
        expectedSize += desc->iov[i].uiLength;
    }
    if(size<0 || expectedSize != (size_t)size) {
        BDBG_WRN(("I/O %p result %d expected %u", (void*)muxFile, (int)size, (unsigned)expectedSize));
        (void)BERR_TRACE(NEXUS_OS_ERROR);return;
    }
    muxFile->completed++;
    BFIFO_READ_COMMIT(&muxFile->fifo, 1);
    if(BFIFO_READ_LEFT(&muxFile->fifo)) {
        NEXUS_P_FileMux_Dequeue(muxFile);
    }
    return;
}

static BERR_Code NEXUS_P_FileMux_AddStorageDescriptors(void *pStorageInterfaceContext, const BMUXlib_StorageDescriptor *pDescriptors, size_t uiNumDescriptors, size_t *puiQueuedCount)
{
    NEXUS_P_FileMux_File *muxFile = pStorageInterfaceContext;
    unsigned i;
    unsigned j;
    bool idle;

    BDBG_OBJECT_ASSERT(muxFile, NEXUS_P_FileMux_File);
    BDBG_ASSERT(pDescriptors);
    BDBG_ASSERT(puiQueuedCount);
    idle = BFIFO_READ_LEFT(&muxFile->fifo)==0;
    BDBG_MODULE_MSG(nexus_file_mux_io, ("%p:%p > add descriptors %u:%u %s", (void*)muxFile->mux, (void*)muxFile, (unsigned)uiNumDescriptors, BFIFO_WRITE_PEEK(&muxFile->fifo), idle?"idle":""));
    for(i=0;i<uiNumDescriptors;i++) {
        const BMUXlib_StorageDescriptor **desc = BFIFO_WRITE(&muxFile->fifo);
        if(BFIFO_WRITE_LEFT(&muxFile->fifo)==0) {
            break;
        }
        *desc = pDescriptors;
        BFIFO_WRITE_COMMIT(&muxFile->fifo, 1);
        for(j=0;j<pDescriptors->uiVectorCount;j++) {
            BDBG_MODULE_MSG(nexus_file_mux_io, ("%p:%p queuing request %u:%u %p:%s:%u (%u:%u) %u -> %p", (void*)muxFile->mux, (void*)muxFile, (unsigned)uiNumDescriptors, i, (void *)pDescriptors, pDescriptors->bWriteOperation?"write":"read", (unsigned)pDescriptors->uiOffset, pDescriptors->uiVectorCount, j, (unsigned)pDescriptors->iov[j].uiLength, pDescriptors->iov[j].pBufferAddress));
            /*BDBG_ASSERT(pDescriptors->iov[j].pBufferAddress!=0);*//* this seems to be valid for null descriptors! */
        }
        pDescriptors = BMUXLIB_STORAGE_GET_NEXT_DESC(pDescriptors);
    }
    *puiQueuedCount = i;
    BDBG_MODULE_MSG(nexus_file_mux_io, ("%p:%p < add descriptors %u:%u -> %u", (void*)muxFile->mux, (void*)muxFile, (unsigned)uiNumDescriptors, BFIFO_WRITE_PEEK(&muxFile->fifo), i));
    if(idle && i>0) {
        NEXUS_P_FileMux_Dequeue(muxFile);
    }
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_P_FileMux_GetCompletedStorageDescriptors(void *pStorageInterfaceContext, size_t *puiCompletedCount)
{
    NEXUS_P_FileMux_File *muxFile = pStorageInterfaceContext;
    BDBG_OBJECT_ASSERT(muxFile, NEXUS_P_FileMux_File);
    BDBG_ASSERT(puiCompletedCount);
    BDBG_MODULE_MSG(nexus_file_mux_io, ("%p:%p completed %u", (void*)muxFile->mux, (void*)muxFile, muxFile->completed));
    if(muxFile->ioError) {return BERR_TRACE(BERR_OS_ERROR);}
    *puiCompletedCount = muxFile->completed;
    muxFile->completed = 0;
    return BERR_SUCCESS;
}

BDBG_OBJECT_ID(NEXUS_P_FileMux_TempStorage);

static void NEXUS_P_FileMux_TempStorage_Init(NEXUS_P_FileMux_TempStorage *tempStorage, NEXUS_FileMuxHandle mux)
{
    BDBG_OBJECT_INIT(tempStorage, NEXUS_P_FileMux_TempStorage);
    tempStorage->mux = mux;
    BLST_D_INIT(&tempStorage->files);
    return;
}

static void NEXUS_P_FileMux_GetDefaultStorageSettings(BMUXlib_StorageSettings *pStorageSettings)
{
    BDBG_ASSERT(pStorageSettings);
    BKNI_Memset(pStorageSettings, 0, sizeof(*pStorageSettings));
    return;
}

static BERR_Code NEXUS_P_FileMux_CreateStorage(void *pStorageContext, BMUXlib_StorageObjectInterface *pStorageInterface, const BMUXlib_StorageSettings *pStorageSettings)
{
    NEXUS_P_FileMux_TempFile *tempFile;
    NEXUS_P_FileMux_TempStorage *tempStorage = pStorageContext;
    NEXUS_MuxFileHandle file;
    BERR_Code rc;
    char fname[128];

    BDBG_OBJECT_ASSERT(tempStorage, NEXUS_P_FileMux_TempStorage);
    BDBG_ASSERT(pStorageSettings);
    BDBG_ASSERT(pStorageInterface);
    BSTD_UNUSED(pStorageSettings);

    tempFile = BKNI_Malloc(sizeof(*tempFile));
    if(!tempFile) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    BKNI_Snprintf(fname, sizeof(fname),"%s/mux%p.tmp", tempStorage->mux->startSettings.tempDir, (void*)tempFile);
    BDBG_MSG(("using temporary file %s", fname));
    file = NEXUS_MuxFile_OpenPosix(fname);
    if(!file) {rc=BERR_TRACE(NEXUS_OS_ERROR);goto err_open;}
    BDBG_MSG(("%p: opening file %s:%p(%p)", (void *)tempStorage->mux, fname, (void *)tempFile, (void *)file));
    unlink(fname);
    NEXUS_P_FileMux_File_Attach(&tempFile->file, tempStorage->mux, file);
    tempFile->destroyed = false;
    BLST_D_INSERT_HEAD(&tempStorage->files, tempFile, link);

    pStorageInterface->pContext = &tempFile->file;
    pStorageInterface->pfAddDescriptors = NEXUS_P_FileMux_AddStorageDescriptors;
    pStorageInterface->pfGetCompleteDescriptors = NEXUS_P_FileMux_GetCompletedStorageDescriptors;

    return BERR_SUCCESS;
err_open:
    BKNI_Free(tempFile);
err_alloc:
    return rc;
}

static void NEXUS_P_TempFile_Destroy(NEXUS_P_FileMux_TempStorage *tempStorage,NEXUS_P_FileMux_TempFile *tempFile)
{
    BLST_D_REMOVE(&tempStorage->files, tempFile, link);
    NEXUS_MuxFile_Close(tempFile->file.file);
    NEXUS_P_FileMux_File_Detach(&tempFile->file);
    BKNI_Free(tempFile);
    return;
}

static void NEXIS_P_FileMux_DestroyStorage(void *pStorageContext, void *pStorageObjectContext)
{
    NEXUS_P_FileMux_TempFile *tempFile = pStorageObjectContext;
    NEXUS_P_FileMux_TempStorage *tempStorage = pStorageContext;

    BDBG_OBJECT_ASSERT(tempStorage, NEXUS_P_FileMux_TempStorage);
    BDBG_OBJECT_ASSERT(&tempFile->file, NEXUS_P_FileMux_File);
    if(!tempFile->file.ioInProgress) {
        NEXUS_P_TempFile_Destroy(tempStorage, tempFile);
    } else {
       tempFile->file.ioCanceled = true;
    }
    return;
}

static void NEXUS_P_TempStorage_CancelIo(NEXUS_P_FileMux_TempStorage *tempStorage)
{
    NEXUS_P_FileMux_TempFile *tempFile;

    for(tempFile=BLST_D_FIRST(&tempStorage->files);tempFile;tempFile=BLST_D_NEXT(tempFile, link)) {
        tempFile->file.ioCanceled = true;
    }
    return;
}

static bool NEXUS_P_TempStorage_IsInProgress(NEXUS_P_FileMux_TempStorage *tempStorage)
{
    NEXUS_P_FileMux_TempFile *tempFile;

    for(tempFile=BLST_D_FIRST(&tempStorage->files);tempFile;tempFile=BLST_D_NEXT(tempFile, link)) {
        if(tempFile->file.ioInProgress) {
            return true;
        }
    }
    return false;
}

static void NEXUS_P_TempStorage_Clean(NEXUS_P_FileMux_TempStorage *tempStorage)
{
    NEXUS_P_FileMux_TempFile *tempFile;
    /* coverity[use_after_free] */
    while(NULL!=(tempFile=BLST_D_FIRST(&tempStorage->files))) {
        if(!tempFile->destroyed) {
            BDBG_WRN(("destroying leaked file %p", (void*)tempFile));
        }
        NEXUS_P_TempFile_Destroy(tempStorage, tempFile);
    }
    return;
}

static void
NEXUS_P_FileMux_GetDefaultCreateSettings_MP4( NEXUS_FileMuxCreateSettings *pSettings )
{
    BMUXlib_File_MP4_CreateSettings muxCreateSettings;
    BDBG_ASSERT(pSettings);

    BMUXlib_File_MP4_GetDefaultCreateSettings(&muxCreateSettings);
    pSettings->storageDescriptors = muxCreateSettings.uiNumOutputStorageDescriptors;
    pSettings->mp4.supported = true;
    pSettings->mp4.metadataCache = muxCreateSettings.uiNumMetadataEntriesCached;
    pSettings->mp4.heapSize = muxCreateSettings.uiBoxHeapSizeBytes;
    pSettings->mp4.sizeEntriesCache = muxCreateSettings.uiNumSizeEntries;
    pSettings->mp4.relocationBuffer = muxCreateSettings.uiRelocationBufferSizeBytes;
    return;
}

static void
NEXUS_P_FileMux_GetDefaultCreateSettings_PES( NEXUS_FileMuxCreateSettings *pSettings )
{
    BMUXlib_File_PES_CreateSettings muxCreateSettings;
    BDBG_ASSERT(pSettings);
    BMUXlib_File_PES_GetDefaultCreateSettings(&muxCreateSettings);
    pSettings->pes.supported = true;
    return;
}

static void
NEXUS_P_FileMux_GetDefaultCreateSettings_ASF( NEXUS_FileMuxCreateSettings *pSettings )
{
#if B_FILE_MUX_HAS_ASF
    BMUXlib_File_ASF_CreateSettings muxCreateSettings;
    BDBG_ASSERT(pSettings);

    BMUXlib_File_ASF_GetDefaultCreateSettings(&muxCreateSettings);
    pSettings->asf.supported = true;
    pSettings->asf.packetLength = muxCreateSettings.uiPacketLength;
#endif
    BSTD_UNUSED(pSettings);
    return;
}

void
NEXUS_FileMux_GetDefaultCreateSettings( NEXUS_FileMuxCreateSettings *pSettings )
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    NEXUS_P_FileMux_GetDefaultCreateSettings_MP4(pSettings);
    NEXUS_P_FileMux_GetDefaultCreateSettings_ASF(pSettings);
    NEXUS_P_FileMux_GetDefaultCreateSettings_PES(pSettings);
    NEXUS_CallbackDesc_Init(&pSettings->finished);

    return;
}

static NEXUS_Error
NEXUS_P_FileMux_Create_MP4(NEXUS_FileMuxHandle mux, const NEXUS_FileMuxCreateSettings *pSettings)
{
    BERR_Code rc;
    BMUXlib_File_MP4_CreateSettings muxCreateSettings;

    BMUXlib_File_MP4_GetDefaultCreateSettings(&muxCreateSettings);
    muxCreateSettings.uiNumOutputStorageDescriptors = pSettings->storageDescriptors;
    muxCreateSettings.uiNumMetadataEntriesCached = pSettings->mp4.metadataCache;
    muxCreateSettings.uiBoxHeapSizeBytes = pSettings->mp4.heapSize;
    muxCreateSettings.uiNumSizeEntries = pSettings->mp4.sizeEntriesCache;
    muxCreateSettings.uiRelocationBufferSizeBytes = pSettings->mp4.relocationBuffer;

    rc = BMUXlib_File_MP4_Create(&mux->mp4.mux, &muxCreateSettings);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
}

static void
NEXUS_P_FileMux_Destroy_MP4(NEXUS_FileMuxHandle mux)
{
    if(mux->mp4.mux) {
        BMUXlib_File_MP4_Destroy(mux->mp4.mux);
        mux->mp4.mux = NULL;
        mux->mp4.active = false;
    }
    return;
}

static NEXUS_Error
NEXUS_P_FileMux_Create_PES(NEXUS_FileMuxHandle mux, const NEXUS_FileMuxCreateSettings *pSettings)
{
    BERR_Code rc;
    BMUXlib_File_PES_CreateSettings muxCreateSettings;

    BSTD_UNUSED(pSettings);

    BMUXlib_File_PES_GetDefaultCreateSettings(&muxCreateSettings);

    rc = BMUXlib_File_PES_Create(&mux->pes.mux, &muxCreateSettings);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
}

static NEXUS_Error
NEXUS_P_FileMux_Create_IVF(NEXUS_FileMuxHandle mux, const NEXUS_FileMuxCreateSettings *pSettings)
{
    BERR_Code rc;
    BMUXlib_File_IVF_CreateSettings muxCreateSettings;

    BSTD_UNUSED(pSettings);

    BMUXlib_File_IVF_GetDefaultCreateSettings(&muxCreateSettings);

    rc = BMUXlib_File_IVF_Create(&mux->ivf.mux, &muxCreateSettings);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
}

static void
NEXUS_P_FileMux_Destroy_PES(NEXUS_FileMuxHandle mux)
{
    if(mux->pes.mux) {
        BMUXlib_File_PES_Destroy(mux->pes.mux);
        mux->pes.mux = NULL;
        mux->pes.active = false;
    }
    return;
}

static void
NEXUS_P_FileMux_Destroy_IVF(NEXUS_FileMuxHandle mux)
{
    if(mux->ivf.mux) {
        BMUXlib_File_IVF_Destroy(mux->ivf.mux);
        mux->ivf.mux = NULL;
        mux->ivf.active = false;
    }
    return;
}

static NEXUS_Error
NEXUS_P_FileMux_Create_ASF(NEXUS_FileMuxHandle mux, const NEXUS_FileMuxCreateSettings *pSettings)
{
#if B_FILE_MUX_HAS_ASF
    BERR_Code rc;
    BMUXlib_File_ASF_CreateSettings muxCreateSettings;

    BMUXlib_File_ASF_GetDefaultCreateSettings(&muxCreateSettings);
    muxCreateSettings.uiPacketLength = pSettings->asf.packetLength;

    rc = BMUXlib_File_ASF_Create(&mux->asf.mux, &muxCreateSettings);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(mux);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

static void
NEXUS_P_FileMux_Destroy_ASF(NEXUS_FileMuxHandle mux)
{
#if B_FILE_MUX_HAS_ASF
    if(mux->asf.mux) {
        BMUXlib_File_ASF_Destroy(mux->asf.mux);
        mux->asf.mux = NULL;
        mux->asf.active = false;
    }
#endif
    BSTD_UNUSED(mux);
    return;
}

static void
NEXUS_P_FileMux_ManagedPtr_Init(NEXUS_P_FileMux_ManagedPtr *managed)
{
    managed->block = NULL;
    managed->ptr = NULL;
    return;
}

static void
NEXUS_P_FileMux_ManagedPtr_Release(NEXUS_P_FileMux_ManagedPtr *managed)
{
    if(managed->ptr) {
        BDBG_ASSERT(managed->block);
        NEXUS_MemoryBlock_Unlock(managed->block);
        managed->ptr = NULL;
        managed->block = NULL;
    }
    return;
}

static void NEXUS_P_FileMux_MemoryBlock_Init(NEXUS_P_FileMux_MemoryBlock *muxBlock)
{
    muxBlock->mmaBlock = NULL;
    muxBlock->block = NULL;
    return;
}

static void NEXUS_P_FileMux_MemoryBlock_Release(NEXUS_P_FileMux_MemoryBlock *muxBlock)
{
    if (muxBlock->mmaBlock) {
        BMMA_Free(muxBlock->mmaBlock);
        muxBlock->mmaBlock = NULL;
        NEXUS_MemoryBlock_Unlock(muxBlock->block);
        NEXUS_MemoryBlock_UnlockOffset(muxBlock->block);
        muxBlock->block = NULL;
    }
    return;
}

static BMMA_Block_Handle NEXUS_P_FileMux_MemoryBlock_Attach(NEXUS_P_FileMux_MemoryBlock *muxBlock, NEXUS_MemoryBlockHandle block)
{
    NEXUS_MemoryBlockProperties prop;
    int rc;
    void *addr;
    NEXUS_Addr offset;

    if (muxBlock->block == block) {
        return muxBlock->mmaBlock;
    }
    NEXUS_P_FileMux_MemoryBlock_Release(muxBlock);

    rc = NEXUS_MemoryBlock_LockOffset(block, &offset);
    if (rc) {BERR_TRACE(rc); goto err_lockoffset;}
    rc = NEXUS_MemoryBlock_Lock(block, &addr);
    if (rc) {BERR_TRACE(rc); goto err_lock;}

    NEXUS_MemoryBlock_GetProperties(block, &prop);

    muxBlock->mmaBlock = BMMA_Block_Create(g_NEXUS_FileMux_P_State.mma, offset, prop.size, addr);
    if (muxBlock->mmaBlock) {
        muxBlock->block = block;
    }
    else {
        goto err_create;
    }
    return muxBlock->mmaBlock;

err_create:
    NEXUS_MemoryBlock_Unlock(block);
err_lock:
    NEXUS_MemoryBlock_UnlockOffset(block);
err_lockoffset:
    return NULL;
}


NEXUS_FileMuxHandle
NEXUS_FileMux_Create( const NEXUS_FileMuxCreateSettings *pSettings )
{
    NEXUS_FileMuxHandle  mux;
    NEXUS_FileMuxCreateSettings createSettings;
    NEXUS_Error rc;
    unsigned i;

    if(pSettings==NULL) {
        NEXUS_FileMux_GetDefaultCreateSettings(&createSettings);
        pSettings = &createSettings;
    }
    mux = BKNI_Malloc(sizeof(*mux));
    if(mux==NULL) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}
    BKNI_Memset(mux, 0, sizeof(*mux));
    BDBG_OBJECT_SET(mux, NEXUS_FileMux);
    mux->createSettings = *pSettings;

    NEXUS_P_FileMux_MemoryBlock_Init(&mux->videoFrame);
    NEXUS_P_FileMux_MemoryBlock_Init(&mux->videoMeta);
    NEXUS_P_FileMux_MemoryBlock_Init(&mux->simpleVideoFrame);
    NEXUS_P_FileMux_MemoryBlock_Init(&mux->simpleVideoMeta);
    NEXUS_P_FileMux_MemoryBlock_Init(&mux->audioFrame);
    NEXUS_P_FileMux_MemoryBlock_Init(&mux->audioMeta);
    NEXUS_P_FileMux_MemoryBlock_Init(&mux->simpleAudioFrame);
    NEXUS_P_FileMux_MemoryBlock_Init(&mux->simpleAudioMeta);

    NEXUS_P_FileMux_TempStorage_Init(&mux->tempStorage, mux);

    mux->finishedCallback = NEXUS_TaskCallback_Create(mux, NULL);
    if(!mux->finishedCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    NEXUS_TaskCallback_Set(mux->finishedCallback, &pSettings->finished);

    for(i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        mux->state.audio[i].index = i;
        mux->state.audio[i].mux = mux;
        NEXUS_P_FileMux_ManagedPtr_Init(&mux->state.audio[i].frame);
        NEXUS_P_FileMux_ManagedPtr_Init(&mux->state.audio[i].meta);

        mux->state.video[i].index = i;
        mux->state.video[i].mux = mux;
        NEXUS_P_FileMux_ManagedPtr_Init(&mux->state.video[i].frame);
        NEXUS_P_FileMux_ManagedPtr_Init(&mux->state.video[i].meta);
    }


    if(pSettings->mp4.supported) {
        rc = NEXUS_P_FileMux_Create_MP4(mux, pSettings);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error_mp4;}
    }

    if(pSettings->pes.supported) {
        if(NEXUS_GetEnv("IVF_SUPPORT")) {
            rc = NEXUS_P_FileMux_Create_IVF(mux, pSettings);
        } else {
            rc = NEXUS_P_FileMux_Create_PES(mux, pSettings);
        }
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error_pes;}
    }

    if(pSettings->asf.supported) {
        rc = NEXUS_P_FileMux_Create_ASF(mux, pSettings);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error_asf;}
    }

    return mux;

error_asf:
    NEXUS_P_FileMux_Destroy_IVF(mux);
    NEXUS_P_FileMux_Destroy_PES(mux);
error_pes:
    NEXUS_P_FileMux_Destroy_MP4(mux);
error_mp4:
    BKNI_Free(mux);
error:
    return NULL;
}

void
NEXUS_FileMux_Destroy(NEXUS_FileMuxHandle mux)
{
    BDBG_OBJECT_ASSERT(mux, NEXUS_FileMux);
    if(mux->started) {
        NEXUS_FileMux_Stop(mux);
    }
    NEXUS_TaskCallback_Destroy(mux->finishedCallback);
    NEXUS_P_FileMux_Destroy_MP4(mux);
    NEXUS_P_FileMux_Destroy_IVF(mux);
    NEXUS_P_FileMux_Destroy_PES(mux);
    NEXUS_P_FileMux_Destroy_ASF(mux);
    BDBG_OBJECT_DESTROY(mux, NEXUS_FileMux);
    BKNI_Free(mux);
    return;
}

static void
NEXUS_P_FileMux_GetDefaultStartSettings_MP4( NEXUS_FileMuxStartSettings *pSettings)
{
    BMUXlib_File_MP4_StartSettings  startSettings;
    BMUXlib_File_MP4_GetDefaultStartSettings(&startSettings);
    pSettings->expectedDuration = startSettings.uiExpectedDurationMs;
    pSettings->streamSettings.mp4.progressiveDownloadCompatible = startSettings.bProgressiveDownloadCompatible;
    pSettings->streamSettings.mp4.createTime = startSettings.uiCreateTimeUTC;
    return;
}

static void
NEXUS_P_FileMux_GetDefaultStartSettings_PES( NEXUS_FileMuxStartSettings *pSettings)
{
    BMUXlib_File_PES_StartSettings  startSettings;
    BMUXlib_File_PES_GetDefaultStartSettings(&startSettings);
    BSTD_UNUSED(pSettings);
    return;
}

static void
NEXUS_P_FileMux_GetDefaultStartSettings_ASF( NEXUS_FileMuxStartSettings *pSettings)
{
#if B_FILE_MUX_HAS_ASF
    BMUXlib_File_ASF_StartSettings  startSettings;

    BMUXlib_File_ASF_GetDefaultStartSettings(&startSettings);
    BDBG_CASSERT(sizeof(pSettings->streamSettings.asf.fileId)==sizeof(startSettings.auiFileId));
    BKNI_Memcpy(pSettings->streamSettings.asf.fileId, startSettings.auiFileId, sizeof(pSettings->streamSettings.asf.fileId));
    pSettings->streamSettings.asf.broadcastOnly = startSettings.bBroadcastOnly;
#endif
    BSTD_UNUSED(pSettings);
    return;
}

void
NEXUS_FileMux_GetDefaultStartSettings(NEXUS_FileMuxStartSettings *pSettings, NEXUS_TransportType streamType)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->streamType = streamType;
    switch(streamType) {
    case NEXUS_TransportType_eMp4:
        NEXUS_P_FileMux_GetDefaultStartSettings_MP4(pSettings);
        break;
    case NEXUS_TransportType_eMpeg2Pes:
        NEXUS_P_FileMux_GetDefaultStartSettings_PES(pSettings);
        break;
    case NEXUS_TransportType_eAsf:
        NEXUS_P_FileMux_GetDefaultStartSettings_ASF(pSettings);
        break;
    default:
        BDBG_WRN(("streamType:%u is not supported", (unsigned)streamType));
        break;
    }
    return;
}

static BERR_Code
NEXUS_FileMux_P_GetVideoBufferDescriptors( void *context, const BAVC_VideoBufferDescriptor *descriptors0[], size_t *numDescriptors0, const BAVC_VideoBufferDescriptor *descriptors1[], size_t *numDescriptors1)
{
    const NEXUS_P_FileMux_EncoderState *video = context;
    NEXUS_VideoEncoderHandle videoEncoder = video->mux->startSettings.video[video->index].encoder;
#if NEXUS_HAS_SIMPLE_DECODER
    NEXUS_SimpleEncoderHandle simpleEncoder = video->mux->startSettings.video[video->index].simpleEncoder;
#endif
    BERR_Code rc;

    NEXUS_ASSERT_MODULE();
    NEXUS_ASSERT_STRUCTURE(NEXUS_VideoEncoderDescriptor, BAVC_VideoBufferDescriptor);
    if(videoEncoder) {
        rc = NEXUS_VideoEncoder_GetBuffer(videoEncoder, (const NEXUS_VideoEncoderDescriptor**)descriptors0, numDescriptors0, (const NEXUS_VideoEncoderDescriptor**)descriptors1, numDescriptors1);
    }
#if NEXUS_HAS_SIMPLE_DECODER
    else if(simpleEncoder) {
        rc = NEXUS_SimpleEncoder_GetVideoBuffer(simpleEncoder, (const NEXUS_VideoEncoderDescriptor**)descriptors0, numDescriptors0, (const NEXUS_VideoEncoderDescriptor**)descriptors1, numDescriptors1);
    }
#endif
    else {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    BDBG_MODULE_MSG(nexus_file_mux_video, ("GetVideoBufferDescriptors:%p numDescriptors:%u:%u", (void *)videoEncoder, (unsigned)*numDescriptors0, (unsigned)*numDescriptors1));
    return BERR_TRACE(rc);
}

static BERR_Code
NEXUS_FileMux_P_ConsumeVideoBufferDescriptors( void *context, size_t numDescriptors)
{
    const NEXUS_P_FileMux_EncoderState *video = context;
    NEXUS_VideoEncoderHandle videoEncoder = video->mux->startSettings.video[video->index].encoder;
#if NEXUS_HAS_SIMPLE_DECODER
    NEXUS_SimpleEncoderHandle simpleEncoder = video->mux->startSettings.video[video->index].simpleEncoder;
#endif
    BERR_Code rc;

    NEXUS_ASSERT_MODULE();
    if(videoEncoder) {
        rc = NEXUS_VideoEncoder_ReadComplete(videoEncoder, numDescriptors);
    }
#if NEXUS_HAS_SIMPLE_DECODER
    else if(simpleEncoder) {
        rc = NEXUS_SimpleEncoder_VideoReadComplete(simpleEncoder, numDescriptors);
    }
#endif
    else {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    BDBG_MODULE_MSG(nexus_file_mux_video, ("ConsumeVideoBufferDescriptors:%p numDescriptors:%u", (void *)videoEncoder, (unsigned)numDescriptors));
    return BERR_TRACE(rc);
}

static BERR_Code
NEXUS_FileMux_P_GetVideoBufferStatus( void *context, BMUXlib_CompressedBufferStatus *status)
{
    NEXUS_P_FileMux_EncoderState *video = context;
    NEXUS_VideoEncoderHandle videoEncoder = video->mux->startSettings.video[video->index].encoder;
#if NEXUS_HAS_SIMPLE_DECODER
    NEXUS_SimpleEncoderHandle simpleEncoder = video->mux->startSettings.video[video->index].simpleEncoder;
#endif

    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(status);
    BKNI_Memset(status,0,sizeof(*status));
    if(videoEncoder) {
        NEXUS_VideoEncoderStatus encoderStatus;
        NEXUS_VideoEncoder_GetStatus(videoEncoder, &encoderStatus);

        status->hFrameBufferBlock = NEXUS_P_FileMux_MemoryBlock_Attach(&video->mux->videoFrame, encoderStatus.bufferBlock);
        status->hMetadataBufferBlock = NEXUS_P_FileMux_MemoryBlock_Attach(&video->mux->videoMeta, encoderStatus.metadataBufferBlock);
    }
#if NEXUS_HAS_SIMPLE_DECODER
    else if(simpleEncoder) {
        NEXUS_SimpleEncoderStatus simpleEncoderStatus;

        NEXUS_SimpleEncoder_GetStatus(simpleEncoder, &simpleEncoderStatus);
        status->hFrameBufferBlock = NEXUS_P_FileMux_MemoryBlock_Attach(&video->mux->simpleVideoFrame, simpleEncoderStatus.video.bufferBlock);
        status->hMetadataBufferBlock = NEXUS_P_FileMux_MemoryBlock_Attach(&video->mux->simpleVideoMeta, simpleEncoderStatus.video.metadataBufferBlock);
    }
#endif
    else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if(status->hFrameBufferBlock == NULL || status->hMetadataBufferBlock == NULL) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    return BERR_SUCCESS;
}

static BERR_Code
NEXUS_FileMux_P_GetAudioBufferDescriptors(
   void *context,
   const BAVC_AudioBufferDescriptor *astDescriptors0[], /* Can be NULL if uiNumDescriptors0=0. */
   size_t *puiNumDescriptors0,
   const BAVC_AudioBufferDescriptor *astDescriptors1[], /* Needed to handle FIFO wrap. Can be NULL if uiNumDescriptors1=0. */
   size_t *puiNumDescriptors1
   )
{
    BERR_Code rc;
    const NEXUS_P_FileMux_EncoderState *audio = context;
    NEXUS_AudioMuxOutputHandle audioMuxOutput = audio->mux->startSettings.audio[audio->index].muxOutput;
#if NEXUS_HAS_SIMPLE_DECODER
    NEXUS_SimpleEncoderHandle simpleEncoder = audio->mux->startSettings.audio[audio->index].simpleEncoder;
#endif

    NEXUS_ASSERT_MODULE();
    NEXUS_ASSERT_STRUCTURE(NEXUS_AudioMuxOutputFrame, BAVC_AudioBufferDescriptor);
    if(audioMuxOutput) {
        rc = NEXUS_AudioMuxOutput_GetBuffer(audioMuxOutput, (const NEXUS_AudioMuxOutputFrame**)astDescriptors0, puiNumDescriptors0, (const NEXUS_AudioMuxOutputFrame**)astDescriptors1, puiNumDescriptors1);
    }
#if NEXUS_HAS_SIMPLE_DECODER
    else if(simpleEncoder) {
        rc = NEXUS_SimpleEncoder_GetAudioBuffer(simpleEncoder, (const NEXUS_AudioMuxOutputFrame**)astDescriptors0, puiNumDescriptors0, (const NEXUS_AudioMuxOutputFrame**)astDescriptors1, puiNumDescriptors1);
    }
#endif
    else {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    BDBG_MODULE_MSG(nexus_file_mux_audio, ("GetAudioBufferDescriptors:%p numDescriptors:%u:%u", (void *)audioMuxOutput, (unsigned)*puiNumDescriptors0, (unsigned)*puiNumDescriptors1));
    return BERR_TRACE(rc);
}

static BERR_Code
NEXUS_FileMux_P_ConsumeAudioBufferDescriptors(
   void *context,
   size_t uiNumDescriptors
   )
{
    BERR_Code rc;
    const NEXUS_P_FileMux_EncoderState *audio = context;
    NEXUS_AudioMuxOutputHandle audioMuxOutput = audio->mux->startSettings.audio[audio->index].muxOutput;
#if NEXUS_HAS_SIMPLE_DECODER
    NEXUS_SimpleEncoderHandle simpleEncoder = audio->mux->startSettings.audio[audio->index].simpleEncoder;
#endif

    NEXUS_ASSERT_MODULE();
    if(audioMuxOutput) {
        rc = NEXUS_AudioMuxOutput_ReadComplete(audioMuxOutput, uiNumDescriptors);
    }
#if NEXUS_HAS_SIMPLE_DECODER
    else if(simpleEncoder) {
        rc = NEXUS_SimpleEncoder_AudioReadComplete(simpleEncoder, uiNumDescriptors);
    }
#endif
    else {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    return BERR_TRACE(rc);
}

static BERR_Code
NEXUS_FileMux_P_GetAudioBufferStatus(
   void *context,
   BMUXlib_CompressedBufferStatus *status
   )
{
    NEXUS_P_FileMux_EncoderState *audio = context;
    NEXUS_AudioMuxOutputHandle audioMuxOutput = audio->mux->startSettings.audio[audio->index].muxOutput;
#if NEXUS_HAS_SIMPLE_DECODER
    NEXUS_SimpleEncoderHandle simpleEncoder = audio->mux->startSettings.audio[audio->index].simpleEncoder;
#endif
    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(status);
    BKNI_Memset(status,0,sizeof(*status));

    if(audioMuxOutput) {
        NEXUS_AudioMuxOutputStatus encoderStatus;
        NEXUS_AudioMuxOutput_GetStatus(audioMuxOutput, &encoderStatus);

        status->hFrameBufferBlock = NEXUS_P_FileMux_MemoryBlock_Attach(&audio->mux->audioFrame, encoderStatus.bufferBlock);
        status->hMetadataBufferBlock = NEXUS_P_FileMux_MemoryBlock_Attach(&audio->mux->audioMeta, encoderStatus.metadataBufferBlock);
    }
#if NEXUS_HAS_SIMPLE_DECODER
    else if(simpleEncoder) {
        NEXUS_SimpleEncoderStatus simpleEncoderStatus;

        NEXUS_SimpleEncoder_GetStatus(simpleEncoder, &simpleEncoderStatus);
        status->hFrameBufferBlock = NEXUS_P_FileMux_MemoryBlock_Attach(&audio->mux->simpleAudioFrame, simpleEncoderStatus.audio.bufferBlock);
        status->hMetadataBufferBlock = NEXUS_P_FileMux_MemoryBlock_Attach(&audio->mux->simpleAudioMeta, simpleEncoderStatus.audio.metadataBufferBlock);
    }
#endif
    else {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if(status->hFrameBufferBlock == NULL || status->hMetadataBufferBlock == NULL) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    return BERR_SUCCESS;
}


static void
NEXUS_FileMux_P_MuxTimer(void *context)
{
    NEXUS_FileMuxHandle mux=context;

    BDBG_OBJECT_ASSERT(mux, NEXUS_FileMux);

    mux->muxTimer = NULL;
    NEXUS_FileMux_P_DoMux(mux);
    return;
}

static void
NEXUS_FileMux_P_DoMux(NEXUS_FileMuxHandle mux)
{
    BERR_Code rc;
    BMUXlib_DoMux_Status muxStatus;


    if(mux->stopping) {
        return;
    }
    switch(mux->startSettings.streamType) {
    case NEXUS_TransportType_eMp4:
        if(!mux->mp4.active) { rc=BERR_TRACE(NEXUS_UNKNOWN); goto error; }
        rc = BMUXlib_File_MP4_DoMux(mux->mp4.mux, &muxStatus);
        if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto error; }
        break;
    case NEXUS_TransportType_eMpeg2Pes:
        if(mux->pes.active) {
            rc = BMUXlib_File_PES_DoMux(mux->pes.mux, &muxStatus);
            if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto error; }
        } else if(mux->ivf.active) {
            rc = BMUXlib_File_IVF_DoMux(mux->ivf.mux, &muxStatus);
            if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto error; }
        } else  { rc=BERR_TRACE(NEXUS_UNKNOWN); goto error; }
        break;
#if B_FILE_MUX_HAS_ASF
    case NEXUS_TransportType_eAsf:
        if(!mux->asf.active) { rc=BERR_TRACE(NEXUS_UNKNOWN); goto error; }
        rc = BMUXlib_File_ASF_DoMux(mux->asf.mux, &muxStatus);
        if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto error; }
        break;
#endif
    default:
        rc=BERR_TRACE(NEXUS_UNKNOWN); goto error;
    }
    mux->duration = muxStatus.uiCompletedDuration;
    BDBG_MODULE_MSG(nexus_file_mux_timer, ("MuxTimer:%p nextExecutionTime:%u state:%u %s duration=%u(ms)", (void*)mux, muxStatus.uiNextExecutionTime, (unsigned)muxStatus.eState,muxStatus.bBlockedOutput?"BlockedOutput":"", muxStatus.uiCompletedDuration));
    if(muxStatus.eState!=BMUXlib_State_eFinished) {
        if(muxStatus.bBlockedOutput) {
            unsigned queued = BFIFO_READ_PEEK(&mux->outputFile.fifo);

            if(queued==0) {
                const NEXUS_P_FileMux_TempFile *tempFile;
                for(tempFile=BLST_D_FIRST(&mux->tempStorage.files);tempFile;tempFile=BLST_D_NEXT(tempFile, link)) {
                    queued += BFIFO_READ_PEEK(&tempFile->file.fifo);
                }
            }

            if(queued>0) {
                mux->ioWaitingThreshold = true;
            } else {
                BDBG_WRN(("NEXUS_FileMux_P_MuxTimer:%p BlockedOutput set without pending I/O transactions",(void *)mux));
            }
        }
        if(mux->muxTimer==NULL) {
            unsigned nextExecutionTime = muxStatus.uiNextExecutionTime;
            /* called from the timer  context */
            if(nextExecutionTime<30) {nextExecutionTime=30;}
            mux->muxTimer = NEXUS_ScheduleTimer(nextExecutionTime, NEXUS_FileMux_P_MuxTimer, mux);
            if(mux->muxTimer==NULL) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error; }
        }
    } else {
        BDBG_MODULE_MSG(nexus_file_mux_timer, ("MuxTimer:%p finished", (void*)mux));
        NEXUS_TaskCallback_Fire(mux->finishedCallback);
    }
    return;
error:
    return;
}

static NEXUS_Error
NEXUS_P_FileMux_Start_MP4( NEXUS_FileMuxHandle mux, const NEXUS_FileMuxStartSettings *pSettings, NEXUS_MuxFileHandle file)
{
    BMUXlib_File_MP4_StartSettings  startSettings;
    BERR_Code rc;
    unsigned i,j;

    BSTD_UNUSED(file);

    BDBG_ASSERT(pSettings->streamType==NEXUS_TransportType_eMp4);
    if(mux->mp4.mux==NULL) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;}

    BMUXlib_File_MP4_GetDefaultStartSettings(&startSettings);
    startSettings.uiExpectedDurationMs = pSettings->expectedDuration;
    startSettings.bProgressiveDownloadCompatible = pSettings->streamSettings.mp4.progressiveDownloadCompatible;
    startSettings.uiCreateTimeUTC = pSettings->streamSettings.mp4.createTime;
    startSettings.stOutput.pContext = &mux->outputFile;
    startSettings.stOutput.pfAddDescriptors = NEXUS_P_FileMux_AddStorageDescriptors;
    startSettings.stOutput.pfGetCompleteDescriptors = NEXUS_P_FileMux_GetCompletedStorageDescriptors;
    startSettings.stStorage.pContext = &mux->tempStorage;
    startSettings.stStorage.pfGetDefaultStorageSettings = NEXUS_P_FileMux_GetDefaultStorageSettings;
    startSettings.stStorage.pfCreateStorageObject = NEXUS_P_FileMux_CreateStorage;
    startSettings.stStorage.pfDestroyStorageObject = NEXIS_P_FileMux_DestroyStorage;
    for(i=j=0;i<NEXUS_MAX_MUX_PIDS && j<BMUXLIB_FILE_MP4_MAX_VIDEO_INPUTS;i++) {
        if(pSettings->video[i].encoder==NULL && pSettings->video[i].simpleEncoder==NULL) {
            break;
        }
        startSettings.stVideoInputs[j].uiTrackID = pSettings->video[i].track;
        startSettings.stVideoInputs[j].stInterface.stBufferInfo.eProtocol = NEXUS_P_VideoCodec_ToMagnum(pSettings->video[i].codec, NEXUS_TransportType_eEs);
        startSettings.stVideoInputs[j].stInterface.pContext = &mux->state.video[i];
        startSettings.stVideoInputs[j].stInterface.fGetBufferDescriptors = NEXUS_FileMux_P_GetVideoBufferDescriptors;
        startSettings.stVideoInputs[j].stInterface.fConsumeBufferDescriptors = NEXUS_FileMux_P_ConsumeVideoBufferDescriptors;
        startSettings.stVideoInputs[j].stInterface.fGetBufferStatus = NEXUS_FileMux_P_GetVideoBufferStatus;
        j++;
    }
    startSettings.uiNumVideoInputs = j;
    for(j=i=0;i<NEXUS_MAX_MUX_PIDS && j<BMUXLIB_FILE_MP4_MAX_AUDIO_INPUTS;i++) {
        if(pSettings->audio[i].muxOutput==NULL && pSettings->audio[i].simpleEncoder==NULL) {
            break;
        }
        startSettings.stAudioInputs[j].uiTrackID = pSettings->audio[i].track;
        startSettings.stAudioInputs[j].stInterface.stBufferInfo.eProtocol = NEXUS_P_AudioCodec_ToMagnum(pSettings->audio[i].codec);
        startSettings.stAudioInputs[j].stInterface.pContext = &mux->state.audio[i];
        startSettings.stAudioInputs[j].stInterface.fGetBufferDescriptors = NEXUS_FileMux_P_GetAudioBufferDescriptors;
        startSettings.stAudioInputs[j].stInterface.fConsumeBufferDescriptors = NEXUS_FileMux_P_ConsumeAudioBufferDescriptors;
        startSettings.stAudioInputs[j].stInterface.fGetBufferStatus = NEXUS_FileMux_P_GetAudioBufferStatus;
        j++;
    }
    startSettings.uiNumAudioInputs = j;

    rc = BMUXlib_File_MP4_Start(mux->mp4.mux, &startSettings);
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
    mux->mp4.active = true;

    return NEXUS_SUCCESS;
error:
    return rc;
}

static NEXUS_Error
NEXUS_P_FileMux_Start_PES( NEXUS_FileMuxHandle mux, const NEXUS_FileMuxStartSettings *pSettings, NEXUS_MuxFileHandle file)
{
    BMUXlib_File_PES_StartSettings  startSettings;
    BERR_Code rc;
    unsigned i,j;

    BSTD_UNUSED(file);

    BDBG_ASSERT(pSettings->streamType==NEXUS_TransportType_eMpeg2Pes);
    if(mux->pes.mux==NULL) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;}

    BMUXlib_File_PES_GetDefaultStartSettings(&startSettings);
    startSettings.stOutput.pContext = &mux->outputFile;
    startSettings.stOutput.pfAddDescriptors = NEXUS_P_FileMux_AddStorageDescriptors;
    startSettings.stOutput.pfGetCompleteDescriptors = NEXUS_P_FileMux_GetCompletedStorageDescriptors;
    for(i=j=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        if(pSettings->video[i].encoder==NULL && pSettings->video[i].simpleEncoder==NULL) {
            break;
        }
        if(j>0) {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        startSettings.stInterface.stBufferInfo.eProtocol = NEXUS_P_VideoCodec_ToMagnum(pSettings->video[i].codec, NEXUS_TransportType_eEs);
        startSettings.stInterface.pContext = &mux->state.video[i];
        startSettings.stInterface.fGetBufferDescriptors = NEXUS_FileMux_P_GetVideoBufferDescriptors;
        startSettings.stInterface.fConsumeBufferDescriptors = NEXUS_FileMux_P_ConsumeVideoBufferDescriptors;
        startSettings.stInterface.fGetBufferStatus = NEXUS_FileMux_P_GetVideoBufferStatus;
        j++;
    }
    if( !(pSettings->audio[0].muxOutput==NULL && pSettings->audio[0].simpleEncoder==NULL)) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    rc = BMUXlib_File_PES_Start(mux->pes.mux, &startSettings);
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
    mux->pes.active = true;

    return NEXUS_SUCCESS;
error:
    return rc;
}

static NEXUS_Error
NEXUS_P_FileMux_Start_IVF( NEXUS_FileMuxHandle mux, const NEXUS_FileMuxStartSettings *pSettings, NEXUS_MuxFileHandle file)
{
    BMUXlib_File_IVF_StartSettings  startSettings;
    BERR_Code rc;
    unsigned i,j;

    BSTD_UNUSED(file);

    if(mux->ivf.mux==NULL) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;}

    BMUXlib_File_IVF_GetDefaultStartSettings(&startSettings);
    startSettings.stOutput.pContext = &mux->outputFile;
    startSettings.stOutput.pfAddDescriptors = NEXUS_P_FileMux_AddStorageDescriptors;
    startSettings.stOutput.pfGetCompleteDescriptors = NEXUS_P_FileMux_GetCompletedStorageDescriptors;
    for(i=j=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        if(pSettings->video[i].encoder==NULL && pSettings->video[i].simpleEncoder==NULL) {
            break;
        }
        if(j>0) {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        startSettings.stInterface.stBufferInfo.eProtocol = NEXUS_P_VideoCodec_ToMagnum(pSettings->video[i].codec, NEXUS_TransportType_eEs);
        startSettings.stInterface.pContext = &mux->state.video[i];
        startSettings.stInterface.fGetBufferDescriptors = NEXUS_FileMux_P_GetVideoBufferDescriptors;
        startSettings.stInterface.fConsumeBufferDescriptors = NEXUS_FileMux_P_ConsumeVideoBufferDescriptors;
        startSettings.stInterface.fGetBufferStatus = NEXUS_FileMux_P_GetVideoBufferStatus;
        j++;
    }
    if( !(pSettings->audio[0].muxOutput==NULL && pSettings->audio[0].simpleEncoder==NULL)) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    rc = BMUXlib_File_IVF_Start(mux->ivf.mux, &startSettings);
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
    mux->ivf.active = true;

    return NEXUS_SUCCESS;
error:
    return rc;
}

static NEXUS_Error
NEXUS_P_FileMux_Start_ASF( NEXUS_FileMuxHandle mux, const NEXUS_FileMuxStartSettings *pSettings, NEXUS_MuxFileHandle file)
{
#if B_FILE_MUX_HAS_ASF
    BMUXlib_File_ASF_StartSettings  startSettings;
    BERR_Code rc;
    unsigned i,j;

    BSTD_UNUSED(file);

    BDBG_ASSERT(pSettings->streamType==NEXUS_TransportType_eAsf);
    if(mux->asf.mux==NULL) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;}

    BMUXlib_File_ASF_GetDefaultStartSettings(&startSettings);
    BKNI_Memcpy(startSettings.auiFileId, pSettings->streamSettings.asf.fileId,  sizeof(startSettings.auiFileId));
    startSettings.bBroadcastOnly = pSettings->streamSettings.asf.broadcastOnly;
    startSettings.stOutput.pContext = &mux->outputFile;
    startSettings.stOutput.pfAddDescriptors = NEXUS_P_FileMux_AddStorageDescriptors;
    startSettings.stOutput.pfGetCompleteDescriptors = NEXUS_P_FileMux_GetCompletedStorageDescriptors;
    startSettings.stStorage.pContext = &mux->tempStorage;
    startSettings.stStorage.pfGetDefaultStorageSettings = NEXUS_P_FileMux_GetDefaultStorageSettings;
    startSettings.stStorage.pfCreateStorageObject = NEXUS_P_FileMux_CreateStorage;
    startSettings.stStorage.pfDestroyStorageObject = NEXIS_P_FileMux_DestroyStorage;
    for(i=j=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        if(pSettings->video[i].encoder==NULL) {
            break;
        }
        if(pSettings->video[i].track > 127) {return BERR_TRACE(NEXUS_INVALID_PARAMETER);}
        startSettings.stVideoInputs[j].uiStreamNumber  = pSettings->video[i].track;
        startSettings.stVideoInputs[j].stInterface.stBufferInfo.eProtocol = NEXUS_P_VideoCodec_ToMagnum(pSettings->video[i].codec, NEXUS_TransportType_eEs);
        startSettings.stVideoInputs[j].stInterface.pContext = &mux->state.video[i];
        startSettings.stVideoInputs[j].stInterface.fGetBufferDescriptors = NEXUS_FileMux_P_GetVideoBufferDescriptors;
        startSettings.stVideoInputs[j].stInterface.fConsumeBufferDescriptors = NEXUS_FileMux_P_ConsumeVideoBufferDescriptors;
        startSettings.stVideoInputs[j].stInterface.fGetBufferStatus = NEXUS_FileMux_P_GetVideoBufferStatus;
        j++;
    }
    startSettings.uiNumVideoInputs = j;
    for(j=i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        if(pSettings->audio[i].muxOutput==NULL) {
            break;
        }
        if(pSettings->audio[i].track > 127) {return BERR_TRACE(NEXUS_INVALID_PARAMETER);}
        startSettings.stAudioInputs[j].uiStreamNumber = pSettings->audio[i].track;
        startSettings.stAudioInputs[j].stInterface.stBufferInfo.eProtocol = NEXUS_P_AudioCodec_ToMagnum(pSettings->audio[i].codec);
        startSettings.stAudioInputs[j].stInterface.pContext = &mux->state.audio[i];
        startSettings.stAudioInputs[j].stInterface.fGetBufferDescriptors = NEXUS_FileMux_P_GetAudioBufferDescriptors;
        startSettings.stAudioInputs[j].stInterface.fConsumeBufferDescriptors = NEXUS_FileMux_P_ConsumeAudioBufferDescriptors;
        startSettings.stAudioInputs[j].stInterface.fGetBufferStatus = NEXUS_FileMux_P_GetAudioBufferStatus;
        j++;
    }
    startSettings.uiNumAudioInputs = j;

    rc = BMUXlib_File_ASF_Start(mux->asf.mux, &startSettings);
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
    mux->asf.active = true;

    return NEXUS_SUCCESS;
error:
    return rc;
#else /* B_FILE_MUX_HAS_ASF */
    BSTD_UNUSED(mux);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(file);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}



NEXUS_Error
NEXUS_FileMux_Start( NEXUS_FileMuxHandle mux, const NEXUS_FileMuxStartSettings *pSettings, NEXUS_MuxFileHandle file)
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(mux, NEXUS_FileMux);
    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(file);

    mux->stopping = false;
    mux->ioWaitingThreshold = false;
    mux->startSettings = *pSettings;

    NEXUS_P_FileMux_File_Attach(&mux->outputFile, mux, file);
    switch(pSettings->streamType) {
    case NEXUS_TransportType_eMp4:
        rc = NEXUS_P_FileMux_Start_MP4(mux, pSettings, file);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
        break;
    case NEXUS_TransportType_eMpeg2Pes:
        if(mux->pes.mux) {
            rc = NEXUS_P_FileMux_Start_PES(mux, pSettings, file);
        } else {
            rc = NEXUS_P_FileMux_Start_IVF(mux, pSettings, file);
        }
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
        break;
    case NEXUS_TransportType_eAsf:
        rc = NEXUS_P_FileMux_Start_ASF(mux, pSettings, file);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
        break;
    default:
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error;
    }
    mux->muxTimer = NEXUS_ScheduleTimer(0, NEXUS_FileMux_P_MuxTimer, mux);
    if(mux->muxTimer==NULL) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error; }

    mux->started = true;
    mux->duration = 0;

    return NEXUS_SUCCESS;

error:
    return rc;
}

void
NEXUS_FileMux_Finish(NEXUS_FileMuxHandle mux)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(mux, NEXUS_FileMux);
    switch(mux->startSettings.streamType) {
    case NEXUS_TransportType_eMp4:
        if(mux->mp4.active) {
            BMUXlib_File_MP4_FinishSettings muxFinishSettings;
            BMUXlib_File_MP4_GetDefaultFinishSettings(&muxFinishSettings);
            rc = BMUXlib_File_MP4_Finish(mux->mp4.mux, &muxFinishSettings);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
        }
        break;
    case NEXUS_TransportType_eMpeg2Pes:
        if(mux->pes.active) {
            BMUXlib_File_PES_FinishSettings muxFinishSettings;
            BMUXlib_File_PES_GetDefaultFinishSettings(&muxFinishSettings);
            rc = BMUXlib_File_PES_Finish(mux->pes.mux, &muxFinishSettings);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
        } else if(mux->ivf.active) {
            BMUXlib_File_IVF_FinishSettings muxFinishSettings;
            BMUXlib_File_IVF_GetDefaultFinishSettings(&muxFinishSettings);
            rc = BMUXlib_File_IVF_Finish(mux->ivf.mux, &muxFinishSettings);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
        }
        break;
#if B_FILE_MUX_HAS_ASF
    case NEXUS_TransportType_eAsf:
        if(mux->asf.active) {
            BMUXlib_File_ASF_FinishSettings muxFinishSettings;
            BMUXlib_File_ASF_GetDefaultFinishSettings(&muxFinishSettings);
            rc = BMUXlib_File_ASF_Finish(mux->asf.mux, &muxFinishSettings);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
        }
        break;
#endif
    default:
        break;
    }

error:
    return;
}

void
NEXUS_FileMux_Stop(NEXUS_FileMuxHandle mux)
{
    BERR_Code rc;
    unsigned time;
    unsigned i;

    BDBG_OBJECT_ASSERT(mux, NEXUS_FileMux);
    if(!mux->started) {
        return;
    }
    mux->stopping = true;
    switch(mux->startSettings.streamType) {
    case NEXUS_TransportType_eMp4:
        if(mux->mp4.active) {
            rc = BMUXlib_File_MP4_Stop(mux->mp4.mux);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
        }
        break;
    case NEXUS_TransportType_eMpeg2Pes:
        if(mux->pes.active) {
            rc = BMUXlib_File_PES_Stop(mux->pes.mux);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
            mux->pes.active = false;
        } else if(mux->ivf.active) {
            rc = BMUXlib_File_IVF_Stop(mux->ivf.mux);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
            mux->ivf.active = false;
        }
        break;
#if B_FILE_MUX_HAS_ASF
    case NEXUS_TransportType_eAsf:
        if(mux->asf.active) {
            rc = BMUXlib_File_ASF_Stop(mux->asf.mux);
            if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
        }
        break;
#endif
    default:
        break;
    }
    if(mux->muxTimer) {
        NEXUS_CancelTimer(mux->muxTimer);
        mux->muxTimer = NULL;
    }
    for(time=0;mux->outputFile.ioInProgress || NEXUS_P_TempStorage_IsInProgress(&mux->tempStorage);) {
        NEXUS_P_TempStorage_CancelIo(&mux->tempStorage);
        mux->outputFile.ioCanceled = true;
        if(time<1000) {
            BDBG_ERR(("there are some I/O outstanding and they can't be canceled"));
            break;
        } else {
            BDBG_WRN(("there are some I/O outstanding wait for them to get completed .. %u", time));
        }
        NEXUS_UnlockModule();
        BKNI_Sleep(30);
        NEXUS_LockModule();
        time+=30;
    }

    mux->started = false;
    NEXUS_P_FileMux_File_Detach(&mux->outputFile);
    NEXUS_P_TempStorage_Clean(&mux->tempStorage);

    for(i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        NEXUS_P_FileMux_ManagedPtr_Release(&mux->state.audio[i].frame);
        NEXUS_P_FileMux_ManagedPtr_Release(&mux->state.audio[i].meta);

        NEXUS_P_FileMux_ManagedPtr_Release(&mux->state.video[i].frame);
        NEXUS_P_FileMux_ManagedPtr_Release(&mux->state.video[i].meta);
    }

    NEXUS_P_FileMux_MemoryBlock_Release(&mux->videoFrame);
    NEXUS_P_FileMux_MemoryBlock_Release(&mux->videoMeta);
    NEXUS_P_FileMux_MemoryBlock_Release(&mux->simpleVideoFrame);
    NEXUS_P_FileMux_MemoryBlock_Release(&mux->simpleVideoMeta);
    NEXUS_P_FileMux_MemoryBlock_Release(&mux->audioFrame);
    NEXUS_P_FileMux_MemoryBlock_Release(&mux->audioMeta);
    NEXUS_P_FileMux_MemoryBlock_Release(&mux->simpleAudioFrame);
    NEXUS_P_FileMux_MemoryBlock_Release(&mux->simpleAudioMeta);
    return;

error:
    return;
}

NEXUS_Error
NEXUS_FileMux_GetStatus( NEXUS_FileMuxHandle mux, NEXUS_FileMuxStatus *pStatus )
{
    BDBG_OBJECT_ASSERT(mux, NEXUS_FileMux);
    BDBG_ASSERT(pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->duration = mux->duration;

    return NEXUS_SUCCESS;
}
