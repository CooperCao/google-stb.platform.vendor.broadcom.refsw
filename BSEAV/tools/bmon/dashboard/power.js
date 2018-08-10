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
 ******************************************************************************

TODO - make x-axis labels zero-based
       as markers shift off left-hand side, remove them from annotations array
       make sure new data points match x-axis labels
       add RemovedTime markers
*/
var timestampSec = 0; // gets updated each time a plugin response arrives
var timestampSecOffset = 0; // gets updated each time a plugin response arrives ... starts at 0 and keeps counting up
var timestampSecOffsetPrevious = 0; // sometimes the previous is a delta of 2 ... sometimes it is 3
var timestampSecInitialValue = 0; // gets updated only once when the first timestampSec arrives
var passcount = 0;
var MICROSECONDS_BETWEEN_UPDATES = 2000000;

window.chartColors = {
    red: 'rgb(255, 99, 132)',
    orange: '#c45850',
    /* 'rgba(192, 87, 80, 0.5 )' #c45850 */
    orangeborder: '#c45850',
    /* 'rgba(192, 87, 80, 0.5 )' #c45850 */
    yellow: 'rgb(255, 205, 86)',
    green: 'rgba(75, 192, 192, 0.6 )',
    blue: 'rgb(54, 162, 235 )',
    bluefill: 'rgba(54, 162, 235, 0.3 )',
    purple: 'rgb(153, 102, 255)',
    turquoise: '#3cba9f',
    grey: 'rgb(201, 203, 207)'
};

var sendRequestExtra = "";
var sendRequestPlugin = "";
var sendRequestTimeoutId = 0;

function ButtonChange(event) {
    var target = event.target || event.srcElement;
    var id = target.id;
    if (id) {
        var obj = document.getElementById(id);
        if (obj) {
            var IsChecked = 0;
            if (obj.checked)
                IsChecked = 1;
            else
                IsChecked = 0;
            if (id.indexOf("cpu-deactivate-") == 0) {
                var cpu = id[15];
                if (cpu != "0") {
                    sendRequestExtra = "cpu=" + cpu + "&active=" + IsChecked;
                    sendRequestPlugin = "cpu";
                    cpuUpdateInProgress[cpu] = 2; /* tell other functions to ignore button status while this is going on */
                }
            }
        }
    }
}

function RadioClick(event) {
    var target = event.target || event.srcElement;
    var id = target.id;
    if (id) {
        if (id.indexOf("governor") == 0) {
            var which_selected = id[8];
            var offsetParent = target.offsetParent;
            if (offsetParent) {
                buttonText = offsetParent.innerText;
            }
            sendRequestExtra = "governor=" + which_selected;
            sendRequestPlugin = "cpu";
        }

        var temp2 = 0;
    }
    return true;
}

var X_AXIS_POINTS = 120; // this dictates how many points are on the x axis
var lineChartThermal = {
    labels: [],
    datasets: [{
        label: 'Temperature',
        borderColor: window.chartColors.bluefill,
        backgroundColor: window.chartColors.bluefill,
        fill: true,
        data: [],
        type: 'line',
        pointRadius: 0,
        lineTension: 0,
        yAxisID: 'y-axis-2'
    }, {
        label: 'Power',
        borderColor: window.chartColors.green,
        backgroundColor: window.chartColors.green,
        fill: true,
        data: [],
        type: 'line',
        pointRadius: 0,
        lineTension: 0,
        yAxisID: 'y-axis-1',
    }, {
        label: 'OverTemp Threshold',
        borderColor: window.chartColors.orange,
        backgroundColor: window.chartColors.orangeborder,
        fill: false,
        data: [],
        type: 'line',
        pointRadius: 0,
        lineTension: 0,
        yAxisID: 'y-axis-2'
    }]
};
var ThermalParameters = {
    data: lineChartThermal,
    options: {
        responsive: true,
        hoverMode: 'index',
        stacked: false,
        title: {
            display: false,
            text: 'Thermal Throttling Monitor',
        },
        scales: {
            yAxes: [{
                type: 'linear',
                display: true,
                position: 'left',
                id: 'y-axis-1',
                ticks: {
                    suggestedMin: 1.0,
                    suggestedMax: 4.0
                },
                scaleLabel: {
                    display: true,
                    labelString: 'POWER (Watt)',
                    fontColor: window.chartColors.green,
                    fontSize: '24',
                },
            }, {
                type: 'linear',
                display: true,
                position: 'right',
                id: 'y-axis-2',

                // grid line settings
                gridLines: {
                    drawOnChartArea: false, // only want the grid lines for one axis to show up
                },
                ticks: {
                    suggestedMin: 30,
                    suggestedMax: 110
                },
                scaleLabel: {
                    display: true,
                    labelString: 'TEMPERATURE (°C)',
                    fontColor: window.chartColors.blue,
                    fontSize: '24',
                },
            }],
        }
    }
};

var lineChartCpuFrequency = {
    labels: [],
    datasets: [{
        label: 'CPU Frequency',
        borderColor: window.chartColors.blue,
        backgroundColor: window.chartColors.blue,
        fill: false,
        data: [],
        type: 'line',
        pointRadius: 0,
        lineTension: 0,
        yAxisID: 'y-axis-1',
    }, {
        label: 'GRAPHICS V3D',
        borderColor: window.chartColors.green,
        backgroundColor: window.chartColors.green,
        fill: false,
        data: [],
        type: 'line',
        pointRadius: 0,
        lineTension: 0,
        yAxisID: 'y-axis-2',
    }, {
        label: 'GRAPHICS 2D',
        borderColor: window.chartColors.orangeborder,
        backgroundColor: window.chartColors.orange,
        fill: false,
        data: [],
        type: 'line',
        pointRadius: 0,
        lineTension: 0,
        yAxisID: 'y-axis-2',
    }, {
        label: 'USER',
        borderColor: 'gray',
        backgroundColor: 'gray',
        fill: false,
        data: [],
        type: 'line',
        pointRadius: 0,
        lineTension: 0,
        yAxisID: 'y-axis-1',
    }]
};
var CpuFrequencyParameters = {
    data: lineChartCpuFrequency,
    options: {
        responsive: true,
        hoverMode: 'index',
        stacked: false,
        fillOpacity: .3,
        title: {
            display: false,
            text: 'Cooling Agents',
        },
        tooltips: {
            mode: 'x',
            intersect: false
        },

        /* */
        annotation: {
            annotations: [],
            textStyle: {
                fontName: 'Times-Roman',
                fontSize: 18,
                bold: true,
                italic: true,
                color: '#871b47', // The color of the text.
                auraColor: '#d799ae', // The color of the text outline.
                opacity: 0.8 // The transparency of the text.
            }
        },
        /**/

        scales: {
            yAxes: [{
                type: 'linear',
                display: true,
                position: 'left',
                id: 'y-axis-1',
                ticks: {
                    suggestedMin: 0,
                    suggestedMax: 1800
                },

                // grid line settings
                gridLines: {
                    drawOnChartArea: true // only want the grid lines for one axis to show up
                },
                scaleLabel: {
                    display: true,
                    labelString: 'CPU FREQUENCY (MHz)',
                    fontColor: window.chartColors.blue,
                    fontSize: '24',
                },
            }, {
                type: 'linear',
                display: true,
                position: 'right',
                id: 'y-axis-2',

                // grid line settings
                gridLines: {
                    drawOnChartArea: false // only want the grid lines for one axis to show up
                },
                ticks: {
                    suggestedMin: 100,
                    suggestedMax: 800
                },
                scaleLabel: {
                    display: true,
                    labelString: 'GRAPHICS FREQUENCY (MHz)',
                    fontColor: window.chartColors.green,
                    fontSize: '24',
                },
            }],
            /*
            xAxes: [{
               "id": 'x-axis-0',
                type: 'linear'
            }],
            */
        }
    }
};

function lineChartCreateNewXaxis(which_chart, start_index) {
    var new_labels = new Array();
    var new_data = 0;
    //console.log("lineChartCreateNewXaxis - start " + start_index + " ... points " + X_AXIS_POINTS);
    for (var idx = 0; idx < X_AXIS_POINTS; idx++) {
        new_labels[idx] = 0;
        new_data[idx] = 0;
    }

    which_chart.labels = new_labels;

    for (var dataset = 0; dataset < which_chart.datasets.length; dataset++) {
        new_data = new Array();
        for (var idx = 0; idx < X_AXIS_POINTS; idx++) {
            new_data[idx] = 0;
        }
        //console.log("lineChartCreateNewXaxis - new dataset " + dataset);
        which_chart.datasets[dataset].data = new_data;
    }
}

window.onload = function () {
    var ctx_thermal = document.getElementById('canvas_thermal').getContext('2d');
    lineChartCreateNewXaxis(lineChartThermal, 0);

    window.myLine = Chart.Line(ctx_thermal, ThermalParameters);

    var ctx_cpu_frequency = document.getElementById('canvas_cpu_frequency').getContext('2d');

    lineChartCreateNewXaxis(lineChartCpuFrequency, 0);

    var ctx = document.getElementById("voltage");
    window.voltageChart = new Chart(ctx, {
        type: 'bar',
        data: {
            labels: ["AVS-CPU", "AVS-MAIN"],
            datasets: [{
                label: "voltages (volts)",
                backgroundColor: [window.chartColors.orange, window.chartColors.turquoise],
                data: [0, 0]
            }]
        },
        options: {
            barPercentage: 0.7,
            responsive: true,
            legend: {
                display: false
            },
            title: {
                display: true
            },
            //scaleOverride: true,
            //scaleStartValue: 10,
            scales: {
                yAxes: [{
                    ticks: {
                        beginAtZero: true
                    }
                }]
            }
        }
    });

    window.myLineCpuFrequency = Chart.Line(ctx_cpu_frequency, CpuFrequencyParameters);

    sendRequest();
    initializeMvc();
}

var objdebug = 0;
var rest_plugin_idx = 0; // incremented each time we send a restful request to remote client (index to var plugins)
var plugins = ["power", "cpu"]; // trigger these plugins for each 1-second pass
var plugin_time_start = 0;
var plugin_time_end = 0;
var POWER_GRAPH = 1; // index into datasets[] array
var TEMPERATURE_GRAPH = 0; // index into datasets[] array
var THERMAL_THRESHOLD_GRAPH = 2; // index into datasets[] array
var CPU_FREQUENCY_GRAPH = 0; // index into datasets[] array for Cooling Agents
var GRAPHICS_3D_CORE_FREQUENCY = 1; // index into datasets[] array for Cooling Agents
var GRAPHICS_2D_CORE_FREQUENCY = 2; // index into datasets[] array for Cooling Agents
var USER_TRIGGERS_GRAPH = 3; // index into datasets[] array for Cooling Agents
var ResponseCount = 0;

function rtrim(stringToTrim) {
    return stringToTrim.replace(/\s+$/, "");
}

function gettimenow() {
    var local = new Date();
    return (Math.floor(local.getTime()));
}

function sendRequest() {
    var debug = 0;
    var url = "";
    var rc = 0;
    var which_plugin = plugins[rest_plugin_idx];
    var ipaddr = location.host;
    var local = new Date();

    if (rest_plugin_idx == 0) {
        // if we are starting at the first plugin in the list
        plugin_time_start = gettimenow();
    }

    if (sendRequestTimeoutId) {
        clearTimeout(sendRequestTimeoutId);
        sendRequestTimeoutId = 0;
    }

    url = "http://" + ipaddr + ":8888/" + which_plugin;
    if (sendRequestExtra.length && which_plugin == sendRequestPlugin) {
        url += "?" + sendRequestExtra;
        sendRequestExtra = "";
        sendRequestPlugin = "";
    }

    if (which_plugin == "power" && g_clktreeExpand != "") {
        url += "?expand=" + g_clktreeExpand;
    }
    //console.log("sendRequest -" + Number( local.getTime() / 1000 ).toFixed(3) +" ... sending" + url );

    xmlhttpRest = new XMLHttpRequest();
    xmlhttpRest.onreadystatechange = serverHttpResponseRest;
    xmlhttpRest.open("GET", url, true);
    try {
        rc = xmlhttpRest.send(null);
    } catch (err) {
        console.log("sendRequest - Error sending ..." + url);
    }
}

function serverHttpResponseRest() // This function runs as an asynchronous response to a previous server request
{
    var debug = 0;

    //console.log("serverHttpResponse: got readyState" + xmlhttpRest.readyState );

    if (xmlhttpRest.readyState == 4) {
        //console.log("serverHttpResponse: got readyState" + xmlhttpRest.readyState +"; status" + xmlhttpRest.status );

        // only if"OK"
        if (xmlhttpRest.status == 200) {
            ResponseCount++;

            //var responseText1 = xmlhttpRest.responseText.replace(/</g,"&lt;"); // fails on ie7, safari
            //var responseText2 = responseText1.replace(/</g,"&lt;"); // fails on ie7, safari

            var entry = rtrim(xmlhttpRest.responseText);

            {
                if (objdebug) {
                    objdebug.innerHTML = ""; // clear out any previous entries
                }

                if (entry.indexOf("name") > 0) {
                    var json = JSON.parse(entry);
                    var string = JSON.stringify(json, null, 4);
                    var plugin_name = json["name"];
                    var datetime = json["datetime"];
                    var match_found = false;

                    if (plugin_name == "power") {
                        var jsondata = json["data"];
                        var systemPowerMw = 0;
                        var systemPowerScale = 0;
                        var graphicsV3dCoreFrequency = 0;
                        var graphics2dCoreFrequency = 0;
                        var cpuVoltageVolts = 0;
                        var mainVoltageVolts = 0;

                        timestampSec = Math.floor(json["timestampSec"]);
                        if (timestampSecInitialValue == 0) {
                            timestampSecInitialValue = timestampSec;
                        }
                        timestampSecOffsetPrevious = timestampSecOffset;
                        timestampSecOffset = Math.max(timestampSec - timestampSecInitialValue, 0); // do not let this value go negative

                        ShiftLabels();

                        // the data array has many elements; find the"power" element
                        for (var idx = 0; idx < jsondata.length; idx++) {
                            var obj = 0;
                            if (jsondata[idx]["power"]) {
                                obj = jsondata[idx]["power"];
                                /* if (obj['systemPowerMw'] == "NULL") {
                                    $("#idSystemPower").hide();
                                }*/
                                if (obj['systemPowerMw'] && obj['systemPowerMw'] != "NULL") {
                                    systemPowerMw = Number(obj['systemPowerMw'] * 2 / 1000).toFixed(1);
                                }
                                if (obj['systemPowerScale']) {
                                    systemPowerScale = obj['systemPowerScale'];
                                }
                                if (obj['graphicsV3dCoreFrequencyMhz']) {
                                    graphicsV3dCoreFrequency = obj['graphicsV3dCoreFrequencyMhz'];
                                }
                                if (obj['graphics2dCoreFrequencyMhz']) {
                                    graphics2dCoreFrequency = obj['graphics2dCoreFrequencyMhz'];
                                }
                                if (obj['cpuVoltageVolts']) {
                                    cpuVoltageVolts = obj['cpuVoltageVolts'];
                                }
                                if (obj['mainVoltageVolts']) {
                                    mainVoltageVolts = obj['mainVoltageVolts'];
                                }

                                lineChartThermal.datasets[POWER_GRAPH].data.push(systemPowerMw);
                                lineChartCpuFrequency.datasets[GRAPHICS_3D_CORE_FREQUENCY].data.push(graphicsV3dCoreFrequency);
                                lineChartCpuFrequency.datasets[GRAPHICS_2D_CORE_FREQUENCY].data.push(graphics2dCoreFrequency);
                                document.getElementById('CHIP_POWER').innerHTML = systemPowerMw;

                                lineChartThermal.labels.push(timestampSecOffset);
                                lineChartCpuFrequency.labels.push(timestampSecOffset);

                                var graph_length = lineChartCpuFrequency.datasets[GRAPHICS_3D_CORE_FREQUENCY].data.length - 1;
                                //console.log("power - num data points " + graph_length + " ... label " + lineChartCpuFrequency.labels[graph_length]);

                                if (0) {
                                    var num_datapoints = lineChartCpuFrequency.datasets[GRAPHICS_3D_CORE_FREQUENCY].data.length;
                                    if (num_datapoints) {
                                        var labelLastOne = lineChartCpuFrequency.labels[num_datapoints - 1];
                                        if (labelLastOne != timestampSec) { //console.log("time skew ... timestampSec" + timestampSec +" ... labelLastOne" + labelLastOne );
                                        }
                                    }
                                }

                                var temperatureDegreesCelcius = 0;
                                if (obj['temperatureDegreesCelcius']) {
                                    temperatureDegreesCelcius = Number(obj['temperatureDegreesCelcius']).toFixed(1);
                                }
                                lineChartThermal.datasets[TEMPERATURE_GRAPH].data.push(temperatureDegreesCelcius);
                                //console.log("celcius" + temperatureDegreesCelcius );
                                //document.getElementById('TEMPERATURE').innerHTML = temperatureDegreesCelcius +"&deg;";
                                document.getElementById('temparatureTextId').innerHTML = temperatureDegreesCelcius;
                                var tmpGauge = document.getElementById('temperature-gauge');
                                tmpGauge.setAttribute('data-value', temperatureDegreesCelcius);

                                if (window.voltageChart) {
                                    var voltagedata = window.voltageChart["data"];
                                    if (voltagedata) {
                                        var datasets = voltagedata["datasets"];
                                        if (datasets) {
                                            var datasets_data = datasets[0]["data"];
                                            if (datasets_data && datasets_data.length > 1) {
                                                datasets_data[0] = cpuVoltageVolts;
                                                datasets_data[1] = mainVoltageVolts;
                                                window.voltageChart.update();
                                            }
                                        }
                                    }
                                }
                                document.getElementById('CHIP_VOLTAGE').innerHTML = mainVoltageVolts;

                            } else if (jsondata[idx]["softwarePowerState"]) {
                                var objSOFTWARE_POWER_STATE = document.getElementById('SVG_SOFTWARE_POWER_STATE');
                                if (objSOFTWARE_POWER_STATE) {
                                    var SoftwareStateJson = jsondata[idx]["softwarePowerState"];
                                    var keymsg = "";
                                    var count = 0;

                                    GlobalSvg = "";
                                    g_YMax = 0;
                                    //WalkTree(SoftwareStateJson, "SVG_SOFTWARE_POWER_STATE");
                                    scanObject(SoftwareStateJson, "", DEFAULT_X, 0, false);

                                    objSOFTWARE_POWER_STATE.innerHTML = GlobalSvg;
                                    objSOFTWARE_POWER_STATE.style.height = Number(g_YMax + RECTANGLE_HEIGHT + GAP_BETWEEN_ROWS);
                                    objSOFTWARE_POWER_STATE.scrollTop = Number(g_YMax + RECTANGLE_HEIGHT + GAP_BETWEEN_ROWS);
                                    //alert( GlobalSvg );
                                }

                            } else if (jsondata[idx]["dvfs"]) {
                                obj = jsondata[idx]["dvfs"];
                                var cpu = 0;
                                var cpuFrequenciesKhz = obj['cpuFrequenciesKhz'];
                                var cpuFrequencyCurrently = 0;
                                if (cpuFrequenciesKhz)
                                    cpuFrequencyCurrently = cpuFrequenciesKhz[0];
                                var powerSavingSetting = obj['powerSavingSetting'];
                                var cpuFrequencyMhz = (cpuFrequencyCurrently / 1000);

                                lineChartCpuFrequency.datasets[CPU_FREQUENCY_GRAPH].data.push(cpuFrequencyMhz);
                                lineChartCpuFrequency.datasets[USER_TRIGGERS_GRAPH].data.push(0);
                                // align with the bottom of the y axis

                                for (cpu = 0; cpu < cpuFrequenciesKhz.length; cpu++) {
                                    if (cpuFrequenciesKhz) {
                                        cpuFrequencyCurrently = cpuFrequenciesKhz[cpu];
                                    }
                                    cpuFrequencyMhz = (cpuFrequencyCurrently / 1000);
                                    //document.getElementById('CPU_FREQUENCY_' + cpu).innerHTML = cpuFrequencyMhz;

                                    /* if all other cpus are inactive, only cpu 0 will show accurate Mhz */
                                    if ( cpu == 0 ) {
                                        document.getElementById('idCpuFrequency').innerHTML = cpuFrequencyMhz;
                                    }
                                }
                                // if this board does not have 4 CPU cores, disable the buttons for the extra non-existent cores
                                if (cpuFrequenciesKhz.length < 4) {
                                    var obj_cpu = 0;
                                    for (cpu = cpuFrequenciesKhz.length; cpu < 4; cpu++) {
                                        $("#CPU_NAME_" + cpu).remove();
                                        //$("#CPU_FREQUENCY_" + cpu).remove();
                                        $("#CPU_SWITCH_" + cpu).remove();
                                    }
                                }
                            } else if (jsondata[idx]["thermal"]) {
                                obj = jsondata[idx]["thermal"];
                                var overTemperatureThresholdMillidegrees = obj["overTemperatureThresholdMillidegrees"];
                                var overTemperatureThresholdNumber = overTemperatureThresholdMillidegrees / 1000;
                                if (!tmpGauge) {
                                    var tmpGauge = document.getElementById('temperature-gauge');
                                    tmpGauge.setAttribute('data-highlights', '[{"from":' + overTemperatureThresholdNumber + ', "to": 120, "color": "rgba(255, 0, 0, 1)"}]');
                                }

                                lineChartThermal.datasets[THERMAL_THRESHOLD_GRAPH].data.push(overTemperatureThresholdNumber);
                                var coolingAgents = obj['coolingAgents'];

                                PlotCoolingAgentMarkers(coolingAgents);
                                PopulateCoolingAgentTable(jsondata);

                                SetInnerHtml("OVERTEMPERATURE_THRESHOLD", overTemperatureThresholdNumber.toString());
                                SetInnerHtml("OVERTEMPERATURE_DURATION", ConvertToDuration(0));

                            } else if (jsondata[idx]["chipPowerState"]) {
                                var objCHIP_POWER_STATE = document.getElementById('CHIP_POWER_STATE');
                                var chipPowerStateObj = jsondata[idx]["chipPowerState"];
                                if (chipPowerStateObj) {
                                    WalkTree(chipPowerStateObj, "SVG_CHIP_POWER_STATE");
                                }
                            } else if (jsondata[idx]["pllsTopLevel"]) {
                                var objCHIP_POWER_STATE = document.getElementById('CHIP_POWER_STATE_REVERSED');
                                var chipPowerStateObj = jsondata[idx]["pllsTopLevel"];
                                if (chipPowerStateObj) {
                                    WalkTree(chipPowerStateObj, "SVG_CHIP_POWER_STATE_REVERSED");
                                }
                            } else if (jsondata[idx]["standby"]) {
                                var unixtime_msec = (new Date()).getTime();
                                var newdate = new Date();
                                var standbyObj = jsondata[idx]["standby"];
                                if (standbyObj) {
                                    // compute how many seconds ago the box went into standby
                                    var lastUptimeMsec = Number(standbyObj.uptimeSec * 1000);
                                    var lastStandbyTimeDeltaMsec = Number((lastUptimeMsec - standbyObj.lastStandbyTime * 1000));

                                    // compute how many seconds ago the box came out of standby
                                    var lastResumeTimeDeltaMsec = Number((lastUptimeMsec - standbyObj.lastResumeTime * 1000));

                                    var lastStandbyDuration = Math.floor((lastStandbyTimeDeltaMsec) / 1000);
                                    var lastStandbyDurationStr = ConvertToDuration(lastStandbyDuration);

                                    var lastResumeDuration = Math.floor((lastResumeTimeDeltaMsec) / 1000);
                                    var lastResumeDurationStr = ConvertToDuration(lastResumeDuration);

                                    SetInnerHtml("WAKEUP_DURATION", lastResumeDurationStr);
                                    SetInnerHtml("SLEEP_DURATION", ConvertToDuration(standbyObj.lastResumeTime - standbyObj.lastStandbyTime));
                                    SetInnerHtml("SLEEP_STATE", "S" + standbyObj.lastStandbyMode.toString());
                                    SetInnerHtml("RESET_DURATION", ConvertToDuration(standbyObj.uptimeSec));

                                    // extract the first reason in the list
                                    var resetList = standbyObj.resetList.split(",");
                                    if (resetList.length) {
                                        // the first entry in the list is the "last" reason
                                        SetInnerHtml("RESET_REASON", resetList[0]);
                                    }
                                }
                            }
                        }
                    }

                    window.myLine.update();
                    window.myLineCpuFrequency.update();

                    get_remote_info_next();
                }
            }
        } else {
            var row = 0;
            console.log("There was a problem retrieving JSON data:" + xmlhttpRest.statusText + ".");

            setTimeout(sendRequest, 1000);
        }

    }
    //if (xmlhttpRest.readyState==4 )
}
// serverHttpResponseRest

function increment(varname, max_value, reset_value) {
    varname++;
    if (varname >= max_value)
        varname = reset_value;

    return (varname);
}
// increment

var total_msec_missed = 0;

function get_remote_info_next() {
    rest_plugin_idx = increment(rest_plugin_idx, plugins.length, 0);
    var temp = rest_plugin_idx;
    //console.log("get_remote_info_next - rest_plugin_idx" + rest_plugin_idx );

    // if the index has wrapped around to the beginning, wait a second before sending the request
    if (rest_plugin_idx == 0) {
        var msec_to_wait = 0;
        plugin_time_end = gettimenow();
        //console.log("get_remote_info_next - plugin_end -  " + Number( plugin_time_end / 1000 ).toFixed(3) );
        if (plugin_time_start && plugin_time_end) {
            var delta = Number(((plugin_time_end / 1000) - (plugin_time_start / 1000)) * 1000).toFixed(0);
            //console.log("get_remote_info_next - delta" + delta +" ... end" + plugin_time_end/1000 +" - start" + (plugin_time_start/1000) );
            msec_to_wait = Math.min((MICROSECONDS_BETWEEN_UPDATES - (delta * 1000)) / 1000, (MICROSECONDS_BETWEEN_UPDATES / 1000)).toFixed(0);
            if (msec_to_wait < 1000) { //console.log("get_remote_info_next ... msec_to_wait " + msec_to_wait + " ... total_msec_missed " + total_msec_missed);
            }
            if (msec_to_wait < 0) {
                total_msec_missed -= msec_to_wait;
                msec_to_wait = 0; // we have already waited past a full second ... don't wait any more time
            }
        }

        sendRequestTimeoutId = setTimeout("sendRequest()", msec_to_wait);

    } else {
        //console.log("get_remote_info_next - calling get_remote_info ... rest_plugin_idx" + rest_plugin_idx );
        sendRequestTimeoutId = sendRequest();
    }
}

function ShiftLabels() {
    if (lineChartThermal.datasets[TEMPERATURE_GRAPH].data.length >= X_AXIS_POINTS) {
        if (lineChartThermal.labels.length) {
            var new_xcoord = lineChartThermal.labels[0] + (X_AXIS_POINTS * (MICROSECONDS_BETWEEN_UPDATES / 1000000));
        }
        if (lineChartCpuFrequency.labels.length) {
            var new_xcoord = lineChartCpuFrequency.labels[0] + (X_AXIS_POINTS * (MICROSECONDS_BETWEEN_UPDATES / 1000000));
            //console.log("ShiftLabels - new_xcoord" + new_xcoord);
        }
        lineChartThermal.labels.shift(); // remove first array element
        lineChartCpuFrequency.labels.shift(); // remove first array element
        lineChartThermal.datasets[POWER_GRAPH].data.shift();
        lineChartCpuFrequency.datasets[CPU_FREQUENCY_GRAPH].data.shift();
        lineChartCpuFrequency.datasets[USER_TRIGGERS_GRAPH].data.shift();
        lineChartCpuFrequency.datasets[GRAPHICS_3D_CORE_FREQUENCY].data.shift();
        lineChartCpuFrequency.datasets[GRAPHICS_2D_CORE_FREQUENCY].data.shift();
        lineChartThermal.datasets[THERMAL_THRESHOLD_GRAPH].data.shift();
        lineChartThermal.datasets[TEMPERATURE_GRAPH].data.shift();

        // if any of the vertical lines has shifted off the page, hide the line
        if (lineChartCpuFrequency.datasets.length > 3) {
            var verticalLineIdx = 0;
            var lowest_x_axis = lineChartCpuFrequency.labels[0];
            var coolingVerticalLines = lineChartCpuFrequency.datasets.length - 3;
            var points = 0;
            for (verticalLineIdx = 0; verticalLineIdx < coolingVerticalLines; verticalLineIdx++) {
                points = lineChartCpuFrequency.datasets[3 + verticalLineIdx]["data"];
                if (points && points[0]["x"] < lowest_x_axis) {
                    points[0]["x"] = -1;
                    points[1]["x"] = -1;
                }
            }
        }
    }
}

function PopulateCoolingAgentTable(data) {

    var coolingAgents = null;
    var thermal = null;
    var power = null;
    var dfs = null;
    for (var i = 0; i < data.length; i++) {
        if (data[i]["thermal"]) {
            thermal = data[i]["thermal"];
            coolingAgents = thermal['coolingAgents'];
            break;
        }
    }
    for (var i = 0; i < data.length; i++) {
        if (data[i]["power"]) {
            power = data[i]["power"];
            break;
        }
    }
    for (var i = 0; i < data.length; i++) {
        if (data[i]["dvfs"]) {
            dvfs = data[i]["dvfs"];
            break;
        }
    }

    if (coolingAgents && coolingAgents.length) {
        $("#idAgentTableBody").empty();
        for (var coolingAgentIdx = 0; coolingAgentIdx < coolingAgents.length; coolingAgentIdx++) {
            var name = coolingAgents[coolingAgentIdx]["agent" + coolingAgentIdx]["name"];
            var markerAppliedTime = Math.floor(coolingAgents[coolingAgentIdx]["agent" + coolingAgentIdx]["lastAppliedTime"]);
            var markerRemovedTime = Math.floor(coolingAgents[coolingAgentIdx]["agent" + coolingAgentIdx]["lastRemovedTime"]);
            var extraStatus = "";
            if (name == 'm2mc') {
                if (power.graphics2dCoreFrequencyMhz) {
                    extraStatus = power.graphics2dCoreFrequencyMhz + "Mhz";
                }

            } else if (name == "v3d") {
                if (power.graphicsV3dCoreFrequencyMhz) {
                    extraStatus = power.graphicsV3dCoreFrequencyMhz + "Mhz";
                }

            } else if (name == "cpu_pstate") {
                extraStatus = (dvfs.cpuFrequenciesKhz[0].dvfs / 1000) + "Mhz";
            }

            $("#idAgentTableBody").prepend("<tr><td>" + name + "</td><td>" + markerAppliedTime + "</td><td>" + markerRemovedTime + "</td><td>" + extraStatus + "</td></tr>");
        }

    } else {
        /* delete rows */
        $("#idAgentTableBody").empty();
    }

}

function PlotCoolingAgentMarkers(coolingAgents) {
    if (coolingAgents && coolingAgents.length) {
        var coolingAgentIdx = 0;
        var coolingAgentNextLine = 0;
        var coolingAgentsUsed = 0;
        var coolingAgentName = 0;
        var coolingAgentLastAppliedTime = 0;
        var coolingAgentLastRemovedTime = 0;
        var lowest_x_axis = lineChartCpuFrequency.labels[0];
        var highest_x_axis = lineChartCpuFrequency.labels[lineChartCpuFrequency.labels.length - 1];
        var num_points = lineChartCpuFrequency.datasets[CPU_FREQUENCY_GRAPH].data.length;

        RemoveOldMarkers(CpuFrequencyParameters, lowest_x_axis);

        for (coolingAgentIdx = 0; coolingAgentIdx < coolingAgents.length; coolingAgentIdx++) {
            var markerName = "";
            var markerAppliedTime = Math.floor(coolingAgents[coolingAgentIdx]["agent" + coolingAgentIdx]["lastAppliedTime"]) - timestampSecInitialValue;
            var markerRemovedTime = Math.floor(coolingAgents[coolingAgentIdx]["agent" + coolingAgentIdx]["lastRemovedTime"]) - timestampSecInitialValue;
            var ycoord = 0;
            var backgroundColor = "yellow";
            var chartIndex = 0;
            var markerTime = 0;

            for (var marker = 0; marker < 2; marker++) {
                if (marker == 0) {
                    markerTime = markerAppliedTime;
                    markerName = coolingAgents[coolingAgentIdx]["agent" + coolingAgentIdx]["name"] + " (+)" /* CAD + coolingAgentIdx.toString()*/ ;
                } else {
                    markerTime = markerRemovedTime;
                    markerName = coolingAgents[coolingAgentIdx]["agent" + coolingAgentIdx]["name"] + " (-)" /* CAD + coolingAgentIdx.toString()*/ ;
                }
                if ((lowest_x_axis <= markerTime) && (markerTime <= highest_x_axis)) {
                    if (markerName.indexOf("cpu") >= 0) {
                        chartIndex = CPU_FREQUENCY_GRAPH;
                    } else if (markerName.indexOf("m2m") >= 0) {
                        chartIndex = GRAPHICS_2D_CORE_FREQUENCY;
                    } else if (markerName.indexOf("3d") >= 0) {
                        chartIndex = GRAPHICS_3D_CORE_FREQUENCY;
                    } else {
                        /* these are the custom ... user ... triggers */
                        chartIndex = USER_TRIGGERS_GRAPH;
                    }
                    ycoord = lineChartCpuFrequency.datasets[chartIndex].data[num_points - 1];
                    backgroundColor = lineChartCpuFrequency.datasets[chartIndex].backgroundColor;
                    AddNewMarker(CpuFrequencyParameters, markerTime, markerName, backgroundColor, coolingAgentIdx.toString());
                } else { //debuglog("marker" + markerName +" outside window range ..." + markerTime );
                }
            }
        }
    }
}

function AddNewMarker(which_chart, new_value, description, backgroundColor, level) {
    var new_marker = {
        drawTime: "afterDatasetsDraw",
        type: "line",
        id: "",
        mode: "vertical",
        scaleID: "x-axis-0",
        value: 0,
        borderWidth: 2,
        borderColor: 'black', //"rgba(211,211,211, .2)",
        label: {
            content: "",
            enabled: true,
            position: "top",
            backgroundColor: "yellow"
        }
    }
    var modified = false;
    if ((new_value % 2 == 1)) {
        modified = true;
        if (new_value < timestampSecOffset) {
            new_value++;
        } else {
            new_value--;
        }
    }
    new_marker.value = timestampSecOffsetPrevious;
    new_marker.id = description + level;
    new_marker.label.content = description;
    new_marker.label.backgroundColor = backgroundColor;

    // make sure this marker is not already in the list
    var markerIdx = 0;
    var markerObj = 0;
    var markerFound = false;

    for (markerIdx = 0; markerIdx < which_chart.options.annotation.annotations.length; markerIdx++) {
        markerObj = which_chart.options.annotation.annotations[markerIdx];
        if (markerObj) {
            if (markerObj.id == new_marker.id) {
                markerFound = true;
                break;
            }
        }
    }

    if (markerFound == false) {
        which_chart.options.annotation.annotations.push(new_marker);
        var lowest_x_axis = lineChartCpuFrequency.labels[0];
        var highest_x_axis = lineChartCpuFrequency.labels[lineChartCpuFrequency.labels.length - 1];
        var msg = "Marker " + description + " ... value " + new_value + " ... TimeOffset " + timestampSecOffset + " ... num annotations " + which_chart.options.annotation.annotations.length + " ... Xaxis " + lowest_x_axis + " .. " + highest_x_axis + " ... Prev " + timestampSecOffsetPrevious;
        //console.log(msg);
        //debuglog(msg);
    }

}

function debuglog(newmsg) {
    if (document.getElementById('debug_text')) {
        document.getElementById('debug_text').innerHTML += newmsg + "\n";
    }
}
// Remove any existing markers that have shifted off the left hand side of the window
function RemoveOldMarkers(which_chart, lowest_x_axis) {
    var markerIdx = 0;
    var markerObj = 0;

    for (markerIdx = 0; markerIdx < which_chart.options.annotation.annotations.length; markerIdx++) {
        markerObj = which_chart.options.annotation.annotations[markerIdx];
        if (markerObj) {
            if (markerObj.value <= lowest_x_axis) {
                //console.log("Removing marker " + which_chart.options.annotation.annotations[markerIdx].id);
                which_chart.options.annotation.annotations.splice(markerIdx, 1);
            }
        }
    }
}

function BCOOL_THERMAL_MONITORING(event) {
    var obj = document.getElementById('debug_text');
    if (obj) {
        var temp = obj.style.visibility;
        if (obj.style.visibility == "hidden") {
            obj.style.display = '';
            obj.style.visibility = '';
        } else {
            obj.style.display = 'none';
            obj.style.visibility = 'hidden';
        }
        var temp2 = 1;
    }
}

/* new mvc code start here */
/* call this function from onload */
function initializeMvc() {
    window.controllerResource = [{
        uri: "wifi/powerStats",
        uri1: null,
        pullRateMSec: 2000,
        xmlHttp: null,
        timeOutId: null,
        visible: true,
        dataHandler: function (index) {
            wifiPOWSViewHandler(index);
        },
        timeoutHandler: function (index) {
            commonTimeOutHandler(index);
        },

    }, {
        uri: "cpu",
        uri1: null,
        pullRateMSec: 1000,
        xmlHttp: null,
        timeOutId: null,
        visible: true,
        dataHandler: function (index) {
            cpuCardViewHandler(index);
        },
        timeoutHandler: function (index) {
            commonTimeOutHandler(index);
        },
        /* customer properties */
        numCpus: 0,
        jsonTextCtx: null,
        startingTimeStampSec: 0,
        cpuUtilizationLineChart: new Chart(document.getElementById("cpuUtilizationLineChartId"), {
            type: 'line',
            data: {
                labels: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                datasets: [{
                    data: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                    label: "cpu3",
                    borderColor: "rgba(62, 149, 205, 0.2)",
                    backgroundColor: "rgba(62, 149, 205, 0.2)",
                    fill: false
                }, {
                    data: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                    label: "cpu2",
                    borderColor: "rgba(232, 195, 185, 0.2)",
                    backgroundColor: "rgba(232, 195, 185, 0.2)",
                    fill: false
                }, {
                    data: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                    label: "cpu1",
                    borderColor: "rgba(60, 186, 159, 0.2)",
                    backgroundColor: "rgba(60, 186, 159, 0.2)",
                    fill: false
                }, {
                    data: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                    label: "cpu0",
                    borderColor: "rgba(196, 88, 80, 0.2)",
                    backgroundColor: "rgba(196, 88, 80, 0.2)",
                    fill: false
                }, {
                    data: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                    label: "Total",
                    borderColor: "#8e5ea2",
                    backgroundColor: "#8e5ea2",
                    fill: false
                }]
            },
            options: {
                title: {
                    display: false,
                    text: 'CPU Utilization'
                },
                scales: {
                    yAxes: [{
                        type: 'linear',
                        display: true,
                        position: 'left',
                        id: 'y-axis-3',
                        ticks: {
                            suggestedMin: 0,
                            suggestedMax: 100
                        },
                        scaleLabel: {
                            display: false,
                        },
                    }]
                }
            }
        })
    }, {
        uri: "hdmi_output/output/videoFormat",
        uri1: "hdmi_output/output/videoFormat",
        pullRateMSec: 1000,
        xmlHttp: null,
        timeOutId: null,
        visible: true,
        dataHandler: function (index) {
            coolingAgentsViewHandler(index);
        },
        timeoutHandler: function (index) {
            commonTimeOutHandler(index);
        },
        /* custom properties */
        displayFormatCtx: document.getElementById("coolingAgentsDisplayFormatId"),
        decodeStatusCtx: document.getElementById("coolingAgentsDecodeStatusId")
    }, {
        uri: "wifi?POWSWindow=",
        pullRateMSec: 1000,
        xmlHttp: null,
        timeOutId: null,
        visible: true,
        dataHandler: function (index) {
            wifiPOWSUpdateHandler(index);
        },
        timeoutHandler: function (index) {
            commonTimeOutHandler(index);
        },
    }, {
        uri: "network",
        pullRateMSec: 2000,
        xmlHttp: null,
        timeOutId: null,
        visible: true,
        dataHandler: function (index) {
            throughputUpdateHandler(index);
        },
        timeoutHandler: function (index) {
            commonTimeOutHandler(index);
        },
        startingTimeStampSec: 0,
        startingMbps: 0,
        throughputLineChart: new Chart(document.getElementById("throughputLineChartId"), {
            type: 'line',
            data: {
                labels: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                datasets: [{
                    data: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                    label: "Mbps",
                    borderColor: "rgba(62, 149, 205, 1)",
                    backgroundColor: "rgba(62, 149, 205, 1)",
                    fill: false
                }]
            },
            options: {
                title: {
                    display: false,
                    text: 'CPU Utilization'
                },
                scales: {
                    yAxes: [{
                        type: 'linear',
                        display: true,
                        position: 'left',
                        id: 'y-axis-3',
                        ticks: {
                            suggestedMin: 0,
                            suggestedMax: 100
                        },
                        scaleLabel: {
                            display: false,
                        },
                    }]
                }
            }
        })
    }, {
        uri: "screenshot",
        pullRateMSec: 5000,
        xmlHttp: null,
        timeOutId: null,
        visible: true,
        dataHandler: function (index) {
            screenshotUpdateHandler(index);
        },
        timeoutHandler: function (index) {
            commonTimeOutHandler(index);
        },
    }];
    /* pows */
    startDataCollection(0);
    /* cpu */
    startDataCollection(1);
    /* cooling agents */
    startDataCollection(2);
    startDataCollection(4);
    startDataCollection(5);
}

/* view handlers */
function screenshotUpdateHandler(index) {
    var resource = window.controllerResource[index];
    httpRequest = resource.xmlHttp;
    if (httpRequest.status == 200 && httpRequest.readyState == 4) {
        var responseText = httpRequest.responseText;
        if (!responseText)
            return;
        var myObj = JSON.parse(responseText);
        if (myObj.data && myObj.data.length) {
            $("#idScreenshotImage").attr("src", myObj.data[0].uri + "?timestamp=" + new Date().getTime());
        }
    }
}

function coolingAgentsViewHandler(index) {
    var resource = window.controllerResource[index];
    httpRequest = resource.xmlHttp;
    if (httpRequest.status == 200 && httpRequest.readyState == 4) {
        var responseText = httpRequest.responseText;
        if (!responseText)
            return;
        var myObj = JSON.parse(responseText);
        resource.displayFormatCtx.innerText = myObj.data[0].output.videoFormat;
    }
}

function wifiPOWSViewHandler(index) {
    var resource = window.controllerResource[index];
    httpRequest = resource.xmlHttp;
    if (httpRequest.status == 200 && httpRequest.readyState == 4) {
        var dis = 0;
        var responseText = httpRequest.responseText;
        if (!responseText)
            return;
        var myObj = JSON.parse(responseText);
        if (myObj.data[0].powerStats.POWSWindow == 0)
            dis = 100;
        else
            dis = myObj.data[0].powerStats.POWSWindow;
        document.getElementById('wifiPowsJsonId').innerHTML = dis;
        startDataCollection(4);
    }
    var slider = document.getElementById("myRange");
    var output = document.getElementById("Value");
    output.innerHTML = slider.value;
    slider.oninput = function () {
        var pows = 0;
        var resource = window.controllerResource[3];
        if (resource.timeOutId) {
            clearTimeout(resource.timeOutId);
            resource.timeOutId = null;
        }
        if (resource.xmlHttp) {
            resource.xmlHttp.abort();
            resource.xmlHttp = null;
        }
        if (!resource.visible) {
            return;
        }
        resource.xmlHttp = new XMLHttpRequest();
        resource.xmlHttp.onreadystatechange = function () {
            httpRequest = this;
            if (httpRequest.status == 200 && httpRequest.readyState == 4) {
                if (!httpRequest.responseText) {
                    return;
                }
                /* dispatch callback */
                resource.dataHandler(index);
            }
        };
        resource.xmlHttp.timeout = 5000;
        resource.xmlHttp.ontimeout = function () {
            resource.timeoutHandler(index);
        };
        if (slider.value > 80)
            pows = 0;
        else if (slider.value < 10)
            pows = 10;
        else
            pows = slider.value;
        var request = "http://" + location.host + ":8888";
        request += '/' + resource.uri + pows;
        resource.xmlHttp.open("GET", request, true);
        console.log("request " + request);
        ScheduleNextRequest(index, resource.pullRateMSec);
        resource.xmlHttp.send(null);
    }
}

function wifiPOWSUpdateHandler(index) {
    var resource = window.controllerResource[index];
    httpRequest = resource.xmlHttp;
    if (httpRequest.status == 200 && httpRequest.readyState == 4) {
        var responseText = httpRequest.responseText;
        if (!responseText)
            return;
        var myObj = JSON.parse(responseText);
    }
}

function throughputUpdateHandler(index) {
    var resource = window.controllerResource[index];
    httpRequest = resource.xmlHttp;
    if (httpRequest.status == 200 && httpRequest.readyState == 4) {
        var responseText = httpRequest.responseText;
        if (!responseText)
            return;
        var myObj = JSON.parse(responseText);
        if (resource.startingTimeStampSec == 0) {
            resource.startingTimeStampSec = myObj.timestampSec;
        }
        var newLabel = Math.round(myObj.timestampSec - resource.startingTimeStampSec);
        resource.throughputLineChart.data.labels.push(newLabel);
        resource.throughputLineChart.data.labels.shift();
        for (var i = 0; i < myObj.data[0].networks.length; i++) {
            if (myObj.data[0].networks[i].name == "wlan0") {
                if (resource.startingMbps)
                    resource.throughputLineChart.data.datasets[0].data.push(((myObj.data[0].networks[i].rx_bytes - resource.startingMbps) * 8) / (resource.pullRateMSec * 1000));
                resource.startingMbps = myObj.data[0].networks[i].rx_bytes;
                resource.throughputLineChart.data.datasets[0].data.shift();
            }
        }
        resource.throughputLineChart.update();
    }
}

function cpuCardViewHandler(index) {
    var resource = window.controllerResource[index];
    var responseText = resource.xmlHttp.responseText;
    if (!responseText) {
        return;
    }
    var myObj = JSON.parse(responseText);
    if (myObj.data.length) {
        //console.log( "cpuCardViewHandler" );
        if (resource.numCpus == 0) {
            resource.numCpus = myObj.data[0].total_cpus;
            var n = resource.cpuUtilizationLineChart.data.datasets.length - myObj.data[0].total_cpus - 1;
            resource.cpuUtilizationLineChart.data.datasets.splice(0, n);
            resource.cpuUtilizationLineChart.update();
        }
        if (resource.startingTimeStampSec == 0) {
            resource.startingTimeStampSec = myObj.timestampSec;
        }
        /* update label */
        var newLabel = Math.round(myObj.timestampSec - resource.startingTimeStampSec);
        resource.cpuUtilizationLineChart.data.labels.push(newLabel);
        resource.cpuUtilizationLineChart.data.labels.shift();

        /* compute the cpu numbers & push and shift the elements */
        updateCpuUtilization(myObj.data, resource.cpuUtilizationLineChart.data.datasets);

        /* make sure the CPU buttons match the current active/inactive state of each CPU */
        updateCpuButtons(myObj.data);

        resource.cpuUtilizationLineChart.update();

    } else { //invalid data
    }

}

function commonDataHandler(index) {
    var resource = window.controllerResource[index];
    if (resource.jsonTextCtx == null) {
        return;
    }
    var httpRequest = resource.xmlHttp;
    var responseText = httpRequest.responseText;
    if (!responseText) {
        return;
    }
    var myObj = JSON.parse(responseText);
    var myJSON = JSON.stringify(myObj, null, 3);
    resource.jsonTextCtx.innerHTML = '<pre style=\"color:white\">' + myJSON + '</pre>';
}

function commonTimeOutHandler(index) {
    var resource = window.controllerResource[index];
    if (resource.jsonTextCtx == null) {
        return;
    }
    resource.jsonTextCtx.innerHTML = '<pre style=\"color:white\">' + "Connection Timeout: Rest server failed to respond to the request" + '</pre>';
}

function sendRestRequest(index) {
    var resource = window.controllerResource[index];
    if (resource.timeOutId) {
        clearTimeout(resource.timeOutId);
        resource.timeOutId = null;
    }
    if (resource.xmlHttp) {
        resource.xmlHttp.abort();
        resource.xmlHttp = null;
    }
    if (!resource.visible) {
        return;
    }
    resource.xmlHttp = new XMLHttpRequest();
    resource.xmlHttp.onreadystatechange = function () {
        httpRequest = this;
        if (httpRequest.status == 200 && httpRequest.readyState == 4) {
            if (!httpRequest.responseText) {
                return;
            }
            /* dispatch callback */
            resource.dataHandler(index);
        }
    };
    resource.xmlHttp.timeout = 5000;
    resource.xmlHttp.ontimeout = function () {
        resource.timeoutHandler(index);
    };
    var request = "http://" + location.host + ":8888";
    request += '/' + resource.uri;
    resource.xmlHttp.open("GET", request, true);
    ScheduleNextRequest(index, resource.pullRateMSec);
    resource.xmlHttp.send(null);
}

function startDataCollection(index) {
    var resource = window.controllerResource[index];
    resource.visible = true;
    sendRestRequest(index);
}

function stopDataCollection(index) {
    var resource = window.controllerResource[index];
    resource.visible = false;
    if (resource.timeOutId) {
        clearTimeout(resource.timeOutId);
        resource.timeOutId = null;
    }
    if (resource.xmlHttp) {
        resource.xmlHttp.abort();
        resource.xmlHttp = null;
    }
}

function ScheduleNextRequest(index, delay) {
    var resource = window.controllerResource[index];
    if (resource.timeOutId) {
        clearTimeout(resource.timeOutId);
        resource.timeOutId = null;
    }
    if (delay) {
        resource.timeOutId = setTimeout(function () {
            sendRestRequest(index);
        }, delay);
    }
}
/* end of mvc code */

/*
   The CPU plugin returns two structures. The first structure contains num_active_cpus, total_cpus, and uptime_msec.
   The second structure is an array. Each array element contains the times for idle, user, system, etc. These times
   are used to compute the utilization.
*/
function updateCpuUtilization(jsondata, jsonDatasets) {
    var cpu_utilization = 0;
    var cpu_total_idx = jsonDatasets.length - 1;

    cpu_utilization = computeCpuUtilization(jsondata, jsonDatasets);
    //console.log( "updateCpuUtilization - " + cpu_utilization );

    document.getElementById("idCpuUtilizationSpan").innerHTML = cpu_utilization;
}

var countClocksTotal = 0;
var countClocksHot = 0;

/* This function will scan the elements in the specified object and count how many clock
 * values are set on. The resulting number will be used to determine what color to display
 * in the tree representation. */
function countClocks(object) {
    var key = "";
    var clock = 0;
    var new_values = 0;

    for (key in object) {
        var temp2 = key;
        if (key == "clock") {
            clock = object[key].toString();
            countClocksTotal++;
            if (clock == 1 || clock == "1") {
                countClocksHot++;
            }
        } else if (key == "sram") {} else if (key == "freqHz") {} else if (key == "phy") {} else {
            var temp3 = object[key];
            countClocks(object[key]);
        }
    }

    return;
}

/* This is a recursive function that will scan the specified object and all sub-objects that
 * it finds. When a final "leaf" node is found, this function will output an SVG rectange to
 * the global variable GlobalSvg using the API DrawRectange. Along with each rectange is a
 * text value that describes the name of the leaf object. */
function scanObject(object, path, X, Y, bShowBlock) {
    var key = 0;
    var clock = "";
    var sram = "";
    var freqHz = "";
    var freqMHz = 0;
    var Ylocal = Y;
    var Xlocal = X;
    var node = {
        "register": "",
        "field": "",
        "polarity": 0,
        "X": 0,
        "Y": 0
    };
    var offset = 40;

    for (key in object) {
        var temp2 = key;
        if ( key != "phy") {
            if (key == "clock") {
                if (bShowBlock) {
                    clock = object[key].toString();

                    // if this clock structure also has a frequency specified, add the frequency to the clock name
                    // for example ... graphics, transport, & videoDecoder
                    if (object["freqHz"]) {
                        freqMHz = Math.round(object["freqHz"] / 1000000);
                        node.register = path + key + " (" + freqMHz + " Mhz)";
                    } else {
                        node.register = path + key;
                    }
                    if (clock == 1 || clock == "1") {
                        node.state = 1;
                    } else {
                        node.state = 0;
                    }
                    Ylocal = DrawRectangle(node, Xlocal, Ylocal, 0);
                    Ylocal += GAP_BETWEEN_ROWS; // value returned from DrawRectangle() does not account for the space between rows
                }
            } else if (key == "sram") {
                sram = object[key].toString();
            } else if (key == "freqHz") {
                freqHz = object[key].toString();
                freqMHz = Math.round(freqHz / 1000000);
            } else {
                var temp3 = object[key];
                var separator = "";
                if (path.length) {
                    separator = "-&gt; ";
                }

                // if the separator is already in the name, do not add another one
                if (path.indexOf(separator) > 0) {
                    node.register = path + key + separator;
                } else {
                    node.register = key + separator;
                }

                // if this is one of the highl-level blocks
                if (Xlocal == DEFAULT_X) {
                    countClocksTotal = 0;
                    countClocksHot = 0;
                    countClocks(object[key]);

                    node.register = key + " (" + countClocksHot + " of " + countClocksTotal + ")";

                    DrawRectangle(node, Xlocal, Ylocal, 0);
                    Ylocal += RECTANGLE_HEIGHT;

                    if (g_clktreeExpand.indexOf(key) >= 0) {
                        bShowBlock = true;
                        if (key == "display") {
                            var temp3 = 0;
                        }
                    } else {
                        bShowBlock = false;
                    }
                } else {}

                scanObject(object[key], node.register, Xlocal + 40, Ylocal, bShowBlock);

                Ylocal = g_YMax;
            }
        } else {}
    }
}

function formatDate(date) {
    var monthNames = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];

    var day = date.getDate();
    var monthIndex = date.getMonth();
    var year = date.getFullYear();
    var tzoffset = date.getTimezoneOffset();
    var date_as_string = date.toString();
    var dash_pos = date_as_string.indexOf('-');
    var date_time_tz = date_as_string.substr

    return monthNames[monthIndex] + ' ' + day + ' ' + year + ' ' + date.getHours().padZero() + ":" + date.getMinutes().padZero() + ":" + date.getSeconds().padZero();
}

Number.prototype.padZero = function (len) {
    var s = String(this);
    var c = '0';
    len = len || 2;
    while (s.length < len) {
        s = c + s;
    }
    return s;
}

/* This function will convert the specified number of seconds to a string that contains hours,
 * minutes, and seconds ... like 2:15:49. */
function ConvertToDuration(seconds) {
    var DurationHour = Math.floor(seconds / 3600);
    var DurationMin = Math.floor((seconds - DurationHour * 3600) / 60);
    var DurationSec = Math.floor(seconds % 60);

    return DurationHour + ":" + DurationMin.padZero() + ":" + DurationSec.padZero();
}

/**
 *  Function: This function will set the innerHTML of the specified element with the value provided.
 **/
function SetInnerHtml(targetId, newvalue) {
    var obj = document.getElementById(targetId);
    if (obj) {
        obj.innerHTML = newvalue;
    }
    return true;
}
