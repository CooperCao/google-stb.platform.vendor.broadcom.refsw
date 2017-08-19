#!/usr/bin/env python2.7
"""
Masquerade a program over ssh from an unsupported platform.

When this program is run via a symlink 'foo' it will exec the real 'foo'
program. If real 'foo' isn't installed locally it will be run via "ssh
xlinux foo ...". The intent is that these links can be set up to provide
a consistent CLI across platforms. For instance while LSF is supported
natively on Linux and Windows it is not on MacOS, so we could install
"bsub", "bjobs", etc. symlinks on MacOS.

"""

from __future__ import print_function
from __future__ import unicode_literals

import os
import sys

PROG = os.path.basename(sys.argv[0])
TOOLDIR = os.path.dirname(os.path.realpath(sys.path[0]))
sys.path.insert(1, TOOLDIR)
# Suppress pep8 warnings here and below. We need to defer
# imports until we've modified sys.path.
import lib.consts  # nopep8
lib.consts.PROG = PROG
lib.consts.TOOLDIR = TOOLDIR

import lib.opts  # nopep8
import lib.util  # nopep8

MARKER = 'SSHMASQ'


def main():
    """Entry point for all uses."""
    if MARKER in os.environ:
        real = None
        paths = lib.util.path_whence(PROG, full=True)
        for path in paths:
            with open(path, 'r') as f:
                if MARKER not in f.read():
                    real = path
                    break

        if not real:
            lib.util.die('no handoff for %s' % PROG)
    else:
        real = PROG

    if MARKER in os.environ:
        cmd = [real] + sys.argv[1:]
    else:
        cmd = lib.consts.SSH_CMD[:]
        cmd.append('-X')
        site = (lib.consts.SITE if lib.consts.SITE else 'sj')
        cmd.append('xlinux.%s.%s' % (site, lib.consts.DOMAIN))
        cmd.append('--')
        cmd.append('%s=1 %s' % (
            MARKER,
            lib.util.cmdline([real] + sys.argv[1:]),
        ))

    lib.util.execute(cmd, execvp=True)
    sys.exit(2)

if __name__ == '__main__':
    main()

# vim: ts=8:sw=4:tw=80:et:
