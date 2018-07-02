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

(function () {
    //UI configuration
    var width = 1200,
        height = 800;

    //data vars for rendering
    var dateExtent = null,
        data = null,
        dayOffset = 0,
        dailyValueExtent = {};

    var ssid     = [];

    for (i = 0; i < SSID_NUM; i++) {
        ssid[i] = "ssid" + i;
    }

    for (j = 0; j < ssid.length; j++) {
        for (i = 0; i < CHAN_NUM_5G_20M; i++) {
            if (chan_arr_5g_20[i] >= 52 && chan_arr_5g_20[i] <= 144){
                channels.push({ number: i, ssid: j, value: 40 });
            } else {
                channels.push({ number: i, ssid: j, value: 20 });
            }
        }
    }

    //axises and scales
    var axisWidth = 0,
        axisHeight = itemSize * SSID_NUM;
        xAxisScale = d3.scale.linear()
            .range([0, (itemSize)* (CHAN_NUM_5G_20M)])
            .domain([d3.min(chan_arr_5g_20), d3.max(chan_arr_5g_20)]);
        xAxis = d3.svg.axis()
            .orient('top')
            .ticks(CHAN_NUM_5G_20M)
            .tickFormat(d3.format('02s'))
            .scale(xAxisScale);
        yAxisScale = d3.scale.linear()
            .range([0, axisHeight])
            .domain([0, SSID_NUM]),
        yAxis = d3.svg.axis()
            .orient('left')
            .ticks(SSID_NUM)
            .tickFormat(d3.format('15s'))
            .scale(yAxisScale);

    initCalibration();

    var svg = d3.select('[role="chanmap"]');
    svg_chart = svg;

        chanmap = svg
            .attr('width', width)
            .attr('height', height)
            .append('g')
            .attr('width', width - margin.left - margin.right)
            .attr('height', height - margin.top - margin.bottom)
            .attr('transform', 'translate(' + margin.left + ',' + margin.top + ')');
        var rect = null;

        var dummyJSON = [];
        d3.json(dummyJSON, function (err, data) {
            dateExtent = d3.extent(dummyJSON, function (d) {
                return d3.extent(chan_arr_5g_20);
        });

        axisWidth = itemSize * (chan_arr_5g_20.length);
        console.log("ext[0]=" + dateExtent[0] + " ext[1]=" + dateExtent[1] + "width:" + axisWidth);
        //render axises
        svg.append('g')
            .attr('transform', 'translate(' + (margin.left+itemSize/2) + ',' + margin.top + ')')
            .attr('class', 'x axis')
            .call(xAxis)
            .append('text')
            .text('Channel')
            .attr('transform', 'translate(' + (axisWidth-60) + ', -30)');

        svg.append('g')
            .attr('transform', 'translate(' + margin.left + ',' + (margin.top+itemSize/2) + ')')
            .attr('class', 'y axis')
            .call(yAxis)
            .append('text')
            .text('SSID')
            .attr('transform', 'translate(-30,' + (axisHeight+40) + ') rotate(-90)');

        rect = chanmap.selectAll('rect')
            .data(channels)
            .enter().append('rect')
            .attr('width', cellSize)
            .attr('height', cellSize)
            .attr('x', function (d, i) {
                return itemSize * d.number;
            })
            .attr('y', function (d, i) {
                return itemSize * (d.ssid % CHAN_NUM_5G_20M);
            })
            .attr('fill', '#ffffff');

    renderColor();
  });

  function initCalibration(){
    d3.select('[role="calibration"] [role="example"]').select('svg')
      .selectAll('rect').data(colorCalibration).enter()
    .append('rect')
      .attr('width',cellSize)
      .attr('height',cellSize)
      .attr('x',function(d,i){
        return i*itemSize;
      })
      .attr('fill',function(d){
        return d;
      });

    //bind click event
    d3.selectAll('[role="calibration"] [name="displayType"]').on('click',function() {
      renderColor();
    });
  }

  function renderColor(){
    var renderByCount = true;
    rect
      .filter(function(d,i){
        return (d.value > 0);
      })
      .transition()
      .delay(function(d){
        return 15;
      })
      .duration(1000)
      .attrTween('fill',function(d,i,a){
        //choose color dynamicly
        var colorIndex = d3.scale.quantize()
          .range([0, 1, 2, 3, 4])
          .domain((renderByCount?[0,150]:  dailyValueExtent[d.day]));

        return d3.interpolate(a,colorCalibration[colorIndex(d.value)]);
      });
    }
})();