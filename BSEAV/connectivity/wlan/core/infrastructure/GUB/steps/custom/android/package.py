"""Manage the packaging part of an Android build."""

from __future__ import print_function
from __future__ import unicode_literals

import steps.custom.base


class AndroidPackageStep(steps.custom.base.BrandStep):

    """Manage the packaging part of an Android build."""

    def __init__(self, *args, **kwargs):
        steps.custom.base.BrandStep.__init__(self, *args, **kwargs)

    def run(self):  # pylint: disable=no-self-use
        """Conventional entry point for running this step."""
        print('PACKAGE')

# vim: ts=8:sw=4:tw=80:et:
