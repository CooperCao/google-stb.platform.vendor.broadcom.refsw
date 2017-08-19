"""Handle logging of stdout/stderr from child processes."""

from __future__ import print_function
from __future__ import unicode_literals

import os
import sys
import time

import lib.util

START_TIME = time.time()


class Watcher(object):

    """Handle logging of stdout/stderr from child processes."""

    STATUS = '_PARENT_STATUS_'

    def __init__(self, full_stdout=False, summary=None,
                 timestamp=False, logs=None):
        self.status = 0
        self.started = time.time()
        self.outfd = os.dup(sys.stdout.fileno())
        self.errfd = os.dup(sys.stderr.fileno())
        rfd, wfd = os.pipe()
        self.pid = os.fork()
        if self.pid:
            # parent
            os.close(rfd)
            os.dup2(wfd, sys.stdout.fileno())
            os.dup2(wfd, sys.stderr.fileno())
        else:
            # child
            os.close(wfd)
            os.dup2(rfd, sys.stdin.fileno())
            cmd = lib.util.xscript('teetime.py')
            cmd.extend(['-t', '%f' % START_TIME])
            if not full_stdout:
                cmd.extend(['-q'])
            if summary:
                cmd.extend(['-s', summary])
            if timestamp:
                cmd.extend(['-d'])
            if logs:
                cmd.extend(logs)
            sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)
            lib.util.xtrace(cmd, vl=3)
            try:
                os.execvp(cmd[0], cmd)
            except OSError as e:
                raise OSError('%s: %s' % (e, cmd[0]))

    @property
    def seconds(self):
        """Return a delta in fractional seconds since build start time."""
        return int(time.time() - self.started)

    def __del__(self):
        sys.stdout.write('%s=%d\n' % (Watcher.STATUS, self.status))
        sys.stdout.flush()
        sys.stderr.flush()
        os.waitpid(self.pid, 0)
        os.dup2(self.outfd, sys.stdout.fileno())
        os.dup2(self.errfd, sys.stderr.fileno())

# vim: ts=8:sw=4:tw=80:et:
