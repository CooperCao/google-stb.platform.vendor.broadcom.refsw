"""Manage the compile-and-link part of a router build."""

from __future__ import print_function
from __future__ import unicode_literals

import glob
import os

import lib.opts
import lib.util
from lib.util import mkpath

import steps.custom.base
import steps.custom.router.common

BLD = 'build'
REL = 'release'

MJOBS = '$[MAKEJOBS]'
MVARS = 'LINUX_VERSION=$[LINUX_VERSION] ARCH=$[ARCH] PLT=$[PLT]'

PAX_SKIP_VCS = ' '.join([
    "-s '?.*/[.]git/.*??g'",
    "-s '?.*/[.]git$??g'",
    "-s '?.*/[.]svn/.*??g'",
    "-s '?.*/[.]svn$??g'",
    "-s '?.*/_SUBVERSION_REVISION$??g'",
])


class RouterMakeStep(steps.custom.base.BrandStep):

    """Manage the compile-and-link part of a router build."""

    def __init__(self, *args, **kwargs):
        steps.custom.base.BrandStep.__init__(self, *args, **kwargs)

        self.mwd, self.swd, self.fwds, self.rwd = \
            steps.custom.router.common.locate()

        self.reldir = mkpath(REL, self.mwd)
        self.xmogrify = lib.util.xscript('xmogrify', shell=True, vd=1)

    def run(self):
        """Conventional entry point for running any step."""
        step_methods = [
            self.prep,
            self.mogrify,
            self.make_src_release,
            self.pre_build,
            self.release_pre_build,
            self.final_build,
        ]
        for step_method in step_methods:
            if not self.as_step(step_method):
                break

    def prep(self):
        """Copy a few files into position."""
        # Force DLNA settings into the default config file before it gets
        # copied around.
        if self.getbool('DLNA'):
            cmd = """
                perl -pi
                    -e "s/# CONFIG_DLNA_DMS is not set/CONFIG_DLNA_DMS=y/;"
                    -e "s/# CONFIG_FFMPEG is not set/CONFIG_FFMPEG=y/;"
                    -e "s/# CONFIG_LIBZ is not set/CONFIG_LIBZ=y/;"
                    %s/config/$[ROUTERCFG]
            """ % self.rwd
            self.execute(cmd, cwd=self.mwd)

            routercfg = self.sub(mkpath(self.mwd, self.rwd,
                                        'config/$[ROUTERCFG]'))
            with open(routercfg, 'a') as f:
                f.write('\n# Settings to support DLNA\n')
                f.write('CONFIG_SALSA=y\n')
                f.write('CONFIG_APLAY=y\n')
                f.write('CONFIG_LIBMAD=y\n')
                f.write('CONFIG_LIBID3TAG=y\n')
                f.write('CONFIG_MADPLAY=y\n')

        self.cp(mkpath(self.mwd, self.rwd, 'config/$[ROUTERCFG]'),
                mkpath(self.mwd, self.rwd, '.config'))

        self.cp(mkpath(self.mwd, '$[LINUX_DIR]/arch/$[ARCH]/$[KERNELCFG]'),
                mkpath(self.mwd, '$[LINUX_DIR]/.config'))

        cmd = 'src/hndcvs/rem_comments.awk $[SRCFILELIST]'
        for flist in self.sub('$[SRCFILELISTS_COMPONENTS]').split():
            if os.path.exists(mkpath(self.mwd, flist)):
                cmd += ' ' + flist
        self.execute(cmd, cwd=self.mwd, stdout='master_filelist.h')

        cmd = """
            gcc -E -undef
                $(perl -e 'print "-D${_}_SRC " for @ARGV' $[MOGRIFY_ON])
                -Ulinux -o %s master_filelist.h
        """ % self.basename('$[SRCFILELIST]')
        self.execute(cmd, cwd=self.mwd)

        # Don't apply to old tree structure (compatibility issue)
        if self.swd and self.swd != 'dhd':
            cmd = 'src/hndcvs/rem_comments.awk'
            for flist in self.getvar('SYS_SRCFILELISTS_COMPONENTS').split():
                if os.path.exists(mkpath(self.swd, flist)):
                    cmd += ' ' + '../' + mkpath(self.swd, flist)
            self.execute(cmd, cwd=self.mwd, stdout='sys_master_filelist.h')

            cmd = """
                gcc -E -undef
                    $(perl -e 'print "-D${_}_SRC " for @ARGV' $[MOGRIFY_ON])
                    -Ulinux -o sys-filelist.txt sys_master_filelist.h
            """
            self.execute(cmd, cwd=self.mwd)

        if self.getvar('FW_SRCFILELISTS_COMPONENTS'):
            cmd = 'src/hndcvs/rem_comments.awk'
            for flist in self.getvar('FW_SRCFILELISTS_COMPONENTS').split():
                # don't apply to old tree structure (compatibility issue)
                if self.swd != 'dhd':
                    for fw in self.fwds:
                        flpath = mkpath(fw, flist)
                        if os.path.isfile(flpath):
                            cmd += ' ' + mkpath('../', flpath)
                else:
                    if os.path.exists(mkpath(self.mwd, flist)):
                        cmd += ' ' + flist
            self.execute(cmd, cwd=self.mwd, stdout='fw_master_filelist.h')

            cmd = """
                gcc -E -undef
                    $(perl -e 'print "-D${_}_SRC " for @ARGV' $[MOGRIFY_ON])
                    -Ulinux -o %s fw_master_filelist.h
            """ % self.basename('$[FW_SRCFILELISTS_COMPONENTS]')
            self.execute(cmd, cwd=self.mwd)

    def mogrify(self):
        """Do the standard initial mogrification."""
        # Mogrify main tree
        if self.mwd:
            self.mkdir(mkpath(self.mwd, lib.opts.METADIR))
            cmd = self.xmogrify + """
                $[MOGRIFY_FLAGS] --list-files=%s/mogrified.list
                    --mogrifier=latest
                    --define $[MOGRIFY_ON] --undefine $[MOGRIFY_OFF]
                    -- $[MOGRIFY_PATHS]
            """ % lib.opts.METADIR
            self.execute(cmd, cwd=self.mwd)

        # Mogrify apps tree if present
        if self.swd:
            self.mkdir(mkpath(self.swd, lib.opts.METADIR))
            cmd = self.xmogrify + """
                $[MOGRIFY_FLAGS] --list-files=%s/%s_mogrified.list
                    --mogrifier=latest
                    --define $[MOGRIFY_ON] --undefine $[MOGRIFY_OFF]
                    -- .
            """ % (
                lib.opts.METADIR,
                os.path.basename(self.swd)
            )
            self.execute(cmd, cwd=self.swd)

        # Mogrify firmware trees if present
        for fw in self.fwds:
            self.mkdir(mkpath(fw, lib.opts.METADIR))
            cmd = self.xmogrify + """
                $[MOGRIFY_FLAGS] --list-files=%s/%s_mogrified.list
                    --mogrifier=latest
                    --define %s --undefine %s
                    -- .
            """ % (
                lib.opts.METADIR,
                os.path.basename(fw),
                self.getvar('FW_MOGRIFY_ON'),
                self.getvar('FW_MOGRIFY_OFF')
            )
            self.execute(cmd, cwd=fw)

    def make_src_release(self):
        """Cribbed from linux-router.mk."""
        self.mkdir(mkpath(self.mwd, self.reldir, 'doc'))
        self.mkdir(mkpath(self.mwd, self.reldir, 'tools'))
        srcfltr = 'srcfilter.py -L srcfilter.out -d'
        cfedir = 'components/cfe'
        if not os.path.exists(mkpath(self.mwd, cfedir)):
            cfedir = 'src/cfe'

        cmd = """
            find {2} |
                {0} src/tools/release/cfe-filelist.txt |
                pax -rwkl {1}
        """.format(srcfltr, self.reldir, cfedir)
        self.execute(cmd, cwd=self.mwd)

        cmd = """
            find src components |
                {0} -v {1} |
                pax -rwkl {2}
        """.format(srcfltr, self.basename('$[SRCFILELIST]'), self.reldir)
        self.execute(cmd, cwd=self.mwd)

        if self.swd:
            self.mkdir(mkpath(self.mwd, REL, self.swd))
            srchdirs = [d for d in ['src', 'components']
                        if os.path.isdir(mkpath(self.swd, d))]
            # don't apply to old tree structure (compatibility issue)
            if self.swd != 'dhd':
                cmd = """
                    find {0} |
                        {1} -v ../{2}/sys-filelist.txt |
                        pax -rwkl ../{2}/release/{3}
                """.format(
                    ' '.join(srchdirs),
                    srcfltr,
                    self.mwd,
                    self.swd,
                )
            else:
                cmd = """
                    find {0} |
                        {1} -v ../{2}/{3} |
                        pax -rwkl ../{2}/release/{4}
                """.format(
                    ' '.join(srchdirs),
                    srcfltr,
                    self.mwd,
                    self.basename('$[SRCFILELIST]'),
                    self.swd,
                )
            self.execute(cmd, cwd=self.swd)

        for fw in self.fwds:
            self.mkdir(mkpath(self.mwd, REL, fw))
            srchdirs = [d for d in ['src', 'components']
                        if os.path.isdir(mkpath(fw, d))]
            cmd = """
                find {0} |
                    {1} -v ../{2}/{3} |
                    pax -rwkl ../{2}/release/{4}
            """.format(
                ' '.join(srchdirs),
                srcfltr,
                self.mwd,
                self.basename('$[FW_SRCFILELISTS_COMPONENTS]'),
                fw,
            )
            self.execute(cmd, cwd=fw)

        self.cp(mkpath(self.mwd,
                       self.getvar('LINUX_DIR'),
                       'arch',
                       self.getvar('ARCH'),
                       self.getvar('KERNELCFG')),
                mkpath(self.mwd,
                       self.reldir,
                       self.getvar('LINUX_DIR'),
                       'arch',
                       self.getvar('ARCH'),
                       'defconfig-2.6-bcm947xx'))

        cmd = """
            perl -p
                -e "s/# CONFIG_LIBCRYPT is not set/CONFIG_LIBCRYPT=y/;"
                -e "s/CONFIG_LIBOPT=y/# CONFIG_LIBOPT is not set/;"
                %s/config/$[ROUTERCFG]
                    > %s/%s/config/defconfig-2.6
        """ % (self.rwd, self.reldir, self.rwd)
        self.execute(cmd, cwd=self.mwd)

        self.cp(mkpath(self.mwd, 'src/doc/bcm47xx/LinuxReadme.txt'),
                mkpath(self.mwd, self.reldir, 'README.TXT'))
        self.cp(mkpath(self.mwd, 'src/doc/BCMLogo.gif'),
                mkpath(self.mwd, self.reldir))
        self.cp(mkpath(self.mwd, 'src/tools/release/release_linux.sh'),
                mkpath(self.mwd, self.reldir, 'release.sh'))
        self.cp(mkpath(self.mwd, 'src/tools/release/validate_gpl.sh'),
                mkpath(self.mwd, 'validate_gpl.sh'))
        self.cp(mkpath(self.mwd, self.sub('$[GPL_FILELIST]')),
                mkpath(self.mwd, self.reldir, 'gpl-filelist.txt'))
        self.cp(mkpath(self.mwd, self.rwd, 'misc/toplevel-release.mk'),
                mkpath(self.mwd, self.reldir, 'src/Makefile'))

    def pre_build(self):
        """Cribbed from linux-router.mk."""
        self.cp(
            mkpath(self.mwd, '$[LINUX_DIR]/arch/$[ARCH]/$[KERNELCFG]'),
            mkpath(self.mwd, '$[LINUX_DIR]/.config')
        )

        cmd = """
            perl -p
                -e "s/# CONFIG_LIBCRYPT is not set/CONFIG_LIBCRYPT=y/;"
                -e "s/# CONFIG_UCLIBC is not set/CONFIG_UCLIBC=y/;"
                -e "s/CONFIG_LIBOPT=y/# CONFIG_LIBOPT is not set/;"
                -e "s/CONFIG_GLIBC=y/# CONFIG_GLIBC is not set/;"
            %s/config/$[ROUTERCFG] > %s/.config
        """ % (self.rwd, self.rwd)
        self.execute(cmd, cwd=self.mwd)

        self.execute('$[MAKE] -C %s %s oldconfig' %
                     (mkpath(self.mwd, self.rwd), MVARS))
        self.execute('$[MAKE] -C %s %s %s all' %
                     (mkpath(self.mwd, self.rwd), MJOBS, MVARS))
        self.execute('$[MAKE] -C %s %s install' %
                     (mkpath(self.mwd, self.rwd), MVARS))
        self.execute('$[MAKE] -C %s %s router-clean' %
                     (mkpath(self.mwd, self.rwd), MVARS))
        self.execute('$[MAKE] -C %s %s %s pciefd' %
                     (mkpath(self.mwd, self.rwd), MJOBS, MVARS))
        self.execute('$[MAKE] -C %s %s %s %s' %
                     (mkpath(self.mwd, self.rwd), MJOBS, MVARS,
                      'libbcmcrypto ses/ses ses/ses_cl httpd'))
        cfedir = 'components/cfe'
        if not os.path.exists(mkpath(self.mwd, cfedir)):
            cfedir = 'src/cfe'
        self.execute('$[MAKE] -C %s %s %s ARCH=$[ARCH]' %
                     (mkpath(self.mwd, cfedir), MJOBS, MVARS))
        self.execute('$[MAKE] -C %s %s %s %s' %
                     (mkpath(self.mwd, 'src/tools/misc'), MJOBS, MVARS,
                      'addhdr swap trx nvserial lzma lzma_4k'))
        self.execute('$[MAKE] -C %s %s %s %s' %
                     (mkpath(self.mwd, self.rwd), MJOBS, MVARS,
                      'emf igs igmp bcmupnp libupnp igd bmac'))
        self.execute('$[MAKE] -C %s %s %s %s' %
                     (mkpath(self.mwd, self.rwd), MJOBS, MVARS,
                      'nfc wps voipd hspot_ap'))

        # Copy generated source files in a way that doesn't complain
        # if they already exist as links due to previous pax -rwl.
        self.mkdir(mkpath(self.mwd, REL))
        self.execute('find src/$WLAN_GEN_BASEDIR | pax -rwkl release',
                     cwd=self.mwd)

        # Copy generated source files and clm blobs from firmware folders
        for fw in self.fwds:
            self.execute(
                'find ../%s/src/wl/clm/src/*.GEN | '
                'pax -rwkl -s "?.GEN??" release/%s' % (fw, fw),
                cwd=self.mwd)
            self.execute(
                'find ../%s/src/wl/clm/src/*.clm_blob | '
                'pax -rwkl %s/%s' % (fw, REL, fw),
                cwd=self.mwd)

    def release_pre_build(self):
        """Cribbed from linux-router.mk."""
        cmd = """
            LINUX_DIR=$[LINUX_DIR] src/tools/release/linux-router-rpb.sh %s
        """ % self.reldir
        self.execute(cmd, cwd=self.mwd)

        # Copy the pre-built wl*.[o.ko] to release directory
        self.mkdir(mkpath(self.mwd, self.reldir, 'src/wl/linux'))
        cmd = """
            perl -le '$to = "%s/src/wl/linux";
                    for (grep -f, @ARGV) {$cmd = "cp $_ $to";
                    print "+ $cmd"; exit(2) if system $cmd;}'
                $[LINUX_DIR]/drivers/net/wl/wl_*/*.{o,ko}
        """ % self.reldir
        self.execute(cmd, cwd=self.mwd)

        if self.swd:
            # Copy the pre-built dhd.[o.ko] to release directory
            self.mkdir('%s/release/%s/src/dhd/linux' % (self.mwd, self.swd))
            cmd = """
                perl -le '$to = "release/%s/src/dhd/linux";
                        for (grep -f, @ARGV) {$cmd = "cp $_ $to";
                        print "+ $cmd"; exit(2) if system $cmd;}'
                    $[LINUX_DIR]/drivers/net/dhd/*.{o,ko}
            """ % self.swd
            self.execute(cmd, cwd=self.mwd)

            # Copy the pre-built firmware to release/dhd directory
            for fw in glob.glob('%s/src/shared/rtecdc*.h' % self.swd):
                self.cp(fw, mkpath(self.mwd, REL, self.swd, 'src/shared'))

            # Copy clm blob file from src
            clm_from = mkpath(self.bldwd, self.mwd,
                              'src/wl/clm/src/router.clm_blob')
            if os.path.isfile(clm_from):
                clm_to = mkpath(self.bldwd, self.mwd,
                                '%s/src/wl/clm/src' % self.reldir,
                                'router.clm_blob')
                self.cp(clm_from, clm_to)

        cmd = """
            find %s/$[ARCH]* | pax -rwkl %s %s
        """ % (self.rwd, PAX_SKIP_VCS, self.reldir)
        self.execute(cmd, cwd=self.mwd)

        # Mogrify release/main src tree
        if self.mwd:
            cmd = self.xmogrify + """
                $[MOGRIFY_FLAGS] --list-files=%s/release_mogrified.list
                    --mogrifier=latest
                    --define $[MOGRIFY_ON]
                    --undefine $[MOGRIFY_OFF] PREBUILD
                    -- %s
            """ % (
                lib.opts.METADIR,
                mkpath(self.mwd, REL, self.mwd)
            )
            self.execute(cmd)

        # Mogrify release/dhd src tree
        if self.swd:
            cmd = self.xmogrify + """
                $[MOGRIFY_FLAGS] --list-files=%s/%s_release_mogrified.list
                    --mogrifier=latest
                    --define $[MOGRIFY_ON]
                    --undefine $[MOGRIFY_OFF] PREBUILD
                    -- %s
            """ % (
                lib.opts.METADIR,
                os.path.basename(self.swd),
                mkpath(self.mwd, REL, self.swd)
            )
            self.execute(cmd)

        # Mogrify firmware src tree
        for fw in self.fwds:
            cmd = self.xmogrify + """
                $[MOGRIFY_FLAGS] --list-files=%s/%s_release_mogrified.list
                    --mogrifier=latest
                    --define %s --undefine %s
                    -- %s
            """ % (
                lib.opts.METADIR,
                os.path.basename(fw),
                self.getvar('FW_MOGRIFY_ON'),
                self.getvar('FW_MOGRIFY_OFF'),
                mkpath(self.mwd, REL, fw)
            )
            self.execute(cmd)

        self.execute(mkpath(self.bldwd,
                            self.mwd,
                            'src/tools/release/linux-postbuild.sh'),
                     cwd=mkpath(self.mwd, self.reldir))

    def final_build(self):
        """Cribbed from linux-router.mk."""
        cmd = 'pax -rwkl %s -s "?^release?build?" release .' % PAX_SKIP_VCS
        self.execute(cmd, cwd=self.mwd)

        self.rm(mkpath(self.mwd, BLD, self.mwd, self.rwd, '.config'),
                mkpath(self.mwd, BLD, self.mwd, '$[LINUX_DIR]/.config'))

        bldsrc = mkpath(BLD, self.mwd, 'src')
        cmd = '$[MAKE] -C %s %s %s $[PREBUILT]' % (bldsrc, MJOBS, MVARS)
        self.execute(cmd, cwd=self.mwd)

        cmd = """
            pax -rwkl %s -s "?^build?release?" %s .
        """ % (PAX_SKIP_VCS, mkpath(BLD, self.mwd, 'image'))
        self.execute(cmd, cwd=self.mwd)

        # See http://jira.broadcom.com/browse/SWWLAN-35629. This is easier
        # than getting make to do the copy and for routers it's good enough.
        clm_gen_dir = mkpath(self.mwd, 'release/src/generated/wl/clm/src')
        clm_data_gen = mkpath(clm_gen_dir, 'wlc_clm_data.c.GEN')
        if os.path.isfile(clm_data_gen):
            self.cp(clm_data_gen,
                    mkpath(self.mwd,
                           self.reldir,
                           'src/wl/clm/src/wlc_clm_data.c'))

        # Copy clm blob files. Note that these really shouldn't be in
        # the 'generated' area, because it was intended for generated
        # *source* files, but that's where they are.
        for clmb in glob.glob('%s/*.clm_blob' % clm_gen_dir):
            self.cp(clmb, mkpath(self.mwd, self.reldir, 'src/wl/clm/src'))

# vim: ts=8:sw=4:tw=80:et:
