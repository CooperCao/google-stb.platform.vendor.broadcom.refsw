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
var GAP_BETWEEN_COLUMNS = 50;
var GAP_BETWEEN_ROWS = 10;
var RECTANGLE_WIDTH = Number(130 + GAP_BETWEEN_COLUMNS);
var RECTANGLE_HEIGHT = 40;
var DRAW_RECTANGLES = 1;
var DRAW_LINES = 2;
var FREE_NODES = 3;
var HighIsOff = 0;
var HighIsOn = 1;
var MULT = 2;
var PREDIV = 3;
var POSTDIV = 4;
var NULL = 0;
var BG_HOTREGISTER = 0;
var BG_GREEN = 1;
var BG_NONPOWER = 2;
var SUBLEVEL = 0;
var SAMELEVEL = 1;
var DEFAULT_X = 10; /* initial x coordinate */
var RectCount = 0;
var blockHasAtLeastOnePowerOn = 0;
/* used to determine if all of the lower-level clocks are turned off for a particular logic block */
var GlobalSvg = "";
var g_freqMult = NULL;
var g_freqPostDiv = NULL;
var g_freqPreDiv = NULL;
var g_clktreeExpand = "";
var g_YMax = 0;

function GetNodeName(node) {
    if (node) {
        return (node.register);
    }

    return ("null");
}

function GetNodeField(node) {
    if (node) {
        return (node.field);
    }

    return ("null");
}

/**
 *  Function: This function will determine if the specified node is one of the registers that is used to compute the megahertz
 *  for the CPU or core blocks.
 **/
function IsMhzRegister(node) {
    var rc = false;

    if ((node && node.register.length && node.field.length) && ((GetRegisterPolarity(node.register, node.field) == MULT) || (GetRegisterPolarity(node.register, node.field) == PREDIV) || (GetRegisterPolarity(node.register, node.field) == POSTDIV))) {
        rc = true;
    }
    return (rc);
}

function GetRegisterPolarity(node) {
    var idx = 0;
    var polarity = HighIsOff;

    if (node && node.polarity) {
        polarity = node.polarity;
    }

    return (polarity);
}
/* GetRegisterPolarity */

/**
 *  Function: This function will draw a rectangle for the specified node ... along with the appropriate text
 *  describing the node.
 **/
function DrawRectangle(node, X, Y, ShrinkCmdPos)
{
    var leftX = Number(X + RECTANGLE_WIDTH - GAP_BETWEEN_COLUMNS);
    var bottomY = Number(Y + RECTANGLE_HEIGHT - GAP_BETWEEN_ROWS);
    var longstr = ""
    var nameLength = Math.max( Number(node.register.length + node.field.length + 1), 20); // for really short names, don't go below this minimum
    var bgColorClass = "";
    var freqString = "";
    var textColor = " ";

    /* if this is the upper leve block ... like AIO, AVS, BVN, etc. ... or the top-level of the reverse tree */
    if ( X == DEFAULT_X || node.register.indexOf( " of " ) /* node.field.length == 0 */ ) {
        leftX = Number(X + nameLength * 15);
    } else {
        leftX = Number(X + nameLength * 11);
    }

    /* determine the color of the background ... lime, yellow, orange, red */
    bgColorClass = GetClkgenColorClass( node );
    if ( bgColorClass == "register_hot" || bgColorClass == "register_nearly_on" ) {
        textColor = " fill:white;";
    }

    //FPRINTF( stderr, "%s: for (%s:%s); regValue 0x%08lx; blockHasAtLeastOnePowerOn %lu; ShrinkCmdPos %p\n",
    //   __FUNCTION__, node.register, node.field, outputs.regValue, blockHasAtLeastOnePowerOn, ShrinkCmdPos );
    /* if the block is NOT shrunk down to just the one node  AND node is NOT a non-power register (skip the clock freq registers ) */
    if (((ShrinkCmdPos == NULL) || (node.field.length == 0))
    ) {
        var separator = "";
        if (node.field.length)
            separator = "-";
        GlobalSvg += "<polygon points=\"" + X + "," + Y + " " + leftX + "," + Y + " ";
        GlobalSvg += leftX + "," + bottomY + " " + X + "," + bottomY + "\" class=\"" + bgColorClass + "\" id=\"" + node.register + separator + node.field;
        GlobalSvg += "\" /></polygon>";

        if (node.field.length) {
            longstr = node.register + "->" + node.field + freqString;
        } else {
            longstr = node.register;
        }

        DrawText(X, Y, RECTANGLE_HEIGHT - 5, longstr, node.register, node.field, textColor );
        node.X = X;
        node.Y = Y;

        g_YMax = Math.max( g_YMax, Number( bottomY + GAP_BETWEEN_ROWS) );

        RectCount++;
    } else {
        bottomY = Y;
    }
    return (bottomY);
}
/* DrawRectangle */

/**
 *  Function: This function will create the HTML to display text for each rectangle for each node in the clock
 *  tree.
 **/
function DrawText(X, Y, height, description, regName, regField, textColor ) {
    var textX = Number(X + 10);
    var textY = Number(Y + height / 2 + 3) /* position text near the center of the rectangle */;
    var fontSize = 13;
    var separator = "";
    var onclick = "";
    if (regField.length)
        separator = "-";

    /* If register is the upper-most logic block (this is the left-most block); make its font bigger */
    if ( X == DEFAULT_X ) {
        fontSize = 18;
        onclick = " onclick=\"ClkgenClick(event)\" ";
    }
    GlobalSvg += "<text class=\"textclkgen\" x=\"" + textX + "\" y=\"" + textY + "\" " + onclick + " ";
    GlobalSvg += "style=\"font-size:" + fontSize + "pt; " + textColor + "\" id=" + regName + separator + regField + " >" + description + "</text>\n";
}
/* DrawText */

/**
 *  Function: This function will compute the frequency in megahertz based on whether or not the previous
 *  1st-level node had three specific registers defined: MULT, PREDIV, and POSTDIV. if all three of these
 *  registers were found, then the frequency equation is (54 * MULT / PREDIV ) / POSTDIV.
 **/
function ComputeFreq() {
    var freq = 0;
    var regMult = 0;
    var regPreDiv = 0;
    var regPostDiv = 0;

    /* if this block has associated frequency registers, use the current values to compute the freq */
    if (g_freqMult && g_freqPostDiv && g_freqPreDiv) {
        regMult = g_freqMult.state;
        regPreDiv = g_freqPreDiv.state;
        regPostDiv = g_freqPostDiv.state;

        /* if the divisors are not zero */
        if (regPreDiv && regPostDiv) {
            freq = Number(54 * regMult / regPreDiv / regPostDiv.toFixed(0));
        }
        //FPRINTF( stderr, "%s: mult (%ld); prediv (%ld); postdiv (%ld); freq (%ld)\n", __FUNCTION__, regMult.regValue, regPreDiv.regValue, regPostDiv.regValue, freq );
    }

    return (freq);
}
/* ComputeFreq */

function WalkTreeStructure(node, parent, X, Y, action, queryString, level) {
    var localY = Y;
    var next = NULL;
    var ShrinkCmd = "";
    var ShrinkCmdPos = 0;
    var polarity = 0;
    var levelName = GetNodeName(node);
    var levelIdx = 0;
    var tempY = 0;
    var offset = 40;

    //console.log( "<!-- %s: top - node %s; parent (%s); X (%d); Y (%d); action (%s) polarity (%s)-->\n",
    //    __FUNCTION__, GetNodeName( node ), GetNodeName( parent ), X, Y, GetActionString( action ), GetPolarityString( node ));

    if (node.subclocks && node.subclocks.length) {

        for (levelIdx = 0; levelIdx < node.subclocks.length; levelIdx++) {
            if (levelName == "BVN") {
                var temp3 = 0;
            }
            next = node.subclocks[levelIdx].node;
            //FPRINTF( stderr, "%s: for node %p: calling subclocks: node %p (%s)\n", __FUNCTION__, node, next, next.register );
            if (IsMhzRegister(node)) {
                offset = 0;
            }

            //FPRINTF( stderr, "<!-- %s:       node %s; offset %u -->\n", __FUNCTION__, GetNodeName( node ), offset );
            tempY = WalkTreeStructure(next, node, X + offset, localY + offset, action, queryString, Number(level + 1));

            localY = Math.max(localY, tempY);
        }
    }

    if (action == DRAW_LINES) {
        if (ShrinkCmdPos == NULL) {//DrawConnectingLine( node, parent ); // CAD debug
        }
    } else if (action == DRAW_RECTANGLES) {

        polarity = GetRegisterPolarity(node.register, node.field);

        if ((polarity == HighIsOff) || (polarity == HighIsOn)) {
            DrawRectangle(node, X, Y, ShrinkCmdPos);
        }

        /* if this node is main level node */
        if (node && level == 1 /* && node.field.length == 0 */
        ) {
            blockHasAtLeastOnePowerOn = 0;
            //FPRINTF( stderr, "block (%lu); completed block (%s); mult (%s); prediv (%s); postdiv (%s)\n", blockHasAtLeastOnePowerOn,
            //    GetNodeName( node ), GetNodeName( g_freqMult ), GetNodeName( g_freqPreDiv ), GetNodeName( g_freqPostDiv ));
        } else if (X == GAP_BETWEEN_COLUMNS) /* if this node is the first subnode under the 1st-level main node */
        {
            /* clear out global variables for next pass */
            g_freqMult = NULL;
            g_freqPostDiv = NULL;
            g_freqPreDiv = NULL;
        } else {
            if (polarity == MULT) {
                g_freqMult = node;
            } else if (polarity == PREDIV) {
                g_freqPreDiv = node;
            } else if (polarity == POSTDIV) {
                g_freqPostDiv = node;
            }
            //FPRINTF( stderr, "processing field (%s -> %s; pol %u); mult (%s); prediv (%s); postdiv (%s)\n", GetNodeName( node ), node.field,
            //    polarity, GetNodeName( g_freqMult ), GetNodeName( g_freqPreDiv ), GetNodeName( g_freqPostDiv ));
        }
    }

    //FPRINTF( stderr, "<!-- %s:       node %s: returning Y %d -->\n", __FUNCTION__, node.register, localY );
    return (localY);
}
/* WalkTreeStructure */

function WalkTree(chipPowerStateObj, whichSvgBlock ) {
    var idx = 0;
    var queryString = "/";
    var localY = 10;
    var localYNext = 10;
    var CHIP_POWER_STATE_OBJ = document.getElementById( whichSvgBlock );

    //console.log( "WalkTree - " + whichSvgBlock );
    GlobalSvg = "";
    g_freqMult = NULL;
    g_freqPostDiv = NULL;
    g_freqPreDiv = NULL;
    g_YMax = 0;

    if (CHIP_POWER_STATE_OBJ) {
        CHIP_POWER_STATE_OBJ.innerHTML = "";
        for (idx = 0; idx < chipPowerStateObj.length; idx++) {
            RectCount = 0;

            localYNext = WalkTreeStructure(chipPowerStateObj[idx].node, NULL, DEFAULT_X, localY, DRAW_RECTANGLES, queryString, 1);

            /*
               The X and Y coordinates of each rectangle can't get computed until the final leaf nodes are drawn. Once
               the final leaf nodes are drawn, then we know were to draw the parent node. Once all of the nodes have been
               drawn, go back and connect the parents with the child nodes.
            */

            /*
            WalkTreeStructure( chipPowerStateObj[idx].node, NULL, DEFAULT_X, localY, DRAW_LINES, queryString, 1 );
            */

            localY = g_YMax;
        }

        CHIP_POWER_STATE_OBJ.innerHTML += GlobalSvg;
        CHIP_POWER_STATE_OBJ.style.height = Number(g_YMax + RECTANGLE_HEIGHT + GAP_BETWEEN_ROWS );
        CHIP_POWER_STATE_OBJ.scrollTop = Number(g_YMax + RECTANGLE_HEIGHT + GAP_BETWEEN_ROWS );
    }

}
function ClkgenClick(event) {
    var target = event.target || event.srcElement;
    var id = target.id;
    var svgParent = target.nearestViewportElement;

    // All of the high-level nodes in the Top-Level PLL tree start with CLKGEN_
    if ( svgParent && svgParent.id.indexOf( "REVERSED" ) > 0 ) {
        id = "CLKGEN_" + target.id;
    }

    /* if this element is already in the expand list, take it out */
    if (g_clktreeExpand.indexOf(id) >= 0) {
        var temp_expand = g_clktreeExpand.replace(id, "");
        g_clktreeExpand = temp_expand.replace("++", "+");
        var new_len = g_clktreeExpand.length;
        if (new_len) {
            /* if 1st char is +, remove it */
            g_clktreeExpand = g_clktreeExpand.replace(/^\+?|\+?$/, "");
        }
    } else {
        /* add this element to the list */
        if (g_clktreeExpand.length) {
            g_clktreeExpand += "+";
        }
        g_clktreeExpand += id;
    }

    if ( svgParent ) {
        svgParent.innerHTML = "empty;";
    }

    GlobalSvg = "";
    g_YMax = 0;
    rest_plugin_idx = 0;
    /* point to power plugin */
    sendRequest();
}

function GetClkgenColorClass( node ) {
    var colorClass = "register_hot";
    var num1 = "";
    var num2 = "";
    var pos_paren = 0;
    var pos_of = 0;
    var percentage = 0.0;
    var name = "";

    if ( node ) {
        name = node.register;
        // check to see if this register has a summary like ... (5 of 10)
        pos_paren = node.register.indexOf('(');
        if (pos_paren > 0) {
            var parenthesis = node.register.substr(pos_paren + 1).replace(/\)/g, "");
            pos_of = parenthesis.indexOf(' of ');
            if ( pos_of > 0 ) {
                num1 = parenthesis.substr(0, pos_of);
                num2 = parenthesis.substr(pos_of + 4);
                if (num1 == 0) {
                    colorClass = "register_off";
                } else {
                    percentage = Number(num1 / num2).toFixed(2);
                    if (percentage < 0.50) {
                        colorClass = "register_nearly_off";
                    } else if (percentage < 1.00) {
                        colorClass = "register_nearly_on";
                    }
                }
            } else if ( node.state == 0 ) {
                colorClass = "register_off";
            }
        } else if ( node.state == 0 ) {
            colorClass = "register_off";
        }
    }
    return colorClass;
}
