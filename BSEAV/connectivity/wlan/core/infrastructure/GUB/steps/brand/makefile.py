"""
Build in legacy mode using a brand makefile.

Most of what's done below is an attempt to copy what build_linux.sh did.
Much of it may be redundant or obsolete. This step is supposed to be
transitional so not much work was put into making it pretty.

There are two kinds of legacy mode: "full" and "prepopulated".  Full
legacy mode is analogous to build_brands (BB); it copies <brand>.mk to
"release.mk" in the current working directory, invokes make on it,
and lets the makefile handle the entire build from there, including
checkout (aka tree population). In prepopulated legacy mode, GUB handles
populating the tree and hands off to the brand makefile only for the
rest, which is typically compilation and packaging.

Full legacy mode is strongly deprecated and no buildables still use it.
It should be removed from the code (TODO).  Prepopulated mode is also
deprecated but less strongly and remains in heavy use. New buildables
should not come through here at all.
"""

from __future__ import print_function
from __future__ import unicode_literals

import os
import re
import time

import lib.consts
import lib.opts
import lib.scm
import lib.times
import lib.util

import steps.base

RELDIR = 'src/tools/release'


class BrandMakefileStep(steps.base.Step):

    """Build in legacy mode using a brand makefile."""

    def __init__(self, *args, **kwargs):
        """Boilerplate initializer."""
        steps.base.Step.__init__(self, *args, **kwargs)

    def run(self):
        """Conventional entry point for running this step."""
        # Derive the buildable makefile name.
        srcmk = self.cfg.getvar('MAKEFILE', self.bldable + '.mk')

        # Build up a custom environment for the step.
        nenv = os.environ.copy()

        # Remove timezone for compatibility with legacy toolset.
        # Note that we've already forced TZ to "Broadcom Standard Time".
        nenv['SVNCUTOFF'] = re.sub(
            r'\{(\d+-?\d+-?\d+)T(\d+:?\d+:?\d+)(-\d+)\}', r'\1 \2',
            self.reftime
        )

        # Override the following assumption in linux-dongle-image-launch.mk
        # that FW images should always be borrowed from "TODAY" even when
        # the reftime actually refers to a previous day:
        #     TODAY ?= $(shell date +'%Y.%-m.%-d')
        try:
            secs = lib.times.iso2seconds(self.reftime)
        except Exception:
            pass
        else:
            # This originally used time.strftime('%Y.%-m.%-d', ...) where
            # the '-' is intended to suppress leading zeroes but apparently
            # it's not part of the Python API. Rather, it's supported by
            # some libc implementations and not others. On Cygwin it's been
            # observed to result in an empty string with no error message,
            # presumably because the Cygwin libc implementation ("newlib")
            # doesn't understand it. So instead we stick to the documented
            # format and remove leading zeroes manually.
            parts = time.strftime('%Y.%m.%d', time.localtime(secs)).split('.')
            nenv['TODAY'] = '.'.join([p.lstrip('0') for p in parts])

        svnbase = lib.consts.WL_REPO_ROOT
        if self.build and self.build.user_url:
            nenv['REPO_URL'] = svnbase
        nenv.update(self.env)
        nenv.update(self.cfg.exports(final=True))
        if self.platform == 'linux':
            nenv['HNDRTE_ITERS'] = '60'
            nenv['HNDRTE_WAITTIME'] = '120'
        nenv['SHELL'] = '/bin/bash'
        nenv['SVNBASE'] = svnbase
        nenv['SVNCMD'] = ' '.join(lib.scm.svncmd())

        # 'TAG' is inherited from BB and referenced in some brand makefiles.
        if self.tag != 'trunk':
            nenv['TAG'] = self.tag

        # Adapt to either modern multiple-tree build areas (flagged by
        # existence of the conventional "main" tree) or the traditional
        # single-tree model.
        if os.path.isdir('main/src'):
            mwd = 'main'
            lib.util.mkdir(os.path.join(mwd, lib.opts.METADIR))
            envlog = os.path.join('..', lib.opts.METADIR, ',env.log')
        else:
            mwd = '.'
            envlog = os.path.join(lib.opts.METADIR, ',env.log')

        # Determine which make program to use.
        mkprog = self.cfg.makeprog(self.tag, default='gmake')

        # Always use "hnd make" to wrap the make program. Partly for
        # consistency since we need it in emake/lsmake modes anyway, but
        # the more important reason is to have the same executable name
        # everywhere since "gmake" doesn't exist on Mac and "make"
        # finds the wrong (BSD) make on NetBSD. These details are handled
        # by "hnd make".
        cmd = lib.util.xscript() + ['make', '--%s' % mkprog]

        mkflags = []
        if mkprog == 'lsmake':
            lib.util.die('static lsmake use removed - dynamic preferred')
        elif mkprog == 'emake':
            cmd.extend(['--tag', self.tag])
            cmd.extend(['--buildable', self.bldable])

        # Derive the make version.
        # Use self.execute() to ensure MAKEVER overrides are present.
        vcmd = cmd + ['--', '-v']
        vcmd.insert(2, '--quiet')
        proc = self.execute(vcmd, stdout=lib.util.PIPE, replay=False, vl=3)
        stdout = proc.communicate()[0]
        if proc.returncode == 0:
            mkvers = stdout.split('\n')[0]
            print('Using: %s' % mkvers)
        else:
            mkvers = ''

        # The -O flag is useful for descrambling build log output.
        # It was introduced with GNU make 4.0 so we must avoid it
        # for older versions and non-GNU make programs.
        # But we do not want -O in lsmake-dynamic mode because the
        # top-level make will just be the wrapper for running lsmake.
        # No sense hiding that interaction till build end.
        if 'GNU Make ' in mkvers and 'GNU Make 3' not in mkvers:
            syncstyle = self.cfg.getvar('OUTPUT_SYNC')
            if syncstyle and not lib.lsf.lsmake_dynamic(self.cfg, self.tag):
                mkflags.append('--output-sync=%s' % syncstyle)

        # Undocumented, untested, advanced make debug feature.
        if self.opts.debug_tgt:
            cmd.append('--debug-tgt=' + self.opts.debug_tgt)

        # Pass verbosity along.
        for _ in range(1, lib.opts.VERBOSITY):
            cmd.insert(2, '-V')

        # Write the enhanced env to ,env.log.
        cmd.append('--env-log=' + envlog)

        # Now append the actual make command line.
        cmd.append('--')
        cmd.extend(mkflags)
        cmd.extend(['-f', srcmk, '-I', RELDIR])
        # Can't rename BRAND here without renaming references in makefiles too.
        cmd.append('BRAND=' + self.bldable)
        cmd.append('NOMAXSIZE=1')
        cmd.append('GUB_LEGACY_MODE=1')

        for name in self.cfg.getvars():
            if name.startswith('LSMAKE_'):
                rhs = self.cfg.getvar(name)
                if name == 'LSMAKE_FLAGS' and lib.util.verbose_enough(2):
                    rhs = '-V %s' % rhs
                cmd.append('%s=%s' % (name, lib.util.quoted(rhs)))

        if lib.util.verbose_enough(3):
            cmd.append('V=1')

        # Special case aka hack. When users make a patch it could
        # affect firmware but that would be defeated if the FW is
        # borrowed from a previous unpatched HDW build. Thus a
        # patch forces a new, private hndrte-dongle-wl build.
        # We could do the same thing with --copydir but there's
        # a difference: the worst thing that could come from forcing
        # HDW with --patch is extra work and time, whereas forcing
        # it with --copydir could cause HDW to fail if the copy is the
        # wrong DEPS type. The ideal might be to derive a patch from
        # the --copydir and pass it to HDW but that would require new
        # coding and complexity.
        if self.patch:  # or lib.opts.COPYDIR:
            cmd.append('FORCE_HNDRTE_BUILD=1')

        # Analogous to the -flags option of build_brands.
        cmd.extend(self.opts.makeflags)

        # The brand makefile must be copied to the base directory and
        # used from there along with mogrify_common_rules.mk. This
        # is required by the way mogrify_common_rules.mk is used,
        # because make does not search the -I path for -f values.
        rel = os.path.join(mwd, RELDIR)
        lib.util.cp(os.path.join(rel, srcmk), mwd)
        lib.util.cp(os.path.join(rel, 'mogrify_common_rules.mk'), mwd)
        lib.util.cp(os.path.join(rel, 'brand_common_rules.mk'), mwd)

        # Suppress checkout logic in checkout-rules.mk since
        # we're handling it ourselves in the Populate step.
        nenv['CHECKOUT_RULES_DISABLED'] = '1'

        self.execute(cmd, env=nenv, cwd=mwd)

# vim: ts=8:sw=4:tw=80:et:
