#!/usr/bin/perl 
# basically flock, modified from source on the web.

use warnings;
use strict;
use Fcntl qw(:flock);

my $lockfile = shift;
my $command = join(" ",@ARGV);

if (!$lockfile || !$command) {
    die("usage: $0 <file> <command>\n");
}

open(FH,$lockfile) || die($!); 
flock(FH,LOCK_EX) || die($!);
my $retval = 0;
system($command) == 0 or $retval = $? >> 8;
flock(FH,LOCK_UN); 
exit($retval);
