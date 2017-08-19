"""
Populate a tree of source code via SCM checkouts.

Takes a list of DEPS, sparse, or regular SCM URLs and a directory
name and checks out a coherent tree of versioned files into the named
directory. You may also specify a buildable item and tag to check out
the files required for that build. This command is available under its
full name 'checkout' or shorthand 'co'.

When given DEPS or sparse files it will write out the same metadata as
the "real" sparse or gclient commands so the workspace remains compatible
with them.  In other words there is no lock-in effect from using this to
make a checkout. However, "%(prog)s scm up" is recommended for updates.

If more than one URL is listed explicitly, each must be assigned to
a subdirectory as shown in the examples.

A --no-metadata option is supported which gets only files (without .svn
or .git metadata). This loses the ability to commit changes, do updates,
get diffs, etc. but can be much faster and takes half as much disk space.

There is support for parallel checkouts but note that parallelism is
most efficient with --no-metadata due to the way svn locking works. In
general, parallelism isn't helpful for sparse checkouts but it should
speed up DEPS checkouts with multiple components such as DHDAP routers.

The -N, --dry-run option will print a list of files to be checked out
rather than doing it. This is a good way of checking the validity of
a modified DEPS file. Doubling the -N flag (-NN) will only print the
parsed DEPS file(s) to stdout.

This command supports 'co' as a synonym for 'checkout'.

EXAMPLES:

To check out a given deps or sparse file into "DIR", specify its URL:

    %(prog)s scm co -d DIR $WL_REPO_ROOT/components/deps/trunk/dhd
    %(prog)s scm co -d DIR $WL_REPO_ROOT/proj/trunk/dhd.sparse

The URL can also be derived from tag and name:

    %(prog)s scm checkout -t trunk -d DIR wl-src         (deps)
    %(prog)s scm checkout -t trunk -d DIR wl-src.sparse  (sparse)

Of course trunk is default so the above could be trimmed to:

    %(prog)s scm co -d DIR wl-src
    %(prog)s scm co -d DIR wl-src.sparse

Check out the union of two DEPS files from trunk, verbosely:

    %(prog)s -V scm co -t trunk -d DIR wl-build hndrte-dongle-wl

To check out all files required for a DHDAP router on trunk:

    %(prog)s scm co -b linux-2.6.36-arm-internal-router-dhdap -d DIR

As above but just print files to stdout without the checkout:

    %(prog)s -N scm co -b linux-2.6.36-arm-internal-router-dhdap

Check out for linux-internal-wl with more verbosity and a branch:

    %(prog)s -V scm co -b linux-internal-wl -t DINGO_BRANCH_9_10 -d DIR

To check out a list of URLS (deps, sparse, or svn) into DIR/aa and DIR/bb:

    %(prog)s scm checkout -d DIR aa=<url1> bb=<url2>

Same as above but get just the files (no .svn dirs) at rev 123456:

    %(prog)s scm checkout -d DIR -m -r 123456 aa=<url1> bb=<url2>

Same as above but in serial mode:

    %(prog)s scm checkout -d DIR -m -r123456 -j1 aa=<url1> bb=<url2>

To check out into DIR from the trunk efi deps file in your user copy BLAH:

    %(prog)s scm co -t trunk -d DIR -u BLAH efi
"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os
import sys

import lib.consts
import lib.nfscp
import lib.opts
import lib.patch
import lib.pop
import lib.scm
import lib.times
import lib.util

DEPSPATH = '/components/deps/'


def parse_cli(prog, alias, subcmds, advanced, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser_item = parser.add_mutually_exclusive_group()
    parser.add_argument(
        '--any-repo', action='store_true',
        help="suppress enforcement of a single consistent svn repo"
        if advanced else argparse.SUPPRESS)
    parser_item.add_argument(
        '-b', '--buildable',
        default=[], action='append',
        help="check out the code associated with this buildable")
    parser.add_argument(
        '-d', '--basedir',
        metavar='DIR',
        help="place results in DIR")
    parser.add_argument(
        '--direct', action='store_true',
        help="check out directly to DIR, no tmp dir + copy")
    parser.add_argument(
        '--dry-run', action='count',
        help="list files in the checkout without checking out")
    parser.add_argument(
        '-j', '--jobs', type=int, default=4,
        metavar='N',
        help="allow up to N parallel checkout jobs (default=%(default)s)")
    parser.add_argument(
        '-m', '--no-metadata', action='store_true',
        help="get versioned files only (no SCM metadata)")
    parser.add_argument(
        '--patch',
        help="apply PATCH file after finishing checkout")
    parser.add_argument(
        '-i', '--prepare', action='store_true',
        help="prep the checkout by building epivers.h et al")
    parser.add_argument(
        '-p', '--preserve-times', action='store_true',
        help="give files their original commit timestamps")
    parser.add_argument(
        '-r', '--reftime',
        metavar='TIME',
        help="peg all floating URLs to this time or revision")
    parser_item.add_argument(
        '--superset', action='store_true',
        help="get the superset of files used by all buildables on this tag"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-t', '--tag',
        help="basename of branch/tag/twig")
    parser.add_argument(
        '-u', '--user-url',
        metavar='URL',
        help="alternative base url for svn files")
    parser.add_argument('args', nargs=argparse.REMAINDER)
    parser.set_defaults(func=call)


def call(opts, cfgproxy):
    """Standard subcommand entry point."""
    cfgroot = cfgproxy.parse()

    basedir = None
    if opts.basedir:
        basedir = opts.basedir
    elif opts.args:
        check = opts.args[-1]
        if check.endswith('.sparse'):
            pass
        elif not (opts.buildable or opts.superset) and len(opts.args) == 1:
            pass
        elif '/' in check:
            basedir = opts.args.pop()
        elif os.path.isdir(os.path.dirname(os.path.abspath(check))):
            basedir = opts.args.pop()

    if basedir:
        if opts.dry_run:
            lib.util.die('no target directory allowed with --dry-run')

        if os.path.exists(basedir):
            if opts.debug_mode:
                lib.util.rm(basedir)
            elif (os.path.isdir(os.path.join(basedir, '.svn')) or
                  os.path.isdir(os.path.join(basedir, '.git')) or
                  os.path.isfile(os.path.join(basedir, '.gclient'))):
                lib.util.die('%s has an existing checkout' %
                             os.path.abspath(basedir))

        # If target dir looks local, no point taking the nfscp path.
        if not opts.direct:
            opts.direct = lib.util.is_local_fs(basedir)
    elif not opts.dry_run:
        lib.util.die('no target directory specified')

    tags = lib.util.tolist(opts.tag, 'trunk')
    if len(tags) > 1:
        lib.util.die('only one tag allowed here')
    tag = lib.scm.svn_expand_tag(tags[0], cfgroot=cfgroot)

    todos = []

    for arg in opts.args:
        if '=' in arg:
            to, url = arg.split('=', 1)
        else:
            to, url = '.', arg.rstrip('/')

        # Normally we try to use the repo of the local site, if it
        # can be determined, and the SJ repo if not. But if the user
        # provided an explicit URL then let that override. If the
        # user provides multiple URLs the first one wins.
        if lib.consts.WL_REPO_ROOT_EV not in os.environ:
            if '/wlansvn/' in url and DEPSPATH in url:
                lib.util.export(lib.consts.WL_REPO_ROOT_EV,
                                url.split(DEPSPATH)[0], vl=3)

        # Special cases to handle local DEPS and sparse files.
        if url.endswith('DEPS') and os.path.isfile(url):
            url = os.path.abspath(url)
        elif url.endswith('.sparse') and not lib.util.is_url(url):
            url = '/'.join([
                lib.consts.WL_REPO_ROOT,
                'proj',
                lib.scm.svn_tag_path(tag),
                os.path.basename(url)
            ])

        if '/' not in url and not os.path.isfile(url):
            if url.endswith('.sparse'):
                url = '/'.join([
                    lib.consts.WL_REPO_ROOT,
                    'proj',
                    lib.scm.svn_tag_path(tag),
                    url,
                ])
            else:
                url = '/'.join([
                    lib.consts.WL_REPO_ROOT,
                    'components/deps',
                    lib.scm.svn_tag_path(tag),
                    url,
                ])

        todos.append((to, url))

    if opts.buildable or opts.superset:
        todos.extend(lib.item.items2todos(cfgroot, opts, tag))

    if not todos:
        print('Choose between these buildables to check out from %s:' % tag)
        for buildable in cfgroot.enabled_buildables(tag):
            print('    ' + buildable)

    if opts.dry_run or not basedir:
        if opts.dry_run > 1:
            tree = lib.pop.Tree(basedir='.', question={})
        else:
            tree = lib.pop.Tree(basedir='.', question=[])
    elif opts.direct:
        tree = lib.pop.Tree(basedir=basedir, meta=not opts.no_metadata)
    else:
        tmpbase = lib.util.tmpdir(prefix='scm.co.')
        tree = lib.pop.Tree(basedir=tmpbase, meta=not opts.no_metadata)

    # This is kind of a hack ... I want the nightly builds to show each URL
    # checked out in the log, but believe that users don't need/want this on
    # the screen, so we lower the verbosity when using the command line tool.
    if lib.opts.VERBOSITY > 0:
        lib.opts.VERBOSITY -= 1

    # Lock in the reference time (default=now).
    reftime = lib.times.timeref(opts.reftime)

    for todo in todos:
        qsto = '='.join([lib.pop.QS_TO, todo[0]])
        if '?' in todo[1]:
            url = '&'.join([todo[1], qsto])
        else:
            url = '?'.join([todo[1], qsto])
        single = not opts.any_repo
        tree.add(url, reftime=reftime, single=single, vl=0)

    # Do what was asked.
    tree.populate(jobs=opts.jobs,
                  preserve=opts.preserve_times,
                  reftime=reftime)

    if tree.question:
        # Here we were just asking for data; print it out.
        if isinstance(tree.question, list):
            for path in tree.question:
                print(path)
        else:
            for path in sorted(tree.question, key=len):
                print('%-40s %s' % (path + ':', tree.question[path]))
    elif basedir:
        # Here we've populated a directory; do some final tasks on it.
        if not opts.no_metadata:
            tree.write_metadata(reftime=reftime)

        if not opts.preserve_times:
            # See comment within this function.
            lib.util.normalize_timestamps(basedir if opts.direct else tmpbase)

        if not opts.direct:
            lib.util.xtrace('cp -pr %s %s' % (tmpbase, basedir), vl=1)
            cpobj = lib.nfscp.TreeSync(tmpbase, basedir, preserve_time=True)
            cpobj.copy(strict=True)
            lib.util.xtrace('rm -rf %s' % tmpbase, vl=1)

        # Perform remaining activities in the NFS copy.
        lib.util.chdir(basedir, vl=3)

        if opts.patch:
            patch = os.path.abspath(opts.patch)
            if lib.patch.applypatch(patch):
                sys.exit(2)

        if opts.prepare:
            for cmd in lib.prep.epivers('.'):
                lib.util.execute(cmd[0])

    sys.exit(0 if tree.success else 2)

# vim: ts=8:sw=4:tw=80:et:
