#!/bin/bash
kernelSrcUrl=http://www.kernel.org/pub/linux/kernel/v3.0/linux-3.0.tar.bz2
kernelSrcTar=`basename $kernelSrcUrl`
kernelSrcDir=`echo $kernelSrcTar | perl -ne 'print split /.tar.bz2/'`
myDate=`date +%F%H%M`
stageDir=/projects/hnd_software_misc/swdata/linuxkernel-stage
scriptPath=$PWD

function makeKernel
{
#DISABLED: make defconfig >> defconfig.out.$myDate 2>&1
make >> make.out.$myDate 2>&1
}

function getKernel
{
rm -f $kernelSrcTar
wget $kernelSrcUrl
rm -rf $kernelSrcDir
tar xjf $kernelSrcTar
}

pushd $stageDir
  getKernel
  cp $scriptPath/.config $kernelSrcDir
  pushd $kernelSrcDir
    makeKernel
    echo "$0 has finished. Test a hybrid build (linux-external-wl-portsrc-hybrid), then
       upload to /tools/linux/src and update this symbolic link latest_generic64" | 
       mail -s "[scriptmail]: $0 has finished" jvarghes@broadcom.com
  popd
popd
