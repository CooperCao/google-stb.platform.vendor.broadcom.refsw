#! /bin/sh
# 	$Id$	
# Cleans up unused oem*.inf packages.
# Uses iconv to read .inf files

# Uses shell pipelining so that deletion can start while scanning is
# still in progress.

# dp_delete may incorrectly identify broken drivers as non-third
# party.  Delete these directly.

type iconv >/dev/null || exit 0

if [ -z "$*" ]; then
    match=';; bcm.*\.inf'
    force=''
else
    match=$(echo "$*"|sed 's/\\/\\\\/g')
    force=-f
fi

cd $(cygpath -W)/inf

(for i in oem*.inf; do
    if grep -i "$match" $i >/dev/null || \
	iconv -c -f UTF-16LE $i 2>&1|grep -i "$match" >/dev/null; then
	echo $i
    fi
    done
) | while read j; do
    d=$(devcon $force dp_delete $j 2>&1)
    if [ $? -eq 0 ]; then
	echo "$j: deleted with devcon"
    elif expr "$d" : ".* third-party" >/dev/null; then
	echo "$j: cleaned manually"
	rm -f $(basename $j .inf).{inf,PNF}
#    else
#	echo "$j: $d"
    fi
done
