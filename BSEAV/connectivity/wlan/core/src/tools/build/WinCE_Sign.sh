#!/bin/sh
#
# Script to sign WinCE objects from certificate that keep changing
#
# First priority is to use centrally stored signing certificate and tool
# and fall back to branch version, only if network versions aren't visible
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$

signscript=$0
signobjects=$*
signhost=`hostname`
signtime=`date '+%Y/%m/%d %H:%M:%S'`
winpfx=${WLAN_WINPFX:-Z:}

if [ -f 'c:/tools/build/bcmretrycmd.exe' ]
then
    retrycmd=c:/tools/build/bcmretrycmd.exe
else
    retrycmd=$winpfx/projects/hnd_software/gallery/src/tools/build/bcmretrycmd.exe
fi

signtool=$winpfx/projects/hnd/tools/win/msdev/WinCE-Cert/signtool.exe
signcert=$winpfx/projects/hnd/tools/win/msdev/WinCE-Cert/SDKSamplePrivDeveloper.pfx
signpass=brcm
signscriptdir=`dirname $0`


if [ ! -s "$signtool" ]; then
	$signtool="$signscriptdir/signtool.exe"
fi

if [ ! -s "$signcert" ]; then
	$signcert="$signscriptdir/SDKSamplePrivDeveloper.pfx"
fi


echo "[$signhost $signtime] $retrycmd $signtool sign /p $signpass /v /f $signcert $signobjects"
$retrycmd $signtool sign /p $signpass /v /f $signcert $signobjects
