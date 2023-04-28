


#ifndef lint
static char *rcsid_Iconify_c = "$Header: /usr/graph2/X11.3/contrib/windowmgrs/awm/RCS/Iconify.c,v 1.2 89/02/07 20:05:12 jkh Exp $";
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
 * 001 -- R. Kittell, DEC Storage A/D May 20, 1986
 *  Add optional warp of mouse to the upper right corner on de-iconify,
 *  and to the icon center on iconify.
 * 002 -- Loretta Guarino Reid, DEC Ultrix Engineering Group,
 *  Western Software Lab. Port to X11.
 * 003 -- Jordan Hubbard, Ardent Computer.
 *  Many mods to cope with context manager, titled windows. Almost a total
 *  rewrite.
 * 1.2 -- Support for IconLabels.. (Isaac Salzman). A few fixes (jkh).
 */

#include "awm.h"

Boolean Iconify(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                             /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
     XWindowAttributes fromInfo;	/* info on "from" window */
     XWindowAttributes toInfo;		/* info on "to" window */
     int mse_x, mse_y;			/* Mouse X and Y coordinates. */
     int sub_win_x, sub_win_y;		/* relative Mouse coordinates. */
     int num_vectors;			/* Number of vectors in zap buffer. */
     unsigned int mmask;			/* Mouse state */
     Window root;			/* Mouse root window. */
     Window from, to;			/* from -> to windows */
     Window sub_win;			/* Mouse position sub-window. */
     XSegment zap[MAX_ZAP_VECTORS];	/* Zap effect vertex buffer. */
     Boolean dozap;			/* local Zap */
     AwmInfoPtr awi;
     extern Window MakeIcon();

     Entry("Iconify")

     /*
      * Do not try to iconify the root window.
      */
     if (window == RootWindow(dpy, scr))
	  Leave(FALSE)
     /*
      * The original idea of zap lines has one flaw. If a window wants
      * to be created iconic, it should just appear that way without any
      * fuss. CheckMap() calls us with (win, 0, 0, 0, 0) when this is the
      * case, so we can special case this to turn off Zap temporarily.
      * Since we don't want to mess with the global "Zap", we use
      * "dozap" instead. 
      */
     dozap = (mask || button || x || y) ? Zap : FALSE;
     /*
      * Clear the vector buffer.
      */
     if (dozap)
	  bzero(zap, sizeof(zap));
     
     /*
      * Get the mouse cursor position in case we must put a new
      * icon there.
      */
     XQueryPointer(dpy, RootWindow(dpy, scr), &root, &sub_win, 
		   &mse_x, &mse_y, &sub_win_x, &sub_win_y, &mmask);
     
     
     /*
      * Figure out which direction we're going in (icon->window or vica-versa)
      */
     awi = GetAwmInfo(window);
     if (!awi)
          Leave(FALSE)
     if (awi->state & ST_ICON) {
          from = awi->icon;
          to = (awi->frame) ? awi->frame : awi->client;
	  if (!(awi->state & ST_PLACED)) {
	       PlaceWindow(to, None);
	       XMapWindow(dpy, awi->client);
	  }
     }
     else if (awi->state & ST_WINDOW) {
          from = (awi->frame) ? awi->frame : awi->client;
          to = (awi->icon) ? awi->icon :  MakeIcon(window, mse_x, mse_y, TRUE);
     }
     else {
          printf("Iconify: Window %x has unknown state '%x'\n",
		 awi->client, awi->state);
	  Leave(FALSE)
     }
     status = XGetWindowAttributes(dpy, from, &fromInfo);
     if (status == FAILURE)
          Leave(FALSE)
     status = XGetWindowAttributes(dpy, to, &toInfo);
     if (status == FAILURE)
          Leave(FALSE)
	
     /*
      * Store the zap vector buffer.
      */
     if (dozap) {
	  num_vectors =
	       StoreZap(zap,
			fromInfo.x - 1,
			fromInfo.y - 1,
			fromInfo.x + fromInfo.width +
			(fromInfo.border_width << 1),
			fromInfo.y + fromInfo.height +
			(fromInfo.border_width << 1),
			toInfo.x - 1,
			toInfo.y - 1,
			toInfo.x + toInfo.width +
			(toInfo.border_width << 1),
			toInfo.y + toInfo.height +
			(toInfo.border_width << 1));
     }
     if (awi->state & ST_ICON) {
	  if (!awi->frame)
	       XRemoveFromSaveSet(dpy, awi->client);
	  awi->state ^= ST_ICON;
	  awi->state |= ST_WINDOW;
#ifdef WMSTATE
	  awi->wm_state.state=NormalState;
	  XChangeProperty(dpy,awi->client,wm_state_atom,wm_state_atom,32,
			  PropModeReplace,(char *) &awi->wm_state,2);
#endif /* WMSTATE */
     }
     else if (awi->state & ST_WINDOW) {
          XAddToSaveSet(dpy, awi->client);
	  awi->state ^= ST_WINDOW;
	  awi->state |= ST_ICON;
#ifdef WMSTATE
	  awi->wm_state.state=IconicState;
	  XChangeProperty(dpy,awi->client,wm_state_atom,wm_state_atom,32,
			  PropModeReplace,(char *) &awi->wm_state,2);
#endif /* WMSTATE */
     }
     else
          fprintf(stderr, "Window state for window %x got munged!\n",
             awi->client);
     /*
      * Map the target.
      */
     XMapRaised(dpy, to);
     if (dozap) {
	  /*
	   * Draw the zap lines.
	   */
	  DrawZap();
     }
     /*
      * Unmap the "from" window.
      */
     XUnmapWindow(dpy, from);
     XFlush(dpy);
     /*
      * Optionally warp the mouse to the upper right corner of the
      *  window.
      */
     if (WarpOnDeIconify && awi->state & ST_WINDOW) {
	  int y;

	  y = (toInfo.height >= 10) ? 10 : toInfo.height / 2;
	  if (awi->frame) { /* compensate for title */
	       XWindowAttributes xwa;
	       XGetWindowAttributes(dpy, awi->title, &xwa);
	       y += xwa.height + 2;
	  }
	  status = XWarpPointer (dpy, None, to,
				 0, 0, 0, 0,
				 toInfo.width >= 7 ?
				 toInfo.width - 7 : toInfo.width / 2,
				 y);
     }

     if (WarpOnIconify && awi->state & ST_ICON)
	  status = XWarpPointer (dpy, None, to, 
				 0, 0, 0, 0,
				 toInfo.width / 2, toInfo.height / 2);
     Leave(FALSE)
}
