#!/bin/bash
#
# PackageMaker build script for
# Broadcom 802.11abg Networking Device Driver Install Package
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
#
# <<Broadcom-WL-IPTag/Private1743:>>
#
# $Id: build_package.sh,v 1.21 2010-10-21 18:01:43 $
#

if [ $# -lt 2 ]; then
	echo "$0: too few arguments"
	echo "usage: $0 [src_kext] [dest_name]"
	exit 1
fi

virtif=`perl -e '{ $_ = @ARGV[0]; \
    chop; \
    if ($_ =~ /VirtIf_1/) { print "1"} else { print "0"}; }' $1`;

src=$1
if [ ! -d $src ]; then
    echo "$0: Can't find source kext \"$src\", exiting ..."
    exit 1
fi

dst=$2
if [ ! -d `dirname $dst` ]; then
    echo "$0: Can't find output directory \"`dirname $dst`\", exiting ..."
    exit 1
fi

if [ $virtif -eq 1 ]; then
    virtif_kext=$3
    virtif_profile=`perl -e '{ print $1 if $ARGV[0] =~ /(Debug|Release)/; }' $src`;
    virtif_os=`perl -e '{ print $1 if $ARGV[0] =~ /(10_5|10_4|10_6)/; }' $src`
    virtif_kext_file="../$virtif_kext/build/$virtif_kext"_"$virtif_profile"_"$virtif_os"/"$virtif_kext".kext
    if [ ! -d $virtif_kext_file ]; then
	echo "$0: Can't find VIRTIF kext file $virtif_kext_file, exiting ..."
	exit 1
    fi
fi

if [ -f /Developer/Tools/packagemaker ]; then
	packagemaker=/Developer/Tools/packagemaker
elif [ -f /Developer/usr/bin/packagemaker ]; then
	packagemaker=/Developer/usr/bin/packagemaker
else
    echo "$0: Can't find packagemaker tool, exiting ..."
    exit 1
fi

remove_kext() {
src_tmp=$1
tempdir=$2
kext=`basename $src_tmp`
#sudo chown -R `id -u` $tempdir/$kext
}

setup_kext() {
src_tmp=$1
tempdir=$2
kext=`basename $src_tmp`
kext_base=`basename $kext .kext`

# Make a copy of the kext from the input dir to our tmp dir
cp -R $src_tmp $tempdir

# change the kext permissions to rwxr-xr-x for the directories
# and rw-r--r-- for the regular files

chmod -R 755 $tempdir/$kext
chmod 644 $tempdir/$kext/Contents/Info.plist
chmod 644 $tempdir/$kext/Contents/MacOS/$kext_base

# change the kext ownership to root:wheel
# sudo will prompt for a password unless the machine executing 
# this make has been modified to allow non-password execution of chown
#sudo -p "User %u password in order to execute chown:" \
#    chown -R root:wheel $tempdir/$kext
}

tempdir=`mktemp -d /private/tmp/package_tmp.XXXX` || exit 1
setup_kext $src $tempdir

if [ $virtif -eq 1 ]; then
    setup_kext $virtif_kext_file $tempdir
fi

if [ -d $dst ]; then rm -rf $dst; fi

# Inject OS Version requirement for installation
os_ver=`perl -e '{ $_ = @ARGV[0]; \
		$_ =~ s/^Debug_VirtIf_(.*)\/(.*)/\1/; \
		$_ =~ s/^Release_VirtIf_(.*)\/(.*)/\1/; \
		$_ =~ s/^Debug_P2P_(.*)\/(.*)/\1/; \
		$_ =~ s/^Release_P2P_(.*)\/(.*)/\1/; \
		$_ =~ s/^Debug_Mfg_(.*)\/(.*)/\1/; \
		$_ =~ s/^Debug_AP_(.*)\/(.*)/\1/; \
		$_ =~ s/^Debug_(.*)\/(.*)/\1/; \
		$_ =~ s/^Release_Mfg_(.*)\/(.*)/\1/; \
		$_ =~ s/^Release_AP_(.*)\/(.*)/\1/; \
		$_ =~ s/^Release_(.*)\/(.*)/\1/; \
		$_ =~ s/_/./; \
		print $_."\n";}' $dst`
dir_dst=`perl -e '{ $_ = @ARGV[0]; \
		$_ =~ /^(.*)\//; \
		print $1;}' $dst`
next_os_ver=`perl -e '{ $_ = @ARGV[0];\
		$_ =~ /(.*)\.(.*)/; \
		print $1, ".", $2+1;}' $os_ver`
p2p=`perl -e '{ $_ = @ARGV[0]; \
    chop; \
    if ($_ =~ /P2P_1/) { print "1"} else { print "0"}; }' $1`;

if [ $p2p -eq 1 ]; then
    file=package_P2P_Info.plist
    iokit=/projects/hnd_archives/Mac/IO80211Family/2010_03_12_P2P/IO80211Family.pkg
else
    file=package_Info.plist
    if [ $os_ver == "10.6" ]; then
	iokit=/projects/hnd_archives/Mac/IO80211Family/2009_08_20_SL/IO80211Family.pkg
    fi

    if [ $os_ver == "10.7" ]; then
	file=package_P2P_Info.plist
	iokit=/projects/hnd_archives/Mac/IO80211Family/Barolo/2011_07_13_Lion_400_40/IO80211Family.pkg
    fi
fi
sed -e "s/OS_VER/${os_ver}/" $file > tmp_package_Info.plist
cp tmp_package_Info.plist tmp2_package_Info.plist
sed -e "s/NEXT_OSVER/${next_os_ver}/" tmp2_package_Info.plist > tmp_package_Info.plist
cp tmp_package_Info.plist tmp2_package_Info.plist

iokit_ver=`perl -e '{ $_ = @ARGV[0]; \
		if ($_ =~ /10_5/) { \
		     print "214.1\n"; \
		} elsif ($_ =~ /10_4/) { \
                     print "160.2\n"; \
		} elsif ($_ =~ /P2P_10_6/) { \
		     print "310.5\n"; \
		} elsif ($_ =~ /10_6/) { \
		     print "300.12\n"; \
		} elsif ($_ =~ /10_7/) { \
		     print "400.40\n"; \
		     } }' $dst`
sed -e "s/IOKIT_VER/${iokit_ver}/" tmp2_package_Info.plist > tmp_package_Info.plist

$packagemaker -build \
    -p $dst -f $tempdir \
    -ds \
    -r resources \
    -i tmp_package_Info.plist -d package_Description.plist

rm tmp*_package_Info.plist

pkg_err=$?

# change ownership back and delete the tmp dir
remove_kext $src $tempdir

if [ $virtif -eq 1 ]; then
    remove_kext $virtif_kext_file $tempdir
fi
rm -rf $tempdir

# packagemaker appears to return 1 even if success
#echo "PackageMaker return $pkg_err"
#if [ $pkg_err -ne 0 ]; then
#    echo "$0: Failure $pkg_err from packagemaker"
#    exit 1
#fi

if [ ! -d $dst ]; then
    echo "$0: Failed to create install package \"$dst\""
    exit 1
fi

if [ -d $iokit ]; then
#    echo "$0: copying $iokit \"$dir_dst\""
    cp -R $iokit $dir_dst
fi
exit 0
