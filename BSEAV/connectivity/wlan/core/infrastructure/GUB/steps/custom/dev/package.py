"""Manage the packaging part of a dev build."""

from __future__ import print_function
from __future__ import unicode_literals

import steps.custom.base


class DevPackageStep(steps.custom.base.BrandStep):

    """Manage the packaging part of a dev build."""

    def __init__(self, *args, **kwargs):
        steps.custom.base.BrandStep.__init__(self, *args, **kwargs)

    def run(self):
        """Conventional entry point for running this step."""
        print('PACKAGE [%s]' % self)

# vim: ts=8:sw=4:tw=80:et:
