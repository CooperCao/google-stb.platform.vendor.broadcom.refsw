"""Track files used (prereqs) and generated (targets)."""

from __future__ import print_function
from __future__ import unicode_literals

import os
import time

# Look for the fast C-based implementation first.
import yaml
try:
    from yaml import CDumper as Dumper
except ImportError:
    from yaml import Dumper

INTERMEDIATES = 'INTERMEDIATES'
POST_COUNT = 'POST_COUNT'
PRE_COUNT = 'PRE_COUNT'
PREREQS = 'PREREQS'
REFTIME = 'REFTIME'
TERMINALS = 'TERMINALS'
UNUSED = 'UNUSED'


class AtimeAudit(object):

    """Track files used (prereqs) and generated (targets)."""

    def __init__(self, indir, exclude=None, notes=None):
        self.indir = indir
        self.exclude = (set(exclude) if exclude else set())
        self.exclude.update(['.svn', '.git'])
        self.notes = notes
        self.prereqs = {}
        self.intermediates = {}
        self.terminals = {}
        self.targets = {}
        self.unused = {}
        self.reftime = None
        self.pre_existing = None

    def get_reftime(self):
        """Derive a unique file reference time and prepare for the build.

        Different filesystems have different granularities for time
        stamps. For instance, ext3 records one-second granularity while
        ext4 records nanoseconds. Regardless of host filesystem, this
        method guarantees to return a timestamp value newer than any
        file previously accessed within the same filesystem and same
        thread, and no newer than any timestamp created subsequently.

        """

        ref_fname = os.path.join(self.indir, '.audit-ref.tmp')

        def get_time_past(previous):
            """Return a granular file time past the previous time."""
            this_mtime = this_atime = 0
            while True:
                with open(ref_fname, 'w+') as f:
                    stats = os.fstat(f.fileno())
                    this_mtime = stats.st_mtime
                    this_atime = stats.st_atime
                os.remove(ref_fname)
                if this_mtime > previous:
                    break
                time.sleep(0.1)
            return this_mtime, this_atime

        mtime1, atime1 = get_time_past(0)
        mtime2, atime2 = get_time_past(mtime1)
        if atime2 > atime1:
            return mtime2
        else:
            return -1

    def get_existing(self):
        """Derive the set of files which exist prior to the build.

        There are some builds which touch their prerequisites,
        causing them to look like targets. To protect against
        that we use the belt-and-suspenders approach of checking
        against a list of files which predated the build.

        """

        existing = set()
        for parent, dnames, fnames in os.walk(self.indir):
            dnames[:] = (dn for dn in dnames if dn not in self.exclude)
            for fn in fnames:
                rpath = os.path.relpath(os.path.join(parent, fn), self.indir)
                existing.add(rpath)
        return existing

    def start(self):
        """Start the build audit."""
        self.reftime = self.get_reftime()
        self.pre_existing = self.get_existing()

    def finish(self, path):
        """End the build audit."""
        # Note: we can't use os.walk() here.
        # It has a way of updating symlink atimes.
        def visit(data, parent, files):  # pylint: disable=unused-argument
            """Callback function for os.path.walk()."""
            if os.path.basename(parent) not in self.exclude:
                for fn in files:
                    path = os.path.join(parent, fn)
                    if not os.path.isdir(path):
                        rpath = os.path.relpath(path, self.indir)
                        stats = os.lstat(path)
                        adelta = stats.st_atime - self.reftime
                        mdelta = stats.st_mtime - self.reftime
                        if mdelta >= 0:
                            if adelta >= 0 and rpath in self.pre_existing:
                                self.prereqs[rpath] = 'P'
                            elif adelta > mdelta:
                                self.intermediates[rpath] = 'I'
                            else:
                                self.terminals[rpath] = 'T'
                        elif adelta >= 0 and rpath in self.pre_existing:
                            self.prereqs[rpath] = 'P'
                        else:
                            self.unused[rpath] = 'U'

        os.path.walk(self.indir, visit, None)

        # The union of these two sets is the set of "targets".
        self.targets.update(self.intermediates)
        self.targets.update(self.terminals)

        # Dump the result to a file.
        root = []
        if self.notes:
            for k, v in self.notes.items():
                root.append({k: v})
        root.append({PRE_COUNT: '%s' % len(self.pre_existing)})
        post_count = '%s' % len(self.prereqs) + len(self.intermediates) + \
            len(self.terminals) + len(self.unused)
        root.append({POST_COUNT: post_count})
        refstr = time.ctime(self.reftime)
        root.append({REFTIME: '%f (%s)' % (self.reftime, refstr)})
        db = {}
        db[INTERMEDIATES] = self.intermediates
        db[PREREQS] = self.prereqs
        db[TERMINALS] = self.terminals
        db[UNUSED] = self.unused
        root.append(db)
        with open(path, 'w') as f:
            yaml.dump(root, f, indent=2, default_flow_style=False,
                      Dumper=Dumper)

# vim: ts=8:sw=4:tw=80:et:
