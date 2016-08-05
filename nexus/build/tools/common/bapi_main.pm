#!/usr/bin/perl
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
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
#############################################################################
package bapi_main;

use strict;
use warnings FATAL => 'all';

use Getopt::Long;
use bapi_parse_c;
use bapi_untrusted_api;
use bapi_classes;

sub filter_file {
    my $file = shift;
    return 1 if($file =~ /nexus_base_types_client.h$/ || $file =~ /nexus_base_os.h$/ );

    if (($file =~ /_init.h$/ && $file !~ /nexus_platform_init.h$/) ||
        $file =~ /\*/ || # skip not expaneded wildcards
        $file =~ /nexus_base_\w+.h$/) {
        return 0;
    }
    return 1;
}

# This uses and consumes ARGV
sub main
{
    my $mode=shift;
    my $file;
    my %structs;
    my $source_file;
    my $file_headers;
    my @file_list;
    my @funcrefs;
    my $module_number;
    my $class_list_file;
    my @class_handles;
    my @includes;
    my $arg_mode='';
    my $stdin=0;
    my $name;
    my $members;
    my $output_file;

    if ($#ARGV == -1) {
        print "Usage: perl bapi_build.pl [--source listfile] [--module_number number] [--class_list file] module destdir file1.h file2.h ...\n";
        exit;
    }

    #check presence of a sources file and process if present
    GetOptions (
        'class_list=s' => \$class_list_file,
        'mode=s' => \$arg_mode,
        'module_number=s' => \$module_number,
        'output=s' => \$output_file,
        'source=s' => \$source_file,
        'stdin' => \$stdin,
    );
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


    my @untrusted_api;
    if ($mode ne 'syncthunk') {
        @untrusted_api = bapi_untrusted_api::parse "nexus/build/tools/common/nexus_untrusted_api.txt";
    }

    # Process all files on the command line
    my $module = shift @ARGV;
    my $destdir = shift @ARGV;

    # read class_handles.inc file into a list
    if(defined $class_list_file) {
        open FILE, $class_list_file or die "Can't open $class_list_file";
        @class_handles = <FILE>;
        close FILE;
        chomp @class_handles;
    }

    my %preload_structs;
    my %api_files;
    for $file (@file_list,@ARGV) {
        if (filter_file($file) && $file =~ /\.preload/) {
            # a .preload file allows nexus to load structs from other header files
            # for proxy autogen. this removes the need to have attr/callback definitions present
            # in the same header file.
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
                    my $include = "$dir\/$f";
                    my $file_structs = bapi_parse_c::parse_struct $include;
                    while (($name, $members) = each %$file_structs) {
                        $preload_structs{$name} = $members;
                    }
                }
            }
            next;
        } elsif($file =~ m!([^/]+)$!) {
            $api_files{$1}=1;
        }
    }
    if($stdin) {
        while(<STDIN>) {
            chomp;
            s/^[^:]+://;
            s!\\!!;
            while(/(\S+)/sg) {
                my $file = $1;
                next if( $file =~ m!(^|/)magnum/! && not $file =~ m/bm2mc_packet.h$/ );
                if(filter_file($file)) {
                    if($file =~ m!([^/]+)$!) {
                        if(not exists $api_files{$1}) {
                            my $file_structs = bapi_parse_c::parse_struct $file;
                            while (($name, $members) = each %$file_structs) {
                                $preload_structs{$name} = $members;
                            }
                        }
                    }
                }
            }
        }
    }

    for $file (@file_list,@ARGV) {
        next if ($mode ne 'kernelproxy' && $mode ne 'abiverify') && (bapi_classes::skip_thunk($file));
        #print "bapi_main parsing $file\n";

        if( not filter_file($file)) {
            next;
        }

        push @includes, $file;

        my @funcs;
        push @funcs, bapi_parse_c::get_func_prototypes $file;
        my @refs = bapi_parse_c::parse_funcs @funcs;

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

        my $basename = $file;
        $basename =~ s{.*/}{};
        if (!($basename =~ /\*/) && grep {/$basename/} @untrusted_api) {
            for (@refs) {
                $_->{CALLABLE_BY_UNTRUSTED} = 1;
                #print "untrusted api: $_->{FUNCNAME}\n";
            }
        }
        push @funcrefs, @refs;

        my $file_structs = bapi_parse_c::parse_struct $file;
        while (($name, $members) = each %$file_structs) {
            $structs{$name} = $members;
        }
    }
    bapi_parse_c::expand_structs(\%structs,\%preload_structs);
    if($mode ne 'abiverify') {
        while (($name, $members) = each %structs) {
            $structs{$name} = [grep { index($_->{NAME},'[') == -1 } @$members];
        }
    }
    bapi_parse_c::print_api @funcrefs  if 0;
    my $version = bapi_parse_c::version_api @funcrefs;
    $version = ($version * 0x10000) + bapi_parse_c::version_struct \%structs;
    {
        CLASS_HANDLES => \@class_handles,
        DESTDIR => $destdir,
        FUNCREFS => \@funcrefs,
        INCLUDES => \@includes,
        MODE => $arg_mode,
        MODULE => $module,
        MODULE_NUMBER => $module_number,
        OUTPUT => $output_file,
        STRUCTS => \%structs,
        VERSION => $version,
    }
}

1;;
