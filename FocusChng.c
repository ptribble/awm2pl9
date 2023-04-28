


#ifndef lint
static char *rcsid_FocusChng_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/FocusChng.c,v 1.2 89/02/07 20:04:50 jkh Exp $";
#endif	lint

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
 * 001 -- Jordan Hubbard, U.C. Berkeley.
 *   Hacks for autoraise and titles.
 * 002 -- Jordan Hubbard, Ardent Computer
 *   Window pixmap changes on focus in/out.
 * 1.4 -- Fixed Focus In/Out handling. Created LightsOn() and LightsOff()
 * to break out the actual process of "highlighting".
 * 1.5 -- Added dynamic installation of colormaps.
 */


#include "awm.h"

#ifdef PCS
#include <bsd/sys/time.h>
#endif /* PCS */
#ifdef titan
#include <sys/time.h>
#endif /* titan */

Window FocusWindow;

HandleFocusIn(ev)
XEvent *ev;
{
     XEnterWindowEvent *e = (XEnterWindowEvent *)ev;
     Window w = e->window;
     AwmInfoPtr awi;
     XEvent event;

     Entry("HandleFocusIn")

     if (!(Hilite || Autoraise || InstallColormap) || Snatched)
	 Leave(FALSE)

     awi = GetAwmInfo(w);
     if (!awi)  /* probably a menu or something */
         Leave(FALSE)
     w = (FrameFocus && awi->frame) ? awi->frame : awi->client;

     if (e->detail != NotifyInferior && (e->focus || FrameFocus)) {
	  struct timeval foo;

	  if (e->type != FocusIn) {
	       foo.tv_sec = 0;
	       foo.tv_usec = RaiseDelay * 1000; 
	       /*
		* Sleep for awhile to avoid race conditions and give any
		* potential leave events a chance to get here..
		*/
	       select(0, 0, 0, 0, &foo);
	       /* Did we leave this window already? */
	       if (XCheckTypedWindowEvent(dpy, w, LeaveNotify, &event))
		    if (event.xcrossing.detail != NotifyInferior)
			 Leave(FALSE)
	       /*
		* Install a colormap, if necessary.
		*/
	       if (InstallColormap) {
		    XWindowAttributes xwa;

		    XGetWindowAttributes(dpy, w, &xwa);
		    XInstallColormap(dpy, xwa.colormap);
	       }

	       /*
		* If Autoraise is set, raise that puppy..
		*/
	       if  (Autoraise && (awi->attrs & AT_RAISE))
		    XRaiseWindow(dpy, (awi->frame) ? awi->frame : awi->client);

	       if (!FocusSetByUser && FocusWindow != awi->client &&
		   (awi->attrs & AT_INPUT)) {
		    if (FrameFocus) {
			 XSetInputFocus(dpy, awi->client, RevertToPointerRoot,
					CurrentTime);
			 FocusSetByWM = TRUE;
		    }
		    FocusWindow = awi->client;
	       }
	       else
		    Leave(FALSE)
	  }
	  else {
	       FocusWindow = awi->client;
	       FocusSetByWM = FALSE;
	  }
	  LightsOn(awi);
     }
     Leave(FALSE)
}

HandleFocusOut(ev)
XEvent *ev;
{
     XLeaveWindowEvent *e = (XEnterWindowEvent *)ev;
     Window w = e->window;
     AwmInfoPtr awi;
     XEvent event;

     Entry("HandleFocusOut")

     awi = GetAwmInfo(w);
     if (!awi) /* probably a menu or something */
         Leave(FALSE)

     if (!(Hilite || Autoraise || InstallColormap) || Snatched)
	 Leave(FALSE)

     w =  (FrameFocus && awi->frame) ? awi->frame : awi->client;

     if (e->detail != NotifyInferior && (e->focus || FrameFocus)) {
	  if (e->type != FocusOut) {
	       /* Did we come back into this window? */
	       if (XCheckTypedWindowEvent(dpy, w, EnterNotify, &event)) {
		    if (event.xcrossing.detail != NotifyInferior)
			 Leave(FALSE)
	       }
	       if (!FocusSetByUser) {
		    if (FrameFocus) {
			 XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot,
					CurrentTime);
			 FocusSetByWM = TRUE;
		    }
		    FocusWindow = RootWindow(dpy, scr);
	       }
	       else
		    Leave(FALSE)
	  }
	  else {
	       FocusWindow = RootWindow(dpy, scr);
	       FocusSetByWM = FALSE;
	  }
	  if (InstallColormap)
	       XInstallColormap(dpy, DefaultColormap(dpy, scr));
	  LightsOff(awi);
     }
     Leave(FALSE)
}

/*
 * Turns higlighting off for a window. Counterpart to LightsOn
 */
LightsOff(awi)
AwmInfoPtr awi;
{
     Entry("LightsOff")

     /* set the window border to "gray" stipple */
     if (awi->attrs & AT_INPUT) {
#ifndef RAINBOW
	  if (BorderHilite)
	       SetBorderPixmaps(awi, GrayPixmap);
#else
	  if (BorderHilite)
	       SetBorderPixmaps(awi, awi->grayPixmap);
#endif
	  if (awi->frame) {
	       if (awi->back && awi->title)
		    XSetWindowBackgroundPixmap(dpy, awi->title,
					       awi->back);
	       if (awi->BC_back && (awi->attrs & AT_BORDER)) {
		    XSetWindowBackgroundPixmap(dpy, awi->frame,
					       awi->BC_back);
		    XClearWindow(dpy, awi->frame);
	       }
	       PaintTitle(awi->title, FALSE);
	  }
	  XSync(dpy, 0);
     }
     Leave(FALSE)
}


/*
 * Highlight a window. This may involve shuffling pixmaps around
 * and affecting more than just the client window.
 */
LightsOn(awi)
AwmInfoPtr awi;
{
     Entry("LightsOn")

     if (awi->attrs & AT_INPUT) {
#ifndef RAINBOW
	  if (BorderHilite)
	       SetBorderPixmaps(awi, SolidPixmap);
#else
	  if (BorderHilite)
	       SetBorderPixmaps(awi, awi->solidPixmap);
#endif
	  if (awi->frame) {
	       if (awi->bold && awi->title) {
		    XSetWindowBackgroundPixmap(dpy, awi->title,
					       awi->bold);
		    PaintTitle(awi->title, TRUE);
	       }
	       if (awi->BC_bold && (awi->attrs & AT_BORDER)) {
		    XSetWindowBackgroundPixmap(dpy, awi->frame,
					       awi->BC_bold);
		    XClearWindow(dpy, awi->frame);
	       }
	       XSync(dpy, 0);
	  }
     }
     Leave(FALSE)
}

void SetBorderPixmaps(awi, pix)
AwmInfoPtr awi;
Pixmap pix;
{
     Entry("SetBorderPixmaps")

     XSetWindowBorderPixmap(dpy, awi->client, pix);
     if (awi->frame) {
	  if (awi->title)
	  	XSetWindowBorderPixmap(dpy, awi->title, pix);
	  XSetWindowBorderPixmap(dpy, awi->frame, pix);
     }
     Leave_void
}
