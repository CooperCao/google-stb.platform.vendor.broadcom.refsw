#!/bin/env utf
# -*-tcl-*-

#
# UTF Object for collecting and reporting AMPDU/AMSDU statistics
# $Id$
#

package require snit
package require UTF::doc
package require UTF::Heatmap


package provide UTF::MpduStats 2.0

snit::type UTF::MpduStats {

    typemethod init {args} {
	[UTF::MpduStats %AUTO%] init {*}$args
    }

    # AMPDU
    variable ampduhost
    variable ampdustats

    variable vhtheatmap
    variable mcsheatmap
    variable vhtperheatmap
    variable mcsperheatmap
    variable ampduheatmap
    variable fbheatmap

    # AMSDU
    variable amsduhost
    variable amsdustat

    variable sduheatmap

    variable new_clear_ampdu 0
    variable new_clear_amsdu 0

    method has_ampdu_dumps {S} {
	if {[catch {$S wl -silent dump ampdu} ret]} {
	    # No dump command
	    return 0
	}
	if {![regexp {AMPDU} $ret]} {
	    # Dump did not contain valid data
	    return 0
	}
	if {![catch {$S wl dump_clear ampdu}]} {
	    # New style clear
	    set new_clear_ampdu 1
	    return 1
	}
	if {![catch {$S wl ampdu_clear_dump}]} {
	    # Old style clear
	    set new_clear_ampdu 0
	    return 1
	}
	return 0
    }

    method has_amsdu_dumps {S} {
	if {[catch {$S wl -silent dump amsdu} ret]} {
	    # No dump command
	    return 0
	}
	if {![regexp {AMSDU} $ret]} {
	    # Dump did not contain valid data
	    return 0
	}
	if {![catch {$S wl dump_clear amsdu}]} {
	    # New style clear
	    set new_clear_amsdu 1
	    return 1
	}
	if {![catch {$S wl amsdu_clear_counters}]} {
	    # Old style clear
	    set new_clear_amsdu 0
	    return 1
	}
	return 0
    }

    method init {SRC SNK file {dir 0}} {
	#set ratetype ALL
	# AMPDU
	if {[$self has_ampdu_dumps $SRC]} {
	    set ampdustats "tx"
	    set ampduhost $SRC
	    set vhtheatmap \
		[UTF::Heatmap %AUTO% -file ${file}_vht.csv \
		     -title "Rate" -dir $dir -yoff 0]
	    set mcsheatmap \
		[UTF::Heatmap %AUTO% -file ${file}_mcs.csv \
		     -title "Rate" -dir $dir -yoff 0]
	    set vhtperheatmap \
		[UTF::Heatmap %AUTO% -file ${file}_vhtper.csv \
		     -title "PER" -dir $dir -yoff 0 -pad -1 \
		     -palette {-1 "black", 0 "white", 50 "yellow", 100 "red"}]
	    set mcsperheatmap \
		[UTF::Heatmap %AUTO% -file ${file}_mcsper.csv \
		     -title "PER" -dir $dir -yoff 0 -pad -1 \
		     -palette {-1 "black", 0 "white", 50 "yellow", 100 "red"}]
	    set ampduheatmap \
		[UTF::Heatmap %AUTO% -file ${file}_ampdu.csv \
		     -title "AMPDUdens" -dir $dir -showmean 1]
	    set fbheatmap \
		[UTF::Heatmap %AUTO% -file ${file}_fb.csv \
		     -title "Frameburst" -dir $dir -showmean 1]
	} elseif {[$self has_ampdu_dumps $SNK]} {
	    set ampdustats "rx"
	    set ampduhost $SNK
	    set vhtheatmap \
		[UTF::Heatmap %AUTO% -file ${file}_vht.csv \
		     -title "Rate (vht)" -dir $dir]
	    set mcsheatmap \
		[UTF::Heatmap %AUTO% -file ${file}_mcs.csv \
		     -title "Rate (mcs)" -dir $dir]
	} else {
	    set ampdustats "none"
	    set ampduhost "none"
	}

	# AMSDU
	if {[$SRC wl -u amsdu] eq "1" && [$SNK wl -u amsdu] eq "1"} {
	    if {[$self has_amsdu_dumps $SRC]} {
		set amsduhost $SRC
		set amsdustat TxMSDUdens%
		set sduheatmap \
		    [UTF::Heatmap %AUTO% -file ${file}_amsdu.csv \
			 -title "AMSDUdens" -dir $dir -showmean 1]
	    } elseif {[$self has_amsdu_dumps $SNK]} {
		set amsduhost $SNK
		set amsdustat RxMSDUdens%
		set sduheatmap \
		    [UTF::Heatmap %AUTO% -file ${file}_amsdu.csv \
			 -title "AMSDUdens" -dir $dir -yoff 0 -showmean 1]
	    }
	}
	set self
    }

    method clear {} {

	# AMPDU
	if {$ampduhost ne "none"} {
	    if {$new_clear_ampdu} {
		$ampduhost wl dump_clear ampdu
	    } else {
		$ampduhost wl ampdu_clear_dump
	    }
	}

	# AMSDU
	if {[info exists amsduhost]} {
	    if {$new_clear_amsdu} {
		$amsduhost wl dump_clear amsdu
	    } else {
		$amsduhost wl amsdu_clear_counters
	    }
	}

    }

    method collect {x} {

	# AMPDU
	if {$ampduhost ne "none"} {

	    set pdump [$ampduhost wl_dump_ampdu]
	    if {$ampdustats eq "tx"} {
		set vhthist [$pdump TXVHTs%]
		set mcshist [$pdump TXMCSs%]
		set den [$pdump MPDUdens%]
		set vhtper [$pdump VHTPER%]
		set mcsper [$pdump MCSPER%]
		set fb [$pdump Frameburst%]

		$vhtperheatmap add $x $vhtper
		$mcsperheatmap add $x $mcsper
		$ampduheatmap add $x $den
		$fbheatmap add $x $fb

	    } else {
		set vhthist [$pdump RXVHTs%]
		set mcshist [$pdump RXMCSs%]
	    }

	    if {[info exists vhtheatmap]} {
		$vhtheatmap add $x $vhthist
	    }
	    if {[info exists mcsheatmap]} {
		$mcsheatmap add $x $mcshist
	    }
	    $pdump destroy

	}

	# AMSDU
	if {[info exists amsduhost]} {
	    set sdump [$amsduhost wl_dump_amsdu]
	    set sdu [$sdump $amsdustat]
	    $sduheatmap add $x $sdu
	    $sdump destroy
	}
    }

    method plotcount {} {
	if {$ampdustats eq "tx"} {
	    return 5
	} else {
	    return 2
	}
    }

    method plotscripts {} {

	set vhtytic ""
	if {[info exists vhtheatmap]} {
	    set names [$vhtheatmap names]
	    foreach i [dict keys $names] {
		if {$i % 4 == 2} {
		    lappend vhtytic "'[dict get $names $i]' $i"
		}
	    }
	}

	set mcsytic ""
	if {[info exists mcsheatmap]} {
	    set names [$mcsheatmap names]
	    foreach i [dict keys $names] {
		if {$i % 4 == 2} {
		    lappend mcsytic "'[dict get $names $i]' $i"
		}
	    }
	}

	set vhtperytic ""
	set mcsperytic ""
	if {$ampdustats eq "tx"} {
	    set names [$vhtperheatmap names]
	    foreach i [dict keys $names] {
		if {$i % 2 == 1} {
		    lappend vhtperytic "'[dict get $names $i]' $i"
		}
	    }
	    set names [$mcsperheatmap names]
	    foreach i [dict keys $names] {
		if {$i % 2 == 1} {
		    lappend mcsperytic "'[dict get $names $i]' $i"
		}
	    }
	}

	set script "
# Prep for heatmaps
unset y2tics
unset ylabel
set autoscale fix
"

	if {$vhtytic ne "" && $mcsytic ne ""} {
	    # Both VHT and MCS
	    append script "# VHT\n"
	    append script "set origin 0,0.45; set size 0.5,0.15\n"
	    append script [$vhtheatmap plotscript -ytics "([join $vhtytic ,])" -noxtics]
	    append script "# MCS\n"
	    append script "set origin 0,0.3; set size 0.5,0.15\n"
	    append script [$mcsheatmap plotscript -ytics "([join $mcsytic ,])" -notitle]
	} elseif {$vhtytic ne ""} {
	    # VHT only
	    append script "# VHT\n"
	    append script "set origin 0,0.3; set size 0.5,0.3\n"
	    append script [$vhtheatmap plotscript -ytics "([join $vhtytic ,])"]
	} elseif {$mcsytic ne ""} {
	    # MCS only
	    append script "# MCS\n"
	    append script "set origin 0,0.3; set size 0.5,0.3\n"
	    append script [$mcsheatmap plotscript -ytics "([join $mcsytic ,])"]
	}
	if {[info exists ampduheatmap]} {
	    append script "# AMPDU\n"
	    append script "set origin 0.5,0.3; set size 0.5,0.3\n"
	    append script [$ampduheatmap plotscript -ytics 8]
	}

	if {$vhtperytic ne "" && $mcsperytic ne ""} {
	    # Both VHT and MCS
	    append script "# PER (vht)\n"
	    append script "set origin 0,0.15; set size 0.5,0.15\n"
	    append script "set cbrange \[-1:100\]\n"
	    append script [$vhtperheatmap plotscript -ytics "([join $vhtperytic ,])" -noxtics]
	    append script "set cbrange \[*:*\]\n"
	    append script "# PER (mcs)\n"
	    append script "set origin 0,0; set size 0.5,0.15\n"
	    append script "set cbrange \[-1:100\]\n"
	    append script [$mcsperheatmap plotscript -ytics "([join $mcsperytic ,])" -notitle]
	    append script "set cbrange \[*:*\]\n"
	} elseif {$vhtperytic ne ""} {
	    # VHT only
	    append script "# PER (vht)\n"
	    append script "set origin 0,0; set size 0.5,0.3\n"
	    append script "set cbrange \[-1:100\]\n"
	    append script [$vhtperheatmap plotscript -ytics "([join $vhtperytic ,])"]
	    append script "set cbrange \[*:*\]\n"
	} elseif {$mcsperytic ne ""} {
	    # MCS only
	    append script "# PER (mcs)\n"
	    append script "set origin 0,0; set size 0.5,0.3\n"
	    append script "set cbrange \[-1:100\]\n"
	    append script [$mcsperheatmap plotscript -ytics "([join $mcsperytic ,])"]
	    append script "set cbrange \[*:*\]\n"
	}

	if {[info exists sduheatmap]} {
	    append script "# AMSDU\n"
	    append script "set origin 0.5,0.18; set size 0.5,0.12\n"
	    append script [$sduheatmap plotscript -ytics 1]
	}
	if {[info exists fbheatmap]} {
	    append script "# Frameburst\n"
	    append script "set origin 0.5,0.0; set size 0.5,0.18\n";
	    append script [$fbheatmap plotscript];
	}
	return $script
    }

    destructor {

	# AMPDU
	if {[info exists vhtheatmap]} {
	    $vhtheatmap destroy
        }
	if {[info exists mcsheatmap]} {
	    $mcsheatmap destroy
        }
	if {$ampdustats eq "tx"} {
	    $vhtperheatmap destroy
	    $mcsperheatmap destroy
	    $ampduheatmap destroy
	    $fbheatmap destroy
	}
	# AMSDU
	if {[info exists sduheatmap]} {
	    $sduheatmap destroy
	}
    }
}
