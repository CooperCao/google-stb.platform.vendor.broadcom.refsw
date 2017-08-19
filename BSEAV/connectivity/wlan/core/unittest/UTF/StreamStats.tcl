#!/bin/env utf
#
# $Copyright Broadcom Corporation$
# $Id: 03c17ea3266d85c617801bb579095603324b9a4d $
#

package require snit
package require UTF
package require UTF::Streams

#package provide UTF::StreamStats 2.0

snit::type UTF::StreamStats {

    method compstats {} { 
	UTF::Message INFO $self "Printing stats"
	return
    }

    option -streams 

    constructor {args} {
	set options(-streams) [UTF::stream info instances]
	UTF::Message INFO $self "Streams $options(-streams)"
    }

    destructor {

    }
}

