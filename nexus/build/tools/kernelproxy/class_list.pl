#############################################################################
#  Broadcom Proprietary and Confidential. (c)2008-2016 Broadcom. All rights reserved.
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
#
#############################################################################
use strict;
use lib "../common";
use bapi_parse_c;
use bapi_common;
use bapi_classes;
use bapi_main;

my $class_file = shift @ARGV;
my $file;
my @funcs;
my %structs;
my @modules;

# @ARGV is a list of all nexus public api header files and module names
# -headers FILE FILE FILE -modules MODULE MODULE MODULE
my $parsing_headers;
foreach $file (@ARGV) {
    if ($file eq "-headers") {
        $parsing_headers = 1;
        next;
    }
    elsif ($file eq "-modules") {
        undef $parsing_headers;
        next;
    }
    
    if ($parsing_headers) {
        next unless (bapi_main::filter_file($file));
        #print "kernelproxy/class_list.pl parsing $file\n";
    
        push @funcs, bapi_parse_c::get_func_prototypes $file;
    
        my $file_structs = bapi_parse_c::parse_struct $file;
        my $name;
        my $members;
        while (($name, $members) = each %$file_structs) {
            $structs{$name} = $members;
        }
    }
    else {
        push @modules, $file;
    }
}

# Build the perl datastructure
my @funcrefs = bapi_parse_c::parse_funcs @funcs;
my $funcs = \@funcrefs;

# create a file which contains the handles for every "class" (an interface with a constructor/destructor)
my $destructors = bapi_classes::get_destructors $funcs;
my $classes = bapi_classes::get_classes $funcs, $destructors;

# create class_list.inc
open(OUTFILE, ">$class_file") or die "Unable to open output file $class_file";
for (@$classes) {
    my $class = $_;
    #print "class $class->{CLASS_TYPE}\n";
    print OUTFILE "$class->{CLASS_TYPE}\n";
}
close OUTFILE;
