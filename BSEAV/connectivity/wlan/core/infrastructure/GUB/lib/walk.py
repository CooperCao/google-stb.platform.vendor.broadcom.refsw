"""A replacement for os.walk() with some enhancements."""

from __future__ import print_function
from __future__ import unicode_literals

import os
import stat


def os_walk(top, topdown=True, onerror=None, followlinks=False):
    """A variant of os.walk() with enhanced error handling.

    This version is identical to os.walk() except:

    1. If an unreadable/untraversable directory is encountered and
       onerror is set to the special string value 'fix', it will
       chmod u+rx the directory it couldn't read and then retry.
       If that doesn't work it reverts to original os.walk() behavior
       which is to silently skip the inaccessible subtree.

    """

    islink, join, isdir = os.path.islink, os.path.join, os.path.isdir

    # We may not have read permission for top, in which case we can't
    # get a list of the files the directory contains.
    # BRCM Local mod which attempts to chmod u+rx top rather than
    # silently skipping it or aborting.
    try:
        names = os.listdir(top)
    except os.error as err:
        if onerror == 'fix':
            try:
                stats = os.stat(err.filename)
                upmode = stats.st_mode | stat.S_IRUSR | stat.S_IXUSR
                os.chmod(err.filename, upmode)
                names = os.listdir(top)
            except os.error:
                return
        elif onerror is not None:
            onerror(err)
            return
        else:
            return

    dirs, nondirs = [], []
    for name in names:
        if isdir(join(top, name)):
            dirs.append(name)
        else:
            nondirs.append(name)

    if topdown:
        yield top, dirs, nondirs
    for name in dirs:
        new_path = join(top, name)
        if followlinks or not islink(new_path):
            for x in os_walk(new_path, topdown, onerror, followlinks):
                yield x
    if not topdown:
        yield top, dirs, nondirs

# vim: ts=8:sw=4:tw=80:et:
