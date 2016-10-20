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
#include "power.h"
#include "graphics.h"
#include "atlas_cfg.h"
#include "nexus_platform_standby.h"

#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <fcntl.h>
#include <scsi/scsi.h>
#include <scsi/sg.h>

BDBG_MODULE(atlas_power);

void CMountData::dump(bool bForce)
{
    BDBG_Level level;

    if (true == bForce)
    {
        BDBG_GetModuleLevel("atlas_power", &level);
        BDBG_SetModuleLevel("atlas_power", BDBG_eMsg);
    }

    BDBG_MSG(("partition:%s%d mount point:%s type:%s", _strDrive, _nPartition, _strMountPoint, _strMountType));
    BDBG_MSG(("drive serial:%s", _strDriveSerial));
    BDBG_MSG(("drive model :%s", _strDriveModel));
    BDBG_MSG(("drive wwn   :%s", _strDriveWWN));

    if (true == bForce)
    {
        BDBG_SetModuleLevel("atlas_power", level);
    }
} /* dump */

/* read drive data for given device */
eRet CMountData::initializeDriveData(const char * str)
{
    eRet              ret = eRet_NotAvailable;
    int               fd  = 0;
    struct hd_driveid driveId;

    BDBG_ASSERT(NULL != str);

    BDBG_MSG(("%s opening %s", __FUNCTION__, str));
    fd = open(str, O_RDONLY|O_NONBLOCK);
    if (fd)
    {
        struct sg_io_hdr io_hdr;
        unsigned char    buf[64];
        unsigned char    sense[32];
        unsigned char    inq_cmd_serial[] = { INQUIRY, 1, 0x80, 0, sizeof(buf), 0 };

        memset(&io_hdr, 0, sizeof(io_hdr));
        io_hdr.interface_id    = 'S';
        io_hdr.cmdp            = inq_cmd_serial;
        io_hdr.cmd_len         = sizeof(inq_cmd_serial);
        io_hdr.dxferp          = buf;
        io_hdr.dxfer_len       = sizeof(buf) - 1;
        io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
        io_hdr.sbp             = sense;
        io_hdr.mx_sb_len       = sizeof(sense);
        io_hdr.timeout         = 5000;

        /* try get sata drive info */
        if (0 == ioctl(fd, HDIO_GET_IDENTITY, &driveId))
        {
            strncpy(_strDriveSerial, (char *)driveId.serial_no, sizeof(_strDriveSerial));
            strncpy(_strDriveModel, (char *)driveId.model, sizeof(_strDriveModel));
            snprintf(_strDriveWWN, sizeof(_strDriveWWN) - 1, "0x%04x%04x%04x%04x", driveId.words104_125[4], driveId.words104_125[5], driveId.words104_125[6], driveId.words104_125[7]);
            ret = eRet_Ok;
        }
        else /* try usb drive info */
        if (0 == ioctl(fd, SG_IO, &io_hdr))
        {
            if ((io_hdr.info & SG_INFO_OK_MASK) != SG_INFO_OK)
            {
                /* usb drive does NOT support serial number */
                CHECK_ERROR_GOTO("USB drive does not have serial number support", ret, error);
            }

            /* usb drive supports serial number */
            buf[4 + buf[3]] = '\0';
            memset(&_strDriveSerial, 0, sizeof(_strDriveSerial));
            strncpy(_strDriveSerial, (char *)buf + 4, sizeof(_strDriveSerial) - 1);
            memset(&_strDriveWWN, 0, sizeof(_strDriveWWN));
            strncpy(_strDriveWWN, (char *)buf + 4, sizeof(_strDriveWWN) - 1);
            ret = eRet_Ok;

            /* get model number */
            {
                unsigned char inq_cmd[] = { INQUIRY, 0, 0, 0, sizeof(buf), 0 };
                io_hdr.cmdp    = inq_cmd;
                io_hdr.cmd_len = sizeof(inq_cmd);

                if (0 == ioctl(fd, SG_IO, &io_hdr))
                {
                    /* parse model number */
                    strncpy(_strDriveModel, (char *)buf + 8, 7);
                    strncat(_strDriveModel, " - ", 3);
                    strncat(_strDriveModel, (char *)buf + 16, 15);

                    /*
                     * revision level
                     * strncat(_strDriveModel, " - ", 3);
                     * strncat(_strDriveModel, (char * )buf + 32, 3);
                     */

                    /*
                     * vendor specific (sometimes serial)
                     * strncat(_strDriveModel, " - ", 3);
                     * strncat(_strDriveModel, (char * )buf + 36, 19);
                     */
                }
            }
            ret = eRet_Ok;
        }
        else
        {
            BDBG_WRN(("unable to get drive identity"));
        }
    }
    else
    {
        BDBG_WRN(("unable to open %s", str));
    }

error:
    if (0 <= fd)
    {
        close(fd);
        fd = 0;
    }
    return(ret);
} /* initializeDriveData */

/* read mount/drive data for given mount */
eRet CMountData::initializeMountData(const char * str)
{
    eRet ret = eRet_Ok;
    char strPartition[16];

    BDBG_ASSERT(NULL != str);

    /* coverity[secure_coding] */
    sscanf(str, "%15s %31s %15s", strPartition, _strMountPoint, _strMountType);
    _nPartition = atoi(strPartition + 8);
    strncpy(_strDrive, strPartition, 8);
    _strDrive[8] = '\0';

    /* get drive id */
    ret = initializeDriveData(_strDrive);
    CHECK_WARN_GOTO("unable to get drive data", ret, error);

error:
    return(ret);
} /* initializeMountData */

/* read partition/drive data for given partition */
eRet CMountData::initializePartitionData(const char * str)
{
    eRet ret              = eRet_Ok;
    int  nMajor           = 0;
    int  nMinor           = 0;
    int  nSize            = 0;
    char strPartition[16] = "/dev/";

    BDBG_ASSERT(NULL != str);
    /* coverity[secure_coding] */
    sscanf(str, "%16d %16d %32d %10s", &nMajor, &nMinor, &nSize, strPartition + 5);

    if (4 > strlen(strPartition))
    {
        ret = eRet_NotAvailable;
        goto error;
    }

    _nPartition = atoi(strPartition + 8);
    strncpy(_strDrive, strPartition, 8);
    _strDrive[8] = '\0';

    memset(_strMountPoint, 0, sizeof(_strMountPoint));
    memset(_strMountType, 0, sizeof(_strMountType));

    /* get drive id */
    ret = initializeDriveData(_strDrive);
    CHECK_WARN_GOTO("unable to get drive data", ret, error);

error:
    return(ret);
} /* initializePartitionData */

void CMountData::setDriveName(const char * str)
{
    BDBG_ASSERT(NULL != str);
    memset(_strDrive, 0, sizeof(_strDrive));
    strncpy(_strDrive, str, sizeof(_strDrive) - 1);
}

eRet CMountData::mount(bool bMount)
{
    eRet ret    = eRet_Ok;
    int  retSys = 0;

    if ((0 == strlen(_strDrive)) ||
        (0 == _nPartition) ||
        (0 == strlen(_strMountPoint)) ||
        (0 == strlen(_strMountType)))
    {
        ret = eRet_InvalidParameter;
        goto error;
    }

    if (false == bMount)
    {
        MString strCmd = "umount ";

        /* unmount */
        strCmd += _strMountPoint;
        retSys  = system(strCmd.s());
        BDBG_MSG(("system command:%s", strCmd.s()));
        if (0 != retSys)
        {
            BDBG_WRN(("unmount failed:"));
            dump(true);
            ret = eRet_ExternalError;
        }

        BDBG_WRN(("unmount:"));
        dump(true);
    }
    else
    {
        MString strCmd = "mount ";

        /* mount */
        strCmd += "-t ";
        strCmd += _strMountType;
        strCmd += " ";
        strCmd += _strDrive;
        strCmd += MString(_nPartition);
        strCmd += " ";
        strCmd += _strMountPoint;
        retSys  = system(strCmd.s());
        BDBG_MSG(("system command:%s", strCmd.s()));
        if (0 != retSys)
        {
            BDBG_WRN(("mount failed:"));
            dump(true);
            ret = eRet_ExternalError;
        }

        BDBG_WRN(("mount:"));
        dump(true);
    }

error:
    return(ret);
} /* mount */

/* we purposely do not compare _strDrive as this can change and does not
 * uniquely identify a given partition on a drive anyways */
bool CMountData::operator ==(CMountData &other)
{
    if (0 == strncmp(other._strDriveWWN, _strDriveWWN, sizeof(_strDriveWWN)))
    {
        /* world wide name matches */
        if (other._nPartition == _nPartition)
        {
            /* partition number matches */
            return(true);
        }
    }

    return(false);
}

CPower::CPower(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_power, pCfg),
    _pPmLib(NULL)
{
#if POWERSTANDBY_SUPPORT
    _pPmLib = brcm_pm_init();
#endif

    memset(&_settings, 0, sizeof(_settings));

    NEXUS_Platform_GetStandbySettings(&_settings);
}

CPower::~CPower()
{
#if POWERSTANDBY_SUPPORT
    brcm_pm_close(_pPmLib);
    _pPmLib = NULL;
#endif
}

eRet CPower::enableMoca(bool bEnable)
{
    eRet ret = eRet_Ok;

#if MOCA_SUPPORT
    {
        int retSys = 0;

        if (true == bEnable)
        {
            retSys = system("mocactl start");
            CHECK_PMLIB_ERROR("mocactl start failure", retSys);
            retSys = system("ifup eth1");
            CHECK_PMLIB_ERROR("ifup eth1 failure", retSys);
        }
        else
        {
            retSys = system("ifdown eth1");
            CHECK_PMLIB_ERROR("ifdown eth1 failure", retSys);
            retSys = system("mocactl stop");
            CHECK_PMLIB_ERROR("mocactl stop failure", retSys);
        }
    }
#else /* if MOCA_SUPPORT */
    BSTD_UNUSED(bEnable);
#endif /* if MOCA_SUPPORT */

    return(ret);
} /* enableMoca */

eRet CPower::enableEthernet(bool bEnable)
{
    eRet ret    = eRet_Ok;
    int  retSys = 0;

    if (true == bEnable)
    {
        retSys = system("ifup eth0");
        CHECK_PMLIB_ERROR("ifup eth0 failure", retSys);
    }
    else
    {
        retSys = system("ifdown eth0");
        CHECK_PMLIB_ERROR("ifdown eth0 failure", retSys);
    }

    return(ret);
} /* enableEthernet */

eRet CPower::mountDrives(bool bMount)
{
    eRet   ret     = eRet_Ok;
    FILE * fMounts = NULL;
    char   strLine[80];

    if (false == bMount)
    {
        /* unmount drives */
        BDBG_MSG(("UNMOUNT DRIVES..."));

        /* find mount points */
        fMounts = fopen("/proc/mounts", "r");
        CHECK_PTR_ERROR_GOTO("unable to find mount points", fMounts, ret, eRet_NotAvailable, error);

        while (NULL != fgets(strLine, sizeof(strLine), fMounts))
        {
            CMountData * pMount = NULL;

            if ((0 != strncmp("/dev/sd", strLine, 6)) &&
                (0 != strncmp("/dev/hd", strLine, 6)))
            {
                /* only unmount ide/sata/usb drives */
                BDBG_MSG(("skip: %s", strLine));
                continue;
            }

            /* create CMountData to remember existing mount information */
            pMount = new CMountData;
            CHECK_PTR_ERROR_GOTO("unable to allocate mount data", pMount, ret, eRet_OutOfMemory, error);

            ret = pMount->initializeMountData(strLine);
            if (eRet_Ok != ret)
            {
                DEL(pMount);
            }
            CHECK_WARN_GOTO("unable to init mount data", ret, error);

            {
                /* unmount mount point */
                ret = pMount->mount(false);
                if (ret == eRet_Ok)
                {
                    /* save mount point information so we can remount later */
                    BDBG_MSG(("saving mount point:"));
                    pMount->dump(false);
                    _mountList.add(pMount);
                }
                else
                {
                    DEL(pMount);
                }
            }
        }
    }
    else
    {
        CMountData * pMount = NULL;

        if (0 == _mountList.total())
        {
            BDBG_WRN(("No saved partitions to mount"));
            goto done;
        }

        BDBG_MSG(("MOUNT DRIVES..."));

        /* mount drives by going through list of available partitions and
         * remounting matches in our mount list */
        fMounts = fopen("/proc/partitions", "r");
        CHECK_PTR_ERROR_GOTO("unable to find partitions", fMounts, ret, eRet_NotAvailable, error);

        while (NULL != fgets(strLine, sizeof(strLine), fMounts))
        {
            CMountData mountData;

            if ((NULL == strstr(strLine, "sd")) &&
                (NULL == strstr(strLine, "hd")))
            {
                /* only mount ide/sata/usb drives */
                BDBG_MSG(("skip partition: %s", strLine));
                continue;
            }

            if (eRet_Ok == mountData.initializePartitionData(strLine))
            {
                /* look for matching drive and mount point in our saved list */
                for (pMount = _mountList.first(); pMount; pMount = _mountList.next())
                {
                    /* we rely on the drive WWN number to determine which drive is
                     * mapped to which set of partitions.  the "==" operator compares
                     * WWN number and partition number. */
                    if (mountData == *pMount)
                    {
                        /* partition name may have changed after powering down/up,
                         * so update it before mounting. */
                        pMount->setDriveName(mountData.getDriveName());

                        /* mount partition */
                        ret = pMount->mount(true);
                        CHECK_ERROR_GOTO("unable to mount drive", ret, error);

                        _mountList.remove(pMount);
                        DEL(pMount);
                    }
                }
            }
        }

        /* only success if our mountlist is empty */
        ret = (0 == _mountList.total()) ? eRet_Ok : eRet_NotAvailable;
    }

error:
done:
    if (NULL != fMounts)
    {
        fclose(fMounts);
        fMounts = NULL;
    }
    return(ret);
} /* mountDrives */

eRet CPower::mountDrives(
        bool     bMount,
        uint32_t nRetries
        )
{
    uint32_t nAttempts = nRetries;
    eRet     ret       = eRet_Ok;

    /* make multiple attempts to mount/unmount drives.  it may take an indeterminate
     * amount of time for the drives to power back up and become available so
     * we will poll for a little while based on current power state */
    do
    {
        ret = mountDrives(bMount);
        if (eRet_Ok == ret)
        {
            /* successfully mounted/unmounted drives */
            BDBG_WRN(("Drive partitions %s successfully", (bMount ? "mounted" : "UNmounted")));
            break;
        }

        nAttempts--;
        BDBG_WRN(("Unable to %s drives - retries left:%d", (bMount ? "mounted" : "UNmounted"), nAttempts));
        BKNI_Sleep(1000);
    }
    while (0 < nAttempts);

    return(ret);
} /* mountDrives */

/* return true if transition from modeOld to modeNew requires setting linux power settings */
bool CPower::doLinuxPower(
        ePowerMode modeOld,
        ePowerMode modeNew
        )
{
    /* modeOld x modeNew array */
    bool transition[ePowerMode_Max][ePowerMode_Max] =
    {
        { false, false, true,  true  },
        { false, false, true,  true  },
        { true,  true,  false, false },
        { true,  true,  false, false }
    };

    return(transition[modeOld][modeNew]);
}

/* return true if transition from modeOld to modeNew requires mounting hard drives */
bool CPower::doDriveMount(
        ePowerMode modeOld,
        ePowerMode modeNew
        )
{
    /* modeOld x modeNew array */
    bool transition[ePowerMode_Max][ePowerMode_Max] =
    {
        { false, false, false, false },
        { false, false, false, false },
        { true,  true,  false, false },
        { true,  true,  false, false }
    };

    return(transition[modeOld][modeNew]);
}

/* return true if transition from modeOld to modeNew requires UNmounting hard drives */
bool CPower::doDriveUnmount(
        ePowerMode modeOld,
        ePowerMode modeNew
        )
{
    /* modeOld x modeNew array */
    bool transition[ePowerMode_Max][ePowerMode_Max] =
    {
        { false, false, true,  true  },
        { false, false, true,  true  },
        { false, false, false, false },
        { false, false, false, false }
    };

    return(transition[modeOld][modeNew]);
}

eRet CPower::setMode(
        ePowerMode  mode,
        CGraphics * pGraphics
        )
{
    eRet       ret     = eRet_Ok;
    ePowerMode modeOld = getMode();

    BDBG_MSG(("CPower::%s set power mode:S%d", __FUNCTION__, mode));
    /* notifiy observers first before graphics are shut off */
    notifyObservers(eNotify_PowerModeChanged, &mode);

    if (mode == modeOld)
    {
        BDBG_MSG(("power mode unchanged so return"));
        return(ret);
    }

    NEXUS_Platform_GetStandbySettings(&_settings);
    _settings.mode               = (NEXUS_PlatformStandbyMode)mode; /* direct mapping */
    _settings.openFrontend       = (ePowerMode_S0 == mode) ? true : false;
    _settings.wakeupSettings.ir  = true;
    _settings.wakeupSettings.uhf = true;

#if POWERSTANDBY_SUPPORT
    {
        NEXUS_Error   nerror = NEXUS_SUCCESS;
        int           powerModeToPmStandbyState[ePowerMode_Max] = { 0, 0, BRCM_PM_STANDBY, BRCM_PM_SUSPEND };
        int           retPm          = 0;
        bool          bStandby       = (ePowerMode_S0 != mode) ? true : false;
        int           pmStandbyState = 0;
        brcm_pm_state pmState;

        _settings.wakeupSettings.transport = true;

        BDBG_WRN(("wakeup settings:"));
        BDBG_WRN(("  mode:%d", _settings.mode));
        BDBG_WRN(("  ir::%d", _settings.wakeupSettings.ir));
        BDBG_WRN(("  uhf::%d", _settings.wakeupSettings.uhf));
        BDBG_WRN(("  keypad::%d", _settings.wakeupSettings.keypad));
        BDBG_WRN(("  gpio::%d", _settings.wakeupSettings.gpio));
        BDBG_WRN(("  nmi::%d", _settings.wakeupSettings.nmi));
        BDBG_WRN(("  cec::%d", _settings.wakeupSettings.cec));
        BDBG_WRN(("  transport::%d", _settings.wakeupSettings.transport));
        BDBG_WRN(("  timeout::%d", _settings.wakeupSettings.timeout));
        BDBG_WRN(("  open FE::%d", _settings.openFrontend));

        if ((NULL != pGraphics) && (ePowerMode_S0 != mode))
        {
            /* disable graphics before turning off (S1, S2, S3) */
            pGraphics->setActive(false);
        }

        nerror = NEXUS_Platform_SetStandbySettings(&_settings);
        CHECK_NEXUS_ERROR_GOTO("unable to set standby mode", ret, nerror, error);

        /* set linux power mode */
        if (true == doLinuxPower(modeOld, mode))
        {
            BDBG_MSG(("setting LINUX POWER"));

            brcm_pm_get_status(_pPmLib, &pmState);

            if (BRCM_PM_UNDEF != pmState.sata_status)
            {
                pmState.sata_status = bStandby ? !GET_BOOL(_pCfg, POWER_MGMT_SATA) : true;
            }
            /* if CPUFREQ_AVAIL_MAXLEN is defined, then pmState is 3.14 version, which does not have these members */
#ifndef CPUFREQ_AVAIL_MAXLEN
            if (BRCM_PM_UNDEF != pmState.usb_status)
            {
                pmState.usb_status = bStandby ? !GET_BOOL(_pCfg, POWER_MGMT_USB) : true;
            }
            if (BRCM_PM_UNDEF != pmState.cpu_divisor)
            {
                pmState.cpu_divisor = bStandby ? GET_INT(_pCfg, POWER_MGMT_CPU_DIVISOR) : 1;
            }
#endif /* ifndef CPUFREQ_AVAIL_MAXLEN */
            if (BRCM_PM_UNDEF != pmState.tp1_status)
            {
                pmState.tp1_status = bStandby ? !GET_BOOL(_pCfg, POWER_MGMT_TP1) : true;
            }
            if (BRCM_PM_UNDEF != pmState.tp2_status)
            {
                pmState.tp2_status = bStandby ? !GET_BOOL(_pCfg, POWER_MGMT_TP2) : true;
            }
            if (BRCM_PM_UNDEF != pmState.tp3_status)
            {
                pmState.tp3_status = bStandby ? !GET_BOOL(_pCfg, POWER_MGMT_TP3) : true;
            }
#ifdef CPUFREQ_AVAIL_MAXLEN
            if (BRCM_PM_UNDEF != pmState.srpd_status)
            {
                pmState.srpd_status = bStandby ? !GET_BOOL(_pCfg, POWER_MGMT_DDR) : false;
            }
#else /* ifdef CPUFREQ_AVAIL_MAXLEN */
            if (BRCM_PM_UNDEF != pmState.ddr_timeout)
            {
                pmState.ddr_timeout = bStandby ? !GET_BOOL(_pCfg, POWER_MGMT_DDR) : false;
            }
#endif /* ifdef CPUFREQ_AVAIL_MAXLEN */
#if MEMC1_SUPPORT
#ifndef CPUFREQ_AVAIL_MAXLEN
            if (BRCM_PM_UNDEF != pmState.memc1_status)
            {
                pmState.memc1_status = bStandby ? !GET_BOOL(_pCfg, POWER_MGMT_MEMC1) : true;
                BDBG_WRN(("MEMC1:%d", pmState.memc1_status));
            }
#endif /* ifndef CPUFREQ_AVAIL_MAXLEN */
#endif /* if MEMC1_SUPPORT */
            BDBG_WRN(("PMLIB status:"));
            BDBG_WRN(("     sata:%d", pmState.sata_status));
#ifndef CPUFREQ_AVAIL_MAXLEN
            BDBG_WRN(("     usb:%d", pmState.usb_status));
            BDBG_WRN(("     cpu divisor:%d", pmState.cpu_divisor));
#endif
            BDBG_WRN(("     tp1:%d", pmState.tp1_status));
            BDBG_WRN(("     tp2:%d", pmState.tp2_status));
            BDBG_WRN(("     tp3:%d", pmState.tp3_status));
#ifndef CPUFREQ_AVAIL_MAXLEN
            BDBG_WRN(("     ddr timeout:%d", pmState.ddr_timeout));
            BDBG_WRN(("     memc1:%d", pmState.memc1_status));
#endif

            if (true == doDriveUnmount(modeOld, mode))
            {
                /* unmount currently mounted partitions */
                ret = mountDrives(false, 10);
                CHECK_ERROR("unable to unmount drives", ret);
            }

            retPm = brcm_pm_set_status(_pPmLib, &pmState);
            CHECK_PMLIB_ERROR_GOTO("unable to set power management status", ret, retPm, error);

            if (true == doDriveMount(modeOld, mode))
            {
                /* mount previously saved partitions */
                ret = mountDrives(true, 10);
                CHECK_ERROR_GOTO("unable to mount drives", ret, error);
            }

            if (true == GET_BOOL(_pCfg, POWER_MGMT_ENET))
            {
                BDBG_WRN(("ENET:%d", !bStandby));
                enableEthernet(bStandby ? false : true);
            }
#if MOCA_SUPPORT
            if (true == GET_BOOL(_pCfg, POWER_MGMT_MOCA))
            {
                BDBG_WRN(("MOCA:%d", !bStandby));
                enableMoca(bStandby ? false : true);
            }
#endif /* if MOCA_SUPPORT */

            pmStandbyState = powerModeToPmStandbyState[mode];
            if (0 < pmStandbyState)
            {
                BDBG_MSG(("setting pm standby state:%d", pmStandbyState));
                retPm = brcm_pm_suspend(_pPmLib, pmStandbyState);
                CHECK_PMLIB_ERROR_GOTO("unable to set power management suspend state", ret, retPm, error);
            }
        }

        if ((NULL != pGraphics) && (ePowerMode_S0 == mode))
        {
            /* re-enable graphics after turning on (S0) */
            pGraphics->setActive(true);
        }
    }

    SET(_pCfg, POWER_STATE, MString(mode));
error:
#else /* if POWERSTANDBY_SUPPORT */
    BSTD_UNUSED(pGraphics);
#endif /* if POWERSTANDBY_SUPPORT */
    return(ret);
} /* setMode */