"""Functions related to preparing a checkout tree for build."""

from __future__ import print_function
from __future__ import unicode_literals

import os

import lib.scm
import lib.util


def epivers(basedir, envlog=None):
    """Find src/include dirs and generate commands to create epivers.h."""
    cmds = []
    for parent, dnames, _ in os.walk(basedir):
        if 'src' not in dnames:
            continue

        incdir = os.path.join(parent, 'src', 'include')
        if not os.path.exists(os.path.join(incdir, 'epivers.h.in')):
            continue

        # No need to go deeper, there's only one src/include per tree.
        dnames = []

        # In copydir mode we could come through here twice.
        if os.path.exists(os.path.join(incdir, 'epivers.h')):
            continue

        gcinfo = os.path.join(basedir, '.gclient_info')
        if os.path.exists(gcinfo):
            # Always report a tag checkout as such regardless of gcinfo.
            co_from = lib.scm.urlof(os.path.dirname(incdir))
            if co_from and '/tags/' in co_from:
                extra_env = None
            else:
                extra_env = {'GCLIENT_INFO': gcinfo}
        else:
            extra_env = None

        cmd = lib.util.xscript() + ['make', '--gmake']
        if envlog:
            cmd.append('--env-log=' + envlog)
        cmd.extend(['--', '-C', incdir])
        cmds.append((cmd, extra_env))

    return cmds

# vim: ts=8:sw=4:tw=80:et:
