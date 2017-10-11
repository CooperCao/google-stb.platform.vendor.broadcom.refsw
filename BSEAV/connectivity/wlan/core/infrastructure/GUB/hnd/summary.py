"""
Generate a build summary for the specified tag(s).

EXAMPLES:

To email a summary of the current build state of a branch/tag:

    %(prog)s summary -t IGUANA_BRANCH_13_10

To send results to stdout rather than emailing:

    %(prog)s summary -t IGUANA_BRANCH_13_10 -o -

To summarize multiple tags/branches at once:

    %(prog)s summary -t trunk -t IGUANA_BRANCH_13_10 ...

To summarize builds on a user copy:

    [%(prog)s svn2user MYCOPY]
    %(prog)s summary -u MYCOPY -t trunk

To list available logfiles for branch 13.10:

    %(prog)s summary -t 13.10 -l

To "follow" (tail -f) a running build of hndrte-dongle-wl on trunk:

    %(prog)s summary -t trunk -b hndrte-dongle-wl -f

"""

from __future__ import print_function
from __future__ import unicode_literals

import argparse
import datetime
import os
import re
import shutil
import string
import sys
import tempfile
import time

import lib.bld
import lib.consts
import lib.mail
import lib.opts
import lib.results
import lib.times
import lib.util


def parse_cli(prog, alias, subcmds, _, usage):
    """Standard subcommand command line option definitions."""
    parser = subcmds.add_parser(
        alias,
        epilog=usage,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        prog=prog)
    parser_logs = parser.add_mutually_exclusive_group()
    parser.add_argument(
        '--all', action='count', default=0,
        help="check even buildables not enabled for this branch")
    parser.add_argument(
        '-b', '--buildable', default=[], action='append',
        help="limit search to specified buildable(s)")
    parser.add_argument(
        '-c', '--cc-blames', action='store_true',
        help="add relevant committers to emails")
    parser.add_argument(
        '-d', '--bldday',
        metavar='DAY',
        help="summarize DAY (format=yyyy.m.d, default=today)")
    parser_logs.add_argument(
        '-f', '--follow', action='count', default=0,
        help="find the most recent live build log and 'tail -f' it")
    parser_logs.add_argument(
        '-l', '--logfiles', action='count', default=0,
        help="show available logfiles for specified tag/buildable set")
    parser.add_argument(
        '-m', '--mailto', default=[], action='append',
        metavar='ADDRESSES',
        help="comma- or space-separated list of addresses, default=$USER")
    parser.add_argument(
        '--message', default='',
        help="text to include with the summary email")
    parser.add_argument(
        '-o', '--outfile',
        help="save output to this file (use '-' for stdout)")
    parser.add_argument(
        '-r', '--reftime',
        help="report only on builds pegged to this time")
    parser.add_argument(
        '-t', '--tag', default=[], action='append',
        help="basename of branch/tag/twig/trunk (may be repeated)")
    parser.add_argument(
        '--to-users', action='count',
        help="move search for build results into USERS area")
    parser.add_argument(
        '-u', '--user-url',
        metavar='URL',
        help="alternative base url for svn files")
    parser.set_defaults(func=call)

TOKENS = {
    'http_base': lib.consts.HTTP_BASE,
    'twiki_url': 'http://hwnbu-twiki.%s/bin/view/Mwgroup' % lib.consts.DOMAIN,
}

HEADFMT = '======== %s ========\n'


###############################################################################
SUMMARY_TMPL = string.Template("""
$headline_summary
$live_summary
$gone_summary
$dead_summary
$failed_summary
$failure_detail
$passed_summary
""".strip())
###############################################################################


###############################################################################
HEADLINE_TMPL = string.Template("""
Build summary [$bldday] for $tag

Build Set Began: $set_began
Build Set Ended: $set_ended
Report generated at $time on $summhost

Break The Build Policy and Respinning Information:
Refer to     : $twiki_url/BreakTheBuild
Reply to     : hnd-build-list.pdl@broadcom.com
GUB docs     : $twiki_url/GUB
""")
###############################################################################


###############################################################################
ERROR_TMPL = string.Template("""
----------------------------------------
$n) $bname
[Host: $bldhost; Start: $started; End: $ended]
PATH:   $path
RESPIN: $respin
----------------------------------------
""")
###############################################################################


###############################################################################
FAILED_TMPL = string.Template("""
$n. $bname ($ended): FAILED
LOG       : ${http_base}$log
ERRORS    : $error_log
CONTENTS  : $contents
$blames

""")
###############################################################################


###############################################################################
PASSED_TMPL = string.Template("""
$bname ($elapsed elapsed at $ended): PASSED
LOG     : ${http_base}$log
CONTENTS: $contents

""")
###############################################################################


###############################################################################
GONE_TMPL = string.Template("""
$bname: NEVER SHOWED UP
LOG   : None
LSF LOG: None

""")
###############################################################################


###############################################################################
DEAD_TMPL = string.Template("""
$bname: DIED BEFORE COMPLETING
LOG   : ${http_base}$log
LSF LOG: $lsflog

""")
###############################################################################


###############################################################################
LIVE_TMPL = string.Template("""
$bname: RUNNING since $started on $bldhost
LOG   : ${http_base}$log

""")
###############################################################################


def failure_detail(fail_list):
    """Return a textual summary describing errors in detail."""
    summary = ''
    for n, bld in enumerate(fail_list):
        summary += ERROR_TMPL.substitute(TOKENS,
                                         n=n + 1,
                                         bname=bld.bldable,
                                         respin=bld.respin,
                                         bldhost=bld.hostname,
                                         path=bld.path,
                                         started=bld.started,
                                         ended=bld.ended)
        summary += open(bld.error_log, 'r').read()
    return summary


def failed_summary(fail_list):
    """Return a textual summary of failed builds."""
    summary = ''
    for n, bld in enumerate(fail_list):
        if os.path.exists(bld.error_log) and os.path.getsize(bld.error_log):
            errlog = lib.consts.HTTP_BASE + bld.error_log
        else:
            errlog = '(no matched error messages)'

        if os.path.exists(bld.contents) and os.path.getsize(bld.contents):
            contents = lib.consts.HTTP_BASE + bld.contents
        else:
            contents = '(none)'

        blstr = 'COMMITTERS: '
        for name in sorted(bld.blames):
            blstr += name + ','
            blstr += '\n' if len(blstr) - blstr.rfind('\n') > 120 else ' '
        summary += FAILED_TMPL.substitute(TOKENS,
                                          n=n + 1,
                                          blames=blstr.rstrip().rstrip(','),
                                          bname=bld.bldable,
                                          respin=bld.respin,
                                          ended=bld.ended,
                                          log=bld.log,
                                          contents=contents,
                                          error_log=errlog).lstrip()
    if summary:
        tmpl = 'FAILURE OVERVIEW [$failed_count]:\n'
        preface = string.Template(tmpl).substitute(TOKENS)
        for failed in TOKENS.get('failed_brands', '').split():
            preface += '    %s\n' % failed
        preface = '\n' + HEADFMT % 'FAILED'
        summary = preface + summary
    return summary


def passed_summary(pass_list):
    """Return a textual summary of passed builds."""
    summary = ''
    for bld in pass_list:
        if os.path.exists(bld.contents) and os.path.getsize(bld.contents):
            contents = lib.consts.HTTP_BASE + bld.contents
        else:
            contents = '(none)'

        summary += PASSED_TMPL.substitute(TOKENS,
                                          bname=bld.bldable,
                                          elapsed=bld.elapsed,
                                          ended=bld.ended,
                                          log=bld.log,
                                          contents=contents).lstrip()
    if summary:
        preface = HEADFMT % 'PASSED'
        summary = preface + summary
    return summary


def gone_summary(gone_list):
    """Return a textual summary of gone (missing) builds."""
    summary = ''
    for bldable in gone_list:
        summary += GONE_TMPL.substitute(TOKENS, bname=bldable)

    if summary:
        preface = '== MISSING ====================================\n'
        summary = preface + summary
    return summary


def dead_summary(dead_list):
    """Return a textual summary of dead builds."""
    summary = ''
    for bld in dead_list:
        if bld.lsflog:
            lsflog = '/'.join([TOKENS['http_base'], bld.lsflog])
        else:
            lsflog = None
        summary += DEAD_TMPL.substitute(TOKENS,
                                        bname=bld.bldable,
                                        log=bld.log,
                                        lsflog=lsflog)
    if summary:
        preface = HEADFMT % 'DEAD'
        summary = preface + summary
    return summary


def live_summary(live_list):
    """Return a textual summary of live builds."""
    summary = ''
    for bld in live_list:
        summary += LIVE_TMPL.substitute(TOKENS,
                                        bldhost=bld.hostname,
                                        bname=bld.bldable,
                                        log=bld.log,
                                        started=bld.started)
    if summary:
        preface = HEADFMT % 'LIVE'
        summary = preface + summary
    return summary


def summarize(cfgroot, opts, tag, bldday):
    """Generate a summary for the given tag."""
    results, gone_set = lib.results.find_results(cfgroot, tag,
                                                 bldday=bldday,
                                                 bldname=opts.buildable,
                                                 findall=opts.all,
                                                 reftime=opts.reftime)

    if opts.follow or opts.logfiles:
        if results:
            results = sorted(results, key=lambda o: o.start_time)
            if opts.follow > 1:
                cmd = ['tail', '-f', results[-1].log]
                lib.util.execute(cmd)
            else:
                live = [r for r in results if r.live]
                if opts.follow and len(live) == 1:
                    cmd = ['tail', '-f', live[-1].log]
                    lib.util.execute(cmd)
                else:
                    if opts.logfiles > 1:
                        print('Live logs for %s [%s]:' % (tag, bldday))
                        for result in live:
                            print(result.log)
                    else:
                        print('Available logs for %s [%s]:' % (tag, bldday))
                        for result in results:
                            print(result.log)
        return None, None

    match = re.search(r'_([\d_]*)$', tag)
    if match:
        subj = '%s (%s)' % (match.group(1).replace('_', '.'), tag)
    else:
        subj = tag

    if not results:
        return '%s not built today' % subj, []

    # Find the first build of this set to start and the last to finish.
    hare = min(results, key=lambda obj: obj.start_time)
    tortoise = max(results, key=lambda obj: obj.end_time)

    # Work out exactly what passed, what failed, and what died.
    dead_set = set((r for r in results if r.dead))
    live_set = set((r for r in results if r.live))
    fail_set = set((r for r in results if r.failed))
    pass_set = set((r for r in results if r.passed))

    # Convert the sets into sorted lists.
    def by_brand(obj):
        """Sort buildables by name."""
        return obj.bldable

    dead_list = sorted(dead_set, key=by_brand)
    live_list = sorted(live_set, key=by_brand)
    fail_list = sorted(fail_set, key=by_brand)
    pass_list = sorted(pass_set, key=by_brand)
    # These are different - not objects, just strings
    gone_list = sorted(gone_set)

    # Collect reported checkins related to failed builds.
    blames = set()
    for fl in fail_list:
        blames |= fl.blames
    committers = sorted(blames)

    failed_brands = ' '.join(fl.bldable for fl in fail_list)
    TOKENS['bldday'] = bldday + '.*'
    if lib.times.is_offset(opts.reftime):
        TOKENS['bldday'] += ' %+d hours' % int(opts.reftime)
    TOKENS['time'] = time.strftime('%H:%M:%S')
    TOKENS['committers'] = ',\n\t'.join(committers)
    TOKENS['failed_brands'] = (failed_brands if failed_brands else ':-)')
    TOKENS['failed_count'] = len(fail_list)
    TOKENS['build_base'] = lib.consts.STATE.base
    TOKENS['http_base'] = lib.consts.HTTP_BASE
    TOKENS['set_began'] = hare.started
    TOKENS['set_ended'] = ('' if live_set else tortoise.ended)
    TOKENS['tag'] = tag

    sections = {}
    sections['headline_summary'] = HEADLINE_TMPL.substitute(
        TOKENS,
        summhost=lib.consts.HOSTNAME,
    ).lstrip()
    sections['failed_summary'] = failed_summary(fail_list)
    sections['failure_detail'] = failure_detail(fail_list)
    sections['gone_summary'] = gone_summary(gone_list)
    sections['dead_summary'] = dead_summary(dead_list)
    sections['live_summary'] = live_summary(live_list)
    sections['passed_summary'] = passed_summary(pass_list)
    summary = SUMMARY_TMPL.substitute(sections)
    summary = re.sub('\n{2,}', '\n\n', summary)
    print(summary)

    total = len(results) + len(gone_list)
    ratio = (len(pass_list) + (len(live_list) * .5)) / float(total)
    subj += ' {:.0%} '.format(ratio)
    if len(pass_list) == total:
        plural = ('s' if total > 1 else '')
        subj += '(%d build%s)' % (total, plural)
    else:
        subj += '('
        if fail_list:
            subj += '%d failed, ' % len(fail_list)
        if gone_list:
            subj += '%d missing, ' % len(gone_list)
        if dead_list:
            subj += '%d died, ' % len(dead_list)
        if live_list:
            subj += '%d live, ' % len(live_list)
        if pass_list:
            subj += '%d passed, ' % len(pass_list)
        if subj.endswith(', '):
            subj = subj[0:-2]
        subj += ')'
    subj += ' [%s]' % TOKENS['bldday']

    return subj, committers


def call(opts, cfgproxy):
    """Standard subcommand entry point."""
    cfgroot = cfgproxy.parse()

    if opts.follow or opts.logfiles:
        opts.localhost = 1
        opts.outfile = None

    # Allow an interactively-provided message.
    if opts.message == '-':
        prompt = 'Message (end with "."): '
        opts.message = ''
        while True:
            try:
                line = raw_input(prompt)
            except EOFError:
                break
            else:
                prompt = '> '
                if line == '.':
                    break
                else:
                    opts.message += line + '\n'

    # A user message should be URL-encoded for the command line.
    if opts.message:
        for i, word in enumerate(sys.argv):
            if word == '--message':
                sys.argv[i + 1] = lib.util.url_encode(opts.message)
                break

    # Immediately re-run ourselves under LSF unless asked not to.
    if not lib.lsf.submitted() and not lib.lsf.no_lsf(opts):
        foreground = opts.lsf_foreground or opts.outfile is not None
        sys.exit(lib.lsf.bsub(sys.argv,
                              app='sw_ondemand_builds',
                              foreground=foreground,
                              jobname='BUILD.SUMMARY',
                              queue=cfgroot.getvar('LSF_QUEUE'),
                              resource=cfgroot.getvar('LSF_RESOURCE')))

    # Set stdout to line buffering.
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)

    if not opts.bldday:
        now = datetime.datetime.now()
        opts.bldday = '%s.%s.%s' % (now.year, now.month, now.day)

    rc = 0

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

    # Results are sent by email if any of these conditions are true.
    # We have to be careful around opts.mailto because the current
    # user may be added to it automatically.
    emailing = opts.cc_blames or \
        (lib.lsf.submitted() and not opts.outfile) or \
        (opts.mailto and opts.mailto != [lib.consts.STATE.by])

    # Implement the usual convention that "-" means stdout.
    # Do not move this up - ordering is very tricky.
    if opts.outfile == '-':
        opts.outfile = None

    for tag in tags:
        if opts.outfile or emailing:
            f = tempfile.NamedTemporaryFile(
                mode='w',
                prefix=lib.consts.PROG + '.summary.',
                suffix='.tmp',
                delete=not lib.opts.DEBUG_MODE,
            )
            os.dup2(f.fileno(), sys.stdout.fileno())
            os.dup2(f.fileno(), sys.stderr.fileno())

        subject, committers = summarize(cfgroot, opts, tag, opts.bldday)

        if opts.outfile or emailing:
            f.flush()
            if emailing:
                bcc = (committers if opts.cc_blames else [])
                lib.mail.Msg(to=opts.mailto, reply_to=opts.mailto, bcc=bcc,
                             subject=subject,
                             body=lib.util.url_decode(opts.message),
                             files=[f.name], via=not lib.util.am_builder(),
                             filtertype='summary-%s' % tag,).send()
            if opts.outfile:
                shutil.copyfileobj(open(f.name, 'rb'),
                                   open(opts.outfile, 'ab'))

    if rc:
        sys.exit(2)

# vim: ts=8:sw=4:tw=80:et:
