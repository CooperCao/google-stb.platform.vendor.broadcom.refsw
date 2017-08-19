"""Represent the unit of build, aka 'buildable item' or 'brand'."""

from __future__ import print_function
from __future__ import unicode_literals

import os.path
import re
import sys

import lib.consts
import lib.results
import lib.scm
import lib.util


class Item(object):

    """Represent the unit of build, aka 'buildable' or 'brand'."""

    def __init__(self, text):
        words = text.split(':', 1)
        if len(words) == 1:
            name = words[0]
            if name.startswith('macos'):
                hplat = 'macos'
            elif name.startswith('netbsd'):
                hplat = 'netbsd'
            elif name.startswith('win') or name.startswith('efi'):
                hplat = 'windows'
            else:
                hplat = 'linux'
        else:
            name = words[1]
            hplat = words[0]

        self.name = name
        self.hplat = hplat
        self.offset = 0.0

    def __cmp__(self, other):
        return cmp(self.combined, other.combined)

    def __hash__(self):
        return hash(self.combined)

    def __repr__(self):
        return self.name

    @property
    def combined(self):
        """Return "<host-platform>:<name>" form."""
        return ':'.join([self.hplat, self.name])

    @property
    def win_hosted(self):
        """Return True iff the specified buildable runs on Windows."""
        return self.hplat.startswith('win') or self.hplat.startswith('efi')


def derive_build_item_list(cfgroot, opts, tag, warn_hdw=False):
    """Expand out a list of unique builds, preserving CLI order."""
    rc = 0
    items = []

    # Buildables may be specified with the -b flag or, in some cases,
    # as positional parameters. If no buildables specified, assume ALL.
    if 'buildables' in opts and opts.buildables:
        requests = opts.buildable + opts.buildables
    elif opts.buildable:
        requests = opts.buildable
    else:
        requests = [lib.consts.ALL_ENABLED]
    ref_tag = tag
    for text in lib.util.tolist(requests):
        if text == lib.consts.ALL_KNOWN:
            for known in cfgroot.known_buildables():
                item = Item(known)
                if item not in items:
                    items.append(item)
            continue
        elif text == lib.consts.ALL_ENABLED:
            # Find the right build set to work from. If building a
            # tag or twig we may need to inherit from a parent branch.
            enabled = []
            path, rev = (tag, None)
            while path:
                ref_tag = os.path.basename(path)
                if rev and 'verbose' in opts and opts.verbose:
                    msg = 'inheriting build set from %s ...' % ref_tag
                    lib.util.note(msg, vl=2)
                enabled = cfgroot.enabled_buildables(ref_tag)
                if enabled:
                    # Special case: it's wasteful to have more than one
                    # dongle build in the default set without HDW present
                    # too because each would need to build it privately.
                    hdw_users = [b for b in enabled if
                                 cfgroot.buildable(b).uses_hdw()]
                    if warn_hdw and len(hdw_users) > 1:
                        if lib.consts.HDW not in enabled:
                            msg = 'dongle builds (%s) on %s without %s %s' % (
                                ', '.join(hdw_users),
                                ref_tag, lib.consts.HDW, 'for seeding'
                            )
                            lib.util.warn(msg)
                    break
                if path.endswith('/trunk'):
                    break
                path, rev = lib.scm.svn_tag_parent(path, rev)[0:2]
            for en in enabled:
                item = Item(en)
                if item not in items:
                    items.append(item)
            continue

        item = Item(text)
        if item not in items:
            items.append(item)

    # Filter out passed builds on request.
    if items and 'failed' in opts and opts.failed:
        results, _ = lib.results.find_results(cfgroot, tag)
        ok = [Item(r.bldable) for r in results if r.passed or r.live]
        # Do set arithmetic while preserving list order.
        if opts.failed == 1:
            for item in set(items) & set(ok):
                items.remove(item)
        else:
            for item in set(items) - set(ok):
                items.remove(item)

    # Filter list by pattern if requested. Multiple patterns
    # form an "or" relationship.
    if items and 'pattern' in opts and opts.pattern:
        survivors = set()
        for pattern in opts.pattern:
            # We'd need better bsub/ssh quoting to support this.
            lib.util.assert_('|' not in pattern, 'no "|" allowed in pattern')
            pattern = lib.util.rmchars(pattern, '/')
            survivors |= set([i for i in items if re.search(pattern, i.name)])
        for item in items[:]:
            if item not in survivors:
                items.remove(item)

    # Filter explicitly-skipped patterns from build list.
    if items and 'except_pattern' in opts and opts.except_pattern:
        for pattern in opts.except_pattern:
            pattern = lib.util.rmchars(pattern, '/')
            for item in items[:]:
                if re.search(pattern, item.name):
                    items.remove(item)

    # Check validity of final build list. Make sure the special HDW
    # buildable is first since other buildables might depend on it.
    for i, item in enumerate(items):
        if not cfgroot.buildables().get(item.name):
            lib.util.warn('no such build item: "%s"' % item.name)
            items.pop(i)
            rc = 1
        elif item.name == lib.consts.HDW:
            items.insert(0, items.pop(i))

    # Keep lazy or naive un-privileged users from building all items.
    # We must allow them to start from ALL in order to filter down
    # but the result should be a proper subset of ALL unless it's a
    # small list to start with. A number of exceptions are provided.
    if lib.consts.ALL_ENABLED in requests and \
       lib.consts.PROG == 'gub' and \
       not lib.util.am_builder() and \
       len(items) > 4 and \
       len(items) >= len(set(cfgroot.enabled_buildables(ref_tag))) and \
       not opts.test_tree and \
       opts.verbose < 2 and \
       tag in cfgroot.active_tags() and \
       ('diffs_from' in opts and not opts.diffs_from) and \
       ('to_users' in opts and opts.to_users < 2) and \
       ('nightly' in opts and not opts.nightly):
        lib.util.error('builds must be chosen individually'
                       ' from the following set of %d:' % len(items))
        for item in items:
            sys.stderr.write('\t%s\n' % item.name)
        rc = 1

    return rc, items


def items2todos(cfgroot, opts, tag):
    """Return a list of deps/sparse URLs for the specified buildable(s)."""
    todos = []
    items = derive_build_item_list(cfgroot, opts, tag)[1]
    for item in items:
        buildable = item.name
        bcfg = cfgroot.buildables().get(buildable)
        lib.util.assert_(bcfg, 'no such buildable item: ' + buildable)
        treemap = bcfg['steps']['Populate']['treemap']
        for todir, upath in treemap.items():
            if lib.util.is_url(upath) or os.path.isabs(upath):
                url = upath
            else:
                url = '/'.join([lib.consts.WL_REPO_ROOT, upath])

            extravars = [
                'BUILDABLE=' + buildable,
                'TAG=' + tag,
                'TAGPATH=' + lib.scm.svn_tag_path(tag),
            ]
            todir = bcfg.sub(todir, extravars=extravars)
            url = bcfg.sub(url, extravars=extravars)
            todos.append((todir, url))

    return todos

# vim: ts=8:sw=4:tw=80:et:
