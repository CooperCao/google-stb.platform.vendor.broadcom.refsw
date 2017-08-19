#!/bin/sh 

# Script to fix up f15 networking setup.
# For details, type: ./fix_f15_net.sh -h

# NB: There is a log file in /root/fix_f15_net.log.html
# NB: Log file is best viewed in web browser.

# Written by John Brearley, Dec 2011

# If TCL was not installed on the PC to begin with, the network services may
# still be non-functional, so it wont be easy to install TCL. Hence this script
# uses the bourne shell, which is guaranteed to be present.

# set -xv ;# shell traces

# Sheida: Note to Self: Caution for highly sensative testbeds.

#==============================================================================
# User is expected to change variables below as needed.
link=em1
domain=sanjose
nis=nis1.sj.broadcom.com
gateway=10.19.84.1
dns1=10.19.84.12
dns2=10.19.84.13
search=sj.broadcom.com
ntp=10.10.32.12
# Leave yum_repo null if no local repository is available
# NB: no trailing "/" please!
yum_repo="fs-sj1-16:/vol/vol02031/hnd_cdroms/os/Linux/Fedora/Fedora15"
# When done, test access to this NFS directory.
test_dir="/projects/hnd"
# End user customization section.
#==============================================================================


#==============================================================================
# Common procedures for this script.
# Main script is at the bottom of the file.
#==============================================================================

# REMINDER: in Bourne shell, all variables are global. 
# So a proc using variable "s" will stomp on any other use of variable "s".
# To make the proc variables unique, put the initials of proc in front of
# variable, eg: sc_s

# NB: shell return can only return integer, GE 0, no text strings.

#========= backup_file ========================================================
# Makes a numbered backup copy of the specified file in the bu_dir.
# Backups are named: file.n, where nnn starts at 0.
# This allows for many different backups of the same file to coexist.
#
# Calling parameter: file
# Returns: null
#==============================================================================
backup_file ()
{

    # Find a suitable integer suffix to use for the file that is going to be
    # backed up.
    bf_i=0
    bf_m=20 ;# max backups to keep
    while [ $bf_i -le $bf_m ]
    do
        # Set dest pathname for file in bu_dir with .n suffix
        bf_d="$bu_dir/`basename $1`.$bf_i"
        # echo "bf_d=$bf_d"

        # Does this backup .n exist?
        if [ ! -f $bf_d ]
        then 
            # We found an unused .n for this file.
            break
        fi

        # Keep going, looking for unused backup .n
        bf_i=`expr $bf_i + 1`
    done

    # Its possible we will hit the max backup value. In this case we give the user a warning
    # and proceed to overwrite the youngest backup. We still have the original file and all the
    # intermediate backup copies.
    if [ $bf_i -ge $bf_m -a -f $bf_d ]
    then
        log_warn "backup_file will overwrite youngest backup: $bf_d"
    fi

    # Make the backup copy.
    echo " "
    echo " "
    log_info "backup_file saving $1 as $bf_d"
    cp -f $1 $bf_d
    check_rc $? "backup_file could not copy $1 to $bf_d"
}

#========= check_pkgs =========================================================
# Checks if packages are installed, uses yum as needed to install.
#
# Calling parameters: pkg_list
# Returns: null
#==============================================================================
check_pkgs ()
{
    log_info check_pkgs: $*
    for cp_p in $*
    do
        # Install/update package. Option -y turns off interactive prompts.
        if [ "$yum_repo" == "" ]
        then
            # Use repository from internet, as per /etc/yum.repos.d
            cp_str=`yum -y install $cp_p 2>&1`
            cp_rc=$?
        else 
            # Use local repostory. Need filetype .rpm
            cd $repo
            cp_rpm=`find . -name "$cp_p*.rpm"`
            echo "$cp_p find: $cp_rpm"
            if [ "$cp_rpm" == "" ]
            then
                log_error "check_pkgs no .rpm found for: $cp_p"
                continue
            fi
            cp_str=`yum -y localinstall $cp_rpm 2>&1`
            cp_rc=$?
        fi

        # Check for errors.
        cp_c1=`echo $cp_str |egrep -i -c no.*package.*available`
        cp_c2=`echo $cp_str |egrep -i -c cannot.*open.*skipping`
        echo "cp_p=$cp_p cp_c1=$cp_c1 cp_c2=$cp_c2 cp_rc=$cp_rc cp_str=$cp_str"
        if [ "$cp_rc" != "0" -o "$cp_c1" != "0" -o "$cp_c2" != "0" ]
        then
            # Log the error.
            log_error "$cp_p cp_rc=$cp_rc cp_c1=$cp_c1 cp_c2=$cp_c2 cp_str: <pre>$cp_str</pre>"
        else
            # Package is OK. Did we really do anything?
            cp_c=`echo $cp_str |egrep -i -c nothing.*to.*do`
            # echo "$cp_p OK, cp_c=$cp_c"
            if [ "$cp_c" != "0" ]
            then
                # Did nothing...
                log_info "$cp_p <pre>$cp_str</pre>"
            else
                # Package was installed/updated.
                log_bold "$cp_p <pre>$cp_str</pre>"
            fi
        fi
    done
}

#========= check_rc ===========================================================
# Checks return code for error. On error, logs error and calls cleanup to 
# exit the script.
#
# Calling parameters: rc msg
# Returns: null
#==============================================================================
check_rc ()
{
    # Exit script if rc != 0
    if [ "$1" != "0" ]
    then
        log_error "rc=$1 $2"
        cleanup
    fi
}

#========= cleanup ============================================================
# Cleanup routine for end of script.
#
# Calling parameters: none
# Returns: Always exits the script 
#==============================================================================
cleanup ()
{
    # First cd out of yum repository. That way, umount wont think the
    # device is still busy.
    cd $saved_pwd

    # Unmount optional local yum repository.
    if [ "$yum_repo" != "" ] 
    then
        umount $yum_repo
    fi

    # Set common exit messages.
    m1="Backup files location: $bu_dir Log file location: $log_file"
    m2="NB: Log is best viewed in a web browser!"
    m3="All done, $tot_change changes made, $tot_error errors, $tot_warn warnings"

    # Color of exit message depends on error & warn counts.
    echo " "
    echo " "
    if [ $tot_error != 0 ] 
    then
        log_error "$m1"
        log_error "$m2"
        log_error "$m3"
    elif [ $tot_warn != 0 ]
    then
        log_warn "$m1"
        log_warn "$m2"
        log_warn "$m3"
    else
        log_ok "$m1"
        log_ok "$m2"
        log_ok "$m3"
    fi

    # Tell user that changes are highlighted in bold text.
    if [ $tot_change != 0 ]
    then
        log_info "NB: Changes made are logged with <b>CHANGED: bold text!</b>"
    fi
    exit $tot_error
}

#========= help ===============================================================
# Provides online help info
#
# Calling parameters: all command line tokens
# Returns: null
#==============================================================================
help ()
{
    # Return if help was not requested.
    h_c2=`echo $1 | cut -c1-2 |tr [A-Z] [a-z]`
    # echo "help h_c2=$h_c2"
    if [ "$h_c2" != "-h" ]
    then
        return
    fi

    # Basic help info.
    echo " "
    echo "Basic useage: `basename $0`"
    echo " "
    echo "Script will modify networking related config files, start/stop services"
    echo "as appropriate. Files are automatically backed up before being modified."
    echo " "
    echo "There is a running log file showing what was changed or not. The log file"
    echo "is best viewed in a web browser."
    echo " "
    echo "You may need to update the parameters at the top of the script for your"
    echo "specific subnet. The script will fix up f15 network settings using:"
    echo "link=$link domain=$domain nis=$nis gateway=$gateway"
    echo "search=$search dns=$dns1, $dns2 ntp=$ntp"
    echo "yum_repo=$yum_repo"
    echo "test_dir=$test_dir"
    echo " "
    exit 1
}

#========= log_bold ===========================================================
# Adds a bold log message to running log file, send msg to stdout.
# Bold is used to indicate items that were changed.
#
# Calling parameters: msg str
# Returns: null
#==============================================================================
log_bold ()
{
    log_info "<b>CHANGED: $*</b>"
    tot_change=`expr $tot_change + 1`
}

#========= log_error ==========================================================
# Adds a bold RED ERROR log message to running log file, send msg to stdout.
# Increments tot_error counter.
#
# Calling parameters: msg str
# Returns: null
#==============================================================================
log_error ()
{
    log_info "<b><font color=\"red\">ERROR: $*</font></b>"
    tot_error=`expr $tot_error + 1`
}

#========= log_ok =============================================================
# Adds a bold GREEN OK log message to running log file, send msg to stdout.
#
# Calling parameters: msg str
# Returns: null
#==============================================================================
log_ok ()
{
    log_info "<b><font color=\"green\">OK: $*</font></b>"
}

#========= log_info ===========================================================
# Adds a non-bold log message to running log file, send msg to stdout.
#
# Calling parameters: msg str 
# Returns: null
#==============================================================================
log_info ()
{
    # Send msg to stdout.
    echo "$*"

    # Send msg to log_file with html tags.
    echo "$*<br>" >> $log_file
}

#========= log_warn ===========================================================
# Adds a bold GOLD WARN log message to running log file, send msg to stdout.
# Increments tot_warn counter.
#
# Calling parameters: msg str
# Returns: null
#==============================================================================
log_warn ()
{
    log_info "<b><font color=\"gold\">WARN: $*</font></b>"
    tot_warn=`expr $tot_warn + 1`
}

#========= mount_repo ===========================================================
# Mounts the local yum repository, if any.
#
# Calling parameters: none
# Returns: null
#==============================================================================
mount_repo ()
{
    # Local yum repository is optional.
    if [ "$yum_repo" == "" ] 
    then
        return
    fi

    # Mount local yum repository.
    repo="/yum_repo"
    log_info "mount_repo yum_repo=$yum_repo as repo=$repo"
    mkdir -p $repo
    check_rc $? "mount_repo could not mkdir $repo"
    mp_str=`mount $yum_repo $repo`
    check_rc $? "mount_repo could not mount $repo: $mp_str <br>Try manually running: yum install nfs-utils<br>Then run the script again."
}

#========= remove_pkgs ========================================================
# Uses yum to remove packages if they are currently installed.
#
# Calling parameters: pkg_list
# Returns: null
#==============================================================================
remove_pkgs ()
{
    log_info remove_pkgs: $*
    for rp_p in $*
    do
        # Remove package. Option -y turns off interactive prompts.
        rp_str=`yum -y remove $rp_p 2>&1`
        rp_rc=$?

        # Check if anything was done.
        rp_c=`echo $rp_str |egrep -i -c No.*packages.*marked.*for.*removal`
        # echo "rp_p=$rp_p rp_rc=$rp_rc rp_c=$rp_c rp_str=$rp_str"
        if [ "$rp_rc" != "0" -o "$rp_c" != "0" ]
        then
            # Did nothing.
            log_info "<pre>$rp_p $rp_str</pre>"
        else
            # Did we really do anything?
            rp_c=`echo $rp_str |egrep -i -c Removed:.*$rp_p`
            # echo "$rp_p gone, rp_c=$rp_c"
            if [ "$rp_c" != "0" ]
            then
                # Package was removed.
                log_bold "<pre>$rp_p $rp_str</pre>"
            else
                # Error.
                log_error "<pre>$rp_p rp_rc=$rp_rc $rp_str</pre>"
            fi
        fi
    done
}

#========= service_control ====================================================
# Turns specified service on/off as desired, sets chkconfig appropriately.
# Used for services not yet fully integrated into systemctl.
#
# Calling parameters: name levels state <pattern value>
#
# For some services, eg iptables, we recognize the service is running when
# certain phrases are present in the service status. However, we usually want
# to turn off iptables. This is why we have a separate state & value calling
# parameters.
#
# Returns: null
#==============================================================================
service_control ()
{
    # Get calling parameters
    # log_info "service_control 1=$1 2=$2 3=$3 4=$4 5=$5"

    # Check state is on, off. Set sc_cmd according to state.
    if [ "$3" == "on" ]
    then 
        sc_cmd=restart
    elif [ "$3" == "off" ]
    then
        sc_cmd=stop
    else
        log_error "service_control $* invalid state: $3"
        exit 1
    fi
    
    # Check current state of service
    service_status $1 $4 $5
    # echo "1=$1 svc=$svc"
    if [ "$svc" != "$3" ]
    then
        # Change the service state
        log_bold "service_control $1 is currently $svc, will $sc_cmd"
        changed=yes
        service $1 $sc_cmd
        check_rc $? "service_control $1 $sc_cmd failed"
    fi

    # Query the chkconfig levels.
    sc_str=`chkconfig --list $1 2>&1`
    # echo "1=$1 sc_str=$sc_str"

    # Do the levels match the desired settings?
    # NB: Levels are space separated for easier processing.
    sc_ok=1 ;# flag to indicate levels are OK.
    for sc_l in $2
    do
        # Look for patterns like: on:3
        sc_c=`echo $sc_str | egrep -i -c "${sc_l}:${3}"`
        # echo  "sc_l=$sc_l sc_c=$sc_c"
        if [ $sc_c == 0 ]
        then
            sc_ok=0
            # echo "not OK: sc_l=$sc_l sc_c=$sc_c"
            break
        fi
    done

    # If levels are OK, return.
    if [ $sc_ok == 1 ] 
    then
        log_info "service_control $1 chkconfig levels $2 are OK, all: $3"
        return
    fi

    # Correct the chkconfig levels
    log_bold "service_control $1 chkconfig setting levels $2 to: $3"
    changed=yes
    for sc_l in $2
    do
        # echo "sc_l=$sc_l"
        chkconfig --level $sc_l $1 $3
        check_rc $? "service_control chkconfig --level $sc_l $1 $3 failed"
    done
}

#========= service_status ====================================================
# Queries the specified service status. If optional pattern & value are
# specified and the status string matches the optional pattern, the
# optional value is returned in variable svc.
#
# Calling parameters: service <pattern value>
# Returns: null
# Sets variable svc.
#==============================================================================
service_status ()
{
    # NB: Some services, like iptables, report rc=0 no matter what.
    # So we need the calling routine to specify a service specific pattern
    # that truly indicate whats going on.
    # echo "Checking $1 status 2=$2 3=$3"

    # Check value is on, off or null. Set svc default to opposite of value.
    if [ "$3" == "on" ] 
    then
        svc="off"
    elif [ "$3" == "off" ]
    then 
        svc="on"
    elif [ "$3" == "" ]
    then 
        svc=""
    else
        log_error "service_status $* invalid value: $3"
        exit 1
    fi

    # Get service status.
    ss_str=`service $1 status 2>&1`
    # echo "1=$1 ss_str=$ss_str"

    # Parse output for not.*running first, to avoid matching running
    # Also include service name in pattern. This avoid matching NOTCONFIGURED
    # at start of NFS status output.
    for ss_l in not.*running
    do
        # echo "checking ss_l=$ss_l"
        ss_c=`echo $ss_str | egrep -c -i $1.*$ss_l`
        if [ $ss_c != 0 ]
        then
            # echo "$1 service match: $ss_l ss_c=$ss_c"
            svc=off
            log_info "service_status $1 is $svc"
            return
        fi
    done

    # Parse output for known indicators service is on.
    # Also include service name in pattern.
    # NFS shows multiple status, one deamon always seems to be exited,
    # but the service is OK. So we so this check running next.
    for ss_l in running
    do
        # echo "checking ss_l=$ss_l"
        ss_c=`echo $ss_str | egrep -c -i $1.*$ss_l`
        if [ $ss_c != 0 ]
        then
            # echo "$1 service match: $ss_l ss_c=$ss_c"
            svc=on
            log_info "service_status $1 is $svc"
            return
        fi
    done

    # Parse output for other known indicators service is off.
    # Also include service name in pattern.
    for ss_l in inactive exited dead unrecognized
    do
        # echo "checking ss_l=$ss_l"
        ss_c=`echo $ss_str | egrep -c -i $1.*$ss_l`
        if [ $ss_c != 0 ]
        then
            # echo "$1 service match: $ss_l ss_c=$ss_c"
            svc=off
            log_info "service_status $1 is $svc"
            return
        fi
    done

    # Parse output for optional pattern.
    if [ "$2" != "" -a "$3" != "" ]
    then
        ss_c=`echo $ss_str | egrep -c -i $2`
        if [ $ss_c != 0 ]
        then
            # echo "$1 service match: 2=$2 ss_c=$ss_c"
            svc=$3
        fi
    fi
    log_info "service_status $1 is $svc"
}

#========= setup ==========================================================
# Sets up backup directory, running log file & various counters.
#
# Calling parameters: none
# Returns: null
#==============================================================================
setup ()
{
    # Initialize counters
    tot_change=0
    tot_error=0
    tot_warn=0

    # Setup running log file.
    log_file=`basename $0`
    # echo "log_file=$log_file"
    log_file=`echo $log_file |cut -d '.' -f1`
    log_file="/root/$log_file.log.html"
    # echo "log_file=$log_file"

    # Save pwd
    saved_pwd=`pwd `

    # Add datestamp to running log file.
    log_info " "
    log_info " "
    log_info "<hr>"
    log_info "`date` `hostname` `uname -r` pwd=$saved_pwd $0 starting"

    # Setup backup file directory
    bu_dir=/root/bu
    mkdir -p $bu_dir
    check_rc $? "setup could not create directory: $bu_dir"

    # Check OS is fc15.
    if [ ! `uname -r |egrep -i fc15` ]
    then 
        log_error "Script intended for fc15, not `uname -r`"
        exit 1
    fi
}

#========= show_file ==========================================================
# Displays file on stdout for user to see.
#
# Calling parameters: file
# Returns: null
#==============================================================================
show_file ()
{
    # Display file on terminal for user.
    echo " "
    echo "file: $1"
    cat $1
    echo " "
}

#========= system_ctl =========================================================
# In F15, some services are fully integrated into systemctl. This routine is
# used for those services.
#
# For some services we recognize the service is running when certain phrases
# are present in the service status. However, we may want to turn off the 
# service. This is why we have a separate state & value calling parameters.
#
# Calling parameters: service state <pattern value>
# Returns: null
#==============================================================================
system_ctl ()
{
    # Get calling parameters
    # log_info "system_ctl 1=$1 2=$2 3=$3 4=$4"

    # Check state is on, off. Set sl_cmds to achieve the desired state.
    if [ "$2" == "on" ]
    then 
        sl_cmds="enable restart"
    elif [ "$2" == "off" ]
    then
        sl_cmds="disable stop"
    else
        log_error "system_ctl $* invalid state: $2"
        exit 1
    fi
    
    # Check current state of service
    service_status $1 $3 $4
    # echo "1=$1 svc=$svc"
    if [ "$svc" == "$2" ]
    then
        return
    fi

    # Change the service state
    for sl_c in $sl_cmds
    do 
        log_bold "system_ctl $1 is currently $svc, will $sl_c"
        changed=yes
        systemctl $sl_c $1.service
        check_rc $? "system_ctl systemctl $sl_c $1.service failed"
    done

    # There are no chkconfig levels to query or set.
}
  
#========= update_file ========================================================
# Used to update data in a file. If keyword is found, then operand and value
# are checked for desired values. If all is correct, no change is made. If
# incorrect values are found, line is deleted and line with desired values
# is appended to the end of file. If value is null, line with keyword will be
# deleted. If no line with keyword is found, line with values will be appended
# to the end of the file.
#
# On occasion, you will need the same keyword on multiple lines with different
# values on each line. To allow this, specify the last parameter as "dup".
# Otherwise, leave last paramter null.
#
# Calling parameters: file keywork operand value duplicate_allowed
# Returns: null
#==============================================================================
update_file ()
{
    # log_info "update_file 1=$1 2=$2 3=$3 4=$4 5=$5"

    # Does the file(1) exist?
    if [ ! -f "$1" ]
    then
        log_error "update_file $1 not found!"
        exit 1
    fi

    # Look for desired key(2) in file(1).
    # egrep -c returns count of matching lines, suppresses line output. 
    # Feeding multiple lines of text into shell test operator gives errors.
    uf_c=`egrep -c -i $2 $1`
    # echo "uf_c=$uf_c"
    if [ $uf_c != 0 ]
    then
        # We found the desired key
        # log_info "update_file $1 have: $2"
        if [ "$4" == "" ]
        then
            # Null val(4) means delete this key from the file.
            log_bold "update_file $1 deleting: $2"
            uf_tmp="$1.tmp"
            egrep -v -i $2 $1 > $uf_tmp
            mv $uf_tmp $1
            changed=yes
            return
        else
            # Do we have all 3 desired key(2), op(3) & val(4)?
            uf_c=`egrep -c -i "${2}${3}${4}" $1`
            # echo "uf_c=$uf_c"
            if [ $uf_c != 0 ]
            then
                # We do have all 3 desired key, op & val.
                log_info "update_file $f found: ${2}${3}${4}"
                return
            else 
                # We have wrong op/val. Delete old line, add new line.
                if [ "$5" != "dup" ]
                then
                    # There are some cases where we want to allow multiple values for same key
                    uf_tmp="$1.tmp"
                    egrep -v -i $2 $1 > $uf_tmp
                    mv $uf_tmp $1
                fi
                log_bold "update_file $1 updating: ${2}${3}${4}"
                echo "${2}${3}${4}" >> $1
                changed=yes
                return
            fi
        fi
    fi
    # log_info "egrep $2 not found"

    # We did NOT find the key.
    if [ "$4" == "" ]
    then
        # Val(4) is null, so leave this key out of the file.
        return
    fi

    # Add this key.
    log_bold "update_file $1 adding: ${2}${3}${4}"
    echo "${2}${3}${4}" >> $1
    changed=yes
}

#==============================================================================
# Main program starts here
#==============================================================================
# Offer online help
help $*

# Do basic setup.
setup

# Prompt user for permission to continue.
echo " "
log_info "Script will fix up f15 network settings using:"
log_info "link=$link domain=$domain nis=$nis gateway=$gateway"
log_info "search=$search dns=$dns1, $dns2 ntp=$ntp"
log_info "yum_repo=$yum_repo"
log_info "test_dir=$test_dir"
echo " "
echo "OK to proceed? (y/N)"
read x
if [ "$x" != "y" ]; then log_info "Halting..."; exit 1; fi

# Look for static IP address in network link script, which is required for this setup.
file="/etc/sysconfig/network-scripts/ifcfg-$link"
if [ ! -f "$file" ] ; then log_error "$file not found!" ; exit 1 ; fi
ipaddr=`egrep -i "IPADDR=.*[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+" $file` ;# may have quotes around IP address
# echo "ipaddr=$ipaddr"
ipaddr=`echo $ipaddr | cut -d '=' -f2 | tr '"' ' '`
ipaddr=`echo $ipaddr` ;# trims spaces
# echo "ipaddr=$ipaddr" 
if [ "$ipaddr" != "" ]
then
    log_info "$file has static IP: $ipaddr" 
else
    log_error "You need a static IP address for this configuration to work! Please fix: $file"
    exit 1
fi

# Check NetworkManager is off first, before doing anything else.
changed=no
service_status NetworkManager
log_info "NetworkManager is: $svc"
if [ "$svc" != "off" ] 
then 
    # NetworkManager is now run by syscontrol.
    log_bold "Stopping NetworkManager"
    systemctl stop NetworkManager.service
    log_bold "Disabling NetworkManager"
    systemctl disable NetworkManager.service
    log_bold "Enabling network"
    chkconfig --level 23456 network on
    changed=yes
    log_info "Wait while NetworkManager shuts down"
    sleep 30
fi

# Update network link script after NetworkManager is off.
backup_file $file
update_file $file ONBOOT = yes
update_file $file BOOTPROTO = none
update_file $file NM_CONTROLLED = no
update_file $file GATEWAY = $gateway
update_file $file PREFIX = 22
update_file $file PEERDNS = YES
update_file $file PEERNIS = NO
update_file $file DNS1 ;# delete DNS entries
update_file $file DNS2
update_file $file SEARCH
update_file $file HWADDR
show_file $file

# Update network file after NetworkManager is off.
file="/etc/sysconfig/network"
backup_file $file
host=`hostname`
update_file $file HOSTNAME = $host
update_file $file NETWORKING = YES
update_file $file GATEWAYDEV = $link
update_file $file GATEWAY = $gateway
update_file $file NISDOMAIN = $domain
update_file $file NTPSERVERARGS ;# delete line
show_file $file

# Update hosts file after NetworkManager is off.
file="/etc/hosts"
backup_file $file
update_file $file 127.0.0.1 " " "localhost.localdomain localhost"
update_file $file $ipaddr " " "$host $host.$search"
show_file $file

# Update resolv.conf after NetworkManager is off.
file="/etc/resolv.conf"
backup_file $file
update_file $file nameserver " " $dns1 dup ;# allow duplicate key
update_file $file nameserver " " $dns2 dup
update_file $file search " " $search
update_file $file domain " " $search
show_file $file

# Turn off both firewalls.
# Iperf and telnet wont be able to connect if the firewalls are on.
service_control iptables  "0 1 2 3 4 5 6" off ESTABLISHED on
service_control ip6tables "0 1 2 3 4 5 6" off ESTABLISHED on

# Retart network as needed
# echo "changed=$changed"
if [ "$changed" != "no" ]
then
    log_bold "Restarting network"
    service network restart
    check_rc $? "network failed to restart"
fi

# Make sure network settings are correct.
service_control network "2 3 4 5 6" on "active.*$link" on

# Ping google to test DNS, etc.
url=google.com
echo " " 
echo "Test ping $url"
ping -c 5 $url
check_rc $? "ping $url failed"

# Now that NIS is working, we should be able to mount the local
# yum repository, if any.
mount_repo

# These are mandatory packages.
mandatory_pkg="ypbind autofs rpcbind nfs-utils"
echo " "
log_info "Mandatory packages: $mandatory_pkg"
check_pkgs "$mandatory_pkg"

# These are optional packages. 
pkg_list="@system-tools xfsprogs mtools gpgme gpm lua\
     cmake pptp gvfs-obexftp hdparm gssdp geoclue radeontool enca apg festival ntpdate\
     xsel gupnp fuse ncftp gdm tcsh crash expect tftp tcl hping3 gnuplot dhcp synergy-plus"
echo " "
log_info "Install optional packages: $pkg_list"
echo "OK to proceed? (y/N)"
read x
if [ "$x" == "y" ]
then
    check_pkgs "$pkg_list"
else
    log_info "Skipping optional packages"
fi

# Update NTP server
file="/etc/ntp.conf"
backup_file $file
update_file $file SERVER " " $ntp
# show_file $file
system_ctl ntpd on

# Turn off sendmail.
echo " " 
service_control sendmail "0 1 2 3 4 5 6" off

# Turn off dhcpd.
echo " "
system_ctl dhcpd off

# Make sure selinux is off.
# NB: If selinux is on, UTF will get prompted for a password for
# each ssh command sent to the PC!
changed=no
file="/etc/selinux/config"
backup_file $file
update_file $file SELINUX = disabled
show_file $file
if [ $changed != "no" ]
then
    log_warn "$file changed, please reboot PC for changes to take effect!"
fi

# Add options for kernel to reboot on panic or oops
changed=no
file="/etc/sysctl.conf"
backup_file $file
update_file $file kernel.panic " = " 10
update_file $file kernel.panic_on_oops " = " 1
show_file $file
if [ $changed != "no" ]
then
    log_warn "$file changed, please reboot PC for changes to take effect!"
fi

# Items needed to get NFS working properly.
# Turn off selected services.
service_control rpcgssd "0 1 2 3 4 5 6" off    
service_control rpcidmapd "0 1 2 3 4 5 6" off
service_control rpcsvcgssd "0 1 2 3 4 5 6" off

# Check yp.conf
changed=no
file="/etc/yp.conf"
backup_file $file
update_file $file domain " " "$domain server $nis"
show_file $file

# Check nsswitch.conf
file="/etc/nsswitch.conf"
backup_file $file
update_file $file passwd: " " "files nis"
update_file $file shadow: " " "files nis"
update_file $file group: " " "files nis"
update_file $file hosts: " " "files nis dns"
update_file $file netgroup: " " "files nis"
update_file $file automount: " " "files nis " ;# trailing space after nis needed to avoid nisplus!
show_file $file

# Check auto.master
file="/etc/auto.master"
backup_file $file 
update_file $file /home  " " "yp:auto_home   rw,intr,actimeo=3,bg"
update_file $file /projects " " "yp:auto_projects   rw,intr,actimeo=3,bg"
update_file $file /tools " " "yp:auto_tools   rw,intr,actimeo=3,bg"
show_file $file

# If config files changed, restart nfs related services.
if [ $changed != "no" ]
then
    log_bold "Restart rpcbind"
    service rpcbind restart

    log_bold "Restart ypbind"
    service ypbind restart 

    log_bold "Restart autofs"
    service autofs restart 

    log_bold "Restart nfs"
    service nfs restart 
fi

service_control ypbind  "2 3 4 5" on
service_control rpcbind  "2 3 4 5" on
service_control autofs  "2 3 4 5" on
service_control nfs  "2 3 4 5" on

# After PC is rebooted, even though NetworkManager has been turned off,
# it still has a negative impact on NFS. So remove it completely!
remove_pkgs NetworkManager

# Can we access file servers?
echo " "
echo "Test access to: $test_dir"
str=`ls $test_dir`
check_rc $? "Could not ls $test_dir: $str <br>You may need to reboot PC to get NFS working properly."
log_info "ls $test_dir: $str"

# Run cleanup routine
cleanup
