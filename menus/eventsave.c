
#ifndef lint
static char sccs_id[] = "@(#)eventsave.c	2.1 12/16/87  Siemens Corporate Research and Support, Inc.";
#endif


/* 
  RTL Menu Package Version 1.0
  by Joe Camaratta and Mike Berman, Siemens RTL, Princeton NJ, 1987

  eventsave.c: saves/restores the X event queue
*/

/*
 *
 * Copyright 1987, 1988 by Ardent Computer Corporation, Sunnyvale, Ca.
 *
 * Copyright 1987 by Jordan Hubbard.
 *
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Ardent Computer
 * Corporation or Jordan Hubbard not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.
 *
 */

/*

Copyright 1987 by
	Siemens Corporate Research and Support, Inc., Princeton, New Jersey

Permission to use, copy, modify, and distribute this software
and its documentation for any purpose and without fee is
hereby granted, provided that the above copyright notice
appear in all copies and that both that copyright notice and
this permission notice appear in supporting documentation, and
that the name of Siemens not be used in advertising or
publicity pertaining to distribution of the software without
specific, written prior permission.  Siemens makes no
representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied
warranty.

*/

#include <stdio.h>
#include "dbug.h"
#include "X11/Xlib.h"

#include "eventstack.h"


void SaveEvents(display, Stack, mask)
    Display *display;
    struct Ev_q **Stack;
    unsigned long mask;	 /* Save only events that match mask; */
			 /* others remain on the event queue  */
{
    XEvent ev;
    int Match = 1;

    Entry("SaveEvents")

    while (XPending(display) && Match)
    {
	if (XCheckMaskEvent(display, mask, &ev))
	{
	    DBUG_3("SaveEvents","Saving event type %d\n",ev.type);
	    PushEvent(&ev, Stack);
	}
	else
	    Match = False;
    }
    Leave_void
}

void RestoreEvents(display, Stack)
    Display *display;
    struct Ev_q **Stack;    /* pointer to stack containing saved events */
{
    XEvent ev, *tmp;

    Entry("RestoreEvents")
    
    while (!EventStackIsEmpty(Stack))
    {
	bcopy(&ev, tmp = PopEvent(Stack), sizeof(XEvent));
	XPutBackEvent(display, &ev);
	free(tmp);
    }
    XFlush(display);
    Leave_void
}

void DisposeEvents(display, mask)
    Display *display;
    unsigned long mask;	 /* dispose events that match mask */
{
    int Match = 1;
    XEvent ev;
    
    Entry("DisposeEvents")

    while (XPending(display) && Match)
    {
	if (XCheckMaskEvent(display, mask, &ev))
	{
	    DBUG_3("SaveEvents","Disposing event type %d\n",ev);
	}
	else
	    Match = False;
    }
    Leave_void
}
	
void DiscardEventStore(Stack)
    struct Ev_q **Stack;
{
    Entry("DiscardEventStack")
    
    while (!EventStackIsEmpty(Stack))
    {
	(void) free(PopEvent(Stack));
    }
    Leave_void
}

void AddEventToStore(Stack, ev)
    struct Ev_q **Stack;
    XEvent ev;
{
    Entry("AddEventToStore")

    PushEvent(&ev, Stack);
    Leave_void
}


void RemoveEventFromStore(Stack, ev)
    struct Ev_q **Stack;
    XEvent *ev;
{
     XEvent *tmp;
     Entry("RemoveEventFromStore")

    bcopy(ev, tmp = PopEvent(Stack), sizeof(XEvent));
    free(tmp);
    Leave_void
}
