#!/usr/bin/perl -W
#
# $Id: get-fcc-rac.pl,v 1.11 2012/11/01 18:57:19 we7u Exp $
#
# Copyright (C) 2000-2012  The Xastir Group
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#
# Updated on 7/5/03 to reflect the new directory structure
# N0VH
#
# Note: Run this script as root in order to write the files into the
# destination directory listed below, or change directory write access.
use File::Basename;
$dirname=dirname($0);
require ($dirname."/values.pl");

my $XASTIR_BASE="${prefix}/share/xastir";


# This script uses temporary storage space in /var/tmp to do its work.
chdir "/var/tmp";


#####################################################################
# Get the RAC database, process it.
#   Download size:  ~2MB
# Final file size: ~13MB
#####################################################################
#

$file  = "amateur.zip";
#$file2 = "amateur.rpt";
$file2 = "amateur.txt";


print STDERR "*********************************\n";
print STDERR "*** Fetching the RAC database ***\n";
print STDERR "*********************************\n";
#`wget -c http://205.236.99.41/%7Eindicatif/download/$file`;
`wget -c http://apc-cap.ic.gc.ca/datafiles/$file`;


if (-e $file && -r $file && -f $file) {

  print STDERR "***********************************\n";
  print STDERR "*** Installing the RAC database ***\n";
  print STDERR "***********************************\n";
  `unzip $file $file2`;
  `mv $file2 $XASTIR_BASE/fcc/AMACALL.LST`;
}

# Remove the RAC download files
unlink $file, $file2;


#####################################################################
# Get the FCC database, process it.
#   Download size:  ~84MB
# Final file size: ~101MB
#####################################################################
#
my $file  = "l_amat.zip";
my $file2 = "EN.dat";

 
print STDERR "*********************************\n";
print STDERR "*** Fetching the FCC database ***\n";
print STDERR "*********************************\n";
`wget -c http://wireless.fcc.gov/uls/data/complete/$file`;

if (-e $file && -r $file && -f $file) {

  my $file_out = "$XASTIR_BASE/fcc/$file2";

  # Get rid of characters "^M^M^J" which are sometimes present, sort
  # the file by callsign & remove old entries for vanity call access.
  print STDERR "*****************************************************\n";
  print STDERR "*** Filtering/sorting/installing the FCC database ***\n";
  print STDERR "*****************************************************\n";

  my %from = ();
 
  open FILE, "unzip -p $file $file2|" or die "Can't open $file2 in $file : $!";
  open FILE_OUT, '|-', "sort -k 5,5 -t \\| -o $file_out" or die "Can't sort $file_out : $!";
  while( <FILE> ) {
    if (/^EN\|(\d+)\|\|\|(\w+)\|.*/) {
      $x = $1;
      $z = $2;
      chop;
      chop;
      $y = $_;
      if (defined $from{$2}) { # check for vanity reassignment
        if ($from{$z} =~ /^EN\|(\d+)\|\|\|(\w+)\|.*/) {
          if ($1 < $x) {
            $replaced++;
            $from{$2} = $y;
          }
        }
      }
      else {
        $from{$2} = $_;
      }
    }
  }
  close FILE;
 
  for my $callsign ( keys %from ) {
    $total++;
    print FILE_OUT "$from{$callsign}\n";
  }
  close FILE_OUT;

  print STDERR "Total callsigns:  " . $total . ".\n";
  print STDERR " Replaced callsigns:  " . $replaced . ".\n";
}

# Remove the FCC download files
unlink $file;




print STDERR "*************\n";
print STDERR "*** Done! ***\n";
print STDERR "*************\n";


