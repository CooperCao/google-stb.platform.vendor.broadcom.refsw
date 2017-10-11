"""Manage the packaging part of a router build."""

from __future__ import print_function
from __future__ import unicode_literals

import glob
import os

import lib.scm
import lib.util
from lib.util import mkpath

import steps.custom.base
import steps.custom.router.common

PACKAGE_VERSION = 'ROUTER_PACKAGE_VERSION'


class RouterPackageStep(steps.custom.base.BrandStep):

    """Manage the packaging part of a router build."""

    def __init__(self, *args, **kwargs):
        steps.custom.base.BrandStep.__init__(self, *args, **kwargs)

        self.mwd, self.swd, self.fwds, self.rwd = \
            steps.custom.router.common.locate()

        path = mkpath(self.mwd, self.rwd, 'shared/router_version.h')
        for line in open(path, 'r'):
            if PACKAGE_VERSION in line:
                self.verstr = line.split()[-1]
                break
        else:
            lib.util.die('no %s found in %s' % (PACKAGE_VERSION, path))

    def run(self):
        """Conventional entry point for running any step."""
        if self.getbool('OPEN_SRC_PKG'):
            if not self.as_step(self.release_open_archive):
                return

        if not self.as_step(self.release_archive):
            return

        if not os.getenv('LINUX_ROUTER_NO_CLEAN'):
            self.as_step(self.build_clean)

    def release_open_archive(self):
        """Make a tar package of the GPL router."""

        # Before removing firmware source trees, copy the clm blobs to a
        # clmsave directory
        for fw in self.fwds:
            clm_rel_fw_dir = mkpath(self.mwd, 'release', fw, 'src/wl/clm/src')
            clm_sav_fw_dir = mkpath(self.mwd, 'clmsave', fw, 'src/wl/clm/src')
            lib.util.mkdir(clm_sav_fw_dir)
            for clmb in glob.glob('%s/*.clm_blob' % clm_rel_fw_dir):
                self.cp(clmb, clm_sav_fw_dir)

        # remove firmware sources for partial-src
        for fw in self.fwds:
            lib.util.rm(mkpath(self.mwd, 'release', fw))

        # restore clm_blobs in release pkg for partial-src
        for fw in self.fwds:
            clm_rel_fw_dir = mkpath(self.mwd, 'release', fw, 'src/wl/clm/src')
            clm_sav_fw_dir = mkpath(self.mwd, 'clmsave', fw, 'src/wl/clm/src')
            lib.util.mkdir(clm_rel_fw_dir)
            for clmb in glob.glob('%s/*.clm_blob' % clm_sav_fw_dir):
                self.cp(clmb, clm_rel_fw_dir)

        relgpl = mkpath(self.mwd, 'release.gpl')
        self.mkdir(relgpl)
        self.execute('cd %s/release && pax -rwk . ../release.gpl' % self.mwd)

        cmd = lib.util.xscript('xmogrify', shell=True) + """
            $[MOGRIFY_FLAGS]
                --mogrifier=latest
                --undefine __CONFIG_NORTON__ __CONFIG_TREND_IQOS__
                -- %s
        """ % mkpath(relgpl, self.mwd, self.rwd)
        self.execute(cmd)

        openpkg = '%s-router-%s.tar.gz' % (self.platform, self.verstr)

        # This file exists to mark the base of the build tree.
        open(mkpath(relgpl, lib.scm.WLBASE), 'w').close()

        cmd = """
            tar -C %s -czf %s --ignore-failed-read %s -T %s/%s
        """ % (relgpl, openpkg, lib.scm.WLBASE, self.mwd, '$[GPL_FILELIST]')
        self.execute(cmd)

        self.mkdir(mkpath(self.mwd, 'tmp'))
        self.execute('tar -C %s -xzf %s' % (mkpath(self.mwd, 'tmp'), openpkg))

        self.execute('bash validate_gpl.sh %s' % mkpath('tmp', self.mwd),
                     cwd=self.mwd)

        if self.swd:
            self.execute('bash validate_gpl.sh %s' % mkpath('tmp', self.swd),
                         cwd=self.mwd)

        # restore clm_blobs for partial-src build
        for fw in self.fwds:
            clm_tmp_fw_dir = mkpath(self.mwd, 'tmp', fw, 'src/wl/clm/src')
            clm_sav_fw_dir = mkpath(self.mwd, 'clmsave', fw, 'src/wl/clm/src')
            lib.util.mkdir(clm_tmp_fw_dir)
            for clmb in glob.glob('%s/*.clm_blob' % clm_sav_fw_dir):
                self.cp(clmb, clm_tmp_fw_dir)

        cmd = """
            $[MAKE] -C %s $[MAKEJOBS]
                       LINUX_VERSION=$[LINUX_VERSION] PLT=$[PLT]
        """ % mkpath(self.mwd, 'tmp', self.mwd, 'src')
        self.execute(cmd)

    def release_archive(self):
        """Make a tar package of the standard (proprietary) router."""
        openpkg = '%s-router-%s.tar.gz' % (self.platform, self.verstr)
        proppkg = '$[ROUTER_OS]-router-%s.tar.gz' % self.verstr
        reldir = mkpath(self.mwd, 'release')
        # This file exists to mark the base of the build tree.
        open(mkpath(reldir, lib.scm.WLBASE), 'w').close()
        self.execute('cd %s && tar -czf ../%s %s *' %
                     (reldir, proppkg, lib.scm.WLBASE))
        self.mv(reldir, mkpath(self.mwd, 'release.bak'))
        self.mkdir(reldir)
        self.chmod(reldir, 0o775)
        self.cp(mkpath(self.mwd, 'src/tools/release/install_linux.sh'),
                mkpath(reldir, 'install.sh'))
        self.mv(mkpath(self.mwd, proppkg), reldir)
        if '-mfgtest-' in self.bldable:
            self.cp(mkpath(self.mwd, self.rwd, 'compressed/vmlinuz'), reldir)

        if self.exists(openpkg):
            self.mv(openpkg, reldir)

        # Move the release folder to the top
        if self.mwd != '.':
            self.mv(reldir, '.')

    def build_clean(self):
        """Handle cleanup of unused build artifacts."""
        # This could use os.walk() instead.
        cmd = """
            find * -name '*.o' ! -name '*_dbgsym.o' -type f -print0 |
                xargs -0 /bin/rm -f
        """
        self.execute(cmd, cwd=self.mwd)

        # Remove source folders from build and move the build dir to top
        self.execute('rm -rf release.* tmp', cwd=self.mwd)
        self.rm(mkpath(self.mwd, 'build', self.mwd, 'src'))
        self.rm(mkpath(self.mwd, 'build', self.mwd, 'components'))
        if self.swd:
            self.rm(mkpath(self.mwd, 'build', self.swd))
        # If firmware xyz is built under ./main/build/xyz we prefer to keep
        # that one and remove ./xyz.
        rmset = set()
        mwd_bld = mkpath(self.mwd, 'build')
        for entry in os.listdir(mwd_bld):
            for parent, dnames, fnames in os.walk(mkpath(mwd_bld, entry)):
                if 'rtecdc.bin' in fnames:
                    dnames[:] = []  # prune
                    parts = parent.split(os.sep)
                    if len(parts) > 2:
                        rmset.add(parts[2])
        self.rm(sorted(rmset))
        if self.mwd != '.':
            self.execute('mv build/%s/* build/' % self.mwd, cwd=self.mwd)
            self.rm(mkpath(self.mwd, 'build', self.mwd))
            self.mv(mkpath(self.mwd, 'build'), '.')

        # Move the _SUBVERSION_REVISION to top
        if self.exists('%s/src/_SUBVERSION_REVISION' % self.mwd):
            self.mv(mkpath(self.mwd, 'src/_SUBVERSION_REVISION'), '.')

# vim: ts=8:sw=4:tw=80:et:
