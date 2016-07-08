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
  This is a copy of the file: /projects/stbgit/scripts/bin/.plib/js/bcm/soap.js
*/
var bcm_soap = {};



//==============================================================================
//
// (in)  context -- an object that is passed through to the callback func.
// (in)  ip -- the ip address (eg '10.28.7.123')
// (in)  addrs -- array of int32
// (in)  callback -- the callback function when answer is computed.
//          It's three params will be the error indicator, the context param,
//          and an array of int32
//
bcm_soap.read_multiple = function(context, ip, addrs, callback) {
	var post_url = 'http://' + window.location.hostname + '/brbviewer.cgi'; // CAD BRCM different web address
	var uuenc_send_str = base64.encode_from_int32_arr(addrs);
	var xmlhttp = new XMLHttpRequest();

	xmlhttp.open("POST", post_url, true);
	xmlhttp.onreadystatechange=function() {
		if (xmlhttp.readyState == 4) {
            //alert("got response");
			var json = XMLObjectifier.xmlToJSON(xmlhttp.responseXML);
			if (json && ('Body' in json) && ('GetRegisterCollectionWithElmErrsResponse' in json.Body[0])) {
                update_javascript_elements ( json );
				var uuenc_recv_str_errs = json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].RegisterErrs[0].Text;
				var uuenc_recv_str_vals = json.Body[0].GetRegisterCollectionWithElmErrsResponse[0].RegisterValues[0].Text;
				var vals = base64.decode_to_int32_arr(uuenc_recv_str_vals);
				var errs = base64.decode_to_int32_arr(uuenc_recv_str_errs);
                                if (callback != null) callback(0, context, vals, errs);
			} else {
				if (callback != null) callback(1, context, [], []);
			}
		}
	}

	var action = '"urn:schemas-upnp-org:service:RegSvc:1#GetRegisterCollectionWithElmErrs"';
	xmlhttp.setRequestHeader("SOAPAction", action);
	xmlhttp.setRequestHeader("Content-Type", "text/xml");
	var xml = '<?xml version="1.0" encoding="UTF-8"?>' + "\n" +
        '<soap:Envelope ' + "\n" +
        '    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ' + "\n" +
        '    xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/" ' +  "\n" +
        '    xmlns:xsd="http://www.w3.org/2001/XMLSchema" ' + "\n" +
        '    soap:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" ' + "\n" +
        '    xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">' + "\n" +
        '    <soap:Body>' + "\n" +
        '        <GetRegisterCollectionWithElmErrs xmlns="urn:schemas-upnp-org:service:NumberStorage:1">' + "\n" +
        '            <Count xsi:type="xsd:int">' + addrs.length + '</Count>' + "\n" +
        '            <RegisterCollection xsi:type="xsd:base64Binary">' +
                     uuenc_send_str +
		'            </RegisterCollection>' + "\n" +
        '        </GetRegisterCollectionWithElmErrs>' + "\n" +
        '    </soap:Body>' + "\n" +
        '</soap:Envelope>' + "\n";

    xmlhttp.send(xml);
}


//==============================================================================
//
// (in)  context -- an object that is passed through to the callback func.
// (in)  ip -- the ip address (eg '10.28.7.123')
// (in)  addr -- an int32
// (in)  callback -- the callback function when answer is computed.
//          It's three params will be an error indicator, context param,
//          and array of int32.
//
bcm_soap.read = function(context, ip, addr, callback) {
	var addrs = [ addr ];
	return bcm_soap.read_multiple(context, ip, addrs, callback);
}



//==============================================================================
//
// (in)  context -- an object that is passed through to the callback func.
// (in)  ip -- the ip address (eg '10.28.7.123')
// (in)  addrs -- array of int32 of length N
// (in)  vals -- array of int32 of length N
// (in)  callback -- the callback function when answer is computed.
//          It's two params will be the error indicator and context param.
//
bcm_soap.write_multiple = function(context, ip, addrs, vals, callback) {
	var post_url = 'http://' + ip + ':1700/ncs_control';
	if (addrs.length != vals.length) throw Error("arrays are of different lenghts");
	var uuenc_send_str_addrs = base64.encode_from_int32_arr(addrs);
	var uuenc_send_str_vals = base64.encode_from_int32_arr(vals);
	var xmlhttp = new XMLHttpRequest();

	xmlhttp.open("POST", post_url, true);
	xmlhttp.onreadystatechange=function() {
		if (xmlhttp.readyState == 4) {
			var json = XMLObjectifier.xmlToJSON(xmlhttp.responseXML);
			var err = 0; // TBD: check for error.
			if (callback != null) callback(err, context);
		}
	}

	var action = '"urn:schemas-upnp-org:service:RegSvc:1#SetRegisterCollection"';
	xmlhttp.setRequestHeader("SOAPAction", action);
	xmlhttp.setRequestHeader("Content-Type", "text/xml");
	var xml = '<?xml version="1.0" encoding="UTF-8"?>' + "\n" +
        '<soap:Envelope ' + "\n" +
        '    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ' + "\n" +
        '    xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/" ' +  "\n" +
        '    xmlns:xsd="http://www.w3.org/2001/XMLSchema" ' + "\n" +
        '    soap:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" ' + "\n" +
        '    xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">' + "\n" +
        '    <soap:Body>' + "\n" +
        '        <SetRegisterCollection xmlns="urn:schemas-upnp-org:service:NumberStorage:1">' + "\n" +
        '            <Count xsi:type="xsd:int">' + addrs.length + '</Count>' + "\n" +
        '            <RegisterCollection xsi:type="xsd:base64Binary">' +
                     uuenc_send_str_addrs +
		'            </RegisterCollection>' + "\n" +
        '            <CollectionInputValues xsi:type="xsd:base64Binary">' +
                     uuenc_send_str_vals +
		'            </CollectionInputValues>' + "\n" +
        '        </SetRegisterCollection>' + "\n" +
        '    </soap:Body>' + "\n" +
        '</soap:Envelope>' + "\n";

    xmlhttp.send(xml);
}



//==============================================================================
// (in)  context -- an object that is passed through to the callback func.
// (in)  ip -- the ip address (eg '10.28.7.123')
// (in)  addr -- an int32
// (in)  val -- an int32
// (in)  callback -- the callback function when answer is computed.
//          It's two params will be an error indicator and the context param.
//
bcm_soap.write = function(context, ip, addr, val, callback) {
    var addrs = [ addr ];
    var vals = [ val ];
    return bcm_soap.write_multiple(context, ip, addrs, vals, callback);
}



//==============================================================================
//
// (in)  context -- an object that is passed through to the callback func.
// (in)  ip -- the ip address (eg '10.28.7.123')
// (in)  addr_hi -- int32, starting address
// (in)  addr_lo -- int32 starting address
// (in)  size -- size of range in bytes
// (in)  callback -- the callback function when answer is computed.
//          It's three params will be error indicator, the context param and
//          an array of int32
//
bcm_soap.get_mem_range = function(context, ip, addr_hi, addr_lo, size, callback) {
	var post_url = 'http://' + ip + ':1700/ncs_control';
	var xmlhttp = new XMLHttpRequest();

	xmlhttp.open("POST", post_url, true);
	xmlhttp.onreadystatechange=function() {
		if (xmlhttp.readyState == 4) {
			var json = XMLObjectifier.xmlToJSON(xmlhttp.responseXML);
			if (json && ('Body' in json) && ('GetMemoryRangeResponse' in json.Body[0])) {
				var uuenc_recv_str = json.Body[0].GetMemoryRangeResponse[0].DataOut[0].Text;
				var vals = base64.decode_to_int32_arr(uuenc_recv_str);
				if (callback != null) callback(0, context, vals);
			} else {
				if (callback != null) callback(1, null, []);
			}
		}
	}

	var action = '"urn:schemas-upnp-org:service:RegSvc:1#GetMemoryRange"';
	xmlhttp.setRequestHeader("SOAPAction", action);
	xmlhttp.setRequestHeader("Content-Type", "text/xml");
	var xml = '<?xml version="1.0" encoding="UTF-8"?>' + "\n" +
        '<soap:Envelope ' + "\n" +
        '    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ' + "\n" +
        '    xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/" ' +  "\n" +
        '    xmlns:xsd="http://www.w3.org/2001/XMLSchema" ' + "\n" +
        '    soap:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" ' + "\n" +
        '    xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">' + "\n" +
        '    <soap:Body>' + "\n" +
        '        <GetMemoryRange xmlns="urn:schemas-upnp-org:service:NumberStorage:1">' + "\n" +
        '            <AddrHi xsi:type="xsd:int">' + addr_hi + '</AddrHi>' + "\n" +
        '            <AddrLo xsi:type="xsd:int">' + addr_lo + '</AddrLo>' + "\n" +
        '            <Count xsi:type="xsd:int">' + size + '</Count>' + "\n" +
        '        </GetMemoryRange>' + "\n" +
        '    </soap:Body>' + "\n" +
        '</soap:Envelope>' + "\n";

    xmlhttp.send(xml);
}




//==============================================================================
//
// (in)  context -- an object that is passed through to the callback func.
// (in)  ip -- the ip address (eg '10.28.7.123')
// (in)  addr_hi -- int32, starting address
// (in)  addr_lo -- int32 starting address
// (in)  vals -- array of int32 of length N
// (in)  callback -- the callback function when answer is computed.
//          It's two params will be an error indicator and the context param.
//
bcm_soap.set_mem_range = function(context, ip, addr_hi, addr_lo, vals, callback) {
	var post_url = 'http://' + ip + ':1700/ncs_control';
	var uuenc_send_str_vals = base64.encode_from_int32_arr(vals);
	var xmlhttp = new XMLHttpRequest();

	xmlhttp.open("POST", post_url, true);
	xmlhttp.onreadystatechange=function() {
		if (xmlhttp.readyState == 4) {
			var json = XMLObjectifier.xmlToJSON(xmlhttp.responseXML);
			var err = 0; // TBD: check for error.
			if (callback != null) callback(err, context);
		}
	}

	var action = '"urn:schemas-upnp-org:service:RegSvc:1#SetMemoryRange"';
	xmlhttp.setRequestHeader("SOAPAction", action);
	xmlhttp.setRequestHeader("Content-Type", "text/xml");
	var xml = '<?xml version="1.0" encoding="UTF-8"?>' + "\n" +
        '<soap:Envelope ' + "\n" +
        '    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ' + "\n" +
        '    xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/" ' +  "\n" +
        '    xmlns:xsd="http://www.w3.org/2001/XMLSchema" ' + "\n" +
        '    soap:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" ' + "\n" +
        '    xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">' + "\n" +
        '    <soap:Body>' + "\n" +
        '        <SetMemoryRange xmlns="urn:schemas-upnp-org:service:NumberStorage:1">' + "\n" +
        '            <AddrHi xsi:type="xsd:int">' + addr_hi       + '</AddrHi>' + "\n" +
        '            <AddrLo xsi:type="xsd:int">' + addr_lo       + '</AddrLo>' + "\n" +
        '            <Count xsi:type="xsd:int">'  + vals.length*4 + '</Count>'  + "\n" +
        '            <DataIn xsi:type="xsd:base64Binary">' +
                     uuenc_send_str_vals +
	'            </DataIn>' + "\n" +
        '        </SetMemoryRange>' + "\n" +
        '    </soap:Body>' + "\n" +
        '</soap:Envelope>' + "\n";

    xmlhttp.send(xml);
}


//==============================================================================
// (in)  context -- an object that is passed through to the callback func.
// (in)  ip -- the ip address (eg '10.28.7.123')
// (in)  addr -- an int32
// (in)  val -- an int32
// (in)  callback -- the callback function when answer is computed.
//          It's two params will be an error indicator and the context param.
//
bcm_soap.write_mem = function(context, ip, addr, val, callback) {
    var vals = [ val ];
    var addr_lo = addr & 0xffffffff;
    var addr_hi = 0;
    var tmp = addr.toString(16);
    if (tmp.length > 8)
	addr_hi = parseInt(tmp.replace(/........$/, ""), 16);

    return bcm_soap.set_mem_range(context, ip, addr_hi, addr_lo, vals, callback);
}
