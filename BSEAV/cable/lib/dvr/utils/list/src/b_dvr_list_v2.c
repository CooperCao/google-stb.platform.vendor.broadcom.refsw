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
 * ****************************************************************************/
#include "b_dvr_list.h"
#include "b_dvr_mediastorage.h"
#include "b_dvr_manager_priv.h"

BDBG_MODULE(b_dvr_list);
BDBG_OBJECT_ID(B_DVR_Media);
struct B_DVR_List
{
     B_DVR_MediaStorageHandle mediaStorage;
     BLST_Q_HEAD(mediaNodelist,B_DVR_Media) recordingList[B_DVR_MEDIASTORAGE_MAX_VOLUME];
     char metaDataPath[B_DVR_MEDIASTORAGE_MAX_VOLUME][B_DVR_MAX_FILE_NAME_LENGTH];
     B_MutexHandle listMutex;
};

void dumpMediaNode(B_DVR_MediaNode mediaNode)
{
    unsigned esStreamIndex;
    BDBG_OBJECT_ASSERT(mediaNode,B_DVR_Media);
    BDBG_MSG(("program %s",mediaNode->programName));
    BDBG_MSG(("Media Node Name %s",mediaNode->mediaNodeFileName));
    BDBG_MSG(("Media MetaData %s",mediaNode->mediaFileName));
    BDBG_MSG(("Nav MetaData %s",mediaNode->navFileName));
    BDBG_MSG(("media Offset %lld %lld",mediaNode->mediaLinearStartOffset,mediaNode->mediaLinearEndOffset));
    BDBG_MSG(("media time %ld %ld",mediaNode->mediaStartTime,mediaNode->mediaEndTime));
    BDBG_MSG(("Nav offset %lld %lld",mediaNode->navLinearStartOffset,mediaNode->navLinearEndOffset));
    BDBG_MSG(("recording type %d",mediaNode->recording));
    BDBG_MSG(("Attributes %u",mediaNode->mediaAttributes));
    BDBG_MSG(("Version %u",mediaNode->version));
    BDBG_MSG(("refCount %u",mediaNode->refCount));
    for (esStreamIndex=0;esStreamIndex < mediaNode->esStreamCount;esStreamIndex++) 
    {
        BDBG_MSG(("%u:pid %u ",esStreamIndex,mediaNode->esStreamInfo[esStreamIndex]));
    }

    return;
}

bool B_DVR_List_P_IsDirectoryAvailable(B_DVR_ListHandle dvrList, unsigned volumeIndex, char *subDir)
{
    DIR *dir;
    struct dirent *entry;
    char metaDataPath[B_DVR_MAX_FILE_NAME_LENGTH];
    bool subDirAvailable = false;
    B_DVR_MediaStorage_GetMetadataPath(dvrList->mediaStorage,volumeIndex,metaDataPath);
    dir = opendir(metaDataPath);

    for(;;)
    {
        entry = readdir(dir);
        if(!entry)
        {
            break;
        }

        if((strcmp(entry->d_name,".")==0) || (strcmp(entry->d_name,"..")==0))
        {
            continue;
        }

        BDBG_ERR(("subDirectory %s type %d",entry->d_name,entry->d_type));

        if((strcmp(subDir,entry->d_name)==0))
        {
            subDirAvailable = true;
            break;
        }
    }
    closedir(dir);
    return subDirAvailable;
}

int B_DVR_List_P_FindFileWithSpecificExtention(
    char * name,
    const char * ext)
{
    int len1, len2;
    char * tmp;
    int rc;
    len1 = strlen(name);
    len2 = strlen(ext);
    if (len2 > len1 ) return 0;
    tmp = &name[len1 - len2];
    rc = (strncmp(tmp, ext, len2) ? 0 : 1);
    return rc;
}

B_DVR_ERROR B_DVR_List_P_GetMediaNodeFile(
    B_DVR_ListHandle dvrList,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    B_DVR_MediaNode mediaNode)
{
    char fullFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    int mediaNodeFileFD=0,len=0;
    unsigned volumeIndex = mediaNodeSettings->volumeIndex;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_ASSERT(dvrList);
    BDBG_ASSERT(mediaNodeSettings);
    BDBG_ASSERT(mediaNode);
    BDBG_MSG(("B_DVR_List_P_GetMediaNodeFile >>>"));
    if(mediaNodeSettings->subDir)
    {
        BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s/%s%s", dvrList->metaDataPath[volumeIndex],
                      mediaNodeSettings->subDir,mediaNodeSettings->programName,B_DVR_MEDIA_NODE_FILE_EXTENTION);
    }
    else
    {
        BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s%s", dvrList->metaDataPath[volumeIndex],
                      mediaNodeSettings->programName,B_DVR_MEDIA_NODE_FILE_EXTENTION);
    }
    BKNI_Memset(mediaNode, 0, sizeof(*mediaNode));
    if ((mediaNodeFileFD = open(fullFileName,O_RDONLY|O_CLOEXEC,0666)) < 0)
    {
        BDBG_MSG(("%s cannot open metadata file %s", __FUNCTION__, fullFileName));
        rc = B_DVR_INVALID_PARAMETER;
        goto error;
    }
    len = read( mediaNodeFileFD,(void *)mediaNode,sizeof(*mediaNode));
    if (len < (int)sizeof(*mediaNode))
    {
        BDBG_MSG(("%s empty or corrupted metadata file %s", __FUNCTION__, fullFileName));
        close(mediaNodeFileFD);
        goto error;
    }
    BDBG_OBJECT_SET(mediaNode,B_DVR_Media);
    mediaNode->refCount=0;     
    close(mediaNodeFileFD);
error:
    BDBG_MSG(("B_DVR_List_P_GetMediaNodeFile <<<"));
    return rc;
}


B_DVR_ListHandle B_DVR_List_Open(
    B_DVR_MediaStorageHandle mediaStorage)
{
    B_DVR_ListHandle dvrList=NULL;
    BDBG_MSG(("B_DVR_List_Open >>>>>"));
    dvrList = BKNI_Malloc(sizeof(*dvrList));
    if(!dvrList)
    {
        BDBG_ERR(("alloc failed for dvrList"));
        goto error;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*dvrList),true,
                                            __FUNCTION__, __LINE__);
    BKNI_Memset((void *)dvrList,0,sizeof(*dvrList));
    dvrList->mediaStorage = mediaStorage;
    dvrList->listMutex = B_Mutex_Create(NULL);
    if(!dvrList->listMutex)
    {
        BDBG_ERR(("dvrList->listMutex failed"));
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*dvrList),
                                                false, __FUNCTION__, __LINE__);
        BKNI_Free(dvrList);
        goto error;
    }
    BDBG_MSG(("B_DVR_List_Open <<<<"));
    return dvrList;
error:
    return NULL;
}

B_DVR_ERROR B_DVR_List_Close(B_DVR_ListHandle dvrList)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_Mutex_Destroy(dvrList->listMutex);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*dvrList),false,
                                            __FUNCTION__, __LINE__);
    BKNI_Free(dvrList);
    return rc;
}

B_DVR_ERROR B_DVR_List_Create(
    B_DVR_ListHandle dvrList,
    unsigned volumeIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaStorageStatus storageStatus;
    struct dnode;
    struct dirent *entry,*subEntry;
    struct stat stDirInfo;
    DIR *dir=NULL,*subDir=NULL;
    B_DVR_MediaNode mediaNode=NULL;
    char subDirPath[B_DVR_MAX_FILE_NAME_LENGTH];
    B_DVR_MediaNodeSettings mediaNodeSettings;

    BDBG_MSG(("B_DVR_List_Create >>>>>"));
    B_Mutex_Lock(dvrList->listMutex);
    B_DVR_MediaStorage_GetStatus(dvrList->mediaStorage,&storageStatus);
    if(!storageStatus.volumeInfo[volumeIndex].mounted)
    {
        BDBG_ERR((" volume isn't mounted"));
        rc = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    B_DVR_MediaStorage_GetMetadataPath(dvrList->mediaStorage,volumeIndex,dvrList->metaDataPath[volumeIndex]);
    BDBG_MSG(("metaDataPath for vol %u is %s",volumeIndex,dvrList->metaDataPath[volumeIndex]));

    BLST_Q_INIT(&dvrList->recordingList[volumeIndex]);
    
    dir = opendir(dvrList->metaDataPath[volumeIndex]);
    if(dir == NULL)
    {
        BDBG_ERR(("Cannot open metadata folder %s",storageStatus.volumeInfo->name,dvrList->metaDataPath[volumeIndex]));
        rc = B_DVR_INVALID_PARAMETER;
        goto error;
    }
    mediaNode = BKNI_Malloc(sizeof(*mediaNode));
    if(!mediaNode)
    {
        BDBG_ERR(("%s mediaNode alloc error",__FUNCTION__));
        rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
        closedir(dir);
        goto error;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaNode),true,
                                            __FUNCTION__, __LINE__);
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
        {
            if ((entry->d_name[1] == 0) || ( entry->d_name[1] == '.' && entry->d_name[2] == 0))
            continue;
        }
        BKNI_Snprintf(subDirPath,sizeof(subDirPath),"%s/%s",dvrList->metaDataPath[volumeIndex],entry->d_name);
        lstat((char *)subDirPath,&stDirInfo);
        if(!S_ISDIR(stDirInfo.st_mode))
        {
            if (!B_DVR_List_P_FindFileWithSpecificExtention(entry->d_name, B_DVR_MEDIA_NODE_FILE_EXTENTION))
            {
                continue;
            }
            BDBG_MSG((" mediaName %s",entry->d_name));
            entry->d_name[strlen(entry->d_name)-strlen(B_DVR_MEDIA_NODE_FILE_EXTENTION)] = '\0';
            mediaNodeSettings.volumeIndex = volumeIndex;
            mediaNodeSettings.subDir = NULL;
            mediaNodeSettings.programName = entry->d_name;
            rc = B_DVR_List_P_GetMediaNodeFile(dvrList,&mediaNodeSettings,mediaNode);
            if (rc!=B_DVR_SUCCESS)
            {
                BDBG_MSG(("B_DVR_List_P_GetMediaNodeFile failed for  %s",mediaNodeSettings.programName));
                continue;
            }
            if(mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS ||
               mediaNode->refCount)
            {
                if(mediaNode->recording == eB_DVR_RecordingTSB)
                {
                    BDBG_WRN(("reset the in progress flag for TSB program %s",mediaNode->programName));
                    mediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
                }
                else
                {
                    if(mediaNode->recording == eB_DVR_RecordingPermanent) 
                    {
                        BDBG_WRN(("change in progress flag to aborted flag for %s",mediaNode->programName));
                        mediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
                        mediaNode->mediaAttributes |= (B_DVR_MEDIA_ATTRIBUTE_RECORDING_ABORTED);
                    }
                }
                mediaNode->refCount = 0;
                B_DVR_List_UpdateMediaNode(dvrList,mediaNode,volumeIndex,true);
            }
            
        }
        else
        {
            subDir = opendir(subDirPath);
            while((subEntry = readdir(subDir))!=NULL)
            {
                /* skipping . and .. inodes in linux filesystem */
                if (subEntry->d_name[0] == '.')
                {
                    if ((subEntry->d_name[1] == 0) || ( subEntry->d_name[1] == '.' && subEntry->d_name[2] == 0))
                    continue;
                }
                if (!B_DVR_List_P_FindFileWithSpecificExtention(subEntry->d_name, B_DVR_MEDIA_NODE_FILE_EXTENTION))
                {
                    continue;
                }
                BDBG_MSG(("mediaName %s/%s",entry->d_name,subEntry->d_name));
                subEntry->d_name[strlen(subEntry->d_name)-strlen(B_DVR_MEDIA_NODE_FILE_EXTENTION)] = '\0';
                mediaNodeSettings.volumeIndex = volumeIndex;
                mediaNodeSettings.subDir = entry->d_name;
                mediaNodeSettings.programName = subEntry->d_name;
                rc = B_DVR_List_P_GetMediaNodeFile(dvrList,&mediaNodeSettings,mediaNode);
                if (rc!=B_DVR_SUCCESS)
                {
                    BDBG_MSG(("B_DVR_List_P_GetMediaNodeFile failed for %s/%s",
                              mediaNodeSettings.subDir,mediaNodeSettings.programName));
                    continue;
                }
                
                if(mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS ||
                   mediaNode->refCount)
                {
                    if(mediaNode->recording == eB_DVR_RecordingTSB)
                    {
                        BDBG_WRN(("reset the in progress flag for TSB program %s",mediaNode->programName));
                        mediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
                    }
                    else
                    {
                        if(mediaNode->recording == eB_DVR_RecordingPermanent) 
                        {
                            BDBG_WRN(("change in progress flag to aborted flag for %s",mediaNode->programName));
                            mediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
                            mediaNode->mediaAttributes |= (B_DVR_MEDIA_ATTRIBUTE_RECORDING_ABORTED);
                        }
                     }
                     mediaNode->refCount=0;
                     B_DVR_List_UpdateMediaNode(dvrList,mediaNode,volumeIndex,true);
                 }
             }
             closedir(subDir);
         }
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaNode),
                                            false, __FUNCTION__, __LINE__);
    BKNI_Free(mediaNode);
    closedir(dir);
error:
    B_Mutex_Unlock(dvrList->listMutex);
    BDBG_MSG(("B_DVR_List_Create <<<<"));
    return rc;
}

B_DVR_ERROR B_DVR_List_Destroy(
    B_DVR_ListHandle dvrList, unsigned volumeIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaNode mediaNode=NULL;
    B_DVR_MediaStorageStatus storageStatus;
    
    BDBG_ASSERT(dvrList);
    B_Mutex_Lock(dvrList->listMutex);
    B_DVR_MediaStorage_GetStatus(dvrList->mediaStorage,&storageStatus);
    if(!storageStatus.volumeInfo[volumeIndex].mounted)
    {
        BDBG_ERR(("volumeIndex isn't mounted"));
        rc = B_DVR_INVALID_PARAMETER;
        goto error;
    }
    while((mediaNode=BLST_Q_FIRST(&dvrList->recordingList[volumeIndex]))!=NULL)
    {
        BLST_Q_REMOVE(&dvrList->recordingList[volumeIndex],mediaNode,nextMedia);
        BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),false,
                                                 __FUNCTION__, __LINE__);
        BKNI_Free(mediaNode);
    }
error:
    B_Mutex_Unlock(dvrList->listMutex);
    return rc;
}

B_DVR_ERROR B_DVR_List_AddMediaNodeFile(
    B_DVR_ListHandle dvrList,
    unsigned volumeIndex,
    B_DVR_MediaNode newMediaNode)
{

    B_DVR_ERROR rc = B_DVR_SUCCESS;
    int mediaNodeFile;
    bool subDirAvailable = false;
    void * reservedBuf;
    char fullFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    BDBG_ASSERT(dvrList);
    BDBG_OBJECT_ASSERT(newMediaNode,B_DVR_Media);
    BDBG_MSG(("B_DVR_List_AddMediaNodeFile >>>"));
    B_Mutex_Lock(dvrList->listMutex);

    if(newMediaNode->mediaNodeSubDir[0]!='\0')
    { 
        subDirAvailable = B_DVR_List_P_IsDirectoryAvailable(dvrList,volumeIndex,newMediaNode->mediaNodeSubDir); 
        if(!subDirAvailable)
        {
            char subDirPath[B_DVR_MAX_FILE_NAME_LENGTH];
            BKNI_Snprintf(subDirPath, sizeof(subDirPath),"%s/%s", dvrList->metaDataPath[volumeIndex],newMediaNode->mediaNodeSubDir);
            if(mkdir(subDirPath,00777) < 0)
            {
                BDBG_MSG(("Unable to create directory %s",subDirPath));
                rc = B_DVR_OS_ERROR;
                goto error; 
           }
           else
           {
               BDBG_MSG(("created a subDirectory %s",subDirPath));
           }
       }
    }

    if(newMediaNode->mediaNodeSubDir[0]!='\0')
    { 
        BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s/%s", 
                      dvrList->metaDataPath[volumeIndex],newMediaNode->mediaNodeSubDir,
                      newMediaNode->mediaNodeFileName);
    }
    else
    {
        BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s", dvrList->metaDataPath[volumeIndex],newMediaNode->mediaNodeFileName);
    }

    BDBG_MSG(("mediaNode file with path %s",fullFileName));
    mediaNodeFile = open(fullFileName,O_CREAT|O_WRONLY|O_EXCL|O_CLOEXEC,0666);
    dumpMediaNode(newMediaNode);
    if(mediaNodeFile < 0)
    {
        BDBG_ERR(("Error in opening %s",fullFileName));
        rc = B_DVR_UNKNOWN;
        goto error;
    }
    newMediaNode->version = B_DVR_MEDIANODE_VERSION;
    write(mediaNodeFile, (void*)newMediaNode,sizeof(*newMediaNode));
    newMediaNode->reserved = true;
    newMediaNode->reservedDataCount = B_DVR_MEDIANODE_RESERVED_DATA_SIZE;
    reservedBuf = BKNI_Malloc(B_DVR_MEDIANODE_RESERVED_DATA_SIZE);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            B_DVR_MEDIANODE_RESERVED_DATA_SIZE,
                                            true, __FUNCTION__, __LINE__);
    if(reservedBuf)
    { 
        BKNI_Memset(reservedBuf,0,B_DVR_MEDIANODE_RESERVED_DATA_SIZE);
        write(mediaNodeFile,reservedBuf,B_DVR_MEDIANODE_RESERVED_DATA_SIZE);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                B_DVR_MEDIANODE_RESERVED_DATA_SIZE,
                                                false, __FUNCTION__, __LINE__);
        BKNI_Free(reservedBuf);
    }
    fdatasync(mediaNodeFile);
    close(mediaNodeFile);
error:
    B_Mutex_Unlock(dvrList->listMutex);
    BDBG_MSG(("B_DVR_List_AddMediaNodeFile >>>"));
    return rc;
}

B_DVR_ERROR B_DVR_List_AddMediaNode(
    B_DVR_ListHandle dvrList,
    unsigned volumeIndex,
    B_DVR_MediaNode newMediaNode)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_ASSERT(dvrList);
    BDBG_OBJECT_ASSERT(newMediaNode,B_DVR_Media);
    BDBG_MSG(("B_DVR_List_AddMediaNode >>>"));
    B_Mutex_Lock(dvrList->listMutex);
    BLST_Q_INSERT_TAIL(&dvrList->recordingList[volumeIndex],newMediaNode,nextMedia);
    newMediaNode->refCount=1;
    B_Mutex_Unlock(dvrList->listMutex);
    BDBG_MSG(("B_DVR_List_AddMediaNode refCount:%u>>>",newMediaNode->refCount));
    return rc;
}

void B_DVR_List_RemoveMediaNodeFile(
    B_DVR_ListHandle dvrList,
    unsigned volumeIndex,
    B_DVR_MediaNode mediaNode)
{
    char fullFileName[B_DVR_MAX_FILE_NAME_LENGTH];

    BDBG_MSG(("B_DVR_List_RemoveMediaNodeFile >>>"));
    BDBG_OBJECT_ASSERT(mediaNode,B_DVR_Media);
    B_Mutex_Lock(dvrList->listMutex);

    if(mediaNode->mediaNodeSubDir[0]=='\0')
    {
        BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s", dvrList->metaDataPath[volumeIndex],
                      mediaNode->mediaNodeFileName);
    }
    else
    {
        BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s/%s", dvrList->metaDataPath[volumeIndex],
                      mediaNode->mediaNodeSubDir,mediaNode->mediaNodeFileName);
    }
    unlink(fullFileName);
    BDBG_MSG(("Removed file %s",fullFileName));
    B_Mutex_Unlock(dvrList->listMutex);
    BDBG_MSG(("B_DVR_List_RemoveMediaNodeFile <<<"));
    return;
}

B_DVR_ERROR B_DVR_List_RemoveMediaNode(
    B_DVR_ListHandle dvrList,
    unsigned volumeIndex,
    B_DVR_MediaNode mediaNode)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_MSG(("B_DVR_List_RemoveMediaNode >>>"));
    BDBG_OBJECT_ASSERT(mediaNode,B_DVR_Media);
    B_Mutex_Lock(dvrList->listMutex);
    /* there is a possibility that a media node isn't used by any service or util */
    if(mediaNode->refCount) 
    {
        mediaNode->refCount--;
    }
    if(mediaNode->refCount) 
    {
        BDBG_MSG(("mediaNode %s refCount:%u non-zero",mediaNode->programName,mediaNode->refCount));
        rc = B_DVR_REC_IN_USE;
    }
    else
    {
        if (!(BLST_Q_EMPTY(&dvrList->recordingList[volumeIndex])))
        {
            BLST_Q_REMOVE(&dvrList->recordingList[volumeIndex],mediaNode,nextMedia);
        }
    }
    B_Mutex_Unlock(dvrList->listMutex);
    BDBG_MSG(("B_DVR_List_RemoveMediaNode <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_List_GetMediaNodeFile(
    B_DVR_ListHandle dvrList,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    B_DVR_MediaNode mediaNode)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_ASSERT(dvrList);
    BDBG_ASSERT(mediaNodeSettings);
    BDBG_ASSERT(mediaNode);
    BDBG_MSG(("B_DVR_List_GetMediaNode >>>"));
    B_Mutex_Lock(dvrList->listMutex);
    rc = B_DVR_List_P_GetMediaNodeFile(dvrList,mediaNodeSettings,mediaNode);
    B_Mutex_Unlock(dvrList->listMutex);
    BDBG_MSG(("B_DVR_List_GetMediaNode <<<"));
    return rc;
}

B_DVR_MediaNode B_DVR_List_GetMediaNode(
    B_DVR_ListHandle dvrList,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    bool incRefCount)
{
    B_DVR_MediaNode mediaNode=NULL;
    BDBG_ASSERT(dvrList);
    BDBG_ASSERT(mediaNodeSettings);

    BDBG_MSG(("B_DVR_List_GetMediaNode >>>"));
    B_Mutex_Lock(dvrList->listMutex);

    mediaNode=BLST_Q_FIRST(&dvrList->recordingList[mediaNodeSettings->volumeIndex]);

    for(;mediaNode!=NULL;mediaNode=BLST_Q_NEXT(mediaNode,nextMedia))
    {
        BDBG_OBJECT_ASSERT(mediaNode,B_DVR_Media);
        if(mediaNodeSettings->subDir)
        {
            if(!strncmp(mediaNodeSettings->subDir,mediaNode->mediaNodeSubDir,B_DVR_MAX_FILE_NAME_LENGTH) 
               && !strncmp(mediaNode->programName,mediaNodeSettings->programName,B_DVR_MAX_FILE_NAME_LENGTH))
            {
                BDBG_MSG(("mediaNode %s found in %s/%s",mediaNode->mediaNodeFileName,
                          dvrList->metaDataPath[mediaNodeSettings->volumeIndex],mediaNode->mediaNodeSubDir));
                break;
            }
        }
        else
        {
            if(!strncmp(mediaNode->programName,mediaNodeSettings->programName,B_DVR_MAX_FILE_NAME_LENGTH))
            {
                BDBG_MSG(("mediaNode %s found in %s ",mediaNode->mediaNodeFileName,
                          dvrList->metaDataPath[mediaNodeSettings->volumeIndex]));
                break;
            }
        }
    }

    if(mediaNode == NULL)
    {
        BDBG_MSG(("Couldn't find program %s's mediaNode in the cached list",mediaNodeSettings->programName));
    }
    else
    {
        if(incRefCount) 
        {
            mediaNode->refCount++;
        }
        dumpMediaNode(mediaNode);
        BDBG_MSG(("%s: refCount:%u ",__FUNCTION__,mediaNode->refCount));
    }
    
   BDBG_MSG(("B_DVR_List_GetMediaNode <<<"));
   B_Mutex_Unlock(dvrList->listMutex);
   return mediaNode;
}

B_DVR_ERROR B_DVR_List_UpdateMediaNode(
    B_DVR_ListHandle dvrList,
    B_DVR_MediaNode mediaNode,
    unsigned volumeIndex,
    bool sync)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    int mediaNodeFile;
    char fullFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    BDBG_MSG(("B_DVR_List_UpdateMediaNode >>>"));
    BDBG_OBJECT_ASSERT(mediaNode,B_DVR_Media);
    if(mediaNode->mediaNodeSubDir[0]=='\0')
    {
        BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s", dvrList->metaDataPath[volumeIndex],
                      mediaNode->mediaNodeFileName);
    }
    else
    {
        BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s/%s", dvrList->metaDataPath[volumeIndex],
                      mediaNode->mediaNodeSubDir,mediaNode->mediaNodeFileName);
    }
    BDBG_MSG(("Updating media node file %s",fullFileName));
    mediaNodeFile = open(fullFileName,O_WRONLY|O_CLOEXEC,0666);
    if(mediaNodeFile < 0)
    {
        BDBG_ERR(("Error in opening %s",fullFileName));
        rc = B_DVR_UNKNOWN;
        goto error;
    }
    write(mediaNodeFile, (void*)mediaNode,sizeof(*mediaNode));
    if(sync) 
    {
        fdatasync(mediaNodeFile);
    }
    close(mediaNodeFile);
error:
    BDBG_MSG(("B_DVR_List_UpdateMediaNode <<<<"));
    return rc;
}

unsigned B_DVR_List_GetNumberOfRecordings(
    B_DVR_ListHandle dvrList,
    B_DVR_MediaNodeSettings *mediaNodeSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned index=0;
    struct dnode;
    struct dirent *entry;
    struct stat stDirInfo;
    DIR *dir=NULL;
    char subDirPath[B_DVR_MAX_FILE_NAME_LENGTH];
    unsigned volumeIndex = mediaNodeSettings->volumeIndex;
    B_DVR_MediaNode mediaNode = NULL;
    BDBG_ASSERT(dvrList);
    BDBG_MSG(("B_DVR_List_GetNumberOfRecordings >>>"));
    B_Mutex_Lock(dvrList->listMutex);
    if(mediaNodeSettings->subDir)
    {
        BKNI_Snprintf(subDirPath,sizeof(subDirPath),"%s/%s",dvrList->metaDataPath[volumeIndex],mediaNodeSettings->subDir);
    }
    else
    {
        strcpy(subDirPath,dvrList->metaDataPath[volumeIndex]);
    }
    dir = opendir(subDirPath);
    if(dir == NULL)
    {
        BDBG_ERR(("Cannot open metadata folder %s",subDirPath));
        goto error;
    }
    mediaNode = BKNI_Malloc(sizeof(*mediaNode));
    if (!mediaNode)
    {
        BDBG_ERR(("%s mediaNode alloc failed", __FUNCTION__));
        closedir(dir);
        goto error;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaNode),true,
                                            __FUNCTION__, __LINE__);
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
        {
            if ((entry->d_name[1] == 0) || ( entry->d_name[1] == '.' && entry->d_name[2] == 0))
            continue;
        }
        BKNI_Snprintf(subDirPath,sizeof(subDirPath),"%s/%s",dvrList->metaDataPath[volumeIndex],entry->d_name);
        lstat((char *)subDirPath,&stDirInfo);
        if(!S_ISDIR(stDirInfo.st_mode) && 
           B_DVR_List_P_FindFileWithSpecificExtention(entry->d_name, B_DVR_MEDIA_NODE_FILE_EXTENTION))
        {
            BDBG_MSG((" mediaName %s",entry->d_name));
            entry->d_name[strlen(entry->d_name)-strlen(B_DVR_MEDIA_NODE_FILE_EXTENTION)] = '\0';
            mediaNodeSettings->programName = entry->d_name;
            rc = B_DVR_List_P_GetMediaNodeFile(dvrList,mediaNodeSettings,mediaNode);
            if(rc==B_DVR_SUCCESS) 
            {
                BDBG_MSG((" mediaName %s is counted, index %d",entry->d_name, index));
                index++;
            }
            else
            {
                BDBG_ERR(("unable to get mediaNode from %s",mediaNodeSettings->programName));
            }
        }
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaNode),
                                            false, __FUNCTION__, __LINE__);
    BKNI_Free(mediaNode);
    closedir(dir);
error:
    B_Mutex_Unlock(dvrList->listMutex);
    BDBG_MSG(("B_DVR_List_GetNumberOfRecordings %u <<<",index));
    return index;
}

unsigned B_DVR_List_GetRecordingList(
    B_DVR_ListHandle dvrList,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    unsigned listCount,
    char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH])
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned index=0;
    struct dnode;
    struct dirent *entry;
    struct stat stDirInfo;
    DIR *dir=NULL;
    char subDirPath[B_DVR_MAX_FILE_NAME_LENGTH];
    unsigned volumeIndex = mediaNodeSettings->volumeIndex;
    B_DVR_MediaNode mediaNode = NULL;
    BDBG_ASSERT(dvrList);
    BDBG_MSG(("B_DVR_List_GetRecordingList >>>"));
    B_Mutex_Lock(dvrList->listMutex);
    if(mediaNodeSettings->subDir)
    {
        BKNI_Snprintf(subDirPath,sizeof(subDirPath),"%s/%s",dvrList->metaDataPath[volumeIndex],mediaNodeSettings->subDir);
    }
    else
    {
        strcpy(subDirPath,dvrList->metaDataPath[volumeIndex]);
    }
    dir = opendir(subDirPath);
    if(dir == NULL)
    {
        BDBG_ERR(("Cannot open metadata folder %s",subDirPath));
        goto error;
    }

    mediaNode = BKNI_Malloc(sizeof(*mediaNode));
    if (!mediaNode)
    {
        BDBG_ERR(("%s mediaNode alloc failed", __FUNCTION__));
        closedir(dir);
        goto error;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaNode),
                                            true, __FUNCTION__, __LINE__);
    while ((entry = readdir(dir)) != NULL)
    {
        if(index > listCount)
        {
            break;
        }
        if (entry->d_name[0] == '.')
        {
            if ((entry->d_name[1] == 0) || ( entry->d_name[1] == '.' && entry->d_name[2] == 0))
            continue;
        }
        BKNI_Snprintf(subDirPath,sizeof(subDirPath),"%s/%s",dvrList->metaDataPath[volumeIndex],entry->d_name);
        lstat((char *)subDirPath,&stDirInfo);
        if(!S_ISDIR(stDirInfo.st_mode) && 
           B_DVR_List_P_FindFileWithSpecificExtention(entry->d_name, B_DVR_MEDIA_NODE_FILE_EXTENTION))
        {
            BDBG_MSG((" mediaName %s",entry->d_name));
            entry->d_name[strlen(entry->d_name)-strlen(B_DVR_MEDIA_NODE_FILE_EXTENTION)] = '\0';
            mediaNodeSettings->programName = entry->d_name;
            rc = B_DVR_List_P_GetMediaNodeFile(dvrList,mediaNodeSettings,mediaNode);
            
            if (rc==B_DVR_SUCCESS) 
            {
                strncpy(recordingList[index],entry->d_name,sizeof(entry->d_name));
                index++;
                BDBG_WRN((" mediaName %s is added to list, index %d",entry->d_name, index));
            }
            else
            {
                BDBG_ERR(("unabled to get mediaNode from %s",mediaNodeSettings->programName));
            }
        }
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaNode),false,
                                            __FUNCTION__, __LINE__);
    BKNI_Free(mediaNode);
    closedir(dir);
error:
    B_Mutex_Unlock(dvrList->listMutex);
    BDBG_MSG(("B_DVR_List_GetRecordingList <<<"));
    return index;
}

B_DVR_ERROR B_DVR_List_GetVendorSpecificMetaData(
    B_DVR_ListHandle dvrList,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    void *vendorData,
    unsigned *size,
    unsigned offset)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    int mediaNodeFile;
    B_DVR_MediaNode mediaNode = NULL;
    off_t returnOffset=0;
    ssize_t sizeRead=0;
    char fullFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    bool bCachedMediaNode=true;

    BDBG_MSG(("B_DVR_List_GetVendorSpecificMetaData >>>>"));
    mediaNode = B_DVR_List_GetMediaNode(dvrList,mediaNodeSettings,false);

    if(!mediaNode) 
    {
        if ((mediaNode = BKNI_Malloc(sizeof(*mediaNode))) == NULL)
        {
            BDBG_ERR(("%s media node allocation failed", __FUNCTION__));
            rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
            goto error1;
        }
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),true,
                                                __FUNCTION__, __LINE__);
        rc = B_DVR_List_GetMediaNodeFile(dvrList,mediaNodeSettings,mediaNode);
        if(rc!=B_DVR_SUCCESS) 
        {
            rc = B_DVR_INVALID_PARAMETER;
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*mediaNode),false,
                                                    __FUNCTION__, __LINE__);
            BKNI_Free(mediaNode);
            if(mediaNodeSettings->subDir)
            {
                BDBG_ERR(("mediaNode not found for %s/%s",mediaNodeSettings->subDir,mediaNodeSettings->programName));
            }
            else
            {
                BDBG_ERR(("mediaNode not found for %s",mediaNodeSettings->programName));
            }
            goto error1;
        }
        bCachedMediaNode = false;
    }

    B_Mutex_Lock(dvrList->listMutex);
    BDBG_OBJECT_ASSERT(mediaNode,B_DVR_Media);
    if(mediaNodeSettings->subDir)
    { 
        BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s/%s", 
                      dvrList->metaDataPath[mediaNodeSettings->volumeIndex],
                      mediaNodeSettings->subDir,mediaNode->mediaNodeFileName);
    }
    else
    {
        BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s", 
                      dvrList->metaDataPath[mediaNodeSettings->volumeIndex],
                      mediaNode->mediaNodeFileName);
    }
    BDBG_MSG(("mediaNode file with path %s",fullFileName));
    mediaNodeFile = open(fullFileName,O_RDONLY|O_CLOEXEC,0666);
    if(mediaNodeFile < 0)
    {
        BDBG_ERR(("Error in opening %s",fullFileName));
        rc = B_DVR_UNKNOWN;
        goto error2;
    }

    returnOffset = lseek(mediaNodeFile, (sizeof(*mediaNode)+mediaNode->reservedDataCount + offset),SEEK_SET);
    if(returnOffset!= (sizeof(*mediaNode)+mediaNode->reservedDataCount+offset))
    {
        BDBG_ERR(("seek error in %s",mediaNode->mediaNodeFileName));
        *size = 0;
        rc = B_DVR_OUT_OF_DEVICE_MEMORY;
        goto error3;
    }
    sizeRead = read(mediaNodeFile, (void*)vendorData,*size);
    *size = (unsigned)sizeRead;
error3:
    close(mediaNodeFile);
error2:
    if(!bCachedMediaNode)
    {
        BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),false,
                                                __FUNCTION__, __LINE__);
        BKNI_Free(mediaNode);
    }
    B_Mutex_Unlock(dvrList->listMutex);
error1:
    BDBG_MSG(("B_DVR_List_GetVendorSpecificMetaData <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_List_SetVendorSpecificMetaData(
     B_DVR_ListHandle dvrList,
     B_DVR_MediaNodeSettings *mediaNodeSettings,
     void *vendorData,
     unsigned *size)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    int mediaNodeFile;
    B_DVR_MediaNode mediaNode = NULL;
    off_t returnOffset=0;
    ssize_t sizeWritten=0;
    char fullFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    bool bCachedMediaNode = true;
    BDBG_MSG(("B_DVR_List_SetVendorSpecificMetaData >>>>"));
    mediaNode = B_DVR_List_GetMediaNode(dvrList,mediaNodeSettings,false);
    if(!mediaNode) 
    {
        if ((mediaNode = BKNI_Malloc(sizeof(*mediaNode))) == NULL)
        {
            BDBG_ERR(("%s media node allocation failed", __FUNCTION__));
            rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
            goto error1;
        }
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),true,
                                                __FUNCTION__, __LINE__);
        rc = B_DVR_List_GetMediaNodeFile(dvrList,mediaNodeSettings,mediaNode);
        if(rc!=B_DVR_SUCCESS) 
        {
            rc = B_DVR_INVALID_PARAMETER;
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*mediaNode),false,
                                                    __FUNCTION__, __LINE__);
            BKNI_Free(mediaNode);
            if(mediaNodeSettings->subDir)
            {
                BDBG_ERR(("mediaNode not found for %s/%s",mediaNodeSettings->subDir,mediaNodeSettings->programName));
            }
            else
            {
                BDBG_ERR(("mediaNode not found for %s",mediaNodeSettings->programName));
            }
            goto error1;
        }
        bCachedMediaNode = false;
    }
    B_Mutex_Lock(dvrList->listMutex);
    BDBG_OBJECT_ASSERT(mediaNode,B_DVR_Media);
    if(mediaNodeSettings->subDir)
    { 
        BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s/%s", 
                      dvrList->metaDataPath[mediaNodeSettings->volumeIndex],
                      mediaNodeSettings->subDir,mediaNode->mediaNodeFileName);
    }
    else
    {
        BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s", 
                      dvrList->metaDataPath[mediaNodeSettings->volumeIndex],
                      mediaNode->mediaNodeFileName);
    }
    BDBG_MSG(("mediaNode file with path %s",fullFileName));
    mediaNodeFile = open(fullFileName,O_WRONLY|O_CLOEXEC,0666);
    if(mediaNodeFile < 0)
    {
        BDBG_ERR(("Error in opening %s",fullFileName));
        rc = B_DVR_UNKNOWN;
        goto error2;
    }

    returnOffset = lseek(mediaNodeFile, sizeof(*mediaNode)+mediaNode->reservedDataCount,SEEK_SET);
    if(returnOffset!= sizeof(*mediaNode) + mediaNode->reservedDataCount)
    {
        BDBG_ERR(("%s: error in seeking %s",__FUNCTION__,mediaNode->mediaNodeFileName));
        rc = B_DVR_FILE_IO_ERROR;
        goto error3;
    }
    sizeWritten = write(mediaNodeFile,vendorData,*size);
    *size = (unsigned) sizeWritten;
    mediaNode->maxCustomDataCount = *size;
    mediaNode->customData = true;
    returnOffset = lseek(mediaNodeFile,0x0,SEEK_SET);
    if(returnOffset!=0)
    {
        BDBG_ERR(("%s: error in seeking %s",__FUNCTION__,mediaNode->mediaNodeFileName));
        rc = B_DVR_FILE_IO_ERROR;
        goto error3;
    }
    write(mediaNodeFile, (void*)mediaNode,sizeof(*mediaNode));
    fdatasync(mediaNodeFile);
error3:
    close(mediaNodeFile);
error2:
    if(!bCachedMediaNode)
    {
        BDBG_OBJECT_DESTROY(mediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaNode),false,
                                                __FUNCTION__, __LINE__);
        BKNI_Free(mediaNode);
    }
    B_Mutex_Unlock(dvrList->listMutex);
error1:
    BDBG_MSG(("B_DVR_List_SetVendorSpecificMetaData <<<<"));
    return rc;
}

