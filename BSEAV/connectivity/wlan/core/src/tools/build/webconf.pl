#!perl
# webconfig.pl
#	This script allow user to configured an AP using webpages without a web browser.
#
# Usage:
#	perl webconfig.pl
#		<config>	--> webpage containing desired values.
#		<destip>	--> "192.168.1.1"
#		<destpage>	--> webpages "index.asp", "firmware.asp" ...
#		<timeout>	--> web time out
#		<pageact>	--> Apply/Cancel/Restore Defaults/Release/Renew
#		<webact>	--> "Verify/Post/Get" "Verify, Post, Get"
#		<verbose>	--> print to STDOUT
#		<debuglog>	--> output debug log
#		<qvtlog>	--> write result to qvt format.
# TODO:
#		<user>		--> username
#		<pass>		--> password
# $Id$
#

	use strict;
	use HTTP::Status;
	use HTTP::Headers;
	use HTTP::Request::Common;
	use LWP::UserAgent;
	use HTML::TokeParser;
	use Getopt::Long;


	sub Prints;
	sub ComparePage;
	sub VerifyUpgrade;
	sub VerifyPost;
	sub FillRequest;
	sub FillPost;
	sub UpgradePost;


	my $srcfile = "../webpages/index-template.asp";
	my $dstaddr = "192.168.1.1";
	my $dstpage = "index.asp";
	my $timeout = 60;
	my $verbose = 0;
	my $debug_log;
	my $qvt_log;
	my $pageact  = "Apply";
	my $webact  = "Post/Get";
	my $qvt_dump;
	my $user    = "";
	my $passwd  = "";
	local *DEBUG_LOG;
	local *QVT_LOG;
	local *QVT_DUMP;

	GetOptions ( 'config=s'		=> \$srcfile,
		     'destip=s'		=> \$dstaddr,
		     'destpage=s'	=> \$dstpage,
		     'timeout=i' 	=> \$timeout,
		     'pageact=s' 	=> \$pageact,
		     'webact=s'		=> \$webact,
		     'verbose+'		=> \$verbose,
	     	     'debuglog=s'	=> \$debug_log,
		     'qvtlog=s'		=> \$qvt_log,
		     'dumplog=s'	=> \$qvt_dump,
	#TODO
	     	     'user=s'		=> \$user,
	     	     'pass=s'		=> \$passwd );



	# iterates webactions
	foreach my $act_item ( split(/[\,|\/]/, $webact) ) {

		my $dsturl 	= "http://$dstaddr/$dstpage";
		my $ua 		= LWP::UserAgent->new( timeout => $timeout );

		Prints "************** ACT ITEM: $act_item ***********\n";
		# Do get action
		if( lc($act_item) eq "get" ){
			my $response = $ua->get( $dsturl );
			if( $response->is_success ){
				Prints "GETting $dsturl\n", "PASS";	
				Prints "GETting $dsturl\n";	

			}elsif( $response->is_redirect ){
				Prints "REDIRECTING (302):\n";
				redo;
			}else{
				Prints "Error GETting $dsturl \n", "FAIL";	
				Prints ("Error GETting $dsturl ".$response->request->uri." -- ".$response->status_line."\n");
				die ("Error while GETting ", $response->request->uri, " -- ",
				$response->status_line, "\nAborting");
			}

			if( defined ($qvt_dump) ){
				open( QVT_DUMP, "> $qvt_dump") || die(" Can't open $qvt_dump, $!\n");
				printf (QVT_DUMP "${$response->content_ref}" );
				close( QVT_DUMP );
			}else{
				Prints "********** GET content *********\n";
				Prints "${$response->content_ref}\n";
			}

		# Do post of desired key/value
		}elsif( lc($act_item) eq "post"){

			my $post_req = FillRequest( $srcfile, $dstaddr, $dstpage, $user, $passwd );

			Prints "********** POST content *********\n";
			#Prints "${$post_req->content_ref}\n";

			my $response = $ua->send_request( $post_req );
			if( $response->is_success ){
				Prints "POSTing $dsturl\n", "PASS";
				Prints "POSTing $dsturl\n";

			}elsif( $response->is_redirect ){
				Prints "REDIRECTING (302): Do a GET to get a proper response to a POST\n";
				Prints "POSTing $dsturl\n", "PASS";
				Prints "POSTing $dsturl\n";
			}else{
				Prints "Error while POSTing $dsturl\n", "FAIL";
				Prints ("Error while POSTing, ".$response->request->uri." -- ".$response->status_line."\n" );
				die "Error while POSTing ", $response->request->uri, " -- ",
				$response->status_line, "\nAborting";
			}

			Prints "********** POST response ************\n";
			Prints "${$response->content_ref}\n";
			sleep 10

		# Do verify of key/value
		}elsif( lc($act_item) eq "verify" ){
			
			my $response = $ua->get( $dsturl );
			if( $response->is_success ){
				Prints "GETting $dsturl\n", "PASS";	
				Prints "GETting $dsturl\n";	
			}elsif( $response->is_redirect ){
				Prints "REDIRECTING\n";
				redo;
			}else{
				Prints "Error GETting $dsturl \n", "FAIL";	
				Prints ("Error GETting $dsturl ".$response->request->uri." -- ".$response->status_line."\n");
				die ("Error while GETting ", $response->request->uri, " -- ",
				$response->status_line, "\nAborting");
			}

			Prints "********** GET content *********\n";
			Prints "${$response->content_ref}\n";

			ComparePage( $srcfile, $response, $dstpage );
		}
	}


	if( fileno(DEBUG_LOG) ){
		close(DEBUG_LOG);
	}

	if( fileno(QVT_LOG) ){
		close(QVT_LOG);
	}

#
# FillRequest
#	initialized a request.
#
sub FillRequest{

	my ($file, $dest, $page) = @_;
	my $req;

	if( $page eq "firmware.asp" ){
		$req = UpgradePost( $file, $dest );
	}else{
		$req = FillPost( $file, $dest );
	}

	$req;
}

#
# UpgradePost
#	Post upgrade content.
#
sub UpgradePost{

	my ($file, $dest) = @_;
	my $input = HTML::TokeParser->new( $file ) || die "Can't open $file";
	my $dstpage;
	my $page;
	my $type;
	my $method;
	my $imagefile;

	Prints "\n ********* Input file UpgradePost ***********\n";
	READ_CONF: while( my $token = $input->get_tag( "form", "input", "select")){

		my ($tag, $att, $attseq, $text) = @$token;
		Prints "token --> [@$token]\n";

		if ($tag eq "input" ){
			if( $att->{name} eq "page" ){
				$page = $att->{value};
			}elsif( $att->{type} eq "file" ){
				$imagefile = $att->{value};
			}

		}elsif( $tag eq "select" ){

		}elsif( $tag eq "form" ){
			Prints "\t form [$att->{method}, $att->{action}, $att->{enctype}]\n";
			$method    = $att->{method};
			$dstpage   = $att->{action};
			$type      = $att->{enctype};

		}else{

		}
	}
	
	my $req = HTTP::Request::Common::POST( "http://$dest/$dstpage",
				Content_Type => $type,
				Content      => [ page => $page,
						file => ["$imagefile"] ]);

	$req;
}


#
# FillPost
# 	Parsing key & value pairs out of a
# 	web page into a 'POST request'.
#
sub FillPost{

	my ($file, $dest) = @_;
	my $input = HTML::TokeParser->new( $file ) || die "Can't open $file";
	my $req = HTTP::Request->new;

	Prints "\n ********* Input file FillPost ***********\n";
	READ_CONF: while( my $token = $input->get_tag( "form", "input", "select")){

		my ($tag, $att, $attseq, $text) = @$token;
		Prints "token --> [@$token]\n";

		if( $tag eq "input" ){

			Prints "\t input [$att->{name}, $att->{value}]\n";

			if( $att->{name} ne "action" ){
				$req->add_content("$att->{name}=$att->{value}&");
			}elsif( ($att->{name} eq "action") and ($att->{value} eq $pageact) ){
				$req->add_content("$att->{name}=$pageact");
			}else{
				#XXX problem?
			}

		}elsif( $tag eq "select" ){

			Prints "\t select [$att->{name}, ";
			$req->add_content("$att->{name}=");

			while( $token = $input->get_tag( "option", "/select" )){
				($tag, $att, $attseq, $text) = @$token;

				if( ($tag eq "option") && ($text =~ /selected/) ){
					Prints "$att->{value}]\n";
					$req->add_content("$att->{value}&");
					next READ_CONF;
				}elsif( $tag eq "/select" ){
					Prints " ]\n";
					$req->add_content ("&");
					next READ_CONF;
				}
			}

		}elsif( $tag eq "form" ){

			Prints "\t form [$att->{method}, $att->{action}]\n";
			$req->method( $att->{method} );
			$req->uri("http://$dest/$att->{action}" );
		}else{
			Prints "*********FALSE TOKEN *********\n"
		}
	}

	$req;
}

#
# ComparePage
#	compared desire value against actual value.
#
sub ComparePage {
	my ($file, $res, $page) = @_;

	if( $page eq "firmware.asp" ){
		VerifyUpgrade( $file, $res );
	}else{
		VerifyPost( $file, $res );
	}
}
	
#
# VerifyUpgrade
#
sub VerifyUpgrade {
	my ($src, $cur) = @_;

	my $in = HTML::TokeParser->new( $src ) || die "Can't open $src\n";
	my $ap = HTML::TokeParser->new( $cur->content_ref ) || die "Can't open AP\n";

	my $a_text;
	my $i_text;
	my $os=0;
	my $boot=0;

	Prints "\n************ Verify Upgrade ************\n";
	# Verify OS version
	while ( (my $ap_token = $ap->get_tag("td"))
			and (my $in_token = $in->get_tag("td")) ){

		$a_text = $ap->get_trimmed_text;
		$i_text = $in->get_trimmed_text;

		if( $a_text ne $i_text ){
			Prints "Mis-match $i_text \n", "FAIL";
		}elsif( ($a_text =~ /^Linux/) or ($a_text =~ /^VxWorks/) ){
			Prints ":OS Version:  $a_text\n", "PASS";
			$os=1;
		}elsif( $a_text =~ /^CFE/ ){
			Prints ":BOOT Version:  $a_text\n", "PASS";
			$boot=1;
		}
		
		if( $os and $boot ){
			last;
		}
	}
	Prints "Upgrade Image \n", "DONE";
}

#
# VerifyPost
#
sub VerifyPost {
	my ($src, $cur) = @_;
	my $in = HTML::TokeParser->new( $src );

	# Verify key/value pair
	VER_POST: while( my $intoken = $in->get_tag( "input", "select" )) {

		my $ap = HTML::TokeParser->new( $cur->content_ref );
		my ($i_tag, $i_att, $i_attseq, $i_text) = @$intoken;
		my $exp_name = $i_att->{name};
		my $exp_value = $i_att->{value};

		# Tag is select, then look for selected option
		if( $i_tag eq "select" ){
			while( $intoken = $in->get_tag( "option", "/select" )) {
				($i_tag, $i_att, $i_attseq, $i_text) = @$intoken;

				if( $i_text=~ /selected/ ){
					$exp_value = $i_att->{value};
					$i_tag = "select";
					last;
				}elsif( $i_tag eq "/select" ){
					Prints "Web syntax error, cant find selected value for [$exp_name]\n", "FAIL";
					next VER_POST;
				}
			}
		}

		# search AP for a given pair of key/value
		while( my $aptoken = $ap->get_tag( $i_tag )) {
			my ($a_tag, $a_att, $a_attseq, $a_text) = @$aptoken;

			if( $i_tag eq "select" ){
				if( ($a_att->{name} eq $exp_name ) ){
					while( $aptoken = $ap->get_tag( "option", "/select" )) {
						($a_tag, $a_att, $a_attseq, $a_text) = @$aptoken;
						if( $a_text =~ /selected/ ){
							if( $a_att->{value} ne $exp_value ){
								Prints "Key/value pair does not match [$exp_name, $exp_value]", "FAIL";
							}
							next VER_POST;
						}elsif( $a_tag eq "/select" ){
							Prints "Web syntax error, cant find selected value for [$exp_name]\n", "FAIL";
							next VER_POST;
						}
					}
				}
			}elsif( $i_tag eq "input" ){
				if( ($a_att->{value} eq $i_att->{value}) ){
					next VER_POST;
				}
			}else{
				Prints "Unknown HTML tag[$i_tag]\n", "FAIL";
				next VER_POST;
			}
		}
		Prints "Cant find tag ($i_tag)[$exp_name, $exp_value]\n", "FAIL";
	}
	Prints "$src Verify\n", "DONE";
}

#
# Prints
#
sub Prints {

	my ($text, $stat) = @_;

	if( $verbose ){
		print "$text";
	}

	# XXX DEBUG_LOG complains about uninit usage with -w
	if( defined ($debug_log) and (fileno(DEBUG_LOG) eq undef) ){
		if( -e $debug_log ){
			open( DEBUG_LOG, ">> $debug_log") || die("Can't open $debug_log, $!\n");
		}else{
			open( DEBUG_LOG, "> $debug_log") || die("Can't open $debug_log, $!\n");
		}
	}

	if( defined ($qvt_log) and (fileno(QVT_LOG) eq undef) ){
		if( -e $qvt_log ){
			open( QVT_LOG, ">> $qvt_log") || die("Can't open $qvt_log, $!\n");
		}else{
			open( QVT_LOG, "> $qvt_log") || die("Can't open $qvt_log, $!\n");
		}
	}

	if( fileno(DEBUG_LOG) ne undef ){
		printf (DEBUG_LOG "$text");
	}

	if( defined($stat) and (fileno(QVT_LOG) ne undef)  ){
		printf (QVT_LOG "$stat: $text" );
	}
}

