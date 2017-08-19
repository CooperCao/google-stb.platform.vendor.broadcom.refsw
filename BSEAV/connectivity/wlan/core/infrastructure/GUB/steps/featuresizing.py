"""
Contains the Sizing class which kicks off analysis of FW size growth.

"""

from __future__ import print_function
from __future__ import unicode_literals

import os

import lib.fsizing
import lib.scm
import lib.util

import steps.base

BRANCH_TOKEN = '_BRANCH_'


class FeatureSizingStep(steps.base.Step):

    """Kick off analysis of firmware size growth"""

    def __init__(self, *args, **kwargs):
        steps.base.Step.__init__(self, *args, **kwargs)

    def run(self):
        """Conventional entry point for running this step."""
        # Run a sizing analysis if (a) this build is on a
        # static tag and its first branch ancestor is enabled
        # in the config file or (b) this is a nightly build
        # on a branch which is enabled in the config file.
        enabled = self.build.cfg.get('size_tags_from', {}).values()
        if self.build.nightly:
            branch = self.tag
        elif lib.scm.is_static(self.tag):
            tpath = self.tag
            while (tpath and BRANCH_TOKEN not in tpath and
                   os.path.basename(tpath) not in enabled):
                tpath = lib.scm.svn_tag_parent(tpath, None)[0]
            branch = (os.path.basename(tpath) if tpath else tpath)
        else:
            lib.util.note('sizing analysis on branches for .0 builds only')
            branch = None

        # Quit here if it's not enabled.
        if not branch:
            return
        elif branch not in enabled:
            lib.util.note('%s not configured for sizing analysis' % branch)
            return

        # Create the sizing dir with mode 775 so the EC user can write to it.
        # WAR: The sizing folder must be created in both the bldwd (local)
        # and pubwd (NFS) locations because it has to be group writable.
        # If we created it only in the bldwd it could be made read-only
        # during the copy to pubwd. If we created it only in the pubwd
        # it would be removed at build end as part of the cleanup process.
        # But if we create it in both places it gets left alone.
        pubsize = os.path.join(self.pubwd, lib.fsizing.SIZING_DIR)
        lib.util.note('creating sizing directory %s' % pubsize)
        lib.util.mkdir(pubsize)
        lib.util.chmod(pubsize, 0o775)
        lib.util.mkdir(os.path.join(self.bldwd, lib.fsizing.SIZING_DIR))

        # Use the library sizing function to trigger the actual work.
        if lib.fsizing.run(self.pubwd, branch) != 0:
            self.passed = False

# vim: ts=8:sw=4:tw=80:et:
