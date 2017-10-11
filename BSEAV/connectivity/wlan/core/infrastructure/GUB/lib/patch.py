"""Functions for diffing and patching"""

from __future__ import print_function
from __future__ import unicode_literals

import glob
import os
import re

import lib.opts
import lib.scm
import lib.util


def makepatch(pfile, checkout):
    """Create a patch by running some variant of diff."""
    cmd = lib.util.xscript() + ['scm', 'diff']

    # This is needed for files added with history (from other branches).
    # It requires svn 1.7 but all interactive users should have that.
    cmd.append('--svn-option=--show-copies-as-adds')

    # This requires svn 1.8+ which is our standard nowadays though
    # unfortunately some platforms may not be there yet.
    # Even though we recommend svn 1.8, we only enforce 1.7 or
    # above in precommit, so the following should be turned on
    # when 1.8 is required.
    # Note: the previous version (r652351) implemented a fallback
    # but that was reverted because it's better to encourage use
    # of 1.8 than to support 1.7 indefinitely.
    # cmd.append('--svn-option=--ignore-properties')

    lib.util.execute(cmd, cwd=checkout, stdout=pfile, check=True)


def applypatch(patch, dig=0, prompt=False):
    """Apply specified patch while filtering out non-applicable files."""
    rc = 0

    # Windows machines may not have "patch" but they do have git.
    # If we could assume svn 1.7+ we could use "svn patch" too but
    # git has a good patcher which is available everywhere.
    if lib.util.path_whence('git'):
        patchcmd = ['git', 'apply', '-v', '-p0', '-', '--include']
        if lib.opts.DRY_RUN:
            patchcmd.insert(2, '--check')
    else:
        patchcmd = ['patch', '-p0']
        if lib.opts.DRY_RUN:
            patchcmd.append('--dry-run')

    def patch1(path, diff, dig=0):
        """Run the patch command and feed it a single file patch on stdin."""
        # Look N levels below if the file's target directory isn't
        # present at this level.
        if dig and not os.path.isabs(path):
            parent = os.path.dirname(path)
            while dig and not os.path.isdir(parent):
                dig -= 1
                parent = os.path.join('*', parent)
                matches = glob.glob(parent)
                if matches:
                    rc = 0
                    for match in matches:
                        mpath = os.path.join(match, os.path.basename(path))
                        # Must tweak the diff to refer to the found file.
                        xdiff = []
                        for line in diff[0:2]:
                            s1 = line.split(' ', 1)
                            s2 = s1[1].split('\t', 1)
                            xdiff.append('%s %s\t%s' % (s1[0], mpath, s2[1]))
                        rc += patch1(mpath, xdiff + diff[2:], dig=0)
                    return rc

        # We apply the patch only if (a) there's a file there to patch
        # or (b) the patch is a new file addition (all + lines).
        edits = [ln for ln in diff[3:] if ln[0] != '+']
        if os.path.exists(path):
            if not edits:
                lib.util.warn('new file "%s" conflicts with existing' % path)
                return 2
        elif edits:
            lib.util.warn('no file "%s" to patch' % path)
            return 0

        if prompt:
            response = raw_input('Patch %s? ' % path)
            if not response or response.lstrip().lower()[0] != 'y':
                return 0

        # Special case to handle patches containing an svn keyword.
        # The diff is modified to refer to the unexpanded version.
        # This is harder with patches containing non-ascii text so
        # we punt on those for now.
        try:
            if os.path.isfile(path) and os.path.getsize(path):
                text = open(path, 'r').read()
                keywords = ['Id']
                for keyword in keywords:
                    kw = '$%s$' % keyword
                    if kw in ''.join(diff):
                        pat = r'\$%s:.*\$' % keyword
                        unex = re.findall(pat, text)
                        if unex:
                            diff = [s.replace(kw, unex[0]) for s in diff]
        except UnicodeDecodeError as e:
            lib.util.warn(str(e))

        cmd = patchcmd + [path]
        proc = lib.util.execute(cmd, stdin=lib.util.PIPE)
        for line in diff:
            proc.stdin.write(line)
        proc.stdin.close()
        return proc.wait()

    # State machine to collect one diff at a time and maybe apply it.
    # Encoding gets a little tricky because we want to leave the original
    # unmodified but internal comparisons must use UTF-8.
    diff = path = None
    skipping = False
    for line in open(patch, 'r'):
        orig = line
        line = line.decode('utf-8', 'ignore')
        if line.startswith('Index: ') or \
           line.startswith('=====') or \
           line.startswith('diff -'):
            skipping = False
        elif line.startswith('--- '):
            if diff and patch1(path, diff, dig=dig):
                rc = 2
            path = line.split()[1]
            diff = [orig]
        elif line.startswith('Cannot display: '):
            # Ignore binary files completely.
            if diff and patch1(path, diff, dig=dig):
                rc = 2
            skipping = True
            path = diff = None
        elif skipping:
            pass
        elif line.startswith('Property changes on: '):
            # We don't care about svn-specific property changes.
            skipping = True
            if diff and diff[-1].decode('utf-8', 'ignore') == '\n':
                diff.pop()
            if len(diff) == 2:
                diff = None
        elif diff:
            diff.append(orig)

    if diff and patch1(path, diff, dig=dig):
        rc = 2

    return rc

# vim: ts=8:sw=4:tw=80:et:
