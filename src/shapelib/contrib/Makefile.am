
# $Id: Makefile.am,v 1.13 2012/11/01 18:57:19 we7u Exp $
# C2pyright (C) 2004-2012  The Xastir Group

EXTRA_DIST=Makefile_orig ShapeFileII.pas dbfcat.c dbfinfo.c makefile.vc \
	my_nan.h shpcat.c shpcentrd.c shpdata.c shpdxf.c shpfix.c \
	shpgeo.c shpgeo.h shpinfo.c shpproj.c shpwkb.c

noinst_LIBRARIES=libshape.a

noinst_PROGRAMS=dbfcat \
        dbfinfo \
        shpcat \
        shpdxf \
        shpfix \
        shpinfo @CONTSHP@

EXTRA_PROGRAMS = shpcentrd shpproj shpdata shpwkb
libshape_a_SOURCES=../shpopen.c \
        ../shptree.c \
        ../dbfopen.c \
        ../shapefil.h

shpcentrd_SOURCES=shpcentrd.c \
        shpgeo.c \
        shpgeo.h

shpdata_SOURCES=shpdata.c \
        shpgeo.c \
        shpgeo.h

shpproj_SOURCES=shpproj.c \
        shpgeo.c \
        shpgeo.h

shpwkb_SOURCES=shpwkb.c \
        shpgeo.c \
        shpgeo.h

dbfcat_LDADD=libshape.a
dbfinfo_LDADD=libshape.a
shpcat_LDADD=libshape.a
shpcentrd_LDADD=libshape.a
shpdata_LDADD=libshape.a
shpdxf_LDADD=libshape.a
shpfix_LDADD=libshape.a
shpinfo_LDADD=libshape.a
shpproj_LDADD=libshape.a
shpwkb_LDADD=libshape.a


