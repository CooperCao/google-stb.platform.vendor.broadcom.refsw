"""A module to support parallel copy operations over NFS."""

from __future__ import print_function
from __future__ import unicode_literals

import Queue
import collections
import errno
import hashlib
import logging
import multiprocessing
import os
import re
import shutil
import stat
import time
import threading
import traceback

import lib.walk

DEFAULT_JOBS = min(multiprocessing.cpu_count() * 4, 16)

SCM_DIRS = set(['.git', '.svn', 'CHECKOUT'])


class TreeSync(object):

    """Manage the unidirectional, threaded synchronization of two dir trees."""

    def __init__(self, src, dst,
                 backup=False,
                 excludes=None,
                 excludes_bg=None,
                 exclude_scm=0,
                 preserve_time=False,
                 priority=1.0,
                 protect=False):
        self.errno = None
        self.srcdir = (os.path.realpath(src) if src else None)
        self.dstdir = (os.path.realpath(dst) if dst else None)
        self.backup = backup
        excludes = (excludes if excludes else [])
        excludes_bg = (excludes_bg if excludes_bg else [])
        excludes.extend([r'\.snapshot'])
        self.excludes = '|'.join(excludes)
        self.excludes_bg = '|'.join(excludes_bg)
        self.exclude_scm = exclude_scm
        self.preserve_time = preserve_time
        self.priority = priority
        self.protect = protect
        self.pre_existing = self.dstdir and os.path.exists(self.dstdir)
        self.paths = set()

    def cmpfile(self, spath, sstats, dpath, dstats, trusting):
        """Use the cheapest method to see if src and dst are identical."""
        # Clearly if the sizes differ they must have different contents.
        if sstats.st_size != dstats.st_size:
            return False

        # This is a bit pessimistic - it's possible the contents could be
        # the same even if timestamps differ, but better safe than sorry.
        if self.preserve_time:
            # Whole seconds are the lowest common denominator.
            # Even in ext4 there seems to be some rounding of mtimes.
            if int(sstats.st_mtime) != int(dstats.st_mtime):
                return False

        # In strict mode, don't assume files are identical unless
        # they hash the same. In trusting mode, if size and mtime
        # match it's good enough.
        if not trusting:
            ms = hashlib.sha1()
            md = hashlib.sha1()
            try:
                with open(spath, 'rb') as f:
                    ms.update(f.read())
                with open(dpath, 'rb') as f:
                    md.update(f.read())
            except Exception:
                return False
            else:
                return ms.hexdigest() == md.hexdigest()

        return True

    def cpfile(self, path, fixabs, strict, trusting):
        """Copy a single file or symlink from src to dst."""
        spath = os.path.join(self.srcdir, path)
        dpath = os.path.join(self.dstdir, path)

        try:
            sstats = os.lstat(spath)
        except EnvironmentError:
            if strict and os.path.lexists(spath):
                raise
            return

        if stat.S_ISLNK(sstats.st_mode):
            stgt = os.readlink(spath)

            # Absolute symlinks pointing to the src area may be
            # changed to point into the dst area during copy.
            if fixabs and os.path.isabs(stgt):
                if stgt.startswith(self.srcdir + os.sep):
                    stgt = self.dstdir + stgt[len(self.srcdir):]
                    if fixabs > 1:
                        stgt = os.path.relpath(stgt, os.path.dirname(dpath))

            # Avoid rewriting symlinks which are already present and
            # correct to keep timestamps stable.
            try:
                dtgt = os.readlink(dpath)
            except OSError:
                pass
            else:
                if dtgt == stgt:
                    return

            logging.debug('Symlinking %s -> %s', dpath, stgt)

            # An atomic way of copying/replacing a symlink.
            dtmp = '%s.tmp' % dpath
            os.symlink(stgt, dtmp)
            os.rename(dtmp, dpath)
        else:
            if self.pre_existing:
                try:
                    dstats = os.lstat(dpath)
                except OSError:
                    pass
                else:
                    # If dst is identical to src, leave it alone.
                    if self.cmpfile(spath, sstats, dpath, dstats, trusting):
                        return

                    if self.backup:
                        # On request, leave previous versions of dpath around.
                        tm = time.strftime('%Y%m%dT%H%M%S')
                        dbak = '%s.%s.bak' % (os.path.join(
                            os.path.dirname(dpath),
                            '.' + os.path.basename(dpath)), tm)
                        try:
                            os.rename(dpath, dbak)
                        except EnvironmentError:
                            if strict:
                                raise
                    elif not dstats.st_mode & stat.S_IWUSR:
                        # If it exists and is not writable it must be removed.
                        os.unlink(dpath)

            logging.debug('Copying %s', dpath)

            try:
                shutil.copy(spath, dpath)
            except EnvironmentError as e:
                if strict:
                    # Try to recover by removing the target file.
                    logging.info('Retrying %s after %s', dpath, e)
                    pdpath = os.path.dirname(dpath)
                    try:
                        os.chmod(pdpath, 0o777)
                        os.unlink(dpath)
                    except EnvironmentError as e:
                        logging.info('Removing %s: %s', dpath, e)

                    # On the second try we let exceptions propagate.
                    try:
                        shutil.copy(spath, dpath)
                    finally:
                        # Restore directory mode even on copy failure.
                        try:
                            pdmode = (0o755 if self.protect else 0o775)
                            os.chmod(pdpath, pdmode)
                        except EnvironmentError as e:
                            # Don't die on metadata - log error and continue.
                            logging.info('Ignored %s', e)

            # In non-strict mode we care only about contents, not modes.
            if strict:
                try:
                    sstats = os.stat(spath)
                    mode = sstats.st_mode & ~stat.S_IWOTH
                    if self.protect:
                        mode &= ~stat.S_IWGRP
                    else:
                        mode |= stat.S_IWGRP
                    os.chmod(dpath, mode)
                    if self.preserve_time:
                        os.utime(dpath, (sstats.st_atime, sstats.st_mtime))
                except Exception as e:
                    # Don't die on metadata - log the error and continue
                    logging.info('Ignored %s', e)

    def qcpfile(self, queue):
        """Pull a pathname off the queue and copy it."""
        while True:
            try:
                path, fixabs, strict, trusting = queue.get(False)
            except Queue.Empty:
                break

            # If dest partition is full we might as well give up copying
            # but must still drain the queue. For other errors we copy
            # as much as possible and record only the first error.
            if self.errno != errno.ENOSPC:
                try:
                    self.cpfile(path, fixabs, strict, trusting)
                except Exception as e:
                    if strict and self.errno is None:
                        traceback.print_exc(e)
                        if isinstance(e, EnvironmentError):
                            self.errno = e.errno
                        else:
                            self.errno = 0

            # Poor man's thread priority; wait a configurable time
            # to let higher priority threads get their turn. Apparently
            # it would also be also possible to use the ctypes module to
            # get access to pthread_setschedparam() and do it right. See
            # http://pydoc.net/Python/threading2/0.3.1/threading2.t2_posix
            delay = 1.0 - self.priority
            if delay > 0.00000000000001:
                time.sleep(delay)

            queue.task_done()

    def copy(self, fixabs=0, jobs=DEFAULT_JOBS,
             strict=True, trusting=False):
        """Make the dst tree look like the src tree."""
        logging.info('Copy %s -> %s', self.srcdir, self.dstdir)

        # Make sure the target directory exists.
        if os.path.exists(self.dstdir):
            self.pre_existing = True
        else:
            try:
                if self.protect:
                    os.makedirs(self.dstdir, mode=0o755)
                else:
                    os.makedirs(self.dstdir, mode=0o775)
            except Exception as e:
                if not os.path.isdir(self.dstdir):
                    if strict:
                        raise e
                    else:
                        logging.info('Ignored %s', e)

        # Make sure the source dir exists before we get going.
        os.stat(self.srcdir)

        if self.dstdir == self.srcdir:
            return

        self.paths.clear()

        # Creating directories on the fly in a threaded context is scary.
        # There are inherent ordering issues as well as NFS caching
        # complications. We dodge the whole thing by doing a full, serial
        # pass over the src tree up front after which we will have (a)
        # a skeleton dst tree of just directories and (b) a list in
        # memory of all files to be copied. Thus we still traverse
        # the src tree only once.
        # For a big enough src tree this can cause a large and sometimes
        # problematic delay before any files start copying. Maybe we
        # could break this phase up such that as soon as directories
        # sufficient for (say) 10K or 100K files are ready we do them and
        # then loop around.
        for parent, dnames, fnames in lib.walk.os_walk(self.srcdir,
                                                       onerror='fix'):
            pbase = os.path.relpath(parent, self.srcdir)
            if pbase == '.':
                pbase = ''

            # Don't descend into excluded subdirs.
            if self.excludes or self.excludes_bg or self.exclude_scm:
                ndnames = []
                for dn in dnames:
                    if self.exclude_scm:
                        if dn in SCM_DIRS:
                            continue
                        if self.exclude_scm > 1:
                            if os.path.exists(os.path.join(dn, 'DEPS')):
                                continue
                    if self.excludes and re.search(self.excludes, dn):
                        continue
                    if not strict and self.excludes_bg and \
                       re.search(self.excludes_bg, dn):
                        continue
                    ndnames.append(dn)
                dnames[:] = ndnames

            # Create the shadow tree of directories up front.
            for dname in dnames:
                spath = os.path.join(self.srcdir, pbase, dname)
                dpath = os.path.join(self.dstdir, pbase, dname)
                try:
                    sstats = os.lstat(spath)
                    if stat.S_ISLNK(sstats.st_mode):
                        # Treat symlinks like files (because they are).
                        self.paths.add(os.path.join(pbase, dname))
                    elif self.protect:
                        os.mkdir(dpath, 0o755)
                    else:
                        os.mkdir(dpath, 0o775)
                except Exception as e:
                    if not os.path.exists(dpath):
                        if strict:
                            raise e
                        else:
                            logging.info('Ignored %s', e)

            # Remember the set of files to sync.
            for fname in fnames:
                if self.exclude_scm > 1:
                    if fname.endswith('.sparse'):
                        continue
                path = os.path.join(pbase, fname)
                if self.excludes and re.search(self.excludes, path):
                    continue
                if not strict and self.excludes_bg and \
                   re.search(self.excludes_bg, path):
                    continue
                self.paths.add(path)

        # Now we know all the destination directories exist, we can
        # start copying files in any order.
        try:
            if jobs > 1:
                q = Queue.Queue()
                for path in self.paths:
                    q.put((path, fixabs, strict, trusting))

                jobs = min(jobs, len(self.paths))
                for _ in range(jobs):
                    t = threading.Thread(target=self.qcpfile, args=(q,))
                    t.start()
                q.join()
            else:
                for path in self.paths:
                    self.cpfile(path, fixabs, strict, trusting)
        except EnvironmentError:
            if strict:
                raise

    def trim(self, plusmode=None):
        """Remove anything in dst which is not in src."""
        logging.info('Trim %s', self.dstdir)
        for parent, dnames, fnames in lib.walk.os_walk(self.dstdir,
                                                       topdown=False,
                                                       onerror='fix'):
            pbase = os.path.relpath(parent, self.dstdir)

            for name in fnames + dnames:
                dpath = os.path.join(parent, name)

                spath = os.path.join(self.srcdir, pbase, name)
                if os.path.lexists(spath):
                    if not os.path.islink(dpath):
                        if plusmode or self.preserve_time:
                            dstats = os.stat(dpath)
                            if plusmode:
                                newmode = dstats.st_mode | plusmode
                                if newmode != dstats.st_mode:
                                    os.chmod(dpath, newmode)
                            if self.preserve_time:
                                sstats = os.stat(spath)
                                if dstats.st_mtime != sstats.st_mtime:
                                    os.utime(dpath, (sstats.st_atime,
                                                     sstats.st_mtime))
                    continue

                if name in dnames and not os.path.islink(dpath):
                    rmfunc = os.rmdir
                else:
                    rmfunc = os.unlink
                try:
                    logging.debug('Trimming %s', dpath)
                    rmfunc(dpath)
                except OSError:
                    if os.path.lexists(dpath):
                        os.chmod(os.path.dirname(dpath), 0o777)
                        try:
                            rmfunc(dpath)
                        except OSError as e:
                            self.ignore_dot_nfs(dpath, e)

    def _threaded_rmfiles(self, trees, jobs):
        """Remove all files in specified trees in a threaded parallel way."""
        def delpath(queue):
            """Pop a file path from the queue and delete it."""
            while True:
                try:
                    path = queue.get(False)
                except Queue.Empty:
                    break

                logging.debug('Removing %s', path)

                try:
                    os.unlink(path)
                except OSError as e:
                    self.ignore_dot_nfs(path, e)
                finally:
                    queue.task_done()

        # First derive lists of files present in each specified tree.
        # The idea is to get them all up front so the threaded removals
        # can be interspersed between trees (and hopefully filers).
        filemap = collections.defaultdict(list)
        for tree in trees:
            for parent, dnames, fnames in lib.walk.os_walk(tree,
                                                           onerror='fix'):
                for dname in dnames:
                    path = os.path.join(parent, dname)

                    if os.path.islink(path):
                        filemap[tree].append(path)
                    else:
                        # In case the dirs are missing +w bits.
                        # Ignore failures; the real errors will pop up soon.
                        try:
                            os.chmod(path, 0o777)
                        except Exception:
                            pass

                for fname in fnames:
                    path = os.path.join(parent, fname)
                    filemap[tree].append(path)

        # Now we have a set of file lists, one for each directory tree.
        # Put them on the queue in an order intended to disperse work
        # as evenly over different filers as possible, based on the
        # assumption that each tree _may_ be on a different filer but
        # all files within a tree will be on the same one.
        q = Queue.Queue()
        while sum([len(l) for l in filemap.values()]) > 0:
            for tlist in filemap.values():
                try:
                    q.put(tlist.pop())
                except IndexError:
                    pass

        for _ in range(jobs):
            t = threading.Thread(target=delpath, args=(q,))
            t.start()
        q.join()

    def rmtrees(self, trees, jobs=DEFAULT_JOBS):
        """Remove the specified list of directory trees."""
        for tree in trees:
            if not os.path.isdir(tree):
                os.remove(tree)

        trees = [t for t in trees if os.path.lexists(t)]
        if not trees:
            return
        logging.info('Removing dirs: %s', trees)

        # Parallel removals still do final mop-up work serially, while
        # if jobs==1 all the work is done serially. After successful parallel
        # removals the serial pass will remove just the skeleton dir tree
        # since only files and symlinks are removed in parallel.

        if jobs > 1:
            try:
                self._threaded_rmfiles(trees, jobs)
            except Exception:
                # If anything is left over after parallel traversal, let the
                # serial removal below take a shot at it before giving up.
                pass

        # Since shutil.rmtree won't remove a file from an unwritable
        # directory, to ensure a clean rm we use os_walk() to visit
        # remaining dirs and make them writable before removing from them.
        for tree in trees:
            for parent, dnames, fnames in lib.walk.os_walk(tree,
                                                           onerror='fix',
                                                           topdown=False):
                try:
                    os.chmod(parent, 0o777)
                except Exception:
                    continue

                for name in fnames + dnames:
                    path = os.path.join(parent, name)
                    if name in dnames and not os.path.islink(path):
                        rmfunc = os.rmdir
                    else:
                        rmfunc = os.unlink

                    try:
                        rmfunc(path)
                    except OSError as e:
                        self.ignore_dot_nfs(path, e)

            # Try one last time to remove anything that's left.
            try:
                shutil.rmtree(tree)
            except OSError as e:
                if e.errno != errno.ENOENT:
                    raise

    def rmtree(self, tree, jobs=DEFAULT_JOBS):
        """Remove the specified directory tree."""
        self.rmtrees([tree], jobs=jobs)

    def rm_src(self, jobs=DEFAULT_JOBS):
        """Remove the entire source tree (typically post-copy)."""
        self.rmtrees([self.srcdir], jobs=jobs)

    def rm_dst(self, jobs=DEFAULT_JOBS):
        """Remove the entire dest tree (typically pre-copy)."""
        self.rmtrees([self.dstdir], jobs=jobs)

    @staticmethod
    def ignore_dot_nfs(path, e):
        """Ignore error in removal of a .nfs* file.

        See http://nfs.sourceforge.net/#faq_d2"""

        if e.errno == errno.EBUSY and \
           os.path.basename(path).startswith('.nfs'):
            logging.info('Ignored %s', e)
            # Slow down a bit in case there was a thread collision.
            time.sleep(0.1)
        else:
            if os.path.lexists(path):
                raise e

    @property
    def success(self):
        """Return True iff the populate event succeeded."""
        return self.errno is None


def main():
    """Run unit tests."""
    print('Unit tests TBD')
    return 0

if __name__ == '__main__':
    main()

# vim: ts=8:sw=4:tw=80:et:
