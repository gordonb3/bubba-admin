#!/usr/bin/perl

use strict;
use IPC::Open3;
use XML::Simple;
$XML::Simple::PREFERRED_PARSER = 'XML::Parser';

# Gordon: 2015-06-19 - eth0 should not be hardcoded as WAN interface
#use constant WAN_IF=>"eth0";
use constant IPTABLES=>"/sbin/iptables";
use constant IPTABLES_XML=>"/usr/bin/iptables-xml";
use constant DEBUG=>0;

use vars qw($ruleset);


# Gordon: 2015-06-19 - function to retrieve WAN interface
#                      assume default route for now
sub get_wanif {
	my($wtr, $rdr, $err, $if);
	$err = 1; # we want to dump errors here

	my $pid = open3($wtr, $rdr, $err,"/bin/ip route get 128");
	my @data=<$rdr>;
	my @err=<$err>;
	waitpid($pid,0);

	foreach (@data){
		if( /dev (\w+)/ ){
			$if = $1;
		}
	}
	return $if;
}


sub d_print {

	if(DEBUG) {
		print @_;
	}
}

sub get_ifinfo {
	my $IF = shift;

	my($in, $out, $err);
	$err = 1; # we want to dump errors here

	my $pid = open3($in, $out, $err,"/bin/ifconfig " . $IF);

	my $lines=join("",<$out>);
	waitpid($pid,0);
	my @if;

	# get the IP address
	if( $lines =~ m/inet (\d+\.\d+\.\d+\.\d+)/) {
		$if[0] = $1;
	}

	# get the netmask
	if($lines =~ m/netmask (\d+\.\d+\.\d+\.\d+)\s/) {
		$if[1] = $1;
	}
	return @if;

}




sub netmask2net {
	my $netmask = shift;
	my @octs = split('\.',$netmask);
	my $net_bin;
	foreach my $oct (@octs) {
		my $str = unpack("B32", pack("N", $oct));
		$str =~ s/^0+(?=\d)//;   # otherwise you'll get leading zeros
		$net_bin .= $str;
	}
	$net_bin =~ s/0+$//;   # remove trailing zeros
	return length($net_bin);
}

sub exec_cmd { # only the command as argument.

	my $cmd = shift;
	my($wtr, $rdr, $err);
	$err = 1; # we want to dump errors here
	my $pid = open3($wtr, $rdr, $err,$cmd);
	my @out=<$rdr>;
	waitpid($pid,0);

	return @out;
}


sub do_listrules{

	my($wtr, $rdr, $err);
	$err = 1; # we want to dump errors here

	my $pid = open3($wtr, $rdr, $err,IPTABLES." -nL");
	my @data=<$rdr>;
	my @err=<$err>;
	waitpid($pid,0);

	$err = 1; # we want to dump errors here
	$pid = open3($wtr, $rdr, $err,IPTABLES." -t nat -nL");
	my @nat=<$rdr>;
	@err=<$err>;
	waitpid($pid,0);

	push(@data,@nat);

	my $state="START";
	my $rest;
	my $dport;
	my $chain;

	foreach (@data){

		if( /^Chain (\w+)/ ){
			$state="NAME";
			$chain = $1;
			next;
		}
		if($state eq "NAME") {
			if( /^target/ ){
				$state="RULES";
				next;
			}
		}
		if($state eq "RULES") {
			#look for "general" rules.
			if( /^([\w\.]+)\s+([\w\d]+)\s+(--)\s+([\d\.\/]+)\s+([\d\.\/]+)([\w\s\d\.\:]+dpts*:[\w\s\d\.\:]+)/ ){
				print "chain=$chain\ttarget=$1\tprotocol=$2\topt=$3\tsource=$4\tdestination=$5";
				$rest = $6;
				$rest =~ /dpts*:(\d+[\:\d]*)/;
				if ($1) {
					$dport=$1;
					print "\tdport=$dport";
				}
				if (/to:([\d\.-]+):*([\d\:]*)/) {
					print "\tto_ip=$1\tto_port=$2";
				}
				print "\n";
				next;
			}
			#look for "ping" response on WAN UPDATE RULE DETECTION!
			#set destination port to "ping" since icmp does not have portnumbers.
			if( /icmp type 8/ ){
				print "chain=$chain\ttarget=ACCEPT\tprotocol=icmp\topt=0\tsource=0\tdestination=0\tdport=ping\n";
				next;
			}

			if ( /^\s*$/) {
				$state="START";
			}
		}
	}
	return $_;

}

sub print_ruleset {

	print create_xml(1);

}

sub create_xml {

	my $cmd;
	my $print_output = shift;
	if ($print_output) {
		$cmd = IPTABLES ."-save | ". IPTABLES_XML;
	} else {
		$cmd = IPTABLES ."-save | ". IPTABLES_XML ." > /root/.fwc_xml";
	}
	return exec_cmd($cmd);

}

sub get_ruleset {

	if(!create_xml(0)) {
		if(!$ruleset) { #only create rulset once.
			$ruleset = XMLin("/root/.fwc_xml",forcearray => ['rule']);
		}
		return $ruleset;
	} else {
		print "Error creating fw-config structure\n";
		return 0;
	}
}

sub save_rules {

	my $cmd = IPTABLES . "-save > /etc/bubba/firewall.conf";

	my($wtr, $rdr, $err);
	$err = 1; # we want to dump errors here

	my $pid = open3($wtr, $rdr, $err,$cmd);
	waitpid($pid,0);
}


sub check_port { # args are port,protocol,source IP,table,chain,local-ip ( source as 10.10.10.1/24)

	my ($port,$prot,$source,$table,$chain,$localip) = @_;

	my $ruleindex=1;
	my @matched_rules;
	my $source_match;
	my $localip_match;

	my $ruleset = get_ruleset();

	d_print("TEST input: $port,$prot,$source,$table,$chain \n");

	# get all rules in the chain affected
	my $rules = $ruleset->{table}->{$table}->{chain}->{$chain}->{rule}; # this is an array with containing hash refs.

	foreach my $h_rule (@$rules) {
		$source_match = 1;
		$localip_match = 1;
		my $h_conditions = $h_rule->{conditions};
		my $h_actions = $h_rule->{actions};

		if($source) {
			if(! $source eq "*") { # if "*" dont care about the source match.
				$source_match = ($h_conditions->{match}->{s} =~ m/$source/)
			}
		} else {
			# no source may exist in the rule
			if($h_conditions->{match}->{s}) {
				$source_match = 0;
			}
		}
		if($localip) {
			$localip_match = ($h_conditions->{match}->{d} =~ m/$localip/)
		} else {
			# no destination may exist in the rule
			if($h_conditions->{match}->{d} && !($chain eq "Bubba_DNAT") ) {
				$localip_match = 0;
			}
		}
		d_print("LOCALIP: $localip, MATCH RULE: $localip_match\n");
		if( ($h_conditions->{match}->{p} =~ m/$prot/) &&  $source_match && $localip_match) {
			d_print("Protocol match\n");
			my $rule_startport;
			my $rule_endport;
			my $startport;
			my $endport;

			my $type = "dport";
			if ($prot eq 'icmp') {
				$type = "icmp-type";
			}

			$h_conditions->{$prot}->{$type} =~ m/(\d+):*(\d*)/;
			$rule_startport = $1;
			if ($2) {
				$rule_endport = $2;
			} else {
				$rule_endport = $1;
			}

			$port=~ m/(\d+):*(\d*)/;
			$startport = $1;
			if ($2) {
				$endport = $2;
			} else {
				$endport = $1;
			}

			if($startport > $endport) {
				my $tmp = $endport;
				$endport = $startport;
				$startport = $tmp;
			}

			if( ($startport >= $rule_startport && $startport <= $rule_endport) || ($endport >= $rule_startport && $endport <= $rule_endport) || ($startport<$rule_startport && $endport > $rule_endport) ) {
				d_print "Port $port ($startport:$endport) is in use.\n";
				push(@matched_rules,$ruleindex);
			}

		}
		$ruleindex++;
	}
	return @matched_rules;
}

sub do_add_portforward {  # args are port,protocol,source IP, local IP, local port, netmask, serverip

	my $port 		  = $ARGV[1];
	my $prot 		  = $ARGV[2];
	my $source 	  = $ARGV[3];
	my $ip        = $ARGV[4];
	my $localport = $ARGV[5];
	my $netmask   = netmask2net($ARGV[6]);
	my $serverip  = $ARGV[7];

	my @nat_rules = check_port($port,$prot,$source,"nat","Bubba_DNAT",0); # destination IP is of interest here.
	my @filter_rules = check_port($localport,$prot,$source,"filter","Bubba_FWD",$ip);
	my @input_rules = check_port($port,$prot,$source,"filter","Bubba_IN",$ip);
	my @POSTROUTE_rules = check_port($localport,$prot,"*","nat","Bubba_SNAT",$ip);

	my @if = get_ifinfo(get_wanif());

	if(@nat_rules || @filter_rules || @input_rules || @POSTROUTE_rules) {
		print( "Confliction with existing rules\n");
		if(@nat_rules) {
			d_print( " Rule ");
			foreach my $rule (@nat_rules) {
				d_print( "$rule ");
			}
			d_print( " in NAT->Bubba_DNAT\n");
		}
		if(@filter_rules) {
			d_print( " Rule ");
			foreach my $rule (@filter_rules) {
				d_print( "$rule ");
			}
			d_print( " in FILTER->Bubba_FWD\n");
		}
		if(@POSTROUTE_rules) {
			d_print( " Rule ");
			foreach my $rule (@POSTROUTE_rules) {
				d_print( "$rule ");
			}
			d_print( " in NAT->Bubba_SNAT\n");
		}
		if(@input_rules) {
			d_print( " Rule ");
			foreach my $rule (@input_rules) {
				d_print( "$rule ");
			}
			d_print( " in FILTER->Bubba_IN\n");
		}
	} else {
		my $exec_retval;
		my $ret;

		d_print "Port $port ok to forward\n";
		# Create prerouting rule
		my $cmd = IPTABLES . " -t nat -A Bubba_DNAT -p $prot -d " . $if[0] . "/32 --dport $port";
		if ($source) {
			$cmd .= " -s $source";
		}
		$cmd .= " -j DNAT --to-destination $ip:$localport";
		d_print("PREROUTING RULE: " . $cmd ."\n");

		if ($exec_retval = exec_cmd($cmd)) {
			$ret .= $exec_retval;
		}

		# Create forward rule
		if($port =~ m/(\d+):(\d+)/) { # portrange
			$localport = $localport . ":" . ($localport+($2-$1));
		}
		$cmd = IPTABLES." -A Bubba_FWD -p $prot -d $ip --dport $localport";
		if ($source) {
			$cmd .= " -s $source";
		}
		$cmd .= " -j ACCEPT";
		d_print("FORWARD RULE: ".$cmd."\n");
		if ($exec_retval = exec_cmd($cmd)) {
			$ret .= $exec_retval;
		}

		# Create POSTROUTING rule
		# local port range already fixed in FORWARD rule.
		$cmd = IPTABLES." -t nat -A Bubba_SNAT -p $prot -d $ip --dport $localport --source $serverip/$netmask -j SNAT --to-source $serverip";
		d_print("POSTROUTING RULE: ".$cmd."\n");
		if ($exec_retval = exec_cmd($cmd)) {
			$ret .= $exec_retval;
		}
		print $ret;
		save_rules();
	}
}


sub do_rm_portforward { # args are port,protocol,source IP, local IP, local port

	my $port 		  = $ARGV[1];
	my $prot 		  = $ARGV[2];
	my $source 	  = $ARGV[3];
	my $ip        = $ARGV[4];
	my $localport = $ARGV[5];

	my @nat_rules = check_port($port,$prot,$source,"nat","Bubba_DNAT",0);
	my @filter_rules = check_port($localport,$prot,$source,"filter","Bubba_FWD",$ip);
	my @POSTROUTE_rules = check_port($localport,$prot,"*","nat","Bubba_SNAT",$ip);


	if(@nat_rules && @filter_rules) { # matching rules in both chains

		my $exec_retval;
		my $ret;

		foreach my $rule (reverse(@nat_rules)) {
			my $cmd = IPTABLES." -t nat -D Bubba_DNAT $rule";
			d_print("Remove NAT: $cmd\n");
			if ($exec_retval = exec_cmd($cmd)) {
				$ret .= $exec_retval;
			}
		}

		foreach my $rule (reverse(@filter_rules)) {
			my $cmd = IPTABLES." -t filter -D Bubba_FWD $rule";
			d_print("Remove FORWARD: $cmd\n");
			if ($exec_retval = exec_cmd($cmd)) {
				$ret .= $exec_retval;
			}
		}
		if(@POSTROUTE_rules) {  # do not require the rule to be in the POSTROUTE
			foreach my $rule (reverse(@POSTROUTE_rules)) {
				my $cmd = IPTABLES." -t nat -D Bubba_SNAT $rule";
				d_print("Remove POSTROUTE: $cmd\n");
				if ($exec_retval = exec_cmd($cmd)) {
					$ret .= $exec_retval;
				}
			}
		}
		print $ret;

		save_rules();
	} else {
		print "Error, not matching any rule\n";
		d_print("Rules in PREROUTING: ");
		d_print(@nat_rules);
		d_print("\nRules in FORWARD: ");
		d_print(@filter_rules);
		d_print("\n");
		d_print("Rules in POSTROUTING: ");
		d_print(@POSTROUTE_rules);
		d_print("\n");
	}
}

sub do_openport {  # args are port,protocol,source IP,table,chain, (10.10.10.1/24)

	my $port 		= $ARGV[1];
	my $prot 		= $ARGV[2];
	my $source 	= $ARGV[3];
	my $table 	= $ARGV[4];
	my $chain 	= $ARGV[5];

	my @rules = check_port($port,$prot,$source,$table,$chain,0);

	d_print("OPEN PORT\n");
	if(@rules) {
		print "Confliction with existing rules.\n";
		foreach my $rule (@rules) {
			d_print( " $rule");
		}
		d_print " in table/chain: $table/$chain\n";
	} else {
		d_print( "Port $port ok to open\n");
		my $cmd = IPTABLES . " -t $table -A $chain -p $prot -i " .get_wanif();
		if ($prot eq "icmp") {
			$cmd .= " --icmp-type $port";
		} else {
			$cmd .= " --dport $port";
		}
		if ($source) {
			$cmd .= " -s $source";
		}
		$cmd .= " -j ACCEPT";
		d_print("OPEN: $cmd\n");
		print exec_cmd($cmd);
		save_rules();
	}
}

sub do_closeport { # args are port,protocol,source IP,table,chain (10.10.10.1/24)

	my $port 		= $ARGV[1];
	my $prot 		= $ARGV[2];
	my $source 	= $ARGV[3];
	my $table 	= $ARGV[4];
	my $chain 	= $ARGV[5];

	my @rules = check_port($port,$prot,$source,$table,$chain,0);

	if(@rules) {
		d_print( "Matching rule(s) found.\nDeleting rule(s): ");
		foreach my $rule (@rules) {
			d_print( "$rule ");
		}
		d_print( " in table/chain: $table/$chain\n");
		foreach my $rule (reverse(@rules)) {
			my $cmd = IPTABLES . " -t $table -D $chain $rule";
			d_print( "$cmd\n");
			print exec_cmd($cmd);
		}
		save_rules();
	} else {
		print "No matching rules found\n";
	}

}

sub do_set_lanif {
	use XML::LibXML;
	use File::Temp qw(tempfile);
	my $if = $ARGV[1];
	my $old_if = $if eq 'eth1' ? 'br0' : 'eth1';

	unless( -f '/etc/bubba/firewall.conf' ) {
		system("/sbin/iptables-save > /etc/bubba/firewall.conf");
	}
	my $parser = new XML::LibXML();
	my $file_fw = qx{/usr/bin/iptables-xml /etc/bubba/firewall.conf};
	my $doc = $parser->parse_string( $file_fw );
	foreach my $context( $doc->findnodes("//match/i[. = \"$old_if\"]/text()")->get_nodelist() ) {
		$context->setData( $if );
	}
	my ($fh, $filename) = tempfile();
	$doc->toFH($fh);
	close( $fh );
	system( "/usr/bin/xsltproc /var/lib/bubba/iptables.xslt $filename | /sbin/iptables-restore" );
	unlink( $filename );
	save_rules();
}

# Hash with all commands
# Key 	- name to use when called
# value	- function to be called and number of arguments
my %commands=(
	"rm_portforward"	    => [\&do_rm_portforward,5],
	"add_portforward"	    => [\&do_add_portforward,7],
	"closeport"	           => [\&do_closeport,5],
	"openport"	           => [\&do_openport,5],
	"test_port"					=> [\&test_port,0],
	"print_ruleset"					=> [\&print_ruleset,0],
	"listrules"					=> [\&do_listrules,0],
	"set_lanif"					=> [\&do_set_lanif,1],

#	""	=> [\& ,],
);



if ((scalar(@ARGV))==0) {
	die "Not enough arguments";
}

my $args=scalar @ARGV-1;
my $cmd=$ARGV[0];

if($commands{$cmd} && $commands{$cmd}[1]==$args){
	$commands{$cmd}[0]->(@ARGV[1..$args])==0 or exit 1;
}else{
	if(!$commands{$cmd}){
		die "Command $cmd not found";
	}else{
		die "Invalid parametercount";
	}
}
