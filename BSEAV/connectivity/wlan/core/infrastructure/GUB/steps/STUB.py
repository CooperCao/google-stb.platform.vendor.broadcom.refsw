"""
Contains the StubStep class from which to start a new step.

Note that step modules must have lower-case names which define classes
using an upper-case variant of the same name. See existing step classes
for examples.

"""

from __future__ import print_function
from __future__ import unicode_literals

import steps.base


class StubStep(steps.base.Step):

    """An empty step class from which to copy."""

    def __init__(self, *args, **kwargs):
        """Boilerplate initializer."""
        steps.base.Step.__init__(self, *args, **kwargs)

    def run(self):
        """Conventional entry point for running this step."""
        pass  # Replace this with your code.

# vim: ts=8:sw=4:tw=80:et:
