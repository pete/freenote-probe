#!/usr/bin/env perl

use IO::Socket;

$socket = IO::Socket::INET->new(LocalPort => 14597, Proto => 'udp');

while(1) {
	$socket->recv($text, 2048);
	print $text;
}
