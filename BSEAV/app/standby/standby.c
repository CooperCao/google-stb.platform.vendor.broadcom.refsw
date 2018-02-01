/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 *****************************************************************************/

#include "standby.h"
#include "cmdline.h"
#include "pmlib.h"
#include "util.h"

#ifdef WOWLAN
#include "unistd.h"
#include "fcntl.h"
#endif

BDBG_MODULE(standby);

extern B_StandbyNexusHandles g_StandbyNexusHandles;
extern B_DeviceState g_DeviceState;
extern B_CmdOptions g_cmd_options;
BKNI_MutexHandle mutex;

pthread_t cmd_thread;

static struct {
    void *brcm_pm_ctx;
    pmlib_state_t g_state;
} g_power_state;


extern struct if_info eth_info, moca_info;
extern unsigned mocaVer;

NEXUS_TransportWakeupFilter Filter[18] =
{
    { 0x47, 0xFF, 1 },
    { 0x12, 0xFF, 1 },
    { 0x34, 0xFF, 1 },
    { 0x15, 0xFF, 1 },

    { 0x12, 0xFF, 2 },
    { 0x34, 0xFF, 2 },
    { 0x03, 0xFF, 2 },
    { 0x46, 0xFF, 2 },
    { 0x66, 0xFF, 2 },

    { 0x4F, 0xFF, 3 },
    { 0x31, 0xFF, 3 },
    { 0x00, 0xFF, 3 },

    { 0x88, 0xFF, 2 },
    { 0x77, 0xFF, 2 },
    { 0x66, 0xFF, 2 },
    { 0x55, 0xFF, 2 },
    { 0x44, 0xFF, 2 },
    { 0x33, 0xFF, 2 }
};


#ifdef WOWLAN
int wakeonwlan = 0;
#endif
void get_pmlib_state(pmlib_state_t *state)
{
    *state = g_power_state.g_state;
}

int set_pmlib_state(const pmlib_state_t *state)
{
    struct brcm_pm_state pmlib_state;
    BERR_Code rc;
    char buf[256];

    rc = brcm_pm_get_status(g_power_state.brcm_pm_ctx, &pmlib_state);
    if(rc) {printf("Cant get PM status\n");}


    pmlib_state.sata_status = state->sata;
    pmlib_state.tp1_status = state->tp1;
    pmlib_state.tp2_status = state->tp2;
    pmlib_state.tp3_status = state->tp3;
#if PMLIB_VER == 314
    pmlib_state.srpd_status = state->ddr?0:64;
#elif PMLIB_VER == 26
    pmlib_state.ddr_timeout = state->ddr?0:64;
    pmlib_state.usb_status = state->usb;
#if MEMC1_SUPPORT
    pmlib_state.memc1_status = state->memc1;
#else
    pmlib_state.memc1_status = BRCM_PM_UNDEF;
#endif
    pmlib_state.standby_flags = state->flags;
        /* 2.6.37-2.4 Kernel has some issue while resuming from S2 standby mode. So it requires
     * some delay to added while resuming. Changing the flag to 0x4 makes sure that this
     * delay is added. Needs to be removed once the Kernel fix is available.
     */
#if (BCHP_CHIP == 7358)
    pmlib_state.standby_flags |=  0x4;
#endif
#endif

    rc = brcm_pm_set_status(g_power_state.brcm_pm_ctx, &pmlib_state);

    if(state->enet) {
        if(!g_power_state.g_state.enet) {
            snprintf(buf, 256, "ifup %s", eth_info.if_name);
            system(buf);
        }
    } else {
        if(g_power_state.g_state.enet) {
            snprintf(buf, 256, "ifdown %s", eth_info.if_name);
            system(buf);
        }
    }

    if(state->moca) {
        if(!g_power_state.g_state.moca){
            if(mocaVer==1)
                system("mocactl start");
            else if(mocaVer == 2)
                system("mocap set --start");
            snprintf(buf, 256, "ifup %s", moca_info.if_name);
            system(buf);
        }
    } else {
        if(g_power_state.g_state.moca){
            snprintf(buf, 256, "ifdown %s", moca_info.if_name);
            system(buf);
            if(mocaVer==1)
                system("mocactl stop");
            else if(mocaVer == 2)
                system("mocap set --stop");
        }
    }

    g_power_state.g_state = *state;

    return 0;
}

void prepareForStandby(void)
{
    unsigned id;

    encode_stop(0);

    for (id=0; id<MAX_CONTEXTS; id++) {
        if(g_DeviceState.power_mode != ePowerModeS1 || !g_DeviceState.record_started[id]) {
#if NEXUS_HAS_FRONTEND
            untune_frontend(id);
#endif
            playback_stop(id);
            record_stop(id);
        }
        decode_stop(id);
    }

    if(g_DeviceState.power_mode != ePowerModeS1 || !g_DeviceState.record_started[0] || !g_DeviceState.record_started[1])
        umount_all();
}



void postResume(void)
{
    NEXUS_Error rc;
    unsigned id;

    /* Moca might have been turned off in standby Turn it back on again.*/
    if(moca_info.exists) {
        pmlib_state_t pmlib_state;
        get_pmlib_state(&pmlib_state);
        pmlib_state.moca = true;
        set_pmlib_state(&pmlib_state);
    }

    for (id=0; id<MAX_CONTEXTS; id++) {
        if(g_DeviceState.source[id] != eInputSourceNone) {
            rc = decode_start(id);
            if(rc) { BERR_TRACE(rc); BDBG_ASSERT(!rc); }

            switch (g_DeviceState.source[id]) {
#if NEXUS_HAS_FRONTEND
                case eInputSourceQam:
                    rc = tune_qam(id);
                    if(rc) { BERR_TRACE(rc); BDBG_ASSERT(!rc); }
                    break;
                case eInputSourceSat:
                    rc = tune_sat(id);
                    if(rc) { BERR_TRACE(rc); BDBG_ASSERT(!rc); }
                    break;
                case eInputSourceOfdm:
                    rc = tune_ofdm(id);
                    if(rc) { BERR_TRACE(rc); BDBG_ASSERT(!rc); }
                    break;
#endif
                case eInputSourceStreamer:
                    rc = streamer_start(id);
                    if(rc) { BERR_TRACE(rc); BDBG_ASSERT(!rc); }
                    break;
                case eInputSourceFile:
                    mount_all(); /* For playback we need to mount first */
                    rc = playback_start(id);
                    if(rc) { BERR_TRACE(rc); BDBG_ASSERT(!rc); }
                    break;
                default:
                    break;
            }
        }
    }

    /* Uninitialize SAGE system platform */

    /* For everything else we can mount after decode has started */
    mount_all();
}

static NEXUS_Error setStandbySettings(NEXUS_PlatformStandbySettings *standbySettings)
{
    NEXUS_Error rc;
    unsigned i;

    for(i=0;i<10;i++) { /* try for 1 second (10 x 100 msec) */
        standbySettings->timeout = 100;
        rc = NEXUS_Platform_SetStandbySettings(standbySettings);
        if(rc == NEXUS_TIMEOUT) {
            BDBG_WRN(("Timeout on SetStandbySettings, wait and try again"));
            BKNI_ReleaseMutex(mutex);
            BKNI_Sleep(100);
            BKNI_AcquireMutex(mutex);
        } else {
            break;
        }
    }

    return rc;
}

/**
 * Enter S1 standby
 **/
int activeStandbyMode(void)
{
    NEXUS_Error rc;
    NEXUS_TransportWakeupSettings xptWakeupSettings;
    NEXUS_PlatformStandbySettings nexusStandbySettings;
    pmlib_state_t pmlib_state;
    unsigned timeout = g_DeviceState.timer_wake?g_cmd_options.timeout*1000:0xFFFFFFFF;

    printf("\n\nEntering S1 mode\n\n");

    BKNI_AcquireMutex(mutex);

    prepareForStandby();

    /* Don't enable XPT wakeup in auto mode. Auto mode transitions randomly between all
       states and nexus modules could be locked. If we need to set XPT wakeup between
       transitions then we need to enter S0 first */
    if(g_cmd_options.xpt_wakeup) {
        /* Set up XPT wakeup filter */
        NEXUS_TransportWakeup_GetSettings(&xptWakeupSettings);
        BKNI_Memcpy(xptWakeupSettings.filter[0].packet, Filter, sizeof(Filter));
        xptWakeupSettings.inputBand = g_StandbyNexusHandles.wakeupInputBand;
        xptWakeupSettings.packetLength = sizeof(Filter)/sizeof(Filter[0]);
        xptWakeupSettings.enabled = true;
        rc = NEXUS_TransportWakeup_SetSettings(&xptWakeupSettings);
        BDBG_ASSERT(!rc);
    }

    NEXUS_Platform_GetStandbySettings(&nexusStandbySettings);
    nexusStandbySettings.mode = NEXUS_PlatformStandbyMode_eActive;
    rc = setStandbySettings(&nexusStandbySettings);
    if(rc) { BERR_TRACE(rc); BDBG_ASSERT(!rc); }

    get_pmlib_state(&pmlib_state);
    pmlib_state.usb = false;
    pmlib_state.enet = g_cmd_options.ethoff?false:true;
    pmlib_state.moca = g_cmd_options.mocaoff?false:true;
    pmlib_state.sata = g_cmd_options.sataoff &&
                       !g_DeviceState.record_started[0] &&
                       !g_DeviceState.record_started[1] ? false:true;
    pmlib_state.tp1 = false;
    pmlib_state.tp2 = false;
    pmlib_state.tp3 = false;
    pmlib_state.cpu = false;
    pmlib_state.ddr = false;
    pmlib_state.memc1 = false;
    set_pmlib_state(&pmlib_state);

    BKNI_ResetEvent(g_StandbyNexusHandles.s1Event);

    rc = BKNI_WaitForEvent(g_StandbyNexusHandles.s1Event, timeout);

    get_pmlib_state(&pmlib_state);
    pmlib_state.usb = true;
    pmlib_state.enet = true;
    pmlib_state.moca = true;
    pmlib_state.sata = true;
    pmlib_state.tp1 = true;
    pmlib_state.tp2 = true;
    pmlib_state.tp3 = true;
    pmlib_state.cpu = true;
    pmlib_state.ddr = true;
#if MEMC1_SUPPORT
    pmlib_state.memc1 = true;
#else
    pmlib_state.memc1 = false;
#endif
    set_pmlib_state(&pmlib_state);

    if(g_cmd_options._auto) {
        wait_for_all_devices();
    } else {
        g_DeviceState.power_mode = ePowerModeS0;
        if(g_StandbyNexusHandles.event)
            BKNI_SetEvent(g_StandbyNexusHandles.event);
    }

    g_DeviceState.timer_wake = false;

    BKNI_ReleaseMutex(mutex);

    return 0;
}

/**
 * Enter S2 standby
 **/
int passiveStandbyMode(void)
{
#ifdef WOWLAN
    int fd;
    char wakeup_count;
#endif
    NEXUS_Error rc;
    NEXUS_TransportWakeupSettings xptWakeupSettings;
    NEXUS_PlatformStandbySettings nexusStandbySettings;
    NEXUS_PlatformStandbyStatus nexusStandbyStatus;
    unsigned timeout = g_DeviceState.timer_wake?g_cmd_options.timeout:0;
    uint32_t wolopts;

    printf("\n\nEntering S2 Mode\n\n");

    /* Initialize SAGE system platform */

    BKNI_AcquireMutex(mutex);

    prepareForStandby();

    /* Don't enable XPT wakeup in auto mode. Auto mode transitions randomly between all
       states and nexus modules could be locked. If we need to set XPT wakeup between
       transitions then we need to enter S0 first */
    if(g_cmd_options.xpt_wakeup) {
        /* Set up XPT wakeup filter */
        NEXUS_TransportWakeup_GetSettings(&xptWakeupSettings);
        BKNI_Memcpy(xptWakeupSettings.filter[0].packet, Filter, sizeof(Filter));
        xptWakeupSettings.inputBand = g_StandbyNexusHandles.wakeupInputBand;
        xptWakeupSettings.packetLength = sizeof(Filter)/sizeof(Filter[0]);
        xptWakeupSettings.enabled = true;
        rc = NEXUS_TransportWakeup_SetSettings(&xptWakeupSettings);
        BDBG_ASSERT(!rc);
    }

    NEXUS_Platform_GetStandbySettings(&nexusStandbySettings);
    nexusStandbySettings.mode = NEXUS_PlatformStandbyMode_ePassive;
    nexusStandbySettings.wakeupSettings.ir = g_cmd_options.ir_wakeup;
    nexusStandbySettings.wakeupSettings.uhf = g_cmd_options.uhf_wakeup;
    nexusStandbySettings.wakeupSettings.transport = g_cmd_options.xpt_wakeup;
#if NEXUS_HAS_CEC
    nexusStandbySettings.wakeupSettings.cec = g_cmd_options.cec_wakeup;
#endif
    nexusStandbySettings.wakeupSettings.gpio = g_cmd_options.gpio_wakeup;
    nexusStandbySettings.wakeupSettings.keypad = g_cmd_options.kpd_wakeup;
    nexusStandbySettings.wakeupSettings.timeout = timeout;

    rc = setStandbySettings(&nexusStandbySettings);
    if(rc) { BERR_TRACE(rc); BDBG_ASSERT(!rc); }

    /* Set Ethernet WOL */
    getWol(eth_info.if_name, &wolopts);
    wolopts &= g_cmd_options.eth_wol_wakeup ? (WAKE_MAGIC|WAKE_ARP) : 0;
    setWol(eth_info.if_name, wolopts);

    /* Set MoCA WOL */
    if(moca_info.exists) {
        if (g_cmd_options.moca_wol_wakeup) {
            if(mocaVer == 1) {
                getWol(moca_info.if_name, &wolopts);
                wolopts &= (WAKE_MAGIC|WAKE_ARP);
                setWol(moca_info.if_name, wolopts);

                system("mocactl wol --enable");
            } else {
                char buf[256];
                system("mocap set --wom_mode 1");
                snprintf(buf, 256, "mocap set --wom_magic_mac val %s", moca_info.hw_addr);
                system(buf);
                system("mocap set --wom_magic_enable 1");
                system("mocap set --wom_pattern mask 0 0xff 15");
            }
        } else {
            pmlib_state_t pmlib_state;

            if(mocaVer == 1) {
                system("mocactl wol --disable");

                getWol(moca_info.if_name, &wolopts);
                wolopts &= 0;
                setWol(moca_info.if_name, wolopts);
            } else {
                system("mocap set --wom_mode 0");
                system("mocap set --wom_magic_enable 0");
            }

            /* Disable moca before entering S2, if MoCA WOL is not required */
            get_pmlib_state(&pmlib_state);
            pmlib_state.moca = false;
            set_pmlib_state(&pmlib_state);
        }
    }

    brcm_pm_suspend(g_power_state.brcm_pm_ctx, BRCM_PM_STANDBY);

    NEXUS_Platform_GetStandbyStatus(&nexusStandbyStatus);

#ifdef WOWLAN
    fd = open("/sys/devices/platform/rdb/f17e0000.wlan/power/wakeup_count", O_RDONLY);
    read(fd, &wakeup_count, 1);
    if (wakeup_count - '0'){
        printf("#####S2 Wake up due to WOWLAN#####\n");
        wakeonwlan=1;
    }
#endif

    printf("S2 Wake up Status\n"
            "IR      : %d\n"
            "UHF     : %d\n"
            "XPT     : %d\n"
            "CEC     : %d\n"
            "GPIO    : %d\n"
            "KPD     : %d\n"
            "Timeout : %d\n"
            "\n",
            nexusStandbyStatus.wakeupStatus.ir,
            nexusStandbyStatus.wakeupStatus.uhf,
            nexusStandbyStatus.wakeupStatus.transport,
            nexusStandbyStatus.wakeupStatus.cec,
            nexusStandbyStatus.wakeupStatus.gpio,
            nexusStandbyStatus.wakeupStatus.keypad,
            nexusStandbyStatus.wakeupStatus.timeout);

    if(g_cmd_options._auto) {
        wait_for_all_devices();
    } else {
        g_DeviceState.power_mode = ePowerModeS0;
        if(g_StandbyNexusHandles.event)
            BKNI_SetEvent(g_StandbyNexusHandles.event);
    }

    g_DeviceState.timer_wake = false;

    BKNI_ReleaseMutex(mutex);

    return 0;
}

/**
 * Enter S3 standby
 **/
int deepStandbyMode(void)
{
    NEXUS_Error rc;
    NEXUS_PlatformStandbySettings nexusStandbySettings;
    NEXUS_PlatformStandbyStatus nexusStandbyStatus;
    unsigned timeout = g_DeviceState.timer_wake?g_cmd_options.timeout:0;

    printf("\n\nEntering S3 Mode\n\n");

    BKNI_AcquireMutex(mutex);

    prepareForStandby();

    NEXUS_Platform_GetStandbySettings(&nexusStandbySettings);
    nexusStandbySettings.mode = NEXUS_PlatformStandbyMode_eDeepSleep;
    nexusStandbySettings.wakeupSettings.ir = g_cmd_options.ir_wakeup;
#if NEXUS_HAS_CEC
    nexusStandbySettings.wakeupSettings.cec = g_cmd_options.cec_wakeup;
#endif
    nexusStandbySettings.wakeupSettings.gpio = g_cmd_options.gpio_wakeup;
    nexusStandbySettings.wakeupSettings.keypad = g_cmd_options.kpd_wakeup;
    nexusStandbySettings.wakeupSettings.timeout = timeout;
    rc = setStandbySettings(&nexusStandbySettings);
    if(rc) { BERR_TRACE(rc); BDBG_ASSERT(!rc); }

    if(moca_info.exists) {
        /* Disable moca before entering S3 */
        pmlib_state_t pmlib_state;
        get_pmlib_state(&pmlib_state);
        pmlib_state.moca = false;
        set_pmlib_state(&pmlib_state);
    }

    printf("S3 Warm Boot\n\n");
    brcm_pm_suspend(g_power_state.brcm_pm_ctx, BRCM_PM_SUSPEND);

    NEXUS_Platform_GetStandbyStatus(&nexusStandbyStatus);

    printf("S3 Wake up Status\n"
            "IR      : %d\n"
            "CEC     : %d\n"
            "GPIO    : %d\n"
            "KPD     : %d\n"
            "Timeout : %d\n"
            "\n",
            nexusStandbyStatus.wakeupStatus.ir,
            nexusStandbyStatus.wakeupStatus.cec,
            nexusStandbyStatus.wakeupStatus.gpio,
            nexusStandbyStatus.wakeupStatus.keypad,
            nexusStandbyStatus.wakeupStatus.timeout);

    if(g_cmd_options._auto) {
        wait_for_all_devices();
    } else {
        g_DeviceState.power_mode = ePowerModeS0;
        if(g_StandbyNexusHandles.event)
            BKNI_SetEvent(g_StandbyNexusHandles.event);
    }

    g_DeviceState.timer_wake = false;

    BKNI_ReleaseMutex(mutex);

    return 0;
}

/**
 * Enter S5 standby
 **/
int haltMode(void)
{
    NEXUS_Error rc;
    NEXUS_PlatformStandbySettings nexusStandbySettings;
    unsigned timeout = g_DeviceState.timer_wake?g_cmd_options.timeout:0;

    printf("\n\nEntering S5 Mode\n\n");

    BKNI_AcquireMutex(mutex);

    prepareForStandby();

    NEXUS_Platform_GetStandbySettings(&nexusStandbySettings);
    nexusStandbySettings.mode = NEXUS_PlatformStandbyMode_eDeepSleep;
    nexusStandbySettings.wakeupSettings.ir = g_cmd_options.ir_wakeup;
#if NEXUS_HAS_CEC
    nexusStandbySettings.wakeupSettings.cec = g_cmd_options.cec_wakeup;
#endif
    nexusStandbySettings.wakeupSettings.gpio = g_cmd_options.gpio_wakeup;
    nexusStandbySettings.wakeupSettings.keypad = g_cmd_options.kpd_wakeup;
    nexusStandbySettings.wakeupSettings.timeout = timeout;
    rc = setStandbySettings(&nexusStandbySettings);
    if(rc) { BERR_TRACE(rc); BDBG_ASSERT(!rc); }

    BKNI_ReleaseMutex(mutex);

    printf("S3 Cold Boot\n\n");
#if LINUX_VER_GE_3_8
    system("poweroff");
#else
    system("echo 1 > /sys/devices/platform/brcmstb/halt_mode");
    system("halt");
#endif

    /* System reboots. Does not return back to this point */

    return 0;
}
#ifdef WOWLAN
void *load_file(const char *fn, unsigned *_sz)
{
    char *data;
    int sz;
    int fd;
    data = 0;
    fd = open(fn, O_RDONLY);
    if(fd < 0) return 0;
    sz = lseek(fd, 0, SEEK_END);
    if(sz < 0) goto oops;
    if(lseek(fd, 0, SEEK_SET) != 0) goto oops;
    data = (char*) malloc(sz);
    if(data == 0) goto oops;
    if(read(fd, data, sz) != sz) goto oops;
    close(fd);
    if(_sz) *_sz = sz;
    return data;
oops:
    close(fd);
    if(data != 0) free(data);
    return 0;
}

static int insmod(const char *filename, const char *args)
{    void *module;
    unsigned int size;
    int ret;

    module = load_file(filename, &size);
    if (!module)
        return -1;

    ret = init_module(module, size, args);
    printf("#####insmod %s#####\n",filename);
    free(module);
    return ret;
}

static int rmmod(const char *modname)
{
    int ret = -1;
    int maxtry = 10;
    while (maxtry-- > 0) {
        ret = delete_module(modname, O_NONBLOCK | O_EXCL);
        if (ret < 0)
            usleep(500000);
        else
            break;
    }
    if (ret != 0)
        printf("Unable to unload driver module %s \n",
             modname);
    return ret;
}
#endif

int onMode(void)
{
    NEXUS_Error rc;
    NEXUS_PlatformStandbySettings nexusStandbySettings;
    NEXUS_TransportWakeupSettings xptWakeupSettings;

    BKNI_AcquireMutex(mutex);

    printf("\n\nEntering S0 Mode\n\n");

    NEXUS_Platform_GetStandbySettings(&nexusStandbySettings);
    nexusStandbySettings.mode = NEXUS_PlatformStandbyMode_eOn;
    nexusStandbySettings.openFrontend = g_DeviceState.openfe;
    rc = setStandbySettings(&nexusStandbySettings);
    if(rc) { BERR_TRACE(rc); BDBG_ASSERT(!rc); }

    postResume();

    NEXUS_TransportWakeup_GetSettings(&xptWakeupSettings);
    xptWakeupSettings.enabled = false;
    rc = NEXUS_TransportWakeup_SetSettings(&xptWakeupSettings);
    BDBG_ASSERT(!rc);

    BKNI_ReleaseMutex(mutex);

#ifdef WOWLAN
    if (wakeonwlan) {
        rmmod("wlan_plat");
        rmmod("wl");
        insmod("/root/wlan_plat.ko","");
        insmod("/root/wl.ko","wlan0 /root/nvram.txt");
        wakeonwlan = 0;
    }
#endif
    return rc;
}

int initialize_state(int argc, char **argv)
{
    pmlib_state_t pmlib_state;

    /* Initialize default values */
    g_cmd_options.timeout = 5; /* seconds */
    g_cmd_options._auto=false;
    g_cmd_options.ir_wakeup=true;
    g_cmd_options.uhf_wakeup=false;
    g_cmd_options.xpt_wakeup=false;
    g_cmd_options.cec_wakeup=true;
    g_cmd_options.gpio_wakeup=true;
    g_cmd_options.kpd_wakeup=false;
    g_cmd_options.eth_wol_wakeup=false;
    g_cmd_options.moca_wol_wakeup=false;
    g_cmd_options.ethoff=false;
    g_cmd_options.mocaoff=true;
    g_cmd_options.sataoff=true;
    g_cmd_options.standby_flags=0;

    BKNI_Memset(&g_StandbyNexusHandles, 0, sizeof(g_StandbyNexusHandles));
    BKNI_Memset(&g_DeviceState, 0, sizeof(g_DeviceState));
    g_DeviceState.source[0] = eInputSourceFile;
    g_DeviceState.playfile[0] = "videos/cnnticker.mpg";
    g_DeviceState.gpio_pin = 1;
    g_DeviceState.gui = true;

    if(!parse_cmdline_args(argc, argv)) return 0;

    if(g_cmd_options._auto) {
        g_cmd_options.ir_wakeup = g_cmd_options.uhf_wakeup = g_cmd_options.xpt_wakeup =  g_cmd_options.cec_wakeup = g_cmd_options.gpio_wakeup = g_cmd_options.kpd_wakeup = g_cmd_options.eth_wol_wakeup = g_cmd_options.moca_wol_wakeup = false;
    }

    switch(g_DeviceState.source[0]) {
        case eInputSourceQam:
            if (!g_DeviceState.frontend[0].freq || !g_DeviceState.frontend[0].qammode) {
                printf("QAM Frequency and mode is not specified. Using default 765MHz, Qam 64.\n\n");
                g_DeviceState.frontend[0].freq = 765;
                g_DeviceState.frontend[0].qammode = 64;
            }
            g_DeviceState.openfe = true;
            break;
        case eInputSourceSat:
            if (!g_DeviceState.frontend[0].freq) {
                printf("SAT Frequency is not specified. Using default 1119MHz.\n\n");
                g_DeviceState.frontend[0].freq = 1119;
            }
            g_DeviceState.openfe = true;
            break;
        case eInputSourceOfdm:
            if (!g_DeviceState.frontend[0].freq || g_DeviceState.frontend[0].ofdmmode>3) {
                printf("OFDM Frequency and mode is either not specified or specified incorrectly. Using default\n\n");
                g_DeviceState.frontend[0].freq = 578;
                g_DeviceState.frontend[0].ofdmmode = 0;
            }
            g_DeviceState.openfe = true;
            break;
        case eInputSourceFile:
            if(g_DeviceState.playfile[0] != NULL) {
                if(strncasecmp(g_DeviceState.playfile[0], "http", 4)  &&
                   strncasecmp(g_DeviceState.playfile[0], "https", 5) &&
                   strncasecmp(g_DeviceState.playfile[0], "udp", 3)   &&
                   strncasecmp(g_DeviceState.playfile[0], "rtp", 3)) {
                    FILE *fp = fopen(g_DeviceState.playfile[0], "rb");
                    if(!fp) {
                        printf("File %s not found\n", g_DeviceState.playfile[0]);
                        g_DeviceState.source[0] = eInputSourceNone;
                        g_DeviceState.playfile[0] = NULL;
                    } else {
                        fclose(fp);
                    }
                }
            }
            break;
        default:
            break;
    }

    getIfName();
    mocaVer = getMocaVer();
    if(mocaVer)
        printf("MoCA Version %d detected\n", mocaVer);

    g_power_state.brcm_pm_ctx = brcm_pm_init();
    g_power_state.g_state.enet = true;
    g_power_state.g_state.moca = true;

    get_pmlib_state(&pmlib_state);
    pmlib_state.usb = true;
    pmlib_state.enet = true;
    pmlib_state.moca = true;
    pmlib_state.sata = true;
    pmlib_state.tp1 = true;
    pmlib_state.tp2 = true;
    pmlib_state.tp3 = true;
    pmlib_state.cpu = true;
    pmlib_state.ddr = true;
#if MEMC1_SUPPORT
    pmlib_state.memc1 = true;
#else
    pmlib_state.memc1 = false;
#endif
    pmlib_state.flags = g_cmd_options.standby_flags;
    set_pmlib_state(&pmlib_state);

    return 1;
}

int main(int argc, char **argv)
{
    NEXUS_Error rc;
    NEXUS_PlatformSettings platformSettings;
    char buf[256];

    if(!initialize_state(argc, argv)) return 0;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = g_DeviceState.openfe?true:false;
    rc = NEXUS_Platform_Init(&platformSettings);
    BDBG_ASSERT(!rc);

    BKNI_CreateMutex(&mutex);

    NEXUS_Platform_GetStreamerInputBand(0, &g_StandbyNexusHandles.wakeupInputBand);


    rc = start_app();
    if(rc) { rc = BERR_TRACE(rc); goto err; }

    add_outputs();

    printf("\n\n");
    printf("******************************************************************\n");
    printf("******************************************************************\n");
    printf("\n");
    if(eth_info.exists)
        printf("    %s :    HwAddr %s     IpAddr %s\n", eth_info.if_name, eth_info.hw_addr, eth_info.ip_addr);
    if(moca_info.exists)
        printf("    %s :    HwAddr %s     IpAddr %s\n", moca_info.if_name, moca_info.hw_addr, moca_info.ip_addr);
    printf("\n");
    printf("******************************************************************\n");
    printf("******************************************************************\n");
    printf("\n\n");

    switch(g_DeviceState.source[0]) {
        case eInputSourceQam:
        case eInputSourceSat:
        case eInputSourceOfdm:
        case eInputSourceStreamer:
            rc = start_live_context(0);
            if(rc) { rc = BERR_TRACE(rc); goto err; }
            break;
        case eInputSourceFile:
            rc = start_play_context(0);
            if(rc) { rc = BERR_TRACE(rc); goto err; }
            break;
        default:
            break;
    }

    /* Launch the command line thread */
    if(!g_cmd_options._auto) {
        rc = pthread_create(&cmd_thread, NULL, commands_thread, NULL);
        BDBG_ASSERT(!rc);
    }

    while(1) {
        rc = BKNI_WaitForEvent(g_StandbyNexusHandles.event, g_cmd_options._auto?(g_cmd_options.timeout*1000):0xFFFFFFFF);

        if(g_DeviceState.exit_app) {
            break;
        }

        if(g_cmd_options._auto) {
            if(g_cmd_options._autostate) {
                if(g_DeviceState.power_mode == g_cmd_options._autostate)
                    g_DeviceState.power_mode = ePowerModeS0;
                else
                    g_DeviceState.power_mode = g_cmd_options._autostate;
            } else {
                g_DeviceState.power_mode = rand()%4;
            }
            g_DeviceState.timer_wake=true;
        }

        switch(g_DeviceState.power_mode) {
            case ePowerModeS0:
                rc = onMode();
                break;
            case ePowerModeS1:
                rc = activeStandbyMode();
                break;
            case ePowerModeS2:
                rc = passiveStandbyMode();
                break;
            case ePowerModeS3:
                rc = deepStandbyMode();
                break;
            case ePowerModeS5:
                rc = haltMode();
                break;
        }
    }


    if(cmd_thread)
        pthread_join(cmd_thread, NULL);

err:
    stop_decodes();
    remove_outputs();
    stop_app();

    BKNI_DestroyMutex(mutex);

    NEXUS_Platform_Uninit();

    /* Disable WOL */
    setWol(eth_info.if_name, 0);
    if(moca_info.exists) {
        if(mocaVer == 1) {
            system("mocactl wol --disable");
            setWol(moca_info.if_name, 0);
        } else {
            system("mocap set --wom_mode 0");
            system("mocap set --wom_magic_enable 0");
        }
    }

    snprintf(buf, 256, "if [ -e /tmp/udhcpc.%s.pid ]; then kill `cat /tmp/udhcpc.%s.pid`; fi", eth_info.if_name, eth_info.if_name);
    system(buf);
    if(moca_info.exists) {
        snprintf(buf, 256, "if [ -e /tmp/udhcpc.%s.pid ]; then kill `cat /tmp/udhcpc.%s.pid`; fi", moca_info.if_name, moca_info.if_name);
        system(buf);
    }

    return 0;
}
