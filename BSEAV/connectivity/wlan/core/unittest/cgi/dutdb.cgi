#!/usr/bin/perl -w
use strict;
use CGI qw(:standard -xhtml -nosticky);
use CGI::Carp qw(fatalsToBrowser warningsToBrowser);

$CGI::POST_MAX=1024 * 100;  # max 100K posts
$CGI::DISABLE_UPLOADS = 1;  # no uploads

my $caption = "UTF Devices under test";

if (!defined(param('sql')) || param('sql') eq "") {
    param('sql' => "* from dut order by name");
}

print header();
print start_html(-title=>$caption);
print h1($caption)."\n";
warningsToBrowser(1);

print '<script type="text/javascript" src="sorttable.js"></script>'."\n";

print start_form;
print table({border=>0, summary=>'Form'},
	    [Tr([td('SQL: SELECT '.textfield(-name=>'sql', -size=>80)).
		 td(submit)]),
	     Tr([td([a({-href=>self_url}, 'Bookmark')])])]);

print end_form."\n";

open(M, "mysql -H -h sr1end01 -u query --password=query -e 'select ".param('sql')."' utf 2>&1|") ||
    die "$!";
my $count = 0;
while (<M>) {
# fix up XHTML violations

    s!<TABLE !<TABLE summary="report" class="sortable" !;
    s!TABLE!table!g;
    s!BORDER=(\d)!border="$1"!g;
    s!TR!tr!g;
    s!TH!th!g;
    s!TD!td!g;
    s!http:/w!http://w!g; # fix up previous bug
    s!http://[^<]*!<a href="$&">link</a>!g;
    $count=s!<tr!\n<tr!g;
    $count--; # subtract header
    print p("$count rows in set")."\n";

    print $_;
}
close M;
print "\n";

print p(a({-href=>"http://tidy.sourceforge.net"},
	  img({src=>"checked_by_tidy.gif",
	       alt=>"Checked by Tidy!", border=>"0",
	       height=>"32", width=>"32", align=>'right'}))),"\n";

print end_html;
exit;

