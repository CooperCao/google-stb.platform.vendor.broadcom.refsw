#!/usr/bin/env python
""" Prints power difference between two regions """

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
    from ClmCompiler import                                     \
        ClmUtils, ClmData, ClmContainer, FilterParams, RateInfo, \
        RatesInfo, Band, Measure, CcRev, Dfs, ChannelType,        \
        ValidChannel, LocaleType, Bandwidth, SubChanId,            \
        _IGNORED_APPS_VERSION, BlobFormatVersionInfo
except:
    print >> sys.stderr, "ClmQueryDiff: Error: ClmCompiler.py not found. " + \
                         "Please place it to the same directory as this script"
    sys.exit(1)

# Help message
_usage = """ Usage
./ClmQueryDiff.py --ccrev cc/rev1 [--ccrev cc/rev2] [other options]
source1.xml [source2.xml]

--ccrev cc/rev      CC/rev of region to compare. Shall be specified once or
                    twice - to compare powers for same region (ostensibly
                    from two different CLM data sets) or of two different
                    regions (from same or different data sets) respectively
--regulatory        Print difference in regulatory powers. Default is print
                    differences in TX powers
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
--version           Prints CLM data version information: version of CLM data
                    and CLM data format, name and version of utility that
                    generated CLM file
--help              Prints this help
source1/2.xml       CLM XML file(s) with data to query. If two files specified
                    first data set is taken from first file, second - from
                    second
"""

# =============================================================================


class DataSet:
    """ One data set being compared

    Attributes:
    clm_file -- CLM XML file name
    ccrev    -- ClmCompiler.CcRev of region being compared
    clm_data -- ClmCompiler.ClmData obtained from CLM XML file
    """
    def __init__(self):
        self.clm_file = None
        self.ccrev = None
        self.clm_data = None

    def __getattr__(self, name):
        """ Shortcuts to retrieve various facts about regions being compared
        """
        if name == "region":
            return self.clm_data.get_region(self.ccrev)
        ClmUtils.error("Internal error: invalid attribute \"%s\"" % name)

    def get_reg_power(self, band, channel):
        """ Returns regulatory power for given channel

        Arguments:
        band    -- ClmCompiler.Band
        channel -- Channel number
        Returns Regulatory power in dbm (as double) or None
        """
        locale = self.region.get_locale(LocaleType(LocaleType.BASE, band))
        for p in locale.reg_power:
            if self.clm_data.valid_channels.is_channel_in_range(channel,
                                                                p.channel_range
                                                                ):
                return p.powers_dbm[0]
        return None

# =============================================================================


class Params:
    """ Command line parameters

    Attributes:
    data_sets  -- Two-element vector - one per region being compared
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
    pretty     -- ANSI prettyprint
    """

    ALL = "__ALL__"  # 'all' value of parameters

    def __init__(self):
        """ Constructor, initializes self to as if no parameters were specified
        """
        self.data_sets = [DataSet(), DataSet()]
        self._regulatory = False
        self._channel = None
        self._band = None
        self._bandwidth = None
        self._rate = None
        self._sub_chan = Params.ALL
        self._chains = None
        self._version = False
        self._diff_only = False
        self._pretty = False

    def set_param(self, param, value):
        """ Sets value for given parameter

        Arguments:
        param -- Parameter name
        value -- Parameter value
        """
        member_name = "_" + param
        if not hasattr(self, member_name):
            raise KeyError("Wrong parameter name %s" % param)
        member = getattr(self, member_name)
        if (member is None) or (type(member) == bool) or \
           (type(member) == int) or (param == "sub_chan"):
            setattr(self, member_name, value)
        else:
            raise ValueError("Parameter already set")

    def add_list_value(self, value):
        """ Append given value to list of values to list """
        self._list_values.append(value)

    def is_match(self, param, value):
        """ True if given value matches given query parameter """
        member = getattr(self, param)
        if (member is None) or (member == Params.ALL):
            return True
        if isinstance(member, (list, tuple)):
            return value in member
        return value == member

    def __getattr__(self, name):
        """ Returns value of given attribute by attribute name """
        if name in self.__dict__:
            return self.__dict__[name]
        if (name == "sub_chan") and (self._sub_chan is None):
            return [None]
        if ("_" + name) in self.__dict__:
            return self.__dict__["_" + name]
        else:
            raise AttributeError("Attribute \"%s\" not defined" % name)


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
             "help"])
    except getopt.GetoptError:
        print _usage
        ClmUtils.error(ClmUtils.exception_msg())

    params = Params()
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
                ccrev = CcRev(arg)
                if not ccrev.is_singular():
                    raise ValueError(
                        ("%s is multivalue CCrev. Only single-value CCrevs " +
                         "may be used")
                        % arg)
                if params.data_sets[1].ccrev is None:
                    params.data_sets[0 if params.data_sets[0].ccrev is None
                                     else 1].ccrev = ccrev
                else:
                    raise ValueError(
                        "%s parameter may be specified not more than twice"
                        % arg)
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
            elif opt in ("-?", "-h", "--help"):
                print _usage
                sys.exit(0)
            else:
                ClmUtils.error("Internal error: unsupported switch %s" % opt)
        except (SystemExit, KeyboardInterrupt):
            raise
        except:
            ClmUtils.error("\"%s\" option invalid: %s"
                           % (opt, ClmUtils.exception_msg()))
    if 1 <= len(args) <= 2:
        try:
            params.data_sets[0].clm_file = args[0]
            params.data_sets[0].clm_data = \
                ClmContainer(args[0]).fetch(
                    FilterParams.filter_all(), False)
            params.data_sets[1].clm_file = args[-1]
            params.data_sets[1].clm_data = \
                params.data_sets[0].clm_data if \
                params.data_sets[1].clm_file == params.data_sets[0].clm_file \
                else ClmContainer(args[-1]).fetch(
                    FilterParams.filter_all(), False)
        except (SystemExit, KeyboardInterrupt):
            raise
        except:
            ClmUtils.error(ClmUtils.exception_msg())
    else:
        ClmUtils.error("One or two CLM XML files shall be specified")
    if params.data_sets[0].ccrev is None:
        ClmUtils.error("At least one CCrev shall be specified")
    elif params.data_sets[1].ccrev is None:
        params.data_sets[1].ccrev = params.data_sets[0].ccrev
    for data_set in params.data_sets:
        if data_set.region is None:
            ClmUtils.error("\"%s\" doesn't contain region %s"
                           % (data_set.clm_file, str(data_set.ccrev)))
    return params

# =============================================================================


def pretty_print(params, different, s):
    """ Optionally prettyprints given line

    Arguments:
    params    -- Script parameters
    different -- True if line contains difference
    s         -- Line to print
    """
    prefix = ""
    suffix = ""
    hdl = None
    if params.pretty and different:
        if platform.system() == 'Windows':
            # -11 is STD_OUTPUT_HANDLE from winbase.h
            hdl = ctypes.windll.kernel32.GetStdHandle(-11)
            ctypes.windll.kernel32.SetConsoleTextAttribute(hdl, 15)
        else:
            prefix = "\x1b[1m"
            suffix = "\x1b[0m"
    print "%s%s%s" % (prefix, s, suffix)
    if params.pretty and different:
        if platform.system() == 'Windows':
            ctypes.windll.kernel32.SetConsoleTextAttribute(hdl, 7)

# =============================================================================


def print_reg_power_diff(params, band, channel):
    """ Prints regulatory power difference for given channel

    Arguments:
    params  -- Script parameters
    band    -- ClmCompiler.Band
    channel -- Channel number
    """
    # p0, p1 - regulatory power in dBm or None
    p0 = params.data_sets[0].get_reg_power(band, channel)
    p1 = params.data_sets[1].get_reg_power(band, channel)
    different = p0 != p1
    if params.diff_only and not different:
        return
    pretty_print(
        params, different,
        "%-4s %7d %6s %6s %6s"
        % (Band.name[band] + "G", channel,
           "-" if p0 is None else ("%.1f" % p0),
           "-" if p1 is None else ("%.1f" % p1),
           "-" if (p0 is None) or (p1 is None) else ("%.1f" % (p1 - p0))))

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


def print_tx_power_diff(params, ct, channel, sub_chan_id, rate,
                        measure_power_dicts):
    """ Prints TX power difference for given (sub)channel and rate

    Arguments:
    params              -- Script parameters
    ct                  -- ClmCompiler.ChannelType
    channel             -- Channel number
    sub_chan_id         -- ClmCompiler.SubChanId
    rate                -- ClmCompiler.RateInfo
    measure_power_dicts -- Per-data-set vector of dictionaries. Keys are
                           ClmCompiler.Measure objects, values are
                           ClmCompiler.Locale.Power objects
    """
    different = False  # True if powers in first and second source different
    difference = ""    # What shall be printed in 'Diff' column
    if set(measure_power_dicts[0].keys()) != \
       set(measure_power_dicts[1].keys()):
        # Different measures: "-" if on power target is absent, "E/C" if both
        # power targets present but have different measurements
        different = True
        difference = \
            "E/C" if measure_power_dicts[0] and measure_power_dicts[1] else "-"
    else:
        # Same measures

        # Dictionary that maps measures to dBm lists for power difference
        # (similar to measure_power_dicts argument)
        measure_power_diff_dict = {}
        # Loop for all measurements in source data. most likely there will be
        # just one measurement.
        for measure in measure_power_dicts[0].keys():
            # DBm vectors for each power target
            powers = [mpd[measure].powers_dbm for mpd in measure_power_dicts]
            if len(powers[0]) != len(powers[1]):
                # Special cases
                different = True
                if (len(powers[0]) == 0) or (len(powers[1]) == 0) or \
                   ((len(powers[0]) > 1) and (len(powers[1]) > 1)):
                    # One power target disabled or one is 3-antenna, another
                    # is 2-antenna: "-" will be printed
                    difference = "-"
                    break
                else:
                    # One is 1-antenna, another is multiantenna: 1-antenna
                    # extended to multiantenna
                    for i in range(len(powers)):
                        if len(powers[i]) == 1:
                            powers[i] = [powers[i][0]] * len(powers[1 - i])
            # Computing power difference for current measure
            measure_power_diff_dict[measure] = []
            for i in range(len(powers[0])):
                measure_power_diff_dict[measure].append(
                    powers[1][i] - powers[0][i])
                if measure_power_diff_dict[measure][-1] != 0:
                    different = True
        if difference == "":
            difference = format_tx_power(measure_power_diff_dict)
    if params.diff_only and not different:
        return
    sub_chan_bandwidth = \
        SubChanId.sub_chan_bandwidth(ct.bandwidth, sub_chan_id) \
        if sub_chan_id is not None else ct.bandwidth
    pretty_print(params, different,
                 "%-4s %8sM %4d%-3s %-17s %12s %12s %12s" %
                 (Band.name[ct.band] + "G", Bandwidth.name[ct.bandwidth],
                  channel,
                  "" if sub_chan_id is None else SubChanId.name[sub_chan_id],
                  (rate.vht_name if (sub_chan_bandwidth >= Bandwidth._80) and
                   hasattr(rate, "vht_name") else rate.name).replace("_SPEXP",
                                                                     "_SP"),
                  format_tx_power(dict([(kvp[0], kvp[1].powers_dbm)
                                        for kvp in
                                        measure_power_dicts[0].items()])),
                  format_tx_power(dict([(kvp[0], kvp[1].powers_dbm)
                                        for kvp in
                                        measure_power_dicts[1].items()])),
                  difference))

# =============================================================================


def main(argv):
    """ Main function

    Arguments:
    argv -- Command line arguments without script name
    """
    try:
        ClmUtils.error_source = "ClmQueryDiff: "
        print "ClmQueryDiff - prints power difference between two regions"
        params = parse_args(argv)
        channels_dict = {}  # Maps channel types to lists of channels
        # Printing versions, filling in channels_dict
        for ds_idx in range(len(params.data_sets)):
            data_set = params.data_sets[ds_idx]
            cd = data_set.clm_data.get_region_channels_dict(data_set.region)
            for ct, channels in cd.items():
                channels_dict[ct] = \
                    sorted(set(channels) | set(channels_dict.get(ct, [])))
            if params.version:
                print "Power%d. File: \"%s\". CLM Version: %s%s. Region: %s" \
                      % (ds_idx + 1, data_set.clm_file,
                         data_set.clm_data.clm_version
                         if data_set.clm_data.clm_version else "Unknown",
                         (" (Customization version %s)"
                          % data_set.clm_data.apps_version)
                         if data_set.clm_data.apps_version
                         not in (_IGNORED_APPS_VERSION, None) else "",
                         str(data_set.ccrev))
        if params.regulatory:
            # Printing regulatory power difference
            print "Band Channel Power1 Power2   Diff"
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
                    print_reg_power_diff(params, ct.band, channel)
        else:
            # Printing TX power difference
            print "Band Bandwidth Channel       Rate              " + \
                  "Power1       Power2         Diff"
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
                        for data_set in params.data_sets:
                            rate_power_dicts.append(
                                data_set.clm_data.get_rate_power_dict(
                                    data_set.region,
                                    channel,
                                    ct,
                                    True,
                                    sub_chan_id=sub_chan_id))
                            rates_set.update(rate_power_dicts[-1].keys())
                        # Loop over rates
                        for rate in sorted(rates_set,
                                           key=methodcaller("sort_key")):
                            if not params.is_match("rate", rate):
                                continue
                            if not params.is_match("chains", rate.chains):
                                continue
                            print_tx_power_diff(
                                params, ct, channel, sub_chan_id, rate,
                                [rpd.get(rate, (None, {}))[1]
                                 for rpd in rate_power_dicts])
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
