#!/usr/bin/perl

my @prefixes = ("KANJI", "KANJI2");
my $postfix1="_result.txt";
my $postfix2="_result.bmp";

foreach my $prefix (@prefixes) {
	my $infile = $prefix.".ROM";

	if (!open(IN, $infile)) {
		print "Open error: $infile\n";
		last;
	}

	binmode(IN);

	my $data;
	my $len = read(IN, $data, 128 * 1024);

	close(IN);


	my $nums = ($len / 32);
	
	my @chrs = ();

	for(my $chr=0; $chr<$nums; $chr++) {
		my $chdata = [];
		for(my $pos=0; $pos<16; $pos++) {
			my $val = unpack("v", substr($data, $chr * 32 + $pos * 2, 2));
			push(@$chdata, $val);
		}
		push(@chrs, $chdata);
	}


	$data = "";

	my $outfile1 = $prefix.$postfix1;
	if (!open(OUT1, "> ".$outfile1)) {
		print "Open error: $outfile1\n";
		last;
	}

	foreach my $chr (@chrs) {
		for(my $n=0; $n<16; $n++) {
			my $data = $chr->[$n];
			my $str = unpack("B16", pack("S", $data));
			$str =~ tr/01/ o/;
			print OUT1 $str."\n";
		}
		print OUT1 "\n";
	}

	close(OUT1);


	my $chrs_per_line = 64;
	my $width = $chrs_per_line * 16;
	my $height = scalar(@chrs) / $chrs_per_line * 16;

	my $outfile2 = $prefix.$postfix2;
	if (!open(OUT2, "> ".$outfile2)) {
		print "Open error: $outfile2\n";
		last;
	}
	binmode(OUT2);
	print OUT2 set_bmp_header($width, $height);

	for(my $cy=0; $cy<(scalar(@chrs) / $chrs_per_line); $cy++) {
		for(my $ll=0; $ll<16; $ll++) {
		for(my $cx=0; $cx<$chrs_per_line; $cx++) {
			my $ch = $cx + $cy * $chrs_per_line;
			my $dt = $chrs[$ch][$ll];
			print OUT2 pack("S", $dt);
		}
		}
	}

	close(OUT2);


}

close(OUT1);
close(IN);
print "Complete.\n";
getc(STDIN);
exit 0;

sub read_file {
	my($filename,$rdata)=@_;

	my $fh;
	my $buf="";;
	my $data="";

	if (!open($fh, $filename)) {
		print "$filename: file not open.\n";
		return 1;
	}
	binmode($fh);
	while(read($fh, $buf, 1024)) {
		$data .= $buf;
	}
	close($fh);
	$$rdata .= $data;
	return 0;
}

sub write_file {
	my($filename,$rdata)=@_;

	my $fh;
	my $buf="";;
	my $data="";

	if (!open($fh, "> ".$filename)) {
		print "$filename: file not open.\n";
		return 1;
	}
	binmode($fh);
	print {$fh} $$rdata;
	close($fh);
	return 0;
}

sub check_bmp_header {
	my($rdata)=@_;

	# file header 14 bytes
	if (substr($$rdata,0,2) ne "BM") {
		print "This is not BMP format.\n";
		return 1;
	}
	$offset = unpack("V",substr($$rdata,0x0a,4));
	print "offset:".$offset."\n";

	# info header 
	my $infosize = unpack("V",substr($$rdata,0x0e,4));
	if ($infosize != 40) {
		print "Windows BMP format only.\n";
		return 1;
	}
	my $width = unpack("V",substr($$rdata,0x12,4));
	if ($width != 256) {
		print "width must be 256 pixel.\n";
		return 1;
	}
	my $height = unpack("V",substr($$rdata,0x16,4));
	if ($height != 128) {
		print "height must be 128 pixel.\n";
		return 1;
	}
	my $bpp = unpack("v",substr($$rdata,0x1c,2));
	if ($bpp != 1) {
		print "Supported data is only 1bit per pixel(B/W data).\n";
		return 1;
	}
	my $compress = unpack("V",substr($$rdata,0x1e,4));
	if ($compress != 0) {
		print "Supported data is only no compression.\n";
		return 1;
	}

	return 0;
}

sub set_bmp_header {
	my($width, $height)=@_;

	my $header_size = 14 + 40 + 8;
	my $data_size = $width * $height / 8;
	my $file_size = $data_size + $header_size;

	my $header = "BM";

	# file size
	$header .= pack("V", $file_size);
	$header .= pack("V", 0);
	# offset
	$header .= pack("V", $header_size);

	# bitmap information
	$header .= pack("V", 40);	# header size
	$header .= pack("l", $width);
	$header .= pack("l", -$height);
	$header .= pack("v", 1);	# plane
	$header .= pack("v", 1);	# bit count
	$header .= pack("V", 0);	# compression
	$header .= pack("V", $data_size);	# data size 
	$header .= pack("l", 3780);	# x axis dot per meter
	$header .= pack("l", 3780);	# y axis dot per meter
	$header .= pack("V", 2);	# palettes
	$header .= pack("V", 0);	# important colors

	# palette
	$header .= pack("V", 0x00000000);	# palette 0
	$header .= pack("V", 0xffffffff);	# palette 1

	return $header;
}
