
#ifndef lint
static char *rcsid_errHndlr_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/errHndlr.c,v 1.2 89/02/07 21:24:38 jkh Exp $";
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

#include <stdio.h>
#include "X11/Xlib.h"

int errorStatus = False;

#ifdef DEBUG
void debug(s, x1, x2, x3, x4, x5, x6, x7, x8)
char *s;
long x1, x2, x3, x4, x5, x6, x7, x8;
{
	static FILE *con = 0;

	if (!con)
		con = fopen("/dev/console", "w");
	fprintf(con, s, x1, x2, x3, x4, x5, x6, x7, x8);
}
#endif

/*ARGSUSED*/
int ErrorHandler (dpy, event)
    Display *dpy;
    XErrorEvent *event;
{
#ifdef DEBUG
    char *buffer[BUFSIZ];
    XGetErrorText(dpy, event->error_code, buffer, BUFSIZ);
    (void) debug("Hey!\n");
    (void) debug("X Error: %s\n", buffer);
    (void) debug("  Request Major code: %d\n", event->request_code);
    (void) debug("  Request Minor code: %d\n", event->minor_code);
    (void) debug("  ResourceId 0x%x\n", event->resourceid);
    (void) debug("  Error Serial #%d\n", event->serial);
    (void) debug("  Current Serial #%d\n", dpy->request);
#endif
    errorStatus = True;
    return 0;
}

static char *eventname[] = {
	"zero",
	"one",
	"KeyPress",
	"KeyRelease",
	"ButtonPress",
	"ButtonRelease",
	"MotionNotify",
	"EnterNotify",
	"LeaveNotify",
	"FocusIn",
	"FocusOut",
	"KeymapNotify",
	"Expose",
	"GraphicsExpose",
	"NoExpose",
	"VisibilityNotify",
	"CreateNotify",
	"DestroyNotify",
	"UnmapNotify",
	"MapNotify",
	"MapRequest",
	"ReparentNotify",
	"ConfigureNotify",
	"ConfigureRequest",
	"GravityNotify",
	"ResizeRequest",
	"CirculateNotify",
	"CirculateRequest",
	"PropertyNotify",
	"SelectionClear",
	"SelectionRequest",
	"SelectionNotify",
	"ColormapNotify",
	"ClientMessage",
	"MappingNotify",
};

void print_event_info(s, ev)
char *s;
XEvent *ev;
{
	fprintf(stderr, "%s: EVENT %s(%d) on %x\n", s, eventname[ev->type],
	  ev->type, ev->xany.window);
}
