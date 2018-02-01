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
#include "name_value_file_parser_priv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

NameValueFileParserHandle name_value_file_parser_create(const char * filename, NameValueFileParserListener listener, void * listenerContext)
{
    NameValueFileParserHandle parser = NULL;
    FILE * in;

    assert(filename);

    in = fopen(filename, "r");
    if (!in) goto error;

    parser = malloc(sizeof(*parser));
    if (!parser) goto error;
    memset(parser, 0, sizeof(*parser));

    parser->in = in;
    parser->announce = listener;
    parser->listenerContext = listenerContext;

    return parser;

error:
    name_value_file_parser_destroy(parser);
    return NULL;
}

void name_value_file_parser_parse(NameValueFileParserHandle parser)
{
    char * name;
    char * value;
    char * p;
    int nlen;
    int result;

    assert(parser);

    while (!feof(parser->in))
    {
        memset(parser->line, 0, LINE_LEN);
        if (!fgets(parser->line, LINE_LEN, parser->in)) break;

        /* get rid of newline */
        p = strchr(parser->line, '\n');
        if (p) *p = 0;
        /* get rid of comments and everything after */
        p = strchr(parser->line, '#');
        if (p) *p = 0;

        p = strchr(parser->line, '=');

        name = parser->line;
        value = NULL;
        if (p)
        {
            *p = 0;
            value = p + 1;
            /* trim whitespace */
            value = trim(value);
        }
        name = trim(name);
        nlen = strlen(name);

        if (nlen && parser->announce)
        {
            result = parser->announce(parser->listenerContext, name, value);
            if (result < 0) break;
        }
    }
}

void name_value_file_parser_destroy(NameValueFileParserHandle parser)
{
    if (parser)
    {
        if (parser->in) fclose(parser->in);
        free(parser);
    }
}
