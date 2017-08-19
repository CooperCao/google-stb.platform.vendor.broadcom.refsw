#!/usr/bin/env python
""" Retrieves CLM information from CLM XML file """

import os
import sys
import getopt
import re
import copy
import commands
sys.path.append(os.path.dirname(sys.argv[0]) + r"\..\ClmCompiler")
try:
    from ClmCompiler import ClmUtils, ClmData, ClmContainer, FilterParams, \
                            Locale, Region, Aggregation, AdvertisedCountry, \
                            RateInfo, RatesInfo, Band, Measure, CcRev, \
                            ChanInfo, ChannelType, ValidChannel
except:
    print "ClmCompiler.py not found. Please place it to the same directory " \
          "as this script"
    sys.exit(1)


###############################################################################
# MODULE CONSTANTS
###############################################################################

_VERSION = "0.1"    # Program version


###############################################################################
# MODULE CLASSES
###############################################################################


class QueryParams:
    """ Lookup-related parameters specified in command line

    Attributes:
    region     -- CcRev object specified as --region parameter or None
    country    -- CC string specified as --country parameter or None
    band       -- Band object specified as --band parameter or None
    bandwidth  -- Channel bandwidth in MHz specified as --bandwidth parameter
                  or None
    rate       -- RateInfo object specified as --rate parameter or None
    regulatory -- True for regulatory power, false for TX power
    """

    def __init__(self):
        """ Constructor, initializes self to as if no parameters were specified
        """
        self._region = None
        self._channel = None
        self._band = None
        self._bandwidth = None
        self._rate = None
        self._regulatory = False

    def set_param(self, param, value):
        """ Sets value for given parameter

        Arguments:
        param -- Paramter name
        value -- Parameter value
        """
        member = "_" + param
        if member not in self.__dict__:
            raise KeyError("Wrong parameter name %s" % param)
        if (self.__dict__[member] is None) or (type(self.__dict__[member]) == bool):
            self.__dict__[member] = value
        else:
            raise ValueError("Parameter already set")

    def __getattr__(self, name):
        """ Returns value of given attribute by attribute name """
        return self.__dict__["_" + name]

#==============================================================================


class Query:
    """ Performs all required querying """

    def __init__(self, query_params, verbose):
        """ Constructor.

        Arguments:
        query_params  -- QueryParams object filled from command line data
        verbose       -- True for verbose output
        """
        self._verbose = verbose
        self._params = query_params

    def print_power(self):
        """ Prints power """
        region = self._params.region
        if region is None:
            ClmUtils.error("Region parameter not specified")
        channel = self._params.channel
        if channel is None:
            ClmUtils.error("Channel parameter not specified")

        result = commands.getoutput("wl country %s" % region)
        if result.find('Bad') != -1 or result.find('Could not') != -1:
            ClmUtils.error("Could not set the country/regrev to %s" % region)
        result = commands.getoutput("wl chanspec %s" % channel)
        if result.find('Bad Channel') != -1:
            ClmUtils.error("Could not set channel to %s" % channel)

        if self._params.regulatory:
            print commands.getoutput('wl dump txpwr_reg_max')
        else:
            band = self._params.band
            if band is None:
                ClmUtils.error("Band parameter not specified")
            bandwidth = self._params.bandwidth
            if bandwidth is None:
                ClmUtils.error("Bandwidth parameter not specified")
            rate = self._params.rate
            if rate is None:
                ClmUtils.error("Rate parameter not specified")

            rate_index = 100 # invalid index
            for i, rate_entry in enumerate(RatesInfo._RATES):
                if rate_entry[0] == rate:
                    rate_index = i
            if rate_index == None:
                ClmUtils.error("Invalid rate parameter %s" % rate)

            #print "rate index = %s"%rate_index

            if band == 1: # 2.4GHz
                if bandwidth == 40: # 40MHz
                    dump_command = "clm_limits_2G_40M"
                else: # 20MHz
                    if channel.endswith('u') or channel.endswith('l'):
                        dump_command = "clm_limits_2G_20in40M"
                    else: # Normal 20MHz channel
                        dump_command = "clm_limits_2G_20M"
            else: # 5GHz
                if bandwidth == 40: # 40MHz
                    dump_command = "clm_limits_5G_40M"
                else: # 20MHz
                    if channel.endswith('u') or channel.endswith('l'):
                        dump_command = "clm_limits_5G_20in40M"
                    else: # Normal 20MHz channel
                        dump_command = "clm_limits_5G_20M"

            #print dump_command
            pipe = os.popen("wl dump %s" % dump_command)

            lines = pipe.readlines()
            print "%s dBm" % lines[rate_index].rstrip()

            status = pipe.close()
            if status:
                ClmUtils.error("Failed to read wl chanspecs")

    def _vp(self, s):
        """ Prints given string if verbose printing is enabled """
        if self._verbose:
            sys.stdout.write(s)


###############################################################################
# MODULE STATIC VARIABLES
###############################################################################

_query = None
_list_values = []

# Help message
_usage = """Usage:
./ClmDriverQuery.py --channel channel --band band --bandwidth mhz --rate rate
--region cc/rev --regulatory --verbose --help

--channel channel Channel number
--band band       5 or 2.4. This switch might be necessary if channel number
                  is ambiguous (valid for both 2.4 and 5 GHz bands)
--bandwidth mhz   Channel bandwidth in MHz - 20/40. Default is 20
--rate rate       Transmission rate in form like this:
                  {DSSS|OFDM|MCS}<number>[_CDD{1|2}][_SPEXP{1|2}|_MULTI{1|2}]
--region cc/rev   Region to use if --srom switch is not specified or as
                  fallback region if --srom is specified
--regulatory      Print regulatory power instead of TX power
--verbose         Print detailed information on lookup process
--help            Prints this help
"""


###############################################################################
# Module top-level static functions
###############################################################################


def parse_args(argv):
    """ Retrieves command line arguments from given argument list """
    global _query
    global _list_values

    try:
        opts, args = getopt.getopt(argv, "h?",
                                   ["channel=", "band=", "bandwidth=", "rate=",
                                    "region=", "regulatory", "verbose", "help"])
    except getopt.GetoptError:
        ClmUtils.error("%s\n%s" % (ClmUtils.exception_msg(), _usage))

    params = QueryParams()
    verbose_output = False

    for opt, arg in opts:
        try:
            if opt == "--channel":
                params.set_param("channel", arg)
            elif opt == "--band":
                params.set_param("band", Band.parse(arg))
            elif opt == "--bandwidth":
                if arg not in ("20", "40"):
                    raise ValueError("Allowed values are: 20, 40")
                params.set_param("bandwidth", int(arg))
            elif opt == "--rate":
                params.set_param("rate", arg)
            elif opt == "--region":
                params.set_param("region", CcRev(arg))
            elif opt == "--regulatory":
                params.set_param("regulatory", True)
            elif opt == "--verbose":
                verbose_output = True
            elif opt in ("-?", "-h", "--help"):
                print _usage
                sys.exit(0)
            else:
                ClmUtils.error("Internal error: unsupported switch %s" % opt)
        except:
            ClmUtils.error('"%s" option invalid:\n%s' %
                           (opt, ClmUtils.exception_msg()))

    _query = Query(params, verbose_output)

#------------------------------------------------------------------------------


def main(argv):
    """ Main function """
#    print "ClmDriverQuery v%s" % _VERSION
    try:
        parse_args(argv)
    except SystemExit:
        return
    if _list_values:
        for l in _list_values:
            Query.__dict__["print_" + l + "_list"](_query)
    else:
        _query.print_power()

#------------------------------------------------------------------------------

# Standalone module starter
if __name__ == "__main__":
    main(sys.argv[1:])
