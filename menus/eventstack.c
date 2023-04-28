
#ifndef lint
     static char sccs_id[] = "@(#)eventstack.c	2.1 12/16/87  Siemens Corporate Research and Support, Inc.";
#endif


/* 
  RTL Menu Package Version 1.0
  by Joe Camaratta and Mike Berman, Siemens RTL, Princeton NJ, 1987
  
  eventstack.c: stack implementation for X events
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
  
  Almost totally re-written by Jordan Hubbard, Ardent computer corp.
  Previous code did not pass more strict compilers.
  */

#include <stdlib.h>
#include <stdio.h>
#include "dbug.h"
#include "X11/Xlib.h"
#include "eventstack.h"

void PushEvent(event, Eq)
XEvent *event;
struct Ev_q **Eq;
{
     struct Ev_q *ptr;
     Entry("PushEvent")
     
     if (!*Eq) {
	  *Eq = ptr = (struct Ev_q *)malloc(sizeof(struct Ev_q));
	  ptr->prev = 0;
     }
     else {
	  ptr = (struct Ev_q *)malloc(sizeof(struct Ev_q));
	  ptr->prev = *Eq;
	  *Eq = ptr;
     }
     ptr->event = (XEvent *)malloc(sizeof(XEvent));
     bcopy(ptr->event, event, sizeof(XEvent));
     Leave_void
}

XEvent *PopEvent(Eq)
struct Ev_q **Eq;
{
     struct Ev_q *ptr;
     
     Entry("PopEvent")

     if (*Eq) {
	  ptr = *Eq;
	  *Eq = ptr->prev;
	  free(ptr);
	  Leave(ptr->event)
     }
     else
	  Leave((XEvent *)0)
}
