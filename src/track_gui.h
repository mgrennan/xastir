/*
 * $Id: track_gui.h,v 1.12 2012/11/01 18:57:19 we7u Exp $
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
 *
 *
 */

#ifndef __TRACK_GUI_H
#define __TRACK_GUI_H

/* from track_gui.c */
extern void track_gui_init(void);
extern void Track_station(Widget w, XtPointer clientData, XtPointer callData);
extern int track_station_on;
extern int track_me;
extern int track_case;
extern int track_match;
extern char tracking_station_call[30];
extern void Download_findu_trail(Widget w, XtPointer clientData, XtPointer callData);

#endif  // __TRACK_GUI_H


