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
var MasterDebug=0;
var GlobalDebug = 0;
var REFRESH_IN_MILLISECONDS=1000;
var SetVariableCount=0; // number of times setVariable() is called
var gFieldName = "";
var CgiTimeoutId=0;
var ConsoleTimeoutId=0;
var ConsoleCountdown=0;
var CgiCount=0; // number of times the cgi was called
var ResponseCount=0; // number of times serverHttpResponse() is called
var urlSentPreviously = "";
var debugobj=0;
var userAgent = 0;
var EOL = "\n"; // end of line character for Firefox, Chrome, Safari (not for ie7)
var epochSeconds = 0;
var tzOffset = 0;
var localdatetime = "";
var validResponseReceived = 0;
var passcount=0;
var GetConsole = {Value: 0}; // 0 => UNINIT ... 1 => INIT ... 2 => GET_STATUS
var UUID = ""; // used to allow multiple browsers to access the STB
var HIDE = false;
var SHOW = true;
var ttyCommand = ""; // the bash command to send to the STB
var PrevKeyDown = 0;
var CTRL = 17;
var SpecialChar = ""; // used to send ctrl-c to stb

function rtrim(stringToTrim) { return stringToTrim.replace(/\s+$/,""); }

Number.prototype.padZero= function(len){
    var s= String(this), c= '0';
    len= len || 2;
    while (s.length < len) s= c + s;
    return s;
}

window.onunload = function () {
    //console.log("unload function");
}

function MyBeforeUnload ()
{
    //console.log("MyBeforeUnload");
    if ( GetConsole.Value == 2) /* if telnet session is still in progress */ {
        GetConsole.Value = 3; // inform CGI we are wanting to EXIT
        ttyCommand = "exit";
        sendCgiRequest();
    }

    return true;
}

function MyLoad ()
{
    var debug=0;
    if(debug) alert("MyLoad");
    debugobj=document.getElementById("debugoutputbox");
    userAgent = navigator.userAgent;
    if (userAgent.indexOf("MSIE") >= 0 ) { // for ie9, must replace carriage returns with <br>
        EOL = "\r\n";
    }
    if ( passcount == 0 ) {
        UUID = uuid();

        var local = new Date();
        epochSeconds = Math.floor(local.getTime() / 1000);
        //alert("local Date " + epochSeconds );
        tzOffset = local.getTimezoneOffset();
        //alert("TZ offset " + local.getTimezoneOffset() );
        localdatetime = (local.getUTCMonth()+1).padZero() + local.getUTCDate().padZero() + local.getUTCHours().padZero() + local.getUTCMinutes().padZero() +
                        local.getUTCFullYear() + "." + local.getUTCSeconds().padZero();
        var checkboxconsole = document.getElementById('checkboxconsole');
        if ( checkboxconsole && checkboxconsole.checked == false) {
            hideOrShow ( 'row_console', HIDE );
        }

        setTimeout ("ProcessCheckboxConsoleSelected(true)", REFRESH_IN_MILLISECONDS/5 );
    }

    sendCgiRequest();

    passcount++;
};

function OneSecond ()
{
    if (CgiTimeoutId) { clearTimeout(CgiTimeoutId); CgiTimeoutId = 0; /*console.log("OneSecond - clearTimeout 1"); */ }

    if ( GetConsole.Value != 3 /* EXIT */ ) {
        //console.log("OneSecond - setTimeout to run OneSecond() API");
        CgiTimeoutId = setTimeout ('OneSecond()', REFRESH_IN_MILLISECONDS );

        sendCgiRequest();
    }
}

function uuid()
{
  function s4() {
    return Math.floor((1 + Math.random()) * 0x10000)
      .toString(16)
      .substring(1);
  }
  //return s4() + s4() + '-' + s4() + '-' + s4() + '-' + s4() + '-' + s4() + s4() + s4();
  return s4() + s4() + s4() + s4() + s4() + s4() + s4() + s4();
}

function MyClick(event)
{
    var debug=0;
    var target = event.target || event.srcElement;
    var id = target.id;
    //console.log("MyClick: id (" + id + ");" );

    gEvent = event;

    setVariable(id);
}

/* In order to detect a CTRL-C key press, we need to keep track of when the CTRL key goes down and when it goes up */
/* Pressing and releasing the CTRL key and then pressing and releasing the 'c' key is not the same thing as
   pressing the CTRL key and then pressing the 'c' key and then releasing both of them at the same time. */
function MyKeyUp(event)
{
    var debug=0;
    var target = event.target || event.srcElement;
    var id = target.id;
    var contents = "";
    //console.log ("MyKeyUp: id (" + id + "); keyCode=" + event.keyCode + "; contents.length:" + contents.length );

    PrevKeyDown = 0;
}

function MyKeyDown(event)
{
    var debug=0;
    var target = event.target || event.srcElement;
    var id = target.id;
    var contents = "";
    var obj=document.getElementById( "input_ttyConsole" );
    if (obj) {
        contents = obj.value;
    }

    //console.log ("MyKeyDown: id (" + id + "); keyCode=" + event.keyCode + "; contents.length:" + contents.length + "; PrevKey " + PrevKeyDown );
    if ( PrevKeyDown == CTRL && event.keyCode == 67 /* c character */ ) {
        SpecialChar = "ctrl-c";
        //console.log("MyKeyDown: " + SpecialChar + " detected" );
    } else if ( PrevKeyDown == CTRL && event.keyCode == 68 /* d character */ ) {
        //SpecialChar = "ctrl-d";
        //console.log("MyKeyDown: " + SpecialChar + " detected" );
    } else if ( PrevKeyDown == CTRL && event.keyCode == 90 /* z character */ ) {
        //SpecialChar = "ctrl-z";
        //console.log("MyKeyDown: " + SpecialChar + " detected" );
    }
    PrevKeyDown = event.keyCode;

    /* Special Case: if user enters return into an empty input element, we need to still something to telnet */
    if ( event.keyCode === 13 && contents.length == 0 && obj ) {
        //console.log ("MyKeyDown: id (" + id + "); keyCode=" + event.keyCode + "; setting obj.value to newline" );
        ttyCommand = ".";
        sendCgiRequest();
    } else if ( event.keyCode === 13 && contents.length > 0 && obj && userAgent.indexOf("MSIE") >= 0 ) {
        ttyCommand = contents;
        obj.value = ""; // clear out the entry box
        checkForExit( contents );
        sendCgiRequest();
    } else if ( SpecialChar.length ) {
        obj.value = ""; // clear out the entry box
        sendCgiRequest();
    }
}

// When the user enters "exit" on the console, this is a special command to terminate the connection
function checkForExit( fieldValue )
{
    if ( fieldValue == "exit" ) {
        GetConsole.Value = 3; // EXIT
        var objcheckbox=document.getElementById( "checkboxconsole" );
        if ( objcheckbox ) {
            //hideOrShow("row_console", HIDE );
            objcheckbox.checked = false;
        }
    }
}

function ProcessCheckboxConsoleSelected( fieldValue )
{
    //console.log( "ProcessCheckboxConsoleSelected: value (" + fieldValue + ")");

    if ( GetConsole.Value == 0 && fieldValue == true ) GetConsole.Value = 1 /* INIT ... start the telnet threads */;

    hideOrShow("row_console", fieldValue);
    if (fieldValue) {
        ClearInnerHtml ( "text_ttyConsole" );
        AppendTextArea ( "text_ttyConsole", "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" );
        var objinput = document.getElementById('input_ttyConsole');
        if (objinput) {
            objinput.focus();
        }

        sendCgiRequest();
    }
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

            if (fieldName == "checkboxconsole" ) {
                ProcessCheckboxConsoleSelected( fieldValue );
            }

            if (obj.checked) {
                GetConsole.Value = 1;
                //alert("calling sendCgiRequest");
                sendCgiRequest();
            } else {
                GetConsole.Value = 0;
            }
        } else if ( obj.type == "text" ) {
            fieldValue = obj.value;
            if (fieldName == "input_ttyConsole" ) {
                if (debug==1) alert("setVariable: input; value " + fieldValue + " ; id:" + fieldName );
                var obj=document.getElementById( fieldName );
                if ( obj ) {
                    obj.value = "";
                    ttyCommand = fieldValue;
                    //console.log ( 'text_ttyConsole', fieldName + ":" + fieldValue + "; len:" + fieldValue.length + EOL );

                    checkForExit( fieldValue );

                    sendCgiRequest();
                }
            }
        } else if ( obj.type == "radio" ) {
        } else if ( obj.type == "select-one" ) {
        } else { // check to see if we can determine the item by its name
            var objrect = document.getElementById( fieldName );

            //alert("objrect ( " + objrect + ")" );
            if ( objrect ) {
            }
        }

        if (fieldName == "h1bconsole") {
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
    if (userAgent.indexOf("MSIE") >= 0 ) { // for ie9, must replace newlines with carriage-return/newline combination
        var newstring2 = newstring.replace(/\n/g, "\r\n");
        var newstring3 = newstring2.replace(/\r\r\n/g, "\r\n");
        return newstring3;
    }
    return newstring;
}

function AddToDebugOutput ( newstring )
{
    var len = newstring.length;

    if ( len > 1) {
        // if the lines ends with two carriage returns, replace two of them with just one
        if ( newstring.charCodeAt(len-1) == 10 && newstring.charCodeAt(len-2) == 10 ) {
            newstring = newstring.replace("\n\n", "\n");
        }
    }

    AppendTextArea ( "debugoutputbox", newstring );
}

function ClearInnerHtml ( elementid )
{
    var objtxt = document.getElementById( elementid );
    if ( objtxt ) {
        objtxt.innerHTML = "";
    }

    return true;
}

function AppendTextArea ( elementid, newvalue )
{
    var objtxt = document.getElementById( elementid );
    if ( objtxt ) {
        if (userAgent.indexOf("MSIE") >= 0 ) {
            var contents = MsIeCrFix ( newvalue );
            objtxt.value += contents;
        } else {
            objtxt.innerHTML += newvalue;
        }
        objtxt.scrollTop = objtxt.scrollHeight;
    }

    return true;
}

function sendCgiRequest( )
{
    var debug=0;

    var url = "";

    // if previous timeout is already pending, cancel it
    //if (CgiTimeoutId) { clearTimeout(CgiTimeoutId); CgiTimeoutId = 0; console.log("clearTimeout 2"); }

    var RandomValue = randomIntFromInterval(1000000,9999999);
    url = "/cgi/bconsole.cgi?randomvalue=" + RandomValue;

    if (epochSeconds > 0) {
        url += "&datetime=" + epochSeconds + "&tzoffset=" + tzOffset;
        epochSeconds = 0;
    }

    url += "&uuid=" + UUID;

    if (GetConsole.Value == 3 /* EXIT */ ) {
        url += "&ttyConsole=3";
    }

    if (GetConsole.Value == 2 /* GET_STATUS */ ) {
        url += "&ttyConsole=2";
    }

    if (GetConsole.Value == 1 /* INIT */ ) {
        url += "&ttyConsole=1";
        GetConsole.Value = 2 /* GET_STATUS */;
        CgiTimeoutId = setTimeout ('OneSecond()', REFRESH_IN_MILLISECONDS ); // look periodically for output from drivers and background processes
    }

    if ( ttyCommand.length ) {
        url += "&ttyCommand=" + ttyCommand;
        ttyCommand = "";
        var local = new Date();
        var epoch = Math.floor(local.getTime() );
        //console.log(epoch + ": waiting a second to run OneSecond() API");
        CgiTimeoutId = setTimeout ('OneSecond()', REFRESH_IN_MILLISECONDS ); // look periodically for output from drivers and background processes
    }

    if ( SpecialChar.length ) {
        url += "&SpecialChar=" + SpecialChar;
        SpecialChar = "";
    }

    // seems the data sent to the browser cannot exceed 1010 for unknown reason
    if (url.length > 1010) {
        url = url.substr(0,1010);
    }

    urlSentPreviously = url;

    var local = new Date();
    var epoch = Math.floor(local.getTime() );

    xmlhttp=new XMLHttpRequest();
    xmlhttp.onreadystatechange= serverHttpResponse;
    xmlhttp.open("GET",url,true);
    if (debug==1) alert("sending len (" + url.length + ") (" + url + ")" );
    //console.log( epoch + ":sending len (" + url.length + ") (" + url + ")" );
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
        if (xmlhttp.status == 200) {  // only if "OK"
debug=0;
            var response_list = "";

            //if(debugobj) debugobj.innerHTML = ""; // CAD

            if(debug) alert("setting debugdiv");
            var safeHTML1 = xmlhttp.responseText.replace(/</g,'&lt;');
            var safeHTML2 = safeHTML1.replace(/>/g,'&gt;');
            //if(debugobj) debugobj.innerHTML = safeHTML2 // fails on ie7, safari
            if(debug) alert("innerHTML done");

            var responseText = rtrim(xmlhttp.responseText);
            var oResponses = responseText.split( "~" );
            if(debug) alert("num responses is " + oResponses.length );

            var local = new Date();
            var epoch = Math.floor(local.getTime() );
            //console.log(epoch + ": got response");
            // loop through <response> elements, and add each nested
            for (var i = 0; i < oResponses.length; i++) {
                var entry = oResponses[i];
                if(debug && entry.length ) alert("Entry " + i + " of " + oResponses.length + "; entry (" + entry + ")" );
                if (entry == "ttyConsole") {
                    AddToDebugOutput ( entry + ": len " + oResponses[i+1].length + ";" + EOL );
                    if(debug) alert(entry + ":" + oResponses[i+1] );

                    if (oResponses[i+1].length > 0) {
                    }
                    i++;
                } else if (entry == "ALERT") {
                    alert( oResponses[i+1] );
                    i++;
                } else if (entry == "\n") {
                } else if ( entry == "DEBUG" ) {
                    if(debug==0) AddToDebugOutput ( entry + ":" + oResponses[i+1] + EOL );
                    //if(debug) alert(entry + ":" + oResponses[i+1]);
                    i++;
                } else if (entry == "ttyContents") {
                    //AddToDebugOutput ( entry + ":" + oResponses[i+1] + EOL );

                    // for some reason, the results come back with double prompts in them
                    var prompt_idx = oResponses[i+1].indexOf("# ");0
                    if ( prompt_idx > 0 ) {
                        var contents = oResponses[i+1].replace(/# \r\n# /g, "# " );
                        //console.log( contents.charCodeAt(prompt_idx).toString(16) + " " + contents.charCodeAt(prompt_idx+1).toString(16) + " " +
                        //contents.charCodeAt(prompt_idx+2).toString(16) + " " + contents.charCodeAt(prompt_idx+3).toString(16) + " " +
                        //contents.charCodeAt(prompt_idx+4).toString(16) + " " + contents.charCodeAt(prompt_idx+5).toString(16) );
                    } else {
                        var contents = oResponses[i+1];
                    }
                    AppendTextArea ( 'text_ttyConsole', contents );
                    i++;
                } else if ( entry == "ALLDONE" ) {
                    if(debug) AddToDebugOutput ( entry + EOL );
                } else if (entry == "PLATFORM") {
                    var objplatform = document.getElementById("platform");
                    if (objplatform) {
                        objplatform.innerHTML = oResponses[i+1]; CurrentPlatform = oResponses[i+1];
                    }
                    i++;
                } else if (entry == "PLATVER") {
                    var objplatform = document.getElementById("platver");
                    if (objplatform) {
                        objplatform.innerHTML = oResponses[i+1]
                    }
                    window.document.title = CurrentPlatform + " " + oResponses[i+1];
                    i++;
                } else if (entry == "UNAME") {
                    //alert(entry + ":" + oResponses[i+1] );
                    var objplatform = document.getElementById("platver");
                    if (objplatform) {
                        objplatform.innerHTML += "&nbsp;" + oResponses[i+1];
                    }
                    i++;
                } else if (entry == "STBTIME") {
                    //AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
                    var obj2=document.getElementById("stbtime");
                    if (obj2) {
                        //if (debug) alert("setting stbtime to " + oResponses[i+1] );
                        obj2.innerHTML = oResponses[i+1];
                        i++;
                    } else {
                        alert("id=stbtime not found");
                    }
                } else if ( entry.length ) {
                    AddToDebugOutput ( entry + EOL );
                }
            }
        }  else {
            //alert("There was a problem retrieving the XML data:\n" + xmlhttp.statusText);
        }

    } //if (xmlhttp.readyState==4 )
} // this.serverHttpResponse
