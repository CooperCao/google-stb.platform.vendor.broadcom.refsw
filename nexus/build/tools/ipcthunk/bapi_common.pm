#!/usr/bin/perl
#  Broadcom Proprietary and Confidential. (c)2003-2016 Broadcom. All rights reserved.
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
use strict;
use bapi_util;

package bapi_common;

sub version_value
{
    my $version = shift;
    sprintf " 0x%08Xul ", ($version & 0xFFFFFFFF);
}

sub version_ipc
{
    my $module = shift;
    return "B_IPC_${module}_NEXUS_INIT";
}

sub version_define
{
    my $module = shift;
    return "NEXUS_${module}_MODULE_VERSION";
}

sub ipc_name
{
    my $module = uc shift;
    my $func = shift;

    return "B_IPC_${module}_$func->{FUNCNAME}";
}

sub ipc_struct
{
    my $module = uc shift;
    my $func = shift;
    return "B_IPC_${module}_$func->{FUNCNAME}_data";
}

sub ipc_block
{
    my $module = uc shift;
    return "B_IPC_${module}_data";
}

sub ipc_header
{
    my $module = shift;
    my $module_lc = lc $module;

    return "nexus_${module_lc}_ipc_api.h";
}

sub process_function_param {
    my $func = shift;
    my $param = shift;
    my @ipc_inparams;
    my @ipc_outparams;
    my $driver_arg;
    my $completed=0;
    my $null_allowed=0;
    my $name = $param->{NAME};
    my $driver_in_arg = "in_data->data.$func->{FUNCNAME}.in";
    my $driver_out_arg = "out_data->data.$func->{FUNCNAME}.out";
    my $variable_params;

    if (exists $param->{ATTR}) {
        my $attr;
        my $value;
        while (($attr, $value) = each %{$param->{ATTR}} ) {
            next if($attr eq 'nelem_out');
            next if($attr eq 'nelem_convert');
            if($attr eq 'reserved') {
                my $len;
                if($param->{BASETYPE} ne 'void') {
                    $len = " $value * sizeof( $param->{BASETYPE} )";
                } else {
                    $len = $value;
                }
                $variable_params = $len;
            } elsif($attr eq 'nelem') {
                if ($param->{INPARAM}) {
                    push @ipc_inparams, "unsigned ${name}_variable_offset;";
                    $driver_arg = "(void *)((uint8_t*)in_data + $driver_in_arg.${name}_variable_offset)";
                }
                else {
                    push @ipc_outparams, "unsigned ${name}_variable_offset;";
                    $driver_arg = "$driver_in_arg.${value}?(void *)((uint8_t *)out_data + $driver_out_arg.${name}_variable_offset):NULL";
                }
                $completed = 1;
            }
            elsif($attr eq 'memory' && $value eq 'cached' ) {
                if ($param->{INPARAM}) {
                    push @ipc_inparams, "NEXUS_Ipc_DeviceAddress $param->{NAME};";
                    $driver_arg = "NEXUS_OffsetToCachedAddr((unsigned long)$driver_in_arg.$name)";
                }
                else {
                    push @ipc_outparams, "$param->{BASETYPE} $param->{NAME};";
                    $driver_arg = "($param->{TYPE})&$driver_out_arg.$name";
                }
                $completed = 1;
            }
            elsif($attr eq 'null_allowed') {
                $null_allowed = $value;
            }
            elsif($attr eq 'handle_verify' && $value eq 'no' ) {
            }
            else {
                print STDERR "ERROR: Unsupported attribute $attr = $value for $param->{NAME} in $func->{FUNCNAME}\n";
            }
        }
    }

    if($completed) {
    }
    elsif ($param->{ISREF} && $param->{BASETYPE} ne 'void') {
        if ($param->{INPARAM}) {
            push @ipc_inparams, "$param->{BASETYPE} $name;";
            $driver_arg = "($driver_in_arg.$name\_isnull ? NULL : &$driver_in_arg.$name )";
            push @ipc_inparams, "bool $name\_isnull;";
        } else {
            push @ipc_outparams, "$param->{BASETYPE} $name;";
            $driver_arg = "&$driver_out_arg.$name";
        }
    }
    else {
        # must be inparam
        push @ipc_inparams, "$param->{TYPE} $param->{NAME};";
        $driver_arg = "$driver_in_arg.$name";
    }
    my %result;
    $result{'ipc_inparams'} = \@ipc_inparams;
    $result{'ipc_outparams'} = \@ipc_outparams;
    $result{'driver_arg'} = $driver_arg;
    $result{'variable_params'} = $variable_params;
    return \%result;
}

sub process_function_attributes {
    my $func = shift;
    my $structs = shift;
    my $funcs = shift;
    my $params = $func->{PARAMS};
    my @proxy_post_success;
    my @proxy_pre_call;
    my @driver_pre_call;
    my @driver_post_success;
    my @driver_post_error;
    my @driver_post_always;
    my @proxy_vars;
    my %driver_overwrite;
    my ($attr,$value);
    my $param;
    my $field;
    my $arg_no=0;
    my $driver_in_arg = "in_data->data.$func->{FUNCNAME}.in";
    my $driver_out_arg = "out_data->data.$func->{FUNCNAME}.out";

    if(exists $func->{ATTR}) {
        while (($attr, $value) = each %{$func->{ATTR}} ) {
            next if($attr eq 'destructor');
            next if($attr eq 'release');
            next if($attr eq 'local');
            next if($attr eq 'thunk');
            next if($attr eq 'shutdown');
            print STDERR "ERROR: Unsupported attribute $attr = $value for $func->{FUNCNAME}\n";
        }
    }

    for $param (@$params) {
        my $name = $param->{NAME};
        $arg_no ++;

        if (!exists $param->{ATTR}->{'nelem'} &&
            !exists $param->{ATTR}->{'memory'})
        {
            # ordinary params
            if ($param->{ISREF} && $param->{BASETYPE} ne 'void') {
                my $null_allowed = $param->{ATTR}->{'null_allowed'};
                if ($param->{INPARAM}) {
                    if (!defined $null_allowed) {
                        push @driver_pre_call, "if ($driver_in_arg.$name\_isnull) {";
                        push @driver_pre_call, "    BDBG_ERR((\"NULL not allowed for $name param in $func->{FUNCNAME}: %d\", __LINE__));";
                        push @driver_pre_call, "    goto err_fault;";
                        push @driver_pre_call, "}";
                    }
                    push @proxy_pre_call, "if($name!=NULL) {";
                    push @proxy_pre_call, "    $driver_in_arg.$name\_isnull = false;";
                    push @proxy_pre_call, "    $driver_in_arg.$name = * $name;";
                    push @proxy_pre_call, "} else {";
                    push @proxy_pre_call, "    $driver_in_arg.$name\_isnull = true;";
                    push @proxy_pre_call, "}";
                } else {
                    if (defined $null_allowed) {
                        push @proxy_post_success, "if ($name) {";
                        push @proxy_post_success, "  * $name = $driver_out_arg.$name;";
                        push @proxy_post_success, "}";
                    }
                    else {
                        push @proxy_post_success, "* $name = $driver_out_arg.$name;";
                    }
                }
            }
            else {
                # must be inparam
                push @proxy_pre_call, "$driver_in_arg.$name = $name;";
            }
        }

        if($param->{BASETYPE} eq 'NEXUS_CallbackDesc') {
            if ($param->{INPARAM}) {
                my $handle = "$driver_in_arg.$$params[0]->{NAME}";
                my $id = sprintf("0x%05x", ((bapi_util::func_id $funcs, $func)*256 + $arg_no + 0x10000));
                my $notnull_test = "! $driver_in_arg.$name\_isnull";
                push @driver_pre_call, "if($notnull_test) {";
                push @driver_pre_call, "   /* MAP callback from proxy space to the driver space */";
                push @driver_pre_call, "   NEXUS_DRIVER_CALLBACK_TO_DRIVER(&$driver_in_arg.$name, $handle, $id);";
                push @driver_pre_call, "}";
                push @driver_post_success, "if($notnull_test) {";
                push @driver_post_success, "    /* after function succeded, commit changes */";
                push @driver_post_success, "    NEXUS_DRIVER_CALLBACK_TO_DRIVER_COMMIT(&$driver_in_arg.$name, $handle, $id);";
                push @driver_post_success, "}";
                push @driver_post_error, "if($notnull_test) {";
                push @driver_post_error, "    /* if error occurred, then cancel changes in the callback mapping */";
                push @driver_post_error, "    NEXUS_DRIVER_CALLBACK_TO_DRIVER_CANCEL(&$driver_in_arg.$name, $handle, $id);";
                push @driver_post_error, "}";
            } else {
                print STDERR "ERROR: pointer to NEXUS_CallbackDesc could be only input parameter\n";
            }
        }
        
        if (exists $structs->{$param->{BASETYPE}}) {
            my $field_no = 0;
            my @pre_update_proxy;
            my @pre_update_driver;
            my @post_update_proxy;
            my @post_update_driver;
            my @post_error_driver;

#            bapi_parse_c::print_struct $structs->{$param->{BASETYPE}}
            for $field (@{$structs->{$param->{BASETYPE}}}) {
                my $field_driver_in = "$driver_in_arg.$name.$field->{NAME}";
                my $field_driver_out = "$driver_out_arg.$name.$field->{NAME}";

                $field_no ++;
                if($field->{TYPE} eq 'NEXUS_CallbackDesc') {
                    my $id = sprintf("0x%04x", ((bapi_util::struct_id $structs, $param->{BASETYPE})*256 + $field_no));
                    my $handle;
                    if ($func->{RETTYPE_ISHANDLE}) {
                        $handle = 'NULL'; # update after sucess
                    }
                    elsif ($$params[0]->{ISHANDLE}) {
                        $handle = "$driver_in_arg.$$params[0]->{NAME}";
                    }
                    elsif (!$$params[0]->{ISREF} && $param != $$params[0]) {
                        # enum instead of handle.
                        # use enum type + enum value as psuedo handle
                        # hardcoded for a max of 128 enum values per type
                        # value<<2+1 to use non-4 byte aligned value to ensure no conflict with actual handle
                        $handle = "(unsigned long)(($id<<9) + ($driver_in_arg.$$params[0]->{NAME}<<2) + 1)";
                    }
                    if (defined $handle && $param->{ISREF}) {
                        if ($param->{INPARAM}) {
                            push @pre_update_driver, "/* MAP callback from proxy space to the driver space */";
                            push @pre_update_driver, "NEXUS_DRIVER_CALLBACK_TO_DRIVER(&$field_driver_in, $handle, $id);";
                            if($handle eq 'NULL') {
                                push @post_update_driver, "/* since callback wasn't avaliable before calling function, use the return result to update callback */";
                                push @post_update_driver, "NEXUS_DRIVER_CALLBACK_UPDATE(&$field_driver_in, $handle, $id, $driver_out_arg.__retval);";
                            } else {
                                push @post_update_driver, "/* after function succeded, commit changes */";
                                push @post_update_driver, "NEXUS_DRIVER_CALLBACK_TO_DRIVER_COMMIT(&$field_driver_in, $handle, $id);";
                            }
                            push @post_error_driver, "/* if error occurred, then cancel changes in the callback mapping */";
                            push @post_error_driver, "NEXUS_DRIVER_CALLBACK_TO_DRIVER_CANCEL(&$field_driver_in, $handle, $id);";
                        } else {
                            push @post_update_driver, "/* MAP callback from the driver space to proxy space */";
                            push @post_update_driver, "NEXUS_DRIVER_CALLBACK_TO_CLIENT(&$field_driver_out, $handle, $id);";
                        }
                    }
                }

                if (exists $field->{ATTR}) {
#                   print " $func->{FUNCNAME} :";
#                   print "  field: $field->{TYPE} $field->{NAME} -> ";
#                   bapi_parse_c::print_attr $field->{ATTR};
                    while (($attr, $value) = each %{$field->{ATTR}} ) {
                        my $field_proxy = "$name->$field->{NAME}";
                        my $field_copy = "$driver_in_arg.$name.$field->{NAME}";
                        if($attr eq 'kind' && $value eq 'null_ptr' ) {
                            next;
                        } elsif($attr eq 'memory') {
                            if($value eq 'cached') {
                                if ($param->{ISREF}) {
                                    if ($param->{INPARAM}) {
                                        next if (exists $param->{ATTR}->{'nelem'});
                                        push @pre_update_proxy, "/* convert address to the device offset */";
                                        push @pre_update_proxy, "if ($field_copy) {$field_copy = (void *)(unsigned long) NEXUS_AddrToOffset($field_copy); if (!$field_copy) __rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);}";

                                        push @pre_update_driver, "/* convert offset (substituted by the proxy ) to the cached address */";
                                        push @pre_update_driver, "NEXUS_DRIVER_RECV_ADDR($field_driver_in,cached);";
                                        next;
                                    } else {
                                        push @post_update_proxy, "/* convert offset (substituted by the driver) to the cached address */";
                                        push @post_update_proxy, "$field_proxy = $field_proxy ? NEXUS_OffsetToCachedAddr((unsigned long)$field_proxy) : $field_proxy ;";

                                        push @post_update_driver, "/* convert offset (substituted by the proxy) to the cached address */";
                                        push @post_update_driver, "NEXUS_DRIVER_SEND_ADDR($field_driver_out,cached);";
                                        next;
                                    }
                                }
                            }
                        }
                        print STDERR "ERROR: Unsupported attribute $attr = $value for $field->{NAME} in $param->{BASETYPE}\n";
                    }
                }
            }
            if(scalar @pre_update_proxy) {
                push @proxy_pre_call, "if($name) {";
                push @proxy_pre_call, "     /* update values ... */";
                bapi_util::append_code \@proxy_pre_call, \@pre_update_proxy, "     ";
                push @proxy_pre_call, "}";
            }
            bapi_util::append_if \@driver_pre_call, "! $driver_in_arg.$name\_isnull", \@pre_update_driver;
            if ($param->{INPARAM} && defined $param->{ATTR}->{'null_allowed'}) {
                bapi_util::append_if \@driver_post_error, "! $driver_in_arg.$name\_isnull", \@post_error_driver;
                bapi_util::append_if \@driver_post_success, "! $driver_in_arg.$name\_isnull", \@post_update_driver;
            }
            else {
                # out params cannot be null
                bapi_util::append_code \@driver_post_error, \@post_error_driver;
                bapi_util::append_code \@driver_post_success, \@post_update_driver;
            }
            bapi_util::append_code \@proxy_post_success , \@post_update_proxy;
        }
        
        if(exists $param->{ATTR}) {
            while (($attr, $value) = each %{$param->{ATTR}} ) {
                if($attr eq 'nelem_convert' || $attr eq 'nelem_out') {
                    if( !exists $param->{ATTR}->{'nelem'}) {
                        print STDERR "ERROR: $attr requires nelem: $func->{FUNCNAME}\n";
                    }
                    next; # handled with nelem
                }
                next if($attr eq 'null_allowed'); # skip here
                next if($attr eq 'reserved'); # unnecessary for socket ipc

                if($attr eq 'nelem') {
                    if ($param->{INPARAM}) {
                        my $length = "$value";
                        my $array_type;

                        if (exists $param->{ATTR}->{'nelem_convert'}) {
                            $length =  $param->{ATTR}->{'nelem_convert'} . "($length)";
                        }
                        if($param->{BASETYPE} ne 'void') {
                            $length = "(sizeof($param->{BASETYPE})*($length))";
                            $array_type = $param->{BASETYPE};
                        }
                        else {
                            $array_type = "uint8_t";
                        }

                        push @proxy_pre_call, "if (__variable_in_offset+B_IPC_DATA_ALIGN($length)>__data_size) {NEXUS_P_CLIENT_IN_REALLOC(B_IPC_DATA_ALIGN($length));}";
                        push @proxy_pre_call, "$driver_in_arg.${name}_variable_offset = __variable_in_offset ;";
                        push @proxy_pre_call, "{";
                        push @proxy_pre_call, "$array_type *dest = ($array_type*)((uint8_t *)in_data + $driver_in_arg.${name}_variable_offset);";
                        push @proxy_pre_call, "  BKNI_Memcpy(dest, $name, $length);";
                        push @proxy_pre_call, "  __variable_in_offset += B_IPC_DATA_ALIGN($length);";

                        # Handle address translation
                        my @driver_update_structure;
                        my @proxy_update_structure;
                        if (exists $structs->{$param->{BASETYPE}}) {
                            my $field;
                            for $field (@{$structs->{$param->{BASETYPE}}}) {
                                if (exists $field->{ATTR}) {
                                    my ($attr,$value);
                                    while (($attr, $value) = each %{$field->{ATTR}} ) {
                                        if( $attr eq 'memory' && $value eq 'cached' && $param->{ISREF} && $param->{INPARAM}) {
                                            my $field_copy = "dest[i].$field->{NAME}";

                                            push @proxy_update_structure, "    /* convert address to the device offset */";
                                            push @proxy_update_structure, "    if ($field_copy) {$field_copy = (void *)(unsigned long) NEXUS_AddrToOffset($field_copy); if (!$field_copy) __rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);}";

                                            push @driver_update_structure, "    /* convert offset (substituted by the client) to the cached address */";
                                            push @driver_update_structure, "    NEXUS_DRIVER_RECV_ADDR($field_copy,cached);";
                                            next;
                                        }
                                        print STDERR "ERROR: Unsupported attribute $attr = $value for $field->{NAME} in $param->{BASETYPE}\n";
                                    }
                                }
                            }
                        }
                        if (scalar @driver_update_structure) {
                            # no dest pointer needed unless field conversion is required
                            push @driver_pre_call, "{";
                            push @driver_pre_call, "  $array_type *dest = ($array_type*)((uint8_t *)in_data + $driver_in_arg.${name}_variable_offset);";
                            push @driver_pre_call, "  {unsigned i;\n  for(i=0;i<$driver_in_arg.$value;i++) {";
                            push @driver_pre_call, @driver_update_structure;
                            push @driver_pre_call, "  }}";
                            push @driver_pre_call, "}";
                        }
                        if (scalar @proxy_update_structure) {
                            push @proxy_pre_call, "  {unsigned i;\n  for(i=0;i<$value;i++) {";
                            push @proxy_pre_call, @proxy_update_structure;
                            push @proxy_pre_call, "  }}";
                        }
                        push @proxy_pre_call, "}";
                    }
                    else {
                        my $driver_length = "$driver_in_arg.$value";
                        my $proxy_length = "$value";
                        my $proxy_length_out;

                        if (exists $param->{ATTR}->{'nelem_out'}) {
                            $proxy_length_out = "$driver_out_arg.$param->{ATTR}->{'nelem_out'}";
                        }
                        else {
                            $proxy_length_out = $proxy_length;
                        }
                        if (exists $param->{ATTR}->{'nelem_convert'}) {
                            $proxy_length_out =  $param->{ATTR}->{'nelem_convert'} . "($proxy_length_out)";
                        }

                        if($param->{BASETYPE} ne 'void') {
                            $proxy_length_out = "(sizeof($param->{BASETYPE})*($proxy_length_out))";
                            $proxy_length = "(sizeof($param->{BASETYPE})*($proxy_length))";
                            $driver_length = "(sizeof($param->{BASETYPE})*($driver_length))";
                        }

                        push @driver_pre_call, "if (__variable_out_offset + $driver_length>out_mem_size) {NEXUS_P_SERVER_OUT_REALLOC(__variable_out_offset + $driver_length - out_mem_size);}";
                        push @driver_pre_call, "$driver_out_arg.${name}_variable_offset = __variable_out_offset;";
                        push @driver_pre_call, "__variable_out_offset += B_IPC_DATA_ALIGN($driver_length);";
                        push @proxy_post_success, "if ($name) BKNI_Memcpy($name, (uint8_t *)out_data + $driver_out_arg.${name}_variable_offset, $proxy_length_out);";
                    }
                }
                elsif($attr eq 'memory' && $value eq 'cached' ) {
                    if ($param->{INPARAM}) {
                        push @proxy_pre_call, "/* convert offset (substituted by the driver) to the cached address */";
                        push @proxy_pre_call, "if ($name) {$driver_in_arg.$name = (void *)(unsigned long) NEXUS_AddrToOffset((void *)$name); if (!$driver_in_arg.$name) __rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);}";
                        # driver_pre_call NEXUS_OffsetToCachedAddr handled in process_function_param
                    }
                    else {
                        # handle return value. if client passes NULL outparam, only the client crashes
                        push @proxy_post_success, "/* convert offset (substituted by the driver) to the cached address */";
                        push @proxy_post_success, "*$name = $driver_out_arg.$name ? NEXUS_OffsetToCachedAddr((unsigned long)$driver_out_arg.$name) : NULL;";
                        push @driver_post_success, "/* convert address to the device offset */";
                        push @driver_post_success, "NEXUS_DRIVER_SEND_ADDR($driver_out_arg.$name, cached);";
                    }
                }
                elsif($attr eq 'handle_verify' && $value eq 'no' ) {
                }
                else {
                    print STDERR "ERROR: Unsupported attribute $attr = $value for $param->{NAME} in $func->{FUNCNAME}\n";
                }
            }
        }
    }
    
    my %result;
    if (scalar @proxy_post_success) {
        $result{'proxy_post_success'} = \@proxy_post_success;
    }
    if (scalar @driver_post_success) {
        $result{'driver_post_success'} = \@driver_post_success;
    }
    if (scalar @proxy_pre_call) {
        $result{'proxy_pre_call'} = \@proxy_pre_call;
    }
    if (scalar @driver_pre_call) {
        $result{'driver_pre_call'} = \@driver_pre_call;
    }
    if (scalar @proxy_vars) {
        $result{'proxy_vars'} = \@proxy_vars;
    }
    if (scalar @driver_post_error) {
        $result{'driver_post_error'} = \@driver_post_error;
    }
    if (scalar @driver_post_always) {
        $result{'driver_post_always'} = \@driver_post_always;
    }

    $result{'driver_overwrite'} = \%driver_overwrite;
    return \%result;
}


1;

