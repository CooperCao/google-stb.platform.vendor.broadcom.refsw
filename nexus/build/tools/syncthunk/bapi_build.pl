#!/usr/bin/perl
#     (c)2003-2013 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# File Description:
#
# Revision History:
#
# $brcm_Log: $
# 
#############################################################################
use strict;
use warnings FATAL => 'all';

use lib "../common";
use bapi_parse_c;
use bapi_thunks;
use bapi_classes;
use Getopt::Long;

my $file;
my @funcs;
my %structs;
my $source_file;
my $file_headers;
my @file_list;

if ($#ARGV == -1) {
    print "Usage: perl bapi_build.pl [--source listfile] module destdir file1.h file2.h ...\n";
    exit;
}

#check presence of a sources file and process if present
GetOptions ('source=s' => \$source_file);
if($source_file) {
    open(HEADERS, $source_file) || die("Could not open source file: $source_file");
    $file_headers=<HEADERS>;
    close(HEADERS);

    chomp($file_headers);
    @file_list = split(/\s+/,$file_headers);
    my $count;
    $count = @file_list;
    print "Sources file $source_file contains $count headers\n";
}

# Process all files on the command line
my $module = lc shift @ARGV;
my $destdir = shift @ARGV;

my %preload_structs;
for $file (@file_list,@ARGV) {
    my $name;
    my $members;
    next if (bapi_classes::skip_thunk($file));
    #print "syncthunk/bapi_build.pl parsing $file\n";

    # a .preload file allows nexus to load structs from other header files
    # for proxy autogen. this removes the need to have attr/callback definitions present 
    # in the same header file.
    if ($file =~ /\.preload/) {
        open(my $fin , $file) or die "Can't open $file";
        my @preload_files = <$fin>;
        close $fin;
        chomp @preload_files;
        my ($dir) = $file =~ /(.+)\/\w+/;
        for my $f (@preload_files) {
            # trim comments and test for some non-whitespace
            $f =~ s/(.*)\#.*/$1/g;
            if ($f =~ /\w/) {
                #print "ipcthunk/bapi_build.pl parsing preload $dir/$file\n";
                my $file_structs = bapi_parse_c::parse_struct "$dir\/$f";
                while (($name, $members) = each %$file_structs) {
                    $preload_structs{$name} = $members;
                }
            }
        }
        next;
    }
    push @funcs, bapi_parse_c::get_func_prototypes $file;
    
    my $file_structs = bapi_parse_c::parse_struct $file, \%preload_structs;
    while (($name, $members) = each %$file_structs) {
        $structs{$name} = $members;
    }
}

# Build the perl datastructure
my @funcrefs = bapi_parse_c::parse_funcs @funcs;

# test for duplicate functions. these will fail later in the link, but we can catch the failure now.
# this can be from copying a header file (for example, cp nexus_types.h nexus_types.backup.h). the *.h will suck that in.
# it can also happen if you using #if. the perl thunk does not recognize that.
my $f1;
for $f1 (@funcrefs) {
    my $f2;
    my $count = 0;
    for $f2 (@funcrefs) {
        if ($f2->{FUNCNAME} eq $f1->{FUNCNAME}) {
            if (++$count == 2) {
                print "ERROR: Duplicate function $f1->{FUNCNAME} found within $module module.\n";
                print "ERROR:   You may have unnecessary copies of header files in your public API directory.\n";
                print "ERROR:   You may be using #if around duplicate prototypes. Nexus does not run a preprocessor in its thunk.\n";
                exit 1;
            }
        }
    }
}

# Print out the perl datastructure
#bapi_parse_c::print_api @funcrefs;
#bapi_parse_c::print_struct \%structs;

#print "Building synchronization thunk for module ${module}\n";

# Build thunk layer
bapi_thunks::build_thunks ${module}, "${destdir}/nexus_${module}_thunks.c", \@funcrefs, \%structs;
bapi_thunks::build_remapping "${destdir}/nexus_${module}_thunks.h", \@funcrefs;

