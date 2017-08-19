"""
Unifying wrapper for the WCC internal toolset.

There are essentially 3 levels of hnd commands:

    1. Internal commands. These are written in Python, integrated into
       the suite, make use of its standard modules, follow its
       conventions, etc.

    2. Known external commands. These are standalone executables which
       'hnd' has been told about. It knows how to execute them and how
       to get and present their usage message in a standard way. These
       commands can be executed outside of hnd but hnd may be able to
       provide added value in ways like auto-update, tweaking PATH to
       find commands in a nonstandard or dynamically-derived location,
       etc. Making a command known to hnd allows it to be visible as
       part of the suite without forcing it to be integrated in any way.
       Most importantly, it helps people find tools they may not know
       about so they don't waste time reinventing a wheel.

    3. Unknown external commands. These are standalone executables not
       known to hnd at all. Since hnd will exec its argv as a last resort,
       any command can be invoked as "hnd <command> ..." but hnd cannot
       add any value other than a consistent invocation style.

Use "hnd help hnd" for more flags or "hnd help <cmd>" for help with <cmd>.
Run "hnd" alone to see a list of known commands, and use "hnd -B <cmd>"
to run <cmd> from the checkout root.
"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import collections
import glob
import os
import re
import sys
import time

import lib.consts
import lib.opts
import lib.scm
import lib.util

import hnd

FIND_IGNORE = [
    '(',
    '(',
    '-name',
    '.snapshot',
    '-o',
    '-name',
    '.svn',
    '-o',
    '-name',
    '.git',
    '-o',
    '-name',
    '*.git',
    '-o',
    '-name',
    '.repo',
    '-o',
    '-name',
    '.*.swp',
    ')',
    '-prune',
    ')',
]

# Known internal commands.
# It should be possible to derive this dynamically via introspection.
HND_INTERNALS = (
    'basedir',
    'build',
    'cleanup',
    'contents',
    'contributors',
    'cstyle',
    'featuresizing',
    'ffind',
    'fwcmp',
    'gclient',
    'greplog',
    'list',
    'make',
    'makejobs',
    'oly rallow',
    'oly release',
    'oly report',
    'oly review',
    'oly tag2hash',
    'oly whitelist',
    'opath',
    'query',
    'rev2time',
    'rgrep',
    'scm checkout (co)',
    'scm commit (ci)',
    'scm components',
    'scm diff',
    'scm patch',
    'scm scrub',
    'scm stat (st,status)',
    'scm sync (up,update)',
    'sizing',
    'summary',
    'svn2user',
    'svn_ancestry',
    'svn_shrink',
    'validate',
)

# Known external commands.
HND_EXTERNALS = (
    'am_info',
    'find_build_errors',
    'find_svn_checkins',
    'mktag',
    'pr-diff',
    'pr-twikilog',
    'precommit',
    'rbt',
    'replacestring',
    'sparse',
    'svn_rel_tag_info',
    'svntag2rev',
    'svnwho',
)

HND_SKIP = '|'.join([
    r'\.(?:swp|bak|BAK|log|json)$',
    r'(?:epivers|phy_version)\.h',
    r'\.gclient',
    r'\.git',
])

DEPOT_TOOLS_URL = 'groups/software/tools/hnd_depot_tools'


def _depot_tools_path(command):
    """Find the location of gclient and related tools."""
    if 'HND_DEPOT_TOOLS' in os.environ:
        depot_dir = os.environ.get('HND_DEPOT_TOOLS')
    else:
        depot = [d for d in os.environ.get('PATH', '').split(os.pathsep)
                 if os.path.basename(d) == 'hnd_depot_tools']
        if depot:
            depot_dir = depot[0]
        else:
            depot_dir = os.path.join(os.path.expanduser('~/hnd_depot_tools'))
    return os.path.join(depot_dir, command)


def _update_local(path, url):
    """Create or update a checkout as needed."""
    # We've seen cases where people have an empty ~/hnd_depot_tools, probably
    # due to a prior disk space problem. This will alert them.
    if os.path.isdir(path) and not os.path.isdir(os.path.join(path, '.svn')):
        lib.util.warn('damaged gclient checkout: %s' % path)

    if os.path.isdir(path):
        # Only check for updates on RHEL platforms with /tools/bin/svn support
        # because we can't guarantee a consistent svn version across platforms.
        # Also, don't try to update a shared checkout as it would run
        # into probable permissions problems and definite races.
        # Shared checkouts should be managed explicitly by a human.
        if os.path.isfile('/etc/redhat-release') and \
           os.path.isdir('/tools/bin'):
            if lib.util.get_owner(path) == lib.consts.USER:
                cmd = lib.scm.svncmd('up', '--quiet', path)
                lib.util.execute(cmd, check=True, interactive=True, vl=2)
            else:
                lib.util.note(path + ' update skipped (shared checkout)', vl=2)
        else:
            lib.util.note(path + ' update skipped (no /tools/bin/svn support)',
                          vl=2)
    else:
        cmd = lib.scm.svncmd('co', '--quiet', url, path)
        lib.util.execute(cmd, check=True, interactive=True, vl=1)


def basedir(argv):
    """Print the path to the base of the current working tree."""
    parser = argparse.ArgumentParser(
        epilog=basedir.__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.parse_known_args(argv[1:])

    treebase = lib.scm.find_tree_base()
    if treebase:
        print(treebase)
    else:
        lib.util.die('no SCM basedir found in ' + os.getcwd(), exc=False)


def contributors(argv):
    """
    Show the people who have contributed to the specified file(s).

Prints a list of the people who've contributed to each file ordered by
the number of lines they've added/modified. Useful for figuring out who
to send a change review to, who to ask for advice, etc.  It's important
to note that this does not count the number of commits each contributor
has made but the number of lines last edited by them. The --commits flag
can be used to rank contributors by number of commits.

EXAMPLES:

To list the top 5 contributors to foo.c by lines modified:

    hnd contributors --top 5 foo.c

To do the same by number of commits:

    hnd contributors --commits --top 5 foo.c

To see the top 10 contributors within an entire directory:

    hnd contributors --top 10 src/wl/exe
"""

    parser = argparse.ArgumentParser(
        epilog=contributors.__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        '--commits', action='store_true',
        help="track # of commits rather than # of lines affected")
    parser.add_argument(
        '-c', '--current', action='store_true',
        help="filter out ex-employees")
    parser.add_argument(
        '--force-binary', action='store_true',
        help=argparse.SUPPRESS)
    parser.add_argument(
        '-r', '--reftime',
        metavar='REV',
        help="ignore history prior to this point")
    parser.add_argument(
        '-s', '--simple-format', action='count', default=0,
        help="show only usernames")
    parser.add_argument(
        '-t', '--top', type=int, default=-1,
        metavar='N',
        help="show the top N contributors")
    parser.add_argument('paths', nargs=argparse.REMAINDER)
    opts = parser.parse_args(argv[1:])

    if not opts.paths:
        cmd = [lib.consts.PROG, 'scm', 'status', '-q', '-s', '.']
        proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=2)
        stdout = proc.communicate()[0]
        if proc.returncode:
            sys.exit(2)
        opts.paths = stdout.splitlines()
    lib.util.assert_(opts.paths, 'no paths given', exc=False)

    proc = lib.util.execute(['ypcat', 'passwd'], stdout=lib.util.PIPE, vl=3)
    out = proc.communicate()[0].decode('utf-8')
    if proc.returncode:
        return proc.returncode
    ulist = [l.split(':')[0:5:4] for l in out.splitlines()]
    users = {item[0]: item[1] for item in ulist}

    def expand(paths):
        """Expand (potential) directories into file paths."""
        expanded = []
        for path in paths:
            if os.path.isdir(path) or lib.util.is_url(path):
                lscmd = lib.scm.svncmd('ls', '--depth=infinity', path)
                proc = lib.util.execute(lscmd, stdout=lib.util.PIPE, vl=2)
                lines = proc.stdout.read().splitlines()
                proc.wait()
                if len(lines) == 1:
                    expanded.append(path)
                else:
                    expanded.extend([os.path.join(path, l)
                                     for l in lines if l[-1] != '/'])
            else:
                expanded.append(path)
        return expanded

    contrib = collections.defaultdict(int)
    changes = 0
    if opts.commits:
        cmd = lib.scm.svncmd('log', '-q')
        if opts.reftime:
            cmd.extend(['-r', opts.reftime])
        paths = expand(opts.paths)
        if len(paths) > 1:
            cmd.append(lib.scm.urlof('.'))
        cmd.extend(paths)
        proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=2)
        for line in proc.stdout:
            if not (line and line[0] == 'r'):
                continue
            try:
                line = line.decode('utf-8').lstrip()
            except UnicodeDecodeError as e:
                lib.util.warn(str(e) + ':')
                sys.stderr.write(line)
                continue
            user = line.split('|')[1].strip()
            contrib[user] += 1
            changes += 1
        if proc.wait():
            return 2
    else:
        cmd = lib.scm.svncmd('blame')
        if opts.reftime:
            cmd.extend(['-r', opts.reftime])
        if opts.force_binary:
            cmd.append('--force')
        cmd.extend(expand(opts.paths))
        proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=2)
        for line in proc.stdout:
            try:
                line = line.decode('utf-8', 'ignore').lstrip()
            except UnicodeDecodeError as e:
                lib.util.warn(str(e) + ':')
                sys.stderr.write(line)
                continue
            if not line[0].isdigit():
                continue
            user = line.split()[1]
            contrib[user] += 1
            changes += 1
        if proc.wait():
            return 2

    for user in reversed(sorted(contrib, key=lambda c: contrib[c])):
        line = ''
        if user not in users:
            if opts.current:
                continue
            line += 'XX'
        count = contrib[user]
        pct = int((float(count) / changes) * 100)
        if opts.simple_format > 1:
            line = user
        else:
            line += '%s: %d (%d%%)' % (user, count, pct)
            if not opts.simple_format:
                line = '%-22s %s' % (line,
                                     users.get(user, users.get('XX' + user)))
        sys.stdout.write(line + '\n')
        opts.top -= 1
        if opts.top == 0:
            break

    return 0


def cstyle(argv):
    """Redirect to the cstyle Perl program from current branch."""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-t', '--tag',
        help="basename of branch/tag/twig")
    cstyle_argv = argv[1:]
    if '--help' in cstyle_argv:
        cstyle_argv[cstyle_argv.index('--help')] = 'HELP--'
    elif '-h' in cstyle_argv:
        cstyle_argv[cstyle_argv.index('-h')] = 'HELP--'
    opts, _ = parser.parse_known_args(cstyle_argv)

    # Yes, this argv hacking is awful.
    if 'HELP--' in cstyle_argv:
        cstyle_argv[cstyle_argv.index('HELP--')] = '-h'
    if '-t' in cstyle_argv:
        ix = cstyle_argv.index('-t')
        cstyle_argv.pop(ix)
        cstyle_argv.pop(ix)
    elif '--tag' in cstyle_argv:
        ix = cstyle_argv.index('--tag')
        cstyle_argv.pop(ix)
        cstyle_argv.pop(ix)

    cstyle_pl = 'src/tools/misc/cstyle'
    tmpfiles = [cstyle_pl]
    dtemp = lib.util.tmpdir()
    cmd = [os.path.join(dtemp, os.path.basename(cstyle_pl))]

    # This is a little tricky. The cstyle perl script has its own getopt
    # processing, so we don't want to replicate that here and have two
    # parallel arg parsers. But we do need to determine whether files
    # are mentioned explicitly, so the following heuristics are used.
    if '-f' not in cstyle_argv:
        explicit = [a for a in cstyle_argv if (
            a[0] != '-' and (
                a.endswith('.c') or
                a.endswith('.h') or
                a.endswith('/') or
                os.path.exists(a)
            )
        )]
        if not explicit:
            tmpfiles.append(cstyle_pl + '-filelist.txt')
            cmd.extend(['-f', os.path.join(dtemp, 'cstyle-filelist.txt')])

    # Use the cstyle script from the appropriate branch, falling back to trunk.
    if not opts.tag:
        for arg in cstyle_argv:
            if arg[0] != '-' and os.path.exists(arg):
                try:
                    opts.tag = lib.scm.svn_tag_name(lib.scm.urlof(arg))
                except Exception:
                    pass
                else:
                    break

    if not opts.tag:
        lib.util.note('using trunk cstyle rules')
        opts.tag = 'trunk'

    if lib.scm.get(tmpfiles, cwd=dtemp, tag=opts.tag):
        if lib.scm.get(tmpfiles, cwd=dtemp, tag='trunk'):
            lib.util.die('unable to derive svn context')

    bsdir = lib.scm.find_tree_base()
    for arg in cstyle_argv:
        if bsdir and os.path.exists(arg):
            cmd.append(os.path.relpath(arg, bsdir))
        else:
            cmd.append(arg)

    rc = lib.util.execute(cmd, cwd=bsdir, vl=2)
    return 2 if rc else 0


def ffind(argv):
    """Works just like 'find' while skipping .svn and .git dirs."""
    parser = argparse.ArgumentParser(
        epilog=ffind.__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        '--print-pruned', action='store_true',
        help="print skipped files before moving on")
    opts = parser.parse_known_args(argv[1:])

    args = opts[1]
    dirs = []
    while args:
        if args[0][0] == '-':
            break
        dirs.append(args.pop(0))

    arg_set = set(args)
    act_set = set(['-print', '-exec', '-ok', '-fls', '-delete',
                   '-fprint', '-prune', '-quit'])
    if not arg_set & act_set:
        args.append('-print')

    ignores = FIND_IGNORE[:]
    if opts[0].print_pruned:
        ignores.insert(-2, '-print')
    cmd = ['find'] + dirs + ignores + ['-o'] + args
    return lib.util.execute(cmd, vl=2)


def gclient(argv):
    """Updates and invokes our modified gclient."""
    cmdpath = _depot_tools_path('hnd_gclient.py')
    if os.environ.get('DEPOT_TOOLS_UPDATE') != '0':
        url = '/'.join([lib.consts.WL_REPO_ROOT, DEPOT_TOOLS_URL])
        _update_local(os.path.dirname(cmdpath), url)
    elif 'HND_DEPOT_TOOLS' not in os.environ:
        lib.util.warn('DEPOT_TOOLS_UPDATE=0 suppresses update')

    # This will default gclient to the best repo at the current site.
    # No sense having redundant logic between hnd and gclient.
    if lib.consts.WL_REPO_ROOT_EV not in os.environ:
        lib.util.export(lib.consts.WL_REPO_ROOT_EV,
                        lib.consts.WL_REPO_ROOT, vl=3)

    cmd = [sys.executable, cmdpath] + argv[1:]
    if lib.opts.DRY_RUN:
        cmd.insert(3, '--dry-run')

    return lib.util.execute(cmd, interactive=True, vl=2)


def list_(argv):
    """Print a list of known commands."""
    parser = argparse.ArgumentParser(
        epilog=list_.__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.parse_known_args(argv[1:])

    print('Internal Commands:')
    for subcmd in sorted(HND_INTERNALS):
        sm = hnd.__dict__.get(subcmd)
        if hasattr(sm, '__dict__') and 'init' in sm.__dict__:
            subsubs = [k for k in sm.__dict__ if k[0] != '_' and k != 'init']
            for subsub in sorted(set(subsubs)):
                print('\thnd %s %s' % (subcmd, subsub))
        else:
            print('\thnd ' + subcmd)

    print('\nExternal Commands:')
    for subcmd in sorted(HND_EXTERNALS):
        print('\thnd ' + subcmd)

    sys.stdout.write(__doc__)
    return 0


def opath(argv):
    """Given a path, print the "other" style (absolute or relative)."""
    parser = argparse.ArgumentParser(
        epilog=opath.__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.parse_known_args(argv[1:])

    paths = argv[1:] if argv[1:] else ['.']
    for path in paths:
        if os.path.isabs(path):
            print(os.path.relpath(path))
        else:
            print(os.path.abspath(path))

    return 0


def privates(argv):
    """
    THIS COMMAND HAS BEEN SUPERSEDED by the "hnd scm" suite.
Try "hnd scm help status" and "hnd scm help scrub".

EXAMPLES:

To list all non-versioned files:

    hnd scm st -p

To remove non-versioned files:

    hnd scm scrub

To look for FOO in all changed files:

    grep FOO $(hnd scm stat -q -s)
"""

    parser = argparse.ArgumentParser(
        epilog=privates.__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        '-i', '--prompt', action='store_true',
        help="(with -r) prompt before each removal")
    parser.add_argument(
        '-m', '--modified', action='store_true',
        help="print a sorted list of modified versioned files")
    parser.add_argument(
        '-r', '--rm', action='count',
        help="remove non-versioned files")
    parser.add_argument('dirs', nargs='*')
    parser.parse_args(argv[1:])

    sys.stderr.write(privates.__doc__.lstrip())
    return 1


def sparse2deps(argv):
    """Reads a sparse file and dumps a DEPS file."""
    cmdpath = _depot_tools_path('hnd_sparse2deps.py')
    if os.environ.get('DEPOT_TOOLS_UPDATE') != '0':
        url = '/'.join([lib.consts.WL_REPO_ROOT, DEPOT_TOOLS_URL])
        _update_local(os.path.dirname(cmdpath), url)
    else:
        lib.util.warn('DEPOT_TOOLS_UPDATE=0 suppresses update')

    cmd = [sys.executable, cmdpath] + argv[1:]
    if lib.opts.DRY_RUN:
        cmd.insert(3, '--dry-run')
    return lib.util.execute(cmd, interactive=True, vl=2)


def svn_shrink(argv):
    """
    "Shrink" a subversion working copy without losing modifications.

When my work partition filled up recently, I discovered a tendency to
keep old working copies around just because they contain partial work
I hope to get back to someday. The net result is that I, and probably
lots of people, hold onto trees containing possibly gigabytes of data
solely to preserve a few hundred lines of diffs. Svn-shrink tries to make
this less wasteful.

Svn-shrink runs in two modes, regular and aggressive. Regular mode takes
advantage of the fact that a subversion working copy contains two copies of
each versioned file (one visible, the other hidden in the .svn subdir). It
finds all versioned files which have NOT been modified and deletes them;
they can be restored, without even needing a network connection, via
"svn revert -R". This mode cuts the size of a working copy in half without
permanently losing any state other than timestamps on the original files.

Aggressive mode goes farther and removes the .svn directories too while
leaving your modified files in place. This can save close to 100% of the
disk space but to reconstruct from this scenario you'd need to check
out a new working copy manually and apply the preserved changes to it.

In both modes all non-versioned files are preserved, as is the result of
"svn diff" on modified versioned files. The diff is dumped into a file
called "MODS.patch" after which all non-versioned files, including
MODS.patch, are dumped into a tarball called "MODS.tar.bz2". Thus restoring
state into a new work tree is as simple as extracting the tar file and
then applying the patch.

Note that regular mode retains the option of getting back to the original
tree state whereas aggressive mode does not. For instance, say you shrink
a tree and then let it sit around for 6 months before getting back to
it. A regular shrink will allow "svn revert" to restore to the state
as of when you shrunk it, and you'd need to run "svn update" to sync
with the current state.  Aggressive mode does not record that prior state;
you can't (easily) get back to where you were, only to the current TOB
plus your changes.

EXAMPLES:

To shrink the current svn working copy in regular mode:

    hnd svn_shrink .

    """

    parser = argparse.ArgumentParser(
        epilog=svn_shrink.__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        '-a', '--aggressive', action='store_true',
        help="remove .svn dirs, keeping only files changed or added")
    parser.add_argument(
        '-u', '--remove-unversioned', action='store_true',
        help="remove non-versioned files")
    parser.add_argument('dir', nargs='+')
    opts = parser.parse_args(argv[1:])

    def getdirsize(d):
        """Return the number of bytes under the directory."""
        size = 0
        for parent, _, fnames in os.walk(d):
            for fn in fnames:
                path = os.path.join(parent, fn)
                try:
                    size += os.path.getsize(path)
                except OSError:
                    pass
        return size

    tarball = 'MODS.tar.bz2'
    patch = 'MODS.patch'
    dpaths = []
    for d in opts.dir:
        os.stat(d)
        for parent, dnames, _ in os.walk(os.path.abspath(d)):
            if '.svn' in dnames:
                dpaths.append(parent)
                dnames[:] = []
    for dpath in dpaths:
        lib.util.chdir(dpath)
        for fn in [tarball, patch]:
            if os.path.exists(fn):
                os.unlink(fn)
        saved = 0
        delayed = []
        cmd = lib.scm.svncmd('status')
        proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=2)
        mods = {tarball: True}
        tarfiles = []
        diffcmd = lib.scm.svncmd('diff')
        for line in proc.stdout:
            line = line.decode('utf-8').rstrip()
            rpath = re.sub(r'^[IM?]\s+', './', line)
            if rpath != line:
                if line.startswith('M'):
                    diffcmd.append(rpath)
                    mods[rpath] = True
                elif not opts.remove_unversioned:
                    tarfiles.append(rpath)
        if proc.wait():
            sys.exit(2)
        with open(patch, 'w') as f:
            lib.util.execute(diffcmd, check=True, stdout=f, vl=2)
            f.close()
        if os.path.getsize(patch):
            tarfiles.append(patch)
        if tarfiles:
            tarcmd = ['tar', '-cjf', tarball] + tarfiles
            lib.util.execute(tarcmd, check=True)
        os.unlink(patch)
        for parent, dnames, fnames in os.walk('.'):
            for dn in dnames:
                if dn == '.svn':
                    dnames.remove(dn)
                    if opts.aggressive:
                        apath = lib.util.mkpath(dpath, parent, dn)
                        delayed.append(apath)
            for fn in fnames:
                rpath = lib.util.mkpath(parent, fn)
                if rpath not in mods:
                    rescued = False
                    for mod in mods:
                        if rpath.startswith(mod):
                            rescued = True
                    if not rescued:
                        apath = lib.util.mkpath(dpath, parent, fn)
                        try:
                            saved += os.path.getsize(apath)
                        except OSError:
                            pass
                        lib.util.rm(apath, vl=2)
        for parent, dnames, fnames in os.walk('.', topdown=False):
            for dn in dnames:
                rpath = lib.util.mkpath(parent, dn)
                if '.svn' not in rpath and rpath not in mods:
                    apath = lib.util.mkpath(dpath, parent, dn)
                    try:
                        saved += getdirsize(apath)
                        os.rmdir(apath)
                    except OSError as e:
                        lib.util.warn(str(e))
        for apath in delayed:
            saved += getdirsize(apath)
            lib.util.rm(apath, vl=2)
        readme = 'README-svn_shrink'
        with open(readme, 'w') as f:
            cmt = '###########################################################'
            f.write(cmt + '\n')
            f.write('This svn tree was "shrunk" to save %d bytes.' % saved)
            f.write(' To re-populate:\n')
            if opts.aggressive:
                f.write('Create a new workspace, then\n')
                f.write('    tar -xjf %s\n' % tarball)
                f.write('    patch -p0 < %s\n' % patch)
            else:
                f.write('    cd %s\n' % dpath)
                if os.path.exists(os.path.join(dpath, tarball)):
                    f.write('    tar -xjf %s\n' % tarball)
                f.write('    svn revert -R -q .\n')
                if os.path.exists(os.path.join(dpath, patch)):
                    f.write('    patch -p0 < %s\n' % patch)
                f.write('    (optionally) svn update\n')
                existing = ' '.join([fn for fn in [patch, tarball, readme]
                                     if os.path.exists(fn)])
                if existing:
                    f.write('    (optionally) rm %s\n' % existing)
            f.write(cmt + '\n')
        with open(readme, 'r') as f:
            sys.stdout.write(f.read())

    return 0


def restore(argv):
    """Revert damaged files (marked by svn with a '!' in the current
work tree) to their checked-in state.
"""

    parser = argparse.ArgumentParser(
        epilog=restore.__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        '-N', '--dry-run', action='count',
        help="show what would be done without doing it")
    opts = parser.parse_args(argv[1:])
    if opts.dry_run:
        lib.opts.DRY_RUN = opts.dry_run

    reverts = set()
    cmd = lib.scm.svncmd('st', '--no-ignore', '-q')
    proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=3)
    for line in proc.stdout.readlines():
        line = line.decode('utf-8')
        match = re.match(r'^!\s+(\S+)', line)
        if not match:
            match = re.match(r'^M\s+(\S*\bromtable.S)', line)
        if not match:
            match = re.match(r'^M\s+(\S*\b(?:autoconf[.]h|wlconf[.]h))$', line)
        if not match:
            match = re.match(r'^M\s+(\S*/\.config|\.config)$', line)
        if match:
            reverts.add(match.group(1))
    if proc.wait():
        sys.exit(2)

    cmd = lib.scm.svncmd('revert') + sorted(reverts)
    if lib.opts.DRY_RUN:
        cmd.insert(0, 'echo')
    rc = lib.util.execute(cmd)
    return 2 if rc else 0


def rgrep(argv):
    """
    Do a recursive grep (egrep) in current working directory.

EXAMPLES:

To do a case-insensitive search for ABC or DEF:

    hnd rgrep -i 'ABC|DEF'

Same thing but following symlinks:

    hnd rgrep -follow -i 'ABC|DEF'

Same thing while skipping the 'build' directory:

    hnd rgrep --prune build -follow -i 'ABC|DEF'
"""

    parser = argparse.ArgumentParser(
        epilog=rgrep.__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        '-a', '--all', action='store_true',
        help="traverse the entire checkout tree")
    parser.add_argument(
        '-follow', action='store_true',
        help="follow symlinks")
    parser.add_argument(
        '-name', '--name', default=[], action='append',
        help="-name flags to filter find results")
    parser.add_argument(
        '-prune', '--prune', default=[], action='append',
        help="-prune flags to filter find results")
    opts = parser.parse_known_args(argv[1:])

    cmd = ['find']
    if opts[0].follow:
        cmd.append('-L')  # must precede pathnames
    if opts[0].all:
        treebase = os.path.relpath(lib.scm.find_tree_base())
        cmd.extend(glob.glob(os.path.join(treebase, '*')))
    else:
        cmd.extend(glob.glob('*'))
    cmd += FIND_IGNORE
    for prune in opts[0].prune:
        cmd.insert(-3, '-o')
        cmd.insert(-3, '-name')
        cmd.insert(-3, prune)
    cmd += ['-o']
    if opts[0].name:
        if len(opts[0].name) == 1:
            cmd += ['-name', opts[0].name]
        else:
            lparen, rparen = '(', ')'
            cmd += lparen
            for name in opts[0].name:
                cmd += ['-name', name]
                if name != opts[0].name[-1]:
                    cmd += ['-o']
            cmd += rparen
    cmd += ['-type', 'f']
    cmd += ['-exec', 'egrep', '-I'] + opts[1] + ['{}', '+']
    return lib.util.execute(cmd, vl=2)


def tob(argv):
    """
    Convenience wrapper for "sparse update" or "gclient sync".

Uses whichever tool was used to create the work tree originally. Strips
out unnecessary verbosity, focuses on showing file changes, and sorts
"risky" changes to the bottom so that conflicts and merges are not lost
in the noise.  It also keeps a log of past updates in a file called
"UPDATE.log" in the base dir.

Note that because it filters and sorts output, this command is silent
until the update is finished.

EXAMPLES:

To update the current tree:

    %(prog)s tob
"""

    parser = argparse.ArgumentParser(
        epilog=tob.__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.parse_known_args(argv[1:])

    treebase = lib.scm.find_tree_base()
    logfile = os.path.join(treebase, 'UPDATE.log')
    appending = os.path.exists(logfile)
    cmnt = '###################'
    if False:
        with open(logfile, 'a') as f:
            if appending:
                f.write('\n')
            f.write('%s UPDATE AT %s %s\n\n' % (cmnt, time.ctime(), cmnt))

    if os.path.exists(os.path.join(treebase, '.svn/sparsefile_url.py')):
        cmd = lib.scm.svncmd('up')
    elif os.path.exists(os.path.join(treebase, '.gclient')):
        cmd = [lib.consts.PROG, 'gclient', 'sync']
    else:
        cmd = lib.scm.svncmd('up')

    return 2 if lib.util.execute(cmd, cwd=treebase) else 0


def workspace(argv):
    """OBSOLETE INTERFACE MAINTAINED FOR COMPATIBILITY -
SEE "hnd scm man checkout" FOR THE REPLACEMENT.

If an argument is given, it is treated as the name of a directory in
which to create a new workspace according to the BOM spec(s) mentioned
with -spec.  If multiple control specs are given, the resulting
working tree is the union of them all. The value passed with -spec
may be a full URL to the file; otherwise the default SVN URL is used.

On SVN trunk this will default to gclient; otherwise it will
use sparse to populate the workspace. The -gclient flag may
be used to force gclient use or a full URL will force either.

If no argument is given, prints the "name" (root directory name)
of the workspace.

EXAMPLES:

To print a summary of the current workspace:

    %(prog)s workspace

More verbose version of the above:

    %(prog)s workspace -V

Make a trunk hndrte tree in ./abc:

    %(prog)s workspace -s hndrte abc

Same as above but combines two sparse files:

    %(prog)s workspace -s hndrte -s wl-src abc

Make a wl-src tree off DINGO in ./xyz:

    %(prog)s ws -b DINGO_BRANCH_9_10 -s wl-src xyz
"""

    parser = argparse.ArgumentParser(
        epilog=workspace.__doc__.strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        '-b', '--branch',
        default=[], action='append',
        help="check out the code associated with this buildable item")
    parser.add_argument(
        '-g', '--gclient', action='store_true',
        help="force use of DEPS file over sparse")
    parser.add_argument(
        '-s', '--spec',
        default=[], action='append',
        help="check out the code associated with the named BOM")
    parser.add_argument('todir', nargs=argparse.REMAINDER)
    opts = parser.parse_args(argv[1:])

    if not opts.todir:
        if opts.spec:
            lib.util.die('no directory path provided')
    else:
        if len(opts.todir) > 1:
            lib.util.die('only one directory path allowed')
        if not opts.spec:
            lib.util.die('no -s value provided')
        if not opts.branch:
            opts.branch = ['trunk']

        cmd = [lib.consts.PROG, 'checkout']
        for tag in opts.branch:
            cmd.extend(['-t', tag])
        for spec in opts.spec:
            url = lib.util.mkpath(lib.consts.WL_REPO_ROOT,
                                  'proj',
                                  lib.scm.svn_tag_path(opts.branch[0]),
                                  spec)
            if not url.endswith('.sparse'):
                url += '.sparse'
            if opts.gclient or not lib.scm.urlexists(url):
                url = lib.util.mkpath(lib.consts.WL_REPO_ROOT,
                                      'components/deps',
                                      lib.scm.svn_tag_path(opts.branch[0]),
                                      spec)
            cmd.append(url)
        cmd.extend(opts.todir)

        if lib.util.execute(cmd):
            return 2

    lib.util.chdir(opts.todir[0], vl=3)
    treebase = lib.scm.find_tree_base()
    if lib.util.verbose_enough(2):
        sys.stdout.write('WORKSPACE=')
    sys.stdout.write(os.path.basename(treebase) + '\n')
    return 0

# vim: ts=8:sw=4:tw=80:et:
