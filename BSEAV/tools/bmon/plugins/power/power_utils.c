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
/* TODO:
   How should we handle registers that are not accessable?
   How can we determine which registers are not accessable?
   CLKGEN_PLL_SYS0_PLL_DIV: This RDB field is not accessable by software; PDIV:reset value is 1; NDIV_INT:Reset value is 60.
*/
/*#include "bmemperf_types64.h"*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

#include "bmon_utils.h"
#include "bmon_uri.h"
#include "bmon_defines.h"
#include "power_utils.h"
#include "bchp_clkgen.h"
#include "bchp_common.h"
#include "bchp_memc_arb_0.h"
#include "bchp_memc_ddr_0.h"
#include "nexus_platform_features.h"
#include "nexus_base_types.h"
#include "bchp_memc_misc_0.h"
#include "bchp_memc_ddr_0.h"

static unsigned long int      RectCount = 0;
static CLK_TREE_NODE          ListHead;
static CLK_TREE_NODE          ListHeadClk;
static unsigned int           LEVEL = 0;
static unsigned char          WalkTreeStructure1stPass            = 0; /* if 0 don't add 40 to beginning of block's Y coord */
static unsigned long int      mallocCount                         = 0;
static unsigned long int      freeCount                           = 0;
static volatile unsigned int *g_pMem                              = NULL;
static unsigned long int      blockHasAtLeastOnePowerOn           = 0; /* used to determine if all of the lower-level clocks are turned off for a particular logic block */
static CLK_TREE_NODE         *g_freqMult                          = NULL;
static CLK_TREE_NODE         *g_freqPreDiv                        = NULL;
static CLK_TREE_NODE         *g_freqPostDiv                       = NULL;
static int                    ComputeHighLevelPolarity_ClockCount = 0;
static int                    ComputeHighLevelPolarity_OnCount    = 0;
static cJSON                 *arrayChildrenPrevious               = NULL;
static bool                   bDebug                              = false;

#include "power_regs.h"
#ifdef FPRINTF
#undef FPRINTF
#endif
#define FPRINTF
#define CADEBUG if (bDebug) fprintf

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

/**
 *  Function: This function will safely output the field of the specified node. If the node is null, it will
 *  print "null" for the field.
 **/
static char *GetNodeField(
    CLK_TREE_NODE *node
    )
{
    if (node)
    {
        return( node->field );
    }

    return( "null" );
}

static CLK_TREE_POLARITY GetRegisterPolarity(
    const char *regName,
    const char *fieldName
    )
{
    unsigned int      idx;
    CLK_TREE_POLARITY polarity = HighIsOff;

    /* if the user provided strings for the register name and field name */
    if (regName && strlen( regName ) && fieldName && strlen( fieldName ))
    {
        for (idx = 0; idx < sizeof( registerInfo )/sizeof( registerInfo[0] ); idx++)
        {
            if (( strcmp( regName, registerInfo[idx].name ) == 0 ) && ( strcmp( fieldName, registerInfo[idx].field ) == 0 ))
            {
                polarity = registerInfo[idx].polarity;

                /*FPRINTF( stderr, "%s: match (%-40s; field (%-32s) ... polarity %d \n", __FUNCTION__, regName, fieldName, polarity  );*/
                break;
            }
        }
    }

    return( polarity );
}                                                          /* GetRegisterPolarity */

/**
 *  Function: This function will read the specified register address offset and mask the value read with the
 *  specified register mask.
 **/
static unsigned int read_reg_value(
    volatile unsigned int *pMemTemp,
    unsigned long int      regOffset,
    unsigned long int      regMask
    )
{
    /*volatile unsigned int *lMemTemp = pMemTemp;*/
    unsigned long int temp1 = 0;
    unsigned long int temp2 = 0;
    unsigned long int temp3 = 0;

    /*printf("%s: lMemTemp 0x%lx; regOffset 0x%08lx; regMask 0x%08lx; BCHP_REGISTER_START %x \n", __FUNCTION__,
        (long unsigned int) lMemTemp, regOffset, regMask, BCHP_REGISTER_START );*/
    pMemTemp += (( regOffset - BCHP_REGISTER_START )>>2 ); /* adding UINT32 to long unsigned int * results in addand being multiplied by 4 before the addition */
    /*printf("%s: pMemTemp 2 0x%08lx; (after regOffset 0x%08lx - BCHP_REGISTER_START >> 2) ... += 0x%08lx \n", __FUNCTION__, (unsigned long int) pMemTemp, regOffset, (( regOffset - BCHP_REGISTER_START )>>2 ) );*/

    temp1 = (unsigned long int) pMemTemp;
    temp2 = *pMemTemp & regMask;
    temp3 = *pMemTemp;

    /*FPRINTF( stderr, "<!-- %s: offset 0x%lx; addr 0x%lx; actual 0x%lX; mask 0x%lx; result 0x%lx ... %s -->\n",
            __FUNCTION__, regOffset, temp1, temp3, regMask, temp2, ( temp2 ) ? "ON" : "OFF" );*/

    /*FPRINTF( stderr, "%s - offset 0x%lx; addr 0x%lx; actual 0x%lX; mask 0x%lx; result 0x%lx ... %s\n",
            __FUNCTION__, regOffset, temp1, temp3, regMask, temp2, ( temp2 ) ? "ON" : "OFF" );*/

    return( temp2 );
}                                                          /* read_reg_value */

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

    for (idx = 0; idx < sizeof( registerInfo )/sizeof( registerInfo[0] ); idx++)
    {
        if (( strcmp( regName, registerInfo[idx].name ) == 0 ) && ( strcmp( fieldName, registerInfo[idx].field ) == 0 ))
        {
            pOutputs->regValue = read_reg_value( g_pMem, registerInfo[idx].regOffset, registerInfo[idx].regMask ) >> registerInfo[idx].shiftCount /**/;
            pOutputs->polarity = registerInfo[idx].polarity;
            /*fprintf( stderr, "%s: registerInfo ... (%s:%s) ... 0x%x ... pol %d \n", __FUNCTION__, regName, fieldName, pOutputs->regValue, pOutputs->polarity );*/
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
        /* if the divisors are not zero */
        if (regPreDiv.regValue && regPostDiv.regValue)
        {
            freq = ( 54*regMult.regValue/regPreDiv.regValue )/regPostDiv.regValue;
        }
        /*FPRINTF( stderr, "%s: mult (%ld); prediv (%ld); postdiv (%ld); freq (%ld)\n", __FUNCTION__, regMult.regValue, regPreDiv.regValue, regPostDiv.regValue, freq );*/
    }

    return( freq );
}                                                          /* ComputeFreq */

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
    unsigned int   polarity     = 0;

    /*FPRINTF( stderr, "%s: parent (%s); %s; new (%s)\n", __FUNCTION__, parent->name, ( level==SUBLEVEL ) ? "SUB" : "SAME", newNodeName );*/

    posDelimiter = strchr( newNodeName, ':' );
    if (posDelimiter)
    {
        *posDelimiter = '\0';
        /*FPRINTF( stderr, "%s: found delimiter at end of regName; new (%s)\n", __FUNCTION__, newNodeName );*/

        posField     = posDelimiter+1;
        posDelimiter = strchr( posField, ':' );
        /*FPRINTF( stderr, "%s: potential regName (%s); posDelimiter %p\n", __FUNCTION__, posField, (void *) posDelimiter );*/
        if (posDelimiter)
        {
            *posDelimiter = '\0';
            /*FPRINTF( stderr, "%s: found delimiter at end of fieldName; new (%s)\n", __FUNCTION__, posField );*/
        }
        else
        {
            /*FPRINTF( stderr, "%s: no delimiter after fieldName; new (%s)\n", __FUNCTION__, posField );*/
        }
    }

    newNode = malloc( sizeof( CLK_TREE_NODE ));
    mallocCount++;
    memset( newNode, 0, sizeof( CLK_TREE_NODE ));
    /*FPRINTF( stderr, "(%p) malloc'ed for (%s) ... %s; count %lu\n", (void *) newNode, newNodeName, __FUNCTION__, mallocCount );*/

    strncpy( newNode->name, newNodeName, sizeof( newNode->name ) - 1 );
    if (posField)
    {
        strncpy( newNode->field, posField, sizeof( newNode->field ) - 1 );
    }
    if (level == SUBLEVEL)
    {
        temp = parent->subLevel;
        next = parent->subLevel;
        /*FPRINTF( stderr, "%s: next subLevel link for (%s;%p) is (%s;%p)\n", __FUNCTION__, GetNodeName( parent ), parent, GetNodeName( next ), next );*/
        while (temp) {
            /*FPRINTF( stderr, "skipping subLevel link (%s)\n", GetNodeName( temp ));*/
            if (temp->subLevel) {next = temp->subLevel; }
            temp = temp->subLevel;
        }

        if (next)
        {
            newNode->parent = next;
            next->subLevel  = newNode;
            /*FPRINTF( stderr, "%s: adding new subLevel node (%s%p) to (%s;%p); LEVEL %u\n", __FUNCTION__, GetNodeName( newNode ), newNode, GetNodeName( next ), next, LEVEL );*/
        }
        else
        {
            newNode->parent  = parent;
            parent->subLevel = newNode;
            /*FPRINTF( stderr, "%s: adding 1st subLevel node (%s;%p) to (%s;%p); LEVEL %u\n", __FUNCTION__, GetNodeName( newNode ), newNode, GetNodeName( parent ), parent, LEVEL );*/
        }
    }
    else if (level == SAMELEVEL)
    {
        temp = parent->sameLevel;
        next = parent->sameLevel;
        /*FPRINTF( stderr, "%s:%u: next (%s;%p) sameLevel link (%s;%p)\n", __FUNCTION__, __LINE__, GetNodeName( parent ), (void *) parent, GetNodeName( next ), (void *) next );*/
        while (temp) {
            /*FPRINTF( stderr, "%s;%u: skipping sameLevel link (%s;%p)\n", __FUNCTION__, __LINE__, GetNodeName( temp ), (void *) temp );*/
            if (temp->sameLevel) {next = temp->sameLevel; }
            temp = temp->sameLevel;
        }

        if (next)
        {
            newNode->parent = next;
            next->sameLevel = newNode;
            /*FPRINTF( stderr, "%s:%u: adding new sameLevel node (%s;%p) to (%s;%p); LEVEL %u\n", __FUNCTION__, __LINE__, GetNodeName( newNode ), (void *) newNode,
                GetNodeName( next ), (void *) next, LEVEL );*/
        }
        else
        {
            newNode->parent   = parent;
            parent->sameLevel = newNode;
            /*FPRINTF( stderr, "%s:%u: adding 1st sameLevel node (%s;%p) to (%s;%p); LEVEL %u\n", __FUNCTION__, __LINE__, GetNodeName( newNode ), (void *) newNode,
                GetNodeName( parent ), (void *) parent, LEVEL );*/
        }
    }
    else
    {
        /*fprintf( stderr, "%s: level (%u) is unknown\n", __FUNCTION__, level );*/
    }

    /*FPRINTF( stderr, "%s: level %d ... adding node from (%s;%p) %s linked to -> (%s;%p)\n", __FUNCTION__, level, GetNodeName( parent ), parent,
        ( level==SUBLEVEL ) ? "SUB" : "SAME", GetNodeName( newNode ), newNode );*/

    if (newNode)
    {
        read_reg_field_outputs outputs;
        memset( &outputs, 0, sizeof( outputs ));
        read_reg_field( newNode->name, newNode->field, &outputs );
        newNode->value    = outputs.regValue;
        newNode->polarity = outputs.polarity;
        newNode->offOrOn  = (( outputs.regValue && ( outputs.polarity==HighIsOn )) || (( outputs.regValue==0 ) && ( outputs.polarity==HighIsOff )));
        if (newNode->polarity == MULT)
        {
            g_freqMult = newNode;
        }
        else if (newNode->polarity == PREDIV)
        {
            g_freqPreDiv = newNode;
        }
        else if (newNode->polarity == POSTDIV)
        {
            g_freqPostDiv = newNode;
        }

        /*if (newNode->value)*/
        {
            long int freq = 0;
            /*FPRINTF( stderr, "%s - --------------> %s->%s = >>> 0x%x <<< .. %s \n", __FUNCTION__, newNode->name,
                newNode->field, newNode->value, newNode->offOrOn==0 ? "OFF" : "ON" );*/
            /* if we found the 3rd megahertz register of the 3 required, compute the mhz */
            if (strcmp( newNode->field, "PDIV" ) == 0)
            {
                freq = ComputeFreq();
                if (freq)
                {
                    CLK_TREE_NODE *parent           = newNode->parent;
                    CLK_TREE_NODE *node_needing_mhz = NULL;
                    /* We need to walk back up the tree to find the node assosicated with this mhz.
                       The matching node either has CLOCK_ENABLE in the name or it is a high-level node that does not have CLKGEN_ in the name.
                     */
                    while (parent) {
                        /*FPRINTF( stderr, "%s -                 parent %p (%s) (%s) .. level %d \n", __FUNCTION__,
                            parent, GetNodeName( parent ), GetNodeField( parent ), parent->level );*/
                        node_needing_mhz = parent;
                        parent           = parent->parent;
                        if (parent && ( node_needing_mhz->level > 1 ))
                        {
                            /*FPRINTF( stderr, "%s -                 this not it  %s  %s .. level %d \n", __FUNCTION__,
                                node_needing_mhz->name, node_needing_mhz->field, node_needing_mhz->level );*/
                            /* keep walking up the tree */
                        }
                        else
                        {
                            /*FPRINTF( stderr, "%s -                 found end .. %s  %s \n", __FUNCTION__, node_needing_mhz->name, node_needing_mhz->field );*/
                            char mhzStr[16];
                            snprintf( mhzStr, sizeof( mhzStr ), " (%ld Mhz)", freq );
                            if (node_needing_mhz->field)
                            {
                                strncat( node_needing_mhz->field, mhzStr, sizeof( node_needing_mhz->field ) - strlen( node_needing_mhz->field ) -1 );
                            }
                            break;
                        }
                    }

                    /* clear out global variables for next pass */
                    g_freqMult    = NULL;
                    g_freqPostDiv = NULL;
                    g_freqPreDiv  = NULL;
                }
            }
            /*FPRINTF( stderr, "%s -                 MULT %p .. PRE %p .. POST %p .. freq %ld \n", __FUNCTION__, g_freqMult, g_freqPreDiv, g_freqPostDiv, freq );*/
        }

        newNode->polarity = GetRegisterPolarity( newNode->name, newNode->field );
    }

    return( newNode );
}                                                          /* AddNode */

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

    if (lstat( filename, &statbuf ) == -1)
    {
        fprintf( stderr, "%s - (%s); lstat failed; %s\n", __FUNCTION__, filename, strerror( errno ));
        return( NULL );
    }

    contents = (char *) malloc( statbuf.st_size + 1 );
    if (contents == NULL)
    {
        return( NULL );
    }
    /*FPRINTF( stderr, "(%p) malloc'ed ... %s; count %lu\n", (void *) contents, __FUNCTION__, mallocCount );*/

    memset( contents, 0, statbuf.st_size + 1 );

    fpText = fopen( filename, "r" );
    if (fpText == NULL)
    {
        fprintf( stderr, "could not fopen(%s)\n", filename );
        return( NULL );
    }
    numBytes = fread( contents, 1, statbuf.st_size, fpText );
    if (numBytes != (unsigned long int) statbuf.st_size)
    {
        fprintf( stderr, "tried to fread %lu bytes but got %lu\n", (unsigned long int) statbuf.st_size, numBytes );
        fclose( fpText );
        return( NULL );
    }

    fclose( fpText );

    return( contents );
}                                                          /* ReadFileContents */

/**
 *  Function: This function will read the clock tree file, parse it, and create a doubly-linked list of the
 *  nodes in the clock tree.
 **/
static int ReadClockTreeFile(
    CLK_TREE_NODE *pListHead,
    const char    *pFilename
    )
{
    char             *contents = NULL;
    char             *posstart = NULL;
    char             *posend   = NULL;
    unsigned long int lineNum  = 0;
    CLK_TREE_NODE    *node     = NULL;
    CLK_TREE_NODE    *temp     = NULL;
    char              treeFilename[TREE_FILE_FULL_PATH_LEN];

    strncpy( treeFilename, pFilename, sizeof( treeFilename ) - 1 );
    contents = ReadFileContents( treeFilename );

    if (contents == NULL)
    {
        fprintf( stderr, "Could not read file (%s)\n", treeFilename );
        return( 0 );
    }

    posstart = contents;                                   /* start parsing the file at the beginning of the file */

    /* step through the file line by line, searching for CLKGEN tags */
    do {
        posend = strstr( posstart, "\n" );
        if (posend)
        {
            *posend = '\0';
            posend++;
        }
        /*FPRINTF( stderr, "next line (%s); posend %p\n", posstart, posend );*/

        if (strlen( posstart ) > 0)
        {
            char        *posCLK    = strstr( posstart, "______C" );
            unsigned int lineLevel = 0;

            if (( 'A' <= *posstart ) && ( *posstart <= 'Z' ))
            {
                /*FPRINTF( stderr, "block (%lu); found new block (%s); line %lu\n", blockHasAtLeastOnePowerOn, posstart, lineNum );*/

                node  = AddNode( pListHead, posstart, SAMELEVEL );
                LEVEL = 0;
                temp  = node->parent;
                if (node) {node->level = lineLevel; }
                /*FPRINTF( stderr, "Step1: LEVEL %2u: NODE now points to (%s); parent (%s)\n", LEVEL, GetNodeName( node ), GetNodeName( temp ));*/
            }
            else if (posCLK)
            {
                posCLK   += 6;                             /* set pointer to the C character ... skip over the underscores */
                lineLevel = ( posCLK - posstart ) / 8;

                /*FPRINTF( stderr, "\n\nCLK (%s) found at level %u; LEVEL %u; line %lu; NODE is (%s)\n", posCLK, lineLevel, LEVEL, lineNum, GetNodeName( node ));*/

                if (lineLevel == LEVEL)
                {
                    node = AddNode( node, posCLK, SAMELEVEL );
                    if (node) {node->level = lineLevel; }
                    /*FPRINTF( stderr, "Step2: LEVEL %2u: NODE still points to (%s); added to parent (%s)\n", LEVEL, GetNodeName( node ), GetNodeName( node ));*/
                }
                else if (lineLevel > LEVEL)
                {
                    temp = node;
                    node = AddNode( node, posCLK, SUBLEVEL );
                    if (node) {node->level = lineLevel; }
                    LEVEL++;
                    /*FPRINTF( stderr, "Step3: LEVEL %2u: NODE now points to (%s); parent (%s)\n", LEVEL, GetNodeName( node ), GetNodeName( temp ));*/
                }
                else
                {
                    unsigned int backupCount = LEVEL - lineLevel;
                    /* need to backup some number of levels */
                    CLK_TREE_NODE *prev = node /*->parent*/;

                    /*FPRINTF( stderr, "%s: need to backup (%u) levels\n", __FUNCTION__, backupCount );*/
                    while (prev && backupCount > 0) {
                        prev = prev->parent;
                        LEVEL--;
                        backupCount--;

                        /*FPRINTF( stderr, "%s: backed up to (%-22s); lineLevel %u; LEVEL %u\n", __FUNCTION__, prev->name, lineLevel, LEVEL );*/
                    }
                    LEVEL = lineLevel;
                    /*FPRINTF( stderr, "%s: add new node (%s) to (%s); LEVEL %u\n", __FUNCTION__, posCLK, GetNodeName( prev ), LEVEL );*/
                    node = AddNode( prev, posCLK, SAMELEVEL );
                    if (node) {node->level = lineLevel; }
                }
            }
            else
            {
                /*FPRINTF( stderr, "ignored line (%s)\n", posstart );*/
            }
        }

        posstart = posend;

        lineNum++;
    } while (posstart != NULL);

    if (contents)
    {
        freeCount++;
        /*FPRINTF( stderr, "(%p) free'ed ... %s; count %lu\n", (void *) contents, __FUNCTION__, freeCount );*/
        FREE_SAFE( contents );
    }

    return( 0 );
}                                                          /* ReadClockTreeFile */

static int computePowerHotOrCold(
    read_reg_field_outputs *outputs
    )
{
    int lRegisterPowerState = 0;

    if (( outputs->regValue && ( outputs->polarity==HighIsOn )) || (( outputs->regValue==0 ) && ( outputs->polarity==HighIsOff )))
    {
        lRegisterPowerState = BG_HOTREGISTER;
    }
    else
    {
        if (( outputs->polarity==HighIsOn ) || ( outputs->polarity==HighIsOff ))
        {
            lRegisterPowerState = BG_GREEN;
        }
        else
        {
            lRegisterPowerState = BG_NONPOWER;
        }
    }

    return( lRegisterPowerState );
}                                                          /* computePowerHotOrCold */

/**
 *  Function: This function will determine if the specified node is one of the registers that is used to compute the megahertz
 *  for the CPU or core blocks.
 **/
bool IsMhzRegister(
    CLK_TREE_NODE *node
    )
{
    bool rc = false;

    if (node && strlen( node->name ) && strlen( node->field ) && (( node->polarity == MULT ) ||
                                                                  ( node->polarity == PREDIV ) ||
                                                                  ( node->polarity == POSTDIV )))
    {
        rc = true;
        /*fprintf( stderr, "%s - %s-%s ... %d \n", __FUNCTION__, node->name, node->field, rc );*/
    }

    return( rc );
}

/**
 *  Function: This function will free all of the memory allocated for the nodes in the power tree.
 **/
static int FreeNode(
    CLK_TREE_NODE *node
    )
{
    if (node == NULL)
    {
        /*FPRINTF( stderr, "%s: node (%p) ignored\n", __FUNCTION__, (void *) node );*/
        return( 0 );
    }

    if (node->subLevel)
    {
        CLK_TREE_NODE *temp = node->subLevel;
        /*FPRINTF( stderr, "%s: node (%s:%p) has subLevel that is not null (%s)\n", __FUNCTION__, GetNodeName( node ), (void *) node, GetNodeName( temp ));*/
        return( 0 );
    }

    if (node->sameLevel)
    {
        CLK_TREE_NODE *temp = node->sameLevel;
        /*FPRINTF( stderr, "%s: node (%s:%p) has sameLevel that is not null (%s)\n", __FUNCTION__, GetNodeName( node ), (void *) node, GetNodeName( temp ));*/
        return( 0 );
    }

    freeCount++;
    /*FPRINTF( stderr, "(%p) free'ed ... %s; count %lu\n", (void *) node, __FUNCTION__, freeCount );*/
    {
        char *temp = (char *) node;
        FREE_SAFE( temp );
        node = NULL;
    }
    return( 0 );
}                                                          /* FreeNode */

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
    else if (action == FREE_NODES)
    {
        return( "FREE" );
    }

    return( "UNKNOWN" );
}                                                          /* GetActionString */

/**
 *  Function: This function will return a string description of the specified node's polarity.
 **/
static char *GetPolarityString(
    CLK_TREE_NODE *node
    )
{
    if (node)
    {
        CLK_TREE_POLARITY polarity =  node->polarity;

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
        }                                                  /* switch */
    }

    return( "UNKNOWN" );
}                                                          /* GetPolarityString */

static int computeOffOrOn(
    const char             *name,
    const char             *field,
    read_reg_field_outputs *poutputs
    )
{
    int rc = 0;

    /* if top-level block, perform special logic to determine on or off */
    if (name && strlen( name ) && field && ( strlen( field ) == 0 ))
    {
        if (ComputeHighLevelPolarity_OnCount > 0)
        {
            rc = 1;
        }
    }
    else if (computePowerHotOrCold( poutputs ) == BG_HOTREGISTER)
    {
        rc = 1;
    }

    return( rc );
}                                                          /* computeOffOrOn */

static cJSON *AddJsonNode(
    CLK_TREE_NODE *node,
    cJSON         *objectClkgen,
    const char    *filter
    )
{
    char *name  = NULL;
    char *field = NULL;

    if (node)
    {
        /*fprintf( stderr, "%s ------> objectClkgen (%p) ... name (%s) ... field (%s) sub (%p) same (%p) .. pol %d .. val 0x%x .. filter (%s) \n",
            __FUNCTION__, objectClkgen, node->name, node->field, node->subLevel, node->sameLevel, node->polarity, node->value, filter );*/
        if (strcmp( node->name, "HEAD" ) == 0) {return( objectClkgen ); }
    }
    /* do not add nodes that are MULT, PRE_DIV, or POST_DIV */
    if (node && objectClkgen && (( node->polarity == HighIsOn ) || ( node->polarity == HighIsOff )))
    {
        read_reg_field_outputs outputs = {0, 0};

        outputs.regValue = node->value;
        outputs.polarity = node->polarity;

        cJSON *childNode = json_AddObject( objectClkgen, NO_FILTER, NULL, "node" );
        /*fprintf( stderr, "%s ------> json_AddObject (to->%p) ... name (%s) ... field (%s) sub (%p) same (%p) ... childNode (%p) \n",
            __FUNCTION__, objectClkgen, node->name, node->field, node->subLevel, node->sameLevel, childNode );*/

        /* if the name starts with CLKGEN_, don't output that part of the name; this cuts down on the size of the JSON data */
        if (strncmp( node->name, "CLKGEN_", 7 ) == 0)
        {
            name = &node->name[7];
        }
        else
        {
            name = &node->name[0];
        }

        /* if the field starts with CLKGEN_, don't output that part of the name; this cuts down on the size of the JSON data */
        if (strncmp( node->field, "CLKGEN_", 7 ) == 0)
        {
            field = &node->field[7];
        }
        else
        {
            field = &node->field[0];
        }
        json_AddString( childNode, filter, childNode, "register", name );
        json_AddString( childNode, filter, childNode, "field", field );
        json_AddNumber( childNode, filter, childNode, "state", computeOffOrOn( name, field, &outputs ));
        json_AddNumber( childNode, filter, childNode, "value", node->value );

        return( childNode );
    }
    else
    {
        if (node)
        {
            /*fprintf( stderr, "%s ------> SKIPPING ... objectClkgen (%p) ... name (%s) ... field (%s) sub (%p) same (%p) .. pol %d .. val 0x%x \n",
                __FUNCTION__, objectClkgen, node->name, node->field, node->subLevel, node->sameLevel, node->polarity, node->value );*/
        }
    }
    return( NULL );
}                                                          /* AddJsonNode */

static int ComputeHighLevelPolarity(
    CLK_TREE_NODE *node
    )
{
    int polarity = 0;

    if (node->field && strlen( node->field ) && ( strstr( node->field, "NDIV_" ) == NULL ) && ( strstr( node->field, "MDIV_" ) == NULL ) && ( strstr( node->field, "PDIV" ) == NULL ))
    {
        ComputeHighLevelPolarity_ClockCount++;
        ComputeHighLevelPolarity_OnCount += node->offOrOn;
    }
    /*FPRINTF( stderr, "%s - node 0x%x .. pol %d .. cnt %d of %d .. off/on %d .. (%s) (%s) .. level %d \n", __FUNCTION__, node,
            node ? node->polarity : 99, ComputeHighLevelPolarity_OnCount, ComputeHighLevelPolarity_ClockCount, node ? node->offOrOn : 88,
            GetNodeName( node ), node->field ? node->field : "none", node->level );*/
    if (node->subLevel /* && node->level > 0 */)
    {
        /*CADEBUG( stderr, "%s - node 0x%x .. calling subLevel %p \n", __FUNCTION__, node, node->subLevel );*/
        ComputeHighLevelPolarity( node->subLevel );
    }
    if (node->sameLevel && strlen( GetNodeField( node->sameLevel )) && ( node->level > 0 ))
    {
        /*CADEBUG( stderr, "%s - node 0x%x (%s) .. calling sameLevel %p (%s) (%s) \n", __FUNCTION__, node, node->name, node->sameLevel,
            GetNodeName( node->sameLevel ), GetNodeField( node->sameLevel ));*/
        ComputeHighLevelPolarity( node->sameLevel );
    }
}                                                          /* ComputeHighLevelPolarity */

static int PrintTree(
    CLK_TREE_NODE *node,
    unsigned int   level
    )
{
    unsigned int idx = 0;

    return( 0 );                                           /* CAD debug */

    if (node == NULL) {return( 0 ); }

    fprintf( stderr, "%s - ", __FUNCTION__ );
    for (idx = 0; idx<level; idx++) {
        fprintf( stderr, "      " );
    }
    fprintf( stderr, "%s ... %s \n", node->name, node->field );
    if (node->subLevel)
    {
        PrintTree( node->subLevel, level+1 );
    }
    if (node->sameLevel)
    {
        PrintTree( node->sameLevel, level+1 );
    }
    return( 0 );
}                                                          /* PrintTree */

static bool IsInNodeList(
    CLK_TREE_NODE *node,
    const char    *expandNodeList
    )
{
    bool  rc              = false;
    char *token           = NULL;
    char *lexpandNodeList = NULL;

    if (node == NULL) {return( rc ); }

    lexpandNodeList = malloc( strlen( expandNodeList ) + 1 );
    if (lexpandNodeList)
    {
        strncpy( lexpandNodeList, expandNodeList, strlen( expandNodeList ) + 1 );
        token = strtok( lexpandNodeList, "+" );
        while (token)
        {
            /*fprintf(stderr, "%s - trying (%s) len %d ... token (%s) len %d ... cmp %p \n", __FUNCTION__,
                    node->name, strlen(node->name), token, strlen(token), strcmp( token, node->name )  );*/
            if (( strcmp( token, node->name ) == 0 ) && ( strlen( token ) == strlen( node->name )))
            {
                /*fprintf(stderr, "%s - MATCHED (%s) in expand (%s) ... token (%s) \n", __FUNCTION__, node->name, lexpandNodeList, token );*/
                rc = true;
                break;
            }
            token = strtok( NULL, "+" );
        }
        free( lexpandNodeList );
    }
    return( rc );
}                                                          /* IsInNodeList */

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
    const char     *queryString,
    cJSON          *objectClkgen,
    bool            bShowBlock
    )
{
    int            rc        = 0;
    int            localY    = Y;
    CLK_TREE_NODE *next      = NULL;
    cJSON         *childNode = NULL;
    char           expandNodeList[256];

    if (parent == NULL)                                    /* if this is a top-level node */
    {
        memset( expandNodeList, 0, sizeof( expandNodeList ));

        /* did the user specify a list of blocks to expand ... like expand=BVN+CPU */
        rc = bmon_uri_find_tagvalue( queryString, "expand", expandNodeList, sizeof( expandNodeList ));
        /*if(strlen(expandNodeList) ) fprintf( stderr, "%s - top node ... bmon_uri_find_tagvalue (%s) ... rc %d \n", __FUNCTION__, expandNodeList, rc );*/
        if (rc)                                            /* if the api found the string */
        {
            /* if this node name is found in the expand list */
            /*fprintf(stderr, "%s:L%u - node (%s) ... expand (%s) ... found %p \n", __FUNCTION__, __LINE__, GetNodeName( node ), expandNodeList, strstr( expandNodeList, GetNodeName( node )));*/
            if (IsInNodeList( node, expandNodeList ))
            {
                bShowBlock = true;
                /*CADEBUG(stderr, "%s - bShowBlock %d ... (%s) \n", __FUNCTION__, bShowBlock, expandNodeList );*/
            }
            else
            {
                bShowBlock = false;
            }
        }
        /*CADEBUG( stderr, "%s: top - node %s; parent (%s); X (%d); Y (%d); action (%s) polarity (%s) .. show %d \n", __FUNCTION__, GetNodeName( node ),
            GetNodeName( parent ), X, Y, GetActionString( action ), GetPolarityString( node ), bShowBlock);*/
        /*CADEBUG( stderr, "queryString (%s) \n", queryString );*/
    }

    WalkTreeStructure1stPass = 1;

    /*CADEBUG( stderr, "\n%s:L%u node (%s) field (%s) sub (%p) same (%p) .. Show %d .. objectClkgen %p .. parent %p \n",
            __FUNCTION__, __LINE__, GetNodeName( node ), ( node->field ) ? node->field : "none", node->subLevel,
            node->sameLevel, bShowBlock, objectClkgen, parent );*/
    if (objectClkgen && ( bShowBlock || ( parent == NULL )) && ( IsMhzRegister( node ) == false ))
    {
        /* if we are not showing the lower level subclocks, we need to compute the on/off status of the entire block before adding the json */
        if (( parent == NULL ) || ( node->level == 0 ))
        {
            char results[16];
            ComputeHighLevelPolarity_ClockCount = 0;
            ComputeHighLevelPolarity_OnCount    = 0;
            g_freqMult    = NULL;
            g_freqPreDiv  = NULL;
            g_freqPostDiv = NULL;

            /*CADEBUG(stderr, "%s - ComputeHighLevelPolarity_ClockCount and OnCount set to 0 \n", __FUNCTION__ );*/
            ComputeHighLevelPolarity( node );

            /* add the total polarity results to the node name */
            sprintf( results, " (%d of %d)", ComputeHighLevelPolarity_OnCount, ComputeHighLevelPolarity_ClockCount );
            strncat( node->name, results, sizeof( node->name ) - strlen( node->name ) -1 );
        }

        childNode = AddJsonNode( node, objectClkgen, queryString );
    }
    else
    {
        if (strcmp( GetNodeField( node ), "NDIV_INT" ) == 0)
        {
            g_freqMult = node;
        }
        else if (strncmp( GetNodeField( node ), "MDIV_CH", 7 ) == 0)
        {
            g_freqPostDiv = node;
        }
        else if (strcmp( GetNodeField( node ), "PDIV" ) == 0)
        {
            g_freqPreDiv = node;
        }
        /*CADEBUG(stderr, "%s:L%u skipping %s %s .. m (%p) pre (%p) post (%p) \n", __FUNCTION__, __LINE__,
          node->name, node->field, g_freqMult, g_freqPreDiv, g_freqPostDiv );*/
    }

    if (node->subLevel)
    {
        int          tempY         = 0;
        unsigned int offset        = 40;
        cJSON       *arrayChildren = NULL;

        /*CADEBUG( stderr, "%s:L%u node (%s) field (%s) json_AddArray(to-> child %p) = (%p) \n", __FUNCTION__, __LINE__,
            GetNodeName( node ), node->field, childNode, arrayChildren );*/
        /*CADEBUG( stderr, "%s:L%u mult (%p) .. pre (%p) .. post (%p) .. level %d \n", __FUNCTION__, __LINE__, g_freqMult,
          g_freqPreDiv, g_freqPostDiv, node->level );*/
        if (childNode)
        {
            arrayChildren = json_AddArray( childNode, "/" /*filter*/, childNode, "subclocks" );
            /* in case we start skipping the MULT, PREDIV, and POSTDIV registers, remember this node's children array */
            if (arrayChildren)
            {
                arrayChildrenPrevious = arrayChildren;
                /*CADEBUG(stderr, "%s:L%u - arrayChildrenPrevious now is (%p) .. node (%s) \n", __FUNCTION__, __LINE__,
                        arrayChildrenPrevious, GetNodeName(node) );*/
            }
        }
        /* if all three MHz registers are set to something valid, we have reached the end of the 3-register set */
        else if (g_freqMult && g_freqPostDiv && g_freqPreDiv)
        {
            if (arrayChildrenPrevious)
            {
                arrayChildren         = arrayChildrenPrevious;
                arrayChildrenPrevious = NULL;
                /*CADEBUG(stderr, "%s:L%u - arrayChildren now is (%p)/(%p) .. node (%s) \n", __FUNCTION__, __LINE__,
                        arrayChildren, arrayChildrenPrevious, GetNodeName(node) );*/
            }
        }
       /*CADEBUG(stderr, "%s:L%u - arrayChildren (%p)/(%p) .. child (%p) .. node (%s) \n", __FUNCTION__, __LINE__, arrayChildren,
               arrayChildrenPrevious, childNode, GetNodeName(node) );*/

        next = node->subLevel;
        if (( action == DRAW_RECTANGLES ) && IsMhzRegister( node ))
        {
            offset = 0;
        }
        /*FPRINTF( stderr, "%s:L%u calling WalkTreeStructure() - subl next %s; field (%s) .. obj %p \n", __FUNCTION__, __LINE__,
            GetNodeName( next ), ( next ) ? next->field : "none", objectClkgen);*/
        tempY = WalkTreeStructure( next, node, X + offset, localY + offset, action, queryString, arrayChildren, bShowBlock );
    }
    else
    {
        /*FPRINTF( stderr, "%s: for node (%s), skipping DrawTreeRectangeles()\n", __FUNCTION__, GetNodeName( node ));*/
    }
    if (action == DRAW_RECTANGLES)
    {
        CLK_TREE_POLARITY polarity = 0;

        polarity = node->polarity;

        /* if this node is main level node */
        if (node && ( strlen( node->field ) == 0 ))
        {
            blockHasAtLeastOnePowerOn = 0;
            /*FPRINTF( stderr, "block (%lu); completed block (%s); mult (%s); prediv (%s); postdiv (%s)\n", blockHasAtLeastOnePowerOn,
                GetNodeName( node ), GetNodeName( g_freqMult ), GetNodeName( g_freqPreDiv ), GetNodeName( g_freqPostDiv ));*/
        }
        /* if this node is the first subnode under the 1st-level main node */
        else if (X == GAP_BETWEEN_COLUMNS)
        {
            /* clear out global variables for next pass */
            g_freqMult    = NULL;
            g_freqPostDiv = NULL;
            g_freqPreDiv  = NULL;
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
    /*FPRINTF( stderr, "%s:L%u checking sameLevel ... for node (%s - %s): sameLevel (%p) \n", __FUNCTION__, __LINE__, GetNodeName( node ), ( node ) ? node->field : "none", next );*/
    if (next != NULL)
    {
        /*FPRINTF( stderr, "%s:L%u calling WalkTreeStructure() same (%s) field (%s) \n",
            __FUNCTION__, __LINE__, GetNodeName( node ), node->field );*/
        localY = WalkTreeStructure( next, parent, X, localY + RECTANGLE_HEIGHT, action, queryString, objectClkgen, bShowBlock );
    }

    /*FPRINTF( stderr, "%s - node %s: field (%s) ... returning \n\n", __FUNCTION__, node->name, node->field );*/

    return( localY );
}                                                          /* WalkTreeStructure */

/**
 *  Function: This function will parse the power_tree.txt file and create a linked list of nodes for the clock
 *  tree.
 **/
char *get_clock_tree(
    const char *queryString,
    cJSON      *objectClkgen1,
    cJSON      *objectClkgen2
    )
{
    int   localY             = 0;
    char *htmlBuffer         = NULL;
    int   g_memFd            = 0;
    char *queryStringDefault = "/";
    char  htmlFilename[CLOCK_FILE_FULL_PATH_LEN];

    if (queryString == NULL) {queryString = queryStringDefault; }

    /* Open driver for memory mapping */
    g_memFd = bmemperfOpenDriver();

    fcntl( g_memFd, F_SETFD, FD_CLOEXEC );

    /*FPRINTF( stderr, "%s: bmemperfMmap(NULL, mapped_size 0x%x, PROT_READ %u, MAP_SHARED %u, fd %u, addr %x)\n", __FUNCTION__,
        ( BCHP_REGISTER_SIZE<<2 ), PROT_READ|PROT_WRITE, MAP_SHARED, g_memFd, BCHP_PHYSICAL_OFFSET  );*/
    g_pMem = bmemperfMmap( g_memFd );

    /*FPRINTF( stderr, "%s: g_pMem %p\n", __FUNCTION__, (void *) g_pMem );*/
    if (!g_pMem)
    {
        fprintf( stderr, "Failed to bmemperfMmap() fd=%d, addr 0x%08x\n", g_memFd, BCHP_PHYSICAL_OFFSET );
        return( NULL );
    }
    /*FPRINTF( stderr, "g_pMem 0x%08lX\n", (unsigned long int) g_pMem );*/

    GetNodeName( NULL );                                   /* added to resolve warning about GetNodeName() not being used */

    bmon_prepend_temp_directory( htmlFilename, sizeof( htmlFilename ), CLOCK_HTML_FILE );

    memset( &ListHead, 0, sizeof( ListHead ));
    memset( &ListHeadClk, 0, sizeof( ListHeadClk ));
    strcpy( ListHead.name, "HEAD" );
    strcpy( ListHeadClk.name, "HEAD" );

    if (objectClkgen1)
    {
        ReadClockTreeFile( &ListHead, TREE_FILE );

        /*fprintf( stderr, "ListHead->sub (%s); ListHead->same (%s)\n", GetNodeName( ListHead.subLevel ), GetNodeName( ListHead.sameLevel ));*/

        /* if we have a valid head node */
        if (ListHead.sameLevel)
        {
            /*FPRINTF( stderr, "L%u calling WalkTreeStructure() - subl node %s; field (%s) \n", __LINE__, GetNodeName( &ListHead ), ( &ListHead ) ? ListHead.field : "none" );*/
            localY = WalkTreeStructure( ListHead.sameLevel, NULL, 10, 10, DRAW_RECTANGLES, queryString, objectClkgen1, false );
        }
        else
        {
            char treeFilename[TREE_FILE_FULL_PATH_LEN];

            strncpy( treeFilename, TREE_FILE, sizeof( treeFilename ) - 1 );
            fprintf( stderr, "~ALERT~Could not open file %s~CLOCKSFAIL~", treeFilename );
        }
    }

    if (objectClkgen2)
    {
        ReadClockTreeFile( &ListHeadClk, CLK_TREE_FILE );

        /*fprintf( stderr, "ListHeadClk->sub (%s); ListHeadClk->same (%s)\n", GetNodeName( ListHeadClk.subLevel ), GetNodeName( ListHeadClk.sameLevel ));*/

        /* if we have a valid head node */
        if (ListHeadClk.sameLevel)
        {
            /*fprintf( stderr, "L%u calling WalkTreeStructure() - subl node %s; field (%s) \n", __LINE__, GetNodeName( &ListHeadClk ), ( &ListHeadClk ) ? ListHeadClk.field : "none" );*/
            localY = WalkTreeStructure( ListHeadClk.sameLevel, NULL, 10, 10, DRAW_RECTANGLES, queryString, objectClkgen2, false );
        }
        else
        {
            char treeFilename[TREE_FILE_FULL_PATH_LEN];

            strncpy( treeFilename, CLK_TREE_FILE, sizeof( treeFilename ) - 1 );
            fprintf( stderr, "~ALERT~Could not open file %s~CLOCKSFAIL~", treeFilename );
        }
    }

    return( htmlBuffer );
}                                                          /* get_clock_tree */

static char *get_shrink_list(
    void
    )
{
    CLK_TREE_NODE *node      = NULL;
    unsigned int   nodeCount = 0;
    unsigned int   stringLen = 0;
    char          *buffer    = NULL;

    memset( &ListHead, 0, sizeof( ListHead ));
    strcpy( ListHead.name, "HEAD" );

    ReadClockTreeFile( &ListHead, TREE_FILE );

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
            char buffer1[64];                              /* space for just one &shr=name_of_upper_block */

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
}                                                          /* get_shrink_list */

/**
 *  Function: This function will open the device used for reading and writing registers.
 *            The device name is configurable by modifying the file "device_node.cfg".
 *            The Android system is doing away with access to /dev/mem because of
 *            security holes. This mechanism will allow Android to specify whatever
 *            device name they need to allow access to the register space.
 **/
static int bmemperfOpenDriver(
    void
    )
{
    int   fd       = 0;
    char *contents = NULL;
    char  device_name[32];
    char  tempFilename[TEMP_FILE_FULL_PATH_LEN];

    memset( device_name, 0, sizeof( device_name ));
    memset( tempFilename, 0, sizeof( tempFilename ));

    strncpy( tempFilename, "device_node.cfg", sizeof( tempFilename ) - 1 );

    /*printf( "%s: device node configuration file is (%s)\n", __FUNCTION__, tempFilename );*/
    /* attempt to open and read the contents of the configuration file (Android use) */
    contents = GetFileContents( tempFilename );

    /* if the file existed and has some contents to it */
    if (contents)
    {
        /* use the contents to override the device driver name */
        strncpy( device_name, contents, sizeof( device_name ) -1 );
        FREE_SAFE( contents );

        /* if the last character is a new-line character */
        if (strlen( device_name ) && ( device_name[strlen( device_name )-1] == '\n' ))
        {
            device_name[strlen( device_name )-1] = 0;
        }
        /*printf( "%s: read device name (%s) from configuration file \n", __FUNCTION__, device_name );*/
    }
    else
    {
        strncpy( device_name, "/dev/mem", sizeof( device_name ) -1 );
    }

    fd = open( device_name, O_RDWR|O_SYNC );               /*O_SYNC for uncached address */

    /* if the open failed */
    if (fd < 0)
    {
        /*printf("%s: open (%s) failed (%s); \n", __FUNCTION__, device_name, strerror(errno) );*/
    }

    /*printf("%s: returning fd %d\n", __FUNCTION__, fd );*/
    return( fd );
}                                                          /* bmemperfOpenDriver */

static void *bmemperfMmap(
    int g_memFd
    )
{
    void *pMem = NULL;

    /*PRINTFLOG( "%s: mmap64(NULL, mapped_size 0x%x, PROT_READ %u, MAP_SHARED %u, g_memFd %u, BCHP_PYHS_OFF %x; BCHP_REG_START %x)\n",
        __FUNCTION__, ( BCHP_REGISTER_SIZE<<2 ), PROT_READ|PROT_WRITE, MAP_SHARED, g_memFd, BCHP_PHYSICAL_OFFSET, BCHP_REGISTER_START );*/

    pMem = mmap64( 0, ( BCHP_REGISTER_SIZE<<2 ), PROT_READ|PROT_WRITE, MAP_SHARED, g_memFd, BCHP_PHYSICAL_OFFSET + BCHP_REGISTER_START );

    return( pMem );
}
