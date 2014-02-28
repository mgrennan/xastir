#!/usr/bin/env perl
#
# $Id: pos2shp.pl,v 1.7 2012/11/01 18:57:19 we7u Exp $
#
#  Copyright (C) 2006-2012 Tom Russo

#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#  see file COPYING for details

#--------------------------------------------------------------------------
# This script produces an ESRI point shapefile from an APRS overlay
# file (*.pos), according to the "Rolling your own shapefile maps"
# section of README.MAPS The point file will display using the TIGER
# Landmark Point dbfawk file.
#
# This enables fast generation of point maps in Shapefile format
# from APRS overlay files.
#
# Typical usage:  
#   pos2shp.pl file.pos myshape
#--------------------------------------------------------------------------


if ($#ARGV != 1)
{
    print "Usage: $0 <file.pos> <shapefile base name>\n";
    exit 1;
}


open(INOBJ,"<$ARGV[0]") || die "Cannot open input overlay file $ARGV[0]\n";

$cmd[0]="shpcreate $ARGV[1] point";
$cmd[1]="dbfcreate $ARGV[1] -n ID 8 0 -s CFCC 4 -s NAME 30";
$outfile=$ARGV[1];




foreach $command (@cmd)
{
    system($command);
    if ($? == -1) 
    {
        print "failed to execute: $!\n";
    }
    elsif ($? & 127) 
    {
        printf "child died with signal %d, %s coredump\n",
        ($? & 127),  ($? & 128) ? 'with' : 'without';
    }
    elsif ($?&0xF0)
    {
        printf "child exited with value %d\n", $? >> 8;
    }
}


# We now have the shapefile and dbf file created, start populating from the
# overlay file:

$i=0;
$first_line = 1;
while (<INOBJ>)
{
    chomp($_);

    # Skip the first line if it starts with a '*' (comment line).
    next if ($first_line && (substr($_,0,1) eq '*') );
    $first_line = 0;

    $temp = $_;
    @bits = split('!', $temp);

    # Sanity check --- don't try to convert if the line doesn't
    # conform to what we expect.  We should have two items in the
    # array at this point.
    next if (length(@bits) < 1);

    $name2 = $bits[0];
    $rest = $bits[1];

    $name = substr($name2,0,9); # Chop at 9 chars
    $name =~ s/\w+\s+$//; # Remove trailing spaces

    $lat=substr($rest,0,8);
    $symtab=substr($rest,8,1);
    $long=substr($rest,9,9);
    $sym=substr($rest,18,1);

# TODO: Do something with the comment field?  It appears that if we
# add too much here we get strange results when displayed in Xastir.
    $comment=substr($rest,19,100);
#    $name = $name . ":" . $comment;

    $i++;  # bump the ID number so every point has a unique one

    $lat_deg=substr($lat,0,2);
    $lat_min=substr($lat,2,5);
    $lat_hem=substr($lat,7,1);

    $long_deg=substr($long,0,3);
    $long_min=substr($long,3,5);
    $long_hem=substr($long,8,1);

    $lat=$lat_deg+$lat_min/60;
    $lat *= -1 if ($lat_hem eq "S");

    $long=$long_deg+$long_min/60;
    $long *= -1 if ($long_hem eq "W");

    # Construct symbol
    
    if ($symtab ne "/" && $symtab ne "\\")
    {
        print "overlay symbol, symtab is $symtab\n";
        $overlay=$symtab;
        $symtab="\\";
        print " reset values symtab is $symtab, overlay is $overlay\n";
    } 
    else
    {
        $overlay=" ";
    }

    $cmd[0]="shpadd $outfile $long $lat";
    $cmd[1]="dbfadd $outfile $i \'X$symtab$sym$overlay\' $name";
    print $cmd[1]."\n";
    foreach $command (@cmd)
    {
        system($command);
        if ($? == -1) 
        {
            print "failed to execute: $!\n";
        }
        elsif ($? & 127) 
        {
            printf "child died with signal %d, %s coredump\n",
            ($? & 127),  ($? & 128) ? 'with' : 'without';
        }
        elsif ($?&0xF0)
        {
            printf "child exited with value %d\n", $? >> 8;
        }
    }
    
}


