"""Contains the DefaultStep class which runs shell commands."""

from __future__ import print_function
from __future__ import unicode_literals

import lib.util

import steps.base


class DefaultStep(steps.base.Step):

    """Run a sequence of shell commands or invoke other step classes."""

    def __init__(self, *args, **kwargs):
        """Boilerplate initializer."""
        steps.base.Step.__init__(self, *args, **kwargs)

    def run(self):
        """Invoke a sequence of tasks as listed in the config file."""
        tasks = self.cfg.get('tasks')
        lib.util.assert_(tasks, 'no tasks specified')
        for k in tasks:
            task = tasks[k]
            if isinstance(task, basestring):
                self.execute(task)
            else:
                stepobj = steps.base.Step.lookup(task, None,
                                                 self.build, incr=False)
                stepobj.invoke(None)

            if not self.passed:
                break

# vim: ts=8:sw=4:tw=80:et:
