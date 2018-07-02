/******************************************************************************
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
 ******************************************************************************/
#include <stddef.h>

/***************************************************************************
 * Summary:
 * Parsed URI data
 ***************************************************************************/
typedef struct bmon_uri {
    char   protocol[16];
    char   host[1024];
    int    port;
    char   path[1024];
    char   query[1024];
    char   fragment[1024];
} bmon_uri;

/***************************************************************************
 * Summary:
 * Parse the given strURI URI string and return parsed values in the
 * uriParsed struct.  Returns 0 on success, otherwise failure.
 ***************************************************************************/
int bmon_uri_parse(const char * strURI, bmon_uri * uriParsed);
/***************************************************************************
 * Summary:
 * Find tag from the uri string and return 1 if found, 0 otherwise
 * Input starts after plugin name
 ***************************************************************************/
int bmon_uri_find_tag(const char * strURI, char * tag);

/***************************************************************************
 * Summary:
 * Find value of the tag. returns 1 if found, 0 otherwise.
 ***************************************************************************/
int bmon_uri_find_tagvalue(const char * strURI, const char * tag, char * val, size_t valSize);

/***************************************************************************
 * Summary:
 * Decode hex encoded unsafe URI characters
 ***************************************************************************/
int bmon_uri_decode(const char * strURI, char * result, size_t resultSize);

/***************************************************************************
 * Summary:
 * Encode unsafe URI characters as hex
 ***************************************************************************/
int bmon_uri_encode(const char * strURI, char * result, size_t resultSize);

/***************************************************************************
 * Summary:
 * Combine path and query and fill given string buffer
 ***************************************************************************/
void bmon_uri_getPathQuery(
        const bmon_uri * pUri,
        char *           strPathQuery,
        size_t           sizePathQuery
        );