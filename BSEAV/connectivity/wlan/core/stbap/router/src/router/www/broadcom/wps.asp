<!--
Copyright (C) 2018, Broadcom Corporation. All Rights Reserved.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

$Id:
-->

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html lang="en">
<head>
<title>Broadcom Home Gateway Reference Design: WPS</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" type="text/css" href="style.css" media="screen">
<script language="JavaScript" type="text/javascript" src="overlib.js"></script>
<script language="JavaScript" type="text/javascript">
<!--

var wps_interval = 0;
var wps_refresh = "<% wps_refresh(); %>";
if(wps_refresh) {
	wps_interval = parseInt(wps_refresh);
	if (wps_interval != 0 && wps_interval < 2)
		wps_interval = 2;
}

/*	
*/

function wps_config_change()
{
<% wps_config_change_display(); %>
}

function wps_current_psk_window() 
{
<% wps_current_psk_window_display(); %>
}

function wps_psk_window() 
{
<% wps_psk_window_display(); %>
}

function wps_akm_change()
{
<% wps_akm_change_display(); %>
}

function wps_get_ap_config_submit()
{
<% wps_get_ap_config_submit_display(); %>
}

function pre_submit()
{
	var action = document.forms[0].wps_action.value;
	var akm = document.forms[0].wps_akm[document.forms[0].wps_akm.selectedIndex].value;

	if (action == "ConfigAP" || action == "AddEnrollee") {
		/* Check WPS in OPEN security */
		if (akm == "0")
			return confirm("Are you sure to configure WPS in Open security?");
	}
	if (action == "GetAPConfig")
		return wps_get_ap_config_submit();
	return true;
}

function autoRefresh()
{
/*	
*/
	{
		if (wps_interval > 0)
			setTimeout("location.reload(true);", wps_interval * 1000);
	}
}

function wps_onLoad()
{
/*	
*/
	wps_akm_change();

	autoRefresh();
}

//-->
</script>
</head>

<body onload="wps_onLoad();">
<div id="overDiv" style="position: absolute; visibility: hidden; z-index: 1000;"></div>

<table border="0" cellpadding="0" cellspacing="0" width="100%" bgcolor="#cc0000">
  <% asp_list(); %>
</table>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tbody><tr>
    <td colspan="2" class="edge"><img src="blur_new.jpg" alt="" border="0"></td>
  </tr>
  <tr>
    <td><img src="logo_new.gif" alt="" border="0"></td>
    <td valign="top" width="100%">
	<br>
	<span class="title">WPS</span><br>
	<span class="subtitle">This page allows you to configure WPS.</span>
    </td>
  </tr>
</tbody></table>

<form method="post" action="wps.asp">
<input name="page" value="wps.asp" type="hidden">
<input name="invite_name" value="0" type="hidden">
<input name="invite_mac" value="0" type="hidden">
<p>
<table border="0" cellpadding="0" cellspacing="0">
  <tr>
    <th width="310"
	onMouseOver="return overlib('Selects which wireless interface to configure.', LEFT);"
	onMouseOut="return nd();">
	Wireless Interface:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	<select name="wl_unit" onChange="submit();">
	  <% wl_list("INCLUDE_SSID" , "INCLUDE_VIFS"); %>
	</select>
    </td>
    <td>
	<button type="submit" name="action" value="Select">Select</button>
    </td>
  </tr>
</table>

<p>
<% wps_display(); %>

<!--
-->
<p>
<table border="0" cellpadding="0" cellspacing="0">
    <tr>
      <td width="310"></td>
      <td>&nbsp;&nbsp;</td>
      <td>
	  <input type="submit" name="action" value="Apply" onclick="wps_config_change()";>
	  <input type="reset" name="action" value="Cancel">
      </td>
    </tr>
</table>

<!--
-->

<p>

<p class="label">&#169;2001-2016 Broadcom Limited. All rights reserved. 54g and XPress are trademarks of Broadcom Limited.</p>

</form></body></html>
