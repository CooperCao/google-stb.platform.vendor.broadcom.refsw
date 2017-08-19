"""
Remove old nightly builds.

EXAMPLES:

To remove old builds (requires privilege):

    %(prog)s cleanup -t trunk -m ${SCM_LIST}

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import collections
import os
import shutil
import sys
import tempfile
import time

import lib.bld
import lib.consts
import lib.mail
import lib.nfscp
import lib.opts
import lib.results
import lib.scm
import lib.times
import lib.util

BLDFMT = '%Y.%m.%d'
BUILD_SUMMARIES_DAYS_TO_KEEP = 90
KEEP_FILE = '_DO_NOT_DELETE'
MAX_RM_JOBS = 8
STALE_SECS = lib.consts.SECONDS_PER_HOUR * 5


def parse_cli(prog, alias, subcmds, advanced, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    # The policy is really 3 but since it's better to do the cleanup early
    # in the CA day when NFS is less busy we use 4 (resulting in ~3.5).
    parser.add_argument(
        '-d', '--keep-days', type=int, default=4,
        metavar='DAYS',
        help="number of days of good builds to keep (default=%(default)s)")
    parser.add_argument(
        '--extras', action='store_true',
        help="clean up build-related metadata")
    parser.add_argument(
        '-m', '--mailto', default=[], action='append',
        metavar='ADDRESSES',
        help="comma- or space-separated list of addresses, default=$USER")
    parser.add_argument(
        '-o', '--outfile',
        help="save output to this file")
    parser.add_argument(
        '-p', '--rm-parent-dirs', action='store_true',
        help="remove empty parent directories"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-r', '--reftime',
        help="clean up relative to this time (default=<now>)")
    parser.add_argument(
        '--bin-rm', action='store_true',
        help="use actual /bin/rm command, serially"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-t', '--tag', default=[], action='append',
        help="basename of branch/tag/twig/trunk (may be repeated)")
    parser.add_argument(
        '--users', action='store_true',
        help="clean up USERS subtrees too"
        if advanced else argparse.SUPPRESS)
    parser.set_defaults(func=call)


def lsdir(path):
    """Wrapper over os.listdir which tries to fix common perm problems."""
    try:
        names = os.listdir(path)
    except OSError:
        os.chmod(path, 0o775)
        names = os.listdir(path)

    # Ignore Windows shortcuts which tend to be put here for some reason.
    names = [n for n in names if ' ' not in n and not n.endswith('.lnk')]

    return sorted(names)


def cleanup(opts, tags, reftime):
    """Clean up old builds within the standard namespace."""
    rc = 0

    build_base = lib.consts.STATE.base
    assert '/projects/' in build_base

    passed_losers = set()
    failed_losers = set()

    def find_victims(tag_dir):
        """Return a list of builds to be removed."""
        if not os.path.isdir(tag_dir):
            if os.path.exists(tag_dir):
                passed_losers.add(tag_dir)
            return

        try:
            blddirs = lsdir(tag_dir)
        except OSError as e:
            lib.util.error(str(e))
            blddirs = []

        for blddir in blddirs:
            # Delete anything that is not a directory.
            bdir = os.path.join(tag_dir, blddir)
            if not os.path.isdir(bdir):
                failed_losers.add(bdir)
                continue

            try:
                bldnames = lsdir(bdir)
            except OSError as e:
                lib.util.error(str(e))
                continue

            for bldname in bldnames:
                path = os.path.join(bdir, bldname)
                bldday = '.'.join(bldname.split('.')[0:3])
                try:
                    blddate = time.mktime(time.strptime(bldday, BLDFMT))
                    days_old = int(
                        (reftime - blddate) / lib.consts.SECONDS_PER_DAY)
                except ValueError:
                    failed_losers.add(path)
                    continue

                # Use this opportunity to find builds that shouldn't be
                # here even if we don't need to kill them right now.
                if not os.access(path, os.W_OK):
                    try:
                        owner = lib.util.get_owner(path)
                        if lib.util.am_builder():
                            lib.util.warn('owned by %s: %s' % (owner, path))
                    except OSError as e:
                        if os.path.islink(path):
                            lib.util.xtrace('rm -f ' + path, vl=1)
                            if not opts.dry_run:
                                lib.util.rm(path)
                        else:
                            lib.util.error(str(e))
                        continue

                # Overrides default rm behavior.
                if os.path.exists(os.path.join(path, KEEP_FILE)):
                    continue

                # Always clean up dangling symlinks.
                if not os.path.exists(path) and os.path.islink(path):
                    failed_losers.add(path)
                    continue

                # Remove passed builds older than N days.
                # Remove failed builds from all previous days.
                passfile = os.path.join(path, lib.bld.SUCCEEDED_FILE)
                if os.path.exists(passfile):
                    if days_old > opts.keep_days:
                        passed_losers.add(path)
                else:
                    if days_old > 1 or opts.keep_days == 0:
                        failed_losers.add(path)

    if opts.users:
        user_base = os.path.join(build_base, 'USERS')
        if os.path.isdir(user_base):
            try:
                user_dirs = lsdir(user_base)
            except OSError as e:
                lib.util.error(str(e))
                rc = 2
                user_dirs = []

            for user_dir in user_dirs:
                user_dir = os.path.join(user_base, user_dir)
                if os.path.isdir(user_dir):
                    try:
                        names = lsdir(user_dir)
                    except OSError as e:
                        lib.util.error(str(e))
                        rc = 2
                        continue

                    for name in names:
                        if name == 'build_window':
                            continue
                        for tag in tags:
                            find_victims(os.path.join(user_dir, name, tag))
                else:
                    passed_losers.add(user_dir)
        else:
            passed_losers.add(user_base)
    else:
        for name in sorted(os.listdir(build_base)):
            if not name.startswith('build_'):
                continue
            if name == 'build_admin' or name == 'build_window':
                continue
            if name.endswith('Shortcut.lnk') or name.endswith('.tar'):
                continue
            for tag in tags:
                find_victims(os.path.join(build_base, name, tag))

    since = time.time()

    if opts.bin_rm:
        for path in sorted(failed_losers) + sorted(passed_losers):
            if path in passed_losers:
                xstr = '/bin/rm -rf %s (PASSED)' % path
            else:
                xstr = '/bin/rm -fr %s (FAILED)' % path
            if not opts.dry_run:
                xstr += ' +%d' % int(time.time() - since + 0.5)
            lib.util.xtrace(xstr)

            if opts.dry_run:
                continue

            try:
                cmd = ['/bin/rm', '-rf', path]
                if lib.util.execute(cmd, vl=-1) != 0:
                    rc = 2

                if opts.rm_parent_dirs or 'USERS' in path:
                    if not os.path.exists(path):
                        lib.util.rm_empty_parent_dirs(path)
            except Exception:
                rc = 2
    else:
        # Do the actual removals using threaded parallelism for speed.
        # Try to remove one build from each filer in parallel for as long
        # as possible to spread the load. When we see a "long tail" of
        # serial removals from the same filer toward the end in the log,
        # that's the sign of a need to rebalance.

        # Map each build to the filer hosting it. Also derive a reverse map,
        # used in log verbosity, showing the partition each path comes from.
        filers = collections.defaultdict(list)
        path2filer = {}
        mounts = lib.util.mounted_from(sorted(failed_losers | passed_losers))
        for path in mounts:
            filer = mounts[path][0]
            filers[filer].append(path)
            path2filer[path] = ':'.join(mounts[path])

        # Now remove them in clumps of one per filer.
        total = sum([len(l) for l in filers.values()])
        i = 0
        while sum([len(l) for l in filers.values()]) > 0:
            rmlist = []
            for pathlist in filers.values():
                try:
                    rmlist.append(pathlist.pop())
                except IndexError:
                    pass

            for path in rmlist:
                i += 1
                if path in passed_losers:
                    xstr = 'rm -rf %s (PASSED)' % path
                else:
                    xstr = 'rm -fr %s (FAILED)' % path
                xstr += ' [%d=%d/%d]' % (len(rmlist), i, total)
                if not opts.dry_run:
                    xstr += ' +%d' % int(time.time() - since + 0.5)
                xstr += ' {%s}' % path2filer[path]
                lib.util.xtrace(xstr)

            if not opts.dry_run:
                try:
                    rmobj = lib.nfscp.TreeSync(path, None)
                    rmobj.rmtrees(rmlist, jobs=MAX_RM_JOBS)

                    if opts.rm_parent_dirs or 'USERS' in path:
                        if not os.path.exists(path):
                            lib.util.rm_empty_parent_dirs(path)
                except Exception:
                    rc = 2

    print('\nREMOVED %d PASSED and %d FAILED at %s' % (
        len(passed_losers),
        len(failed_losers),
        time.ctime())
    )

    return rc


def call(opts, cfgproxy):
    """Standard subcommand entry point."""
    cfgroot = cfgproxy.parse()

    # Prevent a cascade of uglier errors later.
    if not (opts.dry_run or lib.util.am_builder()):
        lib.util.warn('not builder: switching to --dry-run mode')
        opts.dry_run = True

    # Immediately re-run ourselves under LSF unless asked not to.
    if not lib.lsf.submitted() and not lib.lsf.no_lsf(opts):
        sys.exit(lib.lsf.bsub(sys.argv,
                              app='sw_ondemand_builds',
                              foreground=opts.lsf_foreground,
                              jobname='BUILD.CLEANUP',
                              queue=cfgroot.getvar('LSF_QUEUE'),
                              resource=cfgroot.getvar('LSF_RESOURCE')))

    # It's important to establish a reference time in the appropriate
    # build day because (a) LSF could delay starting this job till
    # past midnight or (b) the job could run into the next day due to
    # having a lot to do. Either of these might result in removing the
    # current day's builds without this reftime.
    if opts.reftime:
        reftime = lib.times.iso2seconds(opts.reftime)
    else:
        reftime = lib.consts.INVOKE_TIME

    # Set stdout to line buffering.
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)

    tags = []
    for t in lib.util.tolist(opts.tag, 'trunk'):
        if t == lib.consts.ALL_ENABLED:
            for tag in cfgroot.active_tags():
                if tag not in tags:
                    tags.append(tag)
        else:
            tag = cfgroot.expand_tag(t)
            if tag not in tags:
                tags.append(tag)

    for tag in tags:
        if lib.scm.is_static(tag):
            lib.util.warn('no cleanup allowed in ' + tag)
            tags.remove(tag)

    # Honor the traditional convention.
    if opts.outfile == '-':
        opts.outfile = None

    emailing = (True if opts.mailto and not lib.opts.DEBUG_MODE else False)

    if opts.outfile or emailing:
        f = tempfile.NamedTemporaryFile(
            delete=not lib.opts.DEBUG_MODE,
            dir=os.path.expanduser('~'),
            mode='w',
            prefix=lib.consts.PROG + '.cleanup.',
            suffix='.%d.tmp' % os.getpid(),
        )
        os.dup2(f.fileno(), sys.stdout.fileno())
        os.dup2(f.fileno(), sys.stderr.fileno())

    print('\nSTART: %s' % time.ctime())
    print('CMDLINE: %s' % lib.util.cmdline())

    if opts.extras and not opts.dry_run:
        lib.util.rm_older_than(lib.consts.BUILD_SUMMARIES,
                               BUILD_SUMMARIES_DAYS_TO_KEEP *
                               lib.consts.SECONDS_PER_DAY)

    rc = cleanup(opts, tags, reftime)

    if opts.outfile or emailing:
        f.flush()
        if emailing:
            subject = 'Cleanup Report for %s' % (lib.consts.STATE.base)
            lib.mail.Msg(to=opts.mailto, subject=subject, files=[f.name],
                         filtertype='cleanup', via=False).send()
        if opts.outfile:
            shutil.copyfileobj(open(f.name, 'rb'), open(opts.outfile, 'ab'))

    if rc:
        sys.exit(2)

# vim: ts=8:sw=4:tw=80:et:
