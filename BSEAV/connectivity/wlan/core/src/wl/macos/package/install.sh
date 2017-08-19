#!/bin/sh

# install
# Install AirPortBroadcom43XX.kext
# Copyright © 2012 Broadcom Corporation.  All rights reserved.

CURDIR=`dirname $0`
INSTALLDIR=/System/Library/Extensions
UNINSTALL='0'
FAMILYDIR=$INSTALLDIR/IO80211Family.kext/Contents/Plugins
FAMILYVER=NoUpdateNeeded
FAMILYOLD="IO80211Family-Caravel-511.6.tar.gz Family_Drop29.tgz 120810_Drop28.tar.gz Family_Drop27.tgz"

# Check for new driver in default location
if [ $# -eq 0 ] && [ -d $CURDIR/AirPortBroadcom43XX.kext ]; then
	KEXTNAME=$CURDIR/AirPortBroadcom43XX.kext
else
	if [ $# -lt 1 ]; then
		echo "ERROR: Need to specify driver kext"
		echo "usage: $0 [-u] [driver_kext] [Family_Drop]"
		echo "       -u Uninstall BRCM and restore production drivers"
		exit 1
	fi
	# check for uninstall
	if [ $# -eq 1 ] && [ $1 == '-u' ]; then
		UNINSTALL='1'
	else
		KEXTNAME=$1
		if [ ! -d $KEXTNAME ]; then
			echo "ERROR: Cannot find driver $KEXTNAME"
			echo "usage: $0 [driver_kext] [Family_Drop]"
			exit 1
		fi
	fi
fi

# Check for 10.8 (Mountain Lion)
sw_vers | grep 10.8
if [ $? == '0' ]; then
	# Check if matching family is specified
	if [ $# -lt 2 ]; then
	    if [ $FAMILYVER != 'NoUpdateNeeded' ]; then
		# Check default location for Family file
		if [ $# -eq 0 ] && [ -e $CURDIR/../../$FAMILYVER ]; then
			FAMILYFILE=$CURDIR/../../$FAMILYVER
		else
			echo "ERROR: Need to specify Family Drop"
			echo "usage: $0 [driver_kext] [Family_Drop]"
			exit 1
		fi
	    fi
	else
		# Family file specified
		FAMILYFILE=$2
		FAMILYVER=`basename $2`
		if [ ! -e $FAMILYFILE ]; then
			echo "ERROR: Cannot find Family Drop $FAMILYFILE"
			echo "usage: $0 [driver_kext] [Family_Drop]"
			exit 1
		fi
	fi

	# Check if matching family is installed
	sudo darwinup list | grep $FAMILYVER
	if [ $? == '1' ]; then
		# Uninstall any previous Family Drops, except 12Dxx
		sw_vers | grep 12D
		if [ $? == '1' ]; then
			for i in $FAMILYOLD; do
				sudo darwinup list | grep $i
				if [ $? == '0' ]; then
					echo "Uninstalling $i..."
					sudo darwinup -f uninstall $i
				fi
			done
		fi

		# Install new family
		if [ $UNINSTALL == '0' ] && [ $FAMILYVER != 'NoUpdateNeeded' ]; then
			echo "Install IO80211 $FAMILYVER..."
			sudo darwinup install $FAMILYFILE
		fi
	fi
fi

# Remove any previously installed drivers
echo "Remove any previously installed drivers..."
sudo rm -rf $FAMILYDIR/AirPortBroadcom43XX.kext
sudo rm -rf $INSTALLDIR/AirPortBroadcom43XX.kext

if [ $UNINSTALL == '0' ]; then
	# Move production drivers out of the way
	PRODLIST=`sudo ls -d $FAMILYDIR/*Brcm*.kext`
	if [ $? == '0' ]; then
		echo "Move production drivers out of the way..."
		for prodkext in $PRODLIST; do
			if [ -d ${prodkext} ]; then
				sudo rm -rf ${prodkext}.hide
				sudo mv ${prodkext} ${prodkext}.hide
			fi
		done
	fi

	# Install the new driver
	echo "Install new driver..."
	sudo cp -R $KEXTNAME $INSTALLDIR
else
	# Restore production drivers
	PRODLIST=`sudo ls -d $FAMILYDIR/*Brcm*.hide`
	if [ $? == '0' ]; then
		echo "Restore production drivers..."
		for prodkext in $PRODLIST; do
			TEMP=`basename ${prodkext} .hide`
			if [ -d ${prodkext} ]; then
				sudo mv ${prodkext} $FAMILYDIR/$TEMP
			fi
		done
	fi
fi

# Add unsigned driver override, if needed
sudo nvram -p | grep boot-arg | grep kext-dev-mode
if [ $? != '0' ]; then
	OLDBOOTARGS=`sudo nvram -p | grep boot-arg | sed /boot-args/s///`
	sudo nvram boot-args="$OLDBOOTARGS kext-dev-mode=1 rootless=0"
fi
sudo plutil -replace "Kernel Flags" -string "debug=0x14e -v msgbuf=0x100000 kext-dev-mode=1 rootless=0" /Library/Preferences/SystemConfiguration/com.apple.Boot.plist

# Remove the driver cache, need to do this if the module is swapped
echo "Remove driver cache..."
sudo rm -rf /System/Library/Caches/com.apple.kext.caches

# Force driver cache rescan
echo "Force driver cache rescan..."
sudo touch $INSTALLDIR

# Report completion and reboot
tput clear
tput cup 5 30
tput setaf 1
tput bold
tput rev
if [ $UNINSTALL == '0' ]; then
echo "DRIVER INSTALLED SUCCESSFULLY!!"
else
echo "DRIVER UNINSTALLED SUCCESSFULLY!!"
fi
tput sgr0
tput cup 10 0
echo "Restart in 15s to load new driver..."
sleep 15
sudo shutdown -r now "Restarting now!"
exit 0
