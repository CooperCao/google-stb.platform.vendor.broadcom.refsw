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

my $elf_file 		= $ARGV[0];
my $ver_file 		= $ARGV[1];
my $tzk_ver_file 	= $ARGV[2];

my @ver = `cat $ver_file`;
die 'cat failed' if $? >> 8;

my $ver_major;
my $ver_minor;
my $ver_build;

foreach my $line (@ver) {

    if ($line =~ /^#define\s+(VERSION_MAJOR)\s+(\d+)/) {
		$ver_major = $2;
    }
    if ($line =~ /^#define\s+(VERSION_MINOR)\s+(\d+)/) {
		$ver_minor = $2;
    }
    if ($line =~ /^#define\s+(VERSION_BUILD)\s+(\d+)/) {
		$ver_build = $2;
    }

}

my @kver = `cat $tzk_ver_file`;
die 'cat failed' if $? >> 8;

my $tzk_ver_major;
my $tzk_ver_minor;
my $tzk_ver_build;

foreach my $line (@kver) {

    if ($line =~ /^#define\s+(ASTRA_VERSION_MAJOR)\s+(\d+)/) {
		$tzk_ver_major = $2;
    }
    if ($line =~ /^#define\s+(ASTRA_VERSION_MINOR)\s+(\d+)/) {
		$tzk_ver_minor = $2;
    }
    if ($line =~ /^#define\s+(ASTRA_VERSION_BUILD)\s+(\d+)/) {
		$tzk_ver_build = $2;
    }

}

my $full_size = -s $elf_file;
my $pad = 16 - ($full_size % 16);
my $text_size = $full_size + $pad;

print "\0" x $pad;
print "\n";
print "SAGE_SGN_INFO|";
print "IMAGE_TYPE=TZA|";
print "BCHP_CHIP=7255|";
print "BCHP_VER=A0|";
print "VERSION_MAJOR=$ver_major|";
print "VERSION_MINOR=$ver_minor|";
print "VERSION_REVISION=0|";
print "VERSION_BRANCH=$ver_build|";
print "TEXT_SIZE=$text_size|";
print "BE_ENCRYPTION=1|";
print "BE_SIGNING=1|";
print "TZA_APPLICATION_ID=0x11223344|";
print "TZK_VERSION_MAJOR=$tzk_ver_major|";
print "TZK_VERSION_MINOR=$tzk_ver_minor|";
print "TZK_VERSION_REVISION=0|";
print "TZK_VERSION_BRANCH=$tzk_ver_build|";
print "\n";
