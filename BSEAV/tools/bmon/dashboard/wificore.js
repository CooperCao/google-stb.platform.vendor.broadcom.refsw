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
 ******************************************************************************/

var intTimer;
var ampduTimer;
var chanMapTimer;
var surveyTimer;

// First, checks if it isn't implemented yet.
  String.prototype.formate = function() {
    var args = arguments;
    return this.replace(/{(\d+)}/g, function(match, number) {
      return typeof args[number] != 'undefined'
        ? args[number]
        : match
      ;
    });
  };

function parseAmpdu(ampdu)
{
    var k, mcs_count = 12;
    sceneType = 0;
    if (myObj.data !== undefined) {
        for (k = 0; k < mcs_count; k++) {
                /* rx sgi */
                sceng0.v[mcs_count * 0 + k].y = ampdu.rx[1].sgi[0].ant[k];
                sceng0.v[mcs_count * 1 + k].y = ampdu.rx[1].sgi[1].ant[k];
                sceng0.v[mcs_count * 2 + k].y = ampdu.rx[1].sgi[2].ant[k];
                sceng0.v[mcs_count * 3 + k].y = ampdu.rx[1].sgi[3].ant[k];
                /* rx vht */
                sceng0.v[mcs_count * 4 + k].y = ampdu.rx[0].vht[0].ant[k];
                sceng0.v[mcs_count * 5 + k].y = ampdu.rx[0].vht[1].ant[k];
                sceng0.v[mcs_count * 6 + k].y = ampdu.rx[0].vht[2].ant[k];
                sceng0.v[mcs_count * 7 + k].y = ampdu.rx[0].vht[3].ant[k];
                /* tx sgi */
                sceng1.v[mcs_count * 0 + k].y = ampdu.tx[1].sgi[0].ant[k];
                sceng1.v[mcs_count * 1 + k].y = ampdu.tx[1].sgi[1].ant[k];
                sceng1.v[mcs_count * 2 + k].y = ampdu.tx[1].sgi[2].ant[k];
                sceng1.v[mcs_count * 3 + k].y = ampdu.tx[1].sgi[3].ant[k];
                /* tx vht */
                sceng1.v[mcs_count * 4 + k].y = ampdu.tx[0].vht[0].ant[k];
                sceng1.v[mcs_count * 5 + k].y = ampdu.tx[0].vht[1].ant[k];
                sceng1.v[mcs_count * 6 + k].y = ampdu.tx[0].vht[2].ant[k];
                sceng1.v[mcs_count * 7 + k].y = ampdu.tx[0].vht[3].ant[k];

                 //sceng0.v[mcs_count * 0 + k].y = myObj.data[0].ampduData.rxtx0[0].vhtsgi0[0].ants0[k]["mcs"+k]; // = 15;
                 //sceng0.v[mcs_count * 1 + k].y = myObj.data[0].ampduData.rxtx0[0].vhtsgi0[1].ants1[k]["mcs"+k]; // = 20;
                 //sceng0.v[mcs_count * 2 + k].y = myObj.data[0].ampduData.rxtx0[0].vhtsgi0[2].ants2[k]["mcs"+k]; // = 25;
                 //sceng0.v[mcs_count * 3 + k].y = myObj.data[0].ampduData.rxtx0[0].vhtsgi0[3].ants3[k]["mcs"+k]; // = 30;

                 ///* "named AMPDU struct (JSON) " */
                 //sceng0.v[mcs_count * 4 + k].y = myObj.data[0].ampduData.rxtx0[1].vhtsgi1[0].ants0[k]["mcs"+k]; // = 45;
                 //sceng0.v[mcs_count * 5 + k].y = myObj.data[0].ampduData.rxtx0[1].vhtsgi1[1].ants1[k]["mcs"+k]; // = 50;
                 //sceng0.v[mcs_count * 6 + k].y = myObj.data[0].ampduData.rxtx0[1].vhtsgi1[2].ants2[k]["mcs"+k]; // = 55;
                 //sceng0.v[mcs_count * 7 + k].y = myObj.data[0].ampduData.rxtx0[1].vhtsgi1[3].ants3[k]["mcs"+k]; // = 60;

                 ///* "named AMPDU struct (JSON) "  */
                 //sceng1.v[mcs_count * 0 + k].y = myObj.data[0].ampduData.rxtx1[0].vhtsgi0[0].ants0[k]["mcs" + k]; // = 15;
                 //sceng1.v[mcs_count * 1 + k].y = myObj.data[0].ampduData.rxtx1[0].vhtsgi0[1].ants1[k]["mcs" + k]; // = 20;
                 //sceng1.v[mcs_count * 2 + k].y = myObj.data[0].ampduData.rxtx1[0].vhtsgi0[2].ants2[k]["mcs" + k]; // = 25;
                 //sceng1.v[mcs_count * 3 + k].y = myObj.data[0].ampduData.rxtx1[0].vhtsgi0[3].ants3[k]["mcs" + k]; // = 30;

                 ///* "named AMPDU struct (JSON) " */
                 //sceng1.v[mcs_count * 4 + k].y = myObj.data[0].ampduData.rxtx1[1].vhtsgi1[0].ants0[k]["mcs" + k]; // = 45;
                 //sceng1.v[mcs_count * 5 + k].y = myObj.data[0].ampduData.rxtx1[1].vhtsgi1[1].ants1[k]["mcs" + k]; // = 50;
                 //sceng1.v[mcs_count * 6 + k].y = myObj.data[0].ampduData.rxtx1[1].vhtsgi1[2].ants2[k]["mcs" + k]; // = 55;
                 //sceng1.v[mcs_count * 7 + k].y = myObj.data[0].ampduData.rxtx1[1].vhtsgi1[3].ants3[k]["mcs" + k]; // = 60;

      //           /* rx normal mcs (non-JSON) */
      //           sceng0.v[mcs_count * 0 + k].y = myObj.data[0].ampduData[0][0][0][0][k];
      //           sceng0.v[mcs_count * 1 + k].y = myObj.data[0].ampduData[0][0][0][1][k];
      //           sceng0.v[mcs_count * 2 + k].y = myObj.data[0].ampduData[0][0][0][2][k];
      //           sceng0.v[mcs_count * 3 + k].y = myObj.data[0].ampduData[0][0][0][3][k];
      //           /* rx sgi mcs (non-JSON) */
      //           sceng0.v[mcs_count * 4 + k].y = myObj.data[0].ampduData[0][1][0][0][k];
      //           sceng0.v[mcs_count * 5 + k].y = myObj.data[0].ampduData[0][1][0][1][k];
      //           sceng0.v[mcs_count * 6 + k].y = myObj.data[0].ampduData[0][1][0][2][k];
      //           sceng0.v[mcs_count * 7 + k].y = myObj.data[0].ampduData[0][1][0][3][k];
      //           /* tx normal mcs (non-JSON) */
      //           sceng1.v[mcs_count * 0 + k].y = myObj.data[0].ampduData[1][0][0][0][k];
      //           sceng1.v[mcs_count * 1 + k].y = myObj.data[0].ampduData[1][0][0][1][k];
      //           sceng1.v[mcs_count * 2 + k].y = myObj.data[0].ampduData[1][0][0][2][k];
      //           sceng1.v[mcs_count * 3 + k].y = myObj.data[0].ampduData[1][0][0][3][k];
      //           /* tx sgi mcs (non-JSON) */
      //           sceng1.v[mcs_count * 4 + k].y = myObj.data[0].ampduData[1][1][0][0][k];
      //           sceng1.v[mcs_count * 5 + k].y = myObj.data[0].ampduData[1][1][0][1][k];
      //           sceng1.v[mcs_count * 6 + k].y = myObj.data[0].ampduData[1][1][0][2][k];
      //           sceng1.v[mcs_count * 7 + k].y = myObj.data[0].ampduData[1][1][0][3][k];
            }

            sceng0.p.update(sceng0.v);
            sceng0.p.draw(sceng0.v, 0, true);

            sceng1.p.update(sceng1.v);
            sceng1.p.draw(sceng1.v, 1, true);
    }

}

function move(k, val) {
    var elem = document.getElementById("myBar"+k);
    elem.style.width = val + '%';
}

function parsePowerStats(power) {
    powsDoughnutChart.data.datasets[0].data[0] = power.POWSWindow;
    powsDoughnutChart.update();
}

function parseCurrStats(stats) {

    rssiChart.data.datasets[0].data[0] = stats.phy_rssi_ant[0];
    rssiChart.data.datasets[0].data[1] = stats.phy_rssi_ant[1];
    rssiChart.data.datasets[0].data[2] = stats.phy_rssi_ant[2];
    rssiChart.data.datasets[0].data[3] = stats.phy_rssi_ant[3];

    var chQual = "";
    var avgRSSI = (rssiChart.data.datasets[0].data[0] + rssiChart.data.datasets[0].data[1] +
        rssiChart.data.datasets[0].data[2] + rssiChart.data.datasets[0].data[3]) / 4;
    rssiChart.update();

    avgRSSI = avgRSSI.toFixed(1);
    if (avgRSSI < -30) chQual = "Excellent";
    if (avgRSSI < -40) chQual = "Very Good";
    if (avgRSSI < -50) chQual = "Good";
    if (avgRSSI < -70) chQual = "Bad";
    if (avgRSSI < -80) chQual = "Very Bad";

    $("#comboRSSI").text(chQual + "(" + avgRSSI + "dBm)");
    $("#apName").text(stats.SSID + " / " + stats.deviceType);
    $("#phyRate").text(/* stats.phyRate <= BAD */ 800);
    $("#chanBW").text(stats.bandwidth);
    $("#chanBand").text(stats.channel > 13 ? "5G" : "2G");
    $("#chanNum").text(stats.channel);
}


var last_spec;
var last_spec_len = 0;

function parseScan(spec) {
    var k;
    sceneType = 1;

    if (spec.length == 0) {
        console.log("empty scan results list");
        return;
    }

    var str = "<table class='scanTable' border=\"0\"><td><tr height='24' border='0'>";
    str += "<th height='24'> SSID </th>";
    str += "<th wifth ='100'> BSSID </th>";
    str += "<th width = '40'> RSSI </th>";
    str += "<th width = '80' align='right'> Signal </th>";
    str += "<th width = '20'> Noise </th>";
    str += "<th width = '40'> Channel </th>";
    str += "<th width = '100'> PhyRate </th>";
    str += "<th> OpMode </th>";
    str += "</tr>";

    /* update matrix overlap chart */
    var t = d3.transition().duration(200);
    yAxisScale.domain([0, spec.length - 1]).range([0, itemSize * (spec.length - 1)]);
    yAxis.scale(yAxisScale).ticks(spec.length - 1).tickFormat(d => (spec[d].ap/*["stanum"+d]*/.SSID))
    svg_chart.select(".y").transition(t).call(yAxis);

    xAxisScale.domain([0, CHAN_NUM_5G_20M - 1]).range([0, itemSize * (CHAN_NUM_5G_20M - 1)]);
    xAxis.scale(xAxisScale).ticks(CHAN_NUM_5G_20M - 1).tickFormat(d => chan_arr_5g_20[d])
    svg_chart.select(".x").transition(t).call(xAxis);

    var chann_pri_on, chann_sec_on;
    var chann_primary, chann_off_color = 20;
    var chann_pri_on_minus2, chann_pri_on_plus2;
    var chann_pri_on_exact, bw;
    var chann_update = [];
    var rects;

    if ((spec.length < last_spec_len)) {
        for (k = spec.length; k < last_spec_len; k++) {
            for (i = 0; i < CHAN_NUM_5G_20M; i++) {
                channels[k * CHAN_NUM_5G_20M + i].value = 200;
            }
        }
    }

    // channels.length = spec.length*CHAN_NUM_5G_20M;
    for (k = 0; k < spec.length; k++) {
        str += "<tr bgcolor = \"{0}\">".formate(k % 2 ? colorCalibration[1] : colorCalibration[4]);
        str += "<td height='24'><b>" + spec[k].ap.SSID + "</b></td>";

        str += "<td>" + spec[k].ap.BSSID + "</td>";
        str += "<td align='middle'>" + spec[k].ap.RSSI + "</td>";
        str += "<td><div class=\"myProgress\" id=\"myProgress{0}\"><div class=\"myBar\" id=\"myBar{0}\"></div></div></td>".formate(k);
        str += "<td align='middle'>" + spec[k].ap.phyNoise + "</td>";
        str += "<td>" + spec[k].ap.channelNum + "/" + (10 * (1 << spec[k].ap.channelBandwidth - 1)) + "</td>";
        str += "<td>" + spec[k].ap.phyRate + "</td>";
        str += "<td>" + spec[k].ap.operatingMode + "</td>";
        str += "</tr>";

        /* fill up channel heatmap */
        chann_primary = 0xFFFF;
        for (i = 0; i < CHAN_NUM_5G_20M; i++) {
            chann_sec_on = false;
            chann_pri_on = false;
            if (chann_primary == 0xFFFF) {
                chann_pri_on_exact  = (chan_arr_5g_20[i] == spec[k].ap.channelPrimNum);
                if (chann_pri_on_exact) {
                    chann_pri_on = true;
                    chann_primary =  i;
                }
            }
            // console.log("SSID: " + spec[k]["stanum"+k].SSID + "/" + spec[k].ap.channelNum + "vs. " + chan_arr_5g_20[i]);
            if (chann_primary == i) {
                chann_on_color  = 110;  /* colorPrimary */
            }
            bw = (1 << spec[k].ap.channelBandwidth - 2);
            if ((i > chann_primary) && (i < chann_primary + bw)) {
                chann_sec_on = true;
                chann_on_color  = 80;   /* colorSecondary */
            }
            if (chan_arr_5g_20[i] >= 52 && chan_arr_5g_20[i] <= 144) {
                chann_off_color = 40;   /* colorDFS */
            } else {
                chann_off_color = 20;   /* colorBK */
            }
            channels[k * CHAN_NUM_5G_20M + i] = { number: i , ssid: k, value: chann_pri_on||chann_sec_on ? chann_on_color : chann_off_color};
        }
    }

    last_spec_len = spec.length;
    rects = svg_chart.selectAll("rect").data(channels);
    rects.attr("class", "update");

    rects.enter().append('rect')
        .attr('width', cellSize)
        .attr('height', cellSize)
        .attr('x', function (d, i) {
            /* on update left margins don't seem to take effect */
            return margin.left + (itemSize) * d.number;
        })
        .attr('y', function (d, i) {
            /* on update top margins don't seem to take effect */
            return margin.top + (itemSize) * (d.ssid % CHAN_NUM_5G_20M);
        })
        // .attr('fill', '0xFFFFFF');
        //.merge(rects);
        //
    //
     // rects.exit().remove();

    // rects.filter(function (d, i) {
    //     return (d.value >= 0);
    // })

    rects.transition()
        .delay(function(d){
          return 15;
         })
        .duration(2000)
        .attrTween('fill', function (d, i, a) {
            var colorIndex = d3.scale.quantize()
                .range([0, 1, 2, 3, 4])
                .domain([0, 150]);
            return d3.interpolate(a, colorCalibration[colorIndex(d.value)]);
        });
    // merge(rects);

    rects.exit().remove();

    str += "</td></table>";
    var rssi_progress;
    $("#insight").html(str);
    for (k = 0; k < spec.length; k++) {
        rssi_progress = (100 + parseInt(spec[k].ap.RSSI))/70.0;
        move(k, rssi_progress*100);
    }

    last_spec = spec;
    drawSurveyChart(spec);
}

// var xcanvas = document.getElementById("myCanvas");
// var xctx = canvas.getContext("2d");
var $xcanvas = $("#myCanvas");
var xcanvasOffset = $xcanvas.offset();
var xoffsetX = xcanvasOffset.left;
var xoffsetY = xcanvasOffset.top;
var xscrollX = $xcanvas.scrollLeft();
var xscrollY = $xcanvas.scrollTop();
var $xresults = $("#results");
var paths = [];

function hitTest(polygon) {
    // redefine the polygon
    // (necessary to isPointInPath to work
    define(polygon);
    // ask isPointInPath to hit test the mouse position
    // against the current path
    return (ctx.isPointInPath(mouseX, mouseY));
}

function handleMouseMove(e) {
    e.preventDefault();
    mouseX = parseInt(e.clientX - xoffsetX);
    mouseY = parseInt(e.clientY - xoffsetY);

    // check if the mouse is inside the polygon
    var isInside = hitTest(poly);
    if (isInside) {
        $results.text("Mouse is inside the Triangle");
    } else {
        $results.text("Outside");
    }

}

function drawSurveyChart(spec)
{
    var c=document.getElementById("myCanvas");
    var ctx=c.getContext("2d");
    var x_leftmost = 42;

    //$("#myCanvas").mousemove(function (e) {
    //    handleMouseMove(e);
    //});

    Chart.defaults.global.animation.duration = 0;

    /* set background color before anything */
    Chart.pluginService.register({
            beforeDraw: function (chart, easing) {
                var dfs_5g_first = 4, dfs_5g_last = 14;
                if (easing == 1 && chart.chartArea.bottom <= chart_max_size &&
                    chart.config.options.chartArea &&
                    chart.config.options.chartArea.backgroundColor) {
                    var ctx = chart.chart.ctx;
                    var chartArea = chart.chartArea;
                    // ctx.save();
                    ctx.fillStyle = chart.config.options.chartArea.backgroundColor;
                    ctx.fillRect(chartArea.left, chartArea.top, chartArea.right - chartArea.left, chartArea.bottom - chartArea.top);
                    var x_step = ((chartArea.right - chartArea.left)/(1.0*chan_arr_5g_20.length) + 0.5);
                    ctx.fillStyle = colorDFS;
                    ctx.fillRect(chartArea.left + x_step * dfs_5g_first - 2, chartArea.top, chartArea.left + x_step * dfs_5g_last + 5, c.height);
                    // ctx.restore();
                } else {
                    // console.log("prevented redraw with bottom at :" + chart.chartArea.bottom + "easing" + easing);
                }
            }
        });

    var fill_chan_arr_5g_20 = [];

    // extend bar type
    Chart.defaults.derivedBar = Chart.defaults.bar;
    var custom = Chart.controllers.bar.extend({
        draw: function(ease) {

        if (ease != 1 || this.chart.chartArea.bottom > chart_max_size) {
            return;
        }

        // call super method first
        Chart.controllers.bar.prototype.draw.call(this, ease);

        // custom drawing
        var meta = this.getMeta();
        var pt0 = meta.data[0];
        var radius = pt0._view.radius;

        var ctx = this.chart.chart.ctx;
        ctx.save();
        ctx.strokeStyle = 'red';
        ctx.lineWidth = 1;

        var k, rssi, bw;
        var ch_span = 90;
        var ch_spacing = 40;
        var height_fact = 0.82;
        var x_stepsize = ((c.width)/(1.0*chan_arr_5g_20.length)) - 2;
        var y_axis = c.height - 27, y_top, y_text;
        var height_text = 12;
        var x_bottomspan = 100;
        var j, stroke_style, fill_style, text_style;
        var chan_fact;

        ctx.lineWidth = 3;
            ctx.font = "bold 11pt Helvetica";

        fill_chan_arr_5g_20.length = chan_arr_5g_20.length;
        for (j = 0; j < chan_arr_5g_20.length; j++) {
            fill_chan_arr_5g_20[j] = 0;
        }

        for (k = 0; k < spec.length; k++) {
           for (j = 0; j <  chan_arr_5g_20.length; j++) {
               if (spec[k].ap.channelPrimNum == chan_arr_5g_20[j] /* || spec[k].ap.channelNum == chan_arr_5g_20[j] + 2 */) {

                   rssi = parseInt(spec[k].ap.RSSI);
                   path = ctx.beginPath();
                   stroke_style = "rgba(0," + Math.floor(255 - 42.5 * k) + "," + Math.floor(255 - 12.5 * j) + ", 0.7)";
                   ctx.strokeStyle = stroke_style;
                   y_top = ((-rssi) - 30) * y_axis / 70.0;
                   bw = (1 << spec[k].ap.channelBandwidth - 2);

                   ctx.moveTo(x_leftmost + (0 + j) * x_stepsize, y_axis);  /* start at 0 */
                   ctx.bezierCurveTo(
                       x_leftmost + (0 + j) * x_stepsize, (y_axis - y_top) * (1 / 2) + y_top,
                       x_leftmost + ((bw / 4) + j) * x_stepsize, y_top,
                       x_leftmost + ((bw / 2) + j) * x_stepsize, y_top);

                   ctx.bezierCurveTo(
                       x_leftmost + ((bw * 3 / 4) + j) * x_stepsize, y_top,
                       x_leftmost + ((bw + j) * x_stepsize), (y_axis - y_top) * (1 / 2) + y_top,
                       x_leftmost + ((bw + j) * x_stepsize), y_axis);
                   ctx.lineTo(x_leftmost + (0 + j) * x_stepsize, y_axis);  /* end at 0 */

                   ctx.stroke();
                   ctx.closePath();

                   fill_style = "rgba(0," + Math.floor(255 - 42.5 * k) + "," + Math.floor(255 - 12.5 * j) + ", 0.2)";
                   ctx.fillStyle = fill_style;
                   ctx.fill();

                   text_style = "rgba(0," + Math.floor(255 - 42.5 * k) + "," + Math.floor(255 - 12.5 * j) + ", 1.0)";
                   ctx.fillStyle = text_style; /* restore for text */

                   fill_chan_arr_5g_20[j + Math.floor((bw - 1) / 2)]++;
                   ctx.textAlign = "center";
                   ctx.fillText(spec[k].ap.SSID, x_leftmost + (bw / 2 + j) * x_stepsize, y_top - 12 * (fill_chan_arr_5g_20[j + Math.floor((bw - 1) / 2)]));
                   break;
                }
            }
            if (j == chan_arr_5g_20.length) {
                console.log(spec[k].ap.SSID + " not on a pri ctrl channel: " + spec[k].ap.channelPrimNum + "/chan" + spec[k].ap.channelNum);
            }
         }

            ctx.restore();}
        });

    // Stores the controller so that the chart initialization routine can look it up with
    // Chart.controllers[type]
    Chart.controllers.derivedBar = custom;

    var myChart = new Chart(ctx, {
     type: 'derivedBar',
     data: {
        xLabels: chan_arr_5g_20,
        // yLabels: chan_arr_5g_20_rssi,
        datasets: [{
        label: 'Site Survey',
            data: [0],
            backgroundColor: [
                colorSurveyDatasetBK
            ],
            borderColor: [
                colorSurveyBorder
            ],
            borderWidth: 1
        }]
    },
    options: {
        scales: {
            yAxes: [{
                ticks: {
                    beginAtZero:false,
                    steps : 14,
                    stepValue : 5,
                    max: -30,
                    min: -100
                }
            }]
        },
        chartArea: {
            backgroundColor: colorSurveyChartBK
        }
    }
    });

    /* hide fake data point */
    var meta = myChart.getDatasetMeta(0);
    meta.hidden=false;
}

function func_countwrap(timer)
{
    if (timer.count < timer.times) {

        console.log("executing token=" + timer.token +
            " timer.count=" + timer.count + " timer.times=" + timer.times);

        var link = "http://" + location.host + ":8888/wifi/" + timer.token;
        var xmlhttp = new XMLHttpRequest();

        // TODO: build a token<->function map
        xmlhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                myObj = JSON.parse(this.responseText);
                if (myObj.data[0] !== undefined) {
                    if ("ampduData" in myObj.data[0]) {
                        timer.handle = timer.func(myObj.data[0].ampduData);
                    } else if ("spectrumData" in myObj.data[0]) {
                        timer.handle = timer.func(myObj.data[0].spectrumData);
                    } else if ("chanimStats" in myObj.data[0]) {
                        timer.handle = timer.func(myObj.data[0].chanimStats);
                    } else if ("currentStats" in myObj.data[0]) {
                        timer.handle = timer.func(myObj.data[0].currentStats);
                    } else if ("powerStats" in myObj.data[0]) {
                        timer.handle = timer.func(myObj.data[0].powerStats);
                    }
                }
            }
        };

        xmlhttp.open("GET", link, true);
        xmlhttp.send();

        window.setTimeout(function() { func_countwrap(timer); }, timer.interval);
        timer.count++;
    }
}

function func_timewrap(token, interval, times, func)
{
    var timer = {
        token: token, func: func,
        times: times, interval: interval,
        count: 0
    };

    timers.push(timer);
    if (times > 0) {
        window.setTimeout( function() { func_countwrap(timer); }, interval );
    } else {
        console.log("times value set to zero! (not executing)");
    }
}

/* delay    - how long to wait until we call the function
   interval - how often
   times    - how many times
   function - func to be called
   param    - parameter to the func to be called
 */
function request_out_start(token, delay, interval, times, func)
{
    var reqTimer = window.setTimeout(function() { func_timewrap(token, interval, times, func); }, delay);
    return (reqTimer);
}

function request_out_stop(reqTimer)
{
    window.clearInterval(reqTimer);
}

function noReturnProc(retVal)
{
    // TODO: handle errs
}

function request_out(token, delay, interval, times)
{
    // TODO: build a token<->function map
    if ("ampduData" == token) {
        reqampdu_timer = request_out_start(token, delay, interval, times, parseAmpdu);
    } else if ("spectrumData" == token) {
        if (reqspec_timer == undefined) {
            reqspec_timer = request_out_start(token, delay, interval, times, parseScan);
        }
    } else if ("chanimStats" == token) {
        reqchan_timer = request_out_start(token, delay, interval, times, parseChanim);
    } else if ("currentStats" == token) {
        reqstats_timer = request_out_start(token, delay, interval, times, parseCurrStats);
    } else if ("powerStats" == token) {
        reqpower_timer = request_out_start(token, delay, interval, times, parsePowerStats);
    } else if (token.includes("triggerScan")) {
        if (reqtrig_timer == undefined) {
            reqtrig_timer = request_out_start(token, delay, interval, times, noReturnProc);
        }
    }
}
