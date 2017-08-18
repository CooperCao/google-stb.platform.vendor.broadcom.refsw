#!/bin/bash
#
# Script to produce recurring coverity baseline builds
# for precommit infra. Baseline build per target are
# used in pre-commit infra to compare developer's
# precommit instances and see if there are any new
# defects introduced by merging defects with baseline.
#
# Usage:
#  This script relies and need to match list of
#  coverity targets in src/makefiles/preco_coverity.mk
#  Precommit infra will tell what coverity targets it
#  needs to build and both these script and above .mk
#  will build that target, update the coverity baseline
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#

# This script doesn't need any cmd line options right now
# If it needs one, it will be updated later on.

# DBG: Uncomment to see what will be executed
# NOOP=echo

export PATH=/home/hwnbuild/bin:/tools/bin:/projects/hnd/tools/linux/bin:${PATH}

SVN=/tools/bin/svn
GMAKE=/tools/bin/gmake

# Number of days to preserve baseline logs
PRECO_BASELINE_CTIME=7

# List of gallery dirs to keep preco baseline checkouts
PRECO_GALLERY=/projects/hnd_swbuild_ext6_scratch/build_admin_ext6_scratch/preco_coverity_gallery

# Main make rules to produce precommit runs and baselines
PRECO_COV_MAKEFILE=/projects/hnd_software/gallery/src/makefiles/preco_coverity_rules.mk

# Migrate defects from production to reference baselines
RECUR_SYNCUP_DEFECTS="/projects/hnd/tools/linux/Perl/bin/perl /projects/hnd_software/gallery/src/tools/coverity/webapi/perl/scripts/migrate-stream-defects.pl --config /projects/hnd_software/gallery/src/tools/coverity/webapi/config/coverity_webapi_config.xml --update"

TIMESTAMP=$(date '+%Y%m%d-%H%M%S')
TODAY=$(date '+%Y-%m-%d')

# Given a branch, target produce a new baseline
# log goes in gallery-workspace/logs/preco_<stream>.log
function generate_coverity_baseline {
	branch=$1
	workspace="${PRECO_GALLERY}/${branch}/$2"
	target=$3
	target_opts="$4"

	recstream=preco_${branch}__${target/\//_}
	log=${workspace}/logs/${recstream}_${TIMESTAMP}.log
	stat=${workspace}/logs/${recstream}_${TODAY}.stat
	[ -d "${workspace}/logs" ] || mkdir -pv "${workspace}/logs/"

	rm -f $(find ${workspace}/logs/ -maxdepth 1 -mindepth 1 -not -mtime -${PRECO_BASELINE_CTIME})


	if [ -d "${workspace}" ]; then

		host=$(hostname)
		echo "INFO: ====== `date` ======"
		echo "INFO: Start Build: $branch $target"
		echo "INFO: $workspace"
		echo "INFO: on $host"
		echo "INFO:"
		(
			echo "INFO: ==== Start Build: $branch $target ====="
			echo "INFO: workspace $workspace"
			echo "INFO: on $host"
			echo "INFO:"

			cd ${workspace}

			env | sort                                     > ,env.log
			echo "SVN ver : `${SVN} --version -q`"        >> ,env.log
			echo "Make ver: `${GMAKE} -v | grep -i make`" >> ,env.log

			date "+[%Y/%d/%m %H:%M:%S] start: svn update $recstream"
			svnstat=$($NOOP ${SVN} update 2>&1)
			svnrc=$?
			echo "$svnstat"
			echo "svn update exit code: $svnrc"
			date "+[%Y/%d/%m %H:%M:%S] end:   svn update $recstream"

			echo "INFO: Update reference baselines from production"
			prodstream=$(echo $recstream | sed -e 's/preco_//g')
			echo $RECUR_SYNCUP_DEFECTS \
				--stream1 $prodstream \
				--stream2 $recstream
			$NOOP $RECUR_SYNCUP_DEFECTS \
				--stream1 $prodstream \
				--stream2 $recstream
			syncrc=$?

			echo "sync defects exit code: $syncrc"

			if echo $svnstat | grep -q "Updated to revision"; then
				echo "INFO: SVN changes found, proceeding with new baseline creation"
				#echo "INFO: SVN changes found, checking for relevant changes"
			        #if echo $svnstat | egrep -q -v -i "\.c|\.h|\.s|\.mk|akefile"; then
				#	echo "INFO: No new relevant SVN changes found, skip baseline"
	 			#	return 0
			        #fi
			else
				# For nightly build reporting, there has to be
				# at-least one preco target on a given day
				if [ -f "$stat" ]; then
					echo "INFO: No new SVN changes found, skip baseline"
	 				return 0
				fi
			fi

			date "+[%Y/%d/%m %H:%M:%S] start: coverity build $recstream"
			date "+[%Y/%d/%m %H:%M:%S] start: coverity build $recstream" > $stat
			echo $GMAKE -f ${PRECO_COV_MAKEFILE} -C src/makefiles \
				PRECO_BRANCH=${branch} \
				COVSTREAM=${recstream} \
				COVTMPDIR=/tmp/precov-recur/${recstream}-$$ \
				PRECO_COVERITY_BASELINE_BUILD=true \
				${target_opts} \
				${target};
			$NOOP $GMAKE -f ${PRECO_COV_MAKEFILE} -C src/makefiles \
				PRECO_BRANCH=${branch} \
				COVSTREAM=${recstream} \
				COVTMPDIR=/tmp/precov-recur/${recstream}-$$ \
				PRECO_COVERITY_BASELINE_BUILD=true \
				${target_opts} \
				${target};
			makerc=$?
			if [ "$makerc" != "0" ]; then
				echo "ERROR: Build failed for $recstream"
			fi
			date "+[%Y/%d/%m %H:%M:%S] end:  coverity build $recstream"
			date "+[%Y/%d/%m %H:%M:%S] end:  coverity build $recstream" >> $stat

			echo "INFO: ==== End Build: $branch $target ====="
		) > $log 2>&1 &

	else

		(
			echo "ERROR: $BRANCH workspace missing at:"
			echo "ERROR: $workspace"
		) > $log 2>&1 &
		# exit 1

	fi
}

# Following baseline creation calls all run in parallel
# TO-DO: These can be submitted to LSF farm in parallel
# TO-DO: It is not done intentionally, as we may add some
# TO-DO: windows targets and those need different launch

# FALCON Coverity targets
generate_coverity_baseline                    \
	"FALCON_BRANCH_5_90"                  \
	"hndrte"                              \
	"4330b2-roml/sdio-g"                  \
	""

# RUBY Coverity targets
# NOTE: The target options here need to be matched across nightly runs
# NOTE: in src/tools/release/coverity-rules.mk and precommit baseline runs
generate_coverity_baseline                    \
	"RUBY_BRANCH_6_20"                    \
	"wl-src"                              \
	"debug-apdef-stadef"                  \
	"LINUXVER=2.6.29.4-167.fc11.i686.PAE GCCVER=4.3.0-32 ARCH=i386 32ON64=1 CROSS_COMPILE=/tools/gcc/4.3.0/i686-2.6/bin/"

# TRUNK Coverity targets
# NOTE: The target options here need to be matched across nightly runs
# NOTE: in src/tools/release/coverity-rules.mk and precommit baseline runs
generate_coverity_baseline                    \
	"TRUNK"                               \
	"wl-src"                              \
	"debug-apdef-stadef-high"             \
	"LINUXVER=2.6.29.4-167.fc11.i686.PAE GCCVER=4.3.0-32 ARCH=i386 32ON64=1 CROSS_COMPILE=/tools/gcc/4.3.0/i686-2.6/bin/"
