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
/* jshint browser: true, devel: true, undef: true, laxcomma: true, smarttabs: true */
/* global Dygraph, BRCM, WFE */

/******** Default constants **********/
var SPAN_MAX = 1000000000;
var SPAN_MIN = 6000000;
var DEFAULT_SPAN = SPAN_MAX;
var DEFAULT_FFT_SIZE = 1024;
var DEFAULT_FREQUENCY_CENTER = SPAN_MAX / 2;
var DEFAULT_NUMBER_OF_SAMPLES = 256;
var DEFAULT_GAIN = 1;
var DEFAULT_REF_LEVEL = 0;
var DEFAULT_YMIN = -140.00;
var DEFAULT_YMAX = 10.00;
/**************************************/

var _graph;
var _coreID = 0;
var _yMin = DEFAULT_YMIN;
var _yMax = DEFAULT_YMAX;
var _fftSize = DEFAULT_FFT_SIZE;
var _gain = DEFAULT_GAIN;
var _samples = DEFAULT_NUMBER_OF_SAMPLES;
var _fftData = [[0.0, 0.0]];     // Initialize as an array of arrays to make dygraph happy
var _fftDataAvg = [];
var _fftDataAvgCounter = 0;
var _fftDataPrev = [];      // Used to calculate max peak
var _numOfSampleSets = 10; // Number of sample sets to average over (for vid averaging)
var _avgEnabled = false;
var _peakHoldEnabled = false;
var _channelPowerEnabled = false;
var _specARunning = false;
var _useBBS = false;	// flag that indicates if we are running this in BBS

var FREQ_TYPES = {
    "CENTER": 0,
    "START": 1,
    "STOP": 2
};

var FREQ_UNITS = {
    "GHZ": 1e9,
    "MHZ": 1e6,
    "KHZ": 1e3,
    "HZ": 1
};

var FREQ_UNITS_MODE = {
        HZ: 0
    ,   KHZ: 1
    ,   MHZ: 2
    ,   GHZ: 3
};

var REF_LEVEL_UNITS_MODE = {
        DBM: 0
    ,   DBMV: 1
    ,   DBUV: 2
};

var _selectedFreqUnits = FREQ_UNITS.MHZ;
var _freqLabel = "MHz";
var _selectedSpanUnits = FREQ_UNITS.MHZ;
var _spanLabel = "MHz";
var _selectedChannelPowerSpanUnits = FREQ_UNITS.MHZ;
var _channelPowerSpanLabel = "MHz";
// Hz
var _span = DEFAULT_SPAN;

// Hz
var _frequencyCenterHz = DEFAULT_FREQUENCY_CENTER;
var _frequencyStartHz = DEFAULT_FREQUENCY_CENTER - (DEFAULT_SPAN / 2);
var _frequencyStopHz = DEFAULT_FREQUENCY_CENTER + (DEFAULT_SPAN / 2);
var _setFreqMode = FREQ_UNITS_MODE.HZ;
var FREQ_MAX = 1000000000;
var FREQ_MIN = 0;
var _frequencyType = FREQ_TYPES.CENTER;

// dBm
var _refLevel = DEFAULT_REF_LEVEL;
var _setRefLevelMode = 0;
var _refLevelLabel = "dBm";
var REF_LEVEL_MAX = 50;
var REF_LEVEL_MIN = -140;
var REF_LEVEL_DBMV_MAX = 98;
var REF_LEVEL_DBMV_MIN = -92;
var REF_LEVEL_DBUV_MAX = 158;
var REF_LEVEL_DBUV_MIN = -32;

// channel power
var _channelPowerSpan = DEFAULT_SPAN;

// var _wfeCommonParams = WFE.common.params;
// var _wfeCommonApi = WFE.common.api;
// var _specAParams = new _wfeCommonParams.SpectrumAnalyzerParameters();

var _specAParams = {
    frontendId: 7,
    startFrequency: 0,
    stopFrequency: 1200000000,
    numOfSamples: 256,
    fftSize: 1024
};

var _broadcastDiag = null;

var urlParams;
(window.onpopstate = function () {
    var match;
    var pl     = /\+/g;  // Regex for replacing addition symbol with a space
    var search = /([^&=]+)=?([^&]*)/g;
    var decode = function (s) { return decodeURIComponent(s.replace(pl, " ")); };
    var query  = window.location.search.substring(1);

    urlParams = {};
    while (match = search.exec(query))
        urlParams[decode(match[1])] = decode(match[2]);
    })();

window.onload = function () {
    var url = location.origin;
//CAD    var rpcClient = new Trellis.RPCClient(url);

    // // Connect to server, and once connected,
    // // start the app.
//CAD    rpcClient.connect().then(function () {
        startApp(/*rpcClient*/ 0 );
//CAD    });

};

function startApp(rpcClient) {
    if(supportsBrowser()) {
//CAD        _broadcastDiag = new Trellis.Diagnostic.BroadcastDiag(rpcClient);

        // Add event handlers
        var buttonHideViewSettings = document.getElementById("buttonHideViewSettings");
        var buttonPreset = document.getElementById("buttonPreset");
        var buttonHold = document.getElementById("buttonHold");
        var buttonRun = document.getElementById("buttonRun");
        var frequencyItem = document.getElementById("frequencyItem");
        var buttonFrequencyCenter = document.getElementById("buttonFrequencyCenter");
        var buttonFrequencyStart = document.getElementById("buttonFrequencyStart");
        var buttonFrequencyStop = document.getElementById("buttonFrequencyStop");
        var buttonFrequencyGHz = document.getElementById("buttonFrequencyGHz");
        var buttonFrequencyMHz = document.getElementById("buttonFrequencyMHz");
        var buttonFrequencykHz = document.getElementById("buttonFrequencykHz");
        var buttonFrequencyHz = document.getElementById("buttonFrequencyHz");
        var spanItem = document.getElementById("spanItem");
        var buttonSpanGHz = document.getElementById("buttonSpanGHz");
        var buttonSpanMHz = document.getElementById("buttonSpanMHz");
        var buttonSpankHz = document.getElementById("buttonSpankHz");
        var buttonSpanHz = document.getElementById("buttonSpanHz");
        var amplitudeItem = document.getElementById("amplitudeItem");
        var buttonAmplitudeSet = document.getElementById("buttonAmplitudeSet");
        var bwItem = document.getElementById("bwItem");
        var radioVidAverageOn = document.getElementById("radioVidAverageOn");
        var radioVidAverageOff = document.getElementById("radioVidAverageOff");
        var radioPeakHoldOn = document.getElementById("radioPeakHoldOn");
        var radioPeakHoldOff = document.getElementById("radioPeakHoldOff");
        var measurementItem = document.getElementById("measurementItem");
        var radioChannelPowerOn = document.getElementById("radioChannelPowerOn");
        var radioChannelPowerOff = document.getElementById("radioChannelPowerOff");
        var buttonChannelPowerSpanGHz = document.getElementById("buttonChannelPowerSpanGHz");
        var buttonChannelPowerSpanMHz = document.getElementById("buttonChannelPowerSpanMHz");
        var buttonChannelPowerSpankHz = document.getElementById("buttonChannelPowerSpankHz");
        var buttonChannelPowerSpanHz = document.getElementById("buttonChannelPowerSpanHz");
        var buttonFullSpan = document.getElementById("buttonFullSpan");
        var markerItem = document.getElementById("markerItem");
        var buttonMarkersOff = document.getElementById("buttonMarkersOff");
        var buttonSearchPeakOn = document.getElementById("buttonSearchPeakOn");
        // var customItem = document.getElementById("customItem");

        buttonHideViewSettings.addEventListener("click", buttonHideViewSettings_onclick, false);
        buttonPreset.addEventListener("click", buttonPreset_onclick, false);
        buttonHold.addEventListener("click", buttonHold_onclick, false);
        buttonRun.addEventListener("click", buttonRun_onclick, false);
        frequencyItem.addEventListener("click", function () { settingsItem_onclick('divFrequencyPage'); }, false);
        buttonFrequencyCenter.addEventListener("click", buttonFrequencyCenter_onclick, false);
        buttonFrequencyStart.addEventListener("click", buttonFrequencyStart_onclick, false);
        buttonFrequencyStop.addEventListener("click", buttonFrequencyStop_onclick, false);
        buttonFrequencyGHz.addEventListener("click", function () { frequencySet('GHz'); }, false);
        buttonFrequencyMHz.addEventListener("click", function () { frequencySet('MHz'); }, false);
        buttonFrequencykHz.addEventListener("click", function () { frequencySet('kHz'); }, false);
        buttonFrequencyHz.addEventListener("click", function () { frequencySet('Hz'); }, false);
        spanItem.addEventListener("click", function () { settingsItem_onclick('divSpanPage'); }, false);
        buttonSpanGHz.addEventListener("click", function () { spanSet('GHz'); }, false);
        buttonSpanMHz.addEventListener("click", function () { spanSet('MHz'); }, false);
        buttonSpankHz.addEventListener("click", function () { spanSet('kHz'); }, false);
        buttonSpanHz.addEventListener("click", function () { spanSet('Hz'); }, false);
        amplitudeItem.addEventListener("click", function () { settingsItem_onclick('divAmplitudePage'); }, false);
        buttonAmplitudeSet.addEventListener("click", amplitudeSet, false);
        bwItem.addEventListener("click", function () { settingsItem_onclick('divBwPage'); }, false);
        radioVidAverageOn.addEventListener("click", radioVidAverage_onclick, false);
        radioVidAverageOff.addEventListener("click", radioVidAverage_onclick, false);
        radioPeakHoldOn.addEventListener("click", radioPeakHold_onclick, false);
        radioPeakHoldOff.addEventListener("click", radioPeakHold_onclick, false);
        measurementItem.addEventListener("click", function () { settingsItem_onclick('divMeasurementPage'); }, false);
        radioChannelPowerOn.addEventListener("click", radioChannelPower_onclick, false);
        radioChannelPowerOff.addEventListener("click", radioChannelPower_onclick, false);
        buttonChannelPowerSpanGHz.addEventListener("click", function () { channelPowerSpanSet('GHz'); }, false);
        buttonChannelPowerSpanMHz.addEventListener("click", function () { channelPowerSpanSet('MHz'); }, false);
        buttonChannelPowerSpankHz.addEventListener("click", function () { channelPowerSpanSet('kHz'); }, false);
        buttonChannelPowerSpanHz.addEventListener("click", function () { channelPowerSpanSet('Hz'); }, false);
        buttonFullSpan.addEventListener("click", function () { spanSetFull('GHz'); }, false);
        markerItem.addEventListener("click", function () { settingsItem_onclick('divMarkerPage'); }, false);
        buttonMarkersOff.addEventListener("click", markersOff, false);
        buttonSearchPeakOn.addEventListener("click", peakSearchOn, false);
        // customItem.addEventListener("click", function () { settingsItem_onclick('divCustomPage'); }, false);

        buttonPreset_onclick();
        initializeGraph();
    }
}

function supportsBrowser() {
    // Spectrum Analyzer is currently only supported in Chrome, Firefox, and Safari.
    // For all other browsers, simply display an "Unsupported browser message".
    var isChrome = navigator.userAgent.indexOf('Chrome') > -1;
    var isSafari = navigator.userAgent.indexOf("Safari") > -1;
    var isFirefox = navigator.userAgent.indexOf("Firefox") > -1;
    if(!isChrome && !isSafari && !isFirefox) {
        // Display unsupported browser message
        var bodyElement = document.getElementsByTagName("body")[0];
        bodyElement.innerHTML = "Spectrum Analyzer not supported in this browser.  Please use Safari, Firefox or Chrome.";
        return false;
    }
    return true;
}

function nameAnnotation(ann) {
    return "(" + ann.series + ")";
}

var graph_initialized = false;
var num = 1;            //the position of the last marker
var num_peak = -1;      //the position of the marker after which the peak is put on
var marker = [0];         //the x value of all markers, where marker[0] is the x value of the peak
var _isPeakMarked = 0;

function initializeGraph() {
    var annotations = [];
    _graph = new Dygraph(
        document.getElementById("divGraph"),
        function() {
            var r = "MHz,PowerSpec\n";
            for (var i=1; i<=_fftData.length; i++) {
              r += _fftData[i-1][0];
              r += "," + _fftData[i-1][1];
              r += "\n";
            }
            return r;
        },
        {
                title: "Spectrum Analyzer"
            ,	height: '100%'
            ,	width: '100%'
            ,	axisLabelColor: '#646464'
            ,	axisLineColor: '#646464'
            ,	gridLineColor: '#646464'
            ,   colors: ['#00baff']
            ,	xlabel: "MHz"
            ,	ylabel: "dBm"
            ,	digitsAfterDecimal: 2
            ,	valueRange: [_yMin, _yMax]
            ,	zoomCallback: function(xStart, xStop, yRanges) {
                        _frequencyStartHz = Math.round(xStart) * 1000000;	// Must be int
                        _frequencyStopHz = Math.round(xStop) * 1000000;	// Must be int
                        setFrequencyStart(_frequencyStartHz);
                        setFrequencyStop(_frequencyStopHz);
                        _yMin = yRanges[0][0];
                        _yMax = yRanges[0][1];

                        updateFields();
                        updateSpecAParams();
                    }
            ,   drawCallback: function(g, is_initial) {
                    if (is_initial) {
                        graph_initialized = true;
                        if (annotations.length > 0) {
                            g.setAnnotations(annotations);
                        }
                    }

                    var ann = g.annotations();
                    var html = "<span>";
                    for (var i = 0; i < ann.length; i++) {
                        var name = nameAnnotation(ann[i]);
                        //html += "<span id='" + name + "'>"
                        html += name + ": " + ann[i].shortText;
                        html += " -> " + ann[i].text + "</span><br/>";
                    }
                    // document.getElementById("list").innerHTML = html;
                }
        }
    );

    if (graph_initialized) {
        _graph.setAnnotations(annotations);
    }

    _graph.updateOptions( {
        pointClickCallback: function(event, p) {
            // Check if the point is already annotated.
            if (p.annotation)
                return;

            // If not, add one annotation for this point.
            var ann = {
                series: p.name,
                xval: p.xval,
                shortText: num,
                text: "Marker#" + num + " :("+ p.xval +","+ parseInt(p.yval, 10) +")"
            };
            var anns = _graph.annotations();
            anns.push(ann);
            _graph.setAnnotations(anns);

            marker[num] = p.xval;
            num++;
        }
    });
}

function markersOff() {
    //1. pop all the current markers, and the peak if existed
    var annotations;
    for (var i = num - 1; i > 0; i--) {
        //pop the peak
        // FIXME: Is this if statement needed?
        if (i == num_peak && _isPeakMarked == 1) {
            annotations = _graph.annotations();
            annotations.pop();
            _graph.setAnnotations(annotations);
        }

        annotations = _graph.annotations();
        annotations.pop();
        _graph.setAnnotations(annotations);
    }

    //2. push the peak if existed
    if (num_peak > 0 && _isPeakMarked == 1) {
        for (var k = 0; k < _fftData.length; k++) {
            if(_fftData[k][0] == marker[0]){
                break;
            }
        }
        var ann = {
            series: 'PowerSpec',
            xval: marker[0],
            shortText: 0,
            text: "Peak#" + 0 + " :("+ marker[0] +","+ parseInt(_fftData[k][1], 10) +")"
        };
        var anns = _graph.annotations();
        anns.push(ann);
        _graph.setAnnotations(anns);

        num_peak = 0;
    }

    num = 1;

    // Turn off peaks as well
    peakSearchOff();
}

function peakSearchOn() {
    var x = 0;
    var i = 0;

    //1. if the peak has been added before, pop it
    if (_isPeakMarked == 1) {
        //pop the markers after the existed peak
        for (i = num - 1; i >= num_peak; i--) {
            var anns1 = _graph.annotations();
            anns1.pop();
            _graph.setAnnotations(anns1);
        }

        //push the markers after the existed peak
        for (i = num_peak + 1; i < num; i++) {
            for (var k = 0; k < _fftData.length; k++) {
                if(_fftData[k][0] == marker[i]){
                    break;
                }
            }
            var ann = {
                series: 'PowerSpec',
                xval: marker[i],
                shortText: i,
                text: "Marker#" + i + " :("+ marker[i] +","+ parseInt(_fftData[k][1], 10) +")"
            };
            var anns = _graph.annotations();
            anns.push(ann);
            _graph.setAnnotations(anns);
        }
    }

    //2. add a peak: only MaxPeak could be marked
    for (i = 1; i < _fftData.length; i++) {
        if(_fftData[i][1] > _fftData[x][1]){
            x = i;
        }
    }

    var annotations = _graph.annotations();
    annotations.push( {
          series: 'PowerSpec',
          x:  x,
          shortText: 0,
          text: "Peak#" + 0 + " :(" + _fftData[x][0]+","+_fftData[x][1] +")",
          tickHeight: 10
    } );
    _graph.setAnnotations(annotations);

    marker[0] = _fftData[x][0];
    num_peak = num - 1;
    _isPeakMarked = 1;
}

function peakSearchOff() {
    var i = 0;
    var annotations;
    //1. if the peak has been added before, pop it
    if (_isPeakMarked == 1) {
        //pop the markers after the existed peak
        for (i = num - 1; i >= num_peak; i--) {
            annotations = _graph.annotations();
            annotations.pop();
            _graph.setAnnotations(annotations);
        }

        //push the markers after the existed peak
        for (i = num_peak + 1; i < num; i++) {
            for (var k = 0; k < _fftData.length; k++) {
                if(_fftData[k][0] == marker[i]){
                    break;
                }
            }
            var ann = {
                series: 'PowerSpec',
                xval: marker[i],
                shortText: i,
                text: "Marker#" + i + " :("+ marker[i] +","+ parseInt(_fftData[k][1], 10) +")"
            };
            annotations = _graph.annotations();
            annotations.push(ann);
            _graph.setAnnotations(annotations);
        }
    }
    num_peak = -1;
    _isPeakMarked = 0;
}

function buttonPreset_onclick() {
    _frequencyType = FREQ_TYPES.CENTER;
    _selectedFreqUnits = FREQ_UNITS.MHZ;
    _selectedSpanUnits = FREQ_UNITS.MHZ;
    _selectedChannelPowerSpanUnits = FREQ_UNITS.MHZ;
    _freqLabel = "MHz";
    _spanLabel = "MHz";
    _channelPowerSpanLabel = "MHz";
    _avgEnabled = false;
    _setFreqMode = FREQ_UNITS_MODE.HZ;
    setFrequencyCenter(DEFAULT_FREQUENCY_CENTER);
    setSpan(DEFAULT_SPAN);
    _fftSize = DEFAULT_FFT_SIZE;
    _gain = DEFAULT_GAIN;
    _samples = DEFAULT_NUMBER_OF_SAMPLES;
    _yMin = DEFAULT_YMIN;
    _yMax = DEFAULT_YMAX; // Use 0.0001 so RGraphs will display 0 (RGraph doesn't like yMax = 0)

    _refLevel = DEFAULT_REF_LEVEL;
    _channelPowerEnabled = false;
    _channelPowerSpan = DEFAULT_SPAN;

    updateFields();
    updateSpecAParams();
}

/*************************************************************
*	IPad functions
**************************************************************/
function TouchProperties() {
    this.graphID = null;
    this.fingerCount = 0;
    this.startX = 0;
    this.startY = 0;
    this.curX = 0;
    this.curY = 0;
    this.deltaX = 0;	// difference between curX and startX
    this.deltaY = 0;	// difference between curY and startY
    this.horzDiff = 0;	// abs of deltaX
    this.vertDiff = 0;	// abs of deltaY
    this.minLength = 72; // the shortest distance the user may swipe
    this.swipeLength = 0;
    this.swipeAngle = null;
    this.swipeDirection = null;
    this.lastCurX = 0;	// this is the previous curX saved during touchMove
    this.lastCurY = 0;	// this is the previous curY saved during touchMove
    this.lastScale = 0;
    this.curScale = 0;
}

var _myTouchProperties = new TouchProperties();

function touchStart(event, graph) {
    // disable the standard ability to select the touched object
    event.preventDefault();

    // get the total number of fingers touching the screen
    _myTouchProperties.fingerCount = event.touches.length;
    // since we're looking for a swipe (single finger) and not a gesture (multiple fingers),
    // check that only one finger was used
    if ( _myTouchProperties.fingerCount == 1 ) {
        // get the coordinates of the touch
        _myTouchProperties.startX = event.touches[0].pageX;
        _myTouchProperties.startY = event.touches[0].pageY;

        _myTouchProperties.lastCurX = _myTouchProperties.startX;
        _myTouchProperties.lastCurY = _myTouchProperties.startY;
        _myTouchProperties.graphID = graph;
    } else {
        // more than one finger touched so cancel
        touchCancel(event);
    }
}

function touchMove(event) {
    event.preventDefault();

    if ( event.touches.length == 1 ) {
        _myTouchProperties.curX = event.touches[0].pageX;
        _myTouchProperties.curY = event.touches[0].pageY;
        //var delta = _myTouchProperties.curX - _myTouchProperties.lastCurX;
        //var horzDiff = Math.abs(delta);
        //var swipeRight = delta > 0;
        //var centerFrequency = _frequencyCenterHz;
        //if(horzDiff > 5) {
            // if(swipeRight) {
                // // Decrease frequencyCenter
                // centerFrequency -= 1000000;
                // setFrequencyCenter(centerFrequency);
            // }
            // else {
                // // Increase frequencyCenter
                // centerFrequency += 1000000;
                // setFrequencyCenter(centerFrequency);
            // }

            calculateAngle(_myTouchProperties);
            determineSwipeDirection(_myTouchProperties);
            processSwipe(_myTouchProperties);
            _myTouchProperties.lastCurX = _myTouchProperties.curX;
            _myTouchProperties.lastCurY = _myTouchProperties.curY;
        //}
    } else {
        touchCancel(event);
    }

}

function touchEnd(event) {
    event.preventDefault();
    // check to see if more than one finger was used and that there is an ending coordinate
    // if ( _myTouchProperties.fingerCount == 1 && _myTouchProperties.curX != 0 ) {
        // _myTouchProperties.deltaX = _myTouchProperties.curX - _myTouchProperties.startX;
        // _myTouchProperties.deltaY = _myTouchProperties.curY - _myTouchProperties.startY;
        // _myTouchProperties.horzDiff = Math.abs(_myTouchProperties.deltaX);
        // _myTouchProperties.vertDiff = Math.abs(_myTouchProperties.deltaY);
        // // use the Distance Formula to determine the length of the swipe
        // _myTouchProperties.swipeLength = Math.round(Math.sqrt(Math.pow(_myTouchProperties.deltaX, 2) + Math.pow(_myTouchProperties.deltaY, 2)));
        // // if the user swiped more than the minimum length, perform the appropriate action
        // if ( _myTouchProperties.swipeLength >= _myTouchProperties.minLength ) {
            // calculateAngle(_myTouchProperties);
            // determineSwipeDirection(_myTouchProperties);
            // processSwipe(_myTouchProperties);
        // }
    // }
    //alert("_myTouchProperties.deltaX = " + _myTouchProperties.deltaX + " _myTouchProperties.deltaY = " + _myTouchProperties.deltaY);
    touchCancel(event); // reset the variables
}

function touchCancel(event) {
    // reset the variables back to default values
    _myTouchProperties.fingerCount = 0;
    _myTouchProperties.startX = 0;
    _myTouchProperties.startY = 0;
    _myTouchProperties.curX = 0;
    _myTouchProperties.curY = 0;
    _myTouchProperties.deltaX = 0;
    _myTouchProperties.deltaY = 0;
    _myTouchProperties.horzDiff = 0;
    _myTouchProperties.vertDiff = 0;
    _myTouchProperties.swipeLength = 0;
    _myTouchProperties.swipeAngle = null;
    _myTouchProperties.swipeDirection = null;
    _myTouchProperties.graphID = null;
    _myTouchProperties.lastCurX = 0;
    _myTouchProperties.lastCurY = 0;
}

function gestureStart(event) {
    event.preventDefault();

    _myTouchProperties.lastScale = event.scale;
}

function gestureChange(event) {
    event.preventDefault();
    var span = _span;
    _myTouchProperties.curScale = event.scale;
    var deltaScale = _myTouchProperties.curScale - _myTouchProperties.lastScale;

    if(deltaScale > 0) {	// expand gesture -- zoom in
        span -= 1000000;
    }
    else {	// pinch gesture -- zoom out
        span += 1000000;
    }

    _myTouchProperties.lastScale = _myTouchProperties.curScale;
    setSpan(span);
}

function gestureEnd(event) {
    event.preventDefault();
    // var scale = event.scale;

    // if(scale > 1) {	// expand gesture -- zoom in
        // _frequencyStartHz += Math.round(scale * 10000000);	// Must be int
        // _frequencyStopHz -= Math.round(scale * 10000000);	// Must be int
    // }
    // else {	// pinch gesture -- zoom out
        // scale = Math.abs(scale - 1) * 10;
        // _frequencyStartHz -= Math.round(scale * 10000000);	// Must be int
        // _frequencyStopHz += Math.round(scale * 10000000);	// Must be int
    // }

    // setFrequencyStart(_frequencyStartHz);
    // setFrequencyStop(_frequencyStopHz);
    //updateFields();
    //updateSpecAParams();
    //alert("Scale = " + event.scale + "	Rotation = " + event.rotation);
    touchCancel(event);
}

function calculateAngle(touchPropertiesObj) {
    // var X = touchPropertiesObj.startX - touchPropertiesObj.curX;
    var X = touchPropertiesObj.lastCurX - touchPropertiesObj.curX;
    // var Y = touchPropertiesObj.curY - touchPropertiesObj.startY;
    var Y = touchPropertiesObj.curY - touchPropertiesObj.lastCurY;
    var Z = Math.round(Math.sqrt(Math.pow(X,2)+Math.pow(Y,2))); //the distance - rounded - in pixels
    var r = Math.atan2(Y,X); //angle in radians (Cartesian system)
    touchPropertiesObj.swipeAngle = Math.round(r*180/Math.PI); //angle in degrees
    if ( touchPropertiesObj.swipeAngle < 0 ) { touchPropertiesObj.swipeAngle =  360 - Math.abs(touchPropertiesObj.swipeAngle); }
}

function determineSwipeDirection(touchPropertiesObj) {
    if ( (touchPropertiesObj.swipeAngle <= 45) && (touchPropertiesObj.swipeAngle >= 0) ) {
        touchPropertiesObj.swipeDirection = 'left';
    } else if ( (touchPropertiesObj.swipeAngle <= 360) && (touchPropertiesObj.swipeAngle >= 315) ) {
        touchPropertiesObj.swipeDirection = 'left';
    } else if ( (touchPropertiesObj.swipeAngle >= 135) && (touchPropertiesObj.swipeAngle <= 225) ) {
        touchPropertiesObj.swipeDirection = 'right';
    } else if ( (touchPropertiesObj.swipeAngle > 45) && (touchPropertiesObj.swipeAngle < 135) ) {
        touchPropertiesObj.swipeDirection = 'down';
    } else {
        touchPropertiesObj.swipeDirection = 'up';
    }
}

function processSwipe(touchPropertiesObj) {
    var swipedElement = document.getElementById(touchPropertiesObj.graphID);
    var centerFrequency = _frequencyCenterHz;
    var deltaX = _myTouchProperties.curX - _myTouchProperties.lastCurX;
    var deltaY = _myTouchProperties.curY - _myTouchProperties.lastCurY;
    var horzDiff = Math.abs(deltaX);
    var vertDiff = Math.abs(deltaY);

    // To reduce sensitivity of the swipe, only adjust when
    // the difference between the last position and the current
    // position is considerable (vertDiff > n)
    if ( touchPropertiesObj.swipeDirection == 'left' ) {
        // Increase center frequency
        centerFrequency += 1000000;
        setFrequencyCenter(centerFrequency);
    } else if ( touchPropertiesObj.swipeDirection == 'right' ) {
        // Decrease center frequency
        centerFrequency -= 1000000;
        setFrequencyCenter(centerFrequency);
    } else if ( touchPropertiesObj.swipeDirection == 'up' && vertDiff > 5 ) {
        // Decrease y-axis (power level)
        _yMin -= 1;
        _yMax -= 1;
    } else if ( touchPropertiesObj.swipeDirection == 'down'  && vertDiff > 5 ) {
        // Increase y-axis (power level)
        _yMin += 1;
        _yMax += 1;
    }

    updateFields();
    updateSpecAParams();
}

/***************************************************************
*	End IPad functions
****************************************************************/

function buttonHideViewSettings_onclick() {
    if(document.getElementById("buttonHideViewSettingsText").innerHTML == "HIDE")
    {
        // Hide the Settings panel
        document.getElementById("divGraph").style.width = "97%";
        document.getElementById("divStatus").style.width = "96.4%";
        document.getElementById("divSettings").className = "hiddenPages";
        document.getElementById("buttonHideViewSettingsText").innerHTML = "VIEW";
    }
    else
    {
        document.getElementById("divGraph").style.width = "70%";
        document.getElementById("divStatus").style.width = "69.4%";
        document.getElementById("divSettings").className = "";
        document.getElementById("buttonHideViewSettingsText").innerHTML = "HIDE";
    }

    // Update the size of the graph
    _graph.resize();
}

function settingsItem_onclick(pageId) {

    // Get the className to toggle to
    var toggleClassName = (document.getElementById(pageId).className === "hiddenPages") ? "visiblePages" : "hiddenPages";

    // Hide all pages
    document.getElementById("divMarkerPage").className = "hiddenPages";
    // document.getElementById("divCustomPage").className = "hiddenPages";
    document.getElementById("divMeasurementPage").className = "hiddenPages";
    document.getElementById("divFrequencyPage").className = "hiddenPages";
    document.getElementById("divSpanPage").className = "hiddenPages";
    document.getElementById("divAmplitudePage").className = "hiddenPages";
    document.getElementById("divBwPage").className = "hiddenPages";

    // Toggle page of selected item
    document.getElementById(pageId).className = toggleClassName;
}

function updateFields() {
    // Turn off markers when changing display
    markersOff();

    if (_setFreqMode == FREQ_UNITS_MODE.HZ)
    {
        _selectedFreqUnits = FREQ_UNITS.HZ;
        _selectedSpanUnits = FREQ_UNITS.HZ;
        _selectedChannelPowerSpanUnits = FREQ_UNITS.HZ;
        _freqLabel = 'Hz';
        _spanLabel = 'Hz';
        _channelPowerSpanLabel = 'Hz';
    }
    else if (_setFreqMode == FREQ_UNITS_MODE.KHZ)
    {
        _selectedFreqUnits = FREQ_UNITS.KHZ;
        _selectedSpanUnits = FREQ_UNITS.KHZ;
        _selectedChannelPowerSpanUnits = FREQ_UNITS.KHZ;
        _freqLabel = 'kHz';
        _spanLabel = 'kHz';
        _channelPowerSpanLabel = 'kHz';
    }
    else if (_setFreqMode == FREQ_UNITS_MODE.MHZ)
    {
        _selectedFreqUnits = FREQ_UNITS.MHZ;
        _selectedSpanUnits = FREQ_UNITS.MHZ;
        _selectedChannelPowerSpanUnits = FREQ_UNITS.MHZ;
        _freqLabel = 'MHz';
        _spanLabel = 'MHz';
        _channelPowerSpanLabel = 'MHz';
    }
    else if (_setFreqMode == FREQ_UNITS_MODE.GHZ)
    {
        _selectedFreqUnits = FREQ_UNITS.GHZ;
        _selectedSpanUnits = FREQ_UNITS.GHZ;
        _selectedChannelPowerSpanUnits = FREQ_UNITS.GHZ;
        _freqLabel = 'GHz';
        _spanLabel = 'GHz';
        _channelPowerSpanLabel = 'GHz';
    }

    if (_setRefLevelMode == REF_LEVEL_UNITS_MODE.DBM)
        _refLevelLabel = 'dBm';
    else if (_setRefLevelMode == REF_LEVEL_UNITS_MODE.DBMV)
        _refLevelLabel = 'dBmV';
    else if (_setRefLevelMode == REF_LEVEL_UNITS_MODE.DBUV)
        _refLevelLabel = 'dBuV';

    switch(_frequencyType)
    {
        case FREQ_TYPES.CENTER:
            document.getElementById("buttonFrequencyCenter").checked = true;
            document.getElementById("numberFrequency").value = _frequencyCenterHz / _selectedFreqUnits;
            break;
        case FREQ_TYPES.START:
            document.getElementById("buttonFrequencyStart").checked = true;
            document.getElementById("numberFrequency").value = _frequencyStartHz / _selectedFreqUnits;
            break;
        case FREQ_TYPES.STOP:
            document.getElementById("buttonFrequencyStop").checked = true;
            document.getElementById("numberFrequency").value = _frequencyStopHz / _selectedFreqUnits;
            break;
        default:
    }

    document.getElementById("numberFrequencyUnits").innerHTML = _freqLabel;
    document.getElementById("numberSpan").value = _span / _selectedSpanUnits;
    document.getElementById("numberSpanUnits").innerHTML = _spanLabel;
    document.getElementById("numberRefLevel").value = _refLevel;
    document.getElementById("numberRefLevelUnits").innerHTML = _refLevelLabel;
    document.getElementById("radioVidAverageOn").checked = _avgEnabled;
    document.getElementById("radioVidAverageOff").checked = !_avgEnabled;
    document.getElementById("radioPeakHoldOn").checked = _peakHoldEnabled;
    document.getElementById("radioPeakHoldOff").checked = !_peakHoldEnabled;
    document.getElementById("radioChannelPowerOn").checked = _channelPowerEnabled;
    document.getElementById("radioChannelPowerOff").checked = !_channelPowerEnabled;
    document.getElementById("numberChannelPowerSpan").value = _channelPowerSpan / _selectedChannelPowerSpanUnits;
    document.getElementById("numberChannelPowerSpanUnits").innerHTML = _channelPowerSpanLabel;

    document.getElementById("statusCenterFrequency").innerHTML = (_frequencyCenterHz / _selectedFreqUnits).toString();
    document.getElementById("statusFrequencyUnits").innerHTML = " " + _freqLabel;
    document.getElementById("statusSpan").innerHTML = (_span / _selectedSpanUnits).toString();
    document.getElementById("statusSpanUnits").innerHTML = " " + _spanLabel;
    document.getElementById("statusRefLevel").innerHTML = _refLevel.toString();
    document.getElementById("statusRefLevelUnits").innerHTML = " " + _refLevelLabel;
    document.getElementById("statusVidAvg").innerHTML = _avgEnabled ? "ON" : "OFF";
    document.getElementById("statusPeakHold").innerHTML = _peakHoldEnabled ? "ON" : "OFF";
    document.getElementById("statusPowerBandwidth").innerHTML = (_channelPowerSpan / _selectedChannelPowerSpanUnits).toString();
    document.getElementById("statusChannelPowerSpanUnits").innerHTML = " " + _channelPowerSpanLabel;
    document.getElementById("statusPowerMeasurementUnits").innerHTML = "  " + _refLevelLabel;
}

function buttonHold_onclick() {
    document.getElementById("buttonHold").disabled = true;
    document.getElementById("buttonRun").disabled = false;

    _specARunning = false;	// Set the flag to false to stop specA.
    if(setTimeoutId) {
        clearTimeout( setTimeoutId );
        setTimeoutId = 0;
    }
}

function buttonRun_onclick() {
    document.getElementById("buttonRun").disabled = true;
    document.getElementById("buttonHold").disabled = false;

    _specARunning = true;	// Set the flag to true and initiate specA.

    updateSpecAParams();
    // _wfeCommonApi.specAOpen(_coreID);
    // _wfeCommonApi.getSpecAData(_coreID, _specAParams);
    if(_broadcastDiag) _broadcastDiag.getSpectrumData(_specAParams).then(plotSpecA);
    getSpectrumData();
}

function buttonFrequencyCenter_onclick() {
    if (_setFreqMode == FREQ_UNITS_MODE.HZ) {
        _selectedFreqUnits = FREQ_UNITS.HZ;
    }
    else if (_setFreqMode == FREQ_UNITS_MODE.KHZ) {
        _selectedFreqUnits = FREQ_UNITS.KHZ;
    }
    else if (_setFreqMode == FREQ_UNITS_MODE.MHZ) {
        _selectedFreqUnits = FREQ_UNITS.MHZ;
    }
    else if (_setFreqMode == FREQ_UNITS_MODE.GHZ) {
        _selectedFreqUnits = FREQ_UNITS.GHZ;
    }

    document.getElementById("numberFrequency").value = _frequencyCenterHz / _selectedFreqUnits;
    _frequencyType = FREQ_TYPES.CENTER;
}

function buttonFrequencyStart_onclick() {
    if (_setFreqMode == FREQ_UNITS_MODE.HZ) {
        _selectedFreqUnits = FREQ_UNITS.HZ;
    }
    else if (_setFreqMode == FREQ_UNITS_MODE.KHZ) {
        _selectedFreqUnits = FREQ_UNITS.KHZ;
    }
    else if (_setFreqMode == FREQ_UNITS_MODE.MHZ) {
        _selectedFreqUnits = FREQ_UNITS.MHZ;
    }
    else if (_setFreqMode == FREQ_UNITS_MODE.GHZ) {
        _selectedFreqUnits = FREQ_UNITS.GHZ;
    }

    document.getElementById("numberFrequency").value = _frequencyStartHz / _selectedFreqUnits;
    _frequencyType = FREQ_TYPES.START;
}

function buttonFrequencyStop_onclick() {
    if (_setFreqMode == FREQ_UNITS_MODE.HZ) {
        _selectedFreqUnits = FREQ_UNITS.HZ;
    }
    else if (_setFreqMode == FREQ_UNITS_MODE.KHZ) {
        _selectedFreqUnits = FREQ_UNITS.KHZ;
    }
    else if (_setFreqMode == FREQ_UNITS_MODE.MHZ) {
        _selectedFreqUnits = FREQ_UNITS.MHZ;
    }
    else if (_setFreqMode == FREQ_UNITS_MODE.GHZ) {
        _selectedFreqUnits = FREQ_UNITS.GHZ;
    }

    document.getElementById("numberFrequency").value = _frequencyStopHz / _selectedFreqUnits;
    _frequencyType = FREQ_TYPES.STOP;
}

function frequencySet(units) {

    var freqHz = 0;
    _freqLabel = units;
    switch(units) {
        case "GHz":
            _selectedFreqUnits = FREQ_UNITS.GHZ;
            _setFreqMode = FREQ_UNITS_MODE.GHZ;
            break;
        case "MHz":
            _selectedFreqUnits = FREQ_UNITS.MHZ;
            _setFreqMode = FREQ_UNITS_MODE.MHZ;
            break;
        case "kHz":
            _selectedFreqUnits = FREQ_UNITS.KHZ;
            _setFreqMode = FREQ_UNITS_MODE.KHZ;
            break;
        case "Hz":
            _selectedFreqUnits = FREQ_UNITS.HZ;
            _setFreqMode = FREQ_UNITS_MODE.HZ;
            break;
    }

    freqHz = parseFloat(document.getElementById("numberFrequency").value) * _selectedFreqUnits;
    if(!isNaN(freqHz)) {
        try {
            switch(_frequencyType)
            {
                case FREQ_TYPES.CENTER:
                    setFrequencyCenter(freqHz);
                    break;
                case FREQ_TYPES.START:
                    setFrequencyStart(freqHz);
                    break;
                case FREQ_TYPES.STOP:
                    setFrequencyStop(freqHz);
                    break;
                default:
            }
        }
        catch(errorMsg) {
            alert(errorMsg);
        }
    }
}

function spanSet(units) {

    var spanHz = 0;
    _spanLabel = units;
    switch(units) {
        case "GHz":
            _selectedSpanUnits = FREQ_UNITS.GHZ;
            _setFreqMode = FREQ_UNITS_MODE.GHZ;
            break;
        case "MHz":
            _selectedSpanUnits = FREQ_UNITS.MHZ;
            _setFreqMode = FREQ_UNITS_MODE.MHZ;
            break;
        case "kHz":
            _selectedSpanUnits = FREQ_UNITS.KHZ;
            _setFreqMode = FREQ_UNITS_MODE.KHZ;
            break;
        case "Hz":
            _selectedSpanUnits = FREQ_UNITS.HZ;
            _setFreqMode = FREQ_UNITS_MODE.HZ;
            break;
    }
    // Set span
    spanHz = parseFloat(document.getElementById("numberSpan").value) * _selectedSpanUnits;
    if(!isNaN(spanHz)) {
        setSpan(spanHz);
    }
}

function spanSetFull(units) {
    var spanHz = 0;
    _spanLabel = units;
    _selectedSpanUnits = FREQ_UNITS.GHZ;
    _setFreqMode = FREQ_UNITS_MODE.GHZ;

    // Set span
    spanHz = 1 * _selectedSpanUnits;
    setFullSpan(spanHz);
}

function amplitudeSet() {
    var refLevel = 0;
    var units = "dBm";
    var radioDBm = document.getElementById('radioDBm');
    var radioDBmV = document.getElementById('radioDBmV');
    var radioDBuV = document.getElementById('radioDBuV');

    if(radioDBm.checked) {
        units = "dBm";
    }
    else if(radioDBmV.checked) {
        units = "dBmV";
    }
    else if(radioDBuV.checked) {
        units = "dBuV";
    }

    refLevel = parseInt(document.getElementById("numberRefLevel").value, 10);
    if(!isNaN(refLevel)) {
        switch(units) {
            case "dBm":
                _setRefLevelMode = REF_LEVEL_UNITS_MODE.DBM;
                setRefLevel(refLevel);
                break;
            case "dBmV":
                _setRefLevelMode = REF_LEVEL_UNITS_MODE.DBMV;
                setRefLeveldbmv(refLevel);
                break;
            case "dBuV":
                _setRefLevelMode = REF_LEVEL_UNITS_MODE.DBUV;
                setRefLeveldbuv(refLevel);
                break;
        }
        updateSpecAParams();
    }
}

function setFrequencyCenter(value) {
    var span = _span;

    var delta = span / 2;

    if((value < FREQ_MIN)) {
        // Value out of range so just return
        // TODO: popup error msg?
        throw "Value out of range!";
    }
    else if((value - delta) < FREQ_MIN)  {
        // value within range, but the span puts
        // it out of range, so we need to adjust
        // the span accordingly
        span = (value - FREQ_MIN) * 2;
        if(span < SPAN_MIN) {
            span = SPAN_MIN;
        }
        _frequencyCenterHz = FREQ_MIN + (span / 2);
        _frequencyStartHz = FREQ_MIN;
        _frequencyStopHz = FREQ_MIN + span;
    }
    else if(value > FREQ_MAX) {
        // Value out of range so just return
        // TODO: popup error msg?
        throw "Value out of range!";
    }
    else if((value + delta) > FREQ_MAX) {
        // value within range, but the span puts
        // it out of range, so we need to adjust
        // the span accordingly
        span = (FREQ_MAX - value) * 2;
        if(span < SPAN_MIN) {
            span = SPAN_MIN;
        }
        _frequencyCenterHz = FREQ_MAX - (span / 2);
        _frequencyStartHz = FREQ_MAX - span;
        _frequencyStopHz = FREQ_MAX;
        //alert(span);
    }
    else {
        _frequencyCenterHz = value;
        _frequencyStartHz = _frequencyCenterHz - delta;
        _frequencyStopHz = _frequencyCenterHz + delta;
    }

    // Changing frequency may affect span, so update span as well
    setSpan(span);
    // setSpan already calls updateFields and updateSpecAParams so no need to call it here
}

function setFrequencyStart(value) {
    var span = _span;

    if(value > FREQ_MAX) {
        _frequencyStartHz = FREQ_MAX - SPAN_MIN;
        _frequencyStopHz = FREQ_MAX;
        _frequencyCenterHz = FREQ_MAX - (SPAN_MIN / 2);
        span = SPAN_MIN;
    }
    else if(value < FREQ_MIN) {
        _frequencyStartHz = FREQ_MIN;
        span = _frequencyStopHz - _frequencyStartHz;
        if(span < SPAN_MIN) {
            span = SPAN_MIN;
            _frequencyStopHz = span;
        }
        _frequencyCenterHz = _frequencyStartHz + (span / 2);
    }
    else if(_frequencyStopHz < value) {
        span = SPAN_MIN;
        if((value + span) > FREQ_MAX) {
            _frequencyStartHz = FREQ_MAX - span;
            _frequencyStopHz = FREQ_MAX;
            _frequencyCenterHz = _frequencyStartHz + (span / 2);
        }
        else {
            _frequencyStartHz = value;
            _frequencyStopHz = value + span;
            _frequencyCenterHz = _frequencyStartHz + (span / 2);
        }
    }
    else {
        span = _frequencyStopHz - value;
        if(span < SPAN_MIN) {
            span = SPAN_MIN;
            _frequencyStartHz = _frequencyStopHz - span;
        }
        else {
            _frequencyStartHz = value;
        }
        _frequencyCenterHz = _frequencyStartHz + (span / 2);
    }
    // Changing frequency may affect span, so update span as well
    setSpan(span);
    // setSpan already calls updateFields and updateSpecAParams so no need to call it here
}

function setFrequencyStop(value) {
    var span = _span;

    if(value < FREQ_MIN) {
        span = SPAN_MIN;
        _frequencyStopHz = FREQ_MIN + span;
        _frequencyStartHz = FREQ_MIN;
        _frequencyCenterHz = FREQ_MIN + (span / 2);
    }
    else if(value > FREQ_MAX) {
        _frequencyStopHz = FREQ_MAX;
        span = _frequencyStopHz - _frequencyStartHz;
        if(span < SPAN_MIN) {
            span = SPAN_MIN;
            _frequencyStartHz = _frequencyStopHz - span;
        }
        _frequencyCenterHz = _frequencyStartHz + (span / 2);
    }
    else if(_frequencyStartHz > value) {
        span = SPAN_MIN;
        if(value < span) {
            _frequencyStopHz = FREQ_MIN + span;
            _frequencyStartHz = _frequencyStopHz - span;
            _frequencyCenterHz = _frequencyStartHz + (span / 2);
        }
        else {
            _frequencyStopHz = value;
            _frequencyStartHz = value - span;
            _frequencyCenterHz = value + (span / 2);
        }
    }
    else {
        span = value - _frequencyStartHz;
        if(span < SPAN_MIN) {
            span = SPAN_MIN;
            if((value - span) < FREQ_MIN) {
                _frequencyStartHz = FREQ_MIN;
                _frequencyStopHz = FREQ_MIN + span;
            }
            else {
                _frequencyStopHz = value;
                _frequencyStartHz = value - span;
            }
        }
        else {
            _frequencyStopHz = value;
        }
        _frequencyCenterHz = _frequencyStartHz + (span / 2);
    }
    // Changing frequency may affect span, so update span as well
    setSpan(span);
    // setSpan already calls updateFields and updateSpecAParams so no need to call it here
}

function setSpan(value) {
    var delta = value / 2;

    if(value <= SPAN_MAX && value >= SPAN_MIN) {
        if((_frequencyCenterHz + delta) > FREQ_MAX) {
            _span = (FREQ_MAX - _frequencyCenterHz) * 2;
        }
        else if((_frequencyCenterHz - delta) < FREQ_MIN) {
            _span = (_frequencyCenterHz - FREQ_MIN) * 2;
        }
        else {
            _span = value;

        }

        delta = _span / 2;
        _frequencyStartHz = _frequencyCenterHz - delta;
        _frequencyStopHz = _frequencyCenterHz + delta;

        // If _span is less than current channel power span,
        // then set channel power span to _span
        if(_span < _channelPowerSpan) {
            _channelPowerSpan = _span;
        }
    }
    else {
        // Value out of range so just return
        // TODO: popup error msg?
        throw "Value out of range!";
    }
    updateFields();
    updateSpecAParams();
}

function setFullSpan(a) {
    _span = a;
    _frequencyStartHz = 0;
    _frequencyStopHz = a;
    _frequencyCenterHz = a / 2;

    updateFields();
    updateSpecAParams();
}

function setRefLevel(value) {
    var ref_level_max;
    var ref_level_min;

    if (_setRefLevelMode == REF_LEVEL_UNITS_MODE.DBM)
    {
        ref_level_max = REF_LEVEL_MAX;
        ref_level_min = REF_LEVEL_MIN;
    }
    else if (_setRefLevelMode == REF_LEVEL_UNITS_MODE.DBMV)
    {
        ref_level_max = REF_LEVEL_DBMV_MAX;
        ref_level_min = REF_LEVEL_DBMV_MIN;
    }
    else if (_setRefLevelMode == REF_LEVEL_UNITS_MODE.DBUV)
    {
        ref_level_max = REF_LEVEL_DBUV_MAX;
        ref_level_min = REF_LEVEL_DBUV_MIN;
    }

    if(value > ref_level_max) {
        _refLevel = ref_level_max;
    }
    else if(value < ref_level_min) {
        _refLevel = ref_level_min;
    }
    else {
        _refLevel = value;
    }

    // Update the refLevel textbox in case the user's value was out of range
    updateFields();
}

function setRefLeveldbmv(value) {
    if (value > REF_LEVEL_DBMV_MAX) {
        _refLevel = REF_LEVEL_DBMV_MAX;
    } else if (value < REF_LEVEL_DBMV_MIN) {
        _refLevel = REF_LEVEL_DBMV_MIN;
    } else {
        _refLevel = value;
    }

    updateFields();
}
function setRefLeveldbuv(value) {
    if (value > REF_LEVEL_DBUV_MAX) {
        _refLevel = REF_LEVEL_DBUV_MAX;
    } else if (value < REF_LEVEL_DBUV_MIN) {
        _refLevel = REF_LEVEL_DBUV_MIN;
    } else {
        _refLevel = value;
    }

    updateFields();
}

function radioVidAverage_onclick() {
    var avgEnabled = document.getElementById("radioVidAverageOn").checked;

    if(_avgEnabled != avgEnabled) {
        _avgEnabled = avgEnabled;
        resetParams();
        // vidAverage and peakHold cannot run simultaneously, so disable peakHold
        if(_avgEnabled) {
            _peakHoldEnabled = false;
        }
        updateFields();
    }
}

function radioPeakHold_onclick() {
    var peakHoldEnabled = document.getElementById("radioPeakHoldOn").checked;

    if(_peakHoldEnabled != peakHoldEnabled) {
        _peakHoldEnabled = peakHoldEnabled;
        resetParams();
        // vidAverage and peakHold cannot run simultaneously, so disable vidAverage
        if(_peakHoldEnabled) {
            _avgEnabled = false;
        }
        updateFields();
    }
}

function updateSpecAParams() {
    _specAParams.startFrequency = _frequencyStartHz;
    _specAParams.stopFrequency = _frequencyStopHz;
    _specAParams.fftSize = _fftSize;
    // _specAParams.dataLength = _gain;
    _specAParams.numOfSamples = _samples;
    // _specAParams.SpectrumDataReadyCallback = plotSpecA;
}

function resetParams() {
    _fftData.length = 0;
    _fftDataAvg.length = 0;
    _fftDataAvgCounter = 0;
    _fftDataPrev.length = 0;
}

function radioChannelPower_onclick() {
    if(document.getElementById("radioChannelPowerOn").checked) {
        _channelPowerEnabled = true;
    }
    else if(document.getElementById("radioChannelPowerOff").checked) {
        _channelPowerEnabled = false;
        document.getElementById("statusPowerMeasurement").innerHTML = "OFF";
    }
}

function channelPowerSpanSet(units)
{

    var powerSpanHz = 0;

    _channelPowerSpanLabel = units;

    switch(units) {
        case "GHz":
            _selectedChannelPowerSpanUnits = FREQ_UNITS.GHZ;
            _setFreqMode = FREQ_UNITS_MODE.GHZ;
            break;
        case "MHz":
            _selectedChannelPowerSpanUnits = FREQ_UNITS.MHZ;
            _setFreqMode = FREQ_UNITS_MODE.MHZ;
            break;
        case "kHz":
            _selectedChannelPowerSpanUnits = FREQ_UNITS.KHZ;
            _setFreqMode = FREQ_UNITS_MODE.KHZ;
            break;
        case "Hz":
            _selectedChannelPowerSpanUnits = FREQ_UNITS.HZ;
            _setFreqMode = FREQ_UNITS_MODE.HZ;
            break;
    }

    powerSpanHz = parseFloat(document.getElementById("numberChannelPowerSpan").value) * _selectedChannelPowerSpanUnits;
    if(!isNaN(powerSpanHz)) {
        // Make sure power span is not out of range
        if(powerSpanHz > SPAN_MAX) {
            powerSpanHz = SPAN_MAX;
        }
        if(powerSpanHz < SPAN_MIN) {
            powerSpanHz = SPAN_MIN;
        }
        //document.getElementById("numberChannelPowerSpan").value = powerSpanHz / _selectedChannelPowerSpanUnits;
        // If _span is less than powerSpan,
        // set _channelPowerSpan to _span.
        if(_span < powerSpanHz) {
            _channelPowerSpan = _span;
        }
        else {
            _channelPowerSpan = powerSpanHz;
        }
        updateFields();
    }
}

function calculatePower(fftYValuesLinear)
{
    var powerMeas = parseFloat(fftYValuesLinear[_samples / 2]);	// Initialize to power at center freq.
    var textPower;
    var powerStartHz = 0.0;
    var powerStopHz = 0.0;
    var startIndex = 0;
    var stopIndex = 0;

    textPower = document.getElementById("statusPowerMeasurement");

    // Make sure power span is greater than 0 and FStart and FStop are not equal,
    // otherwise return the power at center freq.
    if((_channelPowerSpan > 0) || (_frequencyStopHz != _frequencyStartHz)) {
        powerStartHz = _frequencyCenterHz - (_channelPowerSpan / 2.0);
        powerStopHz = _frequencyCenterHz + (_channelPowerSpan / 2.0);
        startIndex = Math.floor(((powerStartHz - _frequencyStartHz) / (_frequencyStopHz - _frequencyStartHz)) * _samples);
        stopIndex = Math.floor(((powerStopHz - _frequencyStartHz) / (_frequencyStopHz - _frequencyStartHz)) * _samples);

        for (var i=startIndex; i < stopIndex; i++) {
            powerMeas = powerMeas + parseFloat(fftYValuesLinear[i]);
        }
    }

    powerMeas = 10.0*(BRCM.math.log10(powerMeas));
    document.getElementById("statusPowerMeasurementUnits").innerHTML = '  dBm';
    if (_setRefLevelMode === REF_LEVEL_UNITS_MODE.DBMV)
    {
        powerMeas = powerMeas + 48;
        document.getElementById("statusPowerMeasurementUnits").innerHTML = '  dBmV';
    }
    else if (_setRefLevelMode === REF_LEVEL_UNITS_MODE.DBUV)
    {
        powerMeas = powerMeas + 108;
        document.getElementById("statusPowerMeasurementUnits").innerHTML = '  dBuV';
    }
    textPower.innerHTML = powerMeas.toFixed(2);
}

function plotSpecA(data)
{
    var xVal = 0;
    var yVal = 0;
    var fStartHz = _frequencyStartHz;
    var fStopHz = _frequencyStopHz;
    var fftVal = 0.0;
    var fftYValues = [];
    var fftYValuesLinear = [];	// Used to calculate channel power
    var xOffset = 0;	// The x length between each sample
    var i = 0;
    var k = 0;

    _fftData.length = 0;    // Clear FFT data

    for(i = 0; i < data.length; i++) {
        fftYValues[i] = parseFloat(data[i]);
    }

    if(_avgEnabled && (typeof(_fftDataAvg[_fftDataAvgCounter]) === "undefined")) {
        _fftDataAvg[_fftDataAvgCounter] = [];
    }

    xOffset = Math.floor(Math.floor(fStopHz - fStartHz) / (_samples - 1));

    // 1. Process data and append to global FFT data array
    for (var n = 0; n < fftYValues.length; n++) {
        fftVal = fftYValues[n];

        // if (_unitsInDbm) {
            //yVal = fftVal / 256 - 10 * (_gain + 1);
            yVal = fftVal / 256;
        // }
        // else {
            // yVal = fftVal / 256 - 10 * (_gain + 1) + 48.75;
        // }
        xVal = fStartHz + (n * xOffset);
        xVal = xVal / 1000000.0;    // convert xVal to MHz

        if (_avgEnabled) {
            var avg = 0;
            _fftDataAvg[_fftDataAvgCounter].push(yVal);
            for (k = 0; k < _fftDataAvgCounter + 1; k++) {
                avg += _fftDataAvg[k][n];
            }
            avg = avg / (_fftDataAvgCounter + 1);
            if (_setRefLevelMode == REF_LEVEL_UNITS_MODE.DBM) {
                _fftData.push([xVal, avg - _refLevel]);
            }
            else if (_setRefLevelMode == REF_LEVEL_UNITS_MODE.DBMV) {
                _fftData.push([xVal, avg - _refLevel + 48]);
            }
            else if (_setRefLevelMode == REF_LEVEL_UNITS_MODE.DBUV) {
                _fftData.push([xVal, avg - _refLevel + 108]);
            }
        }
        else if (_peakHoldEnabled) {
            var maxPeak = 0;

            if (_fftDataPrev.length >= (n + 1)) {
                if (_fftDataPrev[n] > yVal) {
                    maxPeak = _fftDataPrev[n];
                }
                else {
                    maxPeak = yVal;
                }
            }
            else {
                maxPeak = yVal;
            }
            _fftDataPrev[n] = maxPeak;
            if (_setRefLevelMode == REF_LEVEL_UNITS_MODE.DBM) {
                _fftData.push([xVal, maxPeak - _refLevel]);
            }
            else if (_setRefLevelMode == REF_LEVEL_UNITS_MODE.DBMV) {
                _fftData.push([xVal, maxPeak - _refLevel + 48]);
            }
            else if (_setRefLevelMode == REF_LEVEL_UNITS_MODE.DBUV) {
                _fftData.push([xVal, maxPeak - _refLevel + 108]);
            }
        }
        else {
            if (_setRefLevelMode == REF_LEVEL_UNITS_MODE.DBM) {
                _fftData.push([xVal, yVal - _refLevel]);
            }
            else if (_setRefLevelMode == REF_LEVEL_UNITS_MODE.DBMV) {
                _fftData.push([xVal, yVal - _refLevel + 48]);
            }
            else if (_setRefLevelMode == REF_LEVEL_UNITS_MODE.DBUV) {
                _fftData.push([xVal, yVal - _refLevel + 108]);
            }
        }
    }

    //1. pop all the current markers, and the peak if existed
    var annotations;
    for (i = num - 1; i > 0; i--) {
        //pop the peak
        // FIXME: Is if condition needed?
        if (i == num_peak && _isPeakMarked == 1) {
            annotations = _graph.annotations();
            annotations.pop();
            _graph.setAnnotations(annotations);
        }

        annotations = _graph.annotations();
        annotations.pop();
        _graph.setAnnotations(annotations);
    }

    if (num_peak === 0 && _isPeakMarked == 1) {
        annotations = _graph.annotations();
        annotations.pop();
        _graph.setAnnotations(annotations);
    }

    //2. push the updated markers
    for (i = 1; i < num; i++) {
        //push the peak
        if (i == num_peak + 1 && _isPeakMarked == 1) {
            for (k = 0; k < _fftData.length; k++) {
                if(_fftData[k][0] == marker[0]){
                    break;
                }
            }
            var ann = {
                series: 'PowerSpec',
                xval: marker[0],
                shortText: 0,
                text: "Peak#" + 0 + " :("+ marker[0] +","+ parseInt(_fftData[k][1], 10) +")"
            };
            annotations = _graph.annotations();
            annotations.push(ann);
            _graph.setAnnotations(annotations);
        }

        //push the markers
        for (k = 0; k < _fftData.length; k++) {
            if(_fftData[k][0] == marker[i]){
                break;
            }
        }
        var annObj1 = {
          series: 'PowerSpec',
          xval: marker[i],
          shortText: i,
          text: "Marker#" + i + " :("+ marker[i] +","+ parseInt(_fftData[k][1], 10) +")"
        };
        annotations = _graph.annotations();
        annotations.push(annObj1);
        _graph.setAnnotations(annotations);
    }

    if (num_peak == num - 1 && _isPeakMarked == 1)
    {
        //push the peak
        for (k = 0; k < _fftData.length; k++) {
            if(_fftData[k][0] == marker[0]){
                break;
            }
        }
        var annObj = {
            series: 'PowerSpec',
            xval: marker[0],
            shortText: 0,
            text: "Peak#" + 0 + " :("+ marker[0] +","+ parseInt(_fftData[k][1], 10) +")"
        };
        var anns = _graph.annotations();
        anns.push(annObj);
        _graph.setAnnotations(anns);
    }

    // 2. Plot the data if all the data has been received
    plotData(_fftData);

    if(_avgEnabled) {
        _fftDataAvgCounter++;

        if (_fftDataAvgCounter > (_numOfSampleSets - 1)) {
            // Get rid of oldest data
            _fftDataAvg = shift_array(_fftDataAvg);
            _fftDataAvgCounter--;
        }
    }

    if(_channelPowerEnabled) {
        for(var idx = 0; idx < fftYValues.length; idx++) {
            fftYValuesLinear[idx] = Math.pow(10, (fftYValues[idx] / 2560));
        }
        calculatePower(fftYValuesLinear);
    }

    if(_specARunning) {
        if(_useBBS) {
            // WFE.bbs.api.getSpecAData will call back into plotSpecA before it returns, so
            // to avoid infinite recursion, use setTimeout
            setTimeout(function () {
                    // _wfeCommonApi.getSpecAData(_coreID, _specAParams);
                    if(_broadcastDiag) _broadcastDiag.getSpectrumData(_specAParams).then(plotSpecA);
                }, 300);
        }
        else {
            // WFE.trellis.api.getSpecAData doesn't call back into plotSpecA immediately, so we
            // don't have to worry about infinite recursion
            // _wfeCommonApi.getSpecAData(_coreID, _specAParams);
            if(_broadcastDiag) _broadcastDiag.getSpectrumData(_specAParams).then(plotSpecA);
        }
    }
    else {
        // _wfeCommonApi.specAClose(_coreID);
        document.getElementById("buttonHold").disabled = true;
        document.getElementById("buttonRun").disabled = false;
    }

}

function plotData(data) {
    var options = {};

    // if (_unitsInDbm) {
        options = {
                file: data
            ,	dateWindow: [_frequencyStartHz / 1000000, _frequencyStopHz / 1000000]
            //,	valueRange: [_yMin - (_gain * 10), _yMax - (_gain * 10)]
            ,	valueRange: [_yMin + _refLevel, _yMax + _refLevel]
            ,	ylabel: "dBm(dBmV/dBuV)"
        };
    // }
    // else {
          // options = {
                // file: data,
                // valueRange: [_yMin - (_gain * 10) + 50, _yMax - (_gain * 10) + 50],
                // ylabel: "dBmV"
             // };
    // }

   _graph.updateOptions(options);
}

function shift_array(myArray) {
    var newArray = [];

    for (var i = 1; i < myArray.length; ++i) {
        newArray.push(myArray[i]);
    }
    return newArray;
}

var objdebug = 0;
var setTimeoutId = 0;
function getSpectrumData()
{
    var ipaddr = window.location.hostname;
    var cmip   = urlParams["cmip"];
    var which_plugin = "specA"
    var url = "http://" + ipaddr + ":8888/" + which_plugin + "?cmip=" + cmip;

    if ( ! ipaddr || !cmip ) {
        alert( "Append URL with ?addr=<ip_address_of_stb>&cmip=<address_of_cm>" );
        return false;
    }

    xmlhttpRest = new XMLHttpRequest();
    xmlhttpRest.onreadystatechange= serverHttpResponseRest;
    xmlhttpRest.open("GET",url,true);
    var rc = xmlhttpRest.send(null);
    if ( rc ) {
        console.log( "get_remote_info - Error sending ... " + url );
    }
}

// This function runs as an asynchronous response to a previous server request
function serverHttpResponseRest()
{
    var debug=0;
    var CgiCountObj   = 0;
    var totalXptMbps  = 0;
    var totalAspMbps  = 0;
    var summation = 0;

    if (xmlhttpRest.readyState==4 ) {
        //console.log("serverHttpResponse: got readyState " + xmlhttpRest.readyState + "; status " + xmlhttpRest.status );

        // only if "OK"
        if ( xmlhttpRest.status == 200) {
            //var responseText1 = xmlhttpRest.responseText.replace(/</g,"&lt;"); // fails on ie7, safari
            //var responseText2 = responseText1.replace(/</g,"&lt;"); // fails on ie7, safari

            var entry = (xmlhttpRest.responseText);

            {
                if (objdebug) objdebug.innerHTML = ""; // clear out any previous entries

                {
                    var json = JSON.parse ( entry );
                    var string = JSON.stringify ( json, null, 4 );
                    //objdebug.innerHTML += string;

                    {
                        var data = json.data[0].result;
                        if ( data ) {
                            plotSpecA( data );
                        }
                        if(_specARunning) setTimeoutId = setTimeout( "getSpectrumData()", 1000 );
                    }
                }
            } // if response is not blank
        }

        else {
            var row = 0;
            console.log("There was a problem retrieving JSON data: " + xmlhttpRest.statusText + ". remote_info_idx=" + rest_client_idx );
        }
    } //if (xmlhttpRest.readyState==4 )
} // serverHttpResponseRest
