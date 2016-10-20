/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "dvrmgr.h"

BDBG_MODULE(atlas_dvrmgr);

CDvrMgr::CDvrMgr() :
    CMvcModel("CDvrMgr"),
    _hDvrManager(NULL),
    _hMediaStorage(NULL),
    _numTsbBuffers(0),
    _tsbVolume(0),
    _numTsb(0),
    _tsbWindow(0),
    _mediaStorageType(eB_DVR_MediaStorageTypeLoopDevice)
{
    _mutex = B_Mutex_Create(NULL);
    sprintf(_tsbPrefix, "%s", "tsb");
    sprintf(_tsbSubDir, "%s", "tsb");
    BDBG_ASSERT(_mutex);
}

CDvrMgr::~CDvrMgr()
{
    if (_mutex)
    {
        B_Mutex_Destroy(_mutex);
        _mutex = NULL;
    }
}

eRet CDvrMgr::init(B_DVR_MediaStorageType mediaStorageType)
{
    eRet ret = eRet_Ok;
    B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;

    memset(&mediaStorageOpenSettings, 0, sizeof(B_DVR_MediaStorageOpenSettings));
    mediaStorageOpenSettings.storageType = mediaStorageType;
    _hMediaStorage                       = B_DVR_MediaStorage_Open(&mediaStorageOpenSettings);
    if (!_hMediaStorage)
    {
        BDBG_ERR((" B_DVR_MediaStorage_Open failed"));
        ret = eRet_NotAvailable;
    }
    _hDvrManager = B_DVR_Manager_Init(NULL);
    if (!_hDvrManager)
    {
        BDBG_ERR((" B_DVR_Manager_Init failed"));
        B_DVR_MediaStorage_Close(_hMediaStorage);
        ret = eRet_NotAvailable;
    }
    return(ret);
} /* init */

void CDvrMgr::unInit()
{
    B_DVR_Manager_UnInit(_hDvrManager);
    B_DVR_MediaStorage_Close(_hMediaStorage);
    return;
}

eRet CDvrMgr::mountStorageDevice(unsigned volumeIndex)
{
    B_DVR_ERROR dvrRetCode = B_DVR_SUCCESS;
    eRet        ret        = eRet_Ok;

    dvrRetCode = B_DVR_MediaStorage_MountVolume(_hMediaStorage, volumeIndex);
    if (dvrRetCode != B_DVR_SUCCESS)
    {
        BDBG_ERR(("mouting volume %u failed", volumeIndex));
        ret = eRet_ExternalError;
    }
    else
    {
        dvrRetCode = B_DVR_Manager_CreateMediaNodeList(_hDvrManager, volumeIndex);
        if (dvrRetCode != B_DVR_SUCCESS)
        {
            BDBG_ERR(("creating media node list in volume %u failed", volumeIndex));
            B_DVR_MediaStorage_UnmountVolume(_hMediaStorage, volumeIndex);
            ret = eRet_ExternalError;
        }
    }
    return(ret);
} /* mountStorageDevice */

void CDvrMgr::unMountStorageDevice(unsigned volumeIndex)
{
    B_DVR_Manager_DestroyMediaNodeList(_hDvrManager, volumeIndex);
    B_DVR_MediaStorage_UnmountVolume(_hMediaStorage, volumeIndex);
    return;
}

eRet CDvrMgr::registerStorageDevice(
        unsigned volumeIndex,
        char *   virtualDevice,
        uint64_t size
        )
{
    eRet          ret = eRet_Ok;
    struct statfs bufStat;
    B_DVR_MediaStorageRegisterVolumeSettings registerSettings;
    B_DVR_MediaStorageStatus                 mediaStorageStatus;

    if (statfs(virtualDevice, &bufStat) < 0)
    {
        BDBG_ERR(("%s isn't a mounted file system", virtualDevice));
        ret = eRet_NotSupported;
    }
    else
    {
        if (bufStat.f_type == NFS_SUPER_MAGIC)
        {
            BDBG_ERR(("%s is nfs mounted and not suitable for VSFS", virtualDevice));
            ret = eRet_NotSupported;
        }
        else
        {
            uint64_t totalSize, freeSize;
            totalSize  = bufStat.f_blocks*bufStat.f_bsize;
            totalSize /= 1024*1024; /* size in MB */
            BDBG_WRN(("size of mounted fs %s -> %lld MB", virtualDevice, totalSize));
            freeSize  = bufStat.f_bfree*bufStat.f_bsize;
            freeSize /= 1024*1024; /* size in MB */
            BDBG_WRN(("free size of mounted fs %s -> %lld MB", virtualDevice, freeSize));
            if (freeSize < size)
            {
                BDBG_WRN(("freeSize->%lld < requestedSize->%lld", freeSize, size));
                ret = eRet_ExternalError;
            }
            else
            {
                memset(&registerSettings, 0, sizeof(registerSettings));
                sprintf(registerSettings.device, "%s/%s", virtualDevice, "vsfs");
                registerSettings.startSec = 0;
                registerSettings.length   = size;
                B_DVR_MediaStorage_RegisterVolume(_hMediaStorage, &registerSettings, &volumeIndex);
                memset(&mediaStorageStatus, 0, sizeof(mediaStorageStatus));
                B_DVR_MediaStorage_GetStatus(_hMediaStorage, &mediaStorageStatus);
                if (!mediaStorageStatus.volumeInfo[volumeIndex].formatted)
                {
                    BDBG_WRN(("Creating VSFS at %s", registerSettings.device));
                    B_DVR_MediaStorage_FormatVolume(_hMediaStorage, volumeIndex);
                }
                else
                {
                    BDBG_WRN(("VSFS found at %s", registerSettings.device));
                }
            }
        }
    }
    return(ret);
} /* registerStorageDevice */

eRet CDvrMgr::reserveTsbSpace(
        unsigned volumeIndex,
        unsigned numTsb,
        unsigned tsbWindow,
        unsigned maxBitRate
        )
{
    eRet                    ret = eRet_Ok;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    unsigned                _maxTsbSegments;
    char                    programName[B_DVR_MAX_FILE_NAME_LENGTH];
    unsigned                i;
    uint64_t                temp;

    temp            = maxBitRate;
    temp           /= 8;
    temp           *= 1000000;
    tsbWindow      *= 60;
    _tsbWindow      = tsbWindow*1000;
    temp           *= tsbWindow;
    temp           /= B_DVR_MEDIA_SEGMENT_SIZE;
    _maxTsbSegments = (unsigned) temp;
    BDBG_WRN(("_maxTsbSegments %u _tsbWindow %u ms", _maxTsbSegments, _tsbWindow));
    for (i = 0; i < numTsb; i++)
    {
        memset((void *)&mediaNodeSettings, 0, sizeof(mediaNodeSettings));
        sprintf(programName, "%s_%u", _tsbPrefix, i);
        mediaNodeSettings.programName = programName;
        mediaNodeSettings.subDir      = _tsbSubDir;
        mediaNodeSettings.volumeIndex = volumeIndex;
        B_DVR_Manager_AllocSegmentedFileRecord(_hDvrManager, &mediaNodeSettings, _maxTsbSegments);
    }
    _numTsb = numTsb;
    return(ret);
} /* reserveTsbSpace */

void CDvrMgr::unReserveTsbSpace(unsigned volumeIndex)
{
    B_DVR_MediaNodeSettings mediaNodeSettings;
    char                    programName[B_DVR_MAX_FILE_NAME_LENGTH];
    unsigned                i;

    for (i = 0; i < _numTsb; i++)
    {
        memset((void *)&mediaNodeSettings, 0, sizeof(mediaNodeSettings));
        sprintf(programName, "%s_%u", _tsbPrefix, i);
        mediaNodeSettings.programName = programName;
        mediaNodeSettings.subDir      = _tsbSubDir;
        mediaNodeSettings.volumeIndex = volumeIndex;
        B_DVR_Manager_FreeSegmentedFileRecord(_hDvrManager, &mediaNodeSettings);
    }
    return;
} /* unReserveTsbSpace */