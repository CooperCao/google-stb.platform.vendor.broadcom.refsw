"""Manage the compile-and-link part of an Android build."""

from __future__ import print_function
from __future__ import unicode_literals

import lib.util

import steps.custom.base


class AndroidMakeStep(steps.custom.base.BrandStep):

    """Manage the compile-and-link part of an Android build."""

    def __init__(self, *args, **kwargs):
        steps.custom.base.BrandStep.__init__(self, *args, **kwargs)

    def run(self):
        """Conventional entry point for running this step."""
        step_methods = [
            self.mogrify,
            self.make,
        ]
        for step_method in step_methods:
            if not self.as_step(step_method):
                break

    def mogrify(self):
        """Do the initial mogrification."""
        cmd = """
            %s $[MOGRIFY_FLAGS] --list-files=misc/mogrified.list
                --define $[MOGRIFY_ON] --undefine $[MOGRIFY_OFF]
                -- $[MOGRIFY_PATHS]
        """ % lib.util.xscript('xmogrify', shell=True)
        self.execute(cmd)

    def make(self):  # pylint: disable=no-self-use
        """Do the actual build."""
        print('MAKING')


# vim: ts=8:sw=4:tw=80:et:
