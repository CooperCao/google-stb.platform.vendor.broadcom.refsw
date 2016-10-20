/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "file_switcher_priv.h"

static void file_switcher_reset(FileSwitcherHandle switcher)
{
    assert(switcher);
    printf("%s: switcher reset\n", switcher->name);
    switcher->position = -1;
    switcher->pCurrent = NULL;
}

FileSwitcherHandle file_switcher_create(const char * name, const char * path, FileFilter filter, bool loop)
{
    FileSwitcherHandle switcher;

    assert(path);

    switcher = malloc(sizeof(*switcher));
    if (!switcher) goto error;
    memset(switcher, 0, sizeof(*switcher));
    if (name)
    {
        switcher->name = malloc(strlen(name) + 1);
        if (!switcher->name) goto error;
        strcpy(switcher->name, name);
    }
    switcher->root.path = malloc(strlen(path) + 1);
    if (!switcher->root.path) goto error;
    strcpy(switcher->root.path, path);
    BLST_Q_INIT(&switcher->files);
    switcher->loop = loop;

    if (file_switcher_p_load(switcher, filter)) goto error;

    file_switcher_reset(switcher);

    return switcher;

error:
    file_switcher_destroy(switcher);
    return NULL;
}

void file_switcher_destroy(FileSwitcherHandle switcher)
{
    FileSwitcherFile * pFile;
    if (!switcher) return;
    for (pFile = BLST_Q_FIRST(&switcher->files); pFile; pFile = BLST_Q_FIRST(&switcher->files))
    {
        BLST_Q_REMOVE(&switcher->files, pFile, link);
        if (pFile->path)
        {
            free(pFile->path);
            pFile->path = NULL;
        }
        free(pFile);
    }
    if (switcher->name)
    {
        free(switcher->name);
        switcher->name = NULL;
    }
    if (switcher->root.path)
    {
        free(switcher->root.path);
        switcher->root.path = NULL;
    }
    if (switcher->root.dp)
    {
        closedir(switcher->root.dp);
        switcher->root.dp = NULL;
    }
    free(switcher);
}

void file_switcher_p_insert_sort(FileSwitcherHandle switcher, FileSwitcherFile * pFile)
{
    FileSwitcherFile * f;

    assert(switcher);
    assert(pFile);

    if (BLST_Q_EMPTY(&switcher->files))
    {
        BLST_Q_INSERT_HEAD(&switcher->files, pFile, link);
    }
    else
    {
        for (f = BLST_Q_FIRST(&switcher->files); f; f = BLST_Q_NEXT(f, link))
        {
            if (strcoll(f->path, pFile->path) > 0)
            {
                BLST_Q_INSERT_BEFORE(&switcher->files, f, pFile, link);
                break;
            }
        }
        if (!f) BLST_Q_INSERT_TAIL(&switcher->files, pFile, link);
    }
}

int file_switcher_p_add_path(FileSwitcherHandle switcher, const char * path, FileFilter filter)
{
    struct stat stats;
    FILE * f;
    FileSwitcherFile * file;

    assert(switcher);
    assert(path);

    if (stat(path, &stats) == -1) { printf("%s: can't stat '%s'; skipping\n", switcher->name, path); return -1; }
    if (S_ISDIR(stats.st_mode)) { printf("%s: '%s' is directory; skipping\n", switcher->name, path); return -1; }
    if (filter && !filter(path)) { printf("%s: '%s' filtered out\n", switcher->name, path); return -1; }
    f = fopen(path, "r");
    if (!f) { printf ("%s: could not open file: '%s'; skipping\n", switcher->name, path); return -1; }
    fclose(f);
    file = malloc(sizeof(*file));
    if (!file) { printf("%s: coult not allocate file structure; skipping '%s'\n", switcher->name, path); return -1; }
    memset(file, 0, sizeof(*file));
    file->path = malloc(strlen(path) + 1);
    if (!file->path) { printf("%s: coult not allocate file path space; skipping '%s'\n", switcher->name, path); return -1; }
    strcpy(file->path, path);
    file_switcher_p_insert_sort(switcher, file);
    switcher->fileCount++;
    printf("%s: added '%s'\n", switcher->name, path);
    return 0;
}

int file_switcher_p_load(FileSwitcherHandle switcher, FileFilter filter)
{
    struct dirent * de;
    static char buf[MAX_PATH_LEN];

    assert(switcher);

    switcher->root.dp = opendir(switcher->root.path);
    if (!switcher->root.dp) { printf("%s: p_load: could not open dir: '%s'\n", switcher->name, switcher->root.path); return -1; }

    while ((de = readdir(switcher->root.dp)) != NULL)
    {
        if (!strcmp(de->d_name,".") || !strcmp(de->d_name,"..")) continue;
        memset(buf, 0, MAX_PATH_LEN);
        strcpy(buf, switcher->root.path);
        strcat(buf, "/");
        strcat(buf, de->d_name);
        if (file_switcher_p_add_path(switcher, buf, filter)) continue;
    }

    return 0;
}

void file_switcher_first(FileSwitcherHandle switcher)
{
    assert(switcher);
    switcher->pCurrent = BLST_Q_FIRST(&switcher->files);
    switcher->position = 0;
}
void file_switcher_last(FileSwitcherHandle switcher)
{
    assert(switcher);
    switcher->pCurrent = BLST_Q_LAST(&switcher->files);
    switcher->position = switcher->fileCount - 1;
}

void file_switcher_next(FileSwitcherHandle switcher)
{
    assert(switcher);
    if (BLST_Q_EMPTY(&switcher->files)) { printf("%s: next: no files found\n", switcher->name); return; }
    /* if we come in after being reset */
    if (!switcher->pCurrent || (switcher->position < 0))
    {
        file_switcher_first(switcher);
    }
    /* if we can't go next */
    else if (!BLST_Q_NEXT(switcher->pCurrent, link))
    {
        if (switcher->loop) { file_switcher_first(switcher); }
        else { printf("%s: next: end of list\n", switcher->name); }
    }
    else
    {
        /* otherwise, move along */
        switcher->pCurrent = BLST_Q_NEXT(switcher->pCurrent, link);
        switcher->position++;
    }
}

void file_switcher_prev(FileSwitcherHandle switcher)
{
    assert(switcher);
    if (BLST_Q_EMPTY(&switcher->files)) { printf("%s: prev: no files found\n", switcher->name); return; }
    /* if we come in after being reset */
    if (!switcher->pCurrent)
    {
        file_switcher_last(switcher);
    }
    /* if we can't go prev */
    else if (!BLST_Q_PREV(switcher->pCurrent, link))
    {
        if (switcher->loop) { file_switcher_last(switcher); }
        else { printf("%s: prev: start of list\n", switcher->name); }
    }
    else
    {
        /* otherwise, move along */
        switcher->pCurrent = BLST_Q_PREV(switcher->pCurrent, link);
        switcher->position--;
    }
}

void file_switcher_print(FileSwitcherHandle switcher)
{
    assert(switcher);
    if (!switcher->pCurrent) { printf("%s: print: no current file selected\n", switcher->name); return; }
    printf("%s: selected '%s' file\n", switcher->name, switcher->pCurrent->path);
}

const char * file_switcher_get_path(FileSwitcherHandle switcher)
{
    assert(switcher);
    if (!switcher->pCurrent) { printf("%s: get_path: no current file selected\n", switcher->name); return NULL; }
    return switcher->pCurrent->path;
}

int file_switcher_get_position(FileSwitcherHandle switcher)
{
    assert(switcher);
    if (!switcher->pCurrent) { printf("%s: get_position: no current file selected\n", switcher->name); return -1; }
    return switcher->position;
}

unsigned file_switcher_get_count(FileSwitcherHandle switcher)
{
    assert(switcher);
    return switcher->fileCount;
}

void file_switcher_set_position(FileSwitcherHandle switcher, int position)
{
    assert(switcher);
    if (position < 0) { file_switcher_reset(switcher); return; }
    if (position >= switcher->fileCount)  { printf("%s: set_position: position %u out of bounds (%u)\n", switcher->name, position, switcher->fileCount); return; }
    if (switcher->position < position)
    {
        while (switcher->position < position)
        {
            file_switcher_next(switcher);
            assert(switcher->pCurrent);
        }
    }
    else if (switcher->position > position)
    {
        while (switcher->position > position)
        {
            file_switcher_prev(switcher);
            assert(switcher->pCurrent);
        }
    }
    printf("%s: set to position %u (%u)\n", switcher->name, position, switcher->fileCount);
}
