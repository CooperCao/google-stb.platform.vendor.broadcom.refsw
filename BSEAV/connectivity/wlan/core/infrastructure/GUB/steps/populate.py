"""Contains the PopulateStep class which creates the build tree."""

from __future__ import print_function
from __future__ import unicode_literals

import glob
import os
import re
import sys

import lib.consts
import lib.nfscp
import lib.opts
import lib.patch
import lib.pop
import lib.prep
import lib.scm
import lib.times
import lib.util

import steps.base

PERSISTENT_FAIL_DAYS = 10


# TODO this ought to be rethought and moved such that builds which
# put the source tree in a different place, such as DHDAP routers,
# can find it without this ugly local hack.
def usf_exports(basedir=''):
    """Push settings in this file, if present, to the environment."""
    usf = os.path.join(basedir, 'src/tools/release/WLAN.usf')
    usfs = glob.glob(usf) + glob.glob(os.path.join('main', usf))
    if usfs:
        lib.util.note('sourcing ' + usfs[0])
        with open(usfs[0], 'r') as f:
            for line in f:
                match = re.match(r'(\w+)=(.*)', line)
                if match:
                    lib.util.export(match.group(1), match.group(2))
    else:
        lib.util.note('No such file: ' + usf, vl=2)


class PopulateStep(steps.base.Step):

    """Populate the build tree by checkout or copy."""

    def __init__(self, *args, **kwargs):
        steps.base.Step.__init__(self, *args, **kwargs)

        if self.build:
            self.scmdir = self.build.scmdir
        else:
            self.scmdir = os.getcwd()

        self.parallel = False

    def timestamps(self):
        """Normalize file timestamps pre-build."""
        lib.util.normalize_timestamps(self.bldwd)

    def svnrev(self):
        """Write a file containing the current revision of the svn repo."""
        cmd = lib.scm.svncmd('info', '-r', self.build.reftime)
        cmd.append(os.path.join(lib.consts.WL_REPO_ROOT, 'proj',
                                lib.scm.svn_tag_path(self.tag)))
        proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=2)
        stdout = proc.communicate()[0].decode('utf-8')
        if proc.returncode == 0:
            for line in stdout.splitlines():
                if line.startswith('Revision:'):
                    with open(lib.pop.SVN_REV, 'w') as f:
                        f.write(line.split()[-1] + '\n')
                    break

    def checkout(self):
        """Do the actual checkout using lib.pop code."""
        lib.util.execute('echo "Using: svn $(svn --version -q)"', vl=4)

        # Subversion is painfully slow when checking out into NFS and
        # there are also occasional failures (presumably due to NFS
        # locking semantics). Since checkout-to-local is both faster
        # and more reliable, even when the plan is to build directly
        # within NFS we still check out to local and copy into NFS.
        if lib.util.is_local_fs(self.scmdir):
            tmpco = None
            tree = lib.pop.Tree(basedir=self.scmdir, meta=False)
        else:
            tmpco = lib.util.tmpdir(prefix='pop.')
            tree = lib.pop.Tree(basedir=tmpco, meta=False)

        treemap = self.cfg.get('treemap', {})
        for todir, upath in treemap.items():
            if lib.util.is_url(upath) or '.git' in upath:
                url = upath
            else:
                url = '/'.join([lib.consts.WL_REPO_ROOT, upath])
            qsto = '='.join([lib.pop.QS_TO, todir])
            if '?' in url:
                url = '&'.join([url, qsto])
            else:
                url = '?'.join([url, qsto])
            extravars = [
                'BUILDABLE=' + self.build.bldable,
                'TAG=' + self.build.tag,
                'TAGPATH=' + lib.scm.svn_tag_path(self.build.tag),
            ]
            tree.add(self.cfg.sub(url, extravars=extravars),
                     reftime=self.build.reftime, vl=0)

        # Extract the files from SCM.
        jobs = int(self.cfg.getvar('POPULATE_JOBS', 1))
        if not tree.populate(jobs=jobs, reftime=self.build.reftime):
            return False

        # Epivers.sh expects .gclient_info.
        tree.write_metadata(reftime=self.build.reftime)

        # If the original checkout was to /tmp, copy it into NFS.
        # The tmpco copy will be removed automatically.
        if tmpco:
            lib.util.xtrace('cp -pr %s %s' % (tmpco, self.scmdir), vl=1)
            cpobj = lib.nfscp.TreeSync(tmpco, self.scmdir, preserve_time=True)
            cpobj.copy(strict=True)

        # Create a _SUBVERSION_REVISION file at the base of the tree
        # representing the revision of the entire repo as of build reference
        # time, for partial compatibility with the old way of doing things.
        # Each component should also have its own _SUBVERSION_REVISION file.
        if not lib.opts.COPYDIR:
            self.svnrev()

        # Update content list data for "official" builds only.
        if not lib.consts.STATE.is_userbase():
            self.gencontentlist()

        return True

    def gencontentlist(self):
        """Generate content list data."""
        cmd = lib.util.xscript(vd=0) + [
            'contents',
            '-b', self.bldable,
            '-t', self.tag,
            '-l', self.reftime,
            '-w', os.path.join(lib.opts.METADIR, lib.consts.CONTENTS_HTML),
        ]

        # Only "official" builds should advance the CL database. Others can
        # still generate data but do not commit it.
        if self.standard and lib.consts.STATE.is_stdbase():
            cmd.extend(['-n', os.path.basename(self.pubwd)])
            # Accept defaults for the from and to locations.
        else:
            cmd.extend(['-n', lib.util.rmchars(self.reftime, '{}')])
            # Accept default "from" path, override the to path.
            dbto = os.path.join(lib.opts.METADIR, lib.consts.CLNAME)
            cmd.extend(['-T', dbto])

        cmd.append(self.scmdir)
        lib.util.execute(cmd, check=True, vl=1)

    def abandon_persistent(self):
        """
        Refuse nightly service to buildables which fail consistently.

        We frequently see a kind of paralysis; a nightly build has been failing
        for months but nobody turns it off, either because nobody wants to take
        responsibility, or because everyone's just gotten used to seeing the
        failure message, or the key players are filtering out nightly build
        emails, or whatever. The solution here is to refuse automatic nightly
        build service to any buildable/tag combo whose last good build (LGB)
        is more than X days in the past. It remains buildable by hand,
        allowing developers to fix the problem at any time, but will stop
        building automatically until fixed.

        Potentially this feature could be moved earlier. The current spot was
        chosen to cause a visible build failure, giving notice that it's broken
        while not wasting resources on it. An alternative way would be to
        remove persistently broken buildables from the "enabled" set before
        even invoking the build. This would be less obvious to developers but
        save even more resources.
        """
        cmd = lib.util.xscript(vd=0) + [
            'contents',
            '-b', self.bldable,
            '-t', self.tag,
            '--lgb', '--simple-format',
        ]
        proc = lib.util.execute(cmd, stdout=lib.util.PIPE, vl=2)
        lgb = proc.communicate()[0].strip()
        # Err in favor of building. I.e. if the attempt to find LGB
        # failed, we build. If the command ran successfully but no good
        # build was found, we still must build to allow a newly added
        # buildable/branch combo to get started.
        if proc.returncode or not lgb:
            return

        # Now convert LGB to a date and return if it's sufficiently new.
        now = lib.times.iso2seconds(self.reftime)
        then = lib.times.iso2seconds(lgb)
        if now - then <= (PERSISTENT_FAIL_DAYS * lib.consts.SECONDS_PER_DAY):
            return

        # LGB too old - fail on purpose but leave pointers to old logs etc.
        cdb = lib.util.mkpath(lib.consts.CLBASE,
                              lib.scm.svn_tag_path(self.tag),
                              self.bldable, lib.consts.CLNAME)
        chtml = os.path.join(lib.opts.METADIR, lib.consts.CONTENTS_HTML)
        cmd = lib.util.xscript() + ['contents', '-F', cdb, '-w', chtml]
        lib.util.execute(cmd, check=True, vl=2)
        msg = 'nightly build abandoned due to failing ' \
              'at least %d straight days' % PERSISTENT_FAIL_DAYS
        msg += ' (last good build: %s)' % (lgb if lgb else None)
        if self.logdir:
            logsdir = self.pub_path(os.path.dirname(self.logdir))
            msg += '\nSee prior logs in %s' % logsdir
        msg += '\nOr content list: %s' % self.pub_path(chtml)
        msg += '\n*** Respin manually until fixed ***'
        self.abandon(msg)

    def run(self):
        """Make this step do its stuff."""
        if self.nightly and self.tag != 'trunk':
            self.abandon_persistent()

        use_copy = lib.opts.COPYDIR and not self.is_child()
        if use_copy:
            self.scmdir = lib.opts.COPYDIR
        else:
            try:
                if not self.checkout():
                    self.passed = False
            except Exception:
                self.passed = False
                raise

        # No sense going farther if the basic checkout failed.
        if not self.passed:
            return

        # Apply a user-provided patch to the checkout ahead of epivers work.
        # This might fail to apply in a child build even if it succeeds in
        # the main build, because the child build could be on a different
        # branch. We have to accept that possibility because ignoring patch
        # failures is bad policy but there is a workaround: if both --copydir
        # and --patch are used, the parent build does the copy whereas the
        # child uses checkout+patch. This allows disparate pre-commit change
        # sets to be specified for parent and child.
        if self.passed and self.patch and not use_copy:
            if lib.patch.applypatch(self.patch, dig=2):
                self.passed = False

        # Do the epivers.h setup.
        envlog = os.path.join(lib.opts.METADIR, ',env.log')
        for cmds in lib.prep.epivers(self.scmdir, envlog=envlog):
            self.execute(cmds[0], extra_env=cmds[1])

        # Copy the SCM tree into the build area.
        if self.scmdir != self.bldwd:
            srcdir = self.scmdir
            if srcdir.startswith('/projects') or srcdir.startswith('/home'):
                if 'cyg' in sys.platform:
                    srcdir = lib.consts.UNC_BASE + srcdir
            cpobj = lib.nfscp.TreeSync(srcdir, self.bldwd, exclude_scm=1)
            lib.util.xtrace(['nfscp', srcdir, self.bldwd], vl=1)
            cpobj.copy(strict=True)

        # Export settings found in the WLAN.usf file.
        usf_exports()

        # Normalize timestamps within the checkout.
        self.timestamps()

# vim: ts=8:sw=4:tw=80:et:
