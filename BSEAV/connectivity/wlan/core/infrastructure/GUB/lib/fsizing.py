"""Contains the Sizing class which kicks off analysis of FW size growth."""

from __future__ import print_function
from __future__ import unicode_literals

import os

import lib.scm
import lib.util

# Electric Commander constants
ECTOOL = '/projects/hnd/tools/linux/ecloud_commander/3.10/bin/ectool'
ECTOOL_USER = 'wlanswci'
ECTOOL_SERVER = 'commander.sj.broadcom.com'
ECTOOL_PWDFILE = os.path.expanduser('~/restricted/wlanswci.passwd')
# Location of sizing script and cfg in a branch
SIZING_SCRIPT = 'src/tools/release/sizing/size_features.py'
SIZING_CFG = 'src/tools/release/sizing/sizing.cfg'
SIZING_DIR = '__SIZING__'
# Temporary directory
TEMP_DIR = '/projects/wlansw_test_scratch/'
BRANCH_TOKEN = '_BRANCH_'


def run(blddir, tag):
    """Kick off analysis of firmware size growth"""

    # Make everything in the scratch dir group writable so it can be
    # deleted by the background sizing process.
    scratch_dir = lib.util.tmpdir(tmp=TEMP_DIR, keep=True, mode=0o775,
                                  prefix=tag + '.')

    # Copy required files to the scratch directory.
    cmd = ['cp', '-r', '%s/.' % blddir, scratch_dir]
    lib.util.execute(cmd)

    # And make sure they're writable by group.
    cmd = ['chmod', '-R', 'ug+rw', scratch_dir]
    lib.util.execute(cmd)

    # Login to the EC server
    lib.util.note('using EC password file: %s' % ECTOOL_PWDFILE, vl=2)

    # Read password from secret store
    ectool_password = open(ECTOOL_PWDFILE).read().strip()

    lib.util.note('login to EC server %s' % ECTOOL_SERVER)
    cmd = [
        ECTOOL,
        '--server %s' % ECTOOL_SERVER,
        'login',
        ECTOOL_USER,
        ectool_password
    ]
    try:
        # Since this cmd contains a password we suppress its verbosity and
        # also handle the exception to avoid dumping the command line on
        # failure.
        rc = lib.util.execute(cmd, vl=-1)
    except Exception as e:
        lib.util.error(str(e))
        lib.util.rm(scratch_dir)
        return 1

    # Trigger the analysis.
    if rc == 0:
        lib.util.note('start sizing analysis of %s' % tag)
        cmd = [
            ECTOOL,
            '--server %s' % ECTOOL_SERVER,
            'runProcedure',
            'WLAN-SW-CI',
            '--procedureName',
            'feature-sizing-gub',
            '--actualParameter',
            'SCRIPT=%s' % SIZING_SCRIPT,
            'CFG=%s' % SIZING_CFG,
            'WORKSPACE=%s' % scratch_dir,
            'OUTPUT_DIRECTORY=%s' % os.path.join(blddir, SIZING_DIR),
            'DEBUG_MODE=false',
            'BRANCH=%s' % tag,
        ]
        rc = lib.util.execute(cmd)

    if rc != 0:
        lib.util.rm(scratch_dir)

    return rc

# vim: ts=8:sw=4:tw=80:et:
