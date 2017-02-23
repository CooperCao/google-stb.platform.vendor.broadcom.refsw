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

#ifndef _POWER_H__
#define _POWER_H__

#include "resource.h"

#ifdef __cplusplus
extern "C" {
#endif

#if POWERSTANDBY_SUPPORT
#include "pmlib.h"
#endif

class CGraphics;

typedef enum ePowerMode
{
    ePowerMode_S0,
    ePowerMode_S1,
    ePowerMode_S2,
    ePowerMode_S3,
    ePowerMode_Max
} ePowerMode;

class CMountData
{
public:
    CMountData() :
        _nPartition(0)
    {
        memset(_strPartition, 0, sizeof(_strPartition));
        memset(_strDrive, 0, sizeof(_strDrive));
        memset(_strMountType, 0, sizeof(_strMountType));
        memset(_strMountPoint, 0, sizeof(_strMountPoint));
    }

    virtual ~CMountData() {}

    eRet initializeDriveData(const char * str);
    eRet initializeMountData(const char * str);
    eRet initializePartitionData(const char * str);

    void         setDriveName(const char * str);
    const char * getDriveName(void) { return(_strDrive); }
    eRet         mount(bool bMount);
    void         dump(bool bForce = false);

    bool operator ==(CMountData &other);

public:
    int  _nPartition;
    char _strPartition[16];
    char _strDrive[16];
    char _strMountType[16];
    char _strMountPoint[32];
    MString _strDriveGUID;
};

class CPower : public CResource
{
public:
    CPower(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CPower(void);

    eRet       mountDrives(bool bMount, uint32_t nRetries);
    eRet       setMode(ePowerMode mode, CGraphics * pGraphics = NULL);
    ePowerMode getMode(void) { return((ePowerMode)_settings.mode); }

    eRet enableMoca(bool bEnable);
    eRet enableEthernet(bool bEnable);

    bool doLinuxPower(ePowerMode modeOld, ePowerMode modeNew);
    bool doDriveMount(ePowerMode modeOld, ePowerMode modeNew);
    bool doDriveUnmount(ePowerMode modeOld, ePowerMode modeNew);

protected:
    NEXUS_PlatformStandbySettings _settings;
    void *                _pPmLib; /* linux power management library context */
    MAutoList<CMountData> _mountList;
};

#ifdef __cplusplus
}
#endif

#endif /* POWER_H__ */