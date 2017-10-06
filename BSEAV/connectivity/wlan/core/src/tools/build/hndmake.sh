#!/bin/bash
#
# Wrapper script to call out electric cloud emake.exe in place of gmake
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id: hndmake.sh,v 12.22 2011-01-05 21:43:12 $
#

NULL=/dev/null
BLDUSER="hwnbuild"
PRECOUSER="hwnprebd"
PRECOUSER_DBG="hwndebug"
CIUSER="wlanswci"
CURUSER=$(whoami 2> $NULL)

if echo $CURUSER | egrep -q "$BLDUSER|$PRECOUSER|$PRECOUSER_DBG|$CIUSER"; then
   VALID_ECUSER=1
else
   VALID_ECUSER=0
fi

# Default CM across all platforms, if no platform specific are needed
if [ "$EMAKE_CM" == "" ]; then
	# The current CM with rhel4 nodes and windows infra
	export EMAKE_CM="wc-sjca-e001.sj.broadcom.com"
	# The new CM with rhel5 nodes
	# export EMAKE_CM="eam-sj1-001.sj.broadcom.com"
fi

# Platform specific settings
case "`uname`" in
	Linux)
		umask 002
		# Redirect all Linux EA builds to RHEL5 cluster
		export EMAKE_CM="eam-sj1-001.sj.broadcom.com"
		# Emake history archive
		ECHIST_DIR=/projects/hnd_swbuild/build_linux/PRESERVED/ECHISTORY
		export EMAKE_ROOT=`pwd`
		if echo "$EMAKE_CM" | egrep -i -q "wc-sjca-e001"; then
			export PATH=/projects/hnd/tools/linux/ecloud-5.0.0/i686_Linux/bin:/opt/ecloud/i686_Linux/bin:${PATH}
		else
			if echo "$(uname -m)" | egrep -i -q "x86_64"; then
				export PATH=/projects/hnd/tools/linux/ecloud/i686_Linux/64/bin:/opt/ecloud/i686_Linux/64/bin:${PATH}
			else
				export PATH=/projects/hnd/tools/linux/ecloud/i686_Linux/bin:/opt/ecloud/i686_Linux/bin:${PATH}
			fi
		fi
		# Linux resource on the cluster
		export HNDMAKE_RESOURCE="64bit-linux"
		export HNDMAKE_CLASS="64bit-linux"
		# Max available linux agents are 48 and 72 windows agents
		# Max number of agents is set to 24 as only linux firmware
		# builds are done
		export multi_agents=32
		# export one_agent=1
		export num_agents=${multi_agents}
		EXTRA_EMAKEFLAGS="--emake-maxagents=${num_agents}"
		# Following are debug flags, disable when done with debugging
		# EXTRA_EMAKEFLAGS="${EXTRA_EMAKEFLAGS} --emake-debug=afjn"
		# EXTRA_EMAKEFLAGS="${EXTRA_EMAKEFLAGS} --emake-logfile=misc/emake.dlog"
		;;
	*CYGWIN*)
		# Max available linux agents are 48 and 72 windows agents
		export multi_agents=48
		export one_agent=1
		export num_agents=${multi_agents}
		EXTRA_EMAKEFLAGS="--emake-maxagents=${num_agents}"
		#Override if we need a different CM on linux side
		#export EMAKE_CM=wc-sjca-e001.sj.broadcom.com
		# Emake history archive
		ECHIST_DIR=Z:/projects/hnd_swbuild/build_window/PRESERVED/ECHISTORY
		export EMAKE_ROOT=`cygpath -m -p ${PWD}`
		# Windows resource on the cluster
		export HNDMAKE_RESOURCE="win"
		export HNDMAKE_CLASS="win"

		# PATH is set in windows server already
		# export PATH=/cygdrive/c/tools/ECloud/i686_win32/bin:${PATH}
		EXTRA_EMAKEFLAGS="${EXTRA_EMAKEFLAGS} --emake-emulation=cygwin"

		# Visual Studio plugin variables
		# Parallelize Microsoft PDB (debug file) generation steps
		export ECADDIN_MAX_PDB_FILES=48
		# Uncomment to keep generated makefiles from VS projects
		# export ECADDIN_DONT_RM_TMP_MAKEFILES=1
		export ECADDIN_DONT_ALLOW_PCH_AND_PDB=true
		export ECADDIN_USE_WINDOWS_TEMP=true
		set -o igncr
		export SHELLOPTS
		;;
esac

which emake
emake --version

# Save command line args in ARGS for any subsequent use
ARGS="$*"

# Fetch release brand specific env vars and export them to env
for arg in ${ARGS}
do
	case $arg in
		TAG=*) export $arg;;
		BRAND=*) export $arg;;
	esac
done

# Temporary workaround for precommit only (to derive TAG vis svn info)
if echo $CURUSER | egrep -q "$PRECOUSER|$PRECOUSER_DBG"; then
   echo "DBG: Found TAG=$TAG"
   export TAG=$(svn --non-interactive info 2> $NULL | grep "^URL:" | awk -F/ '{print $NF}')
   echo "DBG: Derived TAG=$TAG"
fi

# For tag or release builds from build user, do additional processing
# Generic settings for all E.C platforms. These are not mandatory for
# generic jobs, but they set different emake cmd line flags which can
# help central release builds

if [ "${VALID_ECUSER}" == "1" ] && [ "${TAG}" != "" -o "${BRAND}" != "" ]; then

	MISC_DIR=misc
	[ -d "${MISC_DIR}" ] || mkdir -pv ${MISC_DIR}

	RELDIR=${TAG:-NIGHTLY}

	# Derive ECHISTORY folder name based on the given tag
	# For _REL_, derive parent branch or twig name
	case "${RELDIR}" in
		*_BRANCH_*|*_TWIG_*|*_REL_*)
			TAGPREFIX=$(echo ${RELDIR} | awk -F_ '{print $1}')
			TAGTYPE=$(echo ${RELDIR}   | awk -F_ '{print $2}')
			MAJNUM=$(echo ${RELDIR}    | awk -F_ '{print $3}')
			MINNUM=$(echo ${RELDIR}    | awk -F_ '{print $4}')
			BLDNUM=$(echo ${RELDIR}    | awk -F_ '{print $5}')
			ITERNUM=$(echo ${RELDIR}   | awk -F_ '{print $6}')
			if [ "${ITERNUM}" == "" ]; then
				BRANCH=${TAGPREFIX}_${TAGTYPE}_${MAJNUM}_${MINNUM}
				BRANCH=$(echo "${BRANCH}" | sed -e "s/REL/BRANCH/")
			else
				BRANCH=${TAGPREFIX}_${TAGTYPE}_${MAJNUM}_${MINNUM}_${BLDNUM}
				BRANCH=$(echo "${BRANCH}" | sed -e "s/REL/TWIG/")
			fi
			PARENT_BRANCH=${TAGPREFIX}_${TAGTYPE}_${MAJNUM}_${MINNUM}
			PARENT_BRANCH=$(echo "${PARENT_BRANCH}" | sed -e "s/REL/BRANCH/" -e "s/TWIG/BRANCH/")
			;;
		*NIGHTLY*|*TOT*|*TRUNK*|*trunk*)
			BRANCH=NIGHTLY
			;;
		*)
			BRANCH=UNKNOWN
			;;
	esac

	BRANCH=${BRANCH:-"MISC"}

	ECHIST_PARENT_DIR="${ECHIST_DIR}/${PARENT_BRANCH}"
	ECHIST_DIR="${ECHIST_DIR}/${BRANCH}"

	if [ ! -f "${ECHIST_FILE}" ]; then

		# If custom history file isn't specified. Pick default one
		ECHIST_FILE="${ECHIST_DIR}/emake.${BRAND}.data"

		# History file is set to be stored locally first
		if [ ! -d "${ECHIST_DIR}" ]; then
			mkdir -pv ${ECHIST_DIR}
			chmod ugo+w ${ECHIST_DIR}
			echo "Auto-generated for $RELDIR on `date` for $RELDIR" > \
				${ECHIST_DIR}/README.txt
		fi

		# If custom history file isn't available, try to pick parent
		# branch's history file (if one available)
		if [ ! -f "${ECHIST_FILE}" ]; then
			# If network dir is available, reset ECHIST_FILE
			# For twig builds, any previous parent build provides history
			# to speed up twig builds
			ECHIST_PARENT_FILE="${ECHIST_PARENT_DIR}/emake.${BRAND}.data"
			if [ -f "${ECHIST_PARENT_FILE}" ]; then
				echo -e "\nWARN: Previous history for $BRAND on $TAG not found\n";
				echo -e "\nINFO: Copying history from parent $ECHIST_PARENT_FILE\n"
				cp -pv ${ECHIST_PARENT_FILE} ${ECHIST_FILE}
				echo "INFO: Copied history from ${ECHIST_PARENT_FILE}" >> \
				${ECHIST_DIR}/README.txt
			else
				if touch ${ECHIST_FILE}; then
					chmod ug+w ${ECHIST_FILE}
					echo -e "\nWARN: E.C history file ${ECHIST_FILE} newly created\n"
					echo -e "\nWARN: This build may take longer to finish than usual\n"
				else
					echo -e "\nWARN: E.C history file ${ECHIST_FILE} couldn't be created\n"
					echo -e "\nWARN: This build may take longer to finish than usual\n"
					ECHIST_FILE="misc/temp_emake.${BRAND}.data"
				fi
			fi # ECHIST_PARENT_FILE
		fi # ECHIST_DIR
	fi # ECHIST_FILE

	# Now construct EMAKE flags iteratively
	EMAKEFLAGS="${EMAKEFLAGS} --emake-class=${HNDMAKE_CLASS}"
	EMAKEFLAGS="${EMAKEFLAGS} --emake-resource=${HNDMAKE_RESOURCE}"
	EMAKEFLAGS="${EMAKEFLAGS} ${EXTRA_EMAKEFLAGS}"
	EMAKEFLAGS="${EMAKEFLAGS} --emake-historyfile=${ECHIST_FILE}"
	EMAKEFLAGS="${EMAKEFLAGS} --emake-collapse=0"
	# Disable annotation upload for preco and CI build sessions by default
	if echo $CURUSER | egrep -q "$PRECOUSER|$PRECOUSER_DBG|$CIUSER"; then
		if [ "${EMAKE_ANNOTATION_UPLOAD}" != "true" ]; then
			EMAKEFLAGS="${EMAKEFLAGS} --emake-annoupload=0"
		fi # EMAKE_ANNOTATION_UPLOAD
	fi # CURUSER
	# EMAKEFLAGS="${EMAKEFLAGS} --emake-logfile=misc/emake.${BRAND}.dlog"

	if [ "${EMAKE_ANNOTATION_LEVEL}" == "" ]; then
		# Basic logging for faster builds
		EMAKE_ANNOTATION_LEVEL=basic
		# WARN: Don't set ANNOTAION LEVEL to higher than basic
		# WARN: as disk fills up extremely fast. Following is
		# WARN: meant to be set only for one-off build/debug tests
		# EMAKE_ANNOTATION_LEVEL="basic,file,lookup,env,history,registry,waiting"
	fi

	EMAKEFLAGS="${EMAKEFLAGS} --emake-annofile=${MISC_DIR}/,emake_${BRAND}_anno.xml"
	EMAKEFLAGS="${EMAKEFLAGS} --emake-annodetail=${EMAKE_ANNOTATION_LEVEL}"
	# EMAKEFLAGS="${EMAKEFLAGS} --emake-debug=ja"
	# EMAKEFLAGS="${EMAKEFLAGS} --emake-debug=g"
	# EMAKEFLAGS="${EMAKEFLAGS} --emake-idle-time=3600"

	# For privately firmware builds, label them as private/prebuild
	case $EMAKE_ROOT in
		*PREBUILD_DONGLE*|TEMP/prebuild*)
		  BUILD_LABEL="${BUILD_LABEL}PVT"
		  PARENT_BRAND=$(echo ${EMAKE_ROOT} | awk -F/ '{print $(NF-3)}')
		  PARENT_LABEL="for_${PARENT_BRAND}"
		  ;;
	esac

	EMAKEFLAGS="${EMAKEFLAGS} --emake-build-label=${TAG:-TOT}${BUILD_LABEL:+_${BUILD_LABEL}}_${BRAND}${PARENT_LABEL:+_${PARENT_LABEL}}"

	export EMAKEFLAGS

fi # BLDUSER && (TAG || BRANCH)

# Finally call out emake to launch the build
echo "==== Emake Build Flags ===="
echo "EMAKE_ROOT=$EMAKE_ROOT"
echo "EMAKE_CM=$EMAKE_CM"
env | sort | egrep "^EC|SHELL"
echo "emake ${EMAKEFLAGS} $ARGS"
echo "==========================="

emake ${EMAKEFLAGS} $ARGS
make_exitcode=$?

if [ "$make_exitcode" == "0" ]; then
   echo "INFO: Emake Exit Code; $make_exitcode"
else
   echo "ERROR: Emake Exit Code; $make_exitcode"
fi

# Finally exit with emake exit code
exit $make_exitcode
