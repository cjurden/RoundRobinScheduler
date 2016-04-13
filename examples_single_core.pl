#!/usr/bin/perl

# EECS678
# Adopted from CS 241 @ The University of Illinois

for $file (<examples/*>){
	if( $file =~ /proc(\d+)-c1-(\w+)\.out/){
	#	print "Proc $1 CORE $2 Proc $3\n";
		`./simulator -c 1 -s $2 examples/proc$1.csv | tail -7 > output1`;
		`tail -7 $file > output2`;
		$diff = `diff output1 output2`;
		if($diff){
			print "Test file $file differs\n$diff";
		}
	}
}
#cleanup
`rm output1 output2`;
