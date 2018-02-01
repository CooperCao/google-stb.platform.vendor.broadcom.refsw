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
var REFRESH_IN_MILLISECONDS=1000;
var MAX_VLINES_ON_GRAPH=0;      /*Vertical lines in the graph apart from Y-Axis*/
var GRAPH_YAXIS_XCOORD=0;       /*X coordinate of Y-axis*/
var GRAPH_POINTS_PITCH=0;       /*Horizontal distance between plotted points on the graph*/
var epochSeconds = 0;
var passCount=0;
var timeFetchCount = 0;
var dataFetchIteration = 0;
var tzOffset = 0;
var UUID = "";
var cgiTimeoutId=0;
var cgiRetryTimeoutId=0;
var playbackTimeoutId=0;
var urlSentPreviously = "";
var urlSentSuccessfully = 0;
var urlSentRetryCount = 0;
var urlSentRetryCountAfterSuccess = 0;
var playbackPLATFORM = "";
var playbackVARIANT = "";
var playbackPLATVER = "";
var playbackVERSION = "";
var playbackUNAME = "";
var playbackBOLTVER= "";
var playbackSTBTIME = "";
var optionsArray = [];
var defaultCores = 0;
var selectedCores = 0;
var RTIndex = 0;    /*Index of RT Tasks row in optionsArray*/
var NRTIndex = 0;    /*Index of NRT Tasks row in optionsArray*/
var taskIDIndex = 0;    /*Index of Task ID row in optionsArray*/
var svgDict = {}; /*Object to store Dictionary of svgId(as key) and display details structure(as value)*/
var recordTimeStart = 0;
var recordFileContents = "";
var recordFileSize = 0;
var recordON = 0;
var gTextFile = null;
var playbackFileContents = "";
var playbackFileContentsArray = new Array();
var liveModeOptions = "";
var liveRRIndex = 0;    /*Index of Live mode Refresh Rate index. Used when switching back from playback to live*/
var liveModeSelectedCores = 0;
var liveModeTaskID = "";
var playbackControl = {value:0, arrayIndex: 0, entriesDisplayed : 0, totalEntries : 0, PBWidth: 0, fileUploaded : 0};
var exportingTasks = 0;
var exportTasksCSVTitles = "Time Stamp,Task ID,CPU ID,Priority,Load,CPU %,Status";
var exportTasksCSVContent = "";

function uuid()
{
    function s4()
    {
        return Math.floor((1 + Math.random()) * 0x10000)
          .toString(16)
          .substring(1);
    }
    return s4() + s4() + s4() + s4() + s4() + s4() + s4() + s4();
}

function rtrim(stringToTrim)
{
    return stringToTrim.replace(/\s+$/,"");
}

function randomIntFromInterval(min,max)
{
    return Math.floor(Math.random()*(max-min+1)+min);
}

/**
 *  Function: This function will output a human-readable number of bytes.
 **/
function outputSizeInReadableBytes( num_bytes )
{
    if ( num_bytes < 1024) {
        return ( num_bytes + " bytes" );
    } else if ( num_bytes < 1024*1024) {
        return ( Number( num_bytes / 1024).toFixed(1) + " KB" );
    } else if ( num_bytes < 1024*1024*1024) {
        return ( Number( num_bytes / 1024 / 1024).toFixed(1) + " MB" );
    } else {
        return ( Number( num_bytes / 1024 / 1024 / 1024).toFixed(1) + " GB" );
    }
}

function myLoad()
{
    if ( passCount == 0 ) {
        var local = new Date();
        UUID = uuid();
        epochSeconds = Math.floor(local.getTime() / 1000);
        tzOffset = local.getTimezoneOffset();
        cgiTimeoutId = setTimeout ('oneSecond()', REFRESH_IN_MILLISECONDS );
        var objRefreshRateSelect = document.getElementById("refreshRateList");
        var selection = objRefreshRateSelect.options[0].value;
        dataFetchIteration = parseInt(selection);
        var input = document.getElementById("taskText");
        if (input) {
            input.setAttribute('size',input.getAttribute('placeholder').length);
        }
        var objTable = document.getElementById("overallDataTable")
        playbackControl.PBWidth = Math.floor((objTable.clientWidth * 40)/100);
        var svgPB = document.getElementById("svgPB");
        if (svgPB) {
            input.setAttribute('width', playbackControl.PBWidth + "px");
        }
    }
    passCount++;
    sendCgiRequest();
}

function getOptionsURL()
{
    var url = "";
    if (selectedCores > 0) {
        url += "&selectedCores=" + selectedCores;
    }

    if (typeof optionsArray[RTIndex] !== 'undefined') {
        if (optionsArray[RTIndex].selected == true) {
            url += "&filterRT=1";
            url += "&RTCores=" + (selectedCores > 0 ? selectedCores : defaultCores);
        }
    }

    if (typeof optionsArray[NRTIndex] !== 'undefined') {
        if (optionsArray[NRTIndex].selected == true) {
            url += "&filterNRT=1";
            url += "&NRTCores=" + (selectedCores > 0 ? selectedCores : defaultCores);
        }
    }

    if (typeof optionsArray[taskIDIndex] !== 'undefined') {
        if (optionsArray[taskIDIndex].selected == true) {
            url += "&taskID="+ optionsArray[taskIDIndex].value;
        }
    }

    if (exportingTasks == 1) {
        url += "&exportTasks=1"
    }
    return url;
}

function sendCgiRequest()
{
    var url = "";

    var RandomValue = randomIntFromInterval(1000000,9999999);
    url = "/cgi/astratop.cgi?randomvalue=" + RandomValue;

    if (epochSeconds > 0) {
        url += "&datetime=" + epochSeconds + "&tzoffset=" + tzOffset;
        epochSeconds = 0;
    }

    if (dataFetchIteration == timeFetchCount) {
        timeFetchCount = 0;
        url += getOptionsURL();
    }

    urlSentPreviously = url;
    sendCgiRequestDoItNow ( url );
}

function sendCgiRequestRetry ( )
{
    if ( cgiRetryTimeoutId ) {
        clearTimeout ( cgiRetryTimeoutId );
        cgiRetryTimeoutId = 0;
    }

    cgiRetryTimeoutId = setTimeout ('sendCgiRequestRetry()', REFRESH_IN_MILLISECONDS*2 );
    sendCgiRequestDoItNow( urlSentPreviously );
}

function sendCgiRequestDoItNow( url )
{
    xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange= serverHttpResponse;
    xmlhttp.open("GET",url,true);
    xmlhttp.send(null);
}

function serverHttpResponse ()
{
    if ((playbackControl.value == 0) && (xmlhttp.readyState==4)) {
        if (xmlhttp.status == 200) {
            if ( cgiRetryTimeoutId ) {
                clearTimeout ( cgiRetryTimeoutId );
                cgiRetryTimeoutId = 0;
            }

            urlSentSuccessfully++; // used to try to determine if a failure is intermittant or we have never been successful
            urlSentRetryCountAfterSuccess = 0;

            if (recordON == 1) {
                var localTime = 0;
                recordFileContents += "|!|";
                recordFileSize = recordFileContents.length;

                localTime = new Date();
                var sizeObj=document.getElementById("recordSize");
                if (sizeObj) {
                    var seconds = Math.floor(localTime.getTime() / 1000) - recordTimeStart;
                    sizeObj.innerHTML =  outputSizeInReadableBytes( recordFileSize  ) + " (" + Math.floor(seconds/60) + ":" + seconds%60 + ")"; // leading zero
                }
                var descobj=document.getElementById("recordDescriptor");
                if (descobj) {
                    descobj.innerHTML = "File&nbsp;size:";
                }
            }

            var responseText = rtrim(xmlhttp.responseText);
            var oResponses = responseText.split( "~" );

            // sometimes the very first response is blank; if this happens, send another request very soon after we receive the blank response
            if (responseText.length == 0) {
                clearTimeoutOneSecond();
                cgiTimeoutId = setTimeout ('oneSecond()', REFRESH_IN_MILLISECONDS/10 );
            } else {
                processResponses ( oResponses );

                // if this is the response that arrived after a timeout and retry, manually start the one-second timeout
                //In record case, timer is manually started after first response.
                if ( cgiTimeoutId == 0 ) {
                    oneSecond();
                }
            }
        } else {
            clearTimeoutOneSecond( );
            // if we have previously successfully received some responses (used so we do not ignore the very first failure)
            if ( ( urlSentSuccessfully > 10) && (urlSentRetryCountAfterSuccess < 5 ) ) { // if we have not had too many retries
                urlSentRetryCount++; // this one should never get reset to zero
                urlSentRetryCountAfterSuccess++; // this one should get reset to zero if we are ever successful in getting a response
                cgiRetryTimeoutId = setTimeout ('sendCgiRequestRetry()', REFRESH_IN_MILLISECONDS/4 );
            } else if ( urlSentRetryCount < 2 ) {
                    urlSentRetryCount++; // this one should never get reset to zero
                    cgiRetryTimeoutId = setTimeout ('sendCgiRequestRetry()', REFRESH_IN_MILLISECONDS/4 );
            } else {
                alert("There was a problem retrieving the XML data:" + xmlhttp.statusText);
            }
        }
    }
}

function clearTimeoutOneSecond( ) {
    // if previous timeout is already pending, cancel it
    if (cgiTimeoutId) {
        clearTimeout(cgiTimeoutId);
        cgiTimeoutId = 0;
    }
}

function oneSecond ()
{
    myLoad();
    // if previous timeout is already pending, cancel it
    clearTimeoutOneSecond();
    cgiTimeoutId = setTimeout ('oneSecond()', REFRESH_IN_MILLISECONDS );
}

function processPolyLineResponse ( svgId, polyLineId, nextY)
{
    var polyLineObj = document.getElementById(polyLineId);
    if (polyLineObj) {
        var coords = polyLineObj.getAttribute('points' );
        var x = GRAPH_YAXIS_XCOORD + (svgDict[svgId].nextVLine * GRAPH_POINTS_PITCH);
        if (coords == null || coords == "") {
            svgDict[svgId].linesDisplayed[polyLineId] = { startVLine : 0, updated : 0};
            svgDict[svgId].linesDisplayed[polyLineId].startVLine = svgDict[svgId].nextVLine;
            coords = x + "," + nextY + " ";
            polyLineObj.setAttribute('points', coords );
        } else {
            var coords2 = rtrim(coords);
            var splits = coords2.split(' ');
            var points = splits.length;
            if (svgDict[svgId].iteration <= (MAX_VLINES_ON_GRAPH + 1)) {
                coords += x + "," + nextY + " ";
                polyLineObj.setAttribute('points', coords );
            } else {
                var newcoords = "";
                var startIdx;
                var startVLine;
                svgDict[svgId].linesDisplayed[polyLineId].startVLine -= 1;
                if (svgDict[svgId].linesDisplayed[polyLineId].startVLine >=0) {
                    startIdx = 0;
                } else {
                    svgDict[svgId].linesDisplayed[polyLineId].startVLine = 0;
                    startIdx = 1;
                }
                var vLine = svgDict[svgId].linesDisplayed[polyLineId].startVLine;
                for (var idx=startIdx; idx < points; idx++ ) {
                    var justone = splits[idx].split(',');
                    var newX = GRAPH_YAXIS_XCOORD + (vLine*GRAPH_POINTS_PITCH);
                    newcoords += newX + "," + justone[1] + " " ;
                    vLine++;
                }
                newcoords += x + "," + nextY + " " ;
                polyLineObj.setAttribute('points', newcoords );
            }
        }
        svgDict[svgId].linesDisplayed[polyLineId].updated = 1;
    }
}

/**
 *  Function: This function check if a graph line has become discontinuous and update the broken/discontinuous lines on the graph.
 **/
function handleLineDiscontinuities(svgId)
{
    var dict = svgDict[svgId].linesDisplayed;
    for (var key in dict) {
        if (dict[key].updated) {
            dict[key].updated = 0;
        } else {
            var count = svgDict[svgId].linesDiscontinued.length;
            var tempId = "temp" + svgId + count;
            svgDict[svgId].linesDiscontinued.push({ startVLine : 0, id : tempId, position : count});
            svgDict[svgId].linesDiscontinued[count].startVLine = svgDict[svgId].linesDisplayed[key].startVLine;
            var objPolyline = document.getElementById(key);
            var clone = objPolyline.cloneNode(true);
            clone.id = tempId;
            var objSvg = document.getElementById(svgId);
            objSvg.appendChild(clone);
            var hasChild = objSvg.querySelector("#" + tempId) != null;
            objPolyline.setAttribute('points', "" );
            delete dict[key];
        }
    }
    var array = svgDict[svgId].linesDiscontinued;
    var deleted = 0;
    for (var i = 0; i < array.length; i++) {
        if (svgDict[svgId].iteration > (MAX_VLINES_ON_GRAPH + 1)) {
            var objPolyLine = document.getElementById(array[i].id);
            if (objPolyLine) {
                var coords = objPolyLine.getAttribute('points' );
                var coords2 = rtrim(coords);
                var splits = coords2.split(' ');
                var points = splits.length;
                if (coords2 == "" ) {
                    var objSvg = document.getElementById(svgId);
                    objSvg.removeChild(objPolyLine);
                    array.splice(i,1);
                    deleted++;
                    i--;
                } else {
                    if (deleted > 0) {
                        var newPosition = array[i].position - deleted;
                        array[i].position = newPosition;
                        array[i].id = "temp" + svgId + newPosition;
                        objPolyLine.id = array[i].id;
                    }
                    var newcoords = "";
                    var startIdx;
                    var startVLine;
                    array[i].startVLine -= 1;
                    if (array[i].startVLine >=0) {
                        startIdx = 0;
                    } else {
                        array[i].startVLine = 0;
                        startIdx = 1;
                    }
                    var vLine = array[i].startVLine;
                    for (var idx=startIdx; idx < points; idx++ ) {
                        var justone = splits[idx].split(',');
                        var newX = GRAPH_YAXIS_XCOORD + (vLine*GRAPH_POINTS_PITCH);
                        newcoords += newX + "," + justone[1] + " " ;
                        vLine++;
                    }
                    objPolyLine.setAttribute('points', newcoords );
                }
            }
        }
    }
}

/**
 *  Function: This function will step through all of the responses that come back from the
 *            CGI call. Each response typically has special processing associated with each
 *            of them. This function is also called during Playback to simulate the same
 *            response processing.
 **/
function processResponses ( oResponses )
{
    var i=0;
    var idx=0;
    var idx2=0;
    for ( i = 0; i < oResponses.length; i++) {
        var entry = oResponses[i];
        if ( entry.length == 0 ) {
            continue;
        }
        if (entry == "FATAL") {
            alert("FATAL ... " + oResponses[i+1]);
            i++;
        } else if ( entry == "DEBUG" ) {
            i++;
        } else if (entry == "VERSION") {
            playbackVERSION = oResponses[i+1];
            i++;
        } else if (entry == "STBTIME") {
            playbackSTBTIME = oResponses[i+1];
            if (recordON == 1) {
                recordFileContents += "~" + entry + "~" + oResponses[i+1];
            }
            var obj2=document.getElementById("stbTime");
            if (obj2) {
                obj2.innerHTML = oResponses[i+1];
                timeFetchCount++;
            } else {
                alert("id=stbtime not found");
            }
            i++;
        } else if (entry == "PLATFORM") {
            playbackPLATFORM = oResponses[i+1];
            var objplatform = document.getElementById("platform");
            if (objplatform) {
                objplatform.innerHTML = oResponses[i+1]; CurrentPlatform = oResponses[i+1];
            }
            i++;
        } else if (entry == "VARIANT") {
            playbackVARIANT = oResponses[i+1];
            var objvariant = document.getElementById("variant");
            if ( objvariant ) {
                objvariant.innerHTML = "(Variant: " + oResponses[i+1] + ")";
            }
            i++;
        } else if (entry == "PLATVER") {
            playbackPLATVER = oResponses[i+1];
            var objplatform = document.getElementById("platVer");
            if (objplatform) {
                objplatform.innerHTML = oResponses[i+1]
            }
            window.document.title = CurrentPlatform + " " + oResponses[i+1];
            i++;
        } else if (entry == "UNAME") {
            playbackUNAME = oResponses[i+1];
            var objplatform = document.getElementById("uName");
            if (objplatform) {
                objplatform.innerHTML = "Kernel: " + oResponses[i+1];
            }
            i++;
        } else if (entry == "BOLTVER") {
            playbackBOLTVER = oResponses[i+1];
            var objplatform = document.getElementById("boltVer");
            if (objplatform) {
                objplatform.innerHTML = "Bolt: " + oResponses[i+1];
            }
            i++;
        } else if (entry == "CPUCORES") {
            var objcpuCores = document.getElementById("cpuCores");
            if (objcpuCores) {
                objcpuCores.innerHTML = oResponses[i+1];
            }
            var options = document.getElementById("optionsRow");
            options.style.visibility='visible';
            i++;
        } else if (entry == "CORESBITMASK") {
            var bitMask = oResponses[i+1];
            defaultCores = bitMask;
            selectedCores = bitMask;
            var index = 0;
            var rowIndex = 1;
            var objtable = document.getElementById("overallDataTable");
            if (objtable) {
                optionsArray.push({name: "checkboxRecord", rowId: "rowRecord", selected: false, value: 0});
                var row;
                var thCell;
                while (bitMask) {
                    if (1 == (bitMask & 1)) {
                        row = objtable.insertRow(rowIndex);
                        row.id = "rowCPU" + index;
                        row.className = 'dataRow';
                        thCell =document.createElement("th");
                        row.appendChild(thCell);
                        thCell.id = "core" + index;
                        thCell.className = 'data';
                        thCell.colspan=2;
                        optionsArray.push({name: "checkboxcpu" + index, rowId: row.id, selected: true, value: 1 << index});
                        rowIndex++;
                        svgDict["core" + index + "Graph"] = { nextVLine : -1,
                                                              iteration : 0,
                                                              linesDisplayed : {},  /*Dictionary of Continuous lines displayed*/
                                                              linesDiscontinued : [] /*Array of Discontinuous lines displayed*/
                                                            };
                        svgDict["core" + index + "RTGraph"] = { nextVLine : -1,
                                                              iteration : 0,
                                                              linesDisplayed : {},  /*Dictionary of Continuous lines displayed*/
                                                              linesDiscontinued : [] /*Array of Discontinuous lines displayed*/
                                                            };
                        svgDict["core" + index + "NRTGraph"] = { nextVLine : -1,
                                                              iteration : 0,
                                                              linesDisplayed : {},  /*Dictionary of Continuous lines displayed*/
                                                              linesDiscontinued : [] /*Array of Discontinuous lines displayed*/
                                                            };
                    }
                    index++;
                    bitMask = Math.floor(bitMask/2);
                }
                RTIndex = rowIndex;
                row = objtable.insertRow(rowIndex++);
                row.id = "rowRTTasks";
                row.className = 'dataRow';
                row.style.display = 'none';
                row.style.visibility = 'hidden';
                thCell =document.createElement("th");
                thCell.id = 'RTTasks';
                thCell.className = 'data';
                thCell.colspan=2;
                row.appendChild(thCell);
                optionsArray.push({name: "checkboxRT", rowId: row.id, selected: false, value: 0});
                NRTIndex = rowIndex;
                row = objtable.insertRow(rowIndex++);
                row.id = "rowNRTTasks";
                row.className = 'dataRow';
                row.style.display = 'none';
                row.style.visibility = 'hidden';
                thCell =document.createElement("th");
                thCell.id = 'NRTTasks';
                thCell.className = 'data';
                thCell.colspan=2;
                row.appendChild(thCell);
                taskIDIndex = rowIndex;
                optionsArray.push({name: "checkboxNRT", rowId: row.id, selected: false, value: 0});
                row = objtable.insertRow(rowIndex);
                row.id = "rowTaskID";
                row.className = 'dataRow';
                row.style.display = 'none';
                row.style.visibility = 'hidden';
                thCell =document.createElement("th");
                thCell.id = 'TaskID';
                thCell.className = 'data';
                thCell.colspan=2;
                row.appendChild(thCell);
                optionsArray.push({name: "taskButton", rowId: row.id, selected: false, value: 0});
                svgDict["taskIDGraph"] = { nextVLine : -1,
                                           iteration : 0,
                                           linesDisplayed : {},  /*Dictionary of Continuous lines displayed*/
                                           linesDiscontinued : [] /*Array of Discontinuous lines displayed*/
                                          };
            }
            i++;
        } else if (entry == "VLINESONGRAPH") {
            MAX_VLINES_ON_GRAPH = Number(oResponses[i+1]);
            i++;
        } else if (entry == "YAXISXCOORD") {
            GRAPH_YAXIS_XCOORD = Number(oResponses[i+1]);
            i++;
        } else if (entry == "GRAPHPOINTSPITCH") {
            GRAPH_POINTS_PITCH = Number(oResponses[i+1]);
            i++;
        } else if (entry == "TASKSPERCOREINIT") {
            var cores = oResponses[i+1];
            i += 2;
            for (var count = 0; count < cores; count++) {
                var objCore = document.getElementById(oResponses[i]);
                if (objCore) {
                    objCore.innerHTML = oResponses[i+1];
                }
                i += 2;
            }
            i--;
        } else if (entry == "RTTASKSINIT") {
            var objRTTasks = document.getElementById("RTTasks");
            if (objRTTasks) {
                objRTTasks.innerHTML = oResponses[i+1];
            }
            i++;
        } else if (entry == "NRTTASKSINIT") {
            var objNRTTasks = document.getElementById("NRTTasks");
            if (objNRTTasks) {
                objNRTTasks.innerHTML = oResponses[i+1];
            }
            i++;
        } else if (entry == "TASKSPERCORE" || entry == "RTTASKSPERCORE" || entry == "NRTTASKSPERCORE" || entry == "TASKID") {
            if (recordON == 1) {
                recordFileContents += "~" + entry + "~" + oResponses[i+1];
            }
            var segments = oResponses[i+1];
            i += 2;
            for (var count = 0; count < segments; count++) {
                if (recordON == 1) {
                    recordFileContents += "~" + oResponses[i] + "~" + oResponses[i+1];
                }
                var objCore = document.getElementById(oResponses[i]);
                if (objCore) {
                    objCore.innerHTML = oResponses[i+1];
                }
                i += 2;
            }
            i--;
        } else if (entry == "TASKIDINIT") {
            var objTaskID = document.getElementById("TaskID");
            if (objTaskID) {
                objTaskID.innerHTML = oResponses[i+1];
            }
            i++;
        } else if (entry == "UPDATEGRAPH") {
            if (recordON == 1) {
                recordFileContents += "~" + entry + "~" + oResponses[i+1];
            }
            var svgId = oResponses[i + 1];
            if (svgDict[svgId].nextVLine < MAX_VLINES_ON_GRAPH) {
                svgDict[svgId].nextVLine++;
            }
            /*First point is plotted on Y-Axis. So MAX_VLINES_ON_GRAPH + 1 points can be seen at a time*/
            if (svgDict[svgId].iteration <= MAX_VLINES_ON_GRAPH + 1) {
                svgDict[svgId].iteration++;
            }
            i += 2;
            if (oResponses[i] == "POLYLINESCOUNT") {
                if (recordON == 1) {
                    recordFileContents += "~" + oResponses[i] + "~" + oResponses[i+1];
                }
                var polyLines = oResponses[i + 1];
                i += 2;
                for (var count = 0; count < polyLines; count++) {
                    if (recordON == 1) {
                        recordFileContents += "~" + oResponses[i] + "~" + oResponses[i+1];
                    }
                    var polyLineId = oResponses[i];
                    var nextY = oResponses[i + 1];
                    processPolyLineResponse(svgId, polyLineId, nextY);
                    i += 2;
                }
            }
            handleLineDiscontinuities(svgId);
            i--;
        } else if (entry == "EXPORTTASKS") {
            exportTasksCSVContent += oResponses[i+1];
            i++;
        } else if (entry == "ERROR") {
            alert(oResponses[i+1]);
            i++;
        } else {
            if (entry.length > 0 ) {
                alert("Unknown entry : " + entry);
            }
        }
    } // end for each response
}

function setDataRefreshRate()
{
    var objRefreshRateSelect = document.getElementById("refreshRateList");
    var selection = objRefreshRateSelect.options[objRefreshRateSelect.selectedIndex].value;
    dataFetchIteration = parseInt(selection);
}

function myClick(event)
{
    var target = event.target || event.srcElement;
    var id = target.id;
    setVariable(id);
}

function hideOrShow ( elementid, trueOrFalse )
{
    var obj=document.getElementById(elementid);
    if ( obj ) {
        if (trueOrFalse) {
            obj.style.display = '';
            obj.style.visibility = '';
        } else {
            obj.style.display = 'none';
            obj.style.visibility = 'hidden';
        }
    }
}

/**
*Function : It hides the sections for unselected cores in RT, NRT rows.
**/
function hideOrShowRtNrtSubSections ( whichOne )
{
    var bitMask = (selectedCores == 0) ? defaultCores : selectedCores;
    var refBitMask = defaultCores;
    var index = 0;
    while (refBitMask) {
        var obj = document.getElementById("core" + index + whichOne + "Title");
        var display = ((refBitMask & 1) & (bitMask & 1) == 1);
        if (display) {
            obj.style.display = '';
            obj.style.visibility = '';
        } else {
            obj.style.display = 'none';
            obj.style.visibility = 'hidden';
        }
        obj = document.getElementById("core" + index + whichOne + "Table");
        if (display) {
            obj.style.display = '';
            obj.style.visibility = '';
        } else {
            obj.style.display = 'none';
            obj.style.visibility = 'hidden';
        }
        obj = document.getElementById("core" + index + whichOne + "Graph");
        if (display) {
            obj.style.display = '';
            obj.style.visibility = '';
        } else {
            obj.style.display = 'none';
            obj.style.visibility = 'hidden';
        }
        refBitMask = Math.floor(refBitMask/2);
        bitMask = Math.floor(bitMask/2);
        index++;
    }
}

function checkboxSelected ( fieldName, fieldValue )
{
    for (var i = 0; i < optionsArray.length; i++) {
        if (optionsArray[i].name == fieldName) {
            optionsArray[i].selected = fieldValue;
            selectedCores ^= optionsArray[i].value;
            hideOrShow(optionsArray[i].rowId, fieldValue);
            break;
        }
    }
}

function makeTextFile (text)
{
    var data = new Blob([text], {type: 'text/plain'});

    // If we are replacing a previously generated file we need to
    // manually revoke the object URL to avoid memory leaks.
    if ( gTextFile !== null) {
        window.URL.revokeObjectURL( gTextFile );
    }

    var gTextFile = window.URL.createObjectURL(data);
    return gTextFile;
}

/**
 *  Function: This function will control the processing of all user-changeable
 *            elements ... buttons, images, checkboxes, input fields, etc.
 **/
function setVariable(fieldName)
{
    var fieldValue = "";
    var obj=document.getElementById(fieldName);
    if (obj) {
        if (obj.type == "checkbox") {
            fieldValue = obj.checked;
            checkboxSelected ( fieldName, fieldValue );
            /*Show only the graphs for selected cores*/
            hideOrShowRtNrtSubSections("RT");
            hideOrShowRtNrtSubSections("NRT");
        }
        else if (obj.type == "submit") {
            if (fieldName == "taskButton") {
                var taskID = document.getElementById("taskText").value;
                if (taskID.length) {
                    optionsArray[taskIDIndex].value = parseInt(taskID);
                    optionsArray[taskIDIndex].selected = true;
                    hideOrShow(optionsArray[taskIDIndex].rowId,true);
                }
            } else if (fieldName == "recordButton") {
                var obj=document.getElementById(fieldName);
                if (obj) {
                    if (obj.innerHTML.indexOf( "Start") == 0 ) {
                        var localTime = new Date();
                        recordTimeStart = Math.floor(localTime.getTime() / 1000);
                        recordON = 1;
                        obj.innerHTML = "Stop Record";
                        obj.style.backgroundColor = "salmon";
                        recordPlaybackOptsConfigure("START", "RECORD");
                        clearTimeoutOneSecond();
                        // clear the contents in case we had a previous recording
                        recordFileContents = "";
                        recordFileContents += playbackPLATFORM + "|!|";
                        for (var i = 0; i < optionsArray.length; i++) {
                            recordFileContents += optionsArray[i].selected + ",";
                        }
                        recordFileContents += "|!|";
                        var objRefreshSelect = document.getElementById('refreshRateList');
                        recordFileContents += "refreshRateList" + " " + objRefreshSelect.selectedIndex;
                        recordFileContents += "|!|" + selectedCores;
                        if (optionsArray[taskIDIndex].selected == true) {
                            recordFileContents += "|!|" + optionsArray[taskIDIndex].value;
                        } else {
                            recordFileContents += "|!|" + "";
                        }
                        recordFileContents += "|!|";
                        recordFileContents += "PLATFORM~" + playbackPLATFORM;
                        recordFileContents +=  "~VARIANT~" + playbackVARIANT;
                        recordFileContents +=  "~PLATVER~" + playbackPLATVER;
                        recordFileContents +=  "~VERSION~" + playbackVERSION;
                        recordFileContents +=  "~UNAME~" + playbackUNAME;
                        recordFileContents +=  "~BOLTVER~" + playbackBOLTVER;
                        recordFileContents +=  "~STBTIME~" + playbackSTBTIME;
                        recordFileSize = recordFileContents.length;
                        sendCgiRequest();
                    } else {
                        recordON = 0;
                        obj.innerHTML = "Start Record";
                        obj.style.backgroundColor = "lightgreen";
                        recordPlaybackOptsConfigure("STOP", "RECORD");
                    }
                }
            } else if (fieldName == "recordSaveFileButton") {
                var link = document.createElement('a');
                link.setAttribute('download', "astratop_" + UUID + '.txt' );
                link.href = makeTextFile( recordFileContents );
                document.body.appendChild(link);

                window.requestAnimationFrame(function () {
                    var event = new MouseEvent('click');
                    link.dispatchEvent(event);
                    document.body.removeChild(link);
                });
            } else if (fieldName == "exportTaskStatsButton") {
                var obj=document.getElementById(fieldName);
                if (obj) {
                    if (obj.innerHTML.indexOf( "Export") == 0 ) {
                        exportingTasks = 1;
                        obj.innerHTML = "Stop Exporting";
                        obj.style.backgroundColor = "salmon";
                        setButtonDisabled( 'saveTaskStatsButton', true);
                        setButtonDisabled( 'buttonChooseFile', true );
                        setButtonDisabled( 'buttonPlaybackRun', true );
                        setButtonDisabled( 'buttonPlaybackStop', true );
                        exportTasksCSVContent = "";
                    } else {
                        exportingTasks = 0;
                        obj.innerHTML = "Export Task Stats";
                        obj.style.backgroundColor = "lightblue";
                        setButtonDisabled('saveTaskStatsButton', false);
                        setButtonDisabled( 'buttonChooseFile', false );
                        setButtonDisabled( 'buttonPlaybackRun', false );
                        setButtonDisabled( 'buttonPlaybackStop', false );
                    }
                }
            } else if (fieldName == "saveTaskStatsButton") {
                if (exportTasksCSVContent.length > 0) {
                    var link = document.createElement('a');
                    link.setAttribute('download', "astratop_taskStats" + UUID + '.csv' );
                    link.href = makeTextFile( exportTasksCSVTitles + exportTasksCSVContent );
                    document.body.appendChild(link);

                    window.requestAnimationFrame(function () {
                        var event = new MouseEvent('click');
                        link.dispatchEvent(event);
                        document.body.removeChild(link);
                    });
                } else {
                    alert("No Task stats available to export!!");
                }
            }
        } else if ( obj.type == "image" ) {
            if (fieldName == "buttonPlaybackRun") {
                /*File Upload done and playback has got over or has been stopped*/
                if ((playbackControl.fileUploaded == 1) && (playbackControl.value == 0)) {
                    setButtonImage ( fieldName, "bmemperf_pause.jpg" );
                    filePlayback("START");
                } else if (playbackControl.value == 2) {
                    setButtonImage ( fieldName, "bmemperf_pause.jpg" );
                    playbackControl.value = 1;
                    processPlaybackEvents();
                } else if (playbackControl.value == 1){
                    if ( playbackTimeoutId ) { clearTimeout( playbackTimeoutId ); }
                    setButtonImage ( fieldName, "bmemperf_play.jpg" );
                    playbackControl.value = 2;
                }
            } else if (fieldName == "buttonPlaybackStop") {
                if (playbackControl.value != 0) {
                    filePlayback("STOP");
                }
            }
        }
    }
}

function checkTaskText()
{
    var obj = document.getElementById("taskText");
    if (obj) {
        var text = obj.value;
        if (text.length == 0) {
            hideOrShow(optionsArray[taskIDIndex].rowId,false);
            optionsArray[taskIDIndex].value = 0;
            optionsArray[taskIDIndex].selected = false;
        }
    }
}

function setButtonDisabled ( targetId, newState )
{
    var objButton = document.getElementById( targetId );
    if (objButton) {
        objButton.disabled = newState;
    } else {
        alert( "could not find element ... " + targetId );
    }
}

/**
 *  Function: This function will change the button image file to the new specified
 *            image. It is used during playback to change the "play" button to the
 *            "pause" button and vice-a-versa.
 **/
function setButtonImage ( fieldName, newImageName )
{
    var obj = document.getElementById( fieldName );
    if ( obj ) {
        obj.src = newImageName;
    }
}

function recordPlaybackOptsConfigure ( whichOne , mode)
{
    var newState;
    if ( whichOne == "START" ) {
        newState = true;
    } else if ( whichOne == "STOP" ){
        newState = false ;
    }
    if (mode == "RECORD") {
        setButtonDisabled( 'recordSaveFileButton', newState );
        setButtonDisabled( 'buttonChooseFile', newState );
        setButtonDisabled( 'buttonPlaybackRun', newState );
        setButtonDisabled( 'buttonPlaybackStop', newState );
    } else if (mode == "PLAYBACK") {
        setButtonDisabled( 'recordButton', newState );
        setButtonDisabled( 'exportTaskStatsButton', newState );
    }
    setButtonDisabled( 'taskButton', newState );
    setButtonDisabled( 'refreshRateList', newState );

    for (var i = 0; i < optionsArray.length; i++) {
        setButtonDisabled(optionsArray[i].name,newState);
    }
}

function filePlayback(whichOne)
{
    recordPlaybackOptsConfigure(whichOne, "PLAYBACK");
    clearGraphsAndTable();
    if (whichOne == "START") {
        clearTimeoutOneSecond();
        var i;
        liveModeOptions = "";
        /*Save current user options*/
        for (i = 0; i < optionsArray.length; i++) {
            liveModeOptions += optionsArray[i].selected + ",";
        }
        /*Save current refresh rate*/
        var objRefreshSelect = document.getElementById('refreshRateList');
        liveRRIndex = objRefreshSelect.selectedIndex;

        liveModeSelectedCores = selectedCores;
        if (optionsArray[taskIDIndex].selected == true)
            liveModeTaskID = optionsArray[taskIDIndex].value;
        else
            liveModeTaskID = "";

        /*Load options from playback file*/
        var pbOptions = playbackFileContentsArray[1].split(',');
        for (i = 0; i < optionsArray.length; i++) {
            var objCheckBox = document.getElementById( optionsArray[i].name );
            var select = (pbOptions[i] == 'true');
            optionsArray[i].selected = select;
            if (objCheckBox) {
                objCheckBox.checked = select;
            }
            hideOrShow(optionsArray[i].rowId ,select);
        }

        /*Load refresh rate from playback file*/
        var rateEntry = playbackFileContentsArray[2].split(' ');
        var objSelect = document.getElementById(rateEntry[0]);
        if (objSelect) {
            objSelect.selectedIndex = rateEntry[1];
            var selection = objSelect.options[objSelect.selectedIndex].value;
            dataFetchIteration = parseInt(selection);
        }

        selectedCores = Number(playbackFileContentsArray[3]);
        hideOrShowRtNrtSubSections ("RT");
        hideOrShowRtNrtSubSections ("NRT");

        setButtonImage ( "buttonPlaybackRun", "bmemperf_pause.jpg" );

        var objTaskID = document.getElementById("taskText");
        objTaskID.value = playbackFileContentsArray[4];

        var platInfo = playbackFileContentsArray[5];
        var oResponses;

        oResponses = platInfo.split("~");
        processResponses( oResponses );

        playbackControl.value = 1;  /*Stop mode - value = 0; Play mode - value = 1; Pause mode - value = 2 */
        /*First entry is platform, next 2 entries has user options selected while recording,
              4th entry has the Cores selected while recording, 5th entry has taskID inputted,
              6th entry has header info like platfoem, bolt ver, time etc. So the task content starts from the 7th entry*/
        playbackControl.arrayIndex = 6;
        playbackControl.entriesDisplayed = 0;
        playbackControl.totalEntries = Number ( playbackFileContentsArray.length - 6)   /*Per Iteration entry starts from 7th entry*/
        processPlaybackEvents();
    } else if (whichOne == "STOP") {
        playbackControl.entriesDisplayed = 0;
        updateProgressGraph();
        playbackControl.arrayIndex = 0;
        playbackControl.value = 0;
        playbackControl.totalEntries = 0;
        setButtonImage ( "buttonPlaybackRun", "bmemperf_play.jpg" );

        if ( playbackTimeoutId ) { clearTimeout( playbackTimeoutId ); }
        /*Reset live mode options*/
        var options = liveModeOptions.split(',');
        for (var i = 0; i < optionsArray.length; i++) {
            optionsArray[i].selected = (options[i] == 'true');
            var objCheckBox = document.getElementById( optionsArray[i].name );
            if (objCheckBox) {
                objCheckBox.checked = optionsArray[i].selected;
            }
            hideOrShow(optionsArray[i].rowId ,optionsArray[i].selected);
        }
        var objRefreshSelect = document.getElementById('refreshRateList');
        objRefreshSelect.selectedIndex = liveRRIndex;
        var selection = objRefreshSelect.options[objRefreshSelect.selectedIndex].value;
        dataFetchIteration = parseInt(selection);
        timeFetchCount = dataFetchIteration;
        selectedCores = liveModeSelectedCores;
        hideOrShowRtNrtSubSections ("RT");
        hideOrShowRtNrtSubSections ("NRT");
        var objTaskID = document.getElementById("taskText");
        objTaskID.value = liveModeTaskID;
        if (liveModeTaskID.length > 0)
            optionsArray[taskIDIndex].value = liveModeTaskID;
        /*Start timer for Live*/
        cgiTimeoutId = setTimeout ('oneSecond()', REFRESH_IN_MILLISECONDS );
    }
}

function clearGraphsAndTable()
{
/*Clear graphs*/
    for (var key in svgDict) {
        var dict = svgDict[key].linesDisplayed;
        for (var polyLine in dict) {
            var objPolyline = document.getElementById(polyLine);
            objPolyline.setAttribute('points', "" );
            delete dict[polyLine];
        }
        {
            var array = svgDict[key].linesDiscontinued;
            for (var i = 0; i < array.length; i++) {
                var objPolyLine = document.getElementById(array[i].id);
                var objSvg = document.getElementById(key);
                objSvg.removeChild(objPolyLine);
                var hasChild = objSvg.querySelector("#" + array[i].id) != null;
            }
        }
        svgDict[key].linesDiscontinued = [];
        svgDict[key].nextVLine = -1;
        svgDict[key].iteration = 0;
    }

/*Clear table area*/
    var temp = defaultCores;
    var table;
    var index = 0;
    while (temp) {
        if (1 == (temp & 1)) {
            table = document.getElementById("core" + index + "Table");
            table.innerHTML = "";
            table = document.getElementById("core" + index + "RTTable");
            table.innerHTML = "";
            table = document.getElementById("core" + index + "NRTTable");
            table.innerHTML = "";
        }
        index++;
        temp = Math.floor(temp/2);
    }
    table = document.getElementById("taskIDTable");
    table.innerHTML = "";
}


/**
 *  Function: This function will update the line green progress bar used to show how far
 *            the playback function has progressed.
 **/
function updateProgressGraph ( )
{
    var objprogressbar =document.getElementById("progressBar");
    var percentage = Math.ceil (((playbackControl.entriesDisplayed) / playbackControl.totalEntries ) * playbackControl.PBWidth);
    var points = "0,0 " + percentage + ",0 " + percentage + ",10 0,10";
    if (objprogressbar) {
        objprogressbar.setAttribute('points', points);
    }
}

/**
 *  Function: This function is the main handler when a playback is in progress. It will read the next
 *            playback record and process the playback record just as it would have been processed during
 *            regular live processing ... by calling processResponses() ... which is also called during
 *            a normal response when the XMLHttpRequest comes back with an okay status.
 **/
function processPlaybackEvents( )
{
    if (playbackControl.totalEntries && playbackControl.entriesDisplayed < playbackControl.totalEntries) {
        var seconds_response = playbackFileContentsArray[playbackControl.arrayIndex];

        if ( seconds_response.length > 1 ) {
            var oResponses;
            oResponses = seconds_response.split("~");

            processResponses( oResponses );
            playbackControl.entriesDisplayed++;
            playbackControl.arrayIndex++;

            updateProgressGraph();

            if ( playbackTimeoutId ) { clearTimeout( playbackTimeoutId ); }

            if ( playbackControl.value  == 1) {
                // schedule myself for processing one second from now
                playbackTimeoutId = setTimeout ('processPlaybackEvents()', REFRESH_IN_MILLISECONDS );
            }
        }
    } else {
        filePlayback("STOP");
    }
}

function readText(that)
{
    if (that.files && that.files[0]) {
        var reader = new FileReader();
        reader.onload = function (e) {
            playbackFileContents = e.target.result;
            playbackFileContentsArray = playbackFileContents.split("|!|");
            /*Check if recorded file is from the same platform as the one astratop is currently running on*/
            if (playbackFileContentsArray[0] == playbackPLATFORM) {
                playbackControl.fileUploaded = 1;
                var fullPath = that.value;
                var startIndex = (fullPath.indexOf('\\') >= 0 ? fullPath.lastIndexOf('\\') : fullPath.lastIndexOf('/'));
                var filename = fullPath.substring(startIndex);
                if (filename.indexOf('\\') === 0 || filename.indexOf('/') === 0) {
                    var objFile = document.getElementById("fileLabel");
                    while ( objFile.firstChild ) {
                        objFile.removeChild( objFile.firstChild );
                    }
                    objFile.appendChild( document.createTextNode(filename.substring(1)) );
                }
                filePlayback("START");
            } else {
                if (playbackControl.value != 0) {
                    filePlayback("STOP");
                }
                playbackControl.fileUploaded = 0;
                playbackFileContents = "";
                playbackFileContentsArray = [];
                alert("Please choose a file recorded from " + playbackPLATFORM + " platform!!");
                var objFile = document.getElementById("fileLabel");
                while ( objFile.firstChild ) {
                    objFile.removeChild( objFile.firstChild );
                }
                objFile.appendChild( document.createTextNode("No File Chosen"));
                that.value = null;
            }
        }
        reader.readAsText(that.files[0]);
    }
}
