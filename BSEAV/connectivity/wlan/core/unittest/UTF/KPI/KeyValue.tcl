#
# UTF KPI Key Value Structures
#
# Key Value pairs used for KPI results
#
# Written by: Robert J. McMahon August 2015
#
package require UTF
package require snit
package require md5
package require math::statistics

package provide UTF::KPI::KeyValue 2.0

snit::type UTF::KPI::KeyValue {
    option -name -default {} -readonly true
    option -parent -default {} -readonly true
    option -directory -readonly true -default {}
    option -exclude -default "sample samples name creation"
    option -units -default "" -readonly true
    variable container
    variable sampletype
    variable restultsdirectory
    variable utfmsgtag
    variable keyattributes

    constructor {args} {
	$self configurelist $args
	if {$options(-parent) ne {} && ([::UTF::KPI::KeyValue info instances [namespace which $options(-parent)]] \
					ne [namespace which $options(-parent)])} {
	    error "$options(-parent) not a key value object"
	}
	if {$options(-name) eq {}} {
	    set utfmsgtag [namespace tail $self]
	} else {
	    set utfmsgtag $options(-name)
	}
	if {$options(-directory) eq ""} {
	    if {[info exists ::tcl_interactive] && $::tcl_interactive} {
		set options(-directory) [file join [exec pwd] kpiresults]
	    } else {
		set options(-directory) [file join $::UTF::SummaryDir $::UTF::SessionID kpiresults]
	    }
	}
	set sampletype scalar
	set container [dict create]
	dict set container creation [clock seconds]
	set keyattributes(creation) "time epoch"
	if {$options(-units) ne ""} {
	    set keyattributes(sample) "units $options(-units)"
	}
    }
    destructor {
	catch {unset container}
    }
    method inspect {} {
	parray keyattributes
	puts "Key Values:"
	$self xml
    }
    method hash {args} {
	return [::md5::md5 -hex [concat $args [dict remove $container {*}$options(-exclude)]]]
    }
    method id {args} {
	if {$options(-parent) ne ""} {
	    return [$self hash [$options(-parent) id]]
	} else {
	    return [$self hash]
	}
    }
    method attributes {} {
	return [dict remove $container "sample"]
    }
    method samples {} {
	return [dict get $container "sample"]
    }
    method keys {} {
	return [dict keys $container]
    }
    method filter {filterkey} {
	return [dict filter $container key $filterkey]
    }
    method mycontainer {} {
	return [myvar container]
    }
    method dumpstats {} {
	return [dict info $container]
    }
    method addsample {args} {
	dict lappend container sample {*}$args
    }
    method replace {key value} {
	return [dict replace $container $key $value]
    }
    method append {key value} {
	set container [dict replace $container $key $value]
    }
    method {import array} {name} {
	upvar $name n
	if {![array exists n]} {
	    error "$name is not an array"
	}
	foreach index [array names n] {
	    dict set container $index $n($index)
	}
    }
    method {clear samples} {} {
	dict set container "sample" ""
	set sampletype "scalar"
    }
    method {setarray} {name} {
	upvar $name n
	if {![array exists n]} {
	    error "$name is not an array"
	}
	foreach index [array names n] {
	    dict set t $index $n($index)
	}
	if {[info exists t]} {
	    dict set container "sample" "[dict get $t]"
	    set sampletype "dict"
	} else {
	    dict set container "sample" ""
	    set sampletype "scalar"
	}
	return
    }
    method {appendarray} {name} {
	upvar $name n
	if {![array exists n]} {
	    error "$name is not an array"
	}
	if {[catch {dict get $container sample} inner]} {
	    $self setarray n
	} else {
	    foreach index [array names n] {
		dict lappend inner $index $n($index)
	    }
	    dict set container "sample" "[dict get $inner]"
	}
	return
    }
    method lappend {key value} {
	dict lappend container $key $value
    }
    method merge {p} {
	set container [dict merge [set [$p mycontainer]] $container]
    }
    method combine {} {
	array set keyattributes [$self combineattributes]
	if {$options(-parent) ne ""} {
	    set combined [dict merge [$options(-parent) combine] $container]
	    return $combined
	} else {
	    return $container
	}
    }
    method combineattributes {} {
	set final {}
	if {$options(-parent) ne ""} {
	    set final [concat [$options(-parent) combineattributes] [array get keyattributes]]
	    return $final
	} else {
	    set final [array get keyattributes]
	    return $final
	}
    }
    method get {} {
	return [dict get $container]
    }
    method set {args} {
	UTF::GetKnownopts {
	    {exclude "Exclude key(s) from ID hash"}
	}
	foreach {key val} $args {
	    if {[set ix [lsearch $key "-attribute"]] ne "-1"} {
		set attributes [lrange $key [expr {$ix+1}] end]
		set key [lrange $key 0 [expr {$ix - 1}]]
		foreach attrib  $attributes {
		    foreach {a b} [split $attrib =] {}
		    dict set keyattributes($key) $a $b
		}
	    }
	    dict set container $key $val
	    if {$(exclude) && [lsearch $options(-exclude) $key] eq "-1"} {
		lappend options(-exclude) $key
	    }
	}
    }
    # Post to the database
    method post {} {
    }
    # Write to the file system
    method write {} {
	if {![info exists resultsdirectory]} {
	    if {![file exists $options(-directory)] && [catch {file mkdir $options(-directory)}]} {
		error "could not create results directory $options(-directory)"
	    }
	    file attributes $options(-directory) -permissions 02777
	    set resultsdirectory $options(-directory)
	}
	while {1} {
	    set filename [file join $options(-directory) [clock clicks -milliseconds].kpi]
	    if  {[file exists $filename]} {
		UTF::Sleep 0.01 quiet
	    } else {
		break
	    }
	}
	set fid [open $filename w]
	puts -nonewline $fid [$self xml]
	close $fid
	UTF::_Message KPI WRITE "$utfmsgtag : $filename"
	$self log
    }
    method xml {args} {
	set xml "<?xml version=\"1.0\"?>\n"
	append xml "<kpiresult>\n"
	append xml "\t<selfID>[$self id]</selfID>\n"
	foreach {key val} [$self combine] {
	    set key [string map {< "&lt;" > "&gt;" & "&amp;" " " _} $key]
	    set val [string map {< "&lt;" > "&gt;"} $val]
	    set tmp "\t<$key "
	    if {[info exists keyattributes($key)]} {
		foreach {attribkey attribval} $keyattributes($key) {
		    append tmp "[join "$attribkey \\\"$attribval\\\"" =] "
		}
	    }
	    append xml "[string trimright $tmp]>"
	    if {$key eq "sample" && $sampletype eq "dict"} {
		append xml "\n"
		foreach {nkey nestedval} $val {
		    set nkey [string map {< "&lt;" > "&gt;" & "&amp;" " " _} $nkey]
		    set nestedval [string map {< "&lt;" > "&gt;"} $nestedval]
		    append xml "\t\t<$nkey>$nestedval</$nkey>\n"
		}
		append xml "\t"
	    } else {
		append xml "$val"
	    }
	    append xml "</$key>\n"
	}
	append xml "\t<write_time>[clock format [clock seconds]]</write_time>\n"
	append xml "</kpiresult>\n"
	return $xml
    }
    method json {} {
	package require json
	return [json::dict2json [$self combine]]
    }
    method print {} {
	puts "selfID=[$self id]"
	foreach {key val} [$self combine] {
	    puts "$key=$val"
	}
    }
    method log {} {
	set msg "selfID=[$self id] "
	foreach {key val} [$self combine] {
	    if {[llength $val] > 1} {
		append msg "$key=\{$val\} "
	    } else {
		append msg "$key=$val "
	    }
	}
	UTF::_Message KPI $utfmsgtag [string trim $msg]
    }
}