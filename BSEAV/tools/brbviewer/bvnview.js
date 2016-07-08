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
/*
  This is a copy of the file: /projects/stbgit/scripts/bin/.plib/js/bcm/bcnview.js
*/
//
//
//
//
//  +------+                 +------+                 +------+
//  |      |                 |      |                 |      |
//  |      |                 |      |                 |      |
//  |      |                 |      |                 |      |
//  | box0 |                 |      |                 | box2 |
//  |      |                 |      |                 |      |
//  |      |                 |      |                 |      |
//  |      |                 |      |                 |      |
//  |      |                 |      |                 |      |
//  +------+                 |      |                 +------+
//                           |      |
//                           |      |
//                z0         | box1 |        z1
//             vertical      |      |     vertical
//             channels      |      |     channels
//                           |      |
//                           |      |
//                           |      |
//                           |      |
//                           |      |                 +------+
//                           |      |                 |      |
//                           |      |                 |      |
//                           |      |                 |      |
//                           |      |                 | box3 |
//                           |      |                 |      |
//                           |      |                 |      |
//                           |      |                 |      |
//                           |      |                 |      |
//                           +------+                 +------+
//
//
var invalid_src=0;
var invalid_dest=0;
var invalid_get_source=0;
var body_y_offset = 1;
var body_x_offset = 1;
var num_prereserved_channels = 5; /* on each size of each zone */
var z0_num_channels = 30; /* vertical channels, between boxes 0,1 */
var z1_num_channels = 33; /* vertical channels, between boxes 1,2/3 */
var z0ch = []; /* A 1 indicates that the zone's channel is reserved */
var z1ch = []; /* A 1 indicates that the zone's channel is reserved */
var cw = channel_width;

function int32_to_hex_string(x) {
    x = x >>> 0; // force to unsigned int
    return x.toString(16);
}


function int32_to_hex_string_full(x) {
    x = x >>> 0; // force to unsigned int
    x = x.toString(16);
    return "00000000".substring(0, 8 - x.length) + x;
}


function set_context_defaults()
{
    ctx.fillStyle = "black";
    ctx.font = "Bold " + (cw-6) + "pt Courier";
    ctx.textAlign = "center";
    ctx.textBaseline = "top";
    ctx.rowHeight = cw;
}

// Take a box and assign its dimensions based
// on the contents of its components.
function ascertain_dimensions(box)
{
    ctx.save();
    set_context_defaults(ctx);
    box.x = box.y = box.w = box.h = box.rh = 0;
    var a = box.a;

    var w = 0;
    // Find the string with the maximum width
    for (i=0; i<a.length; i++) {
	var t = ctx.measureText(a[i]).width;
	if (w < t) {
	    w = t;
	}
    }
    w += Math.floor(cw/2); // add a minimal amount of whitespace
    if (w % cw) {
	// Make the width a multiple of the channel width
	w += cw - (w % cw);
    }
    ctx.restore();
    box.rh = 1; // rh is "row height"
    box.h = a.length * box.rh;
    box.w = Math.floor(w/cw);
}


/* We assume
 *   o first line segment is horizontal
 *   o last line segment is horizontal
 *   o alternating horizontal and vertical segments
 */
function draw_segment_raw(path, color)
{
    var radius = cw;
    var i;

    var npoints = path.length;
    var next = path[1];
    var cur = path[0];
    var prev;
    ctx.beginPath();
    ctx.globalAlpha = 0.5;
    ctx.moveTo(path[0].x, path[0].y);
    ctx.lineWidth = Math.floor(cw/3) + 1;
    ctx.strokeStyle = color;

    for (i=1; i<npoints; i++) {
	prev = cur;
	cur = next;
	next = path[i+1];
	var is_last = (i === npoints-1);
	var is_horizontal = i & 1;

	if (is_last) {
	    if (cur.x > prev.x) {
		ctx.lineTo(cur.x-2, cur.y);
		ctx.stroke();
		ctx.beginPath();
		ctx.lineWidth = 2;
		ctx.lineTo(cur.x, cur.y);
		/* Now we draw an arrow head. */
		ctx.lineTo(cur.x - 5, cur.y - 2);
		ctx.arcTo(cur.x -1, cur.y, cur.x - 5, cur.y + 2, 8);
		ctx.lineTo(cur.x, cur.y);
	    } else {
		ctx.lineTo(cur.x+2, cur.y);
		ctx.stroke();
		ctx.beginPath();
		ctx.lineWidth = 2;
		ctx.lineTo(cur.x, cur.y);
		/* Now we draw an arrow head. */
		ctx.lineTo(cur.x + 5, cur.y - 2);
		ctx.arcTo(cur.x +1, cur.y, cur.x + 5, cur.y + 2, 8);
		ctx.lineTo(cur.x, cur.y);
	    }
	    ctx.stroke();
	    ctx.fill();

	} else if (is_horizontal) {
	    ctx.arcTo(next.x, cur.y, next.x, next.y, radius);
	} else {
	    ctx.arcTo(cur.x, next.y, next.x, next.y, radius);
	}
    }
}


// FUNdraw_segment() -- draw an entire path on the canvas.
//
//   s -- the source component (a string).
//   d -- the destination component (a string).
//   color -- the color of the line.
function draw_segment(s, d, color)
{
    if (!s) { // CAD failsafe
        //alert("draw_segment: source is invalid");
        invalid_src++;
        return;
    }
    if (!d) { // CAD failsafe
        //alert("draw_segment: destination is invalid");
        invalid_dest++;
        return;
    }
    var sobj = h_info[s];
    var dobj = h_info[d];

    var sb = sobj.box;
    var db = dobj.box;

    if (sb === 0 && db === 1) {
	// Segment is going from box0 to box1.
	var sp = get_coords(s, 0);
	var dp = get_coords(d, 1);
	if (sp.y !== dp.y) {
	    var c = get_channel(0);
	    var x = sp.x + cw * c;
	    draw_segment_raw([{x:sp.x,y:sp.y},
			   {x:x,y:sp.y},
			   {x:x,y:dp.y},
			   {x:dp.x, y:dp.y}],
			  color);

	} else {
	    draw_segment_raw([{x:sp.x,y:sp.y},
			   {x:dp.x, y:dp.y}],
			  color);
	}

    } else if (sb === 1 && (db === 2 || db === 3)) {
	// Segment is going from box1 to box2 or box3.
	var sp = get_coords(s, 0);
	var dp = get_coords(d, 1);
	if (sp.y !== dp.y) {
	    var c = get_channel(1);
	    var x0 = sp.x + cw * c;
	    draw_segment_raw([{x:sp.x,y:sp.y},
			   {x:x0,y:sp.y},
			   {x:x0,y:dp.y},
			   {x:dp.x, y:dp.y}],
			  color);

	} else {
	    draw_segment_raw([{x:sp.x,y:sp.y},
			   {x:dp.x, y:dp.y}],
			  color);
	}

    } else if (sb === 3 && db === 1) {
	// Segment is going frmo box3 (loopbacks) to box1.
	var sp = get_coords(s, 0);
	var dp = get_coords(d, 1);
	var p = [];
	p.push({x:sp.x,y:sp.y});
	var n = boxes[3].a.length;
	var c = 1 + (n - (1 + sobj.box_id));
	var x0 = sp.x + cw * c;
	p.push({x:x0,y:sp.y});
	var y0 = sp.y +  2 * cw * c;
	p.push({x:x0,y:y0});
	c = get_channel(0,1);
	var x1 = dp.x - cw*(z0_num_channels-(c+1))
	p.push({x:x1,y:y0});
	p.push({x:x1,y:dp.y});
	p.push({x:dp.x,y:dp.y});
	draw_segment_raw(p, color);
    } else {
	// something is wrong as no other possibilites
	// make sense.
    }
}




// Given a box of components, draw it.  The only
// input param to this function is a box, which is an
// object with the following fields:
//   a -- array of strings (components)
//   x -- the x position, in channel widths, of the top left
//        corner of the box.
//   y -- the y position, in channel widths, of the top left
//        corner of the box.
//   h -- the height of the box, in channel widths.
//   w -- the width of the box, in channel widths.
function draw_box(box)
{
    var num_rows = box.a.length;
    var i;

    set_context_defaults();
    ctx.moveTo(box.x*cw,( box.y + box.h)*cw);
    ctx.lineWidth = 1;
    ctx.lineTo(box.x*cw, box.y*cw);
    ctx.lineTo((box.x + box.w)*cw, box.y*cw);
    ctx.lineTo((box.x + box.w)*cw, (box.y + box.h)*cw);
    ctx.stroke();


    for (i=0; i<num_rows; i++) {
	ctx.moveTo(box.x*cw, (box.rh*(i+1)+box.y)*cw);
	ctx.lineTo((box.x+box.w)*cw, (box.rh*(i+1)+box.y)*cw);
	ctx.stroke();
        ctx.fillText(box.a[i], box.x*cw + Math.floor(box.w*cw/2),
		     (box.rh*i+box.y)*cw);
    }
}



bvnview.start_loading = function() {
    var e = document.getElementById('bvnview_loading');
    e.style.display="block";
}


bvnview.stop_loading = function() {
    var e = document.getElementById('bvnview_loading');
    e.style.display="none";
}

bvnview.soap_failure = function() {
    alert("Communications to the device failed.\n\nMake sure a Nexus app is running and, the BVN clocks are clocking.\n\n\n");
    bvnview.stop_loading();
}


// Callback func, Called after SOAP request completes.
bvnview.refresh = function(err, context, vals) {
    var i, j;

    if (err != 0) {
	bvnview.soap_failure();
	return;
    }

    bvnview.vals = vals;
    for (i=0; i<bvnview.addrs.length; i++) {
	var x = '0x' + int32_to_hex_string_full(bvnview.addrs[i]);
	var y = vals[i];
	// Here we use bvnview as an associate array.  Each address in
	// bvnview.addrs[] is now a property whose value is the GISB
	// registers or memory value.
	bvnview[x] = y;
    }
    bvnview.stop_loading();
    bvn_display_continued();
}


bvnview.loadreg = function() {
    bvnview.start_loading();
    bcm_soap.read_multiple(null, bvnview.ip, bvnview.addrs, bvnview.refresh);
}

// Given a component, ascertain the (x,y) point in pixels for
// the line that connects to it.  If the line is coming from
// the left side, into is 1, otherwise, 0.
function get_coords(c, into)
{
    var o = h_info[c];
    var b = o.box;
    var rh = boxes[b].rh;
    var x = (boxes[b].x + (into ? 0 : boxes[b].w)) * cw;
    var y = (boxes[b].y + rh * o.box_id)*cw + Math.floor(rh*cw/2);
    return {x:x,y:y};
}


// Get the source of a component.
function get_source(x)
{
    var src = "none";
    if (!x) { // CAD failsafe
        invalid_get_source++;
        return "none";
    }
    var o = h_info[x];

    if (!o.addr)
	return "none";

    var val = bvnview[o.addr];

    switch (o.box) {
    case 1:
	src = get_box1_src(val);
	break;
    case 2:
    case 3:
	src = get_box23_src(val);
	break;
    default:
	break;
    }

    if (src === "none")
	return "none";

    // Make sure the source is enabled.
    o = h_info[src];
    if (o && o.enable_addr
	&& !is_src_enabled(bvnview[o.enable_addr]))
	return "none";

    return src;
}

// Find an empty channel.
//   zone -- either 0 or 1 (there are only two zones).
//   reverse --  by default we pick the first open
//     channel on the left.  If this argument is
//     non-zero, we pick the first open channel from
//     the right.
// Note: if usage has us running out of channels, we
//     should either (a) declare more channels at the
//     top of this file or (b) change this function
//     and the code that calls it to allow for allocation
//     with "half-channel" granularity, since the actual
//     line drawn is roughly 1/2 the channel width.
function get_channel(zone, reverse)
{
    var rval = -1;

    if (zone === 0) {
	if (reverse) {
	    for (i=z0_num_channels-1; i>=0; i--) {
		if (!z0ch[i]) {
		    z0ch[i] = 1;
		    rval = i;
		    break;
		}
	    }
	} else {
	    for (i=0; i<z0_num_channels; i++) {
		if (!z0ch[i]) {
		    z0ch[i] = 1;
		    rval = i;
		    break;
		}
	    }
	}
    } else if (zone === 1) {
	for (i=0; i<z1_num_channels; i++) {
	    if (!z1ch[i]) {
		z1ch[i] = 1;
		rval = i;
		break;
	    }
	}
    }
    if (rval === -1) {
	alert("Ran out of zone " + zone + " routing channels!  Send an email to JimQ");
	throw "out of channels";
    }

    return rval;
}


function bvn_display() {
    if (use_example_data) {
	// Fake the register data load for our test DEBUG
	bvnview.start_loading();
	var t0 = vnet_b_arr.concat(vnet_f_arr);
	bvnview.refresh(0, null, t0);
    } else {
	bvnview.loadreg();
    }
}


function bvn_display_continued() {
    var i;

    for (i=0; i<z0_num_channels; i++) {
	z0ch[i] = 0;
    }
    for (i=0; i<z1_num_channels; i++) {
	z1ch[i] = 0;
    }

    can = document.getElementById('can');
    ctx = can.getContext('2d');

    // This may be the user refreshing the drawing so we want
    // to clear everything that was already drawn before.
    can.width = can.width;

    // We compute the dimensions based on the number
    // of components (rows) and the string length
    // of the component.
    for (i=0; i<boxes.length; i++) {
	/* Determine box dimensions */
	ascertain_dimensions(boxes[i]);
    }

    var net_height = Math.max(boxes[0].h, boxes[1].h,
			      boxes[2].h + boxes[3].h + 1);

    // boxes[0] is in the top left corner.
    boxes[0].x = body_x_offset;
    boxes[0].y = body_y_offset;

    var t0 = net_height - boxes[3].h;
    var t1 = Math.max(boxes[2].w, boxes[3].w);
    var t2 = boxes[0].x + boxes[0].w + z0_num_channels;
    var t3 = t2 + boxes[1].w + z1_num_channels;


    // boxes[1] is in the top, and somewhat in the middle.  Our
    // calculations assume that boxes[1] height is greater than
    // boxes[2] and boxes[3] heights combined.  We may have
    // to modify our calculations if future chips do not follow
    // this assumption.
    boxes[1].x = t2;
    boxes[1].y = body_y_offset;

    // boxes[2] is on the top right.  It has the same width as
    // boxes[3].
    boxes[2].x = t3;
    boxes[2].y = body_y_offset;
    boxes[2].w = t1;

    // boxes[3] is below boxes[2].  Also, the bottom of boxes[3]
    // aligns with the bottom of boxes[1].
    boxes[3].x = t3;
    boxes[3].y = body_y_offset + t0;
    boxes[3].w = t1;

    // Compute the dimensions needed for the canvas.
    can.width = cw * (boxes[2].x + boxes[2].w + boxes[3].a.length);
    can.height = cw * (boxes[1].y + boxes[1].h + boxes[3].a.length);

    // Draw the four boxes that hold the components.
    for (i=0; i<boxes.length; i++) {
	draw_box(boxes[i]);
    }

    // 'segments' is used as an assoc array to remember what
    // line segments we have already drawn.
    var segments = new Object();
    var dests = [].concat(boxes[2].a, boxes[3].a, boxes[1].a);
    var all = dests.concat(boxes[0].a);
    var paths = [];

    // Since the absence of a source has the source known
    // as "none", we pretend that none is connected to
    // every single component.
    for (i=0; i<all.length; i++) {
	segments["none." + all[i]] = 1;
    }


    // Go through all the destination nodes and follow
    // where there sources go until we can go no further.
    // Each of these paths will become a path on the
    // graph with the same color.
    for (i=0; i<dests.length; i++) {
	var x = dests[i];
	var path = [];

	while (true) {
	    var src = get_source(x);
	    var segment = src + "." + x;

	    // Stop building the path if this segment
	    // has already been drawn.
	    if (segments[segment]) {
		break;
	    }
	    // This first node is a special case.
	    if (path.length === 0) {
		path.push(x);
	    }
	    path.unshift(src);
	    segments[segment] = 1;
	    x = src;
	}
	if (path.length === 0) {
	    continue;
	}
	paths.push(path);
    }

    // Prereserve the channels that are near the
    // boxes (this is for asthetics only).
    for (i=0; i<num_prereserved_channels; i++) {
	z0ch[i] = 1;
	z0ch[z0_num_channels - (1 + i)] = 1;
	z1ch[i] = 1;
	z1ch[z1_num_channels - (1 + i)] = 1;
    }

    // Our varied assortment of colors.  Intentionally absent
    // is white and grey.
    var colors = [
		  '#00FFFF',
		  '#000000',
		  '#0000FF',
		  '#FF00FF',
		  '#008000',
		  '#00FF00',
		  '#800000',
		  '#000080',
		  '#808000',
		  '#800080',
		  '#FF0000',
		  '#808080',
		  '#008080',
		  '#FFFF00',
		  '#fa4',
		  '#4fa',
		  '#a4f',
		  '#af4',
		  ];


    // Go through the list of paths and draw them one
    // by one.  Use a different color for each path.
    for (i=0; i<paths.length; i++) {
	var path = paths[i];
	var color = colors[i%colors.length];
	var arrow = " \u2192 ";

	for (j=0; j<path.length-1; j++) {
	    draw_segment(path[j], path[j+1], color);
	}
    }

    // Now create two grey boxes which represent the two
    // networks, vnet_f and vnet_b.
    var t4 = num_prereserved_channels-1;
    var x0 = cw * (boxes[0].x + boxes[0].w + t4);
    var x1 = cw * (boxes[1].x - t4);
    var y0 = cw * body_y_offset;
    var y1 = cw * (body_y_offset + net_height);
    var x2 = cw * (boxes[1].x + boxes[1].w + t4);
    var x3 = cw * (boxes[2].x - t4);
    ctx.globalCompositeOperation = "destination-over";
    ctx.fillStyle = '#dcdcdc';
    ctx.fillRect(x0, y0, x1-x0, y1-y0);
    ctx.fillRect(x2, y0, x3-x2, y1-y0);
}
