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
#include "string_list_priv.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void string_list_print(StringListHandle list)
{
    StringListEntry * entry;

    printf("list@%p = {\n", (void *)list);
    for (entry = BLST_Q_FIRST(&list->entries); entry; entry = BLST_Q_NEXT(entry, link))
    {
        printf("\t%s\n", entry->value);
    }
    printf("}\n");
}

StringListHandle string_list_create(bool sorted)
{
    StringListHandle list = NULL;

    list = malloc(sizeof(*list));
    if (!list) goto error;
    memset(list, 0, sizeof(*list));
    BLST_Q_INIT(&list->entries);
    list->sorted = sorted;

    return list;

error:
    string_list_destroy(list);
    return NULL;
}

void string_list_destroy(StringListHandle list)
{
    StringListEntry * entry = NULL;

    for (entry = BLST_Q_FIRST(&list->entries); entry; entry = BLST_Q_FIRST(&list->entries))
    {
        BLST_Q_REMOVE(&list->entries, entry, link);
        if (entry->value) free(entry->value);
        free(entry);
    }
    free(list);
}

void string_list_p_insert_sort(StringListHandle list, StringListEntry * pEntry)
{
    StringListEntry * e;

    assert(list);
    assert(pEntry);

    if (BLST_Q_EMPTY(&list->entries))
    {
        BLST_Q_INSERT_HEAD(&list->entries, pEntry, link);
    }
    else
    {
        for (e = BLST_Q_FIRST(&list->entries); e; e = BLST_Q_NEXT(e, link))
        {
            if (strcoll(e->value, pEntry->value) > 0)
            {
                BLST_Q_INSERT_BEFORE(&list->entries, e, pEntry, link);
                break;
            }
        }
        if (!e) BLST_Q_INSERT_TAIL(&list->entries, pEntry, link);
    }
}

void string_list_add(StringListHandle list, const char * value)
{
    StringListEntry * pEntry;
    assert(value);
    pEntry = malloc(sizeof(*pEntry));
    memset(pEntry, 0, sizeof(*pEntry));
    pEntry->value = set_string(pEntry->value, value);
    if (list->sorted) string_list_p_insert_sort(list, pEntry);
    else BLST_Q_INSERT_TAIL(&list->entries, pEntry, link);
}

StringListCursorHandle string_list_create_cursor(StringListHandle list)
{
    StringListCursorHandle cursor = NULL;

    cursor = malloc(sizeof(*cursor));
    if (!cursor) goto error;
    cursor->list = list;

end:
    return cursor;

error:
    string_list_cursor_destroy(cursor);
    cursor = NULL;
    goto end;
}

void string_list_cursor_destroy(StringListCursorHandle cursor)
{
    if (cursor)
    {
        free(cursor);
    }
}

const char * string_list_cursor_first(StringListCursorHandle cursor)
{
    char * value = NULL;
    cursor->entry = BLST_Q_FIRST(&cursor->list->entries);
    if (cursor->entry)
    {
        value = cursor->entry->value;
    }
    return value;
}

const char * string_list_cursor_next(StringListCursorHandle cursor)
{
    char * value = NULL;
    cursor->entry = BLST_Q_NEXT(cursor->entry, link);
    if (cursor->entry)
    {
        value = cursor->entry->value;
    }
    return value;
}
