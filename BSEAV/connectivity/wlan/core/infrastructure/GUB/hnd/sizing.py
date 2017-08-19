#!/usr/bin/env python2.7
"""
Compare size and sym movement between local build and baseline build.

By default, the baseline build from the CI logs will be either:
    - the unmodified version of the build (preferred)
or if that does not exist:
    - the closest previous version of the build

The user may also specify a custom baseline sym file if desired.

EXAMPLES:

   %(prog)s sizing path/to/build/files

   %(prog)s sizing path/to/build/files -u old/build/files/rtecdc.map-size
"""

# Copyright (C) 2016, Broadcom Corporation
# All Rights Reserved.

# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Limited;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import glob
import os
import re
import subprocess
import sys
import yaml

try:
    import lib.consts
    import lib.scm
    import lib.util
except ImportError:

    WLBASE = '.wlbase'

    class LibOverride(object):
        """Implement the limited library functions required to run this script
        stand-alone."""

        class ConstOverride(object):
            """Create a class to override the consts imports from the standard
            GUB infrastructure."""

            SJ_SSH_HOST = 'xxx.broadcom.com'
            SSH_CMD = [
                'ssh',
                '-o', 'BatchMode=yes',
                '-o', 'ConnectTimeout=60',
                '-o', 'ConnectionAttempts=10',
                '-o', 'LogLevel=error',
                '-o', 'StrictHostKeyChecking=no',
            ]

        class ScmOverride(object):
            """Create a class to override the scm imports from the standard GUB
            infrastructure."""

            @staticmethod
            def svncmd(*args, **kwargs):
                """Implements a basic version of the svncmd function found in
                the GUB infrastructure."""
                if kwargs.get('as_builder'):
                    pass
                cmd = ['svn']
                cmd.extend(list(args))
                return cmd

            @staticmethod
            def find_tree_base(path='.'):
                """Find the base of the current checkout."""
                curdir = os.path.realpath(path)
                checkout = tree = None
                while True:
                    parent = os.path.dirname(curdir)
                    if os.stat(parent).st_ino == os.stat(curdir).st_ino:
                        break
                    elif os.path.exists(os.path.join(curdir, WLBASE)):
                        tree = curdir
                        break
                    elif os.path.exists(os.path.join(curdir, '.gclient')):
                        tree = curdir
                        break
                    elif os.path.exists(os.path.join(curdir,
                                                     '.svn/sparsefile_url.py')):
                        tree = curdir
                        break
                    elif not checkout:
                        if os.path.exists(os.path.join(curdir, '.svn')):
                            if os.path.exists(os.path.join(curdir,
                                                           '.svn/wc.db')):
                                checkout = curdir
                            elif not os.path.exists(os.path.join(curdir,
                                                                 '../.svn')):
                                # Backward compatibility for svn 1.6.
                                checkout = curdir
                        elif os.path.exists(os.path.join(curdir, '.git')):
                            checkout = curdir
                    curdir = parent
                if tree:
                    return tree
                else:
                    return checkout

        class UtilOverride(object):
            """Create a class to override the util imports from the standard GUB
            infrastructure."""

            @staticmethod
            def verbose_enough(vl):
                """Setting the verbose logging is not supported in the
                standalone port of this function."""
                if vl:
                    pass
                return False

            @staticmethod
            def die(msg, **kwargs):
                """Raise a simple error for the die function."""
                if kwargs:
                    pass
                raise RuntimeError(msg)

            @staticmethod
            def note(msg, vl=1):
                """Only support manual enable of notes."""
                if False:
                    sys.stderr.write(msg + vl + '\n')

        def __init__(self):
            self.consts = self.ConstOverride()
            self.scm = self.ScmOverride()
            self.util = self.UtilOverride()

    # Global to override the GUB shared lib import. Need to bypass pylint
    # to support this workaround.
    lib = LibOverride()  # pylint: disable=C0103


def parse_cli(prog, alias, subcmds, _, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser.add_argument(
        '-r', '--show-reclaimed', action='store_true', default=False,
        help="show reclaimed syms and include their sizes in summary")
    parser.add_argument(
        '--error-threshold', type=int, default=None,
        help="generate error when size increases above ERROR_THRESHOLD")
    parser.add_argument(
        '--syms-info', default='rtecdc.map-size',
        help="name of syms file in build_dir (default: '%(default)s')")
    parser.add_argument(
        '--sizing-info', default='sizing_info.yaml',
        help="name of sizing file in build_dir (default: '%(default)s')")
    parser.add_argument(
        '-u', '--user-sym-file',
        help="user-specified baseline sym file"
        ' (default: look up sizing file from CI logs)')
    parser.add_argument(
        '-s', '--size-threshold', type=int, default=None,
        help="generate error when a sym made by user exceeds SIZE_THRESHOLD")
    parser.add_argument(
        '-l', '--long-format', action='store_true',
        help="include all syms that exceed SIZE_THRESHOLD")
    parser.add_argument(
        '-i', '--ignore-syms-list', nargs='+',
        help="list of syms to ignore")
    parser.add_argument(
        '-f', '--ignore-syms-file',
        help="file containing syms to ignore")
    parser.add_argument(
        'build_dir',
        help="path to build files")
    parser.set_defaults(func=call)


class SubversionWorkspace(object):

    """Get data from subversion working copy."""

    def __init__(self, top_dir):
        # Converts svn info output into a dictionary.
        cmd = lib.scm.svncmd('info', top_dir)
        svn_info_lines = subprocess.check_output(cmd).splitlines()
        self.info = dict([sl.split(': ', 1)
                          for sl in svn_info_lines if sl.strip()])

    @property
    def tag(self):
        """Extract branch name from repository URL."""
        words = self.info['URL'].split('/')
        if 'branches' in words:
            tag = words[words.index('branches') + 1]
        elif 'tags' in words:
            tag = words[words.index('tags') + 2]
        elif 'trunk' in words:
            tag = 'trunk'
        else:
            lib.util.die('cannot find branch name in ' + self.info['URL'],
                         exc=False)
        return tag

    @property
    def revision(self):
        """Return current svn revision."""
        return int(self.info['Revision'])

    def get_baseline_filename(self, sizing_dir, target, filename_suffix):
        """
        Get a baseline sizing log file for a workspace's revision.

        This is derived from sizing info present in the sizing dir. It
        is managed by an external process, most likely continuous
        integration builds.

        The log file retrieved depends on the suffix used:
            '.log' will retrieve sizing_info.yaml log files
            '_sizing_info.log' will retrieve rtecdc.map-size log files
        """

        # Try to find a suitable sizing file from CI.
        # Prefer sizing info of the same revision, otherwise take the previous
        # closest revision. Baseline sizing info is only generated builds
        # against revisions that have relevant changes in them.
        remote_file = False
        temp_dir = '/tmp'
        base_dir = os.path.join(sizing_dir, self.tag, target)
        file_regex = re.compile(r'(\d+)' + filename_suffix)
        sizing_files = glob.glob(
            os.path.join(
                base_dir,
                '*[0-9][0-9][0-9][0-9]' +
                filename_suffix))

        # List remote files on SJ_SSH_HOST if we can't see a local file
        if not sizing_files:
            lib.util.note('no local sizing info, trying ' +
                          lib.consts.SJ_SSH_HOST)
            try:
                sizing_files = subprocess.check_output(
                    lib.consts.SSH_CMD + [lib.consts.SJ_SSH_HOST,
                                          'ls', base_dir]).split()
            except subprocess.CalledProcessError as e:
                if e.returncode <= 2:
                    # Minor ls error - unable to find sizing log file
                    # Raise SizingLogError for caller to handle
                    raise SizingLogError('cannot find sizing info at %s:%s' %
                                         (lib.consts.SJ_SSH_HOST, base_dir))
                else:
                    # A serious error has occured, abort
                    raise

            # Select relevant file names and prepend full file path
            sizing_files = [os.path.join(base_dir, a)
                            for a in sizing_files
                            if file_regex.match(a)]
            remote_file = True

        # Create list of (revid, filepath).
        sizing_files_pairs = sorted([(int(file_regex.search(a).group(1)), a)
                                     for a in sizing_files])
        try:
            # Finds nearest oldest baseline revision by returning the last
            # element of a list that only contains revisions less than or equal
            # to the local revision.
            rev, sizing_file = [(str(r), path)
                                for r, path in sizing_files_pairs
                                if r <= self.revision][-1]
        except IndexError:
            raise SizingLogError('cannot find sizing info in ' + base_dir)

        # Copy file to /tmp if file is remote
        if remote_file:
            subprocess.check_call(['scp'] + lib.consts.SSH_CMD[1:] +
                                  [lib.consts.SJ_SSH_HOST + ':' +
                                   sizing_file, temp_dir])
            sizing_file = os.path.join(temp_dir, rev + filename_suffix)

        return rev, sizing_file


class SizingLogError(EnvironmentError):

    """Error class for use with CI sizing logs."""
    pass


def compare_syms(sizing_dir, build_dir, user_sym_file, local_sizing_filename,
                 local_syms_filename, show_reclaimed, error_threshold,
                 size_threshold, long_format, ignore_syms_list,
                 ignore_syms_file):
    """
    Analyse sizing changes and sym movement between builds.

    This script will try and locate a syms file and corresponding log to
    generate a full report on sym movement in addition to the overall size
    change. If either the local or baseline (CI log) syms file is missing,
    it will fall back on the sizing file and only report on the total
    size change.
    """

    # find build directory
    abs_build_dir = os.path.abspath(build_dir)

    norm_build_dir = os.path.normpath(
        abs_build_dir.rstrip(os.path.sep)).split(os.path.sep)
    sym_file_exists, top_dir = get_top_dir(norm_build_dir, abs_build_dir,
                                           local_sizing_filename,
                                           local_syms_filename)
    workspace = SubversionWorkspace(top_dir)

    # get local files
    local_rev = workspace.revision
    local_sizing_path = os.path.join(abs_build_dir, local_sizing_filename)
    if sym_file_exists:
        local_syms_path = os.path.join(abs_build_dir, local_syms_filename)

    # get baseline sizing files
    if user_sym_file:
        # use sizing file from user
        if not os.path.isfile(user_sym_file):
            lib.util.die('cannot locate user sizing file %s' % user_sym_file,
                         exc=False)
        baseline_rev = -1  # revision unknown (file supplied by user)
        baseline_syms_path = user_sym_file
        if not sym_file_exists:
            # if user supplied a syms file we need one in the build tree too
            lib.util.die('cannot locate %s in build tree'
                         % local_syms_filename, exc=False)
    else:
        # Get build target from build dir and find appropriate CI sizing file.
        # eg,
        #     ......./.../....../.../../....../(build target dir) /
        #    AARDVARK/src/dongle/rte/wl/builds/4335b0-roml/sdio-ag/
        target = os.path.sep.join(norm_build_dir[-2:])
        if sym_file_exists:
            try:
                baseline_rev, baseline_syms_path = \
                    workspace.get_baseline_filename(sizing_dir, target,
                                                    '_sizing_data.log')
            except SizingLogError:
                sym_file_exists = False
        if not sym_file_exists:
            baseline_rev, baseline_sizing_path = \
                workspace.get_baseline_filename(sizing_dir, target, '.log')
            # check local sizing_info.yaml file actually exists
            # (we only need it if not using sym file)
            if not os.path.isfile(local_sizing_path):
                lib.util.die('cannot locate %s in the build tree'
                             % local_sizing_filename, exc=False)

    # open and parse files
    if sym_file_exists:
        # Build dicts of rom and ram sym data.
        with open(baseline_syms_path, 'r') as old_file:
            old_rom, old_ram = split_rom_ram(old_file)
        with open(local_syms_path, 'r') as new_file:
            new_rom, new_ram = split_rom_ram(new_file)

        sym_movement = build_sym_movement(old_rom, old_ram, new_rom, new_ram)
        if not show_reclaimed:
            sym_movement = scrub_reclaimed(sym_movement, new_ram)
        if not lib.util.verbose_enough(2):
            sym_movement = scrub_toolchain_syms(sym_movement)

        # calculate static ram sizes from sym data
        local_static_ram = get_ram_size(new_ram)
        baseline_static_ram = get_ram_size(old_ram)
    else:
        # calculate static ram sizes from sizing_info.yaml
        baseline_data = yaml.load(open(baseline_sizing_path, 'r').read())
        local_data = yaml.load(open(local_sizing_path, 'r').read())
        baseline_static_ram = baseline_data['static_ram_usage']
        local_static_ram = local_data['static_ram_usage']

    static_ram_delta = local_static_ram - baseline_static_ram
    size_warnings = 0
    # print reports
    if sym_file_exists:
        print_sym_report(sym_movement, static_ram_delta)
        if size_threshold is not None:
            sym_sizing = build_sym_sizing(old_rom, old_ram, new_rom, new_ram)
            size_warnings = print_sizing_warnings(sym_sizing, size_threshold,
                                                  long_format,
                                                  ignore_syms_list,
                                                  ignore_syms_file,
                                                  sym_movement)
    else:
        print('No sym data available')
    print_sizing_report(local_rev, baseline_rev, local_static_ram,
                        baseline_static_ram)

    # check size increase against error threshold
    rc = 0
    if error_threshold is not None and static_ram_delta > error_threshold:
        # size increase is too much, flag as error.
        lib.util.error('SIZING_THRESHOLD_ERROR, %s %s' % (
            'size delta %d' % static_ram_delta,
            'exceeds threshold %d' % error_threshold))
        rc = 142  # a local convention for precommit
    if size_threshold is not None and size_warnings:
        # changes made result in syms larger than threshold.
        lib.util.error('SYM_SIZING_ERROR, %d sym(s) exceed threshold %d' % (
            size_warnings, size_threshold))
        rc = 142  # a local convention for precommit
    return rc


def build_sym_sizing(old_rom, old_ram, new_rom, new_ram):
    """
    Build a dict of data on sym sizing.

    Output dict is keyed by memory region name and contains nested dicts.
    The nested dicts hold dicts keyed by sym name contained the sym's size.
    Syms can be in the following regions:
        added:              new sym added to ram
        rom:                exist in rom
        invalidated:        moved from rom to ram
        grew:               remained in ram but increase in size
        shrank:             remained in ram but reduced in size
        stationary:         remained in ram but remained the same size
        reclaimable:        moved from nonreclaimable to reclaimable section
    """
    sets = {}
    sets['old_rom'] = set(old_rom)
    sets['old_ram'] = set(old_ram)
    sets['new_rom'] = set(new_rom)
    sets['new_ram'] = set(new_ram)

    sets['invalidated'] = sets['old_rom'].intersection(sets['new_ram'])
    sets['added'] = sets['new_ram'].difference(sets['old_ram']
                                               .union(sets['old_rom']))
    sets['stationary'] = sets['old_ram'].intersection(sets['new_ram'])
    sets['rom'] = sets['new_rom']
    sets['reclaimable'] = set()

    nreclaim_ranges = list()

    reclaim_range_labels = (('_rstart1', '_rend1'),
                            ('_rstart2', '_rend2'),
                            ('_rstart3', '_rend3'),
                            ('_rstart4', '_rend4'),
                            ('_rstart5', '_rend5'))

    for rlabelstart, rlabelend in reclaim_range_labels:
        if rlabelstart in old_ram:
            nreclaim_ranges.append((new_ram[rlabelstart]['addr'],
                                    new_ram[rlabelend]['addr']))

    def inranges(ranges, addr):
        """Check if an address is in any of the provided ranges."""
        for x, y in ranges:
            if x <= addr < y:
                return True
        return False

    for sym in sets['stationary']:
        if inranges(nreclaim_ranges, new_ram[sym]['addr']):
            sets['reclaimable'].add(sym)

    # Remove symbols that can be reclaimed from the stationary list.

    sets['stationary'] -= sets['reclaimable']

    sym_sizing = {}

    for zone in ['added', 'invalidated', 'reclaimable']:
        sym_sizing[zone] = {}
        for sym in sets[zone]:
            sym_sizing[zone][sym] = new_ram[sym]['size']

    for zone in ['rom']:
        sym_sizing[zone] = {}
        for sym in sets[zone]:
            sym_sizing[zone][sym] = new_rom[sym]['size']

    sym_sizing['grew'] = {}
    sym_sizing['shrank'] = {}
    sym_sizing['stationary'] = {}
    for sym in sets['stationary']:
        if new_ram[sym]['size'] != old_ram[sym]['size']:
            if (new_ram[sym]['size'] - old_ram[sym]['size']) > 0:
                sym_sizing['grew'][sym] = new_ram[sym]['size']
            else:
                sym_sizing['shrank'][sym] = new_ram[sym]['size']
        else:
            sym_sizing['stationary'][sym] = new_ram[sym]['size']

    return sym_sizing


def print_sizing_warnings(sym_sizing, size_limit, long_format, ignore_syms_list,
                          ignore_syms_file, sym_movement):
    """Print details on the syms exceeding sizing threshold."""
    num_warnings = 0
    zones = ['added', 'grew', 'invalidated', 'shrank']
    if long_format:
        zones.extend(['stationary', 'rom', 'reclaimable'])

    if not ignore_syms_list:
        ignore_syms_list = []
    if ignore_syms_file:
        with open(ignore_syms_file) as f:
            ignore_syms_list.extend(f.read().splitlines())

    print()
    print('Oversized Syms:')
    sym_template = '{:<12} {:>12}    {:<0}'
    print(sym_template.format('Location', 'Size(Delta)', 'Name'))
    for zone in zones:
        for sym in sym_sizing[zone]:
            if sym_sizing[zone][sym] > size_limit and \
               sym not in ignore_syms_list:
                num_warnings += 1
                if re.match(r'\w*[.]\d+$', sym):
                    continue
                if zone in ['grew', 'shrank']:
                    size_with_delta = str(sym_sizing[zone][sym]) + '(' + \
                        str(sym_movement[sym]['size']) + ')'
                    print(sym_template.format(zone, size_with_delta, sym))
                else:
                    print(sym_template.format(zone, sym_sizing[zone][sym], sym))

    return num_warnings


def get_top_dir(norm_build_dir, abs_build_dir, local_sizing_filename,
                local_syms_filename):
    """
    Get top_dir from build_dir (used to work out checkout revision).

    Assumes that build dir is within current checkout.
    eg,
        top_dir/.../....../.../../....../.........../......./build_dir
        AARDVARK/src/dongle/rte/wl/builds/4335b0-roml/sdio-ag/

    Also returns a flag indicating whether the syms file is present
    """

    sym_file_exists = False  # is sym file present in local build tree?
    sizing_path = os.path.join(*(norm_build_dir[-2:] +
                                 [local_sizing_filename]))
    syms_path = os.path.join(*(norm_build_dir[-2:] + [local_syms_filename]))
    try:
        treebase = lib.scm.find_tree_base(abs_build_dir)
    except OSError:
        lib.util.die('cannot locate build tree', exc=False)

    # New location: look for sizing files under build/dongle.
    if os.path.isfile(os.path.join(treebase, 'build', 'dongle', syms_path)):
        sym_file_exists = True
        top_dir = os.path.sep.join(norm_build_dir[:-4]) + os.path.sep + 'src'
    elif os.path.isfile(os.path.join(treebase, 'build', 'dongle',
                                     sizing_path)):
        sym_file_exists = False
        top_dir = os.path.sep.join(norm_build_dir[:-4]) + os.path.sep + 'src'

    # Old location: look for sizing files under src/dongle/rte/wl/builds.
    elif os.path.isfile(os.path.join(treebase, 'src', 'dongle',
                                     'rte', 'wl', 'builds', syms_path)):
        sym_file_exists = True
        top_dir = os.path.sep.join(norm_build_dir[:-7])
    elif os.path.isfile(os.path.join(treebase, 'src', 'dongle',
                                     'rte', 'wl', 'builds', sizing_path)):
        sym_file_exists = False
        top_dir = os.path.sep.join(norm_build_dir[:-7])
    else:
        lib.util.die('cannot locate %s in build tree' % local_sizing_filename,
                     exc=False)

    return sym_file_exists, top_dir


def split_rom_ram(input_file):
    """
    Create dicts of ROM and RAM syms from a sym map file.

    RAM syms are those between text_start and _end, other syms are ROM.
    Output dicts are keyed by sim name and hold nested dicts. The nested
    dicts hold values for the following keys: addr, sym_type, size.
    """
    rom = {}
    ram = {}
    ram_flag = False
    sym_regex = re.compile(r'(?P<addr>[0-9a-fA-F]{8})\s(?P<sym_type>.)\s'
                           r'(?P<sym>[\w$._]+)\s?(?P<size>[0-9a-fA-F]{8})?')

    for line in input_file:
        match = sym_regex.match(line)
        if match:
            sym = match.group('sym')
            addr = int(match.group('addr'), 16)
            sym_type = match.group('sym_type')
            try:
                size = int(match.group('size'), 16)
            except TypeError:
                size = 0
            temp = dict(addr=addr, sym_type=sym_type, size=size)
            if ram_flag:
                ram[sym] = temp
                if sym == '_rend3' and '_end' in ram \
                        or sym == '_end' and '_rend3' in ram:
                    # End of RAM reached.
                    ram_flag = False
            elif sym in ['text_start', 'bootloader_patch_end',
                         'bootloader_patch_start']:
                # Start of RAM section reached.
                ram[sym] = temp
                ram_flag = True
            else:
                rom[sym] = temp
    return rom, ram


def build_sym_movement(old_rom, old_ram, new_rom, new_ram):
    """
    Build a dict of data on sym movement.

    Output dict is keyed by sim name and contains nested dicts. The nested
    dicts hold values for the following keys: addr, sym_type, size, movement.
    Sym movement can have the following values:
        removed:            previously in ram, no longer in use
        added:              new sym added to ram
        invalidated:        moved from rom to ram
        revalidated:        moved from ram back to rom
        stationary:         remained in ram
        toreclaim:          moved from nonreclaimable to reclaimable section
        fromreclaim:        moved from reclaimable to nonreclaimable section
    """

    sets = {}  # keep all the sets of sym names in a dict for convenience
    sets['old_rom'] = set(old_rom)
    sets['old_ram'] = set(old_ram)
    sets['new_rom'] = set(new_rom)
    sets['new_ram'] = set(new_ram)

    sets['invalidated'] = sets['old_rom'].intersection(sets['new_ram'])
    sets['revalidated'] = sets['old_ram'].intersection(sets['new_rom'])
    sets['removed'] = sets['old_ram'].difference(sets['new_ram']
                                                 .union(sets['new_rom']))
    sets['added'] = sets['new_ram'].difference(sets['old_ram']
                                               .union(sets['old_rom']))
    sets['stationary'] = sets['old_ram'].intersection(sets['new_ram'])

    sets['toreclaim'] = set()
    sets['fromreclaim'] = set()

    # Check for symbols that have moved between reclaimable and nonreclaimable
    # sections.

    oreclaim_ranges = list()
    nreclaim_ranges = list()

    reclaim_range_labels = (('_rstart1', '_rend1'),
                            ('_rstart2', '_rend2'),
                            ('_rstart3', '_rend3'),
                            ('_rstart4', '_rend4'),
                            ('_rstart5', '_rend5'))

    for rlabelstart, rlabelend in reclaim_range_labels:
        if rlabelstart in old_ram:
            oreclaim_ranges.append((old_ram[rlabelstart]['addr'],
                                    old_ram[rlabelend]['addr']))
            nreclaim_ranges.append((new_ram[rlabelstart]['addr'],
                                    new_ram[rlabelend]['addr']))

    def inranges(ranges, addr):
        """Check if an address is in any of the provided ranges."""
        for x, y in ranges:
            if x <= addr < y:
                return True
        return False

    for sym in sets['stationary']:
        if (inranges(nreclaim_ranges, new_ram[sym]['addr']) and
                not inranges(oreclaim_ranges, old_ram[sym]['addr'])):
            sets['toreclaim'].add(sym)
        elif (not inranges(nreclaim_ranges, new_ram[sym]['addr']) and
              inranges(oreclaim_ranges, old_ram[sym]['addr'])):
            sets['fromreclaim'].add(sym)

    # Remove symbols that have moved between reclaimable and nonreclaimable
    # sections from the stationary list.

    sets['stationary'] -= sets['toreclaim'] | sets['fromreclaim']

    sym_movement = {}
    for movement in ('added', 'invalidated', 'fromreclaim'):
        for sym in sets[movement]:
            sym_movement[sym] = new_ram[sym].copy()
            sym_movement[sym]['movement'] = movement
    for movement in ('removed', 'revalidated', 'toreclaim'):
        for sym in sets[movement]:
            sym_movement[sym] = old_ram[sym].copy()
            sym_movement[sym]['movement'] = movement
            sym_movement[sym]['size'] = -sym_movement[sym]['size']

    for sym in sets['stationary']:
        if new_ram[sym]['size'] != old_ram[sym]['size']:
            sym_movement[sym] = new_ram[sym].copy()
            sym_movement[sym]['size'] = new_ram[sym]['size'] \
                - old_ram[sym]['size']
            if sym_movement[sym]['size'] > 0:
                sym_movement[sym]['movement'] = 'grew'
            else:
                sym_movement[sym]['movement'] = 'shrank'

    return sym_movement


def scrub_toolchain_syms(sym_movement):
    """Remove compiler-created entries from sym list."""
    tool_regex = re.compile(r'\w*[.]\d+$')
    for sym in sym_movement.keys():
        if tool_regex.match(sym):
            sym_movement.pop(sym)

    return sym_movement


def scrub_reclaimed(sym_movement, new_ram):
    """Remove syms in the reclaimed region."""
    rstart1 = new_ram['_rstart1']['addr']
    rend3 = new_ram['_rend3']['addr']

    for sym in sym_movement.keys():
        if sym_movement[sym]['addr'] >= rstart1 and \
                sym_movement[sym]['addr'] <= rend3:
            try:
                sym_movement.pop(sym)
            except KeyError:
                pass

    return sym_movement


def get_ram_size(ram):
    """Calculate static RAM usage after reclaim for a list of RAM lines."""
    try:
        size = ram['_end']['addr'] - ram['text_start']['addr']
        reclaim = ram['_rend3']['addr'] - ram['_rstart1']['addr']
    except KeyError as e:
        lib.util.die('cannot calculate static ram size: ' + e.args[0] +
                     ' sym missing from map file', exc=False)
    if reclaim <= 15:
        reclaim = 0
    try:
        reclaim += (ram['_patch_table_start']['addr'] -
                    ram['_patch_align_start']['addr'])
        reclaim += (ram['_patch_table_end']['addr'] -
                    ram['_patch_table_last']['addr'])
        reclaim += (ram['_patch_hdr_end']['addr'] -
                    ram['_patch_hdr_start']['addr'])
    except KeyError:
        pass
    size_after_reclaim = size - reclaim

    return size_after_reclaim


def print_sym_report(sym_movement, total_delta):
    """Print the sym data (sorted by sym name) and a summary to stdout."""
    sym_template = '{:<12} {:>8}    {:<0}'
    print(sym_template.format('Change', 'Size', 'Name'))
    for sym in sorted(sym_movement):
        print(sym_template.format(sym_movement[sym]['movement'],
                                  sym_movement[sym]['size'], sym))

    summary_template = '{:<19} {:+10}'
    added_total = sum(sym_movement[sym]['size'] for sym in sym_movement
                      if sym_movement[sym]['movement'] == 'added')
    invalidated_total = sum(sym_movement[sym]['size'] for sym in sym_movement
                            if sym_movement[sym]['movement'] == 'invalidated')
    removed_total = sum(sym_movement[sym]['size'] for sym in sym_movement
                        if sym_movement[sym]['movement'] == 'removed')
    revalidated_total = sum(sym_movement[sym]['size'] for sym in sym_movement
                            if sym_movement[sym]['movement'] == 'revalidated')

    grown_total = sum(sym_movement[sym]['size'] for sym in sym_movement
                      if sym_movement[sym]['movement'] == 'grew')
    shrunk_total = sum(sym_movement[sym]['size'] for sym in sym_movement
                       if sym_movement[sym]['movement'] == 'shrank')
    fromreclaim_total = sum(sym_movement[sym]['size'] for sym in sym_movement
                            if sym_movement[sym]['movement'] == 'fromreclaim')
    toreclaim_total = sum(sym_movement[sym]['size'] for sym in sym_movement
                          if sym_movement[sym]['movement'] == 'toreclaim')

    tool_total = total_delta - (
        added_total + invalidated_total + removed_total +
        revalidated_total + grown_total + shrunk_total +
        fromreclaim_total + toreclaim_total)

    print()
    print(summary_template.format('Added syms:', added_total))
    print(summary_template.format('Invalidated syms:', invalidated_total))
    print(summary_template.format('Removed syms:', removed_total))
    print(summary_template.format('Revalidated syms:', revalidated_total))
    print(summary_template.format('Grown syms:', grown_total))
    print(summary_template.format('Shrunk syms:', shrunk_total))
    print(summary_template.format('FromReclaim syms:', fromreclaim_total))
    print(summary_template.format('ToReclaim syms:', toreclaim_total))
    print(summary_template.format('Toolchain syms:', tool_total))
    sys.stdout.flush()


def print_sizing_report(local_rev, baseline_rev, local_static_ram,
                        baseline_static_ram):
    """Print info on the total static ram delta between revisions."""
    print()
    print('Sizing Delta Results:')
    print('local_rev: %s' % (local_rev))
    print('baseline_rev: %s' % (baseline_rev))
    print('local_static_ram_usage: %d' % (local_static_ram))
    print('baseline_static_ram_usage: %d' % (baseline_static_ram))
    print('static_ram_delta: %+d' % (local_static_ram - baseline_static_ram))
    sys.stdout.flush()


def call(opts, cfgproxy):
    """Standard subcommand entry point."""
    cfgroot = cfgproxy.parse()
    sizing_dir = cfgroot['sizing']['sizing-dir']
    rc = compare_syms(sizing_dir, opts.build_dir, opts.user_sym_file,
                      opts.sizing_info, opts.syms_info, opts.show_reclaimed,
                      opts.error_threshold, opts.size_threshold,
                      opts.long_format, opts.ignore_syms_list,
                      opts.ignore_syms_file)
    sys.exit(rc)


def main():
    """Support standalone running of the sizing script. Only some of the
    options are required."""
    parser = argparse.ArgumentParser(
        description='Compare size and sym movement '
        'between local build and user build.',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument(
        '-u', '--user-sym-file', required=True,
        help="user-specified baseline sym file"
        ' (default: look up sizing file from CI logs)')
    parser.add_argument(
        'build_dir',
        help="path to build files")
    parser.add_argument(
        '-r', '--show-reclaimed', action='store_true', default=False,
        help="show reclaimed syms and include their sizes in summary")
    args = parser.parse_args()
    rc = compare_syms('', args.build_dir, args.user_sym_file,
                      'sizing_info.yaml', 'rtecdc.map-size',
                      args.show_reclaimed,
                      None, None,
                      False, None,
                      None)
    sys.exit(rc)


if __name__ == '__main__':
    main()

# vim: ts=8:sw=4:tw=80:et:ft=python
