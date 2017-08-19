#!/usr/bin/python
#===============================================================================
#
# usage: find_wlan_build.py [-h] [-w WORKQ_BASE_DIR] [-b BUILD_BASE_DIR]
#                           [-r RELEASE] [-l]
#                           branch brand
#
# Find the latest wlan build and place it in the work-queue.
#
# positional arguments:
#   branch                branch or twig name
#   brand                 brand name
#
# optional arguments:
#   -h, --help            show this help message and exit
#   -w WORKQ_BASE_DIR, --workq-base-dir WORKQ_BASE_DIR
#                         directory path to workqueues
#   -b BUILD_BASE_DIR, --build_base_dir BUILD_BASE_DIR
#                         directory path to builds
#   -r RELEASE, --release RELEASE
#                         release name
#   -l, --log-to-console  write log to console
#
# Example:
#   find_wlan_build.py --log-to-console --build_base_dir /projects/hnd_video7/USERS/TAGS/build_linux --release=STB7271_REL_15_10_38 STB7271_BRANCH_15_10 linux-external-stbsoc
#
# Note: This script was tested with Python 2.7.5.
#
#===============================================================================

import os
import sys
import argparse
import re
import logging
import glob
import tarfile
import filecmp
import shutil
import datetime
import pwd
import socket
import subprocess

#
# program constants
#

WORKQ_BASE_DIR = '/projects/hnd_video7/svn_git_bridge/workqueues'  ## can be overridden via cmdline arg -w
BUILD_BASE_DIR = '/projects/hnd_video7/USERS/NIGHTLY/build_linux'  ## can be overridden via cmdline arg -b
## /projects/hnd_video7/USERS/TAGS/build_linux
## /projects/hnd_video7/USERS/NIGHTLY/build_linux
## /projects/hnd/swbuild/build_linux

STATE_LATEST_BUILD_FROM_SVN_VARNAME = 'STATE_LATEST_BUILD_FROM_SVN'
STATE_SUSPENDED_VARNAME = 'STATE_SUSPENDED'
SRC_TARBALL_FILENAME = 'linux-src-pkg-wl.tar.gz'
INFO_FILENAME = 'info.txt'
INFO_VERSION = '1'
LOG_LEVEL = logging.INFO
logger = logging.getLogger()

#===============================================================================
# Take a build date string and return a normalized representation (yyyy.mm.dd.nn)
# that can be compared.
# For examples:
#     2016.4.10.0 -> 2016.04.10.000
#     2012.1.1.9  -> 2012.01.01.009
#===============================================================================
def normaize_build_date(build_date):
    year, month, day, n = build_date.split('.')
    return '{0:04}.{1:02}.{2:02}.{3:03}'.format(int(year), int(month), int(day), int(n))

#===============================================================================
# Return a negative integer if left < right, zero if left == right,
# a positive integer if left > right.
#===============================================================================
def compare_build_date(left, right):
    string_l = normaize_build_date(left)
    string_r = normaize_build_date(right)
    if string_l < string_r:
        return -1
    elif string_l > string_r:
        return 1
    else:
        return 0

def is_var_defined(dir, var):
    if var in os.listdir(dir):
        return True
    else:
        return False

def get_var(dir, var):
    p = re.compile(r'^{0}##(.*)'.format(var))
    dir_listing = os.listdir(dir)
    for item in dir_listing:
        m = p.search(item)
        if m and m.group(1):
            return m.group(1)

def set_var(dir, var, value):
    var_val_path = os.path.join(dir, '{0}##{1}'.format(var, value))
    p = re.compile(r'^{0}##(.*)'.format(var))
    dir_listing = os.listdir(dir)

    for item in dir_listing:
        m = p.search(item)
        if m and m.group(1):
            os.rename(os.path.join(dir, item), var_val_path)

    if not os.path.exists(var_val_path):
        open(var_val_path, 'w').close()

def unset_var(dir, var):
    p = re.compile(r'^{0}##(.*)'.format(var))
    dir_listing = os.listdir(dir)
    for item in dir_listing:
        m = p.search(item)
        if m:
            os.remove(os.path.join(dir, item))

def get_var_path(dir, var, value=None):
    if value:
        return os.path.join(dir, '{0}##{1}'.format(var, value))

    p = re.compile(r'^{0}##(.*)'.format(var))
    dir_listing = os.listdir(dir)
    for item in dir_listing:
        m = p.search(item)
        if m:
            return os.path.join(dir, item)

def get_latest_build(dir):
    return get_var(dir, STATE_LATEST_BUILD_FROM_SVN_VARNAME)

def submit_build(dir, build, build_tarball_path, tag, branch, brand, remember_submitted_build):
    #
    # Enqueue a work-item
    # - create a unique temp dir
    # - copy tarball
    # - create info.txt
    # - chmod to allow full access
    # - rename temp dir to workitem_YYYYMMDD_HHMMSS.mmm (UTC)
    #
    ts = datetime.datetime.utcnow().strftime('%Y%m%d_%H%M%S.%f')[:-3]
    tempdir = os.path.join(dir, 'temp_' + ts)
    os.makedirs(tempdir)
    shutil.copy(build_tarball_path, os.path.join(tempdir, SRC_TARBALL_FILENAME))
    with open(os.path.join(tempdir, INFO_FILENAME), 'w') as f:
        f.write('info_version=%s\n' % INFO_VERSION)
        f.write('original_tarball_path=%s\n' % build_tarball_path)
        f.write('tag=%s\n' % tag)
	f.write('branch=%s\n' % branch)
        f.write('brand=%s\n' % brand)
        f.write('date=%s\n' % build)

    os.chmod(tempdir, 0777)
    for root, dirs, files in os.walk(tempdir):
        for mono in dirs:
            os.chmod(os.path.join(root, mono), 0777)
        for mono in files:
            os.chmod(os.path.join(root, mono), 0777)

    os.rename(os.path.join(dir, 'temp_' + ts), os.path.join(dir, 'workitem_' + ts))
    logger.info('Created %s', 'workitem_' + ts)

    # Remember the build submitted for future comparison
    if remember_submitted_build:
        shutil.copy(build_tarball_path, os.path.join(dir, SRC_TARBALL_FILENAME))
	os.chmod(os.path.join(dir, SRC_TARBALL_FILENAME), 0777)
        set_var(dir, STATE_LATEST_BUILD_FROM_SVN_VARNAME, build)
        build_info_path = get_var_path(dir, STATE_LATEST_BUILD_FROM_SVN_VARNAME, build)
        f = open(build_info_path, 'w')
        f.close()
    else:
        unset_var(dir, STATE_LATEST_BUILD_FROM_SVN_VARNAME)
        if os.path.exists(os.path.join(dir, SRC_TARBALL_FILENAME)):
            os.remove(os.path.join(dir, SRC_TARBALL_FILENAME))

#===============================================================================
# Return false if there was a build and it was older or equal this build;
# otherwise returns true.
#===============================================================================
def is_latest_build(dir, build):
    previous = get_latest_build(dir)
    if previous:     ## a previous build was submitted
        if compare_build_date(previous, build) >= 0:
            return False
    return True

#===============================================================================
# Compare two directories recursively. Files in each directory are
# assumed to be equal if their names and contents are equal.
#
# @param dir1: First directory path
# @param dir2: Second directory path
#
# @return: True if the directory trees are the same and
#     there were no errors while accessing the directories or files,
#     False otherwise.
#
# Note: This function basically does the equivalent of the following linux diff command:
#       diff -rq -x .mogrified -x wlc_clm_data.c.GEN -x epivers.h a b
#       We have to do this recursive dance because dircmp does a "shallow" compare which does
#       not compare file content.
#===============================================================================
def are_dir_trees_equal(dir1, dir2, ignore_list=[]):
    dirs_cmp = filecmp.dircmp(dir1, dir2, ignore_list)
    if len(dirs_cmp.left_only)>0 or len(dirs_cmp.right_only)>0 or \
        len(dirs_cmp.funny_files)>0:
        return False
    (_, mismatch, errors) =  filecmp.cmpfiles(
        dir1, dir2, dirs_cmp.common_files, shallow=False)
    if len(mismatch)>0 or len(errors)>0:
        return False
    for common_dir in dirs_cmp.common_dirs:
        new_dir1 = os.path.join(dir1, common_dir)
        new_dir2 = os.path.join(dir2, common_dir)
        if not are_dir_trees_equal(new_dir1, new_dir2, ignore_list):
            return False
    return True

#===============================================================================
# Returns true if the build tarball passes the pre-submit tests.
#===============================================================================
def presubmit(tarball, temp_dir):
    '''
    - rename x to x_timestamp if residual build exist
    - untar to temp
    - check files
    '''
    tmp_suffix = datetime.datetime.now().strftime("%Y%m%d_%H%M%S_%f")

    dir_path = os.path.join(temp_dir, 'untarred_build')

    if os.path.exists(dir_path):
        os.rename(dir_path, dir_path + '_' + tmp_suffix)

    os.makedirs(dir_path)

    tar = tarfile.open(tarball)
    tar.extractall(path=dir_path)
    tar.close()

    #
    # Tests performed:
    # - executable bit is not turned on for normal files
    # - file size does not exceed 26214400 bytes
    #   find . -type f -size +26214400

    ok_to_submit = True

    cmd = "/tools/bin/find " + dir_path + " -type f -executable  | /bin/grep -v -E '\.sh|\.tcl|\.pl|\.py'"
    output = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE).stdout.read()
    if output:
	ok_to_submit = False
	logger.info('Files have executable bit set:')
	logger.info('%s', output)

    cmd = "/tools/bin/find " + dir_path + " -type f -size +26214400"
    output = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE).stdout.read()
    if output:
	ok_to_submit = False
	logger.info('Files too large (> 26214400):')
	logger.info('%s', output)

    shutil.rmtree(dir_path)

    return ok_to_submit

#===============================================================================
# Returns true is the 2 builds are the same.
# Notes: Some generated files are excluded for comparison.
#===============================================================================
def are_builds_equal(tar_a, tar_b, temp_dir):
    #
    # - rename x to x_timestamp if residual build exist
    # - untar the 2 builds to temp
    # - compare the 2 directories
    #
    tmp_suffix = datetime.datetime.now().strftime("%Y%m%d_%H%M%S_%f")

    dir_a_path = os.path.join(temp_dir, 'a')
    dir_b_path = os.path.join(temp_dir, 'b')

    if os.path.exists(dir_a_path):
        os.rename(dir_a_path, dir_a_path + '_' + tmp_suffix)

    if os.path.exists(dir_b_path):
        os.rename(dir_b_path, dir_b_path + '_' + tmp_suffix)

    os.makedirs(dir_a_path)
    os.makedirs(dir_b_path)

    tar = tarfile.open(tar_a)
    tar.extractall(path=dir_a_path)
    tar.close()

    tar = tarfile.open(tar_b)
    tar.extractall(path=dir_b_path)
    tar.close()

    #
    # Ignore a few files in the comparison.  These files change even when there are
    # no significant code change.
    #
    are_equal = are_dir_trees_equal(dir_a_path, dir_b_path, ['.mogrified', 'wlc_clm_data.c.GEN', 'epivers.h'])

    shutil.rmtree(dir_a_path)
    shutil.rmtree(dir_b_path)

    return are_equal

def main():
    #
    # Basic program setups
    #
    parser = argparse.ArgumentParser(description='Find the latest wlan build and place it in the work-queue.')
    parser.add_argument('branch', action='store', help='branch or twig name')
    parser.add_argument('brand', action='store', help='brand name')
    parser.add_argument('-w', '--workq-base-dir', action='store', default=WORKQ_BASE_DIR, help='directory path to workqueues')
    parser.add_argument('-b', '--build_base_dir', action='store', default=BUILD_BASE_DIR, help='directory path to builds')
    parser.add_argument('-r', '--release', action='store', default=None, help='release name')
    parser.add_argument('-l', '--log-to-console', action='store_true', default=False, help='write log to console')
    args = parser.parse_args()

    workq_name = args.branch + '##' + args.brand
    workq_dir = os.path.join(args.workq_base_dir, workq_name)

    # Create required directories if necessary
    if not os.path.exists(workq_dir):
        os.makedirs(workq_dir)
	os.chmod(workq_dir, 0777)

    logger.setLevel(LOG_LEVEL)
    log_formatter = logging.Formatter('%(asctime)s : %(levelname)-8s : %(message)s')
    file_handler = logging.FileHandler(os.path.join(workq_dir, 'work.log'))
    file_handler.setFormatter(log_formatter)
    logger.addHandler(file_handler)

    if args.log_to_console:
        console_handler = logging.StreamHandler()
        console_handler.setFormatter(log_formatter)
        logger.addHandler(console_handler)

    logger.info('----- Program invoked by %s at %s: branch=%s brand=%s release=%s -----',
                pwd.getpwuid(os.getuid()).pw_name, socket.gethostname(), args.branch, args.brand, args.release)
    logger.debug('args=%s', sys.argv[0:])
    logger.info('work queue=%s', workq_dir)

    if is_var_defined(workq_dir, STATE_SUSPENDED_VARNAME):
        logger.info('Processing suspended.  Exiting...')
        return

    if args.release:
        tag = args.release
    else:
        tag = args.branch

    #
    # Main logic begins here:
    #
    # - loop thru the builds and find the latest successful build
    #     - _WARNING_BUILD_IN_PROGRESS : stop and don't look more, check again later
    #     - ,succeeded : found
    #     - ,build_errors.log : continue to look for the next one
    # - if build found is later the last build submitted to GIT, then submit this build to GIT
    #
    build_dates = os.listdir(os.path.join(args.build_base_dir, tag, args.brand))
    build_dates.sort(key=normaize_build_date, reverse=True)
    logger.debug('build dates=%s', build_dates)

    found = False
    for build in build_dates:
        dir = os.path.join(args.build_base_dir, tag, args.brand, build)
        dir_listing = os.listdir(dir)
        logger.debug('Build %s listing: %s', dir, dir_listing)

        if '_WARNING_BUILD_IN_PROGRESS' in dir_listing:
            logger.info('Build %s is in progress.  Exiting...', build)
            break

        if ',build_errors.log' in dir_listing:
            logger.info('Build %s failed.  Exiting...', build)
            break

        if ',succeeded' in dir_listing:
            logger.info('Found successful build %s', build)
            found = True
            break

    if not found:
        logger.info('Did not find a successful build or a build is in-progress.  Exiting...')
        return

    build_tarball_path = os.path.join(args.build_base_dir, tag, args.brand, build, 'release', 'bcm', 'packages', SRC_TARBALL_FILENAME)

    #
    # Skip this build if:
    # - it is not newer then the previous build that was submitted for git integration -- or --
    # - the content is the same as the previous.
    #
    # Special handling for release build: It has a higher priority and it will always get placed on top of the queue.
    # It is important that the release build does not fall behind the nightly.  Otherwise, will have to wait for the next nightly
    # to get back in sync.  Once the release build has been submitted to the queue, do not remember this build so that the next
    # nightly will get a higher priority.
    #
    if args.release:
        previous_build = None
        remember_submitted_build = False
    else:
        previous_build = get_latest_build(workq_dir)
        remember_submitted_build = True

    # Create a temporary working directory
    temp_dir = os.path.join(workq_dir, 'temp')
    if not os.path.exists(temp_dir):
        os.makedirs(temp_dir)
	os.chmod(temp_dir, 0777)

    if previous_build:
        if compare_build_date(build, previous_build) <= 0:
            logger.info('Build %s is not newer than %s that was submitted for git integration.  Exiting...', build, previous_build)
            return False

        # This is a newer build, see if the content is really different.
        previous_build_tarball_path = os.path.join(workq_dir, SRC_TARBALL_FILENAME)
        if are_builds_equal(build_tarball_path, previous_build_tarball_path, temp_dir):
            logger.info('Current build %s and previous build %s are the same.  Exiting...', build, previous_build)
            return False

    #
    # Do a conformance test to make sure the files conform to the git requirements
    #
    if not presubmit(build_tarball_path, temp_dir):
	logger.info('Build %s failed presubmit test.  Exiting...', build)
        return False

    #
    # If we get this far, we have a new build to submit for git integration
    #
    submit_build(workq_dir, build, build_tarball_path, tag, args.branch, args.brand, remember_submitted_build)
    logger.info('Submitted a new build for git integration %s', build)

if __name__ == '__main__':
    main()
    logger.info('----- Program ended -----')


