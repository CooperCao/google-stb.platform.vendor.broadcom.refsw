"""Manage the build of any buildable described in the config file."""

from __future__ import print_function
from __future__ import unicode_literals

import datetime
import glob
import os
import re
import subprocess
import sys
import tarfile
import time
import traceback
import yaml

import lib.audit
import lib.consts
import lib.log
import lib.lsf
import lib.opts
import lib.scm
import lib.times
import lib.wip
import lib.util

# NOTE: other step class modules are imported on demand.
# Pylint will complain about this but it's needed. Analyze.
import steps.base
import steps.default

# General purpose constants.
SUCCEEDED_FILE = ',succeeded'
SUMMARY_FILE = 'Summary.log'
BUILD_AUDIT_FILE = 'BuildAudit.yaml'
POST_MORTEM_FILE = 'PostMortem.yaml'
REPLAY_SCRIPT_FILE = 'Replay.sh'
BUILD_ERRORS_FILE = ',build_errors.log'
LOGS = 'LOGS'
PRESERVED = 'PRESERVED'
PRESERVE_DAYS = 120
PRESERVE_LIST = '/home/hwnbuild/src/tools/release/preserve_built_objects.txt'
PRESERVE_LOG = ',preserve.log'
SCM_DIR = 'CHECKOUT'
BLDTMP = 'BLDTMP'
WINDOWS_EXCLUDE_BG_COPY_RE = r'(?i)[.](?:bsc|cpl|idb|manifest|pdb|sbr)$'
NIGHTLY_BUILD_TOKEN = '.0'


def fmtline(name, value):
    """Return a summary line in standard format."""
    return '%-15s %s\n' % (name + ':', value)


def atpath(ref):
    """Return a pathname which maps uniquely to the reftime."""
    # Colons are illegal in Windows filenames.
    return os.path.join(lib.opts.METADIR,
                        'AT_' + lib.util.rmchars(ref, '{-: }'))


def lastpath(bldbase, reftime, allow0=False, anyday=False):
    """Return the current and suggested next build locations."""
    try:
        ref = time.localtime(lib.times.iso2seconds(reftime))
    except ValueError:
        # If given an svn revision, rely on the invocation time.
        ref = time.localtime(lib.times.iso2seconds(lib.consts.STATE.at))
    day = '%d.%d.%d.' % (ref.tm_year, ref.tm_mon, ref.tm_mday)
    bldglob = '*' if anyday else day + '*'

    if os.path.exists(bldbase):
        existing = [os.path.basename(d)
                    for d in glob.glob(os.path.join(bldbase, bldglob))]
        existing = [e for e in existing if re.match(r'\d+\.\d+\.\d+\.\d+$', e)]
    else:
        existing = None

    if existing:
        prev = sorted(existing, key=lambda p: int(p.split('.')[-1]))[-1]
        num = int(prev.split('.')[-1])
        curr = os.path.join(bldbase, '%s%d' % (day, num))
        sugg = os.path.join(bldbase, '%s%d' % (day, num + 1))
    else:
        curr = None
        sugg = os.path.join(bldbase, '%s%d' % (day, 0 if allow0 else 1))

    return curr, sugg


def nextpath(bldbase, reftime, allow0=False):
    """Return the next available path in YYYY.mm.dd.n notation."""

    # Loop to avoid race conditions in case someone else beats
    # us to a given build path.
    tries = 4
    for i in range(tries):
        path = lastpath(bldbase, reftime, allow0=allow0)[1]
        try:
            lib.util.mkdir(path, force=True, vl=2)
        except OSError:
            if i == tries - 1 or not os.path.exists(path):
                raise
        else:
            break

    return path


class BuildData(object):

    """
    Derive, track, save, and restore metadata for a given build.

    This object contains static metadata used in a build. The Build
    object itself will contain an instance of it. The separation is
    useful in a couple of ways; first, these objects are saved in a
    POST_MORTEM_FILE and can be reconstituted to provide build summary
    information. Or, an object can be created prospectively to say "If I
    did this build now, what would it look like and where would it go?".

    Put differently, you don't create a Build object unless
    you're starting a build because it creates persistent data
    like build directories.  This object can be instantiated
    at any time without making persistent changes.

    """

    def __init__(self, bldable=None, comment=None,
                 opts=None, patch=None, platform=None, reftime=None,
                 restore=None, tag=None, user_url=None):

        if restore:
            rpath = os.path.join(restore, os.path.join(lib.opts.METADIR,
                                                       POST_MORTEM_FILE))
            # Treat a 0-length file as missing (can happen if disk fills).
            if os.path.exists(rpath) and not os.path.getsize(rpath):
                raise IOError(rpath)
            self.restored = yaml.load(open(rpath, 'r'))
            # Backward compatibility.
            if 'bldable' not in self.restored and 'brand' in self.restored:
                self.restored['bldable'] = self.restored['brand']
                del self.restored['brand']
            return
        else:
            self.restored = None

        self.abandoned = False
        self.bldable = bldable
        self.comment = comment
        # Passing the opts object into the build is uncool but it's
        # only used for legacy modes and could be removed when they're done.
        self.opts = opts
        self.platform = platform
        self.patch = patch
        self.reftime = reftime
        self.tag = tag
        self.user_url = user_url

        self.succeeded = True
        self.start_time = time.time()
        # Initialized to MIN_INT so it never sorts above a good value.
        self.end_time = -sys.maxsize - 1
        self.hostname = lib.consts.HOSTNAME

        # This is always the current username.
        self.user = lib.consts.USER

        # Set on initial invocation, preserve into LSF etc.
        # Requester may or may not be the same as user.
        self.requester = lib.consts.STATE.by

        self.pubwd = None

        self.build_base = lib.consts.STATE.base

        # Work out a recommended command line for respins.
        resp = [lib.consts.PROG, 'build', '-t', self.tag,
                '-b', self.bldable, '--summary']
        self.respin = lib.util.cmdline(resp)

    def __getattr__(self, attr):
        assert self.restored
        return self.restored.get(attr, None)

    def is_child(self):
        """Return True if this build is the child of another."""
        # By convention we attribute child builds to '_user_'.
        return self.requester and self.requester[0] == '_'

    @property
    def duration(self):
        """Return the total duration of the build in seconds."""
        return int(self.end_time - self.start_time + 0.5)

    @property
    def elapsed(self):
        """Return the duration of the build in string form."""
        return '%s' % datetime.timedelta(seconds=self.duration)

    def store(self):
        """Serialize self."""
        if self.pubwd and os.path.isdir(lib.opts.METADIR):
            pm = self.__dict__.copy()
            pm.pop('opts', None)
            path = os.path.join(self.pubwd, lib.opts.METADIR, POST_MORTEM_FILE)
            with open(path, 'w') as f:
                yaml.safe_dump(pm, f, indent=2, default_flow_style=False)


# pylint: disable=too-many-instance-attributes
class Build(object):

    """Manage a build."""

    def __init__(self, cfg, bdata,
                 audit=False,
                 auto_subdir=False,
                 blddir=None,
                 full_stdout=False,
                 localbase=None,
                 nightly=False,
                 stdlogs=False):

        self.cfg = cfg
        self.bdata = bdata
        self.audit = audit
        self.full_stdout = full_stdout
        self.mirror = None
        self.badstep = None
        self.badlog = None
        self.nightly = nightly
        self.stdlogs = stdlogs

        step_od = self.cfg.get('steps')
        if not step_od:
            lib.util.die('no steps for build: ' + self.bldable)

        if blddir:
            self.standard = False
            self.logdir = self.presdir = None
            fullpath = lib.util.realpath(blddir)
            if auto_subdir:
                self.bdata.pubwd = nextpath(fullpath, self.reftime, True)
            else:
                self.bdata.pubwd = fullpath
                lib.util.mkdir(self.pubwd)
        elif lib.consts.STATE.base:
            self.standard = True
            basedir = lib.consts.STATE.base
            plat_dir = os.path.join(basedir, 'build_' + self.platform)
            tag_dir = os.path.join(plat_dir, self.tag)
            bldbase = os.path.join(tag_dir, self.bldable)
            # A REL tag NFS dir may have been newly created, on a different
            # host, so try and make sure NFS is caught up before starting.
            if lib.scm.is_static(self.tag):
                lib.util.nfsflush(bldbase, up=True)
            # Special case: only midnight-based TOB builds are eligible
            # to use the .0 slot in the production space. Otherwise, if
            # a midnight-based nightly kicks off at 4AM there would be 4
            # hours when a user spin could land in .0 and confuse testing.
            allow0 = self.is_stable()
            self.bdata.pubwd = nextpath(bldbase, self.reftime, allow0=allow0)
            bbl = len(self.build_base)
            bbase = self.pubwd[:bbl]
            self.logdir = os.path.join(bbase, LOGS, self.pubwd[bbl + 1:])
            self.presdir = os.path.join(bbase, PRESERVED, self.pubwd[bbl + 1:])
        else:
            self.standard = False

        # Need to let stdout flow in debug and child build scenarios.
        if self.stdlogs and self.pubwd:
            self.logfiles = [os.path.join(self.pubwd,
                                          lib.util.RELEASE_LOG_FILE)]
        else:
            self.logfiles = []

        self.env = {
            'BRAND': self.bldable,  # backward compatibility
            'BUILDABLE': self.bldable,
        }

        # This can be None which breaks the environment.
        if self.pubwd:
            self.env['PUBWD'] = self.pubwd

        # We track both the "public working directory" (the
        # official/permanent/network home of the build) and
        # the "build working directory" (which may be the
        # same but may also be a temporary local dir).

        self.bldwd = self.pubwd
        if localbase:
            # Make sure the local root dir is open to all.
            # This makes it easier to clean up messes and the
            # correct umask will be applied to the NFS copy anyway.
            lib.util.umask(0000)
            lib.util.mkdir(localbase, vl=2)

            # Clean up any old local build dirs.
            # This may seem like overkill given that (a) the local dir is
            # removed at build end and (b) IT does automated /tmp cleaning.
            # However, a build may occasionally abort without cleanup and not
            # all build systems have scheduled /tmp cleanup, so this is kind
            # of a belt-and-suspenders thing. Also, IT only cleans up after
            # 10 days or so whereas we pretty much know that local-build
            # copies more a day old are dead. The difference between 10 days
            # and 1 day can prevent a lot of unforced errors.
            lib.util.rm_older_than(localbase,
                                   lib.consts.SECONDS_PER_DAY * 1,
                                   ignore_errors=True)

            # The old build_linux.sh script uses /tmp/wlanswbuild/$USER and
            # sometimes leaves files behind so we handle cleanup there too.
            for tmpdir in glob.glob('/tmp/wlanswbuild/*'):
                if os.path.isdir(tmpdir):
                    lib.util.rm_older_than(tmpdir,
                                           lib.consts.SECONDS_PER_DAY * 1,
                                           ignore_errors=True)

            # Create a unique local subdir under localbase for the build.
            # This must be kept fairly short to avoid reaching the
            # 260-char limit on *@$#& Windows. The jobid or pid provide
            # sufficient uniqueness per host; the tag and buildable names
            # are frills which can be visually useful.
            subdir = '.'.join(
                [
                    self.tag,
                    self.bldable,
                    os.environ.get('LSB_JOBID', '%d' % os.getpid())
                ]
            )

            # The final full path to the local build tree.
            self.bldwd = os.path.join(localbase, subdir)

            # Just in case the dir previously existed (unlikely).
            lib.util.rm(self.bldwd, vl=2)
            lib.util.mkdir(self.bldwd, vl=2)

            if sys.platform.startswith('cyg'):
                # Do some housekeeping in C:\temp since some other, older
                # automated processes seem to leave files there.
                c_temp = '/cygdrive/c/temp'
                if os.path.isdir(c_temp):
                    lib.util.rm_older_than(c_temp,
                                           lib.consts.SECONDS_PER_DAY * 30,
                                           ignore_errors=True)

                # Guarantee that temp data generated by the build will be
                # cleaned up at build end and will not be copied to NFS.
                wintmp = os.path.join(self.bldwd, BLDTMP)
                lib.util.mkdir(wintmp, vl=3)
                wintmp = lib.util.cygpath(wintmp, flags=['-d'])
                if wintmp:
                    lib.util.export('TEMP', wintmp, vl=3)
                    lib.util.export('TMP', wintmp, vl=3)

            # This is a normal README which has the beneficial side effect
            # of making lib.util.rm_empty_parent_dirs() stop at localbase.
            # Primarily to protect C:\build on Windows which may be shared.
            # Write it each time to keep it young so it isn't removed.
            with open(os.path.join(localbase, 'README'), 'w') as f:
                f.write('This is the base dir for local-disk build trees.\n')
                f.write('WARNING: old files here are removed automatically!\n')

            # It's worth keeping a local copy of the logfile too.
            # This is for user convenience and doesn't get copied over.
            if self.stdlogs:
                self.logfiles.append(lib.util.RELEASE_LOG_FILE)

            # First and (usually) only cd, into base of local build tree.
            os.chdir(self.bldwd)

            # For a POC build we do not copy back the local tree.
            if lib.consts.STATE.base:
                self.mirror = self.start_mirroring()
        elif self.pubwd:
            # First and (usually) only cd, into base of build tree.
            os.chdir(self.pubwd)

        # Create the metadir (./misc) ASAP since lots of files go in it.
        lib.util.mkdir(lib.opts.METADIR, vl=2)
        if localbase and self.pubwd:
            lib.util.mkdir(os.path.join(self.pubwd, lib.opts.METADIR), vl=2)

        # PWD is only a bash feature but people will still expect it to work.
        os.environ['PWD'] = os.getcwd()

        # If a patch file was supplied, copy it in right away.
        # Make sure the patch is in NFS synchronously and refer to
        # it from there in case it's needed for a child build.
        if self.bdata.patch:
            pch = os.path.join(lib.opts.METADIR, 'Build.patch')
            lib.util.cp(self.bdata.patch, pch, vl=3)
            if self.bldwd != self.pubwd:
                pch = os.path.join(self.pubwd, pch)
                lib.util.cp(self.bdata.patch, pch, vl=2)
            self.bdata.patch = pch

        # The LSF log file is interesting primarily if things go wrong
        # and should be easily accessible to the user when needed.
        # We can't copy it into the METADIR dir at start time because
        # it won't have useful data yet, but copying it at end time is
        # no good either because it's only useful when the build aborts
        # without reaching the end. Therefore we use a symlink while
        # the build is running and replace it with a copy when
        # it finishes. In case of abort, the symlink will only resolve
        # until its target is cleaned from ~/.lsbatch but that should
        # be long enough to diagnose.
        lsfout = lib.lsf.outputfile()
        if lsfout and self.pubwd:
            self.lsflog = os.path.join(self.pubwd, lib.opts.METADIR, 'LSF.log')
            # Symlinks made from Cygwin look like junk files over NFS.
            if not sys.platform.startswith('cyg'):
                # We don't care about this file enough to abort a build for it.
                try:
                    lib.util.mkdir(os.path.dirname(self.lsflog), vl=3)
                    lib.util.link(lsfout, self.lsflog,
                                  force=True, symbolic=True, vl=3)
                except Exception:
                    traceback.print_exc()
        else:
            self.lsflog = None

        # Format a start summary.
        self.start_summary = self.get_start_summary()

        # Not dead yet.
        self.end_summary = None

        # Create the in-progress indicator object and its flag files.
        wip_paths = [os.path.join(d, lib.wip.WIP_FILE)
                     for d in set([self.bldwd, self.pubwd]) if d]
        self.wip = lib.wip.Heartbeat(wip_paths, self.start_summary)

        # Record the reftime. This is useful when build X depends on
        # build Y since both must come from the same build set.
        # It also allows summaries to specify the exact build set they want.
        if not lib.opts.COPYDIR:
            with open(atpath(self.reftime), 'w') as f:
                f.write('# Records the checkout timestamp\n')

        # This collects all the replay scripts in one place.
        self.replays = os.path.join(lib.opts.METADIR, REPLAY_SCRIPT_FILE)

        # The old build system used to check out directly into
        # the build tree and then remove .svn dirs, leaving a
        # damaged checkout. My original plan was to improve on that
        # by keeping around a clean/correct checkout for use
        # in debugging failed builds. So the code was designed
        # to check out to a subtree called SCM_DIR, and then
        # use pax -rwl to copy it into the build tree with -s
        # flags to keep .svn/.git/etc from being copied. The idea
        # was that we'd end up with a pristine SCM checkout for
        # reference, while the build tree itself would have never
        # been contaminated with .svn data so we wouldn't have to
        # worry about cleaning or avoiding it.
        # However, this has gradually evolved. Since all builds are
        # done in a local FS and copied out to NFS, the copy (rsync)
        # can be tuned to leave .svn data behind. In other words the
        # line can be between the local tree (which has SCM metadata)
        # and the NFS tree (which doesn't) rather than between the local
        # build and checkout trees (self.bldwd and self.scmdir).
        # Bottom line: nowadays we check out directly to the build
        # tree just like the old tools did but instead of removing .svn
        # metadata we just don't copy it to NFS by default.
        # A prime motivation for this was that while the pax copy
        # was fast on Linux, largely because of the -l (hard-link)
        # flag, it turned out to be pretty slow on Windows.
        # self.scmdir = SCM_DIR
        self.scmdir = self.bldwd

        if audit:
            excl = [lib.wip.WIP_FILE]
            if self.scmdir != self.bldwd:
                excl.append(os.path.relpath(self.scmdir, self.bldwd) + '/')
            self.audit = lib.audit.AtimeAudit(self.bldwd, exclude=excl)

    def __getattr__(self, attr):
        """Pass unrecognized queries to the BuildData object."""
        return getattr(self.bdata, attr)

    def start_mirroring(self):
        """Spin off a nfscp command in the background."""
        # Set up background mirroring local => NFS.
        cpcmd = lib.util.xscript('nfscp', vd=1)

        # Some builds create absolute symlinks which will have to be
        # retargeted in order to resolve within the dest dir.
        cpcmd.append('-f')

        # If the build is going to a non-standard place the result
        # is unofficial and doesn't need to be write-protected.
        if lib.consts.STATE.is_stdbase():
            cpcmd.append('--protect')

        if self.stdlogs:
            # This is being written directly to NFS so no copy needed.
            cpcmd.append('--exclude=' + lib.util.RELEASE_LOG_FILE)

        # To avoid confusion and time lag we manage this flag file elsewhere
        # and do not let it be copied though it probably would do no harm.
        cpcmd.append('--exclude=' + lib.wip.WIP_FILE)

        # Anything placed in a directory with this conventional name
        # will be treated as temporary and not transferred into NFS.
        cpcmd.append('--exclude=' + BLDTMP)

        # Another kind of temp file we don't care about.
        cpcmd.append('--exclude=ecloud_tmp_')

        # On Windows, since Python open() acquires a mandatory lock even
        # when opening a file only for read access, background copying
        # can cause trouble since it might lock a .pdb file and cause
        # cl.exe to fail. Rather than defer the entire copy to the end,
        # try deferring only file types which exhibit lock problems.
        if sys.platform.startswith('cyg') or sys.platform.startswith('win'):
            cpcmd.append('--exclude-bg=' + WINDOWS_EXCLUDE_BG_COPY_RE)

        if not lib.opts.DEBUG_MODE:
            # Prevent SCM metadata from leaking into src releases.
            cpcmd.append('--exclude-scm')
            # Clean up when done.
            cpcmd.append('--remove-src')

        # Copy continuously till stdin closes.
        cpcmd.append('--watch-stdin=30')

        # Run the program in the background and return the process object.
        cpcmd.extend([self.bldwd, self.pubwd])
        return lib.util.execute(cpcmd, bg=True, stdin=lib.util.PIPE, vl=2)

    def get_start_summary(self):
        """Create a build-start summary."""
        summ = ''

        if self.comment:
            summ += fmtline('MESSAGE', lib.util.url_decode(self.comment))
        if self.patch:
            summ += fmtline('PATCH', self.patch)

        summ += fmtline('STARTED', lib.times.ctimez(self.start_time))
        summ += fmtline('FROM', os.environ.get('GUB_FROM', 'unknown'))
        summ += fmtline('BY', self.requester)
        summ += fmtline('AS', self.user)
        summ += fmtline('BUILDABLE', self.bldable)
        summ += fmtline('REFTIME', lib.util.rmchars(self.reftime, '{}'))
        if lib.opts.COPYDIR:
            summ += fmtline('TAG', None)
            summ += fmtline('REPO', lib.opts.COPYDIR)
        else:
            summ += fmtline('TAG', self.tag)
            summ += fmtline('REPO', lib.consts.WL_REPO_ROOT)
        summ += fmtline('BUILD_DIR', self.pubwd)
        if self.bldwd != self.pubwd:
            summ += fmtline('LOCAL_DIR', self.bldwd)
        summ += fmtline('HOSTNAME', self.hostname)
        uname = os.uname()
        htype = ' '.join([uname[0], uname[2], uname[4]])
        if sys.platform.startswith('cyg') or sys.platform.startswith('win'):
            winver = subprocess.check_output(['cmd', '/c', 'ver'])
            htype += ' ' + winver.strip()
        elif os.path.exists('/etc/redhat-release'):
            htype += ' ' + open('/etc/redhat-release', 'r').read().strip()
        elif os.path.exists('/etc/debian_version'):
            htype += ' Ubuntu ' + open('/etc/debian_version').read().strip()
        elif os.path.exists('/usr/bin/sw_vers'):
            out = subprocess.check_output(['/usr/bin/sw_vers'])
            osxver = [l.split(':\t')[-1] for l in out.splitlines()]
            htype += ' ' + ' '.join(osxver)
        summ += fmtline('HOSTTYPE', htype)
        summ += fmtline('PID', str(os.getpid()))
        for lsfev in ('LSB_JOBID', 'LSB_JOBNAME', 'LSB_OUTPUTFILE'):
            if lsfev in os.environ:
                summ += fmtline(lsfev, os.environ[lsfev])
        slots = len(os.environ.get('LSB_HOSTS', '').split())
        if slots:
            summ += fmtline('LSF_SLOTS', str(slots))
        summ += fmtline('CMDLINE', lib.util.cmdline(lib.consts.ARGV))
        if self.pubwd:
            summ += fmtline('BUILD', self.pub_path(self.pubwd))
        if self.logfiles:
            summ += fmtline('BUILD_LOG', self.pub_path(self.logfiles[0]))
        return summ

    def get_end_summary(self):
        """Create a build-end summary."""
        summ = self.start_summary[:]
        if self.logfiles:
            if not self.succeeded:
                try:
                    if os.path.getsize(BUILD_ERRORS_FILE):
                        summ += fmtline('ERRORS',
                                        self.pub_path(BUILD_ERRORS_FILE))
                except OSError:
                    pass
        contents = os.path.join(lib.opts.METADIR, lib.consts.CONTENTS_HTML)
        if os.path.exists(contents) and os.path.getsize(contents):
            summ += fmtline('CONTENTS', self.pub_path(contents))
        else:
            # For debugging when something goes wrong with infrastructure.
            cdb = lib.util.mkpath(lib.consts.CLBASE,
                                  lib.scm.svn_tag_path(self.tag),
                                  self.bldable, lib.consts.CLNAME)
            summ += fmtline('CONTENTS_DB', cdb)
        summ += fmtline('RESPIN', self.respin)
        summ += fmtline('ELAPSED', self.elapsed)
        result = ('SUCCEEDED' if self.succeeded else 'FAILED')
        summ += fmtline(result, lib.times.ctimez(self.end_time))
        return summ

    def run_steps(self):
        """Do the actual build by running each step in order."""
        step_od = self.cfg.get('steps')
        for stepname in step_od:
            # IP Validation is silently skipped on branches < 13 because it's
            # impractical to add IP tags to all old branches. They were added
            # on trunk prior to IGUANA_BRANCH_13_0.
            if 'Validate' in stepname:
                if lib.scm.svn_tag_cmp(self.tag, 'JAGUAR_BRANCH_14_10') < 0:
                    continue

            stepcfg = step_od[stepname]

            stepobj = steps.base.Step.lookup(stepcfg, stepname, self)

            logs = steplog = None

            # We catch ALL exceptions to ensure build results are reported.
            # These are not rethrown; they just print a message, mark the
            # step as a failure, and break out of the step loop.
            try:
                if self.logfiles:
                    logs = ['+' + lf for lf in self.logfiles]
                    stepbase = '%s-%d.log' % (stepname, stepobj.seqno)
                    steplog = os.path.join(lib.opts.METADIR, stepbase)
                    logs.append(steplog)
                    steps.base.mark_start(stepname, self.logfiles)

                # This is where steps actually run.
                stepobj.invoke(logs=logs)
            except BaseException as e:
                # Make sure stepobj gets deleted below.
                stepobj.passed = False
                self.bdata.succeeded = False
                if isinstance(e, lib.util.FatalError) or \
                   isinstance(e, KeyboardInterrupt) or \
                   isinstance(e, lib.util.CalledProcessError):
                    # These are self-explanatory and don't need a traceback.
                    sys.stderr.write('%s\n' % e)
                else:
                    # Any other exceptions.
                    traceback.print_exc()
            finally:
                if not stepobj.passed:
                    # Must address bdata directly here. Not pretty!
                    self.bdata.succeeded = False
                    if not self.badstep:
                        self.badstep = stepname
                        self.badlog = steplog
                # This MUST be deleted in all code paths or the watcher will
                # deadlock the build. Is there a better way to code it?
                del stepobj

            if self.logfiles:
                steps.base.mark_end(stepname, self.logfiles)

            if not self.succeeded:
                break

            print()

            if self.audit and stepname == 'Populate':
                self.audit.start()

    def pub_path(self, path, homedir=False):
        """Return the "public" version of a path - maybe as a URL."""
        if not os.path.isabs(path):
            if self.pubwd:
                path = os.path.join(self.pubwd, path)
            else:
                path = os.path.join(self.bldwd, path)

        # Convert to URL.
        if self.standard:
            path = path.replace('\\', '/')
            if homedir:
                path = path.replace('/home/', '/~')
            path = lib.consts.HTTP_BASE + path

        return path

    def end_mirroring(self):
        """Wait for the mirroring and make sure it's happy."""
        # Make sure we're not in a dir that might go away.
        os.chdir('/tmp')

        self.mirror.stdin.close()
        try:
            if self.mirror.wait():
                self.bdata.succeeded = False
        except Exception as e:
            lib.util.warn('mirror failed: %s' % e)

        self.mirror = None

        # Now it's safe to go back to the NFS location.
        os.chdir(self.pubwd)

    def preserve_built_objects(self):
        """Preserve selected build artifacts"""
        if not self.presdir:
            return

        lib.util.mkdir(self.presdir, vl=2)

        # For backward compatibility we keep two parallel preserve logs,
        # one with the build and the other in the preserve area.
        preslog1 = os.path.join(self.presdir, PRESERVE_LOG)
        plfo = open(preslog1, 'w')
        if os.path.exists(lib.util.RELEASE_LOG_FILE):
            preslog2 = os.path.join(self.pubwd, lib.opts.METADIR, PRESERVE_LOG)
            lib.util.mkdir(os.path.dirname(preslog2), vl=2)
            rlfo = open(preslog2, 'w')
        else:
            rlfo = sys.stderr

        # First, expand any globbing characters in each given path.
        epaths = []
        lineno = 0
        for line in open(PRESERVE_LIST, 'r').readlines():
            lineno += 1

            if line.startswith('#') or len(line) < 9:
                continue

            try:
                _, buildable, path = line.rstrip().split(':', 2)
            except Exception as e:
                msg = '%s: %s:%d: %s' % (e, PRESERVE_LIST, lineno, line)
                for f in [rlfo, plfo]:
                    lib.util.error(msg, ofile=f)
                continue

            if not path:
                continue

            if not re.match(buildable.strip() + '$', self.bldable):
                continue

            gpath = os.path.join(self.pubwd, path.strip())
            expanded = glob.glob(gpath)
            if expanded:
                epaths.extend(expanded)
            else:
                # Don't abort on missing files. PRESERVE_LIST is a filter
                # that grabs whatever subset of named files were produced.
                # Artifacts actually generated will vary by branch/twig.
                for f in [rlfo, plfo]:
                    lib.util.warn('%s: No such file or directory' % gpath,
                                  ofile=f)

        # Then turn the list of paths into a list of files to be preserved,
        # descending into directories as needed.
        fpaths = []
        for epath in epaths:
            if os.path.isdir(epath):
                for parent, _, fnames in os.walk(epath):
                    fpaths.extend(os.path.join(parent, f) for f in fnames)
            else:
                fpaths.append(epath)

        # Next traverse the list of preserved files and copy each one
        # in compressed state.
        for fpath in sorted(fpaths):
            tpath = fpath.replace(self.pubwd, self.presdir)
            lib.util.cp(fpath, tpath, gz=9, out=rlfo, vl=0)
            plfo.write('cp %s %s\n' % (fpath, tpath))

        # Last, always preserve logs too.
        for log in glob.glob(os.path.join(self.pubwd, ',*.log')):
            lib.util.cp(log, self.presdir, gz=9, out=rlfo, vl=3)

        plfo.write('===============================================\n')

    def preserve_metadata(self):
        """Stash logs and other metadata in a separate area."""
        if self.logdir:
            lib.util.mkdir(self.logdir, vl=2)
            metaglob = glob.glob(os.path.join(lib.opts.METADIR, '*'))
            for path in glob.glob('*') + metaglob:
                fname = os.path.basename(path)
                if fname.startswith('_'):
                    continue
                if fname.endswith('.sparse'):
                    continue
                if fname == lib.consts.CONTENTS_HTML:
                    # These are big and re-creatable from the .json
                    # but kept for convenience in release builds.
                    if not lib.scm.is_static(self.tag):
                        continue
                if not os.path.isfile(path):
                    continue
                lib.util.cp(path, self.logdir, gz=9, vl=3)

        # Last, preserve logs into PRESERVED area too. This would
        # be more naturally handled in the Preserve step but logfiles
        # aren't complete at that time.
        if self.presdir and os.path.isdir(self.presdir):
            for log in glob.glob(os.path.join(self.pubwd, ',*.log')):
                lib.util.cp(log, self.presdir, gz=9, vl=3)

    # Ordering is really tricky in shutting down the build
    # gracefully. So be careful when making changes here.
    def finish(self):
        """Close up shop on the build."""
        self.bdata.end_time = time.time()

        if self.audit:
            self.audit.finish(os.path.join(lib.opts.METADIR, BUILD_AUDIT_FILE))

        if self.mirror:
            # Shut down mirroring.
            self.end_mirroring()

        if self.pubwd:
            # Now it's safe to go back to the NFS location.
            os.chdir(self.pubwd)

        # From here we're operating in the pubwd (if there is one), either
        # because we were there all along or because we just cd-ed to it.

        # Stash a copy of the tool with the tag for possible later rebuilds.
        if lib.scm.is_static(self.tag) and self.pubwd:
            os.chdir(lib.consts.TOOLDIR)
            tarball = os.path.join(self.pubwd, lib.opts.METADIR, 'GUB.tar.gz')
            lib.util.mkdir(os.path.dirname(tarball))
            tar = tarfile.open(tarball, 'w:gz')
            # The exclude= is deprecated in 2.7 but we have some 2.6 still.
            for path in glob.glob('*'):
                tar.add(path, exclude=lambda p: '.svn' in p or '.git' in p)
            tar.close()
            os.chdir(self.pubwd)

        # Although the WIP file(s) would be removed during object
        # destruction eventually anyway, do it explicitly here to
        # minimize the window during which status is unclear.
        # In other words the removal of the WIP file and the creation
        # of the ,succeeded file should be as atomic as possible
        # because during an interval when neither exists the build
        # appears failed. If they both exist that would be confusing too.
        self.wip.shutdown()

        # Create the ,build_errors.log file which serves as a "failed" marker
        # or else a ,succeeded file to indicate success. These must exist
        # before the self.preserve*() methods are called.
        if self.succeeded:
            open(SUCCEEDED_FILE, 'w').close()
        elif os.path.isdir(lib.opts.METADIR):
            # Create this redundantly in-process to minimize the race above.
            open(BUILD_ERRORS_FILE, 'w').close()

            # Run the modern error grepper, keeping only the last message
            # since clearly it was the first fatal error.
            if os.path.isfile(lib.util.RELEASE_LOG_FILE):
                cmd = lib.util.xscript(sys.argv[0]) + [
                    'greplog',
                    '-l1',
                    '-o', BUILD_ERRORS_FILE,
                    '--simple-format',
                    lib.util.RELEASE_LOG_FILE,
                ]
                # Ignore status since this doesn't affect the build per se.
                lib.util.execute(cmd, vl=2)

        # Keep a copy of the LSF output for possible future diagnostics.
        if self.lsflog:
            lsfout = lib.lsf.outputfile()
            if lsfout and os.path.exists(lsfout):
                # The LSF log is a nice-to-have; no reason to abort the entire
                # build right at the end just because we couldn't copy it.
                try:
                    lib.util.rm(self.lsflog, vl=3)
                    lib.util.cp(lsfout, self.lsflog, vl=3)
                except Exception:
                    traceback.print_exc()

        self.end_summary = self.get_end_summary()

    def abandon(self, msg):
        """Abort immediately for the specified reason."""
        self.bdata.abandoned = True
        lib.util.die(msg)

    def is_stable(self):
        """Determine whether the build is from a non-floating rev."""
        return self.nightly or lib.scm.is_static(self.tag)

# vim: ts=8:sw=4:tw=80:et:
