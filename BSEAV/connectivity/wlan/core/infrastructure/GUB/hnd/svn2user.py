"""
Create a buildable copy of $WL_REPO_ROOT
within $WL_REPO_ROOT/users/$USER.

The copy is always created as a subdirectory of your "users" dir.
There is one required argument: the name of that subdir, which will
contain a copy of everything needed to build any branch.

Note that this is not a copy of any particular branch, it's a copy
of nearly the ENTIRE REPOSITORY[*] outside of the users area. This is
required in order to pick up components outside of /proj, and as a
beneficial side effect it means that all branches can be merged, built,
etc. within the copy.

[*] Unfortunately, since we have a few access-controlled (unreadable by
most users) areas distributed randomly within our repo it's necessary to
do iterative copying, stepping around those areas, rather than making
one big copy. This takes some extra time and trouble.  But despite the
multiple copies the net result is atomic, i.e. within a repo all copies
will come from the same revision.

It's suggested that these copies not be named for a given branch since
they aren't really branch-specific. Better to name them something
meaningful related to the project they were created for, the bugfix
developed within them, etc. See examples below.

EXAMPLES:

Copy to $WL_REPO_ROOT/users/$USER/BLAH:

  %(prog)s svn2user BLAH

Build hndrte-dongle-wl on trunk from the above copy:

  %(prog)s build -b hndrte-dongle-wl -t trunk -u BLAH

Copy from revision 574740 to <repo>/users/$USER/router/SWWLAN-80000:

  %(prog)s svn2user -r 574740 -m "from r574740" router/SWWLAN-80000

Do a wl-src checkout into DIR from trunk within the copy:

  %(prog)s -V scm co -u BLAH -t trunk -d DIR wl-src

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os
import sys

import lib.consts
import lib.scm
import lib.times
import lib.util

# Unfortunately it's necessary to do piecemeal copying below
# due to unreadable files and directories being in random places.
# Here are the known unreadable areas as of 6/2015:
SKIPS = [
    'groups/mgmt',
    'groups/software/infrastructure/wlanswpwd.xlsx',
    'components/vendor/screenovate',
]


def parse_cli(prog, alias, subcmds, advanced, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser.add_argument(
        '--for-user', default=lib.consts.USER,
        metavar='USER',
        help="copy to specified user folder (default=users/%(default)s)"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-m', '--message',
        help="a comment for this user copy")
    parser.add_argument(
        '--yes', '--noprompt', action='store_true',
        help="no prompt - proceed directly to commit")
    parser.add_argument(
        '-r', '--reftime',
        help="time cutoff or svn revision")
    parser.add_argument(
        '-s', '--svn',
        help="override default svn base URL")
    parser.add_argument(
        'path', help="path to create within user area")
    parser.set_defaults(func=call)


def call(opts, _):
    """Standard subcommand entry point."""
    # An svn copy this global should be done in the master repo
    # and allowed to sync outward to the mirrors. So we always
    # operate on the SJ reference repo regardless of site.
    lib.consts.WL_REPO_ROOT = lib.consts.WL_REPO_ROOT_MASTER
    repo = (opts.svn if opts.svn else lib.consts.WL_REPO_ROOT)

    # Copy as of this time or revision (default=now).
    reftime = lib.times.timeref(opts.reftime)

    # This is where the new copy will land. Make sure it doesn't already exist.
    newpath = lib.util.mkpath('users', opts.for_user, opts.path)
    newurl = lib.util.mkpath(repo, newpath)
    if lib.scm.svn_ls(newurl, stderr=open(os.devnull, 'w')):
        lib.util.die('svn path "%s" exists' % newurl)

    # Prep the local workspace, which will be removed automatically at exit.
    tmpco = lib.util.tmpdir(prefix='svn2user.', vl=2)
    lib.util.chmod(tmpco, 0o775, vl=3)
    cocmd = lib.scm.svncmd('co', '-q', '-r', reftime, '--depth=immediates')
    cocmd.extend([repo, tmpco])
    lib.util.execute(cocmd, check=True, interactive=True, vl=2)

    # Operate within the context of the workspace from here.
    lib.util.chdir(tmpco, vl=2)

    # This uses a pretty tricky recursive algorithm. It's designed to
    # copy as much as possible in one fell swoop but sometimes must
    # recurse into the next level and copy in smaller chunks to avoid
    # areas which can't or shouldn't be copied.
    def svn_cp(src, dst, skips=None):
        """Do an svn cp in a local workspace, avoiding unreadable areas."""
        if not skips:
            lib.scm.svn_updir(src, reftime=reftime)
            lib.scm.svn_mkdir(os.path.dirname(dst))
            cpcmd = lib.scm.svncmd('cp', '-q', src, dst)
            lib.util.execute(cpcmd, check=True, vl=1)
            return

        if src in skips:
            return

        partmatch = [p for p in skips if p.startswith(src)]
        if partmatch:
            url = lib.util.mkpath(repo, src)
            subdirs = lib.scm.svn_ls(url, reftime=reftime)
            for subdir in subdirs:
                subsrc = os.path.join(src, subdir)
                subdst = os.path.join(dst, subdir)
                svn_cp(subsrc, subdst, partmatch)
        else:
            svn_cp(src, dst, None)

    # If the user's dir is already created, update it in the workspace.
    # If not, create it.
    lib.scm.svn_updir(newpath)
    lib.scm.svn_mkdir(newpath)

    # Stage all copies in the local workspace.
    fromdirs = ('components', 'groups', 'proj')
    for fromdir in fromdirs:
        todir = os.path.join(newpath, fromdir)
        svn_cp(fromdir, todir, SKIPS)

    # To date nothing has been changed in the repo. Last chance to quit.
    if opts.dry_run:
        sys.exit()
    elif not opts.yes:
        ok = 'OK to copy %s@%s to .../%s? [yN]: ' % (repo, reftime, newpath)
        response = raw_input(ok)
        if not response or response.lower()[0] != 'y':
            sys.exit()

    # Commit the local workspace with a single atomic revision.
    msgfile = '.comment'
    with open(msgfile, 'w') as f:
        f.write('Copied by svn2user from %s to %s:\n' %
                (lib.util.rmchars(reftime, '{}'), opts.path))
        if opts.message:
            f.write(opts.message + '\n')
    cicmd = lib.scm.svncmd('ci', '-F', msgfile, '.')
    lib.util.execute(cicmd, check=True, vl=2)

    sys.stderr.write(
        'Warning: DEPS files in .../%s are unmodified!\n'
        'DEPS-based checkouts may require manual DEPS changes '
        'if using "hnd gclient sync".\nUse "hnd scm co -u %s ..." and '
        '"hnd build -u %s ..." to avoid needing to change them.\n' % (
            newpath, opts.path, opts.path)
    )

# vim: ts=8:sw=4:tw=80:et:
