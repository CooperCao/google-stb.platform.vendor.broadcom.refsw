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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include "bstd.h"
#include "bkni.h"

#include "file_manager_priv.h"

void file_manager_get_default_create_settings(FileManagerCreateSettings * pSettings)
{
    assert(pSettings);
    memset(pSettings, 0, sizeof(*pSettings));
}

FileManagerHandle file_manager_create(const FileManagerCreateSettings * pSettings)
{
    FileManagerHandle manager;

    assert(pSettings);
    assert(pSettings->path);

    manager = malloc(sizeof(*manager));
    if (!manager) goto error;
    memset(manager, 0, sizeof(*manager));
    if (pSettings->name)
    {
        manager->name = malloc(strlen(pSettings->name) + 1);
        if (!manager->name) goto error;
        strcpy(manager->name, pSettings->name);
    }
    manager->root.path = malloc(strlen(pSettings->path) + 1);
    if (!manager->root.path) goto error;
    strcpy(manager->root.path, pSettings->path);
    BLST_Q_INIT(&manager->files);
    memcpy(&manager->createSettings, pSettings, sizeof(*pSettings));

    if (file_manager_p_load(manager, pSettings->filter)) goto error;

    return manager;

error:
    file_manager_destroy(manager);
    return NULL;
}

void file_manager_destroy(FileManagerHandle manager)
{
    FileManagerFile * pFile;
    if (!manager) return;
    for (pFile = BLST_Q_FIRST(&manager->files); pFile; pFile = BLST_Q_FIRST(&manager->files))
    {
        BLST_Q_REMOVE(&manager->files, pFile, link);
        if (pFile->path)
        {
            free(pFile->path);
            pFile->path = NULL;
        }
        free(pFile);
    }
    if (manager->name)
    {
        free(manager->name);
        manager->name = NULL;
    }
    if (manager->root.path)
    {
        free(manager->root.path);
        manager->root.path = NULL;
    }
    if (manager->root.dp)
    {
        closedir(manager->root.dp);
        manager->root.dp = NULL;
    }
    free(manager);
}

void file_manager_p_insert_sort(FileManagerHandle manager, FileManagerFile * pFile)
{
    FileManagerFile * f;

    assert(manager);
    assert(pFile);

    if (BLST_Q_EMPTY(&manager->files))
    {
        BLST_Q_INSERT_HEAD(&manager->files, pFile, link);
    }
    else
    {
        for (f = BLST_Q_FIRST(&manager->files); f; f = BLST_Q_NEXT(f, link))
        {
            if (strcoll(f->path, pFile->path) > 0)
            {
                BLST_Q_INSERT_BEFORE(&manager->files, f, pFile, link);
                break;
            }
        }
        if (!f) BLST_Q_INSERT_TAIL(&manager->files, pFile, link);
    }
}

int file_manager_p_add_path(FileManagerHandle manager, const char * path, FileFilter filter)
{
    struct stat stats;
    FILE * f;
    FileManagerFile * file;

    assert(manager);
    assert(path);

    if (stat(path, &stats) == -1) { printf("%s: can't stat '%s'; skipping\n", manager->name, path); return -1; }
    if (S_ISDIR(stats.st_mode)) { printf("%s: '%s' is directory; skipping\n", manager->name, path); return -1; }
    if (filter && !filter(path)) { printf("%s: '%s' filtered out\n", manager->name, path); return -1; }
    f = fopen(path, "r");
    if (!f) { printf ("%s: could not open file: '%s'; skipping\n", manager->name, path); return -1; }
    fclose(f);
    file = malloc(sizeof(*file));
    if (!file) { printf("%s: coult not allocate file structure; skipping '%s'\n", manager->name, path); return -1; }
    memset(file, 0, sizeof(*file));
    file->path = malloc(strlen(path) + 1);
    if (!file->path) { printf("%s: coult not allocate file path space; skipping '%s'\n", manager->name, path); return -1; }
    strcpy(file->path, path);
    file_manager_p_insert_sort(manager, file);
    manager->fileCount++;
    printf("%s: added '%s'\n", manager->name, path);
    return 0;
}

int file_manager_p_load(FileManagerHandle manager, FileFilter filter)
{
    struct dirent * de;
    static char buf[MAX_PATH_LEN];

    assert(manager);

    manager->root.dp = opendir(manager->root.path);
    if (!manager->root.dp) { printf("%s: p_load: could not open dir: '%s'\n", manager->name, manager->root.path); return -1; }

    while ((de = readdir(manager->root.dp)) != NULL)
    {
        if (!strcmp(de->d_name,".") || !strcmp(de->d_name,"..")) continue;
        memset(buf, 0, MAX_PATH_LEN);
        strcpy(buf, manager->root.path);
        strcat(buf, "/");
        strcat(buf, de->d_name);
        if (file_manager_p_add_path(manager, buf, filter)) continue;
    }

    return 0;
}

unsigned file_manager_get_count(FileManagerHandle manager)
{
    assert(manager);
    return manager->fileCount;
}

const char * file_manager_find(FileManagerHandle manager, const char * path)
{
    char * found = NULL;
    FileManagerFile * f;
    char fullpath[1024];

    strcpy(fullpath, manager->root.path);
    strcat(fullpath, "/");
    strcat(fullpath, path);

    for (f = BLST_Q_FIRST(&manager->files); f; f = BLST_Q_NEXT(f, link))
    {
        if (!strcmp(f->path, fullpath))
        {
            found = f->path;
            break;
        }
    }

    return found;
}
