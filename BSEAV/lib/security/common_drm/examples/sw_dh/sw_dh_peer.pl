#!/usr/local/bin/perl
###
### sw_dh_peer.pl : Software DH peer
###
### This programs can be use to simulate a Diffie Hellman's exchange
### It requires openssl binary if manipulating PEM file
### 
### ./sw_dh_peer.pl [options]
### options:
### -new create a new DH parameters PEM file
### -pem use existing DH parameters PEM file
### -g override generator
### -m override modulo
### -e override private key (exp) ; mandatory
### -v verbose: print DH parameters (g, m and e)
### -s check result against this secret
###    otherwize print result on standard output
### 
### Basically it generates a new DH parameters (generator and modulo) PEM file,
### computes public keys, computes shared secrets and compare results.
### Note: Prime key generation for private key use is out of the scope of this program.
###
### example, bash script, simulating 2 peers DH1 and DH2 having a Diffie Hellman exchange:
###
#
# # step 1, DH1: generate a new pem file, and output DH1 public key given D1 the private key 'D1D1D1'
# DH1_public=$(./sw_dh_peer.pl -new example.pem -e D1D1D1)
#
# # step 2, DH2: use generated pem file in step 1, and output DH2 public key (using private key 'D2D2D2')
# DH2_public=$(./sw_dh_peer.pl -pem example.pem -e D2D2D2)
#
# # step 3, DH1: compute shared secret using DH1 parameters and DH2 public key
# DH1_shared=$(./sw_dh_peer.pl -pem example.pem -e D1D1D1 -g $DH2_public)
#
# # step 4, DH2: compute shared secret using DH2 parameters and DH1 public key; compare to DH1 shared secret
# ./sw_dh_peer.pl -pem example.pem -e D2D2D2 -g $DH1_public -s $DH1_shared -v
#
# # step 5: result can be checked using return code
# if [ $? == 0 ]; then  echo "OK"; else echo "KO" ; fi
#
# unset DH1_public
# unset DH2_public
# unset DH1_shared
#
###

# initializes DH parameters
$g=''; #generator, public from the PEM file or cmdline
$m=''; #modulo, public from the PEM file or cmdline
$e=''; #exp, private key from cmdline

# tmp vars to override DH parameters
$sg = '';
$se = '';
$sm = '';

# tmp vars for program behavior
$pem_file=0;
$verbose=0;
$secret='';

#parse arguments
while ($arg = shift @ARGV) {
    if ($arg eq '-new') {
	$pem_file = shift @ARGV or die "no pem file given";
	`openssl dhparam -outform PEM -out $pem_file -2 2>/dev/null`
    }
    if ($arg eq '-pem') {
	$pem_file = shift @ARGV or die "no pem file given";
    } elsif ($arg eq '-g') {
	$sg = shift @ARGV or die "no generator given";
    } elsif ($arg eq '-m') {
	$sm = shift @ARGV or die "no modulo given";
    } elsif ($arg eq '-e') {
	$se = shift @ARGV or die "no private key given";
    } elsif ($arg eq '-v') {
	$verbose=1;
    } elsif ($arg eq '-s') {
	$secret = shift @ARGV or die "no private key given";
    }
}

#parse either new or existing pem file
if ($pem_file) {
    $pem_file_txt = $pem_file . ".txt";
    `openssl dhparam -inform PEM -in $pem_file -text > $pem_file_txt`;
    open FILE, $pem_file_txt or die $!;
    $m = '';
    $start=0;
# parse the ouput from openssl binary to extract generator and modulo parameters
    while (<FILE>) {
	if (!$start) {
	    if (m/prime:\s*/) {
		$start=1;
	    }
            next;
        }
	if (m/generator:\s*([0-9]+)/) {
	    $g = $1;
	    $start=0;
	    last;
	}
	if (m/\s*(([0-9a-fA-F]{2}:?)+)/) {
	    $frag = $1;
	    $frag =~ s/://g;
	    $m .= $frag;
	}
    }
    close FILE
}

#cmdline >> pem file
if ($sg) { $g = $sg; }
if ($sm) { $m = $sm; }
if ($se) { $e = $se; }

#check parameters
if (!$g) { die "missing generator"; }
if (!$m) { die "missing modulo"; }
if (!$e) { die "missing private key"; }

if ($verbose) {
    print "g=$g\n";
    print "e=$e\n";
    print "m=$m\n";
}

#dc logic is based on a perl script from http://www.cypherspace.org/rsa/perl-dh.html
$key = `echo "16dio1[d2%Sa2/d0<X+d*La1=z\U$m%0]SX$e"[$g*]\EszlXx+p|dc`;
$key =~ s/(\\\s*)//g;
if ($secret) {
    if ($secret =~ m/\s*$key\s*/ix) {
	if ($verbose) {
	    print "SHARED SECRET=\n$secret\n";
	}
    } else {
	print "computed key != given key ( $key , $secret )\n";
	exit -1;
    }
} else {
    print $key;
}
exit 0;
