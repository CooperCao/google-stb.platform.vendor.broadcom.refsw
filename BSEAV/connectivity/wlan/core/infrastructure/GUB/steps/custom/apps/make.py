"""Manage the compile-and-link part of a combined-apps build."""

from __future__ import unicode_literals

import lib.util

import steps.custom.base
import steps.custom.apps.common


class AppsMakeStep(steps.custom.base.BrandStep):

    """Manage the compile-and-link part of a combined-apps build."""

    def __init__(self, *args, **kwargs):
        steps.custom.base.BrandStep.__init__(self, *args, **kwargs)

    def run(self):
        """Conventional entry point for running this step."""
        for subdir in steps.custom.apps.common.SUBDIRS:
            lib.util.mkdir(subdir)
            upname = subdir.title()
            self.as_step(self.srcfilter, (subdir,),
                         stepname='Filter-' + upname)
            if subdir != 'internal':
                self.as_step(self.mogrify, (subdir,),
                             stepname='Mogrify-' + upname)
            self.as_step(self.make_wl, (subdir,),
                         stepname='Make-wl-' + upname)
            self.as_step(self.make_dhd, (subdir,),
                         stepname='Make-dhd-' + upname)
            if not self.bldable.startswith('win_'):
                self.as_step(self.make_logprint, (subdir,),
                             stepname='Make-logprint-' + upname)
            self.as_step(self.cleanup, (subdir,),
                         stepname='Cleanup-' + upname)

    def srcfilter(self, subdir):
        """Copy the subset of files we're actually going to build with."""
        cmd = ' | '.join([
            'find src components',
            'srcfilter.py -d -v $[FILELISTS_{0}]'.format(subdir.upper()),
            'pax -rw {0}'.format(subdir)
        ])
        self.execute(cmd)

    def mogrify(self, subdir):
        """Mogrify files which may be part of a source package."""
        cmd = ' '.join([
            'xmogrify -q --mogrifier=src/tools/build/mogrify.pl',
            '--define $[MOGRIFY_ON_{0}]'.format(subdir.upper()),
            '--undefine $[MOGRIFY_OFF_{0}]'.format(subdir.upper()),
            '-- {0}'.format(subdir)
        ])
        self.execute(cmd)
        self.execute('tar -czf {0}-src.tar.gz *'.format(subdir), cwd=subdir)

    def make_wl(self, subdir):
        """Build the wl app."""
        cmd = '$[MAKE] -C {0}/$[WLDIR] $[WLARGS]'.format(subdir)
        cmd += ' $[WLARGS_{0}]'.format(subdir.upper())
        cmd += ' $[MAKEFLAGS]'
        if self.bldable == 'linux-combined-apps':
            # This special case appears to be necessary because the linux wl
            # makefile doesn't distinguish 32- and 64-bit variants so we need
            # to build one, move it away, then build the other.
            make32 = ' '.join([cmd, '$[MAKEJOBS]', 'NATIVEBLD=1',
                               'CC="gcc -m32"'])
            self.execute(make32)
            inst32 = ' '.join([cmd, 'INSTALL_DIR=x86',
                               'TARGETARCH=x86', 'release_bins'])
            self.execute(inst32)
            clean32 = ' '.join([cmd, 'clean'])
            self.execute(clean32)

            make64 = ' '.join([cmd, '$[MAKEJOBS]', 'NATIVEBLD=1',
                               'CC="gcc -m64"'])
            self.execute(make64)
            inst64 = ' '.join([cmd, 'INSTALL_DIR=x86_64', 'release_bins'])
            self.execute(inst64)
        elif self.bldable == 'macos-combined-apps':
            self.execute(' '.join([cmd, '$[MAKEJOBS]']))
            # These don't work on the same OS version as the above.
            # xcwd = self.sub('{0}/$[WLDIR]').format(subdir)
            # xcmd = 'xcodebuild -sdk $[SDK] -configuration Release -project '
            # self.execute(xcmd + 'wlm.xcodeproj', cwd=xcwd)
            # self.execute(xcmd + 'wl.xcodeproj', cwd=xcwd)
        elif self.bldable == 'win_combined_apps':
            # There seems to be a problem in GNUmakefile involving -j on
            # Windows so parallelism is suppressed until that's solved.
            for ext in ('', '.wlm_dll', '.wlu_dll'):
                self.execute(' '.join([cmd, '-f', 'GNUmakefile' + ext]))
        else:
            self.execute(' '.join([cmd, '$[MAKEJOBS]']))

    def make_dhd(self, subdir):
        """Build the dhd app."""
        cmd = '$[MAKE] -C {0}/$[DHDDIR] $[MAKEJOBS] $[DHDARGS]'.format(subdir)
        cmd += ' $[DHDARGS_{0}]'.format(subdir.upper()).rstrip()
        if self.bldable == 'linux-combined-apps':
            cmd = ' '.join([cmd, 'CC="gcc -m32"'])
        self.execute(cmd)

    def make_logprint(self, subdir):
        """Build the logprint app."""
        cmd = '$[MAKE] -C {0}/$[LOGPRINTDIR] $[MAKEJOBS] logprint'.format(
            subdir)
        self.execute(cmd)

    def cleanup(self, subdir):
        """Remove unnecessary build byproducts."""
        cmd = 'find {0} {1} -exec rm -f {{}} +'.format(
            subdir,
            r'\( -name "*.depend" -o -name "*.d" \)'
        )
        self.execute(cmd)

# vim: ts=8:sw=4:tw=80:et:
