"""Handle disk partition selection based on available space."""

from __future__ import print_function
from __future__ import unicode_literals

import collections
import os
import sys
import time

import lib.consts
import lib.scm
import lib.util

NtupleDiskUsage = collections.namedtuple('usage', 'total used free')


def disk_usage(path):
    """Return disk usage statistics about the given path.

    Returned value is a named tuple with attributes 'total', 'used' and
    'free', which are the amount of total, used and free space, in bytes.

    This is a backport from Python 3.3, along with hacks to make it
    work well enough on Windows. There's also a retry-with-flush due
    to NFS cache issues with newly created dirs.
    """

    def disk_usage_(path):
        """Do the real work for containing function."""

        if sys.platform.startswith('win'):
            import ctypes
            total_bytes = ctypes.c_ulonglong(0)
            free_bytes = ctypes.c_ulonglong(0)
            ctypes.windll.kernel32.GetDiskFreeSpaceExA(
                ctypes.c_wchar_p(path), None,
                ctypes.pointer(total_bytes),
                ctypes.pointer(free_bytes))
            free = free_bytes.value
            total = total_bytes.value
            used = total - free
        else:
            st = os.statvfs(path)
            free = st.f_bavail * st.f_frsize
            total = st.f_blocks * st.f_frsize
            used = (st.f_blocks - st.f_bfree) * st.f_frsize
        return NtupleDiskUsage(total, used, free)

    try:
        return disk_usage_(path)
    except EnvironmentError:
        lib.util.nfsflush(path, up=True)
        return disk_usage_(path)


def platpath(part, plat, tag):
    """Work out the conventional subdir name under the partition."""
    # I don't know the logic or history behind this subdir, just going with it.
    ext = '_'.join([plat, part.split('swbuild_', 1)[1]])
    return os.path.join(part, ext, tag)


def status(partitions):
    """Return a dict mapping partitions to their "df" data."""
    freemap = {}
    for partition in partitions:
        try:
            freemap[partition] = disk_usage(partition)
        except Exception as e:
            lib.util.error(str(e))
    return freemap


def findbest(cfgroot, plat_name, tag):
    """Check candidate partitions and find the one with most free space.

    Note that the algorithm doesn't look at *percentage* free - it's
    a simple sort by *bytes* free. We use the disk that has the most
    free bytes until it doesn't.

    As a special case, if an existing link target of the
    appropriate name is encountered, abort the search and link
    to the existing target in order to avoid stranded builds.
    """

    # REL builds are backed up, TOB builds are not.
    if lib.scm.is_static(tag):
        partitions = cfgroot.get('partitions').get('static').values()
    else:
        partitions = cfgroot.get('partitions').get('dynamic').values()

    best = None
    freemap = {}
    for partition in partitions:
        if os.path.isfile(os.path.join(partition, 'NO_BUILDS')):
            continue

        # Reattach to a pre-existing dir if present.
        target = platpath(partition, plat_name, tag)
        if os.path.exists(target):
            best = partition
            break

        # Otherwise pick the partition with most free space.
        usage = disk_usage(partition)
        freemap[partition] = usage.free

    if not best:
        best = max(freemap, key=lambda x: freemap[x])

    return best


def link2space(cfgroot, platforms, tag):
    """Create tag dir as a symlink into the emptiest partition."""
    partition = None
    for plat in platforms:
        # I'd prefer to establish a new /projects/hnd_swbuild/builds
        # dir and put all platforms there but that would probably need
        # yet more symlinks to be backward compatible.

        base_dir = lib.consts.STATE.base

        # Look for a pre-existing link two different ways. We've
        # seen some automounter weirdness and there's a theory
        # that reading the parent dir might tickle it in a way
        # that a simple stat check wouldn't. Just a WAG.
        # We retry a few times for the same reason.
        # This may all be overkill now that we make links pre-lSF.
        plat_name = 'build_' + plat
        plat_dir = os.path.join(base_dir, plat_name)
        for i in range(10):
            try:
                os.listdir(plat_dir)
            except Exception as e:
                lib.util.warn(str(e))
                lib.util.nfsflush(base_dir, up=True)
                time.sleep(i)
            else:
                break

        linkname = os.path.join(plat_dir, tag)

        if not os.path.exists(linkname):
            # Need to be builder to do this stuff.
            if lib.util.am_builder():
                bcmd = []
            else:
                bcmd = [lib.consts.AS_BUILDER, '--']

            # Protection against dangling symlinks.
            if os.path.islink(linkname):
                cmd = bcmd + ['rm', '-f', linkname]
                lib.util.execute(cmd, check=True, vl=2)

            # Deal with possible race conditions if multiple builds
            # try to create the same link at once.
            try:
                # Find the partition with the most free space.
                if not partition:
                    partition = findbest(cfgroot, plat_name, tag)
                    if not partition:
                        lib.util.die('no build partition found')
                        break

                # Create the appropriate directory in the chosen partition
                # and symlink to it. Try to be robust against creation races.
                target = platpath(partition, plat_name, tag)
                if not os.path.exists(target):
                    cmd = bcmd + ['mkdir', target]
                    try:
                        lib.util.execute(cmd, check=True, vl=2)
                    except lib.util.CalledProcessError:
                        if not os.path.isdir(target):
                            raise

                cmd = bcmd + ['ln', '-sf', target, linkname]
                lib.util.execute(cmd, check=True, vl=2)
            except Exception:
                if not os.path.exists(linkname):
                    raise

# vim: ts=8:sw=4:tw=80:et:
