/***************************************************************************
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
 ***************************************************************************/

#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

#include "uappd_file.h"
#include "uappd_msg.h"

void UserAppDmon::ufileOpenProc(
    struct tzioc_msg_hdr *pHdr)
{
    int err = 0;

    struct uappd_msg_file_open_cmd *pCmd =
        (struct uappd_msg_file_open_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("Invalid user file open cmd received");
        return;
    }

    LOGD("User file open cmd received, path %s, flags 0x%x",
         pCmd->path, pCmd->flags);

    UserFile *pUFile = NULL;
    PeerFile *pPFile = NULL;

    try {
        // Check existing user files
        pUFile = ufileFindPath(pCmd->path);

        if (!pUFile) {
            // Create a new user file
            pUFile = new UserFile(
                    pCmd->path,
                    pCmd->flags);

            // Add user file
            err = ufileAdd(pUFile);
            if (err) {
                LOGE("Failed to add user file %s", pCmd->path);
                throw(err);
            }

            // Create a new peer file
            pPFile = new PeerFile(
                    pHdr->ucOrig,
                    pCmd->cookie,
                    pCmd->flags);

            // Add peer file to user file
            err = pUFile->pfileAdd(pPFile);
            if (err) {
                LOGE("Failed to add peer file to user file %s", pCmd->path);
                throw(err);
            }

            // Open user file process
            err = pUFile->fileOpen();
            if (err) {
                LOGE("Failed to open user file %s", pCmd->path);
                throw(err);
            }

            LOGI("Opened new user file %s", pCmd->path);
        }
        else {
            LOGD("Found existing user file %s", pCmd->path);

            // Check existing peer files
            pPFile = pUFile->pfileFind(pHdr->ucOrig, pCmd->cookie);

            if (!pPFile) {
                // Create a new peer file
                pPFile = new PeerFile(
                        pHdr->ucOrig,
                        pCmd->cookie,
                        pCmd->flags);

                // Add peer file to user file
                err = pUFile->pfileAdd(pPFile);
                if (err) {
                    LOGE("Failed to add peer file to user file %s", pCmd->path);
                    pUFile = NULL; // don't delete existing user file
                    throw(err);
                }
            }
        }
    }
    catch (int exception) {
        if (pUFile) {
            if (pPFile) {
                pUFile->pfileRmv(pPFile);
                delete pPFile;
            }
            ufileRmv(pUFile);
            delete pUFile;
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
    struct uappd_msg_file_open_rpy *pRpy =
        (struct uappd_msg_file_open_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

    // pRpy->cookie = pCmd->cookie
    // pRpy->path = pCmd->path
    pRpy->retVal = err;

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = TZIOC_CLIENT_ID_UAPPD;
    pHdr->ulLen  = sizeof(*pRpy);

    tzioc_msg_send(hClient, pHdr);
}

void UserAppDmon::ufileCloseProc(
    struct tzioc_msg_hdr *pHdr)
{
    int err = 0;

    struct uappd_msg_file_close_cmd *pCmd =
        (struct uappd_msg_file_close_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("Invalid user file close cmd received");
        return;
    }

    LOGD("User file close cmd received, path %s", pCmd->path);

    UserFile *pUFile = NULL;
    PeerFile *pPFile = NULL;

    try {
        // Check existing user files
        pUFile = ufileFindPath(pCmd->path);

        if (!pUFile) {
            LOGE("Failed to find existing user file %s", pCmd->path);
            throw((int)-ENOENT);
        }

        LOGD("Found existing user file %s", pCmd->path);

        // Check existing peer files
        pPFile = pUFile->pfileFind(pHdr->ucOrig, pCmd->cookie);

        if (!pPFile) {
            LOGE("Failed to find existing peer file for user file %s", pCmd->path);
            throw((int)-ENOENT);
        }

        LOGD("Found existing peer file for user file %s", pCmd->path);

        // Remove peer file
        pUFile->pfileRmv(pPFile);
        delete pPFile;

        LOGD("Removed existing peer file for user file %s", pCmd->path);

        // Close user file process
        if (pUFile->pfileCnt == 0) {
            err = pUFile->fileClose();
            if (err) {
                LOGE("Failed to close user file %s", pCmd->path);
                throw(err);
            }

            LOGI("Closed user file %s", pCmd->path);

            ufileRmv(pUFile);
            delete pUFile;
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
    struct uappd_msg_file_close_rpy *pRpy =
        (struct uappd_msg_file_close_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

    // pRpy->cookie = pCmd->cookie
    // pRpy->path = pCmd->path
    pRpy->retVal = err;

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = TZIOC_CLIENT_ID_UAPPD;
    pHdr->ulLen  = sizeof(*pRpy);

    tzioc_msg_send(hClient, pHdr);
}

void UserAppDmon::ufileWriteProc(
    struct tzioc_msg_hdr *pHdr)
{
    int err = 0;

    struct uappd_msg_file_write_cmd *pCmd =
        (struct uappd_msg_file_write_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("Invalid user file write cmd received");
        return;
    }

    LOGD("User file write cmd received, path %s", pCmd->path);

    UserFile *pUFile = NULL;
    PeerFile *pPFile = NULL;

    try {
        // Check existing user files
        pUFile = ufileFindPath(pCmd->path);

        if (!pUFile) {
            LOGE("Failed to find existing user file %s", pCmd->path);
            throw((int)-ENOENT);
        }

        LOGD("Found existing user file %s", pCmd->path);

        // Check existing peer files
        pPFile = pUFile->pfileFind(pHdr->ucOrig, pCmd->cookie);

        if (!pPFile) {
            LOGE("Failed to find existing peer file for user file %s", pCmd->path);
            throw((int)-ENOENT);
        }

        LOGD("Found existing peer file for user file %s", pCmd->path);

        // Write to file
        err = pUFile->fileWrite(pPFile, pCmd->paddr, pCmd->bytes);
        if (err < 0) {
            LOGE("Failed to write to user file %s", pCmd->path);
            throw(err);
        }

        LOGD("Wrote to user file %s", pCmd->path);
    }
    catch (int exception) {
        err = exception;
    }
    catch (...) {
        LOGI("Unhandled exception");
        err = -ENOENT;
    }

    // Caution: reused the cmd buffer for rpy
    struct uappd_msg_file_write_rpy *pRpy =
        (struct uappd_msg_file_write_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

    // pRpy->cookie = pCmd->cookie
    // pRpy->path = pCmd->path
    pRpy->retVal = err;

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = TZIOC_CLIENT_ID_UAPPD;
    pHdr->ulLen  = sizeof(*pRpy);

    tzioc_msg_send(hClient, pHdr);
}

void UserAppDmon::ufileReadProc(
    struct tzioc_msg_hdr *pHdr)
{
    int err = 0;

    struct uappd_msg_file_read_cmd *pCmd =
        (struct uappd_msg_file_read_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("Invalid user file get id cmd received");
        return;
    }

    LOGD("User file read cmd received, path %s", pCmd->path);

    UserFile *pUFile = NULL;
    PeerFile *pPFile = NULL;

    try {
        // Check existing user files
        pUFile = ufileFindPath(pCmd->path);

        if (!pUFile) {
            LOGE("Failed to find existing user file %s", pCmd->path);
            throw((int)-ENOENT);
        }

        LOGD("Found existing user file %s", pCmd->path);

        // Check existing peer files
        pPFile = pUFile->pfileFind(pHdr->ucOrig, pCmd->cookie);

        if (!pPFile) {
            LOGE("Failed to find existing peer file for user file %s", pCmd->path);
            throw((int)-ENOENT);
        }

        LOGD("Found existing peer file for user file %s", pCmd->path);

        // Read from file
        err = pUFile->fileRead(pPFile, pCmd->paddr, pCmd->bytes);
        if (err < 0) {
            LOGE("Failed to write to user file %s", pCmd->path);
            throw(err);
        }

        LOGD("Read from user file %s", pCmd->path);
    }
    catch (int exception) {
        err = exception;
    }
    catch (...) {
        LOGI("Unhandled exception");
        err = -ENOENT;
    }

    // Caution: reused the cmd buffer for rpy
    struct uappd_msg_file_read_rpy *pRpy =
        (struct uappd_msg_file_read_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

    // pRpy->cookie = pCmd->cookie
    // pRpy->path = pCmd->path
    pRpy->retVal = err;

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = TZIOC_CLIENT_ID_UAPPD;
    pHdr->ulLen  = sizeof(*pRpy);

    tzioc_msg_send(hClient, pHdr);
}

int UserAppDmon::ufileAdd(UserFile *pUFile)
{
    for (uint32_t i = 0; i < UFILE_NUM_MAX; i++) {
        if (!pUFiles[i]) {
            pUFiles[i] = pUFile;
            ufileCnt++;
            return 0;
        }
    }
    return -ENOMEM;
}

int UserAppDmon::ufileRmv(UserFile *pUFile)
{
    for (uint32_t i = 0; i < UFILE_NUM_MAX; i++) {
        if (pUFiles[i] == pUFile) {
            pUFiles[i] = NULL;
            ufileCnt--;
            return 0;
        }
    }
    return -ENOENT;
}

UserAppDmon::UserFile *UserAppDmon::ufileFindPath(string path)
{
    for (uint32_t i = 0; i < UFILE_NUM_MAX; i++) {
        if (pUFiles[i] &&
            pUFiles[i]->path == path)
            return pUFiles[i];
    }
    return NULL;
}

int UserAppDmon::UserFile::fileOpen()
{
    int ret = 0;

    // Open all files read/write
    fd = open(path.c_str(), flags | O_CLOEXEC, S_IRWXU);
    if (fd == -1)
        ret = -errno;
    return ret;
}

int UserAppDmon::UserFile::fileClose()
{
    int ret = 0;

    ret = close(fd);
    if (ret == -1)
        ret = -errno;
    return ret;
}

int UserAppDmon::UserFile::fileWrite(
    UserAppDmon::PeerFile *pPFile,
    uintptr_t paddr,
    size_t bytes)
{
    int ret = 0;

    if (pPFile->access & O_RDONLY)
        return -EACCES;

    // Map physical address
    void *pBuff = tzioc_map_paddr(
        UserAppDmon::hClient,
        paddr,
        bytes,
        TZIOC_MEM_RD_ONLY);

    if (!pBuff) {
        LOGE("Failed to map physical addr 0x%zx, bytes 0x%zx",
             paddr, bytes);
        return -ENOMEM;
    }

    // Switch peer file if necessary
    if (pPFile != pPFileLast) {
        lseek(fd, pPFile->offset, SEEK_SET);
        pPFileLast = pPFile;
    }

    // Write to file
    ssize_t wbytes = write(fd, pBuff, bytes);
    if (wbytes == -1)
        ret = -errno;
    else
        ret = wbytes;

    // Update peer file info
    pPFile->offset += wbytes;

    // Unmap physical address
    tzioc_unmap_paddr(
        UserAppDmon::hClient,
        paddr,
        bytes);

    return ret;
}

int UserAppDmon::UserFile::fileRead(
    UserAppDmon::PeerFile *pPFile,
    uintptr_t paddr,
    size_t bytes)
{
    int ret = 0;

    if (pPFile->access & O_RDONLY)
        return -EACCES;

    // Map physical address
    void *pBuff = tzioc_map_paddr(
        UserAppDmon::hClient,
        paddr,
        bytes,
        0);

    if (!pBuff) {
        LOGE("Failed to map physical addr 0x%zx, bytes 0x%zx",
             paddr, bytes);
        return -ENOMEM;
    }

    // Switch peer file if necessary
    if (pPFile != pPFileLast) {
        lseek(fd, pPFile->offset, SEEK_SET);
        pPFileLast = pPFile;
    }

    // Read from file
    ssize_t rbytes = read(fd, pBuff, bytes);
    if (rbytes == -1)
        ret = -errno;
    else
        ret = rbytes;

    // Update peer file info
    pPFile->offset += rbytes;

    // Unmap physical address
    tzioc_unmap_paddr(
        UserAppDmon::hClient,
        paddr,
        bytes);

    return ret;
}

int UserAppDmon::UserFile::pfileAdd(PeerFile *pPFile)
{
    for (uint32_t i = 0; i < PFILE_NUM_MAX; i++) {
        if (!pPFiles[i]) {
            pPFiles[i] = pPFile;
            pfileCnt++;
            return 0;
        }
    }
    return -ENOMEM;
}

int UserAppDmon::UserFile::pfileRmv(PeerFile *pPFile)
{
    for (uint32_t i = 0; i < PFILE_NUM_MAX; i++) {
        if (pPFiles[i] == pPFile) {
            pPFiles[i] = NULL;
            pfileCnt--;
            return 0;
        }
    }
    return -ENOENT;
}

UserAppDmon::PeerFile *UserAppDmon::UserFile::pfileFind(
    uint8_t id,
    uintptr_t cookie)
{
    for (uint32_t i = 0; i < PFILE_NUM_MAX; i++) {
        if (pPFiles[i] &&
            pPFiles[i]->id == id &&
            pPFiles[i]->cookie == cookie) {
            return pPFiles[i];
        }
    }
    return NULL;
}
