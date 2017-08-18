#!/usr/bin/env python2.7
"""
This is a script to copy and rename Broadcom firmware/nvram/clm files from
Broadcom internal naming structure to Olympic defined file names described
in "Specification for Naming Firmware/NVRAM/CLM Files  ver 0.4". See
http://confluence.broadcom.com/pages/viewpage.action?pageId=220859696
for internal documentation.
"""

#
# $Copyright Broadcom Corporation$
#
# $Id$

from __future__ import print_function
from __future__ import unicode_literals

from collections import OrderedDict
from collections import defaultdict
from collections import Mapping
import argparse
import copy
import multiprocessing
import os
import sys

import yaml
import yaml.constructor

# Assuming the script will always run by firmware developer makefile
# which is located at src/dongle/rte/wl (otherwise cd to that folder to run
# this script)
RLS_DIR = None

# Format string for writing out an intelligent copy command.
# Create intermediate directories
# Doesn't overwrite unnecessarily
# Sets file mode to a reasonable default
# Verbosely shows if file was copied
INSTALL = '\t@install -CDv -m$(mode) %s %s\n'


# Data for generating makefile
def dict_factory():
    """Create a default dict."""
    return defaultdict()


def nested_dict_factory():
    """Create a nested default dict."""
    return defaultdict(dict_factory)


def dict_factoryl():
    """Create a default dict of lists."""
    return defaultdict(list)


def nested_dict_factoryl():
    """Create a nested default dict of lists."""
    return defaultdict(dict_factoryl)


PLATFORMS = nested_dict_factory()
CONFIGS = nested_dict_factoryl()
BINS = nested_dict_factory()
NVRAMS = nested_dict_factory()
DUPES = nested_dict_factoryl()
ROMLSRC = {}


def fatal(message):
    """Print an error message in standard format."""

    sys.stderr.write('%s: Error: %s\n' % (os.path.basename(__file__), message))
    sys.exit(2)


def create_makefile(opts):
    """Create a makefile."""

    mkfile = opts.makefile
    mkf = []

    mkf.append("""#
# Generated makefile - do not edit. Created by command:
# {cmdline}

define usage
Usage: make -f {mkfile} -jN [<target>...]
Where <target> can be a mixture of:
  <program>: [{platforms}]
  or <chip>: [{configs}]
  or any supported combinations of <chip>.<program>
  or "all".

EXAMPLE: make -f {mkfile} -j16 {platform1}
EXAMPLE: make -f {mkfile} -j16 {config1}
EXAMPLE: make -f {mkfile} -j16 {config1}.{platform1}
endef

ifneq (,$(filter help,$(MAKECMDGOALS)))
  $(info $(usage))
  .PHONY: help
  help:; @:
endif

MAKECMDGOALS ?= all

""".format(
        cmdline=' '.join(sys.argv),
        configs=' '.join(sorted(CONFIGS)),
        config1=sorted(CONFIGS)[0],
        mkfile=mkfile,
        platforms=' '.join(sorted(PLATFORMS)),
        platform1=sorted(PLATFORMS)[0],))

    mkf.append('.PHONY: all\n')
    mkf.append('all: ')
    for pl in sorted(PLATFORMS):
        mkf.append('%s ' % pl)

    mkf.append("""

.PHONY: check
check: CHECK := 1
check: all

mwd := $(dir $(lastword $(MAKEFILE_LIST)))
blddir := $(mwd)../../../../build

mode := 644

# Build up the full list of required firmware by examining targets.
req_images :=
""")

    all_fws = set()

    for pl in sorted(PLATFORMS):
        mkf.append('\n# %s-related targets\n' % pl)
        for chip in sorted(CONFIGS):
            fws = []
            for fw in CONFIGS[chip]:
                if pl in CONFIGS[chip][fw]:
                    all_fws.add(fw)
                    fws.append(fw)
            if not fws:
                continue
            fws = sorted(fws)
            mkf.append('ifneq (,$(filter all check %s %s %s.%s,$(%s)))\n' %
                       (pl, chip, chip, pl, 'MAKECMDGOALS'))
            mkf.append('  req_images += \\\n')
            for fw in fws:
                mkf.append('    %s' % fw)
                if fw != fws[-1]:
                    mkf.append(' \\')
                mkf.append('\n')
            mkf.append('endif\n')

    mkf.append('\n')

    # Dedupe (Typically roml.* files common to multiple platforms)
    mkf.append('# Files common to multiple targets, only copy once\n')
    for chip in sorted(DUPES):
        for dest in DUPES[chip].keys():
            if len(DUPES[chip][dest]) < 2:
                del DUPES[chip][dest]
        mkf.append('.PHONY: %s.common-files\n' % chip)
        mkf.append('%s.common-files: \\\n' % chip)
        for dest in sorted(DUPES[chip]):
            mkf.append('  %s \\\n' % dest)
        mkf[-1] = mkf[-1][:-3]
        mkf.append('\n')
        for dest in sorted(DUPES[chip]):
            mkf.append('\n%s: \\\n' % dest)
            if not opts.romldir:
                mkf.append('  %s\n' % DUPES[chip][dest][-1])
            else:
                chiprev = chip.lstrip('C-').replace('__s-', '').lower()
                if ROMLSRC[chiprev]:
                    chiprev = ROMLSRC[chiprev]
                mkf.append('  %s\n' % os.path.join(
                    opts.romldir, chiprev,
                    os.path.basename(DUPES[chip][dest][-1])))
            mkf.append(INSTALL % ('$<', '$@'))
        mkf.append('\n')

    mkf.append('# Do everything needed for a specific chip + product\n')
    for target in sorted(BINS):
        chip = target.partition('.')[0]
        mkf.append('.PHONY: {0}\n'.format(target))
        # skip nvram if not a target
        mkf.append(str(
            '{0}: fwbuild {0}-nvram {1}.common-files\n' if target in NVRAMS else
            '{0}: fwbuild {1}.common-files\n').format(target, chip))
        for dest in sorted(BINS[target]):
            if dest not in DUPES[chip]:
                mkf.append(INSTALL % (BINS[target][dest], dest))
        mkf.append('\n')

    mkf.append('# nvram targets\n')
    for target in sorted(NVRAMS):
        mkf.append('.PHONY: {0}-nvram\n'.format(target))
        mkf.append('{0}-nvram:\n'.format(target))
        for dest in sorted(NVRAMS[target]):
            mkf.append(INSTALL % (NVRAMS[target][dest], dest))
        mkf.append('\n')

    mkf.append('# Typical targets - by platform (all chips)\n')
    for p in sorted(PLATFORMS):
        mkf.append('.PHONY: %s\n' % p)
        mkf.append('%s: ' % p)
        for target in sorted(PLATFORMS[p]):
            mkf.append('%s ' % PLATFORMS[p][target])
        mkf.append('\n\n')

    mkf.append('# Alternate targets by chips (all platforms)\n')
    for chip in sorted(CONFIGS):
        mkf.append('.PHONY: %s\n' % chip)
        mkf.append('%s: ' % chip)
        mkf.append(' '.join(sorted(set(['{0}.{1}'.format(chip, p)
                                       for cfg in CONFIGS[chip]
                                       for p in CONFIGS[chip][cfg]]))))
        mkf.append('\n\n')

        makecmd = '@echo Would run: make' if opts.skip_fw else '$(MAKE)'

    mkf.append('all_images :=')
    for img in sorted(all_fws):
        mkf.append(' \\\n  %s' % img)
    mkf.append('\n')

    mkf.append("""
req_images := $(sort $(req_images))

# The HOSTMAKE construct can be used to switch strategically between
# distributed and local make variants. By default it's a nop.
HOSTMAKE ?= $(MAKE)

.PHONY: $(all_images)
$(all_images): MAKE := $(HOSTMAKE)
$(all_images):
\t{makecmd} OLYMPIC=1 $@

.PHONY: images
images: $(all_images)

.PHONY: print_image_list
print_image_list:
\t@for i in $(all_images); do echo $$i; done

# This makefile supports two different kinds of parallelism.
# In one mode, images are built one at a time and parallelism
# occurs within each image build. In the other, all image builds
# are spun off in parallel and the GNU make "job server" takes
# responsibility for preventing a combinatorial explosion of
# jobs. The serial-parallel style produces more readable output
# and is the default, whereas full-parallel mode works better with
# distributed-make variants (and might be somewhat faster even on
# a single host) so may be preferred when speed is paramount.
.PHONY: fwbuild
ifdef CHECK
  fwbuild: MAKE := $(strip $(HOSTMAKE) $(MAKEJOBS))
  fwbuild: $(req_images)
else
  .NOTPARALLEL:
  fwbuild:
\t$(MAKE) OLYMPIC=1 $(req_images)
endif

.PHONY: clean
clean:
\t$(RM) -r $(blddir)

.PHONY: git-clean
git-clean:
\tgit clean -dfx -e '*.log' -e '*.mk' $(mwd)../../../..
\tgit co $(blddir)
\tgit status --ignored --short
""".format(makecmd=makecmd))

    with open(mkfile, 'w') as f:
        for line in mkf:
            f.write(line)


# pylint: disable=too-many-ancestors
class ODLoader(yaml.Loader):
    """
    A YAML loader that loads mappings into ordered dictionaries.
    Derived from http://stackoverflow.com question #5121931.
    """

    MAP_TAG = u'tag:yaml.org,2002:map'
    OMAP_TAG = u'tag:yaml.org,2002:omap'

    def __init__(self, *args, **kwargs):
        self.add_constructor(ODLoader.MAP_TAG, type(self).construct_yaml_map)
        self.add_constructor(ODLoader.OMAP_TAG, type(self).construct_yaml_map)
        super(ODLoader, self).__init__(*args, **kwargs)

    def construct_yaml_map(self, node):
        data = OrderedDict()
        yield data
        value = self.construct_mapping(node)
        data.update(value)

    def construct_mapping(self, node, deep=False):
        if isinstance(node, yaml.MappingNode):
            self.flatten_mapping(node)
        else:
            warning = 'expected a mapping node, but found %s' % node.id
            raise yaml.constructor.ConstructorError(None, None, warning,
                                                    node.start_mark)

        mapping = OrderedDict()
        for key_node, value_node in node.value:
            key = self.construct_object(key_node, deep=deep)
            try:
                hash(key)
            except TypeError as exc:
                static_warning = 'while constructing a mapping'
                warning = 'found unacceptable key (%s)' % exc
                raise yaml.constructor.ConstructorError(static_warning,
                                                        node.start_mark,
                                                        warning,
                                                        key_node.start_mark)
            value = self.construct_object(value_node, deep=deep)
            mapping[key] = value
        return mapping


def ensure_copy(chip, data, target, frm, to):
    """Collect data."""

    # Copy from original file, not symlink like used for roml.*
    # otherwise install always copies.
    frm_real = os.path.relpath(os.path.realpath(frm))
    data[target][to] = frm_real
    DUPES[chip][to].append(frm_real)


def read_hardware(hw_list):
    """Read list of values from "hardware:" key, return chip_id,
    chip_version values."""

    chip_id = chip_version = romlsrc = ''

    for hw in hw_list:
        if (hw.keys())[0].lower().strip() == 'chip_id':
            chip_id = str((hw.values())[0]).strip()
        if (hw.keys())[0].lower().strip() == 'chip_version':
            chip_version = str((hw.values())[0]).strip()
        # get default roml source if any
        if hw.keys()[0].lower().strip() == 'roml':
            romlsrc = str(hw.values()[0]).strip()

    assert chip_id
    assert chip_version

    return chip_id, chip_version, romlsrc


def read_modules(module_list):
    """Read list of values from "module:" key."""

    nvram = rev = module_name = vendor = append = ''

    for module in module_list:
        if (module.keys())[0].lower().strip() == 'nvram':
            nvram = str((module.values())[0]).strip()
        if (module.keys())[0].lower().strip() == 'rev':
            rev = str((module.values())[0]).strip()
        if (module.keys())[0].lower().strip() == 'name':
            module_name = str((module.values())[0]).strip()
        if (module.keys())[0].lower().strip() == 'vendor':
            vendor = str((module.values())[0]).strip()
        if (module.keys())[0].lower().strip() == 'append':
            append = str((module.values())[0]).strip()

    assert nvram
    assert rev
    assert module_name
    assert vendor
    return nvram, rev, module_name, vendor, append


def construct_chip_name(chip_id, chip_version):
    """Construct an Olympic name."""

    return 'C-%s__s-%s' % (chip_id, chip_version.upper())


def construct_clmb_name(oem, program, chip_id, chip_version):
    """Construct a clmb name."""

    return '%s/%s/C-%s__s-%s/%s.clmb' % (
        RLS_DIR, oem, chip_id, chip_version.upper(), program.strip())


def construct_txcb_name(oem, program, chip_id, chip_version):
    """Construct a txcb name."""

    return '%s/%s/C-%s__s-%s/%s.txcb' % (
        RLS_DIR, oem, chip_id, chip_version.upper(), program.strip())


def construct_fw_name(oem, program, chip_id, chip_version):
    """Construct a fw name."""

    return '%s/%s/C-%s__s-%s/%s.trx' % (
        RLS_DIR, oem, chip_id, chip_version.upper(), program.strip())


def construct_other_fw_name(oem, program, chip_id, chip_version, other):
    """Construct name for other/extra firmware."""

    return '%s/%s/C-%s__s-%s/%s/%s.trx' % (
           RLS_DIR, oem, chip_id, chip_version.upper(), other, program.strip())


def construct_logstr_name(oem, program, chip_id, chip_version):
    """Construct a logstr name."""

    return '%s/%s/C-%s__s-%s/%s_logstrs.bin' % (
        RLS_DIR, oem, chip_id, chip_version.upper(), program[0:4].strip())


def construct_additional_name(oem, program, chip_id, chip_version,
                              dirpath, fname):
    """Construct file name for additional files."""

    path_name = '%s/%s/C-%s__s-%s/%s/' % (RLS_DIR, oem, chip_id,
                                          chip_version.upper(), dirpath)

    if fname == 'logstrs.bin':
        additional_name = path_name + program + '_' + fname
    else:
        base, ext = os.path.splitext(fname)
        if base == 'rtecdc':
            additional_name = path_name + program + ext
        else:
            additional_name = path_name + fname

    return additional_name


def construct_nvram_name(oem, program, chip_id, chip_version,
                         rev, module_name, vendor, append):
    """Construct a nvram name."""

    return '%s/%s/C-%s__s-%s/P-%s_M-%s_V-%s__m-%s%s.txt' % (
        RLS_DIR, oem, chip_id, chip_version.upper(),
        program.strip(), (module_name[:4]).upper(),
        vendor[0].lower(), rev, append)


def construct_ddir_name(oem, chip_id, chip_version):
    """Construct a ddir name."""

    return '%s/%s/C-%s__s-%s' % (
        RLS_DIR, oem, chip_id, chip_version.upper())


def process_config(odict, chips_ary,
                   binsrcdir, nvram_dir, blob_dir):
    """Process config file data in ordered dictionary."""

    firmware_dir = binsrcdir
    clmb_dir = txcb_dir = blob_dir

    mapping = odict.get('path_name_mapping', {})
    for k1 in mapping:
        # Yaml level 1 - OEM
        oem = k1.strip()
        for k2 in mapping[k1]:
            # Yaml level 2 - Program
            program = k2.strip()
            for k3 in mapping[k1][k2]:
                # Yaml level 3 - chip_id + chip_version
                chip_info = k3.strip()
                for k4 in mapping[k1][k2][k3]:
                    # Yaml level 4 - hardware/firmware/clmb/txcb/modules
                    swhw = k4
                    if swhw == 'hardware':
                        hw_list = mapping[k1][k2][k3][k4]
                        chip_id, chip_version, romlsrc = \
                            read_hardware(hw_list)
                        chiprev = chip_id + chip_version.rsplit('-')[0]
                        ROMLSRC[chiprev] = romlsrc
                        chip_info_cmp = chip_id + chip_version
                        if chip_info != chip_info_cmp:
                            fatal('program %s, chip %s mismatch with'
                                  ' chip id %s and chip version %s\n' % (
                                      program, chip_info,
                                      chip_id, chip_version))

                        chip_ver_ary = chip_version.split('-')
                        chip_1 = chip_id + chip_ver_ary[0]

                        # if --chip is used, only package those chips
                        # otherwise, package everything in config file
                        chip_found = False
                        if len(chips_ary) > 0:
                            chip_found = False
                            for chip_2 in chips_ary:
                                if chip_1 == chip_2:
                                    chip_found = True
                                    break
                        else:
                            chip_found = True

                        if not chip_found:
                            break

                        # exception case for chip id 43451
                        if chip_id == '43451':
                            chip_id = '4345'

                        # exception case for chip id 43342a0
                        elif chip_id == '43342' and chip_ver_ary[0] == 'a0':
                            chip_ver_ary[0] = 'a1'

                        # exception case for chip 4350.
                        # c2 and c4 are identical;
                        # copy artifacts from c2 into c4.
                        elif chip_id == '4350' and chip_ver_ary[0] == 'c4':
                            chip_info = chip_info.replace('4350c4', '4350c2')

                        # exception case for chip id 43452c1-roml
                        elif (chip_id == '43452' and
                              chip_version == 'c1-roml'):
                            chip_id = '43451'
                            chip_version = 'c0-roml'
                            chip_ver_ary = chip_version.split('-')

                        # exception case for chip id 43435
                        elif chip_id == '43435':
                            chip_id = '43430'

                        oly_chip_name = construct_chip_name(chip_id,
                                                            chip_ver_ary[0])
                        chip_and_program = oly_chip_name + '.' + program
                        PLATFORMS[program][oly_chip_name] = chip_and_program

                    elif swhw == 'other-firmware':
                        other_fws = mapping[k1][k2][k3][k4]
                        if other_fws:
                            for other_fw in other_fws:
                                fw_list = other_fws[other_fw]
                                oly_other_fw_name = construct_other_fw_name(
                                    oem, program, chip_id, chip_ver_ary[0],
                                    other_fw)
                                bcm_other_fw_name = '%s/%s/%s' % (
                                    firmware_dir, chip_info, fw_list[0])
                                ensure_copy(oly_chip_name, BINS,
                                            chip_and_program,
                                            bcm_other_fw_name,
                                            oly_other_fw_name)
                                # Store additional info for makefile
                                bcm_fw_tgt = os.path.join(
                                    chip_info, fw_list[0].partition('/')[0])
                                CONFIGS[oly_chip_name][bcm_fw_tgt].append(
                                    program)
                                # Copy debug files
                                try:
                                    for f in (odict['additional-files']
                                                   ['firmware']['debug']):
                                        oly_additional_name = \
                                            construct_additional_name(
                                                oem, program, chip_id,
                                                chip_ver_ary[0],
                                                other_fw + '/debug', f)
                                        bcm_additional_name = '%s/%s/%s/%s' % (
                                            firmware_dir,
                                            chip_info,
                                            os.path.dirname(fw_list[0]),
                                            f)
                                        ensure_copy(oly_chip_name, BINS,
                                                    chip_and_program,
                                                    bcm_additional_name,
                                                    oly_additional_name)
                                except (KeyError, TypeError):
                                    print('    no additional debug files '
                                          'to be packaged')
                    elif swhw == 'firmware':
                        fw_list = mapping[k1][k2][k3][k4]
                        oly_fw_name = construct_fw_name(
                            oem, program, chip_id, chip_ver_ary[0])
                        bcm_fw_name = '%s/%s/%s' % (
                            firmware_dir, chip_info, fw_list[0])
                        ensure_copy(oly_chip_name, BINS, chip_and_program,
                                    bcm_fw_name, oly_fw_name)

                        # Store additional info for makefile
                        bcm_fw_tgt = os.path.join(
                            chip_info, fw_list[0].partition('/')[0])
                        CONFIGS[oly_chip_name][bcm_fw_tgt].append(program)

                        # copy additional files
                        try:
                            files = odict['additional-files']['firmware']
                        except (KeyError, TypeError):
                            print('    logstr files to be packaged')
                            oly_logstr_name = construct_logstr_name(
                                oem, program, chip_id, chip_ver_ary[0])
                            bcm_logstr_name = '%s/%s/%s/logstrs.bin' % (
                                firmware_dir,
                                chip_info,
                                os.path.dirname(str(fw_list[0])))
                            ensure_copy(oly_chip_name, BINS, chip_and_program,
                                        bcm_logstr_name, oly_logstr_name)
                        else:
                            for d in files:
                                try:
                                    for f in (odict['additional-files']
                                                   ['firmware'][d]):
                                        oly_additional_name = \
                                            construct_additional_name(
                                                oem, program, chip_id,
                                                chip_ver_ary[0], d, f)
                                        bcm_additional_name = '%s/%s/%s/%s' % (
                                            firmware_dir,
                                            chip_info,
                                            os.path.dirname(fw_list[0]),
                                            f)
                                        ensure_copy(oly_chip_name, BINS,
                                                    chip_and_program,
                                                    bcm_additional_name,
                                                    oly_additional_name)
                                except (KeyError, TypeError):
                                    print('    no additional %s files to be '
                                          'packaged' % d)
                        try:
                            for variant in (odict['additional-files']
                                                 ['optional-firmware']):
                                fw_list = (odict['additional-files']
                                                ['optional-firmware'][variant])
                                bcm_firmware_name = '%s/%s/%s' % (
                                    firmware_dir, chip_info, fw_list[0])
                                if not os.path.exists(bcm_firmware_name):
                                    print('%s not found' % bcm_firmware_name)
                                    continue
                                oly_other_fw_name = construct_other_fw_name(
                                    oem, program, chip_id, chip_ver_ary[0],
                                    variant)
                                ensure_copy(oly_chip_name, BINS,
                                            chip_and_program,
                                            bcm_firmware_name,
                                            oly_other_fw_name)
                        except (KeyError, TypeError):
                            pass
                    elif swhw == 'clmb':
                        clmb_list = mapping[k1][k2][k3][k4]
                        oly_clmb_name = construct_clmb_name(
                            oem, program, chip_id, chip_ver_ary[0])
                        if clmb_dir:
                            bcm_clmb_name = '%s/%s' % (
                                clmb_dir, clmb_list[0].partition('/')[2])
                        else:
                            bcm_clmb_name = '%s/%s/%s' % (
                                firmware_dir, chip_info, clmb_list[0])
                        ensure_copy(oly_chip_name, BINS, chip_and_program,
                                    bcm_clmb_name, oly_clmb_name)
                    elif swhw == 'txcb':
                        txcb_list = mapping[k1][k2][k3][k4]
                        oly_txcb_name = construct_txcb_name(
                            oem, program, chip_id, chip_ver_ary[0])
                        if txcb_dir:
                            bcm_txcb_name = '%s/%s' % (
                                txcb_dir, txcb_list[0].partition('/')[2])
                        else:
                            bcm_txcb_name = '%s/%s/%s' % (
                                firmware_dir, chip_info, txcb_list[0])
                        ensure_copy(oly_chip_name, BINS, chip_and_program,
                                    bcm_txcb_name, oly_txcb_name)
                    elif swhw == 'modules':
                        for k5 in mapping[k1][k2][k3][k4]:
                            module_list = mapping[k1][k2][k3][k4][k5]

                            nvram, rev, module_name, vendor, append = \
                                read_modules(module_list)
                            oly_nvram_name = construct_nvram_name(
                                oem, program, chip_id,
                                chip_ver_ary[0], rev, module_name,
                                vendor, append)
                            bcm_nvram_name = nvram_dir + '/' + nvram
                            ensure_copy(oly_chip_name, NVRAMS,
                                        chip_and_program, bcm_nvram_name,
                                        oly_nvram_name)


def dict_merge(a, b):
    """Recursively combine two data structures with dicts at the base."""
    if not isinstance(b, dict):
        return b
    for k, v in b.iteritems():
        if k in a and isinstance(a[k], Mapping):
            a[k] = dict_merge(a[k], v)
        else:
            a[k] = copy.deepcopy(v)
    return a


def main():
    """Main entry point."""

    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-C', '--directory',
        default='.',
        help="change to DIRECTORY before doing anything")
    parser.add_argument(
        '-b', '--binsrcdir',
        default='builds',
        help="Specify source root dir for FW, clmb and txcb files")
    parser.add_argument(
        '-c', '--chip',
        default='',
        help="Specify chip(s) to package")
    parser.add_argument(
        '-l', '--blobdir',
        help="Override source dir for clmb and txcb files")
    parser.add_argument(
        '-m', '--makefile',
        default='olympic.mk',
        help="Generate the specified makefile")
    parser.add_argument(
        '-n', '--nvramsrcdir',
        default='../../../shared/nvram',
        help="Specify source root dir for nvram files")
    parser.add_argument(
        '-o', '--outputdir',
        default='../../../../release',
        help="Specify output dir")
    parser.add_argument(
        '-r', '--romldir',
        help="Specify roml dir")
    parser.add_argument(
        '-t', '--targets',
        metavar='TGTS',
        help="Run make on generated makefile using TGTS")
    parser.add_argument(
        '-y', '--yaml', default=[], action='append',
        help="Specify yaml config file(s)")
    parser.add_argument(
        '--skip-fw', action='store_true',
        help=argparse.SUPPRESS)
    opts = parser.parse_args()

    os.chdir(opts.directory)

    global RLS_DIR  # pylint: disable=global-statement
    RLS_DIR = opts.outputdir

    if not opts.yaml:
        opts.yaml = [os.path.join(os.path.dirname(__file__),
                                  'rename_fw_cfg.yaml')]

    odict = OrderedDict()
    for path in opts.yaml:
        with open(path) as f:
            doc = yaml.load(f, ODLoader)
            odict = dict_merge(odict, doc)

    process_config(odict, opts.chip.split(),
                   opts.binsrcdir, opts.nvramsrcdir, opts.blobdir)

    create_makefile(opts)
    if opts.targets:
        jobs = multiprocessing.cpu_count()
        mdir, mfile = os.path.split(opts.makefile)
        if mdir:
            mkcmd = 'make -C %s -f %s -j%d %s' % (
                mdir, mfile, jobs, opts.targets)
        else:
            mkcmd = 'make -f %s -j%d %s' % (
                mfile, jobs, opts.targets)
        cmd = ['bash', '-xc', mkcmd]
        os.execvp(cmd[0], cmd)


if __name__ == '__main__':
    if len(sys.argv) == 1:
        sys.argv.append('-h')
    sys.exit(main())

# vim: ts=8:sw=4:tw=80:et:
