////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////
var MasterDebug=0;

var REFRESH_IN_MILLISECONDS=1000;
var BMEMPERF_MAX_NUM_CPUS=8;
var passcount=0;
var previous_height = 0;
var CgiTimeoutId=0;
var CgiRetryTimeoutId=0;
var CgiCount=0; // number of times the cgi was called
var SetVariableCount=0; // number of times setVariable() is called
var ResponseCount=0; // number of times serverHttpResponse() is called
var objdebug = 0;
var epochSeconds = 0;
var tzOffset = 0;
var localdatetime = "";
var userAgent = 0;
var gNumCpus = 0;
var gCpuData = [0,0,0,0,0,0,0,0];
var gIrqLatestData = [0,0,0,0,0,0,0,0];
var gIrqMaxData = [0,0,0,0,0,0,0,0];
var bCpuDataHeightSet = 0;
var bIrqDataHeightSet = 0;
var NetBytesPrev =[[0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0]]; // up to 10 network interfaces with Rx and Tx for each of the 10
var NetBytesCummulative =[[0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0]]; // up to 10 network interfaces with Rx and Tx for each of the 10
var NetBytesRx10SecondsCount=[0,0,0,0,0,0,0,0,0,0];
var NetBytesRx10Seconds =[[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0]
                         ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0]];
var NetBytesTx10SecondsCount=[0,0,0,0,0,0,0,0,0,0];
var NetBytesTx10Seconds =[[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0]
                         ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0] ,[0,0,0,0,0,0,0,0,0,0]];
var NetBytesSeconds = 0; // net bytes are accumulated for as long as the browser is on this page; this value is used to compute the average Mbps
var GetCpuInfo = {Value: 0};
var GetMemory = {Value: 0};
var GetIrqInfo = {Value: 0};
var GetNetStats =  {Value: 0};
var GetHeapStats =  {Value: 0};
var GetSataUsb =  {Value: 0, Stop:0 };
var GetProfiling =  {Value: 0};
var GetContextSwitch = {Value: 0};
var GetPerfTop =  {Value: 0};
var GetLinuxTop =  {Value: 0, Stop:0 };
var GetPerfCache =  {Value: 0, Duration:0, StartReportNow:false };
var GetPerfCacheResults =  {Value: false };
var GetPerfCacheCountdown = 0;
var GetPerfDeep =  {Value: 0, Duration:0, StartReportNow:0 };
var GetPerfDeepResults =  {Value: false };
var GetPerfDeepCountdown = 0;
var GetPerfFlame =  {State:0, Value: 0, Stop:0 };
var GetPerfFlameResults =  {Value: false };
var GetPerfFlameRecordingSeconds = 0;
var GetPerfFlamePidCount = 0;

//  GetPerfFlameState needs to match the enum Bsysperf_PerfFlame_State in bsysperf.c
var GetPerfFlameState = { UNINIT:0, INIT:1, IDLE:2, START:3, RECORDING:4, STOP:5, CREATESCRIPTOUT:6, GETSVG:7, DELETEOUTFILE:8 };

var GetPerfFlameSvgCount = 1;
var PerfRecordUuid = "";
var FlameWindow = 0;
var validResponseReceived = 0;
var GlobalDebug = 0;
var CountdownSeconds = 0;
var PerfCacheCountdownSeconds = 0;
var PERF_STAT_DURATION = 3;
var gEvent = 0;
var PerfError = true; // set to true of perf tool cannot be found
var ChangeCpuState = 0; // used to enable or disable a specific CPU
var urlSentPreviously = "";
var urlSentSuccessfully = 0;
var urlSentRetryCount = 0;
var urlSentRetryCountAfterSuccess = 0;
var eol = "\n"; // end of line character for Firefox, Chrome, Safari (not for ie7 ... use <br>)
var cpuUsageLongAverage = [0,0,0,0,0,0,0,0,0,0]; // 10-second average CPU utilization
var CPU_USAGE_LONG_AVERAGE_MAX = 5; // 10-second window
var cpuUsageLongAverageIdx = 0; // index into 10-second average CPU utilization array (cpuUsageLongAverage)
var cpuUsageLongAverageCount = 0; // number of 10-second average CPU utilization array (cpuUsageLongAverage)

Number.prototype.padZero= function(len){
    var s= String(this), c= '0';
    len= len || 2;
    while (s.length < len) s= c + s;
    return s;
}

function randomFromTo(from, to)
{
    return Math.floor(Math.random() * (to - from + 1) + from);
}

function OneSecond ()
{
    //alert("OneSecond");
    MyLoad();
}
function getNumEntries ( arrayname )
{
    var num_entries = arrayname.length-1;
    if (userAgent.indexOf("MSIE") >= 0 ) {
        num_entries++; // for ie9, length is one less than firefox, chrome, safari
    }
    //if (numlines==1) alert("1 coords (" + coords + "); num points" + num_entries );

    return num_entries;
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

function hideElement ( elementid )
{
    elemobj = document.getElementById(elementid);
    if (elemobj) {
        //alert("hiding element " + elementid );
        elemobj.style.visibility = "hidden";
    }
}

function showRow ( rownum )
{
    var rowid;
    var rowobj;

    rowid = "row0" + rownum + "a";
    rowobj = document.getElementById(rowid);
    if (rownum%2 == 1 && rowobj && rownum < gNumCpus ) {
        var even_number_of_cpus = gNumCpus%2;
        //alert("rowobj " + rowid + "; obj " + rowobj + "; gNumCpus " + gNumCpus + "even_number " + even_number_of_cpus );

        // if we have an odd number of cpus, hide the right hand column
        if (even_number_of_cpus == 0) {
            rowid = "rightcol0" + rownum + "a";
            hideElement(rowid);
            rowid = "rightcol0" + rownum + "b";
            hideElement(rowid);
            rowid = "rightcol0" + rownum + "c";
            hideElement(rowid);
        }

        rowobj.style.visibility = "";
        rowid = "row0" + rownum + "b";
        rowobj = document.getElementById(rowid);
        rowobj.style.visibility = "";
        rowid = "row0" + rownum + "c";
        rowobj = document.getElementById(rowid);
        rowobj.style.visibility = "";
    }
    //alert("showRow done");
}

function SetCheckboxDisabled ( checkboxName, objStatus )
{
    var obj = document.getElementById( checkboxName );
    if (obj ) {
        //if (checkboxName =="checkboxPerfCache"  ) { alert("SetCheckboxDisabled ( " + checkboxName + ") = " + objStatus + "; PerfError=" + PerfError ); }
        if ( (checkboxName =="checkboxPerfTop" || checkboxName == "checkboxPerfDeep" || checkboxName == "checkboxPerfCache" ) && ( PerfError == true ) ) {
            // do not allow changes to Perf checkboxes if kernel is not compiled to include perf tools
            obj.disabled = true;
        } else {
            obj.disabled = objStatus;
        }
        //alert("settings (" + checkboxName + ") status (" + objStatus + ") to value (" + obj.disabled + ")" );
    }
}

function GetCheckboxStatus ( checkboxName )
{
    var rc = false;
    var obj = document.getElementById( checkboxName );
    if (obj ) {
        rc = obj.checked;
        //alert("settings (" + checkboxName + ") status (" + objStatus + ") to value (" + objStatus.Value + ")" );
    }

    return rc;
}

function SetCheckboxStatus ( checkboxName, objStatus )
{
    var obj = document.getElementById( checkboxName );
    if (obj ) {
        //if (checkboxName =="checkboxPerfCache"  ) { alert("SetCheckboxStatus ( " + checkboxName + ") = " + objStatus.Value + "; PerfError=" + PerfError ); }
        if ( (checkboxName =="checkboxPerfTop" || checkboxName == "checkboxPerfDeep" || checkboxName == "checkboxPerfCache" ) && ( PerfError == true ) ) {
            // do not allow changes to Perf checkboxes if kernel is not compiled to include perf tools
            obj.checked = false;
        } else {
            obj.checked = objStatus.Value;
        }
        //alert("SetCheckboxStatus (" + checkboxName + ") to value (" + objStatus.Value + ")" );
    } else {
        //alert("SetCheckboxStatus (" + checkboxName + ") is unknown" );
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

function SvgClickHandler (event)
{
    var target = event.target || event.srcElement;
    var id = target.id;
    //alert("svgClickHandler: id (" + id + ");" );
    MyClick(event);
}

function AppendPolylineXY ( polylinenumber, newValue )
{
    var lineid = "polyline0" + polylinenumber;
    var svgobj = document.getElementById(lineid);

    // alert("polylinenumber " + polylinenumber + "; gNumCpus " + gNumCpus + "; svgobj " + svgobj + "; lineid " + lineid );
    if (svgobj && gNumCpus && polylinenumber<(gNumCpus+1) ) { // added one for limegreen 5-second average line
        // coords example: 0,100 2,85 5,95 6,95 (i.e. x,y where the y value is pixels "below" the top of the graph; 100 pixels means 0 percent
        var coords = svgobj.getAttribute('points' );
        //if(polylinenumber==5) alert("gNumCpus " + gNumCpus + "; lineid " + lineid + "; coords " + coords );
        if (coords == null) {
            coords = "0,100 ";
        }

        if (coords) {
            var splits = coords.split(' ');
            var newcoords = "";
            var starting_idx = 0;
            var num_entries = getNumEntries(splits);

            if ( num_entries < 250 ) {
                starting_idx = 0;
            } else {
                starting_idx = 1;// skip the first element; it is dropping off the left side
            }
            for (idx=starting_idx; idx < num_entries; idx++ ) {
                var justone = splits[idx].split(',');
                newcoords += idx*2 + "," + justone[1] + " " ;
                //if(polylinenumber==5) alert("for point " + idx + ", newcoords (" + newcoords + ")" );
            }
            // add new last point to the far right
            if (polylinenumber < (gNumCpus+1) ) { // if we are processing the cpu poly lines
                if ( newValue == 255) { // if the cpu is inactive
                } else {
                    newcoords += idx*2 + "," + newValue + " " ;
                }
            }
            //if(polylinenumber==5) alert("setAttribute:(" + newcoords + ")" );
            svgobj.setAttribute('points', newcoords );
        }
    } else {
        //alert ("lineid " + lineid + " does not exist");
    }
}

function MyLoad()
{
    userAgent = navigator.userAgent;
    //alert("MyLoad: browser (" + userAgent + "); passcount " + passcount);
    var idx;
    var transformAttr = "";
    var polylinenumber;
    var rowid;
    var lineid;
    var rowobj;
    var svgobj;

    objdebug = document.getElementById("debugoutputbox");
    //alert("passcount==" + passcount + "; gNumCpus " + gNumCpus );
    if ( passcount == 0 ) {
        var local = new Date();
        epochSeconds = Math.floor(local.getTime() / 1000);
        //alert("local Date " + epochSeconds );
        tzOffset = local.getTimezoneOffset();
        //alert("TZ offset " + local.getTimezoneOffset() );
        localdatetime = (local.getUTCMonth()+1).padZero() + local.getUTCDate().padZero() + local.getUTCHours().padZero() + local.getUTCMinutes().padZero() +
                        local.getUTCFullYear() + "." + local.getUTCSeconds().padZero();

        var newpoints = "";
        var newheight = 0;
        for (polylinenumber=1; polylinenumber<(gNumCpus-1)+1 ; polylinenumber++) { //once for all CPUs
            newpoints = "20,100 "; // 100% idle
            lineid = "polyline0" + polylinenumber;
            //alert("lineid " + lineid );
            svgobj = document.getElementById(lineid);
            if (svgobj) {
                var rect = svgobj.getBoundingClientRect();

                //alert("lineid " + lineid + "; left " + rect.left + "; right " + rect.right + "; top " + rect.top + ";btm " + rect.bottom );
                for (idx=2; idx<1-1; idx+=2) {
                    newheight = randomFromTo(0,100);
                    newpoints += idx + "," + newheight + " ";
                }
                //alert("for " + lineid + ", 1st newpoints (" + newpoints + ")" + svgobj );
                svgobj.setAttribute('points', newpoints );
            }
        }

        SetInternalCheckboxStatus ( "checkboxcpus", GetCpuInfo );
        SetInternalCheckboxStatus ( "checkboxmemory", GetMemory );
        SetInternalCheckboxStatus ( "checkboxnets", GetNetStats );
        SetInternalCheckboxStatus ( "checkboxirqs", GetIrqInfo );
        SetInternalCheckboxStatus ( "checkboxprofiling", GetProfiling );
        SetInternalCheckboxStatus ( "checkboxPerfFlame", GetPerfFlame);

        GetSataUsb.Stop = 1; // tell the server to stop any SATA/USB data collection that may have been started earlier

        //alert("pass 0 done");
    } else {
        var objCheckboxCpus = document.getElementById("checkboxcpus");
        if (objCheckboxCpus && objCheckboxCpus.checked ) {
            var adjusted_poly = [0,0,0,0];
            var irqIdx = 0;
            for (polylinenumber=1; polylinenumber<(gNumCpus-1)+1 ; polylinenumber++) { //once for all CPUs
                AppendPolylineXY ( polylinenumber, gCpuData[polylinenumber-1] );
            }
        }
    }
    //alert("polyline01 points (" + document.getElementById('polyline01').getAttribute('points') + ")" );
    previous_height += 10;
    //alert("prev_hgt " + previous_height);
    if ( previous_height > 100) previous_height = 0;

    //var svgid = 'svg' + GetPerfFlameSvgCount.padZero(4);
    var svgid = 'svg001';
    svgid = 'svg' + GetPerfFlameSvgCount.padZero(4);
    //alert("MyLoad - svgid:" + svgid );
    svgobj = document.getElementById( svgid );
    if (svgobj) {
        svgobj.addEventListener("click", SvgClickHandler, false);
    }
    passcount++;

    sendCgiRequest();

    //alert("end passcount==" + passcount);
}

function randomFromTo(from, to)
{
    return Math.floor(Math.random() * (to - from + 1) + from);
}

function rtrim(stringToTrim) { return stringToTrim.replace(/\s+$/,"");}

function MyChange(event)
{
    var target = event.target || event.srcElement;
    var id = target.id;
    var value=0;
    var obj=document.getElementById(id);
    if (obj) {
        value = obj.value;
    }
    //alert("MyChange: value " + value);
    setVariable(id);
}
function AdjustRectangleToTextWidth ( id )
{
    var obj=document.getElementById(id);
    if (obj) {
        var pointsStr = obj.getAttribute("points");
        var pointsStrNew = "";

        //alert("points (" + pointsStr + ")" );

        // points (190,10  320,10  320,60  190,60 )
        var points = pointsStr.split( " " );
        pointsStrNew = points[0] + " ";
        //alert("points1 (" + pointsStrNew + ")" );

        var xypair = points[1].split( ",");
        pointsStrNew += Number(xypair[0]) + Number(400);
        pointsStrNew += "," + xypair[1] + " ";
        //alert("points2 (" + pointsStrNew + ")" );

        xypair = points[2].split( ",");
        pointsStrNew += Number(xypair[0]) + Number(400);
        pointsStrNew += "," + xypair[1] + " ";
        //alert("points3 (" + pointsStrNew + ")" );

        pointsStrNew += points[3];
        //alert("points4 (" + pointsStrNew + ")" );

        obj.setAttribute("points", pointsStrNew );
    } else {
        alert("getElementById(" + id + ") failed");
    }
}

function MyClick(event)
{
    var target = event.target || event.srcElement;
    var id = target.id;
    //alert("MyClick: id (" + id + ");" );

    gEvent = event;

    setVariable(id);
}

function DisableCheckboxes ( newValue )
{
    var elements = document.getElementsByTagName("input");
    //alert("elements (" + elements + ")" );
    for (var i = 0; i < elements.length; i++ ) {
        //alert("element.type (" + elements[i].type + ")" );
        if (elements[i].type === "checkbox" ) {
            var name = elements[i].id;
            var ischecked = elements[i].checked; //check if checked
            //alert("checkbox: " + name + "; ischecked " + ischecked );
            elements[i].disabled = newValue;
        }
    }
}

function GetInputValue ( fieldName )
{
    var rc=0;
    var obj=document.getElementById( fieldName );
    if (obj) {
        rc = obj.value;
    }
    return rc;
}

function SetInputValue ( fieldName, newValue )
{
    var rc=0;
    //alert("SetInput: name " + fieldName + "; newValue " + newValue );
    var obj=document.getElementById( fieldName );
    if (obj) {
        obj.value = newValue;
    }
    return rc;
}

function checkboxPerfDeepDoit( fieldValue )
{
    if ( fieldValue == true ) {
        // if previous timeout is already pending, cancel it
        if (CgiTimeoutId) { clearTimeout(CgiTimeoutId); }

        //alert("checkboxPerfDeepDoit: value " + fieldValue + "; GetPerfDeep.Value (" + GetPerfDeep.Value + ")" );
        SetInternalCheckboxStatus ( "checkboxPerfDeep", GetPerfDeep);
        GetPerfDeep.Duration = GetInputValue ( "PerfDeepDuration" );
        //alert("checkboxPerfDeepDoit done");
    }

    SetCheckboxDisabled ( "checkboxPerfTop", fieldValue );
    SetCheckboxDisabled ( "checkboxPerfCache", fieldValue );
}

function checkboxPerfCacheDoit( fieldValue )
{
    if ( fieldValue == true ) {
        // if previous timeout is already pending, cancel it
        if (CgiTimeoutId) { clearTimeout(CgiTimeoutId); }

        //alert("checkboxPerfCacheDoit: value " + fieldValue + "; GetPerfCache.Value (" + GetPerfCache.Value + ")" );
        SetInternalCheckboxStatus ( "checkboxPerfCache", GetPerfCache);
        GetPerfCache.Duration = GetInputValue ( "PerfCacheDuration" );
        //alert("checkboxPerfCacheDoit done");
    }

    SetCheckboxDisabled ( "checkboxPerfTop", fieldValue );
    SetCheckboxDisabled ( "checkboxPerfDeep", fieldValue );
}

function GetPerfFlameSetState( newstate )
{
    //alert("GetPerfFlameSetState:" + newstate );
    GetPerfFlame.State = newstate;
    var objstate = document.getElementById('PerfFlameState');
    if (objstate) {
        if (newstate == GetPerfFlameState.INIT) {
            objstate.innerHTML = "INIT";
        } else if (newstate == GetPerfFlameState.IDLE) {
            objstate.innerHTML = "IDLE";
        } else if (newstate == GetPerfFlameState.START) {
            objstate.innerHTML = "START";
        } else if (newstate == GetPerfFlameState.RECORDING) {
            objstate.innerHTML = "RECORDING";
        } else if (newstate == GetPerfFlameState.STOP) {
            objstate.innerHTML = "STOP";
        } else if (newstate == GetPerfFlameState.CREATESCRIPTOUT) {
            objstate.innerHTML = "CREATESCRIPTOUT";
        } else if (newstate == GetPerfFlameState.GETSVG) {
            objstate.innerHTML = "GETSVG";
        } else if (newstate == GetPerfFlameState.DELETEOUTFILE) {
            objstate.innerHTML = "DELETEOUTFILE";
        } else {
            objstate.innerHTML = "UNKNOWN";
        }
    }
}

function GetPerfFlameSetStop()
{
    GetPerfFlameSetState( GetPerfFlameState.STOP ); // Stop recording and send back report
    var objButton = document.getElementById('PerfFlameStartStop');
    if (objButton) {
        objButton.value = "Start";
    }
    objButton = document.getElementById('checkboxPerfFlame');
    if (objButton) {
        objButton.disabled = false;
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

            if (fieldName == "checkboxcpus" ) {
                SetInternalCheckboxStatus ( "checkboxcpus", GetCpuInfo );
                hideOrShow("row_cpus", fieldValue);
            } else if (fieldName == "checkboxnets" ) {
                SetInternalCheckboxStatus ( "checkboxnets", GetNetStats );
                hideOrShow("row_net_stats", fieldValue);
            } else if (fieldName == "checkboxirqs" ) {
                SetInternalCheckboxStatus ( "checkboxirqs", GetIrqInfo );
                hideOrShow("row_irqs", fieldValue);
            } else if (fieldName == "checkboxheapstats" ) {
                SetInternalCheckboxStatus ( "checkboxheapstats", GetHeapStats );
            } else if (fieldName == "checkboxsatausb" ) {
                SetInternalCheckboxStatus ( "checkboxsatausb", GetSataUsb );
                if (GetSataUsb.Value == 0) { // if user un-checked the box, tell server to stop collecting data
                    GetSataUsb.Stop = 1;
                }
            } else if (fieldName == "checkboxprofiling" ) {
                GetProfiling.Value = fieldValue;

                // when Profiling checkbox is first checked, default PerfTop checkbox to on also
                GetPerfTop.Value = fieldValue;

                // when hiding profiling, tell server to stop all data collection that might be going on
                if (fieldValue == false) {
                    GetPerfTop.Value = false;
                    GetLinuxTop.Value = false;
                    GetLinuxTop.Stop = 1;
                    GetPerfDeep.StartReportNow = false;
                    GetPerfDeep.Value = 0;
                    GetContextSwitch.Value = false;
                    //AddToDebugOutput ( "checkboxprofiling: PerfCache.Start (" + GetPerfCache.StartReportNow + "; PerfDeep.Start (" + GetPerfDeep.StartReportNow + "); LinuxTop.Stop (" + GetLinuxTop.Stop + ")" );
                }
                SetInternalCheckboxStatus ( "checkboxprofiling", GetProfiling);
                hideOrShow("row_profiling", fieldValue);
            } else if (fieldName == "checkboxmemory" ) {
                GetMemory.Value = fieldValue;
                SetInternalCheckboxStatus ( "checkboxmemory", GetMemory);
                // if the box is being unchecked, clear out the html so that it does not display when we check the box in the future
                if ( fieldValue == false ) {
                    var objmemory = document.getElementById("MEMORY_HTML");
                    if (objmemory) { objmemory.innerHTML = ""; }

                    // when hiding, tell server to stop all data collection that might be going on
                    GetPerfCache.StartReportNow = false;
                    //AddToDebugOutput ( "checkboxprofiling: PerfCache.Start (" + GetPerfCache.StartReportNow + "; PerfDeep.Start (" + GetPerfDeep.StartReportNow + "); LinuxTop.Stop (" + GetLinuxTop.Stop + ")" );
                } else { // we are activating the Memory checkbox
                    GetPerfCache.Value = 0;
                    SetCheckboxStatus ( "checkboxPerfCache", GetPerfCache );
                    SetCheckboxDisabled ( "checkboxPerfCache", false );
                    GetSataUsb.Value = 0;
                    SetCheckboxStatus ( "checkboxsatausb", GetSataUsb );
                    SetCheckboxDisabled ( "checkboxsatausb", false );
                    GetHeapStats.Value = 0;
                    SetCheckboxStatus ( "checkboxheapstats", GetHeapStats );
                    SetCheckboxDisabled ( "checkboxheapstats", false );
                }
                hideOrShow("row_memory", fieldValue);
            } else if (fieldName == "checkboxPerfTop" ) {
                GetPerfTop.Value = fieldValue;
                if (fieldValue) { // if turning on, turn others off
                    GetLinuxTop.Value = 0;
                    GetPerfCache.Value = 0;
                    GetPerfDeep.Value = 0;
                }
                GetLinuxTop.Stop = 1;
                //alert("PerfTop (" + GetPerfTop.Value + "); LinuxTop (" + GetLinuxTop.Value + ")" );
                SetInternalCheckboxStatus ( "checkboxPerfTop", GetPerfTop);
                SetCheckboxDisabled ( "checkboxPerfDeep", fieldValue );
                SetCheckboxDisabled ( "checkboxPerfCache", fieldValue );
                SetCheckboxDisabled ( "checkboxLinuxTop", fieldValue );
            } else if (fieldName == "checkboxLinuxTop" ) {
                GetLinuxTop.Value = fieldValue;
                if (fieldValue) { // if turning on, turn others off
                    GetPerfTop.Value = 0;
                    GetPerfDeep.Value = 0;
                }
                //alert("PerfTop 1 (" + GetPerfTop.Value + "); LinuxTop (" + GetLinuxTop.Value + ")" );
                SetInternalCheckboxStatus ( "checkboxLinuxTop", GetLinuxTop);
                SetCheckboxDisabled ( "checkboxPerfDeep", fieldValue );
                SetCheckboxDisabled ( "checkboxPerfCache", fieldValue );
                SetCheckboxDisabled ( "checkboxPerfTop", fieldValue );

                if (fieldValue == false) { // if we are turning off LinuxTop, tell server to stop collecting data
                    GetLinuxTop.Stop = 1;
                }
            } else if (fieldName == "checkboxPerfCache" ) {
                var objtable = document.getElementById("MEMORY_HTML");
                GetPerfCache.Value = fieldValue;
                checkboxPerfCacheDoit( fieldValue );
                if(debug) alert(fieldName + " checked; value " + fieldValue );
                GetPerfCache.StartReportNow = fieldValue;

                if (objtable) { objtable.innerHTML = "<textarea style=\"width:860px;\" ></textarea>"; } // turning on and off ... clear out the textarea

                if (fieldValue) { // if turning on
                } else { // else turning it off
                }
            } else if (fieldName == "checkboxPerfDeep" ) {
                GetPerfDeep.Value = fieldValue;
                checkboxPerfDeepDoit( fieldValue );
                if(debug) alert(fieldName + " checked; value " + fieldValue );
                GetPerfDeep.StartReportNow = fieldValue;
                if (fieldValue) { // if turning on, turn others off
                    GetPerfCache.Value = 0;
                    GetPerfTop.Value = 0;
                    GetLinuxTop.Value = 0;
                }
                SetCheckboxDisabled ( "checkboxLinuxTop", fieldValue );
                SetCheckboxDisabled ( "checkboxPerfCache", fieldValue );
                SetCheckboxDisabled ( "checkboxPerfTop", fieldValue );
            } else if (fieldName == "checkboxContextSwitch" ) {
                SetInternalCheckboxStatus ( "checkboxContextSwitch", GetContextSwitch );
                hideOrShow("row_profiling_html", fieldValue);
            } else if (fieldName == "checkboxPerfFlame" ) {

                // when hiding flame graphs, tell server to stop all data collection that might be going on
                if (fieldValue == false) {
                    if ( GetPerfFlame.State == GetPerfFlameState.IDLE ) { // if the flame graph is not recording presently
                        GetPerfFlame.Value = false;
                        GetPerfFlameSetStop();
                        SetInternalCheckboxStatus ( "checkboxPerfFlame", GetPerfFlame);
                        hideOrShow("row_PerfFlame", fieldValue);
                        var objspan = document.getElementById('PerfFlameContents');
                        if (objspan) {
                            objspan.innerHTML = ""; // clear out the contents element so old stuff does not display when we check the box in the future
                        }
                    } else {
                        alert("Please stop any recording before hiding the Flame Graph.");
                    }
                } else {
                    GetPerfFlame.Value = true;
                    GetPerfFlameSetState( GetPerfFlameState.INIT ); // Init
                    SetInternalCheckboxStatus ( "checkboxPerfFlame", GetPerfFlame);
                    hideOrShow("row_PerfFlame", fieldValue);
                }
            }

            if (obj.checked) {
                //alert("calling sendCgiRequest");
                sendCgiRequest();
            }
        } else if ( obj.type == "text" ) {
            fieldValue = obj.value;
        } else if ( obj.type == "radio" ) {
        } else if ( obj.type == "button" ) {
            //alert("button:" + fieldName);
            if (fieldName == "PerfFlameStartStop") {
                if (GetPerfFlame.State == GetPerfFlameState.IDLE) { // state is Idle
                    var objCmdLine = document.getElementById('PerfFlameCmdLine');
                    if (objCmdLine) {
                        if (objCmdLine.value.length > 0) {
                            GetPerfFlameSetState( GetPerfFlameState.START ); // Start (send over cmdline contents)
                            // change the text on the button from Start to Stop
                            var objButton = document.getElementById('PerfFlameStartStop');
                            if (objButton) {
                                objButton.value = "Stop";
                            }
                            var local = new Date();
                            GetPerfFlameRecordingSeconds = Math.floor(local.getTime() / 1000);
                            // disable the checkbox until the user stops the perf record
                            objButton = document.getElementById('checkboxPerfFlame');
                            if (objButton) {
                                objButton.disabled = true;
                            }
                            // clear out the SVG container
                            objButton = document.getElementById('PerfFlameSvg');
                            if (objButton) {
                                objButton.innerHTML = "";
                            }
                        } else {
                            alert("The contents of the CmdLine box cannot be empty!");
                        }
                    }
                } else if (GetPerfFlame.State == GetPerfFlameState.RECORDING) { // state is Running/Recording
                    GetPerfFlameSetStop();
                }
            }
        } else if ( obj.type == "select-one" ) {
        } else if (fieldName.indexOf("ChangeCpuState") == 0 ) { // user clicked disable/enable CPU
            ChangeCpuState = fieldName;
            var iconobj = document.getElementById(fieldName);
            var iconfill = "unknown";
            if (iconobj) {
                var temp;

                iconfill = iconobj.getAttribute('fill');
                // Based on the current color of the circle that was clicked, we will either be enabling or disabling the specified CPU.
                // A green (lime) circle means we want the CPU to go active; a red circle means we want the CPU to stop.
                if ( iconfill.indexOf("lime") == -1 ) {
                    temp = ChangeCpuState.replace(/ChangeCpuState/, "ChangeCpuState=-");
                } else {
                    temp = ChangeCpuState.replace(/ChangeCpuState/, "ChangeCpuState=+");
                }
                ChangeCpuState = temp;
            }
            //alert("Checked: (" + fieldName + "); src (" + iconfill + "); ChangeCpuState (" + ChangeCpuState + "); indexOf(lime)=" + iconfill.indexOf("lime") );
            sendCgiRequest();
        }

        if (fieldName == "h1bsysperf") {
            MasterDebug = 1-MasterDebug;
            //alert("MasterDebug " + MasterDebug + "; objdebug " + objdebug );
            if (MasterDebug==1) {
                if (objdebug) {
                    objdebug.style.visibility = "";
                    GlobalDebug = 0;
                }
            } else {
                if (objdebug) {
                    objdebug.style.visibility = "hidden";
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

function sendCgiRequestDoItNow( url )
{
    var debug=0;

    if (debug==1) alert("sending len (" + url.length + ") (" + url + ")" );

    xmlhttp=new XMLHttpRequest();
    xmlhttp.onreadystatechange= serverHttpResponse;
    xmlhttp.open("GET",url,true);
    xmlhttp.send(null);

    CgiCount++;
    var CgiCountObj = document.getElementById('cgicount');
    if ( MasterDebug && CgiCountObj) {
        CgiCountObj.innerHTML = "&nbsp;&nbsp;(" + CgiCount + "," + SetVariableCount + "," + ResponseCount + ")";
    }
}

function sendCgiRequest( )
{
    var debug=0;

    var idx=1;
    var url = "";

    // if previous timeout is already pending, cancel it
    if (CgiTimeoutId) { clearTimeout(CgiTimeoutId); }

    var RandomValue = randomIntFromInterval(1000000,9999999);
    url = "/cgi/bsysperf.cgi?randomvalue=" + RandomValue;

    if (epochSeconds > 0) {
        url += "&datetime=" + epochSeconds + "&tzoffset=" + tzOffset;
        epochSeconds = 0;
    }

    if (GetCpuInfo.Value == true) {
        url += "&cpuinfo=1";
    }

    if (GetIrqInfo.Value == true) {
        url += "&irqinfo=1";
    }

    if (GetNetStats.Value == true) {
        url += "&netstats=1";
    }

    if (GetHeapStats.Value == true) {
        url += "&heapstats=1";
    }

    if (GetSataUsb.Value == true) {
        url += "&satausb=1";
    }

    if (GetSataUsb.Stop == 1) {
        url += "&satausb=2";
        GetSataUsb.Stop = 0;
    }

    if (GetProfiling.Value == true) {
        url += "&profiling=1";
        GetProfiling.Value = false;
    }

    if (GetMemory.Value == true) {
        url += "&memory=1";
        GetMemory.Value = false;
    }

    if (GetLinuxTop.Stop == true) {
        url += "&LinuxTop=2";
        AddToDebugOutput ( "sendCgiRequest: LinuxTop.Stop (" + GetLinuxTop.Stop + ")" );
        GetLinuxTop.Stop = 0;
    } else if (GetLinuxTop.Value == true) {
            url += "&LinuxTop=1";
    }

    if (GetContextSwitch.Value == true ) {
            url += "&ContextSwitch=1";
    }

    // if profiling is displayed and kernel has been compiled with perf enabled
    if (GetCheckboxStatus ( "checkboxprofiling" ) && PerfError == false) {

        //alert("sendCgi: GetPerfDeep.Value (" + GetPerfDeep.Value + ")" );
        if (GetPerfDeep.Value == true && GetPerfDeep.Duration > 0 && GetPerfDeep.StartReportNow == true ) {
            url += "&PerfDeep=" + GetPerfDeep.Duration;
            GetPerfDeep.StartReportNow = false;
        }

        //alert("sendCgi: GetPerfDeepResults.Value (" + GetPerfDeepResults.Value + ")" );
        if (GetPerfDeepResults.Value == true ) {
            url += "&PerfDeepResults=1";
            GetPerfDeepResults.Value = false;
        }

        if (GetPerfTop.Value == true) {
            url += "&PerfTop=1";
            //GetPerfTop.Value = 0; // changed so that the user must un-check the box to get PerfTop to stop updating
        }
    }

    if(debug) alert("checkboxmemory (" + GetCheckboxStatus ( "checkboxmemory" ) + "); PerfError (" + PerfError + ")" );
    // if memory row is displayed and kernel has been compiled with perf enabled
    if (GetCheckboxStatus ( "checkboxmemory" ) && (PerfError == false) ) {

        if(debug) alert("sendCgi: GetPerfCache.Value (" + GetPerfCache.Value + ")" );
        if (GetPerfCache.Value == true && GetPerfCache.Duration > 0 && GetPerfCache.StartReportNow == true ) {
            url += "&PerfCache=" + GetPerfCache.Duration;
            GetPerfCache.StartReportNow = false;
        }

        if(debug) alert("sendCgi: GetPerfCacheResults.Value (" + GetPerfCacheResults.Value + ")" );
        if (GetPerfCacheResults.Value == true ) {
            if(debug) alert("sendCgi: GetPerfCache.Value (" + GetPerfCache.Value + ")" );
            url += "&PerfCacheResults=1";
            GetPerfCacheResults.Value = false;
        }
    }

    // if flame is displayed and kernel has been compiled with perf enabled
    if (GetCheckboxStatus ( "checkboxPerfFlame" ) && PerfError == false ) {

        // states are: Init:1, Idle:2, Start:3, Recording:4, Stop:5
        if (GetPerfFlame.State != GetPerfFlameState.IDLE ) { // if NOT idle
            url += "&PerfFlame=" + GetPerfFlame.State;

            if (GetPerfFlame.State == GetPerfFlameState.START) {
                var objCmdLine = document.getElementById('PerfFlameCmdLine');
                if (objCmdLine) {
                    url += "&PerfFlameCmdLine=" + encodeURIComponent(objCmdLine.value);
                    GetPerfFlameSetState( GetPerfFlameState.RECORDING );
                }
                GetPerfFlameSvgCount++;
            }

            if (GetPerfFlame.State == GetPerfFlameState.DELETEOUTFILE) {
                url += "&PerfFlameSvgCount=" + GetPerfFlameSvgCount;
                url += "&perf_out=" + PerfRecordUuid;
            }

            if (GetPerfFlame.State == GetPerfFlameState.CREATESCRIPTOUT) {
                url += "&perf_out=" + PerfRecordUuid;
            }
        }

        if (GetPerfFlame.State == GetPerfFlameState.INIT ) {
            GetPerfFlameSetState( GetPerfFlameState.IDLE );
        }
    } else if (GetPerfFlame.State == GetPerfFlameState.STOP) { // if we hid the flame graph window while a record was in progress
        url += "&PerfFlame=" + GetPerfFlame.State;
        GetPerfFlameSetState( GetPerfFlameState.IDLE );
    }

    if (ChangeCpuState.length > 0) {
        url += "&" + ChangeCpuState;
        //alert("sendCgiRequest: ChangeCpuState (" + ChangeCpuState + ")" );
        ChangeCpuState = "";
    }

    // seems the data sent to the browser cannot exceed 1010 for unknown reason
    if (url.length > 1010) {
        url = url.substr(0,1010);
    }

    urlSentPreviously = url;

    sendCgiRequestDoItNow ( url );
}

function MsIeCrFix ( newstring )
{
    //if (userAgent.indexOf("MSIE") >= 0 ) { // for ie9, must replace carriage returns with <br>
    //    return newstring.replace(/\n/g, "<br>");
    //}
    return newstring;
}

function AddToDebugOutput ( newstring )
{
    var debug = 1;

    //alert("AddToDebug: objdebug " + objdebug + "); newstring (" + newstring + ")" );
    if (debug && objdebug) {
        objdebug.innerHTML += MsIeCrFix ( newstring );
    }
}

function RetryLinuxTop()
{
    //alert("RetryLinuxTop: GetLinuxTop.Value (" + GetLinuxTop.Value + ")" )
    if (GetLinuxTop.Value) {
        sendCgiRequest();
    }
}

function GetPerfDeepResultsDoit()
{
    //alert("GetPerfDeepResultsDoit: countdown " + GetPerfDeepCountdown );
    if ( GetPerfDeepCountdown > 1) {
        GetPerfDeepCountdown--;
        //alert( "GetPerfDeepResultsDoit: duration " + GetPerfDeepCountdown );
        setTimeout ('GetPerfDeepResultsDoit()', 1000 );
    } else {
        GetPerfDeepResults.Value = true;
        sendCgiRequest();
    }
}

function GetPerfCacheResultsFunc()
{
    //alert("GetPerfCacheResultsFunc: countdown " + GetPerfCacheCountdown );
    if ( GetPerfCacheCountdown > 1) {
        GetPerfCacheCountdown--;
        //alert( "GetPerfCacheResultsFunc: duration " + GetPerfCacheCountdown );
        setTimeout ('GetPerfCacheResultsFunc()', 1000 );
    } else {
        GetPerfCacheResults.Value = true;
        sendCgiRequest();
    }
}

function ComputeNetBytes10SecondsAverage ( InterfaceIndex, RxTxIndex )
{
    var averageMbps     = 0;
    var secs            = 0;
    var cummulativeMbps = 0;
    var precision       = 0;
    var spacer          = "";

    if (objdebug && InterfaceIndex == 1 ) objdebug.innerHTML += "ComputeAverage: num secs " + NetBytesRx10SecondsCount[InterfaceIndex] + ": ";
    for(secs=0; secs<NetBytesRx10SecondsCount[InterfaceIndex]; secs++) {
        spacer = "";
        if (RxTxIndex == 0) {
            cummulativeMbps += NetBytesRx10Seconds[InterfaceIndex][secs];
            if (objdebug && InterfaceIndex == 1 ) if (NetBytesRx10Seconds[InterfaceIndex][secs].toFixed(0) < 10 ) { spacer = " "; }
            if (objdebug && InterfaceIndex == 1 ) objdebug.innerHTML += "[" + secs + "]=" + spacer + NetBytesRx10Seconds[InterfaceIndex][secs].toFixed(2) + ", ";
        } else {
            if (objdebug && InterfaceIndex == 1 ) if (NetBytesTx10Seconds[InterfaceIndex][secs].toFixed(0) < 10 ) { spacer = " "; }
            cummulativeMbps += NetBytesTx10Seconds[InterfaceIndex][secs];
            if (objdebug && InterfaceIndex == 1 ) objdebug.innerHTML += "[" + secs + "]=" + spacer + NetBytesTx10Seconds[InterfaceIndex][secs].toFixed(2) + ", ";
        }
    }
    if ( NetBytesRx10SecondsCount[InterfaceIndex] > 0) {
        averageMbps = Number ( cummulativeMbps / NetBytesRx10SecondsCount[InterfaceIndex] );
    }
    if (objdebug && InterfaceIndex == 1 ) if (averageMbps.toFixed(0) < 10 ) { spacer = " "; }
    if (objdebug && InterfaceIndex == 1 ) objdebug.innerHTML += "; avg=" + spacer + averageMbps.toFixed(2) + "\n";
    // if the average is less than one Mbps, increase the decimal precision to show kilobits
    if (averageMbps.toFixed(0) == 0) {
        precision = 2;
    }
    return averageMbps.toFixed(precision);
}

// This function will insert SVG code into the appropriate HTML structure to either draw a red or green
// circle. The circle will be clicked to enable or disable the CPU.
function ChangeCpuTag ( idx, idle )
{
    var CpuObj = document.getElementById('ChangeCpuTag' + idx );
    if (CpuObj && idx > 0) {
        if ( idle == 255 ) {
            CpuObj.innerHTML = "<svg height=20 width=20 ><circle cx=10 cy=10 r=10 onclick=\"MyClick(event);\" id=ChangeCpuState" + idx + " fill=\"lime\" /></svg>";
        } else {
            CpuObj.innerHTML = "<svg height=20 width=20 ><circle cx=10 cy=10 r=10 onclick=\"MyClick(event);\" id=ChangeCpuState" + idx + " fill=\"red\" /></svg>";
        }
        //alert("for CPU " + idx + ", html is (" + CpuObj.innerHTML + ")" );
    }
}

function sendCgiRequestRetry ( )
{
    // send the previously attempted url request
    AddToDebugOutput ( "Retry url: " + urlSentPreviously + eol );
    sendCgiRequestDoItNow( urlSentPreviously );
}

function ComputeTotalCpuLongAverage()
{
    var idx=0;
    var idxMax=cpuUsageLongAverageCount;
    var totalUtilization = 0;
    var averageUtilization = 0;


    if (cpuUsageLongAverageCount) {
        if ( cpuUsageLongAverageCount >= CPU_USAGE_LONG_AVERAGE_MAX ) idxMax = CPU_USAGE_LONG_AVERAGE_MAX;
        for (idx=0; idx<idxMax; idx++) {
            totalUtilization += Number ( cpuUsageLongAverage[idx] );
        }
        averageUtilization = Number ( totalUtilization / idxMax ).toFixed(1);
    }

    return averageUtilization;
}

// This function runs after the SVG iframe has been loaded with the contents from the Perl script.
// It gives us a chance to do any follow-up work once the SVG had rendered.
function iframeLoad ()
{
    GetPerfFlameSetState( GetPerfFlameState.DELETEOUTFILE );

    //alert("iframeLoad");
    //var iframe = document.getElementById('iframe123');
}

function GetSvgContents ()
{
    var ipaddr = window.location.hostname;
    var perl_server = 'http://home.irv.broadcom.com/~detrick';
    var fg_script = 'fg_generator.cgi'
    var perf_out = PerfRecordUuid + '.out';
    var url = perl_server + '/' +fg_script+ '?ipaddr=' + ipaddr + '&perf_out=' + perf_out;
    var targetDiv = document.getElementById('PerfFlameSvg'); // div002

    if ( targetDiv ) {
        targetDiv.innerHTML = "";
        var newFrame = document.createElement("iframe");
        newFrame.setAttribute("src", url );
        newFrame.setAttribute("id", "iframe123" );
        newFrame.setAttribute("onload", "iframeLoad()" );
        newFrame.style.width="100%";
        newFrame.style.height="800px";
        newFrame.style.border="thick solid #ffffff"; // green #00cc00
        targetDiv.appendChild(newFrame);
    }
}

// This function runs as an asynchronous response to a previous server request
function serverHttpResponse ()
{
    var debug=GlobalDebug;
    var CgiCountObj = document.getElementById('cgicount');
    var idx=0;
    var idx2;

    if (debug) alert("serverHttpResponse: got readyState " + xmlhttp.readyState );

    if (xmlhttp.readyState==4 ) {
        //alert("serverHttpResponse: got readyState " + xmlhttp.readyState + "; status " + xmlhttp.status );

        // only if "OK"
        if (xmlhttp.status == 200) {

            debug=0;

            urlSentSuccessfully++; // used to try to determine if a failure is intermittant or we have never been successful
            urlSentRetryCountAfterSuccess = 0;

            var responseText1 = xmlhttp.responseText.replace(/</g,"&lt;"); // fails on ie7, safari
            var responseText2 = responseText1.replace(/</g,"&lt;"); // fails on ie7, safari

            if (userAgent.indexOf("MSIE") >= 0 ) { // for ie9, must replace carriage returns with <br>
                eol = "<br>";
            }

            //if(debug) alert("setting debugdiv");
            //if(objdebug) objdebug.innerHTML = responseText2;

            if (debug) alert("rtrim");
            var responseText = rtrim(xmlhttp.responseText);

            if (debug) alert("responseText:len " + responseText.length + "(" + responseText + ")" );
            if (debug) alert("split");
            var oResponses = responseText.split( "~" );
            if (debug) alert("num responses is " + oResponses.length );

            // sometimes the very first response is blank; if this happens, send another request very soon after we receive the blank response
            if (responseText.length == 0) {
                if (debug) alert("response is empty; calling setTimeout");
                ResponseCount++;
                CgiTimeoutId = setTimeout ('OneSecond()', REFRESH_IN_MILLISECONDS/10 );
                AddToDebugOutput ( "calling setTimeout(); ID (" + CgiTimeoutId + ")" + eol );
            } else {

                if (objdebug) objdebug.innerHTML = ""; // clear out any previous entries

                AddToDebugOutput ( "urlSentPreviously: " + urlSentPreviously + eol );
                AddToDebugOutput ( "urlSentSuccessfully: " + urlSentSuccessfully + "; urlSentRetryCount: " +
                                    urlSentRetryCount + "; urlSentRetryCountAfterSuccess: " + urlSentRetryCountAfterSuccess+ eol );

                var row_profiling_html = document.getElementById("row_profiling_html");
                var objPerfTop = document.getElementById("PERFUTILS");
                if (row_profiling_html) {
                    row_profiling_html.innerHTML = ""; // clear out the row
                }
                //alert("for i = 0 to " + oResponses.length );
                // loop through <response> elements, and add each nested
                for (var i = 0; i < oResponses.length; i++) {
                    var entry = oResponses[i];
                    if ( debug==1 && entry.length>1 ) alert("Entry " + entry + "; len " + entry.length );
                    if ( entry == "ALLDONE" ) {
                        AddToDebugOutput ( entry + eol );
                        if (debug==1) alert("response: calling sendCgiRequest()");
                        ResponseCount++;
                        CgiTimeoutId = setTimeout ('OneSecond()', REFRESH_IN_MILLISECONDS );
                        //AddToDebugOutput ( entry + ": calling setTimeout(); ID (" + CgiTimeoutId + ")"  + eol );
                    } else if (entry == "FATAL") {
                        AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
                        alert("FATAL ... " + oResponses[i+1]);
                        i++;
                    } else if (entry == "VERSION") {
                        AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
                        i++;
                    } else if (entry == "STBTIME") {
                        //AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
                        var obj2=document.getElementById("stbtime");
                        if (obj2) {
                            if (debug) alert("setting stbtime to " + oResponses[i+1] );
                            obj2.innerHTML = oResponses[i+1];
                            i++;
                        } else {
                            alert("id=stbtime not found");
                        }
                    } else if (entry == "CPUPERCENTS") {
                        AddToDebugOutput ( entry + ": len of entry " + oResponses[i+1].length + eol );
                        var obj2=document.getElementById("CPUPERCENTS");
                        if (obj2) {
                            obj2.innerHTML = oResponses[i+1];
                            //alert("CPUPERCENTS (" + obj2.innerHTML + ")" );
                            i++;
                        } else {
                            alert("id=CPUPERCENTS not found");
                        }
                    } else if (entry == "CPUINFO") { // CPUINFO:4  99.97 100.0  100.0  100.0   99.98    0.1  ;
                        var objCheckboxCpus = document.getElementById("checkboxcpus");
                        if (objCheckboxCpus) {
                            // if the checkbox to show CPU utilization is checked
                            if (objCheckboxCpus.checked) {
                                var response = oResponses[i+1];
                                var cpupercentage = 0;
                                var tempNumCpus = 0;
                                var cpuUsageTotal = 0; // the four CPU usage's combined; used to compute average CPU usage
                                var cpuActiveCount = 0;

                                AddToDebugOutput ( entry + ":" + response + ";" + eol );

                                // if the number of cpus has changed
                                tempNumCpus = Number(response.substr(0,1)) + 1;
                                if (tempNumCpus && tempNumCpus != gNumCpus ) {
                                    var numlines = 0;
                                    gNumCpus = tempNumCpus;
                                    //alert("CPUINFO: gNumCpus " + gNumCpus );
                                    for (numlines=1; numlines<(gNumCpus-1)+1 ; numlines++) { //once for all CPUs
                                        showRow(numlines);
                                    }
                                }

                                for (idx=0; idx < gNumCpus; idx++ ) {
                                    var idle=response.substr(2+(idx*7), 7);

                                    // if the string has some length of characters to parse
                                    //alert("idx " + idx + "; idle " + idle + "; len " + idle.length );
                                    if (idle.length) {
                                        cpupercentage = Math.floor(idle); // each entry is 6 chars long ... CPUINFO:4 000100 000099 000255 000100
                                        gCpuData[idx] = cpupercentage;
                                        var cpuid="cpudata0" + idx;
                                        //alert("cpu " + idx + "=(" + cpupercentage + "); idle (" + idle + "); cpuid " + cpuid + "; gNumCpus " + gNumCpus );
                                        var cpudataobj = document.getElementById(cpuid);
                                        if (cpudataobj) {
                                            var cputitleid = "cputitle0" + idx;
                                            var objtitle = document.getElementById(cputitleid);

                                            if (cpudataobj.innerHTML.length > 100) {
                                                //cpudataobj.innerHTML = cpudataobj.innerHTML.substr(0,)
                                            }
                                            ChangeCpuTag ( idx, idle );
                                            if (idle == 255) { // if the cpu has been disabled
                                                //cpudataobj.innerHTML += "255 ";
                                                // add inactive to the title
                                                if (objtitle) {
                                                    objtitle.innerHTML = "CPU " + idx + "&nbsp;&nbsp;(<span style=\"background-color:red;\">(INACTIVE)</span>)";
                                                }
                                            } else {
                                                var usage = Number(100 - idle);
                                                cpudataobj.innerHTML += usage + " ";

                                                if (objtitle) {
                                                    objtitle.innerHTML = "CPU " + idx + "&nbsp;&nbsp;(&nbsp;" + usage + "%&nbsp;)";
                                                }

                                                cpuUsageTotal += Number(usage);
                                                cpuActiveCount++;
                                            }

                                            cpudataobj.scrollTop = cpudataobj.scrollHeight;
                                            if (bCpuDataHeightSet == 0) {
                                                cpudataobj.style.height ='auto';
                                                cpudataobj.style.height ="18px";
                                            }
                                        }  // if cpudata00 is valid
                                    } // if idle.length is valid
                                }  // for each CPU

                                cpuUsageLongAverage[cpuUsageLongAverageIdx] = (cpuUsageTotal / cpuActiveCount);
                                cpuUsageLongAverageIdx++; // increment index into array ... wrapping around to 0 if it gets too big
                                if ( cpuUsageLongAverageIdx >= CPU_USAGE_LONG_AVERAGE_MAX ) cpuUsageLongAverageIdx = 0;
                                cpuUsageLongAverageCount++; // Count is used to only compute average for 2 cycles if we have only been collecting values for 2 seconds

                                // update the total average CPU usage
                                var objcpuoverall = document.getElementById("cpuoverall");
                                if (objcpuoverall) {
                                    objcpuoverall.innerHTML = Number( cpuUsageTotal / cpuActiveCount ).toFixed(1) + "%";
                                    var avg = ComputeTotalCpuLongAverage();
                                    objcpuoverall.innerHTML += "&nbsp;<span style=\"color:limegreen;\">(" + CPU_USAGE_LONG_AVERAGE_MAX + "s&nbsp;avg:" + avg + "%)</span>";
                                    objcpuoverall.innerHTML += "<span style=\"font-size:8pt;\" >&nbsp;TOTAL&nbsp;CPU</span>";

                                    // if there are two or more CPUs, update the red overall average line and the limegreen 5-second cpu average line
                                    if (gNumCpus > 1) {
                                        AppendPolylineXY ( 0, 100 - Number( cpuUsageTotal / cpuActiveCount ) ); // update red CPU utilization average line
                                        AppendPolylineXY ( gNumCpus, 100 - Number( avg ) ); // line number needs to match the polyline0x definition in bsysperf.c
                                                                                            // update green 5-second CPU utilization average line
                                    }
                                }

                                bCpuDataHeightSet = 1;
                            } else {
                                AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                            }
                        }
                        i++;
                    } else if (entry == "ETHINFO") {
                        var response = oResponses[i+1];
                        AddToDebugOutput ( entry + ":" + response + ";" + eol );
                        i++;
                    } else if (entry == "IRQDETAILS") {
                        var response = oResponses[i+1];
                        var obj2=document.getElementById("IRQDETAILS");
                        if (obj2) {
                            obj2.innerHTML = oResponses[i+1];
                        }
                        i++;
                    } else if (entry == "MEMORY") {
                        var obj2 = document.getElementById("checkboxmemory");
                        if (obj2) {
                            if (obj2.checked) {
                                var response = oResponses[i+1];
                                var obj2=document.getElementById("MEMORY");
                                if (obj2) {
                                    obj2.innerHTML = oResponses[i+1];
                                }
                                validResponseReceived = 1;
                                hideOrShow("row_memory", true );
                            } else {
                                AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                            }
                        }
                        i++;
                    } else if (entry == "NETSTATS") {
                        var obj2 = document.getElementById("checkboxnets");
                        if (obj2) {
                            if (obj2.checked) {
                                var response = oResponses[i+1];
                                var obj2=document.getElementById("NETSTATS");
                                if (obj2) {
                                    obj2.innerHTML = oResponses[i+1];
                                }
                            } else {
                                AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                            }
                        }
                        i++;
                    } else if (entry == "NETBYTES") {
                        var obj2 = document.getElementById("checkboxnets");
                        var Mbps = 0;
                        if (obj2) {
                            if (obj2.checked) {
                                NetBytesSeconds++;

                                var response = oResponses[i+1];
                                AddToDebugOutput ( entry + ":" + response + "; NetBytesSeconds " + NetBytesSeconds + eol );
                                var oRxTxPairs = response.split( "," ); // split the response using comma delimiter

                                for (var idx = 0; idx < oRxTxPairs.length; idx++) {
                                    //AddToDebugOutput ( "NetIF " + idx + "str(" + oRxTxPairs[idx] + ")" + eol );
                                    if (idx < 10 && oRxTxPairs[idx].length >= 3 ) { // if we haven't exceeded array size and rxtx pair has some values
                                        var oRxTx = oRxTxPairs[idx].split( " " ); // split the response using space as delimiter
                                        var tagRx = "netif" + idx + "rx";
                                        var objRx = document.getElementById(tagRx);
                                        var tagTx = "netif" + idx + "tx";
                                        var objTx = document.getElementById(tagTx);

                                        // DEBUG: only interested in seeing gphy numbers
                                        if ( 0 && idx == 1){
                                            if (objdebug) {
                                                objdebug.innerHTML += entry + ": oRxTx[RX] " + oRxTx[0] + " > NetBytesPrev[" + idx + "][RX]=" + NetBytesPrev[idx][0] + "\n";
                                                objdebug.innerHTML += entry + ": RX:";
                                                for (idx2=0; idx2<NetBytesRx10SecondsCount[idx]; idx2++) {
                                                    objdebug.innerHTML += "[" + idx2 + "]" + NetBytesRx10Seconds[idx][idx2] + ", ";
                                                }
                                                objdebug.innerHTML += "\n\n";
                                            }
                                        }

                                        // instead of multiplying by 8 and then dividing by 1024 ... reduce it simply to dividing by 128
                                        Mbps =  Number( (oRxTx[0] - NetBytesPrev[idx][0]) / 128 / 1024);

                                        if (NetBytesPrev[idx][0] > 0 && (oRxTx[0] >= NetBytesPrev[idx][0]) && (NetBytesSeconds > 1) && (Mbps > 0) ) {
                                            // if the array has filled up
                                            if (NetBytesRx10SecondsCount[idx] == 10) {
                                                for (idx2=0; idx2<(NetBytesRx10SecondsCount[idx]-1); idx2++) {
                                                    NetBytesRx10Seconds[idx][idx2] = NetBytesRx10Seconds[idx][idx2+1];
                                                }
                                                NetBytesRx10SecondsCount[idx] = 9;
                                            }
                                            NetBytesRx10Seconds[idx][NetBytesRx10SecondsCount[idx]] = Mbps;
                                            NetBytesRx10SecondsCount[idx]++;
                                            if (objRx) {
                                                // NetBytesSeconds is subtracted by one because we need at least two-second's worth of data to compute a delta
                                                // convert bytes to megabits
                                                // instead of multiplying by 8 and then dividing by 1024 ... reduce it simply to dividing by 128
                                                objRx.innerHTML = Mbps.toFixed(0) + "&nbsp;&nbsp;(" + ComputeNetBytes10SecondsAverage ( idx, 0 ) + ")"; // 0 for RX and 1 for TX
                                            }
                                        } else {
                                            objRx.innerHTML = 0;
                                        }
                                        NetBytesPrev[idx][0] = oRxTx[0];

                                        // DEBUG: only interested in seeing gphy numbers
                                        if ( 0 && idx == 1){
                                            if (objdebug) {
                                                objdebug.innerHTML += entry + ": oRxTx[TX] " + oRxTx[1] + " > NetBytesPrev[" + idx + "][TX]=" + NetBytesPrev[idx][1] + "\n";
                                                objdebug.innerHTML += entry + ": TX:";
                                                for (idx2=0; idx2<NetBytesTx10SecondsCount[idx]; idx2++) {
                                                    objdebug.innerHTML += "[" + idx2 + "]" + NetBytesTx10Seconds[idx][idx2] + ", ";
                                                }
                                                objdebug.innerHTML += "\n\n";
                                            }
                                        }

                                        // instead of multiplying by 8 and then dividing by 1024 ... reduce it simply to dividing by 128
                                        Mbps =  Number( (oRxTx[1] - NetBytesPrev[idx][1]) / 128 / 1024);

                                        if (NetBytesPrev[idx][1] > 0 && (oRxTx[1] >= NetBytesPrev[idx][1]) && (NetBytesSeconds > 1) && (Mbps > 0) ) {
                                            // if the array has filled up, move the entries left one position and add new entry to the very end
                                            if (NetBytesTx10SecondsCount[idx] == 10) {
                                                for (idx2=0; idx2<(NetBytesTx10SecondsCount[idx]-1); idx2++) {
                                                    NetBytesTx10Seconds[idx][idx2] = NetBytesTx10Seconds[idx][idx2+1];
                                                }
                                                NetBytesTx10SecondsCount[idx] = 9;
                                            }
                                            NetBytesTx10Seconds[idx][NetBytesTx10SecondsCount[idx]] = Mbps;
                                            NetBytesTx10SecondsCount[idx]++;
                                            if (objTx) {
                                                // NetBytesSeconds is subtracted by one because we need at least two-second's worth of data to compute a delta
                                                // convert bytes to megabits
                                                // instead of multiplying by 8 and then dividing by 1024 ... reduce it simply to dividing by 128
                                                objTx.innerHTML = Mbps.toFixed(0) + "&nbsp;&nbsp;(" + ComputeNetBytes10SecondsAverage ( idx, 1 ) + ")"; // 0 for RX and 1 for TX
                                            }
                                        } else {
                                            objTx.innerHTML = 0;
                                        }
                                        NetBytesPrev[idx][1] = oRxTx[1];
                                    }
                                }
                            } else {
                                AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                            }
                        }
                        i++;
                    } else if (entry == "IRQINFO") {
                        var obj2 = document.getElementById("checkboxirqs");
                        if (obj2) {
                            if (obj2.checked) {
                                var response = oResponses[i+1];
                                var irqcount = 0;
                                var tempNumCpus = Number(response.substr(0,1)) + 1;
                                //alert("IRQINFO: tempNumCpus " + tempNumCpus );
                                AddToDebugOutput ( entry + ":" + response + "; tempNumCpus " + tempNumCpus + ";" + eol );
                                for (idx=0; idx < tempNumCpus; idx++ ) {
                                    var idle=response.substr(2+(idx*7), 7);

                                    // if the string has some length of characters to parse
                                    if (idle.length) {
                                        gIrqLatestData[idx] = idle;
                                        var irqid="irqdata0" + idx;
                                        //alert("irq " + idx + "; idle (" + idle + "); irqid " + irqid + "; tempNumCpus " + tempNumCpus );
                                        var irqdataobj = document.getElementById(irqid);
                                        if (irqdataobj) {
                                            var usage = Number(idle);
                                            irqdataobj.innerHTML += usage + " ";

                                            irqdataobj.scrollTop = irqdataobj.scrollHeight;
                                            if (bIrqDataHeightSet == 0) {
                                                irqdataobj.style.height ='auto';
                                                irqdataobj.style.height ="18px";
                                            }
                                        }
                                    }
                                }
                                bIrqDataHeightSet = 1;
                            } else {
                                AddToDebugOutput ( entry + ":ignored because checkbox is not checked" + eol );
                            }
                        }
                        i++;
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
                    } else if (entry == "HEAPTABLE") { // used to populate the MEMORY_HTML TH with heap table and PerfCache
                        AddToDebugOutput ( entry + ": len " + oResponses[i+1].length + ";" + eol );
                        //alert(entry + ":" + oResponses[i+1] );

                        var objtable = document.getElementById("MEMORY_HTML");
                        if (objtable) {
                            if (oResponses[i+1].indexOf("id=textareaPerfCache") > 0 ) { // if the response is the html for the Cache textbox
                                if ( GetCheckboxStatus ( "checkboxPerfCache" ) ) { // only display the response if the checkbox is still checked
                                    objtable.innerHTML = oResponses[i+1];

                                    objtable = document.getElementById("textareaPerfCache");
                                    if (objtable) {
                                        objtable.rows = Number(objtable.scrollHeight/14,0) + 1;
                                        //alert("TEXTAREA height (" + objtable.scrollHeight + "); numrows (" + objtable.rows + ")" );
                                    }
                                }
                            } else {
                                objtable.innerHTML = oResponses[i+1];
                            }
                        }

                        i++;
                    } else if (entry == "HEAPINFO") {
                        AddToDebugOutput ( entry + ": len " + oResponses[i+1].length + ";" + eol );
                        //alert(entry + ":" + oResponses[i+1] );
                        var objheapinfo = document.getElementById("heapinfo");
                        if (objheapinfo) {
                            objheapinfo.innerHTML = MsIeCrFix ( oResponses[i+1] );
                        } else {
                            //alert("element heapinfo not found");
                        }
                        AddToDebugOutput ( entry + ": len1:" + oResponses[i+1].length + "; " + oResponses[i+1] + eol );

                        if (oResponses[i+1].length > 0) {
                            validResponseReceived = 1;
                            GetHeapStats.Value = 0;
                            SetCheckboxStatus ( "checkboxPerfDeep", GetHeapStats );
                            hideOrShow("row_memory", true );
                        } else {
                            if (objheapinfo) {
                                //objheapinfo.innerHTML = "Response was invalid; trying again in 1 second!";
                            }
                        }
                        i++;
                    } else if (entry == "HEAPGRAPH") {
                        AddToDebugOutput ( entry + ": len " + oResponses[i+1].length + ";" + eol );
                        //alert(entry + ":" + oResponses[i+1] );

                        var objplatform = document.getElementById("heapgraph");
                        if (objplatform) {
                            objplatform.innerHTML = oResponses[i+1]; CurrentPlatform = oResponses[i+1];
                        } else {
                            //alert("element heapgraph not found");
                        }
                        AddToDebugOutput ( entry + "[1]: len2:" + oResponses[i+1].length + "; " + oResponses[i+1] + eol );
                        i++;
                    } else if (entry == "SATADEBUG") {
                        AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
                        i++;
                    } else if (entry == "SATAUSB") {
                        //alert(entry + ":" + oResponses[i+1]);
                        AddToDebugOutput ( entry + ": html table len " + oResponses[i+1].length + ";" + eol );

                        var objcheckbox = document.getElementById("checkboxsatausb");
                        var objtable = document.getElementById("MEMORY_HTML");
                        if (objcheckbox) {
                            objcheckbox.checked = GetSataUsb.Value;
                        }
                        //alert("got SATAUSB; GetSataUsb is " + GetSataUsb.Value );
                        // if the checkbox is still checked, populate the information
                        if (GetSataUsb.Value) {
                            if (objtable) { objtable.innerHTML = oResponses[i+1]; }
                        } else {
                            if (objtable) { objtable.innerHTML = ""; }
                            //alert("got SATAUSB; cleared innerHTML" );
                        }
                        i++;
                    } else if (entry == "ALERT") {
                        alert( oResponses[i+1] );
                        i++;
                    } else if (entry == "PERFDEEPSTARTED" && GetCheckboxStatus ( "checkboxprofiling" ) && GetCheckboxStatus ( "checkboxPerfDeep" ) ) {
                        AddToDebugOutput ( entry + ": response (" + oResponses[i+1] + "); " + eol );
                        //alert ( entry + ": response (" + oResponses[i+1] + "); " + eol );
                        if (oResponses[i+1] == "SUCCESS") {
                            GetPerfDeepCountdown = GetPerfDeep.Duration;
                            GetPerfDeepCountdown++;
                            //alert("Perf Record succeeded; setting timeout for  " + GetPerfDeepCountdown );
                            if ( GetPerfDeepCountdown > 1) {
                                //alert( entry + "; duration " + GetPerfDeepCountdown );
                                setTimeout ('GetPerfDeepResultsDoit()', 1000 );
                            }
                        } else {
                            alert("Perf Record failed: " + oResponses[i+1] );
                        }
                        i++;
                    } else if (entry == "PERFCACHESTARTED" && GetCheckboxStatus ( "checkboxmemory" ) && GetCheckboxStatus ( "checkboxPerfCache" ) ) {
                        AddToDebugOutput ( entry + ": response (" + oResponses[i+1] + "); " + eol );
                        //alert ( entry + ": response (" + oResponses[i+1] + "); " + eol );
                        if (oResponses[i+1] == "SUCCESS") {
                            GetPerfCacheCountdown = GetPerfCache.Duration;
                            GetPerfCacheCountdown++;
                            //alert("Perf stat succeeded; setting timeout for  " + GetPerfCacheCountdown );
                            if ( GetPerfCacheCountdown > 1) {
                                //alert( entry + "; duration " + GetPerfCacheCountdown );
                                setTimeout ('GetPerfCacheResultsFunc()', 1000 );
                            }
                        } else {
                            alert("Perf Stat failed: " + oResponses[i+1] );
                        }
                        i++;
                    } else if (entry == "PERFDEEPRESULTSDONE") {
                        AddToDebugOutput ( entry + eol );
                        SetCheckboxStatus ( "checkboxPerfDeep", GetPerfDeep );
                        //alert( entry + ": GetPerfDeep.Value " + GetPerfDeep.Value );
                        if ( GetCheckboxStatus ( "checkboxprofiling" ) && GetCheckboxStatus ( "checkboxPerfDeep" ) ) {
                            checkboxPerfDeepDoit( true );
                            //alert("report done; doing it again");
                            GetPerfDeep.StartReportNow = true;
                            //sendCgiRequest();
                        }
                    } else if (entry == "PERFCACHEDONE") {
                        AddToDebugOutput ( entry + eol );
                        SetCheckboxStatus ( "checkboxPerfCache", GetPerfCache );
                        //alert( entry + ": GetPerfCache.Value " + GetPerfCache.Value );
                        if ( GetCheckboxStatus ( "checkboxmemory" ) && GetCheckboxStatus ( "checkboxPerfCache" ) == true ) {
                            checkboxPerfCacheDoit( true );
                            //alert("stat done; doing it again");
                            GetPerfCache.StartReportNow = true;
                            //sendCgiRequest();
                        }
                    } else if (entry == "PERFENABLED") {
                        AddToDebugOutput ( entry + eol );

                        PerfError = false;
                    } else if (entry == "PERFERROR") {
                        AddToDebugOutput ( entry + eol );

                        PerfError = true;

                        GetPerfTop.Value = 0;
                        SetCheckboxStatus ( "checkboxPerfTop", GetPerfTop );
                        SetCheckboxDisabled ( "checkboxPerfTop", true );

                        GetPerfDeep.Value = 0;
                        SetCheckboxStatus ( "checkboxPerfDeep", GetPerfDeep );
                        SetCheckboxDisabled ( "checkboxPerfDeep", true );

                        GetPerfCache.Value = 0;
                        SetCheckboxStatus ( "checkboxPerfCache", GetPerfCache );
                        SetCheckboxDisabled ( "checkboxPerfCache", true );

                        SetCheckboxDisabled ( "checkboxLinuxTop", false );

                        //alert("PERFERROR done");
                    } else if ( (entry == "PerfTop") || (entry == "LinuxTop") || (entry == "PERFDEEPRESULTS") ) {
                        //alert("Response " + entry + "; len=" + oResponses[i+1].length + ":" + oResponses[i+1] );
                        CountdownSeconds = 0;
                        PerfCacheCountdownSeconds = 0;
                        if ( objPerfTop ) {
                            if (entry == "LinuxTop" ) {
                                // sometimes the LinuxTop command comes back "empty" ... meaning there is a bunch of html that contains nothing useful
                                if ( oResponses[i+1].length > 1000 ) {
                                    if (GetLinuxTop.Value) { // if LinuxTop checkbox is checked (we are expecting LinuxTop data)
                                        objPerfTop.innerHTML = MsIeCrFix ( oResponses[i+1] );
                                    }

                                    // these have to be after the setting of innerHTML; otherwise the checkboxes won't be defined yet
                                    SetCheckboxDisabled ( "checkboxPerfTop", GetLinuxTop.Value ); // if LinuxTop is true -> disable PerfTop; else Enable it
                                    SetCheckboxDisabled ( "checkboxPerfDeep", GetLinuxTop.Value ); // if LinuxTop is true -> disable PerfDeep
                                } else { // the LinuxTop command failed to provide data, try again in this many milliseconds
                                    setTimeout ('RetryLinuxTop()', 500 );
                                }
                            } else {
                                if ( (entry == "PerfTop") ) {
                                    objPerfTop.innerHTML = MsIeCrFix ( oResponses[i+1] );

                                    // these have to be after the setting of innerHTML; otherwise the checkboxes won't be defined yet
                                    if (PerfError) { // if not compiled for perf, only enable LinuxTop
                                        SetCheckboxDisabled ( "checkboxLinuxTop", false );
                                        SetCheckboxDisabled ( "checkboxPerfDeep", true );
                                        SetCheckboxDisabled ( "checkboxPerfTop", true );
                                    } else {
                                        SetCheckboxDisabled ( "checkboxLinuxTop", true );
                                        SetCheckboxDisabled ( "checkboxPerfDeep", true );
                                        SetCheckboxDisabled ( "checkboxPerfTop", false );
                                    }
                                } else if (entry == "PERFDEEPRESULTS" && GetCheckboxStatus("checkboxPerfDeep")) {
                                    objPerfTop.innerHTML = MsIeCrFix ( oResponses[i+1] );

                                    // these have to be after the setting of innerHTML; otherwise the checkboxes won't be defined yet
                                    SetCheckboxDisabled ( "checkboxPerfTop", true );
                                    SetCheckboxDisabled ( "checkboxLinuxTop", true );
                                }
                                //alert(entry +": adding to PERFUTILS TH html (" + objPerfTop.innerHTML + ")" );
                            }

                            // we just re-filled the navigation checkboxes; update the one for context switches
                            SetCheckboxStatus ( "checkboxContextSwitch", GetContextSwitch );

                            var objTextareaTopResults = document.getElementById("textareaTopResults");
                            if (objTextareaTopResults) {
                                objTextareaTopResults.rows = Number(objTextareaTopResults.scrollHeight/14,0) + 1;
                                //alert(entry + ": objTextareaTopResults rows:" + objTextareaTopResults.rows );
                            }

                            if (entry == "PerfTop") {
                                var checkbox = document.getElementById('checkboxPerfTop');
                                if (checkbox ) {
                                    checkbox.checked = GetPerfTop.Value;
                                    SetCheckboxDisabled ( "checkboxPerfTop", false );
                                    SetCheckboxDisabled ( "checkboxLinuxTop", true );
                                }
                            } else if (entry == "LinuxTop") {
                                var checkbox = document.getElementById('checkboxLinuxTop');
                                if (checkbox ) { checkbox.checked = GetLinuxTop.Value; }
                            } else if (entry == "PERFDEEPRESULTS") {
                                var checkbox = document.getElementById('checkboxPerfDeep');
                                if (checkbox ) { checkbox.checked = GetPerfDeep.Value; }
                                objTextareaTopResults = document.getElementById("textareaPerfDeep");
                                if (objTextareaTopResults) {
                                    objTextareaTopResults.rows = Number(objTextareaTopResults.scrollHeight/14,0) + 1;
                                    //alert("TEXTAREA height (" + objTextareaTopResults.scrollHeight + "); numrows (" + objTextareaTopResults.rows + ")" );
                                }
                            }

                            if (oResponses[i+1].length > 0) {
                                validResponseReceived = 1;
                                //hideOrShow("row_profiling", true );
                            }
                        } else {
                            alert ("element PERFUTILS not found");
                        }
                        //AddToDebugOutput ( entry + ": len3:" + oResponses[i+1].length + "; " + oResponses[i+1] + eol );

                        if (GetPerfDeep.Duration) SetInputValue ( "PerfDeepDuration", GetPerfDeep.Duration );
                        if (GetPerfCache.Duration) SetInputValue ( "PerfCacheDuration", GetPerfCache.Duration );
                        i++;
                    } else if (entry == "CONTEXTSWITCH") {
                        AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
                        var checkbox = document.getElementById('checkboxContextSwitch');
                        if ( checkbox && checkbox.checked ) {
                            var objspan = document.getElementById('spanContextSwitches');
                            if (objspan) {
                                objspan.innerHTML = "&nbsp;(" + oResponses[i+1] + ")";
                            } else {
                                alert ( entry + ": object spanContextSwitches not found");
                            }
                        } else {
                            //alert ( entry + ": box not checked");
                        }
                        i++;
                    } else if (entry == "PerfFlameInit") {
                        //AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
                        var checkbox = document.getElementById('checkboxPerfFlame');
                        if ( checkbox && checkbox.checked ) {
                            var objspan = document.getElementById('PerfFlameContents');
                            if (objspan) {
                                objspan.innerHTML = oResponses[i+1];
                            } else {
                                alert ( entry + ": object PerfFlameContents not found");
                            }
                            var objtemp = document.getElementById('PerfFlameDuration');
                            if ( objtemp ) {
                                objtemp.innerHTML = "";
                            }
                        } else {
                            //alert ( entry + ": box not checked");
                        }
                        i++;
                    } else if (entry == "PERFFLAMESTATUS") {
                        AddToDebugOutput ( entry + ":" + oResponses[i+1] + eol );
                        if (GetPerfFlame.State > GetPerfFlameState.IDLE) {
                            var objtemp = document.getElementById('PerfFlameSize');
                            if ( objtemp ) {
                                var sizeBytes = oResponses[i+1]; // update the size of the perf.data file
                                if ( sizeBytes > 1024*1024) { // megabytes
                                    var megabytes = sizeBytes / 1024 / 1024;
                                    objtemp.innerHTML = megabytes.toFixed(1) + "MB";
                                } else if ( sizeBytes > 1024) { // kilobytes
                                    var kilobytes = sizeBytes / 1024;
                                    objtemp.innerHTML = kilobytes.toFixed(1) + "KB";
                                } else {
                                    objtemp.innerHTML = sizeBytes;
                                }
                            }
                            var local = new Date();
                            var epochSeconds = Math.floor(local.getTime() / 1000) - GetPerfFlameRecordingSeconds;
                            objtemp = document.getElementById('PerfFlameDuration');
                            if ( objtemp ) {
                                var minutes = Math.floor( epochSeconds / 60 );
                                var seconds = epochSeconds - (minutes * 60 );
                                if (minutes > 0) {
                                    objtemp.innerHTML = minutes + "m" + seconds + "s";
                                } else {
                                    objtemp.innerHTML = epochSeconds + "s";
                                }
                            }
                            if (GetPerfFlame.State == GetPerfFlameState.STOP) { // oce the record has stopped, proceed to next state ... CREATESCRIPTOUT
                                GetPerfFlameSetState( GetPerfFlameState.CREATESCRIPTOUT);
                            }
                        }
                        i++;
                    } else if (entry == "PERFFLAMEPIDCOUNT") {
                        var pidCountNow = oResponses[i+1];
                        AddToDebugOutput ( entry + ": pidCountNow:" + pidCountNow + "; GetPerfFlamePidCount:" + GetPerfFlamePidCount + "; State:" + GetPerfFlame.State + eol );
                        //alert("PidCount:" + GetPerfFlamePidCount + "; Now:" + pidCountNow );
                        if (GetPerfFlame.State == GetPerfFlameState.RECORDING) {
                            // if the server reported that the 'perf record' pids are no longer active, something caused the record to end
                            if ( pidCountNow == 0 && GetPerfFlamePidCount > 0 ) {
                                //alert("ALERT ... looks like perf record is over"); // either the app it was monitoring exited or something unexpected caused it to exit prematurely
                                GetPerfFlameSetStop();
                            }
                        }
                        GetPerfFlamePidCount = pidCountNow;
                        i++;
                    } else if (entry == "PERFRECORDUUID") {
                        PerfRecordUuid = oResponses[i+1];
                        //alert( entry + ":" + PerfRecordUuid );
                    } else if (entry == "PERFSCRIPTDONE") {
                        //alert( entry + ":" + oResponses[i+1] );
                        GetPerfFlameSetState( GetPerfFlameState.GETSVG);
                        GetSvgContents();
                        i++;
                    } else if (entry == "PerfFlameSvgContents") {
                        var svglength = oResponses[i+1].length;
                        var objtemp = document.getElementById('PerfFlameSvg');
                        if ( objtemp ) {
                            objtemp.innerHTML = oResponses[i+1];
                            //alert("svg:" + objtemp.innerHTML.substr(0,300) );
                        }
                        i++;
                        GetPerfFlameSetState( GetPerfFlameState.IDLE );
                    } else if (entry == "PERFFLAME_DELETEOUTFILE_DONE") {
                        //alert(entry + ":" + oResponses[i+1] );
                        GetPerfFlameSetState( GetPerfFlameState.IDLE );
                        i++;
                    } else {
                        if (entry.length > 1 ) {
                            AddToDebugOutput ( entry + eol );
                        }
                    }
                } // end for each response
            }
        } else {
            var msg = "";
            // if previous timeout is already pending, cancel it
            if (CgiTimeoutId) { clearTimeout(CgiTimeoutId); }
            if (CgiRetryTimeoutId) { clearTimeout(CgiRetryTimeoutId); }

            //alert("TIMEOUT1: urlSentSuccessfully:" + urlSentSuccessfully + "; urlSentRetryCount:" + urlSentRetryCount + "; urlSentRetryCountAfterSuccess:" + urlSentRetryCountAfterSuccess );
            // if we have previously successfully received some responses (used so we do not ignore the very first failure)
            if ( ( urlSentSuccessfully > 10) && (urlSentRetryCountAfterSuccess < 5 ) ) { // if we have not had too many retries
                urlSentRetryCount++; // this one should never get reset to zero
                urlSentRetryCountAfterSuccess++; // this one should get reset to zero if we are ever successful in getting a response

                msg = "TIMEOUT2: urlSentSuccessfully:" + urlSentSuccessfully + "; urlSentRetryCount:" + urlSentRetryCount + "; urlSentRetryCountAfterSuccess:" + urlSentRetryCountAfterSuccess;
                AddToDebugOutput ( msg + eol );
                //alert( msg );

                CgiRetryTimeoutId = setTimeout ('sendCgiRequestRetry()', REFRESH_IN_MILLISECONDS/4 );
                //AddToDebugOutput ( "calling setTimeout(); ID (" + CgiRetryTimeoutId + ")" + eol );
            } else {
                msg = "There was a problem retrieving the XML data:" + eol + eol + xmlhttp.statusText;
                AddToDebugOutput ( msg + eol );
                alert( msg );
            }
        }

    } //if (xmlhttp.readyState==4 )
}
