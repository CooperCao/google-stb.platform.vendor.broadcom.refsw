var BoxMode = -1;
var BoxModePrev = 0;
var MasterDebug=0;
var REFRESH_IN_MILLISECONDS=1000;
var ClientDetails = "";
var OneSecondCount=0;
var CgiCount=0; // number of times the cgi was called
var CgiTimeoutId=0;
var SetVariableCount=0; // number of times setVariable() is called
var ResponseCount=0; // number of times serverHttpResponse() is called
var objdebug = document.getElementById("debugoutputbox");
var TextBoxUpdateInProgress=0;
var localdatetime = "";
var Action = "init";
var StartStopButtonState = 0;
var PauseResumeButtonState = 0;
var sCaptureFilename = "";
var top10_value = 256;
var NumberOfMemc = 0;
var ClientCount = [0,0,0];
var RecordNumber = 0;

function rtrim(stringToTrim) { return stringToTrim.replace(/\s+$/,"");}

Number.prototype.padZero= function(len){ // needed by localdatetime
    var s= String(this), c= '0';
    len= len || 2;
    while (s.length < len) s= c + s;
    return s;
}

function countClientDetailActive( memc )
{
    var client;
    var obj;
    var client_id="";
    var client_count = 0;

    for (client=0; client<top10_value; client++) {
        client_id = "client" + client + "memc" + memc;
        obj = document.getElementById(client_id);
        if (obj && obj.checked) {
            client_count++;
        }
    }

    return client_count;
}

function collectClientDetailRequests()
{
    var memc;
    var client;
    var obj;
    var client_id="";

    for (memc=0; memc<NumberOfMemc; memc++) {
        for (client=0; client<top10_value; client++) {
            client_id = "client" + client + "memc" + memc;
            obj = document.getElementById(client_id);
            if (obj && obj.checked) {
                ClientDetails += "&variable=" + client_id + ":" + StartStopButtonState;
            }
        }
    }

    //alert("NumMemc " + NumberOfMemc + "; ClientDetails (" + ClientDetails + ")" );
}

function DisableObject ( objectname )
{
    var obj = document.getElementById(objectname);
    if (obj) {
        //alert("obj " + obj + "; name " + objectname + " disabled");
        obj.disabled = "disabled";
    }
}

function EnableObject ( objectname )
{
    var obj = document.getElementById(objectname);
    if (obj) {
        //alert(objectname + " enabled");
        obj.disabled = "";
    }
}

function MyClick(e)
{
    var debug=0;
    if(debug) alert("MyClick");
    var elem, evt = e ? e:event;
    if (evt.srcElement) {
        elem = evt.srcElement;
    } else if (evt.target) {
        elem = evt.target;
    }
    e = e || window.event; // IE9
    var target = (e.target) ? e.target : e.srcElement; // IE9
    var fieldName = target.id;
    if(debug) alert("MyClick fieldName " + fieldName );

    var obj = document.getElementById(target.id);
    if (debug) alert ('<' + elem.tagName + '>; ID ' + target.id + "; type " + obj.type );

    if (target.id == "startstopbutton") {
        if (CgiTimeoutId) {
            clearTimeout(CgiTimeoutId);
        }

        if (StartStopButtonState == 0 ) { // start capture is displayed on the button
            if (obj) {
                var local = new Date();
                sCaptureFilename = "bmemperf_capture_" + local.getFullYear() + (local.getMonth()+1).padZero() + local.getDate().padZero() + "_" +
                                   local.getHours().padZero() + local.getMinutes().padZero() + local.getSeconds().padZero() + ".dat";
                obj.style.backgroundColor = "salmon";
                obj.value = "Stop Capture";
                StartStopButtonState = 1;
                PauseResumeButtonState = 0
                Action = "start";
                RecordNumber = 0;
                var sizeobj=document.getElementById("capturesize");
                if (sizeobj) {
                    sizeobj.innerHTML =  "";
                }
                var descobj=document.getElementById("capturedesciptor");
                if (descobj) {
                    descobj.innerHTML = "";
                }
                DisableObject("fetchsubmit");
                EnableObject("pauseresumebutton");
                DisableObject("boxmode");

                // loop through all memcs and all client names and gather those that are checked on
                collectClientDetailRequests();

                sendCgiRequest();
            }
        } else {
            if (obj) {
                obj.style.backgroundColor = "lightgreen";
                obj.value = "Start Capture";
                StartStopButtonState = 0;
                PauseResumeButtonState = 0
                Action = "stop";

                DisableObject("pauseresumebutton");

                // loop through all memcs and all client names and gather those that are checked on
                collectClientDetailRequests();

                sendCgiRequest();
            }
        }
    } else if (target.id == "pauseresumebutton") {
        // user just hit PAUSE
        if (PauseResumeButtonState == 0) {
            //alert("PAUSING");
            obj.value = "Resume";
            if (CgiTimeoutId) {
                clearTimeout(CgiTimeoutId);
            }
            PauseResumeButtonState = 1;
        } else { // user just hit resume
            PauseResumeButtonState = 0;
            //alert("RESUMING");
            obj.value = "Pause";
            CgiTimeoutId = setTimeout ('sendCgiRequest()', REFRESH_IN_MILLISECONDS );
        }
    } else if (target.id == "h1bmemperf") {
        var debugobj=document.getElementById("debugoutputbox");
        MasterDebug = 1-MasterDebug;
        if (MasterDebug==1) {
            if (debugobj) {
                debugobj.style.visibility = "";
            }
        } else {
            if (debugobj) {
                debugobj.style.visibility = "hidden";
            }
        }
    } else if (obj.type == "checkbox" ) {
        fieldValue = obj.checked;
        var pos= target.id.indexOf("memc", 0);
        var memc=0;
        var obj2 = document.getElementById(target.id + "box");
        var obj3 = document.getElementById(target.id + "name");

        if (debug) alert("setVariable: checkbox; value " + fieldValue + "; pos " + pos + "; id " + target.id );
        if (pos >= 0) {
            var fullNameValue = "&variable=" + target.id;
            pos += 4;
            memc = target.id.substr(pos, 1);
            // if we are enabling the checkbox, add it to the list
            if (fieldValue) {
                if (countClientDetailActive(memc) <= 10) {
                    if (obj2) obj2.style.backgroundColor = "yellow";
                    if (obj3) obj3.style.backgroundColor = "yellow";
                    ClientDetails += fullNameValue + ":1";
                    sendCgiRequest();
                } else {
                    alert("There are already 10 clients active for this MEMC");
                    obj.checked = false;
                }
            } else { // remove it from the list
                ClientDetails += fullNameValue + ":0";
                if (obj2) obj2.style.backgroundColor = "white";
                if (obj3) obj3.style.backgroundColor = "white";
                sendCgiRequest();
            }
        }
    } else if ( obj.type == "select-one" ) {
        if(debug) alert("select-one detected; " + BoxMode );
        var selectobj = document.getElementById(fieldName);
        if (fieldName == "boxmode") {
            // if a capture is in progress, do not allow a boxmode change
            if (StartStopButtonState == 1) {
                alert("Please stop the current capture before changing boxmode.");
            } else {
                if (selectobj) { fieldValue = selectobj.options[selectobj.selectedIndex].value; }
                if (selectobj) { BoxMode    = selectobj.options[selectobj.selectedIndex].value; }
                BoxModePrev = 99999; // force the sending of request to server
                if(debug==1) alert("new BoxMode " + BoxMode );

                sendCgiRequest();
            }
        }
    } else if ( fieldName == "brcmlogo" ) {
        window.location.href = "index.html";
    }
}
function MyLoad()
{
    //alert("MyLoad");
    objdebug = document.getElementById("debugoutputbox");
    var local = new Date();
    localdatetime = (local.getUTCMonth()+1).padZero() + local.getUTCDate().padZero() + local.getUTCHours().padZero() + local.getUTCMinutes().padZero() + local.getUTCFullYear() + "." + local.getUTCSeconds().padZero();
    //alert("MyLoad: datetime (" + localdatetime + ")" );

    window.onbeforeunload = MyUnload;
    window.onunload = MyUnload;
    DisableObject("fetchsubmit");
    DisableObject("pauseresumebutton");
    EnableObject("boxmode");
    sendCgiRequest();
}

function randomIntFromInterval(min,max)
{
    return Math.floor(Math.random()*(max-min+1)+min);
}

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
        url = "/cgi/bmemperf_capture.cgi?randomvalue=" + RandomValue;

        if (localdatetime.length > 0) {
            url += "&datetime=" + localdatetime;
            localdatetime = "";
        }

        url += "&boxmode=" + BoxMode;
        if (BoxMode != BoxModePrev) {
            url += "&action=stop";
            BoxModePrev = BoxMode;
        }

        if (sCaptureFilename.length > 0) {
            url += "&capturefile=" + sCaptureFilename;
        }

        if (debug) alert("ClientDetails.length " + ClientDetails.length + "; name " + ClientDetails );
        if (ClientDetails.length > 0) {
            url +=  ClientDetails;
            ClientDetails = "";
            if ( Action.length == 0 ) Action = "setdetails";
        }

        if (Action.length > 0) {
            url += "&action=" + Action;

            if (Action == "stop") {
                //alert("sending " + url );
            }
            Action = "";
        } else {
            // if a capture is in progress, get the current status of it
            if (StartStopButtonState == 1) {
                url += "&action=getstats";
                RecordNumber++;
            }
        }

        if (debug==1) alert("sending " + url );

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

// This function runs as an asynchronous response to a previous server request
function serverHttpResponse ()
{
    var debug=0;
    var CgiCountObj = document.getElementById('cgicount');

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
                CgiTimeoutId = setTimeout ('sendCgiRequest()', REFRESH_IN_MILLISECONDS/10 );
                if (debug && objdebug) objdebug.innerHTML += "calling setTimeout(); ID (" + CgiTimeoutId + ")";
            } else {
                //alert("for i = 0 to " + oResponses.length );
                // loop through <response> elements, and add each nested
                if (objdebug) objdebug.innerHTML = ""; // clear out any previous entries
                for (var i = 0; i < oResponses.length; i++) {
                    var entry = oResponses[i];
                    if ( debug==1 && entry.length>1 ) alert("Entry " + entry + "; len " + entry.length );
                    if ( entry == "ALLDONE" ) {
                        if (objdebug) objdebug.innerHTML += entry + "\n";
                        //alert("response: calling sendCgiRequest(); CgiTimeoutId " + CgiTimeoutId );
                        ResponseCount++;
                        //alert("StartStop " + StartStopButtonState + "; PauseResume " + PauseResumeButtonState );
                        if (StartStopButtonState == 1 && PauseResumeButtonState == 0 ) { // capture is in progress
                            CgiTimeoutId = setTimeout ('sendCgiRequest()', REFRESH_IN_MILLISECONDS );
                            if (debug && objdebug) objdebug.innerHTML += "calling setTimeout(); ID (" + CgiTimeoutId + ")";
                        } else {
                        }
                    } else if (entry == "STBTIME") {
                        if (objdebug) objdebug.innerHTML += entry + " - " + oResponses[i+1] ;
                        var obj2=document.getElementById("stbtime");
                        if (obj2) {
                            if (debug) alert("setting stbtime to " + oResponses[i+1] );
                            if (TextBoxUpdateInProgress==1) {
                                obj2.innerHTML = oResponses[i+1] + "<span style=\"background:yellow;\" >&nbsp;&nbsp;TextBoxUpdateInProgress</span>" ;
                            } else {
                                obj2.innerHTML = oResponses[i+1];
                            }
                            i++;
                        } else {
                            alert("id=stbtime not found");
                        }
                    } else if (entry == "VERSION") {
                        //alert("got response: " + entry );
                        var objversion =document.getElementById("version");
                        if (objversion) {
                            objversion.innerHTML = oResponses[i+1];
                        }
                        i++;
                    } else if (entry == "PLATFORM") {
                        //alert("got response: " + entry );
                        var objplatform =document.getElementById("platform");
                        if (objplatform) {
                            objplatform.innerHTML = oResponses[i+1]; CurrentPlatform = oResponses[i+1];
                        }
                        i++;
                    } else if (entry == "PLATVER") {
                        //alert("got response: " + entry );
                        var objplatform =document.getElementById("platver");
                        if (objplatform) {
                            objplatform.innerHTML = oResponses[i+1]
                        }
                        window.document.title = "Capture " + CurrentPlatform + " " + oResponses[i+1];
                        i++;
                    } else if (entry == "BOXMODE") {
                        //if (debugobj) {debugobj.innerHTML += entry + "\n"; }
                        BoxMode = oResponses[i+1];
                        if (BoxMode == -1) {
                            BoxMode = 1;
                        }
                        BoxModePrev = BoxMode;
                        //alert("response: BOXMODE " + BoxMode );
                        i++;
                    } else if (entry == "BOXMODEHTML") {
                        if (StartStopButtonState == 0 ) { // start capture is displayed on the button
                            //if (debugobj) {debugobj.innerHTML += entry + "\n"; }
                            var objboxmode=document.getElementById("tdboxmode");
                            //alert("response: BOXMODEHTML " + oResponses[i+1] );
                            if (objboxmode) { objboxmode.innerHTML = oResponses[i+1] } else { alert("boxmode elementId not found"); }
                        }
                        i++;
                    } else if (entry == "NUMMEMC") {
                        //alert("got response: " + entry );
                        NumberOfMemc = oResponses[i+1];
                        i++;
                    } else if ( entry == "CLIENT_LIST_CHECKBOXES" ) {
                        if (objdebug) objdebug.innerHTML += entry + "\n";
                        var obj2=document.getElementById("top10");
                        if (obj2 && TextBoxUpdateInProgress==0) {
                            obj2.innerHTML = oResponses[i+1];
                            i++;
                        }
                    } else if (entry == "BYTES") {
                        var sizeobj=document.getElementById("capturesize");
                        if (sizeobj) {
                            sizeobj.innerHTML =  oResponses[i+1] + " (" + Math.floor(RecordNumber/60) + ":" + ("00" + RecordNumber%60).slice (-2)+ ")"; // leading zero
                        }
                        var descobj=document.getElementById("capturedesciptor");
                        if (descobj) {
                            descobj.innerHTML = "Capture file size:";
                        }
                        i++;
                    } else if (entry == "TGZSUCCESSFUL") {
                        //alert(entry);
                        EnableObject("fetchsubmit");
                        EnableObject("boxmode");
                    } else if (entry == "ALERT") {
                        alert(oResponses[i+1]);
                        i++;
                    } else {
                        if ( objdebug)  objdebug.innerHTML  +=  entry;
                    }
                } // for each response
            } // if response is not blank
        } else {
            alert("There was a problem retrieving the XML data:\n" + xmlhttp.statusText);
        }

    } //if (xmlhttp.readyState==4 )
} // this.serverHttpResponse

function MyUnload()
{
    //alert("MyUnload");
}

function FetchSubmit()
{
    if (sCaptureFilename.length) {
        var obj=document.getElementById("capturefile");
        //alert("FetchSubmit: file (" + sCaptureFilename + "); obj " + obj);
        if (obj) {
            obj.value = sCaptureFilename;
            obj.innerHTML = sCaptureFilename;
            return true;
        }
    } else {
        alert("Capture file name is blank");
    }

    return false;
}
