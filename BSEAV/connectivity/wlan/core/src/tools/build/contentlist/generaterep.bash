#!/bin/bash
tag=$1;
branch=$2;
bomName=$3;
inFile=$4;
export PS4='+${BASH_SOURCE}:${LINENO}:${FUNCNAME[0]}: '

export _DEBUG=true

export CVSROOT=":pserver:wlanweb:Brcm123!@cvsps-sj1-1.sj.broadcom.com:/projects/cvsroot"
rm -rf src/tools/release
cvs co src/tools/release

cvs stat -v src/tools/release/$bomName.mk  |grep $tag
if [ $? == 0 ]; then
rm -rf src/tools/build
cvs co src/tools/build
rm -rf src/hndcvs
cvs co src/hndcvs
cp ../src/hndcvs/hndcvs_backend src/hndcvs/
src/tools/build/hndcvs -rlog -dr $bomName $tag $branch
cp "diff_"$bomName"_"$tag"_"$branch.txt $inFile
fi
