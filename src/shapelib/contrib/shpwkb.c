/******************************************************************************
 * Copyright (c) 1999, Carl Anderson
 *
 * this code is based in part on the earlier work of Frank Warmerdam
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 *
 * shpwkb.c  - test WKB binary Input / Output
 *
 *
 * $Log: shpwkb.c,v $
 * Revision 1.3  2010/07/11 07:51:03  we7u
 * Fixing more compiler warnings.  There are a few left yet.
 *
 * Revision 1.2  2010/07/11 07:24:37  we7u
 * Fixing multiple minor warnings with Shapelib.  Still plenty left.
 *
 * Revision 1.1  2006/11/10 21:48:10  tvrusso
 * Add shapelib as an internal library, and use it if we don't find an external
 * one.
 *
 * Make a loud warning if we do so, because the result of this is that we'll
 * have a bigger executable.
 *
 * This commit is bigger than it needs to be, because it includes all of
 * shapelib, including the contrib directory.
 *
 * Added an automake-generated Makefile for this thing.
 *
 * Builds only a static library, and calls it "libshape.a" instead of
 * "libshp.a" so that if we use ask to use the static one while there is
 * also an external one installed, the linker doesn't pull in the shared
 * library one unbidden.
 *
 * This stuff can be tested on a system with libshp installed by configuring with
 * "--without-shapelib"
 *
 * I will be removing Makefile.in because it's not supposed to be in CVS.  My
 * mistake.
 *
 * Revision 1.1  1999/05/26 02:29:36  candrsn
 * OGis Well Known Binary test program (output only)
 *
 *
 * 
 */

#include "shapefil.h"
#include "shpgeo.h"

int main( int argc, char ** argv )
{
    SHPHandle	old_SHP;
//    SHPHandle   new_SHP;
    DBFHandle   old_DBF;
//    DBFHandle   new_DBF;
    int		nShapeType, nEntities, i;
//    int     nVertices, nParts, *panParts, iPart;
//    double	*padVertices, adBounds[4];
//    const char 	*pszPlus;
//    DBFFieldType  idfld_type;
//    int		idfld, nflds;
//    char	kv[257] = "";
//    char	idfldName[120] = "";
//    char	fldName[120] = "";
//    char	shpFileName[120] = "";
//    char	dbfFileName[120] = "";
//    char	*DBFRow = NULL;
//    int		Cpan[2] = { 0,0 };
    int		byRing = 0;
//    PT		oCentrd, ringCentrd;
    SHPObject	*psCShape;
//    SHPObject   *cent_pt;
//    double	oArea = 0.0, oLen = 0.0;
    WKBStreamObj *wkbObj = NULL;
    FILE	*wkb_file = NULL;

    if( argc < 3 )
    {
	printf( "shpwkb shp_file wkb_file\n" );
	exit( 1 );
    }
    
    old_SHP = SHPOpen (argv[1], "rb" );
    old_DBF = DBFOpen (argv[1], "rb");
    if( old_SHP == NULL || old_DBF == NULL )
    {
	printf( "Unable to open old files:%s\n", argv[1] );
	exit( 1 );
    }

    wkb_file = fopen ( argv[2], "wb");
    wkbObj = calloc ( 3, sizeof (int) );
    
    SHPGetInfo( old_SHP, &nEntities, &nShapeType, NULL, NULL );
    for( i = 0; i < nEntities; i++ )
    {
	int		res ;

	psCShape = SHPReadObject( old_SHP, i );

        if ( byRing == 1 ) {
          int 	   ring, prevStart;
//          int      ringDir;
//          double   ringArea;

          prevStart = psCShape->nVertices;
          for ( ring = (psCShape->nParts - 1); ring >= 0; ring-- ) {
	    SHPObject 	*psO;
	    int		numVtx, rStart;
//        int     j;
            
            rStart = psCShape->panPartStart[ring];
            if ( ring == (psCShape->nParts -1) )
              { numVtx = psCShape->nVertices - rStart; }
             else
              { numVtx = psCShape->panPartStart[ring+1] - rStart; }
              
            printf ("(shpdata) Ring(%d) (%d for %d) \n", ring, rStart, numVtx);              
	    psO = SHPClone ( psCShape, ring,  ring + 1 );

	    SHPDestroyObject ( psO );
            printf ("(shpdata) End Ring \n");
           }  /* (ring) [0,nParts  */

          }  /* by ring   */
	   
	   printf ("gonna build a wkb \n");
	   res = SHPWriteOGisWKB ( wkbObj, psCShape );
	   printf ("gonna write a wkb that is %d bytes long \n", wkbObj->StreamPos );	   
	   fwrite ( (void*) wkbObj->wStream, 1, wkbObj->StreamPos, wkb_file );
    }


    free ( wkbObj );
    SHPClose( old_SHP );
    DBFClose( old_DBF );
    if ( wkb_file )  fclose ( wkb_file );

    printf ("\n");
    return(0);
}

