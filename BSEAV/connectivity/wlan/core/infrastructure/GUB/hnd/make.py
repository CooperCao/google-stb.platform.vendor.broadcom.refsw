"""
This is intended to provide a wrapper over regular "make" with various
additional features. By default it will simply exec make. Currently
the only value-added feature is emake compatibility.

Flags intended for the underlying make program must be to the right of a
'--' flag. Flags to the left of '--' are parsed by "%(prog)s make" itself.

EXAMPLES:

To invoke regular (GNU) make:

    hnd make

To invoke "make -n foobar":

    hnd make -- -n foobar

Same but also record the environment in env.log:

    hnd make --env-log env.log -- -n foobar

To run emake instead, using EMHIST as its history file:

    hnd make --emake -- --emake-history-file=EMHIST

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os
import platform
import sys

import lib.consts
import lib.lsf
import lib.scm
import lib.util


def parse_cli(prog, alias, subcmds, advanced, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser.add_argument(
        '--buildable',
        help="specify a single buildable (emake only)"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-d', '--debug-tgt',
        metavar='TARGET',
        help="start an interactive shell just before making TARGET"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '--emake', action='store_true',
        help="invoke Electric Make with WCC build configuration"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '--env-log',
        help="write a copy of the make environment to this file")
    parser.add_argument(
        '--gdb', action='count', default=0,
        help="run make command under control of gdb"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '--gmake', action='store_true',
        help="invoke GNU Make (default)")
    parser.add_argument(
        '--inherit-emake-history', action='store_true',
        help="seed emake history from the parent branch/twig")
    parser.add_argument(
        '--lsmake', action='store_true',
        help="invoke LSF Make (lsmake) with WCC build tunings"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-a', '--ls-a',
        metavar='SECONDS',
        help="pass the -a option to lsmake"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-c', '--ls-c',
        metavar='NUM_TASKS',
        help="pass the -c option to lsmake"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-E', '--ls-e', action='store_true',
        help="pass the -E flag to lsmake"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-G', '--ls-g',
        metavar='DEBUG_LEVEL',
        help="pass the -G option to lsmake"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-j', '--ls-j',
        metavar='MAX_CORES',
        help="pass the -j option to lsmake"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-m', '--ls-m',
        metavar='HOSTS',
        help="pass the -m option to lsmake"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-n', '--ls-n',
        metavar='SLOTS',
        help="pass the -n option to lsmake"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-q', '--ls-q',
        metavar='QUEUE',
        help="pass the -q option to lsmake"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-R', '--ls-r',
        metavar='RES_REQ',
        help="pass the -R option to lsmake"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-sp', '--ls-sp',
        metavar='PRIORITY',
        help="pass the -sp option to lsmake"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-T', '--ls-t', action='store_true',
        help="pass the -T flag to lsmake"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-u', '--ls-u', action='store_true',
        help="pass the -u flag to lsmake"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-x', '--ls-x',
        metavar='NUM_RETRIES',
        help="pass the -x option to lsmake"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-y', '--ls-y', action='store_true',
        help="pass the -y flag to lsmake"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '--tag',
        help="basename of branch/tag/twig (emake only)"
        if advanced else argparse.SUPPRESS)
    parser.add_argument('args', nargs=argparse.REMAINDER)
    parser.set_defaults(func=call)


def gmake(opts):
    """Return a command line to exec GNU make."""
    if opts.args and opts.args[0] == '--':
        opts.args = opts.args[1:]

    # On all of our main host platforms (Linux, Mac, Windows/Cygwin)
    # the name "make" resolves to GNU make. "Gmake" is a nonstandard
    # name which is used on some other platforms e.g. NetBSD and
    # Solaris but is usually NOT available where GNU make is the default,
    # such as Mac and Cygwin. Therefore we avoid running "gmake" except
    # where necessary.
    cmd = ['make']

    if 'bsd' in sys.platform:
        # "make" on BSD is a whole different animal.
        cmd = ['gmake']
    elif 'cyg' in sys.platform:
        # Historically we used "gmake" on Cygwin, not sure why, but for
        # backward compatibility we still make it available by that name.
        if not os.path.exists('/usr/bin/gmake.exe'):
            lib.util.link('/usr/bin/make.exe', '/usr/bin/gmake.exe',
                          symbolic=True)

        # This is an awful, but necessary, hack. The /usr/bin/link command
        # interferes with the MS linker "link.exe" and since it's part of the
        # coreutils package it has to be installed. There's no sense doing
        # "chmod a-rx" since native Windows wouldn't see those bits anyway.
        # And every time coreutils is refreshed it could come back, so we
        # make sure it's still gone before starting any build.
        lib.util.rm('/usr/bin/link.exe')

    # This style results in prettier logfiles but output is
    # held in temp files and thus delayed.
    if '--output-sync=recurse' in opts.args:
        lib.util.note('EXPECT A LONG WAIT FOR OUTPUT '
                      'WITH "--output-sync=recurse"')

    return cmd + opts.args


def lsmake(opts):
    """Return a command line to exec LSF make."""
    if opts.args and opts.args[0] == '--':
        opts.args = opts.args[1:]

    cmd = ['lsmake']
    if lib.util.verbose_enough(2):
        cmd.append('-V')
    if opts.ls_a:
        cmd.extend(['-a', opts.ls_a])
    if opts.ls_e:
        cmd.append('-E')
    if opts.ls_c:
        cmd.extend(['-c', opts.ls_c])
    if opts.ls_g:
        cmd.extend(['-G', opts.ls_g])
    if opts.ls_j:
        cmd.extend(['-j', opts.ls_j])
    if opts.ls_m:
        cmd.extend(['-m', opts.ls_m])
    if opts.ls_r:
        cmd.extend(['-R', opts.ls_r])
    if opts.ls_t:
        cmd.append('-T')
    if opts.ls_u:
        cmd.append('-u')
    if opts.ls_x:
        cmd.extend(['-x', opts.ls_x])
    if opts.ls_y:
        cmd.append('-y')
    # The HOSTMAKE variable helps a makefile drop down to
    # gnu make if it wants to.
    cmd.append('HOSTMAKE=make')
    cmd.extend(opts.args)
    return cmd


def emake(opts):
    """Return a command line to exec Electric Accelerator emake."""
    # There's a theory that making .pyc files can confuse emake history.
    # Unproven, but since most if not all Python commands used during a
    # build will be run only once there's little value in caching bytecode.
    os.environ['PYTHONDONTWRITEBYTECODE'] = '1'

    # This feature is hardwired for now.
    opts.inherit_emake_history = True

    cmd = ['emake']

    # Allow reporting emake version without all the work below.
    if '--version' in opts.args or '-v' in opts.args:
        cmd += opts.args
        if '--' in cmd:
            cmd.remove('--')
        return cmd

    # Max available agents are 48/linux and 72/windows.
    # Max may be set lower as only linux firmware builds are done.
    if 'linux' in sys.platform:
        plat_dir = 'build_linux'
        lib.util.umask(0o02)
        num_agents = 32
        resource = '64bit-linux'
        lib.util.export('EMAKE_CM', 'eam-sj1-001.sj.' + lib.consts.DOMAIN)
        lib.util.export('EMAKE_ROOT', os.getcwd())
        # Make sure emake is on PATH.
        if '64' in platform.machine():
            newbins = [
                '/projects/hnd/tools/linux/ecloud/i686_Linux/64/bin',
                '/opt/ecloud/i686_Linux/64/bin',
            ]
        else:
            newbins = [
                '/projects/hnd/tools/linux/ecloud/i686_Linux/bin',
                '/opt/ecloud/i686_Linux/bin',
            ]
        lib.util.path_add(newbins)
    elif sys.platform.startswith('cyg'):
        plat_dir = 'build_windows'
        num_agents = 32
        resource = 'win'
        lib.util.export('EMAKE_CM', 'wc-sjca-e001.sj.' + lib.consts.DOMAIN)
        lib.util.export('EMAKE_ROOT', lib.util.cygpath(os.getcwd()))
        # Windows resources on the cluster
        lib.util.export('one_agent', 1)
        # Visual Studio plugin variables
        # Parallelize Microsoft PDB (debug file) generation steps
        lib.util.export('ECADDIN_MAX_PDB_FILES', 48)
        # Uncomment to keep generated makefiles from VS projects
        # lib.util.export('ECADDIN_DONT_RM_TMP_MAKEFILES', 1)
        lib.util.export('ECADDIN_DONT_ALLOW_PCH_AND_PDB', 'true')
        lib.util.export('ECADDIN_USE_WINDOWS_TEMP', 'true')
        lib.util.export('SHELLOPTS', 'igncr')
        cmd.append('--emake-emulation=cygwin')
    else:
        lib.util.die('unsupported platform "%s"' % sys.platform)

    # Lots of hacking below to deal with $TAG and/or $BUILDABLE in env.
    # References to 'BRAND' are backward compatibility only.

    for arg in opts.args:
        if arg.startswith('BUILDABLE=') or arg.startswith('TAG='):
            lib.util.export(arg)
        elif arg.startswith('BRAND='):
            lib.util.export(arg)
        elif arg != '--':
            cmd.append(arg)

    if opts.buildable:
        bldable = opts.buildable
    else:
        bldable = os.environ.get('BUILDABLE', os.environ.get('BRAND'))

    tag = opts.tag
    if tag:
        lib.util.export('TAG', tag)
    elif 'TAG' in os.environ:
        tag = os.environ.get('TAG')
    else:
        # Find an svn dir to query for tag.
        if os.path.exists('.svn'):
            svndir = '.'
        else:
            svndirs = []
            for parent, dnames, _ in os.walk('.'):
                if '.svn' in dnames:
                    dnames[:] = []
                    svndirs.append(parent)
            # Prefer the shortest path found, which will tend to be ./src.
            if svndirs:
                svndir = sorted(svndirs, key=len)[0]
            else:
                svndir = '.'

        # We want hard errors below because being unable to find the
        # right history file can lead to subtle build failures.
        url = lib.scm.urlof(svndir)
        lib.util.assert_(url, 'unable to derive tag for history file')
        tag = lib.scm.svn_tag_name(url)
        lib.util.assert_(tag, 'unable to find history file via tag')
        lib.util.export('TAG', tag)

    cmd.append('--emake-maxagents=%d' % num_agents)

    if not os.environ.get('EMAKE_ANNOTATION_UPLOAD'):
        cmd.append('--emake-annoupload=0')

    def cpseed(src, dst):
        """Try borrowing history from a different but related place."""
        if os.path.exists(src):
            try:
                lib.util.cp(src, dst)
            except Exception:
                try:
                    lib.util.rm(dst)
                    lib.util.cp(src, dst)
                except Exception:
                    pass
            try:
                os.chmod(dst, 0o666)
            except Exception:
                pass

        if os.path.exists(dst):
            readme = os.path.join(os.path.dirname(dst), 'README.txt')
            with open(readme, 'a') as f:
                f.write('Initialized history file from %s\n' % src)
            return True
        else:
            return False

    # Use an emake history file specific to the tag and buildable.
    # These are initialized by copy from the parent branch.
    # If working in a nonstandard (test/user) tree, also may borrow
    # history from the standard tree.
    hist_path = os.environ.get('ECHIST_FILE')
    base = lib.consts.STATE.base
    stdbase = lib.consts.STATE.stdbase
    if bldable and tag and plat_dir:
        if not hist_path:
            hist_name = '.'.join(['emake', bldable, 'data'])
            plat_path = os.path.join(plat_dir, 'PRESERVED', 'ECHISTORY')
            hist_dir = os.path.join(base, plat_path, tag)
            lib.util.mkdir(hist_dir)
            hist_path = os.path.join(hist_dir, hist_name)

            if opts.inherit_emake_history:
                rev = None
                hist_tag = tag
                while hist_tag:
                    # In test areas, even if history exists we prefer a newer
                    # history from the std location.
                    if not lib.consts.STATE.is_stdbase():
                        seed_path = os.path.join(stdbase, plat_path,
                                                 hist_tag, hist_name)
                        if not os.path.exists(hist_path) or \
                           os.path.getmtime(seed_path) > \
                           os.path.getmtime(hist_path):
                            if cpseed(seed_path, hist_path):
                                break

                    # Otherwise if a history file exists, use it.
                    if os.path.exists(hist_path):
                        break

                    # Try a history file from the parent branch.
                    seed_path = os.path.join(base, plat_path,
                                             hist_tag, hist_name)
                    if seed_path != hist_path and cpseed(seed_path, hist_path):
                        break

                    # No go. Try seeding from parent branch.
                    path, rev, _, _, _ = lib.scm.svn_tag_parent(hist_tag, rev)
                    if not path:
                        break
                    hist_tag = os.path.basename(path)

                if not os.path.exists(hist_path):
                    lib.util.warn('no parent emake history found, creating')

        # Not pretty but transcribed more or less directly from hndmake.sh.
        label = os.environ.get('EMAKE_BUILD_LABEL')
        if not label:
            eroot = os.environ['EMAKE_ROOT']
            if 'PREBUILD_DONGLE' in eroot or 'TEMP/prebuild' in eroot:
                # Label private firmware builds specially.
                names = eroot.split('/')
                parent_brand = (names[-4] if len(names) > 3 else '?')
                label = '%s_PVT_%s_for_%s' % (tag, bldable, parent_brand)
            else:
                label = '%s_%s' % (tag, bldable)
        cmd.append('--emake-build-label=' + label)

        # Inherited from hndmake.sh, not sure what they do.
        cmd.append('--emake-collapse=0')
        cmd.append('--emake-class=' + resource)
        cmd.append('--emake-resource=' + resource)

        if opts.verbose > 1:
            lib.util.mkdir(lib.opts.METADIR)
            anno = os.path.join(lib.opts.METADIR,
                                '_'.join([',emake', bldable, 'anno.xml']))
            cmd.append('--emake-annofile=' + anno)
            cmd.append('--emake-annodetail=' +
                       os.environ.get('EMAKE_ANNOTATION_LEVEL', 'basic'))

            cmd.append('--emake-debug=afjn')
            cmd.append('--emake-logfile=misc/emake.dlog')

    if hist_path:
        cmd.append('--emake-historyfile=' + hist_path)

    # Make sure temp files stay out of NFS.
    # Unfortunately it looks like we don't keep enough local disk space so
    # this can cause /tmp to fill up. Kept for doc purposes, commented out.
    # cmd.append('--emake-tmpdir=/tmp')

    return cmd


def call(opts, cfgproxy):
    """Standard subcommand entry point."""
    if opts.emake or os.environ.get('GUB_EMAKE'):
        cmd = emake(opts)
    elif opts.lsmake:
        cmd = lsmake(opts)
    else:
        cmd = gmake(opts)

    if opts.env_log:
        lib.util.mkdir(os.path.dirname(opts.env_log), vl=3)
        with open(opts.env_log, 'w') as f:
            for ev in sorted(os.environ):
                f.write('%s=%s\n' % (ev, os.environ[ev]))

    if opts.dry_run:
        lib.util.xtrace(cmd)
        sys.exit(0)

    # Trick GNU make into running an interactive shell just ahead
    # of the named target.
    # NOTE: this doesn't seem to work right but is retained for interest.
    if opts.debug_tgt:
        dbgdir = lib.util.tmpdir(prefix='makedbg.')

        rcf = os.path.join(dbgdir, 'rcfile')
        with open(rcf, 'w') as f:
            f.write('echo "\nThis is a $MAKE debug session, stopped')
            f.write(' just prior to making target \'$MAKE_TARGET\'."\n')
            f.write('echo "Exit nonzero to abort:"\n')
            f.write("PS1='*$MAKE_TARGET $?> '\n")

        mkdbg = os.path.join(dbgdir, 'Makefile.dbg')
        with open(mkdbg, 'w') as f:
            recipe = "@MAKE_TARGET='$(subst _mkdbg,,$@)'"
            recipe += ' MAKE=$(notdir $(MAKE))'
            recipe += ' bash --rcfile %s -i </dev/tty' % rcf
            f.write('.PHONY: %s_mkdbg\n' % opts.debug_tgt)
            f.write('%s_mkdbg:\n' % opts.debug_tgt)
            f.write('\t%s\n\n' % recipe)
            f.write('%s: | %s_mkdbg\n' % (opts.debug_tgt, opts.debug_tgt))

        if 'MAKEFILES' in os.environ:
            os.environ['MAKEFILES'] += ' ' + mkdbg
        else:
            os.environ['MAKEFILES'] = mkdbg

    # This will run the make command under gdb. Use once to debug
    # normally or twice to run automatically under gdb control.
    # NOTE: this doesn't seem to work but is retained for interest.
    if opts.gdb == 1:
        cmd = ['gdb', '--args'] + cmd
        lib.util.export('MAKE', lib.util.cmdline(cmd[0:3]))
    elif opts.gdb > 1:
        cmd = ['gdb', '--batch-silent', '--ex', 'run', '--args'] + cmd
        lib.util.export('MAKE', lib.util.cmdline(cmd[0:6]))

    if opts.lsmake:
        # Lsmake may be run directly or via bsub. If we got here via
        # bsub we can derive a MAKEJOBS value from LSB_HOSTS.
        if 'LSB_HOSTS' in os.environ:
            # Make a -j recommendation for potential later use by GNU make.
            cfgroot = cfgproxy.parse()
            jx = float(cfgroot.getvar('LSF_JOBS_MULTIPLIER', 2.0))
            makejobs = '-j %d' % lib.util.makejobs(jx=jx)[0]
            lib.util.export('MAKEJOBS', makejobs, vl=2)
        else:
            if opts.ls_n:
                cmd.extend(['-n', opts.ls_n])
            if opts.ls_q:
                cmd.extend(['-q', opts.ls_q])
            if opts.ls_r:
                cmd.extend(['-R', opts.ls_r])
            if opts.ls_sp:
                cmd.extend(['-sp', opts.ls_sp])

    if opts.elapsed:
        rc = lib.util.execute(cmd)
        sys.exit(2 if rc else 0)
    else:
        # No sense leaving a big Python process around through a long build.
        lib.util.xtrace(cmd)
        os.execvp(cmd[0], cmd)

# vim: ts=8:sw=4:tw=80:et:
