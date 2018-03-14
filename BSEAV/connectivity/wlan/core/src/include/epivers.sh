#! /bin/bash
#
# Create the epivers.h file from epivers.h.in
#
# Epivers.h version support svn/sparse/gclient workspaces
#
#
#
# Version generation works off of svn property HeadURL, if
# not set it keys its versions from current svn workspace or
# via .gclient_info deps contents
#
# GetCompVer.py return value and action needed
#    i. trunk => use current date as version string
#   ii. local => use SVNURL expanded by HeadURL keyword
#  iii. <tag> => use it as as is
#                (some components can override and say give me native ver)
#   iv. empty =>
#             a) If TAG is specified use it
#             a) If no TAG is specified use date
#
# Contact: Prakash Dhavali
# Contact: hnd-software-scm-list
#

# If the version header file already exists, increment its build number.
# Otherwise, create a new file.
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
# <<Broadcom-WL-IPTag/Proprietary:>>
#
# $Id$

if [ -f epivers.h ]; then

	# If REUSE_VERSION is set, epivers iteration is not incremented
	# This can be used precommit and continuous integration projects
	if [ -n "$REUSE_VERSION" ]; then
		echo "Previous epivers.h exists. Skipping version increment"
		exit 0
	fi

	build=$(grep EPI_BUILD_NUMBER epivers.h | sed -e "s,.*BUILD_NUMBER[ 	]*,,")
	build=$(expr ${build} + 1)
	echo build=${build}
	sed -e "s,.*_BUILD_NUMBER.*,#define EPI_BUILD_NUMBER	${build}," \
		< epivers.h > epivers.h.new
	cp -p epivers.h epivers.h.prev
	mv epivers.h.new epivers.h
	exit 0

else # epivers.h doesn't exist

	SVNCMD=${SVNCMD:-"svn --non-interactive"}
	SRCBASE=${SRCBASE:-..}
	NULL=/dev/null
	[ -z "$VERBOSE" ] || NULL=/dev/stderr

	# Check for the in file, if not there we're in the wrong directory
	if [ ! -f epivers.h.in ]; then
		echo "ERROR: No epivers.h.in found"
		exit 1
	fi

	# Following SVNURL should be expanded on checkout
	SVNURL='$HeadURL$'

	# .gclient_info is created by gclient checkout/sync steps
	# and contains "DEPS='<deps-url1> <deps-url2> ..." entry
	GCLIENT_INFO=${GCLIENT_INFO:-${SRCBASE}/../.gclient_info}

	# In gclient, derive SVNURL from gclient_info file
	if [ -s "${GCLIENT_INFO}" ]; then
		source ${GCLIENT_INFO}
		if [ -z "$DEPS" ]; then
			echo "ERROR: DEPS entry missing in $GCLIENT_INFO"
			exit 1
		else
			for dep in $DEPS; do
				SVNURL=${SVNURL:-$dep}
				# Set SVNURL to first DEPS with /tags/ (if any)
				if [[ $dep == */tags/* ]]; then
					SVNURL=$dep
					echo "INFO: Found gclient DEPS: $SVNURL"
					break
				fi
			done
		fi
	elif [ -f "${GCLIENT_INFO}" ]; then
		echo "ERROR: $GCLIENT_INFO exists, but it is empty"
		exit 1
	fi

	# If SVNURL isn't expanded, extract it from svn info
	if echo "$SVNURL" | egrep -vq 'HeadURL.*epivers.sh.*|^[a-z+]+://.*/DEPS'; then
		[ -n "$VERBOSE" ] && \
			echo "DBG: SVN URL ($SVNURL) wasn't expanded. Getting it from svn info"
		SVNURL=$($SVNCMD info epivers.sh 2> $NULL | egrep "^URL:")
	fi

	if echo "${TAG}" | grep -q "_BRANCH_\|_TWIG_"; then
		branchtag=$TAG
	else
		branchtag=""
	fi

	# If this is a tagged build, use the tag to supply the numbers
	# Tag should be in the form
	#    <NAME>_REL_<MAJ>_<MINOR>
	# or
	#    <NAME>_REL_<MAJ>_<MINOR>_RC<RCNUM>
	# or
	#    <NAME>_REL_<MAJ>_<MINOR>_RC<RCNUM>_<INCREMENTAL>

	MERGERLOG=${SRCBASE}/../merger_sources.log
	GETCOMPVER=getcompver.py
	GETCOMPVER_NET=/projects/hnd_software/gallery/src/tools/build/$GETCOMPVER
	GETCOMPVER_NET_WIN=Z:${GETCOMPVER_NET}

	#
	# If there is a local copy GETCOMPVER use it ahead of network copy
	#
	if [ -s "$GETCOMPVER" ]; then
	        GETCOMPVER_PATH="$GETCOMPVER"
	elif [ -s "${SRCBASE}/../src/tools/build/$GETCOMPVER" ]; then
	        GETCOMPVER_PATH="${SRCBASE}/../src/tools/build/$GETCOMPVER"
	elif [ -s "$GETCOMPVER_NET" ]; then
	        GETCOMPVER_PATH="$GETCOMPVER_NET"
	elif [ -s "$GETCOMPVER_NET_WIN" ]; then
	        GETCOMPVER_PATH="$GETCOMPVER_NET_WIN"
	fi

	#
	# If $GETCOMPVER isn't found, fetch it from SVN
	# (this should be very rare)
	#
	if [ ! -s "$GETCOMPVER_PATH" ]; then
		[ -n "$VERBOSE" ] && \
			echo "DBG: Fetching $GETCOMPVER from trunk"

		$SVNCMD export -q \
			^/proj/trunk/src/tools/build/${GETCOMPVER} \
			${GETCOMPVER} 2> $NULL

		GETCOMPVER_PATH=$GETCOMPVER
	fi

	# Now get tag for src/include from automerger log
	[ -n "$VERBOSE" ] && \
		echo "DBG: python $GETCOMPVER_PATH $MERGERLOG src/include"

	COMPTAG=$(python $GETCOMPVER_PATH $MERGERLOG src/include 2> $NULL | sed -e 's/[[:space:]]*//g')

	echo "DBG: Component Tag String Derived = $COMPTAG"

	# Process COMPTAG values
	# Rule:
	# If trunk is returned, use date as component tag
	# If LOCAL_COMPONENT is returned, use SVN URL to get native tag
	# If component is returned or empty, assign it to SVNTAG
	# GetCompVer.py return value and action needed
	#    i. trunk => use current date as version string
	#   ii. local => use SVNURL expanded by HeadURL keyword
	#  iii. <tag> => use it as as is
	#   iv. empty =>
	#             a) If TAG is specified use it
	#             a) If no TAG is specified use SVNURL from HeadURL

	SVNURL_VER=false

	if [ "$COMPTAG" == "" ]; then
		SVNURL_VER=true
	elif [ "$COMPTAG" == "LOCAL_COMPONENT" ]; then
		SVNURL_VER=true
	elif [ "$COMPTAG" == "trunk" ]; then
		SVNTAG=$(date '+TRUNKCOMP_REL_%Y_%m_%d')
	else
		SVNTAG=$COMPTAG
	fi

	# Given SVNURL path conventions or naming conventions, derive SVNTAG
	# TO-DO: SVNTAG derivation logic can move to a central common API
	# TO-DO: ${SRCBASE}/tools/build/svnurl2tag.sh
	if [ "$SVNURL_VER" == "true" ]; then
		case "${SVNURL}" in
			*_BRANCH_*)
				SVNTAG=$(echo $SVNURL | tr '/' '\n' | awk '/_BRANCH_/{printf "%s",$1}')
				;;
			*_TWIG_*)
				SVNTAG=$(echo $SVNURL | tr '/' '\n' | awk '/_TWIG_/{printf "%s",$1}')
				;;
			*_REL_*)
				SVNTAG=$(echo $SVNURL | tr '/' '\n' | awk '/_REL_/{printf "%s",$1}')
				;;
			*/branches/*)
				SVNTAG=${SVNURL#*/branches/}
				SVNTAG=${SVNTAG%%/*}
				;;
			*/proj/tags/*|*/deps/tags/*)
				SVNTAG=${SVNURL#*/tags/*/}
				SVNTAG=${SVNTAG%%/*}
				;;
			*/trunk/*)
				SVNTAG=$(date '+TRUNKURL_REL_%Y_%m_%d')
				;;
			*)
				SVNTAG=$(date '+OTHER_REL_%Y_%m_%d')
				;;
		esac
		echo "DBG: Native Tag String Derived from URL: $SVNTAG"
	else
		echo "DBG: Native Tag String Derived: $SVNTAG"
	fi

    # Important to update/commit EXTERNAL_TAG before an external release. Internally git will find tags.
	EXTERNAL_TAG="STB7271_REL_15_20_49_4"
	external_build=0
	# Finally get version control revision number of <SRCBASE> (if any)
	#vc_version_num=$($SVNCMD info ${SRCBASE} 2> $NULL | awk -F': ' '/^Last Changed Rev: /{printf "%s", $2}')
	#vc_version_num=$(git log -1 --format=%H `git rev-parse --show-toplevel`/BSEAV/connectivity/wlan/core)
	vc_version_num=$(git log -1 --format=%H)
	# TESTING driver branch
	#vc_version_num=""
	#vc_version_num="b2d1ee322666c30da3551853143ca826de55b6e3"
	# driver rel
	#vc_version_num="8ab2c134651189f083faaba9038d79cc858ccdd4"
	# nothing no tags
	#vc_version_num="a3e4dcca461b7fa283247abbd1151f63ba46ba6e"
	echo "git hash: $vc_version_num"
	if [ -z "$vc_version_num" ]; then
		vc_version_num=$AVG_COMMIT_ID
		shared_server=gitbsesw\@stband-git-1.and.broadcom.com:/refsw/shared.git
		echo "No git repo functionality, attempting to use AVG_COMMIT_ID($vc_version_num)"
		if [ -z "$vc_version_num" ]; then
			echo "No AVG_COMMIT_ID, assuming External build (outside Avenger or Devel Git Repo)"
			external_build=1
		fi
	else
		# git repo commands works
		#use local mirrored copy
		shared_server=shared
		#vc_date=$(git log -1 --format=%cd `git rev-parse --show-toplevel`/BSEAV/connectivity/wlan/core)
		vc_date=$(git log -1 --format=%cd)
		vc_remote_branch=$(git rev-parse --abbrev-ref --symbolic-full-name @{u})
		#vc_short_ver_date=$(git log -1 --abbrev-commit --pretty=oneline --format=%h:%cd --date=short `git rev-parse --show-toplevel`/BSEAV/connectivity/wlan/core)
		vc_short_ver_date=$(git log -1 --abbrev-commit --pretty=oneline --format=%h:%cd --date=short)
	fi

	if [ -z "$vc_tag" ] && [ ! -z "$vc_version_num" ] ; then
		echo "trying to look up tag through git published tags"

		output=$(git ls-remote --tags $shared_server | grep "$vc_version_num" | awk -F'/' '{print $NF}' | awk -F'^' '{print $1}')
		IFS=$'\n' out=(${output})
		unset IFS
		echo "combined ${out[0]},${out[1]},${out[2]}"

		for line in "${out[@]}";
		do
	   		case "${line}" in
				*_BRANCH_*)
					echo "_BRANCH_ : $line"
					vc_tag=$line
					break 2
					;;
				*_TWIG_*)
					echo "_TWIG_ : $line"
					vc_tag=$line
					break 2
					;;
				*_REL_*)
					echo "_REL_ : $line"
					vc_tag=$line
					break 2
					;;
			esac
		done

        echo "first tag found: $vc_tag"
	fi

	if [ -z "$vc_tag" ] || [ -z "$vc_version_num" ]; then
            if [ ${external_build} -eq 1 ]; then
                vc_tag="Built external to git:default to most recent committed tag(${EXTERNAL_TAG})"
                vc_wl_ver="$vc_short_ver_date:NoTag"
                echo "Built external to git:default to most recent committed tag(${EXTERNAL_TAG})"
                TAG=${EXTERNAL_TAG}
            else
                vc_tag="No Published Tag for this Hash.Default to recent tag(${EXTERNAL_TAG})"
                vc_wl_ver="$vc_short_ver_date:NoTag"
                echo "No Tag found: default to most recent committed tag(${EXTERNAL_TAG})"
                TAG=${EXTERNAL_TAG}
            fi
	else
        vc_wl_ver="$vc_short_ver_date:$vc_tag"
        TAG=${vc_tag}
	fi
    echo "TAG0: $TAG"
    # Normalize the branch name portion to "D11" in case it has underscores in it
	# The pattern substitutions match the tag num up to the BRANCH/TWIG/REL,
	# (delimited by '_'), and replaces with a consistent 2 words:
	#              AARDVARK_TEST_BRANCH_6_30
	#              ^^^^^^^^^^^^^^^^^^^^^      <- matched
	# The replacement discards the first part of the string and replaces with "D11_"
	#                        D11_BRANCH_6_30
	#                        ^^^^^^^^^^^      <- replaced

	TAG=${TAG/*_REL_/D11_REL_}
	TAG=${TAG/*_TWIG_/D11_TWIG_}
	TAG=${TAG/*_BRANCH_/D11_BRANCH_}
	TAG=${TAG/)/}
 	echo "TAG1: $TAG"
    # Split the tag into an array on underbar or whitespace boundaries.
	IFS="_	     " tag=(${TAG})
	unset IFS

	tagged=1
	if [ ${#tag[*]} -eq 0 ]; then
	   tag=($(date '+TOT REL %Y %m %d 0 %y'));
	   # reconstruct a TAG from the date
	   TAG=${tag[0]}_${tag[1]}_${tag[2]}_${tag[3]}_${tag[4]}_${tag[5]}
	   tagged=0
	   echo "TAG2: $TAG"
	fi
	maj=${EPI_MAJOR_VERSION:-${tag[2]:-0}}
	min=${EPI_MINOR_VERSION:-${tag[3]:-0}}
	rcnum=${EPI_RC_NUMBER:-${tag[4]:-0}}

	echo "combined ${tag[0]}_${tag[1]}_${tag[2]}_${tag[3]}_${tag[4]}_${tag[5]}"
	
	# Allow environment variable to override values.
	# Missing values default to 0
	#
	maj=${EPI_MAJOR_VERSION:-${tag[2]:-0}}
	min=${EPI_MINOR_VERSION:-${tag[3]:-0}}
	rcnum=${EPI_RC_NUMBER:-${tag[4]:-0}}

	# If increment field is 0, set it to date suffix if on TOB
	if [ -n "$branchtag" ]; then
		[ "${tag[5]:-0}" -eq 0 ] && echo "Using date suffix for incr"
		today=${EPI_DATE_STR:-$(date '+%Y%m%d')}
		incremental=${EPI_INCREMENTAL_NUMBER:-${tag[5]:-${today:-0}}}
	else
		incremental=${EPI_INCREMENTAL_NUMBER:-${tag[5]:-0}}
	fi
	origincr=${EPI_INCREMENTAL_NUMBER:-${tag[5]:-0}}
	build=${EPI_BUILD_NUMBER:-0}

	# Strip 'RC' from front of rcnum if present
	rcnum=${rcnum/#RC/}

	# strip leading zero off the number (otherwise they look like octal)
	maj=${maj/#0/}
	min=${min/#0/}
	rcnum=${rcnum/#0/}
	incremental=${incremental/#0/}
	origincr=${origincr/#0/}
	build=${build/#0/}

	# some numbers may now be null.  replace with with zero.
	maj=${maj:-0}
	min=${min:-0}

	rcnum=${rcnum:-0}
	incremental=${incremental:-0}
	origincr=${origincr:-0}
	build=${build:-0}

	if [ -n "$EPI_VERSION_NUM" ]; then
	    vernum=$EPI_VERSION_NUM
	elif [ ${tagged} -eq 1 ]; then
	    # vernum is 32chars max
	    vernum=$(printf "0x%02x%02x%02x%02x" ${maj} ${min} ${rcnum} ${origincr})
	else
	    vernum=$(printf "0x00%02x%02x%02x" ${tag[7]} ${min} ${rcnum})
	fi

	# make sure the size of vernum is under 32 bits.
	# Otherwise, truncate. The string will keep full information.
	vernum=${vernum:0:10}
	epi_ver_dev="${maj}.${min}.${rcnum}"
    
	# OK, go do it
	echo "maj=${maj}, min=${min}, rc=${rcnum}, inc=${incremental}, build=${build}"

	sed \
		-e "s;@EPI_MAJOR_VERSION@;${maj};" \
		-e "s;@EPI_MINOR_VERSION@;${min};" \
		-e "s;@EPI_RC_NUMBER@;${rcnum};" \
		-e "s;@EPI_INCREMENTAL_NUMBER@;${incremental};" \
		-e "s;@EPI_BUILD_NUMBER@;${build};" \
		-e "s;@EPI_VERSION@;${maj}, ${min}, ${rcnum}, ${incremental};" \
		-e "s;@EPI_VERSION_STR@;${tag_ver_str};" \
		-e "s;@EPI_VERSION_TYPE@;${tagtype};" \
		-e "s;@VERSION_TYPE@;${tagtype};" \
		-e "s;@EPI_VERSION_NUM@;${vernum};" \
		-e "s;@EPI_VERSION_DEV@;${epi_ver_dev};" \
		-e "s;@VC_VERSION_NUM@;${vc_version_num};" \
		-e "s;@VC_DATE@;${vc_date};" \
		-e "s;@VC_TAG@;${vc_tag};" \
		-e "s;@VC_REMOTE_BRANCH@;${vc_remote_branch};" \
		-e "s;@VC_WL_VER@;${vc_wl_ver};" \
		< epivers.h.in > epivers.h

	# In shared workspaces across different platforms, ensure that
	# windows generated file is made platform neutral without CRLF
	if uname -s | egrep -i -q "cygwin"; then
	   dos2unix epivers.h > $NULL 2>&1
	fi
fi # epivers.h
