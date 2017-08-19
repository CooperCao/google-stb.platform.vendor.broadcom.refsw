#!/bin/bash
#
# On windows servers, map network drive or home drive on console
# session, if it is missing for some reason
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#

SERVER_USER=$(whoami > /dev/null 2>&1)
SERVER_DRIVER_LOG=c:/temp/server_network_drive_${SERVER_USER}.log
[ -w ${SERVER_DRIVER_LOG} ] || chmod -v +w ${SERVER_DRIVER_LOG}

(
	# Redirect stdout and stderr to ${SERVER_DRIVER_LOG}
	exec 3>> ${SERVER_DRIVER_LOG}
	exec 1>&3 2>&3

	SERVERNAME=$(uname -n)
	echo "DBG: Drive Status on `date '+%Y/%d/%m %H:%M:%S'`"
	net use

	projdrv=Z
	drvstat=$(net use 2>&1 | egrep -i "unavailable.*${projdrv}:")

	if [ "$drvstat" != "" -o ! -d "${projdrv}:/" ]; then
		echo "===== [`date '+%Y/%d/%m %H:%M:%S'`] ======"
		echo "Trying to mount ${projdrv}: on $SERVERNAME"
		echo "Deleting bad drive mount ${projdrv}: first"
		echo "net use ${projdrv}: /d"
		echo Y | net use ${projdrv}: /d

		echo "net use ${projdrv}: \\\\brcm-sj\\dfs /persistent:yes"
		echo Y | net use ${projdrv}: \\\\brcm-sj\\dfs /persistent:yes
		net use ${projdrv}:
	fi

	homedrv=P
	drvstat=$(net use 2>&1 | egrep -i "unavailable.*${homedrv}:")
	if [ "$drvstat" != "" -o ! -d "${homedrv}:/" ]; then
		echo "===== [`date '+%Y/%d/%m %H:%M:%S'`] ======"
		echo "Trying to mount ${homedrv}: on $SERVERNAME"
		echo "Deleting bad drive mount ${homedrv}: first"
		echo "net use ${homedrv}: /d"
		echo Y | net use ${homedrv}: /d
		echo "net use ${homedrv}: \\\\brcm-sj\\dfs\\home\\${USERNAME} /persistent:yes"
		echo Y | net use ${homedrv}: \\\\brcm-sj\\dfs\\home\\${USERNAME} /persistent:yes
		net use ${homedrv}:
	fi
)
