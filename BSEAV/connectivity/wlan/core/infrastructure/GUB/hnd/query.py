"""
Query the config file and print results to stdout.

EXAMPLES:

To print a list of all known queries (a meta-query):

    %(prog)s query queries

To print a list of all known buildable items:

    %(prog)s query known_buildables

To print the default build list for a given tag:

    %(prog)s query enabled_buildables -t DINGO_BRANCH_9_10

To print the set of tags on which a given buildable item is turned on:

    %(prog)s query enabled_tags -b linux-internal-wl

To print a list of all tags with a default build set:

    %(prog)s query active_tags

The standard build area for a given buildable and tag:

    %(prog)s query build_path -t trunk -b efi-external-wl

Show partitions used for REL builds sorted by free space:

    %(prog)s query static_partitions

Print the value of MOGRIFY_ON as used by a given buildable:

    %(prog)s query -b linux-2.6.36-arm-internal-router variable MOGRIFY_ON
"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import collections
import os
import pprint
import sys

import lib.consts
import lib.disk
import lib.item
import lib.results
import lib.scm
import lib.util


def parse_cli(prog, alias, subcmds, advanced, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser.add_argument(
        '-b', '--buildable', default=[], action='append',
        help="specify a buildable item (may be repeated)")
    parser.add_argument(
        '-l', '--long-format', action='count', default=0,
        help="print a more verbose format"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-p', '--pattern', default=[], action='append',
        metavar='RE',
        help="filter buildables by /RE/ (slashes optional)")
    parser.add_argument(
        '-s', '--simple-format', action='count', default=0,
        help="print a simplified format for use by tools"
        if advanced else argparse.SUPPRESS)
    parser.add_argument(
        '-t', '--tag', default=[], action='append',
        help="basename of branch/tag/twig/trunk (may be repeated)")
    parser.add_argument('args', nargs='*')
    parser.set_defaults(func=call)


def call(opts, cfgproxy):
    """Standard subcommand entry point."""
    cfgroot = cfgproxy.parse()

    rc = 0

    recognized_queries = {
        'queries': ('queries', True),
        'active': ('active_tags', True),
        'active_branches': ('active_tags', True),
        'active_tags': ('active_tags', True),
        'build_path': ('build_path', True),
        'build_status': ('build_status', True),
        'enabled': ('enabled_buildables', False),
        'enabled_buildables': ('enabled_buildables', True),
        'enabled_builds': ('enabled_builds', True),
        'enabled_brands': ('enabled_buildables', True),
        'enabled_tags': ('enabled_tags', True),
        'find_best_partition': ('find_best_partition', True),
        'known_buildables': ('known_buildables', True),
        'known_branches': ('known_tags', True),
        'known_tags': ('known_tags', True),
        'latest': ('latest', False),
        'latest_build': ('latest', True),
        'static_partitions': ('_partitions', True),
        'svn_repo': ('svn_repo', True),
        'dynamic_partitions': ('_partitions', True),
        'precommit_partitions': ('_partitions', True),
        'archive_partitions': ('_partitions', True),
        'preserved_partitions': ('_partitions', True),
        'tag_path': ('tag_path', True),
        'treemap': ('treemap', True),
        'uses_hdw': ('uses_hdw', True),
        'variable': ('variable', True),
    }

    # Derive the ordered list of tags (usually just one) involved.
    tags = []
    for t in lib.util.tolist(opts.tag, 'trunk'):
        if t == lib.consts.ALL_ENABLED:
            for tag in cfgroot.active_tags():
                if tag not in tags:
                    tags.append(tag)
        else:
            tag = cfgroot.expand_tag(t)
            if tag not in tags:
                tags.append(tag)

    if opts.args:
        arg0 = query = opts.args.pop(0)
        if query in recognized_queries:
            query = recognized_queries[query][0]
    else:
        query = 'queries'

    if query == 'queries':
        if not opts.simple_format:
            print('SUPPORTED QUERIES:')
        for query in sorted(recognized_queries):
            qtuple = recognized_queries[query]
            if not qtuple[1]:
                continue
            if opts.simple_format:
                print(query)
            else:
                print('%s query %s' % (lib.consts.PROG, query))
    elif query == 'enabled_builds':
        for tag in cfgroot.active_tags():
            for item in lib.item.derive_build_item_list(cfgroot, opts, tag)[1]:
                print('-t %s -b %s' % (tag, item))
    elif query == 'enabled_buildables':
        for tag in tags:
            items = lib.item.derive_build_item_list(cfgroot, opts, tag)[1]
            names = set([item.name for item in items])
            if opts.buildable:
                names = set(names) & set(lib.util.tolist(opts.buildable))
            for name in sorted(names):
                print(name)
    elif query == 'known_buildables':
        lib.util.assert_(not opts.tag, 'this query assumes "-t ALL"')
        for buildable in cfgroot.known_buildables():
            print(buildable)
    elif query == 'enabled_tags':
        for buildable in lib.util.tolist(opts.buildable):
            for tag in cfgroot.enabled_tags(buildable):
                print(tag)
    elif query == 'active_tags':
        # These are called tags internally but "active tags" is an oxymoron
        # so we support referring to them as branches too.
        tagdata = []
        for tag in cfgroot.active_tags():
            toff = cfgroot.tags.get(tag, {}).get('nightly_offset', 0)
            builds = slots = 0
            for build in cfgroot.enabled_buildables(tag):
                builds += 1
                cfg = cfgroot.buildables().get(build)
                if cfg:
                    # When a range is configured, e.g. "LSF_SLOTS=4,12",
                    # use the minimum for calculations.
                    slots += int(cfg.getvar('LSF_SLOTS', '1').split(',')[0])
                else:
                    lib.util.error('misconfigured buildable: "%s"' % build)
                    rc = 2
            if lib.consts.NIGHTLY_DAYS in cfgroot.tags.get(tag, {}):
                nd = '*'
            else:
                nd = ' '
            tagdata.append((tag, toff, nd, builds, slots))
        if not opts.simple_format:
            print('%-30s %s %s %s' % ('TAG', 'OFFSET', 'BUILDS', 'SLOTS'))
        for td in sorted(tagdata, key=lambda t: t[1]):
            if opts.simple_format:
                print(td[0])
            else:
                print('%-30s   %.2f%s %5d %5d' % td)
    elif query == 'known_tags':
        for tag in cfgroot.known_tags():
            print(tag)
    elif query == 'build_path':
        for tag in tags:
            for buildable in lib.util.tolist(opts.buildable):
                if buildable.startswith('macos'):
                    plat = 'macos'
                elif buildable.startswith('netbsd'):
                    plat = 'netbsd'
                elif buildable.startswith('win'):
                    plat = 'windows'
                elif buildable.startswith('efi'):
                    plat = 'windows'
                else:
                    plat = 'linux'
                path = os.path.join(lib.consts.STATE.base,
                                    'build_' + plat,
                                    cfgroot.expand_tag(tag),
                                    buildable)
                print(path)
    elif query == 'build_status':
        for tag in tags:
            items = lib.item.derive_build_item_list(cfgroot, opts, tag)[1]
            names = set([item.name for item in items])
            results = {obj.bldable: obj for obj in
                       lib.results.find_results(cfgroot, tag)[0]}
            failures = {}
            for name in names:
                result = results.get(name)
                if result:
                    if not result.passed:
                        failures[name] = result.log
                else:
                    failures[name] = None
            if not opts.simple_format:
                sys.stdout.write('%s: ' % tag)
            if failures:
                if opts.long_format:
                    output = lib.util.bytestrings(failures)
                    if opts.long_format == 1:
                        print(output)
                    else:
                        pprint.pprint(output)
                else:
                    print('FAIL')
            else:
                print('PASS')
    elif query == 'latest':
        for tag in tags:
            _, items = lib.item.derive_build_item_list(cfgroot, opts, tag)
            names = set([item.name for item in items])
            results, _ = lib.results.find_results(cfgroot, tag)
            for result in results:
                if result.bldable in names:
                    if opts.simple_format:
                        print(result.path)
                    elif opts.long_format:
                        print(repr(result))
                    else:
                        print('%s %s: %s' % (result.bldable,
                                             result.state,
                                             result.path))
    elif query.endswith('partitions'):
        if query == 'partitions':
            ptypes = cfgroot.get(query).keys()
        else:
            ptypes = [arg0.split('_')[0]]

        for ptype in ptypes:
            partitionlist = cfgroot.get('partitions').get(ptype)
            if partitionlist:
                fmt = '%-36s %14s %s'
                partmap = lib.disk.status(partitionlist.values())
                if not opts.simple_format:
                    heading = '_'.join([ptype.upper(), 'PARTITIONS'])
                    print(fmt % (heading, 'BYTES FREE', 'USE%'))
                for entry in sorted(partmap, key=lambda x: partmap[x].free):
                    if opts.simple_format:
                        print(entry)
                    else:
                        pct = '{:.0%}'.format(partmap[entry].used /
                                              float(partmap[entry].total))
                        print(fmt % (entry, partmap[entry].free, ' ' + pct))
            else:
                lib.util.die('unrecognized partition type: "%s"' % ptype)
    elif query == 'find_best_partition':
        for tag in tags:
            for plat in lib.consts.HOST_PLATFORMS:
                partition = lib.disk.findbest(cfgroot, plat, tag)
                target = lib.disk.platpath(partition, 'build_' + plat, tag)
                print(target)
    elif query == 'svn_repo':
        # Remote sites may be queried by exporting SITE=<site>.
        if opts.simple_format:
            print(lib.consts.WL_REPO_ROOT)
        else:
            print('wlan=' + lib.consts.WL_REPO_ROOT)
            print('ucode=' + lib.consts.UC_REPO_ROOT)
    elif query == 'tag_path':
        for tag in tags:
            print(lib.util.mkpath(lib.consts.WL_REPO_ROOT,
                                  'proj', lib.scm.svn_tag_path(tag)))
    elif query == 'treemap':
        for tag in tags:
            treemap = collections.defaultdict(set)
            if opts.buildable:
                items = [lib.item.Item(b)
                         for b in lib.util.tolist(opts.buildable)]
            else:
                items = lib.item.derive_build_item_list(cfgroot, opts, tag)[1]

            known = cfgroot.buildables()
            for item in items:
                steps = known[item.name].get('steps', {})
                imap = steps.get('Populate', {}).get('treemap')
                if not imap:
                    lib.util.warn('no treemap for %s - skipped' % item.name)
                    continue
                extravars = [
                    'BUILDABLE=' + item.name,
                    'TAG=' + tag,
                    'TAGPATH=' + lib.scm.svn_tag_path(tag),
                ]
                for path in imap:
                    url = imap.get(path, extravars=extravars, final=True)
                    treemap[path].add(url)
            for path in treemap:
                print('%s: %s' % (path, ' + '.join(sorted(treemap[path]))))
    elif query == 'uses_hdw':
        for tag in tags:
            _, items = lib.item.derive_build_item_list(cfgroot, opts, tag)
            names = set([item.name for item in items])
            if opts.buildable:
                names = set(names) & set(lib.util.tolist(opts.buildable))
            for name in sorted(names):
                if cfgroot.buildable(name).uses_hdw():
                    print(name)
    elif query == 'variable':
        if opts.buildable:
            cfg = cfgroot.buildable(opts.buildable.pop())
            lib.util.assert_(not opts.buildable, 'only one buildable allowed')
        else:
            cfg = cfgroot
        for variable in opts.args:
            print(cfg.getvar(variable, ''))
    else:
        lib.util.error('unrecognized query: "%s"' % query)
        rc = 2

    sys.exit(rc)

# vim: ts=8:sw=4:tw=80:et:
