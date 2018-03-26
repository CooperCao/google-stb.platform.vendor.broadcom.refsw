/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef DVRMGR_H__
#define DVRMGR_H__

#include <sys/vfs.h>
#include <sys/statfs.h>
#include <linux/magic.h>
#include "atlas_os.h"
#include "config.h"
#include "mvc.h"
#include "mlist.h"
#include "mstring.h"
#include "b_dvr_mediastorage.h"
#include "b_dvr_manager.h"
#ifdef __cplusplus
extern "C" {
#endif

class CDvrMgr : public CMvcModel
{
public:
    CDvrMgr(void);
    ~CDvrMgr(void);
    eRet     init(B_DVR_MediaStorageType mediaStorageType);
    void     unInit();
    eRet     mountStorageDevice(unsigned volumeIndex);
    void     unMountStorageDevice(unsigned volumeIndex);
    eRet     registerStorageDevice(unsigned volumeIndex, char * virtualDevice, uint64_t size);
    eRet     reserveTsbSpace(unsigned volumeIndex, unsigned numTsb, unsigned tsbWindow, unsigned maxBitRate);
    void     unReserveTsbSpace(unsigned volumeIndex);
    unsigned getNumTsbBuffers(void) { return(_numTsb); }
    unsigned getTsbWindow(void)     { return(_tsbWindow); }
    unsigned getTsbVolume(void)     { return(_tsbVolume); }
    char *   getTsbSubDir(void)     { return(_tsbSubDir); }
    char *   getTsbPrefix(void)     { return(_tsbPrefix); }
protected:
    B_DVR_ManagerHandle      _hDvrManager;
    B_DVR_MediaStorageHandle _hMediaStorage;
    unsigned                 _numTsbBuffers;
    unsigned                 _tsbVolume;
    unsigned                 _numTsb;
    unsigned                 _tsbWindow;
    char                     _tsbSubDir[B_DVR_MAX_FILE_NAME_LENGTH];
    char                     _tsbPrefix[B_DVR_MAX_FILE_NAME_LENGTH];
    B_DVR_MediaStorageType   _mediaStorageType;
    B_MutexHandle            _mutex;
};

#ifdef __cplusplus
}
#endif

#endif /* DVRMGR_H__ */