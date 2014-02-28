/*
 * Copyright (c) 1995 Frank Warmerdam
 *
 * This code is in the public domain.
 *
 * $Log: dbfcat.c,v $
 * Revision 1.4  2010/07/11 20:30:00  we7u
 * More minor tweaks to get rid of compiler warnings.  Of particular note are
 * some TODO entries added to a couple of files for two enumerated values that
 * weren't being handled in "switch" statements.  There still isn't any code
 * for those case statements, but the compiler warnings are gone.
 *
 * Revision 1.3  2010/07/11 07:24:37  we7u
 * Fixing multiple minor warnings with Shapelib.  Still plenty left.
 *
 * Revision 1.2  2007/07/25 15:45:27  we7u
 * Adding includes necessary for warning-free compiles.
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
 * Revision 1.1  1999/05/26 02:56:31  candrsn
 * updates to shpdxf, dbfinfo, port from Shapelib 1.1.5 of dbfcat and shpinfo
 *
 * Revision 1.3  1995/10/21  03:15:01  warmerda
 * Changed to use binary file access.
 *
 * Revision 1.2  1995/08/04  03:16:22  warmerda
 * Added header.
 *
 */

//static char rcsid[] = 
//  "$Id: dbfcat.c,v 1.4 2010/07/11 20:30:00 we7u Exp $";

#include "shapefil.h"
#include <stdlib.h>
#include <string.h>

int main( int argc, char ** argv )

{
    DBFHandle	hDBF;
//    int		*panWidth;
    int     i, iRecord;
//    char	szFormat[32], szField[1024];
    char	cTitle[32], nTitle[32];
    int		nWidth, nDecimals;
    int		cnWidth, cnDecimals;
    DBFHandle	cDBF;
    DBFFieldType	hType,cType;
    int		ci, ciRecord;
    char	tfile[160];
    int		hflds, cflds;
//    int     j;
    int 	verbose				= 0;
    int		force 				= 0;
    int		mismatch			= 0;
    int		matches				= 0;
    char	fld_m[256];
    int		shift = 0;
    char	type_names[4][15] = {"integer", "string", "double", "double"};

    if( argc < 3 )
    {
	printf( "dbfcat [-v] [-f] from_DBFfile to_DBFfile\n" );
	exit( 1 );
    }


    if ( strcmp ("-v", argv[1] ) == 0 ) { shift = 1; verbose = 1; }
    if ( strcmp ("-f", argv[1 + shift] ) == 0 ) { shift ++; force = 1; }
    if ( strcmp ("-v", argv[1 + shift] ) == 0 ) { shift ++; verbose = 1; }
    strcpy (tfile, argv[1 + shift]);
    strcat (tfile, ".dbf"); 
    hDBF = DBFOpen( tfile, "rb" );
    if( hDBF == NULL )
    {
	printf( "DBFOpen(%s.dbf,\"r\") failed for From_DBF.\n", tfile );
	exit( 2 );
    }

    strcpy (tfile, argv[2 + shift]);
    strcat (tfile, ".dbf"); 

    cDBF = DBFOpen( tfile, "rb+" );
    if( cDBF == NULL )
    {
	printf( "DBFOpen(%s.dbf,\"rb+\") failed for To_DBF.\n", tfile );
	exit( 2 );
    }
    
    
    if( DBFGetFieldCount(hDBF) == 0 )
    {
	printf( "There are no fields in this table!\n" );
	exit( 3 );
    }

    hflds = DBFGetFieldCount(hDBF);
    cflds = DBFGetFieldCount(cDBF);

    matches = 0;
    for( i = 0; i < hflds; i++ )
    {
	char		szTitle[18];
	char		cname[18];
	int		j;
	hType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );
	fld_m[i] = -1;
        for ( j = 0; j < cflds; j ++ )
          { 
            cType = DBFGetFieldInfo( cDBF, j, cname, &cnWidth, &cnDecimals );
            if ( strcmp (cname, szTitle) == 0 ) 
	     { 
        	if ( hType != cType ) 
            	{ printf ("Incompatible fields %s(%s) != %s(%s),\n",
             	   type_names[hType],nTitle,type_names[cType],cTitle);
			mismatch = 1;
              		}
	        fld_m[i] = j;
		if ( verbose ) 
         	  { printf("%s  %s(%d,%d) <- %s  %s(%d,%d)\n", cname, type_names[cType],
         	  		cnWidth, cnDecimals,
         	       szTitle, type_names[hType], nWidth, nDecimals); }
	        j = cflds;
	        matches = 1;
	      }
	  }
    }

    if ( (matches == 0 ) && !force ) {
      printf ("ERROR: No field names match for tables, cannot proceed\n   use -f to force processing using blank records\n");
      exit(-1); }
    if ( mismatch && !force ) {
      printf ("ERROR: field type mismatch cannot proceed\n    use -f to force processing using attempted conversions\n");
      exit(-1); }

    for( iRecord = 0; iRecord < DBFGetRecordCount(hDBF); iRecord++ )
    {
      ciRecord = DBFGetRecordCount( cDBF );
      for ( i = 0; i < hflds;i ++ )
	{	
//	double	cf;
	ci = fld_m[i];
	if ( ci != -1 )
	{
	cType = DBFGetFieldInfo( cDBF, ci, cTitle, &cnWidth, &cnDecimals );
	hType = DBFGetFieldInfo( hDBF, i, nTitle, &nWidth, &nDecimals );

	    switch( cType )
	    {
	      case FTString:
	        DBFWriteStringAttribute(cDBF, ciRecord, ci,
     			(char *) DBFReadStringAttribute( hDBF, iRecord, i ) );
		break;

	      case FTInteger:
	    	DBFWriteIntegerAttribute(cDBF, ciRecord, ci, 
			(int) DBFReadIntegerAttribute( hDBF, iRecord, i ) );
		break;

	      case FTDouble:
/*	        cf = DBFReadDoubleAttribute( hDBF, iRecord, i );
	        printf ("%s <-  %s (%f)\n", cTitle, nTitle, cf);
*/
	    	DBFWriteDoubleAttribute(cDBF, ciRecord, ci, 
			(double) DBFReadDoubleAttribute( hDBF, iRecord, i ) );
		break;

          case FTLogical:
// TODO:  Add code here
        break;

          case FTInvalid:
// TODO:  Add code here
        break;

	    }
	  }
	}   /* fields names match */
    }

    if ( verbose ) { printf (" %d records appended \n\n", iRecord); }
    DBFClose( hDBF );
    DBFClose( cDBF );

    return( 0 );
}
