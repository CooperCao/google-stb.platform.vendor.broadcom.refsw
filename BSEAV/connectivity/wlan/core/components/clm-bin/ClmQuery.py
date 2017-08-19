#!/usr/bin/env python
""" Retrieves CLM information from CLM XML file """

# Copyright 2016 Broadcom
#
# This program is the proprietary software of Broadcom and/or
# its licensors, and may only be used, duplicated, modified or distributed
# pursuant to the terms and conditions of a separate, written license
# agreement executed between you and Broadcom (an "Authorized License").
# Except as set forth in an Authorized License, Broadcom grants no license
# (express or implied), right to use, or waiver of any kind with respect to
# the Software, and Broadcom expressly reserves all rights in and to the
# Software and all intellectual property rights therein.  IF YOU HAVE NO
# AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
# WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
# THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use
# all reasonable efforts to protect the confidentiality thereof, and to
# use this information only in connection with your use of Broadcom
# integrated circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
# REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
# OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
# DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
# NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
# ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
# CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
# OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
# BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
# SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
# IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
# IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
# ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
# OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
# NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.

import os
import sys
if (sys.version_info[0] != 2) or (sys.version_info[1] < 6):
    print >> sys.stderr, "ClmQuery: Error: This script requires a Python version >= 2.6 and < 3.0 (%d.%d found)" % (sys.version_info[0], sys.version_info[1])
    sys.exit(1)
import getopt
import re
import socket
import pickle
import StringIO
from operator import attrgetter, itemgetter, methodcaller

sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "ClmCompiler"))
try:
    from ClmCompiler import                                    \
        ClmUtils, ClmData, ClmContainer, FilterParams, Locale,  \
        Region, Aggregation, AdvertisedCountry, RateInfo,        \
        RatesInfo, Band, Measure, CcRev, Dfs, ChannelType,        \
        ValidChannel, LocaleType, Bandwidth, SubChanId,            \
        _IGNORED_APPS_VERSION, BlobFormatVersionInfo, Edcrs,        \
        clear_caches
except:
    print >> sys.stderr, "ClmQuery: Error: ClmCompiler.py not found. Please place it to the same directory as this script"
    sys.exit(1)


###############################################################################
# MODULE CONSTANTS
###############################################################################

_PROGRAM_VERSION = "1.37.4"   # Program version
_SERVER_PORT = 17233          # Default TCP port for client-server operation


###############################################################################
# MODULE CLASSES
###############################################################################


class QueryParams:
    """ Parameters from command line

    Attributes:
    xml_file         -- CLM XML file name
    verbose          -- Verbose printing
    list_values      -- List of value lists to print. None for power print
    srom             -- CcRev object specified as --srom parameter or None
    region           -- CcRev object specified as --region parameter or None
    country          -- CC string specified as --country parameter or None
    channel          -- Channel list specified as --channel parameter or None
    band             -- Band object specified as --band parameter or None
    bandwidth        -- Bandwidth list specified as --bandwidth parameter or
                        None
    include_80_80    -- Include 80+80 80MHz channels
    include_non_80_80-- Include non-80+80 80MHz channels
    chains           -- Chain counts list specified as --chains parameter or
                        None
    rate             -- RateInfo objects list specified as --rate parameter or
                        None
    sub_chan         -- Subchannel values list, specified as --subchan
                        parameter. Value of 'None' in this list means main
                        (full) channel
    regulatory       -- True for regulatory power print, false for TX power
                        print
    derive_ht20_siso -- True if MCS SISO rates shall be derived from OFDM rates
    locales          -- True for printing locale names, false for TX power
    singular         -- True for singular mode (when one result shall be
                        returned), false for list mode (when list of results
                        shall be printed)
    server           -- True means server mode of operation
    client           -- None or name of server computer
    server_port      -- TCP port to use
    server_stop      -- Request to stop server
    version          -- True for printing of version information
    ant_gain         -- Antenna gain to subtract from EIRP limits
    sar2             -- 2.4 GHz SAR power cap
    sar5             -- 5 GHz SAR power cap
    """

    ALL = "__ALL__"  # 'all' value of parameters

    def __init__(self):
        """ Constructor, initializes self to as if no parameters were specified
        """
        self._srom = None
        self._region = None
        self._country = None
        self._channel = None
        self._band = None
        self._bandwidth = None
        self._include_80_80 = True
        self._include_non_80_80 = True
        self._rate = None
        self._sub_chan = None
        self._chains = None
        self._regulatory = False
        self._locales = False
        self._singular = True
        self._verbose = False
        self._xml_file = None
        self._list_values = []
        self._server = False
        self._server_stop = False
        self._client = None
        self._server_port = _SERVER_PORT
        self._version = False
        self._derive_ht20_siso = False
        self._ant_gain = None
        self._sar2 = None
        self._sar5 = None

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
        if (member is None) or isinstance(member, bool) or isinstance(member, int):
            setattr(self, member_name, value)
        else:
            raise ValueError("Parameter already set")

    def add_list_value(self, value):
        """ Append given value to list of values to list """
        self._list_values.append(value)

    def is_match(self, param, value):
        """ True if given value matches given query parameter """
        member = getattr(self, param)
        if (member is None) or (member == QueryParams.ALL):
            return True
        if isinstance(member, (list, tuple)):
            return value in member
        return value == member

    def is_channel_match(self, channel, channel_type):
        """ True if given channel matches query parameters """
        if channel_type.bandwidth == Bandwidth._80:
            if ChannelType.is_80_80(channel, channel_type):
                if not self._include_80_80:
                    return False
            else:
                if not self._include_non_80_80:
                    return False
        return self.is_match("channel", channel)

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


class ExtChannel(ValidChannel):
    """ Represents full channel or subchannel

    Attributes:
    channel       -- Main (full) channel number
    channel_type  -- Channel type for main channel
    sub_chan_id   -- Subchannel ID. None for main (full) channel
    effective_bw  -- Bandwidth of subchannel (if object represents subchannel)
                     or main channel
    """

    def __init__(self, channel, band, bandwidth, sub_chan_id=None):
        """ Constructor

        Arguments:
        channel     -- Channel number
        band        -- Band (Band enum)
        bandwidth   -- Bandwidth (Bandwidth enum)
        sub_chan_id -- Subchannel ID (SubChanId enum)
        """
        self.channel = channel
        self.channel_type = ChannelType.create(band, bandwidth)
        self.sub_chan_id = sub_chan_id
        if sub_chan_id is not None:
            self.effective_bw = SubChanId.sub_chan_bandwidth(bandwidth, sub_chan_id)
        else:
            self.effective_bw = None

# =============================================================================


class Query:
    """ Performs all required querying """

    def __init__(self, clm_file_name):
        """ Constructor.

        Arguments:
        clm_file_name -- CLM XML file name
        """
        self._verbose = False
        self._printed_lines = None
        self._data = ClmContainer(clm_file_name).fetch(FilterParams.filter_all(), False)

    def _fmt_powers(self, power, band, query_params):
        """ Returns given power data with gain and SAR limits applied formatted
        as string """
        powers = []
        for dbm in power.powers_dbm:
            if power.measure == Measure.EIRP and query_params.ant_gain is not None:
                dbm -= query_params.ant_gain
            if band == Band._2 and query_params.sar2 is not None:
                dbm = min(dbm, query_params.sar2)
            if band == Band._5 and query_params.sar5 is not None:
                dbm = min(dbm, query_params.sar5)
            powers.append("%5.2f" % dbm)
        return "/".join(powers) if powers else "Disabled"

    def _channel_to_str(self, channel):
        """ Returns string representation of given channel number (which
        bandwidth is unknown)
        """
        return ChannelType.channel_to_string(channel, Bandwidth._80) \
            if ChannelType.is_80_80(channel, Bandwidth._80) else str(channel)

    def _print_power(self, query_params):
        """ Prints regulatory or TX power or locale name """
        region = self._get_region(query_params)
        if region is None:
            self._pr("None")
            return
        if query_params.locales:
            for loc_type in LocaleType.all():
                if loc_type in region.locale_ids:
                    self._pr("Locale %-10s %s" % (str(loc_type) + ":", region.get_locale(loc_type).get_unique_name()))
            return
        if not query_params.singular:
            self._print_power_list(region, query_params)
            return
        channel = self._get_channel(region, query_params)
        if channel is None:
            self._pr("None")
            return
        if query_params.regulatory:
            locale = self._get_region_locale(region, LocaleType.BASE, channel.channel_type.band)
            self._vp("Looking up regulatory power in locale %s\n" % locale.get_unique_name())
            for p in locale.reg_power:
                if self._data.valid_channels.is_channel_in_range(channel.channel, p.channel_range):
                    self._vp("  Using regulatory power defined for range %s\n" %
                             (p.channel_range.range_str))
                    self._pr("%d dBm" % p.powers_dbm[0])
                    self._pr("WAR1: %s" % self._yesno(locale.filt_war1))
                    self._pr("Restricted: %s" % self._yesno(self._data.is_restricted_channel(locale, channel.channel)))
                    self._pr("Dfs: %s" % Dfs.name[locale.dfs])
                    self._pr("Radar: %s" % self._yesno(self._data.is_radar_channel(locale, channel.channel)))
                    self._pr("Beamforming: %s" % self._yesno(region.loc_reg_caps.txbf))
                    self._pr("EDCRS: %s" % Edcrs.name[region.edcrs])
                    self._pr("LoGainNBCAL: %s" % self._yesno(region.lo_gain_nbcal))
                    self._pr("China Spur WAR2: %s" % self._yesno(region.chsprwar2))
                    break
        else:
            if not query_params.rate:
                ClmUtils.error("Rate parameter not specified")
            measure_powers = {Measure.CONDUCTED: "None", Measure.EIRP: "None"}
            for extended in (False, True):
                rate_power_dict = self._data.get_rate_power_dict(region,
                                                                 channel.channel,
                                                                 channel.channel_type,
                                                                 extended,
                                                                 force_ht20_siso_derive=extended and query_params.derive_ht20_siso,
                                                                 sub_chan_id=channel.sub_chan_id)
                locale_power = rate_power_dict.get(query_params.rate[0])
                if locale_power is None:
                    continue
                self._vp("Power %s specified in locale %s\n"
                         % (extended and "derived from information" or "explicitly",
                            locale_power[0].get_unique_name()))
                for measure, power in locale_power[1].items():
                    measure_powers[measure] = self._fmt_powers(power, channel.channel_type.band, query_params)
                    if not power.is_disabled:
                        measure_powers[measure] += " dBm"
                break
            for measure in (Measure.CONDUCTED, Measure.EIRP):
                self._pr("%-10s %s" % (Measure.name[measure] + ":", measure_powers[measure]))

    def _print_power_list(self, region, query_params):
        """ For given region prints power list according to constraints,
        specified by command line parameters
        """
        if query_params.channel is None:
            ClmUtils.error("Channel parameter not specified")
        channels_dict = self._data.get_region_channels_dict(region)
        printed_channels = set()
        if query_params.regulatory:
            self._pr("Region Band Chan Power(dBm) WAR1 Restr  DFS Radar  BF   EDCRS LoGainNBCAL ChSprWar2")
            for ct in ChannelType.all():
                if ct.bandwidth != Bandwidth.default:
                    continue
                if not query_params.is_match("band", ct.band):
                    continue
                locale = self._get_region_locale(region, LocaleType.BASE, ct.band)
                if ct in channels_dict:
                    for channel in channels_dict[ct]:
                        if not query_params.is_match("channel", channel):
                            continue
                        printed_channels.add(channel)
                        power = None
                        for p in locale.reg_power:
                            if self._data.valid_channels.is_channel_in_range(channel, p.channel_range):
                                power = p.powers_dbm[0]
                                break
                        self._pr("%-5s %3sG  %3d     %d       %3s  %3s  %4s   %3s %3s %7s      %3s      %3s" %
                                 (region.ccrev,
                                  Band.name[ct.band],
                                  channel,
                                  power,
                                  self._yesno(locale.filt_war1),
                                  self._yesno(self._data.is_restricted_channel(locale, channel)),
                                  Dfs.name[locale.dfs],
                                  self._yesno(self._data.is_radar_channel(locale, channel)),
                                  self._yesno(region.loc_reg_caps.txbf),
                                  Edcrs.name[region.edcrs],
                                  self._yesno(region.lo_gain_nbcal),
                                  self._yesno(region.chsprwar2)))
        else:
            need_wide = False
            has_80_80 = False
            for narrow in (True, False):
                lines = []
                if narrow:
                    lines.append("Region Band Bandwidth Channel         Rate         Conducted(dBm)   EIRP(dBm)")
                elif has_80_80:
                    lines.append("Region Band Bandwidth    Channel         Rate           Conducted(dBm)         EIRP(dBm)")
                else:
                    lines.append("Region Band Bandwidth Channel         Rate           Conducted(dBm)         EIRP(dBm)")
                for ct in ChannelType.all():
                    if not query_params.is_match("bandwidth", ct.bandwidth):
                        continue
                    if not query_params.is_match("band", ct.band):
                        continue
                    if ct in channels_dict:
                        for channel in channels_dict[ct]:
                            if not query_params.is_channel_match(channel, ct):
                                continue
                            if ChannelType.is_80_80(channel, ct):
                                has_80_80 = True
                                need_wide = True
                            printed_channels.add(channel)
                            for sub_chan_id in [None] + SubChanId.valid_sub_chan_ids(ct.bandwidth):
                                if not query_params.is_match("sub_chan", sub_chan_id):
                                    continue
                                sub_chan_bandwidth = SubChanId.sub_chan_bandwidth(ct.bandwidth, sub_chan_id) if sub_chan_id is not None else ct.bandwidth
                                rate_power_dict = self._data.get_rate_power_dict(region,
                                                                                 channel,
                                                                                 ct,
                                                                                 True,
                                                                                 force_ht20_siso_derive=query_params.derive_ht20_siso,
                                                                                 sub_chan_id=sub_chan_id)
                                rates = rate_power_dict.keys()
                                rates.sort(key=attrgetter("index"))
                                for verbose in (False, True):
                                    if verbose and not self._verbose:
                                        continue
                                    for rate in rates:
                                        if not query_params.is_match("rate", rate):
                                            continue
                                        if not query_params.is_match("chains", rate.chains):
                                            continue
                                        if verbose and not hasattr(rate, "vht_name"):
                                            continue
                                        measure_powers = {Measure.CONDUCTED: "None", Measure.EIRP: "None"}
                                        for measure, power in rate_power_dict[rate][1].items():
                                            measure_powers[measure] = self._fmt_powers(power, ct.band, query_params)
                                        rate_name = ""
                                        if verbose:
                                            rate_name = rate.vht_name if sub_chan_bandwidth != Bandwidth._80 else rate.name
                                        else:
                                            rate_name = rate.vht_name if (sub_chan_bandwidth == Bandwidth._80) and hasattr(rate, "vht_name") else rate.name
                                        if narrow:
                                            line_format = "%-5s  %3sG   %3sM   %3s%-3s   %-19s     %11s %11s"
                                        elif ChannelType.is_80_80(channel, ct):
                                            line_format = "%-5s  %3sG   %3sM %8s%-2s   %-19s %17s %17s"
                                        else:
                                            line_format = "%-5s  %3sG   %3sM   %3s%-3s   %-19s %17s %17s"
                                        lines.append(line_format %
                                                     (region.ccrev, Band.name[ct.band], Bandwidth.name[ct.bandwidth],
                                                      ChannelType.channel_to_string(channel, ct),
                                                      SubChanId.name[sub_chan_id] if sub_chan_id is not None else "",
                                                      rate_name,
                                                      measure_powers[Measure.CONDUCTED],
                                                      measure_powers[Measure.EIRP]))
                                        if narrow and (len(lines[len(lines) - 1]) >= 80):
                                            need_wide = True
                if narrow and not need_wide:
                    break
            for line in lines:
                self._pr(line)
        if query_params.channel != QueryParams.ALL:
            nonprinted_channels = list(set(query_params.channel) - printed_channels)
            nonprinted_channels.sort()
            if nonprinted_channels:
                ClmUtils.error("Channel numbers invalid or incompatible with other parameters: %s" %
                               ", ".join([self._channel_to_str(c) for c in nonprinted_channels]))

    def _print_srom_list(self, query_params):
        """ Prints list of possible --srom switch values """
        self._pr("Aggregations list (possible \"--srom\" switch values):")
        for agg in self._data.aggregations:
            self._pr("    %s -- \"%s\"" % (agg.ccrev, agg.note))

    def _print_region_list(self, query_params):
        """ Prints list of possible --region switch values """
        if query_params.srom is None:
            ccrevs = [region.ccrev for region in self._data.regions]
            self._pr("Regions list (possible \"--region\" switch values):")
        else:
            aggregation = self._data.get_aggregation(query_params.srom)
            if aggregation is None:
                ClmUtils.error("Aggregation %s not found in CLM data" % str(query_params.srom))
            ccrevs = aggregation.mappings
            self._pr("Regions of aggregation %s:" % str(query_params.srom))
        adc_dict = self._data.get_adc_dict()
        for reg_ccrev in ccrevs:
            region = self._data.get_region(reg_ccrev)
            adc = adc_dict.get(region.ccrev)
            flags = []
            if region.sub_chan_rules_name:
                flags.append(region.sub_chan_rules_name)
            if region.edcrs != Edcrs.DEFAULT:
                flags.append(Edcrs.name[region.edcrs])
            for attr, flag in [("txbf", "TXBF"), ("has_1024_qam", "1024QAM"), ("vht_txbf0", "VHTTXBF0")]:
                if getattr(region.loc_reg_caps, attr):
                    flags.append(flag)
            if region.lo_gain_nbcal:
                flags.append("LoGainNBCAL")
            if region.chsprwar2:
                flags.append("ChSprWar2")
            self._pr("    %s%s%s \"%s\"" %
                     (region.ccrev,
                      (" Adc %s" % adc) if adc else "",
                      " [%s]" % (", ".join(flags)) if flags else "",
                      region.country_name))

    def _print_country_list(self, query_params):
        """ Prints list of possible --country switch values """
        self._pr("Country list (possible \"--country\" switch values):")
        if query_params.srom is None:
            ClmUtils.error("Country list could not be printed because SROM parameter not specified")
        agg = self._data.get_aggregation(query_params.srom)
        if agg is None:
            ClmUtils.error("Country list could not be printed because aggregation specified in SROM parameter not found in CLM data")
        adc_dict = self._data.get_adc_dict()
        ccs = [(adc_dict.get(ccrev) if adc_dict.get(ccrev) else ccrev.cc, bool(adc_dict.get(ccrev))) for ccrev in agg.mappings]
        ccs.sort(key=itemgetter(0))
        for cc, advertised in ccs:
            self._pr("    %s%s" % (cc, " (Advertised)" if advertised else ""))

    def _print_channel_list(self, query_params):
        """ Prints list of possible --channel switch values """
        coverage_doesnt_work_without_this = 0
        region = self._get_region(query_params)
        if region is None:
            ClmUtils.error("Can't print channel list because region not defined")
        for ct in ChannelType.all():
            if (not query_params.is_match("band", ct.band)) or (not query_params.is_match("bandwidth", ct.bandwidth)):
                coverage_doesnt_work_without_this += 1
                continue
            locale = self._get_region_locale(region, LocaleType.BASE if ct.bandwidth == Bandwidth.default else LocaleType.HT, ct.band)
            if locale is None:
                self._vp("Region %s contains no locale for %sG %sM channels\n" %
                         (region.ccrev, Band.name[ct.band], Bandwidth.name[ct.bandwidth]))
                continue
            self._vp("Information about %sG %sM channels for region %s will be taken from locale %s\n" %
                     (Band.name[ct.band], Bandwidth.name[ct.bandwidth], region.ccrev, locale.get_unique_name()))
        self._pr("Channel list (possible \"--channel\" switch values):")
        channels_dict = self._data.get_region_channels_dict(region)
        for ct in ChannelType.all():
            if (not query_params.is_match("band", ct.band)) or (not query_params.is_match("bandwidth", ct.bandwidth)):
                coverage_doesnt_work_without_this += 1
                continue
            if ct not in channels_dict:
                continue
            for channel in channels_dict[ct]:
                self._pr("    %s (%sG, %sM)" %
                         (("%8s" if ChannelType.is_80_80(channel, ct) else "%3s") % ChannelType.channel_to_string(channel, ct),
                          Band.name[ct.band], Bandwidth.name[ct.bandwidth]))

    def _print_rate_list(self, query_params):
        """ Prints list of possible --rate switch values """
        region = self._get_region(query_params) if (query_params.srom is not None) or (query_params.region is not None) else None
        rate_dict = {}
        if region is None:
            for ri in range(RatesInfo.NUM_RATES):
                rate = RatesInfo.get_rate(ri)
                if rate.singular:
                    rate_dict[rate] = set()
        else:
            channels_dict = self._data.get_region_channels_dict(region)
            for ct in ChannelType.all():
                if not query_params.is_match("bandwidth", ct.bandwidth):
                    continue
                if not query_params.is_match("band", ct.band):
                    continue
                if ct in channels_dict:
                    for channel in channels_dict[ct]:
                        if not query_params.is_match("channel", channel):
                            continue
                        rate_power_dict = \
                            self._data.get_rate_power_dict(region, channel, ct, False,
                                                           force_ht20_siso_derive=query_params.derive_ht20_siso)
                        for rate in rate_power_dict:
                            rate_dict.setdefault(rate, set())
                            rate_dict[rate].add(rate_power_dict[rate][0].get_unique_name())
        self._pr("Rate list (possible \"--rate\" switch values):")
        for rate in sorted(rate_dict.keys(), key=methodcaller("sort_key")):
            additional_info = []
            if rate_dict[rate]:
                additional_info.append("locale(s): \"%s\"" %
                                       "\", \"".join(loc_full_name for loc_full_name in sorted(rate_dict[rate])))
            if hasattr(rate, "vht_name"):
                additional_info.append("alias: %s" % rate.vht_name)
            self._pr("    %-19s %s" % (rate.name, ("(%s)" % ", ".join(additional_info)) if additional_info else ""))

    def _print_version(self):
        """ Prints CLM data version information """
        self._pr("CLM data version: %s" % (self._data.clm_version and self._data.clm_version or "Unknown"))
        apps_version = self._data.get_apps_version(verbatim=False)
        if apps_version is not None:
            self._pr("Customization data version: %s" % apps_version)
        self._pr("CLM data format version: %s" % (self._data.clm_format_version and self._data.clm_format_version or "Unknown"))
        generator = (self._data.generator_name and self._data.generator_name or "Unknown")
        if self._data.generator_name:
            generator += " (%s)" % (self._data.generator_version and ("v" + self._data.generator_version) or "version unknown")
        self._pr("CLM data generated by: %s" % generator)

    def _get_region(self, query_params):
        """ Returns Region object that matches query parameters or None if
        there is no such object
        """
        if query_params.srom is None:
            if query_params.region is None:
                ClmUtils.error("Neither \"--srom\" nor \"--region\" is specified")
            self._vp("SROM not specified. Will look up region for %s\n" % query_params.region)
            ret = self._data.get_region(query_params.region)
            if ret is None:
                self._vp("Region not found\n")
            else:
                self._vp("Region %s will be used\n" % query_params.region)
            return ret
        if (query_params.country is not None) and (query_params.country == query_params.srom.cc):
            self._vp("SROM CC matches Country IE. Will look up for SROM region\n")
            ret = self._data.get_region(query_params.srom)
            if ret is not None:
                self._vp("  SROM region found and will be used\n")
                return ret
            self._vp("  SROM region not found\n")
        aggregation = self._data.get_aggregation(query_params.srom)
        if aggregation is None:
            self._vp("Aggregation %s not found in CLM data\n" % query_params.srom)
            return None
        if query_params.country is None:
            ccrev = aggregation.ccrev
            self._vp("Country IE CC not specified, will look up for SROM default region %s\n" % query_params.srom)
            ccrev = query_params.srom
        else:
            ccrev = None
            adc_dict = self._data.get_adc_dict()
            self._vp("Looking up for country %s among mappings of aggregation %s\n" %
                     (query_params.country, query_params.srom))
            for cr in aggregation.mappings:
                if cr.cc == query_params.country:
                    self._vp("  Mapping %s found. Will look up region for it\n" % cr)
                    ccrev = cr
                    break
                adc = adc_dict.get(cr)
                if (adc is not None) and (adc == query_params.country):
                    self._vp("  Mapping %s with advertised CC of %s found. Will look up region for it\n" % (cr, adc))
                    ccrev = cr
                    break
            else:
                self._vp("  Mapping not found. Will look up for default (%s/0) region\n" % query_params.country)
                if self._data.get_region(CcRev(query_params.country, 0)):
                    self._vp("  Default region found\n")
                    ccrev = CcRev(query_params.country, 0)
                else:
                    self._vp("  Default region not found\n")
        ret = self._data.get_region(ccrev)
        if ret is not None:
            self._vp("Region %s found and will be used\n" % ccrev)
            return ret
        if query_params.region is None:
            self._vp("Region not found, fallback region was not specified\n")
            return None
        self._vp("Region not found, will look up for fallback region %s\n"
                 % (query_params.region))
        ret = self._data.get_region(query_params.region)
        if ret is None:
            self._vp("Fallback region %s not found\n" % query_params.region)
        else:
            self._vp("Fallback region %s found and will be used\n" % query_params.region)
        return ret

    def _get_channel(self, region, query_params):
        """ Returns ValidChannel object that matches singular query parameters
            or None if there is no such object
        """
        if query_params.channel is None:
            ClmUtils.error("Channel parameter not specified")
        ret = None
        self._vp("Looking up information about channel %s\n" % self._channel_to_str(query_params.channel[0]))
        channels_dict = self._data.get_region_channels_dict(region)
        for band in Band.all():
            if not query_params.is_match("band", band):
                continue
            for bandwidth in Bandwidth.all():
                if (bandwidth != Bandwidth.default) if query_params.regulatory else (not query_params.is_match("bandwidth", bandwidth)):
                    continue
                if bandwidth not in ChannelType.bandwidths_of_band(band):
                    continue
                if ChannelType.create(band, bandwidth) not in channels_dict:
                    self._vp("  Region has no locale for %sG %sM channels\n" % (Band.name[band], Bandwidth.name[bandwidth]))
                    continue
                locale = self._get_region_locale(region, LocaleType.BASE if bandwidth == Bandwidth.default else LocaleType.HT, band)
                if query_params.channel[0] in channels_dict[ChannelType.create(band, bandwidth)]:
                    self._vp("  Found among %sG %sM channels of locale %s\n" %
                             (Band.name[band], Bandwidth.name[bandwidth], locale.get_unique_name()))
                    if ret is not None:
                        ClmUtils.error("Channel number %s is ambiguous. Please specify band and/or bandwidth to disambiguate it" %
                                       self._channel_to_str(query_params.channel[0]))
                    if (query_params.sub_chan[0] is not None) and (query_params.sub_chan[0] not in SubChanId.valid_sub_chan_ids(bandwidth)):
                        ClmUtils.error("Subchannel %s is not defined for %sMHz channel %d" %
                                       (SubChanId.name[query_params.sub_chan[0]],
                                        Bandwidth.name[bandwidth],
                                        query_params.channel[0]))
                    ret = ExtChannel(query_params.channel[0], band, bandwidth, query_params.sub_chan[0])
                else:
                    self._vp("  Not found among %sG %sM channels of locale %s\n" % (Band.name[band], Bandwidth.name[bandwidth], locale.get_unique_name()))
        if ret is None:
            self._vp("  Channel not found\n")
        return ret

    def _get_region_locale(self, region, flavor, band):
        """ Returns locale of given type of given region or None """
        return region.get_locale(LocaleType(flavor, band))

    def _pr(self, s):
        """ Printlns given string """
        if self._printed_lines is None:
            print s
        else:
            self._printed_lines.append(s)
            self._printed_lines.append('\n')

    def _vp(self, s):
        """ Prints given string if verbose printing is enabled """
        if not self._verbose:
            return
        if self._printed_lines is None:
            sys.stdout.write(s)
        else:
            self._printed_lines.append(s)

    def _yesno(self, value):
        """ Returns "Yes" if given value evaluates to True, otherwise "No" """
        return "Yes" if value else "No"

    def query(self, query_params, print_result):
        """ Performs query with given parameters

        Arguments:
        query_params -- Query parameters
        print_result -- True means that query result is printed and return
                        by exception is possible, false means that query
                        result is returned as string and return by exception
                        is not possible
        Returns None if 'print_result' is true, string with query result
        otherwise
        """
        self._verbose = query_params.verbose
        if print_result:
            self._printed_lines = None
        else:
            self._printed_lines = []
        try:
            acted = False
            if query_params.version:
                acted = True
                self._print_version()
            if query_params.list_values:
                acted = True
                for l in query_params.list_values:
                    getattr(Query, "_print_" + l + "_list")(self, query_params)
            elif query_params.srom or query_params.region:
                acted = True
                self._print_power(query_params)
            elif query_params.server_stop:
                acted = True
            if not acted:
                self._pr("No action requested")
            if not print_result:
                return "".join(self._printed_lines)
        except:
            if print_result:
                raise
            else:
                ret = "".join(self._printed_lines)
                if ret:
                    ret += '\n'
                return ret + ClmUtils.exception_msg()


###############################################################################
# MODULE STATIC VARIABLES
###############################################################################

# Help message
_usage = """Usage:
./ClmQuery.py --channel channel --band band --bandwidth mhz --sub_chan subchan
--rate rate --chains chains --srom cc/rev --region cc/rev --country cc
--regulatory --locales --ant_gain db --sar2 dBm --sar5 dBm --list what
--derive_ht20_siso --server --client server_name --server_stop port  --verbose
--help source.xml

--channel channel   Channel number, channel set in quotes (like "1 2 3") or
                    'all' (without quotes). In two latter cases list is
                    printed. For 40MHz channels U/l notation is allowed (e.g.
                    48u is 40MHz channel 46). 80+80 channels use
                    <low>p<high>[{L|H}] notation e.g. 42p58L (if both L and H
                    are requested L/H may be omitted)
--band band         5, 2.4 or all. If concrete channel(s) specified this
                    parameter is optional (disambiguates channel numbers valid
                    for both bands). If channel is 'all' this narrows printed
                    list
--bandwidth mhz     Channel bandwidth in MHz (2.5, 5, 10, 20, 40, 80, 80+80,
                    160), bandwidth set in quotes (like "20 80") or 'all'
                    (without quotes). This parameter is optional - it
                    disambiguates channel numbers valid for several bandwidths
                    or narrows printed list
--sub_chan subchan  Subchannel ID (FULL, L, U, LL, LU, UL, UU, LLL,... UUU),
                    set of subchannel IDs in quotes (like "L UU") or all.
                    Channel and bandwidth parameters are specified for full
                    channel, not for subchannel (e.g.
                    '--channel 42 --sub_chan LL' is correct, but
                    '--channel 36 --sub_chan LL' is incorrect).
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
                DSSS<number>[_MULTI{1|2}] or
                OFDM<number>[{_CDD|_TXBF}{1|2}] or
                MCS<number>[_CDD{1|2}][_STBC][_SPEXP{1|2}][_TXBF{0|1|2}] or
                VHT{8|9}SS{1|2|3}[_CDD{1|2}][_STBC][_SPEXP{1|2}][_TXBF{1|2}]
                Rate group names are DSSS[_MULTI{1|2}],
                OFDM[{_CDD|_TXBF}{1|2}],
                MCS{0-7|8-15|16-23}[{_CDD|_STBC}][_SPEXP]{1|2}][_TXBF{0|1|2}],
                VHT{8-9}SS{1|2|3}[_CDD{1|2}][_STBC][_SPEXP{1|2}][_TXBF{1|2}]
--chains chains     Number of TX rate's post-expansion TX chains (1, 2, 3) or
                    list in quotes (like "2 3"). This is an alternative method
                    to specify group of transmission rates
--srom cc/v         Board SROM contents. Determines aggregation to use.
                    Use "ww" for CC of 00
--region cc/rev     Region to use if --srom switch is not specified or as
                    fallback region if --srom is specified
--country cc        Country code from Country IE to perform lookup for
--regulatory        Print regulatory power instead of TX power
--derive_ht20_siso  Derive power for SISO MCS rates from OFDM rates. This
                    switch is temporary - until SISO MCS rates will be defined
                    in CLM spreadsheet
--locales           Prints locales for region, specified with --region switch
                    or derived from --country/--srom/--region switches
--ant_gain dB       Apply (subtract) given antenna gain offset to all EIRP
                    limits
--sar2 dBm          Apply given SAR limit to 2.4GHz channel powers
--sar5 dBm          Apply given SAR limit to 5gHz channel powers
--server            Server mode of operation. Run ClmQuery with --client switch
                    to make queries. Query-related parameters are ignored
--client server     Client mode of operation. 'server' is a name or IP address
                    of server computer. Use 'localhost' of 127.0.0.1 if server
                    runs on same computer. XML file name is ignored
--server_port port  Port to use for client_server operation. Default is """ + str(_SERVER_PORT) + """
--server_stop       Command from client to stop server. Server IP address is
                    taken from --client switch
--verbose           Print detailed information on lookup process
--list what         Lists possible values for given switch. Possible values for
                    'what': srom (lists included aggregations), country (lists
                    CCs of current aggregations), region (lists all available
                    regions; if --srom specified - lists regions of given
                    aggregation), channel (lists channels with bands and rates
                    current of current srom and country), rate (lists  rates
                    for srom, country and channel)
--version           Prints CLM data version information: version of CLM data
                    and CLM data format, name and version of utility that
                    generated CLM file
--help              Prints this help
source.xml          CLM XML file with data to query
"""


###############################################################################
# Module top-level static functions
###############################################################################


def parse_args(argv):
    """ Retrieves command line arguments from given argument list (without
    first (file name) parameter
    """
    try:
        opts, args = getopt.getopt(argv, "h?",
                                   ["channel=", "band=", "bandwidth=", "rate=",
                                    "chains=", "srom=", "region=", "country=",
                                    "regulatory", "derive_ht20_siso", "locales",
                                    "sub_chan=", "ant_gain=", "sar2=", "sar5=",
                                    "verbose", "server", "client=", "server_stop",
                                    "server_port=", "list=", "version", "help",
                                    "unsupportedswitchfortestpurposes"])
    except getopt.GetoptError:
        print _usage
        ClmUtils.error(ClmUtils.exception_msg())

    params = QueryParams()
    for opt, arg in opts:
        try:
            if opt == "--channel":
                if arg == "all":
                    params.set_param("channel", QueryParams.ALL)
                    params.set_param("singular", False)
                else:
                    channels = []
                    for channel_name in re.split(r"[ ,\t]+", arg):
                        if channel_name == "":
                            continue
                        m = re.match(r"^(\d+)(p(\d+))?((l)|(u))?$", channel_name, re.IGNORECASE)
                        if m is None:
                            raise ValueError("Invalid channel(s) specification: \"%s\"" % channel_name)
                        if m.group(2):
                            l = int(m.group(1))
                            h = int(m.group(3))
                            if l >= h:
                                raise ValueError("Invalid channel(s) specification: \"%s\"" % channel_name)
                            if m.group(4):
                                channels.append(ChannelType.compose_80_80(l if m.group(5) else h,
                                                                          h if m.group(5) else l))
                            else:
                                channels.append(ChannelType.compose_80_80(l, h))
                                channels.append(ChannelType.compose_80_80(h, l))
                        else:
                            channel = int(m.group(1))
                            if m.group(5):
                                channel += 2
                            if m.group(6):
                                channel -= 2
                            channels.append(channel)
                    if len(channels) == 0:
                        raise ValueError("No valid channels specified")
                    params.set_param("channel", channels)
                    if len(channels) > 1:
                        params.set_param("singular", False)
            elif opt == "--band":
                if arg == "all":
                    params.set_param("band", QueryParams.ALL)
                    params.set_param("singular", False)
                elif arg in Band.name.values():
                    params.set_param("band", Band.parse(arg))
                else:
                    raise ValueError("Invalid band specification")
            elif opt == "--bandwidth":
                if arg == "all":
                    params.set_param("bandwidth", QueryParams.ALL)
                    params.set_param("singular", False)
                else:
                    bandwidths = []
                    has_80 = False
                    has_80_80 = False
                    for bandwidth_name in map(lambda m: m.group(0), re.finditer(r"80\+80|(\d+(\.\d)?)", arg)):
                        if bandwidth_name == "80+80":
                            bandwidths.append(Bandwidth._80)
                            has_80_80 = True
                        else:
                            if bandwidth_name == "80":
                                has_80 = True
                            bandwidths.append(Bandwidth.parse(bandwidth_name))
                    if len(bandwidths) > 1:
                        params.set_param("singular", False)
                    elif len(bandwidths) == 0:
                        raise ValueError("Invalid bandwidth specification")
                    params.set_param("bandwidth", bandwidths)
                    params.set_param("include_80_80", has_80_80)
                    params.set_param("include_non_80_80", has_80)
            elif opt == "--sub_chan":
                if arg == "all":
                    params.set_param("sub_chan", QueryParams.ALL)
                    params.set_param("singular", False)
                else:
                    sub_chan_names = arg.replace(',', ' ').split()
                    if not sub_chan_names:
                        raise ValueError("No valid subchannels specified")
                    if len(sub_chan_names) > 1:
                        params.set_param("singular", False)
                    sub_chans = []
                    for sub_chan_name in sub_chan_names:
                        if sub_chan_name.lower() == "full":
                            sub_chans.append(None)
                        else:
                            sub_chans.append(SubChanId.parse(sub_chan_name))
                    params.set_param("sub_chan", sub_chans)
            elif opt == "--chains":
                params.set_param("singular", False)
                if arg == "all":
                    params.set_param("chains", QueryParams.ALL)
                else:
                    chains = []
                    for chains_str in re.findall(r"\d+", arg):
                        c = int(chains_str)
                        if (c < 1) or (c > RatesInfo.get_max_chains()):
                            raise ValueError("Invalid number of chains: \"%s\". Shall be between 1 and %d" % (chains_str, RatesInfo.get_max_chains()))
                        chains.append(c)
                    if len(chains) == 0:
                        raise ValueError("Invalid chains specification")
                    params.set_param("chains", chains)
            elif opt == "--rate":
                if arg == "all":
                    params.set_param("rate", QueryParams.ALL)
                    params.set_param("singular", False)
                else:
                    rate_infos = []
                    for rate_name in re.findall(r"[A-Z0-9_\-]+", arg.upper()):
                        rate_infos += RatesInfo.get_expanded_rate(rate_name)
                    if len(rate_infos) > 1:
                        params.set_param("singular", False)
                    elif len(rate_infos) == 0:
                        raise ValueError("Invalid rate specification")
                    params.set_param("rate", rate_infos)
            elif opt == "--srom":
                params.set_param("srom", CcRev(arg))
            elif opt == "--region":
                params.set_param("region", CcRev(arg))
            elif opt == "--country":
                if len(arg) != 2:
                    raise ValueError("Invalid country code")
                params.set_param("country", arg)
            elif opt == "--regulatory":
                params.set_param("regulatory", True)
            elif opt == "--derive_ht20_siso":
                params.set_param("derive_ht20_siso", True)
            elif opt == "--locales":
                params.set_param("locales", True)
            elif opt == "--ant_gain":
                params.set_param("ant_gain", float(arg))
            elif opt == "--sar2":
                params.set_param("sar2", float(arg))
            elif opt == "--sar5":
                params.set_param("sar5", float(arg))
            elif opt == "--verbose":
                params.set_param("verbose", True)
            elif opt == "--version":
                params.set_param("version", True)
            elif opt == "--list":
                allowed_values = ("srom", "country", "region", "channel", "rate")
                if not(arg in allowed_values):
                    raise ValueError("Allowed values are \"%s\"" % "\", \"".join(allowed_values))
                params.add_list_value(arg)
            elif opt == "--server":
                params.set_param("server", True)
            elif opt == "--client":
                params.set_param("client", arg)
            elif opt == "--server_port":
                params.set_param("server_port", int(arg))
            elif opt == "--server_stop":
                params.set_param("server_stop", True)
            elif opt in ("-?", "-h", "--help"):
                print _usage
                sys.exit(0)
            else:
                ClmUtils.error("Internal error: unsupported switch %s" % opt)
        except (SystemExit, KeyboardInterrupt):
            raise
        except:
            ClmUtils.error("\"%s\" option invalid: %s" % (opt, ClmUtils.exception_msg()))
    if args:
        params.set_param("xml_file", args[0])
    if len(args) > 1:
        ClmUtils.error("Too many positional arguments: \"%s\"" % " ".join(args))
    return params

# =============================================================================


def tcp_client(query_params):
    """ TCP client operation

    Arguments:
    query_params -- Parameters from command line (contain query parameters and
                    server address)
    """
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((query_params.client, query_params.server_port))
        _tcp_send(client_socket, query_params)
        print _tcp_recv(client_socket, ignore_error=query_params.server_stop)
        try:
            client_socket.close()
        except socket.error:  # pragma: no cover
            pass
    except:
        ClmUtils.error(ClmUtils.exception_msg())

# =============================================================================


def tcp_server(query, query_params):
    """ TCP server operation

    Arguments:
    query        -- Query object that would process incoming requests
    query_params -- Parameters from command line (contain server port). Actual
                    query parameters will be received from incoming TCP streams
    """
    listen_socket = None
    try:
        listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        listen_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        if os.name == "nt":  # pragma: no cover
            # On Windows accept() is uninterruptible, so this timeout gives
            # server a chance to stop on Ctrl+C
            listen_socket.settimeout(1)
        listen_socket.bind(("", query_params.server_port))
        listen_socket.listen(5)
    except:
        ClmUtils.error(ClmUtils.exception_msg())
    print "ClmQuery server started.\nPress Ctrl+C to terminate"
    worker_socket = None
    try:
        while 1:
            try:
                worker_socket, _ = listen_socket.accept()
                if os.name == "nt":  # pragma: no cover
                    worker_socket.settimeout(None)
                request_query_params = _tcp_recv(worker_socket)
                result = query.query(request_query_params, False)
                _tcp_send(worker_socket, result)
                worker_socket.close()
                worker_socket = None
                if request_query_params.server_stop:
                    return
            except socket.timeout:  # pragma: no cover
                pass
            except KeyboardInterrupt:  # pragma: no cover
                return
            except:  # pragma: no cover
                print >> sys.stderr, ClmUtils.exception_msg()
                return
    except KeyboardInterrupt:  # pragma: no cover
        return
    finally:
        if listen_socket is not None:
            try:
                listen_socket.close()
            except:  # pragma: no cover
                pass
        if worker_socket is not None:  # pragma: no cover
            try:
                listen_socket.close()
            except:
                pass

# =============================================================================


def _tcp_recv(sock, ignore_error=False):
    """ TCP reception. Receives until end of stream, then unpickles

    Arguments:
    socket       -- Socket to receive from
    ignore_error -- True to ignore socket_error
    Returns Received object. May generate exception
    """
    s = ""
    while 1:
        try:
            chunk = sock.recv(1000)
        except socket.error:  # pragma: no cover
            if not ignore_error:
                raise
            chunk = ""
        if chunk == "":
            break
        s += chunk
    sio = StringIO.StringIO(s)
    unpickler = pickle.Unpickler(sio)
    return unpickler.load()

# =============================================================================


def _tcp_send(sock, data):
    """ TCP transmission of given object. Closes output stream upon completion

    Arguments:
    sock -- Socket to transmit to
    data -- Object to transmit. Object is pickled before transmission
    """
    sio = StringIO.StringIO()
    pickler = pickle.Pickler(sio)
    pickler.dump(data)
    data_str = sio.getvalue()
    sock.sendall(data_str)
    sock.shutdown(socket.SHUT_WR)

# =============================================================================


def main(argv):
    """ Main function """
    try:
        ClmUtils.error_source = "ClmQuery: "
        print "ClmQuery v%s" % _PROGRAM_VERSION
        query_params = parse_args(argv)
        query = None
        if query_params.client is None:
            if query_params.xml_file is None:
                print _usage
                ClmUtils.error("Source CLM XML file shall be specified")
            try:
                query = Query(query_params.xml_file)
            except (SystemExit, KeyboardInterrupt):
                raise
            except:
                ClmUtils.error("Error opening source CLM file \"%s\": %s" %
                               (query_params.xml_file, ClmUtils.exception_msg()))
        if query_params.client:
            tcp_client(query_params)
        elif query_params.server:
            tcp_server(query, query_params)
        else:
            query.query(query_params, True)
    except SystemExit as se:
        if se.code:
            if not isinstance(se.code, int) or (str(se.code) != ClmUtils.exception_msg()):
                print >>sys.stderr, ClmUtils.exception_msg()
            return 1
        return 0
    except KeyboardInterrupt:
        print >> sys.stderr, "ClmQuery: Error: Interrupted"
        return 1
    finally:
        clear_caches()
    return 0

# =============================================================================


# Standalone module starter
if __name__ == "__main__":  # pragma: no cover
    ret = main(sys.argv[1:])
    if ret:
        sys.exit(ret)
