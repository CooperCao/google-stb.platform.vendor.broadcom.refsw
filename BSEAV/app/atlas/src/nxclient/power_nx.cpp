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
#include "power_nx.h"
#include "graphics_nx.h"
#include "graphics.h"
#include "atlas_cfg.h"
#include "nexus_platform_standby.h"

#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <fcntl.h>
#include <scsi/scsi.h>
#include <scsi/sg.h>

BDBG_MODULE(atlas_powernx);

CPowerNx::CPowerNx(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CPower(name, number, pCfg),
    _transition(false)
{
}

CPowerNx::~CPowerNx(void)
{
}

/* Returns true if we are going to S0 mode */
bool CPowerNx::checkPower(void)
{
    NEXUS_Error            rc;
    NxClient_StandbyStatus standbyStatus;
    bool                   powerOn = false;

#if POWERSTANDBY_SUPPORT
    rc = NxClient_GetStandbyStatus(&standbyStatus);

    BKNI_Sleep(100);
    if ((_transition == false) && (_eMode != ePowerMode_S0) && (standbyStatus.settings.mode == NEXUS_PlatformStandbyMode_eOn))
    {
        BDBG_MSG(("NxClient PowerManagement is changing from S%d changing to S0 mode.", _eMode));
        powerOn = true;
    }
#else /* if POWERSTANDBY_SUPPORT */
    BSTD_UNUSED(rc);
    BSTD_UNUSED(standbyStatus);
#endif /* if POWERSTANDBY_SUPPORT */
    return(powerOn);
} /* checkPower */

eRet CPowerNx::setMode(
        ePowerMode  mode,
        CGraphics * pGraphicsX
        )
{
    NxClient_StandbyStatus   standbyStatus;
    NxClient_StandbySettings standbySettings;
    NEXUS_Error              nerror    = NEXUS_SUCCESS;
    eRet                     ret       = eRet_Ok;
    ePowerMode               modeOld   = getMode();
    CGraphicsNx *            pGraphics = (CGraphicsNx *) pGraphicsX;

    BDBG_MSG(("CPowerNx %s set power mode:%s", BSTD_FUNCTION, (powerModeToString(mode)).s()));

    if (mode == modeOld)
    {
        BDBG_MSG(("power mode unchanged so return"));
        return(ret);
    }

    if (ePowerMode_S0 < mode)
    {
        /* disable bwidget events if going to standby/sleep */
        SET(_pCfg, POWER_STATE, MString(mode));
    }

#if POWERSTANDBY_SUPPORT
    _transition = true;
    nerror      = NxClient_GetStandbyStatus(&standbyStatus);
    if (nerror) { BERR_TRACE(nerror); return(ret); }

    BDBG_MSG(("Wake up Status:\n"
              "IR      : %d\n"
              "UHF     : %d\n"
              "XPT     : %d\n"
              "CEC     : %d\n"
              "GPIO    : %d\n"
              "KPD     : %d\n"
              "Timeout : %d\n"
              "\n",
              standbyStatus.status.wakeupStatus.ir,
              standbyStatus.status.wakeupStatus.uhf,
              standbyStatus.status.wakeupStatus.transport,
              standbyStatus.status.wakeupStatus.cec,
              standbyStatus.status.wakeupStatus.gpio,
              standbyStatus.status.wakeupStatus.keypad,
              standbyStatus.status.wakeupStatus.timeout));

    {
        int           powerModeToPmStandbyState[ePowerMode_Max] = { 0, 0, BRCM_PM_STANDBY, BRCM_PM_SUSPEND };
        int           retPm          = 0;
        bool          bStandby       = (ePowerMode_S0 != mode) ? true : false;
        int           pmStandbyState = 0;
        int           cnt            = 15; /* the amount of times to check if PM is done. */
        brcm_pm_state pmState;

        BDBG_MSG(("Entering StandBy Mode S%d\n", mode));
        NxClient_GetDefaultStandbySettings(&standbySettings);
        _eMode                                      = mode;
        standbySettings.settings.mode               = (NEXUS_PlatformStandbyMode) mode;
        standbySettings.settings.wakeupSettings.ir  = true;
        standbySettings.settings.wakeupSettings.uhf = true;
        standbySettings.settings.openFrontend       = (ePowerMode_S0 == mode) ? true : false;
        BDBG_MSG(("Wake up Settings:\n"
                  "IR       : %d\n"
                  "UHF      : %d\n"
                  "XPT      : %d\n"
                  "CEC      : %d\n"
                  "GPIO     : %d\n"
                  "KPD      : %d\n"
                  "Timeout  : %d\n"
                  "Frontend : %d\n"
                  "\n",
                  standbySettings.settings.wakeupSettings.ir,
                  standbySettings.settings.wakeupSettings.uhf,
                  standbySettings.settings.wakeupSettings.transport,
                  standbySettings.settings.wakeupSettings.cec,
                  standbySettings.settings.wakeupSettings.gpio,
                  standbySettings.settings.wakeupSettings.keypad,
                  standbySettings.settings.wakeupSettings.timeout,
                  standbySettings.settings.openFrontend));

        nerror = NxClient_SetStandbySettings(&standbySettings);
        if (nerror)
        {
            BERR_TRACE(nerror);
            goto error;
        }

        if ((NULL != pGraphics) && (ePowerMode_S0 != mode))
        {
            /* disable graphics*/
            pGraphics->setActive(false);
        }

        /*
         * Wait for nexus to enter standby , disable/enable bwin callbacks
         * notifiy observers that we are changing Mode
         */
        notifyObservers(eNotify_PowerModeChanged, &mode);
        SET(_pCfg, POWER_STATE, MString(mode));

        /* limit the amount of loops to 15 for NxServer to respond */
        do
        {
            BKNI_Sleep(1000);
            cnt--;
            nerror = NxClient_GetStandbyStatus(&standbyStatus);
            if (nerror) { BERR_TRACE(nerror); goto error; }
            if (standbyStatus.transition == NxClient_StandbyTransition_eAckNeeded)
            {
                BDBG_ERR(("'Atlas' acknowledges standby state: %s\n", powerModeToString(_eMode).s()));
                NxClient_AcknowledgeStandby(true);
            }

            if (cnt == 0)
            {
                BDBG_ERR(("StandbyTransition Failed!"));
                goto error;
            }
        }
        while ((standbyStatus.transition != NxClient_StandbyTransition_eDone) && mode != ePowerMode_S0);

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
        /* We resume from S2/S3 Right here */

        /* We need to turn on NxServer after we wake up. Just wake up NxServer. Control onIdle()
         * will wake up Atlas */
        if (_eMode != ePowerMode_S0)
        {
            BDBG_MSG(("We are waking up from S%d!\n Wake up NxServer!!", _eMode));
            standbySettings.settings.mode = NEXUS_PlatformStandbyMode_eOn;
            nerror                        = NxClient_SetStandbySettings(&standbySettings);
            /* if we cannot wake up then we need to ASSERT because there is a
             * fundamental issue with Atlas and NxServer that needs to be fixed
             * ASAP */
            BDBG_ASSERT(!nerror);
        }

        if ((NULL != pGraphics) && (ePowerMode_S0 == mode))
        {
            /* re-enable graphics after turning on (S0) */
            pGraphics->setActive(true);
        }

        if (ePowerMode_S0 == mode)
        {
            /* enable bwidget events if going to ON mode */
            SET(_pCfg, POWER_STATE, MString(mode));
        }

        /* transition is done. we need to protect this because it is like isr code */
        _transition = false;
    }

error:
#else /* if POWERSTANDBY_SUPPORT */
    BSTD_UNUSED(pGraphics);
#endif /* if POWERSTANDBY_SUPPORT */
    return(ret);
} /* setMode */