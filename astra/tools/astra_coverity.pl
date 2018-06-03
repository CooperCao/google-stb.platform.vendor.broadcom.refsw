#!/usr/bin/env perl
##############################################################################
#  Copyright (C) 2018 Broadcom.
#  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to
#  the terms and conditions of a separate, written license agreement executed
#  between you and Broadcom (an "Authorized License").  Except as set forth in
#  an Authorized License, Broadcom grants no license (express or implied),
#  right to use, or waiver of any kind with respect to the Software, and
#  Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
#  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
#  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization,
#  constitutes the valuable trade secrets of Broadcom, and you shall use all
#  reasonable efforts to protect the confidentiality thereof, and to use this
#  information only in connection with your use of Broadcom integrated circuit
#  products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
##############################################################################
use strict;
use warnings;
use File::Basename;
use Data::Dumper;
use Getopt::Long;
use Cwd;

my $CMD_BASE = basename $0;

# TZ vars are obtained from env vars if defined
my $TZ_ARCH      = $ENV{TZ_ARCH};

my $TZ_TOOLCHAIN = $ENV{TZ_TOOLCHAIN};
my $TZ_ARMGNU    = $ENV{TZ_ARMGNU};

my $TZ_TOP       = $ENV{TZ_TOP};
my $TZ_OBJ_TOP   = $ENV{TZ_OBJ_TOP};

my $TZ_MOD       = $ENV{TZ_MOD};
my $TZ_DIR       = $ENV{TZ_DIR};
my $TZ_OBJ_DIR   = $ENV{TZ_OBJ_DIR};

my @TZ_COMPILERS = ("gcc", "g++");

# Coverity vars are obtained from env vars if defined
my $COV_BIN_DIR  = $ENV{COV_BIN_DIR};
my $COV_USER_DIR = $ENV{COV_USER_DIR};
my $COV_OPTIONS  = $ENV{COV_OPTIONS};

my $COV_EMIT_DIR   = $ENV{COV_EMIT_DIR};
my $COV_HTML_DIR   = $ENV{COV_HTML_DIR};
my $COV_CONFIG_XML = $ENV{COV_CONFIG_XML};

my @COV_BIN_DIRS = (
    "/tools/bin",
    "/tools/cov-analysis-linux64-8.7.1/bin",
    "/tools/cov-analysis-linux64-8.0.0/bin",
    "/bsec/local/tools/coverity/cov-analysis-linux64-8.7.1/bin");

my $COV_OPTIONS_DEFAULT =
    "--all " .
    "--aggressiveness-level high " .
    "--enable-fnptr ";

sub usage
{
    print "\n";
    print "USAGE:\n";
    print "    $CMD_BASE [OPTION]... [MODULE]\n";
    print "\n";
    print "DESCRIPTION:\n";
    print "    Runs Coverity on the Astra module.\n";
    print "\n";
    print "OPTIONS:\n";
    print "    -h      Show help message.\n";
    print "\n";
    print "ENV VARIABLES:\n";
    print "    The TZ build environment invoked by this script is controlled\n";
    print "    by the same environment variables as in the regular TZ build.\n";
    print "\n";
    print "    The following environment variables specify the directories\n";
    print "    and options of the Coverity tools:\n";
    print "\n";
    print "    COV_BIN_DIR      The full path of the Coverity bin directory.\n";
    print "                     If it is unset, $CMD_BASE looks for Coverity\n";
    print "                     in PATH or commonly used tools dirs.\n";
    print "\n";
    print "    COV_USER_DIR     The Coverity output directory. The default is\n";
    print "                     \$TZ_OBJ_TOP/coverity.\n";
    print "\n";
    print "    COV_OPTIONS      The options given to the Coverity analyze tool.\n";
    print "                     If it is unset, the built-in options are used.\n";
    print "\n";
    print "    The following environment variables further specify the Coverity\n";
    print "    output directories for each types of derived files. If they are\n";
    print "    unset, the files are put under the Coverity user directory:\n";
    print "\n";
    print "    COV_EMIT_DIR     The Coverity intermediate emit repository.\n";
    print "    COV_HTML_DIR     The Coverity output HTML directory.\n";
    print "    COV_CONFIG_XML   The Coverity config XML file.\n";
    print "\n";
    exit(1);
}


sub shell
{
    my ($cmd_line) = @_;
    my @cmd_path = split ' ', $cmd_line;
    my $cmd_base = basename $cmd_path[0];

    printf "\n$CMD_BASE: $cmd_line\n";
    system($cmd_line);
    die "$CMD_BASE: Error: shell command $cmd_base failed!\n" if ($? >> 8);
}


sub tz_build_env
{
    my $ARCH_DIR;

    # The following logic is taken from astra/build/build_env.mk,
    # only modified to use perl syntax.

    # TZ arch is derived from refsw arch if defined
    if (defined $ENV{B_REFSW_ARCH}) {
        if ($ENV{B_REFSW_ARCH} eq 'arm-linux') {
            $TZ_ARCH = $TZ_ARCH || 'Arm32';
        } elsif ($ENV{B_REFSW_ARCH} eq 'aarch64-linux') {
            $TZ_ARCH = $TZ_ARCH || 'Arm64';
        }
    }

    # TZ arch defaults to 64-bit ARM
    $TZ_ARCH = $TZ_ARCH || 'Arm64';

    # TZ arch specific defines
    if ($TZ_ARCH eq 'Arm32') {
        # 32-bit ARM defines
        $TZ_TOOLCHAIN = $TZ_TOOLCHAIN ||
            "/opt/toolchains/tzos/DSO/gcc-arm-tzos-musl-5.3";
        $TZ_ARMGNU = $TZ_ARMGNU ||
            "$TZ_TOOLCHAIN/bin/arm-tzos-musleabi";
        $ARCH_DIR = 'arm';
    } else {
        # 64-bit ARM defines
        $TZ_TOOLCHAIN = $TZ_TOOLCHAIN ||
            "/opt/toolchains/tzos/DSO/gcc-aarch64-tzos-musl-5.3";
        $TZ_ARMGNU = $TZ_ARMGNU ||
            "$TZ_TOOLCHAIN/bin/aarch64-tzos-musleabi";
        $ARCH_DIR = 'aarch64';
    }

    # TZ obj top is derived from refsw obj root if defined
    if (defined($ENV{B_REFSW_OBJ_ROOT})) {
        $TZ_OBJ_TOP = $TZ_OBJ_TOP ||
            "$ENV{B_REFSW_OBJ_ROOT}/astra";
    } elsif (defined($ENV{B_REFSW_OBJ_DIR})) {
        $TZ_OBJ_TOP = $TZ_OBJ_TOP ||
            "$TZ_TOP/../$ENV{B_REFSW_OBJ_DIR}/astra";
    }

    # TZ obj top default
    $TZ_OBJ_TOP = $TZ_OBJ_TOP ||
        "$TZ_TOP/../obj.$ARCH_DIR/astra";

    # TZ (module) src dir and obj dir
    $TZ_DIR = $TZ_DIR || "$TZ_TOP/$TZ_MOD";
    $TZ_OBJ_DIR = $TZ_OBJ_DIR || "$TZ_OBJ_TOP/$TZ_MOD";
}


sub tz_dump_env
{
    print "\n";
    print "TZ Build Environment:\n";
    print "===========================================================\n";
    print "TZ_ARCH      = $TZ_ARCH\n";
    print "\n";
    print "TZ_TOOLCHAIN = $TZ_TOOLCHAIN\n";
    print "TZ_ARMGNU    = $TZ_ARMGNU\n";
    print "\n";
    print "TZ_TOP       = $TZ_TOP\n";
    print "TZ_OBJ_TOP   = $TZ_OBJ_TOP\n";
    print "\n";
    print "TZ_MOD       = $TZ_MOD\n";
    print "TZ_DIR       = $TZ_DIR\n";
    print "TZ_OBJ_DIR   = $TZ_OBJ_DIR\n";
    print "===========================================================\n";
}


sub cov_bin_dir
{
    # Use Coverity in PATH if found
    my $cov = `which cov-analyze 2>/dev/null`;
    chomp($cov);
    return dirname($cov) if (($? >> 8) == 0 && $cov && -x $cov);

    # Search in a list of commonly used tools dirs
    foreach my $dir (@COV_BIN_DIRS) {
        print "Searching $dir...\n";
        return $dir if -x "$dir/cov-analyze";
    }

    die "$CMD_BASE: Error: Can not find Coverity bin dir.\n" .
        "$CMD_BASE: Please set env var COV_BIN_DIR.\n";
}


sub cov_build_env
{
    # Search for Coverity bin dir if not defined
    $COV_BIN_DIR = $COV_BIN_DIR || cov_bin_dir;

    # Use TZ obj dir for Coverity user dir
    $COV_USER_DIR = $COV_USER_DIR || "$TZ_OBJ_DIR/coverity";

    # Use default options if not defined
    $COV_OPTIONS = $COV_OPTIONS || $COV_OPTIONS_DEFAULT;

    # Put derived Coverity files under user dir if not defined
    $COV_EMIT_DIR   = $COV_EMIT_DIR   || "$COV_USER_DIR/emit";
    $COV_HTML_DIR   = $COV_HTML_DIR   || "$COV_USER_DIR/html";
    $COV_CONFIG_XML = $COV_CONFIG_XML || "$COV_USER_DIR/coverity_config.xml";
}


sub cov_dump_env
{
    print "\n";
    print "Coverity Environment:\n";
    print "===========================================================\n";
    print "COV_BIN_DIR  = $COV_BIN_DIR\n";
    print "COV_USER_DIR = $COV_USER_DIR\n";
    print "COV_OPTIONS  = $COV_OPTIONS\n";
    print "\n";
    print "COV_EMIT_DIR   = $COV_EMIT_DIR\n";
    print "COV_HTML_DIR   = $COV_HTML_DIR\n";
    print "COV_CONFIG_XML = $COV_CONFIG_XML\n";
    print "===========================================================\n";
}


sub main
{
    usage if grep(/^-h/, @ARGV);

    chomp(my $DATE = `date +%Y_%b_%d_%R`);

    # build TZ env
    $TZ_TOP = $TZ_TOP || cwd;
    $TZ_MOD = $TZ_MOD || $ARGV[0] || 'kernel';

    tz_build_env;
    tz_dump_env;

    # build Coverity env
    cov_build_env;
    cov_dump_env;

    # Make TZ clean first
    my $cmd_line = "make -C $TZ_DIR clean";
    shell($cmd_line);

    # Remove Coverity user dir
    $cmd_line = "rm -rf $COV_USER_DIR";
    shell($cmd_line);

    # Make Coverity output dirs
    foreach my $dir ($COV_USER_DIR, $COV_EMIT_DIR, $COV_HTML_DIR) {
        $cmd_line = "mkdir -p $dir";
        shell($cmd_line);
    }

    # Create Coverity configuration file
    foreach my $compiler (@TZ_COMPILERS) {
        my $comp_path = "$TZ_ARMGNU-$compiler";
        my $comp_base = basename $comp_path;
        $cmd_line = "$COV_BIN_DIR/cov-configure " .
            "--compiler $comp_base --comptype $compiler --template " .
            "--config $COV_CONFIG_XML";
        shell($cmd_line);
    }

    # Make TZ module under Coverity
    $cmd_line = "$COV_BIN_DIR/cov-build " .
        "--config $COV_CONFIG_XML --dir $COV_EMIT_DIR " .
        "make -C $TZ_DIR";
    shell($cmd_line);

    # Analyze Coverity results
    $cmd_line = " $COV_BIN_DIR/cov-analyze " .
        "$COV_OPTIONS --config $COV_CONFIG_XML --dir $COV_EMIT_DIR";
    shell($cmd_line);

    # Format Coverity errors
    $cmd_line = "$COV_BIN_DIR/cov-format-errors " .
        "--dir $COV_EMIT_DIR --html-output $COV_HTML_DIR";
    shell($cmd_line);

    print "\n\n";
    print "===========================================================\n";
    print "$CMD_BASE: ALL DONE!\n";
    print "\n";
    print "Coverity report can be found here:\n";
    print "    $COV_HTML_DIR/index.html\n";
    print "\n";
    print "Coverity results can be found here:\n";
    print "    $COV_USER_DIR\n";
    print "===========================================================\n";
    print "\n";

    return 0;
}

exit main;
