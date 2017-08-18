#---------------------------------------------------------------------
# TITLE:
#	trycatch.tcl
#
# AUTHOR:
#	Will Duquette
#
# DESCRIPTION:
#	This package implements an exception handler control structure
#	on top of the [catch] command.  See trycatch.ehtml for user
#	documentation.
#
# 	Copyright (C) 2001 by William H. Duquette.  See license.txt
#	for license information.

#---------------------------------------------------------------------
# Package definition

package provide trycatch 2.0

#---------------------------------------------------------------------
# Namespace definition

namespace eval ::trycatch:: {
    # Exported Commands
    namespace export try throw onerr
}

#---------------------------------------------------------------------
# Command definitions

#---------------------------------------------------------------------
# FUNCTION:
#	try {tryScript} 
#	try {tryScript} catch ?options? {catchScript}
#       try {tryScript} finally {finallyScript}
#
# INPUTS:
#	tryScript	Script to execute, catching errors.
#	args		catch or finally clauses
#
# OUTPUTS:
#	none
#
# RETURNS:
#	The value (and return code) of tryScript or catchScript
#
# DESCRIPTION:
#	Evaluates tryScript in the context of the caller, catching the
#	result and the return code (see [return]).  If the return code 
#	indicates no error, then try returns tryScripts result and
#	return code.  messageVar is unchanged in this case.
#
#	If tryScript fails, then catchScript is evaluated in the context of the
#	caller, and catchScript's result and return code are returned.

proc ::trycatch::try {tryScript args} {
    global errorCode errorInfo

    set usage "syntax: try {...} catch ?opts? {...}, try {...} finally {...}"
    set optionsUsage "valid options: -msg msgVar -code codeVar -info infoVar"
    

    # FIRST, if this is the trivial case, just do it.
    if {[llength $args] == 0} {
        # FIRST, evaluate the tryScript.
        set returnCode [catch {uplevel 1 $tryScript} result]
            
        # NEXT, on error return something innocuous
        if {$returnCode == 1} {
            set returnCode 0
            set result ""
        }

        # NEXT, return it.
        return -code $returnCode $result
    }

    # NEXT, do what makes sense based on the first token in args
    if {"catch" == [Next $args]} {
        # FIRST, get any option values and the catch script
        Pop args
        
        while {[string match "-*" [Next $args]]} {
            set option [Pop args]
            set var [Pop args]

            if {"" == $var} {
                error $optionsUsage
            }

            switch -- $option {
                -msg  {upvar $var theMsg}
                -code {upvar $var theCode}
                -info {upvar $var theInfo}
                default {error $optionsUsage}
            }
        }
        
        if {[llength $args] == 0} {
            error $usage
        }

        set catchScript [Pop args]

        if {[llength $args] != 0} {
            error $usage
        }

        # NEXT, execute the try script, saving its return value.
        set returnCode [catch {uplevel 1 $tryScript} result]

        # NEXT, if there was an error, execute the
        # catchScript in the context of the caller.
        if {$returnCode == 1} {
            # If the appropriate options were passed, then upvar has
            # aliased these to the user's variables.  Otherwise, they
            # are just local.
        
            set theMsg $result
            set theCode $errorCode
            set theInfo $errorInfo

            set returnCode [catch {uplevel 1 $catchScript} result]
        }

        # NEXT, return the result and result code.  If there was an error
        # we need to propagate the error information as well, so go ahead
        # and include them; they'll be ignored if there was no error.
        return -code $returnCode \
                -errorcode $errorCode -errorinfo \
                $errorInfo \
                $result
    } elseif {"finally" == [Next $args]} {
        # FIRST, validate the args, and get the finally script.
        Pop args

        if {[llength $args] != 1} {
            error $usage
        }
        set finallyScript [Pop args]

        # NEXT, evaluate the tryScript, saving its return values
        set returnCode [catch {uplevel 1 $tryScript} result]
        
	# Save globals since they could get trashed by finally block
	set savedInfo $errorInfo
	set savedCode $errorCode

        # NEXT, evaluate the finally script.  Throw an error
        # if the result is anything but "ok"
        set finallyCode [catch {uplevel 1 $finallyScript} finallyResult]

        if {$finallyCode == 1} {
            return -code $finallyCode \
                    -errorcode $errorCode -errorinfo \
                    $errorInfo \
                    $finallyResult
        } elseif {$finallyCode != 0} {
            error "break, return, or continue in finally script"
        }

        # NEXT, return the result and result code.  If there was an error
        # we need to propagate the error information as well, so go ahead
        # and include them; they'll be ignored if there was no error.
        return -code $returnCode \
                -errorcode $savedCode -errorinfo \
                $savedInfo \
                $result
    } else {
        error $usage
    }
}

#---------------------------------------------------------------------
# FUNCTION:
#	throw code message
#
# INPUTS:
#	code		An error code; it should be list.
#	message		A human-readable error message
#
# OUTPUTS:
#	none
#
# RETURNS:
#	Throws an error.
#
# DESCRIPTION:
#	Equivalent to 
#
#           error $message "" $code
#
#       It's provided for code that makes explicit use of error codes.

proc ::trycatch::throw {code message} {
    return -code error -errorcode $code $message
}

#---------------------------------------------------------------------
# FUNCTION:
#	onerr ?-code code? pattern script pattern script...
#
# INPUTS:
#	args		Command arguments
#
# OUTPUTS:
#	none
#
# RETURNS:
#	Return value of executed script, if any.
#
# DESCRIPTION:
#	Matches either the global errorCode or the specified code
#	against each pattern in turn (using Match) until a match is
#	found or the patterns are exhausted.  If a match is found, the
#	the matching script is executed.

proc ::trycatch::onerr {args} {
    global errorCode
    global errorInfo

    set usage "syntax: onerr ?-code errorCode? pattern script ..."

    # FIRST, see if a code has been specified; otherwise, use the
    # global errorCode.

    if {[string match "-*" [Next $args]]} {
        set opt [Pop args]
        if {"-code" != $opt || [llength $args] == 0} {
            error $usage
        }

        set code [Pop args]
    } else {
        set code $errorCode
    }

    # NEXT, match each in turn until a match is found.
    
    if {[llength $args] % 2 != 0} {
        error $usage
    }

    foreach {pattern script} $args {
        # Does it Match?
        if {[Match $pattern $code]} {
            # FIRST, execute the script
            set returnCode [catch {uplevel 1 $script} result]
            
            # NEXT, return the results.  The error data will be
            # ignored if the return code is not an error.
            
            return -code $returnCode \
                    -errorcode $errorCode -errorinfo $errorInfo \
                    $result
        }
    }

    return
}

#---------------------------------------------------------------------
# FUNCTION:
#	Next
#
# INPUTS:
#	list
#
# OUTPUTS:
#	none
#
# RETURNS:
#	Next arg in the list
#
# DESCRIPTION:
#	Returns the next arg in a list

proc ::trycatch::Next {list} {
    return [lindex $list 0]
}

#---------------------------------------------------------------------
# FUNCTION:
#	Pop
#
# INPUTS:
#	listVar
#
# OUTPUTS:
#	none
#
# RETURNS:
#	The first arg
#
# DESCRIPTION:
#	Removes the first arg from the list

proc ::trycatch::Pop {listVar} {
    upvar $listVar theList
    set result [lindex $theList 0]
    set theList [lrange $theList 1 end]

    return $result
}

#---------------------------------------------------------------------
# FUNCTION:
#	Match patternList code
#
# INPUTS:
#	patternList		A list of glob-patterns
#	code			An errorCode value
#
# OUTPUTS:
#	none
#
# RETURNS:
#	1 or 0
#
# DESCRIPTION:
#	Matches the patterns in patternList against the tokens of
#	code.  If a pattern doesn't match, or there are more patterns than
#	tokens, the function returns 0.  Otherwise, it returns 1.

proc ::trycatch::Match {patternList code} {
    set len [llength $patternList]

    if {[llength $code] < $len} {
        return 0
    }

    for {set i 0} {$i < $len} {incr i} {
        set pattern [lindex $patternList $i]
        set token [lindex $code $i]

        if {![string match $pattern $token]} {
            return 0
        }
    }

    return 1
}












