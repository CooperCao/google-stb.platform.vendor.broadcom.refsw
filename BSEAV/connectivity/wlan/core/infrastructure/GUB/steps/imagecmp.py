"""
Contains the ImageCmpStep class which compares firmware images.

The linux-olympic-dongle-src build should create firmware images which
are bit-for-bit identical to those created by hndrte-dongle.wl. This
step verifies that this is really the case, but only for tag builds
since dynamic branch builds would not expect to be identical anyway.
"""

from __future__ import print_function
from __future__ import unicode_literals

import os

import lib.bld
import lib.fw
import lib.util
import lib.wip

from lib.consts import HDW

import steps.base


class ImageCmpStep(steps.base.Step):

    """Compare locally built FW images vs those of HDW on the same tag."""

    def __init__(self, *args, **kwargs):
        steps.base.Step.__init__(self, *args, **kwargs)

    def run(self):
        """Conventional entry point for running this step."""
        if not self.pubwd:
            return

        # Not all branches/tags will have the right circumstances for
        # comparison. Skip those which don't.
        lpath = os.path.join('package_check', 'build/dongle')
        if not os.path.isdir(lpath):
            lib.util.note('no "%s" dir, skip image comparison' % lpath)
            return

        # Find the latest matching HDW build for comparison.
        # On tags we'll accept any build but branch builds must
        # be from the same reference day.
        hdw = lib.bld.lastpath(
            os.path.dirname(self.pubwd.replace(self.bldable, HDW)),
            self.reftime, allow0=True, anyday=lib.scm.is_static(self.tag))[0]

        # Stable means that LODS and HDW build from the same version.
        stable = self.is_stable()

        # Require the comparison only for stable builds.
        if not hdw:
            if stable:
                lib.util.error('no build of %s to compare with' % HDW)
                self.passed = False
            else:
                lib.util.note('no build of %s, skip image comparison' % HDW)
            return

        # Don't proceed until the HDW build for comparison is finished.
        msg = 'waiting for %s ...' % hdw
        hdwstat = lib.wip.wait(hdw, msg=msg)

        rpath = os.path.join(hdw, 'build/dongle')
        if os.path.isdir(rpath):
            lib.util.note('comparing against %s' % hdw)
        else:
            skip = 'skip image comparison'
            if hdwstat:
                lib.util.error('no "%s" dir, %s' % (rpath, skip))
            elif hdwstat is None:
                lib.util.error('gave up waiting for "%s", %s' % (hdw, skip))
            else:
                lib.util.error('failure from "%s", %s' % (hdw, skip))
            self.passed = not stable
            return

        # Do the actual compare. Ram images are checked but do not fail
        # the build if different.
        eq = lib.fw.compare(lpath, rpath, forgive=('-ram'), vl=0) == 0

        # Don't insist on identical images in unstable builds since a
        # legitimate version skew may have occurred.
        if stable:
            self.passed = eq

# vim: ts=8:sw=4:tw=80:et:
