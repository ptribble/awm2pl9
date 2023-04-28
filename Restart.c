


#ifndef lint
static char *rcsid_Restart_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/Restart.c,v 1.3 89/02/07 22:39:52 jkh Exp $";
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
    Western Software Lab. Convert to X11.
 * 002 -- Jordan Hubbard, U.C. Berkeley. Titlebar cleanup code.
 */

#include "awm.h"
#include "X11/Xutil.h"

extern XContext AwmContext;

/*ARGSUSED*/
Boolean Restart(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                             /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
    Entry("Restart")

    XFreeFont(dpy, IFontInfo);
    XFreeFont(dpy, PFontInfo);
    XBell(dpy, VOLUME_PERCENTAGE(Volume));
    XBell(dpy, VOLUME_PERCENTAGE(Volume));
    XFlush(dpy);
    Cleanup();
    XSync(dpy, 0); /* be paranoid */
    execvp(*Argv, Argv, Environ);
    fprintf(stderr, "awm: Restart failed!\n");
    exit(1);
}

/*ARGSUSED*/
Boolean Quit(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                             /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
     Entry("Quit")

     Cleanup();
     /*
      * Be a careful boy scout.
      */
     XSync(dpy, 0);
     XCloseDisplay(dpy);
     exit(0);
}

/*ARGSUSED*/
Boolean DestroyClient(window, mask, button, x, y)
Window window;
int mask;
int x, y;
{
     AwmInfoPtr awi;

     Entry("DestroyClient")

     if (window == RootWindow(dpy, scr))
		Leave(FALSE)
     awi = GetAwmInfo(window);
     /*
      * If it's a titled window or an icon (we don't want to nuke ourselves)
      * we should get the client for it. We assume the subsequent destroy
      * notify will tell us to reclaim our resources.
      */
     window = awi->client;
     XKillClient(dpy, window);
     Leave(TRUE)
}

/*
 * Put any necessary cleanup code here, it will be invoked when awm exits
 * or restarts. Currently just checks for title bar resources.
 */
Cleanup()
{
     Window junk, *windows;
     unsigned int nwins;
     AwmInfoPtr awi;

     Entry("Cleanup");

     if (XQueryTree(dpy, DefaultRootWindow(dpy), &junk, &junk, &windows,
		    &nwins)) {
	  int i;
	  
	  for (i = 0; i < (int)nwins; i++) {
	       XWindowAttributes xwa;
	       int bw;

	       awi = GetAwmInfo(windows[i]);
	       if (!awi)
		    continue;
	       bw = awi->border_width;
	       if (awi->state & ST_ICON)
		    XMapWindow(dpy, awi->client);
	       RemoveIcon(windows[i]);
	       XGetWindowAttributes(dpy, awi->frame, &xwa);
	       if (PushDown == FALSE) {
		    if (awi->attrs & AT_TITLE)
			 xwa.y += (titleHeight + 2);
		    else if (awi->attrs & AT_BORDER)
			 xwa.y += BContext;
	       }
	       if (awi->attrs & AT_BORDER)
		    xwa.x -= (BContext + (bw ? bw : 1));
	       XReparentWindow(dpy, awi->client, RootWindow(dpy, scr),
			       xwa.x, xwa.y);
	       XRemoveFromSaveSet(dpy, awi->client);
	       XSetWindowBorderWidth(dpy, awi->client, awi->border_width);
	       free(awi);
	  }
	  if (nwins)
	       XFree(windows);
     }
     Leave_void
}
