<!--

var BoxMode = -1;
var CurrentPage = 1;
var MasterDebug=0;
var MasterSending=0;
var PreviousPageId = "pageHome";
var CurrentPageId = "pageHome";
var gFieldName = ""; // used to send the update to the CGI app
var gFieldValue = ""; // used to send the update to the CGI app
var CurrentPlatform = "";
var downloadFormIsDisplayed=false;
var localdatetime = "";
var userAgent = 0;

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
    var local = new Date();
    localdatetime = (local.getUTCMonth()+1).padZero() + local.getUTCDate().padZero() + local.getUTCHours().padZero() + local.getUTCMinutes().padZero() + local.getUTCFullYear() + "." + local.getUTCSeconds().padZero();
    userAgent = navigator.userAgent;

    processDebugOutputBox( document.getElementById("debugoutputbox") ); // hide or show the debugoutputbox

    brcmAlertConfig();

    GetNexusMemoryConfig();
}

function getPosition(element)
{
    var xPosition = 0;
    var yPosition = 0;

    while(element) {
        xPosition += element.offsetLeft;
        yPosition += element.offsetTop;
        element = element.offsetParent;
    }
    //alert("getPosition: X " + xPosition + "; Y " + yPosition );
    return [ xPosition, yPosition ];
}

function validateForm(event)
{
    var target = event.target || event.srcElement; // needed for IE8
    var id=target.id;
    //alert("validate form: " + downloadFormIsDisplayed + "; id " + id );
    var objfile = document.getElementById('btnstatefile');
    if (objfile) {
        //alert("file len " + objfile.value.length );
        if ( downloadFormIsDisplayed && objfile.value.length == 0 ) {
            downloadFormIsDisplayed = false;
            alert("File name is empty!");
            //var objbody=document.getElementsByTagName('body')[0];
            //if (objbody) { objbody.style.backgroundColor="white"; }
            RestoreClick(event);
        }
    }
    return downloadFormIsDisplayed;
}

function RestoreClick (event)
{
    var target = event.target || event.srcElement; // needed for IE8
    var id=target.id;
    //alert("RestoreClick: " + id );

    var obj=document.getElementById('uploadmain');
    var objbody=document.getElementsByTagName('body')[0];
    if ( obj)
    {
        var position = getPosition(obj);
        var inputElements = document.getElementsByTagName('input');
        var selectElements = document.getElementsByTagName('select');
        //alert("visibility " + obj.style.visibility + "; x " + position[0] + "; y " + position[1] );
        if ( obj.style.visibility == "hidden" )
        {
            obj.style.left = "400px";
            obj.style.top  = "200px";
            obj.style.visibility = "";
            obj.style.backgroundColor="white";
            if (objbody) { objbody.style.backgroundColor="#ababab"; } // gray
            downloadFormIsDisplayed=true;
            var objfile = document.getElementById('btnstatefile');

            if (objfile) objfile.value="";
        }
        else
        {
            obj.style.visibility = "hidden";
            if (objbody) { objbody.style.backgroundColor="white"; }
            downloadFormIsDisplayed=false;
        }

        for (var i=0, max=inputElements.length; i < max; i++) {
            if (inputElements[i].id == "btnstatefile" || inputElements[i].id == "btncancel" || inputElements[i].id == "btnupload" ) {
                //alert("element " + inputElements[i].type + "; name " + inputElements[i].id );
            } else {
                inputElements[i].disabled = downloadFormIsDisplayed;
            }
        }
        for (var i=0, max=selectElements.length; i < max; i++) {
            //alert("element " + inputElements[i].type + "; name " + inputElements[i].id );
            selectElements[i].disabled = downloadFormIsDisplayed;
        }
    }

    return false;
}

function HighlightCurrentPage(pageid)
{
    CurrentPageId = pageid;
    var obj;
    //alert("Highlight: prev " + PreviousPageId + "; current " + CurrentPageId );
    obj = document.getElementById(PreviousPageId);
    if (userAgent.indexOf("MSIE") >= 0 ) { // for ie9, ignore highlighting
    } else {
        if (obj) { obj.style.fontWeight = "bold"; obj.style.background = "white"; }
    }
    obj = document.getElementById(CurrentPageId);
    if (userAgent.indexOf("MSIE") >= 0 ) { // for ie9, ignore highlighting
    } else {
        if (obj) { obj.style.fontWeight = "bold"; obj.style.background = "lightgray"; }
    }
    PreviousPageId = CurrentPageId;
}

function setField(fieldName, fieldValue)
{
    //alert("setField: name " + fieldName + "; value " + fieldValue );
    var obj=document.getElementById(fieldName);
    if (obj) {
        //alert("setField: obj is " + obj.type );
        if (obj.type == "checkbox" ) {
            obj.checked = fieldValue;
        } else {
            obj.value = fieldValue;
        }
    }
}
function setRowStyle(fieldBeginning, newValue)
{
    var obj;
    var fieldName;
    for (idx=1; idx<5; idx++) {
        fieldName = fieldBeginning + "row" + idx;
        //alert("setRowStyle: field " + fieldName + "; value " + newValue );
        obj = document.getElementById(fieldName);
        if ( obj ) {
            obj.style.display = newValue;
        }
    }
}

function processDebugOutputBox( debugobj )
{
    if (MasterDebug==1) {
        if (debugobj) { debugobj.style.visibility = ""; debugobj.style.display= ""; }
    } else {
        if (debugobj) { debugobj.style.visibility = "hidden"; debugobj.style.display= "none"; }
    }
}
function setVariable(fieldName)
{
    var debug=0;
    var obj=document.getElementById(fieldName);
    if (obj) {
        gFieldName = fieldName; // used to send the update to the CGI app
        //alert("setVariable: name " + fieldName + "; type " + obj.type );
        var arrayname = fieldName.substr(0,4);
        var which_entry = fieldName.substr(4,1);
        var sub_index = "";
        var field = "";
        var fieldValue = "";

        if (arrayname == "disp") {
            sub_index = fieldName.substr(8,1); // win0, win1, etc
            field = fieldName.substr(9,20).replace(/[0-9]+/ig,"");
            fieldValue = fieldName.substr(9,20).replace(/[^0-9]+/ig,"");
        } else {
            field = fieldName.substr(5,20).replace(/[0-9]+/ig,"");
            fieldValue = fieldName.substr(5,20).replace(/[^0-9]+/ig,"");
        }

        if (obj.type == "checkbox" ) {
            fieldValue = obj.checked;
        } else if ( obj.type == "text" ) {
            fieldValue = obj.value;
        } else if ( obj.type == "radio" ) {
            var query = 'input[name=' + fieldName + ']:checked'
            fieldValue = document.querySelector(query).value;
        } else if ( obj.type == "select-one" ) {
            var selectobj = document.getElementById(fieldName);
            if (fieldName == "boxmode") {
                if (selectobj) { fieldValue = selectobj.options[selectobj.selectedIndex].value; }
                if (selectobj) { BoxMode    = selectobj.options[selectobj.selectedIndex].value; }
                if(debug) alert("new BoxMode " + BoxMode );
            } else if (fieldName.indexOf("maxFormat")>0) {
                if (selectobj) { fieldValue = selectobj.options[selectobj.selectedIndex].value; }
            } else if (fieldName.indexOf("maxDecoderOutputChannels")>0) {
                if (selectobj) { fieldValue = selectobj.options[selectobj.selectedIndex].value; }
            } else if (fieldName.indexOf("maxDecoderOutputSamplerate")>0) {
                if (selectobj) { fieldValue = selectobj.options[selectobj.selectedIndex].value; }
            } else if (fieldName.indexOf("dolbyCodecVersion")>0) {
                if (selectobj) { fieldValue = selectobj.options[selectobj.selectedIndex].value; }
            }
        }
        if (fieldValue == false) {
            gFieldValue = "0"; // used to send the update to the CGI app
        } else if (fieldValue == true) {
            gFieldValue = "1"; // used to send the update to the CGI app
        } else {
            gFieldValue = fieldValue; // used to send the update to the CGI app
        }
        //alert("variable is " + fieldName + "; type " + obj.type + "; array " + arrayname + "; idx " + which_entry + "; subidx " + sub_index + "; field is " + field + "; value " + fieldValue );
        if (fieldName == "h2broadcom") {
            MasterDebug = 1-MasterDebug;
            processDebugOutputBox( document.getElementById("debugoutputbox") );
        } else if (fieldName == "h2memory") {
            MasterSending = 1-MasterSending;
        } else if ( fieldName == "brcmlogo" ) {
            window.location.href = "index.html";
        }
    }
}

function getCumulativeOffset (element) {
    var xPosition = 0;
    var yPosition = 0;

    while(element) {
        xPosition += (element.offsetLeft - element.scrollLeft + element.clientLeft);
        yPosition += (element.offsetTop - element.scrollTop + element.clientTop);
        element = element.offsetParent;
        //alert("left " + xPosition + "; top " + yPosition );
    }
    //alert("left " + xPosition + "; top " + yPosition );
    //return { x: xPosition, y: yPosition };
}

function MyClick(event)
{
    var target = event.target || event.srcElement; // needed for IE8
    var id=target.id;
    //alert("MyClick: " + id + "; formIsDisplayed " + downloadFormIsDisplayed );
    if ( downloadFormIsDisplayed ) {
    } else {
        var obj=document.getElementById('restoreState');
        if (obj) {
            getCumulativeOffset(obj);
        }
        setVariable(id);
        if ( id != "brcmlogo" ) ShowPage(id);
    }
}

function ShowPage(pageid)
{
    /* this function can be called with pageid values for checkboxes, radio buttons, entry boxes, etc. */
    //alert("showPage: " + pageid);
    if (pageid == "pageHome") {
        CurrentPage = 1;
        HighlightCurrentPage(pageid);
    } else if (pageid == "pageTransport") {
        CurrentPage = 2;
        HighlightCurrentPage(pageid);
    } else if (pageid == "pageVideoDecoder") {
        CurrentPage = 3;
        HighlightCurrentPage(pageid);
    } else if (pageid == "pageAudioDecoder") {
        CurrentPage = 4;
        HighlightCurrentPage(pageid);
    } else if (pageid == "pageDisplay") {
        CurrentPage = 5;
        HighlightCurrentPage(pageid);
    } else if (pageid == "pageGraphics") {
        CurrentPage = 6;
        HighlightCurrentPage(pageid);
    } else if (pageid == "pageEncoder") {
        CurrentPage = 7;
        HighlightCurrentPage(pageid);
    } else if (pageid == "pageGenerateCode") {
        CurrentPage = 8;
        HighlightCurrentPage(pageid);
    } else if (pageid == "pagePictureDecoder") {
        CurrentPage = 9;
        HighlightCurrentPage(pageid);
    } else if (pageid == "pageVideoInput") {
        CurrentPage = 10;
        HighlightCurrentPage(pageid);
    } else if (pageid.substr(0,8) == "pageHelp") {
        CurrentPage = 11;
        HighlightCurrentPage(pageid.substr(0,8));
    }
    //alert("ShowPage: id " + pageid + "; calling GetNexusMemoryConfig");
    GetNexusMemoryConfig();
}

function return0or1(value)
{
    if (value) {
        return 1;
    } else {
        return 0;
    }
}
function GetNexusMemoryConfig()
{
    var debug=0;
    var debugobj=document.getElementById("debugoutputbox");

    xmlhttp=new XMLHttpRequest();

    var url = "http://" + document.location.hostname + "/cgi/bmemconfig.cgi?boxmode=" + BoxMode + "&page=" + CurrentPage;

    if (localdatetime.length > 0) {
        url += "&datetime=" + localdatetime;
        localdatetime = "";
    }

    if (gFieldName && gFieldValue)
    {
        url += "&variable=" + gFieldName + ":" + gFieldValue;
    }
    else
    {
        //alert("invalid gFieldName " + gFieldName + "; len " + gFieldName.length + "; value " + gFieldValue + "; len " + gFieldValue.length );
    }

    if(MasterSending==1) alert("sending " + url );
    if(debugobj) {debugobj.innerHTML = ""; } // clear out the debug output box
    xmlhttp.onreadystatechange = serverHttpResponse;
    xmlhttp.open("GET",url,true);
    xmlhttp.send(null);
}

function serverHttpResponse()
{
    var debug=0;
    var eol = "\n"; // end of line character for Firefox, Chrome, Safari (not for ie7)

    /* Because function runs as an asynchronous response to a previous server request, the value of "this"
       is not the same as the constructor value of ". For this reason, we use the global variable "gthis"
       to access the previous instantiation of "
    */
    if(debug) alert("serverHttpResponse: got readyState " + xmlhttp.readyState );

/*
* 0: The request is uninitialized (before you've called open()).
* 1: The request is set up, but hasn't been sent (before you've called send()).
* 2: The request was sent and is being processed (you can usually get content headers from the response at point).
* 3: The request is being processed; often some partial data is available from the response, but the server hasn't finished with its response.
* 4: The response is complete; you can get the server's response and use it.

var response = request.responseText.split("|");
*/
    if (xmlhttp.readyState==4 )
    {
        if(debug) alert("serverHttpResponse: got readyState " + xmlhttp.readyState + "; status " + xmlhttp.status );

        //var response = xmlhttp.getAllResponseHeaders();
        //var response = xmlhttp.getResponseHeader("Date");
        //alert("Date from header is " + response);

        // only if "OK"

        if (xmlhttp.status == 200)
        {
            var response_list = "";
            var divheapmem=document.getElementById("tdheapmem");
            var divpagecontents=document.getElementById("pagecontents");
            var debugobj=document.getElementById("debugoutputbox");

            if (userAgent.indexOf("MSIE") >= 0 ) { // for ie9, must replace carriage returns with <br>
                //eol = "<br>";
                if(debug) alert("MSIE detected; debugobj:" + debugobj );
            }

            //alert("setting debugdiv");
            //document.getElementById("debugdiv").innerHTML = xmlhttp.responseText; // works firefox, ie7, safari
            if(debug) alert("setting debugoutputbox");
            if(debugobj) {
                // if response has html tags in it <input ...>, you cannot display the response in a textarea box
                // to resolve this IE feature, globally replace the < and > chars with &lt; and &gt;
                var newtext = xmlhttp.responseText.replace(/</g,"&lt;");
                //debugobj.innerHTML = newtext.replace(/>/g,"&gt;");
                debugobj.scrollTop = debugobj.scrollHeight;
            }
            if(debug) alert("innerHTML done");

            //var oResponses = xmlhttp.responseXML.getElementsByTagName("response")[0];
            //alert("var oResponses calling");
            //var oResponses = xmlhttp.responseXML.getElementsByTagName("response");
            var responseText = xmlhttp.responseText;
            var oResponses = responseText.split( "~" );
            if(debug) alert("num responses is " + oResponses.length );

            //alert("for i = 0 to " + oResponses.length );
            // loop through <response> elements, and add each nested
            for (var i = 0; i < oResponses.length; i++)
            {
                var entry = rtrim(oResponses[i]);
                if ( entry.length>0 )
                {
                    //if(debug=) alert("Response: got (" + entry + "); length " + oResponses[i+1].length );
                }
                if (entry == "HEAPS") {
                    //if (debugobj) {debugobj.innerHTML += entry + eol; }
                    if (divheapmem) { divheapmem.innerHTML = oResponses[i+1] }
                    i++;
                } else if (entry == "OPENAPPS") {
                    //if (debugobj) {debugobj.innerHTML += entry + eol; }
                    if (divpagecontents) { divpagecontents.innerHTML = oResponses[i+1] }
                    i++;
                } else if (entry == "HOME") {
                    //if (debugobj) {debugobj.innerHTML += entry + eol; }
                    if (divpagecontents) { divpagecontents.innerHTML = oResponses[i+1] }
                    i++;
                } else if (entry == "DISPLAY") {
                    //if (debugobj) {debugobj.innerHTML += entry + eol; }
                    if (divpagecontents) { divpagecontents.innerHTML = oResponses[i+1] }
                    i++;
                } else if (entry == "VIDEODEC") {
                    //if (debugobj) {debugobj.innerHTML += entry + eol; }
                    if (divpagecontents) { divpagecontents.innerHTML = oResponses[i+1] }
                    i++;
                } else if (entry == "VIDEOINPUT") {
                    //if (debugobj) {debugobj.innerHTML += entry + eol; }
                    if (divpagecontents) { divpagecontents.innerHTML = oResponses[i+1] }
                    i++;
                } else if (entry == "ENCODER") {
                    //if (debugobj) {debugobj.innerHTML += entry + eol; }
                    if (divpagecontents) { divpagecontents.innerHTML = oResponses[i+1] }
                    i++;
                } else if (entry == "AUDIODEC") {
                    //if (debugobj) {debugobj.innerHTML += oResponses[i+1] + eol; }
                    if (divpagecontents) { divpagecontents.innerHTML = oResponses[i+1] }
                    i++;
                } else if (entry == "TRANSPORT") {
                    //if (debugobj) {debugobj.innerHTML += entry + eol; }
                    if (divpagecontents) { divpagecontents.innerHTML = oResponses[i+1] }
                    i++;
                } else if (entry == "GRAPHICS") {
                    //if (debugobj) {debugobj.innerHTML += entry + eol; }
                    if (divpagecontents) { divpagecontents.innerHTML = oResponses[i+1] }
                    i++;
                } else if (entry == "HELP") {
                    //if (debugobj) {debugobj.innerHTML += entry + eol; }
                    if (divpagecontents) { divpagecontents.innerHTML = oResponses[i+1] }
                    i++;
                } else if (entry == "FILES") {
                    //if (debugobj) {debugobj.innerHTML += entry + eol; }
                    if (divpagecontents) { divpagecontents.innerHTML = oResponses[i+1] }
                    var objlog=document.getElementById('consolelog');
                    if (objlog) {
                        objlog.scrollTop = objlog.scrollHeight;
                    }
                    objlog=document.getElementById('accesslog');
                    if (objlog) {
                        objlog.scrollTop = objlog.scrollHeight;
                    }
                    i++;
                } else if (entry == "PICDECODER") {
                    //if (debugobj) {debugobj.innerHTML += entry + eol; }
                    if (divpagecontents) { divpagecontents.innerHTML = oResponses[i+1] }
                    i++;
                } else if (entry == "STBTIME") {
                    //if (debugobj) {debugobj.innerHTML += entry + eol; }
                    var objtime=document.getElementById("stbtime");
                    if (objtime) { objtime.innerHTML = oResponses[i+1] }
                    i++;
                } else if (entry == "BOXMODE") {
                    if (debugobj) {debugobj.innerHTML += entry + "; value (" + oResponses[i+1] + ")" + eol; }
                    BoxMode = oResponses[i+1];
                    i++;
                } else if (entry == "BOXMODEHTML") {
                    if (debugobj) {debugobj.innerHTML += entry + eol; }
                    var objboxmode=document.getElementById("boxmode");
                    if(debug) alert("objboxmode:" + objboxmode + "; response (" + oResponses[i+1] + ")" );
                    if (objboxmode) {
                        // IE9 and families will not re-render the <option> object; must re-create the entire object to get it to re-render
                        if (userAgent.indexOf("MSIE") >= 0 ) {
                            var divboxmode = document.getElementById('divboxmode');
                            if (divboxmode) {
                                divboxmode.innerHTML = "<select id=boxmode onchange=\"MyClick(event);\" >" + oResponses[i+1] + "</select>";
                            }
                        } else {
                            objboxmode.innerHTML = oResponses[i+1];
                        }
                    }

                    i++;
                } else if (entry == "PLATFORM") {
                    //if (debugobj) {debugobj.innerHTML += entry + eol; }
                    var objplatform =document.getElementById("platform");
                    if (objplatform) { objplatform.innerHTML = oResponses[i+1]; CurrentPlatform = oResponses[i+1]; }
                    i++;
                } else if (entry == "PLATVER") {
                    //if (debugobj) {debugobj.innerHTML += entry + eol; }
                    var objplatform =document.getElementById("platver");
                    if (objplatform) { objplatform.innerHTML = oResponses[i+1] }
                    window.document.title = CurrentPlatform + " " + oResponses[i+1];
                    i++;
                } else if (entry == "FATAL") {
                    if (debugobj) {debugobj.innerHTML += entry + "; " + oResponses[i+1] + eol; }
                    brcmAlert( "FATAL ... " + oResponses[i+1] );
                    CurrentPage = 8;
                    CurrentPageId = "pageGenerateCode";
                    HighlightCurrentPage(CurrentPageId);
                    i++;
                } else if (entry == "ALERT") {
                    if (debugobj) {debugobj.innerHTML += entry + eol; }
                    alert("ERROR ... " + oResponses[i+1]);
                    i++;
                } else {
                    if (debugobj && entry.length >0 && entry.length < 200) {
                        debugobj.innerHTML += entry + eol;
                    }
                }
                if (entry.length >0 && entry.length < 20)
                {
                    //alert("Response: done for entry (" + entry + ")" );
                }
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
