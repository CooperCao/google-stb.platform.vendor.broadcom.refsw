#!/bin/env utf

#
# UTF Framework Object Definitions
# Based on snit
# $Id$
# $Copyright Broadcom Corporation$
#

package provide UTF::WebReport 2.0

package require snit
package require UTF::doc
package require struct::tree

UTF::doc {
    # [manpage_begin UTF::WebReport n 1.0]
    # [moddesc {HND Wireless Test Framework}]
    # [titledesc {UTF WebReport}]
    # [copyright {2008 Broadcom Corporation}]
    # [require UTF::WebReport]
    # [description]
    # [para]

    # The UTF::WebReport object is used to create and update Web-based
    # reports for UTF tests.

    # [list_begin definitions]
}

snit::type UTF::WebReport {

    UTF::doc {
	# [call [cmd UTF::WebReport] [arg name]
	#	[option -summaryfile] [arg name]
	#       [option -failfile] [arg name]
	#       [lb][option -title] [arg title][rb]
	#       [lb][option -info] [arg info][rb]]

	# Create a new WebReport.  The report actually consists of two
	# html files, one containing the full report and the second
	# containing a concise fail report suitable for emailing.

	# [list_begin options]

	# [opt_def [option -summaryfile] [arg name]]

        # Specify the full path of the summary file.  Required.

	# [opt_def [option -failfile] [arg name]]

        # Specify the full path of the file file.  Required.

	# [opt_def [option -title] [arg title]]

        # Specify the title of the report.  Defaults to "Untitled"

	# [opt_def [option -info] [arg info]]

	# Specify additional information to br printed at the top of
	# the report.  The header should be an HTML fragment.

	# [list_end]
    }

    # Allow new instances to replace old instances, eg when rereading
    # config files.
    pragma -canreplace yes

    option -summaryfile
    option -failfile
    option -title    "Untitled"
    option -info ""

    variable base
    variable head ""
    variable body ""
    variable tree
    variable fail {}
    variable tail ""
    variable passcount 0
    variable failcount  0
    variable elapsed "0:00"
    variable status "Running"
    variable hidebuttons -array {}

    variable javascript {

	<style type="text/css">
	a img{border: none}
	</style>

	<script type="text/javascript">
	function ShowOrHide(id) {
	    if (document.getElementById) { // DOM3 = IE6, NS6
		el=document.getElementById(id);
		if (el.style.display == 'none') {
		    el.style.display = '';
		} else {
		    el.style.display = 'none';
		}
	    } else {
		if (document.layers) { // Netscape 4
		    if (document.id.display == 'none') {
			document.id.display = '';
		    } else {
			document.id.display = 'none';
		    }
		} else { // IE 4
		    if (document.all.id.style.display == 'none') {
			document.all.id.style.display = '';
		    } else {
			document.all.id.style.display = 'none';
		    }
		}
	    }
        }
	function ShowOrHideAll(tag, c) {
	    var els=document.getElementsByTagName(tag);
	    for (var i = 0; i < els.length; i++) {
		// getAttribute doesn't seem to work on IE
		var cnode = els[i].getAttributeNode("class");
		if (cnode && cnode.value == c) {
		    if (els[i].style.display == 'none') {
			els[i].style.display = '';
		    } else {
			els[i].style.display = 'none';
		    }
		}
	    }
	}
	</script>
    }

    constructor {args} {
	$self configurelist $args

	if {$options(-summaryfile) eq ""} {
	    error "WebReport: missing required option -summaryfile"
	}
	if {$options(-failfile) eq ""} {
	    error "WebReport: missing required option -failfile"
	}
	set base [file join [file normalize $options(-summaryfile)]]
	# URL fix-up if someone is using their home dir
	regsub {^/home/} $base {/~} base

	set base "http://$::UTF::WebServer$base"

	set tree [::struct::tree]
    }

    UTF::doc {
	# [call [arg name] [method header] [arg {args ...}]]

	# Add additional information to the report header.  The
	# information will be formatted as a new table row, with each
	# argument being a seperate column.
    }

    method header {args} {
	lappend head $args
    }

    UTF::doc {
	# [call [arg name] [method header_update] [arg col] [arg value]]

	# Replace an entry in the numbered column of the last header
	# line to the supplied value.  This can be used to implement
	# headers containing information that was not available at the
	# time the header line was created.  This is a temporary API
	# and should eventually be replaced with a table accessible by
	# named fields.
    }

    method header_update {col value} {
	set head [lreplace $head end end \
		      [lreplace [lindex $head end] $col $col $value]]
    }

    method header_table {s} {
	if {$head ne ""} {
	    puts $s "<table>"
	    foreach line $head {
		puts $s "<tr>"
		foreach entry $line {
		    puts $s [subst {<td style="white-space: nowrap">$entry</td>}]
		}
		puts $s "</tr>";
	    }
	    puts $s "</table>"
	}
    }

    UTF::doc {
	# [call [arg name] [method puts] [arg msg]]

	# Add additional information to the full report body table.
	# [arg msg] is applied directly and needs to be specially
	# formatted.  See [cmd UTF::Record] below.  [arg msg] is not
	# added to the fail report.
    }

    method puts {msg} {
	append body "$msg\n"
    }

    UTF::doc {
	# [call [arg name] [method fail] [arg line]]

	# Add additional information to the fail report.  [arg line]
	# should be simple text and will be used in a bullet list.
	# [arg line] is not added to the full report.
    }

    method fail {line} {
	lappend fail $line
    }

    UTF::doc {
	# [call [arg name] [method tputs] [arg msg]]

	# Add additional information to the report trailer.  [arg msg]
	# is not added to the fail report.
    }

    method tputs {msg} {
	append tail "$msg\n"
    }

    method {tree add} {test msg file expand} {
	if {![regsub {\.\d+$} $test {} parent]} {
	    set parent "root"
	}
	set node [$tree insert $parent end $test]
	$tree set $node msg $msg
	$tree set $node state "Running..."
	$tree set $node ret ""
	$tree set $node file $file
	$tree set $node elapsed ""
	$tree set $node expand $expand
    }

    method {tree update} {test key value} {
	$tree set $test $key $value
    }

    UTF::doc {
	# [call [arg name] [method flush]]

	# Write both reports to disk.  The reports are actually
	# written to temporary files, then moved atomically into place
	# so that an error will not cause loss of the report.
    }

    method flush {} {

	if {[info exists ::UTF::_start_time]} {
	    set totalelapsed [UTF::ElapsedTime $::UTF::_start_time]
	}

	# Summary
	set s [open $options(-summaryfile).tmp w]
	puts $s "<!DOCTYPE html>"
	puts $s "<html>"
	puts $s "<head>"
	puts $s "<title>$options(-title)</title>"
	puts $s "<base href=\"$base\" />"
	puts $s $javascript
	puts $s "</head>"
	puts $s "<body><h1>$options(-title)</h1>"
	puts $s "<p>PASS: $passcount<br />FAIL: $failcount<br />"
	foreach b [array names hidebuttons] {
	    puts $s [subst {<button onclick="ShowOrHideAll('span', '$b')">Show $b</button>}]
	}
	puts $s "<button onclick=\"ShowOrHideAll('td', 'time')\">TIME</button> $totalelapsed <span id=\"runstatus\">$status</span></p>"
	puts $s {
	    <table style="width:100%"><tr>
	    <td><a href="fail.html">Failure Report</a></td>
	    <td><a href="summary.html">This Report</a></td>
	    <td><a href="..">Latest Report</a></td>
	    <td><a href="../new.html">Running Report</a></td>
	    </tr></table>
	}
	if {$options(-info) ne ""} {
	    puts $s $options(-info)
	}

	$self header_table $s

	puts $s {<table style="border:0"><tbody style="valign:top">}
	puts $s "<!--body start-->"

	$tree walk root node {
	    if {$node ne "root"} {
		set msg [$tree set $node msg]
		set state [$tree set $node state]
		set file [$tree set $node file]
		set ret [$tree set $node ret]
		set elapsed [$tree set $node elapsed]
		set depth [$tree depth $node]
		if {$depth > 1} {
		    set pad "padding-left: [expr {$depth-1}]em"
		    set p [$tree parent $node]
		    set d " class=\"t$p\""
		    if {[$tree set $p elapsed] ne "" &&
			![$tree set $p expand]} {
			# Hide children only if parent has completed
			# and does not have expand set.
			append d " style=\"display: none\""
		    }
		} else {
		    set d ""
		    set pad ""
		}
		puts -nonewline \
		    $s "<tr$d><th style=\"text-align:left;$pad\">$node"
		if {[$tree numchildren $node] > 0} {
		    puts $s "&nbsp;<span onclick=\"ShowOrHideAll('tr', 't$node')\" style=\"color: blue\">&plusmn;</span>"
		}
		puts $s "</th><td style=\"$pad\">$msg</td>"
		puts $s "<td style=\"$pad\"><a href=\"$file\">$state</a></td>"
		puts $s "<td style=\"$pad\">$ret</td>"
		puts $s "<td class=\"time\" style=\"display: none; $pad\">$elapsed</td>"
		puts $s "</tr>"
	    }
	}
	puts $s "<!--body end-->"
	puts $s {<tr><td colspan="5"><a href="test.log">test.log</a></td></tr>}
	puts $s "</tbody></table>"
	puts $s $tail
	puts $s "</body></html>"
	if {[catch {close $s} ret]} {
	    error "write failed on $options(-summaryfile).tmp"
	}
	file rename -force $options(-summaryfile).tmp $options(-summaryfile)

	# fail
	set s [open $options(-failfile).tmp w]
	puts $s "<!DOCTYPE html>"
	puts $s "<html>"
	puts $s "<head>"
	puts $s "<title>$options(-title)</title>"
	puts $s "<base href=\"$base\" />"
	puts $s "</head>"
	puts $s "<body><h1>$options(-title)</h1>"
	puts $s "<p>PASS: $passcount<br />FAIL: $failcount<br />"
	puts $s "TIME: $totalelapsed <span id=\"runstatus\">$status</span></p>"
	puts $s {
	    <p><a href="summary.html">Complete Report</a><br />
	    <a href="..">Latest Report</a><br />
	    <a href="../new.html">Running Report</a></p>
	}
	if {$options(-info) ne ""} {
	    puts $s $options(-info)
	}

	$self header_table $s

	if {[llength $fail]} {
	    puts $s "<ul>"
	    foreach row $fail {
		puts $s "<li>$row</li>"
	    }
	    puts $s "</ul>"
	}
	puts $s "</body></html>"
	if {[catch {close $s} ret]} {
	    error "write failed on $options(-failfile).tmp"
	}
	file rename -force $options(-failfile).tmp $options(-failfile)
    }

    UTF::doc {
	# [call [arg name] [method incr] [arg var] [lb][arg val][rb]]

	# Increment counter with [arg val].  [arg val] defaults to 1.

	# Valid counters are [arg passcount] and [arg failcount].
    }

    method incr {var {val 1}} {
	incr $var $val
    }

    UTF::doc {
	# [call [arg name] [method passcount]]

	# Return pass count.
    }

    method passcount {} {
	return $passcount
    }

    UTF::doc {
	# [call [arg name] [method failcount]]

	# Return fail count.
    }

    method failcount {} {
	return $failcount
    }

    UTF::doc {
	# [call [arg name] [method failreport]]

	# Return the fail data stucture
    }
    method failreport {} {
	return $fail
    }

    UTF::doc {
	# [call [arg name] [method complete]]

	# Mark report as completed.
    }

    method complete {} {
	# Clears running flag
	set status ""
    }

    method base {} {
	return $base
    }

    method spanhide {class txt} {
	set hidebuttons($class) 1
	subst {<span class="$class" style="display: none">$txt</span>}
    }

    typemethod htaccess {dir} {
	set f [open "$dir/.htaccess" w]
	puts $f {# Serve pre-compressed content
Options +Multiviews
<FilesMatch \.log\.gz$>
   AddEncoding gzip gz
   ForceType text/plain
</FilesMatch>
<FilesMatch \.html\.gz$>
   AddEncoding gzip gz
   ForceType text/html
</FilesMatch>
AddType text/csv csv
}
	close $f
    }
}

# Core UTF procs.
namespace eval UTF {}

proc UTF::StartSummary {logdir TITLE {info ""}} {

    set parent [file dirname $logdir]
    if {![file isdirectory $parent]} {
	error "Log dir $parent does not exist"
    }

    set parentdf [exec df -P $parent]

    if {![regexp {(\d+)\s+\d+%} $parentdf - avail]} {
	if {[regexp -line {^-} $parentdf]} {
	    UTF::Message WARN "" "$parent: not mounted"
	    UTF::Message WARN "" "ls $parent\n[exec ls $parent]"
	}
	set parentdf [exec df -P $parent]
	if {![regexp {(\d+)\s+\d+%} $parentdf - avail]} {
	    error "$parent: unexpected df output: $parentdf"
	}
	UTF::Message WARN "" "$parent: recovered"
	set ::UTF::panic "$parent: automount problem - recovered"
    }
    # Avail is in KB.  Abort immediately if less than 100MB free
    if {$avail < 100000} {
	return -code error "$parent: file system full:\n[exec df -H $parent]"
    }

    if {![regexp {^/home/} $parent]} {
	# Don't touch home dir permissions.
	catch {file attributes $parent -permissions g+w}
    }

    file mkdir $logdir
    catch {file attributes $logdir -permissions g+w}

    set ::UTF::Logdir [file join $logdir $::UTF::SessionID]
    # TCL mkdir can't detect collisions, use shell mkdir instead.
    for {set dir $::UTF::Logdir; set id $::UTF::SessionID; set i 0} {[catch {exec mkdir $::UTF::Logdir} ret]} {incr i} {
	if {![regexp {File exists} $ret]} {
	    error $ret $::errorInfo
	}
	set ::UTF::Logdir "$dir.$i"
	set ::UTF::SessionID "$id.$i"
    }
    catch {file attributes $::UTF::Logdir -permissions g+w}
    if {[catch {exec ln -fs [file join $UTF::SessionID summary.html] \
		    [file join $logdir new.html]} ret]} {
	# We can't make this atomic, and collisions will be resolved
	# at the next run so ignore them.
	if {![regexp {File exists} $ret]} {
	    error $ret $::errorInfo
	} else {
	    UTF::Message WARN "" $ret
	}
    }

    UTF::Logfile [file join $::UTF::Logdir "test.log"]
    # Log top level shell commandline arguments
    UTF::Message INFO "" $::__cmdline

    # Convert into HTML if necessary
    if {$info ne "" & ![regexp "^<" $info]} {
	set info "<p>[join $info <br/>]</p>"
    }

    set ::UTF::Summary [UTF::WebReport %AUTO% \
			    -title $TITLE \
			    -info $info \
			    -summaryfile \
			    [file join $::UTF::Logdir "summary.html"] \
			    -failfile [file join $::UTF::Logdir "fail.html"]]

    UTF::Message INFO "URL" [UTF::LogURL summary.html]
    catch {exec hostname} hostname
    UTF::Message INFO "Controller" $hostname
    UTF::Message INFO "UTF" $::UTF::unittest
    UTF::Message INFO "TCL" $::tcl_version

    set s $UTF::Summary
    $s flush

    set ::UTF::RecordStack 0

    # For consistency, ensure ::UTF::trailer_info is defined.
    if {![info exists UTF::trailer_info]} {
        set ::UTF::trailer_info ""
    }

}

UTF::doc {
    # [call [cmd UTF::LogURL] [lb][arg file][rb]]

    # Returns a URL for accessing the current UTF::Logfile via a web
    # server, or a different file from the same report if the optional
    # [arg file] is specified.  Calling outside of a WebReport context
    # is an error.
}

proc UTF::LogURL {{f ""}} {
    if {![info exists UTF::Summary]} {
	error "Not running in WebReport context"
    }
    if {$f eq ""} {
	set f $UTF::Logfile
    }
    return "[regsub {/[^/]*$} [$UTF::Summary base] {}]/[file tail $f]"
}

proc UTF::Record {msg block {finally {}} {fblock {}}} {
    if {($finally eq {}) ? $fblock ne {} : $finally ne {finally}} {
	error "usage UTF::Record msg {...} ?finally {...}?"
    }

    if {[regsub {^-} $msg {} msg]} {
	set failinfo 1
    } else {
	set failinfo 0
    }
    if {[regsub {^\+} $msg {} msg]} {
	set expand 1
    } else {
	set expand 0
    }

    if {[info exists ::UTF::__tryid]} {
	package require md5
	set ::UTF::__tryid [::md5::md5 -hex [concat "$msg $block"]]
    }

    # push TryStack
    lappend ::UTF::TryStack "NONE"

    if {[info exists ::UTF::RecordStack]} {
	# Web report start
	set index [lindex $UTF::RecordStack end]
	incr index
	set UTF::RecordStack [lreplace $UTF::RecordStack end end $index]
	set _Logfile $UTF::Logfile

	set fullindex [join $UTF::RecordStack "."]

	lappend UTF::RecordStack 0
	UTF::Message INFO $fullindex $msg
	UTF::Logfile [file join $UTF::Logdir "$fullindex.log"]
	# html encode message
	set msg [string map {< "&lt;" > "&gt;" & "&amp;"} $msg]
	if {[info exists ::UTF::__tryid]} {
	    append msg [$UTF::Summary spanhide "ID" "<br />$::UTF::__tryid"]
	}
	$UTF::Summary tree add $fullindex $msg [file tail $UTF::Logfile] $expand

	$UTF::Summary flush
    } else {
	# Text report
	UTF::Message INFO "" $msg
    }
    set start [clock seconds]
    # Execute block
    if {$fblock ne {}} {
	set code [catch {uplevel 1 [list try $block finally $fblock]} ret]
    } else {
	set code [catch {uplevel 1 $block} ret]
    }
    set errorCode $::errorCode

    set elapsed [UTF::ElapsedTime $start]

    if {[info exists ::UTF::warn]} {
	# Background warnings append to regular results
	append ret " $::UTF::warn"
	unset ::UTF::warn
    }
    if {[info exists ::UTF::exit]} {
	# Exit propagates
	set code 1
	set errorCode EXIT$code
	set errorInfo "exit $::UTF::exit"
	set ret "exit $::UTF::exit"
    } elseif {[info exists ::UTF::panic]} {
	# Background panic overrides regular results
	# but an existing error should be reported anyway
	if {$code == 1} {
	    if {$::errorCode eq "FAIL"} {
		UTF::Message FAIL "" $ret
	    } else {
		UTF::Message FAIL "" $::errorInfo
	    }
	}
	set code 1
	set errorInfo $::UTF::panic
	set ret $::UTF::panic
	unset ::UTF::panic
    } elseif {$code == 1 || [lindex $UTF::TryStack end] eq "NONE"} {
	if {$errorCode eq "FAIL"} {
	    # Simple FAIL result, no need for stack trace
	    set errorInfo $ret
	} else {
	    # Unidentified error, use global errorInfo so we get a
	    # stack trace
	    set errorInfo $::errorInfo
	}
    } else {
	# caught error, use try stack
	set code 1
	set errorInfo [lindex $UTF::TryStack end]
    }

    if {[info exists fullindex]} {
	# Web report end
	if {$code == 1} {
	    # FAIL
	    set ret $errorInfo
	    set shortret [lindex [split $ret "\n"] 0]
	    if {[regexp {^html:(.*)} $shortret - shortret]} {
		# strip HTML tags from fail report version
		regsub -all {<[^>]*>} $shortret {} failret
		regsub -all {<[^>]*>} $ret {} ret
	    } elseif {[regexp {^link:(.*)} $shortret - shortret]} {
		if {[regexp {^([A-Z]+)-(\d+)$} $shortret - p n]} {
		    # Jira
		    set shortret \
			"<a href=\"http://jira.broadcom.com/browse/$p-$n\">$p\&#8209;$n</a>"
		} else {
		    set shortret "<a href=\"[UTF::URI $shortret]\">$shortret</a>"
		}
		set failret $shortret
	    } else {
		set shortret [string map {< "&lt;" > "&gt;" & "&amp;"} \
				  $shortret]
		set failret $shortret
	    }
	    UTF::Message FAIL $fullindex "$msg: $ret"

	    $UTF::Summary tree update $fullindex state "*FAIL*"

	    if {[llength $UTF::RecordStack] < 3} {
		# toplevel
		$UTF::Summary incr failcount
	    }
	    $UTF::Summary fail "$fullindex $msg: $failret"
	} else {
	    # PASS
	    set shortret [lindex [split $ret "\n"] 0]
	    if {[regexp {^html:(.*)} $shortret - shortret]} {
		# strip HTML tags from fail report version
		regsub -all {<[^>]*>} $shortret {} failret
	    } elseif {[regexp {^link:(.*)} $shortret - shortret]} {
		if {[regexp {^([A-Z]+)-(\d+)$} $shortret - p n]} {
		    # Jira
		    set shortret \
			"<a href=\"http://jira.broadcom.com/browse/$p-$n\">$p\&#8209;$n</a>"
		} else {
		    set shortret "<a href=\"[UTF::URI $shortret]\">$shortret</a>"
		}
		set failret $shortret
	    } else {
		set shortret [string map {< "&lt;" > "&gt;" & "&amp;"} \
				  $shortret]
		set failret $shortret
	    }
	    UTF::Message PASS $fullindex "$msg: $failret"

	    $UTF::Summary tree update $fullindex state "PASS"

	    if {[llength $UTF::RecordStack] < 3} {
		# toplevel
		$UTF::Summary incr passcount
	    }
	    if {$failinfo} {
		$UTF::Summary fail "$fullindex $msg: $shortret"
	    }

	}

	$UTF::Summary tree update $fullindex ret $shortret
	$UTF::Summary tree update $fullindex elapsed $elapsed
	$UTF::Summary flush

	UTF::Logfile $_Logfile
	set UTF::RecordStack [lreplace $UTF::RecordStack end end]
    }

    # pop TryStack
    set UTF::TryStack [lreplace $UTF::TryStack end end]
    if {$code == 1} {
	error $ret $errorInfo $errorCode
    } else {
	return $ret
    }
}

proc UTF::EndSummary {logdir TITLE EMAIL} {

    # clear Running flag
    $UTF::Summary complete

    # Update published URL
    if {[catch {file copy -force [file join $logdir new.html] \
		    [file join $logdir index.html]} ret]} {
	# We can't make this atomic, and collisions will be resolved
	# at the next run so ignore them.
	if {![regexp {no such file} $ret]} {
	    error $ret $::errorInfo
	} else {
	    UTF::Message WARN "" $ret
	}
    }

    if {[info exists UTF::trailer_info]} {
        # The test script may have some additional information that is
        # desirable to show at the end of the summary page. In order to
        # allow the script to update the trailer info as the script is
        # executing, the trailer info is NOT passed as an argument to
        # the main routine UTF::WrapSummary. The final version of the
        # trailer info, if any, is appended after all the tests are done.
        $UTF::Summary tputs "$UTF::trailer_info"
        # Reset the trailer info in case of nesting.
        set UTF::trailer_info ""
    }
    if {[info exists UTF::CleanCache]} {
	UTF::CleanCache
    }
    $UTF::Summary flush

    set TITLE [$UTF::Summary cget -title]
    set failcount [$UTF::Summary failcount]

    if {$failcount == 1} {
	append TITLE " ($failcount failure)"
    } elseif {$failcount > 1} {
	append TITLE " ($failcount failures)"
    }
    try {
	if {[info exists UTF::DBid]} {
	    UTF::TestResults [$UTF::Summary passcount] $failcount \
		[UTF::ElapsedTime $::UTF::_start_time]
	}
	if {[info exists UTF::dBid]} {
	    UTF::dBuxResults [$UTF::Summary passcount] $failcount
	}
	if {[info exists ::env(UTFDPORT)]} {
	    UTFD::PostTestResults $UTF::Summary
	}
    } finally {
	# Flush main log
	UTF::Logfile ""
	if {$EMAIL ne "none"} {
	    UTF::EmailFile $EMAIL $TITLE [$UTF::Summary cget -failfile]
	}
    }

    # Write htaccess file
    UTF::WebReport htaccess $UTF::Logdir
    if {![info exists ::UTF::NoCompressReports]} {
	# compress report
	UTF::Message LOG "" "gzip -9 *.log"
	exec gzip -9 {*}[glob -nocomplain $UTF::Logdir/*.log]
    }

    return $failcount
}

UTF::doc {
    # [call [cmd UTF::CleanCache]]

    # Delete cache files in UTF's perfcache if they have not been
    # updated in 90 days.
}

proc UTF::CleanCache {args} {
    UTF::Getopts {
	{force "Clean immediately, even if already cleaned today."}
    }

    set perfcache [file join $UTF::SummaryDir perfcache]
    if {![file isdirectory $perfcache]} {
	# No cache to clean
	return
    }
    set days 90; # clean up cache files older than this
    set now [clock seconds]
    set old [expr {$now - ($days * 86400)}]

    # Use timestamp so we only clean once per day
    if {!$(force) &&
	![catch {file stat [file join $perfcache .cleaned] s}] &&
	$s(mtime) > ($now - 86400)} {
	return
    }

    UTF::Message INFO "" "Cleaning perfcache..."

    set pwd [pwd]
    if {[catch {
	cd $perfcache
	set files [glob "*.data"]
	set stale 0
	set count 0
	foreach i $files {
	    incr count
	    file stat $i s
	    if {$s(atime) < $old} {
		incr stale
		UTF::Message LOG "" \
		    "Delete $i: [clock format $s(atime)] < [clock format $old]"
		file delete -- $i
	    }
	}
	UTF::Message LOG "" "$stale/$count files deleted"

	# Update timestamp
	set stamp [open .cleaned w]
	puts $stamp $now
	close $stamp
    } ret]} {
	UTF::Message WARN "" $ret
    }
    cd $pwd
    return
}

UTF::doc {
    # [call [cmd UTF::RecoverEndSummary] [arg dir] [lb][arg email][rb]]

    # Finalizes a web summary report and emails failure report.  Used
    # to recover in case a test was aborted due to a major failure.
    # [arg dir] is the path name of the directory containing the web
    # report (including timestamp).  [arg email] is the email address
    # to send the fail report (defaults to self).
}

proc UTF::RecoverEndSummary {DIR {EMAIL ""}} {
    set logdir [file dirname $DIR]
    set TITLE "Unknown recovered report"

    set summary [file join $DIR "summary.html"]
    set fail [file join $DIR "fail.html"]

    set s [open $summary]
    set body [read $s]
    close $s
    regexp {<title>(.*)</title>} $body - TITLE
    regsub {<span id="runstatus">Running</span>} $body \
	{<span id="runstatus">Aborted</span>} body
    set s [open $summary w]
    puts $s $body
    close $s

    set s [open $fail]
    set body [read $s]
    close $s
    regsub {<span id="runstatus">Running</span>} $body \
	{<span id="runstatus">Aborted</span>} body
    set s [open $fail w]
    puts $s $body
    close $s

    if {$EMAIL ne "none"} {
	UTF::EmailFile $EMAIL $TITLE $fail
    }
}

# Retrieve manpage from last object
UTF::doc [UTF::WebReport man]

UTF::doc {
    # [list_end]

    # [section WARNING]
    # [para]
    # This API is experimental and is changing rapidly.
    # [see_also wl]
    # [see_also [uri APdoc.cgi?UTF.tcl UTF]]
    # [see_also [uri APdoc.cgi?UTF::Base.tcl UTF::Base]]
    # [see_also [uri APdoc.cgi?UTF::Cygwin.tcl UTF::Cygwin]]
    # [see_also [uri APdoc.cgi?UTF::Router.tcl UTF::Router]]
    # [manpage_end]
}

# skip rest if being sourced
if {![info exists argv0] || ![string match [info script] $argv0]} {
    return
}

# main
UTF::man
