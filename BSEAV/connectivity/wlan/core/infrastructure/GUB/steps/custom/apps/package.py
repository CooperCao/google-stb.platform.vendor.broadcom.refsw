"""Manage the packaging part of a combined-apps build."""

from __future__ import unicode_literals

import os
import subprocess

import lib.util

import steps.custom.base
import steps.custom.apps.common

# Derive this automatically so it will adjust to updated servers.
if os.path.exists('/usr/bin/sw_vers'):
    OSVER = '.'.join(subprocess.check_output(
        ['/usr/bin/sw_vers', '-productVersion']).rstrip().split('.')[:2])
else:
    OSVER = None

# This map could be expressed in yaml instead but since some
# special-casing is required in methods below it may make sense
# to keep it here so methods and mapping can be modified in the
# same file.
PKGMAP = {
    'android-combined-apps': {
        'android_x86_64': {
            'src/wl/exe/wlx86_android_ndk_r6b':
            'bcm/android_x86_64/wlx86_android_ndk_r6b',
        },
    },
    'linux-combined-apps': {
        'x86': {
            'src/wl/exe/x86/wl': 'bcm/x86/wl',
            'src/wl/exe/x86/wl_server_dongle':
                'bcm/x86/wl_server_dongle',
            'src/wl/exe/x86/wl_server_serial':
                'bcm/x86/wl_server_serial',
            'src/wl/exe/x86/wl_server_socket':
                'bcm/x86/wl_server_socket',
            'src/wl/exe/x86/wl_server_wifi':
                'bcm/x86/wl_server_wifi',
            'src/dhd/exe/dhd': 'bcm/x86/dhd',
        },
        'x86_64': {
            'src/wl/exe/x86_64/wl': 'bcm/x86_64/wl',
            'src/wl/exe/x86_64/wl_server_dongle':
                'bcm/x86_64/wl_server_dongle',
            'src/wl/exe/x86_64/wl_server_serial':
                'bcm/x86_64/wl_server_serial',
            'src/wl/exe/x86_64/wl_server_socket':
                'bcm/x86_64/wl_server_socket',
            'src/wl/exe/x86_64/wl_server_wifi':
                'bcm/x86_64/wl_server_wifi',
        },
    },
    'linux-efi-combined-apps': {
        'X64': {
            'src/wl/exe/efi/X64/uefidbg/wl.efi': 'bcm/X64/wl.efi',
            'src/dhd/exe/efi/X64/uefidbg/dhd.efi': 'bcm/X64/dhd.efi',
        },
    },
    'macos-combined-apps': {
        'macos': {
            'src/wl/exe/macos/%s/wl' % OSVER: 'bcm/macos/wl',
        },
        'macos-%s' % OSVER: {
            'src/wl/exe/macos/%s/wl' % OSVER: 'bcm/macos-%s/wl' % OSVER,
        },
    },
    'win_combined_apps': {},
}


class AppsPackageStep(steps.custom.base.BrandStep):

    """Manage the packaging part of a combined-apps build."""

    def __init__(self, *args, **kwargs):
        steps.custom.base.BrandStep.__init__(self, *args, **kwargs)

    def run(self):
        """Conventional entry point for running this step."""
        for subdir in steps.custom.apps.common.SUBDIRS:
            self.as_step(self.package, (subdir,))

    def package(self, subdir):
        """Copy generated files to delivery paths and tar them up."""
        pkgmap = PKGMAP[self.bldable]
        for variant in pkgmap:
            for srcpath, dstpath in pkgmap[variant].items():
                srcpath = os.path.join(subdir, srcpath)
                dstpath = os.path.join(subdir, dstpath)
                lib.util.mkdir(os.path.dirname(dstpath))
                self.execute('cp -p {0} {1}'.format(srcpath, dstpath))

        self.execute('cd {0} && tar -czf {0}.tar.gz bcm'.format(subdir))

# vim: ts=8:sw=4:tw=80:et:
