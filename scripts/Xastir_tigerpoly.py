#!/usr/bin/env python
###############################################################################
# $Id: Xastir_tigerpoly.py,v 1.11 2012/11/01 18:57:19 we7u Exp $
#
# Portions Copyright (C) 2004-2012  The Xastir Group
#
# Modified version of GDAL/OGR "tigerpoly.py" script (as described below)
# adapted to assemble information from more tables of the TIGER/Line data
# than had been done by the original, and with the option of dissolving
# common boundaries between areas with identical landmark values.
#
# You must have installed GDAL/OGR, configured to use python in order to use 
# this script
###############################################################################
# 
# Adapted for Xastir use by Tom Russo
#
# NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE
#  When run with the "-d" option, this script changes the topology of 
#  TIGER/Line polygons by dissolving common bounaries between adjacent 
#  polygons with identical Landmark values.  This is done SOLELY for 
#  improvement of on-screen presentation of the shapefiles, and the resulting 
#  files have too much information removed to make them useful for any other 
#  purpose but display in Xastir.
#
#  If any shapefile data produced by this script with the -d option are 
#  distributed publicly, they should have a prominent disclaimer to this 
#  effect.  The data produced by this script with that option bear only a 
#  superficial, graphical resemblence to the  TIGER/Line data from which they 
#  were created.
#
####################Original comments follow
###############################################################################
# tigerpoly.py,v 1.3 2003/07/11 14:52:13 warmerda Exp 
#
# Project:  OGR Python samples
# Purpose:  Assemble TIGER Polygons.
# Author:   Frank Warmerdam, warmerdam@pobox.com
#
###############################################################################
# Copyright (c) 2003, Frank Warmerdam <warmerdam@pobox.com>
# 
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
###############################################################################


import osr
import ogr
import string
import sys

#############################################################################
class Module:

    def __init__( self ):
        self.lines = {}
        self.poly_line_links = {}

#############################################################################
# I dunno, does this initializer make sense?
class Field:
    def __init__( self ):
        self.layer = ''
        self.layerName = ''
        self.field_defn = ''
        self.fld_index = ''

#############################################################################
# We'll use this class to keep track of polygons that get mapped into each 
# other
class PolyMap:
    def __init__( self ):
      self.polyid=''
      self.mappedlist=[]

#############################################################################
def Usage():
    print 'Usage: tigerpoly.py [-d] infile [outfile].shp'
    print
    sys.exit(1)

#############################################################################
# Argument processing.

infile = None
outfile = None
dissolve=0

i = 1
while i < len(sys.argv):
    arg = sys.argv[i]

    if arg == "-d":
        dissolve=1
    
    elif infile is None:
	infile = arg

    elif outfile is None:
	outfile = arg

    else:
	Usage()

    i = i + 1

if outfile is None:
    outfile = 'poly.shp'

if infile is None:
    Usage()

if dissolve == 1:
   print '*******************************************************************'
   print 'WARNING!  WARNING! WARNING!'
   print 'You have chosen to dissolve polygons with identical landmark names/types.'
   print ' Doing this changes the topology of the TIGER/Line data, and renders'
   print ' the resulting shapefiles useful only for display purposes.'
   print ' IF YOU DISTRIBUTE THIS DATA, PLEASE INCLUDE A DISCLAIMER THAT'
   print ' THE DATA HAVE BEEN MODIFIED AND THAT THEIR USE FOR PURPOSES OTHER'
   print ' DISPLAY IS NOT RECOMMENDED!'
   print '*******************************************************************'

#############################################################################
# Open the datasource to operate on.

ds = ogr.Open( infile, update = 0 )

poly_layer = ds.GetLayerByName( 'Polygon' )
pip_layer = ds.GetLayerByName( 'PIP' )
areaLandmarks_layer = ds.GetLayerByName( 'AreaLandmarks' )
Landmarks_layer = ds.GetLayerByName( 'Landmarks' )

#############################################################################
#	Create output file for the composed polygons.

shp_driver = ogr.GetDriverByName( 'ESRI Shapefile' )
shp_driver.DeleteDataSource( outfile )

shp_ds = shp_driver.CreateDataSource( outfile )

shp_layer = shp_ds.CreateLayer( 'out', geom_type = ogr.wkbPolygon )

# Now create a list of all the fields for the DBF file, using the unique
# fields in Polygon, PIP, AreaLandmarks and Landmarks --- eliminate
# all fields of identical names (because those are just the fields that
# link the records to each other).  When we do so, must store layer, field
# index in the hash, and we'll write it back by 

# Create a hash of field definitions indexed by field name.
fields_hash = {}
field_names = []

src_defn = poly_layer.GetLayerDefn()
poly_field_count = src_defn.GetFieldCount()

for fld_index in range(poly_field_count):
    src_fd = src_defn.GetFieldDefn( fld_index )
    fields_hash[src_fd.GetName()] = Field()
    fields_hash[src_fd.GetName()].layer=poly_layer
    fields_hash[src_fd.GetName()].layerName='Polygon'
    fields_hash[src_fd.GetName()].field_defn=src_defn
    fields_hash[src_fd.GetName()].fld_index=fld_index
    field_names.append(src_fd.GetName())
#    print 'Got a Polygon field called %s' % src_fd.GetName()


# now loop over other layers
src_defn = pip_layer.GetLayerDefn()
pip_field_count = src_defn.GetFieldCount()
for fld_index in range(pip_field_count):
    src_fd = src_defn.GetFieldDefn( fld_index )
    try:
      foo = fields_hash[src_fd.GetName()]
#      print ' found pip field %s already in hash' % src_fd.GetName()
    except:
      field=Field()
      fields_hash[src_fd.GetName()] = field
      fields_hash[src_fd.GetName()].layer=pip_layer
      fields_hash[src_fd.GetName()].layerName='PIP'
      fields_hash[src_fd.GetName()].field_defn=src_defn
      fields_hash[src_fd.GetName()].fld_index=fld_index
      field_names.append(src_fd.GetName())
#      print 'Got a pip field called %s' % src_fd.GetName()            

src_defn = areaLandmarks_layer.GetLayerDefn()
areaLandmarks_field_count = src_defn.GetFieldCount()
for fld_index in range(areaLandmarks_field_count):
    src_fd = src_defn.GetFieldDefn( fld_index )
    try:
      foo = fields_hash[src_fd.GetName()]
#      print ' found areaLandmarks field %s already in hash' % src_fd.GetName()
    except:
      field=Field()
      fields_hash[src_fd.GetName()] = field
      fields_hash[src_fd.GetName()].layer=areaLandmarks_layer
      fields_hash[src_fd.GetName()].layerName='AreaLandmarks'
      fields_hash[src_fd.GetName()].field_defn=src_defn
      fields_hash[src_fd.GetName()].fld_index=fld_index
      field_names.append(src_fd.GetName())
#      print 'Got a areaLandmarks field called %s' % src_fd.GetName()            
src_defn = Landmarks_layer.GetLayerDefn()
Landmarks_field_count = src_defn.GetFieldCount()
for fld_index in range(Landmarks_field_count):
    src_fd = src_defn.GetFieldDefn( fld_index )
    try:
      foo = fields_hash[src_fd.GetName()]
#      print ' found Landmarks field %s already in hash' % src_fd.GetName()
    except:
      field=Field()
      fields_hash[src_fd.GetName()] = field
      fields_hash[src_fd.GetName()].layer=Landmarks_layer
      fields_hash[src_fd.GetName()].layerName='Landmarks'
      fields_hash[src_fd.GetName()].field_defn=src_defn
      fields_hash[src_fd.GetName()].fld_index=fld_index
      field_names.append(src_fd.GetName())
#      print 'Got a Landmarks field called %s' % src_fd.GetName()            

# we now have a hash whose keys are all our field names, and from which
# we *should* be able to retreive fields as needed
#print ' we found %d fields' % len(fields_hash)
#print ' the names array has %d names ' % len(field_names)
#for field_name in (field_names):
#  print ' key %s we get field from %s ' % (field_name ,fields_hash[field_name].layerName)

# Now loop over all those field names, create the dbf file definition

for field_name in (field_names):
  src_defn = fields_hash[field_name].field_defn
  src_fd = src_defn.GetFieldDefn( fields_hash[field_name].fld_index )
  fd = ogr.FieldDefn( src_fd.GetName(), src_fd.GetType() )
  fd.SetWidth( src_fd.GetWidth() )
  fd.SetPrecision( src_fd.GetPrecision() )
  shp_layer.CreateField(fd)

print 'Reading Lines'
#############################################################################
# Read all features in the line layer, holding just the geometry in a hash
# for fast lookup by TLID.

line_layer = ds.GetLayerByName( 'CompleteChain' )
line_count = 0

modules_hash = {}

feat = line_layer.GetNextFeature()
geom_id_field = feat.GetFieldIndex( 'TLID' )
tile_ref_field = feat.GetFieldIndex( 'MODULE' )
while feat is not None:
    geom_id = feat.GetField( geom_id_field )
    tile_ref = feat.GetField( tile_ref_field )

    try:
        module = modules_hash[tile_ref]
    except:
        module = Module()
        modules_hash[tile_ref] = module

    module.lines[geom_id] = feat.GetGeometryRef().Clone()
    line_count = line_count + 1

    feat.Destroy()

    feat = line_layer.GetNextFeature()

print 'Got %d lines in %d modules.' % (line_count,len(modules_hash))

#############################################################################
# Now we need to pull in all the PIP, AreaLandmarks and Landmarks features
# and keep them in hashes based on POLYID

#PIP:
feat = pip_layer.GetNextFeature()
polyid_field = feat.GetFieldIndex( 'POLYID' )
pip_hash={}
dest_polyid_hash={}

while feat is not None:
   poly_id = feat.GetField( polyid_field )
   pip_hash[poly_id] = feat
   # initially, every polygon is distinct, with no list of polys mapped into it
   dest_polyid_hash[poly_id]=PolyMap()
   dest_polyid_hash[poly_id].polyid=poly_id
   feat = pip_layer.GetNextFeature()   

print 'Processed %d PIP records.' % len(pip_hash)

#AreaLandmarks
feat = areaLandmarks_layer.GetNextFeature()
polyid_field = feat.GetFieldIndex( 'POLYID' )
areaLandmarks_hash={}

while feat is not None:
   poly_id = feat.GetField( polyid_field )
   areaLandmarks_hash[poly_id] = feat
   feat = areaLandmarks_layer.GetNextFeature()   
print 'Processed %d AreaLandmarks records.' % len(areaLandmarks_hash)


# Landmarks is not looked up by polyid, but by LAND, which links it to 
# Landmarks
feat = Landmarks_layer.GetNextFeature()
land_field = feat.GetFieldIndex( 'LAND' )
Landmarks_hash={}

while feat is not None:
   land_id = feat.GetField( land_field )
   Landmarks_hash[land_id] = feat
   feat = Landmarks_layer.GetNextFeature()   
print 'Processed %d Landmarks records.' % len(Landmarks_hash)

if dissolve == 1:
  print 'Scanning PolyChains for polygons to dissolve.'
  #############################################################################
  #
  # Now we sift through all the PolyChainLink records, find the ones that have
  # distinct left and right polyids, but for which the associated landmark
  # is identical.  We will dissolve those polygons by mapping their polyids
  link_layer = ds.GetLayerByName( 'PolyChainLink' )

  feat = link_layer.GetNextFeature()
  lpoly_field = feat.GetFieldIndex( 'POLYIDL' )
  rpoly_field = feat.GetFieldIndex( 'POLYIDR' )
  
  link_count=0
  ndissolved=0
  
  while feat is not None: 
      link_count = link_count+1
  
      orig_lpoly_id=feat.GetField( lpoly_field )
      orig_rpoly_id=feat.GetField( rpoly_field )
      try:
        lpoly_id = dest_polyid_hash[orig_lpoly_id].polyid
      except:
        lpoly_id = 0
  
      try:
        rpoly_id = dest_polyid_hash[orig_rpoly_id].polyid
      except:
        rpoly_id= 0
   
      #if not already identical (either because already dissolved, or because
      #internal
      if lpoly_id != rpoly_id:
         #find the left and right areaLandmark, if there are any      
         try:
           left_areaLandmark=areaLandmarks_hash[lpoly_id]
           l_landidx=left_areaLandmark.GetFieldIndex('LAND')      
           l_land=left_areaLandmark.GetField(l_landidx)
         except:
           left_areaLandmark=None
  
         try:
           right_areaLandmark=areaLandmarks_hash[rpoly_id]
           r_landidx=right_areaLandmark.GetFieldIndex('LAND')      
           r_land=right_areaLandmark.GetField(r_landidx)
         except:
           right_areaLandmark=None
  
         if left_areaLandmark is not None and right_areaLandmark is not None:
           if l_land == r_land:
              ndissolved = ndissolved+1
              # we have two polys that need to be dissolved
              # we will always do this by copying the left polygon's
              # ID into the right base polygon and every polygon that was
              # mapped to it, then adding the right polygon's
              # ID and any IDs in its map list to the base polygon on the 
              # left.  
              dest_polyid_hash[rpoly_id].polyid=lpoly_id
              for i in dest_polyid_hash[rpoly_id].mappedlist:
                 dest_polyid_hash[i].polyid=lpoly_id
                
              dest_polyid_hash[lpoly_id].mappedlist.append(rpoly_id)
              dest_polyid_hash[lpoly_id].mappedlist.extend(dest_polyid_hash[rpoly_id].mappedlist)
              dest_polyid_hash[rpoly_id].mappedlist=[]
  
      feat.Destroy()
  
      feat = link_layer.GetNextFeature()

  print 'Scanned %d links for polygon dissolve, dissolved %d boundaries.' % (link_count,ndissolved)



#############################################################################
# Read all polygon/chain links and build a hash keyed by POLY_ID listing
# the chains (by TLID) attached to it. 

link_layer = ds.GetLayerByName( 'PolyChainLink' )
link_layer.ResetReading()

feat = link_layer.GetNextFeature()
geom_id_field = feat.GetFieldIndex( 'TLID' )
tile_ref_field = feat.GetFieldIndex( 'MODULE' )
lpoly_field = feat.GetFieldIndex( 'POLYIDL' )
rpoly_field = feat.GetFieldIndex( 'POLYIDR' )

link_count = 0

while feat is not None:
    module = modules_hash[feat.GetField( tile_ref_field )]

    tlid = feat.GetField( geom_id_field )

    orig_lpoly_id=feat.GetField( lpoly_field )
    orig_rpoly_id=feat.GetField( rpoly_field )
    try:
      lpoly_id = dest_polyid_hash[orig_lpoly_id].polyid
    except: 
      lpoly_id = 0

    try:
      rpoly_id = dest_polyid_hash[orig_rpoly_id].polyid
    except:
      rpoly_id = 0

    # sanity checking:
    if lpoly_id != 0:
      if lpoly_id != orig_lpoly_id and len(dest_polyid_hash[orig_lpoly_id].mappedlist) != 0:
         print 'Arrgh --- found a left polygon mapped into another with a non-null list!'
         sys.exit(1)

    if rpoly_id != 0:
      if rpoly_id != orig_rpoly_id and len(dest_polyid_hash[orig_rpoly_id].mappedlist) != 0:
         print 'Arrgh --- found a right polygon mapped into another with a non-null list!'
         sys.exit(1)

    if lpoly_id != rpoly_id :
      if lpoly_id != 0:
        try:
            module.poly_line_links[lpoly_id].append( tlid )
        except:
            module.poly_line_links[lpoly_id] = [ tlid ]
  
      if rpoly_id != 0:
        try:
            module.poly_line_links[rpoly_id].append( tlid )
        except:
            module.poly_line_links[rpoly_id] = [ tlid ]
  
      link_count = link_count + 1

    feat.Destroy()

    feat = link_layer.GetNextFeature()

print 'Processed %d links.' % link_count


# Boy, what a mess.  But we now have all the data we need in hashes, so we
# can loop over POLYGON records and extract data as we need it.
# Do that now:
#############################################################################
# Process all polygon features.

feat = poly_layer.GetNextFeature()
tile_ref_field = feat.GetFieldIndex( 'MODULE' )
polyid_field = feat.GetFieldIndex( 'POLYID' )

poly_count = 0

while feat is not None:
    module = modules_hash[feat.GetField( tile_ref_field )]
    orig_polyid = feat.GetField( polyid_field )

    try:
      polyid=dest_polyid_hash[orig_polyid].polyid
    except:
      polyid=0

    # we must only do those polygons that have not been mapped to others, or
    # we'll be doing them multiple times
    if polyid != 0 and polyid == orig_polyid:
      tlid_list = module.poly_line_links[polyid]
  
      link_coll = ogr.Geometry( type = ogr.wkbGeometryCollection )
      for tlid in tlid_list:
          geom = module.lines[tlid]
          link_coll.AddGeometry( geom )
  
      try:
          poly = ogr.BuildPolygonFromEdges( link_coll )
  
          #print poly.ExportToWkt()
          #feat.SetGeometryDirectly( poly )
  
          feat2 = ogr.Feature(feature_def=shp_layer.GetLayerDefn())
  
          for fld_index in range(len(field_names)):
             theFieldName = field_names[fld_index]
             layerName = fields_hash[theFieldName].layerName
             theFieldIdx = fields_hash[theFieldName].fld_index
#             print 'fetching field %s from layer %s' % (theFieldName, layerName)
             # if it's from Polygon just pop it in because we have it now:
             if layerName == 'Polygon':
                feat2.SetField( fld_index, feat.GetField(theFieldIdx) )
             # If it's from PIP, we definitely have one
             elif layerName == 'PIP':
                feat2.SetField(fld_index, pip_hash[polyid].GetField(theFieldIdx))
             # this could be a problem, coz there might not be one
             elif layerName == 'AreaLandmarks':
                try:
                  feat3 = areaLandmarks_hash[polyid]
#                  print ' found feature with polyid %d in AreaLandmarks' % polyid
                  feat2.SetField(fld_index, feat3.GetField(theFieldIdx))
                except:
                  feat2.UnsetField(fld_index)
             # this one's mega tricky, coz it depends on there being
             # an AreaLandmarks first
             elif layerName == 'Landmarks':
                try:
                  feat3 = areaLandmarks_hash[polyid]
#                  print ' found feature with polyid %d in AreaLandmarks' % polyid
                  landidx1 = feat3.GetFieldIndex('LAND')
#                  print '  LAND is field %d in AreaLandmarks' % landidx1
#                  print '   LAND field in this record is %d.' % feat3.GetField(landidx1)
                  feat4 = Landmarks_hash[feat3.GetField(landidx1)]
#                  print ' found feature with LAND %d in Landmarks' % feat3.GetField(landidx1)
                  feat2.SetField(fld_index, feat4.GetField(theFieldIdx))
                except:
                  feat2.UnsetField(fld_index)
             else:
                print 'unknown layer %s referenced.' % layerName
  
          feat2.SetGeometryDirectly( poly )
  
          shp_layer.CreateFeature( feat2 )
          feat2.Destroy()
  
          poly_count = poly_count + 1
      except:
          print 'BuildPolygonFromEdges failed.'
  
    feat.Destroy()

    feat = poly_layer.GetNextFeature()

print 'Built %d polygons.' % poly_count
           

#############################################################################
# Cleanup

shp_ds.Destroy()
ds.Destroy()
