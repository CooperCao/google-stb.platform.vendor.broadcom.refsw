#
# Windows E.C node agent startup script
#
# It sets timeout value for bash shell and for every other build job 
# and also conditionally mounts Z: drive with no conflicts among
# different agent processes
#
# -Prakash [2009/06/07]
#
# $Id: ecRunAgentLocal.tcl,v 12.1 2009-06-10 07:05:05 $
#

set starttime [exec date '+%Y-%m-%d_%H:%M:%S']

puts ""
puts "========== START: $starttime =========="

set sleeptime [expr int(rand() * 10)]
set agentnum $args(-efsid)
set pass "put-hwnbuild-password-here"

puts "\[[exec date '+%Y-%m-%d_%H:%M:%S']\]"
puts "Agent Name      : $args(-myhost)"
puts "Agent Number    : $args(-efsid) of $args(-numagents)"
puts "Agent CM value  : $args(-cm)"

set commandTimeout\
    {\
         { {.*bin[/\\](ba)?sh.exe.*} 400000 {disk} }\
         { {.*} 400000 {cpu disk} }\
     }
agentexec timeout $::commandTimeout

# Implement our own sleep command and sleep for some random number of secs
# to avoid multiple 'net use' commands from conflicting
proc mysleep {N} {
   global agentnum

   puts "sleep [expr {int($N * 0.5 * $agentnum)}]"
   after [expr {int($N * 500 * $agentnum)}]
}

mysleep $sleeptime
puts "\[[exec date '+%Y-%m-%d_%H:%M:%S']\]"

if {[catch {if {![file exists Z:/]} {exec net use z: \\\\brcm-sj\\dfs $pass /user:broadcom\\hwnbuild /persistent:yes} } message]} {
	puts "Error mounting drive Z: $message"
}

if {[file exists Z:/]} {
	puts "Z: Info: [exec net use z:]"
}

if {[file exists P:/]} {
	puts "P: Info: [exec net use P:]"
}

set endtime [exec date '+%Y-%m-%d_%H:%M:%S']

puts "========== END  : $endtime =========="
puts ""
