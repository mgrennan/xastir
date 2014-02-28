
// $Id: shp_hash.h,v 1.11 2012/11/01 18:57:19 we7u Exp $
// Copyright (C) 2004-2012  The Xastir Group


#ifndef __XASTIR_SHP_HASH_H
#define __XASTIR_SHP_HASH_H

#ifdef USE_RTREE

#ifdef HAVE_SHAPEFIL_H
#include <shapefil.h>
#else
#ifdef HAVE_LIBSHP_SHAPEFIL_H
#include <libshp/shapefil.h>
#else
#error HAVE_LIBSHP defined but no corresponding include defined
#endif  // HAVE_LIBSHP_SHAPEFIL_H
#endif  // HAVE_SHAPEFIL_H

typedef struct _shpinfo{
    char *filename;
    struct Node* root;
    time_t creation;
    time_t last_access;
    int num_accesses;
}shpinfo;

void init_shp_hash(int clobber);
void add_shp_to_hash(char *filename,SHPHandle sHP);
void build_rtree(struct Node **root, SHPHandle sHP);
void destroy_shp_hash(void);
void empty_shpinfo(shpinfo *si);
void destroy_shpinfo(shpinfo *si);
void purge_shp_hash(time_t secs_now);
shpinfo *get_shp_from_hash(char *filename);

#endif // USE_RTREE
#endif // __XASTIR_SHP_HASH_H
