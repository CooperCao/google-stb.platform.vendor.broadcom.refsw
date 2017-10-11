#!/usr/local/bin/perl

use strict;
use XML::Twig;
use Getopt::Long;
use LWP::Simple;
use Date::Parse;
use POSIX;



my $svnBaseUrl="http://svn.sj.broadcom.com/svn/wlansvn/proj/trunk";
my @svnurl;                        #svn url
my $FROM;                          #It may be date or revision
my $TO;                            #It may be date or revision
my $SVNURL;                        #We will get commit info of this URL
my @fromdate;                      #From this date we will get the check-in log
my @todate;                        #Till this date we will get the check-in log; DEFAULT VALUE IS HEAD
my @fromrev;                       #From this svn revision we will get the check-in log
my @torev;                         #Till this svn revision we will get the check-in log; DEFAULT VALUE IS HEAD
my @module;                        #Different relative path of svn repository with respect to svnurl
my $timewindow;
my @user;                          #svn log will display only for user
my $help;
our $full;
# Getting Data From Command line Arguments

GetOptions (
            "url:s"          => \@svnurl,
            "fromdate:s"     => \@fromdate,
            "todate:s"       => \@todate,
            "fromrev:s"      => \@fromrev,
            "torev:s"        => \@torev,
            "module:s"       => \@module,
            "timewindow:s"   => \$timewindow,
            "user=s"         => \@user,
            "help"           => \$help,
            "full"           => \$full,
           );

my $yest = `date --date=yesterday "+%F"`;
chomp $yest;


#Checking the format of the arguments supplied
if (@fromdate > 1 )
{
    print "Pass only one fromdate in the command line.\n";
}
elsif (@fromdate)
{
    &checkdate(@fromdate);
}

if (@todate > 1 )
{
    print "Pass only one todate in the command line.\n";
}
elsif (@todate)
{
    &checkdate(@todate);
}

sub checkdate
{
    if ($_[0] !=~ m/^\d\d\d\d-\d\d-\d\d$/ )
    {
        print "";
    }
else
    {
        print "The format for fromdate and todate is: YYYY-MM-DD.\n";
	exit (1);
    }
}

if (@fromrev > 1 )
{
    print "Pass only one fromrev in the command line argument.\n";
    exit (1);
}
elsif (@fromrev)
{
    &checkrev(@fromrev);
}

if (@torev > 1 )
{
    print "Pass only one torev in the command line argument.\n";
    exit (1);
}
elsif (@torev)
{
    &checkrev(@torev);
}

sub checkrev
{
    if ($_[0] =~ m/^\d+$/ )
    {
        print "";
    }
else
    {
        print "revision number (both fromrev and torev) is a number.\n";
	exit (1);
    }
}

# Either todate or torev shall be supplied as argument; Default will be HEAD incase nither argument
if (@todate && @torev)
{
    print "Pass either todate or torev in the Command line argument.\n";
    print "Default will be HEAD incase nither argument passed.\n";
    print "For more information see help: perl $0 --help \n";
    exit (1);
}
elsif (@todate)
{
    $TO="{$todate[0]}";
    if (!@fromdate || !@fromrev || !$timewindow)
    {
	&find_fromdate($todate[0],1);
    }
}
elsif (@torev)
{
    $TO=$torev[0];
}
else
{
    $TO="HEAD";
}

# Either fromdate or fromrev or timewindow shall be supplied as argument;
# Default will be calculated from torev
if ((@fromdate && @fromrev) || (@fromdate && $timewindow) || (@fromrev && $timewindow))
{
    print "Pass either  fromdate or fromrev or timewindow in the Command line.\n";
    print "Default will be calculated with respect to todate or svn HEAD version.\n";
    print "For more information see help: perl $0 --help \n";
    exit (1);
}
elsif (@fromdate)
{
    $FROM="{$fromdate[0]}";
}
elsif (@fromrev)
{
    $FROM=$fromrev[0];
}
elsif ($timewindow)
{
    if (@todate)
    {
        &find_fromdate($todate[0],$timewindow);
    }
    elsif (@torev)
    {
        print "timewindos is not allowed with torev.\n";
	exit (1);
    }
    else
    {
        my $timenow = `date "+%F"`;
        chomp $timenow;
        &find_fromdate ($timenow,$timewindow);
    }
}
elsif (!@todate)
{
    $FROM = "{$yest}";
}

sub find_fromdate
{
    my ($year, $month, $day);
    my $from_year = substr ($_[0], 0, 4);
    my $from_month = substr ($_[0], 5, 2);
    my $from_date = substr ($_[0], 8, 2);
    use Date::Calc qw(Add_Delta_Days);
    ($year, $month, $day) = Add_Delta_Days($from_year,$from_month,$from_date,-$_[1]);
    $FROM = "{$year-$month-$day}";
}

if (@svnurl > 1 )
{
    print "Pass only one svn url in the command line argumentone.\n";
    print "Pass different svn repository path in --module option by taking relative path with respect to the one svn url.\n";
    exit (1);
}
elsif (@svnurl)
{
     &checksvnurl(@svnurl);
}

sub checksvnurl
{

    if ($_[0] =~ m#\Qhttp://svn.sj.broadcom.com/svn/wlansvn\E# )
    {
        print "";
    }
else
    {
        print "The URL $_[0] is not correct.\n\n";
	exit (1);
    }
}

my $module_string=join(" ",@module);
my $svnurl = $svnurl[0];
if ( grep {/\/$/} $svnurl )
{
    $svnurl = substr $svnurl, 0, -1;
}

our @module_check;
our $fragment;
our $PATH_HEADER;
our $only_from_trunk;

if ($svnurl && @module)
{
    $SVNURL=$svnurl." ". $module_string;
    $fragment = substr $svnurl, 38;
    @module_check = map { "$fragment/$_" } @module;
}
elsif (!$svnurl && @module)
{
    $SVNURL=$svnBaseUrl." ". $module_string;
    $fragment = substr $svnBaseUrl, 38;
    @module_check = map { "$fragment/$_" } @module;
    $PATH_HEADER = "PATH (under /proj/trunk)";
    $only_from_trunk = "$fragment/";
}
elsif ($svnurl && !@module)
{
    $SVNURL=$svnurl;
    $fragment = substr $svnurl, 38;
    @module_check = ("$fragment");
}
else
{
    $SVNURL=$svnBaseUrl;
    $fragment = substr $svnBaseUrl, 38;
    @module_check = ("$fragment");
    $PATH_HEADER = "PATH (under /proj/trunk)";
    $only_from_trunk = "$fragment/";
}

&Help if ( $help );

sub Help
{
    print "$0: Show the log messages of a svn repository in customised format.\n";
    print "\nUsage: perl $0 \n";
    print "       It will display all the commit in last 24 hour in the $svnBaseUrl Only.\n";
    print "       The out put can be customised with differnt options passing as command line argument \n";

    print "\nValid Options:\n";
    print "    --url                : Print the log messages for the PATH under url. If module relative path  \n";
    print "                           is supplied in the argument log will display for the module.\n";
    print "                           The script will accept only ONE url.\n";
    print "                           Default value is $svnBaseUrl.\n";
    print "    --todate             : svn log will display check-in information TILL todate. FORMAT: YYYY-MM-DD.\n";
    print "                           Default value is svn HEAD version.\n";
    print "    --fromdate           : svn log will display FROM fromdate. FORMAT: YYYY-MM-DD.\n";
    print "                           Default value is 1 day from todate.\n";
    print "                           If there is no todate then log will display for last 24 hour.\n";
    print "    --torev              : svn log will display check-in information TILL this revision.\n";
    print "                           Default value is svn HEAD version.\n";
    print "    --fromrev            : svn log will display FROM this revision.\n";
    print "    --module             : relative path of all the folder whose check-in information will display in the log.\n";
    print "                           The path is relative to --url. Any number of path can be supplied.\n";

    print "    --user               : svn log will disply check-in informaiton of all user supplied in --user option from command line.\n";
    print "                           Default all user check-in information will display as output.\n";
    print "                           If user name is not correct or no check in has done by the user, the script will Only ignore the name.\n";
    print "    --timewindow         : No of days. Log will diplay check-in information from \"todate-timewindow\" till \"todate\".\n";
    print "    --full               : With this argument we can see all changes happened in the svn repository\n";
    print "                           Without this argument the output will display the changes happened only to specific modules supplied as argument \n";

    print "    --help               : Display this help and exit.\n";

    print "\nNote:\n";
    print "      1. Use either fromdate or fromrevision or timewindow and do not use two of them at a time. \n";
    print "      2. Use either todate or torev and do not use both.\n";
    print "      3. Any number of user and module can be passed in the argument.\n";
    exit(0);
}
#===========================================================================================
my $xmlstr = `svn log -v --xml -r "$FROM":"$TO" $SVNURL`;
if ($? != 0)
{
    print "Please pass correct argument. or please check perl $0 --help\n";
    exit (1);
}

&deleteTempFile;
sub deleteTempFile
{
    if ( -e "/tmp/svnLog.txt")
    {
        unlink("/tmp/svnLog.txt");
    }
}

sub printHeader
{
    printf "---------------------------------------------------------------------------------\n";
    if ($_[0])
    {
        printf ("%-7s %-8s %-20s %s\n" , "REVISION", "AUTHOR", "DATE", "$_[0]" );
    }
    else
    {
        printf ("%-7s %-8s %-20s %s\n" , "REVISION", "AUTHOR", "DATE", "PATH" );
    }
    printf "---------------------------------------------------------------------------------\n";
}

my $twig = XML::Twig->new(
    twig_handlers => {logentry => \&logENTRY}
);
$twig->parse($xmlstr);

sub logENTRY
{
    my ($t, $elt) = @_;
    my $revision = $elt->att('revision');
    my $author = $elt->first_child('author'  )->text();
    my $date = $elt->first_child('date'      )->text();
    my $paths = $elt->first_child('paths');
    my @path = $paths->children('path');

#Converting UTC time to Local time
    my $UTC_TIME = "$date";
    $UTC_TIME =~ s/T/ /;
    my $UTC = substr($UTC_TIME,0,19);
    my $UTC_SEC = substr($UTC_TIME,17,2);
    my $TIME = strftime("%Y-%m-%d %R", localtime(str2time($UTC, 'GMT')));
    $TIME = "$TIME".":"."$UTC_SEC";

    open my $svnLog, ">>", "/tmp/svnLog.txt";

    if (($author ne "automrgr") && !@user )
    {
        printf {$svnLog} ("%-7s %-8s %-20s %s\n" , $revision, $author, $TIME, "- - - - CHANGED_FILE_LIST - - - -");

        foreach my $path (@path)
        {
            my $src_path = $path->text;
            if ($full)
            {
                printf {$svnLog} ("%-37s %s\n", " ",$src_path);
            }
            else
            {
                foreach my $module_path (@module_check)
                {
                    if ($src_path =~ m#\Q$module_path\E# )
                    {
                        printf {$svnLog} ("%-37s %s\n", " ",$src_path);
                    }
                }
            }
        }
     }
    elsif (@user)
    {
        foreach my $user(@user)
        {
            if ($author eq $user)
            {
                printf {$svnLog}("%-7s %-8s %-20s %s\n" , $revision, $author, $TIME, "- - - - CHANGED_FILE_LIST - - - -");
                foreach my $path (@path)
                {
                    my $src_path = $path->text;
                    if ($full)
                    {
                        printf {$svnLog} ("%-37s %s\n", " ",$src_path);
                    }
                    else
                    {
                        foreach my $module_path (@module_check)
                        {
                            if ($src_path =~ m#\Q$module_path\E# )
                            {
                                 printf {$svnLog} ("%-37s %s\n", " ",$src_path);
                            }
                        }
                    }
                }
            }
        }
    }
}

#Displaying svn log data stored in /tmp/svnLog.txt and /tmp/svnHeader.txt, and Deleting both files.
if ( ! -e "/tmp/svnLog.txt" )
{
    print "There is no commit for the user passing in --user option or there is no change in the module passing in --module option for the time period.\n";
    print "Please pass correct argument.\n";
    exit(1);
}
else
{
    &printHeader($PATH_HEADER);

    open (svnLogInfo, "/tmp/svnLog.txt");
    while(<svnLogInfo>)
    {
        if ($only_from_trunk )
        {
            $_ =~ s/$only_from_trunk//g;
            print "$_";
        }
        else
        {
            print "$_";
        }
    }
}

#Checking if right user name passed through argument or not.
foreach my $user(@user)
{
    if (! `cat /tmp/svnLog.txt | grep $user`)
    {
        print "No check-in for user Name: $user\n";
    }
}

&deleteTempFile;

