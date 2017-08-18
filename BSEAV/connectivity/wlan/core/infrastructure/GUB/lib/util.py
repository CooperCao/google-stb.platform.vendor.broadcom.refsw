"""A collection of utility functions with no other home."""

from __future__ import print_function
from __future__ import unicode_literals

import atexit
import collections
import datetime
import errno
import gzip
import inspect
import os
import pwd
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import time
import urllib

# Try not to import anything else from lib.*. This is a utility module
# which should rely only on core modules (these two are special cases).
import lib.consts
import lib.opts

# So code importing this module doesn't also need subprocess.
PIPE = subprocess.PIPE
STDOUT = subprocess.STDOUT
CalledProcessError = subprocess.CalledProcessError

DELETING = '.DELETING'
RELEASE_LOG_FILE = ',release.log'

SET_LOGIN_ENV_EV = lib.consts.PROG.upper() + '_SET_LOGIN_ENV'


def is_url(path):
    """Return True if the argument looks like a URL."""
    return '://' in path


def mkpath(*args):
    """
    Join and normalize path or URL components.

    When the input looks like a URL this uses / as the separator.
    Otherwise it uses standard native path manipulation functions.
    """

    if is_url(args[0]):
        # Prefer not to use os.path.normpath for URLs. It would remove
        # the // at the beginning and substitute \ on Windows.
        path = '/'.join([p.strip('/') for p in args])
        if args[-1][-1] == '/':
            path += '/'
    else:
        path = os.path.normpath(os.path.join(*args))

    return path


def tmpdir(prefix='', keep=False, mode=None, suffix='.tmp', tmp=None, vl=3):
    """
    Create a temporary directory which by default is removed at exit.

    Normally an atexit handler is set up to remove the temp dir as
    the process exits but in debug mode or when keep=True the dir is
    left behind.  A delay can also be set (e.g. keep=300) which will
    cause it to be removed that many seconds after exit.

    """

    dtemp = tempfile.mkdtemp(prefix=lib.consts.PROG + '.' + prefix,
                             suffix='.%d%s' % (os.getpid(), suffix),
                             dir=tmp)
    xtrace('mkdir %s' % dtemp, vl=vl)

    if mode is not None:
        chmod(dtemp, mode, vl=vl)

    # We do not complain if dtemp is already gone. In other words
    # auto removal still leaves us free to remove earlier if we choose.
    if not lib.opts.DEBUG_MODE:
        if keep is True:
            pass
        elif keep is False:
            atexit.register(rm, dtemp, vl=vl)
        else:
            atexit.register(rm, dtemp, delay=keep, vl=vl)

    return dtemp


def rmchars(text, remove):
    """Remove all chars in the 'remove' string from the 'text' string."""
    for ch in remove:
        text = text.replace(ch, '')
    return text


# Assumes bash user. This has two uses: first, coming out of
# a cron-like environment we need the full PATH etc. Second,
# LSF preserves the environment which can be a mixed blessing;
# we may need to revert to the login env across a bsub especially
# if it's to a different architecture.
def set_login_env():
    """Re-populate environment to login state on request."""
    note('restoring login environment (plus exceptions)', vl=3)

    # Need to clear out most of the current environment in order
    # to get the pristine login env, or close to it anyway.
    # The penv dict is the set of EVs to preserve from current env.
    preserve = set(['HOME', 'LOGNAME', 'USER'])
    penv = {}
    for name in os.environ:
        if name in preserve:
            penv[name] = os.environ[name]
        elif name.startswith(lib.consts.PROG.upper() + '_'):
            penv[name] = os.environ[name]
        elif name.startswith('LSB_') or name.startswith('LSF_'):
            penv[name] = os.environ[name]
        elif name.startswith('LS_') and name != 'LS_COLORS':
            penv[name] = os.environ[name]
    penv['PATH'] = '/usr/bin:/bin'

    # Shell handling is particularly tricky.
    penv['SHELL'] = os.environ.get('SHELL')

    # Cron _always_ sets SHELL=/bin/sh so we undo that here.
    if not penv['SHELL'] or penv['SHELL'] == '/bin/sh':
        shell = pwd.getpwnam(os.environ['LOGNAME']).pw_shell
        os.environ['SHELL'] = penv['SHELL'] = shell

    # This only works for shells which support a -l flag but
    # that includes at least bash, zsh, and tcsh.
    # Lots of people have sloppy logic in their shell rc files, such as
    # stty commands that run even when there's no tty. So we suppress
    # stderr unless the command actually fails.
    if 'csh' in penv['SHELL']:
        cmd = [penv['SHELL'], '-l']
        proc = subprocess.Popen(cmd, env=penv,
                                stdin=PIPE,
                                stdout=PIPE,
                                stderr=PIPE)
        proc.stdin.write('env\n')
    else:
        cmd = [penv['SHELL'], '-l', '-c', 'env']
        proc = subprocess.Popen(cmd, env=penv,
                                stdout=PIPE,
                                stderr=PIPE)

    out, err = proc.communicate()
    if proc.returncode:
        sys.stderr.write(err)
        raise CalledProcessError(proc.returncode, cmd)

    keepers = []
    goodline = True
    for line in out.splitlines():
        # The &#@$% tcsh prints an error message to stdout here.
        if '=' not in line:
            continue
        name, value = line.split('=', 1)
        if name[0].isupper() and name.isupper():
            keepers.append((name, value.rstrip()))
            goodline = True
        else:
            # Deal with multiline env values by dropping them.
            if goodline and keepers:
                keepers.pop(-1)
            goodline = False

    for ev in sorted(keepers):
        name = ev[0]
        value = ev[1]
        os.environ[name] = value


def path_whence(fname, env=None, full=False, xbit=True, pathvar='PATH'):
    """Find the specified executable file on the specified path."""
    found = []

    if os.sep in fname:
        if not xbit or os.access(fname, os.X_OK):
            found.append(fname)
    else:
        if not env:
            env = os.environ
        for entry in env.get(pathvar, '').split(os.pathsep):
            path = os.path.join(entry, fname)
            if os.path.exists(path) and (not xbit or os.access(path, os.X_OK)):
                found.append(path)
                if not full:
                    break

    if full:
        return found
    else:
        return found[0] if found else None


def path_add(entries, env=None, front=False, pathvar='PATH', replace=True):
    """Add a list of entries to a PATH variable."""
    if not env:
        env = os.environ

    updated = env.get(pathvar, '').split(os.pathsep)
    for entry in entries:
        if replace:
            while entry in updated:
                updated.remove(entry)
        if front:
            updated.insert(0, entry)
        else:
            updated.append(entry)

    env[pathvar] = os.pathsep.join(updated)


def path_clean(env=None, missing=False, pathvar='PATH', remove=None):
    """Normalize, and optionally remove entries from, a PATH variable."""
    if not env:
        env = os.environ

    if pathvar in env:
        updated = []
        for entry in env[pathvar].split(os.pathsep):
            if not remove or entry not in remove:
                if not missing or os.path.isdir(entry):
                    if entry not in updated:
                        updated.append(entry)

        if updated:
            newpath = os.pathsep.join(updated)
            if newpath != env[pathvar]:
                env[pathvar] = newpath
        else:
            del env[pathvar]


def cp(src, dst, gz=None, out=None, vl=1):
    """Copy a single file with optional gzipping."""
    if os.path.isdir(dst):
        dst = os.path.join(dst, os.path.basename(src))
    else:
        mkdir(os.path.dirname(dst), vl=vl if vl else 3)

    if gz is None or src.endswith('.gz') or src.endswith('.tgz'):
        xtrace('cp %s %s' % (src, dst), out=out, vl=vl)
        shutil.copy(src, dst)
    else:
        if not dst.endswith('.gz'):
            dst += '.gz'
        xtrace('gzip -c -%d %s > %s' % (gz, src, dst), out=out, vl=vl)
        f_src = open(src, 'rb')
        f_dst = gzip.open(dst, 'wb', compresslevel=gz)
        f_dst.writelines(f_src)

    shutil.copymode(src, dst)


def mv(src, dst, vl=1):
    """Move a file or directory."""
    if os.path.isdir(dst):
        dst = os.path.join(dst, os.path.basename(src))
    else:
        mkdir(os.path.dirname(dst), vl=vl if vl else 3)

    xtrace('mv %s %s' % (src, dst), vl=vl)
    shutil.move(src, dst)


def touch(path, asof=None):
    """Change mtime/atime of path, recursively for dirs."""
    now = (asof if asof else time.time())
    if os.path.isdir(path):
        for parent, dnames, fnames in os.walk(path):
            for name in dnames + fnames:
                subpath = os.path.join(parent, name)
                xtrace('touch ' + subpath, vl=3)
                os.utime(subpath, (now, now))
    else:
        xtrace('touch ' + path, vl=3)
        os.utime(path, (now, now))


def normalize_timestamps(basedir, asof=None):
    """Normalize file timestamps within a build tree."""
    # This is a hack to work around a problem that dev teams
    # should really own. When open-source code containing
    # 'configure' scripts is checked in to our codebase
    # sometimes an aclocal.m4 is also checked in.
    # When both are present, it seems random which one is
    # newer in a given svn checkout and thus random whether
    # configure is regenerated from aclocal.m4. Since we
    # have varying autoconf versions in house, all heck then
    # breaks loose. The workaround is to touch all configure
    # files to make sure they're newer than their prereqs.
    # Update: since this seems to have worked for configure,
    # it may also work for Makefiles (see JIRA
    # http://jira.broadcom.com/browse/SWSCM-162). I.e.
    # as configure scripts are to autoconf, so Makefiles
    # are to automake. Thus this function gives all files in a
    # new checkout the same timestamp of "now", except that
    # Makefile and configure are made slightly newer.

    now = (asof if asof else time.time())
    for parent, _, fnames in os.walk(basedir, topdown=False):
        for fname in sorted(fnames):
            path = os.path.join(parent, fname)
            # Don't fail if unable to update timestamp.
            try:
                if fname == 'Makefile':
                    xtrace('touch ' + path, vl=3)
                    os.utime(path, (now, now))
                elif fname == 'configure':
                    xtrace('touch ' + path, vl=3)
                    os.utime(path, (now - 1, now - 1))
                else:
                    os.utime(path, (now - 2, now - 2))
            except OSError as e:
                if not os.path.islink(path):
                    warn(str(e))


def nfsflush(path, creat=False, down=False, up=False):
    """Ask NFS to flush its file handle cache for the named directory."""
    # See http://icecap.irssi2.org/nfs-coding-howto.html#fhcache
    # which discusses multiple theories about how to force a flush.
    # The opendir/closedir is the one that seems to work reliably here.

    # Optionally flush parent directories too - parents should be
    # flushed before their subdirs.
    parent = os.path.dirname(os.path.abspath(path))
    if parent == os.path.dirname(parent):
        return

    if up:
        nfsflush(parent, down=False, up=True)

    note('nfsflush(%s)' % path, vl=3)

    try:
        # Create and remove a file to update the dir's mtime (theory 1).
        if creat:
            tempfile.NamedTemporaryFile(dir=path, prefix='nfsflush.').close()

        # Force an opendir/closedir sequence to flush it (theory 2).
        len(os.listdir(path))

        # Can't trust the first opendir because it was pre-flush, so use
        # use a second one to traverse downward.
        if down:
            for entry in os.listdir(path):
                subdir = os.path.join(path, entry)
                if os.path.isdir(subdir) and not os.path.islink(subdir):
                    nfsflush(subdir, down=True, up=False)
    except EnvironmentError:
        pass


def realpath(path):
    """Wrapper over OS function with workaround for UNC bug."""
    if path[0] in '\\/' and path[1] == path[0]:
        path = os.path.realpath(path)
        if path[1] != path[0]:
            path = path[0] + path
    else:
        path = os.path.realpath(path)

    return path


def shortpath(path, start='.'):
    """Return the shorter of os.path.{relpath,abspath}(path)."""
    spath = os.path.relpath(path, start=start)
    if spath.startswith('../'):
        apath = os.path.abspath(path)
        if len(apath) < len(spath):
            spath = apath

    return spath


def link(oldpath, newpath, force=False, parents=False,
         relpath=True, symbolic=False, vl=2):
    """ Create a link (symbolic or hard) with optional verbosity."""
    if parents and not os.path.exists(os.path.dirname(newpath)):
        mkdir(os.path.dirname(newpath), vl=vl if vl else 3)

    if os.path.isdir(newpath):
        newpath = os.path.join(newpath, os.path.basename(oldpath))

    # The hacking here is an attempt to alleviate races between
    # unlink and link when multiple jobs are trying to create
    # the same base-level symlink. First we try to avoid replacing
    # the link if it already exists with the right value ...
    try:
        nps = os.stat(newpath)
    except EnvironmentError:
        pass
    else:
        try:
            ops = os.lstat(oldpath)
        except EnvironmentError:
            pass
        else:
            if nps.st_dev == ops.st_dev and nps.st_ino == ops.st_ino:
                return

    if symbolic:
        if relpath:
            oldpath = os.path.relpath(oldpath, os.path.dirname(newpath))
        lnfunc = os.symlink
        xtrace('ln -s %s %s' % (oldpath, newpath), vl=vl)
    else:
        lnfunc = os.link
        xtrace('ln %s %s' % (oldpath, newpath), vl=vl)

    if force:
        # ... then if necessary we update links using the rename() system
        # call which is atomic and guaranteed to overwrite the new path.
        tmppath = '%s.%d' % (newpath, os.getpid())
        lnfunc(oldpath, tmppath)
        os.rename(tmppath, newpath)
    else:
        lnfunc(oldpath, newpath)


def export(*args, **kwargs):
    """Export an environment variable with optional verbosity."""
    if len(args) == 1:
        name, value = args[0].split('=', 1)
    else:
        name = args[0]
        value = str(args[1])

    update = kwargs.get('update', True)
    vl = kwargs.get('vl')

    if not value or value == 'None':
        if update and name in os.environ:
            xtrace('unexport %s' % name, vl=vl)
            del os.environ[name]
    elif name in os.environ and not update:
        pass
    elif os.environ.get(name) != value:
        xtrace('export %s=%s' % (name, value), vl=vl)
        os.environ[name] = value


def umask(newmask, stricter=True):
    """Change and/or return the umask."""
    if newmask is None:
        oldmask = os.umask(0o22)
        os.umask(oldmask)
    else:
        # This is exported so that a cooperating user can use
        # "umask ${UMASK:-022}" in shell config files.
        export('UMASK', '%.4o' % newmask, vl=3)
        oldmask = os.umask(newmask)
        if newmask != oldmask:
            if newmask > oldmask:
                if stricter:
                    xtrace('umask %.4o => %.4o' % (oldmask, newmask), vl=3)
                else:
                    os.umask(oldmask)
            else:
                xtrace('umask %.4o => %.4o' % (oldmask, newmask), vl=3)

    return oldmask


def chdir(path, vl=1):
    """Change the CWD with optional verbosity."""
    xtrace('cd ' + path, vl=vl)
    os.chdir(path)


def chmod(*args, **kwargs):
    """Change the listed paths to the (numeric) mode."""
    paths = list(args)
    mode = paths.pop()
    xtrace('chmod 0%o %s' % (mode, ' '.join(paths)), **kwargs)
    for path in paths:
        os.chmod(path, mode)


def mkdir(*paths, **kwargs):
    """Make sure the specified directories exist."""
    # This function does not complain if the dir already exists.

    needed = [p for p in paths if p and
              (kwargs.get('force') or not os.path.isdir(p))]

    if needed:
        xtrace('mkdir -p %s' % ' '.join(needed), vl=kwargs.get('vl', 1))

    for path in needed:
        # Protect against two jobs creating the same parent structure,
        # or a race from just above.
        try:
            os.makedirs(path, mode=kwargs.get('mode', 0o777))
        except OSError:
            time.sleep(1)
            if not os.path.isdir(path):
                os.makedirs(path, mode=kwargs.get('mode', 0o777))


def rm(*paths, **kwargs):
    """ Remove specified files or dirs with optional verbosity.

    More precisely, the goal is to ensure that the specified paths do
    not exist; if they're already gone no complaint is made.
    """

    bg = kwargs.get('bg', False)
    delay = int(kwargs.get('delay', 0))
    ignore_errors = kwargs.get('ignore_errors', False)
    vl = kwargs.get('vl', 1)

    # Handle lists of pathnames as well as explicit pathnames.
    # If there's nothing to do (meaning none of the pathnames exist),
    # do nothing silently.
    if paths:
        paths_ = []
        for p1 in paths:
            if isinstance(p1, basestring):
                if os.path.exists(p1) or os.path.islink(p1):
                    paths_.append(p1)
            else:
                for p2 in p1:
                    if os.path.exists(p2) or os.path.islink(p2):
                        paths_.append(p2)
        paths = paths_

    if not paths:
        return

    # Determine whether verbose mode should show "rm -f" or "rm -rf".
    flag = '-f'
    for path in paths:
        if path and os.path.isdir(path) and not os.path.islink(path):
            flag = '-rf'

    # Sometimes we need files to be around for a while then cleaned up
    # automatically after some period of time. For this we use the
    # real "rm" command as a background process.
    if delay or bg:
        if delay:
            pfx = 'exec </dev/null >/dev/null 2>/dev/null; /bin/sleep'
            cmd = '%s %d && /bin/rm %s %s' % (pfx, delay, flag, cmdline(paths))
            xtrace(cmd, bg=True, vl=vl)
            subprocess.Popen(cmd, close_fds=True, shell=True)
        else:
            cmd = cmdline(['rm', flag] + paths)
            xtrace(cmd, bg=bg, delay=delay, vl=vl)
            subprocess.Popen(cmd, preexec_fn=lambda: os.nice(2), shell=True)
        return

    xtrace('rm %s %s' % (flag, ' '.join(paths).strip()),
           bg=bg, delay=delay, vl=vl)
    for path in paths:
        if not path:
            continue
        if os.path.isdir(path) and not os.path.islink(path):
            if DELETING in path:
                delpath = path
            else:
                fmt = '%Y%m%dT%H%M%S'
                delpath = '%s%s-%s' % (path.rstrip('/'), DELETING,
                                       datetime.datetime.now().strftime(fmt))

            if sys.platform.startswith('cyg'):
                # This is because the rename often fails on Cygwin.
                # Not sure why, best guess is that there's a timing problem
                # releasing a lock. I've often seen Windows files remain
                # locked for a little while after they're closed
                # (renaming after the failure works fine).
                delpath = path
            else:
                os.rename(path, delpath)

            shutil.rmtree(delpath, ignore_errors=ignore_errors)
        elif os.path.exists(path) or os.path.islink(path):
            try:
                os.unlink(path)
            except Exception as e:
                if not ignore_errors:
                    raise e


def rm_empty_parent_dirs(pdir, vl=1):
    """Remove any empty parent directories of the specified path."""
    while True:
        pdir = os.path.dirname(pdir)
        try:
            if os.path.islink(pdir):
                break
            os.rmdir(pdir)
        except OSError:
            break
        else:
            xtrace(['rmdir', pdir], vl=vl)


def rm_older_than(topdir, older_than, depth=1, ignore_errors=False):
    """Remove files or dirs more than specified seconds old."""
    # Conceivably two processes could be cleaning at the same
    # time, causing a spurious error, so we retry quietly once.
    # Even then an error doesn't abort the current process.
    # This code will also clean up dangling symlinks.
    now = time.time()
    for entry in os.listdir(topdir):
        path = os.path.join(topdir, entry)
        if depth > 1 and os.path.isdir(path):
            rm_older_than(path, older_than, depth=depth - 1)
        try:
            stats = os.lstat(path)
            if now - stats.st_mtime > older_than:
                rm(path, ignore_errors=ignore_errors, vl=3)
        except OSError as e:
            if os.path.exists(path):
                try:
                    rm(path, ignore_errors=ignore_errors)
                except OSError:
                    if os.path.exists(path):
                        error(str(e))


# Hacking with literal flags is fragile but sometimes necessary.
def xargv(argv, rm1=None, rm2=None, after=None):
    """Remove some arguments/flags from the argv and add others."""
    # Iterate over argv removing the rm1 and rm2 sets. The rm1 set is for
    # single-word options ("--flag") and rm2 is for double words ("--opt xyz").
    result = []
    for i, word in enumerate(argv):
        if i == 0:
            result.append(os.path.normpath(word))
        else:
            prev = argv[i - 1]
            if rm1 and word in rm1:
                continue
            if rm2:
                if prev in rm2 or word in rm2:
                    continue
                elif word.split('=')[0] in rm2:
                    continue
            result.append(word)

    # Due to a bug(?) in argparse, words of the form "FOO=bar"
    # cannot be intermingled with options so must be kept at
    # the right end. But working out the difference between a
    # standalone assignment and "-x foo=bar" is hard, so we
    # try to insert the new flags as far left as possible.
    # This doesn't matter any more because there are no
    # standalone FOO=bar assignments but the comment above is
    # kept to explain why it's done this way.
    if after:
        for post in after:
            for i, word in enumerate(result):
                if word.endswith(post) or i == len(result) - 1:
                    if i == len(result) - 1:
                        result += after[post]
                    else:
                        i = i + 1
                        result = result[:i] + after[post] + result[i:]
                    break

    return result


def is_local_fs(path):
    """
    Return True iff the specified path is locally mounted.

    Works even for a path which does not (yet) exist by visiting its parent
    directory recursively.

    """

    # There doesn't seem to be an API to get this directly.
    # We could parse /etc/mtab and analyze the path but there are
    # platform differences. Simplest and most portable I've come
    # up with is this.

    if not path:
        return False

    path = os.path.abspath(path)

    if path.startswith('/projects'):
        # Optimization.
        answer = False
    elif path.startswith('/tmp/') or path.startswith('/ec_'):
        # Optimization.
        answer = True
    elif path.startswith('/home/') and sys.platform.startswith('cyg'):
        # We mount /home on Cygwin but Cygwin df reports mounts strangely.
        # Might be more robustly fixed by parsing df output.
        answer = False
    elif os.path.dirname(path) == path:
        # At the root.
        answer = True
    elif os.path.exists(path):
        # OK, looks like we must invoke the df command.
        # The extra '/' is needed to avoid error.
        cmd = ['df', '-l', path, '/']
        proc = subprocess.Popen(cmd, stdout=PIPE)
        output = proc.stdout.read()
        answer = proc.wait() == 0 and output.count('\n') == 3
    else:
        # Recurse.
        answer = is_local_fs(os.path.dirname(path))

    return answer


def mounted_from(paths):
    """Return the hosts from which these paths are mounted (None if local)."""
    mountmap = {}
    if paths:
        cmd = ['df', '-P'] + paths
        proc = execute(cmd, stdout=PIPE, vl=2)
        stdout = proc.communicate()[0]
        proc.wait()
        lines = stdout.splitlines()[1:]
        for i, line in enumerate(lines):
            path = paths[i]
            mntfrom = line.split()[0]
            if ':' in mntfrom:
                mountmap[path] = mntfrom.split(':', 1)
            else:
                mountmap[path] = [mntfrom, None]
    return mountmap


def makejobs(jx=2.0, lx=None, jobmax=0):
    """
    Determine reasonable parallelism values for GNU make.

    Return a tuple (-j, -l) of recommended parallelism and load limits.
    When within LSF it defers to the LSF slot reservation system
    for load limiting. Outside of LSF the number of jobs is a function
    of the number of CPUs the host has.

    The load value is primarily a belt-and-suspenders safety limit. In
    other words, though on a shared system a given load limit per
    user/task might make sense, on a build server dedicated to the
    current build it would not.  Similarly we cannot determine relative
    priorities of running tasks here. Thus the only goal of the -l
    value is to keep the host from getting absolutely swamped.

    """

    if os.environ.get('LSB_HOSTS'):
        # Determine how many LSF slots have been allocated on this host.
        host = lib.consts.HOSTNAME.split('.')[0]
        slots = len([h for h in os.environ['LSB_HOSTS'].split() if h == host])
        # And apply the multiplier to that value.
        max_jobs = slots * jx
        # The slot reservation system should keep load under control
        # automatically within LSF.
        lval = 0
    else:
        # We're either not in LSF or didn't get a slot reservation
        # for some reason (which shouldn't happen). Fall back to
        # something reasonably fast but neighborly.
        if os.environ.get('LSF_VERSION'):
            warn('LSF build with no slot reservation')
            max_jobs = min(lib.consts.CPUS * jx, 12)
        else:
            max_jobs = max(lib.consts.CPUS * jx, 1)

        # Load average limiting is just a safety net to avoid really
        # pathological situations. Ensure limit is at least 2X jobs.
        max_load = lib.consts.CPUS * (lx if lx else 4)
        lval = (max(max_load, max_jobs * 2) if max_load else 0)

    # Apply an overall limit to jobs if requested.
    if jobmax > 0:
        jval = min(max(max_jobs, 1), jobmax)
    else:
        jval = max(max_jobs, 1)

    return int(jval), int(lval)


def tolist(item, default=None, sep=None):
    """
    Subdivide something which may already be a list by whitespace.

    To allow users to consistently specify either of "-x foo -x bar"
    or "-x 'foo bar'", we tend to define argparse options as creating
    a list and post-process the result through this.
    """

    result = []
    if not item:
        item = default
    if item:
        if isinstance(item, list):
            for word in item:
                result.extend(word.split(sep))
        else:
            result.extend(item.split(sep))

    return result


def shellsafe(text):
    """Return True iff the text can be safely exposed to a POSIX shell."""
    # The RE here selects inert characters. Anything else should be quoted.
    # Special case - a tilde is inert except when leading.
    if text[0] != '~' and re.match(r'^[-+~\w/.%@,:{}=]+$', text):
        return True
    else:
        #  Another special case - a leading caret does not require quoting.
        return text[0] == '^' and '^' not in text[1:]


def quoted(stuff):
    """Quote the provided list or string against shell expansion."""
    q1 = "'"
    q2 = '"'

    listin = isinstance(stuff, list)
    items = (stuff if listin else [stuff])
    result = []

    for item in items:
        if isinstance(item, list):
            result.append(' '.join(quoted(item)))
            continue

        # In case we were passed an integral value.
        item = '%s' % item

        if shellsafe(item):
            result.append(item)
        elif any(c in '&|;' for c in item):
            # If these are present they're probably intended for the shell.
            result.append(item)
        elif q1 in item:
            result.append(q2 + item + q2)
        else:
            # Cosmetic: quote assignments as foo='bar', not 'foo=bar'.
            match = re.match(r'^(\w+=)(\S+.*)', item)
            if match:
                result.append(match.group(1) + q1 + match.group(2) + q1)
            else:
                result.append(q1 + item + q1)

    return result if listin else result[0]


def url_encode(text):
    """URL-encode the provided text."""
    return urllib.quote_plus(text) if text else text


def url_decode(text):
    """URL-decode the provided text."""
    return urllib.unquote_plus(text) if text else text


def cmdline(cmd=None):
    """Convert the provided list to a quoted shell command line."""
    if not cmd:
        cmd = sys.argv

    if isinstance(cmd, list):
        cmdstr = ' '.join(quoted(cmd))
        # Special case to make verbose svn command output line up better.
        for substr in ['--depth=empty', '--depth=files']:
            ptr = cmdstr.find(substr)
            if ptr != -1:
                ptr += len(substr)
                cmdstr = cmdstr[:ptr] + '   ' + cmdstr[ptr:]
    else:
        cmdstr = '%s' % cmd

    # Special case due to argparse limitations in re-exec.
    cmdstr = cmdstr.replace('--makeflags ', '--makeflags=')
    return cmdstr


def xscript(script=None, shell=False, vd=None):
    """Return a reliable way of running a child python script from this one."""
    cmd = [
        sys.executable,
        os.path.join(lib.consts.TOOLDIR,
                     'bin',
                     script if script else lib.consts.PROG)
    ]

    if vd is not None:
        delta = lib.opts.VERBOSITY - vd
        if delta > 0:
            cmd.append('-V')
            for _ in range(2, delta):
                cmd[-1] += 'V'

    if shell:
        return cmdline(cmd)
    else:
        return cmd


def boolean(term):
    """
    Evaluate the term as true/false.

    Interpret any of "{False,Off,No,None,Null}" (case insensitive)
    or 0 or the null string to mean False. Everything else is True.
    """

    if term:
        if isinstance(term, basestring):
            tl = term.lower()
            if tl == 'false' or tl == 'off' or tl == 'no' \
               or tl == 'none' or tl == 'null':
                term = 0
        try:
            result = int(term) != 0
        except ValueError:
            result = True
    else:
        result = False

    return result


def get_owner(path):
    """Return the owner of the specified file."""
    owner = pwd.getpwuid(os.stat(path).st_uid).pw_name
    return owner


def verbose_enough(vl):
    """Return True if the current verbosity should be printed."""
    return vl >= 0 and lib.opts.VERBOSITY >= vl


def message(msg, *args, **kwargs):
    """Return message in standard format."""
    prog = kwargs.get('prog', lib.consts.PROG)
    msgtype = kwargs.get('msgtype')
    if msgtype:
        text = '%s: %s: %s' % (prog, msgtype, msg % args)
    else:
        text = '%s: %s' % (prog, msg % args)
    return text.rstrip()


def error(msg, *args, **kwargs):
    """Print error message in standard format."""
    kwargs.setdefault('msgtype', 'Error')
    ofile = kwargs.get('ofile', sys.stderr)
    ofile.write(message(msg, *args, **kwargs) + '\n')
    return False


def warn(msg, *args, **kwargs):
    """Print warning message in standard format."""
    msg = re.sub(r'\[Errno \d+\]\s*', '', msg)
    error(msg, msgtype='Warning', *args, **kwargs)


def note(msg, *args, **kwargs):
    """Print message in standard format according to verbosity level."""
    if verbose_enough(kwargs.get('vl', 1)):
        ofile = kwargs.get('ofile', sys.stderr)
        ofile.write(message(msg, *args, **kwargs) + '\n')


def dbg(msg, *args, **kwargs):
    """Print debug verbosity in standard format."""
    if verbose_enough(kwargs.get('vl', 1)):
        fi = inspect.getframeinfo(inspect.currentframe().f_back)
        path, lineno = (fi.filename, fi.lineno)
        dname = os.path.basename(os.path.dirname(path))
        fname = os.path.join(dname, os.path.basename(path))
        kwargs['msgtype'] = 'At %s:%d' % (fname, lineno)
        sys.stderr.write(message(msg, *args, **kwargs) + '\n')


class FatalError(Exception):
    """An explicit error as thrown by the code."""

    def __init__(self, msg):
        Exception.__init__(self, msg)


def die(msg, *args, **kwargs):
    """
    Print error message in standard format and die.

    This can die by raising an exception or just exiting.  The exception
    can be used to parachute out of a complex, non-interactive task such
    as a build whereas the exit may be preferred for simple user tools.
    """
    kwargs.setdefault('msgtype', 'Error')
    if kwargs.get('exc', True) is False:
        ofile = kwargs.get('ofile', sys.stderr)
        ofile.write(message(msg, *args, **kwargs) + '\n')
        sys.exit(2)
    raise FatalError(message(msg, *args, **kwargs))


def assert_(condition, msg, exc=True):
    """Die with a standard-format message if condition is untrue."""
    if not condition:
        die(msg, exc=exc)


def xtrace(msg, bg=False, cwd=None, delay=0,
           out=None, pfx=None, vl=None):
    """Print commands according to configured verbosity level (vl)"""
    if vl is None:
        vl = 1

    if verbose_enough(vl):
        line = (pfx if pfx else '+')

        # If this action takes place in some other directory, show it.
        if cwd:
            # Deal with the possibility of going through a symlink and getting
            # back to the same place.
            l = os.stat('.')
            r = os.stat(cwd)
            if l.st_dev != r.st_dev or l.st_ino != r.st_ino:
                line += ' [%s]' % shortpath(cwd)

        line += ' '
        if verbose_enough(4):
            line += '(%s) <%s> [%s] ' % (
                lib.consts.HOSTNAME,
                time.ctime(),
                lib.consts.PROG,
            )

        if delay:
            line += 'sleep %d && ' % delay

        line += cmdline(msg)

        if bg or delay:
            line += ' &'

        if out is None:
            out = sys.stderr

        # Attempt to avoid internal buffers and use the write() system
        # call to prevent scrambled verbosity in parallel/threaded work.
        os.write(out.fileno(), line + '\n')


def get_file_contents_if_utf8(file_path):
    """Return file content if UTF-8 encoded, else None (assume binary)."""
    with open(file_path) as f:
        try:
            contents = f.read().decode('utf-8', 'strict')
        except UnicodeDecodeError:
            contents = None

    return contents


def cygpath(path, flags=None):
    """Normalize a path for Cygwin."""
    cmd = ['cygpath']
    if flags:
        cmd.extend(flags)
    else:
        cmd.append('-m')

    cmd.append(path)
    proc = execute(cmd, stdout=PIPE, vl=2)
    output = proc.communicate()[0]
    return None if proc.returncode else output.strip()


def am_builder():
    """Return True iff the current user is the privileged builder."""
    return lib.consts.STATE.by == lib.consts.BUILDER


def bytestrings(datum):
    """Convert data containing unicode strings to byte strings."""
    if isinstance(datum, basestring):
        return datum.encode('utf-8')
    elif isinstance(datum, collections.Mapping):
        return dict([bytestrings(x) for x in datum.iteritems()])
    elif isinstance(datum, collections.Iterable):
        return type(datum)([bytestrings(x) for x in datum])
    else:
        return datum


def pipeto(cmd):
    """Send upcoming stdout through the specified command."""
    if os.isatty(sys.stdin.fileno()):
        rfd, wfd = os.pipe()
        if os.fork():
            # parent
            os.close(wfd)
            os.dup2(rfd, sys.stdin.fileno())
            xtrace(['|'] + cmd, vl=3)
            os.execvp(cmd[0], cmd)
        else:
            # child
            os.close(rfd)
            os.dup2(wfd, sys.stdout.fileno())


def pager():
    """Send upcoming stdout through the pager."""
    pipeto([os.environ.get('PAGER', 'more')])


def execute_parallel(cmds, **kwargs):
    """
    Use lib.util.execute() in parallel.

    Take a list of commands and run them simultaneously. The 'jobs='
    parameter controls how many can run at a time. The exit status
    returned is that of the first command to fail (if any failed) and
    zero otherwise.

    By default, output from jobs flows unimpeded to stdout but the
    'func' parameter may be used to refer to a callable which handles
    the output.  This can be used to post-process the output and/or to
    prevent output of parallel jobs being scrambled.
    """

    rc = 0
    jobs = kwargs.pop('jobs', 1)
    output_func = kwargs.pop('func', None)
    if output_func:
        kwargs['stdout'] = PIPE
    kwargs['parallel'] = True
    kwargs['bg'] = jobs > 1 and len(cmds) > 1
    ready = cmds[:]
    running = {}
    while ready or running:
        if ready and len(running) < jobs:
            cmd = ready.pop(0)
            proc = execute(cmd, **kwargs)  # pylint: disable=star-args
            running[proc.pid] = proc
        elif running:
            pid, status = os.wait()
            if output_func:
                output_func(running[pid].stdout)
            if pid in running:
                del running[pid]
            if rc == 0:
                rc = status
    return rc


def execute(cmd, bg=False, check=False, cwd=None, env=None, execvp=False,
            interactive=False, pfx=None, parallel=False,
            stdin=None, stdout=None, stderr=None,
            timeout=0, vl=1):
    """
    Invoke a child process with optional verbosity.

    This is the generic process invoker. Except in very special cases
    all child processes should be run through here to standardize
    verbosity. It's just a wrapper over subprocess.Popen() but it keeps
    other modules from needing to "import subprocess" and adds some
    useful convenience features as detailed below:

    It implements exec verbosity (similar in style to "bash -x").
    It determines automatically whether a shell is needed. It allows
    the command to be passed as a multi-word sequence even when the
    command is a list, e.g. "cmd = ['hnd gclient sync', '--transitive']".

    A timeout for the child process can be requested, and the check
    feature a la subprocess.check_call() is supported via the "check"
    keyword.

    If given pipe redirections for stdin, stdout, or stderr it will
    return the process object in order to allow those pipes to be
    manipulated by the caller. Otherwise it will wait for the process
    itself and return only the exit status. Thus, when passing any of
    st{in,out,err} it's necessary for the caller to wait for the child.

    To spin off a command in the background, just pass bg=True and
    ignore the return.
    """

    rc = 0

    if isinstance(cmd, basestring):
        shell = True
        cmd = cmd.strip()
    else:
        shell = False
        cmd = cmd[0].split() + cmd[1:]

    if isinstance(stdout, basestring):
        stdout = open(stdout, 'w')

    if isinstance(stderr, basestring):
        stderr = open(stderr, 'w')

    if env:
        env = env.copy()
        for k in env:
            if vl >= 0:
                if k not in os.environ or env[k] != os.environ[k]:
                    xtrace('(export %s=%s)' % (k, env[k]), vl=vl + 1)
    else:
        env = os.environ.copy()

    # Certain EV's are not useful in automated processes and can
    # cause trouble, especially if their values contain newlines.
    for ev in ['_', 'TERMCAP', 'module']:
        if ev in env:
            del env[ev]

    # Don't need this gift from LSF.
    if sys.platform.startswith('cyg'):
        env.pop('LD_LIBRARY_PATH', None)

    try:
        if shell and cmd.startswith('#'):
            xtrace(cmd, cwd=cwd, vl=vl, pfx='-')
        elif bg or parallel or stdin or stdout == PIPE or stderr == PIPE:
            assert check is False and execvp is False
            xtrace(cmd, bg=bg, cwd=cwd, pfx=pfx, vl=vl)
            if not stdin:
                stdin = open(os.devnull)
            proc = subprocess.Popen(cmd,
                                    cwd=cwd,
                                    env=env,
                                    shell=shell,
                                    stdin=stdin,
                                    stdout=stdout,
                                    stderr=stderr)
            rc = proc
        else:
            xtrace(cmd, cwd=cwd, vl=vl, pfx=pfx)
            if execvp:
                # These are doable but there's no need yet.
                assert not shell and not stdout and not stderr
                if cwd:
                    os.chdir(cwd)
                os.execvpe(cmd[0], cmd, env)
            else:
                if not (interactive or lib.opts.DEBUG_MODE):
                    stdin = open(os.devnull)

                proc = subprocess.Popen(cmd,
                                        cwd=cwd,
                                        env=env,
                                        shell=shell,
                                        stdin=stdin,
                                        stdout=stdout,
                                        stderr=stderr)

                # Set an alarm to handle timeouts if requested.
                if timeout:
                    def timeout_handler(signum, frame):
                        """Time out the running command if requested."""
                        signal.alarm(0)
                        error('timeout pid=%d after %s minutes' %
                              (proc.pid, timeout))
                        # Partly to suppress pylint complaints.
                        note('SIG=%d at: %r\n' % (signum, frame), vl=2)
                        proc.terminate()
                        time.sleep(1)
                        proc.kill()

                    signal.signal(signal.SIGALRM, timeout_handler)
                    signal.alarm(int(timeout * 60))

                    # Handle possible death race.
                    try:
                        rc = proc.wait()
                    except OSError as e:
                        if e.errno == errno.ESRCH or e.errno == errno.ECHILD:
                            rc = -signal.SIGTERM
                        else:
                            raise e
                    finally:
                        signal.alarm(0)
                else:
                    rc = proc.wait()

    except OSError as e:
        rc = e.errno
        if timeout or not shell:
            raise OSError(rc, cmd)

    if rc and check:
        # Convert command to byte strings before reporting (looks better).
        if isinstance(cmd, basestring):
            raise CalledProcessError(rc, str(cmd))
        else:
            raise CalledProcessError(rc, [str(c) for c in cmd])

    return rc

# vim: ts=8:sw=4:tw=80:et:
