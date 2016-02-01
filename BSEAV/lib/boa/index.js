/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/
function rtrim(stringToTrim) { return stringToTrim.replace(/\s+$/,""); }
function MyLoad ()
{
    var debug=0;
    xmlhttp=new XMLHttpRequest();

    var url = "/cgi/index.cgi";

    if(debug) alert("sending " + url );
    xmlhttp.onreadystatechange= serverHttpResponse;
    xmlhttp.open("GET",url,true);
    xmlhttp.send(null);
};

function serverHttpResponse ()
{
    var debug=0;

    /* Because this function runs as an asynchronous response to a previous server request, the value of "this"
       is not the same as the constructor value of "this". For this reason, we use the global variable "gthis"
       to access the previous instantiation of "this"
    */
    if(debug) alert("serverHttpResponse: got readyState " + xmlhttp.readyState );

    if (xmlhttp.readyState==4 )
    {
        //alert("serverHttpResponse: got readyState " + xmlhttp.readyState + "; status " + xmlhttp.status );

        //var response = xmlhttp.getAllResponseHeaders();
        //var response = xmlhttp.getResponseHeader("Date");
        //alert("Date from header is " + response);

        // only if "OK"

        if (xmlhttp.status == 200)
        {
            var response_list = "";
            var objdebug = document.getElementById("debugoutputbox");

            if(debug) alert("setting debugdiv");
            var safeHTML1 = xmlhttp.responseText.replace(/</g,'&lt;');
            var safeHTML2 = safeHTML1.replace(/>/g,'&gt;');
            if(objdebug) objdebug.innerHTML = safeHTML2 // fails on ie7, safari
            if(debug) alert("innerHTML done");

            var responseText = rtrim(xmlhttp.responseText);
            var oResponses = responseText.split( "~" );
            if(debug) alert("num responses is " + oResponses.length );

            //alert("for i = 0 to " + oResponses.length );
            // loop through <response> elements, and add each nested
            for (var i = 0; i < oResponses.length; i++)
            {
                var entry = oResponses[i];
                if(debug) alert("Entry " + entry );
                if ( entry == "HTMLFILES" )
                {
                    var obj=document.getElementById('hrefdiv');
                    if ( obj )
                    {
                        //if(objdebug) objdebug.innerHTML += "going to set innerHTML for " + Entry + "<br>\n";
                        obj.innerHTML = oResponses[i+1];
                        //alert(obj.innerHTML );
                        //if(objdebug) objdebug.innerHTML += "done setting innerHTML for " + Entry + "<br>\n";
                    }
                }
            }
        }

        else
        {
            alert("There was a problem retrieving the XML data:\n" + xmlhttp.statusText);
        }

    } //if (xmlhttp.readyState==4 )
} // this.serverHttpResponse
