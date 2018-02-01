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
#include "string_map_priv.h"
#include "name_value_file_parser.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int string_map_p_handle_nvp(void * ctx, const char * name, const char * value)
{
    StringMapHandle map = ctx;
    assert(name && value);
    string_map_p_put(map, name, value);
    return 0;
}

void string_map_print(StringMapHandle map)
{
    StringPairListEntry * entry;

    printf("map@%p = {\n", (void *)map);
    for (entry = BLST_Q_FIRST(&map->entries); entry; entry = BLST_Q_NEXT(entry, link))
    {
        printf("\t%s = %s\n", entry->pair.key, entry->pair.value);
    }
    printf("}\n");
}

StringMapHandle string_map_deserialize(const char * mapFilename)
{
    StringMapHandle map = NULL;
    NameValueFileParserHandle parser = NULL;

    map = malloc(sizeof(*map));
    if (!map) goto error;
    memset(map, 0, sizeof(*map));

    BLST_Q_INIT(&map->entries);

    parser = name_value_file_parser_create(mapFilename, &string_map_p_handle_nvp, map);
    if (!parser) goto error;

    name_value_file_parser_parse(parser);

    name_value_file_parser_destroy(parser);

    return map;

error:
    string_map_destroy(map);
    return NULL;
}

void string_map_destroy(StringMapHandle map)
{
    StringPairListEntry * entry = NULL;

    for (entry = BLST_Q_FIRST(&map->entries); entry; entry = BLST_Q_FIRST(&map->entries))
    {
        BLST_Q_REMOVE(&map->entries, entry, link);
        if (entry->pair.key) free(entry->pair.key);
        if (entry->pair.value) free(entry->pair.value);
        free(entry);
    }
    free(map);
}

static StringPairListEntry * string_map_p_find(StringMapHandle map, const char * key)
{
    StringPairListEntry * entry = NULL;
    StringPairListEntry * found = NULL;

    for (entry = BLST_Q_FIRST(&map->entries); entry; entry = BLST_Q_NEXT(entry, link))
    {
        if (!strcmp(entry->pair.key, key))
        {
            found = entry;
            break;
        }
    }

    return found;
}

void string_map_p_put(StringMapHandle map, const char * key, const char * value)
{
    StringPairListEntry * entry;

    assert(key);
    assert(value);

    entry = string_map_p_find(map, key);

    if (!entry)
    {
        entry = malloc(sizeof(*entry));
        memset(entry, 0, sizeof(*entry));
        entry->pair.key = set_string(entry->pair.key, key);
        BLST_Q_INSERT_TAIL(&map->entries, entry, link);
    }

    entry->pair.value = set_string(entry->pair.value, value);
}

const char * string_map_get(StringMapHandle map, const char * key)
{
    const char * value = NULL;
    StringPairListEntry * entry = NULL;

    assert(key);

    entry = string_map_p_find(map, key);

    if (entry)
    {
        value = entry->pair.value;
    }

    return value;
}

StringMapCursorHandle string_map_create_key_cursor(StringMapHandle map)
{
    StringMapCursorHandle cursor = NULL;

    cursor = malloc(sizeof(*cursor));
    if (!cursor) goto error;
    cursor->map = map;

end:
    return cursor;

error:
    string_map_cursor_destroy(cursor);
    cursor = NULL;
    goto end;
}

void string_map_cursor_destroy(StringMapCursorHandle cursor)
{
    if (cursor)
    {
        free(cursor);
    }
}

const StringPair * string_map_cursor_first(StringMapCursorHandle cursor)
{
    StringPair * pair = NULL;
    cursor->entry = BLST_Q_FIRST(&cursor->map->entries);
    if (cursor->entry)
    {
        pair = &cursor->entry->pair;
    }
    return pair;
}

const StringPair * string_map_cursor_next(StringMapCursorHandle cursor)
{
    StringPair * pair = NULL;
    cursor->entry = BLST_Q_NEXT(cursor->entry, link);
    if (cursor->entry)
    {
        pair = &cursor->entry->pair;
    }
    return pair;
}
