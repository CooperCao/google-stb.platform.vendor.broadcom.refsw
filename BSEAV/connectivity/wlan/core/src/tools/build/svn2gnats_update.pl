#!/usr/local/bin/perl
# -*-Perl-*-
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#
# Converted from CVS backend hook_run log_full for SVN commit update
# in GNATS audit-trails. This script takes SVN revision as an argument
# and uses its commit message for PR references and submit svn checkin
# info to gnats audit-trail.
#
# WARN: It doesn't know if a particular SVN revision has already been
# WARN: updated or not in a gnats PR.
#
# Usage: $0 -revision xxxxx
#
# Usage: Standalone version of this script is at src/tools/build on trunk
#
# Contact: Prakash D or hnd-software-scm-list team if there are any
# Contact: questions

use       Getopt::Long;
use       strict;

my $OK=0;         # Used for exit code
my $ERROR=1;      # Used for exit code

# Cmd line arg details
my $revision;     # SVN revision number (required)
my $repo_path;    # SVN Repository path (optional, default is production repo)
my @mailto;       # Additional emails, not yet implemented (optional, not implemented)
my $dbg=0;        # Enable debug/verbose output (optional)
my $hook_run=0;   # Invoke in hook context (optional, not implemented)
my $manual_run=1; # Invoke in manual context (optional, not implemented)
my @new_prs;      # Additional PRs can be specified on cmd line (optional)
                  # Useful when commit has wrong PR or if PR spec is wrong

# Command line args
my %opts = (
           'dbg'        => \$dbg,
           'hook_run'   => \$hook_run,
           'mail=s'     => \@mailto,
           'manual_run' => \$manual_run,
           'new_prs=s'  => \@new_prs,
           'repo_path=s'=> \$repo_path,
           'revision=s' => \$revision,
);

# Scan the command line arguments
GetOptions(%opts) or die "ERROR: $0: Error processing command line args";

# -revision is mandatory, so ensure it has been specified
if (!defined($revision)) {
    print "ERROR: Missing SVN Revision number\n\n";
    print "ERROR: Usage: $0 -revision <svn-rev>\n";
    print "ERROR:           [-new_pr <new-gnats-pr-to-update>]\n";
    print "ERROR:           [-repo_path <svn-repo>]\n";
    exit($ERROR);
} else {
    # Take out leading r in revision number
    $revision =~ s/^r//g;
}

$hook_run   = $manual_run ? "0" : "1";
$manual_run = $hook_run   ? "0" : "1";

# Get a current user's login name
my $current_login = getlogin || (getpwuid($<))[0] || "nobody";

# Get a current time stamp
my @now = localtime(time());
my $current_time = sprintf("%d-%.2d-%.2d %.2d:%.2d:%.2d",$now[5]+1900, $now[4]+1, $now[3], $now[2], $now[1], $now[0]);
my $current_session = "$current_time; $current_login";

print "[$current_session] >>> Debug mode enabled\n" if ($dbg);
print "[$current_session] >>> PATH=$ENV{PATH}\n"  if ($dbg);

$ENV{'GNATSDB'} = '::HND_WLAN:hwnbuild:hndwlan';

my $gnats_mail    = 'gnats4-hnd-wlan@broadcom.com';
my $gnats_cgi     = "http://gnats-irva-3.broadcom.com/cgi-bin/gnatsweb.pl";
my $query_pr      = '/projects/hnd/tools/linux/bin/query-pr';
# svnlook used to lookup $rev in $repo_path can be changed to 'svn log ...'
my $svnlook       = '/tools/bin/svnlook';
my $rb_url        = "http://wlan-rb.sj.broadcom.com/r/__rb_id__/";

my $svnroot_path;
my $repo_path_prod = "/projects/wlansvnroot/production_repository/wlansvn/";
my $repo_path_demo = "/projects/wlansvnroot/demo_repositories/svndemo";
$svnroot_path      = $repo_path ? $repo_path : $repo_path_prod;

$gnats_mail   = "${current_login}\@broadcom.com" if ($dbg);;

# ViewVC links
my $viewvcsvn_root;
if ( $svnroot_path eq "$repo_path_prod" ) {
   $viewvcsvn_root = "http://svn.sj.broadcom.com/viewvc/wlansvn";
} else {
   $viewvcsvn_root = "http://svn.sj.broadcom.com/viewvc/wlansvndemo";
}

my $viewvcsvn_rev  = "${viewvcsvn_root}?view=revision&revision=__revision__";
my $viewvcsvn_diff = "${viewvcsvn_root}/__file__?r1=__rev1__&r2=__rev2__&pathrev=__rev1__";
my $viewvcsvn_dir  = "${viewvcsvn_root}/__file__?pathrev=__rev__";
my $viewvcsvn_new  = "${viewvcsvn_root}/__file__?view=markup&pathrev=__rev__";
my $viewvcsvn_del  = "${viewvcsvn_root}/__file__?view=log&pathrev=__rev__";
my $gnats_url      = "http://gnats-irva-3.broadcom.com/cgi-bin/gnatsweb.pl?cmd=view%20audit-trail&database=HND_WLAN&pr=__pr__";

my $sendmail;   # Sendmail util
my $viewvc_url; # Derived viewvc url link
my %files;      # svnlookup changed files for given svnrev
my $author;     # svnlookup author for given svnrev
my $date;       # svnlookup date for given svnrev
my $commit_msg; # svnlookup commit message for given svnrev
my $gnats_msg;  # Mail body to gnats audit trail

# Temporary vars
my $file;
my $pr;
my %prs = ();
my @prs;
my %reviews = ();
my @reviews;


# 'A ' Item added to repository
# 'D ' Item deleted from repository
# 'U ' File contents changed
# '_U' Properties of item changed; note the leading underscore
# 'UU' File contents and properties changed

sub Dbg { my $msg = shift; print "DBG: $msg\n" if ($dbg); }

# Lookup svn changeset for given revision
sub svnlook_files {
       my $rev = shift;
       my $svnlook_cmd = "$svnlook changed -r $rev $svnroot_path";
       my $stat;
       my $file;

       &Dbg("svnlook_files: svnlook_cmd = $svnlook_cmd");

       open(SVNLOOK, "$svnlook_cmd | ") ||
                die "ERROR: $0: Could not run $svnlook_cmd: $!\n";

       while (<SVNLOOK>) {
             chomp;
             next if ($_ =~ /^\s*$/);
             ($stat, $file) = ($_ =~ m/([^ ]*)\s+(.*)/g);

             # Handle white space in file names
             $file =~ s/\s/%20/g;

             &Dbg("svnlook_files(): stat=$stat; file=$file");
             $files{$file} = $stat;
       }
       close(SVNLOOK);
}

sub svnlook_log {
       my $rev = shift;
       my $svnlook_cmd = "$svnlook log -r $rev $svnroot_path";

       &Dbg("svnlook_log(): svnlook_cmd = $svnlook_cmd");
       open(SVNLOOK, "$svnlook_cmd | ") ||
                die "ERROR: $0: Could not run $svnlook_cmd: $!\n";

       while (<SVNLOOK>) {
             $commit_msg .= $_;
       }
       close(SVNLOOK);
}

# Conditionally run routine
sub svnlook_author {
       my $rev = shift;
       my $svnlook_cmd = "$svnlook author -r $rev $svnroot_path";

       &Dbg("svnlook_author(): svnlook_cmd = $svnlook_cmd");
       $author = qx($svnlook_cmd); chomp($author);

       return($author);
}

# Conditionally run routine
sub svnlook_date {
       my $rev = shift;
       my $svnlook_cmd = "$svnlook date -r $rev $svnroot_path";

       &Dbg("svnlook_date(): svnlook_cmd = $svnlook_cmd");
       $date = qx($svnlook_cmd); chomp($date);

       return($date);
}

# utility routines for creating links
sub viewvc_link {
    my ($file, $rev) = @_;
    my ($stat, $rev1, $rev2);

    $stat = $files{$file};

    if ($stat =~ /U/) {

       $viewvc_url = $viewvcsvn_diff;
       $rev1 = $rev;
       $rev2 = $rev - 1;

       &Dbg("viewvc_link(): viewvcsvn_diff $file $rev1 $rev2");

       $viewvc_url =~ s/__file__/$file/g;
       $viewvc_url =~ s/__rev1__/$rev1/g;
       $viewvc_url =~ s/__rev2__/$rev2/g;

    } elsif ($stat =~ /A/) {

       # For new dirs with trailing /, viewvc link differs
       if ($file =~ /\/$/) {
          $viewvc_url = $viewvcsvn_dir;
       } else {
          $viewvc_url = $viewvcsvn_new;
       }

       &Dbg("viewvc_link(): viewvcsvn_new $file $rev");

       $viewvc_url =~ s/__file__/$file/g;
       $viewvc_url =~ s/__rev__/$rev/g;

    } elsif ($stat =~ /D/) {

       my $prev_rev = $rev - 1;

       $viewvc_url = $viewvcsvn_del;

       &Dbg("viewvc_link(): viewvcsvn_del $file $rev");

       $viewvc_url =~ s/__file__/$file/g;
       $viewvc_url =~ s/__rev__/$prev_rev/g;
    }

    return ("$viewvc_url");
}

&svnlook_log($revision);

# turn off setgid
$) = $(;

# find a suitable mail program
$sendmail = "/usr/sbin/sendmail";
$sendmail = "/usr/lib/sendmail" unless -x $sendmail;
$sendmail = "/usr/bin/sendmail" unless -x $sendmail;
die "ERROR: $0: Can't find executable for Mail program\n" unless -x $sendmail;

$gnats_msg   = '';
$commit_msg  =~ s/^\n+//s;		# remove leading newlines
$commit_msg  =~ s/\n+$//s;		# remove extra trailing newlines
$commit_msg .= "\n";

# search the msg for PR number(s), up to 3 per "PR" string or 5 total.
# examples of parsed strings:
#	PR- 12345
#	PR 1111, 2222; 3333
#	PRs: 3333 4444

foreach ($commit_msg =~ m@
	 	\b(?:pr|prs|gnats|war)
	 	[-#:/\s]*
	 	(\d+)
	 	(?:[,;\s]+(\d+))?
	 	(?:[,;\s]+(\d+))?
	 	(?:[,;\s]+(\d+))?
	 	(?:[,;\s]+(\d+))?
	 @igsx) {
    $prs{$_} = 1 if /^\d{5,6}$/;	# skip the non-numeric matches
}

@prs = sort {$a <=> $b} keys %prs;
# Override PR list, only if a PR is specified on cmd line
@prs = grep { /^\d{5,6}$/ } @new_prs if (@new_prs);

print "DBG : [$current_session] Rev $revision; Gnats PRs found: @prs\n";

# some protection from bad input
@prs = () if @prs > 10;
@prs = grep { $_ >= 10000 && $_ <= 999999 } @prs;

exit($OK) if (!@prs);

# Search the msg for RB id number(s) only if pr is going to be updated
foreach ($commit_msg =~ m@
	 	\b(?:RB)
	 	[-#:/\s]*
	 	(\d+)
	 	(?:[,;\s]+(\d+))?
	 	(?:[,;\s]+(\d+))?
	 	(?:[,;\s]+(\d+))?
	 	(?:[,;\s]+(\d+))?
	 @igsx) {
    $reviews{$_} = 1 if /^\d{3,6}$/;	# skip the non-numeric matches
}

@reviews = sort {$a <=> $b} keys %reviews;

print "DBG : [$current_session] Rev $revision; RB IDs found: @reviews\n" if (@reviews);

&svnlook_files($revision);

if (!keys %files) {
   print "ERROR: No files were found in SVN revision: $revision\n";
   exit($ERROR);
}

# For performance these can be turned off conditionally
$author = &svnlook_author($revision);
$date   = &svnlook_date($revision);

if (!$author) {
   &Dbg("svnlook_author() return null author");
   print "WARN: [$current_session] svnlook_author() FAILED for $revision. Usinig $current_login\n";
   $author = $current_login;
}

if (!$date) {
   &Dbg("svnlook_date() return null date");
   print "WARN: [$current_session] svnlook_date() FAILED for $revision. Using $current_time\n";
   $date = $current_time;
}

# Format the gnats message
$viewvcsvn_rev =~ s/__revision__/$revision/;

$gnats_msg .= "\n";
$gnats_msg .= "Date    : $date\n";
$gnats_msg .= "Author  : $author\n";
$gnats_msg .= "Revision: $viewvcsvn_rev\n";
$gnats_msg .= "Note    : PR manual update by $current_login\n" if ($current_login  !~ /^(svn|automrgr)$/);

# Display any review board IDs in commit message. No validation is done
# on review board IDs. This is informationational to cross reference
foreach my $rbid (@reviews) {
   my $this_rb_url = $rb_url;

   $this_rb_url =~ s/__rb_id__/$rbid/g;
   $gnats_msg .= "ReviewID: $this_rb_url\n";
}

$gnats_msg .= "\nCommit Message:\n";
$gnats_msg .= $commit_msg;

my $first = 1;
my $index = 1;
foreach my $file (sort keys %files) {
   my $filename;

   next if ($file =~ /^\s*$/);

   ($filename) = (split(/\//,$file))[-1];

   if ($first) {
      $gnats_msg .= "\nDIFF LINKS:\n";
      undef $first;
   }

   $gnats_msg .= "\t$filename: " . &viewvc_link("$file", $revision) . "\n";
}

# Returns an array triple (category, number, synopsis)
sub lookup_pr {
    my $pr = shift;
    my $query_fmt = '"%s,%d,%s" Category Number Synopsis';
    return "" if !open(QPR, "$query_pr --format '$query_fmt' $pr 2>&1 |");
    chomp (my $data = <QPR>);
    close QPR;
    my @rv = ($data =~ m/^([^,]+),([^,]+),(.+)$/);
    return @rv;
}

my $skip_pr = 1 if !$gnats_mail;

# send mail to automated GNATS parser, to be appended to audit trail
if (!$skip_pr) {
    foreach my $pr (@prs) {
        my @info = &lookup_pr($pr);
        my $gnats_url_pr = $gnats_url;
        $gnats_url_pr    =~ s/__pr__/$pr/g;
        &Dbg("PR=$pr; GNATS LINK: $gnats_url_pr");
        if (@info == 3) {
            open(MAIL, "| $sendmail $gnats_mail") ||
                die "ERROR: $0: Could not run $sendmail: $!\n";
            print MAIL "From: ${author}\@broadcom.com\n";
            print MAIL "To: $gnats_mail\n";
            print MAIL "Subject: Re: $info[0]/$info[1]: $info[2]\n";
            print MAIL "\n";
            print MAIL $gnats_msg;
            print MAIL "\n";
            print MAIL "GNATS LINK: $gnats_url_pr\n";
            print MAIL "\n";
            print MAIL "-" x "70";
            close(MAIL);
            die "ERROR: [$current_session] $sendmail FAILED updating PR $pr for svn $revision" if $?;
            if ($dbg) {
                print "INFO: [$current_session] Sending to $current_login PR $pr details for rev $revision by $author\n";
            } else {
                print "INFO: [$current_session] Updating PR $pr for rev $revision by $author\n";
            }
        } else {
            print "WARN: [$current_session] Referenced Gnats PR $pr not found\n";
            exit($ERROR);
        }
    }
}

exit($OK);
