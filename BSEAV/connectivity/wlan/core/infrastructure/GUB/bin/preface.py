#!/usr/bin/env python
"""
Fix up sys.path etc. in a standard manner across all GUB tools.

There are some bootstrap tasks that need to be done to get
going; the idea is that if the exec-ed script can find this
module it should do all remaining setup. In particular it
enhances sys.path to find all internal modules. It also
determines the program name and location for use by those
modules. It imports compatibility libraries for use by
Python 2.6, it enforces that 2.6 is the minimum level, etc.

"""

from __future__ import print_function
from __future__ import unicode_literals

# Need these ASAP in order to mess with sys.path etc.
import glob
import os
import sys

# This may go away in production but .pyc are annoying
# in development due to ownership issues.
sys.dont_write_bytecode = True

# Valid names by which the main command may be invoked.
NAMES = ('hnd', 'gub', 'wcc')

# The name of the currently running program.
PROG = os.path.basename(sys.argv[0])

# The path to the base directory of this toolset.
TOOLDIR = os.path.dirname(os.path.realpath(sys.path[0]))

# The absolute path to PROG with symlinks expanded.
REALPROG = os.path.realpath(sys.argv[0])

# This allows a symlink directly to a subcommand.
# E.g. "cstyle -> hnd" results in "hnd cstyle ...".
if os.path.islink(sys.argv[0]):
    if PROG not in NAMES:
        if os.path.basename(os.readlink(sys.argv[0])) in NAMES:
            sys.argv[0] = PROG
            sys.argv.insert(0, os.path.join(os.path.dirname(REALPROG),
                                            NAMES[0]))
else:
    # A command line like "bin/hnd foo ..." must be normalized in case of cd.
    sys.argv[0] = REALPROG

# Try to fail as gracefully as possible in bad versions.
if (sys.version_info[0] != 2) or (sys.version_info[1] < 6):
    sys.stderr.write('%s: Error: Python >= 2.6 required (%d.%d found)\n' %
                     (sys.argv[0], sys.version_info[0], sys.version_info[1]))
    sys.exit(2)

# Python creates this directory and then warns about it being
# writable (for people with umask=002). This solves the problem
# by forcing it to mode 755 if it exists.
_PYTHON_EGGS = os.path.expanduser('~/.python-eggs')
if os.path.exists(_PYTHON_EGGS) and os.access(_PYTHON_EGGS, os.W_OK):
    os.chmod(_PYTHON_EGGS, 0o755)

# May be useful later.
SYS_PATH_ORIG = sys.path[:]

# If wheels or eggs are present, place them at the end of sys.path
# so locally installed modules will take precedence. Place
# arch-specific packages before generic ones and .whl before .egg.
# Special case: the uname subdir is forced to lower case because
# IT has an rsync exclusion rule that keeps "Linux" from being copied.
ZIPDIR = os.path.join(TOOLDIR, 'zips')
if os.path.isdir(ZIPDIR):
    _UNAME = os.uname()
    _ARCH_ZIPS = os.path.join(ZIPDIR, _UNAME[0].lower(), _UNAME[4])
    sys.path.extend(glob.glob(os.path.join(_ARCH_ZIPS, '*.whl')))
    sys.path.extend(glob.glob(os.path.join(_ARCH_ZIPS, '*.egg')))
    sys.path.extend(glob.glob(os.path.join(_ARCH_ZIPS, '*.zip')))
    sys.path.extend(glob.glob(os.path.join(ZIPDIR, '*.whl')))
    sys.path.extend(glob.glob(os.path.join(ZIPDIR, '*.egg')))
    sys.path.extend(glob.glob(os.path.join(ZIPDIR, '*.zip')))

# Because GUB scripts live in a ./bin subdirectory.
if TOOLDIR not in sys.path:
    sys.path.insert(1, TOOLDIR)

# Push these into lib.consts so modules don't need to import this to use them.
import lib.consts  # nopep8
lib.consts.ARGV = sys.argv[:]
lib.consts.PROG = PROG
lib.consts.TOOLDIR = TOOLDIR

# vim: ts=8:sw=4:tw=80:et:
