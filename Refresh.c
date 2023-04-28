


#ifndef lint
static char *rcsid_Refresh_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/Refresh.c,v 1.2 89/02/07 21:23:32 jkh Exp $";
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
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 *
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

 

/*
 * MODIFICATION HISTORY
 *
 * 000 -- M. Gancarz, DEC Ultrix Engineering Group
 * 001 -- Loretta Guarino Reid, DEC Ultrix Engineering Group,
 *  Western Software Lab. Convert to X11.
 * 002 -- Jordan Hubbard, Ardent Computer
 *  Changes for awm.
 */

#include "awm.h"

/*ARGSUSED*/
Boolean Refresh(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                           /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
    Entry("Refresh")

#ifdef titan
    XTitanReset(dpy);
#endif /* titan */
    Leave( Redraw(RootWindow(dpy,scr), mask, button, x, y) )
}



/*ARGSUSED*/
Boolean Redraw(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                             /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
    XWindowAttributes winfo;		/* window info. */
    XSetWindowAttributes swa;		/* New window info */
    Window w;				/* Refresh window. */

    Entry("Redraw")

    /*
     * Get info on the target window.
     */
    status = XGetWindowAttributes(dpy, window, &winfo);
    if (status == FAILURE)
	 Error("Refresh -> Can't query target window.");
    /*
     * Create and map a window which covers the target window, then destroy it.
     */
    swa.override_redirect = TRUE;
    swa.background_pixel = 0;

    /*
     * What we have here is a failure to communicate. This window should
     * more properly be created as a subwindow of "window", but
     * when we do that, all events (Create, Map, Destroy) get reported
     * as coming from "window", not the newly created one. This is
     * very nasty when the DestroyNotify is received. For now, we'll
     * create it on the RootWindow and take the chance of generating an
     * expose on an overlapping window.
     */
    if ((w = XCreateWindow(dpy, RootWindow(dpy, scr), winfo.x, winfo.y, 
			   (unsigned int) winfo.width, 
			   (unsigned int) winfo.height, (unsigned int) 0,
			   DefaultDepth(dpy, scr),
			   CopyFromParent,
			   DefaultVisual(dpy, scr),
			   (CWOverrideRedirect | CWBackPixel),
			   &swa)) == NULL)
	 Error("Refresh -> Can't create refresh window.");
    XMapWindow(dpy, w);
    XDestroyWindow(dpy, w);
    XFlush(dpy);
    Leave(FALSE)
}
