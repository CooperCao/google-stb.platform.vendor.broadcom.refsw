#!/bin/sh
#
# Given a SVN URL, extract the TAG or BRANCH info. This is used by
# version generators like epivers or component mkversion generators
#

SVNURL=$1

if [ ! -n "$SVNURL" ]; then
	echo "ERROR: $0 needs SVN URL path specified"
	echo "ERROR: Example $0 http://svn.sj.broadcom.com/svn/wlansvn/tags/FALCON/FALCON_REL_5_90_100"
	exit 1
fi

case "${SVNURL}" in
	*/branches/*)
		SVNTAG=$(echo $SVNURL | sed -e 's%.*/branches/\(.*\)/src.*%\1%g' | xargs printf "%s")
		;;
	*_BRANCH_*)
		SVNTAG=$(echo $SVNURL | sed -e 's%/%\n%g' | egrep _BRANCH_ | xargs printf "%s")
		;;
	*_TWIG_*)
		SVNTAG=$(echo $SVNURL | sed -e 's%/%\n%g' | egrep _TWIG_ | xargs printf "%s")
		;;
	*/tags/*)
		SVNTAG=$(echo $SVNURL | sed -e 's%.*/tags/.*/\(.*\)/src.*%\1%g' | xargs printf "%s")
		;;
	*_REL_*)
		SVNTAG=$(echo $SVNURL | sed -e 's%/%\n%g' | egrep _REL_ | xargs printf "%s")
		;;
	*/trunk/*)
		SVNTAG=$(date '+TRUNKURL_REL_%Y_%m_%d')
		;;
	*)
		SVNTAG=$(date '+OTHER_REL_%Y_%m_%d')
		;;
esac

echo $SVNTAG
exit 0
