#!/usr/bin/perl
#  Copyright (C) 2010-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
use strict;

use bapi_parse_c;
use bapi_verify;

my $ipc_mode;
if ($#ARGV == -1) {
    print "Usage: perl bipc_build.pl file1.h file2.h ...\n";
    exit -1;
}



sub is_handle 
{
    my $type = shift;
    ($type =~ /_Handle$/ || $type =~ /_t$/);
}

sub parse_func
{
    my $prototype = shift;
    my %func;
    my $more;
    my @params; # this is a list of a param hash

    # comment out the attr hint int the actual prototype
    my $actual_prototype = $prototype;
    $actual_prototype =~ s/(attr\{.+?})/\/* $1 *\//sg;
    $func{PROTOTYPE} = $actual_prototype;
#    print "'$actual_prototype'\n";


    ($func{RETTYPE}, $more) = $prototype =~ /(.*?)\s*([\s*\*])\w+\s*\(/;
    if ($more eq "*") {
        $func{RETTYPE} .= $more;
    }
    $func{RETTYPE_ISHANDLE} = is_handle $func{RETTYPE};
    ($func{FUNCNAME}) = $prototype =~ /(\w+)\s*\(/;

    # get the params into a raw list
    $prototype =~ /\(\s*(attr\{(.+?)})?(.*?)\)$/s;
    my $params=$3;
    if(defined $2) {
        $func{ATTR} = parse_attr $2;
    }
    if ($params eq "void") {
        $params = undef;
    }
    my @rawparams = split /,/, $params;
    my $p;

    for $p (@rawparams) {
        my %paramhash;

        # See if we have a attr hint and grab it now
        # This also removes that hint from the variable
        if ($p =~ s/attr\{(.+?)}//) {
#            print "$func{FUNCNAME} attr = $1\n";
            $paramhash{ATTR} = parse_attr $1;
        }

        # parse type and name
        my ($type, $name) = $p =~ /(\w[\w\s]*[\s\*]+)(\w+)/;
        # strip leading and trailing whitespace
        $type =~ s/^\s*(.*?)\s*$/$1/;

        $paramhash{NAME} = $name;
        $paramhash{TYPE} = $type;
        # a pointer in the type means the data is passed by reference
        $paramhash{ISREF} = ($type =~ /\*\s*$/);

        if ($paramhash{ISREF}) {
            # a const pointer is assumed to be an input parameter,
            # note that "const void **" is parsed as a non-const pointer to "const void *", so it's out
            # nexus does not parse [out]
            $paramhash{INPARAM} = ($type =~ /^\s*const/) && !($type =~ /\*\s*\*/);
            if ($paramhash{INPARAM}) {
                ($paramhash{BASETYPE}) = ($type =~ /^\s*const\s*(.*?)\s*\*\s*$/);
            }
            else {
                ($paramhash{BASETYPE}) = ($type =~ /^\s*(.*?)\s*\*\s*$/);

                # non-const void* params w/ attr{nelem_out} are out params, otherwise they are actually in params
                if ($paramhash{BASETYPE} eq 'void' && !defined $paramhash{ATTR}->{'nelem_out'}) {
                    $paramhash{INPARAM} = 1;
                }
            }
        }
        else {
            # if not by-reference, then by-value, which is always in in param
            $paramhash{INPARAM} = 1;
        }
        push @params, \%paramhash;
    }
    $func{PARAMS} = \@params;

    # Return a reference to the func hash
    return \%func;
}

sub parse_funcs
{
    my $func;
    my @funcrefs;
    for $func (@_) {
        my $funcref = parse_func $func;

        push @funcrefs, $funcref;
    }
    @funcrefs;
}

use Data::Dumper;

sub group_api 
{
    my $func;
    my %classes;

    for $func (@_) {
        my $name = $func->{FUNCNAME};
        if( ($name =~ /^(\w+)_create$/) || ($name =~ /^(\w+)_Create$/) || ($name =~ /^(\w+)_open$/) || ($name =~ /^(\w+)_Open$/)) {
            my %class;
            my $name = $1;
            if($func->{RETTYPE_ISHANDLE} && ($func->{RETTYPE} =~ /^(\w+)_Handle/ || $func->{RETTYPE} =~ /^(\w+)_t/)) {
                $class{NAME} = $name;
                $class{TYPE} = $1;
                $class{CONSTRUCTOR} = $func;
                $classes{$name} = \%class;
            } else {
                die "Invalid constructor $func->{PROTOTYPE}";
            }
        }
    }
    for $func (@_) {
        my $name = $func->{FUNCNAME};
        my $classname;
        for $classname (keys %classes) {
            if($name =~ /^${classname}_(\w+)$/) {
                my $method = lc $1;
                my $class = $classes{$classname};
                next if $name eq $class->{CONSTRUCTOR}->{FUNCNAME};
                if($method eq 'close' || $method eq 'destroy') {
                    $class->{DESTRUCTOR} = $func;
                } else {
                    my $methods = $class->{METHODS};
                    push @$methods, $func;
                    $class->{METHODS} = $methods;
                }
            }
        }
    }
#    print Dumper(\%classes);
    return \%classes;
}

sub append_code {
    my $dest = shift;
    my $code = shift;
    my $ident = shift;

    for(@$code) {
        push @$dest, "$ident$_";
    }
}

sub print_code
{
    my $file = shift;
    my $code = shift;
    my $ident = shift;

    if(defined $code) {
        for(@$code) {
            print $file "$ident$_\n";
        }
    }
}
sub append_structure  {
    my $dest = shift;
    my $name = shift;
    my $code = shift;

    if(scalar @$code) {
        push @$dest, "struct {";
        append_code $dest,$code,"    ";
        push @$dest, "} $name;";
    }
}

my $ipc_field = "_b_ipc_id";
my $ipc_data = "_b_ipc_data";
my $ipc_result = "_b_ipc_result";

sub struct_add_field {
    my($struct, $type, $name) = @_;
    push @$struct, {KIND => ['struct'], NAME => $name  , TYPE => $type};
}
sub process_function_args {
    my $ipc = shift;
    my $class = shift;
    my $func = shift;
    my $type = shift;
    my @data_in;
    my @data_out;
    my @server_args;
    my @client_in;
    my @client_out;
    my $params = $func->{PARAMS};
    my $param;
    my $data = "$ipc_data -> $func->{FUNCNAME}";
    my $cnt = 0;
    for $param (@$params) {
        if($cnt == 0) {
            push @server_args, "($param->{TYPE}) * $ipc_field";
            if( $type eq 'CONSTRUCTOR') {
               die "Invalid constructor $func->{PROTOTYPE}" unless $param->{TYPE} eq 'bipc_t';
               struct_add_field(\@data_in, 'bipc_interface_descriptor',  $ipc_field);
               struct_add_field(\@data_out, 'unsigned', $ipc_result);
               push @client_in, "$data . in. $ipc_field = bipc_${class}_descriptor;";
               push @client_out, "$ipc_result -> id = $data . out. $ipc_result;";
           } else {
               die "Invalid function $func->{PROTOTYPE}" unless is_handle $param->{TYPE};
           }
        }  elsif ($param->{ISREF}) { 
            if ($param->{INPARAM}) {
                push @server_args, "& $data .in.  $param->{NAME}";
                struct_add_field(\@data_in, $param->{BASETYPE},  $param->{NAME});
                push @client_in, "$data .in. $param->{NAME} = * $param->{NAME};"
            } else {
                push @server_args, "& $data .out.  $param->{NAME}";
                struct_add_field(\@data_out, $param->{BASETYPE}, $param->{NAME});
                push @client_out, "* $param->{NAME} = $data . out. $param->{NAME};";
            }
        } else {
            push @server_args, "$data .in. $param->{NAME}";
            struct_add_field(\@data_in, $param->{TYPE}, $param->{NAME});
            push @client_in, "$data .in. $param->{NAME} = $param->{NAME};"
        }
        $cnt++;
    }
    if($func->{RETTYPE} ne 'void' && $type ne 'CONSTRUCTOR') {
        struct_add_field(\@data_out, $func->{RETTYPE}, $ipc_result);
        push @client_out, "$ipc_result = $data . out. $ipc_result;";
    }
    if(0 == scalar @data_in) {
        struct_add_field(\@data_in, 'unsigned', '_b_ipc_unused');
    }
    $ipc->{$func->{FUNCNAME}} = {
        'SERVER_ARGS' => \@server_args,
        'DATA_IN' => \@data_in,
        'DATA_OUT' => \@data_out,
        'CLIENT_IN' => \@client_in,
        'CLIENT_OUT' => \@client_out,
        'TYPE' => $type,
        'FUNC' => $func
    }
}

sub make_class_ipc_data
{
    my %ipc;
    my $class = shift;
    my $name = $class->{NAME};
    process_function_args \%ipc, $name, $class->{CONSTRUCTOR}, 'CONSTRUCTOR';
    process_function_args \%ipc, $name, $class->{DESTRUCTOR}, 'DESTRUCTOR';
    for (@{$class->{METHODS}}) {
        process_function_args \%ipc, $name, $_;
    }
#    print Dumper(\%ipc);
    return \%ipc;
}


sub send_offset_and_size {
   my $name = shift;
   my $func_ipc = shift;
   my $offset='0';
   my $size='0';

    if(scalar @{$func_ipc->{CLIENT_OUT}}) { 
        $size = " sizeof($ipc_data ->$func_ipc->{FUNC}->{FUNCNAME} . out)";
        $offset = " offsetof(union b_ipc_${name}_data, $func_ipc->{FUNC}->{FUNCNAME} . out )  - offsetof(union b_ipc_${name}_data, $func_ipc->{FUNC}->{FUNCNAME})";
    }
    ($offset, $size);
}

sub make_ipc
{
    my $classes = shift;
    my $mode = shift;
    my $file = shift;
    my $structs = shift;
    my %ipc_types;
    my %referenced_structs;

    open(FILE, '>', $file) or die "Can't open '$file'";
    print FILE "/*********************************\n";
    print FILE "*\n";
    print FILE "* This file is autogenerated by the IPC script .\n";
    print FILE "*\n";
    print FILE "*********************************/\n";
    print FILE "/*********************************\n";
    print FILE "*\n";
    if($mode eq 'server') {
        print FILE "* This file contains functions that access data over the IPC channel and routes execution to local implementation.\n";
    } elsif($mode eq 'client') {
        print FILE "* This file contains stub for every API function that intercepts call and routes it the server via IPC channel.\n";
    }
    print FILE "*\n";
    print FILE "*********************************/\n";


    for my $c (values %$classes) {
        my @class_data;
        my $ipc = make_class_ipc_data $c;
        my @sorted_keys_ipc = sort (keys %$ipc);
        my $name= $c->{NAME};
        $c->{IPC} = $ipc;
        for my $api (@sorted_keys_ipc) {
            my @api_data;
            for('in','out') {
                my $t = "b_ipc_${api}_data_$_";
                my $data = $ipc->{$api}{"DATA_\U$_"};
                if(scalar @$data) {
                    $ipc_types{$t}=$data;
                    struct_add_field(\@api_data, $t, $_);
                }
            }
            bapi_verify::add_substruct(\@class_data, $api, 'union', \@api_data);
        }
        $ipc_types{"b_ipc_${name}_data"}  =  \@class_data;
    }
    bapi_verify::print_struct(\*FILE, \%ipc_types, [], 0);
    if($ipc_mode eq 'verify') {
        for(values %ipc_types) {
            my $struct_reference = bapi_verify::get_referenced_structs($_, $structs, {'WITH_UNION'=>1});
            for (@$struct_reference) {
                $referenced_structs{$_} = $structs->{$_};
            }
        }
        if($mode eq 'server') {
            for (keys %ipc_types) {
                $referenced_structs{$_} = $ipc_types{$_};
            }
            print FILE "\n#if B_IPC_COMPAT_SUPPORT\n";
            for (values %$classes) {
                my $name= $_->{NAME};
                print FILE "#include \"b_ipc_${name}_compat.h\"\n";
            }
            bapi_verify::print_struct(\*FILE, \%referenced_structs, [], 1);
            print FILE "\n";
            for my $kind ('in','out') {
                my %class_structs;
                for (values %$classes) {
                    my $name= $_->{NAME};
                    my $ipc =$_->{IPC};
                    for my $api (keys %$ipc) {
                        my $t = "b_ipc_${api}_data_$kind";
                        if(exists $ipc_types{$t}) {
                            my $struct_reference = bapi_verify::get_referenced_structs($ipc_types{$t}, \%referenced_structs);
                            for (@$struct_reference) {
                                $class_structs{$_}=$referenced_structs{$_};
                            }
                            $class_structs{$t}=$ipc_types{$t};
                        }
                    }
                }
                bapi_verify::visit_struct(\%class_structs, \&bapi_verify::convert_one_struct, \*FILE, 'ipc', \%referenced_structs, [], $kind);
            }
            print FILE "#endif /* B_IPC_COMPAT_SUPPORT */ \n";
        }
    }
    for (sort {$a->{NAME} cmp $b->{NAME}} (values %$classes)) {
        my $f;
        my $ipc = $_->{IPC};
        my @apis = sort (keys %$ipc);
        my @enums;
        my $name= $_->{NAME};
        my $constructor = $_->{CONSTRUCTOR}->{FUNCNAME};
        my $destructor = $_->{DESTRUCTOR}->{FUNCNAME};
        printf STDOUT "%s IPC $name\n", $mode;
        append_code \@enums, \@apis, "\tb_ipc_";
        print FILE "\n";
        print FILE "enum b_ipc_${name}_methods {\n";
        print FILE join(",\n",sort(@enums));
        print FILE "\n};\n";
        print FILE "\n";

        if($mode eq 'client') {
            print FILE "const bipc_interface_descriptor bipc_${name}_descriptor = {\n";
            print FILE "    \"$name\",\n";
            print FILE "    \"$name\",\n";    
            print FILE "    sizeof(union b_ipc_${name}_data)\n";
            print FILE "};\n";
            print FILE "struct $_->{TYPE} {\n";
            print FILE "    bipc_t ipc;\n";
            print FILE "    unsigned id;\n";
            print FILE "};\n";
            print FILE "#include \"bipc_client_priv.h\"\n";
            print FILE "#include \"bkni.h\"\n";

            # Build client portion 
            for (@apis) {
                my $func_ipc = $ipc->{$_};
                print FILE "$func_ipc->{FUNC}->{PROTOTYPE}\n\{\n";
                my $params = $func_ipc->{FUNC}->{PARAMS};
                my $handle = @$params[0]->{NAME};
                print FILE "    union b_ipc_${name}_data * $ipc_data;\n";
                if($func_ipc->{FUNC}->{RETTYPE} ne 'void') {
                    print FILE "    $func_ipc->{FUNC}->{RETTYPE} $ipc_result;\n";
                    if($func_ipc->{FUNC}->{RETTYPE_ISHANDLE}) {
                        print FILE "    $ipc_result = NULL;\n";
                    } else {
                        print FILE "    $ipc_result = -1;\n";
                    }
                }
                my $ipc_arg;
                my $ipc_id;
                if($func_ipc->{TYPE} eq 'CONSTRUCTOR') {
                    $ipc_arg = $handle;
                    $ipc_id = "BIPC_INSTANCE_ID_NEW";
                    print FILE "    $ipc_data = (union b_ipc_${name}_data *)bipc_client_begin($ipc_arg, &bipc_${name}_descriptor);\n";
                } else {
                    $ipc_arg = "$handle -> ipc";
                    $ipc_id = "$handle -> id";
                    print FILE "    $ipc_data = (union b_ipc_${name}_data *)bipc_client_begin($ipc_arg, NULL);\n";
                }
                print_code \*FILE, $func_ipc->{CLIENT_IN}, "    ";
                my ($send_offset,$send_size) = send_offset_and_size $name, $func_ipc;
                print FILE "    if(bipc_client_send($ipc_arg, $ipc_id, b_ipc_$_ , sizeof( $ipc_data ->$func_ipc->{FUNC}->{FUNCNAME} . in ) , $send_offset, $send_size)==0) {\n";
                if($func_ipc->{TYPE} eq 'CONSTRUCTOR') {
                    print FILE "        if($ipc_data->$func_ipc->{FUNC}->{FUNCNAME} . out . $ipc_result == BIPC_INSTANCE_ID_NEW) { goto done;}\n";
                    print FILE "        $ipc_result = ($func_ipc->{FUNC}->{RETTYPE}) BKNI_Malloc(sizeof( * $ipc_result));\n";
                    print FILE "        $ipc_result->ipc = $handle;\n";
                }
                print_code \*FILE, $func_ipc->{CLIENT_OUT}, "        ";
                print FILE "    } else {\n";
                print FILE "    }\n";
                print FILE "    goto done;\n";
                print FILE "done:\n";
                if($func_ipc->{TYPE} eq 'DESTRUCTOR') {
                    print FILE "    {\n";
                    print FILE "    bipc_t ipc = $ipc_arg;\n";
                    print FILE "    BKNI_Free($handle);\n";
                    print FILE "    bipc_client_end(ipc);\n";
                    print FILE "    }\n";
                }
                else {
                    print FILE "    bipc_client_end($ipc_arg);\n";
                }
                if($func_ipc->{FUNC}->{RETTYPE} ne 'void') {
                    print FILE "    return $ipc_result;\n";
                }
                print FILE "}\n";
            }
        } elsif ($mode eq 'server') {
            # Build server portion 
            print FILE "#include \"bipc_server.h\"\n\n";


            print FILE "static int b_ipc_${name}_process_native( void **$ipc_field, unsigned entry, union b_ipc_${name}_data * $ipc_data, size_t recv_size, bipc_send_data *send_data)\n";
            print FILE "{\n";
            print FILE "    int rc=0;\n";
            print FILE "    switch(entry) {\n";
            for (@apis) {
                my $func_ipc = $ipc->{$_};
                print FILE "    case b_ipc_$_:\n";
                print FILE "        if(recv_size == sizeof( $ipc_data ->$func_ipc->{FUNC}->{FUNCNAME} . in )) {\n";
                my ($send_offset,$send_size) = send_offset_and_size $name, $func_ipc;
                if($func_ipc->{TYPE} eq 'CONSTRUCTOR') {
                    print FILE "            $func_ipc->{FUNC}->{RETTYPE}  $ipc_result = \n";
                } elsif($func_ipc->{FUNC}->{RETTYPE} ne 'void') {
                    print FILE "            $ipc_data -> $func_ipc->{FUNC}->{FUNCNAME} .out. $ipc_result = \n";
                }
                print FILE "            $_(\n";
                print FILE "                ";
                print FILE join(",\n                ",@{$func_ipc->{SERVER_ARGS}});
                print FILE "\n           );\n";
                if($func_ipc->{TYPE} eq 'CONSTRUCTOR') {
                    print FILE "            * $ipc_field = $ipc_result;\n";
                    print FILE "            $ipc_data -> $func_ipc->{FUNC}->{FUNCNAME} .out. $ipc_result = $ipc_result!=NULL ? send_data->offset : BIPC_INSTANCE_ID_NEW;\n";
                }  
                print FILE "            send_data->offset = $send_offset;\n";
                print FILE "            send_data->size = $send_size;\n";
                print FILE "        } else { \n";
                print FILE "            rc = -1; \n";
                print FILE "        };\n";
                print FILE "        break;\n";
            }
            print FILE "    default:\n";
            print FILE "        rc = -1;\n";
            print FILE "        break;\n";
            print FILE "    }\n";
            print FILE "    return rc;\n";
            print FILE "}\n";
            if($ipc_mode eq 'verify') {
                my $u_name = "\U$name";
                print FILE "\n#if B_IPC_COMPAT_SUPPORT\n";
                print FILE "\nB_${u_name}_COMPAT_BEGIN($name)\n";
                for (@apis) {
                    my $func_ipc = $ipc->{$_};
                    print FILE "  B_${u_name}_API_PROCESS_BEGIN($name,$_)\n";
                    print FILE "    B_${u_name}_API_CONVERT_IN($name,$_)\n";
                    print FILE "    B_${u_name}_API_CALL($name,$_)\n";
                    my $out_data = $ipc->{$_}{"DATA_OUT"};
                    if(scalar @$out_data) {
                        print FILE "    B_${u_name}_API_CONVERT_OUT($name,$_)\n";
                    }
                    print FILE "  B_${u_name}_API_PROCESS_END($name,$_)\n";
                }
                print FILE "B_${u_name}_COMPAT_END($name)\n\n";
                print FILE "#else /* B_IPC_COMPAT_SUPPORT */ \n";
            }
            print FILE "static int b_ipc_${name}_process(unsigned abi, void **$ipc_field, unsigned entry, void * $ipc_data, size_t recv_size, bipc_send_data *send_data)\n";
            print FILE "{return b_ipc_${name}_process_native( $ipc_field, entry, $ipc_data, recv_size, send_data);}\n";
            if($ipc_mode eq 'verify') {
                print FILE "#endif /* B_IPC_COMPAT_SUPPORT */ \n";
            }

            print FILE "const bipc_server_descriptor bipc_${name}_server_descriptor = {\n";
            print FILE "    {\n";
            print FILE "        \"$name\",\n";
            print FILE "        \"$name\",\n";    
            print FILE "        sizeof(union b_ipc_${name}_data)\n";
            print FILE "    },\n";
            print FILE "    b_ipc_${constructor},\n";
            print FILE "    b_ipc_${destructor},\n";
            print FILE "    b_ipc_${name}_process\n";
            print FILE "};\n";
        } elsif ($mode eq 'verify') {
            # do nothing per class
        } else {
            die "unsupported mode '$mode'";
        }
    }
    if($mode eq 'verify') {
        my $expanded_ipc_types = bapi_parse_c::copy_structs(\%ipc_types);
        bapi_parse_c::expand_structs($expanded_ipc_types, \%referenced_structs);
        bapi_verify::print_struct(\*FILE, $expanded_ipc_types, [], 1);
        bapi_verify::verify_struct(\*FILE, $expanded_ipc_types, [], 'ipc');
    }
    close FILE;
}

my $file;
my @funcs;
my %structs;

my $verify_file;
my $server_file = shift @ARGV;
if($server_file eq '-verify') {
    $ipc_mode = 'verify';
    $server_file = shift @ARGV;
}
my $client_file = shift @ARGV;
my $verify_file;
if($ipc_mode eq 'verify') {
    $verify_file = shift @ARGV;
}
my %api_files;

for $file (@ARGV) {
    $api_files{$file}=1;
    push @funcs, bapi_parse_c::get_func_prototypes $file;

    my $file_structs = bapi_parse_c::parse_struct $file;
    my $name;
    my $members;
    while (($name, $members) = each %$file_structs) {
        $structs{$name} = $members;
    }
}

if($ipc_mode eq 'verify') {
    while(<STDIN>) {
        chomp;
        s/^[^:]+://;
        s!\\!!;
        while(/(\S+)/sg) {
            my $file = $1;
            if($file =~ m!([^/]+)$!) {
                if(not exists $api_files{$1}) {
                    my $file_structs = bapi_parse_c::parse_struct $file;
                    my ($name, $members);
                    while (($name, $members) = each %$file_structs) {
                        $structs{$name} = $members;
                    }
                }
            }
        }
    }
}

my @funcrefs = parse_funcs @funcs;

my $classes = group_api @funcrefs;

make_ipc $classes,  'server', $server_file, \%structs;
make_ipc $classes,  'client', $client_file, \%structs;
if($ipc_mode eq 'verify') {
    make_ipc $classes,  'verify', $verify_file, \%structs;
}

