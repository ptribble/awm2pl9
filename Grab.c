


#ifndef lint
static char *rcsid_Grab_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/Grab.c,v 1.2 89/02/07 20:05:05 jkh Exp $";
#endif  lint

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
 * MODIFICATION HISTORY
 *
 * 002 -- Jordan Hubbard, Ardent Computer
 *   Modifications for addition contexts.
 */

#include "X11/Xlib.h"
#include "awm.h"

void grab_pointer()
{
     Entry("grab_pointer")

     if (XGrabPointer( dpy, RootWindow(dpy, scr),
		      TRUE, (unsigned int) EVENTMASK, GrabModeAsync,
		      GrabModeAsync, None,
		      ArrowCrossCursor, CurrentTime )
	 != GrabSuccess )
	  fprintf(stderr, "awm (grab_pointer): Can't grab the mouse.");
      Leave_void
}

void ungrab_pointer()
{
     Entry("ungrab_pointer")

     XUngrabPointer(dpy, CurrentTime);
     Leave_void
}
