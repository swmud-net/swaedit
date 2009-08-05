#!/usr/bin/perl

use strict;
use File::Basename;
use Data::Dumper;

die "Syntax: $0 <path>\n" if $#ARGV < 0;

my $path = $ARGV[0];
my $f;
my $tmp;
undef $/;

eval
{
	open($f,'<',$path);
	my $content = <$f>;
	close($f);
	open($tmp,'>',dirname($path).'/tmp_'.basename($path));
	print { $tmp } $content;
	close($tmp);
	open($f,'>',$path);
	$content =~ s!<property.*buddy.*\n.*\n.*</property>.*!!g;
	print { $f } $content;
};
die $@ if $@;

