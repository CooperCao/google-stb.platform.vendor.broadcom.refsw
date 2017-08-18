"""
Contains the ValidateStep class which tests for valid IP tags.

See http://confluence.broadcom.com/display/WLAN/BuildIPValidation.
"""

from __future__ import print_function
from __future__ import unicode_literals

import os

import lib.bld
import lib.util
import lib.val

import steps.base


class ValidateStep(steps.base.Step):

    """IP-validate the built tree's 'release' folder."""

    def __init__(self, *args, **kwargs):
        steps.base.Step.__init__(self, *args, **kwargs)

    def run(self):
        """Conventional entry point for running this step."""

        ipstr = self.getvar(lib.val.IPTAG_MAP)
        if ipstr:
            ipmap = dict([kv.split(':') for kv in ipstr.split()])
            for chkdir, iptags in ipmap.items():
                lib.util.note('validating IP in "%s" against iptags "%s"' %
                              (chkdir, iptags))
                chkdir = os.path.join(self.bldwd, chkdir)
                if not lib.val.checkdir(chkdir, iptags):
                    self.passed = False

        if not self.passed:
            lib.util.error('see %s' % lib.val.WIKI)


# vim: ts=8:sw=4:tw=80:et:
