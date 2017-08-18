#!/bin/sh
#
# Script to do periodic git house-keeping tasks on current WLAN S/W GIT repos
#
# Primary contact: Arend van Spriel
#
# $Id$

ADMIN="arend@broadcom.com hnd-software-scm-list@broadcom.com"

GITCMD=/tools/bin/git
GITREPO_ROOT=/projects/hnd_swgit

echo "[`date`] Running 'git gc' on `hostname` as $USER"

# Show current git version
$GITCMD --version

# Cleanup unnecessary files and optimize the local git repository
for gitdir in $(find $GITREPO_ROOT -path '$GITREPO_ROOT' -prune -o -type d -name "*\.git" -print  2> /dev/null)
do
	cd $gitdir
	echo "[`date '+%Y/%m/%d %H:%M:%S'] cd $gitdir; $GITCMD gc"

	$GITCMD gc
	gitrc=$?
	if [ "$gitrc" != "0" ]; then
		warnmsgs="$warnmsgs: $gitdir git gc returned $gitrc exit code\n"
		gitec=1
	fi
done

if [ "$gitec" != "0" ]; then
	echo "[`date '+%Y/%m/%d'`] WARN: git gc returned exit code\n\n$warnmsgs" | \
		mail -s "[`date '+%Y/%m/%d'`] WARN: git gc returned exit code" \
		$ADMIN	
fi
