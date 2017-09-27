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

/*****************************************************************************
*
* DESCRIPTION:
*       The main project function implementation.
*
*****************************************************************************************/

/************************* INCLUDES *****************************************************/
/* Interfaces for platform initialization and reset. */
#include "bbSysBasics.h"
#include "bbSysMemMan.h"
#include "bbSysTaskScheduler.h"
#include "bbMailService.h"
#include "bbHalIrqCtrl.h"
#include "bbHalSymTmr.h"
#include "bbHalSystemTimer.h"
#include "bbHalRadio.h"
#include "zigbee_rf4ce_registration.h"
#include "bbMailAPI.h"
#include "bbSysNvmManager.h"
#ifdef _ZBPRO_
#include "ha_registration.h"
#endif
#include "zigbee_ioctl.h"
#include "zigbee_rpc_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>

# pragma GCC optimize "short-enums"     /* Implement short enums. */

/************************* Project Main Function ****************************************/
/**//**
 * \brief Mailbox descriptor.
 */
bool wdt_occured;

/**//**
 * \brief Initializes SoC HW and SW.
 */
static void initPlatform(void)
{
    HAL_IRQ_DISABLE();

    SYS_MemoryManagerInit();
    SYS_SchedulerInit();
    HAL_SystemTimeInit();
    Mail_ServiceInit();
    HAL_IRQ_ENABLE();
}


#include "private/bbMacPibApi.h"
#define FIRMWARE_PATH "/etc/zigbee/stack_code.bin"
#define INVALID_THREAD (pthread_t)0
int g_zigbeeFd = -1;

/*
 * \brief Project main function.
 */
    typedef enum _TEST_enum_t
    {
        TEST_ENUM_1 = 0x00,
        TEST_ENUM_2   = 0x08,

        TEST_ENUM_3     = 0x06,

        TEST_ENUM_4   = 0x01,

        TEST_ENUM_5     = 0x09,

        TEST_ENUM_6   = 0x02,

    } TEST_enum_t;


static void *wdt_handler(void *pParam)
{
    while (!wdt_occured) {
        if (Zigbee_Ioctl_WaitForWDTInterrupt((int)pParam, NULL) == ZIGBEE_WDT_OK) {
            printf("zigbee watchdog timer event occured!!!\n");
            wdt_occured = true;
        }
        usleep(20);
    }
    return 0;
}

static void *zigbee_server(void *pParam)
{
    while(1) {
        SYS_SchedulerRunTask();
        if (wdt_occured) {
            break;
        }
        usleep(20);
    }

    /* Stop the driver */
    Zigbee_Ioctl_Stop(g_zigbeeFd);

    /* Wait until wdt_thread is done */
    //pthread_join(wdt_thread, NULL);

    /* Uninitialize the NVM support */
    Zigbee_NVM_Uninit();
}

extern bool is_rpc_mode;
static pthread_t wdt_thread = INVALID_THREAD;
static pthread_t zigbee_server_thread = INVALID_THREAD;

int zigbee_init(int argc, char *argv[])
{
    int fd;
    int fw_len;
    unsigned char *fw_img;
    unsigned char rf4ce_mac_addr[8];
    unsigned char zbpro_mac_addr[8];

    is_rpc_mode = false;
    initPlatform();

#ifdef _RF4CE_
    RF4CE_RegistrationInit("/etc/zigbee/db");
#endif

#ifdef _ZBPRO_
    HA_Registration_Init();
#endif

    /* Initialize the NVM support */
    Zigbee_NVM_Init(open, close, write, read, lseek);

#ifdef WDT_RELOAD_FW_ONLY
    while (1)
#endif
    {
        /* Open the zigbee driver */
        g_zigbeeFd = open("/dev/zigbee", O_RDWR);
        if (g_zigbeeFd < 0)
        {
            perror("open");
            exit(1);
        }

        /* Create the WDT handler */
        wdt_occured = false;
        pthread_create(&wdt_thread, NULL, wdt_handler, (void *)g_zigbeeFd);

        if (argc > 1) {
            printf("loading firmware from %s\n", argv[1]);
            if ((fd = open(argv[1], O_RDONLY)) == -1) {
                printf("error opening file %s\n", argv[1]);
                exit(1);
            }
        } else {
#ifdef FIRMWARE_PATH
            printf("loading firmware from %s\n", FIRMWARE_PATH);
            if ((fd = open(FIRMWARE_PATH, O_RDONLY)) == -1) {
                printf("error opening file %s\n", FIRMWARE_PATH);
                exit(1);
            }
#else
            printf("no path to firmware specified\n");
            exit(1);
#endif
        }

        fw_len = lseek(fd, 0, SEEK_END);
        printf("fw_len=%d\n", fw_len); // TBD - removing this causes problems
        fw_img = mmap(NULL, fw_len, PROT_READ, MAP_SHARED, fd, 0);

        if (fw_img == NULL) {
            printf("can't mmap firmware image\n");
            exit(1);
        }

        close(fd);

        /* Load and start the firmware */
        Zigbee_Ioctl_Start(g_zigbeeFd, fw_img, fw_len);

        Zigbee_Ioctl_GetRf4ceMacAddr(g_zigbeeFd, rf4ce_mac_addr);
        Zigbee_Ioctl_GetZbproMacAddr(g_zigbeeFd, zbpro_mac_addr);
        printf("RF4CE hw address:  %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", rf4ce_mac_addr[3], rf4ce_mac_addr[2], rf4ce_mac_addr[1], rf4ce_mac_addr[0], rf4ce_mac_addr[7], rf4ce_mac_addr[6], rf4ce_mac_addr[5], rf4ce_mac_addr[4]);
        printf("Zigbee pro hw address:  %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", zbpro_mac_addr[3], zbpro_mac_addr[2], zbpro_mac_addr[1], zbpro_mac_addr[0], zbpro_mac_addr[7], zbpro_mac_addr[6], zbpro_mac_addr[5], zbpro_mac_addr[4]);

        pthread_create(&zigbee_server_thread, NULL, zigbee_server, NULL);

        /* Wait until wdt_thread is done */
        //pthread_join(wdt_thread, NULL);
    }
}

void zigbee_deinit()
{
    Zigbee_Ioctl_Stop(g_zigbeeFd);
    wdt_occured = 1;
    Mail_ServiceDeinit();
    if(!pthread_equal(wdt_thread, INVALID_THREAD)){
        pthread_kill(wdt_thread, SIGUSR1);
        pthread_join(wdt_thread, NULL);
    }
    if(!pthread_equal(zigbee_server_thread, INVALID_THREAD))
        pthread_join(zigbee_server_thread, NULL);
}

/* needed for server process only */
int server_main(int argc, char *argv[])
{
    int fd;
    int fw_len;
    unsigned char *fw_img;
    unsigned char rf4ce_mac_addr[8];
    unsigned char zbpro_mac_addr[8];

    is_rpc_mode = true;
    initPlatform();

#ifdef _RF4CE_
    RF4CE_RegistrationInit("/etc/zigbee/db");
#endif

#ifdef _ZBPRO_
    HA_Registration_Init();
#endif

    /* add rpc server support */
    Zigbee_Rpc_ServerOpen();

    /* Initialize the NVM support */
    Zigbee_NVM_Init(open, close, write, read, lseek);

#ifdef WDT_RELOAD_FW_ONLY
    while (1)
#endif
    {
        /* Open the zigbee driver */
        g_zigbeeFd = open("/dev/zigbee", O_RDWR);
        if (g_zigbeeFd < 0)
        {
            perror("open");
            exit(1);
        }

        /* Create the WDT handler */
        wdt_occured = false;
        pthread_create(&wdt_thread, NULL, wdt_handler, (void *)g_zigbeeFd);

        if (argc > 1) {
            printf("loading firmware from %s\n", argv[1]);
            if ((fd = open(argv[1], O_RDONLY)) == -1) {
                printf("error opening file %s\n", argv[1]);
                exit(1);
            }
        } else {
#ifdef FIRMWARE_PATH
            printf("loading firmware from %s\n", FIRMWARE_PATH);
            if ((fd = open(FIRMWARE_PATH, O_RDONLY)) == -1) {
                printf("error opening file %s\n", FIRMWARE_PATH);
                exit(1);
            }
#else
            printf("no path to firmware specified\n");
            exit(1);
#endif
        }

        fw_len = lseek(fd, 0, SEEK_END);
        printf("fw_len=%d\n", fw_len); // TBD - removing this causes problems
        fw_img = mmap(NULL, fw_len, PROT_READ, MAP_SHARED, fd, 0);

        if (fw_img == NULL) {
            printf("can't mmap firmware image\n");
            exit(1);
        }

        close(fd);

        /* Load and start the firmware */
        Zigbee_Ioctl_Start(g_zigbeeFd, fw_img, fw_len);

        Zigbee_Ioctl_GetRf4ceMacAddr(g_zigbeeFd, rf4ce_mac_addr);
        Zigbee_Ioctl_GetZbproMacAddr(g_zigbeeFd, zbpro_mac_addr);
        printf("RF4CE hw address:  %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", rf4ce_mac_addr[3], rf4ce_mac_addr[2], rf4ce_mac_addr[1], rf4ce_mac_addr[0], rf4ce_mac_addr[7], rf4ce_mac_addr[6], rf4ce_mac_addr[5], rf4ce_mac_addr[4]);
        printf("Zigbee pro hw address:  %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", zbpro_mac_addr[3], zbpro_mac_addr[2], zbpro_mac_addr[1], zbpro_mac_addr[0], zbpro_mac_addr[7], zbpro_mac_addr[6], zbpro_mac_addr[5], zbpro_mac_addr[4]);

        while(1) {
            SYS_SchedulerRunTask();
            if (wdt_occured) {
                printf("wdt_occured is true\n");
                break;
            }
            usleep(20);
        }

        /* Stop the driver */
        Zigbee_Ioctl_Stop(g_zigbeeFd);

        /* Wait until wdt_thread is done */
        //pthread_join(wdt_thread, NULL);
    }

    /* Uninitialize the NVM support */
    Zigbee_NVM_Uninit();

    /* Close rpc server support */
    Zigbee_Rpc_ServerClose();
}

/* eof bbPrjMain.c */
