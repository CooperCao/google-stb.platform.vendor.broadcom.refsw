#!/usr/bin/env python2.7
""" Reads/writes SAR data from/to .c and .csv files. Also provides some helper
functionality (printing SAR .csv file content, listing used/available board IDs
and names)
"""

import os
import sys
import re
import csv
import glob
import logging
import tempfile
import subprocess
from collections import OrderedDict
from argparse import ArgumentParser, RawDescriptionHelpFormatter

BCMDEVS_H_PATH = "src/include/bcmdevs.h"
BASE_URL = "http://svn.sj.broadcom.com/svn/wlansvn/"

# =============================================================================
# Common utility functions


def error(msg):
    """ Prints given error message and exists with error

    Arguments:
    msg -- Error message
    """
    logging.error(msg)
    sys.exit(1)

# =============================================================================


class SarData(object):
    """ SAR data

    Attributes:
    comment -- Comment to SAR data or None
    id_num  -- Numerical ID
    id_name -- Name ID
    sar_map -- Maps subband indices to chain->limit maps
    """

    # Subband indices
    SB_2_4, SB_5_2, SB_5_3, SB_5_6, SB_5_8 = range(5)

    # Maximum number of chains
    MAX_CHAINS = 4

    # Maps subband indices to subband names
    sb_name = {SB_2_4: "2.4", SB_5_2: "5.2", SB_5_3: "5.3", SB_5_6: "5.6",
               SB_5_8: "5.8"}
    # Maps subband names to subband indices
    sb_idx = {name: idx for idx, name in sb_name.iteritems()}

    # All subband indices
    sb_indices = sorted(sb_name.iterkeys())

    # Maps subband indices to normal channel lists
    sb_channels = {
        SB_2_4: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13],
        SB_5_2: [36, 40, 48], SB_5_3: [52, 56, 60, 64],
        SB_5_6: [100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144],
        SB_5_8: [149, 153, 157, 161, 165]}

    # Maps subband indices to Japan channel lists
    sb_japan_channels = {SB_2_4: [14], SB_5_2: [34, 38, 42, 46]}

    def __init__(self):
        """ Constructor. Initializes to empty SAR data """
        self.comment = None
        self.id_num = None
        self.id_name = None
        self.sar_map = {}

    def chains(self):
        """ Returns maximum number of chains in SAR data """
        ret = 0
        for chain_dict in self.sar_map.itervalues():
            ret = max(ret, max(chain_dict.iterkeys()) + 1)
        return ret

# =============================================================================


class SarFile(object):
    """ SAR file reader/writer

    Private attributes:
    _file_name -- SAR file name
    _data      -- SAR data currently being read
    _line      -- 1-based line number curently being read
    _chains    -- Number of chains declared in SAR row
    """

    ST_INITIAL, ST_ID, ST_HEADINGS, ST_CHAINS, ST_LIMITS, ST_DONE = range(6)
    HEADINGS = ["Frequency\nBands", "Max Tx Power"]
    ID_HEADING = "Board ID"
    CHAIN_HEADING = "Chain "
    SB_HEADING_FMT = "%sGHz"
    MISS_MESSAGES = {ST_ID: "Missing Board ID row",
                     ST_CHAINS: "Missing \"Chain\" row"}

    def __init__(self, file_name):
        """ Constructor

        Arguments:
        file_name -- SAR file name
        """
        self._file_name = file_name
        self._data = None
        self._line = 0
        self._chains = 0

    def read(self):
        """ Reads SAR file. Returns SarData object """
        try:
            self._data = SarData()
            self._line = 0
            with open(self._file_name, "rU") as f:
                csv_reader = csv.reader(f)
                state = SarFile.ST_INITIAL
                for row in csv_reader:
                    self._line += 1
                    for new_state, check_fnc, parse_fnc in \
                        [(SarFile.ST_ID, self._is_id_row, self._parse_id_row),
                         (SarFile.ST_HEADINGS, self._is_heading_row, None),
                         (SarFile.ST_CHAINS, self._is_chains_row,
                          self._parse_chains_row),
                         (SarFile.ST_LIMITS, self._is_limits_row,
                          self._parse_limits_row),
                         ((SarFile.ST_INITIAL, None, self._parse_comment)
                          if state == SarFile.ST_INITIAL else
                          (SarFile.ST_DONE, None, None))]:
                        if not check_fnc or check_fnc(row):
                            if new_state < state:
                                self._error(
                                    "File may contain just one SAR table")
                            while state < new_state:
                                state += 1
                                if (state < new_state) and \
                                        (state in SarFile.MISS_MESSAGES):
                                    self._error(SarFile.MISS_MESSAGES[state])
                            if parse_fnc:
                                parse_fnc(row)
                            break
        except EnvironmentError as exc:
            error("Error reading \"%s\": %s" % (self._file_name, exc.strerror))
        if state == SarFile.ST_INITIAL:
            error("SAR table not found in \"%s\"" % self._file_name)
        return self._data

    def write(self, data):
        """ Writes given SarData object to SAR file """
        try:
            with open(self._file_name, "wb") as f:
                csv_writer = csv.writer(f)
                csv_writer.writerow(["<<Broadcom-WL-IPTag/Proprietary:>>"])
                if data.comment:
                    csv_writer.writerow([data.comment])
                csv_writer.writerow([SarFile.ID_HEADING,
                                     "0x%04X" % data.id_num])
                csv_writer.writerow(SarFile.HEADINGS)
                csv_writer.writerow(
                    [""] +
                    ["%s%d" % (SarFile.CHAIN_HEADING, i)
                     for i in range(max(1, data.chains()))])
                for subband_idx in sorted(data.sar_map.iterkeys()):
                    row = \
                        [SarFile.SB_HEADING_FMT % SarData.sb_name[subband_idx]]
                    for chain in range(max(data.sar_map[subband_idx].
                                           iterkeys()) + 1):
                        row.append(("%f" % data.sar_map[subband_idx][chain])
                                   if chain in data.sar_map[subband_idx]
                                   else "")
                    csv_writer.writerow(row)
        except EnvironmentError as exc:
            error("Error writing \"%s\": %s" % (self._file_name, exc.strerror))

    def _is_id_row(self, row):
        """ True if given row is Board ID row

        Arguments:
        row -- List of CSV row's cells
        Returns true if given row is Board ID row
        """
        return (len(row) >= 2) and \
            self._headings_equal(row[0], SarFile.ID_HEADING) and \
            re.match(r"^[ \t\nxX0-9a-fA-F]+$", row[1])

    def _parse_id_row(self, row):
        """ Parses ID row into SAR data object being read

        Arguments:
        row -- List of CSV row's cells
        """
        if not row[1].lower().startswith("0x"):
            self._error("Board ID shall have '0x' prefix")
        try:
            self._data.id_num = int(row[1], 16)
        except ValueError:
            self._error("Invalid Board ID format")

    def _is_heading_row(self, row):
        """ True if given row is heading row

        Arguments:
        row -- List of CSV row's cells
        Returns true if given row is heading row
        """
        return (len(row) >= 2) and \
            self._headings_equal(row[0], SarFile.HEADINGS[0]) and \
            self._headings_equal(row[1], SarFile.HEADINGS[1])

    def _is_chains_row(self, row):
        """ True if given row is chain heading

        Arguments:
        row -- List of CSV row's cells
        Returns true if given row is chain heading
        """
        return (len(row) >= 2) and \
            self._headings_equal(row[1], SarFile.CHAIN_HEADING + "0")

    def _parse_chains_row(self, row):
        """ Parses chain heading row into SAR data object being read

        Arguments:
        row -- List of CSV row's cells
        """
        self._chains = 0
        for cell in row[1:]:
            if self._headings_equal(cell,
                                    SarFile.CHAIN_HEADING + str(self._chains)):
                self._chains += 1
            else:
                break

    def _is_limits_row(self, row):
        """ True if given row is SAR limits row

        Arguments:
        row -- List of CSV row's cells
        Returns true if given row is SAR limits row
        """
        return (len(row) >= 1) and (self._get_sb(row[0]) is not None)

    def _parse_limits_row(self, row):
        """ Parses SAR limits row into SAR data object being read

        Arguments:
        row -- List of CSV row's cells
        """
        subband_idx = self._get_sb(row[0])
        if subband_idx in self._data.sar_map:
            self._error(
                "SAR limits for \"%s\" subband specified more than once" %
                (SarFile.SB_HEADING_FMT % SarData.sb_name[subband_idx]))
        for chain_idx in range(min(len(row) - 1, self._chains)):
            cell = row[chain_idx + 1].strip()
            if cell == "":
                continue
            try:
                limit = float(cell)
            except ValueError:
                self._error("Wrong format of SAR limit value: \"%s\"" % cell)
            if (not (-31 <= limit <= 31.75)) or \
                    ((limit * 4) != int(limit * 4)):
                self._error(
                    ("Wrong SAR limit value \"%s\". Must be multiple " +
                     "of 0.25 and less than 32") % cell)
            self._data.sar_map.setdefault(subband_idx, {})[chain_idx] = limit

    def _parse_comment(self, row):
        """ Attempts to parse given row as SAR limits row into SAR data object
        being read

        Arguments:
        row -- List of CSV row's cells
        """
        self._data.comment = row[0].strip() if len(row) >= 1 else None
        if (self._data.comment == "") or \
                ("<<Broadcom-WL-IPTag" in self._data.comment):
            self._data.comment = None

    def _headings_equal(self, heading1, heading2):
        """ True if normalized representation of two given heading strings are
        equal """
        normalize = lambda s: re.sub(r"\s", " ", s.strip().lower())
        return normalize(heading1) == normalize(heading2)

    def _get_sb(self, sb_heading):
        """ If given heading is subband row heading - returns subband index.
        Otherwise returns None
        """
        for subband_idx in SarData.sb_name:
            if self._headings_equal(
                    sb_heading,
                    SarFile.SB_HEADING_FMT % SarData.sb_name[subband_idx]):
                return subband_idx
        return None

    def _error(self, msg):
        """ Prints given error message, prefixed with file name and line number
        """
        error("File %s, line %d. %s" % (self._file_name, self._line, msg))

# =============================================================================


class CFile(object):
    """ Reader/writer/updater of .c file containing SAR table """

    # Name of SAR limits array variable in SAR table file (not in
    # wlc_channel.c)
    SAR_LIST_NAME = "wlc_sar_tbl"

    # Name of SAR limits array length variable in SAR table file (not in
    # wlc_channel.c)
    SAR_LIST_LEN_NAME = "wlc_sar_tbl_len"

    # Type of entry in SAR limits array in SAR table file (not in
    # wlc_channel.c)
    SAR_LIST_TYPE = "wlc_board_sar_limit_t"

    # Constant denoting no SAR limit in a particular entry
    NO_LIMIT = "WLC_TXPWR_MAX"

    # Top part of generated SAR table file
    FILE_HEAD = """/** SAR table
 * This file was generated by %s, please don't edit it manually
 * <<Broadcom-WL-IPTag/Proprietary:>>
 */

#include <wlc_cfg.h>
#ifdef WL_SARLIMIT
#include <wlc_sar_tbl.h>
#include <typedefs.h>
#include <osl.h>
#include <siutils.h>
#include <bcmdevs.h>
#include <wlc_pub.h>
#include <wlc_channel.h>

#define QDB(n) ((n) * WLC_TXPWR_DB_FACTOR)

const %s %s[] = {
""" % (os.path.basename(__file__), SAR_LIST_TYPE, SAR_LIST_NAME)

    # Bottom part of generated SAR table file
    FILE_TAIL = """};

/* Number of entries in SAR table */
const size_t %s = (size_t)(sizeof(%s)/sizeof(%s[0]));

#endif /* WL_SARLIMIT */
""" % (SAR_LIST_LEN_NAME, SAR_LIST_NAME, SAR_LIST_NAME)

    def __init__(self, file_name):
        """ Constructor

        Arguments:
        file_name -- name of SAR table file or of wlc_channel.c file
        """
        self._file_name = file_name

    def read(self):
        """ Reads SAR table file. Returns array of SarData objects """
        ret = []
        try:
            with open(self._file_name, "r") as f:
                content = f.read()
            m = re.search(
                (r"(sar_default|%s)\s*\[\]\s*=\s*\{((.|\n)*?" +
                 r"\}[^{}]*\}[^{}]*\}[^{}]*\})[^{}]*\}\s*;") %
                CFile.SAR_LIST_NAME, content)
            if m is None:
                self._error("SAR limits table not found")
            sar_table = re.sub(r"/\*.*?\*/", "", re.sub(r"\s", "", m.group(2)))
            for entry_match in re.finditer(
                    r"\{(?P<name>\w+),\{\{(?P<l24>[^}]*)\}," +
                    r"\{\{(?P<l52>[^}]*)\},\{(?P<l53>[^}]*)\}," +
                    r"\{(?P<l56>[^}]*)\},\{(?P<l58>[^}]*)\}\}\}\}",
                    sar_table):
                data = SarData()
                name = entry_match.group("name")
                if re.match(r"^\d.*$", name):
                    data.id_num = int(name, 0)
                else:
                    data.id_name = name
                if any(existing_data.id_name == data.id_name
                       for existing_data in ret):
                    self._error(
                        "SAR data for board \"%s\" specified more than once"
                        % data.id_name)
                ret.append(data)
                for group_name, subband_idx in [("l24", SarData.SB_2_4),
                                                ("l52", SarData.SB_5_2),
                                                ("l53", SarData.SB_5_3),
                                                ("l56", SarData.SB_5_6),
                                                ("l58", SarData.SB_5_8)]:
                    chain_limits = entry_match.group(group_name).split(",")
                    for chain_idx in range(len(chain_limits)):
                        limit_str = chain_limits[chain_idx]
                        m = re.match(
                            (r"^((?P<no_limit>%s)|(?P<q>\d+)|" +
                             r"(QDB\((?P<db>\d+)\)(\+(?P<qdb>\d+))?))$") %
                            CFile.NO_LIMIT,
                            limit_str)
                        if m is None:
                            self._error(
                                ("Invalid SAR limit format: \"%s\" in " +
                                 "definition of board \"%s\"") %
                                (limit_str, data.id_name))
                        if m.group("no_limit"):
                            continue
                        if m.group("q"):
                            dbm_str = "0"
                            qdbm_str = m.group("q")
                        else:
                            dbm_str = m.group("db")
                            qdbm_str = m.group("qdb") or "0"
                        try:
                            data.sar_map.setdefault(subband_idx, {})[
                                chain_idx] = float(dbm_str) + float(qdbm_str)/4
                        except ValueError:
                            self._error(
                                "Invalid SAR limit format: \"%s\"" % limit_str)
        except EnvironmentError as exc:
            error("Error reading \"%s\": %s" % (self._file_name, exc.strerror))
        return ret

    def write(self, sars):
        """ Writes SAR table file containing entries from given list of SarData
        objects """
        try:
            with open(self._file_name, "wb") as f:
                f.write(CFile.FILE_HEAD)
                for sar in sars:
                    f.write(self._sar_repr(sar))
                f.write(CFile.FILE_TAIL)
        except EnvironmentError as exc:
            error("Error writing \"%s\": %s" % (self._file_name, exc.strerror))

    def update(self, sars):
        """ Updates/adds SAR table entries from given list of SarData objects
        """
        try:
            with open(self._file_name, "rb") as f:
                content = f.read()
            for sar in sars:
                pattern = \
                    r"\{\s*%s\s*,(.|\n)*?\}[^{}]*\}[^{}]*\}[^{}]*\},?\n?" % \
                    sar.id_name
                if re.search(pattern, content):
                    content = re.sub(pattern, self._sar_repr(sar).lstrip(),
                                     content)
                else:
                    m = re.search(
                        r"(sar_default|%s)\s*\[\]\s*=\s*\{.*\n" %
                        CFile.SAR_LIST_NAME, content)
                    if m is None:
                        self._error("SAR limits table not found")
                    content = content[:m.end()] + self._sar_repr(sar) + \
                        content[m.end():]
            with open(self._file_name, "wb") as f:
                f.write(content)
        except EnvironmentError as exc:
            error("Error updating \"%s\": %s" %
                  (self._file_name, exc.strerror))

    def _sar_repr(self, sar):
        """ Returns C representation of given SAR data object """
        if sar.id_name is None:
            error("Board name constant for board ID 0x%04X not found" %
                  sar.id_num)
        return \
            ("%s\t{%s, {{%s}, /* 2g SAR limits */\n" +
             "\t{{%s}, /* 5g subband 0 SAR limits */\n" +
             "\t{%s}, /* 5g subband 1 SAR limits */\n" +
             "\t{%s}, /* 5g subband 2 SAR limits */\n" +
             "\t{%s} /* 5g subband 3 SAR limits */\n" +
             "\t}}},\n") % \
            (("\t/* %s */\n" % sar.comment) if sar.comment else "",
             sar.id_name,
             self._sar_sb_repr(sar, SarData.SB_2_4),
             self._sar_sb_repr(sar, SarData.SB_5_2),
             self._sar_sb_repr(sar, SarData.SB_5_3),
             self._sar_sb_repr(sar, SarData.SB_5_6),
             self._sar_sb_repr(sar, SarData.SB_5_8))

    def _sar_sb_repr(self, sar, subband_idx):
        """ Returns C representation of SAR limits for given subband

        Arguments:
        sar         -- SarData object
        subband_idx -- Subband index
        Returns C representation of SAR limits for given subband
        """
        sb_dict = sar.sar_map.get(subband_idx, {})
        ret = []
        for chain_idx in range(SarData.MAX_CHAINS):
            dbm = sb_dict.get(chain_idx)
            if dbm is None:
                ret.append(CFile.NO_LIMIT)
                continue
            dbm_repr = "QDB(%d)" % int(dbm)
            if dbm != int(dbm):
                dbm_repr += "+%d" % (int)((dbm - int(dbm)) * 4)
            ret.append(dbm_repr)
        return ", ".join(ret)

    def _error(self, msg):
        """ Prints given error message, prefixed with file name """
        error("File %s. %s" % (self._file_name, msg))


class HFile(object):
    """ Reader and handler of Board IDs from bcmdevs.h

    Private attributes:
    _name_to_num -- Maps board ID name sto board ID numbers. Maintains original
                    order of board names
    _num_to_name -- Maps board numbers to names
    """

    # Types of contiguous segments of #define statements
    ST_UNKNOWN, ST_BOARD, ST_NONBOARD = range(3)

    def __init__(self, file_name):
        """ Constructor. Reads given bcmdevs.h file

        Arguments:
        file_name -- Full name of bcmdevs.h
        """
        self._name_to_num = OrderedDict()
        self._num_to_name = {}
        total_infos = []
        try:
            with open(file_name, "r") as f:
                seg_infos = []
                seg_type = HFile.ST_UNKNOWN
                for line in f:
                    m = re.match(r"^/\*(.*)\*/\s+$", line)
                    if m is not None:
                        if seg_type != HFile.ST_NONBOARD:
                            total_infos += seg_infos
                        seg_type = HFile.ST_BOARD \
                            if re.search(r"(\sboards\s)|(\sboard\stypes\s)",
                                         m.group(1)) else HFile.ST_UNKNOWN
                        seg_infos = []
                        continue
                    if seg_type == HFile.ST_NONBOARD:
                        continue
                    m = re.match(r"#define\s+(\w+)\s+(.*)\n", line)
                    if not m:
                        continue
                    name = m.group(1)
                    if re.search(r"(_BOARD|_SSID)$", name):
                        seg_type = HFile.ST_BOARD
                    value = re.sub(r"/\*.*?((\*/)|$)", "", m.group(2)).strip()
                    if not re.match(r"^0(x|X)[0-9a-fA-F]{4}$", value):
                        if not re.match(r"^\w*$", value):
                            seg_type = HFile.ST_NONBOARD
                        continue
                    seg_infos.append((int(value, 16), name))
        except EnvironmentError as exc:
            error("Error reading \"%s\": %s" % (file_name, exc.strerror))
        if seg_type != HFile.ST_NONBOARD:
            total_infos += seg_infos
        for id_num, id_name in total_infos:
            if id_num in self._num_to_name:
                if self._num_to_name[id_num] is not None:
                    del self._name_to_num[self._num_to_name[id_num]]
                    self._num_to_name[id_num] = None
                continue
            if id_name in self._name_to_num:
                if self._name_to_num[id_name] is not None:
                    del self._num_to_name[self._name_to_num[id_name]]
                    self._name_to_num[id_name] = None
                continue
            self._num_to_name[id_num] = id_name
            self._name_to_num[id_name] = id_num
        for id_name, id_num in list(self._name_to_num.items()):
            if id_num is None:
                del self._name_to_num[id_name]
        for id_num, id_name in list(self._num_to_name.items()):
            if id_name is None:
                del self._num_to_name[id_num]

    def name_to_num(self, id_name):
        """ Returns number for given name or None """
        return self._name_to_num.get(id_name)

    def num_to_name(self, id_num):
        """ Return sname for given  number or None """
        return self._num_to_name.get(id_num)

    def name_num_pairs(self):
        """ Returns list of (name, number) tuples """
        return self._name_to_num.iteritems()


# =============================================================================
# Helper functions for subcommand processing functions

def svn(args):
    """ Runs SVN with given arguments

    Arguments:
    args -- List of SVN arguments
    """
    output = open(os.devnull, 'w')
    return not subprocess.call(["svn"] + args, stdout=output, stderr=output)


def get_h_file(args, allow_absence=False):
    """ Returns HFile object, specified in command line

    Arguments:
    args          -- Parsed arguments from ArgParser. Pertinent arguments are
                     "driver_dir" and "driver_svn"
    allow_absence -- True means that if neither driver_dir nor driver_svn
                     specified and trunk bcmdevs.h not available this function
                     returns None (instead of failure)
    Returns HFile object or None (see comments to 'allow_absence' argument)
    """
    if args.driver_dir is not None:
        file_name = os.path.join(args.driver_dir, BCMDEVS_H_PATH)
        if not os.path.isfile(file_name):
            error("\"%s\" not found" % file_name)
        return HFile(file_name)
    url = args.driver_svn if args.driver_svn else "trunk"
    if "/" not in url:
        if url == "trunk":
            url = BASE_URL + "proj/trunk"
        else:
            url = BASE_URL + "proj/branches/" + url
    url += "/" + BCMDEVS_H_PATH
    if not svn(["info", url]):
        if (args.driver_svn is None) and allow_absence:
            return None
        error("Can't find \"%s\" in SVN" % url)
    file_name = None
    try:
        tmp_fd, file_name = tempfile.mkstemp(suffix=".h",
                                             prefix="bcmdevs_for_sar_conv_")
        os.close(tmp_fd)
        if not svn(["export", "--force", url, file_name]):
            error("Can't retrieve \"%s\"" % url)
        return HFile(file_name)
    finally:
        if file_name is not None:
            os.unlink(file_name)


def parse_ids(ids):
    """ Retrieves board IDs from board list arguments

    Arguments:
    ids -- List (possibly empty) of board list argument of some type (--id,
           --empty, etc.)
    Returns tuple (num_ids, name_ids) of two lists - one with numerical board
    IDs, another with symbolic board IDs
    """
    num_ids = []
    name_ids = []
    for id_or_ids in ids:
        for board_id in re.split(r"[ \t,;]+", id_or_ids):
            if re.match(r"^(0(x|X))?[0-9a-fA-F]{,4}$", board_id):
                try:
                    num_ids.append(int(board_id, 16))
                except ValueError:
                    error("Invalid Board ID: \"%s\"" % board_id)
            else:
                name_ids.append(board_id)
    return num_ids, name_ids


def read_sar_files(filenames, h_file, empty_ids):
    """ Reads SAR files from given list. Initializes id_name attribute
    according to given HFile object

    Arguments:
    filenames -- List of SAR file names
    h_file    -- HFile object or None (in latter case id_name attributes are
                 not initialized)
    Returns list of SarData objects
    """
    ret = []
    for file_name in filenames:
        sar = SarFile(file_name).read()
        if h_file is not None:
            sar.id_name = h_file.num_to_name(sar.id_num)
        ret.append(sar)
    empty_num_ids, empty_name_ids = parse_ids(empty_ids)
    for num_id in empty_num_ids:
        sar = SarData()
        sar.id_num = num_id
        if h_file is not None:
            sar.id_name = h_file.num_to_name(num_id)
        ret.append(sar)
    for name_id in empty_name_ids:
        sar = SarData()
        sar.id_name = name_id
        if h_file is not None:
            sar.id_num = h_file.name_to_num(name_id)
        ret.append(sar)
    return ret


def extract_selected_sars(file_name, h_file, ids):
    """ Extracts selected SarData object from given C SAR table file
    file_name -- Full name of C SAR table file
    h_file    -- HFile object
    ids       -- Board IDs from command line (list of lists). Empty means
                 extract all containing SAR objects
    Returns list of SarData objects with id_num and id_name fields initialized
    """
    num_ids, name_ids = parse_ids(ids)
    ret = []
    for sar in CFile(file_name).read():
        if num_ids or name_ids:
            match = False
            if sar.id_name is not None:
                if sar.id_name in name_ids:
                    match = True
                elif num_ids:
                    id_num = h_file.name_to_num(sar.id_name)
                    if id_num is None:
                        error("Can't get numeric ID for board \"%s\"" %
                              sar.id_name)
                    match = id_num in num_ids
            else:
                if sar.id_num in num_ids:
                    match = True
                elif name_ids:
                    id_name = h_file.num_to_name(sar.id_num)
                    if id_name is None:
                        error("Can't get symbolic name for board ID 0x%04X" %
                              sar.id_num)
                    match = id_name in name_ids
            if not match:
                continue
        if sar.id_name is None:
            sar.id_name = h_file.num_to_name(sar.id_num)
        if sar.id_num is None:
            sar.id_num = h_file.name_to_num(sar.id_name)
        ret.append(sar)
    not_found_name_ids = []
    not_found_num_ids = []
    for in_list, out_list, ids_getter in [
            (name_ids, not_found_name_ids,
             lambda s: (s.id_name, h_file.num_to_name(s.id_num))),
            (num_ids, not_found_num_ids,
             lambda s: (s.id_num, h_file.name_to_num(s.id_name)))]:
        for board_id in in_list:
            for sar in ret:
                if board_id in ids_getter(sar):
                    break
            else:
                out_list.append(board_id)
    if not_found_name_ids or not_found_num_ids:
        error("Board with following IDs not found: %s" %
              (", ".join(not_found_name_ids +
                         ["0x%4X" % id_num for id_num in not_found_num_ids])))
    return ret


def get_sar_file_name(sar, name_format, add_extension=True):
    """ Returns SAR file name in given format for given SarData object

    Arguments:
    sar         -- SarData object to get name for
    name_format -- Name format specified in command line
    Returns file name (without directory)
    """
    if name_format.startswith("hex"):
        ret = "%04X" % sar.id_num
    elif name_format.startswith("name"):
        ret = sar.id_name
    elif name_format.startswith("combined"):
        ret = "%s_%04X" % (sar.id_name, sar.id_num)
    else:
        error("Unknown name format \"%s\"" % name_format)
    if name_format.endswith("_lc"):
        ret = ret.lower()
    if add_extension:
        ret += ".csv"
    return ret


def get_file_list(files):
    """ Do the glob processing in given file list. Returns list of individual
    file names """
    ret = []
    for file_name in files:
        globbed = glob.glob(file_name)
        if globbed:
            ret += list(globbed)
        else:
            ret.append(file_name)
    return ret

# =============================================================================
# Subcommand processing functions


def command_create_c(args):
    """ Processes "create_c" command """
    if len(args.files) < 1:
        error("Destination .c file not specified")
    h_file = get_h_file(args)
    sars = read_sar_files(get_file_list(args.files[:-1]), h_file, args.empty)
    CFile(args.files[-1]).write(sars)


def command_update_c(args):
    """ Processes "update_c" command """
    if len(args.files) < 1:
        error("Destination .c file not specified")
    h_file = get_h_file(args)
    sars = read_sar_files(get_file_list(args.files[:-1]), h_file, args.empty)
    CFile(args.files[-1]).update(sars)


def command_extract_sar(args):
    """ Processes "extract_sar" command """
    if not os.path.isdir(args.dir):
        error("\"%s\" is not a directory" % args.dir)
    for sar in extract_selected_sars(args.file, get_h_file(args), args.id):
        file_name = get_sar_file_name(sar, args.name_format)
        print "Extracting %s to %s" % \
            (sar.id_name if sar.id_name is not None else ("%04X" % sar.id_num),
             file_name)
        SarFile(os.path.join(args.dir, file_name)).write(sar)


def command_print(args):
    """ Processes "print" command """
    if os.path.splitext(args.file)[1].lower() in [".c", ".cpp", ".h", ".hpp"]:
        sars = extract_selected_sars(args.file, get_h_file(args), args.id)
    else:
        sar = SarFile(args.file).read()
        h_file = get_h_file(args, allow_absence=True)
        if h_file is not None:
            sar.id_name = h_file.num_to_name(sar.id_num)
        sars = [sar]
    first = True
    for sar in sars:
        if not first:
            print ""
        first = False
        if sar.comment:
            print "# %s" % sar.comment
        print "Board ID: 0x%04X%s" % \
            (sar.id_num, (" (%s)" % sar.id_name) if sar.id_name else "")
        for subband_idx in SarData.sb_indices:
            limits = []
            for chain_idx in range(sar.chains()):
                limit = sar.sar_map.get(subband_idx, {}).get(chain_idx)
                limits.append(
                    "%5s" % ("None" if limit is None else ("%.2f" % limit)))
            limits_str = " ".join(limits)
            if args.by_channel:
                channels = sorted(
                    SarData.sb_channels.get(subband_idx, []) +
                    (SarData.sb_japan_channels.get(subband_idx, [])
                     if args.japan else []))
                for channel in channels:
                    print "%3d: %s" % (channel, limits_str)
            else:
                print "%s: %s" % (SarData.sb_name[subband_idx], limits_str)


def command_list(args):
    """ Processes "list" command """
    h_file = get_h_file(args)
    if args.file:
        name_num_pairs = []
        for sar in CFile(args.file).read():
            name_num_pairs.append(
                (sar.id_name if sar.id_name else
                 h_file.num_to_name(sar.id_num),
                 sar.id_num if sar.id_num is not None else
                 h_file.name_to_num(sar.id_name)))
    else:
        name_num_pairs = h_file.name_num_pairs()
    for name, num in name_num_pairs:
        print "%7s %s" % \
            (("0x%04X" % num) if num is not None else "Unknown",
             name if name else "Unknown")


def command_rename(args):
    """ Processes "rename" command """
    h_file = \
        get_h_file(args) if not args.name_format.startswith("hex") else None
    for file_name in get_file_list(args.file):
        sar = SarFile(file_name).read()
        if h_file is not None:
            sar.id_name = h_file.num_to_name(sar.id_num)
        new_name = os.path.join(
            os.path.dirname(file_name),
            get_sar_file_name(sar, args.name_format, add_extension=False) +
            os.path.splitext(file_name)[1])
        intermediate_name = file_name + ".sar_rename"
        if os.path.isfile(intermediate_name):
            error(("Can't rename \"%s\", because file with intermediate " +
                   "name \"%s\" exists (maybe it is remained from previous " +
                   "failed rename - so there might be useful content in it)") %
                  (file_name, intermediate_name))
        try:
            os.rename(file_name, intermediate_name)
            os.rename(intermediate_name, new_name)
            print "%s renamed to %s" % (os.path.basename(file_name),
                                        os.path.basename(new_name))
        except EnvironmentError as exc:
            error("Error renaming \"%s\": %s" % (file_name, exc.strerror))


def command_help(args, subparsers):
    """ Processes "help" command """
    if args.subcommand not in subparsers.choices:
        error("Unknown subcommand name \"%s\"" % args.subcommand)
    subparsers.choices[args.subcommand].print_help()

# =============================================================================
# Argument parsing


def main(argv):
    """ Parses arguments and invokes needed subcommand

    Arguments:
    argv -- List of command line arguments (without sys.argv[0]
    """
    # Declaration of --driver_svn and --driver_dir switches, used in many
    # subcommands
    parser_driver_source = ArgumentParser(add_help=False)
    group_driver_source = parser_driver_source.add_mutually_exclusive_group()
    group_driver_source.add_argument(
        "-s", "--driver_svn", metavar="BRANCH_OR_URL",
        help="Branch/twig name or full SVN URL of driver sources. If " +
        "neither this nor --driver_dir switch is specified, trunk is assumed")
    group_driver_source.add_argument(
        "-d", "--driver_dir", metavar="DIR",
        help="Root directory of driver sources")

    # Declaration of "--name_format" switch used in some commands
    parser_name_format = ArgumentParser(add_help=False)
    parser_name_format.add_argument(
        "-f", "--name_format", metavar="FORMAT",
        choices=["hex", "hex_lc", "name", "name_lc", "combined",
                 "combined_lc"],
        default="hex",
        help="How board ID used in SAR file names - as hexadecimal ID (hex)" +
        "as lowercase hexadecimal ID (hex_lc), as board name as specified " +
        "in SAR table in .c file (name), as board name as  specified in SAR " +
        "table in .c file lowercased (name_lc), as board name combined with " +
        "hexadecimal ID (combined), ditto lowercase (combined_lc)")

    # Declaration of "--id" switch used in some commands
    parser_id = ArgumentParser(add_help=False)
    parser_id.add_argument(
        "-i", "--id", metavar="ID(s)", action="append", default=[],
        help="Board ID(s) to extract. Comma or space separated. May be " +
        "specified as hex numbers or as names. May be specified several " +
        "times. If not specified - all SAR table entries are extracted")

    # Declaration of "--empty" switch used in some commands
    parser_empty = ArgumentParser(add_help=False)
    parser_empty.add_argument(
        "-e", "--empty", metavar="ID(s)", action="append", default=[],
        help="Board ID(s) to create empty (no-SAR-limit) SAR data. May be " +
        "specified as hex numbers or as names. May be specified several " +
        "times")

    # Top-level argument parser
    argument_parser = ArgumentParser(
        formatter_class=RawDescriptionHelpFormatter,
        description="Reads/writes SAR data from/to .c and .csv files. Also " +
        "provides some helper functionality",
        epilog="""Examples:
Print help on "create_c" command:
  %(prog)s help create_c

Create sar_data.c from BCM94350X52B.csv and 131.csv
  %(prog)s create_c BCM94350X52B.csv 131.csv sar_data.c
""")
    #argument_parser.add_argument(
    #    "--verbose", action="store_true", default=False,
    #    help="Prints detailed log of operation progress")
    subparsers = argument_parser.add_subparsers(
        dest="subcommands", metavar="SUBCOMMAND")

    # Subparser for "create_c" command
    parser_create_c = subparsers.add_parser(
        "create_c", parents=[parser_driver_source, parser_empty],
        add_help=False, formatter_class=RawDescriptionHelpFormatter,
        help="Creates separate .c with SAR data from given source .csv files",
        usage="%(prog)s [options] sar1.csv ... sar_table.c",
        epilog="""Example:
Create sar_data.c from BCM94350X52B.csv and 131.csv and empty table for
BCM94360X29CP3:
  %(prog)s --empty BCM94360X29CP3 BCM94350X52B.csv 131.csv sar_data.c
""")
    parser_create_c.add_argument(
        "files", metavar="SAR1.csv ... SAR_TABLE.c", nargs="+",
        help="SAR files to get data from, followed by .c file to create")
    parser_create_c.set_defaults(func=command_create_c)

    # Subparser for "update_c" command
    parser_update_c = subparsers.add_parser(
        "update_c", parents=[parser_driver_source, parser_empty],
        add_help=False, formatter_class=RawDescriptionHelpFormatter,
        help="Updates SAR tables within existing .c file (e.g. wlc_channel.c)",
        usage="%(prog)s [options] sar1.csv ... sar_table.c",
        epilog="""Example:
Add/update SAR table entries in wlc_channel.c using data from BCM94350X52B.csv
and 131.csv  and empty table for board id 0x013B:
  %(prog)s --empty 0x013B BCM94350X52B.csv 131.csv wlc_channel.c
""")
    parser_update_c.add_argument(
        "files", metavar="SAR1.csv ... SAR_TABLE.c", nargs="+",
        help="SAR files to get data from, followed by .c file to update")
    parser_update_c.set_defaults(func=command_update_c)

    # Subparser for "extract_sar" command
    parser_extract_sar = subparsers.add_parser(
        "extract_sar", add_help=False,
        formatter_class=RawDescriptionHelpFormatter,
        parents=[parser_driver_source, parser_name_format, parser_id],
        help="Extracts SAR data from given .c file to .csv file(s)",
        usage="%(prog)s [options] sar_table.c dir",
        epilog="""Examples:
Extract SAR data from wlc_channel.c to current directory, file names are
hexadecimal
  %(prog)s --name_format hex wlc_channel.c .

Extracts SAR data on BCM94350X52B and 0x131 boards from wlc_channel.c to
current directory
  %(prog)s --id "BCM94350X52B 131" wlc_channel.c .
""")
    parser_extract_sar.add_argument(
        "file", metavar="SAR_TABLE.C",
        help=".c file containing SAR table")
    parser_extract_sar.add_argument(
        "dir", metavar="DIR",
        help="Destination directory. If not specified, SAR data is printed")
    parser_extract_sar.set_defaults(func=command_extract_sar)

    # Subparser for "print" command
    parser_print = subparsers.add_parser(
        "print", parents=[parser_driver_source, parser_id],
        add_help=False, formatter_class=RawDescriptionHelpFormatter,
        help="Prints SAR data",
        usage="%(prog)s [options] filename",
        epilog="""Examples:
Prints SAR data from BCM94350X52B.csv, by channel number
  %(prog)s --by_channel BCM94350X52B.csv

Prints SAR data on BCM94350X52B and 0x00EF boards from wlc_channel.c to
current directory
  %(prog)s --id BCM94350X52B,ef wlc_channel.c .
""")
    parser_print.add_argument(
        "--by_channel", action="store_true", default=False,
        help="Print limits by channel number (default is by subband)")
    parser_print.add_argument(
        "--japan", action="store_true", default=False,
        help="Modifier to --by_channel - include Japan channel numbers")
    parser_print.add_argument(
        "file", metavar="FILENAME",
        help=".c or .csv file containing SAR data")
    parser_print.set_defaults(func=command_print)

    # Subparser for "list" command
    parser_list = subparsers.add_parser(
        "list", parents=[parser_driver_source], add_help=False,
        formatter_class=RawDescriptionHelpFormatter,
        help="List board IDs - available or contained in .c file",
        usage="%(prog)s [options] [filename]",
        epilog="""Examples:
Prints board types from trunk bcmdevs.h
  %(prog)s

Prints board types from Bison 7.10 bcmdevs.h
  %(prog)s --driver_svn BISON_BRANCH_7_10

Prints board types defined in given wlc_channel.c, using Board IDs defined in
bcmdevs.h of Bison 7.10
  %(prog)s --driver_svn BISON_BRANCH_7_10 wlc_channel.c
""")
    parser_list.add_argument(
        "file", metavar="FILENAME", nargs="?",
        help="Optional name of .c file containing SAR table (if not " +
        "specified all board types defined in bcmdevs.h are printed")
    parser_list.set_defaults(func=command_list)

    # Subparser for "rename" command
    parser_rename = subparsers.add_parser(
        "rename", parents=[parser_driver_source, parser_name_format],
        add_help=False,
        formatter_class=RawDescriptionHelpFormatter,
        help="Rename .csv SAR files to other naming convention",
        usage="%(prog)s [options] filenames",
        epilog="""Example:
Renames BCM94350X52B.csv to hexadecimal name convention (i.e. to 0116.csv)
  %(prog)s --name_format hex BCM94350X52B.csv
""")
    parser_rename.add_argument(
        "file", metavar="FILENAME(S)", nargs="+",
        help="names of .csv SAR files to rename")
    parser_rename.set_defaults(func=command_rename)

    # Subparser for "help" command
    parser_help = subparsers.add_parser(
        "help", add_help=False, formatter_class=RawDescriptionHelpFormatter,
        help="Detailed help on given subcommand",
        usage="%(prog)s subcommand",
        epilog="""Example:
Prints help on "extract_sar" command
  %(prog)s extract_sar
""")
    parser_help.add_argument(
        "subcommand", metavar="SUBCOMMAND", nargs="?",
        help="Name of subcommand to print help about (use " +
        "\"%(prog)s --help\" to get list of all subcommands)")

    if not argv:
        argument_parser.print_help()
        sys.exit(1)

    args = argument_parser.parse_args(argv)

    logging.basicConfig(
        format=os.path.basename(__file__) + ": %(levelname)s: %(message)s",
        # level=logging.INFO if args.verbose else logging.ERROR)
        level=logging.ERROR)

    if args.subcommands == "help":
        if args.subcommand is None:
            argument_parser.print_help()
        else:
            command_help(args, subparsers)
        sys.exit(1)

    args.func(args)


# Standalone module starter
if __name__ == "__main__":
    main(sys.argv[1:])
