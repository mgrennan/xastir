/******************************************************************************
 * $Id: shpdump.c,v 1.4 2010/07/11 07:57:02 we7u Exp $
 *
 * Project:  Shapelib
 * Purpose:  Sample application for dumping contents of a shapefile to 
 *           the terminal in human readable form.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam
 *
 * This software is available under the following "MIT Style" license,
 * or at the option of the licensee under the LGPL (see LICENSE.LGPL).  This
 * option is discussed in more detail in shapelib.html.
 *
 * --
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
 * $Log: shpdump.c,v $
 * Revision 1.4  2010/07/11 07:57:02  we7u
 * Fixing a few more compiler warnings.
 *
 * Revision 1.3  2009/06/04 04:25:19  we7u
 * Getting rid of compiler warnings due to unused variable.
 *
 * Revision 1.2  2007/07/25 15:37:50  we7u
 * Added shprewind.c to the "clean" target of the original Makefile.  Added
 * stdlib.h includes to some of the utilities to get a clean compile.
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
 * Revision 1.10  2002/04/10 16:59:29  warmerda
 * added -validate switch
 *
 * Revision 1.9  2002/01/15 14:36:07  warmerda
 * updated email address
 *
 * Revision 1.8  2000/07/07 13:39:45  warmerda
 * removed unused variables, and added system include files
 *
 * Revision 1.7  1999/11/05 14:12:04  warmerda
 * updated license terms
 *
 * Revision 1.6  1998/12/03 15:48:48  warmerda
 * Added report of shapefile type, and total number of shapes.
 *
 * Revision 1.5  1998/11/09 20:57:36  warmerda
 * use SHPObject.
 *
 * Revision 1.4  1995/10/21 03:14:49  warmerda
 * Changed to use binary file access.
 *
 * Revision 1.3  1995/08/23  02:25:25  warmerda
 * Added support for bounds.
 *
 * Revision 1.2  1995/08/04  03:18:11  warmerda
 * Added header.
 *
 */

//static char rcsid[] =
//    "$Id: shpdump.c,v 1.4 2010/07/11 07:57:02 we7u Exp $";

#include "shapefil.h"
#include <stdlib.h>
#include <string.h>

int main( int argc, char ** argv )

{
    SHPHandle	hSHP;
    int		nShapeType, nEntities, i, iPart, bValidate = 0,nInvalidCount=0;
    const char 	*pszPlus;
    double 	adfMinBound[4], adfMaxBound[4];

    if( argc > 1 && strcmp(argv[1],"-validate") == 0 )
    {
        bValidate = 1;
        argv++;
        argc--;
    }

/* -------------------------------------------------------------------- */
/*      Display a usage message.                                        */
/* -------------------------------------------------------------------- */
    if( argc != 2 )
    {
	printf( "shpdump [-validate] shp_file\n" );
	exit( 1 );
    }

/* -------------------------------------------------------------------- */
/*      Open the passed shapefile.                                      */
/* -------------------------------------------------------------------- */
    hSHP = SHPOpen( argv[1], "rb" );

    if( hSHP == NULL )
    {
	printf( "Unable to open:%s\n", argv[1] );
	exit( 1 );
    }

/* -------------------------------------------------------------------- */
/*      Print out the file bounds.                                      */
/* -------------------------------------------------------------------- */
    SHPGetInfo( hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound );

    printf( "Shapefile Type: %s   # of Shapes: %d\n\n",
            SHPTypeName( nShapeType ), nEntities );
    
    printf( "File Bounds: (%12.3f,%12.3f,%g,%g)\n"
            "         to  (%12.3f,%12.3f,%g,%g)\n",
            adfMinBound[0], 
            adfMinBound[1], 
            adfMinBound[2], 
            adfMinBound[3], 
            adfMaxBound[0], 
            adfMaxBound[1], 
            adfMaxBound[2], 
            adfMaxBound[3] );
    
/* -------------------------------------------------------------------- */
/*	Skim over the list of shapes, printing all the vertices.	*/
/* -------------------------------------------------------------------- */
    for( i = 0; i < nEntities; i++ )
    {
	int		j;
        SHPObject	*psShape;

	psShape = SHPReadObject( hSHP, i );

	printf( "\nShape:%d (%s)  nVertices=%d, nParts=%d\n"
                "  Bounds:(%12.3f,%12.3f, %g, %g)\n"
                "      to (%12.3f,%12.3f, %g, %g)\n",
	        i, SHPTypeName(psShape->nSHPType),
                psShape->nVertices, psShape->nParts,
                psShape->dfXMin, psShape->dfYMin,
                psShape->dfZMin, psShape->dfMMin,
                psShape->dfXMax, psShape->dfYMax,
                psShape->dfZMax, psShape->dfMMax );

	for( j = 0, iPart = 1; j < psShape->nVertices; j++ )
	{
            const char	*pszPartType = "";

            if( j == 0 && psShape->nParts > 0 )
                pszPartType = SHPPartTypeName( psShape->panPartType[0] );
            
	    if( iPart < psShape->nParts
                && psShape->panPartStart[iPart] == j )
	    {
                pszPartType = SHPPartTypeName( psShape->panPartType[iPart] );
		iPart++;
		pszPlus = "+";
	    }
	    else
	        pszPlus = " ";

	    printf("   %s (%12.3f,%12.3f, %g, %g) %s \n",
                   pszPlus,
                   psShape->padfX[j],
                   psShape->padfY[j],
                   psShape->padfZ[j],
                   psShape->padfM[j],
                   pszPartType );
	}

        if( bValidate )
        {
            int nAltered = SHPRewindObject( hSHP, psShape );

            if( nAltered > 0 )
            {
                printf( "  %d rings wound in the wrong direction.\n",
                        nAltered );
                nInvalidCount++;
            }
        }
        
        SHPDestroyObject( psShape );
    }

    SHPClose( hSHP );

    if( bValidate )
    {
        printf( "%d object has invalid ring orderings.\n", nInvalidCount );
    }

#ifdef USE_DBMALLOC
    malloc_dump(2);
#endif

    exit( 0 );
}
