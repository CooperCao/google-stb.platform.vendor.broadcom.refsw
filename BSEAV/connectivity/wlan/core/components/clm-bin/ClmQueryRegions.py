#!/usr/bin/env python
""" Retrieves power targets for multiple regions at once """

import os
import sys
import getopt
import StringIO
import re
try:
    import ClmCompiler
except:
    sys.path.append(os.path.dirname(os.path.realpath(__file__)) +
                    "/../ClmCompiler")
    import ClmCompiler
try:
    import ClmQuery
except:
    sys.path.append(os.path.dirname(os.path.realpath(__file__)) +
                    "/../ClmQuery")
    import ClmQuery

# Help message
_usage = """ Usage
./ClmQueryRegions.py [options] source.xml [output_file]
Options are:
--ccrev cc/rev      Region(s) and/or aggregation(s) to include to BLOB. May be
                    specified multiple times, also may be specified as set of
                    space-separated values in quotes (like
                    `--ccrev "US/0 EU/9"`). Special values: `--ccrev all`
                    includes all  regions, `--ccrev all/0` includes all
                    default regions, `--ccrev CC/all` includes all regions of
                    country CC
--region cc/rev     Similar to --ccrev but only selects regions
--agg cc/rev        Similar to --ccrev but only selects aggregations
--include tag_group The tag group selects regions to include, and has the
                    form: [!]<name>[+...] where <name> selects regions that
                    are tagged with <name> whereas !<name> selects regions
                    that are not tagged with <name>. Multiple names can be
                    conjoined with '+'. All the subexpressions must be
                    true. Examples:
                    '--include OEM1' - include all regions tagged with OEM1
                    '--include OEM1+Mobility' - include all regions tagged
                      with OEM1 in Mobility,
                    '--include PC-OEM+!OEM2' - include all PC-OEM regions
                      not tagged with OEM2
--full_set          Include CC/0 regions for all countries except for those
                    included to all aggregations
--regulatory        True for regulatory power print, false for TX power print
--channel channel   Channel number, channel set in quotes (like "1 2 3") or
                    'all' (without quotes). In two latter cases list is
                    printed. U/l notation is allowed (e.g. 48u is 40MHz
                    channel 46).
--band band         5, 2.4 or all. If concrete channel(s) specified this
                    parameter is optional (disambiguates channel numbers valid
                    for both bands). If cahnnel is 'all' this narows printed
                    list
--bandwidth mhz     Channel bandwidth in MHz (20, 40, 80, 160), bandwidth set
                    in quotes (like "20 80") or 'all' (without quotes). This
                    parameter is optional - it disambiguates channel numbers
                    valid for several bandwidths or narrows printed list
--sub_chan subchan  Subchannel ID (FULL, L, U, LL, LU, UL, UU), set of
                    subchannel IDs in quotes (like "L UU") or all. Channel and
                    bandwidth parameters are specified for full channel, not
                    for subchannel (e.g. '--channel 42 --sub_chan LL' is
                    correct, but '--channel 36 --sub_chan LL' is incorrect).
                    Default is FULL.
                    Note that '--channel 36L' is full 40MHz channel 38,
                    '--channel 36 --subchan L' is invalid,
                    '--channel 38 --sub_chan L' is subchannel 36 (lower) of
                    40MHz 38
--rate rate         Transmission rate, transmission rate group (like 'OFDM'),
                    set of rates/groups in quotes (like "MCS2 DSSS") or 'all'
                    (without quotes). If more than one rate/channel specified -
                    list is printed.
                    General format of rate name is like this:
                    DSSS<number>[_MULTI{1|2}] or OFDM<number>[_CDD{1|2}] or
                    MCS<number>[_CDD{1|2}][_STBC][_SPEXP{1|2}] or
                    VHT{8|9}SS{1|2|3}[_CDD{1|2}][_STBC][_SPEXP{1|2}]
                    Rate group names are DSSS[_MULTI{1|2}], OFDM[_CDD{1|2}],
                    MCS{0-7|8-15|16-23}[{_CDD|_STBC}][_SPEXP]{1|2}],
                    VHT{8-9}SS{1|2|3}[_CDD{1|2}][_STBC][_SPEXP{1|2}]
--chains chains     Number of TX rate's post-expansion TX chains (1, 2, 3) or
                    list in quotes (like "2 3"). This is an alternative method
                    to specify group of transmission rates
--ant_gain dB       Apply (subtract) given antenna gain offset to all EIRP
                    limits
--sar2 dBm          Apply given SAR limit to 2.4GHz channel powers
--sar5 dBm          Apply given SAR limit to 5gHz channel powers
--version           Prints CLM data version information: version of CLM data
                    and CLM data format, name and version of utility that
                    generated CLM file
--max_lines num     Specifies maximum number of lines in output file (that
                    must be specified for this switch to be used). Creates
                    additional output files, if necessary. Split is always on
                    region boundary, so number of lines shall be greater than
                    required for one region
--help              Prints this help
source.xml          CLM XML file with data to query
output_file         Optional output file name. If not specified - result is
                    printed

If none of region selection swiches specified '--ccrev all' (all regions) is
assumed. If none of power target selection switches specified - all power
power targets of selected regions printed
"""

# =============================================================================


class Options:
    """ Parsed command line parameters

    Attributes:
    compiler_args -- List of command-line arguments that shall be passed to
                     ClmCompiler
    query_args    -- List of command-line arguments that shall be passed to
                     ClmQuery
    print_version -- True if version information shall be printed
    max_lines     -- Maximum number of lines in output file or None
    clm_file      -- CLM XML file name
    output_file   -- Output file name or None
    """
    def __init__(self):
        self.compiler_args = []
        self.query_args = []
        self.print_version = False
        self.max_lines = None
        self.clm_file = None
        self.output_file = None

# =============================================================================


def error(errmsg):
    """ Prints given error message and exits """
    print >> sys.stderr, errmsg
    sys.exit(1)

# =============================================================================


def parse_args():
    """ Parses command-line arguments and returns Options object """
    # ClmQuery options that select power targets to print
    query_selector_options = ["channel=", "band=", "bandwidth=", "rate=",
                              "chains=", "sub_chan="]
    # Other ClmQuery options
    query_other_options = ["regulatory", "ant_gain=", "sar2=", "sar5=",
                           "version"]
    # ClmCompiler options
    compiler_options = ["region=", "agg=", "ccrev=", "include=", "full_set"]
    if len(sys.argv[1:]) == 0:
        print _usage
        sys.exit(1)
    try:
        opts, args = getopt.getopt(sys.argv[1:], "h?",
                                   query_selector_options +
                                   query_other_options + compiler_options +
                                   ["version", "help", "max_lines="])
    except getopt.GetoptError:
        error(str(sys.exc_info()[1]))
    ret = Options()
    all_powers = True
    for opt, arg in opts:
        if opt in [("--" + o.replace("=", ""))
                   for o in query_selector_options]:
            all_powers = False
        if opt in [("--" + o.replace("=", ""))
                   for o in (query_selector_options + query_other_options)]:
            ret.query_args.append(opt)
            if arg:
                ret.query_args.append(arg)
        elif opt in [("--" + o.replace("=", "")) for o in compiler_options]:
            ret.compiler_args.append(opt)
            if arg:
                ret.compiler_args.append(arg)
        elif opt in ("-?", "-h", "--help"):
            print _usage
            sys.exit(0)
        elif opt == "--version":
            ret.print_version = True
        elif opt == "--max_lines":
            ret.max_lines = int(arg)
    if len(args) < 1:
        error("CLM XML file name not specified")
    if len(args) > 2:
        error("Extra command line arguments specified: %s" % ", ".join(args))
    if all_powers:
        ret.query_args += \
            "--channel all --rate all --band all --bandwidth all".split()
    ret.clm_file = args[0]
    if len(args) > 1:
        ret.output_file = args[1]
    if (ret.max_lines is not None) and (ret.output_file is None):
        error("--max_lines may only be set if output file name is specified")
    return ret

# =============================================================================


def open_output_file(file_name, file_num):
    """ Opens output file

    Arguments:
    file_name -- Name of first file in sequence
    file_num  -- 0-based file number in sequence
    Returns file object, opened for writing
    """
    if file_num > 0:
        parts = os.path.splitext(file_name)
        file_name = "%s_%03d%s" % (parts[0], file_num, parts[1])
    try:
        return open(file_name, "wt")
    except EnvironmentError:
        error("Can't open output file %s" % file_name)

# =============================================================================


if __name__ == "__main__":
    options = parse_args()
    if options.print_version:
        ClmQuery.main(["--version", options.clm_file])
    file_count = 0
    prev_stdout = sys.stdout
    output_f = open_output_file(options.output_file, file_count) \
        if options.output_file is not None else sys.stdout
    prev_stdout = sys.stdout
    try:
        sys.stdout = StringIO.StringIO()
        ClmCompiler.main(options.compiler_args + ["--list", "region",
                                                  options.clm_file])
        regions = re.findall(r"\s+([A-Z0-9#]{2}/\d+)\n", sys.stdout.getvalue())
        if len(regions) == 0:
            error("No matching regions found")
        query = ClmQuery.Query(options.clm_file)
        title_printed = options.print_version
        printed_lines = 0
        for region in regions:
            sys.stdout = StringIO.StringIO()
            query_params = \
                ClmQuery.parse_args(options.query_args + ["--region", region])
            query.query(query_params, True)
            lines = sys.stdout.getvalue().split("\n")
            num_lines = len(lines) + (0 if title_printed else 1)
            if (options.max_lines is not None) and \
                    (options.max_lines < (printed_lines + num_lines)):
                if options.max_lines < num_lines:
                    error(
                        ("--max_lines option value too small. Please " +
                         "specify at least %d") % num_lines)
                output_f.close()
                file_count += 1
                printed_lines = 0
                output_f = open_output_file(options.output_file, file_count)
            printed_lines += num_lines
            if not title_printed:
                print >>output_f, lines[0]
                title_printed = True
            del lines[0]
            print >>output_f, "Region %s" % region
            print >>output_f, "\n".join(lines)
    finally:
        if options.output_file:
            try:
                output_f.close()
            except EnvironmentError:
                pass
        sys.stdout = prev_stdout
