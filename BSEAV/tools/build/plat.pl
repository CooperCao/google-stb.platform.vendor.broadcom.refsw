#!/usr/bin/perl
#############################################################################
# Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#############################################################################

use strict;
use warnings;
use XML::LibXML;
use File::Basename;
my $dirname = dirname(__FILE__);
my $filename = "$dirname/plat.xml";
my $dom = XML::LibXML->load_xml(location => $filename);

my $params = @ARGV;
if (($params =~ 1) && ($ARGV[0] =~ "cleanup")) {
    # Find all board variables and create an array of env to clean up.
    foreach my $boards ($dom->findnodes('/plat/platforms/platform/boards')) {
        foreach my $board ($boards->findnodes('./board')) {
            if ($board->to_literal() !~ /^\s*$/) { # Whitespace only detection.
                my @board_vars = split /,/, $board->to_literal();
                foreach my $board_var (@board_vars) {
                    my $board_name = substr($board_var, 0, index($board_var, '='));
                    print("unset $board_name\n");
                }
            }
        }
    }
    exit;
}

my @config; # Output array
my $chip = shift @ARGV;
my $revision = shift @ARGV;

# Check for minimum of chip and revision
if ((not defined $chip) || (not defined $revision)) {
    print("Error: Invalid command line parameters '@ARGV', need a minimum of platform and revision.\n");
    exit -1;
}

# Need to sanity check chip and revision.
if (($chip !~ /^[0-9sS]+$/) || (length($chip) < 5) || (length($chip) > 6)) {
    print("Error: Platform '$chip' is invalid.\n");
    exit -1;
}

$revision = uc $revision; # Upper case
if ($revision !~ /^[A-F][0-5]/) {
    print("Error: Chip revision '$revision' is an invalid string.\n");
    exit -2;
}

my $valid_chip = 0;
my $arch;
my $kernel_arch = 0;
my $new_mips = 0;
my $kernel_chip = $chip;
my $kernel_rev = $revision;
my $bootloader_type;

# Check if revision is valid for given chip
sub checkRevision {
    my $valid_revision = 0;
    my $platform = $_[0];
    my @revs = split /,/, $platform->findnodes('./revisions')->to_literal();

    foreach my $rev (@revs) {
        if ($rev =~ /^$revision/i) { # Case insensitive match
            $valid_revision = 1;
            push (@config,"export BCHP_VER=$revision\n");
        }
    }
    if ($valid_revision =~ 0) {
        print ("Error: $chip is supported, but invalid revision $revision. Supported revisions are @revs.\n");
        exit -2;
    }

    return $valid_revision;
}

# Place your platform specific hacks here.
sub configureBoardOptions{
    foreach my $input (@_) {
        if ($input !~ /^\s*$/) {
            my @board_options = split /,/, ($input);
            foreach my $option (@board_options) {
                if ($option =~ "new_mips") {
                    $new_mips=1
                }
            }
        }
    }
}

sub buildBoardList{
    my $platform = $_[0];
    my $input_board = $_[1];
    my $pretty_string="";

    foreach my $board ($platform->findnodes('./boards/board')) {
        my $current_board = $board->findnodes('./@name')->to_literal();
        if ($current_board eq $input_board) {
            $pretty_string="$pretty_string\~<$current_board>";
        } else {
            $pretty_string="$pretty_string\~$current_board";
        }
    }
    return $pretty_string;
}

sub buildBoardOptions{
    my $platform = $_[0];
    my $current_board =$_[1];
    my $board =$_[2];
    my $board_pretty_list;

    if ($board->findnodes('./@options')->to_literal() !~ /^\s*$/) {
        my $board_options = $board->findnodes('./@options')->to_literal();
        # Deal with special case stuff
        &configureBoardOptions($board_options);
    }
    $board_pretty_list = uc &buildBoardList($platform,$current_board);
    return $board_pretty_list;
}

sub findKernelVersion{
    my $kernel_version = $_[0];
    my $kernel_major = 0;
    my $kernel_minor = 0;

    # Start with regexp for current kernel versioning scheme x.y-a.b
    my @kernel_versioning = $kernel_version =~ /^(?:(0\.|([1-9]+\d*)))+(\.)+(?:(0\.|([1-9]+\d*)))\-+((0|([1-9]+\d*)\.)+(0|([1-9]+\d*)))$/ ;
    if ($kernel_versioning[0]) {
        $kernel_major = $kernel_versioning[0];
        $kernel_minor = $kernel_versioning[3];
    } else {
        # Try regexp for older 2.6.x kernel releases x.y.z-a.b
        @kernel_versioning = $kernel_version =~ /^(?:(0\.|([1-9]+\d*)))+(\.)+(?:(0\.|([1-9]+\d*)))(\.)+(?:(0\.|([1-9]+\d*)))\-+((0|([1-9]+\d*)\.)+(0|([1-9]+\d*)))$/ ;
        if ($kernel_versioning[0]) {
            $kernel_major = $kernel_versioning[0];
            $kernel_minor = $kernel_versioning[3];
        } else {
            # Finally try regexp for nightly images x.y-yyyymmdd
            @kernel_versioning = $kernel_version =~ /^(?:(0\.|([1-9]+\d*)))+(\.)+(?:(0\.|([1-9]+\d*)))/ ;
            $kernel_major = $kernel_versioning[0];
            $kernel_minor = $kernel_versioning[3];
        }
    }
    return ($kernel_major,$kernel_minor);
}

foreach my $platform ($dom->findnodes('/plat/platforms/platform')) {

    if ( $platform->findnodes('./@id')->to_literal() eq $chip ) {
        $valid_chip=1;
        my $nexus_chip = $chip;
        if ($platform->findnodes('./nexus_platform')->to_literal() !~ /^\s*$/) {
            $nexus_chip = $platform->findnodes('./nexus_platform')->to_literal();
        }
        push (@config,"export NEXUS_PLATFORM=$nexus_chip\n");
        &checkRevision($platform);
    }

    if (!$valid_chip) {
        my @bondouts = split /,/, $platform->findnodes('./bondouts')->to_literal();
        foreach my $bondout (@bondouts) {
            if ($bondout =~ $chip) {
                $valid_chip=1;
                $chip = $platform->findnodes('./@id')->to_literal();
                push (@config,"export NEXUS_PLATFORM=$chip\n");
                &checkRevision($platform);
            }
        }
    }

    if ($valid_chip) {
        # Collect CPU Architecture info
        $arch = $platform->findnodes('./cpu_arch')->to_literal();
        my $board_pretty_list = "";
        my $board_configured = 0;

        # Check argument list for board types and add env
        foreach my $arg (@ARGV) {
            $board_pretty_list = "";
            foreach my $board ($platform->findnodes('./boards/board')) {
                my $current_board = $board->findnodes('./@name')->to_literal();
                if ($current_board =~ /^$arg/i) {
                    my @board_vars = split /,/, $board->findnodes('.')->to_literal();
                    foreach my $board_var (@board_vars) {
                        push (@config,"export $board_var\n");
                        push (@config,"export BOARD_NAME=$board_var\n");
                    }

                    $board_pretty_list = &buildBoardOptions($platform,$current_board,$board);
                    $board_configured=1;
                }
            }
        }

        # Check for default board type
        if ($board_configured != 1) {
            my $default_board = ($platform->findnodes('./boards/@default')->to_literal());
            if ($default_board !~ /^\s*$/) {
                $board_pretty_list = "";
                foreach my $board ($platform->findnodes('./boards/board')) {
                    my $current_board = $board->findnodes('./@name')->to_literal();
                    if ($current_board =~ /^$default_board/i) {
                        my @board_vars = split /,/, $board->findnodes('.')->to_literal();
                        foreach my $board_var (@board_vars) {
                            push (@config,"export $board_var\n");
                            push (@config,"export BOARD_NAME=$board_var\n");
                        }

                        $board_pretty_list = &buildBoardOptions($platform,$current_board,$board);
                        $board_configured=1;
                    }
                }
            }
        }

        if ($board_configured) {
            push (@config,"BOARD_LIST=$board_pretty_list\n");
        }

        # Some older chips need a kernel different to their name
        if ($platform->findnodes('./kernel_chip')->to_literal() !~ /^\s*$/) {
            $kernel_chip = $platform->findnodes('./kernel_chip')->to_literal();
            $kernel_rev = $platform->findnodes('./kernel_rev')->to_literal();
        }

    }

    if ($valid_chip) { last; }

}

if (!$valid_chip) {
    print ("Unknown chip or bondout $chip, exiting\n");
    exit -1;
}

my $B_REFSW_ARCH;

if (index($arch, "ARM") != -1) {
    # Check for default, 32, 64 or mixed command line options
    foreach my $arg (@ARGV) {
        if ($arg =~ /^mixed/i) {
            if ($arch =~ "ARM64") {
                $kernel_arch=64;
                $B_REFSW_ARCH="arm-linux";
            }
        } elsif ($arg =~ 32) {
            $kernel_arch=32;
            $B_REFSW_ARCH="arm-linux";

        } elsif ($arg =~ 64) {
            if ($arch =~ "ARM64") {
                $kernel_arch=64;
                $B_REFSW_ARCH="aarch64-linux";
            }
        }
    }
    # Use defaults if no override
    if (!$kernel_arch) {
        if ($arch =~ "ARM64") {
            $kernel_arch=64;
            $B_REFSW_ARCH="aarch64-linux";
        } else {
            $kernel_arch=32;
            $B_REFSW_ARCH="arm-linux";
        }
    }
} else {
    # MIPS chips, check for Big or little endian
    my $endian = "le";

    foreach my $arg (@ARGV) {
        if ($arg =~ /^be/i) {
            $endian = "be"
        }
    }

    if ($endian =~ "le") {
        $B_REFSW_ARCH="mipsel-linux";
    } else {
        $B_REFSW_ARCH="mips-linux";
    }

}
push(@config,"export B_REFSW_ARCH=$B_REFSW_ARCH\n");

# Read from the XML the default kernel version for current arch.
my $kernel_version = "";
my $karch = $arch;
if (($arch eq "MIPS") && $new_mips) { $karch = "BMIPS"; } # BMIPS for generic MIPS kernel
my $kernel_version_set = 0;

foreach my $kernel ($dom->findnodes('/plat/kernels/kernel')) {
    if ($kernel->findnodes('./@default')->to_literal() !~ /^\s*$/) {
        my @kernel_arches = split /,/, $kernel->findnodes('./@default')->to_literal();
        foreach my $default_arch (@kernel_arches) {
            if ($default_arch eq $karch) {
                $kernel_version = $kernel->to_literal();
            }
            # Allow Family ID & revision to be used in default list for exceptions to normal rules for generations of chips
            if ($default_arch eq "$kernel_chip$kernel_rev") {
                $kernel_version = $kernel->to_literal();
                # Fast escape from double loop, to make sure we choose chip override before normal family rules
                $kernel_version_set = 1;
                last;
            }
        }
    }
    if ($kernel_version_set =~ 1) { last; }
}

# Check for possible kernel version as a command line parameter
foreach my $arg (@ARGV) {
    my ($kernel_major, $kernel_minor) = findKernelVersion($arg);
    if ($kernel_major) {
        if ($kernel_major > 0) {
            if ( -d "/opt/brcm/linux-$arg" ) {
                $kernel_version = $arg;
            } else {
                print ("Unknown kernel version $arg, check /opt/brcm/ on your machine for supported versions\n");
                exit -3;
            }
        }
    }
}

my ($kernel_major, $kernel_minor) = findKernelVersion($kernel_version);

my $LINUX_ROOT="/opt/brcm/linux-$kernel_version";
my $LINUX;

if ($kernel_major >= 4) {
    if (($arch =~ "ARM64") && ($kernel_arch =~ 64)) { # 64Bit ARM
        $LINUX="$LINUX_ROOT/arm64";
    } elsif (($arch =~ "ARM64") && ($kernel_arch =~ 32)) { # Mixed mode
        $LINUX="$LINUX_ROOT/arm";
    } elsif (($arch =~ "ARM") && ($kernel_arch =~ 32)) { # 32bit ARM
        $LINUX="$LINUX_ROOT/arm";
    }    else { # BMIPS
        $LINUX="$LINUX_ROOT/bmips";
    }
} else { # 3.x chip specific kernels.
    my $LINUX_CHIP=`$LINUX_ROOT/tools/board2build.pl $kernel_chip $kernel_rev`;
    chomp $LINUX_CHIP;
    $LINUX="$LINUX_ROOT/$LINUX_CHIP";
}

# Check if we should use a specific version of the toolchain
foreach my $kernel ($dom->findnodes('/plat/kernels/kernel')) {
    my $toolchain_version = $kernel->findnodes('./@toolchain')->to_literal();
    my $base_version = $kernel->findnodes('./@version')->to_literal();
    my $test_version = "$kernel_major$kernel_minor";
    if (($base_version =~ $test_version) && ($kernel->to_literal() =~ $kernel_version)) {
        if ($toolchain_version !~ /^\s*$/) {
            push(@config,"TOOLCHAIN_SUFFIX=$toolchain_version\n");
            last;
        }
    }

}

# use B_REFSW_SW to test for big endian & set the kernel source appropriately
if (($B_REFSW_ARCH eq "mips-uclibc") || ($B_REFSW_ARCH eq "mips-linux") || ($B_REFSW_ARCH eq "mips-linux-uclibc")) {
    $LINUX="$LINUX\_be";
}

push(@config,"export LINUX=$LINUX\n");
push(@config,"LINUX_ROOT=$LINUX_ROOT\n");

# Deal with some lesser used OS variants
foreach my $arg (@ARGV) {
    $arg = lc $arg; # lower case
    my @alt_os = ("vxworks","linuxclient","linuxemu","linuxkernel","win32","nucleus","no-os","ucos");
    foreach my $os (@alt_os) {
        if ($arg eq $os) {
            push(@config,"export SYSTEM=$os\n");
            push(@config,"export B_REFSW_OS=$os\n");
            last;
        }
    }
    foreach my $os ("linux","linuxuser") {
        if ($arg eq $os) {
            push(@config,"export SYSTEM=linux\n");
            push(@config,"export B_REFSW_OS=linuxuser\n");
        }
    }
}

my $bootloader_version_set = 0;
my $bootloader_version;

foreach my $bootloader ($dom->findnodes('/plat/bootloaders/bootloader')) {
    if ($bootloader->findnodes('./@default')->to_literal() !~ /^\s*$/) {
        my @bootloader_variants = split /,/, $bootloader->findnodes('./@default')->to_literal();
        foreach my $default_bootloader (@bootloader_variants) {
            # Allow Family ID & revision to be used in default list for exceptions to normal rules for generations of chips
            if ($default_bootloader eq "$kernel_chip$kernel_rev") {
                $bootloader_type = $bootloader->findnodes('./@type')->to_literal();
                $bootloader_version = $bootloader->to_literal();
                # Fast escape from double loop, to make sure we choose chip override before normal family rules
                $bootloader_version_set = 1;
                last;
            }
            if ($default_bootloader eq $karch) {
                $bootloader_type = $bootloader->findnodes('./@type')->to_literal();
                $bootloader_version = $bootloader->to_literal();
                last;
            }
        }
        if ($bootloader_version_set =~ 1) { last; }
    }
}

if ($bootloader_version) {
    push(@config,"export B_REFSW_BOOTLOADER_VER=$bootloader_version\n");
    push(@config,"export B_REFSW_BOOTLOADER=$bootloader_type\n");
}

# Push environment to STDOUT to pass to bash wrapper
print("@config");
