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
 *
 *****************************************************************************/

#ifndef __BPOWER_H__
#define __BPOWER_H__

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

#define GAP_BETWEEN_COLUMNS      50
#define GAP_BETWEEN_ROWS         10
#define RECTANGLE_WIDTH          ( 130+GAP_BETWEEN_COLUMNS )
#define RECTANGLE_HEIGHT         40
#define TREE_FILE                "bpower_tree.txt"
#define CLOCK_HTML_FILE          "bpower.html"
#define REGISTER_NAME_LEN_MAX    128
#define FIELD_NAME_LEN_MAX       48
#define PRINTF                   noprintf
#define FPRINTF                  nofprintf
#define TREE_FILE_FULL_PATH_LEN  64
#define CLOCK_FILE_FULL_PATH_LEN 64

typedef struct CLK_TREE_NODE
{
    int           X;
    int           Y;
    char          name[REGISTER_NAME_LEN_MAX];
    char          field[FIELD_NAME_LEN_MAX];
    unsigned char polarity;
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

typedef enum
{
    HighIsOff,
    HighIsOn,
    MULT,
    PREDIV,
    POSTDIV
} CLK_TREE_POLARITY;

typedef struct
{
    /* If the order of these fields changes, you have to also change the order in bpower2.awk */
    char              name[REGISTER_NAME_LEN_MAX];
    char              field[FIELD_NAME_LEN_MAX];
    unsigned char     polarity;
    unsigned long int regOffset;
    unsigned long int regMask;
    unsigned char     shiftCount;
} CLK_TREE_REG_INFO;

typedef struct
{
    unsigned long int regValue;
    CLK_TREE_POLARITY polarity;
} read_reg_field_outputs;

char *get_clock_tree(
    const char *queryString
    );

char *get_shrink_list(
    void
    );
#endif /*__BPOWER_H__*/
