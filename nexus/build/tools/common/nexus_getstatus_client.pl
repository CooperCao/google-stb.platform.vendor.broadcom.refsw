#!/usr/bin/perl
#     (c)2003-2014 Broadcom Corporation
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
use lib '.';
use bapi_parse_c;

my $file;
my %preload_enums;
my %preload_structs;
my @func_refs;
my @status_array;
my @input_files;

# Array of NEXUS_GetStatus functions to ignore
my @blacklisted_functions = (
    "NEXUS_AudioDecoder_GetStatus", # struct cannot currently be parsed
    "NEXUS_AudioProcessor_GetStatus", # struct cannot currently be parsed
    "NEXUS_SimpleAudioDecoder_GetStatus", # struct cannot currently be parsed
    "NEXUS_SimpleAudioDecoder_GetProcessorStatus" # struct cannot currently be parsed
  );

# Array of headers to skip parsing
my @blacklisted_headers = (
    #"nexus_base_os.h",
    "nexus_power_management.h",
    "nexus_surface_compositor.h"
  );

# Given an enum name and array of values (as strings), print a function which maps 
# each value to a string.
sub print_enum_map
{
    my $name = shift;
    my $value = shift;
    my @values = @$value;
    print "\nconst char *".$name."_as_string(".$name." e){\n";
    print "    static char rs[256];\n";
    print "    switch(e) {\n";
    foreach my $v (@values) {
        print "    case ".$v.":\n";
        print "        return \"".$v."\";\n";
    }
    print "    default: ;\n";
    print "    }\n";
    print "    sprintf(rs, \"%d\", e);\n";
    print "    return rs;\n";
    print "}\n\n";
}

# Create a series of functions which map enum values to strings
sub print_enum_maps
{
    my $key;
    my $value;

    while (($key, $value) = each(%preload_enums)){
        print_enum_map $key, $value;
    }
}

# Debugging code to verify parsing by dumping values.
# Also provides a simple example for format.
sub print_enums
{
    my $key;
    my $value;
    print "\n";
    print "===== preload_enums =====\n";
    print "size of preload_enums:  " . keys( %preload_enums ) . ".\n";
    print %preload_enums;
    print "\n";
    while (($key, $value) = each(%preload_enums)){
         print $key.", ".$value."\n";
         foreach my $z (@$value) {
              print $z."\n";
         }
    }
    print "=========================\n\n";
}

sub print_structs
{
    my $key;
    my $value;
    print "\n";
    print "===== preload_structs =====\n";
    print "size of preload_structs:  " . keys( %preload_structs ) . ".\n";
    print %preload_structs;
    print "\n";
    while (($key, $value) = each(%preload_structs)){
         print $key.", ".$value."\n";
    }
    while (($key, $value) = each(%preload_structs)){
         print $key."\n";
         foreach my $z (@$value) {
              my $k;
              my $v;
              while (($k, $v) = each(%$z)){
                   print $k.", ".$v."\n";
              }
         }
    }
    print "===========================\n\n";
}

sub print_funcs
{
    print "===== preload_funcs =====\n";
    print "size of preload_funcs:  " . scalar(@func_refs) . ".\n";
    bapi_parse_c::print_api @func_refs;
    print "=========================\n\n";
}


sub print_headers
{
    print "#include \"nexus_platform.h\"\n";
    print "#include \"nexus_platform_common.h\"\n";

    for $file (@input_files) {
        if ($file =~ /\.h$/) {
            $file =~ s{.*/}{};
            my $blacklisted;
            my $bl;
            foreach $bl (@blacklisted_headers) {
                if ($bl eq $file) {
                    $blacklisted = $bl;
                }
            }
            if (!defined $blacklisted) {
                print "#include \"".$file."\"\n";
            }
        }
    }

    print "#include \"bstd.h\"\n";
    print "#include \"bkni.h\"\n";
    print "#include \"bkni_multi.h\"\n";
    print "#include <stdio.h>\n\n";
    print "#include <stdlib.h>\n\n";
    print "#include <string.h>\n\n";
}

sub print_status_funcs
{
    print "typedef NEXUS_Error(print_status_func)(const char *);\n";
    print "#define MAX_OBJECTS 32\n";
    print "\n";

    my $func;
    for $func (@func_refs) {
        my $params = $func->{PARAMS};
        my $param;
        my $name = $func->{FUNCNAME};
        if ($name =~ /(.+)_GetStatus$/ ) {
            my $base_name = $1;
            my $status_name = $base_name."Status";
            my $handle_name;
            if ($base_name eq "NEXUS_ParserBand" || $base_name eq "NEXUS_Timebase" || $base_name eq "NEXUS_InputBand" || $base_name eq "NEXUS_AudioOutput" || $base_name eq "NEXUS_VideoOutput") {
                $handle_name =  $base_name;
            }
            else {
                $handle_name =  $base_name."Handle";
            }

            my $field;
            my $fields = $preload_structs{$status_name};

            my $rc;
            if ($func->{RETTYPE} eq "NEXUS_Error") { $rc = "rc = "; };

            my $blacklisted;
            my $bl;
            foreach $bl (@blacklisted_functions) {
                if ($bl eq $name) {
                    $blacklisted = $bl;
                }
            }

            print "/* $base_name */\n";
            if (defined $fields && scalar(@$params)==2 && !defined $blacklisted) {

                push @status_array, $base_name;

                print "NEXUS_Error print_".$name."(const char *module)\n";
                print "{\n";
                print "    NEXUS_Error rc;\n";
                print "    unsigned i;\n";
                print "    NEXUS_InterfaceName interfaceName;\n";
                print "    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];\n";
                print "    unsigned num;\n";
                print "\n";
                print "    strcpy(interfaceName.name, module);\n";
                print "    rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);\n";
                print "\n";
                print "    if (!num || rc) {\n";
                print "        printf(\"no handles for '%s' found\\n\", module);\n";
                print "    }\n";
                print "    else {\n";
                print "        for (i=0;i<num;i++) {\n";
                print "            $handle_name handle = ($handle_name)objects[i].object;\n";
                print "            $status_name status;\n";
                print "\n";
                print "            ".$rc."".$name."(handle, &status);\n";
                print "\n";
                print "            printf(\"".$base_name."Handle: %p\\n\", (void *)handle);\n";
                for $field (@$fields) {
                    print "            printf(\"  $field->{NAME}: ";
                    if ($field->{TYPE} eq "bool") {
                        print "\%s\\n\", ";
                        print "status.$field->{NAME}?\"true\":\"false\"";
                    } elsif ($field->{TYPE} eq "const void *" || $field->{TYPE} eq "void *") {
                        print "\%p\\n\", ";
                        print "status.$field->{NAME}";
                    } elsif (exists $preload_enums{$field->{TYPE}}) {
                        print "\%s (%d)\\n\", ";
                        print $field->{TYPE}."_as_string(status.".$field->{NAME}."),status.".$field->{NAME};
                    } elsif ($field->{TYPE} eq "NEXUS_Rect") {
                        print "(\%d,\%d,\%d,\%d)\\n\", ";
                        print "status.$field->{NAME}.x,status.$field->{NAME}.y,status.$field->{NAME}.width,status.$field->{NAME}.height";
                    } elsif ($field->{TYPE} eq "NEXUS_SurfaceRegion") {
                        print "(\%d,\%d)\\n\", ";
                        print "status.$field->{NAME}.width,status.$field->{NAME}.height";
                    } elsif ($field->{TYPE} eq "NEXUS_CallbackDesc") {
                        print "(\%#x,\%p,\%d)\\n\", ";
                        print "(unsigned)status.$field->{NAME}.callback,status.$field->{NAME}.context,status.$field->{NAME}.param";
                    } elsif ($field->{TYPE} eq "uint64_t") {
                        print "\%u\\n\", ";
                        print "(unsigned)status.$field->{NAME}";
                    } elsif ($field->{TYPE} =~ /^unsigned long/ || $field->{TYPE} =~ /^NEXUS_PlaybackPosition/) {
                        print "\%lu\\n\", ";
                        print "status.$field->{NAME}";
                    } elsif ($field->{TYPE} =~ /^unsigned/) {
                        print "\%u\\n\", ";
                        print "status.$field->{NAME}";
                    } elsif ($field->{TYPE} =~ /^long/) {
                        print "\%ld\\n\", ";
                        print "status.$field->{NAME}";
                    } elsif ($field->{TYPE} =~ /Handle$/) {
                        print "\%p\\n\", ";
                        print "(void *)status.$field->{NAME}";
                    } elsif ($field->{TYPE} eq "NEXUS_RecpumpStatus") {
                        # Don't display a nested status struct
                        print "\\n\"";
                    } elsif ($field->{TYPE} eq "NEXUS_DebugFifoInfo") {
                        print "\%p(%u,%#x)\\n\", ";
                        print "(void *)status.$field->{NAME}.buffer, status.$field->{NAME}.elementSize,status.$field->{NAME}.offset";
                    } else {
                        print "\%d\\n\", ";
                        print "status.$field->{NAME}";
                    }
                    print ");\n";
                }
                print "        }\n";
                print "    }\n";
                print "    return rc;\n";
                print "}\n";
            }
        }
    }
}

sub print_status_array
{
    print "typedef struct status_func_entry {\n";
    print "    print_status_func *func;\n";
    print "    const char *module_name;\n";
    print "    const char *function_name;\n";
    print "} status_func_entry;\n";
    print "\n";
    print "const status_func_entry status_functions[] = {\n";
    foreach (@status_array) {
        print "    { print_".$_."_GetStatus, \"".$_."\", \"".$_."_GetStatus\" },\n";
    }
    print "};\n";
}

# load and parse the headers
sub parse_headers
{
    my @funcs;

    print "/*\n"; # any output from parsing will be masked by a comment
    open FILE, @ARGV[0];
    @input_files = <FILE>;
    close FILE;
    chomp @input_files;

    for $file (@input_files) {
        my $blacklisted;
        my $bl;
        foreach $bl (@blacklisted_headers) {
            my $basename = $file;
            $basename =~ s{.*/}{};
            if ($bl eq $basename) {
                $blacklisted = $bl;
            }
        }
        if ($file =~ /\.h$/ && !defined $blacklisted) {
            my $name;
            my $members;

            my $new_enum = bapi_parse_c::parse_enum $file;
            while (($name, $members) = each %$new_enum) {
                $preload_enums{$name} = $members;
            }

            my $new_struct = bapi_parse_c::parse_struct $file;
            while (($name, $members) = each %$new_struct) {
                $preload_structs{$name} = $members;
            }

            push @funcs, bapi_parse_c::get_func_prototypes $file;
        }
    }
    @func_refs = bapi_parse_c::parse_funcs @funcs;

    #print_enums;
    #print_structs;
    #print_funcs;
    print "*/\n";
}

# main code begins here:
if ($#ARGV ne "0") {
    print "Usage: \n";
    print "  $0 input_file.txt\n";
    print "where [input_file.txt] lists all headers, one per line, to be parsed.\n";
    exit;
}

# parse the headers
parse_headers;

# print the required includes
print_headers;

# print all the enum maps
print_enum_maps;

# print the print_x_GetStatus functions
print_status_funcs;

# print the function call and parameters array
print_status_array;

