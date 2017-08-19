#!/usr/bin/env python
""" Compiles CLM XML file (or portion of it) into C BLOB """

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
    print >> sys.stderr, "ClmCompiler: Error: This script requires a Python version >= 2.6 and < 3.0 (%d.%d found)" % (sys.version_info[0], sys.version_info[1])
    sys.exit(1)
import copy
import datetime
import getopt
from operator import attrgetter, itemgetter, methodcaller
import re
import shlex
import textwrap
import subprocess
import tempfile
import shutil

# Iron Python library patches are here
sys.path.append(os.path.dirname(sys.argv[0]) + r"/../ThirdParty/PythonLib")

# This fallback is for Iron Python that does not have cElementTree and has buggy ElementTree. cElementTree alone is good enough for production use
try:
    import xml.etree.cElementTree as Et
except:  # pragma: no cover
    try:
        import etree.ElementTree as Et  # Iron Python library patches
    except:
        import xml.etree.ElementTree as Et

###############################################################################
# MODULE CONSTANTS
###############################################################################


# Program version
_PROGRAM_VERSION = "1.37.4"

# Input XML format version
_XML_VERSION = "5.8"

# Minimally supported input XML version
_XML_MIN_VERSION = "2.0"

# Output BLOB format version
_BLOB_VERSION = "18.2"

# Apps version that shall not be printed
_IGNORED_APPS_VERSION = "Broadcom-0.0"

# Environment variable for options
_ENV_OPTIONS = "CLM_COMPILER_OPTIONS"

# Environment variable to suppress title printing
_ENV_NO_TITLE = "CLM_COMPILER_NO_TITLE"

###############################################################################
# MODULE CLASSES
###############################################################################


class DiffReport:
    """ Difference report with context nesting """

    def __init__(self):
        """ Constructor - empty context stack, empty report """
        self._context = []
        self._difference = None

    def comparing(self, context_name):
        """ Entering comparison context

        Arguments:
        context_name -- String that identifies comparison context
        """
        self._context.append(context_name)

    def compared(self):
        """ Leaving comparison context """
        self._context.pop()

    def difference_in(self, attr_name):
        """ Reports difference - creates difference report

        Arguments:
        attr_name -- name of attribute that differs
        """
        self._difference = ""
        for context in self._context:
            if self._difference:
                self._difference += ", "
            self._difference += context
        self._difference += ": " + attr_name

    def get_diff_report(self, result_for_empty):
        """ Returns difference report

        Arguments:
        result_for_empty -- String to return is difference report is empty
        """
        return self._difference if self._difference is not None else result_for_empty

# =============================================================================


class ClmUtils:
    """ Collection of utility classes and static functions """

    error_source = ""  # Prefix to use in error and warning messages

    class EqualityMixin:
        """ Mixin base class that provides equality implemented by means of
        objects' dictionaries comparison
        """

        def __eq__(self, other):
            """ Equality comparison """
            return hasattr(other, "__dict__") and \
                (self.__dict__ == other.__dict__)

        def __ne__(self, other):
            """ Inequality comparison """
            return not self.__eq__(other)

    @staticmethod
    def dictionaries_partially_equal(dict1, dict2, excluded_keys,
                                     included_keys, diff_report=None):
        """ Compares two dictionaries except for given set of keys

        Arguments:
        dict1         -- First dictionary
        dict2         -- Second dictionary
        excluded_keys -- Keys not to use in comparison (list, tuple, set)
        included_keys -- If nonempty then only keys from this list (tuple, set)
                         are used
        diff_report   -- Optional difference report object
        Returns       -- True if except for given keys dictionaries are equal
        """
        if diff_report is None:
            diff_report = DiffReport()
        for key in dict1:
            if excluded_keys and (key in excluded_keys):
                continue
            if included_keys and (key not in included_keys):
                continue
            if key not in dict2:
                diff_report.difference_in("Only one object has attribute \"%s\"" % str(key))
                return False
            if dict1[key] != dict2[key]:
                diff_report.difference_in("Difference in \"%s\"" % str(key))
                return False
        for key in dict2:
            if excluded_keys and (key in excluded_keys):
                continue
            if included_keys and (key not in included_keys):
                continue
            if key not in dict1:
                diff_report.difference_in("Only one object has attribute \"%s\"" % str(key))
                return False
        return True

    @staticmethod
    def error(msg):
        """ Static function that exits given error message (with 'ERROR: '
        prefix)
        """
        sys.exit(ClmUtils.error_source + "Error: " + msg)

    @staticmethod
    def exception_msg():
        """ Static function that returns text of recent exception """
        try:
            return str(sys.exc_info()[1])
        except:                             # pragma: no cover
            return str(sys.exc_info())

    @staticmethod
    def warning(msg):
        """ Prints given message as warning (with 'WARNING: ' prefix) """
        print >> sys.stderr, ClmUtils.error_source + "Warning: " + msg

    @staticmethod
    def major(version):
        """ For given version string "major.minor" returns major as number """
        m = re.match(r"^(\d+)\.\d+", version)
        if m:
            return int(m.group(1))
        else:
            return None

    @staticmethod
    def minor(version):
        """ For given version string "major.minor" returns minor as number """
        m = re.match(r"^\d+\.(\d+)", version)
        if m:
            return int(m.group(1))
        else:
            return None

    @staticmethod
    def compare_versions(v1, v2):
        """ Compares two given version strings. Returns negative/0/positive """
        if not v1:
            return -1 if v2 else 0
        if not v2:
            return 1
        major1 = ClmUtils.major(v1)
        major2 = ClmUtils.major(v2)
        return (major1 - major2) if (major1 != major2) else (ClmUtils.minor(v1) - ClmUtils.minor(v2))

    @staticmethod
    def open_for_write(file_name):
        """ Opens given file for writing, unlinking (deleting) it first """
        try:
            os.unlink(file_name)
        except:
            pass
        return open(file_name, "w")

# =============================================================================


class EtreeElementWrapper:
    """ Replicates functionality of xml.etree.Element with certain enhancements

    This class enriches Element in the following ways:
        - Encapsulates handling of default namespace prefix (if any)
        - Circumvents bug in Element.find()
    Due to some strange reason it could not be directly used for iteration -
    getchildren() shall be used instead
    """
    @staticmethod
    def get_root_namespace(etree):
        """ Returns namespace of root element of given ElementTree """
        m = re.match(r"{(.*)}", etree.getroot().tag)
        return m.group(1) if m else ""

    def __init__(self, e, default_namespace_prefix=""):
        """ Construct self from element or element-like object

        Argument:
        e                        -- Element, tree (root element is used),
                                    object of this type
        default_namespace_prefix -- Default namespace prefix to use if element
                                    constructed from element object
        """
        if isinstance(e, EtreeElementWrapper):
            self._elem = e._elem
            self._prefix = e._prefix
        elif Et.iselement(e):
            self._elem = e
            self._prefix = default_namespace_prefix
        elif isinstance(e, Et.ElementTree):
            self._elem = e.getroot()
            self._prefix = ("{%s}" % EtreeElementWrapper.get_root_namespace(e)) if EtreeElementWrapper.get_root_namespace(e) else ""
        else:
            raise ValueError("Invalid element type")
        self.text = self._elem.text.encode("ascii", "replace") if isinstance(self._elem.text, unicode) else self._elem.text
        self.tag = self._elem.tag[len(self._prefix):]
        self.attrib = self._elem.attrib

    def find(self, tag, max_version=None):
        """ Similar to Elelent.find(). Automagically appends default namespace
        prefix (if any) to tag name, handles tag versioning

        This function internally uses findall() instead of find() (since latter
        often does not work).

        Arguments:
        tag          -- Tag to look for
        max_version  -- None (for unversioned tag) or maximum tag version to
        look for
        Returns EtreeElementWrapper object if given element found (if tag
        maximum tag version specified - returns element with tag with maximum
        version, not exceeding given maximum version), None
        otherwise
        """
        if max_version is None:
            elems = self._elem.findall(self._prefix + tag)
            return EtreeElementWrapper(elems[0], self._prefix) if elems else None
        else:
            ret = None
            ret_version = -1
            tag_base = self._prefix + tag
            tag_base_len = len(tag_base)
            for elem in self._elem.getchildren():
                if not elem.tag.startswith(tag_base):
                    continue
                version_str = elem.tag[tag_base_len:]
                if version_str and (not version_str.isdigit()):
                    continue
                version = int(version_str) if version_str else 0
                if (version > ret_version) and (version <= max_version):
                    ret = EtreeElementWrapper(elem, self._prefix)
                    ret_version = version
            return ret

    def findall(self, tag, max_version=None):
        """ Similar to Element.findall(). Automagically appends default
        namespace prefix (if any) to tag name, handles tag versioning

        Arguments:
        tag         -- Tag to look for
        max_version -- None (*for unversioned tag) or maximum tag version to
        look for
        Returns list of matching EtreeElementWrapper objects
        """
        if max_version is None:
            return [EtreeElementWrapper(e, self._prefix) for e in
                    self._elem.findall(self._prefix + tag)]
        else:
            ret = []
            tag_base = self._prefix + tag
            tag_base_len = len(tag_base)
            for elem in self._elem.getchildren():
                if not elem.tag.startswith(tag_base):
                    continue
                suffix = elem.tag[tag_base_len:]
                if (suffix == "") or (suffix.isdigit() and (int(suffix) <= max_version)):
                    ret.append(EtreeElementWrapper(elem, self._prefix))
            return ret

    def get(self, key, default=None):
        """ Same as Element.get() - gets attribute name """
        return self._elem.get(key, default)

    def items(self):
        """ Same as Element.items() """
        return self._elem.items()

    def keys(self):
        """ Same as Element.keys() """
        return self._elem.keys()

    def getchildren(self):
        """ Same as Element.getchildren(), but returns list of
        EtreeElementWrapper objects
        """
        return [EtreeElementWrapper(e, self._prefix) for e in self._elem.getchildren()]

# =============================================================================


class Band:
    """ Band enum

    Order of values is important since they may be used in a bitmask
    """
    NONE, _2, _5, ALL = range(4)  # None, 2.4GHz band, 5GHz band, both bands

    name = {_2: "2.4", _5: "5"}

    @staticmethod
    def parse(band_str):
        """ Extracts band vale from a string that contains either '2.4. or '5'
        """
        if band_str == "5":
            return Band._5
        if band_str == "2.4":
            return Band._2
        raise ValueError("Invalid band identifier \"%s\"" % band_str)

    @staticmethod
    def all():
        """ Returns tuple that lists all singular band IDs """
        return (Band._2, Band._5)

# =============================================================================


class Bandwidth:
    """ Bandwidth enum """
    _2_5, _5, _10, _20, _40, _80, _160 = range(7)  # Bandwidths named after their MHz values

    default = _20  # Default bandwidth

    maximum = _160  # Maximum bandwidth

    name = {_2_5: "2.5", _5: "5", _10: "10", _20: "20", _40: "40", _80: "80", _160: "160"}

    id_name = dict(map(lambda kv: (kv[0], kv[1].replace(".", "_")), name.items()))

    @staticmethod
    def parse(bandwidth_str):
        """ Converts string value to enum value """
        for key, value in Bandwidth.name.items():
            if value == bandwidth_str:
                return key
        raise ValueError("Invalid bandwidth value \"%s\"" % bandwidth_str)

    @staticmethod
    def is_ulb(bw):
        """ True if bandwidth is ULB """
        return bw < Bandwidth._20

    @staticmethod
    def all():
        """ Returns all bandwidths """
        return sorted(Bandwidth.name.keys())

# =============================================================================


class Dfs:
    """ DFS type enum """
    NONE, EU, US, TW = range(4)

    name = {NONE: "None", EU: "EU", US: "US", TW: "TW"}

    @staticmethod
    def parse(dfs_str):
        """ Extract DFS type from string that contains either 'eu' or 'us' """
        if dfs_str == "eu":
            return Dfs.EU
        if dfs_str == "us":
            return Dfs.US
        if dfs_str == "tw":
            return Dfs.TW
        raise ValueError("Invalid DFS identifier \"%s\"" % dfs_str)

# =============================================================================


class Edcrs:
    """ EDCRS type enum """
    DEFAULT, EU = range(2)

    name = {DEFAULT: "None", EU: "EU"}

    @staticmethod
    def parse(edcrs_str):
        """ Extract DFS type from XML string. Returns enum value on success,
        None on false """
        if edcrs_str == "eu":
            return Edcrs.EU
        return None

# =============================================================================


class Measure:
    """ Measurement type enum """
    NONE, CONDUCTED, EIRP = range(3)

    name = {NONE: "", CONDUCTED: "Conducted", EIRP: "EIRP"}

    @staticmethod
    def parse(measure_str):
        """ Extracts measurement type from string that contains '', 'conducted'
        or 'eirp'
        """
        if measure_str == "conducted":
            return Measure.CONDUCTED
        if measure_str == "eirp":
            return Measure.EIRP
        raise ValueError("Invalid measure identifier \"%s\"" % measure_str)

# =============================================================================


class ChannelType(ClmUtils.EqualityMixin):
    """ Channel type (context of channel number)

    Private attributes:
    _band      -- Band channel belongs to
    _bandwidth -- Channel width in MHz

    Attributes (all readonly):
    band      -- Band channel belongs to
    bandwidth -- Channel bandwidth
    ulb       -- ULB channel type
    """
    # Masks of fields within 80+80 channel number
    _80_80_H_ACTIVE = 0x00001
    _80_80_L_MASK = 0x001FE
    _80_80_H_MASK = 0x1FE00
    _80_80_L_SHIFT = 1
    _80_80_H_SHIFT = 9

    # All channel types, mapped to self to facilitate single-instance behavior
    _all = {}

    def __init__(self):
        """ Constructor. Shall never be called directly from outside the class
        create() function shall be used to create ChannelType objects """
        self._band = None
        self._bandwidth = None

    def __getattr__(self, name):
        """ Returns values of readonly attributes """
        if name == "band":
            return self._band
        if name == "bandwidth":
            return self._bandwidth
        if name == "ulb":
            return Bandwidth.is_ulb(self._bandwidth)
        raise AttributeError("Internal error: attribute \"%s\" not defined" % name)

    def __hash__(self):
        """ Hash to use this type as dictionary key or set element """
        return (self._band, self._bandwidth).__hash__()

    def __repr__(self):
        """ Returns debugger representation """
        return "<%sM, %sG>" % (Bandwidth.name[self._bandwidth], Band.name[self._band])

    def sort_key(self):
        """ Returns key for valid channels sorting """
        return (self._band, self._bandwidth)

    @staticmethod
    def create(band, bandwidth):
        """ Create and return Channel Type object of given band and bandwidth

        Arguments:
        band      -- One of Band._... enum values
        bandwidth -- One of Bandwidth._... enum values
        Returns ChannelType object
        """
        ct = ChannelType()
        ct._band = band
        ct._bandwidth = bandwidth
        if not ChannelType._all:
            ChannelType._build_all()
        ret = ChannelType._all.get(ct)
        if ret is None:
            raise ValueError("Invalid channel type: %sGHz band, %sMHz bandwidth" % (Band.name[band], Bandwidth.name[bandwidth]))
        return ret

    @staticmethod
    def all():
        """ Generator that returns all channel types """
        if not ChannelType._all:
            ChannelType._build_all()
        return sorted(ChannelType._all.iterkeys(), key=methodcaller("sort_key"))

    @staticmethod
    def bandwidths_of_band(band):
        """ Returns list of valid bandwidths for given band """
        return [ct.bandwidth for ct in ChannelType.all() if ct.band == band]

    @staticmethod
    def channel_step(channel_type):
        """ Returns step of channel numbers for given channel type """
        bw = Bandwidth._20 if channel_type.ulb else channel_type.bandwidth
        return 1 if channel_type.band == Band._2 else (int(Bandwidth.name[bw]) / 5)

    @staticmethod
    def is_80_80(channel, bw_or_ct):
        """ True if given channel is 80+80 channel

        Arguments:
        channel   -- Channel number
        bw_or_ct  -- Channel bandwidth or ChannelType
        Return True if given channel is 80+80 channel
        """
        if not isinstance(bw_or_ct, int):
            bw_or_ct = bw_or_ct.bandwidth
        return ((bw_or_ct == Bandwidth._80) and
                ((channel & ChannelType._80_80_L_MASK) != 0) and
                ((channel & ChannelType._80_80_H_MASK) != 0))

    @staticmethod
    def compose_80_80(active_channel, other_channel):
        """ Makes 80+80 channel number out of components

        Arguments:
        active_channel -- Active channel (channel power limit bound to)
        other_channel  -- Other channel in pair
        Returns Channel number
        """
        active_channel = int(active_channel)
        other_channel = int(other_channel)
        return ((min(active_channel, other_channel) << ChannelType._80_80_L_SHIFT) +
                (max(active_channel, other_channel) << ChannelType._80_80_H_SHIFT) +
                (ChannelType._80_80_H_ACTIVE if active_channel > other_channel else 0))

    @staticmethod
    def decompose_80_80(channel, bw_or_ct):
        """ Decomposes 80+80 channel numbers to components

        Arguments:
        channel   -- Channel number
        bw_or_ct  -- Channel bandwidth or ChannelType
        Return (active, other) with active and other channel numbers if
        arguments seemingly valid, (None, None) otherwise
        """
        if not isinstance(bw_or_ct, int):
            bw_or_ct = bw_or_ct.bandwidth
        if bw_or_ct != Bandwidth._80:
            return (None, None)
        l = (channel & ChannelType._80_80_L_MASK) >> ChannelType._80_80_L_SHIFT
        h = (channel & ChannelType._80_80_H_MASK) >> ChannelType._80_80_H_SHIFT
        if l and h:
            return (h, l) if channel & ChannelType._80_80_H_ACTIVE else (l, h)
        return (None, None)

    @staticmethod
    def channel_to_string(channel, bw_or_ct):
        """ String representation of channel number

        Arguments:
        channel   -- Channel number
        bw_or_ct  -- Channel bandwidth or ChannelType
        Return String representation (numeric or 80+80)
        """
        active, other = ChannelType.decompose_80_80(channel, bw_or_ct)
        if active:
            return "%dp%d%s" \
                   % (min(active, other), max(active, other),
                      "L" if active < other else "U")
        return str(channel)

    @staticmethod
    def clear_cache():
        """ Clears channel types. To be used before each unittest """
        ChannelType._all.clear()

    @staticmethod
    def _build_all():
        """ Fills ChannelType._all if it is not filled yet """
        if ChannelType._all:
            return
        for band in Band.all():
            for bandwidth in Bandwidth.all():
                if (band == Band._2) and (bandwidth > Bandwidth._40):
                    continue
                ct = ChannelType()
                ct._band = band
                ct._bandwidth = bandwidth
                ChannelType._all[ct] = ct

# =============================================================================


class SubChanId:
    """ Subchannel ID enum and related helper functions """
    L, U, LL, LU, UL, UU, LLL, LLU, LUL, LUU, ULL, ULU, UUL, UUU = range(14)

    name = {L: "L", U: "U", LL: "LL", LU: "LU", UL: "UL", UU: "UU",
            LLL: "LLL", LLU: "LLU", LUL: "LUL", LUU: "LUU",
            ULL: "ULL", ULU: "ULU", UUL: "UUL", UUU: "UUU"}

    @staticmethod
    def parse(sub_chan_id_str):
        """ Extracts subchannel ID type from lowercase string """
        for sc_id, sc_name in SubChanId.name.items():
            if sub_chan_id_str.upper() == sc_name:
                return sc_id
        raise ValueError("Invalid subchannel ID \"%s\"" % sub_chan_id_str)

    @staticmethod
    def valid_sub_chan_ids(bandwidth):
        """ Valid subchannel IDs for given bandwidth """
        if (bandwidth in Bandwidth.all()) and (bandwidth <= Bandwidth._20):
            return []
        if bandwidth == Bandwidth._40:
            return [SubChanId.L, SubChanId.U]
        if bandwidth == Bandwidth._80:
            return [SubChanId.L, SubChanId.U,
                    SubChanId.LL, SubChanId.LU, SubChanId.UL, SubChanId.UU]
        if bandwidth == Bandwidth._160:
            return [SubChanId.L, SubChanId.U,
                    SubChanId.LL, SubChanId.LU, SubChanId.UL, SubChanId.UU,
                    SubChanId.LLL, SubChanId.LLU, SubChanId.LUL, SubChanId.LUU,
                    SubChanId.ULL, SubChanId.ULU, SubChanId.UUL, SubChanId.UUU]
        raise ValueError("Invalid bandwidth")

    # Offsets from main channel number to subchannel number, indexed first by bandwidth of
    # main channel,then by bandwidth of subchannel then by subchannel IDs
    _offsets = {
        Bandwidth._40: {Bandwidth._20: {L: -2, U: 2}},
        Bandwidth._80: {Bandwidth._20: {LL: -6, LU: -2, UL: 2, UU: 6},
                        Bandwidth._40: {L: -4, U: 4, LL: -4, LU: -4, UL: 4, UU: 4}},
        Bandwidth._160: {Bandwidth._20: {LLL: -14, LLU: -10, LUL: -6, LUU: -2, ULL: 2, ULU: 6, UUL: 10, UUU: 14},
                         Bandwidth._40: {LL: -12, LU: -4, UL: 4, UU: 12,
                                         LLL: -12, LLU: -12, LUL: -4, LUU: -4, ULL: 4, ULU: 4, UUL: 12, UUU: 12},
                         Bandwidth._80: {L: -8, U: 8, LL: -8, LU: -8, UL: 8, UU: 8,
                                         LLL: -8, LLU: -8, LUL: -8, LUU: -8, ULL: 8, ULU: 8, UUL: 8, UUU: 8}}}

    @staticmethod
    def sub_chan(main_channel, main_channel_bandwidth, sub_channel_bandwidth, sub_chan_id):
        """ Subchannel with given characteristics relative to given main channel

        Arguments:
        main_channel           -- Main channel number
        main_channel_bandwidth -- Main channel bandwidth (see Bandwidth enum)
        sub_channel_bandwidth  -- Subchannel bandwidth (see Bandwidth enum)
        sub_chan_id            -- Subchannel ID (see SubChanId enum)
        Returns                -- Subchannel's channel number or None
        """
        if main_channel_bandwidth == sub_channel_bandwidth:
            return main_channel
        if ChannelType.is_80_80(main_channel, main_channel_bandwidth):
            main_channel = ChannelType.decompose_80_80(main_channel, main_channel_bandwidth)[0]
        offsets = SubChanId._offsets.get(main_channel_bandwidth)
        if offsets is None:
            return None
        offsets = offsets.get(sub_channel_bandwidth)
        if offsets is None:
            return None
        return (main_channel + offsets[sub_chan_id]) if sub_chan_id in offsets else None

    @staticmethod
    def sub_chan_bandwidth(main_channel_bandwidth, sub_chan_id):
        """ Subchannel bandwidth

        Arguments:
        main_channel_bandwidth -- Main channel bandwidth (see Bandwidth enum)
        sub_chan_id            -- Subchannel ID (see SubChanId enum)
        Returns                -- Subchannel bandwidth or None
        """
        offsets = SubChanId._offsets.get(main_channel_bandwidth)
        if offsets is None:
            return None
        for bandwidth in sorted(offsets.keys()):
            if sub_chan_id in offsets[bandwidth]:
                return bandwidth
        return None

    @staticmethod
    def all():
        """ Returns all subchannel IDs """
        return sorted(SubChanId.name.keys())

# =============================================================================


class RateInfo(ClmUtils.EqualityMixin):
    """ Information about rate or rate group (something that has one name)

    Attributes:
    name                -- Rate name as stored in XML
    vht_name            -- Name in VHT notation (defined for MCS rates only)
    blob_index          -- Rate name as stored in blob
    index               -- 0-based rate index in global rate table
    rate_type           -- DSSS/OFDM/MCS/VHT
    rate_split_bin      -- Index of rate set bin to use when rate set splitting
                           is performed
    chains              -- Number of TX chains
    modulation_type     -- String that represents modulation with index (or
                           index range) replaced by %s
    modulation_index    -- Index inside modulation (integer)
    singular            -- True for singular rate name, False for group name
    txbf                -- TXBF rate
    vht_txbf0           -- VHT TXBF0 rate
    is_1024_qam         -- 1024 QAM (VHT9-11) rate
    group               -- Rate group
    """
    # Length of sequence of rates that differ only by OFDM modulation
    NUM_MCS_MODS = 8

    class RateType:
        """ Rate type enum """
        DSSS, OFDM, MCS, VHT = range(4)

    def __init__(self, name, blob_index, index):
        """ Constructor

        Arguments:
        name        -- XML name
        blob_index  -- BLOB name
        index       -- sequential index in global rate table
        """
        self.name = name
        self.blob_index = blob_index
        self.index = index
        self.rate_type = re.search(r"^(DSSS)|(OFDM)|(MCS)|(VHT)", name).lastindex - 1

        self.modulation_type = re.sub(r"^([A-Z]+)(\d+(\-\d+)?)", r"\1%s", name)
        self.singular = bool(blob_index)
        if blob_index:
            self.modulation_index = int(re.match(r"[A-Z]+(\d+)", name).group(1))
        else:
            self.modulation_index = None

        self.txbf = "_TXBF" in name
        if self.rate_type == RateInfo.RateType.MCS:
            m = re.search(r"^MCS(\d+)", name)
            self.chains = 1 + (int(m.group(1)) / 8)
        elif self.rate_type == RateInfo.RateType.VHT:
            m = re.search(r"^VHT\d+(\-\d+)?SS(\d+)", name)
            self.chains = int(m.group(2))
        else:
            self.chains = 1
        if re.search("STBC", name):
            self.chains = self.chains + 1
        self.rate_split_bin = min(self.chains, 3) - 1
        m = re.search(r"(CDD|MULTI|SPEXP|TXBF)(\d)", name)
        if m:
            self.chains += int(m.group(2))
        if self.chains < 3:
            self.rate_split_bin += 10 * self.chains
        self.vht_txbf0 = (self.rate_type == RateInfo.RateType.VHT) and ("_TXBF0" in name)
        self.is_1024_qam = re.match(r"VHT(10|11)", name) is not None
        if self.rate_type == RateInfo.RateType.MCS:
            m = re.search(r"^MCS(\d+)(\-(\d+))?(.*)$", name)
            idx1 = int(m.group(1))
            self.vht_name = "VHT%d%sSS%d%s" % (idx1 % RateInfo.NUM_MCS_MODS,
                                               ("-%d" % (int(m.group(3)) % RateInfo.NUM_MCS_MODS)) if m.group(3) else "",
                                               (idx1 / 8) + 1,
                                               m.group(4))

    def __hash__(self):
        """ Hash for use as dictionary key and set element """
        return self.name.__hash__()

    def __str__(self):
        """ String representation """
        return self.name

    def __repr__(self):
        """ Debugger representation """
        return "<%s>" % self.name

    def __eq__(self, other):
        """ Equality comparison """
        return isinstance(other, RateInfo) and (self.name == other.name)

    def expand(self):
        """ Returns list of singular rate XML names, corresponding to this rate

        For singular rate info returns single-element list, for group rate info
        returns list of names belonging to group
        """
        if re.search(r"^[A-Z]+($|_)", self.name):
            indices = {RateInfo.RateType.DSSS: (1, 2, 5, 11),
                       RateInfo.RateType.OFDM: (6, 9, 12, 18, 24, 36, 48, 54)}
            return [re.sub(r"(^[A-Z]+)", r"\g<1>" + str(idx), self.name) for idx in indices[self.rate_type]]
        m = re.search(r"^(MCS|VHT)(\d+)\-(\d+)", self.name)
        if m:
            return [re.sub(r"(\d+)\-(\d+)", str(idx), self.name) for idx in range(int(m.group(2)), int(m.group(3)) + 1)]
        return [self.name]

    def sort_key(self):
        """ Key for rates' sorting """
        return self.index

# =============================================================================


class RatesInfo:
    """ Static functions for accessing collection of named rates """

    # Sequence of pairs (XMLRateName, BLOBRateName)
    _RATES = (
        ("DSSS1",                   "WL_RATE_1X1_DSSS_1"),
        ("DSSS2",                   "WL_RATE_1X1_DSSS_2"),
        ("DSSS5",                   "WL_RATE_1X1_DSSS_5_5"),
        ("DSSS11",                  "WL_RATE_1X1_DSSS_11"),
        ("OFDM6",                   "WL_RATE_1X1_OFDM_6"),
        ("OFDM9",                   "WL_RATE_1X1_OFDM_9"),
        ("OFDM12",                  "WL_RATE_1X1_OFDM_12"),
        ("OFDM18",                  "WL_RATE_1X1_OFDM_18"),
        ("OFDM24",                  "WL_RATE_1X1_OFDM_24"),
        ("OFDM36",                  "WL_RATE_1X1_OFDM_36"),
        ("OFDM48",                  "WL_RATE_1X1_OFDM_48"),
        ("OFDM54",                  "WL_RATE_1X1_OFDM_54"),
        ("MCS0",                    "WL_RATE_1X1_MCS0"),
        ("MCS1",                    "WL_RATE_1X1_MCS1"),
        ("MCS2",                    "WL_RATE_1X1_MCS2"),
        ("MCS3",                    "WL_RATE_1X1_MCS3"),
        ("MCS4",                    "WL_RATE_1X1_MCS4"),
        ("MCS5",                    "WL_RATE_1X1_MCS5"),
        ("MCS6",                    "WL_RATE_1X1_MCS6"),
        ("MCS7",                    "WL_RATE_1X1_MCS7"),
        ("VHT8SS1",                 "WL_RATE_1X1_VHT8SS1"),
        ("VHT9SS1",                 "WL_RATE_1X1_VHT9SS1"),
        ("DSSS1_MULTI1",            "WL_RATE_1X2_DSSS_1"),
        ("DSSS2_MULTI1",            "WL_RATE_1X2_DSSS_2"),
        ("DSSS5_MULTI1",            "WL_RATE_1X2_DSSS_5_5"),
        ("DSSS11_MULTI1",           "WL_RATE_1X2_DSSS_11 "),
        ("OFDM6_CDD1",              "WL_RATE_1X2_CDD_OFDM_6"),
        ("OFDM9_CDD1",              "WL_RATE_1X2_CDD_OFDM_9"),
        ("OFDM12_CDD1",             "WL_RATE_1X2_CDD_OFDM_12"),
        ("OFDM18_CDD1",             "WL_RATE_1X2_CDD_OFDM_18"),
        ("OFDM24_CDD1",             "WL_RATE_1X2_CDD_OFDM_24"),
        ("OFDM36_CDD1",             "WL_RATE_1X2_CDD_OFDM_36"),
        ("OFDM48_CDD1",             "WL_RATE_1X2_CDD_OFDM_48"),
        ("OFDM54_CDD1",             "WL_RATE_1X2_CDD_OFDM_54"),
        ("MCS0_CDD1",               "WL_RATE_1X2_CDD_MCS0"),
        ("MCS1_CDD1",               "WL_RATE_1X2_CDD_MCS1"),
        ("MCS2_CDD1",               "WL_RATE_1X2_CDD_MCS2"),
        ("MCS3_CDD1",               "WL_RATE_1X2_CDD_MCS3"),
        ("MCS4_CDD1",               "WL_RATE_1X2_CDD_MCS4"),
        ("MCS5_CDD1",               "WL_RATE_1X2_CDD_MCS5"),
        ("MCS6_CDD1",               "WL_RATE_1X2_CDD_MCS6"),
        ("MCS7_CDD1",               "WL_RATE_1X2_CDD_MCS7"),
        ("VHT8SS1_CDD1",            "WL_RATE_1X2_VHT8SS1"),
        ("VHT9SS1_CDD1",            "WL_RATE_1X2_VHT9SS1"),
        ("MCS0_STBC",               "WL_RATE_2X2_STBC_MCS0"),
        ("MCS1_STBC",               "WL_RATE_2X2_STBC_MCS1"),
        ("MCS2_STBC",               "WL_RATE_2X2_STBC_MCS2"),
        ("MCS3_STBC",               "WL_RATE_2X2_STBC_MCS3"),
        ("MCS4_STBC",               "WL_RATE_2X2_STBC_MCS4"),
        ("MCS5_STBC",               "WL_RATE_2X2_STBC_MCS5"),
        ("MCS6_STBC",               "WL_RATE_2X2_STBC_MCS6"),
        ("MCS7_STBC",               "WL_RATE_2X2_STBC_MCS7"),
        ("VHT8SS1_STBC",            "WL_RATE_2X2_STBC_VHT8SS1"),
        ("VHT9SS1_STBC",            "WL_RATE_2X2_STBC_VHT9SS1"),
        ("MCS8",                    "WL_RATE_2X2_SDM_MCS8"),
        ("MCS9",                    "WL_RATE_2X2_SDM_MCS9"),
        ("MCS10",                   "WL_RATE_2X2_SDM_MCS10"),
        ("MCS11",                   "WL_RATE_2X2_SDM_MCS11"),
        ("MCS12",                   "WL_RATE_2X2_SDM_MCS12"),
        ("MCS13",                   "WL_RATE_2X2_SDM_MCS13"),
        ("MCS14",                   "WL_RATE_2X2_SDM_MCS14"),
        ("MCS15",                   "WL_RATE_2X2_SDM_MCS15"),
        ("VHT8SS2",                 "WL_RATE_2X2_VHT8SS2"),
        ("VHT9SS2",                 "WL_RATE_2X2_VHT9SS2"),
        ("DSSS1_MULTI2",            "WL_RATE_1X3_DSSS_1"),
        ("DSSS2_MULTI2",            "WL_RATE_1X3_DSSS_2"),
        ("DSSS5_MULTI2",            "WL_RATE_1X3_DSSS_5_5"),
        ("DSSS11_MULTI2",           "WL_RATE_1X3_DSSS_11"),
        ("OFDM6_CDD2",              "WL_RATE_1X3_CDD_OFDM_6"),
        ("OFDM9_CDD2",              "WL_RATE_1X3_CDD_OFDM_9"),
        ("OFDM12_CDD2",             "WL_RATE_1X3_CDD_OFDM_12"),
        ("OFDM18_CDD2",             "WL_RATE_1X3_CDD_OFDM_18"),
        ("OFDM24_CDD2",             "WL_RATE_1X3_CDD_OFDM_24"),
        ("OFDM36_CDD2",             "WL_RATE_1X3_CDD_OFDM_36"),
        ("OFDM48_CDD2",             "WL_RATE_1X3_CDD_OFDM_48"),
        ("OFDM54_CDD2",             "WL_RATE_1X3_CDD_OFDM_54"),
        ("MCS0_CDD2",               "WL_RATE_1X3_CDD_MCS0"),
        ("MCS1_CDD2",               "WL_RATE_1X3_CDD_MCS1"),
        ("MCS2_CDD2",               "WL_RATE_1X3_CDD_MCS2"),
        ("MCS3_CDD2",               "WL_RATE_1X3_CDD_MCS3"),
        ("MCS4_CDD2",               "WL_RATE_1X3_CDD_MCS4"),
        ("MCS5_CDD2",               "WL_RATE_1X3_CDD_MCS5"),
        ("MCS6_CDD2",               "WL_RATE_1X3_CDD_MCS6"),
        ("MCS7_CDD2",               "WL_RATE_1X3_CDD_MCS7"),
        ("VHT8SS1_CDD2",            "WL_RATE_1X3_VHT8SS1"),
        ("VHT9SS1_CDD2",            "WL_RATE_1X3_VHT9SS1"),
        ("MCS0_STBC_SPEXP1",        "WL_RATE_2X3_STBC_MCS0"),
        ("MCS1_STBC_SPEXP1",        "WL_RATE_2X3_STBC_MCS1"),
        ("MCS2_STBC_SPEXP1",        "WL_RATE_2X3_STBC_MCS2"),
        ("MCS3_STBC_SPEXP1",        "WL_RATE_2X3_STBC_MCS3"),
        ("MCS4_STBC_SPEXP1",        "WL_RATE_2X3_STBC_MCS4"),
        ("MCS5_STBC_SPEXP1",        "WL_RATE_2X3_STBC_MCS5"),
        ("MCS6_STBC_SPEXP1",        "WL_RATE_2X3_STBC_MCS6"),
        ("MCS7_STBC_SPEXP1",        "WL_RATE_2X3_STBC_MCS7"),
        ("VHT8SS1_STBC_SPEXP1",     "WL_RATE_2X3_STBC_VHT8SS1"),
        ("VHT9SS1_STBC_SPEXP1",     "WL_RATE_2X3_STBC_VHT9SS1"),
        ("MCS8_SPEXP1",             "WL_RATE_2X3_SDM_MCS8"),
        ("MCS9_SPEXP1",             "WL_RATE_2X3_SDM_MCS9"),
        ("MCS10_SPEXP1",            "WL_RATE_2X3_SDM_MCS10"),
        ("MCS11_SPEXP1",            "WL_RATE_2X3_SDM_MCS11"),
        ("MCS12_SPEXP1",            "WL_RATE_2X3_SDM_MCS12"),
        ("MCS13_SPEXP1",            "WL_RATE_2X3_SDM_MCS13"),
        ("MCS14_SPEXP1",            "WL_RATE_2X3_SDM_MCS14"),
        ("MCS15_SPEXP1",            "WL_RATE_2X3_SDM_MCS15"),
        ("VHT8SS2_SPEXP1",          "WL_RATE_2X3_VHT8SS2"),
        ("VHT9SS2_SPEXP1",          "WL_RATE_2X3_VHT9SS2"),
        ("MCS16",                   "WL_RATE_3X3_SDM_MCS16"),
        ("MCS17",                   "WL_RATE_3X3_SDM_MCS17"),
        ("MCS18",                   "WL_RATE_3X3_SDM_MCS18"),
        ("MCS19",                   "WL_RATE_3X3_SDM_MCS19"),
        ("MCS20",                   "WL_RATE_3X3_SDM_MCS20"),
        ("MCS21",                   "WL_RATE_3X3_SDM_MCS21"),
        ("MCS22",                   "WL_RATE_3X3_SDM_MCS22"),
        ("MCS23",                   "WL_RATE_3X3_SDM_MCS23"),
        ("VHT8SS3",                 "WL_RATE_3X3_VHT8SS3"),
        ("VHT9SS3",                 "WL_RATE_3X3_VHT9SS3"),
        ("OFDM6_TXBF1",             "WL_RATE_1X2_TXBF_OFDM_6"),
        ("OFDM9_TXBF1",             "WL_RATE_1X2_TXBF_OFDM_9"),
        ("OFDM12_TXBF1",            "WL_RATE_1X2_TXBF_OFDM_12"),
        ("OFDM18_TXBF1",            "WL_RATE_1X2_TXBF_OFDM_18"),
        ("OFDM24_TXBF1",            "WL_RATE_1X2_TXBF_OFDM_24"),
        ("OFDM36_TXBF1",            "WL_RATE_1X2_TXBF_OFDM_36"),
        ("OFDM48_TXBF1",            "WL_RATE_1X2_TXBF_OFDM_48"),
        ("OFDM54_TXBF1",            "WL_RATE_1X2_TXBF_OFDM_54"),
        ("MCS0_TXBF1",              "WL_RATE_1X2_TXBF_MCS0"),
        ("MCS1_TXBF1",              "WL_RATE_1X2_TXBF_MCS1"),
        ("MCS2_TXBF1",              "WL_RATE_1X2_TXBF_MCS2"),
        ("MCS3_TXBF1",              "WL_RATE_1X2_TXBF_MCS3"),
        ("MCS4_TXBF1",              "WL_RATE_1X2_TXBF_MCS4"),
        ("MCS5_TXBF1",              "WL_RATE_1X2_TXBF_MCS5"),
        ("MCS6_TXBF1",              "WL_RATE_1X2_TXBF_MCS6"),
        ("MCS7_TXBF1",              "WL_RATE_1X2_TXBF_MCS7"),
        ("VHT8SS1_TXBF1",           "WL_RATE_1X2_TXBF_VHT8SS1"),
        ("VHT9SS1_TXBF1",           "WL_RATE_1X2_TXBF_VHT9SS1"),
        ("MCS8_TXBF0",              "WL_RATE_2X2_TXBF_SDM_MCS8"),
        ("MCS9_TXBF0",              "WL_RATE_2X2_TXBF_SDM_MCS9"),
        ("MCS10_TXBF0",             "WL_RATE_2X2_TXBF_SDM_MCS10"),
        ("MCS11_TXBF0",             "WL_RATE_2X2_TXBF_SDM_MCS11"),
        ("MCS12_TXBF0",             "WL_RATE_2X2_TXBF_SDM_MCS12"),
        ("MCS13_TXBF0",             "WL_RATE_2X2_TXBF_SDM_MCS13"),
        ("MCS14_TXBF0",             "WL_RATE_2X2_TXBF_SDM_MCS14"),
        ("MCS15_TXBF0",             "WL_RATE_2X2_TXBF_SDM_MCS15"),
        ("OFDM6_TXBF2",             "WL_RATE_1X3_TXBF_OFDM_6"),
        ("OFDM9_TXBF2",             "WL_RATE_1X3_TXBF_OFDM_9"),
        ("OFDM12_TXBF2",            "WL_RATE_1X3_TXBF_OFDM_12"),
        ("OFDM18_TXBF2",            "WL_RATE_1X3_TXBF_OFDM_18"),
        ("OFDM24_TXBF2",            "WL_RATE_1X3_TXBF_OFDM_24"),
        ("OFDM36_TXBF2",            "WL_RATE_1X3_TXBF_OFDM_36"),
        ("OFDM48_TXBF2",            "WL_RATE_1X3_TXBF_OFDM_48"),
        ("OFDM54_TXBF2",            "WL_RATE_1X3_TXBF_OFDM_54"),
        ("MCS0_TXBF2",              "WL_RATE_1X3_TXBF_MCS0"),
        ("MCS1_TXBF2",              "WL_RATE_1X3_TXBF_MCS1"),
        ("MCS2_TXBF2",              "WL_RATE_1X3_TXBF_MCS2"),
        ("MCS3_TXBF2",              "WL_RATE_1X3_TXBF_MCS3"),
        ("MCS4_TXBF2",              "WL_RATE_1X3_TXBF_MCS4"),
        ("MCS5_TXBF2",              "WL_RATE_1X3_TXBF_MCS5"),
        ("MCS6_TXBF2",              "WL_RATE_1X3_TXBF_MCS6"),
        ("MCS7_TXBF2",              "WL_RATE_1X3_TXBF_MCS7"),
        ("VHT8SS1_TXBF2",           "WL_RATE_1X3_TXBF_VHT8SS1"),
        ("VHT9SS1_TXBF2",           "WL_RATE_1X3_TXBF_VHT9SS1"),
        ("MCS8_TXBF1",              "WL_RATE_2X3_TXBF_SDM_MCS8"),
        ("MCS9_TXBF1",              "WL_RATE_2X3_TXBF_SDM_MCS9"),
        ("MCS10_TXBF1",             "WL_RATE_2X3_TXBF_SDM_MCS10"),
        ("MCS11_TXBF1",             "WL_RATE_2X3_TXBF_SDM_MCS11"),
        ("MCS12_TXBF1",             "WL_RATE_2X3_TXBF_SDM_MCS12"),
        ("MCS13_TXBF1",             "WL_RATE_2X3_TXBF_SDM_MCS13"),
        ("MCS14_TXBF1",             "WL_RATE_2X3_TXBF_SDM_MCS14"),
        ("MCS15_TXBF1",             "WL_RATE_2X3_TXBF_SDM_MCS15"),
        ("VHT8SS2_TXBF1",           "WL_RATE_2X3_TXBF_VHT8SS2"),
        ("VHT9SS2_TXBF1",           "WL_RATE_2X3_TXBF_VHT9SS2"),
        ("MCS16_TXBF0",             "WL_RATE_3X3_TXBF_SDM_MCS16"),
        ("MCS17_TXBF0",             "WL_RATE_3X3_TXBF_SDM_MCS17"),
        ("MCS18_TXBF0",             "WL_RATE_3X3_TXBF_SDM_MCS18"),
        ("MCS19_TXBF0",             "WL_RATE_3X3_TXBF_SDM_MCS19"),
        ("MCS20_TXBF0",             "WL_RATE_3X3_TXBF_SDM_MCS20"),
        ("MCS21_TXBF0",             "WL_RATE_3X3_TXBF_SDM_MCS21"),
        ("MCS22_TXBF0",             "WL_RATE_3X3_TXBF_SDM_MCS22"),
        ("MCS23_TXBF0",             "WL_RATE_3X3_TXBF_SDM_MCS23"),
        ("VHT10SS1",                "WL_RATE_P_1X1_VHT10SS1"),
        ("VHT11SS1",                "WL_RATE_P_1X1_VHT11SS1"),
        ("VHT10SS1_CDD1",           "WL_RATE_P_1X2_VHT10SS1"),
        ("VHT11SS1_CDD1",           "WL_RATE_P_1X2_VHT11SS1"),
        ("VHT10SS1_STBC",           "WL_RATE_P_2X2_STBC_VHT10SS1"),
        ("VHT11SS1_STBC",           "WL_RATE_P_2X2_STBC_VHT11SS1"),
        ("VHT10SS1_TXBF1",          "WL_RATE_P_1X2_TXBF_VHT10SS1"),
        ("VHT11SS1_TXBF1",          "WL_RATE_P_1X2_TXBF_VHT11SS1"),
        ("VHT10SS2",                "WL_RATE_P_2X2_VHT10SS2"),
        ("VHT11SS2",                "WL_RATE_P_2X2_VHT11SS2"),
        ("VHT8SS2_TXBF0",           "WL_RATE_2X2_TXBF_VHT8SS2"),
        ("VHT9SS2_TXBF0",           "WL_RATE_2X2_TXBF_VHT9SS2"),
        ("VHT10SS2_TXBF0",          "WL_RATE_P_2X2_TXBF_VHT10SS2"),
        ("VHT11SS2_TXBF0",          "WL_RATE_P_2X2_TXBF_VHT11SS2"),
        ("VHT10SS1_CDD2",           "WL_RATE_P_1X3_VHT10SS1"),
        ("VHT11SS1_CDD2",           "WL_RATE_P_1X3_VHT11SS1"),
        ("VHT10SS1_STBC_SPEXP1",    "WL_RATE_P_2X3_STBC_VHT10SS1"),
        ("VHT11SS1_STBC_SPEXP1",    "WL_RATE_P_2X3_STBC_VHT11SS1"),
        ("VHT10SS1_TXBF2",          "WL_RATE_P_1X3_TXBF_VHT10SS1"),
        ("VHT11SS1_TXBF2",          "WL_RATE_P_1X3_TXBF_VHT11SS1"),
        ("VHT10SS2_SPEXP1",         "WL_RATE_P_2X3_VHT10SS2"),
        ("VHT11SS2_SPEXP1",         "WL_RATE_P_2X3_VHT11SS2"),
        ("VHT10SS2_TXBF1",          "WL_RATE_P_2X3_TXBF_VHT10SS2"),
        ("VHT11SS2_TXBF1",          "WL_RATE_P_2X3_TXBF_VHT11SS2"),
        ("VHT10SS3",                "WL_RATE_P_3X3_VHT10SS3"),
        ("VHT11SS3",                "WL_RATE_P_3X3_VHT11SS3"),
        ("VHT8SS3_TXBF0",           "WL_RATE_3X3_TXBF_VHT8SS3"),
        ("VHT9SS3_TXBF0",           "WL_RATE_3X3_TXBF_VHT9SS3"),
        ("VHT10SS3_TXBF0",          "WL_RATE_P_3X3_TXBF_VHT10SS3"),
        ("VHT11SS3_TXBF0",          "WL_RATE_P_3X3_TXBF_VHT11SS3"),
        ("DSSS1_MULTI3",            "WL_RATE_1X4_DSSS_1"),
        ("DSSS2_MULTI3",            "WL_RATE_1X4_DSSS_2"),
        ("DSSS5_MULTI3",            "WL_RATE_1X4_DSSS_5_5"),
        ("DSSS11_MULTI3",           "WL_RATE_1X4_DSSS_11"),
        ("OFDM6_CDD3",              "WL_RATE_1X4_CDD_OFDM_6"),
        ("OFDM9_CDD3",              "WL_RATE_1X4_CDD_OFDM_9"),
        ("OFDM12_CDD3",             "WL_RATE_1X4_CDD_OFDM_12"),
        ("OFDM18_CDD3",             "WL_RATE_1X4_CDD_OFDM_18"),
        ("OFDM24_CDD3",             "WL_RATE_1X4_CDD_OFDM_24"),
        ("OFDM36_CDD3",             "WL_RATE_1X4_CDD_OFDM_36"),
        ("OFDM48_CDD3",             "WL_RATE_1X4_CDD_OFDM_48"),
        ("OFDM54_CDD3",             "WL_RATE_1X4_CDD_OFDM_54"),
        ("OFDM6_TXBF3",             "WL_RATE_1X4_TXBF_OFDM_6"),
        ("OFDM9_TXBF3",             "WL_RATE_1X4_TXBF_OFDM_9"),
        ("OFDM12_TXBF3",            "WL_RATE_1X4_TXBF_OFDM_12"),
        ("OFDM18_TXBF3",            "WL_RATE_1X4_TXBF_OFDM_18"),
        ("OFDM24_TXBF3",            "WL_RATE_1X4_TXBF_OFDM_24"),
        ("OFDM36_TXBF3",            "WL_RATE_1X4_TXBF_OFDM_36"),
        ("OFDM48_TXBF3",            "WL_RATE_1X4_TXBF_OFDM_48"),
        ("OFDM54_TXBF3",            "WL_RATE_1X4_TXBF_OFDM_54"),
        ("MCS0_CDD3",               "WL_RATE_1X4_CDD_MCS0"),
        ("MCS1_CDD3",               "WL_RATE_1X4_CDD_MCS1"),
        ("MCS2_CDD3",               "WL_RATE_1X4_CDD_MCS2"),
        ("MCS3_CDD3",               "WL_RATE_1X4_CDD_MCS3"),
        ("MCS4_CDD3",               "WL_RATE_1X4_CDD_MCS4"),
        ("MCS5_CDD3",               "WL_RATE_1X4_CDD_MCS5"),
        ("MCS6_CDD3",               "WL_RATE_1X4_CDD_MCS6"),
        ("MCS7_CDD3",               "WL_RATE_1X4_CDD_MCS7"),
        ("MCS0_STBC_SPEXP2",        "WL_RATE_2X4_STBC_MCS0"),
        ("MCS1_STBC_SPEXP2",        "WL_RATE_2X4_STBC_MCS1"),
        ("MCS2_STBC_SPEXP2",        "WL_RATE_2X4_STBC_MCS2"),
        ("MCS3_STBC_SPEXP2",        "WL_RATE_2X4_STBC_MCS3"),
        ("MCS4_STBC_SPEXP2",        "WL_RATE_2X4_STBC_MCS4"),
        ("MCS5_STBC_SPEXP2",        "WL_RATE_2X4_STBC_MCS5"),
        ("MCS6_STBC_SPEXP2",        "WL_RATE_2X4_STBC_MCS6"),
        ("MCS7_STBC_SPEXP2",        "WL_RATE_2X4_STBC_MCS7"),
        ("MCS0_TXBF3",              "WL_RATE_1X4_TXBF_MCS0"),
        ("MCS1_TXBF3",              "WL_RATE_1X4_TXBF_MCS1"),
        ("MCS2_TXBF3",              "WL_RATE_1X4_TXBF_MCS2"),
        ("MCS3_TXBF3",              "WL_RATE_1X4_TXBF_MCS3"),
        ("MCS4_TXBF3",              "WL_RATE_1X4_TXBF_MCS4"),
        ("MCS5_TXBF3",              "WL_RATE_1X4_TXBF_MCS5"),
        ("MCS6_TXBF3",              "WL_RATE_1X4_TXBF_MCS6"),
        ("MCS7_TXBF3",              "WL_RATE_1X4_TXBF_MCS7"),
        ("VHT8SS1_CDD3",            "WL_RATE_1X4_VHT8SS1"),
        ("VHT9SS1_CDD3",            "WL_RATE_1X4_VHT9SS1"),
        ("VHT10SS1_CDD3",           "WL_RATE_P_1X4_VHT10SS1"),
        ("VHT11SS1_CDD3",           "WL_RATE_P_1X4_VHT11SS1"),
        ("VHT8SS1_STBC_SPEXP2",     "WL_RATE_2X4_STBC_VHT8SS1"),
        ("VHT9SS1_STBC_SPEXP2",     "WL_RATE_2X4_STBC_VHT9SS1"),
        ("VHT10SS1_STBC_SPEXP2",    "WL_RATE_P_2X4_STBC_VHT10SS1"),
        ("VHT11SS1_STBC_SPEXP2",    "WL_RATE_P_2X4_STBC_VHT11SS1"),
        ("VHT8SS1_TXBF3",           "WL_RATE_1X4_TXBF_VHT8SS1"),
        ("VHT9SS1_TXBF3",           "WL_RATE_1X4_TXBF_VHT9SS1"),
        ("VHT10SS1_TXBF3",          "WL_RATE_P_1X4_TXBF_VHT10SS1"),
        ("VHT11SS1_TXBF3",          "WL_RATE_P_1X4_TXBF_VHT11SS1"),
        ("MCS8_SPEXP2",             "WL_RATE_2X4_SDM_MCS8"),
        ("MCS9_SPEXP2",             "WL_RATE_2X4_SDM_MCS9"),
        ("MCS10_SPEXP2",            "WL_RATE_2X4_SDM_MCS10"),
        ("MCS11_SPEXP2",            "WL_RATE_2X4_SDM_MCS11"),
        ("MCS12_SPEXP2",            "WL_RATE_2X4_SDM_MCS12"),
        ("MCS13_SPEXP2",            "WL_RATE_2X4_SDM_MCS13"),
        ("MCS14_SPEXP2",            "WL_RATE_2X4_SDM_MCS14"),
        ("MCS15_SPEXP2",            "WL_RATE_2X4_SDM_MCS15"),
        ("MCS8_TXBF2",              "WL_RATE_2X4_TXBF_SDM_MCS8"),
        ("MCS9_TXBF2",              "WL_RATE_2X4_TXBF_SDM_MCS9"),
        ("MCS10_TXBF2",             "WL_RATE_2X4_TXBF_SDM_MCS10"),
        ("MCS11_TXBF2",             "WL_RATE_2X4_TXBF_SDM_MCS11"),
        ("MCS12_TXBF2",             "WL_RATE_2X4_TXBF_SDM_MCS12"),
        ("MCS13_TXBF2",             "WL_RATE_2X4_TXBF_SDM_MCS13"),
        ("MCS14_TXBF2",             "WL_RATE_2X4_TXBF_SDM_MCS14"),
        ("MCS15_TXBF2",             "WL_RATE_2X4_TXBF_SDM_MCS15"),
        ("VHT8SS2_SPEXP2",          "WL_RATE_2X4_VHT8SS2"),
        ("VHT9SS2_SPEXP2",          "WL_RATE_2X4_VHT9SS2"),
        ("VHT10SS2_SPEXP2",         "WL_RATE_P_2X4_VHT10SS2"),
        ("VHT11SS2_SPEXP2",         "WL_RATE_P_2X4_VHT11SS2"),
        ("VHT8SS2_TXBF2",           "WL_RATE_2X4_TXBF_VHT8SS2"),
        ("VHT9SS2_TXBF2",           "WL_RATE_2X4_TXBF_VHT9SS2"),
        ("VHT10SS2_TXBF2",          "WL_RATE_P_2X4_TXBF_VHT10SS2"),
        ("VHT11SS2_TXBF2",          "WL_RATE_P_2X4_TXBF_VHT11SS2"),
        ("MCS16_SPEXP1",            "WL_RATE_3X4_SDM_MCS16"),
        ("MCS17_SPEXP1",            "WL_RATE_3X4_SDM_MCS17"),
        ("MCS18_SPEXP1",            "WL_RATE_3X4_SDM_MCS18"),
        ("MCS19_SPEXP1",            "WL_RATE_3X4_SDM_MCS19"),
        ("MCS20_SPEXP1",            "WL_RATE_3X4_SDM_MCS20"),
        ("MCS21_SPEXP1",            "WL_RATE_3X4_SDM_MCS21"),
        ("MCS22_SPEXP1",            "WL_RATE_3X4_SDM_MCS22"),
        ("MCS23_SPEXP1",            "WL_RATE_3X4_SDM_MCS23"),
        ("MCS16_TXBF1",             "WL_RATE_3X4_TXBF_SDM_MCS16"),
        ("MCS17_TXBF1",             "WL_RATE_3X4_TXBF_SDM_MCS17"),
        ("MCS18_TXBF1",             "WL_RATE_3X4_TXBF_SDM_MCS18"),
        ("MCS19_TXBF1",             "WL_RATE_3X4_TXBF_SDM_MCS19"),
        ("MCS20_TXBF1",             "WL_RATE_3X4_TXBF_SDM_MCS20"),
        ("MCS21_TXBF1",             "WL_RATE_3X4_TXBF_SDM_MCS21"),
        ("MCS22_TXBF1",             "WL_RATE_3X4_TXBF_SDM_MCS22"),
        ("MCS23_TXBF1",             "WL_RATE_3X4_TXBF_SDM_MCS23"),
        ("VHT8SS3_SPEXP1",          "WL_RATE_3X4_VHT8SS3"),
        ("VHT9SS3_SPEXP1",          "WL_RATE_3X4_VHT9SS3"),
        ("VHT10SS3_SPEXP1",         "WL_RATE_P_3X4_VHT10SS3"),
        ("VHT11SS3_SPEXP1",         "WL_RATE_P_3X4_VHT11SS3"),
        ("VHT8SS3_TXBF1",           "WL_RATE_P_3X4_TXBF_VHT8SS3"),
        ("VHT9SS3_TXBF1",           "WL_RATE_P_3X4_TXBF_VHT9SS3"),
        ("VHT10SS3_TXBF1",          "WL_RATE_P_3X4_TXBF_VHT10SS3"),
        ("VHT11SS3_TXBF1",          "WL_RATE_P_3X4_TXBF_VHT11SS3"),
        ("MCS24",                   "WL_RATE_4X4_SDM_MCS24"),
        ("MCS25",                   "WL_RATE_4X4_SDM_MCS25"),
        ("MCS26",                   "WL_RATE_4X4_SDM_MCS26"),
        ("MCS27",                   "WL_RATE_4X4_SDM_MCS27"),
        ("MCS28",                   "WL_RATE_4X4_SDM_MCS28"),
        ("MCS29",                   "WL_RATE_4X4_SDM_MCS29"),
        ("MCS30",                   "WL_RATE_4X4_SDM_MCS30"),
        ("MCS31",                   "WL_RATE_4X4_SDM_MCS31"),
        ("MCS24_TXBF0",             "WL_RATE_4X4_TXBF_SDM_MCS24"),
        ("MCS25_TXBF0",             "WL_RATE_4X4_TXBF_SDM_MCS25"),
        ("MCS26_TXBF0",             "WL_RATE_4X4_TXBF_SDM_MCS26"),
        ("MCS27_TXBF0",             "WL_RATE_4X4_TXBF_SDM_MCS27"),
        ("MCS28_TXBF0",             "WL_RATE_4X4_TXBF_SDM_MCS28"),
        ("MCS29_TXBF0",             "WL_RATE_4X4_TXBF_SDM_MCS29"),
        ("MCS30_TXBF0",             "WL_RATE_4X4_TXBF_SDM_MCS30"),
        ("MCS31_TXBF0",             "WL_RATE_4X4_TXBF_SDM_MCS31"),
        ("VHT8SS4",                 "WL_RATE_4X4_VHT8SS4"),
        ("VHT9SS4",                 "WL_RATE_4X4_VHT9SS4"),
        ("VHT10SS4",                "WL_RATE_P_4X4_VHT10SS4"),
        ("VHT11SS4",                "WL_RATE_P_4X4_VHT11SS4"),
        ("VHT8SS4_TXBF0",           "WL_RATE_P_4X4_TXBF_VHT8SS4"),
        ("VHT9SS4_TXBF0",           "WL_RATE_P_4X4_TXBF_VHT9SS4"),
        ("VHT10SS4_TXBF0",          "WL_RATE_P_4X4_TXBF_VHT10SS4"),
        ("VHT11SS4_TXBF0",          "WL_RATE_P_4X4_TXBF_VHT11SS4"),
        ("DSSS",                    ""),
        ("OFDM",                    ""),
        ("MCS0-7",                  ""),
        ("VHT8-9SS1",               ""),
        ("VHT10-11SS1",             ""),
        ("DSSS_MULTI1",             ""),
        ("OFDM_CDD1",               ""),
        ("OFDM_TXBF1",              ""),
        ("MCS0-7_CDD1",             ""),
        ("VHT8-9SS1_CDD1",          ""),
        ("VHT10-11SS1_CDD1",        ""),
        ("MCS0-7_STBC",             ""),
        ("VHT8-9SS1_STBC",          ""),
        ("VHT10-11SS1_STBC",        ""),
        ("MCS0-7_TXBF1",            ""),
        ("VHT8-9SS1_TXBF1",         ""),
        ("VHT10-11SS1_TXBF1",       ""),
        ("MCS8-15",                 ""),
        ("VHT8-9SS2",               ""),
        ("VHT10-11SS2",             ""),
        ("MCS8-15_TXBF0",           ""),
        ("VHT8-9SS2_TXBF0",         ""),
        ("VHT10-11SS2_TXBF0",       ""),
        ("DSSS_MULTI2",             ""),
        ("OFDM_CDD2",               ""),
        ("OFDM_TXBF2",              ""),
        ("MCS0-7_CDD2",             ""),
        ("VHT8-9SS1_CDD2",          ""),
        ("VHT10-11SS1_CDD2",        ""),
        ("MCS0-7_STBC_SPEXP1",      ""),
        ("VHT8-9SS1_STBC_SPEXP1",   ""),
        ("VHT10-11SS1_STBC_SPEXP1", ""),
        ("MCS0-7_TXBF2",            ""),
        ("VHT8-9SS1_TXBF2",         ""),
        ("VHT10-11SS1_TXBF2",       ""),
        ("MCS8-15_SPEXP1",          ""),
        ("VHT8-9SS2_SPEXP1",        ""),
        ("VHT10-11SS2_SPEXP1",      ""),
        ("MCS8-15_TXBF1",           ""),
        ("VHT8-9SS2_TXBF1",         ""),
        ("VHT10-11SS2_TXBF1",       ""),
        ("MCS16-23",                ""),
        ("VHT8-9SS3",               ""),
        ("VHT10-11SS3",             ""),
        ("MCS16-23_TXBF0",          ""),
        ("VHT8-9SS3_TXBF0",         ""),
        ("VHT10-11SS3_TXBF0",       ""),
        ("DSSS_MULTI3",             ""),
        ("OFDM_CDD3",               ""),
        ("OFDM_TXBF3",              ""),
        ("MCS0-7_CDD3",             ""),
        ("VHT8-9SS1_CDD3",          ""),
        ("VHT10-11SS1_CDD3",        ""),
        ("MCS0-7_STBC_SPEXP2",      ""),
        ("VHT8-9SS1_STBC_SPEXP2",   ""),
        ("VHT10-11SS1_STBC_SPEXP2", ""),
        ("MCS0-7_TXBF3",            ""),
        ("VHT8-9SS1_TXBF3",         ""),
        ("VHT10-11SS1_TXBF3",       ""),
        ("MCS8-15_SPEXP2",          ""),
        ("VHT8-9SS2_SPEXP2",        ""),
        ("VHT10-11SS2_SPEXP2",      ""),
        ("MCS8-15_TXBF2",           ""),
        ("VHT8-9SS2_TXBF2",         ""),
        ("VHT10-11SS2_TXBF2",       ""),
        ("MCS16-23_SPEXP1",         ""),
        ("VHT8-9SS3_SPEXP1",        ""),
        ("VHT10-11SS3_SPEXP1",      ""),
        ("MCS16-23_TXBF1",          ""),
        ("VHT8-9SS3_TXBF1",         ""),
        ("VHT10-11SS3_TXBF1",       ""),
        ("MCS24-31",                ""),
        ("VHT8-9SS4",               ""),
        ("VHT10-11SS4",             ""),
        ("MCS24-31_TXBF0",          ""),
        ("VHT8-9SS4_TXBF0",         ""),
        ("VHT10-11SS4_TXBF0",       ""))

    # Number of rates and rate groups
    NUM_RATES = len(_RATES)

    # Rate info objects, indexed by XML rate name
    _rates = {}

    # VHT 0-7 rates and groups, referring their respective MCS rate objects
    _vht_rates = {}

    # VHT 0-7 rates and groups, referring their respective MCS rate objects
    _vht_rates = {}

    # Lists of rates corresponded to XML rate names (cached RateInfo.expand())
    _expanded_rates = {}

    # Rates for each chain count
    _rates_by_chains = {}

    # Maximum number of TX chains (computed in RateInfo.expand())
    _max_chains = 0

    @staticmethod
    def _initialize():
        """ Initialization - fills-in _rates and _expanded_rates dictionaries
        """
        blob_indices = set()
        if len(RatesInfo._rates) == 0:
            for i in range(len(RatesInfo._RATES)):
                rate_name, rate_blob_index = RatesInfo._RATES[i]
                assert (rate_name not in RatesInfo._rates) and (rate_blob_index not in blob_indices)
                RatesInfo._rates[rate_name] = RateInfo(rate_name, rate_blob_index, i)
                if rate_blob_index:
                    blob_indices.add(rate_blob_index)
                if RatesInfo._max_chains < RatesInfo._rates[rate_name].chains:
                    RatesInfo._max_chains = RatesInfo._rates[rate_name].chains
                if hasattr(RatesInfo._rates[rate_name], "vht_name"):
                    RatesInfo._vht_rates[RatesInfo._rates[rate_name].vht_name] = RatesInfo._rates[rate_name]
                RatesInfo._rates_by_chains.setdefault(RatesInfo._rates[rate_name].chains, set()).\
                    add(RatesInfo._rates[rate_name])
            for rate_name, rate_info in RatesInfo._rates.items():
                RatesInfo._expanded_rates[rate_name] = [RatesInfo._rates[n] for n in rate_info.expand()]
                if not rate_info.singular:
                    rate_info.group = rate_name
                    for r in RatesInfo._expanded_rates[rate_name]:
                        r.group = rate_name

    @staticmethod
    def valid_rate_name(name):
        """ Returns True if given XML or VHT rate name is valid """
        RatesInfo._initialize()
        return (name in RatesInfo._rates) or (name in RatesInfo._vht_rates)

    @staticmethod
    def get_rate(name_or_idx):
        """ Returns Rate info object by XML rate name or rate index

        Arguments:
        name -- XML or VHT rate name or index

        Return --RatesInfo
        """
        RatesInfo._initialize()
        if isinstance(name_or_idx, int):
            if (name_or_idx >= 0) and (name_or_idx < len(RatesInfo._RATES)):
                return RatesInfo._rates[RatesInfo._RATES[name_or_idx][0]]
            raise ValueError("Invalid rate index \"%d\"" % name_or_idx)
        else:
            ret = RatesInfo._rates.get(name_or_idx) or RatesInfo._vht_rates.get(name_or_idx)
            if ret:
                return ret
            # For some strange reason next line not marked as covered without this comment
            raise ValueError("Invalid rate name \"%s\"" % name_or_idx)

    @staticmethod
    def get_expanded_rate(name):
        """ Returns list of singular rates contained in rate or rate group
        given by argument

        Arguments:
        name    -- XML or VHT rate name
        Returns -- List of rates
        """
        RatesInfo._initialize()
        return RatesInfo._expanded_rates[RatesInfo.get_rate(name).name]

    @staticmethod
    def get_derived_rates(rate_info, bandwidth, allow_mcs_rates, interlocale):
        """ Returns list of rates whose power settings may be derived from
        those of given rate

        Return list of rates that shall use power setting for given rate
        Arguments:
        rate_info       -- Rate to give derived rates for
        bandwidth       -- Channel bandwidth in MHz
        allow_mcs_rates -- True allows MCS rates to be defined
        interlocale     -- True means that interlocale rate derivation rules
                           shall be used, false means that intralocale rate
                           derivation rules shall be used
        Returns List of rates that may use same power settings (if power
        settings for the was not specified)
        """
        if interlocale:
            rate_inferences = [("OFDM6", "MCS0")]
        else:
            rate_inferences = [
                ("MCS0_CDD1",   "OFDM6_CDD1"),
                ("MCS0_CDD2",   "OFDM6_CDD2"),
                ("MCS0_CDD1",   "MCS0_STBC")]
        ret = []
        for inference in rate_inferences:
            fr = RatesInfo._rates[inference[0]]
            tr = RatesInfo._rates[inference[1]]
            if (tr.rate_type == RateInfo.RateType.MCS) and not allow_mcs_rates:
                continue
            d = rate_info.index - fr.index
            if (d >= 0) and (d < RateInfo.NUM_MCS_MODS):
                ret.append(RatesInfo._rates[RatesInfo._RATES[tr.index + d][0]])
        return ret

    @staticmethod
    def rate_set_sort_key(rate_set):
        """ Returns sort key for set of rates """
        l = list(rate_set)
        l.sort(key=attrgetter("index"))
        return str(len(l)) + " " + " ".join([str(r.index) for r in l])

    @staticmethod
    def get_max_chains():
        """ Returns maximum number of chains """
        RatesInfo._initialize()
        return RatesInfo._max_chains

    @staticmethod
    def get_rates_by_chains(chains):
        """ Returns set of rates with given chain count """
        RatesInfo._initialize()
        return RatesInfo._rates_by_chains.get(chains, set())

# =============================================================================


class CcRev(ClmUtils.EqualityMixin):
    """ (CountryCode, RegopnRevision) pair

    None in place of each attribute makes it 'wildcard'. For example
    ("US", None) 'matches' all CcRev object with Country Code 'US'.

    Attributes:
    cc           -- string (two-letter country code) or None (wildcard)
    rev          -- integer (region revision) or None (wildcard)
    cc_ext_valid -- CC presents externally valid country code
    rev16        -- Has 16-bit rev
    deleted      -- Deleted CC
    """

    # Minimum 16-bit regrev
    MIN_REV16 = 255

    # Maximum 16-bit regrev
    MAX_REV16 = 65534

    def __init__(self, arg1, arg2=None):
        """ Constructor. Has several 'overloads'

        Arguments:
        Overload 1:
        ccrev -- String of CC/Rev, 'all'/Rev, CC/'all', 'all' form

        Overload 2:
        elem -- etree.Element that contains 'ccode' and 'rev' subelements

        Overload 3:
        cc  -- Country code
        rev -- Revision (integer in integer or string representation)
        """
        if isinstance(arg1, str) and (arg2 is None):
            m = re.search(r"(^all$)|(^all/(\d+)$)|(^([a-zA-Z0-9#][a-zA-Z0-9#])/(\d+)$)|(^([a-zA-Z0-9#][a-zA-Z0-9#])/all$)", arg1)
            if not m:
                raise ValueError("Invalid CC/rev: \"%s\"" % arg1)
            if m.group(1) is not None:
                self._ccrev = (None, None)
            elif m.group(2) is not None:
                self._ccrev = (None, int(m.group(3)))
            elif m.group(4) is not None:
                self._ccrev = (m.group(5), int(m.group(6)))
            elif m.group(7) is not None:
                self._ccrev = (m.group(8), None)
        elif arg2 is None:
            self._ccrev = (arg1.find("ccode").text, int(arg1.find("rev").text))
        else:
            self._ccrev = (arg1, int(arg2))

        cc = self._ccrev[0]
        rev = self._ccrev[1]
        if (rev is not None) and (rev > CcRev.MAX_REV16):
            raise ValueError("Invalid CC/rev: \"%s\" - rev may not exceed %d" % (str(self), CcRev.MAX_REV16))
        self.cc_externally_valid = not(
            (cc is None) or (not re.match(r"[A-Z][A-Z]$", cc)) or
            re.match(r"X[A-Z]$", cc) or re.match(r"Q[M-Z]$", cc) or
            (cc == "AA") or (cc == "ZZ") or (cc == "EU"))

    def __hash__(self):
        """ Hash for use of this type in dictionary key or set element """
        return self._ccrev.__hash__()

    def __getattr__(self, name):
        """ Returns cc and rev attributes """
        if name == "cc":
            return self._ccrev[0]
        if name == "rev":
            return self._ccrev[1]
        if name == "rev16":
            return (self._ccrev[1] is not None) and (self._ccrev[1] >= CcRev.MIN_REV16)
        if name == "deleted":
            return (self._ccrev[1] is not None) and (self._ccrev[1] < 0)
        raise AttributeError("Internal error: attribute \"%s\" not defined" % name)

    def __str__(self):
        """ Returns "CC/rev" string representation """
        if self._ccrev[0] or self._ccrev[1]:
            return (self._ccrev[0] if self._ccrev[0] else "all") + "/" +\
                   ("all" if (self._ccrev[1] is None) else
                    ("DELETED" if self.deleted else str(self._ccrev[1])))
        else:
            return "all"

    def __repr__(self):
        """ Debugger representation """
        return "<%s>" % str(self)

    def is_singular(self):
        """ True if CC/rev contains no wildcards """
        return (self._ccrev[0] is not None) and (self._ccrev[1] is not None)

    def match(self, ccrev):
        """ True if given ccrev matches (in wildcard sense) this object
        (wildcards are expected to be in this object)
        """
        if (self.cc is not None) and (self.cc != ccrev.cc):
            return False
        if (self.rev is not None) and (self.rev != ccrev.rev):
            return False
        return True

    def sort_key(self):
        """ Returns key for ccrev sorting """
        return \
            (self._ccrev[0] if self._ccrev[0] else "all") + "/" + \
            ("all" if (self._ccrev[1] is None) else ("%03d" % self._ccrev[1]))

# =============================================================================


class ChannelRange(ClmUtils.EqualityMixin):
    """ Channel range

    Private atrtibutes:
    _channel_type -- Channel type
    _start        -- Start channel number
    _end          -- End channel number

    Attributes (all readonly):
    channel_type -- Channel type (context of channel numbers)
    start        -- First channel
    end          -- Last channel
    is_80_80     -- True for single-80+80-channel range
    start_str    -- String representation of start channel number
    end_str      -- String representation of end channel number
    range_str    -- String representation of channel range (without band and
                    bandwidth)
    """
    # All existing channel ranges mapped to self
    _all = {}

    def __init__(self):
        """ Constructor. Shall never be called directly. create() shall be
        called instead """
        self._channel_type = None
        self._start = None
        self._end = None

    def __getattr__(self, name):
        """ Returns readonly attributes """
        if name == "channel_type":
            return self._channel_type
        if name == "start":
            return self._start
        if name == "end":
            return self._end
        if name == "is_80_80":
            return ChannelType.is_80_80(self._start, self._channel_type)
        if name == "start_str":
            return ChannelType.channel_to_string(self._start, self._channel_type)
        if name == "end_str":
            return ChannelType.channel_to_string(self._end, self._channel_type)
        if name == "range_str":
            start_str = ChannelType.channel_to_string(self._start, self._channel_type)
            if self._start == self._end:
                return start_str
            end_str = ChannelType.channel_to_string(self._end, self._channel_type)
            return start_str + "-" + end_str
        raise AttributeError("Internal error: attribute \"%s\" not defined" % name)

    def decompose_80_80(self):
        """ For 80+80 single-channel ranges returns (active, other) tuples with
        channel numbers, for others returns (None, None) tuples
        """
        return ChannelType.decompose_80_80(self._start, self._channel_type)

    def __hash__(self):
        """ Hash to use this type as dictionary key or set element """
        return (self._channel_type, self._start, self._end).__hash__()

    def __repr__(self):
        """ Debugger representation """
        return "<%s:%sM>" % \
            (self.range_str, Bandwidth.name[self._channel_type.bandwidth])

    def sort_key(self):
        """ Key for sorting by start then by end channel"""
        return "%s %s %03d %03d" % (Band.name[self.channel_type.band],
                                    Bandwidth.name[self.channel_type.bandwidth],
                                    self.start, self.end)

    @staticmethod
    def create(start, end, band, bandwidth):
        """ Creates ChannelRange object. If such channel range was previously
        created - fetches existing

        Arguments:
        start     -- First channel
        end       -- Last channel
        band      -- Band
        bandwidth -- Bandwidth
        Returns ChannelRange object with given parameters
        """
        cr = ChannelRange()
        cr._channel_type = ChannelType.create(band, bandwidth)
        cr._start = int(start)
        cr._end = int(end)
        return ChannelRange._all.setdefault(cr, cr)

    @staticmethod
    def clear_cache():
        """ Clears cached channel ranges. To be used before each unittest """
        ChannelRange._all.clear()

    @staticmethod
    def range_set_sort_key(range_set):
        """ Returns sort key for set of channel ranges """
        l = list(range_set)
        l.sort(key=methodcaller("sort_key"))
        return "  ".join(cr.sort_key() for cr in l)

# =============================================================================


class ChannelComb(ClmUtils.EqualityMixin):
    """" Group of evenly-spaced channel numbers

    Attributes:
    first  -- First channel number
    number -- Number of channels in group
    stride -- Channel numbers' stride (ignored  if 'number' is 1)
    """

    def __init__(self, first, number, stride):
        """ Construct by all parameters """
        self.first = first
        self.number = number
        self.stride = stride

    def __hash__(self):
        """ Hash for use as dictionary key or set member """
        return (self.first, self.number, self.stride).__hash__()

# =============================================================================


class ValidChannel(ClmUtils.EqualityMixin):
    """ Valid channel listed in CLM

    Attributes:
    channel        -- Channel number
    channel_type   -- Channel type
    channel_flavor -- Channel flavor ID. Only channels of same flavor (and
                      band+bandwidth) may form ranges
    is_80_80       -- Derived attribute. True if channel is 80+80
    channel_str    -- Derived attribute. String representation of channel number
                      (without band and bandwidth)
    """

    def __init__(self, elem):
        """ Constructor

        Arguments:
        elem -- etreeElement of tag <channel>
        """
        self.channel_type = ChannelType.create(Band.parse(elem.get("band")),
                                               Bandwidth.parse(elem.find("chan_width").text))
        num_elem = elem.find("number")
        if num_elem:
            self.channel = int(num_elem.text)
        else:
            self.channel = ChannelType.compose_80_80(int(elem.find("chan_pair_active").text),
                                                     int(elem.find("chan_pair_other").text))
        flavor_elem = elem.find("chan_flavor")
        if flavor_elem:
            self.channel_flavor = flavor_elem.text
        else:
            # Hardcoded values for legacy channel set
            if ChannelType.is_80_80(self.channel, self.channel_type):
                self.channel_flavor = "HalfChannel"
            elif (self.channel_type.bandwidth == Bandwidth._20) and self.channel in (34, 38, 42, 46):
                self.channel_flavor = "Japan"
            else:
                self.channel_flavor = "Normal"

    def decompose_80_80(self):
        """ For 80+80 channel returns (active, other) tuple with channel
        numbers, for others returns (None, None) tuples
        """
        return ChannelType.decompose_80_80(self.channel, self.channel_type)

    def __getattr__(self, name):
        if name == "is_80_80":
            return ChannelType.is_80_80(self.channel, self.channel_type)
        if name == "channel_str":
            return ChannelType.channel_to_string(self.channel, self.channel_type)
        raise AttributeError("Internal error: attribute \"%s\" not defined" % name)

    def __hash__(self):
        """ Hash for use as dictionary key or set member """
        return (self.channel_type, self.channel).__hash__()

    def __str__(self):
        """ String representation """
        return "%sG,%sM:%s" % (Band.name[self.channel_type.band][0],
                               Bandwidth.name[self.channel_type.bandwidth],
                               self.channel_str)

    def __repr__(self):
        """ Debugger representation """
        return "<%s>" % str(self)

    def sort_key(self):
        """ Returns key for valid channels sorting """
        return "%d %d %s %s" % (self.channel_type.band, self.channel_type.bandwidth,
                                self.channel_flavor, self.channel_str)

# =============================================================================


class ValidChannels:
    """ Collection of valid channels

    Attributes (all private):

    _channel_lists          -- Per channel type per channel flavor sorted lists
                               of channel numbers
    _chan_to_flavor         -- Per channel type maps of channel numbers to
                               channel flavors
    _chan_to_vc             -- Per channel type maps of channel numbers to
                               ValidChannel objects
    _sorted                 -- True if _channel_lists were sorted since last
                               addition of channel
    _range_to_channel_cache -- Cache of per-channel range channel lists
    """

    def __init__(self):
        """ Creates empty collection """
        self._clear()

    def add_channel(self, valid_channel):
        """ Adds given ValidChanenl object to collection """
        self._channel_lists.setdefault(valid_channel.channel_type, {}).\
            setdefault(valid_channel.channel_flavor, []).append(valid_channel.channel)
        self._sorted = False
        self._range_to_channel_cache = {}
        self._chan_to_flavor.\
            setdefault(valid_channel.channel_type, {})[valid_channel.channel] = \
            valid_channel.channel_flavor
        self._chan_to_vc.\
            setdefault(valid_channel.channel_type, {})[valid_channel.channel] = \
            valid_channel

    def get_valid_channel(self, channel_type, channel):
        """ Returns ValidChannel object for given channel type and number,
        None if not found """
        channels = self._chan_to_vc.get(channel_type, None)
        return channels.get(channel) if channels else None

    def get_channel_flavor(self, channel_type, channel):
        """ Returns flavor of given channel of given type, None if no such
        channel """
        channel_to_flavor = self._chan_to_flavor.get(channel_type)
        return None if channel_to_flavor is None else channel_to_flavor.get(channel)

    def get_range_flavor(self, channel_range):
        """ Returns flavor of given channel range """
        channel_to_flavor = self._chan_to_flavor.get(channel_range.channel_type)
        return None if channel_to_flavor is None else channel_to_flavor.get(channel_range.start)

    def get_channel_flavors(self, channel_type):
        """ Returns list of channel flavors for given channel type """
        flavor_to_list = self._channel_lists.get(channel_type)
        return [] if flavor_to_list is None else sorted(flavor_to_list.keys())

    def get_channels_of_type(self, channel_type, channel_flavor):
        """ Returns list of channel numbers of given channel type and flavor
        """
        self._sort()
        flavor_to_list = self._channel_lists.get(channel_type)
        if flavor_to_list is None:
            return []
        channels = flavor_to_list.get(channel_flavor)
        return channels if channels is not None else []

    def get_channels_in_range(self, channel_range):
        """ Returns list of channel numbers in given channel range """
        ret = self._range_to_channel_cache.get(channel_range)
        if ret is not None:
            return ret
        self._sort()
        ret = []
        flavor_to_list = self._channel_lists.get(channel_range.channel_type)
        if flavor_to_list is not None:
            channel_list = flavor_to_list.get(self._chan_to_flavor[channel_range.channel_type].
                                              get(channel_range.start))
            if channel_list is not None:
                ret = [channel for channel in channel_list
                       if (channel >= channel_range.start) and (channel <= channel_range.end)]
        self._range_to_channel_cache[channel_range] = ret
        return ret

    def all(self):
        """ All contained ValidChannel objects """
        for channel_type in self._chan_to_vc:
            for vc in sorted(self._chan_to_vc[channel_type].values(), key=attrgetter("channel")):
                yield vc

    def is_channel_in_range(self, channel, channel_range):
        """ True if given range contains channel with given number """
        if not (channel_range.start <= channel <= channel_range.end):
            return False
        chan_to_flavor = self._chan_to_flavor.get(channel_range.channel_type)
        if chan_to_flavor is None:
            return False
        range_flavor = chan_to_flavor.get(channel_range.start)
        chan_flavor = chan_to_flavor.get(channel)
        return (range_flavor is not None) and (range_flavor == chan_flavor)

    def try_split_range(self, channel_range, split_channel_type, split_channel):
        """" Tries to split given range at given channel

        Arguments:
        channel_range      -- Range to split
        split_channel_type -- Type of split channel
        split_channel      -- Number of split channel
        Returns If given range is of different channel type or does not contain
        given channel - returns empty list. Otherwise returns list of two
        channel ranges, split by given split channel
        """
        if (channel_range.channel_type != split_channel_type) or \
                (channel_range.start >= split_channel) or \
                (channel_range.end < split_channel):
            return []
        channels = self.get_channels_in_range(channel_range)
        for i in range(1, len(channels)):
            if channels[i] >= split_channel:
                return [ChannelRange.create(channels[0], channels[i - 1],
                                            channel_range.channel_type.band,
                                            channel_range.channel_type.bandwidth),
                        ChannelRange.create(channels[i], channels[-1],
                                            channel_range.channel_type.band,
                                            channel_range.channel_type.bandwidth)]
        return []  # Should never happen for correct range, but just in case...

    def split_ranges_in_collection(self, collection, range_getter, elem_copier,
                                   elem_splitter, split_channel_type, split_channel):
        """ Splits ranges, contained in collection elements:

        Arguments:
        collection         -- Iterable collection
        range_getter       -- Function that takes collection element and
                              returns channel range contained in it
        elem_copier        -- Function that takes element and copies it to
                              output collection unchanged (because its channel
                              range is not split)
        elem_splitter      -- Function that takes element and one of split
                              subranges and adds to output collection element
                              that corresponds to this subrange
        split_channel_type -- Type of split channel
        split_channel      -- Split channel number
        """
        for elem in collection:
            subranges = self.try_split_range(range_getter(elem), split_channel_type, split_channel)
            if subranges:
                for subrange in subranges:
                    elem_splitter(elem, subrange)
            else:
                elem_copier(elem)

    def get_diffs(self, other):
        """ Returns two-element vector. First element is a sequence of lines
        that describes what's is in self that is absent in other, second element
        is a sequence of lines that describes what's in other that is absent in
        self """
        ret = [[], []]
        for vc in sorted(set(self.all()) ^ set(other.all()), key=methodcaller("sort_key")):
            ret[0 if self.get_valid_channel(vc.channel_type, vc.channel) is not None else 1].\
                append("Valid channel: %s" % str(vc))
        return ret

    def trim(self, channel_ranges):
        """ Trims collection, leaving only those channels that contained in
        given set of channel ranges """
        self._sort()
        trim_result = set()
        for cr in channel_ranges:
            chan_to_flavor = self._chan_to_flavor.get(cr.channel_type)
            if chan_to_flavor is None:
                continue
            range_flavor = chan_to_flavor.get(cr.start)
            if range_flavor is None:
                continue
            chan_to_vc = self._chan_to_vc.get(cr.channel_type)
            if not chan_to_vc:
                continue
            for channel in chan_to_vc.keys():
                vc = chan_to_vc[channel]
                if (cr.start <= channel <= cr.end) and (vc.channel_flavor == range_flavor):
                    trim_result.add(vc)
                    del chan_to_vc[channel]
        self._clear()
        for vc in trim_result:
            self.add_channel(vc)

    def _clear(self):
        """ Clears collection """
        self._channel_lists = {}
        self._sorted = True
        self._chan_to_flavor = {}
        self._chan_to_vc = {}
        self._range_to_channel_cache = {}

    def _sort(self):
        """ Sorts channel lists in collection """
        if self._sorted:
            return
        self._sorted = True
        for flavor_to_list in self._channel_lists.values():
            for l in flavor_to_list.values():
                l.sort()

# =============================================================================


class LocaleType(ClmUtils.EqualityMixin):
    """ Locale type

    Attributes:
    flavor  -- Locale flavor (BASE, HT or HT3)
    band    -- Locale band
    is_base -- Derived attribute: True if flavor is Base
    """

    BASE, HT, HT3 = range(3)  # Locale flavors

    def __init__(self, flavor, band):
        """ Constructor

        Arguments:
        flavor -- Locale flavor (Base, Ht or Ht3)
        band   -- Locale band
        """
        self.flavor = flavor
        self.band = band

    def __getattr__(self, name):
        if name == "is_base":
            return (self.flavor == LocaleType.BASE)
        raise AttributeError("Internal error: attribute \"%s\" not defined" % name)

    def __hash__(self):
        """ Hash to use this type as dictionary key or set element """
        return (self.flavor, self.band).__hash__()

    def __str__(self):
        """ String representation """
        b = Band.name[self.band] + "G "
        if self.flavor == LocaleType.BASE:
            return b + "Base"
        if self.flavor == LocaleType.HT:
            return b + "HT"
        if self.flavor == LocaleType.HT3:
            return b + "HT3"
        raise ValueError("Invalid locale type")

    def __repr__(self):
        """ Debugger representation """
        return "<%s>" % str(self)

    @staticmethod
    def get_elem_type(elem):
        """ Returns locale type corresponded to given etreeElement tag

        Arguments:
        elem -- etreeElement of <..._locale> of <locale_ref> tag
        Returns LocaleType object or None
        """
        if elem.tag == "locale_ref":
            type_attr = elem.get("type")
            for dsc in LocaleType._descriptors:
                if type_attr == dsc[2]:
                    return dsc[3]
            return None
        tag_name = elem.tag
        band_attr = elem.get("band")
        for dsc in LocaleType._descriptors:
            if (dsc[0] == tag_name) and (dsc[1] == band_attr):
                return dsc[3]
        return None

    @staticmethod
    def all():
        """ Generator that sequences over all locale types """
        return [dsc[3] for dsc in LocaleType._descriptors]

    @staticmethod
    def compare(lt1, lt2):
        """ Comparator for sorting """
        return cmp(lt1.band, lt2.band) or cmp(lt1.flavor, lt2.flavor)

LocaleType._descriptors = [
    ("base_locale", "2.4", "base_2.4", LocaleType(LocaleType.BASE, Band._2)),
    ("base_locale",   "5", "base_5",   LocaleType(LocaleType.BASE, Band._5)),
    ("ht_locale",   "2.4", "ht_2.4",   LocaleType(LocaleType.HT,   Band._2)),
    ("ht_locale",     "5", "ht_5",     LocaleType(LocaleType.HT,   Band._5)),
    ("ht3_locale",  "2.4", "ht3_2.4",  LocaleType(LocaleType.HT3,  Band._2)),
    ("ht3_locale",    "5", "ht3_5",    LocaleType(LocaleType.HT3,  Band._5))]

# =============================================================================


class LocaleRegionCaps(ClmUtils.EqualityMixin):
    """ Locale capabilities that correspond to region flags

    Attributes:
    txbf         -- Locale has/region wants TXBF rates
    has_1024_qam -- Locale has/region wants 1024 QAM (VHT10-11) rates
    vht_txbf0    -- locale has/region wants VHT TXBF0 rates
    """
    def __init__(self, txbf, has_1024_qam, vht_txbf0):
        """ Construct by all parameters
        Arguments:
        txbf -- Locale has/region wants TXBF rates
        has_1024_qam -- Locale has/region wants 1024 QAM (VHT10-11) rates
        vht_txbf0 -- locale has/region wants VHT TXBF0 rates
        """
        self.txbf = txbf
        self.has_1024_qam = has_1024_qam
        self.vht_txbf0 = vht_txbf0

    def check_rate(self, rate):
        """ True if given rate can be contained in region with this flags """
        return (not rate.txbf or self.txbf) and \
               (not rate.vht_txbf0 or self.vht_txbf0) and \
               (not rate.is_1024_qam or self.has_1024_qam)

    @classmethod
    def minimal(cls):
        """ Factory function that returns object constructed to minimum
        capabilities """
        return cls(txbf=False, has_1024_qam=False, vht_txbf0=False)

    def __contains__(self, other):
        """ Overloads 'other in self'. True if all capabilities set in other
        also set in self """
        for a, v in self.__dict__.iteritems():
            if (not v) and getattr(other, a):
                return False
        return True

    def __and__(self, other):
        """ Overloads 'self & other'. Returns object containing common
        capabilities """
        ret = self.__class__.minimal()
        for a, v in self.__dict__.iteritems():
            setattr(ret, a, v & getattr(other, a))
        return ret

    def __or__(self, other):
        """ Overloads 'self | other'. Returns object containing capabilities
        supported by at least one """
        ret = self.__class__.minimal()
        for a, v in self.__dict__.iteritems():
            setattr(ret, a, v | getattr(other, a))
        return ret

    def __hash__(self):
        """ Returns hash of self """
        ret = 0
        for a, v in self.__dict__.iteritems():
            ret ^= a.__hash__() ^ v.__hash__()
        return ret

    def __str__(self):
        """ String representation """
        return "_".join(a.upper() for a in sorted(self.__dict__.keys()) if getattr(self, a))

# =============================================================================


class LocaleId(ClmUtils.EqualityMixin):
    """ Locale identity (name + capability flags as several instances of locale
    with different capability flags may exist simultaneously)

    Attributes:
    name     -- Locale name
    reg_caps -- LocaleRegionCaps object that reflects region flag's related
                locale capabilities
    """
    def __init__(self, name, reg_caps):
        """ Construct by name and TXBF flag """
        self.name = name
        self.reg_caps = reg_caps

    def __hash__(self):
        """ Hash for use as dictionary key and set element """
        return (self.name, self.reg_caps).__hash__()

    def __str__(self):
        """ String representation """
        return "%s_%s" % (self.name, str(self.reg_caps))

    def __repr__(self):
        """ Debugger representation """
        return "<%s>" % str(self)

    def sort_key(self):
        """ Sorting key that may be used for locale sorting """
        return (self.name, str(self.reg_caps))

# =============================================================================


class Locale(ClmUtils.EqualityMixin):
    """ Locale data obtained from XML

    Attributes:
    loc_id                 -- Locale id (LocaleId object)
    blob_alias             -- C-compatible alias to use in BLOB
    locale_type            -- Locale type (Base/HT/HT3, band)
    target                 -- Locale target (note) string
    dfs                    -- DFS (always Dfs.NONE for HT locales)
    filt_war1              -- Filter WAR for X19b platform
    max_chains             -- Maximum number of chains in rates contained in
                              locale
    restricted_set_name    -- Name of restricted set or None
    reg_power              -- List of regulatory power objects
    chan_power             -- Channel (transmit) power objects. Stored as
                              dictionary: keys are power objects, values are
                              sets (if locale is unfrozen) or frozensets
                              (if locale is frozen) of rates
    channel_sets           -- Per-bandwidth sets of used channel ranges
    channel_set_id         -- Numeric ID of locale channel set: 20MHz channel
                              set for base locale. Computed later by ClmData
    restricted_set_dict    -- Dictionary that maps restricted set names to
                              restricted sets. Initialized to None, computed
                              later by ClmData
    has_siblings           -- True if ClmData data contains siblings of this
                              locale (same name, different region capability
                              flags)
                              Initialized to False, computed later by ClmData
    max_bandwidth          -- Maximum bandwidth of contained power target
    _valid_channels        -- ValidChannels object - collection of valid
                              channels
    """

    # Rate frozensets mapped to self - used when locale is frozen
    _rate_frozensets = {}

    class Power(ClmUtils.EqualityMixin):
        """ Power target data, obtained from XML

        Private attributes:
        _powers_dbm    -- Tuple of powers in dBm. Empty for disabled power
                          target
        _channel_range -- Channel range (as ChannelRange object)
        _measure       -- TX power measurement mode. None for regulatory
                          power element

        Attributes (all readonly):
        powers_dbm      -- Tuple of powers in dBm. Empty for disabled power
                           target
        channel_range   -- Channel range (as ChannelRange object)
        measure         -- TX power measurement mode. None for regulatory
                           power element
        is_disabled     -- True if power target is disabled
        bandwidth       -- Shortcut to
                           self.channel_range.channel_type.bandwidth
        """

        # Power objects mapped to self - to promote single-instance
        _all = {}

        def __init__(self):
            """ Constructor. Should never be called directly """
            self._powers_dbm = None
            self._channel_range = None
            self._measure = None

        def __getattr__(self, name):
            """ Returns readonly attributes """
            if name == "powers_dbm":
                return self._powers_dbm
            if name == "channel_range":
                return self._channel_range
            if name == "measure":
                return self._measure
            if name == "is_disabled":
                return len(self._powers_dbm) == 0
            if name == "bandwidth":
                return self._channel_range.channel_type.bandwidth
            raise AttributeError("Internal error: attribute \"%s\" not defined" % name)

        def increment(self, inc_db):
            """ Returns power incremented by given amount of dB """
            if (inc_db == 0) or len(self.powers_dbm) == 0:
                return self
            return self.re_power([pwr + inc_db for pwr in self._powers_dbm])

        def re_range(self, channel_range):
            """ Returns copy of current element with range replaced to given
            """
            return Locale.Power._create(self._powers_dbm, channel_range, self._measure)

        def re_power(self, powers):
            """ Returns copy of current element with powers replaced to given
            """
            return Locale.Power._create(powers, self._channel_range, self._measure)

        def __eq__(self, other):
            return isinstance(other, Locale.Power) and \
                ClmUtils.dictionaries_partially_equal(self.__dict__, other.__dict__, [], [])

        def same_power(self, other):
            """ True if objects define same power (channel range may differ) """
            return isinstance(other, Locale.Power) and \
                ClmUtils.dictionaries_partially_equal(self.__dict__, other.__dict__, ["_channel_range"], [])

        def __hash__(self):
            """ Hash to use this type as dictionary key or set element """
            return (self._channel_range, self.powers_dbm, self.measure).__hash__()

        def __str__(self):
            """ String representation """
            return "%s-%s (%sM, %sG): %s" % (self._channel_range.start_str,
                                             self._channel_range.end_str,
                                             Bandwidth.name[self._channel_range.channel_type.bandwidth],
                                             Band.name[self._channel_range.channel_type.band],
                                             self.power_str())

        def power_str(self):
            """ Returns string representation of power data (without channel
            data)
            """
            if self.is_disabled:
                return "disabled"
            ret = "/".join("%.2f" % power for power in self._powers_dbm) + " dBm"
            if self._measure is not None:
                ret += " %s" % Measure.name[self._measure]
            return ret

        @staticmethod
        def create_from_elem(power_elem, band, allow_per_antenna):
            """ Create LocalePower object from ETree element object

            Arguments:
            power_elem        -- etree.Element of <channel_rate_power> or
                                 <regulatory_power> tag
            band              -- Band of locale this element belongs to
            allow_per_antenna -- Per-antenna power targets are allowed (False
                                 means they shall be converted to singular
                                 power targets with power taken from 'port=0')
            Returns Power object retrieved from XML
            """
            powers_dict = {}
            disabled = False
            for power_value_elem in power_elem.findall("power", max_version=1):
                if (power_value_elem.text == "disabled"):
                    disabled = True
                    continue
                port = power_value_elem.get("port")
                if port:
                    if int(port) and not allow_per_antenna:
                        continue
                    powers_dict[int(port)] = float(power_value_elem.text)
                else:
                    powers_dict[0] = float(power_value_elem.text)
            if (sorted(powers_dict.keys()) != list(range(len(powers_dict)))) or \
               ((power_elem.tag != "channel_rate_power") and (len(powers_dict) != 1)) or \
               (len(powers_dict) > RatesInfo.get_max_chains()) or \
               ((not disabled) and (not powers_dict)) or \
               (disabled and powers_dict):
                ClmUtils.error("Inconsistent power specification")

            powers_dbm = [powers_dict[i] for i in sorted(powers_dict.keys())]
            bandwidth_elem = power_elem.find("chan_width")
            chan_start_elem = power_elem.find("chan_start")
            chan_end_elem = power_elem.find("chan_end")
            if chan_start_elem and chan_end_elem:
                chan_start = chan_start_elem.text
                chan_end = chan_end_elem.text
            else:
                chan_start = ChannelType.compose_80_80(power_elem.find("chan_pair_active").text,
                                                       power_elem.find("chan_pair_other").text)
                chan_end = chan_start
            channel_range = \
                ChannelRange.create(
                    chan_start, chan_end, band,
                    Bandwidth.parse(bandwidth_elem.text) if (bandwidth_elem and bandwidth_elem.text) else Bandwidth.default)
            if re.match(r"^channel_rate_power\d*$", power_elem.tag):
                measure_elem = power_elem.find("measure")
                measure = Measure.parse(measure_elem.text) if (measure_elem and measure_elem.text) else Measure.NONE
            else:
                measure = None
            return Locale.Power._create(powers_dbm, channel_range, measure)

        @staticmethod
        def _create(powers_dbm, channel_range, measure):
            """ Creates Power object from components

            Arguments:
            powers_dbm    -- List or tuple of powers
            channel_range -- Channel range
            measure       -- Measure or None
            Returns Power object with given parameters
            """
            p = Locale.Power()
            p._powers_dbm = tuple(powers_dbm)
            p._channel_range = channel_range
            p._measure = measure
            return Locale.Power._all.setdefault(p, p)

        @staticmethod
        def get_extreme_power(p1, p2, get_min):
            """ Returns Power object that has minimum or maximum power value
            Tries to return one of given objects, but if impossible - creates new one.
            Disabled power considered to be less than any other.
            Singular and Diversity powers are compared by powers_dbm[0] only

            Arguments:
            p1      -- First power to compare
            p2      -- Second power to compare
            get_min -- True to get minimum power, false to get maximum power
            Returns Power object with minimum or maximum power
            """
            # If one is disabled?
            if len(p1.powers_dbm) == 0:
                return p1 if get_min else p2
            if len(p2.powers_dbm) == 0:
                return p2 if get_min else p1
            # Comparator for two scalar values. True if first argument has
            # required extreme (minimum or maximum) value, false if second
            if get_min:
                first_is_extreme = lambda p1, p2: p1 < p2
            else:
                first_is_extreme = lambda p1, p2: p1 > p2
            # If both are singular - do it simple way
            if (len(p1.powers_dbm) == 1) and (len(p2.powers_dbm) == 1):
                # Inverse order to exactly match original "<=" implementation
                return p2 if first_is_extreme(p2.powers_dbm[0], p1.powers_dbm[0]) else p1
            # Per-antenna comparison procedure - it is rather complex
            extreme_power = []  # Per-antenna extreme power values
            c1 = 0              # How many times p1 had extreme power
            c2 = 0              # How many times p2 had extreme power
            # Filling extreme_power, c1 and c2
            for i in range(max(len(p1.powers_dbm), len(p2.powers_dbm))):
                mp1 = p1.powers_dbm[0] if len(p1.powers_dbm) == 1 else (p1.powers_dbm[i] if i < len(p1.powers_dbm) else None)
                mp2 = p2.powers_dbm[0] if len(p2.powers_dbm) == 1 else (p2.powers_dbm[i] if i < len(p2.powers_dbm) else None)
                if (mp1 is None) or ((mp2 is not None) and first_is_extreme(mp2, mp1)):
                    extreme_power.append(mp2)
                    c2 += 1
                elif (mp2 is None) or ((mp1 is not None) and first_is_extreme(mp1, mp2)):
                    extreme_power.append(mp1)
                    c1 += 1
                else:
                    extreme_power.append(mp1)
            for i in range(len(extreme_power), RatesInfo.get_max_chains()):
                if len(p1.powers_dbm) == 1:
                    extreme_power.append(p1.powers_dbm[0])
                    c1 += 1
                elif len(p2.powers_dbm) == 1:
                    extreme_power.append(p2.powers_dbm[0])
                    c2 += 1
            # If one object never contributed to extreme power - return other
            if c2 == 0:
                return p1
            if c1 == 0:
                return p2
            # Constructing composite power object. Care only about power
            return p1.re_power(extreme_power)

        def __repr__(self):
            """ Debugger representation """
            return "<%s>" % str(self)

    def __init__(self, loc_elem, valid_channels, max_chains=RatesInfo.get_max_chains(), derive_rates=False,
                 max_bandwidth=Bandwidth.maximum, ac=True, allow_per_antenna=True, requested_reg_caps=None,
                 has_disabled_powers=True, allow_80_80=True, allow_dfs_tw=True, allow_ulb=True):
        """ Constructor

        Arguments:
        loc_elem            -- etree.Element of <base_locale> or <ht_locale>
                               tag
        valid_channels      -- ValidChannels object - collection of valid
                               channels
        max_chains          -- Maximum number of chains. Chain power elements
                               fetched from XML that have rates with chain
                               count larger than this are not included in
                               created object
        derive_rates        -- True means that intralocale rate derivation
                               shall be performed. Simplified derivation
                               method, used in this function is good for
                               legacy XML files, but not universally
                               applicable
        max_bandwidth       -- Maximum bandwidth
        ac                  -- Allow 802.11ac power targets
        allow_per_antenna   -- Per-antenna power targets are allowed (False
                               means they shall be converted to singular
                               power targets with power taken from 'chain=0')
        requested_reg_caps  -- Requested (maximum allowed) region capabilities
                               or None (to request maximum capabilities)
        has_disabled_powers -- True means that disabled powers shall be read
                               in to CLM data, false means that they shall be
                               ignored
        allow_80_80         -- Allows 80+80 power limits
        allow_dfs_tw        -- Allows use of TW DFS
        allow_ulb           -- Allow ULB (narrow) channels
        """
        self._valid_channels = valid_channels
        if (not ac) and (max_bandwidth > Bandwidth._40):
            max_bandwidth = Bandwidth._40
        name = Locale.get_elem_name(loc_elem)
        self.locale_type = LocaleType.get_elem_type(loc_elem)
        target_elem = loc_elem.find("target")
        self.target = target_elem.text.replace("\n", " ") if (target_elem and target_elem.text) else ""
        self.dfs = Dfs.NONE
        self.max_chains = 0
        self.restricted_set_name = None
        self.reg_power = []
        channel_sets = {}
        for bandwidth in ChannelType.bandwidths_of_band(self.locale_type.band):
            channel_sets[bandwidth] = set()
        self.channel_set_id = -1
        self.blob_alias = None
        self.restricted_set_dict = None
        self.has_siblings = False
        self.filt_war1 = loc_elem.find("filt_war1") is not None
        self.max_bandwidth = -1
        if self.locale_type.is_base:
            dfs_elem = loc_elem.find("dfs", max_version=1)
            if dfs_elem and dfs_elem.text:
                self.dfs = Dfs.parse(dfs_elem.text)
                if (not allow_dfs_tw) and (self.dfs == Dfs.TW):
                    self.dfs = Dfs.US
            rs_elem = loc_elem.find("restricted_set")
            if rs_elem:
                self.restricted_set_name = rs_elem.text
            for regpowers_elem in loc_elem.findall("regulatory_power_list", max_version=1):
                for rp_elem in regpowers_elem.getchildren():
                    power = Locale.Power.create_from_elem(rp_elem, self.locale_type.band, allow_per_antenna)
                    if self._valid_channels.get_range_flavor(power.channel_range) is None:
                        continue
                    self.reg_power.append(power)
                    self._add_range_to_set(channel_sets, power.channel_range)
        self.chan_power = {}
        chan_rate_dict = {}
        actual_reg_caps = LocaleRegionCaps.minimal()
        for chanpowers_elem in loc_elem.findall("channel_rate_power_list", max_version=4):
            for chanpower_elem in chanpowers_elem.getchildren():
                power = Locale.Power.create_from_elem(chanpower_elem, self.locale_type.band, allow_per_antenna)
                if (power.is_disabled and not has_disabled_powers) or \
                   (power.bandwidth > max_bandwidth) or \
                   (not allow_ulb and power.channel_range.channel_type.ulb) or \
                   (not allow_80_80 and power.channel_range.is_80_80) or \
                   (self._valid_channels.get_range_flavor(power.channel_range) is None):
                    force_coverage = "ZZZ"
                    continue
                rate_set, actual_max_chains = \
                    Locale._channel_rates(chanpower_elem, max_chains=max_chains, vht=ac,
                                          requested_reg_caps=requested_reg_caps,
                                          actual_reg_caps=actual_reg_caps,
                                          num_limits=len(power.powers_dbm))
                if rate_set:
                    if power.bandwidth > self.max_bandwidth:
                        self.max_bandwidth = power.bandwidth
                    if derive_rates:
                        if power.channel_range not in chan_rate_dict:
                            chan_rate_dict[power.channel_range] = set()
                        chan_rate_dict[power.channel_range] |= rate_set
                    if power not in self.chan_power:
                        self.chan_power[power] = set()
                    self.chan_power[power] |= rate_set
                    self._add_range_to_set(channel_sets, power.channel_range)
                self.max_chains = max(self.max_chains, actual_max_chains)
            if derive_rates:
                rates_to_add = {}
                for power, rates in self.chan_power.items():
                    ra = set()
                    for own_rate in rates:
                        for derived_rate in RatesInfo.get_derived_rates(own_rate, power.bandwidth, True, False):
                            if derived_rate not in chan_rate_dict[power.channel_range]:
                                ra.add(derived_rate)
                    if ra:
                        rates_to_add[power] = ra
                for power, rates in rates_to_add.items():
                    self.chan_power[power] |= rates
        self.channel_sets = {}
        self.loc_id = LocaleId(name, actual_reg_caps)
        for bandwidth in ChannelType.bandwidths_of_band(self.locale_type.band):
            self.channel_sets[bandwidth] = frozenset(channel_sets[bandwidth])
        self.freeze_rate_sets()

    def clone(self):
        """ Makes copy of self and returns it (rate sets in returned object are
        frozen even if in current they are not) """
        # First make shallow copy of self
        ret = copy.copy(self)
        for attr in ret.__dict__.keys():
            value = getattr(ret, attr)
            if attr in ["reg_power", "channel_sets", "chan_power"]:
                # Keys/values are either value types or single-instance
                # read-only objects. The only possible exception is unfrozen
                # rate sets, referenced from chan_power - they are being frozen
                # (and thus - unshared) at the end
                setattr(ret, attr, copy.copy(value))
            elif attr in ["_valid_channels", "restricted_set_dict"]:
                # Point to outer objects - shallow copy is sufficient
                pass
            else:
                # Deep copy for other fields
                setattr(ret, attr, copy.deepcopy(value))
        # Forcing rate sets unsharing
        ret.freeze_rate_sets()
        return ret

    def freeze_rate_sets(self):
        """ Freezes (compacts but makes readonly) locale by converting rate
        sets (referenced by chan_power keys) to frozensets, stored singularly
        """
        for power in self.chan_power.iterkeys():
            rate_set = self.chan_power[power]
            if not isinstance(rate_set, frozenset):
                rate_frozenset = frozenset(rate_set)
                self.chan_power[power] = \
                    Locale._rate_frozensets.setdefault(rate_frozenset,
                                                       rate_frozenset)

    def unfreeze_rate_sets(self):
        """ Assigns each power key in chan_power its individual rate set object
        to make rate sets modifications possible """
        for power in self.chan_power.iterkeys():
            rate_set = self.chan_power[power]
            if isinstance(rate_set, frozenset):
                self.chan_power[power] = set(rate_set)

    @staticmethod
    def merge_ht_ht3(loc_ht, loc_ht3):
        """ Creates locale that is a merge of two given ones

        This function creates deep copy of HT locale and adds channels from
        HT3 one to it

        Arguments:
        loc_ht  -- Locale object for HT locale
        loc_ht3 -- Locale object for HT3 locale or None (which means that
                   3-chain rates shall be excluded from resulted locale)
        Returns Merged locale
        """
        ret = loc_ht.clone()
        ret.unfreeze_rate_sets()

        if loc_ht3 is None:
            for bandwidth in ChannelType.bandwidths_of_band(loc_ht.locale_type.band):
                ret.channel_sets[bandwidth] = set()
            powers_to_remove = set()
            for power in ret.chan_power:
                ret.chan_power[power] -= RatesInfo.get_rates_by_chains(3)
                if ret.chan_power[power]:
                    ret._add_range_to_set(ret.channel_sets, power.channel_range)
                else:
                    powers_to_remove.add(power)
            for power in powers_to_remove:
                del ret.chan_power[power]
        else:
            for bandwidth in ChannelType.bandwidths_of_band(loc_ht.locale_type.band):
                ret.channel_sets[bandwidth] = set(ret.channel_sets[bandwidth])
            for power in loc_ht3.chan_power:
                if power not in ret.chan_power:
                    ret.chan_power[power] = set()
                for rate in loc_ht3.chan_power[power]:  # ret.chan_power[power] |= loc_ht3.chan_power[power] should be here but it doesn't work in IronPython
                    ret.chan_power[power].add(rate)
                ret._add_range_to_set(ret.channel_sets, power.channel_range)
        for bandwidth in ChannelType.bandwidths_of_band(loc_ht.locale_type.band):
            ret.channel_sets[bandwidth] = frozenset(ret.channel_sets[bandwidth])
            if ret.channel_sets[bandwidth]:
                ret.max_bandwidth = max(ret.max_bandwidth, bandwidth)
        ret.max_chains = max(map(lambda rs: max(map(lambda r: getattr(r, "chains"), rs)), ret.chan_power.values()))
        ret.channel_set_id = -1
        ret.loc_id = Locale.merge_ht_ht3_loc_id(loc_ht, loc_ht3)
        ret.freeze_rate_sets()
        return ret

    @staticmethod
    def merge_ht_ht3_loc_id(loc_ht, loc_ht3):
        """ Returns IDs of merged locale """
        if loc_ht3 is None:
            return LocaleId(loc_ht.loc_id.name + "&", loc_ht.loc_id.reg_caps)
        s1, s2 = ("", "") if loc_ht.loc_id.reg_caps == loc_ht3.loc_id.reg_caps else \
            ("_" + str(loc_ht.loc_id.reg_caps), "_" + str(loc_ht3.loc_id.reg_caps))
        return LocaleId(loc_ht.loc_id.name + s1 + "&" + loc_ht3.loc_id.name + s2,
                        loc_ht.loc_id.reg_caps | loc_ht3.loc_id.reg_caps)

    def _add_range_to_set(self, range_sets, new_range):
        """ Adds given channel range to set of locale's rate ranges

        If range belongs to range already in set, range is not included. If it
        contains some ranges already in set - they are removed. 80+80 ranges
        are always single-channel

        Arguments:
        range_sets -- Per-bandwidth set of ranges
        new_range  -- Range to add
        """
        range_set = range_sets[new_range.channel_type.bandwidth]
        if new_range in range_set:
            return
        if new_range.is_80_80:
            range_set.add(new_range)
            return
        new_range_channel_flavor = self._valid_channels.get_range_flavor(new_range)
        items_to_remove = set()
        new_range_start = new_range.start
        new_range_end = new_range.end
        for cr in range_set:
            if new_range_channel_flavor != self._valid_channels.get_range_flavor(cr):
                continue
            if(cr.start <= new_range_start) and (cr.end >= new_range_end):
                return
            if (cr.start >= new_range_start) and (cr.end <= new_range_end):
                items_to_remove.add(cr)
            elif (cr.start < new_range_start) and (cr.end >= new_range_start):
                items_to_remove.add(cr)
                new_range_start = cr.start
            elif (cr.start <= new_range_end) and (cr.end > new_range_end):
                items_to_remove.add(cr)
                new_range_end = cr.end
        if (new_range.start != new_range_start) or (new_range.end != new_range_end):
            new_range = ChannelRange.create(new_range_start, new_range_end,
                                            new_range.channel_type.band,
                                            new_range.channel_type.bandwidth)
        range_set.add(new_range)
        range_set -= items_to_remove

    @staticmethod
    def _channel_rates(power_elem, max_chains, vht, requested_reg_caps,
                       actual_reg_caps, num_limits):
        """ Returns set of rates contained in given <channel_rate_power>
        element

        Arguments:
        power_elem         -- etree.Element of <channel_rate_power> tag
        max_chains         -- Rates with chain count larger than this are not
                              included into resulted set
        vht                -- Include VHT rates
        requested_reg_caps -- Requested (maximum allowed) region-related
                              capabilities
        actual_reg_caps    -- Actual locale's region-related capabilities.
                              Updated by this function
        num_limits -- Number of power limits specified for element
        Returns two-element tuple: first element is set of rates contained in
        given <channel_rate_power> element, second element is maximum number of
        chains in rates of returned set
        """
        rates_set = set()
        actual_max_chains = 0
        for rate_elem in power_elem.findall("rate"):
            if not RatesInfo.valid_rate_name(rate_elem.text):
                continue
            rate = RatesInfo.get_rate(rate_elem.text)
            if num_limits not in (0, 1, rate.chains, rate.chains + 1):
                ClmUtils.error("%d TX limits can't be specified for %d-chain rates" % (num_limits, rate.chains))
            if not Locale.check_rate(rate, max_chains=max_chains, vht=vht, reg_caps=requested_reg_caps):
                continue
            if rate.txbf:
                actual_reg_caps.txbf = True
            if rate.is_1024_qam:
                actual_reg_caps.has_1024_qam = True
            if rate.vht_txbf0:
                actual_reg_caps.vht_txbf0 = True
            if rate.chains > actual_max_chains:
                actual_max_chains = rate.chains
            for singular_rate in RatesInfo.get_expanded_rate(rate_elem.text):
                rates_set.add(singular_rate)
        return (rates_set, actual_max_chains)

    @staticmethod
    def check_rate(rate, max_chains, vht, reg_caps):
        """ Checks if given rate matches given criteria

        Arguments:
        rate       -- Rate to check
        max_chains -- Maximum number of chains
        vht        -- True if VHT rates are allowed
        reg_caps   -- Region locale capability flags
        """
        return \
            (rate.chains <= max_chains) and \
            ((rate.rate_type != RateInfo.RateType.VHT) or vht) and \
            reg_caps.check_rate(rate)

    @staticmethod
    def get_elem_name(loc_elem):
        """ Returns name of locale contained in given ElementTree element """
        return loc_elem.find("id").text

    def split_ranges(self, split_channel_type, split_channel):
        """ Splits channel ranges in all contained collections by given split
        channel """
        new_reg_power = []
        self._valid_channels.split_ranges_in_collection(
            collection=self.reg_power, range_getter=lambda p: p.channel_range,
            elem_copier=lambda p: new_reg_power.append(p),
            elem_splitter=lambda p, cr: new_reg_power.append(p.re_range(cr)),
            split_channel_type=split_channel_type, split_channel=split_channel)
        self.reg_power = new_reg_power
        new_chan_power = {}
        self._valid_channels.split_ranges_in_collection(
            collection=self.chan_power.iteritems(), range_getter=lambda kvp: kvp[0].channel_range,
            elem_copier=lambda kvp: new_chan_power.setdefault(kvp[0], kvp[1]),
            elem_splitter=lambda kvp, new_cr: new_chan_power.setdefault(kvp[0].re_range(new_cr), kvp[1]),
            split_channel_type=split_channel_type, split_channel=split_channel)
        self.chan_power = new_chan_power
        for bw in self.channel_sets.keys():
            new_bw_channel_set = set()
            self._valid_channels.split_ranges_in_collection(
                collection=self.channel_sets[bw], range_getter=lambda cr: cr,
                elem_copier=lambda cr: new_bw_channel_set.add(cr),
                elem_splitter=lambda old_cr, new_cr: new_bw_channel_set.add(new_cr),
                split_channel_type=split_channel_type, split_channel=split_channel)
            self.channel_sets[bw] = frozenset(new_bw_channel_set)

    def get_unique_name(self):
        """ Returns human-readable locale name, unique among all locale names,
        contained in ClmData
        """
        return str(self.loc_id) if self.has_siblings else self.loc_id.name

    def data_hash(self):  # pragma: no cover
        """ Returns hash of data stored in BLOB """
        ret = 0
        if self.locale_type.is_base:
            ret ^= hash(self.dfs) ^ hash(self.filt_war1) ^ hash(self.restricted_set_name)
            for power in self.reg_power:
                ret ^= hash(power)
        for power, rates in self.chan_power.iteritems():
            ret ^= hash(power)
            for rate in rates:
                ret ^= rate.index
        return ret

    def data_equal(self, other, diff_report=None):
        """ True if data (BLOB) contents of this and another locale are the
        same
        """
        if diff_report is None:
            diff_report = DiffReport()
        diff_report.comparing("Locale %s%s" % (str(self.loc_id), ("/" + str(other.loc_id)) if isinstance(other, Locale) and (other.loc_id != self.loc_id) else ""))
        if not isinstance(other, Locale):
            diff_report.difference_in("Different types")
            return False
        keys_to_compare = ["locale_type"]
        if self.locale_type.is_base:
            keys_to_compare += ["dfs", "filt_war1"]
        if not ClmUtils.dictionaries_partially_equal(self.__dict__, other.__dict__, [], keys_to_compare, diff_report):
            return False
        chan_power_diff = self._get_chan_power_diff(other)
        if chan_power_diff != ({}, {}):
            for unique_items in chan_power_diff:
                for valid_chan in sorted(unique_items.keys(), key=methodcaller("sort_key")):
                    rate_power_dict = unique_items[valid_chan]
                    for rate in sorted(rate_power_dict.keys(), key=attrgetter("index")):
                        power = rate_power_dict[rate]
                        diff_report.difference_in("Only one locale defines power target <%s> for rate %s on channel %s" %
                                                  (power.power_str(), rate, valid_chan))
                        return False
        reg_power_diff = self._get_reg_power_diff(other)
        if reg_power_diff != ({}, {}):
            for unique_items in reg_power_diff:
                for chan_num in sorted(unique_items.keys()):
                    power = unique_items[chan_num]
                    diff_report.difference_in("Only one locale defines regulatory power of <%s> for channel %d" %
                                              (power.power_str(), chan_num))
                    return False
        if (self.restricted_set_dict is not None) and (other.restricted_set_dict is not None):
            self_rs_name = self.restricted_set_name
            other_rs_name = other.restricted_set_name
            if (self_rs_name is None) != (other_rs_name is None):
                diff_report.difference_in("Only one locale has restricted set defined")
                return False
            if (self_rs_name is not None) and (self.restricted_set_dict[self_rs_name] != other.restricted_set_dict[other_rs_name]):
                diff_report.difference_in("Different restricted sets")
                return False
        diff_report.compared()
        return True

    def get_diffs(self, other):
        """ Returns two-element vector. First element is a sequence of lines
        that describes what's is in self that is absent in other, second element
        is a sequence of lines that describes what's in other that is absent in
        self """
        ret = [[], []]
        for attr, prefix in [("loc_id", "Id"), ("locale_type", "Type"), ("restricted_set_name", "Restricted set")]:
            if getattr(self, attr) != getattr(other, attr):
                ret[0].append("%s: %s" % (prefix, str(getattr(self, attr))))
                ret[1].append("%s: %s" % (prefix, str(getattr(other, attr))))
        if self.dfs != other.dfs:
            ret[0].append("DFS: %s" % Dfs.name[self.dfs])
            ret[1].append("DFS: %s" % Dfs.name[other.dfs])
        if self.filt_war1 != other.filt_war1:
            ret[0].append("Filt WAR1: %s" % self.filt_war1)
            ret[1].append("Filt WAR1: %s" % other.filt_war1)
        unique_reg_powers = self._get_reg_power_diff(other)
        for up_idx in [0, 1]:
            for chan_num in sorted(unique_reg_powers[up_idx].keys()):
                diff_str = "Regulatory power for channel %d: %s" % (chan_num, unique_reg_powers[up_idx][chan_num].power_str())
                ret[up_idx].append(diff_str)
        unique_chan_powers = self._get_chan_power_diff(other)
        for up_idx in [0, 1]:
            for valid_chan in sorted(unique_chan_powers[up_idx].keys(), key=methodcaller("sort_key")):
                rate_power_dict = unique_chan_powers[up_idx][valid_chan]
                rates = sorted(rate_power_dict.keys(), key=attrgetter("index"))
                diff_str = "TX power targets for channel %s (%sMHz): %s" % (valid_chan.channel_str,
                                                                            Bandwidth.name[valid_chan.channel_type.bandwidth],
                                                                            ", ".join("%s:<%s>" % (rate, rate_power_dict[rate].power_str()) for rate in rates))
                ret[up_idx].append(diff_str)
        return ret

    def _get_chan_power_diff(self, other):
        """ Returns two-element tuple - power targets unique for this and other
        locales

        Arguments:
        other -- Locale to compare with
        Returns Two element tuple. Each element is a dictionary, indexed by
                ValidChannel objects, dictionary values are dictionaries
                indexed by RateInfo objects with values of Locale.Power objects
        """
        # Both following objects are dictionaries, indexed by fully-qualified
        # channels. Values are dictionaries indexed by rates, with values of
        # power objects (channels stored in power objects are ignored, only
        # power matters)
        self_unique = {}
        other_unique = {}
        # Building abovementioned unique_... dictionaries. Fully-identical
        # ranges (identical channel ranges, identical power, identical rate
        # set) present in both locales are skipped. Unique ranges are
        # disassembled to individual channels
        for this, that, unique in [(self, other, self_unique), (other, self, other_unique)]:
            for power, rates in this.chan_power.items():
                this_channels = this._valid_channels.get_channels_in_range(power.channel_range)
                if rates == that.chan_power.get(power):
                    that_channels = that._valid_channels.get_channels_in_range(power.channel_range)
                    if this_channels == that_channels:
                        continue
                    unique_range_channels = set(this_channels) - set(that_channels)
                else:
                    unique_range_channels = this._valid_channels.get_channels_in_range(power.channel_range)
                for chan_num in unique_range_channels:
                    valid_chan = this._valid_channels.get_valid_channel(power.channel_range.channel_type, chan_num)
                    rate_power_dict = unique.setdefault(valid_chan, {})
                    for rate in rates:
                        rate_power_dict[rate] = power
        # From channels, present in both locales removing rates that have
        # identical power targets. Some channel records in process may become
        # empty (because all rates have same power targets)
        for valid_chan, self_rate_power_dict in self_unique.items():
            other_rate_power_dict = other_unique.get(valid_chan)
            if other_rate_power_dict is None:
                continue
            common_rates = []
            for rate, self_power in self_rate_power_dict.items():
                other_power = other_rate_power_dict.get(rate)
                if (other_power is not None) and self_power.same_power(other_power):
                    common_rates.append(rate)
            for rate in common_rates:
                del self_rate_power_dict[rate]
                del other_rate_power_dict[rate]
        # Identifying and removing abovementioned empty channel records
        for unique in (self_unique, other_unique):
            common_channels = []
            for valid_chan, rate_power_dict in unique.items():
                if not rate_power_dict:
                    common_channels.append(valid_chan)
            for valid_chan in common_channels:
                del unique[valid_chan]
        return (self_unique, other_unique)

    def _get_reg_power_diff(self, other):
        """ Returns two-element tuple - regulatory powers unique for this and
        other locales

        Arguments:
        other -- Locale to compare with
        Returns Two element tuple. Each element is a dictionary, indexed by
                channel numbers, dictionary values are Locale.Power objects
        """
        # Both following objects are dictionaries, indexed by 20MHz channel
        # numbers. Values are power objects (channels stored in power objects
        # are ignored, only power matters)
        self_unique = {}
        other_unique = {}
        # Building abovementioned unique_... dictionaries. Fully-identical
        # ranges (identical channel ranges, identical power) present in both
        # locales are skipped. Unique ranges are disassembled to individual
        # channels
        for this, that, unique in [(self, other, self_unique), (other, self, other_unique)]:
            for power in this.reg_power:
                if power in that.reg_power:
                    continue
                for chan_num in self._valid_channels.get_channels_in_range(power.channel_range):
                    unique[chan_num] = power
        # Identifying and removing channels with equal power values
        common_channels = []
        for chan_num, self_power in self_unique.items():
            other_power = other_unique.get(chan_num)
            if (other_power is not None) and self_power.same_power(other_power):
                common_channels.append(chan_num)
        for chan_num in common_channels:
            del self_unique[chan_num]
            del other_unique[chan_num]
        return (self_unique, other_unique)

    def remove_restricted_set(self, restricted_set_name):
        """ If locale uses restricted set with given name - reference to it
        removed

        Returns true if reference was removed
        """
        if self.restricted_set_name and (self.restricted_set_name == restricted_set_name):
            self.restricted_set_name = None
            return True
        return False

    @staticmethod
    def clear_cache():
        """ Clears cached channel rate frozensets and power objects. To be
        used before each unittest """
        Locale._rate_frozensets.clear()
        Locale.Power._all.clear()

    def sort_key(self):
        """ Returns sort key of this object """
        return self.loc_id.sort_key()

    def __repr__(self):
        """ Debugger representation """
        return "<%s>" % self.get_unique_name()

# =============================================================================


class Region(ClmUtils.EqualityMixin):
    """ Region data obtained from XML

    Attributes:
    ccrev               -- (CountryCode/revision) pair, stored as CcRev
    country_name        -- Country name
    ad_c                -- Advertised CC or None
    sub_chan_rules_name -- Name of subchannel rules or None
    locale_ids          -- Dictionary of locale ids, indexed by locale types
    tags                -- Set of tags associated with region (tag types are
                           ignored)
    loc_reg_caps        -- Region-related locale capabilities (TXBF, etc.)
    edcrs               -- EDCRS compliance
    default_for_cc      -- True if region is default for its CC
    lo_gain_nbcal       -- Limit peak power during PAPD calibration
    chsprwar2           -- SPUR WAR for CN
    same_locales        -- Set of types of locales that are the same as those
                           of correspondent base (inc from) region
    locale_dict         -- Dictionary that maps locale ids to locale objects.
                           Initialized to None, set after creation by ClmData
    sub_chan_rules_dict -- Dictionary that maps subchannel rule names to
                           subchannel rule objects. Initialized to None, set
                           after creation by ClmData
    diff_report         -- Difference report, helper data used in incremental
                           BLOB generation
    rev16               -- Region has 16 bit rev
    """

    def __init__(self, reg_elem, country_dict, abg, band, allowed_loc_reg_caps,
                 allow_default_for_cc, allowed_edcrs, allow_lo_gain_nbcal, allow_chsprwar2):
        """ Constructor

        Arguments:
        reg_elem             -- etree.Element of <region> tag
        country_dict         -- Dictionary that maps country codes to country
                                names
        abg                  -- True means that HT locales shall not be
                                included
        band                 -- band(s) of locales to be included
        allowed_loc_reg_caps -- Allowed region-related locale capabilities
        allow_default_for_cc -- Allows to read "default_for_cc" attribute
        allowed_edcrs        -- Allowed EDCRS values
        allow_lo_gain_nbcal  -- Allows to read "lo_gain_nbcal" attribute
        allow_chsprwar2      -- Allows to read "chsprwar2" attribute
        """
        self.ccrev = CcRev(reg_elem)
        if self.ccrev.cc in country_dict:
            self.country_name = country_dict[self.ccrev.cc]
        else:
            self.country_name = ""

        adc_elem = reg_elem.find("ad_c")
        self.ad_c = adc_elem.text if adc_elem else None

        sub_chan_rules_elem = reg_elem.find("subchan_rules_ref")
        self.sub_chan_rules_name = sub_chan_rules_elem.text if sub_chan_rules_elem else None

        self.locale_dict = None
        self.sub_chan_rules_dict = None
        self.default_for_cc = bool(reg_elem.find("default_for_cc")) if allow_default_for_cc else False
        self.lo_gain_nbcal = bool(reg_elem.find("lo_gain_nbcal")) if allow_lo_gain_nbcal else False
        self.chsprwar2 = bool(reg_elem.find("chsprwar2")) if allow_chsprwar2 else False
        self.locale_ids = {}
        self.same_locales = set()
        self.diff_report = DiffReport()
        self.loc_reg_caps = \
            LocaleRegionCaps(txbf=bool(reg_elem.find("txbf")),
                             has_1024_qam=bool(reg_elem.find("is_1024qam")),
                             vht_txbf0=bool(reg_elem.find("vht_txbf0"))) & allowed_loc_reg_caps
        self.edcrs = Edcrs.DEFAULT
        edcrs_elem = reg_elem.find("edcrs")
        if edcrs_elem and edcrs_elem.text:
            edcrs = Edcrs.parse(edcrs_elem.text)
            if edcrs in allowed_edcrs:
                self.edcrs = edcrs

        locales_elem = reg_elem.find("locale_ref_list")
        if locales_elem:
            for loc_elem in locales_elem.findall("locale_ref"):
                locale_type = LocaleType.get_elem_type(loc_elem)
                if locale_type is None:
                    continue
                if abg and not locale_type.is_base:
                    continue
                if not (locale_type.band & band):
                    continue
                self.locale_ids[locale_type] = LocaleId(loc_elem.text, self.loc_reg_caps)
        self.tags = set()
        tags_elem = reg_elem.find("tags")
        if tags_elem:
            for tag_elem in tags_elem.findall("tag"):
                id_elem = tag_elem.find("id")
                if id_elem and id_elem.text:
                    self.tags.add(id_elem.text)

    def data_equal(self, other, diff_report=None):
        """ True if data (BLOB) contents of this and another region are the
        same
        """
        if diff_report is None:
            diff_report = DiffReport()
        diff_report.comparing("Region %s%s" % (str(self.ccrev), ("/" + str(other.ccrev)) if isinstance(other, Region) and (other.ccrev != self.ccrev) else ""))
        if not isinstance(other, Region):
            diff_report.difference_in("Different types")
            return False
        if not ClmUtils.dictionaries_partially_equal(self.__dict__, other.__dict__, [],
                                                     ["ccrev", "ad_c", "loc_reg_caps", "edcrs", "default_for_cc", "lo_gain_nbcal", "chsprwar2"],
                                                     diff_report):
            return False

        for loc_type in LocaleType.all():
            loc_self = self.locale_dict.get(self.locale_ids.get(loc_type))
            loc_other = other.locale_dict.get(other.locale_ids.get(loc_type))
            if (loc_self is None) and (loc_other is None):
                continue
            if (loc_self is None) != (loc_other is None):
                diff_report.difference_in("Only one region has locale \"%s\"" % str(loc_type))
                return False
            if not loc_self.data_equal(loc_other, diff_report):
                return False

        sub_chan_rules_self = self.sub_chan_rules_dict.get(self.sub_chan_rules_name)
        sub_chan_rules_other = other.sub_chan_rules_dict.get(other.sub_chan_rules_name)
        if (sub_chan_rules_self is None) != (sub_chan_rules_other is None):
            diff_report.difference_in("Only one region has subchannel rules defined")
            return False
        if sub_chan_rules_self is not None:
            if not SubChanRules.rules_equal(sub_chan_rules_self, sub_chan_rules_other, self.get_max_bandwidth()):
                diff_report.difference_in("Different subchannel rules")
                return False

        diff_report.compared()
        return True

    def get_max_bandwidth(self):
        """ Returns maximum power target of regions' locales """
        ret = -1
        for loc_name in self.locale_ids.values():
            loc_data = self.locale_dict.get(loc_name)
            if loc_data and loc_data.max_bandwidth > ret:
                ret = loc_data.max_bandwidth
        return ret

    def get_diffs(self, other):
        """ Returns two-element vector. First element is a sequence of lines
        that describes what's is in self that is absent in other, second element
        is a sequence of lines that describes what's in other that is absent in
        self """
        ret = [[], []]
        for attr, name, to_str in [("ccrev", "CC/rev", str),
                                   ("ad_c", "Advertised CC", str),
                                   ("edcrs", "EDCRS", lambda edcrs: Edcrs.name[edcrs]),
                                   ("loc_reg_caps", "Locale Flags", str),
                                   ("default_for_cc", "Default for CC", str),
                                   ("lo_gain_nbcal", "LoGainNBCAL", str),
                                   ("chsprwar2", "ChSprWar2", str)]:
            self_value = getattr(self, attr)
            other_value = getattr(other, attr)
            if self_value != other_value:
                if self_value is not None:
                    ret[0].append("%s: %s" % (name, to_str(self_value)))
                if other_value is not None:
                    ret[1].append("%s: %s" % (name, to_str(other_value)))
        if self.sub_chan_rules_name != other.sub_chan_rules_name:
            if self.sub_chan_rules_name is not None:
                ret[0].append("Subchannel rules: %s" % self.sub_chan_rules_name)
            if other.sub_chan_rules_name is not None:
                ret[1].append("Subchannel rules: %s" % other.sub_chan_rules_name)
        for loc_type in LocaleType.all():
            loc_ids = [self.locale_ids.get(loc_type), other.locale_ids.get(loc_type)]
            if loc_ids[0] == loc_ids[1]:
                continue
            for idx in (0, 1):
                if loc_ids[idx]:
                    ret[idx].append("%s locale: %s" % (str(loc_type), str(loc_ids[idx])))
        return ret

    def merge_ht_ht3(self, trim_ht3_to_ht):
        """ Merges HT3 locales to HT locales

        Arguments:
        trim_ht3_to_ht -- True means that 3-chains locales in HT-only region
                          shall be trimmed to 2-chains
        """
        for band in Band.all():
            ht_loc_type = LocaleType(LocaleType.HT, band)
            ht3_loc_type = LocaleType(LocaleType.HT3, band)
            ht3_loc = None
            if ht3_loc_type in self.locale_ids:
                ht3_loc = self.locale_dict.get(self.locale_ids[ht3_loc_type])
                if (ht3_loc is None) or (not ht3_loc.chan_power):
                    del self.locale_ids[ht3_loc_type]
                    ht3_loc = None
            if (ht_loc_type not in self.locale_ids) or (self.locale_ids[ht_loc_type] not in self.locale_dict):
                if ht3_loc:
                    self.locale_ids[ht_loc_type] = self.locale_ids[ht3_loc_type]
                    del self.locale_ids[ht3_loc_type]
                force_coverage = "ZZZ"
                continue
            ht_loc = self.locale_dict[self.locale_ids[ht_loc_type]]
            if ht3_loc is None:
                if (ht_loc.max_chains < 3) or (not trim_ht3_to_ht):
                    continue
            elif ht_loc.loc_id == ht3_loc.loc_id:
                del self.locale_ids[ht3_loc_type]
                continue
            merged_id = Locale.merge_ht_ht3_loc_id(ht_loc, ht3_loc)
            if merged_id not in self.locale_dict:
                self.locale_dict[merged_id] = Locale.merge_ht_ht3(ht_loc, ht3_loc)
            self.locale_ids[ht_loc_type] = merged_id
            if ht3_loc is not None:
                del self.locale_ids[ht3_loc_type]

    def get_locale(self, loc_type):
        """ Returns locale object of given type, associated with this region.
        None if locale of this type is not associated with region
        """
        loc_id = self.locale_ids.get(loc_type)
        if (loc_id is None) or (self.locale_dict is None):
            return None
        return self.locale_dict.get(loc_id)

    def remove_locale(self, locale_name):
        """ Removes locale with given name from region

        Returns true if locale was removed
        """
        types_to_remove = []
        for loc_type in self.locale_ids:
            if self.locale_ids[loc_type].name == locale_name:
                types_to_remove.append(loc_type)
        for loc_type in types_to_remove:
            del self.locale_ids[loc_type]
        return len(types_to_remove) != 0

    def remove_sub_chan_rules(self, sub_chan_rules_name):
        """ Removes subchannel rules with given name from region

        Returns true if subchannel rules were removed
        """
        if self.sub_chan_rules_name == sub_chan_rules_name:
            self.sub_chan_rules_name = None
            return True
        return False

    def find_same_locales(self, other):
        """ Finds types of locales that are equal to correspondent locales of
        other region
        """
        self.same_locales = set()
        if other is None:
            return
        for loc_type in self.locale_ids:
            loc_self = self.locale_dict.get(self.locale_ids[loc_type])
            id_other = other.locale_ids.get(loc_type)
            if id_other is None:
                continue
            loc_other = other.locale_dict.get(id_other)
            if loc_self.data_equal(loc_other):
                self.same_locales.add(loc_type)

    def __getattr__(self, name):
        if name == "rev16":
            return self.ccrev.rev16
        raise AttributeError("Internal error: attribute \"%s\" not defined" % name)

    def key(self):
        """ Returns unique key of this object """
        return self.ccrev

    def sort_key(self):
        """ Returns sort key of this object """
        return self.ccrev.sort_key()

    def __repr__(self):
        """ Debugger representation """
        return "<%s>" % str(self.ccrev)

# =============================================================================


class AdvertisedCountry(ClmUtils.EqualityMixin):
    """ Advertised country data obtained from XML

    Attributes:
    cc          -- Advertised country code
    regions     -- Set of regions' CcRevs
    diff_report -- Difference report, helper data used in incremental BLOB
                   generation
    """
    def __init__(self, cc):
        """ Constructor

        Arguments:
        cc -- Advertised country code
        """
        self.cc = cc
        self.regions = set()
        self.diff_report = DiffReport()

    def add_region(self, region):
        """ Adds region's CcRev """
        self.regions.add(region)

    def data_equal(self, other, diff_report=None):
        """ True if data (BLOB) contents of this and another advertisement are
        the same
        """
        if diff_report is None:
            diff_report = DiffReport()
        diff_report.comparing("Advertised CC %s%s" % (self.cc, ("/" + other.cc) if isinstance(other, AdvertisedCountry) and (other.cc != self.cc) else ""))
        if not isinstance(other, AdvertisedCountry):
            diff_report.difference_in("Different types")
            return False
        if self.cc != other.cc:
            diff_report.difference_in("Different advertised CC names")
            return False
        if self.regions != other.regions:
            diff_report.difference_in("Different advertised CC country lists")
            return False
        diff_report.compared()
        return True

    def get_diffs(self, other):
        """ Returns two-element vector. First element is a sequence of lines
        that describes what's is in self that is absent in other, second element
        is a sequence of lines that describes what's in other that is absent in
        self """
        ret = [[], []]
        if self.cc != other.cc:
            ret[0].append("CC: %s" % str(self.cc))
            ret[1].append("CC: %s" % str(other.cc))
        for regions, idx in [(self.regions - other.regions, 0), (other.regions - self.regions, 1)]:
            regions = list(regions)
            regions.sort(key=methodcaller("sort_key"))
            if regions:
                ret[idx].append("Regions: %s" % ", ".join([str(ccrev) for ccrev in regions]))
        return ret

    def key(self):
        """ Returns unique key of this object """
        return self.cc

    def sort_key(self):
        """ Returns sort key of this object """
        return self.cc

# =============================================================================


class Aggregation(ClmUtils.EqualityMixin):
    """ Aggregation data obtained from XML

    Attributes:
    ccrev        -- (CountryCode/revision) pair, stored as CcRev
    note         -- Aggregation's note (kind of description)
    mappings     -- List of mappings (CcRev objects)
    diff_report  -- Difference report, helper data used in incremental BLOB
                    generation
    rev16        -- Default region or mappings use 16-bit revs
    """
    def __init__(self, agg_elem):
        self.ccrev = CcRev(agg_elem)
        note_elem = agg_elem.find("note")
        self.note = note_elem.text.replace("\n", " ") if (note_elem and note_elem.text) else ""
        self.diff_report = DiffReport()
        maps_elem = agg_elem.find("mapping_list")
        if maps_elem:
            self.mappings = [CcRev(map_elem) for map_elem in maps_elem.findall("mapping", 1)]
        else:
            self.mappings = []

    def data_equal(self, other, diff_report=None):
        """ True if data (BLOB) contents of this and another aggregation are
        the same
        """
        if diff_report is None:
            diff_report = DiffReport()
        diff_report.comparing("Aggregation %s%s" % (str(self.ccrev), ("/" + str(other.ccrev)) if isinstance(other, Aggregation) and (other.ccrev != self.ccrev) else ""))
        if not isinstance(other, Aggregation):
            diff_report.difference_in("Different types")
            return False
        if self.ccrev != other.ccrev:
            diff_report.difference_in("Different default region names")
            return False
        if set(self.mappings) != set(other.mappings):
            diff_report.difference_in("Different mappings - %s" %
                                      ", ".join(("\"%s\"" % str(ccrev)) for ccrev in set(self.mappings) ^ set(other.mappings)))
            return False

        diff_report.compared()
        return True

    def get_diffs(self, other):
        """ Returns two-element vector. First element is a sequence of lines
        that describes what's is in self that is absent in other, second element
        is a sequence of lines that describes what's in other that is absent in
        self """
        ret = [[], []]
        if self.ccrev != other.ccrev:
            ret[0].append("SROM: %s" % str(self.ccrev))
            ret[1].append("SROM: %s" % str(other.ccrev))
        for mappings, idx in [(set(self.mappings) - set(other.mappings), 0), (set(other.mappings) - set(self.mappings), 1)]:
            if not mappings:
                continue
            mappings = list(mappings)
            mappings.sort(key=methodcaller("sort_key"))
            ret[idx].append("Mappings: %s" % ", ".join([str(mapping) for mapping in mappings]))
        return ret

    def remove_duplicated_mappings(self, other):
        """ Removes entries from other aggregation from this aggregation, marks
        entries present in other aggregation but absent in this as deleted
        """
        deleted_mappings = set([m.cc for m in other.mappings]) - set([m.cc for m in self.mappings])
        new_mappings = set(self.mappings) - set(other.mappings)
        self.mappings = list(new_mappings) + [CcRev(cc, -1) for cc in deleted_mappings]

    def remove_region(self, reg_ccrev):
        """ Tries to remove given region from mapping list, returns true if
        region was removed
        """
        if reg_ccrev not in self.mappings:
            return False
        self.mappings.remove(reg_ccrev)
        return True

    def __getattr__(self, name):
        if name == "rev16":
            if self.ccrev.rev16:
                return True
            for ccrev in self.mappings:
                if ccrev.rev16:
                    return True
            return False
        raise AttributeError("Internal error: attribute \"%s\" not defined" % name)

    def key(self):
        """ Returns unique key of this object """
        return self.ccrev

    def sort_key(self):
        """ Returns sort key of this object """
        return self.ccrev.sort_key()

# =============================================================================


class RestrictedSet(ClmUtils.EqualityMixin):
    """ Restricted set data obtained from XML

    Attributes:
    name            -- Restricted set name
    band            -- Band
    is_all_channels -- True if all channels of band are restricted
    channels        -- List of restricted channel ranges
    """
    def __init__(self, set_elem, valid_channels):
        self.name = set_elem.find("set").text
        self.band = Band.parse(set_elem.get("band"))
        self.is_all_channels = bool(set_elem.find("all_channels"))
        self.channels = []
        if not self.is_all_channels:
            for chan_list_elem in set_elem.findall("channel_list", max_version=1):
                for elem in chan_list_elem.getchildren():
                    if elem.tag == "chan_start":
                        start = elem.text
                    elif elem.tag == "chan_end":
                        cr = ChannelRange.create(start, elem.text, self.band, Bandwidth.default)
                        if valid_channels.get_range_flavor(cr) is not None:
                            self.channels.append(cr)
            self.channels.sort(key=methodcaller("sort_key"))

    def split_ranges(self, valid_channels, split_channel_type, split_channel):
        """ Splits channel ranges in all contained collections by given split
        channel using given valid channels' collection """
        new_channels = []
        valid_channels.split_ranges_in_collection(
            collection=self.channels, range_getter=lambda cr: cr,
            elem_copier=lambda cr: new_channels.append(cr),
            elem_splitter=lambda old_cr, new_cr: new_channels.append(new_cr),
            split_channel_type=split_channel_type, split_channel=split_channel)
        self.channels = sorted(new_channels, key=methodcaller("sort_key"))

    def get_diffs(self, other):
        """ Returns two-element vector. First element is a sequence of lines
        that describes what's is in self that is absent in other, second element
        is a sequence of lines that describes what's in other that is absent in
        self """
        ret = [[], []]
        if self.name != other.name:
            ret[0].append("Name: %s" % str(self.name))
            ret[1].append("Name: %s" % str(other.name))
        if self.band != other.band:
            ret[0].append("Band: %s" % Band.name[self.band])
            ret[1].append("Band: %s" % Band.name[other.band])
        if self.is_all_channels != other.is_all_channels:
            ret[0 if self.is_all_channels else 1].append("All channels")
        for chan_ranges, idx in [(set(self.channels) - set(other.channels), 0),
                                 (set(other.channels) - set(self.channels), 1)]:
            if not chan_ranges:
                continue
            chan_ranges = list(chan_ranges)
            chan_ranges.sort(key=methodcaller("sort_key"))
            ret[idx].append("Channels: %s" % ", ".join(chan_range.range_str for chan_range in chan_ranges))
        return ret

    def sort_key(self):
        """ Returns sort key of this object """
        return self.name

# =============================================================================


class SubChanRules(ClmUtils.EqualityMixin):
    """ Region-level subchannel inference rules

    Attributes:
    name            -- Rule set name
    chan_rules_dict -- Channel-level rules indexed by channel ranges.
                       Channel-level rule is a dictionary of subchannel rules
                       indexed by subchannel ID. Subchannel rule is a tuple:
                       (set_of_bandwidths, power_increment)
    """

    def __init__(self, sub_chan_elem, valid_channels, sub_chan_rules_40,
                 sub_chan_rules_inc):
        """ Constructor

        Arguments:
        sub_chan_elem      -- etree.Element with subchannel rule
        valid_channels     -- ValidChannels object that contains valid channels
        sub_chan_rules_40  -- 40MHz subchannel rules allowed
        sub_chan_rules_inc -- Power increments in rules allowed
        """
        self.name = sub_chan_elem.find("id").text
        self.chan_rules_dict = {}
        for chan_rule_elem in sub_chan_elem.findall("subchan_rule", max_version=3):
            channel_range = ChannelRange.create(chan_rule_elem.find("chan_start").text,
                                                chan_rule_elem.find("chan_end").text,
                                                Band.parse(chan_rule_elem.get("band")),
                                                Bandwidth.parse(chan_rule_elem.find("chan_width").text))
            if valid_channels.get_range_flavor(channel_range) is None:
                continue
            if (channel_range.channel_type.bandwidth == Bandwidth._40) and not sub_chan_rules_40:
                continue
            self.chan_rules_dict[channel_range] = {}
            for sub_chan_rule_elem in chan_rule_elem.findall("subchan"):
                sub_chan_id = SubChanId.parse(sub_chan_rule_elem.find("subchan_id").text)
                bandwidths = set()
                for limit_elem in sub_chan_rule_elem.findall("limit"):
                    bandwidths.add(Bandwidth.parse(limit_elem.text))
                power_inc_elem = sub_chan_rule_elem.find("increment") if sub_chan_rules_inc else None
                self.chan_rules_dict[channel_range][sub_chan_id] = \
                    (bandwidths, 0 if power_inc_elem is None else float(power_inc_elem.text))

    def split_ranges(self, valid_channels, split_channel_type, split_channel):
        """ Splits channel ranges in all contained collections by given split
        channel using given valid channels' collection """
        new_chan_rules_dict = {}
        valid_channels.split_ranges_in_collection(
            collection=self.chan_rules_dict.iteritems(), range_getter=lambda kvp: kvp[0],
            elem_copier=lambda kvp: new_chan_rules_dict.setdefault(kvp[0], kvp[1]),
            elem_splitter=lambda kvp, cr: new_chan_rules_dict.setdefault(cr, copy.deepcopy(kvp[1])),
            split_channel_type=split_channel_type, split_channel=split_channel)
        self.chan_rules_dict = new_chan_rules_dict

    def get_diffs(self, other):
        """ Returns two-element vector. First element is a sequence of lines
        that describes what's is in self that is absent in other, second element
        is a sequence of lines that describes what's in other that is absent in
        self """
        ret = [[], []]
        if self.name != other.name:
            ret[0].append("Name: %s" % str(self.name))
            ret[1].append("Name: %s" % str(other.name))
        for chan_ranges, idx in [(set(self.chan_rules_dict.keys()) - set(other.chan_rules_dict.keys()), 0),
                                 (set(other.chan_rules_dict.keys()) - set(self.chan_rules_dict.keys()), 1)]:
            if not chan_ranges:
                continue
            chan_ranges = list(chan_ranges)
            chan_ranges.sort(key=methodcaller("sort_key"))
            ret[idx].append("Channels: %s" % ", ".join(chan_range.range_str for chan_range in chan_ranges))
        for chan_range in self.chan_rules_dict:
            if chan_range not in other.chan_rules_dict:
                continue
            for sub_chan_ids, idx in [(set(self.chan_rules_dict[chan_range].keys()) - set(other.chan_rules_dict[chan_range].keys()), 0),
                                      (set(other.chan_rules_dict[chan_range].keys()) - set(self.chan_rules_dict[chan_range].keys()), 1)]:
                if not sub_chan_ids:
                    continue
                sub_chan_ids = list(sub_chan_ids)
                sub_chan_ids.sort()
                ret[idx].append("Subchannel IDs for channel range %s: %s" %
                                (chan_range.range_str, ", ".join([SubChanId.name[sub_chan_id] for sub_chan_id in sub_chan_ids])))
            for sub_chan_id in self.chan_rules_dict[chan_range].keys():
                this_rule = self.chan_rules_dict[chan_range][sub_chan_id]
                other_rule = other.chan_rules_dict[chan_range].get(sub_chan_id)
                if (other_rule is None) or (this_rule == other_rule):
                    continue
                for bandwidths, idx in [(this_rule[0] - other_rule[0], 0), (other_rule[0] - this_rule[0], 1)]:
                    if not bandwidths:
                        continue
                    bandwidths = list(bandwidths)
                    bandwidths.sort()
                    ret[idx].append("Limit bandwidths for channel range %s, subchannel ID %s: %s" %
                                    (chan_range.range_str, SubChanId.name[sub_chan_id], ", ".join([Bandwidth.name[bandwidth] for bandwidth in bandwidths])))
                if this_rule[1] != other_rule[1]:
                    for rule, idx in [(this_rule, 0), (other_rule, 1)]:
                        ret[idx].append("Power increment for channel range %s, subchannel ID %s: %f" %
                                        (chan_range.range_str, SubChanId.name[sub_chan_id], rule[1]))
        return ret

    def trim_to_max_bandwidth(self, max_bandwidth):
        """ Leave only rules with that correspond to not more that given main
        channel bandwidth
        """
        ranges_to_delete = []
        for cr in self.chan_rules_dict:
            if cr.channel_type.bandwidth > max_bandwidth:
                ranges_to_delete.append(cr)
        for cr in ranges_to_delete:
            del self.chan_rules_dict[cr]

    def patch(self):
        """ Dirty patch to alleviate absence of disabled powers in BLOB """
        for cr in self.chan_rules_dict:
            if (cr.channel_type.bandwidth == Bandwidth._80) and (cr.start <= 42 <= cr.end):
                for sub_chan_id in self.chan_rules_dict[cr]:
                    if len(self.chan_rules_dict[cr][sub_chan_id][0]) <= 1:
                        continue
                    if sub_chan_id == SubChanId.LL:
                        self.chan_rules_dict[cr][sub_chan_id][0].clear()
                        self.chan_rules_dict[cr][sub_chan_id][0].add(Bandwidth._20)
                    elif sub_chan_id == SubChanId.L:
                        self.chan_rules_dict[cr][sub_chan_id][0].clear()
                        self.chan_rules_dict[cr][sub_chan_id][0].add(Bandwidth._80)

    @staticmethod
    def rules_equal(r1, r2, max_bandwidth):
        """ True if rules equal up to given maximum bandwidth """
        if max_bandwidth < 0:
            return True
        if (r1 is None) != (r2 is None):
            return False
        if r1 is None:
            return True
        for cr in (set(r1.chan_rules_dict.keys()) | set(r2.chan_rules_dict.keys())):
            if cr.channel_type.bandwidth > max_bandwidth:
                continue
            if r1.chan_rules_dict.get(cr) != r2.chan_rules_dict.get(cr):
                return False
        return True

    def get_num_channel_type_rules(self, channel_type):
        """ Returns number of rules for given channel type of main channel """
        ret = 0
        for cr in self.chan_rules_dict:
            if cr.channel_type == channel_type:
                ret += 1
        return ret

    def sort_key(self):
        """ Returns sort key of this object """
        return self.name


# =============================================================================


class BlobFormatVersionInfo(ClmUtils.EqualityMixin):
    """ Outstanding characteristics of supported BLOB format

    Attributes:
    version_string              -- BLOB format version string
    comment                     -- Short descriptor of what format is and what
                                   for is it
    registry_flags_allowed      -- Flag field in clm_data_registry supported
    reg_rec_10_fl               -- Region record with flags and 10 bit locale
                                   indices used
    ac_allowed                  -- 802.11ac targets allowed
    sub_chan_rules              -- Subchannel rules are put to BLOB
    apps_version                -- Apps version is put to BLOB
    loc_reg_caps                -- Maximum locale-related region capabilities
    per_antenna                 -- Has per-antenna TX power limits
    numrates_field              -- NUM_RATES flag field
    max_bandwidth               -- Maximum supported bandwidth
    has_no_80mhz                -- wlc_clm.h defines CLM_FLAG_NO_80MHZ flag
    per_bw_rs                   -- Per-bandwidth channel ranges and rate sets
    has_disabled_powers         -- Supports disabled powers in BLOB
    has_default_for_cc          -- Supports "Default for CC" region flag
    trim_ht3_to_ht              -- Trims HT regions with HT3 locales to HT
    patch_subchan_rules         -- Patch subchannel rules (for now - remove
                                   FCC-a multimeasure rules for 80MHz channel
                                   42)
    has_80_80                   -- Supports 80+80 channels
    per_band_rate_sets          -- Per band (in addition to per-bandwidth)
                                   rate sets
    has_user_string             -- BLOB stores user string
    high_bw_combs               -- BLOB stores all combs
    content_dependent_reg       -- Content-dependent region record format
    rev16                       -- Regions have 16-bit rev
    indexed_ccrev               -- Cc/revs are specified by indices in region
                                   table
    supported_edcrs             -- Supported EDCRS values
    channel_flavors             -- Supported channel flavors
    has_dfs_tw                  -- Supports TW DFS
    max_chains                  -- Maximum TX chains supported
    has_ulb                     -- Supports ULB (narrow) channels
    ext_rate_sets               -- Supports extra rate sets' tables
    dummy_regrev_remap          -- Has dummy regrev remap field
    scr_idx_bits                -- Number of bits in subchannel rule indices
    scr_idx_initialize          -- Initialize SCR_... indices with
                                   CLM_SCR_IDC... values
    sub_chan_rules_40           -- Has 40MHz subchannel rules
    sub_chan_rules_inc          -- Subchannel rules have power increment
    sub_chan_rules_inc_separate -- Separate format for subchannel power
                                   increments
    has_lo_gain_nbcal           -- Supports LoGainNBCAL flag
    has_chsprwar2               -- Supports ChSprWar2 flag
    has_registry_flags2         -- clm_registry_t contains flags2 field
    reg_rec_size                -- Region record size in bytes
    header_size                 -- BLOB header size in bytes
    footer_size                 -- BLOB footer size in bytes
    """

    def __init__(self, version_string, comment, registry_flags_allowed,
                 reg_rec_10_fl, ac_allowed, sub_chan_rules, apps_version,
                 loc_reg_caps, per_antenna, numrates_field, max_bandwidth,
                 has_no_80mhz, per_bw_rs, has_disabled_powers,
                 has_default_for_cc, trim_ht3_to_ht, patch_subchan_rules,
                 has_80_80, per_band_rate_sets, has_user_string, high_bw_combs,
                 content_dependent_reg, rev16, indexed_ccrev, supported_edcrs,
                 channel_flavors, has_dfs_tw, max_chains, has_ulb, ext_rate_sets,
                 dummy_regrev_remap, scr_idx_bits, scr_idx_initialize, sub_chan_rules_40,
                 sub_chan_rules_inc, sub_chan_rules_inc_separate, has_lo_gain_nbcal, has_chsprwar2,
                 has_registry_flags2, reg_rec_size, header_size, footer_size):
        """ Construct by all parameters:

        Arguments:
        version_string              -- BLOB format version string
        comment                     -- Short descriptor of what format is and
                                       what for is it
        registry_flags_allowed      -- Flag field in clm_data_registry supported
        reg_rec_10_fl               -- Region record with flags and 10 bit
                                       locale indices used
        ac_allowed                  -- 802.11ac targets allowed
        sub_chan_rules              -- Subchannel rules are put to BLOB
        apps_version                -- Apps version is put to BLOB
        loc_reg_caps                -- Maximum locale-related region
                                       capabilities
        per_antenna                 -- Has per-antenna TX power limits
        numrates_field              -- NUM_RATES flag field
        max_bandwidth               -- Maximum supported bandwidth
        has_no_80mhz                -- wlc_clm.h defines CLM_FLAG_NO_80MHZ flag
        per_bw_rs                   -- per-bandwidth channel ranges and rate
                                       sets
        has_disabled_powers         -- Supports disabled powers in BLOB
        has_default_for_cc          -- Supports "Default for CC" region flag
        trim_ht3_to_ht              -- Trims HT regions with HT3 locales to HT
        patch_subchan_rules         -- Patch subchannel rules (for now - remove
                                       FCC-a multimeasure rules for 80MHz
                                       channel 42)
        has_80_80                   -- Supports 80+80 channels
        per_band_rate_sets          -- Per band (in addition to per-bandwidth)
                                       rate sets
        has_user_string             -- BLOB stores user string
        high_bw_combs               -- BLOB stores all combs
        content_dependent_reg       -- Content-dependent region record format
        rev16                       -- Regions have 16-bit rev
        indexed_ccrev               -- Cc/revs are specified by indices in
                                       region table
        supported_edcrs             -- Supported EDCRS values
        channel_flavors             -- Supported channel flavors
        has_dfs_tw                  -- Supports TW DFS
        max_chains                  -- Maximum TX chains supported
        has_ulb                     -- Supports ULB (narrow) channels
        ext_rate_sets               -- Supports extra rate sets' tables
        dummy_regrev_remap          -- Has dummy regrev remap field
        scr_idx_bits                -- Number of bits in subchannel rule
                                       indices
        scr_idx_initialize          -- Initialize SCR_... indices with
                                       CLM_SCR_IDC... values
        sub_chan_rules_40           -- Has 40MHz subchannel rules
        sub_chan_rules_inc          -- Subchannel rules have power increment
        sub_chan_rules_inc_separate -- Separate format for subchannel power
                                       increments
        has_lo_gain_nbcal           -- Supports LoGainNBCAL flag
        has_chsprwar2               -- Supports ChSprWar2 (China Spur WAR) flag
        has_registry_flags2         -- clm_registry_t contains flags2 field
        reg_rec_size                -- Region record size in bytes
        header_size                 -- BLOB header size in bytes
        footer_size                 -- BLOB footer size in bytes
        """
        self.version_string = version_string
        self.comment = comment
        self.registry_flags_allowed = registry_flags_allowed
        self.reg_rec_10_fl = reg_rec_10_fl
        self.ac_allowed = ac_allowed
        self.sub_chan_rules = sub_chan_rules
        self.apps_version = apps_version
        self.loc_reg_caps = loc_reg_caps
        self.per_antenna = per_antenna
        self.numrates_field = numrates_field
        self.max_bandwidth = max_bandwidth
        self.has_no_80mhz = has_no_80mhz
        self.per_bw_rs = per_bw_rs
        self.has_disabled_powers = has_disabled_powers
        self.has_default_for_cc = has_default_for_cc
        self.trim_ht3_to_ht = trim_ht3_to_ht
        self.patch_subchan_rules = patch_subchan_rules
        self.has_80_80 = has_80_80
        self.per_band_rate_sets = per_band_rate_sets
        self.has_user_string = has_user_string
        self.high_bw_combs = high_bw_combs
        self.content_dependent_reg = content_dependent_reg
        self.rev16 = rev16
        self.indexed_ccrev = indexed_ccrev
        self.supported_edcrs = supported_edcrs
        self.channel_flavors = channel_flavors
        self.has_dfs_tw = has_dfs_tw
        self.max_chains = max_chains
        self.has_ulb = has_ulb
        self.ext_rate_sets = ext_rate_sets
        self.dummy_regrev_remap = dummy_regrev_remap
        self.scr_idx_bits = scr_idx_bits
        self.scr_idx_initialize = scr_idx_initialize
        self.sub_chan_rules_40 = sub_chan_rules_40
        self.sub_chan_rules_inc = sub_chan_rules_inc
        self.sub_chan_rules_inc_separate = sub_chan_rules_inc_separate
        self.has_lo_gain_nbcal = has_lo_gain_nbcal
        self.has_chsprwar2 = has_chsprwar2
        self.has_registry_flags2 = has_registry_flags2
        self.reg_rec_size = reg_rec_size
        self.header_size = header_size
        self.footer_size = footer_size

    @staticmethod
    def get_supported_versions():
        """ Returns dictionary of all supported versions indexed by format version strings """
        ret = {}
        for vi in [
            BlobFormatVersionInfo(version_string="5.1", comment="Compatible with 4334a/b0, 4324a0. Limit of 252 locales of each type, no 802.11ac",
                                  registry_flags_allowed=False, reg_rec_10_fl=False, ac_allowed=False, sub_chan_rules=False, apps_version=False,
                                  loc_reg_caps=LocaleRegionCaps(txbf=False, has_1024_qam=False, vht_txbf0=False), per_antenna=False,
                                  numrates_field=False, max_bandwidth=Bandwidth._40, has_no_80mhz=False, per_bw_rs=False, has_disabled_powers=False,
                                  has_default_for_cc=False, trim_ht3_to_ht=False, patch_subchan_rules=True, has_80_80=False, per_band_rate_sets=False,
                                  has_user_string=False, high_bw_combs=False, content_dependent_reg=False, rev16=False, indexed_ccrev=False,
                                  supported_edcrs=[Edcrs.DEFAULT], channel_flavors=("Normal", "Japan", "0", "2"),
                                  has_dfs_tw=False, max_chains=3, has_ulb=False, ext_rate_sets=False, dummy_regrev_remap=False, scr_idx_bits=3,
                                  scr_idx_initialize=False, sub_chan_rules_40=False, sub_chan_rules_inc=False, sub_chan_rules_inc_separate=False,
                                  has_lo_gain_nbcal=False, has_chsprwar2=False, has_registry_flags2=False, reg_rec_size=7, header_size=88, footer_size=52),
            BlobFormatVersionInfo(version_string="7.0", comment="Compatible with 43241b0. Has 802.11ac rates, no TXBF rates, per-antenna limits, per-bandwidth rate sets and channel ranges",
                                  registry_flags_allowed=True, reg_rec_10_fl=True, ac_allowed=True, sub_chan_rules=False, apps_version=False,
                                  loc_reg_caps=LocaleRegionCaps(txbf=False, has_1024_qam=False, vht_txbf0=False), per_antenna=False,
                                  numrates_field=False, max_bandwidth=Bandwidth._80, has_no_80mhz=False, per_bw_rs=False, has_disabled_powers=False,
                                  has_default_for_cc=False, trim_ht3_to_ht=False, patch_subchan_rules=True, has_80_80=False, per_band_rate_sets=False,
                                  has_user_string=False, high_bw_combs=False, content_dependent_reg=False, rev16=False, indexed_ccrev=False,
                                  supported_edcrs=[Edcrs.DEFAULT], channel_flavors=("Normal", "Japan", "0", "2"),
                                  has_dfs_tw=False, max_chains=3, has_ulb=False, ext_rate_sets=False, dummy_regrev_remap=False,    scr_idx_bits=3,
                                  scr_idx_initialize=False, sub_chan_rules_40=False, sub_chan_rules_inc=False, sub_chan_rules_inc_separate=False,
                                  has_lo_gain_nbcal=False, has_chsprwar2=False, has_registry_flags2=False, reg_rec_size=9, header_size=88, footer_size=56),
            BlobFormatVersionInfo(version_string="10.0", comment="Compatible with 4335b0. Has TXBF rates and per-antenna limits, no per-bandwidth rate sets, disabled powers, default regions, and channel limits (i.e. requires filtering on latest CLM data)",
                                  registry_flags_allowed=True, reg_rec_10_fl=True, ac_allowed=True, sub_chan_rules=True, apps_version=True,
                                  loc_reg_caps=LocaleRegionCaps(txbf=True, has_1024_qam=False, vht_txbf0=False), per_antenna=True,
                                  numrates_field=True, max_bandwidth=Bandwidth._160, has_no_80mhz=True, per_bw_rs=False, has_disabled_powers=False,
                                  has_default_for_cc=False, trim_ht3_to_ht=False, patch_subchan_rules=True, has_80_80=False, per_band_rate_sets=False,
                                  has_user_string=False, high_bw_combs=False, content_dependent_reg=False, rev16=False, indexed_ccrev=False,
                                  supported_edcrs=[Edcrs.DEFAULT], channel_flavors=("Normal", "Japan", "0", "2"),
                                  has_dfs_tw=False, max_chains=3, has_ulb=False, ext_rate_sets=False, dummy_regrev_remap=False, scr_idx_bits=3,
                                  scr_idx_initialize=False, sub_chan_rules_40=False, sub_chan_rules_inc=False, sub_chan_rules_inc_separate=False,
                                  has_lo_gain_nbcal=False, has_chsprwar2=False, has_registry_flags2=False, reg_rec_size=9, header_size=108, footer_size=64),
            BlobFormatVersionInfo(version_string="11.0", comment="Per-bandwidth channel range sets and rate sets (that solves problem of BLOB overflow), support for disabled powers and default regions. No 80+80 channels support, no per-band rate sets (will require filtering soon)",
                                  registry_flags_allowed=True, reg_rec_10_fl=True, ac_allowed=True, sub_chan_rules=True, apps_version=True,
                                  loc_reg_caps=LocaleRegionCaps(txbf=True, has_1024_qam=False, vht_txbf0=False), per_antenna=True,
                                  numrates_field=True, max_bandwidth=Bandwidth._160, has_no_80mhz=True, per_bw_rs=True, has_disabled_powers=True,
                                  has_default_for_cc=True, trim_ht3_to_ht=True, patch_subchan_rules=False, has_80_80=False, per_band_rate_sets=False,
                                  has_user_string=False, high_bw_combs=False, content_dependent_reg=False, rev16=False, indexed_ccrev=False,
                                  supported_edcrs=[Edcrs.DEFAULT], channel_flavors=("Normal", "Japan", "0", "2"),
                                  has_dfs_tw=False, max_chains=3, has_ulb=False, ext_rate_sets=False, dummy_regrev_remap=False, scr_idx_bits=3,
                                  scr_idx_initialize=False, sub_chan_rules_40=False, sub_chan_rules_inc=False, sub_chan_rules_inc_separate=False,
                                  has_lo_gain_nbcal=False, has_chsprwar2=False, has_registry_flags2=False, reg_rec_size=9, header_size=108, footer_size=88),
            BlobFormatVersionInfo(version_string="12.0", comment="Supports 80+80 channels and per-band rate sets (i.e. larger CLM data). No user string",
                                  registry_flags_allowed=True, reg_rec_10_fl=True, ac_allowed=True, sub_chan_rules=True, apps_version=True,
                                  loc_reg_caps=LocaleRegionCaps(txbf=True, has_1024_qam=False, vht_txbf0=False), per_antenna=True,
                                  numrates_field=True, max_bandwidth=Bandwidth._160, has_no_80mhz=True, per_bw_rs=True, has_disabled_powers=True,
                                  has_default_for_cc=True, trim_ht3_to_ht=True, patch_subchan_rules=False, has_80_80=True, per_band_rate_sets=True,
                                  has_user_string=False, high_bw_combs=False, content_dependent_reg=False, rev16=False, indexed_ccrev=False,
                                  supported_edcrs=[Edcrs.DEFAULT], channel_flavors=("Normal", "Japan", "HalfChannel", "CustomUlb", "0", "1", "2"),
                                  has_dfs_tw=False, max_chains=3, has_ulb=False, ext_rate_sets=False, dummy_regrev_remap=False, scr_idx_bits=3,
                                  scr_idx_initialize=False, sub_chan_rules_40=False, sub_chan_rules_inc=False, sub_chan_rules_inc_separate=False,
                                  has_lo_gain_nbcal=False, has_chsprwar2=False, has_registry_flags2=False, reg_rec_size=9, header_size=108, footer_size=96),
            BlobFormatVersionInfo(version_string="12.1", comment="Has user string. Doesn't have EDCRS support",
                                  registry_flags_allowed=True, reg_rec_10_fl=True, ac_allowed=True, sub_chan_rules=True, apps_version=True,
                                  loc_reg_caps=LocaleRegionCaps(txbf=True, has_1024_qam=False, vht_txbf0=False), per_antenna=True,
                                  numrates_field=True, max_bandwidth=Bandwidth._160, has_no_80mhz=True, per_bw_rs=True, has_disabled_powers=True,
                                  has_default_for_cc=True, trim_ht3_to_ht=True, patch_subchan_rules=False, has_80_80=True, per_band_rate_sets=True,
                                  has_user_string=True, high_bw_combs=False, content_dependent_reg=False, rev16=False, indexed_ccrev=False,
                                  supported_edcrs=[Edcrs.DEFAULT], channel_flavors=("Normal", "Japan", "HalfChannel", "CustomUlb", "0", "1", "2"),
                                  has_dfs_tw=False, max_chains=3, has_ulb=False, ext_rate_sets=False, dummy_regrev_remap=False, scr_idx_bits=3,
                                  scr_idx_initialize=False, sub_chan_rules_40=False, sub_chan_rules_inc=False, sub_chan_rules_inc_separate=False,
                                  has_lo_gain_nbcal=False, has_chsprwar2=False, has_registry_flags2=False, reg_rec_size=9, header_size=108, footer_size=100),
            BlobFormatVersionInfo(version_string="12.2", comment="Has EDCRS-EU support. Has 1021 locale count limit, 254 regrev limit, no Ukrainian channels support",
                                  registry_flags_allowed=True, reg_rec_10_fl=True, ac_allowed=True, sub_chan_rules=True, apps_version=True,
                                  loc_reg_caps=LocaleRegionCaps(txbf=True, has_1024_qam=False, vht_txbf0=False), per_antenna=True,
                                  numrates_field=True, max_bandwidth=Bandwidth._160, has_no_80mhz=True, per_bw_rs=True, has_disabled_powers=True,
                                  has_default_for_cc=True, trim_ht3_to_ht=True, patch_subchan_rules=False, has_80_80=True, per_band_rate_sets=True,
                                  has_user_string=True, high_bw_combs=False, content_dependent_reg=False, rev16=False, indexed_ccrev=False,
                                  supported_edcrs=[Edcrs.DEFAULT, Edcrs.EU], channel_flavors=("Normal", "Japan", "HalfChannel", "CustomUlb", "0", "1", "2"),
                                  has_dfs_tw=True, max_chains=3, has_ulb=False, ext_rate_sets=False, dummy_regrev_remap=False, scr_idx_bits=3,
                                  scr_idx_initialize=False, sub_chan_rules_40=False, sub_chan_rules_inc=False, sub_chan_rules_inc_separate=False,
                                  has_lo_gain_nbcal=False, has_chsprwar2=False, has_registry_flags2=False, reg_rec_size=9, header_size=108, footer_size=100),
            BlobFormatVersionInfo(version_string="15.0", comment="Has 4093 locale count limit, 65534 regrev limit, supports EDCRS-EU, Ukraine and narrow (ULB) channels, VHT10-11 and VHT_TXBF0 rates, 4TX rates. No more than 6 subchannel rules",
                                  registry_flags_allowed=True, reg_rec_10_fl=True, ac_allowed=True, sub_chan_rules=True, apps_version=True,
                                  loc_reg_caps=LocaleRegionCaps(txbf=True, has_1024_qam=True, vht_txbf0=True), per_antenna=True,
                                  numrates_field=True, max_bandwidth=Bandwidth._160, has_no_80mhz=True, per_bw_rs=True, has_disabled_powers=True,
                                  has_default_for_cc=True, trim_ht3_to_ht=True, patch_subchan_rules=False, has_80_80=True, per_band_rate_sets=True,
                                  has_user_string=True, high_bw_combs=True, content_dependent_reg=True, rev16=True, indexed_ccrev=True,
                                  supported_edcrs=[Edcrs.DEFAULT, Edcrs.EU], channel_flavors=("Normal", "Japan", "HalfChannel", "Ukraine", "CustomUlb", "0", "1", "2", "3"),
                                  has_dfs_tw=True, max_chains=4, has_ulb=True, ext_rate_sets=True, dummy_regrev_remap=True, scr_idx_bits=3,
                                  scr_idx_initialize=False, sub_chan_rules_40=False, sub_chan_rules_inc=False, sub_chan_rules_inc_separate=False,
                                  has_lo_gain_nbcal=False, has_chsprwar2=False, has_registry_flags2=False, reg_rec_size=9, header_size=108, footer_size=180),
            BlobFormatVersionInfo(version_string="16.0", comment="Supports up to 15 subchannel rules, does not support 40MHz subchannel rules",
                                  registry_flags_allowed=True, reg_rec_10_fl=True, ac_allowed=True, sub_chan_rules=True, apps_version=True,
                                  loc_reg_caps=LocaleRegionCaps(txbf=True, has_1024_qam=True, vht_txbf0=True), per_antenna=True,
                                  numrates_field=True, max_bandwidth=Bandwidth._160, has_no_80mhz=True, per_bw_rs=True, has_disabled_powers=True,
                                  has_default_for_cc=True, trim_ht3_to_ht=True, patch_subchan_rules=False, has_80_80=True, per_band_rate_sets=True,
                                  has_user_string=True, high_bw_combs=True, content_dependent_reg=True, rev16=True, indexed_ccrev=True,
                                  supported_edcrs=[Edcrs.DEFAULT, Edcrs.EU], channel_flavors=("Normal", "Japan", "HalfChannel", "Ukraine", "CustomUlb", "0", "1", "2", "3"),
                                  has_dfs_tw=True, max_chains=4, has_ulb=True, ext_rate_sets=True, dummy_regrev_remap=True, scr_idx_bits=4,
                                  scr_idx_initialize=True, sub_chan_rules_40=False, sub_chan_rules_inc=False, sub_chan_rules_inc_separate=False,
                                  has_lo_gain_nbcal=False, has_chsprwar2=False, has_registry_flags2=False, reg_rec_size=9, header_size=108, footer_size=180),
            BlobFormatVersionInfo(version_string="16.1", comment="Supports 40MHz subchannel rules, doesn't support power increments in subchannel rules",
                                  registry_flags_allowed=True, reg_rec_10_fl=True, ac_allowed=True, sub_chan_rules=True, apps_version=True,
                                  loc_reg_caps=LocaleRegionCaps(txbf=True, has_1024_qam=True, vht_txbf0=True), per_antenna=True,
                                  numrates_field=True, max_bandwidth=Bandwidth._160, has_no_80mhz=True, per_bw_rs=True, has_disabled_powers=True,
                                  has_default_for_cc=True, trim_ht3_to_ht=True, patch_subchan_rules=False, has_80_80=True, per_band_rate_sets=True,
                                  has_user_string=True, high_bw_combs=True, content_dependent_reg=True, rev16=True, indexed_ccrev=True,
                                  supported_edcrs=[Edcrs.DEFAULT, Edcrs.EU], channel_flavors=("Normal", "Japan", "HalfChannel", "Ukraine", "CustomUlb", "0", "1", "2", "3"),
                                  has_dfs_tw=True, max_chains=4, has_ulb=True, ext_rate_sets=True, dummy_regrev_remap=True, scr_idx_bits=4,
                                  scr_idx_initialize=True, sub_chan_rules_40=True, sub_chan_rules_inc=False, sub_chan_rules_inc_separate=False,
                                  has_lo_gain_nbcal=False, has_chsprwar2=False, has_registry_flags2=False, reg_rec_size=9, header_size=108, footer_size=184),
            BlobFormatVersionInfo(version_string="17.0", comment="Supports power increments in subchannel rules",
                                  registry_flags_allowed=True, reg_rec_10_fl=True, ac_allowed=True, sub_chan_rules=True, apps_version=True,
                                  loc_reg_caps=LocaleRegionCaps(txbf=True, has_1024_qam=True, vht_txbf0=True), per_antenna=True,
                                  numrates_field=True, max_bandwidth=Bandwidth._160, has_no_80mhz=True, per_bw_rs=True, has_disabled_powers=True,
                                  has_default_for_cc=True, trim_ht3_to_ht=True, patch_subchan_rules=False, has_80_80=True, per_band_rate_sets=True,
                                  has_user_string=True, high_bw_combs=True, content_dependent_reg=True, rev16=True, indexed_ccrev=True,
                                  supported_edcrs=[Edcrs.DEFAULT, Edcrs.EU], channel_flavors=("Normal", "Japan", "HalfChannel", "Ukraine", "CustomUlb", "0", "1", "2", "3"),
                                  has_dfs_tw=True, max_chains=4, has_ulb=True, ext_rate_sets=True, dummy_regrev_remap=True, scr_idx_bits=4,
                                  scr_idx_initialize=True, sub_chan_rules_40=True, sub_chan_rules_inc=True, sub_chan_rules_inc_separate=False,
                                  has_lo_gain_nbcal=False, has_chsprwar2=False, has_registry_flags2=False, reg_rec_size=9, header_size=108, footer_size=184),
            BlobFormatVersionInfo(version_string="18.0", comment="Supports up to 256 subchannel rules",
                                  registry_flags_allowed=True, reg_rec_10_fl=True, ac_allowed=True, sub_chan_rules=True, apps_version=True,
                                  loc_reg_caps=LocaleRegionCaps(txbf=True, has_1024_qam=True, vht_txbf0=True), per_antenna=True,
                                  numrates_field=True, max_bandwidth=Bandwidth._160, has_no_80mhz=True, per_bw_rs=True, has_disabled_powers=True,
                                  has_default_for_cc=True, trim_ht3_to_ht=True, patch_subchan_rules=False, has_80_80=True, per_band_rate_sets=True,
                                  has_user_string=True, high_bw_combs=True, content_dependent_reg=True, rev16=True, indexed_ccrev=True,
                                  supported_edcrs=[Edcrs.DEFAULT, Edcrs.EU], channel_flavors=("Normal", "Japan", "HalfChannel", "Ukraine", "CustomUlb", "0", "1", "2", "3"),
                                  has_dfs_tw=True, max_chains=4, has_ulb=True, ext_rate_sets=True, dummy_regrev_remap=True, scr_idx_bits=8,
                                  scr_idx_initialize=False, sub_chan_rules_40=True, sub_chan_rules_inc=True, sub_chan_rules_inc_separate=True,
                                  has_lo_gain_nbcal=False, has_chsprwar2=False, has_registry_flags2=False, reg_rec_size=9, header_size=108, footer_size=200),
            BlobFormatVersionInfo(version_string="18.1", comment="Supports LoGainNBCAL flag",
                                  registry_flags_allowed=True, reg_rec_10_fl=True, ac_allowed=True, sub_chan_rules=True, apps_version=True,
                                  loc_reg_caps=LocaleRegionCaps(txbf=True, has_1024_qam=True, vht_txbf0=True), per_antenna=True,
                                  numrates_field=True, max_bandwidth=Bandwidth._160, has_no_80mhz=True, per_bw_rs=True, has_disabled_powers=True,
                                  has_default_for_cc=True, trim_ht3_to_ht=True, patch_subchan_rules=False, has_80_80=True, per_band_rate_sets=True,
                                  has_user_string=True, high_bw_combs=True, content_dependent_reg=True, rev16=True, indexed_ccrev=True,
                                  supported_edcrs=[Edcrs.DEFAULT, Edcrs.EU], channel_flavors=("Normal", "Japan", "HalfChannel", "Ukraine", "CustomUlb", "0", "1", "2", "3"),
                                  has_dfs_tw=True, max_chains=4, has_ulb=True, ext_rate_sets=True, dummy_regrev_remap=True, scr_idx_bits=8,
                                  scr_idx_initialize=False, sub_chan_rules_40=True, sub_chan_rules_inc=True, sub_chan_rules_inc_separate=True,
                                  has_lo_gain_nbcal=True, has_chsprwar2=False, has_registry_flags2=False, reg_rec_size=9, header_size=108, footer_size=200),
            BlobFormatVersionInfo(version_string=_BLOB_VERSION, comment="Supports ChSprWar2 flag",
                                  registry_flags_allowed=True, reg_rec_10_fl=True, ac_allowed=True, sub_chan_rules=True, apps_version=True,
                                  loc_reg_caps=LocaleRegionCaps(txbf=True, has_1024_qam=True, vht_txbf0=True), per_antenna=True,
                                  numrates_field=True, max_bandwidth=Bandwidth._160, has_no_80mhz=True, per_bw_rs=True, has_disabled_powers=True,
                                  has_default_for_cc=True, trim_ht3_to_ht=True, patch_subchan_rules=False, has_80_80=True, per_band_rate_sets=True,
                                  has_user_string=True, high_bw_combs=True, content_dependent_reg=True, rev16=True, indexed_ccrev=True,
                                  supported_edcrs=[Edcrs.DEFAULT, Edcrs.EU], channel_flavors=("Normal", "Japan", "HalfChannel", "Ukraine", "CustomUlb", "0", "1", "2", "3"),
                                  has_dfs_tw=True, max_chains=4, has_ulb=True, ext_rate_sets=True, dummy_regrev_remap=True, scr_idx_bits=8,
                                  scr_idx_initialize=False, sub_chan_rules_40=True, sub_chan_rules_inc=True, sub_chan_rules_inc_separate=True,
                                  has_lo_gain_nbcal=True, has_chsprwar2=True, has_registry_flags2=False, reg_rec_size=9, header_size=108, footer_size=200),
        ]:
            ret[vi.version_string] = vi
        return ret

    @staticmethod
    def current_blob_format():
        """ Returns current BLOB format version information """
        return BlobFormatVersionInfo.get_supported_versions()[_BLOB_VERSION]

# =============================================================================


class FilterParams:
    """ Filtering parameters, as obtained from command line

    Attributes:
    command_lines               -- Command line parameters - vector of
                                   (configFileName, switches) tuples (first
                                   tuple is from command line, its
                                   configFileName is empty string)
    region_set                  -- Set of CcRev objects that identify regions to
                                   include to BLOB
    agg_set                     -- Set of CcRev objects that identify
                                   aggregations to include to BLOB
    ccrev_set                   -- Set of CcRev objects that identify regions
                                   and aggregations to include to BLOB
    max_chains                  -- Maximum number of TxChains in locales' rates
    max_bandwidth               -- Maximum channel bandwidth
    band                        -- Band restriction for locales
    abg                         -- Only include 802.11a/b/g rates
    ac                          -- Include 802.11ac rates
    txbf                        -- TXBF rates allowed
    dfs_tw                      -- Allows use of TW DFS
    per_antenna                 -- Per-antenna rates allowed
    ignore_missed               -- Do not break if aggregation uses undefined
                                   region or region uses undefined locale
    full_set                    -- Include CC/0 for all countries, except for
                                   those that contained in all included
                                   aggregations
    tag_filters                 -- List of TagFilter objects
    include_80_80               -- Include 80+80 limits
    ulb                         -- Allow ULB (narrow) channels
    include_1024_qam            -- Allow 1024 QAM (VHT10-11) rates
    vht_txbf0                   -- Allow VHT TXBF0 rates
    filtering_set               -- True if some filtering parameters were
                                   specified
    loc_reg_caps                -- Allowed region-related locale capabilities.
                                   Read-only attribute comprised of other
                                   appropriate attributes
    regrev_remap                -- Remap regrevs (lame way of 16-bit regrev
                                   support)
    scr_idx_4bit                -- True if subchannel rule index field has 4
                                   bits. Ignored on BLOB formats that have 4+
                                   bits by default
    scr_idx_8bit                -- True if subchannel rule index field has 8
                                   bits. Ignored on BLOB formats that have
                                   8+ bits by default
    sub_chan_rules_40           -- True if 40MHz subchannel rules shall be
                                   put to BLOB. Ignored on BLOB formats that
                                   natively support 40MHz subchannel rules
    sub_chan_rules_inc          -- True if subchannel rules contain power
                                   increments, that stored in BLOB along with
                                   bandwidths (this format is now deprecated).
                                   Ignored on BLOB formats that natively support
                                   power increments in subchannel rules
    sub_chan_rules_inc_separate -- True if subchannel rules contain power
                                   increments, that stored in BLOB separately
                                   from bandwidths bandwidths. Ignored on BLOB
                                   formats that natively support power
                                   increments in subchannel rules
    loc_idx_12bit               -- True if locale indices are 12-bit (used in
                                   flag-swap mode). Ignored for BLOB formats
                                   that natively support 12-bit locale indices
    blob_format                 -- BLOB format information
                                   (BlobFormatVersionInfo object)
    assume_all_regions          -- True means that if no region selection
                                   attributes switches specified then all
                                   regions shall be assumed
    """

    class TagFilter:
        """ Tag filtering 'and-clause'

        Contains sets of tag names that must be specified and must not be
        specified for it to be included into BLOB

        Attributes:
        source -- Source specification filter was built from
        """

        def __init__(self, source, separator, negator):
            """ Creates filter from --include switch value

            Arguments:
            source    -- --include switch value
            separator -- Tag separator character
            negator   -- Prefix for 'unwanted' tags
            """
            self.source = source
            self._required_tags = set()
            self._unwanted_tags = set()
            for tag in source.split(separator):
                if (tag != "") and (tag[0] == negator):
                    self._unwanted_tags.add(tag[1:])
                else:
                    self._required_tags.add(tag)

        def match(self, tag_set):
            """ True if region with given set of tags passes filtering criteria

            Argument:
            tag_set -- Set of region's tags
            Returns True if region with given set of tags passes filtering
            criteria
            """
            return self._required_tags.issubset(tag_set) and self._unwanted_tags.isdisjoint(tag_set)

    def __init__(self):
        """ Constructor

        Constructs as if no arguments were specified
        """
        self._command_lines = []
        self._region_set = set()
        self._agg_set = set()
        self._ccrev_set = set()
        self._max_chains = RatesInfo.get_max_chains()
        self._max_bandwidth = Bandwidth.maximum
        self._band = Band.ALL
        self._abg = False
        self._ac = True
        self._txbf = False
        self._dfs_tw = False
        self._per_antenna = True
        self._ignore_missed = False
        self._full_set = False
        self._tag_filters = []
        self._include_80_80 = True
        self._ulb = False
        self._include_1024_qam = False
        self._vht_txbf0 = False
        self._filtering_set = False
        self._regrev_remap = False
        self._scr_idx_4bit = False
        self._scr_idx_8bit = False
        self._sub_chan_rules_40 = False
        self._sub_chan_rules_inc = False
        self._sub_chan_rules_inc_separate = False
        self._loc_idx_12bit = False
        self._blob_format = BlobFormatVersionInfo.current_blob_format()
        self._assume_all_regions = False
        self.__ccrev_all = set([CcRev("all")])

    @staticmethod
    def filter_all(all_regions=True):
        """ Returns filter object that filters-in all CLM data """
        ret = FilterParams()
        if all_regions:
            ret.add_ccrev("ccrev", "all")
        ret.set_scalar_attr("txbf", True)
        ret.set_scalar_attr("dfs_tw", True)
        ret.set_scalar_attr("ulb", True)
        ret.set_scalar_attr("include_1024_qam", True)
        ret.set_scalar_attr("vht_txbf0", True)
        ret.add_command_line("", ("--ccrev all " if all_regions else "") + "--txbf --dfs_tw --1024_qam --vht_txbf0 --ulb_")
        return ret

    def add_ccrev(self, ccrev_type, ccrev_str):
        """ Adds region and/or aggregation name

        Arguments:
        ccrev_type -- CC/rev type: "region", "agg", "ccrev"
        ccrev_str  -- CC/rev as string, specified in command line
        """
        self._filtering_set = True
        getattr(self, "_" + ccrev_type + "_set").add(CcRev(ccrev_str))

    def add_tag_filter(self, tag_filter):
        """ Adds tag filter

        Arguments:
        tag_filter -- FilterParams.TagFilter object
        """
        self._filtering_set = True
        self._tag_filters.append(tag_filter)

    def set_scalar_attr(self, attr, value, explicit=True):
        """ Sets scalar attribute of this class

        Arguments:
        attr     -- Attribute name (without leading underscore)
        value    -- Value to set
        explicit -- True if called to set value, specified in command line,
                    false if value implicitly derived from other parameters
        """
        real_attr = "_" + attr
        if not hasattr(self, real_attr):
            raise ValueError("Internal error: unknown attribute %s" % attr)
        current_value = getattr(self, real_attr)
        if hasattr(current_value, "__len__") and not isinstance(current_value, (str, unicode)):
            raise ValueError("Internal error: set_scalar_attr() can't be used to set nonscalar value")
        if attr == "max_chains":
            value = int(value)
            if (value < 1) or (value > RatesInfo.get_max_chains()):
                raise ValueError("max_chains shall be in range 1..%d" % RatesInfo.get_max_chains())
        elif attr == "band":
            if value not in (Band._2, Band._5, Band.ALL):
                raise ValueError("Internal error: wrong band value %d" % value)
        elif isinstance(current_value, bool):
            value = bool(value)
        setattr(self, real_attr, value)
        self._filtering_set |= explicit

    def add_command_line(self, name, command_line):
        """ Adds switches from command line or config file

        Arguments:
        name         -- config file name. Empty for switches from command line
        command_line -- switches from command line or config file
        """
        self._command_lines.append((name, command_line))

    def pop_command_line(self):
        """ Removes recently added command line data """
        self._command_lines.pop()

    def __getattr__(self, name):
        """ Returns attributes """
        if name == "ccrev_set" and self._assume_all_regions and \
                (not (self._ccrev_set or self._agg_set or self._region_set or self._tag_filters)):
            return self.__ccrev_all
        if ("_" + name) in self.__dict__:
            return self.__dict__["_" + name]
        if name == "loc_reg_caps":
            return LocaleRegionCaps(txbf=self._txbf,
                                    has_1024_qam=self._include_1024_qam,
                                    vht_txbf0=self._vht_txbf0)
        raise AttributeError("Internal error: attribute \"%s\" not defined" % name)

# =============================================================================


class AvailableFilters:
    """ Information about available filters

    Attributes:
    ccrevs  -- Set of region/aggregation ccrevs
    tags    -- Set of region tags
    regions -- Set of ccrevs of regions that match filtering criteria
    """

    def __init__(self):
        """ Constructor """
        self.ccrevs = set()
        self.tags = set()
        self.regions = set()

    def get_sorted_ccrevs(self):
        """ Returns list of sorted ccrevs """
        ret = list(self.ccrevs)
        ret.sort(key=methodcaller("sort_key"))
        return ret

    def get_sorted_regions(self):
        """ Returns list of sorted region ccrevs """
        ret = list(self.regions)
        ret.sort(key=methodcaller("sort_key"))
        return ret

    def get_sorted_tags(self):
        """ Returns list of sorted tags """
        ret = list(self.tags)
        ret.sort()
        return ret

# =============================================================================


class Options:
    """ Non-filtering command line options

    Attributes:
    print_version        -- True means that version information shall be
                            printed
    print_options        -- True means that specified options shall be printed
    verbose              -- True for verbose output
    obfuscate            -- Obfuscate comments from generated BLOB
    list_values          -- Switches whose possible values shall be printed
    blob_file_name       -- Name of output C BLOB file
    apitest_listing_name -- Name of file for apitest integrity checking output
    compare              -- None or name of file where to base/final comparison
                            log shall be printed
    force_section        -- Name of section to force data to or None
    user_string          -- User string to store in BLOB or None
    base_user_string     -- Base BLOB user string or None. Used in --apitest
    clmapi_include_dir   -- Directory where wlc_clm_data.h located
    bcmwifi_include_dir  -- Directory where bcmwifi_rates.h located
    """

    possible_list_values = ("region", "tag")  # Possible values for --list switch

    def __init__(self):
        """ Default constructor - initializes everything to 'not specified state """
        self._print_version = False
        self._print_options = False
        self._verbose = False
        self._obfuscate = True
        self._list_values = []
        self._blob_file_name = None
        self._apitest_listing_name = None
        self._compare = None
        self._force_section = None
        self._user_string = None
        self._base_user_string = None
        self._clmapi_include_dir = None
        self._bcmwifi_include_dir = None

    def set_option(self, name, value):
        """ Sets given option to given value.

        Arguments:
        name  -- Option (attribute) name
        value -- Value to assign to (append to, if option is a list) option
        """
        member_name = "_" + name
        if not hasattr(self, member_name):
            raise KeyError("Internal error: Wrong option name \"%s\"" % name)
        member = getattr(self, member_name)
        if isinstance(member, list):
            member.append(value)
        elif (member is None) or isinstance(member, bool) or (member == value):
            setattr(self, member_name, value)
        else:
            raise ValueError("Option already set")

    def __getattr__(self, name):
        """ Returns value of given option """
        if (name == "user_string") and (self._user_string is None):
            self._user_string = datetime.datetime.today().strftime("%Y-%m-%d %H:%M:%S")
        if ("_" + name) in self.__dict__:
            return self.__dict__["_" + name]
        raise AttributeError("Internal error: option \"%s\" not defined" % name)

# =============================================================================


class ClmData:
    """ Container for data to be included to BLOB (filtered XML data)

    Public attributes (read only):
    clm_format_version       -- CLM data file format version string (empty if
                                unknown)
    generator_name           -- Name of generator tool (empty if unknown)
    generator_version        -- Generator tool version string (empty if
                                unknown)
    clm_version              -- CLM data version (empty if unknown)
    clm_base_version         -- Version of base (inc-from) CLM data (None if
                                irrelevant)
    locales                  -- List of locale objects sorted by locale id.
                                Always up to date
    regions                  -- List of region objects sorted by CcRev. Always
                                up to date
    aggregations             -- List of aggregation objects sorted by CcRev.
                                Always up to date
    restricted_sets          -- List of restricted sets sorted by ID. Always up
                                to date
    sub_chan_rules           -- List of subchannel rules, sorted by name.
                                Always up to date
    valid_channels           -- ValidChannels object. Always up to date
    channel_ranges_per_bw    -- Dictionary that contains lists of unique
                                channel ranges, indexed by channel bandwidth.
                                Also channel_ranges_per_bw[None] contains list
                                of all channel ranges. Computed by update()
    locale_channel_sets      -- Sorted list of unique same-bandwidth locale
                                channel sets. Every list item is set of channel
                                ranges. Computed by update()
    rate_sets_per_bw_band    -- Per main/extended then per bandwidth then per
                                band dictionary of lists of rate frozensets,
                                used by channel power items (every list item is
                                a frozenset of rate infos).
                                Also rate_sets_per_bw_band[False][None][None]
                                contains list of all main rate sets,
                                rate_sets_per_bw_band[False][bw][None] contains
                                list of all main rate sets for given bandwidth.
                                Computed by update().
                                Rate sets for ULB channels use 20MHz bw
                                If extended rate sets not used
                                rate_sets_per_bw_band[True] is nonexistent
    rate_set_idx_per_bw_band -- Per main/extended (boolean) then per bandwidth
                                then per band then per rate frozenset
                                dictionary that maps rate frozensets to their
                                indices in corresponded lists in
                                rate_sets_per_bw_band (with same semantics of
                                [None] for band and bandwidth. Computed by
                                update()
                                Rate sets for ULB channels use 20MHz bw
                                If extended rate sets not used
                                rate_set_idx_per_bw_band[True] is nonexistent
    rate_sets_splitter       -- Maps frozensets (used by channel power items)
                                to two-element tuples: first element is a set
                                of main (1TX-3TX) rate frozensets, second
                                element is a set of extended (3TX-4TX) rate
                                frozensets
    rate_sets_fully_split    -- True means that all attempts to fit number of
                                rate sets below threshold are made and hence
                                approaching rate sets' number limit shall be
                                diagnozed. Computed by update()
    adcs                     -- List of advertised countries, sorted by CC.
                                Always up to date
    used_channel_combs       -- Collection of valid channels used by locales.
                                Per-channel-type dictionary of sets of
                                ChannelComb objects
    deleted_regions          -- List of tuples. Each tuple contains CcRev of
                                region that shall be marked as deleted (present
                                in base (inc-from) CLM data but absent in final
                                data) and difference report (DiffReport
                                object). Sorted by CcRev in update()
    preserved_regions        -- List of filtered-out regions that were
                                preserved (not deleted) in incremental BLOB
                                because they are equal to same-named regions in
                                final CLM data
    deleted_aggregations     -- List of tuples. Each tuple contains CcRev of
                                aggregation CcRevs that shall be marked as
                                deleted (present in base CLM data but absent in
                                final data) and difference report (DiffReport
                                object). Sorted by CcRev in update()
    deleted_adcs             -- List of tuples. Each tuple contains CC of
                                advertised country CCs that shall be marked as
                                deleted (present in base CLM data but absent in
                                final data) and difference report (DiffReport
                                object). Sorted by CC in update()
    preserved_adcs           -- List of filtered-out advertised countries' CCs
                                that were preserved (not deleted) in
                                incremental BLOB because they are equal to
                                same-named regions in final CLM data
    empty                    -- Contains no locale/region/aggregation data
    is_incremental           -- True for if data is incremental, false if whole
                                (independent)
    contains_ht20_siso       -- True means that data contains HT20 SISO power
                                information, false means that this information
                                shall be derived
    ccrev_aliases            -- Dictionary that maps contained CcRevs to unique
                                C-compatible aliases. Computed in update()
    extra_ccrevs             -- Sorted list of ccrevs used in mapping lists and
                                lists of advertised CCs and not mentioned in
                                regions' table and table of deleted regions
    has_rev16                -- True if CLM data contains 16-bit regrevs.
                                Computed in update()
    regrev_remap_allowed     -- True if regrev remap allowed. Always up to date
    regrev_remap_required    -- True if BLOB requires regrev remap (remap
                                allowed, 16-bit regrevs present). Computed in
                                update()
    regrev_remap_table       -- 16->8 remap. Dictionary indexed by CC. Elements
                                are dictionaries of external (16-bit) regrevs
                                mapped to internal (8-bit) regrevs. Computed in
                                update()
    regrev_remap_as_base     -- True if remap is the same as remap in base CLM
                                data. For nonincremental CLM data always false
    multiple_regrevs_ccs     -- Sorted list of CCs that has multiple regrevs.
                                Computed in update()
    #equal_locale_ids         -- Maps each locale ID to set of IDs of equivalent
    #                            locales
    #blob_locale_id           -- Maps each locale ID to ID of that or equivalent
    #                            locale that goes to BLOB
    ranges_split             -- True means that channel ranges split was
                                performed
    """

    # Effective bandwidth used in rate_set... indices for given
    # bandwidth. Maps ULB bandwidths to 20MHz, other bandwidths - to self
    rate_set_bw_mapper = dict((bw, Bandwidth._20 if Bandwidth.is_ulb(bw) else bw) for bw in Bandwidth.all())

    # Minimum chain count for rates included into extended rate set
    EXT_RATE_MIN_CHAINS = 3

    # If there are rates with this chain count - extended rate sets shall be
    # created
    EXT_RATES_CHAINS_THRESHOLD = 4

    # If for some channel type number of rate sets reaches this threshold -
    # extended rate sets shall be created
    EXT_RATES_COUNT_THRESHOLD = 256

    # If number of channel ranges belonging to one band exceed this threshold -
    # channel ranges shall be split
    CHANNEL_RANGES_SPLIT_THRESHOLD = 255

    # Type and channel to use for channel split
    CHANNEL_RANGES_SPLIT_CHAN = (ChannelType.create(Band._5, Bandwidth._20), 100)

    def __init__(self):
        """ Constructor. Creates empty container """
        self._clm_format_version = ""
        self._generator_name = ""
        self._generator_version = ""
        self._clm_version = ""
        self._clm_base_version = None
        self._apps_versions = {}
        self._locales_dict = {}
        self._regions_dict = {}
        self._aggregations_dict = {}
        self._restricted_sets_dict = {}
        self._sub_chan_rules_dict = {}
        self._adcs_dict = {}
        self._valid_channels = ValidChannels()
        self._channel_ranges_per_bw = None
        self._locale_channel_sets_list = None
        self._used_channel_combs = None
        self._deleted_regions_list = []
        self._deleted_aggregations_list = []
        self._deleted_adcs_list = []
        self._preserved_regions_list = []
        self._preserved_adcs_list = []
        self._is_incremental = False
        self._contains_ht20_siso = True
        self._ccrev_aliases = None
        self._extra_ccrevs_list = None
        self._has_rev16 = False
        self._regrev_remap_allowed = False
        self._regrev_remap_as_base = False
        self._multiple_regrevs_ccs = []
        self._ranges_split = False

    def __getattr__(self, name):
        """ Returns values of public readonly attributes """
        if ("_" + name) in self.__dict__:
            return self.__dict__["_" + name]
        if ("_" + name + "_list") in self.__dict__:
            return self.__dict__["_" + name + "_list"]
        if ("_" + name + "_dict") in self.__dict__:
            ret = list(self.__dict__["_" + name + "_dict"].values())
            ret.sort(key=methodcaller("sort_key"))
            return ret
        if name == "empty":
            return not (self._locales_dict or self._regions_dict or self._aggregations_dict)
        if name == "apps_version":
            return self.get_apps_version()
        raise AttributeError("Internal error: attribute \"%s\" not defined" % name)

    def set_clm_version(self, clm_version):
        """ Sets CLM XML data version """
        self._clm_version = clm_version

    def set_clm_base_version(self, clm_base_version):
        """ Sets version of base CLM data """
        self._clm_base_version = clm_base_version

    def set_apps_version(self, apps_version, is_base=False):
        """ Sets apps version

        Arguments:
        is_base -- True for setting base apps version, false for setting
        final (effective) apps version
        """
        self._apps_versions[is_base] = apps_version

    def set_clm_format_version(self, clm_format_version):
        """ Sets CLM data format version """
        self._clm_format_version = clm_format_version

    def set_generator_info(self, generator_name, generator_version):
        """ Sets information about application that generated CLM data """
        self._generator_name = generator_name
        self._generator_version = generator_version

    def set_regrev_remap_allowed(self, state):
        """ Sets if regrev remap is allowed """
        self._regrev_remap_allowed = state

    def add_locale(self, locale):
        """ Adds given locale """
        self._locales_dict[locale.loc_id] = locale
        locale.restricted_set_dict = self._restricted_sets_dict

    def add_region(self, region):
        """ Adds given region, updates advertised country list """
        self._regions_dict[region.ccrev] = region
        region.locale_dict = self._locales_dict
        region.sub_chan_rules_dict = self._sub_chan_rules_dict
        if region.ad_c is not None:
            if region.ad_c not in self._adcs_dict:
                self._adcs_dict[region.ad_c] = AdvertisedCountry(region.ad_c)
            self._adcs_dict[region.ad_c].add_region(region.ccrev)

    def add_aggregation(self, aggregation):
        """ Adds given aggregation """
        self._aggregations_dict[aggregation.ccrev] = aggregation

    def add_restricted_set(self, restricted_set):
        """ Adds given restricted set """
        self._restricted_sets_dict[restricted_set.name] = restricted_set

    def add_sub_chan_rules(self, sub_chan_rules):
        """ Adds given subchannel rules """
        self._sub_chan_rules_dict[sub_chan_rules.name] = sub_chan_rules

    def add_deleted_region(self, ccrev, diff_report):
        """ Adds CcRev of deleted region """
        self._deleted_regions_list.append((ccrev, diff_report))

    def add_deleted_aggregation(self, ccrev, diff_report):
        """ Adds CcRev of deleted aggregation """
        self._deleted_aggregations_list.append((ccrev, diff_report))

    def add_deleted_adc(self, cc, diff_report):
        """ Adds CC of deleted advertised country code """
        self._deleted_adcs_list.append((cc, diff_report))

    def add_preserved_region(self, ccrev):
        """ Adds CcRev of preserved region """
        self._preserved_regions_list.append(ccrev)

    def add_preserved_adc(self, cc):
        """ Adds CC of preserved advertised country code """
        self._preserved_adcs_list.append(cc)

    def remove_region(self, region_ccrev):
        """ Removes region by CcRev """
        del self._regions_dict[region_ccrev]

    def remove_aggregation(self, aggregation_ccrev):
        """ Removes aggregation by its default CcRev """
        del self._aggregations_dict[aggregation_ccrev]

    def remove_adc(self, adc_cc):
        """ Removes advertised country by its country code """
        del self._adcs_dict[adc_cc]

    def remove_unused_items(self):
        """ Removes locales and subchannel rules not used by regions and
        restricted sets not used by locales
        """
        used_locale_ids_set = set()
        sub_chan_max_bw_dict = {}
        for region in self._regions_dict.values():
            for loc_type, loc_id in region.locale_ids.items():
                if loc_type in region.same_locales:
                    continue
                used_locale_ids_set.add(loc_id)
            if region.sub_chan_rules_name is not None:
                max_bandwidth = region.get_max_bandwidth()
                if max_bandwidth < Bandwidth._40:
                    region.sub_chan_rules_name = None
                else:
                    sub_chan_max_bw_dict[region.sub_chan_rules_name] = \
                        max(max_bandwidth, sub_chan_max_bw_dict.get(region.sub_chan_rules_name, max_bandwidth))
        unused_locale_ids_set = set(self._locales_dict.keys()) - used_locale_ids_set
        for loc_id in unused_locale_ids_set:
            del self._locales_dict[loc_id]
        for name in (set(self._sub_chan_rules_dict.keys()) - set(sub_chan_max_bw_dict.keys())):
            del self._sub_chan_rules_dict[name]
        for name, max_bandwidth in sub_chan_max_bw_dict.items():
            self._sub_chan_rules_dict[name].trim_to_max_bandwidth(max_bandwidth)
        used_restricted_set_names_set = set()
        for locale in self._locales_dict.values():
            if locale.restricted_set_name is not None:
                used_restricted_set_names_set.add(locale.restricted_set_name)
        unused_restricted_set_names_set = set(self._restricted_sets_dict.keys()) - used_restricted_set_names_set
        for name in unused_restricted_set_names_set:
            del self._restricted_sets_dict[name]

    def remove_used_channel_combs(self):
        """ Empties collection of used channel combs """
        for band in self._used_channel_combs:
            self._used_channel_combs[band] = set()

    def get_apps_version(self, is_base=False, verbatim=True):
        """ Returns apps version

        Arguments:
        is_base  -- True if base apps version shall be returned (note that it
                    is being set only in CLM data returned by
                    ClmDataContainer.get_blob_data() and not set in CLM data returned by
                    ClmDataContainer.get_data() and ClmDataContainer.get_base_data()
        verbatim -- True if apps version shall be returned as is, false if None
                    shall be returned if apps version set to default value
        Returns apps version or None
        """
        ret = self._apps_versions.get(is_base)
        return ret if verbatim or (ret != _IGNORED_APPS_VERSION) else None

    def get_locale(self, loc_id):
        """ Returns locale by its id (None if not found)"""
        return self._locales_dict.get(loc_id, None)

    def get_region(self, ccrev):
        """ Returns region by its CcRev (None if not found) """
        return self._regions_dict.get(ccrev, None)

    def get_adc(self, cc):
        """ Returns advertised country by its CC (None if not found) """
        return self._adcs_dict.get(cc)

    def get_aggregation(self, ccrev):
        """ Returns aggregation by its CcRev (None if not found) """
        return self._aggregations_dict.get(ccrev, None)

    def get_restricted_set(self, name):
        """ Returns restricted set by its ID (None if not found) """
        return self._restricted_sets_dict.get(name, None)

    def get_sub_chan_rules(self, name):
        """ Returns subchannel rules by name (None if not found) """
        return self._sub_chan_rules_dict.get(name, None)

    def has_sub_chan_rules_for(self, channel_type):
        """ True if there are subchannel rules for given main channel
        channel type """
        for rule in self._sub_chan_rules_dict.values():
            if rule.get_num_channel_type_rules(channel_type):
                return True
        return False

    def has_locales(self, loc_type):
        """ True if there are locales of given type """
        for locale in self._locales_dict.values():
            if loc_type == locale.locale_type:
                return True
        return False

    def set_is_incremental(self, value):
        """ Sets if this is incremental or independent (whole) data """
        self._is_incremental = bool(value)

    def set_contains_ht20_siso(self, value):
        """ Sets if contains HT20 SISO power data """
        self._contains_ht20_siso = bool(value)

    def is_restricted_channel(self, locale, channel):
        """ True if given channel belongs to restricted set defined in given locale
        """
        if locale.restricted_set_name is None:
            return False
        rs = self._restricted_sets_dict.get(locale.restricted_set_name, None)
        if rs is None:
            return False
        if rs.is_all_channels:
            return True
        for chan_range in rs.channels:
            if self._valid_channels.is_channel_in_range(channel, chan_range):
                return True
        return False

    def is_radar_channel(self, locale, channel):
        """ True if given channel is radar-sensitive channel """
        if locale.dfs == Dfs.NONE:
            return False
        if locale.dfs == Dfs.TW:
            return 100 <= channel <= 144
        return (52 <= channel <= 64) or (100 <= channel <= 144)

    def update(self, allow_ext_rate_sets, base_clm_data=None):
        """ Updates attributes: channel_ranges, locale_channel_sets,
        power_rate_sets. Updates channel_set_id attributes of base locales.

        Arguments:
        allow_ext_rate_sets -- True allows use of extended rate sets (if
                               needed)
        base_clm_data       -- For incremental BLOB - base ClmData object,
                               None for other nonincremental BLOB
        """
        # Removes unused locales and locale-dependent data
        self.remove_unused_items()

        # Computes BLOB aliases and has_siblings flags for locale names
        sibling_counts = {}
        for locale in self._locales_dict.values():
            locale.blob_alias = None
            sibling_counts.setdefault(locale.loc_id.name, 0)
            sibling_counts[locale.loc_id.name] += 1
        blob_aliases = set()
        for locale in self._locales_dict.values():
            if sibling_counts[locale.loc_id.name] > 1:
                locale.has_siblings = True
            alias = locale.get_unique_name()
            alias = alias.replace("&", "__")
            alias = re.sub(r"[^_a-zA-Z0-9]", "_", alias)
            if alias in blob_aliases:
                i = 1
                while (alias + "_" + str(i)) in blob_aliases:
                    i += 1
                alias += "_" + str(i)
            locale.blob_alias = alias
            blob_aliases.add(alias)

        # # Computing equivalent locales
        # # Maps locale data hashes to lists of sets of locale IDs. All locales
        # # in one set are equivalent
        # equal_locale_ids = {}
        # for locale in self._locales_dict.itervalues():
        #     candidate_sets = equal_locale_ids.setdefault(locale.data_hash(), [])
        #     for candidate_set in candidate_sets:
        #         sample_id = next(iter(candidate_set))
        #         if locale.data_equal(self._locales_dict[sample_id]):
        #             candidate_set.add(locale.loc_id)
        #             break
        #     else:
        #         candidate_sets.append(set([locale.loc_id]))
        # self._equal_locale_ids = {}
        # self._blob_locale_id = {}
        # for locale_id_set_list in equal_locale_ids.itervalues():
        #     for locale_id_set in locale_id_set_list:
        #         representative_id = next(iter(locale_id_set))
        #         for locale_id in locale_id_set:
        #             self._equal_locale_ids[locale_id] = locale_id_set
        #             self._blob_locale_id[locale_id] = representative_id

        self._update_ccrev_aliases()

        # Updating regrev_remap and has_rev16
        self._regrev_remap_table = {}
        # CCrevs that may meaningfully have 16-bit regrevs
        all_ccrevs_set = self._get_ccrevs_set(regions=True, deleted_regions=True, aggregations=True,
                                              deleted_aggregations=True, mappings=True, adcs=True)
        self._has_rev16 = any(map(lambda cr: cr.rev16, all_ccrevs_set))
        if self._regrev_remap_allowed:
            # Collecting all CCs that have 16-bit regrevs
            for ccrev in all_ccrevs_set:
                if ccrev.rev16:
                    self._regrev_remap_table[ccrev.cc] = {}
            self._regrev_remap_required = bool(self._regrev_remap_table)
            if (base_clm_data is not None) and (self._regrev_remap_required or base_clm_data._regrev_remap_required):
                # Adding remaps from base - in two stages
                self._regrev_remap_required = True
                for cc, remap in base_clm_data._regrev_remap_table.iteritems():
                    self._regrev_remap_table[cc] = copy.deepcopy(remap)
                for ccrev in base_clm_data._get_ccrevs_set(regions=True, deleted_regions=True, aggregations=True,
                                                           deleted_aggregations=True, mappings=True, adcs=True):
                    if (ccrev.cc in self._regrev_remap_table) and (ccrev.cc not in base_clm_data._regrev_remap_table):
                        self._regrev_remap_table[ccrev.cc][ccrev.rev] = ccrev.rev
            used_maps = {}
            for cc, remap in self._regrev_remap_table.iteritems():
                used_maps[cc] = set(remap.itervalues())
            for ccrev in sorted(all_ccrevs_set, key=lambda cr: (cr.cc, cr.rev)):
                if (ccrev.cc not in self._regrev_remap_table) or (ccrev.rev in self._regrev_remap_table[ccrev.cc]):
                    continue
                idx = 0
                while idx in used_maps[ccrev.cc]:
                    idx += 1
                self._regrev_remap_table[ccrev.cc][ccrev.rev] = idx
                used_maps[ccrev.cc].add(idx)
        else:
            self.regrev_remap_required = False
        self._regrev_remap_as_base = (base_clm_data is not None) and (base_clm_data._regrev_remap_table == self._regrev_remap_table)

        # Computing multiple_regrevs_ccs
        ccs = {}
        for ccrev in self._get_ccrevs_set(regions=True, aggregations=True):
            ccs.setdefault(ccrev.cc, set()).add(ccrev.rev)
        self._multiple_regrevs_ccs = sorted(filter(lambda cc: len(ccs[cc]) > 1, ccs.iterkeys()))

        # Creates list of all channel ranges (first without split, then splitting)
        for split_args in [None, ClmData.CHANNEL_RANGES_SPLIT_CHAN]:
            self._ranges_split = split_args is not None
            ranges_set = set()
            for locale in self._locales_dict.values():
                if split_args is not None:
                    locale.split_ranges(*split_args)
                for power in locale.reg_power:
                    ranges_set.add(power.channel_range)
                for power in locale.chan_power:
                    ranges_set.add(power.channel_range)
                for channel_set in locale.channel_sets.values():
                    for cr in channel_set:
                        ranges_set.add(cr)
            for restricted_set in self._restricted_sets_dict.values():
                if split_args is not None:
                    restricted_set.split_ranges(self._valid_channels, *split_args)
                for chan_range in restricted_set.channels:
                    ranges_set.add(chan_range)
            for sub_chan_rules in self._sub_chan_rules_dict.values():
                if split_args is not None:
                    sub_chan_rules.split_ranges(self._valid_channels, *split_args)
                for chan_range in sub_chan_rules.chan_rules_dict.keys():
                    ranges_set.add(chan_range)
            self._channel_ranges_per_bw = {}
            self._channel_ranges_per_bw[None] = list(ranges_set)
            self._channel_ranges_per_bw[None].sort(key=methodcaller("sort_key"))
            for bw in Bandwidth.all():
                self._channel_ranges_per_bw[bw] = []
            for cr in self._channel_ranges_per_bw[None]:
                self._channel_ranges_per_bw[cr.channel_type.bandwidth].append(cr)
            if max(len(self._channel_ranges_per_bw[bw]) for bw in Bandwidth.all()) \
                    <= ClmData.CHANNEL_RANGES_SPLIT_THRESHOLD:
                break

        # Creates sorted list of channel sets and initializes channel set IDs in locales
        set_of_range_sets = set()
        for locale in self._locales_dict.values():
            if locale.locale_type.is_base:
                set_of_range_sets.add(locale.channel_sets[Bandwidth._20])
        self._locale_channel_sets_list = []
        for crs in set_of_range_sets:
            self._locale_channel_sets_list.append(crs)
        self._locale_channel_sets_list.sort(key=lambda crs: ChannelRange.range_set_sort_key(crs))
        range_set_ids = {}
        # I do not have even a slightest idea why IronPython's range doesn't work here. But it really doesn't!
        i = 0
        while i < len(self._locale_channel_sets_list):
            range_set_ids[self._locale_channel_sets_list[i]] = i
            i += 1
        for locale in self._locales_dict.values():
            if locale.locale_type.is_base:
                locale.channel_set_id = range_set_ids[locale.channel_sets[Bandwidth._20]]

        # Creates rate sets' indices
        # Assume extended rate sets are not needed
        use_ext_rate_sets = False
        if allow_ext_rate_sets:
            # If extended rate sets are allowed and there are 4TX rates - extended rate sets are needed
            for locale in self._locales_dict.values():
                if locale.max_chains >= ClmData.EXT_RATES_CHAINS_THRESHOLD:
                    use_ext_rate_sets = True
                    break
        # Building rate sets indices using established decision
        self._rate_sets_fully_split = False
        main_ok, ext_ok = self._build_rate_set_indices(use_ext_rate_sets=use_ext_rate_sets, split_main_rates=False, split_ext_rates=False)
        if (not main_ok) and (not use_ext_rate_sets) and allow_ext_rate_sets:
            use_ext_rate_sets = True
            main_ok, ext_ok = self._build_rate_set_indices(use_ext_rate_sets=use_ext_rate_sets, split_main_rates=False, split_ext_rates=False)
        if not (main_ok and ext_ok):
            self._build_rate_set_indices(use_ext_rate_sets=use_ext_rate_sets, split_main_rates=not main_ok, split_ext_rates=not ext_ok)
            self._rate_sets_fully_split = True

        # Computes channel combs and locales' channel sets
        self._valid_channels.trim(ranges_set)
        self._used_channel_combs = {}
        for channel_type in ChannelType.all():
            self._used_channel_combs[channel_type] = set()
            for channel_flavor in self._valid_channels.get_channel_flavors(channel_type):
                channels = list(self._valid_channels.get_channels_of_type(channel_type,
                                                                          channel_flavor))
                step = ChannelType.channel_step(channel_type)
                while channels:
                    start = channels.pop(0)
                    if ChannelType.is_80_80(start, channel_type):
                        continue
                    num = 1
                    while (start + num * step) in channels:
                        channels.remove(start + num * step)
                        num += 1
                    self._used_channel_combs[channel_type].add(ChannelComb(start, num, step))

        # Sorts top-level lists that promised to be sorted
        self._deleted_regions_list.sort(key=lambda dr: dr[0].sort_key())
        self._deleted_aggregations_list.sort(key=lambda da: da[0].sort_key())
        self._deleted_adcs_list.sort(key=lambda da: da[0])

    def _build_rate_set_indices(self, use_ext_rate_sets, split_main_rates, split_ext_rates):
        """ Creates sorted lists of rate sets used in locales and initializes
        rate set ids used in locales' powers

        Arguments:
        use_ext_rate_sets -- True means that extended rate sets shall be used
        split_main_rates  -- Split main rate sets. This reduces overall number
                             of rate sets at a cost of increasing number of
                             chan_power records in BLOB (and hence leads to
                             bigger, slower to read BLOB)
        split_ext_rates   -- Split extended rate sets. Effects are as above
        Returns two-elemenst tuple (main_rates_below_threshold,
        ext_rates_below_threshold). Each element is true if rates of
        correspondent type are below EXT_RATES_COUNT_THRESHOLD
        """
        self._rate_sets_per_bw_band = {}
        self._rate_set_idx_per_bw_band = {}
        self._rate_sets_splitter = {}
        effective_bw_list = [None] + list(ClmData.rate_set_bw_mapper.values())
        for is_ext in [False, True] if use_ext_rate_sets else [False]:
            self._rate_sets_per_bw_band[is_ext] = {}
            self._rate_set_idx_per_bw_band[is_ext] = {}
            for bw in effective_bw_list:
                self._rate_sets_per_bw_band[is_ext][bw] = {}
                self._rate_set_idx_per_bw_band[is_ext][bw] = {}
                for band in [None] + list(Band.all()):
                    if (band is None) or (bw is not None):
                        self._rate_sets_per_bw_band[is_ext][bw][band] = set()
                        self._rate_set_idx_per_bw_band[is_ext][bw][band] = {}
        # Maps is_ext to function that returns rate set bin for given rate
        bin_selector = {}
        for is_ext, split in [(True, split_ext_rates), (False, split_main_rates)]:
            bin_selector[is_ext] = (lambda r: r.rate_split_bin) if split else (lambda r: 0)
        for locale in self._locales_dict.values():
            band = locale.locale_type.band
            for power in locale.chan_power:
                bw = ClmData.rate_set_bw_mapper[power.bandwidth]
                frs = frozenset(locale.chan_power[power])
                if frs not in self._rate_sets_splitter:
                    # Maps is_ext to bins to rate sets in bin
                    rate_set_bins = {}
                    for rate in frs:
                        is_ext = use_ext_rate_sets and self.is_ext_rate(rate)
                        rate_set_bins.setdefault(is_ext, {}).setdefault(bin_selector[is_ext](rate), set()).add(rate)
                    # Maps is_ext to sets of rate frozensets
                    splitter = {}
                    for is_ext, bins in rate_set_bins.iteritems():
                        for rs in bins.itervalues():
                            splitter.setdefault(is_ext, set()).add(frozenset(rs))
                    self._rate_sets_splitter[frs] = (splitter.get(False, set()), splitter.get(True, set()))
                splitter = dict(zip((False, True), self._rate_sets_splitter[frs]))
                for is_ext, rate_subsets in splitter.iteritems():
                    for rate_subset in rate_subsets:
                        self._rate_sets_per_bw_band[is_ext][None][None].add(rate_subset)
                        self._rate_sets_per_bw_band[is_ext][bw][None].add(rate_subset)
                        self._rate_sets_per_bw_band[is_ext][bw][band].add(rate_subset)
        ret = {}
        for is_ext in self._rate_sets_per_bw_band:
            for bw in self._rate_sets_per_bw_band[is_ext]:
                for band in self._rate_sets_per_bw_band[is_ext][bw]:
                    self._rate_sets_per_bw_band[is_ext][bw][band] = \
                        sorted(self._rate_sets_per_bw_band[is_ext][bw][band],
                               key=lambda frs: RatesInfo.rate_set_sort_key(frs))
                    for i in range(len(self._rate_sets_per_bw_band[is_ext][bw][band])):
                        self._rate_set_idx_per_bw_band[is_ext][bw][band][self._rate_sets_per_bw_band[is_ext][bw][band][i]] = i
                    if (band is not None) and (bw is not None) and \
                       (len(self._rate_sets_per_bw_band[is_ext][bw][band]) > ClmData.EXT_RATES_COUNT_THRESHOLD):
                        ret[is_ext] = False
        return (ret.get(False, True), ret.get(True, True))

    def is_ext_rate(self, rate):
        """ True if rate shall belong to extended rate set (if it is used) """
        return rate.chains >= ClmData.EXT_RATE_MIN_CHAINS

    def optimize_aggregations(self):
        """ Optimizes aggregations: removes (mark as deleted if data is final)
        CC/0 entries and entries equal to aggregation default region
        """
        for agg_ccrev in self._aggregations_dict:
            reg_ccrevs_to_remove = []
            for reg_ccrev in self._aggregations_dict[agg_ccrev].mappings:
                if (reg_ccrev == agg_ccrev) or (reg_ccrev.rev == 0):
                    reg_ccrevs_to_remove.append(reg_ccrev)
            for reg_ccrev in reg_ccrevs_to_remove:
                self._aggregations_dict[agg_ccrev].mappings.remove(reg_ccrev)
                if self._is_incremental:
                    self._aggregations_dict[agg_ccrev].mappings.append(CcRev(reg_ccrev.cc, -1))
        self._update_ccrev_aliases()

    def get_rate_power_dict(self, region, channel, channel_type, extended,
                            force_ht20_siso_derive=False, sub_chan_id=None, trim_ht3_to_ht=True):
        """ Returns dictionary of (locale,power) tuples, indexed by rate

        Arguments:
        region                 -- Region object
        channel                -- Channel number
        channel_type           -- ChannelType object for a channel (related to
                                  main channel if subchannel data is requested)
        extended               -- True means that both explicitly specified (in
                                  CLM data) and derived rates shall be
                                  returned, false means that only explicitly
                                  specified rates shall be returned
        force_ht20_siso_derive -- True means that power for HT20 SISO rates
                                  will be derived from power for OFDM rates
                                  even though it shouldn't
        sub_chan_id            -- Subchannel ID. None if main channel power is
                                  requested
        trim_ht3_to_ht         -- True means that 3-chains locales in HT-only
                                  region shall be trimmed to 2-chains
        Returns                -- Dictionary. Keys are rate objects, values are
                                  tuples (locale_object,
                                  measurement_to_power_object_dictionary)
        """
        bw_chan_map = {}     # Maps bandwidths to channels where from minimum power shall be selected
        dsss_channel = None  # Channel to look DSSS power for (None if separate channel not needed)
        power_inc = 0        # Power taken from subchannel rule or 0
        if sub_chan_id is None:
            bw_chan_map[channel_type.bandwidth] = channel
        else:
            rules = self.get_sub_chan_rules(region.sub_chan_rules_name)
            active_channel = \
                ChannelType.decompose_80_80(channel, channel_type)[0] \
                if ChannelType.is_80_80(channel, channel_type) else channel
            rule = None
            if rules is not None:
                for chan_range in rules.chan_rules_dict:
                    if (chan_range.channel_type != channel_type) or \
                       not self._valid_channels.is_channel_in_range(active_channel, chan_range):
                        continue
                    rule = rules.chan_rules_dict[chan_range]
                    break
            if (rule is None) and (channel_type.bandwidth == Bandwidth._40):
                rule = {SubChanId.L: (set([Bandwidth._40]), 0), SubChanId.U: (set([Bandwidth._40]), 0)}
            if rule is None:
                return {}
            bandwidths, power_inc = rule.get(sub_chan_id, (None, 0))
            if bandwidths is None:
                return {}
            for bandwidth in bandwidths:
                chan = SubChanId.sub_chan(channel, channel_type.bandwidth, bandwidth, sub_chan_id)
                if chan is not None:
                    bw_chan_map[bandwidth] = chan
            if (SubChanId.sub_chan_bandwidth(channel_type.bandwidth, sub_chan_id) == Bandwidth._20) and (channel_type.band == Band._2):
                dsss_channel = SubChanId.sub_chan(channel, channel_type.bandwidth, Bandwidth._20, sub_chan_id)
        ret = {}
        has_ht_rates = False
        has_main_channel = False  # Power data for main channel exists (prerequisite for subchannel power)
        for loc_type in LocaleType.all():
            if loc_type.band != channel_type.band:
                continue
            locale = region.get_locale(loc_type)
            if locale is None:
                if trim_ht3_to_ht and (loc_type.flavor == LocaleType.HT3):
                    for rate in RatesInfo.get_rates_by_chains(3):
                        if rate in ret:
                            del ret[rate]
                force_coverage = "ZZZ"
                continue
            for power in locale.chan_power:
                has_main_channel |= (power.bandwidth == channel_type.bandwidth) and (not power.is_disabled) and \
                    self._valid_channels.is_channel_in_range(channel, power.channel_range)
                mapped_channel = bw_chan_map.get(power.bandwidth)
                if (mapped_channel is not None) and \
                   self._valid_channels.is_channel_in_range(mapped_channel, power.channel_range):
                    for rate in locale.chan_power[power]:
                        if not locale.locale_type.is_base:
                            has_ht_rates = True
                        if (dsss_channel is not None) and (rate.rate_type == RateInfo.RateType.DSSS):
                            continue
                        if rate not in ret:
                            ret[rate] = (locale, {})
                        if power.measure in ret[rate][1]:
                            ret[rate][1][power.measure] = \
                                Locale.Power.get_extreme_power(ret[rate][1][power.measure],
                                                               power.increment(power_inc),
                                                               get_min=True)
                        else:
                            ret[rate][1][power.measure] = power.increment(power_inc)
                if (dsss_channel is not None) and (power.bandwidth == Bandwidth._20) and \
                   self._valid_channels.is_channel_in_range(dsss_channel, power.channel_range):
                    for rate in locale.chan_power[power]:
                        if rate.rate_type != RateInfo.RateType.DSSS:
                            continue
                        if not locale.locale_type.is_base:
                            has_ht_rates = True
                        if rate not in ret:
                            ret[rate] = (locale, {})
                        ret[rate][1][power.measure] = power
        if (sub_chan_id is not None) and (not has_main_channel):
            return {}
        if extended and (not self._contains_ht20_siso or force_ht20_siso_derive):
            powers_to_add = {}
            for rate in ret:
                for derived_rate in RatesInfo.get_derived_rates(rate, channel_type.bandwidth, has_ht_rates, True):
                    if derived_rate not in ret:
                        powers_to_add[derived_rate] = ret[rate]
            for rate in powers_to_add:
                ret[rate] = powers_to_add[rate]
        return ret

    def get_channel_list(self, ranges):
        """ Returns sorted list of channel numbers corresponded to given set
        of ranges """
        if not ranges:
            return []
        ret = []
        for cr in ranges:
            ret.extend(self._valid_channels.get_channels_in_range(cr))
        ret.sort()
        return ret

    def get_adc_dict(self):
        """ Returns dictionary that maps regions' ccrev values to their
        advertised CCs
        """
        ret = {}
        for cc, adc in self._adcs_dict.items():
            for ccrev in adc.regions:
                ret[ccrev] = cc
        return ret

    def get_region_channels_dict(self, region):
        """ For given region returns dictionary of its channel lists, indexed
        by channel type (channels in each dictionary are sorted)
        """
        channel_sets = {}
        for channel_type in ChannelType.all():
            channel_sets[channel_type] = set()
        for loc_type in region.locale_ids:
            loc = region.get_locale(loc_type)
            for bandwidth in loc.channel_sets:
                channel_sets[ChannelType.create(loc.locale_type.band, bandwidth)].update(self.get_channel_list(loc.channel_sets[bandwidth]))
        ret = {}
        for channel_type in ChannelType.all():
            if channel_sets[channel_type]:
                ret[channel_type] = list(channel_sets[channel_type])
                ret[channel_type].sort()
        return ret

    def _update_ccrev_aliases(self):
        """ Updates CC/rev aliases and extra_ccrevs """
        self._ccrev_aliases = {}
        for ccrev in self._get_ccrevs_set(regions=True, deleted_regions=True, mappings=True,
                                          deleted_mappings=True, deleted_aggregations=True, adcs=True):
            self._ccrev_aliases[ccrev] = ""
        ccrev_aliases = set()
        for ccrev in self._ccrev_aliases:
            cc = re.sub(r"[^_a-zA-Z0-9]", "_", ccrev.cc)
            revstr = "DELETED" if ccrev.deleted else str(ccrev.rev)
            alias = "%s_%s" % (cc, revstr)
            if alias in ccrev_aliases:
                i = 1
                while True:
                    alias = "%s%d_%s" % (cc, i, revstr)
                    if alias not in ccrev_aliases:
                        break
                    i += 1
            ccrev_aliases.add(alias)
            self._ccrev_aliases[ccrev] = alias

        self._extra_ccrevs_list = \
            list(sorted(set(self._ccrev_aliases.keys()) -
                        self._get_ccrevs_set(regions=True, deleted_regions=True),
                        key=methodcaller("sort_key")))

    def _get_ccrevs_set(self, regions=False, deleted_regions=False,
                        aggregations=False, deleted_aggregations=False,
                        mappings=False, deleted_mappings=False,
                        adcs=False):
        """ Returns set of some CC/revs used in BLOB data

        Arguments:
        regions              -- Include CC/revs of present regions
        deleted_regions      -- Include CC/revs deleted regions
        aggregations         -- Include CC/revs of present aggregations
        deleted_aggregations -- Include CC/revs of deleted aggregations
        mappings             -- Include CC/revs of present mappings
        deleted_mappings     -- Include CC/<deleted> of deleted mappings
        adcs                 -- Include CC/revs of regions with advertisements
        Returns set of selected CC/revs
        """
        ret = set()
        if regions:
            ret.update(self._regions_dict.iterkeys())
        if deleted_regions:
            ret.update(map(lambda t: t[0], self._deleted_regions_list))
        if adcs:
            for adc in self._adcs_dict.values():
                ret.update(adc.regions)
        if aggregations or mappings or deleted_mappings:
            for agg_ccrev, agg in self._aggregations_dict.iteritems():
                if aggregations:
                    ret.add(agg_ccrev)
                for mapping in agg.mappings:
                    if deleted_mappings if mapping.deleted else mappings:
                        ret.add(mapping)
        if deleted_aggregations:
            ret.update(map(lambda da: da[0], self._deleted_aggregations_list))
        return ret

    def remap_rev16_to_rev8(self, ccrev):
        """ Performs rev16->rev8 remap

        Arguments:
        ccrev -- CC/rev to remap
        Returns remapped rev
        """
        return self._regrev_remap_table[ccrev.cc][ccrev.rev] if ccrev.cc in self._regrev_remap_table else ccrev.rev

    def move_ht20_to_base(self):    # pragma: no cover
        """ Moves HT20 powers identical to corresponded OFDM powers to base
        locales. It allows to decrease BLOB size.
        This option will be included in future BLOB format """

        # This function operates with clusters. Cluster is a list of base
        # locales and list of HT locales that shall be updated simultaneously
        # (because every HT locale is paired with some base locales from cluster
        # and every base locale paired with some HT locales from cluster).
        # Cluster organized as dictionary of lists. Indices are LocaleType.BASE
        # and LocaleType.HT, values are lists of Locale objects

        self.pairs = {}

        # Base locale index. Maps base locale IDs to clusters they belong to
        base_idx = {}
        # HT locale index. Maps HT locale IDs to clusters they belong to
        ht_idx = {}
        # Locales that in some regions do not have pair. Presence of such
        # locale in cluster invalidates cluster
        unpaired_locales = set()
        # Loop over all regions to make clusters
        for reg in self._regions_dict.itervalues():
            # Inside region - loop over bands
            for band in Band.all():
                # Base locale of current region on current band
                base_loc = self._locales_dict.get(reg.locale_ids.get(LocaleType(LocaleType.BASE, band)))
                # HT locale of current region on current band
                ht_loc = self._locales_dict.get(reg.locale_ids.get(LocaleType(LocaleType.HT, band)))
                if not (base_loc and ht_loc):
                    if base_loc:
                        unpaired_locales.add(base_loc.loc_id)
                    if ht_loc:
                        unpaired_locales.add(ht_loc.loc_id)
                    continue
                self.pairs.setdefault(ht_loc.loc_id, set()).add(base_loc.loc_id)
                self.pairs.setdefault(base_loc.loc_id, set()).add(ht_loc.loc_id)
                # Their clusters
                base_cluster = base_idx.get(base_loc.loc_id)
                ht_cluster = ht_idx.get(ht_loc.loc_id)
                if base_cluster and ht_cluster:
                    # Both locales already belong to some clusters
                    if base_cluster is ht_cluster:
                        # If they belong to same cluster - good
                        continue
                    # Base and HT locales belong to different clusters.
                    # Merging HT cluster to Base cluster...
                    base_cluster[LocaleType.BASE] += ht_cluster[LocaleType.BASE]
                    base_cluster[LocaleType.HT] += ht_cluster[LocaleType.HT]
                    # ... updating HT cluster indices to point to base locale
                    for ht in ht_cluster[LocaleType.HT]:
                        ht_idx[ht.loc_id] = base_cluster
                    for base in ht_cluster[LocaleType.BASE]:
                        base_idx[base.loc_id] = base_cluster
                elif base_cluster and not ht_cluster:
                    # Base locale belongs to some cluster, HT locale is new -
                    # adding HT locale to same cluster
                    base_cluster[LocaleType.HT].append(ht_loc)
                    ht_idx[ht_loc.loc_id] = base_cluster
                elif ht_cluster and not base_cluster:
                    # HT locale belong to some cluster, Base locale is new -
                    # adding base locale to same cluster
                    ht_cluster[LocaleType.BASE].append(base_loc)
                    base_idx[base_loc.loc_id] = ht_cluster
                else:
                    # Both locales are new - creating new cluster
                    cluster = {}
                    cluster[LocaleType.BASE] = [base_loc]
                    cluster[LocaleType.HT] = [ht_loc]
                    base_idx[base_loc.loc_id] = cluster
                    ht_idx[ht_loc.loc_id] = cluster
        ofdm_rates = set(RatesInfo.get_expanded_rate("OFDM"))
        # Trying to do move in each cluster
        for cluster in base_idx.itervalues():
            # Initialize to common OFDM power: set of Locale.Power objects
            # common to all base locales
            common_ofdm_power_set = None
            # Looking up common OFDM power in all base locales of cluster
            for base_loc in cluster[LocaleType.BASE]:
                # Skip cluster if base locale is in set of unpaired
                if base_loc.loc_id in unpaired_locales:
                    common_ofdm_power_set = None
                    break
                # OFDM power of current locale
                ofdm_power_set = set()
                for power, rates in base_loc.chan_power.iteritems():
                    if ofdm_rates <= rates:
                        ofdm_power_set.add(power)
                if common_ofdm_power_set is None:
                    common_ofdm_power_set = ofdm_power_set
                else:
                    common_ofdm_power_set &= ofdm_power_set
            if not common_ofdm_power_set:
                # Common OFDM power not found - continue to next cluster
                continue
            # Common HT power. Dictionary, keys are power objects (same as in
            # common_ofdm_power_set), values are rates
            common_ht_power_dict = None
            for ht_loc in cluster[LocaleType.HT]:
                # Skip cluster if HT locale is in set of unpaired
                if ht_loc.loc_id in unpaired_locales:
                    common_ht_power_dict = None
                    break
                # HT powers for current locale
                ht_power_dict = {}
                for power, rates in ht_loc.chan_power.iteritems():
                    if power in common_ofdm_power_set:
                        ht_power_dict[power] = rates
                if common_ht_power_dict is None:
                    common_ht_power_dict = ht_power_dict
                else:
                    for power, rates in common_ht_power_dict.items():
                        if rates != ht_power_dict.get(power):
                            del common_ht_power_dict[power]
            if not common_ht_power_dict:
                # Common HT power not found - continue to next cluster
                continue
            # Adding HT20 rates to base locales
            base_loc.unfreeze_rate_sets()
            for base_loc in cluster[LocaleType.BASE]:
                for power, rates in common_ht_power_dict.iteritems():
                    base_loc.chan_power[power] |= rates
            base_loc.freeze_rate_sets()
            # Removing HT20 entries (added to OFDM locale) from HT locales
            for ht_loc in cluster[LocaleType.HT]:
                for power in common_ht_power_dict:
                    del ht_loc.chan_power[power]

# =============================================================================


class ClmContainer:
    """ Holds and operates upon one CLM XML file

    Attributes:
    file_name -- CLM XML file name as passed to constructor
    """

    def __init__(self, clm_file_name):
        """ Constructor. Reads-in given CLM XML file """
        self.file_name = clm_file_name
        self._et = Et.ElementTree(file=clm_file_name)

        m = re.search(r"v(\d+)_(\d+)[^\\/\:]*$", EtreeElementWrapper.get_root_namespace(self._et))
        self._clm_format_version = (m.group(1) + "." + m.group(2)) if m else ""
        if not self._clm_format_version:
            ClmUtils.warning("Can't deduce \"%s\" format version from schema name. Version name check skipped" % self.file_name)
        elif (ClmUtils.compare_versions(self._clm_format_version, _XML_MIN_VERSION) < 0) or (ClmUtils.major(self._clm_format_version) > ClmUtils.major(_XML_VERSION)):
            ClmUtils.error("Format of \"%s\" contents is %s. This script only works with %s - %s.* format files" %
                           (self.file_name, self._clm_format_version, _XML_MIN_VERSION, ClmUtils.major(_XML_VERSION)))

        self._root_elem = EtreeElementWrapper(self._et)

    def fetch(self, filter_params, integrity_required=True, available_filters=None, merge_ht_ht3=False):
        """ Generates ClmData object from CLM XML file contents using given
        filters

        Arguments:
        filter_params          -- Filtering parameters that determine what
                                  data will go to resulting ClmData object
        integrity_required     -- True if referential integrity is required in
                                  output data
        available_filters      -- Optional output parameter, contains
                                  available filtering parameters
        merge_ht_ht3           -- Merge HT3 locales into HT locales
        Returns                -- ClmData object
        """
        try:
            check_failed = False
            ret = ClmData()
            if available_filters is None:
                available_filters = AvailableFilters()

            ret.set_clm_format_version(self._clm_format_version)
            ret.set_clm_version(self._root_elem.find("table_revision").text)
            if self._root_elem.find("apps_version"):
                ret.set_apps_version(self._root_elem.find("apps_version").text)
            generator_elem = self._root_elem.find("generated_by")
            if generator_elem:
                tool_name_elem = generator_elem.find("tool_name")
                tool_version_elem = generator_elem.find("tool_version")
                if tool_name_elem:
                    ret.set_generator_info(tool_name_elem.text,
                                           tool_version_elem.text if tool_version_elem else None)
            derive_rates = ClmUtils.compare_versions(self._clm_format_version, "2.2") < 0
            m = re.match(r"\d+\.\d+", ret.clm_version)
            ret.set_contains_ht20_siso((ClmUtils.compare_versions(self._clm_format_version, "2.3") >= 0) and
                                       (ClmUtils.compare_versions(m.group(0) if m is not None else "0.0", "5.2") >= 0))
            ret.set_regrev_remap_allowed(filter_params.regrev_remap)

            rev16_allowed = filter_params.blob_format.rev16 or filter_params.regrev_remap

            # Get country list
            country_dict = {}
            countries_elem = self._root_elem.find("country_list")
            if countries_elem:
                for country_elem in countries_elem.findall("country"):
                    country_dict[country_elem.find("ccode").text] = country_elem.find("name").text

            # Read all regions and put them to reg_dict
            reg_dict = {}  # Regions indexed by ccrev
            regs_elem = self._root_elem.find("region_list")
            if regs_elem:
                for reg_elem in regs_elem.findall("region", 1):
                    reg = Region(reg_elem, country_dict=country_dict, abg=filter_params.abg, band=filter_params.band,
                                 allowed_loc_reg_caps=filter_params.blob_format.loc_reg_caps & filter_params.loc_reg_caps,
                                 allow_default_for_cc=filter_params.blob_format.has_default_for_cc,
                                 allowed_edcrs=filter_params.blob_format.supported_edcrs,
                                 allow_lo_gain_nbcal=filter_params.blob_format.has_lo_gain_nbcal,
                                 allow_chsprwar2=filter_params.blob_format.has_chsprwar2)
                    reg_dict[reg.ccrev] = reg
                    available_filters.ccrevs.add(reg.ccrev)
                    available_filters.tags |= reg.tags

            # Add to output data all regions that match filtering criteria
            selected_region_ccrevs_set = set()  # Region CC/revs selected due to filtering parameters
            selected_agg_ccrevs_set = set()     # Aggregation CC/revs selected due to filtering parameters
            for reg in reg_dict.values():
                if not self._check_reg(reg, filter_params, rev16_allowed=rev16_allowed):
                    continue
                ret.add_region(reg)
                selected_region_ccrevs_set.add(reg.ccrev)

            # Add to output data all aggregations that match filtering criteria or selected region
            aggs_elem = self._root_elem.find("aggregate_country_list")
            mapped_reg_ccrev_set = set()   # CC/revs of mappings
            country_usage_counts = {}      # How many aggregations us country in mapping
            if aggs_elem:
                for agg_elem in aggs_elem.findall("aggregate_country", 1):
                    agg = Aggregation(agg_elem)
                    available_filters.ccrevs.add(agg.ccrev)
                    if not self._check_agg(agg, filter_params, selected_region_ccrevs_set, rev16_allowed=rev16_allowed):
                        continue
                    ret.add_aggregation(agg)
                    selected_agg_ccrevs_set.add(agg.ccrev)
                    for ccrev in agg.mappings:
                        mapped_reg_ccrev_set.add(ccrev)
                        if ccrev.cc not in country_usage_counts:
                            country_usage_counts[ccrev.cc] = 0
                        country_usage_counts[ccrev.cc] += 1
            full_countries_set = set()  # Country codes used in mapping of every aggregation
            num_agg = len(ret.aggregations)
            for cc, count in country_usage_counts.items():
                if count == num_agg:
                    full_countries_set.add(cc)

            # Add to output data regions mentioned in mappings
            for ccrev in mapped_reg_ccrev_set:
                if ret.get_region(ccrev):
                    continue
                if ccrev in reg_dict:
                    ret.add_region(reg_dict[ccrev])
                    continue
                if integrity_required:
                    ClmUtils.warning("Region \"%s\" (used by some aggregation) not found in \"%s\" (CLM v%s)" %
                                     (ccrev, self.file_name, ret.clm_version))
                    if filter_params.ignore_missed:
                        for agg in ret.aggregations:
                            if agg.remove_region(ccrev):
                                ClmUtils.warning("Reference to region \"%s\" removed from aggregation \"%s\" (CLM v%s)" %
                                                 (ccrev, agg.ccrev, ret.clm_version))
                    else:
                        check_failed = True

            # Complement regions to full set
            if filter_params.full_set:
                for ccrev in reg_dict:
                    if (ccrev.rev == 0) and ccrev.cc_externally_valid and (ccrev.cc not in full_countries_set) and (ret.get_region(ccrev) is None):
                        ret.add_region(reg_dict[ccrev])

            # Checks if all singular region filters were used
            if integrity_required:
                for filter_ccrevs, selected_ccrevs, kind in [(filter_params.ccrev_set, selected_region_ccrevs_set | selected_agg_ccrevs_set, "Region or aggregation"),
                                                             (filter_params.region_set, selected_region_ccrevs_set, "Region"),
                                                             (filter_params.agg_set, selected_agg_ccrevs_set, "Aggregation")]:
                    for ccrev in filter_ccrevs:
                        if ccrev.is_singular() and (ccrev not in selected_ccrevs):
                            ClmUtils.warning("%s \"%s\" was not found in \"%s\" (CLM v%s)" %
                                             (kind, ccrev, self.file_name, ret.clm_version))
                            check_failed = True

            # Copy regions' CC/revs to available_filters.regions
            available_filters.regions = set([region.ccrev for region in ret.regions])

            # Get channels
            channels_elem = self._root_elem.find("channel_list")
            if channels_elem:
                for channel_elem in channels_elem.findall("channel", max_version=4):
                    valid_channel = ValidChannel(channel_elem)
                    if (valid_channel.channel_type.bandwidth <= filter_params.max_bandwidth) and \
                       (valid_channel.channel_flavor in filter_params.blob_format.channel_flavors) and \
                       (not valid_channel.channel_type.ulb or filter_params.ulb):  # filter_params.blob_format.has_ulb):
                        ret.valid_channels.add_channel(valid_channel)

            # Now find names/capabilities of required locales and subchannel rules
            loc_name_dict = {}
            sub_chan_rules_name_set = set()
            for reg in ret.regions:
                for loc_id in reg.locale_ids.values():
                    loc_name_dict.setdefault(loc_id.name, set()).add(
                        loc_id.reg_caps & filter_params.blob_format.loc_reg_caps & filter_params.loc_reg_caps)
                if reg.sub_chan_rules_name is not None:
                    sub_chan_rules_name_set.add(reg.sub_chan_rules_name)

            # Get locales
            restricted_id_set = set()
            locs_elem = self._root_elem.find("locale_list")
            if locs_elem:
                for loc_elem in locs_elem.getchildren():
                    if LocaleType.get_elem_type(loc_elem) is None:
                        continue
                    loc_name = Locale.get_elem_name(loc_elem)
                    if loc_name not in loc_name_dict:
                        continue
                    for loc_reg_caps in loc_name_dict[loc_name]:
                        loc = Locale(loc_elem, ret.valid_channels,
                                     max_chains=min(filter_params.max_chains, filter_params.blob_format.max_chains),
                                     derive_rates=derive_rates,
                                     max_bandwidth=min(filter_params.max_bandwidth, filter_params.blob_format.max_bandwidth),
                                     ac=filter_params.ac,
                                     allow_per_antenna=filter_params.blob_format.per_antenna and filter_params.per_antenna,
                                     requested_reg_caps=loc_reg_caps,
                                     has_disabled_powers=filter_params.blob_format.has_disabled_powers,
                                     allow_80_80=filter_params.include_80_80 and filter_params.blob_format.has_80_80,
                                     allow_dfs_tw=filter_params.dfs_tw and filter_params.blob_format.has_dfs_tw,
                                     allow_ulb=filter_params.ulb)  # and filter_params.blob_format.has_ulb)
                        if loc.restricted_set_name is not None:
                            restricted_id_set.add(str(loc.restricted_set_name))
                        ret.add_locale(loc)
                    del loc_name_dict[loc_name]
            if integrity_required:
                for loc_name in loc_name_dict:
                    ClmUtils.warning("Locale \"%s\" not found in \"%s\" (CLM v%s)" % (loc_name, self.file_name, ret.clm_version))
                    if filter_params.ignore_missed:
                        for region in ret.regions:
                            if region.remove_locale(loc_name):
                                ClmUtils.warning("Reference to locale \"%s\" removed from region \"%s\" (CLM v%s)" %
                                                 (loc_name, region.ccrev, ret.clm_version))
                    else:
                        check_failed = True

            # Get restricted sets
            restricts_elem = self._root_elem.find("restrict_list")
            if restricts_elem:
                for restrict_elem in restricts_elem.findall("restrict"):
                    restrict = RestrictedSet(restrict_elem, ret.valid_channels)
                    if not self._check_restrict(restrict, restricted_id_set):
                        continue
                    restricted_id_set.discard(str(restrict.name))
                    ret.add_restricted_set(restrict)
            if integrity_required:
                for restrict_id in restricted_id_set:
                    ClmUtils.warning("Restricted set \"%s\" not found in \"%s\" (CLM v%s)" %
                                     (str(restrict_id), self.file_name, ret.clm_version))
                    if filter_params.ignore_missed:
                        for locale in ret.locales:
                            if locale.remove_restricted_set(restrict_id):
                                ClmUtils.warning("Reference to restricted set \"%s\" removed from locale \"%s\" (CLM v%s)" %
                                                 (restrict_id, str(locale.loc_id), ret.clm_version))
                    else:
                        check_failed = True

            # Set correct locale references (downgrade requested locale capabilities to actual)
            actual_ids = {}
            for locale in ret.locales:
                actual_ids.setdefault(locale.loc_id.name, set()).add(locale.loc_id.reg_caps)
            for reg in ret.regions:
                for loc_type, loc_id in reg.locale_ids.items():
                    best_reg_caps = LocaleRegionCaps.minimal()
                    for actual_reg_caps in actual_ids.get(loc_id.name, []):
                        if (actual_reg_caps in loc_id.reg_caps) and (best_reg_caps in actual_reg_caps):
                            best_reg_caps = actual_reg_caps
                    reg.locale_ids[loc_type] = LocaleId(loc_id.name, best_reg_caps)

            # Get subchannel rules
            sub_chan_rules_list_elem = self._root_elem.find("subchan_rules_list")
            if sub_chan_rules_list_elem:
                for sub_chan_rules_elem in sub_chan_rules_list_elem.findall("subchan_rules", max_version=1):
                    sub_chan_rules = \
                        SubChanRules(
                            sub_chan_rules_elem, ret.valid_channels,
                            filter_params.blob_format.sub_chan_rules_40 or filter_params.sub_chan_rules_40,
                            filter_params.blob_format.sub_chan_rules_inc or filter_params.sub_chan_rules_inc or
                            filter_params.sub_chan_rules_inc_separate)
                    if not self._check_sub_chan_rules(sub_chan_rules, sub_chan_rules_name_set):
                        continue
                    sub_chan_rules_name_set.discard(sub_chan_rules.name)
                    ret.add_sub_chan_rules(sub_chan_rules)
            if integrity_required:
                for sub_chan_rules_name in sub_chan_rules_name_set:
                    ClmUtils.warning("Subchannel rules set \"%s\" not found in \"%s\" (CLM v%s)" %
                                     (sub_chan_rules_name, self.file_name, ret.clm_version))
                    if filter_params.ignore_missed:
                        for region in ret.regions:
                            if region.remove_sub_chan_rules(sub_chan_rules_name):
                                ClmUtils.warning("Reference to subchannel rules \"%s\" removed from region \"%s\" (CLM v%s)" %
                                                 (sub_chan_rules_name, region.ccrev, ret.clm_version))
                    else:
                        check_failed = True
            if filter_params.blob_format.patch_subchan_rules:
                for rule in ret.sub_chan_rules:
                    rule.patch()

            if check_failed:
                ClmUtils.error("Inconsistencies found in \"%s\" (CLM v%s) (see messages above)" %
                               (self.file_name, ret.clm_version))
            self._check_filters(filter_params, reg_dict, selected_region_ccrevs_set, selected_agg_ccrevs_set, ret.clm_version)
            if merge_ht_ht3:
                for region in ret.regions:
                    region.merge_ht_ht3(filter_params.blob_format.trim_ht3_to_ht)
                # ret.move_ht20_to_base()
            ret.update(filter_params.blob_format.ext_rate_sets)
            return ret
        except (SystemExit, KeyboardInterrupt, MemoryError, NameError):
            raise
        except:
            ClmUtils.error("Error processing contents of \"%s\": %s" % (self.file_name, ClmUtils.exception_msg()))

    def _check_reg(self, reg, filter_params, rev16_allowed):
        """ Checks if given region shall be included into ClmData

        Arguments:
        region        -- Aggregate object in question
        filter_params -- Filtering parameters
        rev16_allowed -- True if 16-bit regrevs allowed
        Returns       -- True if region shall be included to ClmData
        """
        if reg.rev16 and not rev16_allowed:
            return False
        for reg_ccrev in filter_params.ccrev_set:
            if reg_ccrev.match(reg.ccrev):
                return True
        for reg_ccrev in filter_params.region_set:
            if reg_ccrev.match(reg.ccrev):
                return True
        for tag_filter in filter_params.tag_filters:
            if tag_filter.match(reg.tags):
                return True
        return False

    def _check_agg(self, agg, filter_params, selected_region_ccrevs, rev16_allowed):
        """ Checks if given aggregate shall be included into ClmData

        Arguments:
        agg                    -- Aggregate object in question
        filter_params          -- Filtering parameters
        selected_region_ccrevs -- Set of CC/revs of already selected regions
        rev16_allowed          -- True if 16-bit regrevs allowed
        Returns                -- True if aggregate shall be included to ClmData
        """
        if agg.rev16 and not rev16_allowed:
            return False
        for ccrev in filter_params.ccrev_set:
            if ccrev.match(agg.ccrev):
                return True
        for ccrev in filter_params.agg_set:
            if ccrev.match(agg.ccrev):
                return True
        for ccrev in selected_region_ccrevs:
            if agg.ccrev == ccrev:
                # This is for compatibility with --region/--agg behavior: region selected with --region does not select its namesake aggregation
                for reg_ccrev in filter_params.region_set:
                    if reg_ccrev.match(agg.ccrev):
                        break
                else:
                    return True
        return False

    def _check_restrict(self, restrict, restricted_id_set):
        """ Checks if given restricted set shall be included into ClmData

        Arguments:
        restrict          -- Restricted set object in question
        restricted_id_set -- Set of names of restricted sets used by already
                             included locales
        Returns           -- True if restricted set shall be included to
                             ClmData
        """
        return str(restrict.name) in restricted_id_set

    def _check_sub_chan_rules(self, sub_chan_rules, sub_chan_rules_name_set):
        """ Checks if given subchannel rules shall be included into ClmData

        Arguments:
        restrict          -- Subchannel rules object in question
        restricted_id_set -- Set of names of subchannel rules used by already
                             included regions
        Returns           -- True if subchannel rules shall be included to
                             ClmData
        """
        return sub_chan_rules.name in sub_chan_rules_name_set

    def _check_filters(self, given_filters, reg_dict, selected_region_ccrevs_set,
                       selected_agg_ccrevs_set, clm_version):
        """ Generates warnings if some of given filters does not match any
        actual filters

        Arguments:
        given_filters              -- Given filters
        reg_dict                   -- Dictionary with all regions ordered by
                                      CC/revs
        selected_region_ccrevs_set -- Set of selected regions' CC/revs
        selected_agg_ccrevs_set    -- Set of selected aggregations' CC/revs
        clm_version                -- Version of CLM data
        """
        unmatched_tag_filters = []
        for tag_filter in given_filters.tag_filters:
            for region in reg_dict.values():
                if tag_filter.match(region.tags):
                    break
            else:
                unmatched_tag_filters.append(tag_filter)
        if unmatched_tag_filters:
            ClmUtils.warning("The following filters do not match any regions in the CLM data (CLM v%s): \"%s\"" %
                             (clm_version, "\", \"".join([tag_filter.source for tag_filter in unmatched_tag_filters])))
        for selected_ccrevs, filter_ccrevs, kinds in ((selected_region_ccrevs_set, given_filters.region_set, "regions"),
                                                      (selected_agg_ccrevs_set, given_filters.agg_set, "aggregations"),
                                                      (selected_region_ccrevs_set | selected_agg_ccrevs_set, given_filters.ccrev_set, "regions and aggregations")):
            unmatched_ccrev_filters = []
            for ccrev_filter in filter_ccrevs:
                if ccrev_filter == CcRev("all"):
                    continue
                for ccrev in selected_ccrevs:
                    if ccrev_filter.match(ccrev):
                        break
                else:
                    unmatched_ccrev_filters.append(ccrev_filter)
            if unmatched_ccrev_filters:
                unmatched_ccrev_filters.sort(key=methodcaller("sort_key"))
                ClmUtils.warning("The following %s specifications do not match any regions in CLM data (CLM v%s): %s" %
                                 (kinds, clm_version, ", ".join([str(ccrev) for ccrev in unmatched_ccrev_filters])))

    def get_available_filters(self, filter_params):
        """ Returns AvailableFilters object filled with filtering parameters
        available for CLM data

        Arguments:
        filter_params -- BLOB filtering parameters (affects regions, not tags)
        Returns AvailableFilters object that corresponds to given filtering
        parameters
        """
        ret = AvailableFilters()
        self.fetch(filter_params, False, available_filters=ret)
        return ret

# =============================================================================


class ClmDataContainer:
    """ Holds filtering parameters and CLM data (final and base). Responsible
    for fetching ClmData to be stored in BLOB according to filtering parameters

    Attributes:
    filter_params      -- Main filtering parameters
    base_filter_params -- Filtering parameters to use for base CLM data. If no
                          parameters specified then main filtering parameters
                          used
    """

    def __init__(self):
        """ Constructor. Initializes to no filtering parameters, no CLM data
        """
        self.filter_params = FilterParams()
        self.base_filter_params = FilterParams()
        self._clm = None
        self._base_clm = None
        self._blob_data = None
        self._data = None
        self._base_data = None

    def __getattr__(self, name):
        """ Calculates derived attributes

        Attributes calculated:
        has_data -- True if CLM data loaded to filter
        """
        if name == "has_data":
            return self._clm is not None
        raise AttributeError("Internal error: attribute \"%s\" not defined" % name)

    def set_clm(self, file_name, base=False):
        """ Reads CLM XML file

        Arguments:
        file_name -- Name of CLM XML file
        base      -- True for base (increment from) CLM file, False for final
                     CLM file
        """
        field = "_base_clm" if base else "_clm"
        if getattr(self, field) is not None:
            raise ValueError("File already specified")
        setattr(self, field, ClmContainer(file_name))

    def get_available_filters(self):
        """ Returns AvailableFilters object filled with filtering parameters
        available for main CLM data
        """
        return self._clm.get_available_filters(self.filter_params if self.filter_params.filtering_set else FilterParams.filter_all())

    def get_data(self):
        """ Returns data fetched from main (final) CLM file using given BLOB
        formatting
        """
        if self._data is None:
            self._data = self._clm.fetch(self.filter_params)
        return self._data

    def get_base_data(self):
        """ Returns data fetched from base (inc from) CLM file using given
        BLOB formatting
        """
        if (self._base_data is None) and (self._base_clm is not None):
            self._base_data = self._base_clm.fetch(self.base_filter_params if self.base_filter_params.filtering_set else self.filter_params,
                                                   integrity_required=False)
        return self._base_data

    def print_data_diff(self, log_file):
        """ Prints difference between base and main CLM data to given file """
        if (self._clm) is None or (self._base_clm is None):
            ClmUtils.error("CLM data comparison requires that main and base (--inc_from) CLM XML files to be specified")
        base_data = self._base_clm.fetch(self.base_filter_params if self.base_filter_params.filtering_set else self.filter_params, merge_ht_ht3=False, integrity_required=False)
        final_data = self._clm.fetch(self.filter_params, merge_ht_ht3=False, integrity_required=False)
        try:
            _f = ClmUtils.open_for_write(log_file)
            differences_found = False
            print >> _f, "Base  is \"%s\"" % self._base_clm.file_name
            print >> _f, "Final is \"%s\"" % self._clm.file_name
            for attr in ("clm_format_version", "generator_name", "generator_version", "clm_version", "apps_version"):
                base_value = getattr(base_data, attr)
                final_value = getattr(final_data, attr)
                if base_value == final_value:
                    continue
                differences_found = True
                print >> _f, "Difference in %s:" % attr
                for prefix, value in [("Base ", base_value), ("Final", final_value)]:
                    print >> _f, "    %s  value: %s" % (prefix, '"' + value + '"' if value is not None else "<None>")
            for attr in ("locales", "regions", "adcs", "aggregations", "restricted_sets", "sub_chan_rules"):
                dict_attr = attr + "_dict"
                heading = "Differences in %s:" % attr.replace("_", " ")
                keys = list(set(getattr(base_data, dict_attr).keys()) | set(getattr(final_data, dict_attr).keys()))
                keys.sort()
                for key in keys:
                    base_value = getattr(base_data, dict_attr).get(key)
                    final_value = getattr(final_data, dict_attr).get(key)
                    diffs = None
                    if base_value and final_value:
                        diffs = base_value.get_diffs(final_value)
                        if not (diffs[0] or diffs[1]):
                            continue
                    differences_found = True
                    if heading:
                        print >> _f, heading
                        heading = None
                    if diffs:
                        print >> _f, "    %s - differences:" % key
                        for name, content in [("Base", diffs[0]), ("Final", diffs[1])]:
                            if not content:
                                continue
                            print >> _f, "        %s data defines:" % name
                            for line in content:
                                print >> _f, "            %s" % line
                    elif base_value:
                        print >> _f, "    %s - defined only in base data" % key
                    else:
                        print >> _f, "    %s - defined only in final data" % key

            channel_diffs = base_data.valid_channels.get_diffs(final_data.valid_channels)
            if channel_diffs[0] or channel_diffs[1]:
                differences_found = True
                print >> _f, "Difference in valid_channels:"
                for where in range(len(channel_diffs)):
                    for s in channel_diffs[where]:
                        print >> _f, "    %s - defined only in %s data" % (s, "base" if where == 0 else "final")

            if not differences_found:
                print >> _f, "No differences found"
        except IOError:
            ClmUtils.error("Error writing comparison log file \"%s\": %s" % (log_file, ClmUtils.exception_msg()))

    def get_blob_data(self):
        """ Returns data to be written to BLOB """
        if self._clm is None:
            return None
        if self._blob_data is not None:
            return self._blob_data
        self._blob_data = self._clm.fetch(self.filter_params, merge_ht_ht3=True)
        if self._base_clm is not None:
            self._blob_data.set_is_incremental(True)
            base = self._base_clm.fetch(self.base_filter_params if self.base_filter_params.filtering_set else self.filter_params,
                                        merge_ht_ht3=True)
            if base.has_rev16 and not (self.filter_params.blob_format.rev16 or self._blob_data.regrev_remap_allowed):
                ClmUtils.error("Base BLOB uses 16-bit regrevs. Incremental BLOB must support them (via --regrev_remap or recent enough BLOB format)")
            self._blob_data.set_clm_base_version(base.clm_version)
            self._blob_data.set_apps_version(base.get_apps_version(), is_base=True)
            full_data_filter_params = copy.deepcopy(self.filter_params)
            full_data_filter_params.add_ccrev("ccrev", "all")
            full_blob_data = self._clm.fetch(full_data_filter_params, merge_ht_ht3=True, integrity_required=False)
            for data in ("region", "aggregation", "adc"):
                keys_of_equal_data = []
                for final_item in self._blob_data.__getattr__(data + "s"):
                    key = final_item.key()
                    base_item = getattr(ClmData, "get_" + data)(base, key)
                    if (base_item is not None) and final_item.data_equal(base_item, final_item.diff_report):
                        keys_of_equal_data.append(key)
                for base_item in base.__getattr__(data + "s"):
                    key = base_item.key()
                    if getattr(ClmData, "get_" + data)(self._blob_data, key) is None:
                        diff_report = DiffReport()
                        full_data_item = getattr(ClmData, "get_" + data)(full_blob_data, key)
                        if (data in ("region", "adc")) and (full_data_item is not None) and base_item.data_equal(full_data_item, diff_report):
                            getattr(ClmData, "add_preserved_" + data)(self._blob_data, key)
                        else:
                            getattr(ClmData, "add_deleted_" + data)(self._blob_data, key, diff_report)
                for key in keys_of_equal_data:
                    getattr(ClmData, "remove_" + data)(self._blob_data, key)
            for agg in self._blob_data.aggregations:
                base_agg = base.get_aggregation(agg.ccrev)
                if base_agg:
                    agg.remove_duplicated_mappings(base_agg)
            for region in self._blob_data.regions:
                region.find_same_locales(base.get_region(region.ccrev))
            self._blob_data.update(self.filter_params.blob_format.ext_rate_sets, base_clm_data=base)
            if self._blob_data.used_channel_combs == base.used_channel_combs:
                self._blob_data.remove_used_channel_combs()
        return self._blob_data

# =============================================================================


class ClmBlob:
    """ BLOB writer

    Private attributes:
    _options               -- Nonfiltering command line options
    _bf                    -- BLOB format information (BlobFormatVersionInfo
                              object)
    _data                  -- Data to write to BLOB
    _filter_params         -- BLOB filter parameters
    _base_filter_params    -- Base BLOB filter parameters
    _blob_file_name        -- BLOB file name ("" if BLOB writing is not
                              planned)
    _f                     -- BLOB file object (None if BLOB writing not
                              planned)
    _rev16                 -- True if revs are 16-bit
    _reg_idx_16            -- True if region indices are 16-bit
    _locales_of_loc_type   -- Per locale-type list of locales of this type
    _reg_flag2_byte        -- True if region record has second flag byte
    _reg_flag2             -- True if region has non-subchannel-index flags for
                              second flag byte
    _reg_rec_info          -- Region record format information
    _sub_chan_inc_fmt      -- Subchannel rules' increment format
                              (SUB_CHAN_INC_FMT_...)
    _region_sc_increments  -- Vectors of subchannel increments (qDbm values),
                              ordered first by channel type then by subchannel
                              rules' name. All-zero vectors not included
    _loc_idx_12_flag_swap  -- 12-bit locales indices with swapped flag byte
                              used
    _direct_rates_mapping  -- If direct rate mapping used, maps rates to pairs
                              (is_extended, ID). Otherwise None
    _locale_sizes          -- Dictionary, indexed by locale IDs. Elements are
                              sizes of locale bodies
    _locale_channel_ranges -- Dictionary, indexed by locale IDs then by
                              bandwidth. Ultimate elements are sets of channel
                              ranges, used by locales
    _locale_rate_sets      -- Dictionary, indexed by locale IDs, then by band,
                              then by bandwidth, then by normal/extended
                              boolean. Ultimate elements are sets of rate sets
                              used by locales
    """

    class RegRecInfo:
        """ Information about region record

        Attributes:
        struct_type      -- Type of structure that contains region record
        struct_size      -- Size of structure that contains region record
        macro            -- Macro that forms region record
        rev_extra_bytes  -- Number of extra rev bytes (0 or 1)
        idx_extra_bytes  -- Number of extra locale index bytes (0, 1 or 2)
        flag_extra_bytes -- Number of extra flag bytes (0 or 1)
        flags2           -- REGION() macro has flags2 parameter
        """
        def __init__(self, rev_bits, loc_idx_bits, flag_extra_bytes, flags2, loc_idx_12_flag_swap, blob_format):
            """ Constructor. Computes all attributes

            Arguments:
            rev_bits             -- Number of rev bits (8 or 16)
            loc_idx_bits         -- Number of locale index bits (8, 10 or 12)
            flag_extra_bytes     -- Number of extra flag bytes (0 or 1)
            flags2               -- True if there are non-subchannel flags for
                                    second flags byte, and hence REGION() macro
                                    shall have 'flags2' parameter
            loc_idx_12_flag_swap -- 12-bit locale indices with swapped flag
            blob_format          -- BlobFormatVersionInfo object (BLOB format
                                    properties)
            """
            self.rev_extra_bytes = (rev_bits - 8) / 8
            self.idx_extra_bytes = (loc_idx_bits - 8) / 2
            self.flag_extra_bytes = flag_extra_bytes
            self.flags2 = flags2
            content_dependent_reg = blob_format.content_dependent_reg or self.flag_extra_bytes or loc_idx_12_flag_swap
            if content_dependent_reg:
                self.struct_size = 8 + self.rev_extra_bytes + self.idx_extra_bytes + self.flag_extra_bytes
                self.struct_type = "clm_country_rev_definition_cd%d_t" % self.struct_size
            elif blob_format.reg_rec_10_fl:
                self.struct_size = 9
                self.struct_type = "clm_country_rev_definition10_fl_t"
            else:
                self.struct_size = 7
                self.struct_type = "clm_country_rev_definition_t"
            self.macro = "#define REGION(cc, rev, loc_2, loc_5, loc_HT_2, loc_HT_5, scr, flags%s)\\\n" % (", flags2" if flags2 else "")
            fields = \
                ["{cc, (unsigned char)(0xFF & rev)}",
                 [("(unsigned char)(0xFF & %s)" % loc_idx) for loc_idx in ["LOCALE_2G_IDX_##loc_2", "LOCALE_5G_IDX_##loc_5",
                                                                           "LOCALE_2G_HT_IDX_##loc_HT_2", "LOCALE_5G_HT_IDX_##loc_HT_5"]]]
            if content_dependent_reg or blob_format.reg_rec_10_fl:
                if content_dependent_reg:
                    container = []
                    fields.append(container)
                else:
                    container = fields
                if rev_bits > 8:
                    container.append("(unsigned char)(0xFF & ((rev) >> 8))")
                if (loc_idx_bits > 8) or (blob_format.reg_rec_10_fl and not content_dependent_reg):
                    container.append(
                        "(unsigned char)(((LOCALE_2G_IDX_##loc_2>>8)&0x03)+((LOCALE_5G_IDX_##loc_5>>6) & 0x0C) +\\\n" +
                        "  ((LOCALE_2G_HT_IDX_##loc_HT_2>>4)&0x30) + ((LOCALE_5G_HT_IDX_##loc_HT_5>>2)&0xC0))")
                flags_line = "(unsigned char)(0xFF & (flags | %s))" % \
                    ("(scr & CLM_DATA_FLAG_REG_SC_RULES_MASK)" if self.flag_extra_bytes else "scr")
                if loc_idx_12_flag_swap:
                    container.append(flags_line)
                if loc_idx_bits > 10:
                    container.append(
                        "(unsigned char)(((LOCALE_2G_IDX_##loc_2>>10)&0x03)+((LOCALE_5G_IDX_##loc_5>>8) & 0x0C) +\\\n" +
                        "  ((LOCALE_2G_HT_IDX_##loc_HT_2>>6)&0x30) + ((LOCALE_5G_HT_IDX_##loc_HT_5>>4)&0xC0))")
                if not loc_idx_12_flag_swap:
                    container.append(flags_line)
                if self.flag_extra_bytes:
                    container.append("(unsigned char)(0xFF & ((scr >> CLM_DATA_FLAG_REG_SC_RULES_MASK_WIDTH)))%s" %
                                     (" | flags2" if flags2 else ""))
            for field_idx in range(len(fields)):
                if isinstance(fields[field_idx], list):
                    fields[field_idx] = ("{" + (",\\\n".join(fields[field_idx])) + "}").replace("\n", "\n ")
            self.macro += ("    {" + (",\\\n".join(fields)) + "}").replace("\n", "\n     ") + "\n"

    class TxPowerTypeInfo(ClmUtils.EqualityMixin):
        """ Power type - collection of data used in TX rates' group header

        Attributes:
        bandwidth    -- Power targets' bandwidth
        is_80_80     -- Channels are 80+80
        measure      -- Power targets' measure
        extra_powers -- Number of extra powers in record
        ext_rates    -- Extended rate sets table shall be used
        """
        def __init__(self, tx_power, ext_rates):
            """ Constructor

            Arguments:
            tx_power  -- Representative Locale.Power object
            ext_rates -- True if this object is for extended rate set, False if
                         for main
            """
            self.bandwidth = tx_power.bandwidth
            self.is_80_80 = tx_power.channel_range.is_80_80
            self.measure = tx_power.measure
            self.extra_powers = 0 if tx_power.is_disabled else len(tx_power.powers_dbm) - 1
            self.ext_rates = ext_rates

        def use_flag2(self):
            """ True if second flag byte needed """
            return self.ext_rates or Bandwidth.is_ulb(self.bandwidth)

        def __hash__(self):
            """ Hash (for using as dictionary key) """
            ret = 0
            for f in self.__dict__:
                ret ^= getattr(self, f).__hash__()
            return ret

        def sort_key(self):
            """ Sort key """
            return (self.bandwidth, self.is_80_80, self.measure, self.extra_powers, self.ext_rates)

    class MulticolumnWriter:
        """ Simple multicolumn writer

        Private attributes:
        _indent    -- Initial indent
        _col_width -- Column width
        _col_num   -- Number of columns in a row
        _writer    -- Function that performs write
        _row_buf   -- Partial row buffer
        _row_len   -- Number of items in partial row buffer
        """
        def __init__(self, indent, col_width, col_num, writer):
            """ Constructor

            Arguments:
            indent    -- Initial indent
            col_width -- Column width
            col_num   -- Number of columns in a row
            writer    -- Function that performs write
            """
            self._indent = indent
            self._col_width = col_width
            self._col_num = col_num
            self._writer = writer

        def __enter__(self):
            """ With statement initialization """
            self._row_buf = ""
            self._row_len = 0
            return self

        def __exit__(self, exc_type, exc_value, traceback):
            """ With statement finalization. Exception informationis ignored """
            self._flush()

        def __call__(self, s):
            """ Write value to column

            Arguments:
            s -- Value to write
            """
            if self._row_len:
                if self._row_buf[-1] != " ":
                    self._row_buf += " "
            else:
                if self._indent:
                    self._row_buf += " " * self._indent
            self._row_buf += "%-*s" % (self._col_width, s)
            self._row_len += 1
            if self._row_len == self._col_num:
                self._flush()

        def _flush(self):
            """ Flushes partial column (e.g. at the end of print) """
            if self._row_len:
                self._writer(self._row_buf.rstrip() + "\n")
            self._row_buf = ""
            self._row_len = 0

    CHAN_COMB_SIZE = 3               # Valid channels comb record size
    CHAN_COMB_FOOTER_SIZE = 8        # Valid channels' footer record size
    CHAN_RANGE_SIZE = 2              # Channel range record size
    RESTR_SET_HDR_SIZE = 1           # Restricted set header size
    CHAN_SET_HDR_SIZE = 1            # Locale channel set record header size
    RATE_SET_HDR_SIZE = 1            # Rate set record header size
    BASE_LOC_HDR_SIZE = 3            # Base locale header size
    LOC_PUB_PWR_HDR_SIZE = 1         # Locale public power subheader size
    LOC_PUB_PWR_REC_SIZE = 2         # Locale public power subrecord size
    LOC_CHAN_PWR_HDR_SIZE = 2        # Locale channel power subheader size
    LOC_CHAN_PWR_HDR2_SIZE = 3       # Locale channel power subheader size when flags2 used
    LOC_CHAN_PWR_REC_SIZE = 3        # Locale channel power subrecord size
    REG_FOOTER_SIZE = 8              # Regions' footer record size
    ADC_REC_SIZE = 3                 # Advertisement region record size
    ADC_IDX_8_REC_SIZE = 1           # Advertisement region record size (8-bit ccrev index)
    ADC_IDX_16_REC_SIZE = 2          # Advertisement region record size (16-bit ccrev index)
    ADC_SIZE = 12                    # Advertisement record size
    ADC_FOOTER_SIZE = 8              # Advertisements' footer record size
    AGG_MAP_SIZE = 3                 # Aggregation mapping record size
    AGG_MAP_IDX_8_SIZE = 1           # Aggregation mapping record size (8-bit ccrev index)
    AGG_MAP_IDX_16_SIZE = 2          # Aggregation mapping record size (16-bit ccrev index)
    AGG_SIZE = 12                    # Aggregation record size
    AGG_FOOTER_SIZE = 8              # Aggregation footer record size
    SUB_CHAN_CHAN_SIZE_40 = 3        # 40MHz subchannel channel rule size
    SUB_CHAN_CHAN_SIZE_80 = 7        # 80MHz subchannel channel rule size
    SUB_CHAN_CHAN_SIZE_160 = 15      # 160MHz subchannel channel rule size
    SUB_CHAN_CHAN_SIZE_40_INC = 5    # 40MHz subchannel channel rule size with increment
    SUB_CHAN_CHAN_SIZE_80_INC = 13   # 80MHz subchannel channel rule size with increment
    SUB_CHAN_CHAN_SIZE_160_INC = 29  # 160MHz subchannel channel rule size with increment
    SUB_CHAN_REG_SIZE = 8            # Subchannel region rules size
    SUB_CHAN_REG_SIZE_INC = 12       # Subchannel region rules size with increment in separate format
    SUB_CHAN_FOOTER_SIZE = 8         # Subchannel footer size
    EXTRA_CCREV_8_SIZE = 3           # Extra ccrev with 8-bit rev record size
    EXTRA_CCREV_16_SIZE = 4          # Extra ccrev with 16-bit rev record size
    EXTRA_CCREV_FOOTER_SIZE = 8      # Extra ccrevs footer size
    REGREV_REMAP_REC_SIZE = 3        # Size of regrev remap record
    REGREV_CC_REMAP_REC_SIZE = 4     # Size of regrev CC remap record
    REGREV_REMAP_FOOTER_SIZE = 12    # Regrev CC remap footer record size

    IDX_NUM_SPECIAL_CHANNEL_RANGE = 1   # Number of special channel range indices
    IDX_NUM_SPECIAL_RESTRICTED_SET = 1  # Number of special restricted set indices
    IDX_NUM_SPECIAL_CHANNEL_SET = 0     # Number of special channel set indices
    IDX_NUM_SPECIAL_RATE_SET = 0        # Number of special rate set indices
    IDX_NUM_SPECIAL_LOCALE = 3          # Number of special locale indices
    IDX_NUM_SPECIAL_SUB_CHAN_RULE = 1   # Number of special subchannel rule indices
    IDX_NUM_SPECIAL_REV = 1             # Number of special rev (in CC/rev) values

    EXT_RATES_BASE = "WL_RATE_1X3_DSSS_1"  # Base rate for extended rate set

    # Formats for subchannel rule increments' representation in BLOB
    SUB_CHAN_INC_FMT_NONE, SUB_CHAN_INC_FMT_COUPLED, SUB_CHAN_INC_FMT_SEPARATE = range(3)

    # Special 10-bit locale indices that shall be avoided among 12-bit locale indices if loc_idx_12bit parameter is used
    SPECIAL_10_BIT_LOC_INDICES = [0x3FF, 0x3FE, 0x3FD]

    def __init__(self, clm_data_container, options):
        """ Constructor. Performs BLOB output

        Arguments:
        clm_data_container -- Container of CLM data for BLOB
        options            -- Nonfiltering command line options
        """
        assert ClmData.EXT_RATES_COUNT_THRESHOLD == (256 - ClmBlob.IDX_NUM_SPECIAL_RATE_SET)
        assert ClmBlob.IDX_NUM_SPECIAL_REV == (256 - CcRev.MIN_REV16)
        assert ClmData.CHANNEL_RANGES_SPLIT_THRESHOLD == (256 - ClmBlob.IDX_NUM_SPECIAL_CHANNEL_RANGE)
        try:
            self._options = options
            self._bf = clm_data_container.filter_params.blob_format
            self._data = clm_data_container.get_blob_data()
            self._data.optimize_aggregations()
            if (not self._data.is_incremental) and self._data.empty:
                ClmUtils.error("No data to put to BLOB")
            self._filter_params = clm_data_container.filter_params
            self._base_filter_params = clm_data_container.base_filter_params
            if options.blob_file_name is not None:
                self._blob_file_name = options.blob_file_name
                self._f = ClmUtils.open_for_write(options.blob_file_name)
            else:
                self._blob_file_name = ""
                self._f = None
            self._check_limit(max(map(attrgetter("rev"), self._data.ccrev_aliases.keys())) if self._data.ccrev_aliases else 0,
                              "Maximum regrev value",
                              16 if self._bf.rev16 or self._data.regrev_remap_allowed else 8,
                              ClmBlob.IDX_NUM_SPECIAL_REV, is_index=True)
            self._regrev_remap = self._data.regrev_remap_required
            self._rev16 = self._data.has_rev16 and not self._regrev_remap
            if self._regrev_remap:
                max_remaps_num = 0
                for remaps in self._data.regrev_remap_table.values():
                    max_remaps_num = max(max_remaps_num, len(remaps))
                self._check_limit(max_remaps_num, "Number of regrev remaps", 8, ClmBlob.IDX_NUM_SPECIAL_REV, is_index=True)

            self._locales_of_loc_type = {}
            for loc_type in LocaleType.all():
                if loc_type.flavor != LocaleType.HT3:
                    self._locales_of_loc_type[loc_type] = []
            for locale in self._data.locales:
                self._locales_of_loc_type[locale.locale_type].append(locale)

            max_loc_idx = max(map(lambda x: len(x),
                                  self._locales_of_loc_type.values()))
            self._check_limit(
                max_loc_idx, "Number of locales of one type",
                12 if (self._bf.content_dependent_reg or (self._bf.reg_rec_10_fl and self._filter_params.loc_idx_12bit))
                else (10 if self._bf.reg_rec_10_fl else 8),
                ClmBlob.IDX_NUM_SPECIAL_LOCALE +
                (len(ClmBlob.SPECIAL_10_BIT_LOC_INDICES) if self._filter_params.loc_idx_12bit and not self._bf.content_dependent_reg else 0),
                is_index=True)
            for loc_idx_bits in (8, 10, 12):
                if max_loc_idx < self._value_limit(loc_idx_bits, ClmBlob.IDX_NUM_SPECIAL_LOCALE):
                    break
            self._loc_idx_12_flag_swap = (loc_idx_bits == 12) and not self._bf.content_dependent_reg

            flags2 = any([region.lo_gain_nbcal or region.chsprwar2 for region in self._data.regions])

            self._reg_flag2_byte = flags2 or \
                (((self._bf.scr_idx_bits > 4) or (self._filter_params.scr_idx_8bit)) and
                 (len(self._data.sub_chan_rules) > self._value_limit(3, ClmBlob.IDX_NUM_SPECIAL_SUB_CHAN_RULE)))

            self._reg_rec_info = ClmBlob.RegRecInfo(
                rev_bits=16 if self._rev16 else 8, loc_idx_bits=loc_idx_bits, flag_extra_bytes=1 if self._reg_flag2_byte else 0,
                flags2=flags2, loc_idx_12_flag_swap=self._loc_idx_12_flag_swap, blob_format=self._bf)

            self._reg_idx_16 = len(self._data.ccrev_aliases) >= 256

            if self._bf.sub_chan_rules_inc_separate:
                self._sub_chan_inc_fmt = ClmBlob.SUB_CHAN_INC_FMT_SEPARATE
            elif self._bf.sub_chan_rules_inc:
                self._sub_chan_inc_fmt = ClmBlob.SUB_CHAN_INC_FMT_COUPLED
            elif self._filter_params.sub_chan_rules_inc_separate:
                self._sub_chan_inc_fmt = ClmBlob.SUB_CHAN_INC_FMT_SEPARATE
            elif self._filter_params.sub_chan_rules_inc:
                self._sub_chan_inc_fmt = ClmBlob.SUB_CHAN_INC_FMT_COUPLED
            else:
                self._sub_chan_inc_fmt = ClmBlob.SUB_CHAN_INC_FMT_NONE

            if self._sub_chan_inc_fmt != ClmBlob.SUB_CHAN_INC_FMT_NONE:
                has_sub_chan_power_increments = False
                for region_rules in self._data.sub_chan_rules:
                    for channel_rule in region_rules.chan_rules_dict.itervalues():
                        for _, increment in channel_rule.itervalues():
                            if increment:
                                has_sub_chan_power_increments = True
                                break
                        if has_sub_chan_power_increments:
                            break
                    if has_sub_chan_power_increments:
                        break
                if not has_sub_chan_power_increments:
                    self._sub_chan_inc_fmt = ClmBlob.SUB_CHAN_INC_FMT_NONE

            self._direct_rates_mapping = None
            self._locale_sizes = {}
            self._locale_channel_ranges = {}
            self._locale_rate_sets = {}

            self._write_header()
            self._write_used_channels()
            self._write_channel_ranges()
            self._write_restricted_sets()
            self._write_locales()
            self._write_sub_chan_rules()
            self._write_regions()
            self._write_aggregates()
            self._write_regrev_remaps()
            self._write_user_string()
            self._write_footer()
            if self._f is not None:
                self._f.close()
            self._verbose_statistics = options.verbose
            self._print_statistics()
        except IOError:
            ClmUtils.error("Error writing BLOB file \"%s\": %s"
                           % (options.blob_file_name, ClmUtils.exception_msg()))

    def _write_header(self):
        """ Writes BLOB header """
        command_lines = " * ClmCompiler flags: %s\n" % self._filter_params.command_lines[0][1]
        for filename, command_line in self._filter_params.command_lines[1:]:
            command_lines += " * %s: %s\n" % (filename, command_line)
        if self._data.clm_base_version is not None:
            if self._base_filter_params.command_lines:
                command_lines += " * Base flags:\n"
                for filename, command_line in self._base_filter_params._command_lines:
                    command_lines += " * %s: %s\n" % (filename, command_line)
            base_apps_version = self._data.get_apps_version(is_base=True)
            base_version_line = " * Base CLM v%s%s\n" % (self._data.clm_base_version,
                                                         ((" (%s)" % base_apps_version) if base_apps_version is not None else ""))
        else:
            base_version_line = ""
        generator = ClmBlob.get_full_generator_name(self._data)

        self._write_blob(("/** CLM data (BLOB)\n" +
                          " * This file was generated by ClmCompiler v%s\n" +
                          " * From CLM v%s%s data created by %s\n" +
                          "%s" +
                          "%s" +
                          "%s" +
                          "\n\n" +
                          " * <<Broadcom-WL-IPTag/%s:>>\n" +
                          " */\n\n" +
                          "#include \"wlc_clm_data.h\"\n\n" +
                          "extern const struct clm_data_registry %s; /* Forward declaration */\n\n" +
                          "#ifdef _MSC_VER\n" +
                          "    #pragma warning(disable:4295) /* Warning on CCs, specified as \"XX\" */\n" +
                          "#endif /* _MSC_VER */\n" +
                          "#ifdef __GNUC__\n" +
                          "    #pragma GCC diagnostic ignored \"-Wmissing-field-initializers\"\n" +
                          "#endif /* __GNUC__ */\n" +
                          "#if defined(MACOSX)\n" +
                          "    #pragma clang diagnostic push\n" +
                          "    #pragma clang diagnostic ignored \"-Wmissing-field-initializers\"\n" +
                          "#endif /* defined(MACOSX) */\n\n" +
                          "/** BLOB header\n" +
                          " * Base of all references, contains version information */\n" +
                          "const struct clm_data_header %s = {\n" +
                          "    CLM_HEADER_TAG,      /* Magic word */\n" +
                          "    %-21s/* BLOB format version */\n" +
                          "    %-21s/* CLM version, compiler version */\n" +
                          "    %-21s/* Self reference */\n" +
                          "    %-21s/* Data registry */\n" +
                          "    %-21s/* generator version */\n" +
                          "%s" +
                          "};\n\n" +
                          "#if (%s > CLM_FORMAT_VERSION_MAJOR) || ((%s == CLM_FORMAT_VERSION_MAJOR) && (%s > CLM_FORMAT_VERSION_MINOR))\n" +
                          "    #error CLM data format version mismatch between %s (this file) and wlc_clm_data.h (see CLM_FORMAT_VERSION_MAJOR and CLM_FORMAT_VERSION_MINOR macros in wlc_clm_data.h). This BLOB has format %s\n" +
                          "#endif\n\n")
                         % (_PROGRAM_VERSION, self._data.clm_version,
                            (" (%s)" % self._data.get_apps_version()) if self._data.get_apps_version() is not None else "",
                            generator,
                            (" * User string: %s\n" % self._options._user_string) if (self._options._user_string is not None) and self._bf.has_user_string else "",
                            command_lines,
                            base_version_line,
                            "Proprietary" if self._options.obfuscate else "Secret",
                            self._format_name("data", prefix=True),
                            self._format_name("header", prefix=True),
                            self._bf.version_string.replace(".", ", ") + ", ",
                            "\"%s\", \"%s\"," % (self._data.clm_version, _PROGRAM_VERSION),
                            "&%s_header," % self._data_name_prefix(),
                            "&%s_data," % self._data_name_prefix(),
                            "\"%s\"," % generator,
                            ("    %-21s/* SW Apps version */\n" % ("\"%s\"," % (self._data.get_apps_version() if self._data.get_apps_version() is not None else ""))) if self._bf.apps_version else "",
                            ClmUtils.major(self._bf.version_string), ClmUtils.major(self._bf.version_string), ClmUtils.minor(self._bf.version_string),
                            re.search(r"([^\\/:]*)$", self._blob_file_name).group(1),
                            self._bf.version_string
                            ))

    def _write_used_channels(self):
        """ Writes valid used channel combs to BLOB """
        for channel_type in ChannelType.all():
            if (channel_type.ulb and not self._filter_params.ulb) or ((channel_type.bandwidth > Bandwidth._20) and not self._bf.high_bw_combs):
                continue
            combs_list = sorted(self._data.used_channel_combs[channel_type],
                                key=lambda comb: "%s %04d" %
                                                 (self._data.valid_channels.get_channel_flavor(channel_type, comb.first),
                                                  comb.first))
            band_name = Band.name[channel_type.band]
            bandwidth_name = Bandwidth.name[channel_type.bandwidth]
            bandwidth_name_ident = Bandwidth.id_name[channel_type.bandwidth]
            if combs_list:
                list_name = "valid_channels_%sg_%sm" % (band_name[0], bandwidth_name_ident)
                self._write_blob("/** Valid %sM channels of %sG band */\n"
                                 "static const struct clm_channel_comb %s[] = {\n" %
                                 (bandwidth_name, band_name, self._format_name(list_name)))
                for comb in combs_list:
                    last_comb_channel = comb.first + (comb.number - 1) * comb.stride
                    self._write_blob("    {%3d, %3d, %d}, /* %d - %d %s%s */\n" %
                                     (comb.first, last_comb_channel, comb.stride,
                                      comb.first, last_comb_channel,
                                      "with step of " if (comb.number > 1) else "",
                                      str(comb.stride) if (comb.number > 1) else ""))
                self._write_blob("};\n\n")
            else:
                list_name = "0"
                self._write_blob("/* No valid %sM channels of %sG band */\n\n" %
                                 (bandwidth_name, band_name))
            self._write_blob(("/** Set of %sM %sG valid channels' set */\n" +
                              "static const struct clm_channel_comb_set %s = {\n" +
                              "    %d, %s\n" +
                              "};\n\n") %
                             (bandwidth_name,
                              band_name,
                              self._format_name("valid_channel_%sg_%sm_set" % (band_name[0], bandwidth_name_ident)),
                              len(combs_list),
                              list_name))

    def _write_channel_ranges(self):
        """ Writes channel ranges to BLOB """
        for bw in Bandwidth.all() if self._bf.per_bw_rs else [None]:
            bw_comment = "" if bw is None else ("%sM " % Bandwidth.name[bw])
            bw_name_suffix = ("" if bw is None else ("_%sm" % Bandwidth.id_name[bw]))
            channel_ranges = self._data.channel_ranges_per_bw[bw]
            self._check_limit(len(channel_ranges), "Number of %schannel ranges" % bw_comment,
                              8, ClmBlob.IDX_NUM_SPECIAL_CHANNEL_RANGE, is_index=False,
                              no_warning=not self._data.ranges_split)
            if channel_ranges:
                self._write_blob("/** %shannel ranges used in locales and restricted sets */\n"
                                 % ("C" if bw is None else (bw_comment + " c")))
                self._write_blob("static const struct clm_channel_range %s[] = {\n"
                                 % self._format_name("channel_ranges%s" % bw_name_suffix))
                with ClmBlob.MulticolumnWriter(4, 12, 8, self._write_blob) as mc_writer:
                    for cr in channel_ranges:
                        if cr.is_80_80:
                            active, other = cr.decompose_80_80()
                            mc_writer("{%3d, %3d}," % (active, other))
                        else:
                            mc_writer("{%3d, %3d}," % (cr.start, cr.end))
                self._write_blob(("};\n\n" +
                                  "/** Indices for %schannel ranges */\n" +
                                  "enum range%s {\n") % (bw_comment, bw_name_suffix))
                with ClmBlob.MulticolumnWriter(4, 31, 4, self._write_blob) as mc_writer:
                    for i in range(len(channel_ranges)):
                        cr = channel_ranges[i]
                        mc_writer("%-23s = %d," % (self._range_name(cr), i))
                self._write_blob("};\n\n")
            else:
                self._write_blob("/* No %schannel ranges */\n\n" %
                                 ("" if bw is None else (Bandwidth.name[bw] + "M ")))

    def _write_restricted_sets(self):
        """ Writes restricted sets to BLOB """
        self._check_limit(len(self._data.restricted_sets),
                          "Number of restricted sets",
                          8, ClmBlob.IDX_NUM_SPECIAL_RESTRICTED_SET, is_index=False)
        if self._data.restricted_sets:
            self._write_blob("/** Restricted channel sets */\n" +
                             "static const unsigned char %s[] = {\n" % self._format_name("restricted_channels"))
            for restricted_set in self._data.restricted_sets:
                self._write_blob("    /* Restricted set %2s */ " % restricted_set.name)
                if restricted_set.is_all_channels:
                    self._write_blob("1, CLM_RANGE_ALL_CHANNELS,")
                else:
                    self._write_blob("%d," % len(restricted_set.channels))
                    for cr in restricted_set.channels:
                        self._write_blob(" %s," % self._range_name(cr))
                self._write_blob("\n")
            self._write_blob("};\n\n")
        else:
            self._write_blob("/* No restricted channel sets */\n\n")
        self._write_blob("/** Indices for restricted channel sets */\n" +
                         "enum restricted_set {\n" +
                         "    RESTRICTED_SET_NONE = CLM_RESTRICTED_SET_NONE,\n")
        for i in range(len(self._data.restricted_sets)):
            rs = self._data.restricted_sets[i]
            self._write_blob("    %-19s = %d,\n" % (self._restricted_set_name(rs.name), i))
        self._write_blob("};\n\n")

    def _write_locales(self):
        """ Writes locales to BLOB """
        channel_sets = self._data.locale_channel_sets
        self._check_limit(len(channel_sets), "Number of channel sets",
                          8, ClmBlob.IDX_NUM_SPECIAL_CHANNEL_SET, is_index=False)
        if channel_sets:
            self._write_blob("/** Locale channel sets */\n" +
                             "static const unsigned char %s[] = {\n" % self._format_name("locale_channels"))
            for i in range(len(channel_sets)):
                cl = list(channel_sets[i])
                cl.sort(key=methodcaller("sort_key"))
                self._write_blob(("    /* Channel set %3d */" +
                                 " %d,") % (i, len(cl)))
                for cr in cl:
                    self._write_blob(" %s," % self._range_name(cr))
                self._write_blob("\n")
            self._write_blob("};\n\n")

            self._write_blob("/** Channel set indices */\nenum channel_set {\n")
            with ClmBlob.MulticolumnWriter(4, 23, 4, self._write_blob) as mc_writer:
                for i in range(len(channel_sets)):
                    mc_writer("CHANNEL_SET_%-3d = %d," % (i, i))
            self._write_blob("};\n\n")
        else:
            self._write_blob("/** No locale channel sets */\n\n")

        for ext_rates in self._data.rate_sets_per_bw_band:
            for bw in Bandwidth.all() if self._bf.per_bw_rs else [None]:
                if bw not in self._data.rate_sets_per_bw_band[ext_rates]:
                    continue
                for band in Band.all() if self._bf.per_band_rate_sets else [None]:
                    rate_set_count = len(self._data.rate_sets_per_bw_band[ext_rates][bw][band])
                    set_name = "Number of %s rate sets" % self._rate_set_id_for_comment(bw, band, ext_rates)
                    if rate_set_count > self._value_limit(8, ClmBlob.IDX_NUM_SPECIAL_RATE_SET):
                        ClmUtils.warning("%s is %d which exceeds the limit of %d. BLOB will be written in single-rate mode" %
                                         (set_name, rate_set_count, self._value_limit(8, ClmBlob.IDX_NUM_SPECIAL_RATE_SET)))
                        self._direct_rates_mapping = {}
                    elif self._data.rate_sets_fully_split:
                        self._check_limit(rate_set_count, set_name, 8,
                                          ClmBlob.IDX_NUM_SPECIAL_RATE_SET, is_index=False)
        if any(self._data.rate_sets_per_bw_band):
            self._write_blob(
                ("/* Encodes extended rate as difference from %s */\n" +
                 "#define ER(ext_rate) (unsigned char)(ext_rate - %s)\n\n") %
                (ClmBlob.EXT_RATES_BASE, ClmBlob.EXT_RATES_BASE))
        for is_ext in self._data.rate_sets_per_bw_band:
            for bw in Bandwidth.all() if (self._bf.per_bw_rs and (self._direct_rates_mapping is None)) else [None]:
                if bw not in self._data.rate_sets_per_bw_band[is_ext]:
                    continue
                for band in Band.all() if (self._bf.per_band_rate_sets and (self._direct_rates_mapping is None)) else [None]:
                    rate_sets = self._data.rate_sets_per_bw_band[is_ext][bw][band]
                    bw_band_comment = self._rate_set_id_for_comment(bw, band, is_ext)
                    bw_band_prefix_comment = ("%s r" % bw_band_comment) if bw_band_comment else "R"
                    bw_band_name_suffix = self._rate_set_list_suffix(bw, band, is_ext)
                    rate_name = lambda r: ("ER(%s)" % r.blob_index) if is_ext else r.blob_index
                    if rate_sets:
                        self._write_blob("/** %sate sets */\n" % bw_band_prefix_comment)
                        self._write_blob("static const unsigned char %s[] = {\n"
                                         % self._format_name("rate_sets%s" % bw_band_name_suffix.lower()))
                        num_sets = 0
                        if self._direct_rates_mapping is not None:
                            for i in range(RatesInfo.NUM_RATES):
                                rate = RatesInfo.get_rate(i)
                                if (not rate.singular) or \
                                   ((True in self._data.rate_sets_per_bw_band) and (not self._data.is_ext_rate(rate) != (not is_ext))):
                                    continue
                                self._write_blob("    1, %s, /* %sate set %d */\n" %
                                                 (rate_name(rate) if Locale.check_rate(rate, max_chains=self._bf.max_chains, vht=self._bf.ac_allowed, reg_caps=self._bf.loc_reg_caps) else "0",
                                                  bw_band_prefix_comment, num_sets))
                                self._direct_rates_mapping[rate] = (is_ext, num_sets)
                                num_sets += 1
                        else:
                            num_sets = len(rate_sets)
                            for i in range(num_sets):
                                rates = list(rate_sets[i])
                                rates.sort(key=attrgetter("index"))
                                self._write_blob("    /* %sate set %d */ %d,\n"
                                                 % (bw_band_prefix_comment, i, len(rates)))
                                with ClmBlob.MulticolumnWriter(4, 33, 4, self._write_blob) as mc_writer:
                                    for rate in rates:
                                        mc_writer(rate_name(rate) + ",")
                        self._write_blob("};\n\n")

                        self._write_blob("/** %sate set indices */\n" % bw_band_prefix_comment)
                        self._write_blob("enum rate_set%s {\n" % bw_band_name_suffix.lower())
                        with ClmBlob.MulticolumnWriter(4, 30, 4, self._write_blob) as mc_writer:
                            for i in range(num_sets):
                                mc_writer("RATE_SET%s_%-3d = %d," % (bw_band_name_suffix, i, i))
                        self._write_blob("};\n\n")
                    else:
                        self._write_blob("/** No %s rate sets */\n\n" % bw_band_comment)

        for loc_type in self._locales_of_loc_type:
            locales_list = self._locales_of_loc_type[loc_type]
            self._write_blob(("/** Indices for %s locales */\n" +
                              "enum locale_%sg_%sidx {\n")
                             % (str(loc_type), Band.name[loc_type.band][0],
                                ("" if loc_type.is_base else "ht_")))
            for suffix in ("NONE", "SAME", "DELETED"):
                n = "LOCALE_%sG_%s_%s" % (Band.name[loc_type.band][0], ("IDX" if loc_type.is_base else "HT_IDX"), suffix)
                self._write_blob("    %-33s = CLM_LOC_%s,\n" % (n, suffix))
            loc_idx = 0
            with ClmBlob.MulticolumnWriter(4, 41, 3, self._write_blob) as mc_writer:
                for locale in locales_list:
                    while self._loc_idx_12_flag_swap and (loc_idx in ClmBlob.SPECIAL_10_BIT_LOC_INDICES):
                        loc_idx += 1
                    mc_writer("%-33s = %d," % (self._locale_idx_name(locale), loc_idx))
                    loc_idx += 1
            self._write_blob("};\n\n")
        for loc_type in self._locales_of_loc_type:
            locales_list = self._locales_of_loc_type[loc_type]
            if locales_list:
                self._write_blob("/** %s locale definitions */\n" % str(loc_type))
                self._write_blob("static const unsigned char %s[] = {\n"
                                 % self._format_name("locales_%sg_%s"
                                                     % (Band.name[loc_type.band][0],
                                                        "base" if loc_type.is_base else "ht")))
                loc_idx = 0
                for locale in locales_list:
                    self._locale_sizes[locale.loc_id] = 0
                    self._locale_channel_ranges[locale.loc_id] = {}
                    self._locale_rate_sets[locale.loc_id] = {}
                    while self._loc_idx_12_flag_swap and (loc_idx in ClmBlob.SPECIAL_10_BIT_LOC_INDICES):
                        self._write_blob(
                            ("\n    /* Stub for skipping reserved 10-bit locale index */\n" +
                             "    %s\n") % ("0, " * (6 if loc_type.is_base else 2)))
                        loc_idx += 1
                    loc_idx += 1
                    target = "- " + locale.target + " "
                    if self._options.obfuscate:
                        target = ""
                    self._write_blob("\n    /* Locale %s %s%s*/\n"
                                     % (locale.get_unique_name(),
                                        ("(" + locale.blob_alias + ") ") if (locale.get_unique_name() != locale.blob_alias) else "",
                                        target))
                    if loc_type.is_base:
                        flags = ""
                        flags = self._update_flags((locale.dfs == Dfs.EU), "CLM_DATA_FLAG_DFS_EU", flags)
                        flags = self._update_flags((locale.dfs == Dfs.US), "CLM_DATA_FLAG_DFS_US", flags)
                        flags = self._update_flags((locale.dfs == Dfs.TW), "CLM_DATA_FLAG_DFS_TW", flags)
                        flags = self._update_flags(locale.filt_war1, "CLM_DATA_FLAG_FILTWAR1", flags)
                        if not flags:
                            flags = "0"
                        self._write_blob("    CHANNEL_SET_%d, %s, /* Locale channels (%sM), locale flags */\n" %
                                         (locale.channel_set_id, flags, Bandwidth.name[Bandwidth.default]))
                        self._write_blob("    %s, /* Restricted channels */\n" %
                                         self._restricted_set_name(locale.restricted_set_name))
                        self._write_blob("        %d, /* Public Maxpwr: %d elt(s) */\n" %
                                         (len(locale.reg_power), len(locale.reg_power)))
                        sorted_reg_powers = list(locale.reg_power)
                        sorted_reg_powers.sort(key=lambda power: power.channel_range.sort_key())
                        for power in sorted_reg_powers:
                            self._write_blob("    %d, %-21s /* %ddBm, %d-%d */\n" %
                                             (int(power.powers_dbm[0]),
                                              self._range_name(power.channel_range) + ",",
                                              int(power.powers_dbm[0]),
                                              power.channel_range.start,
                                              power.channel_range.end))
                            self._locale_channel_ranges[locale.loc_id].setdefault(power.channel_range.channel_type.bandwidth, set()).add(power.channel_range)
                        self._locale_sizes[locale.loc_id] += ClmBlob.BASE_LOC_HDR_SIZE + ClmBlob.LOC_PUB_PWR_HDR_SIZE + \
                            len(sorted_reg_powers) * ClmBlob.LOC_PUB_PWR_REC_SIZE

                    power_type_dict = self._build_power_type_dict(locale)
                    if power_type_dict:
                        sorted_power_types = sorted(power_type_dict.keys(), key=methodcaller("sort_key"))
                        for pti in range(len(sorted_power_types)):
                            power_type = sorted_power_types[pti]
                            last_power_type = pti == (len(sorted_power_types) - 1)
                            if self._direct_rates_mapping is None:
                                self._locale_sizes[locale.loc_id] += \
                                    self._write_tx_power_group_header(len(power_type_dict[power_type]), power_type, last_power_type)
                            sorted_powers = sorted(power_type_dict[power_type],
                                                   key=lambda power_rsidx: (power_rsidx[0].channel_range.sort_key(), power_rsidx[1]))
                            for pi in range(len(sorted_powers)):
                                power, rate_set_idx, rates_frozenset = sorted_powers[pi]
                                self._locale_channel_ranges[locale.loc_id].setdefault(power.channel_range.channel_type.bandwidth, set()).add(power.channel_range)
                                rates = sorted(rates_frozenset, key=attrgetter("index"))
                                if self._direct_rates_mapping is not None:
                                    self._locale_sizes[locale.loc_id] += \
                                        self._write_tx_power_group_header(len(rates), power_type, last_power_type and (pi == (len(sorted_powers) - 1)))
                                    for rate in rates:
                                        self._locale_sizes[locale.loc_id] += \
                                            self._write_tx_power(power, self._direct_rates_mapping[rate][1], rate.name, power_type)
                                        self._locale_rate_sets[locale.loc_id].\
                                            setdefault(power.channel_range.channel_type.band, {}).\
                                            setdefault(power.channel_range.channel_type.bandwidth, {}).\
                                            setdefault(bool(power_type.ext_rates), set()).\
                                            add(frozenset([rate]))
                                else:
                                    rate_ranges = []
                                    for ri in rates:
                                        prev_ri = rate_ranges[len(rate_ranges) - 1][1] if rate_ranges else None
                                        if prev_ri and (prev_ri.index + 1 == ri.index) and (prev_ri.group == ri.group):
                                            rate_ranges[len(rate_ranges) - 1][1] = ri
                                        else:
                                            rate_ranges.append([ri, ri])

                                    rate_ranges_str = ""
                                    for rr in rate_ranges:
                                        if rate_ranges_str:
                                            rate_ranges_str += ", "
                                        if rr[0] == rr[1]:
                                            rate_ranges_str += rr[0].name
                                        else:
                                            rate_ranges_str += rr[0].modulation_type % ("%d-%d" % (rr[0].modulation_index, rr[1].modulation_index))
                                    self._locale_sizes[locale.loc_id] += \
                                        self._write_tx_power(power, rate_set_idx, rate_ranges_str, power_type)
                                    self._locale_rate_sets[locale.loc_id].\
                                        setdefault(power.channel_range.channel_type.band, {}).\
                                        setdefault(power.channel_range.channel_type.bandwidth, {}).\
                                        setdefault(bool(power_type.ext_rates), set()).\
                                        add(rates_frozenset)
                    else:
                        self._write_blob("        0, 0, /* Locale Maxpwr: 0 elts */\n")
                        self._locale_sizes[locale.loc_id] += ClmBlob.LOC_CHAN_PWR_HDR_SIZE
                self._write_blob("};\n\n\n")
            else:
                self._write_blob("/* No %sG %s locale definitions */\n\n\n" %
                                 (Band.name[loc_type.band], ("base" if loc_type.is_base else "HT")))

    def _build_power_type_dict(self, locale):
        """ Builds power type dictionary for given locale

        Returns dictionary TxPowerTypeInfo objects as keys and lists of
        (power, rate_set_idx, rate_frozenset) tuples as values. rate_set_idx is
        an index of power's rate set in correspondent rate set list (common,
        per-bandwidth or per-bandwidth-band (main or extended))
        """
        ret = {}
        for power, rate_set in locale.chan_power.items():
            bw, band = self._bw_band_of_rate_set_list(power)
            main_rate_sets, ext_rate_sets = self._data.rate_sets_splitter[frozenset(rate_set)]
            for rate_sets, is_ext in [(main_rate_sets, False), (ext_rate_sets, True)]:
                for rs in rate_sets:
                    ret.setdefault(ClmBlob.TxPowerTypeInfo(power, is_ext), []).append(
                        (power, self._data.rate_set_idx_per_bw_band[is_ext][bw][band][rs], rs))
        return ret

    def _write_tx_power_group_header(self, num_records, power_type, last):
        """ Writes to BLOB header of TX power records' group

        Arguments:
        num_records -- Number of TX power records in group
        power_type  -- TxPowerTypeInfo object with common information for
                       records' group
        last        -- True for last group
        Returns length of written header
        """
        ret = ClmBlob.LOC_CHAN_PWR_HDR_SIZE
        self._write_blob("        CLM_DATA_FLAG_WIDTH_%s | CLM_DATA_FLAG_MEAS_%s%s%s%s, /* Maxpower Group Flags%s */\n" %
                         ("80_80" if power_type.is_80_80 else Bandwidth.id_name[power_type.bandwidth],
                          Measure.name[power_type.measure][:4].upper(),
                          (" | CLM_DATA_FLAG_PER_ANT_%d" % power_type.extra_powers) if power_type.extra_powers else "",
                          "" if last else " | CLM_DATA_FLAG_MORE",
                          " | CLM_DATA_FLAG_FLAG2" if power_type.use_flag2() else "",
                          "" if last else ", continuation"))
        if power_type.use_flag2():
            ret = ClmBlob.LOC_CHAN_PWR_HDR2_SIZE
            flag2 = []
            if Bandwidth.is_ulb(power_type.bandwidth):
                flag2.append("CLM_DATA_FLAG2_WIDTH_EXT")
            if power_type.ext_rates:
                flag2.append("CLM_DATA_FLAG2_EXT_RATES")
            self._write_blob("        %s,\n" % (" | ".join(flag2)))
        self._write_blob("        %d, /*  Locale Maxpwr: %d elt(s) */\n" % (num_records, num_records))
        return ret

    def _write_tx_power(self, power, rate_set_id, rate_names, power_type):
        """ Prints single TX power record to BLOB.

        Arguments:
        power       -- Power object that contains channel range and power value
        rate_set_id -- ID of rates' set (integer)
        rate_names  -- Names of rates contained in rate set
        power_type  -- TxPowerTypeInfo object with common information for
                       records' group
        Returns length of written record
        """
        bw, band = self._bw_band_of_rate_set_list(power)
        self._write_blob("    %s, %-21s %-17s %s/* %s, %7s, (%s) */\n"
                         % ("CLM_DISABLED_POWER" if power.is_disabled else str(int(power.powers_dbm[0] * 4)),
                            self._range_name(power.channel_range) + ",",
                            "RATE_SET%s_%d," % (self._rate_set_list_suffix(bw, band, power_type.ext_rates), rate_set_id),
                            "".join("%d, " % (power.powers_dbm[i + 1] * 4) for i in range(power_type.extra_powers)),
                            "Disabled" if power.is_disabled else
                            ("%5.2f%sdBm" % (power.powers_dbm[0],
                                             "".join("/%5.2f" % (power.powers_dbm[i + 1]) for i in range(power_type.extra_powers)))),
                            power.channel_range.range_str,
                            rate_names))
        return ClmBlob.LOC_CHAN_PWR_REC_SIZE + power_type.extra_powers

    def _bw_band_of_rate_set_list(self, power):
        """ Returns band and bandwidth of rate set list that contains rate set
        for given power object
        """
        return (None, None) if (self._direct_rates_mapping is not None) or not self._bf.per_bw_rs else \
               ((power.bandwidth, None) if not self._bf.per_band_rate_sets else
                (ClmData.rate_set_bw_mapper[power.bandwidth], power.channel_range.channel_type.band))

    def _rate_set_id_for_comment(self, bw, band, ext_rates):
        """ Rate set type identifier to use in comments

        Arguments:
        bw        -- Bandwidth or None
        band      -- Band or None
        ext_rates -- True for extended rate sets
        """
        ret = []
        if band is not None:
            ret.append(Band.name[band] + "G")
        if bw is not None:
            ret.append(Bandwidth.name[bw] + "M")
        if ext_rates:
            ret.append("ext")
        return " ".join(ret)

    def _rate_set_list_suffix(self, bw, band, ext_rates):
        """ Returns suffix that identifies rate sets list for given band and
        bandwidth

        Arguments:
        bw        -- Bandwidth for per-bandwidth or per-bandwidth-band rate set
                     list, None if common rate set list is used
        band      -- Band for per-bandwidth-band rate set list, None for common
                    and per-bandwidth list
        ext_rates -- True for extended rates' set, false for main rates' set
        Returns suffix to use for given rate list
        """
        ret = ""
        if band is not None:
            ret += "_" + Band.name[band][0] + "G"
        if bw is not None:
            ret += "_" + Bandwidth.id_name[bw] + "M"
        if ext_rates:
            ret += "_EXT"
        return ret

    def _write_sub_chan_rules(self):
        """ Writes subchannel rules to BLOB """
        if not self._bf.sub_chan_rules:
            return
        sub_chan_rules = self._data.sub_chan_rules
        self._check_limit(len(sub_chan_rules),
                          "Number of subchannel rules",
                          max(self._bf.scr_idx_bits,
                              4 if self._filter_params.scr_idx_4bit else 0,
                              8 if self._filter_params.scr_idx_8bit else 0),
                          ClmBlob.IDX_NUM_SPECIAL_SUB_CHAN_RULE, is_index=False)
        scr_idx_initialize = self._bf.scr_idx_initialize or self._filter_params.scr_idx_4bit
        self._write_blob(
            ("/** Subchannel rule indices */\n" +
             "enum sub_chan_rule {\n" +
             "    SCR_NONE%s,\n") %
            (" = CLM_SCR_IDX_NONE" if scr_idx_initialize else ""))
        for region_rules in sub_chan_rules:
            self._write_blob(
                "    SCR_%d%s,%s\n" %
                (self._sub_chan_region_rules_idx(region_rules.name),
                 (" = CLM_SCR_IDX_%d" % self._sub_chan_region_rules_idx(region_rules.name)) if scr_idx_initialize else "",
                 ("    /* %s */" % region_rules.name) if not self._options.obfuscate else ""))
        self._write_blob("};\n\n")

        sub_chan_rules_40 = self._bf.sub_chan_rules_40 or self._filter_params.sub_chan_rules_40
        self._region_sc_increments = {}
        for band in Band.all():
            for bandwidth in ([Bandwidth._40] if sub_chan_rules_40 else []) + [Bandwidth._80, Bandwidth._160]:
                if bandwidth not in ChannelType.bandwidths_of_band(band):
                    continue
                channel_type = ChannelType.create(band, bandwidth)
                self._region_sc_increments[channel_type] = {}
                bw_name = Bandwidth.name[bandwidth]
                band_name = Band.name[band]
                bw_id_name = Bandwidth.id_name[bandwidth]
                if self._data.has_sub_chan_rules_for(channel_type):
                    type_suffix = "" if (self._bf.max_bandwidth == Bandwidth._80) else (bw_id_name + "_")
                    self._write_blob("/* %sMHz subchannel rules */\n\n" % bw_name)
                    for region_rules in sub_chan_rules:
                        if not region_rules.get_num_channel_type_rules(channel_type):
                            continue
                        if not self._options.obfuscate:
                            self._write_blob("/* %sGHz, %sMHz subchannel region rules %s */\n" %
                                             (band_name, bw_name, region_rules.name))
                        self._write_blob(
                            "static const clm_sub_chan_channel_rules_%s%st %s[] = {\n" %
                            (type_suffix,
                             "inc_" if self._sub_chan_inc_fmt == ClmBlob.SUB_CHAN_INC_FMT_COUPLED else "",
                             self._format_name("sub_chan_channel_rules_%sg_%sm_%d" %
                                               (band_name[0], bw_id_name,
                                                self._sub_chan_region_rules_idx(region_rules.name)))))
                        self._region_sc_increments[channel_type][region_rules.name] = []
                        sorted_channel_ranges = sorted(filter(lambda cr: cr.channel_type == channel_type, region_rules.chan_rules_dict.keys()),
                                                       key=methodcaller("sort_key"))
                        for channel_range in sorted_channel_ranges:
                            channel_rule = region_rules.chan_rules_dict[channel_range]
                            self._write_blob("    {%s, {\n" % self._range_name(channel_range))
                            for sub_chan_idx in range(len(SubChanId.valid_sub_chan_ids(bandwidth))):
                                self._write_blob("    /* %-3s */ " % SubChanId.name[sub_chan_idx])
                                if self._sub_chan_inc_fmt == ClmBlob.SUB_CHAN_INC_FMT_COUPLED:
                                    self._write_blob("{")
                                if sub_chan_idx in channel_rule:
                                    bandwidths = list(channel_rule[sub_chan_idx][0])
                                    bandwidths.sort()
                                    for bw_idx in range(len(bandwidths)):
                                        self._write_blob("%sCLM_DATA_FLAG_SC_RULE_BW_%s" % (" | " if bw_idx else "", Bandwidth.id_name[bandwidths[bw_idx]]))
                                else:
                                    self._write_blob("0")
                                increment = int(channel_rule.get(sub_chan_idx, [0, 0])[1] * 4)
                                self._region_sc_increments[channel_type][region_rules.name].append(increment)
                                if self._sub_chan_inc_fmt == ClmBlob.SUB_CHAN_INC_FMT_COUPLED:
                                    self._write_blob(", %d}" % increment)
                                self._write_blob(",\n")
                            self._write_blob("    }},\n")
                        self._write_blob("};\n\n")
                        if not any(inc != 0 for inc in self._region_sc_increments[channel_type][region_rules.name]):
                            del self._region_sc_increments[channel_type][region_rules.name]
                        if self._sub_chan_inc_fmt == ClmBlob.SUB_CHAN_INC_FMT_SEPARATE:
                            if region_rules.name not in self._region_sc_increments[channel_type]:
                                continue
                            if not self._options.obfuscate:
                                self._write_blob("/* %sGHz, %sMHz subchannel region increments (power offsets) for %s */\n" %
                                                 (band_name, bw_name, region_rules.name))
                            self._write_blob(
                                "static const signed char %s[] = {\n" %
                                (self._format_name("sub_chan_channel_increments_%sg_%sm_%d" %
                                                   (band_name[0], bw_id_name,
                                                    self._sub_chan_region_rules_idx(region_rules.name)))))
                            for range_idx in range(len(sorted_channel_ranges)):
                                self._write_blob(
                                    "    %s, /* %s */\n" %
                                    (", ".join(str(x) for x in self._region_sc_increments[channel_type][region_rules.name]
                                               [range_idx * len(SubChanId.valid_sub_chan_ids(bandwidth)):
                                                (range_idx + 1) * len(SubChanId.valid_sub_chan_ids(bandwidth))]),
                                     sorted_channel_ranges[range_idx].range_str))
                            self._write_blob("};\n\n")
                    self._write_blob(
                        ("/** Definitions of %sGHz, %sMHz region subchannel rules */\n" +
                         "static const %s %s[] = {\n")
                        % (band_name, bw_name,
                           "clm_sub_chan_region_rules_inc_t" if self._sub_chan_inc_fmt == ClmBlob.SUB_CHAN_INC_FMT_SEPARATE else ("clm_sub_chan_region_rules_%st" % type_suffix),
                           self._format_name("sub_chan_region_rules_%sg_%sm" % (band_name[0], bw_id_name))))
                    for region_rules in sub_chan_rules:
                        num_rules = region_rules.get_num_channel_type_rules(channel_type)
                        if num_rules:
                            self._write_blob(
                                "    {%d, sub_chan_channel_rules_%sg_%sm_%d%s},\n" %
                                (num_rules, band_name[0], bw_id_name, self._sub_chan_region_rules_idx(region_rules.name),
                                 "" if self._sub_chan_inc_fmt != ClmBlob.SUB_CHAN_INC_FMT_SEPARATE else
                                 ((", sub_chan_channel_increments_%sg_%sm_%d" % (band_name[0], bw_id_name, self._sub_chan_region_rules_idx(region_rules.name)))
                                  if region_rules.name in self._region_sc_increments[channel_type] else ", 0")))
                        else:
                            self._write_blob("    {0, 0},\n")
                    self._write_blob(("};\n\n" +
                                      "/** Set of %sGHz, %sMHz region subchannel rules */\n" +
                                      "static const clm_sub_chan_rules_set_%st %s = {\n" +
                                      "    %d, sub_chan_region_rules_%sg_%sm\n" +
                                      "};\n\n") %
                                     (band_name, bw_name, type_suffix,
                                      self._format_name("sub_chan_rules_set_%sg_%sm" % (band_name[0], bw_id_name)),
                                      len(sub_chan_rules), band_name[0], bw_id_name))
                elif bandwidth <= self._bf.max_bandwidth:
                    self._write_blob("/* No %sGHz, %sMHz subchannel rules */\n\n" % (band_name, bw_name))

    def _write_regions(self):
        """ Writes regions to BLOB """
        regions = self._data.regions
        deleted_regions = self._data.deleted_regions
        extra_ccrevs = self._data.extra_ccrevs
        if self._bf.indexed_ccrev and \
           (regions or deleted_regions or extra_ccrevs):
            self._write_blob("/** CC/rev indices */\n" +
                             "enum ccrevs {\n")
            with ClmBlob.MulticolumnWriter(4, 31, 4, self._write_blob) as mc_writer:
                for ccrev in (list(map(itemgetter(0), deleted_regions)) +
                              list(map(attrgetter("ccrev"), regions)) +
                              extra_ccrevs):
                    mc_writer("%-14s /* %s */" % ("%s," % self._ccrev_idx_name(ccrev), str(ccrev)))
            self._write_blob("};\n\n")

        if regions or deleted_regions:
            self._write_blob(
                ("/** Expands to region definition record */\n" +
                 "%s\n" +
                 "/** Region definitions */\n" +
                 "static const %s %s[] = {\n" +
                 "    /*      CC    rev      2.4        5            2.4 HT              5 HT  SubChan  flags */\n") %
                (self._reg_rec_info.macro, self._reg_rec_info.struct_type,
                 self._format_name("country_definitions")))
            idx = []
            for loc_type in LocaleType.all():
                if loc_type.flavor == LocaleType.HT3:
                    continue
                idx.append(loc_type)
            for ccrev, _ in deleted_regions:
                self._write_blob("    REGION(\"%s\", %4s, DELETED, DELETED,          DELETED,          DELETED, 0, 0), /* %s deleted */\n" %
                                 (ccrev.cc, self._rev_str(ccrev), ccrev))
            for region in regions:
                names = ", ".join([("%7s" if loc_type.is_base else "%16s") % self._reg_loc_idx_name(region, loc_type) for loc_type in idx])
                if self._bf.sub_chan_rules:
                    scr = "%-9s" % ("SCR_%d," % self._sub_chan_region_rules_idx(region.sub_chan_rules_name) if region.sub_chan_rules_name else "SCR_NONE,")
                else:
                    scr = "0, "
                flags = " | ".join(flag for cond, flag in [
                    (self._bf.has_default_for_cc and region.default_for_cc, "CLM_DATA_FLAG_REG_DEF_FOR_CC"),
                    (self._bf.loc_reg_caps.txbf and region.loc_reg_caps.txbf, "CLM_DATA_FLAG_REG_TXBF"),
                    ((region.edcrs != Edcrs.DEFAULT) and (region.edcrs in self._bf.supported_edcrs),
                     "CLM_DATA_FLAG_REG_EDCRS_%s" % Edcrs.name[region.edcrs])
                ] if cond)
                if not flags:
                    flags = "0"
                if self._reg_rec_info.flags2:
                    flags2 = " | ".join(flag for cond, flag in [
                        (region.lo_gain_nbcal, "CLM_DATA_FLAG_2_REG_LO_GAIN_NBCAL"),
                        (region.chsprwar2, "CLM_DATA_FLAG_2_REG_CHSPRWAR2")
                    ] if cond)
                    if not flags2:
                        flags2 = "0"
                    flags2 = ", " + flags2
                else:
                    flags2 = ""
                self._write_blob("    REGION(\"%s\", %4s, %s, %s%s%s), /* %s%s */\n" %
                                 (region.ccrev.cc, self._rev_str(region.ccrev), names, scr, flags, flags2, region.ccrev,
                                  (", " + region.country_name) if region.country_name and not self._options.obfuscate else ""))
            self._write_blob("};\n\n")
        else:
            self._write_blob("/* No region definitions */\n\n")

        self._write_blob("/** Set of regions */\n"
                         "static const struct clm_country_rev_definition_set %s = {\n"
                         "    %d, %s\n"
                         "};\n\n\n" %
                         (self._format_name("country_definition_set"),
                          len(regions) + len(deleted_regions),
                          ("country_definitions" if regions or deleted_regions else "0")))

        if self._bf.indexed_ccrev:
            if extra_ccrevs:
                self._write_blob(
                    ("/** CC/revs not in region list */\n" +
                     "static const %s %s[] = {\n") %
                    ("clm_cc_rev4_t" if self._rev16 else "clm_cc_rev_t",
                     self._format_name("extra_ccrevs")))
                for ccrev in extra_ccrevs:
                    self._write_blob("    %s,\n" % self._ccrev_ref(ccrev, force_struct=True))
                self._write_blob("};\n\n")
            self._write_blob(
                ("static const clm_cc_rev_set_t %s = {\n" +
                 "    %d, %s\n"
                 "};\n\n") % (self._format_name("extra_ccrevs_set"), len(extra_ccrevs),
                              "&extra_ccrevs" if extra_ccrevs else 0))

        self._write_blob("/* Aliases' definitions */\n\n")
        adcs = self._data.adcs
        for adc in adcs:
            if self._bf.indexed_ccrev:
                ccrev_type = "unsigned short" if self._reg_idx_16 else "unsigned char"
            else:
                ccrev_type = "clm_cc_rev4_t" if self._rev16 else "clm_cc_rev_t"
            self._write_blob(("/** Aliases of region \"%s\" */\n" +
                              "static const %s %s[] = {\n") %
                             (adc.cc, ccrev_type, self._format_name(adc.cc + "_aliases")))
            sorted_adc_regions = list(adc.regions)
            sorted_adc_regions.sort(key=methodcaller("sort_key"))
            for ccrev in sorted_adc_regions:
                self._write_blob("    %s, /* %s */\n" % (self._ccrev_ref(ccrev), ccrev))
            self._write_blob("};\n\n")
        if adcs or self._data.deleted_adcs:
            self._write_blob(
                "/** Aliased regions */\n" +
                "static const struct clm_advertised_cc %s[] = {\n" % self._format_name("advertised_cc_defs"))
            for cc, diff_report in self._data.deleted_adcs:
                self._write_blob("    {\"%s\", CLM_DELETED_NUM, 0},\n" % cc)

            for adc in adcs:
                self._write_blob("    {\"%s\", %d, %s_aliases},\n" %
                                 (adc.cc, len(adc.regions), adc.cc))
            self._write_blob("};\n\n")
        else:
            self._write_blob("/* No aliased regions */\n\n")

        self._write_blob(("/** Set of aliased regions */\n" +
                          "static const struct clm_advertised_cc_set %s = {\n" +
                          "    %d, %s\n" +
                          "};\n\n\n") %
                         (self._format_name("advertised_cc_set"),
                          len(adcs) + len(self._data.deleted_adcs),
                          ("advertised_cc_defs" if adcs or self._data.deleted_adcs else "0")))

    def _write_aggregates(self):
        """ Writes aggregations to BLOB """
        self._write_blob("/** Aggregations' definitions */\n")
        if self._bf.indexed_ccrev:
            ccrev_type = "unsigned short" if self._reg_idx_16 else "unsigned char"
            agg_type = "clm_aggregate_cc16_t"
        else:
            ccrev_type = "clm_cc_rev4_t" if self._rev16 else "clm_cc_rev_t"
            agg_type = "clm_aggregate_cc_t"
        for aggregation in self._data.aggregations:
            if aggregation.mappings:
                name = "cc_map_%s_%d" % (aggregation.ccrev.cc, aggregation.ccrev.rev)
                self._write_blob(("\n/** Mappings of %s/%d aggregation */\n" +
                                  "static const %s %s[] = {\n")
                                 % (aggregation.ccrev.cc, aggregation.ccrev.rev,
                                    ccrev_type, self._format_name(name)))
                mappings = list(aggregation.mappings)
                mappings.sort(key=methodcaller("sort_key"))
                with ClmBlob.MulticolumnWriter(4, 31, 4, self._write_blob) as mc_writer:
                    for ccrev in mappings:
                        mc_writer("%-14s /* %s */" %
                                  (self._ccrev_ref(ccrev) + ",", ccrev))
                self._write_blob("};\n")
        if self._data.aggregations or self._data.deleted_aggregations:
            self._write_blob("\n/** Descriptors of aggregations */\n" +
                             "static const %s %s[] = {\n" %
                             (agg_type, self._format_name("aggregate_cc_defs")))
            for ccrev, _ in self._data.deleted_aggregations:
                self._write_blob("    {{\"%s\", %4s}, CLM_DELETED_NUM, 0}, /* %s */ \n" %
                                 (ccrev.cc, self._rev_str(ccrev), ccrev))
            for aggregation in self._data.aggregations:
                if aggregation.mappings:
                    name = "cc_map_%s_%d" % (aggregation.ccrev.cc, aggregation.ccrev.rev)
                else:
                    name = "0"
                self._write_blob("    {{\"%s\", %4s}, %2d, %s}, /* %s */\n" %
                                 (aggregation.ccrev.cc, self._rev_str(aggregation.ccrev),
                                  len(aggregation.mappings), name, aggregation.ccrev))
            self._write_blob("};\n\n")
        else:
            self._write_blob("\n/* No aggregations */\n\n")

        self._write_blob(
            ("/** Set of aggregations */\n" +
             "static const struct clm_aggregate_cc_set %s = {\n" +
             "    %d, %s\n" +
             "};\n\n\n")
            % (self._format_name("aggregate_cc_set"),
               len(self._data.aggregations) + len(self._data.deleted_aggregations),
               "aggregate_cc_defs" if self._data.aggregations or self._data.deleted_aggregations else "0"))

    def _write_regrev_remaps(self):
        """ Writes regrev remaps """
        if not self._regrev_remap:
            return
        if self._data.regrev_remap_as_base:
            self._write_blob("/* No regrev remaps */\n\n")
            return
        self._write_blob(
            ("#define REMAP(r16, r8) {(unsigned char)(r16 >> 8), (unsigned char)(r16 & 0xFF), (unsigned char)r8}\n"
             "/* Regrev remaps */\n" +
             "static const struct clm_regrev_regrev_remap %s[] = {\n")
            % (self._format_name("regrev_regrev_remaps")))
        cc_remaps = []  # Vector of tuples (cc, first_remap_index), terminated by (None, len_of_remap_table)
        for cc in sorted(self._data.regrev_remap_table):
            cc_remaps.append((cc, 0 if len(cc_remaps) == 0 else (cc_remaps[-1][1] + len(self._data.regrev_remap_table[cc_remaps[-1][0]]))))
            for r16 in sorted(self._data.regrev_remap_table[cc].iterkeys()):
                r8 = self._data.regrev_remap_table[cc][r16]
                self._write_blob(
                    "    REMAP(%4d, %3d), /* %s/%d -> %s/%d */\n"
                    % (r16, r8, cc, r16, cc, r8))
        cc_remaps.append((None, cc_remaps[-1][1] + len(self._data.regrev_remap_table[cc_remaps[-1][0]])))
        self._write_blob(
            ("};\n\n" +
             "/* Per-CC remap descriptors */\n" +
             "static const struct clm_regrev_cc_remap %s[] = {\n")
            % (self._format_name("regrev_cc_remaps")))
        with ClmBlob.MulticolumnWriter(4, 15, 7, self._write_blob) as mc_writer:
            for cc, idx in cc_remaps:
                mc_writer("{%4s, %4d}," % ("\"%s\"" % (cc if cc is not None else ""), idx))
        self._write_blob(
            ("};\n\n" +
             "/* Set of remaps */\n" +
             "static const struct clm_regrev_cc_remap_set %s = {\n" +
             "    %4d, regrev_cc_remaps, regrev_regrev_remaps\n" +
             "};\n\n\n")
            % (self._format_name("regrev_cc_remap_set"), len(cc_remaps) - 1))

    def _write_user_string(self):
        """ Writes user string """
        if not self._bf.has_user_string:
            return
        if self._options.user_string is None:                       # pragma: no cover
            # May not happen because user string has default value. But in case it will change...
            self._write_blob("/* User string not specified */\n")
            return
        self._write_blob(
            ("/** BLOB ID */\n" +
             "static const char %s[] = \"%s\";\n\n\n")
            % (self._format_name("user_string"), self._options.user_string))

    def _write_footer(self):
        """ Writes BLOB footer """
        sub_chan_rules_40 = self._bf.sub_chan_rules_40 or self._filter_params.sub_chan_rules_40
        flags = " | ".join(flag for cond, flag in [
            (self._bf.reg_rec_10_fl, "CLM_REGISTRY_FLAG_COUNTRY_10_FL"),
            (self._bf.apps_version, "CLM_REGISTRY_FLAG_APPS_VERSION"),
            (self._bf.sub_chan_rules, "CLM_REGISTRY_FLAG_SUB_CHAN_RULES"),
            (self._bf.max_bandwidth >= Bandwidth._160, "CLM_REGISTRY_FLAG_160MHZ"),
            (self._bf.per_bw_rs, "CLM_REGISTRY_FLAG_PER_BW_RS"),
            (self._bf.per_band_rate_sets, "CLM_REGISTRY_FLAG_PER_BAND_RATES"),
            (self._bf.has_user_string, "CLM_REGISTRY_FLAG_USER_STRING"),
            (self._bf.numrates_field, "(((int)WL_NUMRATES << CLM_REGISTRY_FLAG_NUM_RATES_SHIFT) & CLM_REGISTRY_FLAG_NUM_RATES_MASK)"),
            (self._bf.high_bw_combs, "CLM_REGISTRY_FLAG_HIGH_BW_COMBS"),
            (self._bf.content_dependent_reg or self._loc_idx_12_flag_swap, "CLM_REGISTRY_FLAG_CD_REGIONS"),
            (self._reg_rec_info.rev_extra_bytes, "CLM_REGISTRY_FLAG_CD_16_BIT_REV"),
            (self._bf.indexed_ccrev and self._reg_idx_16, "CLM_REGISTRY_FLAG_CD_16_BIT_REGION_INDEX"),
            (self._bf.content_dependent_reg or self._loc_idx_12_flag_swap,
             "(%d << CLM_REGISTRY_FLAG_CD_LOC_IDX_BYTES_SHIFT)" % self._reg_rec_info.idx_extra_bytes),
            (self._bf.ext_rate_sets, "CLM_REGISTRY_FLAG_EXT_RATE_SETS"),
            (self._filter_params.ulb, "CLM_REGISTRY_FLAG_ULB"),
            (self._regrev_remap, "CLM_REGISTRY_FLAG_REGREV_REMAP"),
            (sub_chan_rules_40, "CLM_REGISTRY_FLAG_SUBCHAN_RULES_40"),
            (self._sub_chan_inc_fmt == ClmBlob.SUB_CHAN_INC_FMT_COUPLED, "CLM_REGISTRY_FLAG_SUBCHAN_RULES_INC"),
            (self._sub_chan_inc_fmt == ClmBlob.SUB_CHAN_INC_FMT_SEPARATE, "CLM_REGISTRY_FLAG_SUBCHAN_RULES_INC_SEPARATE"),
            (self._reg_flag2_byte, "CLM_REGISTRY_FLAG_REGION_FLAG_2"),
            (self._loc_idx_12_flag_swap, "CLM_REGISTRY_FLAG_REGION_LOC_12_FLAG_SWAP"),
            (self._bf.has_registry_flags2, "CLM_REGISTRY_FLAG_REGISTRY_FLAGS2")
        ] if cond)
        if flags == "":
            flags = "0"
        flags2 = " | ".join(flag for cond, flag in [] if cond)
        if flags2 == "":
            flags2 = "0"

        registry_tags = set()
        for condition, tag in [(True, "MANDATORY"), (self._bf.registry_flags_allowed, "FLAGS"), (self._bf.sub_chan_rules, "SCR80"),
                               (self._bf.sub_chan_rules and (self._bf.max_bandwidth >= Bandwidth._160), "SCR160"),
                               (self._bf.per_bw_rs, "PER_BW_RS"), (self._bf.per_band_rate_sets, "PER_BAND_RS"),
                               (self._bf.has_user_string, "USER_STR"), (self._bf.high_bw_combs, "HIGH_BW_COMBS"),
                               (self._bf.indexed_ccrev, "INDEXED_CCREV"), (self._bf.ext_rate_sets, "EXT_RATE_SETS"),
                               (self._filter_params.ulb, "ULB"), (self._regrev_remap or self._bf.dummy_regrev_remap, "REMAP"),
                               (sub_chan_rules_40, "SCR40"), (self._bf.has_registry_flags2, "FLAGS2")]:
            if condition:
                registry_tags.add(tag)
        self._write_blob(("/** Registry of data structures stored in BLOB */\n" +
                          "const struct clm_data_registry %s = {\n" +
                          "%s\n" +
                          "};\n\n") %
                         (self._format_name("data", prefix=True),
                          self._registry_fields(
                              registry_tags,
                              [("&valid_channel_2g_20m_set", True, "MANDATORY"),
                               ("&valid_channel_5g_20m_set", True, "MANDATORY"),
                               ("channel_ranges%s" % ("_20m" if self._bf.per_bw_rs else ""), self._data.channel_ranges_per_bw[Bandwidth._20 if self._bf.per_bw_rs else None], "MANDATORY"),
                               ("restricted_channels", self._data.restricted_sets, "MANDATORY"),
                               ("locale_channels", self._data.locale_channel_sets, "MANDATORY"),
                               (self._rate_set_list_ptr(Bandwidth._20, Band._5, False), True, "MANDATORY"),
                               ("{%s, %s, %s, %s}" %
                                ("locales_2g_base" if self._data.has_locales(LocaleType(LocaleType.BASE, Band._2)) else "0",
                                 "locales_5g_base" if self._data.has_locales(LocaleType(LocaleType.BASE, Band._5)) else "0",
                                 "locales_2g_ht" if self._data.has_locales(LocaleType(LocaleType.HT, Band._2)) else "0",
                                 "locales_5g_ht" if self._data.has_locales(LocaleType(LocaleType.HT, Band._5)) else "0"),
                                True, "MANDATORY"),
                               ("&country_definition_set", True, "MANDATORY"),
                               ("&advertised_cc_set", True, "MANDATORY"),
                               ("&aggregate_cc_set", True, "MANDATORY"),
                               (flags, True, "FLAGS"),
                               ("&sub_chan_rules_set_5g_80m", self._data.has_sub_chan_rules_for(ChannelType.create(Band._5, Bandwidth._80)), "SCR80"),
                               ("&sub_chan_rules_set_5g_160m", self._data.has_sub_chan_rules_for(ChannelType.create(Band._5, Bandwidth._160)), "SCR160"),
                               ("channel_ranges_40m", self._data.channel_ranges_per_bw[Bandwidth._40], "PER_BW_RS"),
                               ("channel_ranges_80m", self._data.channel_ranges_per_bw[Bandwidth._80], "PER_BW_RS"),
                               ("channel_ranges_160m", self._data.channel_ranges_per_bw[Bandwidth._160], "PER_BW_RS"),
                               (self._rate_set_list_ptr(Bandwidth._40, Band._5, False), True, "PER_BW_RS"),
                               (self._rate_set_list_ptr(Bandwidth._80, Band._5, False), True, "PER_BW_RS"),
                               (self._rate_set_list_ptr(Bandwidth._160, Band._5, False), True, "PER_BW_RS"),
                               (self._rate_set_list_ptr(Bandwidth._20, Band._2, False), True, "PER_BAND_RS"),
                               (self._rate_set_list_ptr(Bandwidth._40, Band._2, False), True, "PER_BAND_RS"),
                               ("user_string", self._options.user_string is not None, "USER_STR"),
                               ("&valid_channel_2g_40m_set", True, "HIGH_BW_COMBS"),
                               ("&valid_channel_5g_40m_set", True, "HIGH_BW_COMBS"),
                               ("&valid_channel_5g_80m_set", True, "HIGH_BW_COMBS"),
                               ("&valid_channel_5g_160m_set", True, "HIGH_BW_COMBS"),
                               ("&extra_ccrevs_set", True, "INDEXED_CCREV"),
                               (self._rate_set_list_ptr(Bandwidth._20, Band._2, True), True, "EXT_RATE_SETS"),
                               (self._rate_set_list_ptr(Bandwidth._40, Band._2, True), True, "EXT_RATE_SETS"),
                               (self._rate_set_list_ptr(Bandwidth._20, Band._5, True), True, "EXT_RATE_SETS"),
                               (self._rate_set_list_ptr(Bandwidth._40, Band._5, True), True, "EXT_RATE_SETS"),
                               (self._rate_set_list_ptr(Bandwidth._80, Band._5, True), True, "EXT_RATE_SETS"),
                               (self._rate_set_list_ptr(Bandwidth._160, Band._5, True), True, "EXT_RATE_SETS"),
                               ("channel_ranges_2_5m", self._data.channel_ranges_per_bw[Bandwidth._2_5], "ULB"),
                               ("channel_ranges_5m", self._data.channel_ranges_per_bw[Bandwidth._5], "ULB"),
                               ("channel_ranges_10m", self._data.channel_ranges_per_bw[Bandwidth._10], "ULB"),
                               ("&valid_channel_2g_2_5m_set", self._bf.high_bw_combs, "ULB"),
                               ("&valid_channel_2g_5m_set", self._bf.high_bw_combs, "ULB"),
                               ("&valid_channel_2g_10m_set", self._bf.high_bw_combs, "ULB"),
                               ("&valid_channel_5g_2_5m_set", self._bf.high_bw_combs, "ULB"),
                               ("&valid_channel_5g_5m_set", self._bf.high_bw_combs, "ULB"),
                               ("&valid_channel_5g_10m_set", self._bf.high_bw_combs, "ULB"),
                               ("&regrev_cc_remap_set", self._regrev_remap and not self._data.regrev_remap_as_base, "REMAP"),
                               ("&sub_chan_rules_set_2g_40m", self._data.has_sub_chan_rules_for(ChannelType.create(Band._2, Bandwidth._40)), "SCR40"),
                               ("&sub_chan_rules_set_5g_40m", self._data.has_sub_chan_rules_for(ChannelType.create(Band._5, Bandwidth._40)), "SCR40"),
                               (flags2, True, "FLAGS2")])))

    def _rate_set_list_ptr(self, bw, band, ext_rates):
        """ Returns name of pointer to rate sets list for given band and
        bandwidth """
        if (self._direct_rates_mapping is not None) or not self._bf.per_bw_rs:
            bw = None
        if (self._direct_rates_mapping is not None) or not self._bf.per_band_rate_sets:
            band = None
        return "rate_sets%s" % (self._rate_set_list_suffix(bw, band, ext_rates).lower()) \
               if (ext_rates in self._data.rate_sets_per_bw_band) and self._data.rate_sets_per_bw_band[ext_rates][bw][band] else "0"

    def _registry_fields(self, tags, fields):
        """ Returns initializer for BLOB registry structure

        Arguments:
        tags   -- set of tags for fields that shall be included to registry
        fields -- Sequence of tuples (value, condition, tag). 'tag' is field
                  tag (roughly corresponds to some field in
                  BlobFormatVersionInfo), 'value' is field value provided field
                  may have (i.e. field tag is in 'tags'), 'condition' is true
                  if field has value. If field doesn't have value ('condition'
                  is false or 'tag' not in tags'), but shall be included into
                  registry (because some subsequent field has value), its value
                  is "0"
        Returns string with registry structure initializers
        """
        prev_tag = None
        completed_tags = set()
        ret = ""
        for field, condition, tag in fields:
            if (completed_tags == tags) and (tag != prev_tag):
                break
            assert (tag not in completed_tags) or (tag == prev_tag)
            prev_tag = tag
            if tag in tags:
                completed_tags.add(tag)
            if ret != "":
                ret += ",\n"
            ret += "    "
            ret += field if (tag in tags) and condition else "0"
        assert completed_tags == tags
        return ret

    def _print_statistics(self):
        """ Prints BLOB statistics """
        header_footer_size = self._bf.header_size + self._bf.footer_size

        valid_channels_size = 0
        for channel_type in ChannelType.all():
            if (channel_type.bandwidth != Bandwidth._20) and (not self._bf.high_bw_combs):
                continue
            combs = list(self._data.used_channel_combs[channel_type])
            valid_channels_size += ClmBlob.CHAN_COMB_FOOTER_SIZE + ClmBlob.CHAN_COMB_SIZE * len(combs)

        channel_ranges_size = 0
        for bw in Bandwidth.all() if self._bf.per_bw_rs else [None]:
            channel_ranges_size += len(self._data.channel_ranges_per_bw[bw]) * ClmBlob.CHAN_RANGE_SIZE

        restricted_sets_size = 0
        self._verbose_print("%d restricted sets included:" % len(self._data.restricted_sets))
        for rs in self._data.restricted_sets:
            self._verbose_print(rs.name)
            record_len = 1 if rs.is_all_channels else len(rs.channels)
            restricted_sets_size += ClmBlob.RESTR_SET_HDR_SIZE + record_len

        locale_channels_size = 0
        for channel_set in self._data.locale_channel_sets:
            locale_channels_size += ClmBlob.CHAN_SET_HDR_SIZE + len(channel_set)

        rate_sets_size = 0
        if self._direct_rates_mapping is not None:
            for ri in range(RatesInfo.NUM_RATES):
                if not RatesInfo.get_rate(ri).singular:
                    break
                rate_sets_size += ClmBlob.RATE_SET_HDR_SIZE + 1
        else:
            for ext_rates in self._data.rate_sets_per_bw_band:
                for bw in Bandwidth.all() if self._bf.per_bw_rs else [None]:
                    if bw not in self._data.rate_sets_per_bw_band[ext_rates]:
                        continue
                    for band in Band.all() if self._bf.per_band_rate_sets else [None]:
                        for rate_set in self._data.rate_sets_per_bw_band[ext_rates][bw][band]:
                            rate_sets_size += ClmBlob.RATE_SET_HDR_SIZE + len(rate_set)
        locales_size = 0
        locale_ids = {}
        for loc_type in LocaleType.all():
            if loc_type.flavor == LocaleType.HT3:
                continue
            locale_ids[loc_type] = []
        for locale in self._data.locales:
            if locale.locale_type.is_base:
                locales_size += ClmBlob.BASE_LOC_HDR_SIZE
                locales_size += ClmBlob.LOC_PUB_PWR_HDR_SIZE + len(locale.reg_power) * ClmBlob.LOC_PUB_PWR_REC_SIZE
            power_type_dict = self._build_power_type_dict(locale)
            if power_type_dict:
                for power_type, power_records in power_type_dict.items():
                    power_type_hdr_size = ClmBlob.LOC_CHAN_PWR_HDR2_SIZE if power_type.use_flag2() else ClmBlob.LOC_CHAN_PWR_HDR_SIZE
                    if self._direct_rates_mapping is not None:
                        for _, _, rate_set in power_records:
                            locales_size += power_type_hdr_size + len(rate_set) * (ClmBlob.LOC_CHAN_PWR_REC_SIZE + power_type.extra_powers)
                    else:
                        locales_size += power_type_hdr_size + len(power_records) * (ClmBlob.LOC_CHAN_PWR_REC_SIZE + power_type.extra_powers)
            else:
                locales_size += ClmBlob.LOC_CHAN_PWR_HDR_SIZE
            locale_ids[locale.locale_type].append(locale.loc_id)
        for loc_type in sorted(locale_ids.iterkeys(), cmp=LocaleType.compare):
            self._verbose_print("\n%d %s locales included:" % (len(locale_ids[loc_type]), str(loc_type)))
            for locale_id in locale_ids[loc_type]:
                locale = self._data.get_locale(locale_id)
                self._verbose_print("%-6s (%s)" % (locale.get_unique_name(), locale.target))

        sub_chan_rules_size = 0
        self._verbose_print("\n%d subchannel rule sets included:" % len(self._data.sub_chan_rules))

        for band in Band.all():
            for bandwidth, chan_rec_size, chan_rec_inc_size in \
                [(Bandwidth._40, ClmBlob.SUB_CHAN_CHAN_SIZE_40, ClmBlob.SUB_CHAN_CHAN_SIZE_40_INC),
                 (Bandwidth._80, ClmBlob.SUB_CHAN_CHAN_SIZE_80, ClmBlob.SUB_CHAN_CHAN_SIZE_80_INC),
                 (Bandwidth._160, ClmBlob.SUB_CHAN_CHAN_SIZE_160, ClmBlob.SUB_CHAN_CHAN_SIZE_160_INC)]:
                if bandwidth not in ChannelType.bandwidths_of_band(band):
                    continue
                channel_type = ChannelType.create(band, bandwidth)
                if not self._data.has_sub_chan_rules_for(channel_type):
                    continue
                rec_size = chan_rec_inc_size if self._sub_chan_inc_fmt == ClmBlob.SUB_CHAN_INC_FMT_COUPLED else chan_rec_size
                region_rec_size = ClmBlob.SUB_CHAN_REG_SIZE_INC if self._sub_chan_inc_fmt == ClmBlob.SUB_CHAN_INC_FMT_SEPARATE else ClmBlob.SUB_CHAN_REG_SIZE
                sub_chan_rules_size += ClmBlob.SUB_CHAN_FOOTER_SIZE
                for region_rules in self._data.sub_chan_rules:
                    num_chan_rules = region_rules.get_num_channel_type_rules(channel_type)
                    if num_chan_rules:
                        self._verbose_print("%s: %sGHz, %sMHz" % (region_rules.name, Band.name[band], Bandwidth.name[bandwidth]))
                    sub_chan_rules_size += region_rec_size + num_chan_rules * rec_size
                    if self._sub_chan_inc_fmt == ClmBlob.SUB_CHAN_INC_FMT_SEPARATE:
                        sub_chan_rules_size += len(self._region_sc_increments[channel_type].get(region_rules.name, []))

        regions_size = ClmBlob.REG_FOOTER_SIZE + len(self._data.regions) * self._reg_rec_info.struct_size
        self._verbose_print("\n%d regions included:" % len(self._data.regions))
        for region in self._data.regions:
            if self._data.is_incremental:
                self._verbose_print("%-5s (%s). Inclusion reason: %s" %
                                    (str(region.ccrev), region.country_name, region.diff_report.get_diff_report("Absent in base data")))
            else:
                self._verbose_print("%-5s (%s)" % (str(region.ccrev), region.country_name))
        if self._data.deleted_regions:
            deleted_regions_size = len(self._data.deleted_regions) * self._reg_rec_info.struct_size
            self._verbose_print("\n%d regions excluded:" % len(self._data.deleted_regions))
            for ccrev, diff_report in self._data.deleted_regions:
                self._verbose_print("%s. Exclusion reason: %s" %
                                    (str(ccrev), diff_report.get_diff_report("Absent in final CLM file")))
        else:
            deleted_regions_size = 0
        if self._data.preserved_regions:
            self._verbose_print("\n%d unchanged regions not excluded:" % len(self._data.preserved_regions))
            for ccrev in self._data.preserved_regions:
                self._verbose_print(str(ccrev))

        extra_ccrevs_size = 0
        if self._bf.indexed_ccrev:
            extra_ccrevs_size = \
                ClmBlob.EXTRA_CCREV_FOOTER_SIZE + \
                len(self._data.extra_ccrevs) * \
                self._size_selector(rev8=ClmBlob.EXTRA_CCREV_8_SIZE, rev16=ClmBlob.EXTRA_CCREV_16_SIZE)

        adcs = self._data.adcs
        self._verbose_print("\n%d aliases included:" % len(adcs))
        aliases_size = ClmBlob.ADC_FOOTER_SIZE
        for adc in adcs:
            self._verbose_print(adc.cc)
            aliases_size += \
                ClmBlob.ADC_SIZE + \
                len(adc.regions) * \
                self._size_selector(nonlegacy_if=self._bf.indexed_ccrev, legacy=ClmBlob.ADC_REC_SIZE,
                                    reg8=ClmBlob.ADC_IDX_8_REC_SIZE, reg16=ClmBlob.ADC_IDX_16_REC_SIZE)
        if self._data.deleted_adcs:
            aliases_size += ClmBlob.ADC_SIZE * len(self._data.deleted_adcs)
            self._verbose_print("\n%d aliases excluded:" % len(self._data.deleted_adcs))
            for cc, diff_report in self._data.deleted_adcs:
                self._verbose_print("%s. Exclusion reason: %s" %
                                    (cc, diff_report.get_diff_report("Absent in final CLM file")))
        if self._data.preserved_adcs:
            self._verbose_print("\n%d unchanged aliases not excluded:" % len(self._data.preserved_adcs))
            for cc in self._data.preserved_adcs:
                self._verbose_print(cc)

        aggregations_size = 0
        self._verbose_print("\n%d aggregations included:" % len(self._data.aggregations))
        mapping_size = self._size_selector(nonlegacy_if=self._bf.indexed_ccrev, legacy=ClmBlob.AGG_MAP_SIZE,
                                           reg8=ClmBlob.AGG_MAP_IDX_8_SIZE, reg16=ClmBlob.AGG_MAP_IDX_16_SIZE)
        for aggregation in self._data.aggregations:
            aggregation_size = len(aggregation.mappings) * mapping_size
            aggregations_size += aggregation_size
            if self._data.is_incremental:
                self._verbose_print("%-5s (%s). Inclusion_reason: %s" %
                                    (str(aggregation.ccrev), aggregation.note, aggregation.diff_report.get_diff_report("Absent in base data")))
            else:
                self._verbose_print("%-5s (%s). size=%d, len=%d" % (str(aggregation.ccrev), aggregation.note, aggregation_size, len(aggregation.mappings)))
        aggregations_size += ClmBlob.AGG_FOOTER_SIZE + len(self._data.aggregations) * ClmBlob.AGG_SIZE
        if self._data.deleted_aggregations:
            deleted_aggregations_size = ClmBlob.AGG_SIZE * len(self._data.deleted_aggregations)
            self._verbose_print("\n%d aggregations excluded:" % len(self._data.deleted_aggregations))
            for ccrev, diff_report in self._data.deleted_aggregations:
                self._verbose_print(str(ccrev))
        else:
            deleted_aggregations_size = 0

        regrev_remap_size = 0
        if self._regrev_remap and not self._data.regrev_remap_as_base:
            regrev_remap_size = ClmBlob.REGREV_REMAP_FOOTER_SIZE + (len(self._data.regrev_remap_table) + 1) * ClmBlob.REGREV_CC_REMAP_REC_SIZE + \
                sum(map(lambda cc_remap: len(cc_remap), self._data.regrev_remap_table.values())) * ClmBlob.REGREV_REMAP_REC_SIZE

        user_string_size = 0 if not self._bf.has_user_string or (self._options.user_string is None) else (len(self._options.user_string) + 1)

        self._verbose_print("\nHeader and footer size: %d bytes" % header_footer_size)
        self._verbose_print("Channel range table size: %d bytes" % channel_ranges_size)
        self._verbose_print("Valid channels table size: %d bytes" % valid_channels_size)
        self._verbose_print("Restricted sets table size: %d bytes" % restricted_sets_size)
        self._verbose_print("Locale channels table size: %d bytes" % locale_channels_size)
        self._verbose_print("Rate sets table size: %d bytes" % rate_sets_size)
        self._verbose_print("Locales table size: %d bytes" % locales_size)
        self._verbose_print("Subchannel rules table size: %d bytes" % sub_chan_rules_size)
        self._verbose_print("Regions table size: %d bytes" % regions_size)
        if deleted_regions_size:
            self._verbose_print("Deleted regions table size: %d bytes" % deleted_regions_size)
        self._verbose_print("Extra ccrevs table size: %d bytes" % extra_ccrevs_size)
        self._verbose_print("Aliases table size: %d bytes" % aliases_size)
        self._verbose_print("Aggregations table size: %d bytes" % aggregations_size)
        if deleted_aggregations_size:
            self._verbose_print("Deleted aggregations table size: %d bytes" % deleted_aggregations_size)
        if self._regrev_remap:
            self._verbose_print("Regrev remap table size: %d bytes" % regrev_remap_size)
        self._verbose_print("User string size: %d bytes" % user_string_size)
        print "Total size (approximately): %d bytes" \
            % (header_footer_size + valid_channels_size +
               channel_ranges_size + restricted_sets_size +
               locale_channels_size + rate_sets_size + locales_size +
               sub_chan_rules_size + regions_size + deleted_regions_size +
               extra_ccrevs_size + aliases_size + aggregations_size +
               deleted_aggregations_size + regrev_remap_size + user_string_size)
        for loc_type in sorted(locale_ids.iterkeys(), cmp=LocaleType.compare):
            self._verbose_print("Number of %s locales: %d" % (str(loc_type), len(locale_ids[loc_type])))
        self._verbose_print("Number of regions: %d" % len(self._data.regions))
        self._verbose_print("Number of aggregations: %d" % len(self._data.aggregations))
        self._verbose_print("Length of region record: %d bytes" % self._reg_rec_info.struct_size)
        self._verbose_print("Length of aggregation mapping: %d bytes" % mapping_size)

    def _range_name(self, cr):
        """ Returns BLOB name for given channel range """
        if cr.is_80_80:
            return "CHANNEL_%sG_%sM_%s" % (Band.name[cr.channel_type.band][0],
                                           Bandwidth.id_name[cr.channel_type.bandwidth],
                                           cr.start_str)
        return "RANGE_%sG_%sM_%s_%s" % (Band.name[cr.channel_type.band][0],
                                        Bandwidth.id_name[cr.channel_type.bandwidth],
                                        cr.start_str, cr.end_str)

    def _restricted_set_name(self, rs_name):
        """ Returns BLOB name for given restricted set """
        return "RESTRICTED_SET_NONE" if (rs_name is None) else ("RESTRICTED_SET_" + rs_name)

    def _locale_idx_name(self, locale):
        """ Returns full name of given locale """
        return "LOCALE_%sG_%sIDX_%s" \
            % (Band.name[locale.locale_type.band][0],
               "" if locale.locale_type.is_base else "HT_",
               locale.blob_alias)

    def _reg_loc_idx_name(self, region, index):
        """ Returns short name for locale at given index in region """
        if index not in region.locale_ids:
            return "NONE"
        if index in region.same_locales:
            return "SAME"
        return region.get_locale(index).blob_alias

    def _ccrev_idx_name(self, ccrev):
        """ Returns CC/rev index name """
        return "CCREV_%s" % self._data.ccrev_aliases[ccrev]

    def _ccrev_ref(self, ccrev, force_struct=False):
        """ Returns string that represent reference to given ccrev

        Arguments:
        ccrev        -- CC/rev to reference
        force_struct -- Use struct representation ({"CC", rev}) even though
                        self._bf.indexed_ccrev is True
        Returns Enum index if self._bf.indexed_ccrev and not force_struct,
        otherwise struct representation
        """
        if self._bf.indexed_ccrev and not force_struct:
            return self._ccrev_idx_name(ccrev)
        return "{\"%s\", %4s}" % \
               (ccrev.cc,
                ("%s%s" % ("" if self._rev16 else "0xFF & ", "CLM_DELETED_MAPPING")) if ccrev.deleted else self._rev_str(ccrev))

    def _rev_str(self, ccrev):
        """ Returns string representation of rev part of given CC/rev """
        return ("%d /* %d */" % (self._data.remap_rev16_to_rev8(ccrev), ccrev.rev)) if self._regrev_remap else str(ccrev.rev)

    def _data_name_prefix(self):
        """ Returns prefix (common part) of BLOB data name """
        return "clm_inc" if self._data.is_incremental else "clm"

    def _format_name(self, name, prefix=False):
        """ Mangles top-level data name.

        Arguments:
        name   -- Tail part of name
        prefix -- True means that clm_/clm_inc_ prefix shall be added to name
        """
        if prefix:
            if self._data.is_incremental:
                name = "clm_inc_" + name
            else:
                name = "clm_" + name

        if self._options.force_section is not None:
            name = "__attribute__ ((__section__ (\"%s\"))) %s" % (self._options.force_section, name)
        elif self._data.is_incremental:
            name = "CLMATTACHDATA(" + name + ")"
        return name

    def _update_flags(self, test, flag_string, prev_flags):
        """ Returns flags' string value conditionally OR-ed with given flag """
        if not test:
            return prev_flags
        if prev_flags:
            prev_flags = prev_flags + " | "
        prev_flags = prev_flags + flag_string
        return prev_flags

    def _value_limit(self, num_bits, special_values):
        """ Returns limit on number of values encodable in bit field

        Arguments:
        num_bits        -- Number of bits in field
        special_indices -- Number of special (reserved) values
        Returns Limit on number of values
        """
        return (1 << num_bits) - special_values

    def _check_limit(self, num, name, num_bits, special_values, is_index, no_warning=False):
        """ Computes limit with _value_limit() and checks given value against it

        If value exceeds limit - prints error message. If value exceeds 90%
        of limit - prints warning

        Arguments:
        num            -- Number to check against limit
        name           -- Value name to use in error messages
        num_bits       -- Argument to pass to _value_limit()
        special_values -- Argument to pass to _value_limit()
        is_index       -- True if 'num' is index, false if it it is number of
                          values
        no_warning     -- True means that 90% warning is not printed
        """
        limit = self._value_limit(num_bits, special_values)
        if is_index:
            limit -= 1
        if num > limit:
            ClmUtils.error("%s is %d which exceeds limit of %d" % (name, num, limit))
        if (num > 0.9 * limit) and (not no_warning):
            ClmUtils.warning("%s is %d which exceeds 90%% of limit of %d" %
                             (name, num, limit))

    def _sub_chan_region_rules_idx(self, name):
        """ Returns 1-based index of subchannel rules with given name """
        sub_chan_rules = self._data.sub_chan_rules
        for i in range(len(sub_chan_rules)):
            if sub_chan_rules[i].name == name:
                return i + 1
        raise ValueError("Invalid subchannel rules name " + name)

    def _size_selector(self, nonlegacy_if=True, legacy=0, rev8=0, rev16=0,
                       reg8=0, reg16=0):
        """ Returns size selected from arguments

        Arguments:
        nonlegacy_if -- False means that `legacy` parameter (which must be
                        nonzero) shall be returned
        legacy       -- Value to return if `nonlegacy_if` is False
        rev8         -- Returned if nonzero and `nonlegacy_if` is true and
                        8-bit revs (in ccrevs) are used
        rev16        -- Returned if nonzero and `nonlegacy_if` is true and
                        16-bit revs (in ccrevs) are used
        reg8         -- Returned if nonzero and `nonlegacy_if` is true and
                        8-bit ccrev indices are used
        reg16        -- Returned if nonzero and `nonlegacy_if` is true and
                        16-bit ccrev indices are used
        """
        if not nonlegacy_if:
            if not legacy:
                raise ValueError("'legacy' parameter not specified")
            return legacy
        if rev8 and not self._rev16:
            return rev8
        if rev16 and self._rev16:
            return rev16
        if reg8 and not self._reg_idx_16:
            return reg8
        if reg16 and self._reg_idx_16:
            return reg16
        raise ValueError("Incomplete parameters")

    def _write_blob(self, s):
        """ If BLOB output is enabled - writes given string to BLOB """
        if self._f:
            self._f.write(s)

    def _verbose_print(self, s):
        """ If verbose printing is enabled - prints given string """
        if self._verbose_statistics:
            print s

    @staticmethod
    def get_full_generator_name(clm_data):
        """ Returns generator name as stored in BLOB

        Arguments:
        clm_data -- ClmData object
        Returns generator name, composed of tool name and version
        """
        if clm_data.generator_name:
            if clm_data.generator_version:
                return clm_data.generator_name + ": " + clm_data.generator_version
            else:
                return clm_data.generator_name + " version unknown"
        return "tool unknown"

# =============================================================================


class ApitestWriter:
    """ Apitest listing writer """

    def __init__(self, clm_data_container, options):
        try:
            self._data = clm_data_container.get_data()
            self._base_data = clm_data_container.get_base_data()
            self._f = ClmUtils.open_for_write(options.apitest_listing_name)
            self._options = options
            self._bf = clm_data_container.filter_params.blob_format
            self._write_versions()
            self._write_regions()
            self._write_aggregations()
        except IOError:
            ClmUtils.error("Error writing apitest listing file \"%s\": %s" % (options.apitest_listing_name, ClmUtils.exception_msg()))

    def _write_versions(self):
        """ Writes BLOB version information """
        if self._bf.apps_version:
            apps_version = self._data.get_apps_version(verbatim=False)
            base_apps_version = self._base_data.get_apps_version(verbatim=False) if self._base_data is not None else None
            for name, app_version in [("Base", base_apps_version if self._base_data is not None else apps_version),
                                      ("Incremental", apps_version if self._base_data is not None else None)]:
                self._write("%s app version string: %s\n" % (name, app_version if app_version is not None else ""))
        if self._bf.has_user_string:
            for name, base_value, effective_value in [
                    ("CLM data version", self._base_data.clm_version if self._base_data is not None else None, self._data.clm_version),
                    ("ClmCompiler version", _PROGRAM_VERSION if self._base_data is not None else None, _PROGRAM_VERSION),
                    ("XML Generator", ClmBlob.get_full_generator_name(self._base_data) if self._base_data is not None else None, ClmBlob.get_full_generator_name(self._data)),
                    ("CLM data apps version", self._base_data.get_apps_version(verbatim=False) if (self._base_data is not None) and self._bf.apps_version else None, self._data.get_apps_version(verbatim=False) if self._bf.apps_version else None),
                    ("User string", self._options.base_user_string if self._base_data is not None else None, self._options.user_string)]:
                if base_value is None:
                    base_value = ""
                if effective_value is None:
                    effective_value = ""
                self._write("%s (base/inc/effective): %s/%s/%s\n" %
                            (name,
                             base_value if self._base_data is not None else effective_value,
                             effective_value if self._base_data is not None else "",
                             effective_value))

    def _write_regions(self):
        adc_dict = self._data.get_adc_dict()
        for reg_idx in range(len(self._data.regions)):
            region = self._data.regions[reg_idx]
            if self._options.verbose:
                sys.stdout.write("Writing country %s (%d of %d)    \r" % (region.ccrev, reg_idx + 1, len(self._data.regions)))
            self._write("Country %s. " % region.ccrev)
            adv_cc = adc_dict.get(region.ccrev)
            self._write("Adv. cc: %s. " % ((adv_cc if adv_cc else region.ccrev.cc)))
            for band in Band.all():
                self._write("DFS for %sG: " % Band.name[band])
                loc = region.get_locale(LocaleType(LocaleType.BASE, band))
                if loc is not None:
                    self._write("%s. " % Dfs.name[loc.dfs])
                else:
                    self._write("None. ")
            self._write("\n")
            channels_dict = self._data.get_region_channels_dict(region)
            for band in Band.all():
                no_40mhz = True
                no_80mhz = True
                no_80_80mhz = True
                no_160mhz = True
                no_mimo = True
                has_dsss_eirp = False
                has_ofdm_eirp = False
                per_antenna = False
                locale = region.get_locale(LocaleType(LocaleType.BASE, band))
                filt_war1 = (locale is not None) and locale.filt_war1
                for ct in ChannelType.all():
                    if(ct.band != band) or (ct not in channels_dict):
                        continue
                    for channel in channels_dict[ct]:
                        for rate, lp in self._data.get_rate_power_dict(region, channel, ct, extended=True,
                                                                       trim_ht3_to_ht=self._bf.trim_ht3_to_ht).items():
                            rt = re.match(r"^([A-Z]+)", rate.name).group(1)
                            disabled = True
                            for power in lp[1].values():
                                if not power.is_disabled:
                                    disabled = False
                                    if len(power.powers_dbm) > 1:
                                        per_antenna = True
                            if disabled:
                                continue
                            if ct.bandwidth == Bandwidth._40:
                                no_40mhz = False
                            elif ct.bandwidth == Bandwidth._80:
                                if ChannelType.is_80_80(channel, ct):
                                    no_80_80mhz = False
                                else:
                                    no_80mhz = False
                            elif ct.bandwidth == Bandwidth._160:
                                no_160mhz = False
                            if rt == "MCS":
                                no_mimo = False
                            if Measure.EIRP in lp[1]:
                                if rt == "DSSS":
                                    has_dsss_eirp = True
                                if rt == "OFDM":
                                    has_ofdm_eirp = True
                self._write("    Debug flags for %sG: " % Band.name[band])
                if no_40mhz:
                    self._write("NO_40MHZ, ")
                if no_80mhz and (self._bf.has_no_80mhz):
                    self._write("NO_80MHZ, ")
                if no_80_80mhz and (self._bf.has_80_80):
                    self._write("NO_80_80MHZ, ")
                if no_160mhz and (self._bf.max_bandwidth >= Bandwidth._160):
                    self._write("NO_160MHZ, ")
                if no_mimo:
                    self._write("NO_MIMO, ")
                if has_dsss_eirp:
                    self._write("HAS_DSSS_EIRP, ")
                if has_ofdm_eirp:
                    self._write("HAS_OFDM_EIRP, ")
                if filt_war1:
                    self._write("FILT_WAR1, ")
                if region.loc_reg_caps.txbf and self._bf.loc_reg_caps.txbf:
                    self._write("TXBF, ")
                if region.default_for_cc and self._bf.has_default_for_cc:
                    self._write("DEFAULT_FOR_CC, ")
                if per_antenna:
                    self._write("PER_ANTENNA, ")
                if (region.edcrs != Edcrs.DEFAULT) and (region.edcrs in self._bf.supported_edcrs):
                    self._write("EDCRS_%s, " % Edcrs.name[region.edcrs])
                if region.lo_gain_nbcal and self._bf.has_lo_gain_nbcal:
                    self._write("LO_GAIN_NBCAL, ")
                if region.chsprwar2:
                    self._write("CHSPR_WAR2, ")
                self._write("\n")
            for band in Band.all():
                self._write("    Restricted channels for %sG: " % Band.name[band])
                loc = region.get_locale(LocaleType(LocaleType.BASE, band))
                if (loc is not None) and (loc.restricted_set_name is not None):
                    rs = self._data.get_restricted_set(loc.restricted_set_name)
                    if rs.is_all_channels:
                        restricted_channels = self._data.get_channel_list(loc.channel_sets[Bandwidth.default])
                    else:
                        restricted_channels = self._data.get_channel_list(rs.channels)
                    restricted_channels.sort()
                    locale_channels = self._data.get_channel_list(loc.channel_sets[Bandwidth.default])
                    for channel in restricted_channels:
                        if channel in locale_channels:
                            self._write("%d, " % channel)
                self._write("\n")
            for channel_type in ChannelType.all():
                if channel_type not in channels_dict:
                    continue
                for channel in channels_dict[channel_type]:
                    for sub_chan_id in [None] + SubChanId.valid_sub_chan_ids(channel_type.bandwidth):
                        if (sub_chan_id is not None) and (channel_type.bandwidth > Bandwidth._40) and \
                           (not self._bf.sub_chan_rules):
                            continue
                        reg_power = None
                        if channel_type.bandwidth == Bandwidth.default:
                            loc = region.get_locale(LocaleType(LocaleType.BASE, channel_type.band))
                            if loc is not None:
                                for p in loc.reg_power:
                                    if self._data.valid_channels.is_channel_in_range(channel, p.channel_range):
                                        reg_power = p
                                        break
                        rate_power_dict = self._data.get_rate_power_dict(region, channel, channel_type, extended=True, sub_chan_id=sub_chan_id)
                        if not reg_power:
                            # Looking for defined power. Wouldn't print line
                            # if not found
                            has_enabled_powers = False
                            for rate, locale_power in rate_power_dict.items():
                                for measure, power in locale_power[1].items():
                                    if not power.is_disabled:
                                        has_enabled_powers = True
                                        break
                                if has_enabled_powers:
                                    break
                            if not has_enabled_powers:
                                if sub_chan_id is None:
                                    break
                                else:
                                    force_coverage = "ZZZ"
                                    continue
                        self._write("    Channel %s%s(%sG), " %
                                    (("%%%ds" % (8 if ChannelType.is_80_80(channel, channel_type) else 3)) % ChannelType.channel_to_string(channel, channel_type),
                                     ("%%-%ds" % max([0] + list(map(lambda x: len(SubChanId.name[x]), SubChanId.valid_sub_chan_ids(channel_type.bandwidth))))) %
                                     ("" if sub_chan_id is None else SubChanId.name[sub_chan_id]),
                                     Band.name[channel_type.band]))
                        self._write(("%sM" if (channel_type.bandwidth < Bandwidth._160) else "%3sM") %
                                    Bandwidth.name[SubChanId.sub_chan_bandwidth(channel_type.bandwidth, sub_chan_id) if sub_chan_id is not None else channel_type.bandwidth])
                        if reg_power:
                            self._write(" (Reg %d)" % reg_power.powers_dbm[0])
                        self._write(": ")
                        if rate_power_dict:
                            rate_power_lists = {Measure.CONDUCTED: [], Measure.EIRP: []}
                            for rate, locale_power in rate_power_dict.items():
                                for measure, power in locale_power[1].items():
                                    if not power.is_disabled:
                                        rate_power_lists[measure].append((rate, power.powers_dbm))
                            for measure in rate_power_lists:
                                mn = Measure.name[measure][:1].upper()
                                rpl = rate_power_lists[measure]
                                rpl.sort(key=lambda rp: rp[0].index)
                                r0 = 0
                                for r in range(len(rpl)):
                                    if ((r + 1) == len(rpl)) or \
                                       (rpl[r + 1][0].index != (rpl[r][0].index + 1)) or \
                                       (rpl[r + 1][1] != rpl[r][1]) or \
                                       (rpl[r + 1][0].group != rpl[r][0].group):
                                        powers = "/".join("%.2f" % dbm for dbm in rpl[r0][1][:rpl[r0][0].chains + 1])
                                        if r == r0:
                                            self._write("%s:%s%s, " % (rpl[r0][0].name, powers, mn))
                                        else:
                                            self._write("(%s-%s):%s%s, " % (rpl[r0][0].name, rpl[r][0].name, powers, mn))
                                        r0 = r + 1
                        self._write("\n")
        if self._options.verbose:
            print "                                        "

    def _write_aggregations(self):
        for aggregation in self._data.aggregations:
            self._write("Aggregation %s\n    Countries: " % aggregation.ccrev)
            mappings = list(aggregation.mappings)
            mappings.sort(key=methodcaller("sort_key"))
            for ccrev in mappings:
                if (ccrev.rev != 0) and (ccrev != aggregation.ccrev):
                    self._write("%s, " % ccrev)
            self._write("\n")

    def _write(self, s):
        self._f.write(s)


class BcmWifiRatesCaps:
    """ Limitations on rates, posed by bcmwifi_rates.h

    Attributes:
    max_chains   -- Maximum chain count
    vht          -- Support of VHT rates
    txbf         -- support of TXBF rates
    has_1024_qam -- Support of 1024QAM (VHT10-11) rates
    txbf0        -- Support of TXBF0 rates
    """
    def __init__(self, bcmwifi_include_dir=None):
        """ Constructor

        Arguments:
        bcmwifi_include_dir -- None or directory that contains bcmwifi_include.h
        """
        if bcmwifi_include_dir is not None:
            try:
                filename = os.path.join(bcmwifi_include_dir, "bcmwifi_rates.h")
                with open(filename) as f:
                    contents = f.read()
            except:
                ClmUtils.error("Can't read %s. Probably bcmwifi include directory specified incorrectly" % filename)
            try:
                max_chains = max(map(lambda m: int(m.group(1)),
                                     re.finditer(r"WL_RATE_\dX(\d)_", contents)))
            except:
                ClmUtils.error("Rate definitions not found in %s" % filename)
        self.max_chains = max_chains if bcmwifi_include_dir else RatesInfo.get_max_chains()
        self.vht = (not bcmwifi_include_dir) or ("VHT8" in contents)
        self.txbf = (not bcmwifi_include_dir) or ("_TXBF_" in contents)
        self.txbf0 = (not bcmwifi_include_dir) or ("2X2_TXBF" in contents)
        self.has_1024_qam = (not bcmwifi_include_dir) or ("VHT10" in contents)


###############################################################################
# MODULE STATIC VARIABLES
###############################################################################

# Help message
_usage = """Usage:
./ClmCompiler.py [options] source.xml [result.c]

Options are:
--ccrev cc/rev         Region(s) and/or aggregation(s) to include to BLOB. May
                       be specified multiple times, also may be specified as
                       set of space-separated values in quotes (like
                       `--ccrev "US/0 EU/9"`). Special values: `--ccrev all`
                       includes all  regions, `--ccrev all/0` includes all
                       default regions, `--ccrev CC/all` includes all regions
                       of country CC
--region cc/rev        Similar to --ccrev but only selects regions
--agg cc/rev           Similar to --ccrev but only selects aggregations
--include tag_group    The tag group selects regions to include, and has the
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
--full_set             Include CC/0 regions for all countries except for those
                       included to all aggregations
--max_chains n         Limits TX power data to not more than this number of TX
                       chains
--max_bandwidth max_bw Maximum bandwidth in MHz: 20, 40, 80, 80+80, 160
--band band            Limits locale TX power data to given band (possible
                       values: 2.4, 5)
--abg                  Limits Tx power data to 802.11a/b/g rates
--abgn                 Limits Tx power data to 802.11a/b/g/n rates
--txbf                 Include TXBF (beamforming) rates in Tx power data
--dfs_tw               Enables TW DFS. Default is treat TW DFS as US DFS
--1024_qam             Include 1024 QAM (VHT10-11) rates
--vht_txbf0            Include VHT TXBF0 rates
--no_per_antenna       Disables per-antenna limits
--all_data             Puts all CLM data to BLOB (equivalent of evergrowing
                       list of "--ccrev all --txbf ..."). For debug only
--regrev_remap         Support 16-bit regrevs by means of regrev remap. Used
                       only with certain old ClmAPI. Produces BLOB incompatible
                       with modern ClmAPI
--scr_idx4             Use 4-bit subchannel rules index. Ignored for BLOB
                       formats that natively support 4 or wider bits index
--scr_idx8             Use 8-bit subchannel rules index. Ignored for BLOB
                       formats that natively support 8 or wider bits index
--scr_40mhz            Support 40MHz subchannel rules. Ignored for BLOB formats
                       that natively support 40MHz subchannel rules
--scr_inc              Support subchannel rules' power increments (offsets) in
                       older 'coupled' format. Ignored for
                       BLOB formats that natively support power increments
--scr_inc2             Support subchannel rules' power increment in newer
                       'separate' format. Ignored for BLOB formats that
                       natively support power increments
--loc_idx12            Use 12-bit locale indices. Ignored for BLOB formats
                       that natively support 12-bit locale indices
--inc_from base.xml    Create an incremental BLOB relative to given CLM file
--compare log          To given file writes detailed comparison log between
                       base and final XML data. Requires --inc_from switch
--ignore_missed_items  Does not terminate with error if aggregation uses
                       undefined regions or region uses undefined locales
--list switch          Prints possible filtering values:
                       '--list region' - prints list of possible CC/revs. If
                       --ccrev, --include, --agg, --region specified - prints
                       matching CC/revs
                       '--list tag' - prints list of available tags
--verbose              Prints detailed listing of what was included to BLOB
--noobfuscate          Puts to generated BLOB comments on locales/regions
                       customers and use
--obfuscate            Obfuscates comments in generated BLOB. This option is
                       obsolete, as comments are now obfuscated by default
--apitest listing_file Generates listing apitest output shall match with
                       Listing generated to specified file
--blob_format version  Format version of generated blob. Allowed values:
""" + "\n".join([textwrap.fill("%s: %s" % (vi.version_string, vi.comment),
                               width=79, initial_indent="                        ",
                               subsequent_indent="                           ")
                 for vi in BlobFormatVersionInfo.get_supported_versions().values()]) + """
--clmapi_include_dir dir Specifies directory where wlc_clm_data.h located.
                       From there ClmCompiler reads maximum supported blob
                       format and uses it as default
--bcmwifi_include_dir dir Specifies directory where bcmwifi_rates.h located.
                       From there ClmCompiler reads maximum supported number
                       of TX chains ands uses it as default
--config_file filename Load additional command line options from the named
                       file. All nonalphanumeric options (e.g. containing
                       spaces, '#', *, etc.) shall be enclosed in quotes.
                       Comments prefixed with '#' are allowed
--base_config_file filename Used in conjunction with the --inc_from option.
                       The specified file should contain the exact flags used
                       to generate the original (ROM) BLOB. Updated flags
                       should be specified directly on the command line or via
                       a --config_file option, and the resulting incremental
                       BLOB will represent the delta from the old data as
                       filtered by the original flags to the new data as
                       filtered by the updated flags
--user user_str        Puts user-defined string to put to BLOB. If not
                       specified date in "YYYY-MM-DD HH:MM:SS" format is used
--base_user base_user_str Sets base BLOB user string to use by --apitest
                       functionality. Doesn't affect BLOB generation
--force_section section Forces placing of all data to given section. Uses GCC
                       syntax for this
--version              Prints version information: version of CLM data and CLM
                       format (if CLM file specified), name and version of
                       utility that generated CLM file, version of generated
                       BLOB format
--print_options        Prints all options, specified directly in config files
--help                 Print this help message
source.xml             Source CLM XML file name
result.c               Output BLOB C source code file

Options may also be set by means of """ + _ENV_OPTIONS + """ environment variable.
Options set this way shall not contradict those set in command line and config
files. Might be useful for options like --verbose, --print_options, etc.
"""

###############################################################################
# Module top-level static functions
###############################################################################


def string_to_argv(s):
    """ Converts string of options to list of them """
    lexer = shlex.shlex(s, posix=True)
    lexer.wordchars += "-/.+"
    return list(lexer)

# =============================================================================


def parse_args(argv, clm_data_container, filter_params, options, name=""):
    """ Retrieves command line arguments from given argument list

    Arguments:
    argv               -- Command line arguments (as list and/or
                          space-separated)
    clm_data_container -- CLM data container
    filter_params      -- Filter parameters object to put filtering parameters
                          to
    options            -- Container for nonfiltering command line parameters
    name               -- Name of configuration file where from arguments were
                     taken (empty if from command line)
    Returns List of positional arguments
    """
    try:
        opts, args = \
            getopt.getopt(
                argv, "h?",
                ["region=", "agg=", "ccrev=", "include=", "full_set",
                 "max_chains=", "max_bandwidth=", "band=", "abg", "abgn",
                 "txbf", "no_per_antenna", "dfs_tw", "ulb", "ulb_", "1024_qam",
                 "vht_txbf0", "all_data", "ignore_missed_items", "inc_from=",
                 "list=", "verbose", "apitest=", "config_file=",
                 "base_config_file=", "regrev_remap", "scr_idx4", "scr_idx8",
                 "scr_40mhz", "scr_inc", "scr_inc2", "loc_idx12", "compare=",
                 "blob_format=", "clmapi_include_dir=", "bcmwifi_include_dir=",
                 "user=", "base_user=", "version", "noobfuscate", "obfuscate",
                 "print_options", "help", "force_section=",
                 "unsupportedswitchfortestpurposes"])
    except getopt.GetoptError:
        print _usage
        ClmUtils.error(ClmUtils.exception_msg())

    # Don't memorize BLOB name in command line, if this name is specified
    filter_params.add_command_line(name, " ".join((('"%s"' % a) if ' ' in a else a) for a in argv))

    for opt, arg in opts:
        try:
            if opt in ("--agg", "--region", "--ccrev"):
                # if (opt != "--ccrev") and (filter_params is clm_data_container.filter_params):
                #     ClmUtils.warning("\"%s\" switch is deprecated" % opt)
                for f in re.findall(r"\S+", arg):
                    filter_params.add_ccrev(opt[2:], f)
            elif opt == "--include":
                filter_params.add_tag_filter(FilterParams.TagFilter(arg, "+", "!"))
            elif opt == "--full_set":
                filter_params.set_scalar_attr("full_set", True)
            elif opt == "--max_chains":
                filter_params.set_scalar_attr("max_chains", arg)
            elif opt == "--max_bandwidth":
                include_80_80 = False
                if arg == "80+80":
                    include_80_80 = True
                    arg = "80"
                filter_params.set_scalar_attr("max_bandwidth", Bandwidth.parse(arg))
                filter_params.set_scalar_attr(
                    "include_80_80",
                    include_80_80 or (filter_params.max_bandwidth >= Bandwidth._160))
            elif opt == "--band":
                filter_params.set_scalar_attr("band", Band.parse(arg))
            elif opt == "--abg":
                filter_params.set_scalar_attr("abg", True)
                filter_params.set_scalar_attr("ac", False)
            elif opt == "--abgn":
                filter_params.set_scalar_attr("abg", False)
                filter_params.set_scalar_attr("ac", False)
            elif opt == "--txbf":
                filter_params.set_scalar_attr("txbf", True)
            elif opt == "--dfs_tw":
                filter_params.set_scalar_attr("dfs_tw", True)
            elif opt == "--ulb":
                raise ValueError("ULB feature is not supported anymore. --ulb switch shall be removed from all .clm files")
            elif opt == "--ulb_":
                filter_params.set_scalar_attr("ulb", True)
            elif opt == "--1024_qam":
                filter_params.set_scalar_attr("include_1024_qam", True)
            elif opt == "--regrev_remap":
                filter_params.set_scalar_attr("regrev_remap", True)
            elif opt == "--scr_idx4":
                filter_params.set_scalar_attr("scr_idx_4bit", True)
            elif opt == "--scr_idx8":
                filter_params.set_scalar_attr("scr_idx_8bit", True)
            elif opt == "--scr_40mhz":
                filter_params.set_scalar_attr("sub_chan_rules_40", True)
            elif opt == "--scr_inc":
                filter_params.set_scalar_attr("sub_chan_rules_inc", True)
            elif opt == "--scr_inc2":
                filter_params.set_scalar_attr("sub_chan_rules_inc_separate", True)
            elif opt == "--loc_idx12":
                filter_params.set_scalar_attr("loc_idx_12bit", True)
            elif opt == "--vht_txbf0":
                filter_params.set_scalar_attr("vht_txbf0", True)
            elif opt == "--no_per_antenna":
                filter_params.set_scalar_attr("per_antenna", False)
            elif opt == "--all_data":
                parse_args(
                    string_to_argv(
                        FilterParams.filter_all(all_regions=False).command_lines[0][1]),
                    clm_data_container, filter_params, options, "")
                filter_params.pop_command_line()
                filter_params.set_scalar_attr("assume_all_regions", True)
            elif opt == "--inc_from":
                clm_data_container.set_clm(arg, True)
            elif opt == "--compare":
                options.set_option("compare", arg)
            elif opt == "--list":
                for f in re.findall(r"\S+", arg):
                    if f[:2] == "--":
                        raise ValueError("This syntax is not supported anymore")
                    if f not in Options.possible_list_values:
                        raise ValueError("Allowed values are: \"%s\"" % "\", \"".join(Options.possible_list_values))
                    options.set_option("list_values", f)
            elif opt == "--ignore_missed_items":
                filter_params.set_scalar_attr("ignore_missed", True)
            elif opt == "--verbose":
                options.set_option("verbose", True)
            elif opt == "--force_section":
                options.set_option("force_section", arg)
            elif opt == "--apitest":
                options.set_option("apitest_listing_name", arg)
            elif opt == "--obfuscate":
                pass
            elif opt == "--noobfuscate":
                options.set_option("obfuscate", False)
            elif opt == "--blob_format":
                if arg not in BlobFormatVersionInfo.get_supported_versions():
                    raise ValueError("Allowed values are: \"%s\"" % "\", \"".join(BlobFormatVersionInfo.get_supported_versions().keys()))
                filter_params.set_scalar_attr("blob_format", BlobFormatVersionInfo.get_supported_versions()[arg])
            elif opt == "--clmapi_include_dir":
                options.set_option("clmapi_include_dir", arg)
            elif opt == "--bcmwifi_include_dir":
                options.set_option("bcmwifi_include_dir", arg)
            elif opt == "--version":
                options.set_option("print_version", True)
            elif opt == "--print_options":
                options.set_option("print_options", True)
            elif opt == "--user":
                options.set_option("user_string", arg)
            elif opt == "--base_user":
                options.set_option("base_user_string", arg)
            elif opt in ("-?", "-h", "--help"):
                print _usage
                sys.exit(0)
            elif opt in ("--config_file", "--base_config_file"):
                filename = arg
                try:
                    args_from_file = string_to_argv(open(filename).read())
                except:
                    ClmUtils.error("Error reading \"%s\": %s" % (filename, ClmUtils.exception_msg()))
                pos_args = parse_args(args_from_file, clm_data_container,
                                      filter_params if opt == "--config_file" else clm_data_container.base_filter_params,
                                      options, name=filename)
                if pos_args:
                    ClmUtils.error("Positional arguments in %s: %s" % (filename, pos_args))
            else:
                ClmUtils.error("Internal error: unsupported switch %s" % opt)
        except (SystemExit, KeyboardInterrupt, MemoryError, NameError):
            raise
        except:
            ClmUtils.error("\"%s\" option invalid: %s" % (opt, ClmUtils.exception_msg()))
    return args

# =============================================================================


def get_maximum_blob_version(options):
    """ Computes maximum BLOB version - ClmAPI version if path to it specified,
    _BLOB_VERSION otherwise

    Arguments:
    options -- Container with nonfiltering options
    Returns Maximum BLOB version in string form
    """
    if options.clmapi_include_dir is None:
        return _BLOB_VERSION
    try:
        filename = os.path.join(options.clmapi_include_dir, "wlc_clm_data.h")
        with open(filename) as f:
            contents = f.read()
    except:
        ClmUtils.error("Can't read %s. Probably ClmAPI include directory specified incorrectly" % filename)
    components = []
    for component in ("CLM_FORMAT_VERSION_MAJOR", "CLM_FORMAT_VERSION_MINOR"):
        m = re.search(r"\#\s*define\s+%s\s+(\d+)" % component, contents)
        if m is None:
            ClmUtils.error("%s not found in %s" % (component, filename))
        components.append(m.group(1))
    clmapi_version = ".".join(components)
    ret = None
    for available_version in BlobFormatVersionInfo.get_supported_versions().iterkeys():
        if ClmUtils.compare_versions(available_version, clmapi_version) <= 0:
            if (ret is None) or (ClmUtils.compare_versions(available_version, ret) > 0):
                ret = available_version
    if ret is None:
        ClmUtils.error("Can't generate BLOB for ClmAPI supporting only up to %s format - This ClmAPI is too old" % clmapi_version)
    return ret

# =============================================================================


def parse_cmd_line_args(argv, clm_data_container, options):
    """ Process command line parameters

    This function actually process only positional parameters. Key parameters
    processing is delegated to parse_args()

    Arguments:
    argv               -- List of command line parameters
    clm_data_container -- Filter data container
    options            -- Container for nonfiltering options
    """
    if _ENV_OPTIONS in os.environ:
        argv = string_to_argv(os.environ[_ENV_OPTIONS]) + argv
    args = parse_args(argv, clm_data_container, clm_data_container.filter_params, options)
    if len(args) >= 1:
        try:
            clm_data_container.set_clm(args[0])
        except (KeyboardInterrupt, SystemExit, MemoryError, NameError):
            raise
        except:
            ClmUtils.error("Error opening source CLM file \"%s\": %s" % (args[0], ClmUtils.exception_msg()))
    if len(args) >= 2:
        options.set_option("blob_file_name", str(args[1]))
    if len(args) > 2:
        ClmUtils.error("Too many positional arguments: \"%s\"" % " ".join(args))
    maximum_blob_version = get_maximum_blob_version(options)
    rates_caps = BcmWifiRatesCaps(options.bcmwifi_include_dir)
    for filter_params in (clm_data_container.filter_params, clm_data_container.base_filter_params):
        if ClmUtils.compare_versions(maximum_blob_version, filter_params.blob_format.version_string) < 0:
            filter_params.set_scalar_attr(
                "blob_format", BlobFormatVersionInfo.get_supported_versions()[maximum_blob_version],
                explicit=False)
        if filter_params.max_chains > rates_caps.max_chains:
            filter_params.set_scalar_attr("max_chains", rates_caps.max_chains, explicit=False)
        if not rates_caps.vht:
            filter_params.set_scalar_attr("ac", False, explicit=False)
        if not rates_caps.txbf:
            filter_params.set_scalar_attr("txbf", False, explicit=False)
        if not rates_caps.txbf0:
            filter_params.set_scalar_attr("vht_txbf0", False, explicit=False)
        if not rates_caps.has_1024_qam:
            filter_params.set_scalar_attr("include_1024_qam", False, explicit=False)
        if not clm_data_container.filter_params.blob_format.ac_allowed:
            filter_params.set_scalar_attr("ac", False, explicit=False)
        if filter_params.max_bandwidth > clm_data_container.filter_params.blob_format.max_bandwidth:
            filter_params.set_scalar_attr("max_bandwidth",
                                          clm_data_container.filter_params.blob_format.max_bandwidth,
                                          explicit=False)
        if clm_data_container.filter_params.blob_format.rev16:
            filter_params.set_scalar_attr("regrev_remap", False, explicit=False)

# =============================================================================


def print_version(clm_data_container):
    """ Prints version information """
    data = clm_data_container.get_blob_data()
    if data is not None:
        print "CLM data version: %s" % (data.clm_version if data.clm_version else "Unknown")
        if data.get_apps_version(verbatim=False) is not None:
            print "Customization data version: %s" % data.get_apps_version(verbatim=False)
        print "CLM data format version: %s" % (data.clm_format_version if data.clm_format_version else "Unknown")

        generator = (data.generator_name if data.generator_name else "Unknown")
        if data.generator_name:
            generator += " (%s)" % (("v" + data.generator_version) if data.generator_version else "version unknown")
        print "CLM data generated by: %s" % generator
    print "Source XML format versions supported: %s - %s" % (_XML_MIN_VERSION, _XML_VERSION)
    print "Generated BLOB format version: %s" % clm_data_container.filter_params.blob_format.version_string

# =============================================================================


def print_options(clm_data_container):
    """ Prints options, specified in command line and in config files """
    command_lines = clm_data_container.filter_params.command_lines
    base_command_lines = clm_data_container.base_filter_params.command_lines
    print "ClmCompiler command line options: %s" % command_lines[0][1]
    for filename, command_line in command_lines[1:]:
        print "ClmCompiler options from %s: %s" % (filename, command_line)
    if base_command_lines:
        for filename, command_line in base_command_lines:
            print "ClmCompiler base command line options from %s: %s" % (filename, command_line)

# =============================================================================


def print_list_values(clm_data_container, options):
    """ Prints possible filter values for options, specified in command line

    Arguments:
    clm_data_container -- CLM data container
    options            -- Nonfiltering command line options
    """
    af = clm_data_container.get_available_filters()
    for list_value in options.list_values:
        values = []
        if list_value == "region":
            values = af.get_sorted_regions() if clm_data_container.filter_params.filtering_set else af.get_sorted_ccrevs()
        elif list_value == "tag":
            values = af.get_sorted_tags()
        else:
            raise ValueError("Internal error: invalid listable \"%s\"" % list_value)
        print "Possible %s values: \n\t%s" % (list_value, "\n\t".join([str(value) for value in values]) if values else "(None)")

# =============================================================================


def try_other_process(argv, options, force=False):
    """ If executed on *nix system with the only task of BLOB generation and
    target file is, probably, network and temporary directory is, probably,
    local then creates BLOB in a temporary directory in a separate process and
    then copy it to proper destination. This allows to avoid stalling on a
    network write with large amoubnt of memory occupied

    Arguments:
    argv    -- Parameters passed to main() (command line options without script
               name and python interpreter)
    options -- Processed parameters
    force   -- Force maing BLON in other process - for debug purposes only
    Returns True if processing in other process was done, False otherwise
    """
    if (not force) and (os.name != "posix"):
        return False  # Abandon if not in *nix

    def is_local_file(path):  # pragma: no cover
        """ *nix coarse approximation of local/remote file detection: path
        does not have mount points below the root

        Arguments:
        path -- Path in question
        Returns True if path is local, False if remote
        """
        path = os.path.realpath(path).split('/')[1:]
        for index in range(1, len(path) + 1):
            if os.path.ismount('/' + '/'.join(path[:index])):
                return False
        return True

    if not force and ((not is_local_file(tempfile.gettempdir())) or
                      is_local_file(options.blob_file_name)):  # pragma: no cover
        return False  # Abandon if dest local or temp directory nonlocal
    temp_blob = None
    try:
        fd, temp_blob = tempfile.mkstemp()
        os.close(fd)
        assert is_local_file(temp_blob)
        argv = list(argv)  # Making copy of argv[] before modifying it
        for i in range(len(argv) - 1, -1, -1):  # Set temp dest file
            if argv[i] == options.blob_file_name:
                argv[i] = temp_blob
                break
            else:
                return False  # Should never happen - just in case
        env = dict(os.environ)
        env[_ENV_NO_TITLE] = "yes"
        ret = subprocess.call([sys.executable, __file__] + argv, env=env)
        if ret:
            sys.exit(ret)
        shutil.move(temp_blob, options.blob_file_name)
        temp_blob = None
        return True
    finally:
        if temp_blob:
            try:
                os.unlink(temp_blob)
            except EnvironmentError:  # pragma: no cover
                pass

# =============================================================================


def clear_caches():
    """ Clears cached single-value objects - for use in preparation to nosetest
    tests """
    ChannelType.clear_cache()
    ChannelRange.clear_cache()
    Locale.clear_cache()

# =============================================================================


def main(argv):
    """ Main function """
    try:
        ClmUtils.error_source = "ClmCompiler: "
        if os.environ.get(_ENV_NO_TITLE) != "yes":
            print "ClmCompiler v%s" % _PROGRAM_VERSION
        clm_data_container = ClmDataContainer()
        options = Options()
        parse_cmd_line_args(argv, clm_data_container, options)
        acted = False
        if options.print_version:
            acted = True
            print_version(clm_data_container)
        if options.print_options:
            acted = True
            print_options(clm_data_container)
        if (not clm_data_container.has_data) and (clm_data_container.filter_params.filtering_set or
                                                  options.list_values or options.blob_file_name or
                                                  options.apitest_listing_name):
            ClmUtils.error("Source CLM XML file shall be specified")
        if options.list_values:
            acted = True
            print_list_values(clm_data_container, options)
        if options.compare:
            acted = True
            clm_data_container.print_data_diff(options.compare)
        if options.blob_file_name or (options.verbose and clm_data_container.filter_params.filtering_set and not options.apitest_listing_name):
            if acted or options.apitest_listing_name or (not try_other_process(argv, options)):
                ClmBlob(clm_data_container, options)
            acted = True
        if options.apitest_listing_name:
            acted = True
            ApitestWriter(clm_data_container, options)
        if not acted:
            print _usage
            sys.exit(1)
    except SystemExit as se:
        if se.code:
            if not isinstance(se.code, int) or (str(se.code) != ClmUtils.exception_msg()):
                print >>sys.stderr, ClmUtils.exception_msg()
            return 1
        return 0
    except KeyboardInterrupt:
        print >> sys.stderr, "ClmCompiler: Error: Interrupted"
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
