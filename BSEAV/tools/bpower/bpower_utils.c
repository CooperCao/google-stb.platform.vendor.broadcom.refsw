/* TODO:
   How should we handle registers that are not accessable?
   How can we determine which registers are not accessable?
   CLKGEN_PLL_SYS0_PLL_DIV: This RDB field is not accessable by software; PDIV:reset value is 1; NDIV_INT:Reset value is 60.
*/
/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
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
******************************************************************************/
#include "bmemperf_types64.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "bmemperf_utils.h"
#include "bpower_utils.h"
#include "bchp_clkgen.h"

static unsigned long int           RectCount  = 0;
static FILE                       *outputHtml = NULL;
static CLK_TREE_NODE               ListHead;
static CLK_TREE_NODE              *workingMainNode           = NULL;
static unsigned int                LEVEL                     = 0;
static unsigned char               WalkTreeStructure1stPass  = 0; /* if 0 don't add 40 to beginning of block's Y coord */
static unsigned long int           mallocCount               = 0;
static unsigned long int           freeCount                 = 0;
static volatile unsigned int      *g_pMem                    = NULL;
static unsigned long int           blockHasAtLeastOnePowerOn = 0; /* used to determine if all of the lower-level clocks are turned off for a particular logic block */
static CLK_TREE_NODE              *g_freqMult                = NULL;
static CLK_TREE_NODE              *g_freqPreDiv              = NULL;
static CLK_TREE_NODE              *g_freqPostDiv             = NULL;

#include "bpower_regs.h"
#ifdef BCHP7250
#endif

/**
 *  Function: This function will safely output the name of the specified node. If the node is null, it will
 *  print "null" for the name.
 **/
static char *GetNodeName(
    CLK_TREE_NODE *node
    )
{
    if (node)
    {
        return( node->name );
    }

    return( "null" );
}

#if 0
static int OutputTree(
    CLK_TREE_NODE *node
    )
{
    unsigned char croutput = 0;

    CLK_TREE_NODE *next = node;

    fprintf( stderr, "%s: (%s;%p); SUB  ", __FUNCTION__, GetNodeName( next ), (void *) next );
    next = next->subLevel;
    while (next)
    {
        fprintf( stderr, "(%s;%p) -> %p;\n", GetNodeName( next ), (void *) next, (void *) next->subLevel );
        OutputTree( next );
        next     = next->subLevel;
        croutput = 1;
    }
    if (croutput==0) {fprintf( stderr, "\n" ); }

    next = node;
    fprintf( stderr, "%s: (%s;%p); SAME ", __FUNCTION__, GetNodeName( next ), (void *) next );
    croutput = 0;
    next     = next->sameLevel;
    while (next)
    {
        fprintf( stderr, "(%s;%p) -> %p;\n", GetNodeName( next ), (void *) next, (void *) next->sameLevel );
        OutputTree( next );
        next     = next->sameLevel;
        croutput = 1;
    }
    if (croutput==0) {fprintf( stderr, "\n" ); }

    return( 0 );
} /* OutputTree */

#endif /* if 0 */

/**
 *  Function: This function will add a new node to the existing clock tree doubly-linked list.
 **/
static CLK_TREE_NODE *AddNode(
    CLK_TREE_NODE *parent,
    char          *newNodeName,
    CLK_TREE_LEVEL level
    )
{
    char          *posDelimiter = NULL;
    char          *posField     = NULL;
    CLK_TREE_NODE *next         = NULL;
    CLK_TREE_NODE *newNode      = NULL;
    CLK_TREE_NODE *temp         = NULL;

    FPRINTF( stderr, "%s: parent (%s); %s; new (%s)\n", __FUNCTION__, parent->name, ( level==SUBLEVEL ) ? "SUB" : "SAME", newNodeName );
    posDelimiter = strchr( newNodeName, ':' );
    if (posDelimiter)
    {
        *posDelimiter = '\0';
        FPRINTF( stderr, "%s: found delimiter at end of regName; new (%s)\n", __FUNCTION__, newNodeName );

        posField     = posDelimiter+1;
        posDelimiter = strchr( posField, ':' );
        FPRINTF( stderr, "%s: potential regName (%s); posDelimiter %p\n", __FUNCTION__, posField, (void *) posDelimiter );
        if (posDelimiter)
        {
            *posDelimiter = '\0';
            FPRINTF( stderr, "%s: found delimiter at end of fieldName; new (%s)\n", __FUNCTION__, posField );
        }
        else
        {
            FPRINTF( stderr, "%s: no delimiter after fieldName; new (%s)\n", __FUNCTION__, posField );
        }
    }

    newNode = malloc( sizeof( CLK_TREE_NODE ));
    mallocCount++;
    memset( newNode, 0, sizeof( CLK_TREE_NODE ));
    FPRINTF( stderr, "(%p) malloc'ed for (%s) ... %s; count %lu\n", (void *) newNode, newNodeName, __FUNCTION__, mallocCount );

    strncpy( newNode->name, newNodeName, sizeof( newNode->name ) - 1 );
    if (posField)
    {
        strncpy( newNode->field, posField, sizeof( newNode->field ) - 1 );
    }
    if (level == SUBLEVEL)
    {
        temp = parent->subLevel;
        next = parent->subLevel;
        FPRINTF( stderr, "%s: next subLevel link for (%s;%p) is (%s;%p)\n", __FUNCTION__, GetNodeName( parent ), parent, GetNodeName( next ), next );
        while (temp) {
            FPRINTF( stderr, "skipping subLevel link (%s)\n", GetNodeName( temp ));
            if (temp->subLevel) {next = temp->subLevel; }
            temp = temp->subLevel;
        }

        if (next)
        {
            newNode->parent = next;
            next->subLevel  = newNode;
            FPRINTF( stderr, "%s: adding new subLevel node (%s%p) to (%s;%p); LEVEL %u\n", __FUNCTION__, GetNodeName( newNode ), newNode, GetNodeName( next ), next, LEVEL );
        }
        else
        {
            newNode->parent  = parent;
            parent->subLevel = newNode;
            FPRINTF( stderr, "%s: adding 1st subLevel node (%s;%p) to (%s;%p); LEVEL %u\n", __FUNCTION__, GetNodeName( newNode ), newNode, GetNodeName( parent ), parent, LEVEL );
        }
    }
    else if (level == SAMELEVEL)
    {
        temp = parent->sameLevel;
        next = parent->sameLevel;
        FPRINTF( stderr, "%s:%u: next (%s;%p) sameLevel link (%s;%p)\n", __FUNCTION__, __LINE__, GetNodeName( parent ), (void *) parent, GetNodeName( next ), (void *) next );
        while (temp) {
            FPRINTF( stderr, "%s;%u: skipping sameLevel link (%s;%p)\n", __FUNCTION__, __LINE__, GetNodeName( temp ), (void *) temp );
            if (temp->sameLevel) {next = temp->sameLevel; }
            temp = temp->sameLevel;
        }

        if (next)
        {
            newNode->parent = next;
            next->sameLevel = newNode;
            FPRINTF( stderr, "%s:%u: adding new sameLevel node (%s;%p) to (%s;%p); LEVEL %u\n", __FUNCTION__, __LINE__, GetNodeName( newNode ), (void *) newNode,
                GetNodeName( next ), (void *) next, LEVEL );
        }
        else
        {
            newNode->parent   = parent;
            parent->sameLevel = newNode;
            FPRINTF( stderr, "%s:%u: adding 1st sameLevel node (%s;%p) to (%s;%p); LEVEL %u\n", __FUNCTION__, __LINE__, GetNodeName( newNode ), (void *) newNode,
                GetNodeName( parent ), (void *) parent, LEVEL );
        }
    }
    else
    {
        fprintf( stderr, "%s: level (%u) is unknown\n", __FUNCTION__, level );
    }

    FPRINTF( stderr, "%s: adding node from (%s;%p) %s linked to -> (%s;%p)\n", __FUNCTION__, GetNodeName( parent ), parent,
        ( level==SUBLEVEL ) ? "SUB" : "SAME", GetNodeName( newNode ), newNode );

    return( newNode );
} /* AddNode */

/**
 *  Function: This function will read the contents of the specified file.
 **/
static char *ReadFileContents(
    const char *filename
    )
{
    struct stat       statbuf;
    char             *contents = NULL;
    unsigned long int numBytes = 0;
    FILE             *fpText   = NULL;

    memset( &statbuf, 0, sizeof( statbuf ));

    FPRINTF( outputHtml, "%s - calling lstat(%s)\n", __FUNCTION__, filename );

    if (lstat( filename, &statbuf ) == -1)
    {
        printf( "%s - (%s); lstat failed; %s\n", __FUNCTION__, filename, strerror( errno ));
        return( NULL );
    }

    FPRINTF( outputHtml, "size of (%s) %lu\n", filename, (unsigned long int) statbuf.st_size );

    contents = (char *) malloc( statbuf.st_size + 1 );
    if (contents == NULL)
    {
        fprintf( outputHtml, "could not malloc(%lu+1) bytes\n", (unsigned long int) statbuf.st_size );
        return( NULL );
    }
    FPRINTF( stderr, "(%p) malloc'ed ... %s; count %lu\n", (void *) contents, __FUNCTION__, mallocCount );

    memset( contents, 0, statbuf.st_size + 1 );

    fpText = fopen( filename, "r" );
    if (fpText == NULL)
    {
        fprintf( outputHtml, "could not fopen(%s)\n", filename );
        return( NULL );
    }
    numBytes = fread( contents, 1, statbuf.st_size, fpText );
    if (numBytes != (unsigned long int) statbuf.st_size)
    {
        fprintf( outputHtml, "tried to fread %lu bytes but got %lu\n", (unsigned long int) statbuf.st_size, numBytes );
        fclose( fpText );
        return( NULL );
    }

    fclose( fpText );

    FPRINTF( outputHtml, "%s: returning buffer of size (%lu)\n", __FUNCTION__, (unsigned long int) strlen( contents ));

    return( contents );
} /* ReadFileContents */

/**
 *  Function: This function will read the clock tree file, parse it, and create a doubly-linked list of the
 *  nodes in the clock tree.
 **/
static int ReadClockTreeFile(
    CLK_TREE_NODE *pListHead
    )
{
    char             *contents = NULL;
    char             *posstart = NULL;
    char             *posend   = NULL;
    unsigned long int lineNum  = 0;
    CLK_TREE_NODE    *node     = NULL;
    CLK_TREE_NODE    *temp     = NULL;
    char              treeFilename[TREE_FILE_FULL_PATH_LEN];

    strncpy ( treeFilename, TREE_FILE, sizeof(treeFilename) - 1 );
    contents = ReadFileContents( treeFilename );

    if (contents == NULL)
    {
        fprintf( stderr, "Could not read file (%s)\n", treeFilename );
        return( 0 );
    }

    posstart = contents; /* start parsing the file at the beginning of the file */

    /* step through the file line by line, searching for CLKGEN tags */
    do {
        posend = strstr( posstart, "\n" );
        if (posend)
        {
            *posend = '\0';
            posend++;
        }
        FPRINTF( stderr, "next line (%s); posend %p\n", posstart, posend );

        if (strlen( posstart ) > 0)
        {
            char        *posCLK    = strstr( posstart, "______C" );
            unsigned int lineLevel = 0;

            if (( 'A' <= *posstart ) && ( *posstart <= 'Z' ))
            {
                FPRINTF( stderr, "block (%lu); found new block (%s); line %lu\n", blockHasAtLeastOnePowerOn, posstart, lineNum );

                node  = AddNode( pListHead, posstart, SAMELEVEL );
                LEVEL = 0;
                temp  = node->parent;
                FPRINTF( stderr, "Step1: LEVEL %2u: NODE now points to (%s); parent (%s)\n", LEVEL, GetNodeName( node ), GetNodeName( temp ));
                workingMainNode = node;
            }
            else if (posCLK)
            {
                posCLK   += 6; /* set pointer to the C character ... skip over the underscores */
                lineLevel = ( posCLK - posstart ) / 8;

                FPRINTF( stderr, "\n\nCLK (%s) found at level %u; LEVEL %u; line %lu; NODE is (%s)\n", posCLK, lineLevel, LEVEL, lineNum, GetNodeName( node ));

                if (lineLevel == LEVEL)
                {
                    node = AddNode( node, posCLK, SAMELEVEL );
                    FPRINTF( stderr, "Step2: LEVEL %2u: NODE still points to (%s); added to parent (%s)\n", LEVEL, GetNodeName( node ), GetNodeName( node ));
                }
                else if (lineLevel > LEVEL)
                {
                    temp = node;
                    node = AddNode( node, posCLK, SUBLEVEL );
                    LEVEL++;
                    FPRINTF( stderr, "Step3: LEVEL %2u: NODE now points to (%s); parent (%s)\n", LEVEL, GetNodeName( node ), GetNodeName( temp ));
                }
                else
                {
                    unsigned int backupCount = LEVEL - lineLevel;
                    /* need to backup some number of levels */
                    CLK_TREE_NODE *prev = node /*->parent*/;

                    FPRINTF( stderr, "%s: need to backup (%u) levels\n", __FUNCTION__, backupCount );
                    while (prev && backupCount > 0) {
                        prev = prev->parent;
                        LEVEL--;
                        backupCount--;

                        FPRINTF( stderr, "%s: backed up to (%-22s); lineLevel %u; LEVEL %u\n", __FUNCTION__, prev->name, lineLevel, LEVEL );
                    }
                    LEVEL = lineLevel;
                    FPRINTF( stderr, "%s: add new node (%s) to (%s); LEVEL %u\n", __FUNCTION__, posCLK, GetNodeName( prev ), LEVEL );
                    node = AddNode( prev, posCLK, SAMELEVEL );
                }
            }
            else
            {
                FPRINTF( stderr, "ignored line (%s)\n", posstart );
            }
        }

        posstart = posend;

        lineNum++;
    } while (posstart != NULL);

    if (contents)
    {
        freeCount++;
        FPRINTF( stderr, "(%p) free'ed ... %s; count %lu\n", (void *) contents, __FUNCTION__, freeCount );
        Bsysperf_Free( contents );
    }

    return( 0 );
} /* ReadClockTreeFile */

/**
 *  Function: This function will create the HTML to display text for each rectangle for each node in the clock
 *  tree.
 **/
static void DrawText(
    int         X,
    int         Y,
    int         height,
    const char *description,
    const char *regName,
    const char *regField
    )
{
    int textX    = X + 10;
    int textY    = Y + height / 2 + 3; /* position text near the center of the rectangle */
    int fontSize = 8;

    /* if the name does NOT have CLKGEN in it, it is the upper-most logic block; make its font bigger */
    if (strstr( description, "CLKGEN" ) == NULL)
    {
        fontSize = 20;
    }
    fprintf( outputHtml, "<text onclick=\"MyClick(event);\" x=\"%u\" y=\"%u\" fill=\"black\" "
            "style=\"font-size:%upt;font-family:Courier New;\" id=%s%s%s >%s</text>\n", textX, textY, fontSize,
        regName, ( strlen( regField )) ? "-" : "", regField, description );
} /* DrawText */

static CLK_TREE_POLARITY GetRegisterPolarity(
    const char *regName,
    const char *fieldName
    )
{
    unsigned int      idx;
    CLK_TREE_POLARITY polarity = HighIsOff;

    FPRINTF( stderr, "%s: name (%-40s; field (%-30s)\n", __FUNCTION__, regName, fieldName );

    /* if the user provided strings for the register name and field name */
    if (regName && strlen( regName ) && fieldName && strlen( fieldName ))
    {
        for (idx = 0; idx < sizeof( registerInfo )/sizeof( registerInfo[0] ); idx++)
        {
            if (( strcmp( regName, registerInfo[idx].name ) == 0 ) && ( strcmp( fieldName, registerInfo[idx].field ) == 0 ))
            {
                polarity = registerInfo[idx].polarity;
                break;
            }
        }
    }

    return( polarity );
} /* GetRegisterPolarity */

#if 0
/**
 *  Function: This function will create a string with debug information about the specified register.
 **/
static unsigned int GetRegisterData(
    const char  *regName,
    const char  *fieldName,
    char        *outputStr,
    unsigned int outputStrLen
    )
{
    unsigned int idx;

    FPRINTF( stderr, "%s: name (%-40s; field (%-30s)\n", __FUNCTION__, regName, fieldName );

    /* if the user provided strings for the register name and field name */
    if (regName && strlen( regName ) && fieldName && strlen( fieldName ))
    {
        for (idx = 0; idx < sizeof( registerInfo )/sizeof( registerInfo[0] ); idx++)
        {
            if (( strcmp( regName, registerInfo[idx].name ) == 0 ) && ( strcmp( fieldName, registerInfo[idx].field ) == 0 ))
            {
                #if 0
                snprintf( outputStr, outputStrLen-1, "0x%08lX; m 0x%08lx; rc 0x%08lx", registerInfo[idx].regOffset, registerInfo[idx].regMask, registerInfo[idx].regOffset & registerInfo[idx].regMask );
                #else
                snprintf( outputStr, outputStrLen-1, "0x%08lX", registerInfo[idx].regOffset );
                #endif
                break;
            }
        }
    }

    return( 0 );
} /* GetRegisterData */

#endif /* if 0 */

/**
 *  Function: This function will read the specified register address offset and mask the value read with the
 *  specified register mask.
 **/
static unsigned int read_reg_value(
    volatile unsigned int *pMemTemp,
    unsigned long int regOffset,
    unsigned long int regMask
    )
{
    /*volatile unsigned int *lMemTemp = pMemTemp;*/
    unsigned long int           temp1 = 0;
    unsigned long int           temp2 = 0;
    unsigned long int           temp3 = 0;

    /*printf("%s: lMemTemp 0x%lx; regOffset 0x%08lx; regMask 0x%08lx; BCHP_REGISTER_START %x \n", __FUNCTION__,
        (long unsigned int) lMemTemp, regOffset, regMask, BCHP_REGISTER_START );*/
    pMemTemp += (( regOffset - BCHP_REGISTER_START )>>2 ); /* adding UINT32 to long unsigned int * results in addand being multiplied by 4 before the addition */
    /*printf("%s: pMemTemp 2 0x%08lx; (after regOffset 0x%08lx - BCHP_REGISTER_START >> 2) ... += 0x%08lx \n", __FUNCTION__, (unsigned long int) pMemTemp, regOffset, (( regOffset - BCHP_REGISTER_START )>>2 ) );*/

    temp1     = (unsigned long int) pMemTemp;
    temp2     = *pMemTemp & regMask;
    temp3     = *pMemTemp;
    if (outputHtml)
    {
        FPRINTF( outputHtml, "<!-- %s: offset 0x%lx; addr 0x%lx; actual 0x%lX; mask 0x%lx; result 0x%lx ... %s -->\n", __FUNCTION__, regOffset, temp1, temp3, regMask, temp2, ( temp2 ) ? "ON" : "OFF" );
    }

    FPRINTF( stderr, "; offset 0x%lx; addr 0x%lx; actual 0x%lX; mask 0x%lx; result 0x%lx ... %s\n", regOffset, temp1, temp3, regMask, temp2, ( temp2 ) ? "ON" : "OFF" );

    return( temp2 );
} /* read_reg_value */

/**
 *  Function: This function will scan through an array of known registers looking for the one specified by the
 *  user. If a match is found in the list of known registers, the actual value will be read and returned to the
 *  user.
 **/
static unsigned int read_reg_field(
    const char             *regName,
    const char             *fieldName,
    read_reg_field_outputs *pOutputs
    )
{
    unsigned int idx;

    FPRINTF( stderr, "%s: (%s:%s)\n", __FUNCTION__, regName, fieldName );
    for (idx = 0; idx < sizeof( registerInfo )/sizeof( registerInfo[0] ); idx++)
    {
        if (( strcmp( regName, registerInfo[idx].name ) == 0 ) && ( strcmp( fieldName, registerInfo[idx].field ) == 0 ))
        {
            pOutputs->regValue = read_reg_value( g_pMem, registerInfo[idx].regOffset, registerInfo[idx].regMask ) >> registerInfo[idx].shiftCount /**/;
            pOutputs->polarity = registerInfo[idx].polarity;
            break;
        }
    }
    return( 0 );
}

/**
 *  Function: This function will compute the frequency in megahertz based on whether or not the previous
 *  1st-level node had three specific registers defined: MULT, PREDIV, and POSTDIV. if all three of these
 *  registers were found, then the frequency equation is (54 * MULT / PREDIV ) / POSTDIV.
 **/
static long int ComputeFreq(
    void
    )
{
    long int               freq = 0;
    read_reg_field_outputs regMult;
    read_reg_field_outputs regPreDiv;
    read_reg_field_outputs regPostDiv;

    /* if this block has associated frequency registers, use the current values to compute the freq */
    if (g_freqMult && g_freqPostDiv && g_freqPreDiv)
    {
        read_reg_field( g_freqMult->name, g_freqMult->field, &regMult );
        read_reg_field( g_freqPreDiv->name, g_freqPreDiv->field, &regPreDiv );
        read_reg_field( g_freqPostDiv->name, g_freqPostDiv->field, &regPostDiv );
        freq = ( 54*regMult.regValue/regPreDiv.regValue )/regPostDiv.regValue;
        FPRINTF( stderr, "%s: mult (%ld); prediv (%ld); postdiv (%ld); freq (%ld)\n", __FUNCTION__, regMult.regValue, regPreDiv.regValue, regPostDiv.regValue, freq );
    }

    return( freq );
} /* ComputeFreq */

/**
 *  Function: This function will draw a rectangle for the specified node ... along with the appropriate text
 *  describing the node.
 **/
static int DrawRectangle(
    CLK_TREE_NODE *node,
    int            X,
    int            Y,
    char          *ShrinkCmdPos
    )
{
    int                    leftX   = X + RECTANGLE_WIDTH - GAP_BETWEEN_COLUMNS;
    int                    bottomY = Y + RECTANGLE_HEIGHT - GAP_BETWEEN_ROWS;
    char                   longstr[REGISTER_NAME_LEN_MAX+128]; /* need space to append [1234,5678] */
    int                    nameLength  = strlen( node->name ) + strlen( node->field ) + 1;
    char                  *bgColors[3] = {"hotregister", "greenbox", "nonpower"};
    unsigned int           bgColorIdx  = 0;
    char                   freqString[64];
    read_reg_field_outputs outputs;

    memset( &outputs, 0, sizeof( outputs ));
    memset( &freqString, 0, sizeof( freqString ));

    leftX = X + nameLength*11;
    if (nameLength*7 < 120)
    {
        leftX = X + nameLength*18;
    }

    /* if this node is the upper-most logic block node, it is a special case where its color is dictated by the summation of the register statuses below it */
    if (strlen( node->field ) == 0)
    {
        if (blockHasAtLeastOnePowerOn == 0)
        {
            bgColorIdx = BG_GREEN;
        }
        else
        {
            bgColorIdx = BG_HOTREGISTER;
        }
        FPRINTF( stderr, "%s: special (%s); blockHasAtLeastOnePowerOn %lu\n", __FUNCTION__, node->name, blockHasAtLeastOnePowerOn );
    }
    else /* this node is a register with a specific field name associated with it */
    {
        /* if the parent of this node does not have a register field, then this node is the first node under the 1st-level logic block node */
        if (X == GAP_BETWEEN_COLUMNS)
        {
            long int freq = ComputeFreq();

            if (freq > 0)
            {
                snprintf( freqString, sizeof( freqString ), " (%ld MHz)", freq );
            }
        }

        read_reg_field( node->name, node->field, &outputs );

        if (( outputs.regValue && ( outputs.polarity==HighIsOn )) || (( outputs.regValue==0 ) && ( outputs.polarity==HighIsOff )))
        {
            bgColorIdx                = BG_HOTREGISTER;
            blockHasAtLeastOnePowerOn = 1;
            FPRINTF( stderr, "block (%lu); node (%s)\n", blockHasAtLeastOnePowerOn, GetNodeName( node ));
        }
        else
        {
            if (( outputs.polarity==HighIsOn ) || ( outputs.polarity==HighIsOff ))
            {
                bgColorIdx = BG_GREEN;
            }
            else
            {
                bgColorIdx = BG_NONPOWER;
            }
            FPRINTF( stderr, "block (%lu); node (%s)\n", blockHasAtLeastOnePowerOn, GetNodeName( node ));
        }
    }

    FPRINTF( stderr, "%s: for (%s:%s); regValue 0x%08lx; blockHasAtLeastOnePowerOn %lu; ShrinkCmdPos %p\n", __FUNCTION__, node->name, node->field, outputs.regValue, blockHasAtLeastOnePowerOn, ShrinkCmdPos );
    /* if the block is NOT shrunk down to just the one node  AND node is NOT a non-power register (skip the clock freq registers ) */
    if ((( ShrinkCmdPos == NULL ) || ( strlen( node->field )==0 )) /*&& (bgColorIdx != BG_NONPOWER)*/)
    {
        fprintf( outputHtml, "<polygon onclick=\"MyClick(event);\" points=\"%u,%u %u,%u %u,%u %u,%u \" "
                "class=%s id=%s%s%s opacity=\"1.0\" />\n", X, Y, leftX, Y, leftX, bottomY, X, bottomY,
                bgColors[bgColorIdx], node->name, ( strlen( node->field )) ? "-" : "", node->field );

#if 0
        {
            CLK_TREE_NODE *parent = node->parent;
            char           registerStr[64];

            if (parent)
            {
                sprintf( longstr, "%s [%4u,%4u]; cnt %lu .. [%4u,%4u]; len %u (%u) ", node->name, X, Y, RectCount, parent->X, parent->Y, nameLength, nameLength*8 );
            }
            else
            {
                sprintf( longstr, "%s [%4u,%4u]; cnt %lu; len %u (%u) ", node->name, X, Y, RectCount, nameLength, nameLength*8 );
            }
            memset( registerStr, 0, sizeof( registerStr ));
            if (strlen( node->field ))
            {
                GetRegisterData( node->name, node->field, registerStr, sizeof( registerStr ));
                sprintf( longstr, "%s -> %s%s; %s<-0x%08lx (%s); pwron %lu", node->name, node->field, freqString, registerStr, outputs.regValue, ( outputs.polarity==HighIsOff ) ? "HighIsOff" :
                    ( outputs.polarity==HighIsOn ) ? "HighIsOn" : ( outputs.polarity==MULT ) ? "MULT" : ( outputs.polarity==PREDIV ) ? "PREDIV" : ( outputs.polarity==POSTDIV ) ? "POSTDIV" : "unknown",
                    blockHasAtLeastOnePowerOn );
            }
            else
            {
                sprintf( longstr, "%s", node->name );
            }
        }
#else /* if 0 */
        if (strlen( node->field ))
        {
            sprintf( longstr, "%s->%s%s", node->name, node->field, freqString );
        }
        else
        {
            sprintf( longstr, "%s", node->name );
        }
#endif /* if 0 */
        DrawText( X, Y, RECTANGLE_HEIGHT-5, longstr, node->name, node->field );
        node->X = X;
        node->Y = Y;

        RectCount++;
    }
    else
    {
        bottomY = Y;
    }
    return( bottomY );
} /* DrawRectangle */

/**
 *  Function: This function will determine if the specified node is one of the registers that is used to compute the megahertz
 *  for the CPU or core blocks.
 **/
bool IsMhzRegister(
    CLK_TREE_NODE *node
    )
{
    bool rc = false;

    if (node && strlen( node->name ) && strlen( node->field ) && (( GetRegisterPolarity( node->name, node->field ) == MULT ) ||
                                                                  ( GetRegisterPolarity( node->name, node->field ) == PREDIV ) ||
                                                                  ( GetRegisterPolarity( node->name, node->field ) == POSTDIV )))
    {
        rc = true;
    }

    return( rc );
}

/**
 *  Function: This function will draw a straight line between the specified node and its parent node. If the
 *  parent node is being hidden, the logic will backup the node tree until it finds the first non-hidden node.
 **/
static int DrawConnectingLine(
    CLK_TREE_NODE *node,
    CLK_TREE_NODE *parent
    )
{
    long int       X1         = 0;
    long int       X2         = 0;
    long int       Y1         = 0;
    long int       Y2         = 0;
    long int       Ydelta     = 0;
    CLK_TREE_NODE *tempParent = NULL;

    FPRINTF( stderr, "<!-- %s: connect line for (%s) and (%s) -->\n", __FUNCTION__, GetNodeName( node ), GetNodeName( parent ));

    if (parent==NULL)
    {
        FPRINTF( stderr, "%s: parent is null; returning\n", __FUNCTION__ );
        return( 0 );
    }

    /* if the parent node is one of the nodes we are supposed to hide (one of the non-power nodes), walk back up the tree to the first non-hidden node */
    tempParent = parent;
    while (tempParent && IsMhzRegister( tempParent ))
    {
        tempParent = tempParent->parent;
    }

    if (tempParent)
    {
        X1 = tempParent->X;
        Y1 = tempParent->Y;

        X2 = node->X;
        Y2 = node->Y;

        /* if the coordinates are non zero */
        if (X1 && X2 && Y1 && Y2)
        {
            /* horizontal line */
            fprintf( outputHtml, "<line x1=%lu y1=%lu x2=%lu y2=%lu stroke=black stroke-width=3 \" />", X2, Y2+( RECTANGLE_HEIGHT-GAP_BETWEEN_ROWS )/2, X2-21,
                Y2+ ( RECTANGLE_HEIGHT-GAP_BETWEEN_ROWS )/2 );
            fprintf( outputHtml, "<!-- horiz from [%s -> %s] to [%s -> %s] -->", GetNodeName( node ), node->field, GetNodeName( tempParent ), tempParent->field );
            fprintf( outputHtml, "\n" );

            /* if we need to draw a longer vertical line back to node on same level ... i.e. x-coord is same */
            Ydelta = Y2 - Y1;
            if (Ydelta > 40)
            {
                fprintf( outputHtml, "<line x1=%lu y1=%lu x2=%lu y2=%lu stroke=black stroke-width=3 \" />", X2-20, Y2+( RECTANGLE_HEIGHT-GAP_BETWEEN_ROWS )/2, X2-20,
                    Y2+( RECTANGLE_HEIGHT-GAP_BETWEEN_ROWS )/2 - Ydelta + 40 );
                fprintf( outputHtml, "<!-- vertlong from [%s -> %s] to [%s -> %s] -->", GetNodeName( node ), node->field, GetNodeName( tempParent ), tempParent->field );
            }
            else
            {
                fprintf( outputHtml, "<line x1=%lu y1=%lu x2=%lu y2=%lu stroke=black stroke-width=3 \" />", X2-20, Y2+( RECTANGLE_HEIGHT-GAP_BETWEEN_ROWS )/2, X2-20, Y2-10 );
                fprintf( outputHtml, "<!-- vertnorm from [%s -> %s] to [%s -> %s] -->", GetNodeName( node ), node->field, GetNodeName( tempParent ), tempParent->field );
            }
            fprintf( outputHtml, "\n" );
        }
    }

    return( 0 );
} /* DrawConnectingLine */

/**
 *  Function: This function will free all of the memory allocated for the nodes in the power tree.
 **/
static int FreeNode(
    CLK_TREE_NODE *node
    )
{
    if (node == NULL)
    {
        FPRINTF( stderr, "%s: node (%p) ignored\n", __FUNCTION__, (void *) node );
        return( 0 );
    }

    if (node->subLevel)
    {
        CLK_TREE_NODE *temp = node->subLevel;
        FPRINTF( stderr, "%s: node (%s:%p) has subLevel that is not null (%s)\n", __FUNCTION__, GetNodeName( node ), (void *) node, GetNodeName( temp ));
        return( 0 );
    }

    if (node->sameLevel)
    {
        CLK_TREE_NODE *temp = node->sameLevel;
        FPRINTF( stderr, "%s: node (%s:%p) has sameLevel that is not null (%s)\n", __FUNCTION__, GetNodeName( node ), (void *) node, GetNodeName( temp ));
        return( 0 );
    }

    freeCount++;
    FPRINTF( stderr, "(%p) free'ed ... %s; count %lu\n", (void *) node, __FUNCTION__, freeCount );
    {
        char *temp = (char*) node;
        Bsysperf_Free( temp );
        node = NULL;
    }
    return( 0 );
} /* FreeNode */

#if 0
/**
 *  Function: This function will determine if the specified node is a power register. Some of the node values are the upper-level block
 *  names (like RFM, AIO, etc.).
 **/
static bool IsPowerRegister(
    CLK_TREE_NODE *node
    )
{
    bool rc = false;

    if (node && strlen( node->name ) && strlen( node->field ) && (( GetRegisterPolarity( node->name, node->field ) == HighIsOff ) || ( GetRegisterPolarity( node->name, node->field ) == HighIsOn )))
    {
        rc = true;
    }

    return( rc );
}

#endif /* if 0 */

/**
 *  Function: This function will return a string description of the specified action.
 **/
static char *GetActionString(
    CLK_TREE_ACTION action
    )
{
    if (action == DRAW_RECTANGLES)
    {
        return( "RECTANGLES" );
    }
    else if (action == DRAW_LINES)
    {
        return( "LINES" );
    }
    else if (action == COMPUTE_FREQUENCY)
    {
        return( "FREQUENCY" );
    }
    else if (action == FREE_NODES)
    {
        return( "FREE" );
    }

    return( "UNKNOWN" );
} /* GetActionString */

/**
 *  Function: This function will return a string description of the specified node's polarity.
 **/
static char *GetPolarityString(
    CLK_TREE_NODE *node
    )
{
    if (node)
    {
        CLK_TREE_POLARITY polarity =  GetRegisterPolarity( GetNodeName( node ), node->field );

        switch (polarity) {
            case HighIsOff:
            {
                return( "HighIsOff" );

                break;
            }
            case HighIsOn:
            {
                return( "HighIsOn" );

                break;
            }
            case MULT:
            {
                return( "MULT" );

                break;
            }
            case PREDIV:
            {
                return( "PREDIV" );

                break;
            }
            case POSTDIV:
            {
                return( "POSTDIV" );

                break;
            }
        } /* switch */
    }

    return( "UNKNOWN" );
} /* GetPolarityString */

/**
 *  Function: This function is recursive. It will create the HTML code to draw rectangles, text description, and
 *  connecting lines representing the clock tree.
 **/
static int WalkTreeStructure(
    CLK_TREE_NODE  *node,
    CLK_TREE_NODE  *parent,
    int             X,
    int             Y,
    CLK_TREE_ACTION action,
    const char     *queryString
    )
{
    int            localY = Y;
    CLK_TREE_NODE *next   = NULL;
    char           ShrinkCmd[128];
    char          *ShrinkCmdPos = NULL;

    FPRINTF( stderr, "<!-- %s: top - node %s; parent (%s); X (%d); Y (%d); action (%s) polarity (%s)-->\n", __FUNCTION__, GetNodeName( node ),
        GetNodeName( parent ), X, Y, GetActionString( action ), GetPolarityString( node ));
    memset( ShrinkCmd, 0, sizeof( ShrinkCmd ));

    if (node && ( strstr( node->name, "CLKGEN_" ) == NULL ))
    {
        workingMainNode = node;
        FPRINTF( stderr, "%s: new workingMainNode (%s)\n", __FUNCTION__, GetNodeName( workingMainNode ));
    }
    sprintf( ShrinkCmd, "&shr=%s&", GetNodeName( workingMainNode ));
    ShrinkCmdPos = strstr( queryString, ShrinkCmd );
    FPRINTF( stderr, "%s: for queryString (%s); ShrinkCmdPos %p\n", __FUNCTION__, queryString, ShrinkCmdPos );

    #if 0
    /* add a bit of extra space between the major 1st-level blocks */
    if (( X == 10 ) && WalkTreeStructure1stPass && ( ShrinkCmdPos == NULL ))
    {
        Y      += 40;
        localY += 40;
    }
    #endif /* if 0 */

    WalkTreeStructure1stPass = 1;

    FPRINTF( stderr, "%s: for node %p: X %4d; Y %4d; (%s); ShrinkCmd(%s); pos %p\n", __FUNCTION__, (void *) node, X, Y,
        ( node==NULL ) ? "null" : node->name, ShrinkCmd, (void *) ShrinkCmdPos );
    if (node->subLevel)
    {
        int          tempY  = 0;
        unsigned int offset = 40;

        next = node->subLevel;
        FPRINTF( stderr, "%s: for node %p: calling subLevel: node %p (%s)\n", __FUNCTION__, node, next, next->name );
        if (IsMhzRegister( node ))
        {
            offset = 0;
        }
        FPRINTF( stderr, "<!-- %s:       node %s; offset %u -->\n", __FUNCTION__, GetNodeName( node ), offset );
        tempY = WalkTreeStructure( next, node, X + offset, localY + offset, action, queryString );
        if (ShrinkCmdPos == NULL)
        {
            localY = tempY;
        }
    }
    else
    {
        FPRINTF( stderr, "%s: for node (%s), skipping DrawTreeRectangeles()\n", __FUNCTION__, GetNodeName( node ));
    }
    if (action == DRAW_LINES)
    {
        if (ShrinkCmdPos == NULL)
        {
            DrawConnectingLine( node, parent );
        }
    }
    else if (action == DRAW_RECTANGLES)
    {
        CLK_TREE_POLARITY polarity = 0;

        polarity = GetRegisterPolarity( node->name, node->field );

        if (( polarity == HighIsOff ) || ( polarity == HighIsOn ))
        {
            DrawRectangle( node, X, Y, ShrinkCmdPos );
        }
        else
        {
        }

        /* if this node is main level node */
        if (node && ( strlen( node->field ) == 0 ))
        {
            blockHasAtLeastOnePowerOn = 0;
            FPRINTF( stderr, "block (%lu); completed block (%s); mult (%s); prediv (%s); postdiv (%s)\n", blockHasAtLeastOnePowerOn,
                GetNodeName( node ), GetNodeName( g_freqMult ), GetNodeName( g_freqPreDiv ), GetNodeName( g_freqPostDiv ));
        }
        /* if this node is the first subnode under the 1st-level main node */
        else if (X == GAP_BETWEEN_COLUMNS)
        {
            /* clear out global variables for next pass */
            g_freqMult    = NULL;
            g_freqPostDiv = NULL;
            g_freqPreDiv  = NULL;
        }
        else
        {
            if (polarity == MULT)
            {
                g_freqMult = node;
            }
            else if (polarity == PREDIV)
            {
                g_freqPreDiv = node;
            }
            else if (polarity == POSTDIV)
            {
                g_freqPostDiv = node;
            }
            FPRINTF( stderr, "processing field (%s -> %s; pol %u); mult (%s); prediv (%s); postdiv (%s)\n", GetNodeName( node ), node->field,
                polarity, GetNodeName( g_freqMult ), GetNodeName( g_freqPreDiv ), GetNodeName( g_freqPostDiv ));
        }
    }
    else if (action == FREE_NODES)
    {
        FreeNode( node );
    }
    else if (action == COMPUTE_FREQUENCY)
    {
    }

    next = node->sameLevel;
    PRINTF( "%s: for node %p: next %p\n", __FUNCTION__, node, next );
    if (next != NULL)
    {
        FPRINTF( stderr, "%s: for node %p: calling sameLevel: node %p (%s)\n", __FUNCTION__, node, next, next->name );
        localY = WalkTreeStructure( next, parent, X, localY + RECTANGLE_HEIGHT, action, queryString );
    }

    FPRINTF( stderr, "<!-- %s:       node %s: returning Y %d -->\n", __FUNCTION__, node->name, localY );

    return( localY );
} /* WalkTreeStructure */

/**
 *  Function: This function will parse the bpower_tree.txt file and create a linked list of nodes for the clock
 *  tree.
 **/
char *get_clock_tree(
    const char *queryString
    )
{
    int   localY     = 0;
    char *htmlBuffer = NULL;
    int   g_memFd    = 0;
    char  htmlFilename[CLOCK_FILE_FULL_PATH_LEN];


    /* Open driver for memory mapping */
    g_memFd = bmemperfOpenDriver();

    fcntl( g_memFd, F_SETFD, FD_CLOEXEC );

    /* PRINTF( "%s: bmemperfMmap(NULL, mapped_size 0x%x, PROT_READ %u, MAP_SHARED %u, fd %u, addr %x)\n", __FUNCTION__,
        ( BCHP_REGISTER_SIZE<<2 ), PROT_READ|PROT_WRITE, MAP_SHARED, g_memFd, BCHP_PHYSICAL_OFFSET  ); */
    g_pMem = bmemperfMmap( g_memFd );

    /*printf("%s: g_pMem %p\n", __FUNCTION__, (void*) g_pMem );*/
    if (!g_pMem)
    {
        printf( "Failed to bmemperfMmap() fd=%d, addr 0x%08x\n", g_memFd, BCHP_PHYSICAL_OFFSET );
        return( NULL );
    }
    FPRINTF( stderr, "g_pMem 0x%08lX\n", (unsigned long int) g_pMem );

    GetNodeName( NULL ); /* added to resolve warning about GetNodeName() not being used */

    PrependTempDirectory ( htmlFilename, sizeof( htmlFilename ), CLOCK_HTML_FILE );
    outputHtml = fopen( htmlFilename, "w" );

    if (outputHtml == NULL)
    {
        fprintf( stderr, "Could not open new file (%s)\n", htmlFilename );
        return( NULL );
    }

    memset( &ListHead, 0, sizeof( ListHead ));
    strcpy( ListHead.name, "HEAD" );

    fprintf( outputHtml, "<style>\n"
                         ".nonpower\n"
                         "{\n"
                         "    fill:" COLOR_NON_POWER ";\n"
                         "    stroke=" COLOR_NON_POWER ";\n"
                         "    stroke-width:1;\n"
                         "}\n"
                         ".hotregister\n"
                         "{\n"
                         "    fill:" COLOR_POWER_ON ";\n"
                         "    stroke=" COLOR_POWER_ON ";\n"
                         "    stroke-width:1;\n"
                         "}\n"
                         ".greenbox\n"
                         "{\n"
                         "    fill:" COLOR_POWER_OFF ";\n"
                         "    stroke=" COLOR_POWER_OFF ";\n"
                         "    stroke-width:1;\n"
                         "}\n</style>\n" );

    fprintf( outputHtml, "<svg id=svg001 height=\"1500\" width=\"850\" style=\"border:1px solid black;\" xmlns='http://www.w3.org/2000/svg' version='1.1' >\n" );
    fprintf( outputHtml, "<defs>\n" );
    fprintf( outputHtml, "<marker id=\"markerCircle\" markerWidth=\"8\" markerHeight=\"8\" refx=\"5\" refy=\"5\">\n" );
    fprintf( outputHtml, "    <circle cx=\"5\" cy=\"5\" r=\"2\" style=\"stroke: none; fill:#000000;\"/>\n" );
    fprintf( outputHtml, "</marker>\n" );
    fprintf( outputHtml, "<marker id=\"markerArrow\" markerWidth=\"13\" markerHeight=\"13\" refx=\"2\" refy=\"6\" orient=\"auto\">\n" );
    fprintf( outputHtml, "    <path d=\"M2,2 L2,11 L10,6 L2,2\" style=\"fill: #000000;\" />\n" );
    fprintf( outputHtml, "</marker>\n" );
    fprintf( outputHtml, "</defs>\n" );

    ReadClockTreeFile( &ListHead );

    PRINTF( "~ListHead->sub (%s); ListHead->same (%s)\n~", GetNodeName( ListHead.subLevel ), GetNodeName( ListHead.sameLevel ));

    /* if we have a valid head node */
    if (ListHead.sameLevel)
    {
        localY = WalkTreeStructure( ListHead.sameLevel, NULL, 10, 10, DRAW_RECTANGLES, queryString );

        /*
           The X and Y coordinates of each rectangle can't get computed until the final leaf nodes are drawn. Once
           the final leaf nodes are drawn, then we know were to draw the parent node. Once all of the nodes have been
           drawn, go back and connect the parents with the child nodes.
        */
        WalkTreeStructure( ListHead.sameLevel, NULL, 10, 10, DRAW_LINES, queryString );

        fprintf( outputHtml, "</svg>\n" );
        fclose( outputHtml );

        WalkTreeStructure( ListHead.sameLevel, NULL, 10, 10, FREE_NODES, queryString );

        htmlBuffer = ReadFileContents( htmlFilename );
    }
    else
    {
        char treeFilename[TREE_FILE_FULL_PATH_LEN];

        strncpy ( treeFilename, TREE_FILE, sizeof(treeFilename) - 1 );
        printf( "~ALERT~Could not open file %s~CLOCKSFAIL~", treeFilename );
    }

    printf( "~SVGHEIGHT~%u~", localY + 500 ); /* CAD DEBUG was 100 */

    return( htmlBuffer );
} /* get_clock_tree */

char *get_shrink_list(
    void
    )
{
    CLK_TREE_NODE *node      = NULL;
    unsigned int   nodeCount = 0;
    unsigned int   stringLen = 0;
    char          *buffer    = NULL;

    memset( &ListHead, 0, sizeof( ListHead ));
    strcpy( ListHead.name, "HEAD" );

    ReadClockTreeFile( &ListHead );

    node = ListHead.sameLevel;

    /* while there are more nodes to process */
    while (node)
    {
        nodeCount++;
        stringLen += strlen( node->name );
        node       = node->sameLevel;
    }

    if (( nodeCount > 0 ) && ( stringLen > 0 ))
    {
        unsigned int bufferLen = nodeCount*8 /*strlen("&shr=")*/ + stringLen + 20;

        fprintf( stderr, "%s: found %u nodes of length %u; bufferLen %u\n", __FUNCTION__, nodeCount, stringLen, bufferLen );

        buffer = (char *) malloc( bufferLen );
        if (buffer)
        {
            char buffer1[64]; /* space for just one &shr=name_of_upper_block */

            memset( buffer, 0, bufferLen );

            node = ListHead.sameLevel;
            /* while there are more nodes to process */
            while (node)
            {
                if (strlen( node->name ))
                {
                    snprintf( buffer1, sizeof( buffer1 ), "&shr=%s", node->name );
                    strncat( buffer, buffer1, bufferLen-1 );
                    fprintf( stderr, "%s: appended (%s) to big buffer; big buffer len (%u)\n", __FUNCTION__, buffer1, (unsigned int) strlen( buffer ));
                }
                node = node->sameLevel;
            }
            strncat( buffer, "&", bufferLen-1 );
        }
    }

    return( buffer );
} /* get_shrink_list */
