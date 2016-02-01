#!/usr/bin/perl
#############################################################################
#
#		Copyright (c) 2003-2009, Broadcom Corporation.
#		All rights reserved.
#		Confidential Property of Broadcom Corporation.
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
# use strict;
my $line;
sub numerically { $a <=> $b }

@pads = (0,3,2,1);

print "build estb_cfg_values c file based on text file\n";

open(ESTB_CFG, "./estb_cfg.txt") or die "Can't open $!\n";
$index = 0;
$offset = 0;
$file_id = -1;
while ($line = <ESTB_CFG>) {
    # print "$line\n";
    $line =~ s/^ //g;
    # skip comment 
    if ($line =~ /^#/) {
	next;
    }
    chop($line);
    if ($line eq "/") {
	next;
    }
    if ($line =~ /^\[/) {
	$line =~ s/^\[//g;
	$line =~ s/\]//g;
	# parsing [file id, default file name]
	# end of last section
	if ($file_id >= 0) {
	    $ctl->{num_nodes} = $index;
	    $ctl->{data_len} = $offset;
	    push @ctl_block, $ctl;
	    push @node_blocks, [ @node_block ];
	    # print "test :$node_block[0]->{path}, $node_blocks[0][0]->{path}\n"
	}
	# beginning of new session
	$file_id++;
	($top_label, $path, $default_fn) = split /,\s*/, $line, 3;
	$ctl = {
	    file_id => 0, 
	    top_label => "", 
	    default_fn => "", 
	    num_nodes => 0,
	    data_len => 0,
	    path => "",
	};

	$ctl->{file_id} = $file_id;
	$ctl->{top_label} = $top_label;
	$ctl->{default_fn} = $default_fn;
	$ctl->{path} = $path;
	$index = 0;
	$offset = 0;
	@node_block = ();
	next;
    }
    ($path, $rest) = split /,\s*/, $line, 2;
    if ($rest) {
	# values
	print "$path: $rest\n";
	($file_id >= 0) or die " file id not found\n";
	($type, $len, $label, $value) = split(",", $rest);
	$node = {};
	($node{index}, $node{path}, $node{type}, $node{len}, $node{label}, $node{value})
	    = ($index, $path, $type, $len, $label, $value);
	# push @node_block, $node;
	$pad = $pads[$len % 4];

	push @node_block, {index => $index, path => $path, type => $type, len => $len, 
		       label =>$label, value => $value, pad => $pad };
	foreach $elem (@node_block) {
	    print "$elem->{path}, $elem->{type}\n";
	}
	$index++;
	$offset += $len;
    } 
    elsif ($path) {
	# directory 
	($file_id >= 0) or die " file id not found\n";
	push @node_block, {index => $index, path => $path, type => "dir"};
	print "$path";
	$index++;
    } 
    else {
	break;
    }
}
close(ESTB_CFG);

# last file id
if ($file_id >= 0) {
    $ctl->{num_nodes} = $index;
    $ctl->{data_len} = $offset;
    push @ctl_block, $ctl;
    push @node_blocks, [ @node_block ];
}



$num_nodes = $index;
$num_files = $file_id + 1;

print "ctl block\n";
foreach $elem (@ctl_block) {
    print "file_id = $elem->{file_id}, top_label = $elem->{top_label}, ";
    print "default_fn = $elem->{default_fn}, num_nodes = $elem->{num_nodes}, ";
    print "data_len = $elem->{data_len}\n";

}

print "num of files $num_files\n";
foreach $file_id (sort numerically keys %ctl_block) {
    for $i (0 .. $#{ ${ctl_block{$file_id}} } ) {
	print "[$i] = $ctl_block{$file_id}[$i] ";
    }
    print "\n";
}

for $k (0 .. $#{ @node_blocks}) { 
    $ctl = $ctl_block[$k];
    print "k = $k num of nodes $ctl->{num_nodes}\n";
#    for $i (0 .. $#{ $node_blocks[$k]} ) {
    for $i (0 .. ($ctl->{num_nodes} - 1) ) {
	$node = $node_blocks[$k][$i];
	print "i = $i $node->{index}, $node->{path},$node->{type}, $node->{len}, $node->{pad}, $node->{label}, $node->{value}\n";
    }
    print "\n";
}

##### now building the C-code #####
 
$offset = 0;
open(ESTB_CFG_OUT, ">./b_estb_cfg_values.c") or die "Can't open $!\n";
$num_files = $#{ @node_blocks} + 1;
print ESTB_CFG_OUT "#define B_ESTB_CFG_NUM_CFG_FILES $num_files\n" ;
for $k (0 .. ($num_files - 1)) { 

    $ctl = $ctl_block[$k];
    $num_nodes = $ctl->{num_nodes} - 1;
    $data_len = $ctl->{data_len};
    %default = ();
    $tmpstr = sprintf "                SET_DEFAULT_NODE_VALUE(node_block_$ctl->{top_label}, default_node_block_$ctl->{top_label});\n";
    $tmpstr .= sprintf "                SET_DEFAULT_CTL_VALUE($ctl->{top_label}, default_ctl_block_$ctl->{top_label});\n";
    $offset = 0;

    # definitions
   #  print ESTB_CFG_OUT "#define DATA_LEN_", $ctl->{top_label}, "  $ctl->{data_len}\n";
    print ESTB_CFG_OUT "#define NUM_NODES_", $ctl->{top_label}, "  $ctl->{num_nodes}\n";

    
    print ESTB_CFG_OUT "struct b_estb_cfg_node node_block_$ctl->{top_label} [$ctl->{num_nodes}];\n";
    print ESTB_CFG_OUT "struct b_estb_cfg_node default_node_block_$ctl->{top_label} [$ctl->{num_nodes}] = {\n";

    print ESTB_CFG_OUT "    /* idx, flag, type, label, parent, next, prev, field len, state, name, data ptr/sub dir, data len/data node, data pad/unused */\n";
    for $index (0 .. $#{ $node_blocks[$k]}) { 
	$node = $node_blocks[$k][$index];
	print ESTB_CFG_OUT "    {$node->{index}, 0,  "; # idx, type 

	if ($node->{type} eq "dir") {
	    print ESTB_CFG_OUT "NODE_TYPE_DIR, B_ESTB_CFG_LABEL_RSVD0, "; # dir

	    # !!! the dir info is not used for now, just fill some data 
	    print ESTB_CFG_OUT "IDX_INVALID, IDX_INVALID, IDX_INVALID, 0, 0, \"$node->{path}\", {{IDX_INVALID, IDX_INVALID,0}}},\n";
	} 
	elsif ($node->{type} eq "uint32") {
	    print ESTB_CFG_OUT "NODE_TYPE_UINT32, $node->{label}, "; 
	    print ESTB_CFG_OUT "IDX_INVALID, IDX_INVALID, IDX_INVALID, NODE_STATE_INIT, $node->{len}, \"$node->{path}\", ";
	    print ESTB_CFG_OUT "{{", $offset, ", ", $node->{len}, ", ", $node->{pad}, "}}},\n";
	    $offset += ($node->{len} + $node->{pad});
	    $node->{label} =~ s/^ //g;
	    $default{$index} = "unsigned int default_$node->{label} = $node->{value};\n" ;
	    $set_default_data_value{$index} = "	        SET_DEFAULT_DATA_VALUE($ctl->{top_label}, $node->{index}, default_$node->{label});\n" ;
	    $tmpstr .= sprintf "$set_default_data_value{$index}";

	}
	elsif ($node->{type} eq "txt") {
	    print ESTB_CFG_OUT "NODE_TYPE_TXT, $node->{label}, "; 
	    # !!! the dir info is not used for now, just fill some data 
	    print ESTB_CFG_OUT "IDX_INVALID, IDX_INVALID, IDX_INVALID, NODE_STATE_INIT, $node->{len}, \"$node->{path}\", ";
	    print ESTB_CFG_OUT "{{", $offset, ", ", $node->{len}, ", ", $node->{pad}, "}}},\n";
	    $offset += ($node->{len} + $node->{pad});
	    $node->{label} =~ s/^ //g;
	    $default{$index} = "char default_$node->{label}\[\] = $node->{value};\n" ;
	    $set_default_data_value{$index} = "	        SET_DEFAULT_DATA_VALUE($ctl->{top_label}, $node->{index}, default_$node->{label});\n" ;
	    $tmpstr .= sprintf "$set_default_data_value{$index}";

	}
	elsif ($node->{type} eq "bin") {
	    print ESTB_CFG_OUT "NODE_TYPE_BIN, $node->{label}, "; 
	    # !!! the dir info is not used for now, just fill some data 
	    print ESTB_CFG_OUT "IDX_INVALID, IDX_INVALID, IDX_INVALID, NODE_STATE_INIT, $node->{len}, \"$node->{path}\", ";
	    print ESTB_CFG_OUT "{{", $offset, ", ", $node->{len}, ", ", $node->{pad}, "}}},\n";
	    $offset += ($node->{len} + $node->{pad});
	    $node->{value} =~ s/\"//g;
	    $node->{value} =~ s/^ //g;
	    $node->{value} =~ s/\ /,/g;
	    $node->{label} =~ s/^ //g;
	    $default{$index} = "unsigned char default_$node->{label}\[\] = {$node->{value}};\n" ;
	    $set_default_data_value{$index} = "	        SET_DEFAULT_DATA_VALUE($ctl->{top_label}, $node->{index}, default_$node->{label});\n" ;
	    $tmpstr .= sprintf "$set_default_data_value{$index}";

	}
	else {
	    printf("Cannot parse type $node->{type}, aborted\n");
	    exit 1;
	}
	
    }
    print ESTB_CFG_OUT "};\n\n";
    push @default_str, $tmpstr;
     
    # data block
    # print ESTB_CFG_OUT "char default_data_block_", $ctl->{top_label}, "[DATA_LEN_", $ctl->{top_label}, "];\n\n";
    # default values
    foreach $index (sort numerically keys %default) {
	print ESTB_CFG_OUT "$default{$index}\n";
    }
    
}
# control block
for $k (0 .. ($num_files - 1)) { 
    $ctl = $ctl_block[$k];
    print ESTB_CFG_OUT "static struct b_estb_cfg_ctl default_ctl_block_$ctl->{top_label} = {\n";
    print ESTB_CFG_OUT "      $ctl->{top_label},\n";
    print ESTB_CFG_OUT "      $ctl->{default_fn},\n";
    print ESTB_CFG_OUT "      0,\n";
    print ESTB_CFG_OUT "      DEFAULT_B_ESTB_CFG_HDR,\n";
    print ESTB_CFG_OUT "      node_block_$ctl->{top_label},\n";
    print ESTB_CFG_OUT "      NUM_NODES_$ctl->{top_label},\n";
    #print ESTB_CFG_OUT "      default_data_block_$ctl->{top_label},\n";
    #print ESTB_CFG_OUT "      DATA_LEN_$ctl->{top_label},\n";
    print ESTB_CFG_OUT "      NULL,\n";
    print ESTB_CFG_OUT "      0,\n";
    print ESTB_CFG_OUT "      0,\n";
    print ESTB_CFG_OUT "      0,\n";
    print ESTB_CFG_OUT "};\n\n";

}
print ESTB_CFG_OUT "static struct b_estb_cfg_ctl ctl_block[B_ESTB_CFG_NUM_CFG_FILES];\n";
print ESTB_CFG_OUT "static struct b_estb_cfg_info info_block[B_ESTB_CFG_NUM_CFG_FILES] = {\n";

for $k (0 .. ($num_files - 1)) { 
    $ctl = $ctl_block[$k];
    print ESTB_CFG_OUT "        {", $ctl->{top_label}, ", \"", $ctl->{path}, "\"},\n";
}
print ESTB_CFG_OUT "};\n\n";

print ESTB_CFG_OUT "static void b_estb_cfg_init_values(int idx) { \n";

for $k (0 .. ($num_files - 1)) { 
    $ctl = $ctl_block[$k];

    print ESTB_CFG_OUT "        if (idx == ", $ctl->{top_label}, ") {\n";
    print ESTB_CFG_OUT "$default_str[$k]";
    print ESTB_CFG_OUT "        };\n\n";

}

print ESTB_CFG_OUT "}\n\n";


close(ESTB_CFG_OUT);

print "Generated file output\n\n";
open(ESTB_CFG, "./b_estb_cfg_values.c") or die "Can't open $!\n";
while ($line = <ESTB_CFG>) {
    print $line;
}
close (ESTB_CFG);
