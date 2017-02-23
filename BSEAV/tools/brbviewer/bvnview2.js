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
  This file was created to include brbviewer-specific changes to the original
  file bvnview.js. I wanted to modify bvnview.js as little as possible. That
  way, any changes made to bvnview.js in the future can be easily incorporated
  into the brbviewer tool.
*/
var MasterDebug=0;
var set_names_done = 0;
var gEvent = 0;
var CgiCount=0; // number of times the cgi was called
var SetVariableCount=0; // number of times setVariable() is called
var gFieldName = "";
var HIDE = false;
var SHOW = true;

function MyLoad(event)
{
    //alert("MyLoad");
    hideOrShow("row_bvnview", SHOW );
    bvn_display();
}

function MyClick(event)
{
    var debug=0;
    var target = event.target || event.srcElement;
    var id = target.id;
    if (debug==1) alert("MyClick: id (" + id + ");" );

    gEvent = event;

    setVariable(id);
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
        } else if ( obj.type == "text" ) {
            fieldValue = obj.value;
        } else if ( obj.type == "radio" ) {
        } else if ( obj.type == "button" ) {
            //alert("button:" + fieldName);
        } else if ( obj.type == "select-one" ) {
        }

        if (fieldName == "h1brbviewer") {
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

function hideOrShow ( elementid, trueOrFalse )
{
    var debug=0;
    if (trueOrFalse) {
        if(debug==1) alert("hideOrShow: " + elementid + "; starting to SHOW; len innerHTML:" + document.getElementById(elementid).innerHTML.length );
        if(debug==1) alert( document.getElementById(elementid).innerHTML );
        document.getElementById(elementid).style.display = '';
        if(debug==1) alert("hideOrShow: " + elementid + "; SHOW");
    } else {
        if(debug==1) alert("hideOrShow: " + elementid + "; starting to HIDE");
        document.getElementById(elementid).style.display = 'none';
        if(debug==1) alert("hideOrShow: " + elementid + "; HIDE");
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

function UpdateField( fieldText, fieldName )
{
    if ( fieldText.length ) {
        var objplatform = document.getElementById( fieldName );
        if (objplatform) {
            objplatform.innerHTML = fieldText;
        }
    }
}

function update_javascript_elements ( json )
{
    var temp_str;

    if ( json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].box1_srcs[0].Text.length ) {

        temp_str = JSON.stringify ( box1_srcs, null, 4 );
        //alert( "stringify before:" + temp_str );

        //alert( "response:" + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].box1_srcs[0].Text );
        box1_srcs = JSON.parse ( "[" + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].box1_srcs[0].Text + "]" );
        temp_str = JSON.stringify ( box1_srcs, null, 4 );
        //alert( "stringify after:" + temp_str );

        var idx = 0;
        var idx2 = 0;
        var prev = "";
        //alert("box1_srcs.length:" + box1_srcs.length );
        for (idx=0; idx<box1_srcs.length; idx++) {
            //alert("idx:" + idx + "(" + box1_srcs[idx] + "); prev (" + prev + ")" );
            if ( idx == 15 && box1_srcs[idx] != "none" ) {
                prev = box1_srcs[idx];
                box1_srcs[idx] = "none";
                //alert("setting 15 to none");
            } else {
                if (prev != "") {
                    temp_str = box1_srcs[idx];
                    box1_srcs[idx] = prev;
                    prev = temp_str;
                }
            }
        }
        if (prev != "") {
            box1_srcs[idx] = prev;
            prev = "";
        }
        temp_str = JSON.stringify ( box1_srcs, null, 4 );
        //alert( "stringify after 15:" + temp_str );
    }
    if ( json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].box23_srcs[0].Text.length ) {
        temp_str = JSON.stringify ( box23_srcs, null, 4 );
        //alert( "stringify before:" + temp_str );

        //alert( "response:" + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].box23_srcs[0].Text );
        box23_srcs = JSON.parse ( "[" + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].box23_srcs[0].Text + "]" );
        //alert( "JSON.parse done");
        temp_str = JSON.stringify ( box23_srcs, null, 4 );
        //alert( "stringify after:" + temp_str );

        var idx = 0;
        var idx2 = 0;
        var prev = "";
        //alert("box23_srcs.length:" + box23_srcs.length );
        for (idx=0; idx<box23_srcs.length; idx++) {
            //alert("idx:" + idx + "(" + box23_srcs[idx] + "); prev (" + prev + ")" );
            if ( idx == 15 && box23_srcs[idx] != "none" ) {
                prev = box23_srcs[idx];
                box23_srcs[idx] = "none";
                //alert("setting 15 to none");
            } else {
                if (prev != "") {
                    temp_str = box23_srcs[idx];
                    box23_srcs[idx] = prev;
                    prev = temp_str;
                }
            }
        }
        if (prev != "") {
            box23_srcs[idx] = prev;
            prev = "";
        }
        temp_str = JSON.stringify ( box23_srcs, null, 4 );
        //alert( "stringify after 15:" + temp_str );
    }
    if ( json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].h_info[0].Text.length ) {
        //alert( "h_info response:" + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].h_info[0].Text );
        h_info = JSON.parse ( '{' + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].h_info[0].Text + '}' );
        //alert( "h_info:" + JSON.stringify ( h_info, null, 4 ) );
    }
    if ( json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].javascript_boxes_0[0].Text.length ) {
        //alert( "response:" + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].javascript_boxes_0[0].Text );
        var javascript_boxes_0 = JSON.parse ( '[' + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].javascript_boxes_0[0].Text + ']' );
        //alert( "boxes_0:" + JSON.stringify ( javascript_boxes_0, null, 4 ) );
        boxes[0].a = javascript_boxes_0;
        //alert( "boxes[0].a:" + boxes[0].a );
    }
    if ( json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].javascript_boxes_1[0].Text.length ) {
        //alert( "response:" + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].javascript_boxes_1[0].Text );
        var javascript_boxes_1 = JSON.parse ( '[' + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].javascript_boxes_1[0].Text + ']' );
        //alert( "boxes_1:" + JSON.stringify ( javascript_boxes_1, null, 4 ) );
        boxes[1].a = javascript_boxes_1;
        //alert( "boxes[1].a:" + boxes[1].a );
    }
    if ( json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].javascript_boxes_2[0].Text.length ) {
        //alert( "response:" + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].javascript_boxes_2[0].Text );
        var javascript_boxes_2 = JSON.parse ( '[' + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].javascript_boxes_2[0].Text + ']' );
        //alert( "boxes_2:" + JSON.stringify ( javascript_boxes_2, null, 4 ) );
        boxes[2].a = javascript_boxes_2;
        //alert( "boxes[2].a:" + boxes[2].a );
    }
    if ( json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].javascript_boxes_3[0].Text.length ) {
        //alert( "response:" + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].javascript_boxes_3[0].Text );
        var javascript_boxes_3 = JSON.parse ( '[' + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].javascript_boxes_3[0].Text + ']' );
        //alert( "boxes_3:" + JSON.stringify ( javascript_boxes_3, null, 4 ) );
        boxes[3].a = javascript_boxes_3;
        //alert( "boxes[3].a:" + boxes[3].a );
    }
    if ( json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].bvnview_addrs[0].Text.length ) {
        //alert( "response:" + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].bvnview_addrs[0].Text );
        var bvnview_addrs = JSON.parse ( '[' + json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].bvnview_addrs[0].Text + ']' );
        //temp_str = JSON.stringify ( bvnview_addrs, null, 4 );
        //alert( "orig bvnview.addrs:" + JSON.stringify ( bvnview.addrs, null, 4 ) );

        bvnview.addrs = bvnview_addrs;
        //alert( "new  bvnview.addrs:" + JSON.stringify ( bvnview.addrs, null, 4 ) );
    }

    UpdateField( json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].PlatformType[0].Text, "platform" );
    UpdateField( json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].PlatformVersion[0].Text, "platver" );
    UpdateField( json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].PlatformVariant[0].Text, "VARIANT" );
    UpdateField( json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].PlatformBoltVer[0].Text, "BOLTVER" );
    UpdateField( json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].PlatformUname[0].Text, "UNAME" );
    UpdateField( json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].StbTime[0].Text, "stbtime" );
}

function bvn_set_names() {
    return true;
    //alert("bvn_set_names - set_names_done:" + set_names_done );
    if ( set_names_done == 0 ) {
		var idx = 0;
		var name = 0;
		for(idx=0; idx<4; idx++ ){
            //if ( idx == 3 ) {
            //    alert("setting boxes[3] with new names");
            //    boxes[idx].a = ["lpb_5", "lpb_4", "lpb_3", "lpb_2", "lpb_1", "lpb_0"];
            //}
			var names = "";
			for(name=0; name<boxes[idx].a.length; name++ ){
				names += boxes[idx].a[name] + " ";
			}
			//alert("boxes[" + idx + "].length = " + boxes[idx].a.length + "; names:" + names );
		}
	} else {
		set_names_done = 1;
	}
}
