/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#ifndef __POWER_UTILS_H__
#define __POWER_UTILS_H__

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <power.h>

#include "bmon_json.h"

#define GAP_BETWEEN_COLUMNS      50
#define GAP_BETWEEN_ROWS         10
#define RECTANGLE_WIDTH          ( 130+GAP_BETWEEN_COLUMNS )
#define RECTANGLE_HEIGHT         40
#define TREE_FILE                "./plugins/power_tree.txt"
#define CLK_TREE_FILE            "./plugins/power_clk_tree.txt"
#define CLOCK_HTML_FILE          "power.html"
#define REGISTER_NAME_LEN_MAX    128
#define FIELD_NAME_LEN_MAX       48
#define PRINTF                   noprintf
#define TREE_FILE_FULL_PATH_LEN  64
#define CLOCK_FILE_FULL_PATH_LEN 64

typedef struct CLK_TREE_NODE
{
    int           X;
    int           Y;
    char          name[REGISTER_NAME_LEN_MAX];
    char          field[FIELD_NAME_LEN_MAX];
    unsigned int  value;
    unsigned char polarity;
    unsigned char showBlock;
    unsigned char offOrOn;
    unsigned char level;
    void         *subLevel;
    void         *sameLevel;
    void         *parent;
} CLK_TREE_NODE;

#define COLOR_POWER_ON  "lightgray"
#define COLOR_POWER_OFF "#00FF00"
#define COLOR_NON_POWER "#F3E2A9"

typedef enum
{
    BG_HOTREGISTER,
    BG_GREEN,
    BG_NONPOWER
} BG_COLORS;

typedef enum
{
    SUBLEVEL,
    SAMELEVEL
} CLK_TREE_LEVEL;

typedef enum
{
    DRAW_RECTANGLES,
    DRAW_LINES,
    COMPUTE_FREQUENCY,
    FREE_NODES
} CLK_TREE_ACTION;

char *get_clock_tree(
    const char *queryString,
    cJSON *objectClkgen1,
    cJSON *objectClkgen2
    );

static char *get_shrink_list(
    void
    );

static int bmemperfOpenDriver( void );
static void *bmemperfMmap( int g_memFd );

#endif /*__POWER_UTILS_H__*/
