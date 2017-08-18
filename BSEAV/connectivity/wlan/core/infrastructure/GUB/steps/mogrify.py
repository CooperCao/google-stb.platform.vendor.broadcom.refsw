"""Contains the MogrifyStep class which manages generic mogrification."""

from __future__ import print_function
from __future__ import unicode_literals

import os

import lib.opts
import lib.util

import steps.base


class MogrifyStep(steps.base.Step):

    """Drive mogrification via the xmogrify wrapper script over mogrify.pl."""

    def __init__(self, *args, **kwargs):
        """Boilerplate initializer."""
        steps.base.Step.__init__(self, *args, **kwargs)

    def run(self):
        """Make this step do its stuff."""
        cmd = lib.util.xscript('xmogrify', shell=True)
        if lib.opts.VERBOSITY < 2:
            cmd += ' --quiet'
        elif lib.opts.VERBOSITY > 1:
            cmd += ' -V'
            for _ in range(2, lib.opts.VERBOSITY):
                cmd += 'V'
        flags = self.cfg.get('mogrify_flags', self.cfg.getvar('MOGRIFY_FLAGS'))
        if flags:
            cmd = '%s %s' % (cmd, flags)
        on = self.cfg.get('mogrify_on', self.cfg.getvar('MOGRIFY_ON'))
        if on:
            cmd = '%s --define %s' % (cmd, on)
        off = self.cfg.get('mogrify_off', self.cfg.getvar('MOGRIFY_OFF'))
        if off:
            cmd = '%s --undefine %s' % (cmd, off)
        cmd += ' --'
        for path in self.cfg.getvar('MOGRIFY_PATHS').split():
            if os.path.isdir(path):
                cmd += ' ' + path
        self.execute(cmd)

# vim: ts=8:sw=4:tw=80:et:
