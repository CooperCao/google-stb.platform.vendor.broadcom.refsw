#! /bin/bash
#
# winupdate_tools_build.sh - Do a svn update of build scripts at /tools/build.
#
# $Id$

TOOLS_BUILD_DIR=${TOOLS_BUILD_DIR:-/cygdrive/c/tools/build}
LOG=/tmp/,update_svn_tools_build.log
BLDADMIN="hnd-software-scm-list@broadcom.com"
BLDUSER=hwnbuild@broadcom.com
SVN_CYG="C:/tools/Subversion/svn.exe"

rm -f $LOG
notify=0
svnrc=0
(
    # Redirect stdout and stderr to ${LOG}
    exec 3> ${LOG}
    exec 1>&3 2>&3

    if [ -d ${TOOLS_BUILD_DIR} ]; then
	    cd ${TOOLS_BUILD_DIR}
	    ${SVN_CYG} cleanup
	    ${SVN_CYG} --non-interactive update
	    svnrc=$?
    else
	    echo "TOOLSDIR: $(TOOLS_BUILD_DIR) not found"
    fi;
)

cd /tmp
if [[ $svnrc != 0 ]]; then
	subject="ERRORS: SVN update on ${COMPUTERNAME}"
	notify=1
fi

if [[ $notify != 0 ]]; then
	blat $LOG -t ${BLDADMIN} -f ${BLDUSER} -s "${subject}"
fi
