#!/usr/bin/env python
""" Prints minimum/maximum power in group of regions """

import os
import sys
import getopt
import re
import platform
import ctypes
from operator import attrgetter, itemgetter, methodcaller
sys.path.append(os.path.dirname(os.path.realpath(__file__)) +
                "/../ClmCompiler")
try:
    from ClmCompiler import                                      \
        ClmUtils, ClmContainer, FilterParams, RateInfo, RatesInfo,\
        Band, Measure, CcRev, ChannelType, LocaleType, Bandwidth,  \
        SubChanId, _IGNORED_APPS_VERSION, Locale
except:
    print >> sys.stderr, "ClmQueryMinMax: Error: ClmCompiler.py not " + \
                         "found. Please place it to the same directory as " + \
                         "this script"
    sys.exit(1)

# Help message
_usage = """ Usage
./ClmQueryMinMax.py --ccrev "ccrevs" [other options] source.xml

--ccrev "ccrevs"    Quoted, space-separated list of regions to compare
--regulatory        Print min/max of regulatory powers. Default is print
                    min/max of TX powers
--channel channel   Channel number, channel set in quotes (like "1 2 3") or
                    'all' (without quotes). In two latter cases list is
                    printed. Default is 'all'
--band band         5, 2.4 or all. If concrete channel(s) specified this
                    parameter is optional (disambiguates channel numbers valid
                    for both bands). If channel is 'all' this shortens printed
                    list. Default is 'all'
--bandwidth mhz     Channel bandwidth in MHz (20, 40, 80, 160), bandwidth set
                    in quotes (like "20 80") or 'all' (without quotes). This
                    parameter is optional - it disambiguates channel numbers
                    valid for several bandwidths or narrows printed list.
                    Default is 'all'
--sub_chan subchan  Subchannel ID (FULL, L, U, LL, LU, UL, UU), set of
                    subchannel IDs in quotes (like "L UU") or 'all' (without
                    quotes). Channel and bandwidth parameters are specified
                    for full channel, not for subchannel (e.g.
                    '--channel 42 --sub_chan LL' is correct, but
                    '--channel 36 --sub_chan LL' is incorrect). Default is
                    'all'.
--rate rate         Transmission rate, transmission rate group (like 'OFDM'),
                    set of rates/groups in quotes (like "MCS2 DSSS") or 'all'
                    (without quotes). If more than one rate/channel specified
                    then list is printed.
                    General format of rate name is like this:
                    DSSS<number>[_MULTI{1|2}] or OFDM<number>[_CDD{1|2}] or
                    MCS<number>[_CDD{1|2}][_STBC][_SPEXP{1|2}] or
                    VHT{8|9}SS{1|2|3}[_CDD{1|2}][_STBC][_SPEXP{1|2}]
                    Rate group names are DSSS[_MULTI{1|2}], OFDM[_CDD{1|2}],
                    MCS{0-7|8-15|16-23}[{_CDD|_STBC}][_SPEXP]{1|2}],
                    VHT{8-9}SS{1|2|3}[_CDD{1|2}][_STBC][_SPEXP{1|2}]
                    Default is 'all'
--chains chains     Number of TX rate's post-expansion TX chains (1, 2, 3) or
                    list in quotes (like "2 3") or 'all' (without quotes).
                    This is an alternative method to specify group of
                    transmission rates. Default is 'all'
--diff_only         Prints only lines where power is different
--pretty            Emphasize lines with differences with bold (using ANSI
                    escape sequences)
--annotate          Prints information about regions in which minimum and
                    maximum was reached
--version           Prints CLM data version information: version of CLM data
                    and CLM data format, name and version of utility that
                    generated CLM file
--help              Prints this help
source.xml          CLM XML file(s) with data to query
"""

# =============================================================================


class Params:
    """ Command line parameters

    Attributes:
    clm_file   -- CLM XML file
    clm_data   -- CLM data containing given regions
    regions    -- Regions to compute min/max for
    regulatory -- True to print differences for regulatory powers
    channel    -- Channel list specified as --channel parameter or None
    band       -- Band object specified as --band parameter or None
    bandwidth  -- Bandwidth list specified as --bandwidth parameter or None
    chains     -- Chain counts list specified as --chains parameter or None
    rate       -- RateInfo objects list specified as --rate parameter or None
    sub_chan   -- Subchannel values list, specified as --subchan
                  parameter. Value of 'None' in this list means main (full)
                  channel
    version    -- True for printing of version information
    diff_only  -- Print only rates with differences
    annotate   -- True to print region names on which minimum/maximum was
                  achieved
    pretty     -- ANSI prettyprint
    """

    ALL = "__ALL__"  # 'all' value of parameters

    def __init__(self):
        """ Constructor, initializes self to as if no parameters were specified
        """
        self.clm_file = None
        self.clm_data = None
        self.regions = []
        self.regulatory = False
        self.channel = None
        self.band = None
        self.bandwidth = None
        self.rate = None
        self.sub_chan = Params.ALL
        self.chains = None
        self.version = False
        self.diff_only = False
        self.pretty = False
        self.annotate = False

    def set_param(self, member_name, value):
        """ Sets value for given parameter

        Arguments:
        member_name -- Parameter name
        value       -- Parameter value
        """
        if not hasattr(self, member_name):
            raise KeyError("Wrong parameter name %s" % member_name)
        member = getattr(self, member_name)
        if (member is None) or (type(member) == bool) or \
           (type(member) == int) or \
           ((member_name == "sub_chan") and (member == Params.ALL)):
            setattr(self, member_name, value)
        else:
            raise ValueError("Parameter already set")

    def is_match(self, param, value):
        """ True if given value matches given query parameter """
        member = getattr(self, param)
        if (member is None) or (member == Params.ALL):
            return True
        if isinstance(member, (list, tuple)):
            return value in member
        return value == member


# =============================================================================

def error(errmsg):
    """ Prints given error message and exits """
    print >> sys.stderr, errmsg
    sys.exit(1)

# =============================================================================


def parse_args(argv):
    """ Retrieves command line arguments from given argument list (without
    first (file name) parameter. Returns Options object
    """
    if not argv:
        print _usage
        sys.exit(0)
    try:
        opts, args = getopt.getopt(
            argv, "h?",
            ["channel=", "band=", "bandwidth=", "rate=", "chains=", "ccrev=",
             "sub_chan=", "diff_only", "regulatory", "pretty", "version",
             "annotate", "help"])
    except getopt.GetoptError:
        print _usage
        ClmUtils.error(ClmUtils.exception_msg())

    params = Params()
    ccrevs = []
    for opt, arg in opts:
        try:
            if opt == "--channel":
                if arg == "all":
                    params.set_param("channel", Params.ALL)
                else:
                    channels = []
                    for channel in re.findall(r"\d+", arg):
                        channels.append(int(channel))
                    params.set_param("channel", channels)
                    if len(channels) == 0:
                        raise ValueError("Invalid channel(s) specification")
            elif opt == "--ccrev":
                for ccrev_str in arg.replace(',', ' ').split():
                    ccrev = CcRev(ccrev_str)
                    if not ccrev.is_singular():
                        raise ValueError(
                            ("%s is multivalue CCrev. Only single-value " +
                             "CCrevs may be used")
                            % ccrev_str)
                    if ccrev in ccrevs:
                        raise ValueError(
                            "%s specified more than once" % ccrev_str)
                    ccrevs.append(ccrev)
            elif opt == "--regulatory":
                params.set_param("regulatory", True)
            elif opt == "--band":
                if arg == "all":
                    params.set_param("band", Params.ALL)
                elif arg in Band.name.values():
                    params.set_param("band", Band.parse(arg))
                else:
                    raise ValueError("Invalid band specification")
            elif opt == "--bandwidth":
                if arg == "all":
                    params.set_param("bandwidth", Params.ALL)
                else:
                    bandwidths = []
                    for rate_name in re.findall(r"\d+", arg):
                        bandwidths.append(Bandwidth.parse(rate_name))
                    if len(bandwidths) == 0:
                        raise ValueError("Invalid bandwidth specification")
                    params.set_param("bandwidth", bandwidths)
            elif opt == "--sub_chan":
                if arg == "all":
                    params.set_param("sub_chan", Params.ALL)
                else:
                    sub_chan_names = arg.replace(',', ' ').split()
                    if not sub_chan_names:
                        raise ValueError("No valid subchannels specified")
                    sub_chans = []
                    for sub_chan_name in sub_chan_names:
                        if sub_chan_name.lower() == "full":
                            sub_chans.append(None)
                        else:
                            sub_chans.append(SubChanId.parse(sub_chan_name))
                    params.set_param("sub_chan", sub_chans)
            elif opt == "--chains":
                if arg == "all":
                    params.set_param("chains", Params.ALL)
                else:
                    chains = []
                    for chains_str in re.findall(r"\d+", arg):
                        c = int(chains_str)
                        if (c < 1) or (c > RatesInfo.get_max_chains()):
                            raise ValueError(
                                ("Invalid number of chains: \"%s\". Shall " +
                                 "be between 1 and %d")
                                % (chains_str, RatesInfo.get_max_chains()))
                        chains.append(c)
                    if len(chains) == 0:
                        raise ValueError("Invalid chains specification")
                    params.set_param("chains", chains)
            elif opt == "--rate":
                if arg == "all":
                    params.set_param("rate", Params.ALL)
                else:
                    rate_infos = []
                    for rate_name in re.findall(r"[A-Z0-9_\-]+", arg.upper()):
                        rate_infos += RatesInfo.get_expanded_rate(rate_name)
                    if len(rate_infos) == 0:
                        raise ValueError("Invalid rate specification")
                    params.set_param("rate", rate_infos)
            elif opt == "--version":
                params.set_param("version", True)
            elif opt == "--pretty":
                if not (hasattr(sys.stdout, "isatty") and sys.stdout.isatty()):
                    raise ValueError(("\"%s\" may be specified only for " +
                                      "terminal output (it can't be " +
                                      "specified for file output)") % opt)
                params.set_param("pretty", True)
            elif opt == "--diff_only":
                params.set_param("diff_only", True)
            elif opt == "--annotate":
                params.set_param("annotate", True)
            elif opt in ("-?", "-h", "--help"):
                print _usage
                sys.exit(0)
            else:
                ClmUtils.error("Internal error: unsupported switch %s" % opt)
        except (SystemExit, KeyboardInterrupt, SyntaxError):
            raise
        except:
            ClmUtils.error("\"%s\" option invalid: %s"
                           % (opt, ClmUtils.exception_msg()))
    if len(args) != 1:
        ClmUtils.error("One CLM XML file argument shall be specified")
    params.clm_file = args[0]
    params.clm_data = ClmContainer(params.clm_file).\
        fetch(FilterParams.filter_all(), False)
    if not ccrevs:
        ClmUtils.error("At least one CCrev shall be specified")
    for ccrev in ccrevs:
        params.regions.append(params.clm_data.get_region(ccrev))
        if params.regions[-1] is None:
            ClmUtils.error("%s doesn't contain region %s" % (params.clm_file,
                                                             str(ccrev)))
    return params

# =============================================================================


def pretty_print(params, bold, s):
    """ Optionally prettyprints given line

    Arguments:
    params  -- Script parameters
    bold    -- True if line shall be bolded
    s       -- Line to print
    """
    prefix = ""
    suffix = ""
    hdl = None
    if params.pretty and bold:
        if platform.system() == 'Windows':
            # -11 is STD_OUTPUT_HANDLE from winbase.h
            hdl = ctypes.windll.kernel32.GetStdHandle(-11)
            ctypes.windll.kernel32.SetConsoleTextAttribute(hdl, 15)
        else:
            prefix = "\x1b[1m"
            suffix = "\x1b[0m"
    print "%s%s%s" % (prefix, s, suffix)
    if params.pretty and bold:
        if platform.system() == 'Windows':
            ctypes.windll.kernel32.SetConsoleTextAttribute(hdl, 7)

# =============================================================================


def get_reg_power_info(params, band, channel):
    """ Returns None or list with first element boolean (True if line shall be
    print in bold), rest elements column contents for regulatory power output

    Arguments:
    params  -- Script parameters
    band    -- ClmCompiler.Band
    channel -- Channel number
    """
    min_power = 1000
    max_power = None
    min_ccrevs = []
    max_ccrevs = []
    for rd in params.regions:
        locale = rd.get_locale(LocaleType(LocaleType.BASE, band))
        for p in locale.reg_power:
            if params.clm_data.valid_channels.is_channel_in_range(
                    channel, p.channel_range):
                reg_power = p.powers_dbm[0]
                break
        else:
            reg_power = None
        if ((reg_power is None) and (min_power is not None)) or \
                ((reg_power is not None) and (min_power is not None) and
                 (reg_power < min_power)):
            min_power = reg_power
            min_ccrevs = [rd.ccrev]
        elif reg_power == min_power:
            min_ccrevs.append(rd.ccrev)
        if (max_power is None) or (reg_power > max_power):
            max_power = reg_power
            max_ccrevs = [rd.ccrev]
        elif reg_power == max_power:
            max_ccrevs.append(rd.ccrev)
    has_differences = (min_power != max_power)
    if params.diff_only and not has_differences:
        return None
    return \
        [has_differences, Band.name[band] + "G", str(channel),
         "  -" if min_power is None else ("  %.1f" % min_power)] + \
        ([", ".join(str(ccrev) for ccrev in min_ccrevs)]
         if params.annotate else []) + \
        ["  -" if max_power is None else ("  %.1f" % max_power)] + \
        ([", ".join(str(ccrev) for ccrev in max_ccrevs)]
         if params.annotate else [])

# =============================================================================


def format_tx_power(measure_power_dict):
    """ Returns string representation of given TX power

    Arguments:
    measure_power_dicts -- Dictionary that represents TX power. Keys are
                           ClmCompiler.Measure objects, values are
                           lists of dBm powers (empty for disabled,
                           longer than one for per antenna)
    """
    if not measure_power_dict:
        return "-"
    ret = ""
    for measure, powers in measure_power_dict.items():
        if ret:
            ret += "&"
        if len(powers) == 0:
            ret += "disabled"
            if len(measure_power_dict) > 1:
                ret += Measure.name[measure][0]
        else:
            ret += "/".join(["%.2f" % power for power in powers]) + \
                Measure.name[measure][0]
    return ret

# =============================================================================


def get_tx_power_info(params, ct, channel, sub_chan_id, rate,
                      measure_power_dicts):
    """ Returns None or list with first element boolean (True if line shall be
    print in bold), rest elements column contents for TX power output

    Arguments:
    params              -- Script parameters
    ct                  -- ClmCompiler.ChannelType
    channel             -- Channel number
    sub_chan_id         -- ClmCompiler.SubChanId
    rate                -- ClmCompiler.RateInfo
    measure_power_dicts -- Per-region vector of dictionaries. Keys are
                           ClmCompiler.Measure objects, values are
                           ClmCompiler.Locale.Power objects
    """
    measure_mismatch = False
    min_mpd = None
    max_mpd = None
    min_ccrevs = []
    max_ccrevs = []
    for reg_idx in range(len(params.regions)):
        rd = params.regions[reg_idx]
        if reg_idx == 0:
            min_mpd = measure_power_dicts[0]
            max_mpd = measure_power_dicts[0]
            min_ccrevs = [rd.ccrev]
            max_ccrevs = [rd.ccrev]
            continue
        mpd = measure_power_dicts[reg_idx]
        if min_mpd.keys():
            if not mpd.keys():
                min_mpd = mpd
                min_ccrevs = [rd.ccrev]
            elif set(min_mpd.keys()) != set(mpd.keys()):
                measure_mismatch = True
                break
            else:
                m = {}
                same_as_min = True
                same_as_current = True
                for measure in min_mpd.keys():
                    m[measure] = Locale.Power.get_extreme_power(
                        min_mpd[measure], mpd[measure], get_min=True)
                    same_as_min &= m[measure].same_power(min_mpd[measure])
                    same_as_current &= m[measure].same_power(mpd[measure])
                if not same_as_min:
                    min_mpd = m
                    min_ccrevs = []
                if same_as_current:
                    min_ccrevs.append(rd.ccrev)
        else:
            if not mpd.keys():
                min_ccrevs.append(rd.ccrev)
        if max_mpd.keys():
            if mpd.keys():
                if set(max_mpd.keys()) != set(mpd.keys()):
                    measure_mismatch = True
                    break
                m = {}
                same_as_max = True
                same_as_current = True
                for measure in max_mpd.keys():
                    m[measure] = Locale.Power.get_extreme_power(
                        max_mpd[measure], mpd[measure], get_min=False)
                    same_as_max &= m[measure].same_power(max_mpd[measure])
                    same_as_current &= m[measure].same_power(mpd[measure])
                if not same_as_max:
                    max_mpd = m
                    max_ccrevs = []
                if same_as_current:
                    max_ccrevs.append(rd.ccrev)
        else:
            if not mpd.keys():
                max_ccrevs.append(rd.ccrev)
            else:
                max_mpd = mpd
                max_ccrevs = [rd.ccrev]
    has_differences = len(params.regions) != len(min_ccrevs)
    if params.diff_only and not has_differences:
        return None
    sub_chan_bandwidth = \
        SubChanId.sub_chan_bandwidth(ct.bandwidth, sub_chan_id) \
        if sub_chan_id is not None else ct.bandwidth
    ret = [has_differences, Band.name[ct.band] + "G",
           Bandwidth.name[ct.bandwidth] + "M",
           ChannelType.channel_to_string(channel, ct) +
           ("%-3s" %
            ("" if sub_chan_id is None else SubChanId.name[sub_chan_id])),
           (rate.vht_name if (sub_chan_bandwidth >= Bandwidth._80) and
            hasattr(rate, "vht_name") else rate.name).replace("_SPEXP",
                                                              "_SP")]
    if measure_mismatch:
        ret += ["Conducted/EIRP mismatch", ""] + \
            (["", ""] if params.annotate else [])
    else:
        ret += \
            ["  " + format_tx_power(dict([(kvp[0], kvp[1].powers_dbm)
                                          for kvp in min_mpd.items()]))] + \
            ([", ".join(str(ccrev) for ccrev in min_ccrevs)]
             if params.annotate else []) + \
            ["  " + format_tx_power(dict([(kvp[0], kvp[1].powers_dbm)
                                          for kvp in max_mpd.items()]))] + \
            ([", ".join(str(ccrev) for ccrev in max_ccrevs)]
             if params.annotate else [])
    return ret

# =============================================================================


def print_result(params, rows, alignment):
    """ Prints resulting table

    Arguments
    params    -- Params object with program command line options
    rows      -- List of row data. Each element is a list with first element
                 boolean (True if row shall be printed in bold), other elements
                 are column contents)
    alignment -- String with per-column alignment data - 'L' is for left
                 alignment, R is for right alignment
    """
    col_width = [0 for i in range(len(rows[0]) - 1)]
    for row in rows:
        for i in range(len(col_width)):
            col_width[i] = max(col_width[i], len(row[i + 1]))
    format = ""
    for i in range(len(col_width)):
        format += "%s%%%s%ds" % (" " if i else "",
                                 "-" if alignment[i] == "L" else "",
                                 col_width[i])
    for row in rows:
        pretty_print(params, row[0], format % tuple(row[1:]))

# =============================================================================


def main(argv):
    """ Main function

    Arguments:
    argv -- Command line arguments without script name
    """
    try:
        ClmUtils.error_source = "ClmQueryMinMax: "
        print "ClmQueryMinMax - prints minimum/maximum power among regions"
        params = parse_args(argv)
        channels_dict = {}  # Maps channel types to lists of channels
        for rd in params.regions:
            cd = params.clm_data.get_region_channels_dict(rd)
            for ct, channels in \
                    params.clm_data.get_region_channels_dict(rd).iteritems():
                channels_dict[ct] = \
                    sorted(set(channels) | set(channels_dict.get(ct, [])))
        if params.version:
            print "File: \"%s\". CLM Version: %s%s." \
                  % (params.clm_file,
                     params.clm_data.clm_version
                     if params.clm_data.clm_version else "Unknown",
                     (" (Customization version %s)"
                      % params.clm_data.apps_version)
                     if params.clm_data.apps_version
                     not in (_IGNORED_APPS_VERSION, None) else "")
        rows = []
        if params.regulatory:
            # Printing regulatory power difference
            if params.annotate:
                rows.append(
                    [False, "Band", "Channel", "Min", "Min Regions",
                     "Max", "Max Regions"])
                alignment = "LRRLRL"
            else:
                rows.append([False, "Band", "Channel", "Min", "Max"])
                alignment = "LRRR"
            # Loop over bands
            for ct in ChannelType.all():
                if not ((ct.bandwidth == Bandwidth.default) and
                        (ct in channels_dict) and
                        params.is_match("band", ct.band)):
                    continue
                # Loop over channels
                for channel in channels_dict[ct]:
                    if not params.is_match("channel", channel):
                        continue
                    p = get_reg_power_info(params, ct.band, channel)
                    if p:
                        rows.append(p)
        else:
            # Printing TX power difference
            if params.annotate:
                rows.append(
                    [False, "Band", "Bandwidth", "Channel", "Rate",
                     "Min", "Min Regions", "Max", "Max Regions"])
                alignment = "LRRLRLRL"
            else:
                rows.append(
                    [False, "Band", "Bandwidth", "Channel", "Rate",
                     "Min", "Max"])
                alignment = "LRRLRR"
            # Loop over band, then bandwidth
            for ct in ChannelType.all():
                if not (params.is_match("bandwidth", ct.bandwidth) and
                        params.is_match("band", ct.band) and
                        (ct in channels_dict)):
                    continue
                # Loop over channels
                for channel in channels_dict[ct]:
                    if not params.is_match("channel", channel):
                        continue
                    # Loop over subchannels
                    for sub_chan_id in ([None] +
                                        SubChanId.valid_sub_chan_ids(
                                            ct.bandwidth)):
                        if not params.is_match("sub_chan", sub_chan_id):
                            continue
                        # Per data set vector of dictionaries, each dictionary
                        # maps measurements to dbm vectors
                        rate_power_dicts = []
                        # Set of rates for which TX powers defined
                        rates_set = set()
                        # Computing rate_power_dicts and rates_set
                        for rd in params.regions:
                            rate_power_dicts.append(
                                params.clm_data.get_rate_power_dict(
                                    rd, channel, ct, True,
                                    sub_chan_id=sub_chan_id))
                            rates_set.update(rate_power_dicts[-1].keys())
                        # Loop over rates
                        for rate in sorted(rates_set,
                                           key=methodcaller("sort_key")):
                            if not params.is_match("rate", rate):
                                continue
                            if not params.is_match("chains", rate.chains):
                                continue
                            p = get_tx_power_info(
                                params, ct, channel, sub_chan_id, rate,
                                [rpd.get(rate, (None, {}))[1]
                                 for rpd in rate_power_dicts])
                            if p:
                                rows.append(p)
        print_result(params, rows, alignment)
    except SystemExit as se:
        if se.code:
            if not isinstance(se.code, int) or \
               (str(se.code) != ClmUtils.exception_msg()):
                print >>sys.stderr, ClmUtils.exception_msg()
            return 1
    return 0

# =============================================================================


# Standalone module starter
if __name__ == "__main__":  # pragma: no cover
    ret = main(sys.argv[1:])
    if ret:
        sys.exit(ret)
