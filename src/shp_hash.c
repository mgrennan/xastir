/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*-
 * $Id: shp_hash.c,v 1.29 2012/08/25 16:38:29 tvrusso Exp $
 *
 * XASTIR, Amateur Station Tracking and Information Reporting
 * Copyright (C) 1999,2000  Frank Giannandrea
 * Copyright (C) 2000-2012  The Xastir Group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Look at the README for more information on the program.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  // HAVE_CONFIG_H

#ifdef USE_RTREE
#ifdef HAVE_LIBSHP

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else   // TIME_WITH_SYS_TIME
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else  // HAVE_SYS_TIME_H
#  include <time.h>
# endif // HAVE_SYS_TIME_H
#endif  // TIME_WITH_SYS_TIME

#ifdef HAVE_SHAPEFIL_H
#include <shapefil.h>
#else
#ifdef HAVE_LIBSHP_SHAPEFIL_H
#include <libshp/shapefil.h>
#else
#error HAVE_LIBSHP defined but no corresponding include defined
#endif  // HAVE_LIBSHP_SHAPEFIL_H
#endif  // HAVE_SHAPEFIL_H


#include <rtree/index.h>

#include "xastir.h"
#include "util.h"
#include "hashtable.h"
#include "hashtable_itr.h"
/// THIS ONLY FOR DEBUGGING!
#include "hashtable_private.h"
#include "shp_hash.h"
#include "snprintf.h"

// Must be last include file
#include "leak_detection.h"



#define PURGE_PERIOD 3600     // One hour, hard coded for now.
                              // This should be in a slider in the timing
                              // configuration instead.

//#define PURGE_PERIOD 120  //  debugging

static struct hashtable *shp_hash=NULL;
static time_t purge_time;

#define SHP_HASH_SIZE 65535
#define CHECKMALLOC(m)  if (!m) { fprintf(stderr, "***** Malloc Failed *****\n"); exit(0); }





unsigned int shape_hash_from_key(void *key) {
    char *str=(char *)key;
    unsigned int shphash=5381;
    int c;
    int i=0;
    while (str[i]!='\0') {
        c=str[i++];
        shphash = ((shphash << 5) + shphash)^c;
    }

    return (shphash);
}





int shape_keys_equal(void *key1, void *key2) {

    //    fprintf(stderr,"Comparing %s to %s\n",(char *)key1,(char *)key2);
    if (strncmp((char *)key1,(char *)key2,strlen((char *)key1))==0) {
        //        fprintf(stderr,"    match\n");
        return(1);
    } else {
        //        fprintf(stderr,"  no  match\n");
        return(0);
    }
}





void init_shp_hash(int clobber) {
    //fprintf(stderr," Initializing shape hash \n");
    // make sure we don't leak
    if (shp_hash) {
        if (clobber) {
            hashtable_destroy(shp_hash, 1);
            shp_hash=create_hashtable(SHP_HASH_SIZE,
                                      shape_hash_from_key,
                                      shape_keys_equal);
        }
    } else {
        shp_hash=create_hashtable(SHP_HASH_SIZE,
                                  shape_hash_from_key,
                                  shape_keys_equal);
    }

    // Now set the static timer value to the next time we need to run the purge
    // routine
    purge_time = sec_now() + PURGE_PERIOD;
}





// destructor for a single shapeinfo structure
void destroy_shpinfo(shpinfo *si) {
    if (si) {
        empty_shpinfo(si);
        //        fprintf(stderr,
        //                "       Freeing shpinfo %lx\n",
        //        (unsigned long int) si);
        free(si);
    }
}





// free the pointers in a shapinfo object
void empty_shpinfo(shpinfo *si) {
    if (si) {
        if (si->root) {
            //            fprintf(stderr,"        Freeing root\n");
            Xastir_RTreeDestroyNode(si->root);
            si->root=NULL;
        }

        // This is a little annoying --- the hashtable functions free the
        // key, which is in our case the filename.  So since we're only going
        // to empty the shpinfo when we're removing from the hashtable, we
        // must not free the filename ourselves.
        //        if (si->filename) {
        //            fprintf(stderr,"        Freeing filename\n");
        //            free(si->filename);
        //            si->filename=NULL;
        //        }
    }
}





void destroy_shp_hash(void) {
    struct hashtable_itr *iterator=NULL;
    shpinfo *si;
    int ret;

    if (shp_hash) {
        // walk through the hashtable, free any pointers in the values
        // that aren't null, or we'll leak like a sieve.

        // the hashtable functions always attempt to dereference iterator,
        // and don't check if you give it a null, but will return null if
        // there's nothing in the table.  Grrrr.
        iterator=hashtable_iterator(shp_hash);
        do {
            ret=0;
            if (iterator) {
                si = hashtable_iterator_value(iterator);
                if (si) 
                    empty_shpinfo(si);
                ret=hashtable_iterator_advance(iterator);
            }
        } while (ret);
        hashtable_destroy(shp_hash, 1);  // destroy the hashtable, freeing
                                         // what's left of the entries
        shp_hash=NULL;
        if (iterator) free(iterator);
    }
}





void add_shp_to_hash(char *filename, SHPHandle sHP) {

    // This function does NOT check whether there already is something in 
    // the hashtable that matches.
    // Check that before calling this routine.

    shpinfo *temp;
    int filenm_len;

    filenm_len=strlen(filename);
    if (!shp_hash) {  // no table to add to
        init_shp_hash(1); // so create one
    }
    temp = (shpinfo *)malloc(sizeof(shpinfo));
    CHECKMALLOC(temp);
    // leave room for terminator
    temp->filename = (char *) malloc(sizeof(char)*(filenm_len+1));
    CHECKMALLOC(temp->filename);

    strncpy(temp->filename,filename,filenm_len+1);
    temp->filename[filenm_len]='\0';  // just to be safe
//    xastir_snprintf(temp->filename,sizeof(shpinfo),"%s",filename);

    temp->root = Xastir_RTreeNewIndex();  
    temp->creation = sec_now();
    temp->last_access = temp->creation;

    build_rtree(&(temp->root),sHP);

    //fprintf(stderr, "  adding %s...",temp->filename);
    if (!hashtable_insert(shp_hash,temp->filename,temp)) {
        fprintf(stderr,"Insert failed on shapefile hash --- fatal\n");
        free(temp->filename);
        free(temp);
        exit(1);
    }
}





shpinfo *get_shp_from_hash(char *filename) {
    shpinfo *result;
    if (!shp_hash) {  // no table to search
        init_shp_hash(1); // so create one
        return NULL;
    }

    //fprintf(stderr,"   searching for %s...",filename);

    result=hashtable_search(shp_hash,filename);

    //    if (result) {
    //        fprintf(stderr,"      found it\n");
    //    } else {
    //        fprintf(stderr,"      it is not there\n");
    //    }

    // If there is one, we have now accessed it, so bump the last access time
    if (result) {
        result->last_access = sec_now();
    }

    return (result);

}





//CAREFUL:  note how adding things to the tree can change the root
// Must not ever use cache a value of the root pointer if there's any
// chance that the tree needs to be expanded!
void build_rtree (struct Node **root, SHPHandle sHP) {
    int nEntities;
    intptr_t i;
    SHPObject	*psCShape;
    struct Rect bbox_shape;
    SHPGetInfo(sHP, &nEntities, NULL, NULL, NULL);
    for( i = 0; i < nEntities; i++ ) {
        psCShape = SHPReadObject ( sHP, i );
     	if (psCShape != NULL) {
            bbox_shape.boundary[0]=(RectReal) psCShape->dfXMin;
            bbox_shape.boundary[1]=(RectReal) psCShape->dfYMin;
            bbox_shape.boundary[2]=(RectReal) psCShape->dfXMax;
            bbox_shape.boundary[3]=(RectReal) psCShape->dfYMax;
            SHPDestroyObject ( psCShape );
            // Only insert the rect if it will not fail the assertion in 
            // Xastir_RTreeInsertRect --- this will cause us to ignore any shapes that
            // have invalid bboxes (or that return invalid bboxes from shapelib
            // for whatever reason
            if (bbox_shape.boundary[0] <= bbox_shape.boundary[2] &&
                    bbox_shape.boundary[1] <= bbox_shape.boundary[3]) {
                Xastir_RTreeInsertRect(&bbox_shape, (void *)(i+1), root, 0);
            }
        }
    }
}





void purge_shp_hash(time_t secs_now) {
    struct hashtable_itr *iterator=NULL;
    shpinfo *si;
    int ret;


    if (secs_now > purge_time) {  // Time to purge
        //time_now = localtime(&secs_now);
        //(void)strftime(timestring,100,"%a %b %d %H:%M:%S %Z %Y",time_now);
        //fprintf(stderr,"Purging...%s\n",timestring);

        purge_time += PURGE_PERIOD;

        if (shp_hash) {
            // walk through the hash table and kill entries that are old

            iterator=hashtable_iterator(shp_hash);
            do {
                ret=0;
                if (iterator) {  // must check this, because could be null
                                 // if the iterator malloc failed
                    si=hashtable_iterator_value(iterator);
                    
                    if (si) {
                        if (secs_now > si->last_access+PURGE_PERIOD) {
                            // this is stale, hasn't been accessed in a while
                            //fprintf(stderr,
                            // "found stale entry for %s, deleting it.\n",
                            // si->filename);
                            //fprintf(stderr,"    Destroying si=%lx\n",
                            //            (unsigned long int) si);
                            ret=hashtable_iterator_remove(iterator);
                            
                            // Important that we NOT do the
                            // destroy first, because we've used
                            // the filename pointer field of the
                            // structure as the key, and the
                            // remove function will free that.  If
                            // we clobber the struct first, we
                            // invite segfaults
                            destroy_shpinfo(si);
                            
                            //fprintf(stderr,"    removing from hashtable\n");
                        } else {
                            ret=hashtable_iterator_advance(iterator);
                        }
                    }
                }
            } while (ret);
            // we're now done with the iterator.  Free it to stop us from
            // leaking!
            if (iterator) free(iterator);
        }
        //fprintf(stderr,"   done Purging...\n");
    }
}

#endif  // HAVE_LIBSHP
#endif // USE_RTREE


// To get rid of "-pedantic" compiler warning:
int NON_EMPTY_SOURCE_FILE;


