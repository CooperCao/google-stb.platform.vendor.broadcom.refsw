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
#include "b_dvr_segmentedfile.h"
#include "bfile_io.h"
#include "bcmindexerpriv.h"
#include "b_dvr_manager_priv.h"
BDBG_MODULE(b_dvr_segmentedfile);
BDBG_OBJECT_ID(B_DVR_SegmentedFileNode);
#define SEGMENTED_FILEIO_TIMING 0
#define BDBG_MSG_TRACE(X)  /* BDBG_MSG(X) */
static B_DVR_ERROR dumpFileNodeList(B_DVR_SegmentedFileHandle segmentedFile);
static void dumpSegmentedFileRecord(B_DVR_SegmentedFileRecordHandle segmentedFileRecord);
static B_DVR_ERROR writeMetaDataFile(B_DVR_FileHandle metaDataFile,B_DVR_SegmentedFileNode *updatedFileNode);
static B_DVR_SegmentedFileNode * findPlayFileNode(B_DVR_SegmentedFileHandle segmentedFile);
static off_t segmented_posix_seekForWrite(bfile_io_write_t fd, off_t offset, int whence);
static off_t findTSBRecStartOffset(B_DVR_SegmentedFileHandle segmentedFile);
static B_DVR_SegmentedFileNode * allocateFileSegment(B_DVR_MediaStorageHandle mediaStorage, 
                                                     B_DVR_FileType fileType,
                                                     unsigned volumeIndex,
                                                     const char *subDir);
static B_DVR_ERROR populateFileList(B_DVR_SegmentedFileHandle segmentedFile);
static B_DVR_ERROR updateFileList(B_DVR_SegmentedFileHandle segmentedFile);
static void freeFileSegment(B_DVR_MediaStorageHandle mediaStorage,
                             B_DVR_FileType fileType,
                             unsigned volumeIndex,
                             const char *subDir,
                             B_DVR_SegmentedFileNode *fileNode);
static int checkMetaDataFile(const char *fileName, 
                             B_DVR_SegmentedFileSettings *pSettings,
                             B_DVR_FileType fileType);
static void checkFileSegmentPair(B_DVR_SegmentedFileRecordHandle segmentedFileRecord);


static void checkFileSegmentPair(B_DVR_SegmentedFileRecordHandle segmentedFileRecord)
{
    off_t navFileSize = 0,mediaFileSize=0;
    unsigned navFileCount = 0;
    unsigned mediaFileCount = 0;
    unsigned mediaAlignment = 4096*188;/*alignment for media segment*/

    if(segmentedFileRecord->navWrite->pCachedFileNode)
    {
        navFileSize = segmentedFileRecord->navWrite->pCachedFileNode->size;
    }
    if(segmentedFileRecord->mediaWrite->pCachedFileNode)
    {
        mediaFileSize = segmentedFileRecord->mediaWrite->pCachedFileNode->size;
    }

    navFileCount = segmentedFileRecord->navWrite->fileCount;
    mediaFileCount = segmentedFileRecord->mediaWrite->fileCount;

    if( (mediaFileCount == navFileCount)
        && ((navFileSize + 2*segmentedFileRecord->itbSize) >= B_DVR_NAV_SEGMENT_SIZE)
        && ((mediaFileSize + 2*segmentedFileRecord->cdbSize) <= B_DVR_MEDIA_SEGMENT_SIZE)
        && navFileCount > segmentedFileRecord->segmentNum )
    {
        mediaFileSize = segmentedFileRecord->mediaWrite->pCachedFileNode->size;
        mediaAlignment -= (mediaFileSize % mediaAlignment);
        mediaFileSize += mediaAlignment;
        segmentedFileRecord->mediaWrite->segmentedSize = mediaFileSize;
        segmentedFileRecord->navWrite->mediaLinearOffset -= segmentedFileRecord->navWrite->segmentedSize;
        segmentedFileRecord->navWrite->segmentedSize = segmentedFileRecord->mediaWrite->segmentedSize;
        segmentedFileRecord->navWrite->mediaLinearOffset += segmentedFileRecord->navWrite->segmentedSize;
        segmentedFileRecord->segmentNum = mediaFileCount;
        BDBG_WRN(("modified media file size %u",segmentedFileRecord->mediaWrite->segmentedSize));
     }
    return;
}

static void dumpSegmentedFileRecord(B_DVR_SegmentedFileRecordHandle segmentedFileRecord)
{
    BKNI_AcquireMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
    BDBG_ERR(("media write lso:%lld lco:%lld",
             segmentedFileRecord->mediaWrite->linearStartOffset,
             segmentedFileRecord->mediaWrite->linearCurrentOffset));
    dumpFileNodeList(segmentedFileRecord->mediaWrite);
    BKNI_ReleaseMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
    BKNI_AcquireMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
    BDBG_ERR(("nav write lso:%lld lco:%lld",
             segmentedFileRecord->navWrite->linearStartOffset,
             segmentedFileRecord->navWrite->linearCurrentOffset));
    BDBG_ERR(("nav read lso:%lld lco:%lld leo:%lld",
             segmentedFileRecord->navRead->linearStartOffset,
             segmentedFileRecord->navRead->linearCurrentOffset,
             segmentedFileRecord->navRead->linearEndOffset));
    dumpFileNodeList(segmentedFileRecord->navWrite);
    BKNI_ReleaseMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
    return;
}
static B_DVR_ERROR populateFileList(B_DVR_SegmentedFileHandle segmentedFile)
{
    B_DVR_SegmentedFileNode *fileNode=NULL;
    ssize_t sizeRead=0;
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    off_t returnOffset, fileNodeOffset;
    unsigned index=0;
    BDBG_MSG(("%s: metaData file:%s",__FUNCTION__,segmentedFile->fileName));
    do
    {
    
        fileNodeOffset = index*sizeof(*fileNode);
        returnOffset =  segmentedFile->dvrFileMetaData.fileInterface.io.readFD.seek(
            &segmentedFile->dvrFileMetaData.fileInterface.io.readFD,
            fileNodeOffset,SEEK_SET);

        if(returnOffset != fileNodeOffset)
        {
            break;
        }

        fileNode = BKNI_Malloc(sizeof(*fileNode));
        if(!fileNode)
        {
            rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
            BDBG_ERR(("%s:alloc failed",__FUNCTION__));
            break;
        }
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*fileNode),
                                                true,__FUNCTION__,__LINE__);
        sizeRead = segmentedFile->dvrFileMetaData.fileInterface.io.readFD.read(
            &segmentedFile->dvrFileMetaData.fileInterface.io.readFD,fileNode,sizeof(*fileNode));
        if(sizeRead == sizeof(*fileNode))
        {
            BDBG_OBJECT_SET(fileNode,B_DVR_SegmentedFileNode);
            if(segmentedFile->service == eB_DVR_ServiceTSB)
            {
                BLST_Q_INSERT_TAIL(&segmentedFile->tsbFreeFileList,fileNode,nextFileNode);
                fileNode->refCount = 0;
                fileNode->linearStartOffset = 0;
                fileNode->segmentOffset = 0;
                fileNode->size = 0;
            }
            else
            {
               BLST_Q_INSERT_TAIL(&segmentedFile->fileList,fileNode,nextFileNode);
            }
            segmentedFile->fileCount++;
            BDBG_MSG(("%s:index:%u fileName:%s lso:%lld size:%u segOff:%lld refCount:%u rec:%d",
                      __FUNCTION__,
                      fileNode->index,
                      fileNode->fileName,
                      fileNode->linearStartOffset,
                      fileNode->size,
                      fileNode->segmentOffset,
                      fileNode->refCount,
                      fileNode->recordType));
        }
        else
        {
            if(!segmentedFile->fileCount)
            {
                rc = B_DVR_UNKNOWN;
            }
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*fileNode),
                                                    false,__FUNCTION__,__LINE__);
            BKNI_Free(fileNode);
            break;
        }
        index++;
    }while(sizeRead);
    return rc;
}

static B_DVR_ERROR updateFileList(B_DVR_SegmentedFileHandle segmentedFile)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    off_t returnOffset, fileNodeOffset;
    B_DVR_SegmentedFileNode updatedFileEntry;
    B_DVR_SegmentedFileNode *fileNode=NULL;
    ssize_t sizeRead=0;
    BDBG_ASSERT(segmentedFile);
    BDBG_MSG_TRACE(("%s:fileCount %u",__FUNCTION__,segmentedFile->fileCount));
    BKNI_Memset((void *)&updatedFileEntry,0,sizeof(updatedFileEntry));
    /* 
     * Read the previous last entry from the metaData file.
     * This entry would always be valid and hence no error
     * checking.
     */
    fileNodeOffset = (segmentedFile->fileCount-1)*sizeof(updatedFileEntry);
    segmentedFile->dvrFileMetaData.fileInterface.io.readFD.seek(&segmentedFile->dvrFileMetaData.fileInterface.io.readFD,
                                                                fileNodeOffset,
                                                                SEEK_SET);
    segmentedFile->dvrFileMetaData.fileInterface.io.readFD.read(&segmentedFile->dvrFileMetaData.fileInterface.io.readFD,
                                                                &updatedFileEntry,
                                                                sizeof(updatedFileEntry));
    fileNode = BLST_Q_LAST(&segmentedFile->fileList);
    BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);

    /* 
     * update the last play file entry if last entry from the metaData file changed 
     */
    if(updatedFileEntry.size > fileNode->size)
    {
        BDBG_MSG_TRACE(("%s:previous last fileNode entry size %u",__FUNCTION__,fileNode->size));
        BDBG_MSG_TRACE(("%s:current last fileNode entry size %u",__FUNCTION__,updatedFileEntry.size));
        fileNode->size = updatedFileEntry.size;
    }

    /*
     * Try reading the next entry if available.
     * 
     */

    fileNodeOffset = (segmentedFile->fileCount)*sizeof(updatedFileEntry);
    returnOffset = segmentedFile->dvrFileMetaData.fileInterface.io.readFD.seek(&segmentedFile->dvrFileMetaData.fileInterface.io.readFD,
                                                                               fileNodeOffset,
                                                                               SEEK_SET);
    if(returnOffset!=fileNodeOffset)
    {
        return rc;
    }
    BKNI_Memset((void *)&updatedFileEntry,0,sizeof(updatedFileEntry));
    sizeRead = segmentedFile->dvrFileMetaData.fileInterface.io.readFD.read(&segmentedFile->dvrFileMetaData.fileInterface.io.readFD,
                                                                           &updatedFileEntry,
                                                                           sizeof(updatedFileEntry));
    if(sizeRead!=sizeof(updatedFileEntry))
    {
        return rc;
    }
    else
    {
        fileNode = BKNI_Malloc(sizeof(*fileNode));
        if(!fileNode)
        {
            BDBG_ERR(("%s:alloc failed",__FUNCTION__));
            rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
            return rc;
        }
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*fileNode),
                                                true,__FUNCTION__,__LINE__);
        BKNI_Memcpy((void *)fileNode,(void *)&updatedFileEntry,sizeof(*fileNode));
        BDBG_OBJECT_SET(fileNode,B_DVR_SegmentedFileNode);
        BLST_Q_INSERT_TAIL(&segmentedFile->fileList,fileNode,nextFileNode);
        segmentedFile->fileCount++;
        BDBG_MSG(("%s:new file entry -> name %s start offset %lld size %u",
                 __FUNCTION__,fileNode->fileName,
                  fileNode->linearStartOffset,fileNode->size));
    }
    
    BDBG_MSG_TRACE(("%s: fileCount %u",__FUNCTION__,segmentedFile->fileCount));
    return rc;
}

static B_DVR_SegmentedFileNode * allocateFileSegment(B_DVR_MediaStorageHandle mediaStorage,
                                                     B_DVR_FileType fileType,
                                                     unsigned volumeIndex,
                                                     const char *subDir)
{
    B_DVR_SegmentedFileNode *newFileNode=NULL;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    newFileNode = BKNI_Malloc(sizeof(*newFileNode));
    if(!newFileNode)
    {
        BDBG_ERR(("%s:alloc failed",__FUNCTION__));
        goto error_alloc;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*newFileNode),
                                            true,__FUNCTION__,__LINE__);
    BKNI_Memset((void *)newFileNode,0,sizeof(*newFileNode));
    BDBG_OBJECT_SET(newFileNode,B_DVR_SegmentedFileNode);
    if(fileType == eB_DVR_FileTypeMedia)
    {
        rc = B_DVR_MediaStorage_AllocateMediaSegment(mediaStorage,
                                                     volumeIndex,
                                                     subDir,
                                                     newFileNode->fileName);
        if(rc!=B_DVR_SUCCESS)
        {
            BDBG_ERR(("%s:media segment allocate failed",__FUNCTION__));
            goto error_file;
        }
        else
        {
            BDBG_MSG_TRACE(("%s:media segment allocated %s",__FUNCTION__,newFileNode->fileName));

        }
    }
    else
    {
        rc = B_DVR_MediaStorage_AllocateNavSegment(mediaStorage,
                                                     volumeIndex,
                                                     newFileNode->fileName);
        if(rc!=B_DVR_SUCCESS)
        {
            BDBG_ERR(("%s:nav file allocate failed",__FUNCTION__));
            goto error_file;
        }
        else
        {
            BDBG_MSG_TRACE(("%s:nav file allocated %s",
                      __FUNCTION__,newFileNode->fileName));
        }
    }
    return newFileNode;
error_file:
    BDBG_OBJECT_DESTROY(newFileNode,B_DVR_SegmentedFileNode);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*newFileNode),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(newFileNode);
    newFileNode = NULL;
error_alloc:
    return newFileNode;
}

static void freeFileSegment(B_DVR_MediaStorageHandle mediaStorage,
                             B_DVR_FileType fileType,
                             unsigned volumeIndex,
                             const char *subDir,
                             B_DVR_SegmentedFileNode *fileNode)
{

    BDBG_MSG_TRACE(("%s: %s",__FUNCTION__,fileNode->fileName));
    if(fileType == eB_DVR_FileTypeMedia)
    {
        B_DVR_MediaStorage_FreeMediaSegment(mediaStorage,
                                            volumeIndex,
                                            subDir,
                                            fileNode->fileName);
    }
    else
    {
        B_DVR_MediaStorage_FreeNavSegment(mediaStorage,
                                          volumeIndex,
                                          fileNode->fileName);
    }
    BDBG_OBJECT_DESTROY(fileNode,B_DVR_SegmentedFileNode);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*fileNode),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(fileNode);
    return;
}

static B_DVR_ERROR dumpFileNodeList(B_DVR_SegmentedFileHandle segmentedFile)
{
    B_DVR_SegmentedFileNode *fileNode=NULL;
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_MSG(("%s: metaDataFile:%s",__FUNCTION__,segmentedFile->fileName));

    BDBG_MSG(("tsb pool"));
    if(segmentedFile->tsbRecordFile)
    {
        fileNode = BLST_Q_FIRST(&segmentedFile->tsbRecordFile->tsbFreeFileList);
    }
    else
    {
        fileNode = BLST_Q_FIRST(&segmentedFile->tsbFreeFileList);
    }
    for(;fileNode!=NULL;fileNode=BLST_Q_NEXT(fileNode,nextFileNode))
    {
        BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
        BDBG_MSG(("file node %lx %s %lld %u rec %d",
                  (unsigned long)fileNode,fileNode->fileName,
                  fileNode->linearStartOffset,
                  fileNode->size,
                  fileNode->recordType));
    }
    BDBG_MSG(("active pool"));
    if(segmentedFile->tsbRecordFile)
    {
        fileNode = BLST_Q_FIRST(&segmentedFile->tsbRecordFile->fileList);
    }
    else
    {
        fileNode = BLST_Q_FIRST(&segmentedFile->fileList);
    }
    for(;fileNode!=NULL;fileNode=BLST_Q_NEXT(fileNode,nextFileNode))
    {
        BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
        BDBG_MSG(("file node %lx %s %lld %u rec %d",
                  (unsigned long)fileNode,fileNode->fileName,
                  fileNode->linearStartOffset,
                  fileNode->size,
                  fileNode->recordType));
    }
    return rc;
}

static int checkMetaDataFile(const char *fileName, 
                             B_DVR_SegmentedFileSettings *pSettings,
                             B_DVR_FileType fileType)
{
    int segFd=0,metaDataFd=0;
    off_t metaDataOffset=0, returnOffset=0;
    ssize_t returnSize=0;
    B_DVR_SegmentedFileNode fileNode;
    int fileCount=0,index=0;
    struct stat sb;
    off_t linearOffset=0;
    char fullFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    char path[B_DVR_MAX_FILE_NAME_LENGTH];
    BDBG_MSG(("%s: metaDataFile:%s",__FUNCTION__,fileName));
    BKNI_Memset((void *)&fileNode,0,sizeof(fileNode));
    BKNI_Memset((void *)&sb,0,sizeof(sb));
    BDBG_MSG(("%s:metaData file[path]:%s",__FUNCTION__, fileName));
    metaDataFd = open(fileName, O_RDWR|O_CLOEXEC, 0666);
    if(metaDataFd < 0)
    {
        BDBG_ERR(("%s:file open error %s fd:%d",__FUNCTION__,fileName,metaDataFd));
        fileCount=-1;
        return fileCount;
    }
    if (fstat(metaDataFd, &sb)<0) 
    {
        BDBG_ERR(("%s:error in getting size of %s",__FUNCTION__,fileName));
        fileCount=-1;
        close(metaDataFd);
        return fileCount;
    }
    fileCount = (int)(sb.st_size/sizeof(fileNode));
    if(fileCount<=0) 
    {
        BDBG_ERR(("%s:fileCount is zero is %s",__FUNCTION__,fileName));
        fileCount=-1;
        close(metaDataFd);
        return fileCount;
    }
    BDBG_MSG(("%s: sizeof(metaDataFile:%s):%lld segFileCount:%d",
              __FUNCTION__,fileName,sb.st_size,fileCount));
    for(index=0;index < fileCount;index++)
    {
        metaDataOffset = sizeof(fileNode)*index;
        returnOffset = lseek(metaDataFd,metaDataOffset, SEEK_SET);
        if(returnOffset != metaDataOffset)
        {
            BDBG_ERR(("%s:metaData file:%s seek error off:%lld ret off:%lld",
                      __FUNCTION__,fileName,metaDataOffset,returnOffset));
            fileCount=index;
            break;
        }
        returnSize = read(metaDataFd,(void *)&fileNode,sizeof(fileNode));
        if(returnSize!=sizeof(fileNode))
        {
            BDBG_ERR(("%s:metaData file:%s read error off: %lld size:%u ret size:%u",
                      __FUNCTION__,fileName,metaDataOffset,sizeof(fileNode),returnSize));
            fileCount = index;
            break;
        }
        if(fileNode.fileName[0]=='\0') 
        {
            fileCount=index;
            break;
        }
        if(!index) 
        {
            linearOffset = fileNode.linearStartOffset;
        }
        if(index && linearOffset!=fileNode.linearStartOffset) 
        {
            BDBG_ERR(("%s:index %d Invalid linearStartOffset:%lld desired linearStartOffset:%lld segment name %s",
                     __FUNCTION__,index,fileNode.linearStartOffset,linearOffset,fileNode.fileName));
            fileCount=index;
            break;
        }
        linearOffset += fileNode.size;
        if(fileType == eB_DVR_FileTypeMedia) 
        {
            B_DVR_MediaStorage_GetMediaPath(pSettings->mediaStorage,pSettings->volumeIndex,path);
        }
        else
        { 
            B_DVR_MediaStorage_GetNavPath(pSettings->mediaStorage,pSettings->volumeIndex,path);
        }
        BKNI_Snprintf(fullFileName, sizeof(fullFileName), "%s/%s", path,fileNode.fileName);
        segFd = open(fullFileName, O_RDONLY|O_CLOEXEC, 0666);
        if(segFd < 0)
        {
            BDBG_ERR(("%s:seg file open error %s",__FUNCTION__,fullFileName));
            fileCount=index;
            break;
        }
        if (fstat(segFd, &sb)<0) 
        {
            BDBG_ERR(("%s:seg file error in getting size of %s",__FUNCTION__,fullFileName));
            close(segFd);
            fileCount=index;
            break;        
        }

        if((fileNode.size + fileNode.segmentOffset) > sb.st_size) 
        {
            BDBG_ERR(("%s:",__FUNCTION__));
            BDBG_ERR(("size of file %s doesn't match with the size in metadata",fullFileName));
            BDBG_ERR(("size of file %s is %llx",fullFileName,sb.st_size));
            BDBG_ERR(("size of file %s in fileNode is %u",fullFileName,(fileNode.size+fileNode.segmentOffset)));
            linearOffset-= ((fileNode.size + fileNode.segmentOffset) - sb.st_size);
            fileNode.size = sb.st_size - fileNode.segmentOffset;
        }
        close(segFd);
        BDBG_MSG(("%s:index:%u segname:%s lso:%lld segOff:%lld size:%u refCount:%u recType:%u",
                  __FUNCTION__,
                  fileNode.index,
                  fileNode.fileName,
                  fileNode.linearStartOffset,
                  fileNode.segmentOffset,
                  fileNode.size,
                  fileNode.refCount,
                  fileNode.recordType));
    }
    close(metaDataFd);
    return fileCount;
}

static B_DVR_ERROR writeMetaDataFile(B_DVR_FileHandle metaDataFile,
                                     B_DVR_SegmentedFileNode *updatedFileNode)
{
    off_t metaDataOffset, returnOffset;
    ssize_t sizeWritten;
    B_DVR_SegmentedFileNode *fileNode = updatedFileNode;
    B_DVR_ERROR rc =B_DVR_SUCCESS;
    metaDataOffset = sizeof(*fileNode)*fileNode->index;

    returnOffset = segmented_posix_seekForWrite(
        &metaDataFile->fileInterface.io.writeFD,
         metaDataOffset, SEEK_SET);
    if(returnOffset != metaDataOffset)
    {
        rc = B_DVR_OS_ERROR;
        BDBG_ERR(("%s: %s seek error off:%lld ret off:%lld",
                  __FUNCTION__,metaDataFile->fileName,
                  metaDataOffset,returnOffset));
        return rc;
    }

    sizeWritten =  
        metaDataFile->fileInterface.io.writeFD.write(&metaDataFile->fileInterface.io.writeFD,
                                                                      (void *)fileNode,
                                                                      sizeof(*fileNode));
    if(sizeWritten <= 0)
    {

        BDBG_ERR(("%s:%s %u %lld %u",
                  __FUNCTION__,
                  fileNode->fileName,
                  fileNode->index,
                  fileNode->linearStartOffset,
                  fileNode->size));
        rc = BERR_TRACE(B_DVR_OS_ERROR);
        BDBG_ERR(("%s:Out of storage space",__FUNCTION__));
    }
    return rc;
}

static off_t findTSBRecStartOffset(B_DVR_SegmentedFileHandle segmentedFile)
{
   B_DVR_SegmentedFileNode *startNode=NULL;
   off_t linearOffset;

   if(segmentedFile->fileCount <= segmentedFile->maxFileCount)
   { 
     linearOffset = segmentedFile->linearStartOffset;
   }
   else
   {
       startNode = BLST_Q_FIRST(&segmentedFile->fileList);
       startNode = BLST_Q_NEXT(startNode,nextFileNode);
       BDBG_OBJECT_ASSERT(startNode,B_DVR_SegmentedFileNode);
       linearOffset = startNode->linearStartOffset;
   }
   return linearOffset;
}
static B_DVR_SegmentedFileNode * findPlayFileNode(B_DVR_SegmentedFileHandle segmentedFile)
{
    B_DVR_SegmentedFileNode *fileNode = segmentedFile->pCachedFileNode;
    B_DVR_SegmentedFileNode *lastFileNode = NULL;
    B_DVR_SegmentedFileNode *firstFileNode = NULL;
    B_DVR_SegmentedFileNode *prevFileNode = NULL;
    B_DVR_SegmentedFileNode *nextFileNode1 = NULL;

    if(fileNode) 
    {
        BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
    }
    if(fileNode && 
       segmentedFile->linearCurrentOffset >= (fileNode->linearStartOffset-fileNode->segmentOffset)
       && (segmentedFile->linearCurrentOffset < fileNode->linearStartOffset +fileNode->size))
    {
        return fileNode;
    }

    if(segmentedFile->tsbRecordFile) 
    {
        lastFileNode = BLST_Q_LAST(&segmentedFile->tsbRecordFile->fileList);
        firstFileNode = BLST_Q_FIRST(&segmentedFile->tsbRecordFile->fileList);
    }
    else
    {
        lastFileNode = BLST_Q_LAST(&segmentedFile->fileList);
        firstFileNode = BLST_Q_FIRST(&segmentedFile->fileList);
    }
    if(lastFileNode) 
    {
        BDBG_OBJECT_ASSERT(lastFileNode,B_DVR_SegmentedFileNode); 
    }
    if(firstFileNode) 
    {
        BDBG_OBJECT_ASSERT(firstFileNode,B_DVR_SegmentedFileNode); 
    }
    if(firstFileNode && 
       segmentedFile->linearCurrentOffset >= (firstFileNode->linearStartOffset-firstFileNode->segmentOffset)
       && (segmentedFile->linearCurrentOffset < firstFileNode->linearStartOffset +firstFileNode->size))
    {
        return firstFileNode;
    }

    if(lastFileNode && 
       segmentedFile->linearCurrentOffset >= (lastFileNode->linearStartOffset-lastFileNode->segmentOffset)
       && (segmentedFile->linearCurrentOffset < lastFileNode->linearStartOffset +lastFileNode->size))
    {
        return lastFileNode;
    }

    if(fileNode) 
    {
        nextFileNode1 = BLST_Q_NEXT(fileNode,nextFileNode);
        prevFileNode = BLST_Q_PREV(fileNode,nextFileNode);
    }

    if(nextFileNode1) 
    {
        BDBG_OBJECT_ASSERT(nextFileNode1,B_DVR_SegmentedFileNode); 
    }
    if(prevFileNode) 
    {
        BDBG_OBJECT_ASSERT(prevFileNode,B_DVR_SegmentedFileNode); 
    }

    if(nextFileNode1 && 
       segmentedFile->linearCurrentOffset >= (nextFileNode1->linearStartOffset-nextFileNode1->segmentOffset)
       && (segmentedFile->linearCurrentOffset < nextFileNode1->linearStartOffset +nextFileNode1->size))
    {
        return nextFileNode1;
    }

    if(prevFileNode && 
       segmentedFile->linearCurrentOffset >= (prevFileNode->linearStartOffset-prevFileNode->segmentOffset)
       && (segmentedFile->linearCurrentOffset < prevFileNode->linearStartOffset +prevFileNode->size))
    {
        return prevFileNode;
    }

    fileNode= firstFileNode;

    for(;fileNode!=NULL;fileNode=BLST_Q_NEXT(fileNode,nextFileNode))
    {
        BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
        if(segmentedFile->linearCurrentOffset >= (fileNode->linearStartOffset-fileNode->segmentOffset)
          && (segmentedFile->linearCurrentOffset < fileNode->linearStartOffset + fileNode->size))
        {
            break;
        }
    }
    return fileNode;
}

static ssize_t segmented_posix_read(bfile_io_read_t fd, void *buf_, size_t length)
{
    B_DVR_SegmentedFileHandle segmentedFile = (B_DVR_SegmentedFileHandle) fd;
    ssize_t returnSize =0;
    void *buf = buf_;
    ssize_t sizeRead=0;
    off_t currentSegmentOffset=0; 
    B_DVR_ERROR rc=B_DVR_SUCCESS;

    BDBG_ASSERT(segmentedFile);
    BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
    for(returnSize=0;length>0;)
    {
        off_t returnOffset;
        ssize_t to_read = length;
        B_DVR_SegmentedFileNode *fileNode=NULL;
        if (segmentedFile->tsbRecordFile)
        {
            if (segmentedFile->linearCurrentOffset < segmentedFile->tsbRecordFile->linearStartOffset)
            {
                BDBG_MSG_TRACE(("%s: reached the beginning metaDataFile:%s",
                          __FUNCTION__,segmentedFile->fileName));
                BDBG_MSG_TRACE(("lco:%lld < lso:%lld",
                          segmentedFile->linearCurrentOffset,segmentedFile->tsbRecordFile->linearStartOffset));
                break;
            }
            if (segmentedFile->linearCurrentOffset >= segmentedFile->tsbRecordFile->linearCurrentOffset)
            {
                BDBG_MSG_TRACE(("%s:reached the end metaDataFile:%s",__FUNCTION__,segmentedFile->fileName));
                BDBG_MSG_TRACE(("lco:%lld < leo:%lld",segmentedFile->linearCurrentOffset,
                          segmentedFile->tsbRecordFile->linearCurrentOffset));
                break;
           }

        }
        else
        {

            if ((segmentedFile->linearCurrentOffset < segmentedFile->linearStartOffset) 
                || (segmentedFile->linearCurrentOffset >= segmentedFile->linearEndOffset))
            {
                BDBG_MSG_TRACE(("%s:boundard condition met metaDataFile:%s",
                          __FUNCTION__,segmentedFile->fileName));
                BDBG_MSG_TRACE(("lso:%lld lco:%lld leo:%lld",
                          segmentedFile->linearStartOffset,
                          segmentedFile->linearCurrentOffset,
                          segmentedFile->linearEndOffset));
                break;
            }
        }

        fileNode= findPlayFileNode(segmentedFile);

        if(!fileNode)
        {
           if(!returnSize)
           {
               if(segmentedFile->service == eB_DVR_ServiceTSB) 
               {
                    BDBG_MSG(("%s:segment file not found in %s",
                             __FUNCTION__,segmentedFile->fileName));
                    BDBG_MSG(("lso:%lld lco:%lld leo:%lld len:%lu",
                             segmentedFile->tsbRecordFile->linearStartOffset,
                             segmentedFile->linearCurrentOffset,
                             segmentedFile->tsbRecordFile->linearCurrentOffset,
                             (unsigned long)length));
               }
               else
               {
                   BDBG_ERR(("%s:segment file not found in %s",
                             __FUNCTION__,segmentedFile->fileName));
                   BDBG_ERR(("lso:%lld lco:%lld leo:%lld len:%lu",
                             segmentedFile->linearStartOffset,
                             segmentedFile->linearCurrentOffset,
                             segmentedFile->linearEndOffset,
                             (unsigned long)length));
                   dumpFileNodeList(segmentedFile);
               }
            }
            break;
        }

        if(segmentedFile->pCachedFileNode && fileNode!=segmentedFile->pCachedFileNode)
        {
            B_DVR_File_Close(&segmentedFile->dvrFile);
        }

        if(fileNode!=segmentedFile->pCachedFileNode)
        {
            B_DVR_FileSettings settings;
            char fullFileName[B_DVR_MAX_FILE_NAME_LENGTH];
            char path[B_DVR_MAX_FILE_NAME_LENGTH];
            settings.fileIO = eB_DVR_FileIORead;
            settings.create = false;
            BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
            if(segmentedFile->fileType == eB_DVR_FileTypeNavigation)
            { 
                 B_DVR_MediaStorage_GetNavPath(segmentedFile->mediaStorage,segmentedFile->volumeIndex,path);
                 BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s", path,fileNode->fileName);
                 settings.directIO = false;
                 rc = B_DVR_File_Open(&segmentedFile->dvrFile,fullFileName,&settings);
                 
            }
            else
            {
               B_DVR_MediaStorage_GetMediaPath(segmentedFile->mediaStorage,segmentedFile->volumeIndex,path);
               BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s", path,fileNode->fileName);
               settings.directIO = true;
               rc = B_DVR_File_Open(&segmentedFile->dvrFile,fullFileName,&settings);
            }
            segmentedFile->pCachedFileNode  = fileNode;

        }
        
        if(rc!=B_DVR_SUCCESS)
        {
            if(!returnSize)
            {
                if(segmentedFile->registeredCallback)
                {
                    BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
                    segmentedFile->registeredCallback((void*)segmentedFile,B_DVR_SegmentedFileEventNotFoundError);
                    BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
                }
                BDBG_ERR(("%s:unable to open segmented file %s in metaData file %s",
                         __FUNCTION__,fileNode->fileName,segmentedFile->fileName));
                dumpFileNodeList(segmentedFile);
            }
            break;
        }

        fileNode = segmentedFile->pCachedFileNode;   
        currentSegmentOffset = segmentedFile->linearCurrentOffset - fileNode->linearStartOffset;
        if (to_read > fileNode->size - currentSegmentOffset) 
        {
            to_read = fileNode->size - currentSegmentOffset;
        }
        returnOffset = segmentedFile->dvrFile.fileInterface.io.readFD.seek(&segmentedFile->dvrFile.fileInterface.io.readFD,
                                                                           currentSegmentOffset+fileNode->segmentOffset, SEEK_SET);
        if(returnOffset != currentSegmentOffset+fileNode->segmentOffset)
        {
            if(!returnSize)
            {
                if (returnOffset < 0)
                {
                    returnSize = (ssize_t)returnOffset; /* propagate error */
                }
                else
                {
                    returnSize = -1;
                }
            }
            BDBG_ERR(("%s:seek error in segmented file %s metaDataFile %s",
                          __FUNCTION__,fileNode->fileName,segmentedFile->fileName));
            BDBG_ERR(("lso:%lld lco:%lld leo:%lld off:%lld ret off:%lld",
                     segmentedFile->linearStartOffset,
                     segmentedFile->linearCurrentOffset,
                     segmentedFile->linearEndOffset,
                     (currentSegmentOffset+fileNode->segmentOffset),
                     returnOffset));
            dumpFileNodeList(segmentedFile);
            if(segmentedFile->registeredCallback)
            {
                BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
                segmentedFile->registeredCallback((void *)segmentedFile,B_DVR_SegmentedFileEventSeekError);
                BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
            }
            break;
        }
        sizeRead = segmentedFile->dvrFile.fileInterface.io.readFD.read(&segmentedFile->dvrFile.fileInterface.io.readFD,buf,to_read);

        if(sizeRead <=0)
        {
            BDBG_ERR(("%s:Unable to read len %s off: %lld size: %u ret size: %u ",
                      __FUNCTION__,
                      fileNode->fileName,
                      returnOffset,
                      to_read, 
                      sizeRead));
            BDBG_ERR(("%s:lso %lld lco %lld leo %lld",
                      __FUNCTION__,
                      segmentedFile->linearStartOffset,
                      segmentedFile->linearCurrentOffset,
                      segmentedFile->linearEndOffset));
            if(segmentedFile->registeredCallback)
            {
                BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
                segmentedFile->registeredCallback((void *)segmentedFile,B_DVR_SegmentedFileEventReadError);
                BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
            }
            dumpFileNodeList(segmentedFile);
            break;
        }
        returnSize += sizeRead;
        length -= (size_t) sizeRead;
        segmentedFile->linearCurrentOffset += sizeRead;
        buf = (uint8_t *)buf + sizeRead;
    }
    BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
    return returnSize;
}


static void segmented_posix_boundsUpdate(B_DVR_SegmentedFileHandle segmentedFile, off_t *first, off_t *last)
{
    BDBG_ASSERT(segmentedFile);
    if (segmentedFile->tsbRecordFile)
    {
        *first = segmentedFile->linearStartOffset = segmentedFile->tsbRecordFile->linearStartOffset;
        *last = segmentedFile->linearEndOffset= segmentedFile->tsbRecordFile->linearCurrentOffset;
    }
    else
    {
        *last = segmentedFile->linearEndOffset;
        *first = segmentedFile->linearStartOffset;
    }
}


static int segmented_posix_bounds(bfile_io_read_t fd, off_t *first, off_t *last )
{
    B_DVR_SegmentedFileHandle segmentedFile = (B_DVR_SegmentedFileHandle) fd;
    BDBG_ASSERT(segmentedFile);
    BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
    segmented_posix_boundsUpdate(segmentedFile, first, last);
    BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
    return 0;
}


static off_t segmented_posix_seek(bfile_io_read_t fd, off_t offset, int whence)
{
    B_DVR_SegmentedFileHandle segmentedFile = (B_DVR_SegmentedFileHandle) fd;
    off_t end,begin;
    BDBG_ASSERT(segmentedFile);
    BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
    if((segmentedFile->service != eB_DVR_ServiceRecord) &&
       segmentedFile->pCachedFileNode && segmentedFile->tsbRecordFile
       && segmentedFile->linearCurrentOffset < findTSBRecStartOffset(segmentedFile->tsbRecordFile))
    {
        BDBG_MSG(("%s:invalidate the cached file node",__FUNCTION__));
        BDBG_MSG(("%s:rd curOff:%lld seek Off:%lld wr startOff:%lld",
                  segmentedFile->fileName,
                  segmentedFile->linearCurrentOffset,
                  offset,
                  segmentedFile->tsbRecordFile->linearStartOffset));
        BDBG_MSG(("closing %s",segmentedFile->dvrFile.fileName));
        B_DVR_File_Close(&segmentedFile->dvrFile);
        segmentedFile->pCachedFileNode = NULL;
    }
    if (whence==SEEK_CUR && offset==0)
    {
        /* special case, which is used to  rerrieve current position, without bound checking  */
        /*BDBG_MSG(("SEEK_CUR w off=0"));*/
        offset = segmentedFile->linearCurrentOffset;
        goto done;
    }
    segmented_posix_boundsUpdate(segmentedFile, &begin, &end);
    /* locate where file begins */
    switch(whence) {
    case SEEK_CUR:
        offset = segmentedFile->linearCurrentOffset + offset;
        break;
    case SEEK_END:
        offset = end;
        break;
    case SEEK_SET:
         break;
    default:
        BDBG_ERR(("%s:unknown seek whence %d",__FUNCTION__,whence));
        goto error;
    }
     if(offset && offset == segmentedFile->linearCurrentOffset)
     goto done;

    if (offset < 0 || offset >= end)
    {
        BDBG_ERR(("%s:out out bounds 0..%lld..%lld",__FUNCTION__,offset, end));
        offset = -1;
        goto error;
    }
    if (offset < begin)
    {
        /*BDBG_MSG(("%s off %lld < begin %lld  end %lld cur %d", segmentedFile->fileName,
            offset, begin, end, segmentedFile->linearCurrentOffset));*/
        offset = begin;
    }
    /* just assign new offset */
done:
    segmentedFile->linearCurrentOffset = offset;
error:
    BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
    return offset;
}

static off_t segmented_posix_seekForWrite(bfile_io_write_t fd, off_t offset, int whence)
{
    off_t off;
    B_DVR_File *dvrFile = (B_DVR_File*) fd;
    BDBG_ASSERT(dvrFile);
    off = lseek(dvrFile->fd, offset, whence);
    return off;
}

static ssize_t segmented_posix_navWrite(B_DVR_SegmentedFileHandle segmentedFile, const void *buf_, size_t length)
{
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)(segmentedFile->context);
    ssize_t returnSize = 0;
    const void *buf = buf_;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    off_t navLinearOffset = 0;
    BNAV_Entry *mpeg2NavEntry;
    BNAV_AVC_Entry *avcNavEntry;
    BDBG_ASSERT(segmentedFile);
    for(returnSize=0;length>0;) 
    {
        size_t to_write=0;
        ssize_t sizeWritten;
        off_t segmentedFileOffset;
        B_DVR_SegmentedFileNode *fileNode = segmentedFile->pCachedFileNode;
        unsigned long currentTimeStamp=0;

        unsigned numEntries = length/segmentedFileRecord->navEntrySize;
        if(numEntries>1)
        {
            BDBG_ERR(("%s:add logic for multiple nav entries in a single nav write",__FUNCTION__));
            BDBG_ASSERT(numEntries==1);
        }
        if(segmentedFileRecord->navEntrySize == sizeof(BNAV_Entry))
        {
            mpeg2NavEntry = (BNAV_Entry *)((const uint8_t *)buf_);
            navLinearOffset = (BNAV_get_frameOffsetLo(mpeg2NavEntry)|((off_t)(BNAV_get_frameOffsetHi( mpeg2NavEntry ))<<32));
            currentTimeStamp = BNAV_get_timestamp(mpeg2NavEntry);
        }
        else
        {
            avcNavEntry = (BNAV_AVC_Entry *)((const uint8_t *)buf_);
            navLinearOffset = (BNAV_get_frameOffsetLo(avcNavEntry)|((off_t)(BNAV_get_frameOffsetHi( avcNavEntry ))<<32));
            currentTimeStamp = BNAV_get_timestamp(avcNavEntry);
        }
        if(navLinearOffset >= segmentedFile->mediaLinearOffset)
        { 
            B_DVR_FileSettings settings;
            char fullFileName[B_DVR_MAX_FILE_NAME_LENGTH];
            char path[B_DVR_MAX_FILE_NAME_LENGTH];

            BDBG_MSG_TRACE(("%s:nav fileName:%s fileCount %u",__FUNCTION__,
                      segmentedFile->fileName,segmentedFile->fileCount));
            BDBG_MSG_TRACE(("%s: mediaLinear %lld navLinear %lld",__FUNCTION__,segmentedFile->mediaLinearOffset,navLinearOffset));
            segmentedFile->mediaLinearOffset += segmentedFile->segmentedSize;
            if(segmentedFile->pCachedFileNode)
            { 
               BDBG_MSG(("%s:%s:close %s",__FUNCTION__,segmentedFile->fileName,
                         segmentedFile->pCachedFileNode->fileName)); 
               B_DVR_File_Close(&segmentedFile->dvrFile);
            }
            if((segmentedFile->service == eB_DVR_ServiceRecord) ||
               (segmentedFile->service == eB_DVR_ServiceMedia) ||
               (segmentedFile->service == eB_DVR_ServiceTSB && segmentedFile->tsbConversion))
            {
                B_DVR_SegmentedFileNode *newFileNode;
                segmentedFile->fileIndex = segmentedFile->fileCount;
                BDBG_ASSERT((segmentedFile->event));
                newFileNode = allocateFileSegment(segmentedFile->mediaStorage,
                                                  segmentedFile->fileType,
                                                  segmentedFile->volumeIndex,
                                                  segmentedFile->tsbConversion?
                                                  segmentedFile->tsbConvMetaDataSubDir:
                                                  segmentedFile->metaDataSubDir);
                if(!newFileNode)
                {
                    if(segmentedFile->registeredCallback)
                    {
                        BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
                        segmentedFile->registeredCallback((void *)segmentedFile,B_DVR_SegmentedFileEventNoFileSpaceError);
                        BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
                    }
                    BDBG_ERR(("%s: allocateFileSegment failed",__FUNCTION__));
                    break;
                }
                newFileNode->recordType = eB_DVR_RecordingPermanent;
                if(segmentedFile->service == eB_DVR_ServiceTSB) 
                {
                    /* During TSB conversion, the indices will be
                     * updated during copy metaData
                     */
                    newFileNode->index=0;
                }
                else
                {
                    newFileNode->index = segmentedFile->fileIndex;
                }
                newFileNode->linearStartOffset = segmentedFile->linearCurrentOffset;
                newFileNode->recordType = eB_DVR_RecordingPermanent;
                newFileNode->segmentOffset=0;
                newFileNode->size =0;
                newFileNode->refCount=1;
                BLST_Q_INSERT_TAIL(&segmentedFile->fileList,newFileNode,nextFileNode);
                fileNode = segmentedFile->pCachedFileNode=newFileNode;
                BDBG_MSG_TRACE(("%s:tsbConversion: index %d startOff:%lld size:%u segOff:%lld permRec:%d refCnt:%u name:%s",
                          __FUNCTION__,
                          segmentedFile->fileIndex,
                          fileNode->linearStartOffset,
                          fileNode->size,
                          fileNode->segmentOffset,
                          fileNode->recordType,
                          fileNode->refCount,
                          fileNode->fileName));
                if(segmentedFile->service == eB_DVR_ServiceTSB) 
                {
                    B_DVR_MediaStorage_IncreaseNavSegmentRefCount(segmentedFile->mediaStorage,
                                                                  segmentedFile->volumeIndex,
                                                                  fileNode->fileName);
                    fileNode->refCount++;
                     BDBG_MSG_TRACE(("%s: refCnt for %s increased",__FUNCTION__,fileNode->fileName));
                }
            }
            else
            {
                segmentedFile->fileIndex = segmentedFile->fileCount%segmentedFile->maxFileCount;
                if((BLST_Q_EMPTY(&segmentedFile->tsbFreeFileList)))
                {
                    fileNode = BLST_Q_FIRST(&segmentedFile->fileList);
                    BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
                    while(fileNode->recordType == eB_DVR_RecordingPermanent)
                    {
                        B_DVR_SegmentedFileNode *removedFileNode=NULL;
                        removedFileNode = fileNode;
                        BLST_Q_REMOVE_HEAD(&segmentedFile->fileList,nextFileNode);
                        fileNode = BLST_Q_FIRST(&segmentedFile->fileList);
                        BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
                        BDBG_MSG_TRACE(("%s:removing perm file from tsb list %s",
                                  __FUNCTION__,removedFileNode->fileName));
                        BDBG_MSG_TRACE(("%s:next file from tsb list %s",
                                  __FUNCTION__,fileNode->fileName));
                        freeFileSegment(segmentedFile->mediaStorage,
                                        segmentedFile->fileType,
                                        segmentedFile->volumeIndex,
                                        segmentedFile->metaDataSubDir,
                                        removedFileNode);
                    }
                    BLST_Q_REMOVE_HEAD(&segmentedFile->fileList,nextFileNode);
                    BLST_Q_INSERT_TAIL(&segmentedFile->fileList,fileNode,nextFileNode);
                    fileNode->size=0;
                    fileNode->segmentOffset=0;
                    fileNode->linearStartOffset = segmentedFile->linearCurrentOffset;
                    segmentedFile->pCachedFileNode = fileNode;
                    BDBG_MSG_TRACE(("%s:TSB Recycling index:%u startOff:%lld size:%u segOff:%lld permRec:%d refCnt:%u name:%s",
                              __FUNCTION__,
                              segmentedFile->fileIndex,
                              fileNode->linearStartOffset,
                              fileNode->size,
                              fileNode->segmentOffset,
                              fileNode->recordType,
                              fileNode->refCount,
                              fileNode->fileName));
                }
                else
                {
                    if(segmentedFile->fileCount > segmentedFile->maxFileCount)
                    {
                        fileNode = BLST_Q_FIRST(&segmentedFile->fileList);
                        BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
                        if(fileNode && fileNode->recordType == eB_DVR_RecordingPermanent) 
                        {
                            BDBG_MSG_TRACE(("%s:removing perm file from tsb list %s",
                                     __FUNCTION__,fileNode->fileName));
                            BLST_Q_REMOVE_HEAD(&segmentedFile->fileList,nextFileNode);
                            BDBG_MSG_TRACE(("%s:decreasing the reference count for %s",
                                    __FUNCTION__,fileNode->fileName));
                            freeFileSegment(segmentedFile->mediaStorage,
                                           segmentedFile->fileType,
                                           segmentedFile->volumeIndex,
                                           segmentedFile->metaDataSubDir,
                                           fileNode);
                        }
                    }
                    fileNode = BLST_Q_FIRST(&segmentedFile->tsbFreeFileList);
                    BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
                    BLST_Q_REMOVE_HEAD(&segmentedFile->tsbFreeFileList,nextFileNode);
                    BLST_Q_INSERT_TAIL(&segmentedFile->fileList,fileNode,nextFileNode);
                    segmentedFile->pCachedFileNode = fileNode;
                    fileNode->size=0;
                    fileNode->segmentOffset=0;
                    fileNode->refCount =1;
                    fileNode->linearStartOffset = segmentedFile->linearCurrentOffset;
                    BDBG_MSG_TRACE(("%s:TSB Queue index:%u startOff:%lld size:%u segOff:%lld permRec:%d refCnt:%u name:%s",
                              __FUNCTION__,
                              segmentedFile->fileIndex,
                              fileNode->linearStartOffset,
                              fileNode->size,
                              fileNode->segmentOffset,
                              fileNode->recordType,
                              fileNode->refCount,
                              fileNode->fileName));
                }
            }

            if((segmentedFile->service == eB_DVR_ServiceTSB) &&
              (segmentedFile->fileCount > segmentedFile->maxFileCount))
            { 
                if(segmentedFile->tsbConversion) 
                {
                    B_DVR_SegmentedFileNode *removedFileNode=NULL;
                    removedFileNode = BLST_Q_FIRST(&segmentedFile->fileList);
                    BDBG_OBJECT_ASSERT(removedFileNode,B_DVR_SegmentedFileNode);
                    BLST_Q_REMOVE_HEAD(&segmentedFile->fileList,nextFileNode);
                    fileNode = BLST_Q_FIRST(&segmentedFile->fileList);
                    BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
                    BDBG_MSG_TRACE(("%s:removing perm file from tsb list %s",
                              __FUNCTION__,removedFileNode->fileName));
                    BDBG_MSG_TRACE(("%s:next file from tsb list %s",
                              __FUNCTION__,fileNode->fileName));
                    if(removedFileNode->recordType == eB_DVR_RecordingPermanent) 
                    {
                        BDBG_MSG_TRACE(("%s:decreasing the reference count for %s",
                                  __FUNCTION__,removedFileNode->fileName));
                        freeFileSegment(segmentedFile->mediaStorage,
                                        segmentedFile->fileType,
                                        segmentedFile->volumeIndex,
                                        segmentedFile->metaDataSubDir,
                                        removedFileNode);
                    }
                    else
                    {
                        BLST_Q_INSERT_TAIL(&segmentedFile->tsbFreeFileList, removedFileNode, nextFileNode);
                        BDBG_MSG_TRACE(("%s: adding back %s to free list",
                                  __FUNCTION__,removedFileNode->fileName));
                    }
                }
                else
                {
                    fileNode = BLST_Q_FIRST(&segmentedFile->fileList);
                    BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
                }

                if(segmentedFile->linearStartOffset < fileNode->linearStartOffset) 
                {
                    BDBG_MSG(("%s:tsb prev bounds %lld %lld",
                             __FUNCTION__,segmentedFile->linearStartOffset, segmentedFile->linearCurrentOffset));
                    segmentedFile->linearStartOffset = fileNode->linearStartOffset;
                    BDBG_MSG(("%s:tsb curr bounds %lld %lld",
                             __FUNCTION__,segmentedFile->linearStartOffset, segmentedFile->linearCurrentOffset));
                }
                
            }

            fileNode = segmentedFile->pCachedFileNode;
            settings.fileIO = eB_DVR_FileIOWrite;
            settings.directIO = false;
            settings.create = false;
            B_DVR_MediaStorage_GetNavPath(segmentedFile->mediaStorage,segmentedFile->volumeIndex,path);
            BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s", path,fileNode->fileName);
            rc = B_DVR_File_Open(&segmentedFile->dvrFile,fullFileName,&settings);
            if(rc!=B_DVR_SUCCESS)
            {
                BDBG_ERR(("%s:nw fopen error %s %lld %u",
                          __FUNCTION__,fullFileName,segmentedFile->linearCurrentOffset,segmentedFile->fileCount));
                if(segmentedFile->registeredCallback)
                {
                    BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
                    segmentedFile->registeredCallback((void *)segmentedFile,B_DVR_SegmentedFileEventOpenError);
                    BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
                }
                break;
            }
            else
            {
                BDBG_MSG(("%s: %s: fopen %s lso:%lld lco:%lld",
                          __FUNCTION__,segmentedFile->fileName,
                          fullFileName,segmentedFile->linearStartOffset,
                          segmentedFile->linearCurrentOffset));
            }
            segmentedFile->fileCount++;
        }
        to_write = length;
        segmentedFileOffset = segmented_posix_seekForWrite(&segmentedFile->dvrFile.fileInterface.io.writeFD,fileNode->size, SEEK_SET);
        if (segmentedFileOffset != fileNode->size)
        {
            if (returnSize==0)
            {
                if (segmentedFileOffset < 0)
                { 
                    returnSize = (ssize_t)segmentedFileOffset; 
                }
                else
                {
                   returnSize = -1;
                }
                if(segmentedFile->registeredCallback)
                {
                    BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
                    segmentedFile->registeredCallback((void *)segmentedFile,B_DVR_SegmentedFileEventSeekError);
                    BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
                }
                BDBG_ERR(("%s:nav file seek error %s off:%lld ret off:%lld",
                          __FUNCTION__,fileNode->fileName,
                          fileNode->size,segmentedFileOffset));
            }
            break;
        }
        sizeWritten = segmentedFile->dvrFile.fileInterface.io.writeFD.write(&segmentedFile->dvrFile.fileInterface.io.writeFD, buf, to_write);
        if (sizeWritten<=0)
        {
            if(!returnSize)
            {
                returnSize = sizeWritten;
                if(segmentedFile->registeredCallback)
                {
                    BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
                    segmentedFile->registeredCallback((void *)segmentedFile,B_DVR_SegmentedFileEventWriteError);
                    BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
                }
            }
            BDBG_ERR(("%s:nav file write error %s %u %u %u",
                      __FUNCTION__,fileNode->fileName,
                      (unsigned)sizeWritten,
                      (unsigned)fileNode->size,
                      (unsigned)to_write));
            break;
        }
        BDBG_ASSERT((size_t)sizeWritten <= to_write);
        returnSize += sizeWritten;
        length -= (size_t) sizeWritten;
        buf = (const uint8_t *)buf + sizeWritten;
        fileNode->size += sizeWritten;
        segmentedFile->linearCurrentOffset += sizeWritten;
        if(returnSize>0)
        { 
           if((segmentedFile->service == eB_DVR_ServiceRecord) ||
             (segmentedFile->service == eB_DVR_ServiceMedia))
           {
               writeMetaDataFile(&segmentedFile->dvrFileMetaData,segmentedFile->pCachedFileNode);
           }
           if((segmentedFile->linearCurrentOffset < segmentedFile->itbThreshhold) ||
              (currentTimeStamp - segmentedFile->timeStamp) >= B_DVR_MEDIANODE_UPDATE_INTERVAL)
           { 
               segmentedFile->timeStamp = currentTimeStamp;
               B_Event_Set(segmentedFile->event);
           }
        }
    }
    return returnSize; 
}

static ssize_t segmented_posix_mediaWrite(B_DVR_SegmentedFileHandle segmentedFile, const void *buf_, size_t length)
{
    ssize_t returnSize = 0;
    const void *buf = buf_;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    
    BDBG_ASSERT(segmentedFile);
   
    for(returnSize=0;length>0;) 
    {
        size_t to_write=0;
        ssize_t sizeWritten=0;
        off_t segmentedFileOffset=0;
        B_DVR_SegmentedFileNode *fileNode = segmentedFile->pCachedFileNode;
        if(fileNode) 
        {
            BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
        }
        if((fileNode && fileNode->size >= segmentedFile->segmentedSize) || !fileNode)
        {
            B_DVR_FileSettings settings;
            char fullFileName[B_DVR_MAX_FILE_NAME_LENGTH];
            char path[B_DVR_MAX_FILE_NAME_LENGTH];

            BDBG_MSG_TRACE(("%s:media fileName:%s fileCount %u",
                      __FUNCTION__,segmentedFile->fileName,segmentedFile->fileCount));

            if(segmentedFile->pCachedFileNode)
            { 
                B_DVR_File_Close(&segmentedFile->dvrFile);
                BDBG_MSG(("%s:%s:close %s fi %u",
                          __FUNCTION__,
                          segmentedFile->fileName,
                          fileNode->fileName,
                          segmentedFile->fileIndex));
            }
           
            if((segmentedFile->service == eB_DVR_ServiceRecord) || 
               (segmentedFile->service == eB_DVR_ServiceMedia) ||
               (segmentedFile->tsbConversion && segmentedFile->service == eB_DVR_ServiceTSB))
            {
                B_DVR_SegmentedFileNode *newFileNode;
                segmentedFile->fileIndex = segmentedFile->fileCount;
                newFileNode = allocateFileSegment(segmentedFile->mediaStorage,
                                                 segmentedFile->fileType,
                                                 segmentedFile->volumeIndex,
                                                 segmentedFile->tsbConversion?
                                                 segmentedFile->tsbConvMetaDataSubDir:
                                                 segmentedFile->metaDataSubDir);
                if(!newFileNode)
                {
                    if(segmentedFile->registeredCallback)
                    {
                        BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
                        segmentedFile->registeredCallback((void *)segmentedFile,B_DVR_SegmentedFileEventNoFileSpaceError);
                        BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
                    }
                    BDBG_ERR(("%s: allocateFileSegment failed",__FUNCTION__));
                    break;
                }
                newFileNode->recordType = eB_DVR_RecordingPermanent;
                if(segmentedFile->service == eB_DVR_ServiceTSB) 
                {
                    /*
                     * The index for new fileNodes during 
                     * tsb conversion shall be updated 
                     * during copy metadata. 
                     */
                    newFileNode->index = 0;
                }
                else
                {
                    newFileNode->index = segmentedFile->fileIndex;
                }
                newFileNode->linearStartOffset = segmentedFile->linearCurrentOffset;
                newFileNode->segmentOffset=0;
                newFileNode->size =0;
                newFileNode->refCount=1;
                BLST_Q_INSERT_TAIL(&segmentedFile->fileList,newFileNode,nextFileNode);
                fileNode = segmentedFile->pCachedFileNode=newFileNode;
                BDBG_MSG_TRACE(("%s: tsbConversion: index:%u startOff:%lld size:%u segOff:%lld permRec:%d refCnt:%u name:%s",
                          __FUNCTION__,
                          segmentedFile->fileIndex,
                          fileNode->linearStartOffset,
                          fileNode->size,
                          fileNode->segmentOffset,
                          fileNode->recordType,
                          fileNode->refCount,
                          fileNode->fileName));
                if(segmentedFile->service == eB_DVR_ServiceTSB) 
                {

                     B_DVR_MediaStorage_IncreaseMediaSegmentRefCount(segmentedFile->mediaStorage,
                                                                     segmentedFile->volumeIndex,
                                                                     segmentedFile->tsbConvMetaDataSubDir,
                                                                     fileNode->fileName);
                     fileNode->refCount++;
                     BDBG_MSG_TRACE(("%s:refCnt for %s increased",__FUNCTION__,fileNode->fileName));
               }
           }
           else
           {
               segmentedFile->fileIndex = (segmentedFile->fileCount%segmentedFile->maxFileCount);
               if((BLST_Q_EMPTY(&segmentedFile->tsbFreeFileList)))
               {
                   fileNode = BLST_Q_FIRST(&segmentedFile->fileList);
                   BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
                   while(fileNode->recordType == eB_DVR_RecordingPermanent)
                   {
                       B_DVR_SegmentedFileNode *removedFileNode=NULL;
                       removedFileNode = fileNode;
                       BLST_Q_REMOVE_HEAD(&segmentedFile->fileList,nextFileNode);
                       fileNode = BLST_Q_FIRST(&segmentedFile->fileList);
                       BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
                       BDBG_MSG_TRACE(("%s:removing perm file from tsb list %s",__FUNCTION__,removedFileNode->fileName));
                       BDBG_MSG_TRACE(("%s: next file from tsb list %s",__FUNCTION__,fileNode->fileName));
                       freeFileSegment(segmentedFile->mediaStorage,
                                       segmentedFile->fileType,
                                       segmentedFile->volumeIndex,
                                       segmentedFile->metaDataSubDir,
                                       removedFileNode);
                   }
                   BLST_Q_REMOVE_HEAD(&segmentedFile->fileList,nextFileNode);
                   BLST_Q_INSERT_TAIL(&segmentedFile->fileList,fileNode,nextFileNode);
                   fileNode->size=0;
                   fileNode->segmentOffset=0;
                   fileNode->linearStartOffset = segmentedFile->linearCurrentOffset;
                   segmentedFile->pCachedFileNode = fileNode;
                   BDBG_MSG_TRACE(("%s:TSB Recycling: index:%u startOff:%lld size:%u segOff:%lld permRec:%d refCnt:%u name:%s",
                             __FUNCTION__,
                             segmentedFile->fileIndex,
                             fileNode->linearStartOffset,
                             fileNode->size,
                             fileNode->segmentOffset,
                             fileNode->recordType,
                             fileNode->refCount,
                             fileNode->fileName));
               }
               else
               {
                   if(segmentedFile->fileCount > segmentedFile->maxFileCount)
                   {
                       fileNode = BLST_Q_FIRST(&segmentedFile->fileList);
                       BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
                       if(fileNode && fileNode->recordType == eB_DVR_RecordingPermanent) 
                       {
                           BDBG_MSG_TRACE(("%s:removing perm file from tsb list %s",__FUNCTION__,fileNode->fileName));
                           BLST_Q_REMOVE_HEAD(&segmentedFile->fileList,nextFileNode);
                           BDBG_MSG_TRACE(("%s:decreasing the reference count for %s",__FUNCTION__,fileNode->fileName));
                           freeFileSegment(segmentedFile->mediaStorage,
                                           segmentedFile->fileType,
                                           segmentedFile->volumeIndex,
                                           segmentedFile->metaDataSubDir,
                                           fileNode);
                       }
                   }
                   fileNode = segmentedFile->pCachedFileNode = BLST_Q_FIRST(&segmentedFile->tsbFreeFileList);
                   BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
                   fileNode->size = 0;
                   fileNode->segmentOffset =0;
                   fileNode->refCount=1;
                   fileNode->linearStartOffset = segmentedFile->linearCurrentOffset;
                   BLST_Q_REMOVE_HEAD(&segmentedFile->tsbFreeFileList,nextFileNode);
                   BLST_Q_INSERT_TAIL(&segmentedFile->fileList,fileNode,nextFileNode);
                   BDBG_MSG_TRACE(("%s:TSB Queue: index:%u startOff:%lld size:%u segOff:%lld permRec:%d refCnt:%u name:%s",
                             __FUNCTION__,
                             segmentedFile->fileIndex,
                             fileNode->linearStartOffset,
                             fileNode->size,
                             fileNode->segmentOffset,
                             fileNode->recordType,
                             fileNode->refCount,
                             fileNode->fileName));
              
               }

           }

           if((segmentedFile->service == eB_DVR_ServiceTSB) &&
              (segmentedFile->fileCount > segmentedFile->maxFileCount))
           { 
               if(segmentedFile->tsbConversion) 
               {
                   B_DVR_SegmentedFileNode *removedFileNode=NULL;
                   removedFileNode = BLST_Q_FIRST(&segmentedFile->fileList);
                   BDBG_OBJECT_ASSERT(removedFileNode,B_DVR_SegmentedFileNode);
                   BLST_Q_REMOVE_HEAD(&segmentedFile->fileList,nextFileNode);
                   fileNode = BLST_Q_FIRST(&segmentedFile->fileList);
                   BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
                   BDBG_MSG_TRACE(("%s:removing perm file from tsb list %s",__FUNCTION__,removedFileNode->fileName));
                   BDBG_MSG_TRACE(("%s:next file from tsb list %s",__FUNCTION__,fileNode->fileName));
                   if(removedFileNode->recordType == eB_DVR_RecordingPermanent) 
                   {
                       BDBG_MSG_TRACE(("%s:decreasing the reference count for %s",
                                 __FUNCTION__,removedFileNode->fileName));
                       freeFileSegment(segmentedFile->mediaStorage,
                                       segmentedFile->fileType,
                                       segmentedFile->volumeIndex,
                                       segmentedFile->metaDataSubDir,
                                       removedFileNode);
                   }
                   else
                   {
                       BLST_Q_INSERT_TAIL(&segmentedFile->tsbFreeFileList, removedFileNode, nextFileNode);
                       BDBG_MSG_TRACE(("%s:adding back %s to free list",
                                 __FUNCTION__,removedFileNode->fileName));
                   }
               }
               else
               {
                   fileNode = BLST_Q_FIRST(&segmentedFile->fileList);
                   BDBG_OBJECT_ASSERT(fileNode,B_DVR_SegmentedFileNode);
               }

               if(segmentedFile->linearStartOffset < fileNode->linearStartOffset) 
               {
                   BDBG_MSG(("%s:tsb prev bounds %lld %lld",
                            __FUNCTION__,segmentedFile->linearStartOffset, segmentedFile->linearCurrentOffset));
                   segmentedFile->linearStartOffset = fileNode->linearStartOffset;
                   BDBG_MSG(("%s:tsb curr bounds %lld %lld",
                            __FUNCTION__,segmentedFile->linearStartOffset, segmentedFile->linearCurrentOffset));
               }
           }
           fileNode = segmentedFile->pCachedFileNode;
           settings.fileIO = eB_DVR_FileIOWrite;
           settings.directIO = true;
           settings.create = false;
           B_DVR_MediaStorage_GetMediaPath(segmentedFile->mediaStorage,segmentedFile->volumeIndex,path);
           BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s", path,fileNode->fileName);
           rc = B_DVR_File_Open(&segmentedFile->dvrFile,fullFileName,&settings);
           if(rc!=B_DVR_SUCCESS)
           {
               BDBG_ERR(("%s: file open error %s %lld %u",
                         __FUNCTION__,fullFileName,
                         segmentedFile->linearCurrentOffset,
                         segmentedFile->fileCount));
               if(segmentedFile->registeredCallback)
                {
                    BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
                    segmentedFile->registeredCallback((void *)segmentedFile,B_DVR_SegmentedFileEventOpenError);
                    BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
                }
               break;
           }
           else
           {
               BDBG_MSG(("%s: %s: file open %s lso:%lld lco:%lld",
                         __FUNCTION__,segmentedFile->fileName,
                         fullFileName,
                         segmentedFile->linearStartOffset,
                         segmentedFile->linearCurrentOffset));
           }
           segmentedFile->fileCount++;
        }
        to_write = length;
        if((segmentedFile->segmentedSize)  && (to_write > (segmentedFile->segmentedSize - fileNode->size)))
        {
            to_write = segmentedFile->segmentedSize - fileNode->size;
        }
        segmentedFileOffset = segmented_posix_seekForWrite(&segmentedFile->dvrFile.fileInterface.io.writeFD,fileNode->size, SEEK_SET);
        if (segmentedFileOffset != fileNode->size)
        {
           if (returnSize==0)
           {
               if (segmentedFileOffset < 0)
               {
                   returnSize = (ssize_t)segmentedFileOffset; 
               }
               else
               {
                   returnSize = -1;
               }
               if(segmentedFile->registeredCallback)
               {
                   BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
                   segmentedFile->registeredCallback((void *)segmentedFile,B_DVR_SegmentedFileEventSeekError);
                   BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
               }
               BDBG_MSG(("%s:file seek error %s off:%lld,ret off:%lld",
                         __FUNCTION__,
                         fileNode->fileName,
                         fileNode->size,
                         segmentedFileOffset));
               BDBG_ASSERT(rc);
           }
           break;
       }

        sizeWritten = segmentedFile->dvrFile.fileInterface.io.writeFD.write(&segmentedFile->dvrFile.fileInterface.io.writeFD, buf, to_write);
        if (sizeWritten<=0)
        {
            if  (returnSize==0)
            {
                returnSize = sizeWritten; 
                
            }
            if(segmentedFile->registeredCallback)
            {
                BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
                segmentedFile->registeredCallback((void *)segmentedFile,B_DVR_SegmentedFileEventWriteError);
                BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
            }
            BDBG_ERR(("%s:media file write error %ld %s",__FUNCTION__,(long)sizeWritten,fileNode->fileName));
            break;
        }
        BDBG_ASSERT((size_t)sizeWritten <= to_write);
        returnSize += sizeWritten;
        length -= (size_t) sizeWritten;
        buf = (const uint8_t *)buf + sizeWritten;
        fileNode->size += sizeWritten;
        segmentedFile->linearCurrentOffset += sizeWritten;

        if(returnSize>0)
        { 
            if((segmentedFile->service == eB_DVR_ServiceRecord) ||
                (segmentedFile->service == eB_DVR_ServiceMedia))
            {
                writeMetaDataFile(&segmentedFile->dvrFileMetaData,segmentedFile->pCachedFileNode);
            }
        }
    }
    return returnSize; 
}

static ssize_t segmented_posix_write(bfile_io_write_t fd, const void *buf_, size_t length)
{
    B_DVR_SegmentedFileHandle segmentedFile = (B_DVR_SegmentedFileHandle) fd;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)(segmentedFile->context);
    ssize_t returnSize = 0;
    BDBG_ASSERT(segmentedFile);
    checkFileSegmentPair(segmentedFileRecord);
    BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
    if(segmentedFile->fileType == eB_DVR_FileTypeMedia)
    {
       returnSize = segmented_posix_mediaWrite(segmentedFile,buf_,length);
    }
    else
    {
       returnSize = segmented_posix_navWrite(segmentedFile,buf_,length);
    }
    BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
    return returnSize;
}


static const struct bfile_io_read segmented_posix_io_read = 
{
    segmented_posix_read,
    segmented_posix_seek,
    segmented_posix_bounds,
    BIO_DEFAULT_PRIORITY
};

static const struct bfile_io_write segmented_posix_io_write = 
{
    segmented_posix_write,
    NULL,
    BIO_DEFAULT_PRIORITY
};

static long segmentedRecord_nav_read(void *buffer, long size, long count, void *fp )
{
    bfile_io_read_t f = (&((B_DVR_SegmentedFileRecordHandle) fp)->navRead->fileInterface.io.readFD);
    long rc=0;
    rc = (long)f->read(f, buffer, (size_t)(count*size));
    return rc;
}

static long segmentedRecord_nav_tell( void *fp )
{
    bfile_io_read_t f = (&((B_DVR_SegmentedFileRecordHandle) fp)->navRead->fileInterface.io.readFD);
    long rc=0;
    rc = (long)f->seek(f, 0, SEEK_CUR);
    return rc;
}

static int segmentedRecord_nav_bounds( BNAV_Player_Handle handle, void *fp, long *firstIndex, long *lastIndex )
{
    BDBG_ASSERT(fp);
    off_t first, last;
    B_DVR_SegmentedFileHandle segmentedFile = ((B_DVR_SegmentedFileRecordHandle) fp)->navRead;
    unsigned navEntrySize =((B_DVR_SegmentedFileRecordHandle)fp)->navEntrySize;
    if(navEntrySize == 0)
    {
       *firstIndex = 0; *lastIndex = 0;
       return -1;
    }
    BSTD_UNUSED(handle);
    BDBG_ASSERT(segmentedFile);
    BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
    segmented_posix_boundsUpdate(segmentedFile, &first, &last );
    BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
    BSTD_UNUSED(handle);
    *firstIndex = first/navEntrySize;
    *lastIndex = (last-1)/navEntrySize;
     return 0;
}

static int segmentedRecord_nav_seek( void *fp, long offset, int origin )
{
    bfile_io_read_t f = (&((B_DVR_SegmentedFileRecordHandle) fp)->navRead->fileInterface.io.readFD);
    off_t rc=0;
    rc = f->seek(f, (off_t)offset, origin);
    if ( rc == (off_t)-1)
    {
        return -1;
    }
    return 0;
}

static long segmentedPlay_nav_read(void *buffer, long size, long count, void *fp )
{
    long rc=0;
    bfile_io_read_t f = (&((B_DVR_SegmentedFilePlayHandle) fp)->navRead->fileInterface.io.readFD);
    rc = (long)f->read(f, buffer, (size_t)(count*size));
    return rc;
}

static long segmentedPlay_nav_tell( void *fp )
{
    long rc=0;
    bfile_io_read_t f = (&((B_DVR_SegmentedFilePlayHandle) fp)->navRead->fileInterface.io.readFD);
    rc = (long)f->seek(f, 0, SEEK_CUR);
    return rc;
}

static int segmentedPlay_nav_bounds( BNAV_Player_Handle handle, void *fp, long *firstIndex, long *lastIndex )
{
    bfile_io_read_t fd = (&((B_DVR_SegmentedFilePlayHandle) fp)->navRead->fileInterface.io.readFD);
    off_t first, last;
    B_DVR_SegmentedFileHandle segmentedFile;
    unsigned navEntrySize =((B_DVR_SegmentedFilePlayHandle)fp)->navEntrySize;
    if(navEntrySize == 0)
    {
       *firstIndex = 0; *lastIndex = 0;
       return -1;
    }
    segmentedFile = (B_DVR_SegmentedFileHandle)fd;
    BDBG_ASSERT(segmentedFile);
    BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
    segmented_posix_boundsUpdate(segmentedFile, &first, &last );
    BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
    BSTD_UNUSED(handle);
    *firstIndex = first/navEntrySize;
    *lastIndex = (last-1)/navEntrySize;
     return 0;
}

static int segmentedPlay_nav_seek( void *fp, long offset, int origin )
{
    bfile_io_read_t f = (&((B_DVR_SegmentedFilePlayHandle) fp)->navRead->fileInterface.io.readFD);
    off_t rc;
    rc = f->seek(f, (off_t)offset, origin);
    if ( rc == (off_t)-1)
    {
        return -1;
    }
    return 0;
}


static B_DVR_ERROR B_DVR_SegmentedFileRecord_P_OpenNavPlayer(B_DVR_SegmentedFileRecordHandle segmentedFileRecord)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;

    if (segmentedFileRecord->navPlayer==NULL)
    {
        BNAV_DecoderFeatures features = {1,1,1,1};
        BNAV_Player_Settings cfg;
        BNAV_Player_GetDefaultSettings(&cfg);
        cfg.videoPid = 0x1FFF; /* since BNAV_Player doesn't like 0 */
        cfg.filePointer = segmentedFileRecord;
        cfg.normalPlayBufferSize = 1024*128;
        cfg.decoderFeatures = features;
        cfg.readCb = segmentedRecord_nav_read;
        cfg.tellCb = segmentedRecord_nav_tell;
        cfg.seekCb   = segmentedRecord_nav_seek;
        cfg.boundsCb = segmentedRecord_nav_bounds;
        cfg.changingBounds = true;
        cfg.transportTimestampEnabled = false;
        /*
         * During overflow recovery, nav player is opened 
         * again and an attempt to check the nav version 
         *  is made based on default nav entry size which
         *  is 32 bytes. For H264 TSB buffering,
         *  the nav index range is incorrectly calculated
         *  based on the defaults.
         */
        if(segmentedFileRecord->navEntrySize==sizeof(BNAV_AVC_Entry)) 
        {
            cfg.navVersion = BNAV_Version_AVC;
        }
        if (BNAV_Player_Open(&segmentedFileRecord->navPlayer, &cfg)!=0)
        {
            BDBG_ERR(("%s:nav player open failed",__FUNCTION__));
            return B_DVR_NOT_INITIALIZED;
        }
    }
    return rc;
}


static B_DVR_ERROR B_DVR_SegmentedFilePlay_P_OpenNavPlayer(B_DVR_SegmentedFilePlayHandle segmentedFilePlay)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    if (segmentedFilePlay->navPlayer==NULL)
    {
        BNAV_DecoderFeatures features = {1,1,1,1};
        BNAV_Player_Settings cfg;
        BNAV_Player_GetDefaultSettings(&cfg);
        cfg.videoPid = 0x1FFF; /* since BNAV_Player doesn't like 0 */
        cfg.filePointer = segmentedFilePlay;
        cfg.normalPlayBufferSize = 1024*128;
        cfg.decoderFeatures = features;
        cfg.readCb = segmentedPlay_nav_read;
        cfg.tellCb = segmentedPlay_nav_tell;
        cfg.seekCb   = segmentedPlay_nav_seek;
        cfg.boundsCb = segmentedPlay_nav_bounds;
        cfg.transportTimestampEnabled = false;
        cfg.changingBounds = true;
        
        if(segmentedFilePlay->navEntrySize==sizeof(BNAV_AVC_Entry)) 
        {
            cfg.navVersion = BNAV_Version_AVC;
        }
        if (BNAV_Player_Open(&segmentedFilePlay->navPlayer, &cfg)!=0)
        {
            BDBG_ERR(("%s:nav player open failed",__FUNCTION__));
            return B_DVR_NOT_INITIALIZED;
        }
    }
    return rc;
}

B_DVR_SegmentedFileHandle B_DVR_SegmentedFile_Open(
    const char *fileName,
    B_DVR_SegmentedFileSettings *pSettings,
    B_DVR_FileType fileType,
    B_DVR_FileIO fileIO,
    B_DVR_SegmentedFile *tsbRecordFile,
    void *context)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_SegmentedFileHandle segmentedFile=NULL;
    B_DVR_FileSettings settings;
    char path[B_DVR_MAX_FILE_NAME_LENGTH];
    #if SEGMENTED_FILEIO_TIMING
    B_Time startTime,endTime;
    B_Time_Get(&startTime);
    #endif
    segmentedFile = BKNI_Malloc(sizeof(*segmentedFile));
    if(!segmentedFile)
    {
        BDBG_ERR(("%s:segmentedFile alloc failed",__FUNCTION__));
        goto error_alloc;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*segmentedFile),
                                            true,__FUNCTION__,__LINE__);
    BKNI_Memset((void*)segmentedFile,0,sizeof(*segmentedFile));
    segmentedFile->fileIO = fileIO;
    segmentedFile->fileType = fileType;
    segmentedFile->context = context;
    segmentedFile->service = pSettings->service;
    segmentedFile->serviceIndex = pSettings->serviceIndex;
    segmentedFile->registeredCallback = pSettings->registeredCallback;
    segmentedFile->volumeIndex = pSettings->volumeIndex;
    segmentedFile->mediaStorage = pSettings->mediaStorage;
    segmentedFile->fileCount = 0;
    segmentedFile->linearCurrentOffset = 0;
    segmentedFile->fileIndex = 0;
    segmentedFile->linearEndOffset =0;
    segmentedFile->linearStartOffset = 0;
    segmentedFile->maxFileCount = pSettings->maxSegmentCount;
    segmentedFile->tsbRecordFile = tsbRecordFile;
    segmentedFile->refCnt=1;
    segmentedFile->pCachedFileNode = NULL;
    segmentedFile->tsbConversion = false;
    segmentedFile->segmentedSize = pSettings->mediaSegmentSize;
    segmentedFile->event = pSettings->event;
    segmentedFile->metaDataSubDir = pSettings->metaDataSubDir;
    segmentedFile->itbThreshhold = pSettings->itbThreshhold;
    segmentedFile->timeStamp = 0;
    segmentedFile->mediaLinearOffset = 0;
    
    B_DVR_MediaStorage_GetMetadataPath(segmentedFile->mediaStorage,segmentedFile->volumeIndex,path);
    if(segmentedFile->metaDataSubDir && segmentedFile->metaDataSubDir[0] != '\0')
    { 
        BKNI_Snprintf(segmentedFile->fileName, sizeof(segmentedFile->fileName), "%s/%s/%s", path,segmentedFile->metaDataSubDir,fileName);
    }
    else
    {
        BKNI_Snprintf(segmentedFile->fileName, sizeof(segmentedFile->fileName), "%s/%s", path,fileName);
    }
    if(tsbRecordFile)
    {
        BKNI_AcquireMutex(tsbRecordFile->metaDataFileMutex);
        segmentedFile->metaDataFileMutex = tsbRecordFile->metaDataFileMutex;
        segmentedFile->tsbRecordFile = tsbRecordFile;
        tsbRecordFile->refCnt++;
        BKNI_ReleaseMutex(tsbRecordFile->metaDataFileMutex);

    }
    else
    {
       rc = BKNI_CreateMutex(&segmentedFile->metaDataFileMutex);
    }

    BLST_Q_INIT(&segmentedFile->fileList);
    BLST_Q_INIT(&segmentedFile->tsbFreeFileList);

    if(fileIO == eB_DVR_FileIORead)
    {
       if(pSettings->service == eB_DVR_ServicePlayback || pSettings->service == eB_DVR_ServiceMedia)
       {
           settings.directIO = false;
           settings.fileIO = eB_DVR_FileIORead;
           settings.create = false;
           rc = B_DVR_File_Open(&segmentedFile->dvrFileMetaData,segmentedFile->fileName,&settings);
           if(rc!=B_DVR_SUCCESS)
           {
               BDBG_ERR(("%s:pb file open failed %s",__FUNCTION__,segmentedFile->dvrFileMetaData.fileName));
               goto file_OpenError;
           }
           else
           {
               BDBG_MSG(("%s:playback metaData file opened %s",
                         __FUNCTION__,
                         segmentedFile->dvrFileMetaData.fileName));

           }
           rc = populateFileList(segmentedFile);

           if(rc!=B_DVR_SUCCESS || !segmentedFile->fileCount)
           {
               BDBG_MSG(("%s:no file segments found in %s",
                         __FUNCTION__,
                         segmentedFile->dvrFileMetaData.fileName));
               goto fileNode_Error;
           }
       }
       segmentedFile->fileInterface.io.readFD = segmented_posix_io_read;
    }
    else
    {        
        if(pSettings->service == eB_DVR_ServiceTSB)
        {
            settings.directIO = false;
            settings.fileIO = eB_DVR_FileIORead;
            settings.create = false;
            rc = B_DVR_File_Open(&segmentedFile->dvrFileMetaData,segmentedFile->fileName,&settings);
            if(rc!=B_DVR_SUCCESS)
            {
                BDBG_ERR(("%s:pre-created TSB metaDataFile failed to open %s",__FUNCTION__,segmentedFile->fileName));
                goto file_OpenError;
                
            }
            rc = populateFileList(segmentedFile);
            if(rc!=B_DVR_SUCCESS || !segmentedFile->fileCount)
            {
                BDBG_ERR(("%s:pre-allocated TSB segments not found",__FUNCTION__));
                goto fileNode_Error;
            }
            segmentedFile->fileCount=0;
            B_DVR_File_Close(&segmentedFile->dvrFileMetaData);

        }
        settings.directIO = false;
        settings.fileIO = eB_DVR_FileIOWrite;
        if(pSettings->service != eB_DVR_ServiceTSB)
        {
            settings.create = true;
        }
        else
        {
            settings.create = false;
        }
        rc = B_DVR_File_Open(&segmentedFile->dvrFileMetaData,segmentedFile->fileName,&settings);
        if(rc!=B_DVR_SUCCESS)
        {
            BDBG_ERR(("%s:dvr file open error in write mode",__FUNCTION__));
            goto file_OpenError;
        }
        else
        {
            BDBG_MSG(("%s: TSB meta file opened %s",
                      __FUNCTION__,
                      segmentedFile->dvrFileMetaData.fileName));
        }
        segmentedFile->fileInterface.io.writeFD = segmented_posix_io_write;
    }
    #if SEGMENTED_FILEIO_TIMING
    B_Time_Get(&endTime);
    BDBG_MSG_TRACE(("time taken by %s:%ld",__FUNCTION__,B_Time_Diff(&endTime,&startTime)));
    #endif
    return segmentedFile;
fileNode_Error:
    B_DVR_File_Close(&segmentedFile->dvrFileMetaData);
file_OpenError:
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*segmentedFile),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(segmentedFile);
error_alloc:
    return NULL;
}

void B_DVR_SegmentedFile_Close(B_DVR_SegmentedFileHandle segmentedFile)
{
    #if SEGMENTED_FILEIO_TIMING
    B_Time startTime,endTime;
    B_Time_Get(&startTime);
    #endif
    BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
    B_DVR_SegmentedFileNode *fileNode=NULL;
    while((fileNode=BLST_Q_FIRST(&segmentedFile->tsbFreeFileList))!=NULL)
    {
        BLST_Q_REMOVE_HEAD(&segmentedFile->tsbFreeFileList,nextFileNode);
        BDBG_MSG_TRACE(("%s:remove %s from tsb list",
                  __FUNCTION__,fileNode->fileName));
        BDBG_OBJECT_DESTROY(fileNode,B_DVR_SegmentedFileNode);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*fileNode),
                                                false,__FUNCTION__,__LINE__);
        BKNI_Free(fileNode);
    }
    while((fileNode=BLST_Q_FIRST(&segmentedFile->fileList))!=NULL)
    {
        BDBG_MSG_TRACE(("%s: remove %s from active list",
                  __FUNCTION__,fileNode->fileName));
        BLST_Q_REMOVE_HEAD(&segmentedFile->fileList,nextFileNode);
        if((segmentedFile->service == eB_DVR_ServiceTSB)
           && (fileNode->recordType == eB_DVR_RecordingPermanent)) 
        {
            BDBG_MSG_TRACE(("%s:decreasing the reference count for %s",
                      __FUNCTION__,fileNode->fileName));
            freeFileSegment(segmentedFile->mediaStorage,
                            segmentedFile->fileType,
                            segmentedFile->volumeIndex,
                            segmentedFile->metaDataSubDir,
                            fileNode);
        }
        else
        {
            BDBG_OBJECT_DESTROY(fileNode,B_DVR_SegmentedFileNode);
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*fileNode),false,
                                                    __FUNCTION__,__LINE__);
            BKNI_Free(fileNode);
        }
    }

    if(segmentedFile->tsbRecordFile)
    {
        segmentedFile->tsbRecordFile->refCnt--;
    }

    B_DVR_File_Close(&segmentedFile->dvrFileMetaData);
    B_DVR_File_Close(&segmentedFile->dvrFile);
   
    BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
    if(!segmentedFile->tsbRecordFile)

    { 
        BKNI_DestroyMutex(segmentedFile->metaDataFileMutex);
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*segmentedFile),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(segmentedFile);
    #if SEGMENTED_FILEIO_TIMING
    B_Time_Get(&endTime);
    BDBG_ERR(("time taken by %s:%ld",__FUNCTION__,B_Time_Diff(&endTime,&startTime)));
    #endif
    return;
}

void B_DVR_SegmentedFilePlay_Close(
    NEXUS_FilePlayHandle nexusFilePlay)
{  
    B_DVR_SegmentedFilePlayHandle segmentedFilePlay = (B_DVR_SegmentedFilePlayHandle)nexusFilePlay;
    #if SEGMENTED_FILEIO_TIMING
    B_Time startTime,endTime;
    B_Time_Get(&startTime);
    #endif
    BKNI_AcquireMutex(segmentedFilePlay->navPlayerMutex);
    if(segmentedFilePlay->navPlayer)
    {
        BNAV_Player_Close(segmentedFilePlay->navPlayer);
    }
    BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
    BKNI_DestroyMutex(segmentedFilePlay->navPlayerMutex);
    if(segmentedFilePlay->navRead)
    { 
        B_DVR_SegmentedFile_Close(segmentedFilePlay->navRead);
    }
    B_DVR_SegmentedFile_Close(segmentedFilePlay->mediaRead);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*segmentedFilePlay),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(segmentedFilePlay);
    #if SEGMENTED_FILEIO_TIMING
    B_Time_Get(&endTime);
    BDBG_MSG_TRACE(("time taken by %s:%ld",__FUNCTION__,B_Time_Diff(&endTime,&startTime)));
    #endif
    return;
}

NEXUS_FilePlayHandle B_DVR_SegmentedFilePlay_Open(
    const char *mediaFileName,
    const char *navFileName,
    B_DVR_SegmentedFileSettings *pSettings,
    NEXUS_FileRecordHandle nexusFileRecord)
{
    
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_SegmentedFilePlayHandle segmentedFilePlay=NULL;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)nexusFileRecord;
    #if SEGMENTED_FILEIO_TIMING
    B_Time startTime,endTime;
    B_Time_Get(&startTime);
    #endif
    if (mediaFileName==NULL)
    {
        BDBG_ERR(("%s:Invalid media file",__FUNCTION__));
        rc = B_DVR_INVALID_PARAMETER;
        return NULL;
    }

    if (navFileName[0]=='\0')
    {
        BDBG_ERR(("%s:no nav file",__FUNCTION__));
    }

    segmentedFilePlay = BKNI_Malloc(sizeof(*segmentedFilePlay));
    if (!segmentedFilePlay)
    {
        BDBG_ERR(("%s:Unable to allocate segmentedFilePlay",__FUNCTION__));
        rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
        goto error_alloc;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*segmentedFilePlay),
                                            true,__FUNCTION__,__LINE__);
    BKNI_Memset(segmentedFilePlay, 0, sizeof(*segmentedFilePlay));
    segmentedFilePlay->navEntrySize= 0;

    if(navFileName[0]!='\0')
    { 
        segmentedFilePlay->navRead = B_DVR_SegmentedFile_Open(navFileName,pSettings,
                                                              eB_DVR_FileTypeNavigation,
                                                              eB_DVR_FileIORead,
                                                              nexusFileRecord?segmentedFileRecord->navWrite:NULL,
                                                              segmentedFilePlay);
        if(!segmentedFilePlay->navRead)
        {
           BDBG_ERR(("%s:segmentedFilePlay navRead open failed %s",__FUNCTION__,navFileName));   
           goto error_navRead_open;
        }
    }

    rc = BKNI_CreateMutex(&segmentedFilePlay->navPlayerMutex);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:Creat navPlayerMutex failed! %s",__FUNCTION__,navFileName));   
        goto error_mutex_create;
    }

    segmentedFilePlay->mediaRead = B_DVR_SegmentedFile_Open(mediaFileName,
                                                              pSettings,
                                                              eB_DVR_FileTypeMedia,
                                                              eB_DVR_FileIORead,
                                                              nexusFileRecord?segmentedFileRecord->mediaWrite:NULL,
                                                              segmentedFilePlay);
    if(!segmentedFilePlay->mediaRead)
    {
        BDBG_ERR(("%s:segmentedFilePlay mediaRead open failed %s",__FUNCTION__,mediaFileName));   
        goto error_mediaRead_open;

    }

    segmentedFilePlay->nexusFilePlay.file.index = &segmentedFilePlay->navRead->fileInterface.io.readFD;
    segmentedFilePlay->nexusFilePlay.file.data = &segmentedFilePlay->mediaRead->fileInterface.io.readFD;
    segmentedFilePlay->nexusFilePlay.file.close = B_DVR_SegmentedFilePlay_Close;
    #if SEGMENTED_FILEIO_TIMING
    B_Time_Get(&endTime);
    BDBG_MSG_TRACE(("time taken by %s:%ld",__FUNCTION__,B_Time_Diff(&endTime,&startTime)));
    #endif
    return &segmentedFilePlay->nexusFilePlay;

error_mutex_create:
error_mediaRead_open:
    if(segmentedFilePlay->navRead)
    { 
        B_DVR_SegmentedFile_Close(segmentedFilePlay->navRead);
    }
error_navRead_open:
    if(segmentedFilePlay->mediaRead)
    { 
        B_DVR_SegmentedFile_Close(segmentedFilePlay->mediaRead);
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*segmentedFilePlay),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(segmentedFilePlay);
error_alloc:
    return NULL;
}

B_DVR_ERROR B_DVR_SegmentedFilePlay_GetDefaultSettings(
    B_DVR_SegmentedFilePlaySettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_ASSERT(pSettings);
    BKNI_Memset((void *)pSettings,0,sizeof(B_DVR_SegmentedFilePlaySettings));
    pSettings->navEntrySize=sizeof(BNAV_Entry);
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFilePlay_GetSettings(
    NEXUS_FilePlayHandle nexusFilePlay,
    B_DVR_SegmentedFilePlaySettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_ASSERT(nexusFilePlay);
    BDBG_ASSERT(pSettings);
    B_DVR_SegmentedFilePlayHandle segmentedFilePlay = (B_DVR_SegmentedFilePlayHandle)nexusFilePlay;
    BKNI_AcquireMutex(segmentedFilePlay->mediaRead->metaDataFileMutex);
    pSettings->mediaLinearStartOffset =segmentedFilePlay->mediaRead->linearStartOffset;
    pSettings->mediaLinearEndOffset = segmentedFilePlay->mediaRead->linearEndOffset ;
    BKNI_ReleaseMutex(segmentedFilePlay->mediaRead->metaDataFileMutex);

    if(segmentedFilePlay->navRead)
    { 
        BKNI_AcquireMutex(segmentedFilePlay->navRead->metaDataFileMutex);
        pSettings->navLinearStartOffset = segmentedFilePlay->navRead->linearStartOffset;
        pSettings->navLinearEndOffset = segmentedFilePlay->navRead->linearEndOffset;
        pSettings->navEntrySize = segmentedFilePlay->navEntrySize;
        BKNI_ReleaseMutex(segmentedFilePlay->navRead->metaDataFileMutex);
    }
    else
    {
        pSettings->navLinearEndOffset = 0;
        pSettings->navLinearStartOffset =0;
        pSettings->navEntrySize=0;
    }
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFilePlay_SetSettings(
    NEXUS_FilePlayHandle nexusFilePlay,
    B_DVR_SegmentedFilePlaySettings *pSettings )
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_ASSERT(nexusFilePlay);
    BDBG_ASSERT(pSettings);
    B_DVR_SegmentedFilePlayHandle segmentedFilePlay = (B_DVR_SegmentedFilePlayHandle)nexusFilePlay;
    BKNI_AcquireMutex(segmentedFilePlay->mediaRead->metaDataFileMutex);
    segmentedFilePlay->mediaRead->linearStartOffset = pSettings->mediaLinearStartOffset;
    segmentedFilePlay->mediaRead->linearEndOffset =   pSettings->mediaLinearEndOffset;
    BKNI_ReleaseMutex(segmentedFilePlay->mediaRead->metaDataFileMutex);

    if(segmentedFilePlay->navRead)
    { 
        BKNI_AcquireMutex(segmentedFilePlay->navRead->metaDataFileMutex);
        segmentedFilePlay->navRead->linearStartOffset = pSettings->navLinearStartOffset;
        segmentedFilePlay->navRead->linearEndOffset =   pSettings->navLinearEndOffset;
        segmentedFilePlay->navEntrySize = pSettings->navEntrySize;
        BKNI_ReleaseMutex(segmentedFilePlay->navRead->metaDataFileMutex);
    }
    return rc;
}

void B_DVR_SegmentedFileRecord_Close(
    NEXUS_FileRecordHandle nexusFileRecord)
{
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)nexusFileRecord;
    #if SEGMENTED_FILEIO_TIMING
    B_Time startTime,endTime;
    B_Time_Get(&startTime);
    #endif
    BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
    if(segmentedFileRecord->navPlayer)
    {
        BNAV_Player_Close(segmentedFileRecord->navPlayer);
    }
    BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
    BKNI_DestroyMutex(segmentedFileRecord->navPlayerMutex);
    if(segmentedFileRecord->navRead)
    { 
        B_DVR_SegmentedFile_Close(segmentedFileRecord->navRead);
    }
    if(segmentedFileRecord->navWrite)
    {
        B_DVR_SegmentedFile_Close(segmentedFileRecord->navWrite);
    }
    B_DVR_SegmentedFile_Close(segmentedFileRecord->mediaWrite);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*segmentedFileRecord),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(segmentedFileRecord);
    #if SEGMENTED_FILEIO_TIMING
    B_Time_Get(&endTime);
    BDBG_MSG_TRACE(("time taken by %s:%ld",__FUNCTION__,B_Time_Diff(&endTime,&startTime)));
    #endif
    return;
}

NEXUS_FileRecordHandle B_DVR_SegmentedFileRecord_Open(
    const char *mediaFileName, 
    const char *navFileName,
    B_DVR_SegmentedFileSettings *pSettings)
{
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord;
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    #if SEGMENTED_FILEIO_TIMING
    B_Time startTime,endTime;
    B_Time_Get(&startTime);
    #endif
    if (mediaFileName==NULL)
    {
        BDBG_ERR(("%s:invalid media file name",__FUNCTION__));
        rc=B_DVR_INVALID_PARAMETER;
        return NULL;
    }

    if (navFileName==NULL)
    {
        BDBG_WRN(("no index file name"));
    }

    segmentedFileRecord = BKNI_Malloc(sizeof(*segmentedFileRecord));
    if (!segmentedFileRecord)
    {
        BDBG_ERR(("%s:unable to allocate memory for segmentedFileRecord",__FUNCTION__));
        rc=B_DVR_INVALID_PARAMETER;
        goto error_alloc;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*segmentedFileRecord),
                                            true,__FUNCTION__,__LINE__);
    BKNI_Memset(segmentedFileRecord, 0, sizeof(*segmentedFileRecord));
    segmentedFileRecord->maxTSBTime = pSettings->maxTSBTime;

    if(navFileName)
    { 
        segmentedFileRecord->navWrite = B_DVR_SegmentedFile_Open(navFileName,
                                                                 pSettings,
                                                                 eB_DVR_FileTypeNavigation,
                                                                 eB_DVR_FileIOWrite,
                                                                 NULL,
                                                                 segmentedFileRecord);
        if(!segmentedFileRecord->navWrite)
        {
            BDBG_ERR(("%s:segmentedFileRecord navWrite open failed %s",__FUNCTION__,navFileName));   
            goto error_navWrite_open;
        }
        segmentedFileRecord->navRead = B_DVR_SegmentedFile_Open(navFileName,
                                                                pSettings,
                                                                eB_DVR_FileTypeNavigation,
                                                                eB_DVR_FileIORead,
                                                                segmentedFileRecord->navWrite,
                                                                segmentedFileRecord);
        if(!segmentedFileRecord->navRead)
        {
            BDBG_ERR(("%s:segmentedFileRecord navRead open failed %s",__FUNCTION__,navFileName));   
            goto error_navRead_open;
        }
    }

    rc = BKNI_CreateMutex(&segmentedFileRecord->navPlayerMutex);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:Creat navPlayerMutex failed! %s",__FUNCTION__,navFileName));   
        goto error_mutex_create;
    }

    segmentedFileRecord->mediaWrite = B_DVR_SegmentedFile_Open(mediaFileName,
                                                               pSettings,
                                                               eB_DVR_FileTypeMedia,
                                                               eB_DVR_FileIOWrite,
                                                               NULL,
                                                               segmentedFileRecord);

    if(!segmentedFileRecord->mediaWrite)
    {
        BDBG_ERR(("%s:segmentedFileRecord navRead open failed %s",__FUNCTION__,mediaFileName));   
        goto error_mediaWrite_open;

    }

    segmentedFileRecord->navEntrySize = 0;
    segmentedFileRecord->cdbSize = pSettings->cdbSize;
    segmentedFileRecord->itbSize = pSettings->itbSize;
    segmentedFileRecord->segmentNum=0;
    segmentedFileRecord->nexusFileRecord.data =  &segmentedFileRecord->mediaWrite->fileInterface.io.writeFD;
    segmentedFileRecord->nexusFileRecord.index = &segmentedFileRecord->navWrite->fileInterface.io.writeFD;
    segmentedFileRecord->nexusFileRecord.close = B_DVR_SegmentedFileRecord_Close;
    #if SEGMENTED_FILEIO_TIMING
    B_Time_Get(&endTime);
    BDBG_MSG_TRACE(("time taken by %s:%ld",__FUNCTION__,B_Time_Diff(&endTime,&startTime)));
    #endif
    return &segmentedFileRecord->nexusFileRecord;

error_mediaWrite_open:
    BKNI_DestroyMutex(segmentedFileRecord->navPlayerMutex);
error_mutex_create:
    if(navFileName && segmentedFileRecord->navRead)
    { 
        B_DVR_SegmentedFile_Close(segmentedFileRecord->navRead);
    }
error_navRead_open:
    if(navFileName && segmentedFileRecord->navWrite)
    { 
        B_DVR_SegmentedFile_Close(segmentedFileRecord->navWrite);
    }
error_navWrite_open:
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*segmentedFileRecord),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(segmentedFileRecord);
error_alloc:
    return NULL;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_GetDefaultSettings(
    B_DVR_SegmentedFileRecordSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_ASSERT(pSettings);
    BKNI_Memset((void *)pSettings,0,sizeof(B_DVR_SegmentedFileRecordSettings));
    pSettings->navEntrySize = sizeof(BNAV_Entry);
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_GetSettings(
    NEXUS_FileRecordHandle nexusFileRecord,
    B_DVR_SegmentedFileRecordSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_ASSERT(nexusFileRecord);
    BDBG_ASSERT(pSettings);
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)(nexusFileRecord);

    if(segmentedFileRecord->navWrite) 
    {
        BKNI_AcquireMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
        pSettings->navLinearStartOffset = segmentedFileRecord->navWrite->linearStartOffset;
        pSettings->navLinearEndOffset = segmentedFileRecord->navWrite->linearCurrentOffset;
        pSettings->navEntrySize = segmentedFileRecord->navEntrySize;
        BKNI_ReleaseMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
    }
    else
    {
        pSettings->navLinearStartOffset = 0;
        pSettings->navLinearEndOffset = 0;
        pSettings->navEntrySize = 0;
    }

    if(segmentedFileRecord->mediaWrite) 
    {
        BKNI_AcquireMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
        pSettings->mediaLinearStartOffset = segmentedFileRecord->mediaWrite->linearStartOffset;
        pSettings->mediaLinearEndOffset = segmentedFileRecord->mediaWrite->linearCurrentOffset;
        BKNI_ReleaseMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
    }
    else
    {
        pSettings->mediaLinearStartOffset = 0;
        pSettings->mediaLinearEndOffset = 0;
    }
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_SetSettings(
    NEXUS_FileRecordHandle nexusFileRecord,
    B_DVR_SegmentedFileRecordSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_SegmentedFileNode *fileNode = NULL;
    off_t sizeDiff=0;
    BDBG_ASSERT(nexusFileRecord);
    BDBG_ASSERT(pSettings);
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)(nexusFileRecord);
    if(segmentedFileRecord->navWrite)
    { 
        BKNI_AcquireMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
        BDBG_MSG(("%s:lso:%lld lco:%lld",
                  segmentedFileRecord->navWrite->fileName,
                  segmentedFileRecord->navWrite->linearStartOffset,
                  segmentedFileRecord->navWrite->linearCurrentOffset));
        segmentedFileRecord->navEntrySize = pSettings->navEntrySize;
        segmentedFileRecord->navWrite->linearStartOffset = pSettings->navLinearStartOffset;
        sizeDiff = segmentedFileRecord->navWrite->linearCurrentOffset - pSettings->navLinearEndOffset;
        BDBG_MSG(("nav off diff %lld",sizeDiff));
        segmentedFileRecord->navWrite->linearCurrentOffset = pSettings->navLinearEndOffset;
        BDBG_MSG(("%s:lso:%lld lco:%lld",
                  segmentedFileRecord->navWrite->fileName,
                  segmentedFileRecord->navWrite->linearStartOffset,
                  segmentedFileRecord->navWrite->linearCurrentOffset));
        if(segmentedFileRecord->navWrite->pCachedFileNode)
        {
            fileNode=segmentedFileRecord->navWrite->pCachedFileNode;
            BDBG_MSG(("%s lso:%lld size:%u",
                      fileNode->fileName,
                      fileNode->linearStartOffset,
                      fileNode->size));
            if(sizeDiff > fileNode->size) 
            {
                BDBG_MSG(("%s:nav sizeDiff > fileNode->size",__FUNCTION__));
                fileNode->size = 0; 
                segmentedFileRecord->navWrite->linearCurrentOffset= fileNode->linearStartOffset;   
                BDBG_MSG(("meta:%s seg:%s lso:%lld size:%u",
                        segmentedFileRecord->navWrite->fileName,
                        fileNode->fileName,
                        fileNode->linearStartOffset,
                        fileNode->size));
                
            }
            else
            {
              fileNode->size -= sizeDiff;    
              BDBG_MSG(("%s lso:%lld size:%u",
                        fileNode->fileName,
                        fileNode->linearStartOffset,
                        fileNode->size));
            }
            BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
            if(segmentedFileRecord->navPlayer)
            {
                BNAV_Player_Close(segmentedFileRecord->navPlayer);
                segmentedFileRecord->navPlayer=NULL;
            }
            BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
        }
        BKNI_ReleaseMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
    }
    BKNI_AcquireMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
    BDBG_MSG(("%s:lso:%lld lco:%lld",
              segmentedFileRecord->mediaWrite->fileName,
              segmentedFileRecord->mediaWrite->linearStartOffset,
              segmentedFileRecord->mediaWrite->linearCurrentOffset));
    segmentedFileRecord->mediaWrite->linearStartOffset = pSettings->mediaLinearStartOffset;
    sizeDiff = segmentedFileRecord->mediaWrite->linearCurrentOffset - pSettings->mediaLinearEndOffset;
    BDBG_MSG(("media off diff %lld",sizeDiff));
    segmentedFileRecord->mediaWrite->linearCurrentOffset = pSettings->mediaLinearEndOffset;
    BDBG_MSG(("%s:lso:%lld lco:%lld",
              segmentedFileRecord->mediaWrite->fileName,
              segmentedFileRecord->mediaWrite->linearStartOffset,
              segmentedFileRecord->mediaWrite->linearCurrentOffset));
    if(segmentedFileRecord->mediaWrite->pCachedFileNode)
    {
        fileNode=segmentedFileRecord->mediaWrite->pCachedFileNode;
        BDBG_MSG(("%s lso:%lld size:%u",
                      fileNode->fileName,
                      fileNode->linearStartOffset,
                      fileNode->size));
        if(sizeDiff > fileNode->size) 
        {
            BDBG_MSG(("%s:media sizeDiff > fileNode->size",__FUNCTION__));
            fileNode->size = 0; 
            segmentedFileRecord->mediaWrite->linearCurrentOffset= fileNode->linearStartOffset;   
            BDBG_MSG(("meta:%s seg:%s lso:%lld size:%u",
                    segmentedFileRecord->mediaWrite->fileName,
                    fileNode->fileName,
                    fileNode->linearStartOffset,
                    fileNode->size));
        }
        else
        {
            fileNode->size-=sizeDiff;
            BDBG_MSG(("%s lso:%lld size:%u",
                      fileNode->fileName,
                      fileNode->linearStartOffset,
                      fileNode->size));
        }
    }
    BKNI_ReleaseMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
    return rc;
}

static int B_DVR_SegmentedFile_P_GetBounds(BNAV_Player_Handle handle, void *fp, long *firstIndex, long *lastIndex)
{
    BDBG_ASSERT(fp);
    off_t first, last;
    B_DVR_SegmentedFileHandle segmentedFile = ((B_DVR_SegmentedFileRecordHandle) fp)->navRead;
    unsigned navEntrySize =((B_DVR_SegmentedFileRecordHandle)fp)->navEntrySize;;
    if(navEntrySize == 0)
    {
       *firstIndex = 0; *lastIndex = 0;
       return -1;
    }
    BSTD_UNUSED(handle);
    BDBG_ASSERT(segmentedFile);
    BKNI_AcquireMutex(segmentedFile->metaDataFileMutex);
    segmented_posix_boundsUpdate(segmentedFile, &first, &last );
    BKNI_ReleaseMutex(segmentedFile->metaDataFileMutex);
    *firstIndex = first/navEntrySize;
    *lastIndex = (last-1)/navEntrySize;
    return 0;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_OpenPermMetaDataFile(
    NEXUS_FileRecordHandle nexusFileRecord)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)(nexusFileRecord);
    B_DVR_PermanentRecord *permRec = &segmentedFileRecord->permRec;
    B_DVR_FileSettings settings;
    BDBG_ASSERT(nexusFileRecord);
    settings.fileIO = eB_DVR_FileIOWrite;
    settings.directIO = false;
    settings.create = true;
    rc = B_DVR_File_Open(&permRec->navMetaDataFile,permRec->navFileName, &settings);
    if (rc !=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:Could not open user provided %s file",__FUNCTION__,permRec->navFileName));
        goto nav_open_error;
    }
    else
    {
        BDBG_MSG(("%s: perm metaData file open %s",__FUNCTION__,permRec->navMetaDataFile.fileName));
    }
    settings.fileIO = eB_DVR_FileIOWrite;
    settings.directIO = false;
    settings.create = true;
    permRec->mediaMetaDataFile.fileIO = eB_DVR_FileIOWrite;
    rc = B_DVR_File_Open(&permRec->mediaMetaDataFile,permRec->mediaFileName, &settings);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:Could not open user provided %s mediafilename",__FUNCTION__,permRec->mediaFileName));
        goto media_open_error;
    }
    else
    {
        BDBG_MSG(("%s: perm metaData file open %s",__FUNCTION__,permRec->mediaMetaDataFile.fileName));
    }
    BKNI_AcquireMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
    segmentedFileRecord->mediaWrite->tsbConvMetaDataSubDir = permRec->tsbConvMetaDataSubDir;
    BKNI_ReleaseMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
    BKNI_AcquireMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
    segmentedFileRecord->navWrite->tsbConvMetaDataSubDir = permRec->tsbConvMetaDataSubDir;
    BKNI_ReleaseMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
    return rc;
media_open_error:
    B_DVR_File_Close(&permRec->navMetaDataFile);
nav_open_error:
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_ClosePermMetaDataFile(
    NEXUS_FileRecordHandle nexusFileRecord)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)nexusFileRecord;
    BDBG_ASSERT(nexusFileRecord);
    B_DVR_PermanentRecord *permRec = &segmentedFileRecord->permRec;
    BKNI_AcquireMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
    segmentedFileRecord->mediaWrite->tsbConvMetaDataSubDir = NULL;
    BKNI_ReleaseMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
    BKNI_AcquireMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
    segmentedFileRecord->navWrite->tsbConvMetaDataSubDir = NULL;
    BKNI_ReleaseMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
    BDBG_MSG(("%s: perm metaData file close %s",__FUNCTION__,permRec->mediaMetaDataFile.fileName));
    B_DVR_File_Close(&permRec->mediaMetaDataFile);
    BDBG_MSG(("%s: perm metaData file close %s",__FUNCTION__,permRec->navMetaDataFile.fileName));
    B_DVR_File_Close(&permRec->navMetaDataFile);
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_OpenAppendedPermMetaDataFile(
     B_DVR_SegmentedFileRecordAppendSettings *appendSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_FileSettings fileSettings;
    struct stat sb;
    unsigned long fileSize=0;
    B_DVR_File readMetaDataFile;
    B_DVR_SegmentedFileNode fileNode;
    off_t trunc;
    BDBG_ASSERT(appendSettings);

    fileSettings.create = false;
    fileSettings.directIO = false;
    fileSettings.fileIO = eB_DVR_FileIORead;
    rc = B_DVR_File_Open(&appendSettings->readTempNavMetaDataFile,appendSettings->tempNavMetaDataFileName,&fileSettings);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:error in opening %s in read mode",__FUNCTION__,appendSettings->tempNavMetaDataFileName));
        rc = B_DVR_OS_ERROR;
        goto error_tmpNavFile;
    }

    fileSettings.create = false;
    fileSettings.directIO = false;
    fileSettings.fileIO = eB_DVR_FileIORead;
    rc = B_DVR_File_Open(&appendSettings->readTempMediaMetaDataFile,appendSettings->tempMediaMetaDataFileName,&fileSettings);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:error in opening %s in read mode",__FUNCTION__,appendSettings->tempMediaMetaDataFileName));
        rc = B_DVR_OS_ERROR;
        goto error_tmpMediaFile;
    }

    fileSettings.create = false;
    fileSettings.directIO = false;
    fileSettings.fileIO = eB_DVR_FileIOWrite;
    rc = B_DVR_File_Open(&appendSettings->writeNavMetaDataFile,appendSettings->navMetaDataFileName,&fileSettings);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:error in opening %s in write mode",__FUNCTION__,appendSettings->navMetaDataFileName));
        rc = B_DVR_OS_ERROR;
        goto error_writeNavFile;
    }

    fileSettings.create = false;
    fileSettings.directIO = false;
    fileSettings.fileIO = eB_DVR_FileIOWrite;
    rc = B_DVR_File_Open(&appendSettings->writeMediaMetaDataFile,appendSettings->mediaMetaDataFileName,&fileSettings);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:error in opening %s in write mode",__FUNCTION__,appendSettings->mediaMetaDataFileName));
        rc = B_DVR_OS_ERROR;
        goto error_writeMediaFile;
    }

    fstat(appendSettings->writeMediaMetaDataFile.fd, &sb);
    fileSize=sb.st_size;
    appendSettings->mediaFileCount =fileSize/sizeof(B_DVR_SegmentedFileNode);
    appendSettings->tmpMediaFileCount = 0;

    fstat(appendSettings->writeNavMetaDataFile.fd, &sb);
    fileSize=sb.st_size;
    appendSettings->navFileCount =fileSize/sizeof(B_DVR_SegmentedFileNode);
    appendSettings->tmpNavFileCount = 0;

    appendSettings->navFileOffset =0;
    fileSettings.create = false;
    fileSettings.directIO = false;
    fileSettings.fileIO = eB_DVR_FileIORead;
    rc = B_DVR_File_Open(&readMetaDataFile,appendSettings->mediaMetaDataFileName,&fileSettings);
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("%s:error in opening %s in read mode",__FUNCTION__,appendSettings->mediaMetaDataFileName));
        rc = B_DVR_OS_ERROR;
        goto error_readMediaFile;
    }
    readMetaDataFile.fileInterface.io.readFD.seek(&readMetaDataFile.fileInterface.io.readFD,
                                                  (appendSettings->mediaFileCount-1)*sizeof(fileNode),
                                                  SEEK_SET);
    readMetaDataFile.fileInterface.io.readFD.read(&readMetaDataFile.fileInterface.io.readFD,
                                                  &fileNode,
                                                  sizeof(fileNode));

    trunc = fileNode.size%188;
    fileNode.size -= trunc;
    fileNode.size -= fileNode.linearStartOffset+fileNode.size - appendSettings->appendMediaOffset;
    appendSettings->writeMediaMetaDataFile.fileInterface.io.writeFD.write(&appendSettings->writeMediaMetaDataFile.fileInterface.io.writeFD,
                                                                          &fileNode,
                                                                          sizeof(fileNode));
    B_DVR_File_Close(&readMetaDataFile);

    rc = B_DVR_File_Open(&readMetaDataFile,appendSettings->navMetaDataFileName,&fileSettings); 
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("%s:error in opening %s in read mode",__FUNCTION__,appendSettings->navMetaDataFileName));
        rc = B_DVR_OS_ERROR;
        goto error_readNavFile;
    }
    readMetaDataFile.fileInterface.io.readFD.seek(&readMetaDataFile.fileInterface.io.readFD,
                                                  (appendSettings->navFileCount-1)*sizeof(fileNode),
                                                  SEEK_SET);
    readMetaDataFile.fileInterface.io.readFD.read(&readMetaDataFile.fileInterface.io.readFD,
                                                  &fileNode,
                                                  sizeof(fileNode));
    fileNode.size -= fileNode.linearStartOffset+fileNode.size - appendSettings->appendNavOffset;
    appendSettings->writeNavMetaDataFile.fileInterface.io.writeFD.write(&appendSettings->writeNavMetaDataFile.fileInterface.io.writeFD,
                                                                          &fileNode,
                                                                          sizeof(fileNode));
    B_DVR_File_Close(&readMetaDataFile);
    BDBG_MSG(("%s:appendSettings->navfileCount %lu",__FUNCTION__,appendSettings->navFileCount));
    BDBG_MSG(("%s: appendSettings->mediafileCount %lu",__FUNCTION__,appendSettings->mediaFileCount));
    return rc;
error_readNavFile:
    appendSettings->mediaFileCount=0;
    appendSettings->tmpMediaFileCount=0;
    appendSettings->navFileCount=0;
    appendSettings->tmpNavFileCount=0;
    B_DVR_File_Close(&appendSettings->readTempMediaMetaDataFile);
error_readMediaFile:
     B_DVR_File_Close(&appendSettings->writeMediaMetaDataFile);
error_writeMediaFile:
     B_DVR_File_Close(&appendSettings->writeNavMetaDataFile);
error_writeNavFile:
    B_DVR_File_Close(&appendSettings->readTempMediaMetaDataFile);
error_tmpMediaFile:
    B_DVR_File_Close(&appendSettings->readTempNavMetaDataFile);
error_tmpNavFile:
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_CloseAppendedPermMetaDataFile(
    B_DVR_SegmentedFileRecordAppendSettings *appendSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_ASSERT(appendSettings);
    appendSettings->mediaFileCount=0;
    appendSettings->tmpMediaFileCount=0;
    appendSettings->navFileCount=0;
    appendSettings->tmpNavFileCount=0;
    B_DVR_File_Close(&appendSettings->writeMediaMetaDataFile);
    B_DVR_File_Close(&appendSettings->writeNavMetaDataFile);
    B_DVR_File_Close(&appendSettings->readTempMediaMetaDataFile);
    B_DVR_File_Close(&appendSettings->readTempNavMetaDataFile);
    return rc;
}

void setTsbConversionFlag(B_DVR_SegmentedFileRecordHandle segmentedFileRecord,bool tsbConversion)
{
    BKNI_AcquireMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
    segmentedFileRecord->mediaWrite->tsbConversion = tsbConversion;
    BKNI_ReleaseMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
    BKNI_AcquireMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
    segmentedFileRecord->navWrite->tsbConversion = tsbConversion;
    BKNI_ReleaseMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
    return;
}

unsigned long B_DVR_FileSegmentedRecord_CopyMetaDataFile(
    NEXUS_FileRecordHandle nexusFileRecord,
    B_DVR_FilePosition tsbStartPosition,
    B_DVR_FilePosition tsbEndPosition)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)nexusFileRecord;
    B_DVR_PermanentRecord *permRec = &segmentedFileRecord->permRec;
    BNAV_Player_Position navPosition;
    long navIndex=0;
    B_DVR_SegmentedFileNode *recCurrentFileNode;
    bool fileAllocFailed = false;
    B_DVR_SegmentedFileNode tmpFileNode;
    BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
    if(segmentedFileRecord->navPlayer==NULL)
    {
        B_DVR_SegmentedFileRecord_P_OpenNavPlayer(segmentedFileRecord);
        if(!segmentedFileRecord->navPlayer)
        {
            rc = B_DVR_UNKNOWN;
            BDBG_ERR(("%s:no navigation data available",__FUNCTION__));
            dumpSegmentedFileRecord(segmentedFileRecord);
            BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
            goto error;
        }
    }
    BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
    if(!permRec->pCachedNavFileNode)
    {
        if(permRec->recStartTime < tsbStartPosition.timestamp)
        {
            BDBG_WRN(("permRec->recStartTime < tsbStartPosition.timestamp "
                      "startTime:%ld tsbStartPosition.timestamp %ld",
                       permRec->recStartTime,tsbStartPosition.timestamp));
            permRec->recStartTime = tsbStartPosition.timestamp;
        }
        BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
        navIndex = BNAV_Player_FindIndexFromTimestamp(segmentedFileRecord->navPlayer,permRec->recStartTime);
        if(navIndex < 0)
        {
            BDBG_ERR(("%s:unable to get nav index for time stamp %ld",__FUNCTION__,permRec->recStartTime));
            rc = B_DVR_INVALID_PARAMETER;
            BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
            goto error;
        }
        navIndex = BNAV_Player_FindIFrameFromIndex(segmentedFileRecord->navPlayer, navIndex, eBpForward);
        if(navIndex < 0)
        {
            BDBG_ERR(("%s:unable to get I Frame nav index from rec start navIndex %ld",
                      __FUNCTION__,navIndex));
            rc = B_DVR_INVALID_PARAMETER;
            BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
            goto error;
        }
        rc = BNAV_Player_GetPositionInformation(segmentedFileRecord->navPlayer, navIndex, &navPosition);
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s:unable to get position info for I Frame rec start navIndex %ld",
                      __FUNCTION__,navIndex));
            rc = B_DVR_UNKNOWN;
            BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
            goto error;
        }
        BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
        permRec->recStartTime = navPosition.timestamp;
        permRec->recNavStartOffset = navPosition.index * segmentedFileRecord->navEntrySize;
        permRec->recMediaStartOffset = ((((uint64_t)(navPosition.offsetHi))<<32)|(unsigned long)(navPosition.offsetLo));
        permRec->recMediaStartOffset -= navPosition.metadataOffset;
        if (permRec->recMediaStartOffset <= 0) 
        {
            permRec->recMediaStartOffset = 0;
        }
        else
        {
           permRec->recMediaStartOffset = B_DVR_IO_ALIGN_TRUNC(permRec->recMediaStartOffset);
        }

        permRec->recMediaCurrentOffset = permRec->recMediaStartOffset;
        permRec->recNavCurrentOffset = permRec->recNavStartOffset;
        permRec->recCurrentTime = permRec->recStartTime;
        BDBG_MSG(("%s: Conversion Start media:%s nav:%s",__FUNCTION__,permRec->mediaFileName,permRec->navFileName));
        BDBG_MSG(("tsb start time:%ld end time:%ld",
                tsbStartPosition.timestamp,tsbEndPosition.timestamp));
        BDBG_MSG(("tsb ms_off: %lld me_off:%lld",
                tsbStartPosition.mpegFileOffset,tsbEndPosition.mpegFileOffset));
        BDBG_MSG(("tsb ns_off: %lld ne_off:%lld",
                tsbStartPosition.navFileOffset,tsbEndPosition.navFileOffset));
        BDBG_MSG(("tsbConv st:%ld ct:%ld et:%ld",
                permRec->recStartTime,permRec->recCurrentTime,permRec->recEndTime));
        BDBG_MSG(("tsbConv ms_off:%lld mc_off:%lld me_off:%lld",
                permRec->recMediaStartOffset,
                permRec->recMediaCurrentOffset,
                permRec->recMediaEndOffset));
        BDBG_MSG(("tsbConv ns_off:%lld nc_off:%lld ne_off:%lld",
                permRec->recNavStartOffset,
                permRec->recNavCurrentOffset,
                permRec->recNavEndOffset));
    }

    if (permRec->recEndTime > tsbEndPosition.timestamp)
    {
       permRec->recMediaEndOffset = tsbEndPosition.mpegFileOffset;
       permRec->recNavEndOffset = tsbEndPosition.navFileOffset;
       setTsbConversionFlag(segmentedFileRecord,true);
    }
    else
    {
       BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
       navIndex = BNAV_Player_FindIndexFromTimestamp(segmentedFileRecord->navPlayer,permRec->recEndTime);
       if(navIndex < 0)
       {
           BDBG_ERR(("%s:unable to find index for time stamp %ld",
                     __FUNCTION__,permRec->recEndTime));
           rc = B_DVR_INVALID_PARAMETER;
           BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
           goto error;
       }
       rc = BNAV_Player_GetPositionInformation(segmentedFileRecord->navPlayer, navIndex, &navPosition);
       if (rc != BERR_SUCCESS)
       { 
           BDBG_ERR(("%s:unable to get position info from rec nav end Index %ld",
                     __FUNCTION__,navIndex));
           rc = B_DVR_UNKNOWN;
           BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
           goto error;
       }
       BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
       permRec->recMediaEndOffset = ((((uint64_t)(navPosition.offsetHi))<<32)|(unsigned long)(navPosition.offsetLo));;
       permRec->recNavEndOffset = navPosition.index*segmentedFileRecord->navEntrySize;
       BDBG_MSG(("tsb start time:%ld end time:%ld",
               tsbStartPosition.timestamp,tsbEndPosition.timestamp));
       BDBG_MSG(("tsb ms_off: %lld me_off:%lld",
               tsbStartPosition.mpegFileOffset,tsbEndPosition.mpegFileOffset));
       BDBG_MSG(("tsb ns_off: %lld ne_off:%lld",
               tsbStartPosition.navFileOffset,tsbEndPosition.navFileOffset));
       BDBG_MSG(("tsbConv st:%ld ct:%ld et:%ld",
               permRec->recStartTime,permRec->recCurrentTime,permRec->recEndTime));
       BDBG_MSG(("tsbConv ms_off:%lld mc_off:%lld me_off:%lld",
               permRec->recMediaStartOffset,
               permRec->recMediaCurrentOffset,
               permRec->recMediaEndOffset));
       BDBG_MSG(("tsbConv ns_off:%lld nc_off:%lld ne_off:%lld",
               permRec->recNavStartOffset,
               permRec->recNavCurrentOffset,
               permRec->recNavEndOffset));

    }

    BKNI_AcquireMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
    if(!permRec->pCachedMediaFileNode)
    {
        recCurrentFileNode= BLST_Q_FIRST(&segmentedFileRecord->mediaWrite->fileList);
        while(recCurrentFileNode!=NULL)
        {
            BDBG_OBJECT_ASSERT(recCurrentFileNode,B_DVR_SegmentedFileNode);
            if((permRec->recMediaStartOffset >= recCurrentFileNode->linearStartOffset) 
                && (permRec->recMediaStartOffset < (recCurrentFileNode->linearStartOffset 
                                                      + recCurrentFileNode->size)))
            {
                permRec->pCachedMediaFileNode=recCurrentFileNode;
                if(permRec->pCachedMediaFileNode->recordType == eB_DVR_RecordingPermanent)
                {
                    B_DVR_MediaStorage_IncreaseMediaSegmentRefCount(segmentedFileRecord->mediaWrite->mediaStorage,
                                                                    segmentedFileRecord->mediaWrite->volumeIndex,
                                                                    segmentedFileRecord->mediaWrite->tsbConvMetaDataSubDir,
                                                                    permRec->pCachedMediaFileNode->fileName);

                    permRec->pCachedMediaFileNode->refCount++;
                    BDBG_MSG_TRACE(("Back to back TSB conversion scenario"));
                }
                break;
            }  
            recCurrentFileNode = BLST_Q_NEXT(recCurrentFileNode,nextFileNode);
         }
    }
    else
    {
        recCurrentFileNode= permRec->pCachedMediaFileNode;
        BDBG_OBJECT_ASSERT(recCurrentFileNode,B_DVR_SegmentedFileNode);
    }
    
    while(recCurrentFileNode!=NULL)
    {
        BDBG_OBJECT_ASSERT(recCurrentFileNode,B_DVR_SegmentedFileNode);
        if(recCurrentFileNode->linearStartOffset > permRec->recMediaEndOffset )
        {
            break;
        }
        if(recCurrentFileNode->recordType == eB_DVR_RecordingTSB)
        {
            B_DVR_SegmentedFileNode *fileNode=NULL;
            fileNode = allocateFileSegment(segmentedFileRecord->mediaWrite->mediaStorage,
                                          segmentedFileRecord->mediaWrite->fileType,
                                          segmentedFileRecord->mediaWrite->volumeIndex,
                                          segmentedFileRecord->mediaWrite->tsbConvMetaDataSubDir);
            if(!fileNode)
            {
                BDBG_ERR(("%s:TSB conversion failed",__FUNCTION__));
                fileAllocFailed = true;
                if(segmentedFileRecord->mediaWrite->registeredCallback)
                { 
                    segmentedFileRecord->mediaWrite->registeredCallback((void *)&segmentedFileRecord->mediaWrite,B_DVR_SegmentedFileEventNoFileSpaceError);
                }
                BDBG_ERR(("%s:TSBConv media: allocateFileSegment failed",__FUNCTION__));
                break;
            }
            fileNode->recordType = eB_DVR_RecordingTSB;
            BLST_Q_INSERT_TAIL(&segmentedFileRecord->mediaWrite->tsbFreeFileList,fileNode,nextFileNode);
            recCurrentFileNode->recordType = eB_DVR_RecordingPermanent;
            fileNode->index = recCurrentFileNode->index;
            writeMetaDataFile(&segmentedFileRecord->mediaWrite->dvrFileMetaData,fileNode);
            BDBG_MSG_TRACE(("TSB Media Queue node to be replaced:index:%u startOff:%lld size:%u segOff:%lld permRec:%d refCnt:%u name:%s",
                      recCurrentFileNode->index,
                      recCurrentFileNode->linearStartOffset,
                      recCurrentFileNode->size,
                      recCurrentFileNode->segmentOffset,
                      recCurrentFileNode->recordType,
                      recCurrentFileNode->refCount,
                      recCurrentFileNode->fileName));
            BDBG_MSG_TRACE(("TSB Media Queue node replacement:index: %u startOff:%lld size:%u segOff:%lld permRec:%d refCnt:%u name:%s",
                      fileNode->index,
                      fileNode->linearStartOffset,
                      fileNode->size,
                      fileNode->segmentOffset,
                      fileNode->recordType,
                      fileNode->refCount,
                      fileNode->fileName));
           B_DVR_MediaStorage_IncreaseMediaSegmentRefCount(segmentedFileRecord->mediaWrite->mediaStorage,
                                                            segmentedFileRecord->mediaWrite->volumeIndex,
                                                            segmentedFileRecord->mediaWrite->tsbConvMetaDataSubDir,
                                                            recCurrentFileNode->fileName);
            BDBG_MSG_TRACE(("refCnt for %s increased",recCurrentFileNode->fileName));
            recCurrentFileNode->refCount++;
        }
            
        if(recCurrentFileNode!=permRec->pCachedMediaFileNode)
        {
           permRec->mediaFileCount++;
           BDBG_MSG_TRACE(("current perm file node:index: %u startOff:%lld size:%u segOff:%lld permRec:%d refCnt:%u name:%s",
                      recCurrentFileNode->index,
                      recCurrentFileNode->linearStartOffset,
                      recCurrentFileNode->size,
                      recCurrentFileNode->segmentOffset,
                      recCurrentFileNode->recordType,
                      recCurrentFileNode->refCount,
                      recCurrentFileNode->fileName));
           BDBG_MSG_TRACE(("prev perm file node:index: %u startOff:%lld size:%u segOff:%lld permRec:%d refCnt:%u name:%s",
                      permRec->pCachedMediaFileNode->index,
                      permRec->pCachedMediaFileNode->linearStartOffset,
                      permRec->pCachedMediaFileNode->size,
                      permRec->pCachedMediaFileNode->segmentOffset,
                      permRec->pCachedMediaFileNode->recordType,
                      permRec->pCachedMediaFileNode->refCount,
                      permRec->pCachedMediaFileNode->fileName));
        }
        segmented_posix_seekForWrite(&permRec->mediaMetaDataFile.fileInterface.io.writeFD, sizeof(*recCurrentFileNode)*permRec->mediaFileCount, SEEK_SET);
        BKNI_Memcpy((void *)&tmpFileNode,recCurrentFileNode,sizeof(tmpFileNode));
        tmpFileNode.index = permRec->mediaFileCount;
        if(permRec->recMediaStartOffset > recCurrentFileNode->linearStartOffset && 
           (permRec->recMediaStartOffset < recCurrentFileNode->linearStartOffset + recCurrentFileNode->size))
        {
            tmpFileNode.segmentOffset = permRec->recMediaStartOffset - tmpFileNode.linearStartOffset;
            tmpFileNode.linearStartOffset = permRec->recMediaStartOffset;
            tmpFileNode.size -= tmpFileNode.segmentOffset;
        }
        permRec->mediaMetaDataFile.fileInterface.io.writeFD.write(&permRec->mediaMetaDataFile.fileInterface.io.writeFD,
                                                                  (void *)&tmpFileNode,sizeof(tmpFileNode));
        permRec->pCachedMediaFileNode = recCurrentFileNode;
        recCurrentFileNode = BLST_Q_NEXT(recCurrentFileNode,nextFileNode);
    }
 
    BKNI_ReleaseMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);

    if(fileAllocFailed)
    {
        BDBG_ERR(("%s:tsb->rec conversion failed because of no storage space in media partition",__FUNCTION__));
        setTsbConversionFlag(segmentedFileRecord,false);
        goto error;
    }
                                                                                                 
    BKNI_AcquireMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
    if(!permRec->pCachedNavFileNode)
    {
        recCurrentFileNode = BLST_Q_FIRST(&segmentedFileRecord->navWrite->fileList);
        while(recCurrentFileNode!=NULL)
        {
            BDBG_OBJECT_ASSERT(recCurrentFileNode,B_DVR_SegmentedFileNode);
            if((permRec->recNavStartOffset >= recCurrentFileNode->linearStartOffset) 
              && (permRec->recNavStartOffset < (recCurrentFileNode->linearStartOffset 
                                                + recCurrentFileNode->size)))
            {
                permRec->pCachedNavFileNode=recCurrentFileNode;
                if(permRec->pCachedNavFileNode->recordType == eB_DVR_RecordingPermanent)
                {
                    B_DVR_MediaStorage_IncreaseNavSegmentRefCount(segmentedFileRecord->navWrite->mediaStorage,
                                                                  segmentedFileRecord->navWrite->volumeIndex,
                                                                  permRec->pCachedNavFileNode->fileName);
                    permRec->pCachedNavFileNode->refCount++;
                    BDBG_MSG_TRACE(("Back to back TSB conversion scenario"));
                }
                break;
            } 
            recCurrentFileNode = BLST_Q_NEXT(recCurrentFileNode,nextFileNode);
        }
    }
    else
    {
        recCurrentFileNode = permRec->pCachedNavFileNode;
        BDBG_OBJECT_ASSERT(recCurrentFileNode,B_DVR_SegmentedFileNode);
    }
            
    while(recCurrentFileNode!=NULL)
    {   
        BDBG_OBJECT_ASSERT(recCurrentFileNode,B_DVR_SegmentedFileNode);
        if(recCurrentFileNode->linearStartOffset > permRec->recNavEndOffset)
        {
            break;
        }
        
        if(recCurrentFileNode->recordType == eB_DVR_RecordingTSB)
        {
            B_DVR_SegmentedFileNode *fileNode=NULL;
            fileNode = allocateFileSegment(segmentedFileRecord->navWrite->mediaStorage,
                                           segmentedFileRecord->navWrite->fileType,
                                           segmentedFileRecord->navWrite->volumeIndex,
                                           segmentedFileRecord->navWrite->tsbConvMetaDataSubDir);
            if(!fileNode)
            {
                BDBG_ERR(("%s:TSB conversion failed",__FUNCTION__));
                fileAllocFailed = true;
                if(segmentedFileRecord->navWrite->registeredCallback)
                {
                    segmentedFileRecord->navWrite->registeredCallback((void *)&segmentedFileRecord->navWrite,B_DVR_SegmentedFileEventNoFileSpaceError);
                }
                BDBG_ERR(("%s:TSBConv nav: allocateFileSegment failed",__FUNCTION__));
                break;
            }
            fileNode->recordType = eB_DVR_RecordingTSB;
            BLST_Q_INSERT_TAIL(&segmentedFileRecord->navWrite->tsbFreeFileList,fileNode,nextFileNode);
            recCurrentFileNode->recordType = eB_DVR_RecordingPermanent;
            fileNode->index = recCurrentFileNode->index;
            writeMetaDataFile(&segmentedFileRecord->navWrite->dvrFileMetaData,fileNode);
            BDBG_MSG_TRACE(("TSB Nav Queue node to be replaced:index:%u startOff:%lld size:%u segOff:%lld permRec:%d refCnt:%u name:%s",
                      recCurrentFileNode->index,
                      recCurrentFileNode->linearStartOffset,
                      recCurrentFileNode->size,
                      recCurrentFileNode->segmentOffset,
                      recCurrentFileNode->recordType,
                      recCurrentFileNode->refCount,
                      recCurrentFileNode->fileName));
            BDBG_MSG_TRACE(("TSB Nav Queue node replacement:index: %u startOff:%lld size:%u segOff:%lld permRec:%d refCnt:%u name:%s",
                      fileNode->index,
                      fileNode->linearStartOffset,
                      fileNode->size,
                      fileNode->segmentOffset,
                      fileNode->recordType,
                      fileNode->refCount,
                      fileNode->fileName));
            B_DVR_MediaStorage_IncreaseNavSegmentRefCount(segmentedFileRecord->navWrite->mediaStorage,
                                                          segmentedFileRecord->navWrite->volumeIndex,
                                                          recCurrentFileNode->fileName);
            recCurrentFileNode->refCount++;
            BDBG_MSG_TRACE(("refCnt for %s increased",recCurrentFileNode->fileName));
        }
                
        if(recCurrentFileNode!=permRec->pCachedNavFileNode)
        {
            permRec->navFileCount++;
            BDBG_MSG_TRACE(("current perm file node:index: %u startOff:%lld size:%u segOff:%lld permRec:%d refCnt:%u name:%s",
                      recCurrentFileNode->index,
                      recCurrentFileNode->linearStartOffset,
                      recCurrentFileNode->size,
                      recCurrentFileNode->segmentOffset,
                      recCurrentFileNode->recordType,
                      recCurrentFileNode->refCount,
                      recCurrentFileNode->fileName));
           BDBG_MSG_TRACE(("prev perm file node:index: %u startOff:%lld size:%u segOff:%lld permRec:%d refCnt:%u name:%s",
                      permRec->pCachedMediaFileNode->index,
                      permRec->pCachedMediaFileNode->linearStartOffset,
                      permRec->pCachedMediaFileNode->size,
                      permRec->pCachedMediaFileNode->segmentOffset,
                      permRec->pCachedMediaFileNode->recordType,
                      permRec->pCachedMediaFileNode->refCount,
                      permRec->pCachedMediaFileNode->fileName));
        }
        segmented_posix_seekForWrite(&permRec->navMetaDataFile.fileInterface.io.writeFD, sizeof(*recCurrentFileNode)*permRec->navFileCount, SEEK_SET);
        BKNI_Memcpy((void*)&tmpFileNode,(void *)recCurrentFileNode,sizeof(tmpFileNode));
        tmpFileNode.index = permRec->navFileCount;
        if(permRec->recNavStartOffset > recCurrentFileNode->linearStartOffset && 
          (permRec->recNavStartOffset < recCurrentFileNode->linearStartOffset + recCurrentFileNode->size))
        {
            tmpFileNode.segmentOffset = permRec->recNavStartOffset - tmpFileNode.linearStartOffset;
            tmpFileNode.linearStartOffset = permRec->recNavStartOffset;
            tmpFileNode.size -= tmpFileNode.segmentOffset;
        }
        permRec->navMetaDataFile.fileInterface.io.writeFD.write(&permRec->navMetaDataFile.fileInterface.io.writeFD,
                                                               (void *)&tmpFileNode,sizeof(tmpFileNode));
        permRec->pCachedNavFileNode = recCurrentFileNode;
        recCurrentFileNode = BLST_Q_NEXT(recCurrentFileNode,nextFileNode);
    }
    BKNI_ReleaseMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
    if(fileAllocFailed)
    {
       BDBG_ERR(("%s: tsb->rec conversion failed because of no storage space in nav partition",__FUNCTION__));
       setTsbConversionFlag(segmentedFileRecord,false);
       goto error;
    }

    permRec->recCurrentTime = tsbEndPosition.timestamp;
    permRec->recNavCurrentOffset = permRec->recNavEndOffset;
    permRec->recMediaCurrentOffset = permRec->recMediaEndOffset;
    if(permRec->recEndTime <= tsbEndPosition.timestamp)
    {
        BDBG_MSG(("%s: Conversion End  media:%s nav:%s",__FUNCTION__,permRec->mediaFileName,permRec->navFileName));
        setTsbConversionFlag(segmentedFileRecord,false);
    }
    return permRec->recCurrentTime;
error:
    return -1;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_UpdateAppendedMediaMetaDataFile(B_DVR_SegmentedFileRecordAppendSettings *appendSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_SegmentedFileNode fileNode;
    ssize_t returnSize=0;
    bool fileEntriesAvailable = true;
    struct stat sb;
    unsigned long fileSize=0;
    unsigned long tmpMediaFileCount=0;

    while(fileEntriesAvailable)
    {
        appendSettings->readTempMediaMetaDataFile.fileInterface.io.readFD.seek(&appendSettings->readTempMediaMetaDataFile.fileInterface.io.readFD,
                                                                               appendSettings->tmpMediaFileCount*sizeof(fileNode),
                                                                               SEEK_SET);
        returnSize = appendSettings->readTempMediaMetaDataFile.fileInterface.io.readFD.read(&appendSettings->readTempMediaMetaDataFile.fileInterface.io.readFD,
                                                                                            &fileNode,
                                                                                            sizeof(fileNode));
        if(returnSize!=sizeof(fileNode))
        {
            fileEntriesAvailable=false;
        }
        if(fileEntriesAvailable)
        {
            fileNode.linearStartOffset -= appendSettings->tsbConvMediaStartOffset;
            fileNode.linearStartOffset += appendSettings->appendMediaOffset;
            segmented_posix_seekForWrite(&appendSettings->writeMediaMetaDataFile.fileInterface.io.writeFD,
                                         appendSettings->mediaFileCount*sizeof(fileNode),SEEK_SET);
            returnSize = appendSettings->writeMediaMetaDataFile.fileInterface.io.writeFD.write(&appendSettings->writeMediaMetaDataFile.fileInterface.io.writeFD,
                                                                                               &fileNode,
                                                                                               sizeof(fileNode));
            if(returnSize!=sizeof(fileNode))
            {
                BDBG_ERR(("%s:write error in %s",appendSettings->mediaMetaDataFileName,__FUNCTION__));
                rc = B_DVR_OS_ERROR;
                break;
            }
            BKNI_Memcpy((void *)&appendSettings->lastMediaFileNode,(void *)&fileNode,sizeof(fileNode));  
            fstat(appendSettings->readTempMediaMetaDataFile.fd, &sb);
            fileSize=sb.st_size;
            tmpMediaFileCount =fileSize/sizeof(B_DVR_SegmentedFileNode);
            if((tmpMediaFileCount-1) > appendSettings->tmpMediaFileCount)
            {
                appendSettings->mediaFileCount++;
                appendSettings->tmpMediaFileCount++;
            }
            else
            {
                fileEntriesAvailable = false;
            }
        }
    }
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_UpdateAppendedNavMetaDataFile(B_DVR_SegmentedFileRecordAppendSettings *appendSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_SegmentedFileNode fileNode;
    ssize_t returnSize=0;
    bool fileEntriesAvailable = true;
    struct stat sb;
    unsigned long fileSize=0;
    unsigned long tmpNavFileCount=0;
    B_DVR_FileSettings fileSettings;
    char navFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    char path[B_DVR_MAX_FILE_NAME_LENGTH];
    char fullNavFileName[B_DVR_MAX_FILE_NAME_LENGTH];

    while(fileEntriesAvailable)
    {
        appendSettings->readTempNavMetaDataFile.fileInterface.io.readFD.seek(&appendSettings->readTempNavMetaDataFile.fileInterface.io.readFD,
                                                                             appendSettings->tmpNavFileCount*sizeof(fileNode),
                                                                             SEEK_SET);
        returnSize = appendSettings->readTempNavMetaDataFile.fileInterface.io.readFD.read(&appendSettings->readTempNavMetaDataFile.fileInterface.io.readFD,
                                                                                          &fileNode,
                                                                                          sizeof(fileNode));
        if(returnSize!=sizeof(fileNode))
        {
            fileEntriesAvailable=false;
        }
        if(fileEntriesAvailable)
        {
            if(!appendSettings->navFileOffset)
            {
                if(appendSettings->tmpNavFileCount)
                { 
                    B_DVR_File_Close(&appendSettings->readNavFile);
                    B_DVR_File_Close(&appendSettings->writeNavFile);
                }

                B_DVR_MediaStorage_GetNavPath(appendSettings->mediaStorage,
                                              appendSettings->volumeIndex,
                                              path);

                rc = B_DVR_MediaStorage_AllocateNavSegment(appendSettings->mediaStorage,
                                                           appendSettings->volumeIndex,
                                                           navFileName);
                if(rc!=B_DVR_SUCCESS)
                {
                    BDBG_ERR(("%s:error in allocating a nav segment to replace %s",__FUNCTION__,fileNode.fileName));
                    rc = B_DVR_OS_ERROR;
                    break;
                }
                strncpy(appendSettings->lastNavFileNode.fileName,navFileName,B_DVR_MAX_FILE_NAME_LENGTH);
                appendSettings->lastMediaFileNode.fileName[B_DVR_MAX_FILE_NAME_LENGTH-1]='\0';
                BKNI_Snprintf(fullNavFileName,sizeof(fullNavFileName),"%s/%s", path,navFileName);
                fileSettings.create = false;
                fileSettings.directIO = false;
                fileSettings.fileIO = eB_DVR_FileIOWrite;
                rc = B_DVR_File_Open(&appendSettings->writeNavFile,fullNavFileName,&fileSettings);
                if(rc!=B_DVR_SUCCESS)
                {
                    BDBG_ERR(("%s:unable to open %s in write mode",__FUNCTION__,fullNavFileName));
                    rc = B_DVR_OS_ERROR;
                    break;
                }
                BKNI_Snprintf(fullNavFileName,sizeof(fullNavFileName),"%s/%s", path,fileNode.fileName);
                fileSettings.create = false;
                fileSettings.directIO = false;
                fileSettings.fileIO = eB_DVR_FileIORead;
                rc = B_DVR_File_Open(&appendSettings->readNavFile,fullNavFileName,&fileSettings);
                if(rc!=B_DVR_SUCCESS)
                {
                    BDBG_ERR(("%s:unable to open %s in write mode",__FUNCTION__,fullNavFileName));
                    rc = B_DVR_OS_ERROR;
                    break;
                }
            }
            fileNode.linearStartOffset -= appendSettings->tsbConvNavStartOffset;
            fileNode.linearStartOffset += appendSettings->appendNavOffset;
            strncpy(fileNode.fileName,appendSettings->lastNavFileNode.fileName,B_DVR_MAX_FILE_NAME_LENGTH);
            fileNode.fileName[B_DVR_MAX_FILE_NAME_LENGTH-1] = '\0';
            segmented_posix_seekForWrite(&appendSettings->writeNavMetaDataFile.fileInterface.io.writeFD,
                                         appendSettings->navFileCount*sizeof(fileNode),
                                         SEEK_SET);
            returnSize = appendSettings->writeNavMetaDataFile.fileInterface.io.writeFD.write(&appendSettings->writeNavMetaDataFile.fileInterface.io.writeFD,
                                                                                             &fileNode,
                                                                                             sizeof(fileNode));
            if(returnSize!=sizeof(fileNode))
            {
                BDBG_ERR(("%s: Error in writing to %s",__FUNCTION__,appendSettings->navMetaDataFileName));
                break;
            }
            
            BKNI_Memcpy((void *)&appendSettings->lastNavFileNode,(void *)&fileNode,sizeof(fileNode));
            B_DVR_SegmentedFileRecord_UpdateAppendedNavFile(appendSettings);
            fstat(appendSettings->readTempNavMetaDataFile.fd, &sb);
            fileSize=sb.st_size;
            tmpNavFileCount =fileSize/sizeof(B_DVR_SegmentedFileNode);
            if(tmpNavFileCount-1 > appendSettings->tmpNavFileCount)
            {
                appendSettings->navFileCount++;
                appendSettings->tmpNavFileCount++;
                appendSettings->navFileOffset =0;
            }
            else
            {
                fileEntriesAvailable = false;

            }
        }
    }
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_UpdateAppendedNavFile(B_DVR_SegmentedFileRecordAppendSettings *appendSettings)
{
    ssize_t returnSize=0;
    bool navEntriesAvailable = true;
    BNAV_Entry mpeg2NavEntry;
    BNAV_AVC_Entry avcNavEntry;
    off_t mediaLinearFileOffset;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned long timeStamp=0;

    while(navEntriesAvailable)
    {
        if(sizeof(mpeg2NavEntry) == appendSettings->navEntrySize)
        { 
            appendSettings->readNavFile.fileInterface.io.readFD.seek(&appendSettings->readNavFile.fileInterface.io.readFD,
                                                                     appendSettings->navFileOffset + appendSettings->lastNavFileNode.segmentOffset,
                                                                     SEEK_SET);
            returnSize = appendSettings->readNavFile.fileInterface.io.readFD.read(&appendSettings->readNavFile.fileInterface.io.readFD,
                                                                                  &mpeg2NavEntry,
                                                                                  sizeof(mpeg2NavEntry));
            if(returnSize!=sizeof(mpeg2NavEntry))
            {
                navEntriesAvailable=false;
                break;
            }
            mediaLinearFileOffset = (BNAV_get_frameOffsetLo(&mpeg2NavEntry)|((off_t)(BNAV_get_frameOffsetHi(&mpeg2NavEntry ))<<32));

            mediaLinearFileOffset -= appendSettings->tsbConvMediaStartOffset;
            mediaLinearFileOffset += appendSettings->appendMediaOffset;

            BNAV_set_frameOffsetLo(&mpeg2NavEntry,(mediaLinearFileOffset & 0xffffffff));
            BNAV_set_frameOffsetHi(&mpeg2NavEntry,((mediaLinearFileOffset & 0xffffffff00000000)>>32));
            timeStamp = BNAV_get_timestamp(&mpeg2NavEntry);

            timeStamp -= appendSettings->tsbConvStartTime;
            timeStamp += appendSettings->appendTimeStamp;

            BNAV_set_timestamp(&mpeg2NavEntry, timeStamp);
            segmented_posix_seekForWrite(&appendSettings->writeNavFile.fileInterface.io.writeFD,
                                         appendSettings->navFileOffset+appendSettings->lastNavFileNode.segmentOffset,
                                         SEEK_SET);
            returnSize = appendSettings->writeNavFile.fileInterface.io.writeFD.write(&appendSettings->writeNavFile.fileInterface.io.writeFD,
                                                                                     &mpeg2NavEntry,
                                                                                     sizeof(mpeg2NavEntry));
            if(returnSize!=sizeof(mpeg2NavEntry))
            {
                BDBG_ERR(("%s:write error in %s",__FUNCTION__,appendSettings->writeNavFile.fileName));
                rc = B_DVR_OS_ERROR;
                break;
            }
            appendSettings->navFileOffset += sizeof(mpeg2NavEntry);
        }
        else
        {
            appendSettings->readNavFile.fileInterface.io.readFD.seek(&appendSettings->readNavFile.fileInterface.io.readFD,
                                                                     appendSettings->navFileOffset + appendSettings->lastNavFileNode.segmentOffset,
                                                                     SEEK_SET);
             returnSize = appendSettings->readNavFile.fileInterface.io.readFD.read(&appendSettings->readNavFile.fileInterface.io.readFD,
                                                                                   &avcNavEntry,
                                                                                   sizeof(avcNavEntry));
            if(returnSize!=sizeof(avcNavEntry))
             {
                 navEntriesAvailable=false;
             }
             mediaLinearFileOffset = (BNAV_get_frameOffsetLo(&avcNavEntry)|((off_t)(BNAV_get_frameOffsetHi(&avcNavEntry ))<<32));

             mediaLinearFileOffset -= appendSettings->tsbConvMediaStartOffset;
             mediaLinearFileOffset += appendSettings->appendMediaOffset;


             BNAV_set_frameOffsetLo(&avcNavEntry,(mediaLinearFileOffset & 0xffffffff));
             BNAV_set_frameOffsetHi(&avcNavEntry,(mediaLinearFileOffset & 0xffffffff00000000)>>32);
             timeStamp = BNAV_get_timestamp(&avcNavEntry);

             timeStamp -= appendSettings->tsbConvStartTime;
             timeStamp += appendSettings->appendTimeStamp;


             BNAV_set_timestamp(&avcNavEntry, timeStamp);
             segmented_posix_seekForWrite(&appendSettings->writeNavFile.fileInterface.io.writeFD,
                                          appendSettings->navFileOffset+appendSettings->lastNavFileNode.segmentOffset,
                                          SEEK_SET);
             returnSize = appendSettings->writeNavFile.fileInterface.io.writeFD.write(&appendSettings->writeNavFile.fileInterface.io.writeFD,
                                                                                      &avcNavEntry,
                                                                                      sizeof(avcNavEntry));
             if(returnSize!=sizeof(avcNavEntry))
             {
                 BDBG_MSG(("%s:error writing to %s",__FUNCTION__,appendSettings->writeNavFile.fileName));
                 break;
             }
             appendSettings->navFileOffset += sizeof(avcNavEntry);
         }
    }
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_GetBounds(
    NEXUS_FileRecordHandle nexusFileRecord, 
    B_DVR_FilePosition *pFirst, 
    B_DVR_FilePosition *pLast)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    long firstIndex,lastIndex;
    BNAV_Player_Position pos;
    unsigned long retry_bounds=0;
    bool tsbService=false;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)nexusFileRecord;

    BDBG_ASSERT(segmentedFileRecord);
    BDBG_ASSERT(pFirst);
    BDBG_ASSERT(pLast);

    BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
    if(segmentedFileRecord->navPlayer==NULL)
    {
        B_DVR_SegmentedFileRecord_P_OpenNavPlayer(segmentedFileRecord);
        if(!segmentedFileRecord->navPlayer)
        {
            rc = B_DVR_UNKNOWN;
            BDBG_ERR(("%s:no navigation data available",__FUNCTION__));
            dumpSegmentedFileRecord(segmentedFileRecord);
            if(pFirst)
            {
                pFirst->mpegFileOffset = 0;
                pFirst->navFileOffset = 0;
                pFirst->timestamp = 0;
            }
            if(pLast)
            { /* fill them with zeros */
                pLast->mpegFileOffset = 0;
                pLast->navFileOffset = 0;
                pLast->timestamp = B_DVR_FILE_INVALID_POSITION;
            }
            BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
            return rc;
        }
    }
    BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
retry_getBounds:
    BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
    rc = B_DVR_SegmentedFile_P_GetBounds(segmentedFileRecord->navPlayer,segmentedFileRecord,&firstIndex,&lastIndex);
    BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
    if (rc!=BERR_SUCCESS)
    {
        BDBG_ERR(("%s:error from segmentedPlay_nav_bounds",__FUNCTION__));
        rc = B_DVR_UNKNOWN;
        goto error;
    }
    BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
    rc = BNAV_Player_GetPositionInformation(segmentedFileRecord->navPlayer, lastIndex, &pos);
    BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
    if (rc!=BERR_SUCCESS)
    {
        if(!retry_bounds) 
        {
            BDBG_MSG(("%s:retry getBounds because last index's(%ld)offset isn't found",__FUNCTION__,lastIndex));
            retry_bounds++;
            goto retry_getBounds;
        }
        else
        { 
            BDBG_ERR(("%s:unable to get position info for last nav index %ld",__FUNCTION__,lastIndex));
        rc = B_DVR_UNKNOWN;
        goto error;
    }
    }

    pLast->timestamp = pos.timestamp;
    pLast->mpegFileOffset = (pos.offsetLo|(((off_t)pos.offsetHi)<<32));
    pLast->navFileOffset = pos.index*segmentedFileRecord->navEntrySize;
    pLast->index = pos.index;

    BKNI_AcquireMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
    tsbService = segmentedFileRecord->mediaWrite->service == eB_DVR_ServiceTSB?true:false;
    BKNI_ReleaseMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
    
    if(tsbService && pLast->timestamp > segmentedFileRecord->maxTSBTime) 
    {
        unsigned long tsbPlayStartTime=0;
        long tsbPlayStartIndex=0;

        tsbPlayStartTime = pLast->timestamp - segmentedFileRecord->maxTSBTime;
        BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
        tsbPlayStartIndex = BNAV_Player_FindIndexFromTimestamp(segmentedFileRecord->navPlayer,
                                                               tsbPlayStartTime);
        BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
        if((tsbPlayStartIndex>0) && (tsbPlayStartIndex > firstIndex))
        {
            firstIndex= tsbPlayStartIndex;
        }
    }
    BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
    rc = BNAV_Player_GetPositionInformation(segmentedFileRecord->navPlayer,firstIndex,&pos);
    BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
    if (rc!=BERR_SUCCESS)
    {
        if(!retry_bounds) 
        {
            BDBG_MSG(("%s:retry getBounds because first index's(%ld)offset isn't found",__FUNCTION__,firstIndex));
            retry_bounds++;
            goto retry_getBounds;
        }
        else
        {
            BDBG_ERR(("%s: unable to get position info for first nav index %ld",__FUNCTION__,firstIndex));
            rc = B_DVR_UNKNOWN;
            goto error;
        }
    }
    pFirst->timestamp = pos.timestamp;
    pFirst->mpegFileOffset = (pos.offsetLo|(((off_t)pos.offsetHi)<<32));
    pFirst->mpegFileOffset = B_DVR_IO_ALIGN_TRUNC(pFirst->mpegFileOffset);
    pFirst->navFileOffset = pos.index*segmentedFileRecord->navEntrySize;
    pFirst->index = pos.index;

    if(tsbService && pLast->timestamp > segmentedFileRecord->maxTSBTime) 
    {
        BKNI_AcquireMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
        segmentedFileRecord->navWrite->linearStartOffset=pFirst->navFileOffset;
        BKNI_ReleaseMutex(segmentedFileRecord->navWrite->metaDataFileMutex);
        BKNI_AcquireMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
        segmentedFileRecord->mediaWrite->linearStartOffset=pFirst->mpegFileOffset;
        BKNI_ReleaseMutex(segmentedFileRecord->mediaWrite->metaDataFileMutex);
    }
    while((pLast->mpegFileOffset > segmentedFileRecord->mediaWrite->linearCurrentOffset) 
          && (lastIndex > (long)pFirst->index)) 
    {
        BDBG_MSG_TRACE(("%s:mismatch %lld:%lld",__FUNCTION__,pLast->mpegFileOffset,segmentedFileRecord->mediaWrite->linearCurrentOffset));
        BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
        rc = BNAV_Player_GetPositionInformation(segmentedFileRecord->navPlayer, --lastIndex, &pos);
        BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
        if (rc!=BERR_SUCCESS)
        {
            BDBG_ERR(("%s:unable to get position info for last nav index %ld",__FUNCTION__,lastIndex));
            rc = B_DVR_UNKNOWN;
            goto error;
        }
        pLast->timestamp = pos.timestamp;
        pLast->mpegFileOffset = (pos.offsetLo|(((off_t)pos.offsetHi)<<32));
        pLast->navFileOffset = pos.index*segmentedFileRecord->navEntrySize;
        pLast->index = pos.index;
    }
error:
   return rc;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_GetLocation(
    NEXUS_FileRecordHandle nexusFileRecord, 
    long index,
    unsigned long timestamp, 
    B_DVR_FilePosition *pPosition)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BNAV_Player_Position pos;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)nexusFileRecord;

    BDBG_ASSERT((nexusFileRecord));
    BDBG_ASSERT((pPosition));
    BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
    if(segmentedFileRecord->navPlayer==NULL)
    {
        B_DVR_SegmentedFileRecord_P_OpenNavPlayer(segmentedFileRecord);
        if(!segmentedFileRecord->navPlayer)
        {
            rc = B_DVR_UNKNOWN;
            BDBG_ERR(("%s:no navigation data available",__FUNCTION__));
            dumpSegmentedFileRecord(segmentedFileRecord);
            if(pPosition)
            {
                pPosition->navFileOffset = 0;
                pPosition->mpegFileOffset = 0;
                pPosition->timestamp = 0;
            }
            BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
            goto error;
        }
    }
    if(index<0)
    {
        index = BNAV_Player_FindIndexFromTimestamp(segmentedFileRecord->navPlayer,timestamp);
    }
    if (index == -1)
    {
        rc = B_DVR_UNKNOWN;
        BDBG_ERR(("%s: unable to get nav index for timestamp %lu",__FUNCTION__,timestamp));
        dumpSegmentedFileRecord(segmentedFileRecord);
        BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
        goto error;
    }

    rc = BNAV_Player_GetPositionInformation(segmentedFileRecord->navPlayer, index, &pos);
    if (rc!=0) 
    { 
       rc = B_DVR_UNKNOWN; 
       BDBG_ERR(("%s: unable to get position info at nav index %ld",__FUNCTION__,index));
       dumpSegmentedFileRecord(segmentedFileRecord);
       BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
       goto error;
    }
    BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
    pPosition->timestamp = pos.timestamp;
    pPosition->mpegFileOffset = (pos.offsetLo|(((off_t)pos.offsetHi)<<32));
    pPosition->navFileOffset = pos.index*segmentedFileRecord->navEntrySize;
    pPosition->index = pos.index;
error:
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFilePlay_GetBounds(
    NEXUS_FilePlayHandle nexusFilePlay, 
    B_DVR_FilePosition *pFirst, 
    B_DVR_FilePosition *pLast)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    long firstIndex,lastIndex;
    BNAV_Player_Position pos;
    off_t first, last;
    B_DVR_SegmentedFileNode *fileNode=NULL;
    B_DVR_SegmentedFilePlayHandle segmentedFilePlay = (B_DVR_SegmentedFilePlayHandle)nexusFilePlay;
    BDBG_ASSERT(segmentedFilePlay);
    BDBG_ASSERT(pFirst);
    BDBG_ASSERT(pLast);

    BKNI_AcquireMutex(segmentedFilePlay->navPlayerMutex);
    if(segmentedFilePlay->navPlayer==NULL)
    {
        B_DVR_SegmentedFilePlay_P_OpenNavPlayer(segmentedFilePlay);
        if(!segmentedFilePlay->navPlayer)
        {
            BDBG_WRN(("%s:index file is empty, position can't be extracted",__FUNCTION__));
            if(pFirst)
            {
                pFirst->mpegFileOffset = 0;
                pFirst->navFileOffset = 0;
                pFirst->timestamp = 0;
            }
            if(pLast)
            { /* fill them with zeros */
                pLast->mpegFileOffset = 0;
                pLast->navFileOffset = 0;
                pLast->timestamp = B_DVR_FILE_INVALID_POSITION;
            }
            BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
            return rc;
        }
    }
    BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);

    fileNode = BLST_Q_LAST(&segmentedFilePlay->navRead->fileList);
    if(!fileNode) 
    {
        BDBG_ERR(("%s: last Node null in %s",__FUNCTION__,segmentedFilePlay->navRead->fileName));
        rc = B_DVR_UNKNOWN;
        goto error;
    }
    else
    {
        segmentedFilePlay->navRead->linearEndOffset = fileNode->linearStartOffset + fileNode->size;
    }

    fileNode = BLST_Q_LAST(&segmentedFilePlay->mediaRead->fileList);
    if(!fileNode) 
    {
        BDBG_ERR(("%s: last Node null in %s",__FUNCTION__,segmentedFilePlay->mediaRead->fileName));
        rc = B_DVR_UNKNOWN;
        goto error;
    }
    else
    {
        segmentedFilePlay->mediaRead->linearEndOffset = fileNode->linearStartOffset + fileNode->size;
    }

    segmentedFilePlay->navRead->fileInterface.io.readFD.bounds(&segmentedFilePlay->navRead->fileInterface.io.readFD,
                                                               &first, &last);
    if (segmentedFilePlay->navEntrySize == 0)
    {
        BDBG_ERR(("%s:error from segmentedPlay_nav_bounds",__FUNCTION__));
        rc = B_DVR_UNKNOWN;
        goto error;
    }
    firstIndex = first/segmentedFilePlay->navEntrySize;
    lastIndex = (last-1)/segmentedFilePlay->navEntrySize;
    if (firstIndex < 0 || lastIndex < 0)
    {
        BDBG_ERR(("%s:error from segmentedPlay_nav_bounds",__FUNCTION__));
        rc = B_DVR_UNKNOWN;
        goto error;
    }
    do{
        BDBG_MSG_TRACE(("%s last Index %ld",__FUNCTION__,lastIndex));
        BKNI_AcquireMutex(segmentedFilePlay->navPlayerMutex);
        rc = BNAV_Player_GetPositionInformation(segmentedFilePlay->navPlayer, lastIndex, &pos);
        BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
        if (rc!=BERR_SUCCESS)
        {
            BDBG_ERR(("%s:unable to get position info for nav index %ld",__FUNCTION__,lastIndex));
            rc = B_DVR_UNKNOWN;
            break;
        }
        pLast->timestamp = pos.timestamp;
        pLast->mpegFileOffset = (pos.offsetLo|(((off_t)pos.offsetHi)<<32));
        pLast->navFileOffset = pos.index*segmentedFilePlay->navEntrySize;
        BDBG_MSG_TRACE(("mpeg Offset from last Index %ld is %lld:%u",lastIndex,pLast->mpegFileOffset,pLast->timestamp));
        BDBG_MSG_TRACE(("mediaRead->linearEndOffset:%lld",segmentedFilePlay->mediaRead->linearEndOffset));
        lastIndex--;
        if(lastIndex <0) 
        {
            BDBG_ERR(("%s:unable to sync nav entries with media offset",__FUNCTION__));
            rc = B_DVR_UNKNOWN;
            break;
        }
    }while(pLast->mpegFileOffset>segmentedFilePlay->mediaRead->linearEndOffset || !pLast->timestamp);

    if (rc!=BERR_SUCCESS) 
    {
        goto error;
    }

    BKNI_AcquireMutex(segmentedFilePlay->navPlayerMutex);
    rc = BNAV_Player_GetPositionInformation(segmentedFilePlay->navPlayer,firstIndex,&pos);
    BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
    if (rc!=BERR_SUCCESS)
    {
        BDBG_ERR(("%s: unable to get position info for nav index %ld",__FUNCTION__,firstIndex));
        rc = B_DVR_UNKNOWN;
        goto error;
    }

    segmentedFilePlay->mediaRead->linearCurrentOffset =0;
    segmentedFilePlay->navRead->linearCurrentOffset =0;

    pFirst->timestamp = pos.timestamp;
    pFirst->mpegFileOffset = (pos.offsetLo|(((off_t)pos.offsetHi)<<32));
    pFirst->mpegFileOffset = B_DVR_IO_ALIGN_TRUNC(pFirst->mpegFileOffset);
    pFirst->navFileOffset = pos.index*segmentedFilePlay->navEntrySize;
error:
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFilePlay_ResetCachedFileNode(
    NEXUS_FilePlayHandle nexusFilePlay)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_SegmentedFilePlayHandle segmentedFilePlay = (B_DVR_SegmentedFilePlayHandle)nexusFilePlay;
    BDBG_ASSERT(segmentedFilePlay);
    BKNI_AcquireMutex(segmentedFilePlay->mediaRead->metaDataFileMutex);
    if(segmentedFilePlay->mediaRead->pCachedFileNode) 
    {
        B_DVR_File_Close(&segmentedFilePlay->mediaRead->dvrFile);
        segmentedFilePlay->mediaRead->pCachedFileNode = NULL;
    }
    BKNI_ReleaseMutex(segmentedFilePlay->mediaRead->metaDataFileMutex);

    BKNI_AcquireMutex(segmentedFilePlay->navRead->metaDataFileMutex);
    if(segmentedFilePlay->navRead->pCachedFileNode) 
    {
        B_DVR_File_Close(&segmentedFilePlay->navRead->dvrFile);
        segmentedFilePlay->navRead->pCachedFileNode = NULL;
    }
    BKNI_ReleaseMutex(segmentedFilePlay->navRead->metaDataFileMutex);
    return rc;
}
B_DVR_ERROR B_DVR_SegmentedFilePlay_GetLocation(
    NEXUS_FilePlayHandle nexusFilePlay, 
    long index,
    unsigned long timestamp, 
    B_DVR_FilePosition *pPosition)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BNAV_Player_Position pos;
    B_DVR_SegmentedFilePlayHandle segmentedFilePlay = (B_DVR_SegmentedFilePlayHandle)nexusFilePlay;

    BDBG_ASSERT((nexusFilePlay));
    BDBG_ASSERT((pPosition));
    BKNI_AcquireMutex(segmentedFilePlay->navPlayerMutex);
    if(segmentedFilePlay->navPlayer==NULL)
    {
        B_DVR_SegmentedFilePlay_P_OpenNavPlayer(segmentedFilePlay);
        if(!segmentedFilePlay->navPlayer)
        {
            rc = B_DVR_UNKNOWN;
            BDBG_ERR(("%s:no navigation data available",__FUNCTION__));
            if(pPosition)
            {
                pPosition->navFileOffset = 0;
                pPosition->mpegFileOffset = 0;
                pPosition->timestamp = 0;
                pPosition->index = 0;
            }
            BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
            goto error;
        }
    }
    if(index<0)
    {
        index = BNAV_Player_FindIndexFromTimestamp(segmentedFilePlay->navPlayer,timestamp);
    }
    if (index == -1)
    {
        rc = B_DVR_UNKNOWN;
        BDBG_ERR(("%s:unable to find nav index at timestamp:%lu",__FUNCTION__,timestamp));
        BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
        goto error;
    }

    rc = BNAV_Player_GetPositionInformation(segmentedFilePlay->navPlayer, index, &pos);
    if (rc!=0) 
    { 
       rc = B_DVR_UNKNOWN; 
       BDBG_ERR(("%s:unable to find position info at nav index:%ld",__FUNCTION__,index));
       BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
       goto error;
    }
    BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
    pPosition->timestamp = pos.timestamp;
    pPosition->mpegFileOffset = (pos.offsetLo|(((off_t)pos.offsetHi)<<32));
    pPosition->navFileOffset = pos.index*segmentedFilePlay->navEntrySize;
    pPosition->index = pos.index;
error:
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_GetNextIFrame(
    NEXUS_FileRecordHandle nexusFileRecord, 
    B_DVR_FilePosition currentPosition,   
    B_DVR_FilePosition *iFramePosition,
    unsigned *sizeOfFrame,
    B_DVR_FileReadDirection direction)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    long currentIndex,iFrameIndex;
    BNAV_Player_Position pos;
    BNAV_Entry mpeg2NavEntry;
    BNAV_AVC_Entry avcNavEntry;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)nexusFileRecord;
    BDBG_ASSERT(nexusFileRecord);
    BDBG_ASSERT(iFramePosition);
    BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
    if(segmentedFileRecord->navPlayer==NULL)
    {
        B_DVR_SegmentedFileRecord_P_OpenNavPlayer(segmentedFileRecord);
        if(!segmentedFileRecord->navPlayer)
        { 
            BDBG_ERR(("%s:no navigation data available",__FUNCTION__));
            rc = B_DVR_UNKNOWN;
            dumpSegmentedFileRecord(segmentedFileRecord);
            BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
            goto error;
        }
    }
    BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);

    currentIndex = currentPosition.index;
    if (currentIndex == -1)
    {
        rc = B_DVR_UNKNOWN;
        BDBG_ERR(("%s:invalid currentPosition.index",__FUNCTION__));
        goto error;
    }
    BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
    iFrameIndex = BNAV_Player_FindIFrameFromIndex(segmentedFileRecord->navPlayer, 
                                                  currentIndex,
                                                  (direction == eB_DVR_FileReadDirectionForward)? eBpForward:eBpReverse);
    if (iFrameIndex == -1)
    {
        rc = B_DVR_UNKNOWN;
        BDBG_ERR(("%s:unable to find i Frame Index from nav index %ld direction %u",__FUNCTION__,currentIndex,direction));
        BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
        goto error;
    }
    
    rc = BNAV_Player_GetPositionInformation(segmentedFileRecord->navPlayer, iFrameIndex, &pos);
    if(rc!=0)
    {
        rc = B_DVR_UNKNOWN;
        BDBG_ERR(("%s:unable to find position info for nav index %ld",__FUNCTION__,iFrameIndex));
        BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
        goto error;
    }

    iFramePosition->mpegFileOffset = (pos.offsetLo|(((off_t)pos.offsetHi)<<32));
    iFramePosition->navFileOffset = pos.index*segmentedFileRecord->navEntrySize;
    iFramePosition->timestamp = pos.timestamp;
    iFramePosition->index = pos.index;    
    iFramePosition->seqHdrOffset = pos.metadataOffset;
    iFramePosition->pts = pos.pts;
    if(segmentedFileRecord->navEntrySize == sizeof(BNAV_AVC_Entry))
    {
         rc =  BNAV_Player_ReadAvcIndex(segmentedFileRecord->navPlayer,iFrameIndex,&avcNavEntry);
         *sizeOfFrame = BNAV_get_frameSize(&avcNavEntry);
    }
    else
    {
         rc =  BNAV_Player_ReadIndex(segmentedFileRecord->navPlayer,iFrameIndex,&mpeg2NavEntry);
         *sizeOfFrame = BNAV_get_frameSize(&mpeg2NavEntry);
    }

    BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
error:
    return rc;
}


B_DVR_ERROR B_DVR_SegmentedFilePlay_GetNextIFrame(
    NEXUS_FilePlayHandle nexusFilePlay, 
    B_DVR_FilePosition currentPosition,   
    B_DVR_FilePosition *iFramePosition,
    unsigned *sizeOfFrame,
    B_DVR_FileReadDirection direction)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    long currentIndex,iFrameIndex;
    BNAV_Player_Position pos;
    BNAV_Entry mpeg2NavEntry;
    BNAV_AVC_Entry avcNavEntry;
    B_DVR_SegmentedFilePlayHandle segmentedFilePlay = (B_DVR_SegmentedFilePlayHandle)nexusFilePlay;
    BDBG_ASSERT(nexusFilePlay);
    BDBG_ASSERT(iFramePosition);
    BKNI_AcquireMutex(segmentedFilePlay->navPlayerMutex);
    if(segmentedFilePlay->navPlayer==NULL)
    {
        B_DVR_SegmentedFilePlay_P_OpenNavPlayer(segmentedFilePlay);
        if(!segmentedFilePlay->navPlayer)
        { 
            BDBG_ERR(("%s:no navigation data available",__FUNCTION__));
            BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
            goto error;
        }
    }
    BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
    currentIndex = currentPosition.index;
    if (currentIndex == -1)
    {
        rc = B_DVR_UNKNOWN;
        BDBG_ERR(("%s:invalid current index",__FUNCTION__));
        goto error;
    }

    BKNI_AcquireMutex(segmentedFilePlay->navPlayerMutex);
    iFrameIndex = BNAV_Player_FindIFrameFromIndex(segmentedFilePlay->navPlayer, 
                                                  currentIndex,
                                                  (direction == eB_DVR_FileReadDirectionForward)? eBpForward:eBpReverse);
    if (iFrameIndex == -1)
    {
        rc = B_DVR_UNKNOWN;
        BDBG_ERR(("%s:unable to find I frame index from nav index %ld",__FUNCTION__,currentIndex));
        BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
        goto error;
    }
    
    rc = BNAV_Player_GetPositionInformation(segmentedFilePlay->navPlayer, iFrameIndex, &pos);
    if(rc!=0)
    {
        rc = B_DVR_UNKNOWN;
        BDBG_ERR(("%s:unable to get position info for I frame index %ld",__FUNCTION__,iFrameIndex));
        BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
        goto error;
    }

    iFramePosition->mpegFileOffset = (pos.offsetLo|(((off_t)pos.offsetHi)<<32));
    iFramePosition->navFileOffset = pos.index*segmentedFilePlay->navEntrySize;
    iFramePosition->timestamp = pos.timestamp;
    iFramePosition->index = pos.index;    
    iFramePosition->seqHdrOffset = pos.metadataOffset;
    iFramePosition->pts = pos.pts;
    if(segmentedFilePlay->navEntrySize == sizeof(BNAV_AVC_Entry))
    {
         rc =  BNAV_Player_ReadAvcIndex(segmentedFilePlay->navPlayer,iFrameIndex,&avcNavEntry);
         *sizeOfFrame = BNAV_get_frameSize(&avcNavEntry);
    }
    else
    {
         rc =  BNAV_Player_ReadIndex(segmentedFilePlay->navPlayer,iFrameIndex,&mpeg2NavEntry);
         *sizeOfFrame = BNAV_get_frameSize(&mpeg2NavEntry);
    }
    BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
error:
    return rc;
}


B_DVR_ERROR B_DVR_SegmentedFileRecord_GetTimestamp(
    NEXUS_FileRecordHandle nexusFileRecord,
    off_t offset,
    unsigned long *timestamp)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned long offsetHi, offsetLow;
    long index;
    BNAV_Player_Position position;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)nexusFileRecord;

    BDBG_ASSERT(nexusFileRecord);
    BDBG_ASSERT(timestamp);
    *timestamp=0;
    offsetLow = offset & 0xFFFFFFFF;
    offsetHi = (offset>>32) & 0xFFFFFFFF;

    BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
    if(segmentedFileRecord->navPlayer==NULL)
    {
        B_DVR_SegmentedFileRecord_P_OpenNavPlayer(segmentedFileRecord);
        if(!segmentedFileRecord->navPlayer)
        {
            BDBG_ERR(("%s:no navigation data available",__FUNCTION__));
            rc = B_DVR_UNKNOWN;
            dumpSegmentedFileRecord(segmentedFileRecord);
            BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
            goto error;
        }
    }

    index = BNAV_Player_FindIndexFromOffset(segmentedFileRecord->navPlayer,offsetHi,offsetLow);
    if(index == -1)
    {
        rc = B_DVR_UNKNOWN;
        BDBG_ERR(("%s:unable to find nav index for off:%lld",__FUNCTION__,offset)); 
        dumpSegmentedFileRecord(segmentedFileRecord); 
        BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
        goto error;
    }
    
    rc = BNAV_Player_GetPositionInformation(segmentedFileRecord->navPlayer,index, &position);
    if (rc != 0)
    {
        rc = B_DVR_UNKNOWN;
        BDBG_ERR(("%s:unable to find offset for nav index:%ld",__FUNCTION__,index)); 
        dumpSegmentedFileRecord(segmentedFileRecord);
        BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
        goto error;      
    }
    *timestamp = position.timestamp;
    BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
error:
    return rc;
}


B_DVR_ERROR B_DVR_SegmentedFilePlay_GetTimestamp(
    NEXUS_FilePlayHandle nexusFilePlay,
    off_t offset,
    unsigned long *timestamp)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned long offsetHi, offsetLow;
    long index;
    BNAV_Player_Position position;
    B_DVR_SegmentedFilePlayHandle segmentedFilePlay = (B_DVR_SegmentedFilePlayHandle)nexusFilePlay;

    BDBG_ASSERT(nexusFilePlay);
    BDBG_ASSERT(timestamp);
    *timestamp=0;
    offsetLow = offset & 0xFFFFFFFF;
    offsetHi = (offset>>32) & 0xFFFFFFFF;

    BKNI_AcquireMutex(segmentedFilePlay->navPlayerMutex);
    if(segmentedFilePlay->navPlayer==NULL)
    {
        B_DVR_SegmentedFilePlay_P_OpenNavPlayer(segmentedFilePlay);
        if(!segmentedFilePlay->navPlayer)
        {
            BDBG_ERR(("%s:no navigation data available",__FUNCTION__));
            rc = B_DVR_UNKNOWN;
            BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
            goto error;
        }
    }
    index = BNAV_Player_FindIndexFromOffset(segmentedFilePlay->navPlayer,offsetHi,offsetLow);
    if(index == -1)
    {
        rc = B_DVR_UNKNOWN;    
        BDBG_ERR(("%s:unable to find nav index for off:%lld",__FUNCTION__,offset));
        BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
        goto error;
    }
    
    rc = BNAV_Player_GetPositionInformation(segmentedFilePlay->navPlayer,index, &position);
    if (rc != 0)
    {
        rc = B_DVR_UNKNOWN; 
        BDBG_ERR(("%s:unable to find offset for index %ld",__FUNCTION__,index));
        BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
        goto error;      
    }
    BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
    *timestamp = position.timestamp;
error:
    return rc;
}

int B_DVR_SegmentedFileRecord_GetNumOfFrames(
    NEXUS_FileRecordHandle nexusFileRecord,
    B_DVR_VideoFrameType frameType,
    off_t startOffset,
    off_t endOffset)
{

    unsigned long offsetLow,offsetHi;
    long startNavIndex,endNavIndex;
    eSCType startCodeType;
    int frameCount=0;
    BNAV_Entry mpeg2NavEntry;
    BNAV_AVC_Entry avcNavEntry;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)nexusFileRecord;
    BDBG_ASSERT(nexusFileRecord);

    BKNI_AcquireMutex(segmentedFileRecord->navPlayerMutex);
    if(segmentedFileRecord->navPlayer==NULL)
    {
        B_DVR_SegmentedFileRecord_P_OpenNavPlayer(segmentedFileRecord);
        if(!segmentedFileRecord->navPlayer)
        {
            BDBG_ERR(("%s:no navigation data available",__FUNCTION__));
            dumpSegmentedFileRecord(segmentedFileRecord);
            BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
            goto error;
        }
    }

    offsetLow = startOffset & 0xFFFFFFFF;
    offsetHi = (startOffset>>32) & 0xFFFFFFFF;

    startNavIndex = BNAV_Player_FindIndexFromOffset(segmentedFileRecord->navPlayer,offsetHi,offsetLow);
    if(startNavIndex == -1)
    {
        BDBG_ERR(("%s:No nav Index at off %lld",__FUNCTION__,startOffset));
        BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
        goto error;
    }

    offsetLow = endOffset & 0xFFFFFFFF;
    offsetHi = (endOffset>>32) & 0xFFFFFFFF;
    endNavIndex = BNAV_Player_FindIndexFromOffset(segmentedFileRecord->navPlayer,offsetHi,offsetLow);
    if(endNavIndex == -1)
    {
        BDBG_ERR(("%s:No nav Index at off %lld",__FUNCTION__,endOffset));
        BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
        goto error;
    }

    for (;startNavIndex<endNavIndex;startNavIndex++) 
    {
        if(segmentedFileRecord->navEntrySize == sizeof(BNAV_AVC_Entry))
        {
            BNAV_Player_ReadAvcIndex(segmentedFileRecord->navPlayer,startNavIndex,&avcNavEntry);
            startCodeType = BNAV_get_frameType(&avcNavEntry);
        }
        else
        {
            BNAV_Player_ReadIndex(segmentedFileRecord->navPlayer,startNavIndex,&mpeg2NavEntry);
            startCodeType = BNAV_get_frameType(&mpeg2NavEntry);
        }
        if((startCodeType == eSCTypeBFrame) &&  (frameType == eB_DVR_VideoFrameTypeB))
        {
            frameCount++;
        }
        else
        {
            if((startCodeType == eSCTypePFrame) && (frameType == eB_DVR_VideoFrameTypeP))
            {
                frameCount++;
            }
            else
            {
                if((startCodeType == eSCTypeIFrame) && (frameType == eB_DVR_VideoFrameTypeI))
                {
                    frameCount++;
                }
            }
        }
    }
    BKNI_ReleaseMutex(segmentedFileRecord->navPlayerMutex);
    return frameCount;
error:
    return -1;
}


int B_DVR_SegmentedFilePlay_GetNumOfFrames(
    NEXUS_FilePlayHandle nexusFilePlay,
    B_DVR_VideoFrameType frameType,
    off_t startOffset,
    off_t endOffset)
{

    unsigned long offsetLow,offsetHi;
    long startNavIndex,endNavIndex;
    eSCType startCodeType;
    int frameCount=0;
    BNAV_Entry mpeg2NavEntry;
    BNAV_AVC_Entry avcNavEntry;
    B_DVR_SegmentedFilePlayHandle segmentedFilePlay = (B_DVR_SegmentedFilePlayHandle)nexusFilePlay;
    BDBG_ASSERT(nexusFilePlay);

    BKNI_AcquireMutex(segmentedFilePlay->navPlayerMutex);
    if(segmentedFilePlay->navPlayer==NULL)
    {
        B_DVR_SegmentedFilePlay_P_OpenNavPlayer(segmentedFilePlay);
        if(!segmentedFilePlay->navPlayer)
        {
            BDBG_ERR(("%s:no navigation data available",__FUNCTION__));
            BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
            goto error;
        }
    }

    offsetLow = startOffset & 0xFFFFFFFF;
    offsetHi = (startOffset>>32) & 0xFFFFFFFF;

    startNavIndex = BNAV_Player_FindIndexFromOffset(segmentedFilePlay->navPlayer,offsetHi,offsetLow);
    if(startNavIndex == -1)
    {
        BDBG_ERR(("%s:No nav Index at off %lld",__FUNCTION__,startOffset));
        BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
        goto error;
    }

    offsetLow = endOffset & 0xFFFFFFFF;
    offsetHi = (endOffset>>32) & 0xFFFFFFFF;
    endNavIndex = BNAV_Player_FindIndexFromOffset(segmentedFilePlay->navPlayer,offsetHi,offsetLow);
    if(endNavIndex == -1)
    {
        BDBG_ERR(("%s:No nav Index at off %lld",__FUNCTION__,endOffset));
        BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
        goto error;
    }

    for (;startNavIndex<endNavIndex;startNavIndex++) 
    {
        if(segmentedFilePlay->navEntrySize == sizeof(BNAV_AVC_Entry))
        {
            BNAV_Player_ReadAvcIndex(segmentedFilePlay->navPlayer,startNavIndex,&avcNavEntry);
            startCodeType = BNAV_get_frameType(&avcNavEntry);
        }
        else
        {
            BNAV_Player_ReadIndex(segmentedFilePlay->navPlayer,startNavIndex,&mpeg2NavEntry);
            startCodeType = BNAV_get_frameType(&mpeg2NavEntry);
        }
        if((startCodeType == eSCTypeBFrame) &&  (frameType == eB_DVR_VideoFrameTypeB))
        {
            frameCount++;
        }
        else
        {
            if((startCodeType == eSCTypePFrame) && (frameType == eB_DVR_VideoFrameTypeP))
            {
                frameCount++;
            }
            else
            {
                if((startCodeType == eSCTypeIFrame) && (frameType == eB_DVR_VideoFrameTypeI))
                {
                    frameCount++;
                }
            }
        }
    }
    BKNI_ReleaseMutex(segmentedFilePlay->navPlayerMutex);
    return frameCount;
error:
    return -1;
     
}

B_DVR_ERROR B_DVR_SegmentedFilePlay_UpdateFileList(NEXUS_FilePlayHandle nexusFilePlay)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_SegmentedFilePlayHandle segmentedFilePlay = (B_DVR_SegmentedFilePlayHandle)nexusFilePlay;
    BDBG_ASSERT(segmentedFilePlay);
    BKNI_AcquireMutex(segmentedFilePlay->mediaRead->metaDataFileMutex);
    BKNI_AcquireMutex(segmentedFilePlay->navRead->metaDataFileMutex);
    rc = updateFileList(segmentedFilePlay->mediaRead);
    rc = updateFileList(segmentedFilePlay->navRead);
    BKNI_ReleaseMutex(segmentedFilePlay->navRead->metaDataFileMutex);
    BKNI_ReleaseMutex(segmentedFilePlay->mediaRead->metaDataFileMutex);
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_Alloc(const char *mediaFileName,
                                            const char *navFileName,
                                            B_DVR_SegmentedFileSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_SegmentedFileNode fileNode;
    B_DVR_FileSettings fileSettings;
    char fullFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    char path[B_DVR_MAX_FILE_NAME_LENGTH];
    B_DVR_File dvrFile;
    unsigned fileCount=0;
    BDBG_ASSERT(mediaFileName);
    BDBG_ASSERT(navFileName);
    BDBG_ASSERT(pSettings->mediaStorage);
    
    fileSettings.directIO = false;
    fileSettings.fileIO = eB_DVR_FileIOWrite;
    fileSettings.create = true;
    B_DVR_MediaStorage_GetMetadataPath(pSettings->mediaStorage,pSettings->volumeIndex,path);
    if(pSettings->metaDataSubDir && pSettings->metaDataSubDir[0] != '\0')
    { 
        BKNI_Snprintf(fullFileName, sizeof(fullFileName), "%s/%s/%s", path,pSettings->metaDataSubDir,mediaFileName);
    }
    else
    {
        BKNI_Snprintf(fullFileName, sizeof(fullFileName), "%s/%s", path,mediaFileName);
    }
    rc = B_DVR_File_Open(&dvrFile,fullFileName,&fileSettings);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:file open error %s",__FUNCTION__,fullFileName));
        goto error_fileOpen;
    }
    else
    {
        BDBG_MSG_TRACE(("%s: meta data file open %s",__FUNCTION__,fullFileName));
    }
    for(fileCount=0;fileCount<pSettings->maxSegmentCount;fileCount++)
    {
        BKNI_Memset((void *)(&fileNode),0,sizeof(fileNode));
        rc = B_DVR_MediaStorage_AllocateMediaSegment(pSettings->mediaStorage,
                                                     pSettings->volumeIndex,
                                                     pSettings->metaDataSubDir,
                                                     fileNode.fileName);
        if(rc!=B_DVR_SUCCESS)
        {
            BDBG_ERR(("%s:media file allocate failed",__FUNCTION__));
            goto error_fileNode;
        }
        else
        {
            BDBG_MSG_TRACE(("media file allocate %s",fileNode.fileName));

        }
        fileNode.recordType = eB_DVR_RecordingTSB;
        fileNode.index=fileCount;
        writeMetaDataFile(&dvrFile,&fileNode);
    }

    BDBG_MSG_TRACE(("%s: meta data file close %s",__FUNCTION__,fullFileName));
    fdatasync(dvrFile.fd);
    B_DVR_File_Close(&dvrFile);

    if(pSettings->metaDataSubDir && pSettings->metaDataSubDir[0] != '\0')
    { 
        BKNI_Snprintf(fullFileName, sizeof(fullFileName), "%s/%s/%s", path,pSettings->metaDataSubDir,navFileName);
    }
    else
    {
        BKNI_Snprintf(fullFileName, sizeof(fullFileName), "%s/%s", path,navFileName);
    }
    fileSettings.directIO = false;
    fileSettings.fileIO = eB_DVR_FileIOWrite;
    fileSettings.create = true;
    rc = B_DVR_File_Open(&dvrFile,fullFileName,&fileSettings);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:file open error %s",__FUNCTION__,fullFileName));
        goto error_fileOpen;
    }
    else
    {
        BDBG_MSG_TRACE(("%s: meta data file open %s",__FUNCTION__,fullFileName));
    }
    for(fileCount=0;fileCount<pSettings->maxSegmentCount;fileCount++)
    {
        BKNI_Memset((void *)(&fileNode),0,sizeof(fileNode));
        rc = B_DVR_MediaStorage_AllocateNavSegment(pSettings->mediaStorage,
                                                   pSettings->volumeIndex,
                                                   fileNode.fileName);
        if(rc!=B_DVR_SUCCESS)
        {
            BDBG_ERR(("%s:nav file allocate failed",__FUNCTION__));
            goto error_fileNode;
        }
        else
        {
            BDBG_MSG_TRACE(("%s:nav file allocate %s",__FUNCTION__,fileNode.fileName));

        }
        fileNode.recordType = eB_DVR_RecordingTSB;
        fileNode.index=fileCount;
        writeMetaDataFile(&dvrFile,&fileNode);
    }
    BDBG_MSG_TRACE(("%s: meta data file close %s",__FUNCTION__,fullFileName));
    fdatasync(dvrFile.fd);
    B_DVR_File_Close(&dvrFile);
    return rc;
error_fileNode:
    B_DVR_File_Close(&dvrFile);
error_fileOpen:
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_Free(const char *mediaFileName,
                                           const char *navFileName,
                                           B_DVR_SegmentedFileSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_SegmentedFileNode fileNode;
    B_DVR_FileSettings fileSettings;
    B_DVR_File dvrFile;
    ssize_t sizeRead=0;
    unsigned fileCount=0;
    off_t returnOffset, fileNodeOffset;
    char fullFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    char path[B_DVR_MAX_FILE_NAME_LENGTH];
    
    BDBG_ASSERT(mediaFileName);
    BDBG_ASSERT(navFileName);
    BDBG_ASSERT(pSettings->mediaStorage);
    
    B_DVR_MediaStorage_GetMetadataPath(pSettings->mediaStorage,pSettings->volumeIndex,path);
    if(pSettings->metaDataSubDir && pSettings->metaDataSubDir[0] != '\0')
    { 
        BKNI_Snprintf(fullFileName, sizeof(fullFileName), "%s/%s/%s", path,pSettings->metaDataSubDir,mediaFileName);
    }
    else
    {
        BKNI_Snprintf(fullFileName, sizeof(fullFileName), "%s/%s", path,mediaFileName);
    }
    fileSettings.directIO = false;
    fileSettings.fileIO = eB_DVR_FileIORead;
    fileSettings.create = false;
    rc = B_DVR_File_Open(&dvrFile,fullFileName,&fileSettings);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:error in opening %s",__FUNCTION__,fullFileName));
        goto error_fileOpen;
    }
    else
    {
        BDBG_MSG_TRACE(("%s: meta data file open %s",__FUNCTION__,fullFileName));
    }

    do
    {

        fileNodeOffset = fileCount*sizeof(fileNode);
        returnOffset =  dvrFile.fileInterface.io.readFD.seek(
            &dvrFile.fileInterface.io.readFD,
            fileNodeOffset,SEEK_SET);

        if(returnOffset != fileNodeOffset)
        {
            break;
        }

        sizeRead = dvrFile.fileInterface.io.readFD.read(
            &dvrFile.fileInterface.io.readFD,(void *)&fileNode,sizeof(fileNode));

        if(sizeRead == sizeof(fileNode))
        {
            BDBG_MSG_TRACE(("%s:free media seg %s",__FUNCTION__,fileNode.fileName));
            rc = B_DVR_MediaStorage_FreeMediaSegment(pSettings->mediaStorage,
                                                pSettings->volumeIndex,
                                                pSettings->metaDataSubDir,
                                                fileNode.fileName);
            if(rc!=B_DVR_SUCCESS)
            {
                BDBG_ERR(("%s: B_DVR_MediaStorage_FreeMediaSegment failed %s",
                          __FUNCTION__,fileNode.fileName));
            }
            fileCount++;

        }
    }while(sizeRead);

    BDBG_MSG_TRACE(("%s: meta data file close %s",__FUNCTION__,fullFileName));
    B_DVR_File_Close(&dvrFile);
    BDBG_MSG_TRACE(("%s: meta data file remove %s",__FUNCTION__,fullFileName));
    unlink(fullFileName);

    if(pSettings->metaDataSubDir && pSettings->metaDataSubDir[0] != '\0')
    { 
        BKNI_Snprintf(fullFileName, sizeof(fullFileName), "%s/%s/%s", path,pSettings->metaDataSubDir,navFileName);
    }
    else
    {
        BKNI_Snprintf(fullFileName, sizeof(fullFileName), "%s/%s", path,navFileName);
    }
    fileCount=0;
    fileSettings.directIO = false;
    fileSettings.fileIO = eB_DVR_FileIORead;
    fileSettings.create = false;
    rc = B_DVR_File_Open(&dvrFile,fullFileName,&fileSettings);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:error in opening %s",__FUNCTION__,fullFileName));
        goto error_fileOpen;
    }
    else
    {
        BDBG_MSG_TRACE(("%s: meta data file open %s",__FUNCTION__,fullFileName));
    }
    do
    {

        fileNodeOffset = fileCount*sizeof(fileNode);
        returnOffset =  dvrFile.fileInterface.io.readFD.seek(
            &dvrFile.fileInterface.io.readFD,
            fileNodeOffset,SEEK_SET);

        if(returnOffset != fileNodeOffset)
        {
            break;
        }

        sizeRead = dvrFile.fileInterface.io.readFD.read(
            &dvrFile.fileInterface.io.readFD,(void *)&fileNode,sizeof(fileNode));

        if(sizeRead == sizeof(fileNode))
        {
            BDBG_MSG_TRACE(("%s:free nav seg %s",__FUNCTION__,fileNode.fileName));
            rc = B_DVR_MediaStorage_FreeNavSegment(pSettings->mediaStorage,
                                                   pSettings->volumeIndex,
                                                   fileNode.fileName);
            if(rc!=B_DVR_SUCCESS)
            {
                BDBG_ERR(("%s:FreeNavSegment failed %s",__FUNCTION__,fileNode.fileName));
            }
            fileCount++;

        }
    }while(sizeRead);

    BDBG_MSG_TRACE(("%s: meta data file close %s",__FUNCTION__,fullFileName));
    B_DVR_File_Close(&dvrFile);
    BDBG_MSG_TRACE(("%s: meta data file remove %s",__FUNCTION__,fullFileName));
    unlink(fullFileName);
error_fileOpen:
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFileRecord_Print(const char *mediaFileName,
                                            const char *navFileName,
                                            B_DVR_SegmentedFileSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_SegmentedFileNode fileNode;
    B_DVR_FileSettings fileSettings;
    B_DVR_File dvrFile;
    ssize_t sizeRead=0;
    off_t returnOffset, fileNodeOffset;
    unsigned fileCount=0;
    char fullFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    char path[B_DVR_MAX_FILE_NAME_LENGTH];

    BDBG_ASSERT(mediaFileName);
    BDBG_ASSERT(navFileName);
    B_DVR_MediaStorage_GetMetadataPath(pSettings->mediaStorage,pSettings->volumeIndex,path);
    if(pSettings->metaDataSubDir && pSettings->metaDataSubDir[0] != '\0')
    { 
        BKNI_Snprintf(fullFileName, sizeof(fullFileName), "%s/%s/%s", path,pSettings->metaDataSubDir,mediaFileName);
    }
    else
    {
        BKNI_Snprintf(fullFileName, sizeof(fullFileName), "%s/%s", path,mediaFileName);
    }
    fileSettings.directIO = false;
    fileSettings.fileIO = eB_DVR_FileIORead;
    fileSettings.create = false;
    rc = B_DVR_File_Open(&dvrFile,fullFileName,&fileSettings);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:error in opening file %s",__FUNCTION__,fullFileName));
        goto error_fileOpen;
    }
    else
    {
        BDBG_MSG_TRACE(("%s: meta data file open %s",__FUNCTION__,fullFileName));
    }

    do
    {

        fileNodeOffset = fileCount*sizeof(fileNode);
        returnOffset =  dvrFile.fileInterface.io.readFD.seek(
            &dvrFile.fileInterface.io.readFD,
            fileNodeOffset,SEEK_SET);

        if(returnOffset != fileNodeOffset)
        {
            BKNI_Printf("\n");
            break;
        }

        sizeRead = dvrFile.fileInterface.io.readFD.read(
            &dvrFile.fileInterface.io.readFD,(void*)&fileNode,sizeof(fileNode));

        if(sizeRead == sizeof(fileNode))
        {
            BKNI_Printf("\n media seg:%u name:%s start:%lld size:%u segoff:%lld",
                        fileCount,
                        fileNode.fileName,
                        fileNode.linearStartOffset,
                        fileNode.size,
                        fileNode.segmentOffset);

            fileCount++;
        }
    }while(sizeRead);

    BDBG_MSG_TRACE(("%s: meta data file close %s",__FUNCTION__,fullFileName));
    B_DVR_File_Close(&dvrFile);

    if(pSettings->metaDataSubDir && pSettings->metaDataSubDir[0] != '\0')
    { 
        BKNI_Snprintf(fullFileName, sizeof(fullFileName), "%s/%s/%s", path,pSettings->metaDataSubDir,navFileName);
    }
    else
    {
        BKNI_Snprintf(fullFileName, sizeof(fullFileName), "%s/%s", path,navFileName);
    }
    fileCount=0;
    fileSettings.directIO = false;
    fileSettings.fileIO = eB_DVR_FileIORead;
    fileSettings.create = false;
    rc= B_DVR_File_Open(&dvrFile,fullFileName,&fileSettings);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:error in opening %s",__FUNCTION__,fullFileName));
        goto error_fileOpen;
    }
    else
    {
        BDBG_MSG_TRACE(("%s: meta data file open %s",__FUNCTION__,fullFileName));
    }
    
    do
    {

        fileNodeOffset = fileCount*sizeof(fileNode);
        returnOffset =  dvrFile.fileInterface.io.readFD.seek(
            &dvrFile.fileInterface.io.readFD,
            fileNodeOffset,SEEK_SET);

        if(returnOffset != fileNodeOffset)
        {
            BKNI_Printf("\n");
            break;
        }

        sizeRead = dvrFile.fileInterface.io.readFD.read(
            &dvrFile.fileInterface.io.readFD,(void *)&fileNode,sizeof(fileNode));

        if(sizeRead == sizeof(fileNode))
        {
            BKNI_Printf("\n nav seg:%d name:%s, start:%lld size:%u segoff:%lld",
                        fileCount,
                        fileNode.fileName,
                        fileNode.linearStartOffset,
                        fileNode.size,
                        fileNode.segmentOffset);
            fileCount++;
        }
    }while(sizeRead);
    BDBG_MSG_TRACE(("%s: meta data file close %s",__FUNCTION__,fullFileName));
    B_DVR_File_Close(&dvrFile);
error_fileOpen:
    return rc;
}

B_DVR_ERROR B_DVR_SegmentedFilePlay_SanityCheck(const char *mediaFileName,
                                                const char *navFileName,
                                                B_DVR_SegmentedFileSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    int mediaFileCount=0, navFileCount=0;
    char fullMediaFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    char fullNavFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    char path[B_DVR_MAX_FILE_NAME_LENGTH];
    BDBG_ASSERT(mediaFileName);
    BDBG_ASSERT(navFileName);
    BDBG_ASSERT(pSettings->mediaStorage);
    B_DVR_MediaStorage_GetMetadataPath(pSettings->mediaStorage,pSettings->volumeIndex,path);
    if(pSettings->metaDataSubDir && pSettings->metaDataSubDir[0] != '\0')
    { 
        BKNI_Snprintf(fullMediaFileName, sizeof(fullMediaFileName), "%s/%s/%s", path,pSettings->metaDataSubDir,mediaFileName);
    }
    else
    {
        BKNI_Snprintf(fullMediaFileName, sizeof(fullMediaFileName), "%s/%s", path,mediaFileName);
    }
    mediaFileCount = checkMetaDataFile(fullMediaFileName,
                                       pSettings,
                                       eB_DVR_FileTypeMedia);
    if(mediaFileCount<=0) 
    {
        BDBG_ERR(("%s has invalid file segments",fullMediaFileName));
        rc = B_DVR_UNKNOWN;
        goto error;
    }

    if(pSettings->metaDataSubDir && pSettings->metaDataSubDir[0] != '\0')
    { 
        BKNI_Snprintf(fullNavFileName, sizeof(fullNavFileName), "%s/%s/%s", path,pSettings->metaDataSubDir,navFileName);
    }
    else
    {
        BKNI_Snprintf(fullNavFileName, sizeof(fullNavFileName), "%s/%s", path,navFileName);
    }

    navFileCount = checkMetaDataFile(fullNavFileName, 
                                     pSettings,
                                     eB_DVR_FileTypeNavigation);
    if(navFileCount<=0) 
    {
        BDBG_ERR(("%s has invalid file segments",navFileName));
        rc = B_DVR_UNKNOWN;
        goto error;
    }
    BDBG_MSG(("navFileCount %d mediaFileCount %d",navFileCount,mediaFileCount));
    if(navFileCount!=mediaFileCount) 
    {
        BDBG_WRN(("media fc %d nav fc %d",mediaFileCount,navFileCount));
        if(navFileCount < mediaFileCount) 
        {
            BDBG_WRN(("reducing the entries in %s",fullMediaFileName));
            if(truncate(fullMediaFileName,sizeof(B_DVR_SegmentedFileNode)*navFileCount) <0)
            {
                BDBG_ERR(("failed to truncate %s",fullMediaFileName));
                rc = B_DVR_UNKNOWN;
                goto error;
            }
        }
        else
        {   
            BDBG_WRN(("reducing the entries in %s",fullNavFileName));
            if(truncate(fullNavFileName,sizeof(B_DVR_SegmentedFileNode)*mediaFileCount)<0)
            {
                BDBG_ERR(("failed to truncate %s",fullNavFileName));
                rc = B_DVR_UNKNOWN;
                goto error;
            }
        }
    }
error:
    return rc;
}
