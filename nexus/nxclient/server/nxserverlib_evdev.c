/******************************************************************************
* (c) 2015 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "nxserverlib_evdev.h"

#if NEXUS_HAS_INPUT_ROUTER
#include "nexus_input_router.h"

/* following used for input_event support */
#include <linux/input.h> 
#include <errno.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>

#include <stdio.h>
#include <malloc.h>
#include <memory.h>

BDBG_MODULE(nxserverlib_evdev);

struct event_device {
    int eventId;
    int fd;
};

struct b_evdev {
    struct nxserver_evdev_settings settings;
    pthread_t evdevThread;
    struct event_device *eventDevices;
    bool stopThread;
    sem_t mutex;
};

static int  evdev_read(nxserver_evdev_t evdev, struct input_event *input);
static void *evdev_thread(void *context);

#define EVDEV_FMT "/dev/input/event%d"
/*
 * WARNING: Use of sizeof(EVDEV_FMT), means that only event0 - event99 is supported
 *          so MAX_EVENT_DEVICES = 100 (0-99)
 */
#define MAX_EVENT_DEVICES_DEFAULT 3
#define MAX_EVENT_DEVICES         100

static void close_event_devices(nxserver_evdev_t evdev)
{
    unsigned i;

    sem_wait(&evdev->mutex);
    for (i=0;i<evdev->settings.maxEventDevices;i++) {
        if (evdev->eventDevices[i].fd != -1) {
            BDBG_MSG(("Closing " EVDEV_FMT, i));
            close(evdev->eventDevices[i].fd);
            evdev->eventDevices[i].eventId = evdev->eventDevices[i].fd = -1;
        }
    }
    sem_post(&evdev->mutex);
}

void nxserver_evdev_get_default_settings(struct nxserver_evdev_settings *settings)
{
    memset(settings, 0, sizeof(struct nxserver_evdev_settings));
    settings->maxEventDevices = MAX_EVENT_DEVICES_DEFAULT;
}

nxserver_evdev_t nxserverlib_evdev_init(const struct nxserver_evdev_settings *settings)
{
    int rc;
    nxserver_evdev_t evdev; 


    evdev = BKNI_Malloc(sizeof(struct b_evdev));
    if (!evdev) {
        BDBG_ERR(("Unable to alloc memory for evdev structure"));
        return NULL;
    }

    memset(evdev, 0, sizeof(struct b_evdev));
    evdev->settings = *settings;
    evdev->stopThread = false;

    if (settings->maxEventDevices > MAX_EVENT_DEVICES) {
        BDBG_ERR(("The number of evdev devices is limited to 100 or less."));
        evdev->settings.maxEventDevices = MAX_EVENT_DEVICES;
    }

    sem_init(&evdev->mutex, 0, 1);
    /* eventDevices array is initialized in the evdevThread.  This is to allow dynamic scanning and updating */
    evdev->eventDevices = BKNI_Malloc(evdev->settings.maxEventDevices*sizeof(struct event_device));
    if (!evdev) {
        BDBG_ERR(("Unable to alloc memory for event_device structure"));
        BKNI_Free(evdev);
        return NULL;
    }

    rc = pthread_create(&evdev->evdevThread, NULL, evdev_thread, evdev);
    if (rc) return NULL;
    
    return evdev;
}

void nxserverlib_evdev_uninit(nxserver_evdev_t evdev)
{
    /* we will need to wait the poll() timeout time for the thread to exit */
    evdev->stopThread = true;
    pthread_join(evdev->evdevThread, NULL);
    BKNI_Free(evdev->eventDevices);
    BKNI_Free(evdev);
}

size_t nxserverlib_get_evdev_input(nxserver_evdev_t evdev, NEXUS_InputRouterCode *pCode )
{
    NEXUS_Error rc;

    struct input_event input;
    rc = evdev_read(evdev, &input);
    if (rc) {
        return 0;
    } else {
        BDBG_MSG(("input_event: time=%ld.%06ld type=%d code=%d value=%d", 
                    input.time.tv_sec, input.time.tv_usec, input.type, input.code, input.value));
        NEXUS_InputRouter_GetDefaultCode(pCode);
        pCode->deviceType = NEXUS_InputRouterDevice_eEvdev;
        pCode->filterMask = 1<<pCode->deviceType;
        pCode->data.evdev.type = input.type;
        pCode->data.evdev.code = input.code;
        pCode->data.evdev.value = input.value;
        return 1;
    } 
}

static void *evdev_thread(void *context)
{
    int pollingfd = epoll_create(1);
    struct epoll_event ev = { 0, { 0 } };
    struct epoll_event pevents[ MAX_EVENT_DEVICES ];
    nxserver_evdev_t evdev = context;
    unsigned i;
    bool timeout = true;

    if (pollingfd < 0) {
        BDBG_ERR(("Unable to create epoll descriptor"));
        return NULL;
    }

    for (i=0;i<evdev->settings.maxEventDevices;i++) {
        evdev->eventDevices[i].eventId = evdev->eventDevices[i].fd = -1;
    }

    while (1) {
        int rc;

        if (evdev->stopThread ) break;

        /* Check if new devices have been added when we are bored */
        if (timeout) {
            sem_wait(&evdev->mutex);
            for (i=0;i<evdev->settings.maxEventDevices;i++) {
                if (evdev->eventDevices[i].fd == -1) {
                    /* maxEventDevices is limited to 100 (0-99) or less, so we can hard code node[] */
                    char node[sizeof(EVDEV_FMT)];
                    snprintf(node, sizeof(node), EVDEV_FMT, i);
                    evdev->eventDevices[i].fd = open(node, O_NONBLOCK|O_RDONLY);
                    if (evdev->eventDevices[i].fd != -1) {
                        evdev->eventDevices[i].eventId = i;
                        BDBG_MSG(("Opened input device %s", node));
#ifdef EPOLLWAKEUP
                        ev.events = EPOLLIN | EPOLLWAKEUP;
#else
                        ev.events = EPOLLIN;
#endif
                        ev.data.fd = evdev->eventDevices[i].fd;
                        if (epoll_ctl(pollingfd, EPOLL_CTL_ADD, evdev->eventDevices[i].fd, &ev) != 0 ) {
                            BDBG_MSG(("EPOLL_CTL_ADD - Unable to add event EPOLLIN, fd=%d", evdev->eventDevices[i].fd));
                        }
                    }
                }
            }
            sem_post(&evdev->mutex);
        }

        timeout = false;
        rc = epoll_wait(pollingfd, pevents, 1, 500);
        if (rc < 0) {
            BDBG_ERR(("Error polling on event devices!"));
            continue;
        } 
        else if (rc == 0) {
            timeout = true;
            continue;
        }
        for (i = 0 ; i < (unsigned)rc ; i++) {
            if (pevents[i].events & EPOLLERR) {
                unsigned x;
                BDBG_MSG(("Received EPOLLERR on fd %d", pevents[i].data.fd));
                for (x = 0 ; x < evdev->settings.maxEventDevices ; x++) {
                    if (pevents[i].data.fd == evdev->eventDevices[x].fd) {
                        BDBG_MSG(("Closing " EVDEV_FMT, x));
                        sem_wait(&evdev->mutex);
                        close(evdev->eventDevices[x].fd);
                        evdev->eventDevices[x].eventId = evdev->eventDevices[x].fd = -1;
                        sem_post(&evdev->mutex);
                    }
                }
                if (epoll_ctl(pollingfd, EPOLL_CTL_DEL, pevents[i].data.fd, NULL) != 0 ) {
                    BDBG_MSG(("EPOLL_CTL_DEL - Unable to delete event, fd=%d", pevents[i].data.fd));
                }
            }
            evdev->settings.eventReady.callback(evdev->settings.eventReady.context,
                    evdev->settings.eventReady.param);
        }
    }

    close_event_devices(evdev);
    close(pollingfd);
    return NULL;
}

static int evdev_read(nxserver_evdev_t evdev, struct input_event *input)
{
    unsigned i;

    for (i=0;i<evdev->settings.maxEventDevices;i++) {
        ssize_t n;

        sem_wait(&evdev->mutex);
        if (evdev->eventDevices[i].fd == -1 || evdev->eventDevices[i].eventId == -1) {
            sem_post(&evdev->mutex);
            continue;
        }
        n = read(evdev->eventDevices[i].fd, input, sizeof(*input));
        sem_post(&evdev->mutex);

        if (!n || n == -1) continue;
    
        if (n != sizeof(*input)) {
            return BERR_TRACE(-1);
        }
        /* got one event */
        return 0;
    }
    /* nothing */
    return -1;
}
#else
#endif /* NEXUS_HAS_INPUT_ROUTER */
