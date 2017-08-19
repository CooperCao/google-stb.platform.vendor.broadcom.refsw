"""Code with special knowledge of firmware image layout."""

from __future__ import print_function
from __future__ import unicode_literals

import hashlib
import os

import lib.util

FW_FILENAMES = set(['rtecdc.dis'])


class Image(object):

    """Represent a single firmware image directory."""

    def __init__(self, filenames, path):
        self.sha1_ = None
        if set(filenames) & FW_FILENAMES:
            self.path = path
            abspath = os.path.abspath(self.path)
            self.name = os.path.join(
                os.path.basename(os.path.dirname(abspath)),
                os.path.basename(abspath))
        else:
            self.path = self.name = None

    @property
    def sha1(self):
        """Return the unique hash for this FW image."""
        if self.sha1_ is None:
            lib.util.note('hashing %s', self.path, vl=3)
            sha_1 = hashlib.sha1()
            for fn in sorted(FW_FILENAMES):
                with open(os.path.join(self.path, fn)) as f:
                    sha_1.update(f.read())
            self.sha1_ = sha_1.hexdigest()
        return self.sha1_

    def __eq__(self, other):
        return self.sha1 == other.sha1

    def __ne__(self, other):
        return not self.__eq__(other)

    def __str__(self):
        return self.name

    def __repr__(self):
        return self.name


def compare(lpath, rpath, forgive=None, vl=1):
    """
    Compare the firmware images in two build paths.

    The set of images to be checked is derived by inspecting the left
    hand path (lpath).  Therefore, order matters and the lpath should
    be considered the control path.
    """

    eq = True

    # Find all fw images in the left path.
    lib.util.note('looking for images in %s ...', lpath, vl=2)
    lfw = {}
    for parent, _, fnames in os.walk(lpath):
        image = Image(fnames, parent)
        if image.name:
            lfw[image.name] = image

    # Find all fw images in the right path.
    lib.util.note('looking for images in %s ...', rpath, vl=2)
    rfw = {}
    for parent, _, fnames in os.walk(rpath):
        image = Image(fnames, parent)
        if image.name:
            rfw[image.name] = image

    # Compare images present in both locations.
    lib.util.note('intersection="%s"', ' '.join(sorted(lfw)), vl=3)
    for name in sorted(lfw):
        if name in rfw:
            if lfw[name] == rfw[name]:
                lib.util.note('identical: %s', name, vl=vl)
            else:
                lib.util.error('DIFFERENT: %s', name, vl=0)
                if forgive:
                    if not any(text in name for text in forgive):
                        eq = False
                else:
                    eq = False
        else:
            lib.util.warn('%s not in %s', name, rpath)

    return 0 if eq else 1

# vim: ts=8:sw=4:tw=80:et:
