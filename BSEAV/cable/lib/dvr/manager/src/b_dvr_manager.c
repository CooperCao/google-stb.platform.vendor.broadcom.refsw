/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/

#include "b_dvr_manager.h"
#include "b_dvr_manager_priv.h"

BDBG_MODULE(b_dvr_manager);
BDBG_OBJECT_ID(B_DVR_Manager);
static B_DVR_ManagerHandle g_dvrManager;

static void B_DVR_P_Scheduler(void * param)
{
    B_DVR_ManagerHandle dvrManager = param;
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("Starting Scheduler 0x%08x", (unsigned)dvrManager->scheduler));
    B_Scheduler_Run(dvrManager->scheduler);
    return;
}

B_DVR_ManagerHandle B_DVR_Manager_GetHandle(void)
{
    return g_dvrManager;
}

B_DVR_ManagerHandle B_DVR_Manager_Init(B_DVR_ManagerSettings *pSettings)
{
    B_DVR_ERROR rc =  B_DVR_SUCCESS;

    BDBG_MSG(("B_DVR_Manager_Init >>>"));
    g_dvrManager = BKNI_Malloc(sizeof (*g_dvrManager));
    if(!g_dvrManager)
    {
        goto error_alloc;
    }
    BKNI_Memset(g_dvrManager,0,sizeof(*g_dvrManager));
    g_dvrManager->memoryUsageMutex = B_Mutex_Create(NULL);
    if( !g_dvrManager->memoryUsageMutex)
    {
        BDBG_ERR(("%s memoryUsageMutex create error", __FUNCTION__));
        rc = B_DVR_OS_ERROR;
        goto error_mutex1;
    }

    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*g_dvrManager),true,
                                            __FUNCTION__,__LINE__);
    BDBG_OBJECT_SET(g_dvrManager,B_DVR_Manager);
    if(!pSettings)
    { 
        g_dvrManager->mediaSegmentSize = B_DVR_MEDIA_SEGMENT_SIZE;
    }
    else
    {
       g_dvrManager->mediaSegmentSize = pSettings->mediaSegmentSize;
    }
    g_dvrManager->dvrManagerMutex = B_Mutex_Create(NULL);
    if( !g_dvrManager->dvrManagerMutex)
    {
        BDBG_ERR(("%s dvrManagerMutex create error", __FUNCTION__));
        rc = B_DVR_OS_ERROR;
        goto error_mutex2;
    }
    g_dvrManager->scheduler = B_Scheduler_Create(NULL);
    if(!g_dvrManager->scheduler)
    {
        BDBG_ERR(("%s scheduler create error", __FUNCTION__));
        rc = B_DVR_OS_ERROR;
        goto error_scheduler;
    }
    g_dvrManager->thread = B_Thread_Create("DVRExtLib Scheduler", B_DVR_P_Scheduler,g_dvrManager, NULL);

    if(!g_dvrManager->thread)
    {
        BDBG_ERR(("%s scheduler thread create error", __FUNCTION__));
        rc = B_DVR_OS_ERROR;
        goto error_thread;
    }
    g_dvrManager->mediaStorage = B_DVR_MediaStorage_Open(NULL);
    if(!g_dvrManager->mediaStorage)
    {
        rc = B_DVR_UNKNOWN;
        BDBG_ERR(("%s B_DVR_MediaStorage_Open error", __FUNCTION__));
        goto error_mediaStorage;
    }

    g_dvrManager->dvrList = B_DVR_List_Open(g_dvrManager->mediaStorage);
    if(!g_dvrManager->dvrList)
    {
        rc = B_DVR_UNKNOWN;
        BDBG_ERR(("%s B_DVR_List_Open error", __FUNCTION__));
        goto error_dvrList;
    }
    BDBG_MSG(("B_DVR_Manager_Init <<<"));
    return g_dvrManager;
error_dvrList:
error_mediaStorage:
    B_Scheduler_Stop(g_dvrManager->scheduler);
    B_Thread_Destroy(g_dvrManager->thread);
error_thread:
    B_Scheduler_Destroy(g_dvrManager->scheduler);
error_scheduler:
    B_Mutex_Destroy(g_dvrManager->dvrManagerMutex);
error_mutex2: 
    B_Mutex_Destroy(g_dvrManager->memoryUsageMutex);
error_mutex1:
    BDBG_OBJECT_DESTROY(g_dvrManager,B_DVR_Manager);
    BKNI_Free(g_dvrManager);
error_alloc:
    BDBG_MSG(("B_DVR_Manager_Init err <<<"));
     return NULL;
}

B_DVR_ERROR B_DVR_Manager_UnInit(
    B_DVR_ManagerHandle manager)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_Manager_UnInit >>>>"));
    B_DVR_List_Close(g_dvrManager->dvrList);
    B_Scheduler_Stop(manager->scheduler);
    B_Thread_Destroy(manager->thread);
    B_Scheduler_Destroy(manager->scheduler);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*manager),false,
                                            __FUNCTION__,__LINE__);
    B_Mutex_Destroy(manager->dvrManagerMutex);
    B_Mutex_Destroy(g_dvrManager->memoryUsageMutex);
    BDBG_OBJECT_DESTROY(manager,B_DVR_Manager);
    BKNI_Free(manager);
    BDBG_MSG(("B_DVR_Manager_UnInit <<<<"));
    return rc;
}

B_DVR_ERROR B_DVR_Manager_CreateRecording(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(manager);
    BSTD_UNUSED(mediaNodeSettings);
    return rc;
}

B_DVR_ERROR B_DVR_Manager_DeleteRecording(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaNode mediaNode = NULL;
    B_DVR_SegmentedFileSettings fileSettings;
    #ifdef MEDIANODE_ONDEMAND_CACHING
    bool bCachedMediaNode = true;
    #endif
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_ASSERT(manager->mediaStorage);

    BDBG_MSG(("B_DVR_Manager_DeleteRecording >>>>"));
    B_Mutex_Lock(manager->dvrManagerMutex);
    
    mediaNode = B_DVR_List_GetMediaNode(manager->dvrList,mediaNodeSettings,false);    
    if (!mediaNode) 
    {
        #ifdef MEDIANODE_ONDEMAND_CACHING
        mediaNode = BKNI_Malloc(sizeof(B_DVR_Media));    
        if (!mediaNode) 
        {
            BDBG_ERR(("error in allocating for %s",mediaNodeSettings->programName));
            rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
            goto error;
        }
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),true,
                                                __FUNCTION__,__LINE__);
        rc = B_DVR_List_GetMediaNodeFile(manager->dvrList,mediaNodeSettings,mediaNode);
        if(rc!=B_DVR_SUCCESS) 
        {
            BDBG_ERR((".info file not found for %s",mediaNodeSettings->programName));
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),false,
                                                __FUNCTION__,__LINE__);
            BKNI_Free(mediaNode);
            goto error;
        }
        bCachedMediaNode = false;
        #else
        BDBG_ERR(("no mediaNode found for %s",mediaNodeSettings->programName));
        rc = B_DVR_INVALID_PARAMETER;
        goto error;
        #endif
    }
    if(mediaNode->refCount) 
    {
        #ifdef MEDIANODE_ONDEMAND_CACHING
        if(!bCachedMediaNode) 
        {
            BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),false,
                                                __FUNCTION__,__LINE__);
            BKNI_Free(mediaNode);
        }
        #endif
        rc = B_DVR_REC_IN_USE;
        BDBG_ERR(("program's recording is in use and can't be deleted"));
        goto error;
    }
    BDBG_MSG(("nav %s media %s info %s subDir %s",
              mediaNode->navFileName,
              mediaNode->mediaFileName,
              mediaNode->mediaNodeFileName,
              mediaNode->mediaNodeSubDir));
    BKNI_Memset((void*)(&fileSettings),0,sizeof(fileSettings));

    fileSettings.maxSegmentCount = 0; /* max segments would be found from metaData files */
    fileSettings.volumeIndex = mediaNodeSettings->volumeIndex;
    fileSettings.mediaStorage = manager->mediaStorage;
    fileSettings.metaDataSubDir = mediaNodeSettings->subDir;

    BDBG_MSG(("Segmented File Free >>>"));
    rc = B_DVR_SegmentedFileRecord_Free(mediaNode->mediaFileName,
                                        mediaNode->navFileName,
                                        &fileSettings);
    BDBG_MSG(("Segmented File Free <<<"));

    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("Segmented File Free failed"));
        #ifdef MEDIANODE_ONDEMAND_CACHING
        if(!bCachedMediaNode) 
        {
            BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),false,
                                                __FUNCTION__,__LINE__);
            BKNI_Free(mediaNode);
        }
        #endif
        goto error;
    }
    #ifdef MEDIANODE_ONDEMAND_CACHING
    if(bCachedMediaNode) 
    #endif
    {   /* no need to check the return value since the refCnt was checked earlier and
         * no service open or media file open would proceed till the dvr manager lock
         * is released 
         */
        B_DVR_List_RemoveMediaNode(manager->dvrList,mediaNodeSettings->volumeIndex,mediaNode);
    }
    #ifdef MEDIANODE_ONDEMAND_CACHING
    B_DVR_List_RemoveMediaNodeFile(manager->dvrList,mediaNodeSettings->volumeIndex,mediaNode);
    #endif
    BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaNode),false,
                                            __FUNCTION__,__LINE__);
    BKNI_Free(mediaNode);
error:
    BDBG_MSG(("B_DVR_Manager_DeleteRecording <<<<"));
    B_Mutex_Unlock(manager->dvrManagerMutex);
    return rc;
}

B_DVR_ERROR B_DVR_Manager_MoveRecording(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *src,
    char *destDir,
    char *destName)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaStorageHandle mediaStorage;
    B_DVR_MediaNode localMediaNode = NULL;
    char metadata[B_DVR_MAX_FILE_NAME_LENGTH];
    char source[B_DVR_MAX_FILE_NAME_LENGTH];
    char target[B_DVR_MAX_FILE_NAME_LENGTH];
    uint64_t sizeKbytes;
    uint64_t freeKbytes;
    uint64_t fileSizeKbytes;
    int segments;
    struct stat stat_buf;

    #ifdef MEDIANODE_ONDEMAND_CACHING
    bool bCachedMediaNode = true;
    #endif
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_ASSERT(destDir);
    BDBG_ASSERT(destName);
    BDBG_MSG(("B_DVR_Manager_MoveRecording >>>>"));

    B_Mutex_Lock(manager->dvrManagerMutex);
    mediaStorage = B_DVR_MediaStorage_GetHandle();

    rc = B_DVR_MediaStorage_GetMetadataPath(mediaStorage,src->volumeIndex,metadata);
    if (rc!= B_DVR_SUCCESS) {
        goto error;
    }
   
    snprintf(target,B_DVR_MAX_FILE_NAME_LENGTH,"%s/%s",metadata,(destDir=='\0')?"":destDir);
    if (stat (target, &stat_buf)!=0) { /* target dir does not exist */
        BDBG_ERR(("%s does not exist",target));
        rc=B_DVR_INVALID_PARAMETER;
        goto error;
    } else {
        BDBG_MSG(("target dir=%s",target));
    }

    /* get file size */
    localMediaNode = B_DVR_List_GetMediaNode(manager->dvrList,src,false);    
    if (localMediaNode==0) {
        #ifdef MEDIANODE_ONDEMAND_CACHING
        localMediaNode = BKNI_Malloc(sizeof(B_DVR_Media));
        if(!localMediaNode) 
        {
            BDBG_ERR(("error in allocating for %s",src->programName));
            rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
            goto error;
        }
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*localMediaNode),true,
                                            __FUNCTION__,__LINE__);
        rc = B_DVR_List_GetMediaNodeFile(manager->dvrList,src,localMediaNode);
        if(rc!=B_DVR_SUCCESS) 
        {
            BDBG_ERR((".info file not found for %s",src->programName));
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*localMediaNode),false,
                                            __FUNCTION__,__LINE__);
            BKNI_Free(localMediaNode);
            goto error;
        }
        bCachedMediaNode = false;
        #else
        BDBG_ERR(("B_DVR_List_GetMediaNode failed"));
        rc = B_DVR_INVALID_PARAMETER;
        goto error;
        #endif
    }
    if(localMediaNode->refCount)
    {
        #ifdef MEDIANODE_ONDEMAND_CACHING
        if(!bCachedMediaNode) 
        {
            BDBG_OBJECT_DESTROY(localMediaNode,B_DVR_Media);
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*localMediaNode),false,
                                                    __FUNCTION__,__LINE__);
            BKNI_Free(localMediaNode);
        }
        #endif
        rc = B_DVR_REC_IN_USE;
        BDBG_ERR(("program's recording is in use and can't be moved"));
        goto error;
    }
    fileSizeKbytes = (localMediaNode->mediaLinearEndOffset - localMediaNode->mediaLinearStartOffset)/1024;
    segments = fileSizeKbytes/(B_DVR_MEDIA_SEGMENT_SIZE/1024)+1;
    BDBG_MSG(("file size=%llu, segments=%d",fileSizeKbytes,segments));

    if (strncmp(src->subDir,destDir,B_DVR_MAX_FILE_NAME_LENGTH)) { /* if moving to another directory, update quota */
    /* check if quota is set in dest */
    rc = B_DVR_MediaStorage_GetDirQuota(mediaStorage,src->volumeIndex,destDir,&sizeKbytes,&freeKbytes);
    if (rc==B_DVR_SUCCESS) {   /* quota is set, update quota */
        rc = B_DVR_MediaStorage_UpdateDirQuota(mediaStorage,src->volumeIndex,destDir,segments);    
    if (rc!= B_DVR_SUCCESS) {
    #ifdef MEDIANODE_ONDEMAND_CACHING
    if (!bCachedMediaNode) {
        BDBG_OBJECT_DESTROY(localMediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*localMediaNode),false,
                                                __FUNCTION__,__LINE__);
        BKNI_Free(localMediaNode);
    }
    #endif
            BDBG_ERR(("unable to move from %s to %s:%d",src->subDir,destDir,rc));
        goto error;
    }
    }
    
    /* check if quota is set in src */
    rc = B_DVR_MediaStorage_GetDirQuota(mediaStorage,src->volumeIndex,src->subDir,&sizeKbytes,&freeKbytes);
    if (rc==B_DVR_SUCCESS) {   /* quota is set, update quota */
        rc = B_DVR_MediaStorage_UpdateDirQuota(mediaStorage,src->volumeIndex,src->subDir,segments*-1);    
        if (rc!=B_DVR_SUCCESS) {
            BDBG_ERR(("unable to move from %s to %s:%d",src->subDir,destDir,rc));
            /* ignore error */
        }
    }
    }

    snprintf(source,B_DVR_MAX_FILE_NAME_LENGTH,"%s/%s/%s",metadata,(src->subDir=='\0')?"":src->subDir,basename(localMediaNode->mediaNodeFileName));
    snprintf(target,B_DVR_MAX_FILE_NAME_LENGTH,"%s/%s/%s.info",metadata,(destDir=='\0')?"":destDir,destName);
    rename(source,target);
    BDBG_MSG(("%s->%s\n",source, target));

    snprintf(source,B_DVR_MAX_FILE_NAME_LENGTH,"%s/%s/%s",metadata,(src->subDir=='\0')?"":src->subDir,basename(localMediaNode->mediaFileName));
    snprintf(target,B_DVR_MAX_FILE_NAME_LENGTH,"%s/%s/%s.ts",metadata,(destDir=='\0')?"":destDir,destName);
    rename(source,target);
    BDBG_MSG(("%s->%s\n",source, target));

    snprintf(source,B_DVR_MAX_FILE_NAME_LENGTH,"%s/%s/%s",metadata,(src->subDir=='\0')?"":src->subDir,basename(localMediaNode->navFileName));
    snprintf(target,B_DVR_MAX_FILE_NAME_LENGTH,"%s/%s/%s.nav",metadata,(destDir=='\0')?"":destDir,destName);
    rename(source,target);
    BDBG_MSG(("%s->%s\n",source,src->programName, target, destName));

    /* update mediaNode info */
    memset(localMediaNode->programName,0,B_DVR_MAX_FILE_NAME_LENGTH);
    memset(localMediaNode->mediaNodeFileName,0,B_DVR_MAX_FILE_NAME_LENGTH);
    memset(localMediaNode->mediaFileName,0,B_DVR_MAX_FILE_NAME_LENGTH);
    memset(localMediaNode->navFileName,0,B_DVR_MAX_FILE_NAME_LENGTH);
    memset(localMediaNode->mediaNodeSubDir,0,B_DVR_MAX_FILE_NAME_LENGTH);

    strncpy(localMediaNode->programName,destName,B_DVR_MAX_FILE_NAME_LENGTH);
    localMediaNode->programName[B_DVR_MAX_FILE_NAME_LENGTH-1] = '\0';
    snprintf(localMediaNode->mediaNodeFileName,B_DVR_MAX_FILE_NAME_LENGTH,"%s.info",destName);
    snprintf(localMediaNode->mediaFileName,B_DVR_MAX_FILE_NAME_LENGTH,"%s.ts",destName);
    snprintf(localMediaNode->navFileName,B_DVR_MAX_FILE_NAME_LENGTH,"%s.nav",destName);
    snprintf(localMediaNode->mediaNodeSubDir,B_DVR_MAX_FILE_NAME_LENGTH,"%s",destDir);
    rc = B_DVR_List_UpdateMediaNode(manager->dvrList,localMediaNode,src->volumeIndex,true);

    #ifdef MEDIANODE_ONDEMAND_CACHING
    if(!bCachedMediaNode) 
    {
        BDBG_OBJECT_DESTROY(localMediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*localMediaNode),false,
                                                __FUNCTION__,__LINE__);
        BKNI_Free(localMediaNode);
    }
    #endif
   BDBG_MSG(("B_DVR_Manager_MoveRecording <<<<"));

error:
    B_Mutex_Unlock(manager->dvrManagerMutex);
    return rc;
}

B_DVR_ERROR B_DVR_Manager_GetVendorSpecificMetaData(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    void *vendorData,
    unsigned *size,
    unsigned offset)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_ASSERT(mediaNodeSettings);
    BDBG_ASSERT(vendorData);
    BDBG_ASSERT(size);
    BDBG_MSG(("B_DVR_Manager_GetVendorSpecificMetaData >>> "));
    B_Mutex_Lock(manager->dvrManagerMutex);
    rc = B_DVR_List_GetVendorSpecificMetaData(manager->dvrList,mediaNodeSettings,vendorData,size,offset);
    B_Mutex_Unlock(manager->dvrManagerMutex);
    BDBG_MSG(("B_DVR_Manager_GetVendorSpecificMetaData <<< "));
    return rc;
}

B_DVR_ERROR B_DVR_Manager_SetVendorSpecificMetaData(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    void *vendorData,
    unsigned *size)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_ASSERT(mediaNodeSettings);
    BDBG_ASSERT(vendorData);
    BDBG_ASSERT(size);
    BDBG_MSG(("B_DVR_Manager_SetVendorSpecificMetaData >>>>"));
    B_Mutex_Lock(manager->dvrManagerMutex);
    rc = B_DVR_List_SetVendorSpecificMetaData(manager->dvrList,mediaNodeSettings,vendorData,size);
    B_Mutex_Unlock(manager->dvrManagerMutex);
    BDBG_MSG(("B_DVR_Manager_SetVendorSpecificMetaData <<<<"));
    return rc;
}

unsigned  B_DVR_Manager_GetNumberOfRecordings(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings)
{

    unsigned returnedCount=0;
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_ASSERT(mediaNodeSettings);
    BDBG_MSG(("B_DVR_Manager_GetNumberOfRecordings >>>>"));
    B_Mutex_Lock(manager->dvrManagerMutex);
    returnedCount = B_DVR_List_GetNumberOfRecordings(manager->dvrList,mediaNodeSettings);
    B_Mutex_Unlock(manager->dvrManagerMutex);
    BDBG_MSG(("B_DVR_Manager_GetNumberOfRecordings <<<<"));
    return returnedCount;
}

unsigned B_DVR_Manager_GetRecordingList(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    unsigned listCount,
    char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH])
{
    unsigned returnedCount=0;
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_ASSERT(mediaNodeSettings);
    BDBG_MSG(("B_DVR_Manager_GetRecordingList >>>>"));
    B_Mutex_Lock(manager->dvrManagerMutex);
    returnedCount = B_DVR_List_GetRecordingList(manager->dvrList,mediaNodeSettings,listCount,recordingList);
    B_Mutex_Unlock(manager->dvrManagerMutex);
    BDBG_MSG(("B_DVR_Manager_GetRecordingList <<<<"));
    return returnedCount;
}

B_DVR_ERROR B_DVR_Manager_AllocSegmentedFileRecord(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    unsigned maxFileSegments)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_SegmentedFileSettings fileSettings;
    B_DVR_MediaNode mediaNode=NULL;
    
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_ASSERT(mediaNodeSettings);
    BDBG_MSG(("B_DVR_Manager_AllocSegmentedFile >>>"));
    B_Mutex_Lock(manager->dvrManagerMutex);
    mediaNode = BKNI_Malloc(sizeof(*mediaNode));
    if(!mediaNode)
    {
        BDBG_ERR(("mediaNode alloc failed"));
        rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
        goto alloc_mediaNodeError;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaNode),true,
                                            __FUNCTION__,__LINE__);
    BKNI_Memset((void*)mediaNode,0,sizeof(*mediaNode));
    BDBG_OBJECT_SET(mediaNode,B_DVR_Media);
    BKNI_Snprintf(mediaNode->navFileName, sizeof(mediaNode->navFileName),"%s%s",
                  mediaNodeSettings->programName,B_DVR_NAVIGATION_FILE_EXTENTION);
    BKNI_Snprintf(mediaNode->mediaFileName,sizeof(mediaNode->mediaFileName),"%s%s",
                  mediaNodeSettings->programName,B_DVR_MEDIA_FILE_EXTENTION);
    BKNI_Snprintf(mediaNode->mediaNodeFileName,sizeof(mediaNode->mediaNodeFileName),"%s%s",
                  mediaNodeSettings->programName,B_DVR_MEDIA_NODE_FILE_EXTENTION);

    strncpy(mediaNode->programName,mediaNodeSettings->programName,B_DVR_MAX_FILE_NAME_LENGTH);
    mediaNode->programName[B_DVR_MAX_FILE_NAME_LENGTH-1] = '\0';
    if(mediaNodeSettings->subDir)
    {
        strncpy(mediaNode->mediaNodeSubDir,mediaNodeSettings->subDir,B_DVR_MAX_FILE_NAME_LENGTH);
        mediaNode->mediaNodeSubDir[B_DVR_MAX_FILE_NAME_LENGTH-1] = '\0';
    }
    BDBG_MSG(("nav %s media %s info %s subDir %s",
              mediaNode->navFileName,
              mediaNode->mediaFileName,
              mediaNode->mediaNodeFileName,
              mediaNode->mediaNodeSubDir));

    
    mediaNode->recording = eB_DVR_RecordingTSB;
    mediaNode->mediaAttributes |= B_DVR_MEDIA_ATTRIBUTE_SEGMENTED_STREAM;

    #ifdef MEDIANODE_ONDEMAND_CACHING
    rc = B_DVR_List_AddMediaNodeFile(manager->dvrList,mediaNodeSettings->volumeIndex,mediaNode);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_WRN(("program %s already exists",mediaNodeSettings->programName));
        goto done;
    }
    #else
    rc = B_DVR_List_AddMediaNode(manager->dvrList,mediaNodeSettings->volumeIndex,mediaNode);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_WRN(("program %s already exists",mediaNodeSettings->programName));
        goto done;
    }
    #endif
    BKNI_Memset((void*)(&fileSettings),0,sizeof(fileSettings));

    fileSettings.maxSegmentCount = maxFileSegments;
    fileSettings.volumeIndex = mediaNodeSettings->volumeIndex;
    fileSettings.mediaStorage = manager->mediaStorage;
    fileSettings.metaDataSubDir = mediaNodeSettings->subDir;
    rc = B_DVR_SegmentedFileRecord_Alloc(mediaNode->mediaFileName,
                                         mediaNode->navFileName,
                                         &fileSettings);

    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("Segmented File Alloc failed"));
        rc = B_DVR_FILE_IO_ERROR;
        goto alloc_fileError;
    }
 done:
    #ifdef MEDIANODE_ONDEMAND_CACHING
    BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
    BKNI_Free(mediaNode);
    #endif
    B_Mutex_Unlock(manager->dvrManagerMutex);
    BDBG_MSG(("B_DVR_Manager_AllocSegmentedFile <<<"));
    return rc;
alloc_fileError:
    B_DVR_SegmentedFileRecord_Free(mediaNode->mediaFileName,
                                   mediaNode->navFileName,
                                   &fileSettings);
#ifdef MEDIANODE_ONDEMAND_CACHING
    B_DVR_List_RemoveMediaNodeFile(manager->dvrList,mediaNodeSettings->volumeIndex,mediaNode);
#else
    B_DVR_List_RemoveMediaNode(manager->dvrList,mediaNodeSettings->volumeIndex,mediaNode);
#endif
     BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
     B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaNode),false,
                                            __FUNCTION__,__LINE__);
     BKNI_Free(mediaNode);
alloc_mediaNodeError:
     B_Mutex_Unlock(manager->dvrManagerMutex);
     BDBG_MSG(("B_DVR_Manager_AllocSegmentedFile Error <<<"));
     return rc;
}

B_DVR_ERROR B_DVR_Manager_FreeSegmentedFileRecord(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaNode mediaNode = NULL;
    B_DVR_SegmentedFileSettings fileSettings;
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_ASSERT(mediaNodeSettings);
    BDBG_ASSERT(manager->mediaStorage);
    BDBG_MSG(("B_DVR_Manager_FreeSegmentedFileRecord >>>"));
    B_Mutex_Lock(manager->dvrManagerMutex);
    #ifdef MEDIANODE_ONDEMAND_CACHING
    mediaNode = BKNI_Malloc(sizeof(B_DVR_Media));
    if(!mediaNode) 
    {
       BDBG_ERR(("error in allocating for %s",mediaNodeSettings->programName));
       rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
       goto mediaNode_error;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaNode),true,
                                           __FUNCTION__,__LINE__);
    rc = B_DVR_List_GetMediaNodeFile(manager->dvrList,mediaNodeSettings,mediaNode);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR((".info file not found for %s",mediaNodeSettings->programName));
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),false,
                                                __FUNCTION__,__LINE__);
        BKNI_Free(mediaNode);
        mediaNode= NULL;
    }
    #else
    mediaNode = B_DVR_List_GetMediaNode(manager->dvrList,mediaNodeSettings,false);
    #endif
    if(!mediaNode)
    {
        BDBG_ERR(("mediaNode not found for program %s",mediaNodeSettings->programName));
        goto mediaNode_error;
    }

    BDBG_MSG(("nav %s media %s info %s subDir %s",
              mediaNode->navFileName,
              mediaNode->mediaFileName,
              mediaNode->mediaNodeFileName,
              mediaNode->mediaNodeSubDir));
    BKNI_Memset((void*)(&fileSettings),0,sizeof(fileSettings));

    fileSettings.maxSegmentCount = 0; /* max segments would be found from metaData files */
    fileSettings.volumeIndex = mediaNodeSettings->volumeIndex;
    fileSettings.mediaStorage = manager->mediaStorage;
    fileSettings.metaDataSubDir = mediaNodeSettings->subDir;

    BDBG_MSG(("Segmented File Free >>>"));
    rc = B_DVR_SegmentedFileRecord_Free(mediaNode->mediaFileName,
                                  mediaNode->navFileName,
                                  &fileSettings);
    BDBG_ERR(("Segmented File Free <<<"));

     if(rc!=B_DVR_SUCCESS)
     {
         BDBG_ERR(("Segmented File Free failed"));
         goto free_fileError;
     }

     BDBG_MSG(("B_DVR_List_RemoveMediaNode >>>"));
      /* no need to check the return value since the refCnt was checked earlier and
       * no service open or media file open would proceed till the dvr manager lock
       * is released 
       */
     #ifdef MEDIANODE_ONDEMAND_CACHING
     B_DVR_List_RemoveMediaNodeFile(manager->dvrList,mediaNodeSettings->volumeIndex,mediaNode);
     #else
     B_DVR_List_RemoveMediaNode(manager->dvrList,mediaNodeSettings->volumeIndex,mediaNode);
     #endif
     BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
     B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                             sizeof(*mediaNode),false,
                                             __FUNCTION__,__LINE__);
     BKNI_Free(mediaNode);
     BDBG_MSG(("B_DVR_List_RemoveMediaNode <<<"));
     B_Mutex_Unlock(manager->dvrManagerMutex);
     BDBG_MSG(("B_DVR_Manager_FreeSegmentedFileRecord <<<"));
     return rc;
free_fileError:
mediaNode_error:
    BDBG_MSG(("B_DVR_Manager_FreeSegmentedFileRecord <<< error"));
    B_Mutex_Unlock(manager->dvrManagerMutex);
    return rc;
}

B_DVR_ERROR B_DVR_Manager_PrintSegmentedFileRecord(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_SegmentedFileSettings fileSettings;
    B_DVR_MediaNode mediaNode;
    #ifdef MEDIANODE_ONDEMAND_CACHING
    bool bCachedMediaNode = true;
    #endif
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_ASSERT(mediaNodeSettings);
    BDBG_ASSERT(manager->mediaStorage);
    BDBG_MSG(("B_DVR_Manager_PrintSegmentedFileRecord >>>"));
    B_Mutex_Lock(manager->dvrManagerMutex);
    mediaNode = B_DVR_List_GetMediaNode(manager->dvrList,mediaNodeSettings,false);
    if(!mediaNode)
    {
        #ifdef MEDIANODE_ONDEMAND_CACHING
        mediaNode = BKNI_Malloc(sizeof(B_DVR_Media));
        if(!mediaNode) 
        {
            BDBG_ERR(("error in allocating for %s",mediaNodeSettings->programName));
            rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
            goto mediaNode_error;
        }
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),true,
                                                __FUNCTION__,__LINE__);
        rc = B_DVR_List_GetMediaNodeFile(manager->dvrList,mediaNodeSettings,mediaNode);
        if(rc!=B_DVR_SUCCESS)
        {
            BDBG_ERR((".info file not found for %s",mediaNodeSettings->programName));
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*mediaNode),false,
                                                    __FUNCTION__,__LINE__);
            BKNI_Free(mediaNode);
            goto mediaNode_error;
        }
        bCachedMediaNode = false;
        #else
        BDBG_ERR(("media node not found %s",mediaNodeSettings->programName));
        rc = B_DVR_INVALID_PARAMETER;
        goto mediaNode_error;
        #endif
    }

    BDBG_MSG(("nav %s media %s info %s subDir %s",
              mediaNode->navFileName,
              mediaNode->mediaFileName,
              mediaNode->mediaNodeFileName,
              mediaNode->mediaNodeSubDir));

    BKNI_Memset((void*)(&fileSettings),0,sizeof(fileSettings));

    fileSettings.maxSegmentCount = 0; /* max segments would be found from metaData files */
    fileSettings.volumeIndex = mediaNodeSettings->volumeIndex;
    fileSettings.mediaStorage = manager->mediaStorage;
    fileSettings.metaDataSubDir = mediaNodeSettings->subDir;
    rc = B_DVR_SegmentedFileRecord_Print(mediaNode->mediaFileName,
                                         mediaNode->navFileName,
                                         &fileSettings);
    #ifdef MEDIANODE_ONDEMAND_CACHING
    if(!bCachedMediaNode) 
    {
        BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),false,
                                                __FUNCTION__,__LINE__);
        BKNI_Free(mediaNode);
    }
    #endif
mediaNode_error:
    B_Mutex_Unlock(manager->dvrManagerMutex);
    BDBG_MSG(("B_DVR_Manager_PrintSegmentedFileRecord <<<<"));
    return rc;
}

B_DVR_ERROR B_DVR_Manager_GetMediaNode(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    B_DVR_MediaNode mediaNode)
{

    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaNode localMediaNode = NULL;
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_ASSERT(mediaNodeSettings);
    BDBG_MSG(("B_DVR_Manager_GetMediaNode >>>>"));
    B_Mutex_Lock(manager->dvrManagerMutex);
    localMediaNode = B_DVR_List_GetMediaNode(manager->dvrList,mediaNodeSettings,false);
    if(localMediaNode)
    {
        BKNI_Memcpy((void *)(mediaNode),(void *)(localMediaNode),sizeof(*mediaNode));
    }
    else
    {
        #ifdef MEDIANODE_ONDEMAND_CACHING
        rc = B_DVR_List_GetMediaNodeFile(manager->dvrList,mediaNodeSettings,mediaNode);
        if(rc!=B_DVR_SUCCESS) 
        {
            BDBG_ERR((".info file not found for %s",mediaNodeSettings->programName));
        }
        #else
        BDBG_MSG(("mediaNode not found"));
        rc = B_DVR_UNKNOWN;
        #endif
    }
    B_Mutex_Unlock(manager->dvrManagerMutex);
    BDBG_MSG(("B_DVR_Manager_GetMediaNode <<<<"));
    return rc;
}


B_DVR_ERROR B_DVR_Manager_CreateMediaNodeList(
    B_DVR_ManagerHandle manager,
    unsigned volumeIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_Manager_CreateMediaNodeList >>>>"));
    B_Mutex_Lock(manager->dvrManagerMutex);
    rc = B_DVR_List_Create(manager->dvrList,volumeIndex);
    B_Mutex_Unlock(manager->dvrManagerMutex);
    BDBG_MSG(("B_DVR_Manager_CreateMediaNodeList <<<<"));
    return rc;
}

B_DVR_ERROR B_DVR_Manager_DestroyMediaNodeList(
    B_DVR_ManagerHandle manager,
    unsigned volumeIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_Manager_DestroyMediaNodeList >>>>"));
    B_Mutex_Lock(manager->dvrManagerMutex);
    rc = B_DVR_List_Destroy(manager->dvrList,volumeIndex);
    B_Mutex_Unlock(manager->dvrManagerMutex);
    BDBG_MSG(("B_DVR_Manager_DestroyMediaNodeList <<<<"));
    return rc;
}

B_DVR_ERROR B_DVR_Manager_SetKeyBlobPerEsStream(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    unsigned pid,
    uint8_t *keyBlob)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaNode mediaNode = NULL;
    unsigned esStreamIndex;
    #ifdef MEDIANODE_ONDEMAND_CACHING
    bool bCachedMediaNode = true;
    #endif
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_ASSERT(mediaNodeSettings);
    BDBG_ASSERT(keyBlob);
    BDBG_MSG(("B_DVR_Manager_SetKeyBlobPerEsStream >>>"));
    B_Mutex_Lock(manager->dvrManagerMutex);
    mediaNode = B_DVR_List_GetMediaNode(manager->dvrList,mediaNodeSettings,false);
    if(!mediaNode)
    {
        #ifdef MEDIANODE_ONDEMAND_CACHING
        mediaNode = BKNI_Malloc(sizeof(B_DVR_Media));
        if(!mediaNode) 
        {
            BDBG_ERR(("error in allocating for %s",mediaNodeSettings->programName));
            rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
            goto error;
        }
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),true,
                                                __FUNCTION__,__LINE__);
        rc = B_DVR_List_GetMediaNodeFile(manager->dvrList,mediaNodeSettings,mediaNode);
        if(rc!=B_DVR_SUCCESS)
        {
            BDBG_ERR((".info file not found for %s",mediaNodeSettings->programName));
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*mediaNode),false,
                                                    __FUNCTION__,__LINE__);
            BKNI_Free(mediaNode);
            goto error;
        }
        bCachedMediaNode = false;
        #else
        BDBG_ERR(("media node not found %s",mediaNodeSettings->programName));
        rc = B_DVR_INVALID_PARAMETER;
        goto error;
        #endif
    }
    for(esStreamIndex=0;esStreamIndex < mediaNode->esStreamCount; esStreamIndex++)
    {
        if(mediaNode->esStreamInfo[esStreamIndex].pid == pid)
        {
            break;
        }
    }

    if(esStreamIndex >= mediaNode->esStreamCount)
    {
        #ifdef MEDIANODE_ONDEMAND_CACHING
        if(!bCachedMediaNode)
        {
            BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*mediaNode),false,
                                                    __FUNCTION__,__LINE__);
            BKNI_Free(mediaNode);
        }
        #endif
        BDBG_ERR(("pid %d not found",pid));
        rc = B_DVR_INVALID_PARAMETER;
        goto error;
    }
    BKNI_Memcpy((void *)&mediaNode->keyBlob[esStreamIndex][0],(void *)keyBlob,mediaNode->keyLength);
    B_DVR_List_UpdateMediaNode(manager->dvrList,mediaNode,mediaNodeSettings->volumeIndex,false);
    #ifdef MEDIANODE_ONDEMAND_CACHING
    if(!bCachedMediaNode) 
    {
        BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),false,
                                                __FUNCTION__,__LINE__);
        BKNI_Free(mediaNode);
    }
    #endif
error:
    B_Mutex_Unlock(manager->dvrManagerMutex);
    BDBG_MSG(("B_DVR_Manager_SetKeyBlobPerEsStream <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_Manager_GetKeyBlobPerEsStream(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    unsigned pid,
    uint8_t *keyBlob)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaNode mediaNode = NULL;
    unsigned esStreamIndex;
    #ifdef MEDIANODE_ONDEMAND_CACHING
    bool bCachedMediaNode = true;
    #endif
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_ASSERT(mediaNodeSettings);
    BDBG_MSG(("B_DVR_Manager_GetKeyBlobPerEsStream >>>"));
    B_Mutex_Lock(manager->dvrManagerMutex);
    mediaNode = B_DVR_List_GetMediaNode(manager->dvrList,mediaNodeSettings,false);
    if(!mediaNode)
    {
        #ifdef MEDIANODE_ONDEMAND_CACHING
        mediaNode = BKNI_Malloc(sizeof(B_DVR_Media));
        if(!mediaNode) 
        {
            BDBG_ERR(("error in allocating for %s",mediaNodeSettings->programName));
            rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
            goto error;
        }
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),true,
                                                __FUNCTION__,__LINE__);
        rc = B_DVR_List_GetMediaNodeFile(manager->dvrList,mediaNodeSettings,mediaNode);
        if(rc!=B_DVR_SUCCESS)
        {
            BDBG_ERR((".info file not found for %s",mediaNodeSettings->programName));
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*mediaNode),false,
                                                    __FUNCTION__,__LINE__);
            BKNI_Free(mediaNode);
            goto error;
        }
        bCachedMediaNode = false;
        #else
        BDBG_ERR(("media node not found %s",mediaNodeSettings->programName));
        rc = B_DVR_INVALID_PARAMETER;
        goto error;
        #endif
    }
    for(esStreamIndex=0;esStreamIndex < mediaNode->esStreamCount; esStreamIndex++)
    {
        if(mediaNode->esStreamInfo[esStreamIndex].pid == pid)
        {
            break;
        }
    }

    if(esStreamIndex >= mediaNode->esStreamCount)
    {
        #ifdef MEDIANODE_ONDEMAND_CACHING
        if(!bCachedMediaNode) 
        {
            BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*mediaNode),false,
                                                    __FUNCTION__,__LINE__);
            BKNI_Free(mediaNode);
        }
        #endif
        BDBG_ERR(("pid %d not found",pid));
        rc = B_DVR_INVALID_PARAMETER;
        goto error;
    }
    BKNI_Memcpy((void *)keyBlob,(void *)&mediaNode->keyBlob[esStreamIndex][0],mediaNode->keyLength);
    #ifdef MEDIANODE_ONDEMAND_CACHING
    if(!bCachedMediaNode) 
    {
        BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),false,
                                                __FUNCTION__,__LINE__);
        BKNI_Free(mediaNode);
    }
    #endif
error:
    B_Mutex_Unlock(manager->dvrManagerMutex);
    BDBG_MSG(("B_DVR_Manager_GetKeyBlobPerEsStream <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_Manager_SetKeyBlobLengthPerEsStream(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    unsigned keyLength)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaNode mediaNode = NULL;
    #ifdef MEDIANODE_ONDEMAND_CACHING
    bool bCachedMediaNode = true;
    #endif
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_ASSERT(mediaNodeSettings);
    BDBG_MSG(("B_DVR_Manager_SetKeyBlobLengthPerEsStream >>>"));
    B_Mutex_Lock(manager->dvrManagerMutex);
    mediaNode = B_DVR_List_GetMediaNode(manager->dvrList,mediaNodeSettings,false);
    if(!mediaNode)
    {
        #ifdef MEDIANODE_ONDEMAND_CACHING
        mediaNode = BKNI_Malloc(sizeof(B_DVR_Media));
        if(!mediaNode) 
        {
            BDBG_ERR(("error in allocating for %s",mediaNodeSettings->programName));
            rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
            goto error;
        }
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),true,
                                                __FUNCTION__,__LINE__);
        rc = B_DVR_List_GetMediaNodeFile(manager->dvrList,mediaNodeSettings,mediaNode);
        if(rc!=B_DVR_SUCCESS) 
        {
            BDBG_ERR((".info file not found for %s",mediaNodeSettings->programName));
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*mediaNode),false,
                                                    __FUNCTION__,__LINE__);
            BKNI_Free(mediaNode);
            goto error;
        }
        bCachedMediaNode=false;
        #else
        BDBG_ERR(("media node not found %s",mediaNodeSettings->programName));
        rc = B_DVR_INVALID_PARAMETER;
        goto error;
        #endif
    }
    mediaNode->keyLength = keyLength;
    B_DVR_List_UpdateMediaNode(manager->dvrList,mediaNode,mediaNodeSettings->volumeIndex,true);
    #ifdef MEDIANODE_ONDEMAND_CACHING
    if(!bCachedMediaNode) 
    {
        BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),false,
                                                __FUNCTION__,__LINE__);
        BKNI_Free(mediaNode);
    }
    #endif
error:
    B_Mutex_Unlock(manager->dvrManagerMutex);
    BDBG_MSG(("B_DVR_Manager_SetKeyBlobLengthPerEsStream <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_Manager_GetKeyBlobLengthPerEsStream(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    unsigned *keyLength)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaNode mediaNode = NULL;
    #ifdef MEDIANODE_ONDEMAND_CACHING
    bool bCachedMediaNode=true;
    #endif
    BDBG_OBJECT_ASSERT(manager,B_DVR_Manager);
    BDBG_ASSERT(mediaNodeSettings);
    BDBG_MSG(("B_DVR_Manager_GetKeyBlobLengthPerEsStream >>>"));
    B_Mutex_Lock(manager->dvrManagerMutex);
    mediaNode = B_DVR_List_GetMediaNode(manager->dvrList,mediaNodeSettings,false);
    if(!mediaNode)
    {
        #ifdef MEDIANODE_ONDEMAND_CACHING
        mediaNode = BKNI_Malloc(sizeof(B_DVR_Media));
        if(!mediaNode) 
        {
            BDBG_ERR(("error in allocating for %s",mediaNodeSettings->programName));
            rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
            goto error;
        }
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),true,
                                                __FUNCTION__,__LINE__);
        rc = B_DVR_List_GetMediaNodeFile(manager->dvrList,mediaNodeSettings,mediaNode);
        if(rc!=B_DVR_SUCCESS)
        {
            BDBG_ERR((".info file not found for %s",mediaNodeSettings->programName));
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*mediaNode),false,
                                                    __FUNCTION__,__LINE__);
            BKNI_Free(mediaNode);
            goto error;
        }
        bCachedMediaNode=false;
        #else
        BDBG_ERR(("media node not found %s",mediaNodeSettings->programName));
        rc = B_DVR_INVALID_PARAMETER;
        goto error;
        #endif
    }
    *keyLength = mediaNode->keyLength;
    #ifdef MEDIANODE_ONDEMAND_CACHING
    if(!bCachedMediaNode) 
    {
        BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),false,
                                                __FUNCTION__,__LINE__);
        BKNI_Free(mediaNode);
    }
    #endif
error:
    B_Mutex_Unlock(manager->dvrManagerMutex);
    BDBG_MSG(("B_DVR_Manager_GetKeyBlobLengthPerEsStream <<<"));
    return rc;
}
unsigned long B_DVR_Manager_GetMemoryUsage(
    B_DVR_ManagerHandle manager,
    B_DVR_MemoryType memoryType)
{
    unsigned memoryUsed=0;
    BDBG_ASSERT(manager);
    BDBG_MSG(("B_DVR_Manager_GetSystemMemoryUsage >>>"));
    B_Mutex_Lock(manager->memoryUsageMutex);
    if(memoryType == eB_DVR_MemoryType_System) 
    {
        memoryUsed = manager->systemMemoryUsed/1024;
    }
    else
    {
        if(memoryType == eB_DVR_MemoryType_Device) 
        {
            memoryUsed = manager->deviceMemoryUsed/1024;
        }
        else
        {
            BDBG_ERR(("%s: Invalid dvr memory type",__FUNCTION__));
        }
    }
    B_Mutex_Unlock(manager->memoryUsageMutex);
    BDBG_MSG(("B_DVR_Manager_GetSystemMemoryUsage memoryUsed %uKB <<<",memoryUsed));
    return memoryUsed;
}


