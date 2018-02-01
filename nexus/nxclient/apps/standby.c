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
 ******************************************************************************/
 #if NEXUS_POWER_MANAGEMENT
#include "nexus_platform_client.h"
#include "nexus_core_utils.h"
#include "nxclient.h"
#include "bgui.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bfont.h"
#include "binput.h"
#include "namevalue.h"

#include "nexus_platform.h"
#if NEXUS_HAS_GPIO
#include "nexus_gpio.h"
#endif
#include "nexus_transport_wakeup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "pmlib.h"

BDBG_MODULE(standby);

struct appcontext {
    bgui_t gui;
    binput_t input;
    pthread_t remote_thread_id, cmdline_thread_id;
    bfont_t font;
    BKNI_EventHandle event, wakeupEvent;
    void *pm_ctx;
    NEXUS_PlatformStandbyMode mode;
    unsigned total_buttons, focused_button;
    bool coldBoot;
    bool done;
#if NEXUS_HAS_GPIO
    NEXUS_GpioHandle pin_handle;
#endif
    struct {
        bool ir;
        bool cec;
        bool gpio;
        bool kpd;
        bool xpt;
        bool eth;
        bool moca;
        int timeout;
    } wakeups;
    struct {
        char if_name[32];
        char ip_addr[32];
        char hw_addr[32];
        bool exists;
    } eth, moca;
} g_context;

static const char *g_standby_state[] = {
    "ACTIVE STANDBY  (S1)",
    "PASSIVE STANDBY (S2)",
    "DEEP SLEEP WARM (S3)",
    "DEEP SLEEP COLD (S5)"
};

#if NEXUS_HAS_GPIO
static const char *gpio_type_str[] = {
    "GPIO",
    "SGPIO",
    "unused",
    "AON_GPIO",
    "AON_SGPIO"
};
#endif

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

#define GUI_WIDTH  250
#define GUI_HEIGHT 50

void getIfInfo(struct appcontext *pContext)
{
    struct ifaddrs *ifaddr, *ifa;
    struct ifreq s;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        char *ip_addr=NULL, *hw_addr=NULL;
        if (ifa->ifa_addr == NULL)
            continue;

        if(!strncmp(ifa->ifa_name, pContext->eth.if_name, sizeof(pContext->eth.if_name))) {
            if (pContext->eth.exists) continue;
            ip_addr = pContext->eth.ip_addr;
            hw_addr = pContext->eth.hw_addr;
            pContext->eth.exists = true;
        } else if(!strncmp(ifa->ifa_name, pContext->moca.if_name, sizeof(pContext->moca.if_name))) {
            if (pContext->moca.exists) continue;
            ip_addr = pContext->moca.ip_addr;
            hw_addr = pContext->moca.hw_addr;
            pContext->moca.exists = true;
        }
        if(ip_addr && hw_addr) {
            snprintf(s.ifr_name, IFNAMSIZ, ifa->ifa_name);
            if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
                snprintf(hw_addr, 32, "%02x:%02x:%02x:%02x:%02x:%02x",
                        (unsigned char) s.ifr_addr.sa_data[0],
                        (unsigned char) s.ifr_addr.sa_data[1],
                        (unsigned char) s.ifr_addr.sa_data[2],
                        (unsigned char) s.ifr_addr.sa_data[3],
                        (unsigned char) s.ifr_addr.sa_data[4],
                        (unsigned char) s.ifr_addr.sa_data[5]);
            }
            if (0 == ioctl(fd, SIOCGIFADDR, &s)) {
                snprintf(ip_addr, 32, "%s", inet_ntoa(((struct sockaddr_in *)&s.ifr_addr)->sin_addr));
            }
            printf("    %s :    HwAddr %s     IpAddr %s\n", ifa->ifa_name, hw_addr, ip_addr);
        }
    }
    freeifaddrs(ifaddr);
    close(fd);
}

void setWol(struct appcontext *pContext, bool enabled)
{
    char buf[256];

    /* Eth WOL */
    if (pContext->wakeups.eth && pContext->eth.exists) {
        BKNI_Snprintf(buf, 256, "ethtool -s %s wol %s", pContext->eth.if_name, enabled?"g":"d");
        system(buf);
    }

    /* MocA WOL */
    if (pContext->wakeups.moca && pContext->moca.exists) {
       if (enabled) {
            system("mocap set --wom_mode 1");
            BKNI_Snprintf(buf, 256, "mocap set --wom_magic_mac val %s", pContext->moca.hw_addr);
            system(buf);
            system("mocap set --wom_magic_enable 1");
            system("mocap set --wom_pattern mask 0 0xff 15");
        } else {
            system("mocap set --wom_mode 0");
            system("mocap set --wom_magic_enable 0");
        }
    }
}

int setXptWakeupPacket(struct appcontext *pContext, bool enabled)
{
    NEXUS_Error rc=NEXUS_SUCCESS;
    NEXUS_TransportWakeupSettings xptWakeupSettings;

    if (pContext->wakeups.xpt) {
        NEXUS_TransportWakeup_GetSettings(&xptWakeupSettings);
        if(enabled) {
            BKNI_Memcpy(xptWakeupSettings.filter[0].packet, Filter, sizeof(Filter));
            NEXUS_Platform_GetStreamerInputBand(0, &xptWakeupSettings.inputBand);
            xptWakeupSettings.packetLength = sizeof(Filter)/sizeof(Filter[0]);
            xptWakeupSettings.enabled = true;
        } else {
            xptWakeupSettings.enabled = false;
        }
        rc = NEXUS_TransportWakeup_SetSettings(&xptWakeupSettings);
        if (rc) rc = BERR_TRACE(rc);
    }

    return rc;
}

static void render_ui(struct appcontext *pContext)
{
    NxClient_StandbyStatus standbyStatus;
    NEXUS_Graphics2DFillSettings fillSettings;
    int rc;
    unsigned i;

    if(!pContext->gui) return;

    NxClient_GetStandbyStatus(&standbyStatus);
    if(standbyStatus.settings.mode != NEXUS_PlatformStandbyMode_eOn)
        return;

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = bgui_surface(pContext->gui);
    fillSettings.color = 0;
    rc = NEXUS_Graphics2D_Fill(bgui_blitter(pContext->gui), &fillSettings);
    BDBG_ASSERT(!rc);

    for (i=0;i<pContext->total_buttons;i++) {
        bool focused = i == pContext->focused_button;

        fillSettings.rect.x = 0;
        fillSettings.rect.y = i*GUI_HEIGHT;
        fillSettings.rect.width = GUI_WIDTH;
        fillSettings.rect.height = GUI_HEIGHT;
        fillSettings.color = (i == pContext->focused_button) ? 0xFF00FF00 : 0xFF008888;
        rc = NEXUS_Graphics2D_Fill(bgui_blitter(pContext->gui), &fillSettings);
        BDBG_ASSERT(!rc);

        bgui_checkpoint(pContext->gui);

        if (pContext->font) {
            struct bfont_surface_desc desc;
            bfont_get_surface_desc(bgui_surface(pContext->gui), &desc);
            bfont_draw_aligned_text(&desc, pContext->font, &fillSettings.rect, g_standby_state[i], -1, focused?0xFF333333:0xFFCCCCCC, bfont_valign_center, bfont_halign_center);
            NEXUS_Surface_Flush(bgui_surface(pContext->gui));
        }
    }

    bgui_submit(pContext->gui);
}

static void set_wake_event(struct appcontext *pContext)
{
    if(pContext->mode != NEXUS_PlatformStandbyMode_eOn) {
        /* Set event for S1 wakeup or S2/S3 when pmlib not used*/
        if(pContext->mode == NEXUS_PlatformStandbyMode_eActive || !pContext->pm_ctx) {
            BKNI_SetEvent(pContext->wakeupEvent);
        }
    }
}

#if NEXUS_HAS_GPIO
void gpio_interrupt(void *context, int param)
{
    struct appcontext *pContext = context;
    BSTD_UNUSED(param);

    set_wake_event(pContext);
    pContext->mode = NEXUS_PlatformStandbyMode_eOn;
    BKNI_SetEvent(pContext->event);
    if (pContext->pin_handle)
    {
        NEXUS_Gpio_ClearInterrupt(pContext->pin_handle);
    }
}
#endif

static void *remote_key_monitor(void *context)
{
    struct appcontext *pContext = context;

    while(!pContext->done) {
        b_remote_key key;
        if (binput_read_no_repeat(pContext->input, &key)) {
            binput_wait(pContext->input, 1000);
            continue;
        }
        switch (key) {
        case b_remote_key_up:
            if (pContext->focused_button) {
                pContext->focused_button--;
            }
            else {
                pContext->focused_button = pContext->total_buttons-1;
            }
            render_ui(pContext);
            break;
        case b_remote_key_down:
            if (++pContext->focused_button == pContext->total_buttons) {
                pContext->focused_button = 0;
            }
            render_ui(pContext);
            break;
        case b_remote_key_select:
            pContext->coldBoot = false;
            switch (pContext->focused_button) {
                case 3:
                pContext->coldBoot = true;
                case 2:
                pContext->mode = NEXUS_PlatformStandbyMode_eDeepSleep;
                break;
                case 1:
                pContext->mode = NEXUS_PlatformStandbyMode_ePassive;
                break;
                case 0:
                pContext->mode = NEXUS_PlatformStandbyMode_eActive;
                break;
                default:
                break;
            }
            BKNI_SetEvent(pContext->event);
            break;
        case b_remote_key_stop:
        case b_remote_key_clear:
            set_wake_event(pContext);
            pContext->done = true;
            BKNI_SetEvent(pContext->event);
            break;
        case b_remote_key_power:
            set_wake_event(pContext);
            pContext->mode = NEXUS_PlatformStandbyMode_eOn;
            BKNI_SetEvent(pContext->event);
            break;
        default:
            break;
        }
    }

    return NULL;
}

static void *cmdline_monitor(void *context)
{
    struct appcontext *pContext = context;
    int retval = 1;

    while(!pContext->done) {
        fd_set rfds;
        struct timeval timeout;
        char buffer[256];
        char *buf;

        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 250000;

        if(retval) {
            fflush(stdout); printf("standby>"); fflush(stdout);
        }

        retval = select(1, &rfds, NULL, NULL, &timeout);
        if(retval == -1) {
            printf("Stdin Error\n");
            return NULL;
        } else if (!retval) {
            continue;
        }


        fgets(buffer, 256, stdin);
        if (feof(stdin)) break;
        buffer[strlen(buffer)-1] = 0; /* chop off \n */

        buf = strtok(buffer, ";");
        if (!buf) continue;

        do {
            if(!strncmp(buf, "s", 1)) {
                NEXUS_PlatformStandbyMode mode;
                buf++;
                pContext->coldBoot = false;
                if (!strncmp(buf, "0", 1)) {
                    mode = NEXUS_PlatformStandbyMode_eOn;
                } else if (!strncmp(buf, "1", 1)) {
                    mode = NEXUS_PlatformStandbyMode_eActive;
                } else if (!strncmp(buf, "2", 1)) {
                    mode = NEXUS_PlatformStandbyMode_ePassive;
                } else if (!strncmp(buf, "3", 1)) {
                   mode = NEXUS_PlatformStandbyMode_eDeepSleep;
                } else if (!strncmp(buf, "5", 1)) {
                    mode = NEXUS_PlatformStandbyMode_eDeepSleep;
                    pContext->coldBoot = true;
                } else {
                    printf("Unknown standby mode\n");
                    break;
                }

                if(mode != NEXUS_PlatformStandbyMode_eOn) {
                    pContext->focused_button = mode-1;
                    if( mode == NEXUS_PlatformStandbyMode_eDeepSleep && pContext->coldBoot == true)
                        pContext->focused_button++;
                    BDBG_ASSERT(pContext->focused_button<pContext->total_buttons);
                    render_ui(pContext);
                }
                set_wake_event(pContext);
                pContext->mode = mode;
                BKNI_SetEvent(pContext->event);
            } else if (!strncmp(buf, "q", 1)) {
                set_wake_event(pContext);
                pContext->done = true;
                BKNI_SetEvent(pContext->event);
            } else {
                printf("Unknown command\n");
            }
        } while ((buf = strtok(NULL, ";")));
    }

    return NULL;
}

static void print_wakeup(struct appcontext *pContext)
{
    NxClient_StandbyStatus standbyStatus;
    NEXUS_Error rc;
    FILE * fd;
    int wakeup_count = 0;

    if(pContext->focused_button == 0 || !pContext->pm_ctx)
        return;

    rc = NxClient_GetStandbyStatus(&standbyStatus);
    if (rc) {BERR_TRACE(rc); return;}

    fd = fopen("/sys/class/net/eth0/device/power/wakeup_count", "r");
    if (!fd) {
        printf("Could not open eth wakeup status\n");
    }
    fscanf(fd, "%d", &wakeup_count);
    fclose(fd);

    BDBG_WRN(("Wake up Status:\n"
           "IR      : %d\n"
           "UHF     : %d\n"
           "XPT     : %d\n"
           "CEC     : %d\n"
           "GPIO    : %d\n"
           "KPD     : %d\n"
           "Timeout : %d\n"
           "WoL     : %d\n"
           "\n",
           standbyStatus.status.wakeupStatus.ir,
           standbyStatus.status.wakeupStatus.uhf,
           standbyStatus.status.wakeupStatus.transport,
           standbyStatus.status.wakeupStatus.cec,
           standbyStatus.status.wakeupStatus.gpio,
           standbyStatus.status.wakeupStatus.keypad,
           standbyStatus.status.wakeupStatus.timeout,
           wakeup_count));
}

int set_pmlib_state(struct appcontext *pContext, bool power_down)
{
    struct brcm_pm_state pmlib_state;

    if (!pContext->pm_ctx) return 0;

    brcm_pm_get_status(pContext->pm_ctx, &pmlib_state);
    pmlib_state.sata_status = !power_down;
    pmlib_state.tp1_status  = !power_down;
    pmlib_state.tp2_status  = !power_down;
    pmlib_state.tp3_status  = !power_down;
#if PMLIB_VER == 314
    pmlib_state.srpd_status = power_down?64:0;
#elif PMLIB_VER == 26
    pmlib_state.ddr_timeout = power_down?64:0;
#endif
    brcm_pm_set_status(pContext->pm_ctx, &pmlib_state);

    return 0;
}

#define DEFAULT_TIMEOUT 5

static void set_power_state(struct appcontext *pContext)
{
    NxClient_StandbyStatus standbyStatus;
    NxClient_StandbySettings standbySettings;
    NEXUS_Error rc;
    unsigned timeout = (pContext->wakeups.timeout == -1) ? DEFAULT_TIMEOUT : pContext->wakeups.timeout;
    unsigned cnt=15;

    printf("Setting Mode %s\n", lookup_name(g_platformStandbyModeStrs, pContext->mode));

    if (pContext->mode != NEXUS_PlatformStandbyMode_eOn)
        setXptWakeupPacket(pContext, true);

    NxClient_GetDefaultStandbySettings(&standbySettings);
    standbySettings.settings.mode = pContext->mode;
    standbySettings.settings.wakeupSettings.ir = pContext->wakeups.ir;
    standbySettings.settings.wakeupSettings.cec = pContext->wakeups.cec;
    standbySettings.settings.wakeupSettings.gpio = pContext->wakeups.gpio;
    standbySettings.settings.wakeupSettings.keypad = pContext->wakeups.kpd;
    standbySettings.settings.wakeupSettings.transport = pContext->wakeups.xpt;
    standbySettings.settings.wakeupSettings.timeout = timeout;
    rc = NxClient_SetStandbySettings(&standbySettings);
    if (rc) {BERR_TRACE(rc); goto done;}

    switch (pContext->mode) {
        case NEXUS_PlatformStandbyMode_eOn:
            set_pmlib_state(pContext, false);
            setWol(pContext, false);
            setXptWakeupPacket(pContext, false);
            return;
        case NEXUS_PlatformStandbyMode_eActive:
            set_pmlib_state(pContext, true);
            break;
        case NEXUS_PlatformStandbyMode_ePassive:
            setWol(pContext, true);
            break;
        default:
            break;
    }

    if (timeout) printf("Timeout %u seconds\n", timeout);

    /* Wait for nexus to enter standby */
    while(!pContext->done) {
        NxClient_GetStandbyStatus(&standbyStatus);
        if(standbyStatus.transition == NxClient_StandbyTransition_eDone)
            break;
        BKNI_Sleep(1000);
        cnt--;
        if(!cnt) {BDBG_WRN(("Timeout waiting for standby")); break;}
    }
    /* Return if standby fails */
    if(standbyStatus.settings.mode == NEXUS_PlatformStandbyMode_eOn) {
        BDBG_WRN(("Failed to enter Standby"));
        goto done;
    }

    if(pContext->mode == NEXUS_PlatformStandbyMode_eActive || !pContext->pm_ctx) {
        rc = BKNI_WaitForEvent(pContext->wakeupEvent, timeout*1000);
    } else if(pContext->mode == NEXUS_PlatformStandbyMode_ePassive) {
        brcm_pm_suspend(pContext->pm_ctx, BRCM_PM_STANDBY);
    } else if(pContext->mode == NEXUS_PlatformStandbyMode_eDeepSleep) {
        if(!pContext->coldBoot) {
            brcm_pm_suspend(pContext->pm_ctx, BRCM_PM_SUSPEND);
        } else {
#if PMLIB_VER == 314
            system("poweroff");
#else
            system("echo 1 > /sys/devices/platform/brcmstb/halt_mode");
            system("halt");
#endif
        }
    }

    print_wakeup(pContext);

    NxClient_GetStandbyStatus(&standbyStatus);
    if(rc == NEXUS_TIMEOUT || !standbyStatus.status.wakeupStatus.ir) {
        pContext->mode = NEXUS_PlatformStandbyMode_eOn;
        BKNI_SetEvent(pContext->event);
    }
done:
    return;
}

static void print_usage(void)
{
    printf(
    "Usage: nexus.client standby OPTIONS\n"
    "\n"
    "OPTIONS:\n"
    "  --help or -h for help\n");
    printf(
    "  -timeout X               Timeout in seconds. Default is 5 seconds\n"
    "  -ir      off             Disable Ir wakeup. Enabled by default\n"
    "  -cec     on              Enable Cec wakeup. Disabled by default\n"
    "  -kpd     on              Enable Keypad wakeup. Disabled by default\n");
    printf(
    "  -xpt     on              Enable Transport wakeup. Disabled by default\n"
    "  -eth     <interface>     Enable Ethernet wakeup on interface. Disabled by default\n"
    "  -moca    <interface>     Enable Moca wakeup on interface. Disabled by default\n");
    printf(
#if NEXUS_HAS_GPIO
    "  -gpio                    [s|a|as][0-99]  Wakeup GPIO pin (ex. -p a9)\n"
    "                             s=SGPIO, special\n"
    "                             a=AON_GPIO, AonStandard\n"
    "                             as=AON_SGPIO, AonSpecial\n"
#endif
    "  -prompt  off             disable user prompt.\n"
    "  -gui     off             disable gui.\n"
    "  -pmlib   off             disable pmlib support.\n");
    printf(
    "  -s0                      wake up and exit\n"
    "  -s1                      put system into S1 and exit (unless timeout set)\n"
    "  -s2                      put system into S2 and suspend\n"
    "  -s3                      put system into S3 and suspend\n"
    "  -s5                      put system into S5 (cold boot) and suspend\n");
}

int main(int argc, const char **argv)
{
    NEXUS_Error rc;
    struct appcontext *pContext = &g_context;
    NxClient_JoinSettings joinSettings;
    int curarg = 1;
    struct bgui_settings gui_settings;
    bool gui=true, prompt=true, pmlib=true, exit=false;
#if NEXUS_HAS_GPIO
    NEXUS_GpioType gpio_type = 0;
    unsigned gpio_pin = 0;
    NEXUS_GpioSettings gpioSettings;
#endif

    BKNI_Memset(pContext, 0, sizeof(struct appcontext));
    pContext->mode = NEXUS_PlatformStandbyMode_eOn;
    pContext->done = false;
    pContext->total_buttons = sizeof(g_standby_state)/sizeof(g_standby_state[0]);

    pContext->wakeups.ir = true;
    pContext->wakeups.timeout = -1;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
#if NEXUS_HAS_GPIO
        else if (!strcmp(argv[curarg], "-gpio") && argc>curarg+1) {
            char *gpio_string;
            gpio_pin = strtoul(argv[curarg+1], &gpio_string, 10);
            if (gpio_string == argv[curarg+1]) return 1;
            if (*gpio_string == '\0') {
                gpio_type = NEXUS_GpioType_eAonStandard;
            }
            else {
                if      (gpio_string[0] == 'a' && gpio_string[1] == 's') gpio_type = NEXUS_GpioType_eAonSpecial;
                else if (gpio_string[0] == 's')                          gpio_type = NEXUS_GpioType_eSpecial;
                else if (gpio_string[0] == 'a')                          gpio_type = NEXUS_GpioType_eAonStandard;
                else return 1;
            }
            pContext->wakeups.gpio = true;
        }
#endif
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            pContext->wakeups.timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-ir") && argc>curarg+1) {
            if (!strcmp(argv[++curarg], "off"))
                pContext->wakeups.ir = false;
        }
        else if (!strcmp(argv[curarg], "-cec") && argc>curarg+1) {
            if (!strcmp(argv[++curarg], "on"))
                pContext->wakeups.cec = true;
        }
        else if (!strcmp(argv[curarg], "-kpd") && argc>curarg+1) {
            if (!strcmp(argv[++curarg], "on"))
            pContext->wakeups.kpd = true;
        }
        else if (!strcmp(argv[curarg], "-xpt") && argc>curarg+1) {
            if (!strcmp(argv[++curarg], "on"))
                pContext->wakeups.xpt = true;
        }
        else if (!strcmp(argv[curarg], "-eth") && argc>curarg+1) {
            pContext->wakeups.eth = true;
            BKNI_Snprintf(pContext->eth.if_name, 16, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-moca") && argc>curarg+1) {
            pContext->wakeups.moca = true;
            BKNI_Snprintf(pContext->moca.if_name, 16, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-prompt") && argc>curarg+1) {
            if (!strcmp(argv[++curarg], "off")){
                prompt = false;
            }
            else {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-gui") && argc>curarg+1) {
            if (!strcmp(argv[++curarg], "off")) {
                gui = false;
            }
            else {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-pmlib") && argc>curarg+1) {
            if (!strcmp(argv[++curarg], "off")) {
                pmlib = false;
            }
            else {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-s0")) {
            pContext->mode = NEXUS_PlatformStandbyMode_eOn;
            exit = true;
        }
        else if (!strcmp(argv[curarg], "-s1")) {
            pContext->mode = NEXUS_PlatformStandbyMode_eActive;
            exit = true;
        }
        else if (!strcmp(argv[curarg], "-s2")) {
            pContext->mode = NEXUS_PlatformStandbyMode_ePassive;
            exit = true;
        }
        else if (!strcmp(argv[curarg], "-s3")) {
            pContext->mode = NEXUS_PlatformStandbyMode_eDeepSleep;
            exit = true;
        }
        else if (!strcmp(argv[curarg], "-s5")) {
            pContext->mode = NEXUS_PlatformStandbyMode_eDeepSleep;
            pContext->coldBoot = true;
        }
        curarg++;
    }

    if(!gui && !prompt) {
        printf("Both gui and prompt cannot be disabled\n");
        print_usage();
        return -1;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    BKNI_CreateEvent(&pContext->event);
    BKNI_CreateEvent(&pContext->wakeupEvent);

    if(pmlib) {
        pContext->pm_ctx = brcm_pm_init();
    }

    NxClient_UnregisterAcknowledgeStandby(NxClient_RegisterAcknowledgeStandby());

#if NEXUS_HAS_GPIO
    if (pContext->wakeups.gpio) {
        printf("Enabling wakeup GPIO: %s-%d\n", gpio_type_str[gpio_type], gpio_pin);
        NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eAonStandard, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eInput;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eRisingEdge;
        gpioSettings.interrupt.callback = gpio_interrupt;
        gpioSettings.interrupt.context = pContext;
        pContext->pin_handle = NEXUS_Gpio_Open(gpio_type, gpio_pin, &gpioSettings);
    }
#endif

    if (pContext->wakeups.eth || pContext->wakeups.moca) {
        getIfInfo(pContext);
        if (pContext->wakeups.eth && !pContext->eth.exists) {
            printf("Interface %s not found\n", pContext->eth.if_name);
        }
        if (pContext->wakeups.moca && !pContext->moca.exists) {
            printf("Interface %s not found\n", pContext->moca.if_name);
        }
    }

    set_power_state(pContext);

    if(exit) goto done;

    if(gui) {
        bgui_get_default_settings(&gui_settings);
        gui_settings.width = GUI_WIDTH;
        gui_settings.height = GUI_HEIGHT*pContext->total_buttons;
        pContext->gui = bgui_create(&gui_settings);
        pContext->input = binput_open(NULL);

        {
            NEXUS_SurfaceComposition comp;
            NxClient_GetSurfaceClientComposition(bgui_surface_client_id(pContext->gui), &comp);
            comp.position.x = GUI_HEIGHT;
            comp.position.y = GUI_HEIGHT;
            comp.position.width = GUI_WIDTH;
            comp.position.height = GUI_HEIGHT*pContext->total_buttons;
            comp.zorder = 100; /* always on top */
            NxClient_SetSurfaceClientComposition(bgui_surface_client_id(pContext->gui), &comp);
        }

        pContext->font = bfont_open("nxclient/arial_18_aa.bwin_font");

        render_ui(pContext);

        rc = pthread_create(&pContext->remote_thread_id, NULL, remote_key_monitor, pContext);
        if (rc) return -1;
    }

    if(prompt) {
        rc = pthread_create(&pContext->cmdline_thread_id, NULL, cmdline_monitor, pContext);
        if (rc) return -1;
    }

    while(1) {
        BKNI_WaitForEvent(pContext->event, BKNI_INFINITE);

        if(pContext->done)
            break;

        set_power_state(pContext);
    }

    if(pContext->cmdline_thread_id) {
        pthread_join(pContext->cmdline_thread_id, NULL);
    }
    if(pContext->remote_thread_id) {
        pthread_join(pContext->remote_thread_id, NULL);
    }
    if (pContext->font) {
        bfont_close(pContext->font);
    }
    if (pContext->input) {
        binput_close(pContext->input);
    }
    if (pContext->gui) {
        bgui_destroy(pContext->gui);
    }

done:
#if NEXUS_HAS_GPIO
    if (pContext->wakeups.gpio) {
        NEXUS_Gpio_Close(pContext->pin_handle);
    }
#endif

    if(pContext->pm_ctx) {
        brcm_pm_close(pContext->pm_ctx);
    }
    if (pContext->wakeupEvent) {
        BKNI_DestroyEvent(pContext->wakeupEvent);
    }
    if (pContext->event) {
        BKNI_DestroyEvent(pContext->event);
    }
    NxClient_Uninit();

    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform.\n");
    return 0;
}
#endif
