<!DOCTYPE html>
<!--
 HTML part for WiFi Blanket Demo Tab

 $ Copyright Open Broadcom Corporation $


 <<Broadcom-WL-IPTag/Open:>>

 $Id: wbd_demo.asp 761224 2018-05-07 07:14:30Z tp888369 $
-->

<html>
<head>
<meta charset="ISO-8859-1">
<link rel="stylesheet" href="wbd_demo.css"/>
<script src="jquery-2.0.3.min.js"></script>
<script src="wbd_demo.js"></script>
<title>Broadcom SmartMesh</title>
</head>
<body>
	<div id="main_div">
		<table class="logotable">
			<tr>
				<td colspan="2" class="edge"><img border="0" src="blur_new.jpg" alt=""/></td>
			</tr>
			<tr>
				<td><img border="0" src="logo_new.gif" alt=""/></td>
				<td width="100%" valign="top">
					<br/>
					<span id="TextHeading"> <h1>Broadcom SmartMesh</h1> </span><br/>
				</td>
			</tr>
		</table>
		<div id="contentarea" class="btmbrdr">
			<div id="wbdcontent" class="maindiv">
				<div id="5gdiv" class="outerdivcommon">
					<h2 id="heading"> 5G Low </h2>
				</div>
				<div id="5gdivH" class="outerdivcommon">
					<h2 id="heading"> 5G High </h2>
				</div>
				<div id="2gdiv" class="outerdivcommon">
					<h2> 2.4G </h2>
				</div>
				<div id="logsdiv" class="outerdivcommon">
					<h2> Logs </h2>
					<div id="logsdivcontainer" class="commonbdr innerdivcommonforlogs">
						<div id="stamsgs" class="stamsgstyle txtstyle">
						</div>
					</div>
					<button id="clearlogs" type="button" class="logsbtn">Clear Logs</button>
				</div>
				<br style="clear:left"/>
			</div>
		</div>
	</div>

	<div id="templatesAdv" style="display:none">
	<table id="tableTemplate" class="tablestylecommon">
		<thead>
		<tr>
			<th style="width:9.5em">Client (MAC)</th>
			<th style="width:9.5em">RSSI</th>
		</tr>
		</thead>
		<tbody>
		</tbody>
	</table>
	</div>

</body>
</html>
