set ::UTFD::OSVER_CANDIDATE 15B30
set ::UTFD::OSVER_REFERENCE 15A284
set ::AP "4708ap1"
set ::SECONDAP "4360ap2"
set ::maxruntime 6.0
set ::stressor "::43602lx2"

# CLONES MacBook-A clone MacBook-$::UTFD::OSVER_REFERENCExb
MacBook-B clone MacBook-$::UTFD::OSVER_REFERENCE -tag "BIS715GALA_REL_7_21_94_25" -custom 1
MacBook-$::UTFD::OSVER_REFERENCE configure -attngrp L4

MacBook-A clone MacBook-$::UTFD::OSVER_CANDIDATE -name MacBook-$::UTFD::OSVER_CANDIDATE -tag "BIS715GALA_REL_7_21_104"   -custom 1
MacBook-$::UTFD::OSVER_CANDIDATE configure -attngrp L4

MacAir-B clone MacAir-${::UTFD::OSVER_REFERENCE} -name MacAir-$::UTFD::OSVER_REFERENCE -tag "BIS715GALA_REL_7_21_94_25" -custom 1
MacAir-${::UTFD::OSVER_REFERENCE} configure -attngrp L5

MacAir-A clone MacAir-${::UTFD::OSVER_CANDIDATE} -name MacAir-$::UTFD::OSVER_CANDIDATE -tag "BIS715GALA_REL_7_21_104" -custom 1
MacAir-${::UTFD::OSVER_CANDIDATE} configure -attngrp L5

set ::UTFD::MACDUTS {}
foreach STA [UTF::STA info instances] {
    if {![$STA hostis MacOS] || $STA eq "::AppleCore" || $STA eq $stressor || ([lsearch $::UTFD::physicalmacstas $STA] ne "-1")} {
	continue
    } else {
	lappend ::UTFD::MACDUTS $STA
    }
}

set ::UTFD::LINUXDUTS "43602lx1 43602lx2"

#  METASCRIPTS

proc ::restart_kpi_scripts {} {
    set distances {50 35 25}
    set thisutfconf [file rootname [file tail $::utfconf]]
    UTF::Message INFO "" "Instantiating metascripts for rig $thisutfconf"
    UTF::Message INFO "" "MacDUTs are $::UTFD::MACDUTS"
    set macsecurities {open aespsk2}
#    set macsecurities {open}
    set email {hnd-utf-list rmcmahon}
    set suitescript "/home/rmcmahon/UTF/svn/unittest/Test/KPI/KPISuite.test -utfconf $thisutfconf -suiteid meta"
    UTFD::destroyall
    catch {awdlscripts 5}
    foreach STA $::UTFD::MACDUTS {
	set thisscript [list {*}$suitescript -email $email -sta $STA -ap "$::AP $::SECONDAP" -distance $distances -security $macsecurities -title "$STA w/$::AP and $::SECONDAP" -stressor $::stressor -noapload]
	UTFD::metascript %AUTO% -type background  -maxruntime $::maxruntime -script $thisscript
    }
    foreach STA $::UTFD::MACDUTS {
	set thisscript [list {*}$suitescript -email $email -sta $STA -ap "$::AP" -distance $distances -security $macsecurities -title "$STA w/$::AP" -chanspec 6 -stressor $::stressor -noapload]
	UTFD::metascript %AUTO% -type background  -maxruntime $::maxruntime -script $thisscript
    }
    if {[info exists linuxdhdstas]} {
	foreach STA $linuxdhdstas {
	    set thisscript [list {*}$suitescript  -email $email -sta $STA -ap $::AP -distance $distances -security $dhdsecurities -title "$STA w/$::AP"]
	    UTFD::metascript %AUTO% -type background -script $thisscript
	}
    }
    foreach STA $::UTFD::LINUXDUTS {
	set thisscript [list {*}$suitescript -email $email -sta $STA -ap "$::AP $::SECONDAP" -distance $distances -security $macsecurities -title "$STA w/$::AP and $::SECONDAP" -noapload]
	UTFD::metascript %AUTO% -type background  -maxruntime $::maxruntime -script $thisscript
    }
    UTF::_Message CTRLR "" "Started [llength [UTFD::metascript info instances]] scripts"
}
