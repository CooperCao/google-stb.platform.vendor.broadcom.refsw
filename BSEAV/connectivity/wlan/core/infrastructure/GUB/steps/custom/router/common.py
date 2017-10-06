"""Code or data common to this package."""

from __future__ import print_function
from __future__ import unicode_literals

import os

MAIN_DIR = 'main'
SYS_DIR = 'sys'
FW_DIR = 'dongle'
CHIPDIRS = ['43602', '43602a1', '43602a3', '4365', '4366', '43684']


def locate():
    """Adapt to build trees placed here by the populate step."""
    if os.path.isdir(MAIN_DIR):
        mwd = MAIN_DIR
    else:
        mwd = '.'

    if os.path.isdir(SYS_DIR):
        swd = SYS_DIR
    elif os.path.isdir('dhd'):
        swd = 'dhd'
    else:
        swd = None

    fwds = []
    if os.path.isdir(FW_DIR):
        fwds.append(FW_DIR)
    else:
        fwds.extend([d for d in CHIPDIRS if os.path.isdir(d)])

    if os.path.isdir(os.path.join(mwd, 'components/router')):
        rwd = 'components/router'
    else:
        rwd = 'src/router'

    return mwd, swd, fwds, rwd

# vim: ts=8:sw=4:tw=80:et:
