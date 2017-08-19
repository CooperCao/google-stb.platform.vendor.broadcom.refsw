"""Base class for custom steps."""

from __future__ import print_function
from __future__ import unicode_literals

import os

import lib.util

import steps.base


class BrandStep(steps.base.Step):

    """Base class for custom steps."""

    def __init__(self, *args, **kwargs):
        steps.base.Step.__init__(self, *args, **kwargs)

    def as_step(self, func, args=(), stepname=None):
        """Call the supplied function/method as a step."""
        if not stepname:
            stepname = func.__name__.title()

        steps.base.mark_start(stepname, self.build.logfiles)
        func(*args)  # pylint: disable=star-args
        steps.base.mark_end(stepname, self.build.logfiles)

        return self.passed

    def execute(self, cmd, **kwargs):
        """Execute the command after doing gvar substitutions."""
        if steps.base.Step.execute(self, cmd, dosubs=True, **kwargs):
            return True
        else:
            raise lib.util.CalledProcessError(2, self.sub(cmd))

    def basename(self, path):
        """Call the utility function after doing gvar substitutions."""
        return os.path.basename(self.sub(path))

    def cd(self, *args, **kwargs):
        """Call the utility function after doing gvar substitutions."""
        lib.util.chdir(*self.sub(args), **kwargs)  # pylint: disable=star-args

    def chmod(self, *args, **kwargs):
        """Call the utility function after doing gvar substitutions."""
        lib.util.chmod(*self.sub(args), **kwargs)  # pylint: disable=star-args

    def cp(self, src, dst, **kwargs):
        """Call the utility function after doing gvar substitutions."""
        lib.util.cp(self.sub(src), self.sub(dst), **kwargs)

    def echo(self, *args):
        """Print arguments to stdout after doing gvar substitutions."""
        for arg in args:
            print(self.sub(arg))

    def exists(self, path):
        """Call the os function after doing gvar substitutions."""
        return os.path.exists(self.sub(path))

    def mkdir(self, *args, **kwargs):
        """Call the utility function after doing gvar substitutions."""
        lib.util.mkdir(*self.sub(args), **kwargs)  # pylint: disable=star-args

    def mv(self, src, dst, **kwargs):
        """Call the utility function after doing gvar substitutions."""
        lib.util.mv(self.sub(src), self.sub(dst), **kwargs)

    def rm(self, *args, **kwargs):
        """Call the utility function after doing gvar substitutions."""
        lib.util.rm(*self.sub(args), **kwargs)  # pylint: disable=star-args

# vim: ts=8:sw=4:tw=80:et:
