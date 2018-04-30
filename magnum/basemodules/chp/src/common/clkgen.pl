#!/usr/bin/perl
#############################################################################
#   Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#############################################################################

use strict;
use warnings;

package clkResource;

use Data::Dumper;
$Data::Dumper::Sortkeys  = 1;
$Data::Dumper::Useqq    = 1;
$Data::Dumper::Indent   = 1;
$Data::Dumper::Terse    = 1;

my @profiles = ('Standard');
my $num_profiles = scalar @profiles;
my $num_pmap_settings = 0;

my $disable_vec_656 = 0;
my $disable_aud_dac = 0;
my $num_aud_dacs = 0;
my $num_aud_plls = 0;
my $num_vcxo_plls = 0;

my $perf;

my %parent_map;

my %xpt_wakeup = (
    xpt_core_source => "unknown",
);

my @nexus_functions = (
        "AIO",
        "AUD_PLL0", "AUD_PLL1",
        "BVN",
        "DVPHR", "DVPHR0", "DVPHR1",
        "DVPHT", "DVPHT0", "DVPHT1",
        "HVD", "HVD0", "HVD1", "HVD2",
        "ITU656",
        "M2MC", "M2MC0", "M2MC1",
        "RAAGA", "RAAGA0", "RAAGA1",
        "RFM",
        "SC", "SC0", "SC1", "PAD",
        "SID", "SID0", "SID1",
        "V3D", "V3D0", "V3D1",
        "VDAC",
        "VEC",
        "VICE2", "VICE20", "VICE21",
        "XPT",
        "VIP",
        "SECBUS",
        "AFEC", "AIF", "CHAN",
        "DSEC", "FSK", "LEAP",
        "SDS",
        "ASP",
        "UFE", "DS",
        "UNCLAIMED" #Used for Plls that are not claimed by anyone
    );

my @div_functions = (
        "HVD", "HVD0", "HVD1", "HVD2",
        "RAAGA", "RAAGA0", "RAAGA1",
        "M2MC", "M2MC0", "M2MC1",
        "MM_M2MC", "MM_M2MC0",
        "V3D", "V3D0", "V3D1",
        "VICE2", "VICE20", "VICE21",
        "SID", "SID0", "SID1",
        "XPT", "ASP", "UFE"
    );

sub is_nexus_function {
    my $function = shift;

    foreach (@nexus_functions) {
        if(index($function, $_) > -1) {return 1;}
    }

    return 0;
}

sub is_div_function {
    my $function = shift;

    foreach (@div_functions) {
        if(index($function, $_) > -1) {return 1;}
    }

    return 0;
}

sub new {
    my $class = shift;
    my ($resource, $reg, $field, $default) = @_;

    my $self = {
        _name => get_name($resource),
        _reg => $reg,
        _field => $field,
        _default => $default,
        _funcs => [],
        _toplvl => [],
        _srcs => [],
        _delay => 0,
        _pda => "",
        _polarity => "",
        _div => "",
        _mux => 0,
    };
    bless($self, $class);
    return($self);
}

my @modified_function_list = (
    "XPT_CORE", "XPT_TSIO", "ITU656",
    "AUD_PLL0", "AUD_PLL1", "AUD_PLL2"
);

sub verify_parent {
    my ($clk_tree, $resource) = @_;
    my $index = 0;

    for ($index = @{$clk_tree->{$resource}{_funcs}}; $index > 0; $index--) {
        my $found = 0;
        my $func = $clk_tree->{$resource}{_funcs}[$index-1];
        foreach my $parent (@{$parent_map{$resource}}) {
            if(grep(/\b$func\b/, @{$clk_tree->{$parent}{_funcs}})) {
                $found = 1;
                last;
            }
        }
        if($found == 0) {
            print "    Removing $func from $resource\n";
            splice(@{$clk_tree->{$resource}{_funcs}}, $index-1, 1);
            splice(@{$clk_tree->{$resource}{_toplvl}}, $index-1, 1);
        }
    }
}

sub verify_sub_tree {
    my ($clk_tree, $src, $func) = @_;

    foreach (@{$clk_tree->{$src}{_srcs}}) {
        if($_ eq "null") {next;}
        if(exists $clk_tree->{$_}) {
            if (!grep(/\b$func\b/, @{$clk_tree->{$_}{_funcs}})) {
                if (grep(/\b$func\b/, @modified_function_list)) {
                    print "    Adding $func to $_\n";
                    push(@{$clk_tree->{$_}{_funcs}}, $func);
                    push(@{$clk_tree->{$_}{_toplvl}}, $func);
                    verify_parent($clk_tree, $_);
                } else {
                    print "    $func Missing in $_\n";
                }
                verify_sub_tree($clk_tree, $_, $func);
            }
        }
    }
}

sub verify_tree {
    my $clk_tree = shift;

    for (keys %$clk_tree) {
        my $cnt = 0;
		foreach my $src (@{$clk_tree->{$_}{_srcs}}) {
            if($src eq "null") {next;}
			if(exists $clk_tree->{$src}) {
                foreach my $func (@{$clk_tree->{$_}{_funcs}}) {
                    if (!grep(/\b$func\b/, @{$clk_tree->{$src}{_funcs}})) {
                        print "$func : Exists in $_\n";
                        if (grep(/\b$func\b/, @modified_function_list)) {
                            print "    Adding $func to $src\n";
                            push(@{$clk_tree->{$src}{_funcs}}, $func);
                            push(@{$clk_tree->{$src}{_toplvl}}, $func);
                            verify_parent($clk_tree, $src);
                        } else {
                            print "    $func Missing in $src\n";
                        }
                        verify_sub_tree($clk_tree, $src, $func);
					}
				}
			} else {
				print "$src does not exist in tree. Removing it from $_"."'s source\n";
                splice(@{$clk_tree->{$_}{_srcs}}, $cnt, 1);
                delete $parent_map{$src};
			}
            $cnt++;
		}
    }
}

sub get_name {
    my $name = shift;

    $name =~ s/CLKGEN_//g;
    $name =~ s/ONOFF_//g;
    $name =~ s/PLL_//g;
    $name =~ s/CLOCK_//g;
    $name =~ s/_CLOCK$//;
    $name =~ s/CLK//g;
    $name =~ s/ENABLE\d*_//g;
    $name =~ s/_ENABLE\d*$//;
    $name =~ s/DISABLE\d*_//g;
    $name =~ s/_DISABLE\d*$//;
    $name =~ s/INST_//g;
    $name =~ s/DIVIDER/DIV/g;
    $name =~ s/CHANNEL/CH/g;
    $name =~ s/TOP_//g;

	return $name;
}

sub get_pda {
    my $func = shift;

    if ($1 =~ /PDAW/){
        return "WRITE";
    } elsif ($1 =~ /PDAR/) {
        return "READ";
    } else  {
        return "";
    }
}

sub verify_resource {
    my ($clk_tree, $resource, $reg, $field, $default, $reg_desc) = @_;

    if (not defined $resource) {
        return;
    }
    if ($resource =~ /reserved/) {
        return;
    }

    # TODO : Figure out how to find the AUD_PLL MUX resource
    if ($resource =~ /AUDIO(\d)_OSCREF_CMOS_CLOCK/ || $resource =~ /PLLAUDIO(\d)_REFERENCE_CLOCK/) {
        if (!exists $clk_tree->{$resource}) {
            if ($1 < $num_aud_plls && $num_vcxo_plls > 1) {
                print "Adding resource $resource : AUD_PLL$1\n";
                $clk_tree->{$resource} = clkResource->new($resource, $reg, $field, $default);
                push(@{$clk_tree->{$resource}{_funcs}}, "AUD_PLL".$1);
                push(@{$clk_tree->{$resource}{_toplvl}}, "AUD_PLL".$1);
                push(@{$clk_tree->{$resource}{_srcs}}, "CLKGEN_PLL_VCXO0_PLL_CHANNEL_CTRL_CH_2_POST_DIVIDER_HOLD_CH2");
                push(@{$clk_tree->{$resource}{_srcs}}, "CLKGEN_PLL_VCXO1_PLL_CHANNEL_CTRL_CH_2_POST_DIVIDER_HOLD_CH2");
                $clk_tree->{$resource}{_mux} = 2;
            }
        }
    } elsif ($resource =~ /PM_CLOCK_Async_ALIVE_SEL_CLOCK_Async_CG_XPT/ || $resource =~ /PM_PLL_ALIVE_SEL/) {
        if (!exists $clk_tree->{$resource}) {
            if ($resource =~ /PM_PLL_ALIVE_SEL/ && $resource !~ /$xpt_wakeup{xpt_core_source}/) {
                #Save this resource incase we don't know the Xpt core's source yet and need to figure out the keep alive pll later
                $xpt_wakeup{pll_alive_sel}{$resource}{_reg} = $reg;
                $xpt_wakeup{pll_alive_sel}{$resource}{_field} = $field;
                $xpt_wakeup{pll_alive_sel}{$resource}{_default} = $default;
            } else {
                print "Adding resource $resource : XPT_WAKEUP\n";
                $clk_tree->{$resource} = clkResource->new($resource, $reg, $field, $default);
                push(@{$clk_tree->{$resource}{_funcs}}, "XPT_WAKEUP");
                push(@{$clk_tree->{$resource}{_toplvl}}, "XPT_WAKEUP");
                push(@{$clk_tree->{$resource}{_srcs}}, "null");
                $clk_tree->{$resource}{_polarity} = "HighIsOn";
            }
        } else {
            if ($resource =~ /PM_PLL_ALIVE_SEL/ && $resource !~ /$xpt_wakeup{xpt_core_source}/) {
                print "Removing resource $resource : $clk_tree->{$resource}{_funcs}\n";
                delete $clk_tree->{$resource};
            }
        }
    } elsif ($resource =~ /XPT_CORE_CLOCK/) {
        if (exists $clk_tree->{$resource}) {
            foreach my $src (@{$clk_tree->{$resource}{_srcs}}) {
                if ($src =~ /(XPT|SYS)(\d?)_PLL/) {
                    $xpt_wakeup{xpt_core_source} = $1.$2;
                }
            }

            # Try to find a match incase the xpt core source was found after keep alive plls
            if (exists $xpt_wakeup{pll_alive_sel}) {
                foreach my $pll (keys %{$xpt_wakeup{pll_alive_sel}}) {
                    if ($pll =~ /$xpt_wakeup{xpt_core_source}/) {
                        print "Adding resource $pll : XPT_WAKEUP\n";
                        $clk_tree->{$pll} = clkResource->new($pll, $xpt_wakeup{pll_alive_sel}{$pll}{_reg}, $xpt_wakeup{pll_alive_sel}{$pll}{_field}, $xpt_wakeup{pll_alive_sel}{$pll}{_default});
                        push(@{$clk_tree->{$pll}{_funcs}}, "XPT_WAKEUP");
                        push(@{$clk_tree->{$pll}{_toplvl}}, "XPT_WAKEUP");
                        push(@{$clk_tree->{$pll}{_srcs}}, "null");
                        $clk_tree->{$pll}{_polarity} = "HighIsOn";
                        last;
                    }
                }
            }
        }
    } elsif ($resource =~ /PLL_CHANNEL_CTRL_CH/) {
        foreach (keys %$reg_desc) {
            if($_ eq $reg) {
                if (!exists $clk_tree->{$resource}) {
                    if ($resource =~ /CLOCK_DIS_CH(\d)/) {
                        print "Adding resource $resource : @{$reg_desc->{$_}{_funcs}}\n";
                        $clk_tree->{$resource} = clkResource->new($resource, $reg, $field, $default);
                        push(@{$clk_tree->{$resource}{_funcs}}, @{$reg_desc->{$_}{_funcs}});
                        push(@{$clk_tree->{$resource}{_toplvl}}, @{$reg_desc->{$_}{_funcs}});
                        push(@{$clk_tree->{$resource}{_srcs}}, $reg."_MDIV_CH".$1);
                        $clk_tree->{$resource}{_polarity} = "HighIsOff";
                    } elsif ($resource =~ /MDIV_CH(\d)/) {
                        print "Adding resource $resource : @{$reg_desc->{$_}{_funcs}}\n";
                        $clk_tree->{$resource} = clkResource->new($resource, $reg, $field, $default);
                        push(@{$clk_tree->{$resource}{_funcs}}, @{$reg_desc->{$_}{_funcs}});
                        push(@{$clk_tree->{$resource}{_toplvl}}, @{$reg_desc->{$_}{_funcs}});
                        $reg =~ s/CHANNEL_CTRL_CH_(\d)/DIV/;
                        push(@{$clk_tree->{$resource}{_srcs}}, $reg."_NDIV_INT");
                        $clk_tree->{$resource}{_div} = "POSTDIV";
                    } elsif ($resource =~ /POST_DIVIDER_HOLD_CH(\d)/) {
                        print "Adding resource $resource : @{$reg_desc->{$_}{_funcs}}\n";
                        $clk_tree->{$resource} = clkResource->new($resource, $reg, $field, $default);
                        push(@{$clk_tree->{$resource}{_funcs}}, @{$reg_desc->{$_}{_funcs}});
                        push(@{$clk_tree->{$resource}{_toplvl}}, @{$reg_desc->{$_}{_funcs}});
                        push(@{$clk_tree->{$resource}{_srcs}}, $reg."_CLOCK_DIS_CH".$1);
                        $clk_tree->{$resource}{_polarity} = "HighIsOff";
                    }
                } else {
                    my $new_funcs = join(' ', sort(@{$reg_desc->{$_}{_funcs}}));
                    my $old_funcs = join(' ', sort(@{$clk_tree->{$resource}{_funcs}}));
                    if ($new_funcs ne $old_funcs) {
                        print "Rename resource $resource : @{$clk_tree->{$resource}{_funcs}} ==> @{$reg_desc->{$_}{_funcs}}\n";
                        @{$clk_tree->{$resource}{_funcs}} = @{$reg_desc->{$_}{_funcs}};
                        @{$clk_tree->{$resource}{_toplvl}} = @{$reg_desc->{$_}{_funcs}};
                    }
                }
            }
        }
    }
    if (!exists $clk_tree->{$resource}) {
        if ($resource =~ /CLOCK_DIS_CH/ || $resource =~ /POST_DIVIDER_HOLD_CH/) {
            print "Adding resource $resource : UNCLAIMED\n";
            $clk_tree->{$resource} = clkResource->new($resource, $reg, $field, $default);
            push(@{$clk_tree->{$resource}{_funcs}}, "UNCLAIMED");
            push(@{$clk_tree->{$resource}{_toplvl}}, "UNCLAIMED");
            my $src = ($resource =~ /POST_DIVIDER_HOLD_CH(\d)/)?$reg."_CLOCK_DIS_CH".$1:"null";
            push(@{$clk_tree->{$resource}{_srcs}}, $src);
            $clk_tree->{$resource}{_polarity} = "HighIsOff";
        }
    }
}

sub rename_function {
    my ($clk, $func) = @_;
    my @func_arr;
    my %func_unique;

    if($func =~/\bXPT\b/) {
        if($clk =~ /DISABLE_XPT_(20P25|27|40P5|54|81)_CLOCK/) {
            $func =~ s/XPT/XPT_REMUX/g;
         } else {
            $func .= " XPT_REMUX";
         }
         if($clk =~ /XPT_CORE_CLOCK/) {
            $func .= " XPT_CORE";
         }
    }
    if($clk =~ /VCXO/) {
        if($func =~ /\bAIO\b/) {
            $func =~ s/AIO//g;
        }
        if($func =~ /\bVEC\b/) {
            $func =~ s/VEC//g;
        }
    }
    $func =~ s/\bHVD\b/HVD0/g;
    $func =~ s/\bHVD_PDA(R|W)\b/HVD0_PDA$1/g;
    $func =~ s/\bDVPHT\b/DVPHT0/g;
    $func =~ s/\bDVPHT_PDA(R|W)\b/DVPHT0_PDA$1/g;
    $func =~ s/\bDVPHRLP\b/DVPHR/g;
    $func =~ s/\bDVPHR\b/DVPHR0/g;
    $func =~ s/\bDVPHR_PDA(R|W)\b/DVPHR0_PDA$1/g;
    $func =~ s/\bBVN(.*?)_PDA(R|W)\b/BVN_PDA$2/g;
    $func =~ s/\bXPTWAKEUP\b/XPT_WAKEUP/g;
    $func =~ s/\bAIFSAT\b/AIFSAT0/g;
    $func =~ s/OBSERVE//g;
    if(not defined $perf) {
        $func =~ s/\bSEC\b//g;
    }

    @func_arr = split(/\s+/, $func);
    foreach (@func_arr) {
        $func_unique{$_}++;
    }
    @func_arr = (sort keys %func_unique);

    return @func_arr;
}

sub parse_rdb_file {
	my ($file, $clk_tree, $featuredef, $reg_desc, $coreprefix) = @_;
	my ($reg, $field, $resource, $default, $case);
    my $disabled = 0;
    my @files;
    my %tmp;

    #print "Parsing $file\n";
    open(INFILE,"$file") or die "Can't open input file $file";

    foreach (<INFILE>) {
        s/\r\n/\n/; # convert DOS to Unix
		s/^\s+//;   # Remove whitespace at beginning of line
		s/\s+$//;   # Remove whitespace at end of line
		s/"//g;

        if (/^endfeature/) {
            $disabled = 0;
            next;
        }
        if ($disabled == 1) {
            next;
        }

        if (/^featuredef\s*(.*?)\s*(\d+)/) {
            if(!exists $featuredef->{$1}) {
                $featuredef->{$1} = $2;
            }
        } elsif (/^feature\s*(.*)/) {
            $disabled = !has_feature($featuredef, $1);
        } elsif (/^coreprefix\s*(\w+)/) {
            $coreprefix = $1;
            $coreprefix =~ s/_$//;
            $coreprefix =~ s/TOP_CTRL/SUN_TOP_CTRL/;
        } elsif (/^regtype32\s*(\w+)/) {
			$reg = $1;
			$reg =~ s/Type/$coreprefix/;
            $case = "";
        } elsif (/^case\s*(\w+)/) {
			$case = $1;
        } elsif (/\s*union\s*/) {
            $case = "";
		} elsif (/^field\s*(\w+)/) {
            verify_resource($clk_tree, $resource, $reg, $field, $default, $reg_desc);
            $field = $1;
            $resource = $reg."_".$field;
            if($case ne "") {
                $field = $case."_".$field;
            }
		} elsif (/^default\s*(\d+)/) {
            $default = $1;
		} elsif (/^\/\/PMFunction:\s*(.*?)\s*$/) {
            my @func = rename_function($resource, $1);
            if (@func) {
                $clk_tree->{$resource} = clkResource->new($resource, $reg, $field, $default);
                push(@{$clk_tree->{$resource}{_funcs}}, @func);
                push(@{$clk_tree->{$resource}{_toplvl}}, @func);
                $clk_tree->{$resource}{_pda} = get_pda($1);
            }
		} elsif (/^\/\/PMSource(\d*):\s*(.*?)\s*$/) {
            if (exists $clk_tree->{$resource}) {
                my $indx = $1;
                my $srcs = $2;
                if($indx =~ /\d+/) {
                    $clk_tree->{$resource}{_mux}++;
                }
                $srcs =~ s/\//_/g;
                my @src_arr = split(' ', $srcs);
                push(@{$clk_tree->{$resource}{_srcs}}, @src_arr);
                foreach my $src (@src_arr) {
                    if($src ne "null") {
                        push(@{$parent_map{$src}}, $resource);
                    }
                }
            }
		} elsif (/^\/\/PMDelay:(\d+)\/(\d+)us/) {
            if (exists $clk_tree->{$resource}) {
                $clk_tree->{$resource}{_delay} = $2;
            }
        } elsif (/^\/\/PMPolarity:\s*(.*?)\s*$/) {
            if (exists $clk_tree->{$resource}) {
                $clk_tree->{$resource}{_polarity} = $1;
            }
        } elsif (/^\/\/PMLogic:\s*(.*?)\s*$/) {
            if (exists $clk_tree->{$resource}) {
                $clk_tree->{$resource}{_div} = $1;
            }
        } elsif (/^\/\/PMPState:\s*STB:PMap(\d+):P(\d+):(\d+)/) {
            $clk_tree->{$resource}{_pmap}{$1}{_pstate}{$2} = $3;
            if (($1+1) > $num_profiles) {
                $num_profiles = $1+1;
            }
        } elsif (/^\/\/PMPState:\s*PMap(\d+):P(\d+):(\d+)/) {
            $clk_tree->{$resource}{_pmap}{$1}{_pstate}{$2} = $3;
            if (($1+1) > $num_profiles) {
                $num_profiles = $1+1;
            }
        } elsif (/^\/\/PMPowerProfile:\s*STB\s*:\s*(.*?)\s*:\s*PMap(\d+)/) {
            $profiles[$2] = $1;
        } elsif (/^include\s*(.*?)$/) {
            if (($coreprefix eq "CLKGEN") ||
                ($coreprefix eq "SUN_TOP_CTRL" && $1 eq "sram_pda.rdb")) { # TODO : Pass in a filter list
                my($filename, $directory, $suffix) = fileparse($file);
                push(@files, $directory.$1) if -e $directory.$1;
            }
        } elsif (/^endtype$/) {
            verify_resource($clk_tree, $resource, $reg, $field, $reg_desc);
            undef $resource;
        } elsif (/^Type_(\w+)\s*(\w+)\s*\+(\d+)/) {
            $reg = $coreprefix."_".$2;
            if ($1 =~ /PLL_VCXO(\d)_PLL/) {
                if($1+1 > $num_vcxo_plls) {
                    $num_vcxo_plls = $1+1;
                }
            }
        } elsif (/^desc\s*-(.*?)\s*:\s*(.*?)\s*:\s*\((.*?)\)$/) {
            # Special Processing for these nodes as they may not be tagged correctly in RDB.
            if ($2 =~ /tsio/ || $3 =~ /tsio/) {
                push(@{$reg_desc->{$reg}{_funcs}},"XPT_TSIO");
            } elsif ($2 =~ /itu656/) {
                push(@{$reg_desc->{$reg}{_funcs}},"ITU656") if($disable_vec_656 == 0);
            } elsif ($2 =~ /rm_en_pllvcxo/) {
                push(@{$reg_desc->{$reg}{_funcs}},"ITU656") if($disable_vec_656 == 0);
                for (my $i = 0; $i < $num_aud_plls; $i++) {
                    push(@{$reg_desc->{$reg}{_funcs}},"AUD_PLL".$i);
                }
            } elsif ($2 =~ /pllaudio(\d)/) {
                push(@{$reg_desc->{$reg}{_funcs}},"AUD_PLL".$1) if($1 < $num_aud_plls)
            }
        } elsif (/^(regset|block)\s*(.*?)\s*AUD_FMM_IOP_(OUT_DAC|PLL)_(\d)/) {
            my $cnt = $4+1;
            $num_aud_plls = $cnt if($3 eq "PLL" && $cnt > $num_aud_plls);
            $num_aud_dacs = $cnt if($3 eq "OUT_DAC" && $cnt > $num_aud_dacs && $disable_aud_dac == 0);
        }
	}

    close(INFILE);

    parse_rdb_file($_, $clk_tree, $featuredef, $reg_desc, $coreprefix) foreach (@files);
}

sub skip_node {
    my ($clk_tree, $src) = @_;

    if(($src eq "null") ||
       ($src =~ /PLL_(SYS|SCB).*_PLL/)){
        return 1;
    }

    if(!exists $clk_tree->{$src}) {
        print "$src does not exist in tree. Skip it\n";
        return 1;
    }

    foreach my $func (@{$clk_tree->{$src}{_funcs}}){
        if(!is_nexus_function($func) && !$clk_tree->{$src}{_div}) {
            return 1;
        }
    }

    if($clk_tree->{$src}{_div}) {
        my $match = 0;
        foreach my $func (@{$clk_tree->{$src}{_funcs}}){
            if(is_div_function($func)) {
                $match = 1;
            }
        }
        if($match == 0) {
            return 1;
        }
    }

    return 0;
}

sub generate_top_level_nodes {
	my ($clk_tree, $functions) = @_;

    for (keys %$clk_tree) {
		foreach my $src (@{$clk_tree->{$_}{_srcs}}) {
			if(exists $clk_tree->{$src}) {
				# Eliminate resource's functions from its parent's functions.
				# Do this till we reach null. This will give a list of top level nodes
                for(my $index = @{$clk_tree->{$src}{_toplvl}}; $index > 0; $index--) {
                    my $func = $clk_tree->{$src}{_toplvl}[$index-1];
                    if(grep(/\b$func\b/, @{$clk_tree->{$_}{_funcs}})) {
                        splice(@{$clk_tree->{$src}{_toplvl}}, $index-1, 1)
                    }
                }
			}
		}
	}

	for (sort keys %$clk_tree) {
		if (@{$clk_tree->{$_}{_toplvl}}) {
			foreach my $func (@{$clk_tree->{$_}{_toplvl}}) {
                push(@{$functions->{$func}}, $_);
			}
		}
	}
}

sub print_clock_tree {
	my ($fh, $clk_tree, $sources, $level) = @_;

	foreach my $src (@$sources) {

        if($level > 0) {print $fh "|       ";}
        foreach (2..$level) {print $fh "        ";}
        print $fh "|_______$src\n";

        if(!grep(/null/, @{$clk_tree->{$src}{_srcs}})) {
            print_clock_tree($fh, $clk_tree, \@{$clk_tree->{$src}{_srcs}}, $level+1);
        }
	}
}

sub print_nexus_tree {
	my ($fh, $clk_tree, $sources, $level) = @_;

	foreach my $src (@$sources) {

        my $next_level = $level;
        if(!skip_node($clk_tree, $src)) {

            my $fncs = join(' ', @{$clk_tree->{$src}{_funcs}});

            # Print each sources
            if($level > 0) {print $fh "|       ";}
            foreach (2..$level) {print $fh "        ";}
            print $fh "|_______$src".($clk_tree->{$src}{_delay}?" ($clk_tree->{$src}{_delay})":"")."  [$fncs]\n";

            $next_level++;
        }
        print_nexus_tree($fh, $clk_tree, \@{$clk_tree->{$src}{_srcs}}, $next_level);
	}
}

sub print_tree {
	my ($clk_tree, $functions) = @_;

	open(my $nexus_tree_fh, ">nexus_tree.txt") or die "Can't open output file";
	open(my $clock_tree_fh, ">clock_tree.txt") or die "Can't open output file";


	foreach my $func (sort keys %$functions) {

        if(is_nexus_function($func)) {
            print $nexus_tree_fh "$func\n";
            print_nexus_tree($nexus_tree_fh, $clk_tree, \@{$functions->{$func}}, 0);
            print $nexus_tree_fh "\n\n";
        }

        print $clock_tree_fh "$func\n";
        print_clock_tree($clock_tree_fh, $clk_tree, \@{$functions->{$func}}, 0);
        print $clock_tree_fh "\n\n";
	}

    close($nexus_tree_fh);
	close($clock_tree_fh);
}

sub print_power_p_tree {
	my ($fh, $clk_tree, $sources, $level) = @_;

	foreach my $src (@$sources) {

        if($level > 0) {print $fh "|       ";}
        foreach (2..$level) {print $fh "        ";}
        print $fh "|_______$clk_tree->{$src}{_reg}:$clk_tree->{$src}{_field}";
        if($clk_tree->{$src}{_polarity}) {print $fh    ":$clk_tree->{$src}{_polarity}";}
        if($clk_tree->{$src}{_div}) {print $fh    ":$clk_tree->{$src}{_div}";}
        print $fh "\n";

        if(!grep(/null/, @{$clk_tree->{$src}{_srcs}})) {
            print_power_p_tree($fh, $clk_tree, \@{$clk_tree->{$src}{_srcs}}, $level+1);
        }
	}
}

sub print_power_tree {
	my ($clk_tree, $functions, $chip) = @_;
    my $file = "bpower_".$chip."_tree.txt";

	open(my $fh, ">$file") or die "Can't open output file";

	foreach my $func (sort keys %$functions) {
        if($func !~ /PDA/) {
            print $fh "$func\n";
            print_power_p_tree($fh, $clk_tree, \@{$functions->{$func}}, 0);
            print $fh "\n\n";
        }
	}

	close($fh);
    return 0;
}

sub print_power_p_clk_tree {
	my ($fh, $clk_tree, $src, $level) = @_;

    if($level > 0) {print $fh "|       ";}
    foreach (2..$level) {print $fh "        ";}
    if($level > 0) {print $fh "|_______";}
    print $fh "$clk_tree->{$src}{_reg}:$clk_tree->{$src}{_field}";
    if($clk_tree->{$src}{_polarity}) {print $fh    ":$clk_tree->{$src}{_polarity}";}
    if($clk_tree->{$src}{_div}) {print $fh    ":$clk_tree->{$src}{_div}";}
    print $fh "\n";

    if (!exists $parent_map{$src}) {
        return;
    }

	foreach (@{$parent_map{$src}}) {
        print_power_p_clk_tree($fh, $clk_tree, $_, $level+1);
    }
}

sub print_power_clk_tree {
    my ($clk_tree, $chip) = @_;
    my @values = map {@$_} values(%parent_map);
    my @top_plls;
    my $file = "bpower_".$chip."_clk_tree.txt";

	open(my $fh, ">$file") or die "Can't open output file";
    foreach my $key (keys %parent_map) {
        if (!grep(/$key/, @values)) {
            push(@top_plls, $key);
        }
    }

    foreach (sort @top_plls) {
        print_power_p_clk_tree($fh, $clk_tree, $_, 0);
        print $fh "\n\n";
    }

	close($fh);
    return 0;
}

sub rename_node {
    my $function = shift;

    $function =~ s/^HVD(\d*)$/AVD$1_CLK/g;
    $function =~ s/^HVD[a-zA-Z](\d*)$/AVD$1_CLK/g;
    $function =~ s/^HVD(\d*)_PDA(R|W)$/AVD$1_PWR/g;
    $function =~ s/^HVD[a-zA-Z](\d*)_PDA(R|W)$/AVD$1_PWR/g;
    $function =~ s/^HVD(\d*)_CPU$/AVD$1_CPU/g;
    $function =~ s/^HVD[a-zA-Z](\d*)_CPU$/AVD$1_CPU/g;
    $function =~ s/^VICE2(\d*)$/VICE$1_CLK/g;
    $function =~ s/^VICE2(\d*)_PDA(R|W)$/VICE$1_PWR/g;
    $function =~ s/^AIO(\d*)$/AIO$1_CLK/g;
    $function =~ s/^RAAGA(\d*)$/RAAGA$1_CLK/g;
    $function =~ s/^RAAGA(\d*)_CPU$/RAAGA$1_DSP/g;
    $function =~ s/^VDAC(\d*)$/VDC_DAC$1/g;
    $function =~ s/^VEC/VDC_VEC/g;
    $function =~ s/^ITU656(\d*)/VDC_656_OUT$1/g;
    $function =~ s/^XPT$/XPT_XMEMIF/g;
    $function =~ s/^DVPHT(\d*)/HDMI_TX$1/g;
    $function =~ s/^DVPHR(\d*)/HDMI_RX$1/g;
    $function =~ s/^HDMI_TX(\d*)$/HDMI_TX$1_CLK/g;
    $function =~ s/^HDMI_RX(\d*)$/HDMI_RX$1_CLK/g;
    $function =~ s/^V3D/GRAPHICS3D/g;
    $function =~ s/^SC(\d*)$/SMARTCARD$1/g;
    $function =~ s/^PAD$//g;
    $function =~ s/PDA(R|W)/SRAM/g;
    $function =~ s/^SECBUS$/SECURE_ACCESS/g;

    return $function;
}

sub generate_nodes {
    my ($clk_tree, $sources, $nodes, $hw_desc, $node, $hw_node, $recurse) = @_;
    my ($combine, $create_hw);

    $node = rename_node($node);
    if($node =~ /^\s*$/) {
        return;
    }

    if($hw_node eq "") {
        $create_hw = 1;
    } else {
        $create_hw = 0;
    }

    foreach my $src (@$sources) {
        $combine = 0;

        if(!skip_node($clk_tree, $src)) {
            my $reg = $clk_tree->{$src}{_reg};

            if($clk_tree->{$src}{_pda} ne "") {
                $reg = $clk_tree->{$src}{_reg}."_".$clk_tree->{$src}{_pda};
            }

            if($clk_tree->{$src}{_div}) {

                if($create_hw) {
                    #$hw_node = $node;
                    $hw_node = $src;
                    $hw_node =~ s/^HW_//;
                    $hw_node =~ s/DIS_//;
                    $hw_node =~ s/CH_\d_//;
                    $hw_node =~ s/CH_//;
                    $hw_node =~ s/POST_//;
                    $hw_node =~ s/(M|N|P)DIV_//;
                    $hw_node =~ s/DIV_//;
                    $hw_node =~ s/PLL_//g;
                    $hw_node =~ s/HOLD_//;
                    $hw_node =~ s/CLKGEN_//;
                    $hw_node =~ s/CHANNEL_//g;
                    $hw_node = "DV_".$hw_node."_div";
                }

                if(!exists $nodes->{$node}{_list}{$hw_node}) {
                    push(@{$nodes->{$node}{_order}}, $hw_node);
                }
                $nodes->{$node}{_list}{$hw_node}++;
                if(!exists $hw_desc->{$hw_node}{$reg}{_field}{$clk_tree->{$src}{_div}}{$clk_tree->{$src}{_field}}) {
                    $hw_desc->{$hw_node}{$reg}{_field}{$clk_tree->{$src}{_div}}{$clk_tree->{$src}{_field}} = $clk_tree->{$src}{_default};
                }
                if(!exists $hw_desc->{$hw_node}{$reg}{_field}{$clk_tree->{$src}{_div}}{_pmap}) {
                    if(exists $clk_tree->{$src}{_pmap}) {
                        $hw_desc->{$hw_node}{$reg}{_field}{$clk_tree->{$src}{_div}}{_pmap} = $clk_tree->{$src}{_pmap}
                    }
                }

                if(!exists $hw_desc->{$hw_node}{$reg}{_field}{$clk_tree->{$src}{_div}}{_shared}) {
                    $hw_desc->{$hw_node}{$reg}{_field}{$clk_tree->{$src}{_div}}{_shared} = 0;
                    foreach my $func (@{$clk_tree->{$src}{_funcs}}){
                        if(!is_nexus_function($func)) {
                            $hw_desc->{$hw_node}{$reg}{_field}{$clk_tree->{$src}{_div}}{_shared} = 1;
                            last;
                        }
                    }
                }

                # Combine divisor nodes
                foreach (@{$clk_tree->{$src}{_srcs}}) {
                    if($clk_tree->{$_}{_div}) {
                        $combine = 1;
                        last;
                    }
                }
            } elsif($clk_tree->{$src}{_mux}) {
                $hw_node = $src;
                $hw_node =~ s/CLKGEN_//;
                $hw_node =~ s/STB_//;
                $hw_node =~ s/INST_//;
                $hw_node =~ s/CLOCK_//;
                $hw_node =~ s/TOP_//;
                $hw_node = "MX_".$hw_node;

                if(!exists $nodes->{$node}{_list}{$hw_node}) {
                    push(@{$nodes->{$node}{_order}}, $hw_node);
                }
                $nodes->{$node}{_list}{$hw_node}++;
                if(!exists $hw_desc->{$hw_node}{$reg}{_field}{_mux}{$clk_tree->{$src}{_field}}) {
                    $hw_desc->{$hw_node}{$reg}{_field}{_mux}{$clk_tree->{$src}{_field}} = $clk_tree->{$src}{_default};
                }
                if(!exists $hw_desc->{$hw_node}{$reg}{_field}{_mux}{_pmap}) {
                    if(exists $clk_tree->{$src}{_pmap}) {
                        $hw_desc->{$hw_node}{$reg}{_field}{_mux}{_pmap} = $clk_tree->{$src}{_pmap}
                    }
                }
            } else {
                if($create_hw) {
                    $hw_node = join(' ', @{$clk_tree->{$src}{_funcs}});
                    if(((!grep(/null/, @{$clk_tree->{$src}{_srcs}})) &&
                        (!grep(/PLL_(SYS|SCB).*_PLL/, @{$clk_tree->{$src}{_srcs}}))
                       ) ||
                       ($recurse > 0)
                      ) {
                            $hw_node = $clk_tree->{$src}{_name};
                    }
                    $hw_node =~ s/\s+$//g;
                    $hw_node =~ s/\s+/_/g;
                    $hw_node = "HW_".$hw_node;
                    $hw_node = rename_node($hw_node);
                }

                if(!exists $nodes->{$node}{_list}{$hw_node}) {
                    push(@{$nodes->{$node}{_order}}, $hw_node);
                }
                $nodes->{$node}{_list}{$hw_node}++;
                $hw_desc->{$hw_node}{$reg}{_field}{$clk_tree->{$src}{_polarity}}{$clk_tree->{$src}{_field}} = $clk_tree->{$src}{_default};

                # Combine consecutive sources with same register
                if(scalar(@{$clk_tree->{$src}{_srcs}}) == 1) {
                    foreach (@{$clk_tree->{$src}{_srcs}}) {
                        if($_ ne "null" && !$clk_tree->{$_}{_div} && !$clk_tree->{$_}{_mux}) {
                            if($reg eq $clk_tree->{$_}{_reg}) {
                                $combine = 1;
                                last;
                            }
                        }
                    }
                }
            }
            $hw_desc->{$hw_node}{$reg}{_delay} = $clk_tree->{$src}{_delay};
            $hw_desc->{$hw_node}{$reg}{_pda} = $clk_tree->{$src}{_pda};
        } else {
            $hw_node = $node;
        }
        if(!$combine) {
            generate_nodes($clk_tree, \@{$clk_tree->{$src}{_srcs}}, $nodes, $hw_desc, $hw_node, "", 1);
        } else {
            generate_nodes($clk_tree, \@{$clk_tree->{$src}{_srcs}}, $nodes, $hw_desc, $node, $hw_node, 1);
        }
	}
}

sub trim_dv_nodes {
    my ($nodes, $hw_desc) = @_;

    foreach my $node (keys %$nodes) {
        foreach my $hw_node (keys %{$nodes->{$node}{_list}}) {
            if($hw_node =~ /^DV_/) {
                my $pmap_found = 0;
                foreach my $reg (sort keys %{$hw_desc->{$hw_node}}) {
                    foreach my $div (sort keys %{$hw_desc->{$hw_node}{$reg}{_field}}) {
                        if(exists $hw_desc->{$hw_node}{$reg}{_field}{$div}{_pmap}) {
                            $pmap_found = 1;
                            last;
                        }
                    }
                    if($pmap_found) {last;}
                }
                if(!$pmap_found) {
                    #print "Delete $hw_node\n";
                    delete $hw_desc->{$hw_node};
                    if(exists $nodes->{$hw_node}) {
                        #print "Assign $hw_node $node\n";
                        delete $nodes->{$node}{_list};
                        delete $nodes->{$node}{_order};
                        foreach my $list (keys %{$nodes->{$hw_node}{_list}}) {
                            $nodes->{$node}{_list}{$list} = $nodes->{$hw_node}{_list}{$list}
                        }
                        foreach my $order (@{$nodes->{$hw_node}{_order}}) {
                            push(@{$nodes->{$node}{_order}}, $order);
                        }

                        #$nodes->{$node}{_list} = $nodes->{$hw_node}{_list};
                        #$nodes->{$node}{_order} = $nodes->{$hw_node}{_order};
                        #print "Delete $hw_node\n";
                        delete $nodes->{$hw_node}{_list};
                        delete $nodes->{$hw_node}{_order};
                        delete $nodes->{$hw_node};
                    } else {
                        #print "Delete $node\n";
                        delete $nodes->{$node};
                    }
                }
            }
        }
    }
}

sub generate_nexus_nodes {
    my ($clk_tree, $functions, $nodes, $hw_desc) = @_;

    foreach my $func (sort keys %$functions) {
        if(is_nexus_function($func)) {
            generate_nodes($clk_tree, \@{$functions->{$func}}, $nodes, $hw_desc, $func, "", 0);
        }
    }
    #trim_dv_nodes($nodes, $hw_desc);
}

sub has_feature {
    my ($featuredef, $feature) = @_;
    if(exists $featuredef->{$feature} && $featuredef->{$feature} != 0) {
        return 1;
    } else {
        return 0;
    }
}

sub generate_user_defined_nodes {
    my ($nodes, $featuredef) = @_;

    foreach my $node (keys %$nodes) {
        if($node =~ /(HDMI_)(TX|RX)(\d*)_CLK/) {
            my $parent_node = $1.$2.$3."_PHY";
            my $child_node = "HW_".$1.$2.$3."_PHY";
            $nodes->{$parent_node}{_list}{$child_node}++;
            foreach my $hw_node (keys %{$nodes->{$node}{_list}}) { $nodes->{$child_node}{_list}{$hw_node} = $nodes->{$node}{_list}{$hw_node} }
            @{$nodes->{$child_node}{_order}} = @{$nodes->{$node}{_order}};

            $nodes->{"BINT_OPEN"}{_list}{$1.$2.$3."_CLK"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1.$2.$3."_CLK"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1.$2.$3."_PHY"}++;

            if ($2 eq "TX") {
                $nodes->{"VDC_".$1.$2."_PHY".$3}{_list}{$1.$2.$3."_PHY"}++;
                $nodes->{"MAGNUM_CONTROLLED"}{_list}{"VDC_".$1.$2."_PHY".$3}++;
            } elsif ($2 eq "RX") {
                $nodes->{"VDC_".$1.$2."_CLK".$3}{_list}{$1.$2.$3."_CLK"}++;
                $nodes->{"MAGNUM_CONTROLLED"}{_list}{"VDC_".$1.$2."_CLK".$3}++;
            }
        } elsif($node =~ /^(RFM)$/) {
            my $parent_node = $1."_PHY";
            my $child_node = "HW_".$1."_PHY";
            $nodes->{$parent_node}{_list}{$child_node}++;
            foreach my $hw_node (keys %{$nodes->{$node}{_list}}) { $nodes->{$child_node}{_list}{$hw_node} = $nodes->{$node}{_list}{$hw_node} }
            @{$nodes->{$child_node}{_order}} = @{$nodes->{$node}{_order}};

            $nodes->{"BINT_OPEN"}{_list}{$1}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1."_PHY"}++;
        } elsif($node =~ /AIO_CLK/) {
            $nodes->{"AUD_AIO"}{_list}{"$node"}++;
            $nodes->{"BINT_OPEN"}{_list}{"AUD_AIO"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{"AUD_AIO"}++;

            if($num_aud_dacs > 0) {
                $nodes->{"AUD_DAC"}{_list}{"HW_AUD_DAC"}++;
                foreach my $hw_node (keys %{$nodes->{$node}{_list}}) {
                    $nodes->{"HW_AUD_DAC"}{_list}{$hw_node} = $nodes->{$node}{_list}{$hw_node}
                }
                @{$nodes->{"HW_AUD_DAC"}{_order}} = @{$nodes->{$node}{_order}};
                $nodes->{"MAGNUM_CONTROLLED"}{_list}{"AUD_DAC"}++;
            }
        } elsif($node =~ /AIO_SRAM/) {
            $nodes->{"AUD_AIO"}{_list}{"$node"}++;
        } elsif($node =~ /(AUD_PLL)(\d*)/) {
            my $parent_node = $1.$2;
            my $child_node = "HW_".$1.$2;
            $nodes->{$parent_node}{_list}{$child_node}++;
            push(@{$nodes->{$parent_node}{_order}}, $child_node);
            foreach my $hw_node (keys %{$nodes->{"AIO_CLK"}{_list}}) {
                $nodes->{$child_node}{_list}{$hw_node} = $nodes->{"AIO_CLK"}{_list}{$hw_node};
            }
            @{$nodes->{$child_node}{_order}} = @{$nodes->{"AIO_CLK"}{_order}};
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$parent_node}++;
        } elsif($node =~ /^(BVN|VDC_VEC|VIP)_SRAM/) {
            foreach my $hw_node (keys %{$nodes->{$node}{_list}}) {
                $nodes->{$1}{_list}{$hw_node} = $nodes->{$node}{_list}{$hw_node};
                push(@{$nodes->{$1}{_order}}, $hw_node);
                delete($nodes->{$node});
            }
        } elsif($node =~ /^(AVD|VICE)(\d*)(.*?)$/) {
            if($3 eq "_CPU") {
                foreach my $cpu_node (keys %{$nodes->{$1.$2.$3}{_list}}) {
                    foreach my $clk_node (keys %{$nodes->{$1.$2."_CLK"}{_list}}) {
                        $nodes->{$cpu_node}{_list}{$clk_node}++;
                    }
                    push(@{$nodes->{$cpu_node}{_order}}, @{$nodes->{$1.$2."_CLK"}{_order}});
                }

                delete($nodes->{$1.$2."_CLK"}{_list});
                undef(@{$nodes->{$1.$2."_CLK"}{_order}});

                foreach my $cpu_node (keys %{$nodes->{$1.$2.$3}{_list}}) {
                    $nodes->{$1.$2."_CLK"}{_list}{$cpu_node}++;
                    push(@{$nodes->{$1.$2."_CLK"}{_order}}, $cpu_node);
                }
                delete($nodes->{$1.$2.$3});
            } else {
                $nodes->{$1.$2}{_list}{$node}++;
                $nodes->{$1}{_list}{$1.$2}++;

                $nodes->{"BINT_OPEN"}{_list}{$1}++;
                $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1}++;
            }

            if($node =~ /^(VICE)(\d*)(_CLK)$/) {
                $nodes->{"VDC_STG".($2*2)}{_list}{$node}++;
                $nodes->{"VDC_STG".($2*2+1)}{_list}{$node}++;
                $nodes->{"MAGNUM_CONTROLLED"}{_list}{"VDC_STG".($2*2)}++;
                $nodes->{"MAGNUM_CONTROLLED"}{_list}{"VDC_STG".($2*2+1)}++;
            }
        } elsif($node =~ /^(RAAGA)(\d*)(.*?)$/) {
            if($3 eq "_DSP") {
                if (exists $nodes->{$1.$2."_CLK"}) {
                    foreach my $cpu_node (keys %{$nodes->{$1.$2.$3}{_list}}) {
                        foreach my $clk_node (keys %{$nodes->{$1.$2."_CLK"}{_list}}) {
                            $nodes->{$cpu_node}{_list}{$clk_node}++;
                        }
                        push(@{$nodes->{$cpu_node}{_order}}, @{$nodes->{$1.$2."_CLK"}{_order}});
                    }
                }
            }
            $nodes->{$1}{_list}{$node}++;

            $nodes->{"BINT_OPEN"}{_list}{$1}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1}++;
        } elsif($node =~ /^(SID|GRAPHICS3D)(_CPU)$/) {
            foreach my $cpu_node (keys %{$nodes->{$1.$2}{_list}}) {
                foreach my $clk_node (keys %{$nodes->{$1}{_list}}) {
                    $nodes->{$cpu_node}{_list}{$clk_node}++;
                }
                push(@{$nodes->{$cpu_node}{_order}}, @{$nodes->{$1}{_order}});
            }

            delete($nodes->{$1}{_list});
            undef(@{$nodes->{$1}{_order}});

            foreach my $cpu_node (keys %{$nodes->{$1.$2}{_list}}) {
                $nodes->{$1}{_list}{$cpu_node}++;
                push(@{$nodes->{$1}{_order}}, $cpu_node);
            }
            delete($nodes->{$1.$2});
        } elsif($node =~ /^(M2MC)(\d*)(.*?)$/) {
            $nodes->{$1.$3}{_list}{$node}++;

            $nodes->{"BINT_OPEN"}{_list}{$1}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1}++;
        } elsif($node =~ /^(VIP)(\d*)$/) {
            my $idx = $2 ne ""?$2:0;
            $nodes->{"VDC_STG".($idx)}{_list}{$node}++;

            $nodes->{"BINT_OPEN"}{_list}{$node}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$node}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{"VDC_STG".($idx)}++;
        } elsif($node =~ /^(XPT)(_XMEMIF)$/) {
            $nodes->{$1}{_list}{$1."_PARSER"}++;
            $nodes->{$1}{_list}{$1."_PLAYBACK"}++;
            $nodes->{$1}{_list}{$1."_RAVE"}++;
            $nodes->{$1}{_list}{$1."_PACKETSUB"}++;
            $nodes->{$1."_PARSER"}{_list}{$node}++;
            $nodes->{$1."_PLAYBACK"}{_list}{$node}++;
            $nodes->{$1."_RAVE"}{_list}{$node}++;
            $nodes->{$1."_PACKETSUB"}{_list}{$node}++;
            $nodes->{"DMA"}{_list}{$node}++;
            $nodes->{"HSM"}{_list}{$node}++;

            $nodes->{"BINT_OPEN"}{_list}{$1}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1}++;
        } elsif($node =~ /^(XPT)(_REMUX|_WAKEUP)$/) {
            $nodes->{$1}{_list}{$1.$2}++;
        } elsif($node =~ /^(BVN|VDC)(.*?)(DAC|VEC|656)?(.*?)/) {
            $nodes->{"VDC"}{_list}{$node}++;

            $nodes->{"BINT_OPEN"}{_list}{"VDC"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{"VDC"}++;
        } elsif($node =~ /^MX.*V3D/) {
            $nodes->{"GRAPHICS3D_PLL_CH"}{_list}{$node}++;
        } else {
            if($node !~ /^HW/ && $node !~ /^SECURE_ACCESS$/ && $node !~ /^DV_/ && $node !~ /^MX_/) {
                $nodes->{"MAGNUM_CONTROLLED"}{_list}{$node}++;
                if($node !~ /SRAM/ && $node !~ /PWR/ && $node !~ /PHY/ && $node !~ /DAC/) {
                    $nodes->{"BINT_OPEN"}{_list}{$node}++;
                }
            }
        }
    }

    #check for nodes with no order
    foreach my $node (keys %$nodes) {
        if(!exists $nodes->{$node}{_order}) {
            foreach my $child_node ( sort keys %{$nodes->{$node}{_list}}) {
                push (@{$nodes->{$node}{_order}}, $child_node);
            }
        } else {
            # check for duplicates and remove
            my %tmp;
            my $cnt=0;
            undef %tmp;
            foreach my $child_node (@{$nodes->{$node}{_order}}) {
                if (exists $tmp{$child_node}) {
                    splice(@{$nodes->{$node}{_order}}, $cnt, 1);
                }
                $tmp{$child_node}++;
            }
            #@{$nodes->{$node}{_order}} = keys %tmp;
        }
    }
}

sub generate_brcm_copyright_header
{
    my @lines;

    push @lines, " /******************************************************************************\n";
    push @lines, " *  Copyright (C) 2017 Broadcom.  The term".' "Broadcom" '."refers to Broadcom Limited and/or its subsidiaries.\n";
    push @lines, " *\n";
    push @lines, " *  This program is the proprietary software of Broadcom and/or its licensors,\n";
    push @lines, " *  and may only be used, duplicated, modified or distributed pursuant to the terms and\n";
    push @lines, " *  conditions of a separate, written license agreement executed between you and Broadcom\n";
    push @lines, " *  (an ".'"Authorized License"'.").  Except as set forth in an Authorized License, Broadcom grants\n";
    push @lines, " *  no license (express or implied), right to use, or waiver of any kind with respect to the\n";
    push @lines, " *  Software, and Broadcom expressly reserves all rights in and to the Software and all\n";
    push @lines, " *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU\n";
    push @lines, " *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY\n";
    push @lines, " *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.\n";
    push @lines, " *\n";
    push @lines, " *  Except as expressly set forth in the Authorized License,\n";
    push @lines, " *\n";
    push @lines, " *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade\n";
    push @lines, " *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,\n";
    push @lines, " *  and to use this information only in connection with your use of Broadcom integrated circuit products.\n";
    push @lines, " *\n";
    push @lines, " *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED".' "AS IS"'."\n";
    push @lines, " *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR\n";
    push @lines, " *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO\n";
    push @lines, " *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES\n";
    push @lines, " *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,\n";
    push @lines, " *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION\n";
    push @lines, " *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF\n";
    push @lines, " *  USE OR PERFORMANCE OF THE SOFTWARE.\n";
    push @lines, " *\n";
    push @lines, " *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS\n";
    push @lines, " *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR\n";
    push @lines, " *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR\n";
    push @lines, " *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF\n";
    push @lines, " *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT\n";
    push @lines, " *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S.".' $1'.", WHICHEVER IS GREATER. THESE\n";
    push @lines, " *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF\n";
    push @lines, " *  ANY LIMITED REMEDY.\n";
    push @lines, "\n";
    push @lines, " ******************************************************************************/\n";
    push @lines, "\n";
    return @lines;
}

sub generate_bchp_resources_txt_file {
    my ($nodes, $file) = @_;
    my @resource_list;

    open(my $fh, ">$file") or die "Can't open output file $file";

    foreach my $node (sort keys %$nodes) {
        undef @resource_list;
        foreach my $hw_node (@{$nodes->{$node}{_order}}) {
            push(@resource_list, $hw_node);
        }

        print $fh "$node -> {".join(", ", @resource_list)."}\n";
    }

    close($fh);

    print "Output: $file\n";
}

sub generate_bchp_impl_string {
    my ($hw_desc, $hw_node, $reg) = @_;
    my @lines;
    my $pdar = ($hw_desc->{$hw_node}{$reg}{_pda} eq "READ");
    my $vcxo_reset = ($reg =~ /VCXO\d*_PLL_RESET/)?1:0;
    my $div = ($hw_node =~ /^DV_.*_div/)?1:0;
    my $mux = ($hw_node =~ /^MX_/)?1:0;
    my $register = $reg;

    #Strip away PDA tag from register. This was done to seperate
    #write and read when both the bits are in the same register.
    #TODO : Find a better way to  seperate the wirte and read
    if ($hw_desc->{$hw_node}{$reg}{_pda} ne "") {
        $register =~ s/_$hw_desc->{$hw_node}{$reg}{_pda}//;
    }

    if($pdar) {
        push @lines, "    {\n";
        push @lines, "        uint32_t val=0, cnt=50;\n";
        push @lines, "        while(cnt--) {\n";
        push @lines, "            BKNI_Delay(10);\n";
        push @lines, "        ";
    }
    if($vcxo_reset) {
        push @lines, "    BSTD_UNUSED(reg);\n\n";
    } else {
        push @lines, "    reg = BREG_Read32(handle->regHandle, BCHP_".$register.");\n";
    }

    if ($div) {
        my $var;
        my $write_count = 0;
        my (@lines0, @lines1, @lines2, @lines3);
        push @lines, "    if(!set) {\n";
        push @lines1, "        BREG_Write32(handle->regHandle, BCHP_".$register.", reg);\n";
        push @lines0, "    } else {\n";
        foreach my $divisor (keys %{$hw_desc->{$hw_node}{$reg}{_field}}) {
            foreach my $field (keys %{$hw_desc->{$hw_node}{$reg}{_field}{$divisor}}) {
                if($field eq "_pmap") {next;}
                if($field eq "_shared") {next;}
                if ($divisor eq "POSTDIV") {
                    $var = "*postdiv";
                } else {
                    if ($divisor eq "MULT") {
                        $var = "*mult";
                    } elsif ($divisor eq "PREDIV") {
                        $var = "*prediv";
                    } else {
                        print "Unknown Div field\n";
                        next;
                    }
                }
                if(!$hw_desc->{$hw_node}{$reg}{_field}{$divisor}{_shared} && $divisor eq "POSTDIV") {
                    push @lines2, "        BCHP_SET_FIELD_DATA(reg, $register, $field, ".$var.");\n";
                    $write_count++;
                }
                push @lines3, "        ".$var." = BCHP_GET_FIELD_DATA(reg, $register, $field);\n";
            }
        }
        push @lines, @lines3;
        if($write_count) {
            push @lines, @lines0;
            push @lines, @lines2;
            push @lines, @lines1;
        }
        push @lines, "    }\n";
    } elsif ($mux) {
        foreach my $field (keys %{$hw_desc->{$hw_node}{$reg}{_field}{_mux}}) {
            if($field eq "_pmap") {next;}
            push @lines, "    if(set) {\n";
            push @lines, "        BCHP_SET_FIELD_DATA(reg, $register, $field, *mux);\n";
            push @lines, "        BREG_Write32(handle->regHandle, BCHP_".$register.", reg);\n";
            push @lines, "    } else {\n";
            push @lines, "        *mux = BCHP_GET_FIELD_DATA(reg, $register, $field);\n";
            push @lines, "    }\n";
        }
    } else {
        foreach my $polarity (keys %{$hw_desc->{$hw_node}{$reg}{_field}}) {
            my $count = keys %{$hw_desc->{$hw_node}{$reg}{_field}{$polarity}};
            my $idx = 0;
            if($pdar) {push @lines, "        ";}

            push @lines, "    mask = ";
            if($count > 1) {push @lines, "(";}
            foreach my $field (keys %{$hw_desc->{$hw_node}{$reg}{_field}{$polarity}}) {
                if($idx > 0) {push @lines, "            ";}
                push @lines, "BCHP_".$register."_".$field."_MASK";
                $idx++;
                if($idx < $count) {push @lines, " |\n";}
            }
            if($count > 1) {push @lines, ")";}
            push @lines, ";\n";

            if($pdar) {
                push @lines, "            reg &= mask;\n";
                if($polarity eq "HighIsOn") {
                    push @lines, "            val |= activate?mask:0;\n";
                } elsif($polarity eq "HighIsOff") {
                    push @lines, "            val |= activate?0:mask;\n";
                }
            } elsif($vcxo_reset) {
                push @lines, "    BREG_AtomicUpdate32(handle->regHandle, BCHP_".$register.", mask, ";
                if($polarity eq "HighIsOn") {
                    push @lines, "activate?mask:0);\n";
                } elsif($polarity eq "HighIsOff") {
                    push @lines, "activate?0:mask);\n";
                }
            } else {
                push @lines, "    reg &= ~mask;\n";
                if($polarity eq "HighIsOn") {
                    push @lines, "    reg |= activate?mask:0;\n";
                } elsif($polarity eq "HighIsOff") {
                    push @lines, "    reg |= activate?0:mask;\n";
                }
            }
        }
    }
    if($pdar) {
        push @lines, "            if (val == reg)\n";
        push @lines, "                break;\n";
        push @lines, "        }\n";
        push @lines, "        if(!cnt)\n";
        push @lines, "            BDBG_ERR((\"".$hw_node." Timeout\"));\n";
        push @lines, "    }\n";
    } elsif(!$vcxo_reset && !$div && !$mux) {
        push @lines, "    BREG_Write32(handle->regHandle, BCHP_".$register.", reg);\n";
    }
    if($hw_desc->{$hw_node}{$reg}{_delay} > 0) {
        push @lines, "    if(activate)\n";
        push @lines, "        BKNI_Delay(".$hw_desc->{$hw_node}{$reg}{_delay}.");\n";
    }

    return @lines;
}

sub generate_bchp_impl_c_file {
    my ($hw_desc, $file) = @_;

    open(my $fh, ">$file") or die "Can't open output file $file";

    my @lines = generate_brcm_copyright_header();
    print $fh @lines;

    print $fh "/***************************************************************************\n";
    print $fh "*\n";
    print $fh "* This file is auto-generated\n";
    print $fh "*\n";
    print $fh "* This file maps the power resource control to register writes.\n";
    print $fh "*\n";
    print $fh "***************************************************************************/\n";
    print $fh "\n";
    print $fh "#include \"bchp.h\"\n";
    print $fh "#include \"bchp_priv.h\"\n";
    print $fh "#include \"bdbg.h\"\n";
    print $fh "#include \"bkni.h\"\n";
    print $fh "#include \"bchp_clkgen.h\"\n";
    #print $fh "#include \"bchp_avs_top_ctrl.h\"\n";
    print $fh "#include \"bchp_sun_top_ctrl.h\"\n";
    print $fh "\n";
    print $fh "BDBG_MODULE(BCHP_PWR_IMPL);\n";

    foreach my $hw_node (sort keys %$hw_desc) {
        my ($pdar_done,$pdaw_done) = (0,0);
        my $tmp_reg;
        print $fh "\n";
        if($hw_node =~ /^DV_.*_div/) {
            print $fh "static void BCHP_PWR_P_".$hw_node."_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)\n";
            print $fh "{\n";
            print $fh "    uint32_t reg;\n";
            print $fh "\n";
            print $fh "    BDBG_MSG((\"".$hw_node.": %s\", set?\"write\":\"read\"));\n";
            print $fh "\n";
        } elsif ($hw_node =~ /^MX_/) {
            print $fh "static void BCHP_PWR_P_".$hw_node."_Control(BCHP_Handle handle, unsigned *mux, bool set)\n";
            print $fh "{\n";
            print $fh "    uint32_t reg;\n";
            print $fh "\n";
            print $fh "    BDBG_MSG((\"".$hw_node.": %s\", set?\"write\":\"read\"));\n";
            print $fh "\n";
        } elsif ($hw_node =~ /^HW_/) {
            print $fh "static void BCHP_PWR_P_".$hw_node."_Control(BCHP_Handle handle, bool activate)\n";
            print $fh "{\n";
            print $fh "    uint32_t mask, reg;\n";
            print $fh "\n";
            print $fh "    BDBG_MSG((\"".$hw_node.": %s\", activate?\"on\":\"off\"));\n";
            print $fh "\n";
        } else {
            print "Unknown node $hw_node\n";
            next;
        }
        foreach my $reg (sort keys %{$hw_desc->{$hw_node}}) {
            if($hw_desc->{$hw_node}{$reg}{_pda} eq "READ") {
                if(!$pdaw_done) {
                    $tmp_reg = $reg;
                    next;
                } else {
                    $pdar_done=1;
                }
            }
            if($hw_desc->{$hw_node}{$reg}{_pda} eq "WRITE") {$pdaw_done=1;}

            @lines = generate_bchp_impl_string($hw_desc, $hw_node, $reg);
            print $fh @lines;

            if($pdaw_done && !$pdar_done) {
                $pdar_done=1;
                if($tmp_reg) {
                    @lines = generate_bchp_impl_string($hw_desc, $hw_node, $tmp_reg);
                    print $fh @lines;
                } else {
                    print "PDAR not found : $hw_node\n";
                }
                $pdaw_done=0;
                $pdar_done=0;
            }
        }
        printf $fh "}\n";
    }

    close($fh);

    print "Output: $file\n";
}

sub generate_bchp_resources_priv_h_file {
    my ($nonleafs, $nonleafshw, $leafs, $nonleafsdv, $leafsdv, $nonleafsmx, $leafsmx, $file) = @_;
    my @nonleafshw = keys %$nonleafshw;
    my @leafs = keys %$leafs;
    my @hw_all = (@nonleafshw, @leafs);
    my @nonleafsdv = keys %$nonleafsdv;
    my @leafsdv = keys %$leafsdv;
    my @DV_all = (@nonleafsdv, @leafsdv);
    my @nonleafsmx = keys %$nonleafsmx;
    my @leafsmx = keys %$leafsmx;
    my @MX_all = (@nonleafsmx, @leafsmx);

    open(my $fh, ">$file") or die "Can't open output file $file";

    my @lines = generate_brcm_copyright_header();
    print $fh @lines;

    print $fh "/***************************************************************************\n";
    print $fh "*\n";
    print $fh "* This file is auto-generated\n";
    print $fh "*\n";
    print $fh "* This file contains a list of private power resource IDs that\n";
    print $fh "* represent HW clocks, and function prototypes for controlling\n";
    print $fh "* them.\n";
    print $fh "*\n";
    print $fh "***************************************************************************/\n";
    print $fh "\n";
    print $fh "#ifndef BCHP_PWR_RESOURCES_PRIV_H__\n";
    print $fh "#define BCHP_PWR_RESOURCES_PRIV_H__\n";
    print $fh "\n";
    print $fh "#include \"bchp_pwr.h\"\n";
    print $fh "\n";
    print $fh "/* Private power resource IDs */\n";

    # get the length of the longest node string
    my $maxlen = 0;
    foreach (@hw_all, @MX_all, @DV_all) {
        if (length > $maxlen) {
            $maxlen = length;
        }
    }
    $maxlen += length("BCHP_PWR_")+3;

    my $idx = 1;
    foreach (sort @hw_all) {
        printf $fh ("#define %-${maxlen}s 0x%08x\n", "BCHP_PWR_$_", 0xff000000+$idx++);
    }
    foreach (sort @MX_all) {
        printf $fh ("#define %-${maxlen}s 0x%08x\n", "BCHP_PWR_$_", 0xff000000+$idx++);
    }
    foreach (sort @DV_all) {
        printf $fh ("#define %-${maxlen}s 0x%08x\n", "BCHP_PWR_$_", 0xff000000+$idx++);
    }

    print $fh "\n";
    print $fh "/* This is the link between the public and private interface */\n";
    print $fh "void BCHP_PWR_P_HW_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, bool activate);\n";
    print $fh "void BCHP_PWR_P_HW_ControlId(BCHP_Handle handle, unsigned id, bool activate);\n";
    print $fh "void BCHP_PWR_P_MUX_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, unsigned *mux, bool set);\n";
    print $fh "void BCHP_PWR_P_DIV_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set);\n";
    print $fh "\n";

    print $fh "#define BCHP_PWR_P_NUM_NONLEAFS     ", scalar(keys %$nonleafs), "\n";
    print $fh "#define BCHP_PWR_P_NUM_NONLEAFSHW   ", scalar(keys %$nonleafshw), "\n";
    print $fh "#define BCHP_PWR_P_NUM_LEAFS        ", scalar(keys %$leafs), "\n";
    print $fh "#define BCHP_PWR_P_NUM_MUXES        ", scalar(@MX_all), "\n";
    print $fh "#define BCHP_PWR_P_NUM_DIVS         ", scalar(@DV_all), "\n";
    print $fh "#define BCHP_PWR_P_NUM_ALLNODES     ", scalar(keys %$nonleafs)+scalar(keys %$leafs)+scalar(keys %$nonleafshw)+scalar(@MX_all)+scalar(@DV_all), "\n";
    print $fh "#define BCHP_PWR_NUM_P_PMAPSETTINGS ", $num_pmap_settings, "\n";
    print $fh "\n";

    print $fh "#endif\n";

    close($fh);

    print "Output: $file\n";
}

sub generate_bchp_resources_h_file {
    my ($nonleafs, $file) = @_;

    open(my $fh, ">$file") or die "Can't open output file $file";

    my @lines = generate_brcm_copyright_header();
    print $fh @lines;

    print $fh "/***************************************************************************\n";
    print $fh "*\n";
    print $fh "* This file is auto-generated\n";
    print $fh "*\n";
    print $fh "* This file contains a list of power resource IDs that can be\n";
    print $fh "* acquired and released.\n";
    print $fh "*\n";
    print $fh "***************************************************************************/\n";
    print $fh "\n";
    print $fh "#ifndef BCHP_PWR_RESOURCES_H__\n";
    print $fh "#define BCHP_PWR_RESOURCES_H__\n";
    print $fh "\n";

    # get the length of the longest node string
    my $maxlen = 0;
    foreach (keys %$nonleafs) {
        if (length > $maxlen) {
            $maxlen = length;
        }
    }
    $maxlen += length("BCHP_PWR_RESOURCE_")+3;

    my $idx = 1;
    foreach (sort keys %$nonleafs) {
        printf $fh ("#define %-${maxlen}s 0x%08x\n", "BCHP_PWR_RESOURCE_$_", $idx++);
    }

    print $fh "\n";
    print $fh "#endif\n";

    close $fh;

    print "Output: $file\n";
}

sub generate_resource_string
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $type = shift;
    my $elements = shift;
    my @lines;

    foreach (@$elements) {
        push @lines, "const $prefix1 ${prefix1}_${_}[] = {{\n";
        push @lines, "    ${prefix1}Type_e$type,\n";
        push @lines, "    ${prefix2}_${_},\n";
        push @lines, "    BDBG_STRING(\"$_\")\n";
        push @lines, "}};\n\n";
    }

    return @lines;
}

sub generate_array_string
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $prefix3 = shift;
    my $prefix4 = shift;
    my $nodes = shift;
    my @lines;

    push @lines, "const $prefix1 ${prefix2}[${prefix3}] = {\n";
    foreach (@$nodes) {
        if ($_ eq "NULL") {
            push @lines, "    NULL,\n";
        } else {
            push @lines, "    ${prefix4}$_,\n";
        }
    }
    push @lines, "};\n\n";

    return @lines;
}

sub generate_dependency_string
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $graph = shift;
    my $nodes = shift;
    my @lines;

    foreach (@$nodes) {
        push @lines, "static const ${prefix1}* const $prefix2${_}[] = {\n";
        foreach my $dep (@{$graph->{$_}{_order}}) {
            push @lines, "    ${prefix1}_${dep},\n";
        }
        push @lines, "    NULL\n";
        push @lines, "};\n\n";
    }

    return @lines;
}

sub generate_hwcontrol_function_string
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $nodes = shift;
    my @lines;

    push @lines, "#include \"bchp_pwr_impl.c\"\n";
    push @lines, "#include \"bchp_pwr_common.c\"\n";
    push @lines, "\n";
    push @lines, "void BCHP_PWR_P_HW_ControlId(BCHP_Handle handle, unsigned id, bool activate)\n";
    push @lines, "{\n";
    push @lines, "    switch(id) {\n";
    foreach (@$nodes) {
        push @lines, "        case ${prefix1}_${_}:\n";
        push @lines, "            ${prefix2}_${_}_Control(handle, activate);\n";
        push @lines, "            break;\n";
    }
    push @lines, "        default:\n";
    push @lines, "            BDBG_ASSERT(0);\n";
    push @lines, "            break;\n";
    push @lines, "    }\n";
    push @lines, "}\n\n";
    push @lines, "void BCHP_PWR_P_HW_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, bool activate)\n";
    push @lines, "{\n";
    push @lines, "    BCHP_PWR_P_HW_ControlId(handle, resource->id, activate);\n";
    push @lines, "}\n";

    return @lines;
}

sub generate_mxcontrol_function_string
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $nodes = shift;
    my @lines;

    push @lines, "\n";
    push @lines, "void BCHP_PWR_P_MUX_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, unsigned *mux, bool set)\n";
    push @lines, "{\n";
    if(scalar(@$nodes) == 0) {
        push @lines, "    BSTD_UNUSED(handle);\n";
        push @lines, "    BSTD_UNUSED(mux);\n";
        push @lines, "    BSTD_UNUSED(set);\n";
    }
    push @lines, "\n";
    push @lines, "    switch(resource->id) {\n";
    foreach (@$nodes) {
        push @lines, "        case ${prefix1}_${_}:\n";
        push @lines, "            ${prefix2}_${_}_Control(handle, mux, set);\n";
        push @lines, "            break;\n";
    }
    push @lines, "        default:\n";
    push @lines, "            BDBG_ASSERT(0);\n";
    push @lines, "            break;\n";
    push @lines, "    }\n";
    push @lines, "}\n";

    return @lines;
}

sub generate_dvcontrol_function_string
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $nodes = shift;
    my @lines;

    push @lines, "\n";
    push @lines, "void BCHP_PWR_P_DIV_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)\n";
    push @lines, "{\n";
    if(scalar(@$nodes) == 0) {
        push @lines, "    BSTD_UNUSED(handle);\n";
        push @lines, "    BSTD_UNUSED(mult);\n";
        push @lines, "    BSTD_UNUSED(prediv);\n";
        push @lines, "    BSTD_UNUSED(postdiv);\n";
        push @lines, "    BSTD_UNUSED(set);\n";
    }
    push @lines, "\n";
    push @lines, "    switch(resource->id) {\n";
    foreach (@$nodes) {
        push @lines, "        case ${prefix1}_${_}:\n";
        push @lines, "            ${prefix2}_${_}_Control(handle, mult, prediv, postdiv, set);\n";
        push @lines, "            break;\n";
    }
    push @lines, "        default:\n";
    push @lines, "            BDBG_ASSERT(0);\n";
    push @lines, "            break;\n";
    push @lines, "    }\n";
    push @lines, "}\n";

    return @lines;
}

sub generate_pmap_table
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $nodes = shift;
    my $hw_desc = shift;
    my (@lines1, @lines2, @lines3, @lines);
    my %pmap_table;
    my %div_table;
    my $cnt = 0;

    foreach (sort @$nodes) {
        foreach my $reg (sort keys %{$hw_desc->{$_}}) {
            foreach my $type (keys %{$hw_desc->{$_}{$reg}{_field}}) {
                foreach my $field (keys %{$hw_desc->{$_}{$reg}{_field}{$type}}) {
                    if($field eq "_pmap") {next;}
                    if($field eq "_shared") {next;}
                    if(!$hw_desc->{$_}{$reg}{_field}{$type}{_shared}) {
                        if (exists $hw_desc->{$_}{$reg}{_field}{$type}{_pmap}) {
                            foreach my $pmap (0 .. 8) {
                                if (exists $hw_desc->{$_}{$reg}{_field}{$type}{_pmap}{$pmap}{_pstate}{0}) {
                                    $pmap_table{$reg}{$field}{_value} = $hw_desc->{$_}{$reg}{_field}{$type}{_pmap}{$pmap}{_pstate}{0};
                                    last;
                                }
                            }
                        } else {
                            $pmap_table{$reg}{$field}{_value} = $hw_desc->{$_}{$reg}{_field}{$type}{$field};
                        }
                        $pmap_table{$reg}{$field}{_type} = $type;
                        push(@{$pmap_table{$reg}{$field}{_nodes}}, $_);
                    }
                }
            }
        }
    }

    push @lines, "\n";
    push @lines1, "const BCHP_PmapSettings ${prefix2}_DefaultPMapSettings[BCHP_PWR_NUM_P_PMAPSETTINGS] = {\n";

    foreach my $reg (sort keys %pmap_table) {
        foreach my $field (sort keys $pmap_table{$reg}) {
            push @lines1, "    PMAP(${reg}, ${field}, $pmap_table{$reg}{$field}{_value}),\n";
            #push @lines1, "    {$pmap_table{$reg}{$field}{_value}, BCHP_${reg}_${field}_SHIFT, BCHP_${reg}_${field}_MASK, BCHP_${reg}},\n";
            foreach (sort @{$pmap_table{$reg}{$field}{_nodes}}) {
                $div_table{$_}{$pmap_table{$reg}{$field}{_type}} = $cnt;
            }
            $cnt++;
        }
    }
    $num_pmap_settings = $cnt;

    push @lines1, "};\n\n";
    push @lines3, "const ${prefix2}_FreqMap ${prefix2}_FreqMapList[BCHP_PWR_P_NUM_DIVS] = {\n";

    foreach (sort keys %div_table) {
        if(!exists $div_table{$_}{MULT}) { $div_table{$_}{MULT} = -1; }
        if(!exists $div_table{$_}{PREDIV}) { $div_table{$_}{PREDIV} = -1; }
        if(!exists $div_table{$_}{POSTDIV}) { $div_table{$_}{POSTDIV} = -1; }
        push @lines2, "DIVTABLE(${_}, $div_table{$_}{MULT}, $div_table{$_}{PREDIV}, $div_table{$_}{POSTDIV});\n";
        push @lines3, "    FREQMAP(${_}),\n";

        #push @lines2, "const ${prefix2}_DivTable ${prefix2}_DivTable_${_}[] = {";
        #push @lines2, "{$div_table{$_}{MULT}, $div_table{$_}{PREDIV}, $div_table{$_}{POSTDIV}}";
        #push @lines2, "};\n";
        #push @lines3, "    {${prefix1}_${_}, ${prefix2}_DivTable_${_}},\n";
    }

    push @lines2, "\n";
    push @lines3, "};";

    push @lines, @lines1;
    push @lines, @lines2;
    push @lines, @lines3;
    push @lines, "\n";

    #print Dumper (%pmap_table);
    #print Dumper (%div_table);

    return @lines;
}

sub generate_freq_map
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $prefix3 = shift;
    my $nodes = shift;
    my $hw_desc = shift;
    my (@lines1, @lines2, @lines);
    my %div_table;
    push @lines, "\n";
    push @lines2, "const ${prefix3} ${prefix3}List[BCHP_PWR_P_NUM_DIVS] = {\n";

    foreach (sort @$nodes) {
        foreach my $reg (sort keys %{$hw_desc->{$_}}) {
            foreach my $type (keys %{$hw_desc->{$_}{$reg}{_field}}) {
                push(@{$div_table{$_}{$type}}, "BCHP_".$reg);
            }
        }

        if (exists $div_table{$_}{MULT} && exists $div_table{$_}{PREDIV} && exists $div_table{$_}{POSTDIV}) {
            push @lines1, "const ${prefix2} ${prefix2}_${_}[] = {";
            push @lines1, "{$div_table{$_}{MULT}[0],$div_table{$_}{PREDIV}[0],$div_table{$_}{POSTDIV}[0]}";
            push @lines1, "};\n";
            push @lines2, "    {${prefix1}_${_}, ${prefix2}_${_}},\n";
        }
    }
    push @lines2, "};\n";

    push @lines, @lines1;
    push @lines, "\n";
    push @lines, @lines2;

    #print Dumper (%div_table);

    return @lines;
}

sub generate_dv_table
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $prefix3 = shift;
    my $nodes = shift;
    my $hw_desc = shift;
    my (@lines1, @lines2, @lines);
    my ($mult, $prediv, $postdiv);
    my %div_table;
    push @lines, "\n";
    push @lines2, "const ${prefix3} ${prefix3}List[BCHP_PWR_P_NUM_DIVS] = {\n";

    foreach (sort @$nodes) {
        my $postdiv = 1;
        my $prediv = 1 ;
        my $mult = 1;
        foreach my $reg (sort keys %{$hw_desc->{$_}}) {
            foreach my $type (keys %{$hw_desc->{$_}{$reg}{_field}}) {
                foreach my $field (keys %{$hw_desc->{$_}{$reg}{_field}{$type}}) {
                    if($field eq "_pmap") {next;}
                    if($field eq "_shared") {next;}
                    if(exists $hw_desc->{$_}{$reg}{_field}{$type}{_pmap}) {
                        my $entries = scalar keys %{$hw_desc->{$_}{$reg}{_field}{$type}{_pmap}};
                        if($num_profiles < $entries) {
                            print "Mismatch between PMPowerProfile tag and number of profiles profiles found for $reg:$type\n";
                        }
                        foreach my $profile (0..$num_profiles-1) {
                            if(exists $hw_desc->{$_}{$reg}{_field}{$type}{_pmap}{$profile}) {
                                if(exists $hw_desc->{$_}{$reg}{_field}{$type}{_pmap}{$profile}{_pstate}{0}) {
                                    push(@{$div_table{$_}{$type}}, $hw_desc->{$_}{$reg}{_field}{$type}{_pmap}{$profile}{_pstate}{0});
                                } else {
                                    print "Undefined PMap $profile for $reg:$type. Using default $hw_desc->{$_}{$reg}{_field}{$type}{$field}\n";
                                    push(@{$div_table{$_}{$type}}, $hw_desc->{$_}{$reg}{_field}{$type}{$field});
                                }
                            } else {
                                if($profile == $num_profiles) {
                                    push(@{$div_table{$_}{$type}}, $hw_desc->{$_}{$reg}{_field}{$type}{$field});
                                } else {
                                    push(@{$div_table{$_}{$type}}, 0);
                                }
                            }
                        }
                    } else {
                        #print "no PMAP found for $reg:$type\n";
                        if ($type eq "MULT") {
                            $mult = 0;
                        } elsif ($type eq "PREDIV") {
                            $prediv = 0;
                        } elsif ($type eq "POSTDIV") {
                            $postdiv = 0;
                        }
                        foreach my $profile (0..$num_profiles-1) {
                            push(@{$div_table{$_}{$type}}, $hw_desc->{$_}{$reg}{_field}{$type}{$field});
                        }
                    }
                }
            }
        }
        if(exists $div_table{$_}{MULT}) {
            my $len= scalar(@{$div_table{$_}{MULT}});
            if($num_profiles>$len+1) {
                printf "Missing entries for $:MULT\n";
                next;
            }
        } else {
            printf "$_:MULT does not exist\n";
            next;
        }
        if(exists $div_table{$_}{PREDIV}) {
            my $len= scalar(@{$div_table{$_}{PREDIV}});
            if($num_profiles>$len+1) {
                printf "Missing entries for $:PREDIV\n";
                next;
            }
        } else {
            printf "$_:PREDIV does not exist\n";
            next;
        }
        if(exists $div_table{$_}{POSTDIV}) {
            my $len= scalar(@{$div_table{$_}{POSTDIV}});
            if($num_profiles>$len+1) {
                printf "Missing entries for $_:POSTDIV\n";
                next;
            }
        } else {
            printf "$_:POSTDIV does not exist\n";
            next;
        }
        if ($prediv == 0 && $mult == 0 && $postdiv == 0) {
            print "No Pmap for $_\n";
            delete $div_table{$_};
        }

        if (exists $div_table{$_}{MULT} && exists $div_table{$_}{PREDIV} && exists $div_table{$_}{POSTDIV}) {
            push @lines1, "const ${prefix2} ${prefix2}_${_}[] = {";
            foreach my $profile (0..$num_profiles-1) {
                push @lines1, "{$div_table{$_}{MULT}[$profile],$div_table{$_}{PREDIV}[$profile],$div_table{$_}{POSTDIV}[$profile]},";
            }
            push @lines1, "};\n";
            push @lines2, "    {${prefix1}_${_}, ${prefix2}_${_}},\n";
        }
    }
    push @lines2, "};\n";

    push @lines, @lines1;
    push @lines, "\n";
    push @lines, @lines2;

    #print Dumper (%div_table);

    return @lines;
}

sub generate_mx_table
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $prefix3 = shift;
    my $nodes = shift;
    my $hw_desc = shift;
    my (@lines1, @lines2, @lines);
    my %mux_table;
    #my $num_profiles = scalar @profiles;
    push @lines, "\n";
    push @lines2, "const ${prefix3} ${prefix3}List[BCHP_PWR_P_NUM_MUXES] = {\n";

    foreach (sort @$nodes) {
        foreach my $reg (sort keys %{$hw_desc->{$_}}) {
                foreach my $elem (keys %{$hw_desc->{$_}{$reg}{_field}{_mux}}) {
                    if($elem eq "_pmap") {next;}
                    if($elem eq "_shared") {next;}
                    if(exists $hw_desc->{$_}{$reg}{_field}{_mux}{_pmap}) {
                        my $entries = scalar keys %{$hw_desc->{$_}{$reg}{_field}{_mux}{_pmap}};
                        if($num_profiles < $entries) {
                            print "Mismatch between PMPowerProfile tag ($entries) and number of profiles ($num_profiles) found for $reg:$elem\n";
                        }
                        foreach my $profile (0..$num_profiles-1) {
                            if(exists $hw_desc->{$_}{$reg}{_field}{_mux}{_pmap}{$profile}) {
                                push(@{$mux_table{$_}}, $hw_desc->{$_}{$reg}{_field}{_mux}{_pmap}{$profile}{_pstate}{0});
                            } else {
                                print "No Pstate found for PMPowerProfile $profile ($reg:$elem). Using default $hw_desc->{$_}{$reg}{_field}{_mux}{$elem}\n";
                                push(@{$mux_table{$_}}, $hw_desc->{$_}{$reg}{_field}{_mux}{$elem});
                            }
                        }
                    } else {
                        #print "No PMap found for $reg:$elem\n";
                        foreach my $profile (0..$num_profiles-1) {
                            push(@{$mux_table{$_}}, $hw_desc->{$_}{$reg}{_field}{_mux}{$elem});
                        }
                    }
                }
        }
        if(exists $mux_table{$_}) {
            my $len= scalar(@{$mux_table{$_}});
            if($num_profiles>$len+1) {
                printf "Missing entries for $\n";
                next;
            }
            push @lines1, "const ${prefix2} ${prefix2}_${_}[] = {";
            foreach my $profile (0..$num_profiles-1) {
                push @lines1, "{$mux_table{$_}[$profile]},";
            }
            push @lines1, "};\n";
            push @lines2, "    {${prefix1}_${_}, ${prefix2}_${_}},\n";
        } else {
            printf " No Mux table found for $_\n";
            next;
        }
    }
    push @lines2, "};\n";

    push @lines, @lines1;
    push @lines, "\n";
    push @lines, @lines2;

    #print Dumper (%mux_table);

    return @lines;
}

sub generate_bchp_resources_c_file {
    my ($nonleafs, $nonleafshw, $leafs, $nonleafsdv, $leafsdv, $nonleafsmx, $leafsmx, $nodes, $hw_desc, $file) = @_;
    my @nonleafs = sort keys %$nonleafs;
    my @nonleafshw = sort keys %$nonleafshw;
    my @leafs = sort keys %$leafs;
    my @hw_all = (@nonleafshw, @leafs);
    my @nonleafsdv = sort keys %$nonleafsdv;
    my @leafsdv = sort keys %$leafsdv;
    my @DV_all = (@nonleafsdv, @leafsdv);
    my @nonleafsmx = sort keys %$nonleafsmx;
    my @leafsmx = sort keys %$leafsmx;
    my @MX_all = (@nonleafsmx, @leafsmx);
    @hw_all = sort @hw_all;
    @DV_all = sort @DV_all;
    @MX_all = sort @MX_all;

    open(my $fh, ">$file") or die "Can't open output file $file";

    my @lines = generate_brcm_copyright_header();
    print $fh @lines;

    print $fh "/***************************************************************************\n";
    print $fh "*\n";
    print $fh "* This file is auto-generated\n";
    print $fh "*\n";
    print $fh "* This file contains a coded representation of power resource\n";
    print $fh "* nodes and their dependencies.\n";
    print $fh "*\n";
    print $fh "***************************************************************************/\n";
    print $fh "\n";
    print $fh "#include \"bchp_pwr.h\"\n";
    print $fh "#include \"bchp_pwr_resources.h\"\n";
    print $fh "#include \"bchp_pwr_resources_priv.h\"\n";
    print $fh "\n";

    print $fh "/* Resource definitions */\n";
    @lines = generate_resource_string("BCHP_PWR_P_Resource", "BCHP_PWR_RESOURCE", "NonLeaf", \@nonleafs);
    print $fh @lines;

    @lines = generate_resource_string("BCHP_PWR_P_Resource", "BCHP_PWR", "Leaf", \@leafs);
    print $fh @lines;

    # NonLeafHw nodes are tacked-on at the end
    @lines = generate_resource_string("BCHP_PWR_P_Resource", "BCHP_PWR", "NonLeafHw", \@nonleafshw);
    print $fh @lines;

    @lines = generate_resource_string("BCHP_PWR_P_Resource", "BCHP_PWR", "Mux", \@MX_all);
    print $fh @lines;

    @lines = generate_resource_string("BCHP_PWR_P_Resource", "BCHP_PWR", "Div", \@DV_all);
    print $fh @lines;

    my @resources;
    push @resources, @nonleafs;
    push @resources, @hw_all;
    push @resources, @MX_all;
    push @resources, @DV_all;
    print $fh "/* List of resources */\n";
    @lines = generate_array_string("BCHP_PWR_P_Resource* const", "BCHP_PWR_P_ResourceList",
        "BCHP_PWR_P_NUM_ALLNODES", "BCHP_PWR_P_Resource_", \@resources);
    print $fh @lines;

    print $fh "/* Coded dependencies */\n";
    @lines = generate_dependency_string("BCHP_PWR_P_Resource", "BCHP_PWR_P_Depend_", $nodes, \@nonleafs);
    print $fh @lines;

    # NonLeafHw nodes are tacked-on at the end
    @lines = generate_dependency_string("BCHP_PWR_P_Resource", "BCHP_PWR_P_Depend_", $nodes, \@nonleafshw);
    print $fh @lines;

    @lines = generate_dependency_string("BCHP_PWR_P_Resource", "BCHP_PWR_P_Depend_", $nodes, \@nonleafsmx);
    print $fh @lines;

    @lines = generate_dependency_string("BCHP_PWR_P_Resource", "BCHP_PWR_P_Depend_", $nodes, \@nonleafsdv);
    print $fh @lines;

    foreach (@resources) {
        if (!exists $nodes->{$_}) {
            $_ = "NULL";
        }
    }

    print $fh "/* List of coded dependencies */\n";
    @lines = generate_array_string("BCHP_PWR_P_Resource* const * const", "BCHP_PWR_P_DependList",
        "BCHP_PWR_P_NUM_ALLNODES", "BCHP_PWR_P_Depend_", \@resources);
    print $fh @lines;

    @lines = generate_hwcontrol_function_string("BCHP_PWR", "BCHP_PWR_P", \@hw_all);
    print $fh @lines;

    @lines = generate_mxcontrol_function_string("BCHP_PWR", "BCHP_PWR_P", \@MX_all);
    print $fh @lines;

    @lines = generate_dvcontrol_function_string("BCHP_PWR", "BCHP_PWR_P", \@DV_all);
    print $fh @lines;

    #@lines = generate_mx_table("BCHP_PWR", "BCHP_PWR_P_MuxTable", "BCHP_PWR_P_MuxMap", \@MX_all, $hw_desc);
    #print $fh @lines;

    #@lines = generate_dv_table("BCHP_PWR", "BCHP_PWR_P_DivTable", "BCHP_PWR_P_FreqMap", \@DV_all, $hw_desc);
    #print $fh @lines;

    #@lines = generate_freq_map("BCHP_PWR", "BCHP_PWR_P_DivTable", "BCHP_PWR_P_FreqMap", \@DV_all, $hw_desc);
    #print $fh @lines;

    @lines = generate_pmap_table("BCHP_PWR", "BCHP_PWR_P", \@DV_all, $hw_desc);
    print $fh @lines;

    close $fh;

    print "Output: $file\n";
}

use Cwd;
use Cwd 'abs_path';
use Sys::Hostname;
use File::Basename;

sub get_bchp_path {
    my ($chp, $ver) = @_;
    my $cur_dir = abs_path(getcwd);
    my $src_dir = ".";
    my $inc_dir = ".";
    if ($cur_dir =~ /(BSEAV|nexus|magnum|rockford)/) {
        my $top_dir = substr($cur_dir, 0, rindex($cur_dir, $1));
        if (-e ($top_dir."BSEAV") &&
            -e ($top_dir."nexus") &&
            -e ($top_dir."magnum") &&
            -e ($top_dir."rockford")) {
            my $ver_lc = lc $ver;
            $src_dir = $top_dir."magnum/basemodules/chp/src/".$chp."/pwr/".$ver_lc;
            $inc_dir = $top_dir."magnum/basemodules/chp/include/".$chp."/common/pwr/".$ver_lc;
        }
    }

    return ($src_dir, $inc_dir);
}

sub generate_bchp_files {
    my ($nodes, $hw_desc, $chp, $ver) = @_;
    my (%nonleafs, %nonleafshw, %leafs, %nonleafsdv, %leafsdv, %nonleafsmx, %leafsmx);

    foreach my $node (keys %$nodes) {
        if($node !~ /^(HW|DV|MX)_/) {
            $nonleafs{$node}++;
        } elsif ($node =~ /^HW_/) {
            $nonleafshw{$node}++;
        } elsif ($node =~ /^DV_/) {
            $nonleafsdv{$node}++;
        } elsif ($node =~ /^MX_/) {
            $nonleafsmx{$node}++;
        }
        foreach my $child_node (@{$nodes->{$node}{_order}}) {
            if($child_node !~ /^(HW|DV|MX)_/) {
                $nonleafs{$child_node}++;
                if(!exists $nodes->{$child_node}) {
                    print "Error : Non leaf node $child_node has no child nodes\n";
                }
            } elsif ($child_node =~ /^HW_/) {
                if(exists $nodes->{$child_node}) {
                    $nonleafshw{$child_node}++;
                } else {
                    $leafs{$child_node}++;
                }
            } elsif ($child_node =~ /^DV_/) {
                if(exists $nodes->{$child_node}) {
                    $nonleafsdv{$child_node}++;
                } else {
                    $leafsdv{$child_node}++;
                }
            } elsif ($child_node =~ /^MX_/) {
                if(exists $nodes->{$child_node}) {
                    $nonleafsmx{$child_node}++;
                } else {
                    print "Leaf mux node found $child_node\n";
                    $leafsmx{$child_node}++;
                }
            }
        }
    }

    my ($src_dir, $inc_dir) = get_bchp_path($chp, $ver);
    generate_bchp_resources_txt_file($nodes, $src_dir."/bchp_pwr_resources_".$chp.".txt");
    generate_bchp_impl_c_file($hw_desc, $src_dir."/bchp_pwr_impl.c");
    generate_bchp_resources_h_file(\%nonleafs, $inc_dir."/bchp_pwr_resources.h");
    generate_bchp_resources_c_file(\%nonleafs, \%nonleafshw, \%leafs, \%nonleafsdv, \%leafsdv, \%nonleafsmx, \%leafsmx, $nodes, $hw_desc, $src_dir."/bchp_pwr_resources.c");
    generate_bchp_resources_priv_h_file(\%nonleafs, \%nonleafshw, \%leafs, \%nonleafsdv, \%leafsdv, \%nonleafsmx, \%leafsmx, $src_dir."/bchp_pwr_resources_priv.h");
}

sub get_rdb_path {
    my ($chp, $ver) = @_;
    my $cnt = 0;
    my $prefix = "projects";
    my $cur_dir = getcwd;
    my $dir;

    if (hostname =~ /stbirv-bld-1/) {
        $prefix = "project_it";
    }

    $dir = "/".$prefix."/BCM".$chp."/".$ver."/snapshot";

    while ( -l $dir ) {
        my($filename, $directory, $suffix) = fileparse($dir);

        chdir($directory);

        $dir = readlink($filename) ;
        $dir =~ s/projects/$prefix/;

        $cnt++;
        if ($cnt > 10) { last; }
    }

    chdir($cur_dir);

    return $dir;
}

sub main {
	my (%clk_tree, %functions, %nodes, %hw_desc, %featuredef, %reg_desc);
    my ($chp, $ver, $dir, $dbg);

    foreach (@ARGV) {
        if(/^\d\d\d\d\d?$/) {
            $chp = $_;
        }
        if(/^[a-zA-Z]\d$/) {
            $ver = uc($_);
        }
        if(/\/project_it\//) {
            $dir = $_;
        }
        if(/perf/) {
            $perf = 1;
        }
        if(/debug/) {
            $dbg = 1;
        }
        if(/nodac/) {
            $disable_aud_dac = 1;
        }
        if(/no656/) {
            $disable_vec_656 = 1;
        }
    }

    if(not defined $chp) {
        print "Please specify chip number";
        return;
    }
    if(not defined $ver) {
        print "Please specify chip version";
        return;
    }
    if(not defined $dir) {
        $dir = get_rdb_path($chp, $ver);
    }

    if($chp == 7271 || $chp == 7268 || $chp == 7260) {
        $disable_aud_dac = 1
    } elsif($chp == 7255) {
        $disable_vec_656 = 1;
    }

    print "\n\n";
    print "Processing clock tree\n";
    print "Chip    : $chp\n";
    print "Version : $ver\n";
    print "Using RDB $dir\n";
    if( defined $perf) {
        print "\nGenerating files for performance tools only\n"
    }
    print"\n\n";

    my @files = (
        "/design/brahms/iop/rdb/aud_iop_top.rdb",
        "/design/clkgen/rdb/clkgen_top.rdb",
        "/design/sys_ctrl/rdb/top_ctrl.rdb"
    );

    foreach (@files) {
        my $file = $dir.$_;
        parse_rdb_file($file, \%clk_tree, \%featuredef, \%reg_desc, '');
    }

    print "Found $num_aud_dacs AUD_DACS, $num_aud_plls AUD_PLLS, $num_vcxo_plls VCXO_PLLS\n";

	verify_tree(\%clk_tree);

	generate_top_level_nodes(\%clk_tree, \%functions);

    if(not defined $perf) {
        generate_nexus_nodes(\%clk_tree, \%functions, \%nodes, \%hw_desc);

        generate_user_defined_nodes(\%nodes, \%featuredef);

        generate_bchp_files(\%nodes, \%hw_desc, $chp, $ver);

        if(defined $dbg) {
            my $fh;

            print_tree(\%clk_tree, \%functions);

            open($fh, ">clk.txt") or die "Can't open output file";
            print $fh Dumper (%clk_tree);
            close $fh;
            open($fh, ">parent.txt") or die "Can't open output file";
            print $fh Dumper (%parent_map);
            close $fh;
            open($fh, ">func.txt") or die "Can't open output file";
            print $fh Dumper (%functions);
            close $fh;
            open($fh, ">nodes.txt") or die "Can't open output file";
            print $fh Dumper (%nodes);
            close $fh;
            open($fh, ">desc.txt") or die "Can't open output file";
            print $fh Dumper (%hw_desc);
            close $fh;
            open($fh, ">profiles.txt") or die "Can't open output file";
            print $fh Dumper (@profiles);
            close $fh;
        }
    } else {
        print_power_tree(\%clk_tree, \%functions, $chp);
        print_power_clk_tree(\%clk_tree, $chp);
    }
}

exit main();
