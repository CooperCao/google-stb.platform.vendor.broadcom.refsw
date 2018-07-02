/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ***************************************************************************/


#include <cstdlib>
#include <cstring>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "uappd_uapp.h"
#include "uappd_msg.h"

void * coreDumpMem = NULL;
typedef struct coredump_memregion {
    unsigned int startAddr,endAddr, offset,flags;
} coredump_memregion;

void UserAppDmon::uappReap(int pid)
{
    // Mark user app reaped, handle it in main loop
    UserApp *pUApp = uappFindPid(pid);
    if (pUApp) {
        pUApp->reaped = true;
    }
    else {
        // Hanging reap case:
        // There is a corner case when the parent has no child pid
        // because it has not get a chance to run after the fork()
        // call before the child exits (most likely due to error).
        reapPid = pid;
    }
}

void UserAppDmon::uappReapProc()
{
    // Handle hanging reap case
    if (reapPid) {
        UserApp *pUApp = uappFindPid(reapPid);
        if (pUApp) {
            pUApp->reaped = true;
        }
        else {
            LOGE("Reaped pid %d not found", reapPid);
        }
        reapPid = 0;
    }

    // Clean-up all reaped user apps
    for (uint32_t i = 0; i < UAPP_NUM_MAX; i++) {
        if (pUApps[i] &&
            pUApps[i]->reaped) {

            LOGI("Reaped user app %s", pUApps[i]->name.c_str());

            // Exit user app if necessary
            pUApps[i]->terminate();

            delete pUApps[i];
            pUApps[i] = NULL;
            uappCnt--;
        }
    }
}

void UserAppDmon::uappStartProc(
    struct tzioc_msg_hdr *pHdr)
{
    int err = 0;

    struct uappd_msg_uapp_start_cmd *pCmd =
        (struct uappd_msg_uapp_start_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("Invalid user app start cmd received");
        return;
    }

    LOGD("User app start cmd received, name %s, exec %s, shared %d",
         pCmd->name, pCmd->exec, pCmd->shared);

    UserApp *pUApp = NULL;
    PeerApp *pPApp = NULL;

    try {
        // Check existing user apps
        pUApp = uappFindName(pCmd->name);

        if (!pUApp) {
            // Create a new user app
            pUApp = new UserApp(
                    pCmd->name,
                    pCmd->exec,
                    pCmd->shared);

            // Add user app
            err = uappAdd(pUApp);
            if (err) {
                LOGE("Failed to add user app %s", pCmd->name);
                throw(err);
            }

            // Create a new peer app
            pPApp = new PeerApp(
                    pHdr->ucOrig,
                    pCmd->cookie);

            // Add peer app to user app
            err = pUApp->pappAdd(pPApp);
            if (err) {
                LOGE("Failed to add peer app to user app %s", pCmd->name);
                throw(err);
            }

            // Start user app process
            err = pUApp->start();
            if (err) {
                LOGE("Failed to start user app %s", pCmd->name);
                throw(err);
            }

            LOGI("Started new user app %s", pCmd->name);
        }
        else {
            LOGD("Found existing user app %s", pCmd->name);

            // Check existing peer apps
            pPApp = pUApp->pappFind(pHdr->ucOrig, pCmd->cookie);

            if (!pPApp) {
                // Is user app shared?
                if (!pUApp->shared || !pCmd->shared) {
                    LOGE("Can not add peer app to user app %s", pCmd->name);
                    pUApp = NULL; // don't delete existing user app
                    throw((int)-EEXIST);
                }

                // Create a new peer app
                pPApp = new PeerApp(
                        pHdr->ucOrig,
                        pCmd->cookie);

                // Add peer app to user app
                err = pUApp->pappAdd(pPApp);
                if (err) {
                    LOGE("Failed to add peer app to user app %s", pCmd->name);
                    pUApp = NULL; // don't delete existing user app
                    throw(err);
                }
            }
            else {
                LOGD("Found existing peer app for user app %s", pCmd->name);
            }
        }
    }
    catch (int exception) {
        if (pUApp) {
            if (pPApp) {
                pUApp->pappRmv(pPApp);
                delete pPApp;
            }
            uappRmv(pUApp);
            delete pUApp;
        }
        err = exception;
    }
    catch (...) {
        LOGI("Unhandled exception");
        err = -ENOENT;
    }

    // Caution: reused the cmd buffer for rpy. This only works if:
    // * rpy is smaller then cmd buffer (true if static buffer is used);
    // * it is OK to reuse or to copy on-spot of the relevent fields.
    struct uappd_msg_uapp_start_rpy *pRpy =
        (struct uappd_msg_uapp_start_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

    // pRpy->cookie = pCmd->cookie
    // pRpy->name = pCmd->name
    pRpy->retVal = err;

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = TZIOC_CLIENT_ID_UAPPD;
    pHdr->ulLen  = sizeof(*pRpy);

    tzioc_msg_send(hClient, pHdr);
}

void UserAppDmon::uappStopProc(
    struct tzioc_msg_hdr *pHdr)
{
    int err = 0;

    struct uappd_msg_uapp_stop_cmd *pCmd =
        (struct uappd_msg_uapp_stop_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("Invalid user app stop cmd received");
        return;
    }

    LOGD("User app stop cmd received, name %s", pCmd->name);

    UserApp *pUApp = NULL;
    PeerApp *pPApp = NULL;

    try {
        // Check existing user apps
        pUApp = uappFindName(pCmd->name);

        if (!pUApp) {
            LOGE("Failed to find existing user app %s", pCmd->name);
            throw((int)-ENOENT);
        }

        LOGD("Found existing user app %s", pCmd->name);

        // Check existing peer apps
        pPApp = pUApp->pappFind(pHdr->ucOrig, pCmd->cookie);

        if (!pPApp) {
            LOGE("Failed to find existing peer app for user app %s", pCmd->name);
            throw((int)-ENOENT);
        }

        LOGD("Found existing peer app for user app %s", pCmd->name);

        // Remove peer app
        pUApp->pappRmv(pPApp);
        delete pPApp;

        LOGD("Removed existing peer app for user app %s", pCmd->name);

        // Stop user app process
        if (pUApp->pappCnt == 0) {
            err = pUApp->stop();
            if (err) {
                LOGE("Failed to stop user app %s", pCmd->name);
                throw(err);
            }

            LOGI("Stopped user app %s", pCmd->name);
        }
    }
    catch (int exception) {
        err = exception;
    }
    catch (...) {
        LOGI("Unhandled exception");
        err = -ENOENT;
    }

    // Caution: reused the cmd buffer for rpy
    struct uappd_msg_uapp_stop_rpy *pRpy =
        (struct uappd_msg_uapp_stop_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

    // pRpy->cookie = pCmd->cookie
    // pRpy->name = pCmd->name
    pRpy->retVal = err;

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = TZIOC_CLIENT_ID_UAPPD;
    pHdr->ulLen  = sizeof(*pRpy);

    tzioc_msg_send(hClient, pHdr);
}

void UserAppDmon::uappGetIdProc(
    struct tzioc_msg_hdr *pHdr)
{
    int err = 0;

    struct uappd_msg_uapp_getid_cmd *pCmd =
        (struct uappd_msg_uapp_getid_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("Invalid user app get id cmd received");
        return;
    }

    LOGD("User app getid cmd received, name %s", pCmd->name);

    UserApp *pUApp = NULL;
    PeerApp *pPApp = NULL;

    try {
        // Check existing user apps
        pUApp = uappFindName(pCmd->name);

        if (!pUApp) {
            LOGE("Failed to find existing user app %s", pCmd->name);
            throw((int)-ENOENT);
        }

        LOGD("Found existing user app %s", pCmd->name);

        // Check existing peer apps
        pPApp = pUApp->pappFind(pHdr->ucOrig, pCmd->cookie);

        if (!pPApp) {
            LOGE("Failed to find existing peer app for user app %s", pCmd->name);
            throw((int)-ENOENT);
        }

        LOGD("Found existing peer app for user app %s", pCmd->name);

        if (!pUApp->id) {
            err = pUApp->getId();
            if (err) {
                LOGE("Failed to get id for user app %s", pCmd->name);
                throw(err);
            }
        }

        LOGD("Got id %d for user app %s", pUApp->id, pCmd->name);
    }
    catch (int exception) {
        err = exception;
    }
    catch (...) {
        LOGI("Unhandled exception");
        err = -ENOENT;
    }

    // Caution: reused the cmd buffer for rpy
    struct uappd_msg_uapp_getid_rpy *pRpy =
        (struct uappd_msg_uapp_getid_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

    // pRpy->cookie = pCmd->cookie
    // pRpy->name = pCmd->name
    pRpy->retVal = err;
    pRpy->id = (err) ? 0 : pUApp->id;

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = TZIOC_CLIENT_ID_UAPPD;
    pHdr->ulLen  = sizeof(*pRpy);

    tzioc_msg_send(hClient, pHdr);
}

void UserAppDmon::uappCoreDumpProc(
    struct tzioc_msg_hdr *pHdr)
{
    int err = 0;

    struct uappd_msg_uapp_coredump_cmd *pCmd =
        (struct uappd_msg_uapp_coredump_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("Invalid user app stop cmd received");
        return;
    }

    LOGD("User app Core dump cmd received, name %s Addr=0x%zx size=0x%x", pCmd->name, (size_t)pCmd->paddr, pCmd->bytes);
    // Map physical address
    coreDumpMem = tzioc_map_paddr(
        UserAppDmon::hClient,
        pCmd->paddr,
        pCmd->bytes,
        0);

    if (!coreDumpMem) {
        LOGE("Failed to map physical addr 0x%zx, bytes 0x%x",
            (size_t)pCmd->paddr, pCmd->bytes);
        return;
    }
    UserApp *pUApp = NULL;
    PeerApp *pPApp = NULL;

    try {
        // Check existing user apps
        pUApp = uappFindName(pCmd->name);

        if (!pUApp) {
            LOGE("Failed to find existing user app %s", pCmd->name);
            throw((int)-ENOENT);
        }

        LOGD("Found existing user app %s", pCmd->name);

        // Check existing peer apps
        pPApp = pUApp->pappFind(pHdr->ucOrig, pCmd->cookie);

        if (!pPApp) {
            LOGE("Failed to find existing peer app for user app %s", pCmd->name);
            throw((int)-ENOENT);
        }

        LOGD("Found existing peer app for user app %s", pCmd->name);

        // Remove peer app
        pUApp->pappRmv(pPApp);
        delete pPApp;

        LOGD("Removed existing peer app for user app %s", pCmd->name);

        // Stop user app process
        if (pUApp->pappCnt == 0) {
            err = pUApp->coredump();
            if (err) {
                LOGE("Failed to stop user app %s", pCmd->name);
                throw(err);
            }

            LOGI("Core dumped user app %s", pCmd->name);
        }
    }
    catch (int exception) {
        err = exception;
    }
    catch (...) {
        LOGI("Unhandled exception");
        err = -ENOENT;
    }

    // Caution: reused the cmd buffer for rpy
    struct uappd_msg_uapp_coredump_rpy *pRpy =
        (struct uappd_msg_uapp_coredump_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

    // pRpy->cookie = pCmd->cookie
    // pRpy->name = pCmd->name
    pRpy->retVal = err;

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = TZIOC_CLIENT_ID_UAPPD;
    pHdr->ulLen  = sizeof(*pRpy);

    tzioc_msg_send(hClient, pHdr);

    // Unmap physical address
    tzioc_unmap_paddr(
        UserAppDmon::hClient,
        pCmd->paddr,
        pCmd->bytes);
}

int UserAppDmon::uappAdd(UserApp *pUApp)
{
    for (uint32_t i = 0; i < UAPP_NUM_MAX; i++) {
        if (!pUApps[i]) {
            pUApps[i] = pUApp;
            uappCnt++;
            return 0;
        }
    }
    return -ENOMEM;
}

int UserAppDmon::uappRmv(UserApp *pUApp)
{
    for (uint32_t i = 0; i < UAPP_NUM_MAX; i++) {
        if (pUApps[i] == pUApp) {
            pUApps[i] = NULL;
            uappCnt--;
            return 0;
        }
    }
    return -ENOENT;
}

UserAppDmon::UserApp *UserAppDmon::uappFindPid(int pid)
{
    for (uint32_t i = 0; i < UAPP_NUM_MAX; i++) {
        if (pUApps[i] &&
            pUApps[i]->pid == pid)
            return pUApps[i];
    }
    return NULL;
}

UserAppDmon::UserApp *UserAppDmon::uappFindName(string name)
{
    for (uint32_t i = 0; i < UAPP_NUM_MAX; i++) {
        if (pUApps[i] &&
            pUApps[i]->name == name &&
            pUApps[i]->pappCnt != 0) /* NOT stopped */
            return pUApps[i];
    }
    return NULL;
}

int UserAppDmon::UserApp::start()
{
    pid = fork();
    if (pid == -1) {
        pid = 0;
        return -1;
    }
    else if (pid == 0) {
        setpgid(0,0);
        char *argv[] = {const_cast<char *>(exec.c_str()), NULL};
        execv(exec.c_str(), argv);
        exit(0);
    }
    return 0;
}

int UserAppDmon::UserApp::stop()
{
    int status;
    int error;
    kill(-pid, SIGTERM);
    do {
        error = waitpid(-pid, &status, 0);
        if (error == -1) {
            if (errno == EINTR) {
                printf("Info waitpid interrupted, continuing waitpid: %s\n", strerror(errno));
                error = 0;
                continue;
            }
        }
    } while (error != -1);

    return 0;
}

int UserAppDmon::UserApp::terminate()
{
    if (pappCnt) {
        uint8_t msg[sizeof(struct tzioc_msg_hdr) +
                    sizeof(struct uappd_msg_uapp_exit_nfy)];
        struct tzioc_msg_hdr *pHdr =
            (struct tzioc_msg_hdr *)msg;
        struct uappd_msg_uapp_exit_nfy *pNfy =
            (struct uappd_msg_uapp_exit_nfy *)TZIOC_MSG_PAYLOAD(pHdr);

        pHdr->ucOrig = TZIOC_CLIENT_ID_UAPPD;
        pHdr->ucType = UAPPD_MSG_UAPP_EXIT;
        pHdr->ulLen  = sizeof(*pNfy);
        strncpy(pNfy->name, name.c_str(), UAPPD_NAME_LEN_MAX-1);
        pNfy->name[UAPPD_NAME_LEN_MAX-1] = '\0';
        for (uint32_t i = 0; i < PAPP_NUM_MAX; i++) {
            if (pPApps[i]) {
                pHdr->ucDest = pPApps[i]->id;
                pNfy->cookie = pPApps[i]->cookie;

                tzioc_msg_send(UserAppDmon::hClient, pHdr);
            }
        }
    }
    return 0;
}

int UserAppDmon::UserApp::coredump()
{
    int status;
    kill(pid, SIGQUIT);
    printf("Core Dump : SIQQUIT Returned\n");

    /* Wait for child process to terminate*/
    waitpid(pid, &status, WNOHANG);

    /*Received Core Dump*/
    FILE *fd= fopen("Core.core","r");
    if (fd == nullptr) {
        LOGE("Failed to open Core file");
        return -1;
    }
    unsigned int * buf = (unsigned int *)coreDumpMem;
    ssize_t rbytes = fread(buf, sizeof(unsigned long),20,fd);
    if (rbytes == -1)
        printf("Could not read Core file\n");

    buf += sizeof(unsigned long) * 20;

    int NumMemSections=0;
    rbytes = fread(&NumMemSections, sizeof(int),1,fd);
    if (rbytes == -1)
        printf("Could not read Num Of mem Sections Core file\n");

    *buf = NumMemSections;
    buf +=  sizeof(int);

    coredump_memregion *memRegion = (coredump_memregion *)malloc(sizeof(coredump_memregion) * NumMemSections);
    if(!memRegion) {
        printf("Out of Memory : Malloc failed");
        goto error;
    }
    rbytes = fread(memRegion, sizeof(coredump_memregion),NumMemSections,fd);
    if (rbytes == -1)
        printf("Could not read Num Of mem Sections Core file\n");

    memcpy(buf,memRegion,(sizeof(coredump_memregion) * NumMemSections));
    buf += (sizeof(coredump_memregion) * NumMemSections);

    for(int i=0;i<NumMemSections; i++)
    {
        int size = memRegion[i].endAddr - memRegion[i].startAddr + 1;
        rbytes = fread(buf, size,1,fd);
        if (rbytes == -1)
            printf("Could not read Num Of mem Sections Core file\n");
        buf += size;
    }
error:
    if(memRegion)
        free(memRegion);
    fclose(fd);

    return 0;
}


int UserAppDmon::UserApp::getId()
{
    int err = tzioc_client_getid(
        UserAppDmon::hClient,
        const_cast<char *>(name.c_str()),
        pid,
        &id);

    if (err) {
        LOGE("Error getting TZIOC client id, err %d", err);
        return err;
    }
    return 0;
}

int UserAppDmon::UserApp::pappAdd(PeerApp *pPApp)
{
    for (uint32_t i = 0; i < PAPP_NUM_MAX; i++) {
        if (!pPApps[i]) {
            pPApps[i] = pPApp;
            pappCnt++;
            return 0;
        }
    }
    return -ENOMEM;
}

int UserAppDmon::UserApp::pappRmv(PeerApp *pPApp)
{
    for (uint32_t i = 0; i < PAPP_NUM_MAX; i++) {
        if (pPApps[i] == pPApp) {
            pPApps[i] = NULL;
            pappCnt--;
            return 0;
        }
    }
    return -ENOENT;
}

UserAppDmon::PeerApp *UserAppDmon::UserApp::pappFind(
    uint8_t id,
    uint32_t cookie)
{
    for (uint32_t i = 0; i < PAPP_NUM_MAX; i++) {
        if (pPApps[i] &&
            pPApps[i]->id == id &&
            pPApps[i]->cookie == cookie) {
            return pPApps[i];
        }
    }
    return NULL;
}
