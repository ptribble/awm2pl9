
/*
#ifndef lint
static char sccs_id[] = "@(#)eventstack.h	2.1 12/16/87  Siemens Corporate Research and Support, Inc.";
#endif
*/


/* 
 *  RTL Menu Package Version 1.0
 * by Joe Camaratta and Mike Berman, Siemens RTL, Princeton NJ, 1987
 *
 * eventstack.h: data structure definitions for eventstack package.
 * rewritten almost entirely by Jordan Hubbard, Ardent Computer corp.
 * Previous version did not agree with many compilers.
 *
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

#ifndef _XLIB_H_
#include "X11/Xlib.h"
#endif

#include "std_defs.h"

#define EventStackIsEmpty(stk) (*stk == (struct Ev_q *)0)

struct Ev_q {
     struct Ev_q *prev;
     XEvent *event;
};


extern void SaveEvents(), RestoreEvents(), DisposeEvents(),
    DiscardEventStore(), AddEventToStore(), RemoveEventFromStore();

extern Boolean EventStoreIsEmpty();
extern void PushEvent();
extern XEvent *PopEvent();



