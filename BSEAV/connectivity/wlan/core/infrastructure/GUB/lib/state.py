"""Preserve state across platform hops via environment variables."""

from __future__ import print_function
from __future__ import unicode_literals

import getpass
import os
import socket

# In order to avoid import loops, this module should
# not import any other non-core modules.
import lib.times

# Time of *original* invocation. For builds, that means that
# if a build is requested at time A on host X but LSF doesn't
# let it start on host Y until time B, this will still refer
# to time A.
AT_EV = 'GUB_AT'

# NFS base directory for standard builds.
BASE_EV = 'GUB_BASE'

# Username of original requester.
BY_EV = 'GUB_BY'

# GUB depth (number of times it's been invoked).
DEPTH_EV = 'GUB_DEPTH'

# Host and dir originating the current request.
FROM_EV = 'GUB_FROM'

# Boolean set when we know we're on the final build host.
LAST_HOP_EV = 'GUB_LAST_HOP'

# Default base for standard builds.
SWBUILD = '/projects/hnd_swbuild'

# Subdirectory for test builds.
TESTDIR = 'TEST'

# Subdirectory for private builds.
USERDIR = 'USERS'

# The set of transmitted state environment variables.
STATE_EVS = (AT_EV, BASE_EV, BY_EV, DEPTH_EV, FROM_EV)


class State(object):

    """Preserve state across platform hops via environment variables."""

    def __init__(self):
        if AT_EV in os.environ:
            self.at = os.environ[AT_EV]
        else:
            self.at = lib.times.timeref(None, bracket=False)
            os.environ[AT_EV] = self.at

        os.environ[BASE_EV] = os.environ.get(BASE_EV, SWBUILD)
        if os.environ[BASE_EV] == 'None':
            self.base = None
        else:
            self.base = os.environ[BASE_EV]

        self.by = os.environ.get(BY_EV, getpass.getuser())
        os.environ[BY_EV] = self.by

        self.depth = int(os.environ.get(DEPTH_EV, '0')) + 1
        os.environ[DEPTH_EV] = '%d' % self.depth

        if FROM_EV in os.environ:
            self.frm = os.environ[FROM_EV]
        else:
            self.frm = ':'.join([socket.getfqdn(), os.getcwd()])
            os.environ[FROM_EV] = self.frm

        self.in_last_hop = os.environ.get(LAST_HOP_EV) == 'True'

        assert self.depth < 10, 'EXEC LOOP!'

    def reset(self):
        """Clear any state accumulated in environment."""
        for ev in STATE_EVS:
            os.environ.pop(ev, None)

        self.at = None
        self.base = SWBUILD
        self.by = getpass.getuser()
        self.depth = 0
        self.frm = None

    def add2base(self, *subdirs):
        """Append a relative path to the standard build base dir."""
        if self.depth == 1 and self.base:
            self.base = os.path.join(self.base, *subdirs)
            os.environ[BASE_EV] = self.base

    def to_test_area(self):
        """Append "/TEST" to the current base dir."""
        if self.base and not self.is_testbase() and not self.is_userbase():
            self.add2base(TESTDIR)

    def to_user_area(self):
        """Append "/USERS/<user>" to the current base dir."""
        if self.base and USERDIR not in self.base:
            self.add2base(USERDIR, self.by)

    def is_stdbase(self):
        """Return True iff the current build base dir is the standard one."""
        return self.base == self.stdbase

    def is_testbase(self):
        """Return True iff the current build base dir is in TEST."""
        return self.base and TESTDIR in self.base

    def is_userbase(self):
        """Return True iff the current build base dir is in USERS."""
        return self.base and USERDIR in self.base

    @property
    def stdbase(self):  # pylint: disable=no-self-use
        """Return the standard base dir."""
        return SWBUILD

    @property
    def bases(self):
        """Return a list of build base dirs to search for metadata."""
        baselist = []
        if self.base:
            baselist.append(self.base)
        if SWBUILD not in baselist:
            baselist.append(SWBUILD)

        return baselist

    def as_exports(self, to_last_hop=False):
        """Return the current state as an exporting command prefix."""
        exports = []
        if self.depth:
            exports.extend([
                'env',
                '%s=%s' % (AT_EV, self.at),
                '%s=%s' % (BASE_EV, self.base),
                '%s=%s' % (BY_EV, self.by),
                '%s=%s' % (DEPTH_EV, self.depth),
                '%s="%s"' % (FROM_EV, self.frm),
            ])
        if 'GUB_DIR' in os.environ:
            exports.append('GUB_DIR=%s' % os.environ['GUB_DIR'])
        if to_last_hop:
            exports.append('%s=True' % LAST_HOP_EV)
        return exports

    def __repr__(self):
        rpr = ''
        for ev in STATE_EVS:
            rpr += '%s=%s\n' % (ev, os.environ.get(ev))
        return rpr

# vim: ts=8:sw=4:tw=80:et:
