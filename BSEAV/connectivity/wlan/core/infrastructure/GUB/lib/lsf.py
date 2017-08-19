"""Interface to the LSF job-submission system."""

from __future__ import print_function
from __future__ import unicode_literals

import os
import sys
import time

import lib.consts
import lib.opts
import lib.scm
import lib.util

DFLT_BLD_QUEUE = 'sj-wlanswbuild'


class JobName(object):
    """
    Create and manage a unique LSF job name for a given task.

    LSF job names are not required to be unique by LSF, but we may
    need to reliably derive the name of a previously-submitted job in
    order to add a dependency on it. The unique name is also helpful
    in debugging infra issues via logs.

    """

    def __init__(self, by, reftime, tag, buildable, prqtype=None):
        self.by = by
        self.reftime = lib.util.rmchars(reftime, '{-:}')
        self.tag = tag
        self.buildable = buildable
        self.prqtype = prqtype

    def __str__(self):
        return self.name

    @property
    def name(self):
        """Return a fully qualified unique name string."""
        return '.'.join([self.by, self.reftime, self.tag, self.buildable])


def submitted():
    """Has this process been re-submitted in an LSF environment?"""
    # Treat null value of LSB_JOBID same as unset.
    return os.environ.get('LSB_JOBID') and lib.consts.STATE.depth > 1


def outputfile():
    """Return the pathname of the LSF output file or None"""
    # Must be careful about nested LSF environments.
    path = (os.environ.get('LSB_OUTPUTFILE') if submitted() else None)

    return path


def app_name(tag):
    """Support some metrics requested by IT."""
    if lib.consts.STATE.is_stdbase():
        if lib.scm.is_static(tag):
            return 'sw_release_builds'
        else:
            return 'sw_ondemand_builds'
    elif lib.consts.STATE.base:
        return 'sw_dev_builds'
    else:
        return 'sw_ci_builds'


def no_lsf(opts):
    """Has the user requested suppression of LSF?"""
    return (opts.localhost or
            lib.consts.STATE.in_last_hop or
            lib.util.boolean(os.environ.get('GUB_LSF_OFF')))


# This is a transitional test which can go away when/if every active
# branch supports lsmake-dynamic*.mk.
def lsmake_dynamic(cfg, tag):
    """Return True if this branch has dynamic-lsmake capability."""
    ld_mk = ['src/tools/release/lsmake-dynamic.mk',
             'src/tools/release/lsmake-dynamic-dongle.mk']
    # Allow lsmake use to be explicitly suppressed.
    if cfg.getvar('LSMAKE_DYNAMIC') != '1':
        return False
    if lib.opts.COPYDIR:
        found = [l for l in ld_mk
                 if os.path.isfile(os.path.join(lib.opts.COPYDIR, l))]
    else:
        found = [l for l in ld_mk if lib.scm.urlexists(l, tag=tag)]
    return len(found) > 0


def bsub(bldcmd, app=None, as_user=None, cwd=None, dry_run=False,
         env=None, foreground=False, hostgroup=None, jobname=None,
         newenv=True, notify=None, output_dir=None, prereqs=None,
         priority=None, queue=None, resource=None, slots=None,
         start_at=None, time_limit=None, win_hosted=False):
    """
    Run bsub command and parse its output.
    """

    nenv = (env.copy() if env else os.environ.copy())

    cmd = []

    # Unfortunately it's very hard to determine dynamically where bsub
    # is supported. If it's not on PATH and not in /tools/bin then no,
    # of course, but /tools/bin/bsub may exist and be completely unusable
    # (currently the case on macos) or /tools/bin/bsub may not exist
    # though "bsub" via PATH works fine (currently the case on windows).
    # So it's necessary to enumerate and hard-wire facts about platforms.
    bsubcmd = '/tools/bin/bsub'
    if 'linux' in sys.platform:
        sjlsf = lib.consts.SITE == 'sj' and os.path.exists(bsubcmd)
        if sjlsf:
            bsubpath = lib.util.path_whence('bsub')
            if bsubpath and bsubpath != bsubcmd and '/tools/' in bsubpath:
                bsubcmd = bsubpath
    elif 'cygwin' in sys.platform:
        bsubpath = lib.util.path_whence('bsub')
        if bsubpath:
            sjlsf = True
            bsubcmd = bsubpath
        else:
            sjlsf = False
    else:
        sjlsf = False

    # If SJ bsub isn't natively available, try to use it via ssh.
    # And make that ssh verbose so people understand what went wrong
    # if their ssh isn't set up right.
    ssh_bsub = not sjlsf and lib.consts.STATE.depth < 2
    if ssh_bsub:
        cmd.extend(lib.consts.SSH_CMD)
        cmd.append(lib.consts.SJ_SSH_HOST)
        cmd.extend(lib.consts.STATE.as_exports())
        bsub_vl = 1
    else:
        bsub_vl = 2

    # May prefix the setuid program "as_<user>" to become that user.
    requester = lib.consts.STATE.by
    if as_user and as_user != requester:
        # Setuid program to change uid/euid, currently installed only on Linux.
        as_prog = 'as_%s' % as_user
        if lib.util.path_whence(as_prog):
            cmd.append(as_prog)
        else:
            cmd.append(os.path.join(lib.consts.LXBIN, as_prog))

        # Since the job will be changing userids across LSF, tell it
        # to start from the new user's login env to avoid pollution.
        cmd.append('-l')

        # State variables should be preserved from the current env
        # into the new login env.
        cmd.extend(['-p', '^GUB_'])

        if win_hosted:
            # Windows env is handled specially. Preserve these EVs:
            cmd.extend(['-p', 'BASH_ENV|CYGWIN|WINPFX'])
            # And remove these.
            cmd.extend(['-r', '^CVS|^HIST|^HOST|^KDE|^LD_|^LESS|^QT'])
        else:
            # This might seem redundant with "AS_BUILDER -l ..." but no.
            # A build must set a clean login env once on the submission
            # host (using AS_BUILDER -l) to make sure PATH, HOME, etc.
            # are set correctly for the target user, and then set a clean
            # login env _again_ on the execution host because it may be a
            # different architecture requiring a different PATH for example.
            # Here we request the remote (second) login env reset.
            nenv[lib.util.SET_LOGIN_ENV_EV] = '1'

        # Optional verbosity.
        if lib.util.verbose_enough(3):
            cmd.append('-x')
            if lib.util.verbose_enough(4):
                cmd.append('-v')

        cmd.append('--')
    else:
        as_user = requester

    cmd.append(bsubcmd)

    if app:
        cmd.extend(['-app', app])

    # If synchronous execution requested.
    if foreground:
        cmd.append('-I')

    if jobname:
        cmd.extend(['-J', str(jobname)])

    # Tell the new job to wait for the prerequisite build(s).
    prqnames = []
    prqlist = []
    if prereqs and not foreground:
        for prq in prereqs:
            prqnames.append(prq.buildable)
            prqlist.append('%s("%s")' % (prq.prqtype, prq.name))
    if prqlist:
        prqexpr = ' && '.join(prqlist)
        cmd.extend(['-w', prqexpr])

    # LSF may be asked to schedule this for some time in the future or
    # after a prerequisite build. Either way, work out a string
    # representation telling the user what to expect.
    if prqnames:
        # Explain that this job will start after prerequisites finish.
        start_desc = 'after %s' % ','.join(prqnames)
    else:
        if start_at:
            try:
                tm = time.localtime(float(start_at))
            except ValueError:
                cmd.extend(['-b', str(start_at)])
                start_desc = cmd[-1]
            else:
                cmd.extend(['-b', '%d:%02d' % (tm.tm_hour, tm.tm_min)])
                start_desc = time.strftime('%Y-%m-%dT%H:%M:%S', tm)
        else:
            start_desc = time.strftime('%Y-%m-%dT%H:%M:%S')

    # We want to submit jobs to the SJ queue regardless of current site.
    if not queue:
        queue = DFLT_BLD_QUEUE
    cmd.extend(['-q', queue])

    if resource:
        cmd.extend(['-R', resource])

    if hostgroup:
        cmd.extend(['-m', hostgroup])

    # Export the priority for potential use by child invocations.
    if priority is not None:
        nenv['LSF_PRIORITY'] = str(priority)
        cmd.extend(['-sp', nenv['LSF_PRIORITY']])

    # Allow this to be a string or integer for maximum configurability.
    if slots:
        cmd.extend(['-n', '%s' % slots])

    if time_limit:
        cmd.extend(['-W', time_limit])

    # LSF allows to send email or to save output in a named file,
    # but not both.
    if notify:
        cmd.extend(['-N', '-u', notify])
        assert not output_dir
    elif not foreground:
        if output_dir:
            lsbdir = output_dir
        else:
            # Need to use the home dir of the eventual (far)
            # user to ensure write access to ~/.lsbatch.
            lsbdir = os.path.expanduser('~%s/.lsbatch' % as_user)
            if sjlsf and os.access(lsbdir, os.W_OK):
                lib.util.rm_older_than(lsbdir, lib.consts.SECONDS_PER_DAY * 2)

        if win_hosted:
            # LSF is a native Windows app so it needs the UNC path.
            lsbdir = lib.consts.UNC_BASE + lsbdir
        lsbfile = os.path.join(lsbdir, '%s.%d.gub' % (jobname, os.getpid()))
        cmd.extend(['-o', lsbfile])

    # Tweak the submitting env a bit before LSF sees it.
    # First remove some stuff that's not interesting
    # or helpful to a non-interactive job on another
    # host/OS/architecture.
    # TODO this may be useless/redundant when using 'AS_BUILDER -l'.
    # Better to remove unhelpful EVs on the far side of the bsub.
    nenv.pop('EDITOR', None)
    nenv.pop('LD_LIBRARY_PATH', None)
    nenv.pop('MAIL', None)
    nenv.pop('MANPATH', None)
    nenv.pop('OLDPWD', None)
    nenv.pop('TERM', None)
    nenv.pop('TERMCAP', None)

    # NOTE: the lib.consts.STATE variables must already be present.

    # Maybe we should be consistent and always use a job starter
    # based on the value of newenv?

    # It takes some special setup to pass an LSF job from Linux to Windows.
    if win_hosted:
        nenv['CYGWIN'] = 'nodosfilewarning'

        # This needs to be present when set_buildenv.sh is sourced.
        nenv['WLAN_WINPFX'] = lib.consts.UNC_BASE

        # Make sure the remote top-level shell reads cygrc.sh by pointing
        # BASH_ENV at it and preserving BASH_ENV during privilege promotion.
        # The target script (cygrc.sh) should immediately unset BASH_ENV.
        # We used to preserve and restore any original BASH_ENV
        # setting but that's incompatible with promotion. For instance if
        # user XYZ exports BASH_ENV=/home/XYZ/.bashrc it would cause the
        # build environment to be contaminated by XYZ's settings.
        nenv['BASH_ENV'] = '/cygdrive/c/tools/build/cygrc.sh'

        # The LSF 'augmentstarter' merges the submitting (near) env
        # with the execution (far) env.
        bldcmd = [
            'augmentstarter',
            # lib.consts.WINBIN + '/wlanstarter', '--augment', '-v',
            "C:/%s/bin/bash -c '%s'" % (
                nenv['CYGWIN_DIRECTORY'],
                lib.util.cmdline(bldcmd)
            )
        ]
    elif sys.platform.startswith('win') or \
            sys.platform.startswith('cyg') or \
            not newenv:
        # We use preservestarter because we can't ever send a Windows
        # env over to Unix. And we need this hardwired path to it
        # because the shipped version has a command line length limit
        # of 2048 so we had to build a fixed version locally.
        preserve = [lib.consts.LXBIN + '/preservestarter']

        # When submitting a child build we want to lose state in
        # order to treat the new build as a fresh start. But when
        # passing along a bsub via ssh from a non-SJ-LSF system we
        # want to keep state (depth etc).
        if not sjlsf:
            preserve.extend(lib.consts.STATE.as_exports())

        # Add the prefix command.
        bldcmd = preserve + bldcmd

        # Very special cases. These two EVs are special in that they
        # can't be modified directly by the shell; they can be changed
        # only in a non-shell language such as Python.
        # Apparently the job starters are themselves started by a shell
        # so preservestarter cannot clean them in time, so we must
        # scrub them from the submitting environment to keep flags like
        # -x or -e from leaking across the LSF divide. In particular, on
        # Cygwin we put 'igncr' in SHELLOPTS but that's not recognized
        # by non-Cygwin bash so we need to keep it from leaking out.
        nenv.pop('BASHOPTS', None)
        nenv.pop('SHELLOPTS', None)

    # Join the bsub and build command lines.
    cmd.append('--')
    cmd.extend(bldcmd)

    # LSF will try to cd to the current local dir remotely.
    # This will fail if the cwd is locally unique, so in that
    # case we use /tmp which is available to Cygwin too.
    if not cwd and lib.util.is_local_fs('.'):
        cwd = '/tmp'

    # Use of "ssh bsub ..." implies another layer of shell so even though
    # we pass our cmd directly to exec we need to protect it against ssh.
    if ssh_bsub:
        for i, arg in enumerate(cmd):
            cmd[i] = lib.util.quoted(arg)

    # Verbosity of the commands below is handled here for ease of debugging.
    if dry_run:
        lib.util.xtrace(cmd, cwd=cwd, vl=0)
        return 0
    lib.util.xtrace(cmd, cwd=cwd, vl=bsub_vl)

    # Submit build command and parse bsub output for job details.
    if foreground:
        rc = lib.util.execute(cmd, cwd=cwd, env=nenv, vl=-1)
    else:
        proc = lib.util.execute(cmd, cwd=cwd, env=nenv, vl=-1,
                                stdout=lib.util.PIPE,
                                stderr=lib.util.PIPE)
        out, err = proc.communicate()
        rc = proc.returncode

        for line in out.decode('utf-8').splitlines():
            if line.startswith('Job '):
                print('%s for %s as <%s>' % (line.rstrip('.\n'),
                                             start_desc,
                                             jobname))
                # May be using shared stdout logfile.
                sys.stdout.flush()

        for line in err.decode('utf-8').splitlines():
            if not line.startswith('Job will be scheduled'):
                sys.stderr.write(line + '\n')

    return rc

# vim: ts=8:sw=4:tw=80:et:
