/******************************************************************************
 *    (c)2008-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/
var MasterDebug=0;
var GlobalDebug = 0;
var SetVariableCount=0; // number of times setVariable() is called
var gFieldName = "";
var CgiTimeoutId=0;
var CgiCount=0; // number of times the cgi was called
var urlSentPreviously = "";
var debugobj=0;
var userAgent = 0;
var EOL = "\n"; // end of line character for Firefox, Chrome, Safari (not for ie7)
var epochSeconds = 0;
var tzOffset = 0;
var localdatetime = "";
var validResponseReceived = 0;
var ShrinkList = ""; // when user clicks the first-level rectangle, either shrink it or expand it
var SvgHeight = 0;
var GetShrinkList = 1; // during initialization, get a list of the upper block names. these will be used to force all upper blocks to be collapsed (shrunk)
var GetClocks =  {Value: 0};
var passcount=0;

function rtrim(stringToTrim) { return stringToTrim.replace(/\s+$/,""); }

Number.prototype.padZero= function(len){
    var s= String(this), c= '0';
    len= len || 2;
    while (s.length < len) s= c + s;
    return s;
}

function MyLoad ()
{
    var debug=0;
    if(debug) alert("MyLoad");
    debugobj=document.getElementById("debugoutputbox");
    userAgent = navigator.userAgent;
    if (userAgent.indexOf("MSIE") >= 0 ) { // for ie9, must replace carriage returns with <br>
        EOL = "<br>";
    }
    if ( passcount == 0 ) {
        var local = new Date();
        epochSeconds = Math.floor(local.getTime() / 1000);
        //alert("local Date " + epochSeconds );
        tzOffset = local.getTimezoneOffset();
        //alert("TZ offset " + local.getTimezoneOffset() );
        localdatetime = (local.getUTCMonth()+1).padZero() + local.getUTCDate().padZero() + local.getUTCHours().padZero() + local.getUTCMinutes().padZero() +
                        local.getUTCFullYear() + "." + local.getUTCSeconds().padZero();
    }

    sendCgiRequest();

    passcount++;
};

function MyClick(event)
{
    var debug=0;
    var target = event.target || event.srcElement;
    var id = target.id;
    if(debug) alert("MyClick: id (" + id + ");" );

    gEvent = event;

    setVariable(id);
}

function AddRemoveShrinkList ( fieldName )
{
    var ShrinkCmd = "&shr=" + fieldName + "&";

    var result = ShrinkList.indexOf(ShrinkCmd);
    //alert("AddRemoveShrink " + fieldName + "; ShrinkCmd (" + ShrinkCmd + "); indexOf (" + result + ")" );
    // if the clicked rectangle is not already in the list, append it
    if (ShrinkList.indexOf(ShrinkCmd) == -1 ) {
        ShrinkList += ShrinkCmd;
        //alert("AddRemoveShrink: added (" + ShrinkCmd + "); ShrinkList now is (" + ShrinkList + ");" );
    } else { // remove this rectangle from the shrink list
        var temp = ShrinkList.replace(ShrinkCmd, "&"); // remove this rectangle from the shrink list
        ShrinkList = temp;

        if (ShrinkList == "&") { // if the list is now empty
            ShrinkList = "";
        }
        //alert("AddRemoveShrink: removed (" + ShrinkCmd + "); ShrinkList now is (" + ShrinkList + ");" );
    }

    ShrinkList = ShrinkList.replace(/&&/g,'&'); // adding multiple entries above results in double ampersands; remove the doubles
    //alert("ShrinkList now is (" + ShrinkList + ")" );

    GetClocks.Value = 1; // if something changed above, need to refresh the clock tree

    sendCgiRequest();
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

        if (obj.type == "checkbox" ) {
            fieldValue = obj.checked;
            if (debug) alert("setVariable: checkbox; value " + fieldValue );

            if (fieldName == "checkboxclocks" ) {
                // this section of code does not seem to work
                var objclocks = document.getElementById("CLOCKINFO");
                if (objclocks) {
                    objclocks.innerHTML = "";
                    if(debug) alert("clocks checked; cleared out html");
                }
                SetInternalCheckboxStatus ( "checkboxclocks", GetClocks );
                hideOrShow("row_clocks", fieldValue);
            }

            if (obj.checked) {
                //alert("calling sendCgiRequest");
                sendCgiRequest();
            }
        } else if ( obj.type == "text" ) {
            fieldValue = obj.value;
        } else if ( obj.type == "radio" ) {
        } else if ( obj.type == "select-one" ) {
        } else { // check to see if we can determine the item by its name
            var objrect = document.getElementById( fieldName );

            //alert("objrect ( " + objrect + ")" );
            if ( objrect ) {
                var points = objrect.getAttribute("points");
                //alert("points ( " + points + ")" );
                if ( points && points.length > 0) {
                    //alert("clicked rectangle (" + fieldName + ")" );
                    if (fieldName.indexOf("CLKGEN") == 0 ) { // if rectangle name starts with CLKGEN
                    } else {
                        AddRemoveShrinkList ( fieldName );
                    }
                }
            }
        }

        if (fieldName == "h1bpower") {
            MasterDebug = 1-MasterDebug;
            //alert("MasterDebug " + MasterDebug + "; debugobj " + debugobj );
            if (MasterDebug==1) {
                if (debugobj) {
                    debugobj.style.visibility = "";
                    GlobalDebug = 0;
                }
            } else {
                if (debugobj) {
                    debugobj.style.visibility = "hidden";
                    GlobalDebug = 0;
                }
                var CgiCountObj = document.getElementById('cgicount');
                if (CgiCountObj) {
                    CgiCountObj.innerHTML = "";
                }
            }
        } else if ( fieldName == "brcmlogo" ) {
            window.location.href = "index.html";
        }
    }
    //alert("setVariable: done");
}

function randomIntFromInterval(min,max)
{
    return Math.floor(Math.random()*(max-min+1)+min);
}

function setSvgHeight(newheight)
{
  var obj=document.getElementById('svg001');
  if (obj){
    //alert("setting svg height " + newheight + "; svg001 obj (" + obj + ")" );
    obj.setAttribute('height', newheight );
  }
}

function hideOrShow ( elementid, trueOrFalse )
{
    if (trueOrFalse) {
        //alert("hideOrShow: " + elementid + "; starting to SHOW");
        document.getElementById(elementid).style.display = '';
        //alert("hideOrShow: " + elementid + "; SHOW");
    } else {
        //alert("hideOrShow: " + elementid + "; starting to HIDE");
        document.getElementById(elementid).style.display = 'none';
        //alert("hideOrShow: " + elementid + "; HIDE");
    }
}

function SetInternalCheckboxStatus ( checkboxName, objStatus )
{
    var obj = document.getElementById( checkboxName );
    if (obj ) {
        objStatus.Value = obj.checked;
        //alert("SetInternalCheckboxStatus (" + checkboxName + ") to value (" + objStatus.Value + ")" );
    }
}

function MsIeCrFix ( newstring )
{
    /*
    if (userAgent.indexOf("MSIE") >= 0 ) { // for ie9, must replace carriage returns with <br>
        return newstring.replace(/\n/g, "<br>");
    }
/**/
    return newstring;
}

function AddToDebugOutput ( newstring )
{
    var debug = 1;
    var len   = newstring.length;

    if ( len > 1) {
        // if the lines ends with two carriage returns, replace two of them with just one
        if ( newstring.charCodeAt(len-1) == 10 && newstring.charCodeAt(len-2) == 10 ) {
            newstring = newstring.replace("\n\n", "\n");
        }
    }

    if (debug && debugobj) {
        debugobj.innerHTML += MsIeCrFix ( newstring );
    }
}

function sendCgiRequest( )
{
    var debug=0;

    var url = "";

    // if previous timeout is already pending, cancel it
    if (CgiTimeoutId) { clearTimeout(CgiTimeoutId); }

    var RandomValue = randomIntFromInterval(1000000,9999999);
    url = "/cgi/bpower.cgi?randomvalue=" + RandomValue;

    if (epochSeconds > 0) {
        url += "&datetime=" + epochSeconds + "&tzoffset=" + tzOffset;
        epochSeconds = 0;
    }

    if (GetClocks.Value == 1) {
        url += "&clocks=1";

        if (ShrinkList.length > 0) {
            url += ShrinkList + "&dummy=1"; // to distinguish between &shr=MOCA and &shr=MOCAWOL, add a dummy tag at the end
        }
        var temp = url.replace(/&&/g,'&'); // adding the dummy above results in double ampersands; remove the doubles
        url = temp;
    }

    if (GetShrinkList == 1) {
        url += "&getshrinklist=1";
        GetShrinkList = 0;
    }

    // seems the data sent to the browser cannot exceed 1010 for unknown reason
    if (url.length > 1010) {
        url = url.substr(0,1010);
    }

    urlSentPreviously = url;

    xmlhttp=new XMLHttpRequest();
    xmlhttp.onreadystatechange= serverHttpResponse;
    xmlhttp.open("GET",url,true);
    if (debug==1) alert("sending len (" + url.length + ") (" + url + ")" );
    xmlhttp.send(null);

    CgiCount++;
    var CgiCountObj = document.getElementById('cgicount');
    if ( MasterDebug && CgiCountObj) {
        CgiCountObj.innerHTML = "&nbsp;&nbsp;(" + CgiCount + "," + SetVariableCount + "," + ResponseCount + ")";
    }
}

function serverHttpResponse ()
{
    var debug=0;

    /* Because this function runs as an asynchronous response to a previous server request, the value of "this"
       is not the same as the constructor value of "this". For this reason, we use the global variable "gthis"
       to access the previous instantiation of "this"
    */
    if(debug) alert("serverHttpResponse: got readyState " + xmlhttp.readyState );

    if (xmlhttp.readyState==4 )
    {
        //alert("serverHttpResponse: got readyState " + xmlhttp.readyState + "; status " + xmlhttp.status );

        //var response = xmlhttp.getAllResponseHeaders();
        //var response = xmlhttp.getResponseHeader("Date");
        //alert("Date from header is " + response);

        // only if "OK"

        if (xmlhttp.status == 200) {
            var response_list = "";

            if(debugobj) debugobj.innerHTML = "";

            if(debug) alert("setting debugdiv");
            var safeHTML1 = xmlhttp.responseText.replace(/</g,'&lt;');
            var safeHTML2 = safeHTML1.replace(/>/g,'&gt;');
            //if(debugobj) debugobj.innerHTML = safeHTML2 // fails on ie7, safari
            if(debug) alert("innerHTML done");

            var responseText = rtrim(xmlhttp.responseText);
            var oResponses = responseText.split( "~" );
            if(debug) alert("num responses is " + oResponses.length );
debug=0;

            // loop through <response> elements, and add each nested
            for (var i = 0; i < oResponses.length; i++) {
                var entry = oResponses[i];
                if(debug && entry.length ) alert("Entry " + i + " of " + oResponses.length + ";" + entry );
                if (entry == "CLOCKS") {
                    AddToDebugOutput ( entry + ": len " + oResponses[i+1].length + ";" + EOL );
                    //alert(entry + ":" + oResponses[i+1] );

                    if (oResponses[i+1].length > 0) {
                        validResponseReceived = 1;
                        GetClocks.Value = 0;
                        var objclocks = document.getElementById("CLOCKINFO");
                        if (objclocks) {
                            objclocks.innerHTML = oResponses[i+1];
                        } else {
                            //alert("element CLOCKINFO not found");
                        }
                        setSvgHeight( SvgHeight );
                        hideOrShow("row_clocks", true );
                    } else {
                        if (objclockinfo) {
                            //objclockinfo.innerHTML = "Response was invalid; trying again in 1 second!";
                        }
                    }
                    i++;
                } else if (entry == "CLOCKSFAIL") {
                    var objcheckbox = document.getElementById("checkboxclocks");
                    if (objcheckbox) {
                        objcheckbox.checked = false;
                    }
                } else if (entry == "SHRINKLIST") {
                    ShrinkList = oResponses[i+1];
                    //alert("ShrinkList set to (" + ShrinkList + ")" );
                    i++;
                } else if (entry == "SVGHEIGHT") {
                    AddToDebugOutput ( entry + ": height " + oResponses[i+1] + ";" + EOL );
                    // Because of the way the svg html is created, the SVGHEIGHT command arrives before the actual SVG code.
                    // Save the height to a global variable, and set the height once the svg code arrives "CLOCKS"
                    SvgHeight = oResponses[i+1];
                    i++;
                } else if (entry == "ALERT") {
                    alert( oResponses[i+1] );
                    i++;
                } else if ( entry.length ) {
                    AddToDebugOutput ( entry + EOL );
                }
            }
        }  else {
            alert("There was a problem retrieving the XML data:\n" + xmlhttp.statusText);
        }

    } //if (xmlhttp.readyState==4 )
} // this.serverHttpResponse
