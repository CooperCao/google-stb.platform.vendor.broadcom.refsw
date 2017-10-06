#!/bin/bash
#
# Wrapper script to prune aged local directories and any stray temporary
# files on windows build servers. This is run on all build launch systems.
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#
# SVN: $HeadURL$
#

# Prune builds older than this period on local drives to reclaim space

# Number of days local builds will remain on windows build servers
# before getting purged. This needs to be balanced with available
# local disk-space.
PRESERVEPERIOD=1

DRIVES="C D"
DIRS="build build_window"
SCRIPT="c:/tools/build/prune_windows.sh"
LOG="c:/temp/prune_windows.log"

PATH=/usr/bin:/bin:${PATH}

echo "START PRUNE_BUILDS: `date`" >> ${LOG}

# Search through each ${DRIVES} for ${DIRS} directories and prune aged folders
for drive in ${DRIVES}; do
    for dir in ${DIRS}; do
        if [ -d "${drive}:/${dir}" ]; then
           echo "${SCRIPT} -d ${drive}:/${dir} -x ${PRESERVEPERIOD} -l ${LOG} -v"
           ${SCRIPT} -d ${drive}:/${dir} -x ${PRESERVEPERIOD} -l ${LOG} -v
        fi
    done
done

# Remove any stray/empty directories in build or build_window
echo "Removing any stray/empty directories"
for drive in ${DRIVES}; do
    for dir in ${DIRS}; do
        if [ -d "${drive}:/${dir}" ]; then
           for subdir in `find ${drive}:/${dir} -maxdepth 1 -mindepth 1 -type d`
           do
               contents=`find ${subdir} -maxdepth 1 -mindepth 1`
               if [ "${contents}" == "" ]; then
                  rmdir -v ${subdir} >> ${LOG}
               fi
           done
        fi
    done
done

# Cleanup temporary files created by build process
find c:/temp/prune -type f -not -mtime -7 | xargs -t -l1 -r rm -f >> ${LOG}
find c:/temp       -type f -name "temp_NIGHTLY*.bat" -o   		\
                           -name "temp_*_BRANCH_*.bat" -o 		\
                           -name "temp_*_TWIG_*.bat" -o   		\
                           -name "temp_build_windows_brand_*.sh" -o     \
                           -name "temp_*_REL_*.bat"       		\
                           -not -mtime -7 | 				\
			xargs -t -l1 -r rm -f >> ${LOG}

echo "END PRUNE_BUILDS  : `date`" >> ${LOG}
