//*****************************************************************************
// Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
//
// This program is the proprietary software of Broadcom and/or its
// licensors, and may only be used, duplicated, modified or distributed pursuant
// to the terms and conditions of a separate, written license agreement executed
// between you and Broadcom (an "Authorized License").  Except as set forth in
// an Authorized License, Broadcom grants no license (express or implied), right
// to use, or waiver of any kind with respect to the Software, and Broadcom
// expressly reserves all rights in and to the Software and all intellectual
// property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
// HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
// NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
//
// Except as expressly set forth in the Authorized License,
//
// 1. This program, including its structure, sequence and organization,
//    constitutes the valuable trade secrets of Broadcom, and you shall use all
//    reasonable efforts to protect the confidentiality thereof, and to use
//    this information only in connection with your use of Broadcom integrated
//    circuit products.
//
// 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
//    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
//    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
//    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
//    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
//    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
//    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
//    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
//    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
//    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
//    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
//    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
//    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
//    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
//    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//******************************************************************************
<!--

var MasterDebug=0;
var MasterSending=0;
var CurrentPlatform = "";
var localdatetime = "";
var userAgent = 0;
var FirstPass = true;
var debugobj="";
var eol = "\n"; // end of line character for Firefox, Chrome, Safari (not for ie7)
var boxmodes=""; // array of known box modes (returned from CGI during init)
var boxmodesIdx=0; // used to asynchronously loop through the boxmodes array
var boxmodeResultsRecvd=0; // incremented by one each time of the boxmode diagrams is recieved
var sendBoxmode=false;
var percentComplete = "";
var boxmodeTableHtml= "";
var SUPPORTED_CODECS = ""; // we have to wait until the very last boxmode plot arrives before we can update this element

function trim(stringToTrim) { return stringToTrim.replace(/^\s+|\s+$/g,""); }
function ltrim(stringToTrim) { return stringToTrim.replace(/^\s+/,""); }
function rtrim(stringToTrim) { return stringToTrim.replace(/\s+$/,""); }

Number.prototype.padZero= function(len){
    var s= String(this), c= '0';
    len= len || 2;
    while (s.length < len) s= c + s;
    return s;
}

function MyLoad()
{
    //alert("MyLoad");
    debugobj=document.getElementById("debugoutputbox");
    var local = new Date();
    localdatetime = (local.getUTCMonth()+1).padZero() + local.getUTCDate().padZero() + local.getUTCHours().padZero() + local.getUTCMinutes().padZero() + local.getUTCFullYear() + "." + local.getUTCSeconds().padZero();
    userAgent = navigator.userAgent;
    sendCgiRequest();
}

function MyMouseOver(event, decodeID, rectangleID )
{
    var target = event.target || event.srcElement; // needed for IE8
    var id=target.id;
    //alert("MyMouseOver: " + id + "; decodeID:" + decodeID + "; rectangleID:" + rectangleID );
    var boxmodetest = document.getElementById( decodeID );
    if (boxmodetest) {
        var rectangleobj = document.getElementById( rectangleID );
        {
            //alert("MyMouseOver: " + id + "; decodeID:" + decodeID + "; rect:" + rectangleID + "; left:" + rectangleobj.style.left + "; top:" + rectangleobj.style.top + "; X:" + event.clientX + "; Y:" + event.clientY );
            boxmodetest.style.position = "fixed";
            boxmodetest.style.left = event.clientX /*+ rectangleobj.style.left*/ + 20;
            boxmodetest.style.top = event.clientY /* rectangleobj.style.top*/;
            boxmodetest.style.display = "";
            //var testobj = document.getElementById('boxmodetest1');
            //if (testobj) testobj.innerHTML =  "(coords:" + event.clientX + "," + event.clientY + ")";
        }
    }
}

function MyMouseOut(event, decodeID, rectangleID )
{
    var target = event.target || event.srcElement; // needed for IE8
    var id=target.id;
    //alert("MyMouseOut: " + id + "; decodeID:" + decodeID );
    var boxmodetest = document.getElementById( decodeID );
    if (boxmodetest) {
        boxmodetest.style.display = "none";
    }
}

function MyClick(event)
{
    var target = event.target || event.srcElement; // needed for IE8
    var id=target.id;
    //alert("MyClick: " + id + "; MasterDebug:" + MasterDebug + "; debugobj:" + debugobj );
    if (id == "h2broadcom") {
        MasterDebug = 1-MasterDebug;
        if (MasterDebug==1) {
            if (debugobj) { debugobj.style.visibility = ""; }
        } else {
            if (debugobj) { debugobj.style.visibility = "hidden"; }
        }
    } else if ( id == "brcmlogo" ) {
        window.location.href = "index.html";
    }
}

function setSvgHeight ( svgid, svgheight )
{
    //alert("setSvgHeight top");
    var svgobj = document.getElementById(svgid);
    if ( svgobj ) {
        svgobj.style.height=svgheight;
        //alert("setSvgHeight:" + svgid + "; hgt:" + svgheight );
    } else {
        alert("setSvgHeight:" + svgid + " not found");
    }
}

function randomFromTo(from, to)
{
   //alert("random:" + from + "; to " + to );
   return Math.floor(Math.random() * (to - from + 1) + from);
   //return "123456";
}

function UpdateProgressBar ( percentage )
{
    var debug=0;
    var obj = document.getElementById('BOXMODETABLE');
    if(obj)
    {
        // update progress bar
        obj.innerHTML = "<h2>Completion: " + percentage + "%</h2>";
        if(debug==1) alert("ProgressBar to (" + percentage + ")" );
    }
    else
    {
        if(debug==1) alert("BOXMODETABLE element not found");
    }

    return 0;
}

function last_boxmode_received()
{
    //alert("last boxmode row recvd:" + boxmodeTableHtml + eol );
    var boxmodeTableObj = document.getElementById('BOXMODETABLE');
    if(boxmodeTableObj) {
        boxmodeTableObj.innerHTML = boxmodeTableHtml;
    } else {
        alert("BOXMODETABLE element not found");
    }

    // after the last boxmode plot arrives, we also have to update the SUPPORTED_CODECS element
    var codecsObj = document.getElementById('SUPPORTED_CODECS');
    if( codecsObj ) {
        codecsObj.innerHTML = SUPPORTED_CODECS;
    } else {
        alert("SUPPORTED_CODECS element not found");
    }
}

function sendCgiRequest ()
{
    var doit = false;
    var url = "/bboxreport.cgi?randvalue=" + randomFromTo(100000,999999);
    xmlhttp=new XMLHttpRequest();

    //alert("sendCgiRequest: First:" + FirstPass + "; sendBox:" + sendBoxmode + "; boxIdx:" + boxmodesIdx);

    if ( FirstPass ) {
        url += "&init=1";
        FirstPass = false;
        doit = true;
    } else if ( sendBoxmode ) {
        if (boxmodesIdx == 0) {
            url += "&boxmodefirst=1";// CGI needs to know when it is receiving the very first box mode
        }
        url += "&boxmode=" + boxmodes[boxmodesIdx];
        boxmodesIdx++;
        doit = true;
    }

    sendBoxmode=false;

    if ( doit ) {
        //alert("sending " + url );
        xmlhttp.onreadystatechange= serverHttpResponse;
        xmlhttp.open("GET",url,true);
        xmlhttp.send(null);
    }
}

function serverHttpResponse()
{
    var debug=0;

    if(debug) alert("serverHttpResponse: got readyState " + xmlhttp.readyState );

    if (xmlhttp.readyState==4 )
    {
        if(debug) alert("serverHttpResponse: got readyState " + xmlhttp.readyState + "; status " + xmlhttp.status );

        //var response = xmlhttp.getAllResponseHeaders();
        //var response = xmlhttp.getResponseHeader("Date");
        //alert("Date from header is " + response);

        // only if "OK"

        if (xmlhttp.status == 200)
        {
            debug = 0;
            var response_list = "";
            var debugobj=document.getElementById("debugoutputbox");

            if (userAgent.indexOf("MSIE") >= 0 ) { // for ie9, must replace carriage returns with <br>
                //eol = "<br>";
                eol = "\n";
                if(debug) alert("MSIE detected. setting eol to (" + eol + ")" );
            }

            //alert("setting debugdiv");
            //document.getElementById("debugdiv").innerHTML = xmlhttp.responseText; // works firefox, ie7, safari
            //if(debug) alert("setting debugoutputbox");
            if(debugobj) {
                // if response has html tags in it <input ...>, you cannot display the response in a textarea box
                // to resolve this IE feature, globally replace the < and > chars with &lt; and &gt;
                var newtext = xmlhttp.responseText.replace(/</g,"&lt;");
                //debugobj.innerHTML = newtext.replace(/>/g,"&gt;");
                debugobj.scrollTop = debugobj.scrollHeight;
            }
            if(debug) alert("innerHTML done ... len:" + xmlhttp.responseText.length + " ... (" + newtext.replace(/>/g,"&gt;") + ")" );

            //var oResponses = xmlhttp.responseXML.getElementsByTagName("response")[0];
            //alert("var oResponses calling");
            //var oResponses = xmlhttp.responseXML.getElementsByTagName("response");
            var responseText = rtrim(xmlhttp.responseText);
            var oResponses = responseText.split( "~" );
            if(debug) alert("for i = 0 to " + oResponses.length );

            // loop through <response> elements, and add each nested
            for (var i = 0; i < oResponses.length; i++)
            {
                var entry = oResponses[i];

                // MAJOR ERROR: entire response is empty ... CGI app may have crashed before sending anything back
                if ( oResponses.length == 1 &&  xmlhttp.responseText.length == 0 ) {
                    if (debugobj) {debugobj.innerHTML += entry + "; " + oResponses[i+1] + eol; }
                    var resultsObj = document.getElementById('PAGECONTENTS');
                    if( resultsObj ) {
                        resultsObj.innerHTML += "<h3>Boxmode " + boxmodes[boxmodesIdx-1] + " may have crashed; received empty response from device.</h3>";
                    }
                    boxmodeResultsRecvd++;
                    percentComplete = Math.round ( (boxmodeResultsRecvd / boxmodes.length) * 100 );
                    UpdateProgressBar ( percentComplete );
                    //alert("percentComplete:" + percentComplete );
                    boxmodeTableHtml += "<tr><td class=border align=center >" + boxmodes[boxmodesIdx-1] + "</td><td class=border align=left colspan=9 >Bad response; CGI may have crashed.</td></tr>";
                    if ( boxmodeResultsRecvd == boxmodes.length ) {
                        last_boxmode_received();
                    } else {
                        sendBoxmode = true; // still more box modes to receive
                    }
                }

                if (entry.length == 0 ) continue; // if entry is empty, go on to the next one

                if (entry == "STBTIME") {
                    if(debugobj) debugobj.innerHTML += entry + ":" + oResponses[i+1] + eol;
                    var objtime=document.getElementById("stbtime");
                    if (objtime) { objtime.innerHTML = oResponses[i+1] }
                    i++;
                } else if (entry == "FATAL") {
                    if (debugobj) {debugobj.innerHTML += entry + "; " + oResponses[i+1] + eol; }
                    var resultsObj = document.getElementById('PAGECONTENTS');
                    if( resultsObj ) {
                        resultsObj.innerHTML += "<h3>" + oResponses[i+1] + "</h3>";
                    }
                    alert("FATAL ... " + oResponses[i+1]);
                    i++;
                } else if (entry == "FATALNOALERT") { // same as FATAL but without the alert
                    if (debugobj) {debugobj.innerHTML += entry + "; " + oResponses[i+1] + eol; }
                    var resultsObj = document.getElementById('PAGECONTENTS');
                    if( resultsObj ) {
                        resultsObj.innerHTML += "<h3>" + oResponses[i+1] + "</h3>";
                    }
                    boxmodeResultsRecvd++;
                    percentComplete = Math.round ( (boxmodeResultsRecvd / boxmodes.length) * 100 );
                    UpdateProgressBar ( percentComplete );
                    //alert("percentComplete:" + percentComplete );
                    boxmodeTableHtml += "<tr><td class=border align=center >" + boxmodes[boxmodesIdx-1] + "</td><td class=border align=left colspan=9 >Fatal error returned from device.</td></tr>";
                    i++;
                } else if (entry == "ALERT") {
                    if (debugobj) {debugobj.innerHTML += entry + eol; }
                    alert("ERROR ... " + oResponses[i+1]);
                    i++;
                } else if ( entry == "DEBUG" ) {
                    if(debugobj) debugobj.innerHTML += oResponses[i+1] + eol;
                    if(debug) alert(entry + ":" + oResponses[i+1]);
                    i++;
                } else if ( entry == "boxmodes" ) {
                    if(debugobj) debugobj.innerHTML += entry + ":" + oResponses[i+1] + eol;
                    if(debug) alert(entry + ":" + oResponses[i+1]);
                    boxmodes = oResponses[i+1].split("|");
                    sendBoxmode = true;
                    i++;
                } else if ( entry == "PAGECONTENTS" ) {
                    //if(debugobj) debugobj.innerHTML += entry + " START###" + oResponses[i+1] + "### END " + entry + eol;
                    if(debug==1) alert(entry + ":" + oResponses[i+1]);
                    var resultsObj = document.getElementById('PAGECONTENTS');
                    if( resultsObj ) {
                        //alert("PAGECONTENTS (" + oResponses[i+1] + ")" );
                        resultsObj.innerHTML += oResponses[i+1];
                    }
                    boxmodeResultsRecvd++;
                    percentComplete = Math.round ( (boxmodeResultsRecvd / boxmodes.length) * 100 );
                    UpdateProgressBar ( percentComplete );
                    //alert("percentComplete:" + percentComplete );

                    i++;
                } else if ( entry == "BOXMODETABLE" ) {
                    //if(debugobj) debugobj.innerHTML += entry + ":" + oResponses[i+1] + eol;
                    if(debug) alert(entry + ":" + oResponses[i+1]);

                    boxmodeTableHtml += oResponses[i+1];
                    // when the last boxmode is received, update the boxmode table
                    //alert("boxmodeResultsRecvd " + boxmodeResultsRecvd + "; boxmodes.length " + boxmodes.length );
                    if ( boxmodeResultsRecvd == boxmodes.length ) {
                        last_boxmode_received();
                    } else {
                        sendBoxmode = true; // still more box modes to receive
                    }

                    i++;
                } else if ( entry == "SUPPORTED_CODECS" ) {
                    // if one of the boxmodes provides a longer list of codecs, it becomes the new superset
                    if (oResponses[i+1].length > SUPPORTED_CODECS.length ) {
                        //alert("new CODEC string: old (" + SUPPORTED_CODECS + "); new (" + oResponses[i+1] + ")" );
                        SUPPORTED_CODECS = oResponses[i+1];
                    }
                    i++;
                } else if ( entry == "SVGHEIGHT" ) {
                    if(debugobj) debugobj.innerHTML += entry + ":" + oResponses[i+1] + eol;
                    if(debug) alert(entry + ":" + oResponses[i+1]);
                    svgheight = oResponses[i+1].split("|");
                    setSvgHeight ( svgheight[0], svgheight[1] );
                    i++;
                } else if ( entry == "SVGSPACER" ) {
                    if(debugobj) debugobj.innerHTML += entry + ":" + oResponses[i+1] + eol;
                    svgspacer = oResponses[i+1].split("|");
                    var objspacer = document.getElementById( svgspacer[0] );
                    if ( objspacer ) {
                        objspacer.style.height = svgspacer[1];
                        //alert(entry + ": " + svgspacer[0] + ".height set to " + svgspacer[1] );
                    } else {
                        alert("element " + svgspacer[0] + " not found. could not set height to " + svgspacer[1] );
                    }
                    i++;
                } else if ( entry == "ALLDONE" ) {
                    if(debugobj) debugobj.innerHTML += entry + ":" + oResponses[i+1] + eol + eol + eol;
                    if(debug) alert(entry);
                    i++;
                } else if (entry == "OPENAPPS") {
                    if (debugobj) {debugobj.innerHTML += entry + eol; }
                    var boxmodeTableObj = document.getElementById('BOXMODETABLE');
                    if(boxmodeTableObj) {
                        boxmodeTableObj.innerHTML = oResponses[i+1];
                    }
                    i++;
                } else {
                    if (debugobj && entry.length >0 && entry.length < 200) { debugobj.innerHTML += entry + eol; }
                }
                //alert("bottom of loop: Entry" + entry );
            }

            if(debug) alert("Response: for loop done; sendBoxmode:" + sendBoxmode + "; boxIdx:" + boxmodesIdx + "; boxSize:" + boxmodes.length );

            if ( sendBoxmode && boxmodesIdx < boxmodes.length ) {
               sendCgiRequest();
            }
        }

        else
        {
            alert("There was a problem retrieving the XML data: " + xmlhttp.statusText + eol );
        }

    } //if (xmlhttp.readyState==4 )
} // serverHttpResponse

function displayAnswer(id)
{
    var obj = document.getElementById(id);
    if(obj.style.display == 'none') {
        obj.style.display = '';
    } else {
        obj.style.display = 'none';
    }
}
 -->
