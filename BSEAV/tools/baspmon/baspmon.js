////////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
//
//  This program is the proprietary software of Broadcom and/or its licensors,
//  and may only be used, duplicated, modified or distributed pursuant to the terms and
//  conditions of a separate, written license agreement executed between you and Broadcom
//  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
//  no license (express or implied), right to use, or waiver of any kind with respect to the
//  Software, and Broadcom expressly reserves all rights in and to the Software and all
//  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
//  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
//  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
//
//  Except as expressly set forth in the Authorized License,
//
//  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
//  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
//  and to use this information only in connection with your use of Broadcom integrated circuit products.
//
//  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
//  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
//  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
//  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
//  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
//  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
//  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
//  USE OR PERFORMANCE OF THE SOFTWARE.
//
//  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
//  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
//  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
//  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
//  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
//  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
//  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
//  ANY LIMITED REMEDY.
//
////////////////////////////////////////////////////////////////////////////////
var MasterDebug=0;
var REFRESH_IN_MILLISECONDS=1000;
var epochSeconds = 0;
var OneSecondCount=0;
var CgiCount=0; // number of times the cgi was called
var CgiTimeoutId=0;
var SetVariableCount=0; // number of times setVariable() is called
var ResponseCount=0; // number of times serverHttpResponse() is called
var objdebug = document.getElementById("debugoutputbox");
var localdatetime = "";
var userAgent = 0;
var eol = "\n"; // end of line character for Firefox, Chrome, Safari (not for ie7 ... use <br>)
var HIDE = false;
var SHOW = true;
var numberOfMemcs = 0;
var passcount=0;
var svgTopOffset = -190; // -50 in the CSS file, the svg if positioned absolute 50 pixels down from top to allow logo to respond to clicks
                         // based of of floatDIV definition in baspmon.css
var svgLeftOffset = -110; // 110 in the CSS file, the svg if positioned absolute 110 pixels from left to allow logo to respond to clicks
var ACTIVE_BLOCK = "ASP"; // switches between ASP and SYSTEM_PORT depending on which block is active
var DashedLinesDisplayed = false;
var TransportBlockTopRightY = 0;
var XptPacketCountPrev = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]; // up to 32 xpt channels
var AspPacketCountPrev = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]; // up to 32 asp channels
var MouseOverCount = 0;
var BITSPERSECOND = 0;
var DDRFREQ = 0;
var PrevXptValues = [0,0,0,0,0,0,0,0,0,0]
var PrevXptValuesIdx = 0;
var PrevXptValuesMax = 10;
var PrevAspValues = [0,0,0,0,0,0,0,0,0,0]
var PrevAspValuesIdx = 0;
var PrevAspValuesMax = 10;
var PreviousTime = 0;
var AverageTimes = [0,0,0,0,0,0,0,0,0,0];
var AverageTimesIdx = 0;
var PreviousTime2 = 0;
var POINTS_PER_GRAPH = 165;
var POWER_PROBE_MAX = 8;
var PowerProbeZeros = [0,0,0,0,0,0,0,0]
var PowerProbeNoResponse = 0; // if we count up to 5 seconds with no response, assume something is wrong with IP addr
var PowerProbeIpAddrComplete = false;
var PowerProbeConfigComplete = false;
var PowerProbe5SecAverage = [[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0]]; // 5-second average for 3 probes
var PowerProbe5SecAverageIdx = [0,0,0]; // 5-second average for 3 probes ... index into which of the 5 values we saved last ... range 0..4
var JSONdata = 0;
var supplies = 0;
var div_floatDIV_top = 0; // this value is set dynamically and used by the getY() functions
var div_floatDIV = 0;
var ResetOption = "";
var ResetDetected = false; // set to true if the time every reverts back to Jan 01, 1970 ... like if the stb reboots
var STBTIME = ""; // set when the actual time comes in from the STB
var COLOR_ROSE = "ea5a49";
var Months=new Array("Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec");
var Weekdays=new Array("Sun","Mon","Tue","Wed","Thu","Fri","Sat");
var XPTRAVEPACKETCOUNT0 = 0;
var XPTRAVEPACKETCOUNT1 = 0;
var ClientStreamerRequestCount = 0; /* total summation of time user hit the PLUS button */
var ClientStreamerActualCount = 0; /* number of PIDs that comes back from the server */
var ClientStreamerAction = 0; // incremented when user hits the PLUS sign; decrements when user hits the MINUS sign
var ClientStreamerNoResponse = 0; // if we count up to 5 seconds with no response, assume something is wrong with IP addr
var ClientStreamerIpAddrComplete = false;
var ClientStreamerConfigComplete = false;
var CfgNewValue = ""; // set to ClientStreamerIpAddrNew=10.14.233.198 when user enters a new IP address
var http_file_streamer_pid = 0;
var http_file_streamer_alert = false;
var BCM45316_IMAGE = "";
var RIGHT_HAND_LAYOUT_IMAGE = "";
var EXTERNAL_DISK_IMAGE = "";

function rtrim(stringToTrim) { return stringToTrim.replace(/\s+$/,"");}

Number.prototype.padZero= function(len){ // needed by localdatetime
    var s= String(this), c= '0';
    len= len || 2;
    while (s.length < len) s= c + s;
    return s;
}

function MyClick(event)
{
    var target = event.target || event.srcElement;
    var id = target.id;
    //alert("MyClick: " + id );
    setVariable(id);
}

function setVariable(fieldName)
{
    var debug=0;
    var fieldValue = "";
    if (debug) alert("setVariable: name " + fieldName );
    var obj=document.getElementById(fieldName);

    SetVariableCount++;

    if (debug) alert("setVariable: name " + fieldName + "; type " + obj.type );
    if (obj) {
        gFieldName = fieldName; // used to send the update to the CGI app
        var arrayname = fieldName.substr(0,4);
        var which_entry = fieldName.substr(4,1);

        //alert("setVariable: type " + obj.type );
        if (obj.type == "checkbox" ) {
            fieldValue = obj.checked;
            if ( fieldName == "PowerProbeCheckbox" ) {
                if ( fieldValue ) {
                    hideOrShow ( "PowerProbeHistograms", SHOW );
                    PowerProbeNoResponse = 0;
                    DrawDashedLines(); // the histograms push everything down ... redraw the dashed lines
                } else {
                    hideOrShow ( "PowerProbeHistograms", HIDE );
                    //setPowerProbeEntriesColor ( "yellow" );
                    DrawDashedLines(); // the histograms push everything up ... redraw the dashed lines
                }
            }
        } else if ( obj.type == "text" ) {
        } else if ( obj.type == "radio" ) {
        } else if ( obj.type == "select-one" ) {
            var selectobj = document.getElementById(fieldName);
        } else if ( obj.tagName == "IMG" ) {
            if ( fieldName == "plus_sign" ) {
                ClientStreamerAction++;
                ClientStreamerRequestCount++;

                if ( ClientStreamerRequestCount == 1 && http_file_streamer_alert == false && http_file_streamer_pid == 0) {
                    alert( "http_file_streamer does not appear to be running" );
                    http_file_streamer_alert = true; // remember not to show the alert again
                }
            } else if ( fieldName == "minus_sign" && ClientStreamerRequestCount > 0 ) {
                ClientStreamerAction--;
                ClientStreamerRequestCount--;
            } else if ( fieldName == "brcmlogo" ) {
                // if previous timeout is already pending, cancel it
                if (CgiTimeoutId) {
                    clearTimeout(CgiTimeoutId);
                }
                window.location.href = "index.html";
            }
        } else if ( obj.type == "button" ) {
            fieldValue = obj.value;
            if ( fieldName == "buttonReset" ) {
                if ( IsChecked( "checkboxAsp" ) ) {
                    ResetOption = 'A';
                } else {
                    ResetOption = 'N';
                }
            }
        } else if ( obj.tagName == "SPAN" ) {
        }

        if (debug) ("variable is " + fieldName + "; type " + obj.type + "; array " + arrayname + "; idx " + which_entry + "; subidx " + sub_index + "; field is " + field + "; value " + fieldValue );
        if (fieldName == "h1baspmon") {
            MasterDebug = 1-MasterDebug;
            if (MasterDebug==1) {
                if (objdebug) {
                    objdebug.style.visibility = "";
                }
            } else {
                if (objdebug) {
                    objdebug.style.visibility = "hidden";
                }
                var CgiCountObj = document.getElementById('cgicount');
                if (CgiCountObj) {
                    CgiCountObj.innerHTML = "";
                }
            }
        }
    }
    //alert("setVariable: done");
}

function DrawDashedLines()
{
    var newpoints = "";
    var MEMC_X = 0;
    var MEMC_Y = 0;
    var rectDIV = document.getElementById('div_floatDIV').getBoundingClientRect();

    // if the boxes have not moved since the last time we drew them, skip drawing them again
    if ( TransportBlockTopRightY == getTop("div_transport") ) return;

    TransportBlockTopRightY = getTop("div_transport");

    div_floatDIV = document.getElementById('div_floatDIV');
    div_floatDIV_top = Math.round(Number( rectDIV.top ) + 3 );

    //console.log("DrawDashedLines: div_mem2mem=" + getX( "div_mem2mem" ) + "," + getY( "div_mem2mem" ) + " ... TOP = " + getTop("div_mem2mem") );
    // Draw the five lines connecting from transport to MEMC to Mem-to-Mem DMA back to MEMC and then to System Port block
    var objline = document.getElementById( "polyline_reg1" ); // goes from MEMC at top to Transport top right corner
    if ( objline ) {
        MEMC_X = Number( getXRight("div_transport") + 30 ); // used in reg2 below
        MEMC_Y = Number( getBottom( "div_memc" ) ); // used in reg2 below
        newpoints = MEMC_X + "," + MEMC_Y;
        newpoints += " " + Number( getXRight("div_transport") + 0 ) + "," + Number(getTop("div_transport") );
        objline.setAttribute('points', newpoints );
    }

    objline = document.getElementById( "polyline_reg2" ); // goes from Mem-to-Mem DMA top to starting point of reg1
    if ( objline ) {
        newpoints = Number( getX( "div_mem2mem" ) + 40 ) + "," + Number( getTop( "div_mem2mem" ) );
        newpoints += " " + MEMC_X + "," + MEMC_Y;
        objline.setAttribute('points', newpoints );
    }

    objline = document.getElementById( "polyline_reg3" ); // goes from Mem-to-Mem DMA top to starting point of reg2
    if ( objline ) {
        var tempX = Number( getX( "div_mem2mem" ) + 40 );
        var tempY = Number( getTop( "div_mem2mem" ) );
        newpoints = Number( getXRight("div_transport") + 150 ) + "," + Number( getBottom( "div_memc" ) ); // used in reg4
        newpoints += " " + Number( tempX ) + "," + Number( tempY );
        objline.setAttribute('points', newpoints );

        var text = document.getElementById( "text_reg3" ); // DTCP Encrypt (Host + SAGE)
        if ( text ) {
            text.setAttribute('x', Number( tempX + 0 ) );
            text.setAttribute('y', Number( tempY - 8 ) );
            text.style.visibility = "";
        }
    }

    var parabolaWidth = 50;
    objline = document.getElementById( "polyline_reg6" ); // parabola (arc) along bottom of MEMC block
    if ( objline ) {
        // create a string similar to: "M10 10 Q 40 80 70 10"
        var startX = Number( getXRight("div_transport") + 150 );
        var startY = Number( getBottom( "div_memc" ) );
        var parabolaHeight = 90;
        newpoints = "M" + Number( startX + (parabolaWidth *2 ) ) + " " + startY;
        newpoints += " Q" + Number( startX + parabolaWidth ) + " " + Number( startY + parabolaHeight );
        newpoints += " " + startX + " " + Number( startY );
        objline.setAttribute('d', newpoints );

        var text = document.getElementById( "text_reg6" ); // memcpy
        if ( text ) {
            text.setAttribute('x', Number( startX + 20 ) );
            text.setAttribute('y', Number( startY + 20 ) );
            text.style.visibility = "";
        }
    }

    objline = document.getElementById( "polyline_reg4" ); // goes from middle-left of System Port to starting point of reg3
    if ( objline ) {
        newpoints = getX( "div_systemport" ) + "," + getY( "div_systemport" ) ;
        // 50 is the width of the arc
        newpoints += " " + Number( getXRight("div_transport") + 200 + parabolaWidth  ) + "," + Number( getBottom( "div_memc" ) );
        objline.setAttribute('points', newpoints );

        var text = document.getElementById( "text_reg4" );
        if ( text ) {
            text.setAttribute('x', Number( getX( "div_systemport" ) - 50 ) );
            text.setAttribute('y', Number( getY( "div_systemport" ) + 90 ) );
            text.style.visibility = "";
        }
    }

    objline = document.getElementById( "polyline_reg5" ); // goes from switch to right-middle of System Port block
    if ( objline ) {
        var switch_points = getX( "div_switch" );
        switch_points += "," + getYMiddle ( "div_systemport" );
        newpoints = switch_points;
        newpoints += " " + getXYRightMiddle ( "div_systemport" );
        objline.setAttribute('points', newpoints );
    }

    // Draw the three lines connecting the ASP block
    var objline = document.getElementById( "polyline_asp1" ); // goes from MEMC at top to Transport top right corner (same as polyline_reg1)
    var aspY = Number(getTop("div_transport") + 100 ) ;
    if ( objline ) {
        newpoints = Number( MEMC_X + 30 ) + "," + aspY;
        newpoints += " " + MEMC_X + "," + MEMC_Y;
        objline.setAttribute('points', newpoints );
    }

    objline = document.getElementById( "polyline_asp2" );
    if ( objline ) {
        newpoints = getX( "div_asp" ) + "," + aspY;
        newpoints += " " + Number( MEMC_X + 30 ) + "," + aspY;
        objline.setAttribute('points', newpoints );
    }

    objline = document.getElementById( "polyline_asp3" );
    if ( objline ) {
        var switch_temp = getXYLeftMiddle( "div_switch" );
        var switch_points = getX( "div_switch" );
        switch_points += "," + aspY;

        newpoints = switch_points;
        newpoints += " " + getXRight( "div_asp" ) + "," + aspY;
        objline.setAttribute('points', newpoints );
    }
}

function MyLoad()
{
    var local = new Date(); // needed for localdatetime
    userAgent = navigator.userAgent;
    //window.onunload(function () { alert("jquery unload"); });
    //alert("load: local " + local );
    objdebug = document.getElementById("debugoutputbox");
    epochSeconds = Math.floor(local.getTime() / 1000);
    //alert("local Date " + epochSeconds );
    tzOffset = local.getTimezoneOffset();
    //alert("TZ offset " + local.getTimezoneOffset() );
    localdatetime = (local.getUTCMonth()+1).padZero() + local.getUTCDate().padZero() + local.getUTCHours().padZero() + local.getUTCMinutes().padZero() + local.getUTCFullYear() + "." + local.getUTCSeconds().padZero();
    if ( passcount == 0 ) {
        var idx=0;
        var newpoints="";
        var rectDIV = document.getElementById('div_floatDIV').getBoundingClientRect();

        div_floatDIV = document.getElementById('div_floatDIV');
        div_floatDIV_top = Math.round(Number( rectDIV.top ) + 3 );

        for(idx=16; idx<32; idx++ ) {
            hideOrShow ( "XPTROW" + idx, HIDE ); // initially hide the rows
            hideOrShow ( "ASPROW" + idx, HIDE ); // initially hide the rows
        }
        hideOrShow ( "PowerProbeHistograms", HIDE );

        // adjust the position of the 45316 PNG and the harddisk PNG to align with the top of the Transport block
        var transport_y = Math.round( Number( getY("div_transport") ) );
        document.getElementById("chp45316").style.top = Math.round( transport_y + 300 );
        document.getElementById("harddisk").style.top = Math.round( transport_y + 400 );
        //document.getElementById("chp45316").setAttribute('y', transport_y + 100 );

        /*
        // Draw the line that connects the 45316 to the Transport block
        objline = document.getElementById( "polyline_45316" );
        if ( objline ) {
            newpoints = Number(getXRight( "chp45316" ) - 12 ) + "," + Number( getTop( "chp45316" ) + 50 );
            newpoints += " " + Number(getX( "div_transport" ) ) + "," + Number( getTop( "chp45316" ) + 50 );
            objline.setAttribute('points', newpoints );
            hideOrShow( "polyline_45316", SHOW );
        }

        // Draw the line that connects the harddisk to the Transport block
        objline = document.getElementById( "polyline_harddisk" );
        if ( objline ) {
            newpoints = Number(getXRight( "harddisk" ) - 10 ) + "," + Number( getTop( "harddisk" ) + 25 );
            newpoints += " " + Number(getX( "div_transport" ) ) + "," + Number( getTop( "harddisk" ) + 25 );
            objline.setAttribute('points', newpoints );
            hideOrShow( "polyline_harddisk", SHOW );
        }
        */

        var new_bottom = Math.max( getBottom( "table_transport" ), getBottom( "table_asp" ) );
        {
            var obj = document.getElementById( "table_switch" );
            if ( obj ) {
                var switch_height = obj.style.height;
                var delta = new_bottom - getBottom( "table_switch" );
                var Y = getY( "table_switch" );
                var bottom = getBottom( "table_switch" );
                var height = obj.style.height;
                obj.style.height += delta;
                Y = getY( "table_switch" );
                bottom = getBottom( "table_switch" );
                height = obj.style.height;
            }
        }

        DrawDashedLines();

        MouseOutSetLinesToGray();
        var obj= document.getElementById( "polyline_reg1" );
        if ( obj ) {
            obj.className.animVal = "pathgray";
            obj.className.baseVal = "pathgray";
        }
    }
    sendCgiRequest();
}
function MyUnload()
{
    alert("unload");
    var debug=0;
    xmlhttp=new XMLHttpRequest();

    var url = "/cgi/baspmon.cgi&action=quit";

    xmlhttp.onreadystatechange= serverHttpResponse;
    xmlhttp.open("GET",url,true);
    xmlhttp.send(null);
}

function randomIntFromInterval(min,max)
{
    return Math.floor(Math.random()*(max-min+1)+min);
}

function hideOrShow ( elementid, trueOrFalse )
{
    var obj=document.getElementById(elementid);
    if ( obj ) {
        if (trueOrFalse) {
            //if (elementid.indexOf('wifi') ) alert("hideOrShow: " + elementid + "; starting to SHOW");
            obj.style.display = '';
            obj.style.visibility = '';
            //alert("hideOrShow: " + elementid + "; SHOW");
        } else {
            //if (elementid.indexOf('wifi') ) alert("hideOrShow: " + elementid + "; starting to HIDE");
            obj.style.display = 'none';
            obj.style.visibility = 'hidden';
            //console.log("hideOrShow: " + elementid + "; HIDE");
        }
    }

    return true;
}

/**
 *  Function: This function will return to the caller the current contents of the innerHTML
 *            field of the element specified.
 **/
function GetInnerHtml ( targetId )
{
    var obj = document.getElementById ( targetId );
    if ( obj ) {
        return obj.innerHTML;
    }
    return ( targetId + " is unknown" );
}
/**
 *  Function: This function will set the innerHTML of the specified element with the value
 *            provided.
 **/
function SetInnerHtml ( targetId, newvalue )
{
    var obj = document.getElementById ( targetId );
    if ( obj ) {
        obj.innerHTML = newvalue;
    }
    return true;
}
function SetBackgroundColor ( targetId, newvalue )
{
    var obj = document.getElementById ( targetId );
    if ( obj ) {
        obj.style.backgroundColor = newvalue;
    }
    return true;
}
var first_request = "";
function sendCgiRequest( )
{
    var debug=0;

    if (OneSecondCount == 0 ) {
        var idx=0;
        var url = "";

        // if previous timeout is already pending, cancel it
        if (CgiTimeoutId) {
            clearTimeout(CgiTimeoutId);
        }

        xmlhttp=new XMLHttpRequest();

        var RandomValue = randomIntFromInterval(1000000,9999999);
        url = "/cgi/baspmon.cgi?randomvalue=" + RandomValue;

        if ( passcount == 0 || ResetDetected ) {
            var local = new Date();
            epochSeconds = Math.floor(local.getTime() / 1000);
            tzOffset = local.getTimezoneOffset();
            url += "&datetime=" + epochSeconds + "&tzoffset=" + tzOffset;

            ResetDetected = false;
        }

        if ( PowerProbeConfigComplete ) {
            var IpAddr = GetInputValue( "PowerProbeIpAddr" );
            if ( IsChecked( "PowerProbeCheckbox" ) && IpAddrIsValid( IpAddr )) {
                url += "&PowerProbeIpAddr=" + IpAddr;

                var idx=0;
                var probe = 0;
                var shunt = 0;
                url += "&PowerProbeShunts=";
                //url += "&PowerProbeShunts=2,2,10,0,0,0,0,0"; /* sample */
                for(idx=0; idx<POWER_PROBE_MAX; idx++ ) {
                    probe = document.getElementById("probe" + idx );
                    // if at least one probe dropdown has been configured, we have enough info to configure the probe board
                    if ( probe && probe.value != -1 ) {
                        shunt = getShuntValueFromIndex ( probe.value );
                    } else {
                        shunt = 0;
                    }
                    if ( idx) url+= ",";
                    url += shunt;
                }
            }
        }

        if ( ClientStreamerConfigComplete ) {
            var IpAddr = GetInputValue( "ClientStreamerIpAddr" );
            if ( IpAddrIsValid( IpAddr ) && ( ClientStreamerRequestCount > 0 /* || ClientStreamerAction != 0 */ ) ) {
                url += "&ClientStreamerIpAddr=" + IpAddr;

                if ( ClientStreamerAction != 0 ) {
                    url += "&ClientStreamerAction=" + ClientStreamerAction;
                }
            }
            ClientStreamerAction = 0;
        }

        if ( CfgNewValue != "" ) {
            url += "&" + CfgNewValue;
            CfgNewValue = "";
        }

        if ( ResetOption != "" ) {
            url += "&ResetOption=" + ResetOption;
            ResetOption = "";
        }

        if (debug==1) alert("sending " + url );

        ScheduleNextRequest( REFRESH_IN_MILLISECONDS );

        if ( passcount == 0 ) first_request = url;
        xmlhttp.onreadystatechange= serverHttpResponse;
        xmlhttp.open("GET",url,true);
        xmlhttp.send(null);

        CgiCount++;
        var CgiCountObj = document.getElementById('cgicount');
        if ( MasterDebug && CgiCountObj) {
            CgiCountObj.innerHTML = "&nbsp;&nbsp;(" + CgiCount + "," + SetVariableCount + "," + ResponseCount + ")";
        }
    }

    OneSecondCount++;
    if (OneSecondCount>=1) {
        //alert("resetting to 0");
        OneSecondCount=0;
    }
}

var first_response = "";
// This function runs as an asynchronous response to a previous server request
function serverHttpResponse ()
{
    var debug=0;
    var CgiCountObj = document.getElementById('cgicount');
    var totalXptMbps = 0;
    var totalAspMbps = 0;

    //CgiCountObj.innerHTML += xmlhttp.readyState + "-" + xmlhttp.status +";   ";
    if (debug) alert("serverHttpResponse: got readyState " + xmlhttp.readyState );

    if (xmlhttp.readyState==4 ) {
        //alert("serverHttpResponse: got readyState " + xmlhttp.readyState + "; status " + xmlhttp.status );

        //var response = xmlhttp.getAllResponseHeaders();
        //var response = xmlhttp.getResponseHeader("Date");
        //alert("Date from header is " + response);

        // only if "OK"

        if (xmlhttp.status == 200) {
            var responseText1 = xmlhttp.responseText.replace(/</g,"&lt;"); // fails on ie7, safari
            var responseText2 = responseText1.replace(/</g,"&lt;"); // fails on ie7, safari

            //if(debug) alert("setting debugdiv");
            //if(objdebug) objdebug.innerHTML = responseText2;

            if (debug) alert("rtrim");
            var responseText = rtrim(xmlhttp.responseText);

            if (debug) alert("split");
            var oResponses = responseText.split( "~" );
            if (debug) alert("num responses is " + oResponses.length );

            // sometimes the very first response is blank; if this happens, send another request very soon after we receive the blank response
            if (responseText.length == 0) {
                if (debug) alert("response is empty; calling setTimeout");
                ResponseCount++;
                ScheduleNextRequest( REFRESH_IN_MILLISECONDS/10 );
            } else {
                //alert("for i = 0 to " + oResponses.length );
                // loop through <response> elements, and add each nested
                if ( passcount == 0 ) first_response = responseText;
                if (objdebug) objdebug.innerHTML = ""; // clear out any previous entries
                for (var i = 0; i < oResponses.length; i++) {
                    var entry = oResponses[i];
                    if (debug==1 && entry.length>1 ) alert("Entry " + entry + "; len " + entry.length + "; next len " + oResponses[i+1].length );
                    if ( entry.length == 0 ) {
                        continue;
                    }
                    if ( entry == "ALLDONE" ) {
                        //if (objdebug) objdebug.innerHTML += entry + eol;
                        ResponseCount++;

                        if ( TransportBlockTopRightY != getTop("div_transport") ) {
                            //console.log( "TopRight was " + TransportBlockTopRightY + " ... now is " + getTop("div_transport") );
                            //DrawDashedLines();
                        }

                        // try to determine if the PowerProbe telnet has connected ... at least one of the return values will be non-zero
                        var total=0;
                        for( var probe=0; probe<PowerProbeZeros.length; probe++ ) {
                            total += PowerProbeZeros[probe];
                            PowerProbeZeros[probe] = 0;
                        }
                        if (objdebug) objdebug.innerHTML += entry + "... probe total=" + total.toFixed(3) + ";  NoResponse=" + PowerProbeNoResponse + eol;
                        var obj_PowerProbeStatusText = document.getElementById( "PowerProbeStatusText" );
                        if ( total == 0 ) {
                            PowerProbeNoResponse++;
                            if ( PowerProbeNoResponse >= 5 ) {
                                if ( obj_PowerProbeStatusText ) obj_PowerProbeStatusText.innerHTML = "No connection";
                                //setPowerProbeEntriesColor ( "yellow" );
                            }
                        } else {
                            if ( obj_PowerProbeStatusText ) obj_PowerProbeStatusText.innerHTML = "Connected";
                            //setPowerProbeEntriesColor ( "#bfff80" ); // light chartreuse
                        }

                        passcount++;
                    } else if ( entry == "MEMCNUM" ) {
                        numberOfMemcs = Number(oResponses[i+1]);
                        for( var idx=numberOfMemcs; idx<3; idx++ ) {
                            hideOrShow( "MEMCROW" + idx, HIDE );
                        }
                        i++;
                    } else if ( entry == "MEMCDESC" ) {
                        var values = oResponses[i+1].split(",");
                        if ( values[0].length ) {
                            SetInnerHtml( values[0], values[1] );
                            var idx = values[0].substr(8,1);
                            hideOrShow( "MEMCROW" + idx, SHOW );
                        }
                        i++;
                    } else if ( entry == "MEMCUTIL" ) {
                        var values = oResponses[i+1].split(",");
                        if ( values[0].length ) {
                            SetInnerHtml( values[0], values[1] );
                        }
                        i++;
                    } else if ( entry == "CPULOAD" ) {
                        var svgobj=document.getElementById( 'cpu_histogram' );
                        var response = oResponses[i+1];
                        // "CPU Avg:  6.2%"
                        var usage = response.substr(8,20);
                        var parts = usage.split('%');
                        SetInnerHtml( entry, response );
                        AddSvgPoints( svgobj, Number(100 - parts[0] ).toFixed(0), 'CPU_USAGE', parts[0], "%" );

                        i++;
                    } else if ( entry == "CPUIRQS" ) {
                        var count = oResponses[i+1];
                        var countHuman = ConvertCountToHuman ( count );
                        SetInnerHtml( entry, "IRQS: " + countHuman ); // IRQS: 1,435
                        i++;
                    } else if ( entry == "CPUMEMCUTIL" ) {
                        //SetInnerHtml( entry, oResponses[i+1] );
                        i++;
                    } else if ( entry == "CPUFREQ" ) {
                        SetInnerHtml( entry, oResponses[i+1] );
                        i++;
                    } else if ( entry == "SYSPORT_IRQ" ) {
                        //SetInnerHtml( entry, oResponses[i+1] );
                        i++;
                    } else if ( entry == "SYSPORT_MBPS" ) {
                        SetInnerHtml( entry, "Mbps: " + oResponses[i+1] );
                        i++;
                    } else if ( entry == "SYSPORT_UTIL" ) {
                        var svgobj=document.getElementById( 'sysport_histogram' );
                        var response = oResponses[i+1];
                        // "  6.2"
                        AddSvgPoints( svgobj, Number(100 - response ).toFixed(0), 'SYSPORT_TEXT', response, "%" );

                        i++;
                    } else if ( entry == "MEMDMA_MEMC" ) {
                        SetInnerHtml( entry, oResponses[i+1] );
                        i++;
                    } else if ( entry.indexOf( "XPTRATE" ) >= 0 ) {
                        // response is like this:  XPTRATE~0x100 47954,0x300 23467, ... pid0 bytes0, pid1 bytes1, pid2 bytes2
                        var splits=oResponses[i+1].split( ",");
                        totalXptMbps = 0;
                        for(var idx=0; idx<splits.length; idx++) {
                            var row_id = "XPTROW" + idx;
                            var values = splits[idx].split( " " );
                            var bits = 0;
                            var mbps = 0;
                            if ( values.length >= 2 ) {
                                bits = Math.max( Number ( ( values[1] - XptPacketCountPrev[idx] ) * 188 * 8 ), 0); // don't allow negative
                                mbps = ConvertBitsToMbps(bits);
                                XptPacketCountPrev[idx] = values[1]; // save for next pass
                            } else {
                                bits = 0;
                                mbps = 0;
                                XptPacketCountPrev[idx] = 0; // save for next pass
                            }
                            if ( splits[idx].length > 1 ) /* stream is active */ {
                                SetInnerHtml( entry + idx, "S" + idx + ": " +/*": PID " + values[0] + */" " + Number(mbps).toFixed(2) + " Mbps");
                                setClass2 ( entry + idx, "tableblue" );
                                hideOrShow ( row_id, SHOW );
                                totalXptMbps = totalXptMbps + Number( mbps );
                                //console.log( "xpt ch " + idx + ":  mbps=" + Number(mbps).toFixed(2) + " ... total=" + totalXptMbps );
                            } else if ( idx < 16 ) {
                                SetInnerHtml( entry + idx, "" );
                                setClass2 ( entry + idx, "tablekhaki" );
                                hideOrShow ( row_id, SHOW );
                            } else {
                                hideOrShow ( row_id, HIDE );
                            }
                        }
                        PrevXptValues[PrevXptValuesIdx] = totalXptMbps.toFixed(2);
                        PrevXptValuesIdx++; if ( PrevXptValuesIdx >= PrevXptValuesMax) PrevXptValuesIdx = 0; //wrap around back to the beginning
                        var avg = 0;
                        //var avgs = "";
                        for(var idx=0; idx<PrevXptValuesMax; idx++ ) {
                            avg = Number( PrevXptValues[idx] ) + avg;
                            //avgs += "+" + PrevXptValues[idx];
                        }
                        avg /= PrevXptValuesMax;
                        //avgs += "=" + avg;

                        var util = convert_bw_to_utilization( totalXptMbps );
                        var XPT_MEMC = /*"Util: " + util.toFixed(2) + */"Mbps: " + Math.ceil(totalXptMbps) /*+ "<br>Avg: " + avg.toFixed(2)*/;
                        var mbps = ConvertBitsToMbps( Number( (XPTRAVEPACKETCOUNT1 - XPTRAVEPACKETCOUNT0) * 188*8 ) );
                        SetInnerHtml( "XPT_MEMC", XPT_MEMC + "<br>Rave: " + mbps + " Mbps");
                        i++;
                    } else if ( entry.indexOf( "XPT_MEMC" ) >= 0 ) {
                        SetInnerHtml( entry, oResponses[i+1] );
                        i++;
                    } else if ( entry == "XPTRAVEPACKETCOUNT" ) {
                        XPTRAVEPACKETCOUNT0 = XPTRAVEPACKETCOUNT1; // save previous value
                        XPTRAVEPACKETCOUNT1 = oResponses[i+1];     // save current value
                        //if (objdebug) objdebug.innerHTML += entry + ":" + oResponses[i+1] + " ... prev:" + XPTRAVEPACKETCOUNT0 + " ... diff " +
                        //    Number( (XPTRAVEPACKETCOUNT1 - XPTRAVEPACKETCOUNT0)) + eol;
                        i++;
                    } else if ( entry.indexOf( "ASP_MHZ" ) >= 0 ) {
                        SetInnerHtml( entry, oResponses[i+1] );
                        i++;
                    } else if ( entry.indexOf( "ASP_MEMC" ) >= 0 ) {
                        SetInnerHtml( entry, oResponses[i+1] );
                        i++;
                    } else if ( entry.indexOf( "ASPRATE" ) >= 0 ) {
                        // response is like this:  ASPRATE~15.3,11.5,26.8,  bitrate0, bitrate1, bitrate2, etc
                        var splits=oResponses[i+1].split( ",");
                        totalAspMbps = 0;
                        for(var idx=0; idx<splits.length; idx++) {
                            var row_id = "ASPROW" + idx;
                            var bits = Math.max( Number ( ( splits[idx] - AspPacketCountPrev[idx] ) * 188 * 8 ), 0); // don't allow negative
                            var mbps = ConvertBitsToMbps(bits);
                            AspPacketCountPrev[idx] = splits[idx]; // save for next pass
                            if ( splits[idx].length > 1 ) /* stream is active */ {
                                SetInnerHtml( entry + idx, "S" + idx + ": " + Number(mbps).toFixed(1) + " Mbps");
                                setClass2 ( entry + idx, "tableblue" );
                                hideOrShow ( row_id, SHOW );
                                totalAspMbps = totalAspMbps + Number( mbps );
                            } else if ( idx < 16 ) {
                                SetInnerHtml( entry + idx, "" );
                                setClass2 ( entry + idx, "tablekhaki" );
                                hideOrShow ( row_id, SHOW );
                            } else {
                                hideOrShow ( row_id, HIDE );
                            }
                        }

                        PrevAspValues[PrevAspValuesIdx] = totalAspMbps.toFixed(2);
                        PrevAspValuesIdx++; if ( PrevAspValuesIdx >= PrevAspValuesMax ) PrevAspValuesIdx = 0; //wrap around back to the beginning
                        var avg = 0;
                        for(var idx=0; idx<PrevAspValuesMax; idx++ ) {
                            avg = Number( PrevAspValues[idx] ) + avg;
                        }
                        avg /= PrevAspValuesMax;

                        var util = convert_bw_to_utilization( totalAspMbps );
                        var ASP_MEMC = /*"Util: " + util.toFixed(2) + */"Mbps: " + Math.ceil(totalAspMbps) /*+ "<br>Avg: " + avg.toFixed(2)*/;
                        SetInnerHtml( "ASP_MEMC", ASP_MEMC );

                        // if this is the last element, redraw the dashed lines just in case the unhidden rows caused the blocks to move
                        //DrawDashedLines();

                        i++;
                    } else if ( entry == "ACTIVE_BLOCK" ) {
                        //if (objdebug) objdebug.innerHTML += entry + ":" + oResponses[i+1] + eol;
                        if ( ACTIVE_BLOCK != oResponses[i+1] && temp_ACTIVE_BLOCK == "none" ) {
                            ACTIVE_BLOCK = oResponses[i+1];
                            MouseOutSetLinesToGray();
                            MouseOverSetLinesToBlue();
                        }
                        i++;
                    } else if ( entry == "POWERPROBE" ) {
                        //if (objdebug) objdebug.innerHTML += entry + ":" + oResponses[i+1] + eol;
                        var parts = oResponses[i+1].split(","); // each entry has probe index followed by probe power ... e.g. 1.703
                        if ( parts.length >= 2) {
                            var tag = entry + parts[0];
                            var svgobj=document.getElementById( tag );
                            // The range of the power numbers is [0 .. 4] (according to Anand). Since the histogram expects values
                            // between [0 .. 100], multiply the power values by 25 to compensate.
                            AddSvgPoints( svgobj, Number(100 - (parts[1] * 25) ).toFixed(0), tag + '_USAGE', parts[1], "" );

                            // save this value into the 5-second array
                            PowerProbe5SecAverage[ parts[0] ][ PowerProbe5SecAverageIdx[parts[0]] ] = parts[1];
                            PowerProbe5SecAverageIdx[parts[0]]++;
                            if ( PowerProbe5SecAverageIdx[parts[0]] >= 5) PowerProbe5SecAverageIdx[parts[0]] = 0; // wrap around to beginning
                            var avg = PowerProbe5SecAverageCompute(parts[0]);
                            svgobj=document.getElementById( tag + "_AVG" );
                            AddSvgPoints( svgobj, Number(100 - (avg * 25) ).toFixed(0), "", "", "" );

                            // when we get to the last of the 3 probes, compute the total power usage
                            if ( parts[0] == "2" ) {
                                // add the 3 probe values together
                                var total = 0;
                                for( var idx=0; idx<3; idx++ ) {
                                    avg = PowerProbe5SecAverageCompute( idx );
                                    total = total + avg;
                                }
                                svgobj=document.getElementById( tag + "_TOT" );
                                AddSvgPoints( svgobj, Number(100 - ( Math.min(total,4) * 25) ).toFixed(0), tag + '_TOTAL', total.toFixed(3), "" );
                            }

                            if ( parts[0] < PowerProbeZeros.length && Number(parts[1]) > 0 ) {
                                PowerProbeZeros[parts[0]] = Number(parts[1]);
                                PowerProbeNoResponse = 0; // reset the counter to indicate we have good communications with power probe
                                //if (objdebug) objdebug.innerHTML += entry + ":" + oResponses[i+1] + eol;
                            }
                        }
                        i++;
                    } else if ( entry == "PowerProbeIpAddr" ) {
                        SetInputValue( "PowerProbeIpAddr", oResponses[i+1] );
                        IpAddrValidate( entry );
                        i++;
                    } else if ( entry == "ClientStreamerIpAddr" ) {
                        SetInputValue( "ClientStreamerIpAddr", oResponses[i+1] );
                        IpAddrValidate( entry );
                        i++;
                    } else if ( entry == "BCM45316_IMAGE" ) {
                        BCM45316_IMAGE = oResponses[i+1];
                        SetImageSrc( "chp45316",  oResponses[i+1] );
                        i++;
                    } else if ( entry == "RIGHT_HAND_LAYOUT_IMAGE" ) {
                        RIGHT_HAND_LAYOUT_IMAGE = oResponses[i+1];
                        SetImageSrc( "rightimg",  oResponses[i+1] );
                        i++;
                    } else if ( entry == "EXTERNAL_DISK_IMAGE" ) {
                        EXTERNAL_DISK_IMAGE = oResponses[i+1];
                        SetImageSrc( "harddisk",  oResponses[i+1] );
                        i++;
                    } else if ( entry == "ClientStreamerThreadCount" ) {
                        SetInnerHtml( "ClientStreamerThreadCount", "(" + oResponses[i+1] + ")" );
                        if ( oResponses[i+1] > 0 ) {
                            SetInnerHtml( "ClientStreamerStatusText", "Connected" );
                        } else {
                            SetInnerHtml( "ClientStreamerStatusText", "No connection" );
                        }
                        i++;
                    } else if ( entry == "checkboxAsp" ) {
                        CheckIt( entry );
                        i++;
                    } else if ( entry == "http_file_streamer_pid" ) {
                        http_file_streamer_pid = oResponses[i+1];
                        i++;
                    } else if ( entry == "DEBUG" ) {
                        if (objdebug) objdebug.innerHTML += entry + ":" + oResponses[i+1] + eol;
                        console.log(entry + ":" + oResponses[i+1]);
                        i++;
                    } else if (entry == "STBTIME") {
                        SetInnerHtml ( "stbtime", oResponses[i+1] );
                        STBTIME = oResponses[i+1];
                        SetBackgroundColor ( "stbtime", "white" );

                        if ( STBTIME.indexOf( "Jan 01, 1970" ) > 0 ) {
                            ResetDetected = true; // need to send the time over to the stb again
                        }

                        i++;
                    } else if (entry == "VERSION") {
                        //alert("got response: " + entry );
                        var objversion =document.getElementById("version");
                        if (objversion) {
                            objversion.innerHTML = oResponses[i+1];
                        }
                        i++;
                    } else if (entry == "PLATFORM") {
                        var objplatform =document.getElementById("platform");
                        if (objplatform) {
                            objplatform.innerHTML = oResponses[i+1]; CurrentPlatform = oResponses[i+1];

                            readTextFile( "BCM" + oResponses[i+1] + ".json"); // read the associated JSON data file for power probes
                        }
                        i++;
                    } else if (entry == "PLATVER") {
                        var objplatform =document.getElementById("platver");
                        if (objplatform) {
                            objplatform.innerHTML = oResponses[i+1]
                        }
                        window.document.title = CurrentPlatform + " " + oResponses[i+1];
                        i++;
                    } else if (entry == "VARIANT") {
                        var objvariant = document.getElementById("VARIANT");
                        if ( objvariant ) {
                            objvariant.innerHTML = "(Variant: " + oResponses[i+1] + ")";
                        }
                        i++;
                    } else if (entry == "UNAME") {
                        //alert(entry + ":" + oResponses[i+1] );
                        var objplatform = document.getElementById("UNAME");
                        if (objplatform) {
                            objplatform.innerHTML = "Kernel: " + oResponses[i+1];
                        }
                        i++;
                    } else if (entry == "BOLTVER") {
                        var objplatform = document.getElementById("BOLTVER");
                        if (objplatform) {
                            objplatform.innerHTML = "Bolt: " + oResponses[i+1];
                        }
                    } else if (entry == "BITSPERSECOND") {
                        BITSPERSECOND = oResponses[i+1];
                        i++;
                    } else if (entry == "DDRFREQ") {
                        DDRFREQ = oResponses[i+1];
                        i++;
                    } else {
                        //if (objdebug) objdebug.innerHTML += entry + eol; // causes problems on IE8
                        // if response has html tags in it <input ...>, you cannot display the response in a textarea box
                        // to resolve this IE feature, globally replace the < and > chars with &lt; and &gt;
                        if (objdebug && entry.length && entry.charAt(0) != '\n' ) {
                            if (userAgent.indexOf("MSIE") >= 0 ) { // for ie9, must replace greater than and less than with &gt &lt
                                var newtext = entry.replace(/</g,"&lt;");
                                //alert("MSIE detected (" + entry + ")" );
                                objdebug.innerHTML = newtext.replace(/>/g,"&gt;") + "<br>";
                            } else {
                                //alert("Safari/Firefox/Chrom detected (" + entry + ")" );
                                objdebug.innerHTML += entry;
                                // if the entry does not have an end-of-line character, output one now
                                if ( entry.indexOf( '\n' ) < 0 ) objdebug.innerHTML += eol;
                            }

                            //objdebug.scrollTop = objdebug.scrollHeight;
                        }
                    }
                } // for each response
            } // if response is not blank
        }

        else {
            var local = new Date();
            // Thu Apr 13, 2017 12:27:07
            var newdatetime = Weekdays[local.getDay()] + " " + Months[local.getMonth()] + " " + local.getDate() + ", " +
                                   Number(local.getYear() + 1900) + " " + local.getHours().padZero() + ":" + local.getMinutes().padZero() +
                                   ":" + local.getUTCSeconds().padZero();
            SetInnerHtml ( "stbtime", newdatetime );
            SetBackgroundColor ( "stbtime", COLOR_ROSE );
            console.log("There was a problem retrieving the XML data:\n" + xmlhttp.statusText);
        }

    } //if (xmlhttp.readyState==4 )
} // this.serverHttpResponse

function getXYRightMiddle ( elementId )
{
    var obj = document.getElementById ( elementId );
    if ( obj ) {
        var rect = obj.getBoundingClientRect();
        //console.log( "topleft:" + rect.left + "," + rect.top + ";  btmright:" + rect.right + "," + rect.bottom, rect.height, rect.width );
        //console.log( "for " + elementId + " ... " + Number(rect.left + rect.width) + "," + Number(rect.top + rect.height/2).toFixed(0));
        return Number( svgLeftOffset + rect.left + rect.width) + "," + Number( rect.top - div_floatDIV_top + rect.height/2).toFixed(0);
    }

    return "0,0";
}

function getYMiddle ( elementId )
{
    var obj = document.getElementById ( elementId );
    if ( obj ) {
        var rect = obj.getBoundingClientRect();
        return Number( rect.top - div_floatDIV_top + rect.height/2).toFixed(0);
    }

    return "0";
}

function getXYLeftMiddle ( elementId )
{
    var obj = document.getElementById ( elementId );
    if ( obj ) {
        var rect = obj.getBoundingClientRect();
        //console.log( "topleft:" + rect.left + "," + rect.top + ";  btmright:" + rect.right + "," + rect.bottom, rect.height, rect.width );
        //console.log( "for " + elementId + " ... " + Number(rect.left) + "," + Number(rect.top + rect.height/2).toFixed(0));
        return Number(svgLeftOffset + rect.left) + "," + Number( rect.top - div_floatDIV_top + rect.height/2).toFixed(0);
    }

    return "0,0";
}

function getXRight ( elementId )
{
    var obj = document.getElementById ( elementId );
    if ( obj ) {
        var rect = obj.getBoundingClientRect();
        return Math.round( Number(svgLeftOffset + rect.left + rect.width) );
    }

    return "0,0";
}

function getX ( elementId )
{
    var obj = document.getElementById ( elementId );
    if ( obj ) {
        var rect = obj.getBoundingClientRect();
        return Math.round( Number( svgLeftOffset + rect.left ).toFixed(0) );
    }

    return "0";
}

function getY ( elementId )
{
    var obj = document.getElementById ( elementId );
    if ( obj ) {
        var rect = obj.getBoundingClientRect();
        return Math.round( Number( rect.top - div_floatDIV_top ).toFixed(0) );
    }

    return "0";
}

function getTop ( elementId )
{
    var obj = document.getElementById ( elementId );
    if ( obj ) {
        var rect = obj.getBoundingClientRect();
        return Math.round( Number( rect.top - div_floatDIV_top ).toFixed(0) );
    }

    return "0";
}

function getBottom ( elementId )
{
    var obj = document.getElementById ( elementId );
    if ( obj ) {
        var rect = obj.getBoundingClientRect();
        return Math.round( Number( rect.bottom - div_floatDIV_top ).toFixed(0) );
    }

    return "0";
}

function setClass ( elementid, newClass )
{
    var obj=document.getElementById(elementid);

    if ( obj ) {
        //obj.class = newClass;
        if ( newClass.indexOf( "gray" ) > 0 ) {
            obj.style.stroke = "gray";
            obj.setAttribute('style', "stroke:gray;stroke-width:1" );
            obj.className.animVal = "pathgray";
            obj.className.baseVal = "pathgray";
        } else {
            obj.style.stroke = "blue";
            obj.setAttribute('style', "stroke:blue;stroke-width:4" );
            obj.setAttribute('animation', "dash 5s linear" );
            obj.className.animVal = "pathblue";
            obj.className.baseVal = "pathblue";
        }
    }

    return true;
}

function setClass2 ( elementid, newClass )
{
    var obj=document.getElementById(elementid);

    if ( obj ) {
        obj.className = newClass;
    }

    return true;
}

var temp_ACTIVE_BLOCK = "none";
function MouseOver(evt)
{
    var targetId = "";
    if (evt) {
        MouseOverCount++;

        DrawDashedLines();

        targetId = evt.target.id;
        if ( targetId.indexOf( "switch_" == 0 ) ) {
            DashedLinesDisplayed = true;

            temp_ACTIVE_BLOCK = ACTIVE_BLOCK; // save the current value
            if ( MouseOverCount%2 ) { // CAD debug
                ACTIVE_BLOCK = "ASP";
            } else {
                ACTIVE_BLOCK = "SYSTEM_PORT";
            }
            MouseOutSetLinesToGray();
            MouseOverSetLinesToBlue();
        }
    }
}

function MouseOut(evt)
{
    var targetId = "";
    if (evt) {
        targetId = evt.target.id;
        if ( targetId.indexOf( "switch_" == 0 ) ) {
            DashedLinesDisplayed = false;

            ACTIVE_BLOCK = temp_ACTIVE_BLOCK; // restore the saved value

            MouseOutSetLinesToGray();
            MouseOverSetLinesToBlue();

            temp_ACTIVE_BLOCK = "none";
        }
    }
}

function MouseOverSetLinesToBlue()
{
    if ( ACTIVE_BLOCK == "SYSTEM_PORT" ) {
        setClass ( "polyline_reg1", "pathblue" );
        setClass ( "polyline_reg2", "pathblue" );
        setClass ( "polyline_reg3", "pathblue" );
        setClass ( "polyline_reg4", "pathblue" );
        setClass ( "polyline_reg5", "pathblue" );
        setClass ( "polyline_reg6", "pathblue" );
    } else {
        setClass ( "polyline_reg1", "pathblue" );
        setClass ( "polyline_asp1", "pathblue" );
        setClass ( "polyline_asp2", "pathblue" );
        setClass ( "polyline_asp3", "pathblue" );
    }

    FiveSecondTimer();
}

function MouseOutSetLinesToGray()
{
    setClass ( "polyline_reg1", "pathgray" );
    setClass ( "polyline_reg2", "pathgray" );
    setClass ( "polyline_reg3", "pathgray" );
    setClass ( "polyline_reg4", "pathgray" );
    setClass ( "polyline_reg5", "pathgray" );
    setClass ( "polyline_reg6", "pathgray" );
    setClass ( "polyline_asp1", "pathgray" );
    setClass ( "polyline_asp2", "pathgray" );
    setClass ( "polyline_asp3", "pathgray" );
}

function ReplacePolyline( elementId )
{
    var dashed000 = document.getElementById( elementId );
    if ( dashed000 ) {
        var newdash = dashed000.cloneNode(true);
        dashed000.parentNode.replaceChild(newdash, dashed000);
    }

    return true;
}

var FiveSecondTimerId = 0;
function FiveSecondTimer()
{
    var obj = document.getElementById( "polyline_reg1" );
    //console.log("FiveSecondTimer: animation " + obj.style.webkitAnimationPlayState );
    if ( ACTIVE_BLOCK == "ASP" ) {
        ReplacePolyline( "polyline_reg1" );
        ReplacePolyline( "polyline_asp1" );
        ReplacePolyline( "polyline_asp2" );
        ReplacePolyline( "polyline_asp3" );
    } else {
        ReplacePolyline( "polyline_reg1" );
        ReplacePolyline( "polyline_reg2" );
        ReplacePolyline( "polyline_reg3" );
        ReplacePolyline( "polyline_reg4" );
        ReplacePolyline( "polyline_reg5" );
        ReplacePolyline( "polyline_reg6" );
    }
    //dashed000 = document.getElementById("dashed000");
    //var newdash = dashed000.cloneNode(true);
    //dashed000.parentNode.replaceChild(newdash, dashed000);
    if ( FiveSecondTimerId ) clearTimeout( FiveSecondTimerId );
    //FiveSecondTimerId = setTimeout( "FiveSecondTimer()", 5000 );
}

function ComputePrecision ( Mbps )
{
    var precision = 2;
    var integerPart = Mbps.toFixed(0);
    var decimalPoint = ""; // if the integer part is 4 digits or more, add the decimal point after the integer

    // if the Mbps is less than one Mbps, increase the decimal precision to show kilobits
    if ( integerPart == 0 ) {
        precision = 3;
    } else if ( integerPart.length > 3 ) {
        precision = 0;
        decimalPoint = ".";
    } else if ( integerPart.length > 2 ) {
        precision = 1;
    }

    return [ precision, decimalPoint ];
}

function ConvertBitsToMbps ( bits )
{
    var Mbps      = 0;
    var precision = 2;
    var decimalPoint = ""; // if the integer part is 4 digits or more, add the decimal point after the integer

    Mbps =  Number( bits / 1024 / 1024 );

    var return_values = ComputePrecision ( Mbps );
    precision = return_values[0];
    decimalPoint = return_values[1];

    return Mbps.toFixed(precision) + decimalPoint;
}
function convert_bw_to_utilization ( bw )
{
    var util = 99;
    if ( BITSPERSECOND && DDRFREQ ) {
        util = Number( bw * 8 * 100 ) / Number( BITSPERSECOND * DDRFREQ );
    }

    return util;
}

function ScheduleNextRequest( delay )
{
    var debug=0;
    var local = new Date();
    var delta = 0;
    var total = 0;
    if ( CgiTimeoutId ) clearTimeout( CgiTimeoutId );
    CgiTimeoutId = setTimeout ('sendCgiRequest()', delay );
    delta = Number (local - PreviousTime);
    AverageTimes[AverageTimesIdx++] = delta;
    for ( var idx=0; idx<AverageTimes.length; idx++ ) {
        total = AverageTimes[idx];
    }
    if ( AverageTimesIdx >= 10 ) AverageTimesIdx = 0;
    if (debug && objdebug) objdebug.innerHTML += "calling setTimeout(); ID (" + CgiTimeoutId + ") ... delta " + delta + "; ... avg " + Number(total).toFixed(2);
    PreviousTime = local;
}

function PrintDeltaTime( tag )
{
    var debug=0;
    var local = new Date();
    var delta = 0;
    delta = Number (local - PreviousTime2);
    if (debug && objdebug) objdebug.innerHTML += tag + " ... delta " + delta + eol;
    PreviousTime2 = local;
}

function getNumEntries ( arrayname )
{
    var num_entries = arrayname.length;
    if (userAgent.indexOf("MSIE") >= 0 ) {
        //num_entries++; // for ie9, length is one less than firefox, chrome, safari
    }
    //alert("1 array(" + arrayname + "); num points" + num_entries );

    return num_entries;
}
/*
 * This function is called to update the CPU utilization graph as well as the Network Interface graphs.
*/
function AddSvgPoints ( svgobj, newValue, textTarget, textValue, textSuffix /* could be % */ )
{
    var coords = svgobj.getAttribute('points' );
    var minYcoord = 100;

    if ( textTarget.indexOf( "PROBE" ) > 0 ) {
        var temp = textTarget;
    }
    if (coords == null) {
        coords = "0," + newValue + " ";
    }

    if (coords) {
        var coords2 = rtrim(coords);
        var splits = coords2.split(' ');
        var newcoords = "";
        var starting_idx = 0;
        var num_entries = getNumEntries(splits);

        if ( num_entries < POINTS_PER_GRAPH ) {
            starting_idx = 0;
        } else {
            starting_idx = 1;// skip the first element; it is dropping off the left side
        }
        for (idx=starting_idx; idx < num_entries; idx++ ) {
            var justone = splits[idx].split(',');
            newcoords += idx*2 + "," + justone[1] + " " ;
            minYcoord = Math.min ( minYcoord, justone[1] );
        }

        // add new last point to the far right
        newcoords += idx*2 + "," + newValue + " " ;
        minYcoord = Math.min ( minYcoord, newValue );

        svgobj.setAttribute('points', newcoords );

        if ( textTarget.length ) {
            var X = Math.min ( Number( idx*2 + 2 ), 280 ); // don't let X coord get too close to right of histogram
            var Y = Math.max ( Number( newValue - 5 ), 15 ); // don't let Y coord get too close to top of histogram

            // change position of the floating text so that text is at the right end of the polyline
            var text=document.getElementById( textTarget );
            if ( text ) {
                text.innerHTML = textValue + textSuffix; // + " ... " + num_entries;
                text.setAttribute('x', X );
                text.setAttribute('y', Y );
            }
        }
    }

    return minYcoord;
}

function readTextFile(file)
{
    var rawFile = new XMLHttpRequest();
    rawFile.open("GET", file, false);
    rawFile.onreadystatechange = function ()
    {
        if(rawFile.readyState === 4)
        {
            if(rawFile.status === 200 || rawFile.status == 0)
            {
                var allText = rawFile.responseText;
                var option = 0;

                //console.log(allText);
                // convert file contents into JSON object
                JSONdata = JSON.parse(allText);

                // save all known valid rails
                supplies = new Array(num_rails);
                var supplies_idx = 0;
                for (var rail in JSONdata.config_rails) {
                    if (!JSONdata["config_rails"].hasOwnProperty(rail)) {
                        continue;
                    }
                    supplies[supplies_idx] = rail;

                    supplies_idx++;
                }

                // create dropdown boxes for just 3 probes
                for(var probe_idx=0; probe_idx<3; probe_idx++ ) {
                    var obj=document.getElementById( "probe" + probe_idx );
                    var y = obj.options;
                    var supplies_idx = 0;
                    var num_rails = Object.keys(JSONdata.config_rails).length;

                    for (var rail=0; rail<supplies.length; rail++) {
                        // only show the elements that are shown in the UUT Info Table web page
                        var nominalv = JSONdata["config_rails"][ supplies[rail] ]["nominalv"]; // 1.0 for example
                        var volt_refdes = JSONdata["config_rails"][ supplies[rail] ]["volt_refdes"]; // J5102 for example
                        if ( "0.0" < nominalv && nominalv <= "1.1" ) {
                            option = document.createElement("option");
                            option.text = "[" + volt_refdes + "] " + supplies[rail];
                            option.value = rail;
                            obj.add(option);
                        }
                    }
                }
            }
        }
    }
    rawFile.send(null);
}

function IsChecked ( targetId )
{
    var obj=document.getElementById( targetId );
    if ( obj && obj.checked ) return true;
    return false;
}

function CheckIt ( targetId )
{
    var obj=document.getElementById( targetId );
    if ( obj ) obj.checked = true;
    return true;
}

function IpAddrIsValid ( IpAddr )
{
    var parts=IpAddr.split('.');
    if ( parts.length == 4 && parts[3].length > 0 ) return true;
    return false;
}
// called when user has finished inputting IP address ... either by hitting ENTER or moving focus to some other element (TAB)
function IpAddrBlur( event )
{
    var valid = false;
    if ( event ) {
        var targetId = event.target.id;
        IpAddrValidate( targetId );
    }
}
// called when user has finished inputting IP address ... either by hitting ENTER or moving focus to some other element (TAB)
function IpAddrValidate( targetId )
{
    var valid = false;
    var ipaddr=document.getElementById( targetId );
    var parts=ipaddr.value.split('.');
    //console.log( ipaddr.value + " ... values " + parts.length );
    if ( parts.length == 4 && parts[3].length > 0 ) {
        valid = true;
    }

    if ( targetId == "PowerProbeIpAddr" ) {
        PowerProbeIpAddrComplete = valid;
        if ( valid ) {
            ProbeDataComplete();
            if ( passcount ) CfgNewValue = targetId + "New=" + ipaddr.value;
        }
    } else if ( targetId == "ClientStreamerIpAddr" ) {
        ClientStreamerIpAddrComplete = valid;
        ClientStreamerConfigComplete = valid;
        if ( valid ) {
            if ( passcount ) CfgNewValue = targetId + "New=" + ipaddr.value;
        }
    }
}

function MyKeyUp(event)
{
    //alert("MyKeyUp");
    PowerProbeIpAddrComplete = false;

    if ( event.keyCode == 13 /* ENTER */ ) {
        IpAddrBlur( event );
    }
}

function setPowerProbeEntriesColor ( color )
{
    var ipaddr = document.getElementById("PowerProbeIpAddr");
    if ( ipaddr ) ipaddr.style.backgroundColor=color;

    var lstatus = document.getElementById("PowerProbeStatusText");
    if ( lstatus ) lstatus.style.backgroundColor=color;

    for( var idx=0; idx<3; idx++ ) {
        probe = document.getElementById("probe" + idx );
        if ( probe ) probe.style.backgroundColor=color;
    }
}

function ProbeDataComplete()
{
    var color = "white"; // rose "ea5a49";

    PowerProbeConfigComplete = false; // assume the config is not complete

    if ( PowerProbeIpAddrComplete ) {
        var idx=0;
        var probe = 0;
        for(idx=0; idx<3; idx++ ) {
            probe = document.getElementById("probe" + idx );
            // if at least one probe dropdown has been configured, we have enough info to configure the probe board
            if ( probe && probe.value != -1 ) {
                color = "yellow";
                PowerProbeConfigComplete = true;
                break;
            }
        }
    }
    //setPowerProbeEntriesColor ( color );
}

function getShuntValueFromIndex ( supplies_idx )
{
    var idx=0;
    var shunt=0;
    shunt = JSONdata["config_rails"][ supplies[supplies_idx] ]["shunt_r_mohm"];
    return shunt;
}

function getShuntValueFromName ( supplies_name )
{
    var idx=0;
    var shunt=0;

    for(idx=0; idx<JSONdata.config_rails.length; idx++) {
        var temp1 = JSONdata.config_rails[idx];
        var temp2 = JSONdata.config_rails[idx].shunt_r_mohm;
        if ( JSONdata.config_rails[idx] == supplies_name ) {
            shunt = JSONdata.config_rails[idx].shunt_r_mohm;
            break;
        }
    }

    return shunt;
}

function ProbeSelect (event)
{
    var target = event.target.id;
    var obj=document.getElementById( target );
    var ipaddr=document.getElementById("PowerProbeIpAddr");
    var idx = obj.selectedIndex;
    var y = obj.options;
    var text = y[idx].text;
    //alert("ProbeSelect: idx is " + idx + ";   Index:" + y[idx].index + ";   text:" + text + "; value:" + y[idx].value + ";   target=" + target);
    ProbeDataComplete();
}
/**
 *  Function: This function will output a human-readable number of bytes.
 **/
function ConvertCountToHuman ( count )
{
    var precision = 1; // could change to 2 if the number is small enough
    var human = ""

    if ( count < 1024)
    {
        return ( count );
    }

    if ( count < 1024*1024)
    {
        human = ( Number( count / 1024).toFixed(precision) + "K" );
    }
    else if ( count < 1024*1024*1024)
    {
        human = ( Number( count / 1024 / 1024).toFixed(precision) + "M" );
    }
    else
    {
        human = ( Number( count / 1024 / 1024 / 1024).toFixed(precision) + "G" );
    }

    // if the final number with 1 precision is really short, do it again with more precision
    if ( human.length <= 4 ) {
        precision = 2;
        if ( count < 1024*1024)
        {
            human = ( Number( count / 1024).toFixed(precision) + "K" );
        }
        else if ( count < 1024*1024*1024)
        {
            human = ( Number( count / 1024 / 1024).toFixed(precision) + "M" );
        }
        else
        {
            human = ( Number( count / 1024 / 1024 / 1024).toFixed(precision) + "G" );
        }
    }

    return human;
}
function PowerProbe5SecAverageCompute( ProbeIdx )
{
    var idx=0;
    var avg=0;
    var value = 0;
    for(idx=0; idx<5; idx++) {
        value = PowerProbe5SecAverage[ProbeIdx][idx];
        avg = avg + Number(value);
    }
    return (avg/5);
}
function GetInputValue( targetId )
{
    var obj=document.getElementById( targetId );
    if ( obj ) return obj.value;
    return "";
}

function SetInputValue ( fieldName, newValue )
{
    var rc=0;
    var obj=document.getElementById( fieldName );
    if (obj) {
        obj.value = newValue;
    }
    return rc;
}

function SetImageSrc( fieldName, newValue )
{
    var rc=0;
    if ( newValue.length == 0 ) return rc;

    var obj=document.getElementById( fieldName );
    if (obj) {
        obj.src = newValue;
        hideOrShow ( fieldName, SHOW );
    }
    return rc;
}
