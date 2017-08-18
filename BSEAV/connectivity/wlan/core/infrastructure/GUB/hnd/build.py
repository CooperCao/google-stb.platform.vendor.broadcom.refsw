"""
This is the interface for fully automated, checkout-to-packaged
builds.  Builds are placed by default in a managed area under
/projects/hnd_swbuild. Branch (dynamic) builds are cleaned up
automatically on a set schedule whereas tag (static) builds are not.

EXAMPLES:

To respin "hndrte" on a given branch/twig/tag:

    %(prog)s build -t JAGUAR_BRANCH_14_10 -b hndrte --summary

To respin all failing builds on a given branch/twig/tag:

    %(prog)s build -t <tag> --failed --summary

To build every linux-2.6.36-* buildable item enabled on a given tag:

    %(prog)s build -t <tag> -p linux-2.6.36

To build with the changes in your workspace:

    %(prog)s build -b <name> -t <tag> -f <dir>

To do a test build without keeping the results:

    %(prog)s build -b <name> -t <tag> -d /dev/null

To make a subversion copy and then build from it on trunk:

    %(prog)s svn2user RB131072
    %(prog)s build -b <name> -t trunk -u RB131072

To schedule a build to run at 5:30 PM:

    %(prog)s build -b <name> -t <tag> --start-at 17:30

For email notifications, use a comma-separated list of addresses:

    %(prog)s build -b <name> -t <tag> -m user1,user2,...

To avoid using prebuilt firmware:

    %(prog)s build -b <name> --makeflags=FORCE_HNDRTE_BUILD=1

Builds submitted interactively automatically send build-start and
build-end notifications to the submitter ("$USER"). Use "--mailto
addr1,addr2" to cc others and "--mailto None" to suppress self-email.

For extended docs see http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/GUB

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import os
import platform
import random
import re
import signal
import stat
import sys
import tempfile
import time
import traceback

import lib.bld
import lib.consts
import lib.disk
import lib.item
import lib.lsf
import lib.mail
import lib.opts
import lib.patch
import lib.prep
import lib.scm
import lib.times
import lib.util

# Override options which may need to be passed to child builds.
GENOPTS_EV = 'GUB_GENOPTS'
BLDOPTS_EV = 'GUB_BLDOPTS'

# Nightly start time fuzz factor in fractional hours.
NIGHTLY_FUZZ = 0.005

SSH_HOST = 'SSH_HOST'


def parse_cli(prog, alias, subcmds, advanced, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser.set_defaults(func=call)
    parser_lcl = parser.add_mutually_exclusive_group()
    parser_pch = parser.add_mutually_exclusive_group()
    parser_rev = parser.add_mutually_exclusive_group()
    parser_to = parser.add_mutually_exclusive_group()
    parser.add_argument(
        '--as-me', action='store_true',
        help="run the build as yourself (requires -d)"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '--audit', action='store_true',
        help="derive data about files opened during the build"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '--auto-subdir', action='store_true',
        help="use a unique yyyy.mm.dd.n subdir (requires -d)"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-b', '--buildable', '--brand', default=[], action='append',
        metavar='ITEM',
        help="specify a buildable item (may be repeated)")
    # Undocumented - used to submit child builds.
    parser.add_argument(
        '--child-build-by',
        help=argparse.SUPPRESS)
    parser.add_argument(
        '--copydir',
        metavar='DIR',
        help="copy from DIR (deprecated: -f is preferred)")
    parser_to.add_argument(
        '-d', '--basedir',
        metavar='DIR',
        help="build will take place in DIR/<buildable-item-name>"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '--debug-tgt',
        metavar='TARGET',
        help="start an interactive shell just before making TARGET"
        if advanced else argparse.SUPPRESS)
    parser_pch.add_argument(
        '-f', '--diffs-from',
        metavar='DIR',
        help="generate '--patch PATCH' using diffs from DIR")
    parser.add_argument(
        '--except-pattern', default=[], action='append',
        metavar='RE',
        help="filter buildables *out* by /RE/ (slashes optional)"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '--failed', action='count',
        help="skip builds which have passed today or are running")
    parser.add_argument(
        '--full-stdout', action='store_true',
        help="send full output to stdout in addition to logfiles")
    parser.add_argument(
        '--independent', action='store_true',
        help="ignore inter-buildable dependency data in config file"
        if advanced else argparse.SUPPRESS)
    parser_lcl.add_argument(
        '--localdisk', action='store_true',
        help="do build in temp dir then move result to NFS"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-m', '--mailto', default=[], action='append',
        metavar='ADDRS',
        help="comma- or space-separated list of email addresses")
    parser.add_argument(
        '--makeflags', default=[], action='append',
        metavar='FLAGS',
        help="'--makeflags=--foo' passes --foo to make")
    parser.add_argument(
        '--message',
        help="a comment to be stored with the build")
    parser.add_argument(
        '--nightly', action='store_true',
        help="enable special handling for nightly builds"
        if advanced else argparse.SUPPRESS)
    parser_lcl.add_argument(
        '--nolocaldisk', action='store_true',
        help="force build to take place in final (NFS) location"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '--yes', '--noprompt', action='store_true',
        help="suppress prompting before spin")
    parser_pch.add_argument(
        '--patch',
        help="apply PATCH file before beginning build")
    parser.add_argument(
        '-p', '--pattern', default=[], action='append',
        metavar='RE',
        help="filter buildables by /RE/ (slashes optional)")
    parser_rev.add_argument(
        '-r', '--reftime',
        help="time cutoff or revision or [+-]offset-from-midnight")
    parser.add_argument(
        '--start-at',
        metavar='HH:MM',
        help="schedule build for a later time")
    parser.add_argument(
        '--summary',
        nargs='?', const='hnd-build-list.pdl',
        metavar='ADDRS',
        help="send a summary after builds finish")
    parser.add_argument(
        '-t', '--tag', default=[], action='append',
        help="basename of branch/tag/twig/trunk (may be repeated)")
    parser_to.add_argument(
        '--to-users', action='count',
        help="force build results into USERS area")
    parser.add_argument(
        '-u', '--user-url',
        metavar='URL',
        help="alternative base url for svn files")
    parser.add_argument(
        '-v', '--var', action='append', default=[],
        metavar='VAR=value',
        help="override a config-file variable assignment")

LOCAL_NAME = 'GLUB'


def build(cfgroot, opts):
    """Manage the entire build: all buildables, all tags."""
    rc = 0

    # Push custom builds out of production space.
#  RJ: Allow for FOR_USER option
#    if opts.reftime and not (opts.user_url or lib.util.am_builder()):
#       lib.consts.STATE.to_user_area()

    # In order to keep reference times stable for builds invoked from
    # around the world, builds operate exclusively in Pacific time.
    os.environ['TZ'] = lib.times.BRCM_TZ

    # Unbuffer stdout to avoid scrambling of log data.
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)

    # The --debug-tgt feature won't work without -D.
    # And also it doesn't seem to work right over LSF. Probably
    # fixable but for now, require no LSF.
    if opts.debug_tgt:
        if not lib.opts.DEBUG_MODE:
            lib.util.die('--debug-tgt requires debug (-D) mode')
        if not opts.localhost:
            lib.util.die('--debug-tgt requires localhost (-L) mode')

    # Remove a set of env vars which are known to be useless
    # in the build environment. Most of these are probably harmless,
    # just trying to remove noise from before-and-after comparisons.
    # But TERMCAP may have newlines which can cause real trouble.
    # Also, just as we always build in the Pacific timezone, we
    # remove any locale-setting EVs so the build starts in the
    # C locale although of course it's free to adjust from there.
    # Note that LSF seems to provide LANG and LC_COLLATE which have
    # been known to cause trouble so that's another reason to
    # remove them.
    unneeded = set(['BINARY_TYPE_HPC', 'G_BROKEN_FILENAMES',
                    'FONTCONFIG_FILE', 'FONTCONFIG_PATH', 'LANG',
                    'MAIL', 'OLDPWD', 'OSS_CHECK_LEGACY_USR_LOCAL',
                    'SHLIB_PATH', 'SHLVL',
                    'SSH_CLIENT', 'SSH_CONNECTION', 'SSH_TTY',
                    'STY', 'TERMCAP', 'WINDOW', 'XLSF_UIDDIR'])
    for ev in os.environ.keys():
        if ev in unneeded or ev.startswith('LC_'):
            del os.environ[ev]

    # Make sure -d doesn't get interpreted differently on the
    # far side of LSF or SSH.
    # TODO this doesn't actually help because it doesn't change sys.argv.
    # It may be necessary to pre-parse sys.argv and convert path values
    # to absolute paths.
    if opts.basedir:
        opts.basedir = os.path.abspath(opts.basedir)

        # If this build will run as a different user, make sure the base
        # dir is group writable. Ignore errors, because it could be a
        # directory we can write to but not chmod if we don't own it.
        # If we can't write _or_ chmod it will fail soon enough.
        if not opts.as_me and not lib.util.am_builder() and \
           os.path.exists(opts.basedir):
            try:
                omode = os.stat(opts.basedir).st_mode
                nmode = omode | stat.S_IWGRP
                if nmode != omode:
                    lib.util.chmod(opts.basedir, nmode, vl=2)
            except Exception:
                pass

    # Certain restrictions apply.
    if opts.copydir:
        lib.opts.COPYDIR = opts.copydir
        if opts.reftime:
            lib.util.die('--reftime is incompatible with --copydir')
        if opts.user_url and '/users/' in opts.user_url:
            lib.util.die('--user-url is incompatible with --copydir')
    if opts.as_me and not opts.basedir:
        lib.util.die('--as-me requires -d, --basedir')
    if opts.auto_subdir and not opts.basedir:
        lib.util.die('--auto-subdir requires -d, --basedir')
    if opts.full_stdout and not (opts.lsf_foreground or opts.localhost):
        lib.util.die('--full-stdout requires -F, --lsf-foreground')

    if opts.copydir:
        os.stat(opts.copydir)
        # Dir must be visible on the far side of a bsub from a different cwd.
        if not os.path.isabs(opts.copydir):
            lib.util.die('--copydir location must be an absolute path')
# RJ: Remove NFS check
#        if lib.util.is_local_fs(opts.copydir):
#            lib.util.die('--copydir location must be in NFS')

        # The tag value could be passed through to a child build and
        # users may need to control the tag used there. In fact we
        # require an explicit tag with --copydir because it's too easy
        # to use it alone and forget that the build may still need
        # to know what branch/tag it's building.
        if opts.tag:
            if lib.consts.STATE.depth == 1:
                lib.util.warn('files will come from --copydir despite --tag')
        else:
            lib.util.die('explicit -t, --tag required with --copydir')

    # It's legal to use --copydir and --patch/--diffs together. The copydir
    # will be used for the original (parent) build and the patch will be used
    # in a child build.
    if opts.diffs_from:
        os.stat(opts.diffs_from)

        # Patches can't be generated on target hosts because it's
        # nearly impossible to ensure that the builder will have
        # access to the same svn version on all build hosts that
        # the user's checkout has. Not to mention that the user
        # may be operating in local disk. Thus we must diff locally
        # and treat it like a prefab --patch option.

        # The patch will be applied from tree root so must
        # be relative to there.
        patchbase = lib.scm.find_tree_base(opts.diffs_from)
        if not patchbase:
            patchbase = os.path.abspath(opts.diffs_from)
        f = tempfile.NamedTemporaryFile(delete=False,
                                        prefix=lib.consts.PROG + '.build.',
                                        suffix='.patch')
        opts.patch = f.name
        lib.util.note('creating patch "%s" from %s' % (
            os.path.join(lib.consts.NFS_TMPDIR, os.path.basename(opts.patch)),
            patchbase))
        lib.patch.makepatch(f, patchbase)
        f.close()
        lib.util.chmod(opts.patch, 0o664, vl=3)

        if not os.path.getsize(opts.patch):
            os.remove(opts.patch)
            lib.util.die('no diffs found in ' + patchbase)

    # This clause could be entered via either --patch or --diffs-from.
    # We want all patches to end up in NFS_TMPDIR for consistency.
    # Note that this clause may be traversed multiple times per build.
    if opts.patch:
        if not opts.patch.startswith(lib.consts.NFS_TMPDIR):
            # Patches must be in an NFS directory accessible from all build
            # hosts. Rely on IT cleanup scripts to remove them eventually.
            # At remote sites we must use scp.
            nfspatch = os.path.join(lib.consts.NFS_TMPDIR,
                                    os.path.basename(opts.patch))
            if os.path.isdir(lib.consts.NFS_TMPDIR):
                lib.util.mv(opts.patch, nfspatch, vl=3)
            else:
                cmd = ['scp'] + lib.consts.SSH_CMD[1:] + ['-pq', opts.patch]
                cmd.append(':'.join([lib.consts.SJ_SSH_HOST, nfspatch]))
                lib.util.execute(cmd, check=True, vl=3)
                os.remove(opts.patch)
            opts.patch = nfspatch
        if lib.consts.SITE == 'sj':
            # Only enforce patch existence at final build site.
            # It may have been created on a different host so flush the NFS
            # cache first to make sure it's visible here.
            lib.util.nfsflush(os.path.dirname(opts.patch), creat=True)
            os.stat(opts.patch)

        # Pretend that generated patches were passed in with --patch.
        for i, word in enumerate(sys.argv):
            if word.startswith('--patch') or \
               word.startswith('--diff') or word.startswith('-f'):
                if '=' in word:
                    end = 1
                elif word.startswith('--'):
                    end = 2
                else:
                    end = (2 if word == '-f' else 1)
                sys.argv[i:i + end] = ['--patch', opts.patch]
                break

    # We always build from a reftime even if none was specified.
    # The default is the time of original build request, which
    # could be very different from "now" due to e.g. LSF delays.
    # This could be important for determining the "yyyy.mm.dd.n"
    # build name in addition to checkout reference time.
    # But if we've told LSF to start at a specific time then
    # be guided by that time.
    if opts.start_at:
        reftime = lib.times.timeref(opts.reftime)
    elif opts.reftime:
        reftime = lib.times.timeref(opts.reftime, ref=lib.consts.STATE.at)
    else:
        reftime = lib.times.timeref(lib.consts.STATE.at)

    # Permissions should be "open" outside of the std tree.
    # This allows users to work on files owned by the standard
    # builder and also lets the build support team do cleanups.
    # Note that we don't always have full control over umask;
    # the user's shell rc files could reset it in child shells.
    # The core problem is that many people here have .bashrc
    # linked to .bash_profile with the result that a umask set
    # in the profile will be reapplied by any child shells.
    # One workaround is for the user to replace "umask 002"
    # with "umask ${UMASK:-022}". This allows us to override
    # umask throughout a process tree by exporting UMASK=002.
    if opts.debug_mode or not lib.consts.STATE.is_stdbase():
        lib.util.umask(0o02)
    else:
        lib.util.umask(0o22)

    # Derive the list of tags (usually just one) we're building.
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

    # Finally, start whichever builds were requested for each tag.
    for tag in tags:
        rc += build_tag(cfgroot, opts, tag, reftime, opts.patch)

    return rc


# pylint: disable=too-many-return-statements
def build_tag(cfgroot, opts, tag, reftime, patch):
    """Build requested items for specified tag/branch."""
    # Guard against all illegal tag names including "-tag=foo" (one hyphen).
# RJ: Remove check for valid -t option
#    if not (tag == 'trunk' or re.match(r'^[A-Z][A-Z0-9_]+[0-9]$', tag)):
#        lib.util.error('illegal tag name: %s' % tag)
#        return 2

    # Weekly tag builds are requested by an optional tag key e.g.
    # 'nightly_days: Tue,Sat' meaning 'do only Tuesday and Saturday builds'.
    # Note that this refers to the day when the build actually runs; thus a
    # build pegged to the midnight boundary between Sat/Sun would be
    # represented by 'Sun'.
    today = time.strftime('%a', time.localtime())
    if opts.nightly:
        blddays = cfgroot.tags.get(tag, {}).get(lib.consts.NIGHTLY_DAYS)
        if blddays:
            if today not in blddays.split(','):
                return 0

    # Expand out a list of unique buildables, preserving CLI order.
    rc, items = lib.item.derive_build_item_list(cfgroot, opts, tag)

    # The 'nightly_days' key may be placed on a buildable
    # causing it to be dropped from the set on non-matching days.
    for i, item in enumerate(items):
        bcfg = cfgroot.buildables().get(item.name)
        if bcfg and not bcfg.enabled_for(today):
            items.pop(i)
        elif opts.nightly:
            # Tags can specify an offset from the zero hour (midnight) which
            # is added only for nightly builds. Time offsets are interpreted
            # as floating point values in hours. Missing offsets default to 0.
            toff = cfgroot.tags.get(tag, {}).get('nightly_offset', 0)
            item.offset = float(toff)

    # If the user requests a given buildable (or set of them)
    # against "ALL_ENABLED" branches, consider only the enabled
    # intersection.
    if items and opts.tag == [lib.consts.ALL_ENABLED]:
        limit_to = cfgroot.enabled_buildables(tag)
        for item in items[:]:
            if item.name not in limit_to:
                items.remove(item)
        if not items:
            return 0

    # Stop here if nothing to do or an error occurred.
    if not items:
        if opts.failed == 0:
            lib.util.warn('no matching build items for %s' % tag)
        else:
            verb = ('selected' if opts.pattern else 'found')
            last = (' failed' if opts.failed == 1 else '')
            lib.util.note('no%s buildables %s for %s' % (last, verb, tag),
                          vl=0)

        return 0
    elif rc:
        return rc

    # Build items ending with -dev are reserved for development purposes
    # and must not be built into the production space.
    if lib.consts.STATE.is_stdbase():
        if [i for i in items if i.name.endswith('-dev')]:
            lib.consts.STATE.to_test_area()

    # Only one user has permission to build into the standard area so if
    # the build is going there and LSF is in use, bsub as that user.
    if opts.as_me or opts.child_build_by:
        as_user = None
        if opts.as_me:
            # This apparently must exist before LSF can write to it.
            lib.util.mkdir(os.path.expanduser('~/.lsbatch'))
    else:
        as_user = lib.consts.BUILDER

    # Figure out if we need to pass the job to LSF. If basedir is specified
    # and not available via NFS then LSF makes no sense so we drop it.
    use_lsf = (not lib.lsf.submitted() and
               not lib.lsf.no_lsf(opts) and
               not (opts.basedir and lib.util.is_local_fs(opts.basedir)))

    if opts.message:
        # Allow an interactively-provided message for iterative testing.
        if opts.message == '-':
            opts.message = unicode(raw_input('Message: '))
        # Protect message against possible shell exposure e.g. ssh.
        if not opts.message or lib.util.shellsafe(opts.message):
            opts.message = opts.message
        else:
            opts.message = lib.util.url_encode(opts.message)
        for i, word in enumerate(sys.argv):
            if word.startswith('--mess'):
                if not opts.message:
                    if '=' not in word:
                        sys.argv.pop(i + 1)
                    sys.argv.pop(i)
                elif '=' in word:
                    sys.argv[i] = '='.join([word.split('=')[0], opts.message])
                else:
                    sys.argv[i + 1] = opts.message

    # The interactive prompt gives users confidence and safety.
    if not opts.yes and not opts.nightly and \
       os.isatty(sys.stdin.fileno()) and \
       os.isatty(sys.stdout.fileno()) and \
       not lib.opts.DEBUG_MODE and \
       lib.consts.STATE.depth == 1:
        if len(items) == 1:
            these = 'this build'
        else:
            these = 'these %d builds' % len(items)
        if opts.copydir:
            msg = 'Will queue %s from %s' % (these, opts.copydir)
        else:
            msg = 'Will queue %s on %s' % (these, tag)
        if opts.basedir:
            print('%s under %s:' % (msg, opts.basedir))
        elif not lib.consts.STATE.is_stdbase():
            if lib.consts.STATE.base:
                print('%s under %s:' % (msg, lib.consts.STATE.base))
            else:
                print('%s to /dev/null:' % msg)
        else:
            print('%s:' % (msg))
        for item in items:
            print('\t' + item.name)

        # TODO after bash command line editing this prompt doesn't work.
        response = raw_input('IS THIS OK? [yN]: ')
        if not response or response.lstrip().lower()[0] != 'y':
            return 0

    # Make symlinks from the SWBUILD partition into partitions
    # with more space for these builds to land in.
    # This is MUCH trickier than it would seem ... making the
    # base symlink can only be done in SJ since mounts are
    # going to be different elsewhere. And they should be made
    # on Linux because link creation requires privileges and
    # the setuid program may not be built for any other platform.
    # Also, it's best not to let individual builds create links on
    # demand because we could have lots of builds starting at the
    # same time on the same tag and racing to make a common symlink.
    # We used to always make symlinks in the top-level invocation
    # but that runs into trouble on remote systems which don't
    # have full /projects mounts. Now we defer link creation to
    # the first Linux platform at the home (SJ) site, and try to
    # catch and handle races in symlink creation.
    # Build submitted from SJ Linux systems will make links in the
    # top-level invocation with no chance of a race but in the case
    # of remote invocation we must try to handle one.
    # A corollary of this design is that the code block below may
    # be traversed more than once during a build but it won't
    # do much after the first time.
    if 'linux' in sys.platform and lib.consts.SITE == 'sj':
        if not opts.basedir and lib.consts.STATE.is_stdbase():
            if tag != 'trunk':
                platforms = set([i.hplat for i in items])
                lib.disk.link2space(cfgroot, platforms, tag)

    # Finish by re-invoking each buildable (under LSF unless asked not to).
    # This requires some special-case hacking with the command line,
    # replacing potential multiple buildable items and tags with single ones.
    for i, item in enumerate(items):
        # Build up a command line with all but buildable/tag options,
        # then add the current values back.
        rm1 = set(['-L', '--localhost'])
        rm2 = set(['-b', '--buildable', '--build', '--brand',
                   '-p', '--pattern',
                   '-t', '--tag',
                   '-Q', '--queue'])

        after = {'build': ['-t', tag, '-b', item.name]}

        cfg = cfgroot.buildables().get(item.name)
     
		# RJ:  Add FOR_USER to specify user directory   
        for_user = cfg.getvar('FOR_USER')
        print ("for user %s" % (for_user))
       	if for_user:
    		if lib.consts.STATE.base and 'USERS' not in lib.consts.STATE.base:
           		lib.consts.STATE.add2base('USERS', for_user)
    	else:
    		lib.consts.STATE.to_user_area()

    	print ("OS base dir %s" % (os.environ['GUB_BASE']))
    
        # In --copydir mode this needs to be done as the original
        # user for reasons of both permission and svn context.
        if opts.copydir:
            for cmd in lib.prep.epivers(opts.copydir):
                if cmd[1]:
                    env = os.environ.copy()
                    env.update(cmd[1])
                else:
                    env = None
                lib.util.execute(cmd[0], env=env)

        if lib.util.is_local_fs(sys.argv[0]):
            # A private, local copy of GUB won't be found across
            # LSF so warn and switch to the public version.
            cmd = [os.path.basename(sys.argv[0])]
            lib.util.warn('cannot use local %s within LSF' % sys.argv[0])
        else:
            # Since this program may be found via a symlink on Linux,
            # and Cygwin doesn't understand real NFS symlinks, we must
            # get the symlink out of $0 before passing it along.
            cmd = [os.path.realpath(sys.argv[0])]

        # This can get very tricky. When a child build is submitted
        # we want it to be eligible to use LSF. However, we use
        # the LSF "preservestarter" for firing child builds, largely
        # because we can't afford to have a Windows environment with
        # all that whitespace in PATH and so on show up on Linux.
        # Since preservestarter doesn't transmit any environment we
        # need a way of communicating to the submitted build that
        # it's not a top-level invocation. We use a --child-build-by
        # flag which indicates that this is a child. The "by" username
        # is surrounded by underscores so the build knows it's a child.
        if opts.child_build_by:
            lib.consts.STATE.by = '_%s_' % opts.child_build_by
            cmd.append('-L')

        # Rest of original command line.
        cmd.extend(lib.util.xargv(sys.argv[1:], rm1=rm1, rm2=rm2, after=after))

        if use_lsf:
            # Using LSF (normal case for initial invocation).
            nenv = os.environ.copy()
            nenv.update(cfg.exports(final=True))

            # /tools/bin is supported only on RHEL.
            if item.win_hosted:
                cmd.insert(0, '/usr/bin/python')

            # If no explicit reftime was provided, make sure to pass
            # the derived reftime along to LSF.
            if not opts.reftime and not opts.copydir:
                cmd.extend(['-r', reftime])

            jobname = lib.lsf.JobName(lib.consts.STATE.by,
                                      reftime, tag, item.name)

            if opts.start_at:
                start_at = opts.start_at
            else:
                # Optionally delay the start of a given build according
                # to config data in GUB.yaml. Each tag (aka branch) can be
                # configured with a base start time and each buildable can give
                # an offset from that using +/-. A build can also override the
                # tag base time by giving a number without a literal + or -.
                offset = item.offset
                if opts.nightly:
                    # Add a fuzz factor for nightly builds so they don't all
                    # start at exactly the same time. This is to distribute
                    # the load on the svn server and LSF job acceptance.
                    offset += NIGHTLY_FUZZ * i
                if offset > 0:
                    future = time.time() + int(offset * 60 * 60)
                    start_at = future - (future % 60)
                else:
                    start_at = None

            # No use wasting extra slots on distributed builders because
            # they'll take care of farming out the work to servers.
            if lib.lsf.lsmake_dynamic(cfg, tag):
                slots = '1'
            else:
                slots = cfg.getvar('LSF_SLOTS')

            # Tell LSF about configured dependencies in the same set
            # (typically on hndrte-dongle-wl).
            prereqs = []
            if not opts.independent:
                build_set = set([i.name for i in items])
                prqs = cfg.build_prereqs()
                reqd = set(prqs) & build_set
                if reqd:
                    for prq in sorted(reqd):
                        jnm = lib.lsf.JobName(lib.consts.STATE.by,
                                              reftime, tag, prq,
                                              prqtype=prqs[prq])
                        prereqs.append(jnm)

            priority = opts.lsf_priority
            if priority is None:
                # LSF priorities range from 0 to 200 with default=100. Send
                # release builds to the head of the line and give custom builds
                # a boost over scheduled nightlies. Deltas may be negative.
                priority = int(cfg.getvar('LSF_PRIORITY', 100))
                priority += int(cfg.getvar('LSF_PRIORITY_DELTA', 0))
                if lib.scm.is_static(tag):
                    priority += 50
                elif not opts.nightly:
                    priority += 20

            # Finally, submit the job to LSF.
            rc += lib.lsf.bsub(cmd,
                               app=lib.lsf.app_name(tag),
                               as_user=as_user,
                               dry_run=opts.dry_run,
                               env=nenv,
                               foreground=opts.lsf_foreground,
                               hostgroup=cfg.getvar('LSF_HOSTGROUP'),
                               jobname=jobname,
                               newenv=not opts.child_build_by,
                               prereqs=prereqs,
                               priority=min(priority, 200),
                               queue=cfg.getvar('LSF_QUEUE'),
                               resource=cfg.getvar('LSF_RESOURCE'),
                               slots=slots,
                               start_at=start_at,
                               time_limit=cfg.getvar('BUILD_TIMEOUT'),
                               win_hosted=item.win_hosted)
        else:
            # Not using LSF. Normally happens in the second invocation
            # when we're already submitted to LSF.

            # Start the actual build.
            rc += build_item_on_tag(cfg, opts, cmd, reftime, tag, item, patch)

    # Upon request, submit a summary job to run after all builds started by
    # this invocation have ended.
    if opts.summary and not lib.lsf.submitted():
        if lib.consts.STATE.is_stdbase():
            summary_prereqs = []
            for item in items:
                jnm = lib.lsf.JobName(lib.consts.STATE.by, reftime,
                                      tag, item.name, prqtype='ended')
                summary_prereqs.append(jnm)
            cmd = [lib.consts.PROG, 'summary', '-t', tag, '-m', opts.summary]
            if opts.message:
                cmd.extend(['--message', opts.message])
            rc += lib.lsf.bsub(cmd,
                               app=lib.lsf.app_name(tag),
                               as_user=as_user,
                               dry_run=opts.dry_run,
                               foreground=opts.lsf_foreground,
                               jobname='RESPIN.SUMMARY',
                               prereqs=summary_prereqs,
                               priority=180,  # we know this won't take long
                               queue=lib.lsf.DFLT_BLD_QUEUE,
                               resource='osmajor=RHEL6 span[hosts=1]')
        else:
            lib.util.warn('--summary not supported in %s' %
                          lib.consts.STATE.base)

    return 2 if rc else 0


def build_item_on_tag(cfg, opts, cmd, reftime, tag, item, patch):
    """Perform a single build per tag and buildable."""
    # TODO - spread use of this abstraction throughout.
    bldname = item.name
    plat = item.hplat

    # This is a bit of a hack but the complexities around svn version
    # are mindbending. Making sure SUBVERSIONVER is correctly exported
    # per child process is difficult so we just do it globally here.
    # Additional hack: some people like to use GUB for PHOENIX2 even
    # though it's not supported but PHOENIX2 would need some backports
    # to support svn 1.7+. For now we just exempt it from the upgrade
    # though backporting would be better.
    # TODO Of course when/if PHOENIX2 goes away this test can too.
    if os.path.exists('/etc/redhat-release') and os.path.exists('/tools/bin'):
        if not tag.startswith('PHO'):
            svnver = cfg.getvar('SUBVERSIONVER', '1.8.14')
            lib.util.export('SUBVERSIONVER', svnver, vl=2)

    # Another hack, to allow a build to override the svn repo. Rather than
    # make all the code understand cfg contexts, we let everybody else treat
    # it as a dumb 'constant' which we may have modified here.
    lib.consts.WL_REPO_ROOT = cfg.getvar(lib.consts.WL_REPO_ROOT_EV,
                                         lib.consts.WL_REPO_ROOT)

    bld_exception = None

    # Remove redundancies in PATH.
    lib.util.path_clean()

    # If an ssh host is given, re-run ourself over there. This may be a list
    # of semantically disparate "host sets" separated by '+'; each host set
    # may be a collection of semantically equivalent hosts separated by '|'.
    # The build will be run once per _different_ host set using the first
    # host among the _equivalents_ to respond. TODO - these could be
    # parallelized.
    ssh_hosts = cfg.getvar(SSH_HOST)
    if ssh_hosts and opts.localhost < 2 and not lib.consts.STATE.in_last_hop:
        # Squeeze out any whitespace employed for readability.
        ssh_hosts = ''.join(ssh_hosts.split())

        # Split first into a list of semantically different host sets, while
        # within each set the hosts are semantically equivalent.
        failcount = 0
        for diff_hosts in ssh_hosts.split('+'):
            # Now split the equivalent hosts out a list and shuffle it to
            # keep the hosts for platform X rotating. Then run the job
            # on the first host to respond.
            equiv_hosts = diff_hosts.split('|')
            random.shuffle(equiv_hosts)
            for ssh_host in equiv_hosts:
                sshcmd = lib.consts.SSH_CMD[:]
                if lib.util.verbose_enough(3):
                    sshcmd.append('-' + ''.join(
                        ['v' for _ in range(2, opts.verbose)]))
                if opts.as_me:
                    sshcmd.append('@'.join([lib.consts.STATE.by, ssh_host]))
                else:
                    sshcmd.append('@'.join([lib.consts.BUILDER, ssh_host]))
                sshcmd.append('--')
                sshcmd.extend(['cd', os.getcwd(), '&&'])
                sshcmd.extend(lib.consts.STATE.as_exports(to_last_hop=True))
                for ev in ['LSB_JOBID', 'LSB_JOBNAME', 'LSB_OUTPUTFILE']:
                    if os.environ.get(ev):
                        sshcmd.append('%s=%s' % (ev, os.environ[ev]))
                # Really we should use /usr/bin/python2.7 below but as of
                # 2015 there are still a few Ubuntu machines around without
                # Python 2.7. Meanwhile even the latest OSX does not have
                # /usr/bin/python2 though they do have /usr/bin/python2.7.
                sshcmd.append('/usr/bin/python')
                sshcmd.extend(cmd)
                # ssh returns 255 when it couldn't connect, so in that case
                # (and only in that case) try the next server in line.
                sshret = lib.util.execute(sshcmd)
                if sshret != 255:
                    if sshret != 0:
                        failcount += 1
                    break

        return 2 if failcount else 0

    ####################################################################
    # Now - finally! - host-hopping is done and we're ready to build.
    ####################################################################

    # I hate these special cases but unfortunately /tools/bin is present
    # on non-RHEL Linux systems though supported only on RHEL. Using it
    # on other distributions (like Ubuntu) is problematic, and there's
    # no simple test other than looking at distro.
    if 'linux' not in sys.platform or \
       'Red Hat' not in platform.linux_distribution()[0]:
        lib.util.path_clean(remove=['/tools/bin', '/usr/local/bin'])

    # Export a suggested amount of parallelism for current host.
    # There's no requirement that a build use ${MAKEJOBS} but it
    # embodies the approved relationship between LSF slot reservations
    # and GNU make -j values.
    if 'MAKEJOBS' not in os.environ:
        # The default multipliers here are SWAGs.
        jx = float(cfg.getvar('LSF_JOBS_MULTIPLIER', 2.0))
        lx = float(cfg.getvar('LSF_LOAD_MULTIPLIER', 0))
        jval, lval = lib.util.makejobs(jx=jx, lx=lx)

        if lval:
            makejobs = '-j %d -l %d' % (jval, lval)
            lib.util.export(lib.consts.LOAD_LIMIT_EV, str(lval), vl=2)
        else:
            makejobs = '-j %d' % jval

        # Make the recommendation available to any build invoking GNU make.
        lib.util.export('MAKEJOBS', makejobs, vl=2)

    def replace_tokens(text):
        """Replace certain standard tokens."""
        text = text.replace('%b', bldname)
        text = text.replace('%p', plat)
        text = text.replace('%r', lib.util.rmchars(reftime, '{-:}'))
        text = text.replace('%t', tag)
        text = text.replace('%u', lib.consts.STATE.by)
        return text

    # If a base dir was specified, do some token replacements within
    # it for the convenience of people doing automated regression build
    # testing etc.
    if opts.basedir:
        blddir = replace_tokens(os.path.join(opts.basedir, bldname))
        if not opts.auto_subdir and os.path.exists(blddir):
            if opts.debug_mode:
                lib.util.rm(blddir)
            else:
                lib.util.die('directory exists: ' + blddir)
    else:
        blddir = None

    # Similarly, allow a set of builds to be kicked off with a
    # tokenized metadir.
    if lib.opts.METADIR:
        lib.opts.METADIR = replace_tokens(lib.opts.METADIR)

    # Tell Makefiles to save "generated source" files for source packages.
    lib.util.export('WLAN_COPY_GEN', '1', vl=2)

    # Standard builds are local by default unless they use lsmake,
    # which expects to be distributed and thus requires NFS.
    localbase = None
    if lib.lsf.lsmake_dynamic(cfg, tag):
        if not opts.basedir:
            # Distributed builds are required to run in NFS so we have the
            # concept of a temporary NFS location. Doesn't buy speed of
            # course but removes the data when done just as in /tmp.
            localbase = os.path.join(lib.consts.NFS_TMPDIR, LOCAL_NAME)
            # Write this file each time to keep it young so it isn't removed.
            readme = os.path.join(lib.consts.NFS_TMPDIR, 'README')
            with open(readme, 'w') as f:
                f.write('This location is reserved for use by temporary ')
                f.write('files which must be\nkept in NFS, typically so ')
                f.write('LSF can find them. Do not remove it.\n')
                f.write('WARNING: old files here are removed automatically!\n')
            lib.util.chmod(readme, 0o664, vl=3)
    elif opts.localdisk or not (opts.nolocaldisk or blddir):
        if sys.platform.startswith('win') or sys.platform.startswith('cyg'):
            # Use a Cygwin mount to make paths a little shorter. This keeps
            # the Cygwin package clean by moving builds out of /tmp.
            # Also helps avoid warning MSB8029.
            # The use of /build is for historical compatibility. It's the share
            # name too, so URLS like "file://///hostname/build/..." just work.
            localbase = '/build'
        else:
            # Realpath is used because of "/tmp -> private/tmp" on Mac.
            localbase = lib.util.realpath(os.path.join('/tmp', LOCAL_NAME))

    # Default to a dedicated read-only repo for builds,
    # after first making sure it's up to date.
#	RJ: Remove syncing with svn mirror
#    if lib.consts.WL_REPO_ROOT_EV not in os.environ:
#        if lib.scm.svn_wait(master=lib.consts.WL_REPO_ROOT,
#                            mirror=lib.consts.WL_REPO_ROOT_RO):
#            # Modifying a so-called constant. There should be a better way.
#            lib.consts.WL_REPO_ROOT = lib.consts.WL_REPO_ROOT_RO
#        else:
#            lib.util.warn('using %s', lib.consts.WL_REPO_ROOT)

    # All metadata about the build that may be worth saving.
    bdata = lib.bld.BuildData(bldable=bldname,
                              comment=opts.message,
                              opts=opts,
                              patch=patch,
                              platform=plat,
                              reftime=reftime,
                              tag=tag,
                              user_url=lib.consts.WL_REPO_ROOT)

    # Another unfortunate special case requiring flag enumeration.
    # Certain "override options" must be made available to child builds.
    # For instance if using a private workspace with --copydir or a
    # specific reftime with -r, child builds should see the same data.
    gen_ov_opts = []
    if opts.as_of:
        gen_ov_opts.extend(['--as-of', opts.as_of])
    if opts.config_file:
        gen_ov_opts.extend(['--config-file', opts.config_file])
    if opts.debug_mode:
        gen_ov_opts.append('-' + ''.join(
            ['D' for _ in range(0, opts.debug_mode)]))
    if opts.lsf_priority:
        gen_ov_opts.append('--lsf-priority=%s' % opts.lsf_priority)
    if opts.verbose > 1:
        gen_ov_opts.append('-' + ''.join(
            ['V' for _ in range(1, opts.verbose)]))
    if gen_ov_opts:
        gen_ov_str = lib.util.cmdline(gen_ov_opts)
        lib.util.export(GENOPTS_EV, gen_ov_str, vl=2)

    bld_ov_opts = []
    if opts.as_me:
        bld_ov_opts.append('--as-me')
    if opts.copydir:
        bld_ov_opts.extend(['--copydir', opts.copydir])
    if opts.makeflags:
        bld_ov_opts.extend(['--makeflags', opts.makeflags])
    if bdata.comment:
        bld_ov_opts.extend(['--message', bdata.comment])
    if bdata.patch:
        bld_ov_opts.extend(['--patch', bdata.patch])
    if not opts.copydir:
        bld_ov_opts.extend(['--reftime', bdata.reftime])
    if bdata.user_url:
        bld_ov_opts.extend(['--user-url', bdata.user_url])
    for ov in opts.var:
        # This is a very special case. When the outer build has its ssh
        # host overridden it must not be passed through to the inner build.
        if ov.startswith(SSH_HOST):
            continue
        bld_ov_opts.extend(['--var', ov])
    if bld_ov_opts:
        bld_ov_str = lib.util.cmdline(bld_ov_opts)
        lib.util.export(BLDOPTS_EV, bld_ov_str, vl=2)

    # Need to let stdout flow to the terminal in debug mode.
    stdlogs = not opts.debug_mode

    # In some cases we may want to let output flow in addition to
    # being captured in logfiles. In particular, in a child-build
    # scenario the child should have its own logfile but the same
    # output should also flow into its parent's logfile.
    full_stdout = opts.full_stdout or opts.lsf_foreground > 1 or \
        opts.child_build_by

    # Create a Build object which will do the actual work.
    # This will determine the base of the build tree and
    # cd into it; the CWD of this program will remain
    # the base of the tree for the duration of the build.
    # Subprocesses may run in subdirectories but this process
    # must stay put.
    bld = lib.bld.Build(cfg, bdata,
                        audit=opts.audit,
                        auto_subdir=opts.auto_subdir,
                        blddir=blddir,
                        full_stdout=full_stdout,
                        localbase=localbase,
                        nightly=opts.nightly,
                        stdlogs=stdlogs)

    # Set up logging.
    logto = bld.logfiles + [sys.stdout]

    # Save summary data to SUMMARY_FILE, and also to an
    # ever-growing list in ~/build-summaries if present.
    summary_files = [os.path.join(lib.opts.METADIR, lib.bld.SUMMARY_FILE)]
    if os.access(lib.consts.BUILD_SUMMARIES, os.W_OK) and \
       stdlogs and lib.consts.STATE.is_stdbase():
        jn = lib.lsf.JobName(bld.requester, bld.reftime, bld.tag, bld.bldable)
        summary_files.append(os.path.join(lib.consts.BUILD_SUMMARIES, jn.name))

    # Print a build-started summary to appropriate logfiles.
    for out in logto + summary_files:
        if isinstance(out, basestring):
            # Enable this if it seems helpful.
            # lib.util.nfsflush(os.path.dirname(out), creat=True, up=True)
            mode = ('w' if out in summary_files else 'a')
            with open(out, mode) as f:
                f.write(bld.start_summary)
                if out not in summary_files:
                    f.write('\n')
                f.close()
        else:
            out.write(bld.start_summary)

    blddesc = bldname
    if bld.pubwd:
        blddesc += ' ' + os.path.basename(bld.pubwd)

    # When email is requested, send a "build started" notification
    # with helpful details like job id in addition to the traditional
    # "build ended" message.
    if opts.mailto:
        body = bld.start_summary
        if bld.lsflog:
            # Cygwin symlinks don't translate to NFS so we have to point
            # directly at the original.
            if sys.platform.startswith('cyg'):
                lsfout = lib.lsf.outputfile()
                body += lib.bld.fmtline('LSF_LOG', bld.pub_path(lsfout,
                                                                homedir=True))
            else:
                body += lib.bld.fmtline('LSF_LOG', bld.pub_path(bld.lsflog))
        lib.mail.Msg(to=opts.mailto,
                     body=body,
                     filtertype='build-started',
                     html=True,
                     subject='STARTED: %s on %s' % (blddesc, tag),
                     via=not lib.util.am_builder()).send()

    print()

    try:
        # All the action happens here.
        bld.run_steps()
    except Exception as bld_exception:
        if lib.opts.DEBUG_MODE:
            raise
        else:
            # Defer the exception while we close up shop.
            bld.bdata.succeeded = False

    # If we've been mirroring bldwd => pubwd, stop now.
    # This will also move the cwd back to pubwd so all
    # subsequent file writes are direct into pubwd.
    bld.finish()

    print()

    # This is a cheesy hack to keep the build-end summary from being
    # double-written to ,release.log. The problem is that once we get rid
    # of the local dir and cd back to pubwd, the same file is listed twice.
    # This ought to be done better. In fact the whole logging infra could
    # use another look.
    if lib.util.RELEASE_LOG_FILE in logto:
        logto.remove(lib.util.RELEASE_LOG_FILE)

    # Preserve results for all standard builds, successful or not.
    if lib.consts.STATE.is_stdbase() and not bld.abandoned:
        try:
            bld.preserve_built_objects()
        except Exception as e:
            lib.util.error(str(e))

    # Record build status in the content list database.
    if bld.pubwd and bld.standard and \
       lib.consts.STATE.is_stdbase() and not bld.abandoned:
        bldnum = os.path.basename(bld.pubwd)
        cmd = lib.util.xscript() + [
            'contents',
            '-b', bld.bldable,
            '-t', bld.tag,
            '-n', bldnum,
            '-m', 'status update by %s' % bldnum,
            '-r', 'HEAD',
        ]
        if bld.succeeded:
            cmd.append('--set-succeeded')
            stdout = None
        else:
            cmd.append('--set-failed')
            # On failure, dump a list of email addresses to misc/Blames.txt.
            # These represent the people who made relevant commits since
            # the last good build (LGB).
            cmd.extend(['--blames', 'LGB', '--simple-format'])
            stdout = os.path.join(lib.opts.METADIR, 'Blames.txt')
        lib.util.execute(cmd, check=True, stdout=stdout, vl=2)

    # Print the build-ended summary to appropriate logfiles.
    for out in logto + summary_files:
        if isinstance(out, basestring):
            lib.util.mkdir(os.path.dirname(out), vl=3)
            mode = ('w' if out in summary_files else 'a')
            with open(out, mode) as f:
                f.write(bld.end_summary)
                f.close()
        else:
            out.write(bld.end_summary)

    if bld.pubwd:
        # Save metadata for later build summaries.
        bld.bdata.store()

        # Preserve logs for all standard builds.
        if lib.consts.STATE.is_stdbase() and not bld.abandoned:
            try:
                bld.preserve_metadata()
            except Exception as e:
                lib.util.error(str(e))

        # Although we go to great lengths to manage umask carefully
        # it's still safest to do a post-build permissions fixup.
        # And since managing symbolic permissions is difficult in
        # native Python we bend the rules and use the underlying
        # find and chmod utilities (with only POSIX options) to do so.
        # Names matching __*__ are skipped in case they have special
        # permission needs. In particular __SIZING__ is special; it
        # must be left writable for post-processing by a different user.
        if lib.consts.STATE.is_stdbase():
            mode = 'u+rw,g+r,go-w'
        else:
            mode = 'ug+rw,o-w'
        cmd = ['find', bld.pubwd, '-name', '__*__', '-prune', '-o',
               '!', '-type', 'l', '-exec', 'chmod', mode, '{}', '+']
        lib.util.execute(cmd, vl=3)

        os.chdir(bld.pubwd)

    # Send the "build ended" notification.
    if opts.mailto:
        resultstr = ('SUCCEEDED' if bld.succeeded else 'FAILED')
        mailfiles = [os.path.join(lib.opts.METADIR, lib.bld.SUMMARY_FILE)]
        if os.path.exists(lib.bld.BUILD_ERRORS_FILE):
            mailfiles.append(lib.bld.BUILD_ERRORS_FILE)
        lib.mail.Msg(to=opts.mailto,
                     files=mailfiles,
                     filtertype='build-ended-' + resultstr.lower(),
                     html=True,
                     subject='%s: %s on %s' % (resultstr, blddesc, tag),
                     via=not lib.util.am_builder()).send()

    if bld_exception:
        raise bld_exception  # pylint: disable=raising-bad-type
    else:
        if bld.bldwd and not lib.consts.STATE.base:
            lib.util.rm(bld.bldwd, vl=2)

        return 0 if bld.succeeded else 2


def call(opts, cfgproxy):
    """Standard subcommand entry point."""
    # Try to put something in the build log when LSF kills us.
    # Otherwise it may seem like a mysterious disappearance to users.
    # TODO doesn't seem to work but seems to do no harm so left in
    # pending diagnosis.
    def sigquit_handler(signum, frame):
        """Catch a kill from LSF and explain it."""
        signal.signal(signal.SIGQUIT, signal.SIG_DFL)
        lib.util.warn('caught SIGQUIT (%d) at %s' % (signum, frame))
        sys.exit(128 + signum)
    signal.signal(signal.SIGQUIT, sigquit_handler)

    # Debug builds should never go into production space.
    if lib.opts.DEBUG_MODE:
        lib.consts.STATE.to_test_area()

    # All errors result in a message or stack trace to stderr and a
    # nonzero exit status (of course). But we also send email if it
    # looks like the user may not have direct access to stderr.
    try:
        cfgroot = cfgproxy.parse()
        rc = build(cfgroot, opts)
    except Exception as e:
        # Print message to stderr
        if isinstance(e, lib.util.FatalError):
            result = '%s\n' % e
        else:
            result = traceback.format_exc(e)
        sys.stderr.write(result + '\n')

        # Send email to --mailto list.
        if lib.lsf.submitted() or not os.isatty(sys.stderr.fileno()):
            to = set(opts.mailto if opts.mailto else [])
            if not lib.util.am_builder():
                to.add(lib.consts.STATE.by)
                via = False
            else:
                via = True

            # The mail subject should uniqify itself. Otherwise we may see
            # a potentially years-long thread of "aborted build" messages.

            # Don't cc list if running in test or debug modes.
            if opts.test_tree or opts.debug_mode or opts.lsf_foreground:
                cc = set()
                subject = 'ABORTED TEST BUILD'
            else:
                cc = set([lib.consts.SCM_LIST])
                subject = 'ABORTED BUILD'
            subject += ' BY %s AT %s' % (lib.consts.STATE.by, time.ctime())

            msg = 'The build command:\n\n%s\n\n' % lib.util.cmdline()

            if isinstance(e, lib.util.FatalError):
                problem = 'Encountered a fatal error in ' + os.getcwd()
            else:
                problem = 'Raised an exception in ' + os.getcwd()

            msg += '%s on %s:\n\n%s' % (problem, lib.consts.HOSTNAME, result)
            lsfout = os.environ.get('LSB_OUTPUTFILE')
            if lsfout and lsfout != '/dev/null':
                msg += '\nMore details may be available in %s\n' % lsfout

            lib.mail.Msg(to=to,
                         cc=cc,
                         body=msg,
                         filtertype='abort',
                         html=True,
                         subject=subject,
                         via=via).send()
            lib.util.note('abort message sent to: ' + ' '.join(to | cc))
        # Special exit status indicating infastructure failure.
        sys.exit(3)
    else:
        sys.exit(2 if rc else 0)

# vim: ts=8:sw=4:tw=80:et:
