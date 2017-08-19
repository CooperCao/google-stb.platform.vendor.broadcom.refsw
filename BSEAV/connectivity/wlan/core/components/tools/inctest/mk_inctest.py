#!/usr/bin/env python

# @file
# @brief
# Generate sources for include file test.

# $ Copyright Broadcom Corporation $

# <<Broadcom-WL-IPTag/Proprietary:>>

# $Id$

# Example:
# ./mk_inctest.py ../../../components/wlioctl/include/wlioctl.h

"""Prepare and run unit test of include file struct padding.

Include files are passed on the command line.
Environment variables:
 INCTEST_TOP, top checkout directory.
 INCTEST_BUILD, build directory.
"""

from contextlib import contextmanager
import logging
import os
import re
import subprocess
import sys

ENV_NAMES = (E_TOP, E_BUILD) = ('INCTEST_TOP', 'INCTEST_BUILD')
# {var_name:var_default, ...}
ENV_VARS = {E_TOP: '../../../', E_BUILD: 'inctest_build'}

GEN_NAMES = (
    G_ARRAY, G_UNPACKED, G_PACKED
) = (
    'array', 'unpacked', 'packed'
)
GEN_FILES = {G_ARRAY: 'inctest_arr.c',
             G_UNPACKED: 'inctest_unpacked.c',
             G_PACKED: 'inctest_packed.c', }
KEYWORDS = [
    'IHV_NIC_SPECIFIC_EXTENSION',
    'gtk_keyinfo_t',
    'wlc_rateset_t',
    'channel_info_t',
    'sta_info_ecounters_t',
    'wl_wowl_pattern_list_t',
    'wl_wr_type_t',
    'wl_radar_thr2_t',
    'wl_wowl_wakeind_t',
    'log_idstr_t',
    'wl_nan_sd_params_t',
    'wl_assoc_params_t',
    'wl_join_params_t',
    'wl_extjoin_params_t',
    'wl_country_list_t',
    'wl_rm_req_t',
    'pmkid_list_t',
    'pmkid_cand_list_t',
    'maclist_t',
    'wl_ioctl_t',
    'compat_wl_ioctl_t',
    'wl_txppr_t',
    'wl_awdl_action_frame_t',
    'awdl_peer_table_t',
    'awdl_peer_op_tbl_t',
    'wl_ptk_start_iov_t',
    'wl_nan_service_list_t',
    'wl_avail_entry_t',
    'wl_proxd_range_req_t',
    'rsdb_config_t',
    'chan_seq_tlv_data_t',
    'chan_seq_tlv_t',
    'sb_channel_sequence_t',
    'chanspec_list_t',
    'wl_dfs_forced_t',
    'wl_scan_params_t',
    'wl_iscan_params_t',
    'wl_escan_params_t',
    'wl_rm_rep_elt_t',
    'wl_rm_rep_t',
    'wl_p2po_wfds_seek_add_t',
    'wl_anqpo_set_t',
    'wl_keep_alive_pkt_t',
    'awdl_payload_t',
    'awdl_long_payload_t',
    'wl_awdl_pscan_params_t',
    'wl_pscan_params_t',
    'wl_ptk_start_tlv_t',
    'wl_nan_oob_af_params_t',
    'wl_wsec_info_tlv_t',
    'wl_wsec_info_t',
    'wl_randmac_tlv_t',
    'wl_randmac_event_t',
    'bcn_req_t',
    'wl_rrm_config_ioc_t',
    'wl_proxd_tlv_t',
    'wl_proxd_iov_t',
    'wl_proxd_event_t',
    'wlc_fbt_auth_resp_t',
    'wl_fbt_params_t',
    'rsdb_caps_response_t',
    'wlc_clm_power_limits_req_t',
    'BWL_POST_PACKED_STRUCT',
]

logging.basicConfig(
    format='%(asctime)s %(module)s:%(levelname)s %(message)s',
    datefmt='%Y%m%d.%H%M%S', level=logging.INFO)
logging.addLevelName(logging.INFO, '')
logging.addLevelName(logging.ERROR, ' Error:')


@contextmanager
def open_out(name):
    outf = open(name, 'w')

    def out(buf):
        outf.write(buf + '\n')

    try:
        yield out
    finally:
        outf.close()


def main():

    if len(sys.argv) <= 1:
        print '%s Error: Need at least one arg.' % sys.argv[0]
        return 1

    # Get environment variables or set defaults.
    var_vals = dict([(var, os.environ.get(var, dflt))
                     for var, dflt in ENV_VARS.items()])
    build = var_vals[E_BUILD]
    if not os.path.exists(build):
        os.makedirs(build)
    # Set paths of generated files.
    gen_paths = dict([(gen_name, os.path.join(build, gen_file))
                      for gen_name, gen_file in GEN_FILES.items()])

    # Walk thru the argument include files.
    for subject in sys.argv[1:]:
        with open(subject) as inf:

            def select_line(line):
                """Filter the lines."""

                if line.startswith('} '):
                    for kw in KEYWORDS:
                        if kw in line:
                            return
                    line = line.replace('} ', '')
                    line = re.sub(r';$', '', line)
                    return line

            with open_out(gen_paths[G_ARRAY]) as out:
                out("""\
#include<inctest.h>
char inc_test_arr[][100] = {""")
                for line in inf:
                    line = select_line(line)
                    if line:
                        out('"%s",' % line.strip())
                out('};')
                out("""\
char * get_str_name(uint32 i) {
    return inc_test_arr[i];
}""")
            inf.seek(0)
            with open_out(gen_paths[G_UNPACKED]) as out:
                out("""\
#include<inctest.h>
uint32 inc_test_arr_unpacked[] = {""")
                for line in inf:
                    line = select_line(line)
                    if line:
                        out('sizeof(%s),' % line.strip())
                out('};')

            inf.seek(0)
            with open_out(gen_paths[G_PACKED]) as out:
                out("#pragma pack(1)")
                with open(gen_paths[G_UNPACKED]) as inf:
                    for line in inf:
                        line = line.replace(
                            'inc_test_arr_unpacked',
                            'inc_test_arr_packed')
                        out(line)
                out("""\
uint32 get_arr_size() {
    return sizeof(inc_test_arr_packed)/sizeof(uint32);
}""")

    # Pass environment variables to make as name=val.
    set_vars = ['='.join((var, val.rstrip(os.sep) + os.sep))
                for var, val in var_vals.items()]

    def run(args):
        logging.info('Running %s', ' '.join(args))
        subprocess.check_call(args)

    try:
        run(['make'] + set_vars + ['test'])
    except EnvironmentError as err:
        logging.error(str(err))
        return 3
    except subprocess.CalledProcessError as err:
        # Error messages printed by children, nothing to add here.
        return err.returncode

if __name__ == '__main__':
    sys.exit(main())
