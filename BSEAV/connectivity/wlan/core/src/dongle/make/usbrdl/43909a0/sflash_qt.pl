#!/usr/local/bin/perl

while ($ARGV[0] =~ /^([\-+])/) {
    $full_arg = $ARGV[0];
    $arg = substr (shift, 1);

   die "Unknown option: $full_arg\n" if  $arg;
}

$qt_file = shift ARGV;
$trx_file = shift ARGV;
$out_file = shift ARGV;
print "qt $qt_file \ntrx: $trx_file\n";

open (INF, $qt_file) || die "Unable to open $qt_file. $!\n";
open (TRX, $trx_file) || die "Unable to open $trx_file. $!\n";
binmode(TRX);


my $trxheader_len = 36;
my $lCt = 0; 

open (OUT, ">", $out_file) || die "Unable to open $out_file. $!\n";	
print "opening $out_file\n";

while (<INF>) {
   if (/INSTANCE/) {
	print OUT $_;
   }
   elsif (/RADIX/) {
	print OUT $_;
   } elsif (/ADDRESS/) {
	if (/([0-9|a-f]+)$/) {
		my $new_len = $1;
		$addr = substr($_, 0, -(length($new_len)+1));
		print "len $new_len, $addr\n";
		$new_len = hex($new_len);
		$new_len += $trxheader_len;
		$addr = sprintf("$addr%0x",$new_len); 
		print OUT "$addr\n";
	} else {
		print OUT $_;
		print "not find\n";
	}
   }
   elsif (/END/) {
	print OUT $_;
   }
   else {
	if($lCt == 0) {
	while( $lCt < $trxheader_len) {
		read(TRX, $buf, 1);
	        $hexval = sprintf("%08X    ", $lCt);
		$hdr = sprintf("%02X\n", ord($buf));
		my $trx_line = $hexval.$hdr;
		print OUT $trx_line;
		$lCt++;
	}
	}
        $hexval = sprintf("%08X", $lCt);
        s/^......../$hexval/;
	print OUT $_; 
	$lCt++;
   }
}

 
close (INF);
close (OUT);
close (TRX);
