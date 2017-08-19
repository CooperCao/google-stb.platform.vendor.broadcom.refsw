##########################################################################
# This file is meant to be sourced.
# Defines functions which call dhd to display dongle console output.
# Environment variables are used to save state between calls:
#   cons0 -> address of cons0 structure
#   logbuf -> address of actual log buffer
#   maxidx -> buffer size (maximum index)
#   lastidx -> last index displayed
#
# Function "setcons" sets initializes vars for fresh console run
# Function "showcons" reads structure and displays new output
#  with -a argument shows all data in the cons structure (e.g. if wrapped)
############################################################################
#
# $Id: dhd_consp.sh,v 12.2 2007-04-03 06:21:42 $
#

function setcons {
    if [ -z "$1" ]; then
	echo "Usage: setcons <address>"
	return 1
    fi

    cons0=$1
    cons0=$(($cons0 & 0xffffff))
    unset logbuf maxidx lastidx

    if (($cons0 != $1)); then
	echo "Masking cons0 to low 24 bits of address: $1 -> $cons0"
    fi

    if ! consinfo=($(dhd membytes $cons0 4)); then
	echo "Failed initial pointer read"
	return 2
    fi

    if [ "${#consinfo[@]}" -lt 5 ]; then
	echo "Unexpectedly short pointer read: ${consinfo[*]}"
	return 2
    fi

    cons0=0x${consinfo[4]}${consinfo[3]}${consinfo[2]}${consinfo[1]}
    if (($cons0 & 0xff000000)); then
	echo "Masking cons0 structure address $cons0 to 24 bits."
    fi
    cons0=$(($cons0 & 0xffffff))

    if ! consinfo=($(dhd membytes $cons0 0x10)); then
	echo "Failed on initial membytes read"
	return 2
    fi

    if [ "${#consinfo[@]}" -lt 18 ]; then
	echo "Unexpectedly short initial read: ${consinfo[*]}"
	return 2
    fi

    logbuf=0x${consinfo[12]}${consinfo[11]}${consinfo[10]}${consinfo[9]}
    maxidx=0x${consinfo[16]}${consinfo[15]}${consinfo[14]}${consinfo[13]}

    if (($logbuf & 0xff000000)); then
	echo "Masking logbuf address $logbuf to 24 bits."
    fi
    logbuf=$(($logbuf & 0xffffff))

    lastidx=0
}

function showcons {
    local curidx;

    if [ "$1" ] && [ "$1" != "-a" ] && [ "$1" != "-d" ]; then
	echo Usage: showcons [-a | -d]
	return 1
    fi

    if [ -z "$logbuf" ] || [ -z "$maxidx" ] || [ -z "$lastidx" ]; then
	echo "Expected environment variables empty, use setcons first."
	return 1
    fi

    if ! consinfo=($(dhd membytes $(($cons0 + 0x10)) 0x10)); then
	echo "Failed on initial membytes read for curidx"
	return 2
    fi

    if [ "${#consinfo[@]}" -lt 18 ]; then
	echo "Unexpectedly short initial read: ${consinfo[*]}"
	return 2
    fi

    curidx=0x${consinfo[4]}${consinfo[3]}${consinfo[2]}${consinfo[1]}

    if (($curidx > $maxidx)); then
	echo "Invalid: curidx exceeds maxidx ($curidx > $maxidx)"
	return 2
    fi

    if [ "$1" = "-d" ]; then
	echo "cons0 $cons0"
	echo "logbuf $logbuf"
	echo "maxidx $maxidx"
	echo "lastidx $lastidx"
	echo "curidx $curidx"
    else
	if [ "$1" = "-a" ]; then
	    dhd membytes -r $(($logbuf + $curidx)) $(($maxidx - $curidx))
	    dhd membytes -r $(($logbuf)) $curidx
	else
	    if (($curidx < $lastidx)); then
		dhd membytes -r $(($logbuf + $lastidx)) $(($maxidx - $lastidx))
		lastidx=0
	    fi
	    dhd membytes -r $(($logbuf + $lastidx)) $(($curidx - $lastidx))
	fi
	lastidx=$curidx
    fi
}

function clearcons {
    local clrloc
    local clrsiz
    local remains

    if [ -z "$logbuf" ] || [ -z "$maxidx" ] || [ -z "$lastidx" ]; then
	echo "Expected environment variables empty, use setcons first."
	return 1
    fi

    remains=$maxidx
    clrloc=$logbuf
    clrsiz=2000

    while (($remains)); do
	if (($remains <= 2000)); then
	    clrsiz=$remains
	fi
	dhd membytes -h $clrloc $clrsiz 00
	clrloc=$(($clrloc + $clrsiz))
	remains=$(($remains - $clrsiz))
    done
}
