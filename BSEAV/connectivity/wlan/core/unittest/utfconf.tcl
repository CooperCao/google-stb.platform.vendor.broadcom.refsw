######
#
# Site UTF configuration
#
# $Id: 76e5d9675a5864606f1e46c3ea315715799eabee $
# $Copyright Broadcom Corporation$

package provide utfconf 2.0

UTF::MacOS AppleCore -lan_ip 10.19.60.63 -user hwnsig -onall 0 -infrastructure 1

namespace eval UTF {}


set UTF::smtphost "rnd-relay.smtp.broadcom.com"
set UTF::smtpdomain "broadcom.com"

set UTF::projswbuild	/projects/hnd/swbuild
set UTF::projtools     	/projects/hnd/tools
set UTF::projgallery    /projects/hnd/software/gallery
set UTF::projarchives   /projects/hnd_archives

# Relay for testing mapping of ta921953@broadcom.com
#set UTF::smtphost "bcacpe-irv-1.irv.broadcom.com:8225"

# Use for simulating limited bandwidth
# set UTF::CopytoBW 1000

# Use rsync for copying files - disabled because it provided no benefits
# set UTF::UseRsyncforCopyto 1

# Collect TCP performance stats from the receiver, not the sender.
# set UTF::TcpReadStats 1

# Disable report compression
# set UTF::NoCompressReports 1

# Enable to push out new iperf versions
set UTF::PushNewIperf 1

set UTF::CleanCache 1
set UTF::TcpReadStats 1
set UTF::NoTCPTuning 1; # Don't tune TCP, just set core *mem_max and
			# let iperf do the rest.
set UTF::UseFCP 1; # Use fcp instead of scp

set UTF::MSTimeStamps 1; # Report MS in timestamps
set UTF::MultiperfE 1; # Enable iperf -e by default

set UTF::Use11h 1; # Enable 11h for non-radar channels
set UTF::UseCSA 1; # Enable CSA for ChannelSweep

set UTF::dBuxRegister 1

# Beta test features
switch $::tcl_platform(user) {
    "xwei" -
    "jqliu" -
    "nisar" -
    "chunyuhu" -
    "manojkrn" -
    "pcao" -
    "vyass" {
	set UTF::FBDefault 1
    }
    "tima" {
	set UTF::CountedErrors 1
	set UTF::FBDefault 1
	set UTF::MultiperfPortCycling 1; # Use port cycling
	#set UTF::BindIperf 1; # Use -B to bind iperf
	# turn airportpower on at load and off at unload
	set UTF::MacOSLoadPower 1
	set UTF::UTFStatus 1

	# Dump TX and RX seperately - no advantage at this time.
	#set UTF::DumpAMPDUTXRX 1
	set UTF::DNSDomain sj.broadcom.com
    }
    "rmcmahon" {
    }
}
